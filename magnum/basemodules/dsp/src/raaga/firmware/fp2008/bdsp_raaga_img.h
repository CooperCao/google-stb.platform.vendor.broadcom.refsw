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
 *****************************************************************************/

#ifndef BDSP_RAAGA_IMG_H_
#define BDSP_RAAGA_IMG_H_

#include "bimg.h"

#define BDSP_IMG_ID_BASE(algId)   (BDSP_SystemImgId_eMax+((algId)*3))
#define BDSP_IMG_ID_CODE(algId)   (BDSP_IMG_ID_BASE(algId)+0)
#define BDSP_IMG_ID_IFRAME(algId) (BDSP_IMG_ID_BASE(algId)+1)
#define BDSP_IMG_ID_TABLE(algId)  (BDSP_IMG_ID_BASE(algId)+2)
#define BDSP_IMG_ID_MAX           (BDSP_IMG_ID_BASE(BDSP_AF_P_AlgoId_eMax))

#define BDSP_IMG_ID_TO_ALGO(imgId) (((imgId)<BDSP_SystemImgId_eMax)?(BDSP_AF_P_AlgoId_eMax):(((imgId)-BDSP_SystemImgId_eMax)/3))

/* This chunk size will be used when the firmware binary is actually present in
    multiple chunks. The BDSP_IMG_CHUNK_SIZE will then give the size of each
    such chunk
*/
#define BDSP_IMG_CHUNK_SIZE 65532


extern void *BDSP_IMG_Context;
extern const BIMG_Interface BDSP_IMG_Interface;

extern const void * BDSP_IMG_system_code [];
extern const void * BDSP_IMG_system_rdbvars [];
extern const void * BDSP_IMG_algolib_code[];
extern const void * BDSP_IMG_syslib_code [];
extern const void * BDSP_IMG_idscommon_code [];
extern const void * BDSP_IMG_vididscommon_code[];
extern const void * BDSP_IMG_vididscommon_inter_frame[];
extern const void * BDSP_IMG_scm_task_code [];
extern const void * BDSP_IMG_video_decode_task_code [];
extern const void * BDSP_IMG_video_encode_task_code [];
extern const void * BDSP_IMG_cdb_passthru_code[];
extern const void * BDSP_IMG_cdb_passthru_tables[];
extern const void * BDSP_IMG_cdb_passthru_inter_frame[];

/* Passthru AC3/MPEG/DDP/DTS/AAC even without license */
#ifdef  BDSP_AC3_PASSTHRU_SUPPORT
extern const void * BDSP_IMG_ac3_ids [];
extern const void * BDSP_IMG_ac3_ids_inter_frame [];
#endif

#ifdef  BDSP_MPEG_PASSTHRU_SUPPORT
extern const void * BDSP_IMG_mpeg1_ids [];
extern const void * BDSP_IMG_mpeg1_ids_inter_frame [];
#endif

#ifdef  BDSP_AACSBR_PASSTHRU_SUPPORT
extern const void * BDSP_IMG_aacheadts_ids [];
extern const void * BDSP_IMG_aacheloas_ids [];
extern const void * BDSP_IMG_aacheadts_ids_inter_frame [];
extern const void * BDSP_IMG_aacheloas_ids_inter_frame [];
#endif

#ifdef  BDSP_DDP_PASSTHRU_SUPPORT
extern const void * BDSP_IMG_ddp_ids [];
extern const void * BDSP_IMG_ddp_ids_inter_frame [];
#endif

#ifdef  BDSP_DTSBROADCAST_PASSTHRU_SUPPORT
extern const void * BDSP_IMG_dts_ids [];
extern const void * BDSP_IMG_dts_ids_inter_frame [];
#endif

#ifdef  BDSP_DTSHD_PASSTHRU_SUPPORT
extern const void * BDSP_IMG_dtshd_ids [];
extern const void * BDSP_IMG_dtshd_ids_inter_frame [];
#endif

#ifdef  BDSP_DRA_PASSTHRU_SUPPORT
extern const void * BDSP_IMG_dra_ids[];
extern const void * BDSP_IMG_dra_ids_inter_frame[];
#endif

#ifdef  BDSP_REALAUDIOLBR_SUPPORT
extern const void * BDSP_IMG_ralbr_ids [];
extern const void * BDSP_IMG_ralbr_ids_inter_frame [];
#endif

