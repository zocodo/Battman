#import <UIKit/UIKit.h>

@class MultiSelectSegmentedControl;

@protocol MultiSelectSegmentedControlDelegate <NSObject>
-(void)multiSelect:(MultiSelectSegmentedControl*) multiSelecSegmendedControl didChangeValue:(BOOL) value atIndex: (NSUInteger) index;
@end

@interface MultiSelectSegmentedControl : UISegmentedControl

@property (nonatomic, assign) NSIndexSet *selectedSegmentIndexes;
@property (nonatomic, weak) id<MultiSelectSegmentedControlDelegate> delegate;
@property (nonatomic, assign) NSArray *selectedSegmentTitles;

- (void)selectAllSegments:(BOOL)select; // pass NO to deselect all

@end
