#pragma once
#import <UIKit/UIKit.h>
#include "battery_utils/libsmc.h"

@interface ChargingLimitViewController:UITableViewController
{
	UIDatePicker *fromPicker;
	UIDatePicker *untilPicker;
}
@end

@interface PowerUI_someinterface : NSObject
- (BOOL)setState:(NSUInteger)state error:(NSError **)err;
- (id)initWithClientName:(NSString *)name;
- (void)engageFrom:(NSDate *)f until:(NSDate*)u repeatUntil:(NSDate *)ru overrideAllSignals:(BOOL)oas;
@end