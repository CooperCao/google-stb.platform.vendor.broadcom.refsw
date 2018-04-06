/***************************************************************************
 * Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 * ***************************************************************************/
/*
    Title: ViCE Firmware API definitions (bvce_fw_api_v3.h)

    This file defines the interface between the Host and the ViCE firmware.
    It contains definitions for the commands formatting.
*/

#ifndef BVCE_FW_API_V3_H__
#define BVCE_FW_API_V3_H__

/* ==========================  INCLUDES =========================== */

/* NOTE: the definitions of uint8_t, uint16_t, ...  are coming from "defs.h"
 *       when the firmware includes this file and from "bstd.h" when the
 *       PI is using this file. */

/* NOTE2: This file can be compiled by our customer on C90 compilers
 *        where C++-style comments are not recognized, Please use only C-style
 *        comments in this file. */

/* ==========================  DEFINITIONS ======================== */

#ifdef __cplusplus
extern "C"
{
#endif


/*
 * API Version  MM.mm.pp.bb
 */

#define VICE_API_VERSION                  0x09000000

/*
 * Size of the command buffer between host (PI) and FW in bytes
 */
#define HOST_CMD_BUFFER_SIZE                      136

/*
 * ViCE to Host interrupts
 */
#define HOST_CMD_MAILBOX_INTERRUPT_LEVEL           28
#define HOST_EVENTS_INTERRUPT_LEVEL                27
#define HOST_FW_WDOG_PIC_INTERRUPT_LEVEL           0
#define HOST_FW_WDOG_MB_INTERRUPT_LEVEL            1

/*
 * Offset of the ViCE Mailboxes from the start of DCCM
 */
#define HOST2VICE_MBOX_OFFSET                       0
#define VICE2HOST_MBOX_OFFSET                       4
#define BVN2VICE_MBOX_OFFSET                        8
#define SCRATCH_MBOX_OFFSET                        12
#define BVN2VICE_MBOX_PAYLOAD_OFFSET               16


/*
 * Minimum size of the CDB and ITB buffers which is required by the FW.
 * The host may allocate larger buffers than that.
 */
#define MIN_CDB_SIZE_IN_BYTES                               0x00600000    /*  06MB */
#define MIN_ITB_SIZE_IN_BYTES                               0x00030200    /* 192.5kB */


/*
 * Minimum size per channel of the secure and command buffers that is required
 * for correct operation of the encoder. The host may allocate larger
 * buffers. V3 does not use the CMD buffer.
 */
#define MIN_SECURE_BUFFER_SIZE_IN_BYTES_FOR_VICE2_V1_AND_V2_0   0x00400000    /*   4MB - Bin buffer */
#define MIN_SECURE_BUFFER_SIZE_IN_BYTES_FOR_VICE2_V2_1          0x00580000    /* 5.5MB  - Bin buffers */
#define MIN_SECURE_BUFFER_SIZE_IN_BYTES_FOR_VICE3               0x000024E0    /* 5*1888 Bytes - CABAC probabilites buffers */
#define MIN_CMD_BUFFER_SIZE_IN_BYTES                            0x00004000    /*  16kB */

/*
 * Maximum FW binary size is used by VCE PI to allocate its own buffers.
 */
#define MAX_PICARC_FW_SIZE_IN_BYTES                             0x40000       /* 256 kB */
#define MAX_MBARC_FW_SIZE_IN_BYTES                              0xC000        /* 48 kB */


/* =====================================================*/
/* -------------- Host command opcode ------------------*/

/* NOTE: Enumerations cannot be used here because their size
         is not guaranteed to be the same on different compilers
*/

/*
 * VICE Boot Status
 * It is returned in the HOST2VICE mail box register
 */
#define VICE_BOOT_STATUS_INIT                           (0xFFFFFFFF)
#define VICE_BOOT_STATUS_INIT_PICARC_UART                      (0x1)
#define VICE_BOOT_STATUS_INIT_PICARC_INTERRUPTS                (0x2)
#define VICE_BOOT_STATUS_INIT_ICACHE                           (0x3)
#define VICE_BOOT_STATUS_INIT_HOST_IF                          (0x4)
#define VICE_BOOT_STATUS_INIT_BVN_IF                           (0x5)
#define VICE_BOOT_STATUS_BOOT_MBARC                            (0x6)
#define VICE_BOOT_STATUS_INIT_DINO                             (0x7)
#define VICE_BOOT_STATUS_INIT_CMD_BUFFER                       (0x8)
#define VICE_BOOT_STATUS_CHECK_VERSION                         (0x9)
#define VICE_BOOT_STATUS_INIT_SCHED                            (0xA)
#define VICE_BOOT_STATUS_INIT_HG                               (0xB)
#define VICE_BOOT_STATUS_COMPLETE                              (0x0)


/*
 * Error return codes for ViCE Command
 */
#define VICE_CMD_RETURN_OK                                          (0)    /* command was processed by FW successfully  */
#define VICE_CMD_RETURN_ERR_UNKNOWN_COMMAND_ID                      (1)    /* the command opcode could not be recognised by the FW */
#define VICE_CMD_RETURN_ERR_NO_RESOURCES                            (2)    /* resources required to run the command are not available */
#define VICE_CMD_RETURN_ERR_INVALID_CHANNEL_ID                      (3)    /* the channel is invalid */
#define VICE_CMD_RETURN_ERR_NULL_ADDRESS                            (4)    /* non secure buffer has NULL address */
#define VICE_CMD_RETURN_ERR_SECURE_BUFFER_SIZE_IS_TOO_SMALL         (5)    /* Secure buffer size is too small for FW needs */
#define VICE_CMD_RETURN_ERR_IN_CDB_OR_ITB_BUFFER                    (6)    /* CDB/ITB buffer is too small or not aligned properly */
#define VICE_CMD_RETURN_ERR_ENCODER_BUFFER_SIZE_IS_TOO_SMALL        (7)    /* Encoder dram buffer size is too small for the EPM needs */
#define VICE_CMD_RETURN_ERR_CHANNEL_IS_ALREADY_INITIALIZED          (8)    /* Trying to initialize a channel which was already initialized */
#define VICE_CMD_RETURN_ERR_CHANNEL_IS_ALREADY_OPENED               (9)    /* Trying to open a channel which was already opened */
#define VICE_CMD_RETURN_ERR_CHANNEL_WAS_NOT_INITIALIZED            (10)    /* Trying to open a channel which was not initialized */
#define VICE_CMD_RETURN_ERR_CHANNEL_WAS_NOT_OPENED                 (11)    /* Trying to configure/start/close a channel which was not opened */
#define VICE_CMD_RETURN_ERR_VICE_IS_IN_FLUSHING_MODE               (12)    /* ViCE can not configure/start/close a channel while it is flushing data */
#define VICE_CMD_RETURN_ERR_CHANNEL_IS_ALREADY_STARTED             (13)    /* Trying to start a channel which was already started */
#define VICE_CMD_RETURN_ERR_CHANNEL_WAS_NOT_CONFIGURED             (14)    /* Trying to start a channel which was not configured */
#define VICE_CMD_RETURN_ERR_CHANNEL_WAS_NOT_STARTED                (15)    /* Trying to stop a channel which was not started */
#define VICE_CMD_RETURN_ERR_CHANNEL_WAS_NOT_STOPPED                (16)    /* Trying to close a channel which was not stopped */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_FRAME_RATE                 (17)    /* Configure an unsupported frame rate */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_DEE                        (18)    /* DDE value given by the host is invalid */
#define VICE_CMD_RETURN_ERR_FRAMERATE_CHANGE_INVALID_IN_INTERLACE  (19)    /* ViCE does not support frame rate changes in interlaced mode */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_INTERLACED_TYPE_LOW_DELAY  (20)    /* Low delay mode is supported only for progressive input */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_B_PICTURES_IN_LOW_DELAY    (21)    /* Low delay mode can not support B picture ( reordering ) */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_B_PICTURES_IN_STD          (22)    /* VICE does not support B picture for MPEG4 or VP8 standards */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_INTERLACED_IN_STD          (23)    /* VICE does not support interlaced type for MPEG4 or VP8 standards */
#define VICE_CMD_RETURN_ERR_INVALID_DEBUG_COMMAND                  (24)    /* Command String passed to the Debug Command is not valid */
#define VICE_CMD_RETURN_ERR_SG_CMD_BUFFER_SIZE_IS_TOO_SMALL        (25)    /* SG-CABAC command buffer size is too small (< MIN_CMD_BUFFER_SIZE_IN_BYTES) */
#define VICE_CMD_RETURN_ERR_MAX_RESOLUTION_ERROR                   (26)    /* MaxPictureHeight/MaxPictureWidth are larger than the largest supported res.*/
#define VICE_CMD_RETURN_ERR_MAX_FRAMERATE_LIMIT_TOO_LARGE          (27)    /* MaxFrameRateLimit is larger than the largest supported frame rate */
#define VICE_CMD_RETURN_ERR_MIN_FRAMERATE_LIMIT_TOO_SMALL          (28)    /* MinFrameRateLimit is smaller than the smallest supported frame rate */
#define VICE_CMD_RETURN_ERR_MIN_BVN_FRAMERATE_LIMIT_TOO_SMALL      (29)    /* BvnFrameRate is smaller than the smallest supported frame rate */
#define VICE_CMD_RETURN_ERR_FRAMERATE_LESS_THAN_LIMIT              (30)    /* Given frame rate is smaller than the limit (MinFrameRateLimit) */
#define VICE_CMD_RETURN_ERR_FRAMERATE_MORE_THAN_LIMIT              (31)    /* Given frame rate is larger than the limit (MaxFrameRateLimit) */
#define VICE_CMD_RETURN_ERR_WRONG_GOP_LENGTH                       (32)    /* Gop length is wrong ( should be 1 for I_ONLY and 1+3N for IBBP */
#define VICE_CMD_RETURN_ERR_DEBUG_COMMAND_USE_IN_INVALID_STATE     (33)    /* Cmd String passed to the Debug Command used in state in which it is invalid*/
#define VICE_CMD_RETURN_ERR_MULTIPLE_SLICES_NOT_SUPPORTED          (34)    /* Multiple slice not supported for VICE V1 */
#define VICE_CMD_RETURN_ERR_INVALID_FORCE_INTRA_CONFIGURATION      (35)    /* Invlaid Forced Intra configuration setting */
#define VICE_CMD_RETURN_ERR_LOW_DELAY_SUPPORTED_ONLY_CHANNEL_0     (36)    /* Low delay is supported only for single channel and the channel must be 0 */
#define VICE_CMD_RETURN_ERR_LOW_DELAY_UNSUPPORTED_ON_TARGET        (37)    /* Low delay mode is not supported for this core version */
#define VICE_CMD_RETURN_ERR_RESTART_GOP_SCENE_INVALID_GOP          (38)    /* Restarting GOP is only valid w/ I or IP GOP or IBBP open GOP structures */
#define VICE_CMD_RETURN_ERR_ITFP_INVALID_IN_LOW_DELAY              (39)    /* ITFP command is not valid in Low-Delay mode */
#define VICE_CMD_RETURN_ERR_TARGET_BITRATE_LARGER_MAX              (40)    /* Target bitrate is larger than the maximum bitrate */
#define VICE_CMD_RETURN_ERR_WRONG_NUM_PARALLEL_ENC                 (41)    /* Too many parallel FNRT encode specified for the given version */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_FINITE_GOP_IN_LOW_DELAY    (42)    /* Low delay mode does not support finite GOP */
#define VICE_CMD_RETURN_ERR_MAX_NUM_CH_NOT_SUPPORTED               (43)    /* The max number of channels given in the open cmd is too high on this target */
#define VICE_CMD_RETURN_ERR_MAX_NUM_CH_INCONSISTENT                (44)    /* The max number of channels should be the same on all channels */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_ENCODING_STD               (45)    /* The encoding standard in not supported for this core */
#define VICE_CMD_RETURN_ERR_INVALID_OPEN_GOP_IN_GOP_STRUCTURE      (46)    /* Open GOP can be set only for a GOP structure which includes B pictures */
#define VICE_CMD_RETURN_ERR_DCXV_NOT_SUPPORTED                     (47)    /* DCXV is supported only for core 2.1.0.3 or later */
#define VICE_CMD_RETURN_ERR_NON_SECURE_NOT_ALIGNED                 (48)    /* non secure buffer address is not 10bit aligned */
#define VICE_CMD_RETURN_ERR_SECURE_NOT_ALIGNED                     (49)    /* secure buffer address is not J-word aligned */
#define VICE_CMD_RETURN_ERR_CMD_BUFFER_NOT_ALIGNED                 (50)    /* command buffer address is not J-word aligned */
#define VICE_CMD_RETURN_ERR_VICE_IS_IN_CHANNEL_FLUSHING_MODE       (51)    /* ViCE can not configure/start/close a channel while it is flushing data */
#define VICE_CMD_RETURN_ERR_VICE_IS_IN_CABAC_FLUSHING_MODE         (52)    /* ViCE can not configure/start/close a channel while it is flushing data */
#define VICE_CMD_RETURN_ERR_VARIABLE_BITRATE_UNSUPPORTED           (53)    /* VBR unsupported in this mode */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_INTERLACED_TYPE_VP9        (54)    /* Interlaced mode is not supported for VP9 encoding standard */
#define VICE_CMD_RETURN_ERR_WRONG_MEMORY_SETTINGS                  (55)    /* Memory settings are wrong / not supported */
#define VICE_CMD_RETURN_ERR_UNSUPPORTED_B_REF_IN_STANDARD          (56)    /* Using B pictures as references is not supported for this standard */
#define VICE_CMD_RETURN_ERR_GOP_STRUCTURE_GREATER_THAN_MAX         (57)    /* The requested GOP structure is greater than the maximal gop structure */
#define VICE_CMD_RETURN_ERR_GOP_STR_NOT_SUPPORTED_FOR_INTERLACE    (58)    /* The requested GOP structure is not supported for interlace input */
#define VICE_CMD_RETURN_ERR_MAXGOP_STR_NOT_SUPPORTED_FOR_INTERLACE (59)    /* The requested MAX GOP structure is not supported for interlace input */

/*
 * VICE Commands
 */
#define VICE_COMMAND_SIGNATURE                           (0x42434D00)
#define VICE_CMD_INITIALIZE                              (0x42434D01)
#define VICE_CMD_OPEN_CHANNEL                            (0x42434D02)
#define VICE_CMD_START_CHANNEL                           (0x42434D03)
#define VICE_CMD_STOP_CHANNEL                            (0x42434D04)


#define VICE_CMD_CLOSE_CHANNEL                           (0x42434D07)
#define VICE_CMD_CONFIG_CHANNEL                          (0x42434D08)
#define VICE_CMD_DEBUG_CHANNEL                           (0x42434D09)
#define VICE_CMD_GET_CHANNEL_STATUS                      (0x42434D0A)
#define VICE_CMD_GET_DEVICE_STATUS                       (0x42434D0B)


/*
 * Channel-specific Event codes returned by the ViCE through the Status and Event interface
 */
/* Errors (LSBs) */
#define VICE_ERROR_INVALID_INPUT_DIMENSION_BIT                     (0)
#define VICE_ERROR_USER_DATA_LATE_BIT                              (1)
#define VICE_ERROR_USER_DATA_DUPLICATE_BIT                         (2)
#define VICE_ERROR_FW_ADJUSTS_WRONG_FRAME_RATE                     (3)
#define VICE_ERROR_UNSUPPORTED_BVN_FRAME_RATE                      (4)
#define VICE_ERROR_UNSUPPORTED_RESOLUTION                          (5)
#define VICE_ERROR_BVN_FRAMERATE_IS_SMALLER_THAN_THE_MINIMUM_ALLOWED        (6)
#define VICE_ERROR_MISMATCH_BVN_PIC_RESOLUTION                     (7)
#define VICE_ERROR_FW_INCREASED_BITRATE_ABOVE_MAX                  (8)
#define VICE_ERROR_BIN_BUFFER_IS_FULL                              (9)
#define VICE_ERROR_CDB_IS_FULL                                     (10)
#define VICE_ERROR_PICARC_TO_CABAC_DINO_BUFFER_IS_FULL             (11)
#define VICE_ERROR_EBM_IS_FULL                                     (12)
#define VICE_ERROR_NUM_SLICES_ADJUSTED_TO_MAX_ALLOWED              (13)
#define VICE_ERROR_NUM_ENTRIES_INTRACODED_ADJUSTED_TO_MAX_ALLOWED  (14)
#define VICE_ERROR_IBBP_NOT_SUPPORTED_FOR_THIS_RESOLUTION          (15)
#define VICE_ERROR_MBARC_BOOT_FAILURE                              (16)
#define VICE_ERROR_MEASURED_ENCODER_DELAY_LONGER_THAN_ESTIMATED    (17)
#define VICE_ERROR_CRITICAL                                        (18)
#define VICE_ERROR_RES_AND_FRAMERATE_NOT_SUPPORTED_IN_3_CH_MODE    (19)
#define VICE_ERROR_RES_AND_FRAMERATE_NOT_SUPPORTED_IN_2_CH_MODE    (20)
#define VICE_ERROR_RESOLUTION_IS_TOO_HIGH_FOR_THIS_LEVEL           (21)
#define VICE_ERROR_FW_INCREASED_BITRATE_TO_MINIMUM_SUPPORTED       (22)
#define VICE_ERROR_UNSUPPORTED_FRAME_RATE_FOR_THIS_RESOLUTION_AND_GOP_STRUCTURE       (23)
/* Events (MSBs) */
#define VICE_EVENT_EOS_SENT_BIT                                    (30)
#define VICE_EVENT_BVN_METADATA_CHANGE_BIT                         (31)


/*
 * Channel-independent Error codes located in a word which location is returned from Initialize cmd
 */
#define VICE_ERROR_STACK_OVERFLOW_BIT                      0x00


/*
 * Watchdog error codes
 *
 * Note that there is a convention that the "AFTER" codes are always the previous Code + 1
 */
typedef enum
{
    VICE_WDOG_TRACE_ISR_WAIT_VIP_BUSY                           = 0x010,
    VICE_WDOG_TRACE_ISR_WAIT_VIP_BUSY_AFTER                     = 0x011,
    VICE_WDOG_TRACE_ISR_WAIT_VIP_SHADOW                         = 0x020,
    VICE_WDOG_TRACE_ISR_WAIT_VIP_SHADOW_AFTER                   = 0x021,
    VICE_WDOG_TRACE_ISR_ITFP_AND_IPB                            = 0x030,
    VICE_WDOG_TRACE_ISR_ITFP_AND_IPB_AFTER                      = 0x031,
    VICE_WDOG_TRACE_SCHED_WAIT_START_PIC_TO_MBARC               = 0x040,
    VICE_WDOG_TRACE_SCHED_WAIT_START_PIC_TO_MBARC_AFTER         = 0x041,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_ARC_TO_MC_EMPTY        = 0x050,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_ARC_TO_MC_EMPTY_AFTER  = 0x051,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_DMA_IDLE               = 0x060,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_DMA_IDLE_AFTER         = 0x061,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_SEND_MC_DINO           = 0x070,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_SEND_MC_DINO_AFTER     = 0x071,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_DMA1_IDLE              = 0x080,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_DMA1_IDLE_AFTER        = 0x081,
    VICE_WDOG_TRACE_ENCODE_WAIT_FME_SHADOW                      = 0x090,
    VICE_WDOG_TRACE_ENCODE_WAIT_FME_SHADOW_AFTER                = 0x091,
    VICE_WDOG_TRACE_ENCODE_WAIT_IMD_SHADOW                      = 0x0A0,
    VICE_WDOG_TRACE_ENCODE_WAIT_IMD_SHADOW_AFTER                = 0x0A1,
    VICE_WDOG_TRACE_ENCODE_WAIT_MC_SHADOW                       = 0x0B0,
    VICE_WDOG_TRACE_ENCODE_WAIT_MC_SHADOW_AFTER                 = 0x0B1,
    VICE_WDOG_TRACE_ENCODE_WAIT_HA_SHADOW                       = 0x0C0,
    VICE_WDOG_TRACE_ENCODE_WAIT_HA_SHADOW_AFTER                 = 0x0C1,
    VICE_WDOG_TRACE_ENCODE_WAIT_XQ_SHADOW                       = 0x0D0,
    VICE_WDOG_TRACE_ENCODE_WAIT_XQ_SHADOW_AFTER                 = 0x0D1,
    VICE_WDOG_TRACE_ENCODE_WAIT_SG_SHADOW                       = 0x0E0,
    VICE_WDOG_TRACE_ENCODE_WAIT_SG_SHADOW_AFTER                 = 0x0E1,
    VICE_WDOG_TRACE_ENCODE_WAIT_ILF_SHADOW                      = 0x0F0,
    VICE_WDOG_TRACE_ENCODE_WAIT_ILF_SHADOW_AFTER                = 0x0F1,
    VICE_WDOG_TRACE_ENCODE_WAIT_XQ_DMA                          = 0x100,
    VICE_WDOG_TRACE_ENCODE_WAIT_XQ_DMA_AFTER                    = 0x101,
    VICE_WDOG_TRACE_ENCODE_WAIT_XQ_DMA1                         = 0x110,
    VICE_WDOG_TRACE_ENCODE_WAIT_XQ_DMA1_AFTER                   = 0x111,
    VICE_WDOG_TRACE_ENCODE_WAIT_CME_SHADOW                      = 0x120,
    VICE_WDOG_TRACE_ENCODE_WAIT_CME_SHADOW_AFTER                = 0x121,
    VICE_WDOG_TRACE_ENCODE_WAIT_MAU_SHADOW                      = 0x130,
    VICE_WDOG_TRACE_ENCODE_WAIT_MAU_SHADOW_AFTER                = 0x131,
    VICE_WDOG_TRACE_ENCODE_WAIT_PAINT_DMA                       = 0x140,
    VICE_WDOG_TRACE_ENCODE_WAIT_PAINT_DMA_AFTER                 = 0x141,
    VICE_WDOG_TRACE_HG_WAIT_USERDATA_DMA1                       = 0x150, /* 0x152 , 0x154, 0x156, 0x158 */
    VICE_WDOG_TRACE_HG_WAIT_USERDATA_DMA1_AFTER                 = 0x151, /* 0x153 , 0x155, 0x157, 0x159 */
    VICE_WDOG_TRACE_HG_WAIT_USERDATA_DMA2                       = 0x160,
    VICE_WDOG_TRACE_HG_WAIT_USERDATA_DMA2_AFTER                 = 0x161,
    VICE_WDOG_TRACE_RC_WAIT_SEND_DINO                           = 0x190,
    VICE_WDOG_TRACE_RC_WAIT_SEND_DINO_AFTER                     = 0x191,
    VICE_WDOG_TRACE_RC_WAIT_GET_DINO                            = 0x1A0,
    VICE_WDOG_TRACE_RC_WAIT_GET_DINO_AFTER                      = 0x1A1,
    VICE_WDOG_TRACE_RC_WAIT_SEND_DINO_UF                        = 0x1B0,
    VICE_WDOG_TRACE_RC_WAIT_SEND_DINO_UF_AFTER                  = 0x1B1,
    VICE_WDOG_TRACE_RC_WAIT_SEND_DINO_UF2                       = 0x1C0,
    VICE_WDOG_TRACE_RC_WAIT_SEND_DINO_UF2_AFTER                 = 0x1C1,
    VICE_WDOG_TRACE_BOOT_WAIT_MBARC                             = 0x1D0,
    VICE_WDOG_TRACE_BOOT_WAIT_MBARC_AFTER                       = 0x1D1,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA1                             = 0x1E0,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA1_AFTER                       = 0x1E1,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA2                             = 0x1F0,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA2_AFTER                       = 0x1F1,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA3                             = 0x200,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA3_AFTER                       = 0x201,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA4                             = 0x210,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA4_AFTER                       = 0x211,
    VICE_WDOG_TRACE_UART_WAIT_TX                                = 0x220,
    VICE_WDOG_TRACE_UART_WAIT_TX_AFTER                          = 0x221,
    VICE_WDOG_TRACE_SCHED_FORCE_INTRA_WAIT_DINO_EMPTY           = 0x230,
    VICE_WDOG_TRACE_SCHED_FORCE_INTRA_WAIT_DINO_EMPTY_AFTER     = 0x231,
    VICE_WDOG_TRACE_SCHED_FORCE_INTRA_WAIT_SEND_DINO            = 0x240,
    VICE_WDOG_TRACE_SCHED_FORCE_INTRA_WAIT_SEND_DINO_AFTER      = 0x241,
    VICE_WDOG_TRACE_HOSTCMD_WAIT_DMA1                           = 0x250,
    VICE_WDOG_TRACE_HOSTCMD_WAIT_DMA1_AFTER                     = 0x251,
    VICE_WDOG_TRACE_ENCODE_WAIT_ILF_IDLE                        = 0x260,
    VICE_WDOG_TRACE_ENCODE_WAIT_ILF_IDLE_AFTER                  = 0x261,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA5                             = 0x270,
    VICE_WDOG_TRACE_TELEM_WAIT_DMA5_AFTER                       = 0x271,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_FOR_DATA_FROM_MC       = 0x280,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_FOR_DATA_FROM_MC_AFTER = 0x281,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_DMA2_IDLE              = 0x290,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_DMA2_IDLE_AFTER        = 0x291,
    VICE_WDOG_TRACE_SCHED_WAIT_CME_SLEEP                        = 0x2A0,
    VICE_WDOG_TRACE_SCHED_WAIT_CME_SLEEP_AFTER                  = 0x2A1,
    VICE_WDOG_TRACE_SCHED_WAIT_MBP_SLEEP                        = 0x2B0,
    VICE_WDOG_TRACE_SCHED_WAIT_MBP_SLEEP_AFTER                  = 0x2B1,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_DMA3_IDLE              = 0x2C0,
    VICE_WDOG_TRACE_SCHED_COLOCATED_WAIT_DMA3_IDLE_AFTER        = 0x2C1,
    VICE_WDOG_TRACE_MB_WAIT_DBLK_STATUS_BUSY                    = 0x2F0,
    VICE_WDOG_TRACE_MB_WAIT_DBLK_STATUS_BUSY_AFTER              = 0x2F1,
    VICE_WDOG_TRACE_MB_WAIT_MAU_STATUS_BUSY                     = 0x300,
    VICE_WDOG_TRACE_MB_WAIT_MAU_STATUS_BUSY_AFTER               = 0x301,
    VICE_WDOG_TRACE_MB_WAIT_FME_STATUS_BUSY                     = 0x310,
    VICE_WDOG_TRACE_MB_WAIT_FME_STATUS_BUSY_AFTER               = 0x311,
    VICE_WDOG_TRACE_MB_WAIT_MC_STATUS_BUSY                      = 0x320,
    VICE_WDOG_TRACE_MB_WAIT_MC_STATUS_BUSY_AFTER                = 0x321,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_XQ_PICSTART                   = 0x330,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_XQ_PICSTART_AFTER             = 0x331,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_MC_PICSTART                   = 0x340,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_MC_PICSTART_AFTER             = 0x341,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_TO_MC_DINO                    = 0x350,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_TO_MC_DINO_AFTER              = 0x351,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_FROM_MC_DINO                  = 0x360,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_FROM_MC_DINO_AFTER            = 0x361,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_TO_XQ_DINO                    = 0x370,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_TO_XQ_DINO_AFTER              = 0x371,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_FROM_SG_DINO                  = 0x380,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_FROM_SG_DINO_AFTER            = 0x381,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_TO_PICARC_DINO                = 0x390,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_TO_PICARC_DINO_AFTER          = 0x391,
    VICE_WDOG_TRACE_MB_WAIT_IMD_STATUS_BUSY                     = 0x3A0,
    VICE_WDOG_TRACE_MB_WAIT_IMD_STATUS_BUSY_AFTER               = 0x3A1,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_IMD_PICSTART                  = 0x3B0,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_IMD_PICSTART_AFTER            = 0x3B1,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_TO_IMD_DINO                   = 0x3C0,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_TO_IMD_DINO_AFTER             = 0x3C1,
    VICE_WDOG_TRACE_MB_WAIT_REWIND_SG_DINO                      = 0x3D0,
    VICE_WDOG_TRACE_MB_WAIT_REWIND_SG_DINO_AFTER                = 0x3D1,
    VICE_WDOG_TRACE_MB_CHECK_FOR_CABAC_DATA                     = 0x3E0,
    VICE_WDOG_TRACE_MB_CHECK_FOR_CABAC_DATA_AFTER               = 0x3E1,
    VICE_WDOG_TRACE_ENCODE_WAIT_DBLK_BUSY                       = 0x3F0,
    VICE_WDOG_TRACE_ENCODE_WAIT_DBLK_BUSY_AFTER                 = 0x3F1,
    VICE_WDOG_TRACE_ENCODE_WAIT_SG_BUSY                         = 0x400,
    VICE_WDOG_TRACE_ENCODE_WAIT_SG_BUSY_AFTER                   = 0x401,
    VICE_WDOG_TRACE_ENCODE_WAIT_XQ_BUSY                         = 0x410,
    VICE_WDOG_TRACE_ENCODE_WAIT_XQ_BUSY_AFTER                   = 0x411,
    VICE_WDOG_TRACE_ENCODE_WAIT_SG_DONE                         = 0x420,
    VICE_WDOG_TRACE_ENCODE_WAIT_SG_DONE_AFTER                   = 0x421,
    VICE_WDOG_TRACE_INITHOSTCMD_WAIT_DMA1                       = 0x430,
    VICE_WDOG_TRACE_INITHOSTCMD_WAIT_DMA1_AFTER                 = 0x431,
    VICE_WDOG_MBARC_WAIT_4_PICARC_DINO                          = 0x440,
    VICE_WDOG_MBARC_WAIT_4_PICARC_DINO_AFTER                    = 0x441,
    VICE_WDOG_WAIT_PICARC_TO_MBARC_ADI_RECEIVED                 = 0x450,
    VICE_WDOG_WAIT_PICARC_TO_MBARC_ADI_RECEIVED_AFTER           = 0x451,
    VICE_WDOG_TRACE_WAIT_PICARC_FROM_MBARC_DINO                 = 0x460,
    VICE_WDOG_TRACE_WAIT_PICARC_FROM_MBARC_DINO_AFTER           = 0x461,
    VICE_WDOG_MBARC_WAIT_MBARC_TO_PICARC_ADI_RECEIVED           = 0x470,
    VICE_WDOG_MBARC_WAIT_MBARC_TO_PICARC_ADI_RECEIVED_AFTER     = 0x471,
    VICE_WDOG_MBARC_WAIT_PICARC_TO_MBARC_ADI_SENT               = 0x480,
    VICE_WDOG_MBARC_WAIT_PICARC_TO_MBARC_ADI_SENT_AFTER         = 0x481,
    VICE_WDOG_MBARC_WAIT_MBARC_TO_PICARC_ADI_SENT               = 0x490,
    VICE_WDOG_MBARC_WAIT_MBARC_TO_PICARC_ADI_SENT_AFTER         = 0x491,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_FROM_PICARC_DINO              = 0x4A0,
    VICE_WDOG_TRACE_MB_WAIT_MBARC_FROM_PICARC_DINO_AFTER        = 0x4A1,
    VICE_WDOG_TRACE_MB_WAIT_CABAC_STAT_TO_MBARC_DINO            = 0x4B0,
    VICE_WDOG_TRACE_MB_WAIT_CABAC_STAT_TO_MBARC_DINO_AFTER      = 0x4B1,
    VICE_WDOG_TRACE_MB_WAIT_CABAC_READ_PROB_TO_BE_DONE          = 0x4C0,
    VICE_WDOG_TRACE_MB_WAIT_CABAC_READ_PROB_TO_BE_DONE_AFTER    = 0x4C1

} WdogTraceCode_e;

/* Add more here, note: codes are bit-mapped (max: 32) */


/*
 * MACRO: VCE_PTR(data_type)
 *  This macro does not use the argument (data_type) and redefines it as a uint32_t
 *  it is used to indicate what data type the pointer is intended to point to.
 */
#define VCE_PTR(data_type)    uint32_t




/* ---- ERROR ---- */

typedef struct ViceGenericCmd_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_*  */
} ViceGenericCmd_t;

