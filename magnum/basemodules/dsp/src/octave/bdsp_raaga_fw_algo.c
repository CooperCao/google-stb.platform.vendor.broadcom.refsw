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

#include "bdsp_raaga_priv_include.h"

const uint32_t BDSP_SystemID_MemoryReqd[BDSP_SystemImgId_eMax] = {
    BDSP_IMG_SYSTEM_KERNEL_SIZE,
    BDSP_IMG_SYSTEM_RDBVARS_SIZE,
    BDSP_IMG_INIT_ROMFS_SIZE,
    BDSP_IMG_SYSTEM_CODE_SIZE,
    BDSP_IMG_SYSTEM_LIB_SIZE,
};

static const BDSP_P_AlgorithmSupportInfo BDSP_sAlgorithmSupportInfo[] =
{
#ifdef BDSP_MPEG_SUPPORT
    {BDSP_Algorithm_eMpegAudioDecode, "MPEG Audio Decode", true},
#endif
#ifdef BDSP_UDC_SUPPORT
	{BDSP_Algorithm_eUdcDecode, "UDC Audio Decode", true},
#endif
#ifdef BDSP_AC3_SUPPORT
	{BDSP_Algorithm_eAc3Decode, "AC3 Decode", true},
#endif
#ifdef BDSP_AC3_PASSTHRU_SUPPORT
	{BDSP_Algorithm_eAc3Passthrough, "AC3 Passthru", true},
#endif
#ifdef BDSP_DDP_SUPPORT
	{BDSP_Algorithm_eAc3Decode,     "AC3  Decode", true},
	{BDSP_Algorithm_eAc3PlusDecode, "AC3+ Decode", true},
#endif
#ifdef BDSP_DDP_PASSTHRU_SUPPORT
{BDSP_Algorithm_eAc3PlusPassthrough,  "AC3+ Passthru", true},
#endif
#ifdef BDSP_AACSBR_SUPPORT
	{BDSP_Algorithm_eAacAdtsDecode, "AAC ADTS Decode", true},
	{BDSP_Algorithm_eAacLoasDecode, "AAC LOAS Decode", true},
#endif
#ifdef BDSP_AACSBR_PASSTHRU_SUPPORT
	{BDSP_Algorithm_eAacLoasPassthrough,  "AAC LOAS Passthru", true},
	{BDSP_Algorithm_eAacAdtsPassthrough,  "AAC ADTS Passthru", true},
#endif
#ifdef BDSP_PCMWAV_SUPPORT
	{BDSP_Algorithm_ePcmWavDecode, "PCMWAV Decode", true},
#endif
#ifdef BDSP_SRC_SUPPORT
	{BDSP_Algorithm_eSrc, "Sample Rate Conversion", true},
#endif
	{BDSP_Algorithm_eMax, "Invalid", false}
};

