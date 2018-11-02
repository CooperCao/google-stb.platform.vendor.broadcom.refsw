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
#ifndef BDSP_ARM_IMG_H_
#define BDSP_ARM_IMG_H_

#include "bimg.h"

typedef enum BDSP_ARM_SystemImgId
{
    BDSP_ARM_SystemImgId_eRdbVars,
    BDSP_ARM_SystemImgId_eInitProcess,
    BDSP_ARM_SystemImgId_eIdleProcess,
    BDSP_ARM_SystemImgId_eAlgoProcess,
    BDSP_ARM_SystemImgId_eMessageProcess,
    BDSP_ARM_SystemImgId_eRoutingProcess,
    BDSP_ARM_SystemImgId_eSchedulingProcess,
    BDSP_ARM_SystemImgId_eLibCommon,
    BDSP_ARM_SystemImgId_eLibCommonIds,
    BDSP_ARM_SystemImgId_eLibKernalOps,
    BDSP_ARM_SystemImgId_eLibMMClient,
    BDSP_ARM_SystemImgId_eLibStageIO,
    BDSP_ARM_SystemImgId_eLibDebugControl,
    BDSP_ARM_SystemImgId_eLibMMServer,
    BDSP_ARM_SystemImgId_eMax,
    BDSP_ARM_SystemImgId_eInvalid = 0x7FFFFFFF
} BDSP_ARM_SystemImgId;

#define BDSP_ARM_IMG_ID_BASE(algorithm)   (BDSP_ARM_SystemImgId_eMax+((algorithm)*4))
#define BDSP_ARM_IMG_ID_CODE(algorithm)   (BDSP_ARM_IMG_ID_BASE(algorithm)+0)
#define BDSP_ARM_IMG_ID_IDS(algorithm)    (BDSP_ARM_IMG_ID_BASE(algorithm)+1)
#define BDSP_ARM_IMG_ID_TABLE(algorithm)  (BDSP_ARM_IMG_ID_BASE(algorithm)+2)
#define BDSP_ARM_IMG_ID_IFRAME(algorithm) (BDSP_ARM_IMG_ID_BASE(algorithm)+3)
#define BDSP_ARM_IMG_ID_MAX               (BDSP_ARM_IMG_ID_BASE(BDSP_Algorithm_eMax))

#define BDSP_ARM_IMG_ID_TO_ALGO(imgId) (((imgId)<BDSP_ARM_SystemImgId_eMax)?(BDSP_Algorithm_eMax):(((imgId)-BDSP_ARM_SystemImgId_eMax)/4))

extern void *BDSP_ARM_IMG_Context;
extern const BIMG_Interface BDSP_ARM_IMG_Interface;

extern const void * BDSP_ARM_IMG_system_rdbvars[];
extern const void * BDSP_ARM_IMG_init_process[];
extern const void * BDSP_ARM_IMG_idle_process [];
extern const void * BDSP_ARM_IMG_algo_process [];
extern const void * BDSP_ARM_IMG_messaging_process [];
extern const void * BDSP_ARM_IMG_routing_process [];
extern const void * BDSP_ARM_IMG_scheduling_process [];
extern const void * BDSP_ARM_IMG_common [];
extern const void * BDSP_ARM_IMG_common_ids [];
extern const void * BDSP_ARM_IMG_kernel_ops [];
extern const void * BDSP_ARM_IMG_mm_client [];
extern const void * BDSP_ARM_IMG_stage_io [];
extern const void * BDSP_ARM_IMG_debug_control [];
extern const void * BDSP_ARM_IMG_mm_server [];

#if defined (BDSP_MPEG_SUPPORT) || defined (BDSP_MPEG_PASSTHRU_SUPPORT)
extern const void * BDSP_ARM_IMG_aids_mpeg1[];
#endif

#ifdef BDSP_MPEG_SUPPORT
extern const void * BDSP_ARM_IMG_adec_mpeg1[];
extern const void * BDSP_ARM_IMG_adec_mpeg1_tables[];
extern const void * BDSP_ARM_IMG_adec_mpeg1_inter_frame[];
#endif /*BDSP_MPEG_SUPPORT*/

