#import "BatteryDetailsViewController.h"
#include "battery_utils/libsmc.h"
#include "battery_utils/iokit_connection.h"
#include "common.h"
#include "intlextern.h"
#import "SegmentedViewCell.h"
#import "MultilineViewCell.h"

// TODO: Function for advanced users to call SMC themselves.
// or add them to tracklist
static NSMutableArray *sections_detail;
// TODO: Config
NSTimeInterval reload_interval = 5.0;
BOOL configured_autorefresh = NO;

/* Adapter Details */
static charging_state_t charging_stat;
static mach_port_t adapter_family;
static device_info_t adapter_info;
static charger_data_t adapter_data;
static hvc_menu_t *hvc_menu;
static int8_t hvc_index;
static size_t hvc_menu_size;
static bool hvc_soft;
static NSMutableArray *adapter_cells;

/* Desc */
static NSArray *desc_batt;
static NSArray *desc_adap;

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
    /* Consider add a "Accessory" section in data struct */
    if ([desc_batt indexOfObject:cell.textLabel.text] != NSNotFound) {
        DBGLOG(@"Accessory %@, Index %lu", cell.textLabel.text, [desc_batt indexOfObject:cell.textLabel.text]);
        cell.accessoryType = UITableViewCellAccessoryDetailButton;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }

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

    /* Consider add a "Accessory" section in data struct */
    desc_batt = @[
        _("Device Name"), _("This indicates the name of the current Gas Gauge IC used by the installed battery."),
        _("Depth of Discharge"), _("Current chemical depth of discharge (DOD₀). The gas gauge updates information on the DOD₀ based on open-circuit voltage (OCV) readings when in a relaxed state."),
        _("Chemistry ID"), _("Chemistry unique identifier (ChemID) assigned to each battery in Texas Instruments' database. It ensures accurate calculations and predictions."),
        _("Passed Charge"), _("The cumulative capacity of the current charging or discharging cycle. It is reset to zero with each DOD₀ update."),
        _("IT Misc Status"), _("This field refers to the miscellaneous data returned by battery Impedance Track™ Gas Gauge IC."),
        _("Flags"), _("The status information provided by the battery Gas Gauge IC, which may include the battery's operational modes, capabilities, or status codes. The format may vary depending on the Gas Gauge IC model."),
        _("Simulation Rate"), _("This field refers to the rate of Gas Gauge performing Impedance Track™ simulations."),
    ];

    desc_adap = @[
        _("Port"), _("Port of currently connectd adapter. On macOS, this is the USB port that the adapter currently attached."),
        _("Type"), _("This field refers to the Family Code (kIOPSPowerAdapterFamilyKey) of currently connected power adapter."),
        _("Current Rating"), _("Current rating of connected power source, this does not indicates the real-time passing current."),
        _("Voltage Rating"), _("Voltage rating of connected power source, this does not indicates the real-time passing voltage."),
        _("Description"), _("Short description provided by Apple PMU on current power adapter Family Code. Sometimes may not set."),
        _("Not Charging Reason"), _("If this field appears in the list, it indicates that an issue has occurred or that a condition was met, causing charging to stop."),
        _("HVC Mode"), _("High Voltage Charging (HVC) Mode may accquired by your power adapter or system, all supported modes will be listed below."),
        _("PMUConfiguration"), _("Private field used by Apple PMU. Sadly I don't know how to parse this yet. Contributing welcomed.")
    ];

    [self.tableView registerClass:[SegmentedViewCell class] forCellReuseIdentifier:@"HVC"];
    [self.tableView registerClass:[MultilineViewCell class] forCellReuseIdentifier:_("Adapter Details")];
    self.tableView.estimatedRowHeight = 100;
    self.tableView.rowHeight = UITableViewAutomaticDimension;
}

