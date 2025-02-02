#import "BatteryInfoViewController.h"
#import "BatteryCellView/BatteryInfoTableViewCell.h"

@implementation BatteryInfoViewController

- (NSString *)title {
	return @"Battman";
}

- (instancetype)init {
	UITabBarItem *tabbarItem=[UITabBarItem new];
	tabbarItem.title=@"Battery";
	tabbarItem.image=[UIImage systemImageNamed:@"battery.100"];
	tabbarItem.tag = 0;
	self.tabBarItem=tabbarItem;
	return [super initWithStyle:UITableViewStyleGrouped]; // or grouped if desired
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return 1;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (NSString *)tableView:(id)tv titleForFooterInSection:(NSInteger)section {
	return @"2025 Ⓒ Torrekie <me@torrekie.dev>";
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	BatteryInfoTableViewCell *cell=[[BatteryInfoTableViewCell alloc] initWithFrame:CGRectMake(0,0,1000,100)];
	[cell.batteryCell updateForegroundPercentage:50];
	[cell.batteryCell updateBackgroundPercentage:80];
	cell.batteryLabel.text=@"Battery Capacity: 80%\nCharge: 50%\nTest: 0%";
	return cell;
}

- (CGFloat)tableView:(id)tv heightForRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.row==0) {
		return 130;
	}
	return 30;
}

@end
