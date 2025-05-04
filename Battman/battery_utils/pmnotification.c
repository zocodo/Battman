#include <stdint.h>
#include <stdlib.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include "../common.h"

#if __has_include(<IOKit/IOKitKeys.h>)
#include <IOKit/IOKitKeys.h>
#else
#define kIOFirstMatchNotification "IOServiceFirstMatch"
#define kIOGeneralInterest "IOGeneralInterest"
#endif

#if __has_include(<IOKit/IOKitLib.h>)
#include <IOKit/IOKitLib.h>
#else
/* Consider move those externs a single shared header */
typedef __darwin_mach_port_t mach_port_t;
typedef mach_port_t io_object_t;

typedef io_object_t io_iterator_t;
typedef io_object_t io_service_t;
typedef io_object_t io_registry_entry_t;

typedef struct IONotificationPort * IONotificationPortRef;

typedef void
(*IOServiceInterestCallback)(
    void *            refcon,
    io_service_t        service,
    uint32_t        messageType,
    void *            messageArgument );
typedef void (*IOServiceMatchingCallback)(void *refcon, io_iterator_t iterator);

extern int IOServiceAddMatchingNotification(void *, const char *, void *, void *, void *, io_iterator_t *);
extern void *IOServiceMatching(const char *);
extern int IOServiceAddInterestNotification(IONotificationPortRef, io_service_t, const char *, void *, int, void *);
extern io_object_t IOIteratorNext(io_iterator_t);
extern void IOObjectRelease(io_object_t);
extern int IORegistryEntryCreateCFProperties(io_registry_entry_t, CFMutableDictionaryRef *, int, int);
extern void *IONotificationPortCreate(int);
extern void IONotificationPortSetDispatchQueue(void *, void *);
#endif

#if __has_include(<dispatch/dispatch.h>)
#include <dispatch/dispatch.h>
#else
extern void *dispatch_get_global_queue(int, int);
#endif


extern void NSLog(CFStringRef, ...);

// IOServiceMatchingCallback
static void stpe_cb(void **pcb, io_iterator_t it) {
	if (!it)
		return;
	io_object_t next;
	while ((next = IOIteratorNext(it))) {
		void *buf;
		int err = IOServiceAddInterestNotification(*pcb, next, kIOGeneralInterest, (IOServiceInterestCallback)pcb[1], 0, (void *)&buf);
		if(err)
			abort();
		IOObjectRelease(next);
	}
}

void subscribeToPowerEvents(void (*cb)(int, io_registry_entry_t, int32_t)) {
	void *port[] = {IONotificationPortCreate(0), cb};
	IONotificationPortSetDispatchQueue(*port,dispatch_get_global_queue(0,0));
	io_iterator_t nit = 0;
	int err = IOServiceAddMatchingNotification(*port, kIOFirstMatchNotification, IOServiceMatching("IOPMPowerSource"), (IOServiceMatchingCallback)stpe_cb, port, &nit);
	if (err)
		abort();
	stpe_cb(port, nit);
	IOObjectRelease(nit);
}

void pmncb(int a, io_registry_entry_t b, int32_t c) {
	if (c != -536723200)
		return;
	CFMutableDictionaryRef props;
	IORegistryEntryCreateCFProperties(b, &props, 0, 0);
	CFStringRef desc = CFCopyDescription(props);
	CFRelease(props);
	//NSLog(CFSTR("Power Update: %@"),desc);
	show_alert("Power", CFStringGetCStringPtr(desc, kCFStringEncodingUTF8), "ok");
	CFRelease(desc);
}

//__attribute__((constructor)) static void startpmn() {
//	subscribeToPowerEvents(pmncb);
//}
