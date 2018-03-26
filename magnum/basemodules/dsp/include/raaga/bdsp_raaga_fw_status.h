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
 ******************************************************************************/

#ifndef BRAAGA_FWSTATUS_PRIV_H__

#define BRAAGA_FWSTATUS_PRIV_H__

#define BDSP_FWSTATUS_STATUS_INVALID    0

#ifdef BDSP_DOLBY_AACHE_SUPPORT
#define     BDSP_Raaga_Audio_AacheStreamInfo    BDSP_Raaga_Audio_DolbyAacheStreamInfo
#elif BDSP_MS10_SUPPORT
#define     BDSP_Raaga_Audio_AacheStreamInfo    BDSP_Raaga_Audio_DolbyPulseStreamInfo
#else
#define     BDSP_Raaga_Audio_AacheStreamInfo    BDSP_Raaga_Audio_AacStreamInfo
#endif

/* BDSP_AudioType */
#define BDSP_Raaga_MpegStatus               BDSP_Raaga_Audio_MpegStreamInfo

#ifdef BDSP_DOLBY_AACHE_SUPPORT
    #define BDSP_Raaga_AacStatus            BDSP_Raaga_Audio_DolbyAacheStreamInfo
    #define BDSP_Raaga_AacAdtsStatus        BDSP_Raaga_Audio_DolbyAacheStreamInfo
    #define BDSP_Raaga_AacLoasStatus        BDSP_Raaga_Audio_DolbyAacheStreamInfo
    #define BDSP_Raaga_AacSbrAdtsStatus     BDSP_Raaga_Audio_DolbyAacheStreamInfo
    #define BDSP_Raaga_AacSbrStatus         BDSP_Raaga_Audio_DolbyAacheStreamInfo
    #define BDSP_Raaga_AacSbrLoasStatus     BDSP_Raaga_Audio_DolbyAacheStreamInfo
#elif BDSP_MS10_SUPPORT
    #define BDSP_Raaga_AacStatus            BDSP_Raaga_Audio_DolbyPulseStreamInfo
    #define BDSP_Raaga_AacAdtsStatus        BDSP_Raaga_Audio_DolbyPulseStreamInfo
    #define BDSP_Raaga_AacLoasStatus        BDSP_Raaga_Audio_DolbyPulseStreamInfo
    #define BDSP_Raaga_AacSbrAdtsStatus     BDSP_Raaga_Audio_DolbyPulseStreamInfo
    #define BDSP_Raaga_AacSbrStatus         BDSP_Raaga_Audio_DolbyPulseStreamInfo
    #define BDSP_Raaga_AacSbrLoasStatus     BDSP_Raaga_Audio_DolbyPulseStreamInfo
#else
    #define BDSP_Raaga_AacStatus            BDSP_Raaga_Audio_AacheStreamInfo
    #define BDSP_Raaga_AacAdtsStatus        BDSP_Raaga_Audio_AacheStreamInfo
    #define BDSP_Raaga_AacLoasStatus        BDSP_Raaga_Audio_AacheStreamInfo
    #define BDSP_Raaga_AacSbrAdtsStatus     BDSP_Raaga_Audio_AacheStreamInfo
    #define BDSP_Raaga_AacSbrStatus         BDSP_Raaga_Audio_AacheStreamInfo
    #define BDSP_Raaga_AacSbrLoasStatus     BDSP_Raaga_Audio_AacheStreamInfo
#endif

#define BDSP_Raaga_Ac3Status                BDSP_Raaga_Audio_MultiStreamDDPStreamInfo
#define BDSP_Raaga_Ac3PlusStatus            BDSP_Raaga_Audio_MultiStreamDDPStreamInfo
#define BDSP_Raaga_LpcmBdStatus             BDSP_Raaga_Audio_LpcmStreamInfo
#define BDSP_Raaga_LpcmHdDvdStatus          BDSP_Raaga_Audio_LpcmStreamInfo
#define BDSP_Raaga_DtshdStatus              BDSP_Raaga_Audio_DtsHdStreamInfo
#define BDSP_Raaga_LpcmDvdStatus            BDSP_Raaga_Audio_LpcmStreamInfo
#define BDSP_Raaga_WmaStdStatus             BDSP_Raaga_Audio_WmaStreamInfo
#define BDSP_Raaga_MlpStatus                BDSP_Raaga_Audio_MlpStreamInfo
#define BDSP_Raaga_PcmStatus                BDSP_Raaga_Audio_DecodeStreamInfo
#define BDSP_Raaga_DtsLbrStatus             BDSP_Raaga_Audio_DtslbrStreamInfo
#define BDSP_Raaga_Ddp7_1Status             BDSP_Raaga_Audio_DecodeStreamInfo
#define BDSP_Raaga_MpegMcStatus             BDSP_Raaga_Audio_DecodeStreamInfo
#define BDSP_Raaga_WmaProStatus             BDSP_Raaga_Audio_WmaProStreamInfo
#define BDSP_Raaga_LpcmDvdAStatus           BDSP_Raaga_Audio_DecodeStreamInfo
#define BDSP_Raaga_DtsBroadcastStatus       BDSP_Raaga_Audio_DtsHdStreamInfo
#define BDSP_Raaga_PcmWavStatus             BDSP_Raaga_Audio_PcmWavStreamInfo
#define BDSP_Raaga_AmrStatus                BDSP_Raaga_Audio_AmrStreamInfo
#define BDSP_Raaga_DraStatus                BDSP_Raaga_Audio_DraStreamInfo
#define BDSP_Raaga_RealAudioLbrStatus       BDSP_Raaga_Audio_RalbrStreamInfo
#define BDSP_Raaga_AdpcmStatus              BDSP_Raaga_Audio_AdpcmStreamInfo
#define BDSP_Raaga_G711G726Status           BDSP_Raaga_Audio_G726StreamInfo
#define BDSP_Raaga_G729Status               BDSP_Raaga_Audio_G729DecStreamInfo
#define BDSP_Raaga_VorbisStatus             BDSP_Raaga_Audio_VorbisDecStreamInfo
#define BDSP_Raaga_FlacStatus               BDSP_Raaga_Audio_FlacDecStreamInfo
#define BDSP_Raaga_MacStatus                BDSP_Raaga_Audio_MacDecStreamInfo
#define BDSP_Raaga_AmrWbStatus              BDSP_Raaga_Audio_AmrWbStreamInfo
#define BDSP_Raaga_iLBCStatus               BDSP_Raaga_Audio_iLBCStreamInfo
#define BDSP_Raaga_iSACStatus               BDSP_Raaga_Audio_iSACStreamInfo
#define BDSP_Raaga_UdcStatus                BDSP_Raaga_Audio_UdcStreamInfo
#define BDSP_Raaga_OpusDecStatus            BDSP_Raaga_Audio_OpusDecStreamInfo
#define BDSP_Raaga_ALSDecStatus             BDSP_Raaga_Audio_ALSDecStreamInfo
#define BDSP_Raaga_AC4Status                BDSP_Raaga_Audio_AC4StreamInfo

/* BDSP_AudioProcessing */
#define BDSP_Raaga_DdbmStatus                   BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_DtsNeoStatus                 BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_AVLStatus                    BDSP_Raaga_Audio_AvlPPStatusInfo
#define BDSP_Raaga_DDConvertStatus              BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_PLllStatus                   BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_SrsXtStatus                  BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_XenStatus                    BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_BbeStatus                    BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_SrcStatus                    BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_CustomSurroundStatus         BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_CustomBassStatus             BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_CustomVoiceStatus            BDSP_Raaga_Audio_CustomVoiceStatusInfo
#define BDSP_Raaga_PeqStatus                    BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_Downmix0Status               BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_AudioDescriptorFadeStatus    BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_AudioDescriptorPanStatus     BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_PCMRouterStatus              BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_WMAPassThroughStatus         BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_DsolaStatus                  BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_SrsHdStatus                  BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_GenericPassThruStatus        BDSP_Raaga_Audio_PassthruStreamInfo
#define BDSP_Raaga_SrsTruVolumeStatus           BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_DolbyVolumeStatus            BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_Brcm3DSurroundStatus         BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_FWMixerStatus                BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_MonoDownmixStatus            BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_DdreStatus                   BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_Dv258Status                  BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_DpcmrStatus                  BDSP_Raaga_Audio_ProcessingStreamInfo
#define BDSP_Raaga_GenCdbItbStatus              BDSP_Raaga_Audio_GenCdbItbStreamInfo
#define BDSP_Raaga_OutputFormatterStatus        BDSP_Raaga_Audio_OutputFormatterStatusInfo
#define BDSP_Raaga_VocalPPStatus                BDSP_Raaga_Audio_VocalPPStatusInfo
#define BDSP_Raaga_FadeCtrlPPStatus             BDSP_Raaga_Audio_FadeCtrlPPStatusInfo
#define BDSP_Raaga_AmbisonicsPPStatus           BDSP_Raaga_Audio_AmbisonicsPPStatusInfo
#define BDSP_Raaga_MixerDapv2PPStatus           BDSP_Raaga_Audio_MixerDapv2StatusInfo
#define BDSP_Raaga_TsmCorrectionPPStatus        BDSP_Raaga_Audio_TsmCorrectionPPStatusInfo

/* BDSP_AudioEncode */
#define BDSP_Raaga_Mpeg1Layer3Status        BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_Mpeg1Layer2Status        BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_DTSEncodeStatus          BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_AacLcStatus              BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_AacHeStatus              BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_Ac3EncodeStatus          BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_SbcStatus                BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_G711G726EncodeStatus     BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_G729EncodeStatus         BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_G722EncodeStatus         BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_AmrEncodeStatus          BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_AmrwbEncodeStatus        BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_iLBCEncodeStatus         BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_iSACEncodeStatus         BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_LpcmEncodeStatus         BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_OpusEncodeStatus         BDSP_Raaga_Audio_EncodeStreamInfo
#define BDSP_Raaga_DDPEncodeStatus          BDSP_Raaga_Audio_EncodeStreamInfo

/* BDSP_VideoType */
#define BDSP_Raaga_RealVideo9Status BDSP_Raaga_Video_DecodeStreamInfo
#define BDSP_Raaga_VP6Status BDSP_Raaga_Video_DecodeStreamInfo


/*********************************************************************
Summary:
    This is the structure which defines the default stream information for audio decoders.
Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_DecodeStreamInfo
{
    /* This is the cumulative number of samples decoded by any decoder from the
        channel open time. Decoder should accumulate number of samples decoded for
        every frame. Dummy samples are not included. */
    uint32_t    ui32NumSamplesDecoded;

    /* This is the cumulative count for number of Valid frames decoded */
    uint32_t    ui32TotalFramesDecoded;

    /* This is the cumulative count for number of Dummy decodes done by decoder */
    uint32_t    ui32TotalFramesDummy;

    /* This is the cumulative count for number of erroneous frames detected by decoder. */
    uint32_t    ui32TotalFramesInError;

    /* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

} BDSP_Raaga_Audio_DecodeStreamInfo;


/*********************************************************************
Summary:
    This is the structure which defines the default stream information  for audio
    processing types.
Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_ProcessingStreamInfo
{
    /*  No of channels */
    uint32_t   ui32NumChannels;
}BDSP_Raaga_Audio_ProcessingStreamInfo;


/*********************************************************************
Summary:
    This is the structure which defines the default stream information for Audio Encode
    types.
Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_EncodeStreamInfo
{
    /*  No of channels */
    uint32_t   ui32NumChannels;
}BDSP_Raaga_Audio_EncodeStreamInfo;


/*********************************************************************
Summary:
    This is the structure which defines the default stream information for Video Decode
    types.
Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Video_DecodeStreamInfo
{
    /*  Dummy variable */
     uint32_t   ui32Dummy;
}BDSP_Raaga_VideoStreamInfo;

/*********************************************************************
Summary:
    This structure defines all the stream information parameters of
    DTSHD Broadcast  Decoder which is required by PI at every frame boundary.
Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_DtsHdFrameInfo
{
    uint32_t    ui32NumOfChannels;
    uint32_t    ui32SampleRate;
    uint32_t    ui32ChannelAllocation;
    uint32_t    ui32LFEPresent;
    uint32_t    ui32BitsPerSample;
    uint32_t    ui32ErrorStatus;
    uint32_t    ui32CoreExtensionMask;
    uint32_t    ui32DTSNeoEnable;
    uint32_t    ui32EsFlag;
    uint32_t    ui32ExtendedCodingFlag;
    uint32_t    ui32ExtensionAudioDescriptorFlag;
    uint32_t    ui32PCMFrameSize;

} BDSP_Raaga_Audio_DtsHdFrameInfo;

typedef struct BDSP_Raaga_Audio_DtsHdStreamInfo
{
    uint32_t      ui32CrcFlag;
    uint32_t      ui32NBlocks;
    uint32_t      ui32Fsize;
    uint32_t      ui32Amode;
    uint32_t      ui32TransRate;
    uint32_t      ui32DownMix;
    uint32_t      ui32DynRangeCoeff;
    uint32_t      ui32HdcdFormat;
    uint32_t      ui32ExtAudioId;
    uint32_t      ui32ExtAudioFlag;
    uint32_t      ui32VerNum;
    uint32_t      ui32CopyHist;
    uint32_t      ui32PcmResolution;

    BDSP_Raaga_Audio_DtsHdFrameInfo sDtsFrameInfo;
/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/

    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/

    uint32_t    ui32TotalFramesInError;

    uint32_t        ui32StatusValid;

    /* Bit rate of the input stream */
    uint32_t       ui32BitRate;

    /* Will be used for SPDIF/HDMI status in passthru */
    uint32_t      ui32SPDIFTransferRate;
    uint32_t      ui32HDMITransferRate;

} BDSP_Raaga_Audio_DtsHdStreamInfo;

typedef struct BDSP_Raaga_Audio_DtslbrStreamInfo
{
    uint32_t           ui32SamplingFrequency;              /*           The sampling frequency value, 0 in case of error, otherwise sampling frequency as it's*/

    uint32_t           ui32FrameSize;                                 /*           This field contains the total frame size in bytes of the current frame.*/

    uint32_t           ui32UserOutputMode;                  /*           User defined Output configuration, same as eBufferOutputMode*/
                                        /*Syntex according to BCOM_eACMOD                                        */

    uint32_t           ui32BitResolution;                           /*PCM resolution in bits*/

    int32_t                              DRCCoefPresent;                                             /*1 -> if stream has DRC, 0 -> if stream does not contain DRC */

    uint32_t           ui32NumChannels;                          /*           Number of channels in input stream*/

    uint32_t           ui32ErrorStatus;
        /*   Error status as sent by the decoder,
            0 - Frame is sane
            1 - Frame is corrupt. O/P has been Interpolated based on prev values
        */

    uint32_t           ui32NumSamplesDecoded;
        /*   This is the cumulative number of samples decoded by any decoder from the channel open time.
            Decoder should accumulate number of samples decoded for every frame. Dummy samples are not included.
        */

    uint32_t           ui32TotalFramesDecoded;
        /*   This is the cumulative count for number of valid frames decoded
        */

    uint32_t           ui32TotalFramesDummy;
        /*   This is the cumulative count for number of Dummy decodes done by decoder
        */

    uint32_t           ui32TotalFramesInError;
        /*   This is the cumulative count for number of erroneous frames detected by decoder.
       */

    uint32_t           ui32StatusValid;
        /*   This field tells whether the stream info fields are valid or not range according to BCOM_eSUCCESS_FAIL
            eFAIL => invalid, eSUCCESS => valid
        */
    /* Bit Rate of the stream */
    uint32_t    ui32BitRate;

}BDSP_Raaga_Audio_DtslbrStreamInfo;

