/***************************************************************************
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
 *
 * Module Description:
 *
 ***************************************************************************/
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
#include "bavc_vee.h"
#include "bchp_vice2_vip_0_0.h"
#if BVDC_P_DUMP_VIP_PICTURE
#include <stdio.h>
#include <stdlib.h>
#endif
/***************************************************************************
 * Private defines
 ***************************************************************************/
/* 7250 7364A:
 */
#define BVDC_P_VIP_VER_0                      (0)
/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#define BVDC_P_VIP_GET_REG_IDX(reg) \
    ((BCHP##_##reg - BCHP_VICE2_VIP_0_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_VIP_GET_REG_DATA(reg) \
    (hVip->aulRegs[BVDC_P_VIP_GET_REG_IDX(reg)])
#define BVDC_P_VIP_SET_REG_DATA(reg, data) \
    (BVDC_P_VIP_GET_REG_DATA(reg) = (uint32_t)(data))

/* Get field */
#define BVDC_P_VIP_GET_FIELD_NAME(reg, field) \
    (BVDC_P_GET_FIELD(BVDC_P_VIP_GET_REG_DATA(reg), reg, field))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_VIP_WRITE_TO_RUL(reg, addr_ptr) \
{ \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hVip->ulRegOffset); \
    *addr_ptr++ = BVDC_P_VIP_GET_REG_DATA(reg); \
}

/* This macro does a block write into RUL */
#define BVDC_P_VIP_BLOCK_WRITE_TO_RUL(from, to, pulCurrent) \
do { \
    uint32_t ulBlockSize = \
        BVDC_P_REGS_ENTRIES(from, to);\
    *pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
    *pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hVip->ulRegOffset); \
    BKNI_Memcpy((void*)pulCurrent, \
        (void*)&(hVip->aulRegs[BVDC_P_VIP_GET_REG_IDX(from)]), \
        ulBlockSize * sizeof(uint32_t)); \
    pulCurrent += ulBlockSize; \
} while(0)

/* number of registers in one block. */
#define BVDC_P_VIP_REGS_COUNT      \
    BVDC_P_REGS_ENTRIES(VICE2_VIP_0_0_REG_START, VICE2_VIP_0_0_REG_END)

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

/***************************************************************************
 * BVDC_P_Buffer_Head
 *      Head of the double Link List for VIP buffers
 ***************************************************************************/
typedef struct BVDC_P_VipBufferQueue  BVDC_P_VipBufferQueue;
BLST_SQ_HEAD(BVDC_P_VipBufferQueue, BVDC_P_VipBufferNode);

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
    BVDC_P_VipBufferQueue         stFreeQ;   /* free Q available for VIP capture */
    BVDC_P_VipBufferQueue         stCaptureQ;/* captured Q to be delivered */
    BVDC_P_VipBufferQueue         stDeliverQ;/* deliver Q to encoder */

    /* Note RUL has a picture delay, and completing VIP capture takes another picture delay.
       The following picture node pointers are used to implement the picture delays! */
    BVDC_P_VipBufferNode         *pToCapture, *pCapture;

    /* flag initial state, requires reset; */
    bool                           bInitial;
    uint32_t                       ulResetRegAddr;
    uint32_t                       ulResetMask;

    /* private fields. */
    unsigned                       eId;
    uint32_t                       ulRegOffset; /* VIP_0, VIP_1, and etc. */
    uint32_t                       aulRegs[BVDC_P_VIP_REGS_COUNT];

    /* picture heap */
    BVDC_P_VipMemSettings          stMemSettings;
    BVDC_P_VipMemConfig            stMemConfig;
    BMMA_Block_Handle              hBlock;
    uint32_t                       ulDeviceOffset;

    /* Data mode */
    BVDC_P_VipDataMode             eVipDataMode;

    /* if previous MFD picture ignore == false && previous VIP is full (non-ignore drop), this time don't drop the repeated ignore picture! */
    bool                           bPrevNonIgnoreDropByFull;

#if BVDC_P_DUMP_VIP_PICTURE
    void                          *pY, *pC;
    FILE                          *pfY, *pfC;
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
