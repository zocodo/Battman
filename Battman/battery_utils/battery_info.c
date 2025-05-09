#include "battery_info.h"
#include "libsmc.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFString.h>

// Internal IDs:
// They are intended to be here, not in headers

// Add IDs to the end, MUST match the struct template.
typedef enum {
    ID_BI_BATTERY_NAME = 0,
    ID_BI_BATTERY_HEALTH,
    ID_BI_BATTERY_SOC,
    ID_BI_BATTERY_TEMP,
    ID_BI_BATTERY_CHARGING,
    ID_BI_BATTERY_ASOC,
} id_bi_t;

#ifdef _C
#undef _C
#endif
// Stub _C definition for gettext, locales are caller processed
#define _C(x) x
const char *bin_unit_strings[] = {
    _C("℃"), _C("%"), _C("mA"), _C("mAh"), _C("mV"), _C("mW"), _C("min"),
    _C("Hr") // Do not modify, thats how Texas Instruments documented
};

struct battery_info_node main_battery_template[] = {
    {_C("Device Name"),
     _C("This indicates the name of the current Gas Gauge IC used by the "
        "installed battery."),
     0},
    {_C("Health"), NULL, BIN_IS_BACKGROUND | BIN_UNIT_PERCENT | BIN_SECTION},
    {_C("SoC"), NULL, BIN_IS_FLOAT | BIN_UNIT_PERCENT},
    {_C("Avg. Temperature"), NULL,
     BIN_IS_FLOAT | BIN_UNIT_DEGREE_C | BIN_DETAILS_SHARED},
    {_C("Charging"), NULL, BIN_IS_BOOLEAN},
    {"ASoC(Hidden)", NULL, BIN_IS_FOREGROUND | BIN_IS_HIDDEN},
    {_C("Full Charge Capacity"), NULL, BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_C("Designed Capacity"), NULL, BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_C("Remaining Capacity"), NULL, BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_C("Battery Uptime"),
     _C("The length of time the Battery Management System (BMS) has been up."),
     BIN_UNIT_MIN | BIN_IN_DETAILS},
    {_C("Qmax"), NULL, BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_C("Depth of Discharge"),
     _C("Current chemical depth of discharge (DOD₀). The gas gauge updates "
        "information on the DOD₀ based on open-circuit voltage (OCV) readings "
        "when in a relaxed state."),
     BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_C("Passed Charge"),
     _C("The cumulative capacity of the current charging or discharging cycle. "
        "It is reset to zero with each DOD₀ update."),
     BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_C("Voltage"), NULL, BIN_UNIT_MVOLT | BIN_IN_DETAILS},
    {_C("Avg. Current"), NULL, BIN_UNIT_MAMP | BIN_IN_DETAILS},
    {_C("Avg. Power"), NULL, BIN_UNIT_MWATT | BIN_IN_DETAILS},
    {_C("Cell Count"), NULL, BIN_IN_DETAILS},
    /* TODO: TimeToFull */
    {_C("Time To Empty"), NULL, BIN_UNIT_MIN | BIN_IN_DETAILS},
    {_C("Cycle Count"), NULL, BIN_IN_DETAILS},
    {_C("Designed Cycle Count"), NULL, BIN_IN_DETAILS},
    {_C("State Of Charge"), NULL, BIN_UNIT_PERCENT | BIN_IN_DETAILS},
    {_C("State Of Charge (UI)"),
     _C("The \"Battery Percentage\" displayed exactly on your status bar. This "
        "is the SoC that Apple wants to tell you."),
     BIN_UNIT_PERCENT | BIN_IN_DETAILS},
    {_C("Resistance Scale"), NULL, BIN_IN_DETAILS},
    {_C("Battery Serial No."), NULL, 0},
    {_C("Chemistry ID"),
     _C("Chemistry unique identifier (ChemID) assigned to each battery in "
        "Texas Instruments' database. It ensures accurate calculations and "
        "predictions."),
     0},
    {_C("Flags"), NULL, 0},
    {_C("True Remaining Capacity"), NULL, BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_C("OCV Current"), NULL, BIN_UNIT_MAMP | BIN_IN_DETAILS},
    {_C("OCV Voltage"), NULL, BIN_UNIT_MVOLT | BIN_IN_DETAILS},
    {_C("Max Load Current"), NULL, BIN_UNIT_MAMP | BIN_IN_DETAILS},
    {_C("Max Load Current 2"), NULL, BIN_UNIT_MAMP | BIN_IN_DETAILS},
    {_C("IT Misc Status"),
     _C("This field refers to the miscellaneous data returned by battery "
        "Impedance Track™ Gas Gauge IC."),
     0},
    {_C("Simulation Rate"),
     _C("This field refers to the rate of Gas Gauge performing Impedance "
        "Track™ simulations."),
     BIN_UNIT_HOUR | BIN_IN_DETAILS},
    {_C("Daily Max SoC"), NULL, BIN_UNIT_PERCENT | BIN_IN_DETAILS},
    {_C("Daily Min SoC"), NULL, BIN_UNIT_PERCENT | BIN_IN_DETAILS},
    {NULL} // DO NOT DELETE
};

