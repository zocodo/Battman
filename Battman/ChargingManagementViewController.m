#import "ChargingManagementViewController.h"
#include "battery_utils/libsmc.h"
#include "common.h"
#include "intlextern.h"

enum sections_cl {
	CM_SECT_GENERAL,
	CM_SECT_SMART_CHARGING,
	CM_SECT_LOW_POWER_MODE,
	CM_SECT_COUNT
};

static NSArray *sections_cl = nil;
static bool lpm_supported = true;
static bool lpm_on = false;

@implementation ChargingManagementViewController

- (NSString *)title {
	return _("Charging Management");
}

- (instancetype)init {
    if (@available(iOS 13.0, *)) {
        self = [super initWithStyle:UITableViewStyleInsetGrouped];
    } else {
        self = [super initWithStyle:UITableViewStyleGrouped];
    }
	//self.tableView.allowsSelection=NO;
	return self;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
	switch(sect) {
	case CM_SECT_GENERAL:
		return _("General");
	case CM_SECT_SMART_CHARGING:
		return _("Smart Charging");
	case CM_SECT_LOW_POWER_MODE:
		return _("Low Power Mode");
	}
	return nil;
}

- (NSString *)tableView:(UITableView *)tv titleForFooterInSection:(NSInteger)sect {
    if (sect == 0) {
        return _("Block Charging suspends battery charging and allows the battery to discharge while maintaining power source operation.");
    } else if (sect == 1) {
		return _("Smart Charging will start 900 seconds (15 minutes) after power is plugged-in, or the date you scheduled, whichever one comes first.");
    } else if (sect == 2) {
        NSUserDefaults *batterysaver_state = [[NSUserDefaults alloc] initWithSuiteName:@"com.apple.coreduetd.batterysaver.state"];
	void *mobileGestalt=dlopen("/usr/lib/libMobileGestalt.dylib",RTLD_LAZY);
	if(mobileGestalt) {
		bool (*MGGetBoolAnswer)(CFStringRef)=(bool(*)(CFStringRef))dlsym(mobileGestalt,"_MGGetBoolAnswer");
		if(MGGetBoolAnswer)
			lpm_supported = MGGetBoolAnswer(CFSTR("f+PE44W6AO2UENJk3p2s5A"));
		dlclose(mobileGestalt);
	}
        if (lpm_supported) {
            NSMutableString *finalStr = [[NSMutableString alloc] init];
            [batterysaver_state synchronize];

            /* State */
            id state = [batterysaver_state valueForKey:@"state"];
            lpm_on = [state boolValue];
            if (state)
                [finalStr appendString:lpm_on ? _("Enabled") : _("Disabled")];
            else
                return _("Never been used before");

            /* Date */
            id date = [batterysaver_state objectForKey:@"stateChangeDate"];
            if (date) {
                NSDateFormatter *fmt = [[NSDateFormatter alloc] init];
                fmt.locale = [NSLocale localeWithLocaleIdentifier:[NSString stringWithUTF8String:preferred_language()]];
//                fmt.locale = [NSLocale currentLocale];
                [fmt setLocalizedDateFormatFromTemplate:@"MMM ddHH:mm:ss"];
                NSString *strfmt = [NSString stringWithFormat:_(" since %@"), [fmt stringFromDate:date]];
                [finalStr appendString:strfmt];
            }

            /* at SoC */
            id soc = [batterysaver_state valueForKey:@"stateBatteryCharge"];
            if (soc) {
                double value = [soc doubleValue];
                NSString *strfmt = [NSString stringWithFormat:_(" at %d%% charge"), (unsigned int)(int)value];
                [finalStr appendString:strfmt];
            }
            return finalStr;
        } else {
            return _("Not supported on this device");
        }
    }
	return nil;
}

- (NSInteger)tableView:(UITableView *)tv numberOfRowsInSection:(NSInteger)sect {
	switch(sect) {
	case CM_SECT_GENERAL:
		return 2;
	case CM_SECT_SMART_CHARGING:
		return 3;
	case CM_SECT_LOW_POWER_MODE:
	default:
		return 1;
	}
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return CM_SECT_COUNT;
}

- (void)setBlockCharging:(UISwitch *)cswitch {
	BOOL val = cswitch.on;
    /* FIXME: kIOReturnNotPrivileged */
	int ret = smc_write_safe('CH0C', &val, 1);
    if(ret)
    	show_alert(_C("Failed"), _C("Something went wrong when setting this property."), _C("OK"));
    int new_val;
	smc_read_n('CH0C', &new_val, 1);
	new_val &= 0xFF;
	if ((new_val != 0) != val) {
		cswitch.on = (new_val != 0);
	}
}

- (void)setBlockPower:(UISwitch *)cswitch {
    BOOL val = cswitch.on;
    /* FIXME: kIOReturnNotPrivileged */
    int ret = smc_write_safe('CH0I', &val, 1);
    if(ret)
    	show_alert(_C("Failed"), _C("Something went wrong when setting this property."), _C("OK"));
    int new_val;
    smc_read_n('CH0I', &new_val, 1);
    new_val &= 0xFF;
    if ((new_val != 0) != val) {
        cswitch.on = (new_val != 0);
    }
}

