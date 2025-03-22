#import "DividerView.h"
#import "Constants.h"

@implementation DividerView

- (instancetype)initWithAlpha:(CGFloat)alpha {
    self = [self initWithFrame:CGRectZero];
    if (self) {
        self.alpha = alpha;
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        self.opaque = NO;
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    [NSException raise:NSGenericException format:@"initWithCoder: not implemented"];
    return nil;
}

- (void)drawRect:(CGRect)rect {
#if TARGET_OS_IPHONE
    CGContextRef context = UIGraphicsGetCurrentContext();
#else
    CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
#endif
    if (!context) return;

    CGContextSaveGState(context);

    CGRect gradientRect = CGRectMake(CGRectGetMaxX(self.bounds) - 1,
                                     CGRectGetMinY(self.bounds),
                                     CGRectGetMaxX(self.bounds),
                                     CGRectGetHeight(self.bounds));

    CGContextAddRect(context, gradientRect);
    CGContextClip(context);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    
    NSArray *colors = @[
        (id)[UIColor clearColor].CGColor,
        (id)[ConstantsColor divider].CGColor,
        (id)[ConstantsColor divider].CGColor,
        (id)[UIColor clearColor].CGColor
    ];
    
    CGFloat locations[] = { 0.0, 0.1, 0.9, 1.0 };
    CGGradientRef gradient = CGGradientCreateWithColors(colorSpace, (__bridge CFArrayRef)colors, locations);
    
    CGPoint startPoint = CGPointMake(CGRectGetMidX(self.bounds), CGRectGetMinY(self.bounds));
    CGPoint endPoint = CGPointMake(CGRectGetMidX(self.bounds), CGRectGetMaxY(self.bounds));
    
    CGContextDrawLinearGradient(context, gradient, startPoint, endPoint, kCGGradientDrawsAfterEndLocation);

    CGGradientRelease(gradient);
    CGColorSpaceRelease(colorSpace);
    
    CGContextRestoreGState(context);
}

@end
