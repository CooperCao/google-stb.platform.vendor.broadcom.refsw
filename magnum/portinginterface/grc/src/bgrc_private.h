/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#ifndef BGRC_PRIVATE_H__
#define BGRC_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bgrc_packet.h"
#include "bgrc_packet_priv.h"

/***************************************************************************/
BDBG_OBJECT_ID_DECLARE(BGRC);

typedef enum
{
    BGRC_eM2mc0 = 0,
    BGRC_eM2mc1 = 1,
    BGRC_eMM_M2mc0 = 2,
    BGRC_eM2mcMax
}BGRC_eM2mcId;
typedef struct BGRC_P_Handle
{
    BDBG_OBJECT(BGRC)
    BCHP_Handle hChip;                          /* handle to chip module */
    BREG_Handle hRegister;                      /* handle to register module */
    BMMA_Heap_Handle hMemory;                   /* handle to memory module */
    BINT_Handle hInterrupt;                     /* handle to interrupt module */

    BGRC_eM2mcId eDeviceNum;                    /* number of M2MC device being used */

    /* Core & Vnet Channel Reset */
    uint32_t                                    ulRegCoreReset;
    uint32_t                                    ulRegClearCore;
    uint32_t                                    ulCoreResetMask;
    uint32_t                                    ulWeightReg;
    uint32_t                                    ulM2mcPwrId;
    uint32_t                                    ulSramPwrId;
    uint32_t                                    ulIntid;


    uint32_t ulPacketMemoryMax;                 /* max packet memory */
    uint32_t ulOperationMax;                    /* max operations */
    uint32_t ulWaitTimeout;                     /* seconds to wait before assuming device is hung */
    uint32_t ulRegOffset;                       /* offset from m2mc0*/
#if BGRC_P_MULTI_CONTEXT_SCHEDULER_SUPPORT
    uint32_t ulWeight;                          /* scheduling */
#endif
    bool secure;

    BGRC_StandbySettings  stStandbySettings;

    BINT_CallbackHandle hCallback;
    BGRC_Callback callback_isr;
    void *callback_data;

    BLST_D_HEAD(context_list, BGRC_P_PacketContext) context_list;
    BGRC_PacketContext_Handle hContext; /* current context */
    BGRC_PacketContext_Handle hLastSyncCtx; /* last context that did sync */
    BGRC_PacketContext_Handle hDummyCtx; /* dummy context for extra flush blit */
    uint32_t  ulNumCreates;
    bool waitForSync;

#if BGRC_PACKET_P_BLIT_WORKAROUND
    /* List status to hardware. */
    BGRC_P_ListStatus             eListStatus;
#endif

    BMMA_Block_Handle pHwPktFifoBaseAlloc;
    uint8_t  *pHwPktFifoBase;
    BSTD_DeviceOffset ulHwPktFifoBaseOffset;
    uint32_t  ulHwPktFifoSize;

    uint8_t  *pHwPktWritePtr;
    uint8_t  *pHwPktPrevWritePtr;
    uint8_t  *pHwPktSubmitLinkPtr;

    uint8_t  *pLastHwPktPtr;      /* NULL means no blit in hw pkt fifo */
    BSTD_DeviceOffset ulHwPktOffsetExecuted;

    BMMA_Block_Handle pDummySurAlloc;
    BSTD_DeviceOffset ulDummySurOffset;
    uint8_t  *pDummySurBase;
    uint32_t  ulSyncCntr;
    uint32_t  ulExtraFlushCntr;

#if BGRC_P_CHECK_RE_ENTRY
    int       iGrcLock;  /* to check module re-entry */
#endif

#ifdef BCHP_MM_M2MC0_REG_START
    /* memory related info but not in stMemInfo for mipmap only*/
    BPXL_Uif_Memory_Info  stPxlMemoryInfo;
    /* How many UIF-block rows the "page cache" covers */
#endif
    BCHP_MemoryInfo stMemInfo;
}

BGRC_P_Handle;

