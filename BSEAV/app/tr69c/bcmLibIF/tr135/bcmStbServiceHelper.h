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
#include <float.h>
#include <malloc.h>
#include <assert.h>
#include "nexus_platform.h"
#include "nexus_platform_common.h"
#include "nexus_audio_decoder.h"
#include "nexus_video_decoder.h"
#include "nexus_playpump.h"
#include "nxclient.h"
#include "nexus_display.h"
#include "nexus_video_types.h"
#include "bkni.h"
#include "nexus_display_types.h"
#include "nexus_platform_features.h"
#include "nexus_video_decoder_types.h"
#include "nexus_video_types.h"



#ifndef __BCMSTBSERVICEHELPER_H__
#define __BCMSTBSERVICEHELPER_H__

#define MAX_OBJECTS 					32
#define MPEG2PART2_PROFILE_LEVEL	"MPEG2ProfileLevel"
#define MPEG4PART2_PROFILE_LEVEL	"MPEG4Part2ProfileLevel"
#define MPEG4PART10_PROFILE_LEVEL	"MPEG4Part10ProfileLevel"
#define SMPTEVC1_PROFILE_LEVEL		"SMPTEVC1ProfileLevel"

/*These definitions are for SpectrumAnalyzer*/
#define MIN_NUMSAMPLES 0
#define MAX_NUMSAMPLES 1024
#define DEFAULT_NUMSAMPLES 256
#define MIN_MEASUREMENTSPERBIN  1
#define MAX_MEASUREMENTSPERBIN  16
#define DEFAULT_MEASUREMENTSPERBIN  4
#define MIN_NUMAVERAGES  1
#define MAX_NUMAVERAGES  16
#define SPECTRUMMEASUREANDSTORE_TRUE  1
#define SPECTRUMMEASUREANDSTORE_FALSE  2
#define MIN_SPECTRUMCOMPARISONTHRESHOLD  0
#define MAX_SPECTRUMCOMPARISONTHRESHOLD  30
#define MAX_SPAN	1000000000
#define MIN_SPAN	6000000
#define DEFAULT_SPAN	MAX_SPAN
#define MIN_FREQ	0
#define MAX_FREQ	/*1070000000*/1000000000
#define DEFAULT_NUMENTRIES 32
#define MAX_AVPLAYERS 1
#define MAX_AVSTREAMS 4
#define MAX_QAM_FRONTENDS 1
#define MAX_MAIN_STREAMS 1


typedef enum {
	disabled = 0,
	enabled,
	error
} b_status;

typedef struct profileLevel {
	NEXUS_VideoProtocolProfile profile;
	NEXUS_VideoProtocolLevel level;
	float max_decode;
	char *alias;
}profileLevel;


typedef struct {
    const char *name; /* string for nexus enum */
    int value; /* nexus enum */
} nameValue;

typedef struct decoder {
	int index;
	char *alias;
	const char *name;
}decoder;

typedef struct spectrumAnalyzer {
	unsigned numSamples;
	int measurementsPerBin;
	int numAverages;
	bool measureAndStore;
	int comparisonThreshold;
	int comparisonInterval;
	unsigned span;
	unsigned startFrequency;
	unsigned stopFrequency;
	unsigned centerFrequency;
	unsigned measurementTableNumberOfEntries;
}spectrumAnalyzer;

typedef struct measurementTable {
	int32_t refeferenceFrequency;
	int32_t amplitudeData;
}measurementTable;

typedef struct comparisonTable {
	int32_t compFrequency;
}comparisonTable;


typedef struct b_AVStream {
	b_status status;
	char alias[64];
	char name[256];
	char *PVRState;
	char frontend[256];
	char inbound[256];
	char outbound[256];
	char audiodecoder[256];
	char videodecoder[256];
	char CA[256];
	char DRM[256];
}b_AVStream_t;

typedef struct b_AVStreams
{
	uint32_t activeAVStreams;
	uint32_t AVStreamNumberOfEntries;
}b_AVStreams_t;

typedef struct b_AVPlayer
{
	bool enable;
	b_status status;
	char alias[64];
	char name[256];
	char audioLanguage[64];
	char subtitlingStatus[10];
	char subtitlingLanguage[64];
	char audioOutputs[1024];
	char videoOutputs[1024];
	char mainStream[256];
	char PIPStreams[256];
}b_AVPlayer_t;

typedef struct b_AVPlayers
{
	uint32_t activeAVPlayers;
	uint32_t AVPlayerNumberOfEntries;
	char preferredAudioLanguage[64];
	char preferredSubtitlingLanguage[64];
	char preferredBehaviour[64];
	bool resetPINCode;
}b_AVPlayers_t;

typedef struct b_videoOutput
{
	bool enabled;
	b_status status;
	char alias[64];
	bool colorbarEnabled;
	char name[256];
	char *compositeVideoStandard;
	char *videoFormat;
	char *aspectRatioBehaviour;
	char *displayFormat;
	bool macrovision;
	bool HDCP;
	char SCART[1024];
}b_videoOutput_t;

