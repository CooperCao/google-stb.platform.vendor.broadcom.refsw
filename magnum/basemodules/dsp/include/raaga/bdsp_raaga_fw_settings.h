/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BRAAGA_FW_USER_CONFIG_H
#define BRAAGA_FW_USER_CONFIG_H

#include "bdsp_common_fw_settings.h"
#include "bdsp_raaga_fw.h"

#define  DDP_DEC_GBL_MAXPCMCHANS                    6
#define  DTSHD_DEC_MAXCHANS                         8

/* BDSP_AudioType */
#define BDSP_Raaga_MpegSettings                     BDSP_Raaga_Audio_MpegConfigParams

#ifdef BDSP_DOLBY_AACHE_SUPPORT
    #define BDSP_Raaga_AacSettings                  BDSP_Raaga_Audio_DolbyAacheUserConfig
    #define BDSP_Raaga_AacAdtsSettings              BDSP_Raaga_Audio_DolbyAacheUserConfig
    #define BDSP_Raaga_AacLoasSettings              BDSP_Raaga_Audio_DolbyAacheUserConfig
    #define BDSP_Raaga_AacSbrAdtsSettings           BDSP_Raaga_Audio_DolbyAacheUserConfig
    #define BDSP_Raaga_AacSbrSettings               BDSP_Raaga_Audio_DolbyAacheUserConfig
    #define BDSP_Raaga_AacSbrLoasSettings           BDSP_Raaga_Audio_DolbyAacheUserConfig
#elif BDSP_MS10_SUPPORT
    #define BDSP_Raaga_AacSettings                  BDSP_Raaga_Audio_DolbyPulseUserConfig
    #define BDSP_Raaga_AacAdtsSettings              BDSP_Raaga_Audio_DolbyPulseUserConfig
    #define BDSP_Raaga_AacLoasSettings              BDSP_Raaga_Audio_DolbyPulseUserConfig
    #define BDSP_Raaga_AacSbrAdtsSettings           BDSP_Raaga_Audio_DolbyPulseUserConfig
    #define BDSP_Raaga_AacSbrSettings               BDSP_Raaga_Audio_DolbyPulseUserConfig
    #define BDSP_Raaga_AacSbrLoasSettings           BDSP_Raaga_Audio_DolbyPulseUserConfig
#else
    #define BDSP_Raaga_AacSettings                  BDSP_Raaga_Audio_AacheConfigParams
    #define BDSP_Raaga_AacAdtsSettings              BDSP_Raaga_Audio_AacheConfigParams
    #define BDSP_Raaga_AacLoasSettings              BDSP_Raaga_Audio_AacheConfigParams
    #define BDSP_Raaga_AacSbrAdtsSettings           BDSP_Raaga_Audio_AacheConfigParams
    #define BDSP_Raaga_AacSbrSettings               BDSP_Raaga_Audio_AacheConfigParams
    #define BDSP_Raaga_AacSbrLoasSettings           BDSP_Raaga_Audio_AacheConfigParams
#endif

#define BDSP_Raaga_Ac3Settings                      BDSP_Raaga_Audio_DDPMultiStreamConfigParams
#define BDSP_Raaga_Ac3PlusSettings                  BDSP_Raaga_Audio_DDPMultiStreamConfigParams
#define BDSP_Raaga_LpcmBdSettings                   BDSP_Raaga_Audio_LpcmUserConfig
#define BDSP_Raaga_LpcmHdDvdSettings                BDSP_Raaga_Audio_LpcmUserConfig
#define BDSP_Raaga_DtshdSettings                    BDSP_Raaga_Audio_DtsHdConfigParams
#define BDSP_Raaga_LpcmDvdSettings                  BDSP_Raaga_Audio_LpcmUserConfig
#define BDSP_Raaga_WmaStdSettings                   BDSP_Raaga_Audio_WmaConfigParams
#define BDSP_Raaga_MlpSettings                      BDSP_Raaga_Audio_MlpConfigParams
#define BDSP_Raaga_PcmSettings                      BDSP_Raaga_Audio_PassthruConfigParams
#define BDSP_Raaga_DtsLbrSettings                   BDSP_Raaga_Audio_DtslbrConfigParams
#define BDSP_Raaga_Ddp7_1Settings                   BDSP_Raaga_Audio_DecodeConfigParams
#define BDSP_Raaga_MpegMcSettings                   BDSP_Raaga_Audio_DecodeConfigParams
#define BDSP_Raaga_WmaProSettings                   BDSP_Raaga_Audio_WmaProConfigParams
#define BDSP_Raaga_LpcmDvdASettings                 BDSP_Raaga_Audio_DecodeConfigParams
#define BDSP_Raaga_DtsBroadcastSettings             BDSP_Raaga_Audio_DtsHdConfigParams
#define BDSP_Raaga_PcmWavSettings                   BDSP_Raaga_Audio_PcmWavConfigParams
#define BDSP_Raaga_AmrSettings                      BDSP_Raaga_Audio_AmrConfigParams
#define BDSP_Raaga_DraSettings                      BDSP_Raaga_Audio_DraConfigParams
#define BDSP_Raaga_RealAudioLbrSettings             BDSP_Raaga_Audio_RalbrConfigParams
#define BDSP_Raaga_AdpcmSettings                    BDSP_Raaga_Audio_AdpcmConfigParams
#define BDSP_Raaga_G711G726Settings                 BDSP_Raaga_Audio_G726ConfigParams
#define BDSP_Raaga_G729Settings                     BDSP_Raaga_Audio_G729DecConfigParams
#define BDSP_Raaga_VorbisSettings                   BDSP_Raaga_Audio_VorbisDecConfigParams
#define BDSP_Raaga_G723_1Settings                   BDSP_Raaga_Audio_G723_1DEC_ConfigParams
#define BDSP_Raaga_FlacSettings                     BDSP_Raaga_Audio_FlacDecConfigParams
#define BDSP_Raaga_MacSettings                      BDSP_Raaga_Audio_MacDecConfigParams
#define BDSP_Raaga_AmrwbSettings                    BDSP_Raaga_Audio_AmrwbdecConfigParams
#define BDSP_Raaga_iLBCSettings                     BDSP_Raaga_Audio_iLBCdecConfigParams
#define BDSP_Raaga_iSACSettings                     BDSP_Raaga_Audio_iSACdecConfigParams
#define BDSP_Raaga_UdcSettings                      BDSP_Raaga_Audio_UdcdecConfigParams
#define BDSP_Raaga_OpusDecSettings                  BDSP_Raaga_Audio_OpusDecConfigParams
#define BDSP_Raaga_ALSDecSettings                   BDSP_Raaga_Audio_ALSDecConfigParams
#define BDSP_Raaga_AC4DecSettings                   BDSP_Raaga_Audio_AC4DecConfigParams

/* BDSP_AudioProcessing */
#define BDSP_Raaga_DtsNeoSettings                   BDSP_Raaga_Audio_ProcessingConfigParams
#define BDSP_Raaga_AVLSettings                      BDSP_Raaga_Audio_AVLConfigParams
#define BDSP_Raaga_BbeSettings                      BDSP_Raaga_Audio_ProcessingConfigParams
#define BDSP_Raaga_SrcSettings                      BDSP_Raaga_Audio_SRCUserConfigParams
#define BDSP_Raaga_CustomVoiceSettings              BDSP_Raaga_Audio_CustomVoiceConfigParams
#define BDSP_Raaga_AacDownmixSettings               BDSP_Raaga_Audio_ProcessingConfigParams
#define BDSP_Raaga_DsolaSettings                    BDSP_Raaga_Audio_DsolaConfigParams
#define BDSP_Raaga_SrsHdSettings                    BDSP_Raaga_Audio_TruSurrndHDConfigParams
#define BDSP_Raaga_GenericPassThruSettings          BDSP_Raaga_Audio_PassthruConfigParams
#define BDSP_Raaga_SrsTruVolumeSettings             BDSP_Raaga_Audio_TruVolumeUserConfig
#define BDSP_Raaga_Brcm3DSurroundSettings           BDSP_Raaga_Audio_Brcm3DSurroundConfigParams
#define BDSP_Raaga_FWMixerSettings                  BDSP_Raaga_Audio_ProcessingConfigParams
#define BDSP_Raaga_DdreSettings                     BDSP_Raaga_Audio_DDReencodeConfigParams
#define BDSP_Raaga_Dv258Settings                    BDSP_Raaga_Audio_DV258ConfigParams
#define BDSP_Raaga_DpcmrSettings                    BDSP_Raaga_Audio_DpcmrConfigParams
#define BDSP_Raaga_GenCdbItbSettings                BDSP_Raaga_Audio_GenCdbItbConfigParams
#define BDSP_Raaga_BtscEncoderSettings              BDSP_Raaga_Audio_BtscEncoderConfigParams
#define BDSP_Raaga_Broadcom3DSurroundSettings       BDSP_Raaga_Audio_Broadcom3DSurroundConfigParams
#define BDSP_Raaga_SpeexAecSettings                 BDSP_Raaga_Audio_SpeexAECConfigParams
#define BDSP_Raaga_KaraokeSettings                  BDSP_Raaga_Audio_KaraokeConfigParams
#define BDSP_Raaga_MixerDapv2Settings               BDSP_Raaga_Audio_MixerDapv2ConfigParams
#define BDSP_Raaga_OutputFormatterSettings          BDSP_Raaga_Audio_OutputFormatterConfigParams
#define BDSP_Raaga_VocalPPSettings                  BDSP_Raaga_Audio_VocalPPConfigParams
#define BDSP_Raaga_FadeCtrlPPSetting                BDSP_Raaga_Audio_FadeCtrlConfigParams
#define BDSP_Raaga_AmbisonicsPPSetting              BDSP_Raaga_Audio_AmbisonicsConfigParams
#define BDSP_Raaga_TsmCorrectionPPSetting           BDSP_Raaga_Audio_TsmCorrectionConfigParams

/* BDSP_AudioEncode */
#define BDSP_Raaga_Mpeg1Layer3Settings              BDSP_Raaga_Audio_Mpeg1L3EncConfigParams
#define BDSP_Raaga_Mpeg1Layer2Settings              BDSP_Raaga_Audio_Mpeg1L2EncConfigParams
#define BDSP_Raaga_DTSEncodeSettings                BDSP_Raaga_Audio_DtsBroadcastEncConfigParams
#define BDSP_Raaga_AacLcSettings                    BDSP_Raaga_Audio_AaclcEncConfigParams
#define BDSP_Raaga_AacHeSettings                    BDSP_Raaga_Audio_AacheEncConfigParams
#define BDSP_Raaga_Ac3EncodeSettings                BDSP_Raaga_Audio_DDTranscodeConfigParams
#define BDSP_Raaga_G711G726EncodeSettings           BDSP_Raaga_Audio_G711_G726EncConfigParams
#define BDSP_Raaga_G729EncodeSettings               BDSP_Raaga_Audio_G729EncoderUserConfig
#define BDSP_Raaga_G723_1EncodeSettings             BDSP_Raaga_Audio_G723EncoderUserConfig
#define BDSP_Raaga_G722EncodeSettings               BDSP_Raaga_Audio_G722EncConfigParams
#define BDSP_Raaga_AmrEncodeSettings                BDSP_Raaga_Audio_AmrEncoderUserConfig
#define BDSP_Raaga_AmrwbEncodeSettings              BDSP_Raaga_Audio_AMRWBConfigParams
#define BDSP_Raaga_ILBCEncodeSettings               BDSP_Raaga_Audio_ILBCConfigParams
#define BDSP_Raaga_ISACEncodeSettings               BDSP_Raaga_Audio_ISACConfigParams
#define BDSP_Raaga_LpcmEncodeSettings               BDSP_Raaga_Audio_LpcmEncConfigParams
#define BDSP_Raaga_OpusEncodeSettings               BDSP_Raaga_Audio_OpusEncConfigParams
#define BDSP_Raaga_DDPEncodeSettings                BDSP_Raaga_Audio_DDPEncConfigParams

/* BDSP_VideoType */
#define BDSP_Raaga_RealVideo9Settings               BDSP_Raaga_Video_DecodeConfigParams
#define BDSP_Raaga_VP6Settings                      BDSP_Raaga_Video_DecodeConfigParams
#define BDSP_Raaga_H264EncodeSettings               BDSP_Raaga_Audio_sBH264_USER_CONFIG
/*
    Common Enum for Acmode
*/

typedef enum BDSP_Raaga_Audio_AcMode
{
    BDSP_Raaga_Audio_AcMode_eMode10 = 1, /*C*/
    BDSP_Raaga_Audio_AcMode_eMode20 = 2, /*L,R*/
    BDSP_Raaga_Audio_AcMode_eMode30 = 3, /*L,C,R*/
    BDSP_Raaga_Audio_AcMode_eMode21 = 4, /*L,R,S*/
    BDSP_Raaga_Audio_AcMode_eMode31 = 5, /*L,C,R,S*/
    BDSP_Raaga_Audio_AcMode_eMode22 = 6, /*L,R,LS,RS*/
    BDSP_Raaga_Audio_AcMode_eMode32 = 7, /*L,C,R,LS,RS*/
    BDSP_Raaga_Audio_AcMode_eMode33 = 8, /*L,C,R,LS,RS,CS*/
    BDSP_Raaga_Audio_AcMode_eMode32_BSDigital = 9,/*L,C,R,LS,RS*/
    BDSP_Raaga_Audio_AcMode_eModeLtRt=0x20,/*L,R*/
    BDSP_Raaga_Audio_AcMode_eModePLII_Movie = 0x21,/*L,C,R,LS,RS*/
    BDSP_Raaga_Audio_AcMode_eModePLII_Music = 0x22,/*L,C,R,LS,RS*/
    BDSP_Raaga_Audio_AcMode_eModePassiveMatrix = 0x23,/*L,R*/
    BDSP_Raaga_Audio_AcMode_eModeCSII = 0x24,/*L,C,R,Ls,Rs,Cs*/
    BDSP_Raaga_Audio_AcMode_eLAST,
    BDSP_Raaga_Audio_AcMode_eINVALID=0x7fffffff

}BDSP_Raaga_Audio_AcMode;

/*
     Default ConfigParams structure for decoder types.
*/
typedef struct BDSP_Raaga_Audio_DecodeConfigParams
{
    /* Number of output ports    */
    uint32_t                ui32NumOutPorts;
    /* Output mode    */
    BDSP_Raaga_Audio_AcMode    eOutMode;
    /* This is channel matrix of size 8 where each index can take any values from 0 to 7
       Depending upon  the channel routing done      */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
}BDSP_Raaga_Audio_DecodeConfigParams;

/*
     Default ConfigParams structure for audio processing types.
*/
typedef struct BDSP_Raaga_Audio_ProcessingConfigParams
{
    /* Number of input/output channels     */
    uint32_t                    ui32NumChans;
    /* Default: 1;  1 -> Process, 0 ->  Bypass */
    uint32_t                    ui32Enable;
}BDSP_Raaga_Audio_ProcessingConfigParams;

typedef struct BDSP_Raaga_Audio_DAPv2UserConfig
{
    uint32_t    ui32Mode;                                           /*Default is mode 0 .It allows features to be enabled/disabled to permit less memory/processing.
                                                                     0-Full Support
                                                                     1-Full Support Except dual virtualizer output modes
                                                                     2.Only Content Processing , no device processing support
                                                                     3.Only Dolby Volume,upmixing and Dialog Enhancer features enabled . no device processing and dual virtualizer support*/
    uint32_t    ui32MiProcessDisable;                               /* MI process status. Default: 0 , set 1 to enable it */
    uint32_t    ui32EnableIntelligentEqualizer;                     /*Default 0- disable make 1-enable */
    uint32_t    ui32IEamount;                                       /*Default 10, Range[0-16]  Specifies the strength of the Intelligent Equalizer effect to apply*/
    uint32_t    ui32IEQFrequency[20];                               /* Default values  32,64,125,250,500,1000,2000,4000,8000,16000.. Range[20, 20000]in Hz  */
    uint32_t    i32IEQInputGain[20];                                /* Default values{ 0}   Range [-480,480] */
    uint32_t    ui32IEQFrequency_count;             /* default 10*/
    uint32_t    ui32EnableVolumeModeler;            /*Default 0- disable make 1-enable */
    int32_t     i32VolModCalibration;               /* Default value- 0  Range [-320,320] Fine-tune the manufacturer calibrated reference level to the listening environment */
    int32_t     i32VolLevelOutTarget;               /* Default value- 320   Range [[-640, 0] ] Calibrate the system to a reference playback sound pressure level */
    int32_t     i32VolLevelInTarget;                /* Default value- 320   Range [[-640, 0] ] Sets the target average loudness level of the Volume Leveler */
    uint32_t    ui32VolLevelInputValue;             /* Default value- 7   Range [0,10]     Adjust the loudness to normalize different audio content */
    uint32_t    ui32leveler_ignore_il ;             /* ignore the IL sent from decoder */
    uint32_t    ui32SurroundBoostLevel;             /*  Default value- 96   Range [0,96]    Surround Compressor boost to be used.
                                                        This is used if the downmixer is Headphone Virtualizer or Speaker Virtualizer  */
    uint32_t    ui32EnableVolLeveler ;              /*Default 0- disable make 1-enable ,Volume leveler enabling*/
    uint32_t    ui32VolMaxBoostLevel;               /* Default value- 144   Range [0,192]    The boost gain applied to the signal in the signal chain.
                                                       Volume maximization will be performed only if Volume Leveler is enabled  */
    uint32_t    ui32EnableDialogEnhancer ;          /*Default 0- disable make 1-enable ,Dialog Enhancer  enabling*/
    uint32_t    ui32DialogEnhancerLevel;            /* Default value =0    Range [0,16]        The strength of the Dialog Enhancer effect  */
    uint32_t    ui32DialogEnhancerDuckLevel;        /* Default value =0    Range [0,16]    The degree of suppresion of channels that don't contain dialog */
    uint32_t    ui32EnableSurroundDecoder;          /*Default 0- disable make 1-enable ,Surround Decoder  enabling*/
    uint32_t    ui32MiEnableSurrCompressorSteering;     /* Default value =0    Set to 1 for enabling  If enabled, the parameters in the Surround Compressor will be updated based on the information from Media Intelligence */
    uint32_t    ui32MiEnableDialogueEnhancerSteering;   /* Default value =0    Set to 1 for enabling   If enabled, the parameters in the Dialog Enhancer will be updated based on the information from Media Intelligence */
    uint32_t    ui32MiEnableVolumeLevelerSteering;      /* Default value =0    Set to 1 for enabling  If enabled, the parameters in the Volume Leveler will be updated based on the information from Media Intelligence */
    uint32_t    ui32MiEnableIEQSteering;                /* Default value =0    Set to 1 for enabling  If enabled, the parameters in the Volume Leveler will be updated based on the information from Media Intelligence */
    uint32_t    ui32CalibrationBoost;                   /* Default value =0    Range [0,192 ]   A boost gain to be applied to the signal  */
    int32_t     i32SystemGain;                          /* Default value =0    Range [-2080,480]    The gain the user would like the signal chain to apply to the signal  */
    int32_t     i32PostGain;                            /* Default value =0    Range [-2080,480 ]   The post gain applied to the signal  */
    int32_t     i32PreGain;                             /* Default value =0    Range [-2080,480 ]   The pre gain applied to the signal  */
    uint32_t    ui32OutputMode;                         /*Default value =1    Range[1-10]
                                                         0 - Output channel count is 1(mono)
                                                         1 - Output channel count is 2 with order L, R
                                                         2 - Output channel count is 2 with order L, R
                                                            PLII LtRt compatible downmix will be carried out, only if a 8 to 2 and 6 to 2 downmix is needed
                                                         3 - Output channel count is 6 with order L, R, C, LFE, Ls, Rs
                                                         4 - Output channel count is 6 with order L R C LFE Ls Rs
                                                          PLIIz decode compatible downmix of Lrs and Rrs into Ls and Rs will be
                                                          carried out, only if a 8 to 6 downmix is needed
                                                         5 - Output channel count is 8 with order L, R, C, LFE, Ls, Rs, Lrs, Rrs
                                                         6 - Output channel count is 2 with order L, R
                                                          Headphone Virtualizer enabled as virtualizer
                                                         7 - Output channel count is 2 with order L, R
                                                         Speaker Virtualizer enabled as virtualizer
                                                         8 - Output channel count is 3 with order L, R, LFE
                                                          Speaker Virtualizer enabled as virtualizer
                                                         9 - Output channel count is 4 with order L headphone output, R headphone output
                                                          L speaker output, R speaker output
                                                          Dual virtualizer: Headphone virtualizer + 2.0 speaker virtualizer
                                                         10 - Output channel count is 5 with order L headphone output, R headphone output
                                                         L speaker output, R speaker output, LFE speaker output
                                                          Dual virtualizer: Headphone virtualizer + 2.1 speaker virtualizer*/
    int32_t     i32CmixlevQ14;                      /**< Linear q14 gain [0, 1.412 (23134)] */
    int32_t     i32SurixlevQ14;                     /**< Linear q14 gain [0, 1.0 (16384)] */
    int32_t     i32VolumeLevelerWeight;             /**< weighting for volume leveler in range [0,1], Q31 */
    int32_t     i32IntelligentEqualizerWeight;      /**< weighting for intelligent_equalizer in range [0,1],Q31 */
    int32_t     i32DialogEnhancerSpeechConfidence;  /**< speech confidence value for dialog_enhancer in range[0,1],Q31 */
    int32_t     i32SurroundCompressorMusicConfidence;   /**< music confidence value for surround_compressor in range[0,1],Q31 */

}BDSP_Raaga_Audio_DAPv2UserConfig;


typedef struct BDSP_Fader_Config{
    /* <attenuation> in % (0 -100) factor in Q1.31 format . default is 0x7FFFFFFF (No fade)*/
    /*in cert mode, allow volumes to pass without alteration (value form config in Q1.31 format to handle all cert cases), Only in production  convert to Q1.31 format*/
    int32_t i32attenuation;
    /*<duration> in ms, default 0ms, range 0ms ... 60000ms*/
    int32_t i32duration;
    /*<type> of easing function, 0: linear, 1: in-cubic, 2: out-cubic*/
    int32_t i32type;
  } BDSP_Fader_Config;
typedef struct BDSP_Raaga_Audio_MixerDapv2ConfigParams
{

   /*Default :0  Make it 1 for Mixer IDK certification */
    int32_t i32Certification_flag;
    /* Default: False, downmix metadata need to use for mixing if the input is downmixed : Set value only when i32Certification_flag is enabled      */
    int32_t i32IsInputDownmixed;
    /* Default: Off, Limiter need can be enabled for system sound and application for clipping protection Set value only when i32Certification_flag is enabled     */
    int32_t i32LimiterEnable;
    /* Default: 0 Channel Matching Mode strigent , 1 for lenient  Set value only when i32Certification_flag is enabled     */
    int32_t i32MixingMode;
    /* Default: 0 possible values from -32 to 32    */
    int32_t i32MixerUserBalance;
    /* Default: UNITY:  Channel wise user gains    */
    int32_t i32MixerVolumeControlGains[4][BDSP_AF_P_MAX_CHANNELS];
    /*Default :1  Make it 0 for non-Ms12 cases */
    int32_t i32Ms12Flag;
    /*Default :1  Make it 0  to disable Dapv2 */
    int32_t i32EnableDapv2;
    /*< Sets the ramp behavior if no input signal is available  */
    /*0: pauses ramp until input is present (default)*/
    /*1: continues ramp in absence of input signal\n"*/

    int32_t  i32RampMode;
   /* input fade configuration*/
    BDSP_Fader_Config    sFadeControl[5];
    /* Dapv2 user config might get changed after latest code change for content processing with no channel mixing */
    BDSP_Raaga_Audio_DAPv2UserConfig sDapv2UserConfig;

    /* channel mode is locked to 5.1 */
    /* This Parameter need to be set either 1 or 0 .1-to get the 5.1 output from mixer onwards*/
    uint32_t    ui32ChannelLockModeEnable;

}BDSP_Raaga_Audio_MixerDapv2ConfigParams;

/*
     Default ConfigParams structure for Video decoder types.
*/
typedef struct BDSP_Raaga_Video_DecodeConfigParams
{
    /* Dummy variable     */
    uint32_t                    ui32Dummy;
}BDSP_Raaga_Video_DecodeConfigParams;

/*
   This data structure defines AC-3/DDP decoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_Ac3UserOutput
{
     /* This is LFE ON/OFF flag and can take  two values 0 or 1
      default value for this field is 0 */
     int32_t i32OutLfe;
     /* i32OutMode =7 default value;*/
     int32_t i32OutMode;
     /* preferred stereo mode  i32StereoMode = 0 is default value */
     int32_t i32StereoMode;
     /* dual mono downmix mode i32DualMode = 0 is default value */
     int32_t i32DualMode;
     /* karaoke capable mode i32Kmode = 3 */
     int32_t i32Kmode;
     /* This i32ExtDnmixEnabled flag which can take  two values 0 or 1 based on disable/enable option
      default value for this i32ExtDnmixEnabled field is 0 */
     int32_t i32ExtDnmixEnabled;
     int32_t i32ExtDnmixTab[DDP_DEC_GBL_MAXPCMCHANS][DDP_DEC_GBL_MAXPCMCHANS];
     /* This i32ExtKaraokeEnabled flag which can take  two values 0 or 1 based on disable/enable option
      default value for this i32ExtKaraokeEnabled field is 0 */
     int32_t i32ExtKaraokeEnabled;
     int32_t i32Extv1Level;
     int32_t i32Extv1Pan;
     int32_t i32Extv2Level;
     int32_t i32Extv2Pan;
     int32_t i32ExtGmLevel;
     int32_t i32ExtGmPan;
     /*This is channel matrix of size 8 where each index can take any values from 0 to 7
       Depending upon  the channel routing done
     */
     uint32_t   ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
}BDSP_Raaga_Audio_Ac3UserOutput;



typedef struct  BDSP_Raaga_Audio_Ac3ConfigParams
{
    /* p_user_cfg->i32CompMode = 2;*/
    /*Custom_0=0, Custom_1 = 1, Line = 2, RF =3 , DRC_OFF = 4, RF_23dB = 5, RF_24dB = 6 */
    int32_t                  i32CompMode;

    /*p_user_cfg->i32PcmScale = 100;    The value is in percentage, 0 to 100. */
     int32_t                     i32PcmScale;

    /* p_user_cfg->i3DynScaleHigh = 0x7FFFFFFF;  0x0 to  0x7FFFFFFF in Q1.31 format */
     int32_t                     i32DynScaleHigh;

    /* p_user_cfg->i32DynScaleLow = 0x7FFFFFFF;  0x0 to  0x7FFFFFFF in Q1.31 format */
     int32_t                     i32DynScaleLow;

    /*  This value "i32NumOutPorts" can be set to 1 or 2 based on output ports */
     int32_t                     i32NumOutPorts;

    /* This value is used to enable/disable stream dialog normalization value.  0 is for Disable and 1 is for Enable
       Default is Enable
     */
     int32_t                     i32StreamDialNormEnable;

    /*  This value indicates how far the average dialogue level is below digital 100 percent. Valid
        values are 1-31. The value of 0 is reserved and it means dialog normalization is turned off. The values of 1 to 31 are interpreted as -1 dB to -31dB
        with respect to digital 100 percent. If the reserved value of 0 is received, the decoder will not do any dialog normalization,
        Default value is 0
    */
    int32_t                     i32UserDialNormVal;

     /* These are user config parameters required from user  */
     BDSP_Raaga_Audio_Ac3UserOutput  sUserOutputCfg[2];

}BDSP_Raaga_Audio_Ac3ConfigParams;



/*
   This data structure defines MPEG-1 L1/L2/L3 decoder user configuration parameters
*/

typedef struct BDSP_Raaga_Audio_MpegConfigParams
{
    BDSP_AF_P_DecoderType       eDecoderType;    /* Decoder Type indicates whether the context is primary (default) or secondary */
    uint32_t    ui32OutMode;                     /* outputmode */
    uint32_t    ui32LeftChannelGain;             /* In 1Q31 format - not supported */
    uint32_t    ui32RightChannelGain;            /* In 1Q31 format - not supported */
    uint32_t    ui32DualMonoMode;
    int32_t     i32InputVolLevel;               /* Used in the case of A85. Please set input = output for non-A85 cases. Value is in dB. For EBU Set input = -23, For ATSC, set input = -14*/
    int32_t     i32OutputVolLevel;              /* Used in the case of A85. Please set input = output for non-A85 cases. Value is in dB. For EBU Set output= -23  For ATSC, set output= -20*/
    uint32_t    ui32AncDataParseEnable;         /* Flag to enable MPEG Ancillary Data Parsing: Default 0 (Disable)*/
    BDSP_AF_P_sSINGLE_CIRC_BUFFER   sAncDataCircBuff;       /* Ancillary RDB Buffer Structure */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
    BDSP_Raaga_eBOOLEAN eMonotoStereoDownScale;
}BDSP_Raaga_Audio_MpegConfigParams;


/*
   This data structure defines WMA-9 decoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_WmaConfigParams
{
    uint32_t    ui32OutputMode;

    /*These 2 variables are used to resolve problems in comparision in RTL platform for Certification */
    uint32_t    decodeOnlyPatternFlag;

    uint32_t    decodePattern;

    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS]; /* Default Channel Matrix = {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF } */
}BDSP_Raaga_Audio_WmaConfigParams;