#ifdef BDSP_UDC_PASSTHRU_SUPPORT
extern const void * BDSP_IMG_udc_ids [];
extern const void * BDSP_IMG_udc_ids_inter_frame [];
#endif

#ifdef BDSP_MPEG_SUPPORT
extern const void * BDSP_IMG_mpeg1_decode [];
extern const void * BDSP_IMG_mpeg1_decode_tables [];
extern const void * BDSP_IMG_mpeg1_decode_inter_frame [];
#endif

#ifdef BDSP_AAC_SUPPORT
#endif

#ifdef BDSP_DOLBY_AACHE_SUPPORT
extern const void * BDSP_IMG_dolby_aache_decode [];
extern const void * BDSP_IMG_dolby_aache_decode_tables [];
extern const void * BDSP_IMG_dolby_aache_decode_inter_frame [];
#elif BDSP_MS10_SUPPORT
extern const void * BDSP_IMG_dolby_pulse_decode [];
extern const void * BDSP_IMG_dolby_pulse_decode_tables [];
extern const void * BDSP_IMG_dolby_pulse_decode_inter_frame [];
#elif  BDSP_AACSBR_SUPPORT
extern const void * BDSP_IMG_aache_decode [];
extern const void * BDSP_IMG_aache_decode_tables [];
extern const void * BDSP_IMG_aache_decode_inter_frame [];
#endif

#ifdef BDSP_MS10_SUPPORT
extern const void * BDSP_IMG_dolby_ms_ddp_decode [];
extern const void * BDSP_IMG_dolby_ms_ddp_decode_tables [];
extern const void * BDSP_IMG_dolby_ms_ddp_decode_inter_frame [];
#elif  BDSP_DDP_SUPPORT
extern const void * BDSP_IMG_ddp_decode [];
extern const void * BDSP_IMG_ddp_decode_tables [];
extern const void * BDSP_IMG_ddp_decode_inter_frame [];
#elif  BDSP_AC3_SUPPORT
extern const void * BDSP_IMG_ac3_decode [];
extern const void * BDSP_IMG_ac3_decode_tables [];
extern const void * BDSP_IMG_ac3_decode_inter_frame [];
#endif


#ifdef BDSP_DTS_SUPPORT
extern const void * BDSP_IMG_dts_ids [];
extern const void * BDSP_IMG_dts_ids_inter_frame [];
#endif

#if defined(BDSP_DTSBROADCAST_SUPPORT) || defined(BDSP_DTSHD_SUPPORT)
extern const void * BDSP_IMG_dtshd_decode [];
extern const void * BDSP_IMG_dtshd_decode_tables [];
extern const void * BDSP_IMG_dtshd_decode_inter_frame [];
#endif

#ifdef BDSP_LPCMBD_SUPPORT
#endif

#ifdef BDSP_LPCMHDDVD_SUPPORT
#endif

#ifdef BDSP_LPCMDVD_SUPPORT
extern const void *   BDSP_IMG_dvdlpcm_ids [];
extern const void *   BDSP_IMG_lpcm_decode_code [];
extern const void *   BDSP_IMG_lpcm_decode_tables [];
extern const void *   BDSP_IMG_lpcm_decode_inter_frame [];
extern const void *   BDSP_IMG_dvdlpcm_ids_inter_frame [];
#endif

#ifdef BDSP_WMA_SUPPORT
extern const void *  BDSP_IMG_wma_ids [];
extern const void *  BDSP_IMG_wma_decode [];
extern const void *  BDSP_IMG_wma_decode_tables [];
extern const void *  BDSP_IMG_wma_decode_inter_frame [];
extern const void *  BDSP_IMG_wma_ids_inter_frame [];
#endif

#ifdef BDSP_AC3LOSSLESS_SUPPORT
#endif

#if defined (BDSP_MLP_SUPPORT) || defined (BDSP_MLP_PASSTHROUGH_SUPPORT)
extern const void *  BDSP_IMG_mlp_ids [];
extern const void *  BDSP_IMG_mlp_ids_inter_frame [];
#endif

#ifdef BDSP_MLP_SUPPORT
extern const void *  BDSP_IMG_truehd_decode [];
extern const void *  BDSP_IMG_truehd_decode_tables [];
extern const void *  BDSP_IMG_truehd_decode_inter_frame [];
#endif