- (instancetype)initWithBatteryInfo:(struct battery_info_node *)bi {
    self = [super initWithStyle:UITableViewStyleGrouped];
    self.tableView.allowsSelection = YES; // for now no ops specified it will just be stuck
    battery_info_update(bi, true);
    batteryInfo = bi;
    charging_stat = is_charging(&adapter_family, &adapter_info);

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

    if (charging_stat > 0) {
        DBGLOG(@"charging_stat: %d", charging_stat);
        [sections_detail addObject:_("Adapter Details")];

        char *adapter_family_str = NULL;
        if (adapter_family) {
            adapter_family_str = get_adapter_family_desc(adapter_family);
        }
        (void)get_charger_data(&adapter_data);

        adapter_cells = [[NSMutableArray alloc] init];
        [adapter_cells addObjectsFromArray:@[
            @[_("Port"),                [NSString stringWithFormat:@"%d", adapter_info.port]],
            // This is terrible
            @[_("Compatibility"),       [NSString stringWithFormat:@"%@: %@\n%@: %@", _("Socket Connected"), (adapter_data.ChargerExist == 1) ? _("True") : _("False"), _("Charger Capable"), (adapter_data.ChargerCapable == 1) ? _("True") : _("False")]],
            @[_("Type"),                [NSString stringWithFormat:@"%@ (%.8X)", _(adapter_family_str), adapter_family]],
            @[_("Status"),              (charging_stat == kIsPausing || adapter_data.NotChargingReason != 0) ? _("Not Charging") : _("Charging")],
            @[_("Current Rating"),      [NSString stringWithFormat:@"%u %@", adapter_info.current, _("mA")]],
            @[_("Voltage Rating"),      [NSString stringWithFormat:@"%u %@", adapter_info.voltage, _("mV")]],
            @[_("Charging Current"),    [NSString stringWithFormat:@"%u %@", adapter_data.ChargingCurrent, _("mA")]],
            @[_("Charging Voltage"),    [NSString stringWithFormat:@"%u %@", adapter_data.ChargingVoltage, _("mV")]],
            @[_("Charger ID"),          [NSString stringWithFormat:@"0x%.4X", adapter_data.ChargerId]],
            @[_("Model Name"),          [NSString stringWithUTF8String:adapter_info.name]],
            @[_("Manufacturer"),        [NSString stringWithUTF8String:adapter_info.vendor]],
            @[_("Model"),               [NSString stringWithUTF8String:adapter_info.adapter]],
            @[_("Firmware Version"),    [NSString stringWithUTF8String:adapter_info.firmware]],
            @[_("Hardware Version"),    [NSString stringWithUTF8String:adapter_info.hardware]],
            /* Known Descriptions:
             pd charger: USB-C PD Charger
             usb charger: USB Charger
             usb host: USB Host device
             usb brick: USB Brick Charger
             usb type-c: Type-C Charger
             baseline arcas: Wireless Charger (Not MagSafe)
             magsafe chg: MagSafe Charger
             magsafe acc: MagSafe Accessory (Typically MagSafe Battery Pack)
             */
            @[_("Description"),         [NSString stringWithUTF8String:adapter_info.description]],
            @[_("Serial"),              [NSString stringWithUTF8String:adapter_info.serial]],
            /* TODO: Parse PMU Configuration Bits */
            @[_("PMUConfiguration"),    [NSString stringWithFormat:@"0x%.4X", adapter_info.PMUConfiguration]],
            @[_("ChargerConfiguration"),[NSString stringWithFormat:@"0x%.4X", adapter_data.ChargerConfiguration]],
            @[_("HVC Mode"),            @""], /* Special type, content controlled later */
        ]];
        if (adapter_data.NotChargingReason != 0) {
            [adapter_cells insertObject:@[_("Not Charging Reason"), [NSString stringWithUTF8String:not_charging_reason_str(adapter_data.NotChargingReason)]] atIndex:3];
        }
    }
    /* TODO: Gas Gauge (Advanced) */

    [self.tableView reloadData];
    [self.refreshControl endRefreshing];
}