#if defined (BDSP_UDC_PASSTHRU_SUPPORT) || defined (BDSP_UDC_SUPPORT)
extern const void * BDSP_ARM_IMG_aids_ddp[];
#endif /*BDSP_UDC_PASSTHRU_SUPPORT*/

#ifdef BDSP_UDC_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
extern const void * BDSP_ARM_IMG_adec_ms11plus_udc[];
extern const void * BDSP_ARM_IMG_adec_ms11plus_udc_tables[];
extern const void * BDSP_ARM_IMG_adec_ms11plus_udc_inter_frame[];
#else
extern const void * BDSP_ARM_IMG_adec_udc[];
extern const void * BDSP_ARM_IMG_adec_udc_tables[];
extern const void * BDSP_ARM_IMG_adec_udc_inter_frame[];
#endif
#endif /*BDSP_UDC_SUPPORT*/

#if defined (BDSP_AC3_PASSTHRU_SUPPORT) || defined (BDSP_AC3_SUPPORT)
extern const void * BDSP_ARM_IMG_aids_ddp[];
#endif /*BDSP_AC3_PASSTHRU_SUPPORT*/

#ifdef BDSP_AC3_SUPPORT
extern const void * BDSP_ARM_IMG_adec_ac3[];
extern const void * BDSP_ARM_IMG_adec_ac3_tables[];
extern const void * BDSP_ARM_IMG_adec_ac3_inter_frame[];
#endif /*BDSP_AC3_SUPPORT*/

#if defined (BDSP_DDP_PASSTHRU_SUPPORT) || defined (BDSP_DDP_SUPPORT)
extern const void * BDSP_ARM_IMG_aids_ddp[];
#endif /*BDSP_AC3_PASSTHRU_SUPPORT*/

#ifdef BDSP_DDP_SUPPORT
extern const void * BDSP_ARM_IMG_adec_ddp[];
extern const void * BDSP_ARM_IMG_adec_ddp_tables[];
extern const void * BDSP_ARM_IMG_adec_ddp_inter_frame[];
#endif /*BDSP_DDP_SUPPORT*/

#if defined (BDSP_AACSBR_PASSTHRU_SUPPORT) || defined (BDSP_AACSBR_SUPPORT)
extern const void * BDSP_ARM_IMG_aids_adts[];
extern const void * BDSP_ARM_IMG_aids_loas[];
#endif/*BDSP_AACSBR_PASSTHRU_SUPPORT*/

#ifdef BDSP_AACSBR_SUPPORT
extern const void * BDSP_ARM_IMG_adec_aache [];
extern const void * BDSP_ARM_IMG_adec_aache_tables[];
extern const void * BDSP_ARM_IMG_adec_aache_inter_frame[];
#endif /*BDSP_AACSBR_SUPPORT*/

#ifdef BDSP_DOLBY_AACHE_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
extern const void * BDSP_ARM_IMG_adec_ms11plus_dolby_aache [];
extern const void * BDSP_ARM_IMG_adec_ms11plus_dolby_aache_tables[];
extern const void * BDSP_ARM_IMG_adec_ms11plus_dolby_aache_inter_frame[];
#else
extern const void * BDSP_ARM_IMG_adec_dolby_aache [];
extern const void * BDSP_ARM_IMG_adec_dolby_aache_tables[];
extern const void * BDSP_ARM_IMG_adec_dolby_aache_inter_frame[];
#endif
#endif /* DOLBY ACCHE BDSP_AACSBR_SUPPORT*/
#ifdef BDSP_SRC_SUPPORT
extern const void * BDSP_ARM_IMG_app_src[];
extern const void * BDSP_ARM_IMG_app_src_tables[];
extern const void * BDSP_ARM_IMG_app_src_inter_frame[];
#endif /*BDSP_SRC_SUPPORT*/

