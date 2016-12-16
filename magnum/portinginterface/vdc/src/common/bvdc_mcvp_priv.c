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
#include "bvdc_buffer_priv.h"
#include "bvdc_mcvp_priv.h"
#include "bvdc_hscaler_priv.h"
#include "bvdc_anr_priv.h"
#include "bvdc_mcdi_priv.h"
#include "bchp_mmisc.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_compositor_priv.h"

#if (BVDC_P_SUPPORT_DMISC)
#include "bchp_dmisc.h"
#endif

#include "bchp_mvp_top_0.h"

BDBG_MODULE(BVDC_MCVP);
BDBG_FILE_MODULE(BVDC_DEINTERLACER_MOSAIC);
BDBG_FILE_MODULE(BVDC_WIN_BUF);
BDBG_FILE_MODULE(BVDC_DITHER);
BDBG_OBJECT_ID(BVDC_MVP);


/* SW7366-343 No MADR core SW_INIT due to wiring error */
#if (((BCHP_CHIP==7435) && (BCHP_VER==BCHP_VER_B0)) || ((BCHP_CHIP==7366) &&(BCHP_VER>=BCHP_VER_B0)))
#define BVDC_P_NO_MADR_SWINIT          (1)
#else
#define BVDC_P_NO_MADR_SWINIT          (0)
#endif

#include "bchp_siob_0.h"
#if (!BVDC_P_SUPPORT_MCDI_SUPERSET)
#include "bchp_siob_1.h"
#endif



/***************************************************************************
 * The followings are exported to other sub-modules inside VDC
 ***************************************************************************/
#if (BVDC_P_SUPPORT_MCVP_VER > BVDC_P_MCVP_VER_3)
#if (BVDC_P_SUPPORT_DMISC)
#define BVDC_P_MAKE_MVP(pMcvp, id, channel_init)                                  \
{                                                                                 \
    (pMcvp)->ulCoreResetAddr = BCHP_DMISC_SW_INIT;                                \
    (pMcvp)->ulCoreResetMask = BCHP_DMISC_SW_INIT_MVP_##id##_MASK;;               \
    (pMcvp)->ulVnetResetAddr = BCHP_##channel_init;                               \
    (pMcvp)->ulVnetResetMask = BCHP_##channel_init##_MVP_##id##_MASK;             \
    (pMcvp)->ulVnetMuxAddr   = BCHP_VNET_F_MVP_##id##_SRC;                        \
    (pMcvp)->ulVnetMuxValue  = BCHP_VNET_B_CAP_0_SRC_SOURCE_MVP_##id;             \
    (pMcvp)->ulRegOffset     = BCHP_MVP_TOP_##id##_REG_START - BCHP_MVP_TOP_0_REG_START;  \
}
#else
#define BVDC_P_MAKE_MVP(pMcvp, id, channel_init)                                  \
{                                                                                 \
    (pMcvp)->ulCoreResetAddr = BCHP_MMISC_SW_INIT;                                \
    (pMcvp)->ulCoreResetMask = BCHP_MMISC_SW_INIT_MVP_##id##_MASK;;               \
    (pMcvp)->ulVnetResetAddr = BCHP_##channel_init;                               \
    (pMcvp)->ulVnetResetMask = BCHP_##channel_init##_MVP_##id##_MASK;             \
    (pMcvp)->ulVnetMuxAddr   = BCHP_VNET_F_MVP_##id##_SRC;                        \
    (pMcvp)->ulVnetMuxValue  = BCHP_VNET_B_CAP_0_SRC_SOURCE_MVP_##id;             \
    (pMcvp)->ulRegOffset     = BCHP_MVP_TOP_##id##_REG_START - BCHP_MVP_TOP_0_REG_START;  \
}
#endif
#else  /*(BVDC_P_SUPPORT_MCVP_VER > BVDC_P_MCVP_VER_1)  beyond 7420 */
#define BVDC_P_MAKE_MVP(pMcvp, id, channel_init)                                  \
{                                                                                 \
    (pMcvp)->ulCoreResetAddr = BCHP_MMISC_SW_INIT;                                \
    (pMcvp)->ulCoreResetMask = BCHP_MMISC_SW_INIT_MVP_##id##_MASK;                \
    (pMcvp)->ulVnetResetAddr = BCHP_##channel_init;                               \
    (pMcvp)->ulVnetResetMask = BCHP_##channel_init##_MVP_##id##_MASK;             \
    (pMcvp)->ulVnetMuxAddr   = BCHP_VNET_F_MVP_##id##_SRC;                        \
    (pMcvp)->ulVnetMuxValue  = BCHP_VNET_B_CAP_0_SRC_SOURCE_MVP_##id;             \
    (pMcvp)->ulRegOffset     = BCHP_MVP_TOP_##id##_REG_START - BCHP_MVP_TOP_0_REG_START;  \
}
#endif

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_Create
*
* called by BVDC_Open only
*/
BERR_Code BVDC_P_Mcvp_Create
    ( BVDC_P_Mcvp_Handle *             phMcvp,
      BVDC_P_McvpId                    eMvpId,
      BREG_Handle                      hRegister,
      BVDC_P_Resource_Handle           hResource )
{
    BVDC_P_McvpContext *pMcvp;
    uint32_t ulReg = 0;
    BERR_Code eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Mcvp_Create);

    /* in case creation failed */
    BDBG_ASSERT(phMcvp);
    *phMcvp = NULL;

    /* Ascertain that VDC MCVP enum and count complies to BOX's */
    BDBG_CASSERT(BBOX_VDC_DEINTERLACER_COUNT >= BVDC_P_SUPPORT_MCVP);
    BDBG_CASSERT(BBOX_Vdc_Deinterlacer_eDeinterlacer5 == (BBOX_Vdc_DeinterlacerId)BVDC_P_MvpId_eMvp5);

    pMcvp = (BVDC_P_McvpContext *) (BKNI_Malloc(sizeof(BVDC_P_McvpContext)));
    if( pMcvp )
    {
        /* init the context */
        BKNI_Memset((void*)pMcvp, 0x0, sizeof(BVDC_P_McvpContext));
        BDBG_OBJECT_SET(pMcvp, BVDC_MVP);
        pMcvp->eId = eMvpId;
        pMcvp->hRegister = hRegister;
        pMcvp->hWindow = NULL;
        pMcvp->ulMaxWidth  = BFMT_1080I_WIDTH;
        pMcvp->ulMaxHeight = BFMT_1080I_HEIGHT;
        pMcvp->ulHsclSizeThreshold = BVDC_P_MAD_SRC_HORZ_THRESHOLD;
        pMcvp->ePqEnhancement = BVDC_Mode_eAuto;


        switch(pMcvp->eId)
        {
        case BVDC_P_MvpId_eMvp0:
            BVDC_P_MAKE_MVP(pMcvp, 0, MMISC_VNET_B_CHANNEL_SW_INIT);
            pMcvp->ulRegOffset1 = 0;
            break;
#if (BVDC_P_SUPPORT_MCVP > 1)
        case BVDC_P_MvpId_eMvp1:
            BVDC_P_MAKE_MVP(pMcvp, 1, MMISC_VNET_B_CHANNEL_SW_INIT);
            pMcvp->ulRegOffset1 = 0;
            break;
#endif
#if (BVDC_P_SUPPORT_MCVP > 2)
        case BVDC_P_MvpId_eMvp2:
            BVDC_P_MAKE_MVP(pMcvp, 2, MMISC_VNET_B_CHANNEL_SW_INIT);
            pMcvp->ulRegOffset1 = BCHP_MVP_TOP_2_REG_START - BCHP_MVP_TOP_1_REG_START;
            break;
#endif
#if (BVDC_P_SUPPORT_MCVP > 3)
        case BVDC_P_MvpId_eMvp3:
            BVDC_P_MAKE_MVP(pMcvp, 3, MMISC_VNET_B_CHANNEL_SW_INIT);
            pMcvp->ulRegOffset1 = BCHP_MVP_TOP_3_REG_START - BCHP_MVP_TOP_1_REG_START;
            break;
#endif
#if (BVDC_P_SUPPORT_MCVP > 4)
        case BVDC_P_MvpId_eMvp4:
            BVDC_P_MAKE_MVP(pMcvp, 4, MMISC_VNET_B_CHANNEL_SW_INIT);
            pMcvp->ulRegOffset1 = BCHP_MVP_TOP_4_REG_START - BCHP_MVP_TOP_1_REG_START;
            break;
#endif
#if (BVDC_P_SUPPORT_MCVP > 5)
        case BVDC_P_MvpId_eMvp5:
            BVDC_P_MAKE_MVP(pMcvp, 5, MMISC_VNET_B_CHANNEL_SW_INIT_1);
            pMcvp->ulRegOffset1 = BCHP_MVP_TOP_5_REG_START - BCHP_MVP_TOP_1_REG_START;
            break;
#endif
        default:
            BDBG_ERR(("Need to handle BVDC_P_McvpId_eMcvp%d", pMcvp->eId));
            BDBG_ASSERT(0);
            break;
        }

        /* init the SubRul sub-module */
        BVDC_P_SubRul_Init(&(pMcvp->SubRul), BVDC_P_Mcvp_MuxAddr(pMcvp),
            BVDC_P_Mcvp_PostMuxValue(pMcvp), BVDC_P_DrainMode_eBack,
            0, hResource);

        /* Initialized HW capabilities specific */
#ifdef BCHP_MVP_TOP_0_HW_CONFIGURATION
        ulReg = BREG_Read32(pMcvp->hRegister, BCHP_MVP_TOP_0_HW_CONFIGURATION + pMcvp->ulRegOffset);
        pMcvp->bAnr = BCHP_GET_FIELD_DATA(ulReg, MVP_TOP_0_HW_CONFIGURATION, ANR) ? true : false;
        pMcvp->eDcxCore = (BVDC_P_MvpDcxCore)BCHP_GET_FIELD_DATA(ulReg, MVP_TOP_0_HW_CONFIGURATION, SIOB);
#elif  BCHP_MVP_TOP_0_HW_CONFIG
        ulReg = BREG_Read32(pMcvp->hRegister, BCHP_MVP_TOP_0_HW_CONFIG + pMcvp->ulRegOffset);
        pMcvp->bAnr = BCHP_GET_FIELD_DATA(ulReg, MVP_TOP_0_HW_CONFIG, ANR) ? true : false;
        pMcvp->eDcxCore = (BVDC_P_MvpDcxCore)BCHP_GET_FIELD_DATA(ulReg, MVP_TOP_0_HW_CONFIG, SIOB);
#else
        BSTD_UNUSED(ulReg);
        pMcvp->bAnr = true;
#endif

        /* create sub-modules of MCVP */
        eResult  = BERR_TRACE(BVDC_P_Hscaler_Create(&pMcvp->hHscaler,
            (BVDC_P_HscalerId)eMvpId, hResource, hRegister));

#if (BVDC_P_SUPPORT_MANR)
        if(pMcvp->bAnr)
        {
            eResult |= BERR_TRACE(BVDC_P_Anr_Create(&pMcvp->hAnr,
                (BVDC_P_AnrId)eMvpId, hRegister, hResource));
        }
#endif
        eResult |= BERR_TRACE(BVDC_P_Mcdi_Create(&pMcvp->hMcdi,
            (BVDC_P_McdiId)eMvpId, hRegister, hResource));

        if (BERR_SUCCESS == eResult)
        {
            *phMcvp = pMcvp;
        }
        else
        {
            BVDC_P_Mcvp_Destroy(pMcvp);
        }
    }
    else
    {
        eResult = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BDBG_LEAVE(BVDC_P_Mcvp_Create);
    return eResult;
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_Destroy
*
* called by BVDC_Close only
*/
BERR_Code BVDC_P_Mcvp_Destroy
    ( BVDC_P_Mcvp_Handle               hMcvp )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Mcvp_Destroy);
    BDBG_OBJECT_ASSERT(hMcvp, BVDC_MVP);

    /* destroy sub-modules */
    if (hMcvp->hHscaler)
        BVDC_P_Hscaler_Destroy(hMcvp->hHscaler);
