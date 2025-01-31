//
//  SPWaterProgressIndicatorView.h
//
//  Created by Antonio081014 on 9/4/15.
//  Copyright (c) 2015 antonio081014.com. All rights reserved.
//
//  Great Thanks to Stefan Ceriu.
//  Ref: https://github.com/stefanceriu/SCSiriWaveformView

@import UIKit;

IB_DESIGNABLE
@interface SPWaterProgressIndicatorView : UIView

/**
 * Update the percent of completion
 */
- (void)updateWithPercentCompletion:(NSUInteger)percent;

/**
 * Start animation
 */
- (void)startAnimation;

/**
 * Stop animation
 */
- (void)stopAnimation;

/*
 * Percent of completion
 * Default: 0
 */
@property (nonatomic, readonly) IBInspectable NSUInteger completionInPercent;

/*
 * Font size used to display the text of completionInPercent.
 * Default: 20.0f
 */
@property (nonatomic) IBInspectable CGFloat fontSize;

/*
 * Color to use when drawing the waves
 * Default: cyan
 */
@property (nonatomic, strong) IBInspectable UIColor *waveColor;

/*
 * The frequency of the sinus wave. The higher the value, the more sinus wave peaks you will have.
 * Default: 1.5
 */
@property (nonatomic) IBInspectable CGFloat frequency;

/*
 * The current amplitude
 */
@property (nonatomic) IBInspectable CGFloat amplitude;

/*
 * The lines are joined stepwise, the more dense you draw, the more CPU power is used.
 * Default: 5
 */
@property (nonatomic) IBInspectable CGFloat density;

/*
 * The phase shift that will be applied with each level setting
 * Change this to modify the animation speed or direction
 * Default: -0.15
 */
@property (nonatomic) IBInspectable CGFloat phaseShift;

@end
