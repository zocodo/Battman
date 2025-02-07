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

static UInt32 makeUInt32Key(char *keyString, int size, int base) {
    UInt32 total = 0;
    int i;

    for (i = 0; i < size; i++) {
        if (base == 16)
            total += keyString[i] << (size - 1 - i) * 8;
        else
            total += ((unsigned char)(keyString[i]) << (size - 1 - i) * 8);
    }
    return total;
}

__attribute__((destructor)) void smc_close(void) {
    if (gConn != 0)
        IOServiceClose(gConn);
}

int get_fan_status(void) {
    IOReturn result = kIOReturnSuccess;
    SMCKey key;
    char keyStr[5];
    uint8_t fan_num;
    int i;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return 0;

    key = makeUInt32Key("FNum", 4, 16);
    result = smc_read(key, &fan_num);
    /* No hardware fan support, or permission deined */
    if (result != kIOReturnSuccess)
        return 0;

    /* FNum(ui8) = 0, no fans on device */
    if (fan_num == 0)
        return 0;

    /* If have fans, check 'F*Ac', which is current speed */
    for (i = 0; i < fan_num; i++) {
        float retval;

        sprintf(keyStr, "F%dAc", i);
        key = makeUInt32Key(keyStr, 4, 16);
        result = smc_read(key, &retval);
        /* F*Ac(flt), return 1 if any fan working */
        if (retval > 0.0)
            return 1;
    }

    return 0;
}

float get_temperature(void) {
    IOReturn result = kIOReturnSuccess;
    float retval;
    SMCKey key;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return -1;

    /* TB*T(flt), but normally they are same */
    key = makeUInt32Key("TB0T", 4, 16);
    result = smc_read(key, &retval);
    if (result != kIOReturnSuccess)
        key = makeUInt32Key("B0AT", 4, 16);

    result = smc_read(key, &retval);
    if (result != kIOReturnSuccess)
        return -1;

    return retval;
}

int get_time_to_empty(void) {
    IOReturn result = kIOReturnSuccess;
    SMCKey key;
    uint16_t retval;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return 0;

#if TARGET_OS_EMBEDDED && !TARGET_OS_SIMULATOR
    /* This is weird, why B0TF means TimeToEmpty on Embedded,
     * but TimeToFullCharge on macOS? */
    key = makeUInt32Key("B0TF", 4, 16);
    /* Tested on iPhone 12 mini: B0TF does not exist */
    result = smc_read(key, &retval);
    if (result != kIOReturnSuccess)
        key = makeUInt32Key("B0TE", 4, 16);
#else
    key = makeUInt32Key("B0TE", 4, 16);
#endif

    result = smc_read(key, &retval);
    if (result != kIOReturnSuccess)
        return 0;

    /* 0xFFFF, battery charging (known scene, possibly others) */
    if (retval == 65535)
        return -1;

    return retval;
}

