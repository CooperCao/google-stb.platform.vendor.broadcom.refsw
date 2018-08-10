/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_rts_priv.h"
#include "bbox_priv.h"
#include "bbox_vdc.h"
#include "bbox_vdc_priv.h"
#include "bbox_priv_modes.h"
#include "bchp_memc_clients.h"
#include "bchp_memc_clients_chip_map.h"
#include "bchp_memc_arb_0.h"
#include "bchp_memc_arb_rdb_remap.h"

BDBG_MODULE(BBOX_PRIV_RTS);
BDBG_FILE_MODULE(BBOX_MEMC);
BDBG_OBJECT_ID(BBOX_PRIV_RTS);

#define INVALID (~0)

typedef struct BBOX_MemcClientEntry
{
    BCHP_MemcClient eClient;
    uint32_t ulMemc0Id;
#if BCHP_P_MEMC_COUNT > 1
    uint32_t ulMemc1Id;
#if BCHP_P_MEMC_COUNT > 2
    uint32_t ulMemc2Id;
#endif /* BCHP_P_MEMC_COUNT >= 2 */
#endif /* BCHP_P_MEMC_COUNT >= 1 */
} BBOX_MemcClientEntry;

static const BBOX_MemcClientEntry g_astMemcClientTbl[] = {

#if BCHP_P_MEMC_COUNT == 1
#define BCHP_P_MEMC_DEFINE_CLIENT(client, id0) { BCHP_MemcClient_e##client, id0 },
#elif BCHP_P_MEMC_COUNT == 2
#define BCHP_P_MEMC_DEFINE_CLIENT(client, id0, id1) { BCHP_MemcClient_e##client, id0, id1 },
#elif BCHP_P_MEMC_COUNT == 3
#define BCHP_P_MEMC_DEFINE_CLIENT(client, id0, id1, id2) { BCHP_MemcClient_e##client, id0, id1, id2 },
#else
#error "not_supported"
#endif

#include "memc/bchp_memc_clients_chip.h"
#undef BCHP_P_MEMC_DEFINE_CLIENT
};

void BBOX_P_SetRdcMemc
    ( BBOX_MemConfig *pBoxMemConfig,
      BBOX_MemcIndex  eMemcIndex )
{
    BDBG_ASSERT(pBoxMemConfig);

    pBoxMemConfig->stVdcMemcIndex.ulRdcMemcIndex = eMemcIndex;
}

void BBOX_P_SetDviCfcMemc
    ( BBOX_MemConfig *pBoxMemConfig,
      BBOX_MemcIndex  eMemcIndex )
{
    BDBG_ASSERT(pBoxMemConfig);

    pBoxMemConfig->stVdcMemcIndex.aulHdmiDisplayCfcMemcIndex[0] = eMemcIndex;
    pBoxMemConfig->stVdcMemcIndex.aulHdmiDisplayCfcMemcIndex[1] = BBOX_MemcIndex_Invalid;
}

void BBOX_P_SetVideoWinMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      BBOX_Vdc_DisplayId  display,
      BBOX_Vdc_WindowId   window,
      BBOX_MemcIndex      eWinMemcIndex,
      BBOX_MemcIndex      eDeinterlacerMemcIndex )
{
    BDBG_ASSERT(pBoxMemConfig);
    BDBG_ASSERT(display != BBOX_VDC_DISREGARD);
    BDBG_ASSERT(window <= BBOX_Vdc_Window_eVideo1);

    pBoxMemConfig->stVdcMemcIndex.astDisplay[display].aulVidWinCapMemcIndex[window] = (uint32_t)eWinMemcIndex;
    pBoxMemConfig->stVdcMemcIndex.astDisplay[display].aulVidWinMadMemcIndex[window] = (uint32_t)eDeinterlacerMemcIndex;
}

void BBOX_P_SetGfxWinMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      BBOX_Vdc_DisplayId  display,
      BBOX_Vdc_WindowId   window,
      BBOX_MemcIndex      eGfdMemcIndex )
{
    BDBG_ASSERT(pBoxMemConfig);
    BDBG_ASSERT(display != BBOX_VDC_DISREGARD);
    BDBG_ASSERT(window >= BBOX_Vdc_Window_eGfx0 && window <= BBOX_Vdc_Window_eGfx2);

    pBoxMemConfig->stVdcMemcIndex.astDisplay[display].aulGfdWinMemcIndex[window-BBOX_Vdc_Window_eGfx0] = (uint32_t)eGfdMemcIndex;
}

void BBOX_P_SetHdrVideoAndGfxMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      BBOX_Vdc_DisplayId  display,
      BBOX_MemcIndex      eCmpCfcMemcIndex,
      BBOX_MemcIndex      eGfdCfcMemcIndex )
{
    BDBG_ASSERT(pBoxMemConfig);
    BDBG_ASSERT(display != BBOX_VDC_DISREGARD);

    pBoxMemConfig->stVdcMemcIndex.astDisplay[display].ulCmpCfcMemcIndex = (uint32_t)eCmpCfcMemcIndex;
    pBoxMemConfig->stVdcMemcIndex.astDisplay[display].ulGfdCfcMemcIndex = (uint32_t)eGfdCfcMemcIndex;
}

