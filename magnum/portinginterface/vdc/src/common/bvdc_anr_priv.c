/***************************************************************************
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
 *
 * Module Description:
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "brdc.h"
#include "bvdc.h"
#include "bchp_vnet_f.h"
#include "bvdc_resource_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_anr_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_capture_priv.h"

#if (BVDC_P_SUPPORT_MANR)

#include "bchp_hd_anr_mctf_0.h"
#include "bchp_hd_anr_and_0.h"
#include "bchp_mmisc.h"

#if BVDC_P_SUPPORT_DMISC
#include "bchp_dmisc.h"
#endif



BDBG_MODULE(BVDC_ANR);
BDBG_OBJECT_ID(BVDC_ANR);

/***************************************************************************
 *
 * The followings are exported to other sub-modules inside VDC
 *
 ***************************************************************************/

#define BVDC_P_ANR_MSG_ON              0
#if (BVDC_P_ANR_MSG_ON==1)
#define BDBG_P_ANR_MSG    BDBG_ERR
#else
#define BDBG_P_ANR_MSG(a)
#endif

/***************************************************************************
 *
 ***************************************************************************/
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_2)
#define HW3548_47_IS_FIXED                            (1)
#else
#define HW3548_47_IS_FIXED                            (0)
#endif

#define BVDC_P_ESB_WIDTH_THRES 720
#define BVDC_P_ESB_HEIGHT_THRES 480

/***************************************************************************
 * {private}
 *
 * BVDC_P_Anr_Create
 *
 * called by BVDC_Open only
 */
BERR_Code BVDC_P_Anr_Create
    ( BVDC_P_Anr_Handle *         phAnr,
      BVDC_P_AnrId                eAnrId,
      BREG_Handle                 hRegister,
      BVDC_P_Resource_Handle      hResource )
{
    BVDC_P_AnrContext  *pAnr;
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Anr_Create);

    if(eAnrId == BVDC_P_AnrId_eUnknown)
        return eResult;
    /* Debug level are control by app.  Only needed to turn on for debugging
     * purpose only. */
    /*BDBG_SetModuleLevel("BVDC_ANR", BDBG_eMsg);*/

    /* in case creation failed */
    BDBG_ASSERT(phAnr);

    pAnr = (BVDC_P_AnrContext *) (BKNI_Malloc(sizeof(BVDC_P_AnrContext)));
    if( pAnr )
    {
        /* init the context */
        BKNI_Memset((void*)pAnr, 0x0, sizeof(BVDC_P_AnrContext));
        BDBG_OBJECT_SET(pAnr, BVDC_ANR);
        pAnr->eId = eAnrId;
        pAnr->hRegister = hRegister;
        pAnr->eWinId = BVDC_P_SRC_INVALID;
        pAnr->eBufHeapId = BVDC_P_BufferHeapId_eUnknown;
        pAnr->ulRegOffset = 0;
        /* init the SubRul sub-module */
        BVDC_P_SubRul_Init(&(pAnr->SubRul), BVDC_P_Anr_MuxAddr(pAnr),
            BVDC_P_Anr_PostMuxValue(pAnr), BVDC_P_DrainMode_eBack,
            0, hResource);

        /* default settings for up sampler and down sampler */
        pAnr->stUpSampler.bUnbiasedRound = true;
        pAnr->stUpSampler.eFilterType    = BVDC_422To444Filter_eTenTaps;
        pAnr->stUpSampler.eRingRemoval   = BVDC_RingSuppressionMode_eNormal;

        pAnr->stDnSampler.eFilterType    = BVDC_444To422Filter_eStandard;
        pAnr->stDnSampler.eRingRemoval   = BVDC_RingSuppressionMode_eNormal;

        /* no memory sharing by default */
        pAnr->ePxlFmt       = BVDC_P_CAP_PIXEL_FORMAT_8BIT422;
        pAnr->bBypassFilter = false;


        *phAnr = pAnr;
    }
    else
    {
        eResult = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        *phAnr = NULL;
    }

    BDBG_LEAVE(BVDC_P_Anr_Create);
    return eResult;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Anr_Destroy
 *
 * called by BVDC_Close only
 */
BERR_Code BVDC_P_Anr_Destroy
    ( BVDC_P_Anr_Handle          hAnr )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Anr_Destroy);
    BDBG_OBJECT_ASSERT(hAnr, BVDC_ANR);

    BDBG_OBJECT_DESTROY(hAnr, BVDC_ANR);
    /* it is gone afterwards !!! */
    BKNI_Free((void*)hAnr);

    BDBG_LEAVE(BVDC_P_Anr_Destroy);
    return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Anr_Init_isr
 *
 */