typedef struct ViceGenericCmdResponse_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */
    uint32_t    Status;                                     /* Status: VICE_CMD_ERR_UNKNOWN_COMMAND_ID */
} ViceGenericCmdResponse_t;



/* ---- INIT ---- */
#define WORD_SIZE_GWORD        0
#define WORD_SIZE_JWORD        1
#define WORD_SIZE_MWORD        2

#define BANK_TYPE_4_BANKS      0
#define BANK_TYPE_8_BANKS      1
#define BANK_TYPE_16_BANKS     2

#define PAGE_SIZE_1_KBYTES     0
#define PAGE_SIZE_2_KBYTES     1
#define PAGE_SIZE_4_KBYTES     2
#define PAGE_SIZE_8_KBYTES     3
#define PAGE_SIZE_16_KBYTES    4

/* VerificationModeFlags Enumerators */
#define INIT_CMD_A2N_MASK                                   0x00000002
#define INIT_CMD_A2N_SHIFT                                  0x00000001
#define INIT_CMD_VERIFICATION_MODE_MASK                     0x00000001
#define INIT_CMD_VERIFICATION_MODE_SHIFT                    0x00000000


typedef struct ViceCmdInit_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_*                                 */
    uint32_t    API_Version;                                /* version of the API, this can remain a 32-bit number               */
    uint32_t    DeviceEndianess;                            /* CABAC Endianness flag 0:Big-endian (HW default), 1:Little-endian (VICE v1/v2 only)  */
    uint32_t    DeviceSG_CABACCmdBuffPtr;                   /* SG-CABAC Command Buffer Address in DRAM (VICE v1/v2 only)         */
    uint32_t    DeviceSG_CABACCmdBuffSize;                  /* SG-CABAC Command Buffer size in bytes   (VICE v1/v2 only)         */
    uint32_t    VerificationModeFlags;                      /* Bit 0 - Verification mode Off/On. Bit 1 - A2N drop On/Off         */
    uint32_t    StripeWidth;                                /* DRAM Stripe width for the given platform                          */
    uint32_t    X;                                          /* X in Xn+Y formula which is used for NBMY calculation              */
    uint32_t    Y;                                          /* Y in Xn+Y formula which is used for NBMY calculation              */
    uint32_t    WordSize;                                   /* 0: G-word (128 bits). 1: J-word (256 bits). 2: M-word (512 bits). */
    uint32_t    BankType;                                   /* 0: 4 Banks.  1: 8 Banks.  2: 16 Banks.                            */
    uint32_t    PageSize;                                   /* 0: 1 Kbytes. 1: 2 Kbytes. 2: 4 Kbytes. 3: 8 Kbytes. 4: 16 Kbytes. */
    uint32_t    Grouping;                                   /* 0: Disable.  1: Enable.                                           */
    uint32_t    NewBvnMailbox;                              /* 0: Disable.  1: Enable.                                           */
}ViceCmdInit_t;

