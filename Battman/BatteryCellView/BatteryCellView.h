#import <UIKit/UIKit.h>
#import "SPWaterProgressIndicatorView.h"

@interface BatteryCellView : UIView
{
	SPWaterProgressIndicatorView *backgroundView;
	SPWaterProgressIndicatorView *foregroundView;
}

- (void)updateForegroundPercentage:(float)percent;
- (void)updateBackgroundPercentage:(float)percent;

- (instancetype)initWithFrame:(CGRect)frame foregroundPercentage:(float)percent backgroundPercentage:(float)bpercent;
@end
