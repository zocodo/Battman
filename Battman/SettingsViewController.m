#import "SettingsViewController.h"
#include "common.h"

@implementation SettingsViewController

- (NSString *)title {
	return _("More");
}

- (instancetype)init {
#warning UITabBarSystemItem cannot change title like this!
	UITabBarItem *tabbarItem = [[UITabBarItem alloc] initWithTabBarSystemItem:UITabBarSystemItemMore tag:1];
	tabbarItem.title = _("More");
	self.tabBarItem = tabbarItem;
	return [super initWithStyle:UITableViewStyleGrouped]; // or plain if desired
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return 2;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
	return _("About Battman");
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if (indexPath.row == 0) {
		[self.navigationController pushViewController:[CreditViewController new] animated:YES];
	} else if (indexPath.row == 1) {
		[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://github.com/Torrekie/Battman"] options:[NSDictionary new] completionHandler:nil];
	}
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	// TODO: REUSE (Too few cells to reuse for now so no need at this moment)
	if (indexPath.row == 0) {
		UITableViewCell *creditCell = [UITableViewCell new];
		creditCell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		creditCell.textLabel.text = _("Credit");
		return creditCell;
	} else if (indexPath.row == 1) {
		UITableViewCell *sourceCodeCell = [UITableViewCell new];
		sourceCodeCell.textLabel.text = _("Source Code");
		sourceCodeCell.textLabel.textColor = [UIColor colorWithRed:0 green:0.478 blue:1 alpha:1];
		return sourceCodeCell;
	}
	UITableViewCell *batteryChargeCell = [UITableViewCell new];
	batteryChargeCell.textLabel.text = @"Test222";
	return batteryChargeCell;
}

@end

@implementation CreditViewController

- (NSString *)title {
	return _("Credit");
}

- (instancetype)init {
	return [super initWithStyle:UITableViewStyleGrouped];
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return 2;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
	return _("Battman Credit");
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	//if(indexPath.row==1) {
	//	[[UIApplication sharedApplication] openURL:@"https://github.com/Torrekie/Battman" options:nil completionHandler:nil];
	//}
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	// TODO: REUSE (Too few cells to reuse for now so no need at this moment)
    // Consider also localize those names?
	if (indexPath.row == 0) {
		UITableViewCell *aCell = [UITableViewCell new];
		aCell.textLabel.text = @"Torrekie";
		return aCell;
	} else if (indexPath.row == 1) {
		UITableViewCell *bCell = [UITableViewCell new];
		bCell.textLabel.text = @"Ruphane";
		return bCell;
	}
	return nil;
}

@end
