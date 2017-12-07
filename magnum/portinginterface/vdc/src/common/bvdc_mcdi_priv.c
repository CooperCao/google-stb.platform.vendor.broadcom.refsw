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
#include "bstd.h"
#include "bkni.h"
#include "brdc.h"

#include "bvdc.h"
#include "bvdc_mcdi_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_compositor_priv.h"

#ifndef BVDC_P_MCDI_DBG_MSG
#define BVDC_P_MCDI_DBG_MSG                       (0)
#endif
#if BVDC_P_MCDI_DBG_MSG
#define BVDC_P_MCDI_MSG                                BDBG_MSG
#else
#define BVDC_P_MCDI_MSG(a)
#endif

/* Nicer formating and ensure correct location for enum indexing. */
#define BVDC_P_MAKE_GAMEMODE_INFO(mode, delay, buffer_cnt)        \
{                                                                 \
    (BVDC_MadGameMode_##mode), (delay), (buffer_cnt), (#mode)     \
}

/* Mcdi game mode delay tables */
static const BVDC_P_McdiGameModeInfo s_aMcdiGameModeInfo[] =
{
    BVDC_P_MAKE_GAMEMODE_INFO(eOff,                   2, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e5Fields_2Delay,        2, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e5Fields_1Delay,        1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e5Fields_0Delay,        0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e5Fields_ForceSpatial,  0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e4Fields_2Delay,        2, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e4Fields_1Delay,        1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e4Fields_0Delay,        0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e4Fields_ForceSpatial,  0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e3Fields_2Delay,        2, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e3Fields_1Delay,        1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e3Fields_0Delay,        0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e3Fields_ForceSpatial,  0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(eMinField_ForceSpatial, 0, 1),
};
#if (BVDC_P_SUPPORT_MADR_VER > BVDC_P_MADR_VER_6)
/* Madr game mode delay tables */
static const BVDC_P_McdiGameModeInfo s_aMadrGameModeInfo[] =
{
    BVDC_P_MAKE_GAMEMODE_INFO(eOff,                   1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e5Fields_2Delay,        1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e5Fields_1Delay,        1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e5Fields_0Delay,        0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e5Fields_ForceSpatial,  0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e4Fields_2Delay,        1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e4Fields_1Delay,        1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e4Fields_0Delay,        0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e4Fields_ForceSpatial,  0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e3Fields_2Delay,        1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e3Fields_1Delay,        1, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e3Fields_0Delay,        0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(e3Fields_ForceSpatial,  0, 4),
    BVDC_P_MAKE_GAMEMODE_INFO(eMinField_ForceSpatial, 0, 0),
};
#endif

#include "bchp_mdi_top_0.h"
#if (BVDC_P_SUPPORT_MCDI_VER != 0)
#include "bchp_mdi_fcb_0.h"
#endif
#include "bchp_mdi_ppb_0.h"
#include "bchp_mmisc.h"
#include "bchp_bvnf_intr2_5.h"
#include "bchp_mdi_fcn_0.h"

#if (BVDC_P_MCDI_VER_8 <= BVDC_P_SUPPORT_MCDI_VER)
#include "bchp_mdi_memc_0.h"
#endif
#if (!BVDC_P_SUPPORT_MCDI_SUPERSET)
#include "bchp_mdi_ppb_1.h"
#include "bchp_mdi_fcn_1.h"
#include "bchp_mdi_top_1.h"
#endif
#if (BVDC_P_SUPPORT_MCVP > 1)
#define BVDC_P_MAKE_MDI(pMcdi, id)                                                                    \
{                                                                                                     \
    (pMcdi)->ulRegOffset     = BCHP_MDI_TOP_##id##_REG_START - BCHP_MDI_TOP_0_REG_START;              \
    (pMcdi)->ulRegOffset1    = (id > 0)?(BCHP_MDI_TOP_##id##_REG_START - BCHP_MDI_TOP_1_REG_START):0; \
}
#else
#define BVDC_P_MAKE_MDI(pMcdi, id)                                                                    \
{                                                                                                     \
    (pMcdi)->ulRegOffset     = BCHP_MDI_TOP_##id##_REG_START - BCHP_MDI_TOP_0_REG_START;              \
    (pMcdi)->ulRegOffset1    = 0;                                                                     \
}
#endif
BDBG_MODULE(BVDC_MCDI);
BDBG_FILE_MODULE(BVDC_DEINTERLACER_MOSAIC);
BDBG_OBJECT_ID(BVDC_MDI);

/***************************************************************************
* {private}
*
*/
BERR_Code BVDC_P_Mcdi_Create
    ( BVDC_P_Mcdi_Handle           *phMcdi,
      BVDC_P_McdiId                 eMcdiId,
      BREG_Handle                   hRegister,
      BVDC_P_Resource_Handle        hResource )
{
    BVDC_P_McdiContext *pMcdi;

    BDBG_ENTER(BVDC_P_Mcdi_Create);

    BDBG_ASSERT(phMcdi);

    /* (1) Alloc the context. */
    pMcdi = (BVDC_P_McdiContext*)
        (BKNI_Malloc(sizeof(BVDC_P_McdiContext)));
    if(!pMcdi)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pMcdi, 0x0, sizeof(BVDC_P_McdiContext));
    BDBG_OBJECT_SET(pMcdi, BVDC_MDI);

    pMcdi->eId              = eMcdiId;
    pMcdi->hRegister        = hRegister;
    pMcdi->ulRegOffset      = 0;
    pMcdi->ulRegOffset1      = 0;


    switch(pMcdi->eId)
    {
        case BVDC_P_McdiId_eMcdi0:
            BVDC_P_MAKE_MDI(pMcdi, 0);
            break;
#if (BVDC_P_SUPPORT_MCVP > 1)
        case BVDC_P_McdiId_eMcdi1:
            BVDC_P_MAKE_MDI(pMcdi, 1);
            break;
#endif
#if (BVDC_P_SUPPORT_MCVP > 2)
        case BVDC_P_McdiId_eMcdi2:
            BVDC_P_MAKE_MDI(pMcdi, 2);
            break;
#endif
#if (BVDC_P_SUPPORT_MCVP > 3)
        case BVDC_P_McdiId_eMcdi3:
            BVDC_P_MAKE_MDI(pMcdi, 3);
            break;
#endif
#if (BVDC_P_SUPPORT_MCVP > 4)
        case BVDC_P_McdiId_eMcdi4:
            BVDC_P_MAKE_MDI(pMcdi, 4);
            break;
#endif
#if (BVDC_P_SUPPORT_MCVP > 5)
        case BVDC_P_McdiId_eMcdi5:
            BVDC_P_MAKE_MDI(pMcdi, 5);
            break;
#endif
        default:
            BDBG_ERR(("Need to handle BVDC_P_McdiId_eMcdi%d", pMcdi->eId));
            BDBG_ASSERT(0);
            break;
    }

    /* init the SubRul sub-module */
    BVDC_P_SubRul_Init(&(pMcdi->SubRul), BVDC_P_Mcdi_MuxAddr(pMcdi),
        BVDC_P_Mcdi_PostMuxValue(pMcdi), BVDC_P_DrainMode_eBack, 0, hResource);

    /* All done. now return the new fresh context to user. */
    *phMcdi = (BVDC_P_Mcdi_Handle)pMcdi;

    BDBG_LEAVE(BVDC_P_Mcdi_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
* {private}
*
*/
void BVDC_P_Mcdi_Destroy
    ( BVDC_P_Mcdi_Handle             hMcdi )
{
    BDBG_ENTER(BVDC_P_Mcdi_Destroy);
    BDBG_OBJECT_ASSERT(hMcdi, BVDC_MDI);

    /* [1] Free the context. */
    BDBG_ASSERT(NULL == hMcdi->apHeapNode[0][0]);
    BDBG_OBJECT_DESTROY(hMcdi, BVDC_MDI);
    BKNI_Free((void*)hMcdi);

    BDBG_LEAVE(BVDC_P_Mcdi_Destroy);
    return;
}


/***************************************************************************
* {private}
*
*/
void BVDC_P_Mcdi_Init_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_Window_Handle             hWindow)
{
    uint32_t  ulReg;
    uint32_t i, j;

    BDBG_ENTER(BVDC_P_Mcdi_Init_isr);
    BDBG_OBJECT_ASSERT(hMcdi, BVDC_MDI);

#ifdef BCHP_MDI_TOP_0_HW_CONFIGURATION
    ulReg = BREG_Read32_isr(hMcdi->hRegister, BCHP_MDI_TOP_0_HW_CONFIGURATION + hMcdi->ulRegOffset);
    hMcdi->bMadr = BVDC_P_COMPARE_FIELD_NAME(ulReg, MDI_TOP_0_HW_CONFIGURATION, TYPE, MADR);
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
    hMcdi->ulMosaicMaxChannels = BVDC_P_GET_FIELD(ulReg, MDI_TOP_0_HW_CONFIGURATION, MULTIPLE_CONTEXT);
    hMcdi->ulMosaicMaxChannels += (hMcdi->ulMosaicMaxChannels!=0);
#endif
#else
    BSTD_UNUSED(ulReg);
    hMcdi->bMadr = false;
#endif

    hMcdi->bInitial              = true;
    hMcdi->bEnableOsd            = false;
    hMcdi->ulOsdHpos             = 0;
    hMcdi->ulOsdVpos             = 0;
    hMcdi->pstCompression = &hWindow->stMadCompression;

    for (i=0; i<BAVC_MOSAIC_MAX;i++)
    {
        for (j=0; j<BVDC_P_MAX_MCDI_BUFFER_COUNT;j++)
        {
            hMcdi->apHeapNode[i][j]=NULL;
        }
        hMcdi->apQmHeapNode[i][0] = NULL;
        hMcdi->usTrickModeStartDelay[i] = 0;
    }
    BDBG_LEAVE(BVDC_P_Mcdi_Init_isr);

    return;
}



/***************************************************************************
* {private}
*  MADR Node Init
*/
static void BVDC_P_Mcdi_BuildRul_Madr_NodeInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    uint32_t ulRegOffset;
    uint32_t ulSize, ulQmBufSize;
    BMMA_DeviceOffset ullDeviceAddr, ullPixAddr[4], ullQmAddr[4];
    int ii, ulBufCount;
    bool    bIsBufContinuous = pPicture->bContinuous;
    bool    bForceSpatial = BVDC_P_MAD_SPATIAL(hMcdi->eGameMode);
    uint32_t ulChannelId = pPicture->ulPictureIdx;

    ulRegOffset = hMcdi->ulRegOffset;

    /* ********* Step 1: pixel buffer ********* */
    if(bForceSpatial)
    {
        ullPixAddr[0] = ullPixAddr[1] = ullPixAddr[2]= ullPixAddr[3]= ullDeviceAddr = 0;
    }
    else /* no force spatial case */
    {

        ullDeviceAddr = ullPixAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][0]);
        ulSize = hMcdi->ulPxlBufSize[ulChannelId];
        BDBG_ASSERT(ulSize);

        if(!bIsBufContinuous)
        {
            /* address can be discountinued */
            ullPixAddr[1] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][1]);
            ullPixAddr[2] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][2]);
#if (BVDC_P_MADR_VER_7 <= BVDC_P_SUPPORT_MADR_VER)
            ullPixAddr[3] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][3]);
#else
            BSTD_UNUSED(ullPixAddr[3]);
#endif
        }
        else
        {
#if (BVDC_P_MADR_VER_7 <= BVDC_P_SUPPORT_MADR_VER)
            ulBufCount = 4;
#else
            ulBufCount = 3;
#endif

            ullPixAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullPixAddr[0]+ ulSize, BVDC_P_PITCH_ALIGN);
            ullPixAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullPixAddr[1]+ ulSize, BVDC_P_PITCH_ALIGN);
#if (BVDC_P_MADR_VER_7 <= BVDC_P_SUPPORT_MADR_VER)
            ullPixAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullPixAddr[2]+ ulSize, BVDC_P_PITCH_ALIGN);
#endif
        }
    }

    /* set bufs addr into reg */
    BRDC_AddrRul_ImmsToRegs_isr(&pList->pulCurrent,
        BCHP_MDI_TOP_0_PIXEL_FIELD_MSTART_0 + ulRegOffset,
#if (BVDC_P_MADR_VER_7 <= BVDC_P_SUPPORT_MADR_VER)
        BCHP_MDI_TOP_0_PIXEL_FIELD_MSTART_3 + ulRegOffset, ullPixAddr);
#else
        BCHP_MDI_TOP_0_PIXEL_FIELD_MSTART_2 + ulRegOffset, ullPixAddr);
#endif


    /* ********* Step 2: QM buffer ********* */
    ulBufCount = BVDC_P_MAD_QM_FIELD_STORE_COUNT / BVDC_P_MAD_QM_BUFFER_COUNT;
    if(!bForceSpatial)
    {
        ullDeviceAddr = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apQmHeapNode[ulChannelId][0]);
        ulSize = hMcdi->apQmHeapNode[ulChannelId][0]->pHeapInfo->ulBufSize / ulBufCount;
    }
    else
    {
        ullDeviceAddr = ulSize = 0;
    }

    for(ii = 0; ii < ulBufCount; ii++)
    {
        /* Make sure address is 32 byte alligned */
        ullQmAddr[ii] = BVDC_P_ADDR_ALIGN_UP(ullDeviceAddr + ii * ulSize, BVDC_P_PITCH_ALIGN);
    }

    if(bForceSpatial)
    {
        ullQmAddr[0] = ullQmAddr[1] = ullQmAddr[2] = ullQmAddr[3] = 0;
    }
    else
    {
        ulQmBufSize = hMcdi->apQmHeapNode[ulChannelId][0]->pHeapInfo->ulBufSize / ulBufCount;
        ullQmAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apQmHeapNode[ulChannelId][0]);
        ullQmAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[0] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
        ullQmAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[1] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
        ullQmAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[2] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
    }


    BRDC_AddrRul_ImmsToRegs_isr(&pList->pulCurrent,
        BCHP_MDI_TOP_0_QM_FIELD_MSTART_1 + ulRegOffset,
        BCHP_MDI_TOP_0_QM_FIELD_MSTART_4 + ulRegOffset, ullQmAddr);

}


/***************************************************************************
* {private}
*  MCDI FCB Init
*/
static void BVDC_P_Mcdi_BuildRul_Madr_FcnInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    bool     bIsHD;
    uint32_t ulRegVal;
#if (BVDC_P_SUPPORT_MCDI_SUPERSET)
    uint32_t ulRegOffset = hMcdi->ulRegOffset;
    uint32_t ulRegCalcCtrl0 = BCHP_MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_0;
#if (BVDC_P_SUPPORT_MADR_VER >= BVDC_P_MADR_VER_8)
    uint32_t ulRegCalcCtrl3  = BCHP_MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_3;
    uint32_t ulRegCalcCtrl5  = BCHP_MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_5;
    uint32_t ulRegCalcCtrl9  = BCHP_MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_9;
    uint32_t ulRegCalcCtrl11 = BCHP_MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_11;
    uint32_t ulRegCalcCtrl18 = BCHP_MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_18;
