/***************************************************************************
 *     Copyright (c) 1999-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "brap_img.h"


BDBG_MODULE(rap_img);

/* External references of firmware binaries */
extern const void * BRAP_IMG_iboot [];

extern const void * BRAP_IMG_scheduler_data [];
extern const void * BRAP_IMG_scheduler_code [];

extern const void * BRAP_IMG_mpeg_framesync [];
extern const void * BRAP_IMG_ac3_framesync [];

extern const void * BRAP_IMG_mpeg_l1_decode [];
extern const void * BRAP_IMG_mpeg_l2_decode [];
extern const void * BRAP_IMG_mp3_decode [];

#if defined ( BCHP_7411_VER ) || (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)
extern const void * BRAP_IMG_ac3_decode [];
#endif 

extern const void * BRAP_IMG_mpeg_decode_table [];
extern const void * BRAP_IMG_ac3_decode_table [];

#ifdef BCHP_7411_VER /* For the 7411 */ 
extern const void * BRAP_IMG_ac3_downmix_code [];
extern const void * BRAP_IMG_ac3_downmix_table [];
#endif

extern const void * BRAP_IMG_ac3_passthru [];
extern const void * BRAP_IMG_mpeg_passthru [];

extern const void * BRAP_IMG_mpeg_interframe_buf [];

#if (BRAP_DVD_FAMILY != 1)
extern const void * BRAP_IMG_aac_framesync [];
extern const void * BRAP_IMG_aac_decode [];
extern const void * BRAP_IMG_aac_decode_table []; 
extern const void * BRAP_IMG_aac_interframe_buf [];
extern const void * BRAP_IMG_aac_passthru [];
extern const void * BRAP_IMG_aac_downmix [];
extern const void * BRAP_IMG_aache_decode [];
#endif

extern const void * BRAP_IMG_aache_framesync [];

extern const void * BRAP_IMG_aacplus_decode_table [];
extern const void * BRAP_IMG_aacplus_interframe_buf [];

extern const void * BRAP_IMG_ddp_framesync [];
extern const void * BRAP_IMG_ddp_be_decode [];
extern const void * BRAP_IMG_ddp_fe_decode [];
extern const void * BRAP_IMG_ddp_decode_table [];
extern const void * BRAP_IMG_ddp_interframe_buf [];

extern const void * BRAP_IMG_ddp_passthru [];
extern const void * BRAP_IMG_ddp_passthru_interframe_buf [];
extern const void * BRAP_IMG_ddp_passthru_table [];

#if (BRAP_DVD_FAMILY == 1)
extern const void * BRAP_IMG_wma_std_framesync[];
extern const void * BRAP_IMG_wma_std_stg0_decode[];
extern const void * BRAP_IMG_wma_std_stg0_decode_table[];
extern const void * BRAP_IMG_wma_std_interframe_buf[];
#else
extern const void * BRAP_IMG_wma_std_framesync[];
extern const void * BRAP_IMG_wma_std_stg1_decode[];
extern const void * BRAP_IMG_wma_std_stg1_decode_table[];
extern const void * BRAP_IMG_wma_std_interframe_buf[];

#endif

#if ( BCHP_CHIP == 3563)
extern const void * BRAP_IMG_dts_framesync [];
extern const void * BRAP_IMG_dts_decode [];
extern const void * BRAP_IMG_dts_decode_table [];
extern const void * BRAP_IMG_dts_interframe_buf [];
extern const void * BRAP_IMG_dts_passthru [];
#endif

#if (BRAP_DVD_FAMILY == 1)
extern const void * BRAP_IMG_dts_framesync [];
extern const void * BRAP_IMG_dts_decode [];
extern const void * BRAP_IMG_dts_1_decode [];
extern const void * BRAP_IMG_dts_decode_table [];
extern const void * BRAP_IMG_dts_interframe_buf [];
extern const void * BRAP_IMG_dts_passthru [];

extern const void * BRAP_IMG_dtshd_x96_decode[];
extern const void * BRAP_IMG_dtshd_xll_decode[];
extern const void * BRAP_IMG_dtshd_extras_decode[];

extern const void * BRAP_IMG_dtshd_framesync [];
extern const void * BRAP_IMG_dtshd_hddvd_framesync [];
extern const void * BRAP_IMG_dtshd_passthru [];


extern const void * BRAP_IMG_bdlpcm_framesync [];
extern const void * BRAP_IMG_bdlpcm_decode [];
extern const void * BRAP_IMG_bdlpcm_interframe_buf[];

extern const void * BRAP_IMG_hddvdlpcm_framesync [];
extern const void * BRAP_IMG_hddvdlpcm_decode [];
extern const void * BRAP_IMG_hddvdlpcm_interframe_buf[];

extern const void * BRAP_IMG_dvdlpcm_framesync [];
extern const void * BRAP_IMG_dvdlpcm_decode [];
extern const void * BRAP_IMG_dvdlpcm_interframe_buf[];

extern const void * BRAP_IMG_ddbm_postprocess[];
extern const void * BRAP_IMG_ddbm_postprocess_table[];
extern const void * BRAP_IMG_ddbm_interframe_buf[];

extern const void * BRAP_IMG_dts_neo_postprocess[];
extern const void * BRAP_IMG_dts_neo_postprocess_table[];
extern const void * BRAP_IMG_dts_neo_interframe_buf[];

extern const void * BRAP_IMG_ac3lossless_framesync [];

