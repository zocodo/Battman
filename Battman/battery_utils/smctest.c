#include "../common.h"

extern void regAppleSMCNotification(void(*)(void*,void*,int,void *));

static void smcCB(void* a,void *b,int c,void *d) {
	//if(c==113) {
		char buf[128];
		sprintf(buf,"type: %d, subtype: %p",c,d);
		show_alert("notify",buf,"ok");
	//}
}

/*__attribute__((constructor)) static void runSMCTest() {
	regAppleSMCNotification(smcCB);
}*/