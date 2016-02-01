#############################################################################
#
#       Copyright (c) 2003-2013, Broadcom Corporation.
#       All rights reserved.
#       Confidential Property of Broadcom Corporation.
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#############################################################################

# rap control build include file
#
# Many of audio algorithms (decoders, encoders, post processing) need license. 
# Hence this makefile uses build flags to conditionally compile the required
# algorithms. These flags are explained below. Each build flag is explained
# in following format.
# Build Flag (Default State): Description
# The build flag that is by default "OFF", can be changed by defining it
# to "y". Similarly the build flag that is by default "ON", can be changed
# by defining it to "n".
#
# RAP_MPEG_SUPPORT (ON): Support MPEG decoder. No licensing requirement.
# RAP_AACSBR_SUPPORT (ON): Support AAC/AAC-HE decoders. No licensing requirement.
# RAP_SRC_SUPPORT (ON): Support MPEG LSF. No licensing requirement.
# RAP_DDP_SUPPORT (OFF): Support DDP. License needed.
# RAP_AC3_SUPPORT (OFF): Support AC3. License needed.
# RAP_WMA_SUPPORT (OFF) : Support WMA. License needed.
# RAP_WMAPRO_SUPPORT (OFF) : Support WMA_PRO. License needed.
# RAP_AUDIODESC_SUPPORT (OFF): Support Audio Descriptor.
# RAP_SRSTRUVOL_SUPPORT (OFF): Support SRS Tru Volume. License needed
# RAP_DTSENC_SUPPORT (OFF): Support DTS Encode.
# RAP_AC3ENC_SUPPORT (OFF): Support AC3 Encode.
# RAP_DSOLA_SUPPORT (ON): Support DSOLA Passthru. No licensing requirement.
# RAP_LPCMDVD_SUPPORT (ON): Support LPCM DVD decoder. No licensing requirement.
# RAP_SRSHD_SUPPORT (OFF): Support True SurroundHD. License needed
# RAP_DOLBYVOL_SUPPORT (OFF) : Support Dolby volume processing . License needed.
# RAP_DTSBROADCAST_SUPPORT (OFF) : Support DTS Broadcast decoder . License needed.
# RAP_PCMWAV_SUPPORT (ON) : Support PCM WAVE decoder . No licensing requirement.
# RAP_DV258_SUPPORT (OFF): Support DV258 i.e. Dolby Volume multichannel. License needed
# RAP_DRA_SUPPORT (OFF) : Support DRA decoder . License needed.
# RAP_REALAUDIOLBR_SUPPORT (OFF) : Support Real Audio Lbr
# RAP_CUSTOMVOICE_SUPPORT (OFF): Support Custom Voice. License needed
# RAP_BTSCENC_SUPPORT (ON): Support BTSC Encode. No licensing requirement.
# RAP_SCM_SUPPORT (OFF): Support SCM. License needed
# RAP_REALVIDEO_SUPPORT  (OFF): Support Real Video. License needed
# RAP_AVL_SUPPORT (ON): Support Auto Volume Level. No licensing requirement.
# RAP_BRCM3DSURROUND_SUPPORT (ON) : Support Broadcom 3D. No licensing requirement.
#

# Define this module
MAGNUM_MODULES += BRAP

# Determine ZSP family
ifeq ($(findstring $(BCHP_CHIP), 7405 7325 7335 7336),$(BCHP_CHIP))
BRAP_ZSP := zsp500
else
BRAP_ZSP := zsp800
endif

# Module Includes
BRAP_INCLUDES += $(MAGNUM)/portinginterface/rap/src/fmm65
BRAP_INCLUDES += $(MAGNUM)/portinginterface/rap/include/fmm65
BRAP_INCLUDES += $(MAGNUM)/portinginterface/rap/src/fmm65/firmware/interface
BRAP_INCLUDES += $(MAGNUM)/portinginterface/rap/src/fmm65/firmware/rdbvars
BRAP_INCLUDES += $(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)


ifeq ($(BRAP_IMG_ONLY),)
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_fwproc_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/interface/brap_cit_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/interface/brap_af_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_dsp.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_dspchn.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_dec.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_fmm.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_rbuf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_srcch.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_mixer.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_op.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_hifidac.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_spdiffm.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_spdiffm_fw.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_rm.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_pb.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_int_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_mixer_soft_coeff.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_pvr.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_fwdwnld_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_fwif_priv.c 

