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

typedef enum {
        kInterfaceDeviceSerialNumber,
        kInterfaceModuleSerialNumber,
        kAccessorySerialNumber,
        kAccessoryManufacturer,
        kAccessoryName,
        kAccessoryModelNumber,
        kAccessoryFirmwareVersion,
        kAccessoryHardwareVersion,
        kAccessoryPPID
} AccessoryInfo;

io_service_t IOAccessoryManagerGetServiceWithPrimaryPort(SInt32 port);
SInt32 IOAccessoryManagerGetPrimaryPort(io_registry_entry_t entry);
SInt32 IOAccessoryManagerGetType(io_registry_entry_t entry);
io_registry_entry_t IOAccessoryManagerGetUpstreamService(io_registry_entry_t entry);
SInt32 IOAccessoryManagerGetUpstreamPrimaryPort(io_registry_entry_t entry);
SInt32 __fastcall IOAccessoryManagerGetAccessoryID(io_registry_entry_t entry);
CFNumberRef __fastcall IOAccessoryManagerCopyAccessoryDeviceUID(io_registry_entry_t entry);
IOReturn __fastcall IOAccessoryManagerGetDigitalID(io_registry_entry_t entry, UInt8 *bytes);
IOReturn __fastcall IOAccesoryManagerGetDigitalIDSInt32erfaceDeviceInfo(io_registry_entry_t entry, UInt8 *bytes);
IOReturn __fastcall IOAccesoryManagerGetDigitalIDAccessoryVersionInfo(io_registry_entry_t entry, UInt8 *bytes);
IOReturn __fastcall IOAccessoryManagerCopyDeviceInfo(io_registry_entry_t entry, AccessoryInfo infoID, CFTypeRef *a3);
SInt32 __fastcall IOAccessoryManagerGetPowerMode(io_registry_entry_t entry);
SInt32 __fastcall IOAccessoryManagerGetActivePowerMode(io_registry_entry_t entry);
SInt32 __fastcall IOAccessoryManagerGetSleepPowerCurrentLimit(io_registry_entry_t entry);
bool __fastcall IOAccessoryManagerGetPowerDuringSleep(io_registry_entry_t entry);
IOReturn __fastcall IOAccessoryManagerGetUSBConnectType(io_registry_entry_t entry, SInt32 *type, bool *active);
IOReturn __fastcall IOAccessoryManagerGetUSBConnectTypePublished(io_registry_entry_t entry, SInt32 *published, bool *active);
IOReturn __fastcall IOAccessoryManagerGetUSBChargingVoltage(io_registry_entry_t entry, SInt32 *voltage);
IOReturn __fastcall IOAccessoryManagerGetUSBCurrentLimit(io_registry_entry_t entry, SInt32 *ilim);
IOReturn __fastcall IOAccessoryManagerSetUSBCurrentLimitBase(io_registry_entry_t entry, uint64_t input);
IOReturn __fastcall IOAccessoryManagerRestoreUSBCurrentLimitBase(io_registry_entry_t entry);
IOReturn __fastcall IOAccessoryManagerGetUSBCurrentLimitBase(io_registry_entry_t entry, SInt32 *ilimBase);
IOReturn __fastcall IOAccessoryManagerSetUSBCurrentOffset(io_registry_entry_t entry, SInt32 offset);
IOReturn __fastcall IOAccessoryManagerGetUSBCurrentLimitOffset(io_registry_entry_t entry, SInt32 *offset);
IOReturn __fastcall IOAccessoryManagerSetUSBCurrentLimitMaximum(io_registry_entry_t entry, uint64_t ilimMax);
IOReturn __fastcall IOAccessoryManagerClearUSBCurrentLimitMaximum(io_registry_entry_t entry);

__END_DECLS

#endif