extern const void * BRAP_IMG_mlp_decode [];
extern const void * BRAP_IMG_mlp_framesync [];
extern const void * BRAP_IMG_mlp_hddvd_framesync [];
extern const void * BRAP_IMG_mlp_decode_table [];
extern const void * BRAP_IMG_mlp_passthru [];

extern const void * BRAP_IMG_scheduler_dsp1_data [];
extern const void * BRAP_IMG_scheduler_dsp1_code [];

extern const void * BRAP_IMG_dtslbr_framesync [];
extern const void * BRAP_IMG_dtslbr0_decode_code [];
extern const void * BRAP_IMG_dtshd_exss_decode_code [];
extern const void * BRAP_IMG_dtslbr1_decode_table [];
extern const void * BRAP_IMG_dtslbr1_decode_interframe_buf [];
extern const void * BRAP_IMG_dtslbr1_decode_code[];
extern const void * BRAP_IMG_mlp_interframe_buf [];

extern const void * BRAP_IMG_ddpdep_interframe_buf [];
extern const void * BRAP_IMG_ddpdep_decode_table [];
extern const void * BRAP_IMG_ddpdep_framesync [];
extern const void * BRAP_IMG_ddpdep_be_decode [];
extern const void * BRAP_IMG_ddpdep_fe_decode [];
extern const void * BRAP_IMG_ddpdep_passthru [];

extern const void * BRAP_IMG_mpeg_mc_framesync [];
extern const void * BRAP_IMG_mpeg_mc_decode [];
extern const void * BRAP_IMG_mpeg_mc_decode_table [];
extern const void * BRAP_IMG_mpeg_mc_passthru [];

extern const void * BRAP_IMG_dtshd_subaudio_framesync [];
extern const void * BRAP_IMG_dtshd_subaudio_1_decode [];
extern const void * BRAP_IMG_dtshd_subaudio_decode [];
extern const void * BRAP_IMG_dtshd_subaudio_decode_table [];
extern const void * BRAP_IMG_dtshd_subaudio_interframe_buf [];

extern const void * BRAP_IMG_dvdalpcm_decode [];
extern const void * BRAP_IMG_dvdlapcm_framesync [];
extern const void * BRAP_IMG_dvdalpcm_interframe [];
#endif

#ifdef BCHP_7411_VER /* For the 7411 */ 
extern const void * BRAP_IMG_dts_framesync [];
extern const void * BRAP_IMG_dts_decode [];
extern const void * BRAP_IMG_dts_decode_table [];
extern const void * BRAP_IMG_dts_interframe_buf [];
extern const void * BRAP_IMG_dts_passthru [];

extern const void * BRAP_IMG_dtshd_framesync [];
extern const void * BRAP_IMG_dtshd_hddvd_framesync [];

extern const void * BRAP_IMG_bdlpcm_framesync [];
extern const void * BRAP_IMG_bdlpcm_decode [];
extern const void * BRAP_IMG_bdlpcm_interframe_buf[];

extern const void * BRAP_IMG_hddvdlpcm_framesync [];
extern const void * BRAP_IMG_hddvdlpcm_decode [];
extern const void * BRAP_IMG_hddvdlpcm_interframe_buf[];

extern const void * BRAP_IMG_dvdlpcm_framesync [];
extern const void * BRAP_IMG_dvdlpcm_decode [];
extern const void * BRAP_IMG_dvdlpcm_interframe_buf[];

extern const void * BRAP_IMG_ddbm_postprocess[];
extern const void * BRAP_IMG_ddbm_postprocess_table[];

extern const void * BRAP_IMG_ac3lossless_framesync [];
extern const void * BRAP_IMG_mlp_decode [];
extern const void * BRAP_IMG_mlp_framesync [];
extern const void * BRAP_IMG_mlp_hddvd_framesync [];
extern const void * BRAP_IMG_mlp_decode_table [];


#endif

/* Adding for MP3 Encoder */
#ifndef BCHP_7411_VER /* For chips other than 7411 */
extern const void * BRAP_IMG_encoder_iboot [];
extern const void * BRAP_IMG_encoder_scheduler_data [];
extern const void * BRAP_IMG_encoder_scheduler_code [];
extern const void * BRAP_IMG_mp3e_table [];
extern const void * BRAP_IMG_mp3e_inter_frame [];
extern const void * BRAP_IMG_mp3e_inter_stage [];
extern const void * BRAP_IMG_mp3e_scratch [];
extern const void * BRAP_IMG_mp3e_stg0 [];
extern const void * BRAP_IMG_mp3e_stg1 [];
#if (BRAP_DVD_FAMILY == 1)||( BCHP_CHIP == 3563 ) || (BRAP_7405_FAMILY == 1)
extern const void * BRAP_IMG_dts_enc_code [];
extern const void * BRAP_IMG_dts_enc_table [];
extern const void * BRAP_IMG_dts_enc_inter_frame [];
extern const void * BRAP_IMG_dts_enc_inter_stage [];
extern const void * BRAP_IMG_dts_enc_scratch [];
#endif /* 7440 */
#endif

#if ( BCHP_CHIP == 7400 || BCHP_CHIP == 3563 || (BRAP_DVD_FAMILY == 1) || BRAP_7405_FAMILY == 1)
#ifdef BRAP_DDP_SUPPORT		
extern const void * BRAP_IMG_ddpdep_framesync [];
extern const void * BRAP_IMG_ddp_convert [];
#endif
#endif

