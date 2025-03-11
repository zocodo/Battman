/* Copyright (c) 2024 Torrekie Gen <me@torrekie.dev>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "libsmc.h"
#include "intlextern.h"
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <string.h>
#include <stdlib.h>
#if __has_include(<mach/mach.h>)
#include <mach/mach.h>
#else
typedef unsigned int mach_port_t;
extern mach_port_t mach_task_self_;
#define mach_task_self() mach_task_self_
#endif
#if __has_include(<IOKit/IOKitLib.h>)
#include <IOKit/IOKitLib.h>
#else
typedef mach_port_t io_service_t;
typedef mach_port_t io_connect_t;
typedef mach_port_t io_object_t;
typedef mach_port_t task_port_t;
typedef int IOReturn;
typedef int kern_return_t;
#define kIOReturnSuccess 0
#define MACH_PORT_NULL 0
IOReturn IOMasterPort(mach_port_t, mach_port_t *);
CFMutableDictionaryRef IOServiceMatching(const char *);
io_service_t IOServiceGetMatchingService(mach_port_t, CFDictionaryRef);
kern_return_t IOServiceOpen(io_service_t, task_port_t, uint32_t,
                            io_connect_t *);
kern_return_t IOConnectCallStructMethod(mach_port_t, uint32_t, const void *,
                                        size_t, void *, size_t *);
kern_return_t IOServiceClose(io_service_t);
#endif
#if __has_include(<IOKit/pwr_mgt/IOPM.h>)
#include <IOKit/pwr_mgt/IOPM.h>
#else
enum {
    kIOPSFamilyCodeDisconnected = 0,
    kIOPSFamilyCodeUnsupported  = E00002C7,

    kIOPSFamilyCodeFirewire     = E0008000,

    kIOPSFamilyCodeUSBHost      = E0004000,
    kIOPSFamilyCodeUSBHostSuspended,
    kIOPSFamilyCodeUSBDevice,
    kIOPSFamilyCodeUSBAdapter,
    kIOPSFamilyCodeUSBChargingPortDedicated,
    kIOPSFamilyCodeUSBChargingPortDownstream,
    kIOPSFamilyCodeUSBChargingPort,
    kIOPSFamilyCodeUSBUnknown,
    kIOPSFamilyCodeUSBCBrick,
    kIOPSFamilyCodeUSBCTypeC,
    kIOPSFamilyCodeUSBCPD,
    kIOPSFamilyCodeAC           = E0024000,
    kIOPSFamilyCodeExternal,
    kIOPSFamilyCodeExternal2,
    kIOPSFamilyCodeExternal3,
    kIOPSFamilyCodeExternal4,
    kIOPSFamilyCodeExternal5,
    kIOPSFamilyCodeExternal6,
    kIOPSFamilyCodeExternal7
};
#endif

#ifdef DEBUG
#define DBGALT(x, y, z) show_alert(x, y, z)
#define DBGLOG(...) NSLog(__VA_ARGS__)
#else
#define DBGALT(x, y, z)
#define DBGLOG(...)
#endif

extern bool show_alert(char *, char *, char *);
extern void show_alert_async(char *, char *, char *, void (^)(bool));
extern void app_exit(void);
extern void NSLog(CFStringRef, ...);

static io_service_t gConn = 0;
gas_gauge_t gGauge = {0};

__attribute__((constructor))
static IOReturn smc_open(void) {
    IOReturn result;
    mach_port_t masterPort;
    io_service_t service;

    if (IOMasterPort(MACH_PORT_NULL, &masterPort) != kIOReturnSuccess) {
        DBGLOG(CFSTR("IOMasterPort() failed"));
        return 1;
    }

    service = IOServiceGetMatchingService(masterPort, IOServiceMatching("AppleSMC"));
    result = IOServiceOpen(service, mach_task_self(), 0, &gConn);
    if (result != kIOReturnSuccess) {
        static dispatch_once_t token;
        dispatch_once(&token, ^{
            /* TODO: Check entitlements and explicitly warn which we loss */
            show_alert_async(_C("AppleSMC Open Failed"), _C("This typically means you did not install Battman with correct entitlements, please reinstall by checking instructions at https://github.com/Torrekie/Battman"), _C("OK"), ^(bool res) {
                app_exit();
            });
        });
        DBGLOG(CFSTR("IOServiceOpen() failed (%d)"), result);
        return result;
    }

    return kIOReturnSuccess;
}

static IOReturn smc_call(int index, SMCParamStruct *inputStruct,
                         SMCParamStruct *outputStruct) {
    size_t inputSize, outputSize;

    inputSize = sizeof(SMCParamStruct);
    outputSize = sizeof(SMCParamStruct);

    return IOConnectCallStructMethod(gConn, index, inputStruct, inputSize,
                                     outputStruct, &outputSize);
}

