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
#include "bstd.h"
#include "bvdc.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_priv.h"

BDBG_MODULE(VNET);
BDBG_OBJECT_ID(BVDC_VNT);

/* INDEX: By source id, do not put ifdefs and nested ifdefs around these that
 * become impossible to decipher. */
static const uint32_t  s_aulVnetFDrainSrc[] =
{
    BVDC_P_VnetF_eInvalid,     /* Mpeg0 */
    BVDC_P_VnetF_eInvalid,     /* Mpeg1 */
    BVDC_P_VnetF_eInvalid,     /* Mpeg2 */
    BVDC_P_VnetF_eInvalid,     /* Mpeg3 */
    BVDC_P_VnetF_eInvalid,     /* Mpeg4 */
    BVDC_P_VnetF_eInvalid,     /* Mpeg5 */
    BVDC_P_VnetF_eAnalog_0,    /* Vdec0 */
    BVDC_P_VnetF_eAnalog_1,    /* Vdec1 */
    BVDC_P_VnetF_eCcir656_0,   /* 656In0 */
    BVDC_P_VnetF_eCcir656_1,   /* 656In1 */
    BVDC_P_VnetF_eInvalid,     /* Gfx0 */
    BVDC_P_VnetF_eInvalid,     /* Gfx1 */
    BVDC_P_VnetF_eInvalid,     /* Gfx2 */
    BVDC_P_VnetF_eInvalid,     /* Gfx3 */
    BVDC_P_VnetF_eInvalid,     /* Gfx4 */
    BVDC_P_VnetF_eInvalid,     /* Gfx5 */
    BVDC_P_VnetF_eInvalid,     /* Gfx6 */
    BVDC_P_VnetF_eHdDvi_0,     /* HdDvi0 */
    BVDC_P_VnetF_eHdDvi_1,     /* HdDvi0 */
    BVDC_P_VnetF_eInvalid,     /* Ds0 */
};


/***************************************************************************
 * Trying to aquire (1) front drain or (2) back drain.
 *
 * (1) SRC -> VNET_F_DRN_x [ 1 resource, x]
 *
 * if fails try
 *
 * (2) SRC -> VNET_F_FCH_x -> VNET_B_DRN_y  [ 2 resources, x and y ]
 *
 */
