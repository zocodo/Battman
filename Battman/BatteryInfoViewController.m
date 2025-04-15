#import "BatteryInfoViewController.h"
#import "BatteryCellView/BatteryInfoTableViewCell.h"
#import "BatteryCellView/TemperatureInfoTableViewCell.h"
#import "BatteryDetailsViewController.h"
#include "battery_utils/battery_utils.h"
#include "license_check.h"

#include "common.h"

// TODO: UI Refreshing

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
    NSString *me = _("2025 Ⓒ Torrekie <me@torrekie.dev>");
#ifdef DEBUG
    /* FIXME: GIT_COMMIT_HASH should be a macro */
    copyright.text = [NSString stringWithFormat:@"%@\n%@ %@\n%s %s\n%@", me, _("Debug Build"), [[NSBundle mainBundle] objectForInfoDictionaryKey:@"GIT_COMMIT_HASH"], __DATE__, __TIME__, _("Redistribution Prohibited")];
    copyright.numberOfLines = 0;
#else
    copyright.text = me;
#endif

    /* FIXME: Containered is not Sandboxed, try some extra checks */
    char *home = getenv("HOME");
    if (match_regex(home, IOS_CONTAINER_FMT) || match_regex(home, MAC_CONTAINER_FMT)) {
        copyright.text = [copyright.text stringByAppendingFormat:@"\n%@", _("Sandboxed")];
    } else if (match_regex(home, SIM_CONTAINER_FMT)) {
        copyright.text = [copyright.text stringByAppendingFormat:@"\n%@", _("Simulator Sandboxed")];
    } else {
        copyright.text = [copyright.text stringByAppendingFormat:@"\n%@", _("Unsandboxed")];
    }

    copyright.font = [UIFont systemFontOfSize:12];
    copyright.textAlignment = NSTextAlignmentCenter;
    copyright.textColor = [UIColor grayColor];
    [copyright sizeToFit];
    self.tableView.tableFooterView = copyright;
}

- (instancetype)init {
    extern bool checked_license;
    if (!checked_license || !has_accepted_terms()) {
#if defined(__arm64__) || defined(__aarch64__) || defined(__arm64e__)
        __asm__ volatile(
            "mov x30, xzr\nmov x29,xzr"
        );
#elif defined(__x86_64__)
        // Are we really going to support X86 in future?
        __asm__ volatile(
            "xor %%rbp, %%rbp\n"
        );
#endif
#if LICENSE == LICENSE_NONFREE
        /* TODO: We will need a more secure check for this */
#endif
    }
    UITabBarItem *tabbarItem = [UITabBarItem new];
    tabbarItem.title = _("Battery");
    tabbarItem.image = [UIImage systemImageNamed:@"battery.100"];
    tabbarItem.tag = 0;
    self.tabBarItem = tabbarItem;
    batteryInfo = battery_info_init();

    sections_batteryinfo = [[NSMutableArray alloc] initWithArray:@[_("Battery Info"), _("Hardware Temperatures"), _("Manage")]];
    
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
    } else if (indexPath.section == [sections_batteryinfo indexOfObject:_("Hardware Temperatures")]) {
        TemperatureInfoTableViewCell *cell = [[TemperatureInfoTableViewCell alloc] initWithFrame:CGRectMake(0, 0, 1000, 100)];
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
    } else if (indexPath.section == [sections_batteryinfo indexOfObject:_("Hardware Temperatures")] && indexPath.row == 0) {
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
