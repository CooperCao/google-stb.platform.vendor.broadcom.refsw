/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bape.h"
#include "bape_priv.h"
#include "bdsp_raaga.h"

BDBG_MODULE(bape_dsp_utils);

#define DISABLE_MPEG_DRA_PASSTHRU 0

/* MS10/11 and FP2008 (7358) require legacy DDP decoder. Everything else uses UDC */
#if BAPE_DSP_LEGACY_DDP_ALGO
#define BAPE_DSP_AC3_ALGO       BDSP_Algorithm_eAc3Decode
#define BAPE_DSP_DDP_ALGO       BDSP_Algorithm_eAc3PlusDecode
#define BAPE_DSP_AC3_PT_ALGO    BDSP_Algorithm_eAc3Passthrough
#define BAPE_DSP_DDP_PT_ALGO    BDSP_Algorithm_eAc3PlusPassthrough
#else /* MS12 or Legacy Standalone decode */
#define BAPE_DSP_AC3_ALGO       BDSP_Algorithm_eUdcDecode
#define BAPE_DSP_DDP_ALGO       BDSP_Algorithm_eUdcDecode
#define BAPE_DSP_AC3_PT_ALGO    BDSP_Algorithm_eUdcPassthrough
#define BAPE_DSP_DDP_PT_ALGO    BDSP_Algorithm_eUdcPassthrough
#endif