#ifdef BDSP_MIXERDAPV2_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT/* Mixer without DAP */
extern const void * BDSP_ARM_IMG_app_ms11plus_mixer[];
extern const void * BDSP_ARM_IMG_app_ms11plus_mixer_tables[];
extern const void * BDSP_ARM_IMG_app_ms11plus_mixer_inter_frame[];
#else/* Mixer with DAP */
extern const void * BDSP_ARM_IMG_app_mixer_dapv2[];
extern const void * BDSP_ARM_IMG_app_mixer_dapv2_tables[];
extern const void * BDSP_ARM_IMG_app_mixer_dapv2_inter_frame[];
#endif
#endif /*BDSP_MIXERDAPV2_SUPPORT*/
#ifdef BDSP_FWMIXER_SUPPORT
extern const void * BDSP_ARM_IMG_app_fw_mixer[];
extern const void * BDSP_ARM_IMG_app_fw_mixer_tables[];
extern const void * BDSP_ARM_IMG_app_fw_mixer_inter_frame[];
#endif /*BDSP_FWMIXER_SUPPORT*/
#ifdef BDSP_PCMWAV_SUPPORT
extern const void * BDSP_ARM_IMG_aids_wavformatex[];/* Move this to passthru like others when pcmwav passthru support comes in */
extern const void * BDSP_ARM_IMG_adec_pcmwav[];
extern const void * BDSP_ARM_IMG_adec_pcmwav_tables[];
extern const void * BDSP_ARM_IMG_adec_pcmwav_inter_frame[];
#endif /*BDSP_PCMWAV_SUPPORT*/

#ifdef BDSP_PASSTHRU_SUPPORT
extern const void * BDSP_ARM_IMG_adec_passthru [];
extern const void * BDSP_ARM_IMG_adec_passthru_tables[];
extern const void * BDSP_ARM_IMG_adec_passthru_inter_frame[];
#endif /*BDSP_PASSTHRU_SUPPORT*/

#ifdef BDSP_DPCMR_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
extern const void * BDSP_ARM_IMG_app_ms11plus_dpcmr[];
extern const void * BDSP_ARM_IMG_app_ms11plus_dpcmr_tables[];
extern const void * BDSP_ARM_IMG_app_ms11plus_dpcmr_inter_frame[];
#else
extern const void * BDSP_ARM_IMG_app_dpcmr[];
extern const void * BDSP_ARM_IMG_app_dpcmr_tables[];
extern const void * BDSP_ARM_IMG_app_dpcmr_inter_frame[];
#endif
#endif /*BDSP_DPCMR_SUPPORT*/

#ifdef BDSP_DDPENC_SUPPORT
#ifdef BDSP_MS11PLUS_SUPPORT
extern const void * BDSP_ARM_IMG_aenc_ms11plus_dd[];
extern const void * BDSP_ARM_IMG_aenc_ms11plus_dd_tables[];
extern const void * BDSP_ARM_IMG_aenc_ms11plus_dd_inter_frame[];
#else
extern const void * BDSP_ARM_IMG_aenc_ddp[];
extern const void * BDSP_ARM_IMG_aenc_ddp_tables[];
extern const void * BDSP_ARM_IMG_aenc_ddp_inter_frame[];
#endif
#endif /*BDSP_DDPENC_SUPPORT*/
#ifdef BDSP_DSOLA_SUPPORT
extern const void * BDSP_ARM_IMG_app_dsola[];
extern const void * BDSP_ARM_IMG_app_dsola_tables[];
extern const void * BDSP_ARM_IMG_app_dsola_inter_frame[];
#endif /*BDSP_DSOLA_SUPPORT*/
#ifdef BDSP_OPUSDEC_SUPPORT
extern const void * BDSP_ARM_IMG_adec_opus[];
extern const void * BDSP_ARM_IMG_adec_opus_tables[];
extern const void * BDSP_ARM_IMG_adec_opus_inter_frame[];
#endif /*BDSP_OPUSDEC_SUPPORT*/
#ifdef BDSP_SOFT_FMM_SUPPORT
extern const void * BDSP_ARM_IMG_app_soft_fmm[];
extern const void * BDSP_ARM_IMG_app_soft_fmm_tables[];
extern const void * BDSP_ARM_IMG_app_soft_fmm_inter_frame[];
#endif /*BDSP_SRC_SUPPORT*/
#if defined (BDSP_DTSHD_PASSTHRU_SUPPORT)
extern const void * BDSP_ARM_IMG_aids_dtshd[];
#endif /*BDSP_DTSHD_PASSTHRU_SUPPORT*/
#if defined (BDSP_DTS_PASSTHRU_SUPPORT)
extern const void * BDSP_ARM_IMG_aids_dts[];
#endif/*BDSP_DTS_PASSTHRU_SUPPORT*/
#endif /*BDSP_ARM_IMG_H_*/