static IOReturn smc_get_keyinfo(UInt32 key, SMCKeyInfoData *keyInfo) {
    SMCParamStruct inputStruct;
    SMCParamStruct outputStruct;
    IOReturn result = kIOReturnSuccess;

    memset(&inputStruct, 0, sizeof(inputStruct));
    memset(&outputStruct, 0, sizeof(outputStruct));

    inputStruct.key = key;
    inputStruct.param.data8 = kSMCGetKeyInfo;

    result = smc_call(kSMCHandleYPCEvent, &inputStruct, &outputStruct);
    if (result == kIOReturnSuccess) {
        *keyInfo = outputStruct.param.keyInfo;
    }

    // Important check: dataSize != 0
    // idk why a nonexist key could return kIOReturnSuccess
    if (outputStruct.param.keyInfo.dataSize == 0)
        result = kIOReturnError;

    return result;
}

static IOReturn smc_read(UInt32 key, void *bytes) {
    IOReturn result;
    SMCParamStruct inputStruct;
    SMCParamStruct outputStruct;
    SMCKeyInfoData keyInfo;

    memset(&inputStruct, 0, sizeof(inputStruct));
    memset(&keyInfo, 0, sizeof(keyInfo));

    inputStruct.key = key;

    result = smc_get_keyinfo(inputStruct.key, &keyInfo);
    if (result != kIOReturnSuccess) {
        return result;
    }

    inputStruct.param.keyInfo.dataSize = keyInfo.dataSize;
    inputStruct.param.data8 = kSMCReadKey;
    
    memset(&outputStruct, 0, sizeof(outputStruct));
    result = smc_call(kSMCHandleYPCEvent, &inputStruct, &outputStruct);
    if (result != kIOReturnSuccess) {
        DBGLOG(CFSTR("smc_call failed %d"), result);
        return result;
    }

    memcpy(bytes, outputStruct.param.bytes, keyInfo.dataSize);

    return kIOReturnSuccess;
}

__attribute__((destructor)) void smc_close(void) {
    if (gConn != 0)
        IOServiceClose(gConn);
}

/* TODO: Return arrays */
int get_fan_status(void) {
    IOReturn result = kIOReturnSuccess;
    uint8_t fan_num = 0;
    int i;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return 0;

    result = smc_read('FNum', &fan_num);
    /* No hardware fan support, or permission deined */
    if (result != kIOReturnSuccess)
        return 0;

    /* FNum(ui8) = 0, no fans on device */
    if (fan_num == 0)
        return 0;

    /* If have fans, check 'F*Ac', which is current speed */
    for (i = 0; i < fan_num; i++) {
        float retval;
        result = smc_read('F\0Ac' | ((0x30 + i) << 0x10), &retval);
        /* F*Ac(flt), return 1 if any fan working */
        if (retval > 0.0)
            return 1;
    }

    return 0;
}

float get_temperature(void) {
    IOReturn result = kIOReturnSuccess;
    uint16_t retval = 0;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return -1;

    result = smc_read('B0AT', &retval);
    if (result != kIOReturnSuccess)
        return -1;

    return (float)retval * 0.01f;
}

float *get_temperature_per_batt(void) {
    IOReturn result = kIOReturnSuccess;
    float retval = 0;

    int num = batt_cell_num();

    float *batts = malloc(sizeof(float) * num);
    /* TB*T(flt), but normally they are same */
    for (int i = 0; i < num; i++) {
        result = smc_read('TB\0T' | ((0x30 + i) << 0x8), &retval);
        if (result != kIOReturnSuccess) {
            /* In design, you should able to get temps of all your batts */
            free(batts);
            return NULL;
        }
        batts[i] = retval;
    }

    return batts;
}

int get_time_to_empty(void) {
    IOReturn result = kIOReturnSuccess;
    uint16_t retval = 0;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return 0;

#if TARGET_OS_EMBEDDED && !TARGET_OS_SIMULATOR
    /* This is weird, why B0TF means TimeToEmpty on Embedded,
     * but TimeToFullCharge on macOS? */
    /* Tested on iPhone 12 mini: B0TF does not exist */
    result = smc_read('B0TF', &retval);
    if (result == kIOReturnSuccess)
        goto got_time;
#endif

    result = smc_read('B0TE', &retval);
    if (result != kIOReturnSuccess)
        return 0;

got_time:
    /* 0xFFFF, battery charging (known scene, possibly others) */
    if (retval == 65535)
        return -1;

    return retval;
}

int estimate_time_to_full() {
    IOReturn result = kIOReturnSuccess;
    int16_t current;
    uint16_t fullcap;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return 0;

    /* B0FC(ui16) FullChargeCapacity (mAh) */
    result = smc_read('B0FC', &fullcap);
    if (result != kIOReturnSuccess)
        return 0;

    /* B0AC(si16) AverageCurrent (mA) */
    result = smc_read('B0AC', &current);
    if (result != kIOReturnSuccess)
        return 0;

    /* Not charging */
    if (current < 0)
        return 0;

    /* TimeToFullCharge = FullChargeCapacity (mAh) / AverageCurrent (mA) */
    return (fullcap / current);
}

float get_battery_health(float *design_cap, float *full_cap) {
    IOReturn result = kIOReturnSuccess;
    uint16_t fullcap;
    uint16_t designcap;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return 0;

    /* B0FC(ui16) FullChargeCapacity (mAh) */
    result = smc_read('B0FC', &fullcap);
    if (result != kIOReturnSuccess)
        return 0;

    /* B0DC(ui16) DesignCapacity (mAh) */
    result = smc_read('B0DC', &designcap);
    if (result != kIOReturnSuccess)
        return 0;

    if (design_cap) {
        *design_cap = designcap;
    }
    if (full_cap) {
        *full_cap = fullcap;
    }
    /* Health = 100.0f * FullChargeCapacity (mAh) / DesignCapacity (mAh) */
    return (100.0f * fullcap / designcap);
}