/* 1)   D_DRC_HIGH: No scaling is applied.  Content is played in original form.
2)  D_DRC_MED: drc_frame_scale_factor is applied along with a scale factor to make
               the output rms of the content rms_amplitude_target_dB,
               and the peak of the content be peak_amplitude_target_med_dB.
3)  D_DRC_LOW: drc_frame_scale_factor is applied along with a scale factor to make
                the output rms of the content rms_amplitude_target_dB, and the peak
                of the content be peak_amplitude_target_low_dB.
*/
typedef enum BDSP_Raaga_Audio_eDrcSetting
{
    BDSP_Raaga_Audio_eDrcSetting_High=0,
    BDSP_Raaga_Audio_eDrcSetting_Med,
    BDSP_Raaga_Audio_eDrcSetting_Low,
    BDSP_Raaga_Audio_eDrcSetting_eLAST,
    BDSP_Raaga_Audio_eDrcSetting_eINVALID=0x7fffffff

}BDSP_Raaga_Audio_eDrcSetting;

/*
   This data structure defines WMA-PRO decoder user configuration parameters
*/
typedef struct BDSP_Raaga_Audio_WmaProUserConfig
{
    uint32_t                    ui32DRCEnable;          /* Default 0 , range 0 and 1*/
    BDSP_Raaga_Audio_eDrcSetting        eDRCSetting;            /* Default 0 */
    int32_t                     i32RmsAmplitudeRef;
    int32_t                     i32PeakAmplitudeRef;
    int32_t                     i32DesiredRms;          /* desired rmsDb for normalization */
    int32_t                     i32DesiredPeak;         /* desired peakDb for normalization */
    uint32_t    ui32OutMode;    /* Output channel configuration */
    uint32_t    ui32OutLfe;     /* Output LFE on/off */
    uint32_t    ui32Stereomode; /* Stereo mode - Auto=0/ LtRt=1 /LoRo=2 */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS]; /* Default Channel Matrix = {0, 1, 2, 3, 4, 5, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF } */

}BDSP_Raaga_Audio_WmaProUserConfig;

typedef struct BDSP_Raaga_Audio_WmaProConfigParams
{
    uint32_t                        ui32NumOutports;    /* 1 -> Only Multichannel out [number of outmodes to be supported] ,  2 -> Stereo out + Multichannel */
    BDSP_Raaga_Audio_WmaProUserConfig   sOutputCfg[2];      /* User Config for Multichanel or Stereo Output */
    uint32_t                        ui32UsageMode;      /* Decode =0, Passthru =1,SimulMode =2*/

}BDSP_Raaga_Audio_WmaProConfigParams;

/* User config of Opus Decoder*/
typedef struct  BDSP_Raaga_Audio_OpusDecUsrCfg
{
        uint32_t    ui32OutMode;    /* Default = 1; Output channel configuration */
        uint32_t    ui32OutputChannelMatrix[8];
            /*
            For Multichannel (5.1) Output Port:
                        ui32OutputChannelMatrix[0] = 0;
                        ui32OutputChannelMatrix[1] = 1;

                        for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
                                ui32OutputChannelMatrix[i] = 0xFFFFFFFF;

            For Stereo Output Port:
                        for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
                                ui32OutputChannelMatrix[i] = 0xFFFFFFFF;  this value shows invalid
            */

} BDSP_Raaga_Audio_OpusDecUsrCfg;

typedef struct BDSP_Raaga_Audio_OpusDecConfigParams
{
       uint32_t    ui32NumOutPorts;      /*  Default = 1; 1 -> Multichannel out, 2 -> Multichannel + Stereo out */

       BDSP_Raaga_Audio_OpusDecUsrCfg sUsrOutputCfg[2];
       BDSP_AF_P_DecoderType   eDecoderType; /* Default : BDSP_AF_P_DecoderType_ePrimary */

}BDSP_Raaga_Audio_OpusDecConfigParams;



/* User config of ALS Decoder*/
typedef struct  BDSP_Raaga_Audio_ALSDecUsrCfg
{
    uint32_t    ui32OutMode;    /* Default = 7; Output channel configuration */
    int32_t     i32ExtDnmixTab[6][6];
    uint32_t    ui32OutputChannelMatrix[8];
    /*
    For Multichannel (5.1) Output Port:
    ui32OutputChannelMatrix[0] = 0;
    ui32OutputChannelMatrix[1] = 1;

    for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
    ui32OutputChannelMatrix[i] = 0xFFFFFFFF;

    For Stereo Output Port:
    for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
    ui32OutputChannelMatrix[i] = 0xFFFFFFFF;  this value shows invalid
    */

} BDSP_Raaga_Audio_ALSDecUsrCfg;

typedef struct BDSP_Raaga_Audio_ALSDecConfigParams
{
    /* Assumption : Port 0 is Multichannel out and Port 1 is  Stereo out  */
    uint32_t    ui32NumOutPorts;                /*  Default = 2; 1 -> Multichannel out, 2 -> Multichannel + Stereo out */
    int32_t     i32DownmixType;                 /* Default = 1, 0 -> USER downmix, 1 -> ARIB downmix  2 -> LTRT downmix*/
    int32_t     i32AribMatrixMixdownIndex;          /* Default = 1, ARIB matrix mixdown index which can take integer values from 1 to 3 */
    BDSP_Raaga_Audio_ALSDecUsrCfg sUsrOutputCfg[2];

}BDSP_Raaga_Audio_ALSDecConfigParams;

/*
   This data structure defines AACHE multi-channel decoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_AacheUserConfig
{
    int32_t     i32OutLfe;                                          /* Default = 0, Output LFE channel present */
    int32_t     i32OutMode;                                         /* Default = 2, Output channel configuration */
    int32_t     i32DualMode;                                        /* Default = 0, Dual mono reproduction mode */
    int32_t     i32ExtDnmixEnabled;                                 /* Default = 0, Enable external downmix */
    int32_t     i32ExtDnmixTab[6][6];                               /* Default = 0, External downmix coefficient table */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

} BDSP_Raaga_Audio_AacheUserConfig;

typedef struct  BDSP_Raaga_Audio_AacheConfigParams
{
    int32_t                         i32NumOutPorts;                 /* Default = 1, 1 -> Multichannel out, 2 -> Multichannel + Stereo out */
    int32_t                         i32DownmixType;                 /* Default = 0, 0 -> BRCM downmix, 1 -> ARIB downmix */
    int32_t                         i32AribMatrixMixdownIndex;      /* Default = 0, ARIB matrix mixdown index which can take integer values from 0 to 3 */
    uint32_t                        ui32DrcGainControlCompress;     /* Default = 0x40000000, Constant used in the DRC calculation, typically between 0 and 1, in Q2.30 format */
    uint32_t                        ui32DrcGainControlBoost;        /* Default = 0x40000000, Constant used in the DRC calculation, typically between 0 and 1, in Q2.30 format */

    /* This parameter ui32DrcTargetLevel is used to ignore PRL when set to 0, any non zero value has no impact on the output levels */
    uint32_t                        ui32DrcTargetLevel;             /* Default = 127, Target level desired by the host */
    BDSP_Raaga_Audio_AacheUserConfig        sUserOutputCfg[2];

                                                                    /* Boost of 6dB, specific to Buffalo */
    int32_t                         i32PcmBoost6dB;                 /* Default = 0, 0 -> FALSE, 1 -> TRUE */

                                                                    /* Attenuation of 4.75dB, specific to DDCO certification */
    int32_t                         i32PcmBoostMinus4p75dB;         /* Default = TRUE for LG branch and Default = FALSE for others, 0 -> FALSE, 1 -> TRUE */

    uint32_t                        ui32FreqExtensionEnable;        /* Default = 0, 0->disable freq extension for high sample rate, 1->Enable Frequency extension for high sample rates */

    uint32_t                        ui32SbrUserFlag;                /* default = 0, 0-> disable SBR processing, 1-> Enable SBR processing/Upsample, This flag is used currently for DVD only */

    int32_t                         i32InputVolLevel;               /* in dB */
    int32_t                         i32OutputVolLevel;              /* in dB */
    uint32_t                        ui32DownmixCoefScaleIndex;      /* Default = 50,  0 -> 0dB, 1 -> -0.5dB, 2 -> -1dB, ... , 23 -> -11.5dB, 24 -> -12dB; all values beyond 24 map to 50 */
                                                                    /* 50 -> Decoder default settings (existing normalization) */

    /* Loudness Equivalence Mode
          0 = No Loudness Equivalence (Default)
          1 = EBU (-23dB)
          2 = ATSC (-24dB)
          3 = EBU_ALL_PORTS (-23dB)
          4 = ATSC_ALL_PORTS (-24dB)

          EBU Settings:
          i32OutputVolLevel = 31*4 (-31dB)
          i32InputVolLevel = 23*4 (-23dB)
          ui32LoudnessEquivalenceMode = 1
              -Multichannel is at -31dB
              -Stereo is at -23dB

          ATSC Settings:
          i32OutputVolLevel = 31*4 (-31dB)
          i32InputVolLevel = 24*4 (-24dB)
          ui32LoudnessEquivalenceMode = 2
              -Multichannel is at -31dB
              -Stereo is at -24dB

          EBU Settings All Ports:
          i32OutputVolLevel = 31*4 (-31dB)
          i32InputVolLevel = 23*4 (-23dB)
          ui32LoudnessEquivalenceMode = 3
             -Multichannel is at -23dB
             -Stereo is at -23dB

          ATSC Settings All Ports:
          i32OutputVolLevel = 31*4 (-31dB)
          i32InputVolLevel = 24*4 (-24dB)
          ui32LoudnessEquivalenceMode = 4
              -Multichannel is at -24dB
              -Stereo is at -24dB */
    uint32_t                        ui32LoudnessEquivalenceMode;


    /* This enum lets the mixer know if the decoder type is primary or secondary or sound effects. */
    BDSP_AF_P_DecoderType   eDecoderType; /* Default : BDSP_AF_P_DecoderType_ePrimary */

} BDSP_Raaga_Audio_AacheConfigParams;


typedef struct BDSP_Raaga_Audio_SpeexAECConfigParams
{
    /* Gain resolution to be used in the preprocessor 0 = Bark Mode (Faster), 1 = Linear mode (Better but much slower). Default = 0 */
    uint32_t ui32GainResolution;

} BDSP_Raaga_Audio_SpeexAECConfigParams;


typedef struct BDSP_Raaga_Audio_OutputFormatterConfigParams
{
        uint32_t ui32NumChannels;   /* Number of channels for the output */
        uint32_t ui32ChannelLayout[BDSP_AF_P_MAX_CHANNELS]; /* Layout of the channels for interleaving */
        uint32_t ui32NumBitsPerSample; /* Number of bits per sample to be interleaved */
        BDSP_Raaga_eBOOLEAN eSignedSample; /* Tells whether the sample is signed or unsigned value */
}BDSP_Raaga_Audio_OutputFormatterConfigParams;

typedef struct BDSP_Raaga_Audio_KaraokeConfigParams
{
    /* Level of reduction of voice 0-5 */
    int32_t level;
    int32_t speechBins; /* range of vocal bins 0-4096 */
    int32_t scaleoutputs; /* amount of scaling of output 0 - No scaling
                                                         1 - 1dB
                                                         2 - 2dB
                                                         3 - 3dB */
}BDSP_Raaga_Audio_KaraokeConfigParams;

/* Vocal post process */
typedef struct BDSP_Raaga_Audio_VocalPPConfigParams
{
    int32_t i32EchoEnable;
    int32_t i32EchoAttn;      /*Q31 number  (default=0x1999999A (0.2)*/
    int32_t i32EchoDelayInMs; /* Delay in ms max=1000 */
}BDSP_Raaga_Audio_VocalPPConfigParams;
/*
    MPEG1-Layer-2 encoder user configuration structure
*/

typedef struct BDSP_Raaga_Audio_Mpeg1L2EncConfigParams
{
    uint32_t    ui32bitRateIdx;
    /* This differs from layer-3
    bitRateIdx      bit-rate
    [0]             0;
    [1]             32;
    [2]             48;
    [3]             56;
    [4]             64;
    [5]             80;
    [6]             96;
    [7]             112;
    [8]             128;
    [9]             160;
    [10]            192;
    [11]            224;
    [12]            256;
    [13]            320;
    [14]            384;
    */

    uint32_t ui32AddCRCProtect;/* not supported */
    /*
    addCRCProtect   Implication
        0           CRC protect off
        1           CRC protect on
    */

    uint32_t ui32Ext; /* Controls private bit setting in the header */
    /*
    ext     Implication
    0       => not Set
    1       => Set
    */

    uint32_t ui32jStereoControl; /* Joint-Stereo Control */
    /*
    Layer-2 does not have provision for m/s stereo. The user can set either 0,1 or 3
    jStereoControl  Implication
    0               No joint stereo (ie: no intensity or m/s stereo )
    1               Allow intensity stereo
    2               Allow m/s stereo
    3               Allow intensity and m/s stereo
    4               Reserved
    5               Reserved
    6               Reserved
    7               Reserved
    */

    uint32_t ui32Copyright; /* Cpyright bit setting in the header */

    uint32_t ui32Original;  /* Original bit setting in the header */

    uint32_t ui32BitsPerSampleIdx;  /* number of bits per PCM sample */
            /* The no of bits per sample is treated as 16 + bitsPerSampleIdx */

    uint32_t ui32SampleRateIdx;
    /*
    sampleRateIdx   Sample-Rate
    [0]             44100;
    [1]             48000;
    [2]             32000;
    */

    uint32_t    ui32Emphasis;    /* not supported */
    /*
    emphasis    Implication
    0           none
    1           50/15 microseconds
    2           reserved
    3           CCITT J.17
    */

    uint32_t ui32InputMode;
    /*
    0 stereo
    1 reserved
    2 dual channel
    3 mono
    */

}BDSP_Raaga_Audio_Mpeg1L2EncConfigParams;

/*
   This data structure defines DTS-5.1 encoder user configuration parameters
*/


typedef struct  BDSP_Raaga_Audio_DtsEncConfigParams
{
    uint32_t ui32Amode;             /* Audio Coding Mode 2 channel, 5 channel etc. */
    uint32_t ui32Sfreq;             /* Sampling Frequency */
    uint32_t ui32Dynf;              /* Embedded Dynamic Range Flag */
    uint32_t ui32Rate;              /* Bit Rate of the Encoder output */
    uint32_t ui32Mix;               /* Downmix to stereo enable Flag */
    uint32_t ui32Timef;             /* Embedded Time Stamp Flag */
    uint32_t ui32Auxf;              /* Auxiliary Data Flag */
    uint32_t ui32Hdcd;              /* Source in HDCD Format */
    uint32_t ui32Sumf;              /* Front Sum Difference Encode Flag */
    uint32_t ui32Sums;              /* Surround Sum Difference Encode Flag */
    uint32_t ui32Lfe;               /* Low Frequency Effect Flag */
    uint32_t ui32Dtses;             /* DTS Extended Surround Enable Format */
    uint32_t ui32Pcmr;              /* Source Input PCM Resolution */
    uint32_t ui32ExtAudio;          /* Extension Audio Present Flag */
    uint32_t ui32ExtAudioId;        /* Type of the Extension Audio like XXCH, XBR, X96 etc */
    uint32_t ui32Vernum;            /* Indicates the revision status of encoder software */
    uint32_t ui32DialNorm;          /* Dialogue Normalization */
    uint32_t ui32Joinx;             /* Joint Intensity Coding Enable Flag */
    uint32_t ui32Cpf;               /* Bit Stream CRC Enable Flag */
    uint32_t ui32Aspf;              /* Indicates the frequency with which DSYNC occurs in the stream */
    uint32_t ui32Ftype;             /* Frame type whether normal or termination frame */
    uint32_t ui32Chist;             /* Copy History.  History of the source material */
    uint32_t ui32Range;             /* Dynamic Range Coefficient */
    uint32_t ui32Addmf;             /* Auxiliary Dynamic Downmix Flag */
}BDSP_Raaga_Audio_DtsEncConfigParams;




/*
   This data structure defines stereo AAC-LC encoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_AaclcEncConfigParams
{
    uint32_t ui32Acmod;             /* Audio Coding Mode 2 channel, 5 channel etc.*/
    uint32_t ui32SampFreq;          /* Sampling Frequency */
    uint32_t ui32BitRate;           /* Bit Rate of the encoder */
/* This is flag which is used by AAC-LC encoder to decide whether PCM  data is coming directly or it is decimated by SBR module */
    uint32_t ui32SBR_PresentFlag;
}BDSP_Raaga_Audio_AaclcEncConfigParams;


/*
    Bit rates supported in AAC encoder.
*/
typedef enum    BDSP_Raaga_Audio_AacEncodeBitRate
{
    BDSP_Raaga_Audio_AacEncodeBitRate_e16kbps = 16000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e20kbps = 20000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e32kbps = 32000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e40kbps = 40000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e44kbps = 44100,
    BDSP_Raaga_Audio_AacEncodeBitRate_e48kbps = 48000,
    /* Below bit rates are supported only when SBR is off */
    BDSP_Raaga_Audio_AacEncodeBitRate_e50kbps = 50000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e52kbps = 52000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e64kbps = 64000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e100kbps = 100000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e128kbps = 128000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e200kbps = 200000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e224kbps = 224000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e256kbps = 256000,
    BDSP_Raaga_Audio_AacEncodeBitRate_e320kbps = 320000,
    BDSP_Raaga_Audio_AacEncodeBitRate_eLast,
    BDSP_Raaga_Audio_AacEncodeBitRate_eInvalid = 0x7fffffff
}BDSP_Raaga_Audio_AacEncodeBitRate;

typedef enum
{
  BDSP_Raaga_Audio_AacEncodeComplexity_Low = 0,
  BDSP_Raaga_Audio_AacEncodeComplexity_Medium_1,
  BDSP_Raaga_Audio_AacEncodeComplexity_Medium_2,
  BDSP_Raaga_Audio_AacEncodeComplexity_High,
  BDSP_Raaga_Audio_AacEncodeComplexity_eInvalid = 0x7fffffff
}BDSP_Raaga_Audio_AacEncodeComplexity;


typedef enum
{
    BDSP_Raaga_Audio_AacEncodeMono_Left      = 0,
    BDSP_Raaga_Audio_AacEncodeMono_Right     = 1,
    BDSP_Raaga_Audio_AacEncodeMono_Mix       = 2,
    BDSP_Raaga_Audio_AacEncodeMono_DualMono  = 3
}BDSP_Raaga_Audio_AacEncodeMonoChannelSelect;

typedef enum
{
    BDSP_Raaga_Audio_AacEncodeAdtsMpeg4 = 0,
    BDSP_Raaga_Audio_AacEncodeAdtsMpeg2 = 1
}BDSP_Raaga_Audio_AacEncodeAdtsMpegType;

/*
   This data structure defines stereo AAC-HE encoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_AacheEncConfigParams
{
    /* Output Bitrate of the Encoder */
    BDSP_Raaga_Audio_AacEncodeBitRate        eAacEncodeBitRate;
    /* Flag which directs the Encoder to encode in Mono or Stereo mode */
    uint32_t        ui32bEncodeMono;
    /* Disable the Spectral Band Replication module if this is reset , by default it is set */
    uint32_t        ui32EnableSBR;
    /* Selects the output Bitstream encode format
       0  RAW AUDIO
       1  ADTS
       2  LOAS
    */
    uint32_t        ui3248SRC32Enable;
      /* Enables/Disables SRC from 48khz to 32khz before encoding.
       1 SRC Enable
       0 No SRC
       */
    uint32_t        ui32EncodeFormatType;
    int32_t                         i32InputVolLevel;               /* in dB */
    int32_t                         i32OutputVolLevel;              /* in dB */
    BDSP_Raaga_Audio_AacEncodeComplexity    eAacEncodeComplexity;
    BDSP_Raaga_Audio_AacEncodeMonoChannelSelect   eAacEncodeMonoChSelcect;
    BDSP_Raaga_Audio_AacEncodeAdtsMpegType eAacEncodeAdtsMpegType;
}BDSP_Raaga_Audio_AacheEncConfigParams;

/*
Settings structure that are populated by APE to be sent as a part of freeze command payload.

*/
typedef struct BDSP_AudioTaskFreezeSettings
{
     uint32_t fmmOutputAddress;   /* Register address for the FMM output register to freeze/un-freeze */
     uint32_t fmmOutputMask;       /* Mask to be applied to the current FMM output register value to freeze/un-freeze */
     uint32_t fmmOutputValue;      /* Value to be OR'ed into the FMM output register value */
} BDSP_AudioTaskFreezeSettings;


/*
Settings structure that are populated by APE to be sent as a part of unfreeze command payload.

*/
typedef struct BDSP_AudioTaskUnFreezeSettings
{
     uint32_t fmmOutputAddress;   /* Register address for the FMM output register to freeze/un-freeze */
     uint32_t fmmOutputMask;       /* Mask to be applied to the current FMM output register value to freeze/un-freeze */
     uint32_t fmmOutputValue;      /* Value to be OR'ed into the FMM output register value */
     uint32_t ui32NumBuffers;   /* Number of buffers */
     BDSP_AF_P_sDRAM_CIRCULAR_BUFFER  sCircBuffer[BDSP_AF_P_MAX_CHANNELS]; /* Circular Buffer Address for dummy sink buffer */
} BDSP_AudioTaskUnFreezeSettings;

/*
   This data structure defines Passthrough Node user configuration parameters
*/

typedef enum BDSP_Raaga_ePassthruType
{
    BDSP_Raaga_ePassthruType_SPDIF,
    BDSP_Raaga_ePassthruType_RAW,
    BDSP_Raaga_ePassthruType_PCM,
    BDSP_Raaga_ePassthruType_Simul,
    BDSP_Raaga_ePassthruType_LAST,
    BDSP_Raaga_ePassthruType_INVALID =0x7FFFFFFF
}BDSP_Raaga_ePassthruType;

/*
 * AAC header type selection. If input stream is ADTS, supported values are BDSP_Raaga_eAacHeaderType_Raw
 * and BDSP_Raaga_eAacHeaderType_Adts. If input is LOAS, supported values are BDSP_Raaga_eAacHeaderType_Raw
 * and BDSP_Raaga_eAacHeaderType_Loas
 */
typedef enum BDSP_Raaga_eAacHeaderType
{
    /* If chosen ADTS/LOAS header will be stripped off and RAW data will be sent */
    BDSP_Raaga_eAacHeaderType_Raw,
    /* RAW data with ADTS header will be sent */
    BDSP_Raaga_eAacHeaderType_Adts,
    /* RAW data with LOAS header will be sent */
    BDSP_Raaga_eAacHeaderType_Loas,
    BDSP_Raaga_eAacHeaderType_Last,
    BDSP_Raaga_eAacHeaderType_Invalid    =   0x7fffffff

}BDSP_Raaga_eAacHeaderType;

typedef struct  BDSP_Raaga_Audio_PassthruConfigParams
{
     BDSP_Raaga_ePassthruType ui32PassthruType;
     BDSP_Raaga_eAacHeaderType  eAacHeaderType;
	 BDSP_AF_P_BurstFillType		  eFMMPauseBurstType;
	 BDSP_AF_P_SpdifPauseWidth	   eSpdifPauseWidth;
}BDSP_Raaga_Audio_PassthruConfigParams;

/* User Configuration custom voice Post Processing tool*/
typedef struct  BDSP_Raaga_Audio_CustomVoiceConfigParams
{
    int32_t     param000;     /*FUNC ENABLE*/
    int32_t    OSDVolume;
    int32_t    param011;
    uint32_t  param017;
    int32_t   param018;
    uint32_t  param019;
    uint32_t  param020;
    int32_t   param021;
    uint32_t  param022;
    int32_t   param023;
    int32_t   param024;
    int32_t   param025;
    uint32_t  param026;
    uint32_t  param027;
    uint32_t  param028;
    uint32_t  param029;
    uint32_t  param030;
    uint32_t  param031;
    uint32_t  param032;
    uint32_t  param033;
    uint32_t  param034;
    uint32_t  param035;
    uint32_t  param036;
    int32_t   param037;
    int32_t   param038;
    int32_t   param039;
    int32_t   param040;
    uint32_t  param041;
    uint32_t  param042;
    uint32_t  param043;
    uint32_t  param044;
    uint32_t  param045;
    uint32_t  param046;
    uint32_t  param047;
    uint32_t  param048;
    int32_t   param049;
    uint32_t  param050;
    uint32_t  param051;
    uint32_t  param052;
    uint32_t  param053;
    uint32_t  param054;
    int32_t   param055;
    int32_t   param056;
    int32_t   param057;
    int32_t   param058[20];
    int32_t   param059[20];
    int32_t   param060;
    int32_t   param061;
    int32_t   param062;
    int32_t fn17_tab02[11][2];
    int32_t fn17_tab03[11][3];
    int32_t fn17_tab04[11][2];
    int32_t fn17_tab05[11][3];
    int32_t   param067[3];
    int32_t   param068[2];
    int32_t   param069;
    int32_t   param070;
    int32_t   param071;
    int32_t   param072;
    int32_t   param073;
    int32_t   param074;
    int32_t   param075;
    int32_t   param076;
    int32_t   param077;
    int32_t   param078;
    int32_t   param079;
    int32_t   param080;
    int32_t   param081;
    int32_t   param082;
    int32_t   param083;
    int32_t   param084;
    int32_t   param085;
    int32_t   param086;
    int32_t   param087;
    int32_t   param088;
    int32_t   param089;
    int32_t   param090;
    int32_t   param091;
    uint32_t  param092;
    int32_t   param093;
    int32_t   param094;
    int32_t   param095;
    int32_t   param096;
    int32_t   param097;
    int32_t   param098;
    int32_t   param099;

    /*NewParams*/
    int32_t paramn093[2];
    int32_t paramn094[3];
    int32_t paramn095[2];
    int32_t paramn096[3];
    int32_t paramn097;
    int32_t paramn098;
    int32_t paramn099;
    int32_t paramn100;
    int32_t paramn101;
    int32_t paramn102;
    int32_t paramn103;
    int32_t paramn104;
    int32_t paramn105[3];
    int32_t paramn106[2];
    int32_t DSPfxdu_C68;


}BDSP_Raaga_Audio_CustomVoiceConfigParams;

/*
   This data structure defines DTS-5.1 Broadcast encoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_DtsBroadcastEncConfigParams
{
    uint32_t            ui32SpdifHeaderEnable;          /* Default value = Disable:  Enable =1 Disable =0 */

    /*Certification Related */
    uint32_t            ui32CertificationEnableFlag;    /*Default 0 Set to 1 while doing certification*/
    uint32_t            ui32LFEEnableFlag;              /*Used only when Certification is set to 1. Enable =1 Disable =0*/
    BDSP_Raaga_Audio_AcMode eInputDataAcMode;               /*Used only when Certification is set to 1*/
    uint32_t            ui32IsIIRLFEInterpolation;                           /*Default 0, range [0, 1]*/
                                                                             /*0 -> FIR interpolation for LFE */
                                                                             /*1 -> IIR interpolation for LFE */
}BDSP_Raaga_Audio_DtsBroadcastEncConfigParams;

/*
   This data structure defines AC3 encoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_Ac3EncConfigParams
{
    int32_t  i32NumChans;

    /* Audio Coding mode, Default: 7(3/2)*/
    int32_t  i32AudCodMode;

    /* Data Rate is dependent on Acmod, 18 for 3/2, 14 for 2/0 and 1/1 */
    int32_t  i32DataRate;

    /*  1 is LFE enabeld */
    int32_t  i32LoFreqEffOn;

    /* Always Set to 48 kHz */
    int32_t  i32SampRateCod;

    /* LFE filter is not being used, always set to 0 */
    int32_t  i32LfeFiltInUse;
    /* compression characteristic 0 = off , always off*/
    int32_t  i32CompChar;

    /* compression characteristic for 1+1 R chan, 0 = off, always off */
    int32_t  i32CompChar2;
    /* Always switched off in release code, can be enabled for certification testing */
    int32_t  i32SurDelayArg;

    /* This is used to enable/disable SPDIF header.. Default value = Enable:  Enable =1 Disable =0 */
    BDSP_AF_P_EnableDisable eSpdifHeaderEnable;

    /* This flag is set to enable when the Encoder is being used in transcode mode */
    BDSP_AF_P_EnableDisable   eTranscodeEnable;

}BDSP_Raaga_Audio_Ac3EncConfigParams;

/*
   This data structure defines SRC user configuration parameters
*/
typedef struct  BDSP_Raaga_Audio_SRCUserConfigParams
{
    uint32_t    ui32SrcDummyUserConfig;/*Default value is 0 */

}BDSP_Raaga_Audio_SRCUserConfigParams;


/* This data structure defines for AUDIO DESCRIPTOR */

typedef struct
{
    uint32_t   ui32FadeValue; /*Default - 0x7FFFFFFF (no fade) */

}BDSP_Raaga_Audio_sPanFadeIf;

typedef enum BDSP_Raaga_Audio_FadeConfigType
{
    BDSP_Raaga_Audio_FadeConfigType_ePrimaryChOnly = 0,
    BDSP_Raaga_Audio_FadeConfigType_eFadedChOnly,
    BDSP_Raaga_Audio_FadeConfigType_ePrimaryAndFadedCh,
    BDSP_Raaga_Audio_FadeConfigType_eLast,
    BDSP_Raaga_Audio_FadeConfigType_eInvalid = 0x7FFFFFFF

} BDSP_Raaga_Audio_FadeConfigType;

/*
   This data structure defines Audio Descriptor Fade user configuration parameters
*/
typedef struct  BDSP_Raaga_Audio_AudioDescFadeConfigParams
{
    BDSP_Raaga_Audio_FadeConfigType   eFadeConfig;              /* Default Value: BDSP_Raaga_Audio_FadeConfigType_eFadedChOnly */

    uint32_t                     ui32PanFadeInterfaceAddr;  /* This is a Dram Structure having the type 'BDSP_Raaga_Audio_sPanFadeIf'*/

    uint32_t                     ui32PanFadeInterfaceValidFlag; /*Default value will be zero and set to 1 when the InterFaceAddr is filled*/

}BDSP_Raaga_Audio_AudioDescFadeConfigParams;

