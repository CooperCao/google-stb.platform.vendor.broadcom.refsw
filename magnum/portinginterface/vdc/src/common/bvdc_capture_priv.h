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
#ifndef BVDC_CAPTURE_PRIV_H__
#define BVDC_CAPTURE_PRIV_H__

#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bchp_cap_0.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Private defines
 ***************************************************************************/
#if 0
/* 3560, 7401A, 7400A:
 * no BVB_IN_SIZE. PIC_OFFSET seperate from PIC_SIZE -> RX_CTRL block
 */
#define BVDC_P_CAP_VER_0                      (0)
/* 7401B, 7400B, 7403, 7118, 7440:
 * PIC_OFFSET -> BVB_IN_SIZE seperate from PIC_SIZE -> RX_CTRL block
 */
#define BVDC_P_CAP_VER_1                      (1)
/* 3563, 7405:
 * PIC_OFFSET and BVB_IN_SIZE in block: PIC_SIZE -> LINE_CMP_TRIG_1_CFG
 */
#define BVDC_P_CAP_VER_2                      (2)
/* 3548:
 * Dither support added.
 */
#define BVDC_P_CAP_VER_3                      (3)
#endif

/* 7422:
 * 3D support added.
 */
#define BVDC_P_CAP_VER_4                      (4)
/* 7425Bx, 7344Bx, 7231Bx, 7346Bx:
 * PIC_OFFSET_R added.
 */
#define BVDC_P_CAP_VER_5                      (5)
/* 7435:
 * CAP_x_CTRL.ENABLE_CTRL added.
 */
#define BVDC_P_CAP_VER_6                      (6)

/* 7366B 7364A:
 * CAP_x_DCEM_RECT_SIZE[0..15]added.
 */
#define BVDC_P_CAP_VER_7                      (7)

/* 7271A:
 * New setting for CAP_0_PITCH in mosaic mode
 */
#define BVDC_P_CAP_VER_8                      (8)

/* New pitch setting for mosaic */
#define BVDC_P_CAP_SUPPORT_NEW_MEMORY_PITCH   \
     (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_8)

/* CRBVN-697 CAP: SCB1/2 full compression mode window size limitation */
#if (BVDC_P_SUPPORT_CAP_VER == BVDC_P_CAP_VER_7)
#define BVDC_P_CAP_DCXM_SCB_WORKAROUND   (1)
#endif

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_CAP_WRITE_TO_RUL(reg, addr_ptr, data) \
{ \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hCapture->ulRegOffset); \
    *addr_ptr++ = data; \
}