int estimate_time_to_full() {
    IOReturn result = kIOReturnSuccess;
    SMCKey key;
    int16_t current;
    uint16_t fullcap;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return 0;

    /* B0FC(ui16) FullChargeCapacity (mAh) */
    key = makeUInt32Key("B0FC", 4, 16);
    result = smc_read(key, &fullcap);
    if (result != kIOReturnSuccess)
        return 0;

    /* B0AC(si16) AverageCurrent (mA) */
    key = makeUInt32Key("B0AC", 4, 16);
    result = smc_read(key, &current);
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
    SMCKey key;
    uint16_t fullcap;
    uint16_t designcap;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return 0;

    /* B0FC(ui16) FullChargeCapacity (mAh) */
    key = makeUInt32Key("B0FC", 4, 16);
    result = smc_read(key, &fullcap);
    if (result != kIOReturnSuccess)
        return 0;

    /* B0DC(ui16) DesignCapacity (mAh) */
    key = makeUInt32Key("B0DC", 4, 16);
    result = smc_read(key, &designcap);
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

    /* B0RM(ui16) RemainingCapacity (mAh) */
    SMCKey key = makeUInt32Key("B0RM", 4, 16);
    IOReturn result = smc_read(key, &B0RM);
    if (result != kIOReturnSuccess)
        return false;

    /* B0FC(ui16) FullChargeCapacity (mAh) */
    key = makeUInt32Key("B0FC", 4, 16);
    result = smc_read(key, &B0FC);
    if (result != kIOReturnSuccess)
        return false;

    /* B0DC(ui16) DesignCapacity (mAh) */
    key = makeUInt32Key("B0DC", 4, 16);
    result = smc_read(key, &B0DC);

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
    SMCKey key;
    uint16_t ui16ret = 0;
    uint32_t ui32ret = 0;
    int16_t si16ret = 0;
    int32_t si32ret = 0;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return false;

    /* TODO: Shorten those code */

    /* B0AT(ui16): Temperature */
    key = makeUInt32Key("B0AT", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->Temperature = ui16ret;

    /* B0AV(ui16): Average Voltage */
    key = makeUInt32Key("B0AV", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->Voltage = ui16ret;
    
    /* B0FI(hex_): Flags */
    key = makeUInt32Key("B0FI", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->Flags = ui16ret;
    
    /* B0RM(ui16): RemainingCapacity */
    key = makeUInt32Key("B0RM", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->RemainingCapacity = ui16ret;
    
    /* B0FC(ui16): FullChargeCapacity */
    key = makeUInt32Key("B0FC", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->FullChargeCapacity = ui16ret;

    /* B0AC(si16): AverageCurrent */
    key = makeUInt32Key("B0AC", 4, 16);
    result = smc_read(key, &si16ret);
    if (result == kIOReturnSuccess)
        gauge->AverageCurrent = si16ret;

    /* B0TF(ui16): TimeToEmpty */
    key = makeUInt32Key("B0TF", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->TimeToEmpty = ui16ret;

    /* BQX1(ui16): Qmax */
    key = makeUInt32Key("BQX1", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->Qmax = ui16ret;

    /* B0AP(si16/si32): AveragePower */
    key = makeUInt32Key("B0AP", 4, 16);
    result = smc_read(key, &si32ret);
    if (result == kIOReturnSuccess)
        gauge->AveragePower = si32ret;

    /* B0OC(si16): OCV_Current */
    key = makeUInt32Key("B0OC", 4, 16);
    result = smc_read(key, &si16ret);
    if (result == kIOReturnSuccess)
        gauge->OCV_Current = si16ret;

    /* B0OV(ui16): OCV_Voltage */
    key = makeUInt32Key("B0OV", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->OCV_Voltage = ui16ret;

    /* B0CT(ui16): CycleCount */
    key = makeUInt32Key("B0CT", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->CycleCount = ui16ret;

    /* BRSC(ui16): StateOfCharge */
    key = makeUInt32Key("BRSC", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->StateOfCharge = ui16ret;

    /* B0TC(si16): TrueRemainingCapacity */
    key = makeUInt32Key("B0TC", 4, 16);
    result = smc_read(key, &si16ret);
    if (result == kIOReturnSuccess)
        gauge->TrueRemainingCapacity = si16ret;

    /* BQCC(si16): PassedCharge */
    key = makeUInt32Key("BQCC", 4, 16);
    result = smc_read(key, &si16ret);
    if (result == kIOReturnSuccess)
        gauge->PassedCharge = si16ret;

    /* BQD1(ui16): DOD0 */
    key = makeUInt32Key("BQD1", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->DOD0 = ui16ret;

    /* B0DC(ui16): DesignCapacity */
    key = makeUInt32Key("B0DC", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->DesignCapacity = ui16ret;

    /* B0IM(si16): IMAX */
    key = makeUInt32Key("B0IM", 4, 16);
    result = smc_read(key, &si16ret);
    if (result == kIOReturnSuccess)
        gauge->IMAX = si16ret;

    /* B0NC(ui16): NCC */
    key = makeUInt32Key("B0NC", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->NCC = ui16ret;

    /* B0RS(si16): ResScale */
    key = makeUInt32Key("B0RS", 4, 16);
    result = smc_read(key, &si16ret);
    if (result == kIOReturnSuccess)
        gauge->ResScale = si16ret;

    /* B0MS(ui16): ITMiscStatus */
    key = makeUInt32Key("B0MS", 4, 16);
    result = smc_read(key, &ui16ret);
    if (result == kIOReturnSuccess)
        gauge->ITMiscStatus = ui16ret;

    /* B0I2(si16): IMAX2 */
    key = makeUInt32Key("B0I2", 4, 16);
    result = smc_read(key, &si16ret);
    if (result == kIOReturnSuccess)
        gauge->IMAX2 = si16ret;

    /* B0CI(hex_): ChemID */
    key = makeUInt32Key("B0CI", 4, 16);
    result = smc_read(key, &ui32ret);
    if (result == kIOReturnSuccess)
        gauge->ChemID = ui32ret;

    /* B0SR(si16): SimRate */
    key = makeUInt32Key("B0SR", 4, 16);
    result = smc_read(key, &si16ret);
    if (result == kIOReturnSuccess)
        gauge->SimRate = si16ret;

    return true;
}

/* -1: Unknown */
int battery_num(void) {
    IOReturn result = kIOReturnSuccess;
    SMCKey key;
    int8_t count = 0;

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return -1;
    
    /* BNCB(si8) Number of Chargable Batteries (Guessed) */
    key = makeUInt32Key("BNCB", 4, 16);
    result = smc_read(key, &count);
    if (result != kIOReturnSuccess)
        return -1;
    
    return (int)count;
}

bool is_charging(io_object_t family, char *type, char *manufacturer, char *name, char *serial) {
    IOReturn result = kIOReturnSuccess;
    SMCKey key;
    uint8_t charging;
    uint32_t family_code; /* D*FC */
    

    if (gConn == 0)
        result = smc_open();

    if (result != kIOReturnSuccess)
        return false;
    
    /* AC-W(si8) Known cases */
    /* -1: Uncharging (M Chip) */
    /* 0: Uncharging (A Chip) */
    /* 1: Charging at USB Port 1 */
    /* 2: Charging at USB Port 2 */
    /* Consider use 'D*AP' for mobile devices (AppleSMCCharger::_checkConnection) */
    key = makeUInt32Key("AC-W", 4, 16);
    result = smc_read(key, &charging);
    if (result != kIOReturnSuccess)
        return false;

#if TARGET_OS_OSX
    uint16_t time_to_full;
    /* B0TF(ui16) TimeToFull */
    key = makeUInt32Key("B0TF", 4, 16);
    result = smc_read(key, &time_to_full);
    if (result != kIOReturnSuccess)
        return false;
    /* Not charging */
    if (time_to_full == 65535)
        return false;
#endif

    /* D?if(ch8*) USB Port ? Firmware version */
    /* D?ih(ch8*) USB Port ? Hardware version */
    /* D?ii(ch8*) USB Port ? Adapter Model */
    /* D?im(ch8*) USB Port ? Vendor */
    /* D?in(ch8*) USB Port ? Name */
    /* D?is(ch8*) USB Port ? Serial */

    /* Not every charger sets those */
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
    D?PI(ui8 ): USB Port ? HVC Index
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
    VQ0u(ioft): VBUS Voltage
    
 */
    
