/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/
/* Nexus example app: play and decode MP3 file to audio dacs */
#include "nexus_platform.h"
#if NEXUS_NUM_SPDIF_OUTPUTS && NEXUS_NUM_AUDIO_DACS
#include "nexus_memory.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_output_group.h"
#include "nexus_audio_input.h"
#include "nexus_stc_channel.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"

#include "nexus_display.h"

#ifdef NEXUS_MS11_SUPPORT
	#define NEXUS_DDRE_ENABLE (1)
	#define NEXUS_DV258_ENABLE (1)
#endif

#ifdef NEXUS_3DSURROUND_ENABLE
    #include "nexus_3d_surround.h"
#endif

#ifdef CUSTOM_VOICE_ENABLE
	#include "nexus_custom_voice.h"
#endif
#ifdef NEXUS_DOLBY_VOLUME_ENABLE
	#include "nexus_dolby_volume.h"
#endif
#ifdef NEXUS_DDRE_ENABLE
	#include "nexus_dolby_digital_reencode.h"
#endif
#include "nexus_dts_encode.h"
#include "nexus_ac3_encode.h"
#include "nexus_audio_mixer.h"

#ifdef EQUALIZER_SUPPORT
	#include "nexus_audio_equalizer.h"
#endif

#if (defined (NEXUS_ADV_ENABLE) || defined (NEXUS_ABX_ENABLE))
	#include "nexus_audyssey_adv.h"
	#include "nexus_audyssey_abx.h"
#endif

#ifdef NEXUS_AVL_ENABLE
#include "nexus_auto_volume_level.h"
#endif

#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
	#include "nexus_studio_sound.h"
#else
#ifdef NEXUS_SRSTRUVOL_ENABLE
	#include "nexus_tru_volume.h"
#endif
#ifdef NEXUS_SRSHD_ENABLE
	#include "nexus_tru_surround_hd.h"
#endif
#endif

#ifdef NEXUS_DV258_ENABLE
	#include "nexus_dolby_volume.h"
#endif

#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#include "basf_probe.h"
#include "bavi_probe.h"
#include "bfile_stdio.h"
#include <errno.h>

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

#ifdef BDSP_FW_RBUF_CAPTURE
#include "rbuf_capture.h"
#endif

#define NEXUS_ERR_UNDEFINED	(-1)

typedef struct APP_AUDIO_ALGO_MAP {
	char *str;
	unsigned int val;
}APP_AUDIO_ALGO_MAP;

APP_AUDIO_ALGO_MAP ALGO_MAP_TABLE[NEXUS_AudioCodec_eMax] = {
		{"mpeg", 			NEXUS_AudioCodec_eMpeg},
		{"mp3", 			NEXUS_AudioCodec_eMp3},
		{"aacadts", 		NEXUS_AudioCodec_eAacAdts},
		{"aacloas", 		NEXUS_AudioCodec_eAacLoas},
		{"aac+loas", 		NEXUS_AudioCodec_eAacPlusLoas},
		{"aac+adts", 		NEXUS_AudioCodec_eAacPlusAdts},
		{"ac3", 			NEXUS_AudioCodec_eAc3},
		{"ac3+", 			NEXUS_AudioCodec_eAc3Plus},
		{"dts", 			NEXUS_AudioCodec_eDts},
		{"lpcmdvd", 		NEXUS_AudioCodec_eLpcmDvd},
		{"lpcmhddvd", 		NEXUS_AudioCodec_eLpcmHdDvd},
		{"lpcmbluray", 		NEXUS_AudioCodec_eLpcmBluRay},
		{"dtshd", 			NEXUS_AudioCodec_eDtsHd},
		{"wmastd",			NEXUS_AudioCodec_eWmaStd},
		{"wmastdts",		NEXUS_AudioCodec_eWmaStdTs},		
		{"wmapro", 			NEXUS_AudioCodec_eWmaPro},
		{"avs", 			NEXUS_AudioCodec_eAvs},
		{"pcm", 			NEXUS_AudioCodec_ePcm},
		{"pcmwav", 			NEXUS_AudioCodec_ePcmWav},
		{"amrnb", 			NEXUS_AudioCodec_eAmrNb},
		{"amrwb", 			NEXUS_AudioCodec_eAmrWb},
		{"dra", 			NEXUS_AudioCodec_eDra},
		{"real", 			NEXUS_AudioCodec_eCook},
		{"adpcm", 			NEXUS_AudioCodec_eAdpcm},
		{"sbc",				NEXUS_AudioCodec_eSbc},		
		{"dtscd", 			NEXUS_AudioCodec_eDtsCd},	
		{"dtsexp", 			NEXUS_AudioCodec_eDtsExpress},	
		{"vorbis", 			NEXUS_AudioCodec_eVorbis},	
		{"lpcm1394", 		NEXUS_AudioCodec_eLpcm1394},	
		{"g711", 			NEXUS_AudioCodec_eG711},
		{"g723", 			NEXUS_AudioCodec_eG723_1},
		{"g726", 			NEXUS_AudioCodec_eG726},
		{"g729", 			NEXUS_AudioCodec_eG729},
		{"flac", 			NEXUS_AudioCodec_eFlac},
		{"mlp", 			NEXUS_AudioCodec_eMlp},
		{"ape", 			NEXUS_AudioCodec_eApe},
		{"end",  			NEXUS_AudioCodec_eMax},
};

NEXUS_AudioCodec b_audiocodec2nexus(baudio_format settop_value);
NEXUS_TransportType b_mpegtype2nexus(bstream_mpeg_type settop_value);

volatile unsigned int loop_idx = 1;
static unsigned int string_to_enum(const char *str)
{
	unsigned int i = 0;
	while((i<NEXUS_AudioCodec_eMax)&&(strcmp(str,ALGO_MAP_TABLE[i].str)!= 0))
	{
		i++;
	}
	return ALGO_MAP_TABLE[i].val;
}

static void eof(void *context, int param)
{
	printf ("\n###ITERATION # %d Complete\n\n", loop_idx);
	loop_idx++;
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void sec_eof(void *context, int param)
{
	printf ("\n###Secondary stream has wrapped around\n\n");
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

typedef struct sMs10_ddp_codec_options
{
	NEXUS_AudioDecoderDolbyStereoDownmixMode	dm_mode;
	NEXUS_AudioDecoderDolbyDrcMode				drc_opmode;
	unsigned int	substream_id;
	unsigned int	hdr_cut;
	unsigned int	ldr_boost;
	/* Below three are applicable only for MS11 */
#ifdef NEXUS_MS11_SUPPORT	
	NEXUS_AudioDecoderDolbyDrcMode	dwmix_drc_opmode;	
	unsigned int	dwmix_hdr_cut;
	unsigned int	dwmix_ldr_boost;
#endif
} sMs10_ddp_codec_options;

typedef struct sMs10_dolby_pulse_options
{
	NEXUS_AudioDecoderAacDownmixMode			dm_mode;
	NEXUS_AudioDecoderDolbyPulseDrcMode			drc_opmode;
	unsigned int	hdr_cut;
	unsigned int	ldr_boost;
	unsigned int	dialnorm_value;
} sMs10_dolby_pulse_options;

typedef struct sChannel_options
{
	bool		file_playback;
	char		filename[300];
	NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    BKNI_EventHandle event;
	int			pcr_pid;
	int			audio_pid;
	int			num_channels;
	NEXUS_AudioCodec				audio_codec;
	NEXUS_AudioDecoderDualMonoMode	dual_mono_mode;
	NEXUS_AudioDecoderOutputMode	dec_out_mode;
	NEXUS_AudioDecoderOutputLfeMode	lfe_out;
} sChannel_options;

typedef enum EChannelType
{
	eChannelTypePrimary = 0,
	eChannelTypeAssociated,
	eChannelTypeLast,
	eChannelTypeMax = 0xFFFF
} EChannelType;

typedef enum ESpdif_Out
{
	eSpdifOutPcm = 0,
	eSpdifOutCompressed,
	eSpdifOutEncAc3,
	eSpdifOutEncDts,
	eSpdifOutPcmMultiLR,
	eSpdifOutPcmMultiLsRs,
	eSpdifOutPcmMultiCLFE,
	eSpdifOutPcmSimulLR,
	eSpdifOutLast,
	eSpdifOutMax = 0xFFFF
} ESpdif_Out;

typedef enum EHdmi_out
{
	eHdmiOutPcmStereo = 0,
	eHdmiOutPcmMultiChannel,
	eHdmiOutEncAc3,
	eHdmiOutEncDts,
	eHdmiOutCompressed,
	eHdmiOutCompressedHbr,
	eHdmiOutLast,
	eHdmiOutMax = 0xFFFF
} EHdmi_out;

typedef struct sInput_options
{
	sChannel_options ChannelOpts[eChannelTypeLast];
	ESpdif_Out	eSpdif_out;
	EHdmi_out	eHdmi_out;
	bool		custom_voice_enable;
	bool		tru_vol_enable;
	bool		brcm_3d_surr_enable;
	bool		dolby_vol_enable;
	bool		ddre_enable;
	bool		mixer_enable;
	bool		targetsync_enable;
#if NEXUS_NUM_I2S_OUTPUTS > 0
    bool    multi_i2s;
#endif
	int			loop_count;
	char		eq_coeff_file[100];
	NEXUS_PlaybackLoopMode				endOfStreamAction;
	NEXUS_AudioDecoderConnectorType 	encInput;
	bool		stereo_disable;
} sInput_options;

#if (defined (NEXUS_ADV_ENABLE) || defined (NEXUS_ABX_ENABLE))
typedef struct sAudysseyOptions
{
	bool adv_enable;
	struct NEXUS_AudysseyAdvSettings adv;
	struct NEXUS_AudysseyAbxSettings abx;
} sAudysseyOptions;
#endif

#ifdef NEXUS_AVL_ENABLE
typedef struct sAVLOpt
{
	NEXUS_AutoVolumeLevelSettings avl;

} sAVLOpt;
struct sAVLOpt g_AVL_options;
#endif


/* Global Variables */
struct sInput_options g_input_options;
struct sMs10_ddp_codec_options g_ddp_options[eChannelTypeLast];
struct sMs10_dolby_pulse_options g_dolby_pulse_options[eChannelTypeLast];
struct NEXUS_AudioDecoderDtsSettings g_dts_options;
#if (defined (NEXUS_ADV_ENABLE) || defined (NEXUS_ABX_ENABLE))
	struct sAudysseyOptions g_adv_options;
#endif

#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
typedef struct sStudioSoundOpt
{
	unsigned int TsHD_StandAlone;
	unsigned int TVol_StandAlone;
	NEXUS_StudioSoundSettings g_StudioSound_Options;
#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM
	char *TruEQ_Mode0_Coeff_FileName;
	char *HPF_Front_Mode0_Coeff_FileName;
	char *HPF_Back_Mode0_Coeff_FileName;
#endif
} sStudioSoundOpt;
struct sStudioSoundOpt sStudioSoundOptions;

#else

#ifdef NEXUS_SRSTRUVOL_ENABLE
NEXUS_TruVolumeSettings g_srs_tvol_settings;	
#endif
#ifdef NEXUS_SRSHD_ENABLE
NEXUS_TruSurroundHdSettings g_srs_tshd_settings;
#endif

#endif

#ifdef NEXUS_DV258_ENABLE
NEXUS_DolbyVolume258Settings DV258Settings;
NEXUS_DolbyVolume258Handle dv258;
#endif

#ifdef NEXUS_DDRE_ENABLE
NEXUS_DolbyDigitalReencodeSettings DDRE_Settings;
#endif

const char *ChannelString[eChannelTypeLast] = {"PRIMARY", "ASSOCIATED"};
EChannelType CurrentChannelType;
NEXUS_AudioMixerHandle g_mixer;
NEXUS_PlatformConfiguration g_platformConfig;
#ifdef EQUALIZER_SUPPORT
NEXUS_AudioEqualizerHandle g_audioEqualizer;
#endif
NEXUS_StcChannelHandle g_primaryStcChannel;

static int decOrhex_atoi(const char *str)
{
	unsigned int i;
	int hexDigitValue, value = 0, sign = 0;

	if ((strlen(str) > 1) && str[0]=='-')
	{
		sign = 1; /* minus */
		str++;
	}
	if ((strlen(str) > 2) && str[0]=='0' && str[1]=='x')
	{
		str += 2;
		for (i=0; i<strlen(str); i++)
		{
			if (str[i] >= 'a' && str[i] <= 'f')
			{
				hexDigitValue = str[i] - 'a' + 10;
			} 
			else if (str[i] >= 'A' && str[i] <= 'F')
			{
				hexDigitValue = str[i] - 'A' + 10;
			}
			else
			{
				hexDigitValue = str[i] - '0';
			}
			value <<= 4; value |= hexDigitValue;
		}
	}
	else
	{
		value = atoi(str);
	}
	
	if (sign == 1)
	{
		value = -value;
	}
	
	return value;
}


int parse_ms10_ddp_options (char **argv)
{
	int retval = -1;
	
	if ( !strcmp(argv[0], "-downmix_mode") )
	{
		retval = 1;
		if (!strcmp(argv[1], "ltrt"))
		{
			g_ddp_options[CurrentChannelType].dm_mode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible;
		}
		else if (!strcmp(argv[1], "loro"))
		{
			g_ddp_options[CurrentChannelType].dm_mode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard;
		}
		else
		{
			g_ddp_options[CurrentChannelType].dm_mode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic;
		}
	}
	else if ( !strcmp(argv[0], "-substream_id") )
	{
		retval = 1;
		g_ddp_options[CurrentChannelType].substream_id = decOrhex_atoi(argv[1]);
		if (g_ddp_options[CurrentChannelType].substream_id > 7)
		{
			printf("error: illegal substream id value : %d\n", g_ddp_options[CurrentChannelType].substream_id);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-cut") )
	{
		retval = 1;
		g_ddp_options[CurrentChannelType].hdr_cut = atoi(argv[1]);
		if (g_ddp_options[CurrentChannelType].hdr_cut > 100)
		{
			printf("error: illegal DRC hdr cut factor : %d\n",g_ddp_options[CurrentChannelType].hdr_cut);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-boost") )
	{
		retval = 1;
		g_ddp_options[CurrentChannelType].ldr_boost = atoi(argv[1]);
		if (g_ddp_options[CurrentChannelType].ldr_boost > 100)
		{
			printf("error: illegal DRC ldr boost factor : %d\n", g_ddp_options[CurrentChannelType].ldr_boost);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-drc_mode") )
	{
		retval = 1;
		if (strcmp(argv[1], "line") == 0)
		{
			g_ddp_options[CurrentChannelType].drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
		}
		else if (strcmp(argv[1], "rf") == 0)
		{
			g_ddp_options[CurrentChannelType].drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
		}
		else if (strcmp(argv[1], "custA") == 0)
		{
			g_ddp_options[CurrentChannelType].drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eCustomA;
		}
		else if (strcmp(argv[1], "custD") == 0)
		{
			g_ddp_options[CurrentChannelType].drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eCustomD;
		}
#ifdef NEXUS_MS11_SUPPORT
		else if (strcmp(argv[1], "off") == 0)
		{
			g_ddp_options[CurrentChannelType].drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eOff;
		}
#endif
		else
		{
			printf("error: illegal compression mode specifier : %s\n", argv[1]);
			retval = -1;
		}
	}
#ifdef NEXUS_MS11_SUPPORT
	/* Below three are applicable only for MS11 */
	else if ( !strcmp(argv[0], "-dwmix_cut") )
	{
		retval = 1;
		g_ddp_options[CurrentChannelType].dwmix_hdr_cut = atoi(argv[1]);
		
		if (g_ddp_options[CurrentChannelType].dwmix_hdr_cut > 100)
		{
			printf("error: illegal DRC hdr cut factor : %d\n",g_ddp_options[CurrentChannelType].dwmix_hdr_cut);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-dwmix_boost") )
	{
		retval = 1;
		g_ddp_options[CurrentChannelType].dwmix_ldr_boost = atoi(argv[1]);
		if (g_ddp_options[CurrentChannelType].dwmix_ldr_boost > 100)
		{
			printf("error: illegal DRC ldr boost factor : %d\n", g_ddp_options[CurrentChannelType].dwmix_ldr_boost);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-dwmix_drc_mode") )
	{
		retval = 1;
		if (strcmp(argv[1], "line") == 0)
		{
			g_ddp_options[CurrentChannelType].dwmix_drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
		}
		else if (strcmp(argv[1], "rf") == 0)
		{
			g_ddp_options[CurrentChannelType].dwmix_drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
		}
		else if (strcmp(argv[1], "custA") == 0)
		{
			g_ddp_options[CurrentChannelType].dwmix_drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eCustomA;
		}
		else if (strcmp(argv[1], "custD") == 0)
		{
			g_ddp_options[CurrentChannelType].dwmix_drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eCustomD;
		}
		else
		{
			printf("error: illegal compression mode specifier : %s\n", argv[1]);
			retval = -1;
		}
	}
#endif
	else
	{
		retval = -1;
	}
	
	return retval;
}

int parse_ms10_dolby_plus_options (char **argv)
{
	int retval = -1;

	if ( !strcmp(argv[0], "-downmix_mode") )
	{
		retval = 1;
		if (!strcmp(argv[1], "matrix"))
		{
			g_dolby_pulse_options[CurrentChannelType].dm_mode = NEXUS_AudioDecoderAacDownmixMode_eMatrix;
		}
		else if (!strcmp(argv[1], "arib"))
		{
			g_dolby_pulse_options[CurrentChannelType].dm_mode = NEXUS_AudioDecoderAacDownmixMode_eArib;
		}
		else if (!strcmp(argv[1], "ltrt"))
		{
			g_dolby_pulse_options[CurrentChannelType].dm_mode = NEXUS_AudioDecoderAacDownmixMode_eLtRt;
		}
		else if (!strcmp(argv[1], "loro"))
		{
			g_dolby_pulse_options[CurrentChannelType].dm_mode = NEXUS_AudioDecoderAacDownmixMode_eLoRo;
		}
		else
		{
			printf ("error: illegal option for downmix mode : %s\n", argv[1]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-cut") )
	{
		retval = 1;
		g_dolby_pulse_options[CurrentChannelType].hdr_cut = atoi(argv[1]);
		if (g_dolby_pulse_options[CurrentChannelType].hdr_cut > 100)
		{
			printf("error: illegal DRC hdr cut factor : %d\n", g_dolby_pulse_options[CurrentChannelType].hdr_cut);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-boost") )
	{
		retval = 1;
		g_dolby_pulse_options[CurrentChannelType].ldr_boost = atoi(argv[1]);
		if (g_dolby_pulse_options[CurrentChannelType].ldr_boost > 100)
		{
			printf("error: illegal DRC ldr boost factor : %d\n", g_dolby_pulse_options[CurrentChannelType].ldr_boost);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-drc_mode") )
	{
		retval = 1;
		if (strcmp(argv[1], "line") == 0)
		{
			g_dolby_pulse_options[CurrentChannelType].drc_opmode = NEXUS_AudioDecoderDolbyPulseDrcMode_eLine;
		}
		else if (strcmp(argv[1], "rf") == 0)
		{
			g_dolby_pulse_options[CurrentChannelType].drc_opmode = NEXUS_AudioDecoderDolbyPulseDrcMode_eRf;
		}
		else
		{
			printf("error: illegal compression mode specifier : %s\n", argv[1]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-dn") )
	{
		retval = 1;
		g_dolby_pulse_options[CurrentChannelType].dialnorm_value = atoi(argv[1]);
		if (g_dolby_pulse_options[CurrentChannelType].dialnorm_value > 127)
		{
			printf("error: illegal dialnorm value : %d\n", g_dolby_pulse_options[CurrentChannelType].dialnorm_value);
			retval = -1;
		}
	}
	else
	{
		retval = -1;
	}
	
	return retval;
}

int parse_dts_options (char **argv)
{
	int retval = -1;
	
	if ( !strcmp(argv[0], "-downmix_mode") )
	{
		retval = 1;
		if (!strcmp(argv[1], "ltrt"))
		{
			g_dts_options.stereoDownmixMode = NEXUS_AudioDecoderDtsDownmixMode_eLtRt;
		}
		else if (!strcmp(argv[1], "loro"))
		{
			g_dts_options.stereoDownmixMode = NEXUS_AudioDecoderDtsDownmixMode_eLoRo;
		}
		else
		{
			g_dts_options.stereoDownmixMode = NEXUS_AudioDecoderDtsDownmixMode_eAuto;
		}
	}
	else if ( !strcmp(argv[0], "-cut") )
	{
		retval = 1;
		g_dts_options.cut = atoi(argv[1]);
		if (g_dts_options.cut > 100)
		{
			printf("error: illegal DRC hdr cut factor : %d\n", g_dts_options.cut);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-boost") )
	{
		retval = 1;
		g_dts_options.boost = atoi(argv[1]);
		if (g_dts_options.boost > 100)
		{
			printf("error: illegal DRC ldr boost factor : %d\n", g_dts_options.boost);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-mix_lfe") )
	{
		retval = 0;
		g_dts_options.mixLfeToPrimary = true;
	}
	else if ( !strcmp(argv[0], "-drc_enable") )
	{
		retval = 1;
		if (!strcmp(argv[1], "on"))
		{
			g_dts_options.enableDrc = true;
		}
		else
		{
			g_dts_options.enableDrc = false;
		}
	}
	else if ( !strcmp(argv[0], "-little_endian") )
	{
		retval = 1;
		if (!strcmp(argv[1], "on"))
		{
			g_dts_options.enableDrc = true;
		}
		else
		{
			g_dts_options.enableDrc = false;
		}
	}
	else
	{
		retval = -1;
	}
	
	return retval;
}

int parse_input_options (char **argv)
{
	int retval = -1;
	
	if ( !strcmp(argv[0], "-file") )
	{
		retval = 1;
		g_input_options.ChannelOpts[CurrentChannelType].file_playback = true;
		strcpy(g_input_options.ChannelOpts[CurrentChannelType].filename, argv[1]);
	}
	else if ( !strcmp(argv[0], "-pcr_pid") )
	{
		retval = 1;
		g_input_options.ChannelOpts[CurrentChannelType].pcr_pid = decOrhex_atoi(argv[1]);
	}
	else if ( !strcmp(argv[0], "-aud_pid") )
	{
		retval = 1;
		g_input_options.ChannelOpts[CurrentChannelType].audio_pid = decOrhex_atoi(argv[1]);
	}
	else if ( !strcmp(argv[0], "-loop") )
	{
		retval = 1;
		g_input_options.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
		g_input_options.loop_count = decOrhex_atoi(argv[1]);
	}
	else if ( !strcmp(argv[0], "-format") )
	{
		retval = 1;
		/*g_input_options.ChannelOpts[CurrentChannelType].audio_codec = decOrhex_atoi(argv[1]);*/
		g_input_options.ChannelOpts[CurrentChannelType].audio_codec = string_to_enum(argv[1]);
		printf("audio codec%d\n",g_input_options.ChannelOpts[CurrentChannelType].audio_codec);
		if ((g_input_options.ChannelOpts[CurrentChannelType].audio_codec == 0) 
			|| (g_input_options.ChannelOpts[CurrentChannelType].audio_codec > NEXUS_AudioCodec_eMax))
		{
			printf ("Invalid audio codec value : %s\n", argv[1]);
			retval = -1;
		}
	}	
	else if ( !strcmp(argv[0], "-eq_coeff_file") )
	{
		retval = 1;
		strcpy(g_input_options.eq_coeff_file, argv[1]);
	}	
	else if ( !strcmp(argv[0], "-targetsync_enable") )
	{
		retval = 0;
		g_input_options.targetsync_enable = true;
	}
#if NEXUS_NUM_I2S_OUTPUTS > 0
    else if ( !strcmp(argv[0], "-multi_i2s") )
    {
        retval = 0;
        g_input_options.multi_i2s = true;
    }
#endif
	else if ( !strcmp(argv[0], "-stereo_disable") )
	{
		retval = 0;
		g_input_options.stereo_disable = true;
	}	
	else if ( !strcmp(argv[0], "-secondary_info") )
	{
		retval = 0;
		if (CurrentChannelType == eChannelTypePrimary)
		{
			CurrentChannelType = eChannelTypeAssociated;
			g_input_options.mixer_enable = true; /* Mixer will be enabled for all cases where associated channels are added */
			memcpy ((void*)&g_input_options.ChannelOpts[eChannelTypeAssociated],
					(void*)&g_input_options.ChannelOpts[eChannelTypePrimary],
					sizeof(sChannel_options));
		}
		else
		{
			printf ("error: There can only be one assocaited Channel for Primary Channel\n");
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-nchs") )
	{
		retval = 1;
		g_input_options.ChannelOpts[CurrentChannelType].num_channels = atoi(argv[1]);
		printf ("app_audio : main() : user specified num_channels = %d\n", g_input_options.ChannelOpts[CurrentChannelType].num_channels);
		
		if (g_input_options.ChannelOpts[CurrentChannelType].num_channels > 8)
		{
			printf ("app_audio : main() : invalid num_channels = %d, forcing to 2 channels\n", g_input_options.ChannelOpts[CurrentChannelType].num_channels);
			g_input_options.ChannelOpts[CurrentChannelType].num_channels = 2;
		}
	}
	else if ( !strcmp(argv[0], "-spdif") )
	{
		retval = 1;
		if ( !strcmp(argv[1], "pcm"))
		{
			g_input_options.eSpdif_out = eSpdifOutPcm;
		}
		else if ( !strcmp(argv[1], "comp"))
		{
			g_input_options.eSpdif_out = eSpdifOutCompressed;
		}
		else if ( !strcmp(argv[1], "enc_ac3"))
		{
			g_input_options.eSpdif_out = eSpdifOutEncAc3;
			g_input_options.encInput = NEXUS_AudioDecoderConnectorType_eStereo;
		}
		else if ( !strcmp(argv[1], "enc_ac3_51"))
		{
			g_input_options.eSpdif_out = eSpdifOutEncAc3;
			g_input_options.encInput = NEXUS_AudioDecoderConnectorType_eMultichannel;
		}
		else if ( !strcmp(argv[1], "enc_dts"))
		{
			g_input_options.eSpdif_out = eSpdifOutEncDts;
			g_input_options.encInput = NEXUS_AudioDecoderConnectorType_eStereo;
		}
		else if ( !strcmp(argv[1], "enc_dts_51"))
		{
			g_input_options.eSpdif_out = eSpdifOutEncDts;
			g_input_options.encInput = NEXUS_AudioDecoderConnectorType_eMultichannel;
		}
		else if ( !strcmp(argv[1], "pcm_multi_lr"))
		{
			g_input_options.eSpdif_out = eSpdifOutPcmMultiLR;
		}
		else if ( !strcmp(argv[1], "pcm_multi_lsrs"))
		{
			g_input_options.eSpdif_out = eSpdifOutPcmMultiLsRs;
		}
		else if ( !strcmp(argv[1], "pcm_multi_clfe"))
		{
			g_input_options.eSpdif_out = eSpdifOutPcmMultiCLFE;
		}
		else if ( !strcmp(argv[1], "pcm_simul"))
		{
			g_input_options.eSpdif_out = eSpdifOutPcmSimulLR;
		}		
		else
		{
			printf ("Invalid value for -spdif option : %s\n", argv[1]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-hdmi") )
	{
		retval = 1;
		if ( !strcmp(argv[1], "pcm"))
		{
			g_input_options.eHdmi_out = eHdmiOutPcmStereo;
		}
		else if ( !strcmp(argv[1], "pcm51"))
		{
			g_input_options.eHdmi_out = eHdmiOutPcmMultiChannel;
		}
		else if ( !strcmp(argv[1], "comp"))
		{
			g_input_options.eHdmi_out = eHdmiOutCompressed;
		}
		else if ( !strcmp(argv[1], "comp_hbr"))
		{
			g_input_options.eHdmi_out = eHdmiOutCompressedHbr;
		}
		else if ( !strcmp(argv[1], "enc_ac3"))
		{
			g_input_options.eHdmi_out = eHdmiOutEncAc3;
			g_input_options.encInput = NEXUS_AudioDecoderConnectorType_eStereo;
		}
		else if ( !strcmp(argv[1], "enc_ac3_51"))
		{
			g_input_options.eHdmi_out = eHdmiOutEncAc3;
			g_input_options.encInput = NEXUS_AudioDecoderConnectorType_eMultichannel;
		}
		else if ( !strcmp(argv[1], "enc_dts"))
		{
			g_input_options.eHdmi_out = eHdmiOutEncDts;
			g_input_options.encInput = NEXUS_AudioDecoderConnectorType_eStereo;
		}
		else if ( !strcmp(argv[1], "enc_dts_51"))
		{
			g_input_options.eHdmi_out = eHdmiOutEncDts;
			g_input_options.encInput = NEXUS_AudioDecoderConnectorType_eMultichannel;
		}
		else
		{
			printf ("Invalid value for -hdmi option : %s\n", argv[1]);
			retval = -1;
		}
	}
#ifdef CUSTOM_VOICE_ENABLE
	else if ( !strcmp(argv[0], "-cv_enable") )
	{
		g_input_options.custom_voice_enable = true;
		retval = 0;
	}
#endif
#if (defined (NEXUS_DOLBY_VOLUME_ENABLE) || defined (NEXUS_DV258_ENABLE))
	else if ( !strcmp(argv[0], "-dv_enable") )
	{
		g_input_options.dolby_vol_enable = true;
		retval = 0;
	}
#endif
#ifdef NEXUS_DDRE_ENABLE
	else if ( !strcmp(argv[0], "-ddre_enable") )
	{
		g_input_options.ddre_enable = true;
		retval = 0;
	}
#endif
#ifdef NEXUS_3DSURROUND_ENABLE
	else if ( !strcmp(argv[0], "-3dsurr_enable") )
	{
		g_input_options.brcm_3d_surr_enable = true;
		retval = 0;
	}
#endif
	else if ( !strcmp(argv[0], "-tv_enable") )
	{
		g_input_options.tru_vol_enable = true;
		retval = 0;
	}
	else if ( !strcmp(argv[0], "-mono_mode") )
	{
		retval = 1;
		if (strcmp(argv[1], "left") == 0)
		{
			g_input_options.ChannelOpts[CurrentChannelType].dual_mono_mode = NEXUS_AudioDecoderDualMonoMode_eLeft;
		}
		else if (strcmp(argv[1], "right") == 0)
		{
			g_input_options.ChannelOpts[CurrentChannelType].dual_mono_mode = NEXUS_AudioDecoderDualMonoMode_eRight;
		}
		else if (strcmp(argv[1], "stereo") == 0)
		{
			g_input_options.ChannelOpts[CurrentChannelType].dual_mono_mode = NEXUS_AudioDecoderDualMonoMode_eStereo;
		}
		else if (strcmp(argv[1], "mixed") == 0)
		{
			g_input_options.ChannelOpts[CurrentChannelType].dual_mono_mode = NEXUS_AudioDecoderDualMonoMode_eMix;
		}
		else
		{
			printf("error: illegal dual mono mode specifier %s\n", argv[1]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-spkrcfg") )
	{
		char speaker_config_str[10];
		retval = 1;
		speaker_config_str[4] = '\0';
		strncpy(speaker_config_str, argv[1], 4);
		if (strlen(speaker_config_str) < 4)
		{
			printf("error: speaker config not specified correctly\n");
			retval = -1;
		}
		else
		{
			if (!strcmp(speaker_config_str, "NLNN"))
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_e1_0;
			}
			else if (!strcmp(speaker_config_str, "lLNN"))
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_e1_1;
			}
			else if (!strcmp(speaker_config_str, "LNNN"))
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_e2_0;
			}
			else if (!strcmp(speaker_config_str, "LLNN"))
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_e3_0;
			}
			else if (!strcmp(speaker_config_str, "LNlN"))
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_e2_1;
			}
			else if (!strcmp(speaker_config_str, "LLlN"))
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_e3_1;
			}
			else if (!strcmp(speaker_config_str, "LNLN"))
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_e2_2;
			}
			else if (!strcmp(speaker_config_str, "LLLN"))
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_e3_2;
			}
			else if (!strcmp(speaker_config_str, "LLLL"))
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_e3_4;
			}
			else
			{
				g_input_options.ChannelOpts[CurrentChannelType].dec_out_mode = NEXUS_AudioDecoderOutputMode_eAuto;
			}
			
			if (argv[1][4] == 'Y')
			{
				g_input_options.ChannelOpts[CurrentChannelType].lfe_out = NEXUS_AudioDecoderOutputLfeMode_eOn;
			}
			else if (argv[1][4] == 'N')
			{
				g_input_options.ChannelOpts[CurrentChannelType].lfe_out = NEXUS_AudioDecoderOutputLfeMode_eOff;
			}
		}
	}
		/*else if ( !strcmp(argv[0], "-csr") )
	{
		retval = 1;
		if (strcmp(argv[1], "32") == 0)
		{
			content_samplerate = 32000;
		}
		else if (strcmp(argv[1], "44.1") == 0)
		{
			content_samplerate = 44100;
		}
		else if (strcmp(argv[1], "12") == 0)
		{
			content_samplerate = 12000;
		}
		else if (strcmp(argv[1], "24") == 0)
		{
			content_samplerate = 24000;
		}
		else if (strcmp(argv[1], "11") == 0)
		{
			content_samplerate = 11025;
		}
		else if (strcmp(argv[1], "22") == 0)
		{
			content_samplerate = 22050;
		}
		else if (strcmp(argv[1], "16") == 0)
		{
			content_samplerate = 16000;
		}
		else if (strcmp(argv[1], "8") == 0)
		{
			content_samplerate = 8000;
		}
		else
		{
		  content_samplerate = 48000;
		}
	}*/
	else
	{
		retval = -1;
	}
	
	return retval;
}

#if (defined (NEXUS_ADV_ENABLE) || defined (NEXUS_ABX_ENABLE))
int parse_adv_options (char **argv)
{
	int retval = -1;
	
	/* ADV options */
	if ( !strcmp(argv[0], "-adv") )
	{
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			g_adv_options.adv.enabled = true;
		}
		else
		{
			g_adv_options.adv.enabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-adv_gain") )
	{
		retval = 1;
		g_adv_options.adv.gainCompression = decOrhex_atoi(argv[1]);
		if ((g_adv_options.adv.gainCompression < 1) || (g_adv_options.adv.gainCompression > 240))
		{
			printf("error: illegal value for gainCompression - %d\n", g_adv_options.adv.gainCompression);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-adv_volume_scaling") )
	{
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			g_adv_options.adv.volumeScalingEnabled = true;
		}
		else
		{
			g_adv_options.adv.volumeScalingEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-adv_volume") )
	{
		retval = 1;
		g_adv_options.adv.volume = decOrhex_atoi(argv[1]);
		if ((g_adv_options.adv.volume > 0) || (g_adv_options.adv.volume < -500))
		{
			printf("error: illegal value for adv volume - %d\n", g_adv_options.adv.volume);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-adv_reinit") )
	{
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			g_adv_options.adv.reInitialize = true;
		}
		else
		{
			g_adv_options.adv.reInitialize = false;
		}
	}
	else if ( !strcmp(argv[0], "-adv_dyn_surr_gain") )
	{
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			g_adv_options.adv.dynamicSurroundGainEnabled = true;
		}
		else
		{
			g_adv_options.adv.dynamicSurroundGainEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-adv_hilo_comp") )
	{
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			g_adv_options.adv.hiLoCompress = true;
		}
		else
		{
			g_adv_options.adv.hiLoCompress = false;
		}
	}
	else if ( !strcmp(argv[0], "-adv_ch_mask") )
	{
		retval = 1;
		g_adv_options.adv.channelMask = (uint8_t)decOrhex_atoi(argv[1]);
	}
	else if ( !strcmp(argv[0], "-adv_headroom_offset") )
	{
		retval = 1;
		g_adv_options.adv.headroomOffset = decOrhex_atoi(argv[1]);
		if ((g_adv_options.adv.headroomOffset < -512)
			|| (g_adv_options.adv.headroomOffset > 512))
		{
			printf("error: illegal value for adv headroomOffset - %d\n", g_adv_options.adv.headroomOffset);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-adv_dyn_eq_mode") )
	{
		retval = 1;
		if ( !strcmp("off", argv[1]))
		{
			g_adv_options.adv.dynamicEqMode = NEXUS_AudysseyAdvDynamicEqMode_eOff;
		}
		else if ( !strcmp("on", argv[1]))
		{
			g_adv_options.adv.dynamicEqMode = NEXUS_AudysseyAdvDynamicEqMode_eOn;
		}
		else if ( !strcmp("rampoff", argv[1]))
		{
			g_adv_options.adv.dynamicEqMode = NEXUS_AudysseyAdvDynamicEqMode_eRampOff;
		}
		else 
		{
			printf("error: illegal value for adv dynamicEqMode - %s\n", argv[1]);
			retval = -1;
		}
	}
	/* ABX options */
	else if ( !strcmp(argv[0], "-abx") )
	{
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			g_adv_options.abx.enabled = true;
		}
		else
		{
			g_adv_options.abx.enabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-abx_harmonicgain") )
	{
		retval = 1;
		g_adv_options.abx.harmonicGain = decOrhex_atoi(argv[1]);
		if (g_adv_options.abx.harmonicGain > 30)
		{
			printf("error: illegal value for harmonicGain - %d\n", g_adv_options.abx.harmonicGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-abx_filterset") )
	{
		retval = 1;
		g_adv_options.abx.filterSet = decOrhex_atoi(argv[1]);
		if ((g_adv_options.abx.filterSet < 1) || (g_adv_options.abx.filterSet > 2))
		{
			printf("error: illegal value for filterSet - %d\n", g_adv_options.abx.filterSet);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-abx_drygain") )
	{
		retval = 1;
		g_adv_options.abx.dryGain = decOrhex_atoi(argv[1]);
		if (g_adv_options.abx.dryGain > 30)
		{
			printf("error: illegal value for dryGain - %d\n", g_adv_options.abx.dryGain);
			retval = -1;
		}
	}
	else
	{
		retval = -1;
	}
	
	if (retval != -1)
	{
		g_adv_options.adv_enable = true;
		
	}
	
	return retval;
}
#endif
#ifdef NEXUS_AVL_ENABLE
int parse_avl_options (char **argv)
{
	int retval = -1;
		
	/* AVL options */
	if ( !strcmp(argv[0], "-avl") )
	{
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			g_AVL_options.avl.enabled = true;
		}
		else
		{
			g_AVL_options.avl.enabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-avl_target") )
	{
		retval = 1;
		g_AVL_options.avl.target = (int)decOrhex_atoi(argv[1]);
	}
	else if ( !strcmp(argv[0], "-avl_lowerBound") )
	{
		retval = 1;
		g_AVL_options.avl.lowerBound = (int)decOrhex_atoi(argv[1]);
	}
	else if ( !strcmp(argv[0], "-avl_fixedBoost") )
	{
		retval = 1;
		g_AVL_options.avl.fixedBoost = (unsigned)decOrhex_atoi(argv[1]);
	}
	else
	{
		retval = -1;
	}
	
	if (retval != -1)
	{
		g_AVL_options.avl.enabled = true;
		
	}

	return retval;
}
#endif

#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE

/*
  Circle surround can to be run as part of Studio Sound only. So if TSHD or TVOL is combined with Circle surround
  TSHD or TVOL cannot be run as standalone
*/
void validate_dependency_srs()
{

	if(sStudioSoundOptions.g_StudioSound_Options.circleSurround.enabled == true)
	{
		if(
			(sStudioSoundOptions.g_StudioSound_Options.truVolume.enabled == true) &&
			(sStudioSoundOptions.TVol_StandAlone == true)
		   )
		{
			printf(" !!! ERROR ::: Circle surround is enabled and cannot be run as part of Studio Sound only. ");
			printf(" TVOL is also enabled and set for running as standalone. This is not possible when CS is enabled \n");

			exit (-1);

		}
		
		if(
			(sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.enabled == true) &&
			(sStudioSoundOptions.TsHD_StandAlone == true)
		   )
		{
			printf(" !!! ERROR ::: Circle surround is enabled and cannot be run as part of Studio Sound only. ");
			printf(" TSHD is also enabled and set for running as standalone. This is not possible when CS is enabled \n");

			exit (-1);
		}
	}
}

#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM
void Assign_SRS_SS_Peq_Coeffs()
{
	FILE *coeff_file;
	char line[300], *value;
	unsigned int sample_rate_idx, filter_band_idx, value_count;
	signed int len;
	int dec_i_value;
	
	
	/*printf("\n\n\nAssign_SRS_SS_Peq_Coeffs:\n");*/
	if(sStudioSoundOptions.g_StudioSound_Options.truEq.eCoefGenMode == 0)
	{
		if (sStudioSoundOptions.TruEQ_Mode0_Coeff_FileName != NULL)
		{
			coeff_file = fopen (sStudioSoundOptions.TruEQ_Mode0_Coeff_FileName, "rt");		
			if (coeff_file == NULL)
			{
				printf ("Assign_SRS_SS_Peq_Coeffs: Could not open file for reading : %s\n", 
								sStudioSoundOptions.TruEQ_Mode0_Coeff_FileName);
				return;
			}
			sample_rate_idx = 0;
			while (!feof (coeff_file))
			{
				/*printf("Assign_SRS_SS_Peq_Coeffs: sample_rate_idx: %d\n", sample_rate_idx);*/
				filter_band_idx =  0;
				while (!feof (coeff_file))
				{
					/*printf("Assign_SRS_SS_Peq_Coeffs: filter_band_idx: %d\n", filter_band_idx);*/
					value_count = 0;
					while (!feof (coeff_file))
					{
						
						fgets(line, 300, coeff_file);
						value = strchr(line, '#');
						if (value!=NULL)
						{
							printf("Assign_SRS_SS_Peq_Coeffs: Coeff File: Comment: %s \n", line);
							continue;
						}
						len = strlen(line);
						if(len!=0)
						{
							if(line[len-1] == '\n')
								line[len-1] = NULL;
						}
						dec_i_value = decOrhex_atoi(line);
						/*printf("Assign_SRS_SS_Peq_Coeffs: Value: %d : %d : %d : 0x%x\n", sample_rate_idx, filter_band_idx, value_count, dec_i_value);*/
						if(value_count == 0)
							sStudioSoundOptions.g_StudioSound_Options.
								truEq.uPEQFilterSettings.sPEQFilterCoef[sample_rate_idx][filter_band_idx].eFilterOrder = dec_i_value;
						else if(value_count == 1)
							sStudioSoundOptions.g_StudioSound_Options.
								truEq.uPEQFilterSettings.sPEQFilterCoef[sample_rate_idx][filter_band_idx].ui32Scale = dec_i_value;
						else if(value_count == 2)
							sStudioSoundOptions.g_StudioSound_Options.
								truEq.uPEQFilterSettings.sPEQFilterCoef[sample_rate_idx][filter_band_idx].i32FilterCoefficientB0 = dec_i_value;
						else if(value_count == 3)
							sStudioSoundOptions.g_StudioSound_Options.
								truEq.uPEQFilterSettings.sPEQFilterCoef[sample_rate_idx][filter_band_idx].i32FilterCoefficientA1 = dec_i_value;
						else if(value_count == 4)
							sStudioSoundOptions.g_StudioSound_Options.
								truEq.uPEQFilterSettings.sPEQFilterCoef[sample_rate_idx][filter_band_idx].i32FilterCoefficientB1 = dec_i_value;
						else if(value_count == 5)
							sStudioSoundOptions.g_StudioSound_Options.
								truEq.uPEQFilterSettings.sPEQFilterCoef[sample_rate_idx][filter_band_idx].i32FilterCoefficientA2 = dec_i_value;
						else if(value_count == 6)
							sStudioSoundOptions.g_StudioSound_Options.
								truEq.uPEQFilterSettings.sPEQFilterCoef[sample_rate_idx][filter_band_idx].i32FilterCoefficientB2 = dec_i_value;
						value_count++;
						if (value_count > 6)
							break;
					}				
					filter_band_idx++;
					if (filter_band_idx > 7)
						break;
				}			
				sample_rate_idx++;
				if (sample_rate_idx > 2)
					break;
			}
			
			fclose(coeff_file);
		}
#if 0		
		/* PEQ coeff for 32k - test document settings */
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][0].eFilterOrder = 			  0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][0].ui32Scale = 			  0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][0].i32FilterCoefficientB0 = 	  0x403fd980   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][0].i32FilterCoefficientA1 = 	  0x7fd42780   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][0].i32FilterCoefficientB1 = 	  0x802bd880   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][0].i32FilterCoefficientA2 = 	  0xc02ad640   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][0].i32FilterCoefficientB2 = 	  0x3f955000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][1].eFilterOrder = 			  0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][1].ui32Scale = 			  0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][1].i32FilterCoefficientB0 = 	  0x3da04b80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][1].i32FilterCoefficientA1 = 	  0x7990df00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][1].i32FilterCoefficientB1 = 	  0x866f2100   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][1].i32FilterCoefficientA2 = 	  0xc6571e40   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][1].i32FilterCoefficientB2 = 	  0x3c089640   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][2].eFilterOrder = 			  0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][2].ui32Scale = 			  0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][2].i32FilterCoefficientB0 = 	  0x43126080   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][2].i32FilterCoefficientA1 = 	  0x7b84ee00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][2].i32FilterCoefficientB1 = 	  0x847b1200   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][2].i32FilterCoefficientA2 = 	  0xc20f9480   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][2].i32FilterCoefficientB2 = 	  0x3ade0b40   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][3].eFilterOrder = 			  0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][3].ui32Scale = 			  0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][3].i32FilterCoefficientB0 = 	  0x317306c0   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][3].i32FilterCoefficientA1 = 	  0x31858a40   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][3].i32FilterCoefficientB1 = 	  0xce7a75c0   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][3].i32FilterCoefficientA2 = 	  0xe6dd0800   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][3].i32FilterCoefficientB2 = 	  0x27aff140   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][4].eFilterOrder = 			  0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][4].ui32Scale = 			  0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][4].i32FilterCoefficientB0 = 	  0x40000000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][4].i32FilterCoefficientA1 = 	  0x798f9f00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][4].i32FilterCoefficientB1 = 	  0x86706100   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][4].i32FilterCoefficientA2 = 	  0xc40eb580   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][4].i32FilterCoefficientB2 = 	  0x3bf14a80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][5].eFilterOrder = 			  0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][5].ui32Scale = 			  0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][5].i32FilterCoefficientB0 = 	  0x40000000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][5].i32FilterCoefficientA1 = 	  0x798f9f00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][5].i32FilterCoefficientB1 = 	  0x86706100   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][5].i32FilterCoefficientA2 = 	  0xc40eb580   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][5].i32FilterCoefficientB2 = 	  0x3bf14a80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][6].eFilterOrder = 			  0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][6].ui32Scale = 			  0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][6].i32FilterCoefficientB0 = 	  0x40000000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][6].i32FilterCoefficientA1 = 	  0x798f9f00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][6].i32FilterCoefficientB1 = 	  0x86706100   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][6].i32FilterCoefficientA2 = 	  0xc40eb580   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][6].i32FilterCoefficientB2 = 	  0x3bf14a80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][7].eFilterOrder = 			  0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][7].ui32Scale = 			  0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][7].i32FilterCoefficientB0 = 	  0x40000000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][7].i32FilterCoefficientA1 = 	  0x798f9f00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][7].i32FilterCoefficientB1 = 	  0x86706100   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][7].i32FilterCoefficientA2 = 	  0xc40eb580   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[0][7].i32FilterCoefficientB2 = 	  0x3bf14a80   ;

		/* PEQ coeff for 44.1k - test document settings */
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][0].eFilterOrder = 			   0x00000002 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][0].ui32Scale = 			   0x00000001 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][0].i32FilterCoefficientB0 = 	   0x402e5900 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][0].i32FilterCoefficientA1 = 	   0x7fe05f80 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][0].i32FilterCoefficientB1 = 	   0x801fa080 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][0].i32FilterCoefficientA2 = 	   0xc01f1840 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][0].i32FilterCoefficientB2 = 	   0x3fb28ec0 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][1].eFilterOrder = 			   0x00000002 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][1].ui32Scale = 			   0x00000001 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][1].i32FilterCoefficientB0 = 	   0x3e40f880 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][1].i32FilterCoefficientA1 = 	   0x7b493680 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][1].i32FilterCoefficientB1 = 	   0x84b6c980 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][1].i32FilterCoefficientA2 = 	   0xc4a9f800 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][1].i32FilterCoefficientB2 = 	   0x3d150f40 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][2].eFilterOrder = 			   0x00000002 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][2].ui32Scale = 			   0x00000001 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][2].i32FilterCoefficientB0 = 	   0x423d1f00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][2].i32FilterCoefficientA1 = 	   0x7d375e00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][2].i32FilterCoefficientB1 = 	   0x82c8a200 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][2].i32FilterCoefficientA2 = 	   0xc18081c0 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][2].i32FilterCoefficientB2 = 	   0x3c425f40 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][3].eFilterOrder = 			   0x00000002 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][3].ui32Scale = 			   0x00000001 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][3].i32FilterCoefficientB0 = 	   0x347fa900 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][3].i32FilterCoefficientA1 = 	   0x499f8a00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][3].i32FilterCoefficientB1 = 	   0xb6607600 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][3].i32FilterCoefficientA2 = 	   0xdeb80980 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][3].i32FilterCoefficientB2 = 	   0x2cc84d80 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][4].eFilterOrder = 			   0x00000002 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][4].ui32Scale = 			   0x00000001 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][4].i32FilterCoefficientB0 = 	   0x40000000 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][4].i32FilterCoefficientA1 = 	   0x7bc37b00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][4].i32FilterCoefficientB1 = 	   0x843c8500 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][4].i32FilterCoefficientA2 = 	   0xc2f83300 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][4].i32FilterCoefficientB2 = 	   0x3d07cd00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][5].eFilterOrder = 			   0x00000002 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][5].ui32Scale = 			   0x00000001 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][5].i32FilterCoefficientB0 = 	   0x40000000 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][5].i32FilterCoefficientA1 = 	   0x7bc37b00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][5].i32FilterCoefficientB1 = 	   0x843c8500 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][5].i32FilterCoefficientA2 = 	   0xc2f83300 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][5].i32FilterCoefficientB2 = 	   0x3d07cd00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][6].eFilterOrder = 			   0x00000002 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][6].ui32Scale = 			   0x00000001 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][6].i32FilterCoefficientB0 = 	   0x40000000 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][6].i32FilterCoefficientA1 = 	   0x7bc37b00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][6].i32FilterCoefficientB1 = 	   0x843c8500 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][6].i32FilterCoefficientA2 = 	   0xc2f83300 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][6].i32FilterCoefficientB2 = 	   0x3d07cd00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][7].eFilterOrder = 			   0x00000002 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][7].ui32Scale = 			   0x00000001 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][7].i32FilterCoefficientB0 = 	   0x40000000 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][7].i32FilterCoefficientA1 = 	   0x7bc37b00 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][7].i32FilterCoefficientB1 = 	   0x843c8500 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][7].i32FilterCoefficientA2 = 	   0xc2f83300 ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[1][7].i32FilterCoefficientB2 = 	   0x3d07cd00 ;

		/* PEQ coeff for 48k - test document settings */
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][0].eFilterOrder = 			   0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][0].ui32Scale = 			   0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][0].i32FilterCoefficientB0 = 	   0x402a9600   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][0].i32FilterCoefficientA1 = 	   0x7fe2fb00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][0].i32FilterCoefficientB1 = 	   0x801d0500   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][0].i32FilterCoefficientA2 = 	   0xc01c9200   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][0].i32FilterCoefficientB2 = 	   0x3fb8d800   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][1].eFilterOrder = 			   0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][1].ui32Scale = 			   0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][1].i32FilterCoefficientB0 = 	   0x3e641300   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][1].i32FilterCoefficientA1 = 	   0x7ba8f000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][1].i32FilterCoefficientB1 = 	   0x84571000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][1].i32FilterCoefficientA2 = 	   0xc44c3600   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][1].i32FilterCoefficientB2 = 	   0x3d4fb6c0   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][2].eFilterOrder = 			   0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][2].ui32Scale = 			   0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][2].i32FilterCoefficientB0 = 	   0x420f0e00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][2].i32FilterCoefficientA1 = 	   0x7d891700   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][2].i32FilterCoefficientB1 = 	   0x8276e900   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][2].i32FilterCoefficientA2 = 	   0xc16199c0   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][2].i32FilterCoefficientB2 = 	   0x3c8f5880   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][3].eFilterOrder = 			   0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][3].ui32Scale = 			   0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][3].i32FilterCoefficientB0 = 	   0x3539ca00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][3].i32FilterCoefficientA1 = 	   0x4eb80b00   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][3].i32FilterCoefficientB1 = 	   0xb147f500   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][3].i32FilterCoefficientA2 = 	   0xdcc6e7c0   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][3].i32FilterCoefficientB2 = 	   0x2dff4e40   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][4].eFilterOrder = 			   0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][4].ui32Scale = 			   0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][4].i32FilterCoefficientB0 = 	   0x40000000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][4].i32FilterCoefficientA1 = 	   0x7c31eb80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][4].i32FilterCoefficientB1 = 	   0x83ce1480   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][4].i32FilterCoefficientA2 = 	   0xc2bbbb80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][4].i32FilterCoefficientB2 = 	   0x3d444480   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][5].eFilterOrder = 			   0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][5].ui32Scale = 			   0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][5].i32FilterCoefficientB0 = 	   0x40000000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][5].i32FilterCoefficientA1 = 	   0x7c31eb80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][5].i32FilterCoefficientB1 = 	   0x83ce1480   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][5].i32FilterCoefficientA2 = 	   0xc2bbbb80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][5].i32FilterCoefficientB2 = 	   0x3d444480   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][6].eFilterOrder = 			   0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][6].ui32Scale = 			   0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][6].i32FilterCoefficientB0 = 	   0x40000000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][6].i32FilterCoefficientA1 = 	   0x7c31eb80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][6].i32FilterCoefficientB1 = 	   0x83ce1480   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][6].i32FilterCoefficientA2 = 	   0xc2bbbb80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][6].i32FilterCoefficientB2 = 	   0x3d444480   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][7].eFilterOrder = 			   0x00000002   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][7].ui32Scale = 			   0x00000001   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][7].i32FilterCoefficientB0 = 	   0x40000000   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][7].i32FilterCoefficientA1 = 	   0x7c31eb80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][7].i32FilterCoefficientB1 = 	   0x83ce1480   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][7].i32FilterCoefficientA2 = 	   0xc2bbbb80   ;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterCoef[2][7].i32FilterCoefficientB2 = 	   0x3d444480   ;		
#endif		
	}
	else if (sStudioSoundOptions.g_StudioSound_Options.truEq.eCoefGenMode == 1)
	{
		/* Will be using defaults from PI */
		/*
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[0].i32BandGain = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[0].ui32BandFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[0].i32QFactor = 0;


		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[1].i32BandGain = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[1].ui32BandFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[1].i32QFactor = 0;

		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[2].i32BandGain = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[2].ui32BandFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[2].i32QFactor = 0;

		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[3].i32BandGain = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[3].ui32BandFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[3].i32QFactor = 0;

		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[4].i32BandGain = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[4].ui32BandFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[4].i32QFactor = 0;

		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[5].i32BandGain = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[5].ui32BandFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[5].i32QFactor = 0;

		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[6].i32BandGain = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[6].ui32BandFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[6].i32QFactor = 0;

		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[7].i32BandGain = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[7].ui32BandFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.truEq.uPEQFilterSettings.sPEQFilterSpec[7].i32QFactor = 0;
		*/

	}
	else
	{
		printf("invalid coeff Gen mode for Teq filters\n");
	}

}


/* This HPF is inside Tvol */
void Assign_SRS_SS_HPF_Front_Filter_Coeffs()
{
	FILE *coeff_file;
	char line[300], *value;
	unsigned int sample_rate_idx, filter_band_idx, value_count;
	signed int len;
	int dec_i_value;

	if(sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.eCoefGenMode == 0)
	{

		if (sStudioSoundOptions.HPF_Front_Mode0_Coeff_FileName != NULL)
		{
			coeff_file = fopen (sStudioSoundOptions.HPF_Front_Mode0_Coeff_FileName, "rt");		
			if (coeff_file == NULL)
			{
				printf ("Assign_SRS_SS_HPF_Front_Filter_Coeffs: Could not open file for reading : %s\n", 
								sStudioSoundOptions.HPF_Front_Mode0_Coeff_FileName);
				return;
			}
			sample_rate_idx = 0;
			while (!feof (coeff_file))
			{
				/*printf("Assign_SRS_SS_HPF_Front_Filter_Coeffs: sample_rate_idx: %d\n", sample_rate_idx);*/
				filter_band_idx =  0;
				while (!feof (coeff_file))
				{
					/*printf("Assign_SRS_SS_HPF_Front_Filter_Coeffs: filter_band_idx: %d\n", filter_band_idx);*/
					value_count = (filter_band_idx==0) ? 0 : 1;
					while (!feof (coeff_file))
					{						
						fgets(line, 300, coeff_file);
						value = strchr(line, '#');
						if (value!=NULL)
						{
							printf("Assign_SRS_SS_HPF_Front_Filter_Coeffs: Coeff File: Comment: %s \n", line);
							continue;
						}
						len = strlen(line);
						if(len!=0)
						{
							if(line[len-1] == '\n')
								line[len-1] = NULL;
						}
						dec_i_value = decOrhex_atoi(line);
						/*printf("Assign_SRS_SS_HPF_Front_Filter_Coeffs: Value: %d : %d : %d : 0x%x\n", sample_rate_idx, filter_band_idx, value_count, dec_i_value);*/
						if(value_count == 0)
							sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.
																		sHPFilterCoef[sample_rate_idx].eFilterOrder = dec_i_value;
						else if(value_count == 1)
							sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].ui32Scale = dec_i_value;
						else if(value_count == 2)
							sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientB0 = dec_i_value;
						else if(value_count == 3)
							sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientB1 = dec_i_value;
						else if(value_count == 4)
							sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientB2 = dec_i_value;
						else if(value_count == 5)
							sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientA1 = dec_i_value;
						else if(value_count == 6)
							sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientA2 = dec_i_value;
						value_count++;
						if (value_count > 6)
							break;
					}
					filter_band_idx++;
					if (filter_band_idx > 2)
						break;
				}			
				sample_rate_idx++;
				if (sample_rate_idx > 2)
					break;
			}
			
			fclose(coeff_file);
		}
	
#if 0	
		/* for sHPFilterCoef[0] */
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].eFilterOrder = 					0x00000004  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientB0 =  	0x0fc81f80  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientB1 = 	0xe06fc100  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientB2 = 	0x0fc81f80  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientA1 = 	0x7e36e680  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientA2 = 	0xc1b4eec0  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientB0 =  	0x3df563c0  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientB1 = 	0x84153880  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientB2 = 	0x3df563c0  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientA1 = 	0x7be0e200  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientA2 = 	0xc40b5300  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientB0 =  	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientB1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientB2 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientA1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientA2 = 	0x00000000  ;


		/* for sHPFilterCoef[1] */

		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].eFilterOrder = 					0x00000004  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientB0 =  	0x0fd78db0  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientB1 = 	0xe050e4a0  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientB2 = 	0x0fd78db0  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientA1 = 	0x7eb71980  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientA2 = 	0xc13e3e40  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientB0 =  	0x3e826c40  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientB1 = 	0x82fb2780  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientB2 = 	0x3e826c40  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientA1 = 	0x7cff9680  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientA2 = 	0xc2f5e600  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientB0 =  	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientB1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientB2 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientA1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientA2 = 	0x00000000  ;


		/* for sHPFilterCoef[2] */
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].eFilterOrder = 					0x00000004 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].ui32Scale = 		0x00000001 ;						
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientB0 =  	0x0fdadc10 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientB1 = 	0xe04a47e0 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientB2 = 	0x0fdadc10 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientA1 = 	0x7ed26000 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientA2 = 	0xc1249f40 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].ui32Scale = 		0x00000001 ;						
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientB0 =  	0x3ea0f4c0 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientB1 = 	0x82be1680 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientB2 = 	0x3ea0f4c0 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientA1 = 	0x7d3d7780 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientA2 = 	0xc2b9a440 ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].ui32Scale = 		0x00000001  ;						
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientB0 =  	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientB1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientB2 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientA1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientA2 = 	0x00000000  ;
#endif

	}
	else if (sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.eCoefGenMode == 1)
	{

		/* 
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterSpec.ui32CutoffFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.uHPFSettings.sHPFilterSpec.eFilterOrder = 0;
		*/

	}
	else
	{
		printf("Warning : invalid Front HPF coeffselection mode ");
	}

}

void Assign_SRS_SS_HPF_Back_Filter_Coeffs()
{
	FILE *coeff_file;
	char line[300], *value;
	unsigned int sample_rate_idx, filter_band_idx, value_count;
	signed int len;
	int dec_i_value;

	if(sStudioSoundOptions.g_StudioSound_Options.highPassFilter.eCoefGenMode == 0)
	{

		if (sStudioSoundOptions.HPF_Back_Mode0_Coeff_FileName != NULL)
		{
			coeff_file = fopen (sStudioSoundOptions.HPF_Back_Mode0_Coeff_FileName, "rt");		
			if (coeff_file == NULL)
			{
				printf ("Assign_SRS_SS_HPF_Back_Filter_Coeffs: Could not open file for reading : %s\n", 
								sStudioSoundOptions.HPF_Back_Mode0_Coeff_FileName);
				return;
			}
			sample_rate_idx = 0;
			while (!feof (coeff_file))
			{
				/*printf("Assign_SRS_SS_HPF_Back_Filter_Coeffs: sample_rate_idx: %d\n", sample_rate_idx);*/
				filter_band_idx =  0;
				while (!feof (coeff_file))
				{
					/*printf("Assign_SRS_SS_HPF_Back_Filter_Coeffs: filter_band_idx: %d\n", filter_band_idx);*/
					value_count = (filter_band_idx==0) ? 0 : 1;
					while (!feof (coeff_file))
					{						
						fgets(line, 300, coeff_file);
						value = strchr(line, '#');
						if (value!=NULL)
						{
							printf("Assign_SRS_SS_HPF_Back_Filter_Coeffs: Coeff File: Comment: %s \n", line);
							continue;
						}
						len = strlen(line);
						if(len!=0)
						{
							if(line[len-1] == '\n')
								line[len-1] = NULL;
						}
						dec_i_value = decOrhex_atoi(line);
						/*printf("Assign_SRS_SS_HPF_Back_Filter_Coeffs: Value: %d : %d : %d : 0x%x\n", sample_rate_idx, filter_band_idx, value_count, dec_i_value);*/
						if(value_count == 0)
							sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.
																		sHPFilterCoef[sample_rate_idx].eFilterOrder = dec_i_value;
						else if(value_count == 1)
							sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].ui32Scale = dec_i_value;
						else if(value_count == 2)
							sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientB0 = dec_i_value;
						else if(value_count == 3)
							sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientB1 = dec_i_value;
						else if(value_count == 4)
							sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientB2 = dec_i_value;
						else if(value_count == 5)
							sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientA1 = dec_i_value;
						else if(value_count == 6)
							sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.
										sHPFilterCoef[sample_rate_idx].sHPFilterCoefTdf2[filter_band_idx].i32FilterCoefficientA2 = dec_i_value;
						value_count++;
						if (value_count > 6)
							break;
					}
					filter_band_idx++;
					if (filter_band_idx > 2)
						break;
				}			
				sample_rate_idx++;
				if (sample_rate_idx > 2)
					break;
			}
			
			fclose(coeff_file);
		}
	
#if 0	
		/* for sHPFilterCoef[0] */
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].eFilterOrder = 					0x00000004  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientB0 =  	0x0fc81f80  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientB1 = 	0xe06fc100  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientB2 = 	0x0fc81f80  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientA1 = 	0x7e36e680  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[0].i32FilterCoefficientA2 = 	0xc1b4eec0  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientB0 =  	0x3df563c0  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientB1 = 	0x84153880  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientB2 = 	0x3df563c0  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientA1 = 	0x7be0e200  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[1].i32FilterCoefficientA2 = 	0xc40b5300  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientB0 =  	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientB1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientB2 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientA1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[0].sHPFilterCoefTdf2[2].i32FilterCoefficientA2 = 	0x00000000  ;


		/* for sHPFilterCoef[1] */

		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].eFilterOrder = 					0x00000004  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientB0 =  	0x0fd78db0  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientB1 = 	0xe050e4a0  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientB2 = 	0x0fd78db0  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientA1 = 	0x7eb71980  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[0].i32FilterCoefficientA2 = 	0xc13e3e40  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientB0 =  	0x3e826c40  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientB1 = 	0x82fb2780  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientB2 = 	0x3e826c40  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientA1 = 	0x7cff9680  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[1].i32FilterCoefficientA2 = 	0xc2f5e600  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].ui32Scale = 		0x00000001  ;					
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientB0 =  	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientB1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientB2 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientA1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[1].sHPFilterCoefTdf2[2].i32FilterCoefficientA2 = 	0x00000000  ;


		/* for sHPFilterCoef[2] */
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].eFilterOrder = 					0x00000004 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].ui32Scale = 		0x00000001 ;						
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientB0 =  	0x0fdadc10 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientB1 = 	0xe04a47e0 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientB2 = 	0x0fdadc10 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientA1 = 	0x7ed26000 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[0].i32FilterCoefficientA2 = 	0xc1249f40 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].ui32Scale = 		0x00000001 ;						
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientB0 =  	0x3ea0f4c0 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientB1 = 	0x82be1680 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientB2 = 	0x3ea0f4c0 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientA1 = 	0x7d3d7780 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[1].i32FilterCoefficientA2 = 	0xc2b9a440 ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].ui32Scale = 		0x00000001  ;						
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientB0 =  	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientB1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientB2 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientA1 = 	0x00000000  ;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterCoef[2].sHPFilterCoefTdf2[2].i32FilterCoefficientA2 = 	0x00000000  ;
#endif

	}
	else if (sStudioSoundOptions.g_StudioSound_Options.highPassFilter.eCoefGenMode == 1)
	{

		/* Will take defaults from PI*/
		/*
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterSpec.ui32CutoffFrequency = 0;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.uHPFSettings.sHPFilterSpec.eFilterOrder = 0;
		*/

	}
	else
	{
		printf("Warning : invalid Back HPF coeffselection mode ");
	}

}
#endif
int parse_SRS_SS_options (char **argv)
{
	int retval = -1;
	FILE *fp_srsconfig;

	printf(" SARA : Parsing SRS options :%s:%s:\n", argv[0], argv[1]);
	
	/* Studio Sound options */
	if ( !strcmp(argv[0], "-srs") )
	{
		retval = 1;
		if ( !strcmp(argv[1], "on"))
		{
			
			sStudioSoundOptions.g_StudioSound_Options.enabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.enabled = false;
		}
	}
	
	else if ( !strcmp(argv[0], "-srs_mode") )
	{
    	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.mode = decOrhex_atoi(argv[1]);
		if (((uint32_t)sStudioSoundOptions.g_StudioSound_Options.mode < 0) || ((uint32_t)sStudioSoundOptions.g_StudioSound_Options.mode > (uint32_t)2))
		{
			printf("error: illegal value for SRS Studio mode (range 0 to 2) - %d\n", sStudioSoundOptions.g_StudioSound_Options.mode);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_igain") )
	{
    	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.inputGain = decOrhex_atoi(argv[1]);
		if (((uint32_t)sStudioSoundOptions.g_StudioSound_Options.inputGain < 0) || ((uint32_t)sStudioSoundOptions.g_StudioSound_Options.inputGain > (uint32_t)100))
		{
			printf("error: illegal value for SRS Studio input gain (range 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.inputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_ogain") )
	{
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.outputGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.outputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.outputGain > 100))
		{
			printf("error: illegal value for SRS Studio output gain (range 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.outputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_bGain") )
	{
    
       unsigned headroomGain;      /* Provides headroom for intermediate calculations to avoid clipping.  Ranges from 0 to 100 in %. */


		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.bypassGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.bypassGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.bypassGain > 100))
		{
			printf("error: illegal value for SRS Studio Bypass gain (range 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.bypassGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_hGain") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.headroomGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.headroomGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.headroomGain > 100))
		{
			printf("error: illegal value for SRS Studio Headroom gain (range 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.headroomGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TVEnable") )
	{
    
       	retval = 1;



		if ( !strncmp("on", argv[1],2))
		{
			
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enabled = false;
		}
		
	}

	else if ( !strcmp(argv[0], "-srs_TV_blksize") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.blockSize = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.blockSize < NEXUS_TruVolumeBlockSize_e256) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.blockSize > NEXUS_TruVolumeBlockSize_e1024))
		{
			printf("error: illegal value for SRS Studio - Tru Volume BlockSize (range 0 to 3) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.blockSize);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TV_enNormGain") )
	{
    
       	retval = 1;

		if ( !strncmp("on", argv[1],2))
		{
			
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enableNormalGain = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enableNormalGain = false;
		}

		

	}

	else if ( !strcmp(argv[0], "-srs_TV_igain") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.inputGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.inputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.inputGain > 3200))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Input gain (range 0 to 3200) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.inputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_ogain") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.outputGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.outputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.outputGain > 3200))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Output gain (range 0 to 3200) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.outputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_bgain") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.bypassGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.bypassGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.bypassGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Bypass gain (range 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.bypassGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TV_reflvl") )
	{
    
       	retval = 1;
		
		sStudioSoundOptions.g_StudioSound_Options.truVolume.referenceLevel = decOrhex_atoi(argv[1]);

		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.referenceLevel < 0x109) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.referenceLevel > 0x7fffff))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Reference Level (Ranges 0x00000109 to 0x007fffff) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.referenceLevel);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TV_mode") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.mode = decOrhex_atoi(argv[1]);

		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.mode < NEXUS_TruVolumeMode_eLight) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.mode > NEXUS_TruVolumeMode_eHeavy))
		{
			printf("error: illegal value for SRS Studio - Tru Volume mode (Ranges 0 to 2) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.mode);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TV_spkrres") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.speakerResolution = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.speakerResolution < NEXUS_TruVolumeSpeakerResolution_e20Hz) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.speakerResolution > NEXUS_TruVolumeSpeakerResolution_e410Hz))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Speaker Resolution (Ranges 0 to 5) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.speakerResolution);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TV_maxgain") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.maxGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.maxGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.maxGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Max Gain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.maxGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TV_enDCfilt") )
	{
    
       	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enableDcNotchFilter = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enableDcNotchFilter = false;
		}


	}
	else if ( !strcmp(argv[0], "-srs_TV_enNoiseMgr") )
	{
    
       	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enableNoiseManager = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enableNoiseManager = false;
		}

		
	}
	else if ( !strcmp(argv[0], "-srs_TV_noisethres") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.noiseManagerThreshold = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.noiseManagerThreshold < 0) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.noiseManagerThreshold > 200))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Noise Manager Threshold (Ranges 0 to 200) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.noiseManagerThreshold);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_enNorm") )
	{
    
       	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enableNormalizer = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.enableNormalizer = false;
		}

		
		
	}		
	else if ( !strcmp(argv[0], "-srs_TV_calibrate") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.calibrate = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.calibrate < 0) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.calibrate > 200))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Calibrate (Ranges 0 to 25600) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.calibrate);
			retval = -1;
		}
	}
#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM
	else if ( !strcmp(argv[0], "-srs_HPF_front_en") )
	{
    
       	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.bEnable = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.bEnable = false;
		}

		
	}
	else if ( !strcmp(argv[0], "-srs_HPF_front_mode") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.eCoefGenMode = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.eCoefGenMode < 0) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.eCoefGenMode > 1))
		{
			printf("error: illegal value for SRS Studio - Tru Volume High passfilter Mode (Ranges 0 to 1) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.eCoefGenMode);
			retval = -1;
		}
	}	
	else if ( !strcmp(argv[0], "-srs_HPF_front_mode0_coeff_file") )
	{
	   	retval = 1;
		fp_srsconfig = fopen(argv[1],"rb");

		if(fp_srsconfig == NULL)
		{
			printf("error: illegal value for -srs_HPF_front_mode0_coeff_file %s cannot be opened\n", argv[1]);
			retval = -1;
		}
		else
		{
			sStudioSoundOptions.HPF_Front_Mode0_Coeff_FileName = argv[1];
			fclose(fp_srsconfig);
		}
	}
#else
	else if ( !strcmp(argv[0], "-srs_HPF_front_en") )
	{
    
       	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilter.enabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilter.enabled = false;
		}

		
	}
	
	else if ( !strcmp(argv[0], "-srs_HPF_front_mode") )
	{
    
       	retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilter.mode = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilter.mode < 0) || (sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilter.mode > 3))
		{
			printf("error: illegal value for SRS Studio - Tru Volume High passfilter Mode (Ranges 0 to 3) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilter.mode);
			retval = -1;
		}
	}
#endif
	

	else if ( !strcmp(argv[0], "-srs_TSHD_enable") )
	{
		retval = 1;
    
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.enabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.enabled = false;
		}

	}
	else if ( !strcmp(argv[0], "-srs_TSHD_headphoneEn") )
	{
		retval = 1;
    
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.headphoneEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.headphoneEnabled = false;
		}

	}
	else if ( !strcmp(argv[0], "-srs_TSHD_mToSEn") )
	{
		retval = 1;
    
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.monoToStereoEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.monoToStereoEnabled = false;
		}

	}

	else if ( !strcmp(argv[0], "-srs_TSHD_dialClarityEn") )
	{
		retval = 1;
    
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.dialogClarityEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.dialogClarityEnabled = false;
		}

	}

	else if ( !strcmp(argv[0], "-srs_TSHD_dialClarityLvl") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.dialogClarityLevel = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.dialogClarityLevel < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.dialogClarityLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD dialog Clarity Level(Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.dialogClarityLevel);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TSHD_truBassEn") )
	{
		
    
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_trubassLvl") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassLevel = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassLevel < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD Tru bass level(Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassLevel);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TSHD_spkrSize") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.speakerSize = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.speakerSize < NEXUS_TruBassSpeakerSize_e40Hz) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.speakerSize > NEXUS_TruBassSpeakerSize_e400Hz))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD Speaker size (Ranges 0 to 7) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.speakerSize);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TSHD_trubassMode") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassMode = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassMode < NEXUS_TruBassMode_eMono) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassMode > NEXUS_TruBassMode_eStereo))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD tru bass mode (Ranges 0 to 1) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.truBassMode);
			retval = -1;
		}
	}


	else if ( !strcmp(argv[0], "-srs_TSHD_ipgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD input gain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_opgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.outputGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.outputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.outputGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD output gain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.outputGain);
			retval = -1;
		}
	}
	
	else if ( !strcmp(argv[0], "-srs_TSHD_bpgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.bypassGain = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.bypassGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.bypassGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD bypass gain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.bypassGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TSHD_defEn") )
	{
		
    
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.definitionEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.definitionEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_deflvl") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.definitionLevel = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.definitionLevel < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.definitionLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD definition level (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.definitionLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_surlvl") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.surroundLevel = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.surroundLevel < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.surroundLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD surround level (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.surroundLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowEn") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowBitrate") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdHighBitRateEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdHighBitRateEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowFocusEn") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdFocusEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdFocusEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowMode") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdMode = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdMode < NEXUS_WowHdMode_eMono) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdMode > NEXUS_WowHdMode_eExtreme))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD WoW HD Mode (Ranges 0 to 3) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdMode);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TSHD_wowSpaceLvl") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdSpaceLevel = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdSpaceLevel < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdSpaceLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD WoW HD Space level (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdSpaceLevel);
			retval = -1;
		}
	}
	
	else if ( !strcmp(argv[0], "-srs_TSHD_wowCenterLvl") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdCenterLevel = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdCenterLevel < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdCenterLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD WoW HD Center level (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdCenterLevel);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TSHD_wowFocusLvl") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdFocusLevel = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdFocusLevel < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdFocusLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD WoW HD Focus level (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.wowHdFocusLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_CertAppEn") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.certificationApp = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.certificationApp = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_lfeEn") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputLfeEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputLfeEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_ipAcmode") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputAcmod = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputAcmod < 0) || (sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputAcmod > 13))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD input Acmode (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.inputAcmod);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_cfgfile") )
	{
		retval = 3;

		fp_srsconfig = fopen(argv[1],"rb");

		if(fp_srsconfig == NULL)
		{
			printf("error: illegal value for SRS Studio - Tru Vol config file %s cannot be opened\n", argv[1]);
			retval = -1;
		}
		else
		{
			fclose(fp_srsconfig);
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_Standalone") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.TVol_StandAlone = true;
		}
		else
		{
			sStudioSoundOptions.TVol_StandAlone = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_THD_Standalone") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.TsHD_StandAlone = true;
		}
		else
		{
			sStudioSoundOptions.TsHD_StandAlone = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_CS_enable") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.circleSurround.enabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.circleSurround.enabled = false;
		}
	}

	else if ( !strcmp(argv[0], "-srs_CS_ipgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.circleSurround.inputGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.circleSurround.inputGain < 125) || (sStudioSoundOptions.g_StudioSound_Options.circleSurround.inputGain > 1000))
		{
			printf("error: illegal value for SRS Studio - Circle surround input gain (Ranges 125 to 1000) - %d\n", sStudioSoundOptions.g_StudioSound_Options.circleSurround.inputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_CS_mode") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.circleSurround.mode= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.circleSurround.mode < NEXUS_CircleSurroundMode_eCinema) || (sStudioSoundOptions.g_StudioSound_Options.circleSurround.mode > NEXUS_CircleSurroundMode_eMusic))
		{
			printf("error: illegal value for SRS Studio - Circle surround mode (Ranges 0 to 1) - %d\n", sStudioSoundOptions.g_StudioSound_Options.circleSurround.mode);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_CS_opgainF") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainFront= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainFront < 89) || (sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainFront > 4000))
		{
			printf("error: illegal value for SRS Studio - Circle surround Output gain front (Ranges 89 to 4000) - %d\n", sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainFront);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_CS_opgainR") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainRear= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainRear < 89) || (sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainRear > 4000))
		{
			printf("error: illegal value for SRS Studio - Circle surround Output gain Rear (Ranges 89 to 4000) - %d\n", sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainRear);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_CS_opgainC") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainCenter= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainCenter < 89) || (sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainCenter > 4000))
		{
			printf("error: illegal value for SRS Studio - Circle surround Output gain center (Ranges 89 to 4000) - %d\n", sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainCenter);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_CS_opgainSW") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainSubwoofer= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainSubwoofer < 89) || (sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainSubwoofer > 4000))
		{
			printf("error: illegal value for SRS Studio - Circle surround Output gain SubWoofer (Ranges 89 to 4000) - %d\n", sStudioSoundOptions.g_StudioSound_Options.circleSurround.outputGainSubwoofer);
			retval = -1;
		}
	}
	


	
	else if ( !strcmp(argv[0], "-srs_geq_lenable") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_renable") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_bandmode") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.bandMode= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.bandMode < NEXUS_SrsGraphicEqBandMode_eFiveBand) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.bandMode > NEXUS_SrsGraphicEqBandMode_eTenBand))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ band mode (Ranges 0 to 2) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.bandMode);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_lbgain0") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[0]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[0] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[0] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[0] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[0]);
			retval = -1;
		}
	}
    
	else if ( !strcmp(argv[0], "-srs_geq_lbgain1") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[1]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[1] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[1] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[1] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[1]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_lbgain2") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[2]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[2] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[2] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[2] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[2]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_lbgain3") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[3]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[3] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[3] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[3] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[3]);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_geq_lbgain4") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[4]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[4] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[4] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[4] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[4]);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_geq_lbgain5") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[5]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[5] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[5] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[5] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[5]);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_geq_lbgain6") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[6]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[6] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[6] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[6] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[6]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_lbgain7") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[7]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[7] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[7] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[7] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[7]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_lbgain8") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[8]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[8] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[8] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[8] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[8]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_lbgain9") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[9]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[9] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[9] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ left gain[9] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftBandGain[9]);
			retval = -1;
		}
	}
	
	
	
	else if ( !strcmp(argv[0], "-srs_geq_rbgain0") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[0]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[0] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[0] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[0] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[0]);
			retval = -1;
		}
	}
    
	else if ( !strcmp(argv[0], "-srs_geq_rbgain1") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[1]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[1] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[1] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[1] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[1]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_rbgain2") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[2]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[2] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[2] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[2] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[2]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_rbgain3") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[3]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[3] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[3] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[3] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[3]);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_geq_rbgain4") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[4]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[4] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[4] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[4] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[4]);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_geq_rbgain5") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[5]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[5] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[5] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[5] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[5]);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_geq_rbgain6") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[6]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[6] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[6] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[6] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[6]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_rbgain7") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[7]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[7] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[7] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[7] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[7]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_rbgain8") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[8]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[8] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[8] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[8] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[8]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_rbgain9") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[9]= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[9] < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[9] > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ right gain[9] (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightBandGain[9]);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_ipgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.inputGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.inputGain < 316) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.inputGain > 3160))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ inputgain (Ranges 316 to 3160) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.inputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_opgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.outputGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.outputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.outputGain > 100))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ outputGain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.outputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_geq_bpgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.graphicEq.bypassGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.graphicEq.bypassGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.graphicEq.bypassGain > 100))
		{
			printf("error: illegal value for SRS Studio - Graphic EQ bypassGain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.graphicEq.bypassGain);
			retval = -1;
		}
	}


	

     
    
    else if ( !strcmp(argv[0], "-srs_teq_lenable") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_renable") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightEnabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightEnabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_lbEn0") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[0] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[0] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_lbEn1") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[1] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[1] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_lbEn2") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[2] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[2] = false;
		}
	}
		else if ( !strcmp(argv[0], "-srs_teq_lbEn3") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[3] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[3] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_lbEn4") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[4] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[4] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_lbEn5") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[5] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[5] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_lbEn6") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[6] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[6] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_lbEn7") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[7] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.leftBandEnabled[7] = false;
		}
	}
    else if ( !strcmp(argv[0], "-srs_teq_rbEn0") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[0] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[0] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_rbEn1") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[1] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[1] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_rbEn2") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[2] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[2] = false;
		}
	}
		else if ( !strcmp(argv[0], "-srs_teq_rbEn3") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[3] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[3] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_rbEn4") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[4] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[4] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_rbEn5") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[5] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[5] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_rbEn6") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[6] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[6] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_rbEn7") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[7] = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truEq.rightBandEnabled[7] = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_ipgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truEq.inputGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truEq.inputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truEq.inputGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru EQ input Gain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truEq.inputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_opgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truEq.outputGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truEq.outputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truEq.outputGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru EQ output Gain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truEq.outputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_bpgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truEq.bypassGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truEq.bypassGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truEq.bypassGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru EQ bypass Gain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truEq.bypassGain);
			retval = -1;
		}
	}
#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM	
	else if ( !strcmp(argv[0], "-srs_teq_mode") )
	{    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truEq.eCoefGenMode = decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truEq.eCoefGenMode < 0) || (sStudioSoundOptions.g_StudioSound_Options.truEq.eCoefGenMode > 1))
		{
			printf("error: illegal value for SRS Studio - Tru EQ Coefficient Generation Mode (Ranges 0 to 1) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truEq.eCoefGenMode);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_teq_mode0_coeff_file") )
	{
	   	retval = 1;
		fp_srsconfig = fopen(argv[1],"rb");

		if(fp_srsconfig == NULL)
		{
			printf("error: illegal value for -srs_teq_mode0_coeff_file %s cannot be opened\n", argv[1]);
			retval = -1;
		}
		else
		{
			sStudioSoundOptions.TruEQ_Mode0_Coeff_FileName = argv[1];
			fclose(fp_srsconfig);
		}
	}
#else
	else if ( !strcmp(argv[0], "-srs_teq_mode") )
	{    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truEq.filterMode= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truEq.filterMode < 0) || (sStudioSoundOptions.g_StudioSound_Options.truEq.filterMode > 1))
		{
			printf("error: illegal value for SRS Studio - Tru EQ FilterMode (Ranges 0 to 1) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truEq.filterMode);
			retval = -1;
		}
	}
#endif
#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM
	else if ( !strcmp(argv[0], "-srs_HPF_back_en") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.highPassFilter.bEnable = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.highPassFilter.bEnable = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_HPF_back_mode") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.eCoefGenMode= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.highPassFilter.eCoefGenMode < 0) || (sStudioSoundOptions.g_StudioSound_Options.highPassFilter.eCoefGenMode > 1))
		{
			printf("error: illegal value for SRS Studio - high pass Filter mode (Ranges 0 to 1) - %d\n", sStudioSoundOptions.g_StudioSound_Options.highPassFilter.eCoefGenMode);
			retval = -1;
		}
	}	
	else if ( !strcmp(argv[0], "-srs_HPF_back_mode0_coeff_file") )
	{
	   	retval = 1;
		fp_srsconfig = fopen(argv[1],"rb");

		if(fp_srsconfig == NULL)
		{
			printf("error: illegal value for -srs_HPF_back_mode0_coeff_file %s cannot be opened\n", argv[1]);
			retval = -1;
		}
		else
		{
			sStudioSoundOptions.HPF_Back_Mode0_Coeff_FileName = argv[1];
			fclose(fp_srsconfig);
		}
	}
#else
	else if ( !strcmp(argv[0], "-srs_HPF_back_en") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.highPassFilter.enabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.highPassFilter.enabled = false;
		}
	}
	else if ( !strcmp(argv[0], "-srs_HPF_back_mode") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.highPassFilter.mode= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.highPassFilter.mode < 0) || (sStudioSoundOptions.g_StudioSound_Options.highPassFilter.mode > 4))
		{
			printf("error: illegal value for SRS Studio - high pass Filter mode (Ranges 0 to 4) - %d\n", sStudioSoundOptions.g_StudioSound_Options.highPassFilter.mode);
			retval = -1;
		}
	}
#endif	
	else if ( !strcmp(argv[0], "-srs_hl_en") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.hardLimiter.enabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.hardLimiter.enabled = false;
		}
	}

	else if ( !strcmp(argv[0], "-srs_hl_blksize") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.hardLimiter.blockSize= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.hardLimiter.blockSize < 0) || (sStudioSoundOptions.g_StudioSound_Options.hardLimiter.blockSize > 3))
		{
			printf("error: illegal value for SRS Studio - hard limiter blocksize (Ranges 0 to 3) - %d\n", sStudioSoundOptions.g_StudioSound_Options.hardLimiter.blockSize);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_hl_ipgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.hardLimiter.inputGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.hardLimiter.inputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.hardLimiter.inputGain > 400))
		{
			printf("error: illegal value for SRS Studio - hard limiter input gain (Ranges 0 to 400) - %d\n", sStudioSoundOptions.g_StudioSound_Options.hardLimiter.inputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_hl_opgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.hardLimiter.outputGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.hardLimiter.outputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.hardLimiter.outputGain > 400))
		{
			printf("error: illegal value for SRS Studio - hard limiter output gain (Ranges 0 to 400) - %d\n", sStudioSoundOptions.g_StudioSound_Options.hardLimiter.outputGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_hl_bpgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.hardLimiter.bypassGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.hardLimiter.bypassGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.hardLimiter.bypassGain > 100))
		{
			printf("error: illegal value for SRS Studio - hard limiter bypass gain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.hardLimiter.bypassGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_hl_boost") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.hardLimiter.boost= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.hardLimiter.boost < 1) || (sStudioSoundOptions.g_StudioSound_Options.hardLimiter.boost > 256000))
		{
			printf("error: illegal value for SRS Studio - hard limiter boost (Ranges 0 to 256000) - %d\n", sStudioSoundOptions.g_StudioSound_Options.hardLimiter.boost);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_hl_level") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.hardLimiter.level= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.hardLimiter.level < 3) || (sStudioSoundOptions.g_StudioSound_Options.hardLimiter.level > 100000))
		{
			printf("error: illegal value for SRS Studio - hard limiter level (Ranges 3 to 100000) - %d\n", sStudioSoundOptions.g_StudioSound_Options.hardLimiter.level);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_hl_delay") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.hardLimiter.delay= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.hardLimiter.delay < 6) || (sStudioSoundOptions.g_StudioSound_Options.hardLimiter.delay > 22))
		{
			printf("error: illegal value for SRS Studio - hard limiter delay (Ranges 6 to 22) - %d\n", sStudioSoundOptions.g_StudioSound_Options.hardLimiter.delay);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TD_en") )
	{
	   	retval = 1;

		if ( !strcmp("on", argv[1]))
		{
			sStudioSoundOptions.g_StudioSound_Options.truDialog.enabled = true;
		}
		else
		{
			sStudioSoundOptions.g_StudioSound_Options.truDialog.enabled = false;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TD_igain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truDialog.inputGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truDialog.inputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truDialog.inputGain > 100))
		{
			printf("error: illegal value for SRS Studio - truDialog inputgain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truDialog.inputGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TD_ogain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truDialog.outputGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truDialog.outputGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truDialog.outputGain > 100))
		{
			printf("error: illegal value for SRS Studio - truDialog outputGain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truDialog.outputGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TD_pgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truDialog.processGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truDialog.processGain < 50) || (sStudioSoundOptions.g_StudioSound_Options.truDialog.processGain > 300))
		{
			printf("error: illegal value for SRS Studio - truDialog processGain (Ranges 50 to 300) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truDialog.processGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TD_bgain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truDialog.bypassGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truDialog.bypassGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truDialog.bypassGain > 100))
		{
			printf("error: illegal value for SRS Studio - truDialog bypassGain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truDialog.bypassGain);
			retval = -1;
		}
	}

	else if ( !strcmp(argv[0], "-srs_TD_egain") )
	{
		
    
		retval = 1;
		sStudioSoundOptions.g_StudioSound_Options.truDialog.enhancementGain= decOrhex_atoi(argv[1]);
		if ((sStudioSoundOptions.g_StudioSound_Options.truDialog.enhancementGain < 0) || (sStudioSoundOptions.g_StudioSound_Options.truDialog.enhancementGain > 100))
		{
			printf("error: illegal value for SRS Studio - truDialog enhancementGain (Ranges 0 to 100) - %d\n", sStudioSoundOptions.g_StudioSound_Options.truDialog.enhancementGain);
			retval = -1;
		}
	}
	else
	{
		retval = -1;
	}
	
	if (retval != -1)
	{
		sStudioSoundOptions.g_StudioSound_Options.enabled = true;
		
	}
	
	return retval;
}


#else
#ifdef NEXUS_SRSTRUVOL_ENABLE
int parse_SRS_TVOL_options (char **argv)
{
	int retval = -1;
	FILE *fp_srsconfig;

	printf("Parsing SRS TVOL options :%s:%s:\n", argv[0], argv[1]);
	
	/* SRS TVOL options */
	if ( !strcmp(argv[0], "-srs_TVEnable") )
	{    
       	retval = 1;
		if ( !strncmp("on", argv[1],2))
			g_srs_tvol_settings.enabled = true;
		else
			g_srs_tvol_settings.enabled = false;		
	}
	else if ( !strcmp(argv[0], "-srs_TV_blksize") )
	{    
       	retval = 1;
		g_srs_tvol_settings.blockSize = decOrhex_atoi(argv[1]);
		if (g_srs_tvol_settings.blockSize > NEXUS_TruVolumeBlockSize_e1024)
		{
			printf("error: illegal value for SRS Studio - Tru Volume BlockSize (range 0 to 3) - %d\n", g_srs_tvol_settings.blockSize);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_enNormGain") )
	{    
       	retval = 1;
		if ( !strncmp("on", argv[1],2))
			g_srs_tvol_settings.enableNormalGain = true;
		else
			g_srs_tvol_settings.enableNormalGain = false;
	}
	else if ( !strcmp(argv[0], "-srs_TV_igain") )
	{
       	retval = 1;
		g_srs_tvol_settings.inputGain = decOrhex_atoi(argv[1]);
		if (g_srs_tvol_settings.inputGain > 3200)
		{
			printf("error: illegal value for SRS Studio - Tru Volume Input gain (range 0 to 3200) - %d\n", g_srs_tvol_settings.inputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_ogain") )
	{    
       	retval = 1;
		g_srs_tvol_settings.outputGain = decOrhex_atoi(argv[1]);
		if (g_srs_tvol_settings.outputGain > 3200)
		{
			printf("error: illegal value for SRS Studio - Tru Volume Output gain (range 0 to 3200) - %d\n", g_srs_tvol_settings.outputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_bgain") )
	{    
       	retval = 1;
		g_srs_tvol_settings.bypassGain = decOrhex_atoi(argv[1]);
		if (g_srs_tvol_settings.bypassGain > 100)
		{
			printf("error: illegal value for SRS Studio - Tru Volume Bypass gain (range 0 to 100) - %d\n", g_srs_tvol_settings.bypassGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_reflvl") )
	{    
       	retval = 1;		
		g_srs_tvol_settings.referenceLevel = decOrhex_atoi(argv[1]);
		if ((g_srs_tvol_settings.referenceLevel < 0x109) || (g_srs_tvol_settings.referenceLevel > 0x7fffff))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Reference Level (Ranges 0x00000109 to 0x007fffff) - %d\n", g_srs_tvol_settings.referenceLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_mode") )
	{    
       	retval = 1;
		g_srs_tvol_settings.mode = decOrhex_atoi(argv[1]);
		if (g_srs_tvol_settings.mode > NEXUS_TruVolumeMode_eHeavy)
		{
			printf("error: illegal value for SRS Studio - Tru Volume mode (Ranges 0 to 2) - %d\n", g_srs_tvol_settings.mode);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_spkrres") )
	{    
       	retval = 1;
		g_srs_tvol_settings.speakerResolution = decOrhex_atoi(argv[1]);
		if (g_srs_tvol_settings.speakerResolution > NEXUS_TruVolumeSpeakerResolution_e410Hz)
		{
			printf("error: illegal value for SRS Studio - Tru Volume Speaker Resolution (Ranges 0 to 5) - %d\n", g_srs_tvol_settings.speakerResolution);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_maxgain") )
	{    
       	retval = 1;
		g_srs_tvol_settings.maxGain = decOrhex_atoi(argv[1]);
		if (g_srs_tvol_settings.maxGain > 100)
		{
			printf("error: illegal value for SRS Studio - Tru Volume Max Gain (Ranges 0 to 100) - %d\n", g_srs_tvol_settings.maxGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_enDCfilt") )
	{    
       	retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tvol_settings.enableDcNotchFilter = true;
		else
			g_srs_tvol_settings.enableDcNotchFilter = false;
	}
	else if ( !strcmp(argv[0], "-srs_TV_enNoiseMgr") )
	{    
       	retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tvol_settings.enableNoiseManager = true;
		else
			g_srs_tvol_settings.enableNoiseManager = false;
	}
	else if ( !strcmp(argv[0], "-srs_TV_noisethres") )
	{    
       	retval = 1;
		g_srs_tvol_settings.noiseManagerThreshold = decOrhex_atoi(argv[1]);
		if ((g_srs_tvol_settings.noiseManagerThreshold < 0) || (g_srs_tvol_settings.noiseManagerThreshold > 200))
		{
			printf("error: illegal value for SRS Studio - Tru Volume Noise Manager Threshold (Ranges 0 to 200) - %d\n", g_srs_tvol_settings.noiseManagerThreshold);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TV_enNorm") )
	{    
       	retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tvol_settings.enableNormalizer = true;
		else
			g_srs_tvol_settings.enableNormalizer = false;
	}		
	else if ( !strcmp(argv[0], "-srs_TV_calibrate") )
	{
    
       	retval = 1;
		g_srs_tvol_settings.calibrate = decOrhex_atoi(argv[1]);
		if (g_srs_tvol_settings.calibrate > 200)
		{
			printf("error: illegal value for SRS Studio - Tru Volume Calibrate (Ranges 0 to 25600) - %d\n", g_srs_tvol_settings.calibrate);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_cfgfile") )
	{
		retval = 3;
		fp_srsconfig = fopen(argv[1],"rb");
		if(fp_srsconfig == NULL)
		{
			printf("error: illegal value for SRS Tru Vol config file %s cannot be opened\n", argv[1]);
			retval = -1;
		}
		else
		{
			fclose(fp_srsconfig);
		}
	}
	else
		retval = -1;
	
	return retval;
}
#endif
#ifdef NEXUS_SRSHD_ENABLE
int parse_SRS_TSHD_options (char **argv)
{
	int retval = -1;
	FILE *fp_srsconfig;

	printf("Parsing SRS TSHD options :%s:%s:\n", argv[0], argv[1]);
	
	/* SRS TSHD options */
	if ( !strcmp(argv[0], "-srs_TSHD_enable") )
	{
		retval = 1;    
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.enabled = true;
		else
			g_srs_tshd_settings.enabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_headphoneEn") )
	{
		retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.headphoneEnabled = true;
		else
			g_srs_tshd_settings.headphoneEnabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_mToSEn") )
	{
		retval = 1;    
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.monoToStereoEnabled = true;
		else
			g_srs_tshd_settings.monoToStereoEnabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_dialClarityEn") )
	{
		retval = 1;    
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.dialogClarityEnabled = true;
		else
			g_srs_tshd_settings.dialogClarityEnabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_dialClarityLvl") )
	{    
		retval = 1;
		g_srs_tshd_settings.dialogClarityLevel = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.dialogClarityLevel < 0) || (g_srs_tshd_settings.dialogClarityLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD dialog Clarity Level(Ranges 0 to 100) - %d\n", g_srs_tshd_settings.dialogClarityLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_truBassEn") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.truBassEnabled = true;
		else
			g_srs_tshd_settings.truBassEnabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_trubassLvl") )
	{
		retval = 1;
		g_srs_tshd_settings.truBassLevel = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.truBassLevel < 0) || (g_srs_tshd_settings.truBassLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD Tru bass level(Ranges 0 to 100) - %d\n", g_srs_tshd_settings.truBassLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_spkrSize") )
	{
		retval = 1;
		g_srs_tshd_settings.speakerSize = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.speakerSize < NEXUS_TruBassSpeakerSize_e40Hz) || (g_srs_tshd_settings.speakerSize > NEXUS_TruBassSpeakerSize_e400Hz))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD Speaker size (Ranges 0 to 7) - %d\n", g_srs_tshd_settings.speakerSize);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_trubassMode") )
	{
		retval = 1;
		g_srs_tshd_settings.truBassMode = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.truBassMode < NEXUS_TruBassMode_eMono) || (g_srs_tshd_settings.truBassMode > NEXUS_TruBassMode_eStereo))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD tru bass mode (Ranges 0 to 1) - %d\n", g_srs_tshd_settings.truBassMode);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_ipgain") )
	{    
		retval = 1;
		g_srs_tshd_settings.inputGain = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.inputGain < 0) || (g_srs_tshd_settings.inputGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD input gain (Ranges 0 to 100) - %d\n", g_srs_tshd_settings.inputGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_opgain") )
	{
		retval = 1;
		g_srs_tshd_settings.outputGain = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.outputGain < 0) || (g_srs_tshd_settings.outputGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD output gain (Ranges 0 to 100) - %d\n", g_srs_tshd_settings.outputGain);
			retval = -1;
		}
	}	
	else if ( !strcmp(argv[0], "-srs_TSHD_bpgain") )
	{
		retval = 1;
		g_srs_tshd_settings.bypassGain = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.bypassGain < 0) || (g_srs_tshd_settings.bypassGain > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD bypass gain (Ranges 0 to 100) - %d\n", g_srs_tshd_settings.bypassGain);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_defEn") )
	{
		retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.definitionEnabled = true;
		else
			g_srs_tshd_settings.definitionEnabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_deflvl") )
	{
		retval = 1;
		g_srs_tshd_settings.definitionLevel = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.definitionLevel < 0) || (g_srs_tshd_settings.definitionLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD definition level (Ranges 0 to 100) - %d\n", g_srs_tshd_settings.definitionLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_surlvl") )
	{    
		retval = 1;
		g_srs_tshd_settings.surroundLevel = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.surroundLevel < 0) || (g_srs_tshd_settings.surroundLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD surround level (Ranges 0 to 100) - %d\n", g_srs_tshd_settings.surroundLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowEn") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.wowHdEnabled = true;
		else
			g_srs_tshd_settings.wowHdEnabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowBitrate") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.wowHdHighBitRateEnabled = true;
		else
			g_srs_tshd_settings.wowHdHighBitRateEnabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowFocusEn") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.wowHdFocusEnabled = true;
		else
			g_srs_tshd_settings.wowHdFocusEnabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowMode") )
	{
		retval = 1;
		g_srs_tshd_settings.wowHdMode = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.wowHdMode < NEXUS_WowHdMode_eMono) || (g_srs_tshd_settings.wowHdMode > NEXUS_WowHdMode_eExtreme))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD WoW HD Mode (Ranges 0 to 3) - %d\n", g_srs_tshd_settings.wowHdMode);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowSpaceLvl") )
	{
		retval = 1;
		g_srs_tshd_settings.wowHdSpaceLevel = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.wowHdSpaceLevel < 0) || (g_srs_tshd_settings.wowHdSpaceLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD WoW HD Space level (Ranges 0 to 100) - %d\n", g_srs_tshd_settings.wowHdSpaceLevel);
			retval = -1;
		}
	}	
	else if ( !strcmp(argv[0], "-srs_TSHD_wowCenterLvl") )
	{
		retval = 1;
		g_srs_tshd_settings.wowHdCenterLevel = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.wowHdCenterLevel < 0) || (g_srs_tshd_settings.wowHdCenterLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD WoW HD Center level (Ranges 0 to 100) - %d\n", g_srs_tshd_settings.wowHdCenterLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_wowFocusLvl") )
	{
		retval = 1;
		g_srs_tshd_settings.wowHdFocusLevel = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.wowHdFocusLevel < 0) || (g_srs_tshd_settings.wowHdFocusLevel > 100))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD WoW HD Focus level (Ranges 0 to 100) - %d\n", g_srs_tshd_settings.wowHdFocusLevel);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_CertAppEn") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.certificationApp = true;
		else
			g_srs_tshd_settings.certificationApp = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_lfeEn") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
			g_srs_tshd_settings.inputLfeEnabled = true;
		else
			g_srs_tshd_settings.inputLfeEnabled = false;
	}
	else if ( !strcmp(argv[0], "-srs_TSHD_ipAcmode") )
	{
		retval = 1;
		g_srs_tshd_settings.inputAcmod = decOrhex_atoi(argv[1]);
		if ((g_srs_tshd_settings.inputAcmod < 0) || (g_srs_tshd_settings.inputAcmod > 13))
		{
			printf("error: illegal value for SRS Studio - Tru Surround HD input Acmode (Ranges 0 to 100) - %d\n", g_srs_tshd_settings.inputAcmod);
			retval = -1;
		}
	}
	else if ( !strcmp(argv[0], "-srs_cfgfile") )
	{
		retval = 3;
		fp_srsconfig = fopen(argv[1],"rb");
		if(fp_srsconfig == NULL)
		{
			printf("error: illegal value for SRS TSHD config file %s cannot be opened\n", argv[1]);
			retval = -1;
		}
		else
		{
			fclose(fp_srsconfig);
		}
	}
	else
		retval = -1;
		
	return retval;
}
#endif
#endif

#ifdef NEXUS_DV258_ENABLE

int parse_DV258_options (char **argv)
{

	int retval = -1;
	FILE *fp_dv258config;
	int tempval=0;
#if 0
	printf("DV258 : Parsing dv258 options :%s:%s:\n", argv[0], argv[1]);
#endif
	if ( !strcmp(argv[0], "-dv258_en") )
	{    
		retval = 1;
		tempval= decOrhex_atoi(argv[1]); 
		if ( tempval > 0)
		{
			DV258Settings.enabled = true;
		}
		else
		{
			DV258Settings.enabled = false;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_irl")) || (!strcmp(argv[0], "irl")) )
	{
		
    
		retval = 1;
		DV258Settings.inputReferenceLevel= decOrhex_atoi(argv[1]);
		
		if ((DV258Settings.inputReferenceLevel < 0) || (DV258Settings.inputReferenceLevel > 2080))
		{
			printf("ERROR: illegal value for DV258 input ref level - (Ranges 0 to 2080) - %d\n", DV258Settings.inputReferenceLevel);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_orl")) || (!strcmp(argv[0], "orl")) )
	{
		
    
		retval = 1;
		DV258Settings.outputReferenceLevel= decOrhex_atoi(argv[1]);
		
		if ((DV258Settings.outputReferenceLevel < 0) || (DV258Settings.outputReferenceLevel > 2080))
		{
			printf("ERROR: illegal value for DV258 op ref level- (Ranges 0 to 2080) - %d\n", DV258Settings.outputReferenceLevel);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_co")) || (!strcmp(argv[0], "co")) )
	{
		
    
		retval = 1;
		DV258Settings.calibrationOffset= decOrhex_atoi(argv[1]);
		
		if ((DV258Settings.calibrationOffset < 0) || (DV258Settings.calibrationOffset > 480))
		{
			printf("ERROR: illegal value for DV258 calibration offset- (Ranges -480 to 480) - %d\n", DV258Settings.calibrationOffset);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_mode")) || (!strcmp(argv[0], "mode")) )
	{    
		/* Volume Modeler Enable/Disable */
		retval = 1;
		tempval= decOrhex_atoi(argv[1]); 
		if ( tempval > 0)
		{
			DV258Settings.volumeModelerEnabled = true;
		}
		else
		{
			DV258Settings.volumeModelerEnabled = false;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_avm")) || (!strcmp(argv[0], "avm")) )
	{
		
    
		retval = 1;
		DV258Settings.analogVolumeLevel= decOrhex_atoi(argv[1]);
		
		if ((DV258Settings.analogVolumeLevel < -1536) || (DV258Settings.analogVolumeLevel > 480))
		{
			printf("ERROR: illegal value for DV258 analog volume level - (Ranges -1536 to 480) - %d\n", DV258Settings.analogVolumeLevel);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_dvm")) || (!strcmp(argv[0], "dvm")) )
	{
		
    
		retval = 1;
		DV258Settings.digitalVolumeLevel= decOrhex_atoi(argv[1]);
		
		if ((DV258Settings.digitalVolumeLevel < -1536) || (DV258Settings.digitalVolumeLevel > 480))
		{
			printf("ERROR: illegal value for DV258 digital volume level - (Ranges -1536 to 480) - %d\n", DV258Settings.digitalVolumeLevel);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_lvle")) || (!strcmp(argv[0], "lvle")) )
	{   
		/* Volume Leveler Enable/Disable */
		retval = 1;
		tempval= decOrhex_atoi(argv[1]);		
		if ( tempval > 0 )
		{
			DV258Settings.volumeLevelerEnabled = true;
		}
		else
		{
			DV258Settings.volumeLevelerEnabled = false;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_lvla")) || (!strcmp(argv[0], "lvla")) )
	{    
		retval = 1;
		DV258Settings.volumeLevelerAmount= decOrhex_atoi(argv[1]);
		
		if ((DV258Settings.volumeLevelerAmount < 0) || (DV258Settings.volumeLevelerAmount > 10))
		{
			printf("ERROR: illegal value for DV258 analog volume level - (Ranges 0 to 10) - %d\n", DV258Settings.volumeLevelerAmount);
			retval = -1;
		}
	}

	else if ( (!strcmp(argv[0], "-dv258_mse")) || (!strcmp(argv[0], "mse")) )
	{
		/* MidSide Mode Enable/Disable */
		retval = 1;
		tempval= decOrhex_atoi(argv[1]); 
		if ( tempval > 0)
		{
			DV258Settings.midsideProcessingEnabled = true;
		}
		else
		{
			DV258Settings.midsideProcessingEnabled = false;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_hmen")) || (!strcmp(argv[0], "hmen")) )
	{    
		retval = 1;
		tempval= decOrhex_atoi(argv[1]); 
		if ( tempval > 0)
		{
			DV258Settings.halfModeEnabled = true;
		}
		else
		{
			DV258Settings.halfModeEnabled = false;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_pregain")) || (!strcmp(argv[0], "pregain")) )
	{    
		retval = 1;
		DV258Settings.preGain= decOrhex_atoi(argv[1]);
		
		if ((DV258Settings.preGain < -480) || (DV258Settings.preGain > 480))
		{
			printf("ERROR: illegal value for DV258 pregain- (Ranges -480 to 480) - %d\n", DV258Settings.preGain);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-dv258_lime")) || (!strcmp(argv[0], "lime")) )
	{   
		/* Limiter Enable/Disable */
		retval = 1;
		tempval= decOrhex_atoi(argv[1]);		
		if ( tempval > 0 )
		{
			DV258Settings.limiterEnabled = true;
		}
		else
		{
			DV258Settings.limiterEnabled = false;
		}
	}
	/*else if ( !strcmp(argv[0], "-dv258_reset") )
	{    
		retval = 1;
		if ( !strcmp("on", argv[1]))
		{
			DV258Settings.reset = true;
		}
		else
		{
			DV258Settings.reset = false;
		}
	}*/
	else if ( !strcmp(argv[0], "-dv258_cfgfile") )
	{
		retval = 3;

		fp_dv258config = fopen(argv[1],"rb");

		if(fp_dv258config == NULL)
		{
			printf("ERROR: illegal value for DV258 config file %s cannot be opened\n", argv[1]);
			retval = -1;
		}
		else
		{
			fclose(fp_dv258config);
		}
	}
	else
	{
		printf("ERROR: Unable to parse %s : %s\n", argv[0], argv[1]);
		retval = -1;
	}
	return retval;
}
#endif

#ifdef NEXUS_DDRE_ENABLE
int parse_DDRE_options (char **argv)
{
	int retval = -1;
	FILE *fp_ddreconfig;

#if 0
	printf("DDRE : Parsing options :%s:%s:\n", argv[0], argv[1]);
#endif

	if ( (!strcmp(argv[0], "-ddre_dual_mono")) )
	{		
		retval = 1;
		DDRE_Settings.dualMonoMode = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.dualMonoMode > NEXUS_AudioDecoderDualMonoMode_eMix)
		{
			printf("error: illegal value for DDRE dualMonoMode (0-stereo-, 1-left and 2-right) - %d\n", DDRE_Settings.dualMonoMode);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-ddre_drc_mode")) )
	{		
		retval = 1;
		DDRE_Settings.drcMode = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.drcMode > NEXUS_AudioDecoderDolbyDrcMode_eRf)
		{
			printf("error: illegal value for DDRE drcMode (Ranges 0(line) and 1(rf) - %d\n", DDRE_Settings.drcMode);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-ddre_downmix_mode")) )
	{		
		retval = 1;
		DDRE_Settings.stereoDownmixMode = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.stereoDownmixMode > NEXUS_DolbyDigitalReencodeDownmixMode_eLoRo)
		{
			printf("error: illegal value for DDRE stereoDownmixMode (Ranges 0(LtRt) and 1(LoRo) - %d\n", DDRE_Settings.stereoDownmixMode);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-ddre_cut")) )
	{		
		retval = 1;
		DDRE_Settings.cut = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.cut > 100)
		{
			printf("error: illegal value for DDRE cut factor (0-100) : %d\n",DDRE_Settings.cut);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-ddre_boost")) )
	{		
		retval = 1;
		DDRE_Settings.boost = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.boost > 100)
		{
			printf("error: illegal value for DDRE boost factor (0-100) : %d\n",DDRE_Settings.boost);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-ddre_ext_pcm_enable")) )
	{		
		retval = 1;
		DDRE_Settings.externalPcmMode = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.externalPcmMode > 1)
		{
			printf("error: illegal value for DDRE externalPcmMode(Ranges 0-disable and 1-enable) - %d\n", DDRE_Settings.externalPcmMode);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-ddre_cert_mode_enable")) )
	{		
		retval = 1;
		DDRE_Settings.certificationMode = decOrhex_atoi(argv[1]);		
		if (DDRE_Settings.certificationMode > 1)
		{
			printf("error: illegal value for DDRE certificationMode(Ranges 0-disable and 1-enable) - %d\n", DDRE_Settings.certificationMode);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-ddre_comp_profile")) )
	{    
		retval = 1;
		DDRE_Settings.profile = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.profile > NEXUS_DolbyDigitalReencodeProfile_eSpeechCompression)
		{
			printf("error: illegal value for DDRE Compression Profile (Ranges 0 to 5) - %d\n", DDRE_Settings.profile);
			retval = -1;
		}
	}
	else if ( (!strcmp(argv[0], "-ddre_cmixlvl")) )
	{    
		retval = 1;
		DDRE_Settings.centerMixLevel = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.centerMixLevel > NEXUS_AudioAc3CenterMixLevel_eReserved)
		{
			printf("error: illegal value for DDRE center mix level (Ranges 0 to 3) - %d\n", DDRE_Settings.centerMixLevel);
			retval = -1;
		}
	
	}
	else if ( (!strcmp(argv[0], "-ddre_smixlvl")) )
	{    
		retval = 1;
		DDRE_Settings.surroundMixLevel= decOrhex_atoi(argv[1]);
		if (DDRE_Settings.surroundMixLevel > NEXUS_AudioAc3SurroundMixLevel_eReserved)
		{
			printf("error: illegal value for DDRE surround mix level (Ranges 0 to 3) - %d\n", DDRE_Settings.surroundMixLevel);
			retval = -1;
		}
	
	}
	else if ( (!strcmp(argv[0], "-ddre_dsurmod")) )
	{    
		retval = 1;
		DDRE_Settings.dolbySurround = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.dolbySurround > NEXUS_AudioAc3DolbySurround_eEncoded)
		{
			printf("error: illegal value for DDRE Dolby surround mode (Ranges 0 to 2) - %d\n", DDRE_Settings.dolbySurround);
			retval = -1;
		}	
	}
	else if ( (!strcmp(argv[0], "-ddre_dialog_lvl")) )
	{    
		retval = 1;
		DDRE_Settings.dialogLevel = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.dialogLevel > 31)
		{
			printf("error: illegal value for DDRE Dialog level (Ranges 0 to 31) - %d\n", DDRE_Settings.dialogLevel);
			retval = -1;
		}	
	}
	else if ( (!strcmp(argv[0], "-ddre_input_acmode")) )
	{    
		retval = 1;
		DDRE_Settings.inputAcmod = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.inputAcmod > 7)
		{
			printf("error: illegal value for DDRE inputAcmod (Ranges 0 to 7) - %d\n", DDRE_Settings.inputAcmod);
			retval = -1;
		}	
	}
	else if ( (!strcmp(argv[0], "-ddre_input_has_lfe")) )
	{    
		retval = 1;
		DDRE_Settings.inputHasLfe = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.inputHasLfe > 1)
		{
			printf("error: illegal value for DDRE inputHasLfe (Ranges 0 to 1) - %d\n", DDRE_Settings.inputHasLfe);
			retval = -1;
		}	
	}
	else if ( (!strcmp(argv[0], "-ddre_dv258_applied")) )
	{    
		retval = 1;
		DDRE_Settings.inputHasDv258Applied = decOrhex_atoi(argv[1]);
		if (DDRE_Settings.inputHasDv258Applied > 1)
		{
			printf("error: illegal value for DDRE inputHasDv258Applied (Ranges 0 to 1) - %d\n", DDRE_Settings.inputHasDv258Applied);
			retval = -1;
		}	
	}
	else if ( !strcmp(argv[0], "-ddre_cfgfile") )
	{
		retval = 3;
		fp_ddreconfig = fopen(argv[1],"rb");
		if(fp_ddreconfig == NULL)
		{
			printf("ERROR: illegal value for DDRE config file %s cannot be opened\n", argv[1]);
			retval = -1;
		}
		else
		{
			fclose(fp_ddreconfig);
		}
	}
	else
	{
		printf("ERROR: Unable to parse %s : %s\n", argv[0], argv[1]);
		retval = -1;
	}
	return retval;
}
#endif

void print_version_info (NEXUS_AudioDecoderHandle audioDecoder)
{
	NEXUS_AudioDecoderStatus decStatus;
	
	NEXUS_AudioDecoder_GetStatus (audioDecoder, &decStatus);
#if 1
	printf ("DTS Decoder Version : %02d.%02d.%02d.%02d.%02d.%02d\n", decStatus.algorithmProfileId,
																	 decStatus.algorithmVersionId[0],
																	 decStatus.algorithmVersionId[1],
																	 decStatus.algorithmVersionId[2],
																	 decStatus.algorithmVersionId[3],
																	 decStatus.firmwareVersionId);
#else
	printf ("Profile ID            :    0x%02X\n", decStatus.algorithmProfileId);
	printf ("Algo Version ID       :    0x%02X%02X%02X%02X\n", decStatus.algorithmVersionId[0],
												 decStatus.algorithmVersionId[1],
												 decStatus.algorithmVersionId[2],
												 decStatus.algorithmVersionId[3]);
	printf ("Firmware Version ID   :    0x%02X", decStatus.firmwareVersionId);
#endif
	
	return;	
}

#ifdef EQUALIZER_SUPPORT
int read_aeq_coeffs(char *filename, NEXUS_AudioEqualizerSettings *eqSettings)
{
	FILE *coeff_file;
	char line[300], *value;
	
	coeff_file = fopen (filename, "rt");
	
	if (coeff_file == NULL)
	{
		printf ("Could not open file for reading : %s\n", filename);
		return -1;
	}
	
	while (!feof (coeff_file))
	{
		fgets(line, 300, coeff_file);
		value = strchr(line, '=');
		if (value==NULL)
		{
			printf("Not a valid equalizer param : %s in %s\n", line, filename);
			continue;
		}

		*value = 0;
		value++;
		/* k=0 */
		if(!strcmp(line,"AEQ_F0_COEF_B0"))
			eqSettings->modeSettings.custom.coefficient0[0] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F0_COEF_B1"))
			eqSettings->modeSettings.custom.coefficient1[0] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F0_COEF_B2"))
			eqSettings->modeSettings.custom.coefficient2[0] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F0_COEF_A1"))
			eqSettings->modeSettings.custom.coefficient3[0] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F0_COEF_A2"))
			eqSettings->modeSettings.custom.coefficient4[0] = decOrhex_atoi(value);
		/* k=1 */
		else if(!strcmp(line,"AEQ_F1_COEF_B0"))
			eqSettings->modeSettings.custom.coefficient0[1] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F1_COEF_B1"))
			eqSettings->modeSettings.custom.coefficient1[1] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F1_COEF_B2"))
			eqSettings->modeSettings.custom.coefficient2[1] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F1_COEF_A1"))
			eqSettings->modeSettings.custom.coefficient3[1] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F1_COEF_A2"))
			eqSettings->modeSettings.custom.coefficient4[1] = decOrhex_atoi(value);
		/* k=2 */
		else if(!strcmp(line,"AEQ_F2_COEF_B0"))
			eqSettings->modeSettings.custom.coefficient0[2] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F2_COEF_B1"))
			eqSettings->modeSettings.custom.coefficient1[2] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F2_COEF_B2"))
			eqSettings->modeSettings.custom.coefficient2[2] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F2_COEF_A1"))
			eqSettings->modeSettings.custom.coefficient3[2] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F2_COEF_A2"))
			eqSettings->modeSettings.custom.coefficient4[2] = decOrhex_atoi(value);
		/* k=3 */
		else if(!strcmp(line,"AEQ_F3_COEF_B0"))
			eqSettings->modeSettings.custom.coefficient0[3] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F3_COEF_B1"))
			eqSettings->modeSettings.custom.coefficient1[3] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F3_COEF_B2"))
			eqSettings->modeSettings.custom.coefficient2[3] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F3_COEF_A1"))
			eqSettings->modeSettings.custom.coefficient3[3] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F3_COEF_A2"))
			eqSettings->modeSettings.custom.coefficient4[3] = decOrhex_atoi(value);
		/* k=4 */
		else if(!strcmp(line,"AEQ_F4_COEF_B0"))
			eqSettings->modeSettings.custom.coefficient0[4] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F4_COEF_B1"))
			eqSettings->modeSettings.custom.coefficient1[4] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F4_COEF_B2"))
			eqSettings->modeSettings.custom.coefficient2[4] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F4_COEF_A1"))
			eqSettings->modeSettings.custom.coefficient3[4] = decOrhex_atoi(value);
		else if(!strcmp(line,"AEQ_F4_COEF_A2"))
			eqSettings->modeSettings.custom.coefficient4[4] = decOrhex_atoi(value);
		else
		{
			printf("Unrecognized config param : %s in %s\n", line, filename);
			continue;
		}
	}
	
	return 0;
}
#endif

static void usage(void)
{
	printf("\nAudio Playback Application\n");
	printf("\nUsage:");
	printf("\napp_audio <args>\n");
	printf("\nArgument:              Description:");
	printf("\n-file <val>                             - Source content file");
	printf("\n-format <val>                           - Audio format");
	printf("\n		FOR	MPEG1-L1/L2			:mpeg");
	printf("\n		FOR	MPEG1-L3   			:mp3");	
	printf("\n		FOR	AAC/AAC-ADTS  		:aacadts");	
	printf("\n		FOR	AAC-LOAS   			:aacloas");	
	printf("\n		FOR	AACPLUS-LOAS  		:aac+loas");	
	printf("\n		FOR	AACPLUS-ADTS  		:aac+adts");	
	printf("\n		FOR	AC3        			:ac3"); 		
	printf("\n		FOR	AC3+ / DDP 			:ac3+"); 
	printf("\n		FOR	DTS        			:dts");		
	printf("\n		FOR	DVD-LPCM   			:lpcmdvd");	
	printf("\n		FOR	DVD-LPCM   			:lpcmhddvd");	
	printf("\n		FOR	DVD-LPCM   			:lpcmbluray");	
	printf("\n		FOR	DTSHD	   			:dtshd");	
	printf("\n		FOR	WMA STD	   			:wmastd");
	printf("\n		FOR	WMA STD TS 			:wmastdts");
	printf("\n		FOR	WMA-PRO   	 		:wmapro");
	printf("\n		FOR	AVS 	   	 		:avs");
	printf("\n		FOR	PCM		   	 		:pcm");	
	printf("\n		FOR	PCM-WAV    			:pcmwav");	
	printf("\n		FOR	AMR/AMR_NB	 		:amrnb");	
	printf("\n		FOR	AMR_WB	   			:amrwb");	
	printf("\n		FOR	DRA 	   			:dra");	
	printf("\n		FOR	REAL AUDIO 			:real"); 
	printf("\n		FOR	ADPCM      			:adpcm");		
	printf("\n		FOR	SBC 	   			:sbc");	
	printf("\n		FOR	DTS_LEGACY/DTS_CD	:dtscd"); 
	printf("\n		FOR	DTS_EXPRESS			:dtsexp");
	printf("\n		FOR	VORBIS	   			:vorbis");
	printf("\n		FOR	LPCM	   			:lpcm1394");	
	printf("\n		FOR	G711	   			:g711");	
	printf("\n		FOR	G723_1	   			:g723");	
	printf("\n		FOR	G726	   			:g726");	
	printf("\n		FOR	G729	   			:g729");	
	printf("\n		FOR	FLAC	   			:flac");	
	printf("\n		FOR	MLP 	   			:mlp");	
	printf("\n		FOR	APE 	   			:ape");	
	printf("\n-pcr_pid <val>                          - PCR PID value in hex or decimal. To be used with streamer input");
	printf("\n-aud_pid <val>                          - AUDIO PID value in hex or decimal. To be used with streamer input");
	printf("\n-loop <0..N>                            - Count for no. of loops of stream playback, To be used with file playback only");
	printf("\n-stereo_disable						  - Disables stereo output on DAC and SPDIF");
	printf("\n-spdif <pcm|comp|enc_ac3|enc_dts \
	        \n       |pcm_multi_lr|pcm_multi_lsrs|pcm_multi_clfe \
			\n       |pcm_simul>                   - Spdif output configuration");
	printf("\n-hdmi <pcm|pcm51|comp|enc_ac3|enc_dts>  - HDMI output configuration (only on demand)");
	printf("\n-secondary_info                         - Option to start an assocaited context. All parameters will be used from the primary channel unless otherwise specified");
#ifdef CUSTOM_VOICE_ENABLE
	printf("\n-cv_enable                              - Enables Custom Voice Post Processing on the Primary or Mixed output");
#endif
#if (defined (NEXUS_DOLBY_VOLUME_ENABLE) || defined (NEXUS_DV258_ENABLE))
	printf("\n-dv_enable                              - Enables Dolby Volume Post Processing on the Primary or Mixed output");
#endif
#ifdef NEXUS_DDRE_ENABLE
	printf("\n\nDolby Digitl Reencode specific options:");
	printf("\n-------------------------------------------");
	printf("\n-ddre_enable                            - Enables DDRE Post Processing on the Primary or Mixed output");
	printf("\n-ddre_dual_mono                         - DDRE Dualmono Mode");
	printf("\n                                               0-Stereo; 1-Left; 2-Right");
	printf("\n-ddre_drc_mode                          - DDRE DRC Mode");
	printf("\n                                               0-Line; 1-RF");
	printf("\n-ddre_downmix_mode                      - DDRE stereo downmix mode");
	printf("\n                                               0-LtRt; 1-LoRo");
	printf("\n-ddre_cut                               - DDRE Cut factor");
	printf("\n-ddre_boost                             - DDRE Boost factor");
	printf("\n-ddre_ext_pcm_enable                    - DDRE Enable/Disable external pcm mode");
	printf("\n                                               0-Disable; 1-Enable");
	printf("\n-ddre_cert_mode_enable                  - Enable/Disable DDRE certification mode");
	printf("\n                                               0-Disable; 1-Enable");
	printf("\n-ddre_input_acmode                      - DDRE input acmode <0-7>");
	printf("\n-ddre_input_has_lfe                     - DDRE input has lfe");
	printf("\n                                               0-Disable; 1-Enable");
	printf("\n-ddre_dv258_applied                     - DDRE input DV258 processed");
	printf("\n                                               0-Disable; 1-Enable");
	printf("\n-ddre_comp_profile <0-5>                - DDRE compression profile [1]");
	printf("\n                                            %d - No Compression", NEXUS_DolbyDigitalReencodeProfile_eNoCompression);
	printf("\n                                            %d - Film Standard", NEXUS_DolbyDigitalReencodeProfile_eFilmStandardCompression);
	printf("\n                                            %d - Film Light", NEXUS_DolbyDigitalReencodeProfile_eFilmLightCompression);
	printf("\n                                            %d - Music Standard", NEXUS_DolbyDigitalReencodeProfile_eMusicStandardCompression);
	printf("\n                                            %d - Music Light", NEXUS_DolbyDigitalReencodeProfile_eMusicLightCompression);	
	printf("\n                                            %d - Speech", NEXUS_DolbyDigitalReencodeProfile_eSpeechCompression);
	printf("\n-ddre_cmixlvl <0-2>                     - DDRE Center mix level [0]");
	printf("\n                                            %d - CenterMixLevel_e3", NEXUS_AudioAc3CenterMixLevel_e3);
	printf("\n                                            %d - CenterMixLevel_e4_5", NEXUS_AudioAc3CenterMixLevel_e4_5);
	printf("\n                                            %d - CenterMixLevel_e6", NEXUS_AudioAc3CenterMixLevel_e6);
	printf("\n-ddre_smixlvl <0-2>                   - DDRE Surround mix level [0]");
	printf("\n                                            %d - SurroundMixLevel_e3", NEXUS_AudioAc3SurroundMixLevel_e3);
	printf("\n                                            %d - SurroundMixLevel_e6", NEXUS_AudioAc3SurroundMixLevel_e6);
	printf("\n                                            %d - SurroundMixLevel_e0", NEXUS_AudioAc3SurroundMixLevel_e0);
	printf("\n-ddre_dsurmod <0-2>                     - DDRE AC3 Dolby Surround Mode [0]");
	printf("\n                                            %d - DolbySurround_eNotIndicated", NEXUS_AudioAc3DolbySurround_eNotIndicated);
	printf("\n                                            %d - DolbySurround_eNotEncoded", NEXUS_AudioAc3DolbySurround_eNotEncoded);
	printf("\n                                            %d - DolbySurround_eEncoded", NEXUS_AudioAc3DolbySurround_eEncoded);
	printf("\n-ddre_dialog_lvl <0-31>                 - Dialog level of incoming PCM in dB [31]\n");
#endif
#ifdef NEXUS_3DSURROUND_ENABLE
	printf("\n-3dsurr_enable                          - Enables BRCM 3D surround Post Processing on the Primary or Mixed output");
#endif
	printf("\n-tv_enable                              - Enables Tru Volume Post Processing on the Primary or Mixed output");
	printf("\n-targetsync_enable                      - Enables Target Sync Validation in Datasync. [disabled]");
#if NEXUS_NUM_I2S_OUTPUTS > 0
    printf("\n-multi_i2s                              - Enables multi channel output on i2s. Used for DTS certification [disabled]");
#endif
	printf("\n-nchs <val>                             - Specify the number of channels up to 8"); /* Not required */
	printf("\n-csr <val>                              - Set the content's SampleRate"); /* Not required */
	printf("\n                                            - Supported values: 192, 96, 48, 44.1, 32, 24, 22.05, 16, 12, 11.025");
	printf("\n-mono_mode <left|right|stereo|mixed>    - Dual Mono mode [stereo]");
	
	printf("\n\nDolby Digitl Plus decoder specific options:");
	printf("\n-------------------------------------------");
	printf("\n-substream_id <0|1|2|3>                 - Substrem ID to decode [0]");
	printf("\n-downmix_mode <ltrt|loro|auto>          - Downmix mode for 2/0 or stereo downmix channels [auto]");
	printf("\n-cut <0-100>                            - DRC HDR cut scale factor [100]");
	printf("\n-boost <0-100>                          - DRC LDR boost scale factor [100]");
	printf("\n-drc_mode <line|rf|custA|custD>         - DRC compression mode [line]");
#ifdef NEXUS_MS11_SUPPORT	
	printf("\n-dwmix_cut <0-100>                            - Simul downmix: DRC HDR cut scale factor [100]");
	printf("\n-dwmix_boost <0-100>                          - Simul downmix: DRC LDR boost scale factor [100]");
	printf("\n-dwmix_drc_mode <line|rf|custA|custD>         - Simul downmix: DRC compression mode [line]");
#endif

	printf("\n\nDolby Pulse decoder specific options:");
	printf("\n-------------------------------------");
	printf("\n-downmix_mode <matrix|arib|ltrt|loro>   - Downmix mode for 2/0 or stereo downmix channels [matrix]");
	printf("\n-cut <0-100>                            - DRC HDR cut scale factor [100]");
	printf("\n-boost <0-100>                          - DRC LDR boost scale factor [100]");
	printf("\n-drc_mode <line|rf>                     - DRC compression mode [line]");
	printf("\n-dn <0-127>                             - Default dialnorm value [108]");

	printf("\n\nDTS decoder specific options:");
	printf("\n-----------------------------");
	printf("\n-downmix_mode <ltrt|loro|auto>          - Downmix mode for 2/0 or stereo downmix channels [auto]");
	printf("\n-drc_enable <on|off>                    - DRC enable/disable [off]");
	printf("\n-cut <0-100>                            - DRC HDR cut scale factor [100]");
	printf("\n-boost <0-100>                          - DRC LDR boost scale factor [100]");
	printf("\n-mix_lfe                                - Enables mixing LFE to primary while downmixing, when Lfe output is disabled. [Disabled]");

#if (defined (NEXUS_ADV_ENABLE) || defined (NEXUS_ABX_ENABLE))
	printf("\n\nAudyssey specific options:");
	printf("\n-----------------------------");
#endif
#ifdef NEXUS_ADV_ENABLE
	printf("\n-adv <on|off>                           - ADV Enable/Disable [off]");
	printf("\n-adv_gain <1-240>                       - ADV Gain Compression Value [180]");
	printf("\n-adv_volume_scaling <on|off>            - ADV Volume scaling enable [on]");
	printf("\n-adv_volume <0-(-500)>                  - ADV Volume value [0]");
	printf("\n-adv_reinit <on|off>                    - ADV Re-initialize [off]");
	printf("\n-adv_dyn_surr_gain <on|off>             - ADV surround channel gain enable [on]");
	printf("\n-adv_hilo_comp <on|off>                 - ADV HiLoCompress value [off]");
	printf("\n-adv_ch_mask <0-255>                    - ADV Channel Mask [255]");
	printf("\n-adv_headroom_offset <(-512)-512>       - ADV Headroom offset in dB [0]");
	printf("\n-adv_dyn_eq_mode <on|off|rampoff>       - ADV Dynamic Equalizer Mode [off]");
#endif
#ifdef NEXUS_ABX_ENABLE
	printf("\n-abx <on|off>                           - ABX Enable/Disable [off]");
	printf("\n-abx_harmonicgain <0-30>                - ABX Harmonic Gain Value [6]");
	printf("\n-abx_filterset <1-2>                    - ADV Filter set value (1 = Roll-off Lower, 2 = Roll-off Higher) [1]");
	printf("\n-abx_drygain <0-30>                     - ADV Dry Gain Value [0]");
#endif

#ifdef NEXUS_AVL_ENABLE
	printf("\n\nAVL specific options:");
	printf("\n-----------------------------");
	printf("\n-avl <on|off>                           - AVL Enable/Disable [off]");
	printf("\n-avl_target <in dB>					  - AVL target in dB [default is usually -6dB]");
	printf("\n-avl_lowerBound <in dB>				  - AVL lowerBound in dB [default is usually -18dB]");
	printf("\n-avl_fixedBoost <in dB>				  - AVL fixedBoost in dB [default is usually 6dB]");

#endif

#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
	printf("\n\nSRS specific options:");
	printf("\n--------------------------------");
	printf("\n\n-srs_cfgfile <FileName>    - All the below mentioned Studio Sound, TVOL and TSHD params \n");
	printf("                                     can also be given in a text file  eg: \n");
	printf("                                     -srs_TSHD_enable=on\n");
	printf("                                     -srs_TSHD_headphoneEn=off\n"); 
	printf("\n\nSRS StudioSound specific options:");
	printf("\n--------------------------------");
	printf("\n-srs <on|off>             - SRS Studio Module is enabled [off]");
	printf("\n-srs_mode <0-2>           - StudioSound  mode [0]");
	printf("\n                              0 - while CS, TD, TVOL, TSHD and HPF_front");	
	printf("\n                              1 - Not valid anymore");
	printf("\n                              2 - while GEQ, PEQ, HL and HPF_back");
	printf("\n-srs_igain <0-100>        - StudioSound Input gain  [100]");
	printf("\n-srs_ogain <0-100>        - StudioSound Output gain [100]");
	printf("\n-srs_bGain <0-100>        - StudioSound Bypass gain [100]");
	printf("\n-srs_hGain <0-100>        - StudioSound headroom gain [100]");
#endif
#if (defined NEXUS_SRSSTUDIOSOUND_ENABLE) || (defined NEXUS_SRSTRUVOL_ENABLE)
	printf("\n\nSRS TruVolume specific options:");
	printf("\n--------------------------------");	
	printf("\n-srs_TVEnable <on|off>        - Tru Volume module Enable [off]"); 	/*PI -> true */
	printf("\n-srs_TV_enNormGain <on|off>   - Enable normal gain processing [on]");
	printf("\n-srs_TV_igain <0-3200>        - Input gain  [100]");
	printf("\n-srs_TV_ogain <0-3200>        - Output gain [50]"); 
	printf("\n-srs_TV_mode <0-2>            - Processing Mode [0->Light]");
	printf("\n                                      0- NEXUS_TruVolumeMode_eLight");
	printf("\n                                      1- NEXUS_TruVolumeMode_eNormal");
	printf("\n                                      2- NEXUS_TruVolumeMode_eHeavy");
	/*printf("\n-srs_TV_bgain <0-100>         - Bypass gain %  0 to 100 ");
	printf("\n-srs_TV_blksize <0-3>     - Block Size for processing");    
	printf("\n                                      0 - NEXUS_TruVolumeBlockSize_e256");
	printf("\n                                      1 - NEXUS_TruVolumeBlockSize_e512");
	printf("\n                                      2 - NEXUS_TruVolumeBlockSize_e768");
	printf("\n                                      3 - NEXUS_TruVolumeBlockSize_e1024");
	printf("\n-srs_TV_spkrres <0-5>         - Speaker resolution"); 
	printf("\n                                      0- TruVolumeSpeakerResolution_e20Hz");
	printf("\n                                      1- TruVolumeSpeakerResolution_e40Hz");
	printf("\n                                      2- TruVolumeSpeakerResolution_e110Hz");
	printf("\n                                      3- TruVolumeSpeakerResolution_e200Hz");
	printf("\n                                      4- TruVolumeSpeakerResolution_e315Hz");
    printf("\n                                       5- TruVolumeSpeakerResolution_e410Hz");
	printf("\n-srs_TV_reflvl <265-8388607>        - Reference level 0x109 to 0x7fffff");
	printf("\n-srs_TV_maxgain <0-100>       - Max gain control % 0 to 100");
	printf("\n-srs_TV_enDCfilt <on|off>     - The DC notch filter enable ");
	printf("\n-srs_TV_enNoiseMgr <on|off>   - Enable/Disable noise manager ");
	printf("\n-srs_TV_noisethres <0-200>    - Noise manager threshold, 0 to 200 ");
	printf("\n-srs_TV_enNorm <on|off>       - Enable normalizer");
	printf("\n-srs_TV_calibrate <0-25600>   - Calibrate 0 to 25600");	*/
#endif
#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
	printf("\n-srs_TV_Standalone <on|off>   - When enabled TVOL is configured in standalone mode and not as part of StudioSound [off]");
#endif
#if (defined NEXUS_SRSSTUDIOSOUND_ENABLE) || (defined NEXUS_SRSHD_ENABLE)
	printf("\n\nSRS TSHD specific options:");
	printf("\n--------------------------------");
	printf("\n-srs_TSHD_enable <on|off>         - SRS TruSurround HD Enable [off]");  /* PI -> true */
	printf("\n-srs_TSHD_headphoneEn <on|off>    - SRS TruSurround HD headphone Enable [off]"); 
	printf("\n-srs_TSHD_mToSEn <on|off>         - SRS TruSurround HD mono To Stereo Enable [on]");
	printf("\n-srs_TSHD_dialClarityEn <on|off>  - SRS TruSurround HD dialog Clarity Enable [on]");
	printf("\n-srs_TSHD_dialClarityLvl <0-100>  - SRS TruSurround HD dialog Clarity Level[50]");
	printf("\n-srs_TSHD_truBassEn <on|off>      - SRS TruSurround HD Tru Bass Enable [on]");
	printf("\n-srs_TSHD_trubassLvl <0-100>      - SRS TruSurround HD Tru Bass Level [30]");
	printf("\n-srs_TSHD_spkrSize <0-7>          - SRS TruSurround HD Speaker Size [3->150hz]");
	printf("\n                                        0 - NEXUS_TruBassSpeakerSize_e40Hz");
	printf("\n                                        1 - NEXUS_TruBassSpeakerSize_e60Hz");
	printf("\n                                        2 - NEXUS_TruBassSpeakerSize_e100Hz");
	printf("\n                                        3 - NEXUS_TruBassSpeakerSize_e150Hz");
	printf("\n                                        4 - NEXUS_TruBassSpeakerSize_e200Hz");
	printf("\n                                        5 - NEXUS_TruBassSpeakerSize_e250Hz");
	printf("\n                                        6 - NEXUS_TruBassSpeakerSize_e300Hz");
	printf("\n                                        7 - NEXUS_TruBassSpeakerSize_e400Hz");
	printf("\n-srs_TSHD_trubassMode <0-1>       - SRS TruSurround HD Tru Bass Mode [1->Stereo]");
	printf("\n                                        0 - NEXUS_TruBassMode_eMono");
	printf("\n                                        1 - NEXUS_TruBassMode_eStereo");
	printf("\n-srs_TSHD_ipgain <0-100>          - SRS TruSurround HD Input gain [50]");
	printf("\n-srs_TSHD_opgain <0-100>          - SRS TruSurround HD Output gain [100]");
	printf("\n-srs_TSHD_bpgain <0-100>          - SRS TruSurround HD Bypass gain [100]");
	printf("\n-srs_TSHD_defEn <on|off>          - SRS TruSurround HD Definition Enable [on]");
	printf("\n-srs_TSHD_deflvl <0-100>          - SRS TruSurround HD Definition level [30]");
	printf("\n-srs_TSHD_surlvl <0-100>          - SRS TruSurround HD Surround level [60]");
	printf("\n-srs_TSHD_wowEn <on|off>          - SRS TruSurround HD WOW HD Enable [on]");
	printf("\n-srs_TSHD_wowBitrate <on|off>     - SRS TruSurround HD WOW HD High bitrate Enable [on]");
	printf("\n-srs_TSHD_wowFocusEn <on|off>     - SRS TruSurround HD WOW HD Focus Enable [on]");
	printf("\n-srs_TSHD_wowMode                 - SRS TruSurround HD WOW HD mode [2->Stereo]");
	printf("\n                                        0 - NEXUS_WowHdMode_eMono");
    printf("\n                                        1 - NEXUS_WowHdMode_eSingleSpeaker");
    printf("\n                                        2 - NEXUS_WowHdMode_eStereo");
    printf("\n                                        3 - NEXUS_WowHdMode_eExtreme");
	printf("\n-srs_TSHD_wowSpaceLvl <0-100>     - SRS TruSurround HD WOW HD Space Level [80]");
	printf("\n-srs_TSHD_wowCenterLvl <0-100>    - SRS TruSurround HD Center Level [50]");
	printf("\n-srs_TSHD_wowFocusLvl <0-100>     - SRS TruSurround HD Focus Level [40]");
	printf("\n-srs_TSHD_CertAppEn <on|off>      - To enable certification mode [off]");
	printf("\n-srs_TSHD_lfeEn <on|off>          - Enable LFE content in the Output [off]");
	printf("\n-srs_TSHD_ipAcmode <0-13>         - Input channel config [1->2_0_LR]");
	/*
	printf("\n                                      0- NEXUS_TruSurroundHdAcmod_e1_0_C,          / C /");
	*/
    printf("\n                                      1- NEXUS_TruSurroundHdAcmod_e2_0_LR, / L,R /");
    /*
	printf("\n                                      2- NEXUS_TruSurroundHdAcmod_e3_0_LCR,        / L,C,R /");
    printf("\n                                      3- NEXUS_TruSurroundHdAcmod_e2_1_LRS,        / L,R,S /");
    printf("\n                                      4- NEXUS_TruSurroundHdAcmod_e3_1_LCRS,       / L,C,R,S /");
    printf("\n                                      5- NEXUS_TruSurroundHdAcmod_e2_2_LRLsRs,     / L,R,LS,RS /");
	*/
    printf("\n                                      6- NEXUS_TruSurroundHdAcmod_e3_2_LCRLsRs, / L,C,R,LS,RS /");
    /*
	printf("\n                                      7- NEXUS_TruSurroundHdAcmod_e3_3_LCRLsRsCs,  / L,C,R,LS,RS,CS /");
    printf("\n                                      8- NEXUS_TruSurroundHdAcmod_e3_2_BSDigital,  / L,C,R,LS,RS /");
	*/
    printf("\n                                      9- NEXUS_TruSurroundHdAcmod_eLtRt, / L,R /");
    /*
	printf("\n                                      10-NEXUS_TruSurroundHdAcmod_ePLII_Movie,     / L,C,R,LS,RS /");
    printf("\n                                      11-NEXUS_TruSurroundHdAcmod_ePLII_Music,     / L,C,R,LS,RS /");
    printf("\n                                      12-NEXUS_TruSurroundHdAcmod_ePassiveMatrix,  / L,R /");
    printf("\n                                      13- NEXUS_TruSurroundHdAcmod_eCSII,          / L,C,R,Ls,Rs,Cs /");
	*/
#endif
#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
	printf("\n-srs_THD_Standalone <on|off>      - When enabled TSHD is configured in standalone mode and not as part of StudioSound [off]");
    	
	printf("\n\nSRS High Pass Filter (HPF) specific options:");
	printf("\n--------------------------------");
#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM
	printf("\n-srs_HPF_front_en <on|off>     -  Enable High pass filter which is prior to TruVol [off]"); /*PI Def: true*/
	printf("\n-srs_HPF_front_mode <0-1>      -  Mode of High Pass Filter which is prior to TruVol [0]");
	printf("\n                                          0 - User Coeff; 1- Generate coeffs");
	printf("\n-srs_HPF_front_mode0_coeff_file <file name> - HPF Front Filter coeff file for Mode-0");
	
	printf("\n-srs_HPF_back_en <on|off>      -  Enable High pass filter which is next to TEQ [off]"); /*PI Def: true*/
	printf("\n-srs_HPF_back_mode <0-1>       -  Mode of High Pass Filter which is next to TEQ [0]");
	printf("\n                                         0 - User Coeff; 1- Generate coeffs");
	printf("\n-srs_HPF_back_mode0_coeff_file <file name> - HPF back Filter coeff file for Mode-0");	
#else
	printf("\n-srs_HPF_front_en <on|off>     -  Enable High pass filter which is prior to TruVol [off]"); /*PI Def: true*/
	printf("\n-srs_HPF_front_mode <0-3>      -  Mode parameter for High Pass Filter which is prior to TruVol [3->180hz]");
	printf("\n                                       0 - NEXUS_SrsHighPassFilterMode_e60Hz4thOrder");
	printf("\n                                       1 - NEXUS_SrsHighPassFilterMode_e80Hz4thOrder");
	printf("\n                                       2 - NEXUS_SrsHighPassFilterMode_e150Hz4thOrder");
	printf("\n                                       3 - NEXUS_SrsHighPassFilterMode_e180Hz4thOrder (Default)");
	printf("\n-srs_HPF_back_en <on|off>      -  Enable High pass filter which is next to TEQ [off]"); /*PI Def: true*/
	printf("\n-srs_HPF_back_mode <0-3>       -  Mode parameter for High Pass Filter which is next to TEQ [3->180hz]");
	printf("\n                                       0 - NEXUS_SrsHighPassFilterMode_e60Hz4thOrder");
	printf("\n                                       1 - NEXUS_SrsHighPassFilterMode_e80Hz4thOrder");
	printf("\n                                       2 - NEXUS_SrsHighPassFilterMode_e150Hz4thOrder");
	printf("\n                                       3 - NEXUS_SrsHighPassFilterMode_e180Hz4thOrder (Default)");
#endif
	
	printf("\n\nSRS Circle Surround (CS) specific options:");
	printf("\n--------------------------------");
	printf("\n-srs_CS_enable <on|off>        - SRS circle Surround Enable [off]"); /*PI Def: true*/
	printf("\n-srs_CS_ipgain <125-1000>      - SRS circle Surround Input Gain [1000]");
	printf("\n-srs_CS_mode <0-1>             - SRS circle Surround mode [1-Music]");
	printf("\n                                     0 - NEXUS_CircleSurroundMode_eCinema");
    printf("\n                                     1 - NEXUS_CircleSurroundMode_eMusic");
	printf("\n-srs_CS_opgainF <89-4000>      - SRS circle Surround output gain front  [1000]");
	printf("\n-srs_CS_opgainR <89-4000>      - SRS circle Surround output gain Rear   [1000]");
	printf("\n-srs_CS_opgainC <89-4000>      - SRS circle Surround output gain Center [1000]");
	printf("\n-srs_CS_opgainSW <89-4000>     - SRS circle Surround output gain SubWoofer [1000]");
	
	printf("\n\nSRS TruDialog (TD) specific options:");
	printf("\n--------------------------------");
	printf("\n-srs_TD_en <on|off>            - SRS TD Enable [off]"); /*PI Def: true*/
	printf("\n-srs_TD_igain <0 - 100>        - SRS TD Input Gain.  [100]");
	printf("\n-srs_TD_ogain <0 - 100>        - SRS TD Output Gain. [100]");
	printf("\n-srs_TD_bgain <0 - 100>        - SRS TD Bypass Gain. [100]");
	printf("\n-srs_TD_pgain <50 - 300>       - SRS TD Final calculated gain applied to final output signal. [150]");
	printf("\n-srs_TD_egain <0 - 100>        - SRS TD Amount of Dialog Clarity enhancement applied to input signal [100]");
	
	printf("\n\nSRS Graphic Equalizer (GEQ) specific options:");
	printf("\n--------------------------------");
	printf("\n-srs_geq_lenable <on|off>      - SRS GEQ Enable Left channel processing [off]"); /*PI Def: false*/
	printf("\n-srs_geq_renable <on|off>      - SRS GEQ Enable Right channel processing [off]"); /*PI Def: false*/
	printf("\n-srs_geq_bandmode <0 - 1>      - SRS GEQ filter configuration [0->5Band]");
	printf("\n                                     0 - NEXUS_SrsGraphicEqBandMode_eFiveBand");
	printf("\n                                     1 - NEXUS_SrsGraphicEqBandMode_eTenBand");
	printf("\n-srs_geq_lbgain0 <316-3160>    - SRS GEQ Left channel band gain. [1000]");
	printf("\n                                     Range for  5-band: 316 to 3160. Default: 1000");
	printf("\n                                     Range for 10-band: 251 to 4000. Default: 1000");
	printf("\n-srs_geq_lbgain1               - as above");
	printf("\n-srs_geq_lbgain2               - as above");
	printf("\n-srs_geq_lbgain3               - as above");
	printf("\n-srs_geq_lbgain4               - as above");
	printf("\n-srs_geq_lbgain5               - as above");
	printf("\n-srs_geq_lbgain6               - as above");
	printf("\n-srs_geq_lbgain7               - as above");
	printf("\n-srs_geq_lbgain8               - as above");
	printf("\n-srs_geq_lbgain9               - as above");
	printf("\n-srs_geq_rbgain0 <316-3160>    - SRS GEQ Right channel band0 gain. [1000]");
	printf("\n                                     Range for  5-band: 316 to 3160. Default: 1000");
	printf("\n                                     Range for 10-band: 251 to 4000. Default: 1000");
	printf("\n-srs_geq_rbgain1               - as above");
	printf("\n-srs_geq_rbgain2               - as above");
	printf("\n-srs_geq_rbgain3               - as above");
	printf("\n-srs_geq_rbgain4               - as above");
	printf("\n-srs_geq_rbgain5               - as above");
	printf("\n-srs_geq_rbgain6               - as above");
	printf("\n-srs_geq_rbgain7               - as above");
	printf("\n-srs_geq_rbgain8               - as above");
	printf("\n-srs_geq_rbgain9               - as above");
	printf("\n-srs_geq_ipgain <316 - 1000>   - SRS GEQ Input Gain [316]");
	printf("\n                                     Range for  5-band: 316 to 1000. Default: 316");
	printf("\n                                     Range for 10-band: 251 to 1000. Default: 316 (srs-251)");
	printf("\n-srs_geq_opgain <0-100>        - SRS GEQ Output gain [100]");
	printf("\n-srs_geq_bpgain <0-100>        - SRS GEQ Bypass gain [100]");
	
	printf("\n\nSRS TruEqualizer (TEQ) (or Parametric Equalizer PEQ) specific options:");
	printf("\n--------------------------------");
    printf("\n-srs_teq_lenable <on|off>      - SRS TEQ Enable Left channel processing [off]"); /*PI Def: true*/
	printf("\n-srs_teq_renable <on|off>      - SRS TEQ Enable Left channel processing [off]"); /*PI Def: true*/
	printf("\n-srs_teq_lbEn0   <on|off>      - SRS TEQ Enable Left Band0 processing [on]");
	printf("\n-srs_teq_lbEn1   <on|off>      - SRS TEQ Enable Left Band1 processing [on]");
	printf("\n-srs_teq_lbEn2   <on|off>      - SRS TEQ Enable Left Band2 processing [on]");
	printf("\n-srs_teq_lbEn3   <on|off>      - SRS TEQ Enable Left Band3 processing [on]");
	printf("\n-srs_teq_lbEn4   <on|off>      - SRS TEQ Enable Left Band4 processing [on]");
	printf("\n-srs_teq_lbEn5   <on|off>      - SRS TEQ Enable Left Band5 processing [on]");
	printf("\n-srs_teq_lbEn6   <on|off>      - SRS TEQ Enable Left Band6 processing [on]");
	printf("\n-srs_teq_lbEn7   <on|off>      - SRS TEQ Enable Left Band7 processing [on]");
    printf("\n-srs_teq_rbEn0   <on|off>      - SRS TEQ Enable Right Band0 processing [on]");
	printf("\n-srs_teq_rbEn1   <on|off>      - SRS TEQ Enable Right Band1 processing [on]");
	printf("\n-srs_teq_rbEn2   <on|off>      - SRS TEQ Enable Right Band2 processing [on]");
	printf("\n-srs_teq_rbEn3   <on|off>      - SRS TEQ Enable Right Band3 processing [on]");
	printf("\n-srs_teq_rbEn4   <on|off>      - SRS TEQ Enable Right Band4 processing [on]");
	printf("\n-srs_teq_rbEn5   <on|off>      - SRS TEQ Enable Right Band5 processing [on]");
	printf("\n-srs_teq_rbEn6   <on|off>      - SRS TEQ Enable Right Band6 processing [on]");
	printf("\n-srs_teq_rbEn7   <on|off>      - SRS TEQ Enable Right Band7 processing [on]");
	printf("\n-srs_teq_ipgain  <0-1000>      - SRS TEQ Input Gain [100]");
	printf("\n-srs_teq_opgain  <0-1000>      - SRS TEQ Output Gain [100]");
	printf("\n-srs_teq_bpgain  <0-1000>      - SRS TEQ Bypass Gain [100]");
#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM	
	printf("\n-srs_teq_mode    <0-1>         - SRS TEQ Coeff Generation Mode");
	printf("\n                                     0 - User Coeff; 1- Generate coeffs");
	printf("\n-srs_teq_mode0_coeff_file <file name> - SRS TEQ Filter coeff file for Mode-0");
#else
	printf("\n-srs_teq_mode    <0-1>         - SRS TEQ Filter Mode [0]");
#endif
	
	printf("\n\nSRS Hard Limiter (HL) specific options:");
	printf("\n--------------------------------");
	printf("\n-srs_hl_en <on|off>            - SRS HL Enable processing [off]"); /*PI Def: false*/
	printf("\n-srs_hl_blksize <0-3>          - SRS HL Block Size [0->256]");
	printf("\n                                     0 - NEXUS_SrsHardLimiterBlockSize_e256");
	printf("\n                                     1 - NEXUS_SrsHardLimiterBlockSize_e512");
	printf("\n                                     2 - NEXUS_SrsHardLimiterBlockSize_e768");
	printf("\n                                     3 - NEXUS_SrsHardLimiterBlockSize_e1024");
	printf("\n-srs_hl_ipgain <0-400>         - SRS HL Input Gain  [100]");
	printf("\n-srs_hl_opgain <0-400>         - SRS HL Output Gain [100]");
	printf("\n-srs_hl_bpgain <0-100>         - SRS HL Bypass Gain [100]");
	printf("\n-srs_hl_boost  <1-256000>      - SRS HL gain boost  [2820]");
	printf("\n-srs_hl_level  <3-100000>      - SRS HL HardLimit   [100000]");
	printf("\n-srs_hl_delay  <6-22>          - SRS HL DelayLength [22]");
	
#endif

#ifdef NEXUS_DV258_ENABLE
	printf("\n\nDV258 specific options:");
	printf("\n--------------------------------\n");
	printf("-dv258_en   <0 | 1>          - DV258 Enable [0-off]\n");
	printf("-dv258_irl  <0 - 2080>       - DV258 Input reference level  [1360]\n");
	printf("-dv258_orl  <0 - 2080>       - DV258 Output reference level [1360]\n");
	printf("-dv258_co   <-480 - +480>    - DV258 Calibration Offset     [0]\n");
	printf("-dv258_mode <0 | 1>          - DV258 Volume modeler enable  [0-off]\n");
	printf("-dv258_avm  <-1536 - +480>   - DV258 Analog volume level    [0]\n");
	printf("-dv258_dvm  <-1536 - +480>   - DV258 Digital volume level   [0]\n");
	printf("-dv258_lvle <0 | 1>          - DV258 Leveler enable         [1-on]\n");
	printf("-dv258_lvla <0 - 10>         - DV258 Leveler amount         [9]\n");
	printf("-dv258_mse  <0 | 1>          - DV258 Midside mode enable    [1-on]\n");
	printf("-dv258_pregain <-480 - +480> - DV258 Pregain                [0]\n");
	printf("-dv258_lime <0 | 1>          - DV258 Limiter enable         [1-on]\n");
	printf("-dv258_cfgfile <filename>    - DV258 config file with any of the above parameters\n");
	printf("-dv258_hmen <0|1>            - DV258 HalfMode enable [0-off]\n");
	/*printf("-dv258_reset                   - DV258 \n");*/
#endif

	printf("\n\nExample:");
	printf("\nFile playback    : app_audio -file stream.es -format 8 -spdif comp -drc_mode rf");
	printf("\nLive playback    : app_audio -pcr_pid 0x11 -aud_pid 0x14 -format 3 -spdif enc_ac3_51");
	printf("\nDual decode      : app_audio -file stream1.es -format 8 -secondary_info -file stream2.es -format 8");
	printf("\nTs File playback : app_audio -file stream.ts -aud_pid 0x34 -format 8 -targetsync_enable\n\n");
}


int read_config_file(char *filename, int (*parse_options) (char **argv))
{
	FILE *config_file;
	char line[300];
	char *param[2];
	int  retval = 0;
	int len = 0;
		
	config_file = fopen (filename, "rt");	
	if (config_file == NULL)
	{
		printf ("Could not open file for reading: %s\n", filename);
		return -1;
	}
	
	while (!feof (config_file))
	{
		fgets(line, 300, config_file);
		param[0] = strtok(line,"=");
		param[1] = strtok(NULL, "=");
		
		if (param[0] != NULL && param[1] != NULL)
		{
			/*check if the param line has been commented with # (ascii = 0x23)*/
			if(param[0][0] == 0x23) 
			{
				printf("\n Commented Line :%s: \n",line);
				continue;
			}
			len = strlen(param[1]);
			if(len!=0)
			{
				if(param[1][len-1] == '\n')
					param[1][len-1] = 0;
			}
			else
			{
				printf("\n Some wrong file parameter %s \n",line);
				usage();
				return NEXUS_ERR_UNDEFINED;
			}
			/*printf("reading %s and split into :%s: and :%s:\n ",line,param[0],param[1]);*/
			retval = parse_options(param);
			if(retval == -1)
			{
				printf("\n Some wrong file parameter %s, %s \n",param[0], param[1]);
				usage();
				return NEXUS_ERR_UNDEFINED;
			}	
		}		
	}
	return retval;
}


int probe_input_file ( char *filename,									/* Input */
					   NEXUS_AudioCodec audioCodec,						/* Input */
					   int *audio_pid, 									/* Output */
					   NEXUS_TransportType *transport_type, 			/* Output */
					   NEXUS_TransportTimestampType *timestamp_type)	/* Output */
{
	/* use media probe to set values */
	bmedia_probe_t probe = NULL;
	bmedia_probe_config probe_config;
	const bmedia_probe_stream *stream = NULL;
	const bmedia_probe_track *track = NULL;
	bfile_io_read_t fd = NULL;
	bool foundAudio = false;
	FILE *fin;
	int pcrPid;
	char stream_info[512];
	
	int ret_val = 0;
	*timestamp_type = NEXUS_TransportTimestampType_eNone;

	probe = bmedia_probe_create();

	fin = fopen64(filename,"rb");
	if (!fin) {
		printf("can't open media file '%s' for probing\n", filename);
		ret_val = -1;
		goto done;
	}

	fd = bfile_stdio_read_attach(fin);

	bmedia_probe_default_cfg(&probe_config);
	probe_config.file_name = filename;
	probe_config.type = bstream_mpeg_type_unknown;
	stream = bmedia_probe_parse(probe, fd, &probe_config);

	/* now stream is either NULL, or stream descriptor with linked list of audio/video tracks */
	bfile_stdio_read_detach(fd);

	fclose(fin);
	if(!stream) {
		printf("media probe can't parse stream '%s'\n", filename);
		ret_val = -1;
		goto done;
	}

	bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
	printf(
		"Media Probe:\n"
		"%s\n\n",
		stream_info);

	*transport_type = b_mpegtype2nexus(stream->type);
	
	if (stream->type == bstream_mpeg_type_ts)
	{
		if ((((bmpeg2ts_probe_stream*)stream)->pkt_len) == 192)
		{
			*timestamp_type = NEXUS_TransportTimestampType_eMod300;
		}
	}

	for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
		switch(track->type) {
			case bmedia_track_type_audio:
				if(track->info.audio.codec != baudio_format_unknown && !foundAudio) {
					*audio_pid = track->number;
					if (audioCodec == b_audiocodec2nexus(track->info.audio.codec))
					{
						foundAudio = true;
					}
					else
					{
						printf ("Found codec : %d\n", track->info.audio.codec);
					}
				}
				break;
			case bmedia_track_type_pcr:
				pcrPid = track->number;
				break;
			case bmedia_track_type_video:
			default:
				break;
		}
	}

done:
	if (probe) {
		if (stream) {
			bmedia_probe_stream_free(probe, stream);
		}
		bmedia_probe_destroy(probe);
	}

	if (foundAudio == false)
	{
		ret_val = -1;
	}
	
	return ret_val;
}

/* TBD: Set the order of close/stop calls right*/ 
void UnSetupDecodeChannel (NEXUS_AudioDecoderHandle audioDecoder, 
						 NEXUS_AudioDecoderStartSettings *audioProgramSetting, 
						 EChannelType chType)
{
	if (g_input_options.ChannelOpts[chType].file_playback == true)
	{	
		NEXUS_Playback_Stop(g_input_options.ChannelOpts[chType].playback);
	}

	
	NEXUS_AudioDecoder_Stop(audioDecoder);

    if (chType == eChannelTypePrimary)
    {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]));
#if NEXUS_NUM_I2S_OUTPUTS > 0
        if (g_input_options.multi_i2s == true) {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_I2sOutput_GetConnector(g_platformConfig.outputs.i2s[2]));
        }
#endif
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]));
    }

	{
		NEXUS_AudioDecoderConnectorType type;
		for(type = 0; type < NEXUS_AudioDecoderConnectorType_eMax; type++)
		{
			NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, type));
		}
	}






	NEXUS_AudioDecoder_Close(audioDecoder);	

	if (g_input_options.ChannelOpts[chType].file_playback == true)
	{
		NEXUS_FilePlay_Close(g_input_options.ChannelOpts[chType].file);
	    /*NEXUS_Playpump_Close(g_input_options.ChannelOpts[chType].playpump);*/
	    NEXUS_Playback_Destroy(g_input_options.ChannelOpts[chType].playback);
		BKNI_DestroyEvent(g_input_options.ChannelOpts[chType].event);
	}
	else
	{
		if (chType == eChannelTypePrimary)
		{
			NEXUS_Playback_ClosePidChannel(g_input_options.ChannelOpts[chType].playback, audioProgramSetting->pidChannel);
		}
	}
	

	if (chType == eChannelTypePrimary)
	{
		if (g_input_options.ChannelOpts[chType].file_playback != true)
		{
			if (g_input_options.ChannelOpts[chType].audio_pid != 0)
			{
				NEXUS_StcChannel_Close(audioProgramSetting->stcChannel);
			}
		}
	}


	
	/*if (chType == eChannelTypePrimary)
	{
		if (g_input_options.mixer_enable == true)
		{
			NEXUS_AudioMixer_RemoveAllInputs(g_mixer);
			NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(g_mixer));
			NEXUS_AudioMixer_Close(g_mixer);
		}
	}
	
	if (g_adv_options.adv_enable == true)
	{
#ifdef NEXUS_ABX_ENABLE
		NEXUS_AudysseyAbx_RemoveAllInputs(abxPostProc);
		NEXUS_AudysseyAbx_Close(abxPostProc);
#endif
#ifdef NEXUS_ADV_ENABLE
		NEXUS_AudysseyAdv_RemoveAllInputs(advPostProc);
		NEXUS_AudysseyAdv_Close(advPostProc);
#endif
	}
    
	if (g_adv_options.brcm_3d_surr_enable == true)
	{
#ifdef NEXUS_3DSURROUND_ENABLE
		NEXUS_3dSurround_RemoveAllInputs(abxPostProc);
		NEXUS_3dSurround_Close(abxPostProc);
#endif
	}
 
#ifdef EQUALIZER_SUPPORT	
	if (strlen (g_input_options.eq_coeff_file) != 0)
	{
		NEXUS_AudioEqualizer_Close (g_audioEqualizer);
	}
#endif
	*/
	return;
}

void SetupDecodeChannel (NEXUS_AudioDecoderHandle audioDecoder, 
						 NEXUS_AudioDecoderStartSettings *audioProgramSetting, 
						 EChannelType chType)
{
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings pidSettings;
	NEXUS_AudioDecoderSettings decSettings;
	NEXUS_AudioDecoderCodecSettings codecSettings;
    NEXUS_PlaybackStartSettings playbackStartSettings;
	
	NEXUS_PidChannelHandle audioPidChannel;
	NEXUS_StcChannelHandle stcChannel;
	
	NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
	
	NEXUS_AudioDecoder_GetDefaultStartSettings(audioProgramSetting);
    audioProgramSetting->codec = g_input_options.ChannelOpts[chType].audio_codec;
    /* For DDP cert testing */
    audioProgramSetting->targetSyncEnabled = g_input_options.targetsync_enable;
	
	/* Get default Decoder Settings for audio decoder */
	NEXUS_AudioDecoder_GetSettings (audioDecoder, &decSettings);
	
	/* Set the Generic Codec Parameters based on the input arguments */
	decSettings.dualMonoMode = g_input_options.ChannelOpts[chType].dual_mono_mode;
	decSettings.outputMode = g_input_options.ChannelOpts[chType].dec_out_mode;
	decSettings.outputLfeMode = g_input_options.ChannelOpts[chType].lfe_out;
	
	/* Set the Decoder Settings for audio decoder */
	NEXUS_AudioDecoder_SetSettings (audioDecoder, &decSettings);
	printf ("decSettings.outputMode = %d\n", decSettings.outputMode);
	NEXUS_AudioDecoder_GetCodecSettings (audioDecoder, g_input_options.ChannelOpts[chType].audio_codec, &codecSettings);
	/* Ac3Plus support not in yet - added for future purposes */
	codecSettings.codec = g_input_options.ChannelOpts[chType].audio_codec;
	if ((g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eAc3) 
		|| (g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eAc3Plus))
	{
		/* Set the Codec Specific Parameters based on the input arguments */
		codecSettings.codecSettings.ac3Plus.drcMode = g_ddp_options[chType].drc_opmode;
		codecSettings.codecSettings.ac3Plus.stereoDownmixMode = g_ddp_options[chType].dm_mode;
		codecSettings.codecSettings.ac3Plus.cut = g_ddp_options[chType].hdr_cut;
		codecSettings.codecSettings.ac3Plus.boost = g_ddp_options[chType].ldr_boost;
		codecSettings.codecSettings.ac3Plus.dialogNormalization = true;
		codecSettings.codecSettings.ac3Plus.dialogNormalizationValue = 31; /* Default Value - Never used if dialogNormalization = true*/
		codecSettings.codecSettings.ac3Plus.substreamId = g_ddp_options[chType].substream_id;
#ifdef NEXUS_MS11_SUPPORT
		/* Below three are applicable only for MS11 */
		codecSettings.codecSettings.ac3Plus.drcModeDownmix = g_ddp_options[chType].dwmix_drc_opmode;
		codecSettings.codecSettings.ac3Plus.cutDownmix = g_ddp_options[chType].dwmix_hdr_cut;
		codecSettings.codecSettings.ac3Plus.boostDownmix = g_ddp_options[chType].dwmix_ldr_boost;
		printf("\n\n\ncodecSettings.codecSettings.ac3Plus.cutDownmix : 0x%x\ncodecSettings.codecSettings.ac3Plus.boostDownmix:0x%x\n\n\n",
					codecSettings.codecSettings.ac3Plus.cutDownmix,
					codecSettings.codecSettings.ac3Plus.boostDownmix);
		

#endif
	}
	else if ((g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eAacAdts) 
			 || (g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eAacLoas))
	{
		/* Set the Codec Specific Parameters based on the input arguments */
		codecSettings.codecSettings.aac.drcMode = g_dolby_pulse_options[chType].drc_opmode;
		codecSettings.codecSettings.aac.downmixMode = g_dolby_pulse_options[chType].dm_mode;
		codecSettings.codecSettings.aac.cut = g_dolby_pulse_options[chType].hdr_cut;
		codecSettings.codecSettings.aac.boost = g_dolby_pulse_options[chType].ldr_boost;
		codecSettings.codecSettings.aac.drcDefaultLevel = g_dolby_pulse_options[chType].dialnorm_value;
	}
	else if ((g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eAacPlusAdts)
			 || (g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eAacPlusLoas))
	{
		/* Set the Codec Specific Parameters based on the input arguments */
		codecSettings.codecSettings.aacPlus.drcMode = g_dolby_pulse_options[chType].drc_opmode;
		codecSettings.codecSettings.aacPlus.downmixMode = g_dolby_pulse_options[chType].dm_mode;
		codecSettings.codecSettings.aacPlus.cut = g_dolby_pulse_options[chType].hdr_cut;
		codecSettings.codecSettings.aacPlus.boost = g_dolby_pulse_options[chType].ldr_boost;
		codecSettings.codecSettings.aacPlus.drcDefaultLevel = g_dolby_pulse_options[chType].dialnorm_value;
	}
	else if ((g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eDts) 
			 || (g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eDtsHd) 
			 || (g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eDtsCd))
	{
		/* Set the Codec Specific Parameters based on the input arguments */
		codecSettings.codecSettings.dts.stereoDownmixMode = g_dts_options.stereoDownmixMode;
		codecSettings.codecSettings.dts.cut = g_dts_options.cut;
		codecSettings.codecSettings.dts.boost = g_dts_options.boost;
		codecSettings.codecSettings.dts.mixLfeToPrimary = g_dts_options.mixLfeToPrimary;
		codecSettings.codecSettings.dts.enableDrc = g_dts_options.enableDrc;
		
		
		if(g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eDtsCd)
		{
			codecSettings.codecSettings.dts.littleEndian=true;
			g_dts_options.littleEndian=true;
			printf("CD SUPPORT and Its little endian%x\n",g_dts_options.littleEndian);
		
		}
	}
	NEXUS_AudioDecoder_SetCodecSettings (audioDecoder, &codecSettings);

	if (g_input_options.ChannelOpts[chType].file_playback == true)
	{
	    g_input_options.ChannelOpts[chType].playpump = NEXUS_Playpump_Open((int)chType, NULL);
	    assert(g_input_options.ChannelOpts[chType].playpump);
	    g_input_options.ChannelOpts[chType].playback = NEXUS_Playback_Create();
	    assert(g_input_options.ChannelOpts[chType].playback);
		
		BKNI_CreateEvent(&g_input_options.ChannelOpts[chType].event);
		
		if (strlen(g_input_options.ChannelOpts[chType].filename) != 0)
		{
			g_input_options.ChannelOpts[chType].file = 
						NEXUS_FilePlay_OpenPosix(g_input_options.ChannelOpts[chType].filename, NULL);
		}
	    if (!g_input_options.ChannelOpts[chType].file)
		{
			if ((g_input_options.ChannelOpts[chType].audio_pid == 0)
				|| (g_input_options.ChannelOpts[chType].pcr_pid == 0))
			{
				printf ("Audio and PCR Pids need to be specified for streamer playback of %s channel\n", ChannelString[chType]);
		        exit (-1);
		    }
			else
			{
				printf ("No input file specified for %s decoder. Streamer input will be decoded\n", ChannelString[chType]);
			}
	    }
		
	    NEXUS_Playback_GetSettings(g_input_options.ChannelOpts[chType].playback, &playbackSettings);
	    playbackSettings.playpump = g_input_options.ChannelOpts[chType].playpump;
		playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eEs;
		if (chType == eChannelTypePrimary)
		{
	    	playbackSettings.endOfStreamCallback.callback = eof;
			playbackSettings.endOfStreamAction = g_input_options.endOfStreamAction;
		}
		else
		{
			playbackSettings.endOfStreamCallback.callback = sec_eof;
			if (g_input_options.loop_count == 1)
			{
				playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
			}
			else
			{
				playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
			}
		}
		/*playbackSettings.timeshifting = true;*/
		
	    playbackSettings.endOfStreamCallback.context = g_input_options.ChannelOpts[chType].event;
		if ((g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eMp3) 
			|| (g_input_options.ChannelOpts[chType].audio_codec == NEXUS_AudioCodec_eMpeg))
		{
			playbackSettings.enableStreamProcessing = true; /* this should be set to true if application needs to seek in the MP3 file, it'd cause decoder to receive MPEG-2 PES data */
		}
		else
		{
			playbackSettings.enableStreamProcessing = false;
		}
		
		if (g_input_options.ChannelOpts[chType].audio_pid != 0)
		{
			parserBand = NEXUS_ParserBand_e0;
			playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
			playbackSettings.playpumpSettings.timestamp.pacing = false;
			playbackSettings.playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eNone;

		    /* Open the audio pid channel */
#if 1
		    audioPidChannel = NEXUS_PidChannel_Open(parserBand, g_input_options.ChannelOpts[chType].audio_pid, NULL);
#else
		    NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
		    pidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
		    pidSettings.pidTypeSettings.audio.primary = audioDecoder;
		    audioPidChannel = NEXUS_Playback_OpenPidChannel(g_input_options.ChannelOpts[chType].playback,
															g_input_options.ChannelOpts[chType].audio_pid, &pidSettings);
#endif

			if (chType == eChannelTypePrimary)
			{
				/* Open the StcChannel to do lipsync with the PCR */
				NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
				stcSettings.timebase = NEXUS_Timebase_e0;
				stcSettings.mode = NEXUS_StcChannelMode_eAuto;

				stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
				g_primaryStcChannel = stcChannel;
			}
			else
			{
				stcChannel = g_primaryStcChannel;
				audioProgramSetting->secondaryDecoder = true;
			}

		    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
			the audio outputs. */
		    audioProgramSetting->pidChannel = audioPidChannel;
		    audioProgramSetting->stcChannel = stcChannel;
		}
		else
		{		
			g_input_options.ChannelOpts[chType].audio_pid = 1;
			if ((NEXUS_AudioCodec_eWmaStd == g_input_options.ChannelOpts[chType].audio_codec)
				|| (NEXUS_AudioCodec_eWmaPro == g_input_options.ChannelOpts[chType].audio_codec)
				|| (NEXUS_AudioCodec_ePcmWav == g_input_options.ChannelOpts[chType].audio_codec)
				|| (NEXUS_AudioCodec_eLpcmDvd == g_input_options.ChannelOpts[chType].audio_codec))
			{
				NEXUS_TransportType				transport_type;
				NEXUS_TransportTimestampType	timestamp_type;
				int err;
				
				/* Parse the stream here and get the Audio PID / TrasportType correctly here */
				err = probe_input_file(g_input_options.ChannelOpts[chType].filename, 
										g_input_options.ChannelOpts[chType].audio_codec, 
										&g_input_options.ChannelOpts[chType].audio_pid, &transport_type, &timestamp_type);
				if (!(err))
				{
				    playbackSettings.playpumpSettings.transportType = transport_type;
				    playbackSettings.playpumpSettings.timestamp.pacing = false;
				    playbackSettings.playpumpSettings.timestamp.type = timestamp_type;
				}
				else
				{
					printf ("Could not parse input stream file. Will run the stream as an elementary stream");
				}
			}
		}
		
	    NEXUS_Playback_SetSettings(g_input_options.ChannelOpts[chType].playback, &playbackSettings);
		
		NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
	    pidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
	    pidSettings.pidSettings.pidTypeSettings.audio.codec = audioProgramSetting->codec;
	    pidSettings.pidTypeSettings.audio.primary = audioDecoder;
	
	    audioProgramSetting->pidChannel = NEXUS_Playback_OpenPidChannel(g_input_options.ChannelOpts[chType].playback, 
																g_input_options.ChannelOpts[chType].audio_pid, 
																&pidSettings);
	
	    NEXUS_Playback_GetDefaultStartSettings(&playbackStartSettings);
	#if 0
	    playbackStartSettings.mode = NEXUS_PlaybackMode_eFixedBitrate;
	    playbackStartSettings.bitrate = 128000;
	#else
		if (playbackSettings.playpumpSettings.transportType == NEXUS_TransportType_eVob)
		{
			playbackStartSettings.mode = NEXUS_PlaybackMode_eIndexed;
		}
		else
		{
			playbackStartSettings.mode = NEXUS_PlaybackMode_eAutoBitrate;
		}
	#endif
		NEXUS_Playback_Start(g_input_options.ChannelOpts[chType].playback, 
							 g_input_options.ChannelOpts[chType].file, 
							 &playbackStartSettings);
		
	}
	else /* Play from Streamer */
	{
		if ((g_input_options.ChannelOpts[chType].audio_pid == 0)
			|| (g_input_options.ChannelOpts[chType].pcr_pid == 0))
		{
			printf ("Audio and PCR Pids need to be specified for streamer playback of %s channel\n", ChannelString[chType]);
			exit (-1);
		}
	    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
	    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

	    /* Map a parser band to the streamer input band. */
	    parserBand = NEXUS_ParserBand_e0;
	    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
	    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
	    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
	    parserBandSettings.transportType = NEXUS_TransportType_eTs;
	    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

	    /* Open the audio and pcr pid channels */
	    audioPidChannel = NEXUS_PidChannel_Open(parserBand, g_input_options.ChannelOpts[chType].audio_pid, NULL);

		if (chType == eChannelTypePrimary)
		{
		    /* Open the StcChannel to do lipsync with the PCR */
		    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
		    stcSettings.timebase = NEXUS_Timebase_e0;
		    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
		    stcSettings.modeSettings.pcr.pidChannel = NEXUS_PidChannel_Open(parserBand, 
																			g_input_options.ChannelOpts[chType].pcr_pid, 
																			NULL);
		    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
			g_primaryStcChannel = stcChannel;
		}
		else
		{
			stcChannel = g_primaryStcChannel;
			audioProgramSetting->secondaryDecoder = true;
		}

	    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
		the audio outputs. */
	    audioProgramSetting->pidChannel = audioPidChannel;
	    audioProgramSetting->stcChannel = stcChannel;

		
	}
	
	
	NEXUS_AudioDecoder_Start(audioDecoder, audioProgramSetting);
	
	return;
}

/* This function initializes all the global variables */
void InputParamsInit(void)
{
	EChannelType chType;
	
	CurrentChannelType = eChannelTypePrimary;
	g_input_options.eSpdif_out = eSpdifOutPcm;
	/* HDMI output only on demand */
	g_input_options.eHdmi_out = eHdmiOutLast;
	g_input_options.tru_vol_enable = false;
	g_input_options.brcm_3d_surr_enable = false;
	g_input_options.custom_voice_enable = false;
	g_input_options.dolby_vol_enable = false;
	g_input_options.ddre_enable = false;
	g_input_options.encInput = NEXUS_AudioDecoderConnectorType_eMultichannel;
	g_input_options.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
	g_input_options.loop_count = 1;
	g_ddp_options[eChannelTypePrimary].substream_id = 0;
	g_ddp_options[eChannelTypeAssociated].substream_id = 0;
	g_input_options.ChannelOpts[eChannelTypePrimary].audio_pid = 0;
	g_input_options.ChannelOpts[eChannelTypeAssociated].audio_pid = 0;
	g_input_options.ChannelOpts[eChannelTypePrimary].pcr_pid = 0;
	g_input_options.ChannelOpts[eChannelTypeAssociated].pcr_pid = 0;
	g_input_options.ChannelOpts[eChannelTypePrimary].filename[0] = '\0';
	g_input_options.ChannelOpts[eChannelTypeAssociated].filename[0] = '\0';
	g_input_options.ChannelOpts[eChannelTypePrimary].file = NULL;
	g_input_options.ChannelOpts[eChannelTypeAssociated].file = NULL;
	g_input_options.ChannelOpts[eChannelTypeAssociated].file_playback = false;
	g_input_options.ChannelOpts[eChannelTypeAssociated].file_playback = false;
	g_input_options.targetsync_enable = false;
	g_input_options.ChannelOpts[eChannelTypePrimary].num_channels = 2;
#if NEXUS_NUM_I2S_OUTPUTS > 0
    g_input_options.multi_i2s = false;
#endif
	g_input_options.stereo_disable = false;
	for (chType = eChannelTypePrimary; chType < eChannelTypeLast; chType++)
	{
		g_ddp_options[chType].dm_mode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic;
		g_ddp_options[chType].drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
		g_ddp_options[chType].substream_id = 0;
		g_ddp_options[chType].hdr_cut = 100;
		g_ddp_options[chType].ldr_boost = 100;
		g_dolby_pulse_options[chType].dm_mode = NEXUS_AudioDecoderAacDownmixMode_eMatrix;
#ifdef NEXUS_MS11_SUPPORT		
		g_ddp_options[chType].dwmix_drc_opmode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
		g_ddp_options[chType].dwmix_hdr_cut = 100;
		g_ddp_options[chType].dwmix_ldr_boost = 100;
#endif
		g_dolby_pulse_options[chType].drc_opmode = NEXUS_AudioDecoderDolbyPulseDrcMode_eLine;
		g_dolby_pulse_options[chType].hdr_cut = 100;
		g_dolby_pulse_options[chType].ldr_boost = 100;
		g_dolby_pulse_options[chType].dialnorm_value=108; 
	}
	g_dts_options.cut = 100;
	g_dts_options.boost = 100;
	g_dts_options.stereoDownmixMode = NEXUS_AudioDecoderDtsDownmixMode_eAuto;
	g_dts_options.mixLfeToPrimary = false;
	g_dts_options.enableDrc = false;
	
#ifdef NEXUS_ADV_ENABLE
	g_adv_options.adv_enable = false;
	g_adv_options.adv.volumeScalingEnabled = true;
	g_adv_options.adv.volume = 0;
	g_adv_options.adv.reInitialize = false;
	g_adv_options.adv.dynamicSurroundGainEnabled = true;
	g_adv_options.adv.hiLoCompress = false;
	g_adv_options.adv.channelMask = 0xFF;
	g_adv_options.adv.headroomOffset = 0;
	g_adv_options.adv.dynamicEqMode = NEXUS_AudysseyAdvDynamicEqMode_eOff;
#endif
#ifdef NEXUS_ABX_ENABLE
	g_adv_options.abx.enabled=1;
	g_adv_options.abx.harmonicGain=6;
	g_adv_options.abx.filterSet=1;
	g_adv_options.abx.dryGain=0;
#endif

#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
	printf("Getting SRS SS default settings\n");
	NEXUS_StudioSound_GetDefaultSettings(&sStudioSoundOptions.g_StudioSound_Options);

	sStudioSoundOptions.g_StudioSound_Options.enabled = false;
	sStudioSoundOptions.g_StudioSound_Options.circleSurround.enabled = false;
	sStudioSoundOptions.g_StudioSound_Options.truDialog.enabled = false;
	sStudioSoundOptions.g_StudioSound_Options.truSurroundHd.enabled = false;
	sStudioSoundOptions.g_StudioSound_Options.truVolume.enabled = false;
#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM
	sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilterParams.bEnable = false;
	sStudioSoundOptions.HPF_Front_Mode0_Coeff_FileName = NULL;
#else
	sStudioSoundOptions.g_StudioSound_Options.truVolume.highPassFilter.enabled = false;
#endif
	sStudioSoundOptions.g_StudioSound_Options.graphicEq.leftEnabled = false;
	sStudioSoundOptions.g_StudioSound_Options.graphicEq.rightEnabled = false;

	sStudioSoundOptions.g_StudioSound_Options.truEq.leftEnabled = false;
	sStudioSoundOptions.g_StudioSound_Options.truEq.rightEnabled = false;
#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM	
	sStudioSoundOptions.TruEQ_Mode0_Coeff_FileName = NULL;

	sStudioSoundOptions.g_StudioSound_Options.highPassFilter.bEnable = false;
	sStudioSoundOptions.HPF_Back_Mode0_Coeff_FileName = NULL;
#else
	sStudioSoundOptions.g_StudioSound_Options.highPassFilter.enabled = false;
#endif
	sStudioSoundOptions.g_StudioSound_Options.hardLimiter.enabled = false;
	sStudioSoundOptions.g_StudioSound_Options.circleSurround.enabled = false;
#else
#ifdef NEXUS_SRSTRUVOL_ENABLE
	printf("Getting SRS TVOL default settings\n");
	NEXUS_TruVolume_GetDefaultSettings(&g_srs_tvol_settings);
	g_srs_tvol_settings.enabled = false;
#endif
#ifdef NEXUS_SRSHD_ENABLE
	printf("Getting SRS TSHD default settings\n");
	NEXUS_TruSurroundHd_GetDefaultSettings(&g_srs_tshd_settings);
	g_srs_tshd_settings.enabled = false;
#endif
#endif

#ifdef NEXUS_DV258_ENABLE
	printf("Getting DV258 default settings\n");
	NEXUS_DolbyVolume258_GetDefaultSettings(&DV258Settings);
	DV258Settings.enabled = false;
#endif

#ifdef NEXUS_DDRE_ENABLE	
	printf("Getting DDRE default settings\n");
	NEXUS_DolbyDigitalReencode_GetDefaultSettings(&DDRE_Settings);
#endif
	g_input_options.eq_coeff_file[0] = '\0';
}

int main(int argc, char **argv)
{
    NEXUS_AudioDecoderHandle primaryDecoder;
    NEXUS_AudioDecoderHandle associatedDecoder;
	
#ifdef CUSTOM_VOICE_ENABLE
	NEXUS_CustomVoiceHandle customVoice;
#endif
#ifdef NEXUS_3DSURROUND_ENABLE
    NEXUS_3dSurroundHandle brcm3DSurroundhandle;
    NEXUS_3dSurroundSettings brcm3DSurroundSettings;
#endif
#ifdef NEXUS_DOLBY_VOLUME_ENABLE
	NEXUS_DolbyVolume258Handle dolbyVolume258;
#endif
#ifdef NEXUS_DDRE_ENABLE
	NEXUS_DolbyDigitalReencodeHandle ddre;
#endif

#ifdef NEXUS_SRSTRUVOL_ENABLE
	NEXUS_TruVolumeHandle truVolume;
#endif

    NEXUS_DtsEncodeHandle dtsEncode;
    NEXUS_Ac3EncodeHandle ac3Encode;
	/* NEXUS_Error rc; */
	
	NEXUS_AudioMixerSettings mixerSettings;
	NEXUS_AudioDecoderAacSettings *paacSettings;
		
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_AudioDecoderStartSettings associatedAudioProgram;
	NEXUS_AudioDecoderOpenSettings audioOpenSettings;
	NEXUS_AudioDecoderSettings decSettings;
	NEXUS_AudioDecoderCodecSettings codecSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_AudioOutputGroupCreateSettings outputGroupCreateSettings;
    NEXUS_AudioOutputGroupHandle outputGroup;
	
#ifdef NEXUS_ADV_ENABLE
	NEXUS_AudysseyAdvHandle	advPostProc;
	NEXUS_AudysseyAdvSettings advSettings;
#endif
#ifdef NEXUS_ABX_ENABLE
	NEXUS_AudysseyAbxHandle	abxPostProc;
	NEXUS_AudysseyAbxSettings abxSettings;
#endif

#ifdef NEXUS_AVL_ENABLE
	NEXUS_AutoVolumeLevelHandle avlPostProc;
	NEXUS_AutoVolumeLevelSettings avlSettings;
#endif

#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
	NEXUS_StudioSoundHandle srsPostProc;
	NEXUS_TruVolumeHandle tvolPostProc;
	NEXUS_TruSurroundHdHandle tshdPostProc;
#else
#ifdef NEXUS_SRSTRUVOL_ENABLE
	NEXUS_TruVolumeHandle tvolPostProc;
#endif
#ifdef NEXUS_SRSHD_ENABLE
	NEXUS_TruSurroundHdHandle tshdPostProc;
#endif
#endif
	
#ifdef EQUALIZER_SUPPORT
	NEXUS_AudioEqualizerSettings eqSettings;
#endif
#if NEXUS_NUM_HDMI_OUTPUTS > 0
	NEXUS_AudioInput			hdmiInput;
	NEXUS_HdmiOutputStatus 		hdmiStatus;
	NEXUS_AudioDecoderHandle 	hdmiDecoder;
	NEXUS_AudioDecoderConnectorType hdmiDecoderConnectorType = NEXUS_AudioDecoderConnectorType_eStereo;
#endif
	
	int		i;
	
	char			input_string[200], one_char, cmd_name[100], arg_val[100];
#ifdef NEXUS_DV258_ENABLE	
	char			*dv258_cmd_str[2];
#endif
	int				retval;
	int				num_chars = 0;
	short int		flags;
	bool			eqEnabled = false;
	
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
	
	flags = fcntl(STDIN_FILENO, F_GETFL, 0);

#ifdef BDSP_FW_RBUF_CAPTURE
	brbuf_init_ringbuf_capture();
#endif


	NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

	printf("Entering Nexus App_audio\n");
	InputParamsInit();
	
	printf("Init params complete \n");


	if ( 2 >= argc )
	{
		usage();
		return NEXUS_ERR_UNDEFINED;
	}
	else
	{
		for (i = 1; i < argc; i++) 
		{
			retval = parse_input_options(&argv[i]);
			if (retval != -1)
			{
				i += retval;
				continue;
			}
			if ((g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eAacAdts)
				|| (g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eAacLoas)
				|| (g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eAacPlusAdts)
				|| (g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eAacPlusLoas))
			{
				retval = parse_ms10_dolby_plus_options(&argv[i]);
				if (retval != -1)
				{
					i += retval;
					continue;
				}
			}
			if ((g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eAc3)
				|| (g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eAc3Plus))
			{
				retval = parse_ms10_ddp_options(&argv[i]);
				if (retval != -1)
				{
					i += retval;
					continue;
				}
			}
			if ((g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eDts)
				|| (g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eDtsHd)
				|| (g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eDtsCd))
			{
				retval = parse_dts_options(&argv[i]);
				if (retval != -1)
				{
					i += retval;
					continue;
				}
			}
#if (defined (NEXUS_ADV_ENABLE) || defined (NEXUS_ABX_ENABLE))
			retval = parse_adv_options(&argv[i]);
			if (retval != -1)
			{
				i += retval;
				continue;
			}
#endif

#ifdef NEXUS_AVL_ENABLE
			retval = parse_avl_options(&argv[i]);
			if (retval != -1)
			{
				i += retval;
				continue;
			}
#endif

#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
			retval = parse_SRS_SS_options(&argv[i]);

			if(retval == 3)
			{
				read_config_file(argv[i+1], &parse_SRS_SS_options);
			}
			if (retval != -1)
			{
				i += retval;
				continue;
			}
#else
#ifdef NEXUS_SRSTRUVOL_ENABLE
			retval = parse_SRS_TVOL_options(&argv[i]);

			if(retval == 3)
			{
				read_config_file(argv[i+1], &parse_SRS_TVOL_options);
			}
			if (retval != -1)
			{
				i += retval;
				continue;
			}
#endif
#ifdef NEXUS_SRSHD_ENABLE
			retval = parse_SRS_TSHD_options(&argv[i]);

			if(retval == 3)
			{
				read_config_file(argv[i+1],&parse_SRS_TSHD_options);
			}
			if (retval != -1)
			{
				i += retval;
				continue;
			}
#endif
#endif

#ifdef NEXUS_DV258_ENABLE
			retval = parse_DV258_options(&argv[i]);
			if(retval == 3)
			{
				read_config_file(argv[i+1], &parse_DV258_options);
				printf("Parsing DV258 cfgfile DONE\n");
				retval=1;
			}
			if (retval != -1)
			{
				i += retval;
				continue;
			}
#endif

#ifdef NEXUS_DDRE_ENABLE
			retval = parse_DDRE_options(&argv[i]);
			if(retval == 3)
			{
				read_config_file(argv[i+1], &parse_DDRE_options);
				printf("Parsing DDRE cfgfile DONE\n");
				retval=1;
			}
			if (retval != -1)
			{
				i += retval;
				continue;
			}
#endif
			usage();
			return NEXUS_ERR_UNDEFINED;
		}
	}

#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
#ifndef  SRS_SSHD_NO_FILTER_COEFF_PROGRAM	 
	 Assign_SRS_SS_Peq_Coeffs();
	 Assign_SRS_SS_HPF_Back_Filter_Coeffs();
	 Assign_SRS_SS_HPF_Front_Filter_Coeffs();
#endif	 
	 validate_dependency_srs();
#endif

	if (strlen (g_input_options.eq_coeff_file) != 0)
	{
		eqEnabled = true;
	}

	

    NEXUS_Platform_GetConfiguration(&g_platformConfig);

	/* Start Video */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
#if NEXUS_DTV_PLATFORM
	displaySettings.displayType = NEXUS_DisplayType_eLvds;
	displaySettings.format = NEXUS_VideoFormat_eCustom0;
#else
	displaySettings.displayType = NEXUS_DisplayType_eAuto;
	displaySettings.format = NEXUS_VideoFormat_eNtsc;
#endif

	display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_DTV_PLATFORM
    NEXUS_Display_AddOutput(display, NEXUS_PanelOutput_GetConnector(g_platformConfig.outputs.panel[0]));
#else
	NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(g_platformConfig.outputs.composite[0]));
    if(g_platformConfig.outputs.component[0])
    {
        NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(g_platformConfig.outputs.component[0]));
    }
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(g_platformConfig.outputs.hdmi[0]));
#endif

	/********************************************************************************/

    /*file = NEXUS_FilePlay_OpenPosix(fname, NULL);*/
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    audioOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
    primaryDecoder = NEXUS_AudioDecoder_Open(0, &audioOpenSettings);

#if NEXUS_NUM_HDMI_OUTPUTS > 0
	/* HDMI output only on demand*/
	if (g_input_options.eHdmi_out != eHdmiOutLast)
	{
		NEXUS_HdmiOutput_GetStatus(g_platformConfig.outputs.hdmi[0], &hdmiStatus);
		hdmiDecoder = primaryDecoder;

	switch (g_input_options.eHdmi_out)
	{
		case eHdmiOutPcmStereo:
			hdmiDecoderConnectorType = NEXUS_AudioDecoderConnectorType_eStereo;
			break;
		case eHdmiOutPcmMultiChannel:
			/*if (hdmiStatus.maxAudioPcmChannels > 2)
			{*/
				hdmiDecoderConnectorType = NEXUS_AudioDecoderConnectorType_eMultichannel;
			/*}
			else
			hdmiDecoderConnectorType = NEXUS_AudioDecoderConnectorType_eStereo;
			*/
			break;
		case eHdmiOutEncAc3:
		case eHdmiOutEncDts:
			if ( hdmiStatus.audioCodecSupported[g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec] )
			{
				hdmiDecoderConnectorType = NEXUS_AudioDecoderConnectorType_eCompressed;
			}
			break;
		case eHdmiOutCompressed:
			if ( hdmiStatus.audioCodecSupported[g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec] )
			{
				/*hdmiDecoder = (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eWmaPro)?audioDecoder:compressedDecoder;*/
				hdmiDecoderConnectorType = NEXUS_AudioDecoderConnectorType_eCompressed;
			}
			if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3Plus) &&
				(hdmiStatus.audioCodecSupported[NEXUS_AudioCodec_eAc3]))
			{
				hdmiDecoderConnectorType = NEXUS_AudioDecoderConnectorType_eCompressed;
			}
		case eHdmiOutCompressedHbr:
			/* Unsupported */
			printf ("Compressed HBR output on HDMI is not supported!!!\n");
			hdmiDecoderConnectorType = NEXUS_AudioDecoderConnectorType_eCompressed;
			break;
		default:
			break;
	}
	
		hdmiInput = NEXUS_AudioDecoder_GetConnector(hdmiDecoder, hdmiDecoderConnectorType);
	}
#endif

#ifdef EQUALIZER_SUPPORT
	if (eqEnabled == true)
	{
		NEXUS_AudioEqualizer_GetDefaultSettings(&eqSettings);
		eqSettings.mode = NEXUS_AudioEqualizerMode_eCustom;
		eqSettings.secondaryMode = NEXUS_AudioEqualizerMode_eDisabled;
		eqSettings.subsonicFilterEnabled = false;
		eqSettings.subwooferFilterEnabled = false;
		
		retval = read_aeq_coeffs(g_input_options.eq_coeff_file, &eqSettings);
		if (retval != 0)
		{
			printf ("Error: Failed to get equalizer co-efficients\n");
			exit (-1);
		}
		
		g_audioEqualizer = NEXUS_AudioEqualizer_Open(0, &eqSettings);
		NEXUS_AudioOutput_SetEqualizer(
				NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
				g_audioEqualizer);
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			NEXUS_AudioOutput_SetEqualizer(
					NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]), 
					g_audioEqualizer);
		}
	}
#endif
	
	
	if (g_input_options.mixer_enable == true)
	{
	    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
	    audioOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
	    associatedDecoder = NEXUS_AudioDecoder_Open(1, &audioOpenSettings);
		
		g_mixer = NEXUS_AudioMixer_Open(NULL);
		
	    NEXUS_AudioMixer_AddInput(g_mixer, NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
	    NEXUS_AudioMixer_AddInput(g_mixer, NEXUS_AudioDecoder_GetConnector(associatedDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

	    /* Set the Mixer to use DSP mixing */
	    NEXUS_AudioMixer_GetSettings(g_mixer, &mixerSettings);
	    mixerSettings.mixUsingDsp = true;
	    mixerSettings.master = NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
	    NEXUS_AudioMixer_SetSettings(g_mixer, &mixerSettings);
	}

#if NEXUS_NUM_I2S_OUTPUTS > 0
    if (g_input_options.multi_i2s == true)
    {
        NEXUS_AudioOutput_AddInput(
                                   NEXUS_I2sOutput_GetConnector(g_platformConfig.outputs.i2s[2]),
                                   NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
    }
#endif

	/* DAC OUTPUT CONFIGURATION and SPDIF for PCM*/
#ifdef NEXUS_SRSTRUVOL_ENABLE
	if (g_input_options.tru_vol_enable == true)
	{
		printf ("Tru Volume Postprocessing added\n\n\n\n");
		truVolume = NEXUS_TruVolume_Open(NULL);
		NEXUS_TruVolume_AddInput(truVolume, NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
		
		NEXUS_AudioOutput_AddInput(
			NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
			NEXUS_TruVolume_GetConnector(truVolume));
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_TruVolume_GetConnector(truVolume));
		}
	}
#else
    if (0) /* Dirty hack - get rid of it/*/
    {
        /* Do Nothing */
    }
#endif
#ifdef NEXUS_3DSURROUND_ENABLE
	else if (g_input_options.brcm_3d_surr_enable == true)
	{
		printf ("3D Surround Postprocessing added\n");
		brcm3DSurroundhandle = NEXUS_3dSurround_Open(NULL);
        
        NEXUS_3dSurround_GetDefaultSettings(&brcm3DSurroundSettings);
        NEXUS_3dSurround_SetSettings(brcm3DSurroundhandle, &brcm3DSurroundSettings);
        
		NEXUS_3dSurround_AddInput(brcm3DSurroundhandle, 
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
		
		NEXUS_AudioOutput_AddInput(
			NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
			NEXUS_3dSurround_GetConnector(brcm3DSurroundhandle));
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_3dSurround_GetConnector(brcm3DSurroundhandle));
		}
	}
#endif
#ifdef CUSTOM_VOICE_ENABLE
	else if (g_input_options.custom_voice_enable == true)
	{
		printf ("Custom voice Postprocessing added\n");
		customVoice = NEXUS_CustomVoice_Open(NULL);
		NEXUS_CustomVoice_DEQ_Mode(customVoice, true);
		NEXUS_CustomVoice_AddInput(customVoice, 
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
		
		NEXUS_AudioOutput_AddInput(
			NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
			NEXUS_CustomVoice_GetConnector(customVoice));
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_CustomVoice_GetConnector(customVoice));
		}
	}
#endif
#if defined NEXUS_DOLBY_VOLUME_ENABLE
	else if (g_input_options.dolby_vol_enable == true)
	{
		printf ("Dolby Voulme Postprocessing added\n");
		dolbyVolume258 = NEXUS_DolbyVolume258_Open(NULL);
		NEXUS_DolbyVolume258_AddInput(dolbyVolume258, 
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
		
#ifdef NEXUS_DDRE_ENABLE
		if (g_input_options.ddre_enable == true)
		{
			ddre = NEXUS_DolbyDigitalReencode_Open(NULL);
			NEXUS_DolbyDigitalReencode_SetSettings(ddre, &DDRE_Settings);

			printf ("DDRE Postprocessing added followed by DV258\n");
			NEXUS_DolbyDigitalReencode_AddInput(ddre, 
							NEXUS_DolbyVolume258_GetConnector(dolbyVolume258));
			
			NEXUS_AudioOutput_AddInput(
				NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
				NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eStereo));
			if (g_input_options.eSpdif_out == eSpdifOutPcm)
			{
				NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
						NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eStereo));
			}
		}
		else
#endif
		{
			NEXUS_AudioOutput_AddInput(
				NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
				NEXUS_DolbyVolume258_GetConnector(dolbyVolume258));
			if (g_input_options.eSpdif_out == eSpdifOutPcm)
			{
				NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
						NEXUS_DolbyVolume258_GetConnector(dolbyVolume258));
			}
		}
	}
#endif
#if (defined (NEXUS_ADV_ENABLE) || defined (NEXUS_ABX_ENABLE))
	else if (g_adv_options.adv_enable == true)
	{
		printf ("Audyssey Postprocessing added\n");
#ifdef NEXUS_ADV_ENABLE
		NEXUS_AudysseyAdv_GetDefaultSettings(&advSettings);
		advSettings.enabled = g_adv_options.adv.enabled;
		advSettings.gainCompression = g_adv_options.adv.gainCompression;
		advSettings.volumeScalingEnabled = g_adv_options.adv.volumeScalingEnabled;
		advSettings.volume = g_adv_options.adv.volume;
		advSettings.reInitialize = g_adv_options.adv.reInitialize;
		advSettings.dynamicSurroundGainEnabled = g_adv_options.adv.dynamicSurroundGainEnabled;
		advSettings.hiLoCompress = g_adv_options.adv.hiLoCompress;
		advSettings.channelMask = g_adv_options.adv.channelMask;
		advSettings.headroomOffset = g_adv_options.adv.headroomOffset;
		advSettings.dynamicEqMode = g_adv_options.adv.dynamicEqMode;
		
		advPostProc = NEXUS_AudysseyAdv_Open(&advSettings);
#endif
		
#ifdef NEXUS_ABX_ENABLE
		abxSettings.enabled = g_adv_options.abx.enabled;
		abxSettings.harmonicGain = g_adv_options.abx.harmonicGain;
		abxSettings.filterSet = g_adv_options.abx.filterSet;
		abxSettings.dryGain = g_adv_options.abx.dryGain;
	
		abxPostProc = NEXUS_AudysseyAbx_Open(&abxSettings);
#endif
		
#ifdef NEXUS_ADV_ENABLE
		/* DECODER -> ADV */
	    NEXUS_AudysseyAdv_AddInput(advPostProc, 
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
	#ifdef NEXUS_ABX_ENABLE
		/* ADV -> ABX */
		NEXUS_AudysseyAbx_AddInput(abxPostProc,
						NEXUS_AudysseyAdv_GetConnector(advPostProc));
	#endif
#elif defined NEXUS_ABX_ENABLE
		/* DECODER -> ABX */
	    NEXUS_AudysseyAbx_AddInput(abxPostProc, 
				NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

#if defined NEXUS_ADV_ENABLE
	#if defined	NEXUS_ABX_ENABLE
		/* ADV -> ABX -> DAC */
	    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
						NEXUS_AudysseyAbx_GetConnector(abxPostProc));
	
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			/* ADV -> ABX -> SPDIF */
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_AudysseyAbx_GetConnector(abxPostProc));
		}
	#else		
		/* ADV -> DAC */
	    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
						NEXUS_AudysseyAdv_GetConnector(advPostProc));
	
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			/* ADV -> SPDIF */
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_AudysseyAdv_GetConnector(advPostProc));
		}
	#endif
#elif defined NEXUS_ABX_ENABLE
	    /* ABX -> DAC */
	    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
				NEXUS_AudysseyAbx_GetConnector(abxPostProc));
		
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			/* ABX -> SPDIF */
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_AudysseyAbx_GetConnector(abxPostProc));
		}
#endif
	}
#endif

#ifdef NEXUS_AVL_ENABLE
	else if (g_AVL_options.avl.enabled == true)
	{
		printf ("AVL Postprocessing added\n");
		NEXUS_AutoVolumeLevel_GetDefaultSettings(&avlSettings);

		avlSettings.enabled = g_AVL_options.avl.enabled;
		avlSettings.target = g_AVL_options.avl.target*100;      /* 0xFFFFFFFA;*/ /*-6dB dec*/
		avlSettings.lowerBound = g_AVL_options.avl.lowerBound*100; /*0xFFFFFFEE;*/ /*-18dB dec*/
		avlSettings.fixedBoost = g_AVL_options.avl.fixedBoost*100; /*this to be phased out as fixed Boost is variable as done by the AVL PP in F/w*/


#if 0
    bool enabled;                   /* If true, processing is enabled.  Otherwise this stage is bypassed */

    /* The settings below are typically used at defaults, but may be modified if desired.
       Careful study and analysis is recommended before adjusting these values.  */

    int target;                     /* Target level of output signal in terms of dBFs */
    int lowerBound;                 /* Lower bound for primary range of volume control */
    unsigned fixedBoost;            /* Amount of fixed boost (in dB) if level < LOWERBOUND */
    unsigned ref;                   /* Reference logarithmic gain value of 0 dBFs */
    unsigned alpha;                 /* Attenuation factor for Level Maximum */
    unsigned beta;					/* Attenuation factor for Level Mean */
    unsigned threshold;				/* Threshold for detecting active portion of signal . Input value should be multiplied by factor of 10 */
    unsigned dtfPercent;            /* Decay Time for DTF Percent delay (in seconds) */
    unsigned alpha2;                /* Attenuation factor for Level Minimum */
    unsigned fastGainRamp;          /* Fast Gain Ramp in milliseconds. Input value should be multiplied by factor of 10 */
    unsigned dtf;                   /* Decay Time Fraction. Input value should be multiplied by factor of 10 */

#endif
#if 1
		printf ("avlSettings.enabled =0x%x\n",avlSettings.enabled );
		printf ("avlSettings.target =0x%x\n",avlSettings.target );
		printf ("avlSettings.lowerBound =0x%x\n",avlSettings.lowerBound );
		printf ("avlSettings.fixedBoost =0x%x\n",avlSettings.fixedBoost );
		printf ("avlSettings.ref =0x%x\n",avlSettings.ref );
		printf ("avlSettings.alpha =0x%x\n",avlSettings.alpha );
		printf ("avlSettings.beta =0x%x\n",avlSettings.beta );
		printf ("avlSettings.threshold =0x%x\n",avlSettings.threshold );
		printf ("avlSettings.dtfPercent =0x%x\n",avlSettings.dtfPercent );
		printf ("avlSettings.alpha2 =0x%x\n",avlSettings.alpha2 );
		printf ("avlSettings.fastGainRamp =0x%x\n",avlSettings.fastGainRamp );
		printf ("avlSettings.dtf =0x%x\n",avlSettings.dtf );
#endif
		avlPostProc = NEXUS_AutoVolumeLevel_Open(&avlSettings);

		/* DECODER -> AVL connection */
	    NEXUS_AutoVolumeLevel_AddInput(avlPostProc,
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

		
		NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
						NEXUS_AutoVolumeLevel_GetConnector(avlPostProc));

		
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			/* ADV -> SPDIF */
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_AutoVolumeLevel_GetConnector(avlPostProc));
		}
		
		printf ("AVL Postprocessing Configured  \n");

	}
#endif

#ifdef NEXUS_SRSSTUDIOSOUND_ENABLE
	else if (
		     (sStudioSoundOptions.g_StudioSound_Options.enabled == true) && 
		     (sStudioSoundOptions.TVol_StandAlone == 0) && 
			 (sStudioSoundOptions.TsHD_StandAlone == 0)
			 )
	{
		printf ("SRS Postprocessing added as Complete SRS Studio Sound \n");

		
		srsPostProc = NEXUS_StudioSound_Open(&sStudioSoundOptions.g_StudioSound_Options);
		

		/* DECODER -> SRS connection */
	    NEXUS_StudioSound_AddInput(srsPostProc,
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

		
		NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
						NEXUS_StudioSound_GetConnector(srsPostProc));

		
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			/* ADV -> SPDIF */
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_StudioSound_GetConnector(srsPostProc));
		}
		
		printf ("SRS Postprocessing Configured  \n");


	}
	else if (sStudioSoundOptions.TsHD_StandAlone == 1)
	{

		printf ("Tru Surround HD Postprocessing added as Standalone \n");

		
		tshdPostProc = NEXUS_TruSurroundHd_Open(&sStudioSoundOptions.g_StudioSound_Options.truSurroundHd);
		
		

		/* DECODER -> SRS connection */
	    NEXUS_TruSurroundHd_AddInput(tshdPostProc,
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));

		
		NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
						NEXUS_TruSurroundHd_GetConnector(tshdPostProc));

		
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			/* ADV -> SPDIF */
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_TruSurroundHd_GetConnector(tshdPostProc));
		}
		
		printf ("Tru Surround HD Postprocessing Standalone Configured  \n");

	}
	else if (sStudioSoundOptions.TVol_StandAlone == 1)
	{

		printf ("Tru Volume Postprocessing added as Standalone \n");

		
		tvolPostProc = NEXUS_TruVolume_Open(&sStudioSoundOptions.g_StudioSound_Options.truVolume);
		
		

		/* DECODER -> SRS connection */
	    NEXUS_TruVolume_AddInput(tvolPostProc,
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

		
		NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
						NEXUS_TruVolume_GetConnector(tvolPostProc));

		
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			/* ADV -> SPDIF */
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_TruVolume_GetConnector(tvolPostProc));
		}
		
		printf ("Tru Volume Postprocessing Standalone Configured  \n");

	}
#else
#ifdef NEXUS_SRSTRUVOL_ENABLE
	else if (g_srs_tvol_settings.enabled == 1)
	{
		printf ("Adding Postprocessing: SRS TruVolume\n");		
		tvolPostProc = NEXUS_TruVolume_Open(&g_srs_tvol_settings);

		/* DECODER -> SRS connection */
	    NEXUS_TruVolume_AddInput(tvolPostProc,
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
		/* SRS -> DAC */
		NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
						NEXUS_TruVolume_GetConnector(tvolPostProc));
		/* SRS -> SPDIF */
		if (g_input_options.eSpdif_out == eSpdifOutPcm)			
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_TruVolume_GetConnector(tvolPostProc));
		
		printf ("Configured Postprocessing: SRS TruVolume\n");
	}
#endif
#ifdef NEXUS_SRSHD_ENABLE
	else if (g_srs_tshd_settings.enabled == 1)
	{
		printf ("Adding Postprocessing: SRS TruSurroundHD\n");	
		tshdPostProc = NEXUS_TruSurroundHd_Open(&g_srs_tshd_settings);
		
		/* DECODER -> SRS connection */
	    NEXUS_TruSurroundHd_AddInput(tshdPostProc,
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
		/* SRS -> DAC */
		NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), 
						NEXUS_TruSurroundHd_GetConnector(tshdPostProc));
		/* SRS -> SPDIF */
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_TruSurroundHd_GetConnector(tshdPostProc));
		
		printf ("Configured Postprocessing: SRS TruSurroundHD\n");
	}
#endif
#endif

#ifdef NEXUS_DV258_ENABLE
		/* DV258 related changes -- adding simple use case */
	else if(DV258Settings.enabled == 1 || g_input_options.dolby_vol_enable == true)
	{		
		printf ("DV258 Postprocessing added \n");
		dv258 = NEXUS_DolbyVolume258_Open(NULL);
		NEXUS_DolbyVolume258_SetSettings(dv258, &DV258Settings);

		if (NEXUS_AudioCodec_ePcmWav == g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec &&
			g_input_options.ChannelOpts[eChannelTypePrimary].num_channels <= 2)
				NEXUS_DolbyVolume258_AddInput(dv258, NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
		else
				NEXUS_DolbyVolume258_AddInput(dv258, NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
		
#ifdef NEXUS_DDRE_ENABLE
		if (g_input_options.ddre_enable == true)
		{
			printf ("DDRE Postprocessing added followed by DV258\n");
			ddre = NEXUS_DolbyDigitalReencode_Open(NULL);
			NEXUS_DolbyDigitalReencode_SetSettings(ddre, &DDRE_Settings);

			NEXUS_DolbyDigitalReencode_AddInput(ddre, 
							NEXUS_DolbyVolume258_GetConnector(dv258));
			
			NEXUS_AudioOutput_AddInput(
				NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
				NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eStereo));
			if (g_input_options.eSpdif_out == eSpdifOutPcm)
			{
				NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
						NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eStereo));
			}
		}
		else
#endif
		{
		
#if NEXUS_NUM_HDMI_OUTPUTS > 0
			printf("DV258: DV258 output connected to HDMI[0] \n");
			NEXUS_AudioOutput_AddInput(
				NEXUS_HdmiOutput_GetAudioConnector(g_platformConfig.outputs.hdmi[0]),
				NEXUS_DolbyVolume258_GetConnector(dv258));		
#elif NEXUS_NUM_I2S_OUTPUTS > 0	
			printf("DV258: DV258 output connected to I2S[0] \n");
			NEXUS_AudioOutput_AddInput(
				NEXUS_I2sOutput_GetConnector(g_platformConfig.outputs.i2s[0]),
				NEXUS_DolbyVolume258_GetConnector(dv258));
#else		
			printf("DV258: DV258 output NOT connected to any multichannel output port \n");
#endif
	
		}
	}
#endif	
#ifdef NEXUS_DDRE_ENABLE
	else if (g_input_options.ddre_enable == true)
	{
		printf ("DDRE Postprocessing added\n");
		ddre = NEXUS_DolbyDigitalReencode_Open(NULL);
		NEXUS_DolbyDigitalReencode_SetSettings(ddre, &DDRE_Settings);
		
		NEXUS_DolbyDigitalReencode_AddInput(ddre, 
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
		
		NEXUS_AudioOutput_AddInput(
			NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
			NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eStereo));
		if (g_input_options.eSpdif_out == eSpdifOutPcm)
		{
			NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eStereo));
		}
	}
#endif
	else
	{
		if (g_input_options.stereo_disable != true)
		{
			if (g_input_options.mixer_enable != true)
			{
				if (g_input_options.ChannelOpts[eChannelTypePrimary].num_channels <= 2)
				{
					NEXUS_AudioOutput_AddInput(
						NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
					if (g_input_options.eSpdif_out == eSpdifOutPcm)
					{
						NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
								NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
					}	
				}
				else
				{
					printf ("No Support for multi channel output on DAC. '-nchs' should be <= 2\n\n");
					exit (-1);
				}
			}
			else
			{
				NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
						NEXUS_AudioMixer_GetConnector(g_mixer));
				if (g_input_options.eSpdif_out == eSpdifOutPcm)
				{
					NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
							NEXUS_AudioMixer_GetConnector(g_mixer));
				}
			}
		}
	}
	
	/* SPDIF OUTPUT CONFIGURATION */
	switch (g_input_options.eSpdif_out)
	{
		case eSpdifOutPcm:
			/* Configuration done as a part of DAC configuration */
			break;
		case eSpdifOutCompressed:
#ifdef NEXUS_DDRE_ENABLE
			if (g_input_options.ddre_enable == true)			
			{
				printf ("DDRE SPDIF:compressed\n");
				NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eCompressed));
			}
			else
#endif		
			{
				if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3) 
					|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3Plus)
					|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eWmaPro))
				{
					NEXUS_AudioOutput_AddInput(
						NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
				}
				else if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacAdts) 
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacLoas)
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusAdts)
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusLoas))
				{
					ac3Encode = NEXUS_Ac3Encode_Open(NULL);
					/* Send multichannel output to DTS encoder */
					NEXUS_Ac3Encode_AddInput(ac3Encode,
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
					/* Send compressed DTS data to SPDIF */
					NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
											   NEXUS_Ac3Encode_GetConnector(ac3Encode));
				}
				else /* DTS Passthru and WMA Passthru support will have to be enabled here */
				{
					NEXUS_AudioOutput_AddInput(
						NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
						NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
				}
			}
			break;
		case eSpdifOutEncAc3:
			ac3Encode = NEXUS_Ac3Encode_Open(NULL);
		    NEXUS_Ac3Encode_AddInput(ac3Encode,
		        NEXUS_AudioDecoder_GetConnector(primaryDecoder, g_input_options.encInput));
		    /* Send compressed AC3 data to SPDIF */
		    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
		                               NEXUS_Ac3Encode_GetConnector(ac3Encode));
			break;
		case eSpdifOutEncDts:
			dtsEncode = NEXUS_DtsEncode_Open(NULL);
		    NEXUS_DtsEncode_AddInput(dtsEncode,
		        NEXUS_AudioDecoder_GetConnector(primaryDecoder, g_input_options.encInput));
		    /* Send compressed DTS data to SPDIF */
		    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
		                               NEXUS_DtsEncode_GetConnector(dtsEncode));
			break;
		case eSpdifOutPcmMultiLR:
			printf ("app_audio: SPDIF channel pair: L_R\n");
			NEXUS_AudioOutputGroup_GetDefaultCreateSettings(&outputGroupCreateSettings);
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eLeftRight] = NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]);
		#if NEXUS_NUM_I2S_OUTPUTS > 0
			printf ("\n\n\nConfiguring I2S[0]\n");
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eLeftRightSurround] = NEXUS_I2sOutput_GetConnector(g_platformConfig.outputs.i2s[0]);
		#endif
		#if NEXUS_NUM_I2S_OUTPUTS > 1
			printf ("\nConfiguring I2S[1]\n");
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eCenterLfe] = NEXUS_I2sOutput_GetConnector(g_platformConfig.outputs.i2s[1]);
		#elif NEXUS_NUM_HDMI_OUTPUTS > 0
			printf ("\nConfiguring HDMI[0]\n");
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eCenterLfe] = NEXUS_HdmiOutput_GetAudioConnector(g_platformConfig.outputs.hdmi[0]);
		#endif
			outputGroup = NEXUS_AudioOutputGroup_Create(&outputGroupCreateSettings);
			if ( NULL == outputGroup )
			{
				fprintf(stderr, "Unable to open output group\n");
				return 0;
			}
#ifdef NEXUS_DDRE_ENABLE
		    if (g_input_options.ddre_enable == true)			
			{
				
				NEXUS_AudioOutput_AddInput(NEXUS_AudioOutputGroup_GetConnector(outputGroup),
											NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eMultichannel));
			}
			else
#endif			
			{
			/* Connect group to decoder */
			NEXUS_AudioOutput_AddInput(NEXUS_AudioOutputGroup_GetConnector(outputGroup),
									   NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
			}
			printf ("\napp_audio: SPDIF channel pair: L_R DONE\n");
			break;
		case eSpdifOutPcmMultiLsRs:
			printf ("app_audio: SPDIF channel pair: Ls_Rs\n");
			NEXUS_AudioOutputGroup_GetDefaultCreateSettings(&outputGroupCreateSettings);
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eLeftRightSurround] = NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]);
		#if NEXUS_NUM_I2S_OUTPUTS > 0
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eLeftRight] = NEXUS_I2sOutput_GetConnector(g_platformConfig.outputs.i2s[0]);
		#endif
		#if NEXUS_NUM_I2S_OUTPUTS > 1
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eCenterLfe] = NEXUS_I2sOutput_GetConnector(g_platformConfig.outputs.i2s[1]);
		#elif NEXUS_NUM_HDMI_OUTPUTS > 0
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eCenterLfe] = NEXUS_HdmiOutput_GetAudioConnector(g_platformConfig.outputs.hdmi[0]);
		#endif
			outputGroup = NEXUS_AudioOutputGroup_Create(&outputGroupCreateSettings);
			if ( NULL == outputGroup )
			{
				fprintf(stderr, "Unable to open output group\n");
				return 0;
			}
#ifdef NEXUS_DDRE_ENABLE
		    if (g_input_options.ddre_enable == true)			
			{
				
				NEXUS_AudioOutput_AddInput(NEXUS_AudioOutputGroup_GetConnector(outputGroup),
									   NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eMultichannel));
			}
			else
#endif			
			{			/* Connect group to decoder */
				NEXUS_AudioOutput_AddInput(NEXUS_AudioOutputGroup_GetConnector(outputGroup),
									   NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
			}
			break;
		case eSpdifOutPcmMultiCLFE:
			printf ("app_audio: SPDIF channel pair: C_LFE\n");
			NEXUS_AudioOutputGroup_GetDefaultCreateSettings(&outputGroupCreateSettings);
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eCenterLfe] = NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]);
		#if NEXUS_NUM_I2S_OUTPUTS > 0
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eLeftRightSurround] = NEXUS_I2sOutput_GetConnector(g_platformConfig.outputs.i2s[0]);
		#endif
		#if NEXUS_NUM_I2S_OUTPUTS > 1
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eLeftRight] = NEXUS_I2sOutput_GetConnector(g_platformConfig.outputs.i2s[1]);
		#elif NEXUS_NUM_HDMI_OUTPUTS > 0
			outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eLeftRight] = NEXUS_HdmiOutput_GetAudioConnector(g_platformConfig.outputs.hdmi[0]);
		#endif
			outputGroup = NEXUS_AudioOutputGroup_Create(&outputGroupCreateSettings);
			if ( NULL == outputGroup )
			{
				fprintf(stderr, "Unable to open output group\n");
				return 0;
			}
#ifdef NEXUS_DDRE_ENABLE
		    if (g_input_options.ddre_enable == true)			
			{
				
				NEXUS_AudioOutput_AddInput(NEXUS_AudioOutputGroup_GetConnector(outputGroup),
									   NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eMultichannel));
			}
			else
#endif			
			{			/* Connect group to decoder */
				NEXUS_AudioOutput_AddInput(NEXUS_AudioOutputGroup_GetConnector(outputGroup),
									   NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
			}
			break;
		case eSpdifOutPcmSimulLR:
			printf ("\n\napp_audio: SPDIF channel pair: Simul Stereo\n");
		#if NEXUS_NUM_HDMI_OUTPUTS
			printf ("Configuring HDMI[0] : Multi channel\n");
			NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(g_platformConfig.outputs.hdmi[0]),
					NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));		
			printf ("Configuring SPDIF[0] : Simul Stereo\n");
			NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(g_platformConfig.outputs.spdif[0]),
					NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
		#else
			printf ("\nError: No HDMI port available. Unable to configure simul stereo output\n");
			exit (-1);
		#endif
			break;
		default:
			break;
	}
	
#if NEXUS_NUM_HDMI_OUTPUTS
	/* HDMI output only on demand */
	if (g_input_options.eHdmi_out != eHdmiOutLast)
	{
		/* HDMI OUTPUT CONFIGURATION */
		switch (g_input_options.eHdmi_out)
		{
			case eHdmiOutPcmStereo:
			case eHdmiOutPcmMultiChannel:
				/* Configuration done as a part of DAC configuration */
#ifdef NEXUS_DDRE_ENABLE
				if (g_input_options.ddre_enable == true)
					hdmiInput = NEXUS_DolbyDigitalReencode_GetConnector(ddre, hdmiDecoderConnectorType);
#endif				
				break;
			case eHdmiOutCompressed:
#ifdef NEXUS_DDRE_ENABLE
				if (g_input_options.ddre_enable == true)			
				{
					hdmiInput = NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioDecoderConnectorType_eCompressed);
				}
				else				
#endif			
				{
					if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3) 
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3Plus))
					{
							hdmiInput = NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eCompressed);
					}
					else if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacAdts) 
							|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacLoas)
							|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusAdts)
							|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusLoas))
					{
						ac3Encode = NEXUS_Ac3Encode_Open(NULL);
						/* Send multichannel output to DTS encoder */
						NEXUS_Ac3Encode_AddInput(ac3Encode,
							NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
						/* Send compressed DTS data to HDMI */
						hdmiInput = NEXUS_Ac3Encode_GetConnector(ac3Encode);
					}
					else /* DTS Passthru and WMA Passthru support will have to be enabled here */
					{
						hdmiInput = NEXUS_AudioDecoder_GetConnector(primaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
					}
				}
				break;
			case eHdmiOutEncAc3:
				ac3Encode = NEXUS_Ac3Encode_Open(NULL);
				NEXUS_Ac3Encode_AddInput(ac3Encode,
					NEXUS_AudioDecoder_GetConnector(primaryDecoder, g_input_options.encInput));
				/* Send compressed AC3 data to HDMI */
				hdmiInput = NEXUS_Ac3Encode_GetConnector(ac3Encode);
				break;
			case eHdmiOutEncDts:
				dtsEncode = NEXUS_DtsEncode_Open(NULL);
				NEXUS_DtsEncode_AddInput(dtsEncode,
					NEXUS_AudioDecoder_GetConnector(primaryDecoder, g_input_options.encInput));
				/* Send compressed DTS data to HDMI */
				hdmiInput = NEXUS_DtsEncode_GetConnector(dtsEncode);
				break;
			default:
				break;
		}

		if ( g_input_options.eSpdif_out != eSpdifOutPcmSimulLR)
		{
			printf ("\n\n Configuring HDMI[0]\n");
			NEXUS_AudioOutput_AddInput(
				NEXUS_HdmiOutput_GetAudioConnector(g_platformConfig.outputs.hdmi[0]),hdmiInput);
		}
		else
			printf("\nError: HDMI[0] is already connected with decoder multi-channel output\n");
	}
#endif

	SetupDecodeChannel(primaryDecoder, &audioProgram, eChannelTypePrimary);
	
	if (g_input_options.mixer_enable == true)
	{
		SetupDecodeChannel(associatedDecoder, &associatedAudioProgram, eChannelTypeAssociated);
	}

#ifdef BDSP_FW_RBUF_CAPTURE
	start_rbuf_channels_from_env();
#endif

    /*for(;;) {
        NEXUS_PlaybackStatus playbackStatus;
        NEXUS_Error rc;

        rc = BKNI_WaitForEvent(event, 1000000);
        if(rc!=NEXUS_TIMEOUT) {
            break;
        }

        if(NEXUS_Playback_GetStatus(playback,&playbackStatus)==NEXUS_SUCCESS) {
            printf("audio position=%u.%03u (%u,%u) sec \n", (unsigned)playbackStatus.position/1000, (unsigned)playbackStatus.position%1000, (unsigned)playbackStatus.first/1000, (unsigned)playbackStatus.last/1000 );
        }
    }*/
	
	num_chars = 0;
	printf ("\napp_audio> ");
	input_string[0] = '\0';
	while ((int)loop_idx <= (int)g_input_options.loop_count)
	{
		/* Poll keyboard inputs here*/
		/* fcntl(STDIN_FILENO, F_SETFL, flags|O_NONBLOCK);
		retval = read(STDIN_FILENO, &one_char, 1); */

		/*SW7425-5703*/
		one_char = getchar();
		retval = 0;
		if(retval != -1)
		{
			if (num_chars == 0)
			{
				/* All Single char commands need to be handled here */
				if (one_char == 'q')
				{
					fflush(stdout);
					break;
				}
				else if (one_char == 0xA)
				{
					printf ("app_audio> ");
					fflush(stdout);
					continue;
				}
				else if (one_char == '?')
				{
					printf ("Commands Available\n------------------\n");
					printf ("q                   - to quit\n");
					printf ("mono_mode           <left,right,stereo,mixed>\n");
					if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3)
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3Plus)
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacAdts) 
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacLoas)
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusAdts)
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusLoas))
					{
						printf ("cut             <0 - 100>\n");
						printf ("boost           <0 - 100>\n");
						if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3) 
							|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3Plus))
						{
							printf ("downmix_mode    <loro,ltrt,auto>\n");
							printf ("drc_mode	     <line,rf,custa,custd>\n");
							printf ("substream_id    <0 - 3>\n");
						}
						else
						{
							printf ("downmix_mode    <matrix,arib,loro,ltrt>\n");
							printf ("drc_mode	     <line,rf>\n");
						}
					}
					else if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eDts)
							 || (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eDtsHd)
							 || (g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eDtsCd))
					{
						printf ("version_info\n");
					}
#ifdef NEXUS_DV258_ENABLE
					else if(DV258Settings.enabled == true)
					{	
						printf ("\nDV258 Commands\n------------------\n");
						printf ("irl <0 - 2080>\n");
						printf ("orl <0 - 2080>\n");
						printf ("co <0 - 480>\n");
						printf ("mode <0 or 1>\n");
						printf ("avm <(-1536) - 480>\n");
						printf ("dvm <(-1536) - 480>\n");
						printf ("lvle <0 or 1>\n");
						printf ("lvla <0 - 10>\n");
						printf ("mse <0 or 1>\n");
						printf ("hmen <0 or 1>\n");
						printf ("pregain <0 - 480>\n\n");	
					}
#endif					
					else
					{
						printf ("No commands available at this time\n");
					}
					continue;
				}
			}
			if (one_char == 0xA)
			{
				/* Execute Command */
				/*printf ("Executing %s", input_string);*/
				retval = 0;
				sscanf(input_string, "%s %s", cmd_name, arg_val);
				if (!strcmp(cmd_name, "mono_mode"))
				{
					NEXUS_AudioDecoder_GetSettings (primaryDecoder, &decSettings);
					if (strcmp(arg_val, "left") == 0)
					{
						decSettings.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eLeft;
					}
					else if (strcmp(arg_val, "right") == 0)
					{
						decSettings.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eRight;
					}
					else if (strcmp(arg_val, "stereo") == 0)
					{
						decSettings.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eStereo;
					}
					else if (strcmp(arg_val, "mixed") == 0)
					{
						decSettings.dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eMix;
					}
					else
					{
				        printf("error: illegal dual mono mode specifier : %s", arg_val);
						retval = -1;
					}
					if (retval == 0)
					{
						printf ("Setting the new configuration for the decoder : %s", input_string);
						NEXUS_AudioDecoder_SetSettings (primaryDecoder, &decSettings);
					}
				}
				else
				{
					NEXUS_AudioDecoder_GetCodecSettings(primaryDecoder, g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec, &codecSettings);
					if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacAdts)
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacLoas)
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusAdts)
						|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusLoas))
					{
						if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusAdts)
							|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAacPlusLoas))
						{
							paacSettings = &codecSettings.codecSettings.aac;
						}
						else
						{
							paacSettings = &codecSettings.codecSettings.aacPlus;
						}
						if (!strcmp(cmd_name, "cut"))
						{
							paacSettings->cut = decOrhex_atoi(arg_val);
							if (paacSettings->cut > 100)
							{
								printf("error: illegal drc_hdr value cut : %s", arg_val);
								retval = -1;
							}
						}
						else if (!strcmp(cmd_name, "boost"))
						{
							paacSettings->boost = decOrhex_atoi(arg_val);
							if (paacSettings->boost > 100)
							{
								printf("error: illegal drc_ldr value boost : %s", arg_val);
								retval = -1;
							}
						}
						else if (!strcmp(cmd_name, "downmix_mode")) 
						{
							if (!strcmp(arg_val, "matrix"))
							{
								paacSettings->downmixMode = NEXUS_AudioDecoderAacDownmixMode_eMatrix;
							}
							else if (!strcmp(arg_val, "arib"))
							{
								paacSettings->downmixMode = NEXUS_AudioDecoderAacDownmixMode_eArib;
							}
							else if (!strcmp(arg_val, "ltrt"))
							{
								paacSettings->downmixMode = NEXUS_AudioDecoderAacDownmixMode_eLtRt;
							}
							else if (!strcmp(arg_val, "loro"))
							{
								paacSettings->downmixMode = NEXUS_AudioDecoderAacDownmixMode_eLoRo;
							}
							else
							{
								printf("error: illegal downmix mode specifier : %s", arg_val);
								retval = -1;
							}
						}
						else if (!strcmp(cmd_name, "drc_mode"))
						{
							if (strcmp(arg_val, "line") == 0)
							{
								paacSettings->drcMode = NEXUS_AudioDecoderDolbyPulseDrcMode_eLine;
							}
							else if (strcmp(arg_val, "rf") == 0)
							{
								paacSettings->drcMode = NEXUS_AudioDecoderDolbyPulseDrcMode_eRf;
							}
							else
							{
						        printf("error: illegal drc compression mode specifier : %s", arg_val);
								retval = -1;
							}
						}
						else
						{
							printf ("Invalid Command : %s", input_string);
							retval = -1;
						}
					}
					else if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3) 
							|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eAc3Plus))
					{
						NEXUS_AudioDecoder_GetCodecSettings(primaryDecoder, g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec, &codecSettings);
						if (!strcmp(cmd_name, "cut"))
						{
							codecSettings.codecSettings.ac3.cut = decOrhex_atoi(arg_val);
							if (codecSettings.codecSettings.ac3.cut > 100)
							{
								printf("error: illegal drc_hdr value cut : %s", arg_val);
								retval = -1;
							}
						}
						else if (!strcmp(cmd_name, "boost"))
						{
							codecSettings.codecSettings.ac3.boost = decOrhex_atoi(arg_val);
							if (codecSettings.codecSettings.ac3.boost > 100)
							{
								printf("error: illegal drc_ldr value boost : %s", arg_val);
								retval = -1;
							}
						}
#ifdef CUSTOM_VOICE_ENABLE
						else if (!strcmp(cmd_name, "cv_vol"))
						{
							if (g_input_options.custom_voice_enable == true)
							{
								int32_t volume;
								volume = decOrhex_atoi(arg_val);
								if ((volume > 100) || (volume < 0))
								{
									printf("error: illegal drc_ldr value boost : %s", arg_val);
									retval = -1;
								}
								else
								{
									NEXUS_CustomVoice_OsdVolume(customVoice, volume);
								}
							}
							else
							{
								printf ("Custom Voice is not enabled : invalid option cv_vol");
								retval = -1;
							}
						}
#endif
						else if (!strcmp(cmd_name, "downmix_mode")) 
						{
							if (!strcmp(arg_val, "ltrt"))
							{
								codecSettings.codecSettings.ac3.stereoDownmixMode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible;
							}
							else if (!strcmp(arg_val, "loro"))
							{
								codecSettings.codecSettings.ac3.stereoDownmixMode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard;
							}
							else
							{
								codecSettings.codecSettings.ac3.stereoDownmixMode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic;
							}
						}
						else if (!strcmp(cmd_name, "drc_mode"))
						{
							if (strcmp(arg_val, "line") == 0)
							{
								codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
							}
							else if (strcmp(arg_val, "rf") == 0)
							{
								codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
							}
							else if ((strcmp(arg_val, "custa") == 0) || (strcmp(arg_val, "custA") == 0))
							{
								codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eCustomA;
							}
							else if ((strcmp(arg_val, "custd") == 0) || (strcmp(arg_val, "custD") == 0))
							{
								codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eCustomD;
							}
							else
							{
						        printf("error: illegal drc compression mode specifier : %s", arg_val);
								retval = -1;
							}
						}
						else if (!strcmp(cmd_name, "substream_id")) 
						{
							codecSettings.codecSettings.ac3.substreamId = decOrhex_atoi(arg_val);
							if (codecSettings.codecSettings.ac3.substreamId <= 0x3)
							{
								codecSettings.codecSettings.ac3.substreamId = NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible;
							}
							else
							{
								printf ("Invalid value for substream_id : %s", arg_val);
								retval = -1;
							}
						}
						else
						{
							printf ("Invalid Command : %s", input_string);
							retval = -1;
						}
					}
					else if ((g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eDts) 
							|| (g_input_options.ChannelOpts[eChannelTypePrimary].audio_codec == NEXUS_AudioCodec_eDtsHd)
							||(g_input_options.ChannelOpts[CurrentChannelType].audio_codec == NEXUS_AudioCodec_eDtsCd))
					{
						if (!strcmp(cmd_name, "version_info"))
						{
							print_version_info(primaryDecoder);
							retval = 1;
						}
						else
						{
							printf ("Invalid Command : %s", input_string);
							retval = -1;
						}
					}
#ifdef NEXUS_DV258_ENABLE
					else if(DV258Settings.enabled == true)
					{						
						dv258_cmd_str[0]=cmd_name;
						dv258_cmd_str[1]=arg_val;
						/*printf ("\n\ndv258_cmd_str[0]:%s: dv258_cmd_str[1]:%s:\n\n", dv258_cmd_str[0], dv258_cmd_str[1]);*/
						retval = parse_DV258_options(dv258_cmd_str);
						if (retval == 1)
						{
						    printf ("Setting the new configuration for DV258 : %s", input_string);
							NEXUS_DolbyVolume258_SetSettings(dv258, &DV258Settings);
							retval = 1;	
						}		
					}
#endif
					else
					{
						printf ("Invalid Command : %s", input_string);
						retval = -1;
					}
					if (retval == 0)
					{
						printf ("Setting the new configuration for the decoder : %s", input_string);
						NEXUS_AudioDecoder_SetCodecSettings(primaryDecoder, &codecSettings);
					}
				}
				
				num_chars = 0;
				printf ("\napp_audio> ");
				fflush (stdout);
				input_string[0] = '\0';
			}
			else
			{
				input_string[num_chars++] = one_char;
				input_string[num_chars] = '\0';
			}
		}
		fcntl(STDIN_FILENO, F_SETFL, flags);
	}

	if (g_input_options.mixer_enable == true)
	{
		printf ("Calling secondary audio decoder stop\n");
		UnSetupDecodeChannel(associatedDecoder, &associatedAudioProgram, eChannelTypeAssociated);
	}

#ifdef BDSP_FW_RBUF_CAPTURE
	printf ("Pausing for a few seconds before stopping capture\n");
	sleep (2);
#endif
	printf ("Calling primary audio decoder stop\n");
	/*printf ("SARA DEBUGG %x \n",audioProgram.stcChannel->swPcrOffset);*/


	
	UnSetupDecodeChannel(primaryDecoder, &audioProgram, eChannelTypePrimary);
	/* TBD: Uncomment this when UnSetupDecodeChannel is fixed */
    /*NEXUS_Platform_Uninit();*/
#ifdef BDSP_FW_RBUF_CAPTURE
	printf ("Calling brbuf_uninit_ringbuf_capture\n");
	brbuf_uninit_ringbuf_capture();
#endif

    return 0;
}

/* Copied from nexus/utils/cmdline_args.c */
struct {
    NEXUS_VideoCodec nexus;
    bvideo_codec settop;
} g_videoCodec[] = {
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_none},
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_unknown},
    {NEXUS_VideoCodec_eMpeg1, bvideo_codec_mpeg1},
    {NEXUS_VideoCodec_eMpeg2, bvideo_codec_mpeg2},
    {NEXUS_VideoCodec_eMpeg4Part2, bvideo_codec_mpeg4_part2},
    {NEXUS_VideoCodec_eH263, bvideo_codec_h263},
    {NEXUS_VideoCodec_eH264, bvideo_codec_h264},
    {NEXUS_VideoCodec_eVc1, bvideo_codec_vc1},
    {NEXUS_VideoCodec_eVc1SimpleMain, bvideo_codec_vc1_sm},
    {NEXUS_VideoCodec_eDivx311, bvideo_codec_divx_311},
    {NEXUS_VideoCodec_eAvs, bvideo_codec_avs}
};

struct {
    NEXUS_AudioCodec nexus;
    baudio_format settop;
} g_audioCodec[] = {
   {NEXUS_AudioCodec_eUnknown, baudio_format_unknown},
   {NEXUS_AudioCodec_eMpeg, baudio_format_mpeg},
   {NEXUS_AudioCodec_eMp3, baudio_format_mp3},
   {NEXUS_AudioCodec_eAac, baudio_format_aac},
   {NEXUS_AudioCodec_eAacPlus, baudio_format_aac_plus},
   {NEXUS_AudioCodec_eAacPlusAdts, baudio_format_aac_plus_adts},
   {NEXUS_AudioCodec_eAacPlusLoas, baudio_format_aac_plus_loas},
   {NEXUS_AudioCodec_eAc3, baudio_format_ac3},
   {NEXUS_AudioCodec_eAc3Plus, baudio_format_ac3_plus},
   {NEXUS_AudioCodec_eDts, baudio_format_dts},
   {NEXUS_AudioCodec_eLpcmHdDvd, baudio_format_lpcm_hddvd},
   {NEXUS_AudioCodec_eLpcmBluRay, baudio_format_lpcm_bluray},
   {NEXUS_AudioCodec_eDtsHd, baudio_format_dts_hd},
   {NEXUS_AudioCodec_eWmaStd, baudio_format_wma_std},
   {NEXUS_AudioCodec_eWmaPro, baudio_format_wma_pro},
   {NEXUS_AudioCodec_eLpcmDvd, baudio_format_lpcm_dvd},
   {NEXUS_AudioCodec_eAvs, baudio_format_avs},
   {NEXUS_AudioCodec_eAmr, baudio_format_amr},
   {NEXUS_AudioCodec_eDra, baudio_format_dra},
   {NEXUS_AudioCodec_ePcmWav, baudio_format_pcm}
};

struct {
    NEXUS_TransportType nexus;
    unsigned settop;
} g_mpegType[] = {
    {NEXUS_TransportType_eTs, bstream_mpeg_type_unknown},
    {NEXUS_TransportType_eEs, bstream_mpeg_type_es},
    {NEXUS_TransportType_eTs, bstream_mpeg_type_bes},
    {NEXUS_TransportType_eMpeg2Pes, bstream_mpeg_type_pes},
    {NEXUS_TransportType_eTs, bstream_mpeg_type_ts},
    {NEXUS_TransportType_eDssEs, bstream_mpeg_type_dss_es},
    {NEXUS_TransportType_eDssPes, bstream_mpeg_type_dss_pes},
    {NEXUS_TransportType_eVob, bstream_mpeg_type_vob},
    {NEXUS_TransportType_eAsf, bstream_mpeg_type_asf},
    {NEXUS_TransportType_eAvi, bstream_mpeg_type_avi},
    {NEXUS_TransportType_eMpeg1Ps, bstream_mpeg_type_mpeg1},
    {NEXUS_TransportType_eMp4, bstream_mpeg_type_mp4},
    {NEXUS_TransportType_eMkv, bstream_mpeg_type_mkv},
    {NEXUS_TransportType_eWav, bstream_mpeg_type_wav},
    {NEXUS_TransportType_eMp4Fragment, bstream_mpeg_type_mp4_fragment},
    {NEXUS_TransportType_eFlv, bstream_mpeg_type_flv}
};

#define CONVERT(g_struct) \
    unsigned i; \
    for (i=0;i<sizeof(g_struct)/sizeof(g_struct[0]);i++) { \
        if (g_struct[i].settop == settop_value) { \
            return g_struct[i].nexus; \
        } \
    } \
    printf("unable to find Settop API value %d in %s\n", settop_value, #g_struct); \
    return g_struct[0].nexus

NEXUS_VideoCodec b_videocodec2nexus(bvideo_codec settop_value)
{
    CONVERT(g_videoCodec);
}

NEXUS_AudioCodec b_audiocodec2nexus(baudio_format settop_value)
{
    CONVERT(g_audioCodec);
}

NEXUS_TransportType b_mpegtype2nexus(bstream_mpeg_type settop_value)
{
    CONVERT(g_mpegType);
}

/* Ring buffer capture specific funtions */
#ifdef BDSP_FW_RBUF_CAPTURE
/* ******************************** */

static
void
rbuf_capture_assert_fail(char *statement,
                         char *file,
                         int line,
                         ...)
{
	printf("\n--RBUF-- ASSERT FAILED! '%s' {%s:%d}\n",statement, file, line);
	return;
}

static
void
rbuf_capture_alert_fail(char *statement,
                        char *file,
                        int line,
                        ...)
{
	printf("\n--RBUF-- ALERT! '%s' {%s:%d}\n",statement, file, line);
	return;
}

static
/*inline*/
int32_t
rbuf_capture_device_open(int8_t *device_name)
{
	RBUF_ASSERT(NULL != device_name, "0x%08X", device_name);

	if(rbuf_capture_app_player)
	{
		/* Create / append to file*/
		return open((char *)device_name,
					(O_CREAT | O_WRONLY | O_APPEND),
					((S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH | S_IWOTH)));
	}
	else
	{
		/* Create / overwrite file*/
		return open((char *)device_name,
					(O_CREAT | O_WRONLY | O_TRUNC | O_DIRECT),
					((S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH | S_IWOTH)));
	}
}

static
/*inline*/
int32_t
rbuf_capture_device_write(int32_t device, void *buf, int32_t count)
{
  return write(device, buf, count);
}

static
/*inline*/
int32_t
rbuf_capture_device_close(int32_t device)
{
  return close(device);
}

static
char *
rbuf_capture_channel_get_type_string(uint32_t   path,
                                     uint32_t	channel,
									 uint32_t	rbuf_id,
                                     BDSP_P_RbufType type)
{
  static char buf[128];
  char *str;

  switch (type) {
    case BDSP_P_RbufType_eDecode: {
      static char *dec="DECODER CHANNEL";
      str = dec;
      break;
    }
    case BDSP_P_RbufType_ePassthru: {
      static char *pass="PASSTHRU CHANNEL";
      str = pass;
      break;
    }
    case BDSP_P_RbufType_eMixer: {
      static char *mix="MIXER CHANNEL";
      str = mix;
      break;
    }
    case BDSP_P_RbufType_eEncoder: {
      static char *enc="ENCODER CHANNEL";
      str = enc;
      break;
    }
    case BDSP_P_RbufType_eTranscode: {
      static char *tran="TRANSCODE CHANNEL";
      str = tran;
      break;
    }
    case BDSP_P_RbufType_eSM: {
      static char *spkr="SPKR MGMT CHANNEL";
      str = spkr;
      break;
    }
    default: {
      RBUF_ASSERT(0, "");
      str = NULL;
      break;
    }
  }
  if(rbuf_id != RBUF_ID_INVALID) {
	sprintf(buf, "PATH[%d]:%s[%d][%d]", path, str, channel, rbuf_id);
  }
  else {
	  sprintf(buf, "PATH[%d]:%s[%d]", path, str, channel);
  }
  return buf;
}

static
char *
rbuf_capture_channel_get_info_string(uint32_t          path,
                                     uint32_t          channel,
                                     BDSP_P_RbufType   type,
                                     uint32_t          rbuf_id)
{
  static char buf[128];

  sprintf(buf, "%s",
          rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type));
  
  return buf;
}

static
void
rbuf_capture_alert(rbuf_capture_channel_t *cap)
{
  RBUF_PRINT(("******************************************************************"));
  RBUF_PRINT(("*                     !!!! ALERT !!!!"));
  RBUF_PRINT(("* Error detected for %s",
             rbuf_capture_channel_get_info_string(cap->path,
                                                  cap->channel,
                                                  cap->type,
                                                  cap->rbuf_id)));

  /* Inactivate this channel*/
  cap->active = 0;

  RBUF_PRINT(("* Capture HAS NOW BEEN DISABLED for this ring buffer!!"));
  RBUF_PRINT(("******************************************************************"));

  return;
}

static
uint32_t
rbuf_capture_from_cap_q_to_disk(rbuf_capture_channel_t *cap)
{
	uint8_t  *cap_q_ptr;
	uint32_t cap_q_r_idx=0;
	uint32_t cap_q_w_idx=0;
	uint32_t cap_q_num_entries = 0;
	uint32_t bytes_captured = 0;
	uint32_t aligned_entries = 0;
	RBUF_CAPTURE_CHANNEL_STATE cap_q_to_disk_state;

	cap_q_to_disk_state = cap->cap_q_to_disk_state;

	if (!cap->active) 
	{
		/* Channel is not active, nothing to do!*/
		bytes_captured = 0;
		return bytes_captured;
	}
	
	if ((RBUF_CAPTURE_CHANNEL_STATE_INACTIVE == cap_q_to_disk_state) ||
		(RBUF_CAPTURE_CHANNEL_STATE_STOPPED == cap_q_to_disk_state)) 
	{
		/* Nothing to do*/
		bytes_captured = 0;
		return bytes_captured;
	}

	if (RBUF_CAPTURE_CHANNEL_STATE_PEND_STOP == cap_q_to_disk_state) 
	{
		/* Transition to stopped state*/
		cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
		bytes_captured = 0;
		return bytes_captured;
	}

	cap_q_ptr         = cap->q_ptr;
	cap_q_r_idx       = cap->q_r_idx;
	cap_q_w_idx       = cap->q_w_idx;
	cap_q_num_entries = cap->q_num_entries; 

	if (RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE == cap_q_to_disk_state) 
	{
		/* If both the Capture Q to disk is empty and the RBUF to Captre Q is*/
		/* stopped, then the output is stopped*/
		if ((Q_IS_EMPTY(cap_q_r_idx, cap_q_w_idx)) && 
			(RBUF_CAPTURE_CHANNEL_STATE_STOPPED == cap->rbuf_to_cap_q_state)) 
		{
			/* Transition to stopped state*/
			cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
			
			RBUF_DEBUG(("%s - DISK All data consumed",
						rbuf_capture_channel_get_info_string(cap->path,
						cap->channel,
						cap->type,
						cap->rbuf_id)));

			bytes_captured = 0;
			return bytes_captured;
		}
	}

	if ((RBUF_CAPTURE_CHANNEL_STATE_ACTIVE == cap_q_to_disk_state) ||
		(RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE == cap_q_to_disk_state)) 
	{ 
		if(cap->q_w_idx > cap->q_r_idx) 
		{
			if((cap->q_w_idx - cap->q_r_idx) >= RBUF_PAGE_SIZE) 
			{
				aligned_entries = (((cap->q_w_idx - cap->q_r_idx)/RBUF_PAGE_SIZE)*RBUF_PAGE_SIZE);
				cap->rbuf_flush_onexit = false;
			}
			else 
			{
				cap->rbuf_flush_onexit = true;
				bytes_captured = 0;
			}
		}
		else 
		{
			if(cap->q_w_idx != cap->q_r_idx) 
			{
				aligned_entries = (RBUF_CAP_MAX_CHAN_ENTRIES - cap->q_r_idx);
			}
		}
		if(!cap->rbuf_flush_onexit) 
		{
			bytes_captured = rbuf_capture_device_write(cap->file, &cap_q_ptr[cap_q_r_idx], aligned_entries);
			cap->q_r_idx += aligned_entries;
		
			if(cap->q_r_idx >= RBUF_CAP_MAX_CHAN_ENTRIES) 
			{
				cap->q_r_idx = 0;
			}
		}
	}
	return bytes_captured;
}

static
void
rbuf_capture_rbuf_copy(rbuf_capture_channel_t *cap,
                       uint8_t  *rbuf_ptr,
                       uint32_t *rbuf_w_idx,
                       uint32_t rbuf_num_entries,
                       uint32_t rbuf_copy_bytes,

                       uint8_t  *cap_q_ptr,
                       uint32_t *cap_q_w_idx,
                       uint32_t cap_q_num_entries,
                       uint32_t *cap_q_avail_to_end_bytes,
                       uint32_t *cap_q_avail_from_start_bytes)
{
	uint32_t copy_bytes;
	uint32_t _cap_q_w_idx;
	uint32_t _rbuf_w_idx;

	_cap_q_w_idx = *cap_q_w_idx;
	_rbuf_w_idx	 = *rbuf_w_idx;

	/* Does the RBUF number of bytes fit into the capture Q's to end?*/
	if (rbuf_copy_bytes <= *cap_q_avail_to_end_bytes) 
	{
		/* RBUF segment will fit in capture Q to end (without wrap)*/
		copy_bytes = rbuf_copy_bytes;
		*cap_q_avail_to_end_bytes -= copy_bytes;
	} 
	else 
	{
		/* RBUF to end will NOT fit in capture Q to end (without wrap)*/

		if (*cap_q_avail_to_end_bytes) 
		{
			/* Fill to the capture Q wrap point*/
			copy_bytes = *cap_q_avail_to_end_bytes;

			RBUF_ASSERT((((_cap_q_w_idx+copy_bytes) <= cap_q_num_entries) && ((_rbuf_w_idx+copy_bytes) <= rbuf_num_entries)),
						"%s - 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X",
						rbuf_capture_channel_get_info_string(cap->path,
						cap->channel,
						cap->type,
						cap->rbuf_id),
						_cap_q_w_idx, cap_q_num_entries,
						_rbuf_w_idx, rbuf_num_entries, copy_bytes);  

			/* Invalidate the cache to cause burst read for memcpy*/
			/* NOTE:Data will never be written to the ring buffer by the MIPS*/
			/*      so the flush has no effect other than invalidate*/
			NEXUS_Memory_FlushCache(&rbuf_ptr[_rbuf_w_idx], copy_bytes);

			BKNI_Memcpy(&cap_q_ptr[_cap_q_w_idx], &rbuf_ptr[_rbuf_w_idx], copy_bytes);

			/* Should wrap to start since all copy to end bytes have been consumed*/
			RBUF_ASSERT((0 == Q_INC(_cap_q_w_idx, copy_bytes, 0, cap_q_num_entries)), "%s - 0x%08X 0x%08X 0x%08X",
						rbuf_capture_channel_get_info_string(cap->path,
						cap->channel,
						cap->type,
						cap->rbuf_id),
						_cap_q_w_idx, copy_bytes, cap_q_num_entries);
			
			_cap_q_w_idx = 0;
			/* No more end bytes available*/
			*cap_q_avail_to_end_bytes = 0;

			/* Can just increment, RBUF guaranteed to not wrap*/
			_rbuf_w_idx += copy_bytes;
			/* Now setup copy of remaining RBUF end bytes to the beginning*/
			/* of the capture Q*/
			copy_bytes = rbuf_copy_bytes - copy_bytes;
			*cap_q_avail_from_start_bytes -= copy_bytes;
		} 
		else 
		{
			/* No space at to the end of the Q, simply copy at the start*/

			copy_bytes = rbuf_copy_bytes;
			RBUF_ASSERT((*cap_q_avail_from_start_bytes >= copy_bytes), "%s - 0x%08X 0x%08X",
						rbuf_capture_channel_get_info_string(cap->path,
						cap->channel,
						cap->type,
						cap->rbuf_id),
						*cap_q_avail_from_start_bytes, copy_bytes);
			*cap_q_avail_from_start_bytes -= copy_bytes;
		}
	}
	RBUF_ASSERT((((_cap_q_w_idx+copy_bytes) <= cap_q_num_entries) && ((_rbuf_w_idx+copy_bytes) <= rbuf_num_entries)),
				"%s - 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X",
				rbuf_capture_channel_get_info_string(cap->path,
				cap->channel,
				cap->type,
				cap->rbuf_id),
				_cap_q_w_idx, cap_q_num_entries,
				_rbuf_w_idx, rbuf_num_entries, copy_bytes);  

	/* Invalidate the cache to cause burst read for memcpy*/
	/* NOTE:Data will never be written to the ring buffer by the MIPS*/
	/*      so the flush has no effect other than invalidate*/
	NEXUS_Memory_FlushCache(&rbuf_ptr[_rbuf_w_idx], copy_bytes);
	
	BKNI_Memcpy(&cap_q_ptr[_cap_q_w_idx], &rbuf_ptr[_rbuf_w_idx], copy_bytes);

	_cap_q_w_idx = Q_INC(_cap_q_w_idx, copy_bytes, 0, cap_q_num_entries);
	*cap_q_w_idx = _cap_q_w_idx;

	_rbuf_w_idx = Q_INC(_rbuf_w_idx, copy_bytes, 0, rbuf_num_entries);
	*rbuf_w_idx = _rbuf_w_idx;

	return;
}

static
void
rbuf_capture_from_rbuf_to_cap_q(rbuf_capture_channel_t *cap)
{
	uint8_t  *rbuf_base_virt;
	uint32_t rbuf_base_phys;
	uint32_t rbuf_r_idx;
	uint32_t rbuf_w_idx;
	uint32_t rbuf_num_entries;
	uint32_t rbuf_copy_bytes;

	uint32_t rbuf_copy_to_end_bytes;
	uint32_t rbuf_copy_from_start_bytes;

	uint8_t  *cap_q_ptr;
	uint32_t cap_q_r_idx;
	uint32_t cap_q_w_idx;
	uint32_t cap_q_num_entries;

	uint32_t cap_q_avail_to_end_bytes;
	uint32_t cap_q_avail_from_start_bytes;
	uint32_t cap_q_avail_total_bytes;

	uint32_t rbuf_cur_wraddr;
	uint32_t wr_cur_addr;
	uint32_t wr_prev_addr;
	uint32_t wr_cur_wrap;
	uint32_t wr_prev_wrap;

	uint32_t rbuf_cur_rdaddr;
	uint32_t rd_cur_wrap;
	uint32_t rd_prev_wrap;
	uint32_t rd_cur_addr;
	uint32_t rd_prev_addr;

	if (!cap->active) 
	{
		/* Channel is not active, nothing to do!*/
		return;
	}
	if ((RBUF_CAPTURE_CHANNEL_STATE_INACTIVE == cap->rbuf_to_cap_q_state) ||
		(RBUF_CAPTURE_CHANNEL_STATE_STOPPED == cap->rbuf_to_cap_q_state)) 
	{
		/* Nothing to do*/
		return;
	}
	if (RBUF_CAPTURE_CHANNEL_STATE_PEND_STOP == cap->rbuf_to_cap_q_state) 
	{
		/* Transition to stopped state*/
		cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
		return;
	}

	/* Check whether to initialize the ring buffer state.  Behavior is*/
	/* as follows:*/
	/*   Decoder / Passthru RBUFs:*/
	/*     1. Ring buffer registers are initialized with default values*/
	/*     2. Decoder adjusts write / end / start write addrss registers*/
	/*        NOTE::Start write must be greater than or equal to end address register*/
	/*     3. Decoder sets start write address register to less than end address*/
	/*     4. Initial write address shall be the start write address and the */
	/*        end address will be then be read to deterime size of RBUF.*/
	/*   Mixer RBUFs:*/
	/*     1. Ring buffer registers are initialized with default values*/
	/*     2. Write address WILL BE equal to base address register until*/
	/*        the final RBUF size is determined (end address register is written)*/
	/*     3. Once write address register does not equal base address, read*/
	/*        end address to determine size of RBUF.*/
	if (!cap->rbuf_initialized) 
	{
		uint32_t baseaddr;
		uint32_t endaddr;
		uint32_t freefull;
		uint32_t wrpoint;
		uint32_t entries;
		void     *virt;
		int32_t  result;

		/* If channel has never been initialized, it is still*/
		/* necessary to check for request to stop*/
		if (RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE == cap->rbuf_to_cap_q_state) 
		{
			cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
			RBUF_DEBUG(("%s - RBUF stopping before initialized!",
						rbuf_capture_channel_get_info_string(cap->path,
						cap->channel,
						cap->type,
						cap->rbuf_id)));
			return;
		}

		/* AUD_FMM_BF_CTRL_RINGBUF_X_RDADDR */
		rbuf_cur_rdaddr = (uint32_t)BREG_Read32(rbuf_reg_handle,
		cap->rbuf_reg_base+0x00);
		/* AUD_FMM_BF_CTRL_RINGBUF_X_WRADDR */
		rbuf_cur_wraddr = (uint32_t)BREG_Read32(rbuf_reg_handle,
		cap->rbuf_reg_base+0x04);
		/* AUD_FMM_BF_CTRL_RINGBUF_X_BASEADDR */
		baseaddr = (uint32_t)BREG_Read32(rbuf_reg_handle,
		cap->rbuf_reg_base+0x08);
		/* AUD_FMM_BF_CTRL_RINGBUF_X_ENDADDR */
		endaddr = (uint32_t)BREG_Read32(rbuf_reg_handle,
		cap->rbuf_reg_base+0x0C);
		/* AUD_FMM_BF_CTRL_RINGBUF_X_FREEFULL_MARK*/
		freefull = (uint32_t)BREG_Read32(rbuf_reg_handle,
		cap->rbuf_reg_base+0x10);
		/* AUD_FMM_BF_CTRL_RINGBUF_X_START_WRPOINT*/
		wrpoint = (uint32_t)BREG_Read32(rbuf_reg_handle,
		cap->rbuf_reg_base+0x14);

		RBUF_DEBUG(("%s - %08X r=%08X w=%08X b=%08X e=%08X f=%08X w=%08X",
					rbuf_capture_channel_get_info_string(cap->path,
														cap->channel,
														cap->type,
														cap->rbuf_id),
														cap->rbuf_reg_base,
														rbuf_cur_rdaddr,
														rbuf_cur_wraddr,
														baseaddr,
														endaddr,
														freefull,
														wrpoint));

		/* Registers default to zero when uninitialized*/
		if ((0 == rbuf_cur_rdaddr) || (0 == rbuf_cur_wraddr) || (0 == baseaddr) ||
		/*(0 == endaddr)         || (0 == freefull)        || (0 == wrpoint)) */
		(0 == endaddr) || (0 == wrpoint)) 
		{
			/* Not yet initialized*/
			return;
		}
		/* The initialized default end address will be maintained until write*/
		/* address starts to change. Until this change, no state can be initialized.*/
		if (baseaddr == rbuf_cur_wraddr) 
		{
			return;
		}

		/* Write address has changed*/
		if ((BDSP_P_RbufType_eDecode == cap->type) || (BDSP_P_RbufType_eEncoder == cap->type) ||
		(BDSP_P_RbufType_ePassthru == cap->type) || (BDSP_P_RbufType_eTranscode == cap->type)) 
		{
			/* Decoder / Passthru capture*/
			if (wrpoint >= endaddr) 
			{
				/* Final config is not yet complete*/
				return;
			} 
			else 
			{
				/* The start write address is now less than the end address,*/
				/* the end address must now be valid, fall through*/
				/* and compute RBUF parameters*/
			}
		} 
		else 
		{
			/* Mixer capture, write address is no longer equal to the*/
			/* base address, the end address must now be valid, fall through*/
			/* and compute RBUF parameters*/
			RBUF_ASSERT(((BDSP_P_RbufType_eMixer == cap->type) || (BDSP_P_RbufType_eSM == cap->type)), "%s",
							rbuf_capture_channel_get_info_string(cap->path,
							cap->channel,
							cap->type,
							cap->rbuf_id));
		}
		
		RBUF_ALERT((endaddr > baseaddr), "%s - 0x%08X 0x%08X",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id),
					baseaddr, endaddr);

		entries = (endaddr - baseaddr) + 1;

		/* Use the physical base address to compute the uncached virtual address*/
		result = BMEM_ConvertOffsetToAddress(rbuf_heap, baseaddr, &virt);
		RBUF_ASSERT((BERR_SUCCESS == result),"%s - 0x%08X 0x%08X 0x%08X 0x%08X",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id),
					result, rbuf_heap, baseaddr, virt);
					RBUF_DEBUG(("%s INITIALIZED:",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id)));

		RBUF_DEBUG(("rdaddr=0x%08X  wraddr=0x%08X  baseaddr=0x%08X (virt=0x%08X)",
						rbuf_cur_rdaddr, rbuf_cur_wraddr, baseaddr,  (uint32_t)virt));
		RBUF_DEBUG(("    endaddr=0x%08X (entries=%08X) freefull=0x%08X wrpoint=0x%08X",
						endaddr, entries, freefull, wrpoint));

		cap->rbuf_num_entries = entries;
		cap->rbuf_base_phys   = baseaddr;

		if ((BDSP_P_RbufType_eDecode == cap->type) || (BDSP_P_RbufType_eEncoder == cap->type) ||
			(BDSP_P_RbufType_ePassthru == cap->type) || (BDSP_P_RbufType_eTranscode == cap->type)) 
		{
			/* For decoder / passthru capture, use the start write address*/
			/* to see the capture start point*/
			cap->rbuf_prev_wraddr = wrpoint;
			cap->rbuf_prev_rdaddr = rbuf_cur_rdaddr;
		} 
		else 
		{
			/* For mixer capture, use the base address*/
			/* to see the capture start point*/
			cap->rbuf_prev_wraddr = baseaddr;
			cap->rbuf_prev_rdaddr = baseaddr;
		}
		cap->rbuf_base_virt_uncached = (uint8_t *)virt;
		cap->rbuf_initialized        = 1;
		cap->rbuf_started            = 1;
	} 
	else 
	{
		/* Already initialized, read the write / read address registers*/

		/* AUD_FMM_BF_CTRL_RINGBUF_X_WRADDR*/
		rbuf_cur_wraddr = (uint32_t)BREG_Read32(rbuf_reg_handle, cap->rbuf_reg_base+0x04);
		/* AUD_FMM_BF_CTRL_RINGBUF_X_RDADDR*/
		rbuf_cur_rdaddr = (uint32_t)BREG_Read32(rbuf_reg_handle, cap->rbuf_reg_base+0x00);
	}
	/* Extract the wrap bit*/
	wr_cur_wrap  = rbuf_cur_wraddr       & (1<<31);
	wr_prev_wrap = cap->rbuf_prev_wraddr & (1<<31);

	/* Mask the wrap bit*/
	wr_cur_addr  = rbuf_cur_wraddr       & (~(1<<31));
	wr_prev_addr = cap->rbuf_prev_wraddr & (~(1<<31));

	/* Extract the wrap bit*/
	rd_cur_wrap  = rbuf_cur_rdaddr       & (1<<31);
	rd_prev_wrap = cap->rbuf_prev_rdaddr & (1<<31);

	/* Mask the wrap bit*/
	rd_cur_addr  = rbuf_cur_rdaddr       & (~(1<<31));
	rd_prev_addr = cap->rbuf_prev_rdaddr & (~(1<<31));

	/* Verify the current WRADDR*/
	if (wr_cur_wrap != wr_prev_wrap) 
	{
		/* If wrapped, prev must be greater than cur*/
		/* else the RBUFs haven't been serviced in time*/
		RBUF_ALERT((wr_cur_addr < wr_prev_addr), "%s - 0x%08X 0x%08X 0x%08X",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id),
					wr_cur_addr, wr_prev_addr, cap->rbuf_count);
		RBUF_ALERT((1 == cap->rbuf_started), "%s",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id)); 
	} 
	else 
	{
		/* Wrap is unchanged, fall through and process the RBUF*/
		if (wr_cur_addr == wr_prev_addr) 
		{
			/* Nothing to do!*/
			if (RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE == cap->rbuf_to_cap_q_state) 
			{
				if (rbuf_cur_rdaddr == cap->rbuf_prev_rdaddr) 
				{
					/* RBUF is stopped and no more data has arrived*/
					/* Transition to stopped state*/
					cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
					RBUF_DEBUG(("%s - RBUF No longer being read",
								rbuf_capture_channel_get_info_string(cap->path,
								cap->channel,
								cap->type,
								cap->rbuf_id)));
				} 
				else 
				{
					cap->rbuf_prev_rdaddr = rbuf_cur_rdaddr;
				}
			}
			return;
		}
		/* Fall through and process the RBUF*/
		else { }
	}
	
	/* Verify the current RDADDR */
	if (rd_cur_wrap != rd_prev_wrap) 
	{
		/* Wrap changed*/
		RBUF_ALERT((rd_cur_addr < rd_prev_addr), "%s - 0x%08X 0x%08X",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id),
					rd_cur_addr, rd_prev_addr);
	} 
	else 
	{
		/* Wrap unchanged*/
		RBUF_ALERT((rd_cur_addr >= rd_prev_addr), "%s - 0x%08X 0x%08X",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id),
					rd_cur_addr, rd_prev_addr);
	}
	
	if (rd_cur_wrap != wr_cur_wrap) 
	{
		/* Read / Write wraps mismatch*/
		/* NOTE:: Can be equal, since wrap removes ambiguity of full/empty*/
		RBUF_ALERT((rd_cur_addr >= wr_cur_addr), "%s - 0x%08X 0x%08X 0x%08X 0x%08X",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id),
					rd_cur_addr, wr_cur_addr, rbuf_cur_rdaddr, rbuf_cur_wraddr);
	} 
	else 
	{
		/* Read / Write wraps match*/
		RBUF_ALERT((rd_cur_addr <= wr_cur_addr), "%s - 0x%08X 0x%08X",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id),
					rd_cur_addr, wr_cur_addr);
	}
	
	cap->rbuf_prev_rdaddr = rbuf_cur_rdaddr;

	/* Extract the RBUF information*/
	rbuf_base_virt   = cap->rbuf_base_virt_cached;

	rbuf_base_phys   = cap->rbuf_base_phys;
	rbuf_num_entries = cap->rbuf_num_entries;
	rbuf_w_idx       = wr_cur_addr  - rbuf_base_phys;
	rbuf_r_idx       = wr_prev_addr - rbuf_base_phys;
	
	RBUF_ALERT((((wr_cur_addr >= rbuf_base_phys) && (wr_cur_addr < (rbuf_base_phys+rbuf_num_entries))) &&
				((wr_prev_addr >= rbuf_base_phys) && (wr_prev_addr < (rbuf_base_phys+rbuf_num_entries)))),
				"%s - 0x%08X 0x%08X 0x%08X 0x%08X",
				rbuf_capture_channel_get_info_string(cap->path,
				cap->channel,
				cap->type,
				cap->rbuf_id),
				wr_cur_addr, wr_prev_addr, rbuf_base_phys, (rbuf_base_phys+rbuf_num_entries));

	/* It is more efficient to memcpy a block(s) from the RBUF*/
	/* to the capture Q and then update the associated pointers once.*/
	/* For this reason, calculate the active RBUF linear sections*/
	/* and capture Q empty linear sections and then memcpy as required.*/

	/* Get the available bytes from the write pointer to the end*/
	rbuf_copy_to_end_bytes = Q_ENTRIES_END(rbuf_r_idx, rbuf_w_idx, rbuf_num_entries);
	/* Get the available bytes from the start to the write pointer*/
	rbuf_copy_from_start_bytes = Q_ENTRIES_START(rbuf_r_idx, rbuf_w_idx, rbuf_num_entries);
	rbuf_copy_bytes = rbuf_copy_to_end_bytes + rbuf_copy_from_start_bytes;
	
	RBUF_ALERT(((rbuf_w_idx < rbuf_num_entries) && (rbuf_r_idx < rbuf_num_entries) &&
				(rbuf_copy_to_end_bytes <= rbuf_num_entries) && (rbuf_copy_from_start_bytes <= rbuf_num_entries)),
				"%s - 0x%08X 0x%08X 0x%08X 0x%08X",
				rbuf_capture_channel_get_info_string(cap->path,
				cap->channel,
				cap->type,
				cap->rbuf_id),
				rbuf_w_idx, rbuf_r_idx, rbuf_copy_to_end_bytes, rbuf_copy_from_start_bytes);

	/* Extract the capture Q information*/
	cap_q_ptr         = cap->q_ptr;
	cap_q_r_idx       = cap->q_r_idx;
	cap_q_w_idx       = cap->q_w_idx;
	cap_q_num_entries = cap->q_num_entries;

	/* Get the available bytes from the write pointer to the end*/
	cap_q_avail_to_end_bytes = Q_AVAIL_END(cap_q_r_idx, cap_q_w_idx, cap_q_num_entries);
	/* Get the available bytes from the start to the write pointer*/
	cap_q_avail_from_start_bytes = Q_AVAIL_START(cap_q_r_idx, cap_q_w_idx, cap_q_num_entries);
	cap_q_avail_total_bytes = cap_q_avail_to_end_bytes + cap_q_avail_from_start_bytes;

	/* There must be space for the entire RBUF active bytes to be copied*/
	/* or else the file output is NOT keeping up!*/
	if (rbuf_copy_bytes > cap_q_avail_total_bytes) 
	{
		RBUF_PRINT(("%s:", rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id)));
		
		RBUF_PRINT(("RBUF w=0x%08X r=0x%08X copy=0x%08X start=0x%08X end=0x%08X",
					rbuf_w_idx,
					rbuf_r_idx,
					rbuf_copy_bytes,
					rbuf_copy_from_start_bytes,
					rbuf_copy_to_end_bytes));
		RBUF_PRINT(("CAP  w=0x%08X r=0x%08X start=0x%08X end=0x%08X count=0x%08X",
					cap_q_w_idx,
					cap_q_r_idx,
					cap_q_avail_from_start_bytes,
					cap_q_avail_to_end_bytes,
					cap->q_count));

		return;
	}

	/* Process active portions of the RBUF*/
	if (rbuf_copy_to_end_bytes) 
	{
		rbuf_capture_rbuf_copy(cap, rbuf_base_virt, &rbuf_r_idx, rbuf_num_entries, rbuf_copy_to_end_bytes,
								cap_q_ptr, &cap_q_w_idx, cap_q_num_entries, &cap_q_avail_to_end_bytes, &cap_q_avail_from_start_bytes);
	}
	
	if (rbuf_copy_from_start_bytes) 
	{
		rbuf_capture_rbuf_copy(cap, rbuf_base_virt, &rbuf_r_idx, rbuf_num_entries, rbuf_copy_from_start_bytes,
								cap_q_ptr, &cap_q_w_idx, cap_q_num_entries, &cap_q_avail_to_end_bytes, &cap_q_avail_from_start_bytes);
	}
	/* Save the current RBUF WADDR pointer*/
	cap->rbuf_prev_wraddr = rbuf_cur_wraddr;
	cap->rbuf_count += rbuf_copy_bytes;

	/* Now update the capture Q write pointer*/
	cap->q_count += rbuf_copy_bytes;

	/* NOTE:: Since the Capture Q to Disk task uses the write index*/
	/*        to determine available data, this MUST be the final step*/
	/*        to guarantee coherency of all other fields.*/
	cap->q_w_idx = cap_q_w_idx;

	return;
alert:
	rbuf_capture_alert(cap);
	return;
}

static
void *
rbuf_capture_from_rbuf_to_cap_q_task(void *unused)
{
	struct timespec sleep_time;	
	uint32_t channel = 0;

	BSTD_UNUSED(unused);

	/* nanosleep setup (at least 1ms)*/
	sleep_time.tv_sec  = 0;
	sleep_time.tv_nsec = (1 * (1000 * 1000));

	/* Used to coordinate task shutdown*/
	rbuf_capture_from_rbuf_to_cap_q_active = 1;

	while (rbuf_capture_from_rbuf_to_cap_q_active) 
	{
		/* Yield*/
		nanosleep(&sleep_time, NULL);

		/* Process the RBUF for all active channel*/
		for (channel=0; channel<cap_channel_count; channel++) 
		{
			rbuf_capture_from_rbuf_to_cap_q(&rbuf_capture_channels[channel]);
		}
	}
	/* Signal this thread is terminated*/
	pthread_exit(NULL);
	return NULL;
}

static
void *
rbuf_capture_from_cap_q_to_disk_task(void *unused)
{
	uint32_t	cur_bytes = 0, channel = 0;
	struct		timespec sleep_time;

	BSTD_UNUSED(unused);

	/* nanosleep setup -- TWD better way to determine sleep time*/
	sleep_time.tv_sec  = 0;
	if(cap_channel_count <= 8)
	{
		sleep_time.tv_nsec = 25 * (1000 * 1000);
	}
	else
	{
		sleep_time.tv_nsec = 12 * (1000 * 1000);
	}

	/* Used to coordinate task shutdown*/
	rbuf_capture_from_cap_q_to_disk_active = 1;

	/* NOTE:: This loop does not do an explicit yield.  This task*/
	/*        has low priority and will be scheduled based on its*/
	/*        priority.  This approach is taken since there should*/
	/*        always be data available for output.*/

	printf ("cap_channel_count = %d\n", cap_channel_count);
	while (rbuf_capture_from_cap_q_to_disk_active) 
	{
		/* Process all active channels*/
		for (channel=0; channel<cap_channel_count; channel++) 
		{
			cur_bytes += rbuf_capture_from_cap_q_to_disk(&rbuf_capture_channels[channel]);
			nanosleep(&sleep_time, NULL);
		}
	}
	/* Signal this thread is terminated*/
	pthread_exit(NULL);
	return NULL;
}

static
void
rbuf_capture_channel_init(rbuf_capture_channel_t *cap)
{
	RBUF_ASSERT((NULL != cap), "0x%08X", cap);

	cap->active              = 0;
	cap->path                = RBUF_PATH_INVALID;
	cap->channel             = RBUF_CHANNEL_INVALID;
	cap->index               = RBUF_INDEX_INVALID;
	cap->rbuf_id             = RBUF_ID_INVALID;
	cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_INACTIVE;
	cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_INACTIVE;

	cap->rbuf_count              = 0;
	cap->rbuf_base_virt_uncached = NULL;
	cap->rbuf_base_virt_cached   = NULL;
	cap->rbuf_base_phys          = 0;
	cap->rbuf_prev_wraddr        = 0;
	cap->rbuf_prev_rdaddr        = 0;
	cap->rbuf_prev_endaddr       = 0;
	cap->rbuf_started            = 0;
	cap->rbuf_num_entries        = 0;
	cap->rbuf_reg_base           = 0;

	cap->q_count                 = 0;
	cap->q_ptr                   = NULL;
	cap->q_r_idx                 = 0;
	cap->q_w_idx                 = 0;
	cap->q_num_entries           = 0;

	cap->file_name[0]            = 0;
	cap->file                    = -1;
	return;
}

static
void
rbuf_capture_channel_uninit(rbuf_capture_channel_t *cap)
{
	int32_t  result = 0;

	if (!cap->active) 
	{
		/* Channel is not active, nothing to do*/
		return;
	}
	if (RBUF_CHANNEL_INVALID != cap->channel) 
	{
		/* Unititialize this capture channel*/
		RBUF_DEBUG(("%s - Uninit",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id)));
					RBUF_ASSERT((NULL != cap->q_ptr),
					"%s - 0x%08X",
					rbuf_capture_channel_get_info_string(cap->path,
					cap->channel,
					cap->type,
					cap->rbuf_id),
					cap->q_ptr);
		
		BKNI_Free(cap->q_ptr);

		result = rbuf_capture_device_close(cap->file);
		if(EBADF == errno) 
		{
			RBUF_DEBUG(("Capture file not created!"));
			return;
		}

		/* If the copy path has been initialized, attempt to copy the capture*/
		/* file to the supplied output path*/
		if (0 != rbuf_copy_path[0]) 
		{
			char cmd[2048];

			sprintf(cmd, "/bin/cp -f %s/%s %s/%s.tmp",
					(char *)rbuf_capture_path,
					(char*)cap->file_name,
					(char *)rbuf_copy_path,
					(char*)cap->file_name);
			
			RBUF_DEBUG(("'%s'", cmd));
			result = system(cmd);
			if(result != 0) 
			{
				RBUF_PRINT(("Copy captured file failed!"));
			}

			sprintf(cmd, "/bin/mv -f %s/%s.tmp %s/%s",
					(char *)rbuf_copy_path,
					(char*)cap->file_name,
					(char *)rbuf_copy_path,
					(char*)cap->file_name);
			
			RBUF_DEBUG(("'%s'", cmd));
			result = system(cmd);
			if(result != 0) 
			{
				RBUF_PRINT(("Rename captured file at copy path failed!"));
			}
	
			RBUF_DEBUG(("COPIED - '%s/%s' to '%s/%s'",
						(char *)rbuf_capture_path,
						(char*)cap->file_name,
						(char *)rbuf_copy_path,
						(char*)cap->file_name));
		}
	} 
	cap->file_name[0] = 0;
	cap->path         = RBUF_PATH_INVALID;
	cap->channel      = RBUF_CHANNEL_INVALID;

	return;
}

static
int32_t
rbuf_capture_channels_index_get_free(void)
{
	int32_t index;

	RBUF_ASSERT((NULL != rbuf_capture_channels), "0x%08X", rbuf_capture_channels);

	for (index=0; index<RBUF_CAP_MAX_CHAN; index++) 
	{
		if (!rbuf_capture_channels[index].active) 
		{
			return index;
		}
	}
	return RBUF_INDEX_INVALID;
}

static
int32_t
rbuf_capture_channels_index_find(uint32_t   path,
                                 uint32_t   channel,
								 uint32_t	rbuf_id,
                                 BDSP_P_RbufType type)
{
	int32_t index;

	RBUF_ASSERT((NULL != rbuf_capture_channels), "0x%08X", rbuf_capture_channels);
	
	if(rbuf_capture_app_player == 1) 
	{
		for (index=0; index<RBUF_CAP_MAX_CHAN; index++) 
		{
			if (rbuf_capture_channels[index].active) 
			{
				if ((rbuf_capture_channels[index].path    == path) &&
				  (rbuf_capture_channels[index].channel == channel) &&
				  (rbuf_capture_channels[index].rbuf_id == rbuf_id) &&
				  (rbuf_capture_channels[index].type    == type)) 
				{
					return index;
				}
			}
		 }
	}
	else 
	{
		for (index=0; index<RBUF_CAP_MAX_CHAN; index++) 
		{
			if (rbuf_capture_channels[index].active) 
			{
				if ((rbuf_capture_channels[index].path    == path) &&
				  (rbuf_capture_channels[index].channel == channel) &&
				  (rbuf_capture_channels[index].type    == type)) 
				{
					return index;
				}
			}
		}
	}
	return RBUF_INDEX_INVALID;
}

static
void
rbuf_capture_channels_init(void)
{
	uint32_t channel;

	rbuf_capture_channels = BKNI_Malloc(RBUF_CAP_MAX_CHAN * sizeof(*rbuf_capture_channels));
	RBUF_ASSERT((NULL != rbuf_capture_channels), "0x%08X", rbuf_capture_channels);

	for (channel=0; channel<RBUF_CAP_MAX_CHAN; channel++) 
	{
		rbuf_capture_channel_init(&rbuf_capture_channels[channel]);
	}

	return;
}

static
void
rbuf_capture_channels_uninit(void)
{
	uint32_t channel;
	
	/*uninitalize and free for all capture channels*/
	for (channel=0; channel<RBUF_CAP_MAX_CHAN; channel++) 
	{
		rbuf_capture_channel_uninit(&rbuf_capture_channels[channel]);
	}

	RBUF_ASSERT((NULL != rbuf_capture_channels), "0x%08X", rbuf_capture_channels);
	BKNI_Free(rbuf_capture_channels);
	return;
}

void
rbuf_capture_startup(void)
{
	int32_t            result;
	/*struct sched_param sc_cfg;*/
	pthread_attr_t     rbuf_thread_attr;

	rbuf_capture_channels_init();

	/*Create read & write threads with default priority */
	result = pthread_attr_init(&rbuf_thread_attr);
	RBUF_ASSERT((0 == result), "0x%08X", result); 
	result = pthread_create(&rbuf_capture_from_cap_q_to_disk_thread,
							&rbuf_thread_attr,
							rbuf_capture_from_cap_q_to_disk_task,
							NULL);
	RBUF_ASSERT((0 == result), "0x%08X", result);

	result = pthread_attr_init(&rbuf_thread_attr);
	RBUF_ASSERT((0 == result), "0x%08X", result); 
	result = pthread_create(&rbuf_capture_from_rbuf_to_cap_q_thread,
							&rbuf_thread_attr,
							rbuf_capture_from_rbuf_to_cap_q_task,
							NULL);
	RBUF_ASSERT((0 == result), "0x%08X", result);

	printf("RBUF Capture Initialized");
	rbuf_capture_initialized = 1;
	return;
}

static
void capture_filename(rbuf_capture_channel_t *cap)
{
	char     *postfix;
	uint32_t chan, rbuf_id;
	static char file_name[512];

	switch (cap->type) 
	{
		case BDSP_P_RbufType_eDecode: {
			static char *dec= "decoder";
			postfix = dec;
			chan = cap->channel;
			rbuf_id = cap->rbuf_id;
			break;
		}
		case BDSP_P_RbufType_ePassthru: {
			static char *pass= "passthru";
			postfix = pass;
			chan = cap->channel;
			rbuf_id = cap->rbuf_id;
			break;
		}
		case BDSP_P_RbufType_eMixer: {
			static char *mix="mixer";
			postfix = mix;
			chan = cap->channel;
			rbuf_id = cap->rbuf_id;
			break;
		}
		case BDSP_P_RbufType_eEncoder: {
			static char *enc="encoder";
			postfix = enc;
			chan = cap->channel;
			rbuf_id = cap->rbuf_id;
			break;
		}
		case BDSP_P_RbufType_eTranscode: {
			static char *tran="transcode";
			postfix = tran;
			chan = cap->channel;
			rbuf_id = cap->rbuf_id;
			break;
		}
		case BDSP_P_RbufType_eSM: {
			static char *spkr="spkr_mgmt";
			postfix = spkr;
			chan = cap->channel;
			rbuf_id = cap->rbuf_id;
			break;
		}
		default: {
			RBUF_ASSERT((0), "");
			postfix = NULL;
			chan = RBUF_CHANNEL_INVALID;
			rbuf_id = RBUF_ID_INVALID;
			return;
		}
	}

	if(rbuf_capture_multifile) 
	{
		sprintf((char *)cap->file_name, "%d_%s_path_%d_%s_%d.cap",
				capture_count,
				rbuf_capture_filename,
				cap->path,
				postfix,
				chan);
	}
	/*If both multifile and app_player is enabled use multifile*/
	else if(rbuf_capture_app_player && !rbuf_capture_multifile) 
	{
		/*Use rbuf_id for capture filename with app_player*/
		sprintf((char *)cap->file_name, "%s_path_%d_%s_%d_%d.cap",
				rbuf_capture_filename,
				cap->path,
				postfix,
				chan,
				rbuf_id);
	}
	else 
	{
		sprintf((char *)cap->file_name, "%s_path_%d_%s_%d.cap",
				rbuf_capture_filename,
				cap->path,
				postfix,
				chan);
	}
	sprintf((char *)file_name, "%s/%s",rbuf_capture_path,(char *)cap->file_name);
	cap->file = rbuf_capture_device_open((int8_t *)file_name);
	if(cap->file < 0) 
	{
		RBUF_PRINT(("Unable to open capture file %s",(char *)cap->file_name));
	}
	return;
}

static
void
rbuf_capture_ringbuf_start(rbuf_capture_channel_t *cap)
{
	uint32_t baseaddr;
	uint32_t endaddr;
	uint32_t entries;
	void     *virt_uncached;
	void     *virt_cached;
	int32_t  result;

	RBUF_DEBUG(("%s - Starting Ring Buffer ID %d FMM Base Address 0x%08X",
				rbuf_capture_channel_get_info_string(cap->path,
													cap->channel,
													cap->type,
													cap->rbuf_id),
													cap->rbuf_id,
													cap->rbuf_reg_base));

	/*Set all pointers to ZERO on start*/
	cap->rbuf_num_entries        = 0;
	cap->rbuf_base_phys          = 0;
	cap->rbuf_prev_rdaddr        = 0;
	cap->rbuf_prev_wraddr        = 0;
	cap->rbuf_prev_endaddr       = 0;
	cap->rbuf_started            = 0;
	cap->rbuf_base_virt_uncached = NULL;
	cap->rbuf_initialized        = 0;

	/* CACHED READS FROM RING BUFFER:*/
	/**/
	/*   Problem:*/
	/*   The MIPS receives access to DRAM essetially based on access count,*/
	/*   not bandwidth, in conjuntion with priority and round robin.  Therefore,*/
	/*   reading single 32bit quantities will result in one read per arbitration*/
	/*   schedule.  In a memory bound system, this will result in starvation*/
	/*   to the MIPS (it has many small accesses to perform and is high frequency).*/
	/*   Also, do to the nature of memory accesses, it is more efficient to */
	/*   burst blocks of contiguous data, rather incurring the setup/termination*/
	/*   overhead of individual accesses.*/
	/**/
	/*   Solution:*/
	/*   Cached accesses to the ring buffer shall be performed.  Using the*/
	/*   Linux cacheflush() system call, each ring buffer's address will be*/
	/*   cache flushed upon initialization.  From that point forward, when new*/
	/*   data is received at a given ring buffer, the address range corresponding*/
	/*   to that new data will be flushed.  Since no data will be written by*/
	/*   the MIPS to the ring buffer address, no data will be written to DRAM.*/
	/*   However, a side effect of cacheflush() is to also invalidate the*/
	/*   cached region.  This invalidate will cause a burst read from the memory*/
	/*   controller when the contents are read from ring buffer.*/

	/* AUD_FMM_BF_CTRL_RINGBUF_X_BASEADDR */
	baseaddr = (uint32_t)BREG_Read32(rbuf_reg_handle,cap->rbuf_reg_base+0x08);

	/* AUD_FMM_BF_CTRL_RINGBUF_X_ENDADDR */
	endaddr = (uint32_t)BREG_Read32(rbuf_reg_handle,cap->rbuf_reg_base+0x0C);
	
	if(endaddr < baseaddr)
	{
		/*if endaddr is less than baseaddr, alert and disable capture on that channel */
		rbuf_capture_alert(cap);
		return;
	}
	
	entries = (endaddr - baseaddr) + 1;
	
	/* Use the physical base address to compute the uncached virtual address*/
	result = BMEM_ConvertOffsetToAddress(rbuf_heap,baseaddr,&virt_uncached);
	RBUF_ASSERT((BERR_SUCCESS == result),"0x%08X",result);

	/* Use the uncached address to compute the cached address*/
	result = BMEM_ConvertAddressToCached(rbuf_heap,virt_uncached,&virt_cached);
	RBUF_ASSERT((BERR_SUCCESS == result),"0x%08X",result);

	RBUF_DEBUG(("%s - Phys:0x%08X Uncached:0x%08X Cached:0x%08X Entries:0x%08X",
				rbuf_capture_channel_get_info_string(cap->path,
				cap->channel,
				cap->type,
				cap->rbuf_id),
				baseaddr,
				(int32_t)virt_uncached,
				(int32_t)virt_cached,
				entries));
	
	/* Initial flush of this entire ring buffer.  It will be possible*/
	/* for the actual used number of bytes to be less than the total*/
	/* allocated size, but this flush will assume the max size to */
	/* put the cache into a known state (invalid).*/
	NEXUS_Memory_FlushCache(virt_cached, entries);
	cap->rbuf_base_virt_cached = (uint8_t *)virt_cached;

	/* Signal this channel's capture is ready*/
	cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_ACTIVE;
	cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_ACTIVE;

	RBUF_PRINT(("%s STARTED!",
				rbuf_capture_channel_get_info_string(cap->path,
				cap->channel,
				cap->type,
				cap->rbuf_id)));

	capture_filename(cap);
	return;
}

static
void
rbuf_capture_channel_start_common(uint32_t channel,
                                  uint32_t path,
                                  BDSP_P_RbufType type,
                                  uint32_t rbuf_id)
{
	rbuf_capture_channel_t *cap;
	int32_t  index;
	
	RBUF_PRINT(("%s START", rbuf_capture_channel_get_info_string(path, channel, type, rbuf_id)));
	
	if (!rbuf_capture_enabled) 
	{
		/*Capture not enabled, just return*/
		return;
	} 
	else if (!rbuf_capture_initialized) 
	{
		RBUF_PRINT(("CAPTURE NOT INITIALIZED, START IGNORED"));
		return;
	}
	
	if(rbuf_capture_app_player) 
	{
		index = rbuf_capture_channels_index_find(path, channel, rbuf_id, type);
	}
	else 
	{
		index = rbuf_capture_channels_index_find(path, channel, RBUF_INDEX_INVALID, type);
	}
	if (RBUF_INDEX_INVALID == index) 
	{
		RBUF_PRINT(("%s CAPTURE NOT CONFIGURED, IGNORING START!", rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type)));
		return;
	}

	cap = &rbuf_capture_channels[index];

	/* Compute the ring buffer ID*/
	RBUF_ASSERT((64 > rbuf_id), "%s", rbuf_capture_channel_get_info_string(path, channel, type, rbuf_id));
	cap->rbuf_id = rbuf_id;

	/* Compute the base address based on the ID*/
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
	cap->rbuf_reg_base = (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR +
						(cap->rbuf_id * (BCHP_AUD_FMM_BF_CTRL_RINGBUF_1_RDADDR -
						 BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)));
#else
    cap->rbuf_reg_base = (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR +
                        (cap->rbuf_id * (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_1_RDADDR -
                         BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR)));
#endif
	
	rbuf_capture_ringbuf_start(cap);
	return;
}
                                   
uint8_t 
rbuf_capture_channel_start_dummy(uint32_t channel,
                                uint32_t path,
                                uint32_t rbuf_id,
								BDSP_P_RbufType CP_TYPE)
{
	BSTD_UNUSED(channel);
	BSTD_UNUSED(path);
	BSTD_UNUSED(rbuf_id);
	BSTD_UNUSED(CP_TYPE);

	return 0;
}

uint8_t 
rbuf_capture_stop_channel_dummy(uint32_t channel,
                                uint32_t path,
                                uint32_t rbuf_id,
								BDSP_P_RbufType CP_TYPE)
{
	BSTD_UNUSED(channel);
	BSTD_UNUSED(path);
	BSTD_UNUSED(rbuf_id);
	BSTD_UNUSED(CP_TYPE);
    
	return 0;
}

uint8_t 
rbuf_capture_channel_start(uint32_t channel,
                                uint32_t path,
                                uint32_t rbuf_id,
								BDSP_P_RbufType CP_TYPE)
{
	if(rbuf_capture_enabled) {
		/*Counter for multifile capture*/
		if(cap_count_flag == true)
		{
			cap_count_flag = false;
			if(capture_count < 65536) 
			{ 
				capture_count +=1; 
			}
			else 
			{
				capture_count = 0;
			}
		}
		
		if(CP_TYPE == BDSP_P_RbufType_eDecode) {
			rbuf_capture_channel_start_common(channel,
											path,
											BDSP_P_RbufType_eDecode,
											rbuf_id);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_ePassthru) {
			rbuf_capture_channel_start_common(channel,
											path,
											BDSP_P_RbufType_ePassthru,
											rbuf_id);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_eMixer) {
			rbuf_capture_channel_start_common(channel,
											path,
											BDSP_P_RbufType_eMixer,
											rbuf_id);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_eEncoder) {
			rbuf_capture_channel_start_common(channel,
											path,
											BDSP_P_RbufType_eEncoder,
											rbuf_id);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_eTranscode) {
			rbuf_capture_channel_start_common(channel,
											path,
											BDSP_P_RbufType_eTranscode,
											rbuf_id);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_eSM) {
			rbuf_capture_channel_start_common(channel,
											path,
											BDSP_P_RbufType_eSM,
											rbuf_id);
			return 0;
		}
		else { 
			return 0;
		}
	}
	else 
	{
		return 0;
	}
}

static
void
rbuf_capture_channel_stop(rbuf_capture_channel_t *cap)
{
	/*Flag to enable/disable count for multifile capture*/
	if(cap_count_flag == false)
	{
		cap_count_flag = true;
	}

	if (((RBUF_CAPTURE_CHANNEL_STATE_INACTIVE == cap->rbuf_to_cap_q_state) &&
		(RBUF_CAPTURE_CHANNEL_STATE_INACTIVE == cap->cap_q_to_disk_state)) ||
		(RBUF_ID_INVALID == cap->rbuf_id)) 
	{
		/* Channel was never started, ignore stop*/
		RBUF_PRINT(("%s CAPTURE NOT CONFIGURED, IGNORING STOP!",
					rbuf_capture_channel_get_type_string(cap->path,
														cap->channel,
														cap->rbuf_id,
														cap->type)));
		cap->rbuf_started     = 0;
		cap->rbuf_initialized = 0;
		return;
	} 
	else 
	{
		struct timespec sleep_time;
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = 25 * 1000 * 1000;

	/*	cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE;*/
	/*	cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE;*/

		do 
		{
			/* Yield for read & write threads to complete the task*/
			nanosleep(&sleep_time, NULL);
			loop_break++;
			if(loop_break > RBUF_CAP_MAX_CHAN)
			{
				cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
				cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
			}
		} while ((RBUF_CAPTURE_CHANNEL_STATE_STOPPED != cap->rbuf_to_cap_q_state) ||
				(RBUF_CAPTURE_CHANNEL_STATE_STOPPED != cap->cap_q_to_disk_state));

		if(cap->rbuf_flush_onexit) 
		{
			if(cap->q_w_idx >= cap->q_r_idx) 
			{
				rbuf_capture_device_write(cap->file, &cap->q_ptr[cap->q_r_idx], (cap->q_w_idx - cap->q_r_idx));
				cap->q_r_idx = cap->q_w_idx;
			}
			else 
			{
				rbuf_capture_device_write(cap->file, &cap->q_ptr[cap->q_r_idx], (RBUF_CAP_MAX_CHAN_ENTRIES - cap->q_r_idx));
				rbuf_capture_device_write(cap->file, &cap->q_ptr[cap->q_r_idx], (cap->q_w_idx  - 0));
				cap->q_r_idx = cap->q_w_idx;
			}
			cap->rbuf_flush_onexit = false;
		}
	}
	RBUF_PRINT(("STOPPED!"));

	/* RBUF is no longer active*/
	cap->rbuf_started     = 0;
	cap->rbuf_initialized = 0;
	return;
}

static
void
rbuf_capture_channel_stop_common(uint32_t channel,
                                 uint32_t path,
								 uint32_t rbuf_id,
                                 BDSP_P_RbufType type)
{
	rbuf_capture_channel_t *cap;
	int32_t  index;

	

	RBUF_PRINT(("%s STOP",rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type)));

	if (!rbuf_capture_enabled) 
	{
		/*capture not enabled, just return*/
		return;
	} 
	else if (!rbuf_capture_initialized) 
	{
		/*Capture not initialized on this channel, ignore stop*/
		RBUF_PRINT(("CAPTURE NOT INITIALIZED, STOP IGNORED"));
		return;
	}
	
	index = rbuf_capture_channels_index_find(path, channel, rbuf_id, type);
	if (RBUF_INDEX_INVALID == index) 
	{
		RBUF_PRINT(("%s CAPTURE NOT CONFIGURED, IGNORING STOP!",rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type)));
		return;
	}
	cap = &rbuf_capture_channels[index];

	rbuf_capture_channel_stop(cap);

	return;
}

uint8_t 
rbuf_capture_stop_channel(uint32_t channel,							   
                                  uint32_t path,
								  uint32_t rbuf_id,
								  BDSP_P_RbufType CP_TYPE)
{
	if(rbuf_capture_enabled) 
	{
		if(CP_TYPE == BDSP_P_RbufType_eDecode) {
			rbuf_capture_channel_stop_common(channel,
											path,
											rbuf_id,
											BDSP_P_RbufType_eDecode);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_ePassthru) {
			rbuf_capture_channel_stop_common(channel,
											path,
											rbuf_id,
											BDSP_P_RbufType_ePassthru);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_eMixer) {
			rbuf_capture_channel_stop_common(channel,
											path,
											rbuf_id,
											BDSP_P_RbufType_eMixer);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_eEncoder) {
			rbuf_capture_channel_stop_common(channel,
											path,
											rbuf_id,
											BDSP_P_RbufType_eEncoder);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_eTranscode) {
			rbuf_capture_channel_stop_common(channel,
											path,
											rbuf_id,
											BDSP_P_RbufType_eTranscode);
			return 0;
		}
		else if(CP_TYPE == BDSP_P_RbufType_eSM) {
			rbuf_capture_channel_stop_common(channel,
											path,
											rbuf_id,
											BDSP_P_RbufType_eSM);
			return 0;
		}
		else { 
			return 0;
		}
	}
	else 
	{
		return 0;
	}
}

static
void
rbuf_capture_channel_add_cap(rbuf_capture_channel_t *cap)
{
	/* If this channel is being actively captured, must stop it first!*/
	if ((RBUF_CAPTURE_CHANNEL_STATE_ACTIVE == cap->rbuf_to_cap_q_state) ||
		(RBUF_CAPTURE_CHANNEL_STATE_ACTIVE == cap->cap_q_to_disk_state)) 
	{
		struct timespec sleep_time;

		cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_PEND_STOP;
		cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_PEND_STOP;
		/* Wait for capture tasks to indicate this channel is stopped*/
		do 
		{
			/* nanosleep setup (sleep the minimal amount of time)*/
			sleep_time.tv_sec = 0;
			sleep_time.tv_nsec = 1;
			/* Yield*/
			nanosleep(&sleep_time, NULL);
		} while ((RBUF_CAPTURE_CHANNEL_STATE_STOPPED != cap->rbuf_to_cap_q_state) ||
				(RBUF_CAPTURE_CHANNEL_STATE_STOPPED != cap->cap_q_to_disk_state));
	}

	cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_INACTIVE;
	cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_INACTIVE;

	cap->rbuf_count              = 0;
	cap->rbuf_base_virt_uncached = NULL;
	cap->rbuf_base_phys          = 0;
	cap->rbuf_num_entries        = 0;
	cap->rbuf_prev_wraddr        = 0;
	cap->rbuf_prev_rdaddr        = 0;
	cap->rbuf_prev_endaddr       = 0;
	cap->rbuf_started            = 0;

	if (NULL == cap->q_ptr) 
	{
		cap->q_count          = 0;
		cap->q_r_idx          = 0;
		cap->q_w_idx          = 0;
		cap->q_num_entries    = RBUF_CAP_MAX_CHAN_ENTRIES;
		
		/*Create a page aligned buffer for capture*/
		posix_memalign((void **)&cap->q_ptr, RBUF_PAGE_SIZE, cap->q_num_entries);
		RBUF_ASSERT((NULL != cap->q_ptr), "0x%08X", (sizeof(*cap->q_ptr) * cap->q_num_entries));

		/*switch (cap->type) 
		{
			case BDSP_P_RbufType_eDecode: {
			static char *dec= "decoder";
			break;
			}
			case BDSP_P_RbufType_ePassthru: {
			static char *pass= "passthru";
			break;
			}
			case BDSP_P_RbufType_eMixer: {
			static char *mix="mixer";
			break;
			}
			case BDSP_P_RbufType_eEncoder: {
			static char *enc="encoder";
			break;
			}
			case BDSP_P_RbufType_eTranscode: {
			static char *tran="transcode";
			break;
			}
			case BDSP_P_RbufType_eSM: {
			static char *spkr="spkr_mgmt";
			break;
			}
			default: {
			RBUF_ASSERT((0), "");
			break;
			}
		}*/
	}

	RBUF_PRINT(("ADDED!"));
	return;
}

static
void
rbuf_capture_channel_add(uint32_t path,
                         uint32_t channel,
						 uint32_t rbuf_id,
						 BDSP_P_RbufType type,
						 Rbuf_Callback_Info * cb_info)
{
	rbuf_capture_channel_t *cap;
	int32_t  index;

	RBUF_PRINT(("%s ADD", rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type)));
	
	if (!rbuf_capture_enabled) 
	{
		RBUF_PRINT(("CAPTURE DISABLED, ADD IGNORED"));
		return;
	} 
	else if (!rbuf_capture_initialized) 
	{
		RBUF_PRINT(("CAPTURE NOT INITIALIZED, ADD IGNORED"));
		return;
	}

	/* Get a free channel data structure index*/
	index = rbuf_capture_channels_index_get_free();
	
	RBUF_ASSERT((RBUF_INDEX_INVALID != index),
				"%s",
				rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type));
	
	cap = &rbuf_capture_channels[index];

	cap->index   = index;
	cap->path    = path;
	cap->channel = channel;
	cap->rbuf_id = rbuf_id;
	cap->type    = type;
	cap->active  = 1;
	cap->cb			= NULL;
	cap->cb_context	= NULL;

	/*Install the function pointer to send capture data*/
	if (cb_info != NULL)
	{
		RBUF_PRINT(("Installing callback capture function"));
		cap->cb			= cb_info->cb;
		cap->cb_context	= cb_info->cb_context;
	}
	rbuf_capture_channel_add_cap(cap);

	return;
}

static
int32_t
rbuf_capture_command_execute(RBUF_CAPTURE_COMMAND_t *cmd)
{
	RBUF_ASSERT((NULL != cmd), "0x%08X", cmd);

	/*If rbuf_id is not required set it to default: RBUF_ID_INVALID*/
	if(!rbuf_capture_app_player) {
	cmd->args[2] = RBUF_ID_INVALID;
	}

	switch (cmd->command) 
	{
		case RBUF_CAPTURE_COMMAND_INIT: {
		/*Start the read & write threads for capture*/
		rbuf_capture_startup();
		break;
		}
		case RBUF_CAPTURE_COMMAND_DECODER: {
		rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eDecode, NULL);
		cap_channel_count += 1;
		break;
		}
		case RBUF_CAPTURE_COMMAND_PASSTHRU: {
		rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_ePassthru, NULL);
		cap_channel_count += 1;
		break;
		}
		case RBUF_CAPTURE_COMMAND_MIXER: {
		rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eMixer, NULL);
		cap_channel_count += 1;
		break;
		}
		case RBUF_CAPTURE_COMMAND_ENCODER: {
		rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eEncoder, NULL);
		cap_channel_count += 1;
		break;
		}
		case RBUF_CAPTURE_COMMAND_TRANSCODE: {
		rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eTranscode, NULL);
		cap_channel_count += 1;
		break;
		}
		case RBUF_CAPTURE_COMMAND_SPKR_MGMT: {
		rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eSM, NULL);
		cap_channel_count += 1;
		break;
		}
		case RBUF_CAPTURE_COMMAND_ECHO: {
		RBUF_PRINT(("SOCKET ECHO RECEIVED"));
		break;
		}
		default: {
		RBUF_ASSERT(0, "0x%08X", 0);
		}
	}
	return 0;
}

static
int8_t *
rbuf_capture_skip_whitespace(int8_t **str)
{
	int8_t *p1;
	int8_t *p2;

	/* Find the first non whitespace character*/
	p1 = *str;
	while (1) 
	{
		if (0 == *p1) {
		return NULL;
		}
		if (isspace(*p1)) {
		p1++;
		} else {
		break;
		}
	}

	/* Now find the first trailing whitespace character*/
	p2 = p1;
	while ((0 != *p2) && (!isspace(*p2))) {
	p2++;
	}

	/* Save the first non-whitespace character address*/
	*str = p1;

	/* Return the first trailing whitespace character address*/
	return p2;
}

static
int32_t
rbuf_capture_command_parse(int8_t *str,
                           RBUF_CAPTURE_COMMAND_t *command)
{
	int8_t  *cmd;
	int32_t cmd_bytes;
	int8_t  *ptr;

	RBUF_ASSERT((NULL != command), "0x%08X", command);

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_INIT];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_INIT]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) 
	{
		command->command = RBUF_CAPTURE_COMMAND_INIT;
		command->args[0] = 0;
		return 0;
	}

	/*Get capture path from command configuration file*/
	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_CAPTURE_PATH];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_CAPTURE_PATH]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes))
	{
		int8_t *ptr;

		str += cmd_bytes;
		ptr = rbuf_capture_skip_whitespace(&str);
		if (NULL != ptr) 
		{
			uint32_t num_bytes;

			RBUF_ASSERT((ptr >= str), "0x%08X 0x%08X", ptr, str);
			num_bytes = (ptr-str);
			RBUF_ASSERT((num_bytes < sizeof(rbuf_capture_path)), "0x%08X 0x%08X", num_bytes, sizeof(rbuf_capture_path));
			strncpy((char *)rbuf_capture_path, (char *)str, num_bytes);
			rbuf_capture_path[num_bytes] = 0;
			RBUF_PRINT(("Capture Path '%s'", rbuf_capture_path));
			return 1;
		} 
		else 
		{
			return -1;
		}
	}

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_CAPTURE_FILENAME];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_CAPTURE_FILENAME]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) 
	{
		int8_t *ptr;

		str += cmd_bytes;
		ptr = rbuf_capture_skip_whitespace(&str);
		if (NULL != ptr) 
		{
			uint32_t num_bytes;

			RBUF_ASSERT((ptr >= str), "0x%08X 0x%08X", ptr, str);
			num_bytes = (ptr-str);
			RBUF_ASSERT((num_bytes < sizeof(rbuf_capture_filename)), "0x%08X 0x%08X", num_bytes, sizeof(rbuf_capture_filename));
			strncpy((char *)rbuf_capture_filename, (char *)str, num_bytes);
			rbuf_capture_filename[num_bytes] = 0;
			RBUF_PRINT(("Capture Filename '%s'", rbuf_capture_filename));
			return 1;
		} 
		else 
		{
			return -1;
		}
	}

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_COPY_PATH];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_COPY_PATH]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) 
	{
		int8_t *ptr;

		str += cmd_bytes;
		ptr = rbuf_capture_skip_whitespace(&str);
		if (NULL != ptr) 
		{
			uint32_t num_bytes;

			RBUF_ASSERT((ptr >= str), "0x%08X 0x%08X", ptr, str);
			num_bytes = (ptr-str);
			RBUF_ASSERT((num_bytes < sizeof(rbuf_copy_path)), "0x%08X 0x%08X", num_bytes, sizeof(rbuf_copy_path));
			strncpy((char *)rbuf_copy_path, (char *)str, num_bytes);
			rbuf_copy_path[num_bytes] = 0;
			RBUF_PRINT(("Copy Path '%s'", rbuf_copy_path));
			return 1;
		} 
		else 
		{
			return -1;
		}
	}

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_ECHO];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_ECHO]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) 
	{
		command->command = RBUF_CAPTURE_COMMAND_ECHO;
		return 0;
	}

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_DECODER];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_DECODER]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) 
	{
		command->command = RBUF_CAPTURE_COMMAND_DECODER;
		goto found;
	}

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_PASSTHRU];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_PASSTHRU]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) {
	command->command = RBUF_CAPTURE_COMMAND_PASSTHRU;
	goto found;
	}

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_MIXER];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_MIXER]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) 
	{
		command->command = RBUF_CAPTURE_COMMAND_MIXER;
		goto found;
	}

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_ENCODER];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_ENCODER]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) 
	{
		command->command = RBUF_CAPTURE_COMMAND_ENCODER;
		goto found;
	}

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_TRANSCODE];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_TRANSCODE]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) 
	{
		command->command = RBUF_CAPTURE_COMMAND_TRANSCODE;
		goto found;
	}

	cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_SPKR_MGMT];
	cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_SPKR_MGMT]);
	if (0 == strncmp((char *)cmd, (char *)str, cmd_bytes)) 
	{
		command->command = RBUF_CAPTURE_COMMAND_SPKR_MGMT;
		goto found;
	}

	/*// Unknown Command*/
	return -1;

found:
	str += cmd_bytes;
	ptr = rbuf_capture_skip_whitespace(&str);
	if (NULL == ptr) 
	{
		/* Malformed command*/
		return -1;
	}
	command->args[0] = (uint32_t)strtol((char *)str, NULL, 0);
	str = ptr;
	ptr = rbuf_capture_skip_whitespace(&str);
	if (NULL == ptr) 
	{
		/* Malformed command*/
		return -1;
	}
	command->args[1] = (uint32_t)strtol((char *)str, NULL, 0);
	if(rbuf_capture_app_player == 1) 
	{
		str = ptr;
		ptr = rbuf_capture_skip_whitespace(&str);
		if (NULL == ptr) 
		{
			/* Malformed command*/
			return -1;
		}
		command->args[2] = (uint32_t)strtol((char *)str, NULL, 0);
	}
	else 
	{
		command->args[2] = RBUF_ID_INVALID;
	}

	/* Successful command / arguments*/
	return 0;
}

static
int32_t
rbuf_capture_parse_buffer(RBUF_CAPTURE_COMMAND_t *cmd,
                                 uint32_t num_bytes)
{
	int32_t  result = 0;
	uint32_t cur = 0;

	RBUF_ASSERT((NULL != cmd), "0x%08X", cmd);

	/* Parse each byte of the input buffer*/
	for (cur=0; cur<num_bytes; cur++) 
	{
		/* Command start delimiter is '#'*/
		if ('#' == cmd->buffer[cur]) 
		{
			/* Clear the arguments*/
			memset(cmd->args, 0, sizeof(cmd->args));
			/* Invalidate command type*/
			cmd->command = RBUF_CAPTURE_COMMAND_NUM_COMMANDS;
			result = rbuf_capture_command_parse(&cmd->buffer[cur], cmd);
			if (result == 0) 
			{
				rbuf_capture_command_execute(cmd);
			} 
			else 
			{
				if (-1 == result) 
				RBUF_PRINT(("Received Unknown Command"));
			}
		}
	}
	return 0;
}

/*
* Supplies a mechanism for the target to load a text file containing
* capture commands.  This file is specified via the environment variable
* 'rbuf_capture_socket_target_commands_filename' where its value contains
* the full target specific path and filename.  The commands file is
* evaluated once at startup ONLY.
*/
int32_t
rbuf_capture_target_commands_process(void)
{
	char     *filename;
	FILE     *file;
	/*int32_t  result;
	int8_t   buf[512];*/
	RBUF_CAPTURE_COMMAND_t cap_cmd;
	uint32_t eof;
	uint32_t  bytes_read;
	static char *env="rbuf_capture_socket_target_commands_filename";

	/* Get the environment variable*/
	filename = getenv(env);
	if (NULL == filename) 
	{
		/* Variable is not defined*/
		RBUF_PRINT(("*   %s=NULL", env));
		return 0;
	}
	
	RBUF_PRINT(("*   %s='%s'", env, filename));

	/* Open the specified file*/
	file = fopen(filename, "rt");
	if (NULL == file) 
	{
		/* File could not be opened*/
		RBUF_PRINT(("* !!!! ERROR: TARGET commands file '%s'"
		" could NOT be opened, IGNORED!",
		filename));
		return 0;    
	}

	/* Read the input command file and process it*/
	eof = 0;
	while (!eof) 
	{
		bytes_read = fread(&cap_cmd.buffer[0], 1, sizeof(cap_cmd.buffer), file);
		if (sizeof(cap_cmd.buffer) != bytes_read) {
		eof = 1;
		}
		rbuf_capture_parse_buffer(&cap_cmd, bytes_read);
	}
	return 0;
}

uint8_t
rbuf_capture_init(void *reg_handle,
                  void *heap)
{
	const char *config_string;

	printf ("rbuf_init clled\n");
	/* Get environment variable settings*/
	config_string = getenv("rbuf_capture_enabled");
	rbuf_capture_enabled = (NULL != config_string) ?
	(uint32_t)strtol(config_string, NULL, 0) : 1;

	if (!rbuf_capture_enabled) 
	{
		/* Capture is not enabled, nothing else to do!*/
		rbuf_capture_debug_enabled  = 0;
		rbuf_capture_app_player		= 0;
		rbuf_capture_multifile		= 0;
		return 0;
	}
	else 
	{
		config_string = getenv("rbuf_capture_debug_enabled");
		rbuf_capture_debug_enabled = (NULL != config_string) ?
		(uint32_t)strtol(config_string, NULL, 0) : 0;

		config_string = getenv("rbuf_capture_multifile");
		rbuf_capture_multifile = (NULL != config_string) ?
		(uint32_t)strtol(config_string, NULL, 0) : 0;

		config_string = getenv("rbuf_capture_app_player");
		rbuf_capture_app_player = (NULL != config_string) ?
		(uint32_t)strtol(config_string, NULL, 0) : 0;

		/* Initialize the default path for captured files*/
		strcpy((char *)rbuf_capture_path, RBUF_CAPTURE_PATH_DEFAULT);
		/* Initialize the default filename for captured files*/
		strcpy((char *)rbuf_capture_filename, RBUF_CAPTURE_FILENAME_DEFAULT);
		/* Initialze the copy path to undefined (copy won't be done)*/
		rbuf_copy_path[0] = 0;

		RBUF_PRINT(("******************************************************************"));
		RBUF_PRINT(("*             *** RING BUFFER CAPTURE: VERSION 5.0 ***"));
		RBUF_PRINT(("*"));
		RBUF_DEBUG(("* Environment Variables:"));
		if(rbuf_capture_debug_enabled) 
		{
			RBUF_PRINT(("*   rbuf_capture_debug_enabled:    %d", rbuf_capture_debug_enabled));
		}
		if(rbuf_capture_multifile) 
		{
			RBUF_PRINT(("*   rbuf_capture_multifile:        %d", rbuf_capture_multifile));
		}
		if(rbuf_capture_app_player) 
		{
			RBUF_PRINT(("*   rbuf_capture_app_player:       %d", rbuf_capture_app_player));
		}
		RBUF_PRINT(("* Default Capture Path:            \"%s\"", rbuf_capture_path));
		RBUF_PRINT(("* Default Capture Filename:        \"%s\"", rbuf_capture_filename));
		RBUF_PRINT(("*"));
		RBUF_PRINT(("******************************************************************\n\n"));

		rbuf_reg_handle = reg_handle;
		rbuf_heap = heap;
		rbuf_capture_initialized = 0;

		rbuf_capture_target_commands_process();

		return 0;
	}
}

uint8_t 
rbuf_capture_uninit(void)
{
	if(rbuf_capture_enabled) 
	{
		int32_t  result;
		RBUF_PRINT(("Unintializing"));

		/* Remove the capture tasks*/
		if (rbuf_capture_initialized) 
		{	
			/* Stop read from ring buffer to capture buffer thread*/
			if (rbuf_capture_from_rbuf_to_cap_q_active) 
			{
				RBUF_PRINT(("Stopping Read thread..."));
				rbuf_capture_from_rbuf_to_cap_q_active= 0;
				result = pthread_join(rbuf_capture_from_rbuf_to_cap_q_thread, NULL);
				RBUF_ASSERT((0 == result), "0x%08X", result);
				RBUF_PRINT(("[Stopped]"));
			}
			
			/* Stop write from capture buffer to file thread*/
			if (rbuf_capture_from_cap_q_to_disk_active) 
			{
				RBUF_PRINT(("Stopping write thread..."));
				rbuf_capture_from_cap_q_to_disk_active = 0;
				result = pthread_join(rbuf_capture_from_cap_q_to_disk_thread, NULL);
				RBUF_ASSERT((0 == result), "0x%08X", result);
				RBUF_PRINT(("[Stopped]"));
			}
			
			/* Uninitalize capture channels and free memory*/
			rbuf_capture_channels_uninit();
		}

		RBUF_PRINT(("******************************************************************"));
		RBUF_PRINT(("*             *** RING BUFFER CAPTURE DONE ***"));
		RBUF_PRINT(("******************************************************************\n\n"));

		return 0;
	}
	else 
	{
		return 0;
	}
}

/* Intialize ring buffer capture interrupts if enabled*/
void brbuf_init_ringbuf_capture(void)
{
    if(local_ringBufCB.rbuf_init == NULL || local_ringBufCB.rbuf_uninit == NULL)
	{
		int i, j;
		int count = 0;
		char buf[20];
		char *str_ptr = NULL;
		
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 8; j++)
			{
				sprintf (&buf[0], "DECODER%1d%1d", i, j);
				str_ptr = getenv(&buf[0]);
				if (NULL != str_ptr)
					count++;
			}
		}
		
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 2; j++)
			{
				sprintf (&buf[0], "PASSTHRU%1d%1d", i, j);
				str_ptr = getenv(&buf[0]);
				if (NULL != str_ptr)
					count++;
			}
		}
		
		local_ringBufCB.rbuf_init = rbuf_capture_init;
		local_ringBufCB.rbuf_uninit = rbuf_capture_uninit;
		
		if (count == 0)
		{
			local_ringBufCB.rbuf_capture_channel_start = rbuf_capture_channel_start;
			local_ringBufCB.rbuf_capture_stop_channel = rbuf_capture_stop_channel;
		}
		else
		{
			local_ringBufCB.rbuf_capture_channel_start = rbuf_capture_channel_start_dummy;
			local_ringBufCB.rbuf_capture_stop_channel = rbuf_capture_stop_channel_dummy;
		}
		
		BDSP_P_RbufSetup(local_ringBufCB);
		printf ("RBUF_Setup Called\n");
	}
	
	return;
}

void start_rbuf_channels_from_env(void)
{
	/* Decoder Support only */
	int i, j, rbuf_id;
	char buf[20];
	char *str_ptr = NULL;
	int count = 0;
	
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 8; j++)
		{
			sprintf (&buf[0], "DECODER%1d%1d", i, j);
			str_ptr = getenv(&buf[0]);
			if (NULL != str_ptr)
			{
				sscanf (str_ptr, "%d", &rbuf_id);
				count++;
				/* Start All channels based on environment variables */
				/*printf ("Starting RBUF CAPTURE for DECODER [%d][%d] - rbuf id = %d\n\n", i, j, rbuf_id);*/
				rbuf_capture_channel_start(
											i,
											j,
											rbuf_id,
											BDSP_P_RbufType_eDecode);
			}
		}
	}
	
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			sprintf (&buf[0], "PASSTHRU%1d%1d", i, j);
			str_ptr = getenv(&buf[0]);
			if (NULL != str_ptr)
			{
				sscanf (str_ptr, "%d", &rbuf_id);
				count++;
				/*printf ("Starting RBUF CAPTURE for PASSTHRU [%d][%d] - rbuf id = %d\n\n", i, j, rbuf_id);*/
				/* Start All channels based on environment variables */
				rbuf_capture_channel_start(
											i,
											j,
											rbuf_id,
											BDSP_P_RbufType_ePassthru);
			}
		}
	}
	
	return;
}

/* Unintialize ring buffer capture interrupts if enabled*/
void brbuf_uninit_ringbuf_capture(void)
{
    rbuf_capture_uninit();
    if(local_ringBufCB.rbuf_init != NULL || local_ringBufCB.rbuf_uninit != NULL)
	{
		local_ringBufCB.rbuf_init = NULL;
		local_ringBufCB.rbuf_uninit = NULL;
		local_ringBufCB.rbuf_capture_channel_start = NULL;
		local_ringBufCB.rbuf_capture_stop_channel = NULL;
		BDSP_P_RbufSetup(local_ringBufCB);
	}
	return;
}

int register_ringbuf_capture_callback(Rbuf_Channel_Info * ch_info, Rbuf_Callback_Info * cb_info)
{
    printf("Registering call back\n");
	if (rbuf_capture_initialized == 0) 
	{
		rbuf_capture_startup();
	}
	rbuf_capture_channel_add(ch_info->path, ch_info->channel, ch_info->rbuf_id, ch_info->rb_type, cb_info);
	return 0;
}

#else

void brbuf_init_ringbuf_capture(void)
{
  return;
}

#endif /* BDSP_FW_RBUF_CAPTURE */
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return -1;
}
#endif
