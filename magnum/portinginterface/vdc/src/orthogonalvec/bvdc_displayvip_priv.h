/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef BVDC_VIP_PRIV_H__
#define BVDC_VIP_PRIV_H__

#include "blst_squeue.h"
#include "bchp_common.h"
#include "bvdc.h"
#include "bvdc_common_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#if BVDC_P_SUPPORT_VIP
#if BCHP_VICE2_VIP_0_REG_START
#include "bchp_vice2_vip_0.h"
#elif BCHP_VICE_VIP_0_0_REG_START
#include "bchp_vice_vip_0_0.h"
#else
#include "bchp_vice2_vip_0_0.h"
#endif
#if BVDC_P_DUMP_VIP_PICTURE
#include <stdio.h>
#include <stdlib.h>
#endif
#include "bvdc_displayitfp_priv.h"

/***************************************************************************
 * Private defines
 ***************************************************************************/
BDBG_OBJECT_ID_DECLARE(BVDC_VIP);

/* 7250 7364A:
 */
#if(BCHP_CHIP == 7425) || (BCHP_CHIP == 7435)
#define BVDC_P_VIP_VER                      0
#define BVDC_P_VIP_MAX_H1V_WIDTH            0 /* 2H decimate only */
#define BVDC_P_VIP_MAX_H1V_HEIGHT           1080
#elif (BCHP_CHIP == 7445) || (BCHP_CHIP == 7366) || (BCHP_CHIP == 7439) || \
      (BCHP_CHIP == 7250) || (BCHP_CHIP == 7364)
#define BVDC_P_VIP_VER                      1 /* 960 limit for 2H vs 4H decimate */
#define BVDC_P_VIP_MAX_H1V_WIDTH            960
#define BVDC_P_VIP_MAX_H1V_HEIGHT           1080
#else /* 7278 & later ViCE3 */
#define BVDC_P_VIP_VER                      2 /* 640 limit (CME) for 1H vs 2H decimate */
/* Note: 1H1V limit is imposed by CME throughput capacity */
#define BVDC_P_VIP_MAX_H1V_WIDTH            640
#define BVDC_P_VIP_MAX_H1V_HEIGHT           360
#endif
/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#ifdef BCHP_VICE2_VIP_0_REG_START
#define BVDC_P_VIP_RDB(reg)   BCHP_VICE2_VIP_0_##reg

