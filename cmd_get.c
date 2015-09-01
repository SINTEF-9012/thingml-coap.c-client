/*
 *  cmd_get.c
 *  SMCP
 *
 *  Created by Robert Quattlebaum on 8/17/10.
 *  Copyright 2010 deepdarc. All rights reserved.
 *
 */

/* This file is a total mess and needs to be cleaned up! */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <smcp/assert-macros.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <smcp/smcp.h>
#include <string.h>
#include <sys/errno.h>
#include "cmd_get.h"
#include <smcp/url-helpers.h>
#include <signal.h>
#include "smcpctl.h"
#include <smcp/smcp-missing.h>

#include "thingml_coap_utility.h"



typedef void (*sig_t)(int);

static int gRet;
static bool get_show_headers, get_observe, get_keep_alive;

static uint16_t size_request;
static int redirect_count;
static cms_t get_timeout;
static bool observe_ignore_first;
static bool observe_once;
static coap_transaction_type_t get_tt;

bool send_get_request(smcp_t smcp, const char* next, coap_size_t nextlen, void *thingML_context);

static int retries = 0;
static coap_size_t next_len = ((coap_size_t)(-1));
static int last_observe_value = -1;
static coap_content_type_t request_accept_type = -1;

static struct smcp_transaction_s transaction;



static smcp_status_t
get_response_handler(int statuscode, void* context) {
	ThingMLCOAPContext * thingml_context = (ThingMLCOAPContext *) context;

	const char* content = (const char*)smcp_inbound_get_content_ptr();
	coap_size_t content_length = smcp_inbound_get_content_len();

	if(statuscode>=0) {
		if(content_length>(smcp_inbound_get_packet_length()-4)) {
			fprintf(stderr, "INTERNAL ERROR: CONTENT_LENGTH LARGER THAN PACKET_LENGTH-4! (content_length=%u, packet_length=%u)\n",content_length,smcp_inbound_get_packet_length());
			gRet = ERRORCODE_UNKNOWN;
			thingml_context->fn_onerror_callback(thingml_context->thing_instance, gRet);
			goto bail;
		}

		if((statuscode >= 0) && get_show_headers) {
			if(next_len != ((coap_size_t)(-1)))
				fprintf(stdout, "\n\n");
			coap_dump_header(
				stdout,
				NULL,
				smcp_inbound_get_packet(),
				smcp_inbound_get_packet_length()
			);
		}

		if(!coap_verify_packet((void*)smcp_inbound_get_packet(), smcp_inbound_get_packet_length())) {
			fprintf(stderr, "INTERNAL ERROR: CALLBACK GIVEN INVALID PACKET!\n");
			gRet = ERRORCODE_UNKNOWN;
			thingml_context->fn_onerror_callback(thingml_context->thing_instance, gRet);
			goto bail;
		}
	}

	if(statuscode == SMCP_STATUS_TRANSACTION_INVALIDATED) {
		gRet = 0;
	}

	if(observe_ignore_first) {
		observe_ignore_first = false;
		goto bail;
	}

	if(	((statuscode < COAP_RESULT_200) ||(statuscode >= COAP_RESULT_400))
		&& (statuscode != SMCP_STATUS_TRANSACTION_INVALIDATED)
		&& (statuscode != HTTP_TO_COAP_CODE(HTTP_RESULT_CODE_PARTIAL_CONTENT))
	) {
		if(get_observe && statuscode == SMCP_STATUS_TIMEOUT) {
			gRet = 0;
		} else {
			gRet = (statuscode == SMCP_STATUS_TIMEOUT)?ERRORCODE_TIMEOUT:ERRORCODE_COAP_ERROR;
			fprintf(stderr, "get: Result code = %d (%s)\n", statuscode,
					(statuscode < 0) ? smcp_status_to_cstr(
					statuscode) : coap_code_to_cstr(statuscode));
			thingml_context->fn_onerror_callback(thingml_context->thing_instance, gRet);
		}
	}

	if((statuscode>0) && content && content_length) {
		coap_option_key_t key;
		const uint8_t* value;
		coap_size_t value_len;
		bool last_block = true;
		int32_t observe_value = -1;

		while((key=smcp_inbound_next_option(&value, &value_len))!=COAP_OPTION_INVALID) {

			if(key == COAP_OPTION_BLOCK2) {
				last_block = !(value[value_len-1]&(1<<3));
			} else if(key == COAP_OPTION_OBSERVE) {
				if(value_len)
					observe_value = value[0];
				else observe_value = 0;
			}

		}

		thingml_context->fn_onmsgrcv_callback(thingml_context->thing_instance, content, content_length);

		last_observe_value = observe_value;
	}

	if(observe_once) {
		gRet = 0;
		goto bail;
	}

bail:
	return SMCP_STATUS_OK;
}

