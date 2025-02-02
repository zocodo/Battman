#import <UIKit/UIKit.h>
#import "BatteryCellView.h"

// TODO: Implement underlying functions creating such infos
// Display: "$description: $content"
// or: "$description" in case *content==0

// content: <=1024: integer 1-100
// Bit 1<<9 == 1 : IsTrueOrFalse
// Bit 1<<9 == 0 : IsNum
// Bit 1<<8 == 1 : Affects Progress View (is percentage)
// Bit 1<<7 == 1 : Is Foreground
// Bit 1<<7 == 0 : Is Background
// Bit 1<<7 is ignored if bit 1<<8==0
// ELSE: char *
#define BIN_AFFECTS_BATTERY_CELL 1<<8
#define BIN_IS_FOREGROUND 1<<7|BIN_AFFECTS_BATTERY_CELL
#define BIN_IS_BACKGROUND 0|BIN_AFFECTS_BATTERY_CELL
#define BIN_IS_TRUE_OR_FALSE 1<<9
struct battery_info_node {
	const char *description; // NONNULL
	int identifier;
	void *content;
	struct battery_info_node *prev;
	struct battery_info_node *next;
};

@interface BatteryInfoTableViewCell : UITableViewCell
@property (nonatomic, assign, readwrite) struct battery_info_node *batteryInfo;
@property (nonatomic, strong, readonly) BatteryCellView *batteryCell;
@property (nonatomic, strong, readonly) UILabel *batteryLabel;

- (instancetype)initWithFrame:(CGRect)frame;
- (void)updateBatteryInfo;

@end