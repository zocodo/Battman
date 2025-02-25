#import "BatteryDetailsViewController.h"
#include "battery_utils/libsmc.h"
#include "common.h"

// TODO: Function for advanced users to call SMC themselves.
// or add them to tracklist
static NSMutableArray *sections_detail;
// TODO: Config
NSTimeInterval reload_interval = 5.0;
BOOL configured_autorefresh = NO;

/* Adapter Details */
static mach_port_t adapter_family;
static device_info_t adapter_info;
static NSMutableArray *adapter_cells;

void equipDetailCell(UITableViewCell *cell, struct battery_info_node *i) {
	// PLEASE ENSURE no hidden cell is here when calling
        /*if ((i->content & BIN_DETAILS_SHARED) == BIN_DETAILS_SHARED ||
            (i->content &&
             !((i->content & BIN_IS_SPECIAL) == BIN_IS_SPECIAL))) {
            cell.hidden = NO;
        } else {
            cell.hidden = YES;
            return cell;
        }
        if (((i->content & 1) == 1) && (i->content & (1 << 5)) == (1 << 5)) {
            cell.hidden = YES;
            return cell;
        }*/
        NSString *final_str;
        cell.textLabel.text = _(i->description);
        if ((i->content & BIN_IS_SPECIAL) == BIN_IS_SPECIAL) {
            int16_t value = i->content >> 16;

            if ((i->content & BIN_IS_BOOLEAN) == BIN_IS_BOOLEAN) {
                if (value) {
                    final_str = _("True");
                } else {
                    final_str = _("False");
                }
            } else if ((i->content & BIN_IS_FLOAT) == BIN_IS_FLOAT) {
                final_str = [NSString stringWithFormat:@"%0.2f", bi_node_load_float(i)];
            } else {
                final_str = [NSString stringWithFormat:@"%d", value];
            }
            if (i->content & BIN_HAS_UNIT) {
                uint32_t unit = (i->content & BIN_UNIT_BITMASK) >> 6;
                final_str = [NSString
                    stringWithFormat:@"%@ %@", final_str, _(bin_unit_strings[unit])];
            }
        } else {
            final_str = [NSString stringWithUTF8String:bi_node_get_string(i)];
        }

        cell.detailTextLabel.text = final_str;
        return;
}


@implementation BatteryDetailsViewController

- (NSString *)title {
    return _("Internal Battery");
}

- (void)viewWillAppear:(BOOL)animated {
    [self updateTableView];
}

- (void)viewDidLoad {
    /* We knows too less to listen on SMC events */
    if (configured_autorefresh) {
        (void)[NSTimer scheduledTimerWithTimeInterval:reload_interval
                                               target:self
                                             selector:@selector(updateTableView)
                                             userInfo:nil
                                              repeats:YES];
    }
    UIRefreshControl *puller = [[UIRefreshControl alloc] init];
    [puller addTarget:self action:@selector(updateTableView) forControlEvents:UIControlEventValueChanged];
    self.refreshControl = puller;
}

- (instancetype)initWithBatteryInfo:(struct battery_info_node *)bi {
    self = [super initWithStyle:UITableViewStyleGrouped];
    self.tableView.allowsSelection = NO; // for now no ops specified it will just be stuck
    battery_info_update(bi, true);
    batteryInfo = bi;
    /* Don't remove this, otherwise users will blame us */
    /* TODO: Identify other Gas Gauging system */
    NSString *gauge_disclaimer = _("All Gas Gauge metrics are dynamically retrieved from the onboard sensor array in real time. Should anomalies be detected in specific readings, this may indicate the presence of unauthorized components or require diagnostics through Apple Authorised Service Provider.");
    NSString *explaination_IT = (gGauge.ITMiscStatus != 0) ? [NSString stringWithFormat:@"\n\n%@", _("The \"IT Misc Status\" field refers to the miscellaneous data returned by battery Impedance Track™ Gas Gauge IC.")] : @"";
    NSString *explaination_Sim = (gGauge.SimRate != 0) ? [NSString stringWithFormat:@"\n\n%@", _("The \"Simulation Rate\" field refers to the rate of battery performing Impedance Track™ simulations.")] : @"";
    gasGaugeDisclaimer = [NSString stringWithFormat:@"%@%@%@", gauge_disclaimer, explaination_IT, explaination_Sim];

    NSString *adap_disclaimer = _("All adapter information is dynamically retrieved from the hardware of the currently connected adapter. If any of the data is missing, it may indicate that the connected adapter is not providing the relevant information, or there may be a hardware issue with the adapter.");
    NSString *explaination_Ext = ((adapter_family & 0x20000) && (adapter_family & 0x7)) ? [NSString stringWithFormat:@"\n\n%@", _("\"External Power\" may indicates the connected adapter is a wireless charger.")] : @"";

    adapterDisclaimer = [NSString stringWithFormat:@"%@%@", adap_disclaimer, explaination_Ext];
    return self;
}