#ifdef BDSP_PCM_SUPPORT
extern const void *  BDSP_IMG_pcm_ids [];
extern const void *  BDSP_IMG_pcm_ids_inter_frame [];
#endif

#ifdef BDSP_DTSLBR_SUPPORT
extern const void *        BDSP_IMG_dts_express_ids_decode [];
extern const void *        BDSP_IMG_dts_express_decode [];
extern const void *        BDSP_IMG_dts_express_decode_tables [];
extern const void *        BDSP_IMG_dts_express_decode_inter_frame [];
extern const void *        BDSP_IMG_dts_express_ids_inter_frame[];
#endif

#ifdef BDSP_DDP7_1_SUPPORT
#endif

#ifdef BDSP_MPEGMC_SUPPORT
#endif

#ifdef BDSP_WMAPRO_SUPPORT
extern const void *  BDSP_IMG_wma_pro_ids [];
extern const void *  BDSP_IMG_wma_pro_decode [];
extern const void *  BDSP_IMG_wma_pro_decode_tables [];
extern const void *  BDSP_IMG_wma_pro_ids_inter_frame [];
extern const void *  BDSP_IMG_wma_pro_decode_inter_frame [];
#endif

#ifdef BDSP_DDP_TO_AC3_SUPPORT
extern const void * BDSP_IMG_ddp_convert [];
extern const void * BDSP_IMG_ddp_convert_tables [];
extern const void * BDSP_IMG_ddp_convert_inter_frame [];
#endif

#ifdef BDSP_DDBM_SUPPORT
extern const void * BDSP_IMG_ddbm_postprocess[];
extern const void * BDSP_IMG_ddbm_postprocess_table[];
#endif

#ifdef BDSP_DTSNEO_SUPPORT
extern const void * BDSP_IMG_dts_neo_postprocess[];
extern const void * BDSP_IMG_dts_neo_postprocess_table[];
extern const void * BDSP_IMG_dts_neo_interframe_buf[];
#endif

#ifdef BDSP_AVL_SUPPORT
extern const void *BDSP_IMG_brcm_avl_code[];
extern const void *BDSP_IMG_brcm_avl_tables[];
extern const void *BDSP_IMG_brcm_avl_inter_frame[];
#endif

#ifdef BDSP_PL2_SUPPORT
extern const void *BDSP_IMG_prologic2_code[];
extern const void *BDSP_IMG_prologic2_tables[];
extern const void *BDSP_IMG_prologic2_inter_frame[];
#endif

#ifdef BDSP_SRSXT_SUPPORT
extern const void *BDSP_IMG_trusurroundxt[];
extern const void *BDSP_IMG_trusurroundxt_tables[];
extern const void *BDSP_IMG_trusurroundxt_inter_frame[];
#endif

#ifdef BDSP_SRSHD_SUPPORT
extern const void *BDSP_IMG_srs_trusurroundhd_code [];
extern const void *BDSP_IMG_srs_trusurroundhd_tables [];
extern const void *BDSP_IMG_srs_trusurroundhd_inter_frame [];
#endif

#ifdef BDSP_SRSTRUVOL_SUPPORT
extern const void *BDSP_IMG_srs_tvol_code [];
extern const void *BDSP_IMG_srs_tvol_tables [];
extern const void *BDSP_IMG_srs_tvol_inter_frame [];
#endif

#ifdef BDSP_XEN_SUPPORT
extern const void *BDSP_IMG_xen[];
extern const void *BDSP_IMG_xen_table[];
extern const void *BDSP_IMG_xen_interframe_buf[];
#endif

#ifdef BDSP_BBE_SUPPORT
extern const void *BDSP_IMG_bbe[];
extern const void *BDSP_IMG_bbe_table[];
extern const void *BDSP_IMG_bbe_interframe_buf[];
#endif

#ifdef BDSP_CUSTOMSURROUND_SUPPORT
extern const void *BDSP_IMG_customsurround_code[];
extern const void *BDSP_IMG_customsurround_tables[];
extern const void *BDSP_IMG_customsurround_inter_frame[];
#endif

#ifdef BDSP_CUSTOMBASS_SUPPORT
extern const void *BDSP_IMG_custombass_code[];
extern const void *BDSP_IMG_custombass_tables[];
extern const void *BDSP_IMG_custombass_inter_frame[];
#endif

