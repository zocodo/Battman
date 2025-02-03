#include "battery_info.h"
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>

// Internal IDs:
// They are intended to be here, not in headers

// You are free to change the IDs, as long as they do not collapse
#define ID_BI_BATTERY_HEALTH 1
#define ID_BI_BATTERY_SOC 2
#define ID_BI_BATTERY_TEMP 3
#define ID_BI_BATTERY_CHARGING 4
// Can be omitted in production
#define ID_BI_BATTERY_ALWAYS_FALSE 5

// Templates:
// They are arrays, not linked lists
// They are here for generating linked lists.


    // _("Health"):	        Battery Capacity/Battery Health
    // _("SoC"):            State of Charge
    // _("Temperature"):    Temperature
    // _("Charging"):       Charging


struct battery_info_node main_battery_template[]={
	{"Health", ID_BI_BATTERY_HEALTH, (void*)(BIN_IS_BACKGROUND)},
	{"SoC", ID_BI_BATTERY_SOC, (void*)(BIN_IS_FOREGROUND)},
	{"Temperature", ID_BI_BATTERY_TEMP, (void*)(BIN_IS_VALUE)},
	{"Charing", ID_BI_BATTERY_CHARGING, (void*)BIN_IS_TRUE_OR_FALSE},
	{"TEST FALSE YOU SHOULD NOT SEE THIS!!", ID_BI_BATTERY_ALWAYS_FALSE, (void*)(BIN_IS_TRUE_OR_FALSE)},
	{NULL} // DO NOT DELETE
};

struct battery_info_node *bi_construct_linked_list(struct battery_info_node *template) {
	struct battery_info_node *ret_head=NULL;
	struct battery_info_node *tail=NULL;
	for(struct battery_info_node *i=template;i->description;i++) {
		struct battery_info_node *current=malloc(sizeof(struct battery_info_node));
		current->description=i->description;
		current->identifier=i->identifier;
		current->content=i->content;
		current->prev=tail;
		if(tail) {
			tail->next=current;
		}else{
			ret_head=current;
		}
		tail=current;
	}
	if(tail)
		tail->next=NULL;
	return ret_head;
}

int bi_find_next(struct battery_info_node **v, int identifier) {
	struct battery_info_node *beginning=*v;
	for(struct battery_info_node *i=beginning;i!=NULL;i=i->next) {
		if(i->identifier==identifier) {
			*v=i;
			return 1;
		}
	}
	for(struct battery_info_node *i=beginning;i!=NULL;i=i->prev) {
		if(i->identifier==identifier) {
			*v=i;
			return 1;
		}
	}
	return 0;
}

void bi_node_change_content_value(struct battery_info_node *node, unsigned int value) {
	assert(value<=127);
	node->content=(void*)(
		// Drop lower bits
		( ((uint64_t)node->content) & (((uint64_t)-1)<<7) ) |
		// Attach value
		value
	);
}

struct battery_info_node *battery_info_init() {
	struct battery_info_node *info=bi_construct_linked_list(main_battery_template);
	battery_info_update(info);
	return info;
}

#define MAKE_PERCENTAGE(a,b) (int)((float)a*100.0/(float)b)

CFTypeRef IOPSCopyPowerSourcesByType(int);
CFArrayRef IOPSCopyPowerSourcesList(CFTypeRef);
CFDictionaryRef IOPSGetPowerSourceDescription(CFTypeRef,CFTypeRef);

void battery_info_update(struct battery_info_node *head) {
	CFTypeRef powersources=IOPSCopyPowerSourcesByType(1);
	CFArrayRef pslist=IOPSCopyPowerSourcesList(powersources);
	int pscnt=CFArrayGetCount(pslist);
	for(int i=0;i<pscnt;i++) {
		CFTypeRef cursrc=CFArrayGetValueAtIndex(pslist,i);
		CFTypeRef desc=IOPSGetPowerSourceDescription(powersources, cursrc);
		if(CFStringCompare((CFStringRef)CFDictionaryGetValue(desc, CFSTR("Type")), CFSTR("InternalBattery"),0)==kCFCompareEqualTo) {
			CFNumberRef curCapacity=(CFNumberRef)CFDictionaryGetValue(desc, CFSTR("Current Capacity"));
			CFNumberRef maxCapacity=(CFNumberRef)CFDictionaryGetValue(desc, CFSTR("Max Capacity"));
			CFNumberRef designCapacity=(CFNumberRef)CFDictionaryGetValue(desc, CFSTR("Maximum Capacity Percent"));
			long cc;
			long mc;
			long dc;
			CFNumberGetValue(curCapacity, kCFNumberLongType, &cc);
			CFNumberGetValue(maxCapacity, kCFNumberLongType, &mc);
			CFNumberGetValue(designCapacity, kCFNumberLongType, &dc);
			// idk how to get battery health :(
			CFBooleanRef isCharging=(CFBooleanRef)CFDictionaryGetValue(desc,CFSTR("Is Charging"));
			
			if(bi_find_next(&head, ID_BI_BATTERY_HEALTH)) {
				bi_node_change_content_value(head, dc);
			}
			if(bi_find_next(&head, ID_BI_BATTERY_SOC)) {
				bi_node_change_content_value(head, (int)((float)dc*(float)cc/(float)mc));
			}
			if(bi_find_next(&head, ID_BI_BATTERY_TEMP)) {
				bi_node_change_content_value(head, 32);
			}
			if(bi_find_next(&head, ID_BI_BATTERY_CHARGING)) {
				bi_node_change_content_value(head, CFBooleanGetValue(isCharging));
			}
			if(bi_find_next(&head, ID_BI_BATTERY_ALWAYS_FALSE)) {
				bi_node_change_content_value(head, 0);
			}
			break;
		}
	}
	CFRelease(pslist);
	CFRelease(powersources);
}