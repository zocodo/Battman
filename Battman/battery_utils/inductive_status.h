//
//  inductive_status.h
//  Battman
//
//  Created by Torrekie on 2025/4/16.
//

#ifndef inductive_status_h
#define inductive_status_h

#if !defined(__arm64__) && !defined(__aarch64__) && !defined(__arm64e__)
#warning This version of inductive_status.h is not for your arch!
#endif

/* Basic status */
/**
 * @const   INDUCTIVE_STATUS_INSUFF_POWER
 * @brief   The transmitter cannot supply sufficient power at the current alignment or distance.
 */
#define INDUCTIVE_STATUS_INSUFF_POWER               ((uint64_t)0x01 << 0x00)

/**
 * @const   INDUCTIVE_STATUS_EXCEPTION
 * @brief   A general hardware or firmware exception has occurred.
 */
#define INDUCTIVE_STATUS_EXCEPTION                  ((uint64_t)0x01 << 0x01)

/**
 * @const   INDUCTIVE_STATUS_VERIFY_FAILED
 * @brief   Firmware or authentication signature verification failed.
 */
#define INDUCTIVE_STATUS_VERIFY_FAILED              ((uint64_t)0x01 << 0x02)

/**
 * @const   INDUCTIVE_STATUS_FW_DOWNLOADED
 * @brief   Controller firmware image has been successfully downloaded.
 */
#define INDUCTIVE_STATUS_FW_DOWNLOADED              ((uint64_t)0x01 << 0x03)

/* INDUCTIVE_STATUS_CLOAK_REASONS (0–63) */
/* Not_Cloaked = 0 */
/**
 * @const   INDUCTIVE_STATUS_CLOAK_REASONS
 * @brief   Bitmask covering all cloaking reasons (bits 4–9).
 */
#define INDUCTIVE_STATUS_CLOAK_REASONS_SHIFT        0x04
#define INDUCTIVE_STATUS_CLOAK_REASONS              ((uint64_t)0x3F << INDUCTIVE_STATUS_CLOAK_REASONS_SHIFT)
/* inductive_data.status & (INDUCTIVE_CLOAK_REASON_[] << INDUCTIVE_STATUS_CLOAK_REASONS_SHIFT) */

/**
 * @const   INDUCTIVE_CLOAK_REASON_EXT
 * @brief   External fault (e.g., foreign-object detected), transmitter coil cloaked.
 */
#define INDUCTIVE_CLOAK_REASON_EXT                  ((uint64_t)0x01)

/**
 * @const   INDUCTIVE_CLOAK_REASON_HOT
 * @brief   Over-temperature detected; transmitter coil cloaked until safe.
 */
#define INDUCTIVE_CLOAK_REASON_HOT                  ((uint64_t)0x02)

/**
 * @const   INDUCTIVE_CLOAK_REASON_EOC
 * @brief   End-of-charge reached; transmitter coil cloaked.
 */
#define INDUCTIVE_CLOAK_REASON_EOC                  ((uint64_t)0x04)

/**
 * @const   INDUCTIVE_CLOAK_REASON_COEX
 * @brief   RF coexistence event (e.g., Wi-Fi/Bluetooth interference) triggered cloaking.
 */
#define INDUCTIVE_CLOAK_REASON_COEX                 ((uint64_t)0x08)

/**
 * @const   INDUCTIVE_CLOAK_REASON_OBC
 * @brief   Over-battery-current condition; transmitter coil cloaked.
 */
#define INDUCTIVE_CLOAK_REASON_OBC                  ((uint64_t)0x10)

/**
 * @const   INDUCTIVE_CLOAK_REASON_TX
 * @brief   Transmitter port busy (e.g., multi-protocol negotiation); coil cloaked.
 */
#define INDUCTIVE_CLOAK_REASON_TX                   ((uint64_t)0x20)

/* INDUCTIVE_STATUS_AUTH_STATUS (0–3) */
/**
 * @const   INDUCTIVE_STATUS_AUTH_STATUS
 * @brief   Bitmask for authentication handshake status (bits 10–11).
 */
#define INDUCTIVE_STATUS_AUTH_STATUS_SHIFT          0x0A
#define INDUCTIVE_STATUS_AUTH_STATUS                ((uint64_t)0x03 << INDUCTIVE_STATUS_AUTH_STATUS_SHIFT)

/**
 * @const   INDUCTIVE_AUTH_STATUS_NONE
 * @brief   No charger authentication attempt.
 */
#define INDUCTIVE_AUTH_STATUS_NONE                  ((uint64_t)0x00)

/**
 * @const   INDUCTIVE_AUTH_STATUS_BUSY
 * @brief   Charger authentication handshake in progress.
 */
#define INDUCTIVE_AUTH_STATUS_BUSY                  ((uint64_t)0x01)

/**
 * @const   INDUCTIVE_AUTH_STATUS_PASSED
 * @brief   Charger authentication succeeded.
 */
#define INDUCTIVE_AUTH_STATUS_PASSED                ((uint64_t)0x02)