#endif
    uint32_t ulModeCtrl0    = BCHP_MDI_FCN_0_MODE_CONTROL_0;
#else
    uint32_t ulRegOffset = hMcdi->ulRegOffset1;
    uint32_t ulRegCalcCtrl0  = BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_0;
    uint32_t ulRegCalcCtrl3  = BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_3;
    uint32_t ulRegCalcCtrl5  = BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_5;
    uint32_t ulRegCalcCtrl9  = BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_9;
    uint32_t ulRegCalcCtrl11 = BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_11;
    uint32_t ulRegCalcCtrl18 = BCHP_MDI_FCN_1_IT_FIELD_PHASE_CALC_CONTROL_18;
    uint32_t ulModeCtrl0    = BCHP_MDI_FCN_1_MODE_CONTROL_0;
#endif
    bool    bForceSpatial = BVDC_P_MAD_SPATIAL(hMcdi->eGameMode);

    uint32_t ulHSize = pPicture->pMadIn->ulWidth;
    bIsHD = (ulHSize >= BFMT_720P_WIDTH);

    /* set MADR ENTER_LOCK_LEVEL/EXIT_LOCK_LEVEL*/
    BVDC_P_SUBRUL_START_BLOCK(pList, ulRegCalcCtrl0, ulRegOffset,
        BVDC_P_REGS_ENTRIES(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_0, MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_2));

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_0 */
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_PHASE_MATCH_THRESH, BVDC_P_MADR_REV32_PHASE_MATCH_THRESH) |
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_ENTER_LOCK_LEVEL,   BVDC_P_MADR_REV32_ENTER_LOCK_LEVEL  ) |
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_EXIT_LOCK_LEVEL ,   BVDC_P_MADR_REV32_EXIT_LOCK_LEVEL);

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_1 */
    *pList->pulCurrent++ = bIsHD ?
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_1, REV32_REPF_VETO_LEVEL, BVDC_P_MADR_REV32_REPF_VETO_LEVEL_HD):
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_1, REV32_REPF_VETO_LEVEL, BVDC_P_MADR_REV32_REPF_VETO_LEVEL_SD);

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_2 */
    ulRegVal  = BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_2, REV32_LOCK_SAT_LEVEL,  BVDC_P_MADR_REV32_LOCK_SAT_LEVEL);
    ulRegVal |= bIsHD ?
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_2, REV32_BAD_EDIT_LEVEL ,   BVDC_P_MADR_REV32_BAD_EDIT_LEVEL_HD):
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_2, REV32_BAD_EDIT_LEVEL ,   BVDC_P_MADR_REV32_BAD_EDIT_LEVEL_SD);
    *pList->pulCurrent++ = ulRegVal;

#if (BVDC_P_SUPPORT_MADR_VER <= BVDC_P_MADR_VER_7)
    /* set MADR OBTS                      */
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_MDI_FCN_0_OBTS_DECAY, ulRegOffset,
        BVDC_P_REGS_ENTRIES(MDI_FCN_0_OBTS_DECAY, MDI_FCN_0_MODE_CONTROL_0));
    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_FCN_0_OBTS_DECAY  , VALUE, BVDC_P_MADR_OBTS_DECAY);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_FCN_0_OBTS_HOLDOFF, VALUE, BVDC_P_MADR_OBTS_HOLDOFF);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_FCN_0_OBTS_MAX_HOLDOFF, VALUE, BVDC_P_MADR_OBTS_MAX_HOLDOFF);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(MDI_FCN_0_OBTS_CONTROL, VIDEO_ENABLE, ON                  ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_OBTS_CONTROL, IT_ENABLE, ON                     ) |
        BCHP_FIELD_DATA(MDI_FCN_0_OBTS_CONTROL, OFFSET, BVDC_P_MCDI_ZERO          ) |
        BCHP_FIELD_DATA(MDI_FCN_0_OBTS_CONTROL, CORE,   BVDC_P_MADR_OBTS_CTRL_CORE);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, PIC_ALWAYS_AVAIL         , OFF             ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, REV32_BAD_EDIT_PHASE_VETO, ON              ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, ALT_ENTER_LOCK_ENABLE    , OFF             ) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, BUFFER_SEL               , 1               ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, FIELD_STATE_UPDATE_SEL_0 , FIELD_STATE_FIFO) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, FIELD_STATE_UPDATE_SEL_1 , IT_BLOCK        ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, HARD_START_SEL           , FREEZE_FRAME    ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, FIELD_G_ENABLE           , ON              ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, FIELD_J_ENABLE           , ON              ) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, PIXEL_CAP_ENABLE         , !bForceSpatial) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, GLOBAL_FIELD_L2_ENABLE   , !bForceSpatial) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, GLOBAL_FIELD_K_ENABLE    , !bForceSpatial) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, GLOBAL_FIELD_K2_ENABLE   , !bForceSpatial);

    BSTD_UNUSED(ulModeCtrl0);
#else
    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_3 */
    BVDC_P_SUBRUL_ONE_REG(pList, ulRegCalcCtrl3, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_3, REV22_NONMATCH_MATCH_RATIO, BVDC_P_MADR_REV22_NONMATCH_MATCH_RATIO) |
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_3, REV22_ENTER_LOCK_LEVEL,     BVDC_P_MADR_REV22_ENTER_LOCK_LEVEL  ) |
        BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_3, REV22_EXIT_LOCK_LEVEL ,     BVDC_P_MADR_REV22_EXIT_LOCK_LEVEL));
    BVDC_P_SUBRUL_START_BLOCK(pList, ulRegCalcCtrl5, ulRegOffset,
         BVDC_P_REGS_ENTRIES(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_5, MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_8));

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_5 */
    ulRegVal = bIsHD?
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_5, REV22_UPPER_MATCH_THRESH,         BVDC_P_MADR_REV22_HIGHER_THRESHOLD_HD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_5, REV22_LOWER_NONMATCH_THRESH ,     BVDC_P_MADR_REV22_LOWER_THRESHOLD_HD)) :
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_5, REV22_UPPER_MATCH_THRESH,         BVDC_P_MADR_REV22_HIGHER_THRESHOLD_SD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_5, REV22_LOWER_NONMATCH_THRESH ,     BVDC_P_MADR_REV22_LOWER_THRESHOLD_SD));
    *pList->pulCurrent++ = ulRegVal;

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_8 */
    ulRegVal = bIsHD?
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_8, REV32_MIN_SIGMA_RANGE ,       BVDC_P_MADR_REV32_MIN_SIGMA_RANGE_HD) :
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_8, REV32_MIN_SIGMA_RANGE ,       BVDC_P_MADR_REV32_MIN_SIGMA_RANGE_SD);
    ulRegVal |= BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_8, REV32_LOCK_SAT_THRESH, BVDC_P_MADR_REV32_LOCK_SAT_THRESH);
    *pList->pulCurrent++ = ulRegVal;
    BVDC_P_SUBRUL_START_BLOCK(pList, ulRegCalcCtrl9, ulRegOffset,
         BVDC_P_REGS_ENTRIES(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_9, MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_10));
    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_9 */
    ulRegVal = bIsHD?
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_9, REV32_P0_FEATHERING_MAX, BVDC_P_MADR_REV32_P0_MAX_HD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_9, REV32_T_T1_FEATH_RATIO , BVDC_P_MADR_REV32_T1_RATIO)|
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_9, REV32_T_T1_FEATH_MIN ,   BVDC_P_MADR_REV32_T1_MIN_HD)) :
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_9, REV32_P0_FEATHERING_MAX, BVDC_P_MADR_REV32_P0_MAX_SD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_9, REV32_T_T1_FEATH_RATIO , BVDC_P_MADR_REV32_T1_RATIO)|
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_9, REV32_T_T1_FEATH_MIN ,   BVDC_P_MADR_REV32_T1_MIN_SD));
    *pList->pulCurrent++ = ulRegVal;

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_10 */
    ulRegVal = bIsHD?
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_10, REV32_BW_FEATHERING_MAX,      BVDC_P_MADR_REV32_BW_MAX_HD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_10, REV32_BW_FEATHERING_FF_DIFF , BVDC_P_MADR_REV32_BW_FF_DIFF_HD)) :
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_10, REV32_BW_FEATHERING_MAX,      BVDC_P_MADR_REV32_BW_MAX_SD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_10, REV32_BW_FEATHERING_FF_DIFF , BVDC_P_MADR_REV32_BW_FF_DIFF_SD));
    *pList->pulCurrent++ = ulRegVal;

    /* address hole between MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_10 and MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_11 */
    BVDC_P_SUBRUL_START_BLOCK(pList, ulRegCalcCtrl11, ulRegOffset,
         BVDC_P_REGS_ENTRIES(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_11, MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_15));
    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_11 */
    ulRegVal = bIsHD?
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_11, LG_PCC_THRESH,    BVDC_P_MADR_LG_PCC_THRESHOLD_HD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_11, TICKER_THRESH ,   BVDC_P_MADR_TICKER_THRESHOLD_HD)) :
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_11, LG_PCC_THRESH,    BVDC_P_MADR_LG_PCC_THRESHOLD_SD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_11, TICKER_THRESH,    BVDC_P_MADR_TICKER_THRESHOLD_SD));
    *pList->pulCurrent++ = ulRegVal;

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_12 */
    ulRegVal = bIsHD?
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_12, REV22_SAME_FRAME_THRESH,      BVDC_P_MADR_REV22_SAMEF_THRESHOLD_HD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_12, REV22_SAME_FRAME_DTHRESH ,    BVDC_P_MADR_REV22_SAMEF_DTHRESHOLD_HD)) :
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_12, REV22_SAME_FRAME_THRESH,      BVDC_P_MADR_REV22_SAMEF_THRESHOLD_SD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_12, REV22_SAME_FRAME_DTHRESH,     BVDC_P_MADR_REV22_SAMEF_DTHRESHOLD_SD));
    *pList->pulCurrent++ = ulRegVal;

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_13 */
    ulRegVal = bIsHD?
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_13, REV22_SAME_FRAME_PCC_VETO,    BVDC_P_MADR_REV22_SAMEF_PCCVETO_HD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_13, REV22_UNDO_SWAP_LEVEL ,       BVDC_P_MADR_REV22_UNDO_SWAP_HD)) :
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_13, REV22_SAME_FRAME_PCC_VETO,    BVDC_P_MADR_REV22_SAMEF_PCCVETO_SD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_13, REV22_UNDO_SWAP_LEVEL,        BVDC_P_MADR_REV22_UNDO_SWAP_SD));
    *pList->pulCurrent++ = ulRegVal;

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_14 */
    ulRegVal = bIsHD?
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_14, REV22_BW_STAIR_THRESH,    BVDC_P_MADR_REV22_BW_STAIR_HD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_14, REV22_BW_PCC_MAXIMUM ,    BVDC_P_MADR_REV22_BW_PCC_MAX_HD)) :
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_14, REV22_BW_STAIR_THRESH,    BVDC_P_MADR_REV22_BW_STAIR_SD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_14, REV22_BW_PCC_MAXIMUM,     BVDC_P_MADR_REV22_BW_PCC_MAX_SD));
    *pList->pulCurrent++ = ulRegVal;

    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_15 */
    ulRegVal = bIsHD?
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_15, REV22_BW_FEATHERING_MAX,       BVDC_P_MADR_REV22_BW_MAX_HD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_15, REV22_BW_FEATHERING_FF_DIFF ,  BVDC_P_MADR_REV22_BW_FF_DIFF_HD)) :
        (BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_15, REV22_BW_FEATHERING_MAX,       BVDC_P_MADR_REV22_BW_MAX_SD) |
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_15, REV22_BW_FEATHERING_FF_DIFF,   BVDC_P_MADR_REV22_BW_FF_DIFF_SD));
    *pList->pulCurrent++ = ulRegVal;
    /* MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_18 */
    ulRegVal = bIsHD?
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_18, REV22_T_T1_FEATH_MIN , BVDC_P_MADR_REV22_T1_MIN_HD) :
         BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_18, REV22_T_T1_FEATH_MIN , BVDC_P_MADR_REV22_T1_MIN_SD);
    ulRegVal |= BCHP_FIELD_DATA(MDI_FCN_0_IT_FIELD_PHASE_CALC_CONTROL_18, REV22_T_T1_FEATH_RATIO, BVDC_P_MADR_REV22_P0_MAX);

    BVDC_P_SUBRUL_ONE_REG(pList, ulRegCalcCtrl18, ulRegOffset, ulRegVal);

    BVDC_P_SUBRUL_ONE_REG(pList, ulModeCtrl0, ulRegOffset,
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, PIC_ALWAYS_AVAIL         , OFF             ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, REV32_BAD_EDIT_PHASE_VETO, ON              ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, ALT_ENTER_LOCK_ENABLE    , OFF             ) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, BUFFER_SEL               , 1               ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, FIELD_STATE_UPDATE_SEL_0 , FIELD_STATE_FIFO) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, FIELD_STATE_UPDATE_SEL_1 , IT_BLOCK        ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, HARD_START_SEL           , FREEZE_FRAME    ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, FIELD_G_ENABLE           , ON              ) |
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_0, FIELD_J_ENABLE           , ON              ) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, PIXEL_CAP_ENABLE         , !bForceSpatial) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, GLOBAL_FIELD_L2_ENABLE   , !bForceSpatial) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, GLOBAL_FIELD_K_ENABLE    , !bForceSpatial) |
        BCHP_FIELD_DATA(MDI_FCN_0_MODE_CONTROL_0, GLOBAL_FIELD_K2_ENABLE   , !bForceSpatial));
#endif
    return;
}


/***************************************************************************
* {private}
*  MCDI PPB Init
*/
static void BVDC_P_Mcdi_BuildRul_Madr_PpbInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    uint32_t ulRegOffset, ulRegLaScale0, ulRegStatsRange, ulRegPccCtrl;
    uint32_t ulVSize;
#if ( (BVDC_P_SUPPORT_MADR_VER >= BVDC_P_MADR_VER_2) && (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8))
    uint32_t ulQMRange;
#endif

#if (BVDC_P_SUPPORT_MCDI_SUPERSET)
    ulRegOffset = hMcdi->ulRegOffset;
    ulRegLaScale0  = BCHP_MDI_PPB_0_LA_SCALE_0;
    ulRegStatsRange = BCHP_MDI_PPB_0_STATS_RANGE;
    ulRegPccCtrl    = BCHP_MDI_PPB_0_IT_PCC_CONTROL;
#else
    ulRegOffset = hMcdi->ulRegOffset1;
    ulRegLaScale0  = BCHP_MDI_PPB_1_LA_SCALE_0;
    ulRegStatsRange = BCHP_MDI_PPB_1_STATS_RANGE;
    ulRegPccCtrl    = BCHP_MDI_PPB_1_IT_PCC_CONTROL;
#endif

    ulVSize = pPicture->pMadIn->ulHeight >>(pPicture->PicComRulInfo.eSrcOrigPolarity != BAVC_Polarity_eFrame);

    BVDC_P_SUBRUL_ONE_REG(pList, ulRegStatsRange,  ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_STATS_RANGE, END_LINE, ulVSize - 1));

