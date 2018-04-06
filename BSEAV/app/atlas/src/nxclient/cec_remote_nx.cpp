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
 *****************************************************************************/
#include "nxclient.h"
#include "cec_remote_nx.h"
#include "convert.h"
#include "atlas.h"
#include "atlas_os.h"

#define CALLBACK_CEC_REMOTE  "CallbackCecRemote"

BDBG_MODULE(cecRemote);

extern void nexusDeviceReadyCallback(void *context, int param);
extern void nexusMsgTransmittedCallback(void *context, int param);
extern void nexusMsgReceivedCallback(void *context, int param);
extern void bwinCecRemoteCallback(void *       pObject,const char * strCallback);

eRet CCecRemoteNx::open(CWidgetEngine * pWidgetEngine){

	eRet                 ret       = eRet_Ok;
	NEXUS_Error            nerror = NEXUS_SUCCESS;
	NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputHandle hdmiOutput = NULL;
	NEXUS_HdmiOutputStatus hdmiOutputStatus;
	NEXUS_PlatformConfiguration platformConfig;
    NEXUS_CecSettings cecSettings;
    NEXUS_CecStatus cecStatus;
	unsigned loops=0;
	unsigned connectId = 0;

	NxClient_ConnectSettings settings;
    NxClient_GetDefaultConnectSettings(&settings);
	nerror = NxClient_Connect(&settings, &connectId);
    CHECK_NEXUS_ERROR_GOTO("unable to connect to the nxserver", nerror, ret, done);

	_pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_CEC_REMOTE, bwinCecRemoteCallback);
    }

	/* nxserver does not open CEC, so a client may. */
    NEXUS_Cec_GetDefaultSettings(&cecSettings);
    _hCec = NEXUS_Cec_Open(0, &cecSettings);
    CHECK_PTR_ERROR_GOTO("Unable to open CEC", _hCec, ret, eRet_NotAvailable, error);

	/* nxserver opens HDMI output, so a client can only open as a read-only alias. */
    hdmiOutput = NEXUS_HdmiOutput_Open(0 + NEXUS_ALIAS_ID, NULL);
    rc = NEXUS_Cec_SetHdmiOutput(_hCec, hdmiOutput);
    CHECK_NEXUS_ERROR_GOTO("NEXUS_Cec_SetHdmiOutput failed", ret, rc, error);

	NEXUS_Cec_GetSettings(_hCec, &cecSettings);
	cecSettings.messageReceivedCallback.callback = nexusMsgReceivedCallback;
	cecSettings.messageReceivedCallback.context = this;
	cecSettings.messageTransmittedCallback.callback = nexusMsgTransmittedCallback;
	cecSettings.messageTransmittedCallback.context = this;
	cecSettings.logicalAddressAcquiredCallback.callback = nexusDeviceReadyCallback;
    cecSettings.logicalAddressAcquiredCallback.context = this;
    nerror = NEXUS_Cec_SetSettings(_hCec, &cecSettings);
    CHECK_NEXUS_ERROR_GOTO("NEXUS_Cec_SetSettings failed", ret, nerror, error);

    /* Enable CEC core */
    NEXUS_Cec_GetSettings(_hCec, &cecSettings);
    cecSettings.enabled = true;
    nerror = NEXUS_Cec_SetSettings(_hCec, &cecSettings);
    CHECK_NEXUS_ERROR_GOTO("NEXUS_Cec_SetSettings failed", ret, nerror, error);

    BDBG_MSG(("*************************"));
    BDBG_MSG(("Wait for logical address before starting test..."));
    BDBG_MSG(("*************************"));
    for (loops = 0; loops < 1; loops++) {
        if (_deviceReady)
            break;
			BKNI_Sleep(1000);
	}
	 if (cecStatus.logicalAddress == 0xFF)
    {
        BDBG_ERR(("No CEC capable device found on HDMI output."));
		ret = eRet_NotAvailable;
        goto error;
    }
goto done;
error:
	 BDBG_WRN(("Errors while opening/setting the Cec."));
	 if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_CEC_REMOTE);
    }
done:
	return(ret);
}