BERR_Code BVDC_P_Drain_Acquire
    ( BVDC_P_DrainContext             *pDrain,
      BVDC_P_Resource_Handle           hResource,
      BAVC_SourceId                    eSourceId )
{
    uint32_t ulVnetFDrainId = 0, ulVnetBDrainId = 0;
    BERR_Code eResult = BERR_SUCCESS;

    /* Sanity check and get context */
    BDBG_OBJECT_ASSERT(hResource, BVDC_RES);
    BDBG_ASSERT(pDrain);

    /* Init */
    pDrain->ulDbgOffset = BVDC_P_DRN_INVALID_OFFSET;

    /* acquire BVDC_P_ResourceType_eDrainFrn first */
    BKNI_EnterCriticalSection();
    eResult = BVDC_P_Resource_AcquireHwId_isr(hResource,
        BVDC_P_ResourceType_eDrainFrn,
        (BAVC_SourceId_eHdDvi1 == eSourceId) ? BVDC_P_Able_eAllSrc : 0,
        eSourceId, &ulVnetFDrainId, false);
    BKNI_LeaveCriticalSection();

    if(BERR_SUCCESS == eResult)
    {
        BDBG_ASSERT(BVDC_P_DrainFrnId_eUnknown != ulVnetFDrainId);
        pDrain->eVnetFDrainType = BVDC_P_ResourceType_eDrainFrn;
        pDrain->ulVnetFDrainId  = ulVnetFDrainId;
        pDrain->eVnetFDrainSrc  = s_aulVnetFDrainSrc[eSourceId];
        pDrain->eVnetBDrainType = BVDC_P_ResourceType_eInvalid;
        pDrain->ulVnetBDrainId  = BVDC_P_VnetB_eInvalid;
        pDrain->eVnetBDrainSrc  = BVDC_P_VnetB_eDisabled;

#if (BVDC_P_SUPPORT_DRAIN_VER >= BVDC_P_VNET_VER_1) /* Has drain */
#if (BVDC_P_SUPPORT_DRAIN_F > 1)
        pDrain->ulDbgOffset = ((ulVnetFDrainId) *
             (BCHP_VNET_F_DRAIN_1_ERR_STATUS - BCHP_VNET_F_DRAIN_0_ERR_STATUS));
#endif
#endif
        return BERR_SUCCESS;
    }

    /* didn't get a BVDC_P_ResourceType_eDrainFrn,
     * try BVDC_P_ResourceType_eFreeCh with BVDC_P_ResourceType_eDrainBck or
     * BVDC_P_ResourceType_eVnetCrc */
    BKNI_EnterCriticalSection();
    eResult = BVDC_P_Resource_AcquireHwId_isr(hResource,
        BVDC_P_ResourceType_eFreeCh, 0, eSourceId, &ulVnetFDrainId, false);
    BKNI_LeaveCriticalSection();

    if(BERR_SUCCESS == eResult)
    {
        BDBG_ASSERT(BVDC_P_DrainFrnId_eUnknown != ulVnetFDrainId);
        BDBG_MSG(("Got FCH, try to get Back drain"));

        /* Got FCH, try BVDC_P_ResourceType_eDrainBck */
        BKNI_EnterCriticalSection();
        eResult = BVDC_P_Resource_AcquireHwId_isr(hResource,
            BVDC_P_ResourceType_eDrainBck, 0, eSourceId, &ulVnetBDrainId, false);
        BKNI_LeaveCriticalSection();

        if(BERR_SUCCESS == eResult)
        {
            BDBG_ASSERT(BVDC_P_DrainBckId_eUnknown != ulVnetBDrainId);

            /* Use back drain */
            pDrain->eVnetFDrainType = BVDC_P_ResourceType_eFreeCh;
            pDrain->ulVnetFDrainId  = ulVnetFDrainId;
            pDrain->eVnetFDrainSrc  = s_aulVnetFDrainSrc[eSourceId];
            pDrain->eVnetBDrainType = BVDC_P_ResourceType_eDrainBck;
            pDrain->ulVnetBDrainId  = ulVnetBDrainId;
            pDrain->eVnetBDrainSrc  = BVDC_P_VnetB_eChannel_0 + ulVnetFDrainId;

            /* Offset from drain F_DRAIN_0.  Currently only 1 drain available. */
#if (BVDC_P_SUPPORT_DRAIN_VER >= BVDC_P_VNET_VER_1) /* Has drain */
            pDrain->ulDbgOffset = (
                BCHP_VNET_B_DRAIN_0_ERR_STATUS -
                BCHP_VNET_F_DRAIN_0_ERR_STATUS);
            BDBG_ASSERT(BVDC_P_SUPPORT_DRAIN_B == 1);
#endif
            return BERR_SUCCESS;
        }
    }

    return BERR_TRACE(eResult);

}

/***************************************************************************
 *
 */
void BVDC_P_Drain_Release
    ( const BVDC_P_DrainContext       *pDrain,
      BVDC_P_Resource_Handle           hResource )
{
    /* Sanity check and get context */
    BDBG_OBJECT_ASSERT(hResource, BVDC_RES);

    if(pDrain->eVnetFDrainType != BVDC_P_ResourceType_eInvalid)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource,
            pDrain->eVnetFDrainType, pDrain->ulVnetFDrainId);
    }

    if(pDrain->eVnetBDrainType != BVDC_P_ResourceType_eInvalid)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource,
            pDrain->eVnetBDrainType, pDrain->ulVnetBDrainId);
    }

    return;
}

/***************************************************************************
 *
 */