#ifdef BDSP_AACDOWNMIX_SUPPORT
#endif

#ifdef BDSP_DTSENC_SUPPORT
extern const void * BDSP_IMG_dts_encode [];
extern const void * BDSP_IMG_dts_encode_tables [];
extern const void * BDSP_IMG_dts_encode_inter_frame [];
#endif

#ifdef BDSP_MS10_SUPPORT
extern const void * BDSP_IMG_dd_transcode[];
extern const void * BDSP_IMG_dd_transcode_tables [];
extern const void * BDSP_IMG_dd_transcode_inter_frame [];
#else
#ifdef BDSP_AC3ENC_SUPPORT
extern const void * BDSP_IMG_dd_transcode[];
extern const void * BDSP_IMG_dd_transcode_tables [];
extern const void * BDSP_IMG_dd_transcode_inter_frame [];
#endif
#endif

#ifdef BDSP_CUSTOMVOICE_SUPPORT
extern const void * BDSP_IMG_customvoice_code [];
extern const void * BDSP_IMG_customvoice_tables [];
extern const void * BDSP_IMG_customvoice_inter_frame [];
#endif

#ifdef BDSP_SRC_SUPPORT
extern const void * BDSP_IMG_src_code [];
extern const void * BDSP_IMG_src_tables [];
extern const void * BDSP_IMG_src_inter_frame [];
#endif

#ifdef BDSP_RFAUDIO_SUPPORT
extern const void * BDSP_IMG_RfAudioBtsc[];
extern const void * BDSP_IMG_RfAudioEiaj[];
extern const void * BDSP_IMG_RfAudioKoreaa2[];
extern const void * BDSP_IMG_RfAudioNicam[];
extern const void * BDSP_IMG_RfAudioPala2[];
extern const void * BDSP_IMG_RfAudioSecaml[];
extern const void * BDSP_IMG_RfAudioIndia[];
#endif

#ifdef BDSP_KARAOKE_SUPPORT
extern const void * BDSP_IMG_karaoke_code [];
extern const void * BDSP_IMG_karaoke_tables [];
extern const void * BDSP_IMG_karaoke_inter_frame [];
#endif

#ifdef BDSP_OUTPUTFORMATTER_SUPPORT
extern const void * BDSP_IMG_outputformatter_code [];
extern const void * BDSP_IMG_outputformatter_tables [];
extern const void * BDSP_IMG_outputformatter_inter_frame [];
#endif

#ifdef BDSP_VOCALPP_SUPPORT
extern const void * BDSP_IMG_vocals_code [];
extern const void * BDSP_IMG_vocals_tables [];
extern const void * BDSP_IMG_vocals_inter_frame [];
#endif

#ifdef BDSP_AUDIODESC_SUPPORT
extern const void * BDSP_IMG_fade_control[];
extern const void * BDSP_IMG_pan_control[];
extern const void * BDSP_IMG_pan_control_interframe_buf[];
extern const void * BDSP_IMG_pan_control_tables[];
#endif

#ifdef BDSP_WMAPROPASSTHRU_SUPPORT
extern const void *  BDSP_IMG_wmapro_passthru_code[];
#endif
#ifdef BDSP_PCMROUTER_SUPPORT
extern const void * BDSP_IMG_pcm_router[];
#endif
#ifdef BDSP_DSOLA_SUPPORT
extern const void * BDSP_IMG_dsola_code[];
extern const void * BDSP_IMG_dsola_tables[];
extern const void * BDSP_IMG_dsola_inter_frame[];
#endif

#ifdef BDSP_DOLBYVOL_SUPPORT
extern const void * BDSP_IMG_dolby_volume[];
extern const void * BDSP_IMG_dolby_volume_table[];
extern const void * BDSP_IMG_dolby_volume_inter_frame[];
#endif


#ifdef BDSP_PCMWAV_SUPPORT
extern const void * BDSP_IMG_pcmwav_decode[];
extern const void * BDSP_IMG_pcmwav_decode_inter_frame[];
extern const void * BDSP_IMG_pcmwav_ids[];
extern const void * BDSP_IMG_pcmwav_ids_inter_frame[];
#endif

