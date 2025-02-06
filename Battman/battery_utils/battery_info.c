#include "battery_info.h"
#include "libsmc.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFString.h>

#if 0
#warning TODO: IOKit/ps is not reliable, migrate to other impl
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
#define kIOPSIsPresentKey "Is Present"
#define kIOPSIsChargedKey "Is Charged" // Only appears when charged
#define kIOPSIsFinishingChargeKey "Is Finishing Charge"
#define kIOPSPowerSourceStateKey "Power Source State"
#define kIOPSMaxCapacityKey "Max Capacity"
#define kIOPSCurrentCapacityKey "Current Capacity"
#define kIOPSIsChargingKey "Is Charging"
#define kIOPSHardwareSerialNumberKey "Hardware Serial Number"
#define kIOPSTransportTypeKey "Transport Type"
#define kIOPSTimeToEmptyKey "Time to Empty"
#define kIOPSNameKey "Name"
#define kIOPSTypeKey "Type"
#define kIOPSPowerSourceIDKey "Power Source ID"

/* Implemented Keys (Real device only) */

/* Implemented Keys (Simulator / Mac only) */
#define kIOPSBatteryHealthKey "BatteryHealth"
#define kIOPSCurrentKey "Current"
#define kIOPSBatteryHealthConditionKey "BatteryHealthCondition"

/* Unimplemented Keys */
#define kIOPSDesignCapacityKey "DesignCapacity"
#define kIOPSTemperatureKey "Temperature"

#define kIOPSInternalBatteryType "InternalBattery"
#endif

#if __has_include(<IOKit/ps/IOPSKeysPrivate.h>)
#include <IOKit/ps/IOPSKeysPrivate.h>
#else
/* Implemented Keys */
#define kIOPSBatteryProvidesTimeRemainingKey "Battery Provides Time Remaining"
#define kIOPSOptimizedBatteryChargingEngagedKey                                \
    "Optimized Battery Charging Engaged"

/* Implemented Keys (Real device only) */
#define kIOPSRawExternalConnectivityKey "Raw External Connected"
#define kIOPSShowChargingUIKey "Show Charging UI"
#define kIOPSPlayChargingChimeKey "Play Charging Chime"

/* Implemented Keys (Simulator / Mac only) */
#define kIOPSDesignCycleCountKey "DesignCycleCount"
#endif
#endif

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

    ID_BI_BATTERY_SOC_PER_HEALTH
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
    {"Health", ID_BI_BATTERY_HEALTH, BIN_IS_BACKGROUND | BIN_UNIT_PERCENT},
    {"SoC", ID_BI_BATTERY_SOC, BIN_IS_FLOAT | BIN_UNIT_PERCENT},
    {"Temperature", ID_BI_BATTERY_TEMP, BIN_IS_FLOAT | BIN_UNIT_DEGREE_C},
    {"Charging", ID_BI_BATTERY_CHARGING, BIN_IS_BOOLEAN},
    {"SoC/Health(Hidden)", ID_BI_BATTERY_SOC_PER_HEALTH,
     BIN_IS_FOREGROUND | BIN_IS_HIDDEN},

    {"TEST FALSE YOU SHOULD NOT SEE THIS!!", ID_BI_BATTERY_ALWAYS_FALSE,
     BIN_IS_BOOLEAN},
    {NULL} // DO NOT DELETE
};

struct battery_info_node *
bi_construct_linked_list(struct battery_info_node *template) {
    struct battery_info_node *ret_head = NULL;
    struct battery_info_node *tail = NULL;

    for (struct battery_info_node *i = template; i->description; i++) {
        struct battery_info_node *current =
            malloc(sizeof(struct battery_info_node));
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

bool bi_find_next(struct battery_info_node **v, int identifier) {
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

void bi_node_change_content_value(struct battery_info_node *node,
                                  unsigned int value) {
    uint32_t *sects = (uint32_t *)&node->content;
    sects[1] = value;
}

void bi_node_change_content_value_float(struct battery_info_node *node,
                                        float value) {
    assert((node->content & BIN_IS_FLOAT) == BIN_IS_FLOAT);
    float *sects = (float *)&node->content;
    sects[1] = value;
    // overwrite higher bits;
}

struct battery_info_node *battery_info_init() {
    struct battery_info_node *info =
        bi_construct_linked_list(main_battery_template);
    battery_info_update(info);
    return info;
}

void battery_info_update(struct battery_info_node *head) {
    uint16_t bdata[3];
    get_capacity(bdata, bdata + 1, bdata + 2);
    if (bi_find_next(&head, ID_BI_BATTERY_HEALTH)) {
        bi_node_change_content_value_float(head, 100.0 * (float)bdata[1] /
                                                     (float)bdata[2]);
    }
    if (bi_find_next(&head, ID_BI_BATTERY_SOC)) {
        bi_node_change_content_value_float(head, 100.0 * (float)*bdata /
                                                     (float)bdata[1]);
    }
    if (bi_find_next(&head, ID_BI_BATTERY_TEMP)) {
        // In Celsius, if you don't use Celsius, go learn it or PR to support your unit
        bi_node_change_content_value_float(head, get_temperature());
    }
    if (bi_find_next(&head, ID_BI_BATTERY_CHARGING)) {
        // TODO: Changing Type Display {"Battery Power", "AC Power", "UPS Power"}
        bi_node_change_content_value(head, (get_time_to_empty() == 0));
    }
    if (bi_find_next(&head, ID_BI_BATTERY_SOC_PER_HEALTH)) {
        bi_node_change_content_value_float(head, 100.0 * (float)bdata[0] /
                                                     (float)bdata[2]);
    }
    if (bi_find_next(&head, ID_BI_BATTERY_ALWAYS_FALSE)) {
        bi_node_change_content_value(head, 0);
    }
}
