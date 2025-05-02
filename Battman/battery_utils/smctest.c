#include "../common.h"
#include <CoreFoundation/CFNotificationCenter.h>
#include <CoreFoundation/CFString.h>

extern void regAppleSMCNotification(void(*)(void*,void*,int,void *));

static void smcCB(void* a,void *b,int c,void *d) {
	if(c==113&&(unsigned long long)d==0x60000)
		CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(),CFSTR("SMC60000"),NULL,NULL,1);
	//if(c==113) {
	//	char buf[128];
	//	sprintf(buf,"type: %d, subtype: %p",c,d);
	//	show_alert("notify",buf,"ok");
	//}
}

__attribute__((constructor)) static void runSMCTest() {
	regAppleSMCNotification(smcCB);
}