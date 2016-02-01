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
#include <stdlib.h>
#include <string.h>
#include "nexus_platform.h"
#include "nexus_platform_common.h"
#include "nexus_audio_decoder.h"
#include "nexus_playpump.h"
#include "nxclient.h"
#include "nexus_display.h"
#include "nexus_video_types.h"
#include "nexus_display_types.h"
#include "nexus_video_decoder_types.h"
#include "bcmStbServiceHelper.h"


#define MAX_COMPOSITE_VIDEO_STANDARDS	16
static b_audioDecoder_t g_audioDecoder[MAX_AVSTREAMS];
static b_videoDecoder_t g_videoDecoder[MAX_AVSTREAMS];
static b_AVStream_t avStream[MAX_AVSTREAMS];
static b_AVPlayer_t avPlayer[MAX_AVPLAYERS];
static b_QAMModulation_t qamModulation[MAX_QAM_FRONTENDS];
static b_mainStream_t mainStream[MAX_MAIN_STREAMS];
static b_total_t total;
static b_MPEG2TSStats_t MPEG2TSStats;
static b_videoDecoderStats_t videoDecoderStats;
static b_audioDecoderStats_t audioDecoderStats;
static b_spdif_t spdif[NEXUS_NUM_SPDIF_OUTPUTS];
static b_IpInbound_t inbound[MAX_AVSTREAMS];

static profileLevel Bcm_MPEG2ProfileLevel[]=

{
	{NEXUS_VideoProtocolProfile_eSimple, NEXUS_VideoProtocolLevel_eMain, 15000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_eHigh, 80000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_eHigh1440, 60000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_eMain, 15000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_eLow, 4000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eSnrScalable, NEXUS_VideoProtocolLevel_eMain, 15000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eSnrScalable, NEXUS_VideoProtocolLevel_eLow, 4000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eSpatiallyScalable, NEXUS_VideoProtocolLevel_eHigh1440, 60000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_eHigh, 100000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_eHigh1440, 80000, "CPE-MPEG2Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_eMain, 20000, "CPE-MPEG2Part2.ProfileLevel"},
};
static uint8_t Bcm_MPEG2ProfileLevel_Entries =  sizeof(Bcm_MPEG2ProfileLevel)/sizeof(profileLevel);


static profileLevel Bcm_MPEG4Part2ProfileLevel[]=
{
	{NEXUS_VideoProtocolProfile_eSimple, NEXUS_VideoProtocolLevel_e00, 64, "CPE-MPEG4Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eSimple, NEXUS_VideoProtocolLevel_e10, 64, "CPE-MPEG4Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eSimple, NEXUS_VideoProtocolLevel_e20, 128, "CPE-MPEG4Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eSimple, NEXUS_VideoProtocolLevel_e30, 384, "CPE-MPEG4Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvancedSimple, NEXUS_VideoProtocolLevel_e00, 128, "CPE-MPEG4Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvancedSimple, NEXUS_VideoProtocolLevel_e10, 128, "CPE-MPEG4Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvancedSimple, NEXUS_VideoProtocolLevel_e20, 384, "CPE-MPEG4Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvancedSimple, NEXUS_VideoProtocolLevel_e30, 1500, "CPE-MPEG4Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvancedSimple, NEXUS_VideoProtocolLevel_e40, 3000, "CPE-MPEG4Part2.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvancedSimple, NEXUS_VideoProtocolLevel_e50, 8000, "CPE-MPEG4Part2.ProfileLevel"}
};

static uint8_t Bcm_MPEG4Part2ProfileLevel_Entries =  sizeof(Bcm_MPEG4Part2ProfileLevel)/sizeof(profileLevel);


static profileLevel Bcm_MPEG4Part10ProfileLevel[]=
{
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e10, 76.8, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e1B, 153.6, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e11, 230.4, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e12, 460.8, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e13, 921.6, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e20, 2400, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e21, 4800, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e22, 4800, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e30, 12000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e31, 16800, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e32, 24000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e40, 24000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e41, 60000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_e42, 60000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e10, 76.8, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e1B, 153.6, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e11, 230.4, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e12, 460.8, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e13, 921.6, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e20, 2400, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e21, 4800, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e22, 4800, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e30, 12000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e31, 16800, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e32, 24000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e40, 24000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e41, 60000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eHigh, NEXUS_VideoProtocolLevel_e42, 60000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e10, 76.8, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e1B, 153.6, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e11, 230.4, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e12, 460.8, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e13, 921.6, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e20, 2400, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e21, 4800, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e22, 4800, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e30, 12000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e31, 16800, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e32, 24000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e40, 24000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e41, 60000, "CPE-MPEG4Part10.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eBaseline, NEXUS_VideoProtocolLevel_e42, 60000, "CPE-MPEG4Part10.ProfileLevel"}
};

static uint8_t Bcm_MPEG4Part10ProfileLevel_Entries =  sizeof(Bcm_MPEG4Part10ProfileLevel)/sizeof(profileLevel);


static profileLevel Bcm_SMPTEVC1ProfileLevel[]=
{
	{NEXUS_VideoProtocolProfile_eSimple, NEXUS_VideoProtocolLevel_eLow, 96, "CPE-VC1.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eSimple, NEXUS_VideoProtocolLevel_eMain, 384, "CPE-VC1.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_eLow, 2000, "CPE-VC1.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_eMain, 10000, "CPE-VC1.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eMain, NEXUS_VideoProtocolLevel_eHigh, 20000, "CPE-VC1.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvanced, NEXUS_VideoProtocolLevel_e00, 2000, "CPE-VC1.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvanced, NEXUS_VideoProtocolLevel_e10, 10000, "CPE-VC1.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvanced, NEXUS_VideoProtocolLevel_e20, 20000, "CPE-VC1.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvanced, NEXUS_VideoProtocolLevel_e30, 45000, "CPE-VC1.ProfileLevel"},
	{NEXUS_VideoProtocolProfile_eAdvanced, NEXUS_VideoProtocolLevel_e40, 135000, "CPE-VC1.ProfileLevel"}
};

static uint8_t Bcm_SMPTEVC1ProfileLevel_Entries =  sizeof(Bcm_SMPTEVC1ProfileLevel)/sizeof(profileLevel);

static b_AVPlayers_t avPlayers =
{
	MAX_AVPLAYERS, MAX_AVPLAYERS, "English", "English", "16:9", true
};

static b_AVStreams_t avStreams =
{
	MAX_AVSTREAMS, MAX_AVSTREAMS
};

static decoder audioDecoder[] =
{
	{1, "CPE-AudioDecoder", "Primary Audio Decoder"},
	{2, "CPE-AudioDecoder", "Second Audio Decoder"}
};

static uint8_t Bcm_AudioDecoder_Entries =  sizeof(audioDecoder)/sizeof(decoder);

static decoder videoDecoder[] =
{
	{1, "CPE-VideoDecoder", "Primary Video Decoder"},
	{2, "CPE-VideoDecoder", "Second Video Decoder"}
};

static uint8_t Bcm_VideoDecoder_Entries =  sizeof(videoDecoder)/sizeof(decoder);
static decoder hdmi[] =
{
	{1, "CPE-HDMI", "Primary HDMI"},
	{2, "CPE-HDMI", "Second HDMI"}
};
static uint8_t Bcm_HDMI_Entries =  sizeof(hdmi)/sizeof(decoder);

static spectrumAnalyzer spectrumSettings =
{
	DEFAULT_NUMSAMPLES, DEFAULT_MEASUREMENTSPERBIN, MAX_NUMAVERAGES, false, 0, 0, 1000000000, 0, 1070000000, 446000000, DEFAULT_NUMENTRIES
};


int  getProfileLevel(profileLevel **pProfileLevel, uint8_t *pNumEntries, char *name) {

	if ((pProfileLevel == NULL) || (pNumEntries == NULL) || (name == NULL)) {
		return -1;
	}
	else if (!strcmp(name, MPEG2PART2_PROFILE_LEVEL)){
		*pProfileLevel = &Bcm_MPEG2ProfileLevel[0];
		*pNumEntries = Bcm_MPEG2ProfileLevel_Entries;
	}
	else if (!strcmp(name, MPEG4PART2_PROFILE_LEVEL)){
		*pProfileLevel = &Bcm_MPEG4Part2ProfileLevel[0];
		*pNumEntries = Bcm_MPEG4Part2ProfileLevel_Entries;
	}
	else if (!strcmp(name, MPEG4PART10_PROFILE_LEVEL)){
		*pProfileLevel = &Bcm_MPEG4Part10ProfileLevel[0];
		*pNumEntries = Bcm_MPEG4Part10ProfileLevel_Entries;
	}
	else if (!strcmp(name, SMPTEVC1_PROFILE_LEVEL)){
		*pProfileLevel = &Bcm_SMPTEVC1ProfileLevel[0];
		*pNumEntries = Bcm_SMPTEVC1ProfileLevel_Entries;
	}
	else
		printf("Profile is not recognized\n");

	return 0;
}

int setAudioVideoDecoderAlias(decoder **pDecoder, char *name, char *alias)
{
	if ((pDecoder == NULL) || (name == NULL) || (name == alias)) {
		return -1;
	}
	else if (!strcmp(name, "video"))
		Bcm_MPEG4Part2ProfileLevel[0].alias = strdup(alias);
	else if (!strcmp(name, "audio"))
		audioDecoder[0].alias = strdup(alias);
	else if (!strcmp(name, "hdmi"))
		hdmi[0].alias = strdup(alias);
	return 0;
}

int getAudioVideoDecoder(decoder **pDecoder, uint8_t *pNumEntries, char *name)
{
	if ((pDecoder == NULL) || (pNumEntries == NULL) || (name == NULL)) {
		return -1;
	}
	else if (!strcmp(name, "video")){
		*pDecoder = &videoDecoder[0];
		*pNumEntries = Bcm_AudioDecoder_Entries;
	}
	else if (!strcmp(name, "audio")){
		*pDecoder = &audioDecoder[0];
		*pNumEntries = Bcm_VideoDecoder_Entries;
    }
	else if (!strcmp(name, "hdmi")){
		*pDecoder = &hdmi[0];
		*pNumEntries = Bcm_HDMI_Entries;
    }
	else
		printf("Decoder is not recognized\n");
	return 0;
}

void init_135(void)
{
	int i;

	BKNI_Memset(avPlayer, 0, sizeof(avPlayer) * MAX_AVPLAYERS);
	BKNI_Memset(qamModulation, 0, sizeof(qamModulation) * MAX_QAM_FRONTENDS);
	BKNI_Memset(&MPEG2TSStats, 0, sizeof(MPEG2TSStats));
	BKNI_Memset(&videoDecoderStats, 0, sizeof(videoDecoderStats));
	BKNI_Memset(&audioDecoderStats, 0, sizeof(audioDecoderStats));
	BKNI_Memset(&total, 0, sizeof(total));
	BKNI_Memset(&mainStream, 0, sizeof(mainStream));
	BKNI_Memset(&spdif, 0, sizeof(spdif));
	BKNI_Memset(&inbound, 0, sizeof(inbound));

	for (i=0;i<MAX_AVSTREAMS;i++)
	{
		BKNI_Memset(&avStream[i], 0, sizeof(avStream));
		BKNI_Memset(&g_audioDecoder[i], 0, sizeof(g_audioDecoder));
		BKNI_Memset(&g_videoDecoder[i], 0, sizeof(g_videoDecoder));
	}

}

int setAudioDecoderParams(b_audioDecoder_t *in, int index)
{
	if ((in== NULL) || (index < 0) || (index > MAX_AVSTREAMS))
	{
		return -1;
	}
	BKNI_Memcpy(&g_audioDecoder[index], in, sizeof(b_audioDecoder_t));
	return 0;
}

int setVideoDecoderParams(b_videoDecoder_t *in, int index)
{
	if ((in== NULL) || (index < 0) || (index > MAX_AVSTREAMS))
	{
		return -1;
	}
	BKNI_Memcpy(&g_videoDecoder[index], in, sizeof(b_videoDecoder_t));
	return 0;
}

int getAVPlayers (b_AVPlayers_t **pAVPlayers)
{
	if (pAVPlayers == NULL)
		return -1;
	*pAVPlayers = &avPlayers;
	return 0;
}

int setAVPlayersParams(b_AVPlayers_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&avPlayers, in, sizeof(b_AVPlayers_t));
	return 0;
}


int getAVPlayer (b_AVPlayer_t **pAVPlayer, int index)
{
	if ((pAVPlayer == NULL) || (index < 0) || (index > MAX_AVSTREAMS))
		return -1;
	*pAVPlayer = &avPlayer[index];
	return 0;
}

int setAVPlayerParams(b_AVPlayer_t *in, int index)
{
	if ((in == NULL) || (index < 0) || (index > MAX_AVPLAYERS))
		return -1;
	BKNI_Memcpy(&avPlayer[index], in, sizeof(b_AVPlayer_t));
	return 0;
}

int getAVStreams (b_AVStreams_t **pAVStreams)
{
	if (pAVStreams == NULL)
		return -1;
	*pAVStreams = &avStreams;
	return 0;
}

int setAVStreamsParams(b_AVStreams_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&avStreams, in, sizeof(b_AVStreams_t));
	return 0;
}

