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
#include "cec_remote.h"
#include "convert.h"
#include "atlas.h"
#include "atlas_os.h"

#define CALLBACK_CEC_REMOTE  "CallbackCecRemote"

BDBG_MODULE(cecRemote);

void nexusDeviceReadyCallback(void *context, int param)
{
	CCecRemote * cecRemote = (CCecRemote *)context;
	NEXUS_CecStatus status;
    BSTD_UNUSED(param);
	BDBG_ASSERT(NULL != cecRemote);

    NEXUS_Cec_GetStatus(cecRemote->_hCec, &status);

    BDBG_MSG(("BCM%d Logical Address <%d> Acquired",
        BCHP_CHIP,
        status.logicalAddress));

    BDBG_MSG(("BCM%d Physical Address: %X.%X.%X.%X",
        BCHP_CHIP,
        (status.physicalAddress[0] & 0xF0) >> 4,
        (status.physicalAddress[0] & 0x0F),
        (status.physicalAddress[1] & 0xF0) >> 4,
        (status.physicalAddress[1] & 0x0F)));

    if ((status.physicalAddress[0] != 0xFF) && (status.physicalAddress[1] != 0xFF))
    {
        BDBG_MSG(("CEC Device Ready!"));
        cecRemote->_deviceReady = true;
    }
}

