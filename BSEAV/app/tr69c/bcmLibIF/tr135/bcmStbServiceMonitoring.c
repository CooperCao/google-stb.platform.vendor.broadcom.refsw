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

#include "tr69clib.h"
#include "tr69clib_priv.h"
#include "bcmTR135Objs.h"


extern b_tr69c_server_t tr69c_server;

/******************************************************************************/
/* STBService.ServiceMonitoring.MainStream.SampleHighLevelMetricStats Object */
/*******************************************************************************/
TRX_STATUS setSMMainStreamSampleHighLevelMetricStatsEnable(char **value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsEnable(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsStatus(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS setSMMainStreamSampleHighLevelMetricStatsAlias(char **value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsAlias(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetricName(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS setSMMainStreamSampleHighLevelMetricStatsMetric1Threshold(char *value)
{
	uint32 threshold;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	threshold = atoi(value);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetric1Threshold(char **value)
{
	uint32 threshold=1;

	*value = strdup(itoa(threshold));
	return TRX_OK;
}

TRX_STATUS setSMMainStreamSampleHighLevelMetricStatsMetric2Threshold(char *value)
{
	uint32 threshold;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	threshold = atoi(value);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetric2Threshold(char **value)
{
	uint32 threshold=1;

	*value = strdup(itoa(threshold));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsSampleSeconds(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetric1(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetric2(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetric1Failures(char **value)
{
	uint32 metric_failure=1;

	*value = strdup(itoa(metric_failure));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetric2Failures(char **value)
{
	uint32 metric_failure=1;

	*value = strdup(itoa(metric_failure));
	return TRX_OK;
}

TRX_STATUS setSMMainStreamSampleHighLevelMetricStatsMetricSampleInterval(char *value)
{
	uint32 interval;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	interval = atoi(value);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetricSampleInterval(char **value)
{
	uint32 interval=1;

	*value = strdup(itoa(interval));
	return TRX_OK;
}

TRX_STATUS setSMMainStreamSampleHighLevelMetricStatsMetricThreshold(char *value)
{
	uint32 threshold;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	threshold = atoi(value);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetricThreshold(char **value)
{
	uint32 metric_threshold=1;

	*value = strdup(itoa(metric_threshold));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetric(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsMetricFailures(char **value)
{
	uint32 metric_failure=1;

	*value = strdup(itoa(metric_failure));
	return TRX_OK;
}

/******************************************************************************/
/* STBService.ServiceMonitoring.MainStream.SampleVideoResponseStats Object   */
/******************************************************************************/
TRX_STATUS getSMMainStreamSampleVideoResponseStatsSampleSeconds(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsAverageVideoSystemResponse(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsMinimumVideoSystemResponse(char **value)
{
	uint32 min_video_resp=1;

	*value = strdup(itoa(min_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsMaximumVideoSystemResponse(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsAverageVoDControlResponse(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsMinimumVoDControlResponse(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsMaximumVoDControlResponse(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsVoDControlResponse(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsVideoSystemResponse(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsRequestedTransactions(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsAccessSuccesses(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsCompletionCount(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoResponseStatsChannelChangeFailures(char **value)
{
	uint32 max_video_resp=1;

	*value = strdup(itoa(max_video_resp));
	return TRX_OK;
}

/******************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Sample.AudioDecoderStats Object                     */
/******************************************************************************/
TRX_STATUS getSMMainStreamSampleAudioDecoderStatsTotalSeconds(char **value)
{
	b_sampleAudioDecoderStats_t *sample;

	getSampleAudioDecoderStats(&sample);
	*value = strdup(sample->seconds);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleAudioDecoderStatsDecodedFrames(char **value)
{
	b_sampleAudioDecoderStats_t *sample;

	getSampleAudioDecoderStats(&sample);
	*value = strdup(sample->decodedFrames);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleAudioDecoderStatsDecodingErrors(char **value)
{
	b_sampleAudioDecoderStats_t *sample;

	getSampleAudioDecoderStats(&sample);
	*value = strdup(sample->decodingErrors);
	return TRX_OK;
}

/******************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Sample.VideoDecoderStats Object                     */
/******************************************************************************/
TRX_STATUS getSMMainStreamSampleVideoDecoderStatsTotalSeconds(char **value)
{
	b_sampleVideoDecoderStats_t *sample;

	getSampleVideoDecoderStats(&sample);
	*value = strdup(sample->seconds);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsFrameRate(char **value)
{
	b_sampleVideoDecoderStats_t *sample;

	getSampleVideoDecoderStats(&sample);
	*value = strdup(sample->frameRate);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsDecodedFrames(char **value)
{
	b_sampleVideoDecoderStats_t *sample;

	getSampleVideoDecoderStats(&sample);
	*value = strdup(sample->decodedFrames);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsLostFrames(char **value)
{
	b_sampleVideoDecoderStats_t *sample;

	getSampleVideoDecoderStats(&sample);
	*value = strdup(sample->lostFrames);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsConcealedFrames(char **value)
{
	b_sampleVideoDecoderStats_t *sample;

	getSampleVideoDecoderStats(&sample);
	*value = strdup(sample->concealedFrames);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsIDecodedFrames(char **value)
{
	b_sampleVideoDecoderStats_t *sample;

	getSampleVideoDecoderStats(&sample);
	*value = strdup(sample->IDecodedFrames);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsILostFrames(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsIConcealedFrames(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsPDecodedFrames(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsPLostFrames(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsPConcealedFrames(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsBDecodedFrames(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsBLostFrames(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsBConcealedFrames(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleVideoDecoderStatsAVResynchCounter(char **value)
{
	b_sampleVideoDecoderStats_t *sample;

	getSampleVideoDecoderStats(&sample);
	*value = strdup(sample->AVResynchCounter);
	return TRX_OK;
}

/******************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Sample.MPEG2TSStats Object                     */
/******************************************************************************/
TRX_STATUS getSMMainStreamSampleMPEG2TSStatsSampleSeconds(char **value)
{
	b_sampleMPEG2Stats_t *sample;

	getSampleMPEG2TSStats(&sample);
	*value = strdup(sample->seconds);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleMPEG2TSStatsTSPacketsReceived(char **value)
{
	b_sampleMPEG2Stats_t *sample;

	getSampleMPEG2TSStats(&sample);
	*value = strdup(sample->TSPacketsReceived);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleMPEG2TSStatsTSPacketsDrained(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleMPEG2TSStatsTSSyncByteErrorCount(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleMPEG2TSStatsTSSyncLossCount(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleMPEG2TSStatsPacketDiscontinuityCounter(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleMPEG2TSStatsPacketDiscontinuityCounterBeforeCA(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

/******************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Sample.RTPStats Object                     */
/******************************************************************************/
TRX_STATUS getSMMainStreamSampleRTPStatsSampleSeconds(char **value)
{
	#if PLAYBACK_IP_SUPPORT

	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	b_total_t *total;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_sampleRTP_t *sample;
	char tmp[10];
	const char *sep = ", ";

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
        if (info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eRtp)
        {
			getSampleRTP(&sample);
			getTotal(&total);

			if (sample->seconds[0] == '\0')
				BKNI_Snprintf(tmp, sizeof(tmp), "%d", total->totalSeconds);
			else
				BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, total->totalSeconds);

			if ((strlen(sample->seconds) + strlen(tmp)) <= 256)
				strncat(sample->seconds, tmp, strlen(tmp));
			else
			{
				BKNI_Memset(sample->seconds, 0, sizeof(sample->seconds));
				strncat(sample->seconds, tmp, strlen(tmp));
			}
			setSampleRTP(sample);
			*value = strdup(sample->seconds);
        }
		else
			*value = strdup(itoa(0));
		return TRX_OK;
    }
	else
		return TRX_ERR;

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getSMMainStreamSampleRTPStatsPacketsExpected(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsPacketsDiscarded(char **value)
{
	b_sampleRTP_t *sample;

	getSampleRTP(&sample);
	*value = strdup(sample->packetsDiscarded);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsPacketsOutOfSequence(char **value)
{
	b_sampleRTP_t *sample;

	getSampleRTP(&sample);
	*value = strdup(sample->packetsOutOfSequence);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsPacketsReceived(char **value)
{
	b_sampleRTP_t *sample;

	getSampleRTP(&sample);
	*value = strdup(sample->packetsReceived);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsPacketsReceivedBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsRetransmitTimeouts(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsPacketsLost(char **value)
{
	b_sampleRTP_t *sample;

	getSampleRTP(&sample);
	*value = strdup(sample->packetsLost);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsPacketsLostBeforeEC(char **value)
{
	b_sampleRTP_t *sample;

	getSampleRTP(&sample);
	*value = strdup(sample->packetsLostBeforeEC);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsPacketsLostByEventHistogram(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsPacketsLostByEventHistogramBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsLossEvents(char **value)
{
	b_sampleRTP_t *sample;

	getSampleRTP(&sample);
	*value = strdup(sample->lossEvents);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsLossEventsBeforeEC(char **value)
{
	b_sampleRTP_t *sample;

	getSampleRTP(&sample);
	*value = strdup(sample->lossEventsBeforeEC);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsDelayBetweenLossEventsHistogram(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsDelayBetweenLossEventsHistogramBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsSevereLossIndexCount(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsSevereLossIndexCountBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsAverageLossDistance(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsAverageLossDistanceBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsMinimumLossDistance(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsMinimumLossDistanceBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsSevereLossLengthCount(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsSevereLossLengthCountBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsDurationSevereLossEventsHistogram(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsDurationSevereLossEventsHistogramBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsMaximumLossPeriod(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsMaximumLossPeriodBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsAverageLossPeriod(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsAverageLossPeriodBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsMinimumLossPeriod(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleRTPStatsMinimumLossPeriodBeforeEC(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

/******************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Sample.TCPStats Object                     */
/******************************************************************************/
TRX_STATUS getSMMainStreamSampleTCPStatsSampleSeconds(char **value)
{
	#if PLAYBACK_IP_SUPPORT

	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	b_total_t *total;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_sampleTCP_t *sample;
	char tmp[10];
	const char *sep = ", ";

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
        if (info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttp ||
			info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttps)
        {
			getSampleTCP(&sample);
			getTotal(&total);

			if (sample->seconds[0] == '\0')
				BKNI_Snprintf(tmp, sizeof(tmp), "%d", total->totalSeconds);
			else
				BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, total->totalSeconds);

			if ((strlen(sample->seconds) + strlen(tmp)) <= 256)
				strncat(sample->seconds, tmp, strlen(tmp));
			else
			{
				BKNI_Memset(sample->seconds, 0, sizeof(sample->seconds));
				strncat(sample->seconds, tmp, strlen(tmp));
			}
			setSampleTCP(sample);
			*value = strdup(sample->seconds);
        }
		else
			*value = strdup(itoa(0));
		return TRX_OK;
    }
	else
		return TRX_ERR;

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getSMMainStreamSampleTCPStatsPacketsReceived(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_sampleTCP_t *sample;

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
        if (info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttp ||
			info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttps)
        {
			const char *sep = ", ";
			char tmp[20];

			getSampleTCP(&sample);

			if (sample->packetsReceived[0] == '\0')
				BKNI_Snprintf(tmp, sizeof(tmp), "%d", (info.playback_ip_status.totalConsumed/TS_PACKET_SIZE));
			else
				BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, (info.playback_ip_status.totalConsumed/TS_PACKET_SIZE));

			if ((strlen(sample->packetsReceived) + strlen(tmp)) <= 256)
				strncat(sample->packetsReceived, tmp, strlen(tmp));
			else
			{
				BKNI_Memset(sample->packetsReceived, 0, sizeof(sample->packetsReceived));
				strncat(sample->packetsReceived, tmp, strlen(tmp));
			}
			setSampleTCP(sample);
			*value = strdup(sample->packetsReceived);
        }
		else
			*value = strdup(itoa(0));
		return TRX_OK;
    }
	else
		return TRX_ERR;
	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getSMMainStreamSampleTCPStatsPacketsRetransmitted(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleTCPStatsBytesReceived(char **value)
{
	#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_sampleTCP_t *sample;

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
        if (info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttp ||
			info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttps)
        {
			const char *sep = ", ";
			char tmp[20];

			getSampleTCP(&sample);

			if (sample->bytesReceived[0] == '\0')
				BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.playback_ip_status.totalConsumed);
			else
				BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.playback_ip_status.totalConsumed);

			if ((strlen(sample->bytesReceived) + strlen(tmp)) <= 256)
				strncat(sample->bytesReceived, tmp, strlen(tmp));
			else
			{
				BKNI_Memset(sample->bytesReceived, 0, sizeof(sample->bytesReceived));
				strncat(sample->bytesReceived, tmp, strlen(tmp));
			}
			setSampleTCP(sample);
			*value = strdup(sample->bytesReceived);
        }
		else
			*value = strdup(itoa(0));
		return TRX_OK;
    }
	else
		return TRX_ERR;
	#endif

	return TRX_INVALID_PARAM_TYPE;

}

/******************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Sample.DejitteringStats Object                     */
/******************************************************************************/
TRX_STATUS getSMMainStreamSampleDejitteringStatsSampleSeconds(char **value)
{
	b_total_t *total;
	b_sampleDejittering_t *sample;
	static char tmp[10];
	const char *sep = ", ";


	getTotal(&total);
	getSampleDejittering(&sample);

	if (sample->seconds[0] == '\0')
		BKNI_Snprintf(tmp, sizeof(tmp), "%d", total->totalSeconds);
	else
		BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, total->totalSeconds);

	if ((strlen(sample->seconds) + strlen(tmp)) <= 256)
		strncat(sample->seconds, tmp, strlen(tmp));
	else
	{
		BKNI_Memset(sample->seconds, 0, sizeof(sample->seconds));
		strncat(sample->seconds, tmp, strlen(tmp));
	}
	setSampleDejittering(sample);
	*value = strdup(sample->seconds);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleDejitteringStatsOverruns(char **value)
{
	b_sampleDejittering_t *sample;

	getSampleDejittering(&sample);
	*value = strdup(sample->overruns);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleDejitteringStatsUnderruns(char **value)
{
	b_sampleDejittering_t *sample;

	getSampleDejittering(&sample);
	*value = strdup(sample->underruns);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleDejitteringStatsEmptyBufferTime(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}


/******************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Sample Object                     */
/******************************************************************************/
TRX_STATUS getSMMainStreamSampleSampleSeconds(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleSignificantChanges(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSamplePVRTimeShift(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamSampleHighLevelMetricStatsNumberOfEntries(char **value)
{
	uint32 num_entries=1;

	*value = strdup(itoa(num_entries));
	return TRX_OK;
}

/**************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Total.VideoResponseStats Object */
/**************************************************************************/
TRX_STATUS getSMMainStreamTotalVideoResponseStatsTotalSeconds(char **value)
{
	uint32 total_seconds=1;

	*value = strdup(itoa(total_seconds));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoResponseStatsChannelChangeFailures(char **value)
{
	uint32 channel_change_failures=1;

	*value = strdup(itoa(channel_change_failures));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoResponseStatsChannelFailures(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoResponseStatsMinimumVoDControlResponse(char **value)
{
	uint32 min_vod_control_response=1;

	*value = strdup(itoa(min_vod_control_response));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoResponseStatsMaximumVoDControlResponse(char **value)
{
	uint32 max_vod_control_response=1;

	*value = strdup(itoa(max_vod_control_response));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoResponseStatsRequestedTransactions(char **value)
{
	uint32 requested_transactions=1;

	*value = strdup(itoa(requested_transactions));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoResponseStatsAccessSuccesses(char **value)
{
	uint32 access_successes=1;

	*value = strdup(itoa(access_successes));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoResponseStatsCompletionCount(char **value)
{
	uint32 completion_count=1;

	*value = strdup(itoa(completion_count));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoResponseStatsMinimumVideoSystemResponse(char **value)
{
	uint32 min_video_system_response=1;

	*value = strdup(itoa(min_video_system_response));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoResponseStatsMaximumVideoSystemResponse(char **value)
{
	uint32 max_video_system_response=1;

	*value = strdup(itoa(max_video_system_response));
	return TRX_OK;
}


/**************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Total.AudioDecoderStats Object */
/**************************************************************************/
TRX_STATUS getSMMainStreamTotalAudioDecoderStatsTotalSeconds(char **value)
{
	b_total_t *total;
	struct timeval tv;
	unsigned currTime;
	b_sampleAudioDecoderStats_t *sample;
	static char tmp[10];
	const char *sep = " ,";

	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec;

	getTotal(&total);
	total->totalSeconds = currTime - total->startTime;
	setTotal(total);
	*value = strdup(itoa(total->totalSeconds));
	getSampleAudioDecoderStats(&sample);

	if (sample->seconds[0] == '\0')
		BKNI_Snprintf(tmp, sizeof(tmp), "%d", total->totalSeconds);
	else
		BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, total->totalSeconds);

	if ((strlen(sample->seconds) + strlen(tmp)) <= 256)
		strncat(sample->seconds, tmp, strlen(tmp));
	else
	{
		BKNI_Memset(sample->seconds, 0, sizeof(sample->seconds));
		strncat(sample->seconds, tmp, strlen(tmp));
	}
	setSampleAudioDecoderStats(sample);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalAudioDecoderStatsDecodedFrames(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_sampleAudioDecoderStats_t *sample;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				static char tmp[10];
				const char *sep = " ,";

				getSampleAudioDecoderStats(&sample);
				tr69c_data.type = b_tr69c_type_get_audio_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				*value = strdup(itoa(info.audio_decoder_status.framesDecoded));

				if (sample->decodedFrames[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", *value);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, *value);

				if ((strlen(sample->decodedFrames) + strlen(tmp)) <= 256)
					strncat(sample->decodedFrames, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->decodedFrames, 0, sizeof(sample->decodedFrames));
					strncat(sample->decodedFrames, tmp, strlen(tmp));
				}
				setSampleAudioDecoderStats(sample);
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalAudioDecoderStatsDecodingErrors(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_sampleAudioDecoderStats_t *sample;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				static char tmp[10];
				const char *sep = " ,";

				getSampleAudioDecoderStats(&sample);
				tr69c_data.type = b_tr69c_type_get_audio_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				*value = strdup(itoa(info.audio_decoder_status.frameErrors));

				if (sample->decodedFrames[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", *value);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, *value);

				if ((strlen(sample->decodingErrors) + strlen(tmp)) <= 256)
					strncat(sample->decodingErrors, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->decodingErrors, 0, sizeof(sample->decodingErrors));
					strncat(sample->decodingErrors, tmp, strlen(tmp));
				}
				setSampleAudioDecoderStats(sample);
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;
}

/**************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Total.VideoDecoderStats Object */
/**************************************************************************/
TRX_STATUS getSMMainStreamTotalVideoDecoderStatsTotalSeconds(char **value)
{
	b_total_t *total;
	b_sampleVideoDecoderStats_t *sample;
	static char tmp[10];
	const char *sep = " ,";
	struct timeval tv;
	unsigned currTime;

	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec;

	getTotal(&total);
	total->totalSeconds = currTime - total->startTime;
	setTotal(total);
	*value = strdup(itoa(total->totalSeconds));
	getSampleVideoDecoderStats(&sample);

	if (sample->seconds[0] == '\0')
		BKNI_Snprintf(tmp, sizeof(tmp), "%d", total->totalSeconds);
	else
		BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, total->totalSeconds);

	if ((strlen(sample->seconds) + strlen(tmp)) <= 256)
		strncat(sample->seconds, tmp, strlen(tmp));
	else
	{
		BKNI_Memset(sample->seconds, 0, sizeof(sample->seconds));
		strncat(sample->seconds, tmp, strlen(tmp));
	}
	setSampleVideoDecoderStats(sample);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsFrameRate(char **value)
{

	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_videoDecoderStats_t *stats;
	b_sampleVideoDecoderStats_t *sample;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				static char tmp[10];
				const char *sep = " ,";

				getSampleVideoDecoderStats(&sample);
				tr69c_data.type = b_tr69c_type_get_video_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				getVideoDecoderStats(&stats);
				stats->frameRate = info.video_decoder_status.frameRate;
				setVideoDecoderStats(stats);
				*value = strdup(itoa(info.video_decoder_status.frameRate));

				if (sample->frameRate[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", stats->frameRate);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, stats->frameRate);

				if ((strlen(sample->frameRate) + strlen(tmp)) <= 256)
					strncat(sample->frameRate, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->frameRate, 0, sizeof(sample->frameRate));
					strncat(sample->frameRate, tmp, strlen(tmp));
				}
				setSampleVideoDecoderStats(sample);
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;

}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsDecodedFrames(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_videoDecoderStats_t *stats;
	b_sampleVideoDecoderStats_t *sample;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				static char tmp[10];
				const char *sep = " ,";

				getSampleVideoDecoderStats(&sample);
				tr69c_data.type = b_tr69c_type_get_video_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				getVideoDecoderStats(&stats);
				stats->decodedFrames = info.video_decoder_status.numDecoded;
				setVideoDecoderStats(stats);
				*value = strdup(itoa(info.video_decoder_status.numDecoded));

				if (sample->decodedFrames[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", stats->decodedFrames);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, stats->decodedFrames);

				if ((strlen(sample->decodedFrames) + strlen(tmp)) <= 256)
					strncat(sample->decodedFrames, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->decodedFrames, 0, sizeof(sample->decodedFrames));
					strncat(sample->decodedFrames, tmp, strlen(tmp));
				}
				setSampleVideoDecoderStats(sample);
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;

}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsLostFrames(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_videoDecoderStats_t *stats;
	b_sampleVideoDecoderStats_t *sample;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
			static char tmp[10];
			const char *sep = " ,";

			getSampleVideoDecoderStats(&sample);
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				tr69c_data.type = b_tr69c_type_get_video_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				getVideoDecoderStats(&stats);
				stats->lostFrames = info.video_decoder_status.numDecodeErrors + info.video_decoder_status.numDecodeDrops;
				setVideoDecoderStats(stats);
				*value = strdup(itoa(info.video_decoder_status.numDecodeErrors + info.video_decoder_status.numDecodeDrops));

				if (sample->lostFrames[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", stats->lostFrames);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, stats->lostFrames);

				if ((strlen(sample->lostFrames) + strlen(tmp)) <= 256)
					strncat(sample->lostFrames, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->lostFrames, 0, sizeof(sample->lostFrames));
					strncat(sample->lostFrames, tmp, strlen(tmp));
				}
				setSampleVideoDecoderStats(sample);
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsConcealedFrames(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_videoDecoderStats_t *stats;
	b_sampleVideoDecoderStats_t *sample;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				static char tmp[10];
				const char *sep = " ,";

				getSampleVideoDecoderStats(&sample);
				tr69c_data.type = b_tr69c_type_get_video_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				getVideoDecoderStats(&stats);
				stats->concealedFrames = info.video_decoder_status.numDecodeErrors;
				setVideoDecoderStats(stats);
				*value = strdup(itoa(info.video_decoder_status.numDecodeErrors));

				if (sample->concealedFrames[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", stats->concealedFrames);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, stats->concealedFrames);

				if ((strlen(sample->concealedFrames) + strlen(tmp)) <= 256)
					strncat(sample->concealedFrames, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->concealedFrames, 0, sizeof(sample->concealedFrames));
					strncat(sample->concealedFrames, tmp, strlen(tmp));
				}
				setSampleVideoDecoderStats(sample);
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsIDecodedFrames(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_videoDecoderStats_t *stats;
	b_sampleVideoDecoderStats_t *sample;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				static char tmp[10];
				const char *sep = " ,";

				getSampleVideoDecoderStats(&sample);
				tr69c_data.type = b_tr69c_type_get_video_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				getVideoDecoderStats(&stats);
				stats->IDecodedFrames = info.video_decoder_status.numIFramesDisplayed;
				setVideoDecoderStats(stats);
				*value = strdup(itoa(info.video_decoder_status.numIFramesDisplayed));

				if (sample->IDecodedFrames[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", stats->IDecodedFrames);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, stats->IDecodedFrames);

				if ((strlen(sample->IDecodedFrames) + strlen(tmp)) <= 256)
					strncat(sample->IDecodedFrames, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->IDecodedFrames, 0, sizeof(sample->IDecodedFrames));
					strncat(sample->IDecodedFrames, tmp, strlen(tmp));
				}
				setSampleVideoDecoderStats(sample);
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsILostFrames(char **value)
{
	uint32 num_lost_frames=1;

	*value = strdup(itoa(num_lost_frames));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsIConcealedFrames(char **value)
{
	uint32 num_concealed_frames=1;

	*value = strdup(itoa(num_concealed_frames));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsPDecodedFrames(char **value)
{
	uint32 num_decoded_frames=1;

	*value = strdup(itoa(num_decoded_frames));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsPLostFrames(char **value)
{
	uint32 num_lost_frames=1;

	*value = strdup(itoa(num_lost_frames));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsPConcealedFrames(char **value)
{
	uint32 num_concealed_frames=1;

	*value = strdup(itoa(num_concealed_frames));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsBDecodedFrames(char **value)
{
	uint32 num_decoded_frames=1;

	*value = strdup(itoa(num_decoded_frames));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsBLostFrames(char **value)
{
	uint32 num_lost_frames=1;

	*value = strdup(itoa(num_lost_frames));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsBConcealedFrames(char **value)
{
	uint32 num_concealed_frames=1;

	*value = strdup(itoa(num_concealed_frames));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalVideoDecoderStatsAVResynchCounter(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_videoDecoderStats_t *stats;
	b_sampleVideoDecoderStats_t *sample;
	unsigned video_ptsErrorCount = 0;
	unsigned audio_ptsErrorCount = 0;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				static char tmp[10];
				const char *sep = " ,";

				getSampleVideoDecoderStats(&sample);
				tr69c_data.type = b_tr69c_type_get_video_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				video_ptsErrorCount = info.video_decoder_status.ptsErrorCount;
				tr69c_data.type = b_tr69c_type_get_audio_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				audio_ptsErrorCount = info.audio_decoder_status.ptsErrorCount;
				getVideoDecoderStats(&stats);
				stats->AVResynchCounter = video_ptsErrorCount + audio_ptsErrorCount;
				setVideoDecoderStats(stats);
				*value = strdup(itoa(video_ptsErrorCount + audio_ptsErrorCount));

				if (sample->AVResynchCounter[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", stats->AVResynchCounter);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, stats->AVResynchCounter);

				if ((strlen(sample->AVResynchCounter) + strlen(tmp)) <= 256)
					strncat(sample->AVResynchCounter, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->AVResynchCounter, 0, sizeof(sample->AVResynchCounter));
					strncat(sample->AVResynchCounter, tmp, strlen(tmp));
				}
				setSampleVideoDecoderStats(sample);
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;
}

/**************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Total.MPEG2TSStats             */
/**************************************************************************/
TRX_STATUS getSMMainStreamTotalMPEG2TSStatsTotalSeconds(char **value)
{
	b_total_t *total;
	b_sampleMPEG2Stats_t *sample;
	struct timeval tv;
	unsigned currTime;
	static char tmp[10];
	const char *sep = " ,";

	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec;

	getTotal(&total);
	total->totalSeconds = currTime - total->startTime;
	setTotal(total);
	*value = strdup(itoa(total->totalSeconds));
	getSampleMPEG2TSStats(&sample);

	if (sample->seconds[0] == '\0')
		BKNI_Snprintf(tmp, sizeof(tmp), "%d", total->totalSeconds);
	else
		BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, total->totalSeconds);

	if ((strlen(sample->seconds) + strlen(tmp)) <= 256)
		strncat(sample->seconds, tmp, strlen(tmp));
	else
	{
		BKNI_Memset(sample->seconds, 0, sizeof(sample->seconds));
		strncat(sample->seconds, tmp, strlen(tmp));
	}
	setSampleMPEG2TSStats(sample);
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalMPEG2TSStatsTSPacketsReceived(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_MPEG2TSStats_t *stats;
	b_sampleMPEG2Stats_t *sample;

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
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				static char tmp[10];
				const char *sep = " ,";

				getSampleMPEG2TSStats(&sample);
				tr69c_data.type = b_tr69c_type_get_video_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				getMPEG2TSStats(&stats);
				stats->TSPacketsReceived = info.video_decoder_status.numBytesDecoded/184;
				setMPEG2TSStats(stats);
				*value = strdup(itoa(info.video_decoder_status.numBytesDecoded/184)); /* 188 bytes - 4 bytes TS header */

				if (sample->TSPacketsReceived[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", stats->TSPacketsReceived);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, stats->TSPacketsReceived);

				if ((strlen(sample->TSPacketsReceived) + strlen(tmp)) <= 256)
					strncat(sample->TSPacketsReceived, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->TSPacketsReceived, 0, sizeof(sample->TSPacketsReceived));
					strncat(sample->TSPacketsReceived, tmp, strlen(tmp));
				}
				setSampleMPEG2TSStats(sample);
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalMPEG2TSStatsTSSyncByteErrorCount(char **value)
{
	uint32 num_ts_sync_byte_error_count=1;

	*value = strdup(itoa(num_ts_sync_byte_error_count));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalMPEG2TSStatsTSSyncLossCount(char **value)
{
	uint32 num_ts_sync_loss_count=1;

	*value = strdup(itoa(num_ts_sync_loss_count));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalMPEG2TSStatsPacketDiscontinuityCounter(char **value)
{
	uint32 num_packets_discontinuity_count=1;

	*value = strdup(itoa(num_packets_discontinuity_count));
	return TRX_OK;

	#if 0 /* Not supported at the moment */
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	InstanceDesc *idp;
	int index, rc, num_of_entries, i;
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
			NEXUS_PidChannelStatus pidChannelStatus;
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				*value = strdup(itoa(info.pid_channel_status.continuityCountErrors));
				return TRX_OK;
			}
	    }
	}
	*value = strdup(itoa(0));
	return TRX_OK;
	#endif
}

TRX_STATUS getSMMainStreamTotalMPEG2TSStatsPacketDiscontinuityCounterBeforeCA(char **value)
{
	uint32 num_packets_discontinuity_count_before_ca=1;

	*value = strdup(itoa(num_packets_discontinuity_count_before_ca));
	return TRX_OK;
}

/**************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Total.RTPStats             */
/**************************************************************************/
TRX_STATUS getSMMainStreamTotalRTPStatsTotalSeconds(char **value)
{
	#if PLAYBACK_IP_SUPPORT

	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	b_total_t *total;
	struct timeval tv;
	unsigned currTime, totalTime;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec;

	getTotal(&total);

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
        if (info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eRtp)
        {
			totalTime = currTime - total->startTime;
			*value = strdup(itoa(totalTime));
        }
		else
			*value = strdup(itoa(0));
		return TRX_OK;
    }
	else
		return TRX_ERR;

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getSMMainStreamTotalRTPStatsPacketsExpected(char **value)
{
	uint32 packets_expected=1;

	*value = strdup(itoa(packets_expected));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsPacketsDiscarded(char **value)
{
#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
	void *send_data = &tr69c_data;
	int send_data_len = sizeof(struct b_tr69c_data);
	union b_tr69c_info info;
	void *recv_data = &info;
	int recv_data_len = sizeof(union b_tr69c_info);
    b_tr69c_client_registry_t client_registry;
    int i, rc;
	b_sampleRTP_t *sample;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
		static char tmp[10];
		const char *sep = " ,";

		getSampleRTP(&sample);
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        *value = strdup(itoa(info.playback_ip_status.rtpStats.packetsDiscarded));

		if (sample->packetsDiscarded[0] == '\0')
			BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.playback_ip_status.rtpStats.packetsDiscarded);
		else
			BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.playback_ip_status.rtpStats.packetsDiscarded);

		if ((strlen(sample->packetsDiscarded) + strlen(tmp)) <= 256)
			strncat(sample->packetsDiscarded, tmp, strlen(tmp));
		else
		{
			BKNI_Memset(sample->packetsDiscarded, 0, sizeof(sample->packetsDiscarded));
			strncat(sample->packetsDiscarded, tmp, strlen(tmp));
		}
		setSampleRTP(sample);
    }
    else
    {
        *value = strdup(itoa(0));
    }
#else
	uint32 packets_discarded=1;
	*value = strdup(itoa(packets_discarded));
#endif
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsPacketsOutOfSequence(char **value)
{
#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
	void *send_data = &tr69c_data;
	int send_data_len = sizeof(struct b_tr69c_data);
	union b_tr69c_info info;
	void *recv_data = &info;
	int recv_data_len = sizeof(union b_tr69c_info);
    b_tr69c_client_registry_t client_registry;
    int i, rc;
	b_sampleRTP_t *sample;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
		static char tmp[10];
		const char *sep = " ,";

		getSampleRTP(&sample);
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        *value = strdup(itoa(info.playback_ip_status.rtpStats.packetsOutOfSequence));

		if (sample->packetsOutOfSequence[0] == '\0')
			BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.playback_ip_status.rtpStats.packetsOutOfSequence);
		else
			BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.playback_ip_status.rtpStats.packetsOutOfSequence);

		if ((strlen(sample->packetsOutOfSequence) + strlen(tmp)) <= 256)
			strncat(sample->packetsOutOfSequence, tmp, strlen(tmp));
		else
		{
			BKNI_Memset(sample->packetsOutOfSequence, 0, sizeof(sample->packetsOutOfSequence));
			strncat(sample->packetsOutOfSequence, tmp, strlen(tmp));
		}
		setSampleRTP(sample);
    }
    else
    {
        *value = strdup(itoa(0));
    }
#else
	uint32 packets_out_of_sequence=1;
	*value = strdup(itoa(packets_out_of_sequence));
#endif
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsPacketsReceived(char **value)
{
#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
	void *send_data = &tr69c_data;
	int send_data_len = sizeof(struct b_tr69c_data);
	union b_tr69c_info info;
	void *recv_data = &info;
	int recv_data_len = sizeof(union b_tr69c_info);
    b_tr69c_client_registry_t client_registry;
    int i, rc;
	b_sampleRTP_t *sample;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
		static char tmp[10];
		const char *sep = " ,";

		getSampleRTP(&sample);
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        *value = strdup(itoa(info.playback_ip_status.rtpStats.packetsReceived));

		if (sample->packetsReceived[0] == '\0')
			BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.playback_ip_status.rtpStats.packetsReceived);
		else
			BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.playback_ip_status.rtpStats.packetsReceived);

		if ((strlen(sample->packetsReceived) + strlen(tmp)) <= 256)
			strncat(sample->packetsReceived, tmp, strlen(tmp));
		else
		{
			BKNI_Memset(sample->packetsReceived, 0, sizeof(sample->packetsReceived));
			strncat(sample->packetsReceived, tmp, strlen(tmp));
		}
		setSampleRTP(sample);
    }
    else
    {
        *value = strdup(itoa(0));
    }
#else
	uint32 packets_received=1;
	*value = strdup(itoa(packets_received));
#endif
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsPacketsReceivedBeforeEC(char **value)
{
    uint32 packets_received_before_ec=1;

#if 0 /* EXAMPLE */
	struct b_tr69c_data tr69c_data;
	void *send_data = &tr69c_data;
	int send_data_len = sizeof(struct b_tr69c_data);
    b_tr69c_client_registry_t client_registry;
    int i;
    static bool audio_mute = false;

    audio_mute = !audio_mute;

    b_tr69c_get_client_registry(&client_registry);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "live") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_set_audio_decoder_mute;
        tr69c_data.info.audio_decoder_mute = audio_mute;
        b_tr69c_server_set_client(client_registry[i].client, send_data, send_data_len);

        tr69c_data.type = b_tr69c_type_set_video_decoder_mute;
        tr69c_data.info.video_decoder_mute = true;
        b_tr69c_server_set_client(client_registry[i].client, send_data, send_data_len);

        BKNI_Sleep(2500);

        tr69c_data.type = b_tr69c_type_set_video_decoder_mute;
        tr69c_data.info.video_decoder_mute = false;
        b_tr69c_server_set_client(client_registry[i].client, send_data, send_data_len);
    }
#endif

    *value = strdup(itoa(packets_received_before_ec));

	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsPacketsLost(char **value)
{
#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
	void *send_data = &tr69c_data;
	int send_data_len = sizeof(struct b_tr69c_data);
	union b_tr69c_info info;
	void *recv_data = &info;
	int recv_data_len = sizeof(union b_tr69c_info);
    b_tr69c_client_registry_t client_registry;
    int i, rc;
	b_sampleRTP_t *sample;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
		static char tmp[10];
		const char *sep = " ,";

		getSampleRTP(&sample);
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        *value = strdup(itoa(info.playback_ip_status.rtpStats.packetsLost));

		if (sample->packetsLost[0] == '\0')
			BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.playback_ip_status.rtpStats.packetsLost);
		else
			BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.playback_ip_status.rtpStats.packetsLost);

		if ((strlen(sample->packetsLost) + strlen(tmp)) <= 256)
			strncat(sample->packetsLost, tmp, strlen(tmp));
		else
		{
			BKNI_Memset(sample->packetsLost, 0, sizeof(sample->packetsLost));
			strncat(sample->packetsLost, tmp, strlen(tmp));
		}
		setSampleRTP(sample);
    }
    else
    {
        *value = strdup(itoa(0));
    }
#else
	uint32 packets_lost=1;
	*value = strdup(itoa(packets_lost));
#endif
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsPacketsLostBeforeEC(char **value)
{
#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
	void *send_data = &tr69c_data;
	int send_data_len = sizeof(struct b_tr69c_data);
	union b_tr69c_info info;
	void *recv_data = &info;
	int recv_data_len = sizeof(union b_tr69c_info);
    b_tr69c_client_registry_t client_registry;
    int i, rc;
	b_sampleRTP_t *sample;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
		static char tmp[10];
		const char *sep = " ,";

		getSampleRTP(&sample);
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        *value = strdup(itoa(info.playback_ip_status.rtpStats.packetsLostBeforeErrorCorrection));

		if (sample->packetsLostBeforeEC[0] == '\0')
			BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.playback_ip_status.rtpStats.packetsLostBeforeErrorCorrection);
		else
			BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.playback_ip_status.rtpStats.packetsLostBeforeErrorCorrection);

		if ((strlen(sample->packetsLostBeforeEC) + strlen(tmp)) <= 256)
			strncat(sample->packetsLostBeforeEC, tmp, strlen(tmp));
		else
		{
			BKNI_Memset(sample->packetsLostBeforeEC, 0, sizeof(sample->packetsLostBeforeEC));
			strncat(sample->packetsLostBeforeEC, tmp, strlen(tmp));
		}
		setSampleRTP(sample);
    }
    else
    {
        *value = strdup(itoa(0));
    }
#else
	uint32 packets_lost_before_ec=1;
	*value = strdup(itoa(packets_lost_before_ec));
#endif
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsPacketsLostByEventHistogram(char **value)
{
#if 0 /* EXAMPLE */
        struct b_tr69c_data tr69c_data;
        void *send_data = &tr69c_data;
        int send_data_len = sizeof(struct b_tr69c_data);
        union b_tr69c_info info;
        void *recv_data = &info;
        int recv_data_len = sizeof(union b_tr69c_info);
        b_tr69c_client_registry_t client_registry;
        int i;

        b_tr69c_get_client_registry(&client_registry);

        for (i = 0; i < tr69c_server->num_client; i++)
        if (client_registry[i].client)
        {
            NEXUS_VideoCodec videoCodec;
            NEXUS_VideoProtocolProfile protocolProfile;
            NEXUS_VideoProtocolLevel protocolLevel;

            tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
            b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
            videoCodec = info.video_start_settings.codec;

            tr69c_data.type = b_tr69c_type_get_video_decoder_status;
            b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
            protocolProfile = info.video_decoder_status.protocolProfile;
            protocolLevel = info.video_decoder_status.protocolLevel;

            printf("*****************************************\n");
            printf("**** Video Codec: %d, Profile: %d, Level: %d *****\n", videoCodec, protocolProfile, protocolLevel);
            printf("*****************************************\n");
        }
#endif
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsPacketsLostByEventHistogramBeforeEC(char **value)
{
#if 0 /* EXAMPLE */
    struct b_tr69c_data tr69c_data;
    void *send_data = &tr69c_data;
    int send_data_len = sizeof(struct b_tr69c_data);
    b_tr69c_client_registry_t client_registry;
    int i;

    b_tr69c_get_client_registry(&client_registry);

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
        tr69c_data.type = b_tr69c_type_set_video_decoder_mute;
        tr69c_data.info.video_decoder_mute = true;
        b_tr69c_server_set_client(client_registry[i].client, send_data, send_data_len);

        BKNI_Sleep(2500);

        tr69c_data.type = b_tr69c_type_set_video_decoder_mute;
        tr69c_data.info.video_decoder_mute = false;
        b_tr69c_server_set_client(client_registry[i].client, send_data, send_data_len);
    }
#endif

	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsLossEvents(char **value)
{
#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
	void *send_data = &tr69c_data;
	int send_data_len = sizeof(struct b_tr69c_data);
	union b_tr69c_info info;
	void *recv_data = &info;
	int recv_data_len = sizeof(union b_tr69c_info);
    b_tr69c_client_registry_t client_registry;
    int i, rc;
	b_sampleRTP_t *sample;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
		static char tmp[10];
		const char *sep = " ,";

		getSampleRTP(&sample);
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        *value = strdup(itoa(info.playback_ip_status.rtpStats.lossEvents));

		if (sample->lossEvents[0] == '\0')
			BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.playback_ip_status.rtpStats.lossEvents);
		else
			BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.playback_ip_status.rtpStats.lossEvents);

		if ((strlen(sample->lossEvents) + strlen(tmp)) <= 256)
			strncat(sample->lossEvents, tmp, strlen(tmp));
		else
		{
			BKNI_Memset(sample->lossEvents, 0, sizeof(sample->lossEvents));
			strncat(sample->lossEvents, tmp, strlen(tmp));
		}
		setSampleRTP(sample);
    }
    else
    {
        *value = strdup(itoa(0));
    }
#else
	uint32 loss_events=1;
	*value = strdup(itoa(loss_events));
#endif
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsLossEventsBeforeEC(char **value)
{
#if PLAYBACK_IP_SUPPORT
	struct b_tr69c_data tr69c_data;
	void *send_data = &tr69c_data;
	int send_data_len = sizeof(struct b_tr69c_data);
	union b_tr69c_info info;
	void *recv_data = &info;
	int recv_data_len = sizeof(union b_tr69c_info);
    b_tr69c_client_registry_t client_registry;
    int i, rc;
	b_sampleRTP_t *sample;

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_ERR;

    for (i = 0; i < tr69c_server->num_client; i++)
    {
        if (client_registry[i].client && strcmp(client_registry[i].client->name, "play") == 0)
        {
            break;
        }
    }

    if (client_registry[i].client)
    {
		static char tmp[10];
		const char *sep = " ,";

		getSampleRTP(&sample);
        tr69c_data.type = b_tr69c_type_get_playback_ip_status;
        b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
        *value = strdup(itoa(info.playback_ip_status.rtpStats.lossEventsBeforeErrorCorrection));

		if (sample->lossEventsBeforeEC[0] == '\0')
			BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.playback_ip_status.rtpStats.lossEventsBeforeErrorCorrection);
		else
			BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.playback_ip_status.rtpStats.lossEventsBeforeErrorCorrection);

		if ((strlen(sample->lossEventsBeforeEC) + strlen(tmp)) <= 256)
			strncat(sample->lossEventsBeforeEC, tmp, strlen(tmp));
		else
		{
			BKNI_Memset(sample->lossEventsBeforeEC, 0, sizeof(sample->lossEventsBeforeEC));
			strncat(sample->lossEventsBeforeEC, tmp, strlen(tmp));
		}
		setSampleRTP(sample);
    }
    else
    {
        *value = strdup(itoa(0));
    }
#else
	uint32 loss_events_before_ec=1;
	*value = strdup(itoa(loss_events_before_ec));
#endif
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsDelayBetweenLossEventsHistogram(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsDelayBetweenLossEventsHistogramBeforeEC(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsSevereLossIndexCount(char **value)
{
	uint32 severe_loss_index_count=1;

	*value = strdup(itoa(severe_loss_index_count));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsSevereLossIndexCountBeforeEC(char **value)
{
	uint32 severe_loss_index_count_before_ec=1;

	*value = strdup(itoa(severe_loss_index_count_before_ec));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsSevereLossLengthCount(char **value)
{
	uint32 severe_loss_length_count=1;

	*value = strdup(itoa(severe_loss_length_count));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsSevereLossLengthCountBeforeEC(char **value)
{
	uint32 severe_loss_length_count_before_ec=1;

	*value = strdup(itoa(severe_loss_length_count_before_ec));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsDurationSevereLossEventsHistogram(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsDurationSevereLossEventsHistogramBeforeEC(char **value)
{
	*value = strdup("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalRTPStatsRetransmitTimeouts(char **value)
{
	uint32 retransmit_timeouts=1;

	*value = strdup(itoa(retransmit_timeouts));
	return TRX_OK;
}

/**************************************************************************/
/* STBService.ServiceMonitoring.MainStream.Total.TCPStats             */
/**************************************************************************/
TRX_STATUS getSMMainStreamTotalTCPStatsTotalSeconds(char **value)
{
	#if PLAYBACK_IP_SUPPORT

	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	b_total_t *total;
	struct timeval tv;
	unsigned currTime, totalTime;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec;

	getTotal(&total);

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
        if (info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttp ||
			info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttps)
        {
			totalTime = currTime - total->startTime;
			*value = strdup(itoa(totalTime));
        }
		else
			*value = strdup(itoa(0));
		return TRX_OK;
    }
	else
		return TRX_ERR;

	#endif

	return TRX_INVALID_PARAM_TYPE;
}

TRX_STATUS getSMMainStreamTotalTCPStatsPacketsReceived(char **value)
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
        if (info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttp ||
			info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttps)
        {
			*value = strdup(itoa(info.playback_ip_status.totalConsumed/TS_PACKET_SIZE));
        }
		else
			*value = strdup(itoa(0));
		return TRX_OK;
    }
	else
		return TRX_ERR;
	#endif

	return TRX_INVALID_PARAM_TYPE;
}
TRX_STATUS getSMMainStreamTotalTCPStatsPacketsRetransmitted(char **value)
{
	uint32 packets_retransmitted=1;

	*value = strdup(itoa(packets_retransmitted));
	return TRX_OK;
}
TRX_STATUS getSMMainStreamTotalTCPStatsBytesReceived(char **value)
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
        if (info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttp ||
			info.playback_ip_status.sessionInfo.protocol == B_PlaybackIpProtocol_eHttps)
        {
			*value = strdup(itoa(info.playback_ip_status.totalConsumed));
        }
		else
			*value = strdup(itoa(0));
		return TRX_OK;
    }
	else
		return TRX_ERR;
	#endif

	return TRX_INVALID_PARAM_TYPE;
}

/**************************************************************************/
/*  STBService.ServiceMonitoring.MainStream.TotalDejitteringStats        */
/**************************************************************************/
TRX_STATUS getSMMainStreamTotalDejitteringStatsTotalSeconds(char **value)
{
	b_total_t *total;
	struct timeval tv;
	unsigned currTime, totalTime;

	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec;

	getTotal(&total);
	totalTime = currTime - total->startTime;
	*value = strdup(itoa(totalTime));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalDejitteringStatsEmptyBufferTime(char **value)
{
	uint32 empty_buffer_time=1;

	*value = strdup(itoa(empty_buffer_time));
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalDejitteringStatsOverruns(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_sampleDejittering_t *sample;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
			static char tmp[10];
			const char *sep = ", ";

			getSampleDejittering(&sample);
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				tr69c_data.type = b_tr69c_type_get_video_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				*value = strdup(itoa(info.video_decoder_status.numDecodeOverflows));

				if (sample->overruns[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.video_decoder_status.numDecodeOverflows);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.video_decoder_status.numDecodeOverflows);

				if ((strlen(sample->overruns) + strlen(tmp)) <= 256)
					strncat(sample->overruns, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->overruns, 0, sizeof(sample->overruns));
					strncat(sample->overruns, tmp, strlen(tmp));
				}
				setSampleDejittering(sample);
			}
			else
				*value = strdup(itoa(0));
			rc = TRX_OK;
	    }
		else
			rc = TRX_INVALID_ARGUMENTS;
	}
	return rc;
}

TRX_STATUS getSMMainStreamTotalDejitteringStatsUnderruns(char **value)
{
	struct b_tr69c_data tr69c_data;
    union b_tr69c_info info;
	b_tr69c_client_registry_t client_registry;
	int rc, num_of_entries, i;
	void *send_data, *recv_data;
	int send_data_len, recv_data_len;
	b_sampleDejittering_t *sample;

    send_data = &tr69c_data;
    send_data_len = sizeof(struct b_tr69c_data);
    recv_data = &info;
    recv_data_len = sizeof(union b_tr69c_info);

    rc = b_tr69c_get_client_registry(&client_registry);
	if (rc != BERR_SUCCESS) return TRX_REQUEST_DENIED;

	b_tr69c_get_client_count(&num_of_entries);

	for (i = 0; i < num_of_entries; i++)
	{
	    if (client_registry[i].client)
	    {
		    tr69c_data.type = b_tr69c_type_get_video_decoder_start_settings;
			b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
			if(info.video_start_settings.videoWindowType == NxClient_VideoWindowType_eMain)
			{
				static char tmp[10];
				const char *sep = ", ";

				getSampleDejittering(&sample);
				tr69c_data.type = b_tr69c_type_get_video_decoder_status;
				b_tr69c_server_get_client(client_registry[i].client, send_data, send_data_len, recv_data, recv_data_len);
				*value = strdup(itoa(info.video_decoder_status.numDisplayUnderflows));

				if (sample->overruns[0] == '\0')
					BKNI_Snprintf(tmp, sizeof(tmp), "%d", info.video_decoder_status.numDecodeOverflows);
				else
					BKNI_Snprintf(tmp, sizeof(tmp), "%s%d", sep, info.video_decoder_status.numDecodeOverflows);

				if ((strlen(sample->overruns) + strlen(tmp)) <= 256)
					strncat(sample->overruns, tmp, strlen(tmp));
				else
				{
					BKNI_Memset(sample->overruns, 0, sizeof(sample->overruns));
					strncat(sample->overruns, tmp, strlen(tmp));
				}
				setSampleDejittering(sample);
			}
			else
				*value = strdup(itoa(0));
			rc = TRX_OK;
	    }
		else
			rc = TRX_INVALID_ARGUMENTS;
	}
	return rc;
}

/*********************************************************/
/* STBService.ServiceMonitoring.MainStream.Total Object */
/*********************************************************/
TRX_STATUS getSMMainStreamTotalTotalSeconds(char **value)
{
	b_total_t *total;
	struct timeval tv;
	unsigned currTime;

	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec;

	getTotal(&total);
	total->totalSeconds = currTime - total->startTime;
	setTotal(total);
	*value = strdup(itoa(total->totalSeconds));
	return TRX_OK;
}


TRX_STATUS setSMMainStreamTotalReset(char *value)
{
	b_total_t *total;
	struct timeval tv;
	unsigned currTime;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	getTotal(&total);
	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec;

	if (value !=0)
	{
		total->reset = true;
		total->startTime = currTime;
		setTotal(total);
	}
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalReset(char **value)
{
	b_total_t *total;

	getTotal(&total);

	if (total->reset)
		*value = strdup("1");
	else
		*value = strdup("0");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamTotalResetTime(char **value)
{
	b_total_t *total;
	struct timeval tv;
	unsigned currTime;

	gettimeofday(&tv, NULL);
	currTime = tv.tv_sec;

	getTotal(&total);
	total->resetTime = currTime - total->startTime;
	setTotal(total);
	*value = strdup(itoa(total->resetTime));
	return TRX_OK;
}

/**********************************************************/
/* STBService.ServiceMonitoring.MainStream Object */
/**********************************************************/
TRX_STATUS setSMMainStreamEnable(char *value)
{
	b_mainStream_t *mainStream;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	getMainStream(&mainStream);

	if (value != 0)
	{
		mainStream->enable = true;
		setMainStream(mainStream);
	}
	else
	{
		mainStream->enable = false;
		setMainStream(mainStream);
	}
	return TRX_OK;
}

TRX_STATUS getSMMainStreamEnable(char **value)
{
	b_mainStream_t *mainStream;

	getMainStream(&mainStream);

	if (mainStream->enable)
		*value = strdup("1");
	else
		*value = strdup("0");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamStatus(char **value)
{
	b_mainStream_t *mainStream;

	getMainStream(&mainStream);

	if (mainStream->status == disabled)
		*value = strdup("disabled");
	else if (mainStream->status == enabled)
		*value = strdup("enabled");
	else
		*value = strdup("error");
	return TRX_OK;
}

TRX_STATUS setSMMainStreamAlias(char *value)
{
	b_mainStream_t *mainStream;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	getMainStream(&mainStream);
	BKNI_Snprintf(mainStream->alias, strlen(value), value);
	setMainStream(mainStream);

	return TRX_OK;
}

TRX_STATUS getSMMainStreamAlias(char **value)
{
	b_mainStream_t *mainStream;

	getMainStream(&mainStream);
	*value = strdup(mainStream->alias);
	return TRX_OK;
}

TRX_STATUS setSMMainStreamServiceType(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getSMMainStreamServiceType(char **value)
{
	*value = strdup("IPTV");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamAVStream(char **value)
{
	*value = strdup(".AVStreams.AVStream.2");
	return TRX_OK;
}

TRX_STATUS setSMMainStreamGmin(char *value)
{
	uint32 gmin;
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	gmin = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMMainStreamGmin(char **value)
{
	uint32 gmin=1;

	*value = strdup(itoa(gmin));
	return TRX_OK;
}

TRX_STATUS setSMMainStreamSevereLossMinDistance(char *value)
{
	uint32 min_dist_severe_loss;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	min_dist_severe_loss = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMMainStreamSevereLossMinDistance(char **value)
{
	uint32 min_dist_severe_loss=1;

	*value = strdup(itoa(min_dist_severe_loss));
	return TRX_OK;
}

TRX_STATUS setSMMainStreamSevereLossMinLength(char *value)
{
	uint32 min_len_severe_loss;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	min_len_severe_loss = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMMainStreamSevereLossMinLength(char **value)
{
	uint32 min_len_severe_loss=1;

	*value = strdup(itoa(min_len_severe_loss));
	return TRX_OK;
}

TRX_STATUS setSMMainStreamChannelChangeFailureTimeout(char *value)
{
	uint32 min_len_severe_loss;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	min_len_severe_loss = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMMainStreamChannelChangeFailureTimeout(char **value)
{
	uint32 min_len_severe_loss=1;

	*value = strdup(itoa(min_len_severe_loss));
	return TRX_OK;
}

TRX_STATUS setSMMainStreamPacketsLostByEventHistogramIntervals(char *value)
{
	uint32 packets_lost_by_event_histogram_intervals;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	packets_lost_by_event_histogram_intervals = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMMainStreamPacketsLostByEventHistogramIntervals(char **value)
{
	uint32 packets_lost_by_event_histogram_intervals=1;

	*value = strdup(itoa(packets_lost_by_event_histogram_intervals));
	return TRX_OK;
}

TRX_STATUS setSMMainStreamDelayBetweenLossEventsHistogramIntervals(char *value)
{
	uint32 delay_between_loss_events_histogram_intervals;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	delay_between_loss_events_histogram_intervals = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMMainStreamDelayBetweenLossEventsHistogramIntervals(char **value)
{
	uint32 delay_between_loss_events_histogram_intervals=1;

	*value = strdup(itoa(delay_between_loss_events_histogram_intervals));
	return TRX_OK;
}

TRX_STATUS setSMMainStreamDurationSevereLossEventsHistogramIntervals(char *value)
{
	uint32 duration_severe_loss_events_histogram_intervals;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	duration_severe_loss_events_histogram_intervals = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMMainStreamDurationSevereLossEventsHistogramIntervals(char **value)
{
	uint32 duration_severe_loss_events_histogram_intervals=1;

	*value = strdup(itoa(duration_severe_loss_events_histogram_intervals));
	return TRX_OK;
}

/**********************************************************/
/* STBService.ServiceMonitoringGlobalOperation.Sample Object                    */
/**********************************************************/
TRX_STATUS getSMGlobalOperationSampleMinimumPortalResponse(char **value)
{
	*value = strdup("");
	return TRX_OK;
}

TRX_STATUS getSMGlobalOperationSampleMaximumPortalResponse(char **value)
{
	*value = strdup("");
	return TRX_OK;
}

TRX_STATUS getSMGlobalOperationSamplePortalResponse(char **value)
{
	*value = strdup(",837:453:1234,759,,,923:698,,,1284");
	return TRX_OK;
}

/**********************************************************/
/* STBService.ServiceMonitoring.GlobalOperation.Total Object                    */
/**********************************************************/

TRX_STATUS getSMGlobalOperationTotalServiceAccessTime(char **value)
{
	uint32 service_access_time=1;

	*value = strdup(itoa(service_access_time));
	return TRX_OK;
}

TRX_STATUS getSMGlobalOperationTotalMinimumPortalResponse(char **value)
{
	uint32 min_portal_response=1;

	*value = strdup(itoa(min_portal_response));
	return TRX_OK;
}

TRX_STATUS getSMGlobalOperationTMaximumPortalResponse(char **value)
{
	uint32 max_portal_response=1;

	*value = strdup(itoa(max_portal_response));
	return TRX_OK;
}

/**********************************************************/
/* STBService.ServiceMonitoring Object                    */
/**********************************************************/
TRX_STATUS setSMSampleEnable(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getSMSampleEnable(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getSMSampleState(char **value)
{
	*value = strdup("Enabled");
	return TRX_OK;
}

TRX_STATUS setSMSampleInterval(char *value)
{
	uint32 smaple_interval;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	smaple_interval = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMSampleInterval(char **value)
{
	uint32 smaple_interval=1;

	*value = strdup(itoa(smaple_interval));
	return TRX_OK;
}

TRX_STATUS setSMReportSamples(char *value)
{
	uint32 num_report_smaple;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	num_report_smaple = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMReportSamples(char **value)
{
	uint32 num_report_smaple=1;

	*value = strdup(itoa(num_report_smaple));
	return TRX_OK;
}

TRX_STATUS setSMEventsPerSampleInterval(char *value)
{
	uint32 event_per_sample_interval;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	event_per_sample_interval = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMEventsPerSampleInterval(char **value)
	{
		uint32 event_per_sample_interval=1;

		*value = strdup(itoa(event_per_sample_interval));
		return TRX_OK;
	}


TRX_STATUS setSMFetchSamples(char *value)
{
	uint32 num_fetch_smaple;

	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */
	num_fetch_smaple = atoi(value);

	return TRX_OK;
}

TRX_STATUS getSMFetchSamples(char **value)
{
	uint32 num_fetch_smaple=1;

	*value = strdup(itoa(num_fetch_smaple));
	return TRX_OK;
}


TRX_STATUS setSMTimeReference(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getSMTimeReference(char **value)
{
	*value = strdup ("Temp");
	return TRX_OK;
}

TRX_STATUS setSMForceSample(char *value)
{
	if ( value == NULL ) return TRX_INVALID_PARAM_VALUE;

	/* Store the value */

	return TRX_OK;
}

TRX_STATUS getSMForceSample(char **value)
{
	*value = strdup(("1"));
	return TRX_OK;
}

TRX_STATUS getSMReportStartTime(char **value)
{
	*value = strdup ("Temp");
	return TRX_OK;
}

TRX_STATUS getSMReportEndTime(char **value)
{
	*value = strdup ("Temp");
	return TRX_OK;
}

TRX_STATUS getSMMainStreamNumberOfEntries(char **value)
{
	uint32 num_entries=1;

	*value = strdup (itoa(num_entries));
	return TRX_OK;
}

