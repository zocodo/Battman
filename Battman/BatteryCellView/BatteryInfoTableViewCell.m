#import "BatteryInfoTableViewCell.h"

@implementation BatteryInfoTableViewCell

- (instancetype)initWithFrame:(CGRect)frame {
	self=[super initWithFrame:frame];
	BatteryCellView *batteryCell=[[BatteryCellView alloc] initWithFrame:CGRectMake(20,20,80,80) foregroundPercentage:0 backgroundPercentage:0];
	[self.contentView addSubview:batteryCell];
	UILabel *batteryRemainingLabel=[[UILabel alloc] initWithFrame:CGRectMake(120,10,600,100)];
	batteryRemainingLabel.lineBreakMode=NSLineBreakByWordWrapping;
	batteryRemainingLabel.numberOfLines=0;
	//batteryRemainingLabel.text=@"Battery Capacity: 80%\nCharge: 50%\nTest: 0%";
	[self.contentView addSubview:batteryRemainingLabel];
	_batteryLabel=batteryRemainingLabel;
	_batteryCell=batteryCell;
	return self;
}

@end