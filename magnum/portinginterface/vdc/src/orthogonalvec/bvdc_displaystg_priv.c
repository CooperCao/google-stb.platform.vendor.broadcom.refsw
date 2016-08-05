/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 ***************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bfmt.h"

#include "bchp.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayvip_priv.h"
#include "bavc_vce_mbox.h"

#if (BVDC_P_SUPPORT_STG)

/* ---------------------------------------------
 * STG version
 * --------------------------------------------- */
/*
 *  7425 A0/A1
 */
#define BVDC_P_STG_VER_1                     (1)

/*
 *  7425 B0/B1/B2
 */
#define BVDC_P_STG_VER_2                     (2)

/*
 *  7435
 */
#define BVDC_P_STG_VER_3                     (3)

/*
 *  7445, 7145, 7366, 7439
 *  4 stgs
 */
#define BVDC_P_STG_VER_4                     (4)

/*
 *  7445 D0
 *  6 stgs VIDEO_ENC_STG_0_INP_PIXEL_COUNT_STATUS/ VIDEO_ENC_STG_0_OUT_PIXEL_COUNT_STATUS
 */
#define BVDC_P_STG_VER_5                     (5)
#if BCHP_VICE2_ARCSS_ESS_DCCM_0_0_REG_START
#include "bchp_vice2_arcss_ess_dccm_0_0.h"
#include "bchp_vice2_arcss_ess_p1_intr2_0_0.h"
#include "bchp_vice2_misc_0.h"
#if BCHP_VICE2_ARCSS_ESS_DCCM_0_1_REG_START
#include "bchp_vice2_arcss_ess_dccm_0_1.h"
#include "bchp_vice2_arcss_ess_p1_intr2_0_1.h"
#endif
#elif BCHP_VICE2_ARCSS_ESS_DCCM_0_REG_START
#include "bchp_vice2_arcss_ess_dccm_0.h"
#include "bchp_vice2_arcss_ess_p1_intr2_0.h"
#include "bchp_vice2_misc.h"
#endif

BDBG_MODULE(BVDC_DISP);
BDBG_FILE_MODULE(BVDC_DISP_STG);
BDBG_FILE_MODULE(BVDC_CMP_SIZE);
BDBG_FILE_MODULE(BVDC_IGNORE_RATIO);

#define BVDC_P_MAKE_STG_INFO(stgid, ulCorId, ulchannelId)         \
{                                                                 \
    (stgid), (ulCorId), (ulchannelId)                             \
}

static const BVDC_P_StgViceWireInfo s_aStgViceInfo[] =
{
    BVDC_P_MAKE_STG_INFO(0,  0,  0),
    BVDC_P_MAKE_STG_INFO(1,  0,  1),
    BVDC_P_MAKE_STG_INFO(2,  1,  0),
    BVDC_P_MAKE_STG_INFO(3,  1,  1),
    BVDC_P_MAKE_STG_INFO(4,  0,  2),
    BVDC_P_MAKE_STG_INFO(5,  1,  2),
};

#ifdef BCHP_PWR_RESOURCE_VDC_STG0
void BVDC_P_AcquireStgPwr
    ( BVDC_Display_Handle              hDisplay )
{

    BDBG_ASSERT(hDisplay);
    BDBG_ASSERT(hDisplay->stStgChan.ulStg < BVDC_P_SUPPORT_STG);

    BDBG_MSG(("disp[%d] stg %d aquire power", hDisplay->eId, hDisplay->stStgChan.ulStg));
    switch(hDisplay->stStgChan.ulStg)
    {
    case 0:
        hDisplay->ulStgPwrId = BCHP_PWR_RESOURCE_VDC_STG0;
        break;
#ifdef BCHP_PWR_RESOURCE_VDC_STG1
    case 1:
        hDisplay->ulStgPwrId = BCHP_PWR_RESOURCE_VDC_STG1;
        break;
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_STG2
    case 2:
        hDisplay->ulStgPwrId = BCHP_PWR_RESOURCE_VDC_STG2;
        break;
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_STG3
    case 3:
        hDisplay->ulStgPwrId = BCHP_PWR_RESOURCE_VDC_STG3;
        break;
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_STG4
    case 4:
        hDisplay->ulStgPwrId = BCHP_PWR_RESOURCE_VDC_STG4;
        break;
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_STG5
    case 5:
        hDisplay->ulStgPwrId = BCHP_PWR_RESOURCE_VDC_STG5;
        break;
#endif
    default:
        BDBG_ERR(("Unsupported PWR STG %d", hDisplay->stStgChan.ulStg));
        BDBG_ASSERT(0);
    }

    /* STG master mode, acquire PWR */
    if(hDisplay->ulStgPwrAcquire == 0)
    {
        BCHP_PWR_AcquireResource(hDisplay->hVdc->hChip, hDisplay->ulStgPwrId);
        hDisplay->ulStgPwrAcquire++;
    }
}
#endif


