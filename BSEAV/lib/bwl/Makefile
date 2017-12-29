###############################################################################
#   Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to the terms and
#   conditions of a separate, written license agreement executed between you and Broadcom
#   (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#   no license (express or implied), right to use, or waiver of any kind with respect to the
#   Software, and Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#   HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#   NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#   secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#   and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#   AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#   WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#   THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#   OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#   LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#   OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#   USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#   LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#   EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#   USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#   THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#   ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#   LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#   ANY LIMITED REMEDY.
#
###############################################################################
B_THIS_DIR:=BSEAV/lib/bwl

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
NEXUS_BIN_DIR = $(NEXUS_TOP)/../$(B_REFSW_OBJ_DIR)/nexus/bin
BWL_OBJ_DIR = $(NEXUS_TOP)/../$(B_REFSW_OBJ_DIR)/BSEAV/lib/bwl
BWL_SRC_DIR = $(NEXUS_TOP)/../BSEAV/lib/bwl/src
CONNECTIVITY_OBJ = $(NEXUS_TOP)/../$(B_REFSW_OBJ_DIR)/BSEAV/connectivity
CONNECTIVITY_SRC = $(NEXUS_TOP)/../BSEAV/connectivity

WLAN_DRIVER_PATH ?= BSEAV/connectivity/wlan/core
all: ${BWL_OBJ_DIR} bwl_build_objs $(BWL_OBJ_DIR)/libbwl.a

clean:
	@echo [Clean ... $(BWL_OBJ_DIR)]
	@-$(RM) -rf $(BWL_OBJ_DIR)
###	@-$(RM) -f $(NEXUS_BIN_DIR)/libbwl.a

ifeq ($(ANDROID_BUILD),y)
BWL_CFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DB_ANDROID_BUILD
endif

BWL_CFLAGS += -I$(NEXUS_TOP)/../$(WLAN_DRIVER_PATH)/src/include
BWL_CFLAGS += -DBDBG_DEBUG_BUILD=1
BWL_CFLAGS += -DNEXUS_SERVER_SUPPORT=1
BWL_CFLAGS += -D_GNU_SOURCE=1
BWL_CFLAGS += -DLINUX
BWL_CFLAGS += -pipe
BWL_CFLAGS += -D_FILE_OFFSET_BITS=64
BWL_CFLAGS += -D_LARGEFILE_SOURCE
BWL_CFLAGS += -D_LARGEFILE64_SOURCE
BWL_CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE
BWL_CFLAGS += -DNEXUS_POWER_MANAGEMENT
BWL_CFLAGS += -DBHSM_AUTO_TEST
BWL_CFLAGS += -DBSP_BSECK_COMMAND
BWL_CFLAGS += -DCOMMON_CRYPTO_28NM_CHIP=1
BWL_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=42
BWL_CFLAGS += -DCOMMON_CRYPTO_28NM_CHIP=1
BWL_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=42
BWL_CFLAGS += -I$(NEXUS_TOP)/platforms/common/include
BWL_CFLAGS += -I$(NEXUS_TOP)/platforms
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/core/include
BWL_CFLAGS += -I$(NEXUS_TOP)/base/include/public
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/pwm/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/i2c/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/gpio/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/led/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/ir_input/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/ir_blaster/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/input_capture/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/keypad/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/frontend/common/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/spi/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/security/include
BWL_CFLAGS += -I$(NEXUS_TOP)/extensions/security/keyladder/include/40nm
BWL_CFLAGS += -I$(NEXUS_TOP)/extensions/security/otpmsp/include/40nm
BWL_CFLAGS += -I$(NEXUS_TOP)/extensions/security/otp_id/include/40nm
BWL_CFLAGS += -I$(NEXUS_TOP)/extensions/security/otp_key/include
BWL_CFLAGS += -I$(NEXUS_TOP)/extensions/security/usercmd/include/40nm
BWL_CFLAGS += -I$(NEXUS_TOP)/extensions/security/rawcommand/include
BWL_CFLAGS += -I$(NEXUS_TOP)/extensions/security/bseckcmd/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/picture_decoder/include
BWL_CFLAGS += -I$(NEXUS_TOP)/extensions/picture_decoder/sid_video_decoder/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/hdmi_input/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/graphicsv3d/vc5/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/transport/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/dma/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/video_decoder/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/audio/include/ape_raaga
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/audio/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/surface/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/astm/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/display/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/graphics2d/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/sync_channel/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/hdmi_output/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/cec/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/smartcard/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/surface_compositor/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/input_router/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/simple_decoder/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/file/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/file/include/linuxuser
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/playback/include
BWL_CFLAGS += -I$(NEXUS_TOP)/modules/record/include
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/basemodules/kni/linuxuser
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/basemodules/std
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/basemodules/std/config
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/basemodules/std/types/linuxuser
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/basemodules/dbg
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/basemodules/chp/include/common
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/basemodules/chp/src/common
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/basemodules/reg
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/basemodules/err
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/commonutils/lst
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/commonutils/img
BWL_CFLAGS += -I$(NEXUS_TOP)/../magnum/commonutils/tee
BWL_CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/bfile

