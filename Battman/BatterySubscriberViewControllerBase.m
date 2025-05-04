#import "BatterySubscriberViewControllerBase.h"

@implementation BatterySubscriberViewControllerBase

- (void)batteryStatusDidUpdate {
	[self.tableView reloadData];
}

- (void)viewDidDisappear:(BOOL)a {
	[super viewDidDisappear:a];
	[[NSNotificationCenter defaultCenter] removeObserver:observerToUnsubscribe];
}

- (void)viewDidAppear:(BOOL)a {
	[super viewDidAppear:a];
	observerToUnsubscribe = [[NSNotificationCenter defaultCenter] addObserverForName:@"SMC60000" object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *n){
		[self batteryStatusDidUpdate];
	}];
}

@end