ifeq ($(BCHP_CHIP),7550)
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_gfx_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_gfx.c
else	
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_src.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_cap.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_capport.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_dstch.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_src_coeff_2x.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_src_coeff_4x.c 	
endif 

ifeq ($(BCHP_CHIP),7468)
ifeq ($(RAP_REALVIDEO_SUPPORT),y)
PLATFORM_DEFINES += RAP_REALVIDEO_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_videodecode.c	
endif	
endif
	
endif


BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_img.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_system_iboot.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_system_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_system_data.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/rdbvars/brap_system_rdbvars_$(BCHP_CHIP).c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_cdb_passthru_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dec_tsm_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dec_tsm_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_system_audio_decode_task_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_system_gfx_task_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_system_video_task_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_cdb_passthru_tables.c\
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_cdb_passthru_inter_frame.c
	
ifneq ($(RAP_MPEG_SUPPORT),n)
PLATFORM_DEFINES += RAP_MPEG_SUPPORT
PLATFORM_DEFINES += RAP_MPEG_PASSTHRU_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_mpeg1_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_mpeg1_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_mpeg1_decode_inter_frame.c
endif

ifeq ($(RAP_MS11_SUPPORT),y)
PLATFORM_DEFINES += RAP_MS11_SUPPORT
PLATFORM_DEFINES += RAP_DDRE_SUPPORT
RAP_MULTISTREAM_DECODER_SUPPORT := y
PLATFORM_DEFINES +=RAP_MULTISTREAM_DECODER_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dd_reenc_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dd_reenc_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dd_reenc_tables.c
endif

ifeq ($(RAP_MS10_SUPPORT),y)
PLATFORM_DEFINES += RAP_MS10_SUPPORT
RAP_MULTISTREAM_DECODER_SUPPORT := y
PLATFORM_DEFINES +=RAP_MULTISTREAM_DECODER_SUPPORT
endif

ifeq ($(RAP_MULTISTREAM_DECODER_SUPPORT),y)
PLATFORM_DEFINES += RAP_DDP_SUPPORT
PLATFORM_DEFINES += RAP_DDP_PASSTHRU_SUPPORT

PLATFORM_DEFINES += RAP_AC3_SUPPORT
PLATFORM_DEFINES += RAP_AC3_PASSTHRU_SUPPORT

PLATFORM_DEFINES += RAP_AACSBR_SUPPORT
PLATFORM_DEFINES += RAP_AACSBR_PASSTHRU_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_be_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_fe_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_be_decode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_fe_decode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_be_decode_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_fe_decode_inter_frame.c

#RAP_DDP_TO_AC3_SUPPORT
PLATFORM_DEFINES += RAP_DDP_TO_AC3_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_convert.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_convert_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_convert_inter_frame.c 	

BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg0_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg1_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg2_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg3_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg0_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg1_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg2_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg3_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg0.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg1.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg2.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_pulse_decode_stg3.c
	
else
	
ifeq ($(RAP_DDP_SUPPORT),y)
PLATFORM_DEFINES += RAP_DDP_SUPPORT
PLATFORM_DEFINES += RAP_DDP_PASSTHRU_SUPPORT	
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_be_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_fe_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_be_decode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_fe_decode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_be_decode_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_fe_decode_inter_frame.c

#RAP_DDP_TO_AC3_SUPPORT
PLATFORM_DEFINES += RAP_DDP_TO_AC3_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_convert.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_convert_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_convert_inter_frame.c 	
endif
	
ifeq ($(RAP_AC3_SUPPORT),y)
PLATFORM_DEFINES += RAP_AC3_SUPPORT
PLATFORM_DEFINES += RAP_AC3_PASSTHRU_SUPPORT	
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_be_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_fe_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_be_decode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_fe_decode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_be_decode_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_fe_decode_inter_frame.c 
endif

ifneq ($(RAP_AACSBR_SUPPORT),n)
PLATFORM_DEFINES += RAP_AACSBR_SUPPORT
PLATFORM_DEFINES += RAP_AACSBR_PASSTHRU_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg0_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg1_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg2_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg3_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg4_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg0_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg1_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg2_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg3_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg4_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg0.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg1.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg2.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg3.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_decode_stg4.c
endif	
endif

