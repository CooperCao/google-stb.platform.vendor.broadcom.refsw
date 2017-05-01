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
    switch ( imgId )
    {
#if (BCHP_CHIP != 7278)
    /* Special cases for system ID's (don't follow standard AlgoId->img convention) */
    case BDSP_SystemImgId_eSystemCode:                                  return BDSP_IMG_system_code;
    case BDSP_SystemImgId_eSystemRdbvars:                               return BDSP_IMG_system_rdbvars;
    case BDSP_SystemImgId_eSyslibCode:                                  return BDSP_IMG_syslib_code;
    case BDSP_SystemImgId_eAlgolibCode:                                 return BDSP_IMG_algolib_code;
    case BDSP_SystemImgId_eCommonIdsCode:                               return BDSP_IMG_idscommon_code;
    case BDSP_SystemImgId_eCommonVideoEncodeIdsCode:                    return BDSP_IMG_vididscommon_code;
    case BDSP_SystemImgId_eCommonVideoEncodeIdsInterframe:              return BDSP_IMG_vididscommon_inter_frame;
    case BDSP_SystemImgId_eScm_Task_Code:                               return BDSP_IMG_scm_task_code;
#ifdef BDSP_SCM1_SUPPORT
    case BDSP_SystemImgId_eScm1_Digest:                                 return BDSP_IMG_scm1_digest;
#endif
#ifdef BDSP_SCM2_SUPPORT
    case BDSP_SystemImgId_eScm2_Digest:                                 return BDSP_IMG_scm2_digest;
#endif
    case BDSP_SystemImgId_eVideo_Decode_Task_Code:                      return BDSP_IMG_video_decode_task_code;
    case BDSP_SystemImgId_eVideo_Encode_Task_Code:                      return BDSP_IMG_video_encode_task_code;
#else
    case BDSP_SystemImgId_eSystemKernelCode:                            return BDSP_IMG_system_kernel;
    case BDSP_SystemImgId_eSystemRdbvars:                               return BDSP_IMG_system_rdbvars;
    case BDSP_SystemImgId_eSystemRomfsCode:                             return BDSP_IMG_system_romfs;

#endif

    /* Below values are sorted by BDSP_AF_P_AlgoId values for consistency */

    /* Start of audio decode algorithms */
    #ifdef BDSP_MPEG_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMpegDecode):                return BDSP_IMG_mpeg1_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eMpegDecode):               return BDSP_IMG_mpeg1_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMpegDecode):              return BDSP_IMG_mpeg1_decode_inter_frame;
    #endif
    #if defined BDSP_AC3_SUPPORT && !defined BDSP_DDP_SUPPORT && !defined BDSP_MS10_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAc3Decode):                 return BDSP_IMG_ac3_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAc3Decode):                return BDSP_IMG_ac3_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAc3Decode):               return BDSP_IMG_ac3_decode_inter_frame;
    #endif
    #if defined BDSP_AACSBR_SUPPORT && !defined BDSP_MS10_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAacHeLpSbrDecode):          return BDSP_IMG_aache_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAacHeLpSbrDecode):         return BDSP_IMG_aache_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAacHeLpSbrDecode):        return BDSP_IMG_aache_decode_inter_frame;
    #endif
    #if defined BDSP_DDP_SUPPORT && !defined BDSP_MS10_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDdpDecode):                 return BDSP_IMG_ddp_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDdpDecode):                return BDSP_IMG_ddp_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDdpDecode):               return BDSP_IMG_ddp_decode_inter_frame;
    #endif
    #ifdef BDSP_LPCMDVD_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDvdLpcmDecode):             return BDSP_IMG_lpcm_decode_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDvdLpcmDecode):            return BDSP_IMG_lpcm_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDvdLpcmDecode):           return BDSP_IMG_lpcm_decode_inter_frame;
    #endif
    #ifdef BDSP_WMA_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eWmaStdDecode):              return BDSP_IMG_wma_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eWmaStdDecode):             return BDSP_IMG_wma_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eWmaStdDecode):            return BDSP_IMG_wma_decode_inter_frame;
    #endif
    #ifdef BDSP_WMAPRO_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eWmaProStdDecode):           return BDSP_IMG_wma_pro_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eWmaProStdDecode):          return BDSP_IMG_wma_pro_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eWmaProStdDecode):         return BDSP_IMG_wma_pro_decode_inter_frame;
    #endif
    #ifdef BDSP_MLP_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMlpDecode):                 return BDSP_IMG_truehd_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eMlpDecode):                return BDSP_IMG_truehd_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMlpDecode):               return BDSP_IMG_truehd_decode_inter_frame;
    #endif
    #ifdef BDSP_DTSLBR_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDtsLbrDecode):              return BDSP_IMG_dts_express_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDtsLbrDecode):             return BDSP_IMG_dts_express_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDtsLbrDecode):            return BDSP_IMG_dts_express_decode_inter_frame;
    #endif
    #if defined(BDSP_DTSBROADCAST_SUPPORT) || defined(BDSP_DTSHD_SUPPORT)
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDtsHdDecode):               return BDSP_IMG_dtshd_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDtsHdDecode):              return BDSP_IMG_dtshd_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDtsHdDecode):             return BDSP_IMG_dtshd_decode_inter_frame;
    #endif
    #ifdef BDSP_PCMWAV_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_ePcmWavDecode):              return BDSP_IMG_pcmwav_decode;
    /* PCMWAV has no table entry */
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_ePcmWavDecode):            return BDSP_IMG_pcmwav_decode_inter_frame;
    #endif
    #ifdef BDSP_AMR_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAmrDecode):                 return BDSP_IMG_amr_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAmrDecode):                return BDSP_IMG_amr_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAmrDecode):               return BDSP_IMG_amr_decode_inter_frame;
    #endif
    #ifdef BDSP_DRA_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDraDecode):                 return BDSP_IMG_dra_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDraDecode):                return BDSP_IMG_dra_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDraDecode):               return BDSP_IMG_dra_decode_inter_frame;
    #endif
    #ifdef BDSP_REALAUDIOLBR_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eRealAudioLbrDecode):        return BDSP_IMG_ralbr_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eRealAudioLbrDecode):       return BDSP_IMG_ralbr_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eRealAudioLbrDecode):      return BDSP_IMG_ralbr_decode_inter_frame;
    #endif
    #ifdef BDSP_DOLBY_AACHE_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDolbyAacheDecode):          return BDSP_IMG_dolby_aache_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDolbyAacheDecode):         return BDSP_IMG_dolby_aache_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDolbyAacheDecode):        return BDSP_IMG_dolby_aache_decode_inter_frame;
    #elif BDSP_MS10_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDolbyPulseDecode):          return BDSP_IMG_dolby_pulse_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDolbyPulseDecode):         return BDSP_IMG_dolby_pulse_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDolbyPulseDecode):        return BDSP_IMG_dolby_pulse_decode_inter_frame;
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMs10DdpDecode):             return BDSP_IMG_dolby_ms_ddp_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eMs10DdpDecode):            return BDSP_IMG_dolby_ms_ddp_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMs10DdpDecode):           return BDSP_IMG_dolby_ms_ddp_decode_inter_frame;
    #endif
    #ifdef BDSP_ADPCM_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAdpcmDecode):               return BDSP_IMG_adpcm_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAdpcmDecode):              return BDSP_IMG_adpcm_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAdpcmDecode):             return BDSP_IMG_adpcm_decode_inter_frame;
    #endif
    #ifdef BDSP_G711G726_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eG711G726Decode):            return BDSP_IMG_g711_g726_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eG711G726Decode):           return BDSP_IMG_g711_g726_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eG711G726Decode):          return BDSP_IMG_g711_g726_decode_inter_frame;
    #endif
    #ifdef BDSP_G729_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eG729Decode):                return BDSP_IMG_g729_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eG729Decode):               return BDSP_IMG_g729_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eG729Decode):              return BDSP_IMG_g729_decode_inter_frame;
    #endif
    #ifdef BDSP_VORBIS_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eVorbisDecode):              return BDSP_IMG_vorbis_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eVorbisDecode):             return BDSP_IMG_vorbis_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eVorbisDecode):            return BDSP_IMG_vorbis_decode_inter_frame;
    #endif
    #ifdef BDSP_G723_1_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eG723_1Decode):              return BDSP_IMG_g723_1_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eG723_1Decode):             return BDSP_IMG_g723_1_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eG723_1Decode):            return BDSP_IMG_g723_1_decode_inter_frame;
    #endif
    #ifdef BDSP_FLAC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eFlacDecode):                return BDSP_IMG_flac_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eFlacDecode):               return BDSP_IMG_flac_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eFlacDecode):              return BDSP_IMG_flac_decode_inter_frame;
    #endif
    #ifdef BDSP_MAC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMacDecode):                 return BDSP_IMG_mac_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eMacDecode):                return BDSP_IMG_mac_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMacDecode):               return BDSP_IMG_mac_decode_inter_frame;
    #endif
    #ifdef BDSP_AMRWB_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAmrWbDecode):               return BDSP_IMG_amrwb_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAmrWbDecode):              return BDSP_IMG_amrwb_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAmrWbDecode):             return BDSP_IMG_amrwb_decode_inter_frame;
    #endif
    #ifdef BDSP_ILBC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eiLBCDecode):               return BDSP_IMG_ilbc_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eiLBCDecode):              return BDSP_IMG_ilbc_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eiLBCDecode):             return BDSP_IMG_ilbc_decode_inter_frame;
    #endif
    #ifdef BDSP_ISAC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eiSACDecode):               return BDSP_IMG_isac_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eiSACDecode):              return BDSP_IMG_isac_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eiSACDecode):             return BDSP_IMG_isac_decode_inter_frame;
    #endif
    #ifdef BDSP_UDC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eUdcDecode):               return BDSP_IMG_udc_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eUdcDecode):              return BDSP_IMG_udc_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eUdcDecode):             return BDSP_IMG_udc_decode_inter_frame;
    #endif
    #ifdef BDSP_OPUSDEC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eOpusDecode):              return BDSP_IMG_opus_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eOpusDecode):              return BDSP_IMG_opus_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eOpusDecode):              return BDSP_IMG_opus_decode_inter_frame;
    #endif
    #ifdef BDSP_ALS_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eALSDecode):              return BDSP_IMG_als_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eALSDecode):              return BDSP_IMG_als_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eALSDecode):              return BDSP_IMG_als_decode_inter_frame;
    #endif
    #ifdef BDSP_AC4_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAC4Decode):              return BDSP_IMG_ac4_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAC4Decode):              return BDSP_IMG_ac4_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAC4Decode):              return BDSP_IMG_ac4_decode_inter_frame;
    #endif
    /* End of audio decode algorithms */

    /* Start of video decode algorithms */
    #ifdef BDSP_VP6_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_VF_P_AlgoId_eVP6Decode):                 return BDSP_IMG_vp6_decode;
    case BDSP_IMG_ID_TABLE(BDSP_VF_P_AlgoId_eVP6Decode):                return BDSP_IMG_vp6_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_VF_P_AlgoId_eVP6Decode):               return BDSP_IMG_vp6_decode_inter_frame;
    #endif
    /* End of video decode algorithms */

    /* Start of audio framesync */
    #ifdef BDSP_MPEG_PASSTHRU_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMpegFrameSync):             return BDSP_IMG_mpeg1_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMpegFrameSync):           return BDSP_IMG_mpeg1_ids_inter_frame;
    #endif
    #if defined BDSP_AACSBR_PASSTHRU_SUPPORT || defined BDSP_MS10_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAdtsFrameSync):             return BDSP_IMG_aacheadts_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAdtsFrameSync):           return BDSP_IMG_aacheadts_ids_inter_frame;
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eLoasFrameSync):             return BDSP_IMG_aacheloas_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eLoasFrameSync):           return BDSP_IMG_aacheloas_ids_inter_frame;
    #endif
    #if defined BDSP_WMA_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eWmaStdFrameSync):           return BDSP_IMG_wma_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eWmaStdFrameSync):         return BDSP_IMG_wma_ids_inter_frame;
    #endif
    #if defined BDSP_WMAPRO_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eWmaProFrameSync):           return BDSP_IMG_wma_pro_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eWmaProFrameSync):         return BDSP_IMG_wma_pro_ids_inter_frame;
    #endif
    #ifdef  BDSP_AC3_PASSTHRU_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAc3FrameSync):              return BDSP_IMG_ac3_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAc3FrameSync):            return BDSP_IMG_ac3_ids_inter_frame;
    #endif
    #ifdef  BDSP_DDP_PASSTHRU_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDdpFrameSync):              return BDSP_IMG_ddp_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDdpFrameSync):            return BDSP_IMG_ddp_ids_inter_frame;
    #endif
    #if defined BDSP_DTSBROADCAST_PASSTHRU_SUPPORT || defined BDSP_DTS_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDtsFrameSync):              return BDSP_IMG_dts_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDtsFrameSync):            return BDSP_IMG_dts_ids_inter_frame;
    #endif
    #ifdef BDSP_DTSLBR_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDtsLbrFrameSync):           return BDSP_IMG_dts_express_ids_decode;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDtsLbrFrameSync):         return BDSP_IMG_dts_express_ids_inter_frame;
    #endif
    #ifdef  BDSP_DTSHD_PASSTHRU_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDtsHdFrameSync):            return BDSP_IMG_dtshd_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDtsHdFrameSync):          return BDSP_IMG_dtshd_ids_inter_frame;
    #endif
    #if defined (BDSP_MLP_SUPPORT) || defined (BDSP_MLP_PASSTHROUGH_SUPPORT)
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMlpFrameSync):              return BDSP_IMG_mlp_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMlpFrameSync):            return BDSP_IMG_mlp_ids_inter_frame;
    #endif
    #ifdef BDSP_PCM_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_ePesFrameSync):              return BDSP_IMG_pcm_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_ePesFrameSync):            return BDSP_IMG_pcm_ids_inter_frame;
    #endif
    #ifdef BDSP_LPCMDVD_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDvdLpcmFrameSync):          return BDSP_IMG_dvdlpcm_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDvdLpcmFrameSync):        return BDSP_IMG_dvdlpcm_ids_inter_frame;
    #endif
    #ifdef BDSP_PCMWAV_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_ePcmWavFrameSync):           return BDSP_IMG_pcmwav_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_ePcmWavFrameSync):         return BDSP_IMG_pcmwav_ids_inter_frame;
    #endif

    #ifdef  BDSP_DRA_PASSTHRU_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDraFrameSync):              return BDSP_IMG_dra_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDraFrameSync):            return BDSP_IMG_dra_ids_inter_frame;
    #endif
    #ifdef  BDSP_REALAUDIOLBR_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync):     return BDSP_IMG_ralbr_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync):   return BDSP_IMG_ralbr_ids_inter_frame;
    #endif
    #ifdef BDSP_MS10_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMs10DdpFrameSync):          return BDSP_IMG_ddp_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMs10DdpFrameSync):        return BDSP_IMG_ddp_ids_inter_frame;
    #endif

    #ifdef BDSP_VORBIS_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eVorbisFrameSync):           return BDSP_IMG_vorbis_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eVorbisFrameSync):         return BDSP_IMG_vorbis_ids_inter_frame;
    #endif

    #ifdef BDSP_FLAC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eFlacFrameSync):             return BDSP_IMG_flac_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eFlacFrameSync):           return BDSP_IMG_flac_ids_inter_frame;
    #endif
    #ifdef BDSP_MAC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMacFrameSync):              return BDSP_IMG_mac_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMacFrameSync):            return BDSP_IMG_mac_ids_inter_frame;
    #endif

    #if defined (BDSP_UDC_SUPPORT) || defined (BDSP_UDC_PASSTHRU_SUPPORT)
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eUdcFrameSync):             return BDSP_IMG_udc_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eUdcFrameSync):           return BDSP_IMG_udc_ids_inter_frame;
    #endif
    #if defined (BDSP_AC4_SUPPORT)
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAC4FrameSync):             return BDSP_IMG_ac4_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAC4FrameSync):           return BDSP_IMG_ac4_ids_inter_frame;
    #endif
    #if defined (BDSP_ALS_SUPPORT)
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eALSFrameSync):             return BDSP_IMG_als_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eALSFrameSync):           return BDSP_IMG_als_ids_inter_frame;
    #endif
    /* End of audio framesync */

    /* Start of video framesync */
    #ifdef BDSP_VP6_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_VF_P_AlgoId_eVP6FrameSync):              return BDSP_IMG_vp6_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_VF_P_AlgoId_eVP6FrameSync):            return BDSP_IMG_vp6_ids_inter_frame;
    #endif
    /* End of video framesync */

    /* Start of audio encode */
    #ifdef BDSP_MP3ENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMpegL3Encode):              return BDSP_IMG_mp3_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eMpegL3Encode):             return BDSP_IMG_mp3_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMpegL3Encode):            return BDSP_IMG_mp3_encode_inter_frame;
    #endif
    #ifdef BDSP_AACHEENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAacHeEncode):               return BDSP_IMG_aache_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAacHeEncode):              return BDSP_IMG_aache_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAacHeEncode):             return BDSP_IMG_aache_encode_inter_frame;
    #endif
    #ifdef BDSP_DTSENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDtsEncode):                 return BDSP_IMG_dts_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDtsEncode):                return BDSP_IMG_dts_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDtsEncode):               return BDSP_IMG_dts_encode_inter_frame;
    #endif
    #ifdef BDSP_MS10_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMs10DDTranscode):           return BDSP_IMG_dd_transcode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eMs10DDTranscode):          return BDSP_IMG_dd_transcode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMs10DDTranscode):         return BDSP_IMG_dd_transcode_inter_frame;
    #endif
    #ifdef BDSP_G711G726ENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eG711G726Encode):            return BDSP_IMG_g711_g726_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eG711G726Encode):           return BDSP_IMG_g711_g726_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eG711G726Encode):          return BDSP_IMG_g711_g726_encode_inter_frame;
    #endif
    #ifdef BDSP_G729ENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eG729Encode):                return BDSP_IMG_g729_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eG729Encode):               return BDSP_IMG_g729_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eG729Encode):              return BDSP_IMG_g729_encode_inter_frame;
    #endif
    #ifdef BDSP_G723_1ENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eG723_1Encode):              return BDSP_IMG_g723_1_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eG723_1Encode):             return BDSP_IMG_g723_1_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eG723_1Encode):            return BDSP_IMG_g723_1_encode_inter_frame;
    #endif
    #ifdef BDSP_AMRENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAmrEncode):                 return BDSP_IMG_amr_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAmrEncode):                return BDSP_IMG_amr_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAmrEncode):               return BDSP_IMG_amr_encode_inter_frame;
    #endif
    #ifdef BDSP_AMRWBENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAmrwbEncode):               return BDSP_IMG_amrwb_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAmrwbEncode):              return BDSP_IMG_amrwb_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAmrwbEncode):             return BDSP_IMG_amrwb_encode_inter_frame;
    #endif
    #ifdef BDSP_ILBCENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eiLBCEncode):                return BDSP_IMG_ilbc_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eiLBCEncode):               return BDSP_IMG_ilbc_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eiLBCEncode):              return BDSP_IMG_ilbc_encode_inter_frame;
    #endif
    #ifdef BDSP_ISACENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eiSACEncode):                return BDSP_IMG_isac_encode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eiSACEncode):               return BDSP_IMG_isac_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eiSACEncode):              return BDSP_IMG_isac_encode_inter_frame;
    #endif
    #ifdef BDSP_LPCMENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eLpcmEncode):                return BDSP_IMG_lpcm_encode_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eLpcmEncode):               return BDSP_IMG_lpcm_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eLpcmEncode):              return BDSP_IMG_lpcm_encode_inter_frame;
    #endif
    #ifdef BDSP_OPUS_ENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eOpusEncode):                return BDSP_IMG_opus_encode_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eOpusEncode):               return BDSP_IMG_opus_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eOpusEncode):              return BDSP_IMG_opus_encode_inter_frame;
    #endif
    #ifdef BDSP_DDPENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDDPEncode):                 return BDSP_IMG_ddp_encode_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDDPEncode):                return BDSP_IMG_ddp_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDDPEncode):               return BDSP_IMG_ddp_encode_inter_frame;
    #endif
    /* End of audio encode */

    /* Start of video encode */
    #ifdef BDSP_H264_ENCODE_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_VF_P_AlgoId_eH264Encode):                return BDSP_IMG_h264_encode;
    case BDSP_IMG_ID_TABLE(BDSP_VF_P_AlgoId_eH264Encode):               return BDSP_IMG_h264_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_VF_P_AlgoId_eH264Encode):              return BDSP_IMG_h264_encode_inter_frame;
    #endif

     #ifdef BDSP_X264_ENCODE_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_VF_P_AlgoId_eX264Encode):                return BDSP_IMG_x264_encode;
    case BDSP_IMG_ID_TABLE(BDSP_VF_P_AlgoId_eX264Encode):               return BDSP_IMG_x264_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_VF_P_AlgoId_eX264Encode):              return BDSP_IMG_x264_encode_inter_frame;
    #endif
	#ifdef BDSP_XVP8_ENCODE_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_VF_P_AlgoId_eXVP8Encode):                return BDSP_IMG_xvp8_encode;
    case BDSP_IMG_ID_TABLE(BDSP_VF_P_AlgoId_eXVP8Encode):               return BDSP_IMG_xvp8_encode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_VF_P_AlgoId_eXVP8Encode):              return BDSP_IMG_xvp8_encode_inter_frame;
    #endif
    /* End of video encode */

    /* Start of SCM */
    #ifdef BDSP_SCM1_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eScm1):                      return BDSP_IMG_scm1_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eScm1):                     return BDSP_IMG_scm1_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eScm1):                    return BDSP_IMG_scm1_decode_inter_frame;
    #endif

    #ifdef BDSP_SCM2_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eScm2):                      return BDSP_IMG_scm2_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eScm2):                     return BDSP_IMG_scm2_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eScm2):                    return BDSP_IMG_scm2_decode_inter_frame;
    #endif

    #ifdef BDSP_SCM3_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eScm3):                      return BDSP_IMG_scm3_decode;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eScm3):                     return BDSP_IMG_scm3_decode_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eScm3):                    return BDSP_IMG_scm3_decode_inter_frame;
    #endif
    /* End of SCM */

    /* Start of audio encode framesync */

    /* None of these seem to have FW binaries now */

    /* End of audio encode framesync */
