#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface ConstantsColor : NSObject

+ (UIColor *)divider;
+ (UIColor *)selectedSegmentTint;
+ (UIColor *)label;
+ (UIColor *)background;
+ (UIColor *)segmentShadow;

@end

@interface ConstantsMeasure : NSObject

+ (CGFloat)cornerRadius;
+ (CGFloat)spacingBetweenSegments;
+ (CGFloat)highlightedScale;
+ (CGFloat)highlightedAlpha;
+ (CGFloat)segmentCornerRadius;
+ (CGFloat)segmentShadowRadius;
+ (float)segmentShadowOpacity;
+ (CGSize)segmentShadowOffset;
+ (CGFloat)segmentHeight;

@end

@interface ConstantsMargins : NSObject

+ (UIEdgeInsets)dividerInsets;
+ (UIEdgeInsets)segmentInsets;
+ (UIEdgeInsets)segmentContentEdgeInsets;
+ (UIEdgeInsets)titleEdgeInsets;

@end

@interface ConstantsDuration : NSObject

+ (NSTimeInterval)snappy;
+ (NSTimeInterval)regular;

@end

@interface ConstantsFont : NSObject

+ (UIFont *)segmentTitleLabel;

@end

NS_ASSUME_NONNULL_END
