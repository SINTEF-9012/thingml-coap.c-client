//
//  cmd_monitor.c
//  SMCP
//
//  Created by Robert Quattlebaum on 4/1/12.
//  Copyright (c) 2012 deepdarc. All rights reserved.
//

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
#include "help.h"
#include "cmd_monitor.h"
#include <string.h>
#include <smcp/url-helpers.h>
#include <math.h>
#include <signal.h>
#include "smcpctl.h"
#include <smcp/smcp-node-router.h>

static arg_list_item_t option_list[] = {
	{ 'h', "help",		 NULL,	 "Print Help"				 },
	{ 0 }
};

static int gRet;
static sig_t previous_sigint_handler;
static void
signal_interrupt(int sig) {
	gRet = ERRORCODE_INTERRUPT;
	signal(SIGINT, previous_sigint_handler);
}

static smcp_status_t
monitor_action_func(
	smcp_node_t node
) {
	char* content = (char*)smcp_inbound_get_content_ptr();
	coap_size_t content_length = smcp_inbound_get_content_len();
	fprintf(stdout,
		" *** Received Action! content_length=%d",
		    (int)content_length);
	fprintf(stdout,
		" content_type=\"%s\"",
		coap_content_type_to_cstr(smcp_inbound_get_content_type())
	);

	if(content_length)
		fprintf(stdout, " content=\"%s\"", content);

	fprintf(stdout, "\n");

	return SMCP_STATUS_OK;
}



int
tool_cmd_monitor(
	smcp_t smcp, int argc, char* argv[]
) {
	int i;
	char url[1000];
	coap_msg_id_t tid=0;

	static struct smcp_node_s monitor_node = {};
	if(!monitor_node.name) {
		smcp_node_init(
			(smcp_node_t)&monitor_node,
			smcp_get_root_node(smcp),
			"monitor"
		);
		monitor_node.request_handler = (void*)&monitor_action_func;
	}

	gRet = ERRORCODE_INPROGRESS;
	previous_sigint_handler = signal(SIGINT, &signal_interrupt);
	url[0] = 0;

	BEGIN_LONG_ARGUMENTS(gRet)
	HANDLE_LONG_ARGUMENT("help") {
		print_arg_list_help(option_list, argv[0], "[args] <uri>");
		gRet = ERRORCODE_HELP;
		goto bail;
	}
	BEGIN_SHORT_ARGUMENTS(gRet)
	HANDLE_SHORT_ARGUMENT2('h', '?') {
		print_arg_list_help(option_list, argv[0], "[args] <uri>");
		gRet = ERRORCODE_HELP;
		goto bail;
	}
	HANDLE_OTHER_ARGUMENT() {
		if(url[0] == 0) {
			if(getenv("SMCP_CURRENT_PATH")) {
				strncpy(url, getenv("SMCP_CURRENT_PATH"), sizeof(url));
				url_change(url, argv[i]);
			} else {
				strncpy(url, argv[i], sizeof(url));
			}
		} else {
			fprintf(stderr, "Unexpected extra argument: \"%s\"\n", argv[i]);
			gRet = ERRORCODE_BADARG;
			goto bail;
		}
	}
	END_ARGUMENTS

	if((url[0] == 0) && getenv("SMCP_CURRENT_PATH"))
		strncpy(url, getenv("SMCP_CURRENT_PATH"), sizeof(url));

	if(url[0] == 0) {
		fprintf(stderr, "Missing path argument.\n");
		gRet = ERRORCODE_BADARG;
		goto bail;
	}

//	require(send_list_request(smcp, url, NULL, 0), bail);
	gRet = ERRORCODE_OK;

	while(ERRORCODE_INPROGRESS == gRet) {
		smcp_wait(smcp,50);
		smcp_process(smcp);
	}

bail:
	smcp_invalidate_transaction_old(smcp, tid);
	signal(SIGINT, previous_sigint_handler);
	return gRet;
}
