#pragma once
#import <UIKit/UIKit.h>

@interface ChargingManagementViewController : UITableViewController
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

@interface CoreDuet_someinterface : NSObject
+ (id)batterySaver;
- (int)setMode:(int)mode;
- (BOOL)setPowerMode:(int)mode error:(NSError **)error;
- (long long)getPowerMode;
@end
