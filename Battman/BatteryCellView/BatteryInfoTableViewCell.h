#import <UIKit/UIKit.h>
#import "BatteryCellView.h"

@interface BatteryInfoTableViewCell : UITableViewCell
@property (nonatomic, strong, readonly) BatteryCellView *batteryCell;
@property (nonatomic, strong, readonly) UILabel *batteryLabel;

- (instancetype)initWithFrame:(CGRect)frame;

@end