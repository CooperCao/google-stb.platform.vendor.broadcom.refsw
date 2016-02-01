/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "sharedparams.h"
#include "stbservicemonitoringparams.h" /* profiles for parameter callbacks */

/* STBService.ServiceMonitoring.MainStream.Sample.HighLevelMetricStats Object */
TRXSFUNC(setSMMainStreamSampleHighLevelMetricStatsEnable);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsEnable);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsStatus);
TRXSFUNC(setSMMainStreamSampleHighLevelMetricStatsAlias);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsAlias);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetricName);
TRXSFUNC(setSMMainStreamSampleHighLevelMetricStatsMetric1Threshold);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetric1Threshold);
TRXSFUNC(setSMMainStreamSampleHighLevelMetricStatsMetric2Threshold);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetric2Threshold);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsSampleSeconds);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetric1);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetric2);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetric1Failures);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetric2Failures);
TRXSFUNC(setSMMainStreamSampleHighLevelMetricStatsMetricSampleInterval);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetricSampleInterval);
TRXSFUNC(setSMMainStreamSampleHighLevelMetricStatsMetricThreshold);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetricThreshold);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetric);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsMetricFailures);

/* STBService.ServiceMonitoring.MainStream.SampleVideoResponseStats Object */
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsSampleSeconds);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsAverageVideoSystemResponse);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsMinimumVideoSystemResponse);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsMaximumVideoSystemResponse);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsAverageVoDControlResponse);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsMinimumVoDControlResponse);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsMaximumVoDControlResponse);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsVoDControlResponse);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsVideoSystemResponse);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsRequestedTransactions);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsAccessSuccesses);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsCompletionCount);
TRXGFUNC(getSMMainStreamSampleVideoResponseStatsChannelChangeFailures);

/* STBService.ServiceMonitoring.MainStream.Sample.AudioDecoderStats */
TRXGFUNC(getSMMainStreamSampleAudioDecoderStatsTotalSeconds);
TRXGFUNC(getSMMainStreamSampleAudioDecoderStatsDecodedFrames);
TRXGFUNC(getSMMainStreamSampleAudioDecoderStatsDecodingErrors);

/* STBService.ServiceMonitoring.MainStream.Sample.VideoDecoderStats */
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsTotalSeconds);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsFrameRate);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsDecodedFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsLostFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsConcealedFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsIDecodedFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsILostFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsIConcealedFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsPDecodedFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsPLostFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsPConcealedFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsBDecodedFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsBLostFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsBConcealedFrames);
TRXGFUNC(getSMMainStreamSampleVideoDecoderStatsAVResynchCounter);

/* STBService.ServiceMonitoring.MainStream.Sample.MPEG2TSStats */
TRXGFUNC(getSMMainStreamSampleMPEG2TSStatsSampleSeconds);
TRXGFUNC(getSMMainStreamSampleMPEG2TSStatsTSPacketsReceived);
TRXGFUNC(getSMMainStreamSampleMPEG2TSStatsTSPacketsDrained);
TRXGFUNC(getSMMainStreamSampleMPEG2TSStatsTSSyncByteErrorCount);
TRXGFUNC(getSMMainStreamSampleMPEG2TSStatsTSSyncLossCount);
TRXGFUNC(getSMMainStreamSampleMPEG2TSStatsPacketDiscontinuityCounter);
TRXGFUNC(getSMMainStreamSampleMPEG2TSStatsPacketDiscontinuityCounterBeforeCA);

/* STBService.ServiceMonitoring.MainStream.Sample.RTPStats */
TRXGFUNC(getSMMainStreamSampleRTPStatsSampleSeconds);
TRXGFUNC(getSMMainStreamSampleRTPStatsPacketsExpected);
TRXGFUNC(getSMMainStreamSampleRTPStatsPacketsDiscarded);
TRXGFUNC(getSMMainStreamSampleRTPStatsPacketsOutOfSequence);
TRXGFUNC(getSMMainStreamSampleRTPStatsPacketsReceived);
TRXGFUNC(getSMMainStreamSampleRTPStatsPacketsReceivedBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsRetransmitTimeouts);
TRXGFUNC(getSMMainStreamSampleRTPStatsPacketsLost);
TRXGFUNC(getSMMainStreamSampleRTPStatsPacketsLostBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsPacketsLostByEventHistogram);
TRXGFUNC(getSMMainStreamSampleRTPStatsPacketsLostByEventHistogramBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsLossEvents);
TRXGFUNC(getSMMainStreamSampleRTPStatsLossEventsBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsDelayBetweenLossEventsHistogram);
TRXGFUNC(getSMMainStreamSampleRTPStatsDelayBetweenLossEventsHistogramBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsSevereLossIndexCount);
TRXGFUNC(getSMMainStreamSampleRTPStatsSevereLossIndexCountBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsAverageLossDistance);
TRXGFUNC(getSMMainStreamSampleRTPStatsAverageLossDistanceBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsMinimumLossDistance);
TRXGFUNC(getSMMainStreamSampleRTPStatsMinimumLossDistanceBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsSevereLossLengthCount);
TRXGFUNC(getSMMainStreamSampleRTPStatsSevereLossLengthCountBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsDurationSevereLossEventsHistogram);
TRXGFUNC(getSMMainStreamSampleRTPStatsDurationSevereLossEventsHistogramBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsMaximumLossPeriod);
TRXGFUNC(getSMMainStreamSampleRTPStatsMaximumLossPeriodBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsAverageLossPeriod);
TRXGFUNC(getSMMainStreamSampleRTPStatsAverageLossPeriodBeforeEC);
TRXGFUNC(getSMMainStreamSampleRTPStatsMinimumLossPeriod);
TRXGFUNC(getSMMainStreamSampleRTPStatsMinimumLossPeriodBeforeEC);

/* STBService.ServiceMonitoring.MainStream.Sample.TCPStats */
TRXGFUNC(getSMMainStreamSampleTCPStatsSampleSeconds);
TRXGFUNC(getSMMainStreamSampleTCPStatsPacketsReceived);
TRXGFUNC(getSMMainStreamSampleTCPStatsPacketsRetransmitted);
TRXGFUNC(getSMMainStreamSampleTCPStatsBytesReceived);

/* STBService.ServiceMonitoring.MainStream.Sample.DejitteringStats */
TRXGFUNC(getSMMainStreamSampleDejitteringStatsSampleSeconds);
TRXGFUNC(getSMMainStreamSampleDejitteringStatsOverruns);
TRXGFUNC(getSMMainStreamSampleDejitteringStatsUnderruns);
TRXGFUNC(getSMMainStreamSampleDejitteringStatsEmptyBufferTime);