/* #if (BCHP_CHIP != 7278) */
    /* Start of passthrough */
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_ePassThru):                  return BDSP_IMG_cdb_passthru_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_ePassThru):                 return BDSP_IMG_cdb_passthru_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_ePassThru):                return BDSP_IMG_cdb_passthru_inter_frame;
/* #endif */
#ifdef BDSP_MLP_PASSTHROUGH_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMLPPassThru):               return BDSP_IMG_mlp_passthrough_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eMLPPassThru):              return BDSP_IMG_mlp_passthrough_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMLPPassThru):             return BDSP_IMG_mlp_passthrough_inter_frame;
#endif
    /* End of passthrough */

    /* Start of postprocessing */
#ifdef BDSP_SRC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eSrcPostProc):               return BDSP_IMG_src_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eSrcPostProc):              return BDSP_IMG_src_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eSrcPostProc):             return BDSP_IMG_src_inter_frame;
#endif
#ifdef BDSP_CUSTOMVOICE_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eCustomVoicePostProc):       return BDSP_IMG_customvoice_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eCustomVoicePostProc):      return BDSP_IMG_customvoice_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eCustomVoicePostProc):     return BDSP_IMG_customvoice_inter_frame;
#endif
#ifdef BDSP_AVL_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eAvlPostProc):               return BDSP_IMG_brcm_avl_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eAvlPostProc):              return BDSP_IMG_brcm_avl_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eAvlPostProc):             return BDSP_IMG_brcm_avl_inter_frame;
#endif
#ifdef BDSP_DSOLA_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDsolaPostProc):             return BDSP_IMG_dsola_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDsolaPostProc):            return BDSP_IMG_dsola_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDsolaPostProc):           return BDSP_IMG_dsola_inter_frame;
#endif
#ifdef BDSP_SRSHD_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc):  return BDSP_IMG_srs_trusurroundhd_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc): return BDSP_IMG_srs_trusurroundhd_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc):return BDSP_IMG_srs_trusurroundhd_inter_frame;
#endif
#ifdef BDSP_SRSTRUVOL_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eSrsTruVolumePostProc):      return BDSP_IMG_srs_tvol_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eSrsTruVolumePostProc):     return BDSP_IMG_srs_tvol_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eSrsTruVolumePostProc):    return BDSP_IMG_srs_tvol_inter_frame;
#endif
#ifdef BDSP_BRCM3DSURROUND_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc):    return BDSP_IMG_brcm_3dsurround_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc):   return BDSP_IMG_brcm_3dsurround_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc):  return BDSP_IMG_brcm_3dsurround_inter_frame;
#endif
#ifdef BDSP_FWMIXER_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eFWMixerPostProc):           return BDSP_IMG_fw_mixer_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eFWMixerPostProc):          return BDSP_IMG_fw_mixer_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eFWMixerPostProc):         return BDSP_IMG_fw_mixer_inter_frame;
#endif
#ifdef BDSP_DDRE_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDdrePostProc):              return BDSP_IMG_ddre_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDdrePostProc):             return BDSP_IMG_ddre_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDdrePostProc):            return BDSP_IMG_ddre_inter_frame;
#endif
#ifdef BDSP_DV258_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDv258PostProc):             return BDSP_IMG_dv258_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDv258PostProc):            return BDSP_IMG_dv258_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDv258PostProc):           return BDSP_IMG_dv258_inter_frame;
#endif
#ifdef BDSP_DPCMR_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eDpcmrPostProc):             return BDSP_IMG_dpcmr_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eDpcmrPostProc):            return BDSP_IMG_dpcmr_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eDpcmrPostProc):           return BDSP_IMG_dpcmr_inter_frame;
#endif
#ifdef BDSP_GENCDBITB_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eGenCdbItbPostProc):         return BDSP_IMG_gen_cdbitb_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eGenCdbItbPostProc):        return BDSP_IMG_gen_cdbitb_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eGenCdbItbPostProc):       return BDSP_IMG_gen_cdbitb_inter_frame;
#endif
#ifdef BDSP_BTSCENC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eBtscEncoderPostProc):       return BDSP_IMG_btscenc_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eBtscEncoderPostProc):      return BDSP_IMG_btscenc_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eBtscEncoderPostProc):     return BDSP_IMG_btscenc_inter_frame;
#endif
#ifdef BDSP_KARAOKE_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eKaraokePostProc):       return BDSP_IMG_karaoke_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eKaraokePostProc):      return BDSP_IMG_karaoke_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eKaraokePostProc):     return BDSP_IMG_karaoke_inter_frame;
#endif
#ifdef BDSP_SPEEXAEC_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eSpeexAECPostProc):          return BDSP_IMG_speexaec_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eSpeexAECPostProc):         return BDSP_IMG_speexaec_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eSpeexAECPostProc):        return BDSP_IMG_speexaec_inter_frame;
#endif
#ifdef BDSP_MIXERDAPV2_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMixerDapv2PostProc):        return BDSP_IMG_mixer_dapv2_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eMixerDapv2PostProc):       return BDSP_IMG_mixer_dapv2_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMixerDapv2PostProc):      return BDSP_IMG_mixer_dapv2_inter_frame;
#endif
#ifdef BDSP_OUTPUTFORMATTER_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eOutputFormatterPostProc):        return BDSP_IMG_outputformatter_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eOutputFormatterPostProc):       return BDSP_IMG_outputformatter_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eOutputFormatterPostProc):      return BDSP_IMG_outputformatter_inter_frame;
#endif