/*
   This data structure defines Audio Descriptor Fade user configuration parameters
*/
typedef enum
{
    BDSP_Raaga_Audio_ADChannelConfig_eADChannelOff = 0,
    BDSP_Raaga_Audio_ADChannelConfig_eADChannelOn,
    BDSP_Raaga_Audio_ADChannelConfig_eLast,
    BDSP_Raaga_Audio_ADChannelConfig_eInvalid = 0x7FFFFFFF
}BDSP_Raaga_Audio_ADChannelConfig;

/*
   This data structure defines Frame sync configuration parameters
*/
typedef enum BDSP_Raaga_Audio_CapInputPort
{
    BDSP_Raaga_Audio_CapInputPort_eIntCapPort0 = 0, /* Internal Capture port 0 */
    BDSP_Raaga_Audio_CapInputPort_eIntCapPort1,     /* Internal Capture port 1 */
    BDSP_Raaga_Audio_CapInputPort_eIntCapPort2,     /* Internal Capture port 2 */
    BDSP_Raaga_Audio_CapInputPort_eIntCapPort3,     /* Internal Capture port 3 */
    BDSP_Raaga_Audio_CapInputPort_eExtI2s0,         /* External I2S Capture port */
    BDSP_Raaga_Audio_CapInputPort_eRfI2s,         /* BTSC Capture port  */
    BDSP_Raaga_Audio_CapInputPort_eSpdif,           /* SPDIF Capture port   */
    BDSP_Raaga_Audio_CapInputPort_eHdmi,            /* HDMI */
    BDSP_Raaga_Audio_CapInputPort_eAdc,
    BDSP_Raaga_Audio_CapInputPort_eIntCapPort4,     /* Internal Capture port 4 */
    BDSP_Raaga_Audio_CapInputPort_eIntCapPort5,     /* Internal Capture port 5 */
    BDSP_Raaga_Audio_CapInputPort_eIntCapPort6,     /* Internal Capture port 6 */
    BDSP_Raaga_Audio_CapInputPort_eIntCapPort7,     /* Internal Capture port 7 */
    BDSP_Raaga_Audio_CapInputPort_eDummy,          /* This enum is used for File based input.... Mainly applicable to Certification purposes*/
    BDSP_Raaga_Audio_CapInputPort_eMax,            /* Invalid/last Entry */
    BDSP_Raaga_Audio_CapInputPort_eInvalid = 0x7FFFFFFF

} BDSP_Raaga_Audio_CapInputPort;

/*
    This data structure defines AVL configuration parameters
*/

typedef struct BDSP_Raaga_Audio_AVLConfigParams
{
   uint32_t  ui32AVLEnableFlag;     /* Default value = Enable:  Enable =1 Disable =0 */
   int32_t   i32Target;             /* -20*32768 Target level of output signal in terms of dBFs Q15   */
   int32_t   i32LowerBound;         /* -31*32768 Lower bound for primary range of volume control Q15   */
   int32_t   i32FixedBoost;         /* 11*32768   Amount of fixed boost (in dB) if level < LOWERBOUND Q15  */
   int32_t   i32RefLevel;           /* 2959245  (20*log10(32768)) Reference logarithmic gain value of 0 dBFs Q15 */
   int32_t   i32Alpha;              /* 32736    (1023/1024)     Attenuation factor for Level Maximum Q15    */
   int32_t   i32Beta;               /* 32256    (63/64)         Attenuation factor for Level Mean Q15      */
   int32_t   i32ActiveThreshold;    /* 9830     (0.3)           Threshold for detecting active portion of signal Q15   */
   int32_t   i32DTFPCNT;            /* 750      (750)           Decay Time for DTF Percent delay (in miliseconds) Q0    */
   int32_t   i32Alpha2;             /* 32767    (32767/32768)   Attenuation factor for Level Minimum Q15    */
   int32_t   i32NSFGR_SEC;          /* 16384    (0.5)           Fast Gain Ramp in milliseconds Q15       */
   int32_t   i32DTF;                /* 9830     (0.3)           Decay Time Fraction Q15          */
   int32_t   i32LoudnessLevelEq;    /* Default value = 1:       Enable =1 Disable =0 */

}BDSP_Raaga_Audio_AVLConfigParams;


/*
    This data structure defines SRS Tru Surround configuration parameters
*/

typedef enum BDSP_Raaga_Audio_SpeakerSize
{
    BDSP_Raaga_Audio_SpeakerSize_eLFResponse40Hz  = 0,
    BDSP_Raaga_Audio_SpeakerSize_eLFResponse60Hz  = 1,
    BDSP_Raaga_Audio_SpeakerSize_eLFResponse100Hz = 2,
    BDSP_Raaga_Audio_SpeakerSize_eLFResponse150Hz = 3,
    BDSP_Raaga_Audio_SpeakerSize_eLFResponse200Hz = 4,
    BDSP_Raaga_Audio_SpeakerSize_eLFResponse250Hz = 5,
    BDSP_Raaga_Audio_SpeakerSize_eLFResponse300Hz = 6,
    BDSP_Raaga_Audio_SpeakerSize_eLFResponse400Hz = 7,
    BDSP_Raaga_Audio_SpeakerSize_eLAST,
    BDSP_Raaga_Audio_SpeakerSize_eINVALID = 0x7fffffff

} BDSP_Raaga_Audio_SpeakerSize;

/*
    This data structure defines DVD LPCM configuration parameters
*/

typedef struct BDSP_Raaga_Audio_LpcmOutputConfig
{
    uint32_t    ui32OutMode;                    /* Output channel assignment mode */

    uint32_t    ui32LfeOnFlag;                  /* Flag for LFE
                                                   Default value = Disable:  Enable =1 Disable =0
                                                */
    uint32_t    ui32DualMonoModeFlag ;          /* Mode for source selection in dual-mono LPCM-VR streams
                                                   Default value = Disable:  Enable =1 Disable =0
                                                */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

}BDSP_Raaga_Audio_LpcmOutputConfig ;                /* Output configuration structure for an output port */

typedef struct BDSP_Raaga_Audio_LpcmUserConfig
{
    uint32_t                        ui32NumOutputPorts;    /* Number of output ports */
    BDSP_Raaga_Audio_LpcmOutputConfig   sOutputConfig[2];      /* Array of output configuration structures, one for each output port  */
    /*
     * Currently removing the provision for user defined downmix tables (FWRAA-453)
     */
    /*uint32_t    ui32UseUserDownmixCoeffsFlag;*/   /* Flag to use user downmix coefficients.
                                                   Default value = Disable:  Enable =1 Disable =0
                                                */
    /*int32_t                         i32UserDownmixTables[16][8][8];*/  /* 16x8x8 User DownMix tables - decoder uses only 1 such 8x8 table for a given frame  */

}BDSP_Raaga_Audio_LpcmUserConfig;


/*
    This data structure defines TsmCorrection configuration parameters
*/

typedef struct BDSP_Raaga_Audio_TsmCorrectionConfigParams
{
    /* 0- No Correction (default), 1-DSOLA, 2-SOFT-PPM*
    No Correction: Input will be bypassed. This mode is just bypass tight lipsync correction.
    This mode may get used for certification.

    DSOLA: In this mode, frame will get compress/expand based on the PTS and STC diff.
    If the STC and PTS diff is well within the bound, input will be bypassed to output.
    The correction will be done in one time.

    Soft PPM: In this mode, frame will get compress/expand slowly based on PTS and STC diff.
    If the STC and PTS diff is well within the bound, input will be bypassed to output.*/

    uint32_t ui32TsmCorrectionMode;

}BDSP_Raaga_Audio_TsmCorrectionConfigParams;

/*
    This data structure defines dsola configuration parameters
*/

typedef struct BDSP_Raaga_Audio_DsolaConfigParams
{
     uint32_t ui32InputPcmFrameSize; /* Default value 0x266 */

}BDSP_Raaga_Audio_DsolaConfigParams;

/*
    This data structure defines Fade Control configuration parameters
*/

typedef struct BDSP_Raaga_Audio_FadeCtrlConfigParams
{
    /*  This
    parameter specify the target level in % so the range will be 0 - 100.
    While programming, this range must mapped in 1.31 format as mentioned
    below. Codec will assume default as : 0x7FFFFFFF (which is 100) */
    uint32_t   ui32VolumeTargetLevel;

    /* FadeIn/FadeOut Easing function selection: This supports 0- Linear, 1-cubic, 2-OutCubic Default: 0(Linear)*/
    uint32_t   ui32EasingFunctionType;

    /* Duration in samples. APE need to program duration in samples assuming that the sampling frequency is 48KHz.
    The range supported  3 - 60000 msec (60 seconds): Default: 4800 (100 msec) */
    uint32_t  ui32DurationInSamples;

} BDSP_Raaga_Audio_FadeCtrlConfigParams;


/*
    This data structure defines Ambisonics PP configuration parameters
*/
typedef struct BDSP_Raaga_Audio_AmbisonicsConfigParams
{
    /* Flag to indicate if the incoming audio is in 1st order B-Format Ambisonic (WXYZ) form
        1 - True (Ambisonic Content, Default)
        0 - False (Non ambisonic content, use ambisonic encoding)
    */
    uint32_t ui32AmbisonicProcess;

    /* Flag to indicate if Binaural rendering to headphones is required
        0 - Disable
        1 - Enable (Default)
    */
    uint32_t ui32BinauralRendering;

    /* Anti clockwise Rotation along the z axis in degrees (0-359)
        Default = 0
    */
    uint32_t ui32Yaw;

    /* Anti clockwise Rotation along the x axis in degrees (0-359)
        Default = 0
    */
    uint32_t ui32Pitch;

    /* Anti clockwise Rotation along the y axis in degrees (0-359)
        Default = 0
    */
    uint32_t ui32Roll;

} BDSP_Raaga_Audio_AmbisonicsConfigParams;

/*
    This data structure defines DTS core configuration parameters
*/
typedef struct  BDSP_Raaga_Audio_DtsCoreUserConfig
{
    int32_t     i32OutputMode;                                  /* Default: 2 */
    int32_t     i32OutLfe;                                      /* Default: 0 */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];/* Default Channel Matrix = {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF} */

}BDSP_Raaga_Audio_DtsCoreUserConfig;

typedef struct  BDSP_Raaga_Audio_DtsCoreConfigParams
{
    int32_t                         i32NumOutPorts;     /* Default: 1 */
    BDSP_Raaga_Audio_DtsCoreUserConfig   sUserOutputCfg[2]; /* These are user config parameters required from user  */

}BDSP_Raaga_Audio_DtsCoreConfigParams;

/*
    This data structure defines DTS broadcast decoder configuration parameters
*/
typedef struct BDSP_Raaga_Audio_DtshdUserOutput
{
    int32_t i32UserDRCFlag;     /*  Default =0      Range : 0,1 (0-> DRC enabled; 1-> DRC disabled)
                                    This Flag is used to turn on/off dynamic range compression
                                */

    int32_t i32DynScaleHigh;
                                /*  Default = 0x7fffffff
                                    Range 0 to 0x7fffffff (0 to 100%)
                                    Dynamic range compression cut scale factor. Its in Q1.31 format.*/

    int32_t i32DynScaleLow;

                                /*  Default = 0x7fffffff
                                    Range 0 to 0x7fffffff (0 to 100%)
                                    Dynamic range compression boost scale factor. Its in Q1.31 format.
                                */

    uint32_t ui32OutMode;       /*Default =7;
                                Output channel configuration */
                                /* This field indicates Ouput channel Configuration
                                    0 = Two_mono_channels_1_1_ch1_ch2
                                    1 = One_centre_channel_1_0_C
                                    2 = Two_channels_2_0_L__R
                                    3 = Three_channels_3_0_L_C_R
                                    4 = Three_chanels_2_1_L_R_S
                                    5 = Four_channels_3_1_L_C_R_S
                                    6 = Four_channels_2_2_L_R_SL_SR
                                    7 = Five_channels_3_2_L_C_R_SL_SR */

    uint32_t ui32OutLfe;
                                /* Default = 1
                                   Range : 0,1 (0-> LFE disabled;1-> LFE enabled)
                                   Flag used to enable/disable LFE channel output */

    uint32_t ui32DualMode;
                                /* Default =2
                                   Range : 0-3 (0-> DUALLEFT_MONO; 1->DUALRIGHT_MONO;2->STEREO;3->DUAL MIX MONO)
                                   Configure decoder output for dual mode */

     uint32_t ui32StereoMode;

                                /*  Default=0
                                    Range : 0,1 (1->Lt/Rt downmix;0->Normal output)
                                    Perform stereo downmix of decoder output */

    uint32_t ui32AppSampleRate;
                                /* Default : 48000
                                   Range : 192,000, 176400, 96000, 88200, 48000, 44100 , 320000
                                   This is the sampling rate set by application layer. The decoder will limit decoding additional components in the stream (e.g, X96) to adjust the sampling rate of the decoded output, to that set by the application
                                 */

    uint32_t ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
                                /*Here BDSP_AF_P_MAX_CHANNELS = 8 -> see the detail description in bottom */

     uint32_t ui32ExtdnmixEnabled;
                                /* Default=0
                                    Range : 0,1 (0->Disable external downmix;1->Enable external downmix) */

    int32_t i32ExtDnmixTab[BDSP_AF_P_MAX_CHANNELS][BDSP_AF_P_MAX_CHANNELS];
                                /* Here BDSP_AF_P_MAX_CHANNELS = 8*/
                                /*Default all zeros; User specified downmix coefficients can be given through this matrix*/

}BDSP_Raaga_Audio_DtshdUserOutput;

/*
   This data structure defines DTS-HD decoder user configuration parameters
*/
typedef struct BDSP_Raaga_Audio_DtsHdConfigParams
{
    uint32_t ui32DecodeCoreOnly;
                                /*Default=1; Range : 0,1 (1-> Decode only core, Disable all extension components)
                                 This flag has precedence over other decode flags like  ui32DecodeDtsOnly,  ui32DecodeXLL etc) */
    uint32_t ui32DecodeDtsOnly;
                                /*Default=0; Range : 0,1 (1-> decode components present in core substream only)*/
    uint32_t ui32DecodeXLL;
                                /*Default=1; Range : 0,1 (0-> does not decode XLL component)*/
    uint32_t ui32DecodeX96;
                                /*Default=1; Range : 0,1 (0-> does not decode X96 component)*/
    uint32_t ui32DecodeXCH;
                                /*Default=1; Range : 0,1 (0-> does not decode XCH component)*/
    uint32_t ui32DecodeXXCH;
                                /*Default=1; Range : 0,1 (0-> does not decode XXCH component)*/
    uint32_t ui32DecodeXBR;
                                /*Default=1; Range : 0,1 (0-> does not decode XBR component)*/
    uint32_t ui32EnableSpkrRemapping;
                                /*Default=0; Range : 0,1 (1->Enable Speaker remapping; 0->Disable speaker remapping)*/
    uint32_t ui32SpkrOut;
                                /*Default=2123*/
                                            /*Specifies the speaker out configuration. Instructs the decoder to perform downmixing or speaker remapping to yield the desired output speaker configuration.
                                                SpkrOut - Output Speaker Configuration
                                                0  - Use Encoded Native (default)
                                                1  - C
                                                9  - C LFE1
                                                2  - L R
                                                10  - L R LFE1
                                                262144  Lt Rt (downmix)
                                                3  - C L R
                                                11  - C L R LFE1
                                                18  - L R Cs
                                                26  - L R Cs LFE1
                                                19  - C L R Cs
                                                27  - C L R Cs LFE1
                                                6  - L R Ls Rs
                                                14  - L R Ls Rs LFE1
                                                15  - C L R Ls Rs LFE1
                                                8207  - C L R Ls Rs LFE1 Lhs Rhs
                                                2123  - C L R LFE1 Lsr Rsr Lss Rss
                                                47  - C L R Ls Rs LFE1 Lh Rh
                                                79  - C L R Ls Rs LFE1 Lsr Rsr
                                                159  - C L R Ls Rs LFE1 Cs Ch
                                                287  - C L R Ls Rs LFE1 Cs Oh
                                                1039  - C L R Ls Rs LFE1 Lw Rw
                                                31- C L R Ls Rs LFE1 Cs */

    uint32_t ui32MixLFE2Primary;
                                                /*Default =0; Range : 0,1 (1-> Mix LFE to primary while downmixing, when Lfe output is disabled)*/
    uint32_t ui32ChReplacementSet;
                                                /*Default =0*/
                                                /*It can take all possible values from 0 to 7. But it is not really required in core decoder. But, it will be needed in DTS-HD decoder going forward.*/
    uint32_t i32NumOutPorts;
                                                /* Default =2; Range : 1,2 (2-> enables PCM output and concurrent stereo)*/
    uint32_t  ui32EnableMetadataProcessing;     /* Default =0; Range : 0,1 (0->Disable metadata processing, 1->Enable metadata processing*/

    BDSP_Raaga_Audio_DtshdUserOutput sUserOutputCfg[2];

    /* This enum lets the mixer know if the decoder type is primary or secondary or sound effects. */
    BDSP_AF_P_DecoderType   eDecoderType; /* Default : BDSP_AF_P_DecoderType_ePrimary */
}BDSP_Raaga_Audio_DtsHdConfigParams;


/*
   This data structure defines DTS LBR decoder configuration parameters
*/
typedef struct BDSP_Raaga_Audio_DtsLbrUserOutput
{
    uint32_t           ui32bEnableDialNorm;                                   /*  Default : 1     Range : [0,1] (0-> Dialnorm disabled; 1-> Dialnorm enabled)
                                                This Flag is used to turn on/off Dialnorm */

    uint32_t           ui32EnableDRC;                                                                                /*  Default =0    Range : [0,1] (0-> DRC disabled; 1-> DRC enabled)
                                                This Flag is used to turn on/off dynamic range compression */

    uint32_t           ui32DRCPercent;                                                                              /*  Default = 0x7fffffff (100 %)
                                                Range 0 to 0x7fffffff (0 to 100%) */

    uint32_t           ui32OutMode;                                                                  /*Default =7;
                                                Output channel configuration, this is according to BCOM_eACMOD */

    uint32_t           ui32OutLfe;                                                                                        /* Default = 1
                                                   Range : 0,1 (0-> LFE disabled;1-> LFE enabled)
                                                   Flag used to enable/disable LFE channel output */

    uint32_t           ui32DualMode;                                                                                 /* Default =2
                                                   Range : 0-3 (0-> DUALLEFT_MONO; 1->DUALRIGHT_MONO;2->STEREO;3->DUAL MIX MONO)
                                                   Configure decoder output for dual mode */

     uint32_t          ui32StereoMode;                                                                            /*  Default=0
                                                    Range : 0,1 (1->Lt/Rt downmix;0->Normal output)
                                                    Perform stereo downmix of decoder output */

    uint32_t           ui32MixLFE2Primary;                                                      /*Default =0; Range : 0,1 (1-> Mix LFE to primary while downmixing, when Lfe output is disabled)*/

    uint32_t           ui32bEnableSpkrRemapping;                      /*Default =0; Range : 0,1 (1-> enable speaker remapping , 1-> disable speaker remapping )*/
    uint32_t           ui32SpkrOut;                                                                      /*[required output speaker configurations, default 15 , can take value from the following values*/
                                                /*
                                                ui32SpkrOut =
                                                0       - Use Encoded Native, no speaker remapping
                                                15      - C L R         Ls                            Rs                           LFE1
                                                8207    - C L R       Ls                            Rs                           LFE1       Lhs                         Rhs
                                                2123    - C L R       LFE1       Lsr                          Rsr                          Lss                          Rss
                                                47      - C L R         Ls                            Rs                           LFE1       Lh                           Rh
                                                79      - C L R         Ls                            Rs                           LFE1       Lsr                          Rsr
                                                78      - L R Ls        Rs                           LFE1       Lsr                          Rsr
                                                159     - C L R        Ls                            Rs                           LFE1       Cs                           Ch
                                                287     - C L R        Ls                            Rs                           LFE1       Cs                           Oh
                                                1039    - C L R       Ls                            Rs                           LFE1       Lw                          Rw
                                                31      - C L R         Ls                            Rs                           LFE1       Cs
                                                30      - L R Ls        Rs                           LFE1       Cs*/


    uint32_t           ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
                                                /*Here BDSP_AF_P_MAX_CHANNELS = 8 */
                                                /*This matrix decides the output order of channels

                                                    For Multi-Channel (5.1) ports:

                                                    ui32OutputChannelMatrix[0] = 0;
                                                    ui32OutputChannelMatrix[1] = 1;
                                                    ui32OutputChannelMatrix[2] = 2;
                                                    ui32OutputChannelMatrix[3] = 3;
                                                    ui32OutputChannelMatrix[4] = 4;
                                                    ui32OutputChannelMatrix[5] = 5;
                                                    ui32OutputChannelMatrix[6] = 0xffffffff; this value shows invalid
                                                    ui32OutputChannelMatrix[7] = 0xffffffff; this value shows invalid

                                                    For Stereo Output Port:

                                                    ui32OutputChannelMatrix[0] = 0;
                                                    ui32OutputChannelMatrix[1] = 1;
                                                    for(i=2; i<8; i++)
                                                        ui32OutputChannelMatrix[i] = 0xffffffff; this value shows invalid
                                                    */

}BDSP_Raaga_Audio_DtsLbrUserOutput;


/*
   This data structure defines DTS-LBR decoder user configuration parameters
*/
typedef struct BDSP_Raaga_Audio_DtslbrConfigParams
{
    uint32_t         i32bIsPrimaryDecode;                                                    /*Default: 1, Range [0,1] , 0 -> primary, 1-> secondary*/
    uint32_t         i32bDecodeEmbeddedStereo;                                   /*Default: 0, Range [0,1] , 0 -> disabled, 1-> decode embedded stereo if present*/
    uint32_t         i32auxDynamicDownmixFlag;                                     /*default: 0, range: [0,1], [0, 1] = disableing/enabling Embedded downmix*/
    uint32_t         ui32DramMixerDataBufferAddress;                               /*Dram address for communicating Mixer metadata between Primary, Secondary and Mixer*/

    uint32_t         i32NumOutPorts;                                                                                             /* Default =1; Range : 1,2 (2-> enables PCM output and concurrent stereo)*/

    uint32_t         ui32IsMixingEnabled  ;              /* Default =0; 1-> If Mixing is enabled */                       /* Default =0; 1-> DVD */
    uint32_t         ui32PrimaryOutmode;                 /* When the decoder is configured for primary decoder or if there is no active primary decoder, the value of this variable will be invalid i.e. 0xFFFFFFFF ; If it is called in secondary mode then this variable will carry outmode of primary */

    BDSP_Raaga_Audio_DtsLbrUserOutput sUserOutputCfg[2];

}BDSP_Raaga_Audio_DtslbrConfigParams;


/*
    This data structure defines PCMWAV/PCM_AVI configuration parameters
*/
typedef struct BDSP_FWIF_P_PcmWavUserOutput
{   /*
    enum    {
            BCOM_eACMOD_MODE11, //CH1,CH2
            BCOM_eACMOD_MODE10, //C
            BCOM_eACMOD_MODE20, //L,R -->To be used for BufferOutputMode only.
            BCOM_eACMOD_MODE30, //L,C,R
            BCOM_eACMOD_MODE21, //L,R,S
            BCOM_eACMOD_MODE31, //L,C,R,S
            BCOM_eACMOD_MODE22, //L,R,LS,RS
            BCOM_eACMOD_MODE32, //L,C,R,LS,RS
            BCOM_eACMOD_INVALID=0xffffffff
    };
    default - 2*/
    int32_t i32OutMode;
    /*  preferred stereo mode if i32OutMode = STEREO
        enum { GBL_STEREOMODE_LORO = 0, GBL_STEREOMODE_LTRT };
        i32StereoMode = 0 is default value ;
    */
    int32_t i32StereoMode;
    /* dual mono downmix mode
       enum { GBL_DUAL_STEREO=0, GBL_DUAL_LEFTMONO, GBL_DUAL_RGHTMONO, GBL_DUAL_MIXMONO };
       i32DualMode = 0 is default value;
    */
    int32_t i32DualMode;
    /*
     * Downmix coefficients for each channel
     */
    int32_t i32DownmixCoeffs[BDSP_AF_P_MAX_CHANNELS];

    /*This is channel matrix of size 8 where each index can take any values from 0 to 7
      Depending upon  the channel routing done
    */
    uint32_t   ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

}BDSP_FWIF_P_PcmWavUserOutput;

typedef struct BDSP_Raaga_Audio_PcmWavConfigParams
{

    uint32_t    ui32NumOutputPorts;
     /* This enum lets the mixer know if the decoder type is primary or secondary or sound effects or application audio. */
    BDSP_AF_P_DecoderType               eDecoderType;
    BDSP_FWIF_P_PcmWavUserOutput sUserOutputCfg[2];

}BDSP_Raaga_Audio_PcmWavConfigParams;

/*
    Bit rates supported in MP3 encoder
*/
typedef enum    BDSP_Raaga_Audio_Mp3EncodeBitRate
{
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e32kbps = 32,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e40kbps = 40,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e48kbps = 48,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e56kbps = 56,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e64kbps = 64,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e80kbps = 80,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e96kbps = 96,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e112kbps = 112,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e128kbps = 128,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e160kbps = 160,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e192kbps = 192,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e224kbps = 224,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e256kbps = 256,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e320kbps = 320,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_eLast,
    BDSP_Raaga_Audio_Mp3EncodeBitRate_eInvalid = 0x7fffffff
}BDSP_Raaga_Audio_Mp3EncodeBitRate;

/*
    Emphasis types in mp3 encoder
*/
typedef enum    BDSP_Raaga_Audio_Mp3EncodeEmphasisType
{
    BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eNone,
    BDSP_Raaga_Audio_Mp3EncodeEmphasisType_e50_15uSeconds,
    BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eReserved,
    BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eCCITTJ_17,
    BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eLast,
    BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eInvalid = 0x7fffffff
}BDSP_Raaga_Audio_Mp3EncodeEmphasisType;

typedef enum
{
    BDSP_Raaga_Audio_Mp3EncodeMono_Left      = 0,
    BDSP_Raaga_Audio_Mp3EncodeMono_Right     = 1,
    BDSP_Raaga_Audio_Mp3EncodeMono_Mix       = 2,
    BDSP_Raaga_Audio_Mp3EncodeMono_DualMono  = 3
}BDSP_Raaga_Audio_Mp3EncodeMonoChannelSelect;

/*
    MPEG1-Layer-3 encoder user configuration structure
*/
typedef struct BDSP_Raaga_Audio_Mpeg1L3EncConfigParams
{
    /* Default Value = BDSP_Raaga_Audio_Mp3EncodeBitRate_e128kbps; */
    BDSP_Raaga_Audio_Mp3EncodeBitRate eMp3EncodeBitRate;

    /* CRC Not Supported */
    /* Default Value = 0; */
    uint32_t ui32AddCRCProtect;

    /* Controls private bit setting in the header */
    /* Bit for private use, Default Value = BDSP_AF_P_eDisable */
    BDSP_AF_P_EnableDisable ePrivateBit;

    /* Cpyright bit setting in the header */
    /* Default Value = BDSP_AF_P_eDisable; */
    BDSP_AF_P_EnableDisable eCopyright;

    /* Original bit setting in the header */
    /* Default Value = BDSP_AF_P_eDisable */
    BDSP_AF_P_EnableDisable eOriginal;

    /* Indicates type of de-emphasis that shall be used */
    /* Default Value = BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eNone; */
    BDSP_Raaga_Audio_Mp3EncodeEmphasisType eEmphasisType;

    /* A85 Compliant*/
    int32_t                         i32InputVolLevel;               /* in dB */
    int32_t                         i32OutputVolLevel;              /* in dB */

    /* Flag that indicates to encode in stereo or mono Default=0 (stereo)*/
    uint32_t                        ui32bEncodeMono;

    BDSP_Raaga_Audio_Mp3EncodeMonoChannelSelect eMp3EncodeMonoChannelSelect;

}BDSP_Raaga_Audio_Mpeg1L3EncConfigParams;

/*
   This data structure defines AMR Decoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_AmrUsrCfg
{
    uint32_t    ui32OutMode;    /* Default = 1; Output channel configuration */
    uint32_t    ui32ScaleOp;    /* Default = 0; 0 -> FALSE, 1 -> TRUE */
    uint32_t    ui32ScaleIdx;   /* Default = 0; 0 -> +3dB, 1 -> +6dB, 2 -> +9dB, 3 -> +12dB */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

        /*
        For Multichannel (5.1) Output Port:
                    ui32OutputChannelMatrix[0] = 0;
                    ui32OutputChannelMatrix[1] = 1;

                    for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;

        For Stereo Output Port:
                    for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;  this value shows invalid
        */


} BDSP_Raaga_Audio_AmrUsrCfg;

typedef struct  BDSP_Raaga_Audio_AmrConfigParams
{
    uint32_t                ui32NumOutPorts;    /* Default = 1; 1 -> Multichannel out, 2 -> Multichannel + Stereo out */

    BDSP_Raaga_Audio_AmrUsrCfg  sUsrOutputCfg[2];

} BDSP_Raaga_Audio_AmrConfigParams;

/*
   This data structure defines AMRWB Decoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_AmrwbdecUsrCfg
{
    uint32_t    ui32OutMode;    /* Default = 1; Output channel configuration */
    uint32_t    ui32ScaleOp;    /* Default = 0; 0 -> FALSE, 1 -> TRUE */
    uint32_t    ui32ScaleIdx;   /* Default = 0; 0 -> +3dB, 1 -> +6dB, 2 -> +9dB, 3 -> +12dB */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

        /*
        For Multichannel (5.1) Output Port:
                    ui32OutputChannelMatrix[0] = 0;
                    ui32OutputChannelMatrix[1] = 1;

                    for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;

        For Stereo Output Port:
                    for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;  this value shows invalid
        */


} BDSP_Raaga_Audio_AmrwbdecUsrCfg;