#ifdef BDSP_FADECTRL_SUPPORT
extern const void * BDSP_IMG_fadectrl_code[];
extern const void * BDSP_IMG_fadectrl_tables[];
extern const void * BDSP_IMG_fadectrl_inter_frame[];
#endif

#ifdef BDSP_AMBISONICS_SUPPORT
extern const void * BDSP_IMG_ambisonics_code[];
extern const void * BDSP_IMG_ambisonics_tables[];
extern const void * BDSP_IMG_ambisonics_inter_frame[];
#endif

#ifdef BDSP_TSMCORRECTION_SUPPORT
extern const void * BDSP_IMG_tsmcorrection_code[];
extern const void * BDSP_IMG_tsmcorrection_tables[];
extern const void * BDSP_IMG_tsmcorrection_inter_frame[];
#endif

#ifdef BDSP_MP3ENC_SUPPORT
extern const void * BDSP_IMG_mp3_encode[];
extern const void * BDSP_IMG_mp3_encode_tables[];
extern const void * BDSP_IMG_mp3_encode_inter_frame[];
#endif

#ifdef BDSP_AMR_SUPPORT
extern const void * BDSP_IMG_amr_decode[];
extern const void * BDSP_IMG_amr_decode_tables[];
extern const void * BDSP_IMG_amr_decode_inter_frame[];
#endif

#ifdef BDSP_AMRWB_SUPPORT
extern const void * BDSP_IMG_amrwb_decode[];
extern const void * BDSP_IMG_amrwb_decode_tables[];
extern const void * BDSP_IMG_amrwb_decode_inter_frame[];
#endif

#ifdef BDSP_ILBC_SUPPORT
extern const void * BDSP_IMG_ilbc_decode[];
extern const void * BDSP_IMG_ilbc_decode_tables[];
extern const void * BDSP_IMG_ilbc_decode_inter_frame[];
#endif

#ifdef BDSP_ISAC_SUPPORT
extern const void * BDSP_IMG_isac_decode[];
extern const void * BDSP_IMG_isac_decode_tables[];
extern const void * BDSP_IMG_isac_decode_inter_frame[];
#endif

#ifdef BDSP_UDC_SUPPORT
extern const void * BDSP_IMG_udc_decode[];
extern const void * BDSP_IMG_udc_decode_tables[];
extern const void * BDSP_IMG_udc_decode_inter_frame[];
#endif

#ifdef BDSP_OPUSDEC_SUPPORT
extern const void * BDSP_IMG_opus_decode[];
extern const void * BDSP_IMG_opus_decode_inter_frame[];
extern const void * BDSP_IMG_opus_decode_tables[];
#endif

#ifdef BDSP_ALS_SUPPORT
extern const void * BDSP_IMG_als_ids [];
extern const void * BDSP_IMG_als_ids_inter_frame [];
extern const void * BDSP_IMG_als_decode[];
extern const void * BDSP_IMG_als_decode_inter_frame[];
extern const void * BDSP_IMG_als_decode_tables[];
#endif

#ifdef BDSP_AC4_SUPPORT
extern const void * BDSP_IMG_ac4_ids [];
extern const void * BDSP_IMG_ac4_ids_inter_frame [];
extern const void * BDSP_IMG_ac4_decode[];
extern const void * BDSP_IMG_ac4_decode_inter_frame[];
extern const void * BDSP_IMG_ac4_decode_tables[];
#endif
#ifdef BDSP_DRA_SUPPORT
extern const void * BDSP_IMG_dra_decode[];
extern const void * BDSP_IMG_dra_decode_tables[];
extern const void * BDSP_IMG_dra_decode_inter_frame[];
#endif

#ifdef BDSP_SBCENC_SUPPORT
extern const void * BDSP_IMG_sbc_encode [];
extern const void * BDSP_IMG_sbc_encode_table [];
extern const void * BDSP_IMG_sbc_encode_inter_frame [];
#endif

#ifdef BDSP_REALAUDIOLBR_SUPPORT
extern const void * BDSP_IMG_ralbr_decode [];
extern const void * BDSP_IMG_ralbr_decode_tables [];
extern const void * BDSP_IMG_ralbr_decode_inter_frame [];
#endif

#ifdef BDSP_BRCM3DSURROUND_SUPPORT
extern const void * BDSP_IMG_brcm_3dsurround_code [];
extern const void * BDSP_IMG_brcm_3dsurround_tables [];
extern const void * BDSP_IMG_brcm_3dsurround_inter_frame [];
#endif

