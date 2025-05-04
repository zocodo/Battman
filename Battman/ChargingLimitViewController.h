#pragma once
#import <UIKit/UIKit.h>

@interface ChargingLimitViewController : UITableViewController
{
	int daemon_pid;
	char *vals;
}
@end