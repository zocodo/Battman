#include "accessory.h"
#include "../common.h"
//#include "intlextern.h"
#include <mach/mach.h>

#if TARGET_OS_SIMULATOR
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

io_iterator_t gAccessories;
io_service_t gAccPrimary;

const char *acc_id_0_f[] = {
	"3K: Simple dock (beep on insert)",
	"10K: FireWire card reader",
	"18K: (Reserved/unused)",
	"28K: USB device accessory",
	"39K: (Reserved/unused)",
	"52K: (Reserved/unused)",
	"68K: Line out on/speaker off (MFP-compat, A63)",
	"88K: Diagnostics dock",
	"113K: (Reserved/unused)",
	"147K: (Reserved/unused)",
	"191K: USB iAP HID alternate config",
	"255K: Battery pack (no iPod charge)",
	"360K: Line out off, echo cancel on (MFP-compat, A63)",
	"549K: 30 pin connector serial device",
	"1000K: Car charger (pause on detach)",
	"3010K: Acc detect grounded but no resistor ID)",
};

const char *acc_id_50_5e[] = {
	"USBC: USB Host only",
	"USBC: USB Device only",
	"USBC: USB Host + DP display",
	"USBC: USB Device + DP display",
	"USBC: DP display only",
	"USBC: Snk current only",
	"USBC: Src current only",
	"USBC: Debug only",
	"USBC: Plugged other",
	NULL,
	"Digital ID: not found",
	"Digital ID", // This will need IOAccessoryManagerGetDigitalID()
	"Digital ID: unsupported",
	"Digital ID: unreliable",
	"Digital ID: reversed orientation",
};

/* Sadly I didn't got the full list of accids, but we can guess */
// 62: MagSafe Charger
// 64: MagSafe Battery
const char *acc_id_string(SInt32 accid) {
	static char idstr[256];

	if (accid < 16) sprintf(idstr, "%d\n%s", accid, acc_id_0_f[accid]);
	if (95 > accid && accid > 79 && accid != 89) {
		sprintf(idstr, "%d\n(%s)", accid, acc_id_50_5e[accid]);
	}
	if (accid == 70) sprintf(idstr, "%d\n%s", accid, "Scorpius: unknown");
	if (accid == 71) sprintf(idstr, "%d\n%s", accid, "Scorpius: pencil");

	if (strlen(idstr)) {
		return idstr;
	}

	// Otherwise unknown
	sprintf(idstr, "%d", accid);
	return idstr;
}

#ifdef _C
#undef _C
#endif
#define _C(x) x
static char *acc_powermodes[] = {
	_C("Off"),
	_C("Low"),
	_C("On"),
	_C("High Current"),
	_C("High Current (BM3)"),
	_C("Low Voltage"),
};
#undef _C
extern const char *cond_localize_c(const char *);
#define _C(x) cond_localize_c(x)

const char *acc_powermode_string(AccessoryPowermode powermode) {
	static char modestr[32];
	// IOAM modes are starting form 1
	if ((powermode - 1) < kIOAMPowermodeCount) {
		return _C(acc_powermodes[powermode - 1]);
	}

	snprintf(modestr, 32, "<%d>", powermode);
	return modestr;
}

const char *acc_powermode_string_supported(accessory_powermode_t mode) {
	if (mode.supported_cnt == 0) return NULL;

	static char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s: ", _C("Supported List"));
	for (size_t i = 0; i < mode.supported_cnt; i++) {
		sprintf(buffer, "%s%s<%lu %s>\n", buffer, acc_powermode_string(mode.supported[i]), mode.supported_lim[i], L_MA);
	}

	return buffer;
}

const char *manf_id_string(SInt32 manf) {
	switch (manf) {
		// retrieve from online db? or just common vids?
		case 0x05AC: return "Apple Inc.";
		default: break;
	}
	return NULL;
}

/* Make sure logics are not directly called in the UI */
#pragma mark - IOAccessoryMananger

io_iterator_t IOAccessoryManagerGetServices(void) {
	io_iterator_t services = MACH_PORT_NULL;
	IOReturn kr = kIOReturnSuccess;

	kr = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOAccessoryManager"), &services);
	if (kr != kIOReturnSuccess) {
		DBGLOG(CFSTR("Cannot open IOAccessoryManager (0x%X)"), kr);
	}

	return services;
}

/* Different from AppleSMC calls, we are going to get services quite often */

/* TODO: Runtime check of Sim IOAM existence */

io_service_t acc_open_with_port(int port) {
#if TARGET_OS_SIMULATOR
	return MACH_PORT_NULL;
#endif

	/*
	IOReturn kr = kIOReturnSuccess;
	io_connect_t connect = MACH_PORT_NULL;

	io_service_t service = IOAccessoryManagerGetServiceWithPrimaryPort(port);
	if (!service) NSLog(CFSTR("could not find IOAccessoryManager service for port %d"), port);

	kr = IOServiceOpen(service, mach_task_self(), 0, &connect);
	if (kr != kIOReturnSuccess) {
		NSLog(CFSTR("could not open IOAccessoryManager service: %s"), mach_error_string(kr));
	}

	return connect;
*/
	return IOAccessoryManagerGetServiceWithPrimaryPort(port);
}

SInt32 get_accid(io_connect_t connect) {
#if TARGET_OS_SIMULATOR
	return 100;
#else
	SInt32 accid = IOAccessoryManagerGetAccessoryID(connect);
	DBGLOG(CFSTR("accid: %d"), accid);
	return accid;
#endif
}

