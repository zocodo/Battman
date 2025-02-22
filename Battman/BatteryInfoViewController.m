#import "BatteryInfoViewController.h"
#import "BatteryCellView/BatteryInfoTableViewCell.h"
#import "BatteryDetailsViewController.h"
#include "battery_utils/battery_utils.h"
#include "common.h"

@implementation BatteryInfoViewController

- (NSString *)title {
    return _("Battman");
}

- (instancetype)init {
    UITabBarItem *tabbarItem = [UITabBarItem new];
    tabbarItem.title = _("Battery");
    tabbarItem.image = [UIImage systemImageNamed:@"battery.100"];
    tabbarItem.tag = 0;
    self.tabBarItem = tabbarItem;
    batteryInfo = battery_info_init();
    return
        [super initWithStyle:UITableViewStyleGrouped];
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
    return 1;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
    return 2;
}

- (NSString *)tableView:(id)t titleForHeaderInSection:(NSInteger)sect {
    if (sect == 0)
        return _("Battery Info");
    else if (sect == 1)
        return _("Manage");
    return nil;
}

- (NSString *)tableView:(id)tv titleForFooterInSection:(NSInteger)section {
    if (section == 1)
        return _("2025 Ⓒ Torrekie <me@torrekie.dev>");
    return nil;
}

- (void)tableView:(UITableView *)tv
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == 0)
        [self.navigationController
            pushViewController:[[BatteryDetailsViewController alloc] initWithBatteryInfo:batteryInfo]
                      animated:YES];
    [tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == 0) {
        BatteryInfoTableViewCell *cell = [[BatteryInfoTableViewCell alloc]
            initWithFrame:CGRectMake(0, 0, 1000, 100)];

        cell.batteryInfo = batteryInfo;
        // battery_info_update shall be called within cell impl.
        [cell updateBatteryInfo];
        return cell;
    } else {
        UITableViewCell *cell = [UITableViewCell new];
        cell.textLabel.text = _("Charging Limit");
        cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
        return cell;
    }
}

- (CGFloat)tableView:(id)tv heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == 0 && indexPath.row == 0) {
        return 130;
    } else {
        return [super tableView:tv heightForRowAtIndexPath:indexPath];
        // return 30;
    }
}

- (void)dealloc {
	for(struct battery_info_node *i=batteryInfo;i->description;i++) {
		if(i->content&&!(i->content&BIN_IS_SPECIAL)) {
			bi_node_free_string(i);
		}
	}
	free(batteryInfo);
}

@end