/*************************************************************************
*  {private}
* BVDC_P_Display_EnableSTGTriggers_isr
*  Re-enable trigger after vec reset.
**************************************************************************/
void BVDC_P_Display_EnableSTGTriggers_isr
    ( BVDC_Display_Handle              hDisplay,
      bool                             bEnable )
{
    uint32_t ulStgCtrl;
    uint32_t ulRegOffset = 0;

    ulRegOffset = hDisplay->ulStgRegOffset;

    ulStgCtrl   = BREG_Read32_isr(hDisplay->hVdc->hRegister,
        BCHP_VIDEO_ENC_STG_0_CONTROL + ulRegOffset);

    BDBG_MSG(("Display %d %s STG %d trigger: %s", hDisplay->eId, bEnable?"enables":"disables",
        (uint32_t) (hDisplay->eMasterTg - BVDC_DisplayTg_eStg0),
        hDisplay->stCurInfo.bStgNonRealTime? "NRT":"RT"));
    if(!bEnable)
    {
        /*SLAVE_MODE=0, EOP_TRIG_ENABLE=0, TIMER_TRIG_ENABLE=0, SM_ENABLE=0.*/
        ulStgCtrl &= ~(
            BCHP_VIDEO_ENC_STG_0_CONTROL_SLAVE_MODE_MASK |
            BCHP_VIDEO_ENC_STG_0_CONTROL_TIMER_TRIG_ENABLE_MASK |
            BCHP_VIDEO_ENC_STG_0_CONTROL_SM_ENABLE_MASK |
            BCHP_VIDEO_ENC_STG_0_CONTROL_EOP_TRIG_ENABLE_MASK |
            BCHP_VIDEO_ENC_STG_0_CONTROL_HOST_ARM_ENABLE_MASK);

        BREG_Write32(hDisplay->hVdc->hRegister,
            BCHP_VIDEO_ENC_STG_0_CONTROL + ulRegOffset, ulStgCtrl);
    }
    else
    {
        /* non-real time mode, .EOP trigger*/
        if(hDisplay->stStgChan.bStgNonRealTime)
        {
            /* SLAVE_MODE=0, TIMER_TRIG_ENABLE=0, SM_ENABLE=0.*/
            ulStgCtrl &= ~(
                BCHP_MASK(VIDEO_ENC_STG_0_CONTROL, SLAVE_MODE       ) |
                BCHP_MASK(VIDEO_ENC_STG_0_CONTROL, TIMER_TRIG_ENABLE) |
                BCHP_MASK(VIDEO_ENC_STG_0_CONTROL, SM_ENABLE));

            /* EOP_TRIG_ENABLE=1,*/
            ulStgCtrl |=
                BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CONTROL, EOP_TRIG_ENABLE, ENABLE) |
                BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CONTROL, HOST_ARM_ENABLE, ENABLE);

            BREG_Write32_isr(hDisplay->hVdc->hRegister,
                BCHP_VIDEO_ENC_STG_0_CONTROL + ulRegOffset, ulStgCtrl);

        }
        /* real time mode */
        else
        {
            /*STG Master Mode Timer trigger */
            /*SLAVE_MODE=0, EOP_TRIG_ENABLE=0, */
            ulStgCtrl &= ~(
                BCHP_MASK(VIDEO_ENC_STG_0_CONTROL, SLAVE_MODE) |
                BCHP_MASK(VIDEO_ENC_STG_0_CONTROL, HOST_ARM_ENABLE) |
                BCHP_MASK(VIDEO_ENC_STG_0_CONTROL, EOP_TRIG_ENABLE));

            /*TIMER_TRIG_ENABLE=1, SM_ENABLE=1.
              SW7435-319: SM_ENABLE seemed to cause BVB interface isue at VIP. Disable it for now! */
            ulStgCtrl |=
                BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CONTROL, TIMER_TRIG_ENABLE, ENABLE) |
                BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CONTROL, SM_ENABLE, DISABLE);

            BREG_Write32_isr(hDisplay->hVdc->hRegister,
                BCHP_VIDEO_ENC_STG_0_CONTROL + ulRegOffset, ulStgCtrl);
        }
    }
}

void BVDC_P_ConnectStgSrc_isr
    (BVDC_Display_Handle             hDisplay,
     BVDC_P_ListInfo                *pList )
{
    uint32_t ulCmpSrc = 0;
    uint32_t ulRegOffset = 0;

#if (BVDC_P_SUPPORT_STG > 1)
    ulRegOffset = hDisplay->stStgChan.ulStg * sizeof(uint32_t);
#endif

    BDBG_ASSERT(true== hDisplay->hVdc->pFeatures->abAvailCmp[hDisplay->hCompositor->eId]);

    /* Connect STG to source */
    ulCmpSrc = BCHP_VEC_CFG_STG_0_SOURCE_SOURCE_S_0 +
        (hDisplay->hCompositor->eId - BVDC_CompositorId_eCompositor0);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_STG_0_SOURCE + ulRegOffset);
    *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_STG_0_SOURCE, SOURCE, ulCmpSrc);

#if BVDC_P_SUPPORT_VIP && BCHP_VEC_CFG_VIP_0_SOURCE
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_VIP_0_SOURCE + ulRegOffset);
    *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_VIP_0_SOURCE, SOURCE, hDisplay->stStgChan.ulStg);
#endif

#ifdef BVDC_P_SUPPORT_RDC_STC_FLAG
	if(hDisplay->stStgChan.ulStcFlag == BRDC_MAX_STC_FLAG_COUNT)
	{
		BDBG_MSG(("Display%u to acquire STC flag(%u) for STG%u with trigger %u", hDisplay->eId, hDisplay->stStgChan.ulStcFlag, hDisplay->stStgChan.ulStg, hDisplay->eTopTrigger));
		if((hDisplay->stStgChan.ulStcFlag = BRDC_AcquireStcFlag_isr(hDisplay->hVdc->hRdc, hDisplay->stStgChan.ulStg, hDisplay->eTopTrigger))
			!= hDisplay->stStgChan.ulStg)
		{
			BDBG_ERR(("No STC flag available for display %d STG %d trig %u. Check hardware capability.", hDisplay->eId, hDisplay->stStgChan.ulStg, hDisplay->eTopTrigger));
		}
	}
#endif

	return;
}

void BVDC_P_TearDownStgChan_isr
    ( BVDC_Display_Handle             hDisplay,
      BVDC_P_ListInfo                *pList )
{
    uint32_t  ulRegOffset = 0;

#if (BVDC_P_SUPPORT_STG > 1)
    ulRegOffset = hDisplay->stStgChan.ulStg * sizeof(uint32_t);
#else
    BSTD_UNUSED(hDisplay);
#endif
    /* Disable STG source */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_STG_0_SOURCE + ulRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(VEC_CFG_STG_0_SOURCE, SOURCE, DISABLE);

#if BVDC_P_SUPPORT_VIP && BCHP_VEC_CFG_VIP_0_SOURCE
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_VIP_0_SOURCE + ulRegOffset);
    *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(VEC_CFG_VIP_0_SOURCE, SOURCE, DISABLE);
#endif

#ifdef BVDC_P_SUPPORT_RDC_STC_FLAG
	if(hDisplay->stStgChan.ulStcFlag != BRDC_MAX_STC_FLAG_COUNT)
	{
		BRDC_ReleaseStcFlag_isr(hDisplay->hVdc->hRdc, hDisplay->stStgChan.ulStg);
		hDisplay->stStgChan.ulStcFlag = BRDC_MAX_STC_FLAG_COUNT;
	}
#endif

	return;
}

void BVDC_P_SetupStg_isr
    ( uint32_t                       ulRegOffset,
      BVDC_P_ListInfo               *pList )
{
    /* Reset STG Module*/
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SW_INIT_STG_0 + ulRegOffset);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(VEC_CFG_SW_INIT_STG_0, INIT, 1);
    /* unreset STG Module*/
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SW_INIT_STG_0 + ulRegOffset);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(VEC_CFG_SW_INIT_STG_0, INIT, 0);

    return;
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_STG_Build_RM_isr
 **************************************************************************/
