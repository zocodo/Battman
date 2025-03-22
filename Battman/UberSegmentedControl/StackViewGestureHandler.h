#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface StackViewGestureHandler : NSObject

@property (nonatomic, assign) BOOL isMomentary;

- (instancetype)initWithStackView:(UIStackView *)stackView tracksMultiple:(BOOL)tracksMultiple isMomentary:(BOOL)isMomentary;
- (nullable UIButton *)handleGesture:(UIGestureRecognizer *)recognizer;
- (void)setHighlightedButton:(UIButton * _Nullable)newHighlightedButton;

@end

NS_ASSUME_NONNULL_END
