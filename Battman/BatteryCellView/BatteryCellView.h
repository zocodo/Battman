#import <UIKit/UIKit.h>
#import "SPWaterProgressIndicatorView.h"

@interface BatteryCellView : UIView
{
	SPWaterProgressIndicatorView *backgroundView;
	SPWaterProgressIndicatorView *foregroundView;
}

- (void)updateForegroundPercentage:(NSUInteger)percent;
- (void)updateBackgroundPercentage:(NSUInteger)percent;

- (instancetype)initWithFrame:(CGRect)frame foregroundPercentage:(NSUInteger)percent backgroundPercentage:(NSUInteger)bpercent;
@end