#if ( BCHP_CHIP==3563 )
extern const void *BRAP_IMG_BtscUs[];
extern const void *BRAP_IMG_BtscJapan[];
extern const void *BRAP_IMG_BtscKorea[];
extern const void *BRAP_IMG_pes_framesync[];
extern const void *BRAP_IMG_pcm_input[];
extern const void *BRAP_IMG_prologic2_decode[];
extern const void *BRAP_IMG_prologic2_table[];
extern const void *BRAP_IMG_prologic2_interframe_buf[];
extern const void *BRAP_IMG_srsxt[];
extern const void *BRAP_IMG_srsxt_tables[];
extern const void *BRAP_IMG_srsxt_interframe[];
extern const void *BRAP_IMG_avl[];
extern const void *BRAP_IMG_avl_table[];
extern const void *BRAP_IMG_avl_interframe_buf[];
extern const void *BRAP_IMG_xen[];
extern const void *BRAP_IMG_xen_table[];
extern const void *BRAP_IMG_xen_interframe_buf[];
extern const void *BRAP_IMG_bbe[];
extern const void *BRAP_IMG_bbe_table[];
extern const void *BRAP_IMG_bbe_interframe_buf[];
extern const void *BRAP_IMG_custom_surround[];
extern const void *BRAP_IMG_custom_surround_table[];
extern const void *BRAP_IMG_custom_surround_interframe_buf[];
extern const void *BRAP_IMG_custom_bass[];
extern const void *BRAP_IMG_custom_bass_table[];
extern const void *BRAP_IMG_custom_bass_interframe_buf[];
extern const void *BRAP_IMG_custom_voice[];
extern const void *BRAP_IMG_custom_voice_table[];
extern const void *BRAP_IMG_custom_voice_interframe_buf[];
extern const void *BRAP_IMG_peq[];
extern const void *BRAP_IMG_peq_table[];
extern const void *BRAP_IMG_peq_interframe_buf[];
#endif

#if ( BCHP_CHIP == 7400 || BCHP_CHIP == 3563 || (BRAP_DVD_FAMILY == 1) || BRAP_7405_FAMILY == 1 )
#ifndef BRAP_DDP_SUPPORT
extern const void * BRAP_IMG_ac3_passthru_table [];
extern const void * BRAP_IMG_ac3_be_decode [];
extern const void * BRAP_IMG_ac3_convert [];
extern const void * BRAP_IMG_ac3_fe_decode [];
extern const void * BRAP_IMG_ac3_interframe_buf [];
extern const void * BRAP_IMG_ac3_passthru_interframe_buf [];
#endif
#endif


#if (BCHP_CHIP == 7401 || BCHP_CHIP == 7118 || BCHP_CHIP == 7403 || BCHP_CHIP == 3563 ||BCHP_CHIP == 7400 )
extern const void * BRAP_IMG_src_postprocess[];
extern const void * BRAP_IMG_src_postprocess_table[];
extern const void * BRAP_IMG_src_interframe_buf[];
#endif

#if RAP_DOLBYVOLUME_SUPPORT 
extern const void * BRAP_IMG_dolby_volume[];
extern const void * BRAP_IMG_dolby_volume_table[];
extern const void * BRAP_IMG_dolby_volume_interframe_buf[];
#endif

#ifdef RAP_SRSTRUVOL_SUPPORT
extern const void * BRAP_IMG_srs_TruVolume_post_process[];
extern const void * BRAP_IMG_srs_TruVolume_table[];
extern const void * BRAP_IMG_srs_TruVolume_interframe_buf[];
#endif

#if (BCHP_CHIP == 7401 || BCHP_CHIP == 7118 || BCHP_CHIP == 7403 || BCHP_CHIP == 7400)
extern const void * BRAP_IMG_dsola_postprocess[];
extern const void * BRAP_IMG_dsola_postprocess_table[];
extern const void * BRAP_IMG_dsola_interframe_buf[];
#if (BRAP_AD_SUPPORTED == 1)
extern const void * BRAP_IMG_fade_control[];
extern const void * BRAP_IMG_fade_control_tables[];
extern const void * BRAP_IMG_fade_control_interframe_buf[];
extern const void * BRAP_IMG_pan_control[];
extern const void * BRAP_IMG_pan_control_tables[];
extern const void * BRAP_IMG_pan_control_interframe_buf[];
#endif
#endif

#if ( BCHP_CHIP == 3563 ) || (BRAP_7405_FAMILY == 1)
extern const void * BRAP_IMG_aache_lp_decode_stg0[];
extern const void * BRAP_IMG_aache_lp_decode_stg1[];
extern const void * BRAP_IMG_aache_lp_decode_stg2[];
extern const void * BRAP_IMG_aache_lp_interframe_buf[];
extern const void * BRAP_IMG_aache_lp_table_stg0[];
extern const void * BRAP_IMG_aache_lp_table_stg1[];
extern const void * BRAP_IMG_aache_passthru[];
#endif

#if (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403) || ( BCHP_CHIP == 7400 ) 
extern const void * BRAP_IMG_wmapro_framesync[];
extern const void * BRAP_IMG_wmapro_decode_stg0[];
extern const void * BRAP_IMG_wmapro_decode_stg1[];
extern const void * BRAP_IMG_wmapro_decode_table[];
extern const void * BRAP_IMG_wmapro_passthru[];
#endif