ifeq ($(RAP_MULTISTREAM_DECODER_SUPPORT),y)
ifneq ($(RAP_FWMIXER_SUPPORT),n)
PLATFORM_DEFINES += RAP_FWMIXER_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_fw_mixer_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_fw_mixer_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_fw_mixer_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_fw_mixer_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_fw_mixer_ids_inter_frame.c	
endif
endif

ifeq ($(RAP_WMA_SUPPORT),y)
PLATFORM_DEFINES += RAP_WMASTD_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wma_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wma_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wma_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wma_decode_interframe.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wma_ids_interframe.c
endif

ifeq ($(RAP_WMAPRO_SUPPORT),y)
PLATFORM_DEFINES += RAP_WMAPRO_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wmapro_ids_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wmapro_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wmapro_decode_stg0.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wmapro_decode_stg0_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wmapro_decode_stg0_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wmapro_decode_stg1.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wmapro_decode_stg1_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wmapro_decode_stg1_tables.c

PLATFORM_DEFINES += RAP_WMAPROPASSTHRU_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_wmapro_passthru_code.c
endif

ifneq ($(RAP_SRC_SUPPORT),n)
PLATFORM_DEFINES += RAP_SRC_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_src_PP.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_src_PP_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_src_PP_table.c 
endif

ifeq ($(RAP_AUDIODESC_SUPPORT),y)
PLATFORM_DEFINES += RAP_AUDIODESC_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_fade_control.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pan_control.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pan_control_interframe_buf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pan_control_tables.c

#PCM Router	
#PLATFORM_DEFINES += RAP_PCMROUTER_SUPPORT
#BRAP_SOURCES += \
#	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pcm_router.c
endif
#	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_fade_control_interframe_buf.c \
#	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_fade_control_tables.c \
	

ifeq ($(RAP_SRSTRUVOL_SUPPORT),y)
PLATFORM_DEFINES += RAP_SRSTRUVOL_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_srs_truvolume_post_process.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_srs_truvolume_interframe_buf.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_srs_truvolume_table.c 
endif

ifeq ($(RAP_DTSENC_SUPPORT),y)
PLATFORM_DEFINES += RAP_DTSENC_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dts_broadcast_encode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dts_broadcast_encode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dts_broadcast_encode_inter_frame.c 
endif
	
ifneq ($(RAP_DSOLA_SUPPORT),n)
PLATFORM_DEFINES += RAP_DSOLA_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dsola.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dsola_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dsola_inter_frame.c 
endif
	
ifeq ($(RAP_MULTISTREAM_DECODER_SUPPORT),y)
PLATFORM_DEFINES += RAP_AC3ENC_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dd_transcode_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dd_transcode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dd_transcode_inter_frame.c 
else
ifeq ($(RAP_AC3ENC_SUPPORT),y)
PLATFORM_DEFINES += RAP_AC3ENC_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_encode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_encode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_encode_inter_frame.c 
endif
endif

ifeq ($(findstring $(BCHP_CHIP),7325 7335 7336), $(BCHP_CHIP))
ifneq ($(RAP_BRCM3DSURROUND_SUPPORT),n)
PLATFORM_DEFINES += RAP_BRCM3DSURROUND_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/${BCHP_CHIP}/${BCHP_VER_LOWER}/image/brap_brcm3dsurround.c \
	$(MAGNUM)/portinginterface/rap/${BCHP_CHIP}/${BCHP_VER_LOWER}/image/brap_brcm3dsurround_tables.c \
	$(MAGNUM)/portinginterface/rap/${BCHP_CHIP}/${BCHP_VER_LOWER}/image/brap_brcm3dsurround_inter_frame.c 
endif
endif

ifeq ($(findstring $(BCHP_CHIP),7125 7340), $(BCHP_CHIP))
ifeq ($(RAP_AVL_SUPPORT),y)
PLATFORM_DEFINES += RAP_AVL_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_avl.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_avl_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_avl_tables.c
endif
endif


