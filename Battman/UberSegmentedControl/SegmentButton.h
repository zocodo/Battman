#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface SegmentButton : UIButton

@property (nonatomic, strong, nullable) UIColor *selectedBackgroundColor;

- (instancetype)initWithFont:(UIFont *)font tintColor:(UIColor *)tintColor;

@end

NS_ASSUME_NONNULL_END