void nexusMsgTransmittedCallback(void *context, int param)
{
	CCecRemote * cecRemote = (CCecRemote *)context;
    NEXUS_CecStatus status;
	NEXUS_CecReceivedMessage receivedMessage;
	NEXUS_CecMessage message;
	char requestString[10]={0};

	BDBG_ASSERT(NULL != cecRemote);
	BSTD_UNUSED(param);
	if(cecRemote->_hCec != NULL)
	{
		NEXUS_Cec_GetStatus(cecRemote->_hCec, &status);
		BDBG_MSG(("Msg Xmit Status for Phys/Logical Addrs: %X.%X.%X.%X / %d",
        (status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
        (status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
        status.logicalAddress));
	    BDBG_MSG(("Xmit Msg Acknowledged: %s",
        status.transmitMessageAcknowledged ? "Yes" : "No"));
		BDBG_MSG(("Xmit Msg Pending: %s",
		status.messageTransmitPending ? "Yes" : "No"));
		strcpy(requestString,"ir");
		sscanf(requestString, "%s", &message.buffer[0]);
		message.length = 3;
		message.destinationAddr = receivedMessage.data.initiatorAddr;
		NEXUS_Cec_TransmitMessage( cecRemote->_hCec, &message );
		if(cecRemote->_counter == 0 ){ BDBG_WRN(("Atlas ir command string requested")); cecRemote->_counter++;}
	}
}

void nexusMsgReceivedCallback(void *context, int param)
{
	CCecRemote *  cecRemote = (CCecRemote *)context;
    NEXUS_CecStatus status;
	NEXUS_CecReceivedMessage receivedMessage;
	NEXUS_CecMessage message;
    NEXUS_Error rc = NEXUS_SUCCESS;
	eRet            ret           = eRet_Ok;
	uint8_t i=0, j=0;
	int berror=0;
	int index=0;
    char msgBuffer[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)]={0};
	char ir_string[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)]={0};
	char requestString[10]={0};
	eKey           key          = eKey_Invalid;

	BDBG_ASSERT(NULL != cecRemote);
	CWidgetEngine * pWidgetEngine = cecRemote->getWidgetEngine();

    BSTD_UNUSED(param);
	NEXUS_Cec_GetStatus(cecRemote->_hCec, &status);

    BDBG_MSG(("Message Received: %s", status.messageReceived ? "Yes" : "No"));cecRemote->_messageReceived = status.messageReceived;
    rc = NEXUS_Cec_ReceiveMessage(cecRemote->_hCec, &receivedMessage);
    CHECK_NEXUS_ERROR_GOTO("Failed to receive the cec message", ret, rc, error);
	if(cecRemote->_counter == 0)
	{
		cecRemote->_counter ++;
	}
	else
	{
		for (i = 0, j = 0; i < receivedMessage.data.length && j<(sizeof(msgBuffer)-1); i++)
		{
			j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%c",receivedMessage.data.buffer[i]);
		}
		BDBG_WRN(("cec ir command Received is: %s", msgBuffer));
		cecRemote->_cecKeyStr = msgBuffer;

		if ((cecRemote->_cecKeyStr.findRev(".lua")) != -1)
		{
			BDBG_WRN(("cec command is received for lua script run %s",cecRemote->_cecKeyStr.s()));
			cecRemote->_cecLuaScriptName = cecRemote->_cecKeyStr;
			cecRemote->_cecCommandMode = "luascript";
			if (NULL != pWidgetEngine)
			{
				pWidgetEngine->syncCallback(cecRemote, CALLBACK_CEC_REMOTE);
			}
		}
		else if (((cecRemote->_cecKeyStr.findRev("{")) != -1) || (cecRemote->_JsonReceived))
		{
			cecRemote->_JsonReceived = true;
			cecRemote->_cecTempJsonStr = cecRemote->_cecKeyStr;
			cecRemote->_cecJsonString = cecRemote->_cecJsonString + cecRemote->_cecTempJsonStr ;
			if ((cecRemote->_cecKeyStr.findRev("}")) != -1)
			{
				BDBG_WRN(("cec command received in json format : %s",cecRemote->_cecJsonString.s()));
				if((cecRemote->_cecJsonString.findRev("display_format:")) != -1)
				{
					index = cecRemote->_cecJsonString.findRev("display_format:");
					cecRemote->_cecCommandMode = "display_format_change";
					cecRemote->_displayFormatStr = cecRemote->_cecJsonString.mid(strlen("display_format:")+1,strlen(cecRemote->_cecJsonString.s()));
					index = cecRemote->_displayFormatStr.findRev("}");
					cecRemote->_displayFormatStr = cecRemote->_displayFormatStr.left(index);
					BDBG_WRN(("Display format change requested to :%s",cecRemote->_displayFormatStr.s()));
				}
				else if((cecRemote->_cecJsonString.findRev("play:")) != -1)
				{
					index = cecRemote->_cecJsonString.findRev("play:");
					cecRemote->_cecCommandMode = "playback_start";
					cecRemote->_playbackStreamName = cecRemote->_cecJsonString.mid(strlen("play:")+1,strlen(cecRemote->_cecJsonString.s()));
					index = cecRemote->_playbackStreamName.findRev("}");
					cecRemote->_playbackStreamName = cecRemote->_playbackStreamName.left(index);
					BDBG_WRN(("Playback of %s is requested",cecRemote->_playbackStreamName.s()));
				}
				cecRemote->_cecJsonString="";
				cecRemote->_JsonReceived = false;
				if (NULL != pWidgetEngine)
				{
					pWidgetEngine->syncCallback(cecRemote, CALLBACK_CEC_REMOTE);
				}
			}
		}
		else
		{
			key = stringToKey(msgBuffer);
			if ((eKey_Max == key) || (eKey_Invalid == key))
			{
				BDBG_ERR(("Invalid cec ir key string"));
				ret = eRet_InvalidParameter;
			}
			else
			{
				cecRemote->pRemoteEvent->setCode(key);
				cecRemote->_cecCommandMode = "remotekey";
				if (NULL != pWidgetEngine)
				{
					berror = bwidget_enter_key(pWidgetEngine->getWidgetEngine(), eKey2bwidgets[cecRemote->pRemoteEvent->getCode()], cecRemote->pRemoteEvent->isRepeat() ? false : true);
					pWidgetEngine->syncCallback(cecRemote, CALLBACK_CEC_REMOTE);
				}
			}
		}
		cecRemote->_counter++;
	}
error:
	return;
}
eNotification CCecRemote::cecCmdToNotification(MString cmdString)
{
	eNotification notification = eNotify_Invalid;
	notification = cecCommandStringToNotification(cmdString.lower());
	return notification;
}
/*
 *  Callback from the bwidgets main loop - io trigger is complete and we can
 *  safely notify the observer of the CEC message.
 */
void bwinCecRemoteCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CCecRemote * cecRemote = (CCecRemote *)pObject;
	BDBG_ASSERT(NULL != cecRemote);
    BSTD_UNUSED(strCallback);
	cecRemote->handleCallback();
} /* bwinCecRemoteCallback */

