#pragma once
#import <UIKit/UIKit.h>

@interface BatterySubscriberViewControllerBase : UITableViewController
{
	id observerToUnsubscribe;
}
- (void)batteryStatusDidUpdate;
- (void)batteryStatusDidUpdate:(NSDictionary *)info;
@end
