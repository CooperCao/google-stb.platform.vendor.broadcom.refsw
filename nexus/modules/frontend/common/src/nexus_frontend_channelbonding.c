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

#include "nexus_frontend_module.h"
#include "priv/nexus_frontend_mtsif_priv.h"
#include "priv/nexus_core.h"
#if NEXUS_HAS_MXT
#include "bmxt.h"
#include "bmxt_dcbg.h"
#endif

BDBG_MODULE(nexus_frontend_channelbonding);

#if NEXUS_HAS_MXT
typedef struct NEXUS_FrontendChannelBonding
{
    unsigned index;
    BMXT_Dcbg_Handle hDcbg;
    unsigned destBand;

    struct {
        NEXUS_FrontendHandle frontend;
        unsigned demodParserNum;
    } bands[NEXUS_MAX_FRONTENDS]; /* master and slave bands */
} NEXUS_FrontendChannelBonding;

NEXUS_Error NEXUS_Frontend_P_Sat_SetChannelBondingConfig(void *handle, bool enable);

void NEXUS_Frontend_GetDefaultStartBondingGroupSettings(NEXUS_FrontendStartBondingGroupSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static NEXUS_Error NEXUS_Frontend_P_ConfigureBand(NEXUS_FrontendHandle master, NEXUS_FrontendHandle frontend)
{
    NEXUS_FrontendChannelBondingHandle bond = master->chbond;
    NEXUS_FrontendDeviceMtsifConfig *mtsifConfig = &master->pGenericDeviceHandle->mtsifConfig;
    BMXT_InputBandConfig inputConfig;
    BMXT_ParserConfig parserConfig;
    unsigned i, parserNum, inputNum, mtsifTxSel;
    BERR_Code rc;
    bool isMaster = (master==frontend);

    BDBG_ASSERT(bond);

    parserNum = mtsifConfig->numDemodPb;

    /* find master's existing connection to host PB */
    if (isMaster) {
        for (i=0; i<mtsifConfig->numDemodPb; i++) {
            if (mtsifConfig->demodPbSettings[i].connector==NEXUS_Frontend_GetConnector(frontend)) {
                parserNum = i;
                break;
            }
        }
        if (parserNum >= mtsifConfig->numDemodPb) {
#if 1 /* mandate existing connection before starting bond */
            BDBG_ERR(("%u: master frontend %p not yet connected to a host parserband", bond->index, (void*)frontend));
            return NEXUS_NOT_SUPPORTED;
#else
            BDBG_WRN(("%u: master frontend %p not yet connected to a host parserband", bond->index, (void*)frontend));
            /* TODO: must change nexus_frontend.c logic such that, it picks the same demod parserband that's selected here */
#endif
        }
    }
    else { /* for slaves, pick a free demod parser and "reserve" it by marking it enabled */
        for (i=0; i<mtsifConfig->numDemodPb; i++) {
            if (mtsifConfig->demodPbSettings[i].enabled==false) {
                parserNum = i;
                break;
            }
        }
        if (i>=mtsifConfig->numDemodPb) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
    }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++) {
        if (bond->bands[i].frontend==frontend) { /* already added */
            BDBG_ASSERT(bond->bands[i].demodParserNum==parserNum);
            goto done;
        }
        else if (bond->bands[i].frontend==NULL) {
            break;
        }
    }
    if (i>=NEXUS_MAX_FRONTENDS) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    mtsifTxSel = frontend->mtsif.txOut;
    inputNum = frontend->mtsif.inputBand;
    rc = BMXT_GetInputBandConfig(mtsifConfig->mxt, inputNum, &inputConfig); if (rc) { BERR_TRACE(rc); }
    inputConfig.packetEndDetectEn = true;
    inputConfig.ParallelInputSel = true;
    rc = BMXT_SetInputBandConfig(mtsifConfig->mxt, inputNum, &inputConfig); if (rc) { BERR_TRACE(rc); }

    rc = BMXT_GetParserConfig(mtsifConfig->mxt, parserNum, &parserConfig);  if (rc) { BERR_TRACE(rc); }
    parserConfig.InputBandNumber = inputNum;
    parserConfig.Enable = false; /* BMXT_Dcbg_Start will handle enable/disable of bands */
    parserConfig.mtsifTxSelect = mtsifTxSel;
    if (mtsifConfig->pidfilter) {
        parserConfig.AllPass = true;
        parserConfig.AcceptNulls = true;
    }
    rc = BMXT_SetParserConfig(mtsifConfig->mxt, parserNum, &parserConfig);  if (rc) { BERR_TRACE(rc); }

    rc = BMXT_Dcbg_AddParser(bond->hDcbg, parserNum); if (rc) { return BERR_TRACE(NEXUS_UNKNOWN); }
    mtsifConfig->demodPbSettings[parserNum].enabled = true; /* this prevents nexus_frontend.c code from trying to use it */

    bond->bands[i].frontend = frontend;
    bond->bands[i].demodParserNum = parserNum;
    if (isMaster) {
        bond->destBand = mtsifConfig->demodPbSettings[parserNum].virtualNum;
    }

#if NEXUS_FRONTEND_SAT
    rc = NEXUS_Frontend_P_Sat_SetChannelBondingConfig(frontend, true);
    if (!rc) { BERR_TRACE(rc); } /* keep going, though no guarantee it'll work */
#endif

    BDBG_MSG(("%u: %p:%p, band[%u] demodIB%2u, demodPB%2u, TX%u", bond->index, (void*)master, (void*)frontend, i, inputNum, parserNum, mtsifTxSel));
done:
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_DeconfigureAllBands(NEXUS_FrontendHandle master)
{
    NEXUS_FrontendChannelBondingHandle bond = master->chbond;
    NEXUS_FrontendDeviceMtsifConfig *mtsifConfig = &master->pGenericDeviceHandle->mtsifConfig;
    BMXT_InputBandConfig inputConfig;
    BMXT_ParserConfig parserConfig;
    unsigned i, parserNum, inputNum;
    BERR_Code rc;

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++) {
        NEXUS_FrontendHandle slave = bond->bands[i].frontend;
        if (slave==NULL) { break; }

        inputNum = bond->bands[i].frontend->mtsif.inputBand;
        parserNum = bond->bands[i].demodParserNum;

        rc = BMXT_Dcbg_RemoveParser(bond->hDcbg, parserNum);
        if (rc) { BERR_TRACE(rc); }

        rc = BMXT_GetInputBandConfig(mtsifConfig->mxt, inputNum, &inputConfig); if (rc) { BERR_TRACE(rc); }
        inputConfig.packetEndDetectEn = false;
        inputConfig.ParallelInputSel = false;
        rc = BMXT_SetInputBandConfig(mtsifConfig->mxt, inputNum, &inputConfig); if (rc) { BERR_TRACE(rc); }

        if (slave != master) {
            mtsifConfig->demodPbSettings[parserNum].enabled = false;
        }
        else {
            /* re-enable the demodPB of master, since was enabled prior to starting bonding */
            rc = BMXT_GetParserConfig(mtsifConfig->mxt, parserNum, &parserConfig);  if (rc) { BERR_TRACE(rc); }
            parserConfig.Enable = true;
            rc = BMXT_SetParserConfig(mtsifConfig->mxt, parserNum, &parserConfig);  if (rc) { BERR_TRACE(rc); }
        }

        if (mtsifConfig->pidfilter) {
            rc = BMXT_GetParserConfig(mtsifConfig->mxt, parserNum, &parserConfig);  if (rc) { BERR_TRACE(rc); }
            parserConfig.AllPass = false;
            parserConfig.AcceptNulls = false;
            rc = BMXT_SetParserConfig(mtsifConfig->mxt, parserNum, &parserConfig);  if (rc) { BERR_TRACE(rc); }
        }
        bond->bands[i].frontend->bondingMaster = NULL;

#if NEXUS_FRONTEND_SAT
        NEXUS_Frontend_P_Sat_SetChannelBondingConfig(slave, false);
#endif
    }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Frontend_StartBondingGroup(NEXUS_FrontendHandle master, const NEXUS_FrontendStartBondingGroupSettings *pSettings)
{
    NEXUS_FrontendChannelBondingHandle bond;
    NEXUS_FrontendDeviceMtsifConfig *mtsifConfig;
    BMXT_Dcbg_OpenSettings dcbgOpenSettings;
    BMXT_Dcbg_Settings dcbgSettings;
    unsigned i;
    NEXUS_Error rc;

    NEXUS_OBJECT_ASSERT(NEXUS_Frontend, master);
    if (master->pGenericDeviceHandle->openPending) {
        BDBG_ERR(("Device open still pending on frontend %p", (void*)master));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if (master->chbond) {
        BDBG_ERR(("Frontend %p is already being used as bonding group master", (void*)master));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (master->bondingMaster) {
        BDBG_ERR(("Frontend %p is already being used as slave in a bonding group", (void*)master));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    mtsifConfig = &master->pGenericDeviceHandle->mtsifConfig;
    BDBG_ASSERT(mtsifConfig->mxt);

    /* alloc ChannelBonding struct only on master */
    master->chbond = BKNI_Malloc(sizeof(struct NEXUS_FrontendChannelBonding));
    if (!master->chbond) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    bond = master->chbond;
    BKNI_Memset(bond, 0, sizeof(*bond));
    bond->index = BMXT_Dcbg_GetFreeIndex(mtsifConfig->mxt);

    BMXT_Dcbg_GetDefaultOpenSettings(&dcbgOpenSettings);
    rc = BMXT_Dcbg_Open(mtsifConfig->mxt, &bond->hDcbg, bond->index, &dcbgOpenSettings);
    if (rc) {
        BKNI_Free(bond);
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    BDBG_ASSERT(bond->hDcbg);

    rc = NEXUS_Frontend_P_ConfigureBand(master, master);
    if (rc) { goto error; }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++) {
        if (pSettings->slave[i] == NULL) { continue; }

        if (&pSettings->slave[i]->pGenericDeviceHandle->mtsifConfig != mtsifConfig) {
            BDBG_ERR(("Slave frontend %p is not on same device as master frontend %p", (void*)pSettings->slave[i], (void*)master));
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto error;
        }
        rc = NEXUS_Frontend_P_ConfigureBand(master, pSettings->slave[i]);
        if (rc) { goto error; }
        pSettings->slave[i]->bondingMaster = master;
    }

    /* check frontend lock statuses prior to starting */
    for (i=0; i<NEXUS_MAX_FRONTENDS; i++) {
        if (bond->bands[i].frontend) {
            NEXUS_FrontendFastStatus fstatus;
            NEXUS_Frontend_GetFastStatus(bond->bands[i].frontend, &fstatus);
            if (fstatus.lockStatus!=NEXUS_FrontendLockStatus_eLocked)
                /* (&& !fstatus.acquireInProgress) */ /* TODO: acquireInProgress does not return false if Tune() never called */
            {
                BDBG_WRN(("Bonding group (master %p) started with frontend %p not locked", (void*)master, (void*)bond->bands[i].frontend));
            }
        }
    }

    BDBG_ASSERT(bond->bands[0].frontend==master); /* master is always placed in slot [0] */
    BMXT_Dcbg_GetDefaultSettings(bond->hDcbg, &dcbgSettings);
    dcbgSettings.primaryBand = bond->bands[0].demodParserNum;

    BDBG_MSG(("%u: %p -> PB%u, start (demod priband %u)", bond->index, (void*)master, bond->destBand, dcbgSettings.primaryBand));
    rc = BMXT_Dcbg_Start(bond->hDcbg, &dcbgSettings);
    if (rc) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
    return NEXUS_SUCCESS;

error:
    NEXUS_Frontend_StopBondingGroup(master);
    return rc;
}

void NEXUS_Frontend_StopBondingGroup(NEXUS_FrontendHandle master)
{
    NEXUS_FrontendChannelBondingHandle bond;
    NEXUS_OBJECT_ASSERT(NEXUS_Frontend, master);
    if (master->chbond == NULL) {
        BDBG_ERR(("Frontend %p is not a bonding group master", (void*)master));
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    bond = master->chbond;
    BDBG_ASSERT(bond->hDcbg);

    BMXT_Dcbg_Stop(bond->hDcbg);
    NEXUS_Frontend_P_DeconfigureAllBands(master);
    BMXT_Dcbg_Close(bond->hDcbg);

    BKNI_Free(bond);
    master->chbond = NULL;
}

void NEXUS_Frontend_P_ResetBondingGroup(NEXUS_FrontendHandle frontend)
{
    NEXUS_FrontendHandle master;
    unsigned i;
    bool allLocked = true;

    if (frontend->chbond) {
        master = frontend;
    }
    else if (frontend->bondingMaster) {
        master = frontend->bondingMaster;
    }
    else {
        return;
    }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++) {
        if (master->chbond->bands[i].frontend) {
            NEXUS_FrontendFastStatus fstatus;
            NEXUS_Frontend_GetFastStatus(master->chbond->bands[i].frontend, &fstatus);
            if (fstatus.lockStatus!=NEXUS_FrontendLockStatus_eLocked) {
                allLocked = false;
                break;
            }
        }
    }

    if (allLocked) {
        BDBG_WRN(("%u: Resetting bond on new frontend group lock", master->chbond->index));
        BMXT_Dcbg_Reacquire(master->chbond->hDcbg);
    }

}

NEXUS_Error NEXUS_Frontend_GetBondingGroupStatus(NEXUS_FrontendHandle master, NEXUS_FrontendBondingGroupStatus *pStatus)
{
    NEXUS_FrontendChannelBondingHandle bond;
    BMXT_Dcbg_Status dcbgStatus;
    BERR_Code rc;
    BDBG_ASSERT(pStatus);
    pStatus->locked = false;

    NEXUS_OBJECT_ASSERT(NEXUS_Frontend, master);
    if (master->chbond==NULL) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    bond = master->chbond;

    rc = BMXT_Dcbg_GetStatus(bond->hDcbg, &dcbgStatus);
    if (rc) {
        return BERR_TRACE(rc);
    }
    pStatus->locked = dcbgStatus.locked;
    return NEXUS_SUCCESS;
}

#else /* NEXUS_HAS_MXT */
void NEXUS_Frontend_GetDefaultStartBondingGroupSettings(NEXUS_FrontendStartBondingGroupSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    return;
}

NEXUS_Error NEXUS_Frontend_StartBondingGroup(NEXUS_FrontendHandle master, const NEXUS_FrontendStartBondingGroupSettings *pSettings)
{
    BSTD_UNUSED(master);
    BSTD_UNUSED(pSettings);
    return 0;
}

void NEXUS_Frontend_StopBondingGroup(NEXUS_FrontendHandle master)
{
    BSTD_UNUSED(master);
    return;
}

NEXUS_Error NEXUS_Frontend_GetBondingGroupStatus(NEXUS_FrontendHandle master, NEXUS_FrontendBondingGroupStatus *pStatus)
{
    BSTD_UNUSED(master);
    BSTD_UNUSED(pStatus);
    return 0;
}
#endif
