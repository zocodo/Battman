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
#define CREATE_TEST_NODE(name, _prev, id, desc, cont) \
		struct battery_info_node *name=malloc(sizeof(struct battery_info_node)); \
		name->prev=_prev; \
		name->identifier=id; \
		name->description=desc; \
		name->content=(void*)(cont);
	CREATE_TEST_NODE(bcapnode,NULL,1,"Battery Capacity",80|BIN_IS_BACKGROUND);
	CREATE_TEST_NODE(bchargenode,bcapnode,2,"Charge",50|BIN_IS_FOREGROUND);
	bcapnode->next=bchargenode;
	CREATE_TEST_NODE(bcmnode,bchargenode,3,"Charging",1|BIN_IS_TRUE_OR_FALSE);
	bchargenode->next=bcmnode;
	CREATE_TEST_NODE(bfalsetnode,bcmnode,4,"TEST FALSE YOU SHOULD NOT SEE THIS!!",0|BIN_IS_TRUE_OR_FALSE);
	bcmnode->next=bfalsetnode;
	bfalsetnode->next=NULL;
	cell.batteryInfo=bcapnode;
	[cell updateBatteryInfo];
	return cell;
}

- (CGFloat)tableView:(id)tv heightForRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.row==0) {
		return 130;
	}
	return 30;
}

@end