#if ( (BVDC_P_SUPPORT_MADR_VER >= BVDC_P_MADR_VER_2) && (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8))
    ulQMRange = BCHP_FIELD_DATA(MDI_PPB_0_QM_MAPPING_RANGE, VALUE_3, BVDC_P_MADR_QM_MAPPING_RANGE_VALUE_3)  |
                BCHP_FIELD_DATA(MDI_PPB_0_QM_MAPPING_RANGE, VALUE_2, BVDC_P_MADR_QM_MAPPING_RANGE_VALUE_2)  |
                BCHP_FIELD_DATA(MDI_PPB_0_QM_MAPPING_RANGE, VALUE_1, BVDC_P_MADR_QM_MAPPING_RANGE_VALUE_1);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_QM_MAPPING_RANGE, ulRegOffset, ulQMRange);

        /* jira SW 7231-110*/
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_OBTS_CONTROL_0, ulRegOffset,
        BCHP_FIELD_ENUM(MDI_PPB_0_OBTS_CONTROL_0, OBTS_CHROMA_FILTER_ENABLE, ON) |
        BCHP_FIELD_ENUM(MDI_PPB_0_OBTS_CONTROL_0, OBTS_FILTER_ENABLE,        ON));
#endif

    BVDC_P_SUBRUL_ONE_REG(pList, ulRegLaScale0, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_LA_SCALE_0, DEGREE_7, BVDC_P_MADR_LA_SCALE_0_DEGREE) |
        BCHP_FIELD_DATA(MDI_PPB_0_LA_SCALE_0, DEGREE_6, BVDC_P_MADR_LA_SCALE_0_DEGREE) |
        BCHP_FIELD_DATA(MDI_PPB_0_LA_SCALE_0, DEGREE_5, BVDC_P_MADR_LA_SCALE_0_DEGREE) |
        BCHP_FIELD_DATA(MDI_PPB_0_LA_SCALE_0, DEGREE_4, BVDC_P_MADR_LA_SCALE_0_DEGREE) |
        BCHP_FIELD_DATA(MDI_PPB_0_LA_SCALE_0, DEGREE_3, BVDC_P_MADR_LA_SCALE_0_DEGREE) |
        BCHP_FIELD_DATA(MDI_PPB_0_LA_SCALE_0, DEGREE_2, BVDC_P_MADR_LA_SCALE_0_DEGREE) |
        BCHP_FIELD_DATA(MDI_PPB_0_LA_SCALE_0, DEGREE_1, BVDC_P_MADR_LA_SCALE_0_DEGREE) |
        BCHP_FIELD_DATA(MDI_PPB_0_LA_SCALE_0, DEGREE_0, BVDC_P_MADR_LA_SCALE_0_DEGREE));

#if (BVDC_P_SUPPORT_MADR_VER < BVDC_P_MADR_VER_8)
    BSTD_UNUSED(pPicture);
    BSTD_UNUSED(ulRegPccCtrl);
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_XCHROMA_CONTROL_4, ulRegOffset,
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_4, IT_CHROMA_SEL, IT_DIRECT)                    |
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_4, SIMILAR_MAX  , BVDC_P_MADR_XCHROMA_CTRL_MAX) |
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_4, MAX_XCHROMA  , BVDC_P_MADR_XCHROMA_CTRL_MAX));
#else
    BVDC_P_SUBRUL_ONE_REG(pList, ulRegPccCtrl, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_IT_PCC_CONTROL, IT_STATS_MIN_MOTION,                               5)|
        BCHP_FIELD_DATA(MDI_PPB_0_IT_PCC_CONTROL, TKR_CORING_THRESH,       BVDC_P_MADR_TKR_CORING_THRE)|
        BCHP_FIELD_DATA(MDI_PPB_0_IT_PCC_CONTROL, CHROMA_CORING_THRESH,   BVDC_P_MADR_XOMA_CORING_THRE)|
        BCHP_FIELD_DATA(MDI_PPB_0_IT_PCC_CONTROL, LUMA_CORING_THRESH,     BVDC_P_MADR_LUMA_CORING_THRE));
#endif
    return;
}


/***************************************************************************
* {private}
*  MADR src Init
*/
static void BVDC_P_Mcdi_BuildRul_Madr_SrcInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    uint32_t ulRegOffset;
    uint32_t ulHSize, ulVSize;
#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
    uint32_t   ulReminder, ulBitsPerGroup, ulPixelPerGroup;
#endif

    BDBG_OBJECT_ASSERT(hMcdi, BVDC_MDI);
    ulRegOffset = hMcdi->ulRegOffset;

    /* set pic size into reg */
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
    if((pPicture->bMosaicMode) &&(hMcdi->ulMosaicMaxChannels))
    {
        uint32_t ulChannelId = pPicture->ulPictureIdx;
        BDBG_ASSERT(ulChannelId < pPicture->ulMosaicCount);
#if (!BVDC_P_SUPPORT_MCDI_SUPERSET)
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_1_MULTI_CONTEXT_TO, hMcdi->ulRegOffset1,
            BCHP_FIELD_DATA(MDI_TOP_1_MULTI_CONTEXT_TO, STREAM_PROCESSED, ulChannelId));
#else
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_MULTI_CONTEXT_TO, ulRegOffset,
                BCHP_FIELD_DATA(MDI_TOP_0_MULTI_CONTEXT_TO, STREAM_PROCESSED, ulChannelId));
#endif
    }
#endif

    ulHSize = pPicture->pMadIn->ulWidth;
    ulVSize = pPicture->pMadIn->ulHeight >>(pPicture->PicComRulInfo.eSrcOrigPolarity != BAVC_Polarity_eFrame);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_SRC_PIC_SIZE, ulRegOffset,
        BCHP_FIELD_DATA(MDI_TOP_0_SRC_PIC_SIZE, HSIZE, ulHSize) |
        BCHP_FIELD_DATA(MDI_TOP_0_SRC_PIC_SIZE, VSIZE, ulVSize));

#if (BVDC_P_MADR_HSIZE_WORKAROUND)
    if(ulHSize % 4)
    {
        BDBG_ERR(("BVN hang with bad HSIZE (not multiple of 4): %d",
            hMcdi->ulHSize));
    }
#endif

#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
    ulReminder = BVDC_P_MADR_GET_REMAINDER(ulHSize, ulVSize);
    if(BVDC_P_MADR_BAD_ALIGNMENT(ulReminder))
    {
        BDBG_ERR(("BVN hang with bad MDI_TOP_0_SRC_PIC_SIZE: %dx%d: %d",
            ulHSize, ulVSize, ulReminder));
    }

    BDBG_ASSERT(hMcdi->pstCompression);
    ulBitsPerGroup = BVDC_P_MADR_DCXS_COMPRESSION(hMcdi->pstCompression->ulBitsPerGroup);
    ulPixelPerGroup = hMcdi->pstCompression->ulPixelPerGroup;
#if (BVDC_P_MADR_VARIABLE_RATE)
    ulReminder = BVDC_P_MADR_GET_VARIABLE_RATE_REMAINDER(ulHSize,
        ulVSize, ulBitsPerPixel);

    if(BVDC_P_MADR_VARIABLE_RATE_BAD_ALIGNMENT(ulReminder, ulBitsPerPixel))
    {
        BDBG_ERR(("BVN hang with bad MDI_TOP_0_SRC_PIC_SIZE for variable rate: %dx%dx%d: %d",
            ulHSize, ulVSize, ulBitsPerGroup, ulReminder));
    }
#else
    ulReminder = BVDC_P_MADR_GET_FIX_RATE_REMAINDER(ulHSize,
        ulVSize, ulBitsPerGroup, ulPixelPerGroup);

    if(BVDC_P_MADR_FIX_RATE_BAD_ALIGNMENT(ulReminder, ulBitsPerGroup, ulPixelPerGroup))
    {
        BDBG_WRN(("BVN hang with bad MDI_TOP_0_SRC_PIC_SIZE for fix rate: %dx%dx%d: %d",
            ulHSize, ulVSize, ulBitsPerGroup, ulReminder));
    }
#endif
#endif
    BVDC_P_Mcdi_BuildRul_Madr_NodeInit_isr(hMcdi, pList, pPicture);
    BVDC_P_Mcdi_BuildRul_Madr_FcnInit_isr (hMcdi, pList, pPicture);
    BVDC_P_Mcdi_BuildRul_Madr_PpbInit_isr (hMcdi, pList, pPicture);

    return;
}
static void BVDC_P_Mcdi_BuildRul_Madr_SetEnable_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture,
      bool                          bInit)
{
    bool bRev32Pulldown;
    uint16_t usTrickModeStartDelay;
    uint32_t  ulModeSel, ulFldType, ulTrickModeSel, ulWeaveMode;
    bool                           bRepeat;
    BAVC_Polarity                  eSrcNxtFldId;
    BAVC_FrameRateCode             eFrameRate;
    BFMT_Orientation               eOrientation;
    uint32_t                       ulChannelId;
    uint32_t                       ulRegOffset;
#if (BVDC_P_SUPPORT_MCDI_SUPERSET)
    uint32_t  ulRegOsd = BCHP_MDI_TOP_0_OSD_POSITION;
    uint32_t  ulRegModeCtrl0 = BCHP_MDI_TOP_0_MODE_CONTROL_0;
    uint32_t  ulRegModeCtrl1 = BCHP_MDI_FCN_0_MODE_CONTROL_1;
    uint32_t  ulRegItOutPut = BCHP_MDI_FCN_0_IT_OUTPUT_CONTROL;
    uint32_t  ulRegEnable = BCHP_MDI_TOP_0_ENABLE_CONTROL;
#else
    uint32_t  ulRegOsd = BCHP_MDI_TOP_1_OSD_POSITION;
    uint32_t  ulRegModeCtrl0 = BCHP_MDI_TOP_1_MODE_CONTROL_0;
    uint32_t  ulRegModeCtrl1 = BCHP_MDI_FCN_1_MODE_CONTROL_1;
    uint32_t  ulRegItOutPut = BCHP_MDI_FCN_1_IT_OUTPUT_CONTROL;
    uint32_t  ulRegEnable = BCHP_MDI_TOP_1_ENABLE_CONTROL;
#endif
    bool      bHardStart = false;

    uint32_t ulModeCtrl0 = 0;


    BDBG_OBJECT_ASSERT(hMcdi, BVDC_MDI);
#if (BVDC_P_SUPPORT_MCDI_SUPERSET)
    ulRegOffset  = hMcdi->ulRegOffset;
#else
    ulRegOffset  = hMcdi->ulRegOffset1;
#endif
    bRepeat      = pPicture->stFlags.bPictureRepeatFlag,
    eFrameRate   = pPicture->eFrameRateCode;
    eOrientation = BVDC_P_VNET_USED_MVP_AT_WRITER(pPicture->stVnetMode) ?
            pPicture->eSrcOrientation : pPicture->eDispOrientation;
    eSrcNxtFldId = pPicture->PicComRulInfo.eSrcOrigPolarity;
    ulChannelId = pPicture->ulPictureIdx;

    /*** MDI_FCN_0 ***/
    if(hMcdi->bEnableOsd)
    {
        BVDC_P_SUBRUL_ONE_REG(pList, ulRegOsd, ulRegOffset,
            BCHP_FIELD_DATA(MDI_TOP_0_OSD_POSITION, HPOS, hMcdi->ulOsdHpos) |
            BCHP_FIELD_DATA(MDI_TOP_0_OSD_POSITION, VPOS, hMcdi->ulOsdVpos));
    }
    bRev32Pulldown = (
        (hMcdi->bRev32Pulldown) &&
        (BAVC_FrameRateCode_e25 != eFrameRate));

    bHardStart = BVDC_P_Mcdi_BeHardStart_isr(bInit, hMcdi);

    /* due to HW requirement, FIELD_FREEZE can not be set for the first 4 fields */
    if(bHardStart)
    {
        hMcdi->usTrickModeStartDelay[ulChannelId] = BVDC_P_MCDI_TRICK_MODE_START_DELAY;
    }
    usTrickModeStartDelay = hMcdi->usTrickModeStartDelay[ulChannelId];
    hMcdi->usTrickModeStartDelay[ulChannelId] -= (usTrickModeStartDelay>0);

    ulModeSel = (bHardStart)?
            BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_1, MODE_SEL, HARD_START) :
            (BVDC_P_MAD_SPATIAL(hMcdi->eGameMode)?
            BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_1, MODE_SEL, FORCE_SPATIAL):
            BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_1, MODE_SEL, NORMAL));

    ulTrickModeSel = (bRepeat && (0 == usTrickModeStartDelay)) ?
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_1, TRICK_MODE_SEL, FIELD_FREEZE) :
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_1, TRICK_MODE_SEL, OFF);

    ulFldType = (BAVC_Polarity_eBotField == eSrcNxtFldId)?
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_1, FIELD_B_TYPE, BOTTOM) :
        BCHP_FIELD_ENUM(MDI_FCN_0_MODE_CONTROL_1, FIELD_B_TYPE, TOP);

    BVDC_P_SUBRUL_ONE_REG(pList, ulRegModeCtrl1, ulRegOffset,

        ulModeSel | ulFldType | ulTrickModeSel);

    BVDC_P_SUBRUL_ONE_REG(pList, ulRegItOutPut, ulRegOffset,
        ((hMcdi->bRev22Pulldown)
        ? BCHP_FIELD_ENUM(MDI_FCN_0_IT_OUTPUT_CONTROL, AUTOREV22_ENABLE,ON)
        : BCHP_FIELD_ENUM(MDI_FCN_0_IT_OUTPUT_CONTROL, AUTOREV22_ENABLE,OFF)) |
        ((bRev32Pulldown)
        ? BCHP_FIELD_ENUM(MDI_FCN_0_IT_OUTPUT_CONTROL, AUTOREV32_ENABLE,ON)
        : BCHP_FIELD_ENUM(MDI_FCN_0_IT_OUTPUT_CONTROL, AUTOREV32_ENABLE,OFF)));

#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8)
    /*** MDI_PPB_0 ***/
    ulWeaveMode = BCHP_FIELD_ENUM(MDI_PPB_0_MOTION_CAL_CONTROL, WEAVE_MOTION_ENABLE, ON)   |
                BCHP_FIELD_ENUM(MDI_PPB_0_MOTION_CAL_CONTROL, MA_SUBSAMPLING_ENABLE, ON)   |
                BCHP_FIELD_DATA(MDI_PPB_0_MOTION_CAL_CONTROL, WEAVE_DETECT_THR,      0x10) |
                BCHP_FIELD_DATA(MDI_PPB_0_MOTION_CAL_CONTROL, WEAVE_MOTION_THR_LOW,  0x16);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MOTION_CAL_CONTROL, ulRegOffset,
        ulWeaveMode);
#else
    BSTD_UNUSED(ulWeaveMode);
#endif

    /*** MDI_TOP_0 ***/
    /* MDI_TOP_0_MODE_CONTROL_0: disable double buffering */
#if (BVDC_P_SUPPORT_MADR_VER < BVDC_P_MADR_VER_9)
    ulModeCtrl0 |=
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, UPDATE_SEL, UPDATE_BY_PICTURE);
#endif

    /* MDI_TOP_0_MODE_CONTROL_0: optimal settings */