typedef struct ViceDebugBufferInfo_t                       /* //NS : style check ignored: required for backwards compatibility   */
{
    uint32_t  uiReadOffset;                                 /* PI or FW will update this, depending on the mode (0-indexed relative to start of buffer) */
    uint32_t  uiWriteOffset;                                /* FW will update this (0-indexed relative to start of buffer) */
    uint32_t  uiMode;                                       /* VICE_DEBUG_BUFFER_MODE_STANDARD or VICE_DEBUG_BUFFER_MODE_OVERWRITE (see below) */
    uint32_t  uiSize;                                       /* size of debug buffer in bytes (DWORD-multiple) */
    uint32_t  uiPhysicalOffsetLo;                           /* physical offset in DRAM of debug log. (DWORD-aligned) */
    uint32_t  uiPhysicalOffsetHi;                           /* physical offset in DRAM of debug log. (DWORD-aligned) */
} ViceDebugBufferInfo_t;

/* Debug Buffer Mode Description */
#define VICE_DEBUG_BUFFER_MODE_STANDARD     (0)             /* Standard Circular FIFO  FW writes to the buffer as long as space is available. */
                                                            /* When the buffer is full, FW drops any data that would have been written to the buffer. */
                                                            /* The host would poll and read the debug log on regular time intervals. */
                                                            /* It is the host's responsibility to poll/read the buffer at a fast enough rate. */
                                                            /* This can be used with the debug FW command to implement a virtual UART interface. */
                                                            /* The FW updates the write offset and the host updates the read offset. */