void BVDC_P_STG_Build_RM_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList  )
{
    /* RM pairs*/
    BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;
    uint32_t ulVertFreq = BVDC_P_ROUND_OFF(2 * pCurInfo->pFmtInfo->ulVertFreq,
          BFMT_FREQ_FACTOR/2, BFMT_FREQ_FACTOR); /* round up to whole double freq hz */
    uint32_t ulFrameTicks = 27000000*2/ulVertFreq; /* double freq to support 12.5hz */
    uint32_t ulRegOffset = 0;

    ulRegOffset = hDisplay->ulStgRegOffset;


    if(!BVDC_P_DISPLAY_NODELAY(hDisplay->pStgFmtInfo, hDisplay->stCurInfo.pFmtInfo))
        return;

    BDBG_ENTER(BVDC_P_STG_Build_RM_isr);

    pCurInfo->ulVertFreq = pCurInfo->pFmtInfo->ulVertFreq;
    /* 60/30/24/15/20 -> drop to 59.94/29.97/23.98/14.98/19.98; */
    if((pCurInfo->bMultiRateAllow || (ulVertFreq*BFMT_FREQ_FACTOR != pCurInfo->pFmtInfo->ulVertFreq*2)) &&
        (!pCurInfo->bFullRate))
    {
        ulFrameTicks = ulFrameTicks*1001/1000;
        pCurInfo->ulVertFreq = ulVertFreq * (BFMT_FREQ_FACTOR / 2) * 1000/1001;
    }
    /* 59.94/29.97/23.98/19.98/14.98 -> 60/30/24/20/15; */
    else if((ulVertFreq*BFMT_FREQ_FACTOR != pCurInfo->pFmtInfo->ulVertFreq*2) && (pCurInfo->bFullRate))
    {
        pCurInfo->ulVertFreq = ulVertFreq * BFMT_FREQ_FACTOR / 2;
    }
    if(pCurInfo->ulVertFreq != pCurInfo->stRateInfo.ulVertRefreshRate)
    {
        pCurInfo->stRateInfo.ulVertRefreshRate = pCurInfo->ulVertFreq;
    }
    /* always update rate change callback flag in case the tracked rate is different from  format
       nominal refresh rate which may be used to program NRT STC_INCREMENT by upper layer,
       so we need to update upper layer the actual refresh rate used. */
    hDisplay->bRateManagerUpdated = true;

    /* --- Setup RM --- */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_FRAME_SIZE + ulRegOffset);
    *pList->pulCurrent++ = ulFrameTicks - 1; /* inclusive counter value */

    BDBG_LEAVE(BVDC_P_STG_Build_RM_isr);

    return;
}

void BVDC_P_Stg_Init_isr
    ( BVDC_Display_Handle              hDisplay)
{
#if BVDC_P_SUPPORT_VIP /* VDC owns VIP */
    BSTD_UNUSED(hDisplay);
#else /* VDC does NOT own VIP */
    uint32_t                    ulStgId;
    uint32_t                    ulChannelPerCore = 0, ulCoreOffset = 0, ulChannelOffset = 0, ulViceIntOffset = 0, ulViceIntMask=0;
    uint32_t                    ulViceCoreIdx = 0, ulViceChannel=0;

    if (hDisplay->stStgChan.ulStg == BVDC_P_HW_ID_INVALID)
    {
        return;
    }

    ulStgId = hDisplay->stStgChan.ulStg;

#if BCHP_VICE2_MISC_0_REVISION_CONFIGURATION_DEFAULT
    ulChannelPerCore = hDisplay->stStgChan.ulChannelPerCore = BCHP_VICE2_MISC_0_REVISION_CONFIGURATION_DEFAULT;
#else
    ulChannelPerCore = hDisplay->stStgChan.ulChannelPerCore = 2;
#endif

    ulViceCoreIdx = s_aStgViceInfo[ulStgId].ulViceCoreId;
    ulViceChannel = s_aStgViceInfo[ulStgId].ulViceChannelId;

#ifdef BCHP_VICE2_ARCSS_ESS_DCCM_0_1_REG_START
    ulCoreOffset =
        BCHP_VICE2_ARCSS_ESS_DCCM_0_1_DATAi_ARRAY_BASE - BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_ARRAY_BASE;
#endif
    ulChannelOffset =
        BAVC_VICE_MBOX_OFFSET_BVN2VICE_DATA_1_START - BAVC_VICE_MBOX_OFFSET_BVN2VICE_DATA_0_START;

    /* set up the right Mbox addr */
    hDisplay->stStgChan.ulMBoxAddr =
    /* base address */
#if BCHP_VICE2_ARCSS_ESS_DCCM_0_0_REG_START
        BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_ARRAY_BASE +
#else
        BCHP_VICE2_ARCSS_ESS_DCCM_0_DATAi_ARRAY_BASE +
#endif
        ulViceCoreIdx * ulCoreOffset   +      /* core offset */
        ulViceChannel * ulChannelOffset +     /* channel offset */
        BAVC_VICE_MBOX_OFFSET_BVN2VICE_DATA_0_START;

    /* set up the vice interrupt addr */
#if BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_1_REG_START
    ulViceIntOffset =
        BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_1_REG_START - BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_0_REG_START;
#endif

    hDisplay->stStgChan.ulViceIntAddr =
    /* base address */
#if BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_0_ARC_P1_SET
        BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_0_ARC_P1_SET +
#else
        BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_ARC_P1_SET +
#endif
        ulViceCoreIdx * ulViceIntOffset;   /*interrupt offset */


    /* set up the vice interrupt mask */
#if BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_0_ARC_P1_SET_VIP0_INTR_MASK
    ulViceIntMask = BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_0_ARC_P1_SET_VIP0_INTR_MASK;
#elif BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_0_ARC_P1_SET_ARCESS_SOFT0_00_INTR_MASK
    ulViceIntMask = BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_0_ARC_P1_SET_ARCESS_SOFT0_00_INTR_MASK;
#else
    ulViceIntMask = BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_ARC_P1_SET_ARCESS_SOFT0_00_INTR_MASK;
#endif

    hDisplay->stStgChan.ulViceIntMask = ulViceIntMask << ulViceChannel;

    /* reset ignore flags to true used for capture CRC (two vsyncs initial delay) */
    hDisplay->hCompositor->bCrcIgnored = hDisplay->hCompositor->bCrcToIgnore =
        hDisplay->hCompositor->bStgIgnorePicture = true;

    BDBG_MSG(("disp[%d] stg[%d] vice[%d] channel[%d]/%d vice interrupt 0x%x interrupt mask 0x%x %x",
        hDisplay->eId, ulStgId, ulViceCoreIdx, ulViceChannel, ulChannelPerCore,
        hDisplay->stStgChan.ulViceIntAddr, hDisplay->stStgChan.ulViceIntMask, ulViceIntMask));
    BSTD_UNUSED(ulChannelPerCore);
    /* init STC flag */
#ifdef BVDC_P_SUPPORT_RDC_STC_FLAG
    hDisplay->stStgChan.ulStcFlag = BRDC_MAX_STC_FLAG_COUNT;
#endif
#endif
}