/**
 * @const   INDUCTIVE_AUTH_STATUS_FAILED
 * @brief   Charger authentication failed.
 */
#define INDUCTIVE_AUTH_STATUS_FAILED                ((uint64_t)0x03)

/* CL status */
/**
 * @const   INDUCTIVE_STATUS_CL_PWR_TRANSIENT
 * @brief   Coil power transient event detected.
 */
#define INDUCTIVE_STATUS_CL_PWR_TRANSIENT           ((uint64_t)0x01 << 0x0C)

/**
 * @const   INDUCTIVE_STATUS_CL_PWR_LIMITED
 * @brief   Coil power is currently limited (e.g., for FOD or thermal reasons).
 */
#define INDUCTIVE_STATUS_CL_PWR_LIMITED             ((uint64_t)0x01 << 0x0D)

/**
 * @const   INDUCTIVE_STATUS_CL_ILIM_FROZEN
 * @brief   Current-limit register frozen until next reset.
 */
#define INDUCTIVE_STATUS_CL_ILIM_FROZEN             ((uint64_t)0x01 << 0x0E)

/**
 * @const   INDUCTIVE_STATUS_QUIESCED
 * @brief   Coil is in a low-power idle (quiesced) state.
 */
#define INDUCTIVE_STATUS_QUIESCED                   ((uint64_t)0x01 << 0x0F)

/**
 * @const   INDUCTIVE_STATUS_LDO_DISABLED
 * @brief   Onboard LDO regulator disabled until VRECT threshold is met.
 */
#define INDUCTIVE_STATUS_LDO_DISABLED               ((uint64_t)0x01 << 0x10)

/**
 * @const   INDUCTIVE_STATUS_ON_MAT_RAW
 * @brief   Raw mat-presence detected before full handshake.
 */
#define INDUCTIVE_STATUS_ON_MAT_RAW                 ((uint64_t)0x01 << 0x11)

/**
 * @const   INDUCTIVE_STATUS_CL_ACTIVE
 * @brief   Coil is actively transmitting power.
 */
#define INDUCTIVE_STATUS_CL_ACTIVE                  ((uint64_t)0x01 << 0x12)

/**
 * @const   INDUCTIVE_STATUS_HB_FAILED
 * @brief   Heartbeat ping from host was not received.
 */
#define INDUCTIVE_STATUS_HB_FAILED                  ((uint64_t)0x01 << 0x13)

/**
 * @const   INDUCTIVE_STATUS_COEX_LIMITED
 * @brief   Coil power limited due to RF coexistence constraints.
 */
#define INDUCTIVE_STATUS_COEX_LIMITED               ((uint64_t)0x01 << 0x14)

/**
 * @const   INDUCTIVE_STATUS_LDO_LIMITED
 * @brief   LDO current is being dynamically limited.
 */
#define INDUCTIVE_STATUS_LDO_LIMITED                ((uint64_t)0x01 << 0x15)

/**
 * @const   INDUCTIVE_STATUS_HIGH_TEMP_DISC
 * @brief   Coil disabled due to high-temperature disconnect trip.
 */
#define INDUCTIVE_STATUS_HIGH_TEMP_DISC             ((uint64_t)0x01 << 0x16)

/**
 * @const   INDUCTIVE_STATUS_UID_ROLLED
 * @brief   Unique device ID counter has rolled over.
 */
#define INDUCTIVE_STATUS_UID_ROLLED                 ((uint64_t)0x01 << 0x17)

/* INDUCTIVE_STATUS_DRV_STATE/kBcm5935xState (u16) */
/**
 * @const   INDUCTIVE_STATUS_DRV_STATE
 * @brief   Mask covering driver state bits (24–31).
 */
#define INDUCTIVE_STATUS_DRV_STATE_SHIFT            0x18
#define INDUCTIVE_STATUS_DRV_STATE                  ((uint64_t)0xFF << INDUCTIVE_STATUS_DRV_STATE_SHIFT)

/**
 * @const   kBcm5935xStateUnknown
 * @brief   Initial or unknown driver state.
 */
#define kBcm5935xStateUnknown                       ((uint64_t)0x00)

/**
 * @const   kBcm5935xStateDBB
 * @brief   Dead-battery bootloader mode.
 */
#define kBcm5935xStateDBB                           ((uint64_t)0x01)

/**
 * @const   kBcm5935xStateFWDL
 * @brief   In firmware-download bootloader.
 */
#define kBcm5935xStateFWDL                          ((uint64_t)0x02)

/**
 * @const   kBcm5935xStateMain
 * @brief   Main charging operation.
 */
#define kBcm5935xStateMain                          ((uint64_t)0x03)

/**
 * @const   kBcm5935xStateMainLPM
 * @brief   Main operation in low-power mode.
 */
#define kBcm5935xStateMainLPM                       ((uint64_t)0x04)

/**
 * @const   kBcm5935xStateNoBackpower
 * @brief   No back-power path present (host off).
 */
#define kBcm5935xStateNoBackpower                   ((uint64_t)0x05)

/**
 * @const   kBcm5935xStateCloaked
 * @brief   Coil is currently cloaked (disabled).
 */
