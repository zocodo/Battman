#import "BatteryDetailsViewController.h"
#include "battery_utils/libsmc.h"

// TODO: Function for advanced users to call SMC themselves.
// or add them to tracklist

@implementation BatteryDetailsViewController

- (NSString *)title {
	return @"System Battery";
}

- (instancetype)init {
	self=[super initWithStyle:UITableViewStyleGrouped];
	self.tableView.allowsSelection=NO;
	get_capacity(&b_remaining_capacity,&b_full_capacity,&b_designed_capacity);
	return self;
}

- (NSString *)tableView:(id)tv titleForHeaderInSection:(NSInteger)section {
	if(section==0)
		return @"SMC Data";
	return nil;
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	if(section==0) {
		return 3;
	}
	return 0;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (UITableViewCell *)tableView:(UITableView *)tv cellForRowAtIndexPath:(NSIndexPath *)ip {
	UITableViewCell *cell=[tv dequeueReusableCellWithIdentifier:@"battmanbdvccl"];
	if(!cell) {
		cell=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"battmanbdvccl"];
	}
	if(ip.section==0) {
		if(ip.row==0) {
			cell.textLabel.text=@"Full Charge Capacity";
			cell.detailTextLabel.text=[NSString stringWithFormat:@"%u mAh",b_full_capacity];
			return cell;
		}else if(ip.row==1) {
			cell.textLabel.text=@"Designed Charge Capacity";
			cell.detailTextLabel.text=[NSString stringWithFormat:@"%u mAh",b_designed_capacity];
			return cell;
		}else if(ip.row==2) {
			cell.textLabel.text=@"Remaining Charge";
			cell.detailTextLabel.text=[NSString stringWithFormat:@"%u mAh",b_remaining_capacity];
			return cell;
		}
	}
	return nil;
}

@end