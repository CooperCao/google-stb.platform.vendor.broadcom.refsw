/******************************************************************************
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
 ******************************************************************************/

#include "nexus_parser_band_channelbonding.h"
#include "nexus_transport_module.h"
#include "priv/nexus_transport_priv.h"

BDBG_MODULE(nexus_parser_band_channelbonding);

NEXUS_GcbSwHandle NEXUS_Gcb_P_Open(unsigned index, const NEXUS_ParserBandStartBondingGroupSettings *pSettings);
NEXUS_Error NEXUS_Gcb_P_AddParserBand(NEXUS_ParserBandHandle master, NEXUS_ParserBandHandle slave);
void NEXUS_Gcb_P_CheckStart(void* param);
NEXUS_Error NEXUS_Gcb_P_Start(NEXUS_GcbSwHandle hGcb);
void NEXUS_Gcb_P_Stop(NEXUS_GcbSwHandle hGcb);
void NEXUS_Gcb_P_Close(NEXUS_GcbSwHandle hGcb);
NEXUS_Error NEXUS_Gcb_P_GetStatus(NEXUS_GcbSwHandle hGcb, NEXUS_ParserBandBondingGroupStatus *pStatus);

void NEXUS_ParserBand_GetDefaultStartBondingGroupSettings(NEXUS_ParserBandStartBondingGroupSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->bondingPid = 0x1ffe;
    pSettings->soft.playpumpIndex = NEXUS_ANY_ID;
}

NEXUS_Error NEXUS_ParserBand_StartBondingGroup(NEXUS_ParserBand master, const NEXUS_ParserBandStartBondingGroupSettings *pSettings)
{
    unsigned i;
    NEXUS_Error rc;
    NEXUS_ParserBandHandle masterPb = NULL;
    masterPb = NEXUS_ParserBand_Resolve_priv(master);

    if (masterPb==NULL) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto error;
    }

    masterPb->gcbSwHandle = NEXUS_Gcb_P_Open(masterPb->hwIndex, pSettings);
    if (masterPb->gcbSwHandle == NULL) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }

    rc = NEXUS_Gcb_P_AddParserBand(masterPb->gcbSwHandle, masterPb);
    if (rc) { goto error; }

    for (i=0; i<NEXUS_MAX_PARSER_BANDS; i++) {
        if (pSettings->slave[i]) {
            NEXUS_ParserBandHandle slavePb = NULL;
            slavePb = NEXUS_ParserBand_Resolve_priv(pSettings->slave[i]);
            if (slavePb==NULL) {
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto error;
            }
            rc = NEXUS_Gcb_P_AddParserBand(masterPb->gcbSwHandle, slavePb);
            if (rc) { goto error; }
        }
    }

    NEXUS_ScheduleTimer(250, NEXUS_Gcb_P_CheckStart, masterPb->gcbSwHandle);
    return NEXUS_SUCCESS;

error:
    if (masterPb && masterPb->gcbSwHandle) {
        NEXUS_Gcb_P_Close(masterPb->gcbSwHandle);
    }
    return rc;
}

void NEXUS_ParserBand_StopBondingGroup(NEXUS_ParserBand master)
{
    NEXUS_Error rc;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_ParserBandHandle masterPb = NULL;
    masterPb = NEXUS_ParserBand_Resolve_priv(master);

    if (masterPb==NULL) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    if (masterPb->gcbSwHandle==NULL) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    NEXUS_Gcb_P_Stop(masterPb->gcbSwHandle);
    playpump = ((NEXUS_GcbSwHandle)masterPb->gcbSwHandle)->playpump;
    NEXUS_OBJECT_UNREGISTER(NEXUS_Playpump, playpump, Destroy);
    NEXUS_Gcb_P_Close(masterPb->gcbSwHandle);
    masterPb->gcbSwHandle = NULL;
}

NEXUS_Error NEXUS_ParserBand_GetBondingGroupStatus(NEXUS_ParserBand master, NEXUS_ParserBandBondingGroupStatus *pStatus)
{
    NEXUS_ParserBandHandle masterPb;
    BDBG_ASSERT(pStatus);
    masterPb = NEXUS_ParserBand_Resolve_priv(master);
    if (!masterPb) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (masterPb->gcbSwHandle==NULL) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    return NEXUS_Gcb_P_GetStatus(masterPb->gcbSwHandle, pStatus);
}

NEXUS_PlaypumpHandle NEXUS_ParserBand_GetBondingGroupPlaypump(NEXUS_ParserBand master)
{
    NEXUS_Error rc;
    NEXUS_ParserBandHandle masterPb = NULL;
    NEXUS_PlaypumpHandle playpump;
    masterPb = NEXUS_ParserBand_Resolve_priv(master);

    if (masterPb==NULL) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (masterPb->gcbSwHandle==NULL) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    playpump = ((NEXUS_GcbSwHandle)masterPb->gcbSwHandle)->playpump;
    NEXUS_OBJECT_REGISTER(NEXUS_Playpump, playpump, Create);

    return playpump;
}
