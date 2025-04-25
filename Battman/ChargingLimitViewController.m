#import "ChargingLimitViewController.h"
#include "battery_utils/libsmc.h"
#include "common.h"

@implementation ChargingLimitViewController

- (NSString *)title {
	return @"Charging Limit";
}

- (instancetype)init {
	self=[super initWithStyle:UITableViewStyleGrouped];
	//self.tableView.allowsSelection=NO;
	return self;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
	if(sect==0) {
		return @"General";
	}else{
		return @"Smart Charging";
	}
}

- (NSString *)tableView:(UITableView *)tv titleForFooterInSection:(NSInteger)sect {
	if(sect==1) {
		return @"Smart Charging will start 900 seconds (15 minutes) after power is plugged-in, or the date you scheduled, whichever one comes first.";
	}
	return nil;
}

- (NSInteger)tableView:(UITableView *)tv numberOfRowsInSection:(NSInteger)sect {
	if(sect==1)
		return 3;
	return 1;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 2;
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

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.section==1&&indexPath.row==2) {
		NSBundle *powerUIBundle=[NSBundle bundleWithPath:@"/System/Library/PrivateFrameworks/PowerUI.framework"];
		if(![powerUIBundle load]) {
			show_alert("Failed","Failed to load PowerUI bundle.","ok");
			goto tvend;
		}
		id sccClass=[powerUIBundle classNamed:@"PowerUISmartChargeClient"];
		id sccObject=[[sccClass alloc] initWithClientName:@"ok"];
		if(![sccObject setState:1 error:nil]) {
			show_alert("Failed","Failed to enable smart charging.","ok");
			goto tvend;
		}
		[sccObject engageFrom:fromPicker.date until:untilPicker.date repeatUntil:untilPicker.date overrideAllSignals:1];
		//BOOL yyy=1;
		//smc_write_safe('CH0C', &yyy, 1);
		show_alert("Engaged", "Smart Charging has been engaged. It will start after 15 minutes of power supply, or at the date you picked, whichever comes first.", "OK");
	}
tvend:
	return [tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(UITableView *)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	UITableViewCell *cell=[UITableViewCell new];
	cell.selectionStyle=UITableViewCellSelectionStyleNone;
	if(indexPath.section==1) {
		if(indexPath.row==0) {
			cell.textLabel.text=@"Starting at";
			UIDatePicker *datePicker=[UIDatePicker new];
			datePicker.datePickerMode=UIDatePickerModeDateAndTime;
			datePicker.date=[NSDate now];
			datePicker.minimumDate=datePicker.date;
			cell.accessoryView=datePicker;
			fromPicker=datePicker;
		}else if(indexPath.row==1) {
			cell.textLabel.text=@"Until";
			UIDatePicker *datePicker=[UIDatePicker new];
			datePicker.datePickerMode=UIDatePickerModeDateAndTime;
			datePicker.date=[NSDate dateWithTimeIntervalSinceNow:3600*5];
			datePicker.minimumDate=[NSDate now];
			cell.accessoryView=datePicker;
			untilPicker=datePicker;
		}else{
			cell.selectionStyle=UITableViewCellSelectionStyleDefault;
			cell.textLabel.text=@"Schedule";
			cell.textLabel.textColor=[UIColor colorWithRed:0 green:0.478 blue:1 alpha:1];
		}
		return cell;
	}
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