static const BAPE_CodecAttributes g_codecAttributes[] =
{
/*   AVC Codec                              DSP Decode                            DSP Passthrough                       DSP Encode                        Name            Max multichannel format       SPDIF? SRC?  PassthruReq'd , Simul?, mono?,     4x?,   16x? */
#if DISABLE_MPEG_DRA_PASSTHRU
    {BAVC_AudioCompressionStd_eMpegL1,      BDSP_Algorithm_eMpegAudioDecode,      BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "MPEG",         BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eMpegL2,      BDSP_Algorithm_eMpegAudioDecode,      BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "MPEG",         BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eMpegL3,      BDSP_Algorithm_eMpegAudioDecode,      BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMpegAudioEncode,  "MPEG",         BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
#else
    {BAVC_AudioCompressionStd_eMpegL1,      BDSP_Algorithm_eMpegAudioDecode,      BDSP_Algorithm_eMpegAudioPassthrough, BDSP_Algorithm_eMax,              "MPEG",         BAPE_MultichannelFormat_e2_0, true,  true,  true,          true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eMpegL2,      BDSP_Algorithm_eMpegAudioDecode,      BDSP_Algorithm_eMpegAudioPassthrough, BDSP_Algorithm_eMax,              "MPEG",         BAPE_MultichannelFormat_e2_0, true,  true,  true,          true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eMpegL3,      BDSP_Algorithm_eMpegAudioDecode,      BDSP_Algorithm_eMpegAudioPassthrough, BDSP_Algorithm_eMpegAudioEncode,  "MPEG",         BAPE_MultichannelFormat_e2_0, true,  true,  true,          true,   false,   false,   false},
#endif
#if BDSP_MS12_SUPPORT
    {BAVC_AudioCompressionStd_eAc3,         BAPE_DSP_AC3_ALGO,                    BAPE_DSP_AC3_PT_ALGO,                 BDSP_Algorithm_eDDPEncode,        "AC3",          BAPE_MultichannelFormat_e5_1, true,  false, false,         true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eAc3Plus,     BAPE_DSP_DDP_ALGO,                    BAPE_DSP_DDP_PT_ALGO,                 BDSP_Algorithm_eDDPEncode,        "AC3+",         BAPE_MultichannelFormat_e7_1, true,  false, false,         true,   false,   true,    false},
    {BAVC_AudioCompressionStd_eAacAdts,     BDSP_Algorithm_eDolbyAacheAdtsDecode, BDSP_Algorithm_eAacAdtsPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC ADTS",     BAPE_MultichannelFormat_e5_1, true,  false, true,          false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eAacLoas,     BDSP_Algorithm_eDolbyAacheLoasDecode, BDSP_Algorithm_eAacLoasPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC LOAS",     BAPE_MultichannelFormat_e5_1, true,  false, true,          false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eAacPlusAdts, BDSP_Algorithm_eDolbyAacheAdtsDecode, BDSP_Algorithm_eAacAdtsPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC+ ADTS",    BAPE_MultichannelFormat_e5_1, true,  false, true,          false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eAacPlusLoas, BDSP_Algorithm_eDolbyAacheLoasDecode, BDSP_Algorithm_eAacLoasPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC+ LOAS",    BAPE_MultichannelFormat_e5_1, true,  false, true,          false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eAc4,         BDSP_Algorithm_eAC4Decode,            BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "AC4",          BAPE_MultichannelFormat_e7_1, true,  false, false,         true,   false,   false,   false},
#elif BDSP_MS10_SUPPORT
    {BAVC_AudioCompressionStd_eAc3,         BAPE_DSP_AC3_ALGO,                    BAPE_DSP_AC3_PT_ALGO,                 BDSP_Algorithm_eAc3Encode,        "AC3",          BAPE_MultichannelFormat_e5_1, true,  false, false,         true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eAc3Plus,     BAPE_DSP_DDP_ALGO,                    BAPE_DSP_DDP_PT_ALGO,                 BDSP_Algorithm_eMax,              "AC3+",         BAPE_MultichannelFormat_e5_1, true,  false, false,         true,   false,   true,    false},
    {BAVC_AudioCompressionStd_eAacAdts,     BDSP_Algorithm_eDolbyPulseAdtsDecode, BDSP_Algorithm_eAacAdtsPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC ADTS",     BAPE_MultichannelFormat_e5_1, true,  false, true,          false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eAacLoas,     BDSP_Algorithm_eDolbyPulseLoasDecode, BDSP_Algorithm_eAacLoasPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC LOAS",     BAPE_MultichannelFormat_e5_1, true,  false, true,          false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eAacPlusAdts, BDSP_Algorithm_eDolbyPulseAdtsDecode, BDSP_Algorithm_eAacAdtsPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC+ ADTS",    BAPE_MultichannelFormat_e5_1, true,  false, true,          false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eAacPlusLoas, BDSP_Algorithm_eDolbyPulseLoasDecode, BDSP_Algorithm_eAacLoasPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC+ LOAS",    BAPE_MultichannelFormat_e5_1, true,  false, true,          false,  false,   false,   false},
#else
    {BAVC_AudioCompressionStd_eAc3,         BAPE_DSP_AC3_ALGO,                    BAPE_DSP_AC3_PT_ALGO,                 BDSP_Algorithm_eAc3Encode,        "AC3",          BAPE_MultichannelFormat_e5_1, true,  false, false,         true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eAc3Plus,     BAPE_DSP_DDP_ALGO,                    BAPE_DSP_DDP_PT_ALGO,                 BDSP_Algorithm_eMax,              "AC3+",         BAPE_MultichannelFormat_e5_1, true,  false, false,         true,   false,   true,    false},
    {BAVC_AudioCompressionStd_eAacAdts,     BDSP_Algorithm_eAacAdtsDecode,        BDSP_Algorithm_eAacAdtsPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC ADTS",     BAPE_MultichannelFormat_e5_1, true,  true,  true,          true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eAacLoas,     BDSP_Algorithm_eAacLoasDecode,        BDSP_Algorithm_eAacLoasPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC LOAS",     BAPE_MultichannelFormat_e5_1, true,  true,  true,          true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eAacPlusAdts, BDSP_Algorithm_eAacAdtsDecode,        BDSP_Algorithm_eAacAdtsPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC+ ADTS",    BAPE_MultichannelFormat_e5_1, true,  true,  true,          true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eAacPlusLoas, BDSP_Algorithm_eAacLoasDecode,        BDSP_Algorithm_eAacLoasPassthrough,   BDSP_Algorithm_eAacEncode,        "AAC+ LOAS",    BAPE_MultichannelFormat_e5_1, true,  true,  true,          true,   false,   false,   false},
#endif
    {BAVC_AudioCompressionStd_eDts,         BDSP_Algorithm_eDtsHdDecode,          BDSP_Algorithm_eDtsHdPassthrough,     BDSP_Algorithm_eDtsCoreEncode,    "DTS",          BAPE_MultichannelFormat_e5_1, true,  false, true,          true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eDtshd,       BDSP_Algorithm_eDtsHdDecode,          BDSP_Algorithm_eDtsHdPassthrough,     BDSP_Algorithm_eMax,              "DTS-HD",       BAPE_MultichannelFormat_e7_1, true,  true,  true,          true,   false,   true,    true},
    {BAVC_AudioCompressionStd_eDtsLegacy,   BDSP_Algorithm_eDts14BitDecode,       BDSP_Algorithm_eDts14BitPassthrough,  BDSP_Algorithm_eMax,              "DTS-Legacy",   BAPE_MultichannelFormat_e5_1, true,  false, true,          true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eDtsExpress,  BDSP_Algorithm_eDtsLbrDecode,         BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "DTS-Express",  BAPE_MultichannelFormat_e5_1, true,  false, true,          true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eWmaStd,      BDSP_Algorithm_eWmaStdDecode,         BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "WMA Std",      BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eWmaStdTs,    BDSP_Algorithm_eWmaStdDecode,         BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "WMA Std TS",   BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eWmaPro,      BDSP_Algorithm_eWmaProDecode,         BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "WMA Pro",      BAPE_MultichannelFormat_e5_1, true,  false, false,         true,   false,   false,   false},
    {BAVC_AudioCompressionStd_eMlp,         BDSP_Algorithm_eMlpDecode,            BDSP_Algorithm_eMlpPassthrough,       BDSP_Algorithm_eMax,              "MLP",          BAPE_MultichannelFormat_e7_1, true,  true,  true,          true,   false,   false,   true},
    {BAVC_AudioCompressionStd_ePcm,         BDSP_Algorithm_ePcmDecode,            BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "PCM",          BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_ePcmWav,      BDSP_Algorithm_ePcmWavDecode,         BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "PCM WAV",      BAPE_MultichannelFormat_e7_1, false, true,  false,         false,  true,    false,   false},
    {BAVC_AudioCompressionStd_eLpcmDvd,     BDSP_Algorithm_eLpcmDvdDecode,        BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "LPCM DVD",     BAPE_MultichannelFormat_e5_1, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eLpcmBd,      BDSP_Algorithm_eLpcmBdDecode,         BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "LPCM BD",      BAPE_MultichannelFormat_e7_1, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eLpcm1394,    BDSP_Algorithm_eLpcm1394Decode,       BDSP_Algorithm_eMax,                  BDSP_Algorithm_eLpcmEncode,       "LPCM 1394",    BAPE_MultichannelFormat_e7_1, false, false, false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eAmr,         BDSP_Algorithm_eAmrNbDecode,          BDSP_Algorithm_eMax,                  BDSP_Algorithm_eAmrNbEncode,      "AMR-NB",       BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  true,    false,   false},
    {BAVC_AudioCompressionStd_eAmrWb,       BDSP_Algorithm_eAmrWbDecode,          BDSP_Algorithm_eMax,                  BDSP_Algorithm_eAmrWbEncode,      "AMR-WB",       BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  true,    false,   false},
#if DISABLE_MPEG_DRA_PASSTHRU
    {BAVC_AudioCompressionStd_eDra,         BDSP_Algorithm_eDraDecode,            BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "DRA",          BAPE_MultichannelFormat_e5_1, false, true,  false,         false,  false,   false,   false},
#else
    {BAVC_AudioCompressionStd_eDra,         BDSP_Algorithm_eDraDecode,            BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "DRA",          BAPE_MultichannelFormat_e5_1, true,  true,  true,          true,   false,   false,   false},
#endif
    {BAVC_AudioCompressionStd_eCook,        BDSP_Algorithm_eCookDecode,           BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "Cook",         BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eAdpcm,       BDSP_Algorithm_eAdpcmDecode,          BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "ADPCM",        BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eVorbis,      BDSP_Algorithm_eVorbisDecode,         BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "Vorbis",       BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eG711,        BDSP_Algorithm_eG711Decode,           BDSP_Algorithm_eMax,                  BDSP_Algorithm_eG711Encode,       "G.711",        BAPE_MultichannelFormat_e2_0, false, false, false,         false,  true,    false,   false},
    {BAVC_AudioCompressionStd_eG726,        BDSP_Algorithm_eG726Decode,           BDSP_Algorithm_eMax,                  BDSP_Algorithm_eG726Encode,       "G.726",        BAPE_MultichannelFormat_e2_0, false, false, false,         false,  true,    false,   false},
    {BAVC_AudioCompressionStd_eG729,        BDSP_Algorithm_eG729Decode,           BDSP_Algorithm_eMax,                  BDSP_Algorithm_eG729Encode,       "G.729",        BAPE_MultichannelFormat_e2_0, false, false, false,         false,  true,    false,   false},
    {BAVC_AudioCompressionStd_eG723_1,      BDSP_Algorithm_eG723_1Decode,         BDSP_Algorithm_eMax,                  BDSP_Algorithm_eG723_1Encode,     "G.723.1",      BAPE_MultichannelFormat_e2_0, false, false, false,         false,  true,    false,   false},
    {BAVC_AudioCompressionStd_eFlac,        BDSP_Algorithm_eFlacDecode,           BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "FLAC",         BAPE_MultichannelFormat_e7_1, false, true,  false,         false,  false,   false,   false},
    {BAVC_AudioCompressionStd_eApe,         BDSP_Algorithm_eMacDecode,            BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "APE Monkey",   BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  false,   false,   false},
#if 0
    {BAVC_AudioCompressionStd_eDtsHd,       BDSP_AudioType_eDtshd,                BDSP_AudioEncode_eMax,         "DTS-HD",       BAPE_MultichannelFormat_e7_1, true,  true,  true,          true,   false,   true,    true},
    {BAVC_AudioCompressionStd_eDtsCd,       BDSP_AudioType_eDtsBroadcast,         BDSP_AudioEncode_eMax,         "DTS-CD",       BAPE_MultichannelFormat_e5_1, true,  false, true,          true,   false,   false,   false},
#endif
    {BAVC_AudioCompressionStd_eIlbc,        BDSP_Algorithm_eiLBCDecode,           BDSP_Algorithm_eMax,                  BDSP_Algorithm_eiLBCEncode,       "iLbc",         BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  true,    false,   false},
    {BAVC_AudioCompressionStd_eIsac,        BDSP_Algorithm_eiSACDecode,           BDSP_Algorithm_eMax,                  BDSP_Algorithm_eiSACEncode,       "iSac",         BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  true,    false,   false},
    {BAVC_AudioCompressionStd_eOpus,        BDSP_Algorithm_eOpusDecode,           BDSP_Algorithm_eMax,                  BDSP_Algorithm_eOpusEncode,       "Opus",         BAPE_MultichannelFormat_e2_0, false, true,  false,         false,  true,    false,   false},
    {BAVC_AudioCompressionStd_eAls,         BDSP_Algorithm_eALSDecode,            BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "MPEG-4 Als",   BAPE_MultichannelFormat_e5_1, false, true,  false,         false,  false,   false,   false},
    /* This entry must be last */
    {BAVC_AudioCompressionStd_eMax,         BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,                  BDSP_Algorithm_eMax,              "Unknown",      BAPE_MultichannelFormat_e2_0, false, false, false,         false,  false,   false,   false}
};

const BAPE_CodecAttributes *BAPE_P_GetCodecAttributes_isrsafe(
    BAVC_AudioCompressionStd codec
    )
{
    unsigned i, tableSize;

    tableSize = sizeof(g_codecAttributes)/sizeof(BAPE_CodecAttributes);

    for ( i = 0; i < tableSize; i++ )
    {
        if ( codec == g_codecAttributes[i].codec )
        {
            return &g_codecAttributes[i];
        }
    }

    return &g_codecAttributes[tableSize-1];
}

BAPE_DolbyMSVersion BAPE_P_GetDolbyMSVersion_isrsafe(void)
{
    #if BDSP_MS12_SUPPORT
    return BAPE_DolbyMSVersion_eMS12;
    #elif BDSP_MS10_SUPPORT
    #if BDSP_DDRE_SUPPORT
    return BAPE_DolbyMSVersion_eMS11;
    #else
    return BAPE_DolbyMSVersion_eMS10;
    #endif
    #else
    return BAPE_DolbyMSVersion_eNone;
    #endif
}

BAPE_DolbyMs12Config BAPE_P_GetDolbyMS12Config_isrsafe(void)
{
    if ( BAPE_P_GetDolbyMSVersion_isrsafe() == BAPE_DolbyMSVersion_eMS12 )
    {
        BDSP_CodecCapabilities dspCaps;

        BDSP_Raaga_GetCodecCapabilities(&dspCaps);
        /*BDBG_ERR(("ddEncode %d, ddpEncode51 %d, ddpEncode71 %d, pcm71 %d, dapv2 %d",
                  dspCodecCaps.dolbyMs.ddEncode, dspCodecCaps.dolbyMs.ddpEncode51, dspCodecCaps.dolbyMs.ddpEncode71, dspCodecCaps.dolbyMs.pcm71, dspCodecCaps.dolbyMs.dapv2));*/
        if ( dspCaps.dolbyMs.ddpEncode71 && dspCaps.dolbyMs.pcm71 )
        {
            /* Full - 7.1 PCM and DDP encode supported */
            return BAPE_DolbyMs12Config_eA;
        }
        else if ( !dspCaps.dolbyMs.ddpEncode51 )
        {
            /* Limited - Only Ac3 encode supported */
            return BAPE_DolbyMs12Config_eC;
        }
        else
        {
            /* Default - 5.1 DDP encode supported */
            return BAPE_DolbyMs12Config_eB;
        }
    }

    return BAPE_DolbyMs12Config_eNone;
}

BDSP_Algorithm BAPE_P_GetCodecMixer_isrsafe(void)
{
    if ( BAPE_P_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        return BDSP_Algorithm_eMixerDapv2;
    }

    return BDSP_Algorithm_eMixer;
}

BAPE_ChannelMode BAPE_DSP_P_GetChannelMode(BAVC_AudioCompressionStd codec, BAPE_ChannelMode outputMode, bool multichannelOutput, BAPE_MultichannelFormat maxFormat)
{
    BAPE_MultichannelFormat multichannelFormat;
    BAPE_ChannelMode maxMode;

    multichannelFormat = BAPE_P_GetCodecMultichannelFormat(codec);

    switch ( multichannelFormat )
    {
    default:
    case BAPE_MultichannelFormat_e2_0:
        maxMode = BAPE_ChannelMode_e2_0;
        break;
    case BAPE_MultichannelFormat_e5_1:
        maxMode = BAPE_ChannelMode_e3_2;
        break;
    case BAPE_MultichannelFormat_e7_1:
        maxMode = BAPE_ChannelMode_e3_4;
        break;
    }

    /* Do we have any multichannel outputs? */
    if ( multichannelOutput )
    {
        /* Don't allow > the number of ringbuffers we allocate */
        switch ( maxFormat )
        {
        case BAPE_MultichannelFormat_e2_0:
            maxMode = BAPE_ChannelMode_e2_0;
            break;
        case BAPE_MultichannelFormat_e5_1:
            if ( maxMode > BAPE_ChannelMode_e3_2 )
            {
                maxMode = BAPE_ChannelMode_e3_2;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        maxMode = BAPE_ChannelMode_e2_0;
    }

    return (outputMode > maxMode)?maxMode:outputMode;
}

uint32_t BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode outputMode)
{
    switch ( outputMode )
    {
    case BAPE_ChannelMode_e1_1:
        return 0;
    case BAPE_ChannelMode_e1_0:
        return 1;
    default:
    case BAPE_ChannelMode_e2_0:
        return 2;
    case BAPE_ChannelMode_e3_0:
        return 3;
    case BAPE_ChannelMode_e2_1:
        return 4;
    case BAPE_ChannelMode_e3_1:
        return 5;
    case BAPE_ChannelMode_e2_2:
        return 6;
    case BAPE_ChannelMode_e3_2:
        return 7;
    case BAPE_ChannelMode_e3_4:
        return 21;
    }
}

void BAPE_DSP_P_GetChannelMatrix(
    BAPE_ChannelMode outputMode,
    bool lfe,
    uint32_t *pChannelMatrix
    )
{
    unsigned i=0;
    static const uint32_t defaultChannelMatrix[BAPE_ChannelMode_eMax][BDSP_AF_P_MAX_CHANNELS] =
    {/*
     L,R,Ls,       ,Rs        ,C,        ,Lfe,      ,RLs       ,RRs  CHECKPOINT   */
    {4,4,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},/*1_0 */
    {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},/*1_1 */
    {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},/*2_0 */
    {0,1,0xFFFFFFFF,0xFFFFFFFF,4,         0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},/*3_0 */
    {0,1,2,         3,         0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},/*2_1 - Assumes mixing done in FW between LsRs */
    {0,1,0xFFFFFFFF,0xFFFFFFFF,4,         0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},/*3_1 */
    {0,1,2,         3,         0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},/*2_2 */
    {0,1,2,         3,         4,         0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},/*3_2 */
    {0,1,2,         3,         4,         0xFFFFFFFF,6,         7         },/*3_4 */
    };

    if ( outputMode >= BAPE_ChannelMode_eMax )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(outputMode < BAPE_ChannelMode_eMax);
        return;
    }

    for( i = 0; i < BDSP_AF_P_MAX_CHANNELS; i++ )
    {
        pChannelMatrix[i] = defaultChannelMatrix[outputMode][i];
    }

    if( outputMode == BAPE_ChannelMode_e3_2 || outputMode == BAPE_ChannelMode_e3_4 )
    {
        if( lfe )
        {
            pChannelMatrix[5] = 5;
        }
        else
        {
            pChannelMatrix[5] = 0xFFFFFFFF;
        }
    }
}

void BAPE_DSP_P_GetMonoChannelMatrix(uint32_t *pChannelMatrix)
{
    unsigned i;

    static const uint32_t monoChannelMatrix[BDSP_AF_P_MAX_CHANNELS] =
        {4,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

    for(i = 0; i<BDSP_AF_P_MAX_CHANNELS; i++)
    {
        pChannelMatrix[i] = monoChannelMatrix[i];
    }
}

/*----------------------------------------------------------------------------*/
uint32_t BAPE_P_FloatToQ131(int32_t floatVar, unsigned int uiRange)
{
    int32_t     temp;

    if (floatVar >= (int32_t)uiRange)
        return(uint32_t) 0x7FFFFFFF;

    if (floatVar <= (int32_t)-uiRange)
        return(uint32_t) 0x80000000;

    /* Conversion formula for float to Q1.31 is
     * q = float * 2^31,
     * Since we take float values scaled by uiRange from application, we need
     * to scale it down
     * by uiRange. Hence above formula would become
     * q = float * ( 2^31/uiRange )
     * However this won't be a precise computation, as reminder of
     * (2^31/uiRange) gets dropped
     * in this calculation. To compesate for this reminder formula needs to be
     * modified as below
     * q = float * ( 2^31/uiRange ) + (float * (2^31 %uiRange))/uiRange
     */

    temp = floatVar * (0x80000000/uiRange) +
           (floatVar * (0x80000000 % uiRange)) / uiRange;

    return temp;
}

/*----------------------------------------------------------------------------*/
uint32_t BAPE_P_FloatToQ230(int16_t floatVar)
{
    int32_t     temp;

    BDBG_ASSERT(floatVar >= 0);
    /* TODO: conversion for negative values */
    /* Conversion formula for float to Q2.30 is
     * q = float * 2^30,
     * Since we take float values scaled by 100 from application, we need to
     * scale it down
     * by 100. Hence above formula would become
     * q = float * ( 2^30/100 )
     * However this won't be a precise computation, as reminder of (2^30/100)
     * gets dropped
     * in this calculation. To compesate for this reminder formula needs to be
     * modified as below
     * q = float * ( 2^30/100 ) + ( float * ( 2^30 %100))/100
     */
    if (floatVar >= 100)
        return(uint32_t) 0x40000000;

    temp = floatVar * (0x40000000/100) + ( floatVar * (0x40000000 % 100))/100;

    return temp;
}


/*----------------------------------------------------------------------------*/
uint32_t BAPE_P_FloatToQ521(uint32_t floatVar, unsigned int uiRange)
{
    int32_t     temp;

    /* TODO: conversion for negative values */
    /* Conversion formula for float to Q5.21 is
     * q = float * 2^26,
     * Since the entire range of values in PI is mapped to the range of 0 to
     * 2^26 in FW,
     * a given value in PI gets converted to corresponding Q5.21 value as below,
     * q = float * ( 2^26/uiRange )
     * However this won't be a precise computation, as remainder of
     * (2^26/uiRange) gets dropped
     * in this calculation. To compensate for this remainder formula needs to be
     * modified as below
     * q = float * ( 2^26/uiRange ) + ( float * ( 2^26 % uiRange))/uiRange
     * But if the value corresponding to the multiplication of reminder turns
     * out to be
     * a fractional value then the value gets rounded off to zero but if the
     * value is >= uiRange/2
     * it should be rounded off to 1(as per SRS). Hence forth the formula
     * becomes,
     * q = float * ( 2^26/uiRange ) + (unsigned int)((float * ( 2^26 % uiRange)
     * + (uiRange/2))/uiRange)
     */

    if (floatVar >= uiRange)
        return(uint32_t)0x03FFFFFF;

    temp = floatVar * (0x03FFFFFF/uiRange) +
           (unsigned int)(( floatVar * (0x03FFFFFF % uiRange) +
                            (uiRange/2))/uiRange);

    return temp;
}


/*----------------------------------------------------------------------------*/
/*
    This function converts the floating point value to 8.24 fixed format
*/
int32_t BAPE_P_FloatToQ824(int32_t floatVar, unsigned int uiRange)
{
    int32_t     temp;

    /*Range should be multiple of 127 i.e. 127,254,381,508.....*/

    if (floatVar >= (int32_t)uiRange)
        return(uint32_t) 0x7F000000;

    if (floatVar <= -(int32_t)(uiRange + (uiRange/127)))
        return(uint32_t)0x80000000;

    temp = floatVar *   (0x7F000000/uiRange) +
           (unsigned int)(( floatVar * (0x7F000000 % uiRange) +
                            (uiRange/2))/uiRange);

    return temp;
}


/*----------------------------------------------------------------------------*/
int32_t BAPE_P_FloatToQ923(uint32_t floatVar, unsigned int uiRange)
{
    int32_t  temp;

    /* TODO: conversion for negative values */
    /* Conversion formula for float to Q9.23 is
     * q = float * 2^23,
     * Since the entire range of values in PI is mapped to the range of 0 to
     * 2^23 in FW,
     * a given value in PI gets converted to corresponding Q9.23 value as below,
     * q = float * ( 2^23/uiRange )
     * However this won't be a precise computation, as remainder of
     * (2^23/uiRange) gets dropped
     * in this calculation. To compensate for this remainder, formula needs to
     * be modified as below
     * q = float * ( 2^23/uiRange ) + ( float * ( 2^23 % uiRange))/uiRange
     * But if the value corresponding to the multiplication of reminder turns
     * out to be
     * a fractional value then the value gets rounded off to zero but if the
     * value is >= uiRange/2
     * it should be rounded off to 1(as per SRS). Hence forth the formula
     * becomes,
     * q = float * ( 2^23/uiRange ) + (unsigned int)((float * ( 2^23 % uiRange)
     * + (uiRange/2))/uiRange)
     */

    if (floatVar >= uiRange)
        return(uint32_t)0x007fffff;

    temp = floatVar *(0x007fffff/uiRange) +
           (unsigned int)(((floatVar * (0x7fffff % uiRange)) +
                           (uiRange/2)) / uiRange);

    return temp;
}

/* Function to convert input floating point value into Q1.15 format
 * Intended floating point value to be converted, should be
 * multiplied by 100 and then passed to this function
 */
int32_t BAPE_P_FloatToQ1022(int32_t floatVar, unsigned int uiRange)
{
    int32_t     temp;

    /* TODO: conversion for negative values */
    /* Conversion formula for float to Q10.22 is
     * q = float * 2^22,
     * Since the entire range of values in PI is mapped to the range of 0 to 2^22 in FW,
     * a given value in PI gets converted to corresponding Q10.22 value as below,
     * q = float * ( 2^22/uiRange )
     * However this won't be a precise computation, as remainder of (2^22/uiRange) gets dropped
     * in this calculation. To compensate for this remainder formula needs to be modified as below
     * q = float * ( 2^22/uiRange ) + ( float * ( 2^22 % uiRange))/uiRange
     * But if the value corresponding to the multiplication of reminder turns out to be
     * a fractional value then the value gets rounded off to zero but if the value is >= uiRange/2
     * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
     * q = float * ( 2^22/uiRange ) + (unsigned int)((float * ( 2^22 % uiRange) + (uiRange/2))/uiRange)
     */


    if (floatVar >= 0)
    {
        temp = floatVar * (0x003FFFFF/uiRange) +
               (unsigned int)(( floatVar * (0x003FFFFF % uiRange) +
                                (uiRange/2))/uiRange);
    }
    else
    {
        floatVar = (-1)*floatVar;

        temp = floatVar * (0x003FFFFF/uiRange) +
               (unsigned int)(( floatVar * (0x003FFFFF % uiRange) +
                                (uiRange/2))/uiRange);
        temp = (-1)*temp;
    }
    return temp;
}

/*
    This function converts the floating point value to 5.18 fixed format
*/
uint32_t BAPE_P_FloatToQ518(uint32_t floatVar, unsigned int uiRange)
{
    int32_t     temp;

    /* TODO: conversion for negative values */
    /* Conversion formula for float to Q5.18 is
     * q = float * 2^23,
     * Since the entire range of values in PI is mapped to the range of 0 to 2^23 in FW,
     * a given value in PI gets converted to corresponding Q5.18 value as below,
     * q = float * ( 2^23/uiRange )
     * However this won't be a precise computation, as remainder of (2^23/uiRange) gets dropped
     * in this calculation. To compensate for this remainder formula needs to be modified as below
     * q = float * ( 2^23/uiRange ) + ( float * ( 2^23 % uiRange))/uiRange
     * But if the value corresponding to the multiplication of reminder turns out to be
     * a fractional value then the value gets rounded off to zero but if the value is >= uiRange/2
     * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
     * q = float * ( 2^23/uiRange ) + (unsigned int)((float * ( 2^23 % uiRange) + (uiRange/2))/uiRange)
     */

    if (floatVar >= uiRange)
        return(uint32_t)0x007FFFFF;

    temp = floatVar * (0x007FFFFF/uiRange) +
           (unsigned int)(( floatVar * (0x007FFFFF % uiRange) +
                            (uiRange/2))/uiRange);

    return temp;
}

/*
    This function converts the floating point value to 8.15 fixed format
*/
uint32_t BAPE_P_FloatToQ815(uint32_t floatVar, unsigned int uiRange)
{
    int32_t     temp;

    /* TODO: conversion for negative values */
    /* Conversion formula for float to Q8.15 is
     * q = float * 2^23,
     * Since the entire range of values in PI is mapped to the range of 0 to 2^23 in FW,
     * a given value in PI gets converted to corresponding Q8.15 value as below,
     * q = float * ( 2^23/uiRange )
     * However this won't be a precise computation, as remainder of (2^23/uiRange) gets dropped
     * in this calculation. To compensate for this remainder formula needs to be modified as below
     * q = float * ( 2^23/uiRange ) + ( float * ( 2^23 % uiRange))/uiRange
     * But if the value corresponding to the multiplication of reminder turns out to be
     * a fractional value then the value gets rounded off to zero but if the value is >= uiRange/2
     * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
     * q = float * ( 2^23/uiRange ) + (unsigned int)((float * ( 2^23 % uiRange) + (uiRange/2))/uiRange)
     */

    if (floatVar >= uiRange)
        return(uint32_t)0x007FFFFF;

    temp = floatVar * (0x007FFFFF/uiRange) +
           (unsigned int)(( floatVar * (0x007FFFFF % uiRange) +
                            (uiRange/2))/uiRange);

    return temp;
}


/*
    This function converts the floating point value to 5.27 fixed format
*/
uint32_t BAPE_P_FloatToQ527(uint32_t floatVar, unsigned int uiRange)
{
    int32_t     temp;

    /* TODO: conversion for negative values */
    /* Conversion formula for float to Q5.27 is
     * q = float * 2^27,
     * Since the entire range of values in PI is mapped to the range of 0 to 2^27 in FW,
     * a given value in PI gets converted to corresponding Q5.27 value as below,
     * q = float * ( 2^27/uiRange )
     * However this won't be a precise computation, as remainder of (2^27/uiRange) gets dropped
     * in this calculation. To compensate for this remainder formula needs to be modified as below
     * q = float * ( 2^27/uiRange ) + ( float * ( 2^27 % uiRange))/uiRange
     * But if the value corresponding to the multiplication of reminder turns out to be
     * a fractional value then the value gets rounded off to zero but if the value is >= uiRange/2
     * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
     * q = float * ( 2^27/uiRange ) + (unsigned int)((float * ( 2^27 % uiRange) + (uiRange/2))/uiRange)
     * 0x08000000 = 2^27 + 1, the value 2^27 is being rounded off as that will be the precise value in
     * decoder implementation. Either ways of using 2^27 or 0x08000000 doesn't result in much difference.
     */

    temp = floatVar * (0x08000000/uiRange) +
           (unsigned int)(( floatVar * (0x08000000 % uiRange) +
                            (uiRange/2))/uiRange);

    return temp;
}

/* Function to convert input floating point value into Q3.29 format
 * Intended floating point value to be converted, should be
 * multiplied by uiRange value and then passed to this function
 */
uint32_t BAPE_P_FloatToQ329(uint32_t floatVar, unsigned int uiRange)
{
    int32_t     temp;

    /* Conversion formula for float to Q3.29 is
     * q = float * 2^29,
     * Since we take float values scaled by uiRange from application, we need to scale it down
     * by uiRange. Hence above formula would become
     * q = float * ( 2^29/uiRange )
     * However this won't be a precise computation, as reminder of (2^29/uiRange) gets dropped
     * in this calculation. To compesate for this reminder formula needs to be modified as below
     * q = float * ( 2^29/uiRange ) + (float * (2^29 %uiRange))/uiRange
     */

    temp = floatVar * (0x1FFFFFFF/uiRange) +
           ( floatVar * (0x1FFFFFFF % uiRange))/uiRange;

    return temp;
}

/* Function to convert input floating point value into Q4.28 format
 * Intended floating point value to be converted, should be
 * multiplied by uiRange value and then passed to this function
 */
uint32_t BAPE_P_FloatToQ428(uint32_t floatVar, unsigned int uiRange)
{
    int32_t     temp;

    /* Conversion formula for float to Q4.28 is
     * q = float * 2^28,
     * Since we take float values scaled by uiRange from application, we need to scale it down
     * by uiRange. Hence above formula would become
     * q = float * ( 2^28/uiRange )
     * However this won't be a precise computation, as reminder of (2^28/uiRange) gets dropped
     * in this calculation. To compesate for this reminder formula needs to be modified as below
     * q = float * ( 2^28/uiRange ) + (float * (2^28 %uiRange))/uiRange
     */

    temp = floatVar * (0x10000000/uiRange) +
           ( floatVar * (0x10000000 % uiRange))/uiRange;

    if (temp >= 0x40000000) return(uint32_t) 0x40000000;

    return temp;
}

/***************************************************************************
Summary:
Add FMM Buffer Output to a stage
***************************************************************************/
BERR_Code BAPE_DSP_P_AddFmmBuffer(
    BAPE_PathConnection *pConnection
                                 )
{
    BERR_Code errCode;
    unsigned i, numChannels, numChannelPairs, outputIndex;
    bool pcm;
    BDSP_FmmBufferDescriptor bufferDesc;
    BDSP_DataType dataType;

    /* Sanity Checks */
    BDBG_ASSERT(NULL != pConnection);
    BDBG_ASSERT(NULL != pConnection->pSource);

    BKNI_Memset(&bufferDesc, 0, sizeof(bufferDesc));

    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&pConnection->pSource->format);
    pcm = BAPE_FMT_P_IsLinearPcm_isrsafe(&pConnection->pSource->format);
    dataType = BAPE_FMT_P_GetDspDataType_isrsafe(&pConnection->pSource->format);

    if ( pcm )
    {
        numChannels = 2*numChannelPairs;
    }
    else
    {
        BDBG_ASSERT(numChannelPairs == 1);
        numChannels = 1;
    }

    if ( pConnection->pSink->type==BAPE_PathNodeType_eMixer )
    {
        BAPE_MixerHandle mixer = pConnection->pSink->pHandle;
        BDBG_ASSERT(NULL != mixer);
        bufferDesc.delay= mixer->settings.outputDelay;
    }
    BDBG_ASSERT(NULL != pConnection->sfifoGroup);
    bufferDesc.numBuffers = numChannels;
    bufferDesc.buffers[0].base = BAPE_P_SFIFO_TO_BASEADDR_REG(BAPE_SfifoGroup_P_GetHwIndex(pConnection->sfifoGroup, 0));
    bufferDesc.buffers[0].end = BAPE_P_SFIFO_TO_ENDADDR_REG(BAPE_SfifoGroup_P_GetHwIndex(pConnection->sfifoGroup, 0));
    bufferDesc.buffers[0].read = BAPE_P_SFIFO_TO_RDADDR_REG(BAPE_SfifoGroup_P_GetHwIndex(pConnection->sfifoGroup, 0));
    bufferDesc.buffers[0].write = BAPE_P_SFIFO_TO_WRADDR_REG(BAPE_SfifoGroup_P_GetHwIndex(pConnection->sfifoGroup, 0));
    bufferDesc.buffers[0].wrpoint = BAPE_P_SFIFO_TO_WRPOINT_REG(BAPE_SfifoGroup_P_GetHwIndex(pConnection->sfifoGroup, 0));
    if ( numChannels > 1 )
    {
        /* PCM - Noninterleaved */
        for ( i = 1; i < numChannels; i++ )
        {
            bufferDesc.buffers[i].base = bufferDesc.buffers[i-1].base + BAPE_P_RINGBUFFER_STRIDE;
            bufferDesc.buffers[i].end = bufferDesc.buffers[i-1].end + BAPE_P_RINGBUFFER_STRIDE;
            bufferDesc.buffers[i].read = bufferDesc.buffers[i-1].read + BAPE_P_RINGBUFFER_STRIDE;
            bufferDesc.buffers[i].write = bufferDesc.buffers[i-1].write + BAPE_P_RINGBUFFER_STRIDE;
            bufferDesc.buffers[i].wrpoint = bufferDesc.buffers[i-1].wrpoint + BAPE_P_RINGBUFFER_STRIDE;
        }
        bufferDesc.numRateControllers = BAPE_SfifoGroup_P_GetAdaptRateWrcntAddress(pConnection->sfifoGroup, i) == 0xffffffff ? 0 : numChannelPairs;
        for ( i = 0; i < numChannelPairs; i++ )
        {
            /* CITTODO - update rate controllers variable name */
            bufferDesc.rateControllers[i].wrcnt = BAPE_SfifoGroup_P_GetAdaptRateWrcntAddress(pConnection->sfifoGroup, i);
        }
    }

    errCode = BDSP_Stage_AddFmmOutput(pConnection->pSource->hStage, dataType, &bufferDesc, &outputIndex);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    pConnection->dspOutputIndex = outputIndex;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Init an FMM buffer descriptor from a DFIFO group
***************************************************************************/
BERR_Code BAPE_DSP_P_InitFmmInputDescriptor(
    BAPE_DfifoGroupHandle hDfifoGroup,
    BDSP_FmmBufferDescriptor *pDesc
    )
{
    unsigned i, numChannels, numChannelPairs;
    BAPE_DfifoGroupSettings dfifoSettings;

    BDBG_ASSERT(NULL != hDfifoGroup);
    BDBG_ASSERT(NULL != pDesc);

    BKNI_Memset(pDesc, 0, sizeof(BDSP_FmmBufferDescriptor));

    BAPE_DfifoGroup_P_GetSettings(hDfifoGroup, &dfifoSettings);

    for ( numChannelPairs = 4; numChannelPairs > 0; numChannelPairs-- )
    {
        if ( BAPE_DfifoGroup_P_GetHwIndex(hDfifoGroup, numChannelPairs-1) != 0xffffffff )
            break;
    }
    if ( numChannelPairs == 0 )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    numChannels = dfifoSettings.interleaveData ? numChannelPairs : 2*numChannelPairs;

    pDesc->numBuffers = numChannels;
    pDesc->buffers[0].base = BAPE_P_DFIFO_TO_BASEADDR_REG(BAPE_DfifoGroup_P_GetHwIndex(hDfifoGroup, 0));
    pDesc->buffers[0].end = BAPE_P_DFIFO_TO_ENDADDR_REG(BAPE_DfifoGroup_P_GetHwIndex(hDfifoGroup, 0));
    pDesc->buffers[0].read = BAPE_P_DFIFO_TO_RDADDR_REG(BAPE_DfifoGroup_P_GetHwIndex(hDfifoGroup, 0));
    pDesc->buffers[0].write = BAPE_P_DFIFO_TO_WRADDR_REG(BAPE_DfifoGroup_P_GetHwIndex(hDfifoGroup, 0));
    pDesc->buffers[0].wrpoint = BAPE_P_DFIFO_TO_WRPOINT_REG(BAPE_DfifoGroup_P_GetHwIndex(hDfifoGroup, 0));
    if ( numChannels > 1 )
    {
        /* PCM - Noninterleaved */
        for ( i = 1; i < numChannels; i++ )
        {
            pDesc->buffers[i].base = pDesc->buffers[i-1].base + BAPE_P_RINGBUFFER_STRIDE;
            pDesc->buffers[i].end = pDesc->buffers[i-1].end + BAPE_P_RINGBUFFER_STRIDE;
            pDesc->buffers[i].read = pDesc->buffers[i-1].read + BAPE_P_RINGBUFFER_STRIDE;
            pDesc->buffers[i].write = pDesc->buffers[i-1].write + BAPE_P_RINGBUFFER_STRIDE;
            pDesc->buffers[i].wrpoint = pDesc->buffers[i-1].wrpoint + BAPE_P_RINGBUFFER_STRIDE;
        }
    }

    return BERR_SUCCESS;

}

/***************************************************************************
Summary:
Get Connector Data Type
***************************************************************************/
BDSP_DataType BAPE_DSP_P_GetDataTypeFromConnector(
                                                            BAPE_Connector connector
                                                            )
{
    BDBG_OBJECT_ASSERT(connector, BAPE_PathConnector);
    return BAPE_FMT_P_GetDspDataType_isrsafe(&connector->format);
}

BERR_Code BAPE_DSP_P_AllocatePathToOutput(
                                         BAPE_PathNode *pNode,
                                         BAPE_PathConnection *pConnection
                                         )
{
    BAPE_PathConnector *pSource;
    unsigned outputIndex;
    BAPE_PathNode *pSink;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    pSource = pConnection->pSource;
    pSink = pConnection->pSink;

    BDBG_ASSERT(NULL != pSource->hStage);

    if ( pSink->type == BAPE_PathNodeType_eMixer && pSink->subtype == BAPE_MixerType_eDsp )
    {
        errCode = BDSP_Stage_AddInterTaskBufferOutput(pSource->hStage, BAPE_DSP_P_GetDataTypeFromConnector(pSource), pConnection->hInterTaskBuffer, &outputIndex);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        pConnection->dspOutputIndex = outputIndex;
    }
    else if ( pSink->type == BAPE_PathNodeType_eMixer || pSink->type == BAPE_PathNodeType_eEqualizer )
    {
        /* Dropping into the FMM - need to add FMM buffers to the stage */
        errCode = BAPE_DSP_P_AddFmmBuffer(pConnection);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else if ( pSink->type == BAPE_PathNodeType_eEchoCanceller )
    {
        BAPE_EchoCancellerHandle hEchoCanceller = pSink->pHandle;

        if ( pSource == hEchoCanceller->remoteInput )
        {
            errCode = BDSP_Stage_AddInterTaskBufferOutput(pSource->hStage, BAPE_DSP_P_GetDataTypeFromConnector(pSource), pConnection->hInterTaskBuffer, &outputIndex);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
            pConnection->dspOutputIndex = outputIndex;
        }
    }
    /* Other nodes don't require anything done here */

    return BERR_SUCCESS;
}

BERR_Code BAPE_DSP_P_ConfigPathToOutput(
                                       BAPE_PathNode *pNode,
                                       BAPE_PathConnection *pConnection
                                       )
{
    BAPE_PathConnector *pSource;
    BAPE_PathNode *pSink;
    BERR_Code errCode;
    unsigned i, numChannelPairs;
    bool pcm;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    pSource = pConnection->pSource;
    pSink = pConnection->pSink;

    if ( pSink->type == BAPE_PathNodeType_eMixer && pSink->subtype == BAPE_MixerType_eDsp )
    {
        /* DSP mixers don't require anything */
    }
    else if ( pSink->type == BAPE_PathNodeType_eMixer || pSink->type == BAPE_PathNodeType_eEqualizer )
    {
        BAPE_SfifoGroupSettings sfifoSettings;
        /* Dropping into the FMM - need to config SFIFO's.  DSP does not use master/slave, it will instead reuse ringbuffers in multiple
           master SFIFOs */

        pcm = BAPE_FMT_P_IsLinearPcm_isrsafe(&pSource->format);
        numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&pSource->format);

        BAPE_SfifoGroup_P_GetSettings(pConnection->sfifoGroup, &sfifoSettings);
        sfifoSettings.highPriority = false; /* TODO: How to handle this on the fly? */
        sfifoSettings.reverseEndian = false;
        sfifoSettings.signedData = true;
        sfifoSettings.wrpointEnabled = true;
        if ( pcm )
        {
            sfifoSettings.dataWidth = 32;
            sfifoSettings.sampleRepeatEnabled = true;
            sfifoSettings.interleaveData = false;
            /* Setup buffers from pool */
            for ( i = 0; i < numChannelPairs; i++ )
            {
                unsigned bufferId = 2*i;
                BAPE_BufferNode *pBuffer;

                if ( pSource->pParent->type == BAPE_PathNodeType_eDecoder )
                {
                    BAPE_DecoderHandle decoder;
                    BAPE_ConnectorFormat connectorFormat;
                    /* Decoders have the odd property of occasionally sending PCM data on the compressed path when a source
                       codec can't handle compressed output (e.g. PCM).  If that happens, the DSP will really only populate a
                       set of data buffers.  So, if you have both actual PCM outputs and fake compressed outputs receiving data
                       at the same time, you need to setup the fake compressed outputs to actually use the buffer populated on
                       the PCM path.  Same can also happen for multichannel data. */
                    decoder = (BAPE_DecoderHandle)pSource->pParent->pHandle;
                    BDBG_OBJECT_ASSERT(decoder, BAPE_Decoder);

                    /* Determine connector format */
                    for ( connectorFormat = BAPE_ConnectorFormat_eStereo; connectorFormat < BAPE_ConnectorFormat_eMax; connectorFormat++ )
                    {
                        if ( pSource == &pSource->pParent->connectors[connectorFormat] )
                        {
                            break;
                        }
                    }
                    BDBG_ASSERT(connectorFormat < BAPE_ConnectorFormat_eMax);   /* If this fails something has gone seriously wrong */

                    if ( (connectorFormat == BAPE_ConnectorFormat_eMultichannel && decoder->stereoOnMultichannel &&
                          decoder->outputStatus.connectorStatus[BAPE_ConnectorFormat_eStereo].directConnections > 0) ||
                         ((connectorFormat == BAPE_ConnectorFormat_eCompressed ||
                           connectorFormat == BAPE_ConnectorFormat_eCompressed4x ||
                           connectorFormat == BAPE_ConnectorFormat_eCompressed16x) && decoder->stereoOnCompressed &&
                          decoder->outputStatus.connectorStatus[BAPE_ConnectorFormat_eStereo].directConnections > 0) )
                    {
                        BDBG_MSG(("%s path of decoder %u reusing data buffers from stereo path",
                                 (connectorFormat == BAPE_ConnectorFormat_eMultichannel)?"Multichannel":"Stereo", decoder->index));
                        /* Multichannel or Compressed sending same data as stereo path.  Reuse that. */
                        pBuffer = pSource->pParent->connectors[BAPE_ConnectorFormat_eStereo].pBuffers[i];
                    }
                    else if ( ((connectorFormat == BAPE_ConnectorFormat_eCompressed ||
                                connectorFormat == BAPE_ConnectorFormat_eCompressed4x ||
                                connectorFormat == BAPE_ConnectorFormat_eCompressed16x) &&
                               decoder->stereoOnMultichannel && decoder->stereoOnCompressed &&
                               decoder->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].directConnections > 0) )
                    {
                        BDBG_MSG(("Compressed path of decoder %u reusing data buffers from multichannel path", decoder->index));
                        /* Compressed sending same data as stereo data on multichannel path (no direct stereo consumers).  Reuse that. */
                        pBuffer = pSource->pParent->connectors[BAPE_ConnectorFormat_eMultichannel].pBuffers[i];
                    }
                    else
                    {
                        pBuffer = pSource->pBuffers[i]; /* Use this connector's buffers */
                    }
                }
                else
                {
                    pBuffer = pSource->pBuffers[i]; /* Not a decoder, use the connector's buffers */
                }

                if ( !pBuffer )
                {
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }

                sfifoSettings.bufferInfo[bufferId].base = pBuffer->offset;
                sfifoSettings.bufferInfo[bufferId].length = pBuffer->bufferSize/2;
                sfifoSettings.bufferInfo[bufferId].wrpoint = sfifoSettings.bufferInfo[bufferId].base+(pBuffer->bufferSize/2)-1;
                bufferId++;
                sfifoSettings.bufferInfo[bufferId].base = pBuffer->offset+(pBuffer->bufferSize/2);
                sfifoSettings.bufferInfo[bufferId].length = pBuffer->bufferSize/2;
                sfifoSettings.bufferInfo[bufferId].wrpoint = sfifoSettings.bufferInfo[bufferId].base+(pBuffer->bufferSize/2)-1;
            }
        }
        else
        {
            sfifoSettings.dataWidth = 16;
            sfifoSettings.sampleRepeatEnabled = false;
            sfifoSettings.interleaveData = true;
            /* Setup buffers from pool */
            for ( i = 0; i < numChannelPairs; i++ )
            {
                unsigned bufferId = 2*i;
                BAPE_BufferNode *pBuffer = pSource->pBuffers[i];
                BDBG_ASSERT(NULL != pBuffer);
                sfifoSettings.bufferInfo[bufferId].base = pBuffer->offset;
                sfifoSettings.bufferInfo[bufferId].length = pBuffer->bufferSize;
                sfifoSettings.bufferInfo[bufferId].wrpoint = pBuffer->offset+pBuffer->bufferSize-1;
                bufferId++;
                sfifoSettings.bufferInfo[bufferId].base = 0;
                sfifoSettings.bufferInfo[bufferId].length = 0;
                sfifoSettings.bufferInfo[bufferId].wrpoint = 0;
            }
        }

        errCode = BAPE_SfifoGroup_P_SetSettings(pConnection->sfifoGroup, &sfifoSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    /* Other nodes don't require anything done here */

    return BERR_SUCCESS;
}

BERR_Code BAPE_DSP_P_StartPathToOutput(
                                      BAPE_PathNode *pNode,
                                      BAPE_PathConnection *pConnection
                                      )
{
    BAPE_PathNode *pSink;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    pSink = pConnection->pSink;

    if ( pSink->type == BAPE_PathNodeType_eMixer && pSink->subtype == BAPE_MixerType_eDsp )
    {
        /* FW Mixer is a special case. Nothing to do */
    }
    else if ( pSink->type == BAPE_PathNodeType_eMixer || pSink->type == BAPE_PathNodeType_eEqualizer )
    {
        /* TODO: Sure would be nice to use PLAY_RUN instead of WRPOINT */
        errCode = BAPE_SfifoGroup_P_Start(pConnection->sfifoGroup, false);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        if ( pSink->type == BAPE_PathNodeType_eMixer )
        {
            BAPE_StandardMixer_P_SfifoStarted(pSink->pHandle, pConnection);
        }
    }

    return BERR_SUCCESS;
}

void BAPE_DSP_P_StopPathToOutput(
                                BAPE_PathNode *pNode,
                                BAPE_PathConnection *pConnection
                                )
{
    BAPE_PathNode *pSink;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    pSink = pConnection->pSink;

    if ( pSink->type == BAPE_PathNodeType_eMixer && pSink->subtype == BAPE_MixerType_eDsp )
    {
        /* FW Mixer is a special case. Nothing to do */
    }
    else if ( pSink->type == BAPE_PathNodeType_eMixer || pSink->type == BAPE_PathNodeType_eEqualizer )
    {
        BAPE_SfifoGroup_P_Stop(pConnection->sfifoGroup);
    }
}

#if BAPE_DSP_SUPPORT
bool BAPE_DSP_P_AlgorithmSupported(BAPE_Handle hApe, BDSP_Algorithm algorithm)
{
    BDSP_AlgorithmInfo algoInfo;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);
    errCode = BDSP_GetAlgorithmInfo(hApe->dspHandle, algorithm, &algoInfo);
    if ( errCode || !algoInfo.supported )
    {
        return false;
    }

    return true;
}
#else
bool BAPE_DSP_P_AlgorithmSupported(BAPE_Handle hApe, BDSP_Algorithm algorithm)
{
    BSTD_UNUSED(hApe);
    BSTD_UNUSED(algorithm);

    return false;
}
#endif

bool BAPE_DSP_P_AlgorithmSupportedByApe(BAPE_Handle hApe, BDSP_Algorithm algorithm)
{
    unsigned i, tableSize;

    tableSize = sizeof(g_codecAttributes)/sizeof(BAPE_CodecAttributes);

    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);

    if ( BAPE_DSP_P_AlgorithmSupported(hApe, algorithm) )
    {
        for ( i=0; i < tableSize; i++ )
        {
            if ( g_codecAttributes[i].decodeAlgorithm == algorithm ||
                 g_codecAttributes[i].encodeAlgorithm == algorithm ||
                 g_codecAttributes[i].passthroughAlgorithm == algorithm )
            {
                return true;
            }
        }
    }

    return false;
}

BERR_Code BAPE_DSP_P_DeriveTaskStartSettings(
    BAPE_PathNode *pNode,
    BDSP_TaskStartSettings *pStartSettings
    )
{
    unsigned numFound;
    BAPE_PathNode *pMuxOutput;
    BAPE_MixerHandle hDspMixer = NULL;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    errCode = BAPE_PathNode_P_GetDecodersDownstreamDspMixer(pNode, &hDspMixer);
    if ( errCode != BERR_SUCCESS )
    {
        return BERR_TRACE(errCode);
    }

    if ( hDspMixer != NULL )
    {
        /* Setup Master Mode */
        pStartSettings->masterTask = hDspMixer->hTask;
        pStartSettings->schedulingMode = BDSP_TaskSchedulingMode_eSlave;

        /* Check if MuxOutput is after DSP Mixer */
        BAPE_PathNode_P_FindConsumersByType(&hDspMixer->pathNode, BAPE_PathNodeType_eMuxOutput, 1, &numFound, &pMuxOutput);
        if ( numFound > 0 )
        {
            /* The settings will affect the mixer task, not this node */
            return BERR_SUCCESS;
        }
    }
    BAPE_PathNode_P_FindConsumersByType(pNode, BAPE_PathNodeType_eMuxOutput, 1, &numFound, &pMuxOutput);
    if ( numFound > 0 )
    {
        BAPE_MuxOutputHandle hMuxOutput;

        hMuxOutput = pMuxOutput->pHandle;
        /* If configuring for NRT, setup STC increment linkage */
        if ( hMuxOutput->nonRealTimeIncrement.StcIncLo != 0 &&
             hMuxOutput->nonRealTimeIncrement.StcIncHi != 0 &&
             hMuxOutput->nonRealTimeIncrement.IncTrigger != 0 &&
             hMuxOutput->state == BAPE_MuxOutputState_Started)
        {
            pStartSettings->stcIncrementConfig.enableStcTrigger = true;
            pStartSettings->stcIncrementConfig.stcIncHiAddr = hMuxOutput->nonRealTimeIncrement.StcIncHi + BCHP_PHYSICAL_OFFSET;
            pStartSettings->stcIncrementConfig.stcIncLowAddr = hMuxOutput->nonRealTimeIncrement.StcIncLo + BCHP_PHYSICAL_OFFSET;
            pStartSettings->stcIncrementConfig.stcIncTrigAddr = hMuxOutput->nonRealTimeIncrement.IncTrigger + BCHP_PHYSICAL_OFFSET;
            #ifdef BCHP_XPT_PCROFFSET_STC0_INC_TRIG
            pStartSettings->stcIncrementConfig.triggerBit = BCHP_XPT_PCROFFSET_STC0_INC_TRIG_SOFT_INC_TRIG_SHIFT;
            #endif
            /* TODO: This shouldn't be mutually exclusive with Master/Slave */
            pStartSettings->realtimeMode = BDSP_TaskRealtimeMode_eNonRealTime;
        }
    }

    return BERR_SUCCESS;
}