#define VICE_DEBUG_BUFFER_MODE_OVERWRITE   (1)             /* Circular FIFO  FW continually writes to the buffer, overwriting the oldest data as needed. */
                                                            /* The latest data is always available in the buffer. The FW updates the write offset. */
                                                            /* The FW also updates the read offset (equal to (write_offset-1) ) when overwriting old data. */
                                                            /* This mode is typically used to obtain a UART log history to help debug errors such as watchdog */
                                                            /* reset or an error interrupt.                                                                 */
                                                            /* If the host wants to read the data, the host needs to: */
                                                            /* 1.    Set the physical offset to NULL */
                                                            /* 2.    Wait XX ms  */
                                                            /* 3.    Read the data  */
                                                            /* 4.    Set the physical offset back  */

/* Response to the VICE_CMD_INITIALIZE command (sent back to the Host) */
typedef struct ViceCmdInitResponse_t
{
    uint32_t                        Command;                /* OpCode of the command: VICE_CMD_* */
    uint32_t                        Status;                 /* Status: VICE_CMD_OK, or VICE_CMD_ERR_* */
    uint32_t                        Version;                /* Firmware version: format is 0xMMmmppbb (where: MM=major, mm=minor, pp=patch, bb=build) */
    VCE_PTR(ViceDebugBufferInfo_t) pPicArcDebugBuff;       /* Buffer used to send debug information back up to the host for offline processing (PicARC) */
    VCE_PTR(ViceDebugBufferInfo_t) pMbArcDebugBuff;        /* Buffer used to send debug information back up to the host for offline processing (MbARC) */
    VCE_PTR(uint32)                 pStatusBase;            /* Pointer to a location containing a bit-mapped u32 indicating which channel encounter an error */
    uint32_t                        FwApiVersion;           /* Version of the API that the Firmware has been built with */
    VCE_PTR(const char*)            pszVersionStr;          /* Pointer to a zero-terminated string containing information about the build */
    uint32_t                        BvnApiVersion;          /* Version of the BVN Mailbox API that the Firmware has been build with */
    VCE_PTR(uint32)                 pPicArcWdogErrCodeBase; /* Pointer to a 32-bit error code which identifies a loop where a watchdog error could happen */
    VCE_PTR(uint32)                 pMbArcWdogErrCodeBase;  /* Pointer to a 32-bit error code which identifies a loop where a watchdog error could happen */
} ViceCmdInitResponse_t;


#define DEVICE_ERROR_CHANNEL_ID     31                      /* channel to use to indicate a device error */


/* ---- OPEN ---- */

/* User Data */
#define BVCE_FW_P_UserData_Payload_CC_608Metadata_LineOffset_MASK       0xF8
#define BVCE_FW_P_UserData_Payload_CC_608Metadata_LineOffset_SHIFT      3

#define BVCE_FW_P_UserData_Payload_CC_608Metadata_Priority_MASK         0x06
#define BVCE_FW_P_UserData_Payload_CC_608Metadata_Priority_SHIFT        1

#define BVCE_FW_P_UserData_Payload_CC_608Metadata_Valid_MASK            0x01
#define BVCE_FW_P_UserData_Payload_CC_608Metadata_Valid_SHIFT           0

#define BVCE_FW_P_UserData_Payload_CC_608_BYTES_PER_LINE_MAX            2
#define BVCE_FW_P_UserData_Payload_CC_608_LINES_PER_FIELD_MAX           5
#define BVCE_FW_P_UserData_Payload_CC_608_LENGTH                        ( 1 + ( 1 + BVCE_FW_P_UserData_Payload_CC_608_BYTES_PER_LINE_MAX ) *    \
                                                                                BVCE_FW_P_UserData_Payload_CC_608_LINES_PER_FIELD_MAX )

#define BVCE_FW_P_UserData_Payload_CC_708Metadata_Reserved_MASK         0xFC
#define BVCE_FW_P_UserData_Payload_CC_708Metadata_Reserved_SHIFT        2

#define BVCE_FW_P_UserData_Payload_CC_708Metadata_PacketStart_MASK      0x02
#define BVCE_FW_P_UserData_Payload_CC_708Metadata_PacketStart_SHIFT     1

#define BVCE_FW_P_UserData_Payload_CC_708Metadata_Valid_MASK            0x01
#define BVCE_FW_P_UserData_Payload_CC_708Metadata_Valid_SHIFT           0

#define BVCE_FW_P_UserData_Payload_CC_708_BYTES_PER_LINE_MAX            2
#define BVCE_FW_P_UserData_Payload_CC_708_LINES_PER_FIELD_MAX           9
#define BVCE_FW_P_UserData_Payload_CC_708_LENGTH                        ( 1 + ( 1 + BVCE_FW_P_UserData_Payload_CC_708_BYTES_PER_LINE_MAX ) *     \
                                                                                BVCE_FW_P_UserData_Payload_CC_708_LINES_PER_FIELD_MAX )

#define BVCE_FW_P_UserData_Payload_CC_MAX_LENGTH                        ( BVCE_FW_P_UserData_Payload_CC_608_LENGTH + \
                                                                          BVCE_FW_P_UserData_Payload_CC_708_LENGTH )

typedef struct BVCE_FW_P_UserData_Payload_CC
{
        uint8_t uiNumValid608Lines; /* Should be 1 for A/53.  Can be more than 1 for SCTE20/21 */
        /* If multiple lines of CC data exist, they must be added in ascending line_offset */
        struct
        {
                uint8_t uiMetadata; /* Bits[7:3] = line_offset (see PacketType) (Not for A/53), Bits[2:1] - priority (Only for SCTE 20), Bit[0] = cc_valid */
                uint8_t uiData[BVCE_FW_P_UserData_Payload_CC_608_BYTES_PER_LINE_MAX]; /* 2 bytes of CC */
        } st608Line[BVCE_FW_P_UserData_Payload_CC_608_LINES_PER_FIELD_MAX];

        uint8_t uiNumValid708Lines; /* Should be 0 for SCTE20/21 */
        struct
        {
               uint8_t uiMetadata; /* Bits[7:2] = Reserved, Bit[1] - Packet Start, Bit[0] = cc_valid */
               uint8_t uiData[BVCE_FW_P_UserData_Payload_CC_708_BYTES_PER_LINE_MAX]; /* 2 bytes of CC */
        } st708Line[BVCE_FW_P_UserData_Payload_CC_708_LINES_PER_FIELD_MAX];
} BVCE_FW_P_UserData_Payload_CC;

typedef enum BVCE_FW_P_UserData_PacketType
{
        BVCE_FW_P_UserData_PacketType_eSCTE_20 = 0,
        BVCE_FW_P_UserData_PacketType_eSCTE_21,
        BVCE_FW_P_UserData_PacketType_eATSC_A53,

        /* Add new user data encapsulation type ABOVE this line */
        BVCE_FW_P_UserData_PacketType_eMax
} BVCE_FW_P_UserData_PacketType;

/* BVCE_FW_P_UserData_PacketDescriptor contains the information necessary to create a single user data packet of ePacketType */
#define BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH ( 4 + BVCE_FW_P_UserData_Payload_CC_MAX_LENGTH )

typedef struct BVCE_FW_P_UserData_PacketDescriptor
{
        uint16_t uiPacketLength; /* size of the entire packet including this field */
        uint16_t uiPacketType; /* See BVCE_FW_P_UserData_PacketType */
        BVCE_FW_P_UserData_Payload_CC stCC;
} BVCE_FW_P_UserData_PacketDescriptor;

#define BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_MASK    0xFF000000
#define BVCE_FW_P_UserData_QueueEntry_Metadata_STGPictureID_SHIFT   24

#define BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_MASK        0x00008000
#define BVCE_FW_P_UserData_QueueEntry_Metadata_Polarity_SHIFT       15

#define BVCE_FW_P_UserData_QueueEntry_Metadata_Length_MASK          0x000000FF
#define BVCE_FW_P_UserData_QueueEntry_Metadata_Length_SHIFT         0

typedef struct BVCE_FW_P_UserData_QueueEntry
{
   uint32_t uiMetadata; /* Bits[31:24] - LSB of STG Picture ID
                         * Bits[23:16] - Reserved
                         * Bit[15]     - Polarity (0=Top,1=Bottom)
                         * Bits[14:8]  - Reserved
                         * Bits[ 7:0]  - Length
                         */
   uint32_t uiOffset;  /* Offset into DRAM of the user data packet descriptor relative to the base offset. See BVCE_FW_P_UserData_PacketDescriptor */
} BVCE_FW_P_UserData_QueueEntry;

#define BVCE_FW_P_USERDATA_QUEUE_LENGTH 72

typedef struct BVCE_FW_P_UserData_Queue
{
   uint32_t uiReadOffset;
   uint32_t uiWriteOffset; /* Offset where 0 is the first entry in the queue, and BVCE_FW_P_USERDATA_QUEUE_LENGTH-1 is the last */
   BVCE_FW_P_UserData_QueueEntry astQueue[BVCE_FW_P_USERDATA_QUEUE_LENGTH];
   uint32_t uiBaseOffsetLo;
   uint32_t uiBaseOffsetHi;
} BVCE_FW_P_UserData_Queue;

/* Picture Buffer Info
 *
 * There are 5 types of buffers that the VIP can generate
 *  1) Luma
 *  2) Chroma
 *  3) Decimated Luma - 2H2V
 *  4) Decimated Luma - 2H1V
 *  5) Shifted Chroma
 *
 *  Each progressive picture consists of 1/2 + 3 and/or 4 (depending on how the VIP is configured)
 *  Each interlaced picture also contains 5
 *
 *  The lifetime (# of vsyncs) the buffer is needed varies.
 *   - 1 and 2 are released together (aka LumaChroma)
 *   - 3 and 4 are released together (aka DecimatedLuma)
 *
 *  A picture is added via BVN2VICE Mailbox and the VIP interrupt.
 *
 *  Each _picture_ consists 2 types of elements:
 *     1) 3-5 buffers as described above
 *     2) Metadata (BVN MailBox Info, ITFP, etc)
 *
 *  There are 5 picture buffer "release" queues that is used by VCE to release
 *  buffers for re-use back to BVN.
 *
 */

#define BVCE_P_FW_PICTURE_QUEUE_LENGTH 20

typedef struct BVCE_P_FW_PictureBufferOffset
{
   uint32_t uiOffsetLo; /* Physical offset of picture buffer (LSB) */
   uint32_t uiOffsetHi; /* Physical offset of picture buffer (MSB) */
} BVCE_P_FW_PictureBufferOffset;

#define BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK 0x80000000
#define BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT 31
#define BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK 0x70000000
#define BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT 28
#define BVCE_P_FW_PictureBufferInfo_Metadata_RESERVED_MASK 0x0FFF0000
#define BVCE_P_FW_PictureBufferInfo_Metadata_RESERVED_SHIFT 16
#define BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK 0x0000FFFF
#define BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT 0

typedef struct BVCE_P_FW_PictureBufferInfo
{
   BVCE_P_FW_PictureBufferOffset stOffset;
   uint32_t uiMetadata; /* [31:31] DCXV Flag
                         * [30:28] Decimation Value (in power of two) (0=no decimation, 1=2x decimation, 2=4x decimation, etc)
                         * [27:16] Reserved
                         * [15:00] Number of rows in macroblocks (NMBY)
                         */
} BVCE_P_FW_PictureBufferInfo;

typedef struct BVCE_P_FW_PictureBuffer_Queue
{
   uint32_t uiReadOffset;
   uint32_t uiWriteOffset; /* Offset where 0 is the first entry in the queue, and BVCE_P_FW_Picture_QUEUE_LENGTH-1 is the last */
   BVCE_P_FW_PictureBufferOffset astQueue[BVCE_P_FW_PICTURE_QUEUE_LENGTH];
} BVCE_P_FW_PictureBuffer_Queue;

typedef enum
{
   SOURCE_POLARITY_TOP = 0,
   SOURCE_POLARITY_BOT,
   SOURCE_POLARITY_FRAME
} Polarity_e;

typedef enum
{
   SOURCE_CADENCE_TYPE_UNLOCKED = 0,
   SOURCE_CADENCE_TYPE_3_2,
   SOURCE_CADENCE_TYPE_2_2
} CadenceType_e;