void BBOX_P_SetNumMemc
    ( BBOX_MemConfig     *pBoxMemConfig,
      uint32_t            ulNumMemc )
{
    BDBG_ASSERT(pBoxMemConfig);

    pBoxMemConfig->ulNumMemc = ulNumMemc;
}

void BBOX_P_SetDramRefreshRate
    ( BBOX_MemConfig          *pBoxMemConfig,
      BBOX_DramRefreshRate     eRefreshRate )
{
    BDBG_ASSERT(pBoxMemConfig);

    pBoxMemConfig->eRefreshRate = eRefreshRate;
}

void BBOX_P_SetDefaultMemConfig
    ( BBOX_MemConfig     *pMemConfig )
{
    uint32_t i, j;

    pMemConfig->stVdcMemcIndex.ulRdcMemcIndex = 0;

    for (i=0; i<BBOX_VDC_HDMI_DISPLAY_COUNT; i++)
    {
        pMemConfig->stVdcMemcIndex.aulHdmiDisplayCfcMemcIndex[i] = BBOX_MemcIndex_Invalid;
    }

    for (i=0; i<BBOX_VDC_DISPLAY_COUNT; i++)
    {
        for (j=0; j<BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j] = BBOX_MemcIndex_Invalid;
            pMemConfig->stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j] = BBOX_MemcIndex_Invalid;
        }
        for (j=0; j<BBOX_VDC_GFX_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            pMemConfig->stVdcMemcIndex.astDisplay[i].aulGfdWinMemcIndex[j] = BBOX_MemcIndex_Invalid;
        }

        pMemConfig->stVdcMemcIndex.astDisplay[i].ulCmpCfcMemcIndex = BBOX_MemcIndex_Invalid;
        pMemConfig->stVdcMemcIndex.astDisplay[i].ulGfdCfcMemcIndex = BBOX_MemcIndex_Invalid;
    }
    pMemConfig->ulNumMemc = BBOX_INVALID_NUM_MEMC;
    pMemConfig->eRefreshRate = BBOX_DramRefreshRate_e1x;
}

