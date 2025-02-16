#import "BatteryDetailsViewController.h"
#include "battery_utils/libsmc.h"
#include "common.h"

// TODO: Function for advanced users to call SMC themselves.
// or add them to tracklist
NSArray *sections;
// TODO: Config
NSTimeInterval reload_interval = 5.0;


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
//    [self updateTableView]; // Why SIGABRT here?
    /* We knows too less to listen on SMC events */
    (void)[NSTimer scheduledTimerWithTimeInterval:reload_interval
                                           target:self
                                         selector:@selector(updateTableView)
                                         userInfo:nil
                                          repeats:YES];
}

- (instancetype)initWithBatteryInfo:(struct battery_info_node *)bi {
    self = [super initWithStyle:UITableViewStyleGrouped];
    self.tableView.allowsSelection = NO; // for now no ops specified it will just be stuck
    battery_info_update(bi, true);
    batteryInfo = bi;
        /* Don't remove this, otherwise users will blame us */
        NSString *gauge_disclaimer = _("All Gas Gauge metrics are dynamically retrieved from the onboard sensor array in real time. Should anomalies be detected in specific readings, this may indicate the presence of unauthorized components or require diagnostics through Apple Authorised Service Provider.");
        NSString *explaination_IT = (gGauge.ITMiscStatus != 0) ? [NSString stringWithFormat:@"\n\n%@", _("The \"IT Misc Status\" field refers to the miscellaneous data returned by battery Impedance Track™ Gas Gauge IC.")] : @"";
        NSString *explaination_Sim = (gGauge.SimRate != 0) ? [NSString stringWithFormat:@"\n\n%@", _("The \"Simulation Rate\" field refers to the rate of battery performing Impedance Track™ simulations.")] : @"";
        gasGaugeDisclaimer = [NSString stringWithFormat:@"%@%@%@", gauge_disclaimer, explaination_IT, explaination_Sim];
    
    return self;
}