- (void)setLPM:(UISwitch *)cswitch {
    BOOL val = cswitch.on;
    NSLog(@"%@abling Low Power Mode", val ? @"En" : @"Dis");

    NSError *err = nil;
    NSBundle *CDBundle = [NSBundle bundleWithPath:@"/System/Library/PrivateFrameworks/CoreDuet.framework"];
    if (![CDBundle loadAndReturnError:&err]) {
        NSString *errorMessage = [NSString stringWithFormat:@"%@ %@\n\n%@: %@", _("Failed to load"), @"CoreDuet.framework", _("Error"), [err localizedDescription]];
        show_alert(_C("Failed"), [errorMessage UTF8String], _C("OK"));
        return;
    }

    id CDClass = [CDBundle classNamed:@"_CDBatterySaver"];
    id CDObject = [CDClass batterySaver];
    /* 0 = Normal, 1 = LPM */
    [CDObject setPowerMode:val error:&err];
    BOOL now = [CDObject getPowerMode] & 1;
    if (now != val) {
        NSString *errorMessage = [NSString stringWithFormat:@"%@\n\n%@: %@", _("Unable to set Low Power Mode."), _("Error"), [err localizedDescription]];
        show_alert(_C("Failed"), [errorMessage UTF8String], _C("OK"));
    } else {
        NSLog(@"[batterySaver getPowerMode] = %lld", [CDObject getPowerMode]);
    }
    cswitch.on = now;
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if (indexPath.section == 1 && indexPath.row == 2) {
        NSError *err = nil;
		NSBundle *powerUIBundle = [NSBundle bundleWithPath:@"/System/Library/PrivateFrameworks/PowerUI.framework"];
		if (![powerUIBundle loadAndReturnError:&err]) {
            NSString *errorMessage = [NSString stringWithFormat:@"%@ %@\n\n%@: %@", _("Failed to load"), @"PowerUI.framework", _("Error"), [err localizedDescription]];
			show_alert(_C("Failed"), [errorMessage UTF8String], _C("OK"));
			goto tvend;
		}
		id sccClass = [powerUIBundle classNamed:@"PowerUISmartChargeClient"];
		id sccObject = [[sccClass alloc] initWithClientName:@"ok"];
		if(![sccObject setState:1 error:&err]) {
            NSString *errorMessage = [NSString stringWithFormat:@"%@\n\n%@: %@", _("Failed to enable Smart Charging."), _("Error"), [err localizedDescription]];
			show_alert(_C("Failed"), [errorMessage UTF8String], _C("OK"));
			goto tvend;
		}
		[sccObject engageFrom:fromPicker.date until:untilPicker.date repeatUntil:untilPicker.date overrideAllSignals:1];
		//BOOL yyy=1;
		//smc_write_safe('CH0C', &yyy, 1);
		show_alert(_C("Engaged"), _C("Smart Charging has been engaged. It will start after 15 minutes of power supply, or at the date you picked, whichever comes first."), _C("OK"));
	}
tvend:
	return [tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(UITableView *)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	UITableViewCell *cell = [UITableViewCell new];
	cell.selectionStyle = UITableViewCellSelectionStyleNone;
    if (indexPath.section == 0) {
        UISwitch *cswitch = [UISwitch new];
        int switchOn = 0;
        SEL action = nil;
        // TODO: Reduce redundant codes
        if (indexPath.row == 0) {
            cell.textLabel.text = _("Block Charging");
            smc_read_n('CH0C', &switchOn, 1);
            action = @selector(setBlockCharging:);
        } else if (indexPath.row == 1) {
            cell.textLabel.text = _("Block Power Supply");
            smc_read_n('CH0I', &switchOn, 1);
            action = @selector(setBlockPower:);
        }
        [cswitch addTarget:self action:action forControlEvents:UIControlEventValueChanged];
        cswitch.on = (switchOn & 0xff) != 0;
        cell.accessoryView = cswitch;
    } else if (indexPath.section == 1) {
		if (indexPath.row == 0) {
			cell.textLabel.text = _("Starting at");
			UIDatePicker *datePicker = [UIDatePicker new];
            // Default behavior determines locale by Region
            [datePicker setLocale:[NSLocale localeWithLocaleIdentifier:[NSString stringWithUTF8String:preferred_language()]]];
			datePicker.datePickerMode = UIDatePickerModeDateAndTime;
            if (@available(iOS 13.0, *)) {
                datePicker.date = [NSDate now];
            } else {
                datePicker.date = [NSDate date];
            }
			datePicker.minimumDate = datePicker.date;
			cell.accessoryView = datePicker;
			fromPicker = datePicker;
		} else if (indexPath.row == 1) {
			cell.textLabel.text = _("Until");
			UIDatePicker *datePicker = [UIDatePicker new];
            [datePicker setLocale:[NSLocale localeWithLocaleIdentifier:[NSString stringWithUTF8String:preferred_language()]]];
			datePicker.datePickerMode = UIDatePickerModeDateAndTime;
			datePicker.date = [NSDate dateWithTimeIntervalSinceNow:3600 * 5];
            if (@available(iOS 13.0, *)) {
                datePicker.minimumDate = [NSDate now];
            } else {
                datePicker.minimumDate = [NSDate date];
            }
			cell.accessoryView = datePicker;
			untilPicker = datePicker;
		} else {
			cell.selectionStyle = UITableViewCellSelectionStyleDefault;
			cell.textLabel.text = _("Schedule");
            if (@available(iOS 13.0, *)) {
                cell.textLabel.textColor = [UIColor linkColor];
            } else {
                cell.textLabel.textColor = [UIColor colorWithRed:0 green:(122.0f / 255) blue:1 alpha:1];
            }
		}
    } else if (indexPath.section == 2) {
        cell.textLabel.text = _("Low Power Mode");
        UISwitch *cswitch = [UISwitch new];
        cswitch.enabled = lpm_supported;
        /* This is not [_CDBatterySaver getPowerMode], which retrieves LPM state differntly */
        cswitch.on = lpm_on;
        [cswitch addTarget:self action:@selector(setLPM:) forControlEvents:UIControlEventValueChanged];
        cell.accessoryView = cswitch;
    }

    return cell;
}

@end