#if BGRC_P_CHECK_RE_ENTRY
#define BGRC_P_ENTER(grc) \
    BDBG_ASSERT(0==hGrc->iGrcLock); hGrc->iGrcLock += 1; \
    BDBG_ASSERT(1==hGrc->iGrcLock); hGrc->iGrcLock += 1;
#define BGRC_P_LEAVE(grc) \
    BDBG_ASSERT(2==hGrc->iGrcLock); hGrc->iGrcLock -= 1; \
    BDBG_ASSERT(1==hGrc->iGrcLock); hGrc->iGrcLock -= 1;
#define BGRC_P_INSIDE(grc) \
    (0!=hGrc->iGrcLock)
#define BGRC_P_BACK_OUT(grc) \
    bool bInsideGrc = BGRC_P_INSIDE(grc); \
    if (bInsideGrc) \
    { \
        BGRC_P_LEAVE(hGrc); \
    }
#define BGRC_P_RE_ENTER(grc) \
    if (bInsideGrc) \
    { \
        BGRC_P_ENTER(hGrc); \
    }

#else
#define BGRC_P_ENTER(grc)
#define BGRC_P_LEAVE(grc)
#define BGRC_P_INSIDE(grc) (false)
#define BGRC_P_BACK_OUT(grc)
#define BGRC_P_RE_ENTER(grc)
#endif

/***************************************************************************/
#define BCHP_M2MC_LIST_PACKET_HEADER_1         BCHP_M2MC_SCRATCH_LIST