#if (BVDC_P_SUPPORT_MADR_VER >= BVDC_P_MADR_VER_5)
    ulModeCtrl0 |=
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, FEEDER_PRE_FETCH, ON);
#endif

#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_8)
    ulModeCtrl0 |=
        BCHP_FIELD_ENUM(MDI_TOP_1_MODE_CONTROL_0, SUSPEND_EOP,   ON) |
        BCHP_FIELD_ENUM(MDI_TOP_1_MODE_CONTROL_0, WAIT_DMA_DONE, ON);

#elif (BVDC_P_SUPPORT_MADR_VER >= BVDC_P_MADR_VER_6)
    ulModeCtrl0 |=
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, SUSPEND_EOP,   ON)|
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, WAIT_DMA_DONE, ON);
#endif

    /* Debug OSD, Orientation, Gamemode handling */
    ulModeCtrl0 |= hMcdi->bEnableOsd ?
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, ON_SCREEN_STATUS_ENABLE, ON) :
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, ON_SCREEN_STATUS_ENABLE, OFF);


    ulModeCtrl0 |=
        BCHP_FIELD_DATA(MDI_TOP_0_MODE_CONTROL_0, BVB_VIDEO, eOrientation);
    switch(hMcdi->eGameMode)
    {
        default:
        case BVDC_MadGameMode_eOff:
        case BVDC_MadGameMode_e5Fields_2Delay:
        case BVDC_MadGameMode_e4Fields_2Delay:
        case BVDC_MadGameMode_e3Fields_2Delay:
            ulModeCtrl0 |=
                BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, LOW_DELAY_COUNT, NORMAL_DELAY);
            break;

        case BVDC_MadGameMode_e5Fields_1Delay:
        case BVDC_MadGameMode_e4Fields_1Delay:
        case BVDC_MadGameMode_e3Fields_1Delay:
            ulModeCtrl0 |=
                BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, LOW_DELAY_COUNT, ONE_FIELD_DELY);
            break;

        case BVDC_MadGameMode_e5Fields_0Delay:
        case BVDC_MadGameMode_e4Fields_0Delay:
        case BVDC_MadGameMode_e3Fields_0Delay:

        /* fall thru */
        case BVDC_MadGameMode_e5Fields_ForceSpatial:
        case BVDC_MadGameMode_e4Fields_ForceSpatial:
        case BVDC_MadGameMode_e3Fields_ForceSpatial:
        case BVDC_MadGameMode_eMinField_ForceSpatial:
            ulModeCtrl0 |=
                BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, LOW_DELAY_COUNT, ZERO_FIELD_DELAY);
            break;
    }

    BVDC_P_SUBRUL_ONE_REG(pList, ulRegModeCtrl0, ulRegOffset, ulModeCtrl0);
    BVDC_P_SUBRUL_ONE_REG(pList, ulRegEnable, ulRegOffset,
        BCHP_FIELD_ENUM(MDI_TOP_0_ENABLE_CONTROL, ENABLE, ON));
}



#if (!BVDC_P_SUPPORT_MCDI_VER)
static void BVDC_P_Mcdi_BuildRul_Mcdi_NodeInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    BSTD_UNUSED(hMcdi);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(pPicture);
}
static void BVDC_P_Mcdi_BuildRul_Mcdi_FcbInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    BSTD_UNUSED(hMcdi);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(pPicture);
}
static void BVDC_P_Mcdi_BuildRul_Mcdi_PpbInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    BSTD_UNUSED(hMcdi);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(pPicture);
}
static void BVDC_P_Mcdi_BuildRul_Mcdi_SrcInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    BVDC_P_Mcdi_BuildRul_Mcdi_NodeInit_isr(hMcdi, pList, pPicture);
    BVDC_P_Mcdi_BuildRul_Mcdi_FcbInit_isr(hMcdi, pList, pPicture);
    BVDC_P_Mcdi_BuildRul_Mcdi_PpbInit_isr(hMcdi, pList, pPicture);
}

static void BVDC_P_Mcdi_BuildRul_Mcdi_SetEnable_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture,
      bool                          bInit)
{
    BSTD_UNUSED(hMcdi);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(pPicture);
    BSTD_UNUSED(bInit);
}
#else

/***************************************************************************
* {private}
*  MCDI FCB Init
*/
static void BVDC_P_Mcdi_BuildRul_Mcdi_NodeInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    uint32_t ulBufCount;
    uint32_t ulSize, ulQmBufSize;
    BMMA_DeviceOffset ullPixAddr[4],ullQmAddr[4];
    uint32_t ulRegOffset = hMcdi->ulRegOffset;
    bool    bForceSpatial = BVDC_P_MAD_SPATIAL(hMcdi->eGameMode);
    bool    bIsBufContinuous = pPicture->bContinuous;
    uint32_t ulChannelId = pPicture->ulPictureIdx;


    /* ********* Step 1: pixel buffer ********* */
    if((bForceSpatial)&&(NULL!=hMcdi->apHeapNode[ulChannelId][0]))
    {
        ullPixAddr[0] = ullPixAddr[1] = ullPixAddr[2] = ullPixAddr[3] =
            BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][0]);
        BDBG_MSG(("0 addr "BDBG_UINT64_FMT" "BDBG_UINT64_FMT" "BDBG_UINT64_FMT" "BDBG_UINT64_FMT,
            BDBG_UINT64_ARG(ullPixAddr[0]), BDBG_UINT64_ARG(ullPixAddr[1]),
            BDBG_UINT64_ARG(ullPixAddr[2]), BDBG_UINT64_ARG(ullPixAddr[3])));
    }
    else
    {
        /* 7420 BVDC_P_SUPPORT_MCDI_VER == 0 does not have siob */
        ullPixAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][0]);
        ulSize = hMcdi->ulPxlBufSize[ulChannelId];
        BDBG_ASSERT(ulSize);

        if(bIsBufContinuous)
        {
            ullPixAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullPixAddr[0] + ulSize, BVDC_P_PITCH_ALIGN);
            ullPixAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullPixAddr[1] + ulSize, BVDC_P_PITCH_ALIGN);
            ullPixAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullPixAddr[2] + ulSize, BVDC_P_PITCH_ALIGN);
        }
        else /* no siob compression */
        {
            ullPixAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][0]);
            ullPixAddr[1] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][1]);
            ullPixAddr[2] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][2]);
            ullPixAddr[3] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulChannelId][3]);
        }
    }
    /* set bufs addr into reg */
    BRDC_AddrRul_ImmsToRegs_isr(&pList->pulCurrent,
        BCHP_MDI_TOP_0_PIXEL_FIELD_MSTART_0 + ulRegOffset,
        BCHP_MDI_TOP_0_PIXEL_FIELD_MSTART_3 + ulRegOffset,ullPixAddr);

    /* 7420 x does  not have QM */
    /* Buffer inside each group*/
    ulBufCount = BVDC_P_MCDI_QM_FIELD_STORE_COUNT / BVDC_P_MCDI_QM_BUFFER_COUNT;

    if(bForceSpatial)
    {
        ullQmAddr[0] = ullQmAddr[1] = ullQmAddr[2] = ullQmAddr[3] = 0;
    }
    else
    {
        ulQmBufSize = hMcdi->apQmHeapNode[ulChannelId][0]->pHeapInfo->ulBufSize / ulBufCount;
        BDBG_ASSERT(ulQmBufSize);
        ullQmAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apQmHeapNode[ulChannelId][0]);
        ullQmAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[0] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
        ullQmAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[1] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
        ullQmAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[2] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
    }

    BRDC_AddrRul_ImmsToRegs_isr(&pList->pulCurrent,
        BCHP_MDI_TOP_0_QM_FIELD_MSTART_1 + ulRegOffset,
        BCHP_MDI_TOP_0_QM_FIELD_MSTART_4 + ulRegOffset, ullQmAddr);

    if(!bForceSpatial)
    {
        ullQmAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apQmHeapNode[ulChannelId][1]);
        ullQmAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[0] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
        ullQmAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[1] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
        ullQmAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[2] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
    }


    BRDC_AddrRul_ImmsToRegs_isr(&pList->pulCurrent,
        BCHP_MDI_TOP_0_QM_FIELD_MSTART_5 + ulRegOffset,
        BCHP_MDI_TOP_0_QM_FIELD_MSTART_0 + ulRegOffset, ullQmAddr);

    return;
}


/***************************************************************************
* {private}
*  MCDI FCB Init
*/
static void BVDC_P_Mcdi_BuildRul_Mcdi_FcbInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    uint32_t ulRegOffset;
    uint32_t ulModeCtrl;
    uint32_t ulHSize;
#if (BVDC_P_SUPPORT_MCDI_VER > BVDC_P_MCDI_VER_3)
    bool     bForceSpatial;
#endif
    uint32_t ulData;
    bool bIsHD=false;

    BDBG_OBJECT_ASSERT(hMcdi, BVDC_MDI);
    ulHSize = pPicture->pMadIn->ulWidth;
    ulRegOffset = hMcdi->ulRegOffset;
    bIsHD = (ulHSize >= BFMT_720P_WIDTH);
#if (BVDC_P_SUPPORT_MCDI_VER > BVDC_P_MCDI_VER_3)
    bForceSpatial = BVDC_P_MAD_SPATIAL(hMcdi->eGameMode);
#endif


#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_3)
    ulModeCtrl =
        BCHP_FIELD_DATA(MDI_FCB_0_MODE_CONTROL_0, GLOBAL_FIELD_K_ENABLE, !bForceSpatial) |
        BCHP_FIELD_DATA(MDI_FCB_0_MODE_CONTROL_0, GLOBAL_FIELD_L_ENABLE, !bForceSpatial) |
        BCHP_FIELD_DATA(MDI_FCB_0_MODE_CONTROL_0, GLOBAL_FIELD_M_ENABLE, !bForceSpatial);

    /* CRBVN-290 no capture for forcespatial */
#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_9)
    ulModeCtrl |=
        BCHP_FIELD_DATA(MDI_FCB_0_MODE_CONTROL_0, PIXEL_CAP_ENABLE, !bForceSpatial);
#endif
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_MODE_CONTROL_0, ulRegOffset, ulModeCtrl);
#else
    BSTD_UNUSED(ulModeCtrl);
#endif

    /* setting ALT_REPF_VETO_LEVEL_ENABLE to 1, due to HW-7420:
    * Low angle test stream "3 lines jaggies" showes broken jaggies once in a while */

    /* setting according to Jim Tseng's email:
    MDI_PPB_0_XCHROMA_CONTROL_1.PIC_422_MA = 0 (MA_CHROMA)
    MDI_PPB_0_XCHROMA_CONTROL_1.PIC_422_IT = 0 (MA_CHROMA)
    MDI_PPB_0_XCHROMA_CONTROL_1.FIELD_420_MA = 0 (MA_CHROMA)
    MDI_PPB_0_XCHROMA_CONTROL_1.FIELD_420_IT = 0 (MA_CHROMA)
    MDI_PPB_0_XCHROMA_CONTROL_1.FRAME_420_MA = 0 (MA_CHROMA)
    MDI_PPB_0_XCHROMA_CONTROL_1.FRAME_420_IT = 0 (MA_CHROMA)
    */

    /* MCDI Golden value setting   */
    /* FCB Module */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_MC_DECISION_CONTROL_0, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_MC_DECISION_CONTROL_0, PCC_TOTAL_THRESH, BVDC_P_MCDI_FCB_MC_CTRL_PCC_TOTAL_THRESH));

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_1, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_1, REV22_LOCK_SAT_LEVEL    ,32) |
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_1, PCC_NONMATCH_MATCH_RATIO, 6));

#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_6)

    /* BLOCK: +++ */
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, ulRegOffset,
        BVDC_P_REGS_ENTRIES(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_2));

#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_5)
    /* set MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0*/
    *pList->pulCurrent++ = (
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_PCC_INC_ENABLE,                 ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_SAME_FRAME_ENABLE,                OFF)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_LG_MAX_ENABLE,                  ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_LG_FF_ENABLE,                   ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_LG_ENABLE,                      ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_BW_LG_FF_ENABLE,                   ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_BW_LG_ENABLE,                      ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_NOINC_ENABLE,                      ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REPF_FL_ENABLE,                          ON)|
        BCHP_FIELD_DATA(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, USE_RANGE_PCC,             BVDC_P_MCDI_ZERO)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, ALT_ENTER_LOCK_ENABLE,                   ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, ALT_REPF_VETO_LEVEL_ENABLE,              ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_BAD_EDIT_PHASE_VETO,               ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, BAD_EDIT_DETECT_ENABLE,                  ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, BAD_WEAVE_DETECT_ENABLE,                 ON));
#else
    /* set MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0*/
    *pList->pulCurrent++ = (
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_STAIR_ENABLE,                  ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_FEATH_ENABLE,                  ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_BW_FEATH_ENABLE,                  ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_PCC_INC_ENABLE,                ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_SAME_FRAME_ENABLE,               OFF)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_LG_MAX_ENABLE,                 ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_LG_FF_ENABLE,                  ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_BW_LG_ENABLE,                     ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_BW_LG_FF_ENABLE,                  ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_BW_LG_ENABLE,                     ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_NOINC_ENABLE,                     ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REPF_FL_ENABLE,                         ON)|
        BCHP_FIELD_DATA(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, USE_RANGE_PCC,            BVDC_P_MCDI_ZERO)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, ALT_ENTER_LOCK_ENABLE,                  ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, ALT_REPF_VETO_LEVEL_ENABLE,             ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, REV32_BAD_EDIT_PHASE_VETO,              ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, BAD_EDIT_DETECT_ENABLE,                 ON)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_0, BAD_WEAVE_DETECT_ENABLE,                ON));
#endif
#else
        /* BLOCK: +++ */
        BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_1, ulRegOffset,
            BVDC_P_REGS_ENTRIES(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_1, MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_2));
#endif

    /* set MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_1*/
    ulData = bIsHD?
       (BCHP_FIELD_DATA(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_1, REV32_REPF_VETO_LEVEL,         BVDC_P_MCDI_REV32_REPF_VETO_LEVEL_HD) |
        BCHP_FIELD_DATA(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_1, REV32_REPF_PIX_CORRECT_LEVEL,   BVDC_P_MCDI_REV32_REPF_PIX_LEVEL_HD)):
       (BCHP_FIELD_DATA(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_1, REV32_REPF_VETO_LEVEL,         BVDC_P_MCDI_REV32_REPF_VETO_LEVEL_SD) |
        BCHP_FIELD_DATA(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_1, REV32_REPF_PIX_CORRECT_LEVEL,   BVDC_P_MCDI_REV32_REPF_PIX_LEVEL_SD));
    *pList->pulCurrent++ = ulData;

    /* set MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_2*/
    ulData = bIsHD? BCHP_FIELD_DATA(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_2, REV32_BAD_EDIT_LEVEL, BVDC_P_MCDI_REV32_BAD_EDIT_LEVEL_HD):
                    BCHP_FIELD_DATA(MDI_FCB_0_IT_FIELD_PHASE_CALC_CONTROL_2, REV32_BAD_EDIT_LEVEL, BVDC_P_MCDI_REV32_BAD_EDIT_LEVEL_SD);
    *pList->pulCurrent++ = ulData;
    /* BLOCK: --- */

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_6, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_6, REV32_BW_LG_PCC_RATIO, BVDC_P_MCDI_REV32_BW_LG_PCC_RATIO)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_6, REV32_BW_LG_PCC_MIN,   BVDC_P_MCDI_REV32_BW_LG_PCC_MIN));

