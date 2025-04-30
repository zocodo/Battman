#import "BatteryDetailsViewController.h"
#include "battery_utils/libsmc.h"
#include "battery_utils/iokit_connection.h"
#include "common.h"
#include "intlextern.h"
#import "SegmentedViewCell.h"
#import "MultilineViewCell.h"
#import "WarnAccessoryView.h"

#include <sys/sysctl.h>

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
static NSMutableArray *warns;

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
        if (@available(iOS 13.0, *)) {
            cell.accessoryType = UITableViewCellAccessoryDetailButton;
        } else {
            WarnAccessoryView *button = [WarnAccessoryView altAccessoryView];
            [cell setAccessoryType:UITableViewCellAccessoryNone];
            [cell setAccessoryView:button];
        }
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

typedef enum {
    WARN_NONE,          // OK
    WARN_GENERAL,       // General warning
    WARN_UNUSUAL,       // Unusual value warning
    WARN_EXCEDDED,      // Excedded value warning
    WARN_EMPTYVAL,      // Empty value warning
    WARN_MAX,           // max count of warn, should always be at bottom
} warn_condition_t;

void equipWarningCondition_b(UITableViewCell *equippedCell, NSString *textLabel, warn_condition_t (^condition)(NSString **warn)) {
    if (!equippedCell.textLabel.text) {
        DBGLOG(@"equipWarningCondition() called too early");
        return;
    }
    if (condition == nil) return;
    if (![equippedCell.textLabel.text isEqualToString:textLabel]) return;
    if (warns == nil) warns = [NSMutableArray array];

    UITableViewCellAccessoryType oldType = [equippedCell accessoryType];
    NSString *warnText;
    warn_condition_t number = condition(&warnText);
    if (number == WARN_NONE) {
        [equippedCell setAccessoryType:oldType];
        [equippedCell setAccessoryView:nil];
        return; // Do nothing when condition is normal
    } else {
        WarnAccessoryView *button = [WarnAccessoryView warnAccessoryView];
        [equippedCell setAccessoryType:UITableViewCellAccessoryNone];
        [equippedCell setAccessoryView:button];
        equippedCell.detailTextLabel.textColor = [UIColor systemRedColor];
        

        if (warnText == nil) {
            switch (number) {
                case WARN_EMPTYVAL:
                    warnText = _("No value returned from sensor, device should be checked by service technician.");
                    break;
                case WARN_EXCEDDED:
                    warnText = _("Value exceeded than designed, device should be checked by service technician.");
                    break;
                case WARN_UNUSUAL:
                    warnText = _("Unusual value, device should be checked by service technician.");
                    break;
                case WARN_GENERAL:
                default:
                    warnText = _("Significant abnormal data, device should be checked by service technician.");
                    break;
            }
        }
        NSString *warn_strid = [NSString stringWithFormat:@"%@_%d", textLabel, number];
        if ([warns indexOfObject:warn_strid] == NSNotFound) {
            [warns addObjectsFromArray:@[warn_strid, warnText]];
        }
    }
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
        _("State Of Charge (UI)"), _("The \"Battery Percentage\" displayed exactly on your status bar. This is the SoC that Apple wants to tell you."),
        _("Battery Uptime"), _("The length of time the Battery Management System (BMS) has been up."),
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
    ];

    [self.tableView registerClass:[SegmentedViewCell class] forCellReuseIdentifier:@"HVC"];
    [self.tableView registerClass:[SegmentedFlagViewCell class] forCellReuseIdentifier:@"FLAGS"];
    [self.tableView registerClass:[MultilineViewCell class] forCellReuseIdentifier:_("Adapter Details")];
    self.tableView.estimatedRowHeight = 100;
    self.tableView.rowHeight = UITableViewAutomaticDimension;
}

- (instancetype)initWithBatteryInfo:(struct battery_info_node *)bi {
    if (@available(iOS 13.0, *)) {
        self = [super initWithStyle:UITableViewStyleInsetGrouped];
    } else {
        self = [super initWithStyle:UITableViewStyleGrouped];
    }
    self.tableView.allowsSelection = YES; // for now no ops specified it will just be stuck
    battery_info_update(bi, true);
    batteryInfo = bi;
    charging_stat = is_charging(&adapter_family, &adapter_info);

    return self;
}

