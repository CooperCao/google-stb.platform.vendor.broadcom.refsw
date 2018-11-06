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

typedef enum BDSP_SystemImgId
{
    BDSP_SystemImgId_eSystemKernelCode,
    BDSP_SystemImgId_eSystemRdbvars,
    BDSP_SystemImgId_eInitRomfsCode,
	BDSP_SystemImgId_eSystemCode,
	BDSP_SystemImgId_eSystemLib,
    BDSP_SystemImgId_eMax,
    BDSP_SystemImgId_eInvalid = 0x7FFFFFFF
} BDSP_SystemImgId;

#define BDSP_IMG_ID_BASE(algorithm)   (BDSP_SystemImgId_eMax+((algorithm)*4))
#define BDSP_IMG_ID_CODE(algorithm)   (BDSP_IMG_ID_BASE(algorithm)+0)
#define BDSP_IMG_ID_IDS(algorithm)    (BDSP_IMG_ID_BASE(algorithm)+1)
#define BDSP_IMG_ID_TABLE(algorithm)  (BDSP_IMG_ID_BASE(algorithm)+2)
#define BDSP_IMG_ID_IFRAME(algorithm) (BDSP_IMG_ID_BASE(algorithm)+3)
#define BDSP_IMG_ID_MAX               (BDSP_IMG_ID_BASE(BDSP_Algorithm_eMax))

#define BDSP_IMG_ID_TO_ALGO(imgId) (((imgId)<BDSP_SystemImgId_eMax)?(BDSP_Algorithm_eMax):(((imgId)-BDSP_SystemImgId_eMax)/4))

extern void *BDSP_IMG_Context;
extern const BIMG_Interface BDSP_IMG_Interface;

extern const void * BDSP_IMG_system_kernel[];
extern const void * BDSP_IMG_system_rdbvars[];
extern const void * BDSP_IMG_init_romfs[];
extern const void * BDSP_IMG_system_code[];
extern const void * BDSP_IMG_system_lib[];

#if defined (BDSP_MPEG_SUPPORT) || defined (BDSP_MPEG_PASSTHRU_SUPPORT)
extern const void * BDSP_IMG_aids_mpeg1[];
#endif

#ifdef BDSP_MPEG_SUPPORT
extern const void * BDSP_IMG_adec_mpeg1[];
extern const void * BDSP_IMG_adec_mpeg1_tables[];
extern const void * BDSP_IMG_adec_mpeg1_inter_frame[];
#endif /*BDSP_MPEG_SUPPORT*/

#if defined (BDSP_UDC_PASSTHRU_SUPPORT) || defined (BDSP_UDC_SUPPORT)
extern const void * BDSP_IMG_aids_ddp[];
#endif /*BDSP_UDC_PASSTHRU_SUPPORT*/

#ifdef BDSP_UDC_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
extern const void * BDSP_IMG_adec_ms11plus_udc[];
extern const void * BDSP_IMG_adec_ms11plus_udc_tables[];
extern const void * BDSP_IMG_adec_ms11plus_udc_inter_frame[];
#else
extern const void * BDSP_IMG_adec_udc[];
extern const void * BDSP_IMG_adec_udc_tables[];
extern const void * BDSP_IMG_adec_udc_inter_frame[];
#endif
#endif /*BDSP_UDC_SUPPORT*/

#if defined (BDSP_AC3_PASSTHRU_SUPPORT) || defined (BDSP_AC3_SUPPORT)
extern const void * BDSP_IMG_aids_ddp[];
#endif /*BDSP_AC3_PASSTHRU_SUPPORT*/

#ifdef BDSP_AC3_SUPPORT
extern const void * BDSP_IMG_adec_ac3[];
extern const void * BDSP_IMG_adec_ac3_tables[];
extern const void * BDSP_IMG_adec_ac3_inter_frame[];
#endif /*BDSP_AC3_SUPPORT*/

#if defined (BDSP_DDP_PASSTHRU_SUPPORT) || defined (BDSP_DDP_SUPPORT)
extern const void * BDSP_IMG_aids_ddp[];
#endif /*BDSP_AC3_PASSTHRU_SUPPORT*/

#ifdef BDSP_DDP_SUPPORT
extern const void * BDSP_IMG_adec_ddp[];
extern const void * BDSP_IMG_adec_ddp_tables[];
extern const void * BDSP_IMG_adec_ddp_inter_frame[];
#endif /*BDSP_DDP_SUPPORT*/

#if defined (BDSP_AACSBR_PASSTHRU_SUPPORT) || defined (BDSP_AACSBR_SUPPORT)
extern const void * BDSP_IMG_aids_adts[];
extern const void * BDSP_IMG_aids_loas[];
#endif/*BDSP_AACSBR_PASSTHRU_SUPPORT*/