void BVDC_P_ProgramStgChan_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;
    const BFMT_VideoInfo        *pFmtInfo;
    uint32_t                    ulIsFull3d, ulSrc, ulStgId;
    uint32_t                    ulStgRegOffset = 0, ulRegOffset=0;


    BDBG_ENTER(BVDC_P_ProgramStgChan);
    ulStgRegOffset = hDisplay->ulStgRegOffset;

    ulStgId = hDisplay->stStgChan.ulStg;



    pFmtInfo = (hDisplay->pStgFmtInfo == NULL)?hDisplay->stCurInfo.pFmtInfo:hDisplay->pStgFmtInfo;
    ulIsFull3d = (uint32_t)BFMT_IS_3D_MODE(pFmtInfo->eVideoFmt);

    if(hDisplay->stCurInfo.bEnableStg)
    {
        /* Dimension */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_BVB_SIZE + ulStgRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_BVB_SIZE, FULLSIZE_3D, ulIsFull3d) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_BVB_SIZE, HORIZONTAL, pFmtInfo->ulDigitalWidth) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_BVB_SIZE, VERTICAL,   pFmtInfo->ulDigitalHeight>>(pFmtInfo->bInterlaced));
        BDBG_MODULE_MSG(BVDC_CMP_SIZE, ("disp[%d] %d x %d%s%d", hDisplay->eId,
            pFmtInfo->ulDigitalWidth, pFmtInfo->ulDigitalHeight,
            pFmtInfo->bInterlaced? "i" : "p", pCurInfo->ulVertFreq));

        /* interlace or progressive */
        /* disable both timer and EOP triggers */
        /* TODO: HOST_ARM enable for non-real-time mode */

        /* Frame rate:
         * to make sure 32-bit math not to overflow, cancel one '0'
         * TODO: support 1000/1001 frame rate tracking */
        BVDC_P_STG_Build_RM_isr(hDisplay, pList);

        /* TODO: STC_CONTROL for non-real-time*/
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_STC_CONTROL + ulStgRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_STC_CONTROL, STC_FLAG_ENABLE, ENABLE);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_CONTROL + ulStgRegOffset);
        *pList->pulCurrent++ = /* disable both timer and EOP triggers */
#if (BVDC_P_SUPPORT_STG_VER > BVDC_P_STG_VER_1)
            BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CONTROL, TRIG_MODE, AND) |
#endif
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, TIMEBASE_SEL, pCurInfo->eTimeBase) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, SLAVE_MODE, !BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg)) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, HOST_ARM_ENABLE, pCurInfo->bStgNonRealTime) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, TIMER_TRIG_ENABLE, !pCurInfo->bStgNonRealTime || hDisplay->hCompositor->bIgnorePicture) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, EOP_TRIG_ENABLE, pCurInfo->bStgNonRealTime) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, SCAN_MODE, pCurInfo->pFmtInfo->bInterlaced);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_DOWN_SAMP + ulStgRegOffset);
        *pList->pulCurrent++ =
            ((!hDisplay->stCurInfo.bBypassVideoProcess)
            ? BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_DOWN_SAMP, FILTER_MODE, FILTER3)
            : BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_DOWN_SAMP, FILTER_MODE, BYPASS)) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_DOWN_SAMP, DERING_EN, !hDisplay->stCurInfo.bBypassVideoProcess); /* 1 if filter is FILTER_MODE == FILTER3 */

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_CRC_CONTROL + ulStgRegOffset);
        *pList->pulCurrent++ = BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CRC_CONTROL, CRC_ENABLE, ENABLE) |
                                BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CRC_CONTROL, CRC_WINDOW, FIELD);

#if (BVDC_P_ORTHOGONAL_VEC_VER >= BVDC_P_ORTHOGONAL_VEC_VER_1)
        if(pCurInfo->bStgNonRealTime){
            ulRegOffset = (hDisplay->hCompositor->eId - BVDC_CompositorId_eCompositor0) * sizeof(uint32_t);
            ulSrc       = BCHP_VEC_CFG_TRIGGER_SEL_0_SRC_STG_0 + ulStgId;

            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_TRIGGER_SEL_0 + ulRegOffset);
            *pList->pulCurrent++ = BCHP_MASK(VEC_CFG_TRIGGER_SEL_0, OVERRIDE) |
                ulSrc;
        }
#else
        BSTD_UNUSED(ulRegOffset);
        BSTD_UNUSED(ulSrc);
#endif
    }
    BDBG_LEAVE(BVDC_P_ProgramStgChan);
    return;
}

void BVDC_P_ProgrameStgMBox_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
#if ((BCHP_VICE2_ARCSS_ESS_DCCM_0_REG_START) ||(BCHP_VICE2_ARCSS_ESS_DCCM_0_0_REG_START)) || BVDC_P_SUPPORT_VIP
    BVDC_P_DisplayInfo              *pCurInfo = &hDisplay->stCurInfo;
    BVDC_P_DisplayStgChan           *pstChan;
    bool                            bRepeatPol;
    uint32_t                        ulRegOffset = 0;
#if !BVDC_P_SUPPORT_VIP
    bool                            bChannelChange = true;
    const BFMT_VideoInfo            *pFmtInfo;
    BFMT_Orientation                eOrientation;
    uint32_t                        ulViceIntAddr=0, ulViceIntMask=0, ulMboxAddr = 0;
    uint32_t                        *pulMboxBase = 0, ulCount = 0;
    BDBG_CASSERT(3 == BAVC_VICE_BVN2VICE_MAJORREVISION_ID);
