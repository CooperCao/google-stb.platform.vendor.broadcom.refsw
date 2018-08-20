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

#include "bdsp_common_priv_include.h"

static const BDSP_P_AlgorithmInfo BDSP_sAlgorithmInfo[] =
{
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eMpegAudioDecode, BDSP_AlgorithmType_eAudioDecode, "MPEG Audio Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sMpegDefaultUserConfig, sizeof(BDSP_Raaga_Audio_MpegConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_MpegStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_MpegStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		2,   					         1152,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eUdcDecode, BDSP_AlgorithmType_eAudioDecode, "UDC Audio Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sUdcdecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_UdcdecConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_UdcStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_UdcStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		8,   					         1536,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */ 				   /* Type */						      /* Name */
		BDSP_Algorithm_eUdcPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "UDC Passthru",
		/* Default User Config */			 /* User config size */
		&BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
		/* Stream Info Size */					 /* Valid offset */
		sizeof(BDSP_Raaga_Audio_DdpStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DdpStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		1,								 24576,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eAc3Decode, BDSP_AlgorithmType_eAudioDecode, "AC3 Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sUdcdecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_UdcdecConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_UdcStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_UdcStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		8,   					         1536,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */ 				   /* Type */						      /* Name */
		BDSP_Algorithm_eAc3Passthrough, BDSP_AlgorithmType_eAudioPassthrough, "AC3 Passthru",
		/* Default User Config */			 /* User config size */
		&BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
		/* Stream Info Size */					 /* Valid offset */
		sizeof(BDSP_Raaga_Audio_DdpStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DdpStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		1,								 6144,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eAc3PlusDecode, BDSP_AlgorithmType_eAudioDecode, "AC3+ Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sUdcdecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_UdcdecConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_UdcStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_UdcStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		8,   					         1536,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */ 				         /* Type */						      /* Name */
		BDSP_Algorithm_eAc3PlusPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "AC3+ Passthru",
		/* Default User Config */			 /* User config size */
		&BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
		/* Stream Info Size */					 /* Valid offset */
		sizeof(BDSP_Raaga_Audio_DdpStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DdpStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		1,								 6144,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eAacAdtsDecode, BDSP_AlgorithmType_eAudioDecode, "AAC ADTS Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sAacheDefaultUserConfig, sizeof(BDSP_Raaga_Audio_AacheConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_AacStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		6,   					         2048,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */ 				   		/* Type */								/* Name */
		BDSP_Algorithm_eAacAdtsPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "AAC ADTS Passthru",
		/* Default User Config */			 /* User config size */
		&BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
		/* Stream Info Size */					 /* Valid offset */
		sizeof(BDSP_Raaga_Audio_AacStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		1,								 6144,/* For time being only LOAS 6144 samples */
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eAacLoasDecode, BDSP_AlgorithmType_eAudioDecode, "AAC LOAS Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sAacheDefaultUserConfig, sizeof(BDSP_Raaga_Audio_AacheConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_AacStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		6,   					         2048,
		/* Preemption levels*/
		{
		  true,
		  false,
		  false
		}
	},
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eDolbyAacheAdtsDecode, BDSP_AlgorithmType_eAudioDecode, "DOLBY AAC ADTS Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sDolbyAacheDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DolbyAacheUserConfig),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_AacheStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacheStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		6,   					         2048,
		/* Preemption levels*/
		{
		  true,
		  false,
		  false
		}
	},
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eDolbyAacheLoasDecode, BDSP_AlgorithmType_eAudioDecode, "DOLBY AAC LOAS Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sDolbyAacheDefaultUserConfig, sizeof(BDSP_Raaga_Audio_DolbyAacheUserConfig),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_AacheStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacheStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		6,   					         2048,
		/* Preemption levels*/
		{
		  true,
		  false,
		  false
		}
	},

	{
		/* Algorithm */					 /* Type */ 					  /* Name */		  /* Supported */
		BDSP_Algorithm_eAacLoasPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "AAC LOAS Audio Passthru",
		/* Default User Config */ 		   /* User config size */
		&BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
		/* Stream Info Size */				   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_AacStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacStreamInfo, ui32StatusValid),
		/* IDS status size */ 				   /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */		/* samples per channel */
		1,							   6144,/* For time being only LOAS 6144 samples */
		/* Preemption levels*/
		{
		  true,
		  false,
		  false
		}
	},
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_ePcmWavDecode, BDSP_AlgorithmType_eAudioDecode, "PCMWAV Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sPcmWavDefaultUserConfig, sizeof(BDSP_Raaga_Audio_PcmWavConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_PcmWavStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_PcmWavStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		8,   					         5760,/* 30 ms at 192KHz*/
		/* Preemption levels*/
		{
		  true,
		  false,
		  false
		}
	},
	{
		/* Algorithm */				   /* Type */						/* Name */
		BDSP_Algorithm_eSrc, BDSP_AlgorithmType_eAudioProcessing, "Sample Rate Conversion",
		/* Default User Config */ 		 /* User config size */
		NULL, 							  0,
		/* Stream Info Size */			/* Valid offset */
		0,								0xffffffff,
		/* IDS status size */ 			  /* TSM status size */
		0,								  0,
		/* Max Channels Supported */		/* Samples per channel */
		6,							   (8*1024),
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */				   /* Type */						/* Name */
		BDSP_Algorithm_eMixerDapv2, BDSP_AlgorithmType_eAudioMixer, "Mixer Dapv2",
		/* Default User Config */ 		 /* User config size */
		&BDSP_sDefMixerDapv2ConfigParams, sizeof(BDSP_Raaga_Audio_MixerDapv2ConfigParams),
		/* Stream Info Size */			/* Valid offset */
		sizeof(BDSP_Raaga_Audio_MixerDapv2StatusInfo), 0xffffffff,
		/* IDS status size */ 			  /* TSM status size */
		0,								  0,
		/* Max Channels Supported */		/* Samples per channel */
		8,								 (8*1536),
        /* Preemption levels*/
		{
			true,
			false,
			false
		}
    },
	{
		/* Algorithm */				   /* Type */						/* Name */
		BDSP_Algorithm_eMixer, BDSP_AlgorithmType_eAudioMixer, "Mixer",
		/* Default User Config */ 		 /* User config size */
		&BDSP_sDefFwMixerConfigSettings, sizeof(BDSP_Raaga_Audio_MixerConfigParams),
		/* Stream Info Size */			/* Valid offset */
		0, 0xffffffff,
		/* IDS status size */ 			  /* TSM status size */
		0,								  0,
		/* Max Channels Supported */		/* Samples per channel */
		8,								 (8*1536),
        /* Preemption levels*/
		{
			true,
			false,
			false
		}
    },
    {
		/* Algorithm */				   /* Type */						/* Name */
		BDSP_Algorithm_eDpcmr, BDSP_AlgorithmType_eAudioProcessing, "Dolby PCM Renderer",
		/* Default User Config */ 		 /* User config size */
		&BDSP_sDefDpcmrConfigSettings, 	sizeof(BDSP_Raaga_Audio_DpcmrConfigParams),
		/* Stream Info Size */			/* Valid offset */
		0,								0xffffffff,
		/* IDS status size */ 			  /* TSM status size */
		0,								  0,
		/* Max Channels Supported */		/* Samples per channel */
		8,							   (8*1536),
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */				   /* Type */						/* Name */
		BDSP_Algorithm_eDDPEncode, BDSP_AlgorithmType_eAudioEncode, "DDP Encode",
		/* Default User Config */ 		  /* User config size */
		&BDSP_sDefDdpencConfigSettings, 	sizeof(BDSP_Raaga_Audio_DDPEncConfigParams),
		/* Stream Info Size */			  /* Valid offset */
		0,								0xffffffff,
		/* IDS status size */ 			  /* TSM status size */
		0,								  0,
		/* Max Channels Supported */	  /* Samples per channel */
		1,							   1536,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
    {
		/* Algorithm */				   /* Type */						/* Name */
		BDSP_Algorithm_eAacEncode, BDSP_AlgorithmType_eAudioEncode, "AACHE Encode",
		/* Default User Config */ 		  /* User config size */
		&BDSP_sDefAacHeENCConfigSettings, 	sizeof(BDSP_Raaga_Audio_AacheEncConfigParams),
		/* Stream Info Size */			  /* Valid offset */
		0,								0xffffffff,
		/* IDS status size */ 			  /* TSM status size */
		0,								  0,
		/* Max Channels Supported */	  /* Samples per channel */
		1,							   (2*1536),
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
    {
        /* Algorithm */				   /* Type */						/* Name */
        BDSP_Algorithm_eGenCdbItb, BDSP_AlgorithmType_eAudioProcessing, "Gen CDB/ITB PP",
        /* Default User Config */ 		  /* User config size */
        &BDSP_sDefGenCdbItbConfigSettings, 	sizeof(BDSP_Raaga_Audio_GenCdbItbConfigParams),
        /* Stream Info Size */			                    /* Valid offset */
        sizeof(BDSP_Raaga_Audio_GenCdbItbStreamInfo),		   0xffffffff,
        /* IDS status size */ 			  /* TSM status size */
        0,								  0,
        /* Max Channels Supported */	  /* Samples per channel */
        2,							   (2*1536),
        /* Preemption levels*/
        {
            true,
            false,
            false
        }
    },
    {
        /* Algorithm */                /* Type */                       /* Name */
        BDSP_Algorithm_eDsola, BDSP_AlgorithmType_eAudioProcessing, "DSOLA PP",
        /* Default User Config */         /* User config size */
        &BDSP_sDefDsolaConfigSettings,  sizeof(BDSP_Raaga_Audio_DsolaConfigParams),
        /* Stream Info Size */            /* Valid offset */
        0,                              0xffffffff,
        /* IDS status size */             /* TSM status size */
        0,                                0,
        /* Max Channels Supported */      /* Samples per channel */
        2,                             8192,
        /* Preemption levels*/
        {
            true,
            false,
            false
        }
    },
    {
        /* Algorithm */                    /* Type */                       /* Name */
        BDSP_Algorithm_eOpusDecode, BDSP_AlgorithmType_eAudioDecode, "OPUS Audio Decode",
        /* Default User Config */            /* User config size */
        &BDSP_sOpusDecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_OpusDecConfigParams),
        /* Stream Info Size */                   /* Valid offset */
        sizeof(BDSP_Raaga_Audio_OpusDecStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_OpusDecStreamInfo, ui32StatusValid),
        /* IDS status size */					 /* TSM status size */
        sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
        /* Max Channels Supported */      /* samples per channel */
        8,   					         2880,
        /* Preemption levels*/
        {
            true,
            false,
            false
        }
    },
	{
		/* Algorithm */ 				   /* Type */						/* Name */
		BDSP_Algorithm_eFlacDecode, BDSP_AlgorithmType_eAudioDecode, "Flac Audio Decode",
		/* Default User Config */			 /* User config size */
		&BDSP_sFlacDecUserConfig, sizeof(BDSP_Raaga_Audio_FlacDecConfigParams),
		/* Stream Info Size */					 /* Valid offset */
		sizeof(BDSP_Raaga_Audio_FlacDecStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_FlacDecStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		8,								 16384,/* TODO: Check the value */
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
        /* Algorithm */                    /* Type */                       /* Name */
        BDSP_Algorithm_eALSDecode, BDSP_AlgorithmType_eAudioDecode, "ALS Audio Decode",
        /* Default User Config */            /* User config size */
        &BDSP_sALSDecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_ALSDecConfigParams),
        /* Stream Info Size */                   /* Valid offset */
        sizeof(BDSP_Raaga_Audio_ALSDecStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_ALSDecStreamInfo, ui32StatusValid),
        /* IDS status size */					 /* TSM status size */
        sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
        /* Max Channels Supported */      /* samples per channel */
        6,   					         4096,
        /* Preemption levels*/
        {
            true,
            false,
            false
        }
    },
    {
        /* Algorithm */                    /* Type */                       /* Name */
        BDSP_Algorithm_eALSLoasDecode, BDSP_AlgorithmType_eAudioDecode, "ALS LOAS Audio Decode",
        /* Default User Config */            /* User config size */
        &BDSP_sALSDecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_ALSDecConfigParams),
        /* Stream Info Size */                   /* Valid offset */
        sizeof(BDSP_Raaga_Audio_ALSDecStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_ALSDecStreamInfo, ui32StatusValid),
        /* IDS status size */					 /* TSM status size */
        sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
        /* Max Channels Supported */      /* samples per channel */
        6,   					         4096,
        /* Preemption levels*/
        {
            true,
            false,
            false
        }
    },
    {
        /* Algorithm */                /* Type */                       /* Name */
        BDSP_Algorithm_eLpcmEncode, BDSP_AlgorithmType_eAudioEncode, "LPCM Encode",
        /* Default User Config */         /* User config size */
        &BDSP_sDefLpcmEncConfigSettings,  sizeof(BDSP_Raaga_Audio_LpcmEncConfigParams),
        /* Stream Info Size */            /* Valid offset */
        0,                              0xffffffff,
        /* IDS status size */             /* TSM status size */
        0,                                0,
        /* Max Channels Supported */      /* Samples per channel */
        1,                             3072,
        /* Preemption levels*/
        {
            true,
            false,
            false
        }
    },

    {
        /* Algorithm */                /* Type */                       /* Name */
        BDSP_Algorithm_eMpegAudioEncode, BDSP_AlgorithmType_eAudioEncode, "MPEG Audio Encode",
        /* Default User Config */         /* User config size */
        &BDSP_sDefMpeg1L3EncConfigSettings,  sizeof(BDSP_Raaga_Audio_Mpeg1L3EncConfigParams),
        /* Stream Info Size */            /* Valid offset */
        0,                              0xffffffff,
        /* IDS status size */             /* TSM status size */
        0,                                0,
        /* Max Channels Supported */      /* Samples per channel */
        2,                             1024,
        /* Preemption levels*/
        {
            true,
            false,
            false
        }
    },
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eAC4Decode, BDSP_AlgorithmType_eAudioDecode, "AC4 Audio Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sAC4DecDefaultUserConfig, sizeof(BDSP_Raaga_Audio_AC4DecConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_AC4StreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AC4StreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		8,   					         2230,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */ 				   /* Type */						      /* Name */
		BDSP_Algorithm_eMpegAudioPassthrough, BDSP_AlgorithmType_eAudioPassthrough, "MPEG Passthru",
		/* Default User Config */			 /* User config size */
		&BDSP_sDefaultPassthruSettings, sizeof(BDSP_Raaga_Audio_PassthruConfigParams),
		/* Stream Info Size */					 /* Valid offset */
		sizeof(BDSP_Raaga_Audio_MpegStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_MpegStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		1,								 1152,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */ 				   /* Type */						/* Name */
		BDSP_Algorithm_eWmaStdDecode, BDSP_AlgorithmType_eAudioDecode, "Wma Audio Decode",
		/* Default User Config */			 /* User config size */
		&BDSP_sWmaDefaultUserConfig, sizeof(BDSP_Raaga_Audio_WmaConfigParams),
		/* Stream Info Size */					 /* Valid offset */
		sizeof(BDSP_Raaga_Audio_WmaStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_WmaStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		2,								 2048,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eWmaProDecode, BDSP_AlgorithmType_eAudioDecode, "WmaPro Audio Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sWmaProDefaultUserConfig, sizeof(BDSP_Raaga_Audio_WmaProConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_WmaProStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_WmaProStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		6,   					         2048,
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
    /* This entry must always be last used to derive the unsupported/invalid information */
	{
		BDSP_Algorithm_eLpcmDvdDecode, BDSP_AlgorithmType_eAudioDecode, "DVD LPCM Audio Decode",
		&BDSP_sLcpmDvdDefaultUserConfig, sizeof(BDSP_Raaga_Audio_LpcmUserConfig),
		sizeof(BDSP_Raaga_Audio_LpcmStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_LpcmStreamInfo, ui32StatusValid),
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		8,   					         1152,
		{
			true,
			false,
			false
		}
	},
	{
		BDSP_Algorithm_eLpcm1394Decode, BDSP_AlgorithmType_eAudioDecode, "LPCM 1394 Audio Decode",
		&BDSP_sLcpmDvdDefaultUserConfig, sizeof(BDSP_Raaga_Audio_LpcmUserConfig),
		sizeof(BDSP_Raaga_Audio_LpcmStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_LpcmStreamInfo, ui32StatusValid),
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		8,   					         1152,
		{
			true,
			false,
			false
		}
	},
	{
		BDSP_Algorithm_eLpcmBdDecode, BDSP_AlgorithmType_eAudioDecode, "BD LPCM Audio Decode",
		&BDSP_sLcpmDvdDefaultUserConfig, sizeof(BDSP_Raaga_Audio_LpcmUserConfig),
		sizeof(BDSP_Raaga_Audio_LpcmStreamInfo), BDSP_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_LpcmStreamInfo, ui32StatusValid),
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		8,   					         1152,
		{
			true,
			false,
			false
		}
	},
    {
		BDSP_Algorithm_eOutputFormatter, BDSP_AlgorithmType_eAudioProcessing, "Output Formatter",
		&BDSP_sDefOutputFormatterConfigSettings, sizeof(BDSP_Raaga_Audio_OutputFormatterConfigParams),
		0, 0xffffffff,
		0, 0,
		8,   					         4096,
		{
			true,
			false,
			false
		}
	},
	{
		/* Algorithm */				   /* Type */						/* Name */
		BDSP_Algorithm_eSoftFMM, BDSP_AlgorithmType_eAudioProcessing, "Soft FMM",
		/* Default User Config */ 		 /* User config size */
		NULL,                            0,
		/* Stream Info Size */			/* Valid offset */
		0,								0xffffffff,
		/* IDS status size */ 			  /* TSM status size */
		0,								  0,
		/* Max Channels Supported */		/* Samples per channel */
		6,							   (8*1024),
		/* Preemption levels*/
		{
			true,
			false,
			false
		}
	},
    /* This entry must always be last used to derive the unsupported/invalid information */
	{
		BDSP_Algorithm_eMax, BDSP_AlgorithmType_eMax, "Invalid",
		NULL, 0,
		0, 0xffffffff,
		0, 0,
		0, 0,
		{
			false,
			false,
			false
		}
	}
};

const BDSP_P_AlgorithmInfo *BDSP_P_LookupAlgorithmInfo_isrsafe(
    BDSP_Algorithm algorithm
)
{
  unsigned i;

  for ( i = 0; BDSP_sAlgorithmInfo[i].algorithm != BDSP_Algorithm_eMax; i++ )
  {
    if ( BDSP_sAlgorithmInfo[i].algorithm == algorithm )
    {
      break;
    }
    }

    return &BDSP_sAlgorithmInfo[i];
}
