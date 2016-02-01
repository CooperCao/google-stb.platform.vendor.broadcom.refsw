/***************************************************************************
*     Copyright (c) 2006-2012, Broadcom Corporation
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
* Module Description:
*	This file is part of image interface implementation for RAP PI.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef BRAP_IMG_H__
#define BRAP_IMG_H__

#include "bimg.h"

/* 
	Firmware Image IDs that are to be used in the BRAP_IMG_Open function of the BRAP_IMG_Interface
*/

typedef enum BRAP_Img_Id
{
BRAP_Img_Id_eSystemIboot,
BRAP_Img_Id_eSystemCode,
BRAP_Img_Id_eSystemData,
BRAP_Img_Id_eSystemRdb,
BRAP_Img_Id_eDecodeTsm,
BRAP_Img_Id_eDecodeTsmInterframe,
BRAP_Img_Id_eEncodeTsm,
BRAP_Img_Id_ePassthruTsm,
BRAP_Img_Id_ePassthruCode,
BRAP_Img_Id_ePassthruTable,
BRAP_Img_Id_ePassthruInterFrame,
BRAP_Img_Id_eSystemAudioTask,
BRAP_Img_Id_eSystemGfxTask,
BRAP_Img_Id_eSystemVideoTask,

#ifndef RAP_MULTISTREAM_DECODER_SUPPORT
#ifdef  RAP_AC3_PASSTHRU_SUPPORT       
BRAP_Img_Id_eAc3_Ids,
BRAP_Img_Id_eAc3_Ids_Interframe,
#endif
#endif

#ifdef  RAP_MPEG_PASSTHRU_SUPPORT       
BRAP_Img_Id_eMpeg1_Ids,
BRAP_Img_Id_eMpeg1_Ids_Interframe,
#endif

#ifdef RAP_AACSBR_PASSTHRU_SUPPORT
BRAP_IMG_Id_eAache_Adts_Ids,
BRAP_IMG_Id_eAache_Loas_Ids,
BRAP_IMG_Id_eAache_Adts_Ids_Interframe,
BRAP_IMG_Id_eAache_Loas_Ids_Interframe,
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
BRAP_Img_Id_eMsDdp_Ids,
BRAP_Img_Id_eMsDdp_Ids_Interframe,
#else
#ifdef  RAP_DDP_PASSTHRU_SUPPORT       
BRAP_Img_Id_eDdp_Ids,
BRAP_Img_Id_eDdp_Ids_Interframe,
#endif
#endif

#ifdef  RAP_DTSBROADCAST_PASSTHRU_SUPPORT       
BRAP_Img_Id_eDtsBroadcast_Ids,
BRAP_Img_Id_eDtsBroadcast_Ids_Interframe,
#endif

#ifdef  RAP_DTSHD_PASSTHRU_SUPPORT       
BRAP_Img_Id_eDtsHd_Ids,
BRAP_Img_Id_eDtsHd_Ids_Interframe,
#endif

#ifdef  RAP_DRA_PASSTHRU_SUPPORT       
BRAP_Img_Id_eDRA_Ids,
BRAP_Img_Id_eDRA_Ids_Interframe,
#endif

#ifdef  RAP_REALAUDIOLBR_PASSTHRU_SUPPORT       
BRAP_Img_Id_eRealAudioLbr_Ids,
BRAP_Img_Id_eRealAudioLbr_Ids_Interframe,
#endif

#ifdef RAP_MPEG_SUPPORT
BRAP_Img_Id_eMpeg1_Decode,
BRAP_Img_Id_eMpeg1_Decode_Table,
BRAP_Img_Id_eMpeg1_Interframe,
#endif

#ifdef RAP_AAC_SUPPORT
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
BRAP_IMG_Id_eDolbyPulse_Decode_Stg0,
BRAP_IMG_Id_eDolbyPulse_Decode_Stg1,
BRAP_IMG_Id_eDolbyPulse_Decode_Stg2,
BRAP_IMG_Id_eDolbyPulse_Decode_Stg3,
BRAP_IMG_Id_eDolbyPulse_Decode_Table_Stg0,
BRAP_IMG_Id_eDolbyPulse_Decode_Table_Stg1,
BRAP_IMG_Id_eDolbyPulse_Decode_Table_Stg2,
BRAP_IMG_Id_eDolbyPulse_Decode_Table_Stg3,
BRAP_IMG_Id_eDolbyPulse_Interframe_Stg0,
BRAP_IMG_Id_eDolbyPulse_Interframe_Stg1,
BRAP_IMG_Id_eDolbyPulse_Interframe_Stg2,
BRAP_IMG_Id_eDolbyPulse_Interframe_Stg3,
#else
#ifdef RAP_AACSBR_SUPPORT
BRAP_IMG_Id_eAache_Decode_Stg0,
BRAP_IMG_Id_eAache_Decode_Stg1,
BRAP_IMG_Id_eAache_Decode_Stg2,
BRAP_IMG_Id_eAache_Decode_Stg3,
BRAP_IMG_Id_eAache_Decode_Stg4,
BRAP_IMG_Id_eAache_Decode_Table_Stg0,
BRAP_IMG_Id_eAache_Decode_Table_Stg1,
BRAP_IMG_Id_eAache_Decode_Table_Stg2,
BRAP_IMG_Id_eAache_Decode_Table_Stg3,
BRAP_IMG_Id_eAache_Decode_Table_Stg4,
BRAP_IMG_Id_eAache_Interframe_Stg0,
BRAP_IMG_Id_eAache_Interframe_Stg1,
BRAP_IMG_Id_eAache_Interframe_Stg2,
BRAP_IMG_Id_eAache_Interframe_Stg3,
BRAP_IMG_Id_eAache_Interframe_Stg4,
#endif
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
BRAP_Img_Id_eMsDdp_Be_Decode,
BRAP_Img_Id_eMsDdp_Fe_Decode,
BRAP_Img_Id_eMsDdp_Be_Decode_Tables,
BRAP_Img_Id_eMsDdp_Fe_Decode_Tables,
BRAP_Img_Id_eMsDdp_Be_Interframe,
BRAP_Img_Id_eMsDdp_Fe_Interframe,

#else
#ifdef RAP_AC3_SUPPORT
/*BRAP_Img_Id_eAc3_Ids,*/
BRAP_Img_Id_eAc3_Be_Decode,
BRAP_Img_Id_eAc3_Fe_Decode,
BRAP_Img_Id_eAc3_Be_Decode_Tables,
BRAP_Img_Id_eAc3_Fe_Decode_Tables,
BRAP_Img_Id_eAc3_Be_Interframe,
BRAP_Img_Id_eAc3_Fe_Interframe,
/*BRAP_Img_Id_eAc3_Ids_Interframe,*/
#endif

#ifdef RAP_DDP_SUPPORT
BRAP_Img_Id_eDdp_Be_Decode,
BRAP_Img_Id_eDdp_Fe_Decode,
BRAP_Img_Id_eDdp_Be_Decode_Tables,
BRAP_Img_Id_eDdp_Fe_Decode_Tables,
BRAP_Img_Id_eDdp_Be_Interframe,
BRAP_Img_Id_eDdp_Fe_Interframe,
#endif
#endif

#ifdef RAP_DTS_SUPPORT
BRAP_Img_Id_eDts_Ids,
BRAP_Img_Id_eDts_Ids_Interframe,
#endif

#if defined(RAP_DTSBROADCAST_SUPPORT) || defined(RAP_DTSHD_SUPPORT)
BRAP_Img_Id_eDtsBroadcast_Decode_Stg0,
BRAP_Img_Id_eDtsBroadcast_Decode_Table_Stg0,
BRAP_Img_Id_eDtsBroadcast_Decode_Interframe_Stg0,
BRAP_Img_Id_eDtsBroadcast_Decode_Stg1,
BRAP_Img_Id_eDtsBroadcast_Decode_Table_Stg1,
BRAP_Img_Id_eDtsBroadcast_Decode_Interframe_Stg1,
#endif

#ifdef RAP_LPCMBD_SUPPORT
#endif

#ifdef RAP_LPCMHDDVD_SUPPORT
#endif

#ifdef RAP_LPCMDVD_SUPPORT
BRAP_Img_Id_eLpcmDvd_Ids,
BRAP_Img_Id_eLpcmDvd_Decode,
BRAP_Img_Id_eLpcmDvd_Decode_Table,
BRAP_Img_Id_eLpcmDvd_Interframe,
BRAP_Img_Id_eLpcmDvd_Ids_Interframe,
#endif

#ifdef RAP_WMASTD_SUPPORT
BRAP_Img_Id_eWma_Ids,
BRAP_Img_Id_eWma_Decode,
BRAP_Img_Id_eWma_Decode_Table,
BRAP_Img_Id_eWma_Interframe,
BRAP_Img_Id_eWma_Ids_Interframe,
#endif

#ifdef RAP_AC3LOSSLESS_SUPPORT
#endif

#ifdef RAP_MLP_SUPPORT
#endif

#ifdef RAP_PCM_SUPPORT
BRAP_Img_Id_ePcm_Ids,
BRAP_Img_Id_ePcm_Interframe,
#endif

#ifdef RAP_DTSLBR_SUPPORT
#endif

#ifdef RAP_DDP7_1_SUPPORT
#endif

#ifdef RAP_MPEGMC_SUPPORT
#endif

#ifdef RAP_WMAPRO_SUPPORT
BRAP_Img_Id_eWmaPro_Ids,
BRAP_Img_Id_eWmaPro_Decode_Stg0,
BRAP_Img_Id_eWmaPro_Decode_Stg0_Table,
BRAP_Img_Id_eWmaPro_Decode_Stg1,
BRAP_Img_Id_eWmaPro_Decode_Stg1_Table,
BRAP_Img_Id_eWmaPro_Ids_Interframe,
BRAP_Img_Id_eWmaPro_Stg0_Interframe,
BRAP_Img_Id_eWmaPro_Stg1_Interframe,
#endif


#ifdef RAP_DDBM_SUPPORT
BRAP_Img_Id_eDdbm_Code,
BRAP_Img_Id_eDdbm_Table,
#endif

#ifdef RAP_DTSNEO_SUPPORT
BRAP_Img_Id_eDtsNeo_Code,
BRAP_Img_Id_eDtsNeo_Table,
BRAP_Img_Id_eDtsNeo_Interframe,
#endif

#ifdef RAP_AVL_SUPPORT
BRAP_Img_Id_eAvl_Code,
BRAP_Img_Id_eAvl_Table,
BRAP_Img_Id_eAvl_Interframe,
#endif

#ifdef RAP_PL2_SUPPORT
BRAP_Img_Id_ePL2_Code,
BRAP_Img_Id_ePL2_Table,
BRAP_Img_Id_ePL2_Interframe,
#endif

#ifdef RAP_SRSXT_SUPPORT
BRAP_Img_Id_eSrsXt_Code,
BRAP_Img_Id_eSrsXt_Table,
BRAP_Img_Id_eSrsXt_Interframe,
#endif

#ifdef RAP_SRSHD_SUPPORT
BRAP_Img_Id_eSrsHd_Code,
BRAP_Img_Id_eSrsHd_Table,
BRAP_Img_Id_eSrsHd_Interframe,
#endif

#ifdef RAP_SRSTRUVOL_SUPPORT
BRAP_Img_Id_eSrsTruVolume_Code,
BRAP_Img_Id_eSrsTruVolume_Table,
BRAP_Img_Id_eSrsTruVolume_Interframe,
#endif

#ifdef RAP_XEN_SUPPORT
BRAP_Img_Id_eXen_Code,
BRAP_Img_Id_eXen_Table,
BRAP_Img_Id_eXen_Interframe,
#endif

#ifdef RAP_BBE_SUPPORT
BRAP_Img_Id_eBbe_Code,
BRAP_Img_Id_eBbe_Table,
BRAP_Img_Id_eBbe_Interframe,
#endif

#ifdef RAP_CUSTOMSURROUND_SUPPORT
BRAP_Img_Id_eCustomSurround_Code,
BRAP_Img_Id_eCustomSurround_Table,
BRAP_Img_Id_eCustomSurround_Interframe,
#endif

#ifdef RAP_CUSTOMBASS_SUPPORT
BRAP_Img_Id_eCustomBass_Code,
BRAP_Img_Id_eCustomBass_Table,
BRAP_Img_Id_eCustomBass_Interframe,
#endif

#ifdef RAP_AACDOWNMIX_SUPPORT
#endif


#ifdef RAP_DTSENC_SUPPORT
BRAP_Img_Id_eDts_Encode_Code,
BRAP_Img_Id_eDts_Encode_Table,
BRAP_Img_Id_eDts_Encode_Interframe,
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
BRAP_Img_Id_eDolbyTranscode_Code,
BRAP_Img_Id_eDolbyTranscode_Table,
BRAP_Img_Id_eDolbyTranscode_Interframe,
#else
#ifdef RAP_AC3ENC_SUPPORT
BRAP_Img_Id_eAc3_Encode_Code,
BRAP_Img_Id_eAc3_Encode_Table,
BRAP_Img_Id_eAc3_Encode_Interframe,
#endif
#endif

#ifdef RAP_WMAPROPASSTHRU_SUPPORT
BRAP_Img_Id_eWmaPro_Passthru_Code,
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
BRAP_Img_Id_eMsDdp_Convert,
BRAP_Img_Id_eMsDdp_Convert_Tables,
BRAP_Img_Id_eMsDdp_Convert_Interframe,
#else
#ifdef RAP_DDP_TO_AC3_SUPPORT
BRAP_Img_Id_eDdp_Convert,
BRAP_Img_Id_eDdp_Convert_Tables,
BRAP_Img_Id_eDdp_Convert_Interframe,
#endif
#endif

#ifdef RAP_CUSTOMVOICE_SUPPORT
BRAP_Img_Id_eCustomVoice_Code,
BRAP_Img_Id_eCustomVoice_Tables,
BRAP_Img_Id_eCustomVoice_Interframe,
#endif
#ifdef RAP_SRC_SUPPORT
BRAP_Img_Id_eSrc_Code,
BRAP_Img_Id_eSrc_Tables,
BRAP_Img_Id_eSrc_Interframe,
#endif
#ifdef RAP_RFAUDIO_SUPPORT
BRAP_Img_Id_eRfAudio_Btsc,
BRAP_Img_Id_eRfAudio_Eiaj,
BRAP_Img_Id_eRfAudio_KoreaA2,
BRAP_Img_Id_eRfAudio_Nicam,
BRAP_Img_Id_eRfAudio_PalA2,
BRAP_Img_Id_eRfAudio_SecamL,
BRAP_Img_Id_eRfAudio_India,
#endif
#ifdef RAP_AUDIODESC_SUPPORT
BRAP_Img_Id_eFadeControl_Code,
/*BRAP_Img_Id_eFadeControl_Tables,
BRAP_Img_Id_eFadeControl_Interframe,*/
BRAP_Img_Id_ePanControl_Code,
BRAP_Img_Id_ePanControl_Tables,
BRAP_Img_Id_ePanControl_Interframe,
#endif
#ifdef RAP_PCMROUTER_SUPPORT
BRAP_Img_Id_ePCMRouter_Code,
#endif
#ifdef RAP_DSOLA_SUPPORT
BRAP_Img_Id_eDsola_Code,
BRAP_Img_Id_eDsola_Tables,
BRAP_Img_Id_eDsola_Interframe,
#endif
#ifdef RAP_DOLBYVOL_SUPPORT
BRAP_Img_Id_eDolbyVol_Code,
BRAP_Img_Id_eDolbyVol_Tables,
BRAP_Img_Id_eDolbyVol_Interframe,
#endif

#ifdef RAP_PCMWAV_SUPPORT
BRAP_Img_Id_ePcmWav_Code,
BRAP_Img_Id_ePcmWav_Code_Interframe,
BRAP_Img_Id_ePcmWav_Ids,
BRAP_Img_Id_ePcmWav_Ids_Interframe,
#endif

#ifdef RAP_MP3ENC_SUPPORT
BRAP_Img_Id_eMp3Enc_Code,
BRAP_Img_Id_eMp3Enc_Code_Tables,
BRAP_Img_Id_eMp3Enc_Code_Interframe,
#endif

#ifdef RAP_AMR_SUPPORT
BRAP_Img_Id_eAMR_Ids,
BRAP_Img_Id_eAMR_Decode,
BRAP_Img_Id_eAMR_Decode_Table,
BRAP_Img_Id_eAMR_Interframe,
BRAP_Img_Id_eAMR_Ids_Interframe,
#endif

#ifdef RAP_DRA_SUPPORT
BRAP_Img_Id_eDRA_Decode,
BRAP_Img_Id_eDRA_Decode_Table,
BRAP_Img_Id_eDRA_Interframe,
#endif

#ifdef RAP_SBCENC_SUPPORT
BRAP_Img_Id_eSbc_Encode_Code,
BRAP_Img_Id_eSbc_Encode_Table,
BRAP_Img_Id_eSbc_Encode_Interframe,
#endif

#ifdef  RAP_REALAUDIOLBR_SUPPORT
BRAP_Img_Id_eRealAudioLbr_Decode_Code,
BRAP_Img_Id_eRealAudioLbr_Decode_Table,
BRAP_Img_Id_eRealAudioLbr_Decode_Interframe,
#endif

#ifdef RAP_BRCM3DSURROUND_SUPPORT
BRAP_Img_Id_eBrcm3DSurround_Code,
BRAP_Img_Id_eBrcm3DSurround_Table,
BRAP_Img_Id_eBrcm3DSurround_Interframe,
#endif

#ifdef RAP_MONODOWNMIX_SUPPORT
BRAP_Img_Id_eMonoDownmix_Code,
#endif

#ifdef RAP_FWMIXER_SUPPORT
BRAP_Img_Id_eFwMixer_Ids,
BRAP_Img_Id_eFwMixer_Code,
BRAP_Img_Id_eFwMixer_Table,
BRAP_Img_Id_eFwMixer_Interframe,
BRAP_Img_Id_eFwMixer_Ids_Interframe,
#endif

#ifdef RAP_ADV_SUPPORT
BRAP_Img_Id_eAvd_Code,
BRAP_Img_Id_eAvd_Table,
BRAP_Img_Id_eAvd_Interframe, 
#endif

#ifdef RAP_ABX_SUPPORT
BRAP_Img_Id_eAbx_Code,
BRAP_Img_Id_eAbx_Table,
BRAP_Img_Id_eAbx_Interframe, 
#endif

#ifdef RAP_SRSCSTD_SUPPORT
BRAP_Img_Id_eSrsCsTd_Code,
BRAP_Img_Id_eSrsCsTd_Table,
BRAP_Img_Id_eSrsCsTd_Interframe,
#endif

#ifdef RAP_SRSEQHL_SUPPORT
BRAP_Img_Id_eSrsEqHl_Code,
BRAP_Img_Id_eSrsEqHl_Table,
BRAP_Img_Id_eSrsEqHl_Interframe,
#endif

#ifdef RAP_DV258_SUPPORT
BRAP_Img_Id_eDV258_Code,
BRAP_Img_Id_eDV258_Table,
BRAP_Img_Id_eDV258_Interframe, 
#endif
#ifdef RAP_DDRE_SUPPORT
BRAP_Img_Id_eDDRE_Code,
BRAP_Img_Id_eDDRE_Table,
BRAP_Img_Id_eDDRE_Interframe, 
#endif



#ifdef RAP_GFX_SUPPORT
BRAP_Img_Id_egfx_Code,
BRAP_Img_Id_egfx_Table,
BRAP_Img_Id_egfx_Interframe,
#endif
#ifdef RAP_REALVIDEO_SUPPORT
BRAP_Img_Id_eRealVideo_Decode_Stg1,
BRAP_Img_Id_eRealVideo_Decode_Stg2,
BRAP_Img_Id_eRealVideo_Decode_Stg3,
BRAP_Img_Id_eRealVideo_Decode_Stg1_Table,
BRAP_Img_Id_eRealVideo_Decode_Stg2_Table,
BRAP_Img_Id_eRealVideo_Decode_Stg3_Table,
BRAP_Img_Id_eRealVideo_Ppd,
BRAP_Img_Id_eRealVideo_Plf,
BRAP_Img_Id_eRealVideo_Decode_Interframe,
BRAP_Img_Id_eRealVideo_Plf_Table,
#endif
#ifdef RAP_BTSC_SUPPORT
BRAP_Img_Id_eBtsc_Code,
BRAP_Img_Id_eBtsc_Table,
BRAP_Img_Id_eBtsc_Interframe,
#endif
BRAP_Img_Id_eInvalid
}BRAP_Img_Id;

#define BRAP_MAX_IMG_ID BRAP_Img_Id_eInvalid

/* 
	This chunk size will be used when the firmware binary is actually present in multiple chunks.
	The BRAP_IMG_CHUNK_SIZE will then give the size of each such chunk
*/
#define BRAP_IMG_CHUNK_SIZE	65532


extern void *BRAP_IMG_Context;
extern const BIMG_Interface BRAP_IMG_Interface;

#endif /* BRAP_IMG_H__ */


