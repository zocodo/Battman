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
#include "../intlextern.h"
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
#define kIOReturnError 0xE00002BC
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
    kIOPSFamilyCodeUnsupported  = 0xE00002C7,

    kIOPSFamilyCodeFirewire     = 0xE0008000,

    kIOPSFamilyCodeUSBHost      = 0xE0004000,
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
    kIOPSFamilyCodeAC           = 0xE0024000,
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

extern bool show_alert(const char *, const char *, const char *);
extern void show_alert_async(const char *, const char *, const char *, void (^)(bool));
extern void app_exit(void);
extern void NSLog(CFStringRef, ...);

static io_service_t gConn = 0;
gas_gauge_t gGauge = {0};
board_info_t gBoard = {0};

static IOReturn smc_open(void) {
    IOReturn result;
    mach_port_t masterPort;
    io_service_t service;
    const char *fail_title = NULL;

    if (IOMasterPort(MACH_PORT_NULL, &masterPort) != kIOReturnSuccess) {
        DBGLOG(CFSTR("IOMasterPort() failed"));
        fail_title = _C("IOMainPort Open Failed");
        goto fail;
    }

    service = IOServiceGetMatchingService(masterPort, IOServiceMatching("AppleSMC"));
    result = IOServiceOpen(service, mach_task_self(), 0, &gConn);
    if (result != kIOReturnSuccess) {
        /* TODO: Check entitlements and explicitly warn which we loss */
        fail_title = _C("AppleSMC Open Failed");
        DBGLOG(CFSTR("IOServiceOpen() failed (%d)"), result);
        goto fail;
    }

    return kIOReturnSuccess;

fail:
    show_alert_async(fail_title, _C("This typically means you did not install Battman with correct entitlements, please reinstall by checking instructions at https://github.com/Torrekie/Battman"), _C("OK"), ^(bool res) {
        app_exit();
    });
    return kIOReturnError;
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
    SMCParamStruct inputStruct={0};
    SMCParamStruct outputStruct;
    IOReturn result;

    inputStruct.key = key;
    inputStruct.param.data8 = kSMCGetKeyInfo;

    result = smc_call(kSMCHandleYPCEvent, &inputStruct, &outputStruct);

    // Important check: dataSize != 0
    // idk why a nonexist key could return kIOReturnSuccess
    if (outputStruct.param.keyInfo.dataSize == 0)
        result = kIOReturnError;
    
    if (result == kIOReturnSuccess) {
        *keyInfo = outputStruct.param.keyInfo;
    }

    return result;
}

static IOReturn smc_read(uint32_t key, void *bytes) {
    IOReturn result;
    SMCParamStruct inputStruct={0};
    SMCParamStruct outputStruct;
    SMCKeyInfoData keyInfo;

    inputStruct.key = key;

    result = smc_get_keyinfo(inputStruct.key, &keyInfo);
    if (result != kIOReturnSuccess) {
        return result;
    }

    inputStruct.param.keyInfo.dataSize = keyInfo.dataSize;
    inputStruct.param.data8 = kSMCReadKey;
    
    result = smc_call(kSMCHandleYPCEvent, &inputStruct, &outputStruct);
    if (result != kIOReturnSuccess) {
        DBGLOG(CFSTR("smc_call failed %d"), result);
        return result;
    }

    memcpy(bytes, outputStruct.param.bytes, keyInfo.dataSize);

    return kIOReturnSuccess;
}

static float ioft2flt(void *bytes) {
    uint64_t res = 0;
    uint8_t *x = (uint8_t *)bytes;

    /* ioft has size 8 */
    for (uint32_t i = 0; i < 8; i++) {
        res |= (uint64_t)x[i] << (8 * i);
    }

    return (float)res / 65536.0f;
}

__attribute__((destructor)) static void smc_close(void) {
    if (gConn != 0)
        IOServiceClose(gConn);
}

const board_info_t *get_board_info(void) {
    static bool retrieved = false;

    /* These info are constants, only retrieve once and set gBoard */
    if (!retrieved) {
        /* RGEN(ui8 ) Generation */
        smc_read('RGEN', &gBoard.Generation);
        /* RESV(ch8*)[16] EmbeddedOSVersion */
        smc_read('RESV', &gBoard.EmbeddedOSVersion);
        /* RECI(ui64) ChipEcid */
        smc_read('RECI', &gBoard.ChipEcid);
        /* RCRV(ui32) ChipRev */
        smc_read('RCRV', &gBoard.ChipRev);
        /* RBRV(ui32) BoardRev */
        smc_read('RBRV', &gBoard.BoardRev);
        /* RBID(ui32) BoardId */
        smc_read('RBID', &gBoard.BoardId);
        /* RPlt(ch8*)[8] TargetName */
        smc_read('RPlt', &gBoard.TargetName);

        retrieved = true;
    }

    return &gBoard;
}

