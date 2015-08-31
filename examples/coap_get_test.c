/*
 * coap_get_test.c
 *
 *  Created on: Aug 31, 2015
 *      Author: vassik
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "smcp/smcp.h"


#include "../thingml_coap_utility.h"
#include "../cmd_get.h"


static smcp_t gSMCPInstance;

void coap_error_callback(void* _instance, ...) {
	int result;

	va_list valist;
	va_start(valist, _instance);

	result = va_arg(valist, int);

	va_end(valist);

	printf("coap_error_callback is called, error code -> %d\n", result);
}

void coap_message_recieved_callback(void* _instance, ...) {
	char * message;
	int message_size;

	va_list valist;
	va_start(valist, _instance);

	message = va_arg(valist, char*);
	message_size = va_arg(valist, int);

	va_end(valist);

	printf("message size %d, and message '%s' \n", message_size, message);

}


int main(int argc, char* argv[]) {

	uint16_t port = 5683;
	const char * resource_url = "coap://localhost:5683/outside_temperature";
	ThingMLCOAPContext* context = malloc(sizeof(ThingMLCOAPContext));

	gSMCPInstance = smcp_create(port);

	context->thing_instance = NULL;
	context->url = resource_url;
	context->fn_onerror_callback = coap_error_callback;
	context->fn_onmsgrcv_callback = coap_message_recieved_callback;

	tool_cmd_get_url(gSMCPInstance, (void*) context);

	if(gSMCPInstance)
		smcp_release(gSMCPInstance);

}



