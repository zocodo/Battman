#import <UIKit/UIKit.h>

@interface UIView (Fill)

- (nullable NSArray<NSLayoutConstraint *> *)fillWithView:(UIView * _Nonnull)view
                                                constant:(CGFloat)constant
                                              usingGuide:(nullable UILayoutGuide *)layoutGuide
                                       shouldAutoActivate:(BOOL)shouldAutoActivate;

@end