#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8)
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_4, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_4, REPF_FL_RATIO, BVDC_P_MCDI_REV32_REPF_FL_RATIO)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_4, REPF_FL_MIN,   BVDC_P_MCDI_REV32_REPF_FL_MIN));
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_4_MCDI, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_4_MCDI, REPF_FL_RATIO, BVDC_P_MCDI_REV32_REPF_FL_RATIO)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_4_MCDI, REPF_FL_MIN,   BVDC_P_MCDI_REV32_REPF_FL_MIN));
#else
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_4, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_4, REPF_FL_RATIO, BVDC_P_MCDI_REV32_REPF_FL_RATIO)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_4, REPF_FL_MIN,   BVDC_P_MCDI_REV32_REPF_FL_MIN));
    ulData =
        BCHP_FIELD_DATA(MDI_FCB_0_DEBUG_CURRENT_FIELD_CONTROL_3, SCENE_CHG_FLAG           ,                  0)|
        BCHP_FIELD_ENUM(MDI_FCB_0_DEBUG_CURRENT_FIELD_CONTROL_3, SCENE_CHG_FLAG_UPDATE_SEL, INT_SCENE_CHG_FLAG)|
        BCHP_FIELD_DATA(MDI_FCB_0_DEBUG_CURRENT_FIELD_CONTROL_3, GMVFREQ                  ,                  0)|
        BCHP_FIELD_DATA(MDI_FCB_0_DEBUG_CURRENT_FIELD_CONTROL_3, GMVY                     ,                  0)|
        BCHP_FIELD_DATA(MDI_FCB_0_DEBUG_CURRENT_FIELD_CONTROL_3, GMVX                     ,                  0)|
        BCHP_FIELD_ENUM(MDI_FCB_0_DEBUG_CURRENT_FIELD_CONTROL_3, IT_GMV_UPDATE_SEL        ,             IT_GMV)|
        BCHP_FIELD_ENUM(MDI_FCB_0_DEBUG_CURRENT_FIELD_CONTROL_3, IT_APS_UPDATE_SEL        ,             IT_APS)|
        BCHP_FIELD_DATA(MDI_FCB_0_DEBUG_CURRENT_FIELD_CONTROL_3, SADBYBUSY_THR      , BVDC_P_MCDI_SADBYBUSY_THR);
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_DEBUG_CURRENT_FIELD_CONTROL_3, ulRegOffset, ulData);


    /* SWSTB-963: adjust for HD setting: MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_7~9*/
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_7, ulRegOffset,
        BVDC_P_REGS_ENTRIES(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_7, MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_9));

    /* MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_7*/
    ulData =
        (bIsHD?
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_7, REV32_BW_LG_FF_OFFSET, BVDC_P_MCDI_REV32_BW_LG_FF_OFFSET_HD):
        BCHP_FIELD_ENUM(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_7, REV32_BW_LG_FF_OFFSET, DEFAULT))|
        BCHP_FIELD_ENUM(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_7, REV32_BW_LG_FF_RATIO,  DEFAULT);
    *pList->pulCurrent++ = ulData;

    /* MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_8*/
    ulData =
        bIsHD?
        (BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_8, REV32_BW_FEATHERING_MAX,     BVDC_P_MCDI_REV32_REV32_BW_FEATHERING_MAX_HD    )|
         BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_8, REV32_BW_FEATHERING_FF_DIFF, BVDC_P_MCDI_REV32_REV32_BW_FEATHERING_FF_DIFF_HD)):
        (BCHP_FIELD_ENUM(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_8, REV32_BW_FEATHERING_MAX,     DEFAULT)|
         BCHP_FIELD_ENUM(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_8, REV32_BW_FEATHERING_FF_DIFF, DEFAULT));
    *pList->pulCurrent++ = ulData;

    /* MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_9*/
    ulData =
        (bIsHD?
        BCHP_FIELD_DATA(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_9, REV32_T_T1_FEATH_MIN,   BVDC_P_MCDI_REV32_REV32_T_T1_FEATH_MIN_HD):
        BCHP_FIELD_ENUM(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_9, REV32_T_T1_FEATH_MIN,   DEFAULT))|
        BCHP_FIELD_ENUM(MDI_FCB_0_REV32_IT_FIELD_PHASE_CALC_CONTROL_9, REV32_T_T1_FEATH_RATIO, DEFAULT);
    *pList->pulCurrent++ = ulData;
#endif

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_0, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_NONMATCH_MATCH_RATIO, BVDC_P_MCDI_REV22_NONMATCH_MATCH_RATIO)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_ENTER_LOCK_LEVEL,         BVDC_P_MCDI_REV22_ENTER_LOCK_LEVEL)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_0, REV22_EXIT_LOCK_LEVEL,           BVDC_P_MCDI_REV22_EXIT_LOCK_LEVEL));

#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8)
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_3, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_3, MIN_USABLE_PCC,           BVDC_P_MCDI_REV22_MIN_USABLE_PCC)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_3, PW_MATCH_MULTIPLIER, BVDC_P_MCDI_REV22_PW_MATCH_MULTIPLIER));

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_3_MCDI, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_3_MCDI, MIN_USABLE_PCC,           BVDC_P_MCDI_REV22_MIN_USABLE_PCC_MCDI)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_3_MCDI, PW_MATCH_MULTIPLIER, BVDC_P_MCDI_REV22_PW_MATCH_MULTIPLIER_MCDI));
#else
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_3, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_3, MIN_USABLE_PCC,           BVDC_P_MCDI_REV22_MIN_USABLE_PCC_MCDI)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_3, PW_MATCH_MULTIPLIER, BVDC_P_MCDI_REV22_PW_MATCH_MULTIPLIER_MCDI));
#endif

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_8, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_8, REV22_ALMOST_LOCK_LEVEL, BVDC_P_MCDI_REV22_ALMOST_LOCK_LEVEL));

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_9, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_9, REV22_BW_LG_PCC_RATIO, BVDC_P_MCDI_REV22_BW_LG_PCC_RATIO)|
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_9, REV22_BW_LG_PCC_MIN,   BVDC_P_MCDI_REV22_BW_LG_PCC_MIN));

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_11, ulRegOffset,
        BCHP_FIELD_DATA(MDI_FCB_0_REV22_IT_FIELD_PHASE_CALC_CONTROL_11, REV22_BW_LG_PCC_MAXIMUM, BVDC_P_MCDI_REV22_BW_LG_PCC_MAXIMUM));

    /* Bad weave */
    ulData = bIsHD ?
        (BCHP_FIELD_DATA(MDI_FCB_0_BWV_CONTROL_1, LUMA_32_THRESH,         BVDC_P_MCDI_BWV_LUMA32_THD_HD) |
         BCHP_FIELD_DATA(MDI_FCB_0_BWV_CONTROL_1, LUMA_32_AVG_THRESH, BVDC_P_MCDI_BWV_LUMA32_AVG_THD_HD)):
        (BCHP_FIELD_DATA(MDI_FCB_0_BWV_CONTROL_1, LUMA_32_THRESH,         BVDC_P_MCDI_BWV_LUMA32_THD_SD) |
         BCHP_FIELD_DATA(MDI_FCB_0_BWV_CONTROL_1, LUMA_32_AVG_THRESH, BVDC_P_MCDI_BWV_LUMA32_AVG_THD_SD));

#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8)
    /* MDI_FCB_0_BWV_CONTROL_3 */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_BWV_CONTROL_1, ulRegOffset, ulData);

    /* MDI_FCB_0_BWV_CONTROL_3_MCDI */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_BWV_CONTROL_1_MCDI, ulRegOffset, ulData);
#else
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_BWV_CONTROL_1, ulRegOffset, ulData);
#endif

    ulData = BCHP_FIELD_DATA(MDI_FCB_0_BWV_CONTROL_3, TKR_PCC_MULT,            BVDC_P_MCDI_BWV_TKR_PCC_MULT) |
        (bIsHD ? BCHP_FIELD_DATA(MDI_FCB_0_BWV_CONTROL_3, TKR_MIN_REPF_VETO_LEVEL, BVDC_P_MCDI_BWV_TKR_VETO_LEVEL_HD) :
                 BCHP_FIELD_DATA(MDI_FCB_0_BWV_CONTROL_3, TKR_MIN_REPF_VETO_LEVEL, BVDC_P_MCDI_BWV_TKR_VETO_LEVEL_SD));

#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8)
    /* MDI_FCB_0_BWV_CONTROL_3 */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_BWV_CONTROL_3, ulRegOffset, ulData);

    /* MDI_FCB_0_BWV_CONTROL_3_MCDI */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_BWV_CONTROL_3_MCDI, ulRegOffset, ulData);
#else
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_BWV_CONTROL_3, ulRegOffset, ulData);
#endif

#if (BVDC_P_MCDI_VER_4 <= BVDC_P_SUPPORT_MCDI_VER )
    /* MDI_FCB_0_SCENE_CHANGE_CTRL */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_SCENE_CHANGE_CTRL, ulRegOffset,
        (BCHP_FIELD_ENUM(MDI_FCB_0_SCENE_CHANGE_CTRL, SCENE_CHANGE_ENABLE,                               ON) |
         BCHP_FIELD_DATA(MDI_FCB_0_SCENE_CHANGE_CTRL, SCENE_CHANGE_THRESHOLD,            BVDC_P_MCDI_SC_THD) |
         BCHP_FIELD_DATA(MDI_FCB_0_SCENE_CHANGE_CTRL, SCENE_CHANGE_COUNTER_PRE_THR,         BVDC_P_MCDI_ONE) |
         BCHP_FIELD_DATA(MDI_FCB_0_SCENE_CHANGE_CTRL, SCENE_CHANGE_COUNTER_POST_THR, BVDC_P_MCDI_SC_PST_THD) |
         BCHP_FIELD_DATA(MDI_FCB_0_SCENE_CHANGE_CTRL, SCENE_CHANGE_POST_THR,           BVDC_P_MCDI_SC_THD)));
#endif

    return;
}


/***************************************************************************
* {private}
*  MCDI PPB Init
*/
static void BVDC_P_Mcdi_BuildRul_Mcdi_PpbInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    uint32_t ulRegOffset;
    uint32_t ulRegNum;
    BVDC_Deinterlace_ChromaSettings *pChromaSettings = hMcdi->pChromaSettings;
    uint32_t ulVSize, ulHSize, ulDecisionCtrl, ulGmvThd;
    bool bIsHD = false;
    bool bIsPAL = false;

    ulRegOffset = hMcdi->ulRegOffset;


    ulHSize = pPicture->pMadIn->ulWidth;
    ulVSize = pPicture->pMadIn->ulHeight >>(pPicture->PicComRulInfo.eSrcOrigPolarity != BAVC_Polarity_eFrame);
    bIsHD = (ulHSize >= BFMT_720P_WIDTH);
    bIsPAL = (pPicture->pMadIn->ulHeight == BFMT_PAL_HEIGHT);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_START_IT_LINE, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_START_IT_LINE, VALUE, ulVSize - 1));
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_MDI_PPB_0_STATS_RANGE, ulRegOffset,
        BVDC_P_REGS_ENTRIES(MDI_PPB_0_STATS_RANGE, MDI_PPB_0_STATS_HRANGE_VIEW2));
    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_STATS_RANGE,         END_LINE,  ulVSize - 1);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_STATS_HRANGE,        END_PIXEL, ulHSize - 1);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_STATS_RANGE_VIEW2,   END_LINE,  ulVSize - 1);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_STATS_HRANGE_VIEW2,  END_PIXEL, ulHSize - 1);


    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_XCHROMA_CONTROL_0, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_0, CHROMA_MOTION_ENABLE,           pChromaSettings->eChroma422MotionAdaptiveMode )|
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_0, CHROMA_422_MOTION_MODE,         pChromaSettings->eChroma422MotionMode         )|
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_0, CHROMA_FIELD_420_MOTION_MODE,   pChromaSettings->eChroma420MotionMode         )|
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_0, IT_PPRFM_CR_ENABLE,             ON                                            )|
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_0, IT_PPUFM_CR_ENABLE,             ON                                            )|
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_0, CHROMA_FIELD_420_EDGE_DET_MODE, ON                                            )|
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_0, CHROMA_422_MA_EDGE_DET_MODE,    pChromaSettings->bChromaField420EdgeDetMode   )|
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_0, CHROMA_FIELD_420_INITIAL_PHASE, pChromaSettings->bChromaField420InitPhase     )|
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_0, CHROMA_FIELD_420_INV_METHOD,    pChromaSettings->eChromaField420InvMethod     ));
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_XCHROMA_CONTROL_1, ulRegOffset,
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_1, PIC_422_MA,   MA_CHROMA)|
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_1, PIC_422_IT,   IT_DIRECT)|
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_1, FIELD_420_MA,   INT_420)|
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_1, FIELD_420_IT,   INT_420)|
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_1, FRAME_420_MA, MA_CHROMA)|
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_1, FRAME_420_IT, IT_DIRECT));

#if ((BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_6) && (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_2))
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_WEAVE_DETECT_CTRL, ulRegOffset,
        BCHP_FIELD_ENUM(MDI_PPB_0_WEAVE_DETECT_CTRL, WEAVE_MOTION_BOOST_ENABLE,   ON)|
        BCHP_FIELD_DATA(MDI_PPB_0_WEAVE_DETECT_CTRL, WEAVE_DETECT_THR,             4)|
        BCHP_FIELD_DATA(MDI_PPB_0_WEAVE_DETECT_CTRL, WEAVE_MOTION_THR_LOW,         0));

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MVD_K1_VALUE, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MVD_K1_VALUE, SMALL, BVDC_P_MVD_K1_VALUE));

    ulRegNum = BVDC_P_REGS_ENTRIES(MDI_PPB_0_MA_MC_BLEND_CTRL_0, MDI_PPB_0_MA_MC_BLEND_CTRL_1);
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_MDI_PPB_0_MA_MC_BLEND_CTRL_0, ulRegOffset, ulRegNum);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MDI_PPB_0_MA_MC_BLEND_CTRL_0, STAT_THRESH_GMV_CHOSEN,      BVDC_P_MCDI_STAT_THRESH_GMV_CHOSEN)|
        BCHP_FIELD_DATA(MDI_PPB_0_MA_MC_BLEND_CTRL_0, STAT_THRESH_GMV_NOTCHOSEN,   BVDC_P_MCDI_STAT_THRESH_GMV_CHOSEN)|
        BCHP_FIELD_DATA(MDI_PPB_0_MA_MC_BLEND_CTRL_0, STAT_THRESH,                            BVDC_P_MCDI_STAT_THRESH);

    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MDI_PPB_0_MA_MC_BLEND_CTRL_1, STAT_SCALE_GMV_CHOSEN,        BVDC_P_MCDI_STAT_SCALE_GMV_CHOSEN)|
        BCHP_FIELD_DATA(MDI_PPB_0_MA_MC_BLEND_CTRL_1, STAT_SCALE_GMV_NOTCHOSEN,     BVDC_P_MCDI_STAT_SCALE_GMV_CHOSEN)|
        BCHP_FIELD_DATA(MDI_PPB_0_MA_MC_BLEND_CTRL_1, STAT_SCALE,                              BVDC_P_MCDI_STAT_SCALE);
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_XCHROMA_CONTROL_6, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_6, MA_420_422_BLEND_STRENGTH,   BVDC_P_MCDI_MA_420422_BLEND_STRENGTH)|
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_6, IT_420_422_BLEND_STRENGTH,   BVDC_P_MCDI_IT_420422_BLEND_STRENGTH)|
        BCHP_FIELD_ENUM(MDI_PPB_0_XCHROMA_CONTROL_6, IT_PPREG_CR_ENABLE,                                            ON)|
        BCHP_FIELD_DATA(MDI_PPB_0_XCHROMA_CONTROL_6, IT_420_422_BLEND_MODE,                            BVDC_P_MCDI_ONE));
