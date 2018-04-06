/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_transport_module.h"

BDBG_MODULE(nexus_pid_channel_scrambling);

void NEXUS_PidChannel_GetDefaultScramblingSettings( NEXUS_PidChannelScramblingSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

#if BXPT_HAS_RAVE_SCRAMBLING_CONTROL
static NEXUS_Error NEXUS_P_PidChannel_Master_StartScramblingCheck( NEXUS_P_HwPidChannel *pidChannel, const NEXUS_PidChannelScramblingSettings *pSettings )
{
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);

    if (pidChannel->scramblingCheck.rave) {
        NEXUS_P_HwPidChannel_StopScramblingCheck(pidChannel);
    }

    if (pSettings) {
        pidChannel->scramblingCheck.settings = *pSettings;
    }
    else {
        NEXUS_PidChannel_GetDefaultScramblingSettings(&pidChannel->scramblingCheck.settings);
    }

    if ( pidChannel->scramblingCheck.settings.raveContext) {
        if (!pidChannel->scramblingCheck.settings.raveContext->enabled) {
            BDBG_ERR(("external RAVE context must be already started before calling NEXUS_PidChannel_StartScramblingCheck"));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        pidChannel->scramblingCheck.rave = pidChannel->scramblingCheck.settings.raveContext;
    }
    else {
        NEXUS_Error rc;
        NEXUS_RaveSettings raveSettings;
        NEXUS_RaveOpenSettings raveOpenSettings;
        
        /* the nexus code does not try to evacuate the rave buffer. scrambling check is done before any overflow check,
        so it's ok if the rave buffer overflows. this is the minimum amount of memory for rave to put data into a buffer. */
        NEXUS_Rave_GetDefaultOpenSettings_priv(&raveOpenSettings);
        raveOpenSettings.config.Cdb.Length = 4096*2; /* minimum value for XPT PI threshold calculations */
        raveOpenSettings.config.Cdb.Alignment = 4;
        raveOpenSettings.config.Cdb.LittleEndian = true;
        raveOpenSettings.config.Itb.Length = 4096*2;
        raveOpenSettings.config.Itb.Alignment = 4;
        raveOpenSettings.config.Itb.LittleEndian = true;

        pidChannel->scramblingCheck.rave = NEXUS_Rave_Open_priv(&raveOpenSettings);
        if (!pidChannel->scramblingCheck.rave) return BERR_TRACE(NEXUS_UNKNOWN);

        NEXUS_Rave_GetDefaultSettings_priv(&raveSettings);
        raveSettings.hwPidChannel = pidChannel;
        raveSettings.bandHold = false; /* bandHold doesn't matter for live. but for playback, there's no value in pushing back on playback. so we hardcode to false. */
        rc = NEXUS_Rave_ConfigureAll_priv(pidChannel->scramblingCheck.rave, &raveSettings);
        if (rc) {
            if (!pidChannel->scramblingCheck.settings.raveContext) {
                NEXUS_Rave_Close_priv(pidChannel->scramblingCheck.rave);
            }
            pidChannel->scramblingCheck.rave = NULL;
            return BERR_TRACE(rc);
        }
    
        NEXUS_Rave_Enable_priv(pidChannel->scramblingCheck.rave);
    }

    BDBG_MSG(("Start Scrambling Check for pid=0x%2x ", pidChannel->status.pid ));
    BXPT_Rave_ClearSCRegister(pidChannel->scramblingCheck.rave->raveHandle);

    return 0;
}

NEXUS_Error NEXUS_PidChannel_StartScramblingCheck( NEXUS_PidChannelHandle pidChannel, const NEXUS_PidChannelScramblingSettings *pSettings )
{
   NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pidChannel);
   return NEXUS_P_PidChannel_Master_StartScramblingCheck(pidChannel->hwPidChannel, pSettings);
}

void NEXUS_P_HwPidChannel_StopScramblingCheck( NEXUS_P_HwPidChannel *pidChannel )
{
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);
    if (pidChannel->scramblingCheck.rave) {
        BDBG_MSG(("Stop Scrambling Check for pid=0x%2x ", pidChannel->status.pid ));
        if (!pidChannel->scramblingCheck.settings.raveContext) {
            NEXUS_Rave_Close_priv(pidChannel->scramblingCheck.rave);
        }
        pidChannel->scramblingCheck.rave = NULL;
    }
}

void NEXUS_PidChannel_StopScramblingCheck( NEXUS_PidChannelHandle pidChannel )
{
   NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pidChannel);
   NEXUS_P_HwPidChannel_StopScramblingCheck(pidChannel->hwPidChannel);
}

static NEXUS_Error NEXUS_P_HwPidChannel_GetScramblingStatus( NEXUS_P_HwPidChannel *pidChannel, NEXUS_PidChannelScramblingStatus *pStatus )
{
    unsigned avScramble = 0;
    BXPT_Rave_ScrambleCtrl scrambleCtrl;
    NEXUS_Error rc;

    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (!pidChannel->scramblingCheck.rave) {
        /* must start first */
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    rc = BXPT_Rave_GetScramblingCtrl(pidChannel->scramblingCheck.rave->raveHandle, &scrambleCtrl);
    if (rc) return BERR_TRACE(rc);

    if (pidChannel->scramblingCheck.settings.pusiOnly) {
        if (scrambleCtrl.PusiValid) {
            avScramble = scrambleCtrl.Pusi;
            pStatus->pidExists = true;
        }
        else {
            pStatus->pidExists = false;
        }
    }
    else {
        if (scrambleCtrl.AllValid) {
            avScramble = scrambleCtrl.ScAll;
            pStatus->pidExists = true;
        }
        else {
            pStatus->pidExists = false;
        }
    }

    if (pStatus->pidExists) {
        /* by default, XPT PI programs RAVE HW to have sticky state (aka SC_OR_MODE), so we don't have to remember */
        pStatus->scrambled = avScramble;
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_PidChannel_GetScramblingStatus( NEXUS_PidChannelHandle pidChannel, NEXUS_PidChannelScramblingStatus *pStatus )
{
   NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pidChannel);
   return NEXUS_P_HwPidChannel_GetScramblingStatus(pidChannel->hwPidChannel, pStatus);
}

static void NEXUS_P_HwPidChannel_ClearScramblingStatus( NEXUS_P_HwPidChannel *pidChannel )
{
    NEXUS_OBJECT_ASSERT(NEXUS_P_HwPidChannel, pidChannel);
    if (pidChannel->scramblingCheck.rave) {
        BXPT_Rave_ClearSCRegister(pidChannel->scramblingCheck.rave->raveHandle);
    }
}

void NEXUS_PidChannel_ClearScramblingStatus( NEXUS_PidChannelHandle pidChannel )
{
   NEXUS_OBJECT_ASSERT(NEXUS_PidChannel, pidChannel);
   NEXUS_P_HwPidChannel_ClearScramblingStatus(pidChannel->hwPidChannel);
}
#endif
