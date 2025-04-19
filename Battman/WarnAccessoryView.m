#import "WarnAccessoryView.h"

@implementation WarnAccessoryView

+ (instancetype)warnAccessoryView {
    // Create an instance using the system's factory method.
    WarnAccessoryView *button = [super buttonWithType:UIButtonTypeSystem];
    if (button) {
        // Set the SF Symbol image.
        if (@available(iOS 13.0, *)) {
            UIImage *symbolImage = [UIImage systemImageNamed:@"exclamationmark.triangle"];
            [button setImage:symbolImage forState:UIControlStateNormal];
        } else {
            // Fallback to SF-Pro-Display-Regular.otf
            // U+1001FE
            [button setTitle:@"􀇾" forState:UIControlStateNormal];
            [button.titleLabel setFont:[UIFont fontWithName:@"SFProDisplay-Regular" size:22]];
        }
        button.tintColor = [UIColor systemBlueColor];
        button.frame = CGRectZero;
        [button sizeToFit];
    }
    return button;
}

@end
