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

#include "bdsp_common_priv_include.h"

const BDSP_AudioTaskDatasyncSettings BDSP_sDefaultFrameSyncSettings =
{                                       /* sFrameSyncConfigParams */
    0,
    BDSP_Audio_AudioInputSource_eExtI2s0,
    {
        48000
    },
        1,
        {{
        BDSP_Audio_ASFPTSType_eInterpolated,
            BDSP_Audio_WMAIpType_eASF
        }},
    0,                                           /* eForceCompleteFirstFrame */
    BDSP_Raaga_Audio_DatasyncType_eNone
};
const BDSP_AudioTaskTsmSettings BDSP_sDefaultTSMSettings =
{                                       /* sTsmConfigParams */
        90,                                 /* i32TSMSmoothThreshold */
        0,                                  /* i32TSMSyncLimitThreshold */
        360,                                /* i32TSMGrossThreshold */
        135000,                             /* i32TSMDiscardThreshold */
        0,                                  /* i32TsmTransitionThreshold */
#ifdef BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE
        BDSP_REGSET_PHY_ADDR_FOR_DSP( BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE ),/* ui32STCAddr */
#else
        BDSP_REGSET_PHY_ADDR_FOR_DSP( BCHP_AUD_MISC_STC_UPPERi_ARRAY_BASE ),/* ui32STCAddr */
#endif
        0,                                  /* ui32AVOffset */
        0,                                  /* ui32SwSTCOffset */
        5760,                               /* ui32AudioOffset */
        true,                				/* eEnableTSMErrorRecovery */
        false,                				/* eSTCValid */
        true,                 				/* ePlayBackOn */
        false,                				/* eTsmEnable */
        true,                 				/* eTsmLogEnable */
        false                				/* eASTMEnable */
};

const BDSP_Raaga_Audio_MpegConfigParams  BDSP_sMpegDefaultUserConfig =
{
    0x0,                                    /*eDecoderType*/
    2,                                      /*ui32OutMode*/
    0x7FFFFFFF,                             /*ui32LeftChannelGain*/
    0x7FFFFFFF,                             /*ui32RightChannelGain*/
    2,                                      /*ui32DualMonoMode*/
    -23,                                    /* Input Volume Level */
    -23,                                    /* Output Volume Level */
    0,                                      /* Anc Data Parsing Enable */
    {0, { 0,0,0,0,0}},                                      /* Anc Data BDSP_AF_P_sSINGLE_CIRC_BUFFER structure */
    {0, 1,0xFFFFFFFF,0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF,0xFFFFFFFF},                 /* ui32OutputChannelMatrix */
    true                         			/* Boolean MonotoStereoDownscale*/
};

const BDSP_Raaga_Audio_UdcdecConfigParams BDSP_sUdcdecDefaultUserConfig =
{
    0,                      /*eDolbyMsUsageMode*/
    0,                      /*eDecoderType*/
    1,                      /*i32NumOutPorts*/
    {
    {
        2,                  /*i32CompMode*/
        0x64,                   /*i32PcmScale*/
        0x64,               /*i32DynScaleHigh*/
        0x64,               /*i32DynScaleLow*/
        0,                  /*i32OutLfe*/
        1,                  /*i32decorr_mode*/
        2,                  /*i32OutMode*/
        0,                  /*i32StereoMode*/
        0,                  /*i32DualMode*/
        3,                  /*i32Kmode*/
        0,                  /*i32MdctBandLimitEnable*/
        0,                  /*i32ExtDnmixEnabled*/
        {
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0}
        },
        0,                  /*i32ExtKaraokeEnabled*/
        0,                  /*i32Extv1Level*/
        0,                  /*i32Extv1Pan*/
        0,                  /*i32Extv2Level*/
        0,                  /*i32Extv2Pan*/
        0,                  /*i32ExtGmLevel*/
        0,                  /*i32ExtGmPan*/
        0,                  /*i32FadeOutputMode*/
        { 0,
          1,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF
        }
    },
    {
        2,                  /*i32CompMode*/
        0x64,                   /*i32PcmScale*/
        0x64,               /*i32DynScaleHigh*/
        0x64,               /*i32DynScaleLow*/
        0,                  /*i32OutLfe*/
        1,                  /*i32decorr_mode*/
        2,                  /*i32OutMode*/
        0,                  /*i32StereoMode*/
        0,                  /*i32DualMode*/
        3,                  /*i32Kmode*/
        0,                  /*i32MdctBandLimitEnable*/
        0,                  /*i32ExtDnmixEnabled*/
        {
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0}
        },
        0,                  /*i32ExtKaraokeEnabled*/
        0,                  /*i32Extv1Level*/
        0,                  /*i32Extv1Pan*/
        0,                  /*i32Extv2Level*/
        0,                  /*i32Extv2Pan*/
        0,                  /*i32ExtGmLevel*/
        0,                  /*i32ExtGmPan*/
        0,                  /*i32FadeOutputMode*/
        { 0,
          1,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF
        }
    },
    {
        2,                  /*i32CompMode*/
        0x64,                   /*i32PcmScale*/
        0x64,               /*i32DynScaleHigh*/
        0x64,               /*i32DynScaleLow*/
        0,                  /*i32OutLfe*/
        1,                  /*i32decorr_mode*/
        2,                  /*i32OutMode*/
        0,                  /*i32StereoMode*/
        0,                  /*i32DualMode*/
        3,                  /*i32Kmode*/
        0,                  /*i32MdctBandLimitEnable*/
        0,                  /*i32ExtDnmixEnabled*/
        {
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0},
            {0,0,0,0,0,0}
        },
        0,                  /*i32ExtKaraokeEnabled*/
        0,                  /*i32Extv1Level*/
        0,                  /*i32Extv1Pan*/
        0,                  /*i32Extv2Level*/
        0,                  /*i32Extv2Pan*/
        0,                  /*i32ExtGmLevel*/
        0,                  /*i32ExtGmPan*/
        0,                  /*i32FadeOutputMode*/
        { 0,
          1,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF,
          0xFFFFFFFF
        }
    }},
    1,                      /*i32StreamDialNormEnable*/
    0,                      /*i32UserDialNormVal*/
    0,                      /*ui32SubstreamIDToDecode*/
    0,                      /*ui32mdatflags*/
    0,                      /*ui32EvolutionFileOut*/
    0,                      /*ui32EnableDynamicUserParams*/
    0,                       /*ui32EnableAtmosMetadata*/
	0,						/*disable_associated_decorr */
	0,						/*disable_associated_evolution */
	0,						/*dec_errorconcealflag */
	0,						/*dec_errorconcealtype */
	0,						/*cnv_errorconcealflag */
	0,						/*evohashflag */
	0,						/*is_evolution_quickaccess */
	0,						/*evoquickaccess_substreamid */
	0,						/*evoquickaccess_strmtype */
	0						/*joc_force_downmix */
};

