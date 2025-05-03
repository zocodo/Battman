#include <stdint.h>
#include <stdlib.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include "../common.h"

extern int IOServiceAddMatchingNotification(void *,const char *,void *,void*,void*,void**);
extern void *IOServiceMatching(const char *);
extern int IOServiceAddInterestNotification(void*,void*,const char*,void*,int,void*);
extern void *IOIteratorNext(void*);
extern void IOObjectRelease(void*);
extern int IORegistryEntryCreateCFProperties(void *,CFMutableDictionaryRef*,int,int);
extern void *IONotificationPortCreate(int);
extern void *dispatch_get_global_queue(int,int);
extern void IONotificationPortSetDispatchQueue(void*,void*);
extern void NSLog(CFStringRef,...);

static void stpe_cb(void **pcb, void *it) {
	if(!it)
		return;
	void *next;
	while((next=IOIteratorNext(it))) {
		void *buf;
		int err=IOServiceAddInterestNotification(*pcb,next,"IOGeneralInterest",pcb[1],0,(void*)&buf);
		if(err)
			abort();
		IOObjectRelease(next);
	}
}

void subscribeToPowerEvents(void (*cb)(int,void*,int32_t)) {
	void *port[]={IONotificationPortCreate(0),cb};
	IONotificationPortSetDispatchQueue(*port,dispatch_get_global_queue(0,0));
	void *nit=NULL;
	int err=IOServiceAddMatchingNotification(*port,"IOServiceFirstMatch",IOServiceMatching("IOPMPowerSource"),stpe_cb,port,&nit);
	if(err)
		abort();
	stpe_cb(port,nit);
	IOObjectRelease(nit);
}

void pmncb(int a,void *b,int32_t c) {
	if(c!=-536723200)
		return;
	CFMutableDictionaryRef props;
	IORegistryEntryCreateCFProperties(b,&props,0,0);
	CFStringRef desc=CFCopyDescription(props);
	CFRelease(props);
	//NSLog(CFSTR("Power Update: %@"),desc);
	show_alert("Power",CFStringGetCStringPtr(desc,0x08000100),"ok");
	CFRelease(desc);
}

//__attribute__((constructor)) static void startpmn() {
//	subscribeToPowerEvents(pmncb);
//}
