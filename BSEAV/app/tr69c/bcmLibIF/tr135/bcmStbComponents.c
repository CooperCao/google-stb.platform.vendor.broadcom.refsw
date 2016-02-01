 /******************************************************************************
  *    (c)2010-2013 Broadcom Corporation
  *
  * This program is the proprietary software of Broadcom Corporation and/or its licensors,
  * and may only be used, duplicated, modified or distributed pursuant to the terms and
  * conditions of a separate, written license agreement executed between you and Broadcom
  * (an "Authorized License").	Except as set forth in an Authorized License, Broadcom grants
  * no license (express or implied), right to use, or waiver of any kind with respect to the
  * Software, and Broadcom expressly reserves all rights in and to the Software and all
  * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
  * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
  * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  *
  * Except as expressly set forth in the Authorized License,
  *
  * 1.	   This program, including its structure, sequence and organization, constitutes the valuable trade
  * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
  * and to use this information only in connection with your use of Broadcom integrated circuit products.
  *
  * 2.	   TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
  * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
  * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
  * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
  * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
  * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
  * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
  * USE OR PERFORMANCE OF THE SOFTWARE.
  *
  * 3.	   TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
  * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
  * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
  * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
  * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
  * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
  * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
  * ANY LIMITED REMEDY.
  *
  * $brcm_Workfile: $
  * $brcm_Revision: $
  * $brcm_Date: $
  *
  *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include "../main/utils.h"
#include "../SOAPParser/CPEframework.h"

#include "../main/types.h"
#include "syscall.h"
#include "bcmTR135Objs.h"
#include "tr69clib_priv.h"
#include "nexus_surface_client.h"


extern b_tr69c_server_t tr69c_server;

/************************************/
/* STBService.Components.DRM Object */
/************************************/
TRX_STATUS setCompDRMEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompDRMEnable(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getCompDRMStatus(char **value)
{
	*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS setCompDRMAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompDRMAlias(char **value)
{
	*value = strdup("DRM.Alias");
	return TRX_OK;
}

TRX_STATUS getCompDRMName(char **value)
{
	*value = strdup("Broadcom TR69 CA");
	return TRX_OK;
}

TRX_STATUS getCompDRMSmartCardReader(char **value)
{
	*value = strdup("");
	return TRX_OK;
}

/************************************/
/* STBService.Components.CA Object  */
/************************************/
TRX_STATUS setCompCAEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompCAEnable(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getCompCAStatus(char **value)
{
	*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS setCompCAAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompCAAlias(char **value)
{
	*value = strdup("CA.Alias");
	return TRX_OK;
}

TRX_STATUS getCompCAName(char **value)
{
	*value = strdup("Broadcom TR69 CA");
	return TRX_OK;
}

TRX_STATUS getCompCASmartCardReader(char **value)
{
	*value = strdup("");
	return TRX_OK;
}

/**************************************/
/* STBService.Components.HDMI.DisplayDevice Object */
/**************************************/
TRX_STATUS getCompHDMIDisplayDeviceStatus(char **value)
{
	NEXUS_Error rc;
    NEXUS_InterfaceName interfaceName;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_HdmiOutputHandle handle;
	NEXUS_HdmiOutputStatus status;
    unsigned num;
	char *module = "NEXUS_HdmiOutputHandle";

    strcpy(interfaceName.name, module);
    rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
		*value = strdup("Error");
		return TRX_OK;
    }
	handle = (NEXUS_HdmiOutputHandle)objects[0].object;
    rc = NEXUS_HdmiOutput_GetStatus(handle, &status);

	if (status.rxPowered)
		*value = strdup("Present");
	else
		*value = strdup("None");
	return TRX_OK;
}

TRX_STATUS getCompHDMIDisplayDeviceName(char **value)
{
	*value = strdup("DisplayDeviceName");
	return TRX_OK;
}

TRX_STATUS getCompHDMIDisplayDeviceEEDID(char **value)
{
	*value = strdup("1234567890ABCDEF");
	return TRX_OK;
}

TRX_STATUS getCompHDMIDisplayDeviceSupportedResolutions(char **value)
{
	*value = strdup("1920x1080i/60Hz,1920x1080p/30Hz:");
	return TRX_OK;
}

TRX_STATUS getCompHDMIDisplayDevicePreferredResolution(char **value)
{
	*value = strdup("1920x1080p/30Hz:");
	return TRX_OK;
}

TRX_STATUS getCompHDMIDisplayDeviceVideoLatency(char **value)
{
	*value = strdup("50");
	return TRX_OK;
}
TRX_STATUS getCompHDMIDisplayDeviceCECSupport(char **value)
{
	NEXUS_Error rc;
	NEXUS_InterfaceName interfaceName;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_CecHandle handle;
    NEXUS_CecStatus status;
    unsigned num;
	char *module = "NEXUS_Cec";

    strcpy(interfaceName.name, module);
    rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc)
    {
        printf("no handles for '%s' found\n", module);
		*value = strdup("Error");
		return TRX_OK;
    }
    handle = (NEXUS_CecHandle)objects[0].object;
    rc = NEXUS_Cec_GetStatus(handle, &status);

	if (status.ready)
		*value = strdup("1");
	else
		*value = strdup("0");
	return TRX_OK;
}

TRX_STATUS getCompHDMIDisplayDeviceAutoLipSyncSupport(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getCompHDMIDisplayDeviceHDMI3DPresent(char **value)
{
	NEXUS_Error rc;
    NEXUS_InterfaceName interfaceName;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_SurfaceClientHandle handle;
	NEXUS_SurfaceClientStatus status;
    unsigned num;
	char *module = "NEXUS_SurfaceClientHandle";

    strcpy(interfaceName.name, module);
    rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
		*value = strdup("Error");
		return TRX_OK;
    }
    handle = (NEXUS_SurfaceClientHandle)objects[0].object;

    rc = NEXUS_SurfaceClient_GetStatus(handle, &status);

	if (status.display.enabled3d)
		*value = strdup("1");
	else
		*value = strdup("0");
	return TRX_OK;
}

/**************************************/
/* STBService.Components.HDMI Object */
/**************************************/
TRX_STATUS setCompHDMIEnable(char *value)
{
	NxClient_DisplaySettings displaySettings;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	NxClient_GetDisplaySettings(&displaySettings);

	if (!strcmp(value, "1"))
		displaySettings.hdmiPreferences.enabled = true;
	else
		displaySettings.hdmiPreferences.enabled = false;
	NxClient_SetDisplaySettings(&displaySettings);
	return TRX_OK;
}

TRX_STATUS getCompHDMIEnable(char **value)
{
	NxClient_DisplaySettings displaySettings;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	NxClient_GetDisplaySettings(&displaySettings);

	if (displaySettings.hdmiPreferences.enabled){
		*value = strdup("1");
	}
	else
		*value = strdup(("0"));
	return TRX_OK;
}

TRX_STATUS getCompHDMIStatus(char **value)
{
	NxClient_DisplaySettings displaySettings;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	NxClient_GetDisplaySettings(&displaySettings);
	if (!displaySettings.hdmiPreferences.enabled)
		*value = strdup("Disabled");
	else
		*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS setCompHDMIAlias(char *value)
{
	decoder *hdmi;
	uint8_t num_entries = 0;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	getAudioVideoDecoder(&hdmi, &num_entries, "hdmi");
	setAudioVideoDecoderAlias(&hdmi, "hdmi", value);
	return TRX_OK;
}

TRX_STATUS getCompHDMIAlias(char **value)
{
	decoder *hdmi;
	uint8_t num_entries = 0;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	getAudioVideoDecoder(&hdmi, &num_entries, "hdmi");
	*value = strdup(hdmi->alias);
	return TRX_OK;
}

TRX_STATUS getCompHDMIName(char **value)
{
	decoder *hdmi;
	uint8_t num_entries = 0;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	getAudioVideoDecoder(&hdmi, &num_entries, "hdmi");
	*value = strdup(hdmi->name);
	return TRX_OK;
}

TRX_STATUS setCompHDMIResolutionMode(char *value)
{
	NxClient_DisplaySettings displaySettings;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

    NxClient_GetDisplaySettings(&displaySettings);
	if (!strcmp(value, "Auto"))
		displaySettings.hdmiPreferences.followPreferredFormat = true;
	else
		displaySettings.hdmiPreferences.followPreferredFormat = false;
	NxClient_SetDisplaySettings(&displaySettings);
	return TRX_OK;
}

TRX_STATUS getCompHDMIResolutionMode(char **value)
{
	NxClient_DisplaySettings displaySettings;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	NxClient_GetDisplaySettings(&displaySettings);
	*value = strdup(displaySettings.hdmiPreferences.followPreferredFormat?"Auto":"Manual");
	return TRX_OK;
}

TRX_STATUS setCompHDMIResolutionValue(char *value)
{
	NxClient_DisplaySettings displaySettings;
	NEXUS_Error rc = 0;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

    NxClient_GetDisplaySettings(&displaySettings);
	if (!displaySettings.hdmiPreferences.followPreferredFormat){
		displaySettings.format = stringToEnum(hdmiVideoFormat,value); /*change to accordingly with value, e.g NEXUS_VideoFormat_e1080i*/
		rc = NxClient_SetDisplaySettings(&displaySettings);
		if (rc!= NEXUS_SUCCESS)
			return TRX_ERR;
		else
			return TRX_OK;
	}
	return TRX_OK;
}

TRX_STATUS getCompHDMIResolutionValue(char **value)
{
	NxClient_DisplaySettings displaySettings;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	NxClient_GetDisplaySettings(&displaySettings);
	if(enumToString(hdmiVideoFormat, displaySettings.format))
		*value = strdup(enumToString(hdmiVideoFormat, displaySettings.format));
	else
		*value = strdup("");
	return TRX_OK;
}

/**************************************/
/* STBService.Components.SCART Object */
/**************************************/
TRX_STATUS setCompSCARTEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompSCARTEnable(char **value)
{
	*value = strdup(("0"));
	return TRX_OK;
}

TRX_STATUS getCompSCARTStatus(char **value)
{
	*value = strdup("Disabled");
	return TRX_OK;
}

TRX_STATUS setCompSCARTAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompSCARTAlias(char **value)
{
	*value = strdup("CPE-SCART");
	return TRX_OK;
}

TRX_STATUS getCompSCARTName(char **value)
{
	*value = strdup("SCART");
	return TRX_OK;
}

TRX_STATUS setCompSCARTPresence(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompSCARTPresence(char **value)
{
	*value = strdup("0");
	return TRX_OK;
}

/********************************************/
/* STBService.Components.VideoOutput Object */
/********************************************/
TRX_STATUS setCompVideoOutputEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompVideoOutputEnable(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getCompVideoOutputStatus(char **value)
{
	*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS setCompVideoOutputAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompVideoOutputAlias(char **value)
{
	*value = strdup("VideoOutput.Alias");
	return TRX_OK;
}

TRX_STATUS setCompVideoOutputColorbarEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompVideoOutputColorbarEnable(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getCompVideoOutputName(char **value)
{
	*value = strdup("Broadcom TR69 Video Output");
	return TRX_OK;
}

TRX_STATUS getCompVideoOutputCompositeVideoStandard(char **value)
{
	*value = strdup(("NTSC-M"));
	return TRX_OK;
}

TRX_STATUS setCompVideoOutputVideoFormat(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompVideoOutputVideoFormat(char **value)
{
	*value = strdup("HDMI");
	return TRX_OK;
}

TRX_STATUS getCompVideoOutputAspectRatioBehaviour(char **value)
{
	*value = strdup("Letterbox");
	return TRX_OK;
}

TRX_STATUS setCompVideoOutputDisplayFormat(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompVideoOutputDisplayFormat(char **value)
{
	*value = strdup(("1920x1080p/30Hz"));
	return TRX_OK;
}

TRX_STATUS getCompVideoOutputMacrovision(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getCompVideoOutputHDCP(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getCompVideoOutputSCARTs(char **value)
{
	*value = strdup("");
	return TRX_OK;
}

/********************************************/
/* STBService.Components.SPDIF Object */
/********************************************/
TRX_STATUS setCompSPDIFEnable(char *value)
{
	#if NEXUS_NUM_SPDIF_OUTPUTS
	NxClient_AudioSettings settings;
	NEXUS_Error rc;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	NxClient_GetAudioSettings(&settings);

	if (value == 0)
	{
		settings.spdif.muted = true;
		rc = NxClient_SetAudioSettings(&settings);
		if(!rc)
			return TRX_OK;
		return TRX_ERR;
	}
	return TRX_OK;

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getCompSPDIFEnable(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getCompSPDIFStatus(char **value)
{
	#if NEXUS_NUM_SPDIF_OUTPUTS
	NxClient_AudioSettings settings;
	NxClient_GetAudioSettings(&settings);

	if (settings.spdif.muted == false)
		*value = strdup("Enabled");
	else
		*value = strdup("Disabled");
	return TRX_OK;

	#endif
	return TRX_REQUEST_DENIED;
}

TRX_STATUS setCompSPDIFAlias(char *value)
{
	b_spdif_t *spdif;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	getSpdif(&spdif);
	strcpy(spdif->alias, value);
	setSpdif(spdif);
	return TRX_OK;
}

TRX_STATUS getCompSPDIFAlias(char **value)
{
	*value = strdup("CPE-SPDIF");
	return TRX_OK;
}

TRX_STATUS getCompSPDIFName(char **value)
{
	*value = strdup("Broadcom SPDIF");
	return TRX_OK;
}

TRX_STATUS setCompSPDIFForcePCM(char *value)
{
	#if NEXUS_NUM_SPDIF_OUTPUTS
	NxClient_AudioSettings settings;
	NEXUS_Error rc;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	NxClient_GetAudioSettings(&settings);

	if (!strcmp(value, "true"))
	{
		settings.spdif.outputMode = NxClient_AudioOutputMode_ePcm;
		rc = NxClient_SetAudioSettings(&settings);
		if(!rc)
			return TRX_OK;
		return TRX_ERR;
	}
	else
		settings.spdif.outputMode = NxClient_AudioOutputMode_ePassthrough;
		rc = NxClient_SetAudioSettings(&settings);
		if(!rc)
			return TRX_OK;
		return TRX_ERR;
	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getCompSPDIFForcePCM(char **value)
{
	#if NEXUS_NUM_SPDIF_OUTPUTS
	NEXUS_PlatformConfiguration platformConfig;
	int i;

	NEXUS_Platform_GetConfiguration(&platformConfig);

	for (i=0; i<NEXUS_NUM_SPDIF_OUTPUTS; i++)
	{
		if (platformConfig.outputs.spdif[i] != NULL)
		{
			NxClient_AudioStatus audioStatus;
			NxClient_GetAudioStatus(&audioStatus);
			if (audioStatus.spdif.outputMode == NxClient_AudioOutputMode_ePcm ||
				audioStatus.spdif.outputMode == NxClient_AudioOutputMode_eMultichannelPcm)
			{
				*value = strdup("true");
			}
			else
				*value = strdup("false");
			return TRX_OK;
		}
	}
	return TRX_ERR;
	#endif
	return TRX_REQUEST_DENIED;
}

TRX_STATUS getCompSPDIFPassthrough(char **value)
{
	#if NEXUS_NUM_SPDIF_OUTPUTS
	NEXUS_PlatformConfiguration platformConfig;
	int i;

	NEXUS_Platform_GetConfiguration(&platformConfig);

	for (i=0; i<NEXUS_NUM_SPDIF_OUTPUTS; i++)
	{
		if (platformConfig.outputs.spdif[i] != NULL)
		{
			NxClient_AudioStatus audioStatus;
			NxClient_GetAudioStatus(&audioStatus);
			if (audioStatus.spdif.outputMode == NxClient_AudioOutputMode_ePassthrough)
			{
				*value = strdup("true");
			}
			else
				*value = strdup("false");
			return TRX_OK;
		}
	}
	return TRX_ERR;
	#endif
	return TRX_REQUEST_DENIED;
}

TRX_STATUS getCompSPDIFAudioDelay(char **value)
{
	*value = strdup("50");
	return TRX_OK;
}

/********************************************/
/* STBService.Components.AudioOutput Object */
/********************************************/
TRX_STATUS setCompAudioOutputEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompAudioOutputEnable(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getCompAudioOutputStatus(char **value)
{
	*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS setCompAudioOutputAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompAudioOutputAlias(char **value)
{
	*value = strdup("AudioOutput.Alias");
	return TRX_OK;
}

TRX_STATUS getCompAudioOutputName(char **value)
{
	*value = strdup("Broadcom TR69 Audio Output");
	return TRX_OK;
}

TRX_STATUS getCompAudioOutputAudioFormat(char **value)
{
	*value = strdup("DIGITAL-OPTICAL-SP/DIF");
	return TRX_OK;
}

TRX_STATUS setCompAudioOutputAudioLevel(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompAudioOutputAudioLevel(char **value)
{
	*value = strdup("50");
	return TRX_OK;
}

TRX_STATUS setCompAudioOutputCancelMute(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompAudioOutputCancelMute(char **value)
{
	*value = strdup("EnABLE");
	return TRX_OK;
}

TRX_STATUS getCompAudioOutputSCARTs(char **value)
{
	*value = strdup("");
	return TRX_OK;
}

/*********************************************/
/* STBService.Components.VideoDecoder Object */
/*********************************************/
TRX_STATUS setCompVideoDecoderEnable(char *value)
{
	struct b_tr69c_data tr69c_data;
    b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
    uint32 index, rc;
	void *send_data;
	int send_data_len;

	send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client && value)
    {
        tr69c_data.type = b_tr69c_type_set_video_decoder_mute;
		if (value != 0)
			tr69c_data.info.video_decoder_mute = true;
		tr69c_data.info.video_decoder_mute = false;
		b_tr69c_server_set_client(client_registry[index].client, send_data, send_data_len);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompVideoDecoderEnable(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	uint32 index, rc;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
        NEXUS_VideoDecoderStatus status;
        tr69c_data.type = b_tr69c_type_get_video_decoder_status;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
        status.started = info.video_decoder_status.started;
		if (status.started)
			*value = strdup("Enabled");
		*value = strdup("Disabled");
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompVideoDecoderStatus(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
        NEXUS_VideoDecoderStatus status;
        tr69c_data.type = b_tr69c_type_get_video_decoder_status;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
        status.started = info.video_decoder_status.started;
		if (status.started)
			*value = strdup("Enabled");
		*value = strdup("Disabled");
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS setCompVideoDecoderAlias(char *value)
{
	b_tr69c_client_registry_t client_registry;
	b_videoDecoder_t *videoDecoder;
	InstanceDesc *idp;
	int index, rc;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
		getVideoDecoder(&videoDecoder, index);
		snprintf(videoDecoder[index].alias, NEXUS_INTERFACE_NAME_MAX, value);
		setVideoDecoderParams(&videoDecoder[index], index);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompVideoDecoderAlias(char **value)
{
	b_tr69c_client_registry_t client_registry;
	b_videoDecoder_t *videoDecoder;
	InstanceDesc *idp;
	int index, rc;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
		getVideoDecoder(&videoDecoder, index);
		*value = strdup(videoDecoder[index].alias);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompVideoDecoderName(char **value)
{
	b_tr69c_client_registry_t client_registry;
	b_videoDecoder_t *videoDecoder;
	InstanceDesc *idp;
	int index, rc;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
		getVideoDecoder(&videoDecoder, index);
		*value = strdup(videoDecoder[index].name);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompVideoDecoderMPEG2Part2(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, i, rc;
	profileLevel *profileLevel;
	uint8_t numEntries;
	char tmp[256];
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
        tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.video_start_settings.codec == NEXUS_VideoCodec_eMpeg2)
		{
			tr69c_data.type = b_tr69c_type_get_video_decoder_status;
			b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
			getProfileLevel(&profileLevel, &numEntries, MPEG2PART2_PROFILE_LEVEL);
			for (i = 0; i < numEntries; i++)
			{
				if ((profileLevel[i].level == info.video_decoder_status.protocolLevel) && (profileLevel[i].profile == info.video_decoder_status.protocolProfile))
				{
					BKNI_Snprintf(tmp, 256, "%s%d", ".Capabilities.VideoDecoder.MPEG2Part2.ProfileLevel.", i);
					break;
				}

			}
			*value = strdup(tmp);
			return TRX_OK;
		}
		else
		{
			*value = strdup("");
			return TRX_OK;
		}
    }
	return TRX_ERR;
}

TRX_STATUS getCompVideoDecoderMPEG4Part2(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, i, rc;
	profileLevel *profileLevel;
	uint8_t numEntries;
	char tmp[256];
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
        tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.video_start_settings.codec == NEXUS_VideoCodec_eMpeg4Part2)
		{
			tr69c_data.type = b_tr69c_type_get_video_decoder_status;
			b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
			getProfileLevel(&profileLevel, &numEntries, MPEG4PART2_PROFILE_LEVEL);
			for (i = 0; i < numEntries; i++)
			{
				if ((profileLevel[i].level == info.video_decoder_status.protocolLevel) && (profileLevel[i].profile == info.video_decoder_status.protocolProfile))
				{
					BKNI_Snprintf(tmp, 256, "%s%d", ".Capabilities.VideoDecoder.MPEG4Part2.ProfileLevel.", i);
					break;
				}

			}
			*value = strdup(tmp);
			return TRX_OK;
		}
		else
		{
			*value = strdup("");
			return TRX_OK;
		}
    }
	return TRX_ERR;
}

TRX_STATUS getCompVideoDecoderMPEG4Part10(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, i, rc;
	profileLevel *profileLevel;
	uint8_t numEntries;
	char tmp[256];
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
        tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.video_start_settings.codec == NEXUS_VideoCodec_eH264)
		{
			tr69c_data.type = b_tr69c_type_get_video_decoder_status;
			b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
			getProfileLevel(&profileLevel, &numEntries, MPEG4PART10_PROFILE_LEVEL);
			for (i = 0; i < numEntries; i++)
			{
				if ((profileLevel[i].level == info.video_decoder_status.protocolLevel) && (profileLevel[i].profile == info.video_decoder_status.protocolProfile))
				{
					BKNI_Snprintf(tmp, 256, "%s%d", ".Capabilities.VideoDecoder.MPEG4Part10.ProfileLevel.", i);
					break;
				}

			}
			*value = strdup(tmp);
			return TRX_OK;
		}
		else
		{
			*value = strdup("");
			return TRX_OK;
		}
    }
	return TRX_ERR;
}

TRX_STATUS getCompVideoDecoderSMPTEVC1(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, i, rc;
	profileLevel *profileLevel;
	uint8_t numEntries;
	char tmp[256];
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
        tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.video_start_settings.codec == NEXUS_VideoCodec_eVc1)
		{
			tr69c_data.type = b_tr69c_type_get_video_decoder_status;
			b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
			getProfileLevel(&profileLevel, &numEntries, SMPTEVC1_PROFILE_LEVEL);
			for (i = 0; i < numEntries; i++)
			{
				if ((profileLevel[i].level == info.video_decoder_status.protocolLevel) && (profileLevel[i].profile == info.video_decoder_status.protocolProfile))
				{
					BKNI_Snprintf(tmp, 256, "%s%d", ".Capabilities.VideoDecoder.SMPTEVC1.ProfileLevel.", i);
					break;
				}
			}
			*value = strdup(tmp);
			return TRX_OK;
		}
		else
		{
			*value = strdup("");
			return TRX_OK;
		}
    }
	return TRX_ERR;
}

TRX_STATUS getCompVideoDecoderContentAspectRatio(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, i, rc;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
        NEXUS_VideoDecoderStatus status;
        tr69c_data.type = b_tr69c_type_get_video_decoder_status;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
        status.aspectRatio = info.video_decoder_status.aspectRatio;
		for (i = 0; videoOutputDisplayFormat[i].name; i++)
		{
			if (status.aspectRatio == (unsigned)videoOutputDisplayFormat[i].value)
			{
				*value = strdup(videoOutputDisplayFormat[i].name);
			}
			else
				*value = strdup("");
		}
		return TRX_OK;
    }
	return TRX_ERR;
}

/*********************************************/
/* STBService.Components.AudioDecoder Object */
/*********************************************/
TRX_STATUS setCompAudioDecoderEnable(char *value)
{
	struct b_tr69c_data tr69c_data;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	uint32 index;
	int rc;
	void *send_data;
	int send_data_len;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
		tr69c_data.type = b_tr69c_type_set_audio_decoder_mute;
		if (value != 0)
			tr69c_data.info.audio_decoder_mute = true;
		else
			tr69c_data.info.audio_decoder_mute = false;
		b_tr69c_server_set_client(client_registry[index].client, send_data, send_data_len);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompAudioDecoderEnable(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	uint32 index;
	int rc;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
        NEXUS_AudioDecoderStatus status;
        tr69c_data.type = b_tr69c_type_get_audio_decoder_status;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
        status.started = info.audio_decoder_status.started;
		if (status.started)
			*value = strdup("Enabled");
		else
			*value = strdup("Disabled");
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompAudioDecoderStatus(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
        NEXUS_AudioDecoderStatus status;
        tr69c_data.type = b_tr69c_type_get_audio_decoder_status;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
        status.started = info.audio_decoder_status.started;
		if (status.started)
			*value = strdup("Enabled");
		else
			*value = strdup("Disabled");
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS setCompAudioDecoderAlias(char *value)
{
	b_audioDecoder_t *audioDecoder;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
		getAudioDecoder(&audioDecoder, index);
		snprintf(audioDecoder[index].alias, NEXUS_INTERFACE_NAME_MAX, value);
		setAudioDecoderParams(&audioDecoder[index], index);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompAudioDecoderAlias(char **value)
{
	b_tr69c_client_registry_t client_registry;
	b_audioDecoder_t *audioDecoder;
	InstanceDesc *idp;
	int index, rc;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
		getAudioDecoder(&audioDecoder, index);
		*value = strdup(audioDecoder[index].alias);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompAudioDecoderName(char **value)
{
	b_tr69c_client_registry_t client_registry;
	b_audioDecoder_t *audioDecoder;
	InstanceDesc *idp;
	int index, rc;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
		getAudioDecoder(&audioDecoder, index);
		*value = strdup(audioDecoder[index].name);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getCompAudioDecoderStandard(char **value)
{
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    if (client_registry[index].client)
    {
		NEXUS_AudioDecoderStatus status;
		tr69c_data.type = b_tr69c_type_get_audio_decoder_status;
		b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
		status.codec = info.audio_decoder_status.codec;
		*value = strdup(enumToString(audioCodecStandard, status.codec));
		return TRX_OK;
    }
	return TRX_ERR;
}

/*********************************************/
/* STBService.Components.PVR.Storage Object  */
/*********************************************/
TRX_STATUS setCompPVRStorageAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompPVRStorageAlias(char **value)
{
	*value = strdup("PVR.Storage.Alias");
	return TRX_OK;
}

TRX_STATUS getCompPVRStorageName(char **value)
{
	*value = strdup("PVR.Storage");
	return TRX_OK;
}

TRX_STATUS getCompPVRStorageReference(char **value)
{
	*value = strdup("");
	return TRX_OK;
}

/*********************************************/
/* STBService.Components.PVR Object          */
/*********************************************/
TRX_STATUS getCompPVRStorageNumberOfEntries(char **value)
{
	uint32 num_entries = 1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

/********************************************************/
/* STBService.Components.FrontEnd.IP.Dejittering Object */
/********************************************************/
TRX_STATUS setCompFEIPDejitteringBufferSize(char *value)
{
	uint32	buf_size;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	buf_size = atoi(value);

	return TRX_OK;
}

TRX_STATUS getCompFEIPDejitteringBufferSize(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.playback_ip_status.maxBufferDuration != 0)
			*value = strdup(itoa(info.playback_ip_status.maxBufferDuration));
		else
			*value = strdup("");
		return TRX_OK;
    }

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS setCompFEIPDejitteringBufferInitialLevel(char *value)
{
	uint32	buf_level;
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	buf_level = atoi(value);

	return TRX_OK;
}

TRX_STATUS getCompFEIPDejitteringBufferInitialLevel(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.playback_ip_status.maxBufferDuration != 0)
			*value = strdup(itoa(info.playback_ip_status.maxBufferDuration/2));
		else
			*value = strdup("");
		return TRX_OK;
    }

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

/********************************************************/
/* STBService.Components.FrontEnd.IP.Outbound Object    */
/********************************************************/
TRX_STATUS getCompFEIPOutboundStatus(char **value)
{
	*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS setCompFEIPOutboundAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPOutboundAlias(char **value)
{
	*value = strdup("IP.Outbound.Alias");
	return TRX_OK;
}

TRX_STATUS getCompFEIPOutboundMultiplexType(char **value)
{
	*value = strdup("MPEG2-PS");
	return TRX_OK;
}

TRX_STATUS getCompFEIPOutboundURI(char **value)
{
	*value = strdup("UPnP AV URN");
	return TRX_OK;
}

/********************************************************/
/* STBService.Components.FrontEnd.IP.ServiceConnect Object     */
/********************************************************/

TRX_STATUS getCompFEIPServiceConnect (char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	char uri[256] = "\0";

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        switch(info.playback_ip_status.sessionInfo.protocol)
        {
			case B_PlaybackIpProtocol_eHttp:
				BKNI_Snprintf(uri, sizeof(uri), "%s%s%u%s", "http://", info.playback_ip_status.sessionInfo.ipAddr,
							info.playback_ip_status.sessionInfo.port, "/");
				break;
			case B_PlaybackIpProtocol_eRtp:
				BKNI_Snprintf(uri, sizeof(uri), "%s%s%u%s", "rtp://", info.playback_ip_status.sessionInfo.ipAddr,
							info.playback_ip_status.sessionInfo.port, "/");
				break;
			case B_PlaybackIpProtocol_eUdp:
				BKNI_Snprintf(uri, sizeof(uri), "%s%s%u%s", "udp://", info.playback_ip_status.sessionInfo.ipAddr,
							info.playback_ip_status.sessionInfo.port, "/");
				break;
			default:
				break;
        }
    }
	*value = strdup(uri);
	return TRX_OK;
	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS setCompFEIPServiceConnect (char *value)
{
	#if PLAYBACK_IP_SUPPORT
	char uri[128];

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	strncpy(uri, value, sizeof(uri));
	return TRX_OK;
	#endif

	return TRX_INVALID_PARAM_TYPE;
}


/********************************************************/
/* STBService.Components.FrontEnd.IP.Inbound Object     */
/********************************************************/
TRX_STATUS getCompFEIPInboundStatus(char **value)
{
	b_IpInbound_t *inbound;

	getIpInbound(&inbound);
	if (inbound->status == disabled)
		*value = strdup("Disabled");
	else if (inbound->status == enabled)
		*value = strdup("Enabled");
	else
		*value = strdup("Error");
	return TRX_OK;
}

TRX_STATUS setCompFEIPInboundAlias(char *value)
{
	b_IpInbound_t *inbound;

	getIpInbound(&inbound);
	strncpy(inbound->alias, value, sizeof(inbound->alias));
	setIpInbound(inbound);
	return TRX_OK;
}

TRX_STATUS getCompFEIPInboundAlias(char **value)
{
	b_IpInbound_t *inbound;

	getIpInbound(&inbound);
	strncpy(inbound->alias, "CPE-IpInbound", sizeof(inbound->alias));
	*value = strdup(inbound->alias);
	return TRX_OK;
}

TRX_STATUS getCompFEIPInboundStreamingControlProtocol(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	char protocol[16] = "\0";

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eRtsp)
		{
			BKNI_Snprintf(protocol, sizeof(protocol), "%s", "RTSP");
			*value = strdup(protocol);
		}
		else
			*value = strdup("");
		return TRX_OK;
    }
	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getCompFEIPInboundStreamingTransportProtocol(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	char protocol[16] = "\0";

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        switch(info.playback_ip_status.sessionInfo.protocol)
        {
			case B_PlaybackIpProtocol_eHttp:
				BKNI_Snprintf(protocol, sizeof(protocol), "%s", "HTTP");
				break;
			case B_PlaybackIpProtocol_eRtp:
				BKNI_Snprintf(protocol, sizeof(protocol), "%s", "RTP");
				break;
			case B_PlaybackIpProtocol_eUdp:
				BKNI_Snprintf(protocol, sizeof(protocol), "%s", "UDP");
				break;
			default:
				break;
        }
    }
	*value = strdup(protocol);
	return TRX_OK;
	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getCompFEIPInboundStreamingTransportControlProtocol(char **value)
{
	*value = strdup("RTCP");
	return TRX_OK;
}

TRX_STATUS getCompFEIPInboundMultiplexType(char **value)
{
	*value = strdup("MPEG2-TS");
	return TRX_OK;
}

TRX_STATUS getCompFEIPInboundDownloadTransportProtocol(char **value)
{
	*value = strdup("HTTP");
	return TRX_OK;
}

TRX_STATUS getCompFEIPInboundSourceAddress(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.playback_ip_status.sessionInfo.ipAddr[0] != '\0')
			*value = strdup(info.playback_ip_status.sessionInfo.ipAddr);
		else
			*value = strdup("");
		return TRX_OK;
    }

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getCompFEIPInboundSourcePort(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.playback_ip_status.sessionInfo.port != 0)
		{
			*value = strdup(itoa(info.playback_ip_status.sessionInfo.port));
		}
		else
			*value = strdup("");
		return TRX_OK;
    }

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getCompFEIPInboundDestinationAddress(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.playback_ip_status.sessionInfo.ipAddr[0] != '\0')
			*value = strdup(info.playback_ip_status.sessionInfo.ipAddr);
		else
			*value = strdup("");
		return TRX_OK;
    }

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getCompFEIPInboundDestinationPort(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	char destPort[8];

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.playback_ip_status.sessionInfo.port != 0)
		{
			BKNI_Snprintf(destPort, sizeof(destPort), "%d", info.playback_ip_status.sessionInfo.port);
			*value = strdup(destPort);
		}
		else
			*value = strdup("");
		return TRX_OK;
    }

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getCompFEIPInboundURI(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	char uri[256];

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        switch(info.playback_ip_status.sessionInfo.protocol)
        {
			case B_PlaybackIpProtocol_eHttp:
				BKNI_Snprintf(uri, sizeof(uri), "%s%s%d%s", "http://", info.playback_ip_status.sessionInfo.ipAddr,
							info.playback_ip_status.sessionInfo.port, "/");
				break;
			case B_PlaybackIpProtocol_eRtp:
				BKNI_Snprintf(uri, sizeof(uri), "%s%s%d%s", "rtp://", info.playback_ip_status.sessionInfo.ipAddr,
							info.playback_ip_status.sessionInfo.port, "/");
				break;
			case B_PlaybackIpProtocol_eUdp:
				BKNI_Snprintf(uri, sizeof(uri), "%s%s%d%s", "udp://", info.playback_ip_status.sessionInfo.ipAddr,
							info.playback_ip_status.sessionInfo.port, "/");
				break;
			default:
				BKNI_Snprintf(uri, sizeof(uri), "%s", "");
				break;
        }
    }
	*value = strdup(uri);
	return TRX_OK;
	#endif

	return TRX_INVALID_PARAM_TYPE;
}

/******************************************************************************/
/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats.QuarterHour Object */
/******************************************************************************/
TRX_STATUS getCompFEIPIGMPClientGroupStatsQuarterHourNumberOfJoins(char **value)
{
	uint32	num_joins = 1;
	*value = strdup(itoa(num_joins));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsQuarterHourNumberOfLeaves(char **value)
{
	uint32	num_leaves = 1;
	*value = strdup(itoa(num_leaves));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsQuarterHourMaxJoinDelay(char **value)
{
	uint32	max_joins = 1;
	*value = strdup(itoa(max_joins));
	return TRX_OK;
}

/******************************************************************************/
/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats.Current Object     */
/******************************************************************************/
TRX_STATUS getCompFEIPIGMPClientGroupStatsCurrentDayNumberOfJoins(char **value)
{
	uint32	num_joins = 1;
	*value = strdup(itoa(num_joins));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsCurrentDayNumberOfLeaves(char **value)
{
	uint32	num_leaves = 1;
	*value = strdup(itoa(num_leaves));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsCurrentDayMaxJoinDelay(char **value)
{
	uint32	max_joins = 1;
	*value = strdup(itoa(max_joins));
	return TRX_OK;
}

/******************************************************************************/
/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats.Total Object       */
/******************************************************************************/
TRX_STATUS getCompFEIPIGMPClientGroupStatsTotalNumberOfJoins(char **value)
{
	uint32	num_joins = 1;
	*value = strdup(itoa(num_joins));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsTotalNumberOfLeaves(char **value)
{
	uint32	num_leaves = 1;
	*value = strdup(itoa(num_leaves));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsTotalMaxJoinDelay(char **value)
{
	uint32	max_joins = 1;
	*value = strdup(itoa(max_joins));
	return TRX_OK;
}

/******************************************************************************/
/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats Object             */
/******************************************************************************/
TRX_STATUS setCompFEIPIGMPClientGroupStatsAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsAlias(char **value)
{
	*value = strdup("IGMP.ClientGroupStats.Alias");
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsAddress(char **value)
{
	*value = strdup("10.10.0.0");
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsTotalStart(char **value)
{
	uint32	total_start = 1;
	*value = strdup(itoa(total_start));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsCurrentDayStart(char **value)
{
	uint32	current_start = 1;
	*value = strdup(itoa(current_start));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupStatsQuartHourStart(char **value)
{
	uint32	quarter_hour_start = 1;
	*value = strdup(itoa(quarter_hour_start));
	return TRX_OK;
}

/******************************************************************************/
/* STBService.Components.FrontEnd.IP.IGMP.ClientGroup Object                  */
/******************************************************************************/
TRX_STATUS setCompFEIPIGMPClientGroupAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupAlias(char **value)
{
	*value = strdup("IGMP.ClientGroup.Alias");
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupAddress(char **value)
{
	*value = strdup("10.10.0.0");
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPClientGroupUpTime(char **value)
{
	uint32	up_time = 1;
	*value = strdup(itoa(up_time));
	return TRX_OK;
}

/*************************************************/
/* STBService.Components.FrontEnd.IP.IGMP Object */
/*************************************************/
TRX_STATUS setCompFEIPIGMPEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPEnable(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPStatus(char **value)
{
	*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPMaximumNumberOfConcurrentGroups(char **value)
{
	uint32	max_con_group = 1;
	*value = strdup(itoa(max_con_group));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPMaximumNumberOfTrackedGroups(char **value)
{
	uint32	max_track_group = 1;
	*value = strdup(itoa(max_track_group));
	return TRX_OK;
}

TRX_STATUS setCompFEIPIGMPLoggingEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPLoggingEnable(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS setCompFEIPIGMPDSCPMark(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPDSCPMark(char **value)
{
	uint32 diff_serv=1;
	*value = strdup(itoa(diff_serv));
	return TRX_OK;
}

TRX_STATUS setCompFEIPIGMPVLANIDMark(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPVLANIDMark(char **value)
{
	int vlan_id=1;
	*value = strdup(itoa(vlan_id));
	return TRX_OK;
}

TRX_STATUS setCompFEIPIGMPEthernetPriorityMark(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMPEthernetPriorityMark(char **value)
{
	int eth_prio_mark=1;

	*value = strdup(itoa(eth_prio_mark));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMClientVersion(char **value)
{
	*value = strdup("v1");
	return TRX_OK;
}

TRX_STATUS setCompFEIPIGMClientRobustness(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMClientRobustness(char **value)
{
	int robustness=1;
	*value = strdup(itoa(robustness));
	return TRX_OK;
}

TRX_STATUS setCompFEIPIGMClientUnsolicitedReportInterval(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMClientUnsolicitedReportInterval(char **value)
{
	int reporting_interval=1;
	*value = strdup(itoa(reporting_interval));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMClientGroupNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompFEIPIGMClientGroupStatsNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

/*************************************************/
/* STBService.Components.FrontEnd.IP Object      */
/*************************************************/
TRX_STATUS getCompFEIPInboundNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompFEIPOutboundNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompFEIPActiveInboundIPStreams(char **value)
{
	uint32 num_inbound_streams=1;
	*value = strdup(itoa(num_inbound_streams));
	return TRX_OK;
}

TRX_STATUS getCompFEIPActiveOutboundIPStreams(char **value)
{
	uint32 num_outbound_streams=1;
	*value = strdup(itoa(num_outbound_streams));
	return TRX_OK;
}

/*************************************************/
/* STBService.Components.FrontEnd Object         */
/*************************************************/
TRX_STATUS setCompFEEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEEnable(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getCompFEStatus(char **value)
{
	*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS setCompFEAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEAlias(char **value)
{
	*value = strdup("Frontend-Alias");
	return TRX_OK;
}

TRX_STATUS getCompFEName(char **value)
{
	*value = strdup("TR69 Component Froentend");
	return TRX_OK;
}

/*************************************************/
/* STBService.Components Object                  */
/*************************************************/
TRX_STATUS getCompFENumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompAudioDecoderNumberOfEntries(char **value)
{
	int num_entries=0;
    b_tr69c_get_client_count(&num_entries);
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompVideoDecoderNumberOfEntries(char **value)
{
	int num_entries=0;
    b_tr69c_get_client_count(&num_entries);
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompAudioOutputNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompVideoOutputNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompSCARTNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompCANNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompDRMNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompHDMINumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS getCompSPDIFNumberOfEntries(char **value)
{
	uint32 num_entries=1;
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}


/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.LogicalChannelConnect object */
TRX_STATUS setCompFEQAMLogicalChannelConnectLogicalChannelNumber(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompFEQAMLogicalChannelConnectLogicalChannelNumber(char **value)
{
	int logical_channel = 1;
	*value = strdup(itoa(logical_channel));
	return TRX_OK;
}

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.LogicalChannelService object */
TRX_STATUS getCompFEQAMLogicalChannelServiceAlias(char **value)
{
	*value = strdup("LogicalChannel-Alias");
	return TRX_OK;
}

TRX_STATUS getCompFEQAMLogicalChannelServiceName(char **value)
{
	*value = strdup("LogicalChannel-Name");
	return TRX_OK;
}

TRX_STATUS getCompFEQAMLogicalChannelServiceFrequency(char **value)
{
	int channelFreq = 1;
	*value = strdup(itoa(channelFreq));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMLogicalChannelServiceBER(char **value)
{
	int ber = 1;
	*value = strdup(itoa(ber));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMLogicalChannelServiceCBER(char **value)
{
	int cBer = 1;
	*value = strdup(itoa(cBer));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMLogicalChannelServiceSNR(char **value)
{
	int snr = 1;
	*value = strdup(itoa(snr));
	return TRX_OK;
}

TRX_STATUS setCompFEQAMLogicalChannelServicePreferred(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEQAMLogicalChannelServicePreferred(char **value)
{
	*value = strdup("False");
	return TRX_OK;
}

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.LogicalChannel object */
TRX_STATUS setCompFEQAMServiceListDatabaseLogicalChannelAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompFEQAMServiceListDatabaseLogicalChannelAlias(char **value)
{
	*value = strdup("QAM-LogicalChannel-Alias");
	return TRX_OK;
}
TRX_STATUS getCompFEQAMServiceListDatabaseLogicalChannelLogicalChannelNumber(char **value)
{
	int logicalChannel = 1;
	*value = strdup(itoa(logicalChannel));
	return TRX_OK;
}
TRX_STATUS getCompFEQAMServiceListDatabaseLogicalChannelServiceNumberOfEntries(char **value)
{
	int numOfEntries = 1;
	*value = strdup(itoa(numOfEntries));
	return TRX_OK;
}

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.ServiceListDatabase object */
TRX_STATUS setCompFEQAMServiceListDatabaseReset(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEQAMServiceListDatabaseReset(char **value)
{
	*value = strdup("Disabled");
	return TRX_OK;
}

TRX_STATUS getCompFEQAMServiceListDatabaseTotalServices(char **value)
{
	unsigned totalServices = 1;
	*value = strdup(itoa(totalServices));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMServiceListDatabaseLogicalChannelNumberOfEntries(char **value)
{
	unsigned numOfEntries = 1;
	*value = strdup(itoa(numOfEntries));
	return TRX_OK;
}

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.Install object */
TRX_STATUS setCompFEQAMInstallStart(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEQAMInstallStart(char **value)
{
	bool started = true;
	if(started)
	{
		*value = strdup("Started");
		return TRX_OK;
	}
	*value = strdup("Not-Started");
	return TRX_OK;
}

TRX_STATUS getCompFEQAMInstallStatus(char **value)
{
	*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS getCompFEQAMInstallProgress(char **value)
{
	*value = strdup("100%");
	return TRX_OK;
}

TRX_STATUS setCompFEQAMInstallStartFrequency(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEQAMInstallStartFrequency(char **value)
{
	unsigned startFreq = 0;
	*value = strdup(itoa(startFreq));
	return TRX_OK;
}

TRX_STATUS setCompFEQAMInstallStopFrequency(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEQAMInstallStopFrequency(char **value)
{
	unsigned stopFreq = 0;
	*value = strdup(itoa(stopFreq));
	return TRX_OK;
}

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.Service object */
TRX_STATUS getCompFEQAMServiceCurrentLogicalChannel(char **value)
{
	unsigned currentChannel = 1;
	*value = strdup(itoa(currentChannel));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMServiceCurrentService(char **value)
{
	*value = strdup(".Components.FrontEnd.1.DVBT.ServiceListDatabase.LogicalChannel.12.Service.1.");
	return TRX_OK;
}

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.Modulation object */
TRX_STATUS setCompFEQAMModulationFrequency(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationFrequency(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned freq;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	freq = status.settings.frequency;
	*value = strdup(itoa(freq));
	return TRX_OK;
}

TRX_STATUS setCompFEQAMModulationChannelBandwidth(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationChannelBandwidth(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned bandwidth;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	bandwidth = status.settings.bandwidth;
	*value = strdup(itoa(bandwidth));
	return TRX_OK;
}

TRX_STATUS setCompFEQAMModulationQamMode(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompFEQAMModulationQamMode(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned qamMode;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	qamMode = status.settings.mode;
	*value = strdup(itoa(qamMode));
	return TRX_OK;
}

TRX_STATUS setCompFEQAMModulationAnnex(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompFEQAMModulationAnnex(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned annex;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	annex = status.settings.annex;
	*value = strdup(itoa(annex));
	return TRX_OK;
}

#if 0
TRX_STATUS setCompFEQAMModulationBandwidth(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompFEQAMModulationBandwidth(char **value)
{}
#endif

TRX_STATUS setCompFEQAMModulationEnablePowerMeasurement(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompFEQAMModulationEnablePowerMeasurement(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	bool enabled;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	enabled = status.settings.enablePowerMeasurement;
	if (enabled)
	{
		*value = strdup("Enabled");
		return TRX_OK;
	}
	*value = strdup("Disabled");
	return TRX_OK;
}

TRX_STATUS setCompFEQAMModulationSpectrumMode(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompFEQAMModulationSpectrumMode(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned spectrumMode;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	spectrumMode = status.settings.spectrumMode;
	*value = strdup(itoa(spectrumMode));
	return TRX_OK;
}

TRX_STATUS setCompFEQAMModulationAcquisitionMode(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompFEQAMModulationAcquisitionMode(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned acquisitionMode;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	acquisitionMode = status.settings.acquisitionMode;
	*value = strdup(itoa(acquisitionMode));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationReceiverLock(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	bool locked;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	locked = status.receiverLock;
	if (locked)
	{
		*value = strdup("Locked");
		return TRX_OK;
	}
	*value = strdup("Not-Locked");
	return TRX_OK;
}
TRX_STATUS getCompFEQAMModulationFECLock(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	bool fecLocked;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	fecLocked = status.fecLock;
	if (fecLocked)
	{
		*value = strdup("Locked");
		return TRX_OK;
	}
	*value = strdup("Not-Locked");
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationOpllLock(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	bool opllLocked;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	opllLocked = status.opllLock;
	if (opllLocked)
	{
		*value = strdup("Locked");
		return TRX_OK;
	}
	*value = strdup("Not-Locked");
	return TRX_OK;
}

TRX_STATUS setCompFEQAMModulationSpectrumInverted(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getCompFEQAMModulationSpectrumInverted(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	bool spectrumInverted;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	spectrumInverted = status.spectrumInverted;
	if (spectrumInverted)
	{
		*value = strdup("Spectrum Inverted");
		return TRX_OK;
	}
	*value = strdup("Spectrum Not-Inverted");
	return TRX_OK;
}

TRX_STATUS setCompFEQAMModulationSymbolRate(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationSymbolRate(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned symbolRate;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	symbolRate = status.symbolRate;
	*value = strdup(itoa(symbolRate));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationSymbolRateError(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	int symbolRateError;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	symbolRateError = status.symbolRateError;
	*value = strdup(itoa(symbolRateError));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationIFAgcLevel(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned ifAgcLevel;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	ifAgcLevel = status.ifAgcLevel;
	*value = strdup(itoa(ifAgcLevel));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationRFAgcLevel(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned rfAgcLevel;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	rfAgcLevel = status.rfAgcLevel;
	*value = strdup(itoa(rfAgcLevel));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationIntAgcLevel(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned intAgcLevel;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	intAgcLevel = status.intAgcLevel;
	*value = strdup(itoa(intAgcLevel));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationSNREstimate(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned snrEstimate;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	snrEstimate = status.snrEstimate;
	*value = strdup(itoa(snrEstimate));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationFECCorrected(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned fecCorrected;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	fecCorrected = status.fecCorrected;
	*value = strdup(itoa(fecCorrected));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationFECUncorrected(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned fecUncorrected;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	fecUncorrected = status.fecUncorrected;
	*value = strdup(itoa(fecUncorrected));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationFECClean(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned fecClean;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	fecClean = status.fecClean;
	*value = strdup(itoa(fecClean));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationReacquireCount(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned reacquireCount;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	reacquireCount = status.reacquireCount;
	*value = strdup(itoa(reacquireCount));
	return TRX_OK;
}
TRX_STATUS getCompFEQAMModulationBitErrCorrected (char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned bitErrCorrected;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	bitErrCorrected = status.bitErrCorrected;
	*value = strdup(itoa(bitErrCorrected));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationViterbiUncorrectedBits(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned viterbiUncorrectedBits;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	viterbiUncorrectedBits = status.viterbiUncorrectedBits;
	*value = strdup(itoa(viterbiUncorrectedBits));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationViterbiTotalBits(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned viterbiTotalBits;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	viterbiTotalBits = status.viterbiTotalBits;
	*value = strdup(itoa(viterbiTotalBits));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationViterbiErrorRate(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	uint32_t viterbiErrorRate;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	viterbiErrorRate = status.viterbiErrorRate;
	*value = strdup(itoa(viterbiErrorRate));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationErrorRateUnits(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	uint32_t errorRateUnits;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	errorRateUnits = status.errorRateUnits;
	*value = strdup(itoa(errorRateUnits));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationCarrierFreqOffset(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	int carrierFreqOffset;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	carrierFreqOffset = status.carrierFreqOffset;
	*value = strdup(itoa(carrierFreqOffset));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationCarrierPhaseOffset(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	int carrierPhaseOffset;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	carrierPhaseOffset = status.carrierPhaseOffset;
	*value = strdup(itoa(carrierPhaseOffset));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationGoodRsBlockCount(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned goodRsBlockCount;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	goodRsBlockCount = status.goodRsBlockCount;
	*value = strdup(itoa(goodRsBlockCount));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationBerRawCount(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned berRawCount;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	berRawCount = status.berRawCount;
	*value = strdup(itoa(berRawCount));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationDSChannelPower(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	int dsChannelPower;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	dsChannelPower = status.dsChannelPower;
	*value = strdup(itoa(dsChannelPower));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationMainTap(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned mainTap;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	mainTap = status.mainTap;
	*value = strdup(itoa(mainTap));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationPostRsBer(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned postRsBer;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	postRsBer = status.postRsBer;
	*value = strdup(itoa(postRsBer));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationPostRsBerElapsedTime(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned postRsBerElapsedTime;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	postRsBerElapsedTime = status.postRsBerElapsedTime;
	*value = strdup(itoa(postRsBerElapsedTime));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationInterleaveDepth(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	uint16_t interleaveDepth;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	interleaveDepth = status.interleaveDepth;
	*value = strdup(itoa(interleaveDepth));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationLnaAgcLevel(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	unsigned lnaAgcLevel;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	lnaAgcLevel = status.lnaAgcLevel;
	*value = strdup(itoa(lnaAgcLevel));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationEqualizerGain(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	int equalizerGain;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	equalizerGain = status.equalizerGain;
	*value = strdup(itoa(equalizerGain));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationFrontendGain(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	int frontendGain;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	frontendGain = status.frontendGain;
	*value = strdup(itoa(frontendGain));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationDigitalAgcGain(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	int digitalAgcGain;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	digitalAgcGain = status.digitalAgcGain;
	*value = strdup(itoa(digitalAgcGain));
	return TRX_OK;
}

TRX_STATUS getCompFEQAMModulationHighResEqualizerGain(char **value)
{
	NEXUS_InterfaceName interfaceName;
	NEXUS_FrontendHandle handle;
	NEXUS_FrontendQamStatus status;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
	NEXUS_Error rc;
	char *module = "NEXUS_FrontendHandle";
	unsigned num;
	int highResEqualizerGain;

	strcpy(interfaceName.name, module);
	rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);

    if (!num || rc) {
        printf("no handles for '%s' found\n", module);
    }
    else
    {
		handle = (NEXUS_FrontendHandle)objects[0].object;
    }
    NEXUS_Frontend_GetQamStatus(handle, &status);
	highResEqualizerGain = status.highResEqualizerGain;
	*value = strdup(itoa(highResEqualizerGain));
	return TRX_OK;
}

