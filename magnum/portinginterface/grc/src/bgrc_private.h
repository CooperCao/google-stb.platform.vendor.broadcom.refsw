/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

/***************************************************************************/
#ifdef BCHP_M2MC_OUTPUT_CM_C34
#define BGRC_P_REG_COUNT                                    (((BCHP_M2MC_OUTPUT_CM_C34 - BCHP_M2MC_SCRATCH_LIST) >> 2) + 1)
#else
#define BGRC_P_REG_COUNT                                    (((BCHP_M2MC_SRC_CM_C34 - BCHP_M2MC_SCRATCH_LIST) >> 2) + 1)
#endif

#define BGRC_P_GROUP_COUNT                                  15
#define BGRC_P_HEADER_COUNT                                 2
#define BGRC_P_USERDATA_COUNT                               3

#define BGRC_P_OPERATION_MIN                                128
#define BGRC_P_OPERATION_MAX                                256

#define BGRC_P_LIST_BLOCK_ALIGN                             5
#define BGRC_P_LIST_BLOCK_SIZE                              2048
#define BGRC_P_LIST_BLOCK_MIN_SIZE                          16384

/***************************************************************************/
#if defined(BCHP_M2MC_SRC_SURFACE_1_FORMAT_DEF_1)
#define BGRC_P_LIST_SRC_FEEDER_GRP_CNTRL_COUNT_TMP          13
#if defined(BCHP_M2MC_DCEG_CFG)
#define BGRC_P_LIST_BLIT_GRP_CNTRL_COUNT                    18
#else
#define BGRC_P_LIST_BLIT_GRP_CNTRL_COUNT                    17
#endif
#define BGRC_P_LIST_SCALE_PARAM_GRP_CNTRL_COUNT             13
#else
#define BGRC_P_LIST_SRC_FEEDER_GRP_CNTRL_COUNT_TMP          10
#define BGRC_P_LIST_BLIT_GRP_CNTRL_COUNT                    11
#define BGRC_P_LIST_SCALE_PARAM_GRP_CNTRL_COUNT             9
#endif

#define BGRC_P_LIST_DST_FEEDER_GRP_CNTRL_COUNT_TMP          10
#define BGRC_P_LIST_OUTPUT_FEEDER_GRP_CNTRL_COUNT_TMP       8
#define BGRC_P_LIST_BLEND_PARAM_GRP_CNTRL_COUNT             4
#define BGRC_P_LIST_ROP_GRP_CNTRL_COUNT                     5
#define BGRC_P_LIST_SRC_COLOR_KEY_GRP_CNTRL_COUNT           5
#define BGRC_P_LIST_DST_COLOR_KEY_GRP_CNTRL_COUNT           5

#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_MSB
#ifdef BCHP_M2MC_SRC_SURFACE_ADDR_0_BOT_FLD_MSB
#define BGRC_P_LIST_SRC_FEEDER_GRP_CNTRL_COUNT              (BGRC_P_LIST_SRC_FEEDER_GRP_CNTRL_COUNT_TMP+2+4)
#else
#define BGRC_P_LIST_SRC_FEEDER_GRP_CNTRL_COUNT              (BGRC_P_LIST_SRC_FEEDER_GRP_CNTRL_COUNT_TMP+2)
#endif
#define BGRC_P_LIST_DST_FEEDER_GRP_CNTRL_COUNT              (BGRC_P_LIST_DST_FEEDER_GRP_CNTRL_COUNT_TMP+2)
#define BGRC_P_LIST_OUTPUT_FEEDER_GRP_CNTRL_COUNT           (BGRC_P_LIST_OUTPUT_FEEDER_GRP_CNTRL_COUNT_TMP+2)
#else
#define BGRC_P_LIST_SRC_FEEDER_GRP_CNTRL_COUNT              (BGRC_P_LIST_SRC_FEEDER_GRP_CNTRL_COUNT_TMP)
#define BGRC_P_LIST_DST_FEEDER_GRP_CNTRL_COUNT              (BGRC_P_LIST_DST_FEEDER_GRP_CNTRL_COUNT_TMP)
#define BGRC_P_LIST_OUTPUT_FEEDER_GRP_CNTRL_COUNT           (BGRC_P_LIST_OUTPUT_FEEDER_GRP_CNTRL_COUNT_TMP)
#endif

#if defined(BCHP_M2MC_HORIZ_FIR_COEFF_PHASE0_67)
#define BGRC_P_LIST_SCALE_COEF_GRP_CNTRL_COUNT              64
#else
#define BGRC_P_LIST_SCALE_COEF_GRP_CNTRL_COUNT              16
#endif

#define BGRC_P_LIST_SRC_COLOR_MATRIX_GRP_CNTRL_COUNT        12
#define BGRC_P_LIST_DST_COLOR_MATRIX_GRP_CNTRL_COUNT        12
#define BGRC_P_LIST_OUTPUT_COLOR_MATRIX_GRP_CNTRL_COUNT     12
#define BGRC_P_LIST_SRC_CLUT_GRP_CNTRL_COUNT                1
#define BGRC_P_LIST_DST_CLUT_GRP_CNTRL_COUNT                1

#define BGRC_P_MACROBLOCK_RANGE_NONE                        8
#define BGRC_P_MACROBLOCK_RANGE_REMAPPING                   9
#define BGRC_P_MACROBLOCK_RANGE_EXPANSION                   16

/***************************************************************************/
typedef struct
{
    uint32_t ulX;
    uint32_t ulY;
    uint32_t ulWidth;
    uint32_t ulHeight;
}
BGRC_P_Rect;

/***************************************************************************/
typedef struct BGRC_P_Block BGRC_P_Block;
struct BGRC_P_Block
{
    BGRC_P_Block *pNextBlock;           /* pointer to next block */
    void *pvMemory;                     /* pointer to device memory */
    void *pvCached;                     /* pointer to cached device memory */
    uint32_t ulOffset;                  /* device memory offset */
    uint32_t ulRefCount;
    bool bBusy;                         /* indicates if block is busy */
};

/***************************************************************************/
typedef struct BGRC_P_Operation BGRC_P_Operation;
struct BGRC_P_Operation
{
    BGRC_P_Operation *pPrevOp;
    BGRC_P_Operation *pNextOp;
    BGRC_P_Block *pBlock;
    BGRC_Callback pUserCallback;
    void *pUserData;
    uint32_t ulPacketOffset;
    uint32_t ulPacketSize;
    bool bSetEvent;
    bool bSetPeriodicEvent;
    bool bActive;
};

