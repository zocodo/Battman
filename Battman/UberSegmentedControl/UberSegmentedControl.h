#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@class UberSegmentedControlConfig;

/**
 * A control made of multiple segments, each segment functioning as a discrete button with support for single or
 * multiple selection mode.
 */
@interface UberSegmentedControl : UIControl

/** A segment index value indicating that there is no selected segment. */
@property (class, readonly) NSInteger noSegment;

/** Whether this segmented control allows multiple selection. */
@property (nonatomic, readonly) BOOL allowsMultipleSelection;

/** A Boolean value that determines whether segments in the receiver show selected state. */
@property (nonatomic, assign) BOOL isMomentary;

/** Returns the number of segments the receiver has. */
@property (nonatomic, readonly) NSInteger numberOfSegments;

/** The color to use for highlighting the currently selected segment. */
@property (nonatomic, readonly, nullable) UIColor *selectedSegmentTintColor;

/** Indexes of selected segments (can be more than one if allowsMultipleSelection is YES.) */
@property (nonatomic, copy) NSIndexSet *selectedSegmentIndexes;

/**
 * The index number identifying the selected segment (that is, the last segment touched).
 * When allowsMultipleSelection is enabled, this property returns UberSegmentedControl.noSegment and setting a new value does nothing.
 */
@property (nonatomic, assign) NSInteger selectedSegmentIndex;

/**
 * Initializes and returns a segmented control with segments having the given titles or images.
 *
 * @param items An array of NSString objects (for segment titles) or UIImage objects (for segment images).
 * @param config A Config object.
 */
- (instancetype)initWithItems:(nullable NSArray *)items config:(nullable UberSegmentedControlConfig *)config;

/**
 * Inserts a segment at a specific position in the receiver and gives it a title as content.
 *
 * @param title A string to use as the segment's title.
 * @param segment An index number identifying a segment in the control.
 * @param animated YES if the insertion of the new segment should be animated, otherwise NO.
 */
- (void)insertSegmentWithTitle:(nullable NSString *)title atIndex:(NSInteger)segment animated:(BOOL)animated;

/**
 * Inserts a segment at a specified position in the receiver and gives it an image as content.
 *
 * @param image An image object to use as the content of the segment.
 * @param segment An index number identifying a segment in the control.
 * @param animated YES if the insertion of the new segment should be animated, otherwise NO.
 */
- (void)insertSegmentWithImage:(nullable UIImage *)image atIndex:(NSInteger)segment animated:(BOOL)animated;

/**
 * Removes the specified segment from the receiver, optionally animating the transition.
 *
 * @param segment An index number identifying a segment in the control.
 * @param animated YES if the removal of the segment should be animated, otherwise NO.
 */
- (void)removeSegmentAtIndex:(NSInteger)segment animated:(BOOL)animated;

/**
 * Removes all segments of the receiver.
 */
- (void)removeAllSegments;

/**
 * Sets the title of a segment.
 *
 * @param title A string to display in the segment as its title.
 * @param segment An index number identifying a segment in the control.
 */
- (void)setTitle:(nullable NSString *)title forSegmentAtIndex:(NSInteger)segment;

/**
 * Returns the title of the specified segment.
 *
 * @param segment An index number identifying a segment in the control.
 */
- (nullable NSString *)titleForSegmentAtIndex:(NSInteger)segment;

/**
 * Sets the content of a segment to a given image.
 *
 * @param image An image object to display in the segment.
 * @param segment An index number identifying a segment in the control.
 */
- (void)setImage:(nullable UIImage *)image forSegmentAtIndex:(NSInteger)segment;

/**
 * Returns the image for a specific segment.
 *
 * @param segment An index number identifying a segment in the control.
 */
- (nullable UIImage *)imageForSegmentAtIndex:(NSInteger)segment;

/**
 * Enables the specified segment.
 *
 * @param enabled YES to enable the specified segment or NO to disable the segment. By default, segments are enabled.
 * @param segment An index number identifying a segment in the control.
 */
- (void)setEnabled:(BOOL)enabled forSegmentAtIndex:(NSInteger)segment;

/**
 * Returns whether the indicated segment is enabled.
 *
 * @param segment An index number identifying a segment in the control.
 */
- (BOOL)isEnabledForSegmentAtIndex:(NSInteger)segment;

/**
 * Sets the semantic content attribute for a given segment.
 *
 * @param segment An index number identifying a segment in the control.
 * @param attribute A UISemanticContentAttribute to apply to the segment.
 */
- (void)setSegmentSemanticContentAttributeAtIndex:(NSInteger)segment attribute:(UISemanticContentAttribute)attribute;

/**
 * Sets the default image edge inset for a given segment.
 *
 * @param segment An index number identifying a segment in the control.
 * @param insets The UIEdgeInsets to apply to the segment's image.
 */
- (void)setSegmentImageEdgeInsetsAtIndex:(NSInteger)segment insets:(UIEdgeInsets)insets;

/**
 * Sets the default title edge inset for a given segment.
 *
 * @param segment An index number identifying a segment in the control.
 * @param insets The UIEdgeInsets to apply to the segment's title.
 */
- (void)setSegmentTitleEdgeInsetsAtIndex:(NSInteger)segment insets:(UIEdgeInsets)insets;

@end

/**
 * A config object that may be provided to UberSegmentedControl upon initialization.
 */
@interface UberSegmentedControlConfig : NSObject

/** The UIFont to use for each segment in the UberSegmentedControl. */
@property (nonatomic, strong, readonly) UIFont *font;

/** The UIColor to use for each segment in the UberSegmentedControl. */
@property (nonatomic, strong, readonly) UIColor *tintColor;

/** Whether the UberSegmentedControl supports multiple selection. */
@property (nonatomic, assign) BOOL allowsMultipleSelection;

/** Default font. */
@property (class, readonly) UIFont *defaultFont;

/** Default tint color. */
@property (class, readonly) UIColor *defaultTintColor;

/**
 * Initializes a new Config object with any user-provided options.
 *
 * @param font The font to use for segments.
 * @param tintColor The tint color to use for segments.
 * @param allowsMultipleSelection Whether multiple selection is allowed.
 */
- (instancetype)initWithFont:(nullable UIFont *)font
                   tintColor:(nullable UIColor *)tintColor
      allowsMultipleSelection:(BOOL)allowsMultipleSelection;

@end

NS_ASSUME_NONNULL_END