typedef enum
{
   SOURCE_CADENCE_PHASE_0 = 0,
   SOURCE_CADENCE_PHASE_1,
   SOURCE_CADENCE_PHASE_2,
   SOURCE_CADENCE_PHASE_3,
   SOURCE_CADENCE_PHASE_4
} CadencePhase_e;

typedef enum
{
   SOURCE_BAR_DATA_TYPE_INVALID = 0,
   SOURCE_BAR_DATA_TYPE_TOP_BOTTOM,
   SOURCE_BAR_DATA_TYPE_LEFT_RIGHT
} BarDataType_e;

typedef enum
{
   SOURCE_ORIENTATION_TYPE_2D = 0,
   SOURCE_ORIENTATION_TYPE_3D_LEFT_RIGHT,
   SOURCE_ORIENTATION_TYPE_3D_OVER_UNDER,
   SOURCE_ORIENTATION_TYPE_3D_LEFT,
   SOURCE_ORIENTATION_TYPE_3D_RIGHT,
   SOURCE_ORIENTATION_TYPE_3D_LEFT_RIGHT_ENHANCED
} OrientationType_e;

#define BVCE_P_FW_PictureBufferMailbox_Metadata_RESERVED_MASK 0xFFFFFFF8
#define BVCE_P_FW_PictureBufferMailbox_Metadata_RESERVED_SHIFT 3
#define BVCE_P_FW_PictureBufferMailbox_Metadata_LAST_MASK 0x00000004
#define BVCE_P_FW_PictureBufferMailbox_Metadata_LAST_SHIFT 2
#define BVCE_P_FW_PictureBufferMailbox_Metadata_CHANNEL_CHANGE_MASK 0x00000002
#define BVCE_P_FW_PictureBufferMailbox_Metadata_CHANNEL_CHANGE_SHIFT 1
#define BVCE_P_FW_PictureBufferMailbox_Metadata_BUSY_MASK 0x00000001
#define BVCE_P_FW_PictureBufferMailbox_Metadata_BUSY_SHIFT 0

#define BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_MASK 0xFFFF0000
#define BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_SHIFT 16
#define BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_MASK 0x0000FFFF
#define BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_SHIFT 0

#define BVCE_P_FW_PictureBufferMailbox_Cropping_HORIZONTAL_SIZE_MASK 0xFFFF0000
#define BVCE_P_FW_PictureBufferMailbox_Cropping_HORIZONTAL_SIZE_SHIFT 16
#define BVCE_P_FW_PictureBufferMailbox_Cropping_VERTICAL_SIZE_MASK 0x0000FFFF
#define BVCE_P_FW_PictureBufferMailbox_Cropping_VERTICAL_SIZE_SHIFT 0

#define BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_HORIZONTAL_SIZE_MASK 0xFFFF0000
#define BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_HORIZONTAL_SIZE_SHIFT 16
#define BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_VERTICAL_SIZE_MASK 0x0000FFFF
#define BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_VERTICAL_SIZE_SHIFT 0

#define BVCE_P_FW_PictureBufferMailbox_BarData_RESERVED_MASK 0xC0000000
#define BVCE_P_FW_PictureBufferMailbox_BarData_RESERVED_SHIFT 30
#define BVCE_P_FW_PictureBufferMailbox_BarData_TOP_LEFT_VALUE_MASK 0x3FFF0000
#define BVCE_P_FW_PictureBufferMailbox_BarData_TOP_LEFT_VALUE_SHIFT 16
#define BVCE_P_FW_PictureBufferMailbox_BarData_TYPE_MASK 0x0000C000
#define BVCE_P_FW_PictureBufferMailbox_BarData_TYPE_SHIFT 14
#define BVCE_P_FW_PictureBufferMailbox_BarData_BOTTOM_RIGHT_VALUE_MASK 0x00003FFF
#define BVCE_P_FW_PictureBufferMailbox_BarData_BOTTOM_RIGHT_VALUE_SHIFT 0

#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_RESERVED_MASK 0xFFFF8000
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_RESERVED_SHIFT 15
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_ORIENTATION_MASK 0x00007000
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_ORIENTATION_SHIFT 12
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_MODE_MASK 0x00000F00
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_MODE_SHIFT 8
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_VALID_MASK 0x00000080
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_VALID_SHIFT 7
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_PHASE_MASK 0x00000070
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_PHASE_SHIFT 4
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_TYPE_MASK 0x0000000C
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_TYPE_SHIFT 2
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_POLARITY_MASK 0x00000003
#define BVCE_P_FW_PictureBufferMailbox_FormatInfo_POLARITY_SHIFT 0

/* Example cadence scenarios:
 *
 *     VSYNC    | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
 *
 *   Interlaced:
 *     POLARITY | T | B | T | B | T | B | T | B | T | B |
 *
 *    TYPE = 3:2
 *     PHASE    | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | --> [T0 B1 T2] [B3 T4] [B5 T6 B7] [T8 B9]
 *     PHASE    | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | --> T0 B1] [T2 B3] [T4 B5 T6] [B7 T8] [B9
 *     PHASE    | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | --> T0] [B1 T2] [B3 T4 B5] [T6 B7] [T8 B9
 *     PHASE    | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | --> [T0 B1] [T2 B3 T4] [B5 T6] [B7 T8 B9]
 *     PHASE    | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | --> T0] [B1 T2 B3] [T4 B5] [T6 B7 T8] [B9
 *
 *     TYPE = 2:2
 *     PHASE    | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | --> [T0 B1] [T2 B3] [T4 B5] [T6 B7] [T8 B9]
 *     PHASE    | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | --> T0] [B1 T2] [B3 T4] [B5 T6] [B7 T8] [B9
 *
 *   Progressive:
 *      POLARITY | F | F | F | F | F | F | F | F | F | F |
 *
 *      TYPE = 3:2
 *      PHASE    | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | --> [F0 F1 F2] [F3 F4] [F5 F6 F7] [F8 F9]
 *      PHASE    | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | --> F0 F1] [F2 F3] [F4 F5] [F6 F7 F8] [F9
 *      PHASE    | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | --> F0] [F1 F2] [F3 F4 F5] [F6 F7] [F8 F9
 *      PHASE    | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | --> [F0 F1] [F2 F3 F4] [F5 F6] [F7 F8 F9]
 *      PHASE    | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | --> F0] [F1 F2 F3] [F4 F5] [F6 F7 F8] [F9
 *
 *      TYPE = 2:2
 *      PHASE    | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | --> [F0 F1] [F2 F3] [F4 F5] [F6 F7] [F8 F9]
 *      PHASE    | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | --> F0] [F1 F2] [F3 F4] [F5 F6] [F7 F8] [F9
 */

#define BVCE_P_FW_PictureBufferMailbox_TimingInfo_RESERVED_MASK 0xFFFE0000
#define BVCE_P_FW_PictureBufferMailbox_TimingInfo_RESERVED_SHIFT 17
#define BVCE_P_FW_PictureBufferMailbox_TimingInfo_DELTA_PTS_IN_360KHZ_MASK 0x0001FFFE
#define BVCE_P_FW_PictureBufferMailbox_TimingInfo_DELTA_PTS_IN_360KHZ_SHIFT 1
#define BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_MASK 0x00000001
#define BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_SHIFT 0


typedef struct BVCE_P_FW_PictureBufferMailbox
{
   /* Metadata */
   uint32_t uiMetadata;          /* [31:03] Unused
                                  * [02:02] Last Flag
                                  * [01:01] Channel Change Flag
                                  * [00:00] Busy Flag
                                  *          If 0, then the VCE FW is ready for the next picture buffer
                                  *           For each picture, if uiBusy = 0:
                                  *            1) Host updates the MBOX with the picture info
                                  *            2) Host sets busy flag
                                  *            3) When FW processes the MBOX info, it unsets the busy flag */

   /* Display Info */
   uint32_t uiResolution;        /* [31:16] Horizontal Size (in 8x8 blocks)
                                  * [15:00] Vertical Size (in 8x8 blocks) */
   uint32_t uiCropping;          /* [31:16] Right Cropping Size (in pixels)
                                  * [15:00] Bottom Cropping Size (in pixels) */
   uint32_t uiSampleAspectRatio; /* [31:16] Horizontal Size
                                  * [15:00] Vertical Size */
   uint32_t uiBarData;           /* [31:30] Unused
                                  * [29:16] Top/Left Bar Value
                                  * [15:14] Bar Data Type (See BarDataType_e)
                                  * [13:00] Bottom/Right Bar Value */
   uint32_t uiFormatInfo;        /* [31:23] Unused
                                  * [22:15] Vip Picture Index (for Low Delay)
                                  * [14:12] Orientation (See OrientationType_e)
                                  * [11:08] Active Format Data (AFD) Mode
                                  * [07:07] Active Format Data (AFD) Valid Flag
                                  * [06:04] Cadence Phase (See CadencePhase_e)
                                  * [03:02] Cadence Type (See CadenceType_e)
                                  * [01:00] Polarity (See Polarity_e) */

   /* Timing Info */
   uint32_t uiTimingInfo;        /* [16:01] Delta PTS (in 360 Khz = (90 Khz * 4) )
                                  * [00:00] New PTS LSB
                                  *          NewPTSin90Khz = (uiNewPts << 1) | ( ( uiTimingInfo & BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_MASK ) >> BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_SHIFT ) */
   uint32_t uiNewPts;            /* [31:00] New PTS (in 45 Khz) */
   uint32_t uiOriginalPts;       /* [31:00] Original PTS (in 45 Khz) */
   uint32_t uiPictureId;         /* [31:00] Picture Id */

   /* Debug Info */
   uint32_t uiSTCSnapshotLo;     /* [31:00] STC Snapshot LSBs (in 27 Mhz) */
   uint32_t uiSTCSnapshotHi;     /* [31:00] STC Snapshot MSBs (in 27 Mhz) */

    /* Buffer Physical Offsets */
    BVCE_P_FW_PictureBufferInfo    LumaBufPtr;
    BVCE_P_FW_PictureBufferInfo    ChromaBufPtr;
    BVCE_P_FW_PictureBufferInfo    Luma1VBufPtr;
    BVCE_P_FW_PictureBufferInfo    Luma2VBufPtr;
    BVCE_P_FW_PictureBufferInfo    ShiftedChromaBufPtr;
} BVCE_P_FW_PictureBufferMailbox;

typedef struct ViceCmdOpenChannel_t
{
    uint32_t            Command;                            /* OpCode of the command: VICE_CMD_* */
    uint32_t            uiChannel_id;                       /* channel number to open (0..65535) */
    VCE_PTR(uint8_t)    pNonSecureBufferBaseLo;             /* pointer to the location that will contain the non-secure buffer */
    VCE_PTR(uint8_t)    pNonSecureBufferBaseHi;             /* pointer to the location that will contain the non-secure buffer */
    uint32_t            uiNonSecureBufferSize;              /* size of the non-secure buffer */
    VCE_PTR(uint8_t)    pSecureBufferBaseLo;                /* pointer to the location that will contain the secure buffer. */
    VCE_PTR(uint8_t)    pSecureBufferBaseHi;                /* pointer to the location that will contain the secure buffer. */
    uint32_t            uiSecureBufferSize;                 /* size of the secure buffer */
    uint32_t            uiMaxNumChannels;                   /* maximum number of channels, 0: max num of ch supported on current platform */
} ViceCmdOpenChannel_t;


typedef struct ViceCmdOpenChannelResponse_t
{
    uint32_t                          Command;              /* OpCode of the command: VICE_CMD_* */
    uint32_t                          Status;               /* Status: VICE_CMD_OK, or VICE_CMD_ERR_* (see definition above) */
    VCE_PTR(BVCE_FW_P_UserData_Queue) pUserDataQInfoBase;   /* Base DCCM buffer address for the user data queue */

    /* VIP Buffer Release Info */
    VCE_PTR(BVCE_P_FW_PictureBuffer_Queue) pLumaBufferReleaseQInfoBase;           /* Base DCCM buffer address for the Luma *Buffer* Release Q (VCE --> BVN) */
    VCE_PTR(BVCE_P_FW_PictureBuffer_Queue) pChromaBufferReleaseQInfoBase;         /* Base DCCM buffer address for the Luma *Buffer* Release Q (VCE --> BVN) */
    VCE_PTR(BVCE_P_FW_PictureBuffer_Queue) p1VLumaBufferReleaseQInfoBase;       /* Base DCCM buffer address for the 2H1V Decimated Luma *Buffer* Release Q (VCE --> BVN) */
    VCE_PTR(BVCE_P_FW_PictureBuffer_Queue) p2VLumaBufferReleaseQInfoBase;       /* Base DCCM buffer address for the 2H2V Decimated Luma *Buffer* Release Q (VCE --> BVN) */
    VCE_PTR(BVCE_P_FW_PictureBuffer_Queue) pShiftedChromaBufferReleaseQInfoBase;  /* Base DCCM buffer address for the Shifted Chroma *Buffer* Release Q (VCE --> BVN) */
} ViceCmdOpenChannelResponse_t;