/***************************************************************************/
#ifdef BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1                            BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_FORMAT_TYPE_MASK           BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_FORMAT_TYPE_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_FORMAT_TYPE_SHIFT          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_FORMAT_TYPE_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_CH0_NUM_BITS_MASK          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_CH0_NUM_BITS_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_CH1_NUM_BITS_MASK          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_CH1_NUM_BITS_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_CH2_NUM_BITS_MASK          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_CH2_NUM_BITS_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_CH3_NUM_BITS_MASK          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_CH3_NUM_BITS_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_CH0_NUM_BITS_SHIFT         BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_CH0_NUM_BITS_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_CH1_NUM_BITS_SHIFT         BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_CH1_NUM_BITS_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_CH2_NUM_BITS_SHIFT         BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_CH2_NUM_BITS_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_1_CH3_NUM_BITS_SHIFT         BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_1_CH3_NUM_BITS_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_2                            BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_2
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_2_CH0_LSB_POS_MASK           BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_2_CH0_LSB_POS_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_2_CH1_LSB_POS_MASK           BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_2_CH1_LSB_POS_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_2_CH2_LSB_POS_MASK           BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_2_CH2_LSB_POS_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_2_CH3_LSB_POS_MASK           BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_2_CH3_LSB_POS_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_2_CH0_LSB_POS_SHIFT          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_2_CH0_LSB_POS_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_2_CH1_LSB_POS_SHIFT          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_2_CH1_LSB_POS_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_2_CH2_LSB_POS_SHIFT          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_2_CH2_LSB_POS_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_2_CH3_LSB_POS_SHIFT          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_2_CH3_LSB_POS_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3                            BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CH0_DISABLE_MASK           BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CH0_DISABLE_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CH1_DISABLE_MASK           BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CH1_DISABLE_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CH2_DISABLE_MASK           BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CH2_DISABLE_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CH3_DISABLE_MASK           BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CH3_DISABLE_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CH0_DISABLE_SHIFT          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CH0_DISABLE_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CH1_DISABLE_SHIFT          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CH1_DISABLE_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CH2_DISABLE_SHIFT          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CH2_DISABLE_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CH3_DISABLE_SHIFT          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CH3_DISABLE_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_PALETTE_BYPASS_MASK        BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_PALETTE_BYPASS_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_PALETTE_BYPASS_SHIFT       BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_PALETTE_BYPASS_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_PALETTE_BYPASS_DONT_LOOKUP BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_PALETTE_BYPASS_DONT_LOOKUP
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_PALETTE_BYPASS_LOOKUP      BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_PALETTE_BYPASS_LOOKUP
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_MASK         BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CHROMA_FILTER_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_REPLICATE    BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CHROMA_FILTER_REPLICATE
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_FILTER       BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CHROMA_FILTER_FILTER
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_SHIFT        BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_CHROMA_FILTER_SHIFT
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_ZERO_PAD_MASK              BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_ZERO_PAD_MASK
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_ZERO_PAD_ZERO_PAD          BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_ZERO_PAD_ZERO_PAD
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_ZERO_PAD_REPLICATE         BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_ZERO_PAD_REPLICATE
#define BCHP_M2MC_SRC_SURFACE_FORMAT_DEF_3_ZERO_PAD_SHIFT             BCHP_M2MC_SRC_SURFACE_0_FORMAT_DEF_3_ZERO_PAD_SHIFT
#define BCHP_M2MC_BLIT_SRC_TOP_LEFT                                   BCHP_M2MC_BLIT_SRC_TOP_LEFT_0
#define BCHP_M2MC_BLIT_SRC_TOP_LEFT_LEFT_MASK                         BCHP_M2MC_BLIT_SRC_TOP_LEFT_0_LEFT_MASK
#define BCHP_M2MC_BLIT_SRC_TOP_LEFT_LEFT_SHIFT                        BCHP_M2MC_BLIT_SRC_TOP_LEFT_0_LEFT_SHIFT
#define BCHP_M2MC_BLIT_SRC_TOP_LEFT_TOP_MASK                          BCHP_M2MC_BLIT_SRC_TOP_LEFT_0_TOP_MASK
#define BCHP_M2MC_BLIT_SRC_TOP_LEFT_TOP_SHIFT                         BCHP_M2MC_BLIT_SRC_TOP_LEFT_0_TOP_SHIFT
#define BCHP_M2MC_BLIT_SRC_SIZE                                       BCHP_M2MC_BLIT_SRC_SIZE_0
#define BCHP_M2MC_BLIT_SRC_SIZE_SURFACE_WIDTH_MASK                    BCHP_M2MC_BLIT_SRC_SIZE_0_SURFACE_WIDTH_MASK
#define BCHP_M2MC_BLIT_SRC_SIZE_SURFACE_WIDTH_SHIFT                   BCHP_M2MC_BLIT_SRC_SIZE_0_SURFACE_WIDTH_SHIFT
#define BCHP_M2MC_BLIT_SRC_SIZE_SURFACE_HEIGHT_MASK                   BCHP_M2MC_BLIT_SRC_SIZE_0_SURFACE_HEIGHT_MASK
#define BCHP_M2MC_BLIT_SRC_SIZE_SURFACE_HEIGHT_SHIFT                  BCHP_M2MC_BLIT_SRC_SIZE_0_SURFACE_HEIGHT_SHIFT
#define BCHP_M2MC_BLIT_INPUT_STRIPE_WIDTH                             BCHP_M2MC_BLIT_INPUT_STRIPE_WIDTH_0
#define BCHP_M2MC_BLIT_INPUT_STRIPE_WIDTH_STRIPE_WIDTH_MASK           BCHP_M2MC_BLIT_INPUT_STRIPE_WIDTH_0_STRIPE_WIDTH_MASK
#define BCHP_M2MC_BLIT_INPUT_STRIPE_WIDTH_STRIPE_WIDTH_SHIFT          BCHP_M2MC_BLIT_INPUT_STRIPE_WIDTH_0_STRIPE_WIDTH_SHIFT
#define BCHP_M2MC_BLIT_STRIPE_OVERLAP                                 BCHP_M2MC_BLIT_STRIPE_OVERLAP_0
#define BCHP_M2MC_BLIT_STRIPE_OVERLAP_STRIPE_WIDTH_MASK               BCHP_M2MC_BLIT_STRIPE_OVERLAP_0_STRIPE_WIDTH_MASK
#define BCHP_M2MC_BLIT_STRIPE_OVERLAP_STRIPE_WIDTH_SHIFT              BCHP_M2MC_BLIT_STRIPE_OVERLAP_0_STRIPE_WIDTH_SHIFT
#define BCHP_M2MC_HORIZ_SCALER_INITIAL_PHASE                          BCHP_M2MC_HORIZ_SCALER_0_INITIAL_PHASE
#define BCHP_M2MC_HORIZ_SCALER_INITIAL_PHASE_PHASE_MASK               BCHP_M2MC_HORIZ_SCALER_0_INITIAL_PHASE_PHASE_MASK
#define BCHP_M2MC_HORIZ_SCALER_INITIAL_PHASE_PHASE_SHIFT              BCHP_M2MC_HORIZ_SCALER_0_INITIAL_PHASE_PHASE_SHIFT
#define BCHP_M2MC_HORIZ_SCALER_STEP                                   BCHP_M2MC_HORIZ_SCALER_0_STEP
#define BCHP_M2MC_HORIZ_SCALER_STEP_STEP_MASK                         BCHP_M2MC_HORIZ_SCALER_0_STEP_STEP_MASK
#define BCHP_M2MC_HORIZ_SCALER_STEP_STEP_SHIFT                        BCHP_M2MC_HORIZ_SCALER_0_STEP_STEP_SHIFT
#define BCHP_M2MC_VERT_SCALER_INITIAL_PHASE                           BCHP_M2MC_VERT_SCALER_0_INITIAL_PHASE
#define BCHP_M2MC_VERT_SCALER_INITIAL_PHASE_PHASE_MASK                BCHP_M2MC_VERT_SCALER_0_INITIAL_PHASE_PHASE_MASK
#define BCHP_M2MC_VERT_SCALER_INITIAL_PHASE_PHASE_SHIFT               BCHP_M2MC_VERT_SCALER_0_INITIAL_PHASE_PHASE_SHIFT
#define BCHP_M2MC_VERT_SCALER_STEP                                    BCHP_M2MC_VERT_SCALER_0_STEP
#define BCHP_M2MC_VERT_SCALER_STEP_STEP_MASK                          BCHP_M2MC_VERT_SCALER_0_STEP_STEP_MASK
#define BCHP_M2MC_VERT_SCALER_STEP_STEP_SHIFT                         BCHP_M2MC_VERT_SCALER_0_STEP_STEP_SHIFT
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE0_01_COEFF_0_MASK              BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE0_01_COEFF_0_MASK
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE0_01_COEFF_1_MASK              BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE0_01_COEFF_1_MASK
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE0_2_COEFF_2_MASK               BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE0_2_COEFF_2_MASK
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE1_01_COEFF_0_MASK              BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE1_01_COEFF_0_MASK
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE1_01_COEFF_1_MASK              BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE1_01_COEFF_1_MASK
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE1_2_COEFF_2_MASK               BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE1_2_COEFF_2_MASK
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE0_01_COEFF_0_SHIFT             BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE0_01_COEFF_0_SHIFT
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE0_01_COEFF_1_SHIFT             BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE0_01_COEFF_1_SHIFT
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE0_2_COEFF_2_SHIFT              BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE0_2_COEFF_2_SHIFT
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE1_01_COEFF_0_SHIFT             BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE1_01_COEFF_0_SHIFT
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE1_01_COEFF_1_SHIFT             BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE1_01_COEFF_1_SHIFT
#define BCHP_M2MC_HORIZ_FIR_COEFF_PHASE1_2_COEFF_2_SHIFT              BCHP_M2MC_HORIZ_FIR_0_COEFF_PHASE1_2_COEFF_2_SHIFT

