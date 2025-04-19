//
//  inductive_status.h
//  Battman
//
//  Created by Torrekie on 2025/4/16.
//

#ifndef inductive_status_h
#define inductive_status_h

#if !defined(__arm64__) || !defined(__aarch64__) || !defined(__arm64e__)
#warning This version of inductive_status.h is not for your arch!
#endif

/* Basic status */
#define INDUCTIVE_STATUS_INSUFF_POWER               (uint64_t)(0x01 << 0x00)
#define INDUCTIVE_STATUS_EXCEPTION                  (uint64_t)(0x01 << 0x01)
#define INDUCTIVE_STATUS_VERIFY_FAILED              (uint64_t)(0x01 << 0x02)
#define INDUCTIVE_STATUS_FW_DOWNLOADED              (uint64_t)(0x01 << 0x03)

/* INDUCTIVE_STATUS_CLOAK_REASONS (0-63) */
/* Not_Cloaked = 0 */
#define INDUCTIVE_STATUS_CLOAK_REASONS              (uint64_t)(0x3F << 0x04) /* Mask */
#define INDUCTIVE_CLOAK_REASON_EXT                  (uint64_t)(0x01 << 0x04)
#define INDUCTIVE_CLOAK_REASON_HOT                  (uint64_t)(0x02 << 0x04)
#define INDUCTIVE_CLOAK_REASON_EOC                  (uint64_t)(0x04 << 0x04)
#define INDUCTIVE_CLOAK_REASON_COEX                 (uint64_t)(0x08 << 0x04)
#define INDUCTIVE_CLOAK_REASON_OBC                  (uint64_t)(0x10 << 0x04)
#define INDUCTIVE_CLOAK_REASON_TX                   (uint64_t)(0x20 << 0x04)

/* INDUCTIVE_STATUS_AUTH_STATUS (0-3) */
#define INDUCTIVE_STATUS_AUTH_STATUS                (uint64_t)(0x03 << 0x0A) /* Mask */
#define INDUCTIVE_AUTH_STATUS_NONE                  (uint64_t)(0x00 << 0x0A)
#define INDUCTIVE_AUTH_STATUS_BUSY                  (uint64_t)(0x01 << 0x0A)
#define INDUCTIVE_AUTH_STATUS_PASSED                (uint64_t)(0x02 << 0x0A)
#define INDUCTIVE_AUTH_STATUS_FAILED                (uint64_t)(0x03 << 0x0A)

/* CL status */
#define INDUCTIVE_STATUS_CL_PWR_TRANSIENT           (uint64_t)(0x01 << 0x0C)
#define INDUCTIVE_STATUS_CL_PWR_LIMITED             (uint64_t)(0x01 << 0x0D)
#define INDUCTIVE_STATUS_CL_ILIM_FROZEN             (uint64_t)(0x01 << 0x0E)

#define INDUCTIVE_STATUS_QUIESCED                   (uint64_t)(0x01 << 0x0F)
#define INDUCTIVE_STATUS_LDO_DISABLED               (uint64_t)(0x01 << 0x10)
#define INDUCTIVE_STATUS_ON_MAT_RAW                 (uint64_t)(0x01 << 0x11)
#define INDUCTIVE_STATUS_CL_ACTIVE                  (uint64_t)(0x01 << 0x12)
#define INDUCTIVE_STATUS_HB_FAILED                  (uint64_t)(0x01 << 0x13)
#define INDUCTIVE_STATUS_COEX_LIMITED               (uint64_t)(0x01 << 0x14)
#define INDUCTIVE_STATUS_LDO_LIMITED                (uint64_t)(0x01 << 0x15)
#define INDUCTIVE_STATUS_HIGH_TEMP_DISC             (uint64_t)(0x01 << 0x16)
#define INDUCTIVE_STATUS_UID_ROLLED                 (uint64_t)(0x01 << 0x17)

/* INDUCTIVE_STATUS_DRV_STATE/kBcm5935xState (u16) */
#define INDUCTIVE_STATUS_DRV_STATE                  (uint64_t)(0xFF << 0x18) /* Mask */
#define kBcm5935xStateUnknown                       (uint64_t)(0x00 << 0x18)
#define kBcm5935xStateDBB                           (uint64_t)(0x01 << 0x18)
#define kBcm5935xStateFWDL                          (uint64_t)(0x02 << 0x18)
#define kBcm5935xStateMain                          (uint64_t)(0x03 << 0x18)
#define kBcm5935xStateMainLPM                       (uint64_t)(0x04 << 0x18)
#define kBcm5935xStateNoBackpower                   (uint64_t)(0x05 << 0x18)
#define kBcm5935xStateCloaked                       (uint64_t)(0x06 << 0x18)
#define kBcm5935xStateException                     (uint64_t)(0x10 << 0x18)

/* INDUCTIVE_STATUS_SS_VRECT (u16) */
#define INDUCTIVE_STATUS_SS_VRECT                   (uint64_t)(0xFF << 0x20) /* Mask */

/* INDUCTIVE_STATUS_2PP_STATE (u8) */
#define INDUCTIVE_STATUS_2PP_STATE                  (uint64_t)(0x0F << 0x28) /* Mask */

/* INDUCTIVE_STATUS_ILOAD_MOD (u8) */
#define INDUCTIVE_STATUS_ILOAD_MOD                  (uint64_t)(0x0F << 0x2C) /* Mask */

/* INDUCTIVE_STATUS_NOT_CLOAKING_REASON (u16) */
#define INDUCTIVE_STATUS_NOT_CLOAKING_REASON        (uint64_t)(0xFF << 0x30) /* Mask */
#define INDUCTIVE_NOT_CLOAKING_REASON_NOT_ALLOWED   (uint64_t)(0x01 << 0x30)
#define INDUCTIVE_NOT_CLOAKING_REASON_ROLE_SWAP     (uint64_t)(0x02 << 0x30)
#define INDUCTIVE_NOT_CLOAKING_REASON_NOT_READY     (uint64_t)(0x04 << 0x30)
#define INDUCTIVE_NOT_CLOAKING_REASON_AUTH          (uint64_t)(0x08 << 0x30)
#define INDUCTIVE_NOT_CLOAKING_REASON_QUIESCE       (uint64_t)(0x10 << 0x30)
#define INDUCTIVE_NOT_CLOAKING_REASON_POWER_PAUSE   (uint64_t)(0x20 << 0x30)
#define INDUCTIVE_NOT_CLOAKING_REASON_DEMO          (uint64_t)(0x40 << 0x30)
#define INDUCTIVE_NOT_CLOAKING_REASON_AP_WAKE       (uint64_t)(0x80 << 0x30)

#define INDUCTIVE_STATUS_SYS_TRANS                  (uint64_t)(0x01 << 0x38)
#endif /* inductive_status_h */