bool get_capacity(uint16_t *remaining, uint16_t *full, uint16_t *design) {
    if (!gConn) {
        if (smc_open() != kIOReturnSuccess)
            return false;
    }

    int num = batt_cell_num();
    if (num == -1) num = 1;

    uint16_t B0RM, B0FC, B0DC;
    B0RM = B0FC = B0DC = 0;

    /* B0RM(ui16) RemainingCapacity (mAh) */
    IOReturn result = smc_read('B0RM', &B0RM);
    if (result != kIOReturnSuccess)
        return false;

    /* B0FC(ui16) FullChargeCapacity (mAh) */
    result = smc_read('B0FC', &B0FC);
    if (result != kIOReturnSuccess)
        return false;

    /* B0DC(ui16) DesignCapacity (mAh) */
    result = smc_read('B0DC', &B0DC);
    if (result != kIOReturnSuccess)
        return false;

    /* B0RM should be read reversed that scene (e.g. 0x760D -> 0x0D76) */
    /* TODO: We need a better detection for this */
    if (B0RM > B0DC) {
        B0RM = ((B0RM & 0xFF) << 8) | (B0RM >> 8);
    }

    *remaining = B0RM * num;
    *full = B0FC * num;
    *design = B0DC * num;

    return result == kIOReturnSuccess;
}

bool get_gas_gauge(gas_gauge_t *gauge) {
    IOReturn result = kIOReturnSuccess;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return false;

    // Zero before use
    memset(gauge, 0, sizeof(gas_gauge_t));

    /* TODO: Continue shorten those code */

    /* B0AT(ui16): Temperature */
    (void)smc_read('B0AT', &gauge->Temperature);

    /* B0AV(ui16): Average Voltage */
    (void)smc_read('B0AV', &gauge->Voltage);
    
    /* B0FI(hex_): Flags */
    (void)smc_read('B0FI', &gauge->Flags);
    
    /* B0RM(ui16): RemainingCapacity */
    (void)smc_read('B0RM', &gauge->RemainingCapacity);
    
    /* B0FC(ui16): FullChargeCapacity */
    (void)smc_read('B0FC', &gauge->FullChargeCapacity);

    /* B0AC(si16): AverageCurrent */
    (void)smc_read('B0AC', &gauge->AverageCurrent);

    /* B0TF(ui16): TimeToEmpty */
    (void)smc_read('B0TF', &gauge->TimeToEmpty);

    /* BQX1(ui16): Qmax */
    (void)smc_read('BQX1', &gauge->Qmax);

    /* B0AP(si16/si32): AveragePower */
    (void)smc_read('B0AP', &gauge->AveragePower);

    /* B0OC(si16): OCV_Current */
    (void)smc_read('B0OC', &gauge->OCV_Current);

    /* B0OV(ui16): OCV_Voltage */
    (void)smc_read('B0OV', &gauge->OCV_Voltage);

    /* B0CT(ui16): CycleCount */
    (void)smc_read('B0CT', &gauge->CycleCount);

    /* BRSC(ui16): StateOfCharge */
    (void)smc_read('BRSC', &gauge->StateOfCharge);

    /* B0TC(si16): TrueRemainingCapacity */
    (void)smc_read('B0TC', &gauge->TrueRemainingCapacity);

    /* BQCC(si16): PassedCharge */
    (void)smc_read('BQCC', &gauge->PassedCharge);

    /* BQD1(ui16): DOD0 */
    (void)smc_read('BQD1', &gauge->DOD0);
    
    /* TODO: BDD1(ui8/ui16): PresentDOD */
    /* ui8 (%), ui16 (mAh) */
    // (void)smc_read('BDD1', &gauge->PresentDOD);

    /* B0DC(ui16): DesignCapacity */
    (void)smc_read('B0DC', &gauge->DesignCapacity);

    /* B0IM(si16): IMAX */
    (void)smc_read('B0IM', &gauge->IMAX);

    /* B0NC(ui16): NCC */
    (void)smc_read('B0NC', &gauge->NCC);

    /* B0RS(si16): ResScale */
    (void)smc_read('B0RS', &gauge->ResScale);

    /* B0MS(ui16): ITMiscStatus */
    (void)smc_read('B0MS', &gauge->ITMiscStatus);

    /* B0I2(si16): IMAX2 */
    (void)smc_read('B0I2', &gauge->IMAX2);

    /* B0CI(hex_): ChemID */
    (void)smc_read('B0CI', &gauge->ChemID);

    /* B0SR(si16): SimRate */
    (void)smc_read('B0SR', &gauge->SimRate);

    /* Extensions */

    /* BMDN(ch8*)[32]: DeviceName (MacBooks Only) */
    (void)smc_read('BMDN', &gauge->DeviceName);

    /* B0CU(ui16): DesignCycleCount (MacBooks Only) */
    (void)smc_read('B0CU', &gauge->DesignCycleCount);

    /* BMSC(ui16): DailyMaxSoc */
    (void)smc_read('BMSC', &gauge->DailyMaxSoc);

    /* BNSC(ui16): DailyMinSoc */
    (void)smc_read('BNSC', &gauge->DailyMinSoc);
    
    return true;
}