#endif

#if (BVDC_P_MCDI_VER_7 <= BVDC_P_SUPPORT_MCDI_VER )
        /*7439A0 default HW value does not align with golden value*/
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_IT_PCC_CONTROL, ulRegOffset,
            BCHP_FIELD_DATA(MDI_PPB_0_IT_PCC_CONTROL, IT_STATS_MIN_MOTION,    5)|
            BCHP_FIELD_DATA(MDI_PPB_0_IT_PCC_CONTROL, TKR_CORING_THRESH,   0x80)|
            BCHP_FIELD_DATA(MDI_PPB_0_IT_PCC_CONTROL, CHROMA_CORING_THRESH,   4)|
            BCHP_FIELD_DATA(MDI_PPB_0_IT_PCC_CONTROL, LUMA_CORING_THRESH,     4));
#endif
#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_5 )
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MC_BLEND, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MC_BLEND, K0, BVDC_P_MCDI_MC_BLEND_K0));

#if (BVDC_P_MCDI_VER_3 == BVDC_P_SUPPORT_MCDI_VER )
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MEMC_CTRL, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MEMC_CTRL, SCRATCH,                           BVDC_P_MCDI_ZERO)|
        BCHP_FIELD_DATA(MDI_PPB_0_MEMC_CTRL, USE_IT_4MCDI,                       BVDC_P_MCDI_ONE)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, CHROMA_COST_ENABLE,                             OFF)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, FORCE_WEAVE,                                    OFF)|
        BCHP_FIELD_DATA(MDI_PPB_0_MEMC_CTRL, LAMDA,                  BVDC_P_MCDI_MEMC_CTRL_LAMDA));
#endif

#if (BVDC_P_MCDI_VER_4 <= BVDC_P_SUPPORT_MCDI_VER )
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MEMC_CTRL, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MEMC_CTRL, SCRATCH,                          BVDC_P_MCDI_ZERO)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, ME_3D_REINIT_BIAS_AT_LRVIEW_EDGE,               ON)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, INT_SOURCE_STEP3_ENABLE,                        ON)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, STEP3_ENABLE,                                   ON)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, STEP1_ENABLE,                                   ON)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, ME_3D_LIMIT_1D,                                 ON)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, GMV_IS_ATTRACTOR_AT_EDGE,                       ON)|
        BCHP_FIELD_DATA(MDI_PPB_0_MEMC_CTRL, USE_IT_4MCDI,                      BVDC_P_MCDI_ONE)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, CHROMA_COST_ENABLE,                            OFF)|
        BCHP_FIELD_ENUM(MDI_PPB_0_MEMC_CTRL, FORCE_WEAVE,                                   OFF)|
        BCHP_FIELD_DATA(MDI_PPB_0_MEMC_CTRL, LAMDA,                 BVDC_P_MCDI_MEMC_CTRL_LAMDA));
#endif

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MC_COST_CTRL_02, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_02, SCRATCH,                        BVDC_P_MCDI_ZERO)|
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_02, PCC_CNT_THD_1,         BVDC_P_MCDI_PCC_CNT_THD_1)|
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_02, PCC_CNT_THD_0,         BVDC_P_MCDI_PCC_CNT_THD_0)|
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_02, PCC_CNT_THD,             BVDC_P_MCDI_PCC_CNT_THD)|
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_02, ZERO_PCC_CORE_THD, BVDC_P_MCDI_ZERO_PCC_CORE_THD)|
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_02, MC_PCC_CORE_THD,     BVDC_P_MCDI_MC_PCC_CORE_THD));

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MC_COST_CTRL_04, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_04, SCRATCH,    BVDC_P_MCDI_ZERO)|
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_04, EDGE_K2, BVDC_P_MCDI_EDGE_K2)|
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_04, EDGE_K1, BVDC_P_MCDI_EDGE_K1)|
        BCHP_FIELD_DATA(MDI_PPB_0_MC_COST_CTRL_04, EDGE_K0, BVDC_P_MCDI_EDGE_K0));

    ulRegNum = BVDC_P_REGS_ENTRIES(MDI_PPB_0_MC_MC_THD, MDI_PPB_0_MC_VM_THD);
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_MDI_PPB_0_MC_MC_THD, ulRegOffset, ulRegNum);

    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_MC_MC_THD, MC_THD_HIGH, BVDC_P_MCDI_MC_MC_THD_HIGH)|
                            BCHP_FIELD_DATA(MDI_PPB_0_MC_MC_THD, MC_THD_LOW, BVDC_P_MCDI_MC_MC_THD_LOW);

    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_MC_ZERO_THD_0,  ZERO_THD_LOW_1, BVDC_P_MCDI_ZERO_THD_LOW_1)|
                            BCHP_FIELD_DATA(MDI_PPB_0_MC_ZERO_THD_0, ZERO_THD_LOW_0, BVDC_P_MCDI_ZERO_THD_LOW_0);

    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_MC_ZERO_THD_1,  ZERO_THD_HIGH,  BVDC_P_MCDI_ZERO_THD_HIGH);

    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_MC_SHIFT_MC_THD,  SHIFT_MC_THD_1, BVDC_P_MCDI_SHIFT_MC_THD_1)|
                            BCHP_FIELD_DATA(MDI_PPB_0_MC_SHIFT_MC_THD, SHIFT_MC_THD_0, BVDC_P_MCDI_SHIFT_MC_THD_0);

    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_MC_EDGE_THD_0,  EDGE_THD_LOW_1, BVDC_P_MCDI_EDGE_THD_LOW_1)|
                            BCHP_FIELD_DATA(MDI_PPB_0_MC_EDGE_THD_0, EDGE_THD_LOW_0, BVDC_P_MCDI_EDGE_THD_LOW_0);

    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_MC_EDGE_THD_1,  EDGE_THD_HIGH_1, BVDC_P_MCDI_EDGE_THD_HIGH_1)|
                            BCHP_FIELD_DATA(MDI_PPB_0_MC_EDGE_THD_1, EDGE_THD_HIGH_0, BVDC_P_MCDI_EDGE_THD_HIGH_0);

    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_MC_EDGE_THD_2,  EDGE_THD_HIGH_2, BVDC_P_MCDI_EDGE_THD_HIGH_2);

    *pList->pulCurrent++ = BCHP_FIELD_DATA(MDI_PPB_0_MC_VM_THD, VM_THD_1, BVDC_P_MCDI_VM_THD_1)|
                            BCHP_FIELD_DATA(MDI_PPB_0_MC_VM_THD, VM_THD_0, BVDC_P_MCDI_VM_THD_0);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MC_SHIFT_ZERO_THD, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MC_SHIFT_ZERO_THD,  SHIFT_ZERO_THD, BVDC_P_MCDI_SHIFT_ZERO_THD));

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MC_MOVIE_MC_THD, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MC_MOVIE_MC_THD,  MOVIE_MC_THD, BVDC_P_MCDI_MOVIE_MC_THD));
#else
    BSTD_UNUSED(ulRegNum);
    BSTD_UNUSED(pChromaSettings);
#endif

#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_3)
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MH_MAPPING_VALUE_STATIONARY, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MH_MAPPING_VALUE_STATIONARY, VALUE_3, BVDC_P_MCDI_MH_MAPPING_VALUE_3)|
        BCHP_FIELD_DATA(MDI_PPB_0_MH_MAPPING_VALUE_STATIONARY, VALUE_2, BVDC_P_MCDI_MH_MAPPING_VALUE_2)|
        BCHP_FIELD_DATA(MDI_PPB_0_MH_MAPPING_VALUE_STATIONARY, VALUE_1, BVDC_P_MCDI_MH_MAPPING_VALUE_1)|
        BCHP_FIELD_DATA(MDI_PPB_0_MH_MAPPING_VALUE_STATIONARY, VALUE_0,               BVDC_P_MCDI_ZERO));
#endif

#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8)
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_GMV_NUM_THD, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_GMV_NUM_THD, VALUE,                             BVDC_P_MCDI_GMV_NUM_THD));
    BSTD_UNUSED(ulDecisionCtrl);
    BSTD_UNUSED(ulGmvThd);
    BSTD_UNUSED(bIsHD);
    BSTD_UNUSED(bIsPAL);
#else
    ulGmvThd = bIsHD? BVDC_P_MCDI_GMV_NUM_THD_HD :
            (bIsPAL ? BVDC_P_MCDI_GMV_NUM_THD_PAL : BVDC_P_MCDI_GMV_NUM_THD_SD);
    ulDecisionCtrl =
        BCHP_FIELD_DATA(MDI_MEMC_0_DECISION_CTRL_0, RANGE_GMV_Y, BVDC_P_MCDI_RANGE_GMV_Y)|
        BCHP_FIELD_DATA(MDI_MEMC_0_DECISION_CTRL_0, RANGE_GMV_X, BVDC_P_MCDI_RANGE_GMV_X)|
        BCHP_FIELD_DATA(MDI_MEMC_0_DECISION_CTRL_0, GMV_NUM_THD, ulGmvThd);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_MEMC_0_DECISION_CTRL_0, ulRegOffset, ulDecisionCtrl);
#endif
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MH_MAPPING_VALUE, ulRegOffset,
        BCHP_FIELD_DATA(MDI_PPB_0_MH_MAPPING_VALUE, VALUE_3, BVDC_P_MCDI_MH_MAPPING_VALUE_3)|
        BCHP_FIELD_DATA(MDI_PPB_0_MH_MAPPING_VALUE, VALUE_2, BVDC_P_MCDI_MH_MAPPING_VALUE_2)|
        BCHP_FIELD_DATA(MDI_PPB_0_MH_MAPPING_VALUE, VALUE_1, BVDC_P_MCDI_MH_MAPPING_VALUE_1)|
        BCHP_FIELD_DATA(MDI_PPB_0_MH_MAPPING_VALUE, VALUE_0,               BVDC_P_MCDI_ZERO));
    return;
}



/***************************************************************************
* {private}
*  MCDI src Init
*/
static void BVDC_P_Mcdi_BuildRul_Mcdi_SrcInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    uint32_t ulHSize, ulVSize;

    BDBG_OBJECT_ASSERT(hMcdi, BVDC_MDI);


#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
    /* set the stream index very first */

    if((pPicture->bMosaicMode) &&(hMcdi->ulMosaicMaxChannels))
    {
        uint32_t ulChannelId = pPicture->ulPictureIdx;
        BDBG_ASSERT(ulChannelId<pPicture->ulMosaicCount);

        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_MULTI_CONTEXT_TO, hMcdi->ulRegOffset,
            BCHP_FIELD_DATA(MDI_TOP_0_MULTI_CONTEXT_TO, STREAM_PROCESSED, ulChannelId));
    }
#endif

    ulHSize = pPicture->pMadIn->ulWidth;
    ulVSize = pPicture->pMadIn->ulHeight >>(pPicture->PicComRulInfo.eSrcOrigPolarity != BAVC_Polarity_eFrame);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_SRC_PIC_SIZE, hMcdi->ulRegOffset,
            BCHP_FIELD_DATA(MDI_TOP_0_SRC_PIC_SIZE, HSIZE, ulHSize) |
            BCHP_FIELD_DATA(MDI_TOP_0_SRC_PIC_SIZE, VSIZE, ulVSize));

    BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC, ("init pPicture mosaic %d %d x %d ",
        pPicture->bMosaicMode, ulHSize, ulVSize));

    BVDC_P_Mcdi_BuildRul_Mcdi_NodeInit_isr(hMcdi, pList, pPicture);
    BVDC_P_Mcdi_BuildRul_Mcdi_FcbInit_isr(hMcdi, pList, pPicture);
    BVDC_P_Mcdi_BuildRul_Mcdi_PpbInit_isr(hMcdi, pList, pPicture);

    return;
}