/* Fan Control Keys:
 F?Dc(flt ) PercentPWM
 F?Md(ui8 ) Mode
 F?Ac(flt ) CurrentSpeed
 F?Mn(flt ) MinSpeed
 F?Mx(flt ) MaxSpeed
 F?St(ui8 ) Stat?
 F?Tg(flt ) TargetAccess
 FBAD(hex_)[4] BadFanFlags
 FNum(ui8 ) NumFans
 FOFC(ui32) fanInOffStateCnt
 FOff(ui8 ) fanOffModeEnabled
 FRmn(ui16) fanRampRateIgnore
 FRmp(ui16) fanRampLimited
 */
/* TODO: Return arrays */
int get_fan_status(void) {
    IOReturn result = kIOReturnSuccess;
    uint8_t fan_num;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return 0;

    result = smc_read('FNum', &fan_num);
    /* No hardware fan support, or permission deined */
    if (result != kIOReturnSuccess)
        return 0;

    /* FNum(ui8) == 0, no fans on device */
    if (!fan_num)
        return 0;

    /* If have fans, check 'F*Ac', which is current speed */
    for (int i = 0; i < fan_num; i++) {
        float retval;
        result = smc_read('F\0Ac' | ((0x30 + i) << 0x10), &retval);
        /* F*Ac(flt), return 1 if any fan working */
        if (retval > 0.0)
            return 1;
    }

    return 0;
}

#pragma Temperatures

/* Known keys:
 TP?d(ioft): PMU tdev ?
 
 */

float get_temperature(void) {
    IOReturn result = kIOReturnSuccess;
    uint16_t retval;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return -1;

    result = smc_read('B0AT', &retval);
    if (result != kIOReturnSuccess)
        return -1;

    return (float)retval * 0.01f;
}

float *get_temperature_per_cell(void) {
    IOReturn result;
    float retval;
    uint8_t ioftret[8];

    int num = batt_cell_num();

    float *cells = malloc(sizeof(float) * num);
    /* TB?T(flt ): Cell ? real-time temperature */
    for (int i = 0; i < num; i++) {
        result = smc_read('TB\0T' | ((0x30 + i) << 0x8), &retval);
        if (result != kIOReturnSuccess) {
            /* In design, you should able to get temps of all your batts */
            break;
        }
        cells[i] = retval;
    }

    /* TB*T may not exist on mobile devices */
    if (result != kIOReturnSuccess) {
        /* TG?B(ioft): Cell ? existance & real-time temperature */
        for (int i = 0; i < num; i++) {
            memset(ioftret, 0, sizeof(ioftret));
            result = smc_read('TG\0B' | ((0x30 + i) << 0x8), &ioftret);
            if (result != kIOReturnSuccess) {
                /* In design, you should able to get temps of all your batts */
                free(cells);
                return NULL;
            }
            cells[i] = ioft2flt(ioftret);
        }
    }

    return cells;
}

int get_time_to_empty(void) {
    IOReturn result = kIOReturnSuccess;
    int16_t retval;

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
    //if (retval == 65535)
    //    return -1;
    // signed int16_t 65535=-1

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
    /* TODO: B0IV(si16) InstantAmperage */
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
    if(!remaining&&!full&&!design)
        return true;
    
    if (!gConn&&smc_open())
        return false;

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

    if(remaining)
        *remaining = B0RM * num;
    if(full)
        *full = B0FC * num;
    if(design)
        *design = B0DC * num;

    return result == kIOReturnSuccess;
}

