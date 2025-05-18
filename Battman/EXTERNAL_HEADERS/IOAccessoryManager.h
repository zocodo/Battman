#ifndef _IOACCESSORYMANAGER_H
#define _IOACCESSORYMANAGER_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

/* The name of defs are guessed, since Apple has not exposed anything */

__BEGIN_DECLS

#define kIOAMUSBModeEnableHostMode	2

#define kIOAMFeatureHighCurrentPower	0xFFFF

#define kIOAMDetectOverrideAll		(uint64_t)-1

typedef CF_ENUM(UInt64, IOAMPinID) {
	kIOAMPinDP1,
	kIOAMPinDN1,
	kIOAMPinDP2,
	kIOAMPinDN2,

	kIOAMPinNone = 6,
};
typedef CF_ENUM(UInt64, IOAMMeas) {
	kIOAMBrickID,
	kIOAMAuxID,
};

typedef CF_ENUM(UInt8, IOAMIndex) {
	kIOAMConfigurePower = 0,			// uint64_t
	kIOAMConfigureUSBMode,				// uint64_t
	kIOAMConfigureUSBConnectActive,			// void
	kIOAMConfigureUSBCurrentLimitSetBase,		// uint64_t
	kIOAMConfigureUSBCurrentLimitRestoreBase,	// void
	kIOAMConfigureUSBCurrentLimitSetOffset = 5,	// uint64_t
	kIOAMConfigureUSBCurrentLimitSetMaximum,	// uint64_t
	kIOAMConfigureUSBCurrentLimitClearMaximum,	// void
	kIOAMConfigurePowerDuringSleep,			// uint64_t
	kIOAMConfigureBatteryPack,			// uint64_t
	kIOAMConfigureDetectOverride = 10,		// uint64_t[3]{1(?), YES/NO, acc_id}
	kIOAMConfigureDigitalIDOverride,		// uint64_t
	kIOAMConfigureAttachStyle,			// uint64_t
	kIOAMConfigureAllowedFeatures,			// uint64_t[2]{Allow, Revoke}
	kIOAMConfigureDPDNConfigOverride,		// uint64_t[4]{dpdn1B, dpdn2B, dpdn1T, dpdn2T}
	kIOAMConfigureUSBPowerIgnore = 15,		// uint64_t

	kIOAMCheckPinVoltage,				// uint64_t[2]{PinID, Meas} -> uint64_t

	kIOAMConfigurePowerLowVoltageSelect,		// uint64_t

	kIOAMRequestAccessoryPowerOff,			// void

	kIOAMSetGoldbond,				// uint64_t
	kIOAMConfigureGoldbondMode = 20,		// uint64_t[2]{setMask, clearMask}

	kIOAMRequestLdcmMeasurement,			// uint64_t
	kIOAMGetLdcmParams,				// void -> uint64_t[126]

	kIOAMSetVoltageDetected,			// uint64_t
	kIOAMChallengeCryptoDock9Pin,
	kIOAMChallengeCryptoDock = 25,
	kIOAMConfigureUSBPreventSourceDetection,	// uint64_t
	kIOAMSelectEisPin,				// uint64_t{Enable, PinID} -> uint64_t
	kIOAMGetFreePinMask,				// void -> uint64_t

	kIOAMLDCMGetAvailablePins,			// void -> uint64_t[2]
	kIOAMSetLDCM = 30,				// uint64_t[2] -> uint64_t[2]
	kIOAMLDCMSetLiquidDetected,			// uint64_t
	kIOAMLDCMEnableMitigations,			// uint64_t
	kIOAMLDCMEnableUserOverride,			// uint64_t
	kIOAMLDCMGetMeasurementStatus,			// void -> uint64_t
	kIOAMLDCMSetState,				// uint64_t
};

/*
enum {
    kIOAMConnectionTypeVirtual,
    kIOAMConnectionTypeLightning,
    kIOAMConnectionTypeBT,
    kIOAMConnectionTypeOrion,
    kIOAMConnectionTypeC26,
    kIOAMConnectionTypeIP,
    kIOAMConnectionTypeUSB,
    kIOAMConnectionTypeBuiltInHW,
    kIOAMConnectionTypeScorpius,
    kIOAMConnectionTypeInductive,
    kIOAMConnectionTypeNFC,
};

typedef CF_ENUM(SInt32, IOAMTransportType) {
    kIOAMTransportTypeUSBD,
    kIOAMTransportTypeMikeyBus,
    kIOAMTransportTypeBT,
    kIOAMTransportTypeBLE,
    kIOAMTransportTypeAirPlay,
    kIOAMTransportTypeGeneric,
    kIOAMTransportTypeUART,
    kIOAMTransportTypeAID,
    kIOAMTransportTypeUSBH,
    kIOAMTransportTypeScorpius,
    kIOAMTransportTypeBattery,
    kIOAMTransportTypeTouchController,
    kIOAMTransportTypeInductive,
    kIOAMTransportTypeNFC,
};
*/
typedef enum {
    kIOAMInterfaceDeviceSerialNumber = 1,   // CFNumberRef SInt64
    kIOAMInterfaceModuleSerialNumber,       // CFStringRef char[32]
    kIOAMAccessorySerialNumber,             // CFStringRef char[32]
    kIOAMAccessoryManufacturer,             // CFStringRef char[256]
    kIOAMAccessoryName,                     // CFStringRef char[256]
    kIOAMAccessoryModelNumber,              // CFStringRef char[256]
    kIOAMAccessoryFirmwareVersion,          // CFStringRef char[256]
    kIOAMAccessoryHardwareVersion,
    kIOAMAccessoryPPID                      // CFStringRef char[256]
} AccessoryInfo;