#ifdef BDSP_VOCALPP_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eVocalPostProc):        return BDSP_IMG_vocals_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eVocalPostProc):       return BDSP_IMG_vocals_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eVocalPostProc):      return BDSP_IMG_vocals_inter_frame;
#endif
#ifdef BDSP_FADECTRL_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eFadeCtrlPostProc):        return BDSP_IMG_fadectrl_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eFadeCtrlPostProc):       return BDSP_IMG_fadectrl_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eFadeCtrlPostProc):      return BDSP_IMG_fadectrl_inter_frame;
#endif
#ifdef BDSP_TSMCORRECTION_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eTsmCorrectionPostProc):        return BDSP_IMG_tsmcorrection_code;
    case BDSP_IMG_ID_TABLE(BDSP_AF_P_AlgoId_eTsmCorrectionPostProc):       return BDSP_IMG_tsmcorrection_tables;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eTsmCorrectionPostProc):      return BDSP_IMG_tsmcorrection_inter_frame;
#endif
    /* End of postprocessing */

    /* Start of postprocessing framesync */
#ifdef BDSP_FWMIXER_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMixerFrameSync):            return BDSP_IMG_mixer_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMixerFrameSync):          return BDSP_IMG_mixer_ids_inter_frame;
#endif
#ifdef BDSP_MIXERDAPV2_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eMixerDapv2FrameSync):       return BDSP_IMG_mixer_dapv2_ids;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eMixerDapv2FrameSync):     return BDSP_IMG_mixer_dapv2_ids_inter_frame;
#endif
    /* End of postprocessing framesync */

#if (BCHP_CHIP != 7278)
    /* Start of libraries (not handled above in system code) */
#ifdef BDSP_H264_ENCODE_SUPPORT
    case BDSP_IMG_ID_CODE(BDSP_AF_P_AlgoId_eVidIDSCommonLib):           return BDSP_IMG_vididscommon_code;
    case BDSP_IMG_ID_IFRAME(BDSP_AF_P_AlgoId_eVidIDSCommonLib):         return BDSP_IMG_vididscommon_inter_frame;
#endif
#endif
    /* End of libraries */

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
