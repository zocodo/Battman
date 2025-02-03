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

#if __has_include(<IOKit/ps/IOPowerSources.h>)
#include <IOKit/ps/IOPowerSources.h>
#else
CFArrayRef IOPSCopyPowerSourcesList(CFTypeRef blob);
CFDictionaryRef IOPSGetPowerSourceDescription(CFTypeRef blob, CFTypeRef ps);
#endif

#if __has_include(<IOKit/ps/IOPowerSourcesPrivate.h>)
#include <IOKit/ps/IOPowerSourcesPrivate.h>
#else
CFTypeRef IOPSCopyPowerSourcesByType(int type);

enum {
    kIOPSSourceAll = 0,
    kIOPSSourceInternal,
    kIOPSSourceUPS,
    kIOPSSourceInternalAndUPS,
    kIOPSSourceForAccessories
};
#endif

#if __has_include(<IOKit/ps/IOPSKeys.h>)
#include <IOKit/ps/IOPSKeys.h>
#else
/* Implemented Keys (Documented) */
#define kIOPSIsPresentKey               "Is Present"
#define kIOPSIsFinishingChargeKey       "Is Finishing Charge"
#define kIOPSPowerSourceStateKey        "Power Source State"
#define kIOPSCurrentKey                 "Current"
#define kIOPSMaxCapacityKey             "Max Capacity"
#define kIOPSCurrentCapacityKey         "Current Capacity"
#define kIOPSBatteryHealthConditionKey  "BatteryHealthCondition"
#define kIOPSIsChargingKey              "Is Charging"
#define kIOPSHardwareSerialNumberKey    "Hardware Serial Number"
#define kIOPSTransportTypeKey           "Transport Type"
#define kIOPSTimeToEmptyKey             "Time to Empty"
#define kIOPSNameKey                    "Name"
#define kIOPSTypeKey                    "Type"
#define kIOPSBatteryHealthKey           "BatteryHealth"
#define kIOPSPowerSourceIDKey           "Power Source ID"

/* Unimplemented Keys (At least not in Simulator) */
#define kIOPSDesignCapacityKey          "DesignCapacity"
#define kIOPSTemperatureKey             "Temperature"

#define kIOPSInternalBatteryType        "InternalBattery"
#endif

/* Implemented Keys (Private) */
#define kIOPSDesignCycleCountKey                "DesignCycleCount"
#define kIOPSBatteryProvidesTimeRemainingKey    "Battery Provides Time Remaining"
#define kIOPSOptimizedBatteryChargingEngagedKey "Optimized Battery Charging Engaged"

// Internal IDs:
// They are intended to be here, not in headers

// You are free to change the IDs, as long as they do not collapse
typedef enum {
    ID_BI_BATTERY_HEALTH = 1,
    ID_BI_BATTERY_SOC,
    ID_BI_BATTERY_TEMP,
    ID_BI_BATTERY_CHARGING,

    // Can be omitted in production
    ID_BI_BATTERY_ALWAYS_FALSE,
} id_bi_t;

// Templates:
// They are arrays, not linked lists
// They are here for generating linked lists.

#if 0
/* This is not compiled, but needed for Gettext PO template generation */
NSString *registeredStrings[] = {
    _("Health"),        /* Battery Health */
    _("SoC"),           /* State of Charge */
    _("Temperature"),   /* Temperature */
    _("Charging"),      /* Charging */
};
#endif


struct battery_info_node main_battery_template[] = {
	{"Health",      ID_BI_BATTERY_HEALTH,   (void*)(BIN_IS_BACKGROUND)},
	{"SoC",         ID_BI_BATTERY_SOC,      (void*)(BIN_IS_FOREGROUND)},
	{"Temperature", ID_BI_BATTERY_TEMP,     (void*)(BIN_IS_VALUE)},
	{"Charging",    ID_BI_BATTERY_CHARGING, (void*)(BIN_IS_TRUE_OR_FALSE)},

	{"TEST FALSE YOU SHOULD NOT SEE THIS!!", ID_BI_BATTERY_ALWAYS_FALSE, (void*)(BIN_IS_TRUE_OR_FALSE)},
	{NULL} // DO NOT DELETE
};

struct battery_info_node *bi_construct_linked_list(struct battery_info_node *template)
{
	struct battery_info_node *ret_head = NULL;
	struct battery_info_node *tail = NULL;