#define BCHP_M2MC_1_SRC_SURFACE_FORMAT_DEF_1                          BCHP_M2MC_1_SRC_SURFACE_0_FORMAT_DEF_1
#define BCHP_M2MC_1_SRC_SURFACE_FORMAT_DEF_2                          BCHP_M2MC_1_SRC_SURFACE_0_FORMAT_DEF_2
#define BCHP_M2MC_1_SRC_SURFACE_FORMAT_DEF_3                          BCHP_M2MC_1_SRC_SURFACE_0_FORMAT_DEF_3
#define BCHP_M2MC_1_BLIT_SRC_TOP_LEFT                                 BCHP_M2MC_1_BLIT_SRC_TOP_LEFT_0
#define BCHP_M2MC_1_BLIT_SRC_SIZE                                     BCHP_M2MC_1_BLIT_SRC_SIZE_0
#define BCHP_M2MC_1_BLIT_INPUT_STRIPE_WIDTH                           BCHP_M2MC_1_BLIT_INPUT_STRIPE_WIDTH_0
#define BCHP_M2MC_1_BLIT_STRIPE_OVERLAP                               BCHP_M2MC_1_BLIT_STRIPE_OVERLAP_0
#define BCHP_M2MC_1_HORIZ_SCALER_INITIAL_PHASE                        BCHP_M2MC_1_HORIZ_SCALER_0_INITIAL_PHASE
#define BCHP_M2MC_1_HORIZ_SCALER_STEP                                 BCHP_M2MC_1_HORIZ_SCALER_0_STEP
#define BCHP_M2MC_1_VERT_SCALER_INITIAL_PHASE                         BCHP_M2MC_1_VERT_SCALER_0_INITIAL_PHASE
#define BCHP_M2MC_1_VERT_SCALER_STEP                                  BCHP_M2MC_1_VERT_SCALER_0_STEP

