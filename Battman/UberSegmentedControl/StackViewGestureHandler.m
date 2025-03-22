#import "StackViewGestureHandler.h"

@interface StackViewGestureHandler ()

@property (nonatomic, strong) UIStackView *stackView;
@property (nonatomic, assign) BOOL tracksMultiple;

@property (nonatomic, weak, nullable) UIButton *trackedButton;
@property (nonatomic, strong, nullable) NSValue *beginPoint; // Store CGPoint as NSValue
@property (nonatomic, strong) NSMutableSet<UIGestureRecognizer *> *currentGestures;
@property (nonatomic, strong) NSMutableSet<UIButton *> *recognizedButtons;

@property (nonatomic, weak, nullable) UIButton *highlightedButton;

@end

@implementation StackViewGestureHandler

#pragma mark - Initializer

- (instancetype)initWithStackView:(UIStackView *)stackView tracksMultiple:(BOOL)tracksMultiple isMomentary:(BOOL)isMomentary {
    self = [super init];
    if (self) {
        _stackView = stackView;
        _tracksMultiple = tracksMultiple;
        _isMomentary = isMomentary;
        _currentGestures = [NSMutableSet set];
        _recognizedButtons = [NSMutableSet set];
    }
    return self;
}

#pragma mark - Gesture Handling

- (nullable UIButton *)handleGesture:(UIGestureRecognizer *)recognizer {
    [self.currentGestures addObject:recognizer];

    @try {
        switch (recognizer.state) {
            case UIGestureRecognizerStateEnded:
            case UIGestureRecognizerStateCancelled:
            case UIGestureRecognizerStateFailed:
                [self.currentGestures removeObject:recognizer];

                if (self.currentGestures.count == 0) {
                    self.highlightedButton.highlighted = NO;
                    self.highlightedButton = nil;
                    self.trackedButton = nil;
                    self.beginPoint = nil;
                    [self.recognizedButtons removeAllObjects];
                }
                break;
            default:
                break;
        }
    } @finally {
        // Ensure the function continues execution.
    }

    // Return unless user interaction is enabled in `stackView`.
    // However, we still want to keep track of `currentGestures`.
    if (!self.stackView.isUserInteractionEnabled) {
        return nil;
    }

    CGPoint targetPoint;
    
    if ([recognizer isKindOfClass:[UIPanGestureRecognizer class]]) {
        targetPoint = [self calculateTargetPointUsingRecognizer:(UIPanGestureRecognizer *)recognizer];
    } else {
        targetPoint = [recognizer locationInView:self.stackView];
    }

    NSArray<UIView *> *arrangedSubviews = self.stackView.arrangedSubviews;
    
    for (UIView *view in arrangedSubviews) {
        if (![view isKindOfClass:[UIButton class]]) {
            continue;
        }
        
        UIButton *button = (UIButton *)view;
        
        if (CGRectContainsPoint(button.frame, targetPoint)) {
            if (self.recognizedButtons.count == 0 && self.trackedButton == nil && button.isSelected) {
                // store tracked button
                self.trackedButton = button;
            }
            
            if (!self.isMomentary) {
                self.highlightedButton = button;
            }
            
            if ([recognizer isKindOfClass:[UILongPressGestureRecognizer class]]) {
                if (self.isMomentary) {
                    // Only care about `began` state.
                    if (recognizer.state != UIGestureRecognizerStateBegan) {
                        continue;
                    }
                } else {
                    // Ignore long press gesture until gesture ends.
                    if (recognizer.state == UIGestureRecognizerStateBegan || recognizer.state == UIGestureRecognizerStateChanged) {
                        continue;
                    }
                }
            } else if ([recognizer isKindOfClass:[UIPanGestureRecognizer class]]) {
                // Ignore pan gesture if tracking single button and there is no tracked button.
                if (!self.tracksMultiple && self.trackedButton == nil) {
                    continue;
                }
            }

            if (![self.recognizedButtons containsObject:button]) {
                if (self.tracksMultiple || self.isMomentary) {
                    [self.recognizedButtons addObject:button];
                }
                return button;
            }
        }
    }

    return nil;
}

#pragma mark - Private Methods

- (CGPoint)calculateTargetPointUsingRecognizer:(UIPanGestureRecognizer *)recognizer {
    CGPoint beginPoint;
    
    if (self.beginPoint) {
        beginPoint = [self.beginPoint CGPointValue];
    } else {
        beginPoint = [recognizer locationInView:self.stackView];
        //self.beginPoint = [NSValue valueWithCGPoint:beginPoint];
    }
    
    CGPoint translation = [recognizer translationInView:self.stackView];
    
    CGAffineTransform transform;
    
    if (self.stackView.axis == UILayoutConstraintAxisHorizontal) {
        // Ignore y translation
        transform = CGAffineTransformMakeTranslation(translation.x, 0);
    } else {
        // Ignore x translation
        transform = CGAffineTransformMakeTranslation(0, translation.y);
    }

    return CGPointApplyAffineTransform(beginPoint, transform);
}

#pragma mark - Property Override for Highlighted Button

- (void)setHighlightedButton:(UIButton *)newHighlightedButton {
    if (_highlightedButton != newHighlightedButton) {
        _highlightedButton.highlighted = NO;
        _highlightedButton = newHighlightedButton;
        _highlightedButton.highlighted = YES;
    }
}

@end