/* STBService.ServiceMonitoring.MainStream.Sample Object */
TRXGFUNC(getSMMainStreamSampleSampleSeconds);
TRXGFUNC(getSMMainStreamSampleSignificantChanges);
TRXGFUNC(getSMMainStreamSamplePVRTimeShift);
TRXGFUNC(getSMMainStreamSampleHighLevelMetricStatsNumberOfEntries);

/* STBService.ServiceMonitoring.MainStream.Total.VideoResponseStats Object */
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsTotalSeconds);
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsChannelChangeFailures);
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsChannelFailures);
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsMinimumVoDControlResponse);
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsMaximumVoDControlResponse);
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsRequestedTransactions);
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsAccessSuccesses);
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsCompletionCount);
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsMinimumVideoSystemResponse);
TRXGFUNC(getSMMainStreamTotalVideoResponseStatsMaximumVideoSystemResponse);

/* STBService.ServiceMonitoring.MainStream.Total.AudioDecoderStats Object */
TRXGFUNC(getSMMainStreamTotalAudioDecoderStatsTotalSeconds);
TRXGFUNC(getSMMainStreamTotalAudioDecoderStatsDecodedFrames);
TRXGFUNC(getSMMainStreamTotalAudioDecoderStatsDecodingErrors);

/* STBService.ServiceMonitoring.MainStream.Total.VideoDecoderStats Object */
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsTotalSeconds);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsFrameRate);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsDecodedFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsLostFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsConcealedFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsIDecodedFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsILostFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsIConcealedFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsPDecodedFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsPLostFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsPConcealedFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsBDecodedFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsBLostFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsBConcealedFrames);
TRXGFUNC(getSMMainStreamTotalVideoDecoderStatsAVResynchCounter);

/* STBService.ServiceMonitoring.MainStream.Total.MPEG2TSStats */
TRXGFUNC(getSMMainStreamTotalMPEG2TSStatsTotalSeconds);
TRXGFUNC(getSMMainStreamTotalMPEG2TSStatsTSPacketsReceived);
TRXGFUNC(getSMMainStreamTotalMPEG2TSStatsTSSyncByteErrorCount);
TRXGFUNC(getSMMainStreamTotalMPEG2TSStatsTSSyncLossCount);
TRXGFUNC(getSMMainStreamTotalMPEG2TSStatsPacketDiscontinuityCounter);
TRXGFUNC(getSMMainStreamTotalMPEG2TSStatsPacketDiscontinuityCounterBeforeCA);

/*  STBService.ServiceMonitoring.MainStream.Total.RTPStats */
TRXGFUNC(getSMMainStreamTotalRTPStatsTotalSeconds);
TRXGFUNC(getSMMainStreamTotalRTPStatsPacketsExpected);
TRXGFUNC(getSMMainStreamTotalRTPStatsPacketsDiscarded);
TRXGFUNC(getSMMainStreamTotalRTPStatsPacketsOutOfSequence);
TRXGFUNC(getSMMainStreamTotalRTPStatsPacketsReceived);
TRXGFUNC(getSMMainStreamTotalRTPStatsPacketsReceivedBeforeEC);
TRXGFUNC(getSMMainStreamTotalRTPStatsPacketsLost);
TRXGFUNC(getSMMainStreamTotalRTPStatsPacketsLostBeforeEC);
TRXGFUNC(getSMMainStreamTotalRTPStatsPacketsLostByEventHistogram);
TRXGFUNC(getSMMainStreamTotalRTPStatsPacketsLostByEventHistogramBeforeEC);
TRXGFUNC(getSMMainStreamTotalRTPStatsLossEvents);
TRXGFUNC(getSMMainStreamTotalRTPStatsLossEventsBeforeEC);
TRXGFUNC(getSMMainStreamTotalRTPStatsDelayBetweenLossEventsHistogram);
TRXGFUNC(getSMMainStreamTotalRTPStatsDelayBetweenLossEventsHistogramBeforeEC);
TRXGFUNC(getSMMainStreamTotalRTPStatsSevereLossIndexCount);
TRXGFUNC(getSMMainStreamTotalRTPStatsSevereLossIndexCountBeforeEC);
TRXGFUNC(getSMMainStreamTotalRTPStatsSevereLossLengthCount);
TRXGFUNC(getSMMainStreamTotalRTPStatsSevereLossLengthCountBeforeEC);
TRXGFUNC(getSMMainStreamTotalRTPStatsDurationSevereLossEventsHistogram);
TRXGFUNC(getSMMainStreamTotalRTPStatsDurationSevereLossEventsHistogramBeforeEC);
TRXGFUNC(getSMMainStreamTotalRTPStatsRetransmitTimeouts);

/*  STBService.ServiceMonitoring.MainStream.Total.TCPStats */
TRXGFUNC(getSMMainStreamTotalTCPStatsTotalSeconds);
TRXGFUNC(getSMMainStreamTotalTCPStatsPacketsReceived);
TRXGFUNC(getSMMainStreamTotalTCPStatsPacketsRetransmitted);
TRXGFUNC(getSMMainStreamTotalTCPStatsBytesReceived);

/*  STBService.ServiceMonitoring.MainStream.Total.DejitteringStats */
TRXGFUNC(getSMMainStreamTotalDejitteringStatsTotalSeconds);
TRXGFUNC(getSMMainStreamTotalDejitteringStatsEmptyBufferTime);
TRXGFUNC(getSMMainStreamTotalDejitteringStatsOverruns);
TRXGFUNC(getSMMainStreamTotalDejitteringStatsUnderruns);

/* STBService.ServiceMonitoring.MainStream.Total Object */
TRXGFUNC(getSMMainStreamTotalTotalSeconds);
TRXSFUNC(setSMMainStreamTotalReset);
TRXGFUNC(getSMMainStreamTotalReset);
TRXGFUNC(getSMMainStreamTotalResetTime);

