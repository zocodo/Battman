//
//  SliderTableViewCell.h
//  Battman
//
//  Created by Torrekie on 2025/5/1.
//

#import <UIKit/UIKit.h>

@protocol SliderTableViewCellDelegate;

@interface SliderTableViewCell : UITableViewCell <UITextFieldDelegate>

@property (nonatomic, strong) UISlider *slider;
@property (nonatomic, strong) UITextField *textField;
@property (nonatomic, weak) id<SliderTableViewCellDelegate> delegate;

@end

@protocol SliderTableViewCellDelegate <NSObject>
- (void)sliderTableViewCell:(SliderTableViewCell *)cell didChangeValue:(float)value;
@end
