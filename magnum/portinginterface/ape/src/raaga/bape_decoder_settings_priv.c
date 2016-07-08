/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/


#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bdsp_raaga.h"

BDBG_MODULE(bape_decoder_settings);
BDBG_FILE_MODULE(bape_loudness);

static bool BAPE_Decoder_P_HasMultichannelOutput(
    BAPE_DecoderHandle handle
    )
{
    if ( handle->type == BAPE_DecoderType_eDecodeToMemory )
    {
        BAPE_MultichannelFormat multiFormat = BAPE_P_GetCodecMultichannelFormat(handle->startSettings.codec);
        if ( multiFormat == BAPE_MultichannelFormat_e7_1 || multiFormat == BAPE_MultichannelFormat_e5_1 )
        {
            return handle->decodeToMem.settings.numPcmChannels > 2 ? true : false;
        }
        else
        {
            return false;
        }
    }
    else if ( handle->stereoOnMultichannel )
    {
        return false;
    }
    else
    {
        return handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].numValidOutputs > 0 ? true : false;
    }
}

static bool BAPE_Decoder_P_HasStereoOutput(
    BAPE_DecoderHandle handle
    )
{
    if ( handle->type == BAPE_DecoderType_eDecodeToMemory )
    {
        if ( handle->decodeToMem.settings.numPcmChannels == 2 )
        {
            return true;
        }
        else if ( handle->decodeToMem.settings.numPcmChannels > 2 && BAPE_MultichannelFormat_e2_0 == BAPE_P_GetCodecMultichannelFormat(handle->startSettings.codec) )
        {
            return true;
        }
        return false;
    }
    if ( handle->type == BAPE_DecoderType_eDecodeToMemory )
    {
        return handle->decodeToMem.settings.numPcmChannels == 2 ? true : false;
    }
    if ( handle->stereoOnMultichannel || handle->stereoOnCompressed )
    {
        return true;
    }
    else
    {
        return handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eStereo].numValidOutputs > 0 ? true : false;
    }
}

static bool BAPE_Decoder_P_HasMonoOutput(
    BAPE_DecoderHandle handle
    )
{
    if ( handle->type == BAPE_DecoderType_eDecodeToMemory )
    {
        if ( handle->decodeToMem.settings.numPcmChannels == 1 && BAPE_P_CodecSupportsMono(handle->startSettings.codec) )
        {
            return true;
        }
        return false;
    }
    return handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMono].numValidOutputs > 0 ? true : false;
}

static BAPE_ChannelMode BAPE_Decoder_P_GetChannelMode(BAPE_DecoderHandle handle)
{
    return BAPE_DSP_P_GetChannelMode(handle->startSettings.codec, handle->settings.outputMode, BAPE_Decoder_P_HasMultichannelOutput(handle), handle->settings.multichannelFormat);
}

static void BAPE_Decoder_P_GetAFDecoderType(BAPE_DecoderHandle handle, BDSP_AF_P_DecoderType *pType)
{
    if ( handle->fwMixerMaster )
    {
        *pType = BDSP_AF_P_DecoderType_ePrimary;
    }
    else
    {
        switch ( handle->startSettings.mixingMode )
        {
        default:
        case BAPE_DecoderMixingMode_eDescription:
            *pType = BDSP_AF_P_DecoderType_eSecondary;
            break;
        case BAPE_DecoderMixingMode_eSoundEffects:
            *pType = BDSP_AF_P_DecoderType_eSoundEffects;
            break;
        case BAPE_DecoderMixingMode_eApplicationAudio:
            *pType = BDSP_AF_P_DecoderType_eApplicationAudio;
            break;
        }
    }
}

static void BAPE_Decoder_P_GetDefaultAc3Settings(BAPE_DecoderHandle handle)
{
#if BAPE_DSP_LEGACY_DDP_ALGO
    BDSP_Algorithm bdspAlgo = BDSP_Algorithm_eAc3Decode;
#else
    BDSP_Algorithm bdspAlgo = BDSP_Algorithm_eUdcDecode;
#endif

    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, bdspAlgo) )
    {
        return;
    }
    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(bdspAlgo, &handle->userConfig.ddp, sizeof(handle->userConfig.ddp)));

    handle->ac3Settings.codec = BAVC_AudioCompressionStd_eAc3;
    handle->ac3Settings.codecSettings.ac3.substreamId = handle->userConfig.ddp.ui32SubstreamIDToDecode;
    handle->ac3Settings.codecSettings.ac3.dialogNormalization = handle->userConfig.ddp.i32StreamDialNormEnable?true:false;
    handle->ac3Settings.codecSettings.ac3.dialogNormalizationValue = handle->userConfig.ddp.i32UserDialNormVal;
    handle->ac3Settings.codecSettings.ac3.drcMode = BAPE_Ac3DrcMode_eLine; /* Default multichannel outputs to Line Mode */
    handle->ac3Settings.codecSettings.ac3.drcModeDownmix = BAPE_Ac3DrcMode_eRf; /* Default stereo outputs to RF Mode */

    handle->ac3Settings.codecSettings.ac3.scale = 100;
    handle->ac3Settings.codecSettings.ac3.scaleDownmix = 100;
    handle->ac3Settings.codecSettings.ac3.drcScaleHi = 100;
    handle->ac3Settings.codecSettings.ac3.drcScaleLow = 100;
    handle->ac3Settings.codecSettings.ac3.drcScaleHiDownmix = 100;
    handle->ac3Settings.codecSettings.ac3.drcScaleLowDownmix = 100;
#if BAPE_DSP_LEGACY_DDP_ALGO
    BDBG_ASSERT(0x7fffffff == handle->userConfig.ddp.sUserOutputCfg[0].i32PcmScale);
    BDBG_ASSERT(0x7fffffff == handle->userConfig.ddp.sUserOutputCfg[1].i32PcmScale);
    BDBG_ASSERT(0x7fffffff == handle->userConfig.ddp.sUserOutputCfg[0].i32DynScaleHigh);
    BDBG_ASSERT(0x7fffffff == handle->userConfig.ddp.sUserOutputCfg[1].i32DynScaleHigh);
    BDBG_ASSERT(0x7fffffff == handle->userConfig.ddp.sUserOutputCfg[0].i32DynScaleLow);
    BDBG_ASSERT(0x7fffffff == handle->userConfig.ddp.sUserOutputCfg[1].i32DynScaleLow);
#else
    BDBG_ASSERT(0x64 == handle->userConfig.ddp.sUserOutputCfg[0].i32PcmScale);
    BDBG_ASSERT(0x64 == handle->userConfig.ddp.sUserOutputCfg[1].i32PcmScale);
    BDBG_ASSERT(0x64 == handle->userConfig.ddp.sUserOutputCfg[0].i32DynScaleHigh);
    BDBG_ASSERT(0x64 == handle->userConfig.ddp.sUserOutputCfg[1].i32DynScaleHigh);
    BDBG_ASSERT(0x64 == handle->userConfig.ddp.sUserOutputCfg[0].i32DynScaleLow);
    BDBG_ASSERT(0x64 == handle->userConfig.ddp.sUserOutputCfg[1].i32DynScaleLow);
    handle->ac3Settings.codecSettings.ac3.enableAtmosProcessing = (handle->userConfig.ddp.ui32EnableAtmosMetadata == 1) ? true : false;
#endif
    BKNI_Memcpy(&handle->ac3PlusSettings, &handle->ac3Settings, sizeof(BAPE_DecoderCodecSettings));
    handle->ac3PlusSettings.codec = BAVC_AudioCompressionStd_eAc3Plus;
}

static void BAPE_Decoder_P_GetDefaultAc4Settings(BAPE_DecoderHandle handle)
{
    BDSP_Algorithm bdspAlgo = BDSP_Algorithm_eAC4Decode;

    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, bdspAlgo) )
    {
        return;
    }
    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(bdspAlgo, &handle->userConfig.ac4, sizeof(handle->userConfig.ac4)));

    handle->ac4Settings.codec = BAVC_AudioCompressionStd_eAc4;
    handle->ac4Settings.codecSettings.ac4.drcMode = BAPE_DolbyDrcMode_eLine; /* Default multichannel outputs to Line Mode */
    handle->ac4Settings.codecSettings.ac4.drcModeDownmix = BAPE_DolbyDrcMode_eRf; /* Default stereo outputs to RF Mode */
    handle->ac4Settings.codecSettings.ac4.stereoMode = BAPE_DolbyStereoMode_eLoRo; /* Default stereo outputs to RF Mode */
    handle->ac4Settings.codecSettings.ac4.programSelection = handle->userConfig.ac4.ui32MainAssocDec;
    handle->ac4Settings.codecSettings.ac4.programBalance = handle->userConfig.ac4.sUserOutputCfg[0].i32MainAssocMixPref;
    handle->ac4Settings.codecSettings.ac4.presentationId = handle->userConfig.ac4.ui32PresentationNumber;
    handle->ac4Settings.codecSettings.ac4.dialogEnhancerAmount = handle->userConfig.ac4.sUserOutputCfg[0].i32DialogEnhGainInput;
}

#if BDSP_MS12_SUPPORT
static void BAPE_Decoder_P_GetDefaultMs12AacSettings(BAPE_DecoderHandle handle)
{
    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eDolbyAacheAdtsDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eDolbyAacheAdtsDecode, &handle->userConfig.aac, sizeof(handle->userConfig.aac)));
    handle->aacSettings.codec = BAVC_AudioCompressionStd_eAac;
    handle->aacPlusSettings.codec = BAVC_AudioCompressionStd_eAacPlus;
    handle->aacSettings.codecSettings.aac.drcReferenceLevel = handle->userConfig.aac.ui32RefDialnormLevel;
    handle->aacSettings.codecSettings.aac.drcDefaultLevel = handle->userConfig.aac.ui32DefDialnormLevel;
    handle->aacSettings.codecSettings.aac.drcScaleHi = 100;
    handle->aacSettings.codecSettings.aac.drcScaleLow = 100;
    handle->aacSettings.codecSettings.aac.downmixMode = BAPE_AacStereoMode_eLtRt; /* unused */
    handle->aacSettings.codecSettings.aac.enableGainFactor = false; /* unused */
    handle->aacSettings.codecSettings.aac.gainFactor = 0; /* unused */
    handle->aacSettings.codecSettings.aac.drcMode = BAPE_DolbyPulseDrcMode_eRf;
    BKNI_Memcpy(&handle->aacPlusSettings.codecSettings, &handle->aacSettings.codecSettings, sizeof(handle->aacSettings.codecSettings.aac));
}

#elif BDSP_MS10_SUPPORT

static void BAPE_Decoder_P_GetDefaultMs10AacSettings(BAPE_DecoderHandle handle)
{
    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eDolbyPulseAdtsDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eDolbyPulseAdtsDecode, &handle->userConfig.aac, sizeof(handle->userConfig.aac)));
    handle->aacSettings.codec = BAVC_AudioCompressionStd_eAac;
    handle->aacPlusSettings.codec = BAVC_AudioCompressionStd_eAacPlus;
    handle->aacSettings.codecSettings.aac.drcReferenceLevel = handle->userConfig.aac.ui32RefDialnormLevel;
    handle->aacSettings.codecSettings.aac.drcDefaultLevel = handle->userConfig.aac.ui32DefDialnormLevel;
    handle->aacSettings.codecSettings.aac.drcScaleHi = handle->userConfig.aac.sOutPortCfg[0].ui32DrcCut;
    handle->aacSettings.codecSettings.aac.drcScaleLow = handle->userConfig.aac.sOutPortCfg[0].ui32DrcBoost;
    handle->aacSettings.codecSettings.aac.downmixMode = handle->userConfig.aac.sOutPortCfg[0].ui32LoRoDownmix?BAPE_AacStereoMode_eLoRo:BAPE_AacStereoMode_eLtRt;
    handle->aacSettings.codecSettings.aac.enableGainFactor = handle->userConfig.aac.sOutPortCfg[0].ui32ApplyGain?true:false;
    handle->aacSettings.codecSettings.aac.gainFactor = handle->userConfig.aac.sOutPortCfg[0].i32GainFactor;
    handle->aacSettings.codecSettings.aac.drcMode = BAPE_DolbyPulseDrcMode_eRf;
    BKNI_Memcpy(&handle->aacPlusSettings.codecSettings, &handle->aacSettings.codecSettings, sizeof(handle->aacSettings.codecSettings.aac));
}

#else