void BVDC_P_Anr_Init_isr
    ( BVDC_P_Anr_Handle              hAnr,
      BVDC_Window_Handle             hWindow)
{
    uint32_t ulReg;
    BDBG_ENTER(BVDC_P_Anr_Init_isr);
    BDBG_OBJECT_ASSERT(hAnr, BVDC_ANR);

    hAnr->eWinId = hWindow->eId;
    hAnr->pstCompression = &hWindow->stMadCompression;
#if (BVDC_P_SUPPORT_MOSAIC_DEINTERLACE)
    ulReg = BREG_Read32_isr(hAnr->hRegister, BCHP_HD_ANR_MCTF_0_HW_CONFIGURATION + hAnr->ulRegOffset);
    hAnr->ulMosaicMaxChannels = BVDC_P_GET_FIELD(ulReg, HD_ANR_MCTF_0_HW_CONFIGURATION, MULTIPLE_CONTEXT);
    hAnr->ulMosaicMaxChannels += (hAnr->ulMosaicMaxChannels!=0);
#else
    BSTD_UNUSED(ulReg);
    hAnr->ulMosaicMaxChannels = 0;
#endif

    BDBG_LEAVE(BVDC_P_Anr_Init_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Anr_SetDemoMode_isr
 *
 * called by BVDC_Window_ApplyChanges to set anr demo mode
 */
BERR_Code BVDC_P_Anr_SetDemoMode_isr
    ( BVDC_P_Anr_Handle            hAnr,
      BVDC_SplitScreenMode         eDemoMode )
{
    BDBG_OBJECT_ASSERT(hAnr, BVDC_ANR);
    BDBG_ASSERT(eDemoMode <= BVDC_SplitScreenMode_eRight);
    hAnr->eDemoMode = eDemoMode;

    return BERR_TRACE(BERR_SUCCESS);
}


/***************************************************************************
 *
 * The followings are func used inside ANR/MCVP
 *
 ***************************************************************************/
/* These macros are used in BVDC_P_Anr_BuildRul_SetEnable_isr() and */
/* BVDC_P_Anr_BuildRul_StatisRead_isr() */

/* These macros are used in BVDC_P_Anr_BuildRul_SrcInit_isr() and */
/* BVDC_P_Anr_BuildRul_StatisRead_isr() */
#define BVDC_P_AndThdScl_FRAC       11

/* These macros are used in BVDC_P_Anr_BuildRul_SrcInit_isr() and */
/* BVDC_P_Anr_BuildRul_SetEnable_isr() */
#define BVDC_P_MC_ALPHA_CALC_PARAME_INIT_SETTING     0x0708010e
#define BVDC_P_NMC_ALPHA_CALC_PARAME_INIT_SETTING    0x0013014c
#define BVDC_P_MC_NMC_ALPHA_CALC_PARAME_INIT_SETTING 0x0080010e

/* These macros are used in BVDC_P_Anr_BuildRul_StatisRead_isr() */
#define NUM_NOISY_SAMPLE_THD        (9000) /*threshold for noisy samples: set to be 9000 for picture size of 720x486 or 720x480 */
#define NUMDIFF_THD                 (6000)
#define NUM_NOISY_SAMPLE_THD_BIG    (125000)

/* This function is currently shared between stand alone ANR and ANR inside MCVP */
void BVDC_P_Anr_BuildRul_SrcInit_isr
    ( BVDC_P_Anr_Handle              hAnr,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    uint32_t ulRegOffset;
    uint32_t ulHSize, ulVSize, ulPxlBufSize, ulBvbInSize;
    uint32_t ulDemoSetting;
    int  ii, ulChannelId;
    uint32_t ulNoiseSigma;
    bool     bMemSaving;
    BVDC_P_HeapNodePtr pHeapNode;
    BMMA_DeviceOffset ullBufAddr[4]; /*0: ullTopCapBufAddr, 1: ullTopVfdBufAddr, 2: ullBotCapBufAddr, 3:ullBotVfdBufAddr*/;

    BDBG_OBJECT_ASSERT(hAnr, BVDC_ANR);
    ullBufAddr[0]=ullBufAddr[1]=ullBufAddr[2]=ullBufAddr[3]=0;
    ulRegOffset = hAnr->ulRegOffset;

    ulHSize = pPicture->pAnrIn->ulWidth;
    ulVSize = pPicture->pAnrIn->ulHeight >>(pPicture->PicComRulInfo.eSrcOrigPolarity != BAVC_Polarity_eFrame);
    ulChannelId = pPicture->ulPictureIdx;

    hAnr->bEnableMaaf = BFMT_IS_SECAM(hAnr->pCurSrcInfo->pFmtInfo->eVideoFmt);
    hAnr->ulMosaicInit |= 1<<ulChannelId;

#if (BVDC_P_SUPPORT_MANR_VER <= BVDC_P_MANR_VER_5)
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_AND_0_SW_RESET, ulRegOffset, 1);  /* AND_RESET */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_AND_0_SW_RESET, ulRegOffset, 0);  /* AND_RESET */

    /* SW7445-1454 workaround */
#if (BVDC_P_SUPPORT_MANR_VER == BVDC_P_MANR_VER_5)

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_SOFT_RESET, ulRegOffset, 1);  /* MCTF_RESET */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_SOFT_RESET, ulRegOffset, 0);  /* MCTF_RESET */
#endif
#endif

    /* set STREAM_PROCESSED first for mosaic stream first */
    hAnr->ulAutoCtrlReg =
        (BCHP_FIELD_DATA(HD_ANR_MCTF_0_AUTO_CTRL, HARD_START, 1 )|
        ((BAVC_Polarity_eBotField == pPicture->PicComRulInfo.eSrcOrigPolarity) ?
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_AUTO_CTRL, PIC_TYPE, BOTTOM_FIELD) :
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_AUTO_CTRL, PIC_TYPE, FRAME_OR_TOP_FIELD)));
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
    if(pPicture->bMosaicMode)
    {
        hAnr->ulAutoCtrlReg |=
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_AUTO_CTRL, STREAM_PROCESSED, pPicture->ulPictureIdx);
    }
#endif
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_AUTO_CTRL, ulRegOffset, hAnr->ulAutoCtrlReg);

    /* AND Init  */
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_AND_0_AND_MODE, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_AND_0_AND_MODE, HD_ANR_AND_0_CONTENT_TH));
    *pList->pulCurrent++ = 0x01; /*AND_0.AND_MODE */
#if (BVDC_P_SUPPORT_MANR_VER < BVDC_P_MANR_VER_5)
    /* 8-bit mode:*/
    *pList->pulCurrent++ = 0x0000; /*AND_0.NOISE_LOWER_TH_0 '0 */
    *pList->pulCurrent++ = 0x001C; /*AND_0.NOISE_LOWER_TH_1 '28 */
    *pList->pulCurrent++ = 0x0039; /*AND_0.NOISE_LOWER_TH_2 '57 */
    *pList->pulCurrent++ = 0x0072; /*AND_0.NOISE_LOWER_TH_3 '114 */
    *pList->pulCurrent++ = 0x00E4; /*AND_0.NOISE_LOWER_TH_4 '228 */
    *pList->pulCurrent++ = 0x0037; /*AND_0.NOISE_UPPER_TH_0 '55 */
    *pList->pulCurrent++ = 0x006D; /*AND_0.NOISE_UPPER_TH_1 '109 */
    *pList->pulCurrent++ = 0x00DA; /*AND_0.NOISE_UPPER_TH_2 '218 */
    *pList->pulCurrent++ = 0x01B0; /*AND_0.NOISE_UPPER_TH_3 '432 */
    *pList->pulCurrent++ = 0x035F; /*AND_0.NOISE_UPPER_TH_4 '863 */
    if(hAnr->pCurSrcInfo->pVdcFmt->bHd)
    {
        /* HD */
        *pList->pulCurrent++ = 0x07;  /*JJW*/ /*AND_0.EDGE_TH */
        *pList->pulCurrent++ = 0x0C;  /*JJW*/ /*AND_0.CONTENT_TH '&h14& in chip sim*/
    }
    else
    {
        /* SD or ED */
        *pList->pulCurrent++ = 0x0F;  /*JJW*/ /*AND_0.EDGE_TH */
        *pList->pulCurrent++ = 0x54;  /*JJW*/ /*AND_0.CONTENT_TH '&h14& in chip sim*/
    }
