#import "SettingsViewController.h"

@implementation SettingsViewController

- (NSString *)title {
	return @"More";
}

- (instancetype)init {
	UITabBarItem *tabbarItem=[[UITabBarItem alloc] initWithTabBarSystemItem:UITabBarSystemItemMore tag:1];
	tabbarItem.title=@"More";
	self.tabBarItem=tabbarItem;
	return [super initWithStyle:UITableViewStyleGrouped]; // or plain if desired
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return 2;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
	return @"About Battman";
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.row==0) {
		[self.navigationController pushViewController:[CreditViewController new] animated:YES];
	}else if(indexPath.row==1) {
		[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://github.com/Torrekie/Battman"] options:[NSDictionary new] completionHandler:nil];
	}
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	// TODO: REUSE (Too few cells to reuse for now so no need at this moment)
	if(indexPath.row==0) {
		UITableViewCell *creditCell=[UITableViewCell new];
		creditCell.accessoryType=UITableViewCellAccessoryDisclosureIndicator;
		creditCell.textLabel.text=@"Credit";
		return creditCell;
	}else if(indexPath.row==1) {
		UITableViewCell *sourceCodeCell=[UITableViewCell new];
		sourceCodeCell.textLabel.text=@"Source Code";
		sourceCodeCell.textLabel.textColor=[UIColor colorWithRed:0 green:0.478 blue:1 alpha:1];
		return sourceCodeCell;
	}
	UITableViewCell *batteryChargeCell=[UITableViewCell new];
	batteryChargeCell.textLabel.text=@"Test222";
	return batteryChargeCell;
}

@end

@implementation CreditViewController

- (NSString *)title {
	return @"Credit";
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
	return @"Battman Credit";
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	//if(indexPath.row==1) {
	//	[[UIApplication sharedApplication] openURL:@"https://github.com/Torrekie/Battman" options:nil completionHandler:nil];
	//}
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	// TODO: REUSE (Too few cells to reuse for now so no need at this moment)
	if(indexPath.row==0) {
		UITableViewCell *aCell=[UITableViewCell new];
		aCell.textLabel.text=@"Torrekie";
		return aCell;
	}else if(indexPath.row==1) {
		UITableViewCell *bCell=[UITableViewCell new];
		bCell.textLabel.text=@"Ruphane";
		return bCell;
	}
	return nil;
}

@end