static void BAPE_Decoder_P_GetDefaultLegacyAacSettings(BAPE_DecoderHandle handle)
{
    unsigned i,j;

    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eAacAdtsDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eAacAdtsDecode, &handle->userConfig.aac, sizeof(handle->userConfig.aac)));
    handle->aacSettings.codec = BAVC_AudioCompressionStd_eAac;
    handle->aacPlusSettings.codec = BAVC_AudioCompressionStd_eAacPlus;
    handle->aacSettings.codecSettings.aac.enableSbrDecoding = (handle->userConfig.aac.ui32SbrUserFlag==1) ? true : false;
    handle->aacSettings.codecSettings.aac.drcScaleHi = 100;
    handle->aacSettings.codecSettings.aac.drcScaleLow = 100;
    BDBG_ASSERT(handle->userConfig.aac.ui32DrcGainControlCompress == 0x40000000);
    BDBG_ASSERT(handle->userConfig.aac.ui32DrcGainControlBoost == 0x40000000);
    handle->aacSettings.codecSettings.aac.drcReferenceLevel = handle->userConfig.aac.ui32DrcTargetLevel;
    handle->aacSettings.codecSettings.aac.downmixMode = (handle->userConfig.aac.i32DownmixType==0)?BAPE_AacStereoMode_eMatrix:BAPE_AacStereoMode_eArib;
    handle->aacSettings.codecSettings.aac.enableDownmixCoefficients = handle->userConfig.aac.sUserOutputCfg[0].i32ExtDnmixEnabled?true:false;
    for ( i = 0; i < 6; i++ )
    {
        for ( j = 0; j < 6; j++ )
        {
            handle->aacSettings.codecSettings.aac.downmixCoefficients[i][j] = handle->userConfig.aac.sUserOutputCfg[0].i32ExtDnmixTab[i][j];
        }
    }
    handle->aacSettings.codecSettings.aac.downmixCoefScaleIndex = 0;
    handle->aacSettings.codecSettings.aac.drcMode = BAPE_DolbyPulseDrcMode_eRf;
    BKNI_Memcpy(&handle->aacPlusSettings.codecSettings, &handle->aacSettings.codecSettings, sizeof(handle->aacSettings.codecSettings.aac));
}
#endif

static void BAPE_Decoder_P_GetDefaultAacSettings(BAPE_DecoderHandle handle)
{
#if BDSP_MS12_SUPPORT
    BAPE_Decoder_P_GetDefaultMs12AacSettings(handle);
#elif BDSP_MS10_SUPPORT
    BAPE_Decoder_P_GetDefaultMs10AacSettings(handle);
#else
    BAPE_Decoder_P_GetDefaultLegacyAacSettings(handle);
#endif
}

static void BAPE_Decoder_P_GetDefaultWmaProSettings(
    BAPE_DecoderHandle handle
    )
{
    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eWmaProDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eWmaProDecode, &handle->userConfig.wmaPro, sizeof(handle->userConfig.wmaPro)));

    handle->wmaProSettings.codec = BAVC_AudioCompressionStd_eWmaPro;

    if ( handle->userConfig.wmaPro.sOutputCfg[0].ui32DRCEnable )
    {
        switch ( handle->userConfig.wmaPro.sOutputCfg[0].eDRCSetting )
        {
        default:
            BDBG_WRN(("Unrecognized WMA Pro DRC mode %u", handle->userConfig.wmaPro.sOutputCfg[0].eDRCSetting));
            /* Fall through */
        case BDSP_Raaga_Audio_eDrcSetting_High:
            handle->wmaProSettings.codecSettings.wmaPro.drcMode = BAPE_WmaProDrcMode_eHigh;
            break;
        case BDSP_Raaga_Audio_eDrcSetting_Med:
            handle->wmaProSettings.codecSettings.wmaPro.drcMode = BAPE_WmaProDrcMode_eMedium;
            break;
        case BDSP_Raaga_Audio_eDrcSetting_Low:
            handle->wmaProSettings.codecSettings.wmaPro.drcMode = BAPE_WmaProDrcMode_eLow;
            break;
        }
    }
    else
    {
        handle->wmaProSettings.codecSettings.wmaPro.drcMode = BAPE_WmaProDrcMode_eDisabled;
    }

    handle->wmaProSettings.codecSettings.wmaPro.rmsAmplitudeReference = handle->userConfig.wmaPro.sOutputCfg[0].i32RmsAmplitudeRef;
    handle->wmaProSettings.codecSettings.wmaPro.peakAmplitudeReference = handle->userConfig.wmaPro.sOutputCfg[0].i32PeakAmplitudeRef;
    handle->wmaProSettings.codecSettings.wmaPro.desiredRms = handle->userConfig.wmaPro.sOutputCfg[0].i32DesiredRms;
    handle->wmaProSettings.codecSettings.wmaPro.desiredPeak = handle->userConfig.wmaPro.sOutputCfg[0].i32DesiredPeak;
    handle->wmaProSettings.codecSettings.wmaPro.stereoMode = handle->userConfig.wmaPro.sOutputCfg[0].ui32Stereomode;
    BDBG_ASSERT(handle->wmaProSettings.codecSettings.wmaPro.stereoMode < BAPE_WmaProStereoMode_eMax);
}

static void BAPE_Decoder_P_GetDefaultDtsSettings(
    BAPE_DecoderHandle handle
    )
{
    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eDtsHdDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eDtsHdDecode, &handle->userConfig.dts, sizeof(handle->userConfig.dts)));

    handle->dtsSettings.codec = BAVC_AudioCompressionStd_eDts;

    handle->dtsSettings.codecSettings.dts.littleEndian = false;
    handle->dtsSettings.codecSettings.dts.drcMode = (BAPE_DtsDrcMode)handle->userConfig.dts.sUserOutputCfg[0].i32UserDRCFlag;
    handle->dtsSettings.codecSettings.dts.drcScaleHi = 100;
    BDBG_ASSERT(handle->userConfig.dts.sUserOutputCfg[0].i32DynScaleHigh == 0x7fffffff);
    handle->dtsSettings.codecSettings.dts.drcScaleLow = 100;
    BDBG_ASSERT(handle->userConfig.dts.sUserOutputCfg[0].i32DynScaleLow == 0x7fffffff);
    handle->dtsSettings.codecSettings.dts.stereoMode = handle->userConfig.dts.sUserOutputCfg[0].ui32StereoMode;
    handle->dtsSettings.codecSettings.dts.mixLfeToPrimary = handle->userConfig.dts.ui32MixLFE2Primary ? true : false;
}

static void BAPE_Decoder_P_GetDefaultDtsExpressSettings(
    BAPE_DecoderHandle handle
    )
{
    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eDtsLbrDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eDtsLbrDecode, &handle->userConfig.dtsExpress, sizeof(handle->userConfig.dtsExpress)));

    handle->dtsExpressSettings.codec = BAVC_AudioCompressionStd_eDtsExpress;

    handle->dtsExpressSettings.codecSettings.dts.littleEndian = false;
    handle->dtsExpressSettings.codecSettings.dts.drcMode = (BAPE_DtsDrcMode)handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32EnableDRC;
    handle->dtsExpressSettings.codecSettings.dts.drcScaleHi = 100;
    handle->dtsExpressSettings.codecSettings.dts.drcScaleLow = 100;
    handle->dtsExpressSettings.codecSettings.dts.stereoMode = handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32StereoMode;
    handle->dtsExpressSettings.codecSettings.dts.mixLfeToPrimary = handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32MixLFE2Primary ? true : false;
}

static void BAPE_Decoder_P_GetDefaultAdpcmSettings(
    BAPE_DecoderHandle handle
    )
{
    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eAdpcmDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eAdpcmDecode, &handle->userConfig.adpcm, sizeof(handle->userConfig.adpcm)));

    handle->adpcmSettings.codec = BAVC_AudioCompressionStd_eAdpcm;

    handle->adpcmSettings.codecSettings.adpcm.gain.enabled = handle->userConfig.adpcm.sUsrOutputCfg[0].ui32ApplyGain ? true : false;
    handle->adpcmSettings.codecSettings.adpcm.gain.factor = handle->userConfig.adpcm.sUsrOutputCfg[0].i32GainFactor;
}

static void BAPE_Decoder_P_GetDefaultIlbcSettings(
    BAPE_DecoderHandle handle
    )
{
    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eiLBCDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eiLBCDecode, &handle->userConfig.ilbc, sizeof(handle->userConfig.ilbc)));

    handle->ilbcSettings.codec = BAVC_AudioCompressionStd_eIlbc;

    handle->ilbcSettings.codecSettings.ilbc.gain.enabled = handle->userConfig.ilbc.sUsrOutputCfg[0].ui32ScaleOp ? true : false;
    handle->ilbcSettings.codecSettings.ilbc.gain.factor = (handle->userConfig.ilbc.sUsrOutputCfg[0].ui32ScaleIdx+1)*300;
    handle->ilbcSettings.codecSettings.ilbc.frameLength = handle->userConfig.ilbc.sUsrOutputCfg[0].mode;
    handle->ilbcSettings.codecSettings.ilbc.packetLoss = handle->userConfig.ilbc.sUsrOutputCfg[0].plc;
}

static void BAPE_Decoder_P_GetDefaultIsacSettings(
    BAPE_DecoderHandle handle
    )
{
    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eiSACDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eiSACDecode, &handle->userConfig.isac, sizeof(handle->userConfig.isac)));

    handle->isacSettings.codec = BAVC_AudioCompressionStd_eIsac;

    handle->isacSettings.codecSettings.isac.gain.enabled = handle->userConfig.isac.sUsrOutputCfg[0].ui32ScaleOp ? true : false;
    handle->isacSettings.codecSettings.isac.gain.factor = (handle->userConfig.isac.sUsrOutputCfg[0].ui32ScaleIdx+1)*300;
    handle->isacSettings.codecSettings.isac.bandMode = (handle->userConfig.isac.sUsrOutputCfg[0].ui32BandMode == 1) ? BAPE_IsacBandMode_eNarrow : BAPE_IsacBandMode_eWide;
    handle->isacSettings.codecSettings.isac.packetLoss = handle->userConfig.isac.sUsrOutputCfg[0].plc;
}

static void BAPE_Decoder_P_GetDefaultAlsSettings(BAPE_DecoderHandle handle)
{
    if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eALSDecode) )
    {
        return;
    }

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eALSDecode, &handle->userConfig.als, sizeof(handle->userConfig.als)));

    handle->alsSettings.codec = BAVC_AudioCompressionStd_eAls;
    switch ( handle->userConfig.als.sUsrOutputCfg[0].ui32OutMode )
    {
    default:
    case 1:
        handle->alsSettings.codecSettings.als.stereoMode = BAPE_AlsStereoMode_eArib;
        break;
    case 2:
        handle->alsSettings.codecSettings.als.stereoMode = BAPE_AlsStereoMode_eLtRt;
        break;
    }

    handle->alsSettings.codecSettings.als.aribMatrixMixdownIndex = handle->userConfig.als.i32AribMatrixMixdownIndex;
}

void BAPE_Decoder_P_GetDefaultCodecSettings(BAPE_DecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BAPE_Decoder_P_GetDefaultAc3Settings(handle);
    BAPE_Decoder_P_GetDefaultAc4Settings(handle);
    BAPE_Decoder_P_GetDefaultAacSettings(handle);
    BAPE_Decoder_P_GetDefaultWmaProSettings(handle);
    BAPE_Decoder_P_GetDefaultDtsSettings(handle);
    BAPE_Decoder_P_GetDefaultDtsExpressSettings(handle);
    BAPE_Decoder_P_GetDefaultAdpcmSettings(handle);
    BAPE_Decoder_P_GetDefaultIlbcSettings(handle);
    BAPE_Decoder_P_GetDefaultIsacSettings(handle);
    BAPE_Decoder_P_GetDefaultAlsSettings(handle);
}

/***************************************************************************
Summary:
Certification mode parameters for UDC Decoder
***************************************************************************/
#define BAPE_DOLBY_UDC_DECORRELATION                    ((unsigned)1<<0)
#define BAPE_DOLBY_UDC_MDCT_BANDLIMIT                   ((unsigned)1<<1)

/* |1111 1xxx xxxx xxxx| */
#define BAPE_DOLBY_UDC_OUTPUTMODE_CUSTOM_MASK           (0xf8000000)
#define BAPE_DOLBY_UDC_OUTPUTMODE_CUSTOM_SHIFT          (27)

static BERR_Code BAPE_Decoder_P_ApplyAc3Settings(BAPE_DecoderHandle handle, BAPE_Ac3Settings *pSettings)
{
    bool lfe;
    bool forceDrcModes=false;
    BERR_Code errCode;
    BAPE_ChannelMode channelMode;
    unsigned stereoOutputPort = 0;

    if ( handle->deviceHandle->settings.loudnessMode != BAPE_LoudnessEquivalenceMode_eNone )
    {
        forceDrcModes = true;
    }

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.ddp, sizeof(handle->userConfig.ddp));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Determine output mode */
    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }

    /* Setup global parameters */
    BAPE_Decoder_P_GetAFDecoderType(handle, &handle->userConfig.ddp.eDecoderType);

#if BAPE_DSP_LEGACY_DDP_ALGO
    if ( handle->ddre && BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        handle->userConfig.ddp.eDdpUsageMode = BDSP_Raaga_Audio_eMS11DecodeMode;
    }
    else if ( handle->fwMixer && handle->startSettings.mixingMode == BAPE_DecoderMixingMode_eDescription )
    {
        handle->userConfig.ddp.eDdpUsageMode = BDSP_Raaga_Audio_eMS10DecodeMode;
    }
    else
    {
        handle->userConfig.ddp.eDdpUsageMode = BDSP_Raaga_Audio_eSingleDecodeMode;
    }
#else
    handle->userConfig.ddp.ui32EnableAtmosMetadata = 0; /* disable Atmos metadata by default */
    if ( handle->ddre && BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        switch (handle->userConfig.ddp.eDecoderType)
        {
        case BDSP_AF_P_DecoderType_ePrimary:
            /* Only enable Atmos for Primary and only if requested from above */
            handle->userConfig.ddp.ui32EnableAtmosMetadata = pSettings->enableAtmosProcessing ? 1 : 0;
            break;
        default:
            break;
        }
        handle->userConfig.ddp.eDolbyMSUsageMode = BDSP_AF_P_DolbyMsUsageMode_eMS12DecodeMode;
    }
    /* Stereo mix with metadata is ot supported in MS12 mode
    if ( handle->fwMixer && handle->startSettings.mixingMode == BAPE_DecoderMixingMode_eDescription )
    {
        handle->userConfig.ddp.eDolbyMSUsageMode = BDSP_AF_P_DolbyMsUsageMode_eSingleDecodeMode;
    }
    */
    else
    {
        handle->userConfig.ddp.eDolbyMSUsageMode = BDSP_AF_P_DolbyMsUsageMode_eSingleDecodeMode;
    }