SInt32 get_acc_battery_pack_mode(io_connect_t connect) {
#if TARGET_OS_SIMULATOR
	return 0;
#else
	return IOAccessoryManagerGetBatteryPackMode(connect);
#endif
}

SInt32 get_acc_allowed_features(io_connect_t connect) {
#if TARGET_OS_SIMULATOR
	return -1;
#endif
	SInt32 buffer = -1;
	CFNumberRef AllowedFeatures;

	AllowedFeatures = IORegistryEntryCreateCFProperty(connect, CFSTR("IOAccessoryManagerAllowedFeatures"), kCFAllocatorDefault, kNilOptions);
	if (AllowedFeatures) {
		if (!CFNumberGetValue(AllowedFeatures, kCFNumberSInt32Type, &buffer)) {
			DBGLOG(CFSTR("get_allowed_features: Invalid"));
		}
	} else {
		DBGLOG(CFSTR("get_allowed_features: None"));
	}
	if (AllowedFeatures) CFRelease(AllowedFeatures);

	return buffer;
}

typedef struct {
	AccessoryInfo   key;
	void           *dest;
	size_t          len;
} AccQuery;
accessory_info_t get_acc_info(io_connect_t connect) {
	IOReturn     kr;
	CFTypeRef    buffer;
	accessory_info_t info;
	
	memset(&info, 0, sizeof(info));
#if !TARGET_OS_SIMULATOR
	AccQuery queries[] = {
		{kIOAMAccessorySerialNumber,    info.serial, sizeof(info.serial)},
		{kIOAMAccessoryManufacturer,    info.vendor, sizeof(info.vendor)},
		{kIOAMAccessoryName,            info.name,   sizeof(info.name)},
		{kIOAMAccessoryModelNumber,     info.model,  sizeof(info.model)},
		{kIOAMAccessoryFirmwareVersion, info.fwVer,  sizeof(info.fwVer)},
		{kIOAMAccessoryHardwareVersion, info.hwVer,  sizeof(info.hwVer)},
		{kIOAMAccessoryPPID,            info.PPID,   sizeof(info.PPID)},
	};
	
	for (size_t i = 0; i < sizeof(queries) / sizeof(queries[0]); i++) {
		kr = IOAccessoryManagerCopyDeviceInfo(connect, queries[i].key, &buffer);
		if (kr != kIOReturnSuccess) {
			NSLog(CFSTR("get_acc_info(%d): %s"), queries[i].key, mach_error_string(kr));
			continue;
		}

		memset(queries[i].dest, 0, queries[i].len);

		if (!CFStringGetCString((CFStringRef)buffer, queries[i].dest, queries[i].len, kCFStringEncodingUTF8)) {
			NSLog(CFSTR("get_acc_info(%d): CF Error"), queries[i].key);
			continue;
		}
		DBGLOG(CFSTR("get_acc_info(%d): got %s"), queries[i].key, (char *)queries[i].dest);
		if (buffer) CFRelease(buffer);
	}
#endif
	return info;
}

accessory_powermode_t get_acc_powermode(io_connect_t connect) {
	accessory_powermode_t mode;
	CFArrayRef supported;

	memset(&mode, 0, sizeof(mode));
#if !TARGET_OS_SIMULATOR
	mode.mode = IOAccessoryManagerGetPowerMode(connect);
	mode.active = IOAccessoryManagerGetActivePowerMode(connect);

	supported = IORegistryEntryCreateCFProperty(connect, CFSTR("IOAccessorySupportedPowerModes"), kCFAllocatorDefault, kNilOptions);
	if (supported) {
#if DEBUG
		CFShow(supported);
#endif
		mode.supported_cnt = CFArrayGetCount(supported);
		for (int i = 0; i < mode.supported_cnt; i++) {
			CFNumberRef value = CFArrayGetValueAtIndex(supported, i);
			if (CFNumberGetValue(value, kCFNumberSInt32Type, &mode.supported[i])) {
				mode.supported_lim[i] = IOAccessoryManagerPowerModeCurrentLimit(connect, mode.supported[i]);
			}
			if (value) CFRelease(value);
		}
	}
	if (supported) CFRelease(supported);
#endif
	return mode;
}

accessory_sleeppower_t get_acc_sleeppower(io_connect_t connect) {
	accessory_sleeppower_t sleep;

	memset(&sleep, 0, sizeof(sleep));
#if !TARGET_OS_SIMULATOR
	sleep.supported = IOAccessoryManagerPowerDuringSleepIsSupported(connect);
	sleep.enabled = IOAccessoryManagerGetPowerDuringSleep(connect);
	sleep.limit = IOAccessoryManagerGetSleepPowerCurrentLimit(connect);
#endif
	return sleep;
}

bool get_acc_supervised(io_connect_t connect) {
#if TARGET_OS_SIMULATOR
	return false;
#endif
	CFBooleanRef supervised;

	supervised = IORegistryEntryCreateCFProperty(connect, CFSTR("SupervisedAccessoryAttached"), kCFAllocatorDefault, kNilOptions);

	bool ret = (supervised == kCFBooleanTrue);
	if (supervised) CFRelease(supervised);

	return ret;
}

bool get_acc_supervised_transport_restricted(io_connect_t connect) {
#if TARGET_OS_SIMULATOR
	return false;
#endif
	CFBooleanRef restricted;
	
	restricted = IORegistryEntryCreateCFProperty(connect, CFSTR("SupervisedTransportsRestricted"), kCFAllocatorDefault, kNilOptions);
	
	bool ret = (restricted == kCFBooleanTrue);
	if (restricted) CFRelease(restricted);
	
	return ret;
}

#if TARGET_OS_SIMULATOR
#pragma clang diagnostic pop
#endif