#endif

    pstChan     = &hDisplay->stStgChan;
    ulRegOffset = hDisplay->ulStgRegOffset;

    /*MBox struture programing*/
    if(hDisplay->stCurInfo.bEnableStg)
    {
        /* Picture Id freeze only when NRT && bIgnorePicture */
        hDisplay->hCompositor->ulPicId += (!hDisplay->hCompositor->bIgnorePicture) || (!pCurInfo->bStgNonRealTime);

#ifdef BCHP_RDC_EOP_ID_256_eop_id_vec_0
        /* NRT mode sync slaved simul STGs should wait for EOP */
        if(hDisplay->hCompositor->bSyncSlave) {
            *pList->pulCurrent++ = BRDC_OP_WAIT_EOP(BCHP_RDC_EOP_ID_256_eop_id_vec_0 + hDisplay->hCompositor->eId);
        }
#endif
#if BVDC_P_SUPPORT_VIP
        if(hDisplay->hVip) {
            BVDC_P_Vip_BuildRul_isr(hDisplay->hVip, pList, eFieldPolarity);
        }
#else
        bChannelChange = hDisplay->hCompositor->bChannelChange && hDisplay->hCompositor->bGfxChannelChange;
        /* stg fmt is the display format when no video window connected; otherwise it'll walk thru video pipe. */
        if(0 == hDisplay->hCompositor->ulActiveVideoWindow)
        {
            hDisplay->pStgFmtInfo = pCurInfo->pFmtInfo;
        }
        pFmtInfo = (hDisplay->pStgFmtInfo==NULL)?pCurInfo->pFmtInfo:hDisplay->pStgFmtInfo;
        eOrientation = hDisplay->hCompositor->eDspOrientation;

        /* slave mode */
        ulMboxAddr        = pstChan->ulMBoxAddr;
        ulViceIntAddr     = pstChan->ulViceIntAddr;
        ulViceIntMask     = pstChan->ulViceIntMask;

        /* MBox structure DW 0: BVB Pic Size*/
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BAVC_VICE_BVN2VICE_DATA_SIZE);
        *pList->pulCurrent++ = BRDC_REGISTER(ulMboxAddr);
        pulMboxBase = pList->pulCurrent;
        *pList->pulCurrent++ =
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_00_BVB_PIC_SIZE, H_SIZE, pFmtInfo->ulDigitalWidth) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_00_BVB_PIC_SIZE, V_SIZE, pFmtInfo->ulDigitalHeight>>(pFmtInfo->bInterlaced));

        /* MBox structure DW 1: Sample aspect ratio*/
        /*
        *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MBOX_SAMPLE_ASPECT_RATIO, H_SIZE, hDisplay->hCompositor->iSampleAspectRatioX) |
        BCHP_FIELD_DATA(MBOX_SAMPLE_ASPECT_RATIO, V_SIZE, hDisplay->hCompositor->iSampleAspectRatioY);
        */
        *pList->pulCurrent++ = hDisplay->hCompositor->ulStgPxlAspRatio_x_y;

        /* MBox structure DW 2 Picture Info*/
        *pList->pulCurrent++ =
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_02_PIC_INFO, FRAME_RATE,   pCurInfo->ulVertFreq                     ) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_02_PIC_INFO, SRC_PIC_TYPE, hDisplay->hCompositor->ePictureType      ) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_02_PIC_INFO, POLARITY,     eFieldPolarity                           ) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_02_PIC_INFO, REPEAT,       hDisplay->hCompositor->bPictureRepeatFlag) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_02_PIC_INFO, LAST,         hDisplay->hCompositor->bLast             ) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_02_PIC_INFO, CHANNELCHANGE,bChannelChange                           ) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_02_PIC_INFO, IGNORE,       hDisplay->hCompositor->bIgnorePicture    ) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_02_PIC_INFO, ACTIVEFORMATDATA, hDisplay->hCompositor->bValidAfd     ) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_02_PIC_INFO, ACTIVEFORMATDATAMODE, hDisplay->hCompositor->ulAfd     );

        switch(hDisplay->stCurInfo.eBarDataMode) {
        case BVDC_Mode_eAuto: /* from video window computation */
        default:
            break;
        case BVDC_Mode_eOn:/* user override */
            hDisplay->hCompositor->eBarDataType = hDisplay->stCurInfo.eBarDataType;
            if(BAVC_BarDataType_eInvalid == hDisplay->hCompositor->eBarDataType) {
                hDisplay->hCompositor->ulTopLeftBarValue  = 0;
                hDisplay->hCompositor->ulBotRightBarValue = 0;
            } else {
                hDisplay->hCompositor->ulTopLeftBarValue = hDisplay->stCurInfo.ulTopLeftBarData;
                hDisplay->hCompositor->ulBotRightBarValue = hDisplay->stCurInfo.ulBotRightBarData;
            }
            break;
        case BVDC_Mode_eOff:/* diabled by user */
            hDisplay->hCompositor->eBarDataType = BAVC_BarDataType_eInvalid;
            hDisplay->hCompositor->ulTopLeftBarValue  = 0;
            hDisplay->hCompositor->ulBotRightBarValue = 0;
            break;
        }


        BDBG_MODULE_MSG(BVDC_CMP_SIZE, ("stg %dx%d%s%d orientation %d", pFmtInfo->ulDigitalWidth, pFmtInfo->ulDigitalHeight,
            pFmtInfo->bInterlaced? "i" : "p", pCurInfo->ulVertFreq, eOrientation));
        BDBG_MODULE_MSG(BVDC_DISP_STG,("STG display %d stg_id %d mbox %8x:", hDisplay->eId, pstChan->ulStg, ulMboxAddr));
        BDBG_MODULE_MSG(BVDC_DISP_STG,("%dx%d%s%d", pFmtInfo->ulDigitalWidth, pFmtInfo->ulDigitalHeight,
            pFmtInfo->bInterlaced? "i" : "p", pCurInfo->ulVertFreq));
        BDBG_MODULE_MSG(BVDC_DISP_STG,("ignore? %d", hDisplay->hCompositor->bIgnorePicture));
        BDBG_MODULE_MSG(BVDC_DISP_STG,("stall STC? %d", hDisplay->hCompositor->bStallStc));
        BDBG_MODULE_MSG(BVDC_DISP_STG,("repeat? %d", hDisplay->hCompositor->bPictureRepeatFlag));
        BDBG_MODULE_MSG(BVDC_DISP_STG,("Last? %d",   hDisplay->hCompositor->bLast));
        BDBG_MODULE_MSG(BVDC_DISP_STG,("ChannelChange? %d gfx %d video %d",   bChannelChange, hDisplay->hCompositor->bGfxChannelChange, hDisplay->hCompositor->bChannelChange));
        BDBG_MODULE_MSG(BVDC_DISP_STG,("PicId %d DecodePicId %d type %d, pol %d, asp ratio 0x%x, origPTS 0x%x, orientation %d",
            hDisplay->hCompositor->ulPicId, hDisplay->hCompositor->ulDecodePictureId,
            hDisplay->hCompositor->ePictureType, eFieldPolarity,
            hDisplay->hCompositor->ulStgPxlAspRatio_x_y, hDisplay->hCompositor->ulOrigPTS, eOrientation));
        BDBG_MODULE_MSG(BVDC_DISP_STG, ("bAfd? %d AfdMode %4d BarDataType %d Offset left %4x right %4x", hDisplay->hCompositor->bValidAfd,
            hDisplay->hCompositor->ulAfd, hDisplay->hCompositor->eBarDataType, hDisplay->hCompositor->ulTopLeftBarValue, hDisplay->hCompositor->ulBotRightBarValue));
        BDBG_MODULE_MSG(BVDC_DISP_STG,("chunkId %u, EOC?%d, preCharge?%d, bLast?%d",
            hDisplay->hCompositor->ulChunkId, hDisplay->hCompositor->bEndofChunk, hDisplay->hCompositor->bPreChargePicture, hDisplay->hCompositor->bLast));

        /* MBox structure DW 3: Original PTS*/
        *pList->pulCurrent++ = BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_03_ORIGINAL_PTS, VAL, hDisplay->hCompositor->ulOrigPTS);

        /* MBox structure DW 4 Picture Id*/
        *pList->pulCurrent++ = BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_04_STG_PICTURE_ID, VAL, hDisplay->hCompositor->ulPicId);

        /* MBox structure DW 5 Bar Data */
        *pList->pulCurrent++ =
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_05_BARDATA_INFO, BARDATATYPE,      hDisplay->hCompositor->eBarDataType      ) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_05_BARDATA_INFO, TOPLEFTBARVALUE,  hDisplay->hCompositor->ulTopLeftBarValue ) |
            BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_05_BARDATA_INFO, BOTRIGHTBARVALUE, hDisplay->hCompositor->ulBotRightBarValue);

        /* MBox structure DW 6 FNRT */
        *pList->pulCurrent++ =
        BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_06_FNRT_INFO, ENDOFCHUNK, hDisplay->hCompositor->bEndofChunk            ) |
        BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_06_FNRT_INFO, PRECHARGEPICTURE, hDisplay->hCompositor->bPreChargePicture) |
        BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_06_FNRT_INFO, CHUNKID, hDisplay->hCompositor->ulChunkId                 ) |
        BAVC_FIELD_DATA(VICE_BVN2VICE_DWORD_06_FNRT_INFO, FMT_ORIENTATION, eOrientation  );

        /* MBox structure zero-fill at end to be scalable for expansion in future; */
        ulCount = pList->pulCurrent - pulMboxBase;
        while(ulCount++ < BAVC_VICE_BVN2VICE_DATA_SIZE) {
            *pList->pulCurrent++ = 0;
        }

        /*Write to the interrupt register VICE2_ARCSS_ESS_P1_INTR2_0_ARC_P1_SET to vice*/
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(ulViceIntAddr);
        *pList->pulCurrent++ = ulViceIntMask;