typedef enum {
	kIOAMPowermodeOff = 1,
	kIOAMPowermodeLow,
	kIOAMPowermodeOn,
	kIOAMPowermodeHighCurrent,
	kIOAMPowermodeHighCurrentBM3,
	kIOAMPowermodeLowVoltage,

	kIOAMPowermodeCount = kIOAMPowermodeLowVoltage,
} AccessoryPowermode;

SInt32 IOAccessoryPortGetPort(io_connect_t connect);
SInt32 IOAccessoryPortGetManagerPrimaryPort(io_connect_t connect);
SInt32 IOAccessoryPortGetTransportType(io_connect_t connect);
SInt32 IOAccessoryPortGetStreamType(io_connect_t connect);

io_service_t IOAccessoryManagerGetServiceWithPrimaryPort(SInt32 port);
SInt32 IOAccessoryManagerGetPrimaryPort(io_connect_t connect);
SInt32 IOAccessoryManagerGetType(io_connect_t connect);
SInt32 IOAccessoryManagerGetBatteryPackMode(io_connect_t connect);
io_registry_entry_t IOAccessoryManagerGetUpstreamService(io_connect_t connect);
SInt32 IOAccessoryManagerGetUpstreamPrimaryPort(io_connect_t connect);
SInt32 IOAccessoryManagerGetAccessoryID(io_connect_t connect);
CFNumberRef IOAccessoryManagerCopyAccessoryDeviceUID(io_connect_t connect);
IOReturn IOAccessoryManagerGetDigitalID(io_connect_t connect, UInt8 *bytes);
IOReturn IOAccesoryManagerGetDigitalIDSInt32erfaceDeviceInfo(io_connect_t connect, UInt8 *bytes);
IOReturn IOAccesoryManagerGetDigitalIDAccessoryVersionInfo(io_connect_t connect, UInt8 *bytes);
IOReturn IOAccessoryManagerCopyDeviceInfo(io_connect_t connect, AccessoryInfo infoID, CFTypeRef *a3);

SInt32 IOAccessoryManagerGetPowerMode(io_connect_t connect);
SInt32 IOAccessoryManagerGetActivePowerMode(io_connect_t connect);
unsigned long IOAccessoryManagerPowerModeCurrentLimit(io_connect_t connect, AccessoryPowermode mode);

bool IOAccessoryManagerPowerDuringSleepIsSupported(io_connect_t connect);
bool IOAccessoryManagerGetPowerDuringSleep(io_connect_t connect);
SInt32 IOAccessoryManagerGetSleepPowerCurrentLimit(io_connect_t connect);

IOReturn IOAccessoryManagerGetUSBConnectType(io_connect_t connect, SInt32 *type, bool *active);
IOReturn IOAccessoryManagerGetUSBConnectTypePublished(io_connect_t connect, SInt32 *published, bool *active);
IOReturn IOAccessoryManagerGetUSBChargingVoltage(io_connect_t connect, SInt32 *voltage);
IOReturn IOAccessoryManagerGetUSBCurrentLimit(io_connect_t connect, SInt32 *ilim);
IOReturn IOAccessoryManagerSetUSBCurrentLimitBase(io_connect_t connect, uint64_t input);
IOReturn IOAccessoryManagerRestoreUSBCurrentLimitBase(io_connect_t connect);
IOReturn IOAccessoryManagerGetUSBCurrentLimitBase(io_connect_t connect, SInt32 *ilimBase);
IOReturn IOAccessoryManagerSetUSBCurrentOffset(io_connect_t connect, SInt32 offset);
IOReturn IOAccessoryManagerGetUSBCurrentLimitOffset(io_connect_t connect, SInt32 *offset);
IOReturn IOAccessoryManagerSetUSBCurrentLimitMaximum(io_connect_t connect, uint64_t ilimMax);
IOReturn IOAccessoryManagerClearUSBCurrentLimitMaximum(io_connect_t connect);

__END_DECLS

#endif