bool get_gas_gauge(gas_gauge_t *gauge) {
    if (!gConn&&smc_open())
        return false;

    // Zero before use
    memset(gauge, 0, sizeof(gas_gauge_t));

    /* TODO: Continue shorten those code */

    /* B0AT(ui16): Temperature */
    smc_read('B0AT', &gauge->Temperature);

    /* B0AV(ui16): Average Voltage */
    smc_read('B0AV', &gauge->Voltage);
    
    /* B0FI(hex_): Flags */
    smc_read('B0FI', &gauge->Flags);
    
    /* B0RM(ui16): RemainingCapacity */
    smc_read('B0RM', &gauge->RemainingCapacity);
    
    /* B0FC(ui16): FullChargeCapacity */
    smc_read('B0FC', &gauge->FullChargeCapacity);

    /* B0AC(si16): AverageCurrent */
    smc_read('B0AC', &gauge->AverageCurrent);

    /* B0TF(ui16): TimeToEmpty */
    smc_read('B0TF', &gauge->TimeToEmpty);

    /* BQX1(ui16): Qmax */
    smc_read('BQX1', &gauge->Qmax);

    /* B0AP(si16/si32): AveragePower */
    smc_read('B0AP', &gauge->AveragePower);

    /* B0OC(si16): OCV_Current */
    smc_read('B0OC', &gauge->OCV_Current);

    /* B0OV(ui16): OCV_Voltage */
    smc_read('B0OV', &gauge->OCV_Voltage);

    /* B0CT(ui16): CycleCount */
    smc_read('B0CT', &gauge->CycleCount);

    /* BRSC(ui16): StateOfCharge */
    smc_read('BRSC', &gauge->StateOfCharge);

    /* B0TC(si16): TrueRemainingCapacity */
    smc_read('B0TC', &gauge->TrueRemainingCapacity);

    /* BQCC(si16): PassedCharge */
    smc_read('BQCC', &gauge->PassedCharge);

    /* BQD1(ui16): DOD0 */
    smc_read('BQD1', &gauge->DOD0);
    
    /* TODO: BDD1(ui8/ui16): PresentDOD */
    /* ui8 (%), ui16 (mAh) */
    // smc_read('BDD1', &gauge->PresentDOD);

    /* B0DC(ui16): DesignCapacity */
    smc_read('B0DC', &gauge->DesignCapacity);

    /* B0IM(si16): IMAX */
    smc_read('B0IM', &gauge->IMAX);

    /* B0NC(ui16): NCC */
    smc_read('B0NC', &gauge->NCC);

    /* B0RS(si16): ResScale */
    smc_read('B0RS', &gauge->ResScale);

    /* B0MS(ui16): ITMiscStatus */
    smc_read('B0MS', &gauge->ITMiscStatus);

    /* B0I2(si16): IMAX2 */
    smc_read('B0I2', &gauge->IMAX2);

    /* B0CI(hex_): ChemID */
    smc_read('B0CI', &gauge->ChemID);

    /* B0SR(si16): SimRate */
    smc_read('B0SR', &gauge->SimRate);

    /* Extensions */

    /* BMDN(ch8*)[32]: DeviceName (MacBooks Only) */
    smc_read('BMDN', &gauge->DeviceName);

    /* B0CU(ui16): DesignCycleCount (MacBooks Only) */
    smc_read('B0CU', &gauge->DesignCycleCount);

    /* BMSC(ui16): DailyMaxSoc */
    smc_read('BMSC', &gauge->DailyMaxSoc);

    /* BNSC(ui16): DailyMinSoc */
    smc_read('BNSC', &gauge->DailyMinSoc);

    /* BUIC(ui8 ): UI Displayed SoC */
    smc_read('BUIC', &gauge->UISoC);

    /* B0SC(si8 ): Chemical SoC */
    smc_read('B0SC', &gauge->ChemicalSoC);

    /* BUPT(hex_)[8]: BMS Uptime */
    smc_read('BUPT', &gauge->bmsUpTime);
    
    /* B0FD BattData */
    /* BROS MinDOD? */
    /* B0TI */
    /* B0ET */
    /* BCHT ChargingTable */
    /* BVVP PermanentVacVoltageLimit */
    /* BVVM TerminationVoltage */
    /* B0FU ForceUISoC */
    /* BCBL */

    /* BatteryCriticalFlags */
    /* BCFB */
    /* BCFD */
    /* BCFT */
    /* BCFP NeedsPrecharge */
    /* BCFV Voltage */
    return true;
}