#if (BVDC_P_SUPPORT_MANR)
    if (hMcvp->hAnr)
        BVDC_P_Anr_Destroy(hMcvp->hAnr);
#endif
    if (hMcvp->hMcdi)
        BVDC_P_Mcdi_Destroy(hMcvp->hMcdi);

    BDBG_OBJECT_DESTROY(hMcvp, BVDC_MVP);
    /* it is gone afterwards !!! */
    BKNI_Free((void*)hMcvp);

    BDBG_LEAVE(BVDC_P_Mcvp_Destroy);
    return BERR_TRACE(eResult);
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_Init_isr
*
* called by BVDC_P_Mcvp_AcquireConnect_isr only
*/
static void BVDC_P_Mcvp_Init_isr
    ( BVDC_P_Mcvp_Handle               hMcvp )
{
    BBOX_Vdc_Capabilities *pBoxVdc;
    uint32_t   i;

    BDBG_ENTER(BVDC_P_Mcvp_Init_isr);
    BDBG_OBJECT_ASSERT(hMcvp, BVDC_MVP);

    pBoxVdc = &hMcvp->hHeap->hVdc->stBoxConfig.stVdc;

    if (pBoxVdc->astDeinterlacer[hMcvp->eId].stPictureLimits.ulHeight != BBOX_VDC_DISREGARD)
    {
        if (pBoxVdc->astDeinterlacer[hMcvp->eId].stPictureLimits.ulHeight > hMcvp->ulMaxHeight)
        {
            BDBG_WRN(("Box mode %d imposed height limit for MCVP %d exceeds HW limits!!!",
                hMcvp->hHeap->hVdc->stBoxConfig.stBox.ulBoxId, hMcvp->eId));
        }
        else
        {
            hMcvp->ulMaxHeight = pBoxVdc->astDeinterlacer[hMcvp->eId].stPictureLimits.ulHeight;
        }
    }

    if (pBoxVdc->astDeinterlacer[hMcvp->eId].stPictureLimits.ulWidth != BBOX_VDC_DISREGARD)
    {
        if (pBoxVdc->astDeinterlacer[hMcvp->eId].stPictureLimits.ulWidth > hMcvp->ulMaxWidth)
        {
            BDBG_WRN(("Box mode %d imposed width limit for MCVP %d exceeds HW limits!!!",
                hMcvp->hHeap->hVdc->stBoxConfig.stBox.ulBoxId, hMcvp->eId));
        }
        else
        {
            hMcvp->ulMaxWidth  = pBoxVdc->astDeinterlacer[hMcvp->eId].stPictureLimits.ulWidth;
        }
    }

    if (pBoxVdc->astDeinterlacer[hMcvp->eId].ulHsclThreshold != BBOX_VDC_DISREGARD)
    {
        hMcvp->ulHsclSizeThreshold = pBoxVdc->astDeinterlacer[hMcvp->eId].ulHsclThreshold;
    }

    hMcvp->stDirty.stBits.bCompress = BVDC_P_DIRTY;
    for(i=0; i<BAVC_MOSAIC_MAX;i++)
        hMcvp->ulUpdateAll[i] = BVDC_P_RUL_UPDATE_THRESHOLD;

    BDBG_LEAVE(BVDC_P_Mcvp_Init_isr);
    return;
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_AcquireConnect_isr
*
* It is called by BVDC_Window_Validate after changing from disable mcvp to
* enable mcvp.
*/
BERR_Code BVDC_P_Mcvp_AcquireConnect_isr
    ( BVDC_P_Mcvp_Handle                 hMcvp,
      BVDC_Heap_Handle                   hHeap,
      BVDC_Window_Handle                 hWindow)
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Mcvp_AcquireConnect_isr);

    hMcvp->hWindow = hWindow;
    hMcvp->hHeap = hHeap;

    BVDC_P_Mcvp_Init_isr(hMcvp);
    BVDC_P_Hscaler_Init_isr(hMcvp->hHscaler);
#if (BVDC_P_SUPPORT_MANR)
    if(hMcvp->hAnr)
    {
        BVDC_P_Anr_Init_isr(hMcvp->hAnr, hWindow);
    }
#endif

    BVDC_P_Mcdi_Init_isr(hMcvp->hMcdi, hWindow);

    BDBG_LEAVE(BVDC_P_Mcvp_AcquireConnect_isr);
    return BERR_TRACE(eResult);
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_ReleaseConnect_isr
*
* It is called after window decided that mcvp is no-longer used by HW in its
* vnet mode (i.e. it is really shut down and teared off from vnet).
*/
BERR_Code BVDC_P_Mcvp_ReleaseConnect_isr
    ( BVDC_P_Mcvp_Handle              *phMcvp )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Mcvp_ReleaseConnect_isr);

    /* handle validation */
    if (NULL != *phMcvp)
    {
        BDBG_OBJECT_ASSERT(*phMcvp, BVDC_MVP);

        /* another win might still using it */
        BVDC_P_Resource_ReleaseHandle_isr(
            BVDC_P_SubRul_GetResourceHandle_isr(&(*phMcvp)->SubRul),
            BVDC_P_ResourceType_eMcvp, (void *)(*phMcvp));

        /* this makes win to stop calling mcvp code */
        *phMcvp = NULL;
    }

    BDBG_LEAVE(BVDC_P_Mcvp_ReleaseConnect_isr);
    return BERR_TRACE(eResult);
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_FreeBuf_isr
*
* Called by BVDC_P_Mcvp_UnsetVnetFreeBuf_isr/BVDC_P_Mcvp_SetVnetAllocBuf_isr
* to deallocate buffers
*/