#ifdef BDSP_AACSBR_SUPPORT
extern const void * BDSP_IMG_adec_aache [];
extern const void * BDSP_IMG_adec_aache_tables[];
extern const void * BDSP_IMG_adec_aache_inter_frame[];
#endif /*BDSP_AACSBR_SUPPORT*/

#ifdef BDSP_DOLBY_AACHE_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
extern const void * BDSP_IMG_adec_ms11plus_dolby_aache [];
extern const void * BDSP_IMG_adec_ms11plus_dolby_aache_tables[];
extern const void * BDSP_IMG_adec_ms11plus_dolby_aache_inter_frame[];
#else
extern const void * BDSP_IMG_adec_dolby_aache [];
extern const void * BDSP_IMG_adec_dolby_aache_tables[];
extern const void * BDSP_IMG_adec_dolby_aache_inter_frame[];
#endif
#endif /* DOLBY ACCHE BDSP_AACSBR_SUPPORT*/

#ifdef BDSP_SRC_SUPPORT
extern const void * BDSP_IMG_app_src[];
extern const void * BDSP_IMG_app_src_tables[];
extern const void * BDSP_IMG_app_src_inter_frame[];
#endif /*BDSP_SRC_SUPPORT*/

#ifdef BDSP_MIXERDAPV2_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT/* Mixer without DAP */
extern const void * BDSP_IMG_app_ms11plus_mixer[];
extern const void * BDSP_IMG_app_ms11plus_mixer_tables[];
extern const void * BDSP_IMG_app_ms11plus_mixer_inter_frame[];
#else/* Mixer with DAP */
extern const void * BDSP_IMG_app_mixer_dapv2[];
extern const void * BDSP_IMG_app_mixer_dapv2_tables[];
extern const void * BDSP_IMG_app_mixer_dapv2_inter_frame[];
#endif
#endif /*BDSP_MIXERDAPV2_SUPPORT*/

#ifdef BDSP_FWMIXER_SUPPORT
extern const void * BDSP_IMG_app_fw_mixer[];
extern const void * BDSP_IMG_app_fw_mixer_tables[];
extern const void * BDSP_IMG_app_fw_mixer_inter_frame[];
#endif /*BDSP_FWMIXER_SUPPORT*/

#ifdef BDSP_PCMWAV_SUPPORT
extern const void * BDSP_IMG_aids_wavformatex[];/* Move this to passthru like others when pcmwav passthru support comes in */
extern const void * BDSP_IMG_adec_pcmwav[];
extern const void * BDSP_IMG_adec_pcmwav_tables[];
extern const void * BDSP_IMG_adec_pcmwav_inter_frame[];
#endif /*BDSP_PCMWAV_SUPPORT*/

#ifdef BDSP_PASSTHRU_SUPPORT
extern const void * BDSP_IMG_adec_passthru [];
extern const void * BDSP_IMG_adec_passthru_tables[];
extern const void * BDSP_IMG_adec_passthru_inter_frame[];
#endif /*BDSP_PASSTHRU_SUPPORT*/

#ifdef BDSP_DPCMR_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
extern const void * BDSP_IMG_app_ms11plus_dpcmr[];
extern const void * BDSP_IMG_app_ms11plus_dpcmr_tables[];
extern const void * BDSP_IMG_app_ms11plus_dpcmr_inter_frame[];
#else
extern const void * BDSP_IMG_app_dpcmr[];
extern const void * BDSP_IMG_app_dpcmr_tables[];
extern const void * BDSP_IMG_app_dpcmr_inter_frame[];
#endif
#endif /*BDSP_DPCMR_SUPPORT*/

#ifdef BDSP_DDPENC_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
extern const void * BDSP_IMG_aenc_ms11plus_dd[];
extern const void * BDSP_IMG_aenc_ms11plus_dd_tables[];
extern const void * BDSP_IMG_aenc_ms11plus_dd_inter_frame[];
#else
extern const void * BDSP_IMG_aenc_ddp[];
extern const void * BDSP_IMG_aenc_ddp_tables[];
extern const void * BDSP_IMG_aenc_ddp_inter_frame[];
#endif
#endif /*BDSP_DDPENC_SUPPORT*/

#ifdef BDSP_AACHEENC_SUPPORT
extern const void * BDSP_IMG_aenc_aache[];
extern const void * BDSP_IMG_aenc_aache_tables[];
extern const void * BDSP_IMG_aenc_aache_inter_frame[];
#endif /*BDSP_AACHEENC_SUPPORT*/

