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
#include "bdsp.h"
#include "bdsp_raaga_img.h"

BDBG_MODULE(bdsp_raaga_img);

/*
    This array will contain pointers to the arrays that contain that chunk
    pointers of the firmware binary. Based on the firmware ID one particular
    firmware Image can be accessed from this array
*/

static void *BDSP_IMG_P_GetArray(unsigned imgId)
{
    switch(imgId)
    {

    case BDSP_SystemImgId_eSystemKernelCode:                            return BDSP_IMG_system_kernel;
    case BDSP_SystemImgId_eSystemRdbvars:                               return BDSP_IMG_system_rdbvars;
    case BDSP_SystemImgId_eInitRomfsCode:                               return BDSP_IMG_init_romfs;
    case BDSP_SystemImgId_eSystemCode:                                  return BDSP_IMG_system_code;
    case BDSP_SystemImgId_eSystemLib:                               	return BDSP_IMG_system_lib;
#ifdef BDSP_MPEG_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eMpegAudioDecode):             return BDSP_IMG_adec_mpeg1;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eMpegAudioDecode):            return BDSP_IMG_adec_mpeg1_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eMpegAudioDecode):           return BDSP_IMG_adec_mpeg1_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eMpegAudioDecode):              return BDSP_IMG_aids_mpeg1;
#endif /* BDSP_MPEG_SUPPORT */
#ifdef BDSP_PCMWAV_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_ePcmWavDecode):                return BDSP_IMG_adec_pcmwav;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_ePcmWavDecode):               return BDSP_IMG_adec_pcmwav_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_ePcmWavDecode):              return BDSP_IMG_adec_pcmwav_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_ePcmWavDecode):                 return BDSP_IMG_aids_wavformatex;
#endif /* BDSP_PCMWAV_SUPPORT */
#ifdef BDSP_UDC_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eUdcDecode):					return BDSP_IMG_adec_ms11plus_udc;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eUdcDecode):					return BDSP_IMG_adec_ms11plus_udc_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eUdcDecode):					return BDSP_IMG_adec_ms11plus_udc_inter_frame;
#else
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eUdcDecode):					return BDSP_IMG_adec_udc;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eUdcDecode):					return BDSP_IMG_adec_udc_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eUdcDecode): 				return BDSP_IMG_adec_udc_inter_frame;
#endif
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eUdcDecode):					return BDSP_IMG_aids_ddp;
#endif /* BDSP_UDC_SUPPORT */
#ifdef BDSP_UDC_PASSTHRU_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eUdcPassthrough):				return BDSP_IMG_adec_passthru;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eUdcPassthrough): 			return BDSP_IMG_adec_passthru_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eUdcPassthrough):			return BDSP_IMG_adec_passthru_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eUdcPassthrough):				return BDSP_IMG_aids_ddp;
#endif /*BDSP_UDC_PASSTHRU_SUPPORT*/
#ifdef BDSP_AC3_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAc3Decode):					return BDSP_IMG_adec_ac3;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAc3Decode):					return BDSP_IMG_adec_ac3_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAc3Decode):					return BDSP_IMG_adec_ac3_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAc3Decode):					return BDSP_IMG_aids_ddp;
#endif /* BDSP_AC3_SUPPORT */
#ifdef BDSP_AC3_PASSTHRU_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAc3Passthrough):				return BDSP_IMG_adec_passthru;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAc3Passthrough): 			return BDSP_IMG_adec_passthru_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAc3Passthrough):			return BDSP_IMG_adec_passthru_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAc3Passthrough):				return BDSP_IMG_aids_ddp;
#endif /*BDSP_AC3_PASSTHRU_SUPPORT*/
#ifdef BDSP_DDP_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAc3Decode):					return BDSP_IMG_adec_ddp;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAc3Decode):					return BDSP_IMG_adec_ddp_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAc3Decode):					return BDSP_IMG_adec_ddp_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAc3Decode):					return BDSP_IMG_aids_ddp;
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAc3PlusDecode):				return BDSP_IMG_adec_ddp;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAc3PlusDecode):				return BDSP_IMG_adec_ddp_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAc3PlusDecode):				return BDSP_IMG_adec_ddp_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAc3PlusDecode):				return BDSP_IMG_aids_ddp;
#endif /* BDSP_DDP_SUPPORT */
#ifdef BDSP_DDP_PASSTHRU_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAc3PlusPassthrough):			return BDSP_IMG_adec_passthru;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAc3PlusPassthrough): 		return BDSP_IMG_adec_passthru_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAc3PlusPassthrough):		return BDSP_IMG_adec_passthru_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAc3PlusPassthrough):			return BDSP_IMG_aids_ddp;
#endif /*BDSP_DDP_PASSTHRU_SUPPORT*/
#ifdef BDSP_SRC_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eSrc):							return BDSP_IMG_app_src;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eSrc):						return BDSP_IMG_app_src_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eSrc): 						return BDSP_IMG_app_src_inter_frame;
#endif /* BDSP_SRC_SUPPORT */
#ifdef BDSP_MIXERDAPV2_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT/* Mixer without DAP */
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eMixerDapv2):                  return BDSP_IMG_app_ms11plus_mixer;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eMixerDapv2):                 return BDSP_IMG_app_ms11plus_mixer_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eMixerDapv2):                return BDSP_IMG_app_ms11plus_mixer_inter_frame;
#else/* Mixer with DAP */
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eMixerDapv2):					return BDSP_IMG_app_mixer_dapv2;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eMixerDapv2):					return BDSP_IMG_app_mixer_dapv2_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eMixerDapv2): 				return BDSP_IMG_app_mixer_dapv2_inter_frame;
#endif
#endif/*BDSP_MIXERDAPV2_SUPPORT*/
#ifdef BDSP_FWMIXER_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eMixer):					    return BDSP_IMG_app_fw_mixer;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eMixer):					    return BDSP_IMG_app_fw_mixer_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eMixer): 				    return BDSP_IMG_app_fw_mixer_inter_frame;
#endif /* BDSP_FWMIXER_SUPPORT */
#ifdef BDSP_AACSBR_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAacAdtsDecode):				return BDSP_IMG_adec_aache;
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAacLoasDecode):				return BDSP_IMG_adec_aache;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAacAdtsDecode):				return BDSP_IMG_adec_aache_tables;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAacLoasDecode):				return BDSP_IMG_adec_aache_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAacAdtsDecode):				return BDSP_IMG_adec_aache_inter_frame;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAacLoasDecode):				return BDSP_IMG_adec_aache_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAacAdtsDecode):				return BDSP_IMG_aids_adts;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAacLoasDecode):				return BDSP_IMG_aids_loas;
#endif /* BDSP_AACSBR_SUPPORT */
#ifdef BDSP_DOLBY_AACHE_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDolbyAacheAdtsDecode):		return BDSP_IMG_adec_ms11plus_dolby_aache;
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDolbyAacheLoasDecode):		return BDSP_IMG_adec_ms11plus_dolby_aache;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDolbyAacheAdtsDecode):		return BDSP_IMG_adec_ms11plus_dolby_aache_tables;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDolbyAacheLoasDecode):		return BDSP_IMG_adec_ms11plus_dolby_aache_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDolbyAacheAdtsDecode):		return BDSP_IMG_adec_ms11plus_dolby_aache_inter_frame;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDolbyAacheLoasDecode):		return BDSP_IMG_adec_ms11plus_dolby_aache_inter_frame;
#else
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDolbyAacheAdtsDecode):		return BDSP_IMG_adec_dolby_aache;
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDolbyAacheLoasDecode):		return BDSP_IMG_adec_dolby_aache;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDolbyAacheAdtsDecode):		return BDSP_IMG_adec_dolby_aache_tables;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDolbyAacheLoasDecode):		return BDSP_IMG_adec_dolby_aache_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDolbyAacheAdtsDecode):		return BDSP_IMG_adec_dolby_aache_inter_frame;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDolbyAacheLoasDecode):		return BDSP_IMG_adec_dolby_aache_inter_frame;
#endif
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eDolbyAacheAdtsDecode):			return BDSP_IMG_aids_adts;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eDolbyAacheLoasDecode):			return BDSP_IMG_aids_loas;
#endif /* BDSP_DOLBY_AACSBR_SUPPORT */