/* STBService.ServiceMonitoring.MainStream Object */
TRXSFUNC(setSMMainStreamEnable);
TRXGFUNC(getSMMainStreamEnable);
TRXGFUNC(getSMMainStreamStatus);
TRXSFUNC(setSMMainStreamAlias);
TRXGFUNC(getSMMainStreamAlias);
TRXSFUNC(setSMMainStreamServiceType);
TRXGFUNC(getSMMainStreamServiceType);
TRXGFUNC(getSMMainStreamAVStream);
TRXSFUNC(setSMMainStreamGmin);
TRXGFUNC(getSMMainStreamGmin);
TRXSFUNC(setSMMainStreamSevereLossMinDistance);
TRXGFUNC(getSMMainStreamSevereLossMinDistance);
TRXSFUNC(setSMMainStreamSevereLossMinLength);
TRXGFUNC(getSMMainStreamSevereLossMinLength);
TRXSFUNC(setSMMainStreamChannelChangeFailureTimeout);
TRXGFUNC(getSMMainStreamChannelChangeFailureTimeout);
TRXSFUNC(setSMMainStreamPacketsLostByEventHistogramIntervals);
TRXGFUNC(getSMMainStreamPacketsLostByEventHistogramIntervals);
TRXSFUNC(setSMMainStreamDelayBetweenLossEventsHistogramIntervals);
TRXGFUNC(getSMMainStreamDelayBetweenLossEventsHistogramIntervals);
TRXSFUNC(setSMMainStreamDurationSevereLossEventsHistogramIntervals);
TRXGFUNC(getSMMainStreamDurationSevereLossEventsHistogramIntervals);

/* STBService.ServiceMonitoring.GlobalOperation.Total Object */
TRXGFUNC(getSMGlobalOperationTotalServiceAccessTime);
TRXGFUNC(getSMGlobalOperationTotalMinimumPortalResponse);
TRXGFUNC(getSMGlobalOperationTMaximumPortalResponse);

/* STBService.ServiceMonitoring.GlobalOperation.Sample Object */
TRXGFUNC(getSMGlobalOperationSampleMinimumPortalResponse);
TRXGFUNC(getSMGlobalOperationSampleMaximumPortalResponse);
TRXGFUNC(getSMGlobalOperationSamplePortalResponse);

/* STBService.ServiceMonitoring Object */
TRXSFUNC(setSMSampleEnable);
TRXGFUNC(getSMSampleEnable);
TRXGFUNC(getSMSampleState);
TRXSFUNC(setSMSampleInterval);
TRXGFUNC(getSMSampleInterval);
TRXSFUNC(setSMReportSamples);
TRXGFUNC(getSMReportSamples);
TRXSFUNC(setSMEventsPerSampleInterval);
TRXGFUNC(getSMEventsPerSampleInterval);
TRXSFUNC(setSMFetchSamples);
TRXGFUNC(getSMFetchSamples);
TRXSFUNC(setSMTimeReference);
TRXGFUNC(getSMTimeReference);
TRXSFUNC(setSMForceSample);
TRXGFUNC(getSMForceSample);
TRXGFUNC(getSMReportStartTime);
TRXGFUNC(getSMReportEndTime);
TRXGFUNC(getSMMainStreamNumberOfEntries);