static void BVDC_P_Mcdi_BuildRul_Mcdi_SetEnable_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture,
      bool                           bInit)
{
    uint32_t  ulRegOffset;
    uint32_t  ulModeSel, ulFldType, ulTrickModeSel, ulXmaEnum, ulXmaMode, ulWeaveMode;
    bool      bHardStart = false;
    bool bRev32Pulldown;
    uint32_t ulModeCtrl0 = 0;
    uint16_t usTrickModeStartDelay;
    bool                           bRepeat;
    BAVC_Polarity                  eSrcNxtFldId;
    BAVC_FrameRateCode             eFrameRate;
    BFMT_Orientation               eOrientation;
    uint32_t                       ulChannelId;
    uint32_t                       ulConstBlend;

    BDBG_OBJECT_ASSERT(hMcdi, BVDC_MDI);
    ulRegOffset = hMcdi->ulRegOffset;
    bRepeat      = pPicture->stFlags.bPictureRepeatFlag;
    eFrameRate   = pPicture->eFrameRateCode;
    eOrientation = BVDC_P_VNET_USED_MVP_AT_WRITER(pPicture->stVnetMode) ?
            pPicture->eSrcOrientation : pPicture->eDispOrientation;
    eSrcNxtFldId = pPicture->PicComRulInfo.eSrcOrigPolarity;
    ulChannelId = pPicture->ulPictureIdx;

    /*** MDI_FCB_0 ***/
    bRev32Pulldown = (
        (hMcdi->bRev32Pulldown) &&
        (BAVC_FrameRateCode_e25 != eFrameRate));
    bHardStart = BVDC_P_Mcdi_BeHardStart_isr(bInit, hMcdi);

    /* due to HW requirement, FIELD_FREEZE can not be set for the first 4 fields */
    if (bHardStart)
    {
        hMcdi->usTrickModeStartDelay[ulChannelId] = BVDC_P_MCDI_TRICK_MODE_START_DELAY;
    }
    usTrickModeStartDelay = hMcdi->usTrickModeStartDelay[ulChannelId];
    hMcdi->usTrickModeStartDelay[ulChannelId] -= (usTrickModeStartDelay>0);

    ulModeSel = (bHardStart)?
        BCHP_FIELD_ENUM(MDI_FCB_0_MODE_CONTROL_1, MODE_SEL, HARD_START) :
        (BVDC_P_MAD_SPATIAL(hMcdi->eGameMode)?
        BCHP_FIELD_ENUM(MDI_FCB_0_MODE_CONTROL_1, MODE_SEL, FORCE_SPATIAL):
        BCHP_FIELD_ENUM(MDI_FCB_0_MODE_CONTROL_1, MODE_SEL, NORMAL));

    ulTrickModeSel = (bRepeat && (0 == usTrickModeStartDelay)) ?
                BCHP_FIELD_ENUM(MDI_FCB_0_MODE_CONTROL_1, TRICK_MODE_SEL, FIELD_FREEZE) :
                BCHP_FIELD_ENUM(MDI_FCB_0_MODE_CONTROL_1, TRICK_MODE_SEL, OFF);

    ulFldType = (BAVC_Polarity_eBotField == eSrcNxtFldId)?
        BCHP_FIELD_ENUM(MDI_FCB_0_MODE_CONTROL_1, FIELD_D_TYPE, BOTTOM) :
        BCHP_FIELD_ENUM(MDI_FCB_0_MODE_CONTROL_1, FIELD_D_TYPE, TOP);

    ulXmaEnum = (BVDC_P_ChromaType_eChroma422 == pPicture->eChromaType)?
        BCHP_MDI_FCB_0_MODE_CONTROL_1_CHROMA_MODE_SEL_CHROMA_422:
        ((BVDC_P_ChromaType_eField420 == pPicture->eChromaType) ?
        BCHP_MDI_FCB_0_MODE_CONTROL_1_CHROMA_MODE_SEL_CHROMA_FIELD_420 :
        BCHP_MDI_FCB_0_MODE_CONTROL_1_CHROMA_MODE_SEL_CHROMA_FRAME_420);
    ulXmaMode = BCHP_FIELD_DATA(MDI_FCB_0_MODE_CONTROL_1, CHROMA_MODE_SEL, ulXmaEnum);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_MODE_CONTROL_1, ulRegOffset,
        ulModeSel | ulFldType | ulTrickModeSel|ulXmaMode);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_FCB_0_IT_OUTPUT_CONTROL, ulRegOffset,
        ((hMcdi->bRev22Pulldown)
        ? BCHP_FIELD_ENUM(MDI_FCB_0_IT_OUTPUT_CONTROL, AUTOREV22_ENABLE,ON)
        : BCHP_FIELD_ENUM(MDI_FCB_0_IT_OUTPUT_CONTROL, AUTOREV22_ENABLE,OFF)) |
        ((bRev32Pulldown)
        ? BCHP_FIELD_ENUM(MDI_FCB_0_IT_OUTPUT_CONTROL, AUTOREV32_ENABLE,ON)
        : BCHP_FIELD_ENUM(MDI_FCB_0_IT_OUTPUT_CONTROL, AUTOREV32_ENABLE,OFF)) |
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_OUTPUT_CONTROL, PPUFM_MODE, AUTO)|
        BCHP_FIELD_ENUM(MDI_FCB_0_IT_OUTPUT_CONTROL, PPRFM_MODE, ON));

#if ((BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_2) &&(BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8))
    /*** MDI_PPB_0 ***/
    ulWeaveMode = BCHP_FIELD_ENUM(MDI_PPB_0_MOTION_CAL_CONTROL, WEAVE_MOTION_ENABLE, ON)   |
                    BCHP_FIELD_ENUM(MDI_PPB_0_MOTION_CAL_CONTROL, MA_SUBSAMPLING_ENABLE, ON)   |
                    BCHP_FIELD_DATA(MDI_PPB_0_MOTION_CAL_CONTROL, WEAVE_DETECT_THR,      0x10) |
                    BCHP_FIELD_DATA(MDI_PPB_0_MOTION_CAL_CONTROL, WEAVE_MOTION_THR_LOW,  0x16);

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_MOTION_CAL_CONTROL, ulRegOffset, ulWeaveMode);
#else
    BSTD_UNUSED(ulWeaveMode);
#endif

    /*** MDI_TOP_0 ***/
    if(hMcdi->bEnableOsd)
    {
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_OSD_POSITION, ulRegOffset,
            BCHP_FIELD_DATA(MDI_TOP_0_OSD_POSITION, HPOS, hMcdi->ulOsdHpos) |
            BCHP_FIELD_DATA(MDI_TOP_0_OSD_POSITION, VPOS, hMcdi->ulOsdVpos));
    }

#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_8)
    ulModeCtrl0 = (((pPicture->bMosaicMode) && (pPicture->pMadIn->ulWidth >= BFMT_1080I_WIDTH)) ||
    (BVDC_P_MAD_LOWDELAY(hMcdi->eGameMode)))?
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, MEMC_ENABLE, OFF) :
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, MEMC_ENABLE, ON);

    /* low delay game mode*/
    if(BVDC_P_MAD_LOWDELAY(hMcdi->eGameMode))
    {
        ulConstBlend =
            BCHP_FIELD_DATA(MDI_PPB_0_CONST_BLEND_CTRL_MAD, VALUE_GMV_CHOSEN,    BVDC_P_MCDI_CONST_BLEND)|
            BCHP_FIELD_DATA(MDI_PPB_0_CONST_BLEND_CTRL_MAD, VALUE_GMV_NOTCHOSEN, BVDC_P_MCDI_CONST_BLEND)|
            BCHP_FIELD_DATA(MDI_PPB_0_CONST_BLEND_CTRL_MAD, VALUE,               BVDC_P_MCDI_CONST_BLEND);
    }
    else
    {
        ulConstBlend =
            BCHP_FIELD_DATA(MDI_PPB_0_CONST_BLEND_CTRL_MAD, VALUE_GMV_CHOSEN,    0)|
            BCHP_FIELD_DATA(MDI_PPB_0_CONST_BLEND_CTRL_MAD, VALUE_GMV_NOTCHOSEN, 0)|
            BCHP_FIELD_DATA(MDI_PPB_0_CONST_BLEND_CTRL_MAD, VALUE,               0);
    }
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_CONST_BLEND_CTRL_MAD, ulRegOffset, ulConstBlend);
#else
    BSTD_UNUSED(ulConstBlend);
#endif
/* MDI_TOP_0_MODE_CONTROL_0: optimal settings */
#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_4)
    ulModeCtrl0 |=
#if (BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8)
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, UPDATE_SEL, UPDATE_BY_PICTURE) |
#endif
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, FEEDER_PRE_FETCH, ON);
#endif

#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_4)
    ulModeCtrl0 |=
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, WAIT_DMA_DONE, ON);
#endif

#if ((BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_5) &&(BVDC_P_SUPPORT_MCDI_VER < BVDC_P_MCDI_VER_8))
    ulModeCtrl0 |=
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, SUSPEND_EOP, ON);
#endif

/* Debug OSD, Orientation, Gamemode handling */
    ulModeCtrl0 |= hMcdi->bEnableOsd ?
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, ON_SCREEN_STATUS_ENABLE, ON) :
        BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, ON_SCREEN_STATUS_ENABLE, OFF);

    ulModeCtrl0 |=
        BCHP_FIELD_DATA(MDI_TOP_0_MODE_CONTROL_0, BVB_VIDEO, eOrientation);
    switch(hMcdi->eGameMode)
    {
        default:
        case BVDC_MadGameMode_eOff:
        case BVDC_MadGameMode_e5Fields_2Delay:
        case BVDC_MadGameMode_e4Fields_2Delay:
        case BVDC_MadGameMode_e3Fields_2Delay:
            ulModeCtrl0 |=
                BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, LOW_DELAY_COUNT, NORMAL_DELAY);
            break;

        case BVDC_MadGameMode_e5Fields_1Delay:
        case BVDC_MadGameMode_e4Fields_1Delay:
        case BVDC_MadGameMode_e3Fields_1Delay:
            ulModeCtrl0 |=
                BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, LOW_DELAY_COUNT, ONE_FIELD_DELY);
            break;

        case BVDC_MadGameMode_e5Fields_0Delay:
        case BVDC_MadGameMode_e4Fields_0Delay:
        case BVDC_MadGameMode_e3Fields_0Delay:

        /* fall thru */
        case BVDC_MadGameMode_e5Fields_ForceSpatial:
        case BVDC_MadGameMode_e4Fields_ForceSpatial:
        case BVDC_MadGameMode_e3Fields_ForceSpatial:
        case BVDC_MadGameMode_eMinField_ForceSpatial:
            ulModeCtrl0 |=
                BCHP_FIELD_ENUM(MDI_TOP_0_MODE_CONTROL_0, LOW_DELAY_COUNT, ZERO_FIELD_DELAY);
            break;
    }

    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_MODE_CONTROL_0, ulRegOffset, ulModeCtrl0);
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_ENABLE_CONTROL, ulRegOffset,
        BCHP_FIELD_ENUM(MDI_TOP_0_ENABLE_CONTROL, ENABLE, ON));
}

#endif

/***************************************************************************
* {private}
*
*/
void BVDC_P_Mcdi_BuildRul_SrcInit_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{

#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
    if(pPicture->bMosaicMode)
    {
        hMcdi->ulMosaicInit |= 1<<pPicture->ulPictureIdx;
    }
#endif
    if(hMcdi->bMadr)
    {
        BVDC_P_Mcdi_BuildRul_Madr_SrcInit_isr(hMcdi, pList, pPicture);
    }
    else
    {
        BVDC_P_Mcdi_BuildRul_Mcdi_SrcInit_isr(hMcdi, pList, pPicture);
    }

    return;
}
/***************************************************************************
* {private}
*
*/
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
static void BVDC_P_Mcdi_BuildRul_Mosaic_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      BVDC_P_PictureNode            *pPicture)
{
    uint32_t ulPictureIdx;
    uint32_t ulHSize, ulVSize;
    BMMA_DeviceOffset ullPixAddr[4],ullQmAddr[4];
    uint32_t ulQmBufSize, ulPxlBufSize;
    uint32_t ulRegOffset = hMcdi->ulRegOffset;
    bool    bForceSpatial = BVDC_P_MAD_SPATIAL(hMcdi->eGameMode);


    ulPictureIdx = pPicture->ulPictureIdx;
    BDBG_ASSERT(ulPictureIdx <= hMcdi->ulMosaicMaxChannels);

#if (!BVDC_P_SUPPORT_MCDI_SUPERSET)
    /* 1. context setting*/
    if(hMcdi->bMadr)
    {
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_1_MULTI_CONTEXT_TO, hMcdi->ulRegOffset1,
            BCHP_FIELD_DATA(MDI_TOP_1_MULTI_CONTEXT_TO, STREAM_PROCESSED, ulPictureIdx));
    }
    else
    {
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_MULTI_CONTEXT_TO, ulRegOffset,
                BCHP_FIELD_DATA(MDI_TOP_0_MULTI_CONTEXT_TO, STREAM_PROCESSED, ulPictureIdx));
    }
#else
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_MULTI_CONTEXT_TO, ulRegOffset,
            BCHP_FIELD_DATA(MDI_TOP_0_MULTI_CONTEXT_TO, STREAM_PROCESSED, ulPictureIdx));
#endif
    BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC, ("mcdi[%d] stream %d", hMcdi->eId, ulPictureIdx));

    /* size */
    ulHSize = pPicture->pMadIn->ulWidth;
    ulVSize = pPicture->pMadIn->ulHeight >>(pPicture->PicComRulInfo.eSrcOrigPolarity != BAVC_Polarity_eFrame);
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_SRC_PIC_SIZE, ulRegOffset,
            BCHP_FIELD_DATA(MDI_TOP_0_SRC_PIC_SIZE,  HSIZE, ulHSize) |
            BCHP_FIELD_DATA(MDI_TOP_0_SRC_PIC_SIZE,  VSIZE, ulVSize));

    BDBG_ASSERT(ulHSize);
    BDBG_ASSERT(ulVSize);


    /* 2. statistics range */
    BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_STATS_RANGE,  ulRegOffset,
            BCHP_FIELD_DATA(MDI_PPB_0_STATS_RANGE, END_LINE, ulVSize - 1));

#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_8)
    if(!hMcdi->bMadr)
    {
        /*MDI_PPB_0_START_IT_LINE*/
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_START_IT_LINE,  ulRegOffset,
                    BCHP_FIELD_DATA(MDI_PPB_0_START_IT_LINE, VALUE, ulVSize - 1));

        /*MDI_PPB_0_STATS_HRANGE*/
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_PPB_0_STATS_HRANGE,  ulRegOffset,
            BCHP_FIELD_DATA(MDI_PPB_0_STATS_HRANGE, END_PIXEL, ulHSize - 1));
    }
#endif

    /* 3. pixel buffer */
    ulPxlBufSize = hMcdi->ulPxlBufSize[ulPictureIdx];
    if(pPicture->bContinuous)
    {
        if((bForceSpatial) && (hMcdi->bMadr))
        {
            ullPixAddr[0] = ullPixAddr[1] = ullPixAddr[2] = ullPixAddr[3] = 0;
        }
        else
        {
            BDBG_ASSERT(hMcdi->apHeapNode[ulPictureIdx][0]);
            ullPixAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulPictureIdx][0]);
            ullPixAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullPixAddr[0]+ ulPxlBufSize, BVDC_P_PITCH_ALIGN);
            ullPixAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullPixAddr[1]+ ulPxlBufSize, BVDC_P_PITCH_ALIGN);
            ullPixAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullPixAddr[2]+ ulPxlBufSize, BVDC_P_PITCH_ALIGN);
        }
    }
    else
    {
        ullPixAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulPictureIdx][0]);
        ullPixAddr[1] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulPictureIdx][1]);
        ullPixAddr[2] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulPictureIdx][2]);
        ullPixAddr[3] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apHeapNode[ulPictureIdx][3]);
    }

    BRDC_AddrRul_ImmsToRegs_isr(&pList->pulCurrent,
        BCHP_MDI_TOP_0_PIXEL_FIELD_MSTART_0 + ulRegOffset,
        BCHP_MDI_TOP_0_PIXEL_FIELD_MSTART_3 + ulRegOffset, ullPixAddr);


    BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC,
        ("mcdi[%d] stream[%d] %4d x%4d continusou %s pxl buffer %s offset %d "BDBG_UINT64_FMT" "BDBG_UINT64_FMT" "BDBG_UINT64_FMT" "BDBG_UINT64_FMT,
        hMcdi->eId, ulPictureIdx, ulHSize, ulVSize, pPicture->bContinuous?"true":"false",
        BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hMcdi->apHeapNode[ulPictureIdx][0]->pHeapInfo->eBufHeapId),
        ulPxlBufSize, BDBG_UINT64_ARG(ullPixAddr[0]), BDBG_UINT64_ARG(ullPixAddr[1]),
        BDBG_UINT64_ARG(ullPixAddr[2]), BDBG_UINT64_ARG(ullPixAddr[3])));


    /* 4. qm buffer */
    ulQmBufSize     = bForceSpatial?0 : hMcdi->ulQmBufSize[ulPictureIdx];

    if(bForceSpatial)
    {
        ullQmAddr[0] = ullQmAddr[1] = ullQmAddr[2] = ullQmAddr[3] = 0;
    }
    else
    {
        ullQmAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apQmHeapNode[ulPictureIdx][0]);
        ullQmAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[0] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
        ullQmAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[1] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
        ullQmAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[2] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
    }

    BRDC_AddrRul_ImmsToRegs_isr(&pList->pulCurrent,
        BCHP_MDI_TOP_0_QM_FIELD_MSTART_1 + ulRegOffset,
        BCHP_MDI_TOP_0_QM_FIELD_MSTART_4 + ulRegOffset, ullQmAddr);


    BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC,("Qm buffer offset %d size "BDBG_UINT64_FMT" "BDBG_UINT64_FMT" "BDBG_UINT64_FMT" "BDBG_UINT64_FMT,
        ulQmBufSize, BDBG_UINT64_ARG(ullQmAddr[0]), BDBG_UINT64_ARG(ullQmAddr[1]),
        BDBG_UINT64_ARG(ullQmAddr[2]), BDBG_UINT64_ARG(ullQmAddr[3])));


