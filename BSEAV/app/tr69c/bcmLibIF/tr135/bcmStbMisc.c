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
#include "tr69clib_priv.h"
#include "bcmTR135Objs.h"


/****************************************/
/* STBService Object                    */
/****************************************/
TRX_STATUS setSTBServiceEnable(char **value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getSTBServiceEnable(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS setSTBServiceAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getSTBServiceAlias(char **value)
{
	*value = strdup("STBSerive.Alias");
	return TRX_OK;
}

/****************************************/
/* STBService.AVStreams.AVStream Object */
/****************************************/
TRX_STATUS getAVStreamsAVStreamStatus(char **value)
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
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

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

TRX_STATUS setAVStreamsAVStreamAlias(char *value)
{
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;
	b_AVStream_t *avStream;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

    if (client_registry[index].client)
    {
		getAVStream(&avStream, index);
		snprintf(avStream[index].alias, NEXUS_INTERFACE_NAME_MAX, value);
		setAVStreamParams(&avStream[index], index);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getAVStreamsAVStreamAlias(char **value)
{
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;
	b_AVStream_t *avStream;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

    if (client_registry[index].client)
    {
		getAVStream(&avStream, index);
		*value = strdup(avStream[index].alias);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getAVStreamsAVStreamName(char **value)
{
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;
	b_AVStream_t *avStream;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

    if (client_registry[index].client)
    {
		getAVStream(&avStream, index);
		*value = strdup(avStream[index].name);
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getAVStreamsAVStreamPVRState(char **value)
{
	*value = strdup("Disabled");
	return TRX_OK;
}

TRX_STATUS getAVStreamsAVStreamFrontEnd(char **value)
{
	*value = strdup("Components.Frontend.2");
	return TRX_OK;
}

TRX_STATUS getAVStreamsAVStreamInbound(char **value)
{
	*value = strdup("Components.Frontend.2.Inbound.3");
	return TRX_OK;
}

TRX_STATUS getAVStreamsAVStreamOutbound(char **value)
{
	*value = strdup("Components.Frontend.2.Outbound.3");
	return TRX_OK;
}

TRX_STATUS getAVStreamsAVStreamAudioDecoder(char **value)
{
	*value = strdup("Components.AudioDecoder.s");
	return TRX_OK;
}

TRX_STATUS getAVStreamsAVStreamVideoDecoder(char **value)
{
	*value = strdup("Components.VideoDecoder.s");
	return TRX_OK;
}

TRX_STATUS getAVStreamsAVStreamCA(char **value)
{
	*value = strdup("Components.CA.2");
	return TRX_OK;
}

TRX_STATUS getAVStreamsAVStreamDRM(char **value)
{
	*value = strdup("Components.DRM.2");
	return TRX_OK;
}

/*************************************************/
/* STBService.Components Object                  */
/*************************************************/
TRX_STATUS getAVStreamsActiveAVStreams(char **value)
{
	int num_active_streams=0;
    b_tr69c_get_client_count(&num_active_streams);
	*value = strdup(itoa(num_active_streams));
	return TRX_OK;
}

TRX_STATUS getAVStreamsAVStreamNumberOfEntries(char **value)
{
	int num_entries=0;
    b_tr69c_get_client_count(&num_entries);
	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

/****************************************/
/* STBService.AVPlayers.AVPLayer Object */
/****************************************/
TRX_STATUS setAVPlayersAVPlayerEnable(char *value)
{
	struct b_tr69c_data tr69c_data;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;
	void *send_data;
	int send_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

    if (client_registry[index].client)
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

TRX_STATUS getAVPlayersAVPlayerEnable(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

    if (client_registry[index].client)
    {
        tr69c_data.type = b_tr69c_type_get_video_decoder_status;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
		if (info.video_decoder_status.started)
			*value = strdup("Enabled");
		else
			*value = strdup("Disabled");
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS getAVPlayersAVPlayerStatus(char **value)
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
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

    if (client_registry[index].client)
    {
		NEXUS_VideoDecoderStatus vidStatus;
		NEXUS_AudioDecoderStatus audStatus;
        tr69c_data.type = b_tr69c_type_get_video_decoder_status;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
		vidStatus.started = info.video_decoder_status.started;
		tr69c_data.type = b_tr69c_type_get_audio_decoder_status;
        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
		audStatus.started = info.audio_decoder_status.started;
		if (vidStatus.started && audStatus.started)
			*value = strdup("Enabled");
		else
			*value = strdup("Disabled");
		return TRX_OK;
    }
	return TRX_ERR;
}

TRX_STATUS setAVPlayersAVPlayerAlias(char *value)
{
	b_AVPlayer_t *avPlayer;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	if (client_registry[index].client)
    {
		getAVPlayer(&avPlayer, index);
		BKNI_Snprintf(avPlayer[index].alias, 64, value);
		setAVPlayerParams(&avPlayer[index], index);
		return TRX_OK;
	}
	return TRX_ERR;
}

TRX_STATUS getAVPlayersAVPlayerAlias(char **value)
{
	b_AVPlayer_t *avPlayer;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	if (client_registry[index].client)
    {
		getAVPlayer(&avPlayer, index);
		if(avPlayer[index].alias)
			*value = strdup(avPlayer[index].alias);
		else
			*value = strdup("CPE-AVPlayer");
		return TRX_OK;
	}
	return TRX_ERR;
}

TRX_STATUS getAVPlayersAVPlayerName(char **value)
{
	b_AVPlayer_t *avPlayer;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	if (client_registry[index].client)
    {
		getAVPlayer(&avPlayer, index);
		if(avPlayer[index].name)
			*value = strdup(avPlayer[index].name);
		else
			*value = strdup("Broadcom AV Player");
		return TRX_OK;
	}
	return TRX_ERR;
}

TRX_STATUS getAVPlayersAVPlayerAudioLanguage(char **value)
{
	b_AVPlayer_t *avPlayer;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	if (client_registry[index].client)
    {
		getAVPlayer(&avPlayer, index);
		if(avPlayer[index].audioLanguage)
			*value = strdup(avPlayer[index].audioLanguage);
		else
			*value = strdup("English");
		setAVPlayerParams(avPlayer, index);
		return TRX_OK;
	}
	return TRX_ERR;
}

TRX_STATUS getAVPlayersAVPlayerSubtitlingStatus(char **value)
{
	b_AVPlayer_t *avPlayer;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	if (client_registry[index].client)
    {
		getAVPlayer(&avPlayer, index);
		if(avPlayer[index].subtitlingStatus)
			*value = strdup(avPlayer[index].subtitlingStatus);
		else
			*value = strdup("Disabled");
		setAVPlayerParams(avPlayer, index);
		return TRX_OK;
	}
	return TRX_ERR;
}

TRX_STATUS getAVPlayersAVPlayerSubtitlingLanguage(char **value)
{
	b_AVPlayer_t *avPlayer;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	if (client_registry[index].client)
    {
		getAVPlayer(&avPlayer, index);
		if(avPlayer[index].subtitlingLanguage)
			*value = strdup(avPlayer[index].subtitlingLanguage);
		else
			*value = strdup("English");
		setAVPlayerParams(avPlayer, index);
		return TRX_OK;
	}
	return TRX_ERR;
}

TRX_STATUS getAVPlayersAVPlayerAudioOutputs(char **value)
{
	*value = strdup(".Components.Audiooutput.2");
	return TRX_OK;
}

TRX_STATUS getAVPlayersAVPlayerVideoOutputs(char **value)
{
	*value = strdup(".Components.VideoOutput.2");
	return TRX_OK;
}

TRX_STATUS getAVPlayersAVPlayerMainStream(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	b_AVPlayer_t *avPlayer;
	InstanceDesc *idp;
	int index, rc, num_of_entries = 0;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;
	b_tr69c_get_client_count(&num_of_entries);


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	for (index = 0; index < num_of_entries; index++)
	{
	    if (client_registry[index].client)
	    {
	        tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
	        b_tr69c_server_get_client(client_registry[index].client, send_data, send_data_len, recv_data, recv_data_len);
			getAVPlayer(&avPlayer, index);

			if (info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				BKNI_Snprintf(avPlayer[index].mainStream, 256, "%s%d", ".AVStreams.AVStream.", index);
				setAVPlayerParams(&avPlayer[index], index);
				*value = strdup(avPlayer[index].mainStream);
				return TRX_OK;
			}
			else
			{
				*value = strdup("");
				return TRX_OK;
			}
	    }
	}
	return TRX_ERR;
}

TRX_STATUS getAVPlayersAVPlayerPIPStreams(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	b_AVPlayer_t *avPlayer;
	InstanceDesc *idp;
	int index, i, num_of_entries = 0, rc;
	char *sep = ", ";
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

	idp = getCurrentInstanceDesc();
	index = idp->instanceID - 1;
	b_tr69c_get_client_count(&num_of_entries);


    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	for (i = 0; i < num_of_entries; i++)
	{
		if (client_registry[i].client)
	    {
	        tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
	        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			getAVPlayer(&avPlayer, index);

			if (info.video_start_settings.videoWindowType == NxClient_VideoWindowType_ePip)
			{
				char tmp[32];
				BKNI_Snprintf(tmp, 32, "%s%d", ".AVStreams.AVStream.", index);
				strncat(avPlayer[index].PIPStreams, tmp, strlen(tmp));
				if (i < (num_of_entries -1))
					strncat(avPlayer[index].PIPStreams, sep, strlen(sep));
			}
			else
				BKNI_Snprintf(avPlayer[index].PIPStreams, 2, "");
	    }
		else
			return TRX_REQUEST_DENIED;
	}

	setAVPlayerParams(&avPlayer[index], index);
	if (avPlayer[index].PIPStreams)
		*value = strdup(avPlayer[index].PIPStreams);
	else
		*value = strdup("");
	return TRX_OK;
}

/****************************************/
/* STBService.AVPlayers.AVPLayer Object */
/****************************************/
TRX_STATUS getAVPlayersActiveAVPlayers(char **value)
{
	uint32 num_active_avplayer=1;

	*value = strdup(itoa(num_active_avplayer));
	return TRX_OK;
}

TRX_STATUS getAVPlayersAVPlayerNumberOfEntries(char **value)
{
	uint32 num_entries=1;

	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

TRX_STATUS setAVPlayersPreferredAudioLanguage(char *value)
{
	b_AVPlayer_t *avPlayer;
	b_tr69c_client_registry_t client_registry;
	int index = 0, rc;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	if (client_registry[index].client)
	{
		getAVPlayer(&avPlayer, index);
		BKNI_Snprintf(avPlayer[index].audioLanguage, 64, value);
		setAVPlayerParams(&avPlayer[index], index);
		return TRX_OK;
	}
	return TRX_ERR;
}

TRX_STATUS getAVPlayersPreferredAudioLanguage(char **value)
{
	b_AVPlayers_t *avPlayers;

	getAVPlayers(&avPlayers);
	*value = strdup(avPlayers->preferredAudioLanguage);
	return TRX_OK;
}

TRX_STATUS setAVPlayersPreferredSubtitlingLanguage(char *value)
{
	b_AVPlayers_t *avPlayers;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	getAVPlayers(&avPlayers);
	BKNI_Snprintf(avPlayers->preferredSubtitlingLanguage, 64, value);
	setAVPlayersParams(avPlayers);
	return TRX_OK;
}

TRX_STATUS getAVPlayersPreferredSubtitlingLanguage(char **value)
{
	b_AVPlayers_t *avPlayers;

	getAVPlayers(&avPlayers);
	*value = strdup(avPlayers->preferredSubtitlingLanguage);
	return TRX_OK;
}

TRX_STATUS setAVPlayersPreferredBehaviour(char *value)
{
	b_AVPlayers_t *avPlayers;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	getAVPlayers(&avPlayers);
	BKNI_Snprintf(avPlayers->preferredBehaviour, 64, value);
	setAVPlayersParams(avPlayers);
	return TRX_OK;
}

TRX_STATUS getAVPlayersPreferredBehaviour(char **value)
{
	*value = strdup("Letterbox");
	return TRX_OK;
}

TRX_STATUS setAVPlayersResetPINCode(char *value)
{
	b_AVPlayers_t *avPlayers;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;
	getAVPlayers(&avPlayers);
	avPlayers->resetPINCode = value;
	setAVPlayersParams(avPlayers);
	return TRX_OK;
}

TRX_STATUS getAVPlayersResetPINCode(char **value)
{
	b_AVPlayers_t *avPlayers;

	getAVPlayers(&avPlayers);
	if (avPlayers->resetPINCode)
		*value = strdup("0000");
	else
		*value = strdup("");
	return TRX_OK;
}

/********************************************************/
/* STBService.Applications.CDSPull.ContentItem Object */
/********************************************************/
TRX_STATUS setApplicationsCDSPullContentItemAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getApplicationsCDSPullContentItemAlias(char **value)
{
	*value = strdup("CDSPull Content Item Alias");
	return TRX_OK;
}

TRX_STATUS getApplicationsCDSPullContentItemContentReferenceId(char **value)
{
	uint32 reference_id=1;

	*value = strdup(itoa(reference_id));
	return TRX_OK;
}

TRX_STATUS getApplicationsCDSPullContentItemVersionNumber(char **value)
{
	uint32 version_number=1;

	*value = strdup(itoa(version_number));
	return TRX_OK;
}

TRX_STATUS setApplicationsCDSPullContentItemDeleteItem(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getApplicationsCDSPullContentItemDeleteItem(char **value)
{
	uint32 delete_item=1;

	*value = strdup(itoa(delete_item));
	return TRX_OK;
}

/********************************************************/
/* STBService.Applications.CDSPull Object */
/********************************************************/
TRX_STATUS getApplicationsCDSPullReference(char **value)
{
	*value = strdup("CDSPull Reference");
	return TRX_OK;
}
TRX_STATUS getApplicationsCDSPullContentItemNumberOfEntries(char **value)
{
	uint32 num_entries=1;

	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

/********************************************************/
/* STBService.Applications.CDSPush.ContentItem Object */
/********************************************************/
TRX_STATUS setApplicationsCDSPushContentItemAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getApplicationsCDSPushContentItemAlias(char **value)
{
	*value = strdup("CDSPush Content Item Alias");
	return TRX_OK;
}

TRX_STATUS getApplicationsCDSPushContentItemContentReferenceId(char **value)
{
	uint32 reference_id=1;

	*value = strdup(itoa(reference_id));
	return TRX_OK;
}

TRX_STATUS getApplicationsCDSPushContentItemVersionNumber(char **value)
{
	uint32 version_number=1;

	*value = strdup(itoa(version_number));
	return TRX_OK;
}

TRX_STATUS setApplicationsCDSPushContentItemDeleteItem(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getApplicationsCDSPushContentItemDeleteItem(char **value)
{
	uint32 delete_item=1;

	*value = strdup(itoa(delete_item));
	return TRX_OK;
}

/********************************************************/
/* STBService.Applications.CDSPush Object */
/********************************************************/
TRX_STATUS getApplicationsCDSPushReference(char **value)
{
	*value = strdup("CDSPush Reference");
	return TRX_OK;
}

TRX_STATUS getApplicationsCDSPushContentItemNumberOfEntries(char **value)
{
	uint32 num_entries=1;

	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

/********************************************************/
/* STBService.Applications.AudienceStats.Channel Object */
/********************************************************/
TRX_STATUS setApplicationsAudienceStatsChannelAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getApplicationsAudienceStatsChannelAlias(char **value)
{
	*value = strdup("Broadcom TR69 Audience Channel Alias");
	return TRX_OK;
}

TRX_STATUS getApplicationsAudienceStatsChannelName(char **value)
{
	*value = strdup("Broadcom TR69 Audience Channel Name");
	return TRX_OK;
}

TRX_STATUS getApplicationsAudienceStatsChannelDuration(char **value)
{
	uint32 channel_duration=1;

	*value = strdup(itoa(channel_duration));
	return TRX_OK;
}

/****************************************/
/* STBService.Applications.AudienceStats Object */
/****************************************/
TRX_STATUS setApplicationsAudienceStatsEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getApplicationsAudienceStatsEnable(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS setApplicationsAudienceStatsReset(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getApplicationsAudienceStatsReset(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getApplicationsAudienceStatsResetTime(char **value)
{
	uint32 reset_time=1;

	*value = strdup(itoa(reset_time));
	return TRX_OK;
}

TRX_STATUS getApplicationsAudienceStatsChannelNumberOfEntries(char **value)
{
	uint32 num_entries=1;

	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

/****************************************/
/* STBService.Applications.ServiceProvider Object */
/****************************************/
TRX_STATUS setApplicationsServiceProviderAlias(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getApplicationsServiceProviderAlias(char **value)
{
	*value = strdup("service-provider-alias");
	return TRX_OK;
}
TRX_STATUS setApplicationsServiceProviderName(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getApplicationsServiceProviderName(char **value)
{
	*value = strdup("service-provider-name");
	return TRX_OK;
}
TRX_STATUS setApplicationsServiceProviderDomain(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getApplicationsServiceProviderDomain(char **value)
{
	*value = strdup("service-provider-domain");
	return TRX_OK;
}
TRX_STATUS setApplicationsServiceProviderServiceDiscoveryServer(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}
TRX_STATUS getApplicationsServiceProviderServiceDiscoveryServer(char **value)
{
	*value = strdup("service-provider-service-discovery-server");
	return TRX_OK;
}
TRX_STATUS getApplicationsServiceProviderActiveBCGServers(char **value)
{
	*value = strdup("service-provider-active-bcg-server");
	return TRX_OK;
}

/****************************************/
/* STBService.Applications Object */
/****************************************/
TRX_STATUS getApplicationsServiceProviderNumberOfEntries(char **value)
{
	uint32 num_entries=1;

	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