#endif /* BVDC_P_SUPPORT_VIP */
    }

    /* NRT toggle trigger mode: if ignore frame, use combinational timer trigger to slow down (assume NRT faster than RT);
       else resume EOP only trigger;
       TODO: clarify combination trigger behavior; use timer or eop one at a time for now; */
    if(pCurInfo->bStgNonRealTime)
    {
        uint32_t ulStgCtrl;
#if (BDBG_DEBUG_BUILD)
        hDisplay->ulStgTriggerCount++;
        hDisplay->ulStgIgnoreCount += hDisplay->hCompositor->bIgnorePicture;
        BDBG_MODULE_MSG(BVDC_IGNORE_RATIO,("STG%d: ignore/total = %u/%u", pstChan->ulStg, hDisplay->ulStgIgnoreCount, hDisplay->ulStgTriggerCount));
#endif
        /* disable both timer and EOP triggers */
        ulStgCtrl = (
#if (BVDC_P_SUPPORT_STG_VER > BVDC_P_STG_VER_1)
            BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CONTROL, TRIG_MODE, AND) |
#endif
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, TIMEBASE_SEL,      pCurInfo->eTimeBase) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, SLAVE_MODE,       !BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg)) |
            BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CONTROL, HOST_ARM_ENABLE,   ENABLE) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, TIMER_TRIG_ENABLE, hDisplay->hCompositor->bIgnorePicture) |
#if (BVDC_P_SUPPORT_STG_VER == BVDC_P_STG_VER_1)
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, EOP_TRIG_ENABLE,  !hDisplay->hCompositor->bIgnorePicture) |
#else
            BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CONTROL, EOP_TRIG_ENABLE,  ENABLE) |
#endif
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, SCAN_MODE,         pCurInfo->pFmtInfo->bInterlaced));

        /* @@@How to handle B0 And/or logic for ignore picture*/
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_CONTROL + ulRegOffset);
        *pList->pulCurrent++ = ulStgCtrl;

#if (BVDC_P_SUPPORT_STG_VER == BVDC_P_STG_VER_1)
        /* Kludge: en/disable timer mode in same RUL seems to avoid extra timer
         * trigger when switch to EOP mode! */
        if(hDisplay->hCompositor->bIgnorePicture)
        {
            ulStgCtrl &= ~(
                BCHP_MASK(VIDEO_ENC_STG_0_CONTROL, TIMER_TRIG_ENABLE));

            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_CONTROL + ulRegOffset);
            *pList->pulCurrent++ = ulStgCtrl;
        }
#endif

#ifndef BVDC_P_STG_NRT_CADENCE_WORKAROUND
        bRepeatPol = hDisplay->hCompositor->bIgnorePicture;
        BREG_Write32_isr(hDisplay->hVdc->hRegister, BCHP_VIDEO_ENC_STG_0_REPEAT_POLARITY + ulRegOffset,
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_REPEAT_POLARITY, REPEAT_POL, bRepeatPol));

        BDBG_MODULE_MSG(BVDC_DISP_STG,("bRepeatPol %d mute %d", bRepeatPol, hDisplay->hCompositor->bMute));
#else
        BSTD_UNUSED(bRepeatPol);
#endif
#ifdef BVDC_P_SUPPORT_RDC_STC_FLAG
        BRDC_EnableStcFlag_isr(hDisplay->hVdc->hRdc, hDisplay->stStgChan.ulStg, !hDisplay->hCompositor->bStallStc);
#else
        BREG_Write32_isr(hDisplay->hVdc->hRegister, BCHP_VIDEO_ENC_STG_0_STC_CONTROL + ulRegOffset,
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_STC_CONTROL, STC_FLAG_ENABLE, !hDisplay->hCompositor->bStallStc));
#endif
        /* used for NRT mode transcode: default true to freeze STC and not get encoded */
        /* end of ignore picture scanout: no CRC callback in current isr if it's true! */
        hDisplay->hCompositor->bCrcIgnored       = hDisplay->hCompositor->bCrcToIgnore;
        /* being scanning out the ignore picture right now if true here; save it for next isr (end-of-scanout) to drop crc callback if true. */
        hDisplay->hCompositor->bCrcToIgnore      = hDisplay->hCompositor->bStgIgnorePicture;
        /* save the copy: building the RUL to scan out ignore picture */
        hDisplay->hCompositor->bStgIgnorePicture = hDisplay->hCompositor->bIgnorePicture;
        /* allow gfx only NRT encoding: TODO: allow gfx frame accurate NRT */
        hDisplay->hCompositor->bIgnorePicture = (0==hDisplay->hCompositor->ulActiveGfxWindow) || (hDisplay->hCompositor->ulActiveVideoWindow);
        hDisplay->hCompositor->bStallStc      = true;
        hDisplay->hCompositor->ulOrigPTS      = 0;
        hDisplay->hCompositor->bMute          = (NULL==hDisplay->hCompositor->hSyncLockSrc);
    }
    BSTD_UNUSED(pstChan);