#if (BRAP_7405_FAMILY == 1) || (BRAP_DVD_FAMILY == 1)
extern const void * BRAP_IMG_wmapro_framesync[];
extern const void * BRAP_IMG_wmapro_decode_stg0[];
extern const void * BRAP_IMG_wmapro_decode_stg1[];
extern const void * BRAP_IMG_wmapro_decode_table[];
extern const void * BRAP_IMG_wmapro_passthru[];
#endif

extern const void * BRAP_IMG_dvdlpcm_framesync [];
extern const void * BRAP_IMG_dvdlpcm_decode [];
extern const void * BRAP_IMG_dvdlpcm_interframe_buf[];
#if (BRAP_DVD_FAMILY == 1)
extern const void * BRAP_IMG_dtshd_nbc_core_framesync[]; 
#endif
#if (BCHP_CHIP == 7401) ||(BCHP_CHIP==7403) || (BCHP_CHIP==7118) || ( BCHP_CHIP == 7400 )
extern const void * BRAP_IMG_aache_decode_stg0[];
extern const void * BRAP_IMG_aache_decode_stg1[];
extern const void * BRAP_IMG_aache_decode_stg2[];
extern const void * BRAP_IMG_aache_decode_stg3[];
extern const void * BRAP_IMG_aache_decode_stg4[];

extern const void * BRAP_IMG_aache_decode_table_stg0[];
extern const void * BRAP_IMG_aache_decode_table_stg1[];
extern const void * BRAP_IMG_aache_decode_table_stg2[];
extern const void * BRAP_IMG_aache_decode_table_stg3[];
extern const void * BRAP_IMG_aache_decode_table_stg4[];

extern const void * BRAP_IMG_aache_interframe_buf[];
extern const void * BRAP_IMG_aache_passthru[];
#endif

#if (BRAP_DVD_FAMILY == 1)
extern const void * BRAP_IMG_aacloas_framesync[];
extern const void * BRAP_IMG_aache_decode_stg0[];
extern const void * BRAP_IMG_aache_decode_stg1[];
extern const void * BRAP_IMG_aache_decode_stg2[];
extern const void * BRAP_IMG_aache_decode_stg3[];
extern const void * BRAP_IMG_aache_decode_stg4[];

extern const void * BRAP_IMG_aache_decode_table[];
extern const void * BRAP_IMG_aache_interframe_buf[];

extern const void * BRAP_IMG_aacadts_framesync[];

#endif

extern const void * BRAP_IMG_dtshd_stg1_framesync[];
extern const void * BRAP_IMG_dtshd_stg1_decode_table[];
extern const void * BRAP_IMG_dtshd_stg1_interframe_buf[];

extern const void * BRAP_IMG_scheduler_dwd_code[];
extern const void * BRAP_IMG_scheduler_dsp1_dwd_code[];

extern const void * BRAP_IMG_aache_decode_stg0_tables[];
extern const void * BRAP_IMG_aache_decode_stg1_tables[];
extern const void * BRAP_IMG_aache_decode_stg2_tables[];
extern const void * BRAP_IMG_aache_decode_stg3_tables[];
extern const void * BRAP_IMG_aache_decode_stg4_tables[];


#if (BRAP_DTS_SUPPORTED == 1)		
extern const void * BRAP_IMG_dts_framesync [];
extern const void * BRAP_IMG_dts_core_decode [];
extern const void * BRAP_IMG_dts_core_decode_table [];
extern const void * BRAP_IMG_dts_core_decode_interframe_buf [];
extern const void * BRAP_IMG_dts_passthru [];
#endif

#ifdef RAP_SRSTRUVOL_CERTIFICATION
extern const void * BRAP_IMG_pcm_decode[];
extern const void * BRAP_IMG_pcm_framesync[];
#endif

#if (BRAP_DTS_PASSTHRU_SUPPORTED == 1)		
extern const void * BRAP_IMG_dts_framesync [];
extern const void * BRAP_IMG_dts_passthru [];
#endif

#ifdef RAP_DRA_SUPPORT	
extern const void * BRAP_IMG_dra_decode[];
extern const void * BRAP_IMG_dra_decode_table[];
extern const void * BRAP_IMG_dra_interframe_buf[];
extern const void * BRAP_IMG_dra_passthru[];
extern const void * BRAP_IMG_dra_framesync[];
#endif

#ifdef RAP_PCMWAV_SUPPORT
extern const void * BRAP_IMG_pcmwav_decode[];
extern const void * BRAP_IMG_pcmwav_framesync[];
extern const void * BRAP_IMG_pcmwav_interframe_buf[];
#endif