#ifdef BDSP_MONODOWNMIX_SUPPORT
extern const void * BDSP_IMG_monodownmix [];
#endif

#ifdef BDSP_FWMIXER_SUPPORT
extern const void * BDSP_IMG_mixer_ids[];
extern const void * BDSP_IMG_fw_mixer_code [];
extern const void * BDSP_IMG_fw_mixer_tables [];
extern const void * BDSP_IMG_fw_mixer_inter_frame [];
extern const void * BDSP_IMG_mixer_ids_inter_frame[];
#endif
#ifdef BDSP_MIXERDAPV2_SUPPORT
extern const void * BDSP_IMG_mixer_dapv2_ids[];
extern const void * BDSP_IMG_mixer_dapv2_code [];
extern const void * BDSP_IMG_mixer_dapv2_tables [];
extern const void * BDSP_IMG_mixer_dapv2_inter_frame [];
extern const void * BDSP_IMG_mixer_dapv2_ids_inter_frame[];
#endif

#ifdef BDSP_ADV_SUPPORT
extern const void * BDSP_IMG_adv_code [];
extern const void * BDSP_IMG_adv_tables [];
extern const void * BDSP_IMG_adv_inter_frame [];
#endif

#ifdef BDSP_ABX_SUPPORT
extern const void * BDSP_IMG_abx_code [];
extern const void * BDSP_IMG_abx_tables [];
extern const void * BDSP_IMG_abx_inter_frame [];
#endif

#ifdef BDSP_SRSCSTD_SUPPORT
extern const void * BDSP_IMG_srs_csdec_trudialog_code [];
extern const void * BDSP_IMG_srs_csdec_trudialog_tables [];
extern const void * BDSP_IMG_srs_csdec_trudialog_inter_frame [];
#endif

#ifdef BDSP_SRSEQHL_SUPPORT
extern const void * BDSP_IMG_srs_equ_hl_code [];
extern const void * BDSP_IMG_srs_equ_hl_tables [];
extern const void * BDSP_IMG_srs_equ_hl_inter_frame [];
#endif

#ifdef BDSP_CUSTOMDBE_SUPPORT
extern const void * BDSP_IMG_custom_dbe_code [];
extern const void * BDSP_IMG_custom_dbe_tables [];
extern const void * BDSP_IMG_custom_dbe_inter_frame [];
#endif

#ifdef BDSP_CUSTOMAVLP_SUPPORT
extern const void * BDSP_IMG_custom_avlp_code [];
extern const void * BDSP_IMG_custom_avlp_tables [];
extern const void * BDSP_IMG_custom_avlp_inter_frame [];
#endif

#ifdef BDSP_CUSTOMACF_SUPPORT
extern const void * BDSP_IMG_custom_acf_code [];
extern const void * BDSP_IMG_custom_acf_tables [];
extern const void * BDSP_IMG_custom_acf_inter_frame [];
#endif

#ifdef  BDSP_ADPCM_SUPPORT
extern const void * BDSP_IMG_adpcm_decode [];
extern const void * BDSP_IMG_adpcm_decode_tables [];
extern const void * BDSP_IMG_adpcm_decode_inter_frame [];
#endif

#ifdef BDSP_GENCDBITB_SUPPORT
extern const void * BDSP_IMG_gen_cdbitb_code [];
extern const void * BDSP_IMG_gen_cdbitb_tables [];
extern const void * BDSP_IMG_gen_cdbitb_inter_frame [];
#endif

#ifdef BDSP_BTSCENC_SUPPORT
extern const void * BDSP_IMG_btscenc_code [];
extern const void * BDSP_IMG_btscenc_tables [];
extern const void * BDSP_IMG_btscenc_inter_frame [];
#endif

#ifdef BDSP_KARAOKE_SUPPORT
extern const void * BDSP_IMG_karaoke_code [];
extern const void * BDSP_IMG_karaoke_tables [];
extern const void * BDSP_IMG_karaoke_inter_frame [];
#endif

#ifdef BDSP_AACHEENC_SUPPORT
extern const void * BDSP_IMG_aache_encode [];
extern const void * BDSP_IMG_aache_encode_tables [];
extern const void * BDSP_IMG_aache_encode_inter_frame [];
#endif

