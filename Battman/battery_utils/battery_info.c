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
	if(!node->content)
		return;
	vm_deallocate(mach_task_self(), (vm_address_t)bi_node_get_string(node), 256);
	node->content=0;
}

struct battery_info_node *battery_info_init() {
    struct battery_info_node *info = bi_construct_array();
    battery_info_update(info, false);
    return info;
}

static int _impl_set_item_find_item(struct battery_info_node **head, const char *desc) {
	if(!desc)
		return 0;
	for(struct battery_info_node *i=*head;i->description;i++) {
		if(i->description==desc) {
			*head=i;
			return 1;
		}
	}
	for(struct battery_info_node *i=(*head)-1;i!=head[1];i--) {
		if(i->description==desc) {
			*head=i;
			return 1;
		}
	}
	return 0;
}

static char *_impl_set_item(struct battery_info_node **head, const char *desc, uint64_t value, float valueAsFloat, int options) {
	if(!_impl_set_item_find_item(head,desc))
		return NULL;
	struct battery_info_node *i=*head;
	if(options==2||(i->content&BIN_IS_SPECIAL)==0) {
		if(options==1) {
			if(value&&i->content) {
				bi_node_free_string(i);
			}
			return NULL;
		}
		return bi_node_ensure_string(i, 0, 256);
	}else if(options==1){
		bi_node_set_hidden(i, 0, (bool)value);
	}else if((i->content&BIN_IS_FLOAT)==BIN_IS_FLOAT) {
		bi_node_change_content_value_float(i, 0, valueAsFloat);
		return NULL;
	}else{
		bi_node_change_content_value(i, 0, (uint16_t)value);
	}
	return NULL;
}

#define BI_SET_ITEM(name, value) _impl_set_item(head_arr, name, (uint64_t)(value), (float)(value), 0)
#define BI_ENSURE_STR(name) _impl_set_item(head_arr, name, 0, 0, 2)
#define BI_FORMAT_ITEM(name, ...) sprintf(_impl_set_item(head_arr, name, 0, 0, 2), __VA_ARGS__)
#define BI_SET_ITEM_IF(cond, name, value) if(cond){BI_SET_ITEM(name,value);_impl_set_item(head_arr, name, 0, 0, 1);}else{_impl_set_item(head_arr, name, 1, 0, 1);}
#define BI_FORMAT_ITEM_IF(cond, name, ...) if(cond){BI_FORMAT_ITEM(name,__VA_ARGS__);}else{_impl_set_item(head_arr, name, 1, 0, 1);}

void battery_info_update(struct battery_info_node *head, bool inDetail) {
    uint16_t remain_cap, full_cap, design_cap;
    get_capacity(&remain_cap, &full_cap, &design_cap);
    
    struct battery_info_node *head_arr[2]={head,head};
    	/* Health = 100.0f * FullChargeCapacity (mAh) / DesignCapacity (mAh) */
	BI_SET_ITEM(_ID_("Health"), 100.0f * (float)full_cap / (float)design_cap);
	/* SoC = 100.0f * RemainCapacity (mAh) / FullChargeCapacity (mAh) */
	BI_SET_ITEM(_ID_("SoC"), 100.0f * (float)remain_cap / (float)full_cap);
	// No Imperial units here
	BI_SET_ITEM(_ID_("Temperature"), get_temperature());
	// // TODO: Charging Type Display {"Battery Power", "AC Power", "UPS Power"}
	BI_SET_ITEM(_ID_("Charging"), (get_time_to_empty() == 0));
	/* ASoC = 100.0f * RemainCapacity (mAh) / DesignCapacity (mAh) */
	BI_SET_ITEM("ASoC(Hidden)", 100.0f * remain_cap / design_cap);
	if (inDetail) {
		get_gas_gauge(&gGauge);
		BI_FORMAT_ITEM_IF(*gGauge.DeviceName,
					_ID_("Battery Name"),
					"%s", gGauge.DeviceName);
		BI_SET_ITEM(_ID_("Full Charge Capacity"), full_cap);
		BI_SET_ITEM(_ID_("Designed Capacity"), design_cap);
		BI_SET_ITEM(_ID_("Remaining Capacity"), remain_cap);
		BI_SET_ITEM(_ID_("Qmax"), gGauge.Qmax * battery_num());
		BI_SET_ITEM(_ID_("Depth of Discharge"), gGauge.DOD0);
		BI_SET_ITEM(_ID_("Passed Charge"), gGauge.PassedCharge);
		BI_SET_ITEM(_ID_("Voltage"), gGauge.Voltage);
		BI_SET_ITEM(_ID_("Average Current"), gGauge.AverageCurrent);
		BI_SET_ITEM(_ID_("Average Power"), gGauge.AveragePower);
		BI_SET_ITEM(_ID_("Battery Count"), battery_num());
		/* FIXME: TTE shall display "Never" when -1 */
		int timeToEmpty = get_time_to_empty();
		BI_SET_ITEM_IF(timeToEmpty>0, _ID_("Time To Empty"), timeToEmpty);
		BI_SET_ITEM(_ID_("Cycle Count"), gGauge.CycleCount);
		BI_SET_ITEM(_ID_("State Of Charge"), gGauge.StateOfCharge);
		BI_SET_ITEM_IF(gGauge.ResScale, 
				_ID_("Resistance Scale"), gGauge.ResScale);
		if(!battery_serial(BI_ENSURE_STR(_ID_("Battery Serial")))) {
			BI_FORMAT_ITEM(_ID_("Battery Serial"), "None");
		}
		BI_FORMAT_ITEM(_ID_("Chemistry ID"), "0x%.8X", gGauge.ChemID);
		BI_FORMAT_ITEM(_ID_("Flags"), "0x%.4X", gGauge.Flags);
		BI_SET_ITEM_IF(gGauge.TrueRemainingCapacity,
				_ID_("True Remaining Capacity"),
				gGauge.TrueRemainingCapacity);
		BI_SET_ITEM_IF(gGauge.OCV_Current, _ID_("OCV Current"),
				gGauge.OCV_Current);
		BI_SET_ITEM_IF(gGauge.OCV_Voltage, _ID_("OCV Voltage"),
				gGauge.OCV_Voltage);
		BI_SET_ITEM_IF(gGauge.IMAX, _ID_("Peak Current"), gGauge.IMAX);
		BI_SET_ITEM_IF(gGauge.IMAX2, _ID_("Peak Current 2"), gGauge.IMAX2);
		BI_FORMAT_ITEM_IF(gGauge.ITMiscStatus, _ID_("IT Misc Status"), "0x%.4X",
					gGauge.ITMiscStatus);
		BI_SET_ITEM_IF(gGauge.SimRate, _ID_("Simulation Rate"),
				gGauge.SimRate);
        /* TODO: This id design sucks and bringing difficulties on maintainance,
                I want something just like:
                extern int insert_item(char *label, ...);
                insert_item("Battery Name", gGauge.DeviceName);
                insert_item("ChemID", "0x%.8X", gGauge.ChemID);
         */
    }
}