#ifdef BDSP_AACSBR_PASSTHRU_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAacLoasPassthrough):			return BDSP_IMG_adec_passthru;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAacLoasPassthrough): 		return BDSP_IMG_adec_passthru_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAacLoasPassthrough):		return BDSP_IMG_adec_passthru_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAacLoasPassthrough):			return BDSP_IMG_aids_loas;
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAacAdtsPassthrough):			return BDSP_IMG_adec_passthru;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAacAdtsPassthrough): 		return BDSP_IMG_adec_passthru_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAacAdtsPassthrough):		return BDSP_IMG_adec_passthru_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAacAdtsPassthrough):			return BDSP_IMG_aids_adts;
#endif /*BDSP_AACSBR_PASSTHRU_SUPPORT*/
#ifdef BDSP_DPCMR_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDpcmr):						return BDSP_IMG_app_ms11plus_dpcmr;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDpcmr):						return BDSP_IMG_app_ms11plus_dpcmr_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDpcmr): 					return BDSP_IMG_app_ms11plus_dpcmr_inter_frame;
#else
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDpcmr):						return BDSP_IMG_app_dpcmr;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDpcmr):						return BDSP_IMG_app_dpcmr_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDpcmr): 					return BDSP_IMG_app_dpcmr_inter_frame;
#endif
#endif /* BDSP_DPCMR_SUPPORT */
#ifdef BDSP_DDPENC_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDDPEncode):					return BDSP_IMG_aenc_ms11plus_dd;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDDPEncode):					return BDSP_IMG_aenc_ms11plus_dd_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDDPEncode): 				return BDSP_IMG_aenc_ms11plus_dd_inter_frame;
#else
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDDPEncode):					return BDSP_IMG_aenc_ddp;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDDPEncode):					return BDSP_IMG_aenc_ddp_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDDPEncode): 				return BDSP_IMG_aenc_ddp_inter_frame;
#endif
#endif /* BDSP_DDPENC_SUPPORT */
#ifdef BDSP_AACHEENC_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAacEncode):					return BDSP_IMG_aenc_aache;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAacEncode):					return BDSP_IMG_aenc_aache_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAacEncode): 				return BDSP_IMG_aenc_aache_inter_frame;
#endif /* BDSP_AACHEENC_SUPPORT */
#ifdef BDSP_GENCDBITB_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eGenCdbItb):					return BDSP_IMG_app_gen_cdbitb;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eGenCdbItb):					return BDSP_IMG_app_gen_cdbitb_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eGenCdbItb): 				return BDSP_IMG_app_gen_cdbitb_inter_frame;
#endif /* BDSP_GENCDBITB_SUPPORT */
#ifdef BDSP_DSOLA_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDsola):                       return BDSP_IMG_app_dsola;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDsola):                      return BDSP_IMG_app_dsola_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDsola):                     return BDSP_IMG_app_dsola_inter_frame;
#endif /*BDSP_DSOLA_SUPPORT*/
#ifdef BDSP_OPUSDEC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eOpusDecode):                  return BDSP_IMG_adec_opus;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eOpusDecode):                 return BDSP_IMG_adec_opus_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eOpusDecode):                return BDSP_IMG_adec_opus_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eOpusDecode):                   return BDSP_IMG_aids_wavformatex;
#endif /* BDSP_OPUSDEC_SUPPORT */
#ifdef BDSP_ALS_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eALSDecode):                  return BDSP_IMG_adec_als;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eALSDecode):                 return BDSP_IMG_adec_als_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eALSDecode):                return BDSP_IMG_adec_als_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eALSDecode):                   return BDSP_IMG_aids_wavformatex;
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eALSLoasDecode):              return BDSP_IMG_adec_als;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eALSLoasDecode):             return BDSP_IMG_adec_als_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eALSLoasDecode):            return BDSP_IMG_adec_als_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eALSLoasDecode):               return BDSP_IMG_aids_loas;
#endif /* BDSP_ALS_SUPPORT */

