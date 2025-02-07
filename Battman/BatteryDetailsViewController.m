#import "BatteryDetailsViewController.h"
#include "battery_utils/libsmc.h"
#include "common.h"

// TODO: Function for advanced users to call SMC themselves.
// or add them to tracklist

@implementation BatteryDetailsViewController

- (NSString *)title {
    return _("Internal Battery");
}

- (instancetype)init {
    self = [super initWithStyle:UITableViewStyleGrouped];
    self.tableView.allowsSelection = NO;
    get_capacity(&b_remaining_capacity, &b_full_charge_capacity, &b_designed_capacity);
    return self;
}

- (NSString *)tableView:(id)tv titleForHeaderInSection:(NSInteger)section {
    if (section == 0)
        return _("Hardware Data");
    return nil;
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
    if (section == 0) {
        return 5;
    }
    return 0;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
    return 1;
}

- (UITableViewCell *)tableView:(UITableView *)tv
         cellForRowAtIndexPath:(NSIndexPath *)ip {
    UITableViewCell *cell =
        [tv dequeueReusableCellWithIdentifier:@"battmanbdvccl"];
    if (!cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                      reuseIdentifier:@"battmanbdvccl"];
    }
    /* FIXME: This shall be automatically refreshed without reloading */
    if (ip.section == 0) {
        if (ip.row == 0) {
            cell.textLabel.text = _("Full Charge Capacity");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%u mAh", b_full_charge_capacity];
            return cell;
        } else if (ip.row == 1) {
            cell.textLabel.text = _("Designed Capacity");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%u mAh", b_designed_capacity];
            return cell;
        } else if (ip.row == 2) {
            cell.textLabel.text = _("Remaining Capacity");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%u mAh", b_remaining_capacity];
            return cell;
        } else if (ip.row == 3) {
            cell.textLabel.text = _("Battery Count");
            cell.detailTextLabel.text =
            [NSString stringWithFormat:@"%d", battery_num()];
            return cell;
        } else if (ip.row == 4) {
            int time_to_empty = get_time_to_empty();
            NSString *time_str = [NSString stringWithFormat:@"%d %@", time_to_empty, _("Minutes")];
            cell.textLabel.text = _("Time To Empty");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%@", (time_to_empty == -1) ? _("Never") : time_str];
            return cell;
        }
    }
    return nil;
}

@end