typedef struct  BDSP_Raaga_Audio_AmrwbdecConfigParams
{
    uint32_t                ui32NumOutPorts;    /* Default = 1; 1 -> Multichannel out, 2 -> Multichannel + Stereo out */

    BDSP_Raaga_Audio_AmrwbdecUsrCfg sUsrOutputCfg[2];

} BDSP_Raaga_Audio_AmrwbdecConfigParams;


typedef struct  BDSP_Raaga_Audio_iLBCdecUsrCfg
{
    uint32_t    mode;           /* Default = 20; 20->frame length 20msec, 30->frame length 30msec*/
    uint32_t    plc;            /* Default = 0; 0 -> no pakel loss, 1 -> packet loss*/
    uint32_t    ui32OutMode;    /* Default = 1; Output channel configuration */
    uint32_t    ui32ScaleOp;    /* Default = 0; 0 -> FALSE, 1 -> TRUE */
    uint32_t    ui32ScaleIdx;   /* Default = 0; 0 -> +3dB, 1 -> +6dB, 2 -> +9dB, 3 -> +12dB */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

        /*
        For Multichannel (5.1) Output Port:
                    ui32OutputChannelMatrix[0] = 0;
                    ui32OutputChannelMatrix[1] = 1;

                    for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;

        For Stereo Output Port:
                    for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;  this value shows invalid
        */


} BDSP_Raaga_Audio_iLBCdecUsrCfg;

typedef struct  BDSP_Raaga_Audio_iLBCdecConfigParams
{
    uint32_t                ui32NumOutPorts;    /* Default = 1; 1 -> Multichannel out, 2 -> Multichannel + Stereo out */

    BDSP_Raaga_Audio_iLBCdecUsrCfg  sUsrOutputCfg[2];

} BDSP_Raaga_Audio_iLBCdecConfigParams;



typedef struct  BDSP_Raaga_Audio_isacdecUsrCfg
{
    uint32_t    ui32BandMode;   /* Default = 0; 0 -> Narrowband, 1 -> Wideband  */
    uint32_t    plc;            /* Default = 0; 0 -> no pakel loss, 1 -> packet loss*/
    uint32_t    ui32OutMode;    /* Default = 1; Output channel configuration */
    uint32_t    ui32ScaleOp;    /* Default = 0; 0 -> FALSE, 1 -> TRUE */
    uint32_t    ui32ScaleIdx;   /* Default = 0; 0 -> +3dB, 1 -> +6dB, 2 -> +9dB, 3 -> +12dB */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
        /*
        For Multichannel (5.1) Output Port:
                    ui32OutputChannelMatrix[0] = 0;
                    ui32OutputChannelMatrix[1] = 1;

                    for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;

        For Stereo Output Port:
                    for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;  this value shows invalid
        */

} BDSP_Raaga_Audio_isacdecUsrCfg;

typedef struct BDSP_Raaga_Audio_iSACdecConfigParams
{
    uint32_t    ui32NumOutPorts;      /*  Default = 1; 1 -> Multichannel out, 2 -> Multichannel + Stereo out */

    BDSP_Raaga_Audio_isacdecUsrCfg sUsrOutputCfg[2];

}BDSP_Raaga_Audio_iSACdecConfigParams;


typedef struct BDSP_Raaga_Audio_AmrEncoderUserConfig
{
    /*Enables or Disable DTX */
    uint32_t     ui32DtxEnable; /* ui32DtxEnable =1 => DTX Enable
                                   ui32DtxDisable=0 => DTX Disable
                                   Default => ui32DtxEnable=0 (Disabled)*/
    /*Set required Bitrate */
    uint32_t     ui32Bitrate;   /*ui32Bitrate = 0x0000 >> Mode Name => MR475 BitRate => 4.75 kbit/s
                                  ui32Bitrate = 0x0001 >> Mode Name => MR515   BitRate => 5.15 kbit/s
                                  ui32Bitrate = 0x0002 >> Mode Name => MR59    BitRate => 5.90 kbit/s
                                  ui32Bitrate = 0x0003 >> Mode Name => MR67    BitRate => 6.70 kbit/s
                                  ui32Bitrate = 0x0004 >> Mode Name => MR74    BitRate => 7.40 kbit/s
                                  ui32Bitrate = 0x0005 >> Mode Name => MR795   BitRate => 7.95 kbit/s
                                  ui32Bitrate = 0x0006 >> Mode Name => MR102  BitRate => 10.20 kbit/s
                                  ui32Bitrate = 0x0007 >> Mode Name => MR122  BitRate => 12.20 kbit/s
                                  Default => ui32Bitrate = 0x0000 >> Mode Name => MR475 BitRate => 4.75 kbit/s*/
} BDSP_Raaga_Audio_AmrEncoderUserConfig;

typedef struct  BDSP_Raaga_Audio_AmrwbplusEncConfigParams
{
    int16_t mode;             /* AMR_WB core mode: 0..8 */
    int16_t extension;        /* 0=AMRWB, 1=mono, 2=stereo20%, 3=stereo25% */
    int16_t st_mode;          /* stereo mode 0..13 (not used, for ericsson cmd line?) */
    int16_t fscale;           /* frequency scaling */
    int16_t use_case_mode;    /* use case (for AMRWB+ only) */
    int16_t allow_dtx;        /* dtx (for AMRWB only) */
    int16_t FileFormat;
    int16_t mode_index;
    int16_t fscale_index;
    int16_t bc;             /* Backward compatible file format*/
    int16_t num_channels;
    int32_t sampling_freq;

}BDSP_Raaga_Audio_AmrwbplusEncConfigParams;

typedef struct  BDSP_Raaga_Audio_AmrwbplusUsrCfg
{

  int32_t fs;
  int32_t mono_dec_stereo;
  int32_t limiter_on ;
  int16_t FileFormat;
  int16_t fer_sim;                /* frame errasures simulation */

  uint32_t  ui32OutMode;    /* Default = 1; Output channel configuration */
  uint32_t  ui32ScaleOp;    /* Default = 0; 0 -> FALSE, 1 -> TRUE */
  uint32_t  ui32ScaleIdx;   /* Default = 0; 0 -> +3dB, 1 -> +6dB, 2 -> +9dB, 3 -> +12dB */
  uint32_t  ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

        /*
        For Multichannel (5.1) Output Port:
                    ui32OutputChannelMatrix[0] = 4;
                    ui32OutputChannelMatrix[1] = 4;

                    for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;

        For Stereo Output Port:
                    for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
                            ui32OutputChannelMatrix[i] = 0xFFFFFFFF;  this value shows invalid
        */


} BDSP_Raaga_Audio_AmrwbplusUsrCfg;

typedef struct  BDSP_Raaga_Audio_AmrwbplusConfigParams
{
    uint32_t                ui32NumOutPorts;    /* Default = 1; 1 -> Multichannel out, 2 -> Multichannel + Stereo out */

    BDSP_Raaga_Audio_AmrwbplusUsrCfg sUsrOutputCfg[2];

} BDSP_Raaga_Audio_AmrwbplusConfigParams;



/*
   This data structure defines DRA Decoder user configuration parameters
*/


typedef struct BDSP_Raaga_Audio_DraUserOutput
{
    uint32_t ui32OutMode;   /*
                            Default = 2
                            Range - 1, 2, 7
                            1 - Downmix to mono
                            2 - Downmix to stereo
                            7 - Downmix to 5.1
                            The decoded output is downmixed to output configuration based on ui32OutMode value*/
    uint32_t ui32OutLfe;
                            /*
                            Default=1
                            Range - 0, 1
                            0 - LFE disabled at output
                            1 - LFE enabled at output*/
    uint32_t ui32StereoMode;
                            /*
                            Default = 0
                            Range - 0, 1
                            0 - LoRo downmixed output
                            1 - LtRt downmixed output
                            This value is relevant only when ui32OutMode is 2.*/

    uint32_t ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
                        /*This matrix decides the output order of channels

                            For Multi-Channel (5.1) ports:

                            ui32OutputChannelMatrix[0] = 0;
                            ui32OutputChannelMatrix[1] = 1;
                            ui32OutputChannelMatrix[2] = 2;
                            ui32OutputChannelMatrix[3] = 3;
                            ui32OutputChannelMatrix[4] = 4;
                            ui32OutputChannelMatrix[5] = 5;
                            ui32OutputChannelMatrix[6] = 0xffffffff; this value shows invalid
                            ui32OutputChannelMatrix[7] = 0xffffffff; this value shows invalid

                            For Stereo Output Port:

                                    ui32OutputChannelMatrix[0] = 0;
                                    ui32OutputChannelMatrix[1] = 1;
                                    for(i=2; i<8; i++)
                                        ui32OutputChannelMatrix[i] = 0xffffffff; this value shows invalid
                        */
}BDSP_Raaga_Audio_DraUserOutput;


typedef struct BDSP_Raaga_Audio_DraConfigParams
{
    uint32_t                    ui32NumOutPorts;    /*Default = 2;  5.1 PCM out and concurrent Stereo */
    BDSP_Raaga_Audio_DraUserOutput  sUserOutputCfg[2];

}BDSP_Raaga_Audio_DraConfigParams;


/*
   This data structure defines Real Audio LBR Decoder user configuration parameters
*/

typedef struct BDSP_Raaga_Audio_ralbr_UserOutput
{
    uint32_t            ui32OutMode;
                    /*  Default = 2
                        Value according to BCOM_eACMOD
                    */

    uint32_t            ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
                    /*  This matrix decides the output order of channels

                        For Multi-Channel (5.1) ports:

                        ui32OutputChannelMatrix[0] = 0;
                        ui32OutputChannelMatrix[1] = 1;
                        ui32OutputChannelMatrix[2] = 2;
                        ui32OutputChannelMatrix[3] = 3;
                        ui32OutputChannelMatrix[4] = 4;
                        ui32OutputChannelMatrix[5] = 5;
                        ui32OutputChannelMatrix[6] = 0xffffffff;
                        this value shows invalid
                        ui32OutputChannelMatrix[7] = 0xffffffff; this value shows invalid

                    For Stereo Output Port:

                        ui32OutputChannelMatrix[0] = 0;
                        ui32OutputChannelMatrix[1] = 1;

                        for(i=2; i<8; i++)
                            ui32OutputChannelMatrix[i] = 0xffffffff;

                        this value shows invalid
                    */

}BDSP_Raaga_Audio_RalbrUserOutput;

typedef struct BDSP_Raaga_Audio_RalbrConfigParams
{
    uint32_t            ui32NumOutPorts;

                        /*Default = 2;  5.1 PCM out and concurrent Stereo */

    BDSP_Raaga_Audio_RalbrUserOutput    sUserOutputCfg[2];

}BDSP_Raaga_Audio_RalbrConfigParams;


/*
   This data structure defines Dolby Pulse Decoder user configuration parameters
*/
typedef struct BDSP_Raaga_Audio_DolbyPulsePortConfig
{
    uint32_t    ui32OutMode;    /* Default = 2, Output channel configuration */

    uint32_t    ui32OutLfe;     /* Default = 0, Output LFE channel present */

    uint32_t    ui32DualMode;   /* Default = 0, Dual mono reproduction mode */

    uint32_t    ui32LoRoDownmix;/* Default = 0, 0 -> Lt/Rt Downmix, 1 -> Lo/Ro Downmix, 2 -> ARIB Downmix
                                   Dolby Pulse decoder can be configured for Lt/Rt downmixing only in the single decode usage case.
                                   If dual decode with mixing is enabled then both the primary and the secondary decode have to be set
                                   for Lo/Ro downmixing always and Lt/Rt downmixing should never be allowed. */

    uint32_t    ui32RfMode;     /* Default = 0, 0 -> Line Mode, 1 -> RF Mode, 2 -> RF mode(-23dB EBU), 3->RF Mode(-24dB ATSC) */

    uint32_t    ui32DrcCut;     /* Default = 100, Dynamic range compression cut scale factor in percent, 0 to 100 */

    uint32_t    ui32DrcBoost;   /* Default = 100, Dynamic range compression boost scale factor in percent, 0 to 100 */

    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

    /*
    For Multichannel (5.1) Output Port:
        for(i=0; i<6; i++)
            ui32OutputChannelMatrix[i] = i;

        for(i=6; i<BDSP_AF_P_MAX_CHANNELS; i++)
                ui32OutputChannelMatrix[i] = 0xFFFFFFFF; This represents INVALID


    For Stereo Output Port:
        for(i=0; i<2; i++)
            ui32OutputChannelMatrix[i] = i;

        for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
            ui32OutputChannelMatrix[i] = 0xFFFFFFFF; This represents INVALID
    */

    uint32_t    ui32ApplyGain;      /* Default = 0, 0 -> FALSE, 1 -> TRUE */

    int32_t     i32GainFactor;      /* Default = 0x00B504F3, corresponding to -3dB in Q8.24 format; valid only if ui32ApplyGain = 1 */

} BDSP_Raaga_Audio_DolbyPulsePortConfig;

typedef struct BDSP_Raaga_Audio_DolbyPulseUserConfig
{
    /* This enum lets the mixer know if the decoder type is primary or secondary or sound effects. */
    BDSP_AF_P_DecoderType   eDecoderType; /* Default : BDSP_AF_P_DecoderType_ePrimary */

    /* BDSP_AF_P_DolbyMsUsageMode_eMpegConformanceMode Should only be enabled for
    MPEG Conformance testing and in this only   one 5.1 Multichannel output port should be enabled.   */
    BDSP_AF_P_DolbyMsUsageMode      eDolbyPulseUsageMode; /* Default : BDSP_AF_P_DolbyMsUsageMode_eSingleDecodeMode */

    uint32_t    ui32NumOutPorts;    /* Default = 1, 1 -> Multichannel, 2 -> Multichannel + Stereo */

    uint32_t    ui32RefDialnormLevel;   /* Default = 124, corresponding to -31dB LKFS (31*4) */

    uint32_t    ui32DefDialnormLevel;   /* Default = 108, corresponding to -27dB LKFS (27*4); Maximum = 127 */


    BDSP_Raaga_Audio_DolbyPulsePortConfig   sOutPortCfg[2];

    /* Ensure that the multichannel and stereo ports have the similar loudness based on the selected RF mode: EBU or ATSC */
    uint32_t    ui32EnforceLoudnessLevelsOnAllPorts;    /* Default = 0, 0 -> Disable, 1 -> Enable */

} BDSP_Raaga_Audio_DolbyPulseUserConfig;

/*
   This data structure defines Broadcom 3D surround user configuration parameters
*/

typedef enum BDSP_Raaga_Audio_eSurroundMode
{
    BDSP_Raaga_Audio_eSurroundMode_NoSurround=0,
    BDSP_Raaga_Audio_eSurroundMode_StereoWidening,
    BDSP_Raaga_Audio_eSurroundMode_Broadcom3DSurround,
    BDSP_Raaga_Audio_eSurroundMode_LAST,
    BDSP_Raaga_Audio_eSurroundMode_INVALID=0x7fffffff

} BDSP_Raaga_Audio_eSurroundMode;

/* Different 3D surround modes correspond to different
surround speaker locations (theta) and height (phi) */
/* BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Centre
Speaker position      = 0
theta                 = 30
phi                   = 15
Speaker position      = 1
theta                 = 330
phi                   = 15
Speaker position      = 2
theta                 = 90
phi                   = 0
Speaker position      = 3
theta                 = 270
phi                   = 0
Speaker position      = 4
theta                 = 0
phi                   = 15
Speaker position      = 5
theta                 = 180
phi                   = 15
*/
/* BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Wide
Speaker position      = 0
theta                 = 30
phi                   = 15
Speaker position      = 1
theta                 = 330
phi                   = 15
Speaker position      = 2
theta                 = 105
phi                   = 30
Speaker position      = 3
theta                 = 255
phi                   = 30
Speaker position      = 4
theta                 = 0
phi                   = 15
Speaker position      = 5
theta                 = 180
phi                   = 15
*/
/* BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Extrawide
Speaker position      = 0
theta                 = 60
phi                   = 30
Speaker position      = 1
theta                 = 300
phi                   = 30
Speaker position      = 2
theta                 = 120
phi                   = 45
Speaker position      = 3
theta                 = 240
phi                   = 45
Speaker position      = 4
theta                 = 0
phi                   = 30
Speaker position      = 5
theta                 = 180
phi                   = 30
*/
typedef enum BDSP_Raaga_Audio_eBroadcom3DSurroundMode
{
    BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Centre=0,
    BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Wide,
    BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Extrawide,
    BDSP_Raaga_Audio_eBroadcom3DSurroundMode_LAST,
    BDSP_Raaga_Audio_eBroadcom3DSurroundMode_INVALID=0x7fffffff
} BDSP_Raaga_Audio_eBroadcom3DSurroundMode;

typedef struct BDSP_Raaga_Audio_Broadcom3DSurroundConfigParams
{
    int32_t                                  i32BRCM3DSurroundEnableFlag;    /*Default 1 Enable =1 Disable = 0*/
    BDSP_Raaga_Audio_eSurroundMode           eSurroundMode;                  /*Default BDSP_Raaga_Audio_eSurroundMode_Broadcom3DSurround*/
    BDSP_Raaga_Audio_eBroadcom3DSurroundMode eBroadcom3DSurroundMode;        /*Default BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Wide*/

}BDSP_Raaga_Audio_Broadcom3DSurroundConfigParams;

#define BDSP_Raaga_Audio_Brcm3DSurroundConfigParams BDSP_Raaga_Audio_Broadcom3DSurroundConfigParams

/*
   This data structure defines Mixer user configuration parameters
*/
typedef struct BDSP_Raaga_Audio_MixerConfigParams
{
    uint32_t        ui32NumInput;           /* The number of input ports to the mixer */
    uint32_t        MixingCoeffs[BDSP_AF_P_MAX_IP_FORKS][BDSP_AF_P_MAX_CHANNELS];
                                                              /* Q4.28 foremat : This field is only for sound effect mixing.
                                                                 Separate mixing coefficients can be provided
                                                                 per channel per input basis.
                                                              */
    int32_t         i32UserMixBalance;
    /* This field can be set to enable custom mixing of effect audio with primary*/
    /* default: disable (0) */
    int32_t         i32CustomEffectsAudioMixingEnable;
    /* These mixing coefficients will be used to mix effects audio if custom mixing mode is enabled
    These Mixing Coefficients can be negative and must be programmed in Q2.30 format. For example:
    scale factor as 0.5 must be programmed in Q2.30 as (0.5*(2^30)) =0x20000000. First index in
    this matrix represents the channels of primary input however second corresponds to effects
    audio input. The channel order is fixed as "Left, Right, Left Surround, Right Surround, Center,
    LFE" for both inputs.  The default is one to one matching. For example L->L and R->R and so on
     */
    int32_t  i32CustomMixingCoefficients[6][6];
		BDSP_AF_P_BurstFillType		  eFMMPauseBurstType;
		BDSP_AF_P_SpdifPauseWidth	   eSpdifPauseWidth;
}BDSP_Raaga_Audio_MixerConfigParams;

/*
   This data structure defines MS10 DD transcode  user configuration parameters
*/
typedef struct  BDSP_Raaga_Audio_DDTranscodeConfigParams
{
    /* Audio Coding mode, Possible Values 0(1+1),2(2/0),7(3/2), Default: 7(3/2)*/
    int32_t  i32AudCodMode;

    /*  1 is LFE enabeld: 0 is LFE disabled, Default Value: 1*/
    int32_t  i32LoFreqEffOn;

    /* This is used to enable/disable SPDIF header.  Enable =1 Disable =0. Default value = Enable.*/
    BDSP_AF_P_EnableDisable eSpdifHeaderEnable;

    /*This flag is set to enable when the Encoder is used in transcode mode Enable =1 Disable =0. Default value = Enable. */
    BDSP_AF_P_EnableDisable   eTranscodeEnable;

}BDSP_Raaga_Audio_DDTranscodeConfigParams;

/*

   This data structure defines Ms10 DDP decoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_DDPMs10UserOutput
{
    /* This is LFE ON/OFF flag and can take  two values 0 or 1
     default value for this field is 0
    */
    int32_t i32OutLfe;
    /*  enum { GBL_MODE11=0, GBL_MODE_RSVD=0, GBL_MODE10, GBL_MODE20,
        GBL_MODE30, GBL_MODE21, GBL_MODE31, GBL_MODE22, GBL_MODE32 };
        i32OutMode =7 default value;
    */
    int32_t i32OutMode;
    /*  preferred stereo mode
        enum { GBL_STEREOMODE_AUTO=0, GBL_STEREOMODE_SRND, GBL_STEREOMODE_STEREO };
        i32StereoMode = 0 is default value ;
    */
    int32_t i32StereoMode;
    /* dual mono downmix mode
       enum { GBL_DUAL_STEREO=0, GBL_DUAL_LEFTMONO, GBL_DUAL_RGHTMONO, GBL_DUAL_MIXMONO };
       i32DualMode = 0 is default value;
    */
    int32_t i32DualMode;
    /* karaoke capable mode
       enum { NO_VOCALS=0, GBL_VOCAL1, GBL_VOCAL2, GBL_BOTH_VOCALS };
       i32Kmode = 3;
    */
    int32_t i32Kmode;
    /* This i32ExtDnmixEnabled flag which can take  two values 0 or 1 based on disable/enable option
       default value for this i32ExtDnmixEnabled field is 0
    */
    int32_t i32ExtDnmixEnabled;
    int32_t i32ExtDnmixTab[DDP_DEC_GBL_MAXPCMCHANS][DDP_DEC_GBL_MAXPCMCHANS];
    /* This i32ExtKaraokeEnabled flag which can take  two values 0 or 1 based on disable/enable option
       default value for this i32ExtKaraokeEnabled field is 0
    */
    int32_t i32ExtKaraokeEnabled;
    int32_t i32Extv1Level;
    int32_t i32Extv1Pan;
    int32_t i32Extv2Level;
    int32_t i32Extv2Pan;
    int32_t i32ExtGmLevel;
    int32_t   i32ExtGmPan;
    /*This is channel matrix of size 8 where each index can take any values from 0 to 7
      Depending upon  the channel routing done
    */
    uint32_t   ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
}BDSP_Raaga_Audio_DDPMs10UserOutput;

typedef struct  BDSP_Raaga_Audio_DDPMs10ConfigParams
{
   /* p_user_cfg->i32CompMode = 2;*/
   /*Custom_0=0, Custom_1 = 1, Line = 2, RF =3 , DRC_OFF = 4, RF_23dB = 5, RF_24dB = 6 */
    int32_t                i32CompMode;

    /* p_user_cfg->i32PcmScale = 100;    The value is in percentage, 0 to 100. */
    int32_t                i32PcmScale;

    /* p_user_cfg->i3DynScaleHigh = 0x7FFFFFFF;    0x0 to  0x7FFFFFFF in Q1.31 format */
    int32_t                i32DynScaleHigh;

   /* p_user_cfg->i32DynScaleLow = 0x7FFFFFFF;    0x0 to  0x7FFFFFFF in Q1.31 format */
    int32_t                i32DynScaleLow;

    /*  This value "i32NumOutPorts" can be set to 1 or 2 based on output ports */
    int32_t                i32NumOutPorts;

    /* This value is used to enable/disable stream dialog normalization value.  0 is for Disable and 1 is for Enable
       Default is Enable
     */
     int32_t                i32StreamDialNormEnable;

    /*  This value indicates how far the average dialogue level is below digital 100 percent. Valid
        values are 1-31. The value of 0 is reserved and it means dialog normalization is turned off. The values of 1 to 31 are interpreted as -1 dB to -31dB
        with respect to digital 100 percent. If the reserved value of 0 is received, the decoder will not do any dialog normalization,
        Default value is 0
    */
    int32_t                 i32UserDialNormVal;

    /* These are user config parameters required from user  */
    BDSP_Raaga_Audio_DDPMs10UserOutput  sUserOutputCfg[2];
    uint32_t                ui32SubstreamIDToDecode; /*Default value =0 ; Range 0-3 for MS10*/

} BDSP_Raaga_Audio_DDPMs10ConfigParams;

/*
   This data structure defines DDRE user configuration parameters
*/

typedef struct  BDSP_Raaga_DDReencodeUserConfig
{

    uint32_t        ui32CompMode;   /*  Compression mode setting,Possible values: 1(RF mode), 0(Line mode),
    2 (RF Mode@-23dB EBU), 3 (RF Mode@-24dB ATSC).
    Default: 0(Line mode). Multichannel output can only configured for
    Line mode in the  current reference implementation*/

    uint32_t        ui32DrcCutFac;   /* DRC Cut Scale factor in 1.31 Format. Possble Values: 0 to 0x7FFFFFFF.
Default value: 0x7FFFFFFF */

    uint32_t        ui32DrcBoostFac; /* DRC Boost Scale factor in 1.31 Format. Possble Values: 0 to 0x7FFFFFFF.
Default value: 0x7FFFFFFF */

    uint32_t        ui32OutMode;    /*  Output channel configuration.Possible Values: 2(Stereo) or 7(3/2 Multichannel),
                                        Default = 2(Stereo?). None of the other possible ACMODS are allowed. */

    uint32_t        ui32OutLfe;     /*  Output LFE channel present .Possible Values: 0: LFE disabled, 1: LFE enabled,
                                        Default = 0(Disabled). Can only enabled with 3/2 Multichannel audio*/

    uint32_t        ui32StereoMode; /*  Flag indicating the Stereo downmixed mode, is only effective if Outputmode is Stereo.
                                        Possible Values: 0(LtRt), 1(LoRo) and 2(Arib). Default value: 0(LtRt) */

    uint32_t        ui32DualMode;   /*  Flag indicating the dualmono mode which is to be used if the incoming stream is dual mono.
                                        Possible values: 0(Stereo), 1(Left_Only) and 2(Right_Only). Default value: 0(Stereo). */


    uint32_t        ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS]; /*This is channel matrix of size 8 where each index can take any values from 0 to 7 depending upon  the channel routing done*/

}BDSP_Raaga_DDReencodeUserConfig;


typedef struct BDSP_Raaga_Audio_DDReencodeConfigParams
{
    /* Flag indicating that external Pcm mode is active. This flag is to be set active if input is not from the Dolby MS11 decoders(DDP and Dolby Pulse */
    uint32_t        ui32ExternalPcmEnabled;

    /* Compression profiles applicable. The Settings are only if the input is from a External PCM source.
        Possible values:
    0 : "No Compression"
    1 : "Film Standard Compression"
    2 : "Film Light Compression"
    3 : "Music Standard Compression"
    4 : "Music Light Compression"
    5 : "Speech Compression"
        Default Value: 1: "Film Standard Compression"*/
    uint32_t        ui32CompProfile;

    /*Index for center level in the downmixing case. Possible Values: 0-3. Default: 0. Used only if External PCM Mode is enabled or PCM Soundeffect Mixing is the only context enabled*/
    uint32_t        ui32CmixLev;

    /*Index for surround level in the downmixing case. Possible Values: 0-3. Default: 0. Used only if External PCM Mode is enabled or PCM Soundeffect Mixing is the only context enabled */
    uint32_t        ui32SurmixLev;

    /* Dolby Surround Encoding Mode.
    Possible Values:
        0: NOT INDICATED
        1: NOT SURROUND ENCODED
        2: SURROUND ENCODED
        Default: 0(NOT INDICATED).
    Used only if External PCM Mode is enabled or PCM Soundeffect Mixing is the only context enabled */
    uint32_t        ui32DsurMod;

    /* Dialog level of incoming PCM in dB. Possible values: 0-31. Default: 31 Used only if External PCM Mode is enabled or PCM Soundeffect Mixing is the only context enabled */
    uint32_t        ui32DialNorm;


    /* Total number of output ports enabled. If only stereo output port is set to enabled then the value should be 1, if Multichannel 5.1 PCM output is enabled the value should be 2. Only 1 output port with Multichannel 5.1 PCM output port is NOT allowed */
    uint32_t        ui32NumOutPorts;

    /* Per Output port configuration for DDRE */
    BDSP_Raaga_DDReencodeUserConfig        sUserOutputCfg[2];

}BDSP_Raaga_Audio_DDReencodeConfigParams;

/*
   This data structure defines DV258 user configuration parameters
*/
typedef struct BDSP_Raaga_Audio_DV258ConfigParams
{
    /*General Settings*/

    /*  Range of values -  0 or 1. As per the new guyidelines from Dolby, i32DolbyVolumeEnable =1(True) for both when DV is enabled and disabled.
        The controlling factor is now i32LvlEnable. Default value is 1 (True)*/
    int32_t     i32DolbyVolumeEnable;

    /*  Volume modeler on/off. Possible values are 0(off) and 1(on).
        Default 0(off). Always set to off for DV258
    */
    int32_t     i32VlmMdlEnable;

    /*  Flag to operate Dolby Volume in half mode. Flag is always set to off. Range [0,1]. Default ->0 (disable)*/
    int32_t         i32HalfmodeFlag;

    /*  Enable midside processing. Range [0,1]. Default ->1(enable).
        Flag is always set to enable for Dv258.*/
    int32_t     i32EnableMidsideProc;

    /*  Volume Leveler Settings */

    /*  Range of values -  0=FALSE , 1=TRUE.
        i32LvlEnable = 1 When Dolby Volume is Enabled.
        i32LvlEnable = 0 When Dolby Volume is Disabled.
        Default value : 1 (TRUE - volume leveler enabled)*/
    int32_t     i32LvlEnable;

    /*Range of values - [0 to 10]. Default value: 9 */
    int32_t     i32LvlAmount;


    /*  System Settings*/

    /*  Input reference level in dB. Range of values(in dB )- (0 to 130 in steps of 0.0625 dB). Possible values are 0 to 2080. Default value :1360(85 dB)*/
    int32_t     i32InputReferenceLevel;

    /*  The gain, if any, applied to the signal prior to entering DV258 in dB.
        Range of values(in dB )- (-30 to 30 in steps of 0.0625 dB). Possible values are -480 to 480. Default value : 0 (0 dB)*/
    int32_t     i32Pregain;

    /*  Output reference level in dB. Range of values(in dB )- (0 to 130 in steps of 0.0625 dB).
        Possible values are 0 to 2080. Default value :1360(85 dB)*/
    int32_t     i32OutputReferenceLevel;

    /*  Calibration offset in dB. Range of values(in dB)- (-30 to 30 in steps of 0.0625 dB). Possible values are -480 to 480. Default value    : 0( 0 dB)*/
    int32_t     i32CalibrationOffset;

    /*  Volume Modeler Settings*/

    /*  Volume level gain in dB -- applied after dolby volume.
        Range of values(in dB) - (-96 to 30 in steps of 0.0625 dB). Possible values are -1536 to 480.
        Default value    : 0 (0 dB)*/
    int32_t     i32AnalogVolumeLevel;

    /*  Volume level gain in dB -- applied by dolby volume.
        Range of values(in dB) - (-96 to 30 in steps of 0.0625 dB). Possible values are -1536 to 480.
        Default value    : 0 (0 dB)*/

    int32_t     i32DigitalVolumeLevel;

    /*User-input forced reset flag. Range [0,1], default 0*/
    int32_t     i32ResetNowFlag;

    /* Limiter Settings */

    /*  Enable Limter. When DV is enabled limiter_enable = TRUE.
        When DV is disabled limiter_enable = FALSE*/
    int32_t         i32LimiterEnable;

}BDSP_Raaga_Audio_DV258ConfigParams;