ifeq ($(findstring $(BCHP_CHIP),7550 7125), $(BCHP_CHIP))
ifneq ($(RAP_BTSC_SUPPORT),n)
PLATFORM_DEFINES += RAP_BTSC_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_btsc_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_btsc_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_btsc_inter_frame.c
endif
endif

ifeq ($(BCHP_CHIP),7550)
ifeq ($(RAP_SCM_SUPPORT),y)
PLATFORM_DEFINES += RAP_SCM_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_system_tableofhashes.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_system_scm_task_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_scm_priv.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/brap_scm.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_scm_1_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_scm_1_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_scm_1_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_scm_2_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_scm_2_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_scm_2_inter_frame.c
endif

ifeq ($(RAP_GFX_SUPPORT),y)
PLATFORM_DEFINES += RAP_GFX_SUPPORT	
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_gfx_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_gfx_decode_interframe.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_gfx_decode_tables.c
endif	
endif

ifeq ($(BCHP_CHIP),7468)
ifeq ($(RAP_REALVIDEO_SUPPORT),y)
PLATFORM_DEFINES += RAP_REALVIDEO_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_plf.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_plf_tables.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_ppd.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_stg1.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_stg1_tables.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_stg2.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_stg2_tables.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_stg3.c \
	$(MAGNUM)/portinginterface/rap/$(BCHP_CHIP)/$(BCHP_VER_LOWER)/image/brap_rv40_decode_stg3_tables.c
endif
endif

ifeq ($(FW_DBG),y)
PLATFORM_DEFINES += BRAP_P_FW_DBG_ENABLE
endif

ifneq ($(RAP_LPCMDVD_SUPPORT),n)
PLATFORM_DEFINES += RAP_LPCMDVD_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_lpcm_decode_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_lpcm_decode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_lpcm_decode_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dvdlpcm_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dvdlpcm_ids_inter_frame.c 
endif

ifeq ($(RAP_DOLBYVOL_SUPPORT),y)
PLATFORM_DEFINES += RAP_DOLBYVOL_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_volume.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_volume_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_volume_inter_frame.c 
endif

ifeq ($(RAP_DTSHD_SUPPORT),y)
PLATFORM_DEFINES += RAP_DTSHD_PASSTHRU_SUPPORT
PLATFORM_DEFINES += RAP_DTSHD_SUPPORT
RAP_DTSHD_DTSBROADCAST_SUPPORT := y
endif

ifeq ($(RAP_DTSBROADCAST_SUPPORT),y)
PLATFORM_DEFINES += RAP_DTSBROADCAST_SUPPORT
PLATFORM_DEFINES += RAP_DTSBROADCAST_PASSTHRU_SUPPORT	
RAP_DTSHD_DTSBROADCAST_SUPPORT := y
endif

ifeq ($(RAP_DTSHD_DTSBROADCAST_SUPPORT),y)
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dtshd_dec_stg0.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dtshd_dec_stg0_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dtshd_dec_stg0_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dtshd_dec_stg1.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dtshd_dec_stg1_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dtshd_dec_stg1_inter_frame.c
endif


ifneq ($(RAP_PCMWAV_SUPPORT),n)
PLATFORM_DEFINES += RAP_PCMWAV_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pcmwav_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pcmwav_ids_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pcmwav_decode_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pcmwav_decode_inter_frame.c
endif

ifeq ($(RAP_MP3ENC_SUPPORT),y)
PLATFORM_DEFINES += RAP_MP3ENC_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_mp3_encode_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_mp3_encode_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_mp3_encode_inter_frame.c
endif

ifeq ($(RAP_DRA_SUPPORT),y)
PLATFORM_DEFINES += RAP_DRA_SUPPORT
PLATFORM_DEFINES += RAP_DRA_PASSTHRU_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dra_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dra_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dra_interframe_buf.c
endif

ifeq ($(RAP_SBCENC_SUPPORT),y)
PLATFORM_DEFINES += RAP_SBCENC_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_sbc_encode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_sbc_encode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_sbc_encode_inter_frame.c
endif


ifeq ($(RAP_INDEP_DELAY_DISABLED),y)
PLATFORM_DEFINES += RAP_INDEP_DELAY_DISABLED
endif


ifeq ($(RAP_AUTOMATED_UNIT_TEST),y)
PLATFORM_DEFINES += RAP_AUTOMATED_UNIT_TEST
endif