/* -1: Unknown */
int batt_cell_num(void) {
    IOReturn result = kIOReturnSuccess;
    int8_t count = 0;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return -1;
    
    /* BNCB(si8) Number of Battery Cells */
    result = smc_read('BNCB', &count);
    if (result != kIOReturnSuccess)
        return -1;
    
    return (int)count;
}

bool get_cells(cell_info_t cells) {
    return true;
}

bool battery_serial(char *serial) {
    IOReturn result = kIOReturnSuccess;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return false;

    /* BMSN(ch8*) Battery Serial */
    result = smc_read('BMSN', serial);
    if (result != kIOReturnSuccess)
        return false;

    return true;
}

/* Then I found that this was how exactly IOPSCopyExternalPowerAdapterDetails returns */
/*
 SerialString => D?is
 Watts => D?IR * D?VR
 Description => D?DE
 Model => D?ii
 Name => D?in
 PMUConfiguration => CHI?
 Current => D?IR
 FamilyCode => D?FC
 Voltage => D?VR
 FwVersion => D?if
 UsbHvcMenu => D?PM
 AdapterID => D?ii
 UsbHvcHvcIndex => D?PI
 IsWireless => D?FC - kIOPSFamilyCodeExternal < 5
 HwVersion => D?ih
 Manufacturer => D?im
*/
/* Stub macro for PO generation, actual localization shall be done by callers */
#include "not_charging_reason.h"
#define addreason(reason, x) if (code & reason) sprintf(subreason, "%s\n%s", subreason, x);
#define setreason(x) sprintf(subreason, "%s", x);
#define addfault(reason, x) if (code & reason) { fault_reason = true; sprintf(subreason, "%s\n%s", subreason, x); }
char *not_charging_reason_str(uint64_t code) {
    static char reason[1024];
    static uint8_t gen = 0;
    bool fault_reason = false;
    char subreason[1024];

    memset(reason, 0, sizeof(reason));
    memset(subreason, 0, sizeof(subreason));
    
    /* RGEN(ui8 ) NotChargingReason Generation */
    if (gen == 0 && (smc_read('RGEN', &gen) != kIOReturnSuccess))
        gen = 3; // x86_chnc

    /* Apple Silicon NotChargingReason */
    if (gen >= 4) {
        if (code == DEVICE_IS_CHARGING) return _C("None");

        /* Common */
        addreason(NOT_CHARGING_REASON_FULLY_CHARGED, _C("Fully Charged"));
        addreason(NOT_CHARGING_REASON_TEMP_BELOW_MIN_STOP_CHARGING, _C("Too Cold"));
        addreason(NOT_CHARGING_REASON_TEMP_ABOVE_MAX_STOP_CHARGING, _C("Too Hot"));
        addreason(NOT_CHARGING_REASON_TEMP_BELOW_MIN_START_CHARGING, _C("Too Cold To Start"))
        addreason(NOT_CHARGING_REASON_TEMP_ABOVE_MAX_START_CHARGING, _C("Too Hot To Start"));

        /* Presense */
        addreason(NOT_CHARGING_REASON_CHARGE_TIMER_EXPIRED, _C("Charger Wachdog Timeout"));
        addreason(NOT_CHARGING_REASON_BATTERY_NOT_PRESENT, _C("Battery Not Present"));
        addreason(NOT_CHARGING_REASON_VBUS_NOT_PRESENT, _C("VBUS Not Present"));

        addreason(NOT_CHARGING_REASON_HIGH_SOC_HIGH_TEMP_STOP_CHARGING, _C("High SoC Or High Temperature Stopped"));
        addreason(NOT_CHARGING_REASON_CSM_COMMUNICATION_FAILED, _C("Sensor Communication Failed"));

        /* Modes */
        addreason(NOT_CHARGING_REASON_IOAM, _C("Accessory Controlled")); // IOAccessoryManager
        addreason(NOT_CHARGING_REASON_KIOSK_MODE, _C("Kiosk Mode")); // How?
        addreason(NOT_CHARGING_REASON_COREMOTION, _C("CoreMotion")); // How?
        addreason(NOT_CHARGING_REASON_USBPD, _C("USB-PD Connecting")); // This is not 'not charging' ig

        // Internal setbatt tool controlled
        addreason(NOT_CHARGING_REASON_SETBATT, _C("setbatt Controlled"));
        // System controlled
        addreason(NOT_CHARGING_REASON_PREDICTIVECHARGING, _C("Predictive Charging"));
        // What is that? Wireless inductive? I need a MagSafe Battery to check this.
        addreason(NOT_CHARGING_REASON_INDUCTIVE, _C("MagSafe Battery"));
        // Gas Gauge FW Update (May happens with MagSafe chargers, their GG FW is updatable)
        addreason(NOT_CHARGING_REASON_GG_FW_UPDATE, _C("Gas Gauge FW Updating"));
        // Battery does not support inhibit inflow
        // This typically on MacBooks since it controls inflow purely software based
        addreason(NOT_CHARGING_REASON_INHIBIT_INFLOW_BATTERY_NOT_PRESENT, _C("Battery Inhibit Inflow Unsupported"));

        addreason(NOT_CHARGING_REASON_PCTM, _C("PCTM")); // ?
        addreason(NOT_CHARGING_REASON_INHIBIT_CLIENT_ADAPTER, _C("Inhibit Client Adapter"));
        addreason(NOT_CHARGING_REASON_CELL_VOLTAGE_TOO_HIGH, _C("Cell Voltage Too High"));
        addreason(NOT_CHARGING_REASON_BATTERY_NO_CHG_REQ, _C("Battery Not Requesting Charge"));
        addreason(NOT_CHARGING_REASON_WOMBAT, _C("WOMBAT")); // ?

        /* System */
        addreason(NOT_CHARGING_REASON_VACTFB, _C("VACTFB")); // ?
        addreason(NOT_CHARGING_REASON_FIELDDIAGS, _C("Field Diagnostics"));
        addreason(NOT_CHARGING_REASON_INHIBIT_INFLOW, _C("Inhibit Inflow"));
        addreason(NOT_CHARGING_REASON_CARRIER_TEST, _C("Carrier Mode Testing"));
    
        /* Faults */
        if (code & NOT_CHARGING_REASON_PERMANENT_FAULT_MASK) {
            addfault(NOT_CHARGING_REASON_VBAT_VFAULT, _C("Vbatt Fault"));
            addfault(NOT_CHARGING_REASON_IBAT_MINFAULT, _C("Ibatt MinFault"));
            addfault(NOT_CHARGING_REASON_CHARGER_COMMUNICATION_FAILED, _C("Charger Communication Failure"));
            addfault(NOT_CHARGING_REASON_CELL_CHECK_FAULT, _C("Cell Check Fault"));
            addreason(NOT_CHARGING_REASON_BATT_CHARGED_TOO_LONG, _C("Charged Too Long"));
            if (!fault_reason) {
                sprintf(reason, "%s\n", _C("Permanent Battery Failure")); // kAsbPermanentFailureKey
            }
        }
    } else if (gen == 3) {
        /* X86 NotChargingReason (BNCR/CHNC) */
        if (code < 0x20) {
            switch (code) {
                case NO_REASON: setreason(_C("None")); break;
                case NO_BATTERY: setreason(_C("No Battery")); break;
                case BAD_BATTERY: setreason(_C("Bad Battery")); break;
                case BATTERY_FC: setreason(_C("Fully Charged")); break;
                case BATTERY_NO_CHG_REQ: setreason(_C("Battery Not Requesting Charge"));
                case AC_INSERT: setreason(_C("Using AC Power"));
                case G3: setreason(_C("G3 Mechanical Off")); break;
                case ADAPTER_DISABLED: setreason(_C("Adapter Disabled")); break;
                case ADAPTER_UNKNOWN: setreason(_C("Unknown Adapter")); break;
                case ADAPTER_NOT_ALLOW_CHARGING: setreason(_C("Adapter Not Allowed")); break;
                case CALIBRATION: setreason(_C("Calibration")); break;
                case B0LI_0: setreason(_C("Charging Disabled")); break;
                case OS_NO_CHG: setreason(_C("OS Charging Disabled")); break;
                case BCLM_REACHED: setreason(_C("Charging Limit Reached")); break;
                case UPSTREAM_NO_CHG: setreason(_C("Upstream Charging Disabled")); break; // ?
                case PM_NO_CHG: setreason(_C("PowerManagement Chagring Disabled")); break;
                case TB0T_OVER_50: setreason(_C("Battery Temperature over 50℃")); break;
                case TB0T_OVER_45: setreason(_C("Battery Temperature over 45℃")); break;
                case TEMP_GRADIENT_TOO_HIGH: setreason(_C("Temperature Gradient Too High")); break;
                case TEMP_NOT_ATV_VLD: setreason(_C("TEMP_NOT_ATV_VLD")); break; // ?
                case BATTERY_TCA: setreason(_C("BATTERY_TCA")); break; // ?
                case OW_TDM_LINK_ACTIVE: setreason(_C("OW_TDM_LINK_ACTIVE")); break; // ?
                case CELL_VOLTAGE_TOO_HIGH: setreason(_C("Cell Voltage Too High")); break;
                case OBC_NO_CHG: setreason(_C("Predictive Charging")); break;
                case VACTFB_NO_CHG: setreason(_C("VACTFB_NO_CHG")); break; // ?
                case OBC_NO_INFLOW: setreason(_C("Predictive Charging")); break; // What was the diff with OBC_NO_CHG?
                    
                default: setreason(_C("INVALID_NOT_CHARGING_REASON_VALUE")); break;
            }
        } else {
            sprintf(subreason, "Invalid NotChargingReason");
        }
    } else {
        // Legacy NotChargingReason not parsed (since we don't have any devices)
    }

    /* This is conflicting with NOT_CHARGING_REASON_TEMP_ABOVE_MAX_START_CHARGING, why?
    if (code & 0x10) sprintf(subreason, "%s", _C("Charger Timeout"));
    if (code & 0x20) sprintf(subreason, "%s", _C("Charger Wachdog Timeout"));
     */

    /* This is all possible reasons I could know yet
     * Contributing welcomed.
     */
    if (strlen(subreason) > 0) {
        sprintf(reason, "%s%s(0x%llX)", subreason + (gen == 4), (gen == 4) ? "\n" : " ", code);
    } else {
        sprintf(reason, "(0x%llx)", code);
    }

    DBGLOG(CFSTR("NotChargingReason: %s"), reason);
    return reason;
}
#ifdef _
#undef _
#endif
#define _(x) x
char *charger_status_str(uint8_t code[64]) {
    static char status[1024];
    char *byte2stat = NULL;

    if (code[0] == 0x00) return _("None");

    // code[0]: Charger Type I guess, each device has different 1st byte
    // MacBook Pro (13-inch, M1, 2020), Has always 0x8A set
    if (code[0] == 0x8A) {
        switch (code[1]) {
            case 0x6C: byte2stat = _("Connecting");
            // 0xB0
            case 0xB4: byte2stat = _("Fully Charged");
            case 0xB8: byte2stat = _("Stopped");
            case 0xD0: byte2stat = _("Disconnected");
            case 0xD4: byte2stat = _("Connected");
            case 0xD8: byte2stat = _("Charging");
        }
    }
    return status;
}