smcp_status_t
resend_get_request(void* context) {
	ThingMLCOAPContext * thingml_context = (ThingMLCOAPContext*) context;
	smcp_status_t status = 0;

	status = smcp_outbound_begin(smcp_get_current_instance(),COAP_METHOD_GET, get_tt);
	require_noerr(status,bail);

	status = smcp_outbound_set_uri(thingml_context->url, 0);
	require_noerr(status,bail);

	if(request_accept_type!=COAP_CONTENT_TYPE_UNKNOWN) {
		status = smcp_outbound_add_option_uint(COAP_OPTION_ACCEPT, request_accept_type);
		require_noerr(status,bail);
	}

	status = smcp_outbound_send();

	if(status) {
		check_noerr(status);
		fprintf(stderr,
			"smcp_outbound_send() returned error %d(%s).\n",
			status,
			smcp_status_to_cstr(status));
		goto bail;
	}

bail:
	return status;
}

bool send_get_request(smcp_t smcp, const char* next, coap_size_t nextlen, void *thingML_context) {
	bool ret = false;
	smcp_status_t status = 0;
	int flags = SMCP_TRANSACTION_ALWAYS_INVALIDATE;
	gRet = ERRORCODE_INPROGRESS;

	retries = 0;

	if(get_observe)
		flags |= SMCP_TRANSACTION_OBSERVE;
	if(get_keep_alive)
		flags |= SMCP_TRANSACTION_KEEPALIVE;

	smcp_transaction_end(smcp,&transaction);
	smcp_transaction_init(
		&transaction,
		flags, // Flags
		(void*)&resend_get_request,
		(void*)&get_response_handler,
		thingML_context
	);

	status = smcp_transaction_begin(smcp, &transaction, get_timeout);

	if(status) {
		check(!status);
		fprintf(stderr,
			"smcp_begin_transaction_old() returned %d(%s).\n",
			status,
			smcp_status_to_cstr(status));
		goto bail;
	}

	ret = true;

bail:
	return ret;
}

int
tool_cmd_get_url(void *thingML_context) {
	ThingMLCOAPContext* context = (ThingMLCOAPContext*) thingML_context;

	get_show_headers = context->client_coap_config.get_show_headers;
	next_len = context->client_coap_config.next_len;
	redirect_count = context->client_coap_config.redirect_count;
	size_request = context->client_coap_config.size_request;
	get_observe = context->client_coap_config.get_observe;
	get_keep_alive = context->client_coap_config.get_keep_alive;
	get_timeout = context->client_coap_config.get_timeout; // Default timeout is 30 seconds.
	last_observe_value = context->client_coap_config.last_observe_value;
	request_accept_type = context->client_coap_config.request_accept_type;
	observe_once = context->client_coap_config.observe_once;
	observe_ignore_first = context->client_coap_config.observe_ignore_first;
	get_tt = context->client_coap_config.get_tt;

	smcp_t gSMCPInstance = smcp_create(((ThingMLCOAPContext*) thingML_context)->port);

	gRet = ERRORCODE_INPROGRESS;
	require(send_get_request(gSMCPInstance, NULL, 0, thingML_context), bail);


	while(ERRORCODE_INPROGRESS == gRet) {
		smcp_wait(gSMCPInstance,1000);
		smcp_process(gSMCPInstance);
	}

bail:
	smcp_transaction_end(gSMCPInstance,&transaction);

	if(gSMCPInstance)
		smcp_release(gSMCPInstance);

	return gRet;
}
