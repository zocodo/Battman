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

#define DBGLOG(...) NSLog(__VA_ARGS__)

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

    service =
        IOServiceGetMatchingService(masterPort, IOServiceMatching("AppleSMC"));
    result = IOServiceOpen(service, mach_task_self(), 0, &gConn);
    if (result != kIOReturnSuccess) {
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

    int num = battery_num();

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

    int num = battery_num();
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

    return true;
}

/* -1: Unknown */
int battery_num(void) {
    IOReturn result = kIOReturnSuccess;
    int8_t count = 0;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return -1;
    
    /* BNCB(si8) Number of Chargable Batteries (Guessed) */
    result = smc_read('BNCB', &count);
    if (result != kIOReturnSuccess)
        return -1;
    
    return (int)count;
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
/*
 === kIOPSPowerAdapterFamilyKey ===         Decimal       Hex
 kIOPSFamilyCodeDisconnected                0             0
 kIOPSFamilyCodeUnsupported                 -536870201    E00002C7
 kIOPSFamilyCodeFirewire                    -536838144    E0008000
 kIOPSFamilyCodeUSBHost                     -536854528    E0004000
 kIOPSFamilyCodeUSBHostSuspended            -536854527    E0004001
 kIOPSFamilyCodeUSBDevice                   -536854526    E0004002
 kIOPSFamilyCodeUSBAdapter                  -536854525    E0004003
 kIOPSFamilyCodeUSBChargingPortDedicated    -536854524    E0004004
 kIOPSFamilyCodeUSBChargingPortDownstream   -536854523    E0004005
 kIOPSFamilyCodeUSBChargingPort             -536854522    E0004006
 kIOPSFamilyCodeUSBUnknown                  -536854521    E0004007
 kIOPSFamilyCodeUSBCBrick                   -536854520    E0004008
 kIOPSFamilyCodeUSBCTypeC                   -536854519    E0004009
 kIOPSFamilyCodeUSBCPD                      -536854518    E000400A
 kIOPSFamilyCodeAC                          -536723456    E0024000
 kIOPSFamilyCodeExternal                    -536723455    E0024001
 kIOPSFamilyCodeExternal2                   -536723454    E0024002
 kIOPSFamilyCodeExternal3                   -536723453    E0024003
 kIOPSFamilyCodeExternal4                   -536723452    E0024004
 kIOPSFamilyCodeExternal5                   -536723451    E0024005
 */
charging_state_t is_charging(mach_port_t *family, device_info_t *info) {
    IOReturn result = kIOReturnSuccess;
    SMCKey key;
    int8_t charging = 0;
    bool ret = false;
    

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return kIsUnavail;
    
    /* AC-W(si8) Known cases */
    /* -1: No Adapter (M Chip) */
    /* 0: No Adapter (A Chip) */
    /* 1: Adapter at USB Port 1 */
    /* 2: Adapter at USB Port 2 */
    /* Consider use 'D*AP' for mobile devices (AppleSMCCharger::_checkConnection) */
    result = smc_read('AC-W', &charging);
    if (result != kIOReturnSuccess)
        return kIsUnavail;

    if (!charging || charging == -1)
        return kIsUnavail;

#if TARGET_OS_OSX || TARGET_OS_SIMULATOR
    uint16_t time_to_full;
    /* B0TF(ui16) TimeToFull */
    result = smc_read('B0TF', &time_to_full);
    if (result != kIOReturnSuccess)
        return kIsUnavail;

    /* Not charging, but Adapter attached */
    if (time_to_full == 65535)
        ret = kIsPausing;
    else
#endif
        ret = kIsCharging;

    /* kIOPSPowerAdapterFamily */
    if (family != NULL) {
        key = 'D\0FC' | ((0x30 + charging) << 0x10);
        result = smc_read(key, family);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Family Code: %X"), charging, family);
    }

    /* Not every charger sets those, no return on err */
    if (info != NULL) {
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

        /* CHI?(ui32) USB Port ? PMUConfiguration */
        key = 'CHI\0' | ((0x30 + charging) << 0x0);
        result = smc_read(key, &info->PMUConfiguration);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, PMUConfiguration: 0x%X"), charging, info->PMUConfiguration);

        /* D?IR(ui16) USB Port ? Current */
        key = 'D\0IR' | ((0x30 + charging) << 0x10);
        result = smc_read(key, &info->current);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Current: 0x%X"), charging, info->current);

        /* D?VR(ui16) USB Port ? Voltage */
        key = 'D\0IV' | ((0x30 + charging) << 0x10);
        result = smc_read(key, &info->voltage);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Voltage: %u"), charging, info->voltage);
        
        /* D?PM(hex_) USB Port ? Capabilities */
        key = 'D\0PM' | ((0x30 + charging) << 0x10);
        result = smc_read(key, info->hvc_menu);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Capabilities: 0x%X"), charging, info->hvc_menu);

        /* D?PI(si8 ) USB Port ? Capability Index */
        key = 'D\0PI' | ((0x30 + charging) << 0x10);
        result = smc_read(key, &info->hvc_index);
        if (result == kIOReturnSuccess)
            DBGLOG(CFSTR("Port: %d, Index: %d"), charging, info->hvc_index);
    }

    return ret;
}

/* Sadly we still have to get hvc_menu from IOPS, since Macs has no D?PM */
hvc_menu_t *hvc_menu_parse(uint8_t *input) {
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

    return menu;
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
 
    VBUS(ui32): SMC Detect Status

    MBSE(hex_): Sleep-Wake related
    MBSW(hex_): Sleep-Wake related
 
    UPOF(hex_): Shutdown data error flags
    UBNC(ui16): Shutdown nominal capacity
    UB0C(ui8 ): (write only) Shutdown data (write 1 to clear)
 
 Conditional:
    D?PI(ui8 ): USB Port ? HVC Index (Software HVC on Mac)

    VQ0u(ioft): VBUS Voltage
    
 */
    