char *get_adapter_family_desc(mach_port_t family) {
    switch (family) {
        case kIOPSFamilyCodeDisconnected:               return _("Disconnected");
        case kIOPSFamilyCodeUnsupported:                return _("Unsupported");
        case kIOPSFamilyCodeFirewire:                   return _("Firewire");
        case kIOPSFamilyCodeUSBHost:                    return _("USB Host"); // usb host
        case kIOPSFamilyCodeUSBHostSuspended:           return _("Suspended USB Host");
        case kIOPSFamilyCodeUSBDevice:                  return _("USB Device");
        case kIOPSFamilyCodeUSBAdapter:                 return _("Adapter");
        case kIOPSFamilyCodeUSBChargingPortDedicated:   return _("Dedicated USB Charging Port");
        case kIOPSFamilyCodeUSBChargingPortDownstream:  return _("Downstream USB Charging Port");
        case kIOPSFamilyCodeUSBChargingPort:            return _("USB Charging Port"); // usb charger
        case kIOPSFamilyCodeUSBUnknown:                 return _("Unknown USB");
        case kIOPSFamilyCodeUSBCBrick:                  return _("USB-C Brick"); // usb brick
        case kIOPSFamilyCodeUSBCTypeC:                  return _("USB-C Type-C"); // usb type-c
        case kIOPSFamilyCodeUSBCPD:                     return _("USB-C PD"); // pd charger
        case kIOPSFamilyCodeAC:                         return _("AC Power");
        case kIOPSFamilyCodeExternal:                   return _("Externel Power 1");
        case kIOPSFamilyCodeExternal2:                  return _("Externel Power 2");
        case kIOPSFamilyCodeExternal3:                  return _("Externel Power 3"); // baseline arcas
        case kIOPSFamilyCodeExternal4:                  return _("Externel Power 4");
        case kIOPSFamilyCodeExternal5:                  return _("Externel Power 5");
        case kIOPSFamilyCodeExternal6:                  return _("Externel Power 6"); // magsafe chg
        case kIOPSFamilyCodeExternal7:                  return _("Externel Power 7"); // magsafe acc
    }
    return _("Unknown");
}
#undef _