#else
    /*10-bit mode:*/
    *pList->pulCurrent++ = 0x0000; /*AND_0.NOISE_LOWER_TH_0 '0*/
    *pList->pulCurrent++ = 0x0068; /*AND_0.NOISE_LOWER_TH_1 '105*/
    *pList->pulCurrent++ = 0x00D0; /*AND_0.NOISE_LOWER_TH_2 '209*/
    *pList->pulCurrent++ = 0x01A4; /*AND_0.NOISE_LOWER_TH_3 '418*/
    *pList->pulCurrent++ = 0x0344; /*AND_0.NOISE_LOWER_TH_4 '834*/
    *pList->pulCurrent++ = 0x00DC; /*AND_0.NOISE_UPPER_TH_0 '224*/
    *pList->pulCurrent++ = 0x01B4; /*AND_0.NOISE_UPPER_TH_1 '447*/
    *pList->pulCurrent++ = 0x037C; /*AND_0.NOISE_UPPER_TH_2 '892*/
    *pList->pulCurrent++ = 0x06F4; /*AND_0.NOISE_UPPER_TH_3 '1781*/
    *pList->pulCurrent++ = 0x0DE0; /*AND_0.NOISE_UPPER_TH_4 '3552*/
    if(hAnr->pCurSrcInfo->pVdcFmt->bHd)
    {
        /* HD */
        *pList->pulCurrent++ = 0x1E;  /*JJW*/ /*AND_0.EDGE_TH */
        *pList->pulCurrent++ = 0x54;  /*JJW*/ /*AND_0.CONTENT_TH '&h14& in chip sim*/
    }
    else
    {
        /* SD or ED */
        *pList->pulCurrent++ = 0x3C;  /*JJW*/ /*AND_0.EDGE_TH */
        *pList->pulCurrent++ = 0xA8;  /*JJW*/ /*AND_0.CONTENT_TH '&h14& in chip sim*/
    }
#endif

    /* MCTF init */

    ulDemoSetting =
        ((BVDC_SplitScreenMode_eDisable == hAnr->eDemoMode)?
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_CONT_0_DEMO_SETTING, CTRL, DISABLE) :
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_CONT_0_DEMO_SETTING, CTRL, ENABLE))    |
        ((BVDC_SplitScreenMode_eLeft == hAnr->eDemoMode)?
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_CONT_0_DEMO_SETTING, DEMO_L_R, LEFT) :
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_CONT_0_DEMO_SETTING, DEMO_L_R, RIGHT)) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_DEMO_SETTING, DEMO_BOUNDARY, ulHSize / 2);

    ulBvbInSize =               /* BVB_IN_SIZE */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, HSIZE, ulHSize) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, VSIZE, ulVSize);

    /* memory saving mode will share deinterlacer's memory! */
    bMemSaving = BVDC_P_MVP_USED_MAD(pPicture->stMvpMode);
    if(!bMemSaving)
    {
        bool  bInterlaced;
        bInterlaced = (BAVC_Polarity_eFrame != pPicture->PicComRulInfo.eSrcOrigPolarity);

#if ((BVDC_P_DCX_ANR_CROSS_OVER_WORKAROUND) && (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER))
        if(hAnr->stCompression.bEnable)
        {
            if(hAnr->bSplitBuf)
            {
                uint32_t   ulHeapSize;

                /* Split anr buffers */
                BVDC_P_BufferHeap_GetHeapSizeById_isr(hAnr->hHeap, hAnr->eBufHeapId, &ulHeapSize);
                ullBufAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[0]);
                ullBufAddr[1] = ullBufAddr[0] + ulHeapSize / 2;
                ullBufAddr[2] = bInterlaced
                    ? BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[1])
                    : BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[0]);
                ullBufAddr[3] = ullBufAddr[2] + ulHeapSize / 2;
            }
            else
            {
                if(bInterlaced)
                {
                    /* interlaced */
                    ullBufAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[0]);
                    ullBufAddr[1] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[1]);
                    ullBufAddr[2] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[2]);
                    ullBufAddr[3] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[3]);
                }
                else
                {
                    /* progressive */
                    ullBufAddr[0] = ullBufAddr[2] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[0]);
                    ullBufAddr[1] = ullBufAddr[3] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[2]);
                }
            }
        }
        else
#endif
        {
            pHeapNode = hAnr->apHeapNode[ulChannelId][0];
            ulPxlBufSize = hAnr->ulPxlBufSize[ulChannelId];

            if(pPicture->bContinuous)
            {
                if(bInterlaced)
                {
                    /* Interlaced: need 4 field buffers, alloc 4 field buffers */
                    ullBufAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(pHeapNode);
                    ullBufAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullBufAddr[0] + ulPxlBufSize, BVDC_P_PITCH_ALIGN);
                    ullBufAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullBufAddr[1] + ulPxlBufSize, BVDC_P_PITCH_ALIGN);
                    ullBufAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullBufAddr[2] + ulPxlBufSize, BVDC_P_PITCH_ALIGN);

                    BDBG_MSG(("anr[%d] bufsize %d interlaced top cap "BDBG_UINT64_FMT" vfd "BDBG_UINT64_FMT" bottom cap "BDBG_UINT64_FMT" vfd "BDBG_UINT64_FMT,
                        hAnr->eId, ulPxlBufSize,
                        BDBG_UINT64_ARG(ullBufAddr[0]), BDBG_UINT64_ARG(ullBufAddr[1]),
                        BDBG_UINT64_ARG(ullBufAddr[2]), BDBG_UINT64_ARG(ullBufAddr[3])));
                }
                else
                {
                    /* progressive: need 2 frame buffers, alloc 4 field buffers.
                    Use 2 field buffers as 1 frame buffer for progressive */
                    ullBufAddr[0] = ullBufAddr[2] = BVDC_P_BUFFERHEAP_GetDeviceOffset(pHeapNode);
                    ullBufAddr[1] = ullBufAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullBufAddr[0] + 2*ulPxlBufSize, BVDC_P_PITCH_ALIGN);
                    BDBG_MSG(("anr[%d] bufsize %d progressive top cap "BDBG_UINT64_FMT" vfd "BDBG_UINT64_FMT" bottom cap "BDBG_UINT64_FMT" vfd "BDBG_UINT64_FMT,
                        hAnr->eId, ulPxlBufSize,
                        BDBG_UINT64_ARG(ullBufAddr[0]), BDBG_UINT64_ARG(ullBufAddr[1]),
                        BDBG_UINT64_ARG(ullBufAddr[2]), BDBG_UINT64_ARG(ullBufAddr[3])));
                }
            }
            else
            {
                BVDC_P_HeapNodePtr pHeapNode1, pHeapNode2, pHeapNode3;
                pHeapNode1 = hAnr->apHeapNode[ulChannelId][1];
                pHeapNode2 = hAnr->apHeapNode[ulChannelId][2];
                pHeapNode3 = hAnr->apHeapNode[ulChannelId][3];
                if(bInterlaced)
                {
                    /* Interlaced: need 4 field buffers, alloc 4 field buffers */
                    ullBufAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(pHeapNode);
                    ullBufAddr[1] = BVDC_P_BUFFERHEAP_GetDeviceOffset(pHeapNode1);
                    ullBufAddr[2] = BVDC_P_BUFFERHEAP_GetDeviceOffset(pHeapNode2);
                    ullBufAddr[3] = BVDC_P_BUFFERHEAP_GetDeviceOffset(pHeapNode3);
                }
                else
                {
                    /* progressive: need 2 frame buffers, alloc 4 field buffers.
                       Use 2 field buffers as 1 frame buffer for progressive */
                    ullBufAddr[0] = ullBufAddr[2] = BVDC_P_BUFFERHEAP_GetDeviceOffset(pHeapNode);
                    ullBufAddr[1] = ullBufAddr[3] = BVDC_P_BUFFERHEAP_GetDeviceOffset(pHeapNode2);
                }
            }
        }
    }

    /* the following are coded according to mctf_regs.scr */