struct battery_info_node *bi_construct_array(void) {
    struct battery_info_node *val = malloc(sizeof(main_battery_template));
    memcpy(val, main_battery_template, sizeof(main_battery_template));
    return val;
}

void bi_node_change_content_value(struct battery_info_node *node,
                                  int identifier, unsigned short value) {
    node += identifier;
    uint16_t *sects = (uint16_t *)&node->content;
    sects[1] = value;
}

void bi_node_change_content_value_float(struct battery_info_node *node,
                                        int identifier, float value) {
    node += identifier;
    assert((node->content & BIN_IS_FLOAT) == BIN_IS_FLOAT);
    uint32_t *vptr = (uint32_t *)&value;
    uint32_t vr = *vptr;
    // TODO: No magic numbers!
    node->content =
        ((vr & ((uint64_t)0b11 << 30)) | (vr & (((1 << 4) - 1) << 23)) << 3 |
         (vr & (((1 << 10) - 1) << 13)) << 3) |
        (node->content & ((1 << 16) - 1));
    // overwrite higher bits;
}

float bi_node_load_float(struct battery_info_node *node) {
    float ret;
    uint32_t *vptr = (uint32_t *)&ret;
    uint32_t vr = node->content;
    *vptr =
        ((vr & ((uint64_t)0b11 << 30)) | (vr & (((1 << 4) - 1) << 26)) >> 3 |
         (vr & (((1 << 10) - 1) << 16)) >> 3);
    return ret;
}

void bi_node_set_hidden(struct battery_info_node *node, int identifier,
                        bool hidden) {
    node += identifier;
    assert((node->content & BIN_IN_DETAILS) == BIN_IN_DETAILS);
    if (hidden) {
        node->content |= (1 << 5);
    } else {
        node->content &= ~(1L << 5);
    }
}

#include <mach/mach.h>

char *bi_node_ensure_string(struct battery_info_node *node, int identifier,
                            uint64_t length) {
    node += identifier;
    assert(!(node->content & BIN_IS_SPECIAL));

    if (!node->content) {
        void *allocen = (void *)0x10000000;
        // ^ Preferred addr
        // Use vm_allocate to prevent possible unexpected heap allocation (it
        // crashes in current data structure)
        // TODO: get rid of hardcoded length
        int result = vm_allocate(mach_task_self(), (vm_address_t *)&allocen,
                                 256, VM_FLAGS_ANYWHERE);
        if (result != KERN_SUCCESS) {
            // Fallback to malloc
            // allocen = malloc(length);
            allocen = nil;
        }
        node->content = (uint32_t)(((uint64_t)allocen) >> 3);
    }
    return bi_node_get_string(node);
}

char *bi_node_get_string(struct battery_info_node *node) {
    return (char *)(((uint64_t)node->content) << 3);
}

