#pragma once
#import <UIKit/UIKit.h>

@interface BatteryDetailsViewController : UITableViewController {
    /* We need a better structure for this */
    uint16_t b_temperature;

    uint16_t b_average_voltage;
    uint16_t b_average_current;
    uint16_t b_average_power;

    uint16_t b_ocv_voltage;
    uint16_t b_ocv_current;

    uint16_t b_cycle_count;
    uint16_t b_flags;

    uint16_t b_state_of_charge;
    
    uint16_t b_remaining_capacity;
    uint16_t b_true_remaining_capacity;
    uint16_t b_full_charge_capacity;
    uint16_t b_designed_capacity;
    uint16_t b_qmax;

}
@end