/* -1: Unknown */
int batt_cell_num(void) {
    IOReturn result = kIOReturnSuccess;
    int8_t count;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return -1;
    
    /* BNCB(si8) Number of Battery Cells */
    result = smc_read('BNCB', &count);
    if (result != kIOReturnSuccess)
        return -1;
    
    return count;
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
const char *not_charging_reason_str(uint64_t code) {
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
        addreason(NOT_CHARGING_REASON_FULLY_CHARGED, _C("Fully Charged")); // This will be set when FC flag sets
        addreason(NOT_CHARGING_REASON_TEMP_BELOW_MIN_STOP_CHARGING, _C("Too Cold"));
        addreason(NOT_CHARGING_REASON_TEMP_ABOVE_MAX_STOP_CHARGING, _C("Too Hot"));
        addreason(NOT_CHARGING_REASON_TEMP_BELOW_MIN_START_CHARGING, _C("Too Cold To Start"))
        addreason(NOT_CHARGING_REASON_TEMP_ABOVE_MAX_START_CHARGING, _C("Too Hot To Start"));

        /* Presense */
        addreason(NOT_CHARGING_REASON_CHARGE_TIMER_EXPIRED, _C("Charger Wachdog Timeout"));
        addreason(NOT_CHARGING_REASON_BATTERY_NOT_PRESENT, _C("Battery Not Present"));
        addreason(NOT_CHARGING_REASON_VBUS_NOT_PRESENT, _C("VBUS Not Present"));

        addreason(NOT_CHARGING_REASON_HIGH_SOC_HIGH_TEMP_STOP_CHARGING, _C("High SoC High Temp Stopped"));
        addreason(NOT_CHARGING_REASON_CSM_COMMUNICATION_FAILED, _C("Sensor Communication Failed"));

        /* Modes */
        addreason(NOT_CHARGING_REASON_IOAM, _C("Accessory Connecting")); // IOAccessoryManager
        addreason(NOT_CHARGING_REASON_KIOSK_MODE, _C("Kiosk Mode")); // How?
        addreason(NOT_CHARGING_REASON_COREMOTION, _C("CoreMotion")); // How?
        addreason(NOT_CHARGING_REASON_USBPD, _C("USB-PD Connecting")); // This is not 'not charging' ig

        // Internal setbatt tool controlled (not only setbatt can trigger this btw)
        addreason(NOT_CHARGING_REASON_SETBATT, _C("setbatt Controlled"));
        // System controlled
        addreason(NOT_CHARGING_REASON_PREDICTIVECHARGING, _C("Predictive Charging"));
        // Inductive Charging, but I don't have such charger to trigger this
        addreason(NOT_CHARGING_REASON_INDUCTIVE, _C("Wireless Charger Controlled"));
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

static const char *port_types[] = {
    _ID_("Unknown"),
    _ID_("Virtual"),
    _ID_("USB-C"),
    _ID_("USB-A"),
    _ID_("MiniDP"),
    _ID_("FireWire800"),
    _ID_("HDMI"),
    _ID_("AudioJack-Mini"),
    _ID_("Ethernet"),
    _ID_("MagSafe"), // This is typically old MagSafe charger port on old MacBooks
    _ID_("MagSafe2"),
    _ID_("SD Card"),
    _ID_("Lightning"),
    _ID_("30-Pin"),
    _ID_("Inductive"), // Wireless
    _ID_("SmartConnector"),
    _ID_("DisplayPort")
};

const char *port_type_str(uint8_t pt) {
    if (pt > 16) {
        return _ID_("Undefined");
    }
    return port_types[pt];
}

const char *charger_status_str(uint8_t code[64]) {
    const char *byte2stat = NULL;

    if (code[0] == 0x00) return _ID_("None");

    // code[0]: Charger Type I guess, each device has different 1st byte
    // MacBook Pro (13-inch, M1, 2020), Has always 0x8A set
    if (code[0] == 0x8A) {
        switch (code[1]) {
            case 0x6C: byte2stat = _ID_("Connecting");
            // 0xB0
            case 0xB4: byte2stat = _ID_("Fully Charged");
            case 0xB8: byte2stat = _ID_("Stopped");
            case 0xD0: byte2stat = _ID_("Disconnected");
            case 0xD4: byte2stat = _ID_("Connected");
            case 0xD8: byte2stat = _ID_("Charging");
        }
    }
    return byte2stat;
}

const char *get_adapter_family_desc(mach_port_t family) {
    switch (family) {
        case kIOPSFamilyCodeDisconnected:               return _ID_("Disconnected");
        case kIOPSFamilyCodeUnsupported:                return _ID_("Unsupported");
        case kIOPSFamilyCodeFirewire:                   return _ID_("Firewire");
        case kIOPSFamilyCodeUSBHost:                    return _ID_("USB Host"); // usb host
        case kIOPSFamilyCodeUSBHostSuspended:           return _ID_("Suspended USB Host");
        case kIOPSFamilyCodeUSBDevice:                  return _ID_("USB Device");
        case kIOPSFamilyCodeUSBAdapter:                 return _ID_("Adapter");
        // Consider display abbreviated DCP/CDP/SDP instead
        case kIOPSFamilyCodeUSBChargingPortDedicated:   return _ID_("Dedicated USB Charging Port"); // usb charger
        case kIOPSFamilyCodeUSBChargingPortDownstream:  return _ID_("Downstream USB Charging Port");
        case kIOPSFamilyCodeUSBChargingPort:            return _ID_("USB Charging Port"); // usb charger
        case kIOPSFamilyCodeUSBUnknown:                 return _ID_("Unknown USB");
        case kIOPSFamilyCodeUSBCBrick:                  return _ID_("USB-C Brick"); // usb brick
        case kIOPSFamilyCodeUSBCTypeC:                  return _ID_("USB-C Type-C"); // usb type-c
        case kIOPSFamilyCodeUSBCPD:                     return _ID_("USB-C PD"); // pd charger
        case kIOPSFamilyCodeAC:                         return _ID_("AC Power");
        case kIOPSFamilyCodeExternal:                   return _ID_("Externel Power 1");
        case kIOPSFamilyCodeExternal2:                  return _ID_("Externel Power 2");
        case kIOPSFamilyCodeExternal3:                  return _ID_("Externel Power 3"); // baseline arcas
        case kIOPSFamilyCodeExternal4:                  return _ID_("Externel Power 4");
        case kIOPSFamilyCodeExternal5:                  return _ID_("Externel Power 5");
        case kIOPSFamilyCodeExternal6:                  return _ID_("Externel Power 6"); // magsafe chg
        case kIOPSFamilyCodeExternal7:                  return _ID_("Externel Power 7"); // magsafe acc
    }
    return _ID_("Unknown");
}

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
    /* Consider use 'D*AP' (Adapter Power State) for mobile devices (AppleSMCCharger::_checkConnection) */
    /* Mobile Only:
     1: Wired Charger
     2: Wireless Charger
     */
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
        
        /* D?PM(hex_) USB Port ? HVC Power Modes */
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
        /* D?PT(ui8 ) Port Type */
        key = 'D\0PT' | ((0x30 + charging) << 0x10);
        result = smc_read(key, &info->port_type);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Type: %d"), charging, info->port_type);

        /* D?AR(ui32) Ampere Rating (Mobile Only) */
        /* D?BD(ui32) AdapterID */
        /* D?DB(hex_) debounce */
        /* D?ER(?) */
        /* D?ID(flag) IOAM Inflow Inhibit (Mobile Only) */
        /* D?IG(flag) Ignored (Mobile Only) */
        /* D?NO(ui8 ) Select power source (write only) (Mobile Only) */
        /* D?SD(flag) SourceID (Mobile Only) */
        /* D?SM(ui32) Socket Model (Mobile Only) */
        /* D?SN(ui8 ) ? */
        /* D1SP(ui8 ) HVC Notification (Mobile Only) */
        /* D?SR(ui16) HVC Request Ready (Mobile Only) */
        /* D?SX(ui8 ) HVC Interrupt Action (Mobile Only) */
        /* D?UD(ui32) sharedSource (Mobile Only) */
    }

    return ret;
}