TRxObjNode SMMainStreamSampleHighLevelMetricStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsEnable, getSMMainStreamSampleHighLevelMetricStatsEnable, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {Status, {{tString, 64, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsStatus, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {Alias, {{tString, 64, 0}}, setSMMainStreamSampleHighLevelMetricStatsAlias, getSMMainStreamSampleHighLevelMetricStatsAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {MetricName, {{tString, 256, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetricName, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Metric1Threshold, {{tUnsigned, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsMetric1Threshold, getSMMainStreamSampleHighLevelMetricStatsMetric1Threshold,NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Metric2Threshold, {{tUnsigned, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsMetric2Threshold, getSMMainStreamSampleHighLevelMetricStatsMetric2Threshold,NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SampleSeconds, {{tString, 256, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsSampleSeconds, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Metric1, {{tString, 256, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric1, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Metric2, {{tString, 256, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric2, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Metric1Failures, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric1Failures, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Metric2Failures, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric2Failures, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MetricSampleInterval, {{tUnsigned, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsMetricSampleInterval, getSMMainStreamSampleHighLevelMetricStatsMetricSampleInterval, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MetricThreshold, {{tUnsigned, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsMetricThreshold, getSMMainStreamSampleHighLevelMetricStatsMetricThreshold, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {Metric, {{tString, 64, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MetricFailures, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetricFailures, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{Enable, {{tBool, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsEnable, getSMMainStreamSampleHighLevelMetricStatsEnable, NULL, NULL},
	{Status, {{tString, 64, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setSMMainStreamSampleHighLevelMetricStatsAlias, getSMMainStreamSampleHighLevelMetricStatsAlias, NULL, NULL},
	{MetricName, {{tString, 256, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetricName, NULL, NULL},
	{Metric1Threshold, {{tUnsigned, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsMetric1Threshold, getSMMainStreamSampleHighLevelMetricStatsMetric1Threshold,NULL, NULL},
	{Metric2Threshold, {{tUnsigned, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsMetric2Threshold, getSMMainStreamSampleHighLevelMetricStatsMetric2Threshold,NULL, NULL},
	{SampleSeconds, {{tString, 256, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsSampleSeconds, NULL, NULL},
	{Metric1, {{tString, 256, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric1, NULL, NULL},
	{Metric2, {{tString, 256, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric2, NULL, NULL},
	{Metric1Failures, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric1Failures, NULL, NULL},
	{Metric2Failures, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric2Failures, NULL, NULL},
	{MetricSampleInterval, {{tUnsigned, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsMetricSampleInterval, getSMMainStreamSampleHighLevelMetricStatsMetricSampleInterval, NULL, NULL},
	{MetricThreshold, {{tUnsigned, 0, 0}}, setSMMainStreamSampleHighLevelMetricStatsMetricThreshold, getSMMainStreamSampleHighLevelMetricStatsMetricThreshold, NULL, NULL},
	{Metric, {{tString, 64, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetric, NULL, NULL},
	{MetricFailures, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsMetricFailures, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamSampleHighLevelMetricStatsInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK, {{0, 0, 0}}, NULL, NULL, SMMainStreamSampleHighLevelMetricStatsDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK, {{0, 0, 0}}, NULL, NULL, SMMainStreamSampleHighLevelMetricStatsDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamSampleVideoResponseStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsSampleSeconds, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {AverageVideoSystemResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsAverageVideoSystemResponse, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MinimumVideoSystemResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsMinimumVideoSystemResponse, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MaximumVideoSystemResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsMaximumVideoSystemResponse, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {AverageVoDControlResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsAverageVoDControlResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MinimumVoDControlResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsMinimumVoDControlResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MaximumVoDControlResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsMaximumVoDControlResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {VoDControlResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsVoDControlResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {VideoSystemResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsVideoSystemResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {RequestedTransactions, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsRequestedTransactions, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {AccessSuccesses, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsAccessSuccesses, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {CompletionCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsCompletionCount, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {ChannelChangeFailures, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsChannelChangeFailures, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsSampleSeconds, NULL, NULL},
	{AverageVideoSystemResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsAverageVideoSystemResponse, NULL, NULL},
	{MinimumVideoSystemResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsMinimumVideoSystemResponse, NULL, NULL},
	{MaximumVideoSystemResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsMaximumVideoSystemResponse, NULL, NULL},
	{AverageVoDControlResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsAverageVoDControlResponse, NULL, NULL},
	{MinimumVoDControlResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsMinimumVoDControlResponse, NULL, NULL},
	{MaximumVoDControlResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsMaximumVoDControlResponse, NULL, NULL},
	{VoDControlResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsVoDControlResponse, NULL, NULL},
	{VideoSystemResponse, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsVideoSystemResponse, NULL, NULL},
	{RequestedTransactions, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsRequestedTransactions, NULL, NULL},
	{AccessSuccesses, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsAccessSuccesses, NULL, NULL},
	{CompletionCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsCompletionCount, NULL, NULL},
	{ChannelChangeFailures, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoResponseStatsChannelChangeFailures, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamSampleAudioDecoderStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleAudioDecoderStatsTotalSeconds, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleAudioDecoderStatsDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DecodingErrors, {{tString, 64, 0}}, NULL, getSMMainStreamSampleAudioDecoderStatsDecodingErrors, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleAudioDecoderStatsTotalSeconds, NULL, NULL},
	{DecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleAudioDecoderStatsDecodedFrames, NULL, NULL},
	{DecodingErrors, {{tString, 64, 0}}, NULL, getSMMainStreamSampleAudioDecoderStatsDecodingErrors, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamSampleVideoDecoderStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsTotalSeconds, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {FrameRate, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsFrameRate, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {LostFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsLostFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ConcealedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsConcealedFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {IDecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsIDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ILostFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsILostFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {IConcealedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsIConcealedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PDecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsPDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PLostFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsPLostFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PConcealedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsPConcealedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {BDecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsBDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {BLostFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsBLostFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {BConcealedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsBConcealedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {AVResynchCounter, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsAVResynchCounter, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsTotalSeconds, NULL, NULL},
	{FrameRate, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsFrameRate, NULL, NULL},
	{DecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsDecodedFrames, NULL, NULL},
	{LostFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsLostFrames, NULL, NULL},
	{ConcealedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsConcealedFrames, NULL, NULL},
	{IDecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsIDecodedFrames, NULL, NULL},
	{ILostFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsILostFrames, NULL, NULL},
	{IConcealedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsIConcealedFrames, NULL, NULL},
	{PDecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsPDecodedFrames, NULL, NULL},
	{PLostFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsPLostFrames, NULL, NULL},
	{PConcealedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsPConcealedFrames, NULL, NULL},
	{BDecodedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsBDecodedFrames, NULL, NULL},
	{BLostFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsBLostFrames, NULL, NULL},
	{BConcealedFrames, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsBConcealedFrames, NULL, NULL},
	{AVResynchCounter, {{tString, 64, 0}}, NULL, getSMMainStreamSampleVideoDecoderStatsAVResynchCounter, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamSampleMPEG2TSStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsSampleSeconds, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {TSPacketsReceived, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsTSPacketsReceived, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {TSPacketsDrained, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsTSPacketsDrained, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {TSSyncByteErrorCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsTSSyncByteErrorCount, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {TSSyncLossCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsTSSyncLossCount, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PacketDiscontinuityCounter, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsPacketDiscontinuityCounter, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PacketDiscontinuityCounterBeforeCA, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsPacketDiscontinuityCounterBeforeCA, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsSampleSeconds, NULL, NULL},
	{TSPacketsReceived, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsTSPacketsReceived, NULL, NULL},
	{TSPacketsDrained, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsTSPacketsDrained, NULL, NULL},
	{TSSyncByteErrorCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsTSSyncByteErrorCount, NULL, NULL},
	{TSSyncLossCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsTSSyncLossCount, NULL, NULL},
	{PacketDiscontinuityCounter, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsPacketDiscontinuityCounter, NULL, NULL},
	{PacketDiscontinuityCounterBeforeCA, {{tString, 64, 0}}, NULL, getSMMainStreamSampleMPEG2TSStatsPacketDiscontinuityCounterBeforeCA, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamSampleRTPStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSampleSeconds, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsExpected, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsExpected, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PacketsDiscarded, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsDiscarded, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsOutOfSequence, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsOutOfSequence, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsReceived, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsReceived, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsReceivedBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsReceivedBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {RetransmitTimeouts, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsRetransmitTimeouts, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PacketsLost, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsLost, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsLostBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsLostBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsLostByEventHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsLostByEventHistogram, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {PacketsLostByEventHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsLostByEventHistogramBeforeEC, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {LossEvents, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsLossEvents, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {LossEventsBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsLossEventsBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DelayBetweenLossEventsHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsDelayBetweenLossEventsHistogram, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {DelayBetweenLossEventsHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsDelayBetweenLossEventsHistogramBeforeEC, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {SevereLossIndexCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSevereLossIndexCount, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SevereLossIndexCountBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSevereLossIndexCountBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {AverageLossDistance, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsAverageLossDistance, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {AverageLossDistanceBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsAverageLossDistanceBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MinimumLossDistance, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMinimumLossDistance, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MinimumLossDistanceBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMinimumLossDistanceBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SevereLossLengthCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSevereLossLengthCount, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SevereLossLengthCountBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSevereLossLengthCountBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {DurationSevereLossEventsHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsDurationSevereLossEventsHistogram, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {DurationSevereLossEventsHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsDurationSevereLossEventsHistogramBeforeEC, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {MaximumLossPeriod, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMaximumLossPeriod, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MaximumLossPeriodBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMaximumLossPeriodBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {AverageLossPeriod, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsAverageLossPeriod, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {AverageLossPeriodBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsAverageLossPeriodBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MinimumLossPeriod, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMinimumLossPeriod, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MinimumLossPeriodBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMinimumLossPeriodBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSampleSeconds, NULL, NULL},
	{PacketsExpected, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsExpected, NULL, NULL},
	{PacketsDiscarded, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsDiscarded, NULL, NULL},
	{PacketsOutOfSequence, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsOutOfSequence, NULL, NULL},
	{PacketsReceived, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsReceived, NULL, NULL},
	{PacketsReceivedBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsReceivedBeforeEC, NULL, NULL},
	{RetransmitTimeouts, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsRetransmitTimeouts, NULL, NULL},
	{PacketsLost, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsLost, NULL, NULL},
	{PacketsLostBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsLostBeforeEC, NULL, NULL},
	{PacketsLostByEventHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsLostByEventHistogram, NULL, NULL},
	{PacketsLostByEventHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsPacketsLostByEventHistogramBeforeEC, NULL, NULL},
	{LossEvents, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsLossEvents, NULL, NULL},
	{LossEventsBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsLossEventsBeforeEC, NULL, NULL},
	{DelayBetweenLossEventsHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsDelayBetweenLossEventsHistogram, NULL, NULL},
	{DelayBetweenLossEventsHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsDelayBetweenLossEventsHistogramBeforeEC, NULL, NULL},
	{SevereLossIndexCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSevereLossIndexCount, NULL, NULL},
	{SevereLossIndexCountBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSevereLossIndexCountBeforeEC, NULL, NULL},
	{AverageLossDistance, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsAverageLossDistance, NULL, NULL},
	{AverageLossDistanceBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsAverageLossDistanceBeforeEC, NULL, NULL},
	{MinimumLossDistance, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMinimumLossDistance, NULL, NULL},
	{MinimumLossDistanceBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMinimumLossDistanceBeforeEC, NULL, NULL},
	{SevereLossLengthCount, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSevereLossLengthCount, NULL, NULL},
	{SevereLossLengthCountBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsSevereLossLengthCountBeforeEC, NULL, NULL},
	{DurationSevereLossEventsHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsDurationSevereLossEventsHistogram, NULL, NULL},
	{DurationSevereLossEventsHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsDurationSevereLossEventsHistogramBeforeEC, NULL, NULL},
	{MaximumLossPeriod, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMaximumLossPeriod, NULL, NULL},
	{MaximumLossPeriodBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMaximumLossPeriodBeforeEC, NULL, NULL},
	{AverageLossPeriod, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsAverageLossPeriod, NULL, NULL},
	{AverageLossPeriodBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsAverageLossPeriodBeforeEC, NULL, NULL},
	{MinimumLossPeriod, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMinimumLossPeriod, NULL, NULL},
	{MinimumLossPeriodBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamSampleRTPStatsMinimumLossPeriodBeforeEC, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamSampleTCPStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleTCPStatsSampleSeconds, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleTCPStatsPacketsReceived, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {PacketsRetransmitted, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleTCPStatsPacketsRetransmitted, NULL, NULL, 1, 10, 0, 0, NULL, false},
    {BytesReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleTCPStatsBytesReceived, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleTCPStatsSampleSeconds, NULL, NULL},
	{PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleTCPStatsPacketsReceived, NULL, NULL},
	{PacketsRetransmitted, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleTCPStatsPacketsRetransmitted, NULL, NULL},
	{BytesReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleTCPStatsBytesReceived, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamSampleDejitteringStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleDejitteringStatsSampleSeconds, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Overruns, {{tString, 64, 0}}, NULL, getSMMainStreamSampleDejitteringStatsOverruns, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Underruns, {{tString, 64, 0}}, NULL, getSMMainStreamSampleDejitteringStatsUnderruns, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {EmptyBufferTime, {{tString, 64, 0}}, NULL, getSMMainStreamSampleDejitteringStatsEmptyBufferTime, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleDejitteringStatsSampleSeconds, NULL, NULL},
	{Overruns, {{tString, 64, 0}}, NULL, getSMMainStreamSampleDejitteringStatsOverruns, NULL, NULL},
	{Underruns, {{tString, 64, 0}}, NULL, getSMMainStreamSampleDejitteringStatsUnderruns, NULL, NULL},
	{EmptyBufferTime, {{tString, 64, 0}}, NULL, getSMMainStreamSampleDejitteringStatsEmptyBufferTime, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamSampleDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleSampleSeconds, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SignificantChanges, {{tString, 64, 0}}, NULL, getSMMainStreamSampleSignificantChanges, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PVRTimeShift, {{tString, 64, 0}}, NULL, getSMMainStreamSamplePVRTimeShift, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {HighLevelMetricStatsNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {DejitteringStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleDejitteringStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {TCPStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleTCPStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {RTPStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleRTPStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {MPEG2TSStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleMPEG2TSStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {VideoDecoderStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleVideoDecoderStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {AudioDecoderStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleAudioDecoderStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {VideoResponseStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleVideoResponseStatsDesc, NULL, 1, 0, 0, 0, NULL, false},
    {HighLevelMetricStats, {{tInstance, 0, 0}}, NULL, NULL, SMMainStreamSampleHighLevelMetricStatsInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{SampleSeconds, {{tString, 64, 0}}, NULL, getSMMainStreamSampleSampleSeconds, NULL, NULL},
	{SignificantChanges, {{tString, 64, 0}}, NULL, getSMMainStreamSampleSignificantChanges, NULL, NULL},
	{PVRTimeShift, {{tString, 64, 0}}, NULL, getSMMainStreamSamplePVRTimeShift, NULL, NULL},
	{HighLevelMetricStatsNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamSampleHighLevelMetricStatsNumberOfEntries, NULL, NULL},
	{DejitteringStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleDejitteringStatsDesc, NULL},
	{TCPStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleTCPStatsDesc, NULL},
	{RTPStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleRTPStatsDesc, NULL},
	{MPEG2TSStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleMPEG2TSStatsDesc, NULL},
	{VideoDecoderStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleVideoDecoderStatsDesc, NULL},
	{AudioDecoderStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleAudioDecoderStatsDesc, NULL},
	{VideoResponseStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleVideoResponseStatsDesc, NULL},
	{HighLevelMetricStats, {{tInstance, 0, 0}}, NULL, NULL, SMMainStreamSampleHighLevelMetricStatsInstanceDesc, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamTotalVideoResponseStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsTotalSeconds, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {ChannelChangeFailures, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsChannelChangeFailures, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {ChannelFailures, {{tString, 64, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsChannelFailures, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MinimumVoDControlResponse, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsMinimumVoDControlResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MaximumVoDControlResponse, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsMaximumVoDControlResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {RequestedTransactions, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsRequestedTransactions, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {AccessSuccesses, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsAccessSuccesses, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {CompletionCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsCompletionCount, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MinimumVideoSystemResponse, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsMinimumVideoSystemResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MaximumVideoSystemResponse, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsMaximumVideoSystemResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsTotalSeconds, NULL, NULL},
	{ChannelChangeFailures, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsChannelChangeFailures, NULL, NULL},
	{ChannelFailures, {{tString, 64, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsChannelFailures, NULL, NULL},
	{MinimumVoDControlResponse, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsMinimumVoDControlResponse, NULL, NULL},
	{MaximumVoDControlResponse, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsMaximumVoDControlResponse, NULL, NULL},
	{RequestedTransactions, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsRequestedTransactions, NULL, NULL},
	{AccessSuccesses, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsAccessSuccesses, NULL, NULL},
	{CompletionCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsCompletionCount, NULL, NULL},
	{MinimumVideoSystemResponse, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsMinimumVideoSystemResponse, NULL, NULL},
	{MaximumVideoSystemResponse, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoResponseStatsMaximumVideoSystemResponse, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamTotalAudioDecoderStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalAudioDecoderStatsTotalSeconds, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {DecodedFrames, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalAudioDecoderStatsDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DecodingErrors, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalAudioDecoderStatsDecodingErrors, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalAudioDecoderStatsTotalSeconds, NULL, NULL},
	{DecodedFrames, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalAudioDecoderStatsDecodedFrames, NULL, NULL},
	{DecodingErrors, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalAudioDecoderStatsDecodingErrors, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamTotalVideoDecoderStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoDecoderStatsTotalSeconds, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {FrameRate,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsFrameRate, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DecodedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {LostFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsLostFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ConcealedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsConcealedFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {IDecodedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsIDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ILostFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsILostFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {IConcealedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsIConcealedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PDecodedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsPDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PLostFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsPLostFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PConcealedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsPConcealedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {BDecodedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsBDecodedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {BLostFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsBLostFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {BConcealedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsBConcealedFrames, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {AVResynchCounter,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsAVResynchCounter, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalVideoDecoderStatsTotalSeconds, NULL, NULL},
	{FrameRate,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsFrameRate, NULL, NULL},
	{DecodedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsDecodedFrames, NULL, NULL},
	{LostFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsLostFrames, NULL, NULL},
	{ConcealedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsConcealedFrames, NULL, NULL},
	{IDecodedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsIDecodedFrames, NULL, NULL},
	{ILostFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsILostFrames, NULL, NULL},
	{IConcealedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsIConcealedFrames, NULL, NULL},
	{PDecodedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsPDecodedFrames, NULL, NULL},
	{PLostFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsPLostFrames, NULL, NULL},
	{PConcealedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsPConcealedFrames, NULL, NULL},
	{BDecodedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsBDecodedFrames, NULL, NULL},
	{BLostFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsBLostFrames, NULL, NULL},
	{BConcealedFrames,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsBConcealedFrames, NULL, NULL},
	{AVResynchCounter,{{tUnsigned,0,0}}, NULL, getSMMainStreamTotalVideoDecoderStatsAVResynchCounter, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamTotalMPEG2TSStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsTotalSeconds, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {TSPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsTSPacketsReceived, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {TSSyncByteErrorCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsTSSyncByteErrorCount, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {TSSyncLossCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsTSSyncLossCount, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketDiscontinuityCounter, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsPacketDiscontinuityCounter, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketDiscontinuityCounterBeforeCA, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsPacketDiscontinuityCounterBeforeCA, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsTotalSeconds, NULL, NULL},
	{TSPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsTSPacketsReceived, NULL, NULL},
	{TSSyncByteErrorCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsTSSyncByteErrorCount, NULL, NULL},
	{TSSyncLossCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsTSSyncLossCount, NULL, NULL},
	{PacketDiscontinuityCounter, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsPacketDiscontinuityCounter, NULL, NULL},
	{PacketDiscontinuityCounterBeforeCA, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalMPEG2TSStatsPacketDiscontinuityCounterBeforeCA, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamTotalRTPStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsTotalSeconds, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {PacketsExpected, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsExpected, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PacketsDiscarded, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsDiscarded, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsOutOfSequence, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsOutOfSequence, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsReceived, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsReceivedBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsReceivedBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsLost, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsLost, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsLostBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsLostBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PacketsLostByEventHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsLostByEventHistogram, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {PacketsLostByEventHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsLostByEventHistogramBeforeEC, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {LossEvents, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsLossEvents, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {LossEventsBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsLossEventsBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DelayBetweenLossEventsHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsDelayBetweenLossEventsHistogram, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {DelayBetweenLossEventsHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsDelayBetweenLossEventsHistogramBeforeEC, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {SevereLossIndexCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsSevereLossIndexCount, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SevereLossIndexCountBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsSevereLossIndexCountBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SevereLossLengthCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsSevereLossLengthCount, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SevereLossLengthCountBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsSevereLossLengthCountBeforeEC, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {DurationSevereLossEventsHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsDurationSevereLossEventsHistogram, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {DurationSevereLossEventsHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsDurationSevereLossEventsHistogramBeforeEC, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {RetransmitTimeouts, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsRetransmitTimeouts, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsTotalSeconds, NULL, NULL},
	{PacketsExpected, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsExpected, NULL, NULL},
	{PacketsDiscarded, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsDiscarded, NULL, NULL},
	{PacketsOutOfSequence, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsOutOfSequence, NULL, NULL},
	{PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsReceived, NULL, NULL},
	{PacketsReceivedBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsReceivedBeforeEC, NULL, NULL},
	{PacketsLost, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsLost, NULL, NULL},
	{PacketsLostBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsLostBeforeEC, NULL, NULL},
	{PacketsLostByEventHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsLostByEventHistogram, NULL, NULL},
	{PacketsLostByEventHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsPacketsLostByEventHistogramBeforeEC, NULL, NULL},
	{LossEvents, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsLossEvents, NULL, NULL},
	{LossEventsBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsLossEventsBeforeEC, NULL, NULL},
	{DelayBetweenLossEventsHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsDelayBetweenLossEventsHistogram, NULL, NULL},
	{DelayBetweenLossEventsHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsDelayBetweenLossEventsHistogramBeforeEC, NULL, NULL},
	{SevereLossIndexCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsSevereLossIndexCount, NULL, NULL},
	{SevereLossIndexCountBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsSevereLossIndexCountBeforeEC, NULL, NULL},
	{SevereLossLengthCount, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsSevereLossLengthCount, NULL, NULL},
	{SevereLossLengthCountBeforeEC, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsSevereLossLengthCountBeforeEC, NULL, NULL},
	{DurationSevereLossEventsHistogram, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsDurationSevereLossEventsHistogram, NULL, NULL},
	{DurationSevereLossEventsHistogramBeforeEC, {{tString, 64, 0}}, NULL, getSMMainStreamTotalRTPStatsDurationSevereLossEventsHistogramBeforeEC, NULL, NULL},
	{RetransmitTimeouts, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalRTPStatsRetransmitTimeouts, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamTotalTCPStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTCPStatsTotalSeconds, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTCPStatsPacketsReceived, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {PacketsRetransmitted, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTCPStatsPacketsRetransmitted, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {BytesReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTCPStatsBytesReceived, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTCPStatsTotalSeconds, NULL, NULL},
	{PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTCPStatsPacketsReceived, NULL, NULL},
	{PacketsRetransmitted, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTCPStatsPacketsRetransmitted, NULL, NULL},
	{BytesReceived, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTCPStatsBytesReceived, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamTotalDejitteringStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalDejitteringStatsTotalSeconds, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {EmptyBufferTime, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalDejitteringStatsEmptyBufferTime, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {Overruns, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalDejitteringStatsOverruns, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Underruns, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalDejitteringStatsUnderruns, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalDejitteringStatsTotalSeconds, NULL, NULL},
	{EmptyBufferTime, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalDejitteringStatsEmptyBufferTime, NULL, NULL},
	{Overruns, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalDejitteringStatsOverruns, NULL, NULL},
	{Underruns, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalDejitteringStatsUnderruns, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamTotalDesc[] = {
#ifdef XML_DOC_SUPPORT
    {TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTotalSeconds, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {Reset, {{tBool, 0, 0}}, setSMMainStreamTotalReset, getSMMainStreamTotalReset, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ResetTime, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalResetTime, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DejitteringStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalDejitteringStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {TCPStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalTCPStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {RTPStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalRTPStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {MPEG2TSStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalMPEG2TSStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {VideoDecoderStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalVideoDecoderStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {AudioDecoderStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalAudioDecoderStatsDesc, NULL, 1, 0, 0, 0, NULL, true},
    {VideoResponseStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalVideoResponseStatsDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}} ,NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{TotalSeconds, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalTotalSeconds, NULL, NULL},
	{Reset, {{tBool, 0, 0}}, setSMMainStreamTotalReset, getSMMainStreamTotalReset, NULL, NULL},
	{ResetTime, {{tUnsigned, 0, 0}}, NULL, getSMMainStreamTotalResetTime, NULL, NULL},
	{DejitteringStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalDejitteringStatsDesc, NULL},
	{TCPStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalTCPStatsDesc, NULL},
	{RTPStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalRTPStatsDesc, NULL},
	{MPEG2TSStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalMPEG2TSStatsDesc, NULL},
	{VideoDecoderStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalVideoDecoderStatsDesc, NULL},
	{AudioDecoderStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalAudioDecoderStatsDesc, NULL},
	{VideoResponseStats, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalVideoResponseStatsDesc, NULL},
	{NULL, {{0, 0, 0}} ,NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setSMMainStreamEnable, getSMMainStreamEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getSMMainStreamStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setSMMainStreamAlias, getSMMainStreamAlias, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ServiceType, {{tString, 32, 0}}, setSMMainStreamServiceType, getSMMainStreamServiceType, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AVStream, {{tString, 256, 0}}, NULL, getSMMainStreamAVStream, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Gmin, {{tUnsigned, 0, 0}}, setSMMainStreamGmin, getSMMainStreamGmin, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SevereLossMinDistance, {{tUnsigned,0,0}}, setSMMainStreamSevereLossMinDistance, getSMMainStreamSevereLossMinDistance, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SevereLossMinLength, {{tUnsigned, 0, 0}}, setSMMainStreamSevereLossMinLength, getSMMainStreamSevereLossMinLength, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ChannelChangeFailureTimeout, {{tUnsigned, 0, 0}}, setSMMainStreamChannelChangeFailureTimeout, getSMMainStreamChannelChangeFailureTimeout, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {PacketsLostByEventHistogramIntervals, {{tString, 32, 0}}, setSMMainStreamPacketsLostByEventHistogramIntervals, getSMMainStreamPacketsLostByEventHistogramIntervals, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {DelayBetweenLossEventsHistogramIntervals, {{tString, 32, 0}}, setSMMainStreamDelayBetweenLossEventsHistogramIntervals, getSMMainStreamDelayBetweenLossEventsHistogramIntervals, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {DurationSevereLossEventsHistogramIntervals, {{tString, 32, 0}}, setSMMainStreamDurationSevereLossEventsHistogramIntervals, getSMMainStreamDurationSevereLossEventsHistogramIntervals, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Total, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalDesc, NULL, 1, 0, 0, 0, NULL, true},
    {Sample, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{Enable, {{tBool, 0, 0}}, setSMMainStreamEnable, getSMMainStreamEnable, NULL, NULL},
	{Status, {{tString, 32, 0}}, NULL, getSMMainStreamStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setSMMainStreamAlias, getSMMainStreamAlias, NULL, NULL},
	{ServiceType, {{tString, 32, 0}}, setSMMainStreamServiceType, getSMMainStreamServiceType, NULL, NULL},
	{AVStream, {{tString, 256, 0}}, NULL, getSMMainStreamAVStream, NULL, NULL},
	{Gmin, {{tUnsigned, 0, 0}}, setSMMainStreamGmin, getSMMainStreamGmin, NULL, NULL},
	{SevereLossMinDistance, {{tUnsigned,0,0}}, setSMMainStreamSevereLossMinDistance, getSMMainStreamSevereLossMinDistance, NULL, NULL},
	{SevereLossMinLength, {{tUnsigned, 0, 0}}, setSMMainStreamSevereLossMinLength, getSMMainStreamSevereLossMinLength, NULL, NULL},
	{ChannelChangeFailureTimeout, {{tUnsigned, 0, 0}}, setSMMainStreamChannelChangeFailureTimeout, getSMMainStreamChannelChangeFailureTimeout, NULL, NULL},
	{PacketsLostByEventHistogramIntervals, {{tString, 32, 0}}, setSMMainStreamPacketsLostByEventHistogramIntervals, getSMMainStreamPacketsLostByEventHistogramIntervals, NULL, NULL},
	{DelayBetweenLossEventsHistogramIntervals, {{tString, 32, 0}}, setSMMainStreamDelayBetweenLossEventsHistogramIntervals, getSMMainStreamDelayBetweenLossEventsHistogramIntervals, NULL, NULL},
	{DurationSevereLossEventsHistogramIntervals, {{tString, 32, 0}}, setSMMainStreamDurationSevereLossEventsHistogramIntervals, getSMMainStreamDurationSevereLossEventsHistogramIntervals, NULL, NULL},
 	{Total, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamTotalDesc, NULL},
 	{Sample, {{tObject, 0, 0}}, NULL, NULL, SMMainStreamSampleDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMMainStreamInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK, {{0, 0, 0}}, NULL, NULL, SMMainStreamDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK, {{0, 0, 0}}, NULL, NULL, SMMainStreamDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMGlobalOperationSampleDesc[] = {
#ifdef XML_DOC_SUPPORT
    {MinimumPortalResponse, {{tString, 32, 0}}, NULL, getSMGlobalOperationSampleMinimumPortalResponse, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {MaximumPortalResponse, {{tString, 32, 0}}, NULL, getSMGlobalOperationSampleMaximumPortalResponse, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {PortalResponse, {{tString, 32, 0}}, NULL, getSMGlobalOperationSamplePortalResponse, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{MinimumPortalResponse, {{tString, 32, 0}}, NULL, getSMGlobalOperationSampleMinimumPortalResponse, NULL, NULL},
	{MaximumPortalResponse, {{tString, 32, 0}}, NULL, getSMGlobalOperationSampleMaximumPortalResponse, NULL, NULL},
	{PortalResponse, {{tString, 32, 0}}, NULL, getSMGlobalOperationSamplePortalResponse, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMGlobalOperationTotalDesc[] = {
#ifdef XML_DOC_SUPPORT
    {ServiceAccessTime, {{tUnsigned, 32, 0}}, NULL, getSMGlobalOperationTotalServiceAccessTime, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MinimumPortalResponse, {{tUnsigned, 32, 0}}, NULL, getSMGlobalOperationTotalMinimumPortalResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {MaximumPortalResponse, {{tUnsigned, 32, 0}}, NULL, getSMGlobalOperationTMaximumPortalResponse, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{ServiceAccessTime, {{tUnsigned, 32, 0}}, NULL, getSMGlobalOperationTotalServiceAccessTime, NULL, NULL},
	{MinimumPortalResponse, {{tUnsigned, 32, 0}}, NULL, getSMGlobalOperationTotalMinimumPortalResponse, NULL, NULL},
	{MaximumPortalResponse, {{tUnsigned, 32, 0}}, NULL, getSMGlobalOperationTMaximumPortalResponse, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode SMGlobalOperationDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Total, {{tObject, 0, 0}}, NULL, NULL, SMGlobalOperationTotalDesc, NULL, 1, 1, 0, 0, NULL, false},
    {Sample, {{tObject, 0, 0}}, NULL, NULL, SMGlobalOperationSampleDesc, NULL, 1, 1, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{Total, {{tObject, 0, 0}}, NULL, NULL, SMGlobalOperationTotalDesc, NULL},
	{Sample, {{tObject, 0, 0}}, NULL, NULL, SMGlobalOperationSampleDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ServiceMonitoringDesc[] = {
#ifdef XML_DOC_SUPPORT
    {SampleEnable, {{tBool, 0, 0}}, setSMSampleEnable, getSMSampleEnable, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SampleState, {{tString, 32, 0}}, NULL, getSMSampleState, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SampleInterval, {{tUnsigned, 0, 0}}, setSMSampleInterval, getSMSampleInterval, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ReportSamples, {{tUnsigned, 0, 0}}, setSMReportSamples, getSMReportSamples, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {EventsPerSampleInterval, {{tUnsigned, 0, 0}}, setSMEventsPerSampleInterval, getSMEventsPerSampleInterval, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {FetchSamples, {{tUnsigned, 0, 0}}, setSMFetchSamples, getSMFetchSamples, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {TimeReference, {{tDateTime,0,0}}, setSMTimeReference, getSMTimeReference, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ForceSample, {{tBool, 0, 0}}, setSMForceSample, getSMForceSample, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ReportStartTime, {{tDateTime, 0, 0}}, NULL, getSMReportStartTime, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ReportEndTime, {{tDateTime, 0, 0}}, NULL, getSMReportEndTime, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MainStreamNumberOfEntries, {{tUnsigned,0,0}}, NULL, getSMMainStreamNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {GlobalOperation, {{tObject, 0, 0}}, NULL, NULL, SMGlobalOperationDesc, NULL, 1, 0, 0, 0, NULL, false},
    {MainStream, {{tInstance, 0, 0}}, NULL, NULL, SMMainStreamInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{SampleEnable, {{tBool, 0, 0}}, setSMSampleEnable, getSMSampleEnable, NULL, NULL},
	{SampleState, {{tString, 32, 0}}, NULL, getSMSampleState, NULL, NULL},
	{SampleInterval, {{tUnsigned, 0, 0}}, setSMSampleInterval, getSMSampleInterval, NULL, NULL},
	{ReportSamples, {{tUnsigned, 0, 0}}, setSMReportSamples, getSMReportSamples, NULL, NULL},
	{EventsPerSampleInterval, {{tUnsigned, 0, 0}}, setSMEventsPerSampleInterval, getSMEventsPerSampleInterval, NULL, NULL},
	{FetchSamples, {{tUnsigned, 0, 0}}, setSMFetchSamples, getSMFetchSamples, NULL, NULL},
	{TimeReference, {{tDateTime,0,0}}, setSMTimeReference, getSMTimeReference, NULL, NULL},
	{ForceSample, {{tBool, 0, 0}}, setSMForceSample, getSMForceSample, NULL, NULL},
	{ReportStartTime, {{tDateTime, 0, 0}}, NULL, getSMReportStartTime, NULL, NULL},
	{ReportEndTime, {{tDateTime, 0, 0}}, NULL, getSMReportEndTime, NULL, NULL},
	{MainStreamNumberOfEntries, {{tUnsigned,0,0}}, NULL, getSMMainStreamNumberOfEntries, NULL, NULL},
	{GlobalOperation, {{tObject, 0, 0}}, NULL, NULL, SMGlobalOperationDesc, NULL},
	{MainStream, {{tInstance, 0, 0}}, NULL, NULL, SMMainStreamInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};