/*********************************************************************
Summary:
    This structure defines all the stream information parameters of
    DTS Broadcast  Encoder which is required by PI at every frame boundary.
Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_DTSBroadcastEncoderStreamInfo
{

/*   0 = A (Mono)
     1 = A + B (dual mono)
     2 = L + R (stereo)
     3 = (L+R) + (L-R) (sum-difference)
     4 = LT +RT (left and right total)
     5 = C + L + R
     6 = L + R+ S
     7 = C + L + R+ S
     8 = L + R+ SL+SR
     9 = C + L + R+ SL+SR
     10 = CL + CR + L + R + SL + SR
     11 = C + L + R+ LR + RR + OV
     12 = CF+ CR+LF+ RF+LR + RR
     13 = CL + C + CR + L + R + SL + SR
     14 = CL + CR + L + R + SL1 + SL2+ SR1 + SR2
     15 = CL + C+ CR + L + R + SL + S+ SR
*/
    uint32_t ui32AudioMode;

/* Sampling frequency */
    uint32_t ui32SamplingFreq;

/* Bit Rate */
    uint32_t ui32BitRate;

/* LFE Flag
    1 = LFE ON
    0 = LFE OFF
*/
    uint32_t ui32LFEFlag;


/* This field tells whether the stream info fields are valid or not */
    uint32_t ui32StatusValid;

}BDSP_Raaga_Audio_DTSBroadcastEncoderStreamInfo;
/*********************************************************************
Summary:
    This structure defines all the stream information parameters of
    WMA decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_WmaStreamInfo
{


/*
    The field tells the presence of original copy
    0 = Copy
    1 = Original
*/
    uint32_t    ui32OriginalCopy;
/*
    The field tells the presence of cpyright
    0 = Cpyright_Absent
    1 = Cpyright_Present
*/
    uint32_t    ui32Copyright;
/*
    The stereo mode as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32Mode;
/*
    The sampling frequency value as defined by the ISO 11172-3 audio specification.
    0 = Sampling_44_1_kHz
    1 = Sampling_48_kHz
    2 = Sampling_32_kHz
    3 = Reserved
*/
    uint32_t    ui32SamplingFreq;
/*
    The bitrate value as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32BitRate;
/*
    The field tells the presence of CRC
    0 = CRC_Absent
    1 = CRC_Present
*/
    uint32_t    ui32CrcPresent;

/* ACMOD

    ui32Acmod =0 One Channel
    ui32Acmod =1 Two Channel

*/
    uint32_t        ui32Acmod;

/* Version

    0 = Version1
    1 = Version2

*/
    uint32_t        ui32Version;

/*
    Low Frequency Effects Channel
    0 = LFE_channel_off
    1 = LFE_channel_ON
*/
    uint32_t    ui32LfeOn;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/

    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/

    uint32_t    ui32TotalFramesInError;

/* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;


}BDSP_Raaga_Audio_WmaStreamInfo;


/*********************************************************************
Summary:
    This structure defines all the stream information parameters of
    WMAPro decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_WmaProStreamInfo
{


/*
    The field tells the presence of original copy
    0 = Copy
    1 = Original
*/
    uint32_t    ui32OriginalCopy;
/*
    The field tells the presence of cpyright
    0 = Cpyright_Absent
    1 = Cpyright_Present
*/
    uint32_t    ui32Copyright;
/*
    The stereo mode as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32Mode;
/*
    The sampling frequency value as defined by the ISO 11172-3 audio specification.
    0 = Sampling_44_1_kHz
    1 = Sampling_48_kHz
    2 = Sampling_32_kHz
    3 = Reserved
*/
    uint32_t    ui32SamplingFreq;
/*
    The bitrate value as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32BitRate;
/*
    The field tells the presence of CRC
    0 = CRC_Absent
    1 = CRC_Present
*/
    uint32_t    ui32CrcPresent;

/* ACMOD

    ui32Acmod =0 to 7

*/
    uint32_t        ui32Acmod;

/* Version

    0 = Version1
    1 = Version2

*/
    uint32_t        ui32Version;

/*
    Low Frequency Effects Channel
    0 = LFE_channel_off
    1 = LFE_channel_ON
*/
    uint32_t    ui32LfeOn;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/

    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/

    uint32_t    ui32TotalFramesInError;

/* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;


}BDSP_Raaga_Audio_WmaProStreamInfo;



/*********************************************************************
Summary:
    This structure defines all the stream information parameters of AC-3
    decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_Ac3StreamInfo
{

/*
    This field decides the sampling frequency of the AC-3 stream.
    0 = Sampling_48_kHz
    1 = Sampling_44_1_kHz
    2 = Sampling_32_kHz
*/
    uint32_t    ui32SamplingFrequency;
/*
    The bit stream identification value. Bit stream identification
    value in this standard is 8. The future modifications of this
    standard may define other values.
*/
    uint32_t    ui32BitStreamIdentification;
/*
    Transmission mode
    0 = main_audio_service_complete_main_CM
    1 = main_audio_service_music_and_effects_ME
    2 = associated_service_visually_impaired_VI
    3 = associated_service_hearing_impaired_HI
    4 = associated_service_dialogue_D
    5 = associated_service_commentary_C
    6 = associated_service_emergency_E
    7 = associated_service_stream_identification_along_with_ACMOD_value
*/
    uint32_t    ui32BsmodValue;
/*
    Audio coding mode which describes the number of input channels.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32AcmodValue;
/*
    Dolby Surround Mode (dsurmod)
    0 = Not_indicated
    1 = NOT_Dolby_Surround_encoded
    2 = Dolby_Surround_encoded
*/
    uint32_t    ui32DsurmodValue;
/*
    Low Frequency Effects Channel
    0 = LFE_channel_off
    1 = LFE_channel_ON
*/
    uint32_t    ui32LfeOn;
/*
    This field indicates whether the AC-3 language code exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32LangCodeExists;
    /* This field indicates the language code value. */
    uint32_t    ui32LangCodeValue;
/*
    This field indicates the room type value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomTypeValue;
/*
    This field indicates the room type2 value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomType2Value;
/*
    This field indicates that stream is protected by cpyright.
    0 = stream_not_cpyright
    1 = stream_cpyright
*/
    uint32_t    ui32CopyrightBit;
/*
    This field indicates the stream is original bit stream.
    0 = Stream_not_original
    1 = Stream_original
*/
    uint32_t    ui32OriginalBitStream;

/*
    AC-3 centre mix level value
    0 = three_dB_below
    1 = four_and_half_dB_below
    2 = six_dB_below
    3 = RESERVED
*/
    uint32_t    ui32CmixLevel;
/*
    AC-3 surround mix level value
    0 = three_dB_below
    1 = six_dB_below
    2 = zero_dB_below
    3 = RESERVED
*/
    uint32_t    ui32SurmixLevel;
/*
    This field indicates whether the AC-3 language code for the second channel exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32Langcode2Exists;
/*
    This field indicates the language code value for the second channel.
*/
    uint32_t    ui32Langcode2Value;
/*
    This field indicates whether the time code exits or not.
    0 = Not_present
    1 = First_half_present
    2 = Second_half_present
    3 = Both_halves_present
*/
    uint32_t    ui32TimeCodeExists;
/*
This is time code for low resolution in 8 seconds increments up to 24 hours. First 5 bits
indicates the time in hours. Next 6 bits represents the time in minutes. Final 3 bits
represents the time in 8 seconds increments.
*/
    uint32_t    ui32TimeCodeFirstHalf;
/*
    Output LFE Channel Configuration
    1 = LFE_ON
    0 = LFE_OFF
*/
    uint32_t    ui32OutputLfeMode;
/*
    Ouput channel Configuration
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;
/*
    The frame size code is used in conjunction with the sample rate code to determine
    the number of 16-bit words before the next syncword.
*/
    uint32_t    ui32FrmSizeCod;
/*
    This is time code for high resolution in 1/64th frame increments up to 8 seconds.
    The first 3 bits indicate the time in seconds. The next 5 bits represent the time
    in frames. The final 6 bits represent the time in fractions of 1/64th of a frame.
*/
    uint32_t    ui32TimeCodeSecondHalf;
/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/

    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/

    uint32_t    ui32TotalFramesInError;

/* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

}BDSP_Raaga_Audio_Ac3StreamInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of DDP
    decoder which is required by PI at every frame boundary.
Description:

See Also:
**********************************************************************/


typedef struct BDSP_Raaga_Audio_DdpStreamInfo
{

/*
    This field decides the sampling frequency of the AC-3 stream.
    0 = Sampling_48_kHz
    1 = Sampling_44_1_kHz
    2 = Sampling_32_kHz
*/
    uint32_t    ui32SamplingFrequency;
/*
    The bit stream identification value. Bit stream identification
    value in this standard is 8. The future modifications of this
    standard may define other values.
*/
    uint32_t    ui32BitStreamIdentification;
/*
    Transmission mode
    0 = main_audio_service_complete_main_CM
    1 = main_audio_service_music_and_effects_ME
    2 = associated_service_visually_impaired_VI
    3 = associated_service_hearing_impaired_HI
    4 = associated_service_dialogue_D
    5 = associated_service_commentary_C
    6 = associated_service_emergency_E
    7 = associated_service_stream_identification_along_with_ACMOD_value
*/
    uint32_t    ui32BsmodValue;
/*
    Audio coding mode which describes the number of input channels.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32AcmodValue;
/*
    Dolby Surround Mode (dsurmod)
    0 = Not_indicated
    1 = NOT_Dolby_Surround_encoded
    2 = Dolby_Surround_encoded
*/
    uint32_t    ui32DsurmodValue;
/*
    Low Frequency Effects Channel
    0 = LFE_channel_off
    1 = LFE_channel_ON
*/
    uint32_t    ui32LfeOn;
/*
    This field indicates whether the AC-3 language code exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32LangCodeExists;
    /* This field indicates the language code value. */
    uint32_t    ui32LangCodeValue;
/*
    This field indicates the room type value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomTypeValue;
/*
    This field indicates the room type2 value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomType2Value;
/*
    This field indicates that stream is protected by cpyright.
    0 = stream_not_cpyright
    1 = stream_cpyright
*/
    uint32_t    ui32CopyrightBit;
/*
    This field indicates the stream is original bit stream.
    0 = Stream_not_original
    1 = Stream_original
*/
    uint32_t    ui32OriginalBitStream;

/*
    AC-3 centre mix level value
    0 = three_dB_below
    1 = four_and_half_dB_below
    2 = six_dB_below
    3 = RESERVED
*/
    uint32_t    ui32CmixLevel;
/*
    AC-3 surround mix level value
    0 = three_dB_below
    1 = six_dB_below
    2 = zero_dB_below
    3 = RESERVED
*/
    uint32_t    ui32SurmixLevel;
/*
    This field indicates whether the AC-3 language code for the second channel exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32Langcode2Exists;
/*
    This field indicates the language code value for the second channel.
*/
    uint32_t    ui32Langcode2Value;
/*
    This field indicates whether the time code exits or not.
    0 = Not_present
    1 = First_half_present
    2 = Second_half_present
    3 = Both_halves_present
*/
    uint32_t    ui32TimeCodeExists;
/*
This is time code for low resolution in 8 seconds increments up to 24 hours. First 5 bits
indicates the time in hours. Next 6 bits represents the time in minutes. Final 3 bits
represents the time in 8 seconds increments.
*/
    uint32_t    ui32TimeCodeFirstHalf;
/*
    Output LFE Channel Configuration
    1 = LFE_ON
    0 = LFE_OFF
*/
    uint32_t    ui32OutputLfeMode;
/*
    Ouput channel Configuration
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;
/*
    The frame size code is used in conjunction with the sample rate code to determine
    the number of 16-bit words before the next syncword.
*/
    uint32_t    ui32FrmSizeCod;
/*
    This is time code for high resolution in 1/64th frame increments up to 8 seconds.
    The first 3 bits indicate the time in seconds. The next 5 bits represent the time
    in frames. The final 6 bits represent the time in fractions of 1/64th of a frame.
*/
    uint32_t    ui32TimeCodeSecondHalf;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/

    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/

    uint32_t    ui32TotalFramesInError;
/*
    This field tells whether the stream info fields are valid or not
*/
    uint32_t    ui32StatusValid;

}BDSP_Raaga_Audio_DdpStreamInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     MPEG-1 L1/L2/L3 decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_MpegStreamInfo
{

/*
    This field indicates the type of de-emphasis that shall be used.
    0 = None
    1 = Emph_50_15ms
    2 = Reserved
    3 = CCITT_J17
*/
    uint32_t    ui32Emphasis;
/*
    The field tells the presence of original copy
    0 = Copy
    1 = Original
*/
    uint32_t    ui32OriginalCopy;
/*
    The field tells the presence of cpyright
    0 = Cpyright_Absent
    1 = Cpyright_Present
*/
    uint32_t    ui32Copyright;
/*
    The stereo mode as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32Mode;
/*
    The sampling frequency value as defined by the ISO 11172-3 audio specification.
    0 = Sampling_44_1_kHz
    1 = Sampling_48_kHz
    2 = Sampling_32_kHz
    3 = Reserved
*/
    uint32_t    ui32SamplingFreq;
/*
    The bitrate value as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32BitRate;
/*
    The field tells the presence of CRC
    0 = CRC_Absent
    1 = CRC_Present
*/
    uint32_t    ui32CrcPresent;
/*
    This field tells the MPEG layer type used
    1 = Layer3
    2 = Layer2
    3 = Layer1
*/
    uint32_t    ui32MpegLayer;
/*
    The value of this field is
    1 = ISO11172_Audio
*/
    uint32_t    ui32AlgorithmId;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;
/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/
    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;
/* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

}BDSP_Raaga_Audio_MpegStreamInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     AAC-HE  decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_AacStreamInfo
{


/*
    The bitrate value as defined by the ISO 11172-3 audio specification.
*/
    uint32_t    ui32BitRate;


/*  AAC Profile:
    0 = Main_profile
    1 = Low_complexity_profile
    2 = Scaleable_Sampling_Rate_profile
    3 = RESERVED

*/

    uint32_t    ui32Profile;

/*
    The sampling frequency value as defined by the ISO 11172-3 audio specification.
    0 = Sampling_96_kHz
    1 = Sampling_88_2_kHz
    2 = Sampling_64_kHz
    3 = Sampling_48_kHz
    4 = Sampling_44_1_kHz
    5 = Sampling_32_kHz
    6 = Sampling_24_kHz
    7 = Sampling_22_05_kHz
    8 = Sampling_16_kHz
    9 = Sampling_12_kHz
    10 = Sampling_11_025_kHz
    11 = Sampling_8_kHz
    12 = RESERVED_12
    13 = RESERVED_13
    14 = RESERVED_14
    15 = RESERVED_15
*/
    uint32_t    ui32SamplingFreq;

/*
    The number of LFE channel elements.
*/
    uint32_t    ui32NumLfeChannels;
/*
    The number of audio syntactic elements for the back channels.
*/
    uint32_t    ui32NumBackChannels;

/*
    The number of audio syntactic elements in the side.
*/
    uint32_t    ui32NumSideChannels;

/*
    The number of audio syntactic elements in the front channels, front center to
    back center, symmetrically by left and right, or alternating by left and right
    in the case of single channel element.
*/
    uint32_t    ui32NumFrontChannels;

/*
    Output LFE Channel Configuration
    1 = LFE_ON
    0 = LFE_OFF
*/
    uint32_t    ui32OutputLfeMode;
/*
    Ouput channel Configuration
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;

/*
    Low Frequency Effects Channel
    0 = LFE_channel_off
    1 = LFE_channel_ON
*/
    uint32_t    ui32LfeOn;

/*
    Audio coding mode which describes the number of input channels.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32AcmodValue;

/*
    A 1 bit field indicating that pseudo surround decoding is possible.
    0 = Disabled
    1 = Enabled
*/
    uint32_t        ui32PseudoSurroundEnable;

/*
    A 2 bit field that indicates the coefficient to be used in 5 to 2 matrix mixdown.
*/
    uint32_t        ui32MatrixMixdownIndex;

/*
    One bit indicating that a stereo matrix coefficient is present in the stream.
    0 = Mixdown_Absent
    1 = Mixdown_Present
*/
    uint32_t    ui32MatrixMixdownPresent;

/*
    This is a one bit flag that indicates whether DRC is present in the stream.
    0 = DRC_Absent
    1 = DRC_Present
*/
    uint32_t    ui32DrcPresent;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;
/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/
    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;
/* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

}BDSP_Raaga_Audio_AacStreamInfo;




/*********************************************************************
Summary:
    This structure holds all the TSM related field values which is
    required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_AudioTaskTsmStatus
{
    uint32_t        ui32RunningPts;
    int32_t         i32PtsToStcPhase;
    BDSP_PtsType    ePtsType;   /* The PTS type tag */
    uint32_t        ui32StatusValid;
    int32_t         i32FrameSizeIn45khzTicks;
}BDSP_AudioTaskTsmStatus;