/***************************************************************************/
typedef struct
{
    BSUR_Surface_Handle hSurface;
    BPXL_Format eFormat;
    void *pMemory;
    uint32_t ulOffset;
    uint32_t ulPitch;
    uint32_t ulPaletteOffset;
    uint32_t ulPaletteEntries;
    uint32_t ulSurfaceWidth;
    uint32_t ulSurfaceHeight;
    uint32_t ulX;
    uint32_t ulY;
    uint32_t ulWidth;
    uint32_t ulHeight;
    uint32_t ulID;
}
BGRC_P_Surface;

/***************************************************************************/
typedef struct
{
    BGRC_P_Surface SrcSurface;               /* source surface structure */
    BGRC_P_Surface DstSurface;               /* destination surface structure */
    BGRC_P_Surface OutSurface;               /* output surface structure */
    BGRC_P_Surface SrcAlphaSurface;          /* source alpha surface structure */
    BGRC_P_Surface DstAlphaSurface;          /* destination alpha surface structure */
    BGRC_P_Surface OutAlphaSurface;          /* output alpha surface structure */
    BGRC_P_Rect SrcRect;                     /* source rectangle */
    BGRC_P_Rect DstRect;                     /* destination rectangle */
    BGRC_P_Rect OutRect;                     /* output rectangle */
    BGRC_FilterCoeffs eHorzCoeffs;           /* horizontal scaler coefficient */
    BGRC_FilterCoeffs eVertCoeffs;           /* vertical scaler coefficient */
    const uint32_t *pulHorzFirCoeffs;        /* pointer to horz fir coefficients */
    const uint32_t *pulVertFirCoeffs;        /* pointer to vert fir coefficients */
    uint32_t ulHorzScalerStep;               /* horizontal scaler step */
    uint32_t ulVertScalerStep;               /* vertical scaler step */
    uint32_t ulHorzInitPhase;                /* horizontal initial phase */
    uint32_t ulVertInitPhase;                /* vertical initial phase */
    uint32_t ulHorzScalerNum;                /* horizontal scale factor numerator */
    uint32_t ulHorzScalerDen;                /* horizontal scale factor denominator */
    uint32_t ulVertScalerNum;                /* vertical scale factor numerator */
    uint32_t ulVertScalerDen;                /* vertical scale factor denominator */
    uint32_t ulHorzAveragerCount;            /* horizontal averager count */
    uint32_t ulVertAveragerCount;            /* vertical averager count */
    uint32_t ulHorzAveragerCoeff;            /* horizontal averager coefficient */
    uint32_t ulVertAveragerCoeff;            /* vertical averager coefficient */
    uint32_t ulSrcStripWidth;                /* src strip width for scaling*/
    uint32_t ulOutStripWidth;                /* out strip width for scaling */
    uint32_t ulOverlapStrip;                 /* overlap strip for scaling */
    uint32_t ulPhaseShift;                   /* Phase shift value */
    int32_t iHorzPhaseAdj;                   /* horizontal initial phase ajustment */
    int32_t iVertPhaseAdj;                   /* vertical initial phase ajustment */
    uint8_t aucPattern[8];                   /* 8x8 bit ROP pattern */
    bool bHorzFilter;                        /* enables horizontal filter */
    bool bVertFilter;                        /* enables vertical filter */
    bool bSrcPaletteBypass;                  /* enables bypassing src palette */
    bool bDstPaletteBypass;                  /* enables bypassing dst palette */
    bool bSrcRightToLeft;
    bool bSrcBottomToTop;
    bool bDstRightToLeft;
    bool bDstBottomToTop;
    bool bOutRightToLeft;
    bool bOutBottomToTop;
    uint32_t ulMacroBlockRangeY;
    uint32_t ulMacroBlockRangeC;
    uint32_t ulMacroBlockStripWidth;
    bool bMacroBlockLinear;
    bool bMacroBlockBigEndian;
}
BGRC_P_State;