const BDSP_Raaga_Audio_AacheConfigParams BDSP_sAacheDefaultUserConfig =
{
    1,                                      /* i32NumOutPorts*/
    0,                                      /* DownmixType */
    0x0,                                    /* i32AribMatrixMixdownIndex */
    0x40000000,                             /* ui16DrcGainControlCompress */
    0x40000000,                             /* ui16DrcGainControlBoost */
    127,                                    /* ui16DrcTargetLevel */
    {{                                      /* sUserOutputCfg[0] */
        false,                              /* i32OutLfe */
        2,                                  /* i32OutMode */
        0,                                  /* i32DualMode */
        false,                              /* i32ExtDnmixEnabled */
         {{ 0, 0, 0, 0, 0, 0},              /* i32ExtDnmixTab[0] */
            { 0, 0, 0, 0, 0, 0},            /* i32ExtDnmixTab[1] */
            { 0, 0, 0, 0, 0, 0},            /* i32ExtDnmixTab[2] */
            { 0, 0, 0, 0, 0, 0},            /* i32ExtDnmixTab[3] */
            { 0, 0, 0, 0, 0, 0},            /* i32ExtDnmixTab[4] */
            { 0, 0, 0, 0, 0, 0}},           /* i32ExtDnmixTab[5] */
         {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} /* ui32OutputChannelMatrix */
    },
    {                                       /* sUserOutputCfg[1] */
        false,                              /* i32OutLfe */
        2,                                  /* i32OutMode */
        0,                                  /* i32DualMode */
        false,                              /* i32ExtDnmixEnabled */
         {{ 0, 0, 0, 0, 0, 0},              /* i32ExtDnmixTab[0] */
            { 0, 0, 0, 0, 0, 0},            /* i32ExtDnmixTab[1] */
            { 0, 0, 0, 0, 0, 0},            /* i32ExtDnmixTab[2] */
            { 0, 0, 0, 0, 0, 0},            /* i32ExtDnmixTab[3] */
            { 0, 0, 0, 0, 0, 0},            /* i32ExtDnmixTab[4] */
            { 0, 0, 0, 0, 0, 0}},           /* i32ExtDnmixTab[5] */
         {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} /* ui32OutputChannelMatrix */
    }},
    0,                                      /* i32PcmBoost6dB */
    0,                                      /* i32PcmBoostMinus4p75dB */
    0,                                      /* ui32FreqExtensionEnable */
    1,                                      /* ui32SbrUserFlag */
    -32,                                    /* Input Volume Level */
    -32,                                    /* Output Volume Level */
    50,                                     /* ui32DownmixCoefScaleIndex */
    0,                                      /* ui32LoudnessEquivalenceMode */
    0                                       /* eDecoderType */
};

const BDSP_Raaga_Audio_AC4DecConfigParams BDSP_sAC4DecDefaultUserConfig =
{
    0,                              /*eDolbyMsUsageMode*/
    0,                              /*eDecoderType*/
    1,                              /*i32NumOutPorts*/
    {
        {
            6,                      /*ui32ChannelConfig*/
            0,                    /*i32MainAssocMixPref*/
            0,                      /*ui32Phase90Preference*/
            0,                      /*ui32DialogEnhGainInput*/
            -31,                    /*i32TargetRefLevel*/
            0,                      /*ui32DrcEnable*/
            0,                      /* ui32DrcMode */
            0,                    /*ui32IeqStrength*/
            0,                      /*ui32IeqProfile*/
            { 0,
              1,
              2,
              3,
              4,
              5,
              6,
              7
	    },
	      0xFFFFFFFF,{0},{0},0,0,{0},1
        },
        {
            2,                      /*ui32ChannelConfig*/
            0,                    /*i32MainAssocMixPref*/
            0,                      /*ui32Phase90Preference*/
            0,                      /*ui32DialogEnhGainInput*/
            -31,                    /*i32TargetRefLevel*/
            0,                      /*ui32DrcEnable*/
            0,                      /* ui32DrcMode */
            0,                    /*ui32IeqStrength*/
            0,                      /*ui32IeqProfile*/
            { 0,
              1,
              2,
              3,
              4,
              5,
              6,
              7
	    },
	      0xFFFFFFFF,{0},{0},0,0,{0},1
        },
	{
            2,                      /*ui32ChannelConfig*/
            0,                    /*i32MainAssocMixPref*/
            0,                      /*ui32Phase90Preference*/
            0,                      /*ui32DialogEnhGainInput*/
            -31,                    /*i32TargetRefLevel*/
            0,                      /*ui32DrcEnable*/
            0,                      /* ui32DrcMode */
            0,                    /*ui32IeqStrength*/
            0,                      /*ui32IeqProfile*/
            { 0,
              1,
              2,
              3,
              4,
              5,
              6,
              7
	    },
	      0xFFFFFFFF,{0},{0},0,0,{0},1
        }
    },
    3,      /* ui32InputCplxLevel */
    0,      /* ui32MainAssocDec */
    1,      /* ui32OutputCplxLevel */
    1,      /* ui32SingleInstance */
    0,      /* ui32SamplingRateMode */
    0,      /* ui32DapEnable */
    1,      /* ui32LimiterEnable */
    1,      /* ui32CertificationMode */
    90000,  /* ui32TimeScale */

    0,      /* ui32AC4DecodeMode */
    1,      /* ui32EnableADMixing */
	-1      /* i32StreamInfoPresentationNumber */
};

const BDSP_Raaga_Audio_WmaConfigParams  BDSP_sWmaDefaultUserConfig =
{
    /*-------------------------sWmaStdConfigParam-------------------------*/
    1,                                      /* i32NumOutPorts */
    0,                                      /* uidecodeOnlyPatternFlag */
    0,                                      /* uidecodePattern */
    {0, 1, 0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} /* ui32OutputChannelMatrix */
};

const BDSP_Raaga_Audio_WmaProConfigParams  BDSP_sWmaProDefaultUserConfig =
{
    /*-------------------------sWmaProConfigParam-------------------------*/
    2,                                      /* ui32NumOutports */
    {{                                      /* sOutputCfg[0] */
        0,                                  /* ui32DRCEnable */
        0,/* eDRCSetting */
        0X7FFFFFFF,                         /* i32RmsAmplitudeRef */
        0X7FFFFFFF,                         /* i32PeakAmplitudeRef */
        0X7FFFFFFF,                         /* i32DesiredRms */
        0X7FFFFFFF,                         /* i32DesiredPeak */
        2,                                  /* ui32OutMode */
        false,                              /* ui32OutLfe */
        0,    /* ui32StereoMode */
    {0, 1, 0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} /* ui32OutputChannelMatrix */
    },
    {                                       /* sOutputCfg[1] */
        0,                                  /* ui32DRCEnable */
        0,/* eDRCSetting */
        0X7FFFFFFF,                         /* i32RmsAmplitudeRef */
        0X7FFFFFFF,                         /* i32PeakAmplitudeRef */
        0X7FFFFFFF,                         /* i32DesiredRms */
        0X7FFFFFFF,                         /* i32DesiredPeak */
        2,                                  /* ui32OutMode */
        false,                              /* ui32OutLfe */
        0,    /* ui32StereoMode */
    {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} /* ui32OutputChannelMatrix */
    }},
    0
};

const BDSP_Raaga_Audio_DtsHdConfigParams  BDSP_sDtsHdDefaultUserConfig =
{
    /*-------------------------sDtsHdConfigParam--------------------------*/
    true,                                   /* ui32DecodeCoreOnly */
    false,                                  /* ui32DecodeDtsOnly */
    true,                                   /* ui32DecodeXLL */
    true,                                   /* ui32DecodeX96 */
    true,                                   /* ui32DecodeXCH */
    true,                                   /* ui32DecodeXXCH */
    true,                                   /* ui32DecodeXBR */
    false,                                  /* ui32EnableSpkrRemapping */
    2123,                                   /* ui32SpkrOut */
    false,                                  /* ui32MixLFE2Primary */
    0,                                      /* ui32ChReplacementSet */
    2,                                      /* i32NumOutPorts */
    false,                                  /* ui32EnableMetadataProcessing */
    {{                                      /* sUserOutputCfg[0] */
        false,                              /* i32UserDRCFlag */
        0x7fffffff,                         /* i32DynScaleHigh */
        0x7fffffff,                         /* i32DynScaleLow */
        2,                                  /* ui32OutMode */
        false,                              /* ui32OutLfe */
        2,                                  /* ui32DualMode */
        0,                                  /* ui32StereoMode */
        48000,                              /* ui32AppSampleRate */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
        0,                                  /* ui32ExtdnmixEnabled */
        {{ 0, 0, 0, 0, 0, 0},               /* i32ExtDnmixTab[0] */
    { 0, 0, 0, 0, 0, 0},                    /* i32ExtDnmixTab[1] */
    { 0, 0, 0, 0, 0, 0},                    /* i32ExtDnmixTab[2] */
    { 0, 0, 0, 0, 0, 0},                    /* i32ExtDnmixTab[3] */
    { 0, 0, 0, 0, 0, 0},                    /* i32ExtDnmixTab[4] */
    { 0, 0, 0, 0, 0, 0}}                    /* i32ExtDnmixTab[5] */
    },
    {                                       /* sUserOutputCfg[1 ]*/
        false,                              /* i32UserDRCFlag */
        0x7fffffff,                         /* i32DynScaleHigh */
        0x7fffffff,                         /* i32DynScaleLow */
        2,                                  /* ui32OutMode */
        false,                              /* ui32OutLfe */
        2,                                  /* ui32DualMode */
        0,                                  /* ui32StereoMode */
        48000,                              /* ui32AppSampleRate */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
        0,                                  /* ui32ExtdnmixEnabled*/
        {{ 0, 0, 0, 0, 0, 0},               /* i32ExtDnmixTab[0] */
    { 0, 0, 0, 0, 0, 0},                    /* i32ExtDnmixTab[1] */
    { 0, 0, 0, 0, 0, 0},                    /* i32ExtDnmixTab[2] */
    { 0, 0, 0, 0, 0, 0},                    /* i32ExtDnmixTab[3] */
    { 0, 0, 0, 0, 0, 0},                    /* i32ExtDnmixTab[4] */
    { 0, 0, 0, 0, 0, 0}}                    /* i32ExtDnmixTab[5] */
    } }
    ,
    0
};