// This is different with device_info_t which returned by is_charging()
bool get_charger_data(charger_data_t *data) {
    IOReturn result = kIOReturnSuccess;

    // No matter if charging or not, get data anyway
    memset(data, 0, sizeof(charger_data_t));
    
    /* CHCC(ui8 ) Charger Capable / Charger External Charge Capable */
    result = smc_read('CHCC', &data->ChargerCapable);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charger Capable: %u"), &data->ChargerCapable);

    /* CHCE(ui8 ) Charger Exist / Charger External Connected */
    result = smc_read('CHCE', &data->ChargerExist);
    if (result == kIOReturnSuccess)
        DBGLOG(CFSTR("Charger Exist: %u"), &data->ChargerExist);

    /* CHCF(hex_)[1] Charger Flags / Charger External Connected */
    /* CHCR(ui8 ) */

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

    /* CHFC(ui8 ) */
    /* CHFS(ui32) */

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
 CHIE(hex_)[1]: Inflow Inhibit
 CHIS(ui32): Charger Input State (like D?AP)?
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

/*
 AY-N(ui8 ): Num Wireless ports
 AY1A(hex_): Wireless A Array [0-4 Status?][5 AY1P][6 AY1C][7 AY1T][8 AY1S][9-10 VendorID][11-12 ProductID]
 AY1C(ui8 ): Wireless Charging
 AY1P(ui8 ): Wireless Present
 AY1S(ui8 ): Wireless Curent Capacity
 AY1T(ui8 ):
*/
/*
 (<_BCPowerSourceController: 0x28235c900>) Found power source: {
     "Accessory Category" = "Battery Case";
     "Current Capacity" = 15;
     "Is Charging" = 0;
     "Is Present" = 1;
     "Max Capacity" = 100;
     Name = "MagSafe Battery Pack";
     "Power Source ID" = "-1367146496";
     "Power Source State" = "Battery Power";
     "Product ID" = 5017;
     "Show Charging UI" = 0;
     "Transport Type" = "Inductive In-Band";
     Type = "Battery Case";
     "Vendor ID" = 1452;
 }
*/

/* Notes on some guessed keys
 Mobile Only:
    WAFC(ui32): (write only) Smart Battery control bit (Enable: 0x10000) | (0x7: Wireless Cloak) (0x25: Charge Limit Display)
    WADS(hex_): Inductive status
    BD0E(ui32): DiffAmp
    B0LP(ui16): Lpem Props
 
    GBCM(ui16): GG blockdata related
    GBRW(hex_): GG blockdata related
    GCCM(ui16): GG control related
    GCOP(ui16): GG control related
    GCRW(hex_): GG control related
    GRAD(ui8 ): GG reg related
    GRRD(ui16): GG reg related
    
 
    QQ0u(ioft): iBus accumulator
 
 All:
    AC-N(ui8 ): Adapter count
 
    B0Ti(ui32): Charge Limit Rate Index

    CHSC(ui8 ): Charger Charging?
    CHSE(ui8 ): Charger End-of-charge
    CHST(ui8 ): Charger Terminated?
    CHPS(ui32): selected powerpath
    CHPP(ui32): powerpath priority
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
 
    CIBL(ui32): IBatt target limit

    MBSE(hex_): MachineStateEmbedded
    MBSW(hex_): PmuWakeEvents
    MBSe(hex_): MachineStateEvents
    MST3(ui32): ui32TmTestCase
    MST6(ui64): ui64TmTestCase
    MSTC(ui16): ui16TmTestCase
    MSTD(ui16): ui16TmTestCase2
    MTKN(ui32): ui8Tokens
 
    NESN(hex_): ApcReceiveNotification
    NTAP(flag): ApcNotifyOK
 
    UPOF(hex_): Shutdown data error flags
    UBNC(ui16): Shutdown nominal capacity
    UB0C(ui8 ): (write only) Shutdown data (write 1 to clear)
 
 Conditional:
    VQ0u(ioft): VBUS Voltage
    VP0u VQ0l VQ0B VQ0u VQDD VQHI
 
    DBTE(ui8 ): SMC TGraph Mode
    DCAL(hex_): DispRamUAccess
    DRAM(hex_): DispRam
 
    MEPG(ui8 ): SMC Power Prevent Nap
 
    TQ?B(ioft): Charger B Temperature
    TQ?d(ioft): Charger d Temperature
    TQ?j(ioft): Charger j Temperature
 
    WQ0u(ioft): Charger WQ0u Voltage
 
    aDC!(ui32): RawADC
    aDC#(ui32): ADCSensorsCount
    aDCR(ioft):
    aP??(ui32): ADC sensor ??
    aPMX(ui8 ): PmuAmux (write only)
 
    bHLD(ui32): Power Button Hold
    bPHD(flag): PoweredByHoldButton
    bRIN(ui32): ?
    bVDN(ui32): Volume Down Button Hold
    bVUP(ui32): Volume Up Button Hold
 
    gP??(ui32): PMU GPIO keys

    pmFC(hex_): PmuFeatureControl
    rARA(ui32): PmuRailCTL
    rARa rASO rASo
 
    
 */
    