/***************************************************************************/
BDBG_OBJECT_ID_DECLARE(BGRC);
typedef struct BGRC_P_Handle
{
    BDBG_OBJECT(BGRC)
    BCHP_Handle hChip;                          /* handle to chip module */
    BREG_Handle hRegister;                      /* handle to register module */
    BMEM_Handle hMemory;                        /* handle to memory module */
    BINT_Handle hInterrupt;                     /* handle to interrupt module */

    uint32_t ulDeviceNum;                       /* number of M2MC device being used */
    uint32_t ulPacketMemoryMax;                 /* max packet memory */
    uint32_t ulOperationMax;                    /* max operations */
    uint32_t ulWaitTimeout;                     /* seconds to wait before assuming device is hung */
    bool secure;

    BGRC_StandbySettings  stStandbySettings;

#ifdef BGRC_NON_PACKET_MODE
    BINT_CallbackHandle hInterruptCallback;     /* handle to interrupt callback */
    BKNI_EventHandle hInterruptEvent;           /* handle to interrupt event */
    BKNI_EventHandle hPeriodicEvent;            /* handle to periodic event */
    uint32_t aulCurrentRegs[BGRC_P_REG_COUNT];  /* array of current m2mc registers */
    uint32_t aulDefaultRegs[BGRC_P_REG_COUNT];  /* array of default m2mc registers */
    uint32_t aulActualRegs[BGRC_P_REG_COUNT];   /* array of actual m2mc registers */
    uint32_t aulStoredRegs[BGRC_P_REG_COUNT];   /* array of stored m2mc registers */
    BGRC_P_State CurrentState;                  /* current state information */
    BGRC_P_State DefaultState;                  /* default state information */
    BGRC_P_State StoredState;                   /* stored state information */
    uint32_t ulSurfaceID;                       /* current surface id */
    uint32_t ulPeriodicInterrupts;              /* count of pending periodic interrupts */
    bool bNoScaleFilter;                        /* indicates filtering without scaling */
    bool bYCbCr420Source;                       /* source is YCbCr420 */
    bool bUninitialized;                        /* indicates if module is inited */
    bool bSetEvent;                             /* indicates if isr should set event */
    bool bPeriodicInterrupt;                    /* indicates if interrupt should be fired */

    BGRC_P_Block *pCurrListBlock;               /* pointer to current list memory block */
    BGRC_P_Block *pPrevListBlock;               /* pointer to current list memory block */
    uint32_t ulListBlockPos;                    /* position within current list memory block */
    BGRC_Callback pPrevUserCallback;            /* pointer to previous user interrupt callback */
    uint32_t *pulPrevPacket;                    /* pointer to previous list packet */
    uint32_t ulPacketMemorySize;                /* amount of memory allocated for packets */
    uint32_t ulOperationCount;                  /* count allocated operations */
    uint32_t ulIntExpected;                     /* number of interrupts expected */
    uint32_t ulIntReceived;                     /* number of interrupts received */
    uint32_t ulPacketMemorySinceInterrupt;      /* packet memory allocated since periodic interrupt */
    bool bPreAllocMemory;                       /* allocate memory when opening module */

    BGRC_P_Operation *pCurrOp;
    BGRC_P_Operation *pLastOp;
    BGRC_P_Operation *pFreeOp;

    BSUR_Surface_Handle hWaitSurface;
    BGRC_Callback pPeriodicCallback;
    void *pPeriodicData;

#else
    /* packets */
    BINT_CallbackHandle hCallback;
    BGRC_Callback callback_isr;
    void *callback_data;

    BLST_D_HEAD(context_list, BGRC_P_PacketContext) context_list;
    BGRC_PacketContext_Handle hContext; /* current context */
    BGRC_PacketContext_Handle hLastSyncCtx; /* last context that did sync */
    BGRC_PacketContext_Handle hDummyCtx; /* dummy context for extra flush blit */
    uint32_t  ulNumCreates;
    bool waitForSync;

    void     *pHwPktFifoBaseAlloc;
    uint8_t  *pHwPktFifoBase;
    uint32_t  ulHwPktFifoBaseOffset;
    uint32_t  ulHwPktFifoSize;

    uint8_t  *pHwPktWritePtr;
    uint8_t  *pHwPktPrevWritePtr;
    uint8_t  *pHwPktSubmitLinkPtr;

    uint8_t  *pLastHwPktPtr;      /* NULL means no blit in hw pkt fifo */
    uint32_t  ulHwPktOffsetExecuted;

    void     *pDummySurAlloc;
    uint32_t  ulDummySurOffset;
    uint8_t  *pDummySurBase;
    uint32_t  ulSyncCntr;
    uint32_t  ulExtraFlushCntr;

#if BGRC_P_CHECK_RE_ENTRY
    int       iGrcLock;  /* to check module re-entry */
#endif
#endif
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
#define BGRC_P_WATCHDOG_CNTR_OFF    0
#define BGRC_P_WATCHDOG_CNTR_FIRE   1
#define BGRC_P_WATCHDOG_CNTR_MAX    6

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

/***************************************************************************/
#if defined(BCHP_M2MC_HORIZ_FIR_COEFF_PHASE0_67)
#define BGRC_P_FIR_PHASE_COUNT           8
#define BGRC_P_FIR_TAP_COUNT             8
#define BGRC_P_FIR_FRAC_BITS             10
#define BGRC_P_FIR_OVERLAP_MIN           4
#else
#define BGRC_P_FIR_PHASE_COUNT           2
#define BGRC_P_FIR_TAP_COUNT             4
#define BGRC_P_FIR_FRAC_BITS             8
#define BGRC_P_FIR_OVERLAP_MIN           3
#endif
#define BGRC_P_FIR_FRAC_SCALE            (1 << BGRC_P_FIR_FRAC_BITS)

#ifdef BCHP_M2MC_HORIZ_SCALER_0_STEP_reserved0_SHIFT
#define BGRC_P_SCALER_STEP_FRAC_BITS     (BCHP_M2MC_HORIZ_SCALER_0_STEP_reserved0_SHIFT - 4)
#else
#define BGRC_P_SCALER_STEP_FRAC_BITS     (BCHP_M2MC_HORIZ_SCALER_STEP_reserved0_SHIFT - 4)
#endif

#define BGRC_P_SCALER_STEP_FRAC_MASK     ((1 << BGRC_P_SCALER_STEP_FRAC_BITS) - 1)

#define BGRC_P_AVERAGER_COEFF_FRAC_BITS  19
#define BGRC_P_AVERAGER_COEFF_FRAC_MASK  ((1 << BGRC_P_AVERAGER_COEFF_FRAC_BITS) - 1)

#if (BCHP_CHIP==7630)
#define BGRC_P_STRIP_WIDTH_MAX           120
#else
#define BGRC_P_STRIP_WIDTH_MAX           128
#endif

#define BGRC_P_MATRIX_FRAC_BITS          10

#define BGRC_P_SCALE_DOWN_MAX_X          15
#define BGRC_P_SCALE_DOWN_MAX_Y          15
#define BGRC_P_SCALE_DOWN_MAX            BGRC_P_MIN(BGRC_P_SCALE_DOWN_MAX_X, BGRC_P_SCALE_DOWN_MAX_Y)

#if (BCHP_M2MC_SRC_CM_C04_CM_C04_MASK & 0xF000)
#define BGRC_P_MATRIX_ADD_FRAC_BITS      4
#else
#define BGRC_P_MATRIX_ADD_FRAC_BITS      0
#endif

#define BGRC_P_YCbCr420_STRIP_WIDTH      64

#define BGRC_P_SCALER_STEP_TO_STRIPE_WIDTH_SHIFT    (BGRC_P_SCALER_STEP_FRAC_BITS - 16)

/***************************************************************************/
#define BGRC_P_MIN( v0, v1 )        (((v0) < (v1)) ? (v0) : (v1))
#define BGRC_P_MAX( v0, v1 )        (((v0) > (v1)) ? (v0) : (v1))
#define BGRC_P_CLAMP( v, mn, mx )   BGRC_P_MIN(BGRC_P_MAX(v, mn), mx)

/***************************************************************************/
#define BGRC_P_REG_INDEX( reg ) ((BCHP_M2MC_##reg - BCHP_M2MC_SCRATCH_LIST) >> 2)

/***************************************************************************/
#define BGRC_P_GET_FIELD_DATA( reg, field ) \
    ((hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] & \
        (BCHP_M2MC_##reg##_##field##_MASK)) >> \
        (BCHP_M2MC_##reg##_##field##_SHIFT))

/***************************************************************************/
#define BGRC_P_COMPARE_FIELD( reg, field, flag ) \
    (((hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] & \
        (BCHP_M2MC_##reg##_##field##_MASK)) >> \
        (BCHP_M2MC_##reg##_##field##_SHIFT)) == \
        (BCHP_M2MC_##reg##_##field##_##flag))

/***************************************************************************/
#define BGRC_P_COMPARE_VALUE( reg, field, value ) \
    (((hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] & \
        (BCHP_M2MC_##reg##_##field##_MASK)) >> \
        (BCHP_M2MC_##reg##_##field##_SHIFT)) == value)

/***************************************************************************/
#define BGRC_P_SET_FIELD_FULL( reg, value ) \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] = ((uint32_t) (value))

/***************************************************************************/
#define BGRC_P_SET_FIELD_DATA( reg, field, value ) \
{ \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] &= \
        (~(BCHP_M2MC_##reg##_##field##_MASK)); \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] |= \
        (((uint32_t) (value)) << \
        (BCHP_M2MC_##reg##_##field##_SHIFT)) & \
        (BCHP_M2MC_##reg##_##field##_MASK); \
}

/***************************************************************************/
#define BGRC_P_SET_FIELD_ENUM( reg, field, flag ) \
{ \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] &= \
        (~(BCHP_M2MC_##reg##_##field##_MASK)); \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] |= \
        (BCHP_M2MC_##reg##_##field##_##flag << \
        (BCHP_M2MC_##reg##_##field##_SHIFT)) & \
        (BCHP_M2MC_##reg##_##field##_MASK); \
}

/***************************************************************************/
#define BGRC_P_SET_FIELD_COMP( reg, field, flag_true, flag_false, comp ) \
{ \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] &= \
        (~(BCHP_M2MC_##reg##_##field##_MASK)); \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] |= (((comp) ? \
        (BCHP_M2MC_##reg##_##field##_##flag_true) : \
        (BCHP_M2MC_##reg##_##field##_##flag_false)) << \
        (BCHP_M2MC_##reg##_##field##_SHIFT)) & \
        (BCHP_M2MC_##reg##_##field##_MASK); \
}

/***************************************************************************/
#define BGRC_P_SET_FIELD_COMP_DATA( reg, field, data_true, data_false, comp ) \
{ \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] &= \
        (~(BCHP_M2MC_##reg##_##field##_MASK)); \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] |= (((comp) ? \
        (data_true) : (data_false)) << \
        (BCHP_M2MC_##reg##_##field##_SHIFT)) & \
        (BCHP_M2MC_##reg##_##field##_MASK); \
}

/***************************************************************************/
#define BGRC_P_LOAD_LIST_GRP( reg, field, flag, pState, pulRegs ) \
{ \
    if( ((pulRegs)[BGRC_P_REG_INDEX(reg)] & BCHP_M2MC_##reg##_##field##_MASK) == 0 ) \
    { \
        (pulRegs)[BGRC_P_REG_INDEX(reg)] &= \
            (~(BCHP_M2MC_##reg##_##field##_MASK)); \
        (pulRegs)[BGRC_P_REG_INDEX(reg)] |= \
            (BCHP_M2MC_##reg##_##field##_##flag << \
            (BCHP_M2MC_##reg##_##field##_SHIFT)) & \
            (BCHP_M2MC_##reg##_##field##_MASK); \
    } \
}

/***************************************************************************/
#define BGRC_P_SET_FIELD_FORMAT( reg, field, format, alpha, bypass ) \
{ \
    if( BPXL_IS_YCbCr444_10BIT_FORMAT(format) ) \
    { \
        /* YCbCr444 10-bit */ \
        BGRC_P_SET_FIELD_DATA( reg##_SURFACE_FORMAT_DEF_1, field, 8 ); \
    } \
    else if( BPXL_IS_YCbCr420_FORMAT(format) ) \
    { \
        /* YCbCr420 */ \
        BGRC_P_SET_FIELD_DATA( reg##_SURFACE_FORMAT_DEF_1, field, pState->bMacroBlockBigEndian ? 0 : 7 ); \
    } \
    else if( BPXL_IS_YCbCr422_10BIT_FORMAT(format) ) \
    { \
        /* YCbCr422 10-bit */ \
        BGRC_P_SET_FIELD_DATA( reg##_SURFACE_FORMAT_DEF_1, field, 6 ); \
    } \
    else if( BPXL_IS_ALPHA_ONLY_FORMAT(format) ) \
    { \
        /* ALPHA */ \
        BGRC_P_SET_FIELD_DATA( reg##_SURFACE_FORMAT_DEF_1, field, 5 ); \
    } \
    else if( BPXL_IS_YCbCr422_FORMAT(format) ) \
    { \
        /* YCbCr422 */ \
        BGRC_P_SET_FIELD_DATA( reg##_SURFACE_FORMAT_DEF_1, field, 4 ); \
    } \
    else if( (!(bypass)) && BPXL_IS_PALETTE_FORMAT(format) ) \
    { \
        /* PALETTE */ \
        BGRC_P_SET_FIELD_DATA( reg##_SURFACE_FORMAT_DEF_1, field, 3 ); \
    } \
    else if( (alpha) && \
        (BPXL_COMPONENT_SIZE(format, 3) == 0) && \
        (BPXL_COMPONENT_SIZE(format, 2) == 5) && \
        (BPXL_COMPONENT_SIZE(format, 1) == 6) && \
        (BPXL_COMPONENT_SIZE(format, 0) == 5) ) \
    { \
        /* WRGB_1565 */ \
        BGRC_P_SET_FIELD_DATA( reg##_SURFACE_FORMAT_DEF_1, field, 2 ); \
    } \
    else if( BPXL_IS_WINDOW_FORMAT(format) || ( \
        (BPXL_COMPONENT_SIZE(format, 3) == 1) && \
        (BPXL_COMPONENT_SIZE(format, 2) == 5) && \
        (BPXL_COMPONENT_SIZE(format, 1) == 5) && \
        (BPXL_COMPONENT_SIZE(format, 0) == 5)) ) \
    { \
        /* WRGB_1555 */ \
        BGRC_P_SET_FIELD_DATA( reg##_SURFACE_FORMAT_DEF_1, field, 1 ); \
    } \
    else \
    { \
        /* OTHER */ \
        BGRC_P_SET_FIELD_DATA( reg##_SURFACE_FORMAT_DEF_1, field, 0 ); \
    } \
}

/***************************************************************************/
#define BGRC_P_SET_FIELD_CHANNELS( reg, format, surface ) \
{ \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_SURFACE_FORMAT_DEF_1)] &= ~( \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH0_NUM_BITS_MASK | \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH1_NUM_BITS_MASK | \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH2_NUM_BITS_MASK | \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH3_NUM_BITS_MASK); \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_SURFACE_FORMAT_DEF_2)] &= ~( \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH0_LSB_POS_MASK | \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH1_LSB_POS_MASK | \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH2_LSB_POS_MASK | \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH3_LSB_POS_MASK); \
    if( surface ) \
    { \
        if( BPXL_IS_YCbCr444_10BIT_FORMAT(format) ) \
        { \
            hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_SURFACE_FORMAT_DEF_1)] |= \
                (BPXL_COMPONENT_SIZE(format, 0) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH0_NUM_BITS_SHIFT) | \
                (BPXL_COMPONENT_SIZE(format, 1) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH1_NUM_BITS_SHIFT) | \
                (BPXL_COMPONENT_SIZE(format, 2) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH2_NUM_BITS_SHIFT); \
            hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_SURFACE_FORMAT_DEF_2)] |= \
                (BPXL_COMPONENT_POS(format, 0) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH0_LSB_POS_SHIFT) | \
                (BPXL_COMPONENT_POS(format, 1) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH1_LSB_POS_SHIFT) | \
                (BPXL_COMPONENT_POS(format, 2) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH2_LSB_POS_SHIFT); \
        } \
        else \
        { \
            hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_SURFACE_FORMAT_DEF_1)] |= \
                (BPXL_COMPONENT_SIZE(format, 0) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH0_NUM_BITS_SHIFT) | \
                (BPXL_COMPONENT_SIZE(format, 1) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH1_NUM_BITS_SHIFT) | \
                (BPXL_COMPONENT_SIZE(format, 2) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH2_NUM_BITS_SHIFT) | \
                (BPXL_COMPONENT_SIZE(format, 3) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_1_CH3_NUM_BITS_SHIFT); \
            hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_SURFACE_FORMAT_DEF_2)] |= \
                (BPXL_COMPONENT_POS(format, 0) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH0_LSB_POS_SHIFT) | \
                (BPXL_COMPONENT_POS(format, 1) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH1_LSB_POS_SHIFT) | \
                (BPXL_COMPONENT_POS(format, 2) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH2_LSB_POS_SHIFT) | \
                (BPXL_COMPONENT_POS(format, 3) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_2_CH3_LSB_POS_SHIFT); \
        } \
    } \
}

#define BGRC_P_SET_FIELD_CHANNEL_DISABLE( reg, format, surface ) \
{ \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_SURFACE_FORMAT_DEF_3)] &= ~( \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_3_CH0_DISABLE_MASK | \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_3_CH1_DISABLE_MASK | \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_3_CH2_DISABLE_MASK | \
        BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_3_CH3_DISABLE_MASK); \
    if( surface ) \
    { \
        hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_SURFACE_FORMAT_DEF_3)] |= \
            ((BPXL_HAS_MASKED_ALPHA(format) ? 1 : 0) << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_3_CH3_DISABLE_SHIFT); \
    } \
    else \
    { \
        hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_SURFACE_FORMAT_DEF_3)] |= \
            (1 << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_3_CH0_DISABLE_SHIFT) | \
            (1 << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_3_CH1_DISABLE_SHIFT) | \
            (1 << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_3_CH2_DISABLE_SHIFT) | \
            (1 << BCHP_M2MC_##reg##_SURFACE_FORMAT_DEF_3_CH3_DISABLE_SHIFT); \
    } \
}

/***************************************************************************/
#define BGRC_P_GET_FIELD_MATRIX_ENTRY( entry, left, right ) \
    (((((entry) * (((entry) < 0) ? -1 : 1)) << (left)) >> (right)) * (((entry) < 0) ? -1 : 1))

/***************************************************************************/
#define BGRC_P_SET_FIELD_MATRIX_ROW( reg, row, matrix, index, shift ) \
{ \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_CM_C##row##0_C##row##1)] = \
        BCHP_FIELD_DATA(M2MC_##reg##_CM_C##row##0_C##row##1, CM_C##row##0, \
            BGRC_P_GET_FIELD_MATRIX_ENTRY( (matrix)[(index) + 0], BGRC_P_MATRIX_FRAC_BITS, shift ) & \
            BCHP_MASK(M2MC_##reg##_CM_C##row##0_C##row##1, CM_C##row##1)) | \
        BCHP_FIELD_DATA(M2MC_##reg##_CM_C##row##0_C##row##1, CM_C##row##1, \
            BGRC_P_GET_FIELD_MATRIX_ENTRY( (matrix)[(index) + 1], BGRC_P_MATRIX_FRAC_BITS, shift ) & \
            BCHP_MASK(M2MC_##reg##_CM_C##row##0_C##row##1, CM_C##row##1)); \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_CM_C##row##2_C##row##3)] = \
        BCHP_FIELD_DATA(M2MC_##reg##_CM_C##row##2_C##row##3, CM_C##row##2, \
            BGRC_P_GET_FIELD_MATRIX_ENTRY( (matrix)[(index) + 2], BGRC_P_MATRIX_FRAC_BITS, shift ) & \
            BCHP_MASK(M2MC_##reg##_CM_C##row##2_C##row##3, CM_C##row##3)) | \
        BCHP_FIELD_DATA(M2MC_##reg##_CM_C##row##2_C##row##3, CM_C##row##3, \
            BGRC_P_GET_FIELD_MATRIX_ENTRY( (matrix)[(index) + 3], BGRC_P_MATRIX_FRAC_BITS, shift ) & \
            BCHP_MASK(M2MC_##reg##_CM_C##row##2_C##row##3, CM_C##row##3)); \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg##_CM_C##row##4)] = \
        BCHP_FIELD_DATA(M2MC_##reg##_CM_C##row##4, CM_C##row##4, \
            BGRC_P_GET_FIELD_MATRIX_ENTRY( (matrix)[(index) + 4], BGRC_P_MATRIX_ADD_FRAC_BITS, shift ) & \
            BCHP_MASK(M2MC_##reg##_CM_C##row##4, CM_C##row##4)); \
}

/***************************************************************************/
#define BGRC_P_SET_FIELD_BLEND( reg, srca, srcb, srcc, srcd, srce, subcd, sube ) \
{ \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(BLEND_##reg##_OP)] = \
        (BGRC_P_GetBlendOp( srca ) << BCHP_M2MC_BLEND_##reg##_OP_OP_A_SHIFT) | \
        (BGRC_P_GetBlendOp( srcb ) << BCHP_M2MC_BLEND_##reg##_OP_OP_B_SHIFT) | \
        (BGRC_P_GetBlendOp( srcc ) << BCHP_M2MC_BLEND_##reg##_OP_OP_C_SHIFT) | \
        (BGRC_P_GetBlendOp( srcd ) << BCHP_M2MC_BLEND_##reg##_OP_OP_D_SHIFT) | \
        (BGRC_P_GetBlendOp( srce ) << BCHP_M2MC_BLEND_##reg##_OP_OP_E_SHIFT) | \
        (BGRC_P_GetBlendOpInv( srca ) << BCHP_M2MC_BLEND_##reg##_OP_OP_A_INV_SHIFT) | \
        (BGRC_P_GetBlendOpInv( srcb ) << BCHP_M2MC_BLEND_##reg##_OP_OP_B_INV_SHIFT) | \
        (BGRC_P_GetBlendOpInv( srcc ) << BCHP_M2MC_BLEND_##reg##_OP_OP_C_INV_SHIFT) | \
        (BGRC_P_GetBlendOpInv( srcd ) << BCHP_M2MC_BLEND_##reg##_OP_OP_D_INV_SHIFT) | \
        (BGRC_P_GetBlendOpInv( srce ) << BCHP_M2MC_BLEND_##reg##_OP_OP_E_INV_SHIFT) | \
        (((subcd) ? 1 : 0) << BCHP_M2MC_BLEND_##reg##_OP_SUBTRACT_CD_SHIFT) | \
        (((sube) ? 1 : 0) << BCHP_M2MC_BLEND_##reg##_OP_SUBTRACT_E_SHIFT); \
}

/***************************************************************************/
#define BGRC_P_SET_FIELD_COLORKEY( nsnd, nsd, snd, sd ) \
{ \
    hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(BLEND_COLOR_KEY_ACTION)] = \
        ((nsnd) << BCHP_M2MC_BLEND_COLOR_KEY_ACTION_ACTION_NOT_SRC_NOT_DEST_SHIFT) | \
        ((nsd) << BCHP_M2MC_BLEND_COLOR_KEY_ACTION_ACTION_NOT_SRC_DEST_SHIFT) | \
        ((snd) << BCHP_M2MC_BLEND_COLOR_KEY_ACTION_ACTION_SRC_NOT_DEST_SHIFT) | \
        ((sd) << BCHP_M2MC_BLEND_COLOR_KEY_ACTION_ACTION_SRC_DEST_SHIFT); \
}

/***************************************************************************/
#define BGRC_P_SET_FIELD_DIRECTION( surf0, surf1, surf2, dir, coord, fore, back ) \
{ \
    if( BGRC_P_GET_FIELD_DATA( BLIT_##surf0##_TOP_LEFT, coord ) < \
        BGRC_P_GET_FIELD_DATA( BLIT_##surf1##_TOP_LEFT, coord ) ) \
    { \
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, surf0##_##dir##_DIRECTION, back ); \
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, surf1##_##dir##_DIRECTION, back ); \
        BGRC_P_SET_FIELD_COMP( BLIT_CTRL, surf2##_##dir##_DIRECTION, back, fore, \
            BGRC_P_COMPARE_FIELD( surf2##_FEEDER_ENABLE, ENABLE, ENABLE ) ); \
    } \
    else \
    { \
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, surf0##_##dir##_DIRECTION, fore ); \
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, surf1##_##dir##_DIRECTION, fore ); \
        BGRC_P_SET_FIELD_ENUM( BLIT_CTRL, surf2##_##dir##_DIRECTION, fore ); \
    } \
}

/***************************************************************************/
#define BGRC_P_SURFACE_INTERSECT( surf0, surf1 ) (( \
    BGRC_P_GET_FIELD_DATA( surf0##_SURFACE_ADDR_0, ADDR ) == \
    BGRC_P_GET_FIELD_DATA( surf1##_SURFACE_ADDR_0, ADDR )) && (!((( \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf0##_TOP_LEFT, LEFT ) + \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf0##_SIZE, SURFACE_WIDTH )) < \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf1##_TOP_LEFT, LEFT )) || ( \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf0##_TOP_LEFT, LEFT ) > ( \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf1##_TOP_LEFT, LEFT ) + \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf1##_SIZE, SURFACE_WIDTH ))) || (( \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf0##_TOP_LEFT, TOP ) + \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf0##_SIZE, SURFACE_HEIGHT )) < \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf1##_TOP_LEFT, TOP )) || ( \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf0##_TOP_LEFT, TOP ) > ( \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf1##_TOP_LEFT, TOP ) + \
    BGRC_P_GET_FIELD_DATA( BLIT_##surf1##_SIZE, SURFACE_HEIGHT ))))))

/***************************************************************************/
#define BGRC_P_COPY_FIELD( pState, dst, src, type, reg, field ) \
{ \
    if( ((dst)[BGRC_P_REG_INDEX(reg)] & (BCHP_M2MC_##reg##_##field##_MASK)) != \
        ((src)[BGRC_P_REG_INDEX(reg)] & (BCHP_M2MC_##reg##_##field##_MASK)) ) \
    { \
        (dst)[BGRC_P_REG_INDEX(reg)] &= \
            (~(BCHP_M2MC_##reg##_##field##_MASK)); \
        (dst)[BGRC_P_REG_INDEX(reg)] |= (src)[BGRC_P_REG_INDEX(reg)] & \
            (BCHP_M2MC_##reg##_##field##_MASK); \
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, type, GRP_ENABLE, pState, dst ); \
    } \
}

/***************************************************************************/
#define BGRC_P_COPY_REGISTER( pState, dst, src, type, reg ) \
{ \
    if( (dst)[BGRC_P_REG_INDEX(reg)] != (src)[BGRC_P_REG_INDEX(reg)]) \
    { \
        (dst)[BGRC_P_REG_INDEX(reg)] = (src)[BGRC_P_REG_INDEX(reg)]; \
        BGRC_P_LOAD_LIST_GRP( LIST_PACKET_HEADER_1, type, GRP_ENABLE, pState, dst ); \
    } \
}

/***************************************************************************/
#define BGRC_P_REGISTER_CHANGED( reg ) \
    (hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)] != hGrc->aulActualRegs[BGRC_P_REG_INDEX(reg)])

/***************************************************************************/
#if defined(BCHP_M2MC_1_REG_START)
#define BGRC_P_READ_REG( reg ) \
    (uint32_t) BREG_Read32( hGrc->hRegister, (hGrc->ulDeviceNum ? BCHP_M2MC_1_##reg : BCHP_M2MC_##reg) )
#define BGRC_P_WRITE_REG( reg, val ) \
    BREG_Write32( hGrc->hRegister, (hGrc->ulDeviceNum ? BCHP_M2MC_1_##reg : BCHP_M2MC_##reg), (val) )
#elif defined(BCHP_M2MC1_REG_START)
#define BGRC_P_READ_REG( reg ) \
    (uint32_t) BREG_Read32( hGrc->hRegister, (hGrc->ulDeviceNum ? BCHP_M2MC1_##reg  : BCHP_M2MC_##reg) )
#define BGRC_P_WRITE_REG( reg, val ) \
    BREG_Write32( hGrc->hRegister, (hGrc->ulDeviceNum ? BCHP_M2MC1_##reg : BCHP_M2MC_##reg), (val) )
#else
#define BGRC_P_READ_REG( reg ) \
    (uint32_t) BREG_Read32( hGrc->hRegister, BCHP_M2MC_##reg )
#define BGRC_P_WRITE_REG( reg, val ) \
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_##reg, (val) )
#endif

/***************************************************************************/
#define BGRC_P_SET_REGISTER( reg ) \
{ \
    hGrc->aulActualRegs[BGRC_P_REG_INDEX(reg)] = hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)]; \
    *pulPacket++ = hGrc->aulCurrentRegs[BGRC_P_REG_INDEX(reg)]; \
}

/***************************************************************************/
#define BGRC_P_SET_REGISTERS( group, reg ) \
{ \
    uint32_t ulIndex = BGRC_P_REG_INDEX(reg); \
    uint32_t *pulActualRegs = &hGrc->aulActualRegs[ulIndex]; \
    uint32_t *pulCurrentRegs = &hGrc->aulCurrentRegs[ulIndex]; \
    uint32_t ii; \
    for( ii = 0; ii < BGRC_P_LIST_##group##_GRP_CNTRL_COUNT; ++ii ) \
    { \
        *pulActualRegs++ = *pulCurrentRegs; \
        *pulPacket++ = *pulCurrentRegs++; \
    } \
}

/***************************************************************************/
#define BGRC_P_SET_PALETTE_ENTRY( reg, num, value ) \
    pOp->aulPaletteEntries[(num)] = (value)

/***************************************************************************/
#define BGRC_P_SET_PALETTE( reg, surface, count ) \
{ \
    BSUR_Palette_Handle hPalette; \
    uint32_t *pulEntries; \
    uint32_t ii; \
\
    BSUR_Surface_GetPalette( (surface), &hPalette ); \
    if( hPalette ) \
    { \
        BSUR_Palette_GetAddress( hPalette, (void *) (&pulEntries) ); \
        if( pulEntries ) \
        { \
            for( ii = 0; ii < (count); ++ii ) \
                BGRC_P_SET_PALETTE_ENTRY( reg, ii, pulEntries[ii] ); \
        } \
    } \
}

/***************************************************************************/
#define BGRC_P_VALIDATE_SURFACE_DIMENSIONS( x, y, width, height ) ( \
    (!(((x) || (y)) && (((width) == 0) || ((height) == 0)))) && \
    (((int32_t) (x) >= 0) && ((int32_t) (y) >= 0)) )

/***************************************************************************/
#define BGRC_P_VALIDATE_SURFACE_RECTANGLE( rect ) \
    BGRC_P_VALIDATE_SURFACE_DIMENSIONS( (rect)->ulX, (rect)->ulY, (rect)->ulWidth, (rect)->ulHeight )

/***************************************************************************/
#define BGRC_P_VALIDATE_SURFACE_BOUNDS( state, sur ) \
    (!((state)->sur##Surface.hSurface && ( \
    (((state)->sur##Surface.ulX + (state)->sur##Surface.ulWidth) > (state)->sur##Surface.ulSurfaceWidth) || \
    (((state)->sur##Surface.ulY + (state)->sur##Surface.ulHeight) > (state)->sur##Surface.ulSurfaceHeight))))

/***************************************************************************/
#define BGRC_P_GET_SURFACE_DATA( newsurface, newdata, olddata ) \
{ \
    BSUR_Palette_Handle hPalette = 0; \
    BPXL_Format eFormat = 0; \
    uint32_t ulPaletteOffset = 0; \
    uint32_t ulSurfaceID = 0; \
    if( (newsurface) ) \
    { \
        if( BSUR_Surface_GetID( (newsurface), &ulSurfaceID ) != BERR_SUCCESS ) \
            return BERR_TRACE(BERR_INVALID_PARAMETER); \
        if( ulSurfaceID == 0 ) \
        { \
            ulSurfaceID = ++hGrc->ulSurfaceID; \
            if( BSUR_Surface_SetID( (newsurface), ulSurfaceID ) != BERR_SUCCESS ) \
                return BERR_TRACE(BERR_INVALID_PARAMETER); \
        } \
        if( BSUR_Surface_GetFormat( (newsurface), &eFormat ) != BERR_SUCCESS ) \
            return BERR_TRACE(BERR_INVALID_PARAMETER); \
        if( BPXL_IS_PALETTE_FORMAT( eFormat ) ) \
        { \
            if( BSUR_Surface_GetPalette( (newsurface), &hPalette ) != BERR_SUCCESS ) \
                return BERR_TRACE(BERR_INVALID_PARAMETER); \
            if( BSUR_Palette_GetOffset( hPalette, &ulPaletteOffset ) != BERR_SUCCESS ) \
                return BERR_TRACE(BERR_INVALID_PARAMETER); \
        } \
    } \
    if( hGrc->bUninitialized || ((newsurface) != (olddata).hSurface) || \
        (ulSurfaceID != (olddata).ulID) || (ulPaletteOffset != (olddata).ulPaletteOffset) ) \
    { \
        bSurfaceChanged = true; \
        BKNI_Memset( &(newdata), 0, sizeof (BGRC_P_Surface) ); \
        if( (newsurface) ) \
        { \
            (newdata).ulID = ulSurfaceID; \
            (newdata).hSurface = (newsurface); \
            (newdata).eFormat = eFormat; \
            if( BSUR_Surface_GetAddress( (newsurface), &((newdata).pMemory), &((newdata).ulPitch) ) != BERR_SUCCESS ) \
                return BERR_TRACE(BERR_INVALID_PARAMETER); \
            if( BSUR_Surface_GetDimensions( (newsurface), &((newdata).ulSurfaceWidth), &((newdata).ulSurfaceHeight) ) != BERR_SUCCESS ) \
                return BERR_TRACE(BERR_INVALID_PARAMETER); \
            if( BSUR_Surface_GetOffset( (newsurface), &((newdata).ulOffset) ) != BERR_SUCCESS ) \
                return BERR_TRACE(BERR_INVALID_PARAMETER); \
            if( BPXL_IS_PALETTE_FORMAT((newdata).eFormat) ) \
            { \
                (newdata).ulPaletteOffset = ulPaletteOffset; \
                (newdata).ulPaletteEntries = (uint32_t) BPXL_NUM_PALETTE_ENTRIES((newdata).eFormat); \
            } \
        } \
    } \
}

/***************************************************************************/
#define BGRC_P_SET_SURFACE_DIMENSIONS( reg, surface, xx, yy, width, height ) \
{ \
    if( (surface) ) \
    { \
        uint32_t ulWidth = (width); \
        uint32_t ulHeight = (height); \
        uint32_t ulTemp; \
        if( (ulWidth == 0) || (ulHeight == 0) ) \
            BSUR_Surface_GetDimensions( (surface), \
                (ulWidth == 0) ? (&ulWidth) : (&ulTemp), \
                (ulHeight == 0) ? (&ulHeight) : (&ulTemp) ); \
        BGRC_P_SET_FIELD_DATA( BLIT_##reg##_TOP_LEFT, LEFT, (xx) ); \
        BGRC_P_SET_FIELD_DATA( BLIT_##reg##_TOP_LEFT, TOP, (yy) ); \
        BGRC_P_SET_FIELD_DATA( BLIT_##reg##_SIZE, SURFACE_WIDTH, ulWidth ); \
        BGRC_P_SET_FIELD_DATA( BLIT_##reg##_SIZE, SURFACE_HEIGHT, ulHeight ); \
        if( BGRC_P_REGISTER_CHANGED( BLIT_##reg##_SIZE ) || \
            BGRC_P_REGISTER_CHANGED( BLIT_##reg##_TOP_LEFT ) ) \
            BGRC_P_SET_FIELD_ENUM( LIST_PACKET_HEADER_1, BLIT_GRP_CNTRL, GRP_ENABLE ); \
    } \
}

/***************************************************************************/
#define BGRC_P_ALIGN_PATTERN( pout, pin, xx, yy ) \
{ \
    uint32_t ii; \
    for( ii = 0; ii < 8; ++ii ) \
        ((uint8_t *) (pout))[ii] = ((uint8_t *) (pin))[(ii + (yy)) & 7]; \
    for( ii = 0; ii < 8; ++ii ) \
        ((uint8_t *) (pout))[ii] = (uint8_t) ((((uint8_t *) (pout))[ii] >> (8 - ((xx) & 7))) | \
            (((uint8_t *) (pout))[ii] << ((xx) & 7))); \
}

/***************************************************************************/
#define BGRC_P_WRITE_REGISTER( reg ) \
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_##reg, pOp->aulRegs[BGRC_P_REG_INDEX(reg)] )

#define BGRC_P_WRITE_REGISTER_COEFF( reg, num ) \
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_##reg##_FIR_COEFF_PHASE0_01 + (num) * 4, \
        pOp->aulRegs[BGRC_P_REG_INDEX(reg##_FIR_COEFF_PHASE0_01) + (num)] );

#define BGRC_P_WRITE_REGISTER_ENTRY( reg, num ) \
    BREG_Write32( hGrc->hRegister, BCHP_M2MC_##reg##_CLUT_ENTRY_i_ARRAY_BASE + (num) * 4, \
        pOp->aulPaletteEntries[(num)] );

/****************************************************************************/
uint32_t BGRC_P_GetBlendOp( BGRC_Blend_Source eSource );
uint32_t BGRC_P_GetBlendOpInv( BGRC_Blend_Source eSource );

BERR_Code BGRC_P_Blit( BGRC_Handle hGrc, BGRC_Callback pCallback, void *pData, bool bSetEvent );
BERR_Code BGRC_P_FilterBlit( BGRC_Handle hGrc, BGRC_Callback pCallback, void *pData, bool bSetEvent );

void BGRC_P_Source_CopyState( BGRC_P_State *pDstState, BGRC_P_State *pSrcState, uint32_t aulDstRegs[], uint32_t aulSrcRegs[] );
void BGRC_P_Destination_CopyState( BGRC_P_State *pDstState, BGRC_P_State *pSrcState, uint32_t aulDstRegs[], uint32_t aulSrcRegs[] );
void BGRC_P_Pattern_CopyState( BGRC_P_State *pDstState, BGRC_P_State *pSrcState, uint32_t aulDstRegs[], uint32_t aulSrcRegs[] );
void BGRC_P_Blend_CopyState( BGRC_P_State *pDstState, BGRC_P_State *pSrcState, uint32_t aulDstRegs[], uint32_t aulSrcRegs[] );
void BGRC_P_Output_CopyState( BGRC_P_State *pDstState, BGRC_P_State *pSrcState, uint32_t aulDstRegs[], uint32_t aulSrcRegs[] );
void BGRC_P_PrintRegisters( BGRC_Handle hGrc );

bool BGRC_P_List_InitPacketMemory( BGRC_Handle hGrc, uint32_t ulMemorySize );
void BGRC_P_List_FreePacketMemory( BGRC_Handle hGrc );
void BGRC_P_List_CleanupPacketMemory( BGRC_Handle hGrc );
void BGRC_P_List_PacketIsr( void *pvParam1, int iParam2 );

void BGRC_P_Operation_FreeAll( BGRC_Handle hGrc );
void BGRC_P_Operation_CleanupList( BGRC_Handle hGrc );
bool BGRC_P_Operation_Prealloc( BGRC_Handle hGrc, uint32_t ulCount );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGRC_PRIVATE_H__ */

/* end of file */