void bi_node_free_string(struct battery_info_node *node) {
    if (!node->content)
        return;
    vm_deallocate(mach_task_self(), (vm_address_t)bi_node_get_string(node),
                  256);
    node->content = 0;
}

struct battery_info_node *battery_info_init() {
    struct battery_info_node *info = bi_construct_array();
    battery_info_update(info, false);
    return info;
}

static int _impl_set_item_find_item(struct battery_info_node **head,
                                    const char *desc) {
    if (!desc)
        return 0;
    for (struct battery_info_node *i = *head; i->name; i++) {
        if (i->name == desc) {
            *head = i;
            return 1;
        }
    }
    for (struct battery_info_node *i = (*head) - 1; i != head[1] - 1; i--) {
        if (i->name == desc) {
            *head = i;
            return 1;
        }
    }
    return 0;
}

static char *_impl_set_item(struct battery_info_node **head, const char *desc,
                            uint64_t value, float valueAsFloat, int options) {
    if (!_impl_set_item_find_item(head, desc))
        return NULL;
    struct battery_info_node *i = *head;
    if (options == 2 || (i->content & BIN_IS_SPECIAL) == 0) {
        if (options == 1) {
            if (value && i->content) {
                bi_node_free_string(i);
            }
            return NULL;
        }
        return bi_node_ensure_string(i, 0, 256);
    } else if (options == 1) {
        bi_node_set_hidden(i, 0, (bool)value);
    } else if ((i->content & BIN_IS_FLOAT) == BIN_IS_FLOAT) {
        bi_node_change_content_value_float(i, 0, valueAsFloat);
        return NULL;
    } else {
        bi_node_change_content_value(i, 0, (uint16_t)value);
    }
    return NULL;
}

#define BI_SET_ITEM(name, value)                                               \
    _impl_set_item(head_arr, name, (uint64_t)(value), (float)(value), 0)
#define BI_ENSURE_STR(name) _impl_set_item(head_arr, name, 0, 0, 2)
#define BI_FORMAT_ITEM(name, ...)                                              \
    sprintf(_impl_set_item(head_arr, name, 0, 0, 2), __VA_ARGS__)
#define BI_SET_ITEM_IF(cond, name, value)                                      \
    if (cond) {                                                                \
        BI_SET_ITEM(name, value);                                              \
        _impl_set_item(head_arr, name, 0, 0, 1);                               \
    } else {                                                                   \
        _impl_set_item(head_arr, name, 1, 0, 1);                               \
    }
#define BI_FORMAT_ITEM_IF(cond, name, ...)                                     \
    if (cond) {                                                                \
        BI_FORMAT_ITEM(name, __VA_ARGS__);                                     \
    } else {                                                                   \
        _impl_set_item(head_arr, name, 1, 0, 1);                               \
    }