#endif

    handle->userConfig.ddp.ui32SubstreamIDToDecode = pSettings->substreamId;
    if ( BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        handle->userConfig.ddp.i32NumOutPorts = 2;
        handle->userConfig.ddp.sUserOutputCfg[1].i32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);    /* Stereo */
        handle->userConfig.ddp.sUserOutputCfg[1].i32OutLfe = 0;
    }
    else
    {
        handle->userConfig.ddp.i32NumOutPorts = 1;
    }
    handle->userConfig.ddp.i32StreamDialNormEnable = pSettings->dialogNormalization?1:0;
    handle->userConfig.ddp.i32UserDialNormVal = pSettings->dialogNormalizationValue;
#if BAPE_DSP_LEGACY_DDP_ALGO
    handle->userConfig.ddp.sUserOutputCfg[0].i32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);    /* Stereo or Multichannel */
#else
    if ( (pSettings->certificationMode & BAPE_DOLBY_UDC_DECORRELATION) != 0 )
    {
        BDBG_WRN(("certification mode De-correlation enabled"));
        handle->userConfig.ddp.sUserOutputCfg[0].i32decorr_mode = /*pSettings->enableDecorrelation*/1;
    }
    else
    {
        handle->userConfig.ddp.sUserOutputCfg[0].i32decorr_mode = /*pSettings->enableDecorrelation*/0;
    }
    if ( (pSettings->certificationMode & BAPE_DOLBY_UDC_OUTPUTMODE_CUSTOM_MASK) != 0 )
    {
        handle->userConfig.ddp.sUserOutputCfg[0].i32OutMode = (pSettings->certificationMode & BAPE_DOLBY_UDC_OUTPUTMODE_CUSTOM_MASK) >> BAPE_DOLBY_UDC_OUTPUTMODE_CUSTOM_SHIFT;

        if ( handle->userConfig.ddp.sUserOutputCfg[0].i32OutMode == 31 )
        {
            handle->userConfig.ddp.sUserOutputCfg[0].i32OutMode = -1;
        }
        BDBG_WRN(("Special certification output mode %d for UDC 7.1", handle->userConfig.ddp.sUserOutputCfg[0].i32OutMode));

        /* This is for setting correct output matrix for mono mode */
        if((handle->userConfig.ddp.sUserOutputCfg[0].i32OutMode == 1) && (channelMode <= BAPE_ChannelMode_e2_0))
        {
            channelMode = BAPE_ChannelMode_e1_0;
        }
    }
    else
    {
        handle->userConfig.ddp.sUserOutputCfg[0].i32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);    /* Stereo or Multichannel */
    }
    if ( (pSettings->certificationMode & BAPE_DOLBY_UDC_MDCT_BANDLIMIT) != 0 )
    {
        BDBG_WRN(("certification mode MDCT Band Limit Enabled"));
        handle->userConfig.ddp.sUserOutputCfg[0].i32MdctBandLimitEnable = 1;
    }
    else
    {
        handle->userConfig.ddp.sUserOutputCfg[0].i32MdctBandLimitEnable = 0;
    }
#endif
    handle->userConfig.ddp.sUserOutputCfg[0].i32OutLfe = lfe?1:0;
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.ddp.sUserOutputCfg[0].ui32OutputChannelMatrix);

    if (handle->userConfig.ddp.i32NumOutPorts == 2)
    {
        stereoOutputPort = 1;
    }
    if ( forceDrcModes )
    {
        BDBG_MSG(("Loudness Management Active for AC3/DDP"));
        switch (handle->deviceHandle->settings.loudnessMode)
        {
        case BAPE_LoudnessEquivalenceMode_eAtscA85:
#if BAPE_DSP_LEGACY_DDP_ALGO
            handle->userConfig.ddp.sUserOutputCfg[stereoOutputPort].i32CompMode = 6;   /* -24dB for stereo */
#else
            handle->userConfig.ddp.sUserOutputCfg[stereoOutputPort].i32CompMode = 10;   /* -24dB for stereo */
#endif
            break;
        case BAPE_LoudnessEquivalenceMode_eEbuR128:
#if BAPE_DSP_LEGACY_DDP_ALGO
            handle->userConfig.ddp.sUserOutputCfg[stereoOutputPort].i32CompMode = 5;   /* -23dB for stereo */
#else
            handle->userConfig.ddp.sUserOutputCfg[stereoOutputPort].i32CompMode = 9;   /* -23dB for stereo */
#endif
            break;
        default:
            handle->userConfig.ddp.sUserOutputCfg[stereoOutputPort].i32CompMode = 3;   /* RF mode for stereo (-20dB) */
            break;
        }
        if (handle->userConfig.ddp.i32NumOutPorts == 2){
            handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 2;   /* Line mode for multichannel (-31dB) */
        }

        BDBG_MODULE_MSG(bape_loudness,("%s AC3 decoder is configured for %s loudness mode%s",
            (BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 ? "MS-12" : BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eNone ? "Legacy":"MS-10"),
            (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? "ATSC" :
             handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? "EBU" : "DISABLED"),
            (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? ", expected input level -31 expected output level -24" :
             handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? ", expected input level -23 expected output level -23" : "")));
        BDBG_MODULE_MSG(bape_loudness,("Stereo i32CompMode %d ", handle->userConfig.ddp.sUserOutputCfg[stereoOutputPort].i32CompMode));
    }
    else
    {
        BDBG_MSG(("Loudness Management NOT Active for AC3/DDP"));
        if ( BAPE_Decoder_P_HasStereoOutput(handle) && !BAPE_Decoder_P_HasMultichannelOutput(handle) ) /* just stereo output */
        {
            switch ( pSettings->drcModeDownmix )
            {
            case BAPE_Ac3DrcMode_eCustomA:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 0;
                break;
            case BAPE_Ac3DrcMode_eCustomD:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 1;
                break;
            case BAPE_Ac3DrcMode_eLine:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 2;
                break;
            case BAPE_Ac3DrcMode_eRf:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 3;
                break;
            default:
            case BAPE_Ac3DrcMode_eDisabled:
#if BAPE_DSP_LEGACY_DDP_ALGO
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 4;
#else
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 8;
#endif
                break;
            }
        }
        else if ( !BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle) ) /* just multichannel output */
        {
            switch ( pSettings->drcMode )
            {
            case BAPE_Ac3DrcMode_eCustomA:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 0;
                break;
            case BAPE_Ac3DrcMode_eCustomD:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 1;
                break;
            case BAPE_Ac3DrcMode_eLine:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 2;
                break;
            case BAPE_Ac3DrcMode_eRf:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 3;
                break;
            default:
            case BAPE_Ac3DrcMode_eDisabled:
#if BAPE_DSP_LEGACY_DDP_ALGO
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 4;
#else
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 8;
#endif
                break;
            }
        }
        else
        {
            switch ( pSettings->drcMode )
            {
            case BAPE_Ac3DrcMode_eCustomA:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 0;
                break;
            case BAPE_Ac3DrcMode_eCustomD:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 1;
                break;
            case BAPE_Ac3DrcMode_eLine:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 2;
                break;
            case BAPE_Ac3DrcMode_eRf:
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 3;
                break;
            default:
            case BAPE_Ac3DrcMode_eDisabled:
#if BAPE_DSP_LEGACY_DDP_ALGO
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 4;
#else
                handle->userConfig.ddp.sUserOutputCfg[0].i32CompMode = 8;
#endif
                break;
            }
            switch ( pSettings->drcModeDownmix )
            {
            case BAPE_Ac3DrcMode_eCustomA:
                handle->userConfig.ddp.sUserOutputCfg[1].i32CompMode = 0;
                break;
            case BAPE_Ac3DrcMode_eCustomD:
                handle->userConfig.ddp.sUserOutputCfg[1].i32CompMode = 1;
                break;
            case BAPE_Ac3DrcMode_eLine:
                handle->userConfig.ddp.sUserOutputCfg[1].i32CompMode = 2;
                break;
            case BAPE_Ac3DrcMode_eRf:
                handle->userConfig.ddp.sUserOutputCfg[1].i32CompMode = 3;
                break;
            default:
            case BAPE_Ac3DrcMode_eDisabled:
#if BAPE_DSP_LEGACY_DDP_ALGO
                handle->userConfig.ddp.sUserOutputCfg[1].i32CompMode = 4;
#else
                handle->userConfig.ddp.sUserOutputCfg[1].i32CompMode = 8;
#endif
                break;
            }
        }
    }

    switch ( handle->settings.dualMonoMode )
    {
    default:
    case BAPE_DualMonoMode_eStereo:
        handle->userConfig.ddp.sUserOutputCfg[0].i32DualMode = 0;
        handle->userConfig.ddp.sUserOutputCfg[1].i32DualMode = 0;
        break;
    case BAPE_DualMonoMode_eLeft:
        handle->userConfig.ddp.sUserOutputCfg[0].i32DualMode = 1;
        handle->userConfig.ddp.sUserOutputCfg[1].i32DualMode = 1;
        break;
    case BAPE_DualMonoMode_eRight:
        handle->userConfig.ddp.sUserOutputCfg[0].i32DualMode = 2;
        handle->userConfig.ddp.sUserOutputCfg[1].i32DualMode = 2;
        break;
    case BAPE_DualMonoMode_eMix:
        handle->userConfig.ddp.sUserOutputCfg[0].i32DualMode = 3;
        handle->userConfig.ddp.sUserOutputCfg[1].i32DualMode = 3;
        break;
    }
    switch ( pSettings->stereoMode )
    {
    default:
    case BAPE_Ac3StereoMode_eAuto:
        handle->userConfig.ddp.sUserOutputCfg[0].i32StereoMode = 0;
        handle->userConfig.ddp.sUserOutputCfg[1].i32StereoMode = 0;
        break;
    case BAPE_Ac3StereoMode_eLtRt:
        handle->userConfig.ddp.sUserOutputCfg[0].i32StereoMode = 1;
        handle->userConfig.ddp.sUserOutputCfg[1].i32StereoMode = 1;
        break;
    case BAPE_Ac3StereoMode_eLoRo:
        handle->userConfig.ddp.sUserOutputCfg[0].i32StereoMode = 2;
        handle->userConfig.ddp.sUserOutputCfg[1].i32StereoMode = 2;
        break;
    }
#if BAPE_DSP_LEGACY_DDP_ALGO
    handle->userConfig.ddp.sUserOutputCfg[0].i32PcmScale = BAPE_P_FloatToQ131(pSettings->scale, 100);
    handle->userConfig.ddp.sUserOutputCfg[1].i32PcmScale = BAPE_P_FloatToQ131(pSettings->scaleDownmix, 100);
    handle->userConfig.ddp.sUserOutputCfg[0].i32DynScaleHigh = BAPE_P_FloatToQ131(pSettings->drcScaleHi, 100);
    handle->userConfig.ddp.sUserOutputCfg[0].i32DynScaleLow = BAPE_P_FloatToQ131(pSettings->drcScaleLow, 100);
    handle->userConfig.ddp.sUserOutputCfg[1].i32DynScaleHigh = BAPE_P_FloatToQ131(pSettings->drcScaleHiDownmix, 100);
    handle->userConfig.ddp.sUserOutputCfg[1].i32DynScaleLow = BAPE_P_FloatToQ131(pSettings->drcScaleLowDownmix, 100);
#else
    handle->userConfig.ddp.sUserOutputCfg[0].i32PcmScale = pSettings->scale;
    handle->userConfig.ddp.sUserOutputCfg[1].i32PcmScale = pSettings->scaleDownmix;
    handle->userConfig.ddp.sUserOutputCfg[0].i32DynScaleHigh = pSettings->drcScaleHi;
    handle->userConfig.ddp.sUserOutputCfg[0].i32DynScaleLow = pSettings->drcScaleLow;
    handle->userConfig.ddp.sUserOutputCfg[1].i32DynScaleHigh = pSettings->drcScaleHiDownmix;
    handle->userConfig.ddp.sUserOutputCfg[1].i32DynScaleLow = pSettings->drcScaleLowDownmix;
#endif
    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.ddp, sizeof(handle->userConfig.ddp));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }


    return BERR_SUCCESS;
}

