/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
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
 *                      HdmiOutput: Specific interfaces for an HDMI/DVI output.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/

#include "nexus_hdmi_output_module.h"
#include "priv/nexus_hdmi_output_mhl_priv.h"
#include "priv/nexus_hdmi_output_priv.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_core_video.h"
#include "priv/nexus_core_audio.h"
#include "priv/nexus_core.h"
#include "bhdm.h"

#if BCHP_MT_CBUS_REG_START
#include "bhdm_mhl.h"
#define NEXUS_P_MHL_SUPPORT
#endif

BDBG_MODULE(nexus_hdmi_output_mhl);

#ifdef NEXUS_P_MHL_SUPPORT
NEXUS_Error NEXUS_HdmiOutput_P_OpenMhl(NEXUS_HdmiOutput *pOutput)
{
	NEXUS_Error errCode = NEXUS_SUCCESS;
    BKNI_EventHandle mhlStandbyEvent;

	pOutput->mhlStandbyEventCallback = NULL;
    pOutput->mhlStandbyCallback = NEXUS_TaskCallback_Create(pOutput, NULL);

    /* Register for Mhl Standby Event */
    errCode = BHDM_GetEventHandle(pOutput->hdmHandle, BHDM_EventMhlStandby, &mhlStandbyEvent);
    if (errCode) goto err_event;

    if (mhlStandbyEvent)
    {
        pOutput->mhlStandbyEventCallback =
            NEXUS_RegisterEvent(mhlStandbyEvent, NEXUS_HdmiOutput_P_MhlStandbyCallback, pOutput);

        if ( NULL == pOutput->mhlStandbyEventCallback )
            goto err_event;

        /* Install MHL standby callback */
        errCode = BHDM_MHL_InstallStandbyCallback(pOutput->hdmHandle,
                                                  NEXUS_HdmiOutput_P_MhlStandby_isr, pOutput, 0);
    }

	if (errCode == NEXUS_SUCCESS) goto done;

err_event:
    if (mhlStandbyEvent && pOutput->mhlStandbyEventCallback != NULL)
	{
            NEXUS_UnregisterEvent(pOutput->mhlStandbyEventCallback);
            pOutput->mhlStandbyEventCallback = NULL;
    }

    if (pOutput->mhlStandbyCallback != NULL)
	{
        NEXUS_TaskCallback_Destroy(pOutput->mhlStandbyCallback);
        pOutput->mhlStandbyCallback = NULL;
    }

done:
	return errCode;
}

void NEXUS_HdmiOutput_P_CloseMhl(NEXUS_HdmiOutput *pOutput)
{
    if (pOutput->mhlStandbyEventCallback)
    {
        NEXUS_UnregisterEvent(pOutput->mhlStandbyEventCallback);
        pOutput->mhlStandbyEventCallback = NULL;
    }

    if (pOutput->mhlStandbyCallback != NULL)
	{
        NEXUS_TaskCallback_Destroy(pOutput->mhlStandbyCallback);
        pOutput->mhlStandbyCallback = NULL;
    }
}

void NEXUS_HdmiOutput_P_MhlStandbyCallback(void *pContext)
{
    NEXUS_HdmiOutputHandle output = (NEXUS_HdmiOutputHandle)pContext;

    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    BDBG_MSG(("NEXUS MHL standby callback"));

    NEXUS_TaskCallback_Fire(output->mhlStandbyCallback) ;
}

void NEXUS_HdmiOutput_P_MhlStandby_isr(void *context, int param, void *data)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    BSTD_UNUSED(data);
}
#endif /* NEXUS_P_MHL_SUPPORT */