#else
    BSTD_UNUSED(hDisplay);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eFieldPolarity);
#endif
    return;
}

void BVDC_P_ResetStgChanInfo
    (BVDC_P_DisplayStgChan            *pstStgChan)
{
    pstStgChan->bStgNonRealTime = false;
    pstStgChan->bEnable         = false;
    pstStgChan->ulMBoxAddr      = 0;
    pstStgChan->ulStg           = BVDC_P_HW_ID_INVALID;

    return;
}

BERR_Code BVDC_P_AllocStgChanResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_Display_Handle              hDisplay)
{
    BERR_Code err;
    uint32_t ulStgId;
    BVDC_P_DisplayStgChan            *pstStgChan;
    bool                             bMaster;

    pstStgChan = &hDisplay->stStgChan;
    bMaster = BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg);

    if(bMaster)
    {
        err = BVDC_P_Resource_Reserve_isr(hResource->pResourceRecords + hResource->pulIndex1stEntry[BVDC_P_ResourceType_eStg],
            BVDC_P_ResourceType_eStg, pstStgChan->ulStg, hDisplay->eId, (void**)&pstStgChan, &ulStgId);
    }
    else
    {
        err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eStg, 0, hDisplay->eId, &pstStgChan->ulStg, false);
        BVDC_P_Stg_Init_isr(hDisplay);
    }

    if (err)
    {
        BDBG_ERR(("No STG block available for display %d %s. Check hardware capability.", hDisplay->eId, bMaster?"master":"slave"));
    }
    return BERR_TRACE(err);
}

void BVDC_P_FreeStgChanResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_Display_Handle              hDisplay)
{
    BVDC_P_DisplayStgChan            *pstStgChan = &hDisplay->stStgChan;

    if (pstStgChan->ulStg!= BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eStg, pstStgChan->ulStg);
        pstStgChan->ulStg = BVDC_P_HW_ID_INVALID;
    }
#ifdef BCHP_PWR_RESOURCE_VDC_STG0
    if(hDisplay->ulStgPwrAcquire != 0)
    {
        hDisplay->ulStgPwrAcquire--;
        hDisplay->ulStgPwrRelease = 1;
        BDBG_MSG(("STG slave mode disable: Release pending BCHP_PWR_RESOURCE_VDC_STG"));
    }
#endif

    return;
}

BERR_Code BVDC_P_Display_Validate_Stg_Setting
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo *pNewInfo;
    BERR_Code err;
    bool      bStgSlave;
    pNewInfo = &hDisplay->stNewInfo;

