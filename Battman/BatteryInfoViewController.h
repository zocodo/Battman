#import <UIKit/UIKit.h>
#include "battery_utils/battery_info.h"

@interface BatteryInfoViewController : UITableViewController
{
	struct battery_info_node *batteryInfo;
}
@end

UIImage *imageForSFProGlyph(NSString *glyph, NSString *fontName, CGFloat fontSize, UIColor *tintColor);