#ifdef BDSP_FLACDEC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eFlacDecode):                  return BDSP_IMG_adec_flac;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eFlacDecode):                 return BDSP_IMG_adec_flac_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eFlacDecode):                return BDSP_IMG_adec_flac_inter_frame;
    case BDSP_IMG_ID_IDS(BDSP_Algorithm_eFlacDecode):                   return BDSP_IMG_aids_wavformatex;
#endif /* BDSP_FLACDEC_SUPPORT */
#ifdef BDSP_LPCMENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eLpcmEncode):                       return BDSP_IMG_aenc_lpcm;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eLpcmEncode):                      return BDSP_IMG_aenc_lpcm_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eLpcmEncode):                     return BDSP_IMG_aenc_lpcm_inter_frame;
#endif /*BDSP_LPCMENC_SUPPORT*/
#ifdef BDSP_MP3ENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_Algorithm_eMpegAudioEncode):                       return BDSP_IMG_aenc_mp3;
    case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eMpegAudioEncode):                      return BDSP_IMG_aenc_mp3_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eMpegAudioEncode):                     return BDSP_IMG_aenc_mp3_inter_frame;
#endif /*BDSP_MP3ENC_SUPPORT*/
#ifdef BDSP_AC4_SUPPORT
		case BDSP_IMG_ID_CODE(BDSP_Algorithm_eAC4Decode): 				return BDSP_IMG_adec_ac4;
		case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eAC4Decode):				return BDSP_IMG_adec_ac4_tables;
		case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eAC4Decode):				return BDSP_IMG_adec_ac4_inter_frame;
		case BDSP_IMG_ID_IDS(BDSP_Algorithm_eAC4Decode):				return BDSP_IMG_aids_ac4;