- (void)tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];

    NSArray *target_desc;
    if (indexPath.section == 0)
        target_desc = desc_batt;
    if (indexPath.section == [sections_detail indexOfObject:_("Adapter Details")])
        target_desc = desc_adap;

    NSUInteger index = [target_desc indexOfObject:cell.textLabel.text];
    if (index != NSNotFound) {
        /* Special case: External */
        if ([cell.textLabel.text isEqualToString:_("Type")]) {
            NSString *finalstr = [target_desc objectAtIndex:(index + 1)];
            NSString *explaination_Ext = ((adapter_family & 0x20000) && (adapter_family & 0x7)) ? [NSString stringWithFormat:@"\n\n%@", _("\"External Power\" indicator may suggest that the connected adapter is a wireless charger. Most information may not be displayed because wireless chargers are handled differently by the hardware.")] : @"";
            show_alert([cell.textLabel.text UTF8String], [[NSString stringWithFormat:@"%@%@", finalstr, explaination_Ext] UTF8String], _C("OK"));
        } else {
            show_alert([cell.textLabel.text UTF8String], [[target_desc objectAtIndex:(index + 1)] UTF8String], _C("OK"));
        }
    }
    DBGLOG(@"Accessory Pressed, %@", cell.textLabel.text);
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
    /* Don't remove this, otherwise users will blame us */
    /* TODO: Identify other Gas Gauging system */
    if (section == 0) {
        return _("All Gas Gauge metrics are dynamically retrieved from the onboard sensor array in real time. Should anomalies be detected in specific readings, this may indicate the presence of unauthorized components or require diagnostics through Apple Authorised Service Provider.");
    }
    if (section == [sections_detail indexOfObject:_("Adapter Details")]) {
        return _("All adapter information is dynamically retrieved from the hardware of the currently connected adapter (or cable if you are using Lightning ports). If any of the data is missing, it may indicate that the power source is not providing the relevant information, or there may be a hardware issue with the power source.");
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
    /* Use different identifier to avoid wrong location of Accessory */
    NSString *cell_id = [sections_detail objectAtIndex:ip.section];
    UITableViewCell *cell = [tv dequeueReusableCellWithIdentifier:cell_id];
    if (!cell) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:cell_id];
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
        MultilineViewCell *celll = (MultilineViewCell *)cell;
        celll.titleLabel.text = adapter_cell[0];
        celll.detailLabel.text = ([adapter_cell[1] length] == 0) ? _("None") : adapter_cell[1];

        if ([desc_adap indexOfObject:cell.textLabel.text] != NSNotFound) {
            DBGLOG(@"Accessory %@, Index %lu", cell.textLabel.text, [desc_adap indexOfObject:cell.textLabel.text]);
            cell.accessoryType = UITableViewCellAccessoryDetailButton;
        } else {
            cell.accessoryType = UITableViewCellAccessoryNone;
        }
        /* HVC Mode special handler */
        if ([cell.textLabel.text isEqualToString:_("HVC Mode")]) {
            /* Parse HVC Modes if have any */
            if (adapter_info.hvc_menu[27] != 0xFF) {
#ifdef DEBUG
                NSString *bits = [[NSString alloc] init];
                for (int i = 0; i < 27; i++) {
                    [bits stringByAppendingFormat:@"%x ", adapter_info.hvc_menu[i]];
                }
                DBGLOG(@"HVC Menu Bits: %@", bits);
#endif
                hvc_menu = hvc_menu_parse(adapter_info.hvc_menu, &hvc_menu_size);
                hvc_index = adapter_info.hvc_index;
                hvc_soft = false;
            } else {
                hvc_soft = true;
                /* Avoid IOKit includes, we only use this one */
                extern CFDictionaryRef IOPSCopyExternalPowerAdapterDetails(void);
                hvc_menu = convert_hvc(IOPSCopyExternalPowerAdapterDetails(), &hvc_menu_size, &hvc_index);
            }
#if TARGET_OS_SIMULATOR
            /* Simulator builds cannot use IOPSCopyExternalPowerAdapterDetails() */
            /* We fake some hvc to test the UI instead */
            static hvc_menu_t fake_hvc[2];
            memset(fake_hvc, 0, sizeof(fake_hvc));
            fake_hvc[0].voltage = 114;
            fake_hvc[0].current = 514;
            fake_hvc[1].voltage = 1919;
            fake_hvc[1].current = 810;
            hvc_index = 1;
            hvc_menu = fake_hvc;
            hvc_menu_size = 2;
#endif

            /* Only use SegmentedViewCell when HVC exists */
            if (hvc_menu != NULL && hvc_menu_size != 0) {
                SegmentedViewCell *cell_seg = [tv dequeueReusableCellWithIdentifier:@"HVC"];
                if (!cell_seg) cell_seg = [[SegmentedViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"HVC"];
                cell_seg.accessoryType = UITableViewCellAccessoryDetailButton;
                cell_seg.textLabel.text = cell.textLabel.text; // For Accessory selection
                cell_seg.titleLabel.text = cell.textLabel.text;
                [cell_seg.segmentedControl addTarget:self action:@selector(hvcSegmentSelected:) forControlEvents:UIControlEventValueChanged];

                /* We have kept one sample seg to keep UI existence */
                // [cell_seg.segmentedControl setTitle:@"0" forSegmentAtIndex:0];
                [cell_seg.segmentedControl removeAllSegments];
                for (int i = 0; i < hvc_menu_size; i++) {
                    [cell_seg.segmentedControl insertSegmentWithTitle:[NSString stringWithFormat:@"%d", i] atIndex:i animated:YES];
                }

                /* Content */
                if (!hvc_soft && (hvc_index > hvc_menu_size)) {
                    cell_seg.detailLabel.text = [NSString stringWithFormat:@"%d (%@)", hvc_index, _("Not HVC")];
                    cell_seg.subTitleLabel.text = @" ";
                    cell_seg.subDetailLabel.text = @" ";
                } else if (hvc_soft == true) {
                    cell_seg.detailLabel.text = [NSString stringWithFormat:@"%d (%@)", hvc_index, _("Software Controlled")];
                    [cell_seg.segmentedControl setSelectedSegmentIndex:hvc_index];
                    /* Why its not refreshing label after setSelectedSegmentIndex? */
                    cell_seg.subTitleLabel.text = [NSString stringWithFormat:@"%d %@", hvc_menu[hvc_index].voltage, _("mV")];
                    cell_seg.subDetailLabel.text = [NSString stringWithFormat:@"%d %@", hvc_menu[hvc_index].current, _("mA")];
                } else if (hvc_index == -1) {
                    cell_seg.detailLabel.text = [NSString stringWithFormat:@"%d (%@)", hvc_index, _("Unavailable")];
                    cell_seg.subTitleLabel.text = @" ";
                    cell_seg.subDetailLabel.text = @" ";
                } else {
                    cell_seg.detailLabel.text = [NSString stringWithFormat:@"%d", hvc_index];
                    [cell_seg.segmentedControl setSelectedSegmentIndex:hvc_index];
                    /* Why its not refreshing label after setSelectedSegmentIndex? */
                    cell_seg.subTitleLabel.text = [NSString stringWithFormat:@"%d %@", hvc_menu[hvc_index].voltage, _("mV")];
                    cell_seg.subDetailLabel.text = [NSString stringWithFormat:@"%d %@", hvc_menu[hvc_index].current, _("mA")];
                }

                return cell_seg;
            } else {
                cell.detailTextLabel.text = _("None");
            }

        }
//        [cell layoutIfNeeded];
        return cell;
    }
    return nil;
}

- (void)hvcSegmentSelected:(UISegmentedControl *)segment
{
    UIView *view = segment;
    while (view && ![view isKindOfClass:[SegmentedViewCell class]]) {
        view = [view superview];
    }
    if (view) {
        SegmentedViewCell *cell_seg = (SegmentedViewCell *)view;
        // Now update the cell's title
        cell_seg.subTitleLabel.text = [NSString stringWithFormat:@"%d %@", hvc_menu[segment.selectedSegmentIndex].voltage, _("mV")];
        cell_seg.subDetailLabel.text = [NSString stringWithFormat:@"%d %@", hvc_menu[segment.selectedSegmentIndex].current, _("mA")];
        return;
    }

    DBGLOG(@"FIXME: hvcSegmentSelected without cell view!");
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)handleLongPress:(UILongPressGestureRecognizer *)gestureRecognizer {
    if (gestureRecognizer.state == UIGestureRecognizerStateBegan) {
        UITableViewCell *cell = (UITableViewCell *)gestureRecognizer.view;
        // We need better impl like PSTableCell's copy
        UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
        [pasteboard setString:cell.detailTextLabel.text];
        
        show_alert(_C("Copied!"), [cell.detailTextLabel.text UTF8String], _C("OK"));
    }
}

@end