/*********************************************************************
Summary:
    This structure holds the Datasync related field values which is
    required by PI.

Description:

See Also:
**********************************************************************/
typedef struct BDSP_AudioTaskDatasyncStatus
{
    uint32_t    ui32CDBUuderFlowCount;
    uint32_t    ui32StatusValid;
}BDSP_AudioTaskDatasyncStatus;


/*********************************************************************
Summary:
    This structure holds all the IDS and TSM related field values which
    is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_IdsTsmInfo
{
    BDSP_AudioTaskTsmStatus         sTsmInfo;
    BDSP_AudioTaskDatasyncStatus    sDatasyncInfo;
}BDSP_Raaga_Audio_IdsTsmInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     DTS Core decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_DtsCoreStreamInfo
{
    /*
    This field specifies the sampling frequency of the audio samples in
    the DTS core encoder.
    0 = Invalid_0
    1 = Sampling_8_kHz
    2 = Sampling_16_kHz
    3 = Sampling_32_kHz
    4 = Invalid_4
    5 = Invalid_5
    6 = Sampling_11_025_kHz
    7 = Sampling_22_05_kHz
    8 = Sampling_44_1_kHz
    9 = Invalid_9
    10 = Invalid_10
    11 = Sampling_12_kHz
    12 = Sampling_24_kHz
    13 = Sampling_48_kHz
    14 = Invalid_14
    15 = Invalid_15
    */
    uint32_t    ui32SamplingFreq;
    /*
    This field specifies the number of audio channels and the audio playback
    arrangement.
    0 = One_ch_A
    1 = Two_ch_A_B
    2 = Two_ch_L_R
    3 = Two_ch_LpR_LmR
    4 = Two_ch_LT_RT
    5 = Three_ch_C_L_R
    6 = Three_ch_L_R_S
    7 = Four_ch_C_L_R_S
    8 = Four_ch_L_R_SL_SR
    9 = Five_ch_C_L_R_SL_SR
    10 = Six_ch_CL_CR_L_R_SL_SR
    11 = Six_ch_C_L_R_LR_RR_OV
    12 = Six_ch_CF_CR_LF_RF_Lr_Rr
    13 = Seven_ch_CL_C_CR_L_R_SL_SR
    14 = Eight_ch_CL_CR_L_R_SL1_SL2_SR1_SR2
    15 = Eight_ch_CL_C_CR_L_R_SL_S_SR
    16 = User_Defined_16

    */
    uint32_t ui32AcmodValue;
    /*
    This field contains the total byte size of the current frame
    including primary and any extension audio data.Valid range
    is 95 to 16383. Invalid range is 0 to 94.
    */
    uint32_t ui32FrameSize;
    /*
    This field specifies the number block of 32 PCM samples per channel.
    Invalid range is 0 to 4.
    */
    uint32_t ui32NumPcmBlocks;
    /*
    This field indicates the presence of CRC in the bitstream.
    0 = Not_present
    1 = Present
    */
    uint32_t ui32CrcFlag;
    /*
    This field indicates the quantization resolution of source PCM samples.
    The left and right surround channels of the source material are mastered
    in DTS ES format if ES = 1.
    0 = ES0_16_bits
    1 = ES1_16_bits
    2 = ES0_20_bits
    3 = ES1_20_bits
    4 = ES0_24_bits
    5 = ES1_24_bits
    6 = Invalid_6
    7 = Invalid_7

    */
    uint32_t ui32SourcePcm;
    /*
    This field indicates the cpy history of the audio.
    0 = Copy_prohibited
    1 = First_generation
    2 = Second_generation
    3 = Original_material
    */
    uint32_t ui32CopyHistory;
    /*
    This field indicates the revision status of the encoder software.
    In addition it is used to indicate the presence of the dialog
    normalization parameters.
    A value of 0-6 indicates a future (compatible) revision.
    A value of 7 indicates the current version.
    A value of 8-15 indicates a future (incompatible) revision.
    */
    uint32_t ui32VersionNumber;
    /*
    Indicates if the LFE channel is present and the choice of the
    interpolation factor.
    0 = Not_present
    1 = Present_128
    2 = Present_64
    3 = Invalid
    */
    uint32_t ui32LfePresentFlag;
    /*
    This field indicates if extended audio coding data are present after
    the core audio data.
    0 = Not_present
    1 = Present
    */
    uint32_t ui32ExtAudio;
        /*
        This flag has meaning only if EXT_AUDIO = 1 and then it indicates
        the type of data that has been placed in the extension stream(s).
        0 = XCh
        1 = XXCh
        2 = X96k
        3 = Reserved_3
        4 = Reserved_4
        5 = Reserved_5
        6 = Reserved_6
        7 = Reserved_7
    */
    uint32_t ui32ExtAudioId;
    /*
    The source material is mastered in HDCD format if HDCD=1,
    and otherwise HDCD=0.
    */
    uint32_t ui32HdCdFormat;

    /*
    The embedded dynamic range coefficients are included at the start of
    each subframe.
    0 = Not_present
    1 = Present
    */
    uint32_t ui32EmbDrc;

    /*
    The embedded downmixing coefficients are included at the start of each
    subframe.
    0 = Not_present
    1 = Present
    */
    uint32_t ui32EmbDownmixEnable;

    /*
    The average data rate for the current frame of audio.
    0 = kbps_32
    1 = kbps_56
    2 = kbps_64
    3 = kbps_96
    4 = kbps_112
    5 = kbps_128
    6 = kbps_192
    7 = kbps_224
    8 = kbps_256
    9 = kbps_320
    10 = kbps_384
    11 = kbps_448
    12 = kbps_512
    13 = kbps_576
    14 = kbps_640
    15 = kbps_768
    16 = kbps_960
    17 = kbps_1024
    18 = kbps_1152
    19 = kbps_1280
    20 = kbps_1344
    21 = kbps_1408
    22 = kbps_1411_2
    23 = kbps_1472
    24 = kbps_1536
    25 = kbps_1920
    26 = kbps_2048
    27 = kbps_3072
    28 = kbps_3840
    29 = Open
    30 = Variable
    31 = Lossless
    */
    uint32_t ui32BitRate;

    /*
    Output LFE Channel Configuration
    1 = LFE_ON
    0 = LFE_OFF
    */
    uint32_t    ui32OutputLfeMode;
    /*
    Ouput channel Configuration
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
    */
    uint32_t    ui32OutputMode;

    /*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
    */
    uint32_t    ui32NumSamplesDecoded;
    /*
    This is the cumulative count for number of Valid frames decoded
    */
    uint32_t    ui32TotalFramesDecoded;
    /*
    This is the cumulative count for number of Dummy decodes done by decoder
    */

    uint32_t    ui32TotalFramesDummy;
    /*
    This is the cumulative count for number of erroneous frames detected by decoder.
    */

    uint32_t    ui32TotalFramesInError;


    /* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

}BDSP_Raaga_Audio_DtsCoreStreamInfo;
/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     LPCM decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_LpcmStreamInfo
{

  /*This field indicates the size in bytes of the LPCM audio frame*/
   uint32_t    ui32FrameSize;

  /*This field specifies the channel assignment for the channel configuration in the LPCM audio stream.
    0 = Reserved_0
    1 = Mono
    2 = Reserved_2
    3 = L_R_Stereo
    4 = L_C_R_3_0
    5 = L_R_S_2_1
    6 = L_C_R_S_3_1
    7 = L_R_Ls_Rs_2_2
    8 = L_C_R_Ls_Rs_3_2
    9 = L_C_R_Ls_Rs_LFE_3_2
    10 = L_C_R_Ls_Cs1_Cs2_Rs_3_4
    11 = L_C_R_Ls_Cs1_Cs2_Rs_LFE_3_4
    12 = Reserved_12
    13 = Reserved_13
    14 = Reserved_14
    15 = Reserved_15  */
    uint32_t   ui32ChannelAssignment;


   /*This field specifies the sampling frequency of the LPCM audio stream.
    0 = Reserved_0
    1 = Sampling_48_kHz
    2 = Reserved_2
    3 = Reserved_3
    4 = Sampling_96_kHz
    5 = Sampling_192_kHz
    6 = Reserved_6
    15 = Reserved_15 */
    uint32_t    ui32SamplingFreq;

    /*This field specifies the sampling resolution of the audio samples for all channels in the LPCM audio stream.
    0 = Reserved
    1 = bits_16
    2 = bits_20
    3 = bits_24 */
    uint32_t    ui32BitsPerSample;

    /* This field indicates the track-start or index-point.
    0 = Not_present
    1 = Present*/
    uint32_t    ui32StartFlag;


   /* This field indicates Ouput LFE channel Configuration
    1 = LFE_ON
    0 = LFE_OFF*/
    uint32_t   ui32OutputLfeMode;

    /* This field indicates Ouput channel Configuration
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR */
    uint32_t   ui32OutMode;

    /*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
    */
    uint32_t    ui32NumSamplesDecoded;
    /*
    This is the cumulative count for number of Valid frames decoded
    */
    uint32_t    ui32TotalFramesDecoded;
    /*
    This is the cumulative count for number of Dummy decodes done by decoder
    */
    uint32_t    ui32TotalFramesDummy;
    /*
    This is the cumulative count for number of erroneous frames detected by decoder.
    */

    uint32_t    ui32TotalFramesInError;

    /* This field tells whether the stream info fields are valid or not */
    uint32_t ui32StatusValid;

    /* This field tells us the bitrate of the input stream */
    uint32_t ui32BitRate;

}BDSP_Raaga_Audio_LpcmStreamInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     Custom voice PP which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_CustomVoiceStatusInfo
{
    int32_t i32PVCMonitorLevel[2];

    /* This field tells whether the stream info fields are valid or not */
    uint32_t ui32StatusValid;

} BDSP_Raaga_Audio_CustomVoiceStatusInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     PassThru PP which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_PassthruStreamInfo
{
    /*  Specifies the Algorithm ID that is being transmitted */
    uint32_t              ui32AlgoType;

    /*  Specifies the input stream sampling rate */
    uint32_t              ui32SamplingFreq;

    /*  Specifies the repetition period that is being used for the algorithm */
    uint32_t               ui32RepPeriod;

    /*  Specifies the pause burst repetition period that is being used for the algorithm */
    uint32_t               ui32RepPausePeriod;

    /* This is the cumulative count for number of valid frames passed through */
    uint32_t               ui32TotalFramesPassed;

    /* This is the cumulative count for number of pause bursts */
    uint32_t               ui32TotalFramesDummy;

    /* This field tells whether the stream info fields are valid or not */
    uint32_t               ui32StatusValid;

} BDSP_Raaga_Audio_PassthruStreamInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     PCM WAV decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_PcmWavStreamInfo
{
     uint32_t   ui32SamplingFreq;

     uint32_t   ui32NumChannels;

     /*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
    */
    uint32_t    ui32NumSamplesDecoded;
    /*
        This is the cumulative count for number of Valid frames decoded
    */
    uint32_t    ui32TotalFramesDecoded;
    /*
        This is the cumulative count for number of Dummy decodes done by decoder
    */
    uint32_t    ui32TotalFramesDummy;
    /*
        This is the cumulative count for number of erroneous frames detected by decoder.
    */
    uint32_t    ui32TotalFramesInError;
    /* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

    /* This field gives the bit rate */
    uint32_t    ui32BitRate;

}BDSP_Raaga_Audio_PcmWavStreamInfo;