#endif /* BDSP_AC4_SUPPORT */
#ifdef BDSP_LPCMDVD_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eLpcmDvdDecode): 				return BDSP_IMG_adec_lpcm;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eLpcmDvdDecode):				return BDSP_IMG_adec_lpcm_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eLpcmDvdDecode):				return BDSP_IMG_adec_lpcm_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eLpcmDvdDecode):				return BDSP_IMG_aids_lpcm;
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eLpcm1394Decode): 				return BDSP_IMG_adec_lpcm;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eLpcm1394Decode):				return BDSP_IMG_adec_lpcm_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eLpcm1394Decode):			return BDSP_IMG_adec_lpcm_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eLpcm1394Decode):				return BDSP_IMG_aids_lpcm;
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eLpcmBdDecode): 				return BDSP_IMG_adec_lpcm;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eLpcmBdDecode):				return BDSP_IMG_adec_lpcm_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eLpcmBdDecode):				return BDSP_IMG_adec_lpcm_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eLpcmBdDecode):					return BDSP_IMG_aids_lpcm;
#endif /* BDSP_LPCMDVD_SUPPORT */
#ifdef BDSP_MPEG_PASSTHRU_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eMpegAudioPassthrough):				return BDSP_IMG_adec_passthru;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eMpegAudioPassthrough): 			return BDSP_IMG_adec_passthru_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eMpegAudioPassthrough):			return BDSP_IMG_adec_passthru_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eMpegAudioPassthrough):				return BDSP_IMG_aids_mpeg1;
#endif /*BDSP_MPEG_PASSTHRU_SUPPORT*/
#ifdef BDSP_WMAPRO_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eWmaProDecode): 				return BDSP_IMG_adec_wma_pro;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eWmaProDecode):				return BDSP_IMG_adec_wma_pro_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eWmaProDecode):				return BDSP_IMG_adec_wma_pro_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eWmaProDecode):				return BDSP_IMG_aids_wma_pro;
#endif /* BDSP_WMAPRO_SUPPORT */
#ifdef BDSP_WMA_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eWmaStdDecode):				return BDSP_IMG_adec_wma;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eWmaStdDecode):				return BDSP_IMG_adec_wma_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eWmaStdDecode):				return BDSP_IMG_adec_wma_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eWmaStdDecode): 			    return BDSP_IMG_aids_wma;
#endif /* BDSP_WMA_SUPPORT */
#ifdef BDSP_OUTPUTFORMATTER_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eOutputFormatter):				return BDSP_IMG_app_outputformatter;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eOutputFormatter):			return BDSP_IMG_app_outputformatter_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eOutputFormatter):			return BDSP_IMG_app_outputformatter_inter_frame;
#endif
#ifdef BDSP_DTSHD_PASSTHRU_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDtsHdPassthrough):				return BDSP_IMG_adec_passthru;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDtsHdPassthrough): 			return BDSP_IMG_adec_passthru_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDtsHdPassthrough):			return BDSP_IMG_adec_passthru_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eDtsHdPassthrough):				return BDSP_IMG_aids_dtshd;
#endif /*BDSP_DTSHD_PASSTHRU_SUPPORT*/
#ifdef BDSP_DTS_PASSTHRU_SUPPORT
	case BDSP_IMG_ID_CODE(BDSP_Algorithm_eDts14BitPassthrough):				return BDSP_IMG_adec_passthru;
	case BDSP_IMG_ID_TABLE(BDSP_Algorithm_eDts14BitPassthrough): 			return BDSP_IMG_adec_passthru_tables;
	case BDSP_IMG_ID_IFRAME(BDSP_Algorithm_eDts14BitPassthrough):			return BDSP_IMG_adec_passthru_inter_frame;
	case BDSP_IMG_ID_IDS(BDSP_Algorithm_eDts14BitPassthrough):				return BDSP_IMG_aids_dts;
