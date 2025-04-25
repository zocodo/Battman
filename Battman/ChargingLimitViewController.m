#import "ChargingLimitViewController.h"
#include "battery_utils/libsmc.h"

@implementation ChargingLimitViewController

- (NSString *)title {
	return @"Charging Limit";
}

- (instancetype)init {
	self=[super initWithStyle:UITableViewStyleGrouped];
	self.tableView.allowsSelection=NO;
	return self;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
	if(sect==0) {
		return @"General";
	}else{
		return @"Smart Charging";
	}
}

- (NSInteger)tableView:(UITableView *)tv numberOfRowsInSection:(NSInteger)sect {
	return 1;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (void)setBlockCharging:(UISwitch *)cswitch {
	BOOL val=cswitch.on;
	smc_write_safe('CH0C', &val, 1);
	int new_val;
	smc_read_n('CH0C',&new_val,1);
	new_val&=0xff;
	if((new_val!=0)!=val) {
		cswitch.on=(new_val!=0);
	}
}

- (UITableViewCell *)tableView:(UITableView *)tv cellForRowAtIndexPath:(NSIndexPath *)ip {
	UITableViewCell *cell=[UITableViewCell new];
	cell.textLabel.text=@"Block Charging";
	UISwitch *cswitch=[UISwitch new];
	int switchOn;
	smc_read_n('CH0C', &switchOn, 1);
	cswitch.on=(switchOn&0xff)!=0;
	[cswitch addTarget:self action:@selector(setBlockCharging:) forControlEvents:UIControlEventValueChanged];
	cell.accessoryView=cswitch;
	return cell;
}

@end