/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     AMR decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_AmrStreamInfo
{
/*
    The bitrate of the stream.
    0 = 4.75kbps
    1 = 5.15kbps
    2 = 5.90kbps
    3 = 6.70kbps
    4 = 7.40kbps
    5 = 7.95kbps
    6 = 10.2kbps
    7 = 12.2kbps
    8 = Silence Frame
*/
    uint32_t    ui32BitRate;

/*
    The sampling frequency of the stream, 8kHz always.
*/
    uint32_t    ui32SamplingFreq;

/*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C, always.
*/
    uint32_t    ui32AcmodValue;

/*
    Ouput channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;

/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;

/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;

/*
    This field tells whether the stream info fields are valid or not.
*/
    uint32_t    ui32StatusValid;

} BDSP_Raaga_Audio_AmrStreamInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     AMRWB decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_AmrWbStreamInfo
{
/*
    The bitrate of the stream.
    0 = 4.75kbps
    1 = 5.15kbps
    2 = 5.90kbps
    3 = 6.70kbps
    4 = 7.40kbps
    5 = 7.95kbps
    6 = 10.2kbps
    7 = 12.2kbps
    8 = Silence Frame
*/
    uint32_t    ui32BitRate;

/*
    The sampling frequency of the stream, 8kHz always.
*/
    uint32_t    ui32SamplingFreq;

/*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C, always.
*/
    uint32_t    ui32AcmodValue;

/*
    Ouput channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;

/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;

/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;

/*
    This field tells whether the stream info fields are valid or not.
*/
    uint32_t    ui32StatusValid;

} BDSP_Raaga_Audio_AmrWbStreamInfo;

typedef struct BDSP_Raaga_Audio_iLBCStreamInfo
{
/*
    BCOM_eWVFMTX_TYPES_eiLBC = 0x503
*/

   uint32_t ui32StreamType;
/*
    The bitrate of the stream.
    0 = 15.2kbps
    1 = 13.3kbps

*/
    uint32_t    ui32BitRate;

/*
    The sampling frequency of the stream, 8kHz always.
*/
    uint32_t    ui32SamplingFreq;

/*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C, always.
*/
    uint32_t    ui32AcmodValue;

/*
    Ouput channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;

/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;

/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;

/*
    This field tells whether the stream info fields are valid or not.
*/
    uint32_t    ui32StatusValid;

} BDSP_Raaga_Audio_iLBCStreamInfo;


typedef struct BDSP_Raaga_Audio_iSACStreamInfo
{

/*
    BCOM_eWVFMTX_TYPES_eiSAC = 0x504
*/
    uint32_t ui32StreamType;

    /*

    The bitrate of the stream 10kbps - 32kbps .

    */
    uint32_t    ui32BitRate;

/*
    The sampling frequency of the stream, 8kHz and 16khz.
*/
    uint32_t    ui32SamplingFreq;

/*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C, always.
*/
    uint32_t    ui32AcmodValue;

/*
    Ouput channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;

/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;

/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;

/*
    This field tells whether the stream info fields are valid or not.
*/
    uint32_t    ui32StatusValid;

} BDSP_Raaga_Audio_iSACStreamInfo;

typedef struct BDSP_Raaga_Audio_UdcStreamInfo
{

/*
    This field decides the sampling frequency of the AC-3 stream.
    0 = Sampling_48_kHz
    1 = Sampling_44_1_kHz
    2 = Sampling_32_kHz
*/
    uint32_t    ui32SamplingFrequency;
/*
    The bit stream identification value. Bit stream identification
    value in this standard is 8. The future modifications of this
    standard may define other values.
*/
    uint32_t    ui32BitStreamIdentification;
/*
    Transmission mode
    0 = main_audio_service_complete_main_CM
    1 = main_audio_service_music_and_effects_ME
    2 = associated_service_visually_impaired_VI
    3 = associated_service_hearing_impaired_HI
    4 = associated_service_dialogue_D
    5 = associated_service_commentary_C
    6 = associated_service_emergency_E
    7 = associated_service_stream_identification_along_with_ACMOD_value
*/
    uint32_t    ui32BsmodValue;
/*
    Audio coding mode which describes the number of input channels.
    0   = Dual Mono L, R
    1   = C
    2   = L, R
    3   = L, C, R
    4   = L, R, Ls
    5   = L, C, R, Ls
    6   = L, R, Ls, Rs
    7       = L, R, C, Ls, Rs
    8       = L, C, R, Cvh
    9       = L, R, Ls, Rs, Ts
    10  = L, R, C, Ls, Rs, Ts
    11  = L, R, C, Ls, Rs, Cvh
    12  = L, R, C, Lc, Rc
    13  = L, R, Ls, Rs, Lw, Rw
    14  = L, R, Ls, Rs, Lvh, Rvh
    15  = L, R, Ls, Rs, Lsd, Rsd
    16  = L, R, Ls, Rs, Lrs, Rrs
    17  = L, R, C, Ls, Rs, Lc, Rc
    18  = L, R, C, Ls, Rs, Lw, Rw
    19  = L, R, C, Ls, Rs, Lvh, Rvh
    20  = L, R, C, Ls, Rs, Lsd, Rsd
    21  = L, R, C, Ls, Rs, Lrs, Rrs
    22  = L, R, C, Ls, Rs, Ts, Cvh
*/
    uint32_t    ui32AcmodValue;
/*
    indicate the Channel Mode of Dependant streams
*/
    uint32_t    ui32DepFrameChanmapMode;
/*
    Dolby Surround Mode (dsurmod)
    0 = Not_indicated
    1 = NOT_Dolby_Surround_encoded
    2 = Dolby_Surround_encoded
*/
    uint32_t    ui32DsurmodValue;
/*
    Low Frequency Effects Channel
    0 = LFE_channel_off
    1 = LFE_channel_ON
*/
    uint32_t    ui32LfeOn;
/*
    Low Frequency Effects Channel 2
    0 = LFE2_channel_off
    1 = LFE2_channel_ON
*/
    uint32_t    ui32Lfe2On;
/*
    This field indicates whether the AC-3 language code exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32LangCodeExists;
    /* This field indicates the language code value. */
    uint32_t    ui32LangCodeValue;
/*
    This field indicates the room type value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomTypeValue;
/*
    This field indicates the room type2 value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomType2Value;
/*
    This field indicates that stream is protected by cpyright.
    0 = stream_not_cpyright
    1 = stream_cpyright
*/
    uint32_t    ui32CopyrightBit;
/*
    This field indicates the stream is original bit stream.
    0 = Stream_not_original
    1 = Stream_original
*/
    uint32_t    ui32OriginalBitStream;

/*
    AC-3 centre mix level value
    0 = three_dB_below
    1 = four_and_half_dB_below
    2 = six_dB_below
    3 = RESERVED
*/
    uint32_t    ui32CmixLevel;
/*
    AC-3 surround mix level value
    0 = three_dB_below
    1 = six_dB_below
    2 = zero_dB_below
    3 = RESERVED
*/
    uint32_t    ui32SurmixLevel;
/*
    This field indicates whether the AC-3 language code for the second channel exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32Langcode2Exists;
/*
    This field indicates the language code value for the second channel.
*/
    uint32_t    ui32Langcode2Value;
/*
    This field indicates whether the time code exits or not.
    0 = Not_present
    1 = First_half_present
    2 = Second_half_present
    3 = Both_halves_present
*/
    uint32_t    ui32TimeCodeExists;
/*
This is time code for low resolution in 8 seconds increments up to 24 hours. First 5 bits
indicates the time in hours. Next 6 bits represents the time in minutes. Final 3 bits
represents the time in 8 seconds increments.
*/
    uint32_t    ui32TimeCodeFirstHalf;
/*
    Output LFE Channel Configuration
    1 = LFE_ON
    0 = LFE_OFF
*/
    uint32_t    ui32OutputLfeMode;
/*
    0   = Dual Mono L, R
    1   = C
    2   = L, R
    3   = L, C, R
    4   = L, R, Ls
    5   = L, C, R, Ls
    6   = L, R, Ls, Rs
    7       = L, R, C, Ls, Rs
    8       = L, C, R, Cvh
    9       = L, R, Ls, Rs, Ts
    10  = L, R, C, Ls, Rs, Ts
    11  = L, R, C, Ls, Rs, Cvh
    12  = L, R, C, Lc, Rc
    13  = L, R, Ls, Rs, Lw, Rw
    14  = L, R, Ls, Rs, Lvh, Rvh
    15  = L, R, Ls, Rs, Lsd, Rsd
    16  = L, R, Ls, Rs, Lrs, Rrs
    17  = L, R, C, Ls, Rs, Lc, Rc
    18  = L, R, C, Ls, Rs, Lw, Rw
    19  = L, R, C, Ls, Rs, Lvh, Rvh
    20  = L, R, C, Ls, Rs, Lsd, Rsd
    21  = L, R, C, Ls, Rs, Lrs, Rrs
    22  = L, R, C, Ls, Rs, Ts, Cvh
*/
    uint32_t    ui32OutputMode;
/*
    The frame size code is used in conjunction with the sample rate code to determine
    the number of 16-bit words before the next syncword.
*/
    uint32_t    ui32FrmSizeCod;
/*
    This is time code for high resolution in 1/64th frame increments up to 8 seconds.
    The first 3 bits indicate the time in seconds. The next 5 bits represent the time
    in frames. The final 6 bits represent the time in fractions of 1/64th of a frame.
*/
    uint32_t    ui32TimeCodeSecondHalf;
/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/

    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/

    uint32_t    ui32TotalFramesInError;

    uint32_t    ui32NumIndepStreamPresent;
    uint32_t    ui32IndepStreamId[8];
/*
    Dialouge Normalization information extracted from bit stream.
    Possible range 0 to 31 which corresponds to 0 to -31 dB level.
*/
    uint32_t    ui32CurrentDialNorm;
/*
    Dialouge Normalization information extracted from bit stream.
    Possible range 0 to 31 which corresponds to 0 to -31 dB level.
*/
    uint32_t    ui32PreviousDialNorm;


    /* Flag to indicate of the stream has atmos embedded in it */
    uint32_t    ui32AtmosPresent;

/* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

/*
   This field tells us the bit rate of the input stream
 */
    uint32_t   ui32BitRate;

}BDSP_Raaga_Audio_UdcStreamInfo;



#define AC4_DEC_NUM_OF_PRESENTATIONS                         128
#define AC4_DEC_EXT_PRESENTATION_MD_LENGTH                   (64>>2)
#define AC4_DEC_EXT_BITSTREAM_MD_LENGTH                      (64>>2)
#define AC4_DEC_PRESENTATION_NAME_LENGTH                     (36>>2)
#define AC4_DEC_PRESENTATION_LANGUAGE_LENGTH                 (8>>2)
#define AC4_DEC_PROGRAM_IDENTIFIER_LENGTH                    (20>>2)
#define BDSP_AC4_OUTPUT_MAIN        0
#define BDSP_AC4_OUTPUT_ALTSTEREO   1
#define BDSP_AC4_NUM_OUTPUTS        2
typedef struct BDSP_Raaga_Audio_AC4PresentationInfo
{
    /*  Presentation index of the presentation */
    uint32_t    ui32PresentationIndex;

    /*  Boolean flag indicating the presence of a presentation Group in Stream : 0 = Absent, 1 = Present*/
    uint32_t    ui32BPresentationGroupIndex;

    /*  Presentation Group index */
    uint32_t    ui32PresentationGroupIndex;

    /*  Main language of the presentation */
    uint32_t    ui32MainLanguage[ AC4_DEC_PRESENTATION_LANGUAGE_LENGTH];

    /*  Associate decoded type
        0 = Undefined
        1 = Visually impaired
        2 = Hearing impaired
        3 = Commentary */
    uint32_t    ui32AssociateType;

    /*  Name of the presentation */
    uint32_t    ui32PresentationName[AC4_DEC_PRESENTATION_NAME_LENGTH];

    /* Indicates the program identifier for the presentation.
    The Personalized-program universal unique ID is populated in the array in big endian fashion */
    int32_t     i32ProgramIdentifier[AC4_DEC_PROGRAM_IDENTIFIER_LENGTH];

    /*  Extended presentation metadata is availability */
    uint32_t    ui32ExtPresentationMdAvailable;

    /*  Extended presentation metadata length */
    uint32_t    ui32ExtPresentationMdLength;

    /*  Extended presentation metadata payload Id */
    uint32_t    ui32ExtPresentationMdPayloadId;

    /*  Extended presentation metadata payloads */
    uint32_t    ui32ExtPresentationMd[AC4_DEC_EXT_PRESENTATION_MD_LENGTH];

    /* Presentation Config Type */
    /*{ AC4TOC_PRESENTATION_CONFIG_UNDEFINED = 0
     *  AC4TOC_PRESENTATION_CONFIG_SINGLE_SUBSTREAM
     *  AC4TOC_PRESENTATION_CONFIG_MAIN_PLUS_DE
     *  AC4TOC_PRESENTATION_CONFIG_MAIN_PLUS_ASSOCIATE
     *  AC4TOC_PRESENTATION_CONFIG_MAIN_PLUS_DE_PLUS_ASSOCIATE
     *  AC4TOC_PRESENTATION_CONFIG_MAIN_PLUS_HSF_EXT
     *  AC4TOC_PRESENTATION_CONFIG_MUSIC_AND_EFFECTS_PLUS_DIALOG
     *  AC4TOC_PRESENTATION_CONFIG_MUSIC_AND_EFFECTS_PLUS_DIALOG_PLUS_ASSOCIATE
     *  AC4TOC_PRESENTATION_CONFIG_EMDF_ONLY
     *  AC4TOC_PRESENTATION_CONFIG_ARBITRARY_SUBSTREAM_GROUPS
     *  } */
    uint32_t ui32PresentationType;
}BDSP_Raaga_Audio_AC4PresentationInfo;


