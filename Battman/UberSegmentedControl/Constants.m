#import <UIKit/UIKit.h>
#import "Constants.h"
#import "../CompatibilityHelper.h"

@implementation ConstantsColor

+ (UIColor *)divider {
    if (@available(iOS 13.0, *)) {
        return [UIColor secondarySystemFillColor];
    } else {
        // orig: return [UIColor blackColor];
        return [UIColor colorWithRed:(120.0f / 255) green:(120.0f / 255) blue:(128.0f / 255) alpha:0.16f];
    }
}

+ (UIColor *)selectedSegmentTint {
    if (@available(iOS 13.0, *)) {
        return [UIColor colorWithDynamicProvider:^UIColor * _Nonnull(UITraitCollection * _Nonnull traits) {
            if ([(id)traits userInterfaceStyle] == UIUserInterfaceStyleDark) {
                return [[UIColor whiteColor] colorWithAlphaComponent:0.28];
            } else {
                return [UIColor whiteColor];
            }
        }];
    } else {
        return [UIColor whiteColor];
    }
}

+ (UIColor *)label {
    if (@available(iOS 13.0, *)) {
        return [UIColor labelColor];
    } else {
        // (kCGColorSpaceICCBased; kCGColorSpaceModelMonochrome; Generic Gray Gamma 2.2 Profile; extended range)] ( 0 1 )
        return [UIColor blackColor];
    }
}

+ (UIColor *)background {
    if (@available(iOS 13.0, *)) {
        return [UIColor tertiarySystemFillColor];
    } else {
        // orig: return [[UIColor blackColor] colorWithAlphaComponent:0.25];
        return [UIColor colorWithRed:(118.0f / 255) green:(118.0f / 255) blue:(128.0f / 255) alpha:0.12f];
    }
}

+ (UIColor *)segmentShadow {
    return [UIColor blackColor];
}

@end

@implementation ConstantsMeasure

+ (CGFloat)cornerRadius {
    return 8.0;
}

+ (CGFloat)spacingBetweenSegments {
    return 5.0;
}

+ (CGFloat)highlightedScale {
    return 0.95;
}

+ (CGFloat)highlightedAlpha {
    return 0.25;
}

+ (CGFloat)segmentCornerRadius {
    return 6.0;
}

+ (CGFloat)segmentShadowRadius {
    return 4.0;
}

+ (float)segmentShadowOpacity {
    return 0.1f;
}

+ (CGSize)segmentShadowOffset {
    return CGSizeMake(0, 3);
}

+ (CGFloat)segmentHeight {
    return 32.0;
}

@end

@implementation ConstantsMargins

+ (UIEdgeInsets)dividerInsets {
    return UIEdgeInsetsMake(6, 0, 6, 0);
}

+ (UIEdgeInsets)segmentInsets {
    return UIEdgeInsetsMake(2, 2, 2, 2);
}

+ (UIEdgeInsets)segmentContentEdgeInsets {
    // This is different from orig impl, we want larger edge
    return UIEdgeInsetsMake(5, -10, 5, -10);
}

+ (UIEdgeInsets)titleEdgeInsets {
    return UIEdgeInsetsMake(0, 5, 0, -5);
}

@end

@implementation ConstantsDuration

+ (NSTimeInterval)snappy {
    return 0.250;
}

+ (NSTimeInterval)regular {
    return 0.500;
}

@end

@implementation ConstantsFont

+ (UIFont *)segmentTitleLabel {
    // FIXME: UIFontWeightMedium when selected
    return [UIFont systemFontOfSize:(UIFont.smallSystemFontSize + 1) weight:UIFontWeightRegular];
}

@end
