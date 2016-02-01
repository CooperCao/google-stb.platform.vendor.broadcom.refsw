############################################################
#     Copyright (c) 2003-2012, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################
# Define this module
MAGNUM_MODULES += BRAP

ifeq ($(BRAP_SUPPORT_TSM_LOG),y)
BRAP_DEFINES += BRAP_SUPPORT_TSM_LOG=1
endif

# Module Includes
BRAP_INCLUDES += $(MAGNUM)/portinginterface/rap/src/fmm130
BRAP_INCLUDES += $(MAGNUM)/portinginterface/rap/include/fmm130
BRAP_INCLUDES += $(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500

BRAP_SOURCES += \
    $(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_img.c

ifeq ($(BRAP_IMG_ONLY), )
# Module Sources
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_dec.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_int_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_dsp.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_dspchn.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_fmm.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_rbuf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_srcch.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_dstch.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_mixer.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_spdiffm.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_op.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_rm.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_hifidac.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_spdiffm_fw.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_pb.c\
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_cap.c\
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_pvr.c\
	$(MAGNUM)/portinginterface/rap/src/fmm130/brap_enc.c
endif

BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mpeg_framesync.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mpeg_l1_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mpeg_l2_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mp3_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mpeg_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mpeg_interframe_buf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mpeg_passthru.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mp3e_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mp3e_inter_stage.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mp3e_scratch.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mp3e_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mp3e_stg0.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_mp3e_stg1.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_iboot.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_scheduler_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_scheduler_data.c  \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aac_framesync.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aac_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aac_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aac_interframe_buf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aac_passthru.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aac_downmix.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_framesync.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aacplus_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aacplus_interframe_buf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_encoder_iboot.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_encoder_scheduler_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_encoder_scheduler_data.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_scheduler_dwd_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_src_postprocess_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_src_postprocess.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_src_interframe_buf.c 

ifeq ($(RAP_SRSTRUVOL_SUPPORT),y)
BRAP_DEFINES += RAP_SRSTRUVOL_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_srs_TruVolume_post_process.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_srs_TruVolume_table.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_srs_TruVolume_interframe_buf.c
endif

ifeq ($(RAP_SRSTRUVOL_CERTIFICATION),y)
BRAP_DEFINES += RAP_SRSTRUVOL_CERTIFICATION
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_pcm_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_pcm_framesync.c
endif
	
ifeq ($(RAP_DOLBYVOLUME_SUPPORT),y)
BRAP_DEFINES += RAP_DOLBYVOLUME_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dolby_volume.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dolby_volume_table.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dolby_volume_interframe_buf.c 
endif

BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dsola_postprocess_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dsola_postprocess.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dsola_interframe_buf.c 

BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_passthru.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_stg0.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_stg1.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_stg2.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_stg3.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_stg4.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_table_stg0.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_table_stg1.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_table_stg2.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_table_stg3.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_decode_table_stg4.c	\
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_aache_interframe_buf.c

BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dts_passthru.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dts_framesync.c

BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dvdlpcm_framesync.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dvdlpcm_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_dvdlpcm_interframe_buf.c

ifeq ($(RAP_DDP_SUPPORT),y)
BRAP_DEFINES += BRAP_DDP_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_framesync.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_be_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_fe_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_interframe_buf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_passthru.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_passthru_interframe_buf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_passthru_table.c \
    $(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_convert.c \
    $(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddpdep_framesync.c
else
 BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ddp_stub.c
endif

ifeq ($(RAP_I2S_COMPRESS_SUPPORT),y)
BRAP_DEFINES += RAP_I2S_COMPRESS_SUPPORT
endif

ifeq ($(WMA_CERTIFICATION),y)
BRAP_DEFINES += WMA_CERTIFICATION
endif

ifeq ($(RAP_AC3_SUPPORT),y)
BRAP_DEFINES += RAP_AC3_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_passthru_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_convert.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_be_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_fe_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_interframe_buf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_framesync.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_passthru.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_passthru_interframe_buf.c
else
 BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_ac3_stub.c 
endif 

ifeq ($(RAP_WMA_SUPPORT),y)
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wma_std_framesync.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wma_std_interframe_buf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wma_std_stg1_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wma_std_stg1_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wmapro_framesync.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wmapro_decode_stg0.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wmapro_decode_stg1.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wmapro_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wmapro_passthru.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wmapro_interframe_buf.c 
else
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wma_stub.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_wmapro_stub.c
endif

ifeq ($(RAP_PCMWAV_SUPPORT),y)
BRAP_DEFINES += RAP_PCMWAV_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_pcmwav_framesync.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_pcmwav_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm130/firmware/zsp500/brap_pcmwav_interframe_buf.c
endif