typedef struct BDSP_Raaga_Audio_AC4StreamInfo
{
    /* Active channel configuration */
    uint32_t ui32ActiveChannelConfig;


    /* Downmix applied to the output channels
        0 = Decoder outputs stereo or a LoRo downmix of multichannel content.
        1 = Decoder outputs stereo or a Dolby ProLogic compatible LtRt downmix of multichannel content.
        2 = Decoder outputs stereo or a Dolby ProLogic II compatible LtRt downmix of multichannel content.
        3 = Decoder outputs multichannel (no downmix) */
    uint32_t ui32DownmixConfig;

    /* Output sampling rate */
    uint32_t ui32EffectiveSamplingRate;

    /* Presentation frame rate according to table 83 and 84 in the ETSI spec */
    uint32_t ui32EffectiveFrameRateIndex;

    /* Dialog enhancement data available
        1 = Available
        0 = Not Available */
    uint32_t ui32DialogEnhancementAvailable;


    /* DRC EAC-3 profile */
    uint32_t ui32DrcEac3Profile;

    /*
    Output LFE Channel Configuration
    1 = LFE_ON
    0 = LFE_OFF
    */
    uint32_t    ui32OutputLfeMode;

    /*
    0   = Dual Mono L, R
    1   = C
    2   = L, R
    3   = L, C, R
    4   = L, R, Ls
    5   = L, C, R, Ls
    6   = L, R, Ls, Rs
    7       = L, R, C, Ls, Rs
    8       = L, C, R, Cvh
    9       = L, R, Ls, Rs, Ts
    10  = L, R, C, Ls, Rs, Ts
    11  = L, R, C, Ls, Rs, Cvh
    12  = L, R, C, Lc, Rc
    13  = L, R, Ls, Rs, Lw, Rw
    14  = L, R, Ls, Rs, Lvh, Rvh
    15  = L, R, Ls, Rs, Lsd, Rsd
    16  = L, R, Ls, Rs, Lrs, Rrs
    17  = L, R, C, Ls, Rs, Lc, Rc
    18  = L, R, C, Ls, Rs, Lw, Rw
    19  = L, R, C, Ls, Rs, Lvh, Rvh
    20  = L, R, C, Ls, Rs, Lsd, Rsd
    21  = L, R, C, Ls, Rs, Lrs, Rrs
    22  = L, R, C, Ls, Rs, Ts, Cvh
    */
    uint32_t    ui32OutputMode;

    /*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
    */
    uint32_t    ui32NumSamplesDecoded;

    /*
    This is the cumulative count for number of Valid frames decoded
    */
    uint32_t    ui32TotalFramesDecoded;
    /*
    This is the cumulative count for number of Dummy decodes done by decoder
    */

    uint32_t    ui32TotalFramesDummy;
    /*
   This is the cumulative count for number of erroneous frames detected by decoder.
    */

    uint32_t    ui32TotalFramesInError;

    /*
    Dialog Normalization information extracted from bit stream.
    Possible range 0 to 31 which corresponds to 0 to -31 dB level.
    */
    uint32_t    ui32CurrentDialNorm;

    /*
    Dialog Normalization information extracted from bit stream.
    Possible range 0 to 31 which corresponds to 0 to -31 dB level.
    */
    uint32_t    ui32PreviousDialNorm;


    /*  This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

    /*
    Bit rate of the input stream
    */
    uint32_t   ui32BitRate;

    /*  Stream info version */
    uint32_t    ui32StreamInfoVersion;

    /*  Indicates if the stream info has changed during the current frame decode */
    uint32_t    ui32StreamInfoChanged;

    /*  Decoded presentation index in the current frame */
    uint32_t    ui32DecodedPresentationIndex[BDSP_AC4_NUM_OUTPUTS];

    /*  Maximum dialog gain of the decoded presentation */
    uint32_t    ui32DecodedPresentationMaxDialogGain;

    /*  Id type */
    uint32_t    ui32IdType;

    /* Program Identifier of the Current decoded presentation */
    int32_t     i32ProgramIdentifier[AC4_DEC_PROGRAM_IDENTIFIER_LENGTH];

    /* Number of presentations in the stream */
    uint32_t    ui32NumPresentations;

    /*  Presentations Infos in the stream */
    BDSP_Raaga_Audio_AC4PresentationInfo                AC4DECPresentationInfo;

    /*  Extended Bitstream Metadata availability indication */
    uint32_t    ui32ExtBitstreamMdAvailable;

    /*  Extended Bitstream Metadata length */
    uint32_t    ui32ExtBitstreamMdLength;

    /*  Extended Bitstream Metadata Payload Id */
    uint32_t    ui32ExtBitstreamMdPayloadId;

    /*  Extended Bitstream Metadata payload */
    uint32_t    ui32ExtBitstreamMd[AC4_DEC_EXT_BITSTREAM_MD_LENGTH];

}BDSP_Raaga_Audio_AC4StreamInfo;




/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     Dolby AACHE decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_DolbyAacheStreamInfo
{
    /*
        The bitrate value as defined by the ISO 11172-3 audio specification.
    */
    uint32_t    ui32BitRate;

    /*
        AAC Profile:
        0 = Main_profile
        1 = Low_complexity_profile
        2 = Scaleable_Sampling_Rate_profile
        3 = RESERVED
    */
        uint32_t    ui32Profile;

    /*
    The sampling frequency value as defined by the ISO 11172-3 audio specification:
         0 = Sampling_96_kHz
         1 = Sampling_88_2_kHz
         2 = Sampling_64_kHz
         3 = Sampling_48_kHz
         4 = Sampling_44_1_kHz
         5 = Sampling_32_kHz
         6 = Sampling_24_kHz
         7 = Sampling_22_05_kHz
         8 = Sampling_16_kHz
         9 = Sampling_12_kHz
        10 = Sampling_11_025_kHz
        11 = Sampling_8_kHz
        12 = RESERVED_12
        13 = RESERVED_13
        14 = RESERVED_14
        15 = RESERVED_15
    */
        uint32_t    ui32SamplingFreq;

    /*
        The number of LFE channel elements.
    */
        uint32_t    ui32NumLfeChannels;

    /*
        The number of audio syntactic elements for the back channels.
    */
        uint32_t    ui32NumBackChannels;

    /*
        The number of audio syntactic elements in the side.
    */
        uint32_t    ui32NumSideChannels;

    /*
    The number of audio syntactic elements in the front channels, front center to back center, symmetrically by left and right, or alternating by left and right in the case of single channel element.
    */
        uint32_t    ui32NumFrontChannels;

    /*
        Output LFE Channel Configuration:
        1 = LFE_ON
        0 = LFE_OFF
    */
        uint32_t    ui32OutputLfeMode;

    /*
        Ouput channel Configuration:
        0 = Two_mono_channels_1_1_ch1_ch2
        1 = One_centre_channel_1_0_C
        2 = Two_channels_2_0_L__R
        3 = Three_channels_3_0_L_C_R
        4 = Three_chanels_2_1_L_R_S
        5 = Four_channels_3_1_L_C_R_S
        6 = Four_channels_2_2_L_R_SL_SR
        7 = Five_channels_3_2_L_C_R_SL_SR
    */
        uint32_t    ui32OutputMode;

    /*
        Low Frequency Effects Channel:
        0 = LFE_channel_off
        1 = LFE_channel_ON
    */
        uint32_t    ui32LfeOn;

    /*
        Audio coding mode which describes the number of input channels.
        0 = Two_mono_channels_1_1_ch1_ch2
        1 = One_centre_channel_1_0_C
        2 = Two_channels_2_0_L__R
        3 = Three_channels_3_0_L_C_R
        4 = Three_chanels_2_1_L_R_S
        5 = Four_channels_3_1_L_C_R_S
        6 = Four_channels_2_2_L_R_SL_SR
        7 = Five_channels_3_2_L_C_R_SL_SR
    */
        uint32_t    ui32AcmodValue;

    /*
        A 1 bit field indicating that pseudo surround decoding is possible.
        0 = Disabled
        1 = Enabled
    */
        uint32_t    ui32PseudoSurroundEnable;

    /*
    A 2 bit field that indicates the coefficient to be used in 5 to 2 matrix mixdown.
    */
        uint32_t    ui32MatrixMixdownIndex;

    /*
    One bit indicating that a stereo matrix coefficient is present in the stream.
        0 = Mixdown_Absent
        1 = Mixdown_Present
    */
        uint32_t    ui32MatrixMixdownPresent;

    /*
    This is a one bit flag that indicates whether DRC is present in the stream.
        0 = DRC_Absent
        1 = DRC_Present
    */
        uint32_t    ui32DrcPresent;

    /* Total number of Pulse IDs found in the bitstream till the current frame */
        uint32_t    ui32DolbyPulseIdCnt;

    /*
        This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for every frame. Dummy samples are not included.
    */
        uint32_t    ui32NumSamplesDecoded;

    /*
        This is the cumulative count for number of Valid frames decoded.
    */
        uint32_t    ui32TotalFramesDecoded;

    /*
        This is the cumulative count for number of Dummy decodes done by decoder.
    */
        uint32_t    ui32TotalFramesDummy;

    /*
    This is the cumulative count for number of erroneous frames detected by decoder.
    */
        uint32_t    ui32TotalFramesInError;

    /*
    Dialouge Normalization information extracted from bit stream.
    Dialnorm value in steps of 0.25 dB. Range: 0-127 (0 to -31.75dB).
    */
    uint32_t    ui32CurrentDialNorm;

    /*
    Dialouge Normalization information extracted from bit stream.
    Dialnorm value in steps of 0.25 dB. Range: 0-127 (0 to -31.75dB).
    */
    uint32_t    ui32PreviousDialNorm;
    /*
    This field tells whether the stream info fields are valid or not.
    */
            uint32_t    ui32StatusValid;

} BDSP_Raaga_Audio_DolbyAacheStreamInfo;

typedef struct BDSP_Raaga_Audio_AmrwbplusStreamInfo
{

    uint32_t    ui32BitRate;

/*
    The sampling frequency of the stream, 8kHz always.
*/
    uint32_t    ui32SamplingFreq;

/*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C, always.
*/
    uint32_t    ui32AcmodValue;

/*
    Ouput channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/  uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;

/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;

/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;

/*
    This field tells whether the stream info fields are valid or not.
*/
    uint32_t    ui32StatusValid;

} BDSP_Raaga_Audio_AmrwbplusStreamInfo;
/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     DRA decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_DraStreamInfo
{
    uint32_t ui32SamplingFrequency;
        /*
        The sampling frequency value as defined by DRA Spec. v1.0
        0 = Sampling_8_kHz
        1 = Sampling_11_025_kHz
        2 = Sampling_12_kHz
        3 = Sampling_16_kHz
        4 = Sampling_22_05_kHz
        5 = Sampling_24_kHz
        6 = Sampling_32_kHz
        7 = Sampling_44_1_kHz
        8 = Sampling_48_kHz
        9 = Sampling_88_2_kHz
        10 = Sampling_96_kHz
        11 = Sampling_176_4_kHz
        12 = Sampling_192_kHz
        13 = RESERVED_13
        14 = RESERVED_14
        15 = RESERVED_15
        */

    uint32_t ui32FrameSize;
        /*
        This field contains the total frame size in bytes of the current frame.
        Max frame size is 4096 bytes
        */

    uint32_t ui32NumBlocks;
        /*
        No. of short window mdct blocks in the frame. Num samples = 128 * ui32NumBlocks = 1024.
        Hence ui32NumBlocks has a value of 8
        */

    uint32_t ui32AcmodValue;
        /*
        Input primary channels configuration
        1 = Cf
        2 = Lf, Rf
        3 = Lf, Rf, Cr
        4 = Lf, Rf, Lr, Rr
        5 = Lf, Rf, Lr, Rr, Cf
        6 = Lf, Rf, Lr, Rr, Cr, Cf
        7 = Lf, Rf, Lr, Rr, Ls, Rs, Cf
        8 = Lf, Rf, Lr, Rr, Ls, Rs, Cr, Cf
        */

    uint32_t ui32LFEOn;
        /*
        Input LFE present
        0 - LFE Not Present
        1 - LFE Present
        */

    uint32_t ui32OutputMode;
        /*
        User defined Output configuration
        0 - Mono
        1 - Stereo, LoRo
        2 - Stereo, LtRt
        3 - 5.1
        4 - Invalid
        */

    uint32_t ui32OutputLFEMode;
        /*
        User prompted LFE present on Output
        0 - LFE not required on OutPut
        1 - LFE required on Output
        */

    uint32_t ui32ErrorStatus;
        /*
        Error status as sent by the decoder
        0 - Frame is sane
        1 - Frame is corrupt. O/P has been zero filled
        */

    uint32_t     ui32NumSamplesDecoded;
        /*
        This is the cumulative number of samples decoded by any decoder from the
        channel open time. Decoder should accumulate number of samples decoded for
        every frame. Dummy samples are not included.
        */

    uint32_t      ui32TotalFramesDecoded;
        /*
            This is the cumulative count for number of valid frames decoded
        */

    uint32_t  ui32TotalFramesDummy;
        /*
            This is the cumulative count for number of Dummy decodes done by decoder
        */

    uint32_t  ui32TotalFramesInError;
        /*
            This is the cumulative count for number of erroneous frames detected by decoder.
        */

    uint32_t  ui32StatusValid;
        /* This field tells whether the stream info fields are valid or not */

    uint32_t    ui32BitRate;

}BDSP_Raaga_Audio_DraStreamInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     Real Audio LBR decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_RalbrStreamInfo
{
    uint32_t    ui32SamplingFrequency;
        /*  The sampling frequency value, 0 in case of error,
            otherwise sampling frequency as it's
        */

    uint32_t    ui32FrameSize;
        /*  This field contains the total frame size
            in bytes of the current frame.
        */

    uint32_t    ui32StreamAcmod;
        /*  Acmod of input stream,
            possible value [0: Error,
                            1: C,
                            2: (Lf, Rf)
                           ]
        */

    uint32_t    ui32UserOutputMode;
        /*  User defined Output configuration, same as eBufferOutputMode
            Syntex according to BCOM_eACMOD

                    0, Invalid
                    1,  Centre (C)
                    2,  L,R -->To be used for BufferOutputMode only.
                    3,  L,C,R
                    4,  L,R,S
                    5,  L,C,R,S
                    6,  L,R,LS,RS
                    7,  L,C,R,LS,RS
        */

    uint32_t    ui32ErrorStatus;
        /*  Error status as sent by the decoder,
            0 - Frame is sane
            1 - Frame is corrupt. O/P has been Interpolated based on prev values
        */

    uint32_t    ui32NumSamplesDecoded;
        /*  This is the cumulative number of samples decoded by any decoder from the channel open time.
            Decoder should accumulate number of samples decoded for every frame. Dummy samples are not included.
        */

    uint32_t    ui32TotalFramesDecoded;
        /*  This is the cumulative count for number of valid frames decoded
        */

    uint32_t    ui32TotalFramesDummy;
        /*  This is the cumulative count for number of Dummy decodes done by decoder
        */

    uint32_t    ui32TotalFramesInError;
        /*  This is the cumulative count for number of erroneous frames detected by decoder.
        */

    uint32_t    ui32StatusValid;
        /*  This field tells whether the stream info fields are valid or not range according to BCOM_eSUCCESS_FAIL
            eFAIL => invalid, eSUCCESS => valid
        */

    uint32_t    ui32BitRate;
       /*  Bit Rate of the input stream*/

}BDSP_Raaga_Audio_RalbrStreamInfo;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     Dolby Pulse decoder which is required by PI at every frame
     boundary.

