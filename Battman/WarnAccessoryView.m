#import "WarnAccessoryView.h"

@implementation WarnAccessoryView

+ (instancetype)warnAccessoryView {
    // Create an instance using the system's factory method.
    WarnAccessoryView *button = [super buttonWithType:UIButtonTypeSystem];
    if (button) {
        // Set the SF Symbol image.
        UIImage *symbolImage = [UIImage systemImageNamed:@"exclamationmark.triangle"];
        [button setImage:symbolImage forState:UIControlStateNormal];
        button.tintColor = [UIColor systemBlueColor];
        button.frame = CGRectZero;
        [button sizeToFit];
    }
    return button;
}

@end