static void BVDC_P_Mcvp_FreeBuf_isr
    ( BVDC_P_Mcvp_Handle                hMcvp,
      bool                              bBypassMcvp,
      uint32_t                          ulChannelId,
      bool                              bFreeAll)
{
    uint32_t i, ulStartChannel, ulEndChannel;
    BDBG_ENTER(BVDC_P_Mcvp_FreeBuf_isr);
    BDBG_OBJECT_ASSERT(hMcvp, BVDC_MVP);

    /* free internally used buffers if no win is using this MCVP. */
    if ((0 == BVDC_P_SubRul_GetWinsActFlags(&(hMcvp->SubRul)))
        || bBypassMcvp)
    {
        BVDC_P_HeapNodePtr *ppHeapNode, *ppQmHeapNode;

        ulStartChannel = bFreeAll?0:ulChannelId;
        ulEndChannel = (bFreeAll)? BAVC_MOSAIC_MAX :(ulChannelId+1);
        /* 2. if previous was mosaic mode already, release mosaic buffer */
        for(i=ulStartChannel;i<ulEndChannel; i++)
        {
            ppHeapNode = &(hMcvp->hMcdi->apHeapNode[i][0]);
            if(NULL != *ppHeapNode)
            {
                BDBG_MODULE_MSG(BVDC_WIN_BUF, ("mvp[%d] free mosaic channel %d %d x buffers (%s)", hMcvp->eId,
                    i,
                    hMcvp->hMcdi->ulPxlBufCnt[i],
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME((*ppHeapNode)->pHeapInfo->eBufHeapId)));

                BVDC_P_BufferHeap_FreeBuffers_isr(hMcvp->hHeap,
                    ppHeapNode, hMcvp->hMcdi->ulPxlBufCnt[i], false);
                *ppHeapNode = NULL;
            }
            hMcvp->hMcdi->ulPxlBufCnt[i] = 0;
            hMcvp->hMcdi->astRect[i].ulWidth = 0 ;
            hMcvp->hMcdi->astRect[i].ulHeight = 0 ;

            ppQmHeapNode = &(hMcvp->hMcdi->apQmHeapNode[i][0]);
            if(NULL != *ppQmHeapNode)
            {
                BDBG_MODULE_MSG(BVDC_WIN_BUF, ("mvp[%d] free mosaic channel %d %d x QM buffers (%s)",
                    hMcvp->eId, i, hMcvp->hMcdi->ulQmBufCnt[i],
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME((*ppQmHeapNode)->pHeapInfo->eBufHeapId)));

                BVDC_P_BufferHeap_FreeBuffers_isr(hMcvp->hHeap, ppQmHeapNode,
                    hMcvp->hMcdi->ulQmBufCnt[i], false);
                *ppQmHeapNode = NULL;
            }

            hMcvp->hMcdi->ulQmBufCnt[i] = 0;
        }
    }

    BDBG_LEAVE(BVDC_P_Mcvp_FreeBuf_isr);
}

static void BVDC_P_Mcvp_AllocBuf_isr
    ( BVDC_P_Mcvp_Handle               hMcvp,
      BVDC_P_PictureNode               *pPicture)
{

    bool  bContinous;
    BVDC_P_MvpMode    stMvpMode;
    BVDC_P_BufferHeapId   ePixelBufHeapId, eQmBufHeapId;
    uint32_t ulPixelBufCnt, ulQmBufCnt, ulChannelId;
    BERR_Code eResult = BERR_SUCCESS;
    BVDC_P_HeapNodePtr *ppHeapNode;

    BDBG_ENTER(BVDC_P_Mcvp_AllocBuf_isr);

    /* 1. Difference comparison between new configuration and the current setting*/
    if(hMcvp->hMcdi->stSwDirty.stBits.bBuffer == BVDC_P_CLEAN)
    {
        /*BDBG_MSG(("clean bMosaic return"));*/
        return;
    }

    ulChannelId = pPicture->ulPictureIdx;
    /* 2. release the currently buffer first */
    BVDC_P_Mcvp_FreeBuf_isr(hMcvp, true, ulChannelId, false);

    bContinous = pPicture->bContinuous;

    /* 3. allocate buffer for each mosaic channel */
    ePixelBufHeapId = pPicture->eMadPixelHeapId;
    eQmBufHeapId    = pPicture->eMadQmHeapId;
    ulPixelBufCnt   = pPicture->usMadPixelBufferCnt;
    ulQmBufCnt      = pPicture->usMadQmBufCnt;
    ppHeapNode      = &(hMcvp->hMcdi->apHeapNode[ulChannelId][0]);
    stMvpMode      = pPicture->stMvpMode;

    BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC,
        ("0 Mvp[%d] allocate channel %d/%d pxl buffer %d x %s qm %d x %s",
        hMcvp->eId, ulChannelId, pPicture->ulMosaicCount,
        ulPixelBufCnt, BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(ePixelBufHeapId),
        ulQmBufCnt, BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eQmBufHeapId)));


    if(((BVDC_P_MVP_USED_MAD(stMvpMode)==true) ||
        (BVDC_P_MVP_USED_ANR(stMvpMode)==true))&&
    (NULL == hMcvp->hMcdi->apHeapNode[ulChannelId][0]))
    {
        /* 1. allocate buffer for deinterlacer pixel buffer*/
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Mvp[%d] alloc %d (%s) pixel buffers (req %s, prefer %s)",
            hMcvp->hWindow->eId, hMcvp->eId, ulPixelBufCnt,
            bContinous ? "continous" : "non-continous",
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(ePixelBufHeapId),
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(BVDC_P_BufferHeapId_eUnknown)));
        eResult = BVDC_P_BufferHeap_AllocateBuffers_isr(hMcvp->hHeap, ppHeapNode,
        ulPixelBufCnt, bContinous, ePixelBufHeapId, BVDC_P_BufferHeapId_eUnknown);
        /* Not enough memory, dump out configuration */
        if(eResult == BERR_OUT_OF_DEVICE_MEMORY)
        {
            BDBG_ERR(("Win[%d] Mvp[%d] Not enough memory for MCVP mosaic pixel field buffers! Configuration:",
                hMcvp->hWindow->eId, hMcvp->eId));
            BDBG_ERR(("Mvp[%d] mosaic[%d] pixel BufferCnt: %d",
                hMcvp->eId, ulChannelId, ulPixelBufCnt));
            BVDC_P_PRINT_BUF_DEBUG_INSTRUCTION();
            return;
        }
        hMcvp->hMcdi->ulPxlBufCnt[ulChannelId] = ulPixelBufCnt;

        /* 1.a set the right Anr node*/
        ppHeapNode = &(hMcvp->hMcdi->apHeapNode[ulChannelId][0]);
#if (BVDC_P_SUPPORT_MANR)
        if(hMcvp->bAnr)
        {
            BDBG_OBJECT_ASSERT(hMcvp->hAnr,  BVDC_ANR);
            /* cp buf node ptr to anr context so it can easily set buf addr in the case
            * that MAD is off */
            BVDC_P_Anr_SetBufNodes_isr(hMcvp->hAnr, ppHeapNode, ulChannelId);
            hMcvp->hAnr->ulPxlBufSize[ulChannelId]= hMcvp->hMcdi->ulPxlBufSize[ulChannelId];
        }
#endif

        /* 2. allocate buffer for QM buffer */
        if(ulQmBufCnt)
        {
            ppHeapNode      = &(hMcvp->hMcdi->apQmHeapNode[ulChannelId][0]);

            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Mvp[%d] alloc %d QM buffers (req %s, prefer %s)",
                hMcvp->hWindow->eId, hMcvp->eId, ulQmBufCnt,
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eQmBufHeapId),
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(BVDC_P_BufferHeapId_eUnknown)));
            eResult = BVDC_P_BufferHeap_AllocateBuffers_isr(hMcvp->hHeap, ppHeapNode,
                ulQmBufCnt, false, eQmBufHeapId, BVDC_P_BufferHeapId_eUnknown);

            /* Not enough memory, dump out configuration */
            if(eResult == BERR_OUT_OF_DEVICE_MEMORY)
            {
                BDBG_ERR(("Win[%d] Mvp[%d] Not enough memory for MCVP mosaic QM field buffers! Configuration:",
                    hMcvp->hWindow->eId, hMcvp->eId));
                BDBG_ERR(("Mvp[%d] mosaic[%d] QM BufferCnt: %d, pixel BufferCnt: %d",
                    hMcvp->eId, ulChannelId, ulQmBufCnt, ulPixelBufCnt));
                BVDC_P_PRINT_BUF_DEBUG_INSTRUCTION();
                return;
            }

            hMcvp->hMcdi->ulQmBufCnt[ulChannelId] = ulQmBufCnt;
        }

        BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC,
        ("1 Mvp[%d] allocated channel %d/%d pxl buffer %d x %s qm %d x %s",
        hMcvp->eId, ulChannelId, pPicture->ulMosaicCount,
        hMcvp->hMcdi->ulPxlBufCnt[ulChannelId], BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(ePixelBufHeapId),
        hMcvp->hMcdi->ulQmBufCnt[ulChannelId] , BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eQmBufHeapId)));
    }

    BDBG_LEAVE(BVDC_P_Mcvp_AllocBuf_isr);
}