Description:

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_DolbyPulseStreamInfo
{
    /*
        The bitrate value as defined by the ISO 11172-3 audio specification.
    */
    uint32_t    ui32BitRate;

    /*
        AAC Profile:
        0 = Main_profile
        1 = Low_complexity_profile
        2 = Scaleable_Sampling_Rate_profile
        3 = RESERVED
    */
        uint32_t    ui32Profile;

    /*
    The sampling frequency value as defined by the ISO 11172-3 audio specification:
         0 = Sampling_96_kHz
         1 = Sampling_88_2_kHz
         2 = Sampling_64_kHz
         3 = Sampling_48_kHz
         4 = Sampling_44_1_kHz
         5 = Sampling_32_kHz
         6 = Sampling_24_kHz
         7 = Sampling_22_05_kHz
         8 = Sampling_16_kHz
         9 = Sampling_12_kHz
        10 = Sampling_11_025_kHz
        11 = Sampling_8_kHz
        12 = RESERVED_12
        13 = RESERVED_13
        14 = RESERVED_14
        15 = RESERVED_15
    */
            uint32_t    ui32SamplingFreq;

    /*
        The number of LFE channel elements.
    */
        uint32_t    ui32NumLfeChannels;

    /*
        The number of audio syntactic elements for the back channels.
    */
        uint32_t    ui32NumBackChannels;

    /*
        The number of audio syntactic elements in the side.
    */
        uint32_t    ui32NumSideChannels;

    /*
    The number of audio syntactic elements in the front channels, front center to back center, symmetrically by left and right, or alternating by left and right in the case of single channel element.
    */
        uint32_t    ui32NumFrontChannels;

    /*
        Output LFE Channel Configuration:
        1 = LFE_ON
        0 = LFE_OFF
    */
        uint32_t    ui32OutputLfeMode;

    /*
        Ouput channel Configuration:
        0 = Two_mono_channels_1_1_ch1_ch2
        1 = One_centre_channel_1_0_C
        2 = Two_channels_2_0_L__R
        3 = Three_channels_3_0_L_C_R
        4 = Three_chanels_2_1_L_R_S
        5 = Four_channels_3_1_L_C_R_S
        6 = Four_channels_2_2_L_R_SL_SR
        7 = Five_channels_3_2_L_C_R_SL_SR
    */
        uint32_t    ui32OutputMode;

    /*
        Low Frequency Effects Channel:
        0 = LFE_channel_off
        1 = LFE_channel_ON
    */
        uint32_t    ui32LfeOn;

    /*
        Audio coding mode which describes the number of input channels.
        0 = Two_mono_channels_1_1_ch1_ch2
        1 = One_centre_channel_1_0_C
        2 = Two_channels_2_0_L__R
        3 = Three_channels_3_0_L_C_R
        4 = Three_chanels_2_1_L_R_S
        5 = Four_channels_3_1_L_C_R_S
        6 = Four_channels_2_2_L_R_SL_SR
        7 = Five_channels_3_2_L_C_R_SL_SR
    */
        uint32_t    ui32AcmodValue;

    /*
        A 1 bit field indicating that pseudo surround decoding is possible.
        0 = Disabled
        1 = Enabled
    */
        uint32_t    ui32PseudoSurroundEnable;

    /*
    A 2 bit field that indicates the coefficient to be used in 5 to 2 matrix mixdown.
    */
        uint32_t    ui32MatrixMixdownIndex;

    /*
    One bit indicating that a stereo matrix coefficient is present in the stream.
        0 = Mixdown_Absent
        1 = Mixdown_Present
    */
        uint32_t    ui32MatrixMixdownPresent;

    /*
    This is a one bit flag that indicates whether DRC is present in the stream.
        0 = DRC_Absent
        1 = DRC_Present
    */
        uint32_t    ui32DrcPresent;

    /* Total number of Pulse IDs found in the bitstream till the current frame */
        uint32_t    ui32DolbyPulseIdCnt;

    /*
        This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for every frame. Dummy samples are not included.
    */
        uint32_t    ui32NumSamplesDecoded;

    /*
        This is the cumulative count for number of Valid frames decoded.
    */
        uint32_t    ui32TotalFramesDecoded;

    /*
        This is the cumulative count for number of Dummy decodes done by decoder.
    */
        uint32_t    ui32TotalFramesDummy;

    /*
    This is the cumulative count for number of erroneous frames detected by decoder.
    */
        uint32_t    ui32TotalFramesInError;

    /*
    Dialouge Normalization information extracted from bit stream.
    Dialnorm value in steps of 0.25 dB. Range: 0-127 (0 to -31.75dB).
    */
    uint32_t    ui32CurrentDialNorm;

    /*
    Dialouge Normalization information extracted from bit stream.
    Dialnorm value in steps of 0.25 dB. Range: 0-127 (0 to -31.75dB).
    */
    uint32_t    ui32PreviousDialNorm;
    /*
    This field tells whether the stream info fields are valid or not.
    */
            uint32_t    ui32StatusValid;

} BDSP_Raaga_Audio_DolbyPulseStreamInfo;


/*********************************************************************
Summary:
    This structure defines all the stream information parameters of
    DDP MS10 decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/

typedef struct BDSP_Raaga_Audio_DDPMS10StreamInfo
{

/*
    This field decides the sampling frequency of the AC-3 stream.
    0 = Sampling_48_kHz
    1 = Sampling_44_1_kHz
    2 = Sampling_32_kHz
*/
    uint32_t    ui32SamplingFrequency;
/*
    The bit stream identification value. Bit stream identification
    value in this standard is 8. The future modifications of this
    standard may define other values.
*/
    uint32_t    ui32BitStreamIdentification;
/*
    Transmission mode
    0 = main_audio_service_complete_main_CM
    1 = main_audio_service_music_and_effects_ME
    2 = associated_service_visually_impaired_VI
    3 = associated_service_hearing_impaired_HI
    4 = associated_service_dialogue_D
    5 = associated_service_commentary_C
    6 = associated_service_emergency_E
    7 = associated_service_stream_identification_along_with_ACMOD_value
*/
    uint32_t    ui32BsmodValue;
/*
    Audio coding mode which describes the number of input channels.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32AcmodValue;
/*
    Dolby Surround Mode (dsurmod)
    0 = Not_indicated
    1 = NOT_Dolby_Surround_encoded
    2 = Dolby_Surround_encoded
*/
    uint32_t    ui32DsurmodValue;
/*
    Low Frequency Effects Channel
    0 = LFE_channel_off
    1 = LFE_channel_ON
*/
    uint32_t    ui32LfeOn;
/*
    This field indicates whether the AC-3 language code exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32LangCodeExists;
    /* This field indicates the language code value. */
    uint32_t    ui32LangCodeValue;
/*
    This field indicates the room type value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomTypeValue;
/*
    This field indicates the room type2 value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomType2Value;
/*
    This field indicates that stream is protected by cpyright.
    0 = stream_not_cpyright
    1 = stream_cpyright
*/
    uint32_t    ui32CopyrightBit;
/*
    This field indicates the stream is original bit stream.
    0 = Stream_not_original
    1 = Stream_original
*/
    uint32_t    ui32OriginalBitStream;

/*
    AC-3 centre mix level value
    0 = three_dB_below
    1 = four_and_half_dB_below
    2 = six_dB_below
    3 = RESERVED
*/
    uint32_t    ui32CmixLevel;
/*
    AC-3 surround mix level value
    0 = three_dB_below
    1 = six_dB_below
    2 = zero_dB_below
    3 = RESERVED
*/
    uint32_t    ui32SurmixLevel;
/*
    This field indicates whether the AC-3 language code for the second channel exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32Langcode2Exists;
/*
    This field indicates the language code value for the second channel.
*/
    uint32_t    ui32Langcode2Value;
/*
    This field indicates whether the time code exits or not.
    0 = Not_present
    1 = First_half_present
    2 = Second_half_present
    3 = Both_halves_present
*/
    uint32_t    ui32TimeCodeExists;
/*
This is time code for low resolution in 8 seconds increments up to 24 hours. First 5 bits
indicates the time in hours. Next 6 bits represents the time in minutes. Final 3 bits
represents the time in 8 seconds increments.
*/
    uint32_t    ui32TimeCodeFirstHalf;
/*
    Output LFE Channel Configuration
    1 = LFE_ON
    0 = LFE_OFF
*/
    uint32_t    ui32OutputLfeMode;
/*
    Ouput channel Configuration
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;
/*
    The frame size code is used in conjunction with the sample rate code to determine
    the number of 16-bit words before the next syncword.
*/
    uint32_t    ui32FrmSizeCod;
/*
    This is time code for high resolution in 1/64th frame increments up to 8 seconds.
    The first 3 bits indicate the time in seconds. The next 5 bits represent the time
    in frames. The final 6 bits represent the time in fractions of 1/64th of a frame.
*/
    uint32_t    ui32TimeCodeSecondHalf;
/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/

    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/

    uint32_t    ui32TotalFramesInError;

/* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

}BDSP_Raaga_Audio_DDPMS10StreamInfo;

/*********************************************************************
Summary:
    This structure defines all the stream information parameters of
    DTSHD Broadcast  Decoder which is required by PI at every frame boundary.
Description:
See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_DtshdFrameInfo
{
    uint32_t    ui32NumOfChannels;
    uint32_t    ui32SampleRate;
    /*
        3   :   32 Khz
        8   :   44.1 KHz
        13  :   48Khz
        12  :   24Khz
        11  :   12Khz
    */
    uint32_t    ui32ChannelAllocation;
    uint32_t    ui32LFEPresent;
    uint32_t    ui32BitsPerSample;
    uint32_t    ui32ErrorStatus;
    uint32_t    ui32CoreExtensionMask;
    uint32_t    ui32DTSNeoEnable;
    uint32_t    ui32EsFlag;
    uint32_t    ui32ExtendedCodingFlag;
    uint32_t    ui32ExtensionAudioDescriptorFlag;
    uint32_t    ui32PCMFrameSize;
} BDSP_Raaga_Audio_DtshdFrameInfo;
typedef struct BDSP_Raaga_Audio_DtshdStreamInfo
{
    uint32_t      ui32CrcFlag;
    uint32_t      ui32NBlocks;
    uint32_t      ui32Fsize;
    uint32_t      ui32Amode;
    uint32_t      ui32TransRate;
    uint32_t      ui32DownMix;
    uint32_t      ui32DynRangeCoeff;
    uint32_t      ui32HdcdFormat;
    uint32_t      ui32ExtAudioId;
    uint32_t      ui32ExtAudioFlag;
    uint32_t      ui32VerNum;
    uint32_t      ui32CopyHist;
    uint32_t      ui32PcmResolution;
    BDSP_Raaga_Audio_DtshdFrameInfo  sDtsFrameInfo;
/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/
    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;
    uint32_t        ui32StatusValid;
} BDSP_Raaga_Audio_DtshdStreamInfo;

typedef struct BDSP_Raaga_Audio_MultiStreamDDPStreamInfo
{

/*
    This field decides the sampling frequency of the AC-3 stream.
    0 = Sampling_48_kHz
    1 = Sampling_44_1_kHz
    2 = Sampling_32_kHz
*/
    uint32_t    ui32SamplingFrequency;
/*
    The bit stream identification value. Bit stream identification
    value in this standard is 8. The future modifications of this
    standard may define other values.
*/
    uint32_t    ui32BitStreamIdentification;
/*
    Transmission mode
    0 = main_audio_service_complete_main_CM
    1 = main_audio_service_music_and_effects_ME
    2 = associated_service_visually_impaired_VI
    3 = associated_service_hearing_impaired_HI
    4 = associated_service_dialogue_D
    5 = associated_service_commentary_C
    6 = associated_service_emergency_E
    7 = associated_service_stream_identification_along_with_ACMOD_value
*/
    uint32_t    ui32BsmodValue;
/*
    Audio coding mode which describes the number of input channels.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32AcmodValue;

/*  This Mode represent channel ordering index
        0  - Reserved
        1  - C,
        2  - L, R,
        3  - L, C, R,
        4  - L, R, l,
        5  - L, C, R, l
        6  - L, R, l, r
        7  - L, C, R, l, r
        8  - L, C, R, Cvh
        9  - L, R, l, r, Ts
        10 - L, C, R, l, r, Ts
        11 - L, C, R, l, r, Cvh
        12 - L, C, R, Lc, Rc
        13 - L, R, l, r, Lw, Rw
        14 - L, R, l, r, Lvh, Rvh
        15 - L, R, l, r, Lsd, Rsd
        16 - L, R, l, r, Lrs, Rrs
        17 - L, C, R, l, r, Lc, Rc
        18 - L, C, R, l, r, Lw, Rw
        19 - L, C, R, l, r, Lvh, Rvh
        20 - L, C, R, l, r, Lsd, Rsd
        21 - L, C, R, l, r, Lrs, Rrs
        22 - L, C, R, l, r, Ts, Cvh
        23 - L, R, l, r, Cs
        24 - L, C, R, l, r, Cs
        25 - L, R, l, r, Cs, Ts
        26 - L, C, R, l, r, Cs, Cvh
        27 - L, C, R, l, r, Cs, Ts
*/
    uint32_t    ui32DepFrameChanmapMode;
/*
    Dolby Surround Mode (dsurmod)
    0 = Not_indicated
    1 = NOT_Dolby_Surround_encoded
    2 = Dolby_Surround_encoded
*/
    uint32_t    ui32DsurmodValue;
/*
    Low Frequency Effects Channel
    0 = LFE_channel_off
    1 = LFE_channel_ON
*/
    uint32_t    ui32LfeOn;
/*
    This field indicates whether the AC-3 language code exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32LangCodeExists;
    /* This field indicates the language code value. */
    uint32_t    ui32LangCodeValue;
/*
    This field indicates the room type value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomTypeValue;
/*
    This field indicates the room type2 value.
    0 = Not_indicated
    1 = Large_room
    2 = Small_room
*/
    uint32_t    ui32RoomType2Value;
/*
    This field indicates that stream is protected by cpyright.
    0 = stream_not_cpyright
    1 = stream_cpyright
*/
    uint32_t    ui32CopyrightBit;
/*
    This field indicates the stream is original bit stream.
    0 = Stream_not_original
    1 = Stream_original
*/
    uint32_t    ui32OriginalBitStream;

/*
    AC-3 centre mix level value
    0 = three_dB_below
    1 = four_and_half_dB_below
    2 = six_dB_below
    3 = RESERVED
*/
    uint32_t    ui32CmixLevel;
/*
    AC-3 surround mix level value
    0 = three_dB_below
    1 = six_dB_below
    2 = zero_dB_below
    3 = RESERVED
*/
    uint32_t    ui32SurmixLevel;
/*
    This field indicates whether the AC-3 language code for the second channel exists or not.
    0 = Langauge_code_not_exists
    1 = Language_code_exists
*/
    uint32_t    ui32Langcode2Exists;
/*
    This field indicates the language code value for the second channel.
*/
    uint32_t    ui32Langcode2Value;
/*
    This field indicates whether the time code exits or not.
    0 = Not_present
    1 = First_half_present
    2 = Second_half_present
    3 = Both_halves_present
*/
    uint32_t    ui32TimeCodeExists;
/*
This is time code for low resolution in 8 seconds increments up to 24 hours. First 5 bits
indicates the time in hours. Next 6 bits represents the time in minutes. Final 3 bits
represents the time in 8 seconds increments.
*/
    uint32_t    ui32TimeCodeFirstHalf;
/*
    Output LFE Channel Configuration
    1 = LFE_ON
    0 = LFE_OFF
*/
    uint32_t    ui32OutputLfeMode;
/*
    Ouput channel Configuration
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;
/*
    The frame size code is used in conjunction with the sample rate code to determine
    the number of 16-bit words before the next syncword.
*/
    uint32_t    ui32FrmSizeCod;
/*
    This is time code for high resolution in 1/64th frame increments up to 8 seconds.
    The first 3 bits indicate the time in seconds. The next 5 bits represent the time
    in frames. The final 6 bits represent the time in fractions of 1/64th of a frame.
*/
    uint32_t    ui32TimeCodeSecondHalf;
/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder
*/

    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of erroneous frames detected by decoder.
*/

    uint32_t    ui32TotalFramesInError;

    uint32_t    ui32NumIndepStreamPresent;
    uint32_t    ui32IndepStreamId[8];
    /*
    Dialouge Normalization information extracted from bit stream.
    Possible range 0 to 31 which corresponds to 0 to -31 dB level.
    */
    uint32_t    ui32CurrentDialNorm;

    /*
    Dialouge Normalization information extracted from bit stream.
    Possible range 0 to 31 which corresponds to 0 to -31 dB level.
    */
    uint32_t    ui32PreviousDialNorm;
/* This field tells whether the stream info fields are valid or not */
    uint32_t    ui32StatusValid;

/*
   This field tells us the bit rate of the input stream
*/
    uint32_t   ui32BitRate;

}BDSP_Raaga_Audio_MultiStreamDDPStreamInfo;