#define kBcm5935xStateCloaked                       ((uint64_t)0x06)

/**
 * @const   kBcm5935xStateException
 * @brief   Exception handler active.
 */
#define kBcm5935xStateException                     ((uint64_t)0x10)

/* INDUCTIVE_STATUS_SS_VRECT (u16) */
/**
 * @const   INDUCTIVE_STATUS_SS_VRECT
 * @brief   Mask for secondary-side rectifier voltage sample bits (32–39).
 */
#define INDUCTIVE_STATUS_SS_VRECT_SHIFT             0x20
#define INDUCTIVE_STATUS_SS_VRECT                   ((uint64_t)0xFF << INDUCTIVE_STATUS_SS_VRECT_SHIFT)

/* INDUCTIVE_STATUS_2PP_STATE (u8) */
/**
 * @const   INDUCTIVE_STATUS_2PP_STATE
 * @brief   Mask for two-phase power profile (EPP) state bits (40–43).
 */
#define INDUCTIVE_STATUS_2PP_STATE_SHIFT            0x28
#define INDUCTIVE_STATUS_2PP_STATE                  ((uint64_t)0x0F << INDUCTIVE_STATUS_2PP_STATE_SHIFT)

/* INDUCTIVE_STATUS_ILOAD_MOD (u8) */
/**
 * @const   INDUCTIVE_STATUS_ILOAD_MOD
 * @brief   Mask for load-current modulation index bits (44–47).
 */
#define INDUCTIVE_STATUS_ILOAD_MOD_SHIFT            0x2C
#define INDUCTIVE_STATUS_ILOAD_MOD                  ((uint64_t)0x0F << INDUCTIVE_STATUS_ILOAD_MOD_SHIFT)

/* INDUCTIVE_STATUS_NOT_CLOAKING_REASON (u16) */
/**
 * @const   INDUCTIVE_STATUS_NOT_CLOAKING_REASON
 * @brief   Mask covering non-cloaking reasons (bits 48–55).
 */
#define INDUCTIVE_STATUS_NOT_CLOAKING_REASON_SHIFT  0x30
#define INDUCTIVE_STATUS_NOT_CLOAKING_REASON        ((uint64_t)0xFF << INDUCTIVE_STATUS_NOT_CLOAKING_REASON_SHIFT)

/**
 * @const   INDUCTIVE_NOT_CLOAKING_REASON_NOT_ALLOWED
 * @brief   Cloaking disallowed by configuration or policy.
 */
#define INDUCTIVE_NOT_CLOAKING_REASON_NOT_ALLOWED   ((uint64_t)0x01)

/**
 * @const   INDUCTIVE_NOT_CLOAKING_REASON_ROLE_SWAP
 * @brief   Power role swap in progress; cloaking inhibited.
 */
#define INDUCTIVE_NOT_CLOAKING_REASON_ROLE_SWAP     ((uint64_t)0x02)

/**
 * @const   INDUCTIVE_NOT_CLOAKING_REASON_NOT_READY
 * @brief   System not ready for cloaking (e.g., busy booting).
 */
#define INDUCTIVE_NOT_CLOAKING_REASON_NOT_READY     ((uint64_t)0x04)

/**
 * @const   INDUCTIVE_NOT_CLOAKING_REASON_AUTH
 * @brief   Authentication handshake pending; cloaking held.
 */
#define INDUCTIVE_NOT_CLOAKING_REASON_AUTH          ((uint64_t)0x08)

/**
 * @const   INDUCTIVE_NOT_CLOAKING_REASON_QUIESCE
 * @brief   System quiescing; cloaking disabled.
 */
#define INDUCTIVE_NOT_CLOAKING_REASON_QUIESCE       ((uint64_t)0x10)

/**
 * @const   INDUCTIVE_NOT_CLOAKING_REASON_POWER_PAUSE
 * @brief   Temporary power-pause state; cloaking inhibited.
 */
#define INDUCTIVE_NOT_CLOAKING_REASON_POWER_PAUSE   ((uint64_t)0x20)

/**
 * @const   INDUCTIVE_NOT_CLOAKING_REASON_DEMO
 * @brief   Demo or test mode; cloaking disabled.
 */
#define INDUCTIVE_NOT_CLOAKING_REASON_DEMO          ((uint64_t)0x40)

/**
 * @const   INDUCTIVE_NOT_CLOAKING_REASON_AP_WAKE
 * @brief   Application processor wake event; cloaking inhibited.
 */
#define INDUCTIVE_NOT_CLOAKING_REASON_AP_WAKE       ((uint64_t)0x80)

/**
 * @const   INDUCTIVE_STATUS_SYS_TRANS
 * @brief   System transition event flag (bit 56) for high-level state changes.
 */
#define INDUCTIVE_STATUS_SYS_TRANS                  ((uint64_t)0x01 << 0x38)


#define INDUCTIVE_FW_MODE_RX 0
#define INDUCTIVE_FW_MODE_TX 1


#define HAL_INDUCTIVE_DEVICE_BCM5935X   3

#endif /* inductive_status_h */