/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_SetVnetAllocBuf_isr
*
* Called by BVDC_P_*_BuildRul_isr to setup for joinning into vnet (including
* optionally acquiring loop-back) and allocate buffers
*/
void BVDC_P_Mcvp_SetVnetAllocBuf_isr
    ( BVDC_P_Mcvp_Handle               hMcvp,
      uint32_t                         ulSrcMuxValue,
      BVDC_P_VnetPatch                 eVnetPatchMode,
      bool                             bRfcgVnet)
{
    BDBG_ENTER(BVDC_P_Mcvp_SetVnetAllocBuf_isr);
    BDBG_OBJECT_ASSERT(hMcvp, BVDC_MVP);
    BDBG_OBJECT_ASSERT(hMcvp->hMcdi, BVDC_MDI);

    if(bRfcgVnet)
    {
        /* set up for joining vnet, including acquiring loop-back */
        BVDC_P_SubRul_SetVnet_isr(&(hMcvp->SubRul), ulSrcMuxValue, eVnetPatchMode);
    }

    BDBG_LEAVE(BVDC_P_Mcvp_SetVnetAllocBuf_isr);
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_UnsetVnetFreeBuf_isr
*
* called by BVDC_P_Window_UnsetWriter(Reader)Vnet_isr to to release the
* potentially used loop-back, and free buffers
*/
void BVDC_P_Mcvp_UnsetVnetFreeBuf_isr
    ( BVDC_P_Mcvp_Handle                hMcvp )
{
    BDBG_ENTER(BVDC_P_Mcvp_UnsetVnetFreeBuf_isr);
    BDBG_OBJECT_ASSERT(hMcvp, BVDC_MVP);

    /* release free-channel or loop-back */
    BVDC_P_SubRul_UnsetVnet_isr(&(hMcvp->SubRul));

    /* free internally used buffers if no win is using this MCVP. */
    BVDC_P_Mcvp_FreeBuf_isr(hMcvp, false, 0, true);

    BDBG_LEAVE(BVDC_P_Mcvp_UnsetVnetFreeBuf_isr);
}
/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_BuildRul_SetEnable_isr
*
* called by BVDC_P_Mcvp_BuildRul_isr at every vsync to change siob compression settings.
*/

static void BVDC_P_Mvp_BuildRul_DcxInit_isr
    ( BVDC_P_Mcvp_Handle             hMcvp,
      BVDC_P_ListInfo               *pList,
      bool                           bEnable10Bit)
{
#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_2)
    uint32_t ulRegOffset = hMcvp->ulRegOffset;
    BVDC_P_Compression_Settings    *pstCompression = &hMcvp->stMvpCompression;


#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8)
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_SIOB_0_DCX_PRED_CFG, ulRegOffset,
        BVDC_P_REGS_ENTRIES(SIOB_0_DCX_PRED_CFG, SIOB_0_DCX_COMPR_CFG1));

    BDBG_ASSERT(pstCompression);

    /* SIOB_0_DCX_PRED_CFG */
    *pList->pulCurrent++ = (
        BCHP_FIELD_DATA(SIOB_0_DCX_PRED_CFG, ENABLE,          pstCompression->bEnable) |
        BCHP_FIELD_ENUM(SIOB_0_DCX_PRED_CFG, CONVERT_RGB,     Disable ) |
        BCHP_FIELD_DATA(SIOB_0_DCX_PRED_CFG, PREDICTION_MODE, pstCompression->ulPredictionMode) |
        BCHP_FIELD_ENUM(SIOB_0_DCX_PRED_CFG, EDGE_PRED_ENA,   Enable  ) |
        BCHP_FIELD_ENUM(SIOB_0_DCX_PRED_CFG, LEFT_PRED_ENA,   Enable  ) |
        BCHP_FIELD_ENUM(SIOB_0_DCX_PRED_CFG, ABCD_PRED_ENA,   Enable  ) |
        BCHP_FIELD_ENUM(SIOB_0_DCX_PRED_CFG, LS_PRED_ENA,     Enable  ));

    /* SIOB_0_DCX_COMPR_CFG1 */
    *pList->pulCurrent++ = (
        BCHP_FIELD_DATA(SIOB_0_DCX_COMPR_CFG1, PIXELS_PER_GROUP, pstCompression->ulPixelPerGroup) |
        BCHP_FIELD_DATA(SIOB_0_DCX_COMPR_CFG1, TGT_OFFSET_HI,                               0xfa) |
        BCHP_FIELD_DATA(SIOB_0_DCX_COMPR_CFG1, TGT_OFFSET_LO,                                 12) |
        BCHP_FIELD_DATA(SIOB_0_DCX_COMPR_CFG1, TGT_BPG,          pstCompression->ulBitsPerGroup));

    BSTD_UNUSED(bEnable10Bit);
#else
    uint32_t ulCompression, ulFixedRate;
    BDBG_ASSERT(pstCompression);

    /* non superset is supported in this version, including chips 7445D0/7145B0/7366B0*/
    ulCompression = (pstCompression->ulBitsPerGroup > BVDC_37BITS_PER_GROUP)
                    ? BCHP_SIOB_0_DCXS_CFG_COMPRESSION_BPP_11p25_OR_11 /* 11 bpp */
                    : BCHP_SIOB_0_DCXS_CFG_COMPRESSION_BPP_9p25_OR_9; /* 09 bpp */

    ulFixedRate = BCHP_SIOB_0_DCXS_CFG_FIXED_RATE_Fixed;

    /* SIOB_0_DCXS_CFG */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_SIOB_0_DCXS_CFG, ulRegOffset,
        BCHP_FIELD_DATA(SIOB_0_DCXS_CFG, ENABLE,       pstCompression->bEnable) |
        BCHP_FIELD_ENUM(SIOB_0_DCXS_CFG, APPLY_QERR,   Apply_Qerr             ) | /* nominal */
        BCHP_FIELD_DATA(SIOB_0_DCXS_CFG, FIXED_RATE,   ulFixedRate            ) | /* nominal */
        BCHP_FIELD_DATA(SIOB_0_DCXS_CFG, COMPRESSION,  ulCompression          ));

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_SIOB_0_SCB_MODE_CONTROL, ulRegOffset,
        BCHP_FIELD_DATA(SIOB_0_SCB_MODE_CONTROL, SCB_MODE_SEL, (uint32_t)bEnable10Bit));
#endif
#else
    BSTD_UNUSED(hMcvp);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(bEnable10Bit);
#endif
    return;
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_BuildRul_SetEnable_isr
*
* called by BVDC_P_Mcvp_BuildRul_isr at every vsync to change siob compression settings.
*/

static void BVDC_P_Mvp_BuildRul_DcxsInit_isr
    ( BVDC_P_Mcvp_Handle             hMcvp,
      BVDC_P_ListInfo               *pList )
{
    uint32_t ulRegOffset;
    uint32_t ulCompression, ulFixedRate;
    BVDC_P_Compression_Settings    *pstCompression = &hMcvp->stMvpCompression;

    BDBG_ASSERT(pstCompression);

    ulRegOffset = hMcvp->ulRegOffset;
#if (!BVDC_P_SUPPORT_MCDI_SUPERSET)
    /* non superset is supported in this version, including chips 7445D0/7145B0/7366B0*/
    ulRegOffset = hMcvp->ulRegOffset1;
    ulCompression = (pstCompression->ulBitsPerGroup >  BVDC_36BITS_PER_GROUP)
            ? BCHP_SIOB_1_DCXS_CFG_COMPRESSION_BPP_11 /* 11 bpp */
            : BCHP_SIOB_1_DCXS_CFG_COMPRESSION_BPP_9; /* 09 bpp */
    ulFixedRate = BCHP_SIOB_1_DCXS_CFG_FIXED_RATE_Fixed;
    /* set pic size into reg */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_SIOB_1_DCXS_CFG, ulRegOffset,
        BCHP_FIELD_DATA(SIOB_1_DCXS_CFG, ENABLE,       pstCompression->bEnable) |
        BCHP_FIELD_ENUM(SIOB_1_DCXS_CFG, APPLY_QERR,   Apply_Qerr             ) | /* nominal */
        BCHP_FIELD_DATA(SIOB_1_DCXS_CFG, FIXED_RATE,   ulFixedRate            ) | /* nominal */
        BCHP_FIELD_DATA(SIOB_1_DCXS_CFG, COMPRESSION,  ulCompression          ));

#else
    ulCompression = (pstCompression->ulBitsPerGroup > BVDC_36BITS_PER_GROUP)
            ? BCHP_SIOB_0_DCXS_CFG_COMPRESSION_BPP_11 /* 11 bpp */
            : BCHP_SIOB_0_DCXS_CFG_COMPRESSION_BPP_9; /* 09 bpp */

#if (BVDC_P_MADR_VARIABLE_RATE)
        ulFixedRate = BCHP_SIOB_0_DCXS_CFG_FIXED_RATE_Variable;
#else
        ulFixedRate = BCHP_SIOB_0_DCXS_CFG_FIXED_RATE_Fixed;
#endif
        /* set pic size into reg */
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_SIOB_0_DCXS_CFG, ulRegOffset,
            BCHP_FIELD_DATA(SIOB_0_DCXS_CFG, ENABLE,       pstCompression->bEnable    ) |
            BCHP_FIELD_ENUM(SIOB_0_DCXS_CFG, APPLY_QERR,   No_Apply                   ) | /* nominal */
            BCHP_FIELD_DATA(SIOB_0_DCXS_CFG, FIXED_RATE,   ulFixedRate                ) | /* nominal */
            BCHP_FIELD_DATA(SIOB_0_DCXS_CFG, COMPRESSION,  ulCompression              ));