ifneq ($(RAP_PCM_SUPPORT),n)
PLATFORM_DEFINES += RAP_PCM_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pcm_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_pcm_ids_inter_frame.c
endif

ifeq ($(RAP_REALAUDIOLBR_SUPPORT),y)
PLATFORM_DEFINES += RAP_REALAUDIOLBR_SUPPORT
PLATFORM_DEFINES += RAP_REALAUDIOLBR_PASSTHRU_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ralbr_decode.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ralbr_decode_table.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ralbr_interframe_buf.c
endif

ifeq ($(RAP_CUSTOMVOICE_SUPPORT),y)
PLATFORM_DEFINES += RAP_CUSTOMVOICE_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_customvoice_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_customvoice_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_customvoice_tables.c 
endif

ifeq ($(RAP_SRSHD_SUPPORT),y)
PLATFORM_DEFINES += RAP_SRSHD_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_trusurroundhd.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_trusurroundhd_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_trusurroundhd_tables.c 
endif

ifeq ($(RAP_DV258_SUPPORT),y)
PLATFORM_DEFINES += RAP_DV258_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dv258_code.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dv258_tables.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dv258_inter_frame.c
endif
# Following  audio pass through algorithms do not need license. Hence they
# get compiled unconditionally.
# AC3 PASSTHRU
# DDP PASSTHRU
# DTS BROADCAST PASSTHRU
# AAC/AACHE/DOLBY PULSE PASSTRHU
# MPEG PASSTHRU
# DRA PASSTHRU
# DTSHD PASSTHRU
# REALAUDIO LBR
#To turn them off Please export RAP_AC3_PASSTHRU_SUPPORT=n

ifeq ($(RAP_MULTISTREAM_DECODER_SUPPORT),y)
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dolby_ms_ddp_ids_inter_frame.c
else
ifneq ($(RAP_AC3_PASSTHRU_SUPPORT),n)
PLATFORM_DEFINES += RAP_AC3_PASSTHRU_SUPPORT	
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ac3_ids_inter_frame.c
endif	

ifneq ($(RAP_DDP_PASSTHRU_SUPPORT),n)
PLATFORM_DEFINES += RAP_DDP_PASSTHRU_SUPPORT	
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ddp_ids_inter_frame.c
endif	

endif

ifneq ($(RAP_DTSBROADCAST_PASSTHRU_SUPPORT),n)
PLATFORM_DEFINES += RAP_DTSBROADCAST_PASSTHRU_SUPPORT	
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dts_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dts_ids_inter_frame.c
endif

ifneq ($(RAP_AACSBR_PASSTHRU_SUPPORT),n)
PLATFORM_DEFINES += RAP_AACSBR_PASSTHRU_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_loas_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_loas_ids_inter_frame.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_adts_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_aache_adts_ids_inter_frame.c
endif

ifneq ($(RAP_MPEG_PASSTHRU_SUPPORT),n)
PLATFORM_DEFINES += RAP_MPEG_PASSTHRU_SUPPORT	
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_mpeg1_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_mpeg1_ids_inter_frame.c
endif	

ifneq ($(RAP_DRA_PASSTHRU_SUPPORT),n)
PLATFORM_DEFINES += RAP_DRA_PASSTHRU_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dra_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dra_ids_inter_frame.c
endif

ifneq ($(RAP_DTSHD_PASSTHRU_SUPPORT),n)
PLATFORM_DEFINES += RAP_DTSHD_PASSTHRU_SUPPORT
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dtshd_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_dtshd_ids_inter_frame.c
endif

ifneq ($(RAP_REALAUDIOLBR_PASSTHRU_SUPPORT),n)
PLATFORM_DEFINES += RAP_REALAUDIOLBR_PASSTHRU_SUPPORT	
BRAP_SOURCES += \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ralbr_ids.c \
	$(MAGNUM)/portinginterface/rap/src/fmm65/firmware/$(BRAP_ZSP)/brap_ralbr_ids_inter_frame.c
endif

#MAGNUM_CFLAGS += $(addprefix -D, ${PLATFORM_DEFINES})
#CFLAGS += $(addprefix -D, ${PLATFORM_DEFINES})
BRAP_DEFINES := $(PLATFORM_DEFINES)


	
	
       	