    for (struct battery_info_node *i = template; i->description; i++) {
		struct battery_info_node *current = malloc(sizeof(struct battery_info_node));
		current->description = i->description;
		current->identifier = i->identifier;
		current->content = i->content;
		current->prev = tail;
		if (tail) {
			tail->next = current;
		} else {
			ret_head = current;
		}
		tail = current;
	}
	if (tail)
		tail->next = NULL;

    return ret_head;
}

bool bi_find_next(struct battery_info_node **v, int identifier)
{
	struct battery_info_node *beginning = *v;
	for (struct battery_info_node *i = beginning; i != NULL; i = i->next) {
		if (i->identifier == identifier) {
			*v = i;
			return true;
		}
	}
	for (struct battery_info_node *i = beginning; i != NULL; i = i->prev) {
		if (i->identifier == identifier) {
			*v = i;
			return true;
		}
	}
	return true;
}

#warning Why not float?
void bi_node_change_content_value(struct battery_info_node *node, unsigned int value)
{
	assert(value <= 127);
	node->content = (void*)(
		// Drop lower bits
		( ((uint64_t)node->content) & (((uint64_t)-1) << 7) ) |
		// Attach value
		value
	);
}

struct battery_info_node *battery_info_init()
{
	struct battery_info_node *info = bi_construct_linked_list(main_battery_template);
	battery_info_update(info);
	return info;
}

#define MAKE_PERCENTAGE(a,b) (int)((float)a * 100.0 / (float)b)

void battery_info_update(struct battery_info_node *head)
{
	CFTypeRef powersources = IOPSCopyPowerSourcesByType(kIOPSSourceInternal);
	CFArrayRef pslist = IOPSCopyPowerSourcesList(powersources);
	CFIndex pscnt = CFArrayGetCount(pslist);

	for (int i = 0; i < pscnt; i++) {
		CFTypeRef cursrc = CFArrayGetValueAtIndex(pslist, i);
        /* IOPMPowerSource was not that accurate, consider AppleSMC */
		CFTypeRef desc = IOPSGetPowerSourceDescription(powersources, cursrc);
		if (CFStringCompare((CFStringRef)CFDictionaryGetValue(desc, CFSTR(kIOPSTypeKey)), CFSTR(kIOPSInternalBatteryType),0) == kCFCompareEqualTo) {
			CFNumberRef curCapacity = (CFNumberRef)CFDictionaryGetValue(desc, CFSTR(kIOPSCurrentCapacityKey));
			CFNumberRef maxCapacity = (CFNumberRef)CFDictionaryGetValue(desc, CFSTR(kIOPSMaxCapacityKey));
#if !TARGET_OS_SIMULATOR
            CFNumberRef designCapacity = (CFNumberRef)CFDictionaryGetValue(desc, CFSTR(kIOPSDesignCapacityKey));
#endif

			int cc, mc, dc;
			CFNumberGetValue(curCapacity, kCFNumberIntType, &cc);
			CFNumberGetValue(maxCapacity, kCFNumberIntType, &mc);
#if TARGET_OS_SIMULATOR
            dc = 90;
#else
            CFNumberGetValue(designCapacity, kCFNumberIntType, &dc);
#endif
            // idk how to get battery health :(
			CFBooleanRef isCharging = (CFBooleanRef)CFDictionaryGetValue(desc, CFSTR(kIOPSIsChargingKey));
			
			if (bi_find_next(&head, ID_BI_BATTERY_HEALTH)) {
				bi_node_change_content_value(head, dc);
			}
			if (bi_find_next(&head, ID_BI_BATTERY_SOC)) {
				//bi_node_change_content_value(head, (int)((float)dc * (float)cc / (float)mc));
                bi_node_change_content_value(head, cc);
			}
			if (bi_find_next(&head, ID_BI_BATTERY_TEMP)) {
				bi_node_change_content_value(head, 32);
			}
			if (bi_find_next(&head, ID_BI_BATTERY_CHARGING)) {
				bi_node_change_content_value(head, CFBooleanGetValue(isCharging));
			}
			if (bi_find_next(&head, ID_BI_BATTERY_ALWAYS_FALSE)) {
				bi_node_change_content_value(head, 0);
			}
			break;
		}
	}
	CFRelease(pslist);
	CFRelease(powersources);
}