#endif
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_BuildRul_SetEnable_isr
*
* called by BVDC_P_Mcvp_BuildRul_isr at every vsync to change siob compression settings.
*/
static void BVDC_P_Mcvp_BuildRul_SIOB_isr
    ( BVDC_P_Mcvp_Handle             hMcvp,
      BVDC_P_ListInfo               *pList,
      bool                           bEnable10Bit)
{
    if(hMcvp->hMcdi->bMadr)
    {
        BVDC_P_Mvp_BuildRul_DcxsInit_isr(hMcvp, pList);
    }
    else
    {
        BVDC_P_Mvp_BuildRul_DcxInit_isr(hMcvp, pList, bEnable10Bit);
    }

    hMcvp->stDirty.stBits.bCompress = BVDC_P_CLEAN;
}



/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_BuildRul_SetEnable_isr
*
* called by BVDC_P_Mcvp_BuildRul_isr at every vsync to enable or disable
* hscaler, anr, mcdi and top level.
*/
static void BVDC_P_Mcvp_BuildRul_SetEnable_isr
    ( BVDC_P_Mcvp_Handle             hMcvp,
      BVDC_P_ListInfo               *pList,
      bool                           bEnable,
      BVDC_P_PictureNode            *pPicture,
      bool                           bInit )
{
    uint32_t  ulRegOffset;
    uint32_t  ulModeCtrl;
    bool  bAnr, bMcdi;
    BVDC_FilterMode eMode=BVDC_FilterMode_eDisable;
#if ((BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_3)||(BVDC_P_SUPPORT_MCDI_VER == BVDC_P_MCDI_VER_8))
    uint32_t   ulEoPMask;
    bool       bEoPMask=false, bRepeat=false,bAnrHardStart=false;
#endif

    BDBG_OBJECT_ASSERT(hMcvp, BVDC_MVP);
    ulRegOffset = hMcvp->ulRegOffset;
    bAnr  = BVDC_P_MVP_USED_ANR(pPicture->stMvpMode);
    bMcdi = BVDC_P_MVP_USED_MAD(pPicture->stMvpMode);

    BDBG_OBJECT_ASSERT(hMcvp->hMcdi, BVDC_MDI);

    eMode = BVDC_FilterMode_eDisable;

#if (BVDC_P_SUPPORT_MANR)
    if((hMcvp->bAnr) && (bAnr))
    {
        BDBG_OBJECT_ASSERT(hMcvp->hAnr,  BVDC_ANR);
        eMode = hMcvp->hAnr->pAnrSetting->eMode;
    }
#else
    BSTD_UNUSED(bAnr);
#endif

    if (bEnable)
    {
        /* 1. configure MCVP */
        if(bMcdi)
        {
            ulModeCtrl =
                (BVDC_FilterMode_eEnable == eMode)?
                BCHP_FIELD_ENUM(MVP_TOP_0_CTRL, MODE_CTRL, NORMAL) :
                BCHP_FIELD_ENUM(MVP_TOP_0_CTRL, MODE_CTRL, MDI_ONLY);
        }
        else
        {
            /*BDBG_ERR(("mcvp mode: ND only or ND+MCTF"));*/
            ulModeCtrl =
                (BVDC_FilterMode_eEnable == eMode)?
                BCHP_FIELD_ENUM(MVP_TOP_0_CTRL, MODE_CTRL, ANR_ONLY):
                BCHP_FIELD_ENUM(MVP_TOP_0_CTRL, MODE_CTRL, BYPASS);
        }

        /* 1.a configure MVP_TOP_0_CTRL */
#if ((BVDC_P_SUPPORT_MCVP_VER >= BVDC_P_MCVP_VER_5) &&(BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_8))
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_CTRL, ulRegOffset,
            BCHP_FIELD_ENUM(MVP_TOP_0_CTRL, SUSPEND_EOP, ON)|
            BCHP_FIELD_ENUM(MVP_TOP_0_CTRL, ENABLE_CTRL, STOP_ON_FIELD_COMPLETION) |
            BCHP_FIELD_DATA(MVP_TOP_0_CTRL, MODE_CTRL, ulModeCtrl));
#else
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_CTRL, ulRegOffset,
            BCHP_FIELD_ENUM(MVP_TOP_0_CTRL, ENABLE_CTRL, STOP_ON_FIELD_COMPLETION) |
            BCHP_FIELD_DATA(MVP_TOP_0_CTRL, MODE_CTRL, ulModeCtrl));
#endif

        /* 1.b configure MVP_TOP_0_DEBUG_CTRL */
#if ((BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_3)||(BVDC_P_SUPPORT_MCDI_VER == BVDC_P_MCDI_VER_8))
        /*MCVP_TOP. DEBUG_CTRL.EOP_MASK should be programmed to 0
          unless MCVP is running at ANR only and trick mode, i.e., no TF_OUT_CAP capture */
        /* CRBVN-317.workaround fixed in 7439b0*/
        bRepeat = pPicture->stFlags.bPictureRepeatFlag;
        bAnrHardStart = ((NULL!=hMcvp->hAnr)&&(bAnr))?
            ((hMcvp->hAnr->ulMosaicInit>>pPicture->ulPictureIdx) & 1):
            false;
        bEoPMask = ((!bMcdi) && (bAnr) && bRepeat && (!bAnrHardStart)) ||
            (bMcdi && BVDC_P_MAD_SPATIAL(hMcvp->hMcdi->eGameMode));
        ulEoPMask = bEoPMask? 4:0;

#if (BVDC_P_SUPPORT_MCDI_VER == BVDC_P_MCDI_VER_8)
        /* CRBVN-343 workaround for MCDI only*/
        bEoPMask = bMcdi && bRepeat &&(!hMcvp->hMcdi->bMadr);
        ulEoPMask |= bEoPMask? 8:0;
#endif
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_DEBUG_CTRL, ulRegOffset,
            BCHP_FIELD_DATA(MVP_TOP_0_DEBUG_CTRL, EOP_MASK, ulEoPMask) |
            BCHP_FIELD_ENUM(MVP_TOP_0_DEBUG_CTRL, DIS_AUTO_FLUSH, OFF));
#endif
        /* 1.c configure MVP_TOP_0_DITHER_CTRL */
#if (BVDC_P_SUPPORT_MCVP_VER >= BVDC_P_MCVP_VER_6)
        if(hMcvp->hMcdi->bMadr)
        {
            bool bDitherEn =
#if BVDC_CRC_CAPTURE
                false;
#else
                /* enable dithering for 10 bit source (stream is 10b and MFD */
                /* is 10-bit or psuedo 10 bit source), and MADR is connected */
                /* to a 10-bit window path and MADR is not in bypass mode */
                (pPicture->bSrc10Bit && hMcvp->hWindow->bIs10BitCore && bMcdi &&
                 !hMcvp->hWindow->hCompositor->hDisplay->stCurInfo.bEnableStg) ?
                    true : false;
#endif

            if(hMcvp->bPrevDitherEn != bDitherEn)
            {
                BDBG_MODULE_MSG(BVDC_DITHER,("MVP%d DITHER: %s", hMcvp->eId,
                    (bDitherEn) ? "ENABLE" : "DISABLE"));
                hMcvp->bPrevDitherEn = bDitherEn;
            }

            BVDC_P_Dither_Setting_isr(&hMcvp->stDither, bDitherEn, 0xa2706, 0x1);

            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_DITHER_CTRL, ulRegOffset,
                hMcvp->stDither.ulCtrlReg);
            if(bDitherEn)
            {
                BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_DITHER_LFSR_INIT, ulRegOffset,
                    hMcvp->stDither.ulLfsrInitReg);
                BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_DITHER_LFSR_CTRL, ulRegOffset,
                    hMcvp->stDither.ulLfsrCtrlReg);
            }
        }
#endif


        /* 2. config and enable mcdi */
        BVDC_P_Mcdi_BuildRul_SetEnable_isr(hMcvp->hMcdi, pList, bMcdi, pPicture, bInit);

        /* 3. configure and enable ANR
        *    3.1. program AND thresh, AND_AND_MODE;
        *    3.2. enable and;
        *    3.3. configure and enable MCTF */
#if (BVDC_P_SUPPORT_MANR)
        if(hMcvp->bAnr)
        {
            BVDC_P_Anr_BuildRul_SetEnable_isr(hMcvp->hAnr, pPicture, pList, (eMode==BVDC_FilterMode_eEnable));
        }
#endif
        /* 4. configure and enable HSCL, HSCL cfg and enabling have already set in shadow */
        BVDC_P_Hscaler_BuildRul_SrcInit_isr(hMcvp->hHscaler, pList);
        BVDC_P_Hscaler_SetEnable_isr(hMcvp->hHscaler, true, pList);

        /* 5 enable mcvp top */
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_ENABLE, ulRegOffset,
            BCHP_FIELD_ENUM(MVP_TOP_0_ENABLE, ENABLE, ON));
    }
    else
    {
        /* 1 disable mcvp top */
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_ENABLE, ulRegOffset,
            BCHP_FIELD_ENUM(MVP_TOP_0_ENABLE, ENABLE, OFF));

        /* 2. configure and disable HSCL */
        BVDC_P_Hscaler_SetEnable_isr(hMcvp->hHscaler, false, pList);

