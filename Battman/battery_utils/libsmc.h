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
    uint16_t PresentDOD;            /* %/mAh */
    uint16_t DesignCapacity;        /* mAh */
    int16_t IMAX;                   /* mA */
    uint16_t NCC;
    int16_t ResScale;
    uint16_t ITMiscStatus;          /* Impedance Track™, this should be parseable but we don't know how */
    int16_t IMAX2;                  /* mA */
    uint32_t ChemID;                /* hex_[4] */
    int16_t SimRate;                /* Hr */

    /* Extensions */
    char DeviceName[32];
    uint16_t DailyMaxSoc;           /* % */
    uint16_t DailyMinSoc;           /* % */
    uint16_t DesignCycleCount;
    uint8_t UISoC;                  /* % */
    int8_t ChemicalSoC;             /* % */
    uint64_t bmsUpTime;             /* sec */
} gas_gauge_t;

typedef struct device_info {
    int8_t port;
    char firmware[12];
    char hardware[12];
    char adapter[32];
    char vendor[32];
    char name[32];
    char serial[32];
    char description[32];
    uint32_t PMUConfiguration;
    uint16_t current;
    uint16_t voltage;
    /* HVC (High Voltage Charging) */
    uint8_t hvc_menu[28]; /* hex_[28], 4 bit each hvc, max 7 hvc */
    int8_t hvc_index;
    /* Others */
    uint8_t port_type;
} device_info_t;

typedef struct cell_info {
    uint16_t Voltage;       /* BC?V */
    uint16_t Qmax;          /* BQX? */
    uint16_t DOD0;          /* BQD? */
    uint16_t PresentDOD;    /* BDD? */
    uint16_t WoM;           /* BMW? */
    uint8_t RaTableRaw[32]; /* B0R? */
} cell_info_t;

typedef struct lifetime_data {
    uint8_t ArrayA[32];                         /* BL0A */
    uint8_t ArrayB[32];                         /* BL0B */
    uint8_t CycleCountLastQmax;                 /* BLCC (Embedded Only) */
    uint16_t FlashWriteCount;                   /* BLCF (Conditional) */
    int16_t HighAverageCurrentLastRun;          /* BLCH (Conditional) */
    int16_t LowAverageCurrentLastRun;           /* BLCL (Conditional) */
    uint16_t ResistanceUpdatedDisabledCount;    /* BLCR (Conditional) */
    int16_t MinDeltaVoltage;                    /* BLDM (Conditional) */
    int16_t MaxDeltaVoltage;                    /* BLDX (Conditional) */
    int16_t MinFCC;                             /* BLFM (Conditional) */
    int16_t MaxFCC;                             /* BLFX (Conditional) */
    int16_t MaxChargeCurrent;                   /* BLIC */
    int16_t MaxDishargeCurrent;                 /* BLID */
    int16_t MinPackVoltage;                     /* BLPM */
    int16_t MaxPackVoltage;                     /* BLPX */
    int16_t MaxOverChargedCapacity;             /* BLQC (Conditional) */
    int16_t MaxOverDischargedCapacity;          /* BLQD (Conditional) */
    int16_t MinQmax;                            /* BLQM (Conditional) */
    int16_t MaxQmax;                            /* BLQX (Conditional) */
    uint16_t MinRa08;                           /* BLRM (Conditional) */
    uint16_t MaxRa08;                           /* BLRX (Conditional) */
    int8_t AverageTemperature;                  /* BLTA */
    int16_t MinTemperature;                     /* BLTM */
    uint32_t TotalOperatingTime;                /* BLTO */
    int32_t TemperatureSamples;                 /* BLTS */
    int16_t MaxTemperature;                     /* BLTX */
    uint8_t RdisCnt;                            /* BLCR (Conditional) */
    int16_t NCCMin;                             /* BLNM (Conditional) */
    int16_t NCCMax;                             /* BLNX (Conditional) */
    uint16_t MinRa8;                            /* BLRO (Conditional) */
    uint16_t MaxRa8;                            /* BLRN (Conditional) */
    uint8_t ResetCnt;                           /* BLRC (Conditional) */
    uint16_t QmaxUpdSucCnt;                     /* BLQN (Conditional) */
    uint16_t QmaxUpdFailCnt;                    /* BLQO (Conditional) */
    uint32_t TimeAtHighSoc[10];                 /* BLTP (Min 16, Max 40) */
} lifetime_data_t;