#ifdef BDSP_DV258_SUPPORT
extern const void *BDSP_IMG_dv258_code [];
extern const void *BDSP_IMG_dv258_tables [];
extern const void *BDSP_IMG_dv258_inter_frame [];
#endif

#ifdef BDSP_DPCMR_SUPPORT
extern const void *BDSP_IMG_dpcmr_code [];
extern const void *BDSP_IMG_dpcmr_tables [];
extern const void *BDSP_IMG_dpcmr_inter_frame [];
#endif
#ifdef BDSP_DDRE_SUPPORT
extern const void *BDSP_IMG_ddre_code [];
extern const void *BDSP_IMG_ddre_tables [];
extern const void *BDSP_IMG_ddre_inter_frame [];
#endif

#ifdef BDSP_VP6_SUPPORT
extern const void *BDSP_IMG_vp6_ids[];
extern const void *BDSP_IMG_vp6_decode[];
extern const void *BDSP_IMG_vp6_decode_tables[];
extern const void *BDSP_IMG_vp6_ids_inter_frame[];
extern const void *BDSP_IMG_vp6_decode_inter_frame[];
#endif

#ifdef BDSP_G711G726_SUPPORT
extern const void *BDSP_IMG_g711_g726_decode[];
extern const void *BDSP_IMG_g711_g726_decode_tables[];
extern const void *BDSP_IMG_g711_g726_decode_inter_frame[];
#endif

#ifdef BDSP_G729_SUPPORT
extern const void *BDSP_IMG_g729_decode[];
extern const void *BDSP_IMG_g729_decode_tables[];
extern const void *BDSP_IMG_g729_decode_inter_frame[];
#endif

#ifdef BDSP_G723_1_SUPPORT
extern const void *BDSP_IMG_g723_1_decode[];
extern const void *BDSP_IMG_g723_1_decode_tables[];
extern const void *BDSP_IMG_g723_1_decode_inter_frame[];
#endif

#ifdef BDSP_VORBIS_SUPPORT
extern const void *BDSP_IMG_vorbis_ids[];
extern const void *BDSP_IMG_vorbis_decode[];
extern const void *BDSP_IMG_vorbis_decode_tables[];
extern const void *BDSP_IMG_vorbis_ids_inter_frame[];
extern const void *BDSP_IMG_vorbis_decode_inter_frame[];
#endif

#ifdef BDSP_FLAC_SUPPORT
extern const void *BDSP_IMG_flac_ids[];
extern const void *BDSP_IMG_flac_decode[];
extern const void *BDSP_IMG_flac_decode_tables[];
extern const void *BDSP_IMG_flac_ids_inter_frame[];
extern const void *BDSP_IMG_flac_decode_inter_frame[];
#endif

#ifdef BDSP_MAC_SUPPORT
extern const void *BDSP_IMG_mac_ids[];
extern const void *BDSP_IMG_mac_decode[];
extern const void *BDSP_IMG_mac_decode_tables[];
extern const void *BDSP_IMG_mac_ids_inter_frame[];
extern const void *BDSP_IMG_mac_decode_inter_frame[];
#endif

#ifdef BDSP_G711G726ENC_SUPPORT
extern const void *BDSP_IMG_g711_g726_encode[];
extern const void *BDSP_IMG_g711_g726_encode_tables[];
extern const void *BDSP_IMG_g711_g726_encode_inter_frame[];
#endif

#ifdef BDSP_G729ENC_SUPPORT
extern const void *BDSP_IMG_g729_encode[];
extern const void *BDSP_IMG_g729_encode_tables[];
extern const void *BDSP_IMG_g729_encode_inter_frame[];
#endif

#ifdef BDSP_G723_1ENC_SUPPORT
extern const void *BDSP_IMG_g723_1_encode[];
extern const void *BDSP_IMG_g723_1_encode_tables[];
extern const void *BDSP_IMG_g723_1_encode_inter_frame[];
#endif

#ifdef BDSP_SPEEXAEC_SUPPORT
extern const void *BDSP_IMG_speexaec_code[];
extern const void *BDSP_IMG_speexaec_tables[];
extern const void *BDSP_IMG_speexaec_inter_frame[];
#endif

