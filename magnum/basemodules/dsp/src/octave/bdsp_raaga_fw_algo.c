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

#include "bdsp_raaga_priv_include.h"

const uint32_t BDSP_SystemID_MemoryReqd[BDSP_SystemImgId_eMax] = {
    BDSP_IMG_SYSTEM_KERNEL_SIZE,
    BDSP_IMG_SYSTEM_RDBVARS_SIZE,
    BDSP_IMG_INIT_ROMFS_SIZE,
    BDSP_IMG_SYSTEM_CODE_SIZE,
    BDSP_IMG_SYSTEM_LIB_SIZE,
};

static const BDSP_P_AlgorithmSupportInfo BDSP_sRaagaAlgorithmSupportInfo[] =
{
#ifdef BDSP_MPEG_SUPPORT
    {BDSP_Algorithm_eMpegAudioDecode, "MPEG Decode", true},
#endif
#ifdef BDSP_UDC_SUPPORT
	{BDSP_Algorithm_eUdcDecode, "UDC Decode", true},
#endif
#ifdef BDSP_UDC_PASSTHRU_SUPPORT
	{BDSP_Algorithm_eUdcPassthrough, "UDC Passthru", true},
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
#ifdef BDSP_DOLBY_AACHE_SUPPORT
	{BDSP_Algorithm_eDolbyAacheAdtsDecode, "Dolby AACHE ADTS Decode", true},
	{BDSP_Algorithm_eDolbyAacheLoasDecode, "Dolby AACHE LOAS Decode", true},
#endif

#ifdef BDSP_AACSBR_PASSTHRU_SUPPORT
	{BDSP_Algorithm_eAacLoasPassthrough,  "AAC LOAS Passthru", true},
	{BDSP_Algorithm_eAacAdtsPassthrough,  "AAC ADTS Passthru", true},
#endif
#ifdef BDSP_PCMWAV_SUPPORT
	{BDSP_Algorithm_ePcmWavDecode, "PCMWAV Decode", true},
#endif
#ifdef BDSP_FLACDEC_SUPPORT
    {BDSP_Algorithm_eFlacDecode, "FLAC Decode", true},
#endif
#ifdef BDSP_SRC_SUPPORT
	{BDSP_Algorithm_eSrc, "Sample Rate Conversion", true},
#endif
#ifdef BDSP_MIXERDAPV2_SUPPORT
#ifdef BDSP_MS11_SUPPORT
    {BDSP_Algorithm_eMixerDapv2, "Mixer", true},
#else
	{BDSP_Algorithm_eMixerDapv2, "Mixer Dapv2", true},
#endif
#endif
#ifdef BDSP_FWMIXER_SUPPORT
	{BDSP_Algorithm_eMixer, "FW Mixer", true},
#endif
#ifdef BDSP_DPCMR_SUPPORT
	{BDSP_Algorithm_eDpcmr, "Dolby PCM Renderer", true},
#endif
#ifdef BDSP_DDPENC_SUPPORT
#ifdef BDSP_MS11_SUPPORT
	{BDSP_Algorithm_eDDPEncode, "DD Encode", true},
#else
	{BDSP_Algorithm_eDDPEncode, "DDP Encode", true},
#endif
#endif
#ifdef BDSP_AACHEENC_SUPPORT
	{BDSP_Algorithm_eAacEncode, "AACHE Encode", true},
#endif
#ifdef BDSP_GENCDBITB_SUPPORT
	{BDSP_Algorithm_eGenCdbItb, "Gen CDB/ITB PP", true},
#endif
#ifdef BDSP_DSOLA_SUPPORT
    {BDSP_Algorithm_eDsola, "DSOLA PP", true},
#endif
#ifdef BDSP_OPUSDEC_SUPPORT
    {BDSP_Algorithm_eOpusDecode, "OPUS Decode", true},
#endif
#ifdef BDSP_ALS_SUPPORT
    {BDSP_Algorithm_eALSDecode, "ALS Decode", true},
    {BDSP_Algorithm_eALSLoasDecode, "ALS LOAS Decode", true},
#endif
#ifdef BDSP_LPCMENC_SUPPORT
    {BDSP_Algorithm_eLpcmEncode, "LPCM Encode", true},
#endif
#ifdef BDSP_MP3ENC_SUPPORT
    {BDSP_Algorithm_eMpegAudioEncode, "MPEG Audio Encode", true},
#endif
#ifdef BDSP_AC4_SUPPORT
		{BDSP_Algorithm_eAC4Decode, "AC4 Decode", true},
#endif
#ifdef BDSP_MPEG_PASSTHRU_SUPPORT
	{BDSP_Algorithm_eMpegAudioPassthrough, "MPEG Passthru", true},
#endif
#ifdef BDSP_LPCMDVD_SUPPORT
	{BDSP_Algorithm_eLpcmDvdDecode, "Dvd Lpcm Decode", true},
	{BDSP_Algorithm_eLpcm1394Decode,"Lpcm 1394 Decode", true},
	{BDSP_Algorithm_eLpcmBdDecode,  "BD Lpcm Decode", true},
#endif
#ifdef BDSP_WMAPRO_SUPPORT
	{BDSP_Algorithm_eWmaProDecode, "WMA Pro Decode", true},
#endif
#ifdef BDSP_WMA_SUPPORT
	{BDSP_Algorithm_eWmaStdDecode, "WMA Decode", true},
#endif
#ifdef BDSP_OUTPUTFORMATTER_SUPPORT
	{BDSP_Algorithm_eOutputFormatter, "Output Formatter", true},
#endif
#ifdef BDSP_DTSHD_PASSTHRU_SUPPORT
	{BDSP_Algorithm_eDtsHdPassthrough, "DTSHD Passthru", true},
#endif

#ifdef BDSP_DTS_PASSTHRU_SUPPORT
	{BDSP_Algorithm_eDts14BitPassthrough, "DTS Passthru", true},
#endif
	{BDSP_Algorithm_eMax, "Invalid", false}
};