/*PCM Renderer User config*/

typedef struct  BDSP_FWIF_Pcmr_User_Params
{
    /*< Scale factor for incoming DRC cut value: 0 - 100,  default: 100*/
    uint32_t ui32DrcCut;

     /*< Scale factor for incoming DRC boost value: 0 - 100, default: 100 */
    uint32_t ui32DrcBoost;

    /*< Drc mode: 0 = Line, 1 = RF, 2 = RF_23dB, 3 = RF_24dB,  default: Line*/
    uint32_t ui32DrcMode;

    /*< Downmix mode: 0 = Lt/Rt, 1 = Lo/Ro, 2 = ARIB  default: Lt/Rt */
    uint32_t ui32DmxMode;

    /*< Dual Mode type if channel mode is dual_mono: 0 = Stereo, 1 = Left/first channel, 2 = Right/second channel,  default: Stereo*/
    uint32_t ui32DualMode;

    /*< Enable output default: Enabled, if the port is not enabled, please reset it to false*/
    uint32_t ui32OutputActive;
} BDSP_FWIF_Pcmr_User_Params;


typedef struct  BDSP_Raaga_Audio_DpcmrConfigParams
{
    /* default ui32Delay = 668; */
    /* This Parameter need not to expose at Nexus level since it is fixed for now */
    uint32_t     ui32Delay;
    /* default ui32NumMcChannels = 6; */
    /* This parameter need to set either 6 or 8 based on 5.1 or 7.1 output is tapped from PCMR */
    uint32_t     ui32NumMcChannels ;
    /* default ui32NumRawMcChannels = 6; */
    /* This parameter need to set either 6 or 8 based on 5.1 or 7.1 output is tapped from PCMR for DDP encoder*/
    uint32_t     ui32NumRawMcChannels;
    /* Number of meta data sets, default ui32NumMetadataSets = 1; */
    /* This Parameter need not to expose at Nexus level since it is fixed for now  */
    uint32_t     ui32NumMetadataSets;
    /* User parameters for the multichannel output, sMultiChannelOutputPort: PCMR supports max two multichannel output ports.
    All the elements of this structure need to be populated based on if that port is connected to PCMR */
    BDSP_FWIF_Pcmr_User_Params sMultiChannelOutputPort[2];
    /* User parameters for the downmixed output, sStereoDownmixedPort: PCMR supports stereo downmix output port for DAC.
    All the elements of this structure need to be populated based on if stereo port is enabled from nexus*/
    BDSP_FWIF_Pcmr_User_Params sStereoDownmixedPort;
    /*< Use compressor to generate new DRC meta data but preserve DMX meta data , default ui32DiscardDrc = 0; */
    /* This Parameter need not to expose at Nexus level since it is fixed for now */
    uint32_t   ui32DiscardDrc;
    /*< Compressor profile for generating DRC, default ui32CompressorProfile = 1; */
    /* This Parameter need not to expose at Nexus level since it is fixed for now */
    uint32_t     ui32CompressorProfile;
   /* encoded channel mode is locked to 5.1 */

}BDSP_Raaga_Audio_DpcmrConfigParams;
/*

   This data structure defines Dolby MS DDP decoder user configuration parameters
*/

typedef struct  BDSP_Raaga_Audio_DDPMultiStreamUserOutput
{
   /* p_user_cfg->i32CompMode = 2;*/
    /*Custom_0=0, Custom_1 = 1, Line = 2, RF =3 , DRC_OFF = 4, RF_23dB = 5, RF_24dB = 6 */
    int32_t                i32CompMode;

    /* p_user_cfg->i32PcmScale = 100;    The value is in percentage, 0 to 100. */
    int32_t                i32PcmScale;

   /* p_user_cfg->i3DynScaleHigh = 100;    The value is in percentage, 0 to 100. */
    int32_t                i32DynScaleHigh;

   /* p_user_cfg->i32DynScaleLow = 100;    The value is in percentage, 0 to 100. */
    int32_t                i32DynScaleLow;

    /* This is LFE ON/OFF flag and can take  two values 0 or 1
     default value for this field is 0
    */
    int32_t i32OutLfe;

    /*  enum { GBL_MODE11=0, GBL_MODE_RSVD=0, GBL_MODE10, GBL_MODE20,
        GBL_MODE30, GBL_MODE21, GBL_MODE31, GBL_MODE22, GBL_MODE32 };
        i32OutMode =7 default value;
    */
    int32_t i32OutMode;

    /*  preferred stereo mode
        enum { GBL_STEREOMODE_AUTO=0, GBL_STEREOMODE_SRND, GBL_STEREOMODE_STEREO };
        i32StereoMode = 0 is default value ;
    */
    int32_t i32StereoMode;

    /* dual mono downmix mode
       enum { GBL_DUAL_STEREO=0, GBL_DUAL_LEFTMONO, GBL_DUAL_RGHTMONO, GBL_DUAL_MIXMONO };
       i32DualMode = 0 is default value;
    */
    int32_t i32DualMode;

    /* karaoke capable mode
       enum { NO_VOCALS=0, GBL_VOCAL1, GBL_VOCAL2, GBL_BOTH_VOCALS };
       i32Kmode = 3;
    */
    int32_t i32Kmode;

    /* This i32ExtDnmixEnabled flag which can take  two values 0 or 1 based on disable/enable option
       default value for this i32ExtDnmixEnabled field is 0
    */
    int32_t i32ExtDnmixEnabled;
    int32_t i32ExtDnmixTab[DDP_DEC_GBL_MAXPCMCHANS][DDP_DEC_GBL_MAXPCMCHANS];

    /* This i32ExtKaraokeEnabled flag which can take  two values 0 or 1 based on disable/enable option
       default value for this i32ExtKaraokeEnabled field is 0
    */
    int32_t i32ExtKaraokeEnabled;
    int32_t i32Extv1Level;
    int32_t i32Extv1Pan;
    int32_t i32Extv2Level;
    int32_t i32Extv2Pan;
    int32_t i32ExtGmLevel;
    int32_t   i32ExtGmPan;
    /*This is channel matrix of size 8 where each index can take any values from 0 to 7
      Depending upon  the channel routing done
    */
    uint32_t   ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

} BDSP_Raaga_Audio_DDPMultiStreamUserOutput;


typedef enum BDSP_Raaga_Audio_eDDPUsageMode
{
    BDSP_Raaga_Audio_eSingleDecodeMode=0,
    BDSP_Raaga_Audio_eMS10DecodeMode,
    BDSP_Raaga_Audio_eMS11DecodeMode,
    BDSP_Raaga_Audio_eDDPUsageMode_eLAST,
    BDSP_Raaga_Audio_eDDPUsageMode_eINVALID=0x7fffffff

} BDSP_Raaga_Audio_eDDPUsageMode;

typedef struct BDSP_Raaga_UDC_UserOutput
{
    /* p_user_cfg->i32CompMode = 2;*/
    /*Custom_0=0, Custom_1=1, Line=2, RF=3 , PORTABLE_L8=4, PORTABLE_L11=5, PORTABLE_L14=6,PORTABLE_TEST=7,DRC_OFF=8,RF_23dB=9,RF_24dB=10 */
    int32_t         i32CompMode;
    /* p_user_cfg->i32PcmScale = 100. The value must be in range of 0 to 100, Default: 100 */
    int32_t         i32PcmScale;
    /* p_user_cfg->i3DynScaleHigh = 100. The value must be in range of 0 to 100, Default: 100*/
    int32_t         i32DynScaleHigh;
    /* p_user_cfg->i32DynScaleLow = 100. The value must be in range of 0 to 100, Default: 100*/
    int32_t         i32DynScaleLow;
    /* This is LFE ON/OFF flag and can take two values 0 or 1
    default value for this field is 1
    */
    int32_t     i32OutLfe;
    /* This is Decorrelation mode and can take two values only for now.
    NO_DECORRELATION, GUIDED_DECORRELATION, NONGUIDED_DECORRELATION default: NO_DECORRELATION
    */
    int32_t         i32decorr_mode;
    /* enum { GBL_MODE11=0, GBL_MODE_RSVD=0, GBL_MODE10, GBL_MODE20,
    GBL_MODE30, GBL_MODE21, GBL_MODE31, GBL_MODE22, GBL_MODE32 };
    i32OutMode =21 default value;
    */
    int32_t         i32OutMode;
    /* preferred stereo mode
    enum { GBL_STEREOMODE_AUTO=0, GBL_STEREOMODE_SRND, GBL_STEREOMODE_STEREO };
    i32StereoMode = 0 is default value ;
    */
    int32_t         i32StereoMode;
    /* dual mono downmix mode
    enum { GBL_DUAL_STEREO=0, GBL_DUAL_LEFTMONO, GBL_DUAL_RGHTMONO, GBL_DUAL_MIXMONO };
    i32DualMode = 0 is default value;
    */
    int32_t         i32DualMode;
    /* karaoke capable mode
    enum { NO_VOCALS=0, GBL_VOCAL1, GBL_VOCAL2, GBL_BOTH_VOCALS };
    i32Kmode = 3;
    */
    int32_t         i32Kmode;
    /* This i32MdctBandLimitEnable flag which can take two values 0 or 1 based on disable/enable option
    default value for this i32MdctBandLimitEnable field is 0*/

    int32_t         i32MdctBandLimitEnable;
    /* This i32ExtDnmixEnabled flag which can take two values 0 or 1 based on disable/enable option
    default value for this i32ExtDnmixEnabled field is 0
    */

    int32_t         i32ExtDnmixEnabled;
    int32_t         i32ExtDnmixTab[DDP_DEC_GBL_MAXPCMCHANS][DDP_DEC_GBL_MAXPCMCHANS];
    /* This i32ExtKaraokeEnabled flag which can take two values 0 or 1 based on disable/enable option
    default value for this i32ExtKaraokeEnabled field is 0
    */
    int32_t         i32ExtKaraokeEnabled;
    /*  default value for this i32Extv1Level  field is 0 */

    int32_t         i32Extv1Level;
    /*  default value for this i32Extv1Pan  field is 0 */
    int32_t         i32Extv1Pan;
    /*  default value for this i32Extv2Level  field is 0 */
    int32_t         i32Extv2Level;
    /*  default value for this i32Extv2Pan  field is 0 */
    int32_t         i32Extv2Pan;
    /*  default value for this i32ExtGmLevel  field is 0 */
    int32_t         i32ExtGmLevel;
    /*  default value for this i32ExtGmPan  field is 0 */
    int32_t         i32ExtGmPan;
    /*This is channel matrix of size 8 where each index can take any values from 0 to 7
    Depending upon the channel routing done
    */
    /* Default: 0 - BED_FADE_NO , 1 - FADE_IN, 2- FADE_OUT, 3- FADE_SILENCE*/
    int32_t         i32FadeOutputMode;
    uint32_t        ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
} BDSP_Raaga_UDC_UserOutput;

typedef struct
{
    /* Possible Values: eSingleDecodeMode, eMS12DecodeMode
       Default 0 : eSingleDecodeMode */
    BDSP_AF_P_DolbyMsUsageMode          eDolbyMSUsageMode;
     /* This enum lets the mixer know if the decoder type is primary or secondary or sound effects. */
    BDSP_AF_P_DecoderType               eDecoderType;
    /* This value "i32NumOutPorts" can be set uoto 3 based on output ports enabled 7.1, 5.1 and 2.0 */
    int32_t                             i32NumOutPorts;
    /* These are user config parameters required from user */
    BDSP_Raaga_UDC_UserOutput          sUserOutputCfg[3];
    /* This value is used to enable/disable stream dialog normalization value. 0 is for Disable and 1 is for Enable
    Default is Enable
    */
    int32_t                             i32StreamDialNormEnable;
    /* This value indicates how far the average dialogue level is below digital 100 percent. Valid
    values are 1-31. The value of 0 is reserved and it means dialog normalization is turned off. The values of 1 to 31 are interpreted as -1 dB to -31dB
    with respect to digital 100 percent. If the reserved value of 0 is received, the decoder will not do any dialog normalization,
    Default value is 0
    */
    int32_t                             i32UserDialNormVal;
    /*Default value =0 ; Range 0-3 can be selected
    */
    uint32_t                            ui32SubstreamIDToDecode;
    /* This is to enable metadata logging default is 0: This can take any number upto 65536. This is required only for certification
    */
    uint32_t                            ui32mdatflags;
    /* This is to enable evolution file logging default is 0: possible values are 0 or 1. This is required only for certification
    */
    uint32_t                            ui32EvolutionFileOut;
    /* This needs to enable for dynamic test cases, default is 0: possible values are 0 or 1. This is required only for CIDK certification
    */
    uint32_t                            ui32EnableDynamicUserParams;
    /* default value = 0 (disabled), set as 1 to enable it, if enabled, Decoder will start sending Atmos Metadata out.
    */
    uint32_t                            ui32EnableAtmosMetadata;
} BDSP_Raaga_Audio_UdcdecConfigParams;

#define AC4_DEC_ABBREV_PRESENTATION_LANGUAGE_LENGTH          (8>>2)
#define AC4_DEC_PROGRAM_IDENTIFIER_LENGTH                    (20>>2)
typedef struct BDSP_Raaga_AC4_UserOutput
{
    /* Channel configuration
         0 = Decoder outputs stereo or a LoRo downmix of multichannel content. (Default)
         1 = Decoder outputs stereo or a Dolby ProLogic compatible LtRt downmix of multichannel content.
         2 = Decoder outputs stereo or a Dolby ProLogic II compatible LtRt downmix of multichannel content.
         3 = Decoder outputs LoRo downmix of multichannel content only for dual language context.
         6 = Multichannel */
    uint32_t        ui32ChannelConfig;


    /* Indicates the preferred mixing ratio between main and associate streams
          Range = -32 to 32 (in 1dB steps)
          Default = -32
          Extreme limits: -32 -> Main Audio only; 32 -> Associated only
     */
    int32_t         i32MainAssocMixPref;


    /* Indicates if the output is requested to have a 90 degree phase shift in the surround channels
        0 = Disable 90 degree phase shift (Default)
        1 = Enable 90 degree phase shift */
    uint32_t        ui32Phase90Preference;


    /* Indicates gain that will be applied to dialog enhancement
        Range = -12 to 12 dB (in 1 dB steps)
        Default = 0
     */
    int32_t         i32DialogEnhGainInput;


    /* Target reference level for output signal
        Range = -31 to -7 dB (in 1 dB steps)
        Value of 0 dB = leveling is deactivated
        Default = -31 */
    int32_t         i32TargetRefLevel;


    /* Indicates if DRC shall be performed
         0 = Disable DRC
         1 = Enable DRC (Default)  */
    uint32_t        ui32DrcEnable;

    /* DRC Presets
        0=none (Default), 1=Line@-31dB, 2=RF@-20dB, 3=RF@-23dB, 4=RF@-24dB*/
    uint32_t    ui32DrcMode;

    /*** Audio post processing parameters ***/
    /* Strength factor for intelligent EQ
        Min. value = 0 (intelligent EQ disabled)
        Max. value = 256 (full strength)
        Default : 256 */
     uint32_t        ui32IeqStrength;


    /* Profile for intelligent EQ
         0 = Disabled (Default)
         1 = Open mode
         2 = Rich mode
         3 = Focused mode */
    uint32_t        ui32IeqProfile;

    uint32_t        ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
    /*********************************************************/
    /* Dual Language Personalization Features of AC4 Decoder */
    /* Below variables needs to be programmed in case dual
          language context is enabled for AC4 */
    /*********************************************************/
    /* Presentation number to be decoded
               0 to 511: presentation index
               Default: 0xFFFFFFFF (INVALID)
               A presentation is selected to be decoded based on one of the personalization user configs available, if none of the
               selected personalizations match with what is available in a stream, then the first presentation, of presentation index 0 is decoded
               Personalization options for a AC4 decode
                 - By presentation index
                 - By Language tags
                 - By Associate Type */
    uint32_t        ui32PresentationNumber;

    /* Preferred language 1 of the presentation
               Big Endian Population
               Initialized to /'0'/ by default */
    uint32_t        ui32PreferredLanguage1[AC4_DEC_ABBREV_PRESENTATION_LANGUAGE_LENGTH];

    /* Preferred language 2 of the presentation
               Big Endian Population
               Initialized to /'0'/ by default */
    uint32_t        ui32PreferredLanguage2[AC4_DEC_ABBREV_PRESENTATION_LANGUAGE_LENGTH];

    /*  Preferred Associate type of the presentation
           Identifies the type of associated audio service to decode for Dolby AC-4 inputs
               1 = Visually impaired (Default)
               2 = Hearing impaired
               3 = Commentary */
    uint32_t        ui32PreferredAssociateType;

    /* Indicates the type of ID to be expected in the program_identifier field of the Dolby AC-4 bitstream.
           0 = NONE (Default)
           1 = PROGRAM_ID_TYPE_SHORT
           2 = PROGRAM_ID_TYPE_UUID */
    uint32_t        ui32PreferredIdentifierType;

    /* Identifies the Dolby AC-4 stream that the primary presentation index relates to based on the opted for Preferred Identifier type
          The Personalized-program universal unique ID is populated in the array in big endian fashion
          Initialized to /'0'/ by default */
    int32_t         i32PreferredProgramID[AC4_DEC_PROGRAM_IDENTIFIER_LENGTH];

    /* Prioritize associated audio service type matching
       Indicates whether matching of the preferred associated audio service type takes priority over matching of the
       preferred language type when selecting a presentation for decoding
           0 = No
           1 = Yes (default) */
    uint32_t        ui32PreferAssociateTypeOverLanguage;
} BDSP_Raaga_AC4_UserOutput;

#define BDSP_RAAGA_AC4_NUM_OUTPUTPORTS      3
typedef struct BDSP_Raaga_Audio_AC4DecConfigParams
{
    /* Possible Values: eSingleDecodeMode, eMS12DecodeMode
       Default 0 : eSingleDecodeMode */
    BDSP_AF_P_DolbyMsUsageMode          eDolbyMSUsageMode;

    /* This enum lets the mixer know if the decoder type is primary or secondary or sound effects.
       Default 0 */
    BDSP_AF_P_DecoderType               eDecoderType;

    /* This value "i32NumOutPorts" can be set upto 3 based on output ports enabled 7.1 5.1 and 2.0.
       Default 1 */
    int32_t                             i32NumOutPorts;

    /* Per output port configs
       CIDK Defaults
       {0,-32,0,0,-31,1,0,256,0,{0,1,2,3,4,5,0xFFFFFFFF,0xFFFFFFFF}}, Port Config 1
       {0,-32,0,0,-31,1,0,256,0,{0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}}, Port Config 2

       IDK Defaults
       {6,0,0,0,-31,0,0,0,0,{0,1,2,3,4,5,6,7}}, Port Config 1
       {2,0,0,0,-31,0,0,0,0,{0,1,2,3,4,5,6,7}}  POrt Config 2

    */
    BDSP_Raaga_AC4_UserOutput           sUserOutputCfg[BDSP_RAAGA_AC4_NUM_OUTPUTPORTS];

    /* AC-4 Decoding complexity level
         3 = Level 3 (Default)     */
    uint32_t        ui32InputCplxLevel;


    /* AC-4 Decoding main/associate audio selection
         0 = decode main & associate audio (Default)
         1 = decode main audio only
         2 = decode associate audio only */
    uint32_t        ui32MainAssocDec;


    /* AC-4 Decoder output complexity level
         0 = Stereo
         1 = 5.1 Multichannel (Default) */
    uint32_t        ui32OutputCplxLevel;


    /* Flag that indicates that this instance is the only one to be opened
         0 = multiple instances of the output stage can/will be opened
         1 = single instance of the output stage can/will be opened (Default) */
    uint32_t        ui32SingleInstance;


    /* AC-4 Decoder output sampling rate
         0 = Decoder resamples everything to 48 kHz output sampling rate (IDK DEFAULT)
         1 = Decoder output sampling rate equals input sampling rate up to the max output sampling rate supported, e.g. 48 kHz. (CIDK Default) */
    uint32_t        ui32SamplingRateMode;


    /* Flag that indicates if DAP shall be performed
         0 = Disable DAP (Default)
         1 = Enable DAP  */
    uint32_t        ui32DapEnable;

    /* Enable limiter : Turn off/on limiter for testing
            Default : 1 (Enable)*/
    uint32_t        ui32LimiterEnable;

    /* Dolby certification mode
         0 = Disabled (CIDK Default)
         1 = Enabled (IDK Default) */
    uint32_t        ui32CertificationMode;

    /* Time scale in ticks per second
       Required only for Component level cert testing
        CIDK Default = 90000
         IDK Default = AC4DEC_FRAMER_DEFAULT_TIME_SCALE */
    uint32_t        ui32TimeScale;


    /* Choose a AC4 Decoding Mode
        0 = Single Stream, Single Decode (Default)
        1 = Single Stream, Dual Decode, Single Instance
        2 = Single Stream, Dual Decode, Dual Instance
        3 = Dual Stream, Dual Decode */
    uint32_t        ui32AC4DecodeMode;


    /* Associate mixing in an instance of AC4 Decoding
        0 = Disable
        1 = Enable (Default)*/
    uint32_t        ui32EnableADMixing;

    /* Stream Info Presentation number - index of presentation for which presentation info needs to be sent
       Default = -1
    */
    int32_t i32StreamInfoPresentationNumber;

} BDSP_Raaga_Audio_AC4DecConfigParams;

/*
   These data structures define Dolby AACHE Decoder user configuration parameters
*/
typedef struct BDSP_Raaga_Audio_DolbyAachePortConfig
{

    /*
        For Multichannel (5.1) Output Port:
            for(i=0; i<6; i++)
                ui32OutputChannelMatrix[i] = i;

            for(i=6; i<BDSP_AF_P_MAX_CHANNELS; i++)
                    ui32OutputChannelMatrix[i] = 0xFFFFFFFF; This represents INVALID


        For Stereo Output Port:
            for(i=0; i<2; i++)
                ui32OutputChannelMatrix[i] = i;

            for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
                ui32OutputChannelMatrix[i] = 0xFFFFFFFF; This represents INVALID

    */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

} BDSP_Raaga_Audio_DolbyAachePortConfig;


typedef struct BDSP_Raaga_Audio_DolbyAacheUserConfig
{
    /* This enum lets the mixer know if the decoder type is primary or secondary or sound effects. */
    BDSP_AF_P_DecoderType   eDecoderType; /* Default : BDSP_AF_P_DecoderType_ePrimary */

    BDSP_AF_P_DolbyMsUsageMode      eDolbyAacheUsageMode; /* Default : BDSP_AF_P_DolbyMsUsageMode_eMS12DecodeMode */

    uint32_t    ui32MapTo51Channels;    /* Default = 1 -> 5.1 output, 0 -> channel config as seen in the stream */

    /* PRL and DRC configs are not port specific configurations, and will affect all output ports */
    uint32_t    ui32RefDialnormLevel;   /* Default = 124, corresponding to -31dB LKFS (31*4) */

    uint32_t    ui32DefDialnormLevel;   /* Default = 108, corresponding to -27dB LKFS (27*4); Maximum = 127 */

    /* Mode: Application of DRC/compr and consideration of Program Reference Level on the PCM output of the decoder.
       0=none, 1=DRC/compr, 2=PRL, 3=DRC/compr and PRL, */
    uint32_t    ui32DrcMode;                    /* Default = 2 */

    /*Type: 0=none, 1=MPEG-DRC, 2=DVB-compr, 3=prefer DRC, 4=prefer compr, 5=Portable mode */
    uint32_t    ui32DrcType;                    /* Default = 0 */

    /* Downsample sample rates above 48kHz by a factor of 2
         - 0: force downsampling off
         - 1: force downsampling on */
    uint32_t    ui32Fast2to1Resample;           /* Default = 0 */

    /* Upsample QSF and LSF rates to primary sample rates 32000, 44100, 48000
            - 0: force Upsampling off
            - 1: force Upsampling on */
    uint32_t    ui32UpsampleToPrimaryRates;     /* Default = 0 */


    /* Enable / Disable AAC error concealment */
    uint32_t    ui32EnableErrorConcealment;     /* Default = 0 (Disable) */

    /* Select stereo downmix type
        0 = LoRo Downmix (Default)
        1 = LtRt Downmix
        2 = ARIB Downmix
        3 = Use MPEG AAC PCE bitstream information */
    uint32_t    ui32EnableStereoDownmixType;


    /* Component level certification flag
        0 = Disable (Default)
        1 = Enable */
    uint32_t    ui32IDKCertificationFlag;

    /* Loudness Equivalence Mode
          0 = No Loudness Equivalence (Default)
          1 = EBU (-23dB)
          2 = ATSC (-24dB)
          3 = EBU_ALL_PORTS (-23dB)
          4 = ATSC_ALL_PORTS (-24dB)

          EBU Settings:
          ui32RefDialnormLevel = 31*4 (-31dB)
          ui32DefDialnormLevel = 23*4 (-23dB)
          ui32LoudnessEquivalenceMode = 1
              -Multichannel is at -31dB
              -Stereo is at -23dB

          ATSC Settings:
          ui32RefDialnormLevel = 31*4 (-31dB)
          ui32DefDialnormLevel = 24*4 (-24dB)
          ui32LoudnessEquivalenceMode = 2
              -Multichannel is at -31dB
              -Stereo is at -24dB

          EBU Settings All Ports:
          ui32RefDialnormLevel = 31*4 (-31dB)
          ui32DefDialnormLevel = 23*4 (-23dB)
          ui32LoudnessEquivalenceMode = 3
              -Multichannel is at -23dB
              -Stereo is at -23dB

          ATSC Settings All Ports:
          ui32RefDialnormLevel = 31*4 (-31dB)
          ui32DefDialnormLevel = 24*4 (-24dB)
          ui32LoudnessEquivalenceMode = 4
              -Multichannel is at -24dB
              -Stereo is at -24dB */

    uint32_t    ui32LoudnessEquivalenceMode;

    /* Port specific configs : 0 -> Multichannel; 1 -> Stereo */
    BDSP_Raaga_Audio_DolbyAachePortConfig   sOutPortCfg[2];

} BDSP_Raaga_Audio_DolbyAacheUserConfig;


typedef struct  BDSP_Raaga_Audio_DDPMultiStreamConfigParams
{
    /* Possible Values: 0(Single Decode), 1(MS10 Mode) and 2(MS11 Mode) */
    BDSP_Raaga_Audio_eDDPUsageMode  eDdpUsageMode;

    /* This enum lets the mixer know if the decoder type is primary or secondary or sound effects. */
    BDSP_AF_P_DecoderType   eDecoderType;

    uint32_t                ui32SubstreamIDToDecode; /*Default value =0 ; Range 0-3 for MS10*/

    /*  This value "i32NumOutPorts" can be set to 1 or 2 based on output ports */
    int32_t                i32NumOutPorts;

    /* This value is used to enable/disable stream dialog normalization value.  0 is for Disable and 1 is for Enable
       Default is Enable
     */
     int32_t                i32StreamDialNormEnable;

    /*  This value indicates how far the average dialogue level is below digital 100 percent. Valid
        values are 1-31. The value of 0 is reserved and it means dialog normalization is turned off. The values of 1 to 31 are interpreted as -1 dB to -31dB
        with respect to digital 100 percent. If the reserved value of 0 is received, the decoder will not do any dialog normalization,
        Default value is 0
    */
    int32_t                 i32UserDialNormVal;

    /* This value indicates if Meta data log needs to be enabled. It should be enabled only for certification.
       Default value is Disable.
     */
    BDSP_AF_P_EnableDisable eMetaDataLogsEnable;

    /* These are user config parameters required from user  */
    BDSP_Raaga_Audio_DDPMultiStreamUserOutput  sUserOutputCfg[2];

} BDSP_Raaga_Audio_DDPMultiStreamConfigParams;

typedef struct BDSP_Raaga_Audio_BiquadCoefs
{
  int32_t i32A0Coeff; /* Q30 Format. Default: 0x40000000. */
  int32_t i32A1Coeff; /* Q30 Format. Default: 0x00000000. */
  int32_t i32A2Coeff; /* Q30 Format. Default: 0x00000000. */
  int32_t i32B1Coeff; /* Q30 Format. Default: 0x00000000. */
  int32_t i32B2Coeff; /* Q30 Format. Default: 0x00000000. */
} BDSP_Raaga_Audio_BiquadCoefs;

/*
   -------------------------------------------------------------------------
   This data structure defines SRS StudioSound user configuration parameters
   -------------------------------------------------------------------------
*/

