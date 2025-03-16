#include "../battery_utils/battery_info.h"
#import "BatteryCellView.h"
#import <UIKit/UIKit.h>

@interface TemperatureCellView : UIView
- (instancetype)initWithFrame:(CGRect)frame percentage:(CGFloat)percentage;
@end

@interface TemperatureInfoTableViewCell : UITableViewCell
@property(nonatomic, strong, readonly) TemperatureCellView *temperatureCell;
@property(nonatomic, strong, readonly) UILabel *temperatureLabel;

- (instancetype)initWithFrame:(CGRect)frame;
//- (void)updateTemperatureInfo;

@end