const BDSP_Raaga_Audio_DtslbrConfigParams  BDSP_sDtsLbrDefaultUserConfig =
{
     1,                                                    /*Default: 1, Range [0,1] , 0 -> primary, 1-> secondary*/

     0,                                   /*Default: 0, Range [0,1] , 0 -> disabled, 1-> decode embedded stereo if present*/

     0,                                     /*default: 0, range: [0,1], [0, 1] = disableing/enabling Embedded downmix*/

     0xFFFFFFFF,                               /*Dram address for communicating Mixer metadata between Primary, Secondary and Mixer*/

     1,                                                                                             /* Default =1; Range : 1,2 (2-> enables PCM output and concurrent stereo)*/

     0,              /* Default =0; 1-> If Mixing is enabled */                       /* Default =0; 1-> DVD */

     0xFFFFFFFF,                 /* When the decoder is configured for primary decoder or if there is no active primary decoder, the value of this variable will be invalid i.e. 0xFFFFFFFF ; If it is called in secondary mode then this variable will carry outmode of primary */

    {
        {
            1,                                   /*  Default : 1     Range : [0,1] (0-> Dialnorm disabled; 1-> Dialnorm enabled)
                                                This Flag is used to turn on/off Dialnorm */

            0,                                                                                /*  Default =0    Range : [0,1] (0-> DRC disabled; 1-> DRC enabled)
                                                This Flag is used to turn on/off dynamic range compression */

            0x7fffffff,                                                                        /*  Default = 0x7fffffff (100 %)
                                                Range 0 to 0x7fffffff (0 to 100%) */

            2,                                                                  /*Default =7;
                                                Output channel configuration, this is according to BCOM_eACMOD */

            1,                                                                                        /* Default = 1
                                                   Range : 0,1 (0-> LFE disabled;1-> LFE enabled)
                                                   Flag used to enable/disable LFE channel output */

            2,                                                                                 /* Default =2
                                                   Range : 0-3 (0-> DUALLEFT_MONO; 1->DUALRIGHT_MONO;2->STEREO;3->DUAL MIX MONO)
                                                   Configure decoder output for dual mode */

            0,                                                                            /*  Default=0
                                                    Range : 0,1 (1->Lt/Rt downmix;0->Normal output)
                                                    Perform stereo downmix of decoder output */

            0,                                                      /*Default =0; Range : 0,1 (1-> Mix LFE to primary while downmixing, when Lfe output is disabled)*/

            0,                      /*Default =0; Range : 0,1 (1-> enable speaker remapping , 1-> disable speaker remapping )*/

            15,                                                                      /*[required output speaker configurations, default 15 , can take value from the following values*/
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


    {0, 1, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff},
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

        },
        {
            1,                                   /*  Default : 1     Range : [0,1] (0-> Dialnorm disabled; 1-> Dialnorm enabled)
                                                This Flag is used to turn on/off Dialnorm */

            0,                                                                                /*  Default =0    Range : [0,1] (0-> DRC disabled; 1-> DRC enabled)
                                                This Flag is used to turn on/off dynamic range compression */

            0x7fffffff,                                                                        /*  Default = 0x7fffffff (100 %)
                                                Range 0 to 0x7fffffff (0 to 100%) */

            7,                                                                  /*Default =7;
                                                Output channel configuration, this is according to BCOM_eACMOD */

            1,                                                                                        /* Default = 1
                                                   Range : 0,1 (0-> LFE disabled;1-> LFE enabled)
                                                   Flag used to enable/disable LFE channel output */

            2,                                                                                 /* Default =2
                                                   Range : 0-3 (0-> DUALLEFT_MONO; 1->DUALRIGHT_MONO;2->STEREO;3->DUAL MIX MONO)
                                                   Configure decoder output for dual mode */

            0,                                                                            /*  Default=0
                                                    Range : 0,1 (1->Lt/Rt downmix;0->Normal output)
                                                    Perform stereo downmix of decoder output */

            0,                                                      /*Default =0; Range : 0,1 (1-> Mix LFE to primary while downmixing, when Lfe output is disabled)*/

            0,                      /*Default =0; Range : 0,1 (1-> enable speaker remapping , 1-> disable speaker remapping )*/

            15,                                                                      /*[required output speaker configurations, default 15 , can take value from the following values*/
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


    {0, 1, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff},
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

        }
    }

};