/*
    Common Substucture for General node configuration and top level information for all 4 nodes of SRS STUDIO
    ---------------------------------------------------------------------------------------------------------
*/
/*
    i32IsStudioSound, i32StudioSoundMode and i32mEnable have to be used by all 4 nodes
    i32mInputGain, i32mHeadroomGain will be used by First node(Node0), currently SRS TruVolume
    i32mInputMode will be used by First node(Node0) currently SRS TruVolume and Node 1(Circle Surround Decoder/Tru-Dialog)
    i32mOutputGain and i32mBypassGain will be used by Last node(Node 3), curretly Equalizer and Hard limiter
*/
typedef struct BDSP_Raaga_Audio_StudioSoundTopLevelUserConfig
{
    int32_t                         i32IsStudioSound;   /*To identify whether the postprocess is part of StudioSound */
                                                        /*or standalone(TVOL and/or TSHD) configuration*/
                                                        /*This field can change only at stop start*/
                                                        /*Default : 0, Range [0, 1], others invalid, will be treated as 0*/
                                                        /* 0 -> Stand Alone configuration */
                                                        /* 1 -> Part of StudioSound */

    int32_t                         i32StudioSoundMode; /* if i32IsStudioSound == 1, this field and  will signify the mode to run. Following is the description. */
                                                        /* if i32IsStudioSound == 0, This field and all other field in  BDSP_Raaga_Audio_StudioSoundTopLevelUserConfig will be ignored */

                                                        /*Default : 1, Range [1, 2], others invalid, will be treated as 1*/


/*
    In STAND ALONE MODE TVOL and TSHD :
    -----------------------------------
        (i32IsStudioSound = 0), all other fields in  BDSP_Raaga_Audio_StudioSoundTopLevelUserConfig will be ignored by the FW node.
*/
/*  In STUDIO SURROUND MODE
        (i32IsStudioSound = 1), Based on mode (i32StudioSoundMode), a few of the modules will be disabled by the FW internally.
                                In case of conflict, i32StudioSoundMode will have  Highest priority.

        Different Studio Sound Modes are given Below:

             i32StudioSoundMode = (1) -> GEq-Off. User configures all other modules
             i32StudioSoundMode = (2) -> TVOL Off, CSDecoder Off, TruDialog Off, TSHD Off. User configures all other modules

  User cannot switch ON any module that is specified as OFF for a particular mode. For example, while running in StudioSound mode with i32StudioSoundMode = 1, GEQ cannot be switched ON by the user.
  User can chose to turn ON or OFF rest of the modules.

  Also, CSDecoder can only be switched ON when TSHD is ON, though TSHD can be switched ON with or without CS Decoder.
 */

    int32_t                         i32mEnable;         /*SRS StudioSound processing enable*/
                                                        /*Range [0, 1], Default : 1*/
                                                        /*  1:Enable SRS StudioSound processing,
                                                            0:Bypass SRS StudioSound,
                                                            All modules will be disabled with bypassgain 1.0
                                                        */

    int32_t                         i32mInputGain;      /*SRS StudioSound Input Gain*/
                                                        /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                        /*Fixed point: (0x0 - 0x7fffffff), default 0x7fffffff, QFormat - 1.31    */

    int32_t                         i32mHeadroomGain;   /*SRS StudioSound Headroom Gain*/
                                                        /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                        /*Fixed point: (0x0 - 0x7fffffff), default 0x7fffffff, QFormat - 1.31*/

    int32_t                         i32mInputMode;      /*Channel configuration present in input*/
                                                        /*Range: [0, 1, 2, 3], Default: 3*/
                                                        /* 0 : 1_0_0
                                                            1 : 2_0_0
                                                            2 : 3_2_1
                                                            3 : LtRt
                                                        */

    int32_t                         i32mOutputGain;     /* SRS StudioSound Output Gain */
                                                        /* Range: Floating point: (0.0 - 1.0), Default: 1.0 */
                                                        /* Fixed point: (0x0 - 0x7fffffff), default 0x7fffffff, QFormat - 1.31*/

    int32_t                         i32mBypassGain;     /*SRS StudioSound Bypass Gain*/
                                                        /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                        /*Fixed point: (0x0 - 0x7fffffff), default 0x7fffffff, QFormat - 1.31*/


}BDSP_Raaga_Audio_StudioSoundTopLevelUserConfig;


/*--------------------------*/
/*HighPass Filter UserConfig*/
/*--------------------------*/
typedef struct BDSP_Raaga_FilterSpecHpf
{
    uint32_t  ui32CutoffFrequency;      /*Range: 20 to 1000 hz, default: 180 Hz*/
    uint32_t  ui32Order;                /*Range: [0, 2, 4, 6], default : 4*/

}BDSP_Raaga_FilterSpecHpf;

/*Coefficients should be generated by SRS_Apollo_TruEQ_HPF_Coefficients_Generator.exe*/
typedef struct BDSP_Raaga_FilterCoefHpfTdf2
{
    int32_t             i32Scale;               /*specified q-format of the coefficients. (1+i32Scale).(31-i32Scale) format
                                                Default - 1. i32Scale=1 implies a q-format of 2.30*/
    int32_t             i32FilterCoefficientB0;
    int32_t             i32FilterCoefficientB1;
    int32_t             i32FilterCoefficientB2;

    int32_t             i32FilterCoefficientA1;
    int32_t             i32FilterCoefficientA2;

}BDSP_Raaga_FilterCoefHpfTdf2;

typedef struct BDSP_Raaga_FilterCoefHpf
{
    uint32_t            ui32Order;/*HighPass Filter order. Range: [2, 4, 6] */

    BDSP_Raaga_FilterCoefHpfTdf2    sFilterCoefHpfTdf2[3];  /*number of active biquads filter decided by i32Order*/

}BDSP_Raaga_FilterCoefHpf;


typedef struct BDSP_Raaga_Audio_HighPassFilterUserConfig
{
    uint32_t     ui32mEnable;        /*Highpass filter enable, Range: [0, 1], Default - 1 */
                                     /*0 - Disable HPF Processing, 1 - Enable HPF Processing*/

    uint32_t     ui32CoefGenMode;     /*Range - [0, 1], Default -> 0
                                        0 - > use the coefficients provided by user, in field sFilterCoefHpf[]
                                        1 - > use filter setting provided by user in BDSP_Raaga_FilterSpecHpf[] to generate coefficients,
                                        will overwrite the coefficients in sFilterCoefHpf[][]*/

    BDSP_Raaga_FilterCoefHpf    sFilterCoefHpf[3];          /* Array size 3, representing supported sampling rates 32, 44.1 and 48kHz, will be used if i32CoefGenMode==0*/
    BDSP_Raaga_FilterSpecHpf    sFilterSpecHpf;         /* Filter specification for generating filter coeffcients, will be used if i32CoefGenMode==1 */


}BDSP_Raaga_Audio_HighPassFilterUserConfig;

/*Default values of HighPass filter coefficients*/

/* i32mEnable=1*/

/*Sampling frequency - 32 kHz*/
/*---------------------------*/
/*
sFilterCoefHpf[0].i32Order = 0x00000004
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32Scale = 0x00000001
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientB0 = 0x0fc81f80
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientB1 = 0xe06fc100
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientB2 = 0x0fc81f80
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientA1 = 0x7e36e680
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientA2 = 0xc1b4eec0

sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32Scale = 0x00000001
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientB0 = 0x3df563c0
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientB1 = 0x84153880
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientB2 = 0x3df563c0
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientA1 = 0x7be0e200
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientA2 = 0xc40b5300
*/
/*Sampling frequency - 44.1 kHz*/
/*-----------------------------*/
/*
sFilterCoefHpf[1].i32Order = 0x00000004
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32Scale = 0x00000001
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientB0 = 0x0fd78db0
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientB1 = 0xe050e4a0
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientB2 = 0x0fd78db0
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientA1 = 0x7eb71980
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientA2 = 0xc13e3e40

sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32Scale = 0x00000001
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientB0 = 0x3e826c40
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientB1 = 0x82fb2780
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientB2 = 0x3e826c40
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientA1 = 0x7cff9680
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientA2 = 0xc2f5e600
*/

/*Sampling frequency - 48 kHz*/
/*---------------------------*/
/*
sFilterCoefHpf[2].i32Order = 0x00000004
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32Scale = 0x00000001
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientB0 = 0x0fdadc10
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientB1 = 0xe04a47e0
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientB2 = 0x0fdadc10
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientA1 = 0x7ed26000
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientA2 = 0xc1249f40

sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32Scale = 0x00000001
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientB0 = 0x3ea0f4c0
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientB1 = 0x82be1680
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientB2 = 0x3ea0f4c0
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientA1 = 0x7d3d7780
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientA2 = 0xc2b9a440
*/

/*3rd biquad need not be initialized as filter order is 4*/

/*Default HighPass filter specification -
    i32CutoffFrequency=180 Hz;
    i32Order=4*/


/*
    This data structure defines TruVolume configuration parameters.
    ---------------------------------------------------------------

    TVOL can be a standalone Post processing module or it can be one of the nodes in SRS studio
    The selection of this mode is determined by the i32IsStudioSound field.
*/

typedef struct BDSP_Raaga_Audio_TruVolumeUserConfig
{
    int32_t                     i32TruVolume_enable;        /* Top level disable, no processing will happen at all when 0*/
                                                            /* Deafault : 1, range [0,1]*/
                                                            /* 1 -> processing enable, 0-> total disable of the processing*/

    int32_t                     i32nchans;                  /* Number of input/output channels     */
                                                            /* Default val 2*/

    int32_t                     i32blockSize;               /* Default: 256, range [256, 512, 768, 1024]*/

    int32_t                     i32mEnable;                 /* Default: 1;  1 -> Process, 0 ->  Bypass */

    /* This value needs to be programmed in  Q21 format */
    int32_t                     i32mInputGain;              /* Default: 0x00200000 range[0, 0x03ffffff]   Floating val: default 1: range [0, 32] */

    /* This value needs to be programmed in  Q21 format */
    int32_t                     i32mOutputGain;             /* Default: 0x00200000  range[0, 0x007fffff]   Floating val: default 1: range [0, 4] */

    int32_t                     i32mBypassGain;             /* Bypass gain */
                                                            /* default: 0x007fffff range[0, 0x007fffff]
                                                               Floating val: default 1: range [0, 1] */

    int32_t                     i32mReferenceLevel;         /* Reference Level */
                                                            /* Default: 0x00400000 range[0x00000109, 0x007fffff]
                                                               Floating val: default 0.5: range [3.1623 * 10^-5, 1] */

    int32_t                     i32EnableDCNotchFilter;     /* Enable DC Notch filter*/
                                                            /* Default: 0 range[0 -> disble, 1 -> enable]*/

    int32_t                     i32mMode;                   /* Default: 1 range[0,1] 1-> kSrsLight, 0-> kSrsNormal*/

    int32_t                     i32mSize;                   /* Default: 0 range[0,1,2,3,4, 5]*/
                                                            /*
                                                                0   ->  VIQ_kSrs20Hz,
                                                                1   ->  kSrs40Hz,
                                                                2   ->  kSrs110Hz,
                                                                3   ->  kSrs200Hz,
                                                                4   ->  kSrs315Hz,
                                                                5   ->  kSrs410Hz*/

    int32_t                     i32mMaxGain;                 /* Max Gain Control */
                                                             /* Default: 0x00080000 rnage[0x00002000, 0x007fffff]*/
                                                             /* Float default: 16.0 rnage[.25, 256.0], conversion formula: (f_val/256)*2^23 */


    int32_t                     i32mNoiseManager;           /* Default: 1; 1 -> Noise manager enable, 0 -> disable */
    int32_t                     i32mNoiseManagerThreshold;  /* Default: 0x000ccccd, range [0x0000a3d7, 0x007fffff] */
                                                            /* Float Default: 0.1 range[0.005, 1.0], conversion formula: f_val*2^23 */

    int32_t                     i32mCalibrate;              /* Default: 0x8000, range [0x00000148, 0x007fffff] */
                                                            /* Float Default: 1.0 range[0.01, 256.0]*/

    int32_t                     i32mNormalizerEnable;       /* Default: 1;  1 -> Normalizer Enable, 0   ->  Normalizer Disable */


    BDSP_Raaga_Audio_StudioSoundTopLevelUserConfig   sTopLevelConfig;
    BDSP_Raaga_Audio_HighPassFilterUserConfig        sHighPassFilterConfig;

}BDSP_Raaga_Audio_TruVolumeUserConfig;


/*
    Node1 of SRS STUDIO: This is a combination of SRS-Tru-Dialog and SRS Circle Surround.
    -------------------------------------------------------------------------------------

    The user configuration of Node1 is BDSP_Raaga_Audio_SRS_CSDecTruDialogConfigParams

    User CFG structure for Circle Surround Decoder/Tru-Dialog are separately defined as below
*/

/*TruDialog user config*/
typedef struct BDSP_Raaga_Audio_TruDialogUserConfig
{
    int32_t         i32mEnable;                     /*Enable TruDialog, Range [0, 1], Default : 1*/
                                                    /*1:Enable TruDialog, 0:Bypass TruDialog*/


    int32_t         i32mTruDialogProcessGain;       /*Amount of final calculated gain applied to final  output signal*/
                                                    /*Range: Floating point: (0.5 - 3.0), Default: 1.5*/
                                                    /*Fixed point: (0x04000000 - 0x18000000), default: 0x0c000000,  QFormat - 5.27*/


    int32_t         i32mTruDialogInputGain;         /*TruDialog Input Gain*/
                                                    /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                    /*Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t         i32mTruDialogOutputGain;        /*TruDialog Output Gain*/
                                                    /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                    /*Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t         i32mTruDialogDialogClarityGain; /*Amount of Dialog Clarity enhancement applied to input signal*/
                                                    /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                    /*Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t         i32mTruDialogBypassGain;        /*TruDialog Bypass Gain*/
                                                    /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                    /*Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/
}BDSP_Raaga_Audio_TruDialogUserConfig;

/*CSDecoder user config*/
typedef struct BDSP_Raaga_Audio_CSDecoderUserConfig
{

    int32_t         i32mEnable;                     /*Enable CS Decoding*/
                                                    /*Range [0, 1], Default : 1*/
                                                    /*1:Enable CS Decoding, 0:Bypass CS Decoding*/

    int32_t         i32mInputGain;                  /*CS Decoder Input Gain */
                                                    /*Range: Floating point: (0.12589 - 1.0), Default: 1.0*/
                                                    /*In dB       : (-18dB - 0dB) , Default: 0 dB*/
                                                    /*Fixed point : (0x1020c49c - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t         i32mMode;                       /*CS Decoder processing mode*/
                                                    /*Range [1, 4], Default : 1, [1:Cinema 4:Music]*/

    int32_t         i32mOutputMode;                 /*Output mode Range: [0, 1]. Only supported mode is 0 Default : 0*/
                                                    /*0 - Stereo output mode, 1 - Multi channel output mode*/

    int32_t         i32mCSDecOutputGainLR;          /*CS Decoder output gain for LR channels*/
                                                    /*Range: Floating point (0.089 to 4.0), Default: 1.0*/
                                                    /*Range: Fixed point (0x016c8b44 to 0x40000000), default: 0x10000000, QFormat: 4.28*/
                                                    /*Range: in decibels (-21dB to 12dB), default: 0dB*/

    int32_t         i32mCSDecOutputGainLsRs;        /*CS Decoder output gain for LsRs channels*/
                                                    /*Range: Floating point (0.089 to 4.0), Default: 1.0*/
                                                    /*Range: Fixed point (0x016c8b44 to 0x40000000), default: 0x10000000, QFormat: 4.28*/
                                                    /*Range: in decibels (-21dB to 12dB), default: 0dB*/

    int32_t         i32mCSDecOutputGainC;           /*CS Decoder output gain for Center channel*/
                                                    /*Range: Floating point (0.089 to 4.0), Default: 1.0*/
                                                    /*Range: Fixed point (0x016c8b44 to 0x40000000), default: 0x10000000, QFormat: 4.28*/
                                                    /*Range: in decibels (-21dB to 12dB), default: 0dB*/

    int32_t         i32mCSDecOutputGainSub;         /*CS Decoder output gain for Subwoofer channel*/
                                                    /*Range: Floating point (0.089 to 4.0), Default: 1.0*/
                                                    /*Range: Fixed point (0x016c8b44 to 0x40000000), default: 0x10000000, QFormat: 4.28*/
                                                    /*Range: in decibels (-21dB to 12dB), default: 0dB*/
}BDSP_Raaga_Audio_CSDecoderUserConfig;

/* Top level User Cfg structure for Node1 of SRS studio*/

typedef struct BDSP_Raaga_Audio_SRS_CSDecTruDialogConfigParams
{
    BDSP_Raaga_Audio_StudioSoundTopLevelUserConfig   sTopLevelConfig;
    BDSP_Raaga_Audio_TruDialogUserConfig             sTruDialogConfig;
    BDSP_Raaga_Audio_CSDecoderUserConfig             sCSDecoderConfig;

}BDSP_Raaga_Audio_SRS_CSDecTruDialogConfigParams;


/*
    Node2 of SRS STUDIO: SRS TS-HD.
    -------------------------------------------------------------------------------------

    The user configuration of Node2 is BDSP_Raaga_Audio_TruSurrndHDConfigParams

    SRS TS-HD can be a standalone Post processing module or it can be one of the nodes in SRS studio
    The selection of this mode is determined by the i32IsStudioSound field.
*/

typedef enum BDSP_Raaga_Audio_SRSTruBassProcessMode
{
    BDSP_Raaga_Audio_SRSTruBassProcessMode_eMono,
    BDSP_Raaga_Audio_SRSTruBassProcessMode_eStereo,
    BDSP_Raaga_Audio_SRSTruBassProcessMode_eLast,
    BDSP_Raaga_Audio_SRSTruBassProcessMode_eInvalid = 0x7fffffff

} BDSP_Raaga_Audio_SRSTruBassProcessMode;

typedef enum BDSP_Raaga_Audio_SRS3DMode
{
    BDSP_Raaga_Audio_SRS3DMode_eMono,
    BDSP_Raaga_Audio_SRS3DMode_eSingleSpeaker,
    BDSP_Raaga_Audio_SRS3DMode_eStereo,
    BDSP_Raaga_Audio_SRS3DMode_eExtreme,
    BDSP_Raaga_Audio_SRS3DMode_eLast,
    BDSP_Raaga_Audio_SRS3DMode_eInvalid = 0x7fffffff

} BDSP_Raaga_Audio_SRS3DMode;

typedef enum
{
    BDSP_Raaga_Audio_SRSTSHDCrossoverFrequency_e80Hz = 0,
    BDSP_Raaga_Audio_SRSTSHDCrossoverFrequency_e120Hz,
    BDSP_Raaga_Audio_SRSTSHDCrossoverFrequency_e160Hz,
    BDSP_Raaga_Audio_SRSTSHDCrossoverFrequency_e200Hz,
    BDSP_Raaga_Audio_SRSTSHDCrossoverFrequency_eMax,
    BDSP_Raaga_Audio_SRSTSHDCrossoverFrequency_eInvalid =0x7fffffff

} BDSP_Raaga_Audio_SRSTSHDCrossoverFrequency;

typedef struct BDSP_Raaga_Audio_TruSurrndHDConfigParams
{
    int32_t     i32TruSurrndHDEnableFlag;   /*Default 1 Enable*/
    int32_t     i32HeadPhoneEnableFlag;     /*Default 0 disable*/
    int32_t     i32TruSurrndHDInputGain;    /*Range 0 to 1.0 in Q31 format
                                            Default 0x40000000*/
    int32_t     i32TruSurrndHDOutputGain;   /*Range 0 to 1.0 in Q31 format
                                            Default 0x7fffffff*/
    int32_t     i32TruSurrndHDByPassGain;   /*Range 0 to 1.0 in Q31 format
                                            Default 0x7FFFFFFF*/
    BDSP_Raaga_Audio_SRSTSHDCrossoverFrequency eSubCrossOverFreq; /*Used only for non Apollo mode
                                            Default BDSP_Raaga_Audio_SRSTSHDCrossoverFrequency_e200Hz*/

    int32_t     i32TruBassFrontEnableFlag;  /*Default 1 Enable*/
    int32_t     i32TruBassFrontCtrl;        /*Range 0 to 1.0 in Q31 format
                                            Default 0x26666666*/

    BDSP_Raaga_Audio_SpeakerSize eTruBassFrontSpeakerSize; /*Default BDSP_Raaga_Audio_SpeakerSize_eLFResponse150Hz
                                            with a value of 3*/
    BDSP_Raaga_Audio_SRSTruBassProcessMode   eSRSTruBassProcessMode; /*Default to stereo mode*/

    int32_t     i32DefinitionFrontEnableFlag;   /*Default 1 Enable*/
    int32_t     i32DefinitionFrontCtrl;     /*Range 0 to 1.0 in Q31 format
                                            Default 0x26666666*/


    int32_t     i32DialogClarityEnableFlag; /*Default 1 Enable*/
    int32_t     i32DialogClarityCtrl;       /*Range 0 to 1.0 in Q31 format
                                            Default 0x40000000*/

    int32_t     i32SurroundLevel;           /*Range 0 to 1.0 in Q31 format
                                            Default 4CCCCCCD*/

    int32_t     i32WowHDSRS3DEnableFlag;    /*Default 1 Enable*/
    int32_t     i32SRS3DHighBitRateFlag;    /*Default 1 and sets to High Bit Rate*/
    BDSP_Raaga_Audio_SRS3DMode   eWowHDSRS3DMode;/*Default BDSP_Raaga_Audio_SRS3DMode_eStereo with value 2*/
    int32_t     i32WowHDSRS3DSpaceCtrl;     /*Range 0 to 1.0 in Q31 format
                                            Default 0x66666666*/
    int32_t     i32WowHDSRS3DCenterCtrl;    /*Range 0 to 1.0
                                            Default 0x40000000*/
    int32_t     i32WowHDFocusEnableFlag;    /*Default 1 Enable*/
    int32_t     i32WowHDFocusCtrl;          /*Range 0 to 1.0 in Q31 format
                                            Default 0x33333333*/

    int32_t     i32Mono2StereoEnableFlag;


    BDSP_Raaga_Audio_AcMode  eOutputAcMode;      /*Default SRSTruSurroundHDOutputMode_e2_0_0*/
    int32_t     i32OuputLFEEnableFlag;      /*Used only for TSHD4.  Not used for Studio Sound*/

    int32_t     i32CertificationEnableFlag; /*Default 0 Disable*/
    BDSP_Raaga_Audio_AcMode  eInputAcMode;       /*Used only during certification*/
    int32_t     i32InputLFEPresentFlag;     /*Used only during certification*/

    BDSP_Raaga_Audio_StudioSoundTopLevelUserConfig           sTopLevelConfig;

}BDSP_Raaga_Audio_TruSurrndHDConfigParams;



/*
    Node3 of SRS STUDIO: This is a combination of ParamtericEQ, Graphic equalizer and Hard Limiter.
    -----------------------------------------------------------------------------------------------

    The user configuration of Node3 is BDSP_Raaga_Audio_SRS_EqualizerHardLimiterConfigParams

    User CFG structure for ParamtericEQ, Graphic equalizer and Hard Limiter are separately defined as below
*/

/*------------------------*/
/*ParamtericEQ user config*/
/*------------------------*/
typedef struct  BDSP_Raaga_FilterSpecPeq
{
    int32_t i32BandGain;        /*Range in dB: -12 to 12 db
                                        Format - q24
                                        Range in Fixed point: 0x00404DE6 to 0x03FB2783
                                        Default: 0dB
                                        Default in fixed point: 0x01000000*/

    uint32_t ui32BandFrequency; /*Range: 20 to 20000 Hz
                                  Default: 1000 Hz*/

    int32_t i32QFactor;         /*Range in floating point: 0.25 to 16.0
                                  Format: q24
                                  Range, fixed point: 0x00400000 to 0x10000000
                                  Default in floating point: 2.0
                                  Default in fixed point: 0x02000000

                                */
}BDSP_Raaga_FilterSpecPeq;

/*Coefficients should be generated by SRS_Apollo_TruEQ_HPF_Coefficients_Generator.exe*/
typedef struct
{
    int32_t             i32Order;   /*ParametricEQ filter order*/
    int32_t             i32Scale;   /*specified q-format of the coefficients. (1+i32Scale).(31-i32Scale) format
                                        Default - 1. i32Scale=1 implies a q-format of 2.30*/
    int32_t             i32FilterCoefficientB0;
    int32_t             i32FilterCoefficientA1;
    int32_t             i32FilterCoefficientB1;
    int32_t             i32FilterCoefficientA2;
    int32_t             i32FilterCoefficientB2;

}BDSP_Raaga_FilterCoefPeq;


typedef struct BDSP_Raaga_Audio_ParametricEqUserConfig
{
    int32_t i32mEnable[2];                          /*Left/Right channel ParametricEQ Processing Enable*/
                                                    /*Range: [0,1], Default: i32mEnable[0] = 1, i32mEnable[1] = 1*/
                                                    /*0 - Disable PEQ processing, 1 - Enable PEQ processing*/

    int32_t i32mBandEnable[2][8];                   /*Turn ON/OFF a specific band on one or both channels.*/
                                                    /*Range: [0,1], Default: 1 for all bands*/
                                                    /*0 - Disable band, 1 - Enable band*/

    int32_t i32mInputGain;                          /*Input gain*/
                                                    /*Range: Floating point: [0.0 to 10.0], Default: 1.0*/
                                                    /*Fixed point   : [0x0 to 0x50000000], Default: 0x08000000, QFormat - 5.27*/

    int32_t i32mOutputGain;                         /*Output gain*/
                                                    /*Range: Floating point: [0.0 to 10.0], Default: 1.0*/
                                                    /*Fixed point   : [0x0 to 0x50000000], Default: 0x08000000, QFormat - 5.27*/

    int32_t i32mBypassGain;                         /*Bypass gain*/
                                                    /*Range: Floating point: [0.0 to 10.0], Default: 1.0*/
                                                    /*Fixed point   : [0x0 to 0x50000000], Default: 0x08000000, QFormat - 5.27*/

    int32_t     i32CoefGenMode;                     /*Range - [0, 1], Default -> 0
                                                                                      0 - > use the coefficients provided by user, in field sFilterCoefPeq[]
                                                                                      1 - > use filter setting provided by user in sFilterSpecPeq[] to generate coefficients,
                                                        will overwrite the coefficients in sFilterCoefPeq[][]*/


    BDSP_Raaga_FilterCoefPeq    sFilterCoefPeq[3][8];   /*array size 3x8 implying 3 sampling frequencies with 8 bands each, will be used if i32CoefGenMode==0*/
    BDSP_Raaga_FilterSpecPeq    sFilterSpecPeq[8];      /* Filter specification for generating filter coeffcients, will be used if i32CoefGenMode==1*/

}BDSP_Raaga_Audio_ParametricEqUserConfig;

/*Default filter specification - Gain: 0dB, Frequency: 1000Hz, Q: 2 for all bands*/

/*Default ParametricEQ Filter coefficients*/

/*Sampling Rate - 32 kHz*/
/*
//band 0
sFilterCoefPeq[0][0].i32Order = 0x00000002
sFilterCoefPeq[0][0].i32Scale = 0x00000001
sFilterCoefPeq[0][0].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][0].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][0].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][0].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][0].i32FilterCoefficientB2 = 0x3bf14a80


//band 1
sFilterCoefPeq[0][1].i32Order = 0x00000002
sFilterCoefPeq[0][1].i32Scale = 0x00000001
sFilterCoefPeq[0][1].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][1].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][1].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][1].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][1].i32FilterCoefficientB2 = 0x3bf14a80


//band 2
sFilterCoefPeq[0][2].i32Order = 0x00000002
sFilterCoefPeq[0][2].i32Scale = 0x00000001
sFilterCoefPeq[0][2].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][2].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][2].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][2].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][2].i32FilterCoefficientB2 = 0x3bf14a80


//band 3
sFilterCoefPeq[0][3].i32Order = 0x00000002
sFilterCoefPeq[0][3].i32Scale = 0x00000001
sFilterCoefPeq[0][3].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][3].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][3].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][3].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][3].i32FilterCoefficientB2 = 0x3bf14a80


//band 4
sFilterCoefPeq[0][4].i32Order = 0x00000002
sFilterCoefPeq[0][4].i32Scale = 0x00000001
sFilterCoefPeq[0][4].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][4].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][4].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][4].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][4].i32FilterCoefficientB2 = 0x3bf14a80

//band 5
sFilterCoefPeq[0][5].i32Order = 0x00000002
sFilterCoefPeq[0][5].i32Scale = 0x00000001
sFilterCoefPeq[0][5].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][5].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][5].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][5].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][5].i32FilterCoefficientB2 = 0x3bf14a80

//band 6
sFilterCoefPeq[0][6].i32Order = 0x00000002
sFilterCoefPeq[0][6].i32Scale = 0x00000001
sFilterCoefPeq[0][6].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][6].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][6].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][6].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][6].i32FilterCoefficientB2 = 0x3bf14a80

//band 7
sFilterCoefPeq[0][7].i32Order = 0x00000002
sFilterCoefPeq[0][7].i32Scale = 0x00000001
sFilterCoefPeq[0][7].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][7].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][7].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][7].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][7].i32FilterCoefficientB2 = 0x3bf14a80
*/

/*Sampling Rate - 44.1 kHz*/
/*
//band 0
sFilterCoefPeq[1][0].i32Order = 0x00000002
sFilterCoefPeq[1][0].i32Scale = 0x00000001
sFilterCoefPeq[1][0].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][0].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][0].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][0].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][0].i32FilterCoefficientB2 = 0x3d07cd00

//band 1
sFilterCoefPeq[1][1].i32Order = 0x00000002
sFilterCoefPeq[1][1].i32Scale = 0x00000001
sFilterCoefPeq[1][1].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][1].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][1].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][1].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][1].i32FilterCoefficientB2 = 0x3d07cd00

//band 2
sFilterCoefPeq[1][2].i32Order = 0x00000002
sFilterCoefPeq[1][2].i32Scale = 0x00000001
sFilterCoefPeq[1][2].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][2].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][2].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][2].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][2].i32FilterCoefficientB2 = 0x3d07cd00

//band 3
sFilterCoefPeq[1][3].i32Order = 0x00000002
sFilterCoefPeq[1][3].i32Scale = 0x00000001
sFilterCoefPeq[1][3].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][3].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][3].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][3].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][3].i32FilterCoefficientB2 = 0x3d07cd00

//band 4
sFilterCoefPeq[1][4].i32Order = 0x00000002
sFilterCoefPeq[1][4].i32Scale = 0x00000001
sFilterCoefPeq[1][4].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][4].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][4].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][4].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][4].i32FilterCoefficientB2 = 0x3d07cd00

//band 5
sFilterCoefPeq[1][5].i32Order = 0x00000002
sFilterCoefPeq[1][5].i32Scale = 0x00000001
sFilterCoefPeq[1][5].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][5].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][5].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][5].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][5].i32FilterCoefficientB2 = 0x3d07cd00

//band 6
sFilterCoefPeq[1][6].i32Order = 0x00000002
sFilterCoefPeq[1][6].i32Scale = 0x00000001
sFilterCoefPeq[1][6].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][6].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][6].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][6].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][6].i32FilterCoefficientB2 = 0x3d07cd00

//band 7
sFilterCoefPeq[1][7].i32Order = 0x00000002
sFilterCoefPeq[1][7].i32Scale = 0x00000001
sFilterCoefPeq[1][7].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][7].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][7].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][7].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][7].i32FilterCoefficientB2 = 0x3d07cd00
*/

/*Sampling Rate - 48 kHz*/
/*
//band 0
sFilterCoefPeq[2][0].i32Order = 0x00000002
sFilterCoefPeq[2][0].i32Scale = 0x00000001
sFilterCoefPeq[2][0].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][0].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][0].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][0].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][0].i32FilterCoefficientB2 = 0x3d444480

//band 1
sFilterCoefPeq[2][1].i32Order = 0x00000002
sFilterCoefPeq[2][1].i32Scale = 0x00000001
sFilterCoefPeq[2][1].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][1].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][1].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][1].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][1].i32FilterCoefficientB2 = 0x3d444480

//band 2
sFilterCoefPeq[2][2].i32Order = 0x00000002
sFilterCoefPeq[2][2].i32Scale = 0x00000001
sFilterCoefPeq[2][2].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][2].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][2].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][2].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][2].i32FilterCoefficientB2 = 0x3d444480

//band 3
sFilterCoefPeq[2][3].i32Order = 0x00000002
sFilterCoefPeq[2][3].i32Scale = 0x00000001
sFilterCoefPeq[2][3].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][3].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][3].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][3].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][3].i32FilterCoefficientB2 = 0x3d444480

//band 4
sFilterCoefPeq[2][4].i32Order = 0x00000002
sFilterCoefPeq[2][4].i32Scale = 0x00000001
sFilterCoefPeq[2][4].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][4].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][4].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][4].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][4].i32FilterCoefficientB2 = 0x3d444480

//band 5
sFilterCoefPeq[2][5].i32Order = 0x00000002
sFilterCoefPeq[2][5].i32Scale = 0x00000001
sFilterCoefPeq[2][5].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][5].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][5].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][5].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][5].i32FilterCoefficientB2 = 0x3d444480

//band 6
sFilterCoefPeq[2][6].i32Order = 0x00000002
sFilterCoefPeq[2][6].i32Scale = 0x00000001
sFilterCoefPeq[2][6].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][6].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][6].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][6].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][6].i32FilterCoefficientB2 = 0x3d444480

//band 7
sFilterCoefPeq[2][7].i32Order = 0x00000002
sFilterCoefPeq[2][7].i32Scale = 0x00000001
sFilterCoefPeq[2][7].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][7].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][7].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][7].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][7].i32FilterCoefficientB2 = 0x3d444480
*/



/*GraphicEQ user config*/
typedef struct BDSP_Raaga_Audio_GraphicEqUserConfig
{

    int32_t i32mEnable[2];                          /*Left/Right channel GraphicEQ Processing Enable */
                                                    /*Range: [0,1], Default: i32mEnable[0] = 1, i32mEnable[1] = 1*/
                                                    /*0 - Disable GEQ processing, 1 - Enable GEQ processing*/

    int32_t i32mBandGain[2][10];                    /*Band gain for a specific band for one or both channels.*/
                                                    /*MAX_GEQ_BANDS=10*/

                                                    /*Range:*/
                                                        /*For 10 band design*/
                                                        /*Floating point: (0.251 to 4.0), Default: 1.0*/
                                                        /*In dB         : [-12dB to +12dB], Default: 0dB*/
                                                        /*Fixed point   : [0x08083127 to 0x7fffffff), Default: 0x20000000, QFormat : 3.29*/

                                                        /*For 5 band design*/
                                                        /*Floating point: (0.316 to 3.16), Default: 1.0*/
                                                        /*In dB         : [-10dB to +10dB], Default: 0dB*/
                                                        /*Fixed point   : [0x0a1cac08 to 0x651eb852), Default: 0x20000000, QFormat : 3.29*/


    int32_t i32mInputGain;                          /*Input gain applied to both channels prior to GEQ processing*/
                                                    /*Range:
                                                        For 10 band design
                                                        Floating point: (0 to 1.0), Default: 0.251
                                                        In dB         : [-ve infinity to 0dB], Default: -12dB
                                                        Fixed point   : [ 0 to 0x7fffffff), Default: 0x2020c49c
                                                        QFormat : 1.31


                                                        For 5 band design
                                                        Floating point: : (0 to 1.0), Default: 0.316
                                                        In dB         : [-ve infinity to 0dB],, Default: -10dB
                                                        Fixed point   :[ 0 to 0x7fffffff) Default: 0x2872b021, QFormat : 1.31*/


    int32_t i32mOutputGain;                         /*Output Gain*/
                                                    /*Range:
                                                        Floating point: (0.0 - 1.0), Default: 1.0
                                                        Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t i32mBypassGain;                         /*Bypass Gain
                                                      Range:
                                                        Floating point: (0.0 - 1.0), Default: 1.0
                                                        Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t  i32mFilterMode;                        /*GEQ filter configuration                                  Range: [0, 1, 2], Default: 0
                                                        0: five band config, 1: 10 band config, 2: Speaker compensation config
                                                    */

}BDSP_Raaga_Audio_GraphicEqUserConfig;

/*Hard limiter user config*/
typedef struct BDSP_Raaga_Audio_HardLimiterUserConfig
{
    int32_t             i32LimiterEnable;           /* Limiter enable separate control */
                                                    /* Default: 1;  1 -> Process, 0 ->  Disable of processing */

    int32_t             i32nchans;                  /* Number of input/output channels     */
                                                    /* Default val 2*/

    int32_t             i32blockSize;               /* Default: 256, range [256, 512, 768, 1024]*/

    int32_t             i32InputGain;               /* Input gain */
                                                    /* Default: 0x20000000 range[0, 0x7fffffff]
                                                       Floating val: default 1: range [0, 4] */

    int32_t             i32OutputGain;              /* Output gain */
                                                    /* Default: 0x20000000 range[0, 0x7fffffff]
                                                       Floating val: default 1: range [0, 4] */

    int32_t             i32BypassGain;              /* Bypass gain */
                                                    /* Default: 0x7fffffff range[0, 0x7fffffff]
                                                       Floating val: default 1: range [0, 1] */

    int32_t             i32LimiterBoost;            /* HL gain boost */
                                                    /* Default: 0x0168f5c3 range[0x000020c5, 0x7fffffff]
                                                       Floating val: default 2.82: range [0.001, 256] */

    int32_t             i32HardLimit;               /* HardLimit */
                                                    /* Default: 0x7fffffff range[0x00010945, 0x7fffffff]
                                                       Floating val: default 1: range [3.16227766e-5, 1] */

    int32_t             i32DelayLength;             /* DelayLength */
                                                    /* Default: 22 range[6, 22]
                                                       Floating val: default 22: range [6, 22] */
}BDSP_Raaga_Audio_HardLimiterUserConfig;

/*
Top Level Data structure for Node 3 of SRS Studio
*/
typedef struct BDSP_Raaga_Audio_SRS_EqualizerHardLimiterConfigParams
{
    BDSP_Raaga_Audio_StudioSoundTopLevelUserConfig       sTopLevelConfig;
    BDSP_Raaga_Audio_ParametricEqUserConfig              sParametricEqConfig;
    BDSP_Raaga_Audio_HighPassFilterUserConfig            sHighPassFilterConfig;
    BDSP_Raaga_Audio_GraphicEqUserConfig                 sGraphicEqConfig;
    BDSP_Raaga_Audio_HardLimiterUserConfig               sHardLimiterConfig;

}BDSP_Raaga_Audio_SRS_EqualizerHardLimiterConfigParams;


/*These structure are defined here till cit support gets checked in*/
/*
   This data structure defines G729 decoder user configuration parameters
*/
typedef struct  BDSP_Raaga_Audio_G729DecUsrCfg
{
    uint32_t    ui32OutMode;
            /* Default = 2; Output channel configuration */
    uint32_t    ui32ScaleOp;
            /*/ Default = 1; 0 -> FALSE, 1 -> TRUE */
    uint32_t    ui32ScaleIdx;
            /* Default = 0; 0 -> -3dB, 1 -> -6dB, 2 -> -9dB, 3 -> -12dB */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
    /*
    * For Output Port 0:
    * ui32OutputChannelMatrix[0] = 0;
    * ui32OutputChannelMatrix[1] = 1;
    *
    * for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
    * ui32OutputChannelMatrix[i] = 0xFFFFFFFF;
    *
    * For Output Port 1:
    * for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
    * ui32OutputChannelMatrix[i] = 0xFFFFFFFF; this value shows invalid
    */
}BDSP_Raaga_Audio_G729DecUsrCfg;


typedef struct  BDSP_Raaga_Audio_G729DecConfigParams
{
    uint32_t    ui32NumOutPorts;
/* Default = 1;*/
/*1 -> Stereo out, 2 -> Mono + Stereo out*/
    BDSP_Raaga_Audio_G729DecUsrCfg  sUsrOutputCfg[2];

}BDSP_Raaga_Audio_G729DecConfigParams;


typedef struct  BDSP_Raaga_Audio_VorbisUsrCfg
{
    uint32_t    ui32OutMode;            /* Default = 2; Output channel configuration */
    uint32_t    ui32OutLfe;             /* Default = 0; Output channel configuration */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

} BDSP_Raaga_Audio_VorbisUsrCfg;

typedef struct BDSP_Raaga_Audio_MlpConfigParams
{
    /* The Profile Select parameter has been reamed to outmode for commonality with other decoders */
    uint32_t    ui32OutMode;                /* {2 = STEREO, 7 = 5.1, 21 = 7.1} */
    uint32_t    DrcMode;                    /* {Disabled = 0, Follow = 1, Enabled = 2} */
    uint32_t    DrcBoost;
    uint32_t    DrcCut;
    uint32_t    DialogueEnable;
    int32_t     DialogueLevel;              /* in dB */
    uint32_t    EnableSpeakerRemapping;
    uint32_t    SpeakerOutLayout;
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS]; /* Default Channel Matrix = {0, 1, 2, 3, 4, 5, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF } */
}BDSP_Raaga_Audio_MlpConfigParams;

