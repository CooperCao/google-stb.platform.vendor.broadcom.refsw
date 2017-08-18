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

/* This chunk size will be used when the firmware binary is actually present in
    multiple chunks. The BDSP_IMG_CHUNK_SIZE will then give the size of each
    such chunk
*/
#define BDSP_IMG_CHUNK_SIZE 65532


extern void *BDSP_IMG_Context;
extern const BIMG_Interface BDSP_IMG_Interface;

extern const void * BDSP_IMG_system_kernel[];
extern const void * BDSP_IMG_system_rdbvars[];
extern const void * BDSP_IMG_init_romfs[];
extern const void * BDSP_IMG_system_code[];
extern const void * BDSP_IMG_system_lib[];

#ifdef BDSP_MPEG_SUPPORT
extern const void * BDSP_IMG_aids_mpeg1[]; /* Move this to passthru like others when mpeg passthru support comes in */
extern const void * BDSP_IMG_adec_mpeg1[];
extern const void * BDSP_IMG_adec_mpeg1_tables[];
extern const void * BDSP_IMG_adec_mpeg1_inter_frame[];
#endif /*BDSP_MPEG_SUPPORT*/

#ifdef BDSP_UDC_SUPPORT
extern const void * BDSP_IMG_adec_udc[];
extern const void * BDSP_IMG_adec_udc_tables[];
extern const void * BDSP_IMG_adec_udc_inter_frame[];
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

#ifdef BDSP_SRC_SUPPORT
extern const void * BDSP_IMG_app_src[];
extern const void * BDSP_IMG_app_src_tables[];
extern const void * BDSP_IMG_app_src_inter_frame[];
#endif /*BDSP_SRC_SUPPORT*/

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

#endif /*BDSP_RAAGA_IMG_H_*/