const BDSP_Raaga_Audio_AmrConfigParams BDSP_sAmrDefaultUserConfig =
{
    1,                                  /*ui32NumOutPorts*/
    {{/*sUserOutputCfg[0]*/
        2,                              /* ui32OutMode */
        1,                              /* ui32ScaleOp */
        0,                              /* ui32ScaleIdx */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*ui32OutputChannelMatrix*/
    },
    {/*sUserOutputCfg[1]*/
        1,                              /* ui32OutMode */
        0,                              /* ui32ScaleOp */
        0,                              /* ui32ScaleIdx */
        {0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*ui32OutputChannelMatrix*/
    }}
};

const BDSP_Raaga_Audio_AmrwbdecConfigParams BDSP_sAmrwbdecDefaultUserConfig =
{
    1,                                  /*ui32NumOutPorts*/
    {{/*sUserOutputCfg[0]*/
        2,                              /* ui32OutMode */
        1,                              /* ui32ScaleOp */
        0,                              /* ui32ScaleIdx */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*ui32OutputChannelMatrix*/
    },
    {/*sUserOutputCfg[1]*/
        1,                              /* ui32OutMode */
        0,                              /* ui32ScaleOp */
        0,                              /* ui32ScaleIdx */
        {0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*ui32OutputChannelMatrix*/
    }}
};

const BDSP_Raaga_Audio_DraConfigParams  BDSP_sDraDefaultUserConfig =
{
    2,                                      /* ui32NumOutPorts */
    {{
        2,                                  /* ui32OutMode */
        false,                              /* ui32OutLfe */
        0,                                  /* ui32StereoMode */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
    },
    {
        2,                                  /* ui32OutMode */
        false,                              /* ui32OutLfe */
        0,                                  /* ui32StereoMode */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
    }}
};

const BDSP_Raaga_Audio_RalbrConfigParams  BDSP_sRalbrDefaultUserConfig =
{
    2,                                      /* ui32NumOutPorts */
    {{
        2,                                  /* ui32OutMode */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
    },
    {
        2,                                  /* ui32OutMode */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
    }}
};

const BDSP_Raaga_Audio_PcmWavConfigParams  BDSP_sPcmWavDefaultUserConfig =
{
    1,              /* ui32NumOutputPorts */
    0,              /* eDecoderType */
    {
        {
            2,      /* i32OutMode */
            0,      /* i32StereoMode */
            0,      /* i32DualMode */
            {0x2902de00, 0x2902de00, 0x1CFDF3B6, 0x1CFDF3B6, 0x1CFDF3B6, 0x1CFDF3B6, 0x5A827999, 0x5A827999},/* i32DownmixCoeffs */
            {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}                   /* ui32OutputChannelMatrix */
        },
        {
            2,      /* i32OutMode */
            0,      /* i32StereoMode */
            0,      /* i32DualMode */
            {0x2902de00, 0x2902de00, 0x1CFDF3B6, 0x1CFDF3B6, 0x1CFDF3B6, 0x1CFDF3B6, 0x5A827999, 0x5A827999},/* i32DownmixCoeffs */
            {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}                   /* ui32OutputChannelMatrix */
        }
    }
};

const BDSP_Raaga_Audio_G726ConfigParams BDSP_sG711G726DecUserConfig =
{
    1,                                          /* ui32NumOutPorts */
    0,                                          /* ui32OutputType */
    {                                           /* sUsrOutputCfg */
        {                                       /* sUsrOutputCfg[0] */
            2,                                  /* ui32OutMode */
            1,                                  /* ui32ApplyGain */
            0x5A827999,                         /* i32GainFactor */
            {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
        },
        {                                       /* sUsrOutputCfg[1] */
            1,                                  /* ui32OutMode */
            0,                                  /* ui32ApplyGain */
            0x5A827999,                         /* i32GainFactor */
            {4, 4, 0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
        }
    }
};

const BDSP_Raaga_Audio_G723_1DEC_ConfigParams BDSP_sG723_1_Configsettings =
{
    1,                                          /* ui32NumOutPorts */
    {                                           /* sUsrOutputCfg[2] */
        {                                       /* sUsrOutputCfg[0] */
            2,                                  /* ui32OutMode */
            1,                                  /* ui32ScaleOp */
            0,                                  /* ui32ScaleIdx */
            {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix[8] */
        },
        {                                       /* sUsrOutputCfg[1] */
            1,                                  /* ui32OutMode */
            0,                                  /* ui32ScaleOp */
            0,                                  /* ui32ScaleIdx */
            {4, 4,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix[8] */
        }
    }
};

const BDSP_Raaga_Audio_G729DecConfigParams BDSP_sG729DecUserConfig =
{
    1,                                          /* ui32NumOutPorts */
    {                                           /* sUsrOutputCfg */
        {                                       /* sUsrOutputCfg[0] */
            2,                                  /* ui32OutMode */
            1,                                  /* ui32ScaleOp */
            0,                                  /* ui32ScaleIdx */
            {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
        },
        {                                       /* sUsrOutputCfg[1] */
            1,                                  /* ui32OutMode */
            0,                                  /* ui32ScaleOp */
            0,                                  /* ui32ScaleIdx */
            {4, 4,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
        },
    }
};

const BDSP_Raaga_Audio_AdpcmConfigParams  BDSP_sAdpcmDefaultUserConfig =
{
    2,                                      /* ui32NumOutPorts */
    {{
        2,                                  /* ui32OutMode */
        0,                                  /* ui32ApplyGain */
        0x016A09E6,                         /* i32GainFactor */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
    },
    {
        2,                                  /* ui32OutMode */
        0,                                  /* ui32ApplyGain */
        0x016A09E6,                         /* i32GainFactor */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
    }}
};

const BDSP_Raaga_Audio_LpcmUserConfig  BDSP_sLcpmDvdDefaultUserConfig =
{
    1,                                      /* ui32NumOutports */
    {{                                      /* sOutputCfg[0] */
        2,                                  /* ui32OutMode */
        false,                              /* ui32LfeOnFlag */
        0,                                  /* ui32DualMonoMode */
    {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} /* ui32OutputChannelMatrix */
    },
    {                                       /* sOutputCfg[0] */
        2,                                  /* ui32OutMode */
        false,                              /* ui32LfeOnFlag */
        0,                                  /* ui32DualMonoMode */
    {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} /* ui32OutputChannelMatrix */
    }}
};

const BDSP_Raaga_Audio_FlacDecConfigParams BDSP_sFlacDecUserConfig =
{
    1,                                          /* ui32NumOutPorts */
    {                                           /* sUsrOutputCfg */
        {                                       /* sUsrOutputCfg[0] */
            2,                                  /* ui32OutMode */
            0,                                  /* ui32OutLfe */
            {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
        },
        {                                       /* sUsrOutputCfg[1] */
            2,                                  /* ui32OutMode */
            0,                                  /* ui32OutLfe */
            {0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},/* ui32OutputChannelMatrix */
        },
    }
};

const BDSP_Raaga_Audio_iLBCdecConfigParams BDSP_siLBCdecDefaultUserConfig =
{
    1,                                  /*ui32NumOutPorts*/
    {{/*sUserOutputCfg[0]*/
        20,                             /* frame lenghth*/
        0,                              /* plc*/
        2,                              /* ui32OutMode */
        0,                              /* ui32ScaleOp */
        0,                              /* ui32ScaleIdx */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*ui32OutputChannelMatrix*/
    },
    {/*sUserOutputCfg[1]*/
        20,                             /* frame length*/
        0,                              /* plc*/
        2,                              /* ui32OutMode */
        0,                              /* ui32ScaleOp */
        0,                              /* ui32ScaleIdx */
        {0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*ui32OutputChannelMatrix*/
    }}
};

const BDSP_Raaga_Audio_iSACdecConfigParams BDSP_siSACdecDefaultUserConfig =
{

    1,                                  /*ui32NumOutPorts*/
    {{/*sUserOutputCfg[0]*/
        0,                              /* uint32_t    ui32BandMode */
        0,                              /* uint32_t    plc */
        1,                              /* uint32_t    ui32OutMode */
        0,                              /* uint32_t    ui32ScaleOp */
        0,                              /* uint32_t    ui32ScaleIdx */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*ui32OutputChannelMatrix*/
    },
    {/*sUserOutputCfg[1]*/
        0,                              /* uint32_t    ui32BandMode */
        0,                              /* uint32_t    plc */
        1,                              /* uint32_t    ui32OutMode */
        0,                              /* uint32_t    ui32ScaleOp */
        0,                              /* uint32_t    ui32ScaleIdx */
        {0, 1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*ui32OutputChannelMatrix*/
    }}

};

const BDSP_Raaga_Audio_OpusDecConfigParams BDSP_sOpusDecDefaultUserConfig =
{
  1,
  {
    {
      2,
      {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}
    },

    {
      1,
      {0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}
    },
  },
  BDSP_AF_P_DecoderType_ePrimary
};

const BDSP_Raaga_Audio_ALSDecConfigParams BDSP_sALSDecDefaultUserConfig =
{
    2,    /* ui32NumOutPorts Default = 2; 1 -> Multichannel out, 2 -> Multichannel + Stereo out*/
    1,    /* i32DownmixType  Default = 1, 0 -> USER downmix, 1 -> ARIB downmix  2 -> LTRT downmix */
    1,    /* i32AribMatrixMixdownIndex Default = 1, ARIB matrix mixdown index which can take integer values from 1 to 3*/
    {
    {
    7,   /*  ui32OutMode = BCOM_eACMOD_MODE32 */
    {  /*  i32ExtDnmixTab[6][6] Default: Zeros */
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0}
    },
    {0, 1, 2, 3, 4, 5, 0xFFFFFFFF, 0xFFFFFFFF}    /* Multichannel = L, R, Ls, Rs, C, LFE */
    },

    {
    2,   /* ui32OutMode = BCOM_eACMOD_MODE20 */
    {   /*  i32ExtDnmixTab[6][6] */
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0}
    },
    {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}   /*  Stereo = L, R  */
    },
    }

};

const BDSP_Raaga_Audio_TruVolumeUserConfig BDSP_sDefSrsTruVolumeUserConfig =
{
    1,
    2,
    256,
    1,
    0x00200000,
    0x00200000,
    0x007fffff,
    0x00400000,
    0,
    1,
    0,
    0x00080000,
    1,
    0x000ccccd,
    0x8000,
    1,
    {                                           /* sTopLevelConfig */
        0,                                      /* i32IsStudioSound */
        1,                                      /* i32StudioSoundMode */
        1,                                      /* i32mEnable */
        0x7fffffff,                             /* i32mInputGain */
        0x7fffffff,                             /* i32mHeadroomGain */
        3,                                      /* i32mInputMode */
        0x7fffffff,                             /* i32mOutputGain */
        0x7fffffff,                             /* i32mBypassGain */
    },
    {                                           /* sHighPassFilterConfig */
        1,                                      /* ui32mEnable */
        0,                                      /* ui32CoefGenMode */
        {                                       /* sFilterCoefHpf[] */
            {                                   /* sFilterCoefHpf[0] */
                0x00000004,                     /* ui32Order */
                {                               /* sFilterCoefHpfTdf2[] */
                    {                           /* sFilterCoefHpfTdf2[0] */
                        0x00000001,             /* i32Scale */
                        0x0fc81f80,             /* i32FilterCoefficientB0 */
                        0xe06fc100,             /* i32FilterCoefficientB1 */
                        0x0fc81f80,             /* i32FilterCoefficientB1 */
                        0x7e36e680,             /* i32FilterCoefficientA1 */
                        0xc1b4eec0              /* i32FilterCoefficientA2 */
                    },
                    {                           /* sFilterCoefHpfTdf2[1] */
                        0x00000001,             /* i32Scale */
                        0x3df563c0,             /* i32FilterCoefficientB0 */
                        0x84153880,             /* i32FilterCoefficientB1 */
                        0x3df563c0,             /* i32FilterCoefficientB1 */
                        0x7be0e200,             /* i32FilterCoefficientA1 */
                        0xc40b5300              /* i32FilterCoefficientA2 */
                    },
                    {                           /* sFilterCoefHpfTdf2[2] */
                        0x00000000,             /* i32Scale */
                        0x00000000,             /* i32FilterCoefficientB0 */
                        0x00000000,             /* i32FilterCoefficientB1 */
                        0x00000000,             /* i32FilterCoefficientB1 */
                        0x00000000,             /* i32FilterCoefficientA1 */
                        0x00000000              /* i32FilterCoefficientA2 */
                    },
                }
            },
            {                                   /* sFilterCoefHpf[1] */
                0x00000004,                     /* ui32Order */
                {                               /* sFilterCoefHpfTdf2[] */
                    {                           /* sFilterCoefHpfTdf2[0] */
                        0x00000001,             /* i32Scale */
                        0x0fd78db0,             /* i32FilterCoefficientB0 */
                        0xe050e4a0,             /* i32FilterCoefficientB1 */
                        0x0fd78db0,             /* i32FilterCoefficientB1 */
                        0x7eb71980,             /* i32FilterCoefficientA1 */
                        0xc13e3e40              /* i32FilterCoefficientA2 */
                    },
                    {                           /* sFilterCoefHpfTdf2[1] */
                        0x00000001,             /* i32Scale */
                        0x3e826c40,             /* i32FilterCoefficientB0 */
                        0x82fb2780,             /* i32FilterCoefficientB1 */
                        0x3e826c40,             /* i32FilterCoefficientB1 */
                        0x7cff9680,             /* i32FilterCoefficientA1 */
                        0xc2f5e600              /* i32FilterCoefficientA2 */
                    },
                    {                           /* sFilterCoefHpfTdf2[2] */
                        0x00000000,             /* i32Scale */
                        0x00000000,             /* i32FilterCoefficientB0 */
                        0x00000000,             /* i32FilterCoefficientB1 */
                        0x00000000,             /* i32FilterCoefficientB1 */
                        0x00000000,             /* i32FilterCoefficientA1 */
                        0x00000000              /* i32FilterCoefficientA2 */
                    },
                }
            },
            {                                   /* sFilterCoefHpf[2] */
                0x00000004,                     /* ui32Order */
                {                               /* sFilterCoefHpfTdf2[] */
                    {                           /* sFilterCoefHpfTdf2[0] */
                        0x00000001,             /* i32Scale */
                        0x0fdadc10,             /* i32FilterCoefficientB0 */
                        0xe04a47e0,             /* i32FilterCoefficientB1 */
                        0x0fdadc10,             /* i32FilterCoefficientB1 */
                        0x7ed26000,             /* i32FilterCoefficientA1 */
                        0xc1249f40              /* i32FilterCoefficientA2 */
                    },
                    {                           /* sFilterCoefHpfTdf2[1] */
                        0x00000001,             /* i32Scale */
                        0x3ea0f4c0,             /* i32FilterCoefficientB0 */
                        0x82be1680,             /* i32FilterCoefficientB1 */
                        0x3ea0f4c0,             /* i32FilterCoefficientB1 */
                        0x7d3d7780,             /* i32FilterCoefficientA1 */
                        0xc2b9a440              /* i32FilterCoefficientA2 */
                    },
                    {                           /* sFilterCoefHpfTdf2[2] */
                        0x00000000,             /* i32Scale */
                        0x00000000,             /* i32FilterCoefficientB0 */
                        0x00000000,             /* i32FilterCoefficientB1 */
                        0x00000000,             /* i32FilterCoefficientB1 */
                        0x00000000,             /* i32FilterCoefficientA1 */
                        0x00000000              /* i32FilterCoefficientA2 */
                    },
                }
            }
        },
        {                                       /* sFilterSpecHpf */
            180,                                /* ui32CutoffFrequency */
            4                                   /* ui32Order */
        }
    }

};

const BDSP_Raaga_Audio_PassthruConfigParams   BDSP_sDefaultPassthruSettings =
{
    BDSP_Raaga_ePassthruType_SPDIF,          /* ui32PassthruType */
    BDSP_Raaga_eAacHeaderType_Raw,
	BDSP_AF_P_BurstFill_ePauseBurst,
	BDSP_AF_P_SpdifPauseWidth_eEightWord
};


const BDSP_Raaga_Audio_DDTranscodeConfigParams BDSP_sDefDDTranscodeConfigSettings =
{
    0,                                          /* i32AudCodMode*/
    0,                                          /* i32LoFreqEffOn */
    BDSP_AF_P_eEnable,                          /* eSpdifHeaderEnable */
    BDSP_AF_P_eEnable,                          /* eTranscodeEnable */
};

const BDSP_Raaga_Audio_Broadcom3DSurroundConfigParams   BDSP_sDefBrcm3DSurroundConfigSettings =
{
    1,                                          /* i32BRCM3DSurroundEnableFlag */
    1,                                          /* eSurroundMode */
    0                                           /* eBroadcom3DSurroundMode */
};

const BDSP_Raaga_Audio_DV258ConfigParams BDSP_sDefDV258UserConfig =
{
    1,                                          /*i32DolbyVolumeEnable*/
    0,                                          /*i32VlmMdlEnable*/
    0,                                          /*i32HalfmodeFlag*/
    1,                                          /*i32EnableMidsideProc*/
    1,                                          /*i32LvlEnable*/
    9,                                          /*i32LvlAmount*/
    1360,                                       /*i32InputReferenceLevel*/
    0,                                          /*i32Pregain*/
    1360,                                       /*i32OutputReferenceLevel*/
    0,                                          /*i32CalibrationOffset*/
    0,                                          /*i32AnalogVolumeLevel*/
    0,                                          /*i32DigitalVolumeLevel*/
    0,                                          /*i32ResetNowFlag*/
    1                                           /*i32LimiterEnable*/
};

const BDSP_Raaga_Audio_DDReencodeConfigParams BDSP_sDefDDReencodeUserConfig =
{
    0,                                          /* ui32ExternalPcmEnabled */
    1,                                          /* ui32CompProfile */
    0,                                          /* ui32CmixLev */
    0,                                          /* ui32SurmixLev */
    0,                                          /* ui32DsurMod */
    31,                                         /* ui32DialNorm */
    2,                                          /* ui32NumOutPorts */
    {                                           /* sUserOutputCfg[2] */
        {                                       /* sUserOutputCfg[1] */
            0,                                  /*ui32CompMode*/
            0x7FFFFFFF,                         /*ui32DrcCutFac*/
            0x7FFFFFFF,                         /*ui32DrcBoostFac*/
            2,                                  /*ui32OutMode*/
            0,                                  /*ui32OutLfe*/
            0,                                  /*ui32StereoMode*/
            0,                                  /*ui32DualMode*/
            {                                   /* ui32OutputChannelMatrix[8] */
                0,                              /* ui32OutputChannelMatrix[0] */
                1,                              /* ui32OutputChannelMatrix[1] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[2] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[3] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[4] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[5] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[6] */
                0xFFFFFFFF                      /* ui32OutputChannelMatrix[7] */
            }
        },
        {                                       /* sUserOutputCfg[1] */
            0,                                  /*ui32CompMode*/
            0x7FFFFFFF,                         /*ui32DrcCutFac*/
            0x7FFFFFFF,                         /*ui32DrcBoostFac*/
            2,                                  /*ui32OutMode*/
            0,                                  /*ui32OutLfe*/
            0,                                  /*ui32StereoMode*/
            0,                                  /*ui32DualMode*/
            {                                   /* ui32OutputChannelMatrix[8] */
                0,                              /* ui32OutputChannelMatrix[0] */
                1,                              /* ui32OutputChannelMatrix[1] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[2] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[3] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[4] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[5] */
                0xFFFFFFFF,                     /* ui32OutputChannelMatrix[6] */
                0xFFFFFFFF                      /* ui32OutputChannelMatrix[7] */
            }
        }
    }
};

const BDSP_Raaga_Audio_AVLConfigParams BDSP_sDefAVLConfigSettings =
{
    true,                   /* ui32AVLEnableFlag */
    -20*32768,              /* iTarget */
    -36*32768,              /* iLowerBound */
    15*32768,               /* uiFixedBoost */
    2959245,                /* uiRef */
    32736,                  /* uiAlpha */
    32256,                  /* uiBeta */
    9830,                   /* uiThreshold */
    500,                    /* uiDtfPcnt */
    32767,                  /* uiAlpha2 */
    16384,                  /* uisNsfgr */
    9830,                   /* uiDtf */
    2,                      /* i32LoudnessLevelEq */
};

const BDSP_Raaga_Audio_OutputFormatterConfigParams BDSP_sDefOutputFormatterConfigSettings =
{
        2, /* Number of channels */
        { 0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, }, /* Channel Layout for interleaving */
        16, /* Bits per Sample to interleave */
        BDSP_AF_P_Boolean_eTrue /* Signed or unsigned sample. 1 indicates signed */
};

const BDSP_Raaga_Audio_DsolaConfigParams BDSP_sDefDsolaConfigSettings =
{
    0x266                                       /* ui32InputPcmFrameSize */
};

const BDSP_Raaga_Audio_KaraokeConfigParams   BDSP_sDefKaraokeConfigSettings =
{
    3,   /* Level of voice suppression 0-5 */
    400, /* speech bins */
    3    /* scale outputs */
};

const BDSP_Raaga_Audio_ISACConfigParams BDSP_sDefiSACEncConfigSettings =
{
    30,                 /* uint16_t    ui16frameLen ;  */

    0,                  /* uint16_t    ui16CodingMode; */

    1,                  /* uint16_t    ui16nbTest;  */

    1,                  /* uint16_t    ui16fixedFL; */

    32000,              /* uint16_t    ui16rateBPS;  */

    32000,              /* uint16_t    ui16bottleneck;   */

    200,                /* uint16_t    ui16payloadSize;   */

    32000               /* uint16_t    ui16payloadRate;  */

};

const BDSP_Raaga_Audio_ILBCConfigParams BDSP_sDefiLBCEncConfigSettings =
{

    20                  /*uint32_t    ui16frameLen ;   */           /* range: 20 msec or 30 msec */

};

const BDSP_Raaga_Audio_G723EncoderUserConfig BDSP_sDefG723_1EncodeConfigSettings =
{
    0,                                          /* WrkRate */
    1,                                          /* UseHp */
    0,                                          /* UseVx */
};

const BDSP_Raaga_Audio_G729EncoderUserConfig BDSP_sDefG729EncConfigSettings =
{
    0,                                          /* ui32DtxEnable */
    0                                           /* ui32Bitrate */
};

const BDSP_Raaga_Audio_G711_G726EncConfigParams BDSP_sDefG711G726EncConfigSettings =
{
    BDSP_Raaga_Audio_eCompressionType_uLaw_disableG726,     /* eCompressionType */
    BDSP_Raaga_Audio_eBitRate_64kbps,                       /* eBitRate */
};

const BDSP_Raaga_Audio_Mpeg1L3EncConfigParams   BDSP_sDefMpeg1L3EncConfigSettings =
{
    BDSP_Raaga_Audio_Mp3EncodeBitRate_e128kbps,     /* eMp3EncodeBitRate */
    0,                                          /* ui32AddCRCProtect */
    BDSP_AF_P_eDisable,                             /* ePrivateBit */
    BDSP_AF_P_eDisable,                             /* eCpyright */
    BDSP_AF_P_eDisable,                             /* eOriginal */
    BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eNone,   /* eEmphasisType*/
    -23,   /* i32InputVolLevel */
    -23,   /* i32OutputVolLevel */
     0,    /* Flag that indicates to encode in stereo or mono Default=0 (stereo)*/
    BDSP_Raaga_Audio_Mp3EncodeMono_Mix /* Channel Selection when bEncodeMono flag is set */
};

const BDSP_Raaga_Audio_AacheEncConfigParams BDSP_sDefAacHeENCConfigSettings =
{
    BDSP_Raaga_Audio_AacEncodeBitRate_e48kbps,                                  /* eAacEncodeBitRate */
    false,                                                                      /* ui32bEncodeMono */
    true,                                                                       /* ui32EnableSBR */
    0,                                                                          /*Enables/Disables SRC from 48khz to 32khz before encoding. 1 SRC Enable, 0 No SRC */
    1,                                                                          /* ui32EncodeFormatType */
    -32,                                                                        /* Input Volume Level */
    -32,                                                                        /* Output Volume Level */
    BDSP_Raaga_Audio_AacEncodeComplexity_Low,                                   /* eAacEncodeComplexity */
     BDSP_Raaga_Audio_AacEncodeMono_Mix,                                        /* Channel select */
    BDSP_Raaga_Audio_AacEncodeAdtsMpeg4                                         /* ADTS ID field, MPEG Type */,

};

/* User Config Default values for Opus Encoder */
const BDSP_Raaga_Audio_OpusEncConfigParams BDSP_sDefOpusEncConfigSettings =
{
    64000,  /* uint32_t ui32BitRate,Encoder Bit Rate - Values 6kbps - 510kbps Default : 64kbps ( Bitrate sweetspot for 48k audio at 20ms ) */
    960,    /* uint32_t ui32FrameSize,Frame size -  Values 120-2880, Default : 960 (20 ms for 48k audio) */
    1,      /* uint32_t ui32EncodeMode,Encoder Mode - Values 0,1,2 : 0-SILK only,1-Hybrid,2-CELT only, Default : 1 - Hybrid */
    1,      /* uint32_t ui32VBREnabled,Variable Bit Rate Coding Enable - 0-CBR,1-VBR,2-CVBR, Default: 1 - VBR Encoding*/
    10      /* uint32_t ui32Complexity, Computational Complexity for the encoder - Values 0-10 in increasing order of complexity, Default: 10- Maximum complexity*/
};

const BDSP_Raaga_Audio_DtsBroadcastEncConfigParams BDSP_sDefDTSENCConfigSettings =
{
    1,                                          /* ui32SpdifHeaderEnable */
    0,                                          /* ui32CertificationEnableFlag */
    0,                                          /* ui32LFEEnableFlag */
    BDSP_Raaga_Audio_AcMode_eINVALID,           /* eInputDataAcMode */
    0                                           /* ui32IsIIRLFEInterpolation */
};

const BDSP_Raaga_Audio_SpeexAECConfigParams BDSP_sDefSpeexAECConfigParams =
{
    0,                          /* ui32GainResolution */
};

const BDSP_Raaga_Audio_BtscEncoderConfigParams BDSP_sDefBtscEncoderConfigSettings =
{
    BDSP_AF_P_eEnable,          /* eBTSCEnableFlag               - Default Enable*/
    BDSP_AF_P_eDisable,         /* eUseDeEmphasizedSourceSignals - Default Disable*/
    BDSP_AF_P_eEnable,          /* eMainChannelPreEmphasisOn     - Default Enable*/
    BDSP_AF_P_eDisable,         /* eEquivalenMode75MicroSec      - Default Disable*/
    BDSP_AF_P_eEnable,          /* eClipDiffChannelData          - Default Enable*/
    BDSP_AF_P_eEnable,          /* eDiffChannelPreEmphasisOn     - Default Enable*/
    BDSP_AF_P_eEnable,          /* eUseLowerThresholdSpGain      - Default Enable*/

    0x20000000,                 /*ui32AttenuationFactor          - Q-format 3.29 format: Maximum 0x7fffffff corresponding to 8.0 and default is 0x20000000:   */

    0x843BC1E,                  /*ui32SRDCalibrationFactor       - Q-format 4.28 format: Default :12BE76C9 for nextgen */
                                /* SRD changes from platform to platform*/
    BDSP_AF_P_eEnable,          /*Default Enable Used to Control FM Deviation of Sum Channel to 25Khz*/
    BDSP_AF_P_eEnable,          /*Default Enable Used to Control FM Deviation of Diff Channel to 50Khz*/
    BDSP_AF_P_eEnable,          /*Default Enable Used to Control FM Deviation of Final Output to 50Khz*/
    BDSP_AF_P_eDisable,         /*Default Disable. Used to Generate Test Tones*/
    0,                          /*Default 0 generates frequency sweep.  Programming to a fixed value generates tone*/
    0x20000000,                 /*i32SumChannelLevelCtrl: Q-format 3.29 format: Maximum is 0x7fffffff and default is : 0x20000000  */
    0x20000000,                 /*i32DiffChannelLevelCtrl: Q-format 3.29 format: Maximum is 0x7fffffff and default is : 0x20000000  */
    0,                          /*Mono Mode default disable*/
    0,                          /*eMuteRightChannel, Default Disable, Enable for Stereo Separation Tests*/
    {
            -529181, -2118401, 548225, 4014907, -3168182, -10020762, 10518989, 26513147, -49623988, -209780777, 1550493975,
            -209780777, -49623988, 26513147, 10518989, -10020762, -3168182, 4014907, 548225, -2118401, -529181
    }/*21 Tap Coeff for DAC Opamp Compensation filter */
};

const BDSP_Raaga_Audio_TsmCorrectionConfigParams BDSP_sDefTsmCorrectionConfigSettings =
{
    0                                       /* ui32TsmCorrectionMode */
};

const BDSP_Raaga_Audio_MixerDapv2ConfigParams  BDSP_sDefMixerDapv2ConfigParams =
{
    0,                  /*i32Certification_flag*/
    0,                  /*i32IsInputDownmixed*/
    0,                  /*i32LimiterEnable*/
    0,                  /*i32LenientMixingEnable*/
    0,                  /*i32MixerUserBalance*/
    {
        {0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF},
        {0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF},
        {0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF},
        {0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0FFFFFFF}           /*i32MixerVolumeControlGains*/
    },
    1,                  /*i32Ms12Flag*/
    0,                  /*i32EnableDapv2*/
    0,                  /*i32RampMode*/
    {
        {0x7FFFFFFF,0,0},
        {0x7FFFFFFF,0,0},
        {0x7FFFFFFF,0,0},
        {0x7FFFFFFF,0,0},
        {0x7FFFFFFF,0,0}

    }, /*BDSP_Fader_Config    sFadeControl[5]*/
    {
        3,              /*ui32Mode*/
        0,              /*ui32MiProcessDisable*/
        0,              /*ui32EnableIntelligentEqualizer*/
        10,             /*ui32IEamount*/
        {32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000,0,0,0,0,0,0,0,0,0,0},     /*ui32IEQFrequency[20]*/
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},                                      /*i32IEQInputGain[20]*/
        10,                             /*ui32IEQFrequency_count*/
        0,                              /*ui32EnableVolumeModeler*/
        0,                              /*i32VolModCalibration*/
        -496,                           /*i32VolLevelOutTarget*/
        -496,                           /*i32VolLevelInTarget*/
        7,                              /*ui32VolLevelInputValue*/
        0,                              /*ui32leveler_ignore_il*/
        96,                             /*ui32SurroundBoostLevel*/
        0,                              /*ui32EnableVolLeveler*/
        0,                              /*ui32VolMaxBoostLevel*/
        0,                              /*ui32EnableDialogEnhancer*/
        0,                              /*ui32DialogEnhancerLevel*/
        0,                              /*ui32DialogEnhancerDuckLevel*/
        0,                              /*ui32EnableSurroundDecoder*/
        0,                              /*ui32MiEnableSurrCompressorSteering*/
        0,                              /*ui32MiEnableDialogueEnhancerSteering*/
        0,                              /*ui32MiEnableVolumeLevelerSteering*/
        0,                              /*ui32MiEnableIEQSteering*/
        0,                              /*ui32CalibrationBoost*/
        0,                              /*i32SystemGain*/
        0,                              /*i32PostGain*/
        0,                              /*i32PreGain*/
        3,                              /*ui32OutputMode*/
        0,                              /*i32CmixlevQ14*/
        0,                              /*i32SurixlevQ14*/
        -1,                             /*i32VolumeLevelerWeight*/
        -1,                             /*i32IntelligentEqualizerWeight*/
        -1,                             /*i32DialogEnhancerSpeechConfidence*/
        -1,                             /*i32SurroundCompressorMusicConfidence*/
    },
};

const BDSP_Raaga_Audio_VocalPPConfigParams BDSP_sDefVocalPPConfigSettings =
{
       1, /* Enable echo addition */
      0x1999999A, /* Echo attenuation, 0.2 in Q31 format */
      100 /* Echo Delay in ms */
};

const BDSP_Raaga_Audio_GenCdbItbConfigParams BDSP_sDefGenCdbItbConfigSettings =
{
    BDSP_AF_P_eEnable,                          /* eEnableEncode */
    BDSP_AF_P_eDisable,                         /* eSnapshotRequired */
    0,                                          /* ui32EncSTCAddr */
    0,                                          /* ui32A2PInMilliSeconds */
    BDSP_AF_P_eDisable,                         /* eStcItbEntryEnable */
    0                                           /* ui32EncSTCAddrUpper */
};

const BDSP_Raaga_Audio_MixerConfigParams  BDSP_sDefFwMixerConfigSettings =
{
    3,                                          /* ui32NumInput */
    {
        {                                       /* MixingCoeffs [0][]*/
            0x10000000,                         /* MixingCoeffs [0][0]*/
            0x10000000,                         /* MixingCoeffs [0][1]*/
            0x10000000,                         /* MixingCoeffs [0][2]*/
            0x10000000,                         /* MixingCoeffs [0][3]*/
            0x10000000,                         /* MixingCoeffs [0][4]*/
            0x10000000,                         /* MixingCoeffs [0][5]*/
            0x10000000,                         /* MixingCoeffs [0][6]*/
            0x10000000                          /* MixingCoeffs [0][7]*/
        },
        {                                       /* MixingCoeffs [1][]*/
            0x10000000,                         /* MixingCoeffs [1][0]*/
            0x10000000,                         /* MixingCoeffs [1][1]*/
            0x10000000,                         /* MixingCoeffs [1][2]*/
            0x10000000,                         /* MixingCoeffs [1][3]*/
            0x10000000,                         /* MixingCoeffs [1][4]*/
            0x10000000,                         /* MixingCoeffs [1][5]*/
            0x10000000,                         /* MixingCoeffs [1][6]*/
            0x10000000                          /* MixingCoeffs [1][7]*/
        },
        {                                       /* MixingCoeffs [2][]*/
            0x10000000,                         /* MixingCoeffs [2][0]*/
            0x10000000,                         /* MixingCoeffs [2][1]*/
            0x10000000,                         /* MixingCoeffs [2][2]*/
            0x10000000,                         /* MixingCoeffs [2][3]*/
            0x10000000,                         /* MixingCoeffs [2][4]*/
            0x10000000,                         /* MixingCoeffs [2][5]*/
            0x10000000,                         /* MixingCoeffs [2][6]*/
            0x10000000                          /* MixingCoeffs [2][7]*/
        },
    },
    0,                                          /* i32UserMixBalance */
    0,                                          /* i32CustomEffectsAudioMixingEnable*/
    {
        /*L*/   {0x40000000, 0, 0, 0, 0, 0},
        /*R*/   {0, 0x40000000, 0, 0, 0, 0},
        /*LS*/  {0, 0, 0x40000000, 0, 0, 0},
        /*RS*/  {0, 0, 0, 0x40000000, 0, 0},
        /*C*/   {0, 0, 0, 0, 0x40000000, 0},
        /*LFE*/ {0, 0, 0, 0, 0, 0x40000000}
    },
	BDSP_AF_P_BurstFill_ePauseBurst,
	BDSP_AF_P_SpdifPauseWidth_eEightWord
};

const BDSP_Raaga_Audio_FadeCtrlConfigParams BDSP_sDefFadeCtrlConfigSettings =
{
    0x7FFFFFFF,                                       /* ui32VolumeTargetLevel */
    2,                                                /* ui32EasingFunctionType */
    4800,                                             /* ui32DurationInSamples */
};

const BDSP_Raaga_Audio_AmbisonicsConfigParams BDSP_sDefAmbisonicsConfigSettings =
{
    1,                                                  /* ui32BinauralRender */
    1,                                                  /* ui32BinauralRender */
    0,                                                  /* ui32Yaw */
    0,                                                  /* ui32Pitch */
    0,                                                  /* ui32Roll */
};

const BDSP_Raaga_VideoBH264UserConfig BDSP_sBH264EncodeUserConfigSettings =
{
    BDSP_Raaga_VideoH264Profile_eBaseline,
    31,
    BDSP_Raaga_VideoEncodeMode_eAfap,
    1000000,
    320,
    240,
    30,
    30,
    BDSP_Raaga_VideoGopStruct_eIP,
    false,
    false,
    0,
    {
        1,
        {8,8,8},
        {50,50,50},
        4,
        0
    },
    BDSP_VF_P_EncodeFrameRate_e30,
    1,
    0,
    0,
    1,
    0,
    1
};

const BDSP_Raaga_Audio_DpcmrConfigParams BDSP_sDefDpcmrConfigSettings=
{
            0,          /* delay */
            6,          /* num_mc_channels */
            6,          /* num_raw_mc_channels */
            1,          /* num_metadata_sets */
            {
                {/*BDSP_FWIF_Pcmr_User_Params            multichannel_out[0] */
                    100,                                /* drc_cut */
                    100,                                /* drc_boost*/
                    0,                                  /* drc_mode */
                    0,                                  /* dmx_mode */
                    0,                                  /* dual_mode */
                    1                                   /* b_output_active */
                },
                {/*BDSP_FWIF_Pcmr_User_Params            multichannel_out[1] */
                    100,                                /* drc_cut */
                    100,                                /* drc_boost*/
                    0,                                  /* drc_mode */
                    0,                                  /* dmx_mode */
                    0,                                  /* dual_mode */
                    1                                  /* b_output_active */
                }
            },
            {/*BDSP_FWIF_Pcmr_User_Params            downmix_out */
                100,                                /* drc_cut */
                100,                                /* drc_boost*/
                0,                                  /* drc_mode */
                0,                                  /* dmx_mode */
                0,                                  /* dual_mode */
                1                                   /* b_output_active */
            },
            0,          /* b_discard_drc */
            1,           /*compressor_profile*/
        0
};


const BDSP_Raaga_Audio_DDPEncConfigParams		BDSP_sDefDdpencConfigSettings=
{
	{0,1,2,3,4,5,6,7},	/* Input channel routing */
	8,					/* Encoder Mode */
	7,					/* Audio Coding Mode */
	0,					/* Center Mix Level, not set */
	31,					/* Dialnorm */
	31,					/* Dialnorm Channel 2 */
	0,					/* Dolby Surround Mode */
	1,					/* LFE Enable*/
	0,					/* Evolution Metadata Availability Flag (Mode 8 DDP Only) */
	0,					/* Surround Mix Level, not set  */
	1,					/* 90 Degree Phase Shift Filter */
	640,				/* Datarate */
	0,					/* Preferred Stereo Downmix Mode (Mode 8 DDP Only) */
	4,					/* LtRt Center Mix Level (Mode 8 DDP Only), not set  */
	4,					/* LtRt Surround Mix Level (Mode 8 DDP Only), not set  */
	4,					/* LoRo Center Mix Level, not set  */
	4,					/* LoRo Surround Mix Level, not set  */
	0,					/* Dolby EX Mode (Mode 8 DDP Only) */
	0,					/* Dolby Headphone Mode (Mode 8 DDP Only) */
	0,					/* A/D Converter Type (Mode 8 DDP Only) */
	0,					/* Send Audio Production Info */
	0,					/* Send Audio Production Info Channel 2 */
	80,					/* Audio production mixing level */
	80,					/* Audio production mixing level Channel 2 */
	1,					/* Copyright flag */
	1,					/* Original Bitstream flag */
	0,					/* Bitstream Mode */
	0,					/* Audio production room type */
	0,					/* Audio production room type Channel 2 */
	0,					/* Additional Bitstream Information */
	0,					/* Dolby Certification Flag */
	0,					/* Enable Low Complexity Encoding */
	33,					/* Intelligent loudness payloads only passed through by default */
	0,					/* SPDIF Packing */
	0,					/* Encoder Variant MS12 or MS11 */
	{0},				/* Additional Bitstream Information String */
	0,
	{0},				/* drc1 */
	0,
	{0},				/* drc2 */
	1,                  /* pcm_aligned_flag if 0, disable input PCM alignment to frame boundary (default enabled) */
	1,                  /* converter_snr_offset (default enabled) */
	0                   /* '-di' is specified and downmix input exists '*/
} ;
const BDSP_Raaga_Audio_DolbyAacheUserConfig BDSP_sDolbyAacheDefaultUserConfig =
{
	0,                      /* eDecoderType */
	5,                      /* eDolbyAacheUsageMode */
	1,                      /* ui32MapTo51Channels */
	124,                    /* ui32RefDialnormLevel */
	108,                    /* ui32DefDialnormLevel */
	2,                      /* ui32DrcMode */
	0,                      /* ui32DrcType */
	1,                      /* ui32Fast2to1Resample */
	0,                      /* ui32UpsampleToPrimaryRates */
	0,                      /* ui32EnableErrorConcealment */
	0,                      /* ui32EnableStereoDownmixType */
	0,                      /* ui32IDKCertificationFlag */
	0,                      /* ui32LoudnessEquivalenceMode */
	{
		{
			{ 0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },/* Multichannel */
		},
		{
			{ 0, 1,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },/* Stereo */
		}
	}
};