typedef struct  BDSP_Raaga_Audio_VorbisDecConfigParams
{
    uint32_t        ui32NumOutPorts;        /* Default = 1;                 = 1 - Multichannel / Stereo out,
                                                                                                                                                                  = 2 - Multichannel + Stereo out  */
    BDSP_Raaga_Audio_VorbisUsrCfg    sUsrOutputCfg[2];

} BDSP_Raaga_Audio_VorbisDecConfigParams;


typedef struct  BDSP_Raaga_Audio_FlacUsrCfg
{
    uint32_t    ui32OutMode;            /* Default = 2; Output channel configuration */
    uint32_t    ui32OutLfe;             /* Default = 0; Output channel configuration */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

} BDSP_Raaga_Audio_FlacUsrCfg;

typedef struct  BDSP_Raaga_Audio_FlacDecConfigParams
{
    uint32_t        ui32NumOutPorts;        /* Default = 1;                 = 1 - Multichannel / Stereo out,
                                                                                                                                                                  = 2 - Multichannel + Stereo out  */
    BDSP_Raaga_Audio_FlacUsrCfg    sUsrOutputCfg[2];

} BDSP_Raaga_Audio_FlacDecConfigParams;

typedef struct  BDSP_Raaga_Audio_MacUsrCfg
{
    uint32_t    ui32OutMode;
    /*
    Default = 2(2/0)
    Range -> 0-9
    0 - Dual to mono
    1 - Mono          - 1/0 format
    2 - Stereo        - 2/0 format
    */

    uint32_t    ui32DualMode;
    /*
    Default = 2(Stereo)
    Range : 0-3
    0 -> Normal Stereo - Left & Right
    1 -> Dual Left Mono
    2 -> Dual Right Mono
    3 -> Dual Mix Mono
    1. Mix left to left and right to right
    2. Mix right to left
    3. Mix left to right
    */

    uint32_t    ui32StereoMode;
    /*
    Default = 0
    Range - 0, 1
    0 - LoRo downmixed output
    1 - LtRt downmixed output
    This value is relevant only when ui32OutMode is 2.
    */

    uint32_t    ui32ScaleEnable;
        /*
            Default = 0
            0 -> FALSE
            1 -> TRUE
        */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];

} BDSP_Raaga_Audio_MacUsrCfg;

typedef struct  BDSP_Raaga_Audio_MacDecConfigParams
{
    uint32_t        ui32NumOutPorts;        /* Default = 1;                 = 1 - Multichannel / Stereo out,
                                                                                                                                                                  = 2 - Multichannel + Stereo out  */
    BDSP_Raaga_Audio_MacUsrCfg    sUsrOutputCfg[2];

} BDSP_Raaga_Audio_MacDecConfigParams;

typedef struct BDSP_Raaga_Audio_G729EncoderUserConfig
{
    /*Enables or Disable DTX */
    uint32_t     ui32DtxEnable; /* ui32DtxEnable =1 => DTX Enable
                                   ui32DtxDisable=0 => DTX Disable
                                   Default => ui32DtxEnable=0 (Disabled)*/
    /*Set required Bitrate */
    uint32_t     ui32Bitrate;   /*  ui32Bitrate = 0 => 6.4kbps
                                    ui32Bitrate = 1 => 8.0kbps
                                    ui32Bitrate = 2 =>11.8kbps*/
} BDSP_Raaga_Audio_G729EncoderUserConfig;

/*
   This data structure defines G711/G726 encoder user configuration parameters
*/
/*
1)  BDSP_Raaga_Audio_eCompressionType_aLaw_G726: A law compression to be applied followed by G.726.
2)  BDSP_Raaga_Audio_eCompressionType_uLaw_G726: U law compression to be applied followed by G.726.
3)  BDSP_Raaga_Audio_eCompressionType_aLaw_ext: A law compression has been externally applied. G.726 to be applied.
4)  BDSP_Raaga_Audio_eCompressionType_uLaw_ext: U law compression has been externally applied. G.726 to be applied.
5)  BDSP_Raaga_Audio_eCompressionType_aLaw_disableG726: G.711 A-law to be applied. G.726 compression is disabled
6)  BDSP_Raaga_Audio_eCompressionType_uLaw_disableG726: G.711 U-law to be applied. G.726 compression is disabled
*/

typedef enum BDSP_Raaga_Audio_eG711G726EncCompressionType
{
    BDSP_Raaga_Audio_eCompressionType_aLaw_G726=0,
    BDSP_Raaga_Audio_eCompressionType_uLaw_G726,
    BDSP_Raaga_Audio_eCompressionType_aLaw_ext,
    BDSP_Raaga_Audio_eCompressionType_uLaw_ext,
    BDSP_Raaga_Audio_eCompressionType_aLaw_disableG726,
    BDSP_Raaga_Audio_eCompressionType_uLaw_disableG726,
    BDSP_Raaga_Audio_eCompressionType_eINVALID=0x7fffffff

}BDSP_Raaga_Audio_eG711G726EncCompressionType;

/*
Bit rates supported
*/

typedef enum BDSP_Raaga_Audio_eG711G726EncBitRate
{
    BDSP_Raaga_Audio_eBitRate_16kbps=0,
    BDSP_Raaga_Audio_eBitRate_24kbps=1,
    BDSP_Raaga_Audio_eBitRate_32kbps=2,
    BDSP_Raaga_Audio_eBitRate_40kbps=3,
    /* Bit rates of 16, 24, 32, 40 kbps to be used along with G.726 */
    BDSP_Raaga_Audio_eBitRate_64kbps=4,
    /* Bit rate of 64 kbps to be used along with G.711 only*/
    BDSP_Raaga_Audio_eBitRate_eINVALID=0x7fffffff

}BDSP_Raaga_Audio_eG711G726EncBitRate;

typedef struct BDSP_Raaga_Audio_G711_G726EncConfigParams
{
    BDSP_Raaga_Audio_eG711G726EncCompressionType    eCompressionType;
    /* Default = BDSP_Raaga_Audio_eCompressionType_uLaw_disableG726 */
    BDSP_Raaga_Audio_eG711G726EncBitRate            eBitRate;
    /* Default = BDSP_Raaga_Audio_eBitRate_64kbps */
}BDSP_Raaga_Audio_G711_G726EncConfigParams;


typedef struct BDSP_Raaga_Audio_G726UsrCfg
{
    uint32_t ui32OutMode;
    /* Default = 2; Output channel configuration */
    uint32_t ui32ApplyGain;
    /* Default = 1, 0 -> FALSE, 1 -> TRUE */
    int32_t i32GainFactor;
    /* Default = 0x5A827999, corresponding to -3dB in Q1.31 format; valid
    only if ui32ApplyGain = 1 */
    uint32_t ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
    /*
    * For Output Port 0:
    * ui32OutputChannelMatrix[0] = 0;
    * ui32OutputChannelMatrix[1] = 1;
    *
    * for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
    * ui32OutputChannelMatrix[i] = 0xFFFFFFFF;
    *
    * For Output Port 1:
    * for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
    * ui32OutputChannelMatrix[i] = 0xFFFFFFFF; this value shows invalid
    */
} BDSP_Raaga_Audio_G726UsrCfg;

typedef struct BDSP_Raaga_Audio_G726ConfigParams
{
    uint32_t ui32NumOutPorts;
    /* Default = 1; 1 -> Stereo out, 2 -> Mono + Stereo out */
    uint32_t ui32OutputType;
    /* Default = 0; 0 -> PCM samples, 1 -> 8-bit ADPCM u/A-law packed data */
    BDSP_Raaga_Audio_G726UsrCfg sUsrOutputCfg[2];
} BDSP_Raaga_Audio_G726ConfigParams;

/*
   This data structure defines G722 encoder user configuration parameters
*/

typedef struct BDSP_Raaga_Audio_G722EncConfigParams
{
    int32_t dummy;
}BDSP_Raaga_Audio_G722EncConfigParams;

typedef struct BDSP_Raaga_Audio_AdpcmUsrCfg
{
    uint32_t ui32OutMode;
    /* Default = 1; Output channel configuration */
    uint32_t ui32ApplyGain;
    /* Default = 0, 0 -> FALSE, 1 -> TRUE */
    int32_t i32GainFactor;
    /* Default = 0x016A09E6, corresponding to +3dB in Q8.24 format; valid
    only if ui32ApplyGain = 1 */
    uint32_t ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
    /*
    * For Port 0:
    * ui32OutputChannelMatrix[0] = 0;
    * ui32OutputChannelMatrix[1] = 1;
    *
    * for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
    * ui32OutputChannelMatrix[i] = 0xFFFFFFFF;
    *
    * For Port 1:
    * for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
    * ui32OutputChannelMatrix[i] = 0xFFFFFFFF; this value shows invalid
    */
} BDSP_Raaga_Audio_AdpcmUsrCfg;
typedef struct BDSP_Raaga_Audio_AdpcmConfigParams
{
    uint32_t ui32NumOutPorts;
    /* Default = 1; 1 -> Stereo out, 2 -> Multichannel + Stereo out */
    BDSP_Raaga_Audio_AdpcmUsrCfg sUsrOutputCfg[2];
} BDSP_Raaga_Audio_AdpcmConfigParams;
typedef struct BDSP_Raaga_Audio_AMRWBConfigParams
{
    uint32_t    ui32allow_dtx;          /* Default = 1; 0-false 1-true */
    uint32_t    ui32bitstreamformat;        /* Default = 2; 1-itu    2-mime */
    uint32_t    ui32mode;
    /*
         Default 0
         mode = 0        BitRate = 6600
         mode = 1        BitRate = 8850
         mode = 2        BitRate = 12650
         mode = 3        BitRate = 14250
         mode = 4        BitRate = 15850
         mode = 5        BitRate = 18250
         mode = 6        BitRate = 19850
         mode = 7        BitRate = 23050
         mode = 8        BitRate = 23850
    */
}BDSP_Raaga_Audio_AMRWBConfigParams;

typedef struct BDSP_Raaga_Audio_ILBCConfigParams
{
    uint32_t   mode;        /* Default = 20;20->frame length 20msec ,30->frame length 30msec */

}BDSP_Raaga_Audio_ILBCConfigParams;


typedef struct  BDSP_Raaga_Audio_GenCdbItbConfigParams
{
    /* Switch to turn off encoder's mux output on the fly */
    BDSP_AF_P_EnableDisable     eEnableEncode;

    /* If snapshot of STC HW register is required for ESCR/PTS etc */
    BDSP_AF_P_EnableDisable     eSnapshotRequired;
    /* The RDB register to be read for encoder's ESCR clock */
    uint32_t                    ui32EncSTCAddr;
    /* Arrival to presentation time in Msecs, described in section 3.2.1 of transcode system arch 1.4 */
    uint32_t                    ui32A2PInMilliSeconds;

    /* Enable (1) / Disable (0) : Default - 0 (false)*/
    BDSP_AF_P_EnableDisable     eStcItbEntryEnable;

    /* The RDB register address to read for encoder's STC clock (Upper)*/
    uint32_t                    ui32EncSTCAddrUpper;

    /* STC Offset between audio and video encoder in 27 MHz domain*/
    uint64_t                    ui64AVSTCOffset;

} BDSP_Raaga_Audio_GenCdbItbConfigParams;

typedef struct BDSP_Raaga_Audio_SpectrumAnalyserConfigParams
{

     /* This value indicates whether downmixeing enabled or disable. set to "1"
        to ensure that the input channels and downmixed and then the downmixed
        data is used for generating spectral lines. If set to zero, then data
        is generated for all input channels.

        Range: 0 to 1.
               Set  to 0 for disable downmix, set  to 1 for enable downmix

       */

    int32_t ui32UseDownmixedDataForWindowFFT ;

    /* This value indicates Frame rate of Host device. If set to -1, then all
         input data is passed through FFT and is pushed to output buffer.
       If set to a valid sampling frequency like 25/30, only that many FFT
         blocks are computed.

        Range: -1 to 93.

                Set to -1 for all input data is passed through FFT
                Set to  a valid sampling frequency like 25/30.


        */

    int32_t i32HostRequiredFrameRate ;



} BDSP_Raaga_Audio_SpectrumAnalyserConfigParams;


/* BTSC encoder user config params

    This data structure defines BTSC configuration parameters
*/

typedef struct BDSP_Raaga_Audio_BtscEncoderConfigParams
{
    BDSP_AF_P_EnableDisable eBTSCEnableFlag;                    /*Default Enable*/
    BDSP_AF_P_EnableDisable eUseDeEmphasizedSourceSignals;      /*Default Enable*/
    BDSP_AF_P_EnableDisable eMainChannelPreEmphasisOn;          /*Default Enable*/
    BDSP_AF_P_EnableDisable eEquivalenMode75MicroSec;           /*Default Disable*/
    BDSP_AF_P_EnableDisable eClipDiffChannelData;               /*Default Enable*/
    BDSP_AF_P_EnableDisable eDiffChannelPreEmphasisOn;          /*Default Enable*/
    BDSP_AF_P_EnableDisable eUseLowerThresholdSpGain;           /*Default Enable*/
    uint32_t ui32AttenuationFactor;                             /*Q-format 3.29 format: Maximum is 0x7fffffff and default is : 0x20000000  */
    uint32_t ui32SRDCalibrationFactor;                          /*Q-format 4.28 format: Maximum value is 0x7fffffff corresponding to 8.0:
                                                                Default :12BE76C9 for next gen 0xE666666 for 7425 Ref board*/
    BDSP_AF_P_EnableDisable eSumChanFreqDevCtrl;            /*Default Enable Used to Control FM Deviation of Sum Channel to 25Khz*/
    BDSP_AF_P_EnableDisable eDiffChanFreqDevCtrl;           /*Default Enable Used to Control FM Deviation of Diff Channel to 50Khz*/

    BDSP_AF_P_EnableDisable eOpFreqDevCtrl;                 /*Default Enable Used to Control FM Deviation of Final Output to 50Khz*/
    BDSP_AF_P_EnableDisable eFreqGeneration;                /*Default Disable. Used to Generate Test Tones*/
    int32_t   i32Frequency;                                 /*Default 0 generates frequency sweep.  Programming to a fixed value generates tone*/

    uint32_t i32SumChannelLevelCtrl;                /*Q-format 3.29 format: Maximum is 0x7fffffff and default is : 0x20000000  */
    uint32_t i32DiffChannelLevelCtrl;               /*Q-format 3.29 format: Maximum is 0x7fffffff and default is : 0x20000000  */

    BDSP_AF_P_EnableDisable eBTSCMonoModeOn;  /* Default Disable */
    BDSP_AF_P_EnableDisable eMuteRightChannel; /*Default Disbale - Mute the Right for Stereo Separation Tests*/

    int32_t OpAmpFilterCoefs[21]; /*21 tap DAC compensation filter*/

}BDSP_Raaga_Audio_BtscEncoderConfigParams;

typedef struct  BDSP_Raaga_Audio_G723_1_UsrCfg
{
    uint32_t    ui32OutMode;    /* Default = 2; Output channel configuration */
    uint32_t    ui32ScaleOp;    /* Default = 1; 0 -> FALSE, 1 -> TRUE */
    uint32_t    ui32ScaleIdx;   /* Default = 0; 0 -> -3dB, 1 -> -6dB, 2 -> -9dB, 3 -> -12dB */
    uint32_t    ui32OutputChannelMatrix[BDSP_AF_P_MAX_CHANNELS];
    /*
    * For Output Port 0:
    * ui32OutputChannelMatrix[0] = 0;
    * ui32OutputChannelMatrix[1] = 1;
    *
    * for(i=2; i<BDSP_AF_P_MAX_CHANNELS; i++)
    * ui32OutputChannelMatrix[i] = 0xFFFFFFFF;
    *
    * For Output Port 1:
    * for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
    * ui32OutputChannelMatrix[i] = 0xFFFFFFFF; this value shows invalid
    */

} BDSP_Raaga_Audio_G723_1DEC_UsrCfg;

typedef struct  BDSP_Raaga_Audio_G723_1_ConfigParams
{
    uint32_t                ui32NumOutPorts;    /* Default = 1; 1 -> Stereo out, 2 -> Mono + Stereo out */

    BDSP_Raaga_Audio_G723_1DEC_UsrCfg    sUsrOutputCfg[2];

} BDSP_Raaga_Audio_G723_1DEC_ConfigParams;

typedef struct BDSP_Raaga_Audio_ISACConfigParams
{
    uint16_t    ui16frameLen ;              /* Default = 30;range: 30 msec or 60 msec */

    uint16_t    ui16CodingMode;             /* Default = 0;range: 0 - 1, 0 - channel-adaptive, 1 - channel-independent  */

    uint16_t    ui16nbTest;                 /* Default = 1;range: 0 - 1, 0 - wide band, 1 - narrow band */

    uint16_t    ui16fixedFL;                /* Default = 1;range: 0 - 1, 0 - Variable frame length, 1-Fixed frame length*/

    uint16_t    ui16rateBPS;                /* Default = 32000;range: 10000 - 32000 */

    uint16_t    ui16bottleneck;             /* Default = 32000;range: 10000 - 32000 */

    uint16_t    ui16payloadSize;            /* Default = 200;range: 100 - 400     */

    uint16_t    ui16payloadRate;            /* Default = 32000;range: 32000 - 53400 */
}BDSP_Raaga_Audio_ISACConfigParams;


typedef struct
{
    uint32_t WrkRate;    /* Bitrate: 0 - 6.3 Kbps 1- 5.3 Kbps Default: 0 */
    uint32_t UseHp;      /* High Pass Filter 1 - Enabled 0 - Disabled :  Default 1 */
    uint32_t UseVx;      /* VAD Status 1 - Enabled 0 - Disabled : Default Disable */

}BDSP_Raaga_Audio_G723EncoderUserConfig;
/* Video specific definitions */
/****************************************************************************/
/****************************************************************************/
/************************* VIDEO Algo  **************************************/
/****************************************************************************/
/****************************************************************************/
/* Video Encoder UD */
/* Encoder IDS userconfig */
typedef struct BDSP_VideoEncodeTaskDatasyncSettings
{
    BDSP_AF_P_EnableDisable eEnableStc;             /* Will be used if IpPortType is I2S*/
    uint32_t ui32StcAddress;                        /* For RfI2s i.e. BTSC */
    BDSP_AF_P_EnableDisable eTsmEnable;          /* Enable Tsm based drop in encoder */

} BDSP_VideoEncodeTaskDatasyncSettings ;