typedef struct shutdown_data {
    uint8_t UiSoc;                      /* UBUI */
    int16_t Temperature;                /* UBAT */
    uint16_t Voltage;                   /* UBAV */
    uint16_t PrevVoltage;               /* UBPV */
    uint16_t NominalChargeCapacity;     /* UBNC */
    uint16_t PrevNominalChargeCapacity; /* UBPN */
    uint16_t FullChargeCapacity;        /* UBFC */
    uint16_t PrevFullChargeCapacity;    /* UBPF */
    uint16_t RemainingCapacity;         /* UBPM */
    uint16_t PrevRemainingCapacity;     /* UBPR */
    int16_t AverageCurrent;             /* UBAC */
    int16_t PrevAverageCurrent;         /* UBPI */
    uint16_t RSS;                       /* UBSS */
    uint16_t DOD0;                      /* UBD0 */
    int16_t PresentDOD;                 /* UBDD */
    int16_t PassedCharge;               /* UBPC */
    uint16_t CycleCount;                /* UBCT */
    int16_t ResScale;                   /* UBRS */
    uint8_t CycleCountLastQmax;         /* UBCC */
    uint16_t MaxRa08;                   /* UBRX */
    uint8_t TimeAbove95;                /* UBTP */
    uint8_t RaTableRaw[32];             /* UBRA */
    int16_t MaxDischargeCurrent;        /* UBID */
    int16_t Qstart;                     /* UBQS */
    uint8_t DLog[64];                   /* UBDL */
    uint8_t UnexpectedRestart;          /* UPOR */
    uint64_t RestartTimestamp;          /* UB0T */
    uint8_t DataError;                  /* UPOF */
} shutdown_data_t;

typedef struct carrier_mode {
    uint32_t status;        /* CHTE */
    uint32_t high_voltage;  /* CHTU */
    uint32_t low_voltage;   /* CHTL */
    /* CHTM */
} carrier_mode_t;

/* This is not "Single App Mode", possibly enabled on Demo devices but cannot verify */
typedef struct kiosk_mode {
    /* CHKD */
    /* CHKG */
    /* CHKH */
    /* CHKK */
    uint8_t mode;               /* CHKM */
    uint16_t FullVoltage;       /* CHKL */
    uint32_t HighSocSecs;       /* CHKO */
    uint8_t HighSocDays;        /* CHKP */
    /* CHKQ */
    /* CHKR */
    /* CHKS */
    /* CHKT */
    /* CHKU */
    uint8_t LastHighSocHours;   /* CHKV */
    /* CHKW */
} kiosk_mode_t;

typedef struct charger_data {
    uint32_t ChargerConfiguration;      /* CHAS */
    uint32_t ChargingCurrent;           /* CHBI */
    uint32_t ChargingVoltage;           /* CHBV */
    uint16_t ChargerVacVoltageLimit;    /* BVVL */
    uint64_t NotChargingReason;         /* BNCR / CHNC */
    uint8_t ChargerStatus[64];          /* CHSL */
    uint32_t ChargerId;                 /* CH0D */
    uint8_t ChargerCapable;             /* CHCC */
    uint8_t ChargerExist;               /* CHCE */
    /* Charging Limits: CHA? CHI? CHP? */
} charger_data_t;

typedef struct hvc_menu {
    uint16_t current;
    uint16_t voltage;
} hvc_menu_t;

typedef enum {
    kIsUnavail = -1,    /* Error Occured */
    kIsNotCharging = 0, /* Not charging */
    kIsCharging,        /* Charging */
    kIsPausing,         /* AC connected, not charging */
} charging_state_t;

typedef struct power_state {
    float AdapterPower;
    float SystemPower;
} power_state_t;

typedef struct board_info {
    uint8_t Generation;         /* RGEN */
    char EmbeddedOSVersion[16]; /* RESV */
    uint64_t ChipEcid;          /* RECI */
    uint32_t ChipRev;           /* RCRV */
    uint32_t ChipId;            /* RCID */
    uint32_t BoardRev;          /* RBRV */
    uint32_t BoardId;           /* RBID */
    char TargetName[8];         /* RPlt */
} board_info_t;

typedef enum {
    kIsPresent = 1, /* Capable */
    kIsDetected,    /* Detected */
} wireless_state_t;

typedef struct iktara_fw {
    bool Charging;              /* (WAFS & 0xF000000) == 0xE000000 */
    bool Connected;             /* (WAFS >> 0x0B) & 1 */
    bool FieldPresent;          /* (WAFS >> 0x0A) & 1 */
    bool AppFWRunning;          /* (WAFS >> 0x09) & 1 */
    uint16_t ExceptionState;    /* (WAFS & 0x3F) */
    bool OvpTriggered;          /* (WAFS >> 0x1D) & 1 */
    bool LpmActive;             /* (WAFS >> 0x1C) & 1 */
} iktara_fw_t;

__BEGIN_DECLS

extern gas_gauge_t gGauge;
extern board_info_t gBoard;

board_info_t get_board_info(void);
int get_fan_status(void);
float get_temperature(void);
int get_time_to_empty(void);
int estimate_time_to_full(void);
float get_battery_health(float *design_cap, float *full_cap);
bool get_capacity(uint16_t *remaining, uint16_t *full, uint16_t *design);
int batt_cell_num(void);
bool get_gas_gauge(gas_gauge_t *gauge);
typedef unsigned int mach_port_t;
charging_state_t is_charging(mach_port_t *family, device_info_t *info);
float *get_temperature_per_cells(void);
bool battery_serial(char *serial);
hvc_menu_t *hvc_menu_parse(uint8_t *input, size_t *size);
const char *get_adapter_family_desc(mach_port_t family);
bool get_charger_data(charger_data_t *data);
char *not_charging_reason_str(uint64_t code);
char *port_type_str(uint8_t pt);

__END_DECLS

#endif
