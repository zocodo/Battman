//
//  iokit_connection.h
//  Battman
//
//  Created by Torrekie on 2025/2/9.
//

#ifndef iokit_connection_h
#define iokit_connection_h

#include <stdio.h>

/* IOThunderboltController Power State */
#define kThunderboltPMSleepState 0
#define kThunderboltPMPauseState 1
#define kThunderboltPMWakeState  2

/* Known NotChargingReason */
/*
 0: None
 NotChargingReason & 0x10: ChargerTimeout
 NotChargingReason & 0x20: ChargerWatchdogTimeout
 0x80: Not Connected (Refer to Charger Status)
 0x2000: Connecting
 0x400001: Fully Charged
 */

/* Known ChargerStatus */
/*
 Base: 0x8A
 0: None
 0xB0:
 0xB4: Stopped (charged?)
 0xD4: Not Connected
 0xD8: Charging
 0xB8: Stopped
 */
 
#endif /* iokit_connection_h */