#if (BVDC_P_SUPPORT_MANR)
        /* 3. disable ANR */
        if(hMcvp->bAnr)
        {
            BVDC_P_Anr_BuildRul_SetEnable_isr(hMcvp->hAnr, pPicture, pList, false);
        }
#endif

        /* 4. config and enable mcdi */
        BVDC_P_Mcdi_BuildRul_SetEnable_isr(hMcvp->hMcdi, pList, bEnable,
            pPicture, bInit);
    }

    return;
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_BuildRul_DrainVnet_isr
*
* called by BVDC_P_Mcvp_BuildRul_isr after resetting to drain the module and
* its pre-patch FreeCh or LpBack.
*/
static void BVDC_P_Mcvp_BuildRul_DrainVnet_isr
    ( BVDC_P_Mcvp_Handle             hMcvp,
      BVDC_P_ListInfo               *pList,
      bool                           bNoCoreReset)
{
    BDBG_OBJECT_ASSERT(hMcvp, BVDC_MVP);
#if (BVDC_P_SUPPORT_MCVP_VER > BVDC_P_MCVP_VER_3)
    BVDC_P_SubRul_Drain_isr(&(hMcvp->SubRul), pList,
        bNoCoreReset? 0 : hMcvp->ulCoreResetAddr,
        bNoCoreReset? 0 : hMcvp->ulCoreResetMask,
        bNoCoreReset? 0 : hMcvp->ulVnetResetAddr,
        bNoCoreReset? 0 : hMcvp->ulVnetResetMask);
#else
    BVDC_P_SubRul_Drain_isr(&(hMcvp->SubRul), pList,
        hMcvp->ulCoreResetAddr, hMcvp->ulCoreResetMask,
        hMcvp->ulVnetResetAddr, hMcvp->ulVnetResetMask);
    hMcvp->stDirty.stBits.bCompress = BVDC_P_DIRTY;
    BSTD_UNUSED(bNoCoreReset);
#endif

    return;
}


static void BVDC_P_MCVP_SetSingleInfo_isr
    ( BVDC_P_Mcvp_Handle                 hMcvp,
      BVDC_Window_Handle                 hWindow,
      BVDC_P_PictureNode                 *pPicture)
{
    BVDC_P_Window_Info *pUserInfo = &hWindow->stCurInfo;
    uint32_t ii;
    uint32_t ulHeapSize=0;
    uint32_t ulWidth=0, ulHeight=0;
    uint32_t ulChannelId = pPicture->ulPictureIdx;
    BVDC_P_BufferHeapId   ePixelBufHeapId= BVDC_P_BufferHeapId_eUnknown, eQmBufHeapId=BVDC_P_BufferHeapId_eUnknown;
    const BVDC_Anr_Settings  *pAnrSetting;
    BVDC_P_Source_InfoPtr   pCurSrcInfo;
    BVDC_P_McdiDirtyBits  *pSwDirty = NULL;
    BVDC_P_AnrDirtyBits   *pAnrSwDirty=NULL;
    bool bMad, bAnr, bBypass;
    BVDC_P_MvpMode *pNewMcvpMode, *pCurMcvpMode;

    BDBG_ENTER(BVDC_P_MCVP_SetSingleInfo_isr);

    bAnr = BVDC_P_MVP_USED_ANR(pPicture->stMvpMode);
    bMad = BVDC_P_MVP_USED_MAD(pPicture->stMvpMode);
    bBypass = BVDC_P_MVP_BYPASS_MVP(pPicture->stMvpMode);

    pSwDirty = &hMcvp->hMcdi->stSwDirty;
    BVDC_P_CLEAN_ALL_DIRTY(pSwDirty);
    if(hMcvp->bAnr)
    {
        pAnrSwDirty     = &hMcvp->hAnr->stSwDirty;
        BVDC_P_CLEAN_ALL_DIRTY(pAnrSwDirty);
    }

    /* set the compression */
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_MAD_ANR)
    if(!bBypass)
    {
        BVDC_P_Compression_Settings     *pNewCompression, *pCurCompression;

        BVDC_P_Window_UpdateMadAnrCompression_isr(hWindow,
            pUserInfo->stMadSettings.ePixelFmt,
            bMad?pPicture->pMadIn:pPicture->pAnrIn, pPicture, &hWindow->stMadCompression,
            BVDC_P_VNET_USED_MVP_AT_WRITER(pPicture->stVnetMode));

        pNewCompression = &hWindow->stMadCompression;
        pCurCompression = &hMcvp->stMvpCompression;

        if((pCurCompression->bEnable!=pNewCompression->bEnable)||
            (pCurCompression->ulBitsPerGroup != pNewCompression->ulBitsPerGroup) ||
            (pCurCompression->ulPixelPerGroup != pNewCompression->ulPixelPerGroup)||
            (pCurCompression->ulPredictionMode != pNewCompression->ulPredictionMode))
        {
            hMcvp->stDirty.stBits.bCompress = BVDC_P_DIRTY;
            *pCurCompression = *pNewCompression;
            BDBG_MSG(("hMvp[%d] bCompress dirty", hMcvp->eId));
        }
    }