charging_state_t is_charging(mach_port_t *family, device_info_t *info) {
    IOReturn result = kIOReturnSuccess;
    SMCKey key;
    int8_t charging = 0;
    charging_state_t ret = kIsUnavail;
    

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return ret;
    
    /* AC-W(si8) Known cases */
    /* -1: No Adapter (M Chip) */
    /* 0: No Adapter (A Chip) */
    /* 1: Adapter at USB Port 1 */
    /* 2: Adapter at USB Port 2 */
    /* Consider use 'D*AP' for mobile devices (AppleSMCCharger::_checkConnection) */
    result = smc_read('AC-W', &charging);
    if (result != kIOReturnSuccess)
        return ret;

    if (!charging || charging == -1)
        return kIsNotCharging;

    ret = kIsCharging;

#if TARGET_OS_OSX || TARGET_OS_SIMULATOR
    uint16_t time_to_full;
    /* B0TF(ui16) TimeToFull */
    /* FIXME: determine B0TF/B0TE at runtime */
    result = smc_read('B0TF', &time_to_full);
    if (result != kIOReturnSuccess)
        return kIsUnavail;

    /* Not charging, but Adapter attached */
    if (time_to_full == (uint16_t)65535)
        ret = kIsPausing;
#endif

    /* kIOPSPowerAdapterFamily */
    if (family != NULL) {
        key = 'D\0FC' | ((0x30 + charging) << 0x10);
        result = smc_read(key, family);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Family Code: %X"), charging, family);
    }

    /* Not every charger sets those, no return on err */
    if (info != NULL) {
        /* Zero before use */
        memset(info, 0, sizeof(device_info_t));
        info->port = charging;
        /* D?if(ch8*) USB Port ? Firmware version */
        key = 'D\0if' | ((0x30 + charging) << 0x10);
        result = smc_read(key, info->firmware);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Firmware Version: %s"), charging, info->firmware);

        /* D?ih(ch8*) USB Port ? Hardware version */
        key = 'D\0ih' | ((0x30 + charging) << 0x10);
        result = smc_read(key, info->hardware);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Hardware Version: %s"), charging, info->hardware);

        /* D?ii(ch8*) USB Port ? Adapter Model */
        key = 'D\0ii' | ((0x30 + charging) << 0x10);
        result = smc_read(key, info->adapter);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Adapter Model: %s"), charging, info->adapter);

        /* D?im(ch8*) USB Port ? Vendor */
        key = 'D\0im' | ((0x30 + charging) << 0x10);
        result = smc_read(key, info->vendor);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Vendor: %s"), charging, info->vendor);

        /* D?in(ch8*) USB Port ? Name */
        key = 'D\0in' | ((0x30 + charging) << 0x10);
        result = smc_read(key, info->name);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Name: %s"), charging, info->name);

        /* D?is(ch8*) USB Port ? Serial */
        key = 'D\0is' | ((0x30 + charging) << 0x10);
        result = smc_read(key, info->serial);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Serial: %s"), charging, info->serial);

        /* D?DE(ch8*) USB Port ? Description */
        key = 'D\0DE' | ((0x30 + charging) << 0x10);
        result = smc_read(key, &info->description);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Description: 0x%X"), charging, info->description);

        /* CHI?(ui32) USB Port ? PMUConfiguration */
        key = 'CHI\0' | ((0x30 + charging) << 0x0);
        result = smc_read(key, &info->PMUConfiguration);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, PMUConfiguration: 0x%X"), charging, info->PMUConfiguration);

        /* Mobile Only:
         1: Wired Charger
         2: Wireless Charger
         */
        /* D?IR(ui16) USB Port ? Charger current rating */
        key = 'D\0IR' | ((0x30 + charging) << 0x10);
        result = smc_read(key, &info->current);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Current: 0x%X"), charging, info->current);

        /* D?VR(ui16) USB Port ? Charger voltage rating */
        key = 'D\0VR' | ((0x30 + charging) << 0x10);
        result = smc_read(key, &info->voltage);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Voltage: %u"), charging, info->voltage);
        
        /* D?PM(hex_) USB Port ? Power Modes */
        key = 'D\0PM' | ((0x30 + charging) << 0x10);
        memset(info->hvc_menu, 0, sizeof(info->hvc_menu));
        result = smc_read(key, info->hvc_menu);
        if (result == kIOReturnSuccess) {
            DBGLOG(CFSTR("Port: %d, Modes: 0x%X"), charging, info->hvc_menu);
        } else {
            info->hvc_menu[27] = 0xFF; /* We write a special bit for indicating failure */
        }

        /* D?PI(si8 ) USB Port ? Mode Index */
        key = 'D\0PI' | ((0x30 + charging) << 0x10);
        result = smc_read(key, &info->hvc_index);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Index: %d"), charging, info->hvc_index);

        /* Other info */
    }

    return ret;
}