- (void)updateTableView {
    [self.refreshControl beginRefreshing];
    battery_info_update(batteryInfo, true);
    charging_stat = is_charging(&adapter_family, &adapter_info);

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

        const char *adapter_family_str = NULL;
        if (adapter_family) {
            adapter_family_str = get_adapter_family_desc(adapter_family);
        }
        get_charger_data(&adapter_data);

        adapter_cells = [[NSMutableArray alloc] init];
        [adapter_cells addObjectsFromArray:@[
            @[_("Port"),                [NSString stringWithFormat:@"%d", adapter_info.port]],
            // This is terrible
            @[_("Compatibility"),       [NSString stringWithFormat:@"%@: %@\n%@: %@", _("External Connected"), (adapter_data.ChargerExist == 1) ? _("True") : _("False"), _("Charger Capable"), (adapter_data.ChargerCapable == 1) ? _("True") : _("False")]],
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
            @[_("Serial No."),          [NSString stringWithUTF8String:adapter_info.serial]],
            @[_("PMU Configuration"),   [NSString stringWithFormat:@"%u %@", adapter_info.PMUConfiguration, _("mA")]],
            @[_("Charger Configuration"),[NSString stringWithFormat:@"%u %@", adapter_data.ChargerConfiguration, _("mA")]],
            @[_("HVC Mode"),            @""], /* Special type, content controlled later */
        ]];
        if (adapter_data.NotChargingReason != 0) {
            [adapter_cells insertObject:@[_("Not Charging Reason"), [NSString stringWithUTF8String:not_charging_reason_str(adapter_data.NotChargingReason)]] atIndex:4];
        }
        if (adapter_info.port_type != 0) {
            [adapter_cells insertObject:@[_("Port Type"), _(port_type_str(adapter_info.port_type))] atIndex:1];
        }
    }
    /* TODO: Secondary Adapter & Accessory Adapter */
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