#endif


    if (BVDC_P_VNET_USED_MVP(pPicture->stVnetMode))
    {
        pCurMcvpMode = &hMcvp->stMcvpMode[ulChannelId];
        pNewMcvpMode = &pPicture->stMvpMode;

        if((pNewMcvpMode->stBits.bUseHscl      != pCurMcvpMode->stBits.bUseHscl) ||
           (pNewMcvpMode->stBits.bUseAnr       != pCurMcvpMode->stBits.bUseAnr ) ||
           (pNewMcvpMode->stBits.bUseMad       != pCurMcvpMode->stBits.bUseMad ) ||
           (pNewMcvpMode->stBits.bUseMvpBypass != pCurMcvpMode->stBits.bUseMvpBypass))
        {
            BDBG_MSG(("Mcvp %d bUse Hscl %d  bUseAnr %d      bUseMad %d", hMcvp->eId,
            pNewMcvpMode->stBits.bUseHscl,
            pNewMcvpMode->stBits.bUseAnr,
            pNewMcvpMode->stBits.bUseMad));
            pSwDirty->stBits.bModeSwitch = BVDC_P_DIRTY;
            *pCurMcvpMode = *pNewMcvpMode;
        }

        /* 2. channel change */
        if(pPicture->bChannelChange !=hMcvp->bChannelChange[ulChannelId])
        {
            hMcvp->hMcdi->stSwDirty.stBits.bChannelChange = BVDC_P_DIRTY;
            hMcvp->bChannelChange[ulChannelId] = pPicture->bChannelChange;
            if((hMcvp->bAnr) & (bAnr))
                hMcvp->hAnr->stSwDirty.stBits.bChannel = BVDC_P_DIRTY;
        }

        pCurSrcInfo = &hWindow->stCurInfo.hSource->stCurInfo;

        /* 3. update user setting on pqEnhancement */
        hMcvp->ePqEnhancement = pUserInfo->stMadSettings.ePqEnhancement;

        BVDC_P_Mcdi_Init_Chroma_DynamicDefault_isr(hMcvp->hMcdi,
            &(pUserInfo->stMadSettings.stChromaSettings),
            pUserInfo->hSource->stCurInfo.pFmtInfo,
            BVDC_P_SRC_IS_MPEG(pUserInfo->hSource->eId));
        BVDC_P_Mcdi_GetUserConf_isr(hMcvp->hMcdi,
            (BVDC_P_Deinterlace_Settings*)&pUserInfo->stMadSettings);

        if((hMcvp->bAnr) & (bAnr))
        {
            pAnrSetting = (const BVDC_Anr_Settings*) &pUserInfo->stAnrSettings;
            pCurSrcInfo = &pUserInfo->hSource->stCurInfo;
            BVDC_P_Anr_SetUsrSettingPtr(hMcvp->hAnr, pAnrSetting);
            BVDC_P_Anr_SetSrcInfo(hMcvp->hAnr, pCurSrcInfo);
        }
    }

    if (true == bBypass)  return;

    /* if 0 buffer, set the heap id equal to the new one to avoid unnecessary heap id difference*/
    ulWidth  = bMad?pPicture->pMadIn->ulWidth:pPicture->pAnrIn->ulWidth;
    ulHeight = (bMad?pPicture->pMadIn->ulHeight:pPicture->pAnrIn->ulHeight)>>
            (BAVC_Polarity_eFrame != pPicture->eSrcPolarity);

    if(0==hMcvp->hMcdi->ulQmBufCnt[ulChannelId])
        eQmBufHeapId = pPicture->eMadQmHeapId;
    if(0==hMcvp->hMcdi->ulPxlBufCnt[ulChannelId])
        ePixelBufHeapId = pPicture->eMadPixelHeapId;

    if(NULL != hMcvp->hMcdi->apHeapNode[ulChannelId][0])
        ePixelBufHeapId = hMcvp->hMcdi->apHeapNode[ulChannelId][0]->pHeapInfo->eBufHeapId;
    if(NULL != hMcvp->hMcdi->apQmHeapNode[ulChannelId][0])
        eQmBufHeapId = hMcvp->hMcdi->apQmHeapNode[ulChannelId][0]->pHeapInfo->eBufHeapId;

        /* 2. set Mcdi buffer parameter */
    if((hMcvp->hMcdi->astRect[ulChannelId].ulWidth != ulWidth)||
        (hMcvp->hMcdi->astRect[ulChannelId].ulHeight != ulHeight ))
    {
        BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC,("channel %d cur size %d x %d new %d x %d ",
            ulChannelId, hMcvp->hMcdi->astRect[ulChannelId].ulWidth, hMcvp->hMcdi->astRect[ulChannelId].ulHeight,
            ulWidth, ulHeight));
        hMcvp->hMcdi->astRect[ulChannelId].ulWidth  = ulWidth ;
        hMcvp->hMcdi->astRect[ulChannelId].ulHeight = ulHeight;
        hMcvp->hMcdi->ulPxlBufSize[ulChannelId] = pPicture->ulMadPxlBufSize;
        pSwDirty->stBits.bSize   = BVDC_P_DIRTY;
        if(bAnr && (hMcvp->bAnr))
        {
            pAnrSwDirty->stBits.bSize = BVDC_P_DIRTY;
            hMcvp->hAnr->ulPxlBufSize[ulChannelId] = hMcvp->hMcdi->ulPxlBufSize[ulChannelId];
        }
    }

    /* 3. set Mcdi buffer parameter */
    if((hMcvp->hMcdi->ulPxlBufCnt[ulChannelId] != pPicture->usMadPixelBufferCnt)||
        (hMcvp->hMcdi->ulQmBufCnt[ulChannelId] != pPicture->usMadQmBufCnt) ||
        (ePixelBufHeapId != pPicture->eMadPixelHeapId) ||
        (eQmBufHeapId != pPicture->eMadQmHeapId))
    {
        BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC,("channel %d cur %d x %s  %d x %s new %d x %s  %d x %s ",
            ulChannelId,
            hMcvp->hMcdi->ulPxlBufCnt[ulChannelId],BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(ePixelBufHeapId),
            hMcvp->hMcdi->ulQmBufCnt[ulChannelId], BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eQmBufHeapId),
            pPicture->usMadPixelBufferCnt, BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pPicture->eMadPixelHeapId),
            pPicture->usMadQmBufCnt, BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pPicture->eMadQmHeapId)));
        pSwDirty->stBits.bBuffer   = BVDC_P_DIRTY;

            /* in nexus mosaic playback application, there could be mosaic size is 0 and not display at all */
        if(((pPicture->usMadPixelBufferCnt == 0) ||
            (pPicture->eMadPixelHeapId == BVDC_P_BufferHeapId_eUnknown) ||
                (ulWidth==0) ||(ulHeight==0)) &&
            (!bBypass))
        {
            BDBG_WRN(("channel[%d] size 00000000!!!!! %d x %d buffer  %d x %s",
            ulChannelId, ulWidth, ulHeight, pPicture->usMadPixelBufferCnt,
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pPicture->usMadPixelBufferCnt)));
        }

        if((hMcvp->bAnr) && (bAnr))
        {
            hMcvp->hAnr->ulPxlBufCnt[ulChannelId] = hMcvp->hMcdi->ulPxlBufCnt[ulChannelId];
            pAnrSwDirty->stBits.bBuffer = BVDC_P_DIRTY;
            for (ii=0; ii<BVDC_P_ANR_BUFFER_COUNT;ii++)
                hMcvp->hAnr->apHeapNode[ulChannelId][ii] = hMcvp->hMcdi->apHeapNode[ulChannelId][ii];
        }


        /* 3.2. QM buffer context init */
        eQmBufHeapId = pPicture->eMadQmHeapId;
        if(eQmBufHeapId!=BVDC_P_BufferHeapId_eUnknown)
            BVDC_P_BufferHeap_GetHeapSizeById_isr(hMcvp->hHeap, eQmBufHeapId, &ulHeapSize);
        hMcvp->hMcdi->ulQmBufSize[ulChannelId] = ulHeapSize / BVDC_P_MAD_QM_FIELD_STORE_COUNT;
    }
    /* 4. orientation exam */
    if ((pPicture->eSrcOrientation != hMcvp->hMcdi->eSrcOrientation) ||
        (pPicture->eDispOrientation != hMcvp->hMcdi->eDispOrientation))
    {
        hMcvp->hMcdi->eSrcOrientation = pPicture->eSrcOrientation;
        hMcvp->hMcdi->eDispOrientation = pPicture->eDispOrientation;

        hMcvp->hMcdi->stSwDirty.stBits.bSize = BVDC_P_DIRTY;

        if(hMcvp->bAnr)
        {
            hMcvp->hAnr->stSwDirty.stBits.bSize = BVDC_P_DIRTY;
        }
    }
    BDBG_LEAVE(BVDC_P_MCVP_SetSingleInfo_isr);
}

