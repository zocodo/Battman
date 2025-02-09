#pragma once
#import <UIKit/UIKit.h>
#include "battery_utils/libsmc.h"

@interface BatteryDetailsViewController : UITableViewController {
    /* Even they comes from same source with gauge, they need extra calculations */
    uint16_t b_full_charge_capacity;
    uint16_t b_remaining_capacity;
    uint16_t b_designed_capacity;

    gas_gauge_t gauge;
}
@end
