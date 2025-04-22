#pragma once

#import <UIKit/UIKit.h>

@interface WarnAccessoryView : UIButton

@property (nonatomic) BOOL isWarn;

+ (instancetype)_accessoryViewWithSystemImageNamed:(NSString *)systemName fallback:(NSString *)fallbackGlyph;

+ (instancetype)warnAccessoryView;
+ (instancetype)altAccessoryView;
@end