void CCecRemote::handleCallback()
{
	eRet        ret     = eRet_Ok;
	eNotification notification = eNotify_Invalid;
	notification = cecCmdToNotification(_cecCommandMode);
	uint32_t nFormatType = NEXUS_VideoFormat_eMax;
	NEXUS_VideoFormat * pVideoFormat = NULL;
	CPlaybackData * pPlaybackData = NULL;

	switch(notification)
	{
		case eNotify_RunScript:
			ret = this->notifyObservers(eNotify_RunScript, &_cecLuaScriptName);
			CHECK_ERROR_GOTO("unable to notify observers - RunScript", ret, errorCec);
		break;
		case eNotify_KeyClick:
			BDBG_WRN(("Remote event code:%#x repeat:%d", pRemoteEvent->getCode(), pRemoteEvent->isRepeat()));
			ret = this->notifyObservers(eNotify_KeyClick, pRemoteEvent);
			CHECK_ERROR_GOTO("unable to notify observers - key click", ret, errorCec);
			if ((false == pRemoteEvent->isRepeat()))
			{
				ret = this->notifyObservers(eNotify_KeyDown, pRemoteEvent);
				CHECK_ERROR_GOTO("unable to notify observers - for keyClick", ret, errorCec);
			}
		break;
		case eNotify_SetVideoFormat:
			nFormatType = stringToVideoFormat(_displayFormatStr.s());
			if ((NEXUS_VideoFormat_eMax <= nFormatType) || (NEXUS_VideoFormat_eUnknown == nFormatType))
			{
				BDBG_ERR(("Invalid video format type"));
				ret = eRet_InvalidParameter;
			}
			pVideoFormat = new NEXUS_VideoFormat;
			*pVideoFormat = (NEXUS_VideoFormat)nFormatType;
			ret = this->notifyObservers(eNotify_SetVideoFormat, pVideoFormat);
			CHECK_ERROR_GOTO("unable to notify observers - for setVideoFormat", ret, errorCec);
		break;
		case eNotify_PlaybackStart:
			pPlaybackData = new CPlaybackData();
			if (NULL != _playbackStreamName)
			{
				pPlaybackData->_strFileName = _playbackStreamName;
				pPlaybackData->_strIndexName = pPlaybackData->_strFileName;
				ret = this->notifyObservers(eNotify_PlaybackStart, pPlaybackData);
				CHECK_ERROR_GOTO("unable to notify observers - for PlaybackStart", ret, errorCec);
			}
		break;
		default:
			BDBG_WRN(("Unknown notification event found"));
		break;
	}
goto done;
errorCec:
	if(pVideoFormat != NULL){DEL(pVideoFormat);}
	if(pPlaybackData != NULL){DEL(pPlaybackData);}
done:
	notification = eNotify_Invalid;
} /* handleCallback */

eRet CCecRemote::open(CWidgetEngine * pWidgetEngine){

	eRet                 ret       = eRet_Ok;
	NEXUS_Error            nerror = NEXUS_SUCCESS;
	NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputHandle hdmiOutput = NULL;
	NEXUS_HdmiOutputStatus hdmiOutputStatus;
	NEXUS_PlatformConfiguration platformConfig;
    NEXUS_CecSettings cecSettings;
    NEXUS_CecStatus cecStatus;
	unsigned loops=0;

	_pWidgetEngine = pWidgetEngine;
    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->addCallback(this, CALLBACK_CEC_REMOTE, bwinCecRemoteCallback);
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);
    hdmiOutput = platformConfig.outputs.hdmi[0];
    _hCec = platformConfig.outputs.cec[0];
    CHECK_PTR_ERROR_GOTO("Unable to open CEC", _hCec, ret, eRet_NotAvailable, error);
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

void CCecRemote::close(){

	if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_CEC_REMOTE);
    }
	DEL(pRemoteEvent);
}
/*
 * Constructor
 * widgetEngine - the bwidget engine that will be synchronized with when the cec ir code is received.
 *
 */
CCecRemote::CCecRemote() :
    CMvcModel("CCecRemote"),
	_hCec(NULL),
	_pWidgetEngine(NULL),
	_deviceReady(false),
	_messageReceived(false),
	_JsonReceived(false),
	_counter(0)
{
	pRemoteEvent = new CRemoteEvent;
}

CCecRemote::~CCecRemote()
{
 close();
}