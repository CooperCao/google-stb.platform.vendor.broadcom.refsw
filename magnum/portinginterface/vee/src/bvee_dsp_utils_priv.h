/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BVEE_DSP_UTILS_PRIV_H_
#define BVEE_DSP_UTILS_PRIV_H_

#include "bvee_priv.h"
#include "bdsp.h"
#include "bdsp_task.h"
#include "bdsp_audio_task.h"
#ifdef BCHP_AIO_MISC_REG_START
#include "bchp_aio_misc.h"
#include "bchp_aud_fmm_misc.h"
#else
#include "bchp_aud_misc.h"
#endif

 /***************************************************************************
 Summary:
 Codec Attribute Table Entry
 ***************************************************************************/
 typedef struct BVEE_CodecAttributes
 {
     BAVC_VideoCompressionStd codec;
     BDSP_Algorithm decodeAlgorithm;
     const char *pName;
 } BVEE_CodecAttributes;
 
 /***************************************************************************
 Summary:
 Get Codec Attributes
 ***************************************************************************/
 const BVEE_CodecAttributes *BVEE_P_GetCodecAttributes(
     BAVC_VideoCompressionStd codec
     );
 

 
 #define BVEE_P_GetCodecName(codec) (BVEE_P_GetCodecAttributes((codec))->pName)
 #define BVEE_P_GetCodecVideoType(codec) (BVEE_P_GetCodecAttributes((codec))->decodeAlgorithm)
 /***************************************************************************
 Summary:
 Get STC ADDRESS to be passed to DSP
 ***************************************************************************/
 #ifdef BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END

 /* Legacy RDB */
 #define BVEE_CHP_MAX_STCS (BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END)
 #ifdef BCHP_AUD_FMM_MISC_STC_LOWERi_ARRAY_BASE
 #define BVEE_CHP_GET_STC_ADDR(idx) (BCHP_PHYSICAL_OFFSET + BCHP_AUD_FMM_MISC_STC_LOWERi_ARRAY_BASE + ((BCHP_AUD_FMM_MISC_STC_LOWERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
 #define BVEE_CHP_GET_STC_ADDR_HI(idx)  (BCHP_PHYSICAL_OFFSET + BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE + ((BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
 #else
 #define BVEE_CHP_GET_STC_ADDR(idx) (BCHP_PHYSICAL_OFFSET + BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE + ((BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
 #define BVEE_CHP_GET_STC_ADDR_HI(idx) (uint32_t)(NULL) 
 #endif
 #else
 
 /* 7429 style RDB */
 #define BVEE_CHP_MAX_STCS (BCHP_AUD_MISC_STC_UPPERi_ARRAY_END)
 #ifdef BCHP_AUD_MISC_STC_LOWERi_ARRAY_BASE
 #define BVEE_CHP_GET_STC_ADDR(idx) (BCHP_PHYSICAL_OFFSET + BCHP_AUD_MISC_STC_LOWERi_ARRAY_BASE + ((BCHP_AUD_MISC_STC_LOWERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
 #define BVEE_CHP_GET_STC_ADDR_HI(idx)  (BCHP_PHYSICAL_OFFSET + BCHP_AUD_MISC_STC_UPPERi_ARRAY_BASE + ((BCHP_AUD_MISC_STC_UPPERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
 #else
 #define BVEE_CHP_GET_STC_ADDR(idx) (BCHP_PHYSICAL_OFFSET + BCHP_AUD_MISC_STC_UPPERi_ARRAY_BASE + ((BCHP_AUD_MISC_STC_UPPERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
 #define BVEE_CHP_GET_STC_ADDR_HI(idx) (uint32_t)(NULL)  
 #endif

 #endif

#endif