// This is different with device_info_t which returned by is_charging()
bool get_charger_data(charger_data_t *data) {
    IOReturn result = kIOReturnSuccess;

    // No matter if charging or not, get data anyway
    memset(data, 0, sizeof(charger_data_t));

    /* CHCE(ui8 ) Charger Exist */
    result = smc_read('CHCE', &data->ChargerExist);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charger Exist: %u"), &data->ChargerExist);

    /* CHCC(ui8 ) Charger Capable */
    result = smc_read('CHCC', &data->ChargerCapable);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charger Capable: %u"), &data->ChargerCapable);

    /* CHBI(ui32) Charging Current */
    result = smc_read('CHBI', &data->ChargingCurrent);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charging Current: %u"), &data->ChargingCurrent);

    /* CHBV(ui32) Charging Voltage */
    result = smc_read('CHBV', &data->ChargingVoltage);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charging Voltage: %u"), &data->ChargingVoltage);

    /* BVVL(ui16) Charger Vac Voltage Limit */
    result = smc_read('BVVL', &data->ChargerVacVoltageLimit);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charger Vac Voltage Limit: %u"), &data->ChargerVacVoltageLimit);

    /* CHNC(hex_)[8] Not Charging Reason */
    result = smc_read('CHNC', &data->NotChargingReason);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Not Charging Reason: 0x%X"), &data->NotChargingReason);

    /* CHSL(hex_)[8] Charger Status ([64] on mobile devices) */
    result = smc_read('CHSL', &data->ChargerStatus);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charger Status: 0x%X"), &data->ChargerStatus);

    /* CH0D(hex_)[4] Charger ID */
    result = smc_read('CH0D', &data->ChargerId);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charger ID: 0x%X"), &data->ChargerId);

    /* CHAS(ui32) Charger Configuration */
    result = smc_read('CHAS', &data->ChargerConfiguration);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charger Configuration: 0x%X"), &data->ChargerConfiguration);
    
    return (data->ChargerExist & 1) != 0;
}