#if (BVDC_P_SUPPORT_MANR_VER <= BVDC_P_MANR_VER_6)
#if (BVDC_P_SUPPORT_MANR_VER < BVDC_P_MANR_VER_5)
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME));
    *pList->pulCurrent++ = 0x01d403a8;   /* MC_FILTER_COST_PARAME */
    *pList->pulCurrent++ = BVDC_P_MC_ALPHA_CALC_PARAME_INIT_SETTING;   /* MC_ALPHA_CALC_PARAME */

    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_NMC_FILTER_COST_PARAME, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_NMC_FILTER_COST_PARAME, HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD));
    *pList->pulCurrent++ = 0x01d403a8;   /* NMC_FILTER_COST_PARAME */
    *pList->pulCurrent++ = BVDC_P_NMC_ALPHA_CALC_PARAME_INIT_SETTING;   /* NMC_ALPHA_CALC_PARAME */
    *pList->pulCurrent++ = 0x00000075;   /* MV_CONFID_PARAME */
    *pList->pulCurrent++ = BVDC_P_MC_NMC_ALPHA_CALC_PARAME_INIT_SETTING;   /* MC_NMC_ALPHA_CALC_PARAME */
    *pList->pulCurrent++ = 0x0000001c;   /* ALPHA_LOW_THRESHOLD */
#else
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME));
    *pList->pulCurrent++ =               /* MC_FILTER_COST_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, B1, 0x750) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, B0, 0xEA0);
    *pList->pulCurrent++ =               /* MC_ALPHA_CALC_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME, K1, 0x708) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME, K0, 0x10E);

    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_NMC_FILTER_COST_PARAME, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_NMC_FILTER_COST_PARAME, HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD));
    *pList->pulCurrent++ =               /* NMC_FILTER_COST_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_FILTER_COST_PARAME, C1, 0x1D4) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_FILTER_COST_PARAME, C0, 0x3A8);
    *pList->pulCurrent++ =               /* NMC_ALPHA_CALC_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_ALPHA_CALC_PARAME, K1, 0x013) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_ALPHA_CALC_PARAME, K0, 0x14C);
    *pList->pulCurrent++ =               /* MV_CONFID_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, ADJ_VALUE, 0x08) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, A0,        0x75);
    *pList->pulCurrent++ =               /* MC_NMC_ALPHA_CALC_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME, K1, 0x080) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME, K0, 0x10E);
    *pList->pulCurrent++ =               /* ALPHA_LOW_THRESHOLD */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD, MC,    0x1E) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD, VALUE, 0x1C);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_ADJUST, ulRegOffset, 0x0);
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_DARKNESS_THRESHOLD, ulRegOffset, 0x60);
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST, ulRegOffset,
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST, MC,  0x1) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST, NMC, 0x0));
#endif

    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, HD_ANR_MCTF_0_CONT_0_DEMO_SETTING));
    *pList->pulCurrent++ =               /* BVB_IN_SIZE */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, HSIZE, ulHSize) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, VSIZE, ulVSize);
    *pList->pulCurrent++ = ulDemoSetting;/* DEMO_SETTING */
    if(!bMemSaving)
    {
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_FRAME_OR_TOP_MSTART_0 + ulRegOffset, ullBufAddr[0]);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_FRAME_OR_TOP_MSTART_1 + ulRegOffset, ullBufAddr[1]);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_BOTTOM_MSTART_0 + ulRegOffset, ullBufAddr[2]);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_BOTTOM_MSTART_1 + ulRegOffset, ullBufAddr[3]);
    }

    /* control registers */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_ME_CTRL, ulRegOffset, 0x3);  /* SEL_7X5 */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_CTRL, ulRegOffset, 0x3);  /* SEL_7X5 */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_NMC_CTRL, ulRegOffset, 0x7); /* SSD(?)|SEL_7X5*/

    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_VIP_OUTPUT_DRAIN, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_VIP_OUTPUT_DRAIN, HD_ANR_MCTF_0_AND_OUTPUT_DRAIN));
    *pList->pulCurrent++ = 0x0;  /* VIP_OUTPUT_DRAIN:CTRL_DISABLE */
    *pList->pulCurrent++ = 0x0;  /* SCAD_OUTPUT_DRAIN:CTRL_DISABLE */
    *pList->pulCurrent++ = 0x0;  /* AND_OUTPUT_DRAIN:CTRL_DISABLE */

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_MANUAL_CTRL, ulRegOffset, 0x0);
#else

    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST));
    *pList->pulCurrent++ =               /* MC_FILTER_COST_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, B1, 0x750) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_FILTER_COST_PARAME, B0, 0xEA0);
    *pList->pulCurrent++ = 0x7;          /* HD_ANR_MCTF_0_CONT_0_NMC_CTRL: SSD(?)|SEL_7X5 */
    *pList->pulCurrent++ =               /* NMC_FILTER_COST_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_FILTER_COST_PARAME, C1, 0x1D4) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_FILTER_COST_PARAME, C0, 0x3A8);
    *pList->pulCurrent++ =               /* MV_CONFID_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, ADJ_VALUE, 0x08) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, A0,        0x75);
    *pList->pulCurrent++ =               /* HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST, MC,  0x1) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST, NMC, 0x0);

    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME, HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME));
    *pList->pulCurrent++ =               /* MC_ALPHA_CALC_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME, K1, 0x708) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME, K0, 0x10E);
    *pList->pulCurrent++ =               /* NMC_ALPHA_CALC_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_ALPHA_CALC_PARAME, K1, 0x013) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_ALPHA_CALC_PARAME, K0, 0x14C);
    *pList->pulCurrent++ =               /* MC_NMC_ALPHA_CALC_PARAME */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME, K1, 0x080) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME, K0, 0x10E);

    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD, HD_ANR_MCTF_0_MANUAL_CTRL));
    *pList->pulCurrent++ =               /* ALPHA_LOW_THRESHOLD */
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD, MC,    0x1E) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD, VALUE, 0x1C);
    *pList->pulCurrent++ = 0x0;          /* HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_ADJUST */
    *pList->pulCurrent++ = 0x60;         /* HD_ANR_MCTF_0_CONT_0_DARKNESS_THRESHOLD */
    *pList->pulCurrent++ = 0x0;          /* HD_ANR_MCTF_0_MANUAL_CTRL */

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, ulRegOffset, ulBvbInSize);

    if(!bMemSaving)
    {
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_FRAME_OR_TOP_MSTART_0 + ulRegOffset, ullBufAddr[0]);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_FRAME_OR_TOP_MSTART_1 + ulRegOffset, ullBufAddr[1]);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_BOTTOM_MSTART_0 + ulRegOffset, ullBufAddr[2]);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_BOTTOM_MSTART_1 + ulRegOffset, ullBufAddr[3]);
    }

    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_DEMO_SETTING, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_DEMO_SETTING, HD_ANR_MCTF_0_AND_OUTPUT_DRAIN));
    *pList->pulCurrent++ = ulDemoSetting;/* DEMO_SETTING */
    *pList->pulCurrent++ = 0x0;          /* VIP_OUTPUT_DRAIN:CTRL_DISABLE */
    *pList->pulCurrent++ = 0x0;          /* AND_OUTPUT_DRAIN:CTRL_DISABLE */

    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_HD_ANR_MCTF_0_CONT_0_ME_CTRL, ulRegOffset,
        BVDC_P_REGS_ENTRIES(HD_ANR_MCTF_0_CONT_0_ME_CTRL, HD_ANR_MCTF_0_CONT_0_MC_CTRL));
    /* control registers */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_ME_CTRL, ulRegOffset, 0x3);  /* SEL_7X5 */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_CTRL, ulRegOffset, 0x3);  /* SEL_7X5 */
