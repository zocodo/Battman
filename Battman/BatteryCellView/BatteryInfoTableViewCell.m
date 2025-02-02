#import "BatteryInfoTableViewCell.h"
#include <stdint.h>
#include <stdlib.h>

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
	_batteryInfo=NULL;
	return self;
}

- (void)updateBatteryInfo {
	NSString *final_str=@"";
	for(struct battery_info_node *i=_batteryInfo;i!=NULL;i=i->next) {
		if((uint64_t)i->content > 1024) {
			final_str=[NSString stringWithFormat:@"%@\n%s: %s",final_str,i->description,(char*)i->content];
		}else if(((uint64_t)i->content & (1<<9))){
			// True
			if((uint64_t)i->content & 1) {
				final_str=[NSString stringWithFormat:@"%@\n%s",final_str,i->description];
			}
		}else{
			uint64_t masked_num=(uint64_t)i->content;
			int val=(masked_num&((1<<7)-1));
			final_str=[NSString stringWithFormat:@"%@\n%s: %d",final_str,i->description,val];
			if(masked_num&(1<<8)) {
				final_str=[final_str stringByAppendingString:@"%"];
				if(masked_num&(1<<7)) {
					[_batteryCell updateForegroundPercentage:val];
				}else{
					[_batteryCell updateBackgroundPercentage:val];
				}
			}
		}
	}
	_batteryLabel.text=final_str;
}

- (void)dealloc {
	for(struct battery_info_node *i=_batteryInfo;i!=NULL;/*i=i->next*/) {
		if((uint64_t)i->content > 1024) {
			free(i->content);
		}
		void *cur=i;
		i=i->next;
		free(cur);
	}
	[super dealloc];
}

@end