/*
	This array will contain pointers to the arrays that contain that chunk pointers of the firmware binary.
	Based on the firmware ID one particular firmware Image can be accessed from this array
*/
static const void **BRAP_IMG_FirmwareImages [] = 
	{
		BRAP_IMG_iboot, 
		BRAP_IMG_scheduler_code, 	
		BRAP_IMG_scheduler_data, 
		BRAP_IMG_mpeg_framesync,
#if defined ( BCHP_7411_VER ) || (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)		
		BRAP_IMG_ac3_framesync, 
#else
		NULL, 
#endif
		BRAP_IMG_mpeg_l1_decode, 
		BRAP_IMG_mpeg_l2_decode, 
		BRAP_IMG_mp3_decode, 
#if defined ( BCHP_7411_VER ) || (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)		
		BRAP_IMG_ac3_decode,
#else
		NULL, 
#endif
		BRAP_IMG_mpeg_decode_table, 
#if defined ( BCHP_7411_VER ) || (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)		
		BRAP_IMG_ac3_decode_table,
#else
		NULL, 
#endif

#ifndef BCHP_7411_VER /* For chips other than 7411 */
		NULL, 
		NULL,
#else
		BRAP_IMG_ac3_downmix_code, 
		BRAP_IMG_ac3_downmix_table,
#endif
#if defined ( BCHP_7411_VER ) || (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)		
		BRAP_IMG_ac3_passthru, 
#else
		NULL, 
#endif
		BRAP_IMG_mpeg_passthru,
		BRAP_IMG_mpeg_interframe_buf,
#ifndef BCHP_7411_VER /* For chips other than 7411 */
		
#if (BRAP_DVD_FAMILY == 1)
        BRAP_IMG_aacadts_framesync,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        BRAP_IMG_aacloas_framesync,
		NULL,
        NULL,
#else
        BRAP_IMG_aac_framesync,
		BRAP_IMG_aac_decode,
		BRAP_IMG_aac_decode_table,
		BRAP_IMG_aac_interframe_buf,
		BRAP_IMG_aac_passthru,
		BRAP_IMG_aac_downmix,
        BRAP_IMG_aache_framesync,
		BRAP_IMG_aache_decode,
        BRAP_IMG_aacplus_decode_table,
#endif

#if ((BRAP_DVD_FAMILY == 1) || BCHP_CHIP == 7118 || BCHP_CHIP ==7403 || BCHP_CHIP ==7401 || BCHP_CHIP == 7400)
		BRAP_IMG_aache_interframe_buf,
#else
		BRAP_IMG_aacplus_interframe_buf,
#endif
#if ( BCHP_CHIP == 7400 || BCHP_CHIP == 3563 || (BRAP_DVD_FAMILY == 1) || BRAP_7405_FAMILY == 1 )
#ifndef BRAP_DDP_SUPPORT	
		BRAP_IMG_ac3_framesync,
		BRAP_IMG_ac3_be_decode,
		BRAP_IMG_ac3_fe_decode,
		BRAP_IMG_ac3_decode_table,
		BRAP_IMG_ac3_interframe_buf,
		BRAP_IMG_ac3_passthru,
		BRAP_IMG_ac3_passthru_interframe_buf,
		BRAP_IMG_ac3_passthru_table,
#else
		BRAP_IMG_ddp_framesync,
		BRAP_IMG_ddp_be_decode,
		BRAP_IMG_ddp_fe_decode,
		BRAP_IMG_ddp_decode_table,
		BRAP_IMG_ddp_interframe_buf,
		BRAP_IMG_ddp_passthru,
		BRAP_IMG_ddp_passthru_interframe_buf,
		BRAP_IMG_ddp_passthru_table,
#endif
#else
		BRAP_IMG_ddp_framesync,
		BRAP_IMG_ddp_be_decode,
		BRAP_IMG_ddp_fe_decode,
		BRAP_IMG_ddp_decode_table,
		BRAP_IMG_ddp_interframe_buf,
		BRAP_IMG_ddp_passthru,
		BRAP_IMG_ddp_passthru_interframe_buf,
		BRAP_IMG_ddp_passthru_table,
#endif

#if (BRAP_DVD_FAMILY == 1)
		BRAP_IMG_dts_framesync,
		BRAP_IMG_dts_decode,
		BRAP_IMG_dts_decode_table,
		BRAP_IMG_dts_interframe_buf,
		BRAP_IMG_dts_passthru,
		BRAP_IMG_bdlpcm_framesync,
		BRAP_IMG_bdlpcm_decode,
#elif ( BCHP_CHIP == 3563 )
		NULL,
		NULL,
		NULL,
		NULL,
		BRAP_IMG_dts_passthru,
		NULL,
		NULL,
#elif (BRAP_DTS_SUPPORTED == 1)
        BRAP_IMG_dts_framesync,
        BRAP_IMG_dts_core_decode,
        BRAP_IMG_dts_core_decode_table,
        BRAP_IMG_dts_core_decode_interframe_buf,
        BRAP_IMG_dts_passthru,
        NULL,
        NULL,
#elif (BRAP_DTS_PASSTHRU_SUPPORTED == 1)
        BRAP_IMG_dts_framesync,
        NULL,
        NULL,
        NULL,
        BRAP_IMG_dts_passthru,
        NULL,
        NULL,        
#else
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
        NULL,
        NULL,        
#endif		
#else
		BRAP_IMG_aac_framesync,
		BRAP_IMG_aac_decode,
		BRAP_IMG_aac_decode_table,
		BRAP_IMG_aac_interframe_buf,
		BRAP_IMG_aac_passthru,
		BRAP_IMG_aac_downmix,
		BRAP_IMG_aache_framesync,
		BRAP_IMG_aache_decode,
		BRAP_IMG_aacplus_decode_table,
		BRAP_IMG_aacplus_interframe_buf,
		BRAP_IMG_ddp_framesync,
		BRAP_IMG_ddp_be_decode,
		BRAP_IMG_ddp_fe_decode,
		BRAP_IMG_ddp_decode_table,
		BRAP_IMG_ddp_interframe_buf,
		BRAP_IMG_ddp_passthru,
		BRAP_IMG_ddp_passthru_interframe_buf,
		BRAP_IMG_ddp_passthru_table,
		BRAP_IMG_dts_framesync,
		BRAP_IMG_dts_decode,
		BRAP_IMG_dts_decode_table,
		BRAP_IMG_dts_interframe_buf,
		BRAP_IMG_dts_passthru,
		BRAP_IMG_bdlpcm_framesync,
		BRAP_IMG_bdlpcm_decode,
#endif
/* Adding for MP3 Encoder */
#ifndef BCHP_7411_VER /* For chips other than 7411 */
		BRAP_IMG_encoder_iboot,
		BRAP_IMG_encoder_scheduler_code,
		BRAP_IMG_encoder_scheduler_data,
		BRAP_IMG_mp3e_table,
		BRAP_IMG_mp3e_inter_frame,
		BRAP_IMG_mp3e_inter_stage,
		BRAP_IMG_mp3e_scratch,
		BRAP_IMG_mp3e_stg0,
		BRAP_IMG_mp3e_stg1,
/*		BRAP_IMG_mp3e_stg2,*/
		NULL,
/*		BRAP_IMG_mp3e_stg3,
		BRAP_IMG_mp3e_stg4,*/
		NULL,
		NULL,
#else
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
#endif
#ifndef BCHP_7411_VER /* For chips other than 7411 */
#if (BRAP_DVD_FAMILY == 1)
		BRAP_IMG_hddvdlpcm_framesync,
		BRAP_IMG_hddvdlpcm_decode,
		BRAP_IMG_dtshd_framesync,
	    BRAP_IMG_ddbm_postprocess,
		BRAP_IMG_ddbm_postprocess_table,
		BRAP_IMG_dvdlpcm_framesync,
		BRAP_IMG_dvdlpcm_decode,

        BRAP_IMG_wma_std_framesync,
		BRAP_IMG_wma_std_stg0_decode,
		BRAP_IMG_wma_std_stg0_decode_table,
		BRAP_IMG_wma_std_interframe_buf,

        BRAP_IMG_ac3lossless_framesync,
		BRAP_IMG_dtshd_hddvd_framesync,
		BRAP_IMG_bdlpcm_interframe_buf,
		BRAP_IMG_dvdlpcm_interframe_buf,
		BRAP_IMG_hddvdlpcm_interframe_buf,
		BRAP_IMG_mlp_decode,
		BRAP_IMG_mlp_decode_table,
		BRAP_IMG_mlp_framesync,
		BRAP_IMG_mlp_hddvd_framesync,
#else
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		BRAP_IMG_dvdlpcm_framesync, 
		BRAP_IMG_dvdlpcm_decode,		

		BRAP_IMG_wma_std_framesync,
		BRAP_IMG_wma_std_stg1_decode,
		BRAP_IMG_wma_std_stg1_decode_table,
		BRAP_IMG_wma_std_interframe_buf,

		NULL,
		NULL,
		NULL,
		BRAP_IMG_dvdlpcm_interframe_buf, 
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
#endif		
#else
		BRAP_IMG_hddvdlpcm_framesync,
		BRAP_IMG_hddvdlpcm_decode,
        /* Note: Except for the frame-sync, all other fw is common between DTS and DTS-HD */
		BRAP_IMG_dtshd_framesync,
		BRAP_IMG_ddbm_postprocess,
		BRAP_IMG_ddbm_postprocess_table,
		BRAP_IMG_dvdlpcm_framesync,
		BRAP_IMG_dvdlpcm_decode,
		BRAP_IMG_wma_std_framesync,
		BRAP_IMG_wma_std_stg1_decode,
		BRAP_IMG_wma_std_stg1_decode_table,
		BRAP_IMG_wma_std_interframe_buf,
        /* Note: Except for the frame-sync, all other fw is common between AC3 Lossless and AC3 */
		BRAP_IMG_ac3lossless_framesync,
		BRAP_IMG_dtshd_hddvd_framesync,
		BRAP_IMG_bdlpcm_interframe_buf,
		BRAP_IMG_dvdlpcm_interframe_buf,
		BRAP_IMG_hddvdlpcm_interframe_buf,
		BRAP_IMG_mlp_decode,
		BRAP_IMG_mlp_decode_table,
		BRAP_IMG_mlp_framesync,
		BRAP_IMG_mlp_hddvd_framesync,
#endif		
/* Adding for DTS Encoder */
#if (BRAP_DVD_FAMILY == 1)||( BCHP_CHIP == 3563 ) || (BRAP_7405_FAMILY == 1)
		BRAP_IMG_dts_enc_code,
		BRAP_IMG_dts_enc_table,
		BRAP_IMG_dts_enc_inter_frame,
		BRAP_IMG_dts_enc_inter_stage,
		BRAP_IMG_dts_enc_scratch,
#else
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
#endif/* 7440 */
/* Adding for DDP Converted */
#if ( BCHP_CHIP == 7400 || BCHP_CHIP == 3563 || (BRAP_DVD_FAMILY == 1) || BRAP_7405_FAMILY == 1 )
#ifdef BRAP_DDP_SUPPORT		
		BRAP_IMG_ddp_convert,
#else
		BRAP_IMG_ac3_convert,
#endif
#else
		NULL,
#endif
#if ( BCHP_CHIP==3563 )
        BRAP_IMG_BtscUs,
        BRAP_IMG_BtscJapan,
        BRAP_IMG_BtscKorea,
        BRAP_IMG_pes_framesync,
        BRAP_IMG_pcm_input,
#else
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
#endif
#if (BRAP_DVD_FAMILY == 1)
        BRAP_IMG_dts_1_decode
#else
        NULL
#endif
#if (BCHP_CHIP == 7401 || BCHP_CHIP == 7118 || BCHP_CHIP == 7403 || BCHP_CHIP==3563 || BCHP_CHIP == 7400)
        ,BRAP_IMG_src_postprocess,
        BRAP_IMG_src_postprocess_table,
        BRAP_IMG_src_interframe_buf,           
#else
        ,NULL,
        NULL,
        NULL,
#endif
#if (BCHP_CHIP==3563)
	BRAP_IMG_prologic2_decode,
	BRAP_IMG_prologic2_table,
	BRAP_IMG_prologic2_interframe_buf,
	BRAP_IMG_srsxt,
	BRAP_IMG_srsxt_tables,
	BRAP_IMG_srsxt_interframe,
	BRAP_IMG_avl,
	BRAP_IMG_avl_table,
	BRAP_IMG_avl_interframe_buf,
#else	
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
#endif
#if (BRAP_DVD_FAMILY == 1)
    BRAP_IMG_ddbm_interframe_buf,
    BRAP_IMG_scheduler_dsp1_code,
    BRAP_IMG_scheduler_dsp1_data,
    BRAP_IMG_dtslbr_framesync,
    BRAP_IMG_dtslbr0_decode_code,
    BRAP_IMG_dtslbr1_decode_table,
    BRAP_IMG_dtslbr1_decode_interframe_buf,
    BRAP_IMG_mlp_interframe_buf,
#else
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif
#if (BCHP_CHIP==3563)
	BRAP_IMG_xen,
	BRAP_IMG_xen_table,
	BRAP_IMG_xen_interframe_buf,
	BRAP_IMG_bbe,
	BRAP_IMG_bbe_table,
	BRAP_IMG_bbe_interframe_buf
#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,	
	NULL
#endif
#if (BCHP_CHIP == 7401 || BCHP_CHIP == 7118 || BCHP_CHIP == 7403 || BCHP_CHIP == 7400)
        ,BRAP_IMG_dsola_postprocess,
        BRAP_IMG_dsola_postprocess_table,
        BRAP_IMG_dsola_interframe_buf,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	BRAP_IMG_aache_passthru,
#else
        ,NULL,
        NULL,
        NULL,
#if ( BCHP_CHIP == 7400 ) || ( BCHP_CHIP == 3563 ) || (BRAP_7405_FAMILY == 1)
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	BRAP_IMG_aache_passthru,
#else
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
#endif        
#endif
#if (BRAP_DVD_FAMILY == 1)
		BRAP_IMG_ddpdep_framesync,
		BRAP_IMG_ddpdep_be_decode,
		BRAP_IMG_ddpdep_fe_decode,
		BRAP_IMG_ddpdep_decode_table,
		BRAP_IMG_ddpdep_interframe_buf,
    	BRAP_IMG_mpeg_mc_framesync,
		BRAP_IMG_mpeg_mc_decode,
		BRAP_IMG_mpeg_mc_decode_table,
		BRAP_IMG_mpeg_mc_passthru,
        BRAP_IMG_dts_neo_postprocess,
        BRAP_IMG_dts_neo_postprocess_table,
        BRAP_IMG_dts_neo_interframe_buf,
		BRAP_IMG_dtshd_x96_decode,
		BRAP_IMG_dtshd_xll_decode,
		BRAP_IMG_dtshd_extras_decode,
#else
#ifdef BRAP_DDP_SUPPORT
#if (BCHP_CHIP==7400)
        BRAP_IMG_ddpdep_framesync,
#else
	    NULL,
#endif
#else
	    NULL,
#endif	    
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL,
	    NULL,
        NULL,
	    NULL,
	    NULL,
        NULL,
#endif
#if (BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403) || ( BCHP_CHIP == 7400 ) 
	BRAP_IMG_wmapro_framesync,
	BRAP_IMG_wmapro_decode_stg0,
	BRAP_IMG_wmapro_decode_stg1,
	BRAP_IMG_wmapro_decode_table,
	BRAP_IMG_wmapro_passthru,
#elif (BRAP_7405_FAMILY == 1)||(BRAP_DVD_FAMILY == 1)
	BRAP_IMG_wmapro_framesync,
	BRAP_IMG_wmapro_decode_stg0,
	BRAP_IMG_wmapro_decode_stg1,
	BRAP_IMG_wmapro_decode_table,
	BRAP_IMG_wmapro_passthru,
#else
        NULL,
	    NULL,
	    NULL,
	    NULL,	    
        NULL,
#endif
#if (BRAP_DVD_FAMILY == 1)
		BRAP_IMG_dtshd_exss_decode_code,
		BRAP_IMG_ddpdep_passthru,
		BRAP_IMG_mlp_passthru,
		BRAP_IMG_dtshd_passthru
#else
	    NULL,
	    NULL,
	    NULL,	    
        NULL
#endif
#if (BCHP_CHIP==3563)
	,BRAP_IMG_custom_surround,
	BRAP_IMG_custom_surround_table,
	BRAP_IMG_custom_surround_interframe_buf,
	BRAP_IMG_custom_bass,
	BRAP_IMG_custom_bass_table,
	BRAP_IMG_custom_bass_interframe_buf,
	BRAP_IMG_custom_voice,
	BRAP_IMG_custom_voice_table,
	BRAP_IMG_custom_voice_interframe_buf
#else
	,NULL,
	NULL,
	NULL,
	NULL,	
	NULL,
	NULL,
	NULL,	
	NULL,
	NULL
#endif	

#if (BRAP_DVD_FAMILY == 1)
	,BRAP_IMG_dtslbr1_decode_code
#else
	,NULL
#endif	

#if (BCHP_CHIP==3563)
	,BRAP_IMG_peq,
	BRAP_IMG_peq_table,
	BRAP_IMG_peq_interframe_buf
#else
	,NULL,
	NULL,
	NULL
#endif	



#if (BRAP_DVD_FAMILY == 1)
	,BRAP_IMG_dtshd_subaudio_decode_table,
    BRAP_IMG_dtshd_subaudio_interframe_buf,
	BRAP_IMG_dtshd_subaudio_framesync,
    BRAP_IMG_dtshd_subaudio_decode,
    BRAP_IMG_dtshd_subaudio_1_decode
#else
	,NULL,
	NULL,
	NULL,
	NULL,
	NULL
#endif	

/*DVD_A_LPCM*/
#if (BRAP_DVD_FAMILY == 1)
    ,BRAP_IMG_dvdalpcm_interframe,
    BRAP_IMG_dvdlapcm_framesync,
    BRAP_IMG_dvdalpcm_decode
#else
    ,NULL,
    NULL,
    NULL
#endif
#if (BRAP_DVD_FAMILY == 1)
    ,BRAP_IMG_dtshd_nbc_core_framesync
#else
    ,NULL
#endif

#if (BCHP_CHIP==7401) ||(BCHP_CHIP==7403) ||(BCHP_CHIP==7118)||(BCHP_CHIP==7400)
	,BRAP_IMG_aache_decode_stg0,
	BRAP_IMG_aache_decode_stg1,
	BRAP_IMG_aache_decode_stg2,
	BRAP_IMG_aache_decode_stg3,
	BRAP_IMG_aache_decode_stg4,
	BRAP_IMG_aache_decode_table_stg0,
	BRAP_IMG_aache_decode_table_stg1,
	BRAP_IMG_aache_decode_table_stg2,
	BRAP_IMG_aache_decode_table_stg3,
	BRAP_IMG_aache_decode_table_stg4,
	NULL,
	NULL,
	NULL,
	BRAP_IMG_scheduler_dwd_code,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
	
#elif (BRAP_DVD_FAMILY == 1)
    ,BRAP_IMG_aache_decode_stg0,
	BRAP_IMG_aache_decode_stg1,
	BRAP_IMG_aache_decode_stg2,
	BRAP_IMG_aache_decode_stg3,
	BRAP_IMG_aache_decode_stg4,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	BRAP_IMG_dtshd_stg1_framesync,
	BRAP_IMG_dtshd_stg1_decode_table,
	BRAP_IMG_dtshd_stg1_interframe_buf,
	BRAP_IMG_scheduler_dwd_code,
	BRAP_IMG_scheduler_dsp1_dwd_code,
	BRAP_IMG_aache_decode_stg0_tables,
	BRAP_IMG_aache_decode_stg1_tables,
	BRAP_IMG_aache_decode_stg2_tables,
	BRAP_IMG_aache_decode_stg3_tables,
	BRAP_IMG_aache_decode_stg4_tables
#else
	,NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
#endif

#if (BRAP_AD_SUPPORTED == 1)
    ,BRAP_IMG_fade_control,
    BRAP_IMG_fade_control_tables,
    BRAP_IMG_fade_control_interframe_buf,
    BRAP_IMG_pan_control,
    BRAP_IMG_pan_control_tables,
    BRAP_IMG_pan_control_interframe_buf
#else
	,NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
#endif

#ifdef RAP_DRA_SUPPORT	
	,BRAP_IMG_dra_passthru,
	BRAP_IMG_dra_framesync,
	BRAP_IMG_dra_interframe_buf
#else
	,NULL,
	NULL,
	NULL
#endif

#if RAP_DOLBYVOLUME_SUPPORT 
    ,BRAP_IMG_dolby_volume,
    BRAP_IMG_dolby_volume_table,
    BRAP_IMG_dolby_volume_interframe_buf           
#else
	,NULL,
	NULL,
	NULL
#endif
#ifdef RAP_SRSTRUVOL_SUPPORT
    ,BRAP_IMG_srs_TruVolume_post_process,
    BRAP_IMG_srs_TruVolume_table,
    BRAP_IMG_srs_TruVolume_interframe_buf           
#else
	,NULL,
	NULL,
	NULL
#endif

#ifdef RAP_SRSTRUVOL_CERTIFICATION
    ,BRAP_IMG_pcm_decode,
    BRAP_IMG_pcm_framesync
#else
	,NULL,
	NULL
#endif

#ifdef RAP_DRA_SUPPORT	
	,BRAP_IMG_dra_decode,
	BRAP_IMG_dra_decode_table
#else
	,NULL,
	NULL
#endif

#ifdef RAP_PCMWAV_SUPPORT
    ,BRAP_IMG_pcmwav_decode,
    BRAP_IMG_pcmwav_framesync,
    BRAP_IMG_pcmwav_interframe_buf
#else
	,NULL,
	NULL,
	NULL
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

