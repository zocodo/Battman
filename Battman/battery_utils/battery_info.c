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

#ifndef _ID_
// TODO:
#define _ID_(x) (x)
#endif

// Internal IDs:
// They are intended to be here, not in headers

// Add IDs to the end, MUST match the struct template.
typedef enum {
    ID_BI_BATTERY_NAME = 0,
    ID_BI_BATTERY_HEALTH,
    ID_BI_BATTERY_SOC,
    ID_BI_BATTERY_TEMP,
    ID_BI_BATTERY_CHARGING,
    ID_BI_BATTERY_ASOC
} id_bi_t;

const char *bin_unit_strings[]={
	_ID_("℃"),
	"%",
	_ID_("mA"),
	_ID_("mAh"),
	_ID_("mV"),
	_ID_("mW"),
	_ID_("min"),
	_ID_("Hr") // Do not modify, thats how Texas Instruments documented
};

struct battery_info_node main_battery_template[] = {
    {_ID_("Battery Name"), 0},
    {_ID_("Health"), BIN_IS_BACKGROUND | BIN_UNIT_PERCENT | BIN_SECTION},
    {_ID_("SoC"), BIN_IS_FLOAT | BIN_UNIT_PERCENT},
    {_ID_("Temperature"),
     BIN_IS_FLOAT | BIN_UNIT_DEGREE_C | BIN_DETAILS_SHARED},
    {_ID_("Charging"), BIN_IS_BOOLEAN},
    {"ASoC(Hidden)", BIN_IS_FOREGROUND | BIN_IS_HIDDEN},
    {_ID_("Full Charge Capacity"), BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_ID_("Designed Capacity"), BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_ID_("Remaining Capacity"), BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_ID_("Qmax"), BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_ID_("Depth of Discharge"), BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_ID_("Passed Charge"), BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_ID_("Voltage"), BIN_UNIT_MVOLT | BIN_IN_DETAILS},
    {_ID_("Average Current"), BIN_UNIT_MAMP | BIN_IN_DETAILS},
    {_ID_("Average Power"), BIN_UNIT_MWATT | BIN_IN_DETAILS},
    {_ID_("Battery Count"), BIN_IN_DETAILS},
    {_ID_("Time To Empty"), BIN_UNIT_MIN | BIN_IN_DETAILS},
    {_ID_("Cycle Count"), BIN_IN_DETAILS},
    {_ID_("State Of Charge"), BIN_UNIT_PERCENT | BIN_IN_DETAILS},
    {_ID_("Resistance Scale"), BIN_IN_DETAILS},
    {_ID_("Battery Serial"), 0},
    {_ID_("Chemistry ID"), 0},
    {_ID_("Flags"), 0},
    {_ID_("True Remaining Capacity"), BIN_UNIT_MAH | BIN_IN_DETAILS},
    {_ID_("OCV Current"), BIN_UNIT_MAMP | BIN_IN_DETAILS},
    {_ID_("OCV Voltage"), BIN_UNIT_MVOLT | BIN_IN_DETAILS},
    {_ID_("Peak Current"), BIN_UNIT_MAMP | BIN_IN_DETAILS},
    {_ID_("Peak Current 2"), BIN_UNIT_MAMP | BIN_IN_DETAILS},
    {_ID_("IT Misc Status"), 0},
    {_ID_("Simulation Rate"), BIN_UNIT_HOUR | BIN_IN_DETAILS},
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
    uint32_t* vptr = (uint32_t *)&value;
    uint32_t vr = *vptr;
    // TODO: No magic numbers!
    node->content = (
                     (vr & ((uint64_t)0b11 << 30)) |
                     (vr & (((1 << 4) - 1) << 23)) << 3 |
                     (vr & (((1 << 10) - 1) << 13)) << 3
    ) | (node->content & ((1 << 16) - 1));
    // overwrite higher bits;
}

float bi_node_load_float(struct battery_info_node *node) {
	float ret;
	uint32_t *vptr = (uint32_t *)&ret;
	uint32_t vr = node->content;
	*vptr = (
             (vr & ((uint64_t)0b11 << 30)) |
             (vr & (((1 << 4) - 1) << 26)) >> 3 |
             (vr & (((1 << 10) - 1) << 16)) >> 3
    );
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
        void *allocen=(void*)0x10000000;
        // ^ Preferred addr
        // Use vm_allocate to prevent possible unexpected heap allocation (it crashes in current data structure)
        // TODO: get rid of hardcoded length
        int result = vm_allocate(mach_task_self(), (vm_address_t *)&allocen, 256, VM_FLAGS_ANYWHERE);
        if (result != KERN_SUCCESS) {
            // Fallback to malloc
            //allocen = malloc(length);
            allocen=nil;
        }
        node->content = (uint32_t)(((uint64_t)allocen) >> 3);
    }
    return bi_node_get_string(node);
}

char *bi_node_get_string(struct battery_info_node *node) {
	return (char *)(((uint64_t)node->content) << 3);
}

void bi_node_free_string(struct battery_info_node *node) {
	vm_deallocate(mach_task_self(), (vm_address_t)bi_node_get_string(node), 256);
}

struct battery_info_node *battery_info_init() {
    struct battery_info_node *info = bi_construct_array();
    battery_info_update(info, false);
    return info;
}

