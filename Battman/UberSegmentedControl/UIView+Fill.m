#import "UIView+Fill.h"

@implementation UIView (Fill)

- (nullable NSArray<NSLayoutConstraint *> *)fillWithView:(UIView *)view
                                                constant:(CGFloat)constant
                                              usingGuide:(nullable UILayoutGuide *)layoutGuide
                                       shouldAutoActivate:(BOOL)shouldAutoActivate
{
    if ([self.subviews containsObject:view]) {
        return nil;
    }

    view.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:view];

    id anchorable = layoutGuide ?: self;
    NSMutableArray<NSLayoutConstraint *> *constraints = [NSMutableArray array];

    [constraints addObject:[view.leadingAnchor constraintEqualToAnchor:[anchorable leadingAnchor] constant:constant]];
    [constraints addObject:[view.trailingAnchor constraintEqualToAnchor:[anchorable trailingAnchor] constant:-constant]];
    [constraints addObject:[view.topAnchor constraintEqualToAnchor:[anchorable topAnchor] constant:constant]];
    [constraints addObject:[view.bottomAnchor constraintEqualToAnchor:[anchorable bottomAnchor] constant:-constant]];

    if (shouldAutoActivate) {
        [NSLayoutConstraint activateConstraints:constraints];
    }

    return constraints;
}

@end