#ifdef BDSP_GENCDBITB_SUPPORT
extern const void * BDSP_IMG_app_gen_cdbitb[];
extern const void * BDSP_IMG_app_gen_cdbitb_tables[];
extern const void * BDSP_IMG_app_gen_cdbitb_inter_frame[];
#endif /*BDSP_GENCDBITB_SUPPORT*/

#ifdef BDSP_DSOLA_SUPPORT
extern const void * BDSP_IMG_app_dsola[];
extern const void * BDSP_IMG_app_dsola_tables[];
extern const void * BDSP_IMG_app_dsola_inter_frame[];
#endif /*BDSP_DSOLA_SUPPORT*/
#ifdef BDSP_OPUSDEC_SUPPORT
extern const void * BDSP_IMG_adec_opus[];
extern const void * BDSP_IMG_adec_opus_tables[];
extern const void * BDSP_IMG_adec_opus_inter_frame[];
#endif /*BDSP_OPUSDEC_SUPPORT*/

#ifdef BDSP_ALS_SUPPORT
extern const void * BDSP_IMG_adec_als[];
extern const void * BDSP_IMG_adec_als_tables[];
extern const void * BDSP_IMG_adec_als_inter_frame[];
#endif /*BDSP_ALSDEC_SUPPORT*/
#ifdef BDSP_FLACDEC_SUPPORT
extern const void * BDSP_IMG_adec_flac[];
extern const void * BDSP_IMG_adec_flac_tables[];
extern const void * BDSP_IMG_adec_flac_inter_frame[];
#endif /*BDSP_FLACDEC_SUPPORT*/
#ifdef BDSP_LPCMENC_SUPPORT
extern const void * BDSP_IMG_aenc_lpcm[];
extern const void * BDSP_IMG_aenc_lpcm_tables[];
extern const void * BDSP_IMG_aenc_lpcm_inter_frame[];
#endif /*BDSP_LPCMENC_SUPPORT*/

#ifdef BDSP_MP3ENC_SUPPORT
extern const void * BDSP_IMG_aenc_mp3[];
extern const void * BDSP_IMG_aenc_mp3_tables[];
extern const void * BDSP_IMG_aenc_mp3_inter_frame[];
#endif /*BDSP_MP3ENC_SUPPORT*/

#ifdef BDSP_AC4_SUPPORT
extern const void * BDSP_IMG_aids_ac4[];
extern const void * BDSP_IMG_adec_ac4[];
extern const void * BDSP_IMG_adec_ac4_tables[];
extern const void * BDSP_IMG_adec_ac4_inter_frame[];
#endif /*BDSP_AC4_SUPPORT*/
#ifdef BDSP_LPCMDVD_SUPPORT
extern const void * BDSP_IMG_aids_lpcm[];
extern const void * BDSP_IMG_adec_lpcm[];
extern const void * BDSP_IMG_adec_lpcm_tables[];
extern const void * BDSP_IMG_adec_lpcm_inter_frame[];
#endif /*BDSP_LPCMDVD_SUPPORT*/
#ifdef BDSP_WMAPRO_SUPPORT
extern const void * BDSP_IMG_aids_wma_pro[];
extern const void * BDSP_IMG_adec_wma_pro[];
extern const void * BDSP_IMG_adec_wma_pro_tables[];
extern const void * BDSP_IMG_adec_wma_pro_inter_frame[];
#endif /*BDSP_WMAPRO_SUPPORT*/
#ifdef BDSP_WMA_SUPPORT
extern const void * BDSP_IMG_aids_wma[];
extern const void * BDSP_IMG_adec_wma[];
extern const void * BDSP_IMG_adec_wma_tables[];
extern const void * BDSP_IMG_adec_wma_inter_frame[];
#endif /*BDSP_WMA_SUPPORT*/
#ifdef BDSP_OUTPUTFORMATTER_SUPPORT
extern const void * BDSP_IMG_app_outputformatter[];
extern const void * BDSP_IMG_app_outputformatter_tables[];
extern const void * BDSP_IMG_app_outputformatter_inter_frame[];
#endif
#if defined (BDSP_DTSHD_PASSTHRU_SUPPORT)
extern const void * BDSP_IMG_aids_dtshd[];
#endif /*BDSP_DTSHD_PASSTHRU_SUPPORT*/
#if defined (BDSP_DTS_PASSTHRU_SUPPORT)
extern const void * BDSP_IMG_aids_dts[];
#endif/*BDSP_DTS_PASSTHRU_SUPPORT*/
#endif /*BDSP_RAAGA_IMG_H_*/