void BVDC_P_MCVP_SetInfo_isr
    ( BVDC_P_Mcvp_Handle                 hMcvp,
      BVDC_Window_Handle                 hWindow,
      BVDC_P_PictureNode                *pPicture)
{
    BDBG_OBJECT_ASSERT(hMcvp, BVDC_MVP);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(BVDC_P_MVP_USED_HSCL(pPicture->stMvpMode))
        BVDC_P_Hscaler_SetInfo_isr(hMcvp->hHscaler, hWindow, pPicture);

    /* set one chanel and allocate buffer each time */
    BVDC_P_MCVP_SetSingleInfo_isr(hMcvp, hWindow, pPicture);
    BVDC_P_Mcvp_AllocBuf_isr(hMcvp, pPicture);
    BDBG_LEAVE(BVDC_P_MCVP_SetInfo_isr);
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_BuildRul_isr
*
* called by BVDC_Window_BuildRul_isr at every src or vec vsync (depending on
* whether reader side or writer side is using this module)
*
* Input:
*    eVnetState - reader or writer window/vnet state
*    pPicComRulInfo - the PicComRulInfo that is the shared Picture info by
*      all sub-modules when they build rul.
*/
void BVDC_P_Mcvp_BuildRul_isr
    ( BVDC_P_Mcvp_Handle                 hMcvp,
      BVDC_P_ListInfo                   *pList,
      BVDC_P_State                       eVnetState,
      BVDC_P_WindowContext              *pWindow,
      BVDC_P_PictureNode                *pPicture )
{
#if (BVDC_P_SUPPORT_MANR)
    BVDC_P_AnrDirtyBits  *pAnrSwDirty=NULL, *pAnrHwDirty=NULL;
#endif
    BVDC_P_McdiDirtyBits  *pMcdiSwDirty=NULL, *pMcdiHwDirty=NULL;
    BVDC_P_PicComRulInfo  *pPicComRulInfo;

    uint32_t  ulRulOpsFlags;
    bool  bInit, bEnable, bAnrDirty = false, bMcdiDirty = false, bBypass=false;
    uint32_t ulChannelId = pPicture->ulPictureIdx;

    BDBG_ENTER(BVDC_P_Mcvp_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hMcvp, BVDC_MVP);

    pPicComRulInfo = &pPicture->PicComRulInfo;

    BDBG_OBJECT_ASSERT(hMcvp->hMcdi, BVDC_MDI);
    pMcdiSwDirty = &hMcvp->hMcdi->stSwDirty;
    pMcdiHwDirty = &hMcvp->hMcdi->stHwDirty;
    BVDC_P_CLEAN_ALL_DIRTY(pMcdiHwDirty);

#if (BVDC_P_SUPPORT_MANR)
    if(hMcvp->bAnr)
    {
        BDBG_OBJECT_ASSERT(hMcvp->hAnr, BVDC_ANR);
        pAnrSwDirty = &hMcvp->hAnr->stSwDirty;
        pAnrHwDirty = &hMcvp->hAnr->stHwDirty;
        BVDC_P_CLEAN_ALL_DIRTY(pAnrHwDirty);
    }
#endif

    ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(
        &(hMcvp->SubRul), pWindow->eId, eVnetState, pList->bLastExecuted);
    bEnable = (ulRulOpsFlags & BVDC_P_RulOp_eEnable);
    bInit   = (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit);

    bMcdiDirty = BVDC_P_IS_DIRTY(pMcdiSwDirty);
    if(hMcvp->bAnr)
    {
        bAnrDirty = BVDC_P_IS_DIRTY(&(hMcvp->hAnr->stSwDirty));
    }
    if(bEnable)
    {
        bBypass = BVDC_P_MVP_BYPASS_MVP(pPicture->stMvpMode);
        if(hMcvp->stDirty.stBits.bCompress)
        {
            BVDC_P_Mcvp_BuildRul_SIOB_isr(hMcvp, pList, pPicture->bEnable10Bit);
        }
        if(bBypass)
        {
            /* set mode */
            BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_CTRL, hMcvp->ulRegOffset,
                BCHP_FIELD_ENUM(MVP_TOP_0_CTRL, ENABLE_CTRL, STOP_ON_FIELD_COMPLETION) |
                BCHP_FIELD_ENUM(MVP_TOP_0_CTRL, MODE_CTRL,   BYPASS));

                /* set Enable */
                BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MVP_TOP_0_ENABLE, hMcvp->ulRegOffset,
                            BCHP_FIELD_ENUM(MVP_TOP_0_ENABLE, ENABLE, ON));
        }
        else
        {
            /* if we re-set vnet, or if src video format changes (HdDvi), we need to
            * do SrcInit, such as reset buffer size and stride */
            if(bInit || bAnrDirty || bMcdiDirty ||
                (pPicComRulInfo->PicDirty.stBits.bInputFormat &&(!pPicture->bMosaicMode)) ||
                !pList->bLastExecuted )
            {
                hMcvp->ulUpdateAll[ulChannelId] = BVDC_P_RUL_UPDATE_THRESHOLD;
#if ((BVDC_P_MADR_VER_10 >= BVDC_P_SUPPORT_MADR_VER) &&(!BVDC_P_NO_MADR_SWINIT))
                /* mode/channel switch leads to mcvp core reset, need to reprogram hscl */
                hMcvp->hHscaler->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;

                /* SW7552-178/182 SW7231-1208 workaround for MADR seamless transation and channel change*/
                /* SW7364-123: madr obts crc mistmatch */
                if((hMcvp->hMcdi->bMadr) &&
                    (bMcdiDirty || bInit))
                {
                    BVDC_P_BUILD_RESET_NOOPS(pList->pulCurrent,
                        hMcvp->ulCoreResetAddr, hMcvp->ulCoreResetMask);
                    BVDC_P_Mvp_BuildRul_DcxsInit_isr(hMcvp, pList);
                }
#if (BVDC_P_MADR_VER_10 <= BVDC_P_SUPPORT_MADR_VER)
                /* SW_INIT needs to reprogram every channel size information */
                if(pPicture->bMosaicMode)
                {
                    uint32_t i=0;
                    for(i=0; i<pPicture->ulMosaicCount; i++)
                        hMcvp->ulUpdateAll[i] = BVDC_P_RUL_UPDATE_THRESHOLD;
                }
#endif
#endif
            }

            if(hMcvp->ulUpdateAll[ulChannelId])
                BDBG_MSG(("ch[%d] ulUpdateAll %d bInit %d bAnrDirty %x, bMcdiDirty %x, bLastExecuted %d",
                    pPicture->ulPictureIdx, hMcvp->ulUpdateAll[ulChannelId],bInit,
                    bAnrDirty?hMcvp->hAnr->stSwDirty.aulInts[0]:0, hMcvp->hMcdi->stSwDirty.aulInts[0], !pList->bLastExecuted ));
            if(hMcvp->ulUpdateAll[ulChannelId])
            {
                hMcvp->ulUpdateAll[ulChannelId]--;

                if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
                {
                    BVDC_P_Mcdi_BuildRul_SrcInit_isr(hMcvp->hMcdi, pList, pPicture);
                }
#if (BVDC_P_SUPPORT_MANR)
                if(hMcvp->bAnr && BVDC_P_MVP_USED_ANR(pPicture->stMvpMode))
                {
                    BVDC_P_Anr_BuildRul_SrcInit_isr(hMcvp->hAnr, pList, pPicture);
                }
#endif
            }

            BVDC_P_Mcvp_BuildRul_SetEnable_isr(hMcvp, pList, true, pPicture,
                bInit);
            *pMcdiHwDirty = *pMcdiSwDirty;
            BVDC_P_CLEAN_ALL_DIRTY(pMcdiSwDirty);
#if (BVDC_P_SUPPORT_MANR)
            if(hMcvp->bAnr)
            {
                /* changes has been built to RUL, but not executed yet */
                *pAnrHwDirty = *pAnrSwDirty;
                BVDC_P_CLEAN_ALL_DIRTY(pAnrSwDirty);
            }
#endif
        }

        /* join in vnet after enable. note: its src mux is initialed as disabled */
        if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
        {
            /*BDBG_ERR(("mcvp join vnet, mux 0x%x, value 0x%x", hMcvp->SubRul.ulMuxAddr,
            BVDC_P_LpBack_MuxAddr_To_PostMuxValue(hMcvp->SubRul.ulPatchMuxAddr)));*/
            BVDC_P_SubRul_JoinInVnet_isr(&(hMcvp->SubRul), pList);
        }

    }
    else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
    {
        BVDC_P_SubRul_DropOffVnet_isr(&(hMcvp->SubRul), pList);
        BVDC_P_Mcvp_BuildRul_SetEnable_isr(hMcvp, pList, false, pPicture,
            false);
    }
    else if (ulRulOpsFlags & BVDC_P_RulOp_eDrainVnet)
    {
        BVDC_P_Mcvp_BuildRul_DrainVnet_isr(hMcvp, pList,  pPicComRulInfo->bNoCoreReset);
    }
    BDBG_LEAVE(BVDC_P_Mcvp_BuildRul_isr);

    return;
}

/***************************************************************************
* Initialized back to default whatever user did not customized.
*
*/
void BVDC_P_Mvp_Init_Default
    ( BVDC_MadGameMode                   *peGameMode,
      BPXL_Format                        *pePxlFormat,
      BVDC_Mode                          *pePqEnhancement,
      bool                               *pbShrinkWidth,
      bool                               *pbReverse32Pulldown,
      bool                               *pbReverse22Pulldown,
      BVDC_Deinterlace_ChromaSettings    *pChromaSettings,
      BVDC_Deinterlace_MotionSettings    *pMotionSettings )
{
    if(peGameMode)
    {
        *peGameMode = BVDC_MadGameMode_eOff;
    }

    if(pePxlFormat)
    {
        *pePxlFormat = BVDC_P_CAP_PIXEL_FORMAT_8BIT422;
    }

    if(pePqEnhancement)
    {
        *pePqEnhancement = BVDC_Mode_eAuto;
    }

    if(pbReverse22Pulldown)
    {
#if (BVDC_P_SUPPORT_MADR_VER >= BVDC_P_MADR_VER_8)
        /* SW7439-12: turn on 2:2 pull down */
        *pbReverse22Pulldown = true;
#else
        /* SW7552-213: turn off 2:2 pull down */
        *pbReverse22Pulldown = false;
#endif
    }

    if(pbReverse32Pulldown)
    {
        /* SW7420-2278: turn on 3:2 pull down */
        *pbReverse32Pulldown = true;
    }

    if(pbShrinkWidth)
    {
        *pbShrinkWidth = false;
    }

    if (pChromaSettings)
    {
        pChromaSettings->bChromaField420EdgeDetMode = true;
        pChromaSettings->bChromaField420InitPhase = false;
        pChromaSettings->eChroma422InverseTelecineMode = BVDC_Deinterlace_Chroma422ItMode_eMotionAdaptive;
        pChromaSettings->eChroma422MotionAdaptiveMode = BVDC_Deinterlace_Chroma422MaMode_eMotionAdaptive;
        pChromaSettings->eChromaField420InvMethod = BVDC_Deinterlace_Chroma420InvMethod_ePoly;
        pChromaSettings->ulMaxXChroma = 0xFF;
        pChromaSettings->eChroma420MotionAdaptiveMode = BVDC_Deinterlace_Chroma422MaMode_eMotionAdaptive;
        pChromaSettings->eChroma420MotionMode = BVDC_Deinterlace_ChromaMotionMode_eXchromaAware;
        pChromaSettings->eChroma422MotionMode = BVDC_Deinterlace_ChromaMotionMode_eXchromaAware;
        pChromaSettings->bMS_3548 = false;
        pChromaSettings->bMT_3548 = false;
    }

    if (pMotionSettings)
    {
        pMotionSettings->eTmMode = 2;
        pMotionSettings->eSmMode = 1;
        pMotionSettings->bEnableQmM = true;
        pMotionSettings->bEnableQmL = true;
        pMotionSettings->bEnableQmK = true;
    }

    return;
}


void BVDC_P_Mvp_Init_Custom
    ( BVDC_422To444UpSampler            *pUpSampler,
      BVDC_444To422DnSampler            *pDnSampler,
      BVDC_Deinterlace_LowAngleSettings *pLowAngles )
{
    if(pUpSampler)
    {
        pUpSampler->eFilterType = BVDC_422To444Filter_eTenTaps;
        pUpSampler->eRingRemoval   = BVDC_RingSuppressionMode_eNormal;
        pUpSampler->bUnbiasedRound = true;
    }

    if(pDnSampler)
    {
        pDnSampler->eFilterType    = BVDC_444To422Filter_eDecimate;
        pDnSampler->eRingRemoval   = BVDC_RingSuppressionMode_eNormal;
    }

    if(pLowAngles)
    {
        pLowAngles->ulLaControlDirRatio = 8;
        pLowAngles->ulLaControlRangeLimitScale = 0x11;
        pLowAngles->ulLaMinNorthStrength = 0;
    }

    return;
}

/* End of file. */