- (void)updateTableView {
    battery_info_update(batteryInfo, true);
#if 0
    /* Gas Gauge Section */
    {
        get_capacity(&b_remaining_capacity, &b_full_charge_capacity, &b_designed_capacity);
        get_gas_gauge(&gauge);

        char buf[24];
        NSArray *basic_gas = @[
            @[_("Full Charge Capacity"),    @"%@ mAh", @(b_full_charge_capacity)],
            @[_("Designed Capacity"),       @"%@ mAh", @(b_designed_capacity)],
            @[_("Remaining Capacity"),      @"%@ mAh", @(b_remaining_capacity)],
            @[_("Qmax"),                    @"%@ mAh", @(gauge.Qmax * battery_num())],
            @[_("Depth of Discharge"),      @"%@ mAh", @(gauge.DOD0)],
            @[_("Passed Charge"),           @"%@ mAh", @(gauge.PassedCharge)],
            @[_("Voltage"),                 @"%@ mV",  @(gauge.Voltage)],
            @[_("Temperature"),             @"%@ °C",  @(get_temperature())],
            @[_("Average Current"),         @"%@ mA",  @(gauge.AverageCurrent)],
            @[_("Average Power"),           @"%@ mW",  @(gauge.AveragePower)],
            @[_("Battery Count"),           @"%@",     @(battery_num())],
            @[_("Time To Empty"),           @"%@",     (get_time_to_empty() == -1) ? _("Never") : [NSString stringWithFormat:@"%d %@", get_time_to_empty(), _("Minutes")]],
            @[_("Cycle Count"),             @"%@",     @(gauge.CycleCount)],
            @[_("State Of Charge"),         @"%@%%",   @(gauge.StateOfCharge)],
            @[_("Resistance Scale"),        @"%@",     @(gauge.ResScale)],
            @[_("Battery Serial"),          @"%@",     (battery_serial(buf) ? [NSString stringWithCString:buf encoding:NSUTF8StringEncoding] : _("None"))],
            @[_("Chemistry ID"),            @"%@",     [NSString stringWithFormat:@"0x%.8X", gauge.ChemID]],
            @[_("Flags"),                   @"%@",     [NSString stringWithFormat:@"0x%.4X", gauge.Flags]]
        ];
        gas_gauge_row = [basic_gas mutableCopy];

        /* Not every device sets this, only show when do */
        if (gauge.TrueRemainingCapacity != 0) {
            /* After Remaining Capacity */
            [gas_gauge_row insertObject:@[_("True Remaining Capacity"), @"%d mAh", @(gauge.TrueRemainingCapacity * battery_num())] atIndex:3];
        }
        /* OCV parameters only set when OCV */
        if (gauge.OCV_Current != 0) {
            [gas_gauge_row addObject:@[_("OCV Current"), @"%d mA", @(gauge.OCV_Current)]];
        }
        if (gauge.OCV_Voltage != 0) {
            [gas_gauge_row addObject:@[_("OCV Voltage"), @"%d mV", @(gauge.OCV_Voltage)]];
        }
        /* IMAX and IMAX2 not always set */
        if (gauge.IMAX != 0) {
            [gas_gauge_row addObject:@[_("Peak Current"), @"%d mA", @(gauge.IMAX)]];
        }
        if (gauge.IMAX2 != 0) {
            [gas_gauge_row addObject:@[_("Peak Current 2"), @"%d mA", @(gauge.IMAX2)]];
        }
        /* IT not always exist (at least not on Macs) */
        if (gauge.ITMiscStatus != 0) {
            [gas_gauge_row addObject:@[_("IT Misc Status"), @"%@", [NSString stringWithFormat:@"0x%.4X", gauge.ITMiscStatus]]];
        }
        if (gauge.SimRate != 0) {
            [gas_gauge_row addObject:@[_("Simulation Rate"), @"%@ Hr", @(gauge.SimRate)]];
        }
        /* ResScale also IT, consider add it too */
    }

    /* Adapter Details Section */
    /* TODO: Get this directly from AppleSMC via is_charging, not IOPS */
    /* Simulator won't be able to get this thing */
    extern CFDictionaryRef IOPSCopyExternalPowerAdapterDetails(void); // Avoid include
    NSDictionary *IOPSAdapter = (__bridge NSDictionary *)IOPSCopyExternalPowerAdapterDetails();
    if (IOPSAdapter != nil) {
        NSArray *basic_adap = @[
            @[_("Adapter Name"),     @"%@",    [IOPSAdapter valueForKey:@"Name"]],
            @[_("Adapter Serial"),   @"%@",    [IOPSAdapter valueForKey:@"SerialString"]],
            @[_("Manufacturer"),     @"%@",    [IOPSAdapter valueForKey:@"Manufacturer"]],
            @[_("Description"),      @"%@",    [IOPSAdapter valueForKey:@"Description"]],
            @[_("Adapter ID"),       @"%@",    [IOPSAdapter valueForKey:@"AdapterID"]],
            @[_("Hardware Version"), @"%@",    [IOPSAdapter valueForKey:@"HwVersion"]],
            @[_("Firmware Version"), @"%@",    [IOPSAdapter valueForKey:@"FwVersion"]],
            @[_("Family Code"),      @"%@",    [IOPSAdapter valueForKey:@"FamilyCode"]],
            @[_("Is Wireless"),      @"%@",    [IOPSAdapter valueForKey:@"IsWireless"]],
            @[_("Watts"),            @"%@ W",  [IOPSAdapter valueForKey:@"Watts"]],
            @[_("Current"),          @"%@ mA", [IOPSAdapter valueForKey:@"Current"]],
            @[_("Voltage"),          @"%@ mV", [IOPSAdapter valueForKey:@"Voltage"]],
            @[_("Adapter Model"),    @"%@",    [IOPSAdapter valueForKey:@"Model"]],
            /* TODO: Parse PMUConfiguration */
            @[_("PMU Config Flags"), @"%@",    [IOPSAdapter valueForKey:@"PMUConfiguration"]]

            // UsbHvcMenu => D?PM
            // UsbHvcHvcIndex => D?PI
        ];
        adapter_row = [basic_adap mutableCopy];
    }

    sections = [NSMutableArray arrayWithArray:@[
        @[_("Gas Gauge"), gas_gauge_row]
    ]];
    if (IOPSAdapter != nil) [sections addObject:@[_("Adapter Details"), adapter_row]];
#endif
    /* Dynasects */
    /* TODO: Handle the scene that if battery not present */
#if !__has_feature(objc_arc)
    [sections dealloc];
#endif
    sections = [NSMutableArray arrayWithArray:@[_("Gas Gauge")]];
    /* TODO: */
    // if (is_charging(NULL, NULL))
    //     sections = [NSMutableArray addObject:@[_("Adapter Details")]];
    [self.tableView reloadData];
}

#if 0
// Init was not executed somehow
- (instancetype)init {
    self = [super initWithStyle:UITableViewStyleGrouped];
    self.tableView.allowsSelection = YES;
    [self updateTableView];

    return self;
}
#endif

- (NSString *)tableView:(id)tv titleForHeaderInSection:(NSInteger)section {
    // Doesn't matter, it will be changed by willDisplayHeaderView
    return @"This is a Title yeah";
}

- (void)tableView:(UITableView *)tableView willDisplayHeaderView:(UIView *)view forSection:(NSInteger)section {
    UITableViewHeaderFooterView *header = (UITableViewHeaderFooterView *)view;
    header.textLabel.text = sections[section];
}

- (NSString *)tableView:(UITableView *)tableView
    titleForFooterInSection:(NSInteger)section {
    if (section == 0) {
    	return gasGaugeDisclaimer;
    }
    return nil;
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	if (section == 0) {
		int rows = 0;
		for (struct battery_info_node *i = batteryInfo; i->description; i++) {
			if ((i->content & BIN_DETAILS_SHARED) == BIN_DETAILS_SHARED ||
				(i->content && !((i->content & BIN_IS_SPECIAL) == BIN_IS_SPECIAL))) {
				if((i->content&1)!=1||(i->content&(1<<5))!=1<<5) {
					pendingLoadOffsets[rows]=(unsigned char)((i-batteryInfo)-rows);
					rows++;
				}
			}
		}
		pendingLoadOffsets[rows]=255;
		return rows;
	}
	return 0;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
    return sections.count;
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
