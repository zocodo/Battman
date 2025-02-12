#pragma once
#import <UIKit/UIKit.h>
#include "battery_utils/battery_info.h"

@interface BatteryDetailsViewController : UITableViewController {
	struct battery_info_node *batteryInfo;
}
- (instancetype)initWithBatteryInfo:(struct battery_info_node *)bi;
@end