#define BAPE_DECODER_INVALID_USERCONFIG_ID 0xffffffff
static BERR_Code BAPE_Decoder_P_ApplyAc4Settings(BAPE_DecoderHandle handle, BAPE_Ac4Settings *pSettings)
{
    bool lfe;
    bool forceDrcModes=false;
    BERR_Code errCode;
    BAPE_ChannelMode channelMode;
    unsigned stereoOutputPort = BAPE_DECODER_INVALID_USERCONFIG_ID;
    unsigned multichOutputPort = BAPE_DECODER_INVALID_USERCONFIG_ID;

    if ( handle->deviceHandle->settings.loudnessMode != BAPE_LoudnessEquivalenceMode_eNone )
    {
        forceDrcModes = true;
    }

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.ac4, sizeof(handle->userConfig.ac4));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, i32NumOutPorts, 2);
        multichOutputPort = 0;
        stereoOutputPort = 1;
    }
    else if ( BAPE_Decoder_P_HasStereoOutput(handle) )
    {
        /* only Stereo output port */
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, i32NumOutPorts, 1);
        stereoOutputPort = 0;
    }
    else if ( BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        /* only Multich output port */
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, i32NumOutPorts, 1);
        multichOutputPort = 0;
    }
    else
    {
        BDBG_ERR(("%s, No Valid Output Port found!", __FUNCTION__));
        BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Determine output mode */
    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }

    /* Setup global parameters */
    BAPE_Decoder_P_GetAFDecoderType(handle, &handle->userConfig.ac4.eDecoderType);

    if ( handle->ddre && BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, eDolbyMSUsageMode, 1);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, eDolbyMSUsageMode, 0);
    }

    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.ac4.sUserOutputCfg[0].ui32OutputChannelMatrix);

    if ( multichOutputPort != BAPE_DECODER_INVALID_USERCONFIG_ID )
    {
        if ( forceDrcModes )
        {
            BDBG_MSG(("Loudness Management Active for AC4 Multichannel"));
            BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[multichOutputPort].ui32DrcMode, 1); /* Line mode for multichannel (-31dB) */
        }
        else
        {
            BDBG_MSG(("Loudness Management NOT Active for AC4 Multichannel"));
            switch ( pSettings->drcMode )
            {
            case BAPE_Ac3DrcMode_eLine:
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[multichOutputPort].ui32DrcMode, 1);
                break;
            case BAPE_Ac3DrcMode_eRf:
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[multichOutputPort].ui32DrcMode, 2);
                break;
            default:
            case BAPE_Ac3DrcMode_eDisabled:
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[multichOutputPort].ui32DrcMode, 0);
                break;
            }
        }
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[multichOutputPort].i32MainAssocMixPref, BAPE_DSP_P_VALIDATE_VARIABLE_RANGE(handle->ac4Settings.codecSettings.ac4.programBalance,-32,32));
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[multichOutputPort].i32DialogEnhGainInput, BAPE_DSP_P_VALIDATE_VARIABLE_RANGE(handle->ac4Settings.codecSettings.ac4.dialogEnhancerAmount,-12,12));
    }

    if ( stereoOutputPort != BAPE_DECODER_INVALID_USERCONFIG_ID )
    {
        if ( forceDrcModes )
        {
            BDBG_MSG(("Loudness Management Active for AC4 Stereo"));
            switch (handle->deviceHandle->settings.loudnessMode)
            {
            case BAPE_LoudnessEquivalenceMode_eAtscA85:
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].ui32DrcMode, 4); /* -24dB for stereo */
                break;
            case BAPE_LoudnessEquivalenceMode_eEbuR128:
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].ui32DrcMode, 3); /* -23dB for stereo */
                break;
            default:
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].ui32DrcMode, 2); /* RF mode for stereo (-20dB) */
                break;
            }
            BDBG_MODULE_MSG(bape_loudness,("AC4 decoder (Stereo Port) is configured for %s loudness mode%s",
                (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? "ATSC" :
                 handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? "EBU" : "DISABLED"),
                (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? ", expected input level -31 expected output level -24" :
                 handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? ", expected input level -23 expected output level -23" : "")));
            BDBG_MODULE_MSG(bape_loudness,("Stereo ui32DrcMode %d ", handle->userConfig.ac4.sUserOutputCfg[stereoOutputPort].ui32DrcMode));
        }
        else
        {
            BDBG_MSG(("Loudness Management NOT Active for AC4 Stereo"));
            switch ( pSettings->drcModeDownmix )
            {
            case BAPE_Ac3DrcMode_eLine:
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].ui32DrcMode, 1);
                break;
            case BAPE_Ac3DrcMode_eRf:
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].ui32DrcMode, 2);
                break;
            default:
            case BAPE_Ac3DrcMode_eDisabled:
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].ui32DrcMode, 0);
                break;
            }
        }

        switch ( pSettings->stereoMode )
        {
        default:
        case BAPE_DolbyStereoMode_eLoRo:
            BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].ui32ChannelConfig, 0);
            break;
        case BAPE_DolbyStereoMode_eLtRt:
            BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].ui32ChannelConfig, 2);
            break;
        }

        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].i32MainAssocMixPref, BAPE_DSP_P_VALIDATE_VARIABLE_RANGE(handle->ac4Settings.codecSettings.ac4.programBalance,-32,32));
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, sUserOutputCfg[stereoOutputPort].i32DialogEnhGainInput, BAPE_DSP_P_VALIDATE_VARIABLE_RANGE(handle->ac4Settings.codecSettings.ac4.dialogEnhancerAmount,-12,12));
    }

    BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, ui32MainAssocDec, handle->ac4Settings.codecSettings.ac4.programSelection);
    BAPE_DSP_P_SET_VARIABLE(handle->userConfig.ac4, ui32PresentationNumber, handle->ac4Settings.codecSettings.ac4.presentationId);
    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.ac4, sizeof(handle->userConfig.ac4));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }


    return BERR_SUCCESS;
}

#if BDSP_MS12_SUPPORT
static BERR_Code BAPE_Decoder_P_ApplyMs12AacSettings(
    BAPE_DecoderHandle handle,
    const BAPE_AacSettings *pSettings
    )
{
    BERR_Code errCode;
    bool lfe, multichannel;
    BAPE_ChannelMode channelMode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.aac, sizeof(handle->userConfig.aac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    else if ( handle->fwMixer && handle->startSettings.mixingMode == BAPE_DecoderMixingMode_eDescription )
    {
        handle->userConfig.aac.eDolbyAacheUsageMode = BDSP_AF_P_DolbyMsUsageMode_eMS12DecodeMode;
    }
    else
    {
        if ( pSettings->mpegConformanceMode )
        {
            handle->userConfig.aac.eDolbyAacheUsageMode = BDSP_AF_P_DolbyMsUsageMode_eMpegConformanceMode;
        }
        else
        {
            handle->userConfig.aac.eDolbyAacheUsageMode = BDSP_AF_P_DolbyMsUsageMode_eSingleDecodeMode;
        }
    }

    BAPE_Decoder_P_GetAFDecoderType(handle, &handle->userConfig.aac.eDecoderType);

    /* Force RF mode for loudness equivalence.  This will normalize output level at -20dB.  */
    BDBG_MSG(("Loudness Management Active for Dolby AacHe"));
    switch ( handle->deviceHandle->settings.loudnessMode )
    {
    case BAPE_LoudnessEquivalenceMode_eAtscA85:
        BDBG_MSG(("A/85 Loudness Equivalence Enabled for AAC-HE"));
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, 24*4); /* Default PRL (input) */
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32RefDialnormLevel, 31*4); /* Reference PRL (output) */
        if (pSettings->drcReferenceLevel == 4*24)
        {
            BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32LoudnessEquivalenceMode, 4); /* ATSC -24 all outputs */
        }
        else
        {
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32LoudnessEquivalenceMode, 2); /* ATSC */
        }
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DrcType, 0);
        break;
    case BAPE_LoudnessEquivalenceMode_eEbuR128:
        BDBG_MSG(("EBU-R128 Loudness Equivalence Enabled for AAC-HE"));
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, 23*4); /* Default PRL (input) */
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32RefDialnormLevel, 31*4); /* Reference PRL (output) */
        if (pSettings->drcReferenceLevel == 4*23)
        {
            BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32LoudnessEquivalenceMode, 3); /* EBU -23 all outputs */
        }
        else
        {
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32LoudnessEquivalenceMode, 1); /* EBU */
        }
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DrcType, 0);
        break;
    default:
        BDBG_MSG(("Loudness Equivalence Disabled for AAC-HE"));
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, pSettings->drcDefaultLevel);
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32RefDialnormLevel, pSettings->drcReferenceLevel);
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32LoudnessEquivalenceMode, 0); /* None */
        if ( pSettings->drcMode == BAPE_DolbyPulseDrcMode_eRf )
        {
            BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DrcType, 4);
            if (pSettings->drcReferenceLevel == 4*23)
            {
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, 23*4);
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32RefDialnormLevel, 31*4);
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DrcType, 0);
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32LoudnessEquivalenceMode, 3);
            }
            else if (pSettings->drcReferenceLevel == 4*24)
            {
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, 24*4);
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32RefDialnormLevel, 31*4);
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DrcType, 0);
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32LoudnessEquivalenceMode, 4);
            }
        }
        else
        {
            BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DrcType, 0);
        }
        break;
    }

    BDBG_MODULE_MSG(bape_loudness,("MS-12 AAC decoder is configured for %s loudness mode%s",
        (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? "ATSC" :
         handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? "EBU" : "DISABLED"),
        (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? ", expected input level -24 expected output level -24" :
         handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? ", expected input level -23 expected output level -23" : "")));

    BDBG_MODULE_MSG(bape_loudness,("ui32DefDialnormLevel %d ui32RefDialnormLevel %d ui32LoudnessEquivalenceMode %d",
         handle->userConfig.aac.ui32DefDialnormLevel, handle->userConfig.aac.ui32RefDialnormLevel, handle->userConfig.aac.ui32LoudnessEquivalenceMode));

    multichannel = BAPE_Decoder_P_HasMultichannelOutput(handle);
    /* Determine if the only thing connected to multichannel is the transcoder.  If so, pretend we don't have any multichannel outputs */
    if ( multichannel && handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].directConnections == 1 )
    {
        BAPE_PathConnection *pConnection;
        pConnection = BLST_SQ_FIRST(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].connectionList);
        BDBG_ASSERT(NULL != pConnection);
        if ( pConnection->pSink->type == BAPE_PathNodeType_eEncoder && pConnection->pSink->subtype == BAVC_AudioCompressionStd_eAc3 )
        {
            /* Only connected node is dolby transcoder.  Don't output multichannel. */
            multichannel = false;
        }
    }
#if 0
    if ( BAPE_Decoder_P_HasStereoOutput(handle) && multichannel )
    {
        BAPE_DSP_P_GetChannelMatrix(BAPE_ChannelMode_e2_0, false, handle->userConfig.aac.sOutPortCfg[1].ui32OutputChannelMatrix);
    }
    else
    {
        /* how do we tell BDSP there is no stereo downmix?? */
        /* handle->userConfig.aac.ui32NumOutPorts = 1; */
    }
#else
    BAPE_DSP_P_GetChannelMatrix(BAPE_ChannelMode_e2_0, false, handle->userConfig.aac.sOutPortCfg[1].ui32OutputChannelMatrix);
#endif

    switch ( pSettings->downmixMode )
    {
    default:
    case BAPE_AacStereoMode_eLtRt:
        handle->userConfig.aac.ui32EnableStereoDownmixType = 1;
        break;
    case BAPE_AacStereoMode_eLoRo:
        handle->userConfig.aac.ui32EnableStereoDownmixType = 0;
        break;
    case BAPE_AacStereoMode_eArib:
        handle->userConfig.aac.ui32EnableStereoDownmixType = 2;
        break;
    }

    /* Determine output mode */
    lfe = handle->settings.outputLfe;
    channelMode = BAPE_DSP_P_GetChannelMode(handle->startSettings.codec, handle->settings.outputMode, multichannel, handle->settings.multichannelFormat);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }

#if 0
    handle->userConfig.aac.sOutPortCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);    /* Multichannel */
    handle->userConfig.aac.sOutPortCfg[0].ui32OutLfe = lfe?1:0;
