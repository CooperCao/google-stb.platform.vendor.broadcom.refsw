############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# Module Description:
#   This file defines a list of nexus modules that will be included in the build.
#
############################################################

ifneq ($(NEXUS_PLATFORM_DEFINES),)
$(warning NEXUS_PLATFORM_DEFINES is being cleared. Currently set to NEXUS_PLATFORM_DEFINES=$(NEXUS_PLATFORM_DEFINES))
endif
NEXUS_PLATFORM_DEFINES =

include $(NEXUS_TOP)/platforms/common/build/platform_version.inc
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_VERSION_MAJOR=$(NEXUS_PLATFORM_VERSION_MAJOR)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_VERSION_MINOR=$(NEXUS_PLATFORM_VERSION_MINOR)

BCHP_VER_LOWER := $(shell echo $(BCHP_VER) | tr A-Z a-z)

include $(MAGNUM)/basemodules/chp/build/bchp_rdb.inc

ifeq ( $(wildcard ${BCHP_RDB_PATH}/bchp_common.h),)
$(warning ${BCHP_RDB_PATH} does not exist)
endif

# Define Module Options

##############################
#
# per-platform options
# This is the preferred method for customizing your platform's build instead of modifying this common file

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_options.inc

ifneq ($(NEXUS_CUSTOM_PLATFORM_INC),)
include $(NEXUS_TOP)/$(NEXUS_CUSTOM_PLATFORM_INC)
endif

##############################
#
# include modules
#
# most modules are opt-out: you get the module unless you define NEXUS_<MODULE>_SUPPORT=n

# allow users to use a customized platform.inc for their board
# this must be placed before all other module .inc's for IMG loading support
ifeq ($(PLATFORM_INC),)
PLATFORM_INC=$(NEXUS_TOP)/platforms/common/build/platform.inc
endif
include $(PLATFORM_INC)

include $(NEXUS_TOP)/base/base.inc
include $(NEXUS_TOP)/modules/core/core.inc

ifneq ($(NEXUS_PWM_SUPPORT),n)
include $(NEXUS_TOP)/modules/pwm/pwm.inc
endif
ifneq ($(NEXUS_I2C_SUPPORT),n)
include $(NEXUS_TOP)/modules/i2c/i2c.inc
endif
ifneq ($(NEXUS_GPIO_SUPPORT),n)
include $(NEXUS_TOP)/modules/gpio/gpio.inc
endif
ifneq ($(NEXUS_LED_SUPPORT),n)
include $(NEXUS_TOP)/modules/led/led.inc
endif
ifneq ($(NEXUS_IR_INPUT_SUPPORT),n)
include $(NEXUS_TOP)/modules/ir_input/ir_input.inc
endif
ifneq ($(NEXUS_IR_BLASTER_SUPPORT),n)
include $(NEXUS_TOP)/modules/ir_blaster/ir_blaster.inc
endif
ifneq ($(NEXUS_INPUT_CAPTURE_SUPPORT),n)
include $(NEXUS_TOP)/modules/input_capture/input_capture.inc
endif
ifneq ($(NEXUS_UHF_INPUT_SUPPORT),n)
include $(NEXUS_TOP)/modules/uhf_input/uhf_input.inc
endif
ifneq ($(NEXUS_RFM_SUPPORT),n)
include $(NEXUS_TOP)/modules/rfm/rfm.inc
endif
ifneq ($(NEXUS_KEYPAD_SUPPORT),n)
include $(NEXUS_TOP)/modules/keypad/keypad.inc
endif
ifneq ($(NEXUS_FRONTEND_SUPPORT),n)
include $(NEXUS_TOP)/modules/frontend/build/frontend.inc
endif
ifneq ($(NEXUS_SPI_SUPPORT),n)
include $(NEXUS_TOP)/modules/spi/spi.inc
endif

ifneq ($(NEXUS_SECURITY_SUPPORT),n)
include $(NEXUS_TOP)/modules/security/security.inc
# SAGE requires SECURITY
ifeq ($(SAGE_SUPPORT),y)
include $(NEXUS_TOP)/modules/sage/sage.inc
endif
endif

NEXUS_PICTURE_DECODER_SUPPORT?=y
ifneq ($(NEXUS_PICTURE_DECODER_SUPPORT),n)
include $(NEXUS_TOP)/modules/picture_decoder/picture_decoder.inc
endif
ifneq ($(NEXUS_HDMI_INPUT_SUPPORT),n)
include $(NEXUS_TOP)/modules/hdmi_input/hdmi_input.inc
endif

ifeq ($(V3D_SUPPORT),)
BCHP_HAS_V3D := $(shell test -e ${BCHP_RDB_PATH}/bchp_v3d_vpm*.h && echo y)
ifeq ($(BCHP_HAS_V3D),y)
V3D_SUPPORT := $(shell test -e $(NEXUS_TOP)/modules/graphicsv3d/graphicsv3d.inc && echo y)
endif
endif
ifeq ($(V3D_SUPPORT),y)
include $(NEXUS_TOP)/modules/graphicsv3d/graphicsv3d.inc
endif

ifneq ($(NEXUS_DMA_SUPPORT),n)
include $(NEXUS_TOP)/modules/dma/dma.inc
endif

