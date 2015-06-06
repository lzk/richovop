#ifndef __NETDEVICE_H__
#define __NETDEVICE_H__
#pragma once
#include <stdlib.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>
#include <avahi-common/llist.h>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

//Protocol limitations: you have to open device before do any r/w or get device id ops,
//                      And close it to release system resource.

//
//For VOP uses interfaces start here
//
int connetToNetDevice(char* devURI);
int get_device_id(char *buffer, size_t bufsize);
int wirteToNetDevice(void *buffer, int len);
int readFromNetDevice(void *buffer, int len);
void closeSocket(void);

//
//For backend uses interface start here
//



int mDNScnt;
int mDNSType[1024]; //0: ipv4 1:ipv6
int sockfd;
void *addr[1024]; //store resolved socket_in pointers
void* m_hAvahi_common,*m_hAvahi_client;

/** Create a new main loop object */
AvahiSimplePoll * (*pf_avahi_simple_poll_new)( void);
AvahiClient * (*pf_avahi_client_new)(const AvahiPoll *poll_api,AvahiClientFlags flags,AvahiClientCallback callback,void *userdata,int *error);

/** Create a new address resolver object from an AvahiAddress object */

AvahiAddressResolver * (*pf_avahi_address_resolver_new)(AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const AvahiAddress *a,
    AvahiLookupFlags flags,
    AvahiAddressResolverCallback callback,
    void *userdata);

/** Convert the specified address *a to a human readable character string, use AVAHI_ADDRESS_STR_MAX to allocate an array of the right size */
char * (*pf_avahi_address_snprint)(char *ret_s, size_t length, const AvahiAddress *a);

/** Free a AvahiAddressResolver resolver object */
int (*pf_avahi_address_resolver_free)(AvahiAddressResolver *r);

/** Request that the main loop quits. If this is called the next
 call to avahi_simple_poll_iterate() will return 1 */
void (*pf_avahi_simple_poll_quit)(AvahiSimplePoll *s);

/** Free a client instance. This will automatically free all
 * associated browser, resolve and entry group objects. All pointers
 * to such objects become invalid! */
void (*pf_avahi_client_free)(AvahiClient *client);


/** Free a main loop object */
void (*pf_avahi_simple_poll_free)(AvahiSimplePoll *s);


/** Return a human readable error string for the specified error code */
const char * (*pf_avahi_strerror)(int error);



/** Get the last error number. See avahi_strerror() for converting this error code into a human readable string. */
int (*pf_avahi_client_errno)(AvahiClient*);



/** Create a new service resolver object. Please make sure to pass all
 * the service data you received via avahi_service_browser_new()'s
 * callback function, especially interface and protocol. The protocol
 * argument specifies the protocol (IPv4 or IPv6) to use as transport
 * for the queries which are sent out by this resolver. The
 * aprotocol argument specifies the adress family (IPv4 or IPv6) of
 * the address of the service we are looking for. Generally, on
 * "protocol" you should only pass what was supplied to you as
 * parameter to your AvahiServiceBrowserCallback. In "aprotocol" you
 * should pass what your application code can deal with when
 * connecting to the service. Or, more technically speaking: protocol
 * specifies if the mDNS queries should be sent as UDP/IPv4
 * resp. UDP/IPv6 packets. aprotocol specifies whether the query is for a A
 * resp. AAAA resource record. */
AvahiServiceResolver * (* pf_avahi_service_resolver_new)(
    AvahiClient *client,
    AvahiIfIndex interface,   /**< Pass the interface argument you received in AvahiServiceBrowserCallback here. */
    AvahiProtocol protocol,   /**< Pass the protocol argument you received in AvahiServiceBrowserCallback here. */
    const char *name,         /**< Pass the name argument you received in AvahiServiceBrowserCallback here. */
    const char *type,         /**< Pass the type argument you received in AvahiServiceBrowserCallback here. */
    const char *domain,       /**< Pass the domain argument you received in AvahiServiceBrowserCallback here. */
    AvahiProtocol aprotocol,  /**< The desired address family of the service address to resolve. AVAHI_PROTO_UNSPEC if your application can deal with both IPv4 and IPv6 */
    AvahiLookupFlags flags,
    AvahiServiceResolverCallback callback,
    void *userdata);



/** Free a service resolver object */
int (*pf_avahi_service_resolver_free)(AvahiServiceResolver *r);


/** Get the parent client of an AvahiServiceResolver object */
AvahiClient* (*pf_avahi_service_resolver_get_client)(AvahiServiceResolver *);


/** Call avahi_simple_poll_iterate() in a loop and return if it returns non-zero */
int (*pf_avahi_simple_poll_loop)(AvahiSimplePoll *s);

/** Return the abstracted poll API object for this main loop
 * object. The is will return the same pointer each time it is
 * called. */
const AvahiPoll* (*pf_avahi_simple_poll_get)(AvahiSimplePoll *s);



#endif //__NETDEVICE_H__