- (void)altAccTapped:(UIButton *)button {
    UIView *view = button;
    UITableViewCell *cell;

    UITableView *tv;
    NSIndexPath *ip;
    while (view && ![view isKindOfClass:[UITableViewCell class]]) {
        view = [view superview];
    }
    if (view) {
        cell = (UITableViewCell *)view;
        UIView *tb = view;
        while (tb && ![tb isKindOfClass:[UITableView class]]) {
            tb = [tb superview];
        }
        if (tb) {
            tv = (UITableView *)tb;
            ip = [tv indexPathForCell:cell];
            return [self tableView:tv accessoryButtonTappedForRowWithIndexPath:ip];
        }
    }
    DBGLOG(@"altAccTapped: Something goes wrong! view: %@, cell: %@, table: %@", view, cell, tv);
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

    if (ip.section == 0) {
        UITableViewCell *cell=[tv dequeueReusableCellWithIdentifier:@"bdvc:sect0"];
        cell.accessoryType=0;
        cell.accessoryView=nil;
        if(!cell)
        	cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"bdvc:sect0"];
        UILongPressGestureRecognizer *longPressRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleLongPress:)];
        [cell addGestureRecognizer:longPressRecognizer];

        struct battery_info_node *pending_bi = batteryInfo + ip.row + pendingLoadOffsets[ip.row];
        /* Flags special handler */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-compare"
        // _ID_ is defined as a number ID when not using Gettext
        if (pending_bi->description == _ID_("Flags") || ((uint64_t)pending_bi->description > 5000 && !strcmp(pending_bi->description, "Flags"))) {
            SegmentedFlagViewCell *cellf = [tv dequeueReusableCellWithIdentifier:@"FLAGS"];
            if (!cellf) cellf = [[SegmentedFlagViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"FLAGS"];
            cellf.textLabel.text = _(pending_bi->description);
            cellf.detailTextLabel.text = _(bi_node_get_string(pending_bi));
            cellf.titleLabel.text = _(pending_bi->description);
            cellf.detailLabel.text = _(bi_node_get_string(pending_bi));
            [cellf selectByFlags:gGauge.Flags];
            if (strlen(gGauge.DeviceName)) {
                [cellf setBitSetByModel:[NSString stringWithFormat:@"%s", gGauge.DeviceName]];
            } else {
                [cellf setBitSetByTargetName];
            }
            return cellf;
        }
#pragma clang diagnostic pop

        equipDetailCell(cell, pending_bi);
        /* Warning conditions */
        equipWarningCondition_b(cell, _("Remaining Capacity"), ^warn_condition_t(NSString **str){
            warn_condition_t code = WARN_NONE;
            uint16_t remain_cap, full_cap, design_cap;
            get_capacity(&remain_cap, &full_cap, &design_cap);
            if (remain_cap > full_cap) {
                code = WARN_UNUSUAL;
                *str = _("Unusual Remaining Capacity, A non-genuine battery component may be in use.");
            } else if (remain_cap == 0) {
                code = WARN_EMPTYVAL;
                *str = _("Remaining Capacity not detected.");
            }
            return code;
        });
        equipWarningCondition_b(cell, _("Cycle Count"), ^warn_condition_t(NSString **str){
            warn_condition_t code = WARN_NONE;
            int count, design;
            count = gGauge.CycleCount;
            design = gGauge.DesignCycleCount;

            if (gGauge.DesignCycleCount == 0) {
                // according to https://www.apple.com/batteries/service-and-recycling
                // Pre-iPhone15,3: 500, otherwise 1000
                // Watch*,* iPad*,*: 1000
                // iPod*,*: 400
                // MacBook**,*: 1000
                // AppleTV/Watch/AudioAccessory has no battery so ignored
                size_t size = 0;
                char machine[256];
                // Do not use uname()
                if (sysctlbyname("hw.machine", NULL, &size, NULL, 0) != 0) {
                    DBGLOG(@"sysctlbyname(hw.machine) failed");
                    return code;
                }
                if (sysctlbyname("hw.machine", &machine, &size, NULL, 0) != 0) {
                    DBGLOG(@"sysctlbyname(&machine) failed");
                    return code;
                }
                if (match_regex(machine, "^(iPhone|iPad|iPod|MacBook.*)[0-9]+,[0-9]+$")) {
                    if (strncmp(machine, "iPhone", 6) == 0) {
                        int major = 0, minor = 0;
                        if (sscanf(machine + 6, "%d,%d", &major, &minor) != 2) {
                            DBGLOG(@"Unexpected iPhone model: %s", machine);
                            return code;
                        }
                        if (major < 15 || (major == 15 && minor < 4)) {
                            design = 500;
                        } else {
                            design = 1000;
                        }
                    }
                    else if (strncmp(machine, "iPad", 4) || strncmp(machine, "Watch", 5) || strncmp(machine, "MacBook", 7)) design = 1000;
                    else if (strncmp(machine, "iPod", 4)) design = 400;
                }
                if (design == 0) return code;
            }
            /* TODO: iPhone batteries does not provide DesignCycleCount, get the data from Apple */
            if (count > design) {
                code = WARN_EXCEDDED;
                *str = _("Cycle Count exceeded designed cycle count, consider replacing with a genuine battery.");
            }
            return code;
        });
        WarnAccessoryView *button = (WarnAccessoryView *)[cell accessoryView];
        if (button != nil && [button isKindOfClass:[WarnAccessoryView class]]) {
            if (button.isWarn) {
                [button addTarget:self action:@selector(warnTapped:) forControlEvents:UIControlEventTouchUpInside];
            } else {
                [button addTarget:self action:@selector(altAccTapped:) forControlEvents:UIControlEventTouchUpInside];
            }
        }
        /* TODO: record 1st-read capacity data in defaults in order to observe battery problems */
    	return cell;
    }

    // Consider make this an adapter_info.c?
    if (ip.section == [sections_detail indexOfObject:_("Adapter Details")]) {
        UITableViewCell *cell = [tv dequeueReusableCellWithIdentifier:@"bdvc:addt"];
        if (!cell) {
            cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"bdvc:addt"];
        }

        NSArray *adapter_cell = adapter_cells[ip.row];
        cell.textLabel.text = adapter_cell[0];
        cell.detailTextLabel.text = adapter_cell[1];
        MultilineViewCell *celll = (MultilineViewCell *)cell;
        celll.titleLabel.text = adapter_cell[0];
        celll.detailLabel.text = ([adapter_cell[1] length] == 0) ? _("None") : adapter_cell[1];

        if ([desc_adap indexOfObject:cell.textLabel.text] != NSNotFound) {
            DBGLOG(@"Accessory %@, Index %lu", cell.textLabel.text, [desc_adap indexOfObject:cell.textLabel.text]);
            if (@available(iOS 13, *)) {
                cell.accessoryType = UITableViewCellAccessoryDetailButton;
            } else {
                WarnAccessoryView *button = [WarnAccessoryView altAccessoryView];
                [cell setAccessoryType:UITableViewCellAccessoryNone];
                [cell setAccessoryView:button];
                [button addTarget:self action:@selector(altAccTapped:) forControlEvents:UIControlEventTouchUpInside];
            }
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
                if (@available(iOS 13.0, *)) {
                    cell_seg.accessoryType = UITableViewCellAccessoryDetailButton;
                } else {
                    WarnAccessoryView *button = [WarnAccessoryView altAccessoryView];
                    [cell setAccessoryType:UITableViewCellAccessoryNone];
                    [cell setAccessoryView:button];
                    [button addTarget:self action:@selector(altAccTapped:) forControlEvents:UIControlEventTouchUpInside];
                }
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

- (void)hvcSegmentSelected:(UISegmentedControl *)segment {
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

- (void)warnTapped:(UIButton *)button {
    UIView *view = button;
    UITableViewCell *cell;
    while (view && ![view isKindOfClass:[UITableViewCell class]]) {
        view = [view superview];
    }
    if (view) {
        const char *title = NULL;
        cell = (UITableViewCell *)view;
        for (int i = 0; i < WARN_MAX; i++) {
            CFIndex index = [warns indexOfObject:[NSString stringWithFormat:@"%@_%d", cell.textLabel.text, i]];
            if (index != NSNotFound) {
                switch (i) {
                    case WARN_GENERAL:
                        title = _C("Error Data");
                        break;
                    case WARN_UNUSUAL:
                        title = _C("Unusual Data");
                        break;
                    case WARN_EXCEDDED:
                        title = _C("Data Too Large");
                        break;
                    case WARN_EMPTYVAL:
                        title = _C("Empty Data");
                        break;
                    default:
                        title = _C("Wrong Data");
                        break;
                }
                const char *content = [[warns objectAtIndex:index + 1] UTF8String];
                show_alert(title, content, _C("OK"));
                return;
            }
        }
    }
    DBGLOG(@"warnTapped: Something goes wrong! view: %@, cell: %@", view, cell);
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)handleLongPress:(UILongPressGestureRecognizer *)gestureRecognizer {
    UIPasteboard *pasteboard;
    NSString *pending;

    if (gestureRecognizer.state == UIGestureRecognizerStateBegan) {
        UITableViewCell *cell = (UITableViewCell *)gestureRecognizer.view;
        pending = cell.detailTextLabel.text;
        // special cases
        if ([[cell reuseIdentifier] isEqualToString:@"HVC"]) {
            SegmentedViewCell *cell_seg = (SegmentedViewCell *)cell;
            pending = cell_seg.detailLabel.text;
        }
        if ([[cell reuseIdentifier] isEqualToString:@"FLAGS"]) {
            SegmentedFlagViewCell *cellf = (SegmentedFlagViewCell *)cell;
            pending = cellf.detailLabel.text;
        }
        if ([[cell reuseIdentifier] isEqualToString:_("Adapter Details")]) {
            // Custom cells does not have detailTextLabel, thats how Apple desired
            MultilineViewCell *celll = (MultilineViewCell *)cell;
            pending = celll.detailLabel.text;
        }

        // We need better impl like PSTableCell's copy
        pasteboard = [UIPasteboard generalPasteboard];
        [pasteboard setString:pending];

        show_alert(_C("Copied!"), [pending UTF8String], _C("OK"));
    }
}

@end