typedef struct b_spdif
{
	bool enabled;
	b_status status;
	char alias[64];
	char name[256];
	bool forcePCM;
	bool passthrough;
	unsigned audioDelay;
}b_spdif_t;

typedef struct b_audioDecoder
{
	bool enabled;
	b_status status;
	char alias[64];
	char name[256];
	char audioStandard[256];
}b_audioDecoder_t;

typedef struct b_videoDecoder
{
	bool enabled;
	b_status status;
	char alias[64];
	char name[256];
	char MPEG2Part2[256];
	char MPEG4Part2[256];
	char MPEG4Part10[256];
	char SMPTEVC1[256];
	char aspectRatio[4];
}b_videoDecoder_t;

typedef struct b_FrontendQAM
{
	bool enabled;
	b_status status;
	char alias[64];
	char name[256];
}b_FrontendQAM_t;

typedef struct b_QAMModulation
{
	unsigned frequency;
	bool receiverLock;          /* Do we have QAM lock? */
    bool fecLock;               /* Is the FEC locked? */
    bool opllLock;              /* Is the output PLL locked? */
    bool spectrumInverted;      /* Is the spectrum inverted? */

    unsigned symbolRate;        /* Baud. received symbol rate (in-band) */
    int      symbolRateError;   /* symbol rate error in Baud */

    int berEstimate;            /* deprecated */

    unsigned ifAgcLevel;        /* IF AGC level in units of 1/10 percent */
    unsigned rfAgcLevel;        /* tuner AGC level in units of 1/10 percent */
    unsigned intAgcLevel;       /* Internal AGC level in units of 1/10 percent */
    unsigned snrEstimate;       /* snr estimate in 1/100 dB (in-Band). */

    unsigned fecCorrected;      /* FEC corrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned fecUncorrected;    /* FEC uncorrected block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned fecClean;          /* FEC clean block count, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned bitErrCorrected;   /* deprecated: cumulative bit correctable errors. same as viterbiUncorrectedBits. */
    unsigned reacquireCount;    /* cumulative reacquisition count */

    unsigned viterbiUncorrectedBits; /* uncorrected error bits output from Viterbi, accumulated since tune or NEXUS_Frontent_ResetStatus */
    unsigned viterbiTotalBits;       /* total number of bits output from Viterbi, accumulated since tune or NEXUS_Frontent_ResetStatus */
    uint32_t viterbiErrorRate;  /* Viterbi or pre-Reed-Solomon bit error rate (preRsBer). For units see errorRateUnits.
                                   This is the ratio of uncorrected bits / total bits since the last GetQamStatus.
                                   For a Docsis frontend, units are NEXUS_FrontendErrorRateUnits_eNaturalLog and this value is valid since the last tuner lock. */
    NEXUS_FrontendErrorRateUnits errorRateUnits; /* units for viterbiErrorRate and postRsBer. Default is NEXUS_FrontendErrorRateUnits_eLinear. */

    int      carrierFreqOffset; /* carrier frequency offset in 1/1000 Hz */
    int      carrierPhaseOffset;/* carrier phase offset in 1/1000 Hz */

    unsigned goodRsBlockCount;  /* reset on every read */
    unsigned berRawCount;       /* reset on every read */

    int      dsChannelPower;    /* units of 1/10 dBmV */
    unsigned mainTap;           /* Channel main tap coefficient */
    unsigned postRsBer;         /* post reed-solomon bit error rate. Same behavior as viterbiErrorRate. For units see errorRateUnits. */
    unsigned postRsBerElapsedTime; /* time used in postRsBer calculation. In units of milliseconds. */
    uint16_t interleaveDepth;   /* Used in DOCSIS */
    unsigned lnaAgcLevel;       /* LNA AGC level in units of 1/10 percent */

    int equalizerGain;          /* Channel equalizer gain value in dB */
    int frontendGain;           /* in 1/100th of a dB. Is the accumulated gain of the tuner/ufe/wfe. */
    int digitalAgcGain;         /* in 1/100th of a dB. Is the AGC gain in the demod core. */
    int highResEqualizerGain;   /* in 1/100th of a dB. Is the equalizer gain in the demod core. higher resolution than previous equalizerGain. */
}b_QAMModulation_t;

typedef struct b_mainStream
{
	bool enable;
	b_status status;
	char alias[64];
	char serviceType[32];
	char aVStream[256];
	unsigned gMin;
	unsigned serveralLossMinDistance;
	unsigned serveralLossMinLength;
	unsigned channelChangedFailureTimeout;
	char packetsLostByEventHistogramIntervals[32];
	char delayBetweenLossEventsHistogramIntervals[256];
	char durationSevereLossEventsHistogramIntervals[256];
} b_mainStream_t;