bool get_power_state(power_state_t *power) {
    return true;
}

/* Sadly we still have to get hvc_menu from IOPS, since Macs has no D?PM */
hvc_menu_t *hvc_menu_parse(uint8_t *input, size_t *size) {
    /* Only get 28 bytes */
    uint8_t bytes[28];
    memcpy(bytes, input, 28);

    static hvc_menu_t menu[7];
    memset(menu, 0, sizeof(menu));

    int valid_id = 0;
    int f = 0;
    for (int i = 0; i < 7; i++) {
      f = i * 4;
      if (bytes[f] || bytes[f + 1] || bytes[f + 2] || bytes[f + 3]) {
        menu[valid_id].voltage = bytes[f + 1] << 8 | bytes[f];
        menu[valid_id].current = bytes[f + 3] << 8 | bytes[f + 2];
        valid_id++;
      }
    }

    *size = valid_id;

    return menu;
}

#pragma Iktara Wireless

/* Known charger Keys:
 VQ0u(ioft)[8]: VBUS Voltage (V)
 IQ0u(ioft)[8]: IBUS Current (A)
 D?ID(flag): IOAM Inflow Inhibit
 CHIE(hex_)[1]: Inflow Inhibit
 
 */
wireless_state_t wireless_charging_detect(void) {
    IOReturn result = kIOReturnSuccess;
    uint32_t st = 0;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return false;

    /* VBUS(ui32) Voltage Bus */
    result = smc_read('VBUS', &st);
    if (result != kIOReturnSuccess)
        return false;

    return (st & 0xFE);
}

bool get_iktara_fw_stat(iktara_fw_t *fw) {
    IOReturn result = kIOReturnSuccess;
    uint64_t st = 0;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return false;
    
    /* WAFS(hex_) Iktara Firmware Status */
    result = smc_read('WAFS', &st);
    if (result != kIOReturnSuccess)
        return false;

    if (fw != NULL) {
        memset(fw, 0, sizeof(iktara_fw_t));

        fw->Charging = (st & 0xF000000) == 0xE000000;
        fw->Connected = (st >> 0x0B) & 1;
        fw->FieldPresent = (st >> 0x0A) & 1;
        fw->AppFWRunning = (st >> 0x09) & 1;
        fw->ExceptionState = (st & 0x3F);
        fw->OvpTriggered = (st >> 0x1D) & 1;
        fw->LpmActive = (st >> 0x1C) & 1;
    }

    return true;
}

/* Notes on some guessed keys
 Mobile Only:
    D?DB(hex_): USB Port ? debounce
    D?AR(si32): USB Port ? Ampere
    D?SM(ui32): USB Port ? Socket Model
    D?NO(ui8 ): (write only, unknown)
    D?UD(ui32): USB Port ? SourceID
    D?SD(flag): USB Port ? sharedSource
    D?PM(hex_): USB Port ? HVC
    D1SX(ui8 ): High Voltage Charger Interrupt Action
    D1SR(ui16): High Voltage Charger Request Ready
    D1SP(ui8 ): High Voltage Charger Notification
 
    WAFC(ui32): (write only) Smart Battery control bit (Enable: 0x10000) | (0x7: Wireless Cloak) (0x25: Charge Limit Display)
    BD0E(ui32): DiffAmp
    B0LP(ui16): Lpem Props
 
    QQ0u(ioft): iBus accumulator
 
 
 All:
    AC-N(ui8 ): Adapter count
    AC-W(si8 ): Active Adapter Index
 
    D?FC(ui32): USB Port ? Family Code (kIOPSPowerAdapterFamilyKey)
    D?IR(si32): USB Port ? input Current
    D?VR(si32): USB Port ? input Voltage
    D?BD(ui32): USB Port ? AdapterID
    D?DE(ch8*): USB Port ? Description
    D?PT(ui8 ): USB Port ? Adapter Type
 
    B0Ti(ui32): Charge Limit Rate Index
    

    CHSC(ui8 ): Charger Status
    CHPS(ui32): selected powerpath
    CHA?(ui32): Powerpath ?
    CHHV(ui64): (write only) USB Input High Voltage
    CHI?(ui32): USB Port ? Input Current Limit / PMUConfiguration
    CHBI(ui32): Charge Current Configuration
    CHBV(ui32): Charge Voltage Configuration
    CHTU(ui32): Carrier Mode upper voltage
    CHTL(ui32): Carrier Mode lower voltage
    CHTE(ui32): Carrier Mode
    CHKL(ui16): Kiosk Mode voltage
    CHKM(ui8 ): Kiosk Mode
    CH0I(ui8 ): Battery Connected State 1 << 0
    CH0J(ui8 ): Battery Connected State 1 << 1
    CH0K(ui8 ): Battery Connected State 1 << 2

    MBSE(hex_): Sleep-Wake related
    MBSW(hex_): Sleep-Wake related
 
    UPOF(hex_): Shutdown data error flags
    UBNC(ui16): Shutdown nominal capacity
    UB0C(ui8 ): (write only) Shutdown data (write 1 to clear)
 
 Conditional:
    D?PI(ui8 ): USB Port ? HVC Index (Software HVC on Mac)

    VQ0u(ioft): VBUS Voltage
    
 */
    
