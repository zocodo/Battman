#pragma once
#import <UIKit/UIKit.h>

@interface BatteryDetailsViewController : UITableViewController {
    unsigned short b_remaining_capacity;
    unsigned short b_full_capacity;
    unsigned short b_designed_capacity;
}
@end