/*********************************************************************
Summary:
     This structure defines all the stream information parameters of
     GenCdbItb which is required by PI
     boundary.

Description:ui32StatusValid-Flag to show init.
            ui32ErrorFrames-Number of frames in error
            ui32NumFrames-Number of frames successfully encoded

See Also:
**********************************************************************/
typedef struct BDSP_Raaga_Audio_GenCdbItbStreamInfo
{
    uint32_t ui32StatusValid;
    uint32_t ui32ErrorFrames;
    uint32_t ui32NumFrames;
    uint32_t ui32DroppedOverflowFrames;
    uint32_t ui32DecodePTS;
    uint32_t ui32EncodePTSLower;
    uint32_t ui32EncodePTSUpper;
    uint32_t ui32ESCR;
    uint32_t ui32STCLower;/*This field is invalid for NRT transcode*/
    uint32_t ui32STCUpper;/*This field is invalid for NRT transcode*/
}BDSP_Raaga_Audio_GenCdbItbStreamInfo;

typedef struct BDSP_Raaga_Audio_OutputFormatterStatusInfo
{
    uint32_t ui32Dummy;
}BDSP_Raaga_Audio_OutputFormatterStatusInfo;

typedef struct BDSP_Raaga_Audio_VocalPPStatusInfo
{
    uint32_t ui32Dummy;
}BDSP_Raaga_Audio_VocalPPStatusInfo;

typedef struct BDSP_Raaga_Audio_G726StreamInfo
{
    /*
    Stream type.
    0 = G.726
    1 = G.711
    */
    uint32_t ui32StreamType;
    /*
    The bitrate of the stream.
    0 = 64kbps ----- G.711
    2 = 16kbps --|
    3 = 24kbps   |__ G.726
    4 = 32kbps   |
    5 = 40kbps --|
    */
    uint32_t ui32BitRate;
    /*
    The sampling frequency of the stream, 8kHz always.
    */
    uint32_t ui32SamplingFreq;
    /*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C, always.
    */
    uint32_t ui32AcmodValue;
    /*
    Output channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
    */
    uint32_t ui32OutputMode;
    /*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded
    for every frame. Dummy samples are not included.
    */
    uint32_t ui32NumSamplesDecoded;
    /*
    This is the cumulative count for number of Valid frames decoded.
    */
    uint32_t ui32TotalFramesDecoded;
    /*
    This is the cumulative count for number of Dummy decodes done by decoder.
    */
    uint32_t ui32TotalFramesDummy;
    /*
    This is the cumulative count for number of Erroneous frames detected by
    decoder.
    */
    uint32_t ui32TotalFramesInError;
    /*
    This field tells whether the stream info fields are valid or not.
    */
    uint32_t ui32StatusValid;
} BDSP_Raaga_Audio_G726StreamInfo;


/*Stream info structure defined here till cit support gets checked in*/
typedef struct BDSP_Raaga_Audio_G729DecStreamInfo
{
/*
    Frame Type
    0 = 8kbps
    1 = 6.4kbps
    2 = 11.8kbps
    3 = SID Frame
    4 = Not transmitted frame

*/
    uint32_t    ui32BitRate;

/*
    The sampling frequency of the stream, 8kHz always.
*/
    uint32_t    ui32SamplingFreq;

/*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C, always.
*/
    uint32_t    ui32AcmodValue;

/*
    Ouput channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;

/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;

/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;

/*
    This field tells whether the stream info fields are valid or not.
*/
    uint32_t    ui32StatusValid;

}BDSP_Raaga_Audio_G729DecStreamInfo;


typedef struct BDSP_Raaga_Audio_VorbisDecStreamInfo /** Vorbis header info for file-based formats */
{
    uint32_t    ui32BitRate;
/*
    The sampling frequency of the stream.
*/
    uint32_t    ui32SamplingFreq;
/*
    Audio coding mode which describes the number of input channels.
*/
    uint32_t    ui32AcmodValue;
/*
    Ouput channel configuration.
    1 = One_centre_channel_C
    2 = Two_channels_L_R
    3 = Three_channels_L_C_R
    4 = Four_chanels_L_R_Ls_Rs
    5 = Five_channels_L_C_R_Ls_Rs
    6 = Six_channels_L_C_R_Ls_Rs_LFE
    7 = Seven_channels_L_C_R_Ls_Rs_Cb_LFE
    8 = Eight_channels_L_C_R_Ls_Rs_Lb_Rb_LFE
*/
    uint32_t    ui32OutputMode;
/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;
/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;
/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;
/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;
/*
    This field tells whether the stream info fields are valid or not.
*/
        uint32_t    ui32StatusValid;

} BDSP_Raaga_Audio_VorbisDecStreamInfo;


typedef struct BDSP_Raaga_Audio_FlacDecStreamInfo /* Flac header info for file-based formats */
{
    /* Sample rate in Hz*/
    uint32_t     ui32SamplingFreq;

    /* ACMOD

      1 channel: mono
      2 channels: left, right
      3 channels: left, right, center
      4 channels: left, right, back left, back right
      5 channels: left, right, center, back/surround left, back/surround right
      6 channels: left, right, center, LFE, back/surround left, back/surround right
    */
    uint32_t        ui32Acmod;
    /*
        Low Frequency Effects Channel
        0 = LFE_channel_off
        1 = LFE_channel_ON
    */
    uint32_t        ui32LfeOn;

    uint32_t        ui32OutputMode;

    uint32_t        ui32NumSamplesDecoded;

    /*This is the cumulative count for number of Valid frames decoded*/

    uint32_t    ui32TotalFramesDecoded;

    uint32_t    ui32TotalFramesDummy;

    uint32_t    ui32TotalFramesInError;

    uint32_t    ui32StatusValid;

    uint32_t    ui32BitRate;

} BDSP_Raaga_Audio_FlacDecStreamInfo;


typedef struct BDSP_Raaga_Audio_MacDecStreamInfo /* Mac header info for file-based formats */
{
    uint32_t    ui32FrameLength;
    /*
    Frame length ranges from 64 - 512
    */
    uint32_t    ui32TotalNoOfFrames;
    /*
    Total number of frames present in file.
    */
    uint32_t    ui32CurrentFrame;
    /*
    Current frame.
    */
    uint32_t    ui32nblocks;
    /*
    Number pf Blocks reamaining
    */
    uint32_t    ui32TotalNoOfSamples;
    /*
    Total number of samples present in file.
    */
    uint32_t    ui32SamplingFreq;
    /*
    Sampling frequency of the current frame
    */
    uint32_t    ui32ErrorCountCRC;
    /*
    Number of frames with CRC error.
    */
    uint32_t    ui32NumberOfChan;
    /*
    Number of channels in the current frame.
    */
    uint32_t    ui32Stereo;
    /*
    CELT stereo mode.
    */
    uint32_t    ui32BitsPerSample;
    /*
    BitsPerSample.
    */
    uint32_t    ui32TerminatingByte;
    /*
    Terminating Bytes.
    */
    uint32_t    fileversion;
    /*
    File version.
    */
    uint32_t    ui32CompressionType;
    /*
    CompressionType.
    */
    uint32_t    ui32BitRatePerChan;
    /*
    Bit Rate Per Channel.
    */
    uint32_t    ui32TotalFramesDecoded;
    /*
    Cumulative count for number of Valid frames decoded
    */
    uint32_t    ui32TotalBlocksDecoded;
    /*
    Cumulative count for number of Valid blocks decoded
    */
    uint32_t    ui32TotalFramesDummy;
    /*
    Cumulative count for number of Dummy decodes done by decoder
    */
    uint32_t    ui32TotalFramesInError;
    /*
    Cumulative count for number of erroneous frames detected by decoder.
    */
    uint32_t    ui32TotalBlocksInError;
    /*
    Cumulative count for number of erroneous blocks detected by decoder.
    */
    uint32_t    ui32StatusValid;
    /*
    Status of Erraneous frame.
    0 - SUCCESS
    1 - FAILURE
    */
    uint32_t    ui32BitRate;
    /* Bit Rate of the stream */
} BDSP_Raaga_Audio_MacDecStreamInfo;


typedef struct BDSP_Raaga_Audio_MlpStreamInfo
{
    /* This field gives the information about the number of channels in the stream */
    uint32_t    ui32NumOfChannels;  /* Possible values - 2, 6, 8*/

    /* This field gives the information about the sample rate of the stream */
    uint32_t    ui32SampleRate;     /* 1 = 192kHz, 5 = 96kHz, 13 = 48kHz
                                       3 = 176.4kHz, 7 = 88.2kHz, 15 = 44.1 kHz*/

    /* This field gives the information ACMOD of the stream */
    uint32_t    ui32ChannelAllocation;  /* 2 = STEREO, 7 = 5.1, 21 = 7.1 */

    /* This field indicates whether the stream has an LFE channel */
    uint32_t    ui32LFEPresent; /* 0 - off, 1 - on */

    /* This field gives the bits per sanmple in the decoded stream  */
    uint32_t    ui32BitsPerSample;

    /* This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included. */
    uint32_t    ui32NumSamplesDecoded;

    /* This is the cumulative count for number of Valid frames decoded */
    uint32_t    ui32TotalFramesDecoded;

    /* This is the cumulative count for number of Dummy decodes done by decoder */
    uint32_t    ui32TotalFramesDummy;

    /* This is the cumulative count for number of erroneous frames detected by decoder*/
    uint32_t    ui32TotalFramesInError;

    uint32_t    ui32StatusValid;

    /* Bit Rate of the stream */
    uint32_t    ui32BitRate;

} BDSP_Raaga_Audio_MlpStreamInfo;


typedef struct BDSP_Raaga_Audio_AdpcmStreamInfo
{
    /*
    Stream type.
    0 = MS-ADPCM
    1 = IMA-ADPCM
    */
    uint32_t ui32StreamType;
    /*
    The sampling frequency of the stream.
    */
    uint32_t ui32SamplingFreq;
    /*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    */
    uint32_t ui32AcmodValue;
    /*
    Output channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
    */
    uint32_t ui32OutputMode;
    /*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded
    for every frame. Dummy samples are not included.
    */
    uint32_t ui32NumSamplesDecoded;
    /*
    This is the cumulative count for number of Valid frames decoded.
    */
    uint32_t ui32TotalFramesDecoded;
    /*
    This is the cumulative count for number of Dummy decodes done by decoder.
    */
    uint32_t ui32TotalFramesDummy;
    /*
    This is the cumulative count for number of Erroneous frames detected by
    decoder.
    */
    uint32_t ui32TotalFramesInError;
    /*
    This field tells whether the stream info fields are valid or not.
    */
    uint32_t ui32StatusValid;

    /* Bit Rate of the stream */
    uint32_t    ui32BitRate;
} BDSP_Raaga_Audio_AdpcmStreamInfo;

typedef struct BDSP_Raaga_P_Vp6StreamInfo
{
    /*
    This field tells the Pending fame drop count.
    */
    uint32_t ui32PendingDropCount;
    /*
    This is the cumulative count for number of Valid frames decoded.
    */
    uint32_t ui32TotalFramesDecoded;
    /*
    This is the cumulative count for number of dropped by decoder.
    */
    uint32_t ui32TotalFramesDropped;
    /*
    This is the cumulative count for number of Erroneous frames detected by
    decoder.
    */
    uint32_t ui32TotalFramesInError;
    /*
    This field tells whether the stream info fields are valid or not.
    */
    uint32_t ui32StatusValid;

} BDSP_Raaga_P_Vp6StreamInfo;

typedef struct BDSP_Raaga_Audio_G723_1_StreamInfo
{
/*
    Frame Type
    0 = 6.3kbps
    1 = 5.3kbps
    2 = SID Frame

*/
    uint32_t    ui32BitRate;

/*
    The sampling frequency of the stream, 8kHz always.
*/
    uint32_t    ui32SamplingFreq;


/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;

/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;

/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;

/*
    This field tells whether the stream info fields are valid or not.
*/
    uint32_t    ui32StatusValid;

    uint32_t    ui32FrameSize;

    uint32_t    ui32ChannelAssignment;

    uint32_t    ui32BitsPerSample;

    uint32_t    ui32OutMode;

    uint32_t    ui32OutputLfeMode;

    uint32_t    ui32AcmodValue;

    uint32_t    ui32OpNumOfSamples;

    uint32_t    ui32IpNumOfSamples;

}BDSP_Raaga_Audio_G723_1_StreamInfo;

typedef struct BDSP_Raaga_Audio_OpusDecStreamInfo
{

/*
    BCOM_eWVFMTX_TYPES_eOpus = 0x504
*/
    uint32_t ui32StreamType;

    /*

    The bitrate of the stream 10kbps - 32kbps .

    */
    uint32_t    ui32BitRate;

/*
    The sampling frequency of the stream, 8kHz and 16khz.
*/
    uint32_t    ui32SamplingFreq;

/*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C, always.
*/
    uint32_t    ui32AcmodValue;

/*
    Ouput channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;

/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;

/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;

/*
    This field tells whether the stream info fields are valid or not.
*/
    uint32_t    ui32StatusValid;

}BDSP_Raaga_Audio_OpusDecStreamInfo;

