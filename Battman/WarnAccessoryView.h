#pragma once

#import <UIKit/UIKit.h>

@interface WarnAccessoryView : UIButton

@property (nonatomic) BOOL isWarn;
@property (nonatomic,assign) const char *warn_title;
@property (nonatomic,assign) const char *warn_content;

+ (instancetype)_accessoryViewWithSystemImageNamed:(NSString *)systemName fallback:(NSString *)fallbackGlyph;

+ (instancetype)warnAccessoryView;
+ (instancetype)altAccessoryView;
@end