ifneq ($(NEXUS_TRANSPORT_SUPPORT),n)
include $(NEXUS_TOP)/modules/transport/transport.inc
endif
ifneq ($(NEXUS_VIDEO_DECODER_SUPPORT),n)
include $(NEXUS_TOP)/modules/video_decoder/video_decoder.inc
else
BINT_DECODER_DISABLED := y
endif
ifneq ($(NEXUS_AUDIO_SUPPORT),n)
include $(NEXUS_TOP)/modules/audio/audio.inc
else
BINT_RAAGA_DISABLED := y
NEXUS_ASTM_SUPPORT ?= n
NEXUS_SYNC_CHANNEL_SUPPORT ?= n
endif
ifneq ($(NEXUS_SURFACE_SUPPORT),n)
include $(NEXUS_TOP)/modules/surface/surface.inc
endif
ifneq ($(NEXUS_ASTM_SUPPORT),n)
include $(NEXUS_TOP)/modules/astm/astm.inc
endif
ifneq ($(NEXUS_DISPLAY_SUPPORT),n)
include $(NEXUS_TOP)/modules/display/display.inc
else
BINT_DISPLAY_DISABLED := y
endif
ifneq ($(NEXUS_GRAPHICS2D_SUPPORT),n)
include $(NEXUS_TOP)/modules/graphics2d/graphics2d.inc
endif
ifneq ($(NEXUS_SYNC_CHANNEL_SUPPORT),n)
include $(NEXUS_TOP)/modules/sync_channel/sync_channel.inc
endif
ifneq ($(NEXUS_HDMI_OUTPUT_SUPPORT),n)
include $(NEXUS_TOP)/modules/hdmi_output/hdmi_output.inc
else
# hdmi_dvo is mutually exclusive with hdmi_output.
# it is also seldom used, so default off.
ifeq ($(NEXUS_HDMI_DVO_SUPPORT),y)
include $(NEXUS_TOP)/modules/hdmi_dvo/hdmi_dvo.inc
endif
endif
ifneq ($(NEXUS_CEC_SUPPORT),n)
include $(NEXUS_TOP)/modules/cec/cec.inc
endif
ifneq ($(NEXUS_SMARTCARD_SUPPORT),n)
include $(NEXUS_TOP)/modules/smartcard/smartcard.inc
endif
ifeq ($(NSK2_SUPPORT),y)
include ${NEXUS_TOP}/extensions/nsk2/nsk2hdi/nsk2hdi.inc
endif
ifeq ($(NSK2_EMM_SUPPORT),y)
include ${NEXUS_TOP}/extensions/nsk2/nsk2hdx/emm_ext/nsk2emm_ext.inc
endif
ifeq ($(NSK2_SMARTCARD_SUPPORT),y)
include ${NEXUS_TOP}/extensions/nsk2/nsk2hdx/smartcard_ext/nsk2smartcard_ext.inc
endif
ifeq ($(NEXUS_DVB_CI_SUPPORT),y)
include $(NEXUS_TOP)/modules/dvb_ci/dvb_ci.inc
endif
ifneq ($(NEXUS_SURFACE_COMPOSITOR_SUPPORT),n)
include $(NEXUS_TOP)/modules/surface_compositor/surface_compositor.inc
endif
ifneq ($(NEXUS_INPUT_ROUTER_SUPPORT),n)
include $(NEXUS_TOP)/modules/input_router/input_router.inc
endif
ifneq ($(NEXUS_SIMPLE_DECODER_SUPPORT),n)
include $(NEXUS_TOP)/modules/simple_decoder/simple_decoder.inc
include $(NEXUS_TOP)/modules/simple_audio_playback/simple_audio_playback.inc
endif
ifneq ($(NEXUS_UART_SUPPORT),n)
include $(NEXUS_TOP)/modules/uart/uart.inc
endif
ifneq ($(NEXUS_VIDEO_ENCODER_SUPPORT),n)
include $(NEXUS_TOP)/modules/video_encoder/video_encoder.inc
ifneq ($(NEXUS_STREAM_MUX_SUPPORT),n)
# stream mux and file mux require video encoder
include $(NEXUS_TOP)/modules/stream_mux/stream_mux.inc
endif
ifneq ($(NEXUS_FILE_MUX_SUPPORT),n)
include $(NEXUS_TOP)/modules/file_mux/file_mux.inc
endif
else
BINT_VICE2_DISABLED := y
endif
ifneq ($(NEXUS_FILE_SUPPORT),n)
include $(NEXUS_TOP)/modules/file/file.inc
endif
ifneq ($(NEXUS_PLAYBACK_SUPPORT),n)
include $(NEXUS_TOP)/modules/playback/playback.inc
endif
ifneq ($(NEXUS_RECORD_SUPPORT),n)
include $(NEXUS_TOP)/modules/record/record.inc
endif
ifeq ($(NEXUS_ASP_SUPPORT),y)
include $(NEXUS_TOP)/modules/asp/asp.inc
endif

##############################
#
# opt-in modules

ifeq ($(NEXUS_TOUCHPAD_SUPPORT),y)
    include $(NEXUS_TOP)/modules/touchpad/touchpad.inc
endif
ifeq ($(NEXUS_TEMP_MONITOR_SUPPORT),y)
    include $(NEXUS_TOP)/modules/temp_monitor/temp_monitor.inc
endif

ifeq ($(SCM_SUPPORT),y)
ifeq ($(SAGE_SUPPORT),y)
$(error SCM_SUPPORT is not compatible with SAGE_SUPPORT)
endif
include $(NEXUS_TOP)/extensions/scm/scm.inc
endif

##############################
#
# misc defines

ifeq ($(NEXUS_MPOD_SUPPORT),y)
# deprecated. NEXUS_HAS_XXX is reserved for modules and is auto-defined.
NEXUS_PLATFORM_DEFINES += NEXUS_HAS_MPOD
endif
