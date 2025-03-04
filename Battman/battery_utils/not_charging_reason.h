//
//  not_charging_reason.h
//  Battman
//
//  Created by Torrekie on 2025/3/4.
//

#ifndef not_charging_reason_h
#define not_charging_reason_h

// Not sure how was the original header named
// Use: if (NotChargingReason & NOT_CHARGING_DEFS)

// TODO: Better formatting

#define NOT_CHARGING_REASON_PERMANENT_FAULT_MASK                0xFF00000000000000LL
#define NOT_CHARGING_REASON_VBAT_VFAULT                         0x1000000000000000LL

#define NOT_CHARGING_REASON_FULLY_CHARGED                       0x1
#define NOT_CHARGING_REASON_TEMP_BELOW_MIN_STOP_CHARGING        0x2
#define NOT_CHARGING_REASON_TEMP_ABOVE_MAX_STOP_CHARGING        0x4
#define NOT_CHARGING_REASON_TEMP_BELOW_MIN_START_CHARGING       0x8
#define NOT_CHARGING_REASON_TEMP_ABOVE_MAX_START_CHARGING       0x10

#define NOT_CHARGING_REASON_BATTERY_NOT_PRESENT                 0x40
#define NOT_CHARGING_REASON_VBUS_NOT_PRESENT                    0x80

#define NOT_CHARGING_REASON_INHIBIT_INFLOW_BATTERY_NOT_PRESENT  0x40000
#define NOT_CHARGING_REASON_CHARGER_COMMUNICATION_FAILED        0x400000000000000LL
#define NOT_CHARGING_REASON_INHIBIT_INFLOW                      0x40000000000000LL

#define NOT_CHARGING_REASON_IOAM                                0x400
#define NOT_CHARGING_REASON_KIOSK_MODE                          0x800
#define NOT_CHARGING_REASON_COREMOTION                          0x1000
#define NOT_CHARGING_REASON_USBPD                               0x2000

#define NOT_CHARGING_REASON_FIELDDIAGS                          0x20000000000000LL

#define NOT_CHARGING_REASON_INHIBIT_CLIENT_ADAPTER              0x100000

#define NOT_CHARGING_REASON_SETBATT                             0x4000
#define NOT_CHARGING_REASON_PREDICTIVECHARGING                  0x8000
#define NOT_CHARGING_REASON_INDUCTIVE                           0x10000
#define NOT_CHARGING_REASON_GG_FW_UPDATE                        0x20000

#define NOT_CHARGING_REASON_HIGH_SOC_HIGH_TEMP_STOP_CHARGING    0x100

#define NOT_CHARGING_REASON_IBAT_MINFAULT                       0x2000000000000000LL

#define NOT_CHARGING_REASON_CHARGE_TIMER_EXPIRED                0x20

#define NOT_CHARGING_REASON_CARRIER_TEST                        0x80000000000000LL

#define BATTERY_CHARGING_ENABLE_EVENT                           0

#endif /* not_charging_reason_h */