#define BVDC_P_VIP_RDB_ENUM(Register,Field,Name) \
    BCHP_FIELD_DATA(VICE2_VIP_0_##Register,Field, BCHP_VICE2_VIP_0_##Register##_##Field##_##Name)

#define BVDC_P_VIP_RDB_DATA(Register,Field,data) \
    BCHP_FIELD_DATA(VICE2_VIP_0_##Register,Field, data)

#define BVDC_P_VIP_RDB_MASK(Register,Field) BCHP_MASK(VICE2_VIP_0_##Register,Field)

/* number of registers in one block. */
#define BVDC_P_VIP_REGS_COUNT      \
    BVDC_P_REGS_ENTRIES(VICE2_VIP_0_REG_START, VICE2_VIP_0_REG_END)

#elif BCHP_VICE_VIP_0_0_REG_START
#define BVDC_P_VIP_RDB(reg)   BCHP_VICE_VIP_0_0_##reg

#define BVDC_P_VIP_RDB_ENUM(Register,Field,Name) \
   BCHP_FIELD_DATA(VICE_VIP_0_0_##Register,Field, BCHP_VICE_VIP_0_0_##Register##_##Field##_##Name)

#define BVDC_P_VIP_RDB_DATA(Register,Field,data) \
   BCHP_FIELD_DATA(VICE_VIP_0_0_##Register,Field, data)

#define BVDC_P_VIP_RDB_MASK(Register,Field) BCHP_MASK(VICE_VIP_0_0_##Register,Field)

/* number of registers in one block. */
#define BVDC_P_VIP_REGS_COUNT      \
    BVDC_P_REGS_ENTRIES(VICE_VIP_0_0_REG_START, VICE_VIP_0_0_REG_END)
#else
#define BVDC_P_VIP_RDB(reg)   BCHP_VICE2_VIP_0_0_##reg

#define BVDC_P_VIP_RDB_ENUM(Register,Field,Name) \
    BCHP_FIELD_DATA(VICE2_VIP_0_0_##Register,Field, BCHP_VICE2_VIP_0_0_##Register##_##Field##_##Name)

#define BVDC_P_VIP_RDB_DATA(Register,Field,data) \
    BCHP_FIELD_DATA(VICE2_VIP_0_0_##Register,Field, data)

#define BVDC_P_VIP_RDB_MASK(Register,Field) BCHP_MASK(VICE2_VIP_0_0_##Register,Field)

#endif
/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_VIP_WRITE_TO_RUL(reg, addr_ptr, data) \
{ \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BVDC_P_VIP_RDB(reg) + hVip->ulRegOffset); \
    *addr_ptr++ = data; \
}

/***************************************************************************
 * Vip private data types
 ***************************************************************************/
/* VIP capture data mode */
typedef enum BVDC_P_VipDataMode
{
    BVDC_P_VipDataMode_eStripe = 0,
    BVDC_P_VipDataMode_eLinear,

    BVDC_P_VipDataMode_eMaxCount

} BVDC_P_VipDataMode;

/* VIP memory config settings */
typedef struct BVDC_P_VipMemSettings
{
    /* memory stripe parameters */
    uint32_t    PageSize;    /* 2/4/8KB */
    uint32_t    DramStripeWidth;
    uint32_t    X, Y;        /* MB count = n*X + Y */
    bool        DcxvEnable;  /* compression? */
    bool        bInterlaced; /* support interlaced in max format? */
    bool        bDecimatedLuma; /* support decimated luma? */
    bool        bBframes; /* support B frames? */
    uint32_t    MaxPictureWidthInPels; /* max resolution */
    uint32_t    MaxPictureHeightInPels;

} BVDC_P_VipMemSettings;

/* VIP memory config */
typedef struct BVDC_P_VipMemConfig
{
    uint32_t    ulTotalSize;

    uint32_t    ulNumOrigBuf;
    uint32_t    ulLumaBufSize;          /* original Y/C */
    uint32_t    ulChromaBufSize;
    uint32_t    ulNumShiftedBuf;
    uint32_t    ulShiftedChromaBufSize; /* for interlaced only */

    uint32_t    ulNumDecimBuf;
    uint32_t    ul2H1VBufSize;          /* decimated luma */
    uint32_t    ul2H2VBufSize;

} BVDC_P_VipMemConfig;

/* VIP capture buffer node */
typedef struct BVDC_P_VipBufferNode
{
    /* Node info: linked-list bookeeping */
    BLST_SQ_ENTRY(BVDC_P_VipBufferNode)   link;       /* doubly-linked list support */
    uint32_t                              ulBufferId; /* Buffer ID */

    BAVC_EncodePictureBuffer              stPicture;  /* encode picture parameters */
} BVDC_P_VipBufferNode;

/* VIP capture decim buffer node */
typedef struct BVDC_P_VipAssocBufferNode
{
    /* Node info: linked-list bookeeping */
    BLST_SQ_ENTRY(BVDC_P_VipAssocBufferNode) link;       /* doubly-linked list support */
    uint32_t                              ulBufferId; /* Buffer ID */

    BMMA_Block_Handle                     hBlock;
    uint32_t                              ulOffset;
} BVDC_P_VipAssocBufferNode;

/***************************************************************************
 * BVDC_P_Buffer_Head
 *      Head of the double Link List for VIP buffers
 * 5 queues for
 *   - original luma
 *   - chroma
 *   - shifted chroma (interlaced)
 *   - decimated 1v luma
 *   - decimated 2v luma
 ***************************************************************************/
typedef struct BVDC_P_VipBufferQueue  BVDC_P_VipBufferQueue;
BLST_SQ_HEAD(BVDC_P_VipBufferQueue, BVDC_P_VipBufferNode);

typedef struct BVDC_P_VipChromaBufferQueue  BVDC_P_VipChromaBufferQueue;
BLST_SQ_HEAD(BVDC_P_VipChromaBufferQueue, BVDC_P_VipAssocBufferNode);

typedef struct BVDC_P_VipShiftedBufferQueue  BVDC_P_VipShiftedBufferQueue;
BLST_SQ_HEAD(BVDC_P_VipShiftedBufferQueue, BVDC_P_VipAssocBufferNode);

typedef struct BVDC_P_VipDecim1vBufferQueue  BVDC_P_VipDecim1vBufferQueue;
BLST_SQ_HEAD(BVDC_P_VipDecim1vBufferQueue, BVDC_P_VipAssocBufferNode);

typedef struct BVDC_P_VipDecim2vBufferQueue  BVDC_P_VipDecim2vBufferQueue;
BLST_SQ_HEAD(BVDC_P_VipDecim2vBufferQueue, BVDC_P_VipAssocBufferNode);

typedef struct BVDC_P_VipRegisterSetting
{
    uint32_t                 ulFwControl;       /* VICE2_VIP_0_0_FW_CONTROL */
    uint32_t                 ulConfig;          /* VICE2_VIP_0_0_CONFIG */
    uint32_t                 ulDcxvCfg;         /* VICE2_VIP_0_0_DCXV_CFG */

    uint32_t                 ulInputPicSize;    /* VICE2_VIP_0_0_INPUT_PICTURE_SIZE */
    uint32_t                 ulOutputPicSize;   /* VICE2_VIP_0_0_OUTPUT_PICTURE_SIZE */
    uint32_t                 ulLumaNMBY;        /* VICE2_VIP_0_0_LUMA_NMBY */
    uint32_t                 ulChromaNMBY;      /* VICE2_VIP_0_0_CHROMA_420_NMBY */
    uint32_t                 ul1VLumaNMBY;      /* VICE2_VIP_0_0_DCMH1V_NMBY */
    uint32_t                 ul2VLumaNMBY;      /* VICE2_VIP_0_0_DCMH2V_NMBY */
    uint32_t                 ulShiftedChromaNMBY; /* VICE2_VIP_0_0_SHIFT_CHROMA_NMBY */
    uint32_t                 ulStatsLineRange;    /* VICE2_VIP_0_0_PCC/HIST_LINE_RANGE */

    BMMA_DeviceOffset        ullLumaAddr;       /* VICE2_VIP_0_0_LUMA_ADDR */
    BMMA_DeviceOffset        ullChromaAddr;     /* VICE2_VIP_0_0_CHROMA_420_ADDR */
    BMMA_DeviceOffset        ull1VLumaAddr;     /* VICE2_VIP_0_0_DCMH1V_ADDR */
    BMMA_DeviceOffset        ull2VLumaAddr;     /* VICE2_VIP_0_0_DCMH2V_ADDR */
    BMMA_DeviceOffset        ullShiftedChromaAddr; /* VICE2_VIP_0_0_SHIFT_CHROMA_ADDR */
    BMMA_DeviceOffset        ullPccLumaAddr;    /* VICE2_VIP_0_0_PCC_LUMA_ADDR */
    BMMA_DeviceOffset        ullHistLumaAddr;   /* VICE2_VIP_0_0_HIST_LUMA_ADDR */
} BVDC_P_VipRegisterSetting;

typedef struct BVDC_P_VipContext
{
    BDBG_OBJECT(BVDC_VIP)

    /* Display associated with this VIP capture.
     * note: one VIP can not be shared by more than one display */
    BVDC_Display_Handle            hDisplay;

    /* Life cycle of an encode picture buffer:
       Display_isr would move a buffer from             FreeQ -> captureQ;
       GetBuffer API would move a buffer from     captureQ -> DeliverQ;
       ReturnBuffer API would move a buffer from DeliverQ -> FreeQ. */
    BVDC_P_VipBufferQueue         stFreeQ;   /* free Q (orig pic) available for VIP capture */
    BVDC_P_VipBufferQueue         stCaptureQ;/* captured Q to be delivered (orig pic) */
    BVDC_P_VipBufferQueue         stDeliverQ;/* deliver Q to encoder (orig pic) */

    /* Note RUL has a picture delay, and completing VIP capture takes another picture delay.
       The following picture node pointers are used to implement the picture delays! */
    BVDC_P_VipBufferNode         *pToCapture, *pCapture;

    /* decimated luma buffer queues: note separate queues needed as decimated luma buffers
       could be returned at different time from the original picture buffers. */
    BVDC_P_VipDecim1vBufferQueue  stFreeQdecim1v; /* free Q (decimated 1v luma) available for VIP capture */
    BVDC_P_VipDecim1vBufferQueue  stCaptureQdecim1v;/* captured Q to be delivered (decim 1v Luma) */
    BVDC_P_VipDecim1vBufferQueue  stDeliverQdecim1v;/* deliver Q to encoder (decim 1v luma) */
    BVDC_P_VipAssocBufferNode    *pToCaptureDecim1v, *pCaptureDecim1v; /* intermediate decim 1v buffers */

    BVDC_P_VipDecim2vBufferQueue  stFreeQdecim2v; /* free Q (decimated 2v luma) available for VIP capture */
    BVDC_P_VipDecim2vBufferQueue  stCaptureQdecim2v;/* captured Q to be delivered (decim 2v Luma) */
    BVDC_P_VipDecim2vBufferQueue  stDeliverQdecim2v;/* deliver Q to encoder (decim 2v luma) */
    BVDC_P_VipAssocBufferNode    *pToCaptureDecim2v, *pCaptureDecim2v; /* intermediate decim 2v buffers */

    BVDC_P_VipChromaBufferQueue  stFreeQchroma; /* free Q (chroma) available for VIP capture */
    BVDC_P_VipChromaBufferQueue  stCaptureQchroma;/* captured Q to be delivered (chroma) */
    BVDC_P_VipChromaBufferQueue  stDeliverQchroma;/* deliver Q to encoder (chroma) */
    BVDC_P_VipAssocBufferNode    *pToCaptureChroma, *pCaptureChroma; /* intermediate chroma buffers */

    BVDC_P_VipShiftedBufferQueue  stFreeQshifted; /* free Q (shifted chroma) available for VIP capture */
    BVDC_P_VipShiftedBufferQueue  stCaptureQshifted;/* captured Q to be delivered (shifted chroma) */
    BVDC_P_VipShiftedBufferQueue  stDeliverQshifted;/* deliver Q to encoder (shifted chroma) */
    BVDC_P_VipAssocBufferNode    *pToCaptureShifted, *pCaptureShifted; /* intermediate shifted chroma buffers */

    /* 4-field ITFP queue */
    struct {
       BVDC_P_VipBufferNode      *pPic;
       BVDC_P_VipAssocBufferNode *pDecim1v, *pDecim2v, *pChroma, *pShifted;
    } astItfpPicQ[BVDC_P_ITFP_PREPROCESSOR_PIC_QUEUE_SIZE];

    /* flag initial state, requires reset; */
    bool                           bInitial;
    uint32_t                       ulResetRegAddr;
    uint32_t                       ulResetMask;

    /* private fields. */
    unsigned                       eId;
    uint32_t                       ulRegOffset; /* VIP_0, VIP_1, and etc. */
    BVDC_P_VipRegisterSetting      stRegs;
    uint32_t                       ulSlip2Lock, ulPrevSlip2Lock;

    /* picture heap */
    BVDC_P_VipMemSettings          stMemSettings;
    BVDC_P_VipMemConfig            stMemConfig;
    BMMA_Block_Handle              hBlock;
    BMMA_DeviceOffset              ullDeviceOffset;

    /* Data mode */
    BVDC_P_VipDataMode             eVipDataMode;

    /* if previous MFD picture ignore == false && previous VIP is full (non-ignore drop), this time don't drop the repeated ignore picture! */
    bool                           bPrevNonIgnoreDropByFull;

    /* ITFP algorithm states */
    uint32_t                       prev1VLumaOffset; /* progressive previous frame */
    uint32_t                       prevPrev1VLumaOffset; /* interlaced previous frame */
    int32_t                        prev_sigma[BVDC_P_ITFP_MAX_SIGMA];
    int32_t                        prev_sigma2[BVDC_P_ITFP_MAX_SIGMA];
    int8_t                         vice_cd_fcnt;
    struct {
        uint32_t stats_to_load     : 2;
        uint32_t found_32          : 1;
        uint32_t using_bff         : 1;
        uint32_t try_to_switch_pol : 1;
    }         cad_control;
    BVDC_P_ITFP_cad_info_t         vice_cd_cad[BVDC_P_ITFP_NUM_CADENCE];
    BVDC_P_ITFP_EpmPreprocessorInfo_t stEpmInfo;

#if BVDC_P_DUMP_VIP_PICTURE
    void                          *pY, *pC;
    FILE                          *pfY, *pfC;
    FILE                          *pfY2H1V, *pfY2H2V, *pfCshifted;
    bool                           bDumped;
    unsigned                       dumpCnt;
    unsigned                       numPicsToCapture;
#endif
} BVDC_P_VipContext;


/***************************************************************************
 * Vip functions
 ***************************************************************************/
BERR_Code BVDC_P_Vip_Create
    ( BVDC_P_Vip_Handle           *phVip,
      unsigned                     id,
      BVDC_Handle                  hVdc);

void BVDC_P_Vip_Destroy
    ( BVDC_P_Vip_Handle            hVip );

void BVDC_P_Vip_Init
    ( BVDC_P_Vip_Handle            hVip );

BERR_Code BVDC_P_Vip_AllocBuffer
    ( BVDC_P_Vip_Handle            hVip,
      BVDC_Display_Handle          hDisplay );

BERR_Code BVDC_P_Vip_FreeBuffer
    ( BVDC_P_Vip_Handle            hVip );

void BVDC_P_Vip_GetBuffer_isr
    ( BVDC_P_Vip_Handle         hVip,
      BAVC_EncodePictureBuffer *pPicture );

void BVDC_P_Vip_ReturnBuffer_isr
    ( BVDC_P_Vip_Handle               hVip,
      const BAVC_EncodePictureBuffer *pPicture );

void BVDC_P_Vip_BuildRul_isr
    ( const BVDC_P_Vip_Handle      hVip,
      BVDC_P_ListInfo             *pList,
      BAVC_Polarity                eFieldPolarity );

uint32_t BVDC_P_MemConfig_GetVipBufSizes
    ( const BVDC_P_VipMemSettings *pstMemSettings,
      BVDC_P_VipMemConfig         *pstMemConfig );
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_VIP_PRIV_H__ */
/* End of file. */