int getAVStream(b_AVStream_t **pAVStream, int index)
{
	if ((pAVStream == NULL) || (index < 0) || (index > MAX_AVSTREAMS))
		return -1;
	*pAVStream = &avStream[index];
	return 0;
}

int setAVStreamParams(b_AVStream_t *in, int index)
{
	if ((in == NULL) || (index < 0) || (index > MAX_AVSTREAMS))
		return -1;
	BKNI_Memcpy(&avStream[index], in, sizeof(b_AVStream_t));
	return 0;
}

int getAudioDecoder( b_audioDecoder_t **pAudioDecoder, int index)
{
	if ((pAudioDecoder == NULL) || (index < 0) || (index > MAX_AVSTREAMS))
	{
		return -1;
	}
	*pAudioDecoder = &g_audioDecoder[index];
	return 0;
}

int getVideoDecoder( b_videoDecoder_t **pVideoDecoder, int index)
{
	if ((pVideoDecoder == NULL) || (index < 0) || (index > MAX_AVSTREAMS))
	{
		return -1;
	}
	*pVideoDecoder = &g_videoDecoder[index];
	return 0;
}

int getQamModulation (b_QAMModulation_t **pQamModulation)
{
	if (pQamModulation == NULL)
		return -1;
	*pQamModulation = &qamModulation[0];
	return 0;
}

int setQamModulationsParams(b_QAMModulation_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&qamModulation, in, sizeof(b_QAMModulation_t));
	return 0;
}

int getVideoDecoderStats  (b_videoDecoderStats_t **pVideoDecoderStats)
{
	if (pVideoDecoderStats == NULL)
		return -1;
	*pVideoDecoderStats = &videoDecoderStats;
	return 0;
}

int setVideoDecoderStats(b_videoDecoderStats_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&videoDecoderStats, in, sizeof(b_videoDecoderStats_t));
	return 0;
}

int getAudioDecoderStats (b_audioDecoderStats_t **pAudioDecoderStats)
{
	if (pAudioDecoderStats == NULL)
		return -1;
	*pAudioDecoderStats = &audioDecoderStats;
	return 0;
}

int setAudioDecoderStats(b_audioDecoderStats_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&audioDecoderStats, in, sizeof(b_audioDecoderStats_t));
	return 0;
}

int getMPEG2TSStats (b_MPEG2TSStats_t **pMPEG2TSStats)
{
	if (pMPEG2TSStats == NULL)
		return -1;
	*pMPEG2TSStats = &MPEG2TSStats;
	return 0;
}

int setMPEG2TSStats(b_MPEG2TSStats_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&MPEG2TSStats, in, sizeof(b_MPEG2TSStats_t));
	return 0;
}

int getMainStream(b_mainStream_t **pMainStream)
{
	if (pMainStream == NULL)
		return -1;
	*pMainStream = &mainStream[0];
	return 0;
}

int setMainStream(b_mainStream_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&mainStream, in, sizeof(b_mainStream_t));
	return 0;
}


int getTotal(b_total_t **pTotal)
{
	if (pTotal == NULL)
		return -1;
	*pTotal = &total;
	return 0;
}

int setTotal(b_total_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&total, in, sizeof(b_total_t));
	return 0;
}

int getSpdif(b_spdif_t **pSpdif)
{
	if (pSpdif == NULL)
		return -1;
	*pSpdif = &spdif[0];
	return 0;
}

int setSpdif(b_spdif_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&spdif, in, sizeof(b_spdif_t));
	return 0;
}

int getIpInbound(b_IpInbound_t **pInbound)
{
	if (pInbound == NULL)
		return -1;
	*pInbound = &inbound[0];
	return 0;
}

int setIpInbound(b_IpInbound_t *in)
{
	if (in == NULL)
		return -1;
	BKNI_Memcpy(&inbound, in, sizeof(b_IpInbound_t));
	return 0;
}

spectrumAnalyzer* getSpectrumAnalyzer(void)
{
	return &spectrumSettings;
}

int setCenterFreq (spectrumAnalyzer *pSpectrum, int freq)
{
	int span = DEFAULT_SPAN;
	spectrumAnalyzer *spectrum;

	BSTD_UNUSED(pSpectrum);
	spectrum = getSpectrumAnalyzer();

	if ((freq < MIN_FREQ) || (freq > MAX_FREQ))
	{
		return -1;
	}
	else if ((freq - span/2) < MIN_FREQ)
	{
		span = (freq - MIN_FREQ) * 2;
		if (span < MIN_SPAN)
			span = MIN_SPAN;
		spectrum->centerFrequency= MIN_FREQ + span/2;
		spectrum->startFrequency= MIN_FREQ;
		spectrum->stopFrequency= MIN_FREQ + span;
	}
	else if ((freq + span/2) > MAX_FREQ)
	{
		span = (MAX_FREQ - freq) * 2;
		if (span < MIN_SPAN)
			span = MIN_SPAN;
		spectrum->centerFrequency= MAX_FREQ - (span/2);
		spectrum->startFrequency= MAX_FREQ - span;
		spectrum->stopFrequency= MAX_FREQ;
	}
	else
	{
		spectrum->centerFrequency= freq;
		spectrum->startFrequency= freq - span/2;
		spectrum->stopFrequency= freq + span/2;
	}
	return 0;
}

