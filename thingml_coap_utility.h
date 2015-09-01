/*
 * ThingMLMQTTUtility.h
 *
 *  Created on: Jul 13, 2015
 *      Author: vassik
 */

#ifndef THINGML_COAP_UTILITY_H_
#define THINGML_COAP_UTILITY_H_

#include <smcp/coap.h>
#include <smcp/smcp.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	bool get_show_headers;
	coap_size_t next_len;
	int redirect_count;
	uint16_t size_request;
	bool get_observe;
	bool get_keep_alive;
	cms_t get_timeout;
	int last_observe_value;
	coap_content_type_t request_accept_type;
	bool observe_once;
	bool observe_ignore_first;
	coap_transaction_type_t get_tt;
} coap_client_config;

//typedef struct coap_client_config coap_client_config;

#define coap_client_config_default_m {false , ((coap_size_t)(-1)), 0, 0, false, false, 30*1000 /*Default timeout is 30 seconds.*/, -1, COAP_CONTENT_TYPE_UNKNOWN, false, false, COAP_TRANS_TYPE_CONFIRMABLE}

coap_client_config coap_client_config_default;

typedef void (*pthingMLCOAPClient)(void* _instance, ...);

typedef struct {
	pthingMLCOAPClient fn_onerror_callback;
	pthingMLCOAPClient fn_onmsgrcv_callback;
	const char* url;
	void* thing_instance;
	int port;
	coap_client_config client_coap_config;

} ThingMLCOAPContext;


#ifdef __cplusplus
}
#endif

#endif /* THINGML_COAP_UTILITY_H_ */
