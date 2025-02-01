#import "SettingsViewController.h"

@implementation SettingsViewController

- (instancetype)init {
	UITabBarItem *tabbarItem=[[UITabBarItem alloc] initWithTabBarSystemItem:UITabBarSystemItemMore tag:1];
	tabbarItem.title=@"More";
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
	batteryChargeCell.textLabel.text=@"Test222";
	return batteryChargeCell;
}

@end