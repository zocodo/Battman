#import <UIKit/UIKit.h>

@interface SegmentedViewCell : UITableViewCell

@property (nonatomic, strong, nonnull) UILabel *titleLabel;
@property (nonatomic, strong, nullable) UILabel *detailLabel;
@property (nonatomic, strong, nullable) UIView *accessory;
@property (nonatomic, strong, nullable) UISegmentedControl *segmentedControl;
@property (nonatomic, strong, nullable) UILabel *subTitleLabel;
@property (nonatomic, strong, nullable) UILabel *subDetailLabel;

@end