void BVDC_P_Drain_BuildRul_isr
    ( const BVDC_P_DrainContext       *pDrain,
      BVDC_P_ListInfo                 *pList )
{
    /* Build VNET_F */
    if(pDrain->eVnetFDrainType == BVDC_P_ResourceType_eDrainFrn)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VNET_F_DRAIN_0_SRC + pDrain->ulVnetFDrainId * 4);
        *pList->pulCurrent++ = pDrain->eVnetFDrainSrc;
    }
    else if(pDrain->eVnetFDrainType == BVDC_P_ResourceType_eFreeCh)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VNET_F_FCH_0_SRC + pDrain->ulVnetFDrainId * 4);
        *pList->pulCurrent++ = pDrain->eVnetFDrainSrc;
    }

    /* Build VNET_B */
    if(pDrain->eVnetBDrainType == BVDC_P_ResourceType_eDrainBck)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VNET_B_DRAIN_0_SRC + pDrain->ulVnetBDrainId * 4);
        *pList->pulCurrent++ = pDrain->eVnetBDrainSrc;
    }
    else if(pDrain->eVnetBDrainType == BVDC_P_ResourceType_eVnetCrc)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VNET_B_CRC_SRC + pDrain->ulVnetBDrainId * 4);
        *pList->pulCurrent++ = pDrain->eVnetBDrainSrc;
    }

    return;
}


/***************************************************************************
 * Build Format
 *
 */
void BVDC_P_Drain_BuildFormatRul_isr
    ( const BVDC_P_DrainContext       *pDrain,
      const BVDC_P_Rect               *pScanOut,
      const BFMT_VideoInfo            *pFmtInfo,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t   ulWidth, ulHeight;

    BDBG_ASSERT(pScanOut);
    BDBG_ASSERT(pFmtInfo);

    ulWidth = pScanOut->ulWidth;
    ulHeight = pScanOut->ulHeight >> pFmtInfo->bInterlaced;

#if (BVDC_P_DRAIN_PIC_XSIZE_DUAL_PIXEL_WORKAROUND)
    ulWidth /= 2;
#endif
    BDBG_MSG(("pScanOut: %dx%d fmt: %s - %dx%d", pScanOut->ulWidth,
        pScanOut->ulHeight, pFmtInfo->pchFormatStr, ulWidth, ulHeight));

    if(BVDC_P_DRN_HAS_DEBUG(pDrain))
    {
#if (BVDC_P_SUPPORT_DRAIN_VER == BVDC_P_VNET_VER_3)
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VNET_F_DRAIN_0_DEBUG_CTRL + pDrain->ulDbgOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VNET_F_DRAIN_0_DEBUG_CTRL, VIDEO_3D_MODE, pFmtInfo->eOrientation ) |
            BCHP_FIELD_DATA(VNET_F_DRAIN_0_DEBUG_CTRL, EXP_PIC_XSIZE, ulWidth ) |
            BCHP_FIELD_DATA(VNET_F_DRAIN_0_DEBUG_CTRL, EXP_PIC_YSIZE, ulHeight);

#elif ((BVDC_P_SUPPORT_DRAIN_VER == BVDC_P_VNET_VER_2) || \
       (BVDC_P_SUPPORT_DRAIN_VER >= BVDC_P_VNET_VER_4))
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VNET_F_DRAIN_0_DEBUG_CTRL + pDrain->ulDbgOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VNET_F_DRAIN_0_DEBUG_CTRL, BVB_VIDEO, pFmtInfo->eOrientation ) |
            BCHP_FIELD_DATA(VNET_F_DRAIN_0_DEBUG_CTRL, EXP_PIC_XSIZE, ulWidth ) |
            BCHP_FIELD_DATA(VNET_F_DRAIN_0_DEBUG_CTRL, EXP_PIC_YSIZE, ulHeight);

#elif (BVDC_P_SUPPORT_DRAIN_VER >= BVDC_P_VNET_VER_1)
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VNET_F_DRAIN_0_EXP_PIC_SIZE + pDrain->ulDbgOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VNET_F_DRAIN_0_EXP_PIC_SIZE, EXP_PIC_XSIZE, ulWidth) |
            BCHP_FIELD_DATA(VNET_F_DRAIN_0_EXP_PIC_SIZE, EXP_PIC_YSIZE, ulHeight);
#else
        BSTD_UNUSED(pList);
        BSTD_UNUSED(pDrain);
        BSTD_UNUSED(pFmtInfo);
        BSTD_UNUSED(pScanOut);
#endif
    }

    return;
}

/* End of file. */