#endif /*BDSP_DTS_PASSTHRU_SUPPORT*/
    default:
        BDBG_WRN(("IMG %u not supported (algo %u)", imgId, BDSP_IMG_ID_TO_ALGO(imgId)));
        return NULL;
    }
}

/*  This context is used by other modules to make use of this interface. They
    need to supply this as a parameter to BDSP_IMG_Open */
static void *pDummy = NULL;
void *BDSP_IMG_Context = &pDummy;


/* This function has to be used for opening a firmware image. The pointer to the
    firmware image array is given that contains the header and the chunks.
    This works based on the firmware image id  that is supplied.
*/

static BERR_Code BDSP_IMG_Open(void *context, void **image, unsigned image_id)
{
    BSTD_UNUSED(context);
    BDBG_ASSERT(NULL != context);

    *image = BDSP_IMG_P_GetArray(image_id);

    if (NULL == *image)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BERR_SUCCESS;
}

/*  After opening the firmware image, the user of this interface will then be
    interested in getting the chunks and the header giving information about the
    chunks.
*/

static BERR_Code BDSP_IMG_Next(void *image, unsigned chunk, const void **data, uint16_t length)
{

    BDBG_ASSERT(data);
    BSTD_UNUSED(length);

    *data = ((const void **)image)[chunk];

    if (NULL == *data)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BERR_SUCCESS;
}

/* This function is used to close the firmware image that was opened using
    BDSP_IMG_Open */
static void BDSP_IMG_Close(void *image)
{

    BSTD_UNUSED(image);
    return;
}

/* The interface is actually a set of three functions open, next and close.
    The same has been implemented here and their function pointers supplied */
const BIMG_Interface BDSP_IMG_Interface = {
    BDSP_IMG_Open,
    BDSP_IMG_Next,
    BDSP_IMG_Close
    };