int setNumOfSamples (spectrumAnalyzer *pSpectrum, int numSamples)
{
	if ((pSpectrum == NULL) || (numSamples != 0)) {
		return -1;
	}
	else {
		pSpectrum->numSamples = numSamples;
	}
	return 0;
}


const char *NEXUS_HdmiVideoFormat_string(NEXUS_VideoFormat e){
static char rs[256];
	switch(e) {
	case NEXUS_VideoFormat_eVesa640x480p60hz:
		return "640x480/60Hz";
	case NEXUS_VideoFormat_e480p:
		return "720x480p/60Hz";
	case NEXUS_VideoFormat_e576p:
		return "720x576p/50Hz";
	case NEXUS_VideoFormat_e1080i:
		return "1920x1080i/60Hz";
	case NEXUS_VideoFormat_e1080i50hz:
		return "1920x1080i/50Hz";
	case NEXUS_VideoFormat_e1080p24hz:
		return "1920x1080p/24Hz";
	case NEXUS_VideoFormat_e1080p25hz:
		return "1920x1080p/25Hz";
	case NEXUS_VideoFormat_e1080p30hz:
		return "1920x1080p/30Hz";
	case NEXUS_VideoFormat_e1080p50hz:
		return "1920x1080p/50hz";
	case NEXUS_VideoFormat_e1080p60hz:
		return "1920x1080p/60hz";
	case NEXUS_VideoFormat_e720p:
		return "1280x720p/60Hz";
	case NEXUS_VideoFormat_e720p50hz:
		return "1280x720p/50hz";
	case NEXUS_VideoFormat_e1440x480p60hz:
		return "1440x480p/60hz";
#if 0 /* Need to confirm */
	case NEXUS_VideoFormat_e720p24hz:
		return "1280x720p/24Hz";
	case NEXUS_VideoFormat_e720p25hz:
		return "1280x720p/25Hz";
	case NEXUS_VideoFormat_e720p30hz:
		return "1280x720p/30Hz";
	case NEXUS_VideoFormat_e1250i50hz:
			return "1920x1080i (1250 total)/50Hz";
	case NEXUS_VideoFormat_e240p60hz:
		return "NEXUS_VideoFormat_e240p60hz";
	case NEXUS_VideoFormat_e288p50hz:
		return "NEXUS_VideoFormat_e288p50hz";
	case NEXUS_VideoFormat_e1440x576p50hz:
		return "1440x576p/50hz";
	case NEXUS_VideoFormat_e3840x2160p24hz:
		return "3840x2160p/24hz";
	case NEXUS_VideoFormat_e3840x2160p25hz:
		return "3840x2160p/25hz";
	case NEXUS_VideoFormat_e3840x2160p30hz:
		return "3840x2160p/30hz";
	case NEXUS_VideoFormat_e4096x2160p24hz:
		return "4096x2160p/24hz";
	case NEXUS_VideoFormat_eCustomer1440x240p60hz:
		return "720(1440)x240p/60Hz";
	case NEXUS_VideoFormat_eCustomer1440x288p50hz:
		return "1440x288p/50hz";
	case NEXUS_VideoFormat_eCustomer1366x768p60hz:
		return "1366x768p/60hz";
	case NEXUS_VideoFormat_eCustomer1366x768p50hz:
		return "1366x768p/50hz";
	case NEXUS_VideoFormat_e720p_3DOU_AS:
		return "NEXUS_VideoFormat_e720p_3DOU_AS";
	case NEXUS_VideoFormat_e720p50hz_3DOU_AS:
		return "NEXUS_VideoFormat_e720p50hz_3DOU_AS";
	case NEXUS_VideoFormat_e720p30hz_3DOU_AS:
		return "NEXUS_VideoFormat_e720p30hz_3DOU_AS";
	case NEXUS_VideoFormat_e720p24hz_3DOU_AS:
		return "NEXUS_VideoFormat_e720p24hz_3DOU_AS";
	case NEXUS_VideoFormat_e1080p24hz_3DOU_AS:
		return "NEXUS_VideoFormat_e1080p24hz_3DOU_AS";
	case NEXUS_VideoFormat_e1080p30hz_3DOU_AS:
		return "NEXUS_VideoFormat_e1080p30hz_3DOU_AS";
	case NEXUS_VideoFormat_e720x482_NTSC:
		return "720x482_NTSC";
	case NEXUS_VideoFormat_e720x482_NTSC_J:
		return "720x482_NTSC_J";
	case NEXUS_VideoFormat_e720x483p:
		return "720x483p";
	case NEXUS_VideoFormat_eCustom0:
		return "NEXUS_VideoFormat_eCustom0";
	case NEXUS_VideoFormat_eCustom1:
		return "NEXUS_VideoFormat_eCustom1";
	case NEXUS_VideoFormat_eCustom2:
		return "NEXUS_VideoFormat_eCustom2";
	case NEXUS_VideoFormat_eCustom1920x2160i_48Hz:
		return "NEXUS_VideoFormat_eCustom1920x2160i_48Hz";
	case NEXUS_VideoFormat_eCustom_3D_720p:
		return "NEXUS_VideoFormat_eCustom_3D_720p";
	case NEXUS_VideoFormat_eCustom_3D_720p_50hz:
		return "NEXUS_VideoFormat_eCustom_3D_720p_50hz";
	case NEXUS_VideoFormat_eCustom_3D_720p_30hz:
		return "NEXUS_VideoFormat_eCustom_3D_720p_30hz";
	case NEXUS_VideoFormat_eCustom_3D_720p_24hz:
		return "NEXUS_VideoFormat_eCustom_3D_720p_24hz";
	case NEXUS_VideoFormat_eCustom_3D_1080p_24hz:
		return "NEXUS_VideoFormat_eCustom_3D_1080p_24hz";
	case NEXUS_VideoFormat_eCustom_3D_1080p_30hz:
		return "NEXUS_VideoFormat_eCustom_3D_1080p_30hz";
	case NEXUS_VideoFormat_eMax:
		return "NEXUS_VideoFormat_eMax";
#endif
	default: ;
	}
	sprintf(rs, "%d", e);
	return rs;
}