#ifdef BDSP_WMAENC_SUPPORT
extern const void *BDSP_IMG_wma_encode[];
extern const void *BDSP_IMG_wma_encode_tables[];
extern const void *BDSP_IMG_wma_encode_inter_frame[];
#endif

#ifdef BDSP_H264_ENCODE_SUPPORT
extern const void *BDSP_IMG_h264_encode[];
extern const void *BDSP_IMG_h264_encode_tables[];
extern const void *BDSP_IMG_h264_encode_inter_frame[];
#endif


#ifdef BDSP_X264_ENCODE_SUPPORT
extern const void *BDSP_IMG_x264_encode[];
extern const void *BDSP_IMG_x264_encode_tables[];
extern const void *BDSP_IMG_x264_encode_inter_frame[];
#endif
#ifdef BDSP_XVP8_ENCODE_SUPPORT
extern const void *BDSP_IMG_xvp8_encode[];
extern const void *BDSP_IMG_xvp8_encode_tables[];
extern const void *BDSP_IMG_xvp8_encode_inter_frame[];
#endif
#ifdef BDSP_MLP_PASSTHROUGH_SUPPORT
extern const void *  BDSP_IMG_mlp_passthrough_code [];
extern const void *  BDSP_IMG_mlp_passthrough_tables [];
extern const void *  BDSP_IMG_mlp_passthrough_inter_frame [];
#endif

#ifdef BDSP_G722ENC_SUPPORT
extern const void *BDSP_IMG_g722_encode[];
extern const void *BDSP_IMG_g722_encode_tables[];
extern const void *BDSP_IMG_g722_encode_inter_frame[];
#endif


#ifdef BDSP_AMRENC_SUPPORT
extern const void *BDSP_IMG_amr_encode[];
extern const void *BDSP_IMG_amr_encode_tables[];
extern const void *BDSP_IMG_amr_encode_inter_frame[];
#endif

#ifdef BDSP_AMRWBENC_SUPPORT
extern const void *BDSP_IMG_amrwb_encode[];
extern const void *BDSP_IMG_amrwb_encode_tables[];
extern const void *BDSP_IMG_amrwb_encode_inter_frame[];
#endif

#ifdef BDSP_SCM1_SUPPORT
extern const void *BDSP_IMG_scm1_decode[];
extern const void *BDSP_IMG_scm1_decode_tables[];
extern const void *BDSP_IMG_scm1_decode_inter_frame[];
extern const void * BDSP_IMG_scm1_digest [];
#endif

#ifdef BDSP_SCM2_SUPPORT
extern const void *BDSP_IMG_scm2_decode[];
extern const void *BDSP_IMG_scm2_decode_tables[];
extern const void *BDSP_IMG_scm2_decode_inter_frame[];
extern const void * BDSP_IMG_scm2_digest [];
#endif

#ifdef BDSP_SCM3_SUPPORT
extern const void *BDSP_IMG_scm3_decode[];
extern const void *BDSP_IMG_scm3_decode_tables[];
extern const void *BDSP_IMG_scm3_decode_inter_frame[];
#endif

#ifdef BDSP_ILBCENC_SUPPORT
extern const void *BDSP_IMG_ilbc_encode[];
extern const void *BDSP_IMG_ilbc_encode_tables[];
extern const void *BDSP_IMG_ilbc_encode_inter_frame[];
#endif

#ifdef BDSP_ISACENC_SUPPORT
extern const void *BDSP_IMG_isac_encode[];
extern const void *BDSP_IMG_isac_encode_tables[];
extern const void *BDSP_IMG_isac_encode_inter_frame[];
#endif

#ifdef BDSP_LPCMENC_SUPPORT
extern const void *BDSP_IMG_lpcm_encode_code[];
extern const void *BDSP_IMG_lpcm_encode_tables[];
extern const void *BDSP_IMG_lpcm_encode_inter_frame[];
#endif

#ifdef BDSP_OPUS_ENC_SUPPORT
extern const void *BDSP_IMG_opus_encode_code[];
extern const void *BDSP_IMG_opus_encode_tables[];
extern const void *BDSP_IMG_opus_encode_inter_frame[];
#endif

#ifdef BDSP_DDPENC_SUPPORT
extern const void *BDSP_IMG_ddp_encode_code[];
extern const void *BDSP_IMG_ddp_encode_tables[];
extern const void *BDSP_IMG_ddp_encode_inter_frame[];
#endif

#endif