BWL_CFLAGS += -I$(NEXUS_TOP)/../$(WLAN_DRIVER_PATH)/src/include
BWL_CFLAGS += -I$(NEXUS_TOP)/../$(WLAN_DRIVER_PATH)/router/src/include
BWL_CFLAGS += -I$(NEXUS_TOP)/../$(WLAN_DRIVER_PATH)/components/shared
BWL_CFLAGS += -I$(NEXUS_TOP)/../$(WLAN_DRIVER_PATH)/src/shared/bcmwifi/include
BWL_CFLAGS += -I$(NEXUS_TOP)/../$(WLAN_DRIVER_PATH)/src/wl/exe

BWL_CFLAGS += -I$(NEXUS_TOP)/../$(WLAN_DRIVER_PATH)/components/shared
BWL_CFLAGS += -I$(NEXUS_TOP)/../$(WLAN_DRIVER_PATH)/src/shared/bcmwifi/include
BWL_CFLAGS += -I$(NEXUS_TOP)/../$(WLAN_DRIVER_PATH)/src/wl/exe

BWL_CFLAGS += -DNEXUS_PLATFORM_VERSION_MAJOR=16
BWL_CFLAGS += -DNEXUS_PLATFORM_VERSION_MINOR=3
BWL_CFLAGS += -DNEXUS_HAS_XPT_DMA
BWL_CFLAGS += -DNEXUS_NUM_SID_VIDEO_DECODERS=1
BWL_CFLAGS += -DBCHP_VER=BCHP_VER_A0
BWL_CFLAGS += -DNEXUS_PROFILE_OS_linuxuser
BWL_CFLAGS += -DB_REFSW_MINIMAL
BWL_CFLAGS += -DBBCP_SUPPORT
BWL_CFLAGS += -DNEXUS_SECURITY_ZEUS_VERSION_MAJOR=4
BWL_CFLAGS += -DNEXUS_SECURITY_ZEUS_VERSION_MINOR=2
BWL_CFLAGS += -DNEXUS_SECURITY_EXT_KEY_IV
BWL_CFLAGS += -DNEXUS_SECURITY_HAS_ASKM=1
BWL_CFLAGS += -DNEXUS_SECURITY_CHIP_SIZE=28
BWL_CFLAGS += -DNEXUS_KEYLADDER=1
BWL_CFLAGS += -DNEXUS_HAS_KEYLADDER_SUPPORT=1
BWL_CFLAGS += -DNEXUS_OTPMSP=1
BWL_CFLAGS += -DNEXUS_OTPID=1
BWL_CFLAGS += -DNEXUS_OTPKEY=1
BWL_CFLAGS += -DNEXUS_USERCMD=1
BWL_CFLAGS += -DNEXUS_SECURITY_RAWCMD=1
BWL_CFLAGS += -DNEXUS_SECURITY_BSECK_COMMAND=1
BWL_CFLAGS += -DBXPT_MESG_DONT_AUTO_ENABLE_PID_CHANNEL
BWL_CFLAGS += -DB_REFSW_DSS_SUPPORT=1
BWL_CFLAGS += -DB_REFSW_DSS_SUPPORT=1
BWL_CFLAGS += -DNEXUS_VBI_SUPPORT=1
BWL_CFLAGS += -DNEXUS_HAS_PLATFORM
BWL_CFLAGS += -DNEXUS_HAS_CORE
BWL_CFLAGS += -DNEXUS_HAS_PWM
BWL_CFLAGS += -DNEXUS_HAS_I2C
BWL_CFLAGS += -DNEXUS_HAS_GPIO
BWL_CFLAGS += -DNEXUS_HAS_LED
BWL_CFLAGS += -DNEXUS_HAS_IR_INPUT
BWL_CFLAGS += -DNEXUS_HAS_IR_BLASTER
BWL_CFLAGS += -DNEXUS_HAS_INPUT_CAPTURE
BWL_CFLAGS += -DNEXUS_HAS_KEYPAD
BWL_CFLAGS += -DNEXUS_HAS_FRONTEND
BWL_CFLAGS += -DNEXUS_HAS_SPI
BWL_CFLAGS += -DNEXUS_HAS_SECURITY
BWL_CFLAGS += -DNEXUS_HAS_PICTURE_DECODER
BWL_CFLAGS += -DNEXUS_HAS_HDMI_INPUT
BWL_CFLAGS += -DNEXUS_HAS_GRAPHICSV3D
BWL_CFLAGS += -DNEXUS_HAS_TRANSPORT
BWL_CFLAGS += -DNEXUS_HAS_VIDEO_DECODER
BWL_CFLAGS += -DNEXUS_HAS_AUDIO
BWL_CFLAGS += -DNEXUS_HAS_SURFACE
BWL_CFLAGS += -DNEXUS_HAS_ASTM
BWL_CFLAGS += -DNEXUS_HAS_DISPLAY
BWL_CFLAGS += -DNEXUS_HAS_GRAPHICS2D
BWL_CFLAGS += -DNEXUS_HAS_SYNC_CHANNEL
BWL_CFLAGS += -DNEXUS_HAS_HDMI_OUTPUT
BWL_CFLAGS += -DNEXUS_HAS_CEC
BWL_CFLAGS += -DNEXUS_HAS_SMARTCARD
BWL_CFLAGS += -DNEXUS_HAS_SURFACE_COMPOSITOR
BWL_CFLAGS += -DNEXUS_HAS_INPUT_ROUTER
BWL_CFLAGS += -DNEXUS_HAS_SIMPLE_DECODER
BWL_CFLAGS += -DNEXUS_HAS_FILE
BWL_CFLAGS += -DNEXUS_HAS_PLAYBACK
BWL_CFLAGS += -DNEXUS_HAS_RECORD
BWL_CFLAGS += -DB_REFSW_STRICT_PRINTF_FORMAT=1
BWL_CFLAGS += -DBCHP_PWR_SUPPORT
BWL_CFLAGS += -DBCHP_PWR_HAS_RESOURCES
BWL_CFLAGS += -DBCHP_UNIFIED_IMPL
BWL_CFLAGS += -DEAGLE_WIFI_DRIVER
BWL_CFLAGS += -DWIFI_CHIP=""
BWL_CFLAGS += -DWIFI_CHIPS=""
BWL_CFLAGS += -DBUILDCFG
BWL_CFLAGS += -DOSAPI_ENABLE_RT_SCHED
BWL_CFLAGS += -DBCMWPA2
BWL_CFLAGS += -DBCMSUP_PSK
BWL_CFLAGS += -DWLCNT
BWL_CFLAGS += -DWLPFN
BWL_CFLAGS += -DWLC_HIGH
BWL_CFLAGS += -frecord-gcc-switches
BWL_CFLAGS += -Wall
BWL_CFLAGS += -Wno-write-strings
BWL_CFLAGS += -O2
BWL_CFLAGS += -fPIC
BWL_CFLAGS += -DNO_NFC
BWL_CFLAGS += -DNO_IEEE1905
BWL_CFLAGS += -DNO_WIFI_WFD
BWL_CFLAGS += -DNO_WIFI_WPS
BWL_CFLAGS += -DNO_WIFI_P2P
BWL_CFLAGS += -DNO_WIFI_SECDMA
BWL_CFLAGS += -DNO_DUALSOFTAP
BWL_CFLAGS += -I$(BWL_SRC_DIR)/../include

ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= arm-linux-
else
ifeq ($(filter ${B_REFSW_ARCH}, mipsel-linux ), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= mipsel-linux-
else
B_REFSW_CROSS_COMPILE =
endif
endif
CC := $(B_REFSW_CROSS_COMPILE)gcc
AR := $(B_REFSW_CROSS_COMPILE)ar

${BWL_OBJ_DIR}:
	@mkdir -p $@

${NEXUS_BIN_DIR}:
###	@mkdir -p $@

$(BWL_OBJ_DIR)/bcmwifi_channels.o: $(BWL_SRC_DIR)/bcmwifi_channels.c $(BWL_SRC_DIR)/../include/bwl.h $(BWL_SRC_DIR)/../include/bcmwifi_channels.h
	@echo [Compiling ... bcmwifi_channels.c]
	@(cd $(BWL_OBJ_DIR) && $(CC) $(BWL_CFLAGS) -c $(BWL_SRC_DIR)/bcmwifi_channels.c)

$(BWL_OBJ_DIR)/bwl.o: $(BWL_SRC_DIR)/bwl.c $(BWL_SRC_DIR)/../include/bwl.h $(BWL_SRC_DIR)/../include/bwl_priv.h
	@echo [Compiling ... bwl.c]
	@(cd $(BWL_OBJ_DIR) && $(CC) $(BWL_CFLAGS) -c $(BWL_SRC_DIR)/bwl.c)

$(BWL_OBJ_DIR)/bwl_utils.o: $(BWL_SRC_DIR)/bwl_utils.c $(BWL_SRC_DIR)/../include/bwl.h $(BWL_SRC_DIR)/../include/bwl_priv.h
	@echo [Compiling ... bwl_utils.c]
	@(cd $(BWL_OBJ_DIR) && $(CC) $(BWL_CFLAGS) -c $(BWL_SRC_DIR)/bwl_utils.c)

$(BWL_OBJ_DIR)/bwl_linux.o: $(BWL_SRC_DIR)/bwl_linux.c $(BWL_SRC_DIR)/../include/bwl.h
	@echo [Compiling ... bwl_linux.c]
	@(cd $(BWL_OBJ_DIR) && $(CC) $(BWL_CFLAGS) -c $(BWL_SRC_DIR)/bwl_linux.c)

$(BWL_OBJ_DIR)/bwl_wlu_subcounters.o: $(BWL_SRC_DIR)/bwl_wlu_subcounters.c $(BWL_SRC_DIR)/../include/bwl.h
	@echo [Compiling ... bwl_wlu_subcounters.c]
	@(cd $(BWL_OBJ_DIR) && $(CC) $(BWL_CFLAGS) -c $(BWL_SRC_DIR)/bwl_wlu_subcounters.c)

bwl_build_objs: $(BWL_OBJ_DIR)/bcmwifi_channels.o $(BWL_OBJ_DIR)/bwl.o $(BWL_OBJ_DIR)/bwl_utils.o $(BWL_OBJ_DIR)/bwl_linux.o $(BWL_OBJ_DIR)/bwl_wlu_subcounters.o
	@echo [TARGET ... bwl_build_objs] > /dev/null

$(BWL_OBJ_DIR)/libbwl.a: $(BWL_OBJ_DIR)/bwl.o $(BWL_OBJ_DIR)/bwl_utils.o $(BWL_OBJ_DIR)/bwl_linux.o $(BWL_OBJ_DIR)/bcmwifi_channels.o $(BWL_OBJ_DIR)/bwl_wlu_subcounters.o
	@echo [Building ... $(notdir $@)]
	@$(AR) rcs $(BWL_OBJ_DIR)/libbwl.a $(BWL_OBJ_DIR)/bwl.o $(BWL_OBJ_DIR)/bwl_utils.o $(BWL_OBJ_DIR)/bwl_linux.o $(BWL_OBJ_DIR)/bcmwifi_channels.o $(BWL_OBJ_DIR)/bwl_wlu_subcounters.o

-include *.d
