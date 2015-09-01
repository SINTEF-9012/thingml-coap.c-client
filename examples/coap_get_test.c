/*
 * coap_get_test.c
 *
 *  Created on: Aug 31, 2015
 *      Author: vassik
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include "../thingml_coap_utility.h"
#include "../cmd_get.h"


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

	context->thing_instance = NULL;
	context->url = resource_url;
	context->port = port;
	context->fn_onerror_callback = coap_error_callback;
	context->fn_onmsgrcv_callback = coap_message_recieved_callback;
	context->client_coap_config = coap_client_config_default;

	tool_cmd_get_url((void*) context);

	free(context);
}