#endif

    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.aac.sOutPortCfg[0].ui32OutputChannelMatrix);

    /* If DDRE is downstream, do not upsample*/
    if ( handle->ddre )
    {
        handle->userConfig.aac.ui32UpsampleToPrimaryRates = 0;
    }
    else
    {
        handle->userConfig.aac.ui32UpsampleToPrimaryRates = 1;
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.aac, sizeof(handle->userConfig.aac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

#elif BDSP_MS10_SUPPORT

static BERR_Code BAPE_Decoder_P_ApplyMs10AacSettings(
    BAPE_DecoderHandle handle,
    const BAPE_AacSettings *pSettings
    )
{
    BERR_Code errCode;
    bool lfe, multichannel;
    bool forceDrcModes=false;
    BAPE_ChannelMode channelMode;
    unsigned stereoOutputPort = 0;

    if ( handle->deviceHandle->settings.loudnessMode != BAPE_LoudnessEquivalenceMode_eNone )
    {
        forceDrcModes = true;
    }

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.aac, sizeof(handle->userConfig.aac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( handle->ddre && BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        handle->userConfig.aac.eDolbyPulseUsageMode = BDSP_AF_P_DolbyMsUsageMode_eMS11DecodeMode;
    }
    else if ( handle->fwMixer  && handle->startSettings.mixingMode == BAPE_DecoderMixingMode_eDescription )
    {
        handle->userConfig.aac.eDolbyPulseUsageMode = BDSP_AF_P_DolbyMsUsageMode_eMS10DecodeMode;
    }
    else
    {
        if ( pSettings->mpegConformanceMode )
        {
            handle->userConfig.aac.eDolbyPulseUsageMode = BDSP_AF_P_DolbyMsUsageMode_eMpegConformanceMode;
        }
        else
        {
            handle->userConfig.aac.eDolbyPulseUsageMode = BDSP_AF_P_DolbyMsUsageMode_eSingleDecodeMode;
        }
    }

    BAPE_Decoder_P_GetAFDecoderType(handle, &handle->userConfig.aac.eDecoderType);

    handle->userConfig.aac.sOutPortCfg[0].ui32DrcCut = pSettings->drcScaleHi;
    handle->userConfig.aac.sOutPortCfg[0].ui32DrcBoost = pSettings->drcScaleLow;
    if ( NULL == handle->fwMixer ||
        ( handle->fwMixer && handle->startSettings.mixingMode != BAPE_DecoderMixingMode_eDescription ) ||
        ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS11 ) )
    {
        switch ( pSettings->downmixMode )
        {
        default:
        case BAPE_AacStereoMode_eLtRt:
            handle->userConfig.aac.sOutPortCfg[0].ui32LoRoDownmix = 0;
            break;
        case BAPE_AacStereoMode_eLoRo:
            handle->userConfig.aac.sOutPortCfg[0].ui32LoRoDownmix = 1;
            break;
        case BAPE_AacStereoMode_eArib:
            handle->userConfig.aac.sOutPortCfg[0].ui32LoRoDownmix = 2;
            break;
        }
    }
    else
    {
        /* In multi-stream modes, force LoRo always */
        handle->userConfig.aac.sOutPortCfg[0].ui32LoRoDownmix = 1;
    }
    handle->userConfig.aac.sOutPortCfg[0].ui32ApplyGain = (pSettings->enableGainFactor == true) ? 1 : 0;
    handle->userConfig.aac.sOutPortCfg[0].i32GainFactor = pSettings->gainFactor;

    {
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, pSettings->drcDefaultLevel);
        BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32EnforceLoudnessLevelsOnAllPorts, 0);
        switch (pSettings->drcMode)
        {
        case BAPE_DolbyPulseDrcMode_eRf:
            handle->userConfig.aac.sOutPortCfg[0].ui32RfMode = 1;
            if (pSettings->drcReferenceLevel == 23*4)
            {
                handle->userConfig.aac.sOutPortCfg[0].ui32RfMode = 2;
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, 23*4);
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32EnforceLoudnessLevelsOnAllPorts, 1);
            }
            else if (pSettings->drcReferenceLevel == 24*4)
            {
                handle->userConfig.aac.sOutPortCfg[0].ui32RfMode = 3;
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, 24*4);
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32EnforceLoudnessLevelsOnAllPorts, 1);
            }
            break;
        default:
            handle->userConfig.aac.sOutPortCfg[0].ui32RfMode = 0;
            break;
        }
    }

    switch ( handle->settings.dualMonoMode )
    {
    default:
    case BAPE_DualMonoMode_eStereo:
        /* TODO: This is not documented, assume it's the same as others */
        handle->userConfig.aac.sOutPortCfg[0].ui32DualMode = 0;
        break;
    case BAPE_DualMonoMode_eLeft:
        handle->userConfig.aac.sOutPortCfg[0].ui32DualMode = 1;
        break;
    case BAPE_DualMonoMode_eRight:
        handle->userConfig.aac.sOutPortCfg[0].ui32DualMode = 2;
        break;
    case BAPE_DualMonoMode_eMix:
        handle->userConfig.aac.sOutPortCfg[0].ui32DualMode = 3;
        break;
    }
    handle->userConfig.aac.sOutPortCfg[1] = handle->userConfig.aac.sOutPortCfg[0];  /* Duplicate settings to both outputs for now */

    multichannel = BAPE_Decoder_P_HasMultichannelOutput(handle);
    if ( forceDrcModes )
    {
        if (multichannel)
        {
            stereoOutputPort = 1;
        }
        /* Force RF mode for loudness equivalence. */
        switch ( handle->deviceHandle->settings.loudnessMode )
        {
        case BAPE_LoudnessEquivalenceMode_eAtscA85:
            BDBG_MSG(("A/85 Loudness Equivalence Enabled for AAC-HE"));
            handle->userConfig.aac.sOutPortCfg[stereoOutputPort].ui32RfMode = 3;   /* -24dB for stereo */
            BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, 24*4); /* Input level */
            if (pSettings->drcReferenceLevel == 24*4)
            {
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32EnforceLoudnessLevelsOnAllPorts, 1);
            }
            break;
        case BAPE_LoudnessEquivalenceMode_eEbuR128:
            BDBG_MSG(("EBU-R128 Loudness Equivalence Enabled for AAC-HE"));
            handle->userConfig.aac.sOutPortCfg[stereoOutputPort].ui32RfMode = 2;   /* -23dB for stereo */
            BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32DefDialnormLevel, 23*4); /* Input level */
            if (pSettings->drcReferenceLevel == 23*4)
            {
                BAPE_DSP_P_SET_VARIABLE(handle->userConfig.aac, ui32EnforceLoudnessLevelsOnAllPorts, 1);
            }
            break;
        default:
            BDBG_MSG(("Loudness Equivalence Disabled for AAC-HE"));
            handle->userConfig.aac.sOutPortCfg[stereoOutputPort].ui32RfMode = 1;   /* RF mode for stereo (-20dB) */;
            break;
        }

        if (multichannel)
        {
            handle->userConfig.aac.sOutPortCfg[0].ui32RfMode = 0;   /* Line mode for multichannel (-31dB) */
        }
        BDBG_MSG(("Loudness Management Active for Dolby Pulse"));
        BDBG_MODULE_MSG(bape_loudness,("Dolby Pulse AAC decoder is configured for %s loudness mode%s",
             (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? "ATSC" :
              handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? "EBU" : "DISABLED"),
             (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? ", expected input level -24 expected output level -24" :
              handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? ", expected input level -23 expected output level -23" : "")));
        BDBG_MODULE_MSG(bape_loudness,("ui32DefDialnormLevel %d ui32RfMode %d",
            handle->userConfig.aac.ui32DefDialnormLevel, handle->userConfig.aac.sOutPortCfg[stereoOutputPort].ui32RfMode));
    }
    else
    {
        BDBG_MSG(("Loudness Equivalence not active for Dolby Pulse"));
    }

    /* Determine if the only thing connected to multichannel is the transcoder.  If so, pretend we don't have any multichannel outputs */
    if ( multichannel && handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].directConnections == 1 )
    {
        BAPE_PathConnection *pConnection;
        pConnection = BLST_SQ_FIRST(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].connectionList);
        BDBG_ASSERT(NULL != pConnection);
        if ( pConnection->pSink->type == BAPE_PathNodeType_eEncoder && pConnection->pSink->subtype == BAVC_AudioCompressionStd_eAc3 )
        {
            /* Only connected node is dolby transcoder.  Don't output multichannel. */
            multichannel = false;
        }
    }
    if ( BAPE_Decoder_P_HasStereoOutput(handle) && multichannel )
    {
        handle->userConfig.aac.ui32NumOutPorts = 2;
        handle->userConfig.aac.sOutPortCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);    /* Stereo */
        handle->userConfig.aac.sOutPortCfg[1].ui32OutLfe = 0;
    }
    else
    {
        handle->userConfig.aac.ui32NumOutPorts = 1;
    }

    /* Determine output mode */
    lfe = handle->settings.outputLfe;
    channelMode = BAPE_DSP_P_GetChannelMode(handle->startSettings.codec, handle->settings.outputMode, multichannel, handle->settings.multichannelFormat);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }

    handle->userConfig.aac.sOutPortCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);    /* Multichannel */
    handle->userConfig.aac.sOutPortCfg[0].ui32OutLfe = lfe?1:0;
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.aac.sOutPortCfg[0].ui32OutputChannelMatrix);

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.aac, sizeof(handle->userConfig.aac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

#else

static BERR_Code BAPE_Decoder_P_ApplyLegacyAacSettings(
    BAPE_DecoderHandle handle,
    const BAPE_AacSettings *pSettings
    )
{
    BERR_Code errCode;
    unsigned i,j;
    bool lfe;
    BAPE_ChannelMode channelMode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.aac, sizeof(handle->userConfig.aac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_Decoder_P_GetAFDecoderType(handle, &handle->userConfig.aac.eDecoderType);

    handle->userConfig.aac.ui32SbrUserFlag = pSettings->enableSbrDecoding ? 1 : 0;
    handle->userConfig.aac.ui32DrcGainControlCompress = BAPE_P_FloatToQ230(pSettings->drcScaleHi);
    handle->userConfig.aac.ui32DrcGainControlBoost = BAPE_P_FloatToQ230(pSettings->drcScaleLow);
    /* handle->userConfig.aac.ui32DrcTargetLevel = pSettings->drcTargetLevel; */
    handle->userConfig.aac.i32DownmixType = (pSettings->downmixMode == BAPE_AacStereoMode_eMatrix)?0:1;
    handle->userConfig.aac.ui32DownmixCoefScaleIndex = pSettings->downmixCoefScaleIndex;

    if ( BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        handle->userConfig.aac.i32NumOutPorts = 2;
        handle->userConfig.aac.sUserOutputCfg[1].i32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);    /* Stereo */
        handle->userConfig.aac.sUserOutputCfg[1].i32OutLfe = 0;
    }
    else
    {
        handle->userConfig.aac.i32NumOutPorts = 1;
    }

    /* Determine output mode */
    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }

    handle->userConfig.aac.sUserOutputCfg[0].i32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);    /* Multichannel */
    handle->userConfig.aac.sUserOutputCfg[0].i32OutLfe = lfe?1:0;
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.aac.sUserOutputCfg[0].ui32OutputChannelMatrix);

    switch ( handle->settings.dualMonoMode )
    {
    default:
    case BAPE_DualMonoMode_eStereo:
        handle->userConfig.aac.sUserOutputCfg[0].i32DualMode = 0;
        handle->userConfig.aac.sUserOutputCfg[1].i32DualMode = 0;
        break;
    case BAPE_DualMonoMode_eLeft:
        handle->userConfig.aac.sUserOutputCfg[0].i32DualMode = 1;
        handle->userConfig.aac.sUserOutputCfg[1].i32DualMode = 1;
        break;
    case BAPE_DualMonoMode_eRight:
        handle->userConfig.aac.sUserOutputCfg[0].i32DualMode = 2;
        handle->userConfig.aac.sUserOutputCfg[1].i32DualMode = 2;
        break;
    case BAPE_DualMonoMode_eMix:
        handle->userConfig.aac.sUserOutputCfg[0].i32DualMode = 3;
        handle->userConfig.aac.sUserOutputCfg[1].i32DualMode = 3;
        break;
    }

    handle->userConfig.aac.sUserOutputCfg[0].i32ExtDnmixEnabled = (pSettings->enableDownmixCoefficients)?1:0;
    handle->userConfig.aac.sUserOutputCfg[1].i32ExtDnmixEnabled = (pSettings->enableDownmixCoefficients)?1:0;
    for ( i = 0; i < 6; i++ )
    {
        for ( j = 0; j < 6; j++ )
        {
            handle->userConfig.aac.sUserOutputCfg[0].i32ExtDnmixTab[i][j] = pSettings->downmixCoefficients[i][j];
            handle->userConfig.aac.sUserOutputCfg[1].i32ExtDnmixTab[i][j] = pSettings->downmixCoefficients[i][j];
        }
    }

    switch ( handle->deviceHandle->settings.loudnessMode )
    {
    case BAPE_LoudnessEquivalenceMode_eAtscA85:
        BDBG_MSG(("A/85 Loudness Equivalence Enabled for AAC-HE"));
        handle->userConfig.aac.i32InputVolLevel = -24;
        if (BAPE_Decoder_P_HasMultichannelOutput(handle))
        {
            handle->userConfig.aac.i32OutputVolLevel = -31;   /* We have a multichannel output so configure for -31dB */
        }
        else
        {
            handle->userConfig.aac.i32OutputVolLevel = -24;   /* There is no multichannel output so configure for -24dB */
        }
        if (pSettings->drcReferenceLevel == 4*24)
        {
            handle->userConfig.aac.ui32LoudnessEquivalenceMode = 4; /* Config stereo for ATSC -24 on all output ports */
        }
        else
        {
        handle->userConfig.aac.ui32LoudnessEquivalenceMode = 2; /* Config stereo for ATSC -24 */
        }
        break;
    case BAPE_LoudnessEquivalenceMode_eEbuR128:
        BDBG_MSG(("EBU-R128 Loudness Equivalence Enabled for AAC-HE"));
        handle->userConfig.aac.i32InputVolLevel = -23;
        if (BAPE_Decoder_P_HasMultichannelOutput(handle))
        {
            handle->userConfig.aac.i32OutputVolLevel = -31;   /* We have a multichannel output so configure for -31dB */
        }
        else
        {
            handle->userConfig.aac.i32OutputVolLevel = -23;   /* There is no multichannel output so configure for -23dB */
        }
        if (pSettings->drcReferenceLevel == 4*24)
        {
            handle->userConfig.aac.ui32LoudnessEquivalenceMode = 3; /* config stereo for EBU -23 on output ports */
        }
        else
        {
        handle->userConfig.aac.ui32LoudnessEquivalenceMode = 1; /* config stereo for EBU -23 */
        }
        break;
    default:
        BDBG_MSG(("Loudness Equivalence Disabled for AAC-HE"));
        handle->userConfig.aac.ui32LoudnessEquivalenceMode = 0; /* None */
        if (pSettings->drcMode == BAPE_DolbyPulseDrcMode_eLine)
        {
            handle->userConfig.aac.i32InputVolLevel = -31;
            handle->userConfig.aac.i32OutputVolLevel = -31;   /* Config for -20dB */
        }
        else
        {
            if (pSettings->drcReferenceLevel == 4*23)
            {
                handle->userConfig.aac.i32InputVolLevel = -23;
                handle->userConfig.aac.i32OutputVolLevel = -31;
                handle->userConfig.aac.ui32LoudnessEquivalenceMode = 3;
            }
            else if (pSettings->drcReferenceLevel == 4*24)
            {
                handle->userConfig.aac.i32InputVolLevel = -24;
                handle->userConfig.aac.i32OutputVolLevel = -31;
                handle->userConfig.aac.ui32LoudnessEquivalenceMode = 4;
            }
            else
            {
                handle->userConfig.aac.i32InputVolLevel = -20;
                handle->userConfig.aac.i32OutputVolLevel = -20;   /* Config for -20dB */
            }
        }
        break;
    }
    BDBG_MODULE_MSG(bape_loudness,("Legacy AAC decoder is configured for %s loudness mode%s",
         (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? "ATSC" :
          handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? "EBU" : "DISABLED"),
         (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? ", expected input level -24 expected output level -24" :
          handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? ", expected input level -23 expected output level -23" : "")));
    BDBG_MODULE_MSG(bape_loudness,("i32InputVolLevel %d i32OutputVolLevel %d ui32LoudnessEquivalenceMode %d",
         handle->userConfig.aac.i32InputVolLevel, handle->userConfig.aac.i32OutputVolLevel, handle->userConfig.aac.ui32LoudnessEquivalenceMode));

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.aac, sizeof(handle->userConfig.aac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}
#endif