#endif

    hAnr->ulTopCtrlReg =
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_2)
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_TOP_CTRL, BVB_VIDEO,
            BVDC_P_VNET_USED_MVP_AT_WRITER(pPicture->stVnetMode) ?
            pPicture->eSrcOrientation : pPicture->eDispOrientation) |
#endif
#if (BCHP_HD_ANR_MCTF_0_TOP_CTRL_SCB_MODE_SEL_SHIFT)
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_TOP_CTRL, SCB_MODE_SEL,
                        (BPXL_IS_YCbCr422_10BIT_FORMAT(hAnr->ePxlFmt) ||
                         BPXL_IS_YCbCr422_10BIT_PACKED_FORMAT(hAnr->ePxlFmt)) ? 1 : 0   ) |
#endif
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_TOP_CTRL, TF_OUT_CAP,   bMemSaving ) |
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_TOP_CTRL, RANGE_CHK,    DISABLE        ) |
#if (BVDC_P_SUPPORT_MANR_VER <= BVDC_P_MANR_VER_4)
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_TOP_CTRL, UPDATE_SEL,   NORMAL        ) |
#endif
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_TOP_CTRL, CONTEXT_CTRL, AUTO          ) |
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_TOP_CTRL, ENABLE_CTRL,  STOP_ON_FIELD_COMPLETION);

#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_4)
    /* ESB will be turned on ONLY WHEN the picture size is smaller than SD */
    /* TODO: Add user selection of low/med/high setting */
    hAnr->bEsbEnable = (ulHSize < BVDC_P_ESB_WIDTH_THRES && ulVSize < BVDC_P_ESB_HEIGHT_THRES) ? true : false;
    BDBG_MSG(("ESB = %s - %s",
        hAnr->bEsbEnable ? "ON" : "OFF",
        (BAVC_Polarity_eFrame != pPicture->PicComRulInfo.eSrcOrigPolarity) ? "INTERLACE" : "PROGRESSIVE"));
    hAnr->ulEsbConfig =
        ((hAnr->bEsbEnable) ?
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG,  MC_BLEND_INPUT_SEL, SEL_ESB) :
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG,  MC_BLEND_INPUT_SEL, SEL_INPUT)) |
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG, OUTPUT_BLEND_MODE, SEL_FINAL_BLEND) |
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG, SEARCH_X, NORMAL_RANGE) |
        ((BAVC_Polarity_eFrame != pPicture->PicComRulInfo.eSrcOrigPolarity) ?
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG, SEARCH_Y, SMALL_RANGE) :
         BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG, SEARCH_Y, NORMAL_RANGE)) |
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG, SMOOTHING_ENABLE, HYBRID_MODE) |
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG, SMOOTHING_STRENGTH, MED_STR) |
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG, SYMMETRY_ENABLE, ENABLE) |
        BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ESB_CONFIG, SPOT_ENABLE, ENABLE);
    ulNoiseSigma = BCHP_FIELD_DATA(HD_ANR_MCTF_0_ESB_THR, NOISE_SIGMA, 144); /* default */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_ESB_CONFIG, ulRegOffset, hAnr->ulEsbConfig);
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_ESB_THR, ulRegOffset, ulNoiseSigma);
#else
    BSTD_UNUSED(ulNoiseSigma);
#endif

    /* AND threshold numbers, used in StatisRead */
    hAnr->ulAndThdScl = ((ulHSize * ulVSize) << BVDC_P_AndThdScl_FRAC) / (720 * 240);

    /* new alg */
    hAnr->bMcndEnable = true;
    hAnr->bTstRepeat = true;
    hAnr->bIPulsingReduceEnable = true;
    hAnr->bFixMcCurve = false;
    hAnr->ulMinMethod = 0; /* 0=> From instantaneous snr , 1=> from avg. snr , curr = 0 */
    hAnr->ulFilterUpdate = 1;  /* 0=> Using snr_avg ,  1=> Using the min of snr_inst or snr_avg (depending upon the MIN_METHOD) , */
                               /* 2=> Using (snr_avg-snr_delta_avg), curr = 1 */
    hAnr->ulRndCeil = 3;   /* 2=> round , 3=> ceil , default = 3 */

    hAnr->bInitializeArray = true;
    for(ii = 0; ii < BVDC_P_LONG_ARRAY_LEN; ii++)
    {
        hAnr->alDeltaArray[ii] = 999;
        hAnr->alLongArray[ii] = 0;
    }
    BSTD_UNUSED(ulBvbInSize);
    return;
}