BERR_Code BBOX_P_LoadRts
    ( const BREG_Handle          hReg,
      const uint32_t             ulBoxId,
      const BBOX_DramRefreshRate eRefreshRate )
{
    BERR_Code eStatus = BERR_SUCCESS;
    BBOX_Rts stBoxRts;

    BDBG_ASSERT(eRefreshRate < BBOX_DramRefreshRate_eInvalid);

    if (ulBoxId == 0)
    {
        BSTD_UNUSED(hReg);
        BSTD_UNUSED(ulBoxId);
    }
    else
    {
        BDBG_ASSERT(hReg);

        eStatus = BBOX_P_ValidateId(ulBoxId);
        if (eStatus != BERR_SUCCESS)
            goto done;

        eStatus = BBOX_P_GetRtsConfig(ulBoxId, &stBoxRts);
        if (eStatus == BERR_SUCCESS)
        {
            uint32_t i, j, k;
            BBOX_MemConfig boxMemCfg;

            /* verify box ID */
            if (stBoxRts.ulBoxId != ulBoxId)
            {
                BDBG_ERR(("Mismatched box ID between device tree/env var and RTS file."));
                eStatus = BBOX_ID_AND_RTS_MISMATCH;
                goto done;
            }

            for (i=0;i<stBoxRts.ulNumMemc;i++)
            {
                uint32_t ulMemcBaseAddr = 0x0;

#ifdef BCHP_MEMC_ARB_0_REG_START
                if (i==0)
                {
                    ulMemcBaseAddr = BCHP_MEMC_ARB_0_CLIENT_INFO_0;
                }
#endif
#ifdef BCHP_MEMC_ARB_1_REG_START
                else if (i==1)
                {
                    ulMemcBaseAddr = BCHP_MEMC_ARB_1_CLIENT_INFO_0;
                }
#endif
#ifdef BCHP_MEMC_ARB_2_REG_START
                else if (i==2)
                {
                    ulMemcBaseAddr = BCHP_MEMC_ARB_2_CLIENT_INFO_0;
                }
#endif

                BDBG_ASSERT(ulMemcBaseAddr);

                if (stBoxRts.paulMemc == NULL)
                {
                    BDBG_ERR(("Box mode %d RTS MEMC Client list is uninitialized.", ulBoxId));
                    if (stBoxRts.pchRtsVersion)
                    {
                        BDBG_ERR(("RTS configuration version %s", stBoxRts.pchRtsVersion));
                    }
                    eStatus = BBOX_RTS_CFG_UNINITIALIZED;
                    goto done;
                }

                for (j=0;j<stBoxRts.ulNumMemcEntries;j++)
                {
                    if(stBoxRts.paulMemc[i] == NULL)
                    {
                        BDBG_ERR(("Box mode %d RTS MEMC %d Client list is uninitialized.", ulBoxId, i));
                        if (stBoxRts.pchRtsVersion)
                        {
                            BDBG_ERR(("RTS configuration version %s", stBoxRts.pchRtsVersion));
                        }
                        eStatus = BBOX_RTS_CFG_UNINITIALIZED;
                        goto done;
                    }
                    BREG_Write32(hReg, ulMemcBaseAddr+(j*4), stBoxRts.paulMemc[i][j]);
                    BDBG_MODULE_MSG(BBOX_MEMC, ("MEMC_%d[%d] = 0x%x", i, j, stBoxRts.paulMemc[i][j]));
                }

                eStatus = BBOX_P_SetMemConfig(ulBoxId, &boxMemCfg);
                if (eStatus != BERR_SUCCESS)
                {
                     goto done;
                }
                else
                {
                    uint32_t ulRefreshClient = BCHP_MemcClient_eMax;

                    for(k=0; g_astMemcClientTbl[k].eClient < BCHP_MemcClient_eMax; k++)
                    {
                        unsigned uRefRateFactor = boxMemCfg.eRefreshRate/eRefreshRate;

                        BCHP_MemcClient eClient = g_astMemcClientTbl[k].eClient;

#if BCHP_P_MEMC_CLIENT_EXISTS_REFRESH

                        if (eClient == BCHP_MemcClient_eREFRESH)
                        {
                           ulRefreshClient = k;
                        }
#elif BCHP_P_MEMC_CLIENT_EXISTS_REF
                        if (eClient == BCHP_MemcClient_eREF)
                        {
                           ulRefreshClient = k;
                        }
#else
                        BSTD_UNUSED(eClient);
                        BDBG_WRN(("%d doesn't have a DRAM refresh SCB client", BCHP_CHIP));
                        break;
#endif

                        /* Update REFRESH client blockout if requested by upper layer SW */
                        if (uRefRateFactor > 1 && (ulRefreshClient != BCHP_MemcClient_eMax))
                        {
                            uint32_t ulRefreshClientId = INVALID, ulHiTempBlockOut;

                            if (i==0)
                            {
                                ulRefreshClientId = g_astMemcClientTbl[k].ulMemc0Id;
                            }
#if BCHP_P_MEMC_COUNT > 1
                            else if (i==1)
                            {
                                ulRefreshClientId = g_astMemcClientTbl[k].ulMemc1Id;
                            }
#if BCHP_P_MEMC_COUNT > 2
                            else if (i==2)
                            {
                                ulRefreshClientId = g_astMemcClientTbl[k].ulMemc2Id;
                            }
#endif
#endif
                            if (ulRefreshClientId != (unsigned)INVALID)
                            {
                                ulHiTempBlockOut = (stBoxRts.paulMemc[i][ulRefreshClientId] * uRefRateFactor) - 1 ;
                                BREG_Write32(hReg, ulMemcBaseAddr+(ulRefreshClientId*4), ulHiTempBlockOut);
                                BDBG_MODULE_MSG(BBOX_MEMC, ("Modifying MEMC_%d REFRESH client to 0x%x", i, ulHiTempBlockOut));
                            }
                        }
                    }
                }
            }

            if (stBoxRts.pastAddrDataPairs == NULL)
            {
                BDBG_ERR(("Box mode %d RTS Addr-Data pair list is uninitialized.", ulBoxId));
                if (stBoxRts.pchRtsVersion)
                {
                    BDBG_ERR(("RTS configuration version %s", stBoxRts.pchRtsVersion));
                }
                eStatus = BBOX_RTS_CFG_UNINITIALIZED;
                goto done;
            }

            for (i=0;i<stBoxRts.ulNumAddrDataPairs;i++)
            {
                if (stBoxRts.pastAddrDataPairs[i].ulAddr == BBOX_RTS_INVALID_ADDR) break;

                BREG_Write32(hReg, stBoxRts.pastAddrDataPairs[i].ulAddr, stBoxRts.pastAddrDataPairs[i].ulData);
                BDBG_MODULE_MSG(BBOX_MEMC, ("Addr-Data pair[%d] = 0x%x : 0x%x", i, stBoxRts.pastAddrDataPairs[i].ulAddr, stBoxRts.pastAddrDataPairs[i].ulData));
            }
        }
        else if (eStatus == BBOX_RTS_LOADED_BY_CFE)
        {
            eStatus = BERR_SUCCESS;
            BSTD_UNUSED(hReg);
            BSTD_UNUSED(ulBoxId);
        }
    }
done:
    return eStatus;
}
