/***************************************************************************
 *     (c)2007-2010 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#include "nexus_transport_module.h"
#include "nexus_playpump_impl.h"
#if BXPT_HAS_TSMUX

void 
NEXUS_TsMux_GetDefaultSettings_priv(NEXUS_TsMuxSettings  *pSettings)
{
    NEXUS_ASSERT_MODULE();
    BXPT_TsMux_GetDefaultSettings(pSettings);
    return;
}

NEXUS_TsMuxHandle 
NEXUS_TsMux_Create_priv(void)
{
    NEXUS_TsMuxHandle tsMux;
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();

    rc = BXPT_TsMux_Create(pTransport->xpt, &tsMux);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);return NULL;}
    return tsMux;
}

void 
NEXUS_TsMux_Destroy_priv(NEXUS_TsMuxHandle tsMux)
{
    BXPT_TsMux_Destroy(tsMux);
}

NEXUS_Error 
NEXUS_TsMux_AddPlaypump_priv(NEXUS_TsMuxHandle tsMux, NEXUS_PlaypumpHandle playpump)
{
    BERR_Code rc;
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(playpump, NEXUS_Playpump);
    rc = BXPT_TsMux_AddPlayback(tsMux, playpump->xpt_play);
    return BERR_TRACE(rc);
}

void 
NEXUS_TsMux_RemovePlaypump_priv(NEXUS_TsMuxHandle tsMux, NEXUS_PlaypumpHandle playpump)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(playpump, NEXUS_Playpump);
    BXPT_TsMux_RemovePlayback(tsMux, playpump->xpt_play);
    return ;
}

void 
NEXUS_TsMux_GetSettings_priv(NEXUS_TsMuxHandle tsMux, NEXUS_TsMuxSettings *pSettings)
{
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSettings);
    BXPT_TsMux_GetSettings(tsMux, pSettings);
    return;
}

NEXUS_Error 
NEXUS_TsMux_SetSettings_priv(NEXUS_TsMuxHandle tsMux, NEXUS_TsMuxSettings *pSettings, NEXUS_StcChannelHandle stcChannel)
{
    BERR_Code rc;
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(stcChannel);
    BDBG_OBJECT_ASSERT(stcChannel, NEXUS_StcChannel);
    pSettings->hPcrOffset = stcChannel->pcrOffset;
    rc = BXPT_TsMux_SetSettings(tsMux, pSettings);
    return BERR_TRACE(rc);
}

void 
NEXUS_TsMux_GetStatus_priv(NEXUS_TsMuxHandle tsMux, NEXUS_TsMuxStatus *pStatus)
{
    NEXUS_ASSERT_MODULE();
    BXPT_TsMux_GetStatus(tsMux, pStatus);
    return;
}


#endif /* BXPT_HAS_TSMUX */