#if (BVDC_P_SUPPORT_MOSAIC_DEINTERLACE)
static void BVDC_P_Anr_BuildRul_Mosaic_isr
    ( BVDC_P_Anr_Handle              hAnr,
      BVDC_P_PictureNode            *pPicture,
      BVDC_P_ListInfo               *pList)
{
    uint32_t ulHSize, ulVSize, ulBufSize;
    uint32_t ulChannelId;
    bool     bInterlace = (BAVC_Polarity_eFrame != pPicture->eSrcPolarity), bMemSaving;
    uint32_t ulRegOffset = hAnr->ulRegOffset;
    BMMA_DeviceOffset ullBufAddr[4]; /*0: ullTopCapBufAddr, 1: ullTopVfdBufAddr, 2: ullBotCapBufAddr, 3:ullBotVfdBufAddr*/;
    ulHSize = pPicture->pAnrIn->ulWidth;
    ulVSize = pPicture->pAnrIn->ulHeight >>(pPicture->PicComRulInfo.eSrcOrigPolarity != BAVC_Polarity_eFrame);
    bMemSaving = BVDC_P_MVP_USED_MAD(pPicture->stMvpMode);
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, ulRegOffset,
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, HSIZE, ulHSize) |
        BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_BVB_IN_SIZE, VSIZE, ulVSize));
    if(!bMemSaving)
    {
        ulChannelId = pPicture->ulPictureIdx;
        ulBufSize   = hAnr->ulPxlBufSize[ulChannelId];
        if(bInterlace)
        {
            ullBufAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[ulChannelId][0]);
            ullBufAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullBufAddr[0] + ulBufSize, BVDC_P_PITCH_ALIGN);
            ullBufAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullBufAddr[1] + ulBufSize, BVDC_P_PITCH_ALIGN);
            ullBufAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullBufAddr[2] + ulBufSize, BVDC_P_PITCH_ALIGN);
        }
        else
        {
            ullBufAddr[0] = ullBufAddr[2] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hAnr->apHeapNode[ulChannelId][0]);
            ullBufAddr[1] = ullBufAddr[3] = BVDC_P_ADDR_ALIGN_UP (ullBufAddr[0] + 2*ulBufSize, BVDC_P_PITCH_ALIGN);
        }

        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_FRAME_OR_TOP_MSTART_0 + ulRegOffset, ullBufAddr[0]);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_FRAME_OR_TOP_MSTART_1 + ulRegOffset, ullBufAddr[1]);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_BOTTOM_MSTART_0 + ulRegOffset, ullBufAddr[2]);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_HD_ANR_MCTF_0_CONT_0_BOTTOM_MSTART_1 + ulRegOffset, ullBufAddr[3]);
    }
}
#endif
void BVDC_P_Anr_BuildRul_SetEnable_isr
    ( BVDC_P_Anr_Handle              hAnr,
      BVDC_P_PictureNode            *pPicture,
      BVDC_P_ListInfo               *pList,
      bool                           bEnable)
{
    uint32_t  ulRegOffset;
    uint32_t  ulTopCtrl, ulCap;

    BDBG_OBJECT_ASSERT(hAnr, BVDC_ANR);
    ulRegOffset = hAnr->ulRegOffset;
    ulTopCtrl = hAnr->ulTopCtrlReg;

    if ((bEnable) && (BVDC_FilterMode_eBypass != hAnr->pAnrSetting->eMode))
    {
        bool bMemSaving, bInitAnr, bRepeat;
        uint32_t ulChannelId = pPicture->ulPictureIdx;
        BAVC_Polarity    eSrcNxtFldId = pPicture->PicComRulInfo.eSrcOrigPolarity;

        bMemSaving = BVDC_P_MVP_USED_MAD(pPicture->stMvpMode);

        bInitAnr = (hAnr->ulMosaicInit>>ulChannelId) & 1;
        hAnr->ulMosaicInit &=~(1<< ulChannelId);

        /* ulAutoCtrlReg is also saved for later used */
        if(!bInitAnr)
        {
            hAnr->ulAutoCtrlReg =
                 BCHP_FIELD_DATA(HD_ANR_MCTF_0_AUTO_CTRL, HARD_START, 0 )|
                ((BAVC_Polarity_eBotField == eSrcNxtFldId) ?
                 BCHP_FIELD_ENUM(HD_ANR_MCTF_0_AUTO_CTRL, PIC_TYPE, BOTTOM_FIELD) :
                 BCHP_FIELD_ENUM(HD_ANR_MCTF_0_AUTO_CTRL, PIC_TYPE, FRAME_OR_TOP_FIELD) );
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
            if(pPicture->bMosaicMode)
            {
                hAnr->ulAutoCtrlReg |=
                    BCHP_FIELD_DATA(HD_ANR_MCTF_0_AUTO_CTRL, STREAM_PROCESSED, pPicture->ulPictureIdx);
            }
#endif
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_AUTO_CTRL, ulRegOffset, hAnr->ulAutoCtrlReg);
#if (BVDC_P_SUPPORT_MOSAIC_DEINTERLACE)
            if(pPicture->bMosaicMode)
                BVDC_P_Anr_BuildRul_Mosaic_isr(hAnr, pPicture, pList);
#endif

        }
        BVDC_P_Anr_BuildRul_StatisRead_isr(hAnr, pList);
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_MAD_OUT1_DRAIN, ulRegOffset,
            ((bMemSaving)?
             BCHP_FIELD_ENUM(HD_ANR_MCTF_0_MAD_OUT1_DRAIN, CTRL, DISABLE) :
             BCHP_FIELD_ENUM(HD_ANR_MCTF_0_MAD_OUT1_DRAIN, CTRL, ENABLE)));

        /* PR48136: ANR bypass workaround - tune off filter effect by setting blendor factor K1 as 0 */
        if(BVDC_FilterMode_eBypass == hAnr->pAnrSetting->eMode)
        {
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME, ulRegOffset,
                BVDC_P_MC_ALPHA_CALC_PARAME_INIT_SETTING &
                (~(BCHP_MASK(HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME, K1)) ));
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_NMC_ALPHA_CALC_PARAME, ulRegOffset,
                BVDC_P_NMC_ALPHA_CALC_PARAME_INIT_SETTING &
                (~(BCHP_MASK(HD_ANR_MCTF_0_CONT_0_NMC_ALPHA_CALC_PARAME, K1)) ));
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME, ulRegOffset,
                BVDC_P_MC_NMC_ALPHA_CALC_PARAME_INIT_SETTING &
                (~(BCHP_MASK(HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME, K1)) ));
        }

#if (HW3548_47_IS_FIXED)
        /* mctf trick mode suport */
        bRepeat = pPicture->stFlags.bPictureRepeatFlag;
        /*anr capture is set via (1 && (2||3))*/
        /* 1. mcdi is off && */
        /* 2. hardstart (bInitAnr) ||*/
        /* 3. non repeat picture  */
        ulCap = (!bMemSaving)&&((!bRepeat) || bInitAnr);

        ulTopCtrl =
            hAnr->ulTopCtrlReg & ~(BCHP_MASK(HD_ANR_MCTF_0_TOP_CTRL, TF_OUT_CAP));
        ulTopCtrl |=
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_TOP_CTRL, TF_OUT_CAP, ulCap);
#else
    BSTD_UNUSED(ulCap);
    BSTD_UNUSED(bRepeat);
#endif
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_TOP_CTRL, ulRegOffset, ulTopCtrl);
        hAnr->ulTopCtrlReg = ulTopCtrl;

        /* AND */
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_AND_0_AND_ENABLE, ulRegOffset, 0x1); /* enable */
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_ENABLE, ulRegOffset,
            BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ENABLE, ENABLE, ON));