#if !BVDC_P_SUPPORT_SEAMLESS_ATTACH
    hDisplay->stStgChan.bEnable =
        pNewInfo->bEnableStg && BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg);
    bStgSlave = (!BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg)) &&
                (true == pNewInfo->bEnableStg);

    /* Non Real time can not running @ slave mode */
    if( (true == pNewInfo->bStgNonRealTime) && bStgSlave ) {
        BDBG_ERR(("Slave mode cannot running on Non-real time mode!"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* allocate the stg chan for slave mode */
    /* SWSTB-1281 */
    /* coverity[cond_const: FALSE] */
    if(bStgSlave && (hDisplay->stStgChan.ulStg == BVDC_P_HW_ID_INVALID)) {
        BKNI_EnterCriticalSection();
        err = BVDC_P_AllocStgChanResources_isr(hDisplay->hVdc->hResource, hDisplay);
        BKNI_LeaveCriticalSection();
        if(BERR_SUCCESS !=err)
            return BERR_TRACE(err);

#ifdef BCHP_PWR_RESOURCE_VDC_STG0
        BDBG_MSG(("STG slave mode: Acquire BCHP_PWR_RESOURCE_VDC_STG %d", hDisplay->stStgChan.ulStg));
        BVDC_P_AcquireStgPwr(hDisplay);
#endif

#if (BVDC_P_SUPPORT_STG > 1)
        hDisplay->ulStgRegOffset = (hDisplay->stStgChan.ulStg) * (BCHP_VIDEO_ENC_STG_1_REG_START - BCHP_VIDEO_ENC_STG_0_REG_START);
#endif
    }
#else
    BSTD_UNUSED(err);
    BSTD_UNUSED(bStgSlave);
#endif

    /* allocate VIP buffer at runtime when enabled VIP heap */
#if BVDC_P_SUPPORT_VIP
    BDBG_MSG(("validate: new Vip heap=%p, curHeap=%p, hVip=%p", (void *)hDisplay->stNewInfo.hVipHeap, (void *)hDisplay->stCurInfo.hVipHeap, (void *)hDisplay->hVip));
    if (hDisplay->stNewInfo.hVipHeap && !hDisplay->stCurInfo.hVipHeap && !hDisplay->hVip) {
        BDBG_MSG(("To allocate VIP resource..."));
        /* SWSTB-1281 */
        /* coverity[overrun-local : FALSE] */
        BVDC_P_Vip_AllocBuffer(hDisplay->hVdc->ahVip[hDisplay->stStgChan.ulStg], hDisplay);
        /* SWSTB-1281 */
        /* coverity[overrun-local : FALSE] */
        BVDC_P_Vip_Init(hDisplay->hVdc->ahVip[hDisplay->stStgChan.ulStg]);
    }
    /* Note, when disabled, VIP will be freed after ApplyChanges and isr programs RUL to put VIP back to auto drain mode */
#endif

    return BERR_SUCCESS;
}


void BVDC_P_Display_Copy_Stg_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    if(hDisplay->stCurInfo.bStgNonRealTime != hDisplay->stNewInfo.bStgNonRealTime)
    {
        hDisplay->stStgChan.bModeSwitch = true;
    }

    if (!hDisplay->stCurInfo.bEnableStg && hDisplay->stNewInfo.bEnableStg)
    {
#if BVDC_P_SUPPORT_SEAMLESS_ATTACH
        /* Reset the Stg slave attachment process state */
        hDisplay->eStgSlaveState = BVDC_P_Slave_eInactive;
#else
        hDisplay->eStgState = BVDC_P_DisplayResource_eCreate;
#endif
    }
    else if(hDisplay->stCurInfo.bEnableStg && !hDisplay->stNewInfo.bEnableStg)
    {
        /* Tear down the Stg channel */
        hDisplay->eStgState = BVDC_P_DisplayResource_eDestroy;
    }
    hDisplay->stCurInfo.bEnableStg = hDisplay->stNewInfo.bEnableStg;
    hDisplay->stCurInfo.bStgNonRealTime = hDisplay->stNewInfo.bStgNonRealTime;
    hDisplay->stCurInfo.bBypassVideoProcess = hDisplay->stNewInfo.bBypassVideoProcess;
    hDisplay->stCurInfo.ulStcSnapshotHiAddr = hDisplay->stNewInfo.ulStcSnapshotHiAddr;
    hDisplay->stCurInfo.ulStcSnapshotLoAddr = hDisplay->stNewInfo.ulStcSnapshotLoAddr;
#if BVDC_P_SUPPORT_VIP
    BKNI_Memcpy(&hDisplay->stCurInfo.stVipMemSettings, &hDisplay->stNewInfo.stVipMemSettings, sizeof(BVDC_VipMemConfigSettings));
    hDisplay->stCurInfo.hVipHeap = hDisplay->stNewInfo.hVipHeap;
#endif
    return;
}

void BVDC_P_Display_Apply_Stg_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BVDC_P_DisplayInfo *pCurInfo;
#if BVDC_P_SUPPORT_SEAMLESS_ATTACH
    BVDC_P_DisplayStgChan *pstChan;
#endif

    BSTD_UNUSED(eFieldPolarity);

    pCurInfo = &hDisplay->stCurInfo;
#if BVDC_P_SUPPORT_SEAMLESS_ATTACH
    pstChan = &hDisplay->stStgChan;
#endif

    if (pCurInfo->bEnableStg)
    {
#if BVDC_P_SUPPORT_SEAMLESS_ATTACH /** { **/
        switch (hDisplay->eStgSlaveState)
        {
            case BVDC_P_Slave_eInactive:
                /* Acquire Stg core for slave mode */
                if (!BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg) &&
                  BVDC_P_AllocStgChanResources_isr(hDisplay->hVdc->hResource, hDisplay))
                {
                    BDBG_ERR(("No Stg available "));
                    return;
                }
                BVDC_P_ProgramStgChan_isr(hDisplay, pstChan, pList);

                hDisplay->eStgSlaveState = BVDC_P_Slave_eEnable;
                break;

            case BVDC_P_Slave_eEnable:
                BVDC_P_ConnectStgSrc_isr(hDisplay, pstChan, pList);
                hDisplay->eStgSlaveState = BVDC_P_Slave_eConnectSrc;
                break;

            case BVDC_P_Slave_eConnectSrc:
            default:
                hDisplay->eStgSlaveState = BVDC_P_Slave_eAttached;
                hDisplay->stCurInfo.stDirty.stBits.bStgEnable = BVDC_P_CLEAN;
                break;
        }

#else /** } BVDC_P_SUPPORT_SEAMLESS_ATTACH { **/

        BVDC_P_TearDownStgChan_isr(hDisplay, pList);

        /*
         * 2) Acquire necessary resources and set up the new path
         */
        BDBG_MSG(("BVDC_P_Display_Apply_VideoFormat Display %d allocates resource for STG chan", hDisplay->eId));
        BVDC_P_SetupStg_isr(hDisplay->ulStgRegOffset, pList);
        BVDC_P_ConnectStgSrc_isr(hDisplay, pList);

        /*
         * 3) Program the modules to the new format
         */
        BVDC_P_ProgramStgChan_isr(hDisplay, pList);
        /* When Stg is in slave mode, a format switch has to be done. Even
         * if the RUL somehow didn't get exeucted, our re-do mechanism will keep
         * trying. So it is safe to move the state to eActive directly.
         */
        hDisplay->eStgState = BVDC_P_DisplayResource_eActive;
        hDisplay->stCurInfo.stDirty.stBits.bStgEnable = BVDC_P_CLEAN;

#endif /** } BVDC_P_SUPPORT_SEAMLESS_ATTACH **/

    }
    else
    {
        /* Reset Stg core, disconnect source.
         * If Stg is the master timing generator, will this automatically
         * shut down all the slave paths?
         */
        switch (hDisplay->eStgState)
        {
            case BVDC_P_DisplayResource_eDestroy:
                BVDC_P_TearDownStgChan_isr(hDisplay, pList);
                hDisplay->stStgChan.bEnable = false;
#if BVDC_P_SUPPORT_VIP /* if STG is disabled, disable VIP too and clear the vip heap to free buffers later in ApplyChanges' checkStatus */
                if(hDisplay->hVip)
                {
                    hDisplay->stCurInfo.hVipHeap = NULL;
                }
#endif
                hDisplay->eStgState = BVDC_P_DisplayResource_eShuttingdown;
                break;

            case BVDC_P_DisplayResource_eShuttingdown:
                if (pList->bLastExecuted)
                {
                    BVDC_P_FreeStgChanResources_isr(hDisplay->hVdc->hResource, hDisplay);
                    hDisplay->eStgState = BVDC_P_DisplayResource_eInactive;
                    hDisplay->stCurInfo.stDirty.stBits.bStgEnable = BVDC_P_CLEAN;
                }
                else
                {
                    /* Re-build the teardown RUL */
                    BVDC_P_TearDownStgChan_isr(hDisplay, pList);
                }
                break;

            default:
                hDisplay->stCurInfo.stDirty.stBits.bStgEnable = BVDC_P_CLEAN;
                break;
        }
    }

    return;
}

#if (BVDC_P_STG_RUL_DELAY_WORKAROUND)
void BVDC_P_STG_DelayRUL_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      bool                             bMadr)
{
    if (BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg) &&
        hDisplay->stCurInfo.bStgNonRealTime )
    {
#if (BVDC_P_STG_VER_3 > BVDC_P_SUPPORT_STG_VER)
        /* experiment value */
        *pList->pulCurrent++ = BRDC_OP_REG_TO_REG(400);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_RDC_timer_data);
        *pList->pulCurrent++ = BRDC_REGISTER(hDisplay->ulScratchDummyAddr);
        BSTD_UNUSED(bMadr);
#else
        /* only mcvp needs the workaround */
        if(!bMadr) {
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_EOP_TRIGGER_DELAY + hDisplay->ulStgRegOffset);
            *pList->pulCurrent++ = 120;
        }
#endif
    }
}
#endif
#endif

/* End of file */