/* H.264 */
/***************************************************************************
Summary:
    Defines the Slice type

Description:

See Also:
    None.
****************************************************************************/
typedef enum
{
    BDSP_Raaga_VideoSlice_eI                = 0x0,
    BDSP_Raaga_VideoSlice_eP,
    BDSP_Raaga_VideoSlice_eB,
    BDSP_Raaga_VideoSlice_eMax,
    BDSP_Raaga_VideoSlice_eInvalid   = 0x7FFFFFFF
}BDSP_Raaga_VideoSliceType;

/***************************************************************************
Summary:
    Defines the encode mode

Description:

See Also:
    None.
****************************************************************************/
typedef enum
{
    BDSP_Raaga_VideoEncodeMode_eHighDelay                       = 0x0,
    BDSP_Raaga_VideoEncodeMode_eLowDelay                        = 0x1,
    BDSP_Raaga_VideoEncodeMode_eAfap                            = 0x2,
    BDSP_Raaga_VideoEncodeMode_eInvalid                     = 0x7FFFFFFF
}BDSP_Raaga_VideoEncodeMode;

/***************************************************************************
Summary:
    GOP structure to be used for encoding

Description:

See Also:
    None.
****************************************************************************/
typedef enum
{
    BDSP_Raaga_VideoGopStruct_eIOnly                            = 0x0,
    BDSP_Raaga_VideoGopStruct_eIP                               = 0x1,
    BDSP_Raaga_VideoGopStruct_eIPB                              = 0x2,
    BDSP_Raaga_VideoGopStruct_eIPBB                             = 0x3,
    BDSP_Raaga_VideoGopStruct_eMax,
    BDSP_Raaga_VideoGopStruct_eInvalid                          = 0x7FFFFFFF
}BDSP_Raaga_VideoGopStruct;

/* H264 Encoder User config */
typedef enum
{
    BDSP_Raaga_VideoH264Profile_eBaseline                                   = 66,
    BDSP_Raaga_VideoH264Profile_eMain                                       = 77,
    BDSP_Raaga_VideoH264Profile_eExtended                                   = 88,
    BDSP_Raaga_VideoH264Profile_eFRExtProfileHigh                           = 100,
    BDSP_Raaga_VideoH264Profile_eFRExtProfileHigh10                         = 110,
    BDSP_Raaga_VideoH264Profile_eFRExtProfileHigh422                        = 122,
    BDSP_Raaga_VideoH264Profile_eFRExtProfileHigh444                        = 244,
    BDSP_Raaga_VideoH264Profile_eFRExtProfileHighCavlc444                   = 44
}BDSP_Raaga_VideoH264ProfileIdc;
/***********************************
*
* Following Config Parameters can be set
* as one time parameters
*
***********************************/
typedef struct BDSP_Raaga_VideoBH264RCConfigStruct
{
    uint32_t   basicunit; /* Size of Basic Unit in MBs. '0' impliese FrameLevel RC */
    /* Min/ Max Qp for every slice I/P/B */
    uint32_t            RCMinQP[BDSP_Raaga_VideoSlice_eMax];
    uint32_t            RCMaxQP[BDSP_Raaga_VideoSlice_eMax];

    uint32_t    RCMaxQPChange;/*Max Qp Change allowed between consequetive BUs in BU-level RC
                              * At frame-level, Max Qp change allowed is sett to 2 for smooth quality*/
    uint32_t    SeinitialQp; /* Initial Qp: Initial config parameter.*/

}BDSP_Raaga_VideoBH264RCConfigStruct;


typedef struct BDSP_Raaga_VideoBH264UserConfig
{
    BDSP_Raaga_VideoH264ProfileIdc                  eProfileIDC;
    uint32_t                                        ui32LevelIdc;           /* ranges from 9 to 51. For SD it is 30 */
    BDSP_Raaga_VideoEncodeMode                      eMode;                  /* (High Delay, Low Delay, AFAP)     */
    uint32_t                                        ui32TargetBitRate;          /* Number of bits per sec.  */
    uint32_t                                        ui32EncodPicWidth;
    uint32_t                                        ui32EncodPicHeight;
    uint32_t                                        ui32IntraPeriod;
    uint32_t                                        ui32IDRPeriod;
    BDSP_Raaga_VideoGopStruct                       eGopStruct;
    BDSP_Raaga_Video_eBOOLEAN                       eDblkEnable;
    BDSP_Raaga_Video_eBOOLEAN                       eRateControlEnable;
    uint32_t                                        ui32End2EndDelay;
    /* Rate control */
    BDSP_Raaga_VideoBH264RCConfigStruct             sRCConfig;
    uint32_t                                        ui32FrameRate;

    BDSP_Raaga_Video_eBOOLEAN                       eSendAud;

    /* 0 - Disable Variance based intra/inter mod decsion; 1 - Enable Variance based intra/inter mod decsion */
    BDSP_Raaga_Video_eBOOLEAN                       eVarModeDecsOptEnable;
    /* 0 - Disable Subpel; 1 - Enable Subpel interpolation filter */
    BDSP_Raaga_Video_eBOOLEAN                       eSubPelEnable;
    /* 0 - Disable I4x4 mode  in I frm; 1 - Enable I4x4 in I frm */
    BDSP_Raaga_Video_eBOOLEAN                       eI4x4Enable;
    /* 0 - Disable CC data ; 1 - Enable CC data */
    BDSP_Raaga_Video_eBOOLEAN                       eSendCC;
    /* 0 - Disable Scene change ; 1 - Enable Scene change */
    BDSP_Raaga_Video_eBOOLEAN                       eSceneChangeEnable;
}BDSP_Raaga_VideoBH264UserConfig;

typedef struct BDSP_Raaga_VideoBX264RCConfigStruct
{
    uint32_t    basicunit; /* Size of Basic Unit in MBs. '0' impliese FrameLevel RC */
    uint32_t    RCMinQP[BDSP_Raaga_VideoSlice_eMax];
    uint32_t    RCMaxQP[BDSP_Raaga_VideoSlice_eMax];
    uint32_t    RCMaxQPChange;/*Max Qp Change allowed between consequetive BUs in BU-level RC */
    uint32_t    SeinitialQp; /* Initial Qp: Initial config parameter.*/
}BDSP_Raaga_VideoBX264RCConfigStruct;
typedef struct BDSP_Raaga_VideoBX264UserConfig
{
    BDSP_Raaga_VideoH264ProfileIdc                  eProfileIDC;
    uint32_t                                        ui32LevelIdc;           /* ranges from 9 to 51. For SD it is 30 */
    BDSP_Raaga_VideoEncodeMode                      eMode;                  /* (High Delay, Low Delay, AFAP)     */
    uint32_t                                        ui32TargetBitRate;          /* Number of bits per sec.  */
    uint32_t                                        ui32EncodPicWidth;
    uint32_t                                        ui32EncodPicHeight;
    uint32_t                                        ui32IntraPeriod;
    uint32_t                                        ui32IDRPeriod;
    BDSP_Raaga_VideoGopStruct                       eGopStruct;
    BDSP_Raaga_Video_eBOOLEAN                       eDblkEnable;
    BDSP_Raaga_Video_eBOOLEAN                       eRateControlEnable;
    uint32_t                                        ui32End2EndDelay;
    BDSP_Raaga_VideoBX264RCConfigStruct             sRCConfig;
    uint32_t                                        ui32FrameRate;
    uint32_t                                        ui32AspectRatioIdc;
    uint32_t                                        ui32SARWidth;
    uint32_t                                        ui32SARHeight;
    BDSP_Raaga_Video_eBOOLEAN                       eSendAud;
    BDSP_Raaga_Video_eBOOLEAN                       eVarModeDecsOptEnable;
    BDSP_Raaga_Video_eBOOLEAN                       eSubPelEnable;
    BDSP_Raaga_Video_eBOOLEAN                       eI4x4Enable;
    BDSP_Raaga_Video_eBOOLEAN                       eSendCC;
    BDSP_Raaga_Video_eBOOLEAN                       eSceneChangeEnable;
    BDSP_Raaga_Video_eBOOLEAN                       encodingMode;
    uint32_t                                        picStructure;
    uint32_t                                        InputYUVFormat;
}BDSP_Raaga_VideoBX264UserConfig;
typedef struct BDSP_Raaga_VideoBXVp8RCConfigStruct
{
    uint32_t    ui32BasicUnit; /* Size of Basic Unit in MBs. '0' impliese FrameLevel RC */
    uint32_t    RCMinQP[BDSP_Raaga_VideoSlice_eMax];
    uint32_t    RCMaxQP[BDSP_Raaga_VideoSlice_eMax];
    uint32_t    RCMaxQPChange;/*Max Qp Change allowed between consequetive BUs in BU-level RC */
    uint32_t    SeinitialQp; /* Initial Qp: Initial config parameter.*/
}BDSP_Raaga_VideoBXVP8RCConfigStruct;
typedef struct BDSP_Raaga_VideoBXVP8UserConfig
{
    BDSP_Raaga_VideoEncodeMode                      eMode;                  /* (High Delay, Low Delay, AFAP)     */
    uint32_t                                        ui32TargetBitRate;          /* Number of bits per sec.  */
    uint32_t                                        ui32EncodPicWidth;
    uint32_t                                        ui32EncodPicHeight;
    uint32_t                                        ui32IntraPeriod;
    uint32_t                                        ui32IDRPeriod;
    BDSP_Raaga_VideoGopStruct                       eGopStruct;
    BDSP_Raaga_Video_eBOOLEAN                       eDblkEnable;
    BDSP_Raaga_Video_eBOOLEAN                       eRateControlEnable;
    uint32_t                                        ui32End2EndDelay;
    BDSP_Raaga_VideoBXVP8RCConfigStruct             sRCConfig;
    uint32_t                                        ui32FrameRate;
    uint32_t                                        ui32AspectRatioIdc;
    uint32_t                                        ui32SARWidth;
    uint32_t                                        ui32SARHeight;
    BDSP_Raaga_Video_eBOOLEAN                       eSendCC;
    BDSP_Raaga_Video_eBOOLEAN                       eSceneChangeEnable;
    uint32_t                                        picStructure;
    uint32_t                                        InputYUVFormat;
}BDSP_Raaga_VideoBXVP8UserConfig;
typedef struct BDSP_Raaga_Audio_LpcmEncConfigParams
{
int32_t dummyLpcmEnc;
}BDSP_Raaga_Audio_LpcmEncConfigParams;

/* User Config Structure for Opus Encoder */
typedef struct
{
    uint32_t ui32BitRate;    /* Encoder Bit Rate - Values 6kbps - 510kbps Default : 64kbps ( Bitrate sweetspot for 48k audio at 20ms )  */
    uint32_t ui32FrameSize;  /* Frame size -  Values 120-2880, Default : 960 (20 ms for 48k audio) */
    uint32_t ui32EncodeMode; /* Encoder Mode - Values 0,1,2 : 0-SILK only,1-Hybrid,2-CELT only, Default : 1 - Hybrid */
    uint32_t ui32VBREnabled; /* Variable Bit Rate Coding Enable - 0-CBR,1-VBR,2-CVBR, Default: 1 - VBR Encoding */
    uint32_t ui32Complexity; /* Computational Complexity for the encoder - Values 0-10 in increasing order of complexity, Default: 10- Maximum complexity*/
}BDSP_Raaga_Audio_OpusEncConfigParams;

typedef struct BDSP_Raaga_Audio_DDPEncConfigParams
{   /*
    Input channel routing [Default : 0,1,2,3,4,5,6,7]
                Use this option to designate which input file channels
                should be routed to which encoder inputs. Each value
                in the routing string corresponds to a specific channel:
                L, R, C, LFE, Ls, Rs, Lrs, Rrs
                Use -1 instead of a number to route silence to an input.
                Example: 0,0,0,-1,1,1 routes the first interleaved channel to
                left, center, and right, routes silence to LFE, and routes
                the second interleaved channel to Ls and Rs.
    */
    int32_t     i32ChannelRouting[8];


    /*Encoder mode [Default : 8 = Dolby Digital Plus Consumer Transcoding (bsid 12)]
                8 = Dolby Digital Plus Consumer Transcoding (bsid 12)
                9 = Dolby Digital Consumer Transcoding (bsid 4)
    */
    uint32_t    ui32Mode;

    /*Audio coding mode [Default : 7  = L R C LFE Ls Rs]
                Broadcast Standard channel order (LFE optional):
                    full mix channels (-i)
                    L R C LFE Ls Rs x1  x2
                0  = L R
                1  =     C
                2  = L R
                3  = L R C LFE
                4  = L R   LFE Ls
                5  = L R C LFE Ls
                6  = L R   LFE Ls Rs
                7  = L R C LFE Ls Rs
                19 = L R C LFE Ls Rs Lvh Rvh
                21 = L R C LFE Ls Rs Lrs Rrs
    */
    uint32_t    ui32AudioCodingMode;

    /*
    Center Mix Level [Default : 0 = 0.707 (-3.0 dB)]
                0 = 0.707 (-3.0 dB)
                1 = 0.595 (-4.5 dB)
                2 = 0.500 (-6.0 dB)
                3 = not set */
    uint32_t    ui32CenterMixLevel;

   /*
    Dialog Normalization [Default : 31 = -31dB (quiet input)]
                1  = -1dB (loudest input)
                ...
                27 = -27dB
                31 = -31dB (quiet input)
    */
    uint32_t    ui32DialNorm;

    /* Dialog Normalization channel 2 [Default : 31] */
    uint32_t    ui32DialNormChannel2;

    /*
    Dolby Surround mode [Default : 0 = not indicated]
                0 = not indicated
                1 = Dolby Surround disabled
                2 = Dolby Surround enabled
    */
    uint32_t    ui32DolbySurroundMode;

    /*
    Low frequency effects channel on/off [Default : 1 = LFE on]
                0 = LFE off
                1 = LFE on
    */
    uint32_t    ui32LfeEnable;

    /* Evolution metadata [Default : ] */
    uint32_t    ui32EvoMdAvailable;

    /*
    Surround Mix Level [Default:0 = 0.707 (-3.0 dB)]
                0 = 0.707 (-3.0 dB)
                1 = 0.500 (-6.0 dB)
                2 = 0.000 (-inf dB)
                3 = not set */
    uint32_t    ui32SurroundMixLevel;

    /*
    90 deg phase shift surrounds [Default:1 = enabled]
                0 = disabled
                1 = enabled
    */
    uint32_t    ui32NinetyDegreeFilterEnable;

    /* Datarate (in kilobits per second) [ Default:-dr384] */
    uint32_t    ui32Datarate;

    /*
    Preferred stereo downmix mode [Default:-xa0 = not indicated]
                0 = not indicated
                1 = Pro Logic downmix preferred
                2 = Stereo downmix preferred
                3 = Pro Logic II downmix preferred
    */
    uint32_t    ui32Downmix;

    /*
    Lt/Rt center mix level [Default:-xb4 = 0.707 (-3.0 dB)]
                        0 = 1.414 (+3.0 dB)
                        1 = 1.189 (+1.5 dB)
                        2 = 1.000 (0.0 dB)
                        3 = 0.841 (-1.5 dB)
                        4 = 0.707 (-3.0 dB)
                        5 = 0.595 (-4.5 dB)
                        6 = 0.500 (-6.0 dB)
                        7 = 0.000 (-inf dB)
                        8 = not set */
    uint32_t    ui32LtRtCenterMixLevel;

    /*
    Lt/Rt surround mix level [Default: 4 = 0.707 (-3.0 dB)]
                3 = 0.841 (-1.5 dB)
                4 = 0.707 (-3.0 dB)
                5 = 0.595 (-4.5 dB)
                6 = 0.500 (-6.0 dB)
                7 = 0.000 (-inf dB)
                8 = not set */
    uint32_t    ui32LtRtSurrMixLevel;

    /*
    Lo/Ro center mix level [Default : 4 = 0.707 (-3.0 dB)]
                0 = 1.414 (+3.0 dB)
                1 = 1.189 (+1.5 dB)
                2 = 1.000 (0.0 dB)
                3 = 0.841 (-1.5 dB)
                4 = 0.707 (-3.0 dB)
                5 = 0.595 (-4.5 dB)
                6 = 0.500 (-6.0 dB)
                7 = 0.000 (-inf dB)
                8 = not set */
    uint32_t    ui32LoRoCenterMixLevel;

    /*
    Lo/Ro surround mix level [Default: 4 -xe4 = 0.707 (-3.0 dB)]
                3 = 0.841 (-1.5 dB)
                4 = 0.707 (-3.0 dB)
                5 = 0.595 (-4.5 dB)
                6 = 0.500 (-6.0 dB)
                7 = 0.000 (-inf dB)
                8 = not set */
    uint32_t    ui32LoRoSurrMixLevel;

    /*
    Dolby Surround EX mode [Default: 0 -xf0 = not indicated]
                0 = not indicated
                1 = Dolby Surround EX disabled
                2 = Dolby Surround EX enabled
                */
    uint32_t    ui32DolbyEXMode;

    /*
    Dolby Headphone mode [Default: 0 -xg0 = not indicated]
                0 = not indicated
                1 = Dolby Headphone disabled
                2 = Dolby Headphone enabled
                */
    uint32_t    ui32DolbyHeadPhoneMode;

    /*
    A/D Converter Type [ Default:0 -xh0 = Standard]
                0 = Standard
                1 = HDCD
                */
    uint32_t    ui32ADConvType;

    /*
    Send Audio Production Info [Default: 0 -xi0 = Don't send info]
                0 = Don't send info
                1 = Send info
                */
    uint32_t    ui32SendAudioProductionInfo;

    /*
    Send Audio Production Info 2 [ Default: 0 -xis0 = Don't send info]
                0 = Don't send info
                1 = Send info
                */
    uint32_t    ui32SendAudioProductionInfo2;

    /*
    Audio production mixing level [ Default:105 -xm105 = 105 dB SPL]
                80 =  80 dB SPL
                105 = 105 dB SPL
                111 = 111 dB SPL
                */
    uint32_t    ui32AudioProductionMixLevel;

    /*
    Audio production mixing level 2 [ Default:105 -xm105 = 105 dB SPL]
                80 =  80 dB SPL
                105 = 105 dB SPL
                111 = 111 dB SPL
                */
    uint32_t    ui32AudioProductionMixLevel2;

    /*
    Cpyright flag [Default: 1 -xn1 = copyrighted material]
                0 = non-copyrighted material
                1 = copyrighted material
                */
    uint32_t    ui32CopyrightFlag;

    /*
    Original Bitstream flag [Default: 1 -xo1 = not copied]
                0 = copied
                1 = not copied
                */
    uint32_t    ui32OriginalBitstream;

    /*
    Bitstream Mode [ Default: 0 -xp0 = Main audio service: complete main (CM)]
                0 = Main audio service: complete main (CM)
                1 = Main audio service: music and effects (ME)
                2 = Associated audio service: visually impaired (VI)
                3 = Associated audio service: hearing impaired (HI)
                4 = Associated audio service: dialogue (D)
                5 = Associated audio service: commentary (C)
                6 = Associated audio service: emergency (E)
                7 = Associated audio service: voice over (VO)
                */
    uint32_t    ui32BitstreamMode;

    /*
    Audio production room type [ Default: 2 -xr2 = Small room, flat monitor]
                0 = Not Indicated
                1 = Large Room, X curve monitor
                2 = Small room, flat monitor
                */
    uint32_t    ui32AudioProductionRoomType;

    /*
    Audio production room type 2 [ Default:2 -xrs2 = Small room, flat monitor]
                0 = Not Indicated
                1 = Large Room, X curve monitor
                2 = Small room, flat monitor
                */
    uint32_t    ui32AudioProductionRoomType2;

    /* Additional Bitstream Information Flag [Default : 0 = Disabled]
                0 = Disabled
                1 = Enabled
                */
    uint32_t    ui32AdditionalBsiEnable;

    /* Dolby Certification Flag [Default : 0 = Disabled]
                0 = Disabled
                1 = Enabled
                */
    uint32_t    ui32DolbyCertificationFlag;

    /* Enable Low Complexity Encoding [Default : 0 = Disabled]
                0 = Disabled
                1 = Enabled
        This flag will choose the optimal set of tools to make an encoding
                */
    uint32_t    ui32LowComplexity;

    /* Bitmask of payload IDs to try to pass-through */
    int32_t     ui32SupportedEvolutionPayloads;

    /* Enable / Disable SPDIF or HDMI packing [Default : 1 = enabled] */
    uint32_t    ui32SpdifEnable;

    /* Additional Bitstream Information String [Default : 0]
       A maximum of 64 characters can be represented as ASCII converted integers
       Eg: "Hello" maps to {104, 101, 108, 108, 111, 0}
                */
    int32_t     i32AddBsi[64];

    /* Enable / Disable Atmos lock [Default : 0 = disabled] */
    uint32_t    ui32AtmosLockEnabled;

} BDSP_Raaga_Audio_DDPEncConfigParams;

extern const BDSP_P_Audio_FrameSyncTsmConfigParams   BDSP_sDefaultFrameSyncTsmSettings;
extern const BDSP_Raaga_Audio_MpegConfigParams  BDSP_sMpegDefaultUserConfig;
extern const BDSP_Raaga_Audio_AacheConfigParams BDSP_sAacheDefaultUserConfig;
extern const BDSP_Raaga_Audio_DolbyPulseUserConfig BDSP_sDolbyPulseDefaultUserConfig;
extern const BDSP_Raaga_Audio_DDPMultiStreamConfigParams  BDSP_sDDPDefaultUserConfig;
extern const BDSP_Raaga_Audio_DtsHdConfigParams  BDSP_sDtsHdDefaultUserConfig;
extern const BDSP_Raaga_Audio_DtslbrConfigParams  BDSP_sDtsLbrDefaultUserConfig;
extern const BDSP_Raaga_Audio_LpcmUserConfig  BDSP_sLcpmDvdDefaultUserConfig;
extern const BDSP_Raaga_Audio_PassthruConfigParams  BDSP_sPcmDefaultUserConfig;
extern const BDSP_Raaga_Audio_PcmWavConfigParams  BDSP_sPcmWavDefaultUserConfig;
extern const BDSP_Raaga_Audio_AmrConfigParams BDSP_sAmrDefaultUserConfig;
extern const BDSP_Raaga_Audio_WmaConfigParams  BDSP_sWmaDefaultUserConfig;
extern const BDSP_Raaga_Audio_WmaProConfigParams  BDSP_sWmaProDefaultUserConfig;
extern const BDSP_Raaga_Audio_DraConfigParams  BDSP_sDraDefaultUserConfig;
extern const BDSP_Raaga_Audio_RalbrConfigParams  BDSP_sRalbrDefaultUserConfig;
extern const BDSP_Raaga_Audio_AdpcmConfigParams  BDSP_sAdpcmDefaultUserConfig;
extern const BDSP_Raaga_Audio_G726ConfigParams BDSP_sG711G726DecUserConfig;
extern const BDSP_Raaga_Audio_G729DecConfigParams BDSP_sG729DecUserConfig;
extern const BDSP_Raaga_Audio_VorbisDecConfigParams BDSP_sVorbisDecUserConfig;
extern const BDSP_Raaga_Audio_FlacDecConfigParams BDSP_sFlacDecUserConfig;
extern const BDSP_Raaga_Audio_MacDecConfigParams BDSP_sMacDecUserConfig;
extern const BDSP_Raaga_Audio_AmrwbdecConfigParams BDSP_sAmrwbdecDefaultUserConfig;
extern const BDSP_Raaga_Audio_iLBCdecConfigParams BDSP_siLBCdecDefaultUserConfig;
extern const BDSP_Raaga_Audio_iSACdecConfigParams BDSP_siSACdecDefaultUserConfig;
extern const BDSP_Raaga_Audio_UdcdecConfigParams  BDSP_sUdcdecDefaultUserConfig;
extern const BDSP_Raaga_Audio_DolbyAacheUserConfig BDSP_sDolbyAacheDefaultUserConfig;
extern const BDSP_Raaga_Audio_OpusDecConfigParams BDSP_sOpusDecDefaultUserConfig;
extern const BDSP_Raaga_Audio_ALSDecConfigParams  BDSP_sALSDecDefaultUserConfig;
extern const BDSP_Raaga_Audio_AC4DecConfigParams  BDSP_sAC4DecDefaultUserConfig;
extern const BDSP_Raaga_Audio_MlpConfigParams BDSP_sMlpUserConfig;
extern const BDSP_Raaga_Audio_PassthruConfigParams   BDSP_sDefaultPassthruSettings;
extern const BDSP_Raaga_Audio_AVLConfigParams BDSP_sDefAVLConfigSettings;
extern const BDSP_Raaga_Audio_TruVolumeUserConfig BDSP_sDefSrsTruVolumeUserConfig;
extern const BDSP_Raaga_Audio_DDReencodeConfigParams BDSP_sDefDDReencodeUserConfig;
extern const BDSP_Raaga_Audio_DV258ConfigParams BDSP_sDefDV258UserConfig;
extern const BDSP_Raaga_Audio_DpcmrConfigParams BDSP_sDefDpcmrUserConfig;
extern const BDSP_Raaga_Audio_SRCUserConfigParams   BDSP_sDefaultSrcSettings;
extern const BDSP_Raaga_Audio_CustomVoiceConfigParams BDSP_sDefCustomVoiceConfigSettings;
extern const BDSP_Raaga_Audio_GenCdbItbConfigParams BDSP_sDefGenCdbItbConfigSettings;
extern const BDSP_Raaga_Audio_BtscEncoderConfigParams BDSP_sDefBtscEncoderConfigSettings;
extern const BDSP_Raaga_Audio_DtsBroadcastEncConfigParams BDSP_sDefDTSENCConfigSettings;
extern const BDSP_Raaga_Audio_AacheEncConfigParams BDSP_sDefAacHeENCConfigSettings;
extern const BDSP_Raaga_Audio_DDTranscodeConfigParams BDSP_sDefDDTranscodeConfigSettings;
extern const BDSP_Raaga_Audio_G711_G726EncConfigParams BDSP_sDefG711G726EncConfigSettings;
extern const BDSP_Raaga_Audio_G729EncoderUserConfig BDSP_sDefG729EncConfigSettings;
extern const BDSP_Raaga_Audio_Mpeg1L3EncConfigParams   BDSP_sDefMpeg1L3EncConfigSettings;
extern const BDSP_Raaga_Audio_Brcm3DSurroundConfigParams   BDSP_sDefBrcm3DSurroundConfigSettings;
extern const BDSP_Raaga_Audio_MixerConfigParams  BDSP_sDefFwMixerConfigSettings;
extern const BDSP_Raaga_Audio_DsolaConfigParams BDSP_sDefDsolaConfigSettings;
extern const BDSP_Raaga_Audio_FadeCtrlConfigParams BDSP_sDefFadeCtrlConfigSettings;
extern const BDSP_Raaga_Audio_AmbisonicsConfigParams BDSP_sDefAmbisonicsConfigSettings;
extern const BDSP_Raaga_Audio_TsmCorrectionConfigParams BDSP_sDefTsmCorrectionConfigSettings;
extern const BDSP_Raaga_Audio_TruSurrndHDConfigParams BDSP_sDefTruSurrndHDConfigSettings;
extern const BDSP_Raaga_Audio_G723_1DEC_ConfigParams BDSP_sG723_1_Configsettings;
extern const BDSP_Raaga_Audio_G723EncoderUserConfig BDSP_sDefG723_1EncodeConfigSettings;
extern const BDSP_Raaga_VideoBH264UserConfig BDSP_sBH264EncodeUserConfigSettings;
extern const BDSP_Raaga_VideoBX264UserConfig BDSP_sBX264EncodeUserConfigSettings;
extern const BDSP_Raaga_VideoBXVP8UserConfig BDSP_sBXVP8EncodeUserConfigSettings;
extern const BDSP_Raaga_Audio_Broadcom3DSurroundConfigParams BDSP_sBroadcom3DSurroundConfigSettings;
extern const BDSP_Raaga_Audio_SpeexAECConfigParams  BDSP_sDefSpeexAECConfigParams;
extern const BDSP_Raaga_Audio_KaraokeConfigParams   BDSP_sDefKaraokeConfigSettings;
extern const BDSP_Raaga_Audio_OutputFormatterConfigParams   BDSP_sDefOutputFormatterConfigSettings;
extern const BDSP_Raaga_Audio_MixerDapv2ConfigParams    BDSP_sDefMixerDapv2ConfigParams;
extern const BDSP_Raaga_Audio_VocalPPConfigParams   BDSP_sDefVocalPPConfigSettings;
extern const BDSP_Raaga_Audio_G722EncConfigParams BDSP_sDefG722EncConfigSettings;
extern const BDSP_Raaga_Audio_AmrEncoderUserConfig BDSP_sDefAmrEncConfigSettings;
extern const BDSP_Raaga_Audio_AMRWBConfigParams BDSP_sDefAmrwbEncConfigSettings;
extern const BDSP_Raaga_Audio_ILBCConfigParams BDSP_sDefiLBCEncConfigSettings;
extern const BDSP_Raaga_Audio_ISACConfigParams  BDSP_sDefiSACEncConfigSettings;
extern const BDSP_Raaga_Audio_LpcmEncConfigParams BDSP_sDefLpcmEncConfigSettings;
extern const BDSP_Raaga_Audio_OpusEncConfigParams BDSP_sDefOpusEncConfigSettings;
extern const BDSP_Raaga_Audio_DDPEncConfigParams BDSP_sDefDDPEncConfigSettings;
extern const BDSP_VideoEncodeTaskDatasyncSettings BDSP_sDefaultVideoEncodeFrameSyncSettings;
extern const BDSP_Raaga_VideoBH264UserConfig BDSP_sBH264EncodeUserConfigSettings;
extern const BDSP_Raaga_VideoBX264UserConfig BDSP_sBX264EncodeUserConfigSettings;
#endif