#ifdef BCHP_HD_ANR_0_DOWNSAMPLE_CTL
        /* 444 <-> 422 */
        BDBG_OBJECT_ASSERT(pPicture->hBuffer->hWindow, BVDC_WIN);
        BDBG_OBJECT_ASSERT(pPicture->hBuffer->hWindow->stCurInfo.hSource, BVDC_SRC);

        if(pPicture->hBuffer->hWindow->stCurInfo.hSource->bSrcIs444)
        {
            if(BVDC_P_VNET_USED_MVP_AT_WRITER(pPicture->stVnetMode))
            {
                /* SRC -> ANR */
                hAnr->stDnSampler.eFilterType = BVDC_444To422Filter_eStandard;
            }
            else
            {
                /* SRC -> CAP -> VFD -> ANR */
                if(pPicture->hBuffer->hWindow->stCurResource.hCapture->eCapDataMode == BVDC_P_Capture_DataMode_e10Bit444)
                {
                    hAnr->stDnSampler.eFilterType = BVDC_444To422Filter_eStandard;
                }
                else
                {
                    hAnr->stDnSampler.eFilterType = BVDC_444To422Filter_eDecimate;
                }
            }
        }
        else
        {
            hAnr->stDnSampler.eFilterType = BVDC_444To422Filter_eDecimate;
        }

        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_0_DOWNSAMPLE_CTL, ulRegOffset,
            BCHP_FIELD_DATA(HD_ANR_0_DOWNSAMPLE_CTL, FILTER_TYPE,
                hAnr->stDnSampler.eFilterType) |
            ((BVDC_444To422Filter_eStandard == hAnr->stDnSampler.eFilterType)
            ? BCHP_FIELD_ENUM(HD_ANR_0_DOWNSAMPLE_CTL, RING_SUPPRESSION, ENABLE)
            : BCHP_FIELD_ENUM(HD_ANR_0_DOWNSAMPLE_CTL, RING_SUPPRESSION, DISABLE)));

        if((hAnr->stUpSampler.eFilterType == BVDC_422To444Filter_eTenTaps) ||
           (hAnr->stUpSampler.eFilterType == BVDC_422To444Filter_eSixTaps))
        {
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_0_UPSAMPLE_CTL, ulRegOffset,
                BCHP_FIELD_ENUM(HD_ANR_0_UPSAMPLE_CTL, RING_SUPPION_MODE, DOUBLE) |
                BCHP_FIELD_ENUM(HD_ANR_0_UPSAMPLE_CTL, RING_SUPPION,      ENABLE) |
                BCHP_FIELD_DATA(HD_ANR_0_UPSAMPLE_CTL, UNBIASED_ROUND_ENABLE,
                    hAnr->stUpSampler.bUnbiasedRound) |
                BCHP_FIELD_DATA(HD_ANR_0_UPSAMPLE_CTL, FILT_CTRL,
                    hAnr->stUpSampler.eFilterType));
        }
        else
        {
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_0_UPSAMPLE_CTL, ulRegOffset,
                BCHP_FIELD_ENUM(HD_ANR_0_UPSAMPLE_CTL, RING_SUPPION_MODE, NORMAL)  |
                BCHP_FIELD_ENUM(HD_ANR_0_UPSAMPLE_CTL, RING_SUPPION,      DISABLE) |
                BCHP_FIELD_DATA(HD_ANR_0_UPSAMPLE_CTL, UNBIASED_ROUND_ENABLE,
                    hAnr->stUpSampler.bUnbiasedRound) |
                BCHP_FIELD_DATA(HD_ANR_0_UPSAMPLE_CTL, FILT_CTRL,
                    hAnr->stUpSampler.eFilterType));
        }
#else
    BSTD_UNUSED(pPicture);
#endif

    }
    else
    {
#ifdef BCHP_HD_ANR_0_HD_ANR_CTRL
        /* no double buffer control; disable nosie detection; bypass filter */
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_0_HD_ANR_CTRL, ulRegOffset,
            BCHP_FIELD_ENUM(HD_ANR_0_HD_ANR_CTRL, UPDATE_SEL, UPDATE_BY_PICTURE   ) | /* 1 is disabled */
            BCHP_FIELD_ENUM(HD_ANR_0_HD_ANR_CTRL, BYPASS_MCTF_DATA_ENABLE,   DISABLE) |
            BCHP_FIELD_ENUM(HD_ANR_0_HD_ANR_CTRL, BYPASS_OUTPUT_DATA_ENABLE, ENABLE ) |
            BCHP_FIELD_ENUM(HD_ANR_0_HD_ANR_CTRL, HD_ANR_ENABLE, BYPASS));
#endif

        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_ENABLE, ulRegOffset,
            BCHP_FIELD_ENUM(HD_ANR_MCTF_0_ENABLE, ENABLE, OFF));
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_AND_0_AND_ENABLE, ulRegOffset, 0x2);  /* drain */

        /* next pic will do hard-start if it changes to not bypass */
    }
}


/***************************************************************************
 * {private}
 *
 * BVDC_P_Anr_BuildRul_StatisRead_isr
 *
 * It is called by BVDC_P_Anr_BuildRul_isr to read statistics HW registers
 * and then to dynamically adjust the filter strength.
 *
 * This happens at every multiple of BVDC_P_ANR_NUM_ACCUM fields or frames.
 *
 *'Noise Lower/Upper Thresholds are set in ANR_AND_VPP.bss
 * user_adj = InputBox("Filter Strength Adjustment:" & vbLF & "0: Normal" &
 * vbLF & "+1: Stronger Filter" & vbLF & "-1: Weaker Filter", "ANR Detection &
 * Filtering", 0)
 *
 */