void battery_info_update(struct battery_info_node *head, bool inDetail) {
    uint16_t remain_cap, full_cap, design_cap;
    get_capacity(&remain_cap, &full_cap, &design_cap);

    struct battery_info_node *head_arr[2] = {head, head};
    /* Health = 100.0f * FullChargeCapacity (mAh) / DesignCapacity (mAh) */
    BI_SET_ITEM(_C("Health"), 100.0f * (float)full_cap / (float)design_cap);
    /* SoC = 100.0f * RemainCapacity (mAh) / FullChargeCapacity (mAh) */
    BI_SET_ITEM(_C("SoC"), 100.0f * (float)remain_cap / (float)full_cap);
    // No Imperial units here
    BI_SET_ITEM(_C("Avg. Temperature"), get_temperature());
    // // TODO: Charging Type Display {"Battery Power", "AC Power", "UPS Power"}
    BI_SET_ITEM(_C("Charging"), (is_charging(NULL, NULL) == kIsCharging));
    /* ASoC = 100.0f * RemainCapacity (mAh) / DesignCapacity (mAh) */
    BI_SET_ITEM("ASoC(Hidden)", 100.0f * remain_cap / design_cap);
    if (inDetail) {
        get_gas_gauge(&gGauge);
        BI_FORMAT_ITEM_IF(strlen(gGauge.DeviceName), _C("Device Name"), "%s",
                          gGauge.DeviceName);
        BI_SET_ITEM(_C("Full Charge Capacity"), full_cap);
        BI_SET_ITEM(_C("Designed Capacity"), design_cap);
        BI_SET_ITEM(_C("Remaining Capacity"), remain_cap);
        BI_SET_ITEM(_C("Battery Uptime"), gGauge.bmsUpTime / 60);
        BI_SET_ITEM(_C("Qmax"), gGauge.Qmax * batt_cell_num());
        BI_SET_ITEM(_C("Depth of Discharge"), gGauge.DOD0);
        BI_SET_ITEM(_C("Passed Charge"), gGauge.PassedCharge);
        BI_SET_ITEM(_C("Voltage"), gGauge.Voltage);
        BI_SET_ITEM(_C("Avg. Current"), gGauge.AverageCurrent);
        BI_SET_ITEM(_C("Avg. Power"), gGauge.AveragePower);
        BI_SET_ITEM(_C("Cell Count"), batt_cell_num());
        /* FIXME: TTE shall display "Never" when -1 */
        int timeToEmpty = get_time_to_empty();
        BI_SET_ITEM_IF(timeToEmpty > 0, _C("Time To Empty"), timeToEmpty);
        BI_SET_ITEM(_C("Cycle Count"), gGauge.CycleCount);
        BI_SET_ITEM_IF(gGauge.DesignCycleCount, _C("Designed Cycle Count"),
                       gGauge.DesignCycleCount)
        BI_SET_ITEM(_C("State Of Charge"), gGauge.StateOfCharge);
        BI_SET_ITEM(_C("State Of Charge (UI)"), gGauge.UISoC);
        BI_SET_ITEM_IF(gGauge.ResScale, _C("Resistance Scale"),
                       gGauge.ResScale);
        if (!battery_serial(BI_ENSURE_STR(_C("Battery Serial No.")))) {
            BI_FORMAT_ITEM(_C("Battery Serial No."), "None");
        }
        BI_FORMAT_ITEM("Chemistry ID", "0x%.8X", gGauge.ChemID);

        /* Confirmed Flags format */
        /* bq20z45*: Battery Status (0x16):
         * https://www.ti.com/lit/er/sluu313a/sluu313a.pdf */
        BI_FORMAT_ITEM(_C("Flags"), "0x%.4X", gGauge.Flags);
        BI_SET_ITEM_IF(gGauge.TrueRemainingCapacity,
                       _C("True Remaining Capacity"),
                       gGauge.TrueRemainingCapacity);
        BI_SET_ITEM_IF(gGauge.OCV_Current, _C("OCV Current"),
                       gGauge.OCV_Current);
        BI_SET_ITEM_IF(gGauge.OCV_Voltage, _C("OCV Voltage"),
                       gGauge.OCV_Voltage);
        BI_SET_ITEM_IF(gGauge.IMAX, _C("Max Load Current"), gGauge.IMAX);
        BI_SET_ITEM_IF(gGauge.IMAX2, _C("Max Load Current 2"), gGauge.IMAX2);
        BI_FORMAT_ITEM_IF(gGauge.ITMiscStatus, _C("IT Misc Status"), "0x%.4X",
                          gGauge.ITMiscStatus);
        BI_SET_ITEM_IF(gGauge.SimRate, _C("Simulation Rate"), gGauge.SimRate);
        BI_SET_ITEM_IF(gGauge.DailyMaxSoc, _C("Daily Max SoC"),
                       gGauge.DailyMaxSoc);
        BI_SET_ITEM_IF(gGauge.DailyMinSoc, _C("Daily Min SoC"),
                       gGauge.DailyMinSoc);
        /* TODO: This id design sucks and bringing difficulties on maintainance,
                I want something just like:
                extern int insert_item(char *label, ...);
                insert_item("Battery Name", gGauge.DeviceName);
                insert_item("ChemID", "0x%.8X", gGauge.ChemID);
         */
    }
}
