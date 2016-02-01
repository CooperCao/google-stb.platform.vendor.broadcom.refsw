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
*
* Module Description:
*	This file is part of image interface implementation for RAP PI.
*
* Revision History:
*
* $brcm_Log: $
* 
 ***************************************************************************/

#include "bstd.h"
#include "brap_img.h"


BDBG_MODULE(rap_img);

/* External references of firmware binaries */
extern const void * BRAP_IMG_system_iboot [];
extern const void * BRAP_IMG_system_data [];
extern const void * BRAP_IMG_system_code [];
extern const void * BRAP_IMG_system_rdbvars [];
 
extern const void * BRAP_IMG_dec_tsm_code [];
extern const void * BRAP_IMG_dec_tsm_inter_frame[];
/*extern const void * BRAP_IMG_encode_tsm [];
extern const void * BRAP_IMG_passthru_tsm [];*/
extern const void *	BRAP_IMG_cdb_passthru_code[];
extern const void *	BRAP_IMG_cdb_passthru_tables[];
extern const void *BRAP_IMG_cdb_passthru_inter_frame[];

extern const void * BRAP_IMG_system_audio_decode_task_code [];
extern const void * BRAP_IMG_system_gfx_task_code [];
extern const void * BRAP_IMG_system_video_task_code [];



/* Passthru AC3/MPEG/DDP/DTS/AAC even without license */
#ifndef RAP_MULTISTREAM_DECODER_SUPPORT
#ifdef  RAP_AC3_PASSTHRU_SUPPORT         
extern const void * BRAP_IMG_ac3_ids [];
extern const void * BRAP_IMG_ac3_ids_inter_frame [];
#endif
#endif

#ifdef  RAP_MPEG_PASSTHRU_SUPPORT         
extern const void * BRAP_IMG_mpeg1_ids [];
extern const void * BRAP_IMG_mpeg1_ids_inter_frame [];
#endif

#ifdef RAP_AACSBR_PASSTHRU_SUPPORT
extern const void * BRAP_IMG_aache_adts_ids [];
extern const void * BRAP_IMG_aache_loas_ids [];
extern const void * BRAP_IMG_aache_adts_ids_inter_frame [];
extern const void * BRAP_IMG_aache_loas_ids_inter_frame [];
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
extern const void * BRAP_IMG_dolby_ms_ddp_ids [];
extern const void * BRAP_IMG_dolby_ms_ddp_ids_inter_frame [];
#else
#ifdef  RAP_DDP_PASSTHRU_SUPPORT         
extern const void * BRAP_IMG_ddp_ids [];
extern const void * BRAP_IMG_ddp_ids_inter_frame [];
#endif
#endif
#ifdef  RAP_DTSBROADCAST_PASSTHRU_SUPPORT         
extern const void * BRAP_IMG_dts_ids [];
extern const void * BRAP_IMG_dts_ids_inter_frame [];
#endif

#ifdef  RAP_DTSHD_PASSTHRU_SUPPORT
extern const void * BRAP_IMG_dtshd_ids [];
extern const void * BRAP_IMG_dtshd_ids_inter_frame [];
#endif

#ifdef  RAP_DRA_PASSTHRU_SUPPORT
extern const void * BRAP_IMG_dra_ids[];
extern const void * BRAP_IMG_dra_ids_inter_frame[];
#endif

#ifdef  RAP_REALAUDIOLBR_PASSTHRU_SUPPORT
extern const void * BRAP_IMG_ralbr_ids [];
extern const void * BRAP_IMG_ralbr_ids_inter_frame [];
#endif






#ifdef RAP_MPEG_SUPPORT
extern const void * BRAP_IMG_mpeg1_decode [];
extern const void * BRAP_IMG_mpeg1_decode_table [];
extern const void * BRAP_IMG_mpeg1_decode_inter_frame [];
#endif