void BVDC_P_Anr_BuildRul_StatisRead_isr
    ( BVDC_P_Anr_Handle              hAnr,
      BVDC_P_ListInfo               *pList)
{
    BREG_Handle  hRegister;
    uint32_t  ulRegOffset;
    uint32_t  aulNoisySample[BVDC_P_NOISE_LEVELS];             /*number of noisy samples */
    uint32_t  aulNoiseLevelMsb[BVDC_P_NOISE_LEVELS];           /* noise level msb of noise ranges 0 to 4 */
    uint32_t  aulNoiseLevelLsb[BVDC_P_NOISE_LEVELS];           /* noise level lsb of noise ranges 0 to 4 */
    uint32_t  ulNumNoisySampleThdBig, ulNumDiffThd, ulNumNoisySampleThd, ulAndThdScl;
    BVDC_P_AnrKValue  stKValue;

    BKNI_Memset((void*)&stKValue, 0, sizeof(BVDC_P_AnrKValue));

    BDBG_OBJECT_ASSERT(hAnr, BVDC_ANR);

    /* ulAndThdScl is less than ((1920 * 1088) << 11) / (720 * 240) = 0x60B6 */
    ulAndThdScl = hAnr->ulAndThdScl;
    ulNumNoisySampleThd = (NUM_NOISY_SAMPLE_THD * ulAndThdScl) >> BVDC_P_AndThdScl_FRAC;
    ulNumNoisySampleThdBig = (NUM_NOISY_SAMPLE_THD_BIG * ulAndThdScl) >> BVDC_P_AndThdScl_FRAC;
    ulNumDiffThd = (NUMDIFF_THD * ulAndThdScl) >> BVDC_P_AndThdScl_FRAC;
    BDBG_P_ANR_MSG(("ulAndThdScl 0x%x.%03lx", ulAndThdScl >> BVDC_P_AndThdScl_FRAC,
                    ulAndThdScl & ((1 << BVDC_P_AndThdScl_FRAC) - 1)));

    hRegister = hAnr->hRegister;
    ulRegOffset = hAnr->ulRegOffset;

    /* Read and keep the BVB status to be extracted and used by other modules */
    hAnr->ulBvbStatus = BREG_Read32( hRegister, BCHP_HD_ANR_MCTF_0_BVB_IN_STATUS + ulRegOffset);

    /* collect values of noisy samples and noise levels so that they dont change during script */
    aulNoisySample[0] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_0 + ulRegOffset );
    aulNoisySample[1] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_1 + ulRegOffset );
    aulNoisySample[2] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_2 + ulRegOffset );
    aulNoisySample[3] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_3 + ulRegOffset );
    aulNoisySample[4] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_4 + ulRegOffset );
    aulNoiseLevelMsb[0] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_MSB + ulRegOffset );
    aulNoiseLevelMsb[1] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_MSB + ulRegOffset );
    aulNoiseLevelMsb[2] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_MSB + ulRegOffset );
    aulNoiseLevelMsb[3] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_MSB + ulRegOffset );
    aulNoiseLevelMsb[4] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_MSB + ulRegOffset );
    aulNoiseLevelLsb[0] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_LSB + ulRegOffset );
    aulNoiseLevelLsb[1] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_LSB + ulRegOffset );
    aulNoiseLevelLsb[2] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_LSB + ulRegOffset );
    aulNoiseLevelLsb[3] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_LSB + ulRegOffset );
    aulNoiseLevelLsb[4] = BREG_Read32( hRegister, BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_LSB + ulRegOffset );

    /*BDBG_MSG(("%d %d %d %d %d - %d %d %d %d %d - %d %d %d %d %d",
        aulNoisySample[0], aulNoisySample[1], aulNoisySample[2], aulNoisySample[3], aulNoisySample[4],
        aulNoiseLevelMsb[0], aulNoiseLevelMsb[1], aulNoiseLevelMsb[2], aulNoiseLevelMsb[3], aulNoiseLevelMsb[4],
        aulNoiseLevelLsb[0], aulNoiseLevelLsb[1], aulNoiseLevelLsb[2], aulNoiseLevelLsb[3], aulNoiseLevelLsb[4]));*/

    /* New algorithm */
    hAnr->ulNumNoisySampleThd = ulNumNoisySampleThd;
    hAnr->ulNumNoisySampleThdBig = ulNumNoisySampleThdBig;
    hAnr->ulNumDiffThd = ulNumDiffThd;

    BVDC_P_Anr_McndReview_isr(hAnr, aulNoisySample, aulNoiseLevelMsb, aulNoiseLevelLsb, &stKValue);

    if(stKValue.bBypassFilter == false)
    {
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_4)
        uint32_t ulEsbConfig;
        uint32_t ulEsbOutputBlendMode =
            (hAnr->bEsbEnable) ? 1 :
            (stKValue.ulMctfSetting == 0 || stKValue.ulMctfSetting == 1) ? 0 : 1;
        uint32_t ulEsbMcBlendInputSel =
            (hAnr->bEsbEnable) ? 1 :
            (stKValue.ulMctfSetting == 0) ? 0 : 1;
        uint32_t ulEsbSmoothingEn = (stKValue.ulMctfSetting == 0 || stKValue.ulMctfSetting == 1) ? 0 : 1;
#endif

        BDBG_MSG(("iSnDbAdjust=%d MTCF Setting %d: %d %d %d %d %d %d %d %d %d %d",
            hAnr->pAnrSetting->iSnDbAdjust, stKValue.ulMctfSetting,
            stKValue.ulMcK0, stKValue.ulMcK1,
            stKValue.ulNonMcK0, stKValue.ulNonMcK1,
            stKValue.ulFinalK0, stKValue.ulFinalK1,
            stKValue.ulAlphLowThdNMC, stKValue.ulAlphLowThdMC,
            stKValue.ulMcAdj, stKValue.ulNonMcAdj));

        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD, ulRegOffset,
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD, MC,    stKValue.ulAlphLowThdMC) |
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_ALPHA_LOW_THRESHOLD, VALUE, stKValue.ulAlphLowThdNMC));
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME, ulRegOffset,
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME,     K0, stKValue.ulMcK0) |
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_ALPHA_CALC_PARAME,     K1, stKValue.ulMcK1));
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_NMC_ALPHA_CALC_PARAME, ulRegOffset,
                BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_ALPHA_CALC_PARAME,    K0, stKValue.ulNonMcK0) |
                BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_ALPHA_CALC_PARAME,    K1, stKValue.ulNonMcK1));
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_2)
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_CH_ALPHA_CALC_PARAME, ulRegOffset,
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_CH_ALPHA_CALC_PARAME,     K0, stKValue.ulMcK0_CH) |
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_CH_ALPHA_CALC_PARAME,     K1, stKValue.ulMcK1_CH));
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_NMC_CH_ALPHA_CALC_PARAME, ulRegOffset,
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_CH_ALPHA_CALC_PARAME,    K0, stKValue.ulNonMcK0_CH) |
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_NMC_CH_ALPHA_CALC_PARAME,    K1, stKValue.ulNonMcK1_CH));
#endif
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME, ulRegOffset,
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME, K0, stKValue.ulFinalK0) |
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MC_NMC_ALPHA_CALC_PARAME, K1, stKValue.ulFinalK1));
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST, ulRegOffset,
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST, MC,  stKValue.ulMcAdj) |
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_WIN_COST_ADJUST, NMC, stKValue.ulNonMcAdj));

#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_4)
        ulEsbConfig = hAnr->ulEsbConfig & ~(
            BCHP_MASK(HD_ANR_MCTF_0_ESB_CONFIG, OUTPUT_BLEND_MODE)  |
            BCHP_MASK(HD_ANR_MCTF_0_ESB_CONFIG, MC_BLEND_INPUT_SEL) |
            BCHP_MASK(HD_ANR_MCTF_0_ESB_CONFIG, SMOOTHING_ENABLE));
        ulEsbConfig |= (
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_ESB_CONFIG, OUTPUT_BLEND_MODE,  ulEsbOutputBlendMode) |
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_ESB_CONFIG, MC_BLEND_INPUT_SEL, ulEsbMcBlendInputSel) |
            BCHP_FIELD_DATA(HD_ANR_MCTF_0_ESB_CONFIG, SMOOTHING_ENABLE,   ulEsbSmoothingEn    ));
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_ESB_CONFIG, ulRegOffset, ulEsbConfig);
        hAnr->ulEsbConfig = ulEsbConfig;
#endif

        if(stKValue.ulMctfSetting == 5)
        {
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, ulRegOffset,
                BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, ADJ_VALUE, 0x06) |
                BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, A0,        0x75));
        }
        else
        {
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, ulRegOffset,
                BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, ADJ_VALUE, 0x08) |
                BCHP_FIELD_DATA(HD_ANR_MCTF_0_CONT_0_MV_CONFID_PARAME, A0,        0x75));
        }

    }

    return;
}

#endif  /* #if (BVDC_P_SUPPORT_MANR) */

/* End of file. */
