#import "BatteryInfoViewController.h"
#import "BatteryCellView/BatteryInfoTableViewCell.h"
#import "BatteryDetailsViewController.h"
#include "battery_utils/battery_utils.h"
#include "common.h"

static NSMutableArray *sections_batteryinfo;

@implementation BatteryInfoViewController

- (NSString *)title {
    return _("Battman");
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Copyright text
    UILabel *copyright;
    copyright = [[UILabel alloc] init];
    copyright.text = _("2025 Ⓒ Torrekie <me@torrekie.dev>");
    copyright.font = [UIFont systemFontOfSize:12];
    copyright.textAlignment = NSTextAlignmentCenter;
    copyright.textColor = [UIColor grayColor];
    [copyright sizeToFit];
    self.tableView.tableFooterView = copyright;
}

- (instancetype)init {
    UITabBarItem *tabbarItem = [UITabBarItem new];
    tabbarItem.title = _("Battery");
    tabbarItem.image = [UIImage systemImageNamed:@"battery.100"];
    tabbarItem.tag = 0;
    self.tabBarItem = tabbarItem;
    batteryInfo = battery_info_init();

    sections_batteryinfo = [[NSMutableArray alloc] initWithArray:@[_("Battery Info"), _("Manage")]];
    
    return [super initWithStyle:UITableViewStyleGrouped];
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
    return 1;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
    return sections_batteryinfo.count;
}

- (NSString *)tableView:(id)t titleForHeaderInSection:(NSInteger)sect {
    return sections_batteryinfo[sect];
}

- (NSString *)tableView:(id)tv titleForFooterInSection:(NSInteger)section {
    return nil;
}

- (void)tableView:(UITableView *)tv
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == [sections_batteryinfo indexOfObject:_("Battery Info")])
        [self.navigationController
            pushViewController:[[BatteryDetailsViewController alloc] initWithBatteryInfo:batteryInfo]
                      animated:YES];

    [tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == [sections_batteryinfo indexOfObject:_("Battery Info")]) {
        BatteryInfoTableViewCell *cell = [[BatteryInfoTableViewCell alloc]
            initWithFrame:CGRectMake(0, 0, 1000, 100)];

        cell.batteryInfo = batteryInfo;
        // battery_info_update shall be called within cell impl.
        [cell updateBatteryInfo];
        return cell;
    } else if (indexPath.section == [sections_batteryinfo indexOfObject:_("Manage")]) {
        UITableViewCell *cell = [UITableViewCell new];
        cell.textLabel.text = _("Charging Limit");
        cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
        return cell;
    }

    return nil;
}

- (CGFloat)tableView:(id)tv heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == [sections_batteryinfo indexOfObject:_("Battery Info")] && indexPath.row == 0) {
        return 130;
    } else {
        return [super tableView:tv heightForRowAtIndexPath:indexPath];
        // return 30;
    }
}

- (void)dealloc {
	for (struct battery_info_node *i = batteryInfo; i->description; i++) {
		if (i->content && !(i->content & BIN_IS_SPECIAL)) {
			bi_node_free_string(i);
		}
	}
	free(batteryInfo);
}

@end
