#import "BatteryInfoViewController.h"

@implementation BatteryInfoViewController

- (instancetype)init {
	UITabBarItem *tabbarItem=[UITabBarItem new];
	tabbarItem.title=@"Battery";
	tabbarItem.image=[UIImage systemImageNamed:@"battery.100"];
	tabbarItem.tag = 0;
	self.tabBarItem=tabbarItem;
	return [super initWithStyle:UITableViewStylePlain]; // or grouped if desired
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return 1;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	UITableViewCell *batteryChargeCell=[UITableViewCell new];
	batteryChargeCell.textLabel.text=@"Test";
	return batteryChargeCell;
}

@end