/* ---- START ---- */

typedef struct ViceCmdStartChannel_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */
    uint32_t    uiChannel_id;                               /* channel number to open (0..65535) */

} ViceCmdStartChannel_t;

typedef struct ViceCmdStartChannelResponse_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */
    uint32_t    Status;                                     /* Status: VICE_CMD_OK, or VICE_CMD_ERR_* (see definition above) */

} ViceCmdStartChannelResponse_t;




/* ---- STOP  ---- */

typedef struct ViceCmdStopChannel_t
{
    uint32_t     Command;                                   /* OpCode of the command: VICE_CMD_* */
    uint32_t     uiChannel_id;                              /* channel number to open (0..65535) */
    uint32_t     Flags;                                     /* 32 flags (describe below STOP_FLAG_*) */

} ViceCmdStopChannel_t;

/* Flag indicating if FW should stop the current encode as fast as possible and be ready to accept a START immediately
   or normal stop. 0: normal stop, 1: fast stop.  */
#define STOP_FLAG_FAST_CHANNEL_STOP_POS                 0

typedef struct ViceCmdStopChannelResponse_t
{
    uint32_t     Command;                                   /* OpCode of the command: VICE_CMD_* */
    uint32_t     Status;                                    /* Status: VICE_CMD_OK, or VICE_CMD_ERR_* (see definition above) */

} ViceCmdStopChannelResponse_t;




/* ---- CLOSE ---- */

typedef struct ViceCmdCloseChannel_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */
    uint32_t    uiChannel_id;                               /* channel number to open (0..65535) */

} ViceCmdCloseChannel_t;

typedef struct ViceCmdCloseChannelResponse_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */
    uint32_t    Status;                                     /* Status: VICE_CMD_OK, or VICE_CMD_ERR_* (see definition above) */

} ViceCmdCloseChannelResponse_t;




/* ---- CONFIG ---- */

/* Encoding Standards */
#define ENCODING_STD_H264       0
#define ENCODING_STD_MPEG2      1
#define ENCODING_STD_MPEG4      2
#define ENCODING_STD_VP8        3
#define ENCODING_STD_HEVC       4
#define ENCODING_STD_VP9        5

/* WARNING: These value correspond to values written to hardware registers
 *          DO NOT CHANGE THEM !
 */
#define ENCODING_HEVC_PROFILE_TIER_MAIN         0
#define ENCODING_HEVC_PROFILE_IDC_MAIN          1
#define ENCODING_AVC_PROFILE_HIGH               100
#define ENCODING_AVC_PROFILE_MAIN               77
#define ENCODING_AVC_PROFILE_BASELINE           66
#define ENCODING_MPEG2_PROFILE_MAIN             4
#define ENCODING_MPEG4_PROFILE_SIMPLE           0
#define ENCODING_MPEG4_PROFILE_ADVANCED_SIMPLE  1
#define ENCODING_VP8_PROFILE_STANDARD_LF        0
#define ENCODING_VP9_PROFILE                    0

typedef enum
{
    ENCODING_HEVC_LEVEL_40      = 120,  /* 4*30 */
    ENCODING_HEVC_LEVEL_41      = 123,  /* 4.1*30 */
    ENCODING_HEVC_LEVEL_50      = 150,  /* 5*30 */
    ENCODING_AVC_LEVEL_10       = 10,
    ENCODING_AVC_LEVEL_10b      = 14,
    ENCODING_AVC_LEVEL_11       = 11,
    ENCODING_AVC_LEVEL_12       = 12,
    ENCODING_AVC_LEVEL_13       = 13,
    ENCODING_AVC_LEVEL_20       = 20,
    ENCODING_AVC_LEVEL_21       = 21,
    ENCODING_AVC_LEVEL_22       = 22,
    ENCODING_AVC_LEVEL_30       = 30,
    ENCODING_AVC_LEVEL_31       = 31,
    ENCODING_AVC_LEVEL_32       = 32,
    ENCODING_AVC_LEVEL_40       = 40,
    ENCODING_AVC_LEVEL_41       = 41,
    ENCODING_AVC_LEVEL_42       = 42,
    ENCODING_MPEG2_LEVEL_LOW    = 10,
    ENCODING_MPEG2_LEVEL_MAIN   = 8,
    ENCODING_MPEG2_LEVEL_HIGH   = 4,
    ENCODING_MPEG4_LEVEL_1      = 1,
    ENCODING_MPEG4_LEVEL_2      = 2,
    ENCODING_MPEG4_LEVEL_3      = 3,
    ENCODING_MPEG4_LEVEL_4      = 4,
    ENCODING_MPEG4_LEVEL_5      = 5
}EncoderLevel_e;

typedef enum
{
    ENCODING_FRAME_RATE_0749 =  749,
    ENCODING_FRAME_RATE_0750 =  750,
    ENCODING_FRAME_RATE_0999 =  999,
    ENCODING_FRAME_RATE_1000 = 1000,
    ENCODING_FRAME_RATE_1198 = 1198,
    ENCODING_FRAME_RATE_1200 = 1200,
    ENCODING_FRAME_RATE_1250 = 1250,
    ENCODING_FRAME_RATE_1498 = 1498,
    ENCODING_FRAME_RATE_1500 = 1500,
    ENCODING_FRAME_RATE_1998 = 1998,
    ENCODING_FRAME_RATE_2000 = 2000,
    ENCODING_FRAME_RATE_2397 = 2397,
    ENCODING_FRAME_RATE_2400 = 2400,
    ENCODING_FRAME_RATE_2500 = 2500,
    ENCODING_FRAME_RATE_2997 = 2997,
    ENCODING_FRAME_RATE_3000 = 3000,
    ENCODING_FRAME_RATE_5000 = 5000,
    ENCODING_FRAME_RATE_5994 = 5994,
    ENCODING_FRAME_RATE_6000 = 6000
}FrameRate_e;


typedef enum
{
       ENCODING_FRAME_RATE_CODE_UNKNOWN = 0,
       ENCODING_FRAME_RATE_CODE_2397 = 1,
       ENCODING_FRAME_RATE_CODE_2400,
       ENCODING_FRAME_RATE_CODE_2500,
       ENCODING_FRAME_RATE_CODE_2997,
       ENCODING_FRAME_RATE_CODE_3000,
       ENCODING_FRAME_RATE_CODE_5000,
       ENCODING_FRAME_RATE_CODE_5994,
       ENCODING_FRAME_RATE_CODE_6000,
       ENCODING_FRAME_RATE_CODE_1498,
       ENCODING_FRAME_RATE_CODE_0749,
       ENCODING_FRAME_RATE_CODE_1000,
       ENCODING_FRAME_RATE_CODE_1500,
       ENCODING_FRAME_RATE_CODE_2000,
       ENCODING_FRAME_RATE_CODE_1998,
       ENCODING_FRAME_RATE_CODE_1250,
       ENCODING_FRAME_RATE_CODE_0750,
       ENCODING_FRAME_RATE_CODE_1200,
       ENCODING_FRAME_RATE_CODE_1198,
       ENCODING_FRAME_RATE_CODE_0999
} FrameRateCode_e;

typedef struct FPNumber
{
    uint32_t mantissa;        /* the significant digits of the floating point number */
    int32_t exponent;         /* the floating point number exponent */
} FPNumber_t;

typedef enum
{
    REFRESH_ROW = 0,
    REFRESH_COLUMN = 1
}ForceIntraPattern_e;

/* Valid GOP Structures */
/* GOP struct defines ordered from lowest delay (no B pictures) to highest delay (most consecutive B pictures) with track_input and infinite_IP always last */
#define ENCODING_GOP_STRUCT_I               0               /* [I][I][I][I], Gop Length ignored */
#define ENCODING_GOP_STRUCT_IP              1               /* [IPPPP][IPPPP]                   */
#define ENCODING_GOP_STRUCT_INFINITE_IP     2               /* [IPPPPPPPPPPPPP] until host send RAP         */
#define ENCODING_GOP_STRUCT_IBP             3               /* [IBPBP][IBPBP]                   */
#define ENCODING_GOP_STRUCT_IBBP            4               /* [IBBPBB][IBBPBB] B pictures are not used as a reference                */
#define ENCODING_GOP_STRUCT_IBBP_B_REF      5               /* [IBBPBB][IBBPBB] 1st B picture in a sub GOP is used as a reference     */
#define ENCODING_GOP_STRUCT_IBBBP           6               /* [IBBBPBBBP][IBBBPBBBP]           */
#define ENCODING_GOP_STRUCT_IBBBBBP         7               /* [IBBBBBPBBBBBP][IBBBBBPBBBBBP]               */
#define ENCODING_GOP_STRUCT_IBBBBBBBP       8               /* [IBBBBBBBPBBBBBBBP][IBBBBBBBPBBBBBBBP]       */

/* Mode of Operation */
#define ENCODER_MODE_HIGH_DELAY             0               /* high delay mode (picture by picture) */
#define ENCODER_MODE_LOW_DELAY              1               /* Low delay mode (line by line)        */
#define ENCODER_MODE_AFAP                   2               /* as-fast-as-possible mode (encoder back-pressure) */

/* Input type */
#define ENCODER_INPUT_TYPE_PROGRESSIVE      0
#define ENCODER_INPUT_TYPE_INTERLACED       1

/* Rate mode */
#define FIXED_FRAME_RATE_MODE               0
#define VARIABLE_FRAME_RATE_MODE            1

typedef struct ViceCmdConfigChannel_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */
    uint32_t    uiChannel_id;                               /* channel number to open (0..65535) */
    uint32_t    Protocol;                                   /* encoding standard (see ENCODING_STD_* above) */
    uint32_t    Profile;                                    /* See ENCODING_STD_PROFILE_* above */
    uint32_t    Level;                                      /* See ENCODING_STD_LEVEL_* above */
    uint32_t    FrameRateCode;                              /* See FrameRateCode_e above */
    uint32_t    MaxBitrate;                                 /* Peak output bit rate in bits/sec */
    uint32_t    GopStructure;                               /* See GOP Structure definitions above (GOP_STRUCT_*) and GOP STRUCTURE flags */
    uint32_t    GopLength;                                  /* Number of pictures in a GOP ( e.g. Gop length of IBBPBBP is 7 ) */
    uint32_t    Mode;                                       /* High Delay, Low Delay, AFAP (ENCODER_MODE_*) */
    uint32_t    InputType;                                  /* Interlaced/Progressive (ENCODER_INPUT_TYPE_*) */
    uint32_t    EventMask;                                  /* indicates which events can generate an interrupt */
    uint32_t    Flags;                                      /* 32 flags (describe below CONFIG_FLAG_*) */
    uint32_t    StcID;                                      /* ID of the STC that this channel should use */
    uint32_t    ContextID;                                  /* ID of the ITB/CDB Context that the FW should use. */
    uint32_t    ITBBufPtrLo;                                /* pointer to the ITB buffer base */
    uint32_t    ITBBufPtrHi;                                /* pointer to the ITB buffer base */
    uint32_t    ITBBufSize;                                 /* size of the ITB buffer in bytes */
    uint32_t    CDBBufPtrLo;                                /* pointer to the CDB buffer base */
    uint32_t    CDBBufPtrHi;                                /* pointer to the CDB buffer base */
    uint32_t    CDBBufSize;                                 /* size of the CDB buffer in bytes */
    uint32_t    A2PDelay;                                   /* Desired End-to-end delay given by the Host */

    /* A2P optimizations arguments */
    uint32_t    FrameRateCodeLimit;                         /* Output frame rate limitations in the current session. See FrameRateCode_e above */
    uint32_t    RateBufferDelayInMs;                        /* Delay in ms to give for the Rate Control Buffer (higher=better quality)  */
    uint32_t    MinAllowedBvnFrameRateCode;                 /* The minimum allowed input frame rate (BVN) during the whole session. See FrameRateCode_e above. */
    uint32_t    MaxPictureSizeInPels;                       /* Maximum picture size during the current session  */
    uint32_t    MaxAllowedGopStructure;                     /* Maximum allowed Gop Structure to indicate maximum picture reorder delay */

    uint32_t    NumSlicesPerPicture;                        /* Number of slices per picture */
    uint32_t    ForceIntraMode;                             /* Force Intra Mode configuration */
    uint32_t    TargetBitrate;                              /* Target output bit rate in bits/sec. If 0 : means TargetBitrate == MaxBitrate */
    uint32_t    NumParallelEncodes;                         /* Number of simultaneous NRT encode. If > 1: mean FNRT mode is on */

    /* Segment Mode Rate Control */
    uint32_t   RCSegmentDurationInMsec;                     /* Segment duration in milliseconds. */
    uint32_t   RCSegmentModeParams;                         /* Segment mode RC configuration. */

} ViceCmdConfigChannel_t;


