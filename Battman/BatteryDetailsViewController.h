#pragma once
#import <UIKit/UIKit.h>
#import "BatterySubscriberViewControllerBase.h"
#include "battery_utils/battery_info.h"

@interface BatteryDetailsViewController : BatterySubscriberViewControllerBase {
    struct battery_info_node *batteryInfo;
    unsigned char pendingLoadOffsets[64];
    NSString *gasGaugeDisclaimer;
    NSString *adapterDisclaimer;
}
- (instancetype)initWithBatteryInfo:(struct battery_info_node *)bi;
@end