#ifdef RAP_AAC_SUPPORT
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
extern const void * BRAP_IMG_dolby_pulse_decode_stg0 [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg1 [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg2 [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg3 [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg0_tables [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg1_tables [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg2_tables [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg3_tables [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg0_inter_frame [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg1_inter_frame [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg2_inter_frame [];
extern const void * BRAP_IMG_dolby_pulse_decode_stg3_inter_frame [];
#else
#ifdef RAP_AACSBR_SUPPORT
extern const void * BRAP_IMG_aache_decode_stg0 [];
extern const void * BRAP_IMG_aache_decode_stg1 [];
extern const void * BRAP_IMG_aache_decode_stg2 [];
extern const void * BRAP_IMG_aache_decode_stg3 [];
extern const void * BRAP_IMG_aache_decode_stg4 [];
extern const void * BRAP_IMG_aache_decode_stg0_tables [];
extern const void * BRAP_IMG_aache_decode_stg1_tables [];
extern const void * BRAP_IMG_aache_decode_stg2_tables [];
extern const void * BRAP_IMG_aache_decode_stg3_tables [];
extern const void * BRAP_IMG_aache_decode_stg4_tables [];
extern const void * BRAP_IMG_aache_decode_stg0_inter_frame [];
extern const void * BRAP_IMG_aache_decode_stg1_inter_frame [];
extern const void * BRAP_IMG_aache_decode_stg2_inter_frame [];
extern const void * BRAP_IMG_aache_decode_stg3_inter_frame [];
extern const void * BRAP_IMG_aache_decode_stg4_inter_frame [];
#endif
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
extern const void * BRAP_IMG_dolby_ms_ddp_be_decode [];
extern const void * BRAP_IMG_dolby_ms_ddp_fe_decode [];
extern const void * BRAP_IMG_dolby_ms_ddp_be_decode_tables [];
extern const void * BRAP_IMG_dolby_ms_ddp_fe_decode_tables [];
extern const void * BRAP_IMG_dolby_ms_ddp_be_decode_inter_frame [];
extern const void * BRAP_IMG_dolby_ms_ddp_fe_decode_inter_frame [];
#else
#ifdef RAP_AC3_SUPPORT
/*extern const void * BRAP_IMG_ac3_ids [];*/
extern const void * BRAP_IMG_ac3_be_decode [];
extern const void * BRAP_IMG_ac3_fe_decode [];
extern const void * BRAP_IMG_ac3_be_decode_tables [];
extern const void * BRAP_IMG_ac3_fe_decode_tables [];
extern const void * BRAP_IMG_ac3_be_decode_inter_frame [];
extern const void * BRAP_IMG_ac3_fe_decode_inter_frame [];
/*extern const void * BRAP_IMG_ac3_ids_inter_frame [];*/
#endif

#ifdef RAP_DDP_SUPPORT
extern const void * BRAP_IMG_ddp_be_decode [];
extern const void * BRAP_IMG_ddp_fe_decode [];
extern const void * BRAP_IMG_ddp_be_decode_tables [];
extern const void * BRAP_IMG_ddp_fe_decode_tables [];
extern const void * BRAP_IMG_ddp_be_decode_inter_frame [];
extern const void * BRAP_IMG_ddp_fe_decode_inter_frame [];
#endif
#endif

#ifdef RAP_DTS_SUPPORT
extern const void * BRAP_IMG_dts_ids [];
extern const void * BRAP_IMG_dts_ids_inter_frame [];
#endif

#if defined(RAP_DTSBROADCAST_SUPPORT) || defined(RAP_DTSHD_SUPPORT)
extern const void * BRAP_IMG_dtshd_dec_stg0 [];
extern const void * BRAP_IMG_dtshd_dec_stg0_tables [];
extern const void * BRAP_IMG_dtshd_dec_stg0_inter_frame [];
extern const void * BRAP_IMG_dtshd_dec_stg1 [];
extern const void * BRAP_IMG_dtshd_dec_stg1_tables [];
extern const void * BRAP_IMG_dtshd_dec_stg1_inter_frame [];
#endif

#ifdef RAP_LPCMBD_SUPPORT
#endif

#ifdef RAP_LPCMHDDVD_SUPPORT
#endif

#ifdef RAP_LPCMDVD_SUPPORT
extern const void *   BRAP_IMG_dvdlpcm_ids [];
extern const void *   BRAP_IMG_lpcm_decode_code [];
extern const void *   BRAP_IMG_lpcm_decode_tables [];
extern const void *   BRAP_IMG_lpcm_decode_inter_frame [];
extern const void *   BRAP_IMG_dvdlpcm_ids_inter_frame [];
#endif

#ifdef RAP_WMASTD_SUPPORT
extern const void *  BRAP_IMG_wma_ids [];
extern const void *  BRAP_IMG_wma_decode [];
extern const void *  BRAP_IMG_wma_decode_table [];
extern const void *  BRAP_IMG_wma_decode_interframe [];
extern const void *  BRAP_IMG_wma_ids_interframe [];
#endif

#ifdef RAP_AC3LOSSLESS_SUPPORT
#endif

#ifdef RAP_MLP_SUPPORT
 
#endif

#ifdef RAP_PCM_SUPPORT
extern const void *  BRAP_IMG_pcm_ids [];
extern const void *  BRAP_IMG_pcm_ids_inter_frame [];
#endif

#ifdef RAP_DTSLBR_SUPPORT
#endif

#ifdef RAP_DDP7_1_SUPPORT
#endif

#ifdef RAP_MPEGMC_SUPPORT
#endif

#ifdef RAP_WMAPRO_SUPPORT
extern const void *  BRAP_IMG_wmapro_ids [];
extern const void *  BRAP_IMG_wmapro_decode_stg0 [];
extern const void *  BRAP_IMG_wmapro_decode_stg0_tables [];
extern const void *  BRAP_IMG_wmapro_decode_stg1 [];
extern const void *  BRAP_IMG_wmapro_decode_stg1_tables [];
extern const void *  BRAP_IMG_wmapro_ids_inter_frame []; 
extern const void *  BRAP_IMG_wmapro_decode_stg0_inter_frame []; 
extern const void *  BRAP_IMG_wmapro_decode_stg1_inter_frame [];
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
extern const void * BRAP_IMG_dolby_ms_ddp_convert [];
extern const void * BRAP_IMG_dolby_ms_ddp_convert_tables [];
extern const void * BRAP_IMG_dolby_ms_ddp_convert_inter_frame [];
#else
#ifdef RAP_DDP_TO_AC3_SUPPORT
extern const void * BRAP_IMG_ddp_convert [];
extern const void * BRAP_IMG_ddp_convert_tables [];
extern const void * BRAP_IMG_ddp_convert_inter_frame [];
#endif
#endif

#ifdef RAP_DDBM_SUPPORT
extern const void * BRAP_IMG_ddbm_postprocess[];
extern const void * BRAP_IMG_ddbm_postprocess_table[];
#endif

#ifdef RAP_DTSNEO_SUPPORT
extern const void * BRAP_IMG_dts_neo_postprocess[];
extern const void * BRAP_IMG_dts_neo_postprocess_table[];
extern const void * BRAP_IMG_dts_neo_interframe_buf[];
#endif

#ifdef RAP_AVL_SUPPORT
extern const void *BRAP_IMG_avl[];
extern const void *BRAP_IMG_avl_tables[];
extern const void *BRAP_IMG_avl_inter_frame[];
#endif

#ifdef RAP_PL2_SUPPORT
extern const void *BRAP_IMG_prologic2_code[];
extern const void *BRAP_IMG_prologic2_tables[];
extern const void *BRAP_IMG_prologic2_inter_frame[];
#endif

#ifdef RAP_SRSXT_SUPPORT
extern const void *BRAP_IMG_trusurroundxt[];
extern const void *BRAP_IMG_trusurroundxt_tables[];
extern const void *BRAP_IMG_trusurroundxt_inter_frame[];
#endif

#ifdef RAP_SRSHD_SUPPORT
extern const void *BRAP_IMG_trusurroundhd[];
extern const void *BRAP_IMG_trusurroundhd_tables[];
extern const void *BRAP_IMG_trusurroundhd_inter_frame[];
#endif

#ifdef RAP_SRSTRUVOL_SUPPORT
extern const void *BRAP_IMG_srs_truvolume_post_process[];
extern const void *BRAP_IMG_srs_truvolume_table[];
extern const void *BRAP_IMG_srs_truvolume_interframe_buf[];
#endif

#ifdef RAP_XEN_SUPPORT
extern const void *BRAP_IMG_xen[];
extern const void *BRAP_IMG_xen_table[];
extern const void *BRAP_IMG_xen_interframe_buf[];
#endif

#ifdef RAP_BBE_SUPPORT
extern const void *BRAP_IMG_bbe[];
extern const void *BRAP_IMG_bbe_table[];
extern const void *BRAP_IMG_bbe_interframe_buf[];
#endif

#ifdef RAP_CUSTOMSURROUND_SUPPORT
extern const void *BRAP_IMG_customsurround_code[];
extern const void *BRAP_IMG_customsurround_tables[];
extern const void *BRAP_IMG_customsurround_inter_frame[];
#endif

#ifdef RAP_CUSTOMBASS_SUPPORT
extern const void *BRAP_IMG_custombass_code[];
extern const void *BRAP_IMG_custombass_tables[];
extern const void *BRAP_IMG_custombass_inter_frame[];
#endif

#ifdef RAP_AACDOWNMIX_SUPPORT
#endif

#ifdef RAP_DTSENC_SUPPORT
extern const void * BRAP_IMG_dts_broadcast_encode [];
extern const void * BRAP_IMG_dts_broadcast_encode_tables [];
extern const void * BRAP_IMG_dts_broadcast_encode_inter_frame [];
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
extern const void * BRAP_IMG_dd_transcode_code [];
extern const void * BRAP_IMG_dd_transcode_tables [];
extern const void * BRAP_IMG_dd_transcode_inter_frame [];
#else
#ifdef RAP_AC3ENC_SUPPORT
extern const void * BRAP_IMG_ac3_encode [];
extern const void * BRAP_IMG_ac3_encode_tables [];
extern const void * BRAP_IMG_ac3_encode_inter_frame [];
#endif
#endif

#ifdef RAP_CUSTOMVOICE_SUPPORT
extern const void * BRAP_IMG_customvoice_code [];
extern const void * BRAP_IMG_customvoice_tables [];
extern const void * BRAP_IMG_customvoice_inter_frame [];
#endif

#ifdef RAP_SRC_SUPPORT
extern const void * BRAP_IMG_src_PP [];
extern const void * BRAP_IMG_src_PP_table [];
extern const void * BRAP_IMG_src_PP_inter_frame [];
#endif

#ifdef RAP_RFAUDIO_SUPPORT
extern const void * BRAP_IMG_RfAudioBtsc[];
extern const void * BRAP_IMG_RfAudioEiaj[];
extern const void * BRAP_IMG_RfAudioKoreaa2[];
extern const void * BRAP_IMG_RfAudioNicam[];
extern const void * BRAP_IMG_RfAudioPala2[];
extern const void * BRAP_IMG_RfAudioSecaml[];
extern const void * BRAP_IMG_RfAudioIndia[];
#endif

#ifdef RAP_AUDIODESC_SUPPORT
extern const void * BRAP_IMG_fade_control[];
/*extern const void * BRAP_IMG_fade_control_interframe_buf[];
extern const void * BRAP_IMG_fade_control_tables[];*/
extern const void * BRAP_IMG_pan_control[];
extern const void * BRAP_IMG_pan_control_interframe_buf[];
extern const void * BRAP_IMG_pan_control_tables[];
#endif

#ifdef RAP_WMAPROPASSTHRU_SUPPORT
extern const void *  BRAP_IMG_wmapro_passthru_code[];
#endif
#ifdef RAP_PCMROUTER_SUPPORT
extern const void * BRAP_IMG_pcm_router[];
#endif
#ifdef RAP_DSOLA_SUPPORT
extern const void * BRAP_IMG_dsola[];
extern const void * BRAP_IMG_dsola_tables[];
extern const void * BRAP_IMG_dsola_inter_frame[];
#endif

#ifdef RAP_DOLBYVOL_SUPPORT
extern const void * BRAP_IMG_dolby_volume[];
extern const void * BRAP_IMG_dolby_volume_table[];
extern const void * BRAP_IMG_dolby_volume_inter_frame[];
#endif


#ifdef RAP_PCMWAV_SUPPORT
extern const void * BRAP_IMG_pcmwav_decode_code[];
extern const void * BRAP_IMG_pcmwav_decode_inter_frame[];
extern const void * BRAP_IMG_pcmwav_ids[];
extern const void * BRAP_IMG_pcmwav_ids_inter_frame[];
#endif


#ifdef RAP_MP3ENC_SUPPORT
extern const void * BRAP_IMG_mp3_encode_code[];
extern const void * BRAP_IMG_mp3_encode_tables[];
extern const void * BRAP_IMG_mp3_encode_inter_frame[];
#endif

#ifdef RAP_AMR_SUPPORT
extern const void * BRAP_IMG_amr_ids[];
extern const void * BRAP_IMG_amr_decode[];
extern const void * BRAP_IMG_amr_decode_tables[];
extern const void * BRAP_IMG_amr_decode_inter_frame[];
extern const void * BRAP_IMG_amr_ids_inter_frame[];
#endif

#ifdef RAP_DRA_SUPPORT
extern const void * BRAP_IMG_dra_decode[];
extern const void * BRAP_IMG_dra_decode_table[];
extern const void * BRAP_IMG_dra_interframe_buf[];
#endif

#ifdef RAP_SBCENC_SUPPORT
extern const void * BRAP_IMG_sbc_encode [];
extern const void * BRAP_IMG_sbc_encode_table [];
extern const void * BRAP_IMG_sbc_encode_inter_frame [];
#endif

#ifdef  RAP_REALAUDIOLBR_SUPPORT
extern const void * BRAP_IMG_ralbr_decode [];
extern const void * BRAP_IMG_ralbr_decode_table [];
extern const void * BRAP_IMG_ralbr_interframe_buf [];
#endif

#ifdef RAP_BRCM3DSURROUND_SUPPORT
extern const void * BRAP_IMG_brcm3dsurround [];
extern const void * BRAP_IMG_brcm3dsurround_tables [];
extern const void * BRAP_IMG_brcm3dsurround_inter_frame [];
#endif

#ifdef RAP_MONODOWNMIX_SUPPORT
extern const void * BRAP_IMG_monodownmix [];
#endif

#ifdef RAP_FWMIXER_SUPPORT
extern const void * BRAP_IMG_fw_mixer_ids [];
extern const void * BRAP_IMG_fw_mixer_code [];
extern const void * BRAP_IMG_fw_mixer_tables [];
extern const void * BRAP_IMG_fw_mixer_inter_frame [];
extern const void * BRAP_IMG_fw_mixer_ids_inter_frame[];
#endif

#ifdef RAP_ADV_SUPPORT
extern const void * BRAP_IMG_adv_code [];
extern const void * BRAP_IMG_adv_tables [];
extern const void * BRAP_IMG_adv_inter_frame [];    
#endif

#ifdef RAP_ABX_SUPPORT
extern const void * BRAP_IMG_abx_code [];
extern const void * BRAP_IMG_abx_tables [];
extern const void * BRAP_IMG_abx_inter_frame [];    
#endif

#ifdef RAP_SRSCSTD_SUPPORT
extern const void * BRAP_IMG_srs_cs_td_code [];
extern const void * BRAP_IMG_srs_cs_td_tables [];
extern const void * BRAP_IMG_srs_cs_td_interframe [];    
#endif

#ifdef RAP_SRSEQHL_SUPPORT
extern const void * BRAP_IMG_srs_eq_hl_code [];
extern const void * BRAP_IMG_srs_eq_hl_tables [];
extern const void * BRAP_IMG_srs_eq_hl_interframe [];    
#endif

#ifdef RAP_DV258_SUPPORT
    extern const void * BRAP_IMG_dv258_code [];
    extern const void * BRAP_IMG_dv258_tables [];
    extern const void * BRAP_IMG_dv258_inter_frame [];    
#endif

#ifdef RAP_DDRE_SUPPORT
    extern const void * BRAP_IMG_dd_reenc_code [];
    extern const void * BRAP_IMG_dd_reenc_tables [];
    extern const void * BRAP_IMG_dd_reenc_inter_frame [];    
#endif


#ifdef RAP_GFX_SUPPORT
extern const void * BRAP_IMG_gfx_decode [];
extern const void * BRAP_IMG_gfx_decode_tables [];
extern const void * BRAP_IMG_gfx_decode_interframe [];    
#endif

#ifdef RAP_REALVIDEO_SUPPORT
extern const void * BRAP_IMG_rv40_decode_stg1[];
extern const void * BRAP_IMG_rv40_decode_stg2[];
extern const void * BRAP_IMG_rv40_decode_stg3[];
extern const void * BRAP_IMG_rv40_decode_stg1_tables[];
extern const void * BRAP_IMG_rv40_decode_stg2_tables[];
extern const void * BRAP_IMG_rv40_decode_stg3_tables[];
extern const void * BRAP_IMG_rv40_decode_ppd[];
extern const void * BRAP_IMG_rv40_decode_plf[];
extern const void * BRAP_IMG_rv40_decode_plf_tables[];
extern const void * BRAP_IMG_rv40_decode_inter_frame[];
#endif

#ifdef RAP_BTSC_SUPPORT
extern const void * BRAP_IMG_btsc_code[];
extern const void * BRAP_IMG_btsc_table[];
extern const void * BRAP_IMG_btsc_inter_frame[];
#endif
/*
	This array will contain pointers to the arrays that contain that chunk pointers of the firmware binary.
	Based on the firmware ID one particular firmware Image can be accessed from this array
*/

static const void **BRAP_IMG_FirmwareImages [] = 
{
   		 BRAP_IMG_system_iboot 
		,BRAP_IMG_system_code 	
		,BRAP_IMG_system_data 
              ,BRAP_IMG_system_rdbvars		
		,BRAP_IMG_dec_tsm_code
              ,BRAP_IMG_dec_tsm_inter_frame		
		,BRAP_IMG_dec_tsm_code
		,BRAP_IMG_dec_tsm_code		
		,BRAP_IMG_cdb_passthru_code
		,BRAP_IMG_cdb_passthru_tables
		,BRAP_IMG_cdb_passthru_inter_frame
            ,BRAP_IMG_system_audio_decode_task_code
            ,BRAP_IMG_system_gfx_task_code
            ,BRAP_IMG_system_video_task_code
#ifndef RAP_MULTISTREAM_DECODER_SUPPORT		
#ifdef  RAP_AC3_PASSTHRU_SUPPORT           		
		,BRAP_IMG_ac3_ids
		,BRAP_IMG_ac3_ids_inter_frame
#endif	
#endif

#ifdef  RAP_MPEG_PASSTHRU_SUPPORT           		
        ,BRAP_IMG_mpeg1_ids		
        ,BRAP_IMG_mpeg1_ids_inter_frame		
#endif            

#ifdef RAP_AACSBR_PASSTHRU_SUPPORT
        ,BRAP_IMG_aache_adts_ids        
        ,BRAP_IMG_aache_loas_ids	
        ,BRAP_IMG_aache_adts_ids_inter_frame        
        ,BRAP_IMG_aache_loas_ids_inter_frame         
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
        ,BRAP_IMG_dolby_ms_ddp_ids            
        ,BRAP_IMG_dolby_ms_ddp_ids_inter_frame   
#else
#ifdef  RAP_DDP_PASSTHRU_SUPPORT                       
        ,BRAP_IMG_ddp_ids            
        ,BRAP_IMG_ddp_ids_inter_frame   
#endif
#endif

#ifdef  RAP_DTSBROADCAST_PASSTHRU_SUPPORT                       
        ,BRAP_IMG_dts_ids
        ,BRAP_IMG_dts_ids_inter_frame    
#endif           

#ifdef  RAP_DTSHD_PASSTHRU_SUPPORT       
        ,BRAP_IMG_dtshd_ids
        ,BRAP_IMG_dtshd_ids_inter_frame
#endif

#ifdef  RAP_DRA_PASSTHRU_SUPPORT            
        ,BRAP_IMG_dra_ids            
        ,BRAP_IMG_dra_ids_inter_frame                        
#endif

#ifdef  RAP_REALAUDIOLBR_PASSTHRU_SUPPORT            
        ,BRAP_IMG_ralbr_ids
        ,BRAP_IMG_ralbr_ids_inter_frame
#endif










#ifdef RAP_MPEG_SUPPORT
		,BRAP_IMG_mpeg1_decode
		,BRAP_IMG_mpeg1_decode_table 
		,BRAP_IMG_mpeg1_decode_inter_frame
#endif

#ifdef RAP_AAC_SUPPORT
#endif

#ifdef  RAP_MULTISTREAM_DECODER_SUPPORT
        ,BRAP_IMG_dolby_pulse_decode_stg0
        ,BRAP_IMG_dolby_pulse_decode_stg1
        ,BRAP_IMG_dolby_pulse_decode_stg2
        ,BRAP_IMG_dolby_pulse_decode_stg3
        ,BRAP_IMG_dolby_pulse_decode_stg0_tables
        ,BRAP_IMG_dolby_pulse_decode_stg1_tables
        ,BRAP_IMG_dolby_pulse_decode_stg2_tables
        ,BRAP_IMG_dolby_pulse_decode_stg3_tables
        ,BRAP_IMG_dolby_pulse_decode_stg0_inter_frame
        ,BRAP_IMG_dolby_pulse_decode_stg1_inter_frame
        ,BRAP_IMG_dolby_pulse_decode_stg2_inter_frame
        ,BRAP_IMG_dolby_pulse_decode_stg3_inter_frame
#else
#ifdef RAP_AACSBR_SUPPORT
        ,BRAP_IMG_aache_decode_stg0
        ,BRAP_IMG_aache_decode_stg1
        ,BRAP_IMG_aache_decode_stg2
        ,BRAP_IMG_aache_decode_stg3
        ,BRAP_IMG_aache_decode_stg4        
        ,BRAP_IMG_aache_decode_stg0_tables
        ,BRAP_IMG_aache_decode_stg1_tables
        ,BRAP_IMG_aache_decode_stg2_tables
        ,BRAP_IMG_aache_decode_stg3_tables
        ,BRAP_IMG_aache_decode_stg4_tables        
        ,BRAP_IMG_aache_decode_stg0_inter_frame
        ,BRAP_IMG_aache_decode_stg1_inter_frame
        ,BRAP_IMG_aache_decode_stg2_inter_frame
        ,BRAP_IMG_aache_decode_stg3_inter_frame
        ,BRAP_IMG_aache_decode_stg4_inter_frame        
#endif
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
		,BRAP_IMG_dolby_ms_ddp_be_decode
		,BRAP_IMG_dolby_ms_ddp_fe_decode		
		,BRAP_IMG_dolby_ms_ddp_be_decode_tables	
		,BRAP_IMG_dolby_ms_ddp_fe_decode_tables			
		,BRAP_IMG_dolby_ms_ddp_be_decode_inter_frame
		,BRAP_IMG_dolby_ms_ddp_fe_decode_inter_frame
#else
#ifdef RAP_AC3_SUPPORT
/*		,BRAP_IMG_ac3_ids*/
		,BRAP_IMG_ac3_be_decode
		,BRAP_IMG_ac3_fe_decode		
		,BRAP_IMG_ac3_be_decode_tables	
		,BRAP_IMG_ac3_fe_decode_tables			
		,BRAP_IMG_ac3_be_decode_inter_frame
		,BRAP_IMG_ac3_fe_decode_inter_frame
/*		,BRAP_IMG_ac3_ids_inter_frame*/
#endif

#ifdef RAP_DDP_SUPPORT
		,BRAP_IMG_ddp_be_decode
		,BRAP_IMG_ddp_fe_decode		
		,BRAP_IMG_ddp_be_decode_tables	
		,BRAP_IMG_ddp_fe_decode_tables			
		,BRAP_IMG_ddp_be_decode_inter_frame
		,BRAP_IMG_ddp_fe_decode_inter_frame
#endif
#endif
#ifdef RAP_DTS_SUPPORT
        ,BRAP_IMG_dts_ids
        ,BRAP_IMG_dts_ids_inter_frame
#endif
#if defined(RAP_DTSBROADCAST_SUPPORT) || defined(RAP_DTSHD_SUPPORT)
        ,BRAP_IMG_dtshd_dec_stg0
        ,BRAP_IMG_dtshd_dec_stg0_tables
        ,BRAP_IMG_dtshd_dec_stg0_inter_frame
        ,BRAP_IMG_dtshd_dec_stg1
        ,BRAP_IMG_dtshd_dec_stg1_tables
        ,BRAP_IMG_dtshd_dec_stg1_inter_frame        
#endif

#ifdef RAP_LPCMBD_SUPPORT
#endif

#ifdef RAP_LPCMHDDVD_SUPPORT
#endif

#ifdef RAP_LPCMDVD_SUPPORT
            ,BRAP_IMG_dvdlpcm_ids
            ,BRAP_IMG_lpcm_decode_code
            ,BRAP_IMG_lpcm_decode_tables
            ,BRAP_IMG_lpcm_decode_inter_frame
            ,BRAP_IMG_dvdlpcm_ids_inter_frame
#endif

#ifdef RAP_WMASTD_SUPPORT
            ,BRAP_IMG_wma_ids
            ,BRAP_IMG_wma_decode
            ,BRAP_IMG_wma_decode_table
            ,BRAP_IMG_wma_decode_interframe
            ,BRAP_IMG_wma_ids_interframe
#endif

#ifdef RAP_AC3LOSSLESS_SUPPORT
#endif

#ifdef RAP_MLP_SUPPORT
#endif

#ifdef RAP_PCM_SUPPORT
            ,BRAP_IMG_pcm_ids
            ,BRAP_IMG_pcm_ids_inter_frame
#endif

#ifdef RAP_DTSLBR_SUPPORT
#endif

#ifdef RAP_DDP7_1_SUPPORT
#endif

#ifdef RAP_MPEGMC_SUPPORT
#endif

#ifdef RAP_WMAPRO_SUPPORT
            ,BRAP_IMG_wmapro_ids
            ,BRAP_IMG_wmapro_decode_stg0
            ,BRAP_IMG_wmapro_decode_stg0_tables
            ,BRAP_IMG_wmapro_decode_stg1
            ,BRAP_IMG_wmapro_decode_stg1_tables            
            ,BRAP_IMG_wmapro_ids_inter_frame            
            ,BRAP_IMG_wmapro_decode_stg0_inter_frame            
            ,BRAP_IMG_wmapro_decode_stg1_inter_frame
#endif


#ifdef RAP_DDBM_SUPPORT
		,BRAP_IMG_ddbm_postprocess
		,BRAP_IMG_ddbm_postprocess_table

#endif

#ifdef RAP_DTSNEO_SUPPORT
        ,BRAP_IMG_dts_neo_postprocess
        ,BRAP_IMG_dts_neo_postprocess_table
        ,BRAP_IMG_dts_neo_interframe_buf
#endif

#ifdef RAP_AVL_SUPPORT
		,BRAP_IMG_avl
		,BRAP_IMG_avl_tables
		,BRAP_IMG_avl_inter_frame
#endif

#ifdef RAP_PL2_SUPPORT
		,BRAP_IMG_prologic2_code
		,BRAP_IMG_prologic2_tables
		,BRAP_IMG_prologic2_inter_frame
#endif

#ifdef RAP_SRSXT_SUPPORT
		,BRAP_IMG_trusurroundxt
		,BRAP_IMG_trusurroundxt_tables
		,BRAP_IMG_trusurroundxt_inter_frame
#endif

#ifdef RAP_SRSHD_SUPPORT
		,BRAP_IMG_trusurroundhd
		,BRAP_IMG_trusurroundhd_tables
		,BRAP_IMG_trusurroundhd_inter_frame
#endif

#ifdef RAP_SRSTRUVOL_SUPPORT
        ,BRAP_IMG_srs_truvolume_post_process
        ,BRAP_IMG_srs_truvolume_table
        ,BRAP_IMG_srs_truvolume_interframe_buf
#endif

#ifdef RAP_XEN_SUPPORT
		,BRAP_IMG_xen
		,BRAP_IMG_xen_table
		,BRAP_IMG_xen_interframe_buf
#endif

#ifdef RAP_BBE_SUPPORT
		,BRAP_IMG_bbe
		,BRAP_IMG_bbe_table
		,BRAP_IMG_bbe_interframe_buf
#endif

#ifdef RAP_CUSTOMSURROUND_SUPPORT
		,BRAP_IMG_customsurround_code
		,BRAP_IMG_customsurround_tables
		,BRAP_IMG_customsurround_inter_frame
#endif

#ifdef RAP_CUSTOMBASS_SUPPORT
		,BRAP_IMG_custombass_code
		,BRAP_IMG_custombass_tables
		,BRAP_IMG_custombass_inter_frame
#endif

#ifdef RAP_AACDOWNMIX_SUPPORT
#endif

#ifdef RAP_DTSENC_SUPPORT
		,BRAP_IMG_dts_broadcast_encode
		,BRAP_IMG_dts_broadcast_encode_tables
		,BRAP_IMG_dts_broadcast_encode_inter_frame
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
		,BRAP_IMG_dd_transcode_code
		,BRAP_IMG_dd_transcode_tables
		,BRAP_IMG_dd_transcode_inter_frame
#else
#ifdef RAP_AC3ENC_SUPPORT
		,BRAP_IMG_ac3_encode
		,BRAP_IMG_ac3_encode_tables
		,BRAP_IMG_ac3_encode_inter_frame
#endif
#endif

#ifdef RAP_WMAPROPASSTHRU_SUPPORT
            ,BRAP_IMG_wmapro_passthru_code        
#endif

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
		,BRAP_IMG_dolby_ms_ddp_convert
		,BRAP_IMG_dolby_ms_ddp_convert_tables
		,BRAP_IMG_dolby_ms_ddp_convert_inter_frame
#else
#ifdef RAP_DDP_TO_AC3_SUPPORT
		,BRAP_IMG_ddp_convert
		,BRAP_IMG_ddp_convert_tables
		,BRAP_IMG_ddp_convert_inter_frame
#endif
#endif

#ifdef RAP_CUSTOMVOICE_SUPPORT
        ,BRAP_IMG_customvoice_code
        ,BRAP_IMG_customvoice_tables
        ,BRAP_IMG_customvoice_inter_frame
#endif

#ifdef RAP_SRC_SUPPORT
        ,BRAP_IMG_src_PP
        ,BRAP_IMG_src_PP_table
        ,BRAP_IMG_src_PP_inter_frame
#endif

#ifdef RAP_RFAUDIO_SUPPORT
        ,BRAP_IMG_RfAudioBtsc
        ,BRAP_IMG_RfAudioEiaj
        ,BRAP_IMG_RfAudioKoreaa2
        ,BRAP_IMG_RfAudioNicam
        ,BRAP_IMG_RfAudioPala2
        ,BRAP_IMG_RfAudioSecaml
        ,BRAP_IMG_RfAudioIndia
#endif

#ifdef RAP_AUDIODESC_SUPPORT
        ,BRAP_IMG_fade_control
/*        ,BRAP_IMG_fade_control_tables        
        ,BRAP_IMG_fade_control_interframe_buf*/
        ,BRAP_IMG_pan_control
        ,BRAP_IMG_pan_control_tables
        ,BRAP_IMG_pan_control_interframe_buf        
#endif
#ifdef RAP_PCMROUTER_SUPPORT
        ,BRAP_IMG_pcm_router
#endif
#ifdef RAP_DSOLA_SUPPORT
        ,BRAP_IMG_dsola
        ,BRAP_IMG_dsola_tables
        ,BRAP_IMG_dsola_inter_frame
#endif
#ifdef RAP_DOLBYVOL_SUPPORT
        ,BRAP_IMG_dolby_volume
        ,BRAP_IMG_dolby_volume_table
        ,BRAP_IMG_dolby_volume_inter_frame
#endif
#ifdef RAP_PCMWAV_SUPPORT
        ,BRAP_IMG_pcmwav_decode_code
        ,BRAP_IMG_pcmwav_decode_inter_frame
        ,BRAP_IMG_pcmwav_ids
        ,BRAP_IMG_pcmwav_ids_inter_frame
#endif
#ifdef RAP_MP3ENC_SUPPORT
        ,BRAP_IMG_mp3_encode_code
        ,BRAP_IMG_mp3_encode_tables
        ,BRAP_IMG_mp3_encode_inter_frame
#endif
#ifdef RAP_AMR_SUPPORT
		,BRAP_IMG_amr_ids
		,BRAP_IMG_amr_decode
		,BRAP_IMG_amr_decode_tables
		,BRAP_IMG_amr_decode_inter_frame
		,BRAP_IMG_amr_ids_inter_frame
#endif
#ifdef RAP_DRA_SUPPORT
		,BRAP_IMG_dra_decode
		,BRAP_IMG_dra_decode_table
		,BRAP_IMG_dra_interframe_buf
#endif
#ifdef RAP_SBCENC_SUPPORT
		,BRAP_IMG_sbc_encode
		,BRAP_IMG_sbc_encode_table
		,BRAP_IMG_sbc_encode_inter_frame
#endif
#ifdef  RAP_REALAUDIOLBR_SUPPORT
        ,BRAP_IMG_ralbr_decode
        ,BRAP_IMG_ralbr_decode_table
        ,BRAP_IMG_ralbr_interframe_buf
#endif

#ifdef RAP_BRCM3DSURROUND_SUPPORT
		,BRAP_IMG_brcm3dsurround
		,BRAP_IMG_brcm3dsurround_tables
		,BRAP_IMG_brcm3dsurround_inter_frame
#endif

#ifdef RAP_MONODOWNMIX_SUPPORT
		,BRAP_IMG_monodownmix
#endif

#ifdef RAP_FWMIXER_SUPPORT
            ,BRAP_IMG_fw_mixer_ids
            ,BRAP_IMG_fw_mixer_code
            ,BRAP_IMG_fw_mixer_tables
            ,BRAP_IMG_fw_mixer_inter_frame
            ,BRAP_IMG_fw_mixer_ids_inter_frame
#endif

#ifdef RAP_ADV_SUPPORT
            ,BRAP_IMG_adv_code
            ,BRAP_IMG_adv_tables
            ,BRAP_IMG_adv_inter_frame
#endif

#ifdef RAP_ABX_SUPPORT
            ,BRAP_IMG_abx_code
            ,BRAP_IMG_abx_tables
            ,BRAP_IMG_abx_inter_frame
#endif

#ifdef RAP_SRSCSTD_SUPPORT
            ,BRAP_IMG_srs_cs_td_code
            ,BRAP_IMG_srs_cs_td_tables
            ,BRAP_IMG_srs_cs_td_interframe
#endif

#ifdef RAP_SRSEQHL_SUPPORT
            ,BRAP_IMG_srs_eq_hl_code
            ,BRAP_IMG_srs_eq_hl_tables
            ,BRAP_IMG_srs_eq_hl_interframe
#endif

#ifdef RAP_DV258_SUPPORT
            ,BRAP_IMG_dv258_code
            ,BRAP_IMG_dv258_tables
            ,BRAP_IMG_dv258_inter_frame
#endif

#ifdef RAP_DDRE_SUPPORT
            ,BRAP_IMG_dd_reenc_code
            ,BRAP_IMG_dd_reenc_tables
            ,BRAP_IMG_dd_reenc_inter_frame
#endif



#ifdef RAP_GFX_SUPPORT
            ,BRAP_IMG_gfx_decode
            ,BRAP_IMG_gfx_decode_tables
            ,BRAP_IMG_gfx_decode_interframe
#endif
#ifdef RAP_REALVIDEO_SUPPORT
            ,BRAP_IMG_rv40_decode_stg1
            ,BRAP_IMG_rv40_decode_stg2
            ,BRAP_IMG_rv40_decode_stg3
            ,BRAP_IMG_rv40_decode_stg1_tables
            ,BRAP_IMG_rv40_decode_stg2_tables
            ,BRAP_IMG_rv40_decode_stg3_tables
            ,BRAP_IMG_rv40_decode_ppd
            ,BRAP_IMG_rv40_decode_plf
            ,BRAP_IMG_rv40_decode_inter_frame
            ,BRAP_IMG_rv40_decode_plf_tables
#endif
#ifdef RAP_BTSC_SUPPORT
            ,BRAP_IMG_btsc_code
            ,BRAP_IMG_btsc_table
            ,BRAP_IMG_btsc_inter_frame
#endif
};


/*	This context is used by other modules to make use of this interface. They need to supply this as a parameter to BRAP_IMG_Open */
void *BRAP_IMG_Context = (void *)BRAP_IMG_FirmwareImages;

static BERR_Code BRAP_IMG_Open(void *context, void **image, unsigned image_id)
{
/*
	This function has to be used for opening a firmware image. The pointer to the firmware image array is given
	that contains the header and the chunks. This works based on the firmware image id  that is supplied.
*/
	if (image_id >=  sizeof(BRAP_IMG_FirmwareImages)/sizeof(*BRAP_IMG_FirmwareImages)) 
	{
		/* If the image ID is greater than available IDs return an error */
		BDBG_ERR(("Invalid image_id %d",image_id));
		return BERR_TRACE(BERR_INVALID_PARAMETER);	
	}	

	*image = ((void **)context)[image_id];	

	if (NULL == *image)
	{
	    BDBG_ERR (("Null image pointer!!!"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);	
	}

	return BERR_SUCCESS;
}

static BERR_Code BRAP_IMG_Next(void *image, unsigned chunk, const void **data, uint16_t length)
{
/*
	After opening the firmware image, the user of this interface will then be interested in getting the chunks
	and the header giving information about the chunks.
*/

	BDBG_ASSERT(data);	
	BSTD_UNUSED(length);

	if (chunk > ((uint32_t*)((const void **)image)[0])[1])
	{
		/* if the chunk number is greater than the available chunks in that image return error */
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	*data = ((const void **)image)[chunk];	
	
	if (NULL == *data) 
	{		
		return BERR_TRACE(BERR_INVALID_PARAMETER);	
	}	

	return BERR_SUCCESS;
}

static void BRAP_IMG_Close(void *image)
{	
/* This function is used to close the firmware image that was opened using BRAP_IMG_Open */
	BSTD_UNUSED(image);	
	return;
}

/* The interface is actually a set of three functions open, next and close. The same has been implemented here and their function pointers supplied */
const BIMG_Interface BRAP_IMG_Interface = {BRAP_IMG_Open, BRAP_IMG_Next, BRAP_IMG_Close};