typedef struct BDSP_Raaga_Audio_ALSDecStreamInfo
{


/*
    BCOM_eWVFMTX_TYPES_eALS = 0x505
*/
    uint32_t ui32StreamType;

    /*

    The bitrate of the stream 10kbps - 32kbps .

    */
    uint32_t    ui32BitRate;

/*
    The sampling frequency of the stream, 16kHz to 48khz.
*/
    uint32_t    ui32SamplingFreq;

/*
    Audio coding mode which describes the number of input channels.
    1 = One_centre_channel_1_0_C, always.
*/
    uint32_t    ui32AcmodValue;

/*
    Ouput channel configuration.
    0 = Two_mono_channels_1_1_ch1_ch2
    1 = One_centre_channel_1_0_C
    2 = Two_channels_2_0_L__R
    3 = Three_channels_3_0_L_C_R
    4 = Three_chanels_2_1_L_R_S
    5 = Four_channels_3_1_L_C_R_S
    6 = Four_channels_2_2_L_R_SL_SR
    7 = Five_channels_3_2_L_C_R_SL_SR
*/
    uint32_t    ui32OutputMode;

/*
    This is the cumulative number of samples decoded by any decoder from the
    channel open time. Decoder should accumulate number of samples decoded for
    every frame. Dummy samples are not included.
*/
    uint32_t    ui32NumSamplesDecoded;

/*
    This is the cumulative count for number of Valid frames decoded.
*/
    uint32_t    ui32TotalFramesDecoded;

/*
    This is the cumulative count for number of Dummy decodes done by decoder.
*/
    uint32_t    ui32TotalFramesDummy;

/*
    This is the cumulative count for number of Erroneous frames detected by decoder.
*/
    uint32_t    ui32TotalFramesInError;

/*
    This field tells whether the stream info fields are valid or not.
*/
    uint32_t    ui32StatusValid;

/*
    This field tells the input sample bit width. Used only inside MPEG-4 ALS decoder.
*/

    uint32_t    bit_width;

}BDSP_Raaga_Audio_ALSDecStreamInfo;


typedef struct BDSP_Raaga_Audio_AvlPPStatusInfo
{
    /*  No of channels processed */
    uint32_t   ui32NumChannels;

    /* This field represents the peak power calculated channel wise in DBr(dB-relative) scale. Supports Max 6 channels.
    * 0 dBr results mean the the max power which will be the case for full scale signal. The results are expected to be
    * negative always which indicate that level is below 0 dBr. The peak power is calculated and converted to DBr scale as
    * Peak power in dBr = 20*log (MAX (ABS(X1), ABS(X2), ., ABS (XN))/ MAX32), N:Number of samples in the frame. Log is base 10
    * The channel indices in case of stereo and 5.1 will be as follows
    * Stereo: L=0, R=1
    * 5.1 Multichannel: L=0, R=1, Ls=2, Rs=3, C=4, LFE=5 */
    int32_t    i32PeakPowerinDB[6];

    /*This field represents the peak power calculated channel wise in DBr(dB-relative) scale. Supports Max 6 channels.
    * 0 dBr results mean the the max power which will be the case for full scale signal. The results are expected to be negative
    * always which indicate that level is below 0 dBr. The power is calculated and converted to DBr scale as
    * RMS power in dBr = 20*log (sqrt (sum Square(X0, X1, X2XN)/N)/ MAX32 ), N:Number of samples in the frame. Log is base 10
    * The channel indices in case of stereo and 5.1 will be as follows
    * Stereo: L=0, R=1
    * 5.1 Multichannel: L=0, R=1, Ls=2, Rs=3, C=4, LFE=5 */
    int32_t    i32RMSPowerinDB[6];

    /* This is time snapshot of unadulterated 45KHz timer */
    uint32_t   ui32TimeSnapshot45KHz;

    /* ui32StatusValid=1 indicates status as "valid"
    ui32StatusValid= other than 1 indicates status as "in-valid" */
    uint32_t                        ui32StatusValid;

}BDSP_Raaga_Audio_AvlPPStatusInfo;


typedef struct BDSP_Raaga_Audio_FadeCtrlPPStatusInfo
{
    /* ui32FadeActiveStatus=0 indicates status as "Target Reached"
    ui32FadeActiveStatus=1 indicates status as "Fade Control in Active and Target is not yet reached" */
    uint32_t                        ui32FadeActiveStatus;

    /* ui32RemainingDuration represent remaining duration in current active fade.
    if fade is not active, ui32RemainingDuration will be reported as "0"
    Duration is specified in miliseconds */
    uint32_t                        ui32RemainingDuration;

    /* ui32CurrentVolumeLevel represent percentage current volume level.
    0 - mute and 100 - full scale. */
    uint32_t                        ui32CurrentVolumeLevel;

   /* ui32StatusValid=0 indicates status as "valid"
    ui32StatusValid=0x7fffffff indicates status as "in-valid" */
    uint32_t                        ui32StatusValid;
}BDSP_Raaga_Audio_FadeCtrlPPStatusInfo;


typedef struct BDSP_Raaga_Audio_AmbisonicsPPStatusInfo
{
    /* ui32BinauralRendered - boolean flag indicating if the audio has been binaurally rendered
    0 - False and 1 - True. */
    uint32_t                        ui32BinauralRendered;

   /* ui32StatusValid=0 indicates status as "valid"
    ui32StatusValid=0x7fffffff indicates status as "in-valid" */
    uint32_t                        ui32StatusValid;
}BDSP_Raaga_Audio_AmbisonicsPPStatusInfo;



#define MAX_NUM_FADE_NODES                         5

typedef struct BDSP_Raaga_Audio_Mixing_FadeCtrl_Info
{
    /* ui32FadeActiveStatus=0 indicates status as "Target Reached"
     ui32FadeActiveStatus=1 indicates status as "Fade Control in Active and Target is not yet reached" */
    uint32_t ui32FadeActiveStatus;

    /* ui32RemainingDuration represent remaining duration in current active fade.
     if fade is not active, ui32RemainingDuration will be reported as "0"
     Duration is specified in miliseconds */
    uint32_t ui32RemainingDuration;

    /* ui32CurrentVolumeLevel represent percentage current volume level.
     0 - mute and 100 - full scale. */
    uint32_t ui32CurrentVolumeLevel;

    /* ui32StatusValid=0 indicates status as "valid"
     ui32StatusValid=0x7fffffff indicates status as "in-valid" */
    uint32_t ui32StatusValid;
} BDSP_Raaga_Audio_Mixing_FadeCtrl_Info;

typedef struct BDSP_Raaga_Audio_TsmCorrectionPPStatusInfo
{
    /* PTS Value as it is from the stream */
    uint32_t                        ui32PTS;

    /* 0 - Invalid and 1 - Valid. Any Other Value - Invalid */
    uint32_t                        ui32PTSValid;

    /* This Field Represents PTS types as 0-Coded, 1-INTERPOLATED, 2-INTERPOLATED from INVALID PTS */
    uint32_t                        ui32PTSType;

    /* This Field Represents the time (in msec with 45 KHz running clock) ajusted.
     * Negative Value represents that the frame is shorten which is possible when PTS is behind STC.*/
    int32_t                         i32TimeInMsecAdjusted;

    /* This is time snapshot of unadulterated 45KHz timer */
    uint32_t                        ui32TimeSnapshot45KHz;

    /* This is diff between STC and PTS represented in msec at 45 KHz clock */
    uint32_t                        ui32StcPtsPhaseDiffInT45;

    /* This represents the STC and PTS position. Possible values: 0 or 1, 0 - (STC > PTS),  1 - (STC < PTS) */
    uint32_t                        ui32StcBehindPtsStatus;

    /* ui32StatusValid=1 indicates status as "valid"
    ui32StatusValid = other than 1 indicates status as "in-valid" */
    uint32_t                        ui32StatusValid;
}BDSP_Raaga_Audio_TsmCorrectionPPStatusInfo;

typedef struct BDSP_Raaga_Audio_MixerDapv2StatusInfo
{

    BDSP_Raaga_Audio_Mixing_FadeCtrl_Info FadeCtrl_Info[MAX_NUM_FADE_NODES];

} BDSP_Raaga_Audio_MixerDapv2StatusInfo;

typedef struct BDSP_Raaga_VideoH264EncoderInfo
{
    uint32_t                        TotalFramesRecvd;
    uint32_t                        TotalFramesEncoded;
    uint32_t                        TotalFramesDropedForFRC;
    uint32_t                        TotalFramesDropedForLipSynch;
    uint32_t                        ui32CdbFullCounter;
    uint32_t                        ui32RelinquishCounter;
    uint32_t                        uiEncodedPTS;
    uint32_t                        ui32StcValue;
    uint32_t                        ui32FrameRateConvError;
    uint32_t                        ui32StatusValid;
}BDSP_Raaga_VideoH264EncoderInfo;

typedef struct BDSP_Raaga_VideoX264EncoderInfo
{
    uint32_t                        TotalFramesRecvd;
    uint32_t                        TotalFramesEncoded;
    uint32_t                        TotalFramesDropedForFRC;
    uint32_t                        TotalFramesDropedForLipSynch;
    uint32_t                        ui32CdbFullCounter;
    uint32_t                        ui32RelinquishCounter;
    uint32_t                        uiEncodedPTS;
    uint32_t                        ui32StcValue;
    uint32_t                        ui32FrameRateConvError;
    uint32_t                        ui32StatusValid;
}BDSP_Raaga_VideoX264EncoderInfo;
typedef struct BDSP_Raaga_VideoXVP8EncoderInfo
{
    uint32_t                        ui32TotalFramesRecvd;
    uint32_t                        ui32TotalFramesEncoded;
    uint32_t                        ui32TotalFramesDropedForFRC;
    uint32_t                        ui32TotalFramesDropedForLipSynch;
    uint32_t                        ui32CdbFullCounter;
    uint32_t                        ui32RelinquishCounter;
    uint32_t                        ui32EncodedPTS;
    uint32_t                        ui32StcValue;
    uint32_t                        ui32FrameRateConvError;
    uint32_t                        ui32StatusValid;
}BDSP_Raaga_VideoXVP8EncoderInfo;
/*********************************************************************
Summary:
    This union  holds all the stream info related field values for all
    audio decoders in a common memory which is required by PI at every
    frame boundary.

Description:

See Also:
**********************************************************************/
typedef union BDSP_Raaga_Audio_StreamInfo
{
    BDSP_Raaga_Audio_Ac3StreamInfo      sAc3StreamInfo;
    BDSP_Raaga_Audio_DdpStreamInfo      sDdpStreamInfo;
    BDSP_Raaga_Audio_MpegStreamInfo     sMpegStreamInfo;
    BDSP_Raaga_Audio_AacStreamInfo      sAacheStreamInfo;
    BDSP_Raaga_Audio_WmaStreamInfo      sWmaStreamInfo;
    BDSP_Raaga_Audio_WmaProStreamInfo   sWmaProStreamInfo;
    BDSP_Raaga_Audio_DtsCoreStreamInfo  sDtsCoreStreamInfo;
    BDSP_Raaga_Audio_LpcmStreamInfo     sLpcmStreamInfo;
    BDSP_Raaga_Audio_DtsHdStreamInfo        sDtsHdStreamInfo;
    BDSP_Raaga_Audio_PcmWavStreamInfo   sPcmWavStreamInfo;
    BDSP_Raaga_Audio_AmrStreamInfo      sAmrStreamInfo;
    BDSP_Raaga_Audio_DraStreamInfo      sDraStreamInfo;
    BDSP_Raaga_Audio_RalbrStreamInfo        sRalbrStreamInfo;
    BDSP_Raaga_Audio_DolbyPulseStreamInfo sDolbyPulseStreamInfo;
    BDSP_Raaga_Audio_DDPMS10StreamInfo   sDDPMS10StreamInfo;
    BDSP_Raaga_Audio_MultiStreamDDPStreamInfo    sMSDDPStreamInfo;
    BDSP_Raaga_Audio_G726StreamInfo      sG726StreamInfo;
    BDSP_Raaga_Audio_AdpcmStreamInfo     sAdpcmStreamInfo;
    BDSP_Raaga_Audio_VorbisDecStreamInfo    sVorbisStreamInfo;
    BDSP_Raaga_P_Vp6StreamInfo       sVp6StreamInfo;
    BDSP_Raaga_Audio_G723_1_StreamInfo sG723_1_StreamInfo;
    BDSP_Raaga_Audio_MlpStreamInfo  sMlpStreamInfo;
    BDSP_Raaga_Audio_AmrWbStreamInfo      sAmrWbStreamInfo;
    BDSP_Raaga_Audio_iLBCStreamInfo      siLBCStreamInfo;
    BDSP_Raaga_Audio_iSACStreamInfo      siSACStreamInfo;
    BDSP_Raaga_Audio_UdcStreamInfo       sUdcStreamInfo;
    BDSP_Raaga_Audio_DolbyAacheStreamInfo sDolbyAacheStreamInfo;
    BDSP_Raaga_Audio_OpusDecStreamInfo    sOpusDecStreamInfo;
    BDSP_Raaga_Audio_ALSDecStreamInfo    sALSDecStreamInfo;
    BDSP_Raaga_Audio_FadeCtrlPPStatusInfo sFadeCtrlProcessInfo;
    BDSP_Raaga_Audio_AmbisonicsPPStatusInfo sAmbisonicsStreamInfo;
    BDSP_Raaga_Audio_TsmCorrectionPPStatusInfo sTsmCorrectionProcessInfo;
    BDSP_Raaga_Audio_MixerDapv2StatusInfo sMixerDapv2ProcessInfo;
    BDSP_Raaga_VideoH264EncoderInfo      sH264EncoderInfo;
    BDSP_Raaga_VideoX264EncoderInfo      sX264EncoderInfo;
    BDSP_Raaga_Audio_PassthruStreamInfo         sPassthruStreamInfo;
    /* Other decoders info */
} BDSP_Raaga_Audio_StreamInfo;

typedef union BDSP_Raaga_Audio_DecodeStageStatus
{
    BDSP_Raaga_Audio_IdsTsmInfo  sIdsTsmInfo;
    BDSP_Raaga_Audio_StreamInfo  sStreamInfo;
}BDSP_Raaga_Audio_DecodeStageStatus;

typedef struct BDSP_Raaga_Audio_EncodeStageStatus
{
    BDSP_Raaga_Audio_DTSBroadcastEncoderStreamInfo sDTSBroadcastEncStreamInfo;
    uint32_t    ui32StatusValid;
} BDSP_Raaga_Audio_EncodeStageStatus;

#endif