void battery_info_update(struct battery_info_node *head, bool inDetail) {
    uint16_t remain_cap, full_cap, design_cap;
    remain_cap = full_cap = design_cap = 0; // Init before use
    get_capacity(&remain_cap, &full_cap, &design_cap);

    /* Health = 100.0f * FullChargeCapacity (mAh) / DesignCapacity (mAh) */
    bi_node_change_content_value_float(head, ID_BI_BATTERY_HEALTH,
                                       100.0f * (float)full_cap / (float)design_cap);

    /* SoC = 100.0f * RemainCapacity (mAh) / FullChargeCapacity (mAh) */
    bi_node_change_content_value_float(head, ID_BI_BATTERY_SOC,
                                       100.0f * (float)remain_cap / (float)full_cap);

    /* In Celsius, if you don't use Celsius, go learn it or PR to support your
     * unit */
    bi_node_change_content_value_float(head, ID_BI_BATTERY_TEMP,
                                       get_temperature());

    // TODO: Charging Type Display {"Battery Power", "AC Power", "UPS Power"}
    bi_node_change_content_value(head, ID_BI_BATTERY_CHARGING,
                                 (get_time_to_empty() == 0));

    /* ASoC = 100.0f * RemainCapacity (mAh) / DesignCapacity (mAh) */
    bi_node_change_content_value_float(head, ID_BI_BATTERY_ASOC,
                                       100.0f * remain_cap / design_cap);

    if (inDetail) {
        get_gas_gauge(&gGauge);
        if (strlen(gGauge.DeviceName) != 0) {
            sprintf(bi_node_ensure_string(head, ID_BI_BATTERY_NAME, 32), "%s", gGauge.DeviceName);
        }
        /* TODO: This id design sucks and bringing difficulties on maintainance,
                I want something just like:
                extern int insert_item(char *label, ...);
                insert_item("Battery Name", gGauge.DeviceName);
                insert_item("ChemID", "0x%.8X", gGauge.ChemID);
         */
        bi_node_change_content_value(head, 6, full_cap);
        bi_node_change_content_value(head, 7, design_cap);
        bi_node_change_content_value(head, 8, remain_cap);
        bi_node_change_content_value(head, 9, gGauge.Qmax * battery_num());
        bi_node_change_content_value(head, 10, gGauge.DOD0);
        bi_node_change_content_value(head, 11, gGauge.PassedCharge);
        bi_node_change_content_value(head, 12, gGauge.Voltage);
        bi_node_change_content_value(head, 13, gGauge.AverageCurrent);
        bi_node_change_content_value(head, 14, gGauge.AveragePower);
        bi_node_change_content_value(head, 15, battery_num());
        /* FIXME: TTE shall display "Never" when -1 */
        int timeToEmpty = get_time_to_empty();
        if (timeToEmpty && timeToEmpty != -1) {
            bi_node_set_hidden(head, 16, false);
            bi_node_change_content_value(head, 16, timeToEmpty);
        } else {
            bi_node_set_hidden(head, 16, true);
        }
        bi_node_change_content_value(head, 17, gGauge.CycleCount);
        bi_node_change_content_value(head, 18, gGauge.StateOfCharge);
        if (gGauge.ResScale) {
            bi_node_change_content_value(head, 19, gGauge.ResScale);
            bi_node_set_hidden(head, 19, false);
        } else {
            bi_node_set_hidden(head, 19, true);
        }
        /* We can't make sure if someone's battery have any serial */
        if (!battery_serial(bi_node_ensure_string(head, 20, 21))) {
            sprintf(bi_node_ensure_string(head, 20, 4), "None");
        }
        sprintf(bi_node_ensure_string(head, 21, 12), "0x%.8X", gGauge.ChemID);
        sprintf(bi_node_ensure_string(head, 22, 8), "0x%.4X", gGauge.Flags);
        if (gGauge.TrueRemainingCapacity) {
            bi_node_change_content_value(head, 23, gGauge.TrueRemainingCapacity);
            bi_node_set_hidden(head, 23, false);
        } else {
            bi_node_set_hidden(head, 23, true);
        }
        if (gGauge.OCV_Current) {
            bi_node_change_content_value(head, 24, gGauge.OCV_Current);
            bi_node_set_hidden(head, 24, false);
        } else {
            bi_node_set_hidden(head, 24, true);
        }
        if (gGauge.OCV_Voltage) {
            bi_node_change_content_value(head, 25, gGauge.OCV_Voltage);
            bi_node_set_hidden(head, 25, false);
        } else {
            bi_node_set_hidden(head, 25, true);
        }
        if (gGauge.IMAX) {
            bi_node_change_content_value(head, 26, gGauge.IMAX);
            bi_node_set_hidden(head, 26, false);
        } else {
            bi_node_set_hidden(head, 26, true);
        }
        if (gGauge.IMAX2) {
            bi_node_change_content_value(head, 27, gGauge.IMAX2);
            bi_node_set_hidden(head, 27, false);
        } else {
            bi_node_set_hidden(head, 27, true);
        }

        if (gGauge.ITMiscStatus) {
            sprintf(bi_node_ensure_string(head, 28, 8), "0x%.4X",
                    gGauge.ITMiscStatus);
            bi_node_change_content_value(head, 29, gGauge.SimRate);
            bi_node_set_hidden(head, 29, false);
        } else {
            bi_node_set_hidden(head, 29, true);
        }
    }
}