#define BVDC_P_Capture_MuxAddr(hCap)   (BCHP_VNET_B_CAP_0_SRC + (hCap)->eId * sizeof(uint32_t))
#define BVDC_P_Capture_SetVnet_isr(hCap, ulSrcMuxValue, eVnetPatchMode) \
   BVDC_P_SubRul_SetVnet_isr(&((hCap)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Capture_UnsetVnet_isr(hCap) \
   BVDC_P_SubRul_UnsetVnet_isr(&((hCap)->SubRul))

/***************************************************************************
 * Capture private data sturctures
 ***************************************************************************/
typedef enum BVDC_P_Capture_Trigger
{
    BVDC_P_Capture_Trigger_eDisabled = 0,
    BVDC_P_Capture_Trigger_eBvb,
    BVDC_P_Capture_Trigger_eLineCmp = 2
} BVDC_P_Capture_Trigger;

/* BVN path capture data mode */
typedef enum BVDC_P_Capture_DataMode
{
    BVDC_P_Capture_DataMode_e8Bit422 = 0,
    BVDC_P_Capture_DataMode_e10Bit422,
    BVDC_P_Capture_DataMode_e10Bit444,

    BVDC_P_Capture_DataMode_eMaxCount

} BVDC_P_Capture_DataMode;

typedef struct BVDC_P_CaptureRegisterSetting
{
    uint32_t       ulPicSize;         /* CAP_0_PIC_SIZE */
    uint32_t       ulBvbInSize;       /* CAP_0_BVB_IN_SIZE */
    uint32_t       ulPicOffset;       /* CAP_0_PIC_OFFSET */
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_5)
    uint32_t       ulPicOffsetR;       /* CAP_0_PIC_OFFSET_R */
#endif
    uint32_t       ulPitch;            /* CAP_0_PITCH */

    uint32_t       ulMode;             /* CAP_0_MODE */
    uint32_t       ulCompOrder;        /* CAP_0_COMP_ORDER or CAP_0_BYTE_ORDER */
    uint32_t       ulTrigCtrl;         /* CAP_0_TRIG_CTRL  */
    uint32_t       ulCtrl;             /* CAP_0_CTRL */

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
    uint32_t       ulDcemCfg;          /* CAP_0_DCEM_CFG */
    uint32_t       ulDcemRectCtrl;     /* CAP_0_DCEM_RECT_CTRL */
    uint32_t       ulDcemRectMask;     /* CAP_0_DCEM_RECT_ENABLE_MASK */
    uint32_t       ulDcemRectId;       /* CAP_0_DCEM_RECT_ID */
    uint32_t       ulDecmRect[BAVC_MOSAIC_MAX];        /*CAP_0_DCEM_RECT_SIZEi*/
    uint32_t       ulDecmRectOffset[BAVC_MOSAIC_MAX];  /*CAP_0_DCEM_RECT_OFFSETi*/
#endif

    BMMA_DeviceOffset  ullMStart;       /* CAP_0_MSTART */
#if (BVDC_P_SUPPORT_CAP_VER >= BVDC_P_CAP_VER_4)
    BMMA_DeviceOffset  ullMStartR;       /* CAP_0_MSTART_R */
#endif

} BVDC_P_CaptureRegisterSetting;

typedef struct BVDC_P_CaptureContext
{
    BDBG_OBJECT(BVDC_CAP)

    /* Window associated with this capture.
     * note: one capture can not be shared by more than one window */
    BVDC_Window_Handle             hWindow;

    /* flag initial state, requires reset; */
    bool                           bInitial;
    uint32_t                       ulResetRegAddr;
    uint32_t                       ulResetMask;

    /* private fields. */
    BVDC_P_CaptureId               eId;
    BRDC_Trigger                   eTrig;
    uint32_t                       ulRegOffset; /* CAP_0, CAP_1, and etc. */

    BVDC_P_CaptureRegisterSetting  stRegs;
    BAVC_Polarity                  eCapturePolarity;

    /* A register handle.  Triggers need to be enable by host writes.
     * A memory handle to do address/offset converting. */
    BREG_Handle                    hRegister;
    BRDC_Handle                    hRdc;

    /* Keeps track of when ISR executed */
    uint32_t                       ulTimestamp;

#if (!BVDC_P_USE_RDC_TIMESTAMP)
    BTMR_TimerHandle               hTimer;
    BTMR_TimerRegisters            stTimerReg;
    /* a capture block's scratch register */
    uint32_t                       ulTimestampRegAddr;
#endif

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext           SubRul;

    BVDC_444To422DnSampler         stDnSampler;

    /* Data mode */
    BVDC_P_Capture_DataMode        eCapDataMode;
    bool                           bEnableDcxm;

} BVDC_P_CaptureContext;


/***************************************************************************
 * Capture private functions
 ***************************************************************************/
BERR_Code BVDC_P_Capture_Create
    ( BVDC_P_Capture_Handle           *phCapture,
      BRDC_Handle                      hRdc,
      BREG_Handle                      hRegister,
      BVDC_P_CaptureId                 eCaptureId,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
      BTMR_TimerHandle                 hTimer,
#endif
      BVDC_P_Resource_Handle           hResource );

void BVDC_P_Capture_Destroy
    ( BVDC_P_Capture_Handle            hCapture );

void BVDC_P_Capture_Init
    ( BVDC_P_Capture_Handle            hCapture,
      BVDC_Window_Handle               hWindow );

void BVDC_P_Capture_BuildRul_isr
    ( const BVDC_P_Capture_Handle      hCapture,
      BVDC_P_ListInfo                 *pList,
      BVDC_P_State                     eVnetState,
      const BVDC_P_PictureNodePtr      pPicture);

BERR_Code BVDC_P_Capture_SetBuffer_isr
    ( BVDC_P_Capture_Handle            hCapture,
      BMMA_DeviceOffset                ullDeviceAddr,
      BMMA_DeviceOffset                ullDeviceAddr_R,
      uint32_t                         ulPitch );

BERR_Code BVDC_P_Capture_SetEnable_isr
    ( BVDC_P_Capture_Handle            hCapture,
      bool                             bEnable );

BERR_Code BVDC_P_Capture_SetInfo_isr
    ( BVDC_P_Capture_Handle            hCapture,
      BVDC_Window_Handle               hWindow,
      const BVDC_P_PictureNodePtr      pPicture,
      uint32_t                         ulPictureIdx,
      bool                             bLastPic );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_CAPTURE_PRIV_H__ */
/* End of file. */
