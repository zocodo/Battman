#import "BatteryInfoViewController.h"
#import "BatteryCellView/BatteryInfoTableViewCell.h"
#include "battery_utils/battery_utils.h"
#include "common.h"

@implementation BatteryInfoViewController

- (NSString *)title {
	return _("Battman");
}

- (instancetype)init {
	UITabBarItem *tabbarItem = [UITabBarItem new];
	tabbarItem.title = _("Battery");
	tabbarItem.image =  [UIImage systemImageNamed:@"battery.100"];
	tabbarItem.tag = 0;
	self.tabBarItem = tabbarItem;
	return [super initWithStyle:UITableViewStyleGrouped]; // or grouped if desired
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return 1;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (NSString *)tableView:(id)tv titleForFooterInSection:(NSInteger)section {
	return _("2025 Ⓒ Torrekie <me@torrekie.dev>");
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	// TODO:
	// Call underlying functions [at battery_info.c:void battery_info_update]
	// NOTE that if more than one cell presents
	// if(indexPath.[row or section]) is NEEDED
	
	BatteryInfoTableViewCell *cell = [[BatteryInfoTableViewCell alloc] initWithFrame:CGRectMake(0, 0, 1000, 100)];

	cell.batteryInfo = battery_info_init();
	// battery_info_update shall be called within cell impl.
	[cell updateBatteryInfo];
	return cell;
}

- (CGFloat)tableView:(id)tv heightForRowAtIndexPath:(NSIndexPath *)indexPath {
	if (indexPath.row == 0) {
		return 130;
	}
	return 30;
}

@end
