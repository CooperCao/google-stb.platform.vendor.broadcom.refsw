/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv.h"
#include "bbox_vdc.h"
#include "bbox_vdc_priv.h"
#include "bbox_priv_modes.h"
#include "bchp_memc_clients.h"
#include "bchp_memc_arb_0.h"

#ifdef BCHP_MEMC_ARB_1_REG_START
#include "bchp_memc_arb_1.h"
#endif

#ifdef BCHP_MEMC_ARB_2_REG_START
#include "bchp_memc_arb_2.h"
#endif

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

BERR_Code BBOX_P_LoadRts
    ( const BREG_Handle          hReg,
      const uint32_t             ulBoxId,
      const BBOX_DramRefreshRate eRefreshRate )
{
    BERR_Code eStatus = BERR_SUCCESS;
    BBOX_Rts stBoxRts;
    uint32_t ulRefreshClient;

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
                BDBG_ASSERT(stBoxRts.paulMemc[i][0]);

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


                for (j=0;j<stBoxRts.ulNumMemcEntries;j++)
                {
                    BREG_Write32(hReg, ulMemcBaseAddr+(j*4), stBoxRts.paulMemc[i][j]);
                    BDBG_MODULE_MSG(BBOX_MEMC, ("MEMC_%d[%d] = 0x%x", i, j, stBoxRts.paulMemc[i][j]));

                }

                ulRefreshClient = BCHP_MemcClient_eMax;

                BBOX_P_GetMemConfig(ulBoxId, &boxMemCfg);

                for(k=0; g_astMemcClientTbl[k].eClient < BCHP_MemcClient_eMax; k++)
                {
                    unsigned uRefRateFactor = boxMemCfg.eRefreshRate/eRefreshRate;

                    BCHP_MemcClient eClient = g_astMemcClientTbl[k].eClient;

#if (BCHP_CHIP==7435)
                    if (eClient == BCHP_MemcClient_eREF)
                    {
                       ulRefreshClient = k;
                    }
#else
                    if (eClient == BCHP_MemcClient_eREFRESH)
                    {
                       ulRefreshClient = k;
                    }
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



            for (i=0;i<stBoxRts.ulNumPfriClients;i++)
            {
                BREG_Write32(hReg, stBoxRts.pastPfriClient[i].ulAddr, stBoxRts.pastPfriClient[i].ulData);
                BDBG_MODULE_MSG(BBOX_MEMC, ("PFRI[%d] = 0x%x : 0x%x", i, stBoxRts.pastPfriClient[i].ulAddr, stBoxRts.pastPfriClient[i].ulData));
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