/* Flag that indicates the start of a new RAP point     1:new RAP point, 0: other                                       */
#define CONFIG_FLAG_NEW_RAP_POS                             0
/* Flag indicating that bitrate change is effective only when there is a BVN input change  1: true, 0:false             */
#define CONFIG_FLAG_APPLY_BITRATE_BVN_CHANGE_POS            1
/* Flag indicating if the encoder should encode at a fixed-rate or variable-rate. In a variable-rate mode the encoder
 * could decide to encode at a lower frame-rate if it detects a 3:2 pull-down for example.
 * Values are 0: fixed rate, 1: variable rate                                                                           */
#define CONFIG_FLAG_RATE_MODE_POS                           2
/* Flag indicating that the firmware should use the given Cabac context pointer and program the CABAC registers         */
#define CONFIG_FLAG_RESET_CABAC_CONTEXT_POS                 3
/* Flag indicating if ITFP algorithm is enabled or not. 0: enabled, 1: disabled. By default it is enabled.              */
#define CONFIG_FLAG_ITFP_DISABLED_POS                       4
/* Flag indicating if next start is fast channel change or not. 0: NO fast channel change, 1: YES fast channel change.  */
#define CONFIG_FLAG_FAST_CHANNEL_CHANGE_POS                 5
/* Indicates if rate control is allowed to restart the GOP structure when a scene change is detected. This is only valid
   in IP GOP. In other GOP structures, the flag is ignored
    0: The rate control should follow the GOP structure, 1: YES RC is allowed to change the GOP structure.              */
#define CONFIG_FLAG_NEW_GOP_ON_SCENE_CHANGE_POS             6
/* Flag indicating to the FW whether or not to always send a FPA SEI message.
   0: Do not always send FPA SEI( Default ). 1: Always send FPA SEI.    */
#define CONFIG_FLAG_ALWAYS_SEND_FPA_SEI_POS                 7
/* Flag indicating if CAVLC is forced or not. 0: CAVLC not forced, 1: CAVLC forced. By default it is not forced.        */
#define CONFIG_FLAG_FORCE_CAVLC_POS                         8
/* Flag for restricting the encoder to use only one reference for P pictures. 0: No restriction  1: use 1 ref for P pictures */
#define CONFIG_FLAG_FORCE_1_REF_P_PICTURE                   9
/* Flag for disabling the optional patches. 0: Enable optiona patches  1: Disable Optional patches */
#define CONFIG_FLAG_DISABLE_OPTIONAL_PATCHES                10
/* Flag to enable Segment Mode Rate Control (as opposed to the default HRD buffer model RC).
   0: Use HRD Mode Rate Control.  1: Use Segment Mode Rate Control */
#define CONFIG_FLAG_ENABLE_RC_SEGMENT_MODE                  11
/* Flag for disabling picture drops from HRD underflow. 0: Enable HRD picture drop  1: Disable HRD picture drop */
#define CONFIG_FLAG_DISABLE_HRD_DROP_PICTURE                12
/* Flag for enabling sparse frame rate mode. 0: Disable sparse frame rate. 1: Enable sparse frame rate */
#define CONFIG_FLAG_ENABLE_SPARSE_FRAME_RATE_MODE           13

#define GOP_LENGTH_MASK                                     0x0000FFFF          /* Gop length is limited to 16 bits     */
#define GOP_LENGTH_OR_DURATION_FLAG_MASK                    0x80000000          /* 0 = gop length is in frames. 1= gop length is in 1/1000 sec duration  */
#define GOP_LENGTH_RAMPING_N_MASK                           0x0F000000          /* GOP duration ramping factor N. The duration of the first N GOPs is GopDuration/N.   */
#define GOP_LENGTH_RAMPING_N_SHIFT                          24
#define GOP_LENGTH_RAMPING_M_MASK                           0x00F00000          /* GOP duration ramping factor M. The duration of the next M GOPs is GopDuration*(M+N-1)/M.  */
#define GOP_LENGTH_RAMPING_M_SHIFT                          20
#define GOP_LENGTH_MINIMAL_SCENE_CHANGE_MASK                0x000F0000          /* Minimal Gop length. A scene change can restart the GOP only if the minimal number of frames were encoded */
#define GOP_LENGTH_MINIMAL_SCENE_CHANGE_SHIFT               16

/* GOP STRUCTURE flags */
/* This flags are part of the GopStructure field in ViceCmdConfigChannel_t */
#define GOP_STRUCTURE_MASK                                  0x0000000F          /* 4 LSBs are for the GOP STRUCTURE definitions */
#define ALLOW_OPEN_GOP_STRUCTURE_MASK                       0x80000000          /* Allows the FW to use an open GOP - mask    */
#define ALLOW_OPEN_GOP_STRUCTURE_SHIFT                      31                  /* Allows the FW to use an open GOP - shift   */

/* Minimum output frame rate in the current session */
#define CONFIG_FRAME_RATE_CODE_LIMIT_MAX_MASK                    0x00FF0000
#define CONFIG_FRAME_RATE_CODE_LIMIT_MAX_SHIFT                   16

/* Minimum output frame rate in the current session */
#define CONFIG_FRAME_RATE_CODE_LIMIT_MIN_MASK                    0x000000FF
#define CONFIG_FRAME_RATE_CODE_LIMIT_MIN_SHIFT                   0

/* Maximum picture height during the current session */
#define CONFIG_MAX_PICTURE_SIZE_IN_PELS_HEIGHT_MASK         0xFFFF0000
#define CONFIG_MAX_PICTURE_SIZE_IN_PELS_HEIGHT_SHIFT        16

/* Maximum picture width during the current session  */
#define CONFIG_MAX_PICTURE_SIZE_IN_PELS_WIDTH_MASK          0x0000FFFF
#define CONFIG_MAX_PICTURE_SIZE_IN_PELS_WIDTH_SHIFT         0

/* Intra refresh frame distance */
#define CONFIG_FORCE_INTRA_MODE_INTRA_REFRESH_FRAME_DISTANCE_MASK 0xFF000000
#define CONFIG_FORCE_INTRA_MODE_INTRA_REFRESH_FRAME_DISTANCE_SHIFT  24

/* Number of Entries To refresh */
/* Relevant only for V2.x. For V3 it is always 1 */
#define CONFIG_FORCE_INTRA_MODE_NUMBER_OF_ENTRIES_TO_REFRESH_MASK 0x00FF0000
#define CONFIG_FORCE_INTRA_MODE_NUMBER_OF_ENTRIES_TO_REFRESH_SHIFT  16

/* Refresh Pattern Selected - see ForceIntraPattern_e definition */
/* Relevant only for V2.x. For V3 it is always column pattern refresh */
#define CONFIG_FORCE_INTRA_MODE_REFRESH_PATTERN_SELECTED_MASK 0x00000002
#define CONFIG_FORCE_INTRA_MODE_REFRESH_PATTERN_SELECTED_SHIFT  1

/* Enable Force Intra Mode */
#define CONFIG_FORCE_INTRA_MODE_ENABLE_MASK                 0x00000001
#define CONFIG_FORCE_INTRA_MODE_ENABLE_SHIFT                0

/* RCSegmentModeParams */
/* Maximum  percentage error tolerance allowed relative to target total bits per segment
   when actual bits are greater than target bits. */
#define CONFIG_UPPER_RC_SEGMENT_TOLERANCE_MASK              0x000000FF
#define CONFIG_UPPER_RC_SEGMENT_TOLERANCE_SHIFT             0

/* Maximum  percentage error tolerance allowed relative to target total bits per segment
   when actual bits are less than target bits */
#define CONFIG_LOWER_RC_SEGMENT_TOLERANCE_MASK              0x0000FF00
#define CONFIG_LOWER_RC_SEGMENT_TOLERANCE_SHIFT             8

/* Percentage slope factor equal or smaller than 100% for upper limit relative to slope
*  of average target bitrate */
#define CONFIG_UPPER_LIMIT_SLOPE_FACTOR_MASK                0x00FF0000
#define CONFIG_UPPER_LIMIT_SLOPE_FACTOR_SHIFT               16

/* Percentage slope factor greater than or equal to a 100% for lower limit relative to slope
*  of average target bitrate */
#define CONFIG_LOWER_LIMIT_SLOPE_FACTOR_MASK                0xFF000000
#define CONFIG_LOWER_LIMIT_SLOPE_FACTOR_SHIFT               24


typedef struct ViceCmdConfigChannelResponse_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_*                                */
    uint32_t    Status;                                     /* Status: VICE_CMD_OK, or VICE_CMD_ERR_* (see definition above)   */

} ViceCmdConfigChannelResponse_t;


/* Default A2P parameters values */
#define DEFAULT_A2P_RATE_BUFFER          0                                      /* Default value for RateBufferDelayInMs (0: use FW determined default) */
#define DEFAULT_A2P_MIN_FRAMERATE        ENCODING_FRAME_RATE_1498               /* Default value for MinFrameRateLimit  */
#define DEFAULT_A2P_MAX_FRAMERATE        ENCODING_FRAME_RATE_6000               /* Default value for MaxFrameRateLimit  */
#define DEFAULT_A2P_BVN_RATE             ENCODING_FRAME_RATE_2397               /* Default value for MinAllowedBvnFrameRate    */
#define DEFAULT_A2P_DISABLE_ITFP         0                                      /* Default value for CONFIG_FLAG_ITFP_DISABLED value */
#define DEFAULT_A2P_MAX_PIC_WIDTH_PELS   1920                                   /* Default value for MaxPictureWidthInPels */
#define DEFAULT_A2P_MAX_PIC_HEIGT_PELS   1088                                   /* Default value for MaxPictureHeightInPels */



uint32_t
BVCE_FW_P_CalcVideoA2Pdelay(
    uint32_t            *MaxAllowedA2Pdelay,                /* output max A2P delay allowed for Video */
    uint32_t            Protocol,                           /* encoding standard  */
    uint32_t            Profile,                            /* encoding profile   */
    EncoderLevel_e      Level,                              /* Profile Level      */
    FrameRateCode_e     FrameRateCode,                      /* Frame rate code in unit of integer. See FrameRateCode_e above. */
    uint32_t            MaxBitrate,                         /* Peak output bit rate in bits/sec */
    uint8_t             Mode,                               /* Encoder mode of operation */
    uint32_t            rateBufferDelay,                    /* Input rate buffer delay   */
    FrameRateCode_e     minFrameRateCodeLimit,              /* Minimum frame rate allowed during dynamic rate change. See FrameRateCode_e above. */
    FrameRateCode_e     MinAllowedBvnFrameRateCode,         /* Input BVN frame rate. See FrameRateCode_e above.  */
    uint8_t             MultiChannelEnable,                 /* Flag dual channel operation  */
    uint8_t             ITFPenable,                         /* ITFP enable */
    uint8_t             InputType,                          /* Interlaced or Progressive input type */
    uint8_t             GopStruct,                          /* Gop structure */
    uint32_t            PictureWidthInPels,                 /* Max picture resolution allowed */
    uint32_t            PictureHeightInPels,
    uint8_t             MaxNumFNRT
);