static const BDSP_P_AlgorithmInfo BDSP_sAlgorithmInfo[] =
{
	{
		/* Algorithm */                    /* Type */                       /* Name */
		BDSP_Algorithm_eMpegAudioDecode, BDSP_AlgorithmType_eAudioDecode, "MPEG Audio Decode",
		/* Default User Config */            /* User config size */
		&BDSP_sMpegDefaultUserConfig, sizeof(BDSP_Raaga_Audio_MpegConfigParams),
		/* Stream Info Size */                   /* Valid offset */
		sizeof(BDSP_Raaga_Audio_MpegStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_MpegStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		2,   					         1152,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_MPEG1_SCRATCH_SIZE, BDSP_IMG_ADEC_MPEG1_TABLES_SIZE,
		/* interframe size */                    /*  Compressed interframe size */
		BDSP_IMG_ADEC_MPEG1_INTER_FRAME_SIZE, BDSP_IMG_ADEC_MPEG1_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_MPEG1_SIZE,        BDSP_IMG_AIDS_MPEG1_SIZE,
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
		sizeof(BDSP_Raaga_Audio_UdcStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_UdcStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		8,   					         1536,
		/* Scratch buffer size */			/* rom table size */
		BDSP_IMG_ADEC_UDC_SCRATCH_SIZE,	BDSP_IMG_ADEC_UDC_TABLES_SIZE,
		/* interframe size */					/* Compressed interframe size */
		BDSP_IMG_ADEC_UDC_INTER_FRAME_SIZE, BDSP_IMG_ADEC_UDC_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_UDC_SIZE,        BDSP_IMG_AIDS_DDP_SIZE,
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
		sizeof(BDSP_Raaga_Audio_UdcStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_UdcStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		8,   					         1536,
		/* Scratch buffer size */			/* rom table size */
		BDSP_IMG_ADEC_AC3_SCRATCH_SIZE,	BDSP_IMG_ADEC_AC3_TABLES_SIZE,
		/* interframe size */					/*Compressed interframe size */
		BDSP_IMG_ADEC_AC3_INTER_FRAME_SIZE, BDSP_IMG_ADEC_AC3_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_AC3_SIZE,        BDSP_IMG_AIDS_DDP_SIZE,
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
		sizeof(BDSP_Raaga_Audio_DdpStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DdpStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		1,								 24576,
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		/* interframe size */                       /*Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 		   /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	 BDSP_IMG_AIDS_DDP_SIZE,
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
		sizeof(BDSP_Raaga_Audio_UdcStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_UdcStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		8,   					         1536,
		/* Scratch buffer size */           /* rom table size */
		BDSP_IMG_ADEC_DDP_SCRATCH_SIZE, BDSP_IMG_ADEC_DDP_TABLES_SIZE,
		/* interframe size */					/*Compressed interframe size */
		BDSP_IMG_ADEC_DDP_INTER_FRAME_SIZE, BDSP_IMG_ADEC_DDP_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_DDP_SIZE,        BDSP_IMG_AIDS_DDP_SIZE,
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
		sizeof(BDSP_Raaga_Audio_DdpStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_DdpStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		1,								 98304,
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		/* interframe size */						 /*Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 		   /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	 BDSP_IMG_AIDS_DDP_SIZE,
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
		sizeof(BDSP_Raaga_Audio_AacStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		6,   					         2048,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_AACHE_SCRATCH_SIZE, BDSP_IMG_ADEC_AACHE_TABLES_SIZE,
		/* interframe size */				      /*Compressed interframe size */
		BDSP_IMG_ADEC_AACHE_INTER_FRAME_SIZE, BDSP_IMG_ADEC_AACHE_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_AACHE_SIZE,        BDSP_IMG_AIDS_ADTS_SIZE,
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
		sizeof(BDSP_Raaga_Audio_AacStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */	  /* samples per channel */
		1,								 6144,/* For time being only LOAS 6144 samples */
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		/* interframe size */				  	     /* Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 		   /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	 BDSP_IMG_AIDS_ADTS_SIZE,
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
		sizeof(BDSP_Raaga_Audio_AacStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		6,   					         2048,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_AACHE_SCRATCH_SIZE, BDSP_IMG_ADEC_AACHE_TABLES_SIZE,
		/* interframe size */				      /* Compressed interframe size */
		BDSP_IMG_ADEC_AACHE_INTER_FRAME_SIZE, BDSP_IMG_ADEC_AACHE_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_AACHE_SIZE,        BDSP_IMG_AIDS_LOAS_SIZE,
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
		sizeof(BDSP_Raaga_Audio_AacStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_AacStreamInfo, ui32StatusValid),
		/* IDS status size */ 				   /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */		/* samples per channel */
		1,							   6144,/* For time being only LOAS 6144 samples */
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		 /* interframe size */ 					 	 /* Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			 /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	   BDSP_IMG_AIDS_LOAS_SIZE,
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
		sizeof(BDSP_Raaga_Audio_PcmWavStreamInfo), BDSP_RAAGA_STREAMINFO_VALID_OFFSET(BDSP_Raaga_Audio_PcmWavStreamInfo, ui32StatusValid),
		/* IDS status size */					 /* TSM status size */
		sizeof(BDSP_AudioTaskDatasyncStatus), sizeof(BDSP_AudioTaskTsmStatus),
		/* Max Channels Supported */      /* samples per channel */
		8,   					         5760,/* 30 ms at 192KHz*/
		/* Scratch buffer size */			   /* rom table size */
		BDSP_IMG_ADEC_PCMWAV_SCRATCH_SIZE, BDSP_IMG_ADEC_PCMWAV_TABLES_SIZE,
		/* interframe size */					   /*Compressed interframe size */
		BDSP_IMG_ADEC_PCMWAV_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PCMWAV_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_PCMWAV_SIZE,        BDSP_IMG_AIDS_WAVFORMATEX_SIZE,
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
		/* Scratch buffer size */		   /* Rom table size */
		BDSP_IMG_APP_SRC_SCRATCH_SIZE, BDSP_IMG_APP_SRC_TABLES_SIZE,
		/* Interframe size */  			  /* Compressed Interframe size */
		BDSP_IMG_APP_SRC_INTER_FRAME_SIZE, BDSP_IMG_APP_SRC_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/		  /* IDS codeSize*/
		BDSP_IMG_APP_SRC_SIZE,		 0,
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
		0, 0,
		0, 0,
		0, 0,
		{
			false,
			false,
			false
		}
	}
};

const BDSP_P_AlgorithmInfo *BDSP_Raaga_P_LookupAlgorithmInfo_isrsafe(
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

const BDSP_P_AlgorithmSupportInfo *BDSP_Raaga_P_LookupAlgorithmSupportInfo(
    BDSP_Algorithm algorithm
)
{
	unsigned i;

	for (i = 0; BDSP_sAlgorithmSupportInfo[i].algorithm != BDSP_Algorithm_eMax; i++ )
	{
		if ( BDSP_sAlgorithmSupportInfo[i].algorithm == algorithm )
		{
		  break;
		}
	}
	return &BDSP_sAlgorithmSupportInfo[i];
}
