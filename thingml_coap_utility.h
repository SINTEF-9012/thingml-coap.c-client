/*
 * ThingMLMQTTUtility.h
 *
 *  Created on: Jul 13, 2015
 *      Author: vassik
 */

#ifndef THINGML_COAP_UTILITY_H_
#define THINGML_COAP_UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pthingMLCOAPClient)(void* _instance, ...);

typedef struct {
	pthingMLCOAPClient fn_onerror_callback;
	pthingMLCOAPClient fn_onmsgrcv_callback;
	const char* url;
	void* thing_instance;

} ThingMLCOAPContext;

#ifdef __cplusplus
}
#endif

#endif /* THINGML_COAP_UTILITY_H_ */