- (void)updateTableView {
    [self.refreshControl beginRefreshing];
    battery_info_update(batteryInfo, true);

    /* Dynasects */
    /* TODO: Handle the scene that if battery not present */
#if !__has_feature(objc_arc)
    if (sections_detail) [sections_detail dealloc];
    if (adapter_cells) [adapter_cells dealloc];
#endif
    sections_detail = [NSMutableArray arrayWithArray:@[_("Gas Gauge (Basic)")]];
    charging_state_t charging_stat = is_charging(&adapter_family, &adapter_info);
    if (charging_stat > 0) {
        DBGLOG(@"charging_stat: %d", charging_stat);
        [sections_detail addObject:_("Adapter Details")];

        char *adapter_family_str = NULL;
        if (adapter_family) {
            adapter_family_str = get_adapter_family_desc(adapter_family);
            show_alert("Family", adapter_family_str, "OK");
        }

        adapter_cells = [[NSMutableArray alloc] init];
        [adapter_cells addObjectsFromArray:@[
            @[_("Port"),           [NSString stringWithFormat:@"%d", adapter_info.port]],
            @[_("Type"),           [NSString stringWithFormat:@"%s (%.8X)", adapter_family_str, adapter_family]],
            @[_("Status"),         (charging_stat == kIsPausing) ? _("Not Charging") : _("Charging")],
            /* TODO: Parse NotChargingReason Bits */
            @[_("Current"),        [NSString stringWithFormat:@"%u %@", adapter_info.current, _("mA")]],
            @[_("Voltage"),        [NSString stringWithFormat:@"%u %@", adapter_info.voltage, _("mV")]],
            @[_("Name"),           [NSString stringWithUTF8String:adapter_info.name]],
            @[_("Manufacturer"),   [NSString stringWithUTF8String:adapter_info.vendor]],
            @[_("Model"),          [NSString stringWithUTF8String:adapter_info.adapter]],
            @[_("Firmware"),       [NSString stringWithUTF8String:adapter_info.firmware]],
            @[_("Hardware"),       [NSString stringWithUTF8String:adapter_info.hardware]],
            @[_("Description"),    [NSString stringWithUTF8String:adapter_info.description]],
            @[_("Serial"),         [NSString stringWithUTF8String:adapter_info.serial]],
            /* TODO: Parse PMU Configuration Bits */
            @[_("PMUConfiguration"), [NSString stringWithFormat:@"0x%.4X", adapter_info.PMUConfiguration]],
            /* TODO: Hvc */
        ]];
    }
    /* TODO: Gas Gauge (Advanced) */

    [self.tableView reloadData];
    [self.refreshControl endRefreshing];
}

- (NSString *)tableView:(id)tv titleForHeaderInSection:(NSInteger)section {
    // Doesn't matter, it will be changed by willDisplayHeaderView
    return @"This is a Title yeah";
}

- (void)tableView:(UITableView *)tableView willDisplayHeaderView:(UIView *)view forSection:(NSInteger)section {
    UITableViewHeaderFooterView *header = (UITableViewHeaderFooterView *)view;
    header.textLabel.text = sections_detail[section];
}

- (NSString *)tableView:(UITableView *)tableView
    titleForFooterInSection:(NSInteger)section {
    if (section == 0) {
    	return gasGaugeDisclaimer;
    }
    if (section == [sections_detail indexOfObject:_("Adapter Details")]) {
        return adapterDisclaimer;
    }
    return nil;
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	if (section == 0) {
		int rows = 0;
		for (struct battery_info_node *i = batteryInfo; i->description; i++) {
			if ((i->content & BIN_DETAILS_SHARED) == BIN_DETAILS_SHARED ||
				(i->content && !((i->content & BIN_IS_SPECIAL) == BIN_IS_SPECIAL))) {
				if((i->content & 1) != 1 || (i->content & (1 << 5)) != 1 << 5) {
					pendingLoadOffsets[rows] = (unsigned char)((i - batteryInfo) - rows);
					rows++;
				}
			}
		}
		pendingLoadOffsets[rows] = 255;
		return rows;
	}
    if (section == [sections_detail indexOfObject:_("Adapter Details")]) {
        return adapter_cells.count;
    }
	return 0;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
    return sections_detail.count;
}

- (UITableViewCell *)tableView:(UITableView *)tv
         cellForRowAtIndexPath:(NSIndexPath *)ip {
    UITableViewCell *cell =
        [tv dequeueReusableCellWithIdentifier:@"battmanbdvccl"];
    if (!cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                      reuseIdentifier:@"battmanbdvccl"];
    }
    UILongPressGestureRecognizer *longPressRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleLongPress:)];
    [cell addGestureRecognizer:longPressRecognizer];

    if (ip.section == 0) {
    	equipDetailCell(cell, batteryInfo + ip.row + pendingLoadOffsets[ip.row]);
    	return cell;
    }

    // Consider make this an adapter_info.c?
    if (ip.section == [sections_detail indexOfObject:_("Adapter Details")]) {
        NSArray *adapter_cell = adapter_cells[ip.row];
        cell.textLabel.text = adapter_cell[0];
        cell.detailTextLabel.text = adapter_cell[1];
        return cell;
    }
    return nil;
}

- (void)handleLongPress:(UILongPressGestureRecognizer *)gestureRecognizer {
    if (gestureRecognizer.state == UIGestureRecognizerStateBegan) {
        UITableViewCell *cell = (UITableViewCell *)gestureRecognizer.view;
        // We need better impl like PSTableCell's copy
        UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
        [pasteboard setString:cell.detailTextLabel.text];
        
        show_alert([_("Copied!") UTF8String], [cell.detailTextLabel.text UTF8String], [_("OK") UTF8String]);
    }
}

@end
