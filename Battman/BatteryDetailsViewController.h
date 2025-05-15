#pragma once
#import <UIKit/UIKit.h>
#import "BatterySubscriberViewControllerBase.h"
#include "battery_utils/battery_info.h"

@interface BatteryDetailsViewController : BatterySubscriberViewControllerBase {
    struct battery_info_node *batteryInfoStruct;
    struct battery_info_node *batteryInfo[BI_SECTION_NUM];
    unsigned char *pendingLoadOffsets[BI_SECTION_NUM];
}
- (instancetype)initWithBatteryInfo:(struct battery_info_node *)bi;
@end
