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
# $brcm_Log: $
# 
############################################################

# Define this module
MAGNUM_MODULES += BSYNCLIB

# Module Includes
BSYNCLIB_INCLUDES += $(MAGNUM)/syslib/synclib/include

ifeq ($(BSYNCLIB_STUBS),y) 
BSYNCLIB_SOURCES += \
	$(MAGNUM)/syslib/synclib/src/bsynclib_stubs.c
else # BSYNCLIB_STUBS=n or not defined
BSYNCLIB_SOURCES += \
	$(MAGNUM)/syslib/synclib/src/bsynclib.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_priv.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_audio_sink.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_audio_source.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_video_format.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_video_sink.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_video_source.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_delay_element.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_resource_pool.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_rate_mismatch_detector.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_mute_control.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_state_machine.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_timer.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_algo.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_channel.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_channel_priv.c \
	$(MAGNUM)/syslib/synclib/src/bsynclib_channel_config.c 
endif

ifeq ($(BSYNCLIB_JTI_SUPPORT),y)
BSYNCLIB_DEFINES += BSYNCLIB_JITTER_TOLERANCE_IMPROVEMENT_SUPPORT
endif

