#import <UIKit/UIKit.h>
#import "BatteryCellView.h"
#include "../battery_utils/battery_info.h"

// TODO: Implement underlying functions creating such infos
// Display: "$description: $content"
// or: "$description" in case *content==0



@interface BatteryInfoTableViewCell : UITableViewCell
@property (nonatomic, assign, readwrite) struct battery_info_node *batteryInfo;
@property (nonatomic, strong, readonly) BatteryCellView *batteryCell;
@property (nonatomic, strong, readonly) UILabel *batteryLabel;

- (instancetype)initWithFrame:(CGRect)frame;
- (void)updateBatteryInfo;

@end