const char *NEXUS_VideoProtocolProfile_string(NEXUS_VideoProtocolProfile e){
    static char rs[256];
    switch(e) {
    case NEXUS_VideoProtocolProfile_eUnknown:
        return "NEXUS_VideoProtocolProfile_eUnknown";
    case NEXUS_VideoProtocolProfile_eSimple:
        return "NEXUS_VideoProtocolProfile_eSimple";
    case NEXUS_VideoProtocolProfile_eMain:
        return "NEXUS_VideoProtocolProfile_eMain";
    case NEXUS_VideoProtocolProfile_eHigh:
        return "NEXUS_VideoProtocolProfile_eHigh";
    case NEXUS_VideoProtocolProfile_eAdvanced:
        return "NEXUS_VideoProtocolProfile_eAdvanced";
    case NEXUS_VideoProtocolProfile_eJizhun:
        return "NEXUS_VideoProtocolProfile_eJizhun";
    case NEXUS_VideoProtocolProfile_eSnrScalable:
        return "NEXUS_VideoProtocolProfile_eSnrScalable";
    case NEXUS_VideoProtocolProfile_eSpatiallyScalable:
        return "NEXUS_VideoProtocolProfile_eSpatiallyScalable";
    case NEXUS_VideoProtocolProfile_eAdvancedSimple:
        return "NEXUS_VideoProtocolProfile_eAdvancedSimple";
    case NEXUS_VideoProtocolProfile_eBaseline:
        return "NEXUS_VideoProtocolProfile_eBaseline";
    case NEXUS_VideoProtocolProfile_eMax:
        return "NEXUS_VideoProtocolProfile_eMax";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}

const char *NEXUS_MPEG2Part2VideoProtocolProfile_string(NEXUS_VideoProtocolProfile e){
    static char rs[256];
    switch(e) {
	case NEXUS_VideoProtocolProfile_eUnknown:
        return "NEXUS_VideoProtocolProfile_eUnknown";
    case NEXUS_VideoProtocolProfile_eMain:
        return "Main Profile";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}

const char *NEXUS_MPEG4Part2VideoProtocolProfile_string(NEXUS_VideoProtocolProfile e){
    static char rs[256];
    switch(e) {
	case NEXUS_VideoProtocolProfile_eUnknown:
        return "NEXUS_VideoProtocolProfile_eUnknown";
    case NEXUS_VideoProtocolProfile_eSimple:
        return "Simple Profile";
	case NEXUS_VideoProtocolProfile_eAdvancedSimple:
        return "Advanced Simple Profile";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}

const char *NEXUS_MPEG4Part10VideoProtocolProfile_string(NEXUS_VideoProtocolProfile e){
    static char rs[256];
    switch(e) {
	case NEXUS_VideoProtocolProfile_eUnknown:
        return "NEXUS_VideoProtocolProfile_eUnknown";
    case NEXUS_VideoProtocolProfile_eMain:
        return "Main Profile";
	case NEXUS_VideoProtocolProfile_eHigh:
        return "High Profile";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}

const char *NEXUS_SMPTEVC1VideoProtocolProfile_string(NEXUS_VideoProtocolProfile e){
    static char rs[256];
    switch(e) {
	case NEXUS_VideoProtocolProfile_eUnknown:
        return "NEXUS_VideoProtocolProfile_eUnknown";
    case NEXUS_VideoProtocolProfile_eSimple:
        return "Simple Profile";
	case NEXUS_VideoProtocolProfile_eMain:
        return "Main Profile";
	case NEXUS_VideoProtocolProfile_eAdvanced:
        return "Advanced Profile";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}


const char *NEXUS_VideoProtocolLevel_string(NEXUS_VideoProtocolLevel e){
    static char rs[256];
    switch(e) {
    case NEXUS_VideoProtocolLevel_eUnknown:
        return "NEXUS_VideoProtocolLevel_eUnknown";
    case NEXUS_VideoProtocolLevel_e00:
        return "L0";
    case NEXUS_VideoProtocolLevel_e10:
        return "L1";
    case NEXUS_VideoProtocolLevel_e1B:
        return "NEXUS_VideoProtocolLevel_e1B";
    case NEXUS_VideoProtocolLevel_e11:
        return "NEXUS_VideoProtocolLevel_e11";
    case NEXUS_VideoProtocolLevel_e12:
        return "NEXUS_VideoProtocolLevel_e12";
    case NEXUS_VideoProtocolLevel_e13:
        return "NEXUS_VideoProtocolLevel_e13";
    case NEXUS_VideoProtocolLevel_e20:
        return "L2";
    case NEXUS_VideoProtocolLevel_e21:
        return "NEXUS_VideoProtocolLevel_e21";
    case NEXUS_VideoProtocolLevel_e22:
        return "NEXUS_VideoProtocolLevel_e22";
    case NEXUS_VideoProtocolLevel_e30:
        return "L3";
    case NEXUS_VideoProtocolLevel_e31:
        return "NEXUS_VideoProtocolLevel_e31";
    case NEXUS_VideoProtocolLevel_e32:
        return "NEXUS_VideoProtocolLevel_e32";
    case NEXUS_VideoProtocolLevel_e40:
        return "NEXUS_VideoProtocolLevel_e40";
    case NEXUS_VideoProtocolLevel_e41:
        return "4.1";
    case NEXUS_VideoProtocolLevel_e42:
        return "4.2";
    case NEXUS_VideoProtocolLevel_e50:
        return "L5";
    case NEXUS_VideoProtocolLevel_e51:
        return "NEXUS_VideoProtocolLevel_e51";
    case NEXUS_VideoProtocolLevel_e60:
        return "NEXUS_VideoProtocolLevel_e60";
    case NEXUS_VideoProtocolLevel_e62:
        return "NEXUS_VideoProtocolLevel_e62";
    case NEXUS_VideoProtocolLevel_eLow:
        return "LL";
    case NEXUS_VideoProtocolLevel_eMain:
        return "ML";
    case NEXUS_VideoProtocolLevel_eHigh:
        return "HL";
    case NEXUS_VideoProtocolLevel_eHigh1440:
        return "NEXUS_VideoProtocolLevel_eHigh1440";
    case NEXUS_VideoProtocolLevel_eMax:
        return "NEXUS_VideoProtocolLevel_eMax";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}





typedef enum NEXUS_MPEG2Part2VideoProtocolLevel
{
	NEXUS_MPEG2VideoProtocolLevel_eUnknown = 0,
	LL,
	ML,
	HL
}NEXUS_MPEG2Part2VideoProtocolLevel;

const char *NEXUS_MPEG2Part2VideoProtocolLevel_string(NEXUS_VideoProtocolLevel e){
    static char rs[256];
    switch(e) {
	case NEXUS_MPEG2VideoProtocolLevel_eUnknown:
        return "NEXUS_MPEG2VideoProtocolLevel_eUnknown";
    case NEXUS_VideoProtocolLevel_eLow:
        return "LL";
    case NEXUS_VideoProtocolLevel_eMain:
        return "ML";
    case NEXUS_VideoProtocolLevel_eHigh:
        return "HL";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}

typedef enum NEXUS_MPEG4Part2VideoProtocolLevel
{
	NEXUS_MPEG4Part2VideoProtocolLevel_eUnknown = 0,
	L5
}NEXUS_MPEG4Part2VideoProtocolLevel;

const char *NEXUS_MPEG4Part2VideoProtocolLevel_string(NEXUS_VideoProtocolLevel e){
    static char rs[256];
    switch(e) {
	case NEXUS_MPEG4Part2VideoProtocolLevel_eUnknown:
        return "NEXUS_MPEG4Part2VideoProtocolLevel_eUnknown";
    case NEXUS_VideoProtocolLevel_e50:
        return "L5";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}



const char *NEXUS_SMPTEVC1VideoProtocolLevel_string(NEXUS_VideoProtocolLevel e){
    static char rs[256];
    switch(e) {
	case NEXUS_VideoProtocolLevel_eUnknown:
        return "NEXUS_VideoProtocolLevel_eUnknown";
    case NEXUS_VideoProtocolLevel_e00:
        return "L0";
    case NEXUS_VideoProtocolLevel_e10:
        return "L1";
    case NEXUS_VideoProtocolLevel_e20:
        return "L2";
    case NEXUS_VideoProtocolLevel_e30:
        return "L3";
	case NEXUS_VideoProtocolLevel_eLow:
        return "LL";
    case NEXUS_VideoProtocolLevel_eHigh:
        return "HL";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}



const char *NEXUS_VideoFormat_string(NEXUS_VideoFormat e){
    static char rs[256];
    switch(e) {
    case NEXUS_VideoFormat_eUnknown:
        return "NEXUS_VideoFormat_eUnknown";
    case NEXUS_VideoFormat_eNtsc:
        return "NEXUS_VideoFormat_eNtsc";
    case NEXUS_VideoFormat_eNtsc443:
        return "NEXUS_VideoFormat_eNtsc443";
    case NEXUS_VideoFormat_eNtscJapan:
        return "NTSC-J";
    case NEXUS_VideoFormat_ePalM:
        return "PAL-M";
    case NEXUS_VideoFormat_ePalN:
        return "PAL-N";
    case NEXUS_VideoFormat_ePalNc:
        return "NEXUS_VideoFormat_ePalNc";
    case NEXUS_VideoFormat_ePalB:
        return "PAL-B";
    case NEXUS_VideoFormat_ePalB1:
        return "NEXUS_VideoFormat_ePalB1";
    case NEXUS_VideoFormat_ePalD:
        return "PAL-D";
    case NEXUS_VideoFormat_ePalD1:
        return "NEXUS_VideoFormat_ePalD1";
    case NEXUS_VideoFormat_ePalDK2:
        return "NEXUS_VideoFormat_ePalDK2";
    case NEXUS_VideoFormat_ePalDK3:
        return "NEXUS_VideoFormat_ePalDK3";
    case NEXUS_VideoFormat_ePalG:
        return "PAL-G";
    case NEXUS_VideoFormat_ePalH:
        return "PAL-H";
    case NEXUS_VideoFormat_ePalK:
        return "NEXUS_VideoFormat_ePalK";
    case NEXUS_VideoFormat_ePalI:
        return "PAL-I";
    case NEXUS_VideoFormat_ePal60hz:
        return "NEXUS_VideoFormat_ePal60hz";
    case NEXUS_VideoFormat_eSecamL:
        return "SECAM-L";
    case NEXUS_VideoFormat_eSecamB:
        return "SECAM-B";
    case NEXUS_VideoFormat_eSecamG:
        return "SECAM-G";
    case NEXUS_VideoFormat_eSecamD:
        return "SECAM-D";
    case NEXUS_VideoFormat_eSecamK:
        return "SECAM-K";
    case NEXUS_VideoFormat_eSecamH:
        return "SECAM-H";
    case NEXUS_VideoFormat_e480p:
        return "640x480p/60Hz";
    case NEXUS_VideoFormat_e576p:
        return "NEXUS_VideoFormat_e576p";
    case NEXUS_VideoFormat_e1080i:
        return "1920x1080i/60Hz";
    case NEXUS_VideoFormat_e1080i50hz:
        return "1920x1080i/50Hz";
    case NEXUS_VideoFormat_e1080p24hz:
        return "1920x1080p/24Hz";
    case NEXUS_VideoFormat_e1080p25hz:
        return "1920x1080p/25Hz";
    case NEXUS_VideoFormat_e1080p30hz:
        return "1920x1080p/30Hz";
    case NEXUS_VideoFormat_e1080p50hz:
        return "1920x1080p/50Hz";
    case NEXUS_VideoFormat_e1080p60hz:
        return "1920x1080p/60Hz";
    case NEXUS_VideoFormat_e1250i50hz:
        return "1250i50hz";
    case NEXUS_VideoFormat_e720p:
        return "1280x720p/60Hz";
    case NEXUS_VideoFormat_e720p50hz:
        return "1280x720p/50hz";
    case NEXUS_VideoFormat_e720p24hz:
        return "1280x720p/24hz";
    case NEXUS_VideoFormat_e720p25hz:
        return "1280x720p/25hz";
    case NEXUS_VideoFormat_e720p30hz:
        return "1280x720p/30hz";
    case NEXUS_VideoFormat_e240p60hz:
        return "NEXUS_VideoFormat_e240p60hz";
    case NEXUS_VideoFormat_e288p50hz:
        return "NEXUS_VideoFormat_e288p50hz";
    case NEXUS_VideoFormat_e1440x480p60hz:
        return "1440x480p/60hz";
    case NEXUS_VideoFormat_e1440x576p50hz:
        return "1440x576p/50hz";
    case NEXUS_VideoFormat_e3840x2160p24hz:
        return "3840x2160p/24hz";
    case NEXUS_VideoFormat_e3840x2160p25hz:
        return "3840x2160p/25hz";
    case NEXUS_VideoFormat_e3840x2160p30hz:
        return "3840x2160p/30hz";
    case NEXUS_VideoFormat_e4096x2160p24hz:
        return "e4096x2160p/24hz";
    case NEXUS_VideoFormat_eCustomer1440x240p60hz:
        return "720(1440)x240p/60hz";
    case NEXUS_VideoFormat_eCustomer1440x288p50hz:
        return "1440x288p/50hz";
    case NEXUS_VideoFormat_eCustomer1366x768p60hz:
        return "1366x768p/60hz";
    case NEXUS_VideoFormat_eCustomer1366x768p50hz:
        return "1366x768p/50hz";
    case NEXUS_VideoFormat_eVesa640x480p60hz:
        return "NEXUS_VideoFormat_eVesa640x480p60hz";
    case NEXUS_VideoFormat_eVesa800x600p60hz:
        return "NEXUS_VideoFormat_eVesa800x600p60hz";
    case NEXUS_VideoFormat_eVesa1024x768p60hz:
        return "NEXUS_VideoFormat_eVesa1024x768p60hz";
    case NEXUS_VideoFormat_eVesa1280x768p60hz:
        return "NEXUS_VideoFormat_eVesa1280x768p60hz";
    case NEXUS_VideoFormat_eVesa1280x768p60hzRed:
        return "NEXUS_VideoFormat_eVesa1280x768p60hzRed";
    case NEXUS_VideoFormat_eVesa1280x720p50hz:
        return "NEXUS_VideoFormat_eVesa1280x720p50hz";
    case NEXUS_VideoFormat_eVesa1280x720p60hz:
        return "NEXUS_VideoFormat_eVesa1280x720p60hz";
    case NEXUS_VideoFormat_eVesa1280x720pReducedBlank:
        return "NEXUS_VideoFormat_eVesa1280x720pReducedBlank";
    case NEXUS_VideoFormat_eVesa640x350p60hz:
        return "NEXUS_VideoFormat_eVesa640x350p60hz";
    case NEXUS_VideoFormat_eVesa640x350p70hz:
        return "NEXUS_VideoFormat_eVesa640x350p70hz";
    case NEXUS_VideoFormat_eVesa640x350p72hz:
        return "NEXUS_VideoFormat_eVesa640x350p72hz";
    case NEXUS_VideoFormat_eVesa640x350p75hz:
        return "NEXUS_VideoFormat_eVesa640x350p75hz";
    case NEXUS_VideoFormat_eVesa640x350p85hz:
        return "NEXUS_VideoFormat_eVesa640x350p85hz";
    case NEXUS_VideoFormat_eVesa640x400p60hz:
        return "NEXUS_VideoFormat_eVesa640x400p60hz";
    case NEXUS_VideoFormat_eVesa640x400p70hz:
        return "NEXUS_VideoFormat_eVesa640x400p70hz";
    case NEXUS_VideoFormat_eVesa640x400p72hz:
        return "NEXUS_VideoFormat_eVesa640x400p72hz";
    case NEXUS_VideoFormat_eVesa640x400p75hz:
        return "NEXUS_VideoFormat_eVesa640x400p75hz";
    case NEXUS_VideoFormat_eVesa640x400p85hz:
        return "NEXUS_VideoFormat_eVesa640x400p85hz";
    case NEXUS_VideoFormat_eVesa640x480p66hz:
        return "NEXUS_VideoFormat_eVesa640x480p66hz";
    case NEXUS_VideoFormat_eVesa640x480p70hz:
        return "NEXUS_VideoFormat_eVesa640x480p70hz";
    case NEXUS_VideoFormat_eVesa640x480p72hz:
        return "NEXUS_VideoFormat_eVesa640x480p72hz";
    case NEXUS_VideoFormat_eVesa640x480p75hz:
        return "NEXUS_VideoFormat_eVesa640x480p75hz";
    case NEXUS_VideoFormat_eVesa640x480p85hz:
        return "NEXUS_VideoFormat_eVesa640x480p85hz";
    case NEXUS_VideoFormat_eVesa720x400p60hz:
        return "NEXUS_VideoFormat_eVesa720x400p60hz";
    case NEXUS_VideoFormat_eVesa720x400p70hz:
        return "NEXUS_VideoFormat_eVesa720x400p70hz";
    case NEXUS_VideoFormat_eVesa720x400p72hz:
        return "NEXUS_VideoFormat_eVesa720x400p72hz";
    case NEXUS_VideoFormat_eVesa720x400p75hz:
        return "NEXUS_VideoFormat_eVesa720x400p75hz";
    case NEXUS_VideoFormat_eVesa720x400p85hz:
        return "NEXUS_VideoFormat_eVesa720x400p85hz";
    case NEXUS_VideoFormat_eVesa800x600p56hz:
        return "NEXUS_VideoFormat_eVesa800x600p56hz";
    case NEXUS_VideoFormat_eVesa800x600p59hzRed:
        return "NEXUS_VideoFormat_eVesa800x600p59hzRed";
    case NEXUS_VideoFormat_eVesa800x600p70hz:
        return "NEXUS_VideoFormat_eVesa800x600p70hz";
    case NEXUS_VideoFormat_eVesa800x600p72hz:
        return "NEXUS_VideoFormat_eVesa800x600p72hz";
    case NEXUS_VideoFormat_eVesa800x600p75hz:
        return "NEXUS_VideoFormat_eVesa800x600p75hz";
    case NEXUS_VideoFormat_eVesa800x600p85hz:
        return "NEXUS_VideoFormat_eVesa800x600p85hz";
    case NEXUS_VideoFormat_eVesa848x480p60hz:
        return "NEXUS_VideoFormat_eVesa848x480p60hz";
    case NEXUS_VideoFormat_eVesa1024x768p66hz:
        return "NEXUS_VideoFormat_eVesa1024x768p66hz";
    case NEXUS_VideoFormat_eVesa1024x768p70hz:
        return "NEXUS_VideoFormat_eVesa1024x768p70hz";
    case NEXUS_VideoFormat_eVesa1024x768p72hz:
        return "NEXUS_VideoFormat_eVesa1024x768p72hz";
    case NEXUS_VideoFormat_eVesa1024x768p75hz:
        return "NEXUS_VideoFormat_eVesa1024x768p75hz";
    case NEXUS_VideoFormat_eVesa1024x768p85hz:
        return "NEXUS_VideoFormat_eVesa1024x768p85hz";
    case NEXUS_VideoFormat_eVesa1064x600p60hz:
        return "NEXUS_VideoFormat_eVesa1064x600p60hz";
    case NEXUS_VideoFormat_eVesa1280x720p70hz:
        return "NEXUS_VideoFormat_eVesa1280x720p70hz";
    case NEXUS_VideoFormat_eVesa1280x720p72hz:
        return "NEXUS_VideoFormat_eVesa1280x720p72hz";
    case NEXUS_VideoFormat_eVesa1280x720p75hz:
        return "NEXUS_VideoFormat_eVesa1280x720p75hz";
    case NEXUS_VideoFormat_eVesa1280x720p85hz:
        return "NEXUS_VideoFormat_eVesa1280x720p85hz";
    case NEXUS_VideoFormat_eVesa1024x768i87hz:
        return "NEXUS_VideoFormat_eVesa1024x768i87hz";
    case NEXUS_VideoFormat_eVesa1152x864p75hz:
        return "NEXUS_VideoFormat_eVesa1152x864p75hz";
    case NEXUS_VideoFormat_eVesa1280x768p75hz:
        return "NEXUS_VideoFormat_eVesa1280x768p75hz";
    case NEXUS_VideoFormat_eVesa1280x768p85hz:
        return "NEXUS_VideoFormat_eVesa1280x768p85hz";
    case NEXUS_VideoFormat_eVesa1280x800p_60Hz:
        return "NEXUS_VideoFormat_eVesa1280x800p_60Hz";
    case NEXUS_VideoFormat_eVesa1280x960p60hz:
        return "NEXUS_VideoFormat_eVesa1280x960p60hz";
    case NEXUS_VideoFormat_eVesa1280x960p85hz:
        return "NEXUS_VideoFormat_eVesa1280x960p85hz";
    case NEXUS_VideoFormat_eVesa1280x1024p60hz:
        return "NEXUS_VideoFormat_eVesa1280x1024p60hz";
    case NEXUS_VideoFormat_eVesa1280x1024p69hz:
        return "NEXUS_VideoFormat_eVesa1280x1024p69hz";
    case NEXUS_VideoFormat_eVesa1280x1024p75hz:
        return "NEXUS_VideoFormat_eVesa1280x1024p75hz";
    case NEXUS_VideoFormat_eVesa1280x1024p85hz:
        return "NEXUS_VideoFormat_eVesa1280x1024p85hz";
    case NEXUS_VideoFormat_eVesa832x624p75hz:
        return "NEXUS_VideoFormat_eVesa832x624p75hz";
    case NEXUS_VideoFormat_eVesa1360x768p60hz:
        return "NEXUS_VideoFormat_eVesa1360x768p60hz";
    case NEXUS_VideoFormat_eVesa1366x768p60hz:
        return "NEXUS_VideoFormat_eVesa1366x768p60hz";
    case NEXUS_VideoFormat_eVesa1400x1050p60hz:
        return "NEXUS_VideoFormat_eVesa1400x1050p60hz";
    case NEXUS_VideoFormat_eVesa1400x1050p60hzReducedBlank:
        return "NEXUS_VideoFormat_eVesa1400x1050p60hzReducedBlank";
    case NEXUS_VideoFormat_eVesa1400x1050p75hz:
        return "NEXUS_VideoFormat_eVesa1400x1050p75hz";
    case NEXUS_VideoFormat_eVesa1440x900p60hz:
        return "NEXUS_VideoFormat_eVesa1440x900p60hz";
    case NEXUS_VideoFormat_eVesa1600x1200p60hz:
        return "NEXUS_VideoFormat_eVesa1600x1200p60hz";
    case NEXUS_VideoFormat_eVesa1920x1080p60hzReducedBlank:
        return "NEXUS_VideoFormat_eVesa1920x1080p60hzReducedBlank";
    case NEXUS_VideoFormat_e720p_3DOU_AS:
        return "NEXUS_VideoFormat_e720p_3DOU_AS";
    case NEXUS_VideoFormat_e720p50hz_3DOU_AS:
        return "NEXUS_VideoFormat_e720p50hz_3DOU_AS";
    case NEXUS_VideoFormat_e720p30hz_3DOU_AS:
        return "NEXUS_VideoFormat_e720p30hz_3DOU_AS";
    case NEXUS_VideoFormat_e720p24hz_3DOU_AS:
        return "NEXUS_VideoFormat_e720p24hz_3DOU_AS";
    case NEXUS_VideoFormat_e1080p24hz_3DOU_AS:
        return "NEXUS_VideoFormat_e1080p24hz_3DOU_AS";
    case NEXUS_VideoFormat_e1080p30hz_3DOU_AS:
        return "NEXUS_VideoFormat_e1080p30hz_3DOU_AS";
    case NEXUS_VideoFormat_eVesa1680x1050p_60Hz:
        return "NEXUS_VideoFormat_eVesa1680x1050p_60Hz";
    case NEXUS_VideoFormat_eVesa1280x800p_60Hz_Red:
        return "NEXUS_VideoFormat_eVesa1280x800p_60Hz_Red";
    case NEXUS_VideoFormat_eVesa1600x1200p_75Hz:
        return "NEXUS_VideoFormat_eVesa1600x1200p_75Hz";
    case NEXUS_VideoFormat_eVesa1600x900p_60Hz_Red:
        return "NEXUS_VideoFormat_eVesa1600x900p_60Hz_Red";
    case NEXUS_VideoFormat_eVesa1680x1050p_60Hz_Red:
        return "NEXUS_VideoFormat_eVesa1680x1050p_60Hz_Red";
    case NEXUS_VideoFormat_eVesa1920x1200p_60Hz:
        return "NEXUS_VideoFormat_eVesa1920x1200p_60Hz";
    case NEXUS_VideoFormat_eVesa1920x1200p_60Hz_Red:
        return "NEXUS_VideoFormat_eVesa1920x1200p_60Hz_Red";
    case NEXUS_VideoFormat_eVesa1152x720p_60Hz:
        return "NEXUS_VideoFormat_eVesa1152x720p_60Hz";
    case NEXUS_VideoFormat_eVesa1152x720p_75Hz:
        return "NEXUS_VideoFormat_eVesa1152x720p_75Hz";
    case NEXUS_VideoFormat_eVesa1152x720p_85Hz:
        return "NEXUS_VideoFormat_eVesa1152x720p_85Hz";
    case NEXUS_VideoFormat_eVesa1152x864p_60Hz:
        return "NEXUS_VideoFormat_eVesa1152x864p_60Hz";
    case NEXUS_VideoFormat_eVesa1152x864p_85Hz:
        return "NEXUS_VideoFormat_eVesa1152x864p_85Hz";
    case NEXUS_VideoFormat_eVesa1152x870p_75Hz:
        return "NEXUS_VideoFormat_eVesa1152x870p_75Hz";
    case NEXUS_VideoFormat_eVesa1152x900p_66Hz:
        return "NEXUS_VideoFormat_eVesa1152x900p_66Hz";
    case NEXUS_VideoFormat_eVesa1152x900p_76Hz:
        return "NEXUS_VideoFormat_eVesa1152x900p_76Hz";
    case NEXUS_VideoFormat_eVesa1170x584p_50Hz:
        return "NEXUS_VideoFormat_eVesa1170x584p_50Hz";
    case NEXUS_VideoFormat_eVesa1280x1024p_70Hz:
        return "NEXUS_VideoFormat_eVesa1280x1024p_70Hz";
    case NEXUS_VideoFormat_eVesa1280x1024p_72Hz:
        return "NEXUS_VideoFormat_eVesa1280x1024p_72Hz";
    case NEXUS_VideoFormat_eVesa1280x1024p_76Hz:
        return "NEXUS_VideoFormat_eVesa1280x1024p_76Hz";
    case NEXUS_VideoFormat_eVesa1280x768p_50Hz:
        return "NEXUS_VideoFormat_eVesa1280x768p_50Hz";
    case NEXUS_VideoFormat_eVesa1280x960p_75Hz:
        return "NEXUS_VideoFormat_eVesa1280x960p_75Hz";
    case NEXUS_VideoFormat_eVesa1600x1024p_60Hz:
        return "NEXUS_VideoFormat_eVesa1600x1024p_60Hz";
    case NEXUS_VideoFormat_eVesa1600x1024p_76Hz:
        return "NEXUS_VideoFormat_eVesa1600x1024p_76Hz";
    case NEXUS_VideoFormat_eVesa1728x1080p_60Hz:
        return "NEXUS_VideoFormat_eVesa1728x1080p_60Hz";
    case NEXUS_VideoFormat_eVesa800x600p_100Hz:
        return "NEXUS_VideoFormat_eVesa800x600p_100Hz";
    case NEXUS_VideoFormat_eVesa800x600p_90Hz:
        return "NEXUS_VideoFormat_eVesa800x600p_90Hz";
    case NEXUS_VideoFormat_eVesa848x480p_75Hz:
        return "NEXUS_VideoFormat_eVesa848x480p_75Hz";
    case NEXUS_VideoFormat_eVesa848x480p_85Hz:
        return "NEXUS_VideoFormat_eVesa848x480p_85Hz";
    case NEXUS_VideoFormat_eVesa852x480p_60Hz:
        return "NEXUS_VideoFormat_eVesa852x480p_60Hz";
    case NEXUS_VideoFormat_e720x482_NTSC:
        return "720x482_NTSC";
    case NEXUS_VideoFormat_e720x482_NTSC_J:
        return "720x482_NTSC_J";
    case NEXUS_VideoFormat_e720x483p:
        return "720x483p";
    case NEXUS_VideoFormat_eCustom0:
        return "NEXUS_VideoFormat_eCustom0";
    case NEXUS_VideoFormat_eCustom1:
        return "NEXUS_VideoFormat_eCustom1";
    case NEXUS_VideoFormat_eCustom2:
        return "NEXUS_VideoFormat_eCustom2";
    case NEXUS_VideoFormat_eCustom1920x2160i_48Hz:
        return "NEXUS_VideoFormat_eCustom1920x2160i_48Hz";
    case NEXUS_VideoFormat_eCustom_3D_720p:
        return "NEXUS_VideoFormat_eCustom_3D_720p";
    case NEXUS_VideoFormat_eCustom_3D_720p_50hz:
        return "NEXUS_VideoFormat_eCustom_3D_720p_50hz";
    case NEXUS_VideoFormat_eCustom_3D_720p_30hz:
        return "NEXUS_VideoFormat_eCustom_3D_720p_30hz";
    case NEXUS_VideoFormat_eCustom_3D_720p_24hz:
        return "NEXUS_VideoFormat_eCustom_3D_720p_24hz";
    case NEXUS_VideoFormat_eCustom_3D_1080p_24hz:
        return "NEXUS_VideoFormat_eCustom_3D_1080p_24hz";
    case NEXUS_VideoFormat_eCustom_3D_1080p_30hz:
        return "NEXUS_VideoFormat_eCustom_3D_1080p_30hz";
    case NEXUS_VideoFormat_eMax:
        return "NEXUS_VideoFormat_eMax";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}

const char *NEXUS_VideoFrameRate_string(NEXUS_VideoFrameRate e){
    static char rs[256];
    switch(e) {
    case NEXUS_VideoFrameRate_eUnknown:
        return "NEXUS_VideoFrameRate_eUnknown";
    case NEXUS_VideoFrameRate_e23_976:
        return "NEXUS_VideoFrameRate_e23_976";
    case NEXUS_VideoFrameRate_e24:
        return "NEXUS_VideoFrameRate_e24";
    case NEXUS_VideoFrameRate_e25:
        return "NEXUS_VideoFrameRate_e25";
    case NEXUS_VideoFrameRate_e29_97:
        return "NEXUS_VideoFrameRate_e29_97";
    case NEXUS_VideoFrameRate_e30:
        return "NEXUS_VideoFrameRate_e30";
    case NEXUS_VideoFrameRate_e50:
        return "NEXUS_VideoFrameRate_e50";
    case NEXUS_VideoFrameRate_e59_94:
        return "NEXUS_VideoFrameRate_e59_94";
    case NEXUS_VideoFrameRate_e60:
        return "NEXUS_VideoFrameRate_e60";
    case NEXUS_VideoFrameRate_e14_985:
        return "NEXUS_VideoFrameRate_e14_985";
    case NEXUS_VideoFrameRate_e7_493:
        return "NEXUS_VideoFrameRate_e7_493";
    case NEXUS_VideoFrameRate_e10:
        return "NEXUS_VideoFrameRate_e10";
    case NEXUS_VideoFrameRate_e15:
        return "NEXUS_VideoFrameRate_e15";
    case NEXUS_VideoFrameRate_e20:
        return "NEXUS_VideoFrameRate_e20";
    case NEXUS_VideoFrameRate_e12_5:
        return "NEXUS_VideoFrameRate_e12_5";
    case NEXUS_VideoFrameRate_eMax:
        return "NEXUS_VideoFrameRate_eMax";
    default: ;
    }
    sprintf(rs, "%d", e);
    return rs;
}


const char *videoMPEG2Part2Profile_string[] = {"Simple Profile", "Main Profile", "High Profile",
													"SNR Scalable", "Spatially Scalable"};

const nameValue compositeVideoStandard[] = {
	{"NTSC-J", NEXUS_VideoFormat_eNtsc},
	{"NTSC-4.43", NEXUS_VideoFormat_eNtsc443},
	{"PAL-B", NEXUS_VideoFormat_ePalB},
	{"PAL-D", NEXUS_VideoFormat_ePalD},
	{"PAL-G", NEXUS_VideoFormat_ePalG},
	{"PAL-H", NEXUS_VideoFormat_ePalH},
	{"PAL-I", NEXUS_VideoFormat_ePalI},
	{"PAL-N", NEXUS_VideoFormat_ePalN},
	{"PAL-M", NEXUS_VideoFormat_ePalM},
	{"SECAM-B", NEXUS_VideoFormat_eSecamB},
	{"SECAM-G", NEXUS_VideoFormat_eSecamG},
	{"SECAM-H", NEXUS_VideoFormat_eSecamH},
	{"SECAM-D", NEXUS_VideoFormat_eSecamD},
	{"SECAM-K", NEXUS_VideoFormat_eSecamK},
	{"SECAM-L", NEXUS_VideoFormat_eSecamL},
	{NULL, 0}
};


const char *videoCodecs[] = {"NEXUS_VideoCodec_eMpeg1", "NEXUS_VideoCodec_eMpeg2","NEXUS_VideoCodec_eMpeg4Part2",
									"NEXUS_VideoCodec_eH263", "NEXUS_VideoCodec_eH264", "NEXUS_VideoCodec_eH264_Svc",
									"NEXUS_VideoCodec_eH264_Mvc", "NEXUS_VideoCodec_eVc1", "NEXUS_VideoCodec_eVc1SimpleMain",
									"NEXUS_VideoCodec_eDivx311", "NEXUS_VideoCodec_eAvs", "NEXUS_VideoCodec_eRv40",
									"NEXUS_VideoCodec_eVp6", "NEXUS_VideoCodec_eVp7", "NEXUS_VideoCodec_eVp8",
									"NEXUS_VideoCodec_eSpark", "NEXUS_VideoCodec_eMotionJpeg", "NEXUS_VideoCodec_eH265"};

const nameValue videoCodecStandard[] = {
#if SUPPORT
	{"Unknown", NEXUS_VideoCodec_eUnknown},					/* unknown/not supported video codec */
	{"MPEG1", NEXUS_VideoCodec_eMpeg1},						/* MPEG-1 Video (ISO/IEC 11172-2) */
#endif
	{"MPEG2-Part2", NEXUS_VideoCodec_eMpeg2},				/* MPEG-2 Video (ISO/IEC 13818-2) */
	{"MPEG4-Part2", NEXUS_VideoCodec_eMpeg4Part2},			/* MPEG-4 Part 2 Video */
#if SUPPORT
	{"H263", NEXUS_VideoCodec_eH263},						/* H.263 Video. The value of the enum is not based on PSI standards. */
#endif
	{"MPEG4-Part10", NEXUS_VideoCodec_eH264},				/* H.264 (ITU-T) or ISO/IEC 14496-10/MPEG-4 AVC */
#if SUPPORT
	{"H264_Svc", NEXUS_VideoCodec_eH264_Svc},
    {"H264_Mvc", NEXUS_VideoCodec_eH264_Mvc},				/* Multi View Coding extension of H.264 */
#endif
	{"SMPTE-VC1", NEXUS_VideoCodec_eVc1},					/* VC-1 Advanced Profile */
#if SUPPORT
	{"Vc1SimpleMain", NEXUS_VideoCodec_eVc1SimpleMain},		/* VC-1 Simple & Main Profile */
    {"Divx311", NEXUS_VideoCodec_eDivx311},					/* DivX 3.11 coded video */
	{"Avs", NEXUS_VideoCodec_eAvs},							/* AVS coded video */
	{"Rv40", NEXUS_VideoCodec_eRv40},						/* RV 4.0 coded video */
	{"Vp6", NEXUS_VideoCodec_eVp6},							/* VP6 coded video */
	{"Vp7", NEXUS_VideoCodec_eVp7},							/* VP7 coded video */
	{"Vp8", NEXUS_VideoCodec_eVp8},							/* VP8 coded video */
	{"Spark", NEXUS_VideoCodec_eSpark},						/* H.263 Sorenson Spark coded video */
	{"MotionJpeg", NEXUS_VideoCodec_eMotionJpeg},			/* Motion Jpeg video codec */
	{"H265", NEXUS_VideoCodec_eH265},						/* HEVC,H.265 ITU-T SG16 WP3 and ISO/IEC JTC1/SC29/WG11 */
	{"Max", NEXUS_VideoCodec_eMax},
#endif
	{NULL, 0}
};

const nameValue videoOutputDisplayFormat[] = {
	{"Unknown", NEXUS_AspectRatio_eUnknown},
	{"Square Pixel", NEXUS_AspectRatio_eSquarePixel},/* Square pixel. This is equivalent to NEXUS_AspectRatio_eSar 1:1. */
	{"4:3", NEXUS_AspectRatio_e4x3},
	{"16:9", NEXUS_AspectRatio_e16x9},
	{"2.21:1", NEXUS_AspectRatio_e221x1},
	{"15:9", NEXUS_AspectRatio_e15x9},
	{"SAR", NEXUS_AspectRatio_eSar},
	{NULL, 0}
};

const nameValue audioCodecStandard [] = {
	{"MPEG1/2-Part3-Layer2", NEXUS_AudioCodec_eMpeg},
	{"MPEG1/2-Part3-Layer3", NEXUS_AudioCodec_eMp3},
	{"MPEG2-AAC-LC/MPEG4-AAC-LC", NEXUS_AudioCodec_eAac},
	{"AacLoas", NEXUS_AudioCodec_eAacLoas},
	{"MPEG4-AAC-HE-v2", NEXUS_AudioCodec_eAacPlus},
	{"AacPlusAdts", NEXUS_AudioCodec_eAacPlusAdts},
	{"DOLBY-AC3", NEXUS_AudioCodec_eAc3},
	{"DOLBY-DD+", NEXUS_AudioCodec_eAc3Plus},
	{"DTS", NEXUS_AudioCodec_eDts},
	{"LpcmDvd", NEXUS_AudioCodec_eLpcmDvd},
	{"LpcmBluRay", NEXUS_AudioCodec_eLpcmBluRay},
	{"DTS-HD", NEXUS_AudioCodec_eDtsHd},
	{"WmaStd", NEXUS_AudioCodec_eWmaStd},
	{"WmaStdTs", NEXUS_AudioCodec_eWmaStdTs},
	{"WmaPro", NEXUS_AudioCodec_eWmaPro},
	{"Pcm", NEXUS_AudioCodec_ePcm},
	{"PcmWav", NEXUS_AudioCodec_ePcmWav},
	{"AmrNb", NEXUS_AudioCodec_eAmrNb},
	{"AmrWb", NEXUS_AudioCodec_eAmrWb},
	{"Dra", NEXUS_AudioCodec_eDra},
	{"Cook", NEXUS_AudioCodec_eCook},
	{"Adpcm", NEXUS_AudioCodec_eAdpcm},
	{"DtsCd", NEXUS_AudioCodec_eDtsCd},
	{"DtsExpress", NEXUS_AudioCodec_eDtsExpress},
	{"Vorbis", NEXUS_AudioCodec_eVorbis},
	{"Lpcm1394", NEXUS_AudioCodec_eLpcm1394},
	{"G711", NEXUS_AudioCodec_eG711},
	{"G723_1", NEXUS_AudioCodec_eG723_1},
	{"G726", NEXUS_AudioCodec_eG726},
	{"G729", NEXUS_AudioCodec_eG729},
	{"Flac", NEXUS_AudioCodec_eFlac},
	{"Mlp", NEXUS_AudioCodec_eMlp},
	{"Ape", NEXUS_AudioCodec_eApe},
	{"Ilbc", NEXUS_AudioCodec_eIlbc},           /* Internet Low Bitrate Codec (see http://www.webrtc.org/ilbc-freeware)*/
    {"Isac", NEXUS_AudioCodec_eIsac},           /* Internet Speech Audio Codec */
	{NULL, 0}
};

const nameValue audioOutputMode [] = {
	{"None", NxClient_AudioOutputMode_eNone}, /* no audio */
	{"Auto", NxClient_AudioOutputMode_eAuto}, /* Use server defaults and/or HDMI EDID capabilities. */
	{"PCM", NxClient_AudioOutputMode_ePcm}, /* stereo or mono PCM. See channelMode for left/right selection. */
	{"MultichannelPcm", NxClient_AudioOutputMode_eMultichannelPcm},
	{"Passthrough", NxClient_AudioOutputMode_ePassthrough}, /* compressed output.
        Passthrough of AC3+ to SPDIF automatically does a downconvert to AC3. */
	{"Transcode", NxClient_AudioOutputMode_eTranscode},  /* transcode to compressed output using 'transcodeCodec'.
        The encoder does not support all codecs as the decoder. */
	{NULL, 0}
};


const nameValue videoLevelStrs[] = {
#if NEXUS_HAS_VIDEO_ENCODER
    {"0.0", NEXUS_VideoCodecLevel_e00},
    {"1.0", NEXUS_VideoCodecLevel_e10},
    {"1.b", NEXUS_VideoCodecLevel_e1B},
    {"1.1", NEXUS_VideoCodecLevel_e11},
    {"1.2", NEXUS_VideoCodecLevel_e12},
    {"1.3", NEXUS_VideoCodecLevel_e13},
    {"2.0", NEXUS_VideoCodecLevel_e20},
    {"2.1", NEXUS_VideoCodecLevel_e21},
    {"2.2", NEXUS_VideoCodecLevel_e22},
    {"3.0", NEXUS_VideoCodecLevel_e30},
    {"3.1", NEXUS_VideoCodecLevel_e31},
    {"3.2", NEXUS_VideoCodecLevel_e32},
    {"4.0", NEXUS_VideoCodecLevel_e40},
    {"4.1", NEXUS_VideoCodecLevel_e41},
    {"4.2", NEXUS_VideoCodecLevel_e42},
    {"5.0", NEXUS_VideoCodecLevel_e50},
    {"5.1", NEXUS_VideoCodecLevel_e51},
    {"6.0", NEXUS_VideoCodecLevel_e60},
    {"6.2", NEXUS_VideoCodecLevel_e62},
    {"low", NEXUS_VideoCodecLevel_eLow},
    {"main", NEXUS_VideoCodecLevel_eMain},
    {"high", NEXUS_VideoCodecLevel_eHigh},
    {"high1440", NEXUS_VideoCodecLevel_eHigh1440},
#endif
    {NULL, 0}
};



const nameValue videoProfileStrs[] = {
#if NEXUS_HAS_VIDEO_ENCODER
    {"Simple Profile", NEXUS_VideoCodecProfile_eSimple},
    {"Main Profile", NEXUS_VideoCodecProfile_eMain},
    {"High Profile", NEXUS_VideoCodecProfile_eHigh},
    {"Advanced profile", NEXUS_VideoCodecProfile_eAdvanced},
    {"SNR Scalable", NEXUS_VideoCodecProfile_eSnrScalable},
    {"Spatially Scalable", NEXUS_VideoCodecProfile_eSpatiallyScalable},
    {"Advanced Simple", NEXUS_VideoCodecProfile_eAdvancedSimple},
    {"Baseline", NEXUS_VideoCodecProfile_eBaseline},
#endif
    {NULL, 0}
};

const nameValue hdmiVideoFormat [] = {
	{"640x480/60Hz", NEXUS_VideoFormat_eVesa640x480p60hz},
	{"720x480p/60Hz", NEXUS_VideoFormat_e480p},
	{"720x576p/50Hz", NEXUS_VideoFormat_e576p},
	{"1920x1080i/60Hz", NEXUS_VideoFormat_e1080i},
	{"1920x1080i/50Hz", NEXUS_VideoFormat_e1080i50hz},
	{"1920x1080p/24Hz", NEXUS_VideoFormat_e1080p24hz},
	{"1920x1080p/25Hz", NEXUS_VideoFormat_e1080p25hz},
	{"1920x1080p/30Hz", NEXUS_VideoFormat_e1080p30hz},
	{"1920x1080p/50hz", NEXUS_VideoFormat_e1080p50hz},
	{"1920x1080p/60hz", NEXUS_VideoFormat_e1080p60hz},
	{"1280x720p/60Hz", NEXUS_VideoFormat_e720p},
	{"1280x720p/50hz", NEXUS_VideoFormat_e720p50hz},
	{"1440x480p/60hz", NEXUS_VideoFormat_e1440x480p60hz},
	{NULL, 0}
};

const nameValue videoOutputFormats[] = {
	{"CVBS", 1},
	{"YPrPb", 2},
	{"RGsB", 3},
	{"RGB", 4},
	{"HDMI", 5},
	{"DVI", 6},
#if BCHP_CHIP == 7428 || BCHP_CHIP == 7429
	{"RF", 7},
#endif
	{NULL, 0}
};

const nameValue audioOutputFormat[] = {
	{"ANALOG-MONO", 1},
	{"ANALOG-2-CHANNELS", 2},
	{"ANALOG-5.1-CHANNELS", 2},
	{"DIGITAL-OPTICAL-SP/DIF", 2},
	{"DIGITAL-COAXIAL-SP/DIF", 2},
	{"HDMI", 2},
	{"RF", 2},
	{"HDMI-COMPRESSED4x", 2},
	{"HDMI-COMPRESSED16x", 2},
	{NULL, 0}
};

const char *enumToString(const nameValue *table, int value)
{
	unsigned i;
	for (i=0;table[i].name;i++) {
		if (table[i].value == value) {
			return table[i].name;
		}
	}
	return "";
}

unsigned stringToEnum(const nameValue *table, const char *name)
{
	unsigned i;
	unsigned value;
	char *endptr;
	for (i=0;table[i].name;i++) {
		if (!strcasecmp(table[i].name, name)) {
			return table[i].value;
		}
	}
	value = strtol(name, &endptr, 0);
	if(!endptr || *endptr) { /* if valid, *endptr = '\0' */
		value = table[0].value;
	}
	printf("Unknown string '%s'\n", name);
	return value;
}

void printList(const nameValue *table)
{
    unsigned i;
    const char *sep=" {";
    for (i=0;table[i].name;i++) {
        /* skip aliases */
        if (i > 0 && table[i].value == table[i-1].value) continue;
        printf("%s%s",sep,table[i].name);
        sep = ", ";
    }
    printf("}");
}

const char *strList(const nameValue *table)
{
	unsigned i, len=0;
	static char *rs;
	const char *sep="{";
	rs = malloc(1000);
	for (i=0;table[i].name;i++) {
        /* skip aliases */
        if (i > 0 && table[i].value == table[i-1].value) continue;
        len += sprintf(rs+len+2,"%s%s",sep,table[i].name);
        sep = ", ";
    }
	return rs;
}