/* nonesecure DRAM size calculations */
typedef enum
{
      BVCE_FW_P_COREVERSION_V1   = 0,
      BVCE_FW_P_COREVERSION_V2   = 1,
      BVCE_FW_P_COREVERSION_V2_1 = 2,     /* Use this for any 2.1 core revision except 2.1.0.3, 2.1.1.2 and 2.1.2.2 */
      BVCE_FW_P_COREVERSION_V2_1_0_3 = 3,
      BVCE_FW_P_COREVERSION_V2_1_1_2 = 4,
      BVCE_FW_P_COREVERSION_V2_1_2_2 = 5,
      BVCE_FW_P_COREVERSION_V2_1_3_2 = 6,
      BVCE_FW_P_COREVERSION_V3_0_0_2 = 7,
      BVCE_FW_P_COREVERSION_V3_0_1_2 = 8,
      BVCE_FW_P_COREVERSION_V3_0_2_2 = 9,

      /* Add new revisions ABOVE this line */
      BVCE_FW_P_COREVERSION_MAX
} BVCE_FW_P_CoreVersion_e;

typedef struct BVCE_FW_P_CoreSettings_t
{
      BVCE_FW_P_CoreVersion_e eVersion;
} BVCE_FW_P_CoreSettings_t;


typedef struct BVCE_FW_P_NonSecureMemSettings_t
{
    uint8_t     InputType;
    uint32_t    DramStripeWidth;
    uint32_t    X;
    uint32_t    Y;
    uint32_t    MaxPictureWidthInPels;
    uint32_t    MaxPictureHeightInPels;
    uint32_t    DcxvEnable;
    uint32_t    NewBvnMailboxEnable;
    uint32_t    PageSize;
    uint32_t    MaxGopStructure;
} BVCE_FW_P_NonSecureMemSettings_t;


/* Populates the default non-secure memory settings based on the specified core settings */
void BVCE_FW_P_GetDefaultNonSecureMemSettings( const BVCE_FW_P_CoreSettings_t *pstCoreSettings, BVCE_FW_P_NonSecureMemSettings_t *pstMemSettings );

/* Returns the memory required for the specified core/memory settings */
uint32_t BVCE_FW_P_CalcNonSecureMem ( const BVCE_FW_P_CoreSettings_t *pstCoreSettings, const BVCE_FW_P_NonSecureMemSettings_t *pstMemSettings );


uint32_t BVCE_FW_P_ConvertFrameRate(FrameRate_e framesPerSecond, FPNumber_t *FP_inverse);
uint32_t BVCE_FW_P_CalcHRDbufferSize(uint32_t  Protocol, uint32_t  Profile, EncoderLevel_e  Level);
uint32_t BVCE_FW_P_Compute_DeeIn27MhzTicks(uint32_t averageBitsPerPic, uint32_t hrdBufferSize, FPNumber_t FP_ticksPerBit,
uint32_t maxPictureIntervalIn27MhzTicks, uint32_t *pMaxAllowedBitsPerPicture, uint8_t MaxNumFNRT);

uint32_t BVCE_FW_P_Compute_MaxDeeIn27MhzTicks(uint32_t hrdBufferSize, FPNumber_t FP_ticksPerBit);
void BVCE_FW_P_ComputeRateInTicksPerBit(FPNumber_t *pt_FP_ticksPerBit, uint32_t bitrateBPS);
uint32_t BVCE_FW_P_Compute_EncodeDelayIn27MHzTicks(FrameRate_e MinFrameRateLimit, uint8_t InputType, uint8_t MaxAllowedGopStruct);
FrameRate_e BVCE_FW_P_FrameRateCodeToFrameRate(FrameRateCode_e FrameRateCode);

/* ---- DEBUG ---- */

/* Processor ID */
#define PROC_ID_PICARC      0
#define PROC_ID_MBARC       1

/* size of the command buffer in bytes. We make sure it's a multiple of 4
 * so that the structure is properly aligned.
 * */
#define COMMAND_BUFFER_SIZE_BYTES   ((32/sizeof(uint32_t))*sizeof(uint32_t))

typedef struct ViceCmdDebugChannel_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */
    uint32_t    uiChannel_id;                               /* channel number to open (0..65535) */
    uint32_t    ProcID;                                     /* Processor ID see PROC_ID_* above */
    uint8_t     aCommands[COMMAND_BUFFER_SIZE_BYTES];       /* null-terminated string of debug uart commands (for systems without UARTs) */

} ViceCmdDebugChannel_t;

typedef struct ViceCmdDebugChannelResponse_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */
    uint32_t    Status;                                     /* Status: VICE_CMD_OK, or VICE_CMD_ERR_* (see definition above) */

} ViceCmdDebugChannelResponse_t;




/* ---- CHANNEL STATUS ---- */

typedef struct ViceCmdGetChannelStatus_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */
    uint32_t    uiChannel_id;                               /* channel number to open (0..65535) */

} ViceCmdGetChannelStatus_t;

/* Indicate the state of the encoder picture loop ( debug information ) */
typedef enum
{
    PIPELINE_STATE_WAIT_CME,                   /* Regular state, waiting for a picture to appear in CME output queue */
    PIPELINE_STATE_WAIT_MBP,                   /* Waiting for the End-Of-Picture message from MB-ARC */
    PIPELINE_STATE_MAX_NUM_STATES
} PipelineState_t;


typedef struct StatusInfo_t
{
    uint32_t        uiEventFlags;                           /* These bits are set by FW as events occur.  FW clears these bits */
                                                            /* only after it responds to a GET_STATUS command */
    /* The following fields are overwritten on-the-fly */
    uint32_t        uiTotalErrors;
    uint32_t        NumOfPicsMbArcFinished;
    uint32_t        NumOfPicsVipConfiguredToProcess;
    uint32_t        NumOfPicsVipDroppedDueToFRC;
    uint32_t        NumOfPicsVipDroppedDueToPerformance;
    uint32_t        NumOfPicsVipDroppedDueToChannelNotStarted;
    uint32_t        PicId;
    uint32_t        FwState;
    uint32_t        PictureLoopStatus;
    uint64_t        uiSTCSnapshot;
    uint32_t        NumOfPicsVipDroppedDueToHRDUnderFlow;
    uint32_t        uiEtsDtsOffset;                         /* The ETS to DTS offset for the encode session as determined by RC */
    uint32_t        Throughput;                             /* Average time between picture end messages in 351mhz ticks */
} StatusInfo_t;


typedef struct ViceCmdGetChannelStatusResponse_t
{
    uint32_t      Command;                                  /* OpCode of the command: VICE_CMD_* */
    uint32_t      Status;                                   /* Status: VICE_CMD_OK, or VICE_CMD_ERR_* (see definition above) */
    StatusInfo_t  StatusInfoStruct;                         /* Copy of the Status Info buffer for the given channel */

} ViceCmdGetChannelStatusResponse_t;

/* ---- DEVICE STATUS ---- */

typedef struct ViceCmdGetDeviceStatus_t
{
    uint32_t    Command;                                    /* OpCode of the command: VICE_CMD_* */

} ViceCmdGetDeviceStatus_t;

typedef struct DeviceStatusInfo_t
{
    uint32_t uiEventFlags;                                  /* These bits are set by FW as Event occur.  FW clears these bits */
                                                            /* only after it responds to a GET_STATUS command */

    /* The following fields are overwritten on-the-fly */
    uint32_t uiTotalErrors;
} DeviceStatusInfo_t;


typedef struct ViceCmdGetDeviceStatusResponse_t
{
    uint32_t            Command;                            /* OpCode of the command: VICE_CMD_* */
    uint32_t            Status;                             /* Status: VICE_CMD_OK, or VICE_CMD_ERR_* (see definition above) */
    DeviceStatusInfo_t  DeviceStatusInfoStruct;             /* Copy of the Device Status Info buffer for the given channel */

} ViceCmdGetDeviceStatusResponse_t;

/* ==========================================================*/
/* ------------------  COMMAND UNION ------------------- */
typedef union
{
                uint32_t CommandId;

                ViceCmdInit_t                     ViceCmdInit;
                ViceCmdOpenChannel_t              ViceCmdOpenChannel;
                ViceCmdStartChannel_t             ViceCmdStartChannel;
                ViceCmdStopChannel_t              ViceCmdStopChannel;
                ViceCmdCloseChannel_t             ViceCmdCloseChannel;
                ViceCmdConfigChannel_t            ViceCmdConfigChannel;
                ViceCmdDebugChannel_t             ViceCmdDebugChannel;
                ViceCmdGetChannelStatus_t         ViceCmdGetChannelStatus;
                ViceCmdGetDeviceStatus_t          ViceCmdGetDeviceStatus;

                ViceCmdInitResponse_t             ViceCmdInitResponse;
                ViceCmdOpenChannelResponse_t      ViceCmdOpenChannelResponse;
                ViceCmdStartChannelResponse_t     ViceCmdStartChannelResponse;
                ViceCmdStopChannelResponse_t      ViceCmdStopChannelResponse;
                ViceCmdCloseChannelResponse_t     ViceCmdCloseChannelResponse;
                ViceCmdConfigChannelResponse_t    ViceCmdConfigChannelResponse;
                ViceCmdDebugChannelResponse_t     ViceCmdDebugChannelResponse;
                ViceCmdGetChannelStatusResponse_t ViceCmdGetChannelStatusResponse;
                ViceCmdGetDeviceStatusResponse_t  ViceCmdGetDeviceStatusResponse;

} HostCommand_t;



/* ==========================================================*/
/* ------------------  MEMORY FOOTPRINTS ------------------- */

/* TODO: Might actually be generated in another file during the
         build process
 */

#define PREPROCESSOR_MAX_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_PROGRESSIVE           12
#define PREPROCESSOR_MAX_NUMBER_OF_DECIMATED_PICTURE_BUFF_PROGRESSIVE                      14

#define PREPROCESSOR_MAX_NUMBER_OF_ORIGINAL_PICTURE_BUFF_LUMA_CHROMA_INTERLACE                 (8)*2
#define PREPROCESSOR_MAX_NUMBER_OF_ORIGINAL_PICTURE_BUFF_SHIFTED_CHROMA_INTERLACE              (8)
#define PREPROCESSOR_MAX_NUMBER_OF_DECIMATED_PICTURE_BUFF_INTERLACE                            (10)*2

/* ==========================================================*/
/* ------------------  BUILD TIME CHECKS ------------------- */


#ifndef COMPILE_TIME_ASSERT
/* compile time assertion, can be used to verify sizeof() and offsetof()
 * for example:
 *
 * COMPILE_TIME_ASSERT(sizeof(int32_t) >= 32)
 *
 */
#define COMPILE_TIME_ASSERT(cond)  \
    typedef char VCE_UNIQUE_NAME[(cond)?1:-1]
#define VCE_UNIQUE_NAME         VCE_MAKE_NAME(__LINE__)
#define VCE_MAKE_NAME(line)     VCE_MAKE_NAME2(line)
#define VCE_MAKE_NAME2(line)    compile_time_assert_ ## line

#endif /* COMPILE_TIME_ASSERT */


/* Verify that the API structures don't get bigger than the command buffer size,
 * this will not impact the footprint. */
COMPILE_TIME_ASSERT(sizeof(ViceCmdInit_t)                      <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdInitResponse_t)              <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdOpenChannel_t)               <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdOpenChannelResponse_t)       <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdStartChannel_t)              <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdStartChannelResponse_t)      <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdStopChannel_t)               <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdStopChannelResponse_t)       <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdCloseChannel_t)              <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdCloseChannelResponse_t)      <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdConfigChannel_t)             <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdConfigChannelResponse_t)     <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdDebugChannel_t)              <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdDebugChannelResponse_t)      <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdGetChannelStatus_t)          <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdGetChannelStatusResponse_t)  <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdGetDeviceStatus_t)           <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT(sizeof(ViceCmdGetDeviceStatusResponse_t)   <= HOST_CMD_BUFFER_SIZE);
COMPILE_TIME_ASSERT((BVCE_FW_P_UserData_PacketDescriptor_MAX_LENGTH & 0x3) == 0);              /* NS, make sure that it's a DWORD multiple */


#ifdef __cplusplus
}
#endif


#endif /* BVCE_FW_API_V3_H__ */
