#import "BatteryDetailsViewController.h"
#include "battery_utils/libsmc.h"
#include "common.h"

// TODO: Function for advanced users to call SMC themselves.
// or add them to tracklist

NSInteger rows = 14;
// TODO: Config
NSTimeInterval reload_interval = 5.0;

@implementation BatteryDetailsViewController

- (NSString *)title {
    return _("Internal Battery");
}

- (void)viewDidLoad {
    (void)[NSTimer scheduledTimerWithTimeInterval:reload_interval target:self selector:@selector(updateTableView) userInfo:nil repeats:YES];
}

- (void)updateTableView {
    get_capacity(&b_remaining_capacity, &b_full_charge_capacity, &b_designed_capacity);
    get_gas_gauge(&gauge);

    [self.tableView reloadData];
}

- (instancetype)init {
    self = [super initWithStyle:UITableViewStyleGrouped];
    self.tableView.allowsSelection = NO;
    get_capacity(&b_remaining_capacity, &b_full_charge_capacity, &b_designed_capacity);
    get_gas_gauge(&gauge);
    return self;
}

- (NSString *)tableView:(id)tv titleForHeaderInSection:(NSInteger)section {
    if (section == 0)
        return _("Hardware Data");
    return nil;
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
    if (section == 0) {
        return rows;
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
            NSString *rem_str = [NSString stringWithFormat:@"%d mAh", gauge.TrueRemainingCapacity * battery_num()];  // B0TR May not set
            cell.textLabel.text = _("True Remaining Capacity");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%@", (gauge.TrueRemainingCapacity == 0) ? _("None") : rem_str];
            return cell;
        } else if (ip.row == 4) {
            cell.textLabel.text = _("Qmax");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%u mAh", gauge.Qmax * battery_num()];
            return cell;
        } else if (ip.row == 5) {
            cell.textLabel.text = _("Depth of Discharge");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%u mAh", gauge.DOD0]; // Does this need multiply battery_num?
            return cell;
        } else if (ip.row == 6) {
            cell.textLabel.text = _("Passed Charge");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%u mAh", gauge.PassedCharge]; // Does this need multiply battery_num?
            return cell;
        } else if (ip.row == 7) {
            cell.textLabel.text = _("Voltage");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%u mV", gauge.Voltage];
            return cell;
        } else if (ip.row == 8) {
            cell.textLabel.text = _("Temperature");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%f °C", get_temperature()];
            return cell;
        } else if (ip.row == 9) {
            cell.textLabel.text = _("Average Current");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%d mA", gauge.AverageCurrent];
            return cell;
        } else if (ip.row == 10) {
            cell.textLabel.text = _("Average Power");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%d mW", gauge.AveragePower];
            return cell;
        } else if (ip.row == 11) {
            cell.textLabel.text = _("Battery Count");
            cell.detailTextLabel.text =
            [NSString stringWithFormat:@"%d", battery_num()];
            return cell;
        } else if (ip.row == 12) {
            int time_to_empty = get_time_to_empty(); // Gas Gauge TTE may not set
            NSString *time_str = [NSString stringWithFormat:@"%d %@", time_to_empty, _("Minutes")];
            cell.textLabel.text = _("Time To Empty");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%@", (time_to_empty == -1) ? _("Never") : time_str];
            return cell;
        } else if (ip.row == 13) {
            cell.textLabel.text = _("Cycle Count");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%u", gauge.CycleCount];
            return cell;
        } else if (ip.row == 14) {
            cell.textLabel.text = _("State Of Charge");
            cell.detailTextLabel.text =
                [NSString stringWithFormat:@"%u", gauge.StateOfCharge];
            return cell;
        }
    }
    return nil;
}

@end