#if (BVDC_P_SUPPORT_MCDI_VER >= BVDC_P_MCDI_VER_8)
    if(!hMcdi->bMadr)
    {
        if(bForceSpatial)
        {
            ullQmAddr[0] = ullQmAddr[1] = ullQmAddr[2] = ullQmAddr[3] = 0;
        }
        else
        {
            ullQmAddr[0] = BVDC_P_BUFFERHEAP_GetDeviceOffset(hMcdi->apQmHeapNode[ulPictureIdx][1]);
            ullQmAddr[1] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[0] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
            ullQmAddr[2] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[1] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
            ullQmAddr[3] = BVDC_P_ADDR_ALIGN_UP(ullQmAddr[2] + ulQmBufSize, BVDC_P_PITCH_ALIGN);
        }

        BRDC_AddrRul_ImmsToRegs_isr(&pList->pulCurrent,
            BCHP_MDI_TOP_0_QM_FIELD_MSTART_5 + ulRegOffset,
            BCHP_MDI_TOP_0_QM_FIELD_MSTART_0 + ulRegOffset, ullQmAddr);


        BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC,
            ("Qm buffer Mcdi "BDBG_UINT64_FMT" "BDBG_UINT64_FMT" "BDBG_UINT64_FMT" "BDBG_UINT64_FMT,
                BDBG_UINT64_ARG(ullQmAddr[0]), BDBG_UINT64_ARG(ullQmAddr[1]),
                BDBG_UINT64_ARG(ullQmAddr[2]), BDBG_UINT64_ARG(ullQmAddr[3])));
    }
#endif
    /* 5. To do list:golden set update */
}
#endif

/***************************************************************************
* {private}
*
*/
/* TODO:  for 7420 ANR is after HSCL, not before, take care pic rect seeting,
* how to hard-start delay to scl left, top? */
void BVDC_P_Mcdi_BuildRul_SetEnable_isr
    ( BVDC_P_Mcdi_Handle             hMcdi,
      BVDC_P_ListInfo               *pList,
      bool                           bEnable,
      BVDC_P_PictureNode            *pPicture,
      bool                           bInit)
{
    if (bEnable)
    {
        bool                          bInitMcdi = bInit;

#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
        if((pPicture->bMosaicMode)&&(hMcdi->ulMosaicMaxChannels))
        {
            uint32_t   ulChannelId = pPicture->ulPictureIdx;

            /* assumption: only one picture in first channel carry the init true signal*/
            bInitMcdi = (hMcdi->ulMosaicInit>>ulChannelId) & 1;
            hMcdi->ulMosaicInit &=~(1<< ulChannelId);

            if(!bInitMcdi)
                BVDC_P_Mcdi_BuildRul_Mosaic_isr(hMcdi, pList, pPicture);
        }
#endif

        if(hMcdi->bMadr)
        {
            BVDC_P_Mcdi_BuildRul_Madr_SetEnable_isr(hMcdi, pList, pPicture, bInitMcdi);
        }
        else
        {
            BVDC_P_Mcdi_BuildRul_Mcdi_SetEnable_isr(hMcdi, pList, pPicture, bInitMcdi);
        }

        /* used by BeHardStart_isr */
        hMcdi->bInitial = false;
    }
    else
    {
        /* disable mcdi */
        BVDC_P_SUBRUL_ONE_REG(pList, BCHP_MDI_TOP_0_ENABLE_CONTROL, hMcdi->ulRegOffset,
            BCHP_FIELD_ENUM(MDI_TOP_0_ENABLE_CONTROL, ENABLE, OFF));

        /* used by BeHardStart_isr */
        hMcdi->bInitial = true;
    }

    return;
}

/***************************************************************************
* {private}
*
*/
bool BVDC_P_Mcdi_BeHardStart_isr
    (bool                               bInit,
     BVDC_P_Mcdi_Handle                 hMcdi )
{
    bool bHwDirty;

    BDBG_OBJECT_ASSERT(hMcdi, BVDC_MDI);
    bHwDirty = bInit || hMcdi->stSwDirty.stBits.bSize ||
        hMcdi->stSwDirty.stBits.bChannelChange || hMcdi->stSwDirty.stBits.bModeSwitch ||
        hMcdi->stSwDirty.stBits.bGameMode ||
        BVDC_P_MAD_SPATIAL(hMcdi->eGameMode);
    return (bHwDirty);
}

/***************************************************************************
* {private}
*
* BVDC_P_Mcdi_GetUserConf_isr
*
* called by BVDC_P_MCVP_SetInfo_isr at every src or vec vsync (depending on
* whether reader side or writer side is using this module)
*
*/

void BVDC_P_Mcdi_GetUserConf_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      BVDC_P_Deinterlace_Settings     *pMadSettings)
{
    uint32_t ulCurDelay,ulDelay;

    ulCurDelay = BVDC_P_Mcdi_GetVsyncDelayNum_isr(hMcdi, hMcdi->eGameMode);
    ulDelay    = BVDC_P_Mcdi_GetVsyncDelayNum_isr(hMcdi, pMadSettings->eGameMode);
    if(ulCurDelay != ulDelay)
    {
        hMcdi->stSwDirty.stBits.bGameMode = BVDC_P_DIRTY;
        BDBG_MSG(("deinteracer[%d] delay change %d => %d", hMcdi->eId, ulCurDelay, ulDelay));
    }
    hMcdi->eGameMode      = pMadSettings->eGameMode;
    hMcdi->bRev22Pulldown = pMadSettings->bReverse22Pulldown;
    hMcdi->bRev32Pulldown = pMadSettings->bReverse32Pulldown;
    hMcdi->pChromaSettings               = &pMadSettings->stChromaSettings;
    return;
}


/***************************************************************************
* {private}
*
* BVDC_P_Mcdi_Init_Chroma_DynamicDefault_isr
*
* called by BVDC_P_Mcvp_AcquireConnect_isr at acquire connection
* See Also:
* BVDC_P_Mad_Init_Chroma_DynamicDefault
* Section 7.3~7.4 in
* http://www.sj.broadcom.com/projects/dvt/Chip_Architecture/Video/Released/mad-IT_ph3.pdf
*/
void BVDC_P_Mcdi_Init_Chroma_DynamicDefault_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      BVDC_Deinterlace_ChromaSettings *pChromaSettings,
      const BFMT_VideoInfo            *pFmtInfo,
      bool                             bMfdSrc)
{
#if (BVDC_P_SUPPORT_MCDI_VER)
    BDBG_ASSERT(pChromaSettings);

    if(hMcdi->bMadr) return;

    /* MAD_0_MODE_CONTROL_0.CHROMA_FIELD_420_EDGE_DET_MODE */
    pChromaSettings->bChromaField420EdgeDetMode = true;
    /* MAD_0_MODE_CONTROL_0.CHROMA_FIELD_420_INITIAL_PHASE */
    pChromaSettings->bChromaField420InitPhase = false;
    /* MAD_0_MODE_CONTROL_0.CHROMA_FIELD_420_INV_METHOD */
    pChromaSettings->eChromaField420InvMethod   = !bMfdSrc;

    if(BFMT_IS_PAL(pFmtInfo->eVideoFmt))
    {
        /* MDI_PPB_0_XCHROMA_CONTROL_1.PIC_422_MA */
        pChromaSettings->eChroma422MotionAdaptiveMode =
            BCHP_MDI_PPB_0_XCHROMA_CONTROL_1_PIC_422_MA_INT_420;
        /* MDI_PPB_0_XCHROMA_CONTROL_1.PIC_422_IT */
        pChromaSettings->eChroma422InverseTelecineMode =
            BCHP_MDI_PPB_0_XCHROMA_CONTROL_1_PIC_422_IT_INT_420;
    }
    else
    {
        /* MDI_PPB_0_XCHROMA_CONTROL_1.PIC_422_MA */
        pChromaSettings->eChroma422MotionAdaptiveMode =
            BCHP_MDI_PPB_0_XCHROMA_CONTROL_1_PIC_422_MA_INT_420;

        /* MDI_PPB_0_XCHROMA_CONTROL_1.PIC_422_IT */
        pChromaSettings->eChroma422InverseTelecineMode =
            bMfdSrc ?
            BCHP_MDI_PPB_0_XCHROMA_CONTROL_1_PIC_422_IT_IT_DIRECT:
            BCHP_MDI_PPB_0_XCHROMA_CONTROL_1_PIC_422_IT_MA_CHROMA;
    }

    if((!bMfdSrc) &&
        (BFMT_IS_NTSC(pFmtInfo->eVideoFmt) || BFMT_IS_PAL(pFmtInfo->eVideoFmt)))
    {
        /* MDI_PPB_0_XCHROMA_CONTROL_0.CHROMA_422_MOTION_MODE */
        pChromaSettings->eChroma422MotionMode =
            BCHP_MDI_PPB_0_XCHROMA_CONTROL_0_CHROMA_422_MOTION_MODE_XCHROMA_AWARE;

        /* MDI_PPB_0_XCHROMA_CONTROL_0.CHROMA_FIELD_420_MOTION_MODE */
        pChromaSettings->eChroma420MotionMode =
            BCHP_MDI_PPB_0_XCHROMA_CONTROL_0_CHROMA_FIELD_420_MOTION_MODE_XCHROMA_AWARE;
    }
    else
    {
        /* MDI_PPB_0_XCHROMA_CONTROL_0.CHROMA_422_MOTION_MODE */
        pChromaSettings->eChroma422MotionMode =
            BCHP_MDI_PPB_0_XCHROMA_CONTROL_0_CHROMA_422_MOTION_MODE_DIGITAL_CHROMA;

        /* MDI_PPB_0_XCHROMA_CONTROL_0.CHROMA_FIELD_420_MOTION_MODE */
        pChromaSettings->eChroma420MotionMode =
            BCHP_MDI_PPB_0_XCHROMA_CONTROL_0_CHROMA_FIELD_420_MOTION_MODE_DIGITAL_CHROMA;
    }

    /* MDI_PPB_0_XCHROMA_CONTROL_1.FIELD_420_MA */
    pChromaSettings->eChroma420MotionAdaptiveMode =
        BCHP_MDI_PPB_0_XCHROMA_CONTROL_1_FIELD_420_MA_INT_420;
#else
    BSTD_UNUSED(hMcdi);
    BSTD_UNUSED(pChromaSettings);
    BSTD_UNUSED(pFmtInfo);
    BSTD_UNUSED(bMfdSrc);
#endif

    return;
}


/***************************************************************************
*
*/
uint16_t BVDC_P_Mcdi_GetVsyncDelayNum_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      BVDC_MadGameMode                 eGameMode)
{
#if (BVDC_P_SUPPORT_MADR_VER < BVDC_P_MADR_VER_7)
    /* SW7231-383: MADR has only 1 delay */
    return (BVDC_P_MAD_SPATIAL(eGameMode)) ? 0 :
    ((hMcdi->bMadr) ? 1 : s_aMcdiGameModeInfo[eGameMode].usDelay);
#else
    /* SW7231-383: MADR has only 1 delay */
    return (hMcdi->bMadr) ? s_aMadrGameModeInfo[eGameMode].usDelay :
        s_aMcdiGameModeInfo[eGameMode].usDelay;
#endif
}

/***************************************************************************
*
*/
uint16_t BVDC_P_Mcdi_GetPixBufCnt_isr
    ( bool                             bMadr,
      BVDC_MadGameMode                 eGameMode)
{
    uint16_t ulBufCnt = 0;

    if(bMadr)
#if (BVDC_P_SUPPORT_MADR_VER < BVDC_P_MADR_VER_7)
    {
        ulBufCnt = BVDC_P_MAD_SPATIAL(eGameMode)?0:3;
    }
#else
    {
        ulBufCnt = s_aMadrGameModeInfo[eGameMode].usPixelBufferCnt;
    }
#endif
    else
    {
        ulBufCnt = s_aMcdiGameModeInfo[eGameMode].usPixelBufferCnt;
    }

    return (ulBufCnt);
}

#if BVDC_P_STG_RUL_DELAY_WORKAROUND
/***************************************************************************
*
*/
void BVDC_P_Mcdi_GetDeinterlacerType_isr
    ( BVDC_P_Mcdi_Handle               hMcdi,
      bool                             *pbMadr)
{
    *pbMadr = hMcdi->bMadr;
}
#endif

/***************************************************************************
*
*/
void BVDC_P_Mcdi_ReadOutPhase_isr
    ( BVDC_P_Mcdi_Handle                 hMcdi,
      BVDC_P_PictureNode                *pPicture)
{
    uint32_t  ulReg;

    if (hMcdi->bMadr)
    {
#if (BVDC_P_SUPPORT_MCDI_SUPERSET)
        ulReg = BREG_Read32_isr(hMcdi->hRegister, BCHP_MDI_FCN_0_IT_OUTPUT_CONTROL + hMcdi->ulRegOffset);
#else
        ulReg = BREG_Read32_isr(hMcdi->hRegister, BCHP_MDI_FCN_1_IT_OUTPUT_CONTROL + hMcdi->ulRegOffset1);
#endif
        pPicture->stFlags.bRev32Locked = BCHP_GET_FIELD_DATA(ulReg, MDI_FCN_0_IT_OUTPUT_CONTROL, REV32_LOCKED);
        pPicture->ulMadOutPhase = BCHP_GET_FIELD_DATA(ulReg, MDI_FCN_0_IT_OUTPUT_CONTROL, REV32_FIELD_PHASE);
    }

#if BVDC_P_SUPPORT_MCDI_VER
    if (!hMcdi->bMadr)
    {
        ulReg = BREG_Read32_isr(hMcdi->hRegister, BCHP_MDI_FCB_0_IT_OUTPUT_CONTROL + hMcdi->ulRegOffset);
        pPicture->stFlags.bRev32Locked = BCHP_GET_FIELD_DATA(ulReg, MDI_FCB_0_IT_OUTPUT_CONTROL, REV32_LOCKED);
        pPicture->ulMadOutPhase = BCHP_GET_FIELD_DATA(ulReg, MDI_FCB_0_IT_OUTPUT_CONTROL, REV32_FIELD_PHASE);
    }
#endif
}

/* End of file. */