static const BDSP_P_AlgorithmCodeInfo BDSP_sRaagaAlgorithmCodeInfo[] =
{
	{
		/* Algorithm */
		BDSP_Algorithm_eMpegAudioDecode,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_MPEG1_SCRATCH_SIZE, BDSP_IMG_ADEC_MPEG1_TABLES_SIZE,
		/* interframe size */                    /*  Compressed interframe size */
		BDSP_IMG_ADEC_MPEG1_INTER_FRAME_SIZE, BDSP_IMG_ADEC_MPEG1_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_MPEG1_SIZE,        BDSP_IMG_AIDS_MPEG1_SIZE,
		/* Code Lib name*/                /* IDS Code Lib name*/
		"/libadec_mpeg1.so",               "/libaids_mpeg1.so"
	},
	{
		BDSP_Algorithm_eLpcmDvdDecode,
		BDSP_IMG_ADEC_LPCM_SCRATCH_SIZE, BDSP_IMG_ADEC_LPCM_TABLES_SIZE,
		BDSP_IMG_ADEC_LPCM_INTER_FRAME_SIZE, BDSP_IMG_ADEC_LPCM_INTER_FRAME_ENCODED_SIZE,
		BDSP_IMG_ADEC_LPCM_SIZE,        BDSP_IMG_AIDS_LPCM_SIZE,
		"/libadec_lpcm.so",               "/libaids_lpcm.so"
	},
	{
		BDSP_Algorithm_eLpcm1394Decode,
		BDSP_IMG_ADEC_LPCM_SCRATCH_SIZE, BDSP_IMG_ADEC_LPCM_TABLES_SIZE,
		BDSP_IMG_ADEC_LPCM_INTER_FRAME_SIZE, BDSP_IMG_ADEC_LPCM_INTER_FRAME_ENCODED_SIZE,
		BDSP_IMG_ADEC_LPCM_SIZE,        BDSP_IMG_AIDS_LPCM_SIZE,
		"/libadec_lpcm.so",               "/libaids_lpcm.so"
	},
	{
		BDSP_Algorithm_eLpcmBdDecode,
		BDSP_IMG_ADEC_LPCM_SCRATCH_SIZE, BDSP_IMG_ADEC_LPCM_TABLES_SIZE,
		BDSP_IMG_ADEC_LPCM_INTER_FRAME_SIZE, BDSP_IMG_ADEC_LPCM_INTER_FRAME_ENCODED_SIZE,
		BDSP_IMG_ADEC_LPCM_SIZE,        BDSP_IMG_AIDS_LPCM_SIZE,
		"/libadec_lpcm.so",               "/libaids_lpcm.so"
	},
#ifdef BDSP_MS11PLUS_SUPPORT
	{
		/* Algorithm */
		BDSP_Algorithm_eUdcDecode,
		/* Scratch buffer size */			/* rom table size */
		BDSP_IMG_ADEC_MS11PLUS_UDC_SCRATCH_SIZE, BDSP_IMG_ADEC_MS11PLUS_UDC_TABLES_SIZE,
		/* interframe size */					/* Compressed interframe size */
		BDSP_IMG_ADEC_MS11PLUS_UDC_INTER_FRAME_SIZE, BDSP_IMG_ADEC_MS11PLUS_UDC_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 		   /* IDS codeSize*/
		BDSP_IMG_ADEC_MS11PLUS_UDC_SIZE, 	   BDSP_IMG_AIDS_DDP_SIZE,
		/* Code Lib name*/				  /* IDS Code Lib name*/
		"/libadec_ms11plus_udc.so",				 "/libaids_ddp.so"
	},
#else
	{
		/* Algorithm */
		BDSP_Algorithm_eUdcDecode,
		/* Scratch buffer size */			/* rom table size */
		BDSP_IMG_ADEC_UDC_SCRATCH_SIZE,	BDSP_IMG_ADEC_UDC_TABLES_SIZE,
		/* interframe size */					/* Compressed interframe size */
		BDSP_IMG_ADEC_UDC_INTER_FRAME_SIZE, BDSP_IMG_ADEC_UDC_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_UDC_SIZE,        BDSP_IMG_AIDS_DDP_SIZE,
		/* Code Lib name*/                /* IDS Code Lib name*/
		"/libadec_udc.so",               "/libaids_ddp.so"
	},
#endif
    {
        /* Algorithm */
        BDSP_Algorithm_eUdcPassthrough,
        /* Scratch buffer size */                /* rom table size */
        BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
        /* interframe size */                       /*Compressed interframe size */
        BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/            /* IDS codeSize*/
        BDSP_IMG_ADEC_PASSTHRU_SIZE,     BDSP_IMG_AIDS_DDP_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_passthru.so",           "/libaids_ddp.so"
    },
	{
		/* Algorithm */
		BDSP_Algorithm_eAc3Decode,
		/* Scratch buffer size */			/* rom table size */
		BDSP_IMG_ADEC_AC3_SCRATCH_SIZE,	BDSP_IMG_ADEC_AC3_TABLES_SIZE,
		/* interframe size */					/*Compressed interframe size */
		BDSP_IMG_ADEC_AC3_INTER_FRAME_SIZE, BDSP_IMG_ADEC_AC3_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_AC3_SIZE,        BDSP_IMG_AIDS_DDP_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_ac3.so",               "/libaids_ddp.so"
	},
	{
		/* Algorithm */
		BDSP_Algorithm_eAc3Passthrough,
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		/* interframe size */                       /*Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 		   /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	 BDSP_IMG_AIDS_DDP_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_passthru.so",           "/libaids_ddp.so"

	},
	{
		/* Algorithm */
		BDSP_Algorithm_eAc3PlusDecode,
		/* Scratch buffer size */           /* rom table size */
		BDSP_IMG_ADEC_DDP_SCRATCH_SIZE, BDSP_IMG_ADEC_DDP_TABLES_SIZE,
		/* interframe size */					/*Compressed interframe size */
		BDSP_IMG_ADEC_DDP_INTER_FRAME_SIZE, BDSP_IMG_ADEC_DDP_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_DDP_SIZE,        BDSP_IMG_AIDS_DDP_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_ddp.so",               "/libaids_ddp.so"

	},
	{
		/* Algorithm */
		BDSP_Algorithm_eAc3PlusPassthrough,
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		/* interframe size */						 /*Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 		   /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	 BDSP_IMG_AIDS_DDP_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_passthru.so",           "/libaids_ddp.so"
	},
#ifdef BDSP_MS11PLUS_SUPPORT
	{
		/* Algorithm */
		BDSP_Algorithm_eDolbyAacheAdtsDecode,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_SCRATCH_SIZE, BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_TABLES_SIZE,
		/* interframe size */					  /*Compressed interframe size */
		BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_INTER_FRAME_SIZE, BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 		   /* IDS codeSize*/
		BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_SIZE, 	   BDSP_IMG_AIDS_ADTS_SIZE,
		/* Code Lib name*/				  /* IDS Code Lib name*/
		"/libadec_ms11plus_dolby_aache.so",				"/libaids_adts.so"
	},
	{
		/* Algorithm */
		BDSP_Algorithm_eDolbyAacheLoasDecode,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_SCRATCH_SIZE, BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_TABLES_SIZE,
		/* interframe size */				      /* Compressed interframe size */
		BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_INTER_FRAME_SIZE, BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_MS11PLUS_DOLBY_AACHE_SIZE,        BDSP_IMG_AIDS_LOAS_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_ms11plus_dolby_aache.so",              "/libaids_loas.so"
	},
#else
	{
		/* Algorithm */
		BDSP_Algorithm_eDolbyAacheAdtsDecode,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_DOLBY_AACHE_SCRATCH_SIZE, BDSP_IMG_ADEC_DOLBY_AACHE_TABLES_SIZE,
		/* interframe size */				      /*Compressed interframe size */
		BDSP_IMG_ADEC_DOLBY_AACHE_INTER_FRAME_SIZE, BDSP_IMG_ADEC_DOLBY_AACHE_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_DOLBY_AACHE_SIZE,        BDSP_IMG_AIDS_ADTS_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_dolby_aache.so",              "/libaids_adts.so"
	},
	{
		/* Algorithm */
		BDSP_Algorithm_eDolbyAacheLoasDecode,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_DOLBY_AACHE_SCRATCH_SIZE, BDSP_IMG_ADEC_DOLBY_AACHE_TABLES_SIZE,
		/* interframe size */				      /* Compressed interframe size */
		BDSP_IMG_ADEC_DOLBY_AACHE_INTER_FRAME_SIZE, BDSP_IMG_ADEC_DOLBY_AACHE_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_DOLBY_AACHE_SIZE,        BDSP_IMG_AIDS_LOAS_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_dolby_aache.so",              "/libaids_loas.so"
	},
#endif
	{
		/* Algorithm */
		BDSP_Algorithm_eAacAdtsDecode,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_AACHE_SCRATCH_SIZE, BDSP_IMG_ADEC_AACHE_TABLES_SIZE,
		/* interframe size */				      /*Compressed interframe size */
		BDSP_IMG_ADEC_AACHE_INTER_FRAME_SIZE, BDSP_IMG_ADEC_AACHE_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_AACHE_SIZE,        BDSP_IMG_AIDS_ADTS_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_aache.so",              "/libaids_adts.so"
	},
	{
		/* Algorithm */
		BDSP_Algorithm_eAacAdtsPassthrough,
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		/* interframe size */				  	     /* Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 		   /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	 BDSP_IMG_AIDS_ADTS_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_passthru.so",           "/libaids_adts.so"
	},
	{
		/* Algorithm */
		BDSP_Algorithm_eAacLoasDecode,
		/* Scratch buffer size */			  /* rom table size */
		BDSP_IMG_ADEC_AACHE_SCRATCH_SIZE, BDSP_IMG_ADEC_AACHE_TABLES_SIZE,
		/* interframe size */				      /* Compressed interframe size */
		BDSP_IMG_ADEC_AACHE_INTER_FRAME_SIZE, BDSP_IMG_ADEC_AACHE_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_AACHE_SIZE,        BDSP_IMG_AIDS_LOAS_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_aache.so",              "/libaids_loas.so"
	},
	{
		/* Algorithm */
		BDSP_Algorithm_eAacLoasPassthrough,
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		 /* interframe size */ 					 	 /* Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			 /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	   BDSP_IMG_AIDS_LOAS_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_passthru.so",             "/libaids_loas.so"
	},
	{
		/* Algorithm */
		BDSP_Algorithm_ePcmWavDecode,
		/* Scratch buffer size */			   /* rom table size */
		BDSP_IMG_ADEC_PCMWAV_SCRATCH_SIZE, BDSP_IMG_ADEC_PCMWAV_TABLES_SIZE,
		/* interframe size */					   /*Compressed interframe size */
		BDSP_IMG_ADEC_PCMWAV_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PCMWAV_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_PCMWAV_SIZE,        BDSP_IMG_AIDS_WAVFORMATEX_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_pcmwav.so",              "/libaids_wavformatex.so"

	},
	{
		/* Algorithm */
		BDSP_Algorithm_eFlacDecode,
		/* Scratch buffer size */			   /* rom table size */
		BDSP_IMG_ADEC_FLAC_SCRATCH_SIZE, BDSP_IMG_ADEC_FLAC_TABLES_SIZE,
		/* interframe size */					   /*Compressed interframe size */
		BDSP_IMG_ADEC_FLAC_INTER_FRAME_SIZE, BDSP_IMG_ADEC_FLAC_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			   /* IDS codeSize*/
		BDSP_IMG_ADEC_FLAC_SIZE,        BDSP_IMG_AIDS_WAVFORMATEX_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_flac.so",              "/libaids_wavformatex.so"

	},
	{
		/* Algorithm */
		BDSP_Algorithm_eSrc,
		/* Scratch buffer size */		   /* Rom table size */
		BDSP_IMG_APP_SRC_SCRATCH_SIZE, BDSP_IMG_APP_SRC_TABLES_SIZE,
		/* Interframe size */  			  /* Compressed Interframe size */
		BDSP_IMG_APP_SRC_INTER_FRAME_SIZE, BDSP_IMG_APP_SRC_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/		  /* IDS codeSize*/
		BDSP_IMG_APP_SRC_SIZE,		 0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libapp_src.so",              "INVALID"
	},
#ifdef BDSP_MS11PLUS_SUPPORT
	{
		/* Algorithm */
		BDSP_Algorithm_eMixerDapv2,
		/* Scratch buffer size */		   /* Rom table size */
		BDSP_IMG_APP_MS11PLUS_MIXER_SCRATCH_SIZE, BDSP_IMG_APP_MS11PLUS_MIXER_TABLES_SIZE,
		/* Interframe size */			  /* Compressed Interframe size */
		BDSP_IMG_APP_MS11PLUS_MIXER_INTER_FRAME_SIZE, BDSP_IMG_APP_MS11PLUS_MIXER_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 	  /* IDS codeSize*/
		BDSP_IMG_APP_MS11PLUS_MIXER_SIZE,		 0,
		/* Code Lib name*/				  /* IDS Code Lib name*/
		"/libapp_ms11plus_mixer.so",			"INVALID"
	},
#else
	{
		/* Algorithm */
		BDSP_Algorithm_eMixerDapv2,
		/* Scratch buffer size */		   /* Rom table size */
		BDSP_IMG_APP_MIXER_DAPV2_SCRATCH_SIZE, BDSP_IMG_APP_MIXER_DAPV2_TABLES_SIZE,
		/* Interframe size */  			  /* Compressed Interframe size */
		BDSP_IMG_APP_MIXER_DAPV2_INTER_FRAME_SIZE, BDSP_IMG_APP_MIXER_DAPV2_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/		  /* IDS codeSize*/
		BDSP_IMG_APP_MIXER_DAPV2_SIZE,		 0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libapp_mixer_dapv2.so",           "INVALID"
    },
#endif
	{
		/* Algorithm */
		BDSP_Algorithm_eMixer,
		/* Scratch buffer size */		   /* Rom table size */
		BDSP_IMG_APP_MIXER_DAPV2_SCRATCH_SIZE, BDSP_IMG_APP_MIXER_DAPV2_TABLES_SIZE,
		/* Interframe size */  			  /* Compressed Interframe size */
		BDSP_IMG_APP_MIXER_DAPV2_INTER_FRAME_SIZE, BDSP_IMG_APP_MIXER_DAPV2_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/		  /* IDS codeSize*/
		BDSP_IMG_APP_MIXER_DAPV2_SIZE,		 0,
        /* Code Lib name*/                /* IDS Code Lib name*/
		"/libapp_fw_mixer.so",           "INVALID"

    },
#ifdef BDSP_MS11PLUS_SUPPORT
    {
		/* Algorithm */
		BDSP_Algorithm_eDpcmr,
		/* Scratch buffer size */		   /* Rom table size */
		BDSP_IMG_APP_MS11PLUS_DPCMR_SCRATCH_SIZE, BDSP_IMG_APP_MS11PLUS_DPCMR_TABLES_SIZE,
		/* Interframe size */  			  /* Compressed Interframe size */
		BDSP_IMG_APP_MS11PLUS_DPCMR_INTER_FRAME_SIZE, BDSP_IMG_APP_MS11PLUS_DPCMR_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/		  /* IDS codeSize*/
		BDSP_IMG_APP_MS11PLUS_DPCMR_SIZE,		 0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libapp_ms11plus_dpcmr.so",               "INVALID"
	},
	{
		/* Algorithm */
		BDSP_Algorithm_eDDPEncode,
		/* Scratch buffer size */		  /* Rom table size */
		BDSP_IMG_AENC_MS11PLUS_DD_SCRATCH_SIZE, BDSP_IMG_AENC_MS11PLUS_DD_TABLES_SIZE,
		/* Interframe size */  		  /* Compressed Interframe size */
		BDSP_IMG_AENC_MS11PLUS_DD_INTER_FRAME_SIZE, BDSP_IMG_AENC_MS11PLUS_DD_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/		  /* IDS codeSize*/
		BDSP_IMG_AENC_MS11PLUS_DD_SIZE,		    0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libaenc_ms11plus_dd.so",               "INVALID"
	},
#else
	{
		/* Algorithm */
		BDSP_Algorithm_eDpcmr,
		/* Scratch buffer size */		   /* Rom table size */
		BDSP_IMG_APP_DPCMR_SCRATCH_SIZE, BDSP_IMG_APP_DPCMR_TABLES_SIZE,
		/* Interframe size */			  /* Compressed Interframe size */
		BDSP_IMG_APP_DPCMR_INTER_FRAME_SIZE, BDSP_IMG_APP_DPCMR_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 	  /* IDS codeSize*/
		BDSP_IMG_APP_DPCMR_SIZE,		 0,
		/* Code Lib name*/				  /* IDS Code Lib name*/
		"/libapp_dpcmr.so", 			  "INVALID"
	},
	{
		/* Algorithm */
		BDSP_Algorithm_eDDPEncode,
		/* Scratch buffer size */		  /* Rom table size */
		BDSP_IMG_AENC_DDP_SCRATCH_SIZE, BDSP_IMG_AENC_DDP_TABLES_SIZE,
		/* Interframe size */  		  /* Compressed Interframe size */
		BDSP_IMG_AENC_DDP_INTER_FRAME_SIZE, BDSP_IMG_AENC_DDP_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/		  /* IDS codeSize*/
		BDSP_IMG_AENC_DDP_SIZE,		    0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libaenc_ddp.so",               "INVALID"
	},
#endif
    {
        /* Algorithm */
        BDSP_Algorithm_eAacEncode,
        /* Scratch buffer size */		  /* Rom table size */
        BDSP_IMG_AENC_AACHE_SCRATCH_SIZE, BDSP_IMG_AENC_AACHE_TABLES_SIZE,
        /* Interframe size */  		  /* Compressed Interframe size */
        BDSP_IMG_AENC_AACHE_INTER_FRAME_SIZE, BDSP_IMG_AENC_AACHE_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/		  /* IDS codeSize*/
        BDSP_IMG_AENC_AACHE_SIZE,		    0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libaenc_aache.so",               "INVALID"
    },
    {
        /* Algorithm */
        BDSP_Algorithm_eGenCdbItb,
        /* Scratch buffer size */		  /* Rom table size */
        BDSP_IMG_APP_GEN_CDBITB_SCRATCH_SIZE, BDSP_IMG_APP_GEN_CDBITB_TABLES_SIZE,
        /* Interframe size */  		  /* Compressed Interframe size */
        BDSP_IMG_APP_GEN_CDBITB_INTER_FRAME_SIZE, BDSP_IMG_APP_GEN_CDBITB_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/		  /* IDS codeSize*/
        BDSP_IMG_APP_GEN_CDBITB_SIZE,		    0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libapp_gen_cdbitb.so",               "INVALID"
    },
    {
        /* Algorithm */
        BDSP_Algorithm_eDsola,
        /* Scratch buffer size */         /* Rom table size */
        BDSP_IMG_APP_DSOLA_SCRATCH_SIZE, BDSP_IMG_APP_DSOLA_TABLES_SIZE,
        /* Interframe size */         /* Compressed Interframe size */
        BDSP_IMG_APP_DSOLA_INTER_FRAME_SIZE, BDSP_IMG_APP_DSOLA_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/       /* IDS codeSize*/
        BDSP_IMG_APP_DSOLA_SIZE,           0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libapp_dsola.so",               "INVALID"
    },
    {
        /* Algorithm */
        BDSP_Algorithm_eOpusDecode,
        /* Scratch buffer size */			  /* rom table size */
        BDSP_IMG_ADEC_OPUS_SCRATCH_SIZE, BDSP_IMG_ADEC_OPUS_TABLES_SIZE,
        /* interframe size */                    /*  Compressed interframe size */
        BDSP_IMG_ADEC_OPUS_INTER_FRAME_SIZE, BDSP_IMG_ADEC_OPUS_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/			   /* IDS codeSize*/
        BDSP_IMG_ADEC_OPUS_SIZE,            BDSP_IMG_AIDS_WAVFORMATEX_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_opus.so",               "/libaids_wavformatex.so"
    },
	{
        /* Algorithm */
        BDSP_Algorithm_eALSDecode,
        /* Scratch buffer size */			  /* rom table size */
        BDSP_IMG_ADEC_ALS_SCRATCH_SIZE, BDSP_IMG_ADEC_ALS_TABLES_SIZE,
        /* interframe size */                    /*  Compressed interframe size */
        BDSP_IMG_ADEC_ALS_INTER_FRAME_SIZE, BDSP_IMG_ADEC_ALS_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/			   /* IDS codeSize*/
        BDSP_IMG_ADEC_ALS_SIZE,            BDSP_IMG_AIDS_WAVFORMATEX_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_als.so",               "/libaids_wavformatex.so"
    },
	{
        /* Algorithm */
        BDSP_Algorithm_eALSLoasDecode,
        /* Scratch buffer size */			  /* rom table size */
        BDSP_IMG_ADEC_ALS_SCRATCH_SIZE, BDSP_IMG_ADEC_ALS_TABLES_SIZE,
        /* interframe size */                    /*  Compressed interframe size */
        BDSP_IMG_ADEC_ALS_INTER_FRAME_SIZE, BDSP_IMG_ADEC_ALS_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/			   /* IDS codeSize*/
        BDSP_IMG_ADEC_ALS_SIZE,            BDSP_IMG_AIDS_LOAS_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_als.so",               "/libaids_loas.so"
    },
  {
		/* Algorithm */
		BDSP_Algorithm_eLpcmEncode,
		/* Scratch buffer size */		  /* Rom table size */
		BDSP_IMG_AENC_LPCM_SCRATCH_SIZE, BDSP_IMG_AENC_LPCM_TABLES_SIZE,
		/* Interframe size */  		  /* Compressed Interframe size */
		BDSP_IMG_AENC_LPCM_INTER_FRAME_SIZE, BDSP_IMG_AENC_LPCM_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/		  /* IDS codeSize*/
		BDSP_IMG_AENC_LPCM_SIZE,		    0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libaenc_lpcm.so",               "INVALID"
	},
  {
		/* Algorithm */
		BDSP_Algorithm_eMpegAudioEncode,
		/* Scratch buffer size */		  /* Rom table size */
		BDSP_IMG_AENC_MP3_SCRATCH_SIZE, BDSP_IMG_AENC_MP3_TABLES_SIZE,
		/* Interframe size */  		  /* Compressed Interframe size */
		BDSP_IMG_AENC_MP3_INTER_FRAME_SIZE, BDSP_IMG_AENC_MP3_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/		  /* IDS codeSize*/
		BDSP_IMG_AENC_MP3_SIZE,		    0,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libaenc_mp3.so",               "INVALID"
	},
    {
        /* Algorithm */
        BDSP_Algorithm_eAC4Decode,
        /* Scratch buffer size */         /* Rom table size */
        BDSP_IMG_ADEC_AC4_SCRATCH_SIZE, BDSP_IMG_ADEC_AC4_TABLES_SIZE,
        /* Interframe size */         /* Compressed Interframe size */
        BDSP_IMG_ADEC_AC4_INTER_FRAME_SIZE, BDSP_IMG_ADEC_AC4_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/       /* IDS codeSize*/
        BDSP_IMG_ADEC_AC4_SIZE,           BDSP_IMG_AIDS_AC4_SIZE,
        /* Code Lib name*/                /* IDS Code Lib name*/
        "/libadec_ac4.so",               "/libaids_ac4.so",
    },
	{
		/* Algorithm */
		BDSP_Algorithm_eMpegAudioPassthrough,
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		 /* interframe size */ 					 	 /* Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			 /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	   BDSP_IMG_AIDS_MPEG1_SIZE,
	/* Code Lib name*/                /* IDS Code Lib name*/
	"/libadec_passthru.so",             "/libaids_mpeg1.so"

	},
	{
		/* Algorithm */
		BDSP_Algorithm_eWmaStdDecode,
		/* Scratch buffer size */				/* Rom table size */
		BDSP_IMG_ADEC_WMA_SCRATCH_SIZE,		BDSP_IMG_ADEC_WMA_TABLES_SIZE,
		/* Interframe size */					/* Compressed Interframe size */
		BDSP_IMG_ADEC_WMA_INTER_FRAME_SIZE,	BDSP_IMG_ADEC_WMA_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/ 				/* IDS codeSize*/
		BDSP_IMG_ADEC_WMA_SIZE,				BDSP_IMG_AIDS_WMA_SIZE,
		/* Code Lib name*/						/* IDS Code Lib name*/
		"/libadec_wma.so",					"/libaids_wma.so",
	},
    {
        /* Algorithm */
        BDSP_Algorithm_eWmaProDecode,
        /* Scratch buffer size */               /* Rom table size */
        BDSP_IMG_ADEC_WMA_PRO_SCRATCH_SIZE,      BDSP_IMG_ADEC_WMA_PRO_TABLES_SIZE,
        /* Interframe size */                   /* Compressed Interframe size */
        BDSP_IMG_ADEC_WMA_PRO_INTER_FRAME_SIZE,  BDSP_IMG_ADEC_WMA_PRO_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/                 /* IDS codeSize*/
        BDSP_IMG_ADEC_WMA_PRO_SIZE,              BDSP_IMG_AIDS_WMA_PRO_SIZE,
        /* Code Lib name*/                      /* IDS Code Lib name*/
        "/libadec_wma_pro.so",                   "/libaids_wma_pro.so",
    },
    {
        /* Algorithm */
        BDSP_Algorithm_eOutputFormatter,
        /* Scratch buffer size */               /* Rom table size */
        BDSP_IMG_APP_OUTPUTFORMATTER_SCRATCH_SIZE,      BDSP_IMG_APP_OUTPUTFORMATTER_TABLES_SIZE,
        /* Interframe size */                   /* Compressed Interframe size */
        BDSP_IMG_APP_OUTPUTFORMATTER_INTER_FRAME_SIZE,  BDSP_IMG_APP_OUTPUTFORMATTER_INTER_FRAME_ENCODED_SIZE,
        /* Algorithm codeSize*/                 /* IDS codeSize*/
        BDSP_IMG_APP_OUTPUTFORMATTER_SIZE,              0,
        /* Code Lib name*/                      /* IDS Code Lib name*/
        "/libapp_outputformatter.so",                   "INVALID",
    },
    {
		/* Algorithm */
		BDSP_Algorithm_eDtsHdPassthrough,
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		 /* interframe size */ 					 	 /* Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			 /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	   BDSP_IMG_AIDS_DTSHD_SIZE,
	/* Code Lib name*/                /* IDS Code Lib name*/
	"/libadec_passthru.so",             "/libaids_dtshd.so"

    },
    {
		/* Algorithm */
		BDSP_Algorithm_eDts14BitPassthrough,
		/* Scratch buffer size */				 /* rom table size */
		BDSP_IMG_ADEC_PASSTHRU_SCRATCH_SIZE, BDSP_IMG_ADEC_PASSTHRU_TABLES_SIZE,
		 /* interframe size */ 					 	 /* Compressed interframe size */
		BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_SIZE, BDSP_IMG_ADEC_PASSTHRU_INTER_FRAME_ENCODED_SIZE,
		/* Algorithm codeSize*/			 /* IDS codeSize*/
		BDSP_IMG_ADEC_PASSTHRU_SIZE,	   BDSP_IMG_AIDS_DTS_SIZE,
	/* Code Lib name*/                /* IDS Code Lib name*/
	"/libadec_passthru.so",             "/libaids_dts.so"

    },
    /* This entry must always be last used to derive the unsupported/invalid information */
	{
		BDSP_Algorithm_eMax,
		0, 0,
		0, 0,
		0, 0,
		"INVALID", "INVALID"
	}
};

const BDSP_P_AlgorithmCodeInfo *BDSP_Raaga_P_LookupAlgorithmCodeInfo(
    BDSP_Algorithm algorithm
)
{
  unsigned i;

  for ( i = 0; BDSP_sRaagaAlgorithmCodeInfo[i].algorithm != BDSP_Algorithm_eMax; i++ )
  {
    if ( BDSP_sRaagaAlgorithmCodeInfo[i].algorithm == algorithm )
    {
      break;
    }
    }

    return &BDSP_sRaagaAlgorithmCodeInfo[i];
}

const BDSP_P_AlgorithmSupportInfo *BDSP_Raaga_P_LookupAlgorithmSupportInfo_isrsafe(
    BDSP_Algorithm algorithm
)
{
	unsigned i;

	for (i = 0; BDSP_sRaagaAlgorithmSupportInfo[i].algorithm != BDSP_Algorithm_eMax; i++ )
	{
		if ( BDSP_sRaagaAlgorithmSupportInfo[i].algorithm == algorithm )
		{
		  break;
		}
	}
	return &BDSP_sRaagaAlgorithmSupportInfo[i];
}
