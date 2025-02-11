#ifndef __libsmc_h__
#define __libsmc_h__

#include <CoreFoundation/CFBase.h>
#include <stdio.h>

#if !defined(__arm64__) && !defined(__aarch64__) && !defined(__arm64e__)
#error Current SMC implementation is arm64 only! \
       Please file an issue if you would like to contribute!
#endif

/* SMC operations */
typedef CF_ENUM(UInt8, SMCIndex) {
    /* the user client method name constants */
    kSMCUserClientOpen,
    kSMCUserClientClose,
    kSMCHandleYPCEvent,

    kSMCPlaceholder1, /* *** LEGACY SUPPORT placeholder */
    kSMCNumberOfMethods,

    /* other constants not mapped to individual methods */
    kSMCReadKey,
    kSMCWriteKey,
    kSMCGetKeyCount,
    kSMCGetKeyFromIndex,
    kSMCGetKeyInfo,

    kSMCFireInterrupt,
    kSMCGetPLimits,
    kSMCGetVers,
    kSMCPlaceholder2, /* *** LEGACY SUPPORT placeholder */

    kSMCReadStatus,
    kSMCReadResult,

    kSMCVariableCommand
};

typedef UInt32 SMCKey;
typedef UInt32 SMCDataType;
typedef UInt8 SMCDataAttributes;

/* a struct to hold the SMC version */
typedef struct SMCVersion {
    unsigned char major;
    unsigned char minor;
    unsigned char build;
    unsigned char reserved; // padding for alignment
    unsigned short release;
} SMCVersion;

typedef struct SMCPLimitData {
    UInt16 version;
    UInt16 length;
    UInt32 cpuPLimit;
    UInt32 gpuPLimit;
    UInt32 memPLimit;
} SMCPLimitData;

/* a struct to hold the key info data */
typedef struct SMCKeyInfoData {
    UInt32 dataSize;
    SMCDataType dataType;
    SMCDataAttributes dataAttributes;
} SMCKeyInfoData;

/* the struct passed back and forth between the kext and UC */
/* sizeof(SMCParamStruct) should be 168 or 80, depending on whether uses
 * bytes[32] or bytes[120] */
typedef struct SMCParamStruct {
    SMCKey key;
    struct SMCParam {
        SMCVersion vers;
        SMCPLimitData pLimitData;
        SMCKeyInfoData keyInfo;

        UInt8 result;
        UInt8 status;

        UInt8 data8;
        UInt32 data32;
        UInt8 bytes[120];
    } param;
} SMCParamStruct;

/* libsmc.c */

typedef struct gas_gauge {
    uint16_t Temperature;           /* Celsius */
    uint16_t Voltage;               /* mV */
    uint16_t Flags;                 /* hex_[2] */
    uint16_t RemainingCapacity;     /* mAh */
    uint16_t FullChargeCapacity;    /* mAh */
    int16_t AverageCurrent;         /* mA */
    uint16_t TimeToEmpty;           /* min */
    uint16_t Qmax;                  /* mAh */
    int16_t AveragePower;           /* mW */
    int16_t OCV_Current;            /* mA */
    uint16_t OCV_Voltage;           /* mV */
    uint16_t CycleCount;
    uint16_t StateOfCharge;         /* % */
    int16_t TrueRemainingCapacity;  /* mAh */
    int16_t PassedCharge;           /* mAh */
    uint16_t DOD0;                  /* mAh */
    uint16_t PresentDOD;            /* mAh */
    uint16_t DesignCapacity;        /* mAh */
    int16_t IMAX;                   /* ? */
    uint16_t NCC;                   /* mAh ? */
    int16_t ResScale;
    uint16_t ITMiscStatus;
    int16_t IMAX2;                  /* ? */
    uint32_t ChemID;                /* hex_[4] */
    int16_t SimRate;                /* mA ? */
} gas_gauge_t;

typedef struct device_info {
    char firmware[12];
    char hardware[12];
    char adapter[32];
    char vendor[32];
    char name[32];
    char serial[32];
} device_info_t;

typedef enum {
    kIsCharging,    /* Charging */
    kIsNotCharging, /* Not charging */
    kIsPausing,     /* AC connected, not charging */
    kIsUnavail,     /* Error Occured */
} charging_state_t;

int get_fan_status(void);
float get_temperature(void);
int get_time_to_empty(void);
int estimate_time_to_full(void);
float get_battery_health(float *design_cap, float *full_cap);
bool get_capacity(uint16_t *remaining, uint16_t *full, uint16_t *design);
int battery_num(void);
bool get_gas_gauge(gas_gauge_t *gauge);
typedef unsigned int mach_port_t;
charging_state_t is_charging(mach_port_t family, device_info_t *info);
float *get_temperature_per_batt(void);

#endif
