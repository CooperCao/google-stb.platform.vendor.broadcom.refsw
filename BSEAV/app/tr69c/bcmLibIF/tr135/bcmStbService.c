/******************************************************************************
 *	  (c)2010-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * 1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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

#include "../../SOAPParser/CPEframework.h"
#include "../../standard/tr135/stbcapabilitiesparams.h"
#include "../../standard/tr135/stbcomponentsparams.h"
#include "../../standard/tr135/stbmiscparams.h"
#include "../../standard/tr135/stbservicemonitoringparams.h"
#include "bcmTR135Objs.h"
#include "stbSpectrumAnalyzerParams.h"
#include "tr69clib_priv.h"

extern TRxObjNode STBServiceInstanceDesc[];
extern TRxObjNode CapVideoDecoderMPEG2Part2ProfileLevelInstanceDesc[];
extern TRxObjNode CapVideoDecoderMPEG4Part2ProfileLevelInstanceDesc[];
extern TRxObjNode CapVideoDecoderMPEG4Part10ProfileLevelInstanceDesc[];
extern TRxObjNode CapVideoDecoderSMPTEVC1ProfileLevelInstanceDesc[];
extern TRxObjNode CompFEIPIGMPClientGroupInstanceDesc[];
extern TRxObjNode CompFEIPIGMPClientGroupStatsInstanceDesc[];
extern TRxObjNode CompFEInstanceDesc[];
extern TRxObjNode CompFEIPInboundInstanceDesc[];
extern TRxObjNode CompFEIPOutboundInstanceDesc[];
extern TRxObjNode CompPVRStorageInstanceDesc[];
extern TRxObjNode CompAudioDecoderInstanceDesc[];
extern TRxObjNode CompVideoDecoderInstanceDesc[];
extern TRxObjNode CompAudioOutputInstanceDesc[];
extern TRxObjNode CompSPDIFInstanceDesc[];
extern TRxObjNode CompVideoOutputInstanceDesc[];
extern TRxObjNode CompSCARTInstanceDesc[];
extern TRxObjNode CompHDMIInstanceDesc[];
extern TRxObjNode CompCAInstanceDesc[];
extern TRxObjNode CompDRMInstanceDesc[];
extern TRxObjNode AVStreamsAVStreamInstanceDesc[];
extern TRxObjNode AVPlayersAVPlayerInstanceDesc[];
extern TRxObjNode SMMainStreamInstanceDesc[];
extern TRxObjNode SMMainStreamSampleHighLevelMetricStatsInstanceDesc[];
extern TRxObjNode ApplicationsServiceProviderInstanceDesc[];
extern TRxObjNode ApplicationsAudienceStatsChannelInstanceDesc[];
extern TRxObjNode ApplicationsCDSPushContentItemInstanceDesc[];
extern TRxObjNode ApplicationsCDSPullContentItemInstanceDesc[];
extern TRxObjNode X_BROADCOM_COM_spectrumAnalyzerDesc[];
extern TRxObjNode comparisonTableDesc[];
extern TRxObjNode measurementTableDesc[];

#if 1
extern TRxObjNode CompFEQAMDesc[];
extern TRxObjNode ModulationDesc[];
extern TRxObjNode CompFEQAMInstanceDesc[];

#endif
extern int getInstanceCount(TRxObjNode *n);

static InstanceDesc *stbServiceIdp;
static InstanceDesc *stbServiceCompFeIdp;
static InstanceDesc *stbServiceSMMainStreamIdp;
static InstanceDesc *spectrumAnalyzerIdp;

void initSTBService(void)
{
    TRxObjNode *n;
    int id, count, new_count;
    uint8_t num_instances;
    profileLevel *profileLevel;

    /* Device.Services.STBService.{i}. */
    n = STBServiceInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        stbServiceIdp = getNewInstanceDesc(n, NULL, id);
    }

    /* Device.Services.STBService.{i}.Capabilities.VideoDecoder.MPEG2Part2.ProfileLevel.{i}. */
    n = CapVideoDecoderMPEG2Part2ProfileLevelInstanceDesc;
    getProfileLevel(&profileLevel, &num_instances, MPEG2PART2_PROFILE_LEVEL);
    for (id = 1; id <= num_instances; id ++)
    {
        if(findInstanceDescNoPathCheck(n, id) == NULL)
        {
            getNewInstanceDesc(n, stbServiceIdp, id);
        }
    }

    /* Device.Services.STBService.{i}.Capabilities.VideoDecoder.MPEG4Part2.ProfileLevel.{i}. */
    n = CapVideoDecoderMPEG4Part2ProfileLevelInstanceDesc;
    getProfileLevel(&profileLevel, &num_instances, MPEG4PART2_PROFILE_LEVEL);
    for (id = 1; id <= num_instances; id ++)
    {
        if(findInstanceDescNoPathCheck(n, id) == NULL)
        {
            getNewInstanceDesc(n, stbServiceIdp, id);
        }
    }
    /* Device.Services.STBService.{i}.Capabilities.VideoDecoder.MPEG4Part10.ProfileLevel.{i}. */
    n = CapVideoDecoderMPEG4Part10ProfileLevelInstanceDesc;
    getProfileLevel(&profileLevel, &num_instances, MPEG4PART10_PROFILE_LEVEL);
    for (id = 1; id <= num_instances; id ++)
    {
        if(findInstanceDescNoPathCheck(n, id) == NULL)
        {
            getNewInstanceDesc(n, stbServiceIdp, id);
        }
    }

    /* Device.Services.STBService.{i}.Capabilities.VideoDecoder.SMPTEVC1.ProfileLevel.{i}. */
    n = CapVideoDecoderSMPTEVC1ProfileLevelInstanceDesc;
    getProfileLevel(&profileLevel, &num_instances, SMPTEVC1_PROFILE_LEVEL);
    for (id = 1; id <= num_instances; id ++)
    {
        if(findInstanceDescNoPathCheck(n, id) == NULL)
        {
            getNewInstanceDesc(n, stbServiceIdp, id);
        }
    }

    /* Device.Services.STBService.{i}.Components.FrontEnd.{i}. */
    n = CompFEInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        stbServiceCompFeIdp = getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.FrontEnd.{i}.IP.IGMP.ClientGroup.{i}. */
    n = CompFEIPIGMPClientGroupInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceCompFeIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.FrontEnd.{i}.IP.IGMP.ClientGroupStats.{i}. */
    n = CompFEIPIGMPClientGroupStatsInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceCompFeIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.FrontEnd.{i}.IP.Inbound.{i}. */
    n = CompFEIPInboundInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceCompFeIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.FrontEnd.{i}.IP.Outbound.{i}. */
    n = CompFEIPOutboundInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceCompFeIdp, id);
    }

	/* Device.Services.STBService.{i}.Components.FrontEnd.{i}.X_BROADCOM_COM_QAM.Modulation */
    n = CompFEQAMInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceCompFeIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.PVR.Storage.{i}. */
    n = CompPVRStorageInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.AudioDecoder.{i}. */
    n = CompAudioDecoderInstanceDesc;
    count = getInstanceCount(n);
    b_tr69c_get_client_count(&new_count);
    if (new_count > count)
    {
        for (id = 1; id <= count; id++)
        {
            if(findInstanceDescNoPathCheck(n, id) != NULL)
            {
                deleteInstanceDesc(n, id);
            }
        }
        for (id = 1; id <= new_count; id++)
        {
            if(findInstanceDescNoPathCheck(n, id) == NULL)
            {
                getNewInstanceDesc(n, stbServiceIdp, id);
            }
        }
    }

    /* Device.Services.STBService.{i}.Components.VideoDecoder.{i}. */
    n = CompVideoDecoderInstanceDesc;
    count = getInstanceCount(n);
    b_tr69c_get_client_count(&new_count);
    if (new_count > count)
    {
        for (id = 1; id <= count; id++)
        {
            if(findInstanceDescNoPathCheck(n, id) != NULL)
            {
                deleteInstanceDesc(n, id);
            }
        }
        for (id = 1; id <= new_count; id++)
        {
            if(findInstanceDescNoPathCheck(n, id) == NULL)
            {
                getNewInstanceDesc(n, stbServiceIdp, id);
            }
        }
    }

    /* Device.Services.STBService.{i}.Components.AudioOutput.{i}. */
    n = CompAudioOutputInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.SPDIF.{i}. */
    n = CompSPDIFInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp,  id);
    }

    /* Device.Services.STBService.{i}.Components.VideoOutput.{i}. */
    n = CompVideoOutputInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.SCART.{i}. */
    n = CompSCARTInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.HDMI.{i}. */
    n = CompHDMIInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.CA.{i}. */
    n = CompCAInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Components.DRM.{i}. */
    n = CompDRMInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.AVStreams.AVStream.{i}. */
    n = AVStreamsAVStreamInstanceDesc;
    count = getInstanceCount(n);
    b_tr69c_get_client_count(&new_count);
    if (new_count > count)
    {
        for (id = 1; id <= count; id++)
        {
            if(findInstanceDescNoPathCheck(n, id) != NULL)
            {
                deleteInstanceDesc(n, id);
            }
        }
        for (id = 1; id <= new_count; id++)
        {
            if(findInstanceDescNoPathCheck(n, id) == NULL)
            {
                getNewInstanceDesc(n, stbServiceIdp, id);
            }
        }
    }

    /* Device.Services.STBService.{i}.AVPlayers.AVPlayer.{i}. */
    n = AVPlayersAVPlayerInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.ServiceMonitoring.MainStream.{i}. */
    n = SMMainStreamInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        stbServiceSMMainStreamIdp = getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.ServiceMonitoring.MainStream.{i}.Sample.HighLevelMetricStats.{i}. */
    n = SMMainStreamSampleHighLevelMetricStatsInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceSMMainStreamIdp, id);
    }

    /* Device.Services.STBService.{i}.Applications.ServiceProvider.{i}. */
    n = ApplicationsServiceProviderInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Applications.AudienceStats.Channel.{i}. */
    n = ApplicationsAudienceStatsChannelInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Applications.CDSPush.ContentItem.{i}. */
    n = ApplicationsCDSPushContentItemInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* Device.Services.STBService.{i}.Applications.CDSPull.ContentItem.{i}. */
    n = ApplicationsCDSPullContentItemInstanceDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, stbServiceIdp, id);
    }

    /* The following objects should be moved to somewhere else since they do not belong to Device.Services.STBService.{i}. */

    /* define instance of Spectrum Analyzer */
    n = X_BROADCOM_COM_spectrumAnalyzerDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        spectrumAnalyzerIdp = getNewInstanceDesc(n, NULL, id);
    }

    /*comparisonTableDesc*/
    n = comparisonTableDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, spectrumAnalyzerIdp, id);
    }

    /*measurementTableDesc*/
    n = measurementTableDesc;
    id = 1;
    if(findInstanceDescNoPathCheck(n, id) == NULL)
    {
        getNewInstanceDesc(n, spectrumAnalyzerIdp, id);
    }
}