static BERR_Code BAPE_Decoder_P_ApplyAacSettings(
    BAPE_DecoderHandle handle,
    const BAPE_AacSettings *pSettings
    )
{
#if BDSP_MS12_SUPPORT
    return BAPE_Decoder_P_ApplyMs12AacSettings(handle, pSettings);
#elif BDSP_MS10_SUPPORT
    return BAPE_Decoder_P_ApplyMs10AacSettings(handle, pSettings);
#else
    return BAPE_Decoder_P_ApplyLegacyAacSettings(handle, pSettings);
#endif
    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyMpegSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.mpeg, sizeof(handle->userConfig.mpeg));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    handle->userConfig.mpeg.ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    switch ( handle->settings.dualMonoMode )
    {
    case BAPE_DualMonoMode_eLeft:
        handle->userConfig.mpeg.ui32DualMonoMode = 0;
        break;
    case BAPE_DualMonoMode_eRight:
        handle->userConfig.mpeg.ui32DualMonoMode = 1;
        break;
    default:
    case BAPE_DualMonoMode_eStereo:
        handle->userConfig.mpeg.ui32DualMonoMode = 2;
        break;
    case BAPE_DualMonoMode_eMix:
        handle->userConfig.mpeg.ui32DualMonoMode = 3;
        break;
    }

    BAPE_Decoder_P_GetAFDecoderType(handle, &handle->userConfig.mpeg.eDecoderType);

    switch ( handle->deviceHandle->settings.loudnessMode )
    {
    case BAPE_LoudnessEquivalenceMode_eAtscA85:
        BDBG_MSG(("ATSC A/85 Loudness Equivalence Enabled for MPEG"));
        handle->userConfig.mpeg.i32InputVolLevel = -24;
        if ( handle->ddre )
        {
            handle->userConfig.mpeg.i32OutputVolLevel = -31;
        }
        else
        {
            handle->userConfig.mpeg.i32OutputVolLevel = -24;
        }
        break;
    case BAPE_LoudnessEquivalenceMode_eEbuR128:
        BDBG_MSG(("EBU-R128 Loudness Equivalence Enabled for MPEG"));
        handle->userConfig.mpeg.i32InputVolLevel = -23;
        if ( handle->ddre )
        {
            handle->userConfig.mpeg.i32OutputVolLevel = -31;
        }
        else
        {
            handle->userConfig.mpeg.i32OutputVolLevel = -23;
        }
        break;
    default:
        BDBG_MSG(("Loudness Equivalence Disabled for MPEG"));
        /* Use default config from FW */
        break;
    }
    if ( handle->ddre )
    {
        BDBG_MODULE_MSG(bape_loudness,("MPEG decoder is configured for %s loudness mode.",
             (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? "ATSC" :
              handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? "EBU" : "DISABLED")));
        BDBG_MODULE_MSG(bape_loudness,("DDRE is connected to this decoder, so MPEG output level will be -31 to DDRE and Loudness Equivalence will be handled by DDRE module"));
    }
    else
    {
        BDBG_MODULE_MSG(bape_loudness,("MPEG decoder is configured for %s loudness mode%s",
             (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? "ATSC" :
              handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? "EBU" : "DISABLED"),
             (handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eAtscA85 ? ", expected input level -24 expected output level -24" :
              handle->deviceHandle->settings.loudnessMode == BAPE_LoudnessEquivalenceMode_eEbuR128 ? ", expected input level -23 expected output level -23" : "")));
    }
    BDBG_MODULE_MSG(bape_loudness,("i32InputVolLevel %d i32OutputVolLevel %d", handle->userConfig.mpeg.i32InputVolLevel, handle->userConfig.mpeg.i32OutputVolLevel));

    if ( handle->hAncDataQueue )
    {
        BDSP_AF_P_sIO_BUFFER ioBuffer;
        BDSP_Queue_GetIoBuffer(handle->hAncDataQueue, &ioBuffer);
        handle->userConfig.mpeg.sAncDataCircBuff.eBufferType = ioBuffer.eBufferType;
        handle->userConfig.mpeg.sAncDataCircBuff.sCircBuffer = ioBuffer.sCircBuffer[0];
        handle->userConfig.mpeg.ui32AncDataParseEnable = handle->settings.ancillaryDataEnabled?1:0;
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.mpeg, sizeof(handle->userConfig.mpeg));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyWmaStdSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;

    /* WMA is set in two parts.  The first is in framesync to handle TS vs ASF */
    errCode = BDSP_AudioStage_GetDatasyncSettings(handle->hPrimaryStage, &handle->datasyncSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Setup datasync for ASF vs. TS */
    handle->datasyncSettings.uAlgoSpecConfigStruct.sWmaConfig.eWMAIpType =
        (handle->startSettings.codec == BAVC_AudioCompressionStd_eWmaStdTs)?
        BDSP_Audio_WMAIpType_eTS:BDSP_Audio_WMAIpType_eASF;

    errCode = BDSP_AudioStage_SetDatasyncSettings(handle->hPrimaryStage, &handle->datasyncSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Set remaining codec settings */
    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.wma, sizeof(handle->userConfig.wma));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    handle->userConfig.wma.ui32OutputMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.wma, sizeof(handle->userConfig.wma));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyWmaProSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;
    bool lfe;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.wmaPro, sizeof(handle->userConfig.wmaPro));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    switch ( handle->wmaProSettings.codecSettings.wmaPro.drcMode )
    {
    case BAPE_WmaProDrcMode_eHigh:
        handle->userConfig.wmaPro.sOutputCfg[0].ui32DRCEnable = 1;
        handle->userConfig.wmaPro.sOutputCfg[0].eDRCSetting = BDSP_Raaga_Audio_eDrcSetting_High;
        handle->userConfig.wmaPro.sOutputCfg[1].ui32DRCEnable = 1;
        handle->userConfig.wmaPro.sOutputCfg[1].eDRCSetting = BDSP_Raaga_Audio_eDrcSetting_High;
        break;
    case BAPE_WmaProDrcMode_eMedium:
        handle->userConfig.wmaPro.sOutputCfg[0].ui32DRCEnable = 1;
        handle->userConfig.wmaPro.sOutputCfg[0].eDRCSetting = BDSP_Raaga_Audio_eDrcSetting_Med;
        handle->userConfig.wmaPro.sOutputCfg[1].ui32DRCEnable = 1;
        handle->userConfig.wmaPro.sOutputCfg[1].eDRCSetting = BDSP_Raaga_Audio_eDrcSetting_Med;
        break;
    case BAPE_WmaProDrcMode_eLow:
        handle->userConfig.wmaPro.sOutputCfg[0].ui32DRCEnable = 1;
        handle->userConfig.wmaPro.sOutputCfg[0].eDRCSetting = BDSP_Raaga_Audio_eDrcSetting_Low;
        handle->userConfig.wmaPro.sOutputCfg[1].ui32DRCEnable = 1;
        handle->userConfig.wmaPro.sOutputCfg[1].eDRCSetting = BDSP_Raaga_Audio_eDrcSetting_Low;
        break;
    default:
        BDBG_WRN(("Unsupported WMA Pro DRC Mode %u", handle->wmaProSettings.codecSettings.wmaPro.drcMode));
        /* Fall through */
    case BAPE_WmaProDrcMode_eDisabled:
        handle->userConfig.wmaPro.sOutputCfg[0].ui32DRCEnable = 0;
        handle->userConfig.wmaPro.sOutputCfg[1].ui32DRCEnable = 0;
        break;
    }

    handle->userConfig.wmaPro.sOutputCfg[0].i32RmsAmplitudeRef = handle->wmaProSettings.codecSettings.wmaPro.rmsAmplitudeReference;
    handle->userConfig.wmaPro.sOutputCfg[1].i32RmsAmplitudeRef = handle->wmaProSettings.codecSettings.wmaPro.rmsAmplitudeReference;
    handle->userConfig.wmaPro.sOutputCfg[0].i32PeakAmplitudeRef = handle->wmaProSettings.codecSettings.wmaPro.peakAmplitudeReference;
    handle->userConfig.wmaPro.sOutputCfg[1].i32PeakAmplitudeRef = handle->wmaProSettings.codecSettings.wmaPro.peakAmplitudeReference;
    handle->userConfig.wmaPro.sOutputCfg[0].i32DesiredRms = handle->wmaProSettings.codecSettings.wmaPro.desiredRms;
    handle->userConfig.wmaPro.sOutputCfg[1].i32DesiredRms = handle->wmaProSettings.codecSettings.wmaPro.desiredRms;
    handle->userConfig.wmaPro.sOutputCfg[0].i32DesiredPeak = handle->wmaProSettings.codecSettings.wmaPro.desiredPeak;
    handle->userConfig.wmaPro.sOutputCfg[1].i32DesiredPeak = handle->wmaProSettings.codecSettings.wmaPro.desiredPeak;
    handle->userConfig.wmaPro.sOutputCfg[0].ui32Stereomode = handle->wmaProSettings.codecSettings.wmaPro.stereoMode;
    handle->userConfig.wmaPro.sOutputCfg[1].ui32Stereomode = handle->wmaProSettings.codecSettings.wmaPro.stereoMode;

    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }
    handle->userConfig.wmaPro.sOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.wmaPro.sOutputCfg[0].ui32OutLfe = lfe?1:0;
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.wmaPro.sOutputCfg[0].ui32OutputChannelMatrix);
    handle->userConfig.wmaPro.sOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
    handle->userConfig.wmaPro.sOutputCfg[1].ui32OutLfe = 0;
    handle->userConfig.wmaPro.ui32NumOutports = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;
    handle->userConfig.wmaPro.ui32UsageMode = (handle->simul)? 2 /* Simul */ : 0 /* Decode */;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.wmaPro, sizeof(handle->userConfig.wmaPro));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyDtsSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;
    bool lfe;

    errCode = BDSP_AudioStage_GetDatasyncSettings(handle->hPrimaryStage, &handle->datasyncSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Set endian of compressed input data */
    handle->datasyncSettings.uAlgoSpecConfigStruct.sDtsConfig.eDtsEndianType =
        (handle->dtsSettings.codecSettings.dts.littleEndian)?
        BDSP_Audio_DtsEndianType_eLITTLE_ENDIAN:BDSP_Audio_DtsEndianType_eBIG_ENDIAN;

    errCode = BDSP_AudioStage_SetDatasyncSettings(handle->hPrimaryStage, &handle->datasyncSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.dts, sizeof(handle->userConfig.dts));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Setup global parameters */
    BAPE_Decoder_P_GetAFDecoderType(handle, &handle->userConfig.dts.eDecoderType);

    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }
    handle->userConfig.dts.sUserOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.dts.sUserOutputCfg[0].ui32OutLfe = lfe?1:0;
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.dts.sUserOutputCfg[0].ui32OutputChannelMatrix);
    handle->userConfig.dts.sUserOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
    handle->userConfig.dts.sUserOutputCfg[1].ui32OutLfe = 0;
    handle->userConfig.dts.i32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;

    switch ( handle->settings.dualMonoMode )
    {
    default:
    case BAPE_DualMonoMode_eStereo:
        handle->userConfig.dts.sUserOutputCfg[0].ui32DualMode = 2;
        break;
    case BAPE_DualMonoMode_eLeft:
        handle->userConfig.dts.sUserOutputCfg[0].ui32DualMode = 0;
        break;
    case BAPE_DualMonoMode_eRight:
        handle->userConfig.dts.sUserOutputCfg[0].ui32DualMode = 1;
        break;
    case BAPE_DualMonoMode_eMix:
        handle->userConfig.dts.sUserOutputCfg[0].ui32DualMode = 3;
        break;
    }
    handle->userConfig.dts.sUserOutputCfg[1].ui32DualMode = handle->userConfig.dts.sUserOutputCfg[0].ui32DualMode;

    handle->userConfig.dts.ui32MixLFE2Primary = handle->dtsSettings.codecSettings.dts.mixLfeToPrimary ? 1 : 0;
    handle->userConfig.dts.sUserOutputCfg[0].i32UserDRCFlag = handle->dtsSettings.codecSettings.dts.drcMode == BAPE_DtsDrcMode_eDisabled ? 0 : 1;
    handle->userConfig.dts.sUserOutputCfg[1].i32UserDRCFlag = handle->userConfig.dts.sUserOutputCfg[0].i32UserDRCFlag;

    handle->userConfig.dts.sUserOutputCfg[0].i32DynScaleHigh = BAPE_P_FloatToQ131(handle->dtsSettings.codecSettings.dts.drcScaleHi, 100);
    handle->userConfig.dts.sUserOutputCfg[0].i32DynScaleLow = BAPE_P_FloatToQ131(handle->dtsSettings.codecSettings.dts.drcScaleLow, 100);
    handle->userConfig.dts.sUserOutputCfg[1].i32DynScaleHigh = handle->userConfig.dts.sUserOutputCfg[0].i32DynScaleHigh;
    handle->userConfig.dts.sUserOutputCfg[1].i32DynScaleLow = handle->userConfig.dts.sUserOutputCfg[0].i32DynScaleLow;

    handle->userConfig.dts.sUserOutputCfg[0].ui32StereoMode = (uint32_t)handle->dtsSettings.codecSettings.dts.stereoMode;
    handle->userConfig.dts.sUserOutputCfg[1].ui32StereoMode = handle->userConfig.dts.sUserOutputCfg[0].ui32StereoMode;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.dts, sizeof(handle->userConfig.dts));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyDtsExpressSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;
    bool lfe;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.dtsExpress, sizeof(handle->userConfig.dtsExpress));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }
    handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32OutLfe = lfe?1:0;
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32OutputChannelMatrix);
    handle->userConfig.dtsExpress.sUserOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
    handle->userConfig.dtsExpress.sUserOutputCfg[1].ui32OutLfe = 0;
    handle->userConfig.dtsExpress.i32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;

    switch ( handle->settings.dualMonoMode )
    {
    default:
    case BAPE_DualMonoMode_eStereo:
        handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32DualMode = 2;
        break;
    case BAPE_DualMonoMode_eLeft:
        handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32DualMode = 0;
        break;
    case BAPE_DualMonoMode_eRight:
        handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32DualMode = 1;
        break;
    case BAPE_DualMonoMode_eMix:
        handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32DualMode = 3;
        break;
    }
    handle->userConfig.dtsExpress.sUserOutputCfg[1].ui32DualMode = handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32DualMode;

    switch ( handle->dtsExpressSettings.codecSettings.dts.stereoMode )
    {
    default:
        handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32StereoMode = 0;
        break;
    case BAPE_DtsStereoMode_eLtRt:
        handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32StereoMode = 1;
        break;
    }
    handle->userConfig.dtsExpress.sUserOutputCfg[1].ui32StereoMode = handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32StereoMode;

    handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32EnableDRC = (uint32_t)handle->dtsExpressSettings.codecSettings.dts.drcMode;
    handle->userConfig.dtsExpress.sUserOutputCfg[1].ui32EnableDRC = handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32EnableDRC;

    handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32MixLFE2Primary = handle->dtsExpressSettings.codecSettings.dts.mixLfeToPrimary ? 1 : 0;
    handle->userConfig.dtsExpress.sUserOutputCfg[1].ui32MixLFE2Primary = handle->userConfig.dtsExpress.sUserOutputCfg[0].ui32MixLFE2Primary;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.dtsExpress, sizeof(handle->userConfig.dtsExpress));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyAmrSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.amr, sizeof(handle->userConfig.amr));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* AMR is a mono codec.  We can ignore the requested channel mode and force 1_0 */
    channelMode = BAPE_ChannelMode_e1_0;
    handle->userConfig.amr.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    BAPE_DSP_P_GetChannelMatrix(channelMode, false, handle->userConfig.amr.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    handle->userConfig.amr.sUsrOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.amr.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMonoOutput(handle))?2:1;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.amr, sizeof(handle->userConfig.amr));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyAmrWbSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.amrWb, sizeof(handle->userConfig.amrWb));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* AMR is a mono codec.  We can ignore the requested channel mode and force 1_0 */
    channelMode = BAPE_ChannelMode_e1_0;
    handle->userConfig.amrWb.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    BAPE_DSP_P_GetChannelMatrix(channelMode, false, handle->userConfig.amrWb.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    handle->userConfig.amrWb.sUsrOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.amrWb.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.amrWb, sizeof(handle->userConfig.amrWb));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyDraSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;
    bool lfe;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.dra, sizeof(handle->userConfig.dra));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }
    handle->userConfig.dra.sUserOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.dra.sUserOutputCfg[0].ui32OutLfe = lfe?1:0;
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.dra.sUserOutputCfg[0].ui32OutputChannelMatrix);
    handle->userConfig.dra.sUserOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
    handle->userConfig.dra.sUserOutputCfg[1].ui32OutLfe = 0;
    handle->userConfig.dra.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.dra, sizeof(handle->userConfig.dra));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyCookSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.cook, sizeof(handle->userConfig.cook));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Cook is a stereo codec.  No LFE support. */
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    handle->userConfig.cook.sUserOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    BAPE_DSP_P_GetChannelMatrix(channelMode, false, handle->userConfig.cook.sUserOutputCfg[0].ui32OutputChannelMatrix);
    handle->userConfig.cook.sUserOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.cook.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.cook, sizeof(handle->userConfig.cook));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyPcmWavSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;
    bool lfe;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.pcmWav, sizeof(handle->userConfig.pcmWav));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Setup global parameters */
    BAPE_Decoder_P_GetAFDecoderType(handle, &handle->userConfig.pcmWav.eDecoderType);

    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }

    handle->userConfig.pcmWav.sUserOutputCfg[0].i32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.pcmWav.sUserOutputCfg[0].ui32OutputChannelMatrix);

    handle->userConfig.pcmWav.ui32NumOutputPorts = 0;
    if ( BAPE_Decoder_P_HasStereoOutput(handle) )
    {
        handle->userConfig.pcmWav.ui32NumOutputPorts++;
    }
    if ( BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        handle->userConfig.pcmWav.ui32NumOutputPorts++;
    }
    if ( BAPE_Decoder_P_HasMonoOutput(handle) )
    {
        unsigned monoPathId;

        /* Sanity check - this will be ensured in BAPE_Decoder_P_ValidateSettings */
        BDBG_ASSERT(handle->userConfig.pcmWav.ui32NumOutputPorts < 2);

        handle->userConfig.pcmWav.ui32NumOutputPorts++;
        monoPathId = ( handle->userConfig.pcmWav.ui32NumOutputPorts == 1 ) ? 0 : 1;
        handle->userConfig.pcmWav.sUserOutputCfg[monoPathId].i32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e1_0);
        BAPE_DSP_P_GetMonoChannelMatrix(handle->userConfig.pcmWav.sUserOutputCfg[monoPathId].ui32OutputChannelMatrix);
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.pcmWav, sizeof(handle->userConfig.pcmWav));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }


    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyG711G726Settings(BAPE_DecoderHandle handle)
{
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.g711g726, sizeof(handle->userConfig.g711g726));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->userConfig.g711g726.ui32NumOutPorts = 0;
    if ( BAPE_Decoder_P_HasStereoOutput(handle) )
    {
        handle->userConfig.g711g726.ui32NumOutPorts++;
        handle->userConfig.g711g726.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
        BAPE_DSP_P_GetChannelMatrix(BAPE_ChannelMode_e2_0, false, handle->userConfig.g711g726.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    }
    if ( BAPE_Decoder_P_HasMonoOutput(handle) )
    {
        unsigned monoPathId;

        /* Sanity check - this will be ensured in BAPE_Decoder_P_ValidateSettings */
        BDBG_ASSERT(handle->userConfig.g711g726.ui32NumOutPorts < 2);

        handle->userConfig.g711g726.ui32NumOutPorts++;
        monoPathId = ( handle->userConfig.g711g726.ui32NumOutPorts == 1 ) ? 0 : 1;
        handle->userConfig.g711g726.sUsrOutputCfg[monoPathId].ui32ApplyGain = 0;
        handle->userConfig.g711g726.sUsrOutputCfg[monoPathId].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e1_0);
        BAPE_DSP_P_GetMonoChannelMatrix(handle->userConfig.g711g726.sUsrOutputCfg[monoPathId].ui32OutputChannelMatrix);
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.g711g726, sizeof(handle->userConfig.g711g726));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyG723_1Settings(BAPE_DecoderHandle handle)
{
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.g723, sizeof(handle->userConfig.g723));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->userConfig.g723.ui32NumOutPorts = 0;
    if ( BAPE_Decoder_P_HasStereoOutput(handle) )
    {
        handle->userConfig.g723.ui32NumOutPorts++;
        handle->userConfig.g723.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
        BAPE_DSP_P_GetChannelMatrix(BAPE_ChannelMode_e2_0, false, handle->userConfig.g723.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    }
    if ( BAPE_Decoder_P_HasMonoOutput(handle) )
    {
        unsigned monoPathId;

        /* Sanity check - this will be ensured in BAPE_Decoder_P_ValidateSettings */
        BDBG_ASSERT(handle->userConfig.g723.ui32NumOutPorts < 2);

        handle->userConfig.g723.ui32NumOutPorts++;
        monoPathId = ( handle->userConfig.g723.ui32NumOutPorts == 1 ) ? 0 : 1;
        handle->userConfig.g723.sUsrOutputCfg[monoPathId].ui32ScaleOp = 0;
        handle->userConfig.g723.sUsrOutputCfg[monoPathId].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e1_0);
        BAPE_DSP_P_GetMonoChannelMatrix(handle->userConfig.g723.sUsrOutputCfg[monoPathId].ui32OutputChannelMatrix);
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.g723, sizeof(handle->userConfig.g723));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyG729Settings(BAPE_DecoderHandle handle)
{
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.g729, sizeof(handle->userConfig.g729));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->userConfig.g729.ui32NumOutPorts = 0;
    if ( BAPE_Decoder_P_HasStereoOutput(handle) )
    {
        handle->userConfig.g729.ui32NumOutPorts++;
        handle->userConfig.g729.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
        BAPE_DSP_P_GetChannelMatrix(BAPE_ChannelMode_e2_0, false, handle->userConfig.g729.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    }
    if ( BAPE_Decoder_P_HasMonoOutput(handle) )
    {
        unsigned monoPathId;

        /* Sanity check - this will be ensured in BAPE_Decoder_P_ValidateSettings */
        BDBG_ASSERT(handle->userConfig.g729.ui32NumOutPorts < 2);

        handle->userConfig.g729.ui32NumOutPorts++;
        monoPathId = ( handle->userConfig.g729.ui32NumOutPorts == 1 ) ? 0 : 1;
        handle->userConfig.g729.sUsrOutputCfg[monoPathId].ui32ScaleOp = 0;
        handle->userConfig.g729.sUsrOutputCfg[monoPathId].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e1_0);
        BAPE_DSP_P_GetMonoChannelMatrix(handle->userConfig.g729.sUsrOutputCfg[monoPathId].ui32OutputChannelMatrix);
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.g729, sizeof(handle->userConfig.g729));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyAdpcmSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.adpcm, sizeof(handle->userConfig.adpcm));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* ADPCM is a stereo codec.  No LFE support. */
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    handle->userConfig.adpcm.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    BAPE_DSP_P_GetChannelMatrix(channelMode, false, handle->userConfig.adpcm.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    handle->userConfig.adpcm.sUsrOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.adpcm.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;
    handle->userConfig.adpcm.sUsrOutputCfg[0].ui32ApplyGain = handle->adpcmSettings.codecSettings.adpcm.gain.enabled ? 1 : 0;
    handle->userConfig.adpcm.sUsrOutputCfg[0].i32GainFactor = handle->adpcmSettings.codecSettings.adpcm.gain.factor;
    handle->userConfig.adpcm.sUsrOutputCfg[1].ui32ApplyGain = handle->userConfig.adpcm.sUsrOutputCfg[0].ui32ApplyGain;
    handle->userConfig.adpcm.sUsrOutputCfg[1].i32GainFactor = handle->userConfig.adpcm.sUsrOutputCfg[0].i32GainFactor;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.adpcm, sizeof(handle->userConfig.adpcm));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyDvdLpcmSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;
    bool lfe;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.lpcm, sizeof(handle->userConfig.lpcm));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }
    handle->userConfig.lpcm.sOutputConfig[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.lpcm.sOutputConfig[0].ui32LfeOnFlag = lfe?1:0;
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.lpcm.sOutputConfig[0].ui32OutputChannelMatrix);
    handle->userConfig.lpcm.sOutputConfig[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
    handle->userConfig.lpcm.sOutputConfig[1].ui32LfeOnFlag = 0;
    handle->userConfig.lpcm.ui32NumOutputPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.lpcm, sizeof(handle->userConfig.lpcm));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyFlacSettings(BAPE_DecoderHandle handle)
{
    BAPE_ChannelMode channelMode;
    BERR_Code errCode;
    bool lfe;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.flac, sizeof(handle->userConfig.flac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }
    handle->userConfig.flac.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    handle->userConfig.flac.sUsrOutputCfg[0].ui32OutLfe = lfe?1:0;
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.flac.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    handle->userConfig.flac.sUsrOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
    handle->userConfig.flac.sUsrOutputCfg[1].ui32OutLfe = 0;
    handle->userConfig.flac.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.flac, sizeof(handle->userConfig.flac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


static BERR_Code BAPE_Decoder_P_ApplyIlbcSettings(BAPE_DecoderHandle handle)
{
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.ilbc, sizeof(handle->userConfig.ilbc));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->userConfig.ilbc.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMonoOutput(handle))?2:1;

    /* iLBC is a mono codec.  We can ignore the requested channel mode and force 1_0 */
    handle->userConfig.ilbc.sUsrOutputCfg[0].ui32ScaleOp = handle->ilbcSettings.codecSettings.ilbc.gain.enabled ? 1 : 0;
    if ( handle->ilbcSettings.codecSettings.ilbc.gain.factor >= 1200 )
    {
        handle->userConfig.ilbc.sUsrOutputCfg[0].ui32ScaleIdx = 3;
    }
    else if ( handle->ilbcSettings.codecSettings.ilbc.gain.factor >= 900 )
    {
        handle->userConfig.ilbc.sUsrOutputCfg[0].ui32ScaleIdx = 2;
    }
    else if ( handle->ilbcSettings.codecSettings.ilbc.gain.factor >= 600 )
    {
        handle->userConfig.ilbc.sUsrOutputCfg[0].ui32ScaleIdx = 1;
    }
    else
    {
        handle->userConfig.ilbc.sUsrOutputCfg[0].ui32ScaleIdx = 0;
    }
    handle->userConfig.ilbc.sUsrOutputCfg[0].mode = handle->ilbcSettings.codecSettings.ilbc.frameLength;
    handle->userConfig.ilbc.sUsrOutputCfg[0].plc = handle->ilbcSettings.codecSettings.ilbc.packetLoss;

    if ( handle->userConfig.ilbc.ui32NumOutPorts == 2 )
    {
        BKNI_Memcpy(&handle->userConfig.ilbc.sUsrOutputCfg[1], &handle->userConfig.ilbc.sUsrOutputCfg[0], sizeof(handle->userConfig.ilbc.sUsrOutputCfg[1]));
    }

    if ( BAPE_Decoder_P_HasStereoOutput(handle) )
    {
        handle->userConfig.ilbc.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
        BAPE_DSP_P_GetChannelMatrix(BAPE_ChannelMode_e2_0, false, handle->userConfig.ilbc.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    }
    if ( BAPE_Decoder_P_HasMonoOutput(handle) )
    {
        unsigned monoPathId;

        monoPathId = ( handle->userConfig.ilbc.ui32NumOutPorts == 1 ) ? 0 : 1;
        handle->userConfig.ilbc.sUsrOutputCfg[monoPathId].ui32ScaleOp = 0;
        handle->userConfig.ilbc.sUsrOutputCfg[monoPathId].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e1_0);
        BAPE_DSP_P_GetMonoChannelMatrix(handle->userConfig.ilbc.sUsrOutputCfg[monoPathId].ui32OutputChannelMatrix);
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.ilbc, sizeof(handle->userConfig.ilbc));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyIsacSettings(BAPE_DecoderHandle handle)
{
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.isac, sizeof(handle->userConfig.isac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->userConfig.isac.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMonoOutput(handle))?2:1;

    /* iSAC is a mono codec.  We can ignore the requested channel mode and force 1_0 */
    handle->userConfig.isac.sUsrOutputCfg[0].ui32ScaleOp = handle->isacSettings.codecSettings.isac.gain.enabled ? 1 : 0;
    if ( handle->isacSettings.codecSettings.isac.gain.factor >= 1200 )
    {
        handle->userConfig.isac.sUsrOutputCfg[0].ui32ScaleIdx = 3;
    }
    else if ( handle->isacSettings.codecSettings.isac.gain.factor >= 900 )
    {
        handle->userConfig.isac.sUsrOutputCfg[0].ui32ScaleIdx = 2;
    }
    else if ( handle->isacSettings.codecSettings.isac.gain.factor >= 600 )
    {
        handle->userConfig.isac.sUsrOutputCfg[0].ui32ScaleIdx = 1;
    }
    else
    {
        handle->userConfig.isac.sUsrOutputCfg[0].ui32ScaleIdx = 0;
    }
    handle->userConfig.isac.sUsrOutputCfg[0].ui32BandMode = (handle->isacSettings.codecSettings.isac.bandMode == BAPE_IsacBandMode_eNarrow) ? 1 : 0;
    handle->userConfig.isac.sUsrOutputCfg[0].plc = handle->isacSettings.codecSettings.isac.packetLoss;

    if ( handle->userConfig.isac.ui32NumOutPorts == 2 )
    {
        BKNI_Memcpy(&handle->userConfig.isac.sUsrOutputCfg[1], &handle->userConfig.isac.sUsrOutputCfg[0], sizeof(handle->userConfig.isac.sUsrOutputCfg[1]));
    }

    if ( BAPE_Decoder_P_HasStereoOutput(handle) )
    {
        handle->userConfig.isac.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
        BAPE_DSP_P_GetChannelMatrix(BAPE_ChannelMode_e2_0, false, handle->userConfig.isac.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    }
    if ( BAPE_Decoder_P_HasMonoOutput(handle) )
    {
        unsigned monoPathId;

        monoPathId = ( handle->userConfig.isac.ui32NumOutPorts == 1 ) ? 0 : 1;
        handle->userConfig.isac.sUsrOutputCfg[monoPathId].ui32ScaleOp = 0;
        handle->userConfig.isac.sUsrOutputCfg[monoPathId].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e1_0);
        BAPE_DSP_P_GetMonoChannelMatrix(handle->userConfig.isac.sUsrOutputCfg[monoPathId].ui32OutputChannelMatrix);
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.isac, sizeof(handle->userConfig.isac));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyOpusSettings(BAPE_DecoderHandle handle)
{
    BERR_Code errCode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.opus, sizeof(handle->userConfig.opus));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->userConfig.opus.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMonoOutput(handle))?2:1;

    if ( handle->userConfig.opus.ui32NumOutPorts == 2 )
    {
        BKNI_Memcpy(&handle->userConfig.opus.sUsrOutputCfg[1], &handle->userConfig.opus.sUsrOutputCfg[0], sizeof(handle->userConfig.opus.sUsrOutputCfg[1]));
    }

    if ( BAPE_Decoder_P_HasStereoOutput(handle) )
    {
        handle->userConfig.opus.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
        BAPE_DSP_P_GetChannelMatrix(BAPE_ChannelMode_e2_0, false, handle->userConfig.opus.sUsrOutputCfg[0].ui32OutputChannelMatrix);
    }
    if ( BAPE_Decoder_P_HasMonoOutput(handle) )
    {
        unsigned monoPathId;

        monoPathId = ( handle->userConfig.opus.ui32NumOutPorts == 1 ) ? 0 : 1;
        handle->userConfig.opus.sUsrOutputCfg[monoPathId].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e1_0);
        BAPE_DSP_P_GetMonoChannelMatrix(handle->userConfig.opus.sUsrOutputCfg[monoPathId].ui32OutputChannelMatrix);
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.opus, sizeof(handle->userConfig.opus));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyAlsSettings(BAPE_DecoderHandle handle)
{
    BERR_Code errCode;
    bool lfe;
    BAPE_ChannelMode channelMode;

    errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &handle->userConfig.als, sizeof(handle->userConfig.als));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Determine output mode */
    lfe = handle->settings.outputLfe;
    channelMode = BAPE_Decoder_P_GetChannelMode(handle);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }

    handle->userConfig.als.ui32NumOutPorts = (BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle))?2:1;

    if ( BAPE_Decoder_P_HasStereoOutput(handle) && BAPE_Decoder_P_HasMultichannelOutput(handle) )
    {
        handle->userConfig.als.ui32NumOutPorts = 2;
        handle->userConfig.als.sUsrOutputCfg[1].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(BAPE_ChannelMode_e2_0);
    }
    else
    {
        handle->userConfig.als.ui32NumOutPorts = 1;
    }

    handle->userConfig.als.sUsrOutputCfg[0].ui32OutMode = BAPE_DSP_P_ChannelModeToDsp(channelMode);
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, handle->userConfig.als.sUsrOutputCfg[0].ui32OutputChannelMatrix);

    switch ( handle->alsSettings.codecSettings.als.stereoMode )
    {
    default:
    case BAPE_AlsStereoMode_eArib:
        handle->userConfig.als.i32DownmixType = 1;
        break;
    case BAPE_AlsStereoMode_eLtRt:
        handle->userConfig.als.i32DownmixType = 2;
        break;
    }

    if ( handle->alsSettings.codecSettings.als.aribMatrixMixdownIndex > 3 || handle->alsSettings.codecSettings.als.aribMatrixMixdownIndex == 0 )
    {
        BDBG_WRN(("Invalid aribMatrixMixdownIndex defaulting to 1"));
        handle->userConfig.als.i32AribMatrixMixdownIndex = 1;
    }
    else
    {
        handle->userConfig.als.i32AribMatrixMixdownIndex = handle->alsSettings.codecSettings.als.aribMatrixMixdownIndex;
    }

    errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &handle->userConfig.als, sizeof(handle->userConfig.als));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_P_ApplyCodecSettings(BAPE_DecoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        if ( !handle->passthrough )
        {
            switch ( handle->startSettings.codec )
            {
            case BAVC_AudioCompressionStd_eAc3:
                return BAPE_Decoder_P_ApplyAc3Settings(handle, &handle->ac3Settings.codecSettings.ac3);
            case BAVC_AudioCompressionStd_eAc3Plus:
                return BAPE_Decoder_P_ApplyAc3Settings(handle, &handle->ac3PlusSettings.codecSettings.ac3Plus);
            case BAVC_AudioCompressionStd_eAc4:
                return BAPE_Decoder_P_ApplyAc4Settings(handle, &handle->ac4Settings.codecSettings.ac4);
            case BAVC_AudioCompressionStd_eAacAdts:
            case BAVC_AudioCompressionStd_eAacLoas:
                return BAPE_Decoder_P_ApplyAacSettings(handle, &handle->aacSettings.codecSettings.aac);
            case BAVC_AudioCompressionStd_eAacPlusAdts:
            case BAVC_AudioCompressionStd_eAacPlusLoas:
                return BAPE_Decoder_P_ApplyAacSettings(handle, &handle->aacPlusSettings.codecSettings.aacPlus);
            case BAVC_AudioCompressionStd_eMpegL1:
            case BAVC_AudioCompressionStd_eMpegL2:
            case BAVC_AudioCompressionStd_eMpegL3:
                return BAPE_Decoder_P_ApplyMpegSettings(handle);
            case BAVC_AudioCompressionStd_eWmaStd:
            case BAVC_AudioCompressionStd_eWmaStdTs:
                return BAPE_Decoder_P_ApplyWmaStdSettings(handle);
            case BAVC_AudioCompressionStd_eWmaPro:
                return BAPE_Decoder_P_ApplyWmaProSettings(handle);
            case BAVC_AudioCompressionStd_eDts:
            case BAVC_AudioCompressionStd_eDtsHd:
            case BAVC_AudioCompressionStd_eDtsCd:
                return BAPE_Decoder_P_ApplyDtsSettings(handle);
            case BAVC_AudioCompressionStd_eDtsExpress:
                return BAPE_Decoder_P_ApplyDtsExpressSettings(handle);
            case BAVC_AudioCompressionStd_eAmr:
                return BAPE_Decoder_P_ApplyAmrSettings(handle);
            case BAVC_AudioCompressionStd_eAmrWb:
                return BAPE_Decoder_P_ApplyAmrWbSettings(handle);
            case BAVC_AudioCompressionStd_eDra:
                return BAPE_Decoder_P_ApplyDraSettings(handle);
            case BAVC_AudioCompressionStd_eCook:
                return BAPE_Decoder_P_ApplyCookSettings(handle);
            case BAVC_AudioCompressionStd_eAdpcm:
                return BAPE_Decoder_P_ApplyAdpcmSettings(handle);
            case BAVC_AudioCompressionStd_ePcmWav:
                return BAPE_Decoder_P_ApplyPcmWavSettings(handle);
            case BAVC_AudioCompressionStd_eG711:
            case BAVC_AudioCompressionStd_eG726:
                return BAPE_Decoder_P_ApplyG711G726Settings(handle);
            case BAVC_AudioCompressionStd_eG723_1:
                return BAPE_Decoder_P_ApplyG723_1Settings(handle);
            case BAVC_AudioCompressionStd_eG729:
                return BAPE_Decoder_P_ApplyG729Settings(handle);
            case BAVC_AudioCompressionStd_eLpcmDvd:
            case BAVC_AudioCompressionStd_eLpcmBd:
            case BAVC_AudioCompressionStd_eLpcm1394:
                return BAPE_Decoder_P_ApplyDvdLpcmSettings(handle);
            case BAVC_AudioCompressionStd_eFlac:
                return BAPE_Decoder_P_ApplyFlacSettings(handle);
            case BAVC_AudioCompressionStd_eIlbc:
                return BAPE_Decoder_P_ApplyIlbcSettings(handle);
            case BAVC_AudioCompressionStd_eIsac:
                return BAPE_Decoder_P_ApplyIsacSettings(handle);
            case BAVC_AudioCompressionStd_eOpus:
                return BAPE_Decoder_P_ApplyOpusSettings(handle);
            case BAVC_AudioCompressionStd_eAls:
                return BAPE_Decoder_P_ApplyAlsSettings(handle);
            default:
                BDBG_MSG(("DSP Codec %u (%s) does not have settings", handle->startSettings.codec, BAPE_P_GetCodecName(handle->startSettings.codec)));
                break;
            }
        }
    }

    return BERR_SUCCESS;
}
