#import "BatteryInfoTableViewCell.h"
#include "../common.h"
#include <stdint.h>
#include <stdlib.h>

@implementation BatteryInfoTableViewCell

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    BatteryCellView *batteryCell =
        [[BatteryCellView alloc] initWithFrame:CGRectMake(20, 20, 80, 80)
                          foregroundPercentage:0
                          backgroundPercentage:0];
    batteryCell.translatesAutoresizingMaskIntoConstraints = NO;
    [self.contentView addSubview:batteryCell];
    //[batteryCell.centerYAnchor constraintEqualToAnchor:self.centerYAnchor
    //constant:0].active=YES;
    UILabel *batteryRemainingLabel =
        [[UILabel alloc] initWithFrame:CGRectMake(120, 10, 600, 100)];
    batteryRemainingLabel.lineBreakMode = NSLineBreakByWordWrapping;
    batteryRemainingLabel.numberOfLines = 0;
    // batteryRemainingLabel.text=@"Battery Capacity: 80%\nCharge: 50%\nTest:
    // 0%";
    [self.contentView addSubview:batteryRemainingLabel];
    _batteryLabel = batteryRemainingLabel;
    _batteryCell = batteryCell;
    _batteryInfo = NULL;
    return self;
}

- (void)updateBatteryInfo {
    NSString *final_str = @"";
    // TODO: Arabian? We need Arabian hackers to fix this code
    for (struct battery_info_node *i = _batteryInfo; i != NULL; i = i->next) {
        if (i->content & BIN_IS_SPECIAL) {
            uint32_t value = i->content >> 32;
            float *fvptr = (float *)&value;
            float fvalue = *fvptr;
            if ((i->content & BIN_IS_FOREGROUND) == BIN_IS_FOREGROUND) {
                [_batteryCell updateForegroundPercentage:fvalue];
            } else if ((i->content & BIN_IS_BACKGROUND) == BIN_IS_BACKGROUND) {
                [_batteryCell updateBackgroundPercentage:fvalue];
            }
            if (i->content & BIN_IS_HIDDEN)
                continue;

            if ((i->content & BIN_IS_BOOLEAN) == BIN_IS_BOOLEAN && value) {
                final_str = [NSString
                    stringWithFormat:@"%@\n%@", final_str, _(i->description)];
            } else if ((i->content & BIN_IS_FLOAT) == BIN_IS_FLOAT) {
                final_str =
                    [NSString stringWithFormat:@"%@\n%@: %0.2f", final_str,
                                               _(i->description), fvalue];
            }
            if (i->content & BIN_HAS_UNIT) {
                uint32_t unit = (i->content & BIN_UNIT_BITMASK) >> 6;
                NSString *unit_str =
                    [[NSString alloc] initWithBytes:(char *)&unit
                                             length:4
                                           encoding:NSUTF8StringEncoding];
                final_str =
                    [NSString stringWithFormat:@"%@ %@", final_str, unit_str];
            }
        } else {
            final_str = [NSString stringWithFormat:@"%@\n%@: %s", final_str,
                                                   _(i->description),
                                                   (char *)i->content];
        }
    }
    _batteryLabel.text = [final_str substringFromIndex:1];
}

- (void)dealloc {
    for (struct battery_info_node *i = _batteryInfo; i != NULL; /*i=i->next*/) {
        if (i->content > 1024 && (i->content & 1) == 0) {
            free((void *)i->content);
        }
        void *cur = i;
        i = i->next;
        free(cur);
    }
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

@end