#define BCHP_M2MC1_SRC_SURFACE_FORMAT_DEF_1                            BCHP_M2MC1_SRC_SURFACE_0_FORMAT_DEF_2
#define BCHP_M2MC1_SRC_SURFACE_FORMAT_DEF_2                            BCHP_M2MC1_SRC_SURFACE_0_FORMAT_DEF_1
#define BCHP_M2MC1_SRC_SURFACE_FORMAT_DEF_3                            BCHP_M2MC1_SRC_SURFACE_0_FORMAT_DEF_3
#define BCHP_M2MC1_BLIT_SRC_TOP_LEFT                                   BCHP_M2MC1_BLIT_SRC_TOP_LEFT_0
#define BCHP_M2MC1_BLIT_SRC_SIZE                                       BCHP_M2MC1_BLIT_SRC_SIZE_0
#define BCHP_M2MC1_BLIT_INPUT_STRIPE_WIDTH                             BCHP_M2MC1_BLIT_INPUT_STRIPE_WIDTH_0
#define BCHP_M2MC1_BLIT_STRIPE_OVERLAP                                 BCHP_M2MC1_BLIT_STRIPE_OVERLAP_0
#define BCHP_M2MC1_HORIZ_SCALER_INITIAL_PHASE                          BCHP_M2MC1_HORIZ_SCALER_0_INITIAL_PHASE
#define BCHP_M2MC1_HORIZ_SCALER_STEP                                   BCHP_M2MC1_HORIZ_SCALER_0_STEP
#define BCHP_M2MC1_VERT_SCALER_INITIAL_PHASE                           BCHP_M2MC1_VERT_SCALER_0_INITIAL_PHASE
#define BCHP_M2MC1_VERT_SCALER_STEP                                    BCHP_M2MC1_VERT_SCALER_0_STEP
#endif

#define BGRC_P_SCALE_DOWN_MAX_X          15
#define BGRC_P_SCALE_DOWN_MAX_Y          15
#define BGRC_P_SCALE_DOWN_MAX            BGRC_P_MIN(BGRC_P_SCALE_DOWN_MAX_X, BGRC_P_SCALE_DOWN_MAX_Y)

#define BGRC_P_MIN( v0, v1 )        (((v0) < (v1)) ? (v0) : (v1))

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGRC_PRIVATE_H__ */

/* end of file */