typedef struct b_total
{
	unsigned startTime; /*This will be set as starting time of application */
	unsigned totalSeconds;
	bool reset;
	unsigned resetTime;
}b_total_t;

typedef struct b_MPEG2TSStats
{
	unsigned totalSeconds;
	unsigned TSPacketsReceived;
	unsigned TSSyncByteErrorCount;
	unsigned TSSyncLossCount;
	unsigned packetDiscontinuityCounter;
	unsigned packetDiscontinuityCounterBeforeCA;
}b_MPEG2TSStats_t;

typedef struct b_videoDecoderStats
{
	unsigned totalSeconds;
	unsigned frameRate;
	unsigned decodedFrames;
	unsigned lostFrames;
	unsigned concealedFrames;
	unsigned IDecodedFrames;
	unsigned ILostFrames;
	unsigned IConcealedFrames;
	unsigned PDecodedFrames;
	unsigned PLostFrames;
	unsigned PConcealedFrames;
	unsigned BDecodedFrames;
	unsigned BLostFrames;
	unsigned BConcealedFrames;
	unsigned AVResynchCounter;
} b_videoDecoderStats_t;

typedef struct b_audioDecoderStats
{
	unsigned totalSeconds;
	unsigned decodedFrames;
	unsigned decodingErrors;
} b_audioDecoderStats_t;

typedef struct b_IpInbound
{
	b_status status;
	char alias[64];
	char name[256];
	char streamingControlProtocol[16];
	char streamingTransportProtocol[16];
	char streamingTransportControlProtocol[16];
	char multiplexType[16];
	char downloadTransportProtocol[16];
	char sourceAddr[45];
	unsigned sourcePort;
	char destinationAddr[45];
	unsigned destinationPort;
	char URI[256];
} b_IpInbound_t;


int  getProfileLevel(profileLevel **pProfileLevel, uint8_t *pNumEntries, char *name) ;
int setAudioVideoDecoderAlias(decoder **pDecoder, char *name, char *alias);
int getAudioVideoDecoder(decoder **pDecoder, uint8_t *pNumEntries, char *name);
void printlist(const nameValue *table);
const char *strList(const nameValue *table);

spectrumAnalyzer* getSpectrumAnalyzer(void);
int setCenterFreq (spectrumAnalyzer *pSpectrum, int freq);
int setNumOfSamples (spectrumAnalyzer *pSpectrum, int numSamples);




const char *enumToString(const nameValue *table, int value);
unsigned stringToEnum(const nameValue *table, const char *name);
int getAudioDecoder( b_audioDecoder_t **pAudioDecoder, int index);
int getVideoDecoder( b_videoDecoder_t **pVideoDecoder, int index);
int getAVStream (b_AVStream_t **pAVStream, int index);
int setAVStreamParams(b_AVStream_t *in, int index);
int getAVPlayer (b_AVPlayer_t **pAVPlayer, int index);
int setAVPlayerParams(b_AVPlayer_t *in, int index);
void init_135(void);
int setVideoDecoderParams(b_videoDecoder_t *in, int index);
int setAudioDecoderParams(b_audioDecoder_t *in, int index);
int getAVPlayers (b_AVPlayers_t **pAVPlayers);
int setAVPlayersParams(b_AVPlayers_t *in);
int getAVStreams (b_AVStreams_t **pAVStreams);
int setAVStreamsParams(b_AVStreams_t *in);
int getQamModulation (b_QAMModulation_t **pQamModulation);
int setQamModulationsParams(b_QAMModulation_t * in);
int getVideoDecoderStats  (b_videoDecoderStats_t **pVideoDecoderStats);
int setVideoDecoderStats(b_videoDecoderStats_t *in);
int getAudioDecoderStats (b_audioDecoderStats_t **pAudioDecoderStats);
int setAudioDecoderStats(b_audioDecoderStats_t *in);
int getMPEG2TSStats (b_MPEG2TSStats_t **pMPEG2TSStats);
int setMPEG2TSStats(b_MPEG2TSStats_t *in);
int getMainStream(b_mainStream_t **pMainStream);
int setMainStream(b_mainStream_t *in);
int getTotal(b_total_t **pTotal);
int setTotal(b_total_t *in);
int getSpdif(b_spdif_t **pSpdif);
int setSpdif(b_spdif_t *in);
int getIpInbound(b_IpInbound_t **pInbound);
int setIpInbound(b_IpInbound_t *in);



extern const nameValue videoProfileStrs[];
extern const nameValue hdmiVideoFormat [];
extern const nameValue videoLevelStrs[];
extern const nameValue audioCodecStandard[];
extern const nameValue videoCodecStandard[];
extern const nameValue compositeVideoStandard[];
extern const nameValue videoOutputDisplayFormat[];
extern const nameValue videoOutputFormats[];
extern const nameValue audioOutputFormat[];
extern const nameValue audioOutputMode[];

#endif /* __BCMSTBSERVICEHELPER_H__ */
