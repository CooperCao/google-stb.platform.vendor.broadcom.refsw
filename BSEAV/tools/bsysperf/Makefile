###############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################
# Nexus examples
B_THIS_DIR:=BSEAV/tools/bsysperf
PHONY := all boa clean clean_apps install nexus_headers clean_boa
.PHONY: ${PHONY}
.NOTPARALLEL: apps boa

ifeq ($(LINUXDIR),)
LINUXDIR = $(LINUX)
endif

THEAPPS = \
	bsysperf.cgi \
    bsysperf_server

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/bsysperf

${PHONY} ${THEAPPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile ${MAKECMDGOALS}

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

include $(NEXUS_TOP)/examples/Makefile_prologue.inc

APPS = $(THEAPPS)
APPS ?= $(APP)

vpath %.c ${NEXUS_TOP}/../BSEAV/tools/bsysperf

all: apps

apps: $(APPS)

LDFLAGS += -lpthread
MAIN_OBJS = ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.c ${NEXUS_TOP}/../${B_THIS_DIR}/bheaps.c
SERVER_OBJS = ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_server.c
EXTRA_OBJS += ${NEXUS_TOP}/../BSEAV/tools/bmemperf/common/bmemperf_lib.c
CLKGEN_PATH = ${NEXUS_TOP}/../magnum/basemodules/chp/src/common

CONNECTIVITY_OBJ = $(NEXUS_TOP)/../$(B_REFSW_OBJ_DIR)/BSEAV/connectivity
CONNECTIVITY_SRC = $(NEXUS_TOP)/../BSEAV/connectivity
BWL_DIR = $(CONNECTIVITY_OBJ)/wlan/bwl
ifneq ("$(wildcard $(CONNECTIVITY_SRC))","")
BWL_FILES = $(shell ls -1 $(CONNECTIVITY_SRC) )
else
BWL_FILES =
endif

# This cleans nexus and local apps
clean: clean_apps clean_boa clean_bwl

boa:
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/boa
clean_boa:
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/boa clean

clean_apps:
	@-$(RM) $(APPS) *.d *.o *.out
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.html
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.js
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf_flame.js
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.png
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.cgi
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.sh
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf_server
	@-$(RM) ${NEXUS_BIN_DIR}/bsysperf.css
	@-$(RM) ${NEXUS_BIN_DIR}/chart*.js
	@-$(RM) ${NEXUS_BIN_DIR}/boundingbox.js
	@-$(RM) ${NEXUS_BIN_DIR}/svgvml3d.js

clean_bwl:
	@-$(RM) -rf $(CONNECTIVITY_OBJ)/wlan/bwl $(CONNECTIVITY_OBJ)/wlan/bcmwifi

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -I${NEXUS_TOP}/../BSEAV/tools/bmemperf/include
CFLAGS += -I${LINUXDIR}/include
CFLAGS += -I${NEXUS_BIN_DIR}
CFLAGS += -Wimplicit-function-declaration -Wall -lm
ifeq ($(ANDROID_BUILD),y)
CFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DB_ANDROID_BUILD
endif

ifeq ($(BWL_FILES),)
libbwl.a:
	@echo [BWL files not detected]
else
EXTRA_OBJS += ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_wifi.c

LDFLAGS += -L$(B_REFSW_OBJ_ROOT)/nexus/bsysperf -lbwl
CFLAGS += -DBWL_SUPPORT
CFLAGS += -I$(B_REFSW_OBJ_ROOT)/BSEAV/connectivity/wlan/bwl

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

BWL_CFLAGS += -I$(NEXUS_TOP)/../BSEAV/connectivity/wlan/EAGLE_10_10_84/linux-external-media/sys/src/include
BWL_CFLAGS += -I$(NEXUS_TOP)/../BSEAV/connectivity/wlan/EAGLE_10_10_84/linux-external-media/sys/components/shared
BWL_CFLAGS += -I$(NEXUS_TOP)/../BSEAV/connectivity/wlan/EAGLE_10_10_84/linux-external-media/sys/src/shared/bcmwifi/include
BWL_CFLAGS += -I$(NEXUS_TOP)/../BSEAV/connectivity/wlan/EAGLE_10_10_84/linux-external-media/sys/src/wl/exe

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
BWL_CFLAGS += -I.
BWL_CFLAGS += -O2
BWL_CFLAGS += -fPIC
BWL_CFLAGS += -DNO_NFC
BWL_CFLAGS += -DNO_IEEE1905
BWL_CFLAGS += -DNO_WIFI_WFD
BWL_CFLAGS += -DNO_WIFI_WPS
BWL_CFLAGS += -DNO_WIFI_P2P
BWL_CFLAGS += -DNO_WIFI_SECDMA
BWL_CFLAGS += -DNO_DUALSOFTAP

bwl_sources:
	$(Q_)if [ ! -d $(BWL_DIR) ]; then \
		echo "[mkdir ... bwl $(BWL_DIR)]" ;\
		mkdir -p $(BWL_DIR) ;\
		echo "[Untarring ... bwl.tgz]" ;\
		(cd $(CONNECTIVITY_OBJ)/wlan && tar xvzf ${NEXUS_TOP}/../${B_THIS_DIR}/bwl.tgz ) ;\
	fi

bcmwifi_sources:
	$(Q_)if [ ! -d $(BWL_DIR)/../bcmwifi ]; then \
		echo "[Untarring ... bcmwifi_channels.tgz]" ;\
		(cd $(CONNECTIVITY_OBJ)/wlan && tar xvzf ${NEXUS_TOP}/../${B_THIS_DIR}/bcmwifi_channels.tgz ) ;\
	fi

bwl.o: $(BWL_DIR)/bwl.c
	@echo [Compiling ... $(notdir $<)]
	$(Q_)$(CC) $(BWL_CFLAGS) -c $(BWL_DIR)/bwl.c

bwl_linux.o: $(BWL_DIR)/bwl_linux.c
	@echo [Compiling ... $(notdir $<)]
	$(Q_)$(CC) $(BWL_CFLAGS) -c $(BWL_DIR)/bwl_linux.c

bwl_utils.o: $(BWL_DIR)/bwl_utils.c
	@echo [Compiling ... $(notdir $<)]
	$(Q_)$(CC) $(BWL_CFLAGS) -c $(BWL_DIR)/bwl_utils.c

bcmwifi_channels.o: $(BWL_DIR)/../bcmwifi/src/bcmwifi_channels.c
	@echo [Compiling ... $(notdir $<)]
	$(Q_)$(CC) $(BWL_CFLAGS) -c $(BWL_DIR)/../bcmwifi/src/bcmwifi_channels.c

libbwl.a: bwl_sources bcmwifi_sources bwl.o bwl_linux.o bwl_utils.o bcmwifi_channels.o
	@echo [Building ... $(notdir $@)]
	$(Q_)$(AR) rcs libbwl.a bwl*.o  bcmwifi_channels*.o

endif

# Always build with debug
CFLAGS += -g
CFLAGS += -D_LINUX_TYPES_H

# Implicit rule for building local apps
bsysperf.cgi: boa libbwl.a $(MAIN_OBJS) $(EXTRA_OBJS)
	@echo [Building ... $@]
	$(Q_)$(CC) -o ${NEXUS_BIN_DIR}/$@ $(MAIN_OBJS) $(EXTRA_OBJS) $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.html     ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.js       ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf_flame.js ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.png      ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.css      ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/chart*.js         ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/boundingbox.js    ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/svgvml3d.js       ${NEXUS_BIN_DIR}
	$(Q_)rm -f ${NEXUS_BIN_DIR}/bsysperf.d

bsysperf_server: $(SERVER_OBJS)
	@echo [Building ... $@]
	$(Q_)$(CC) -c $(SERVER_OBJS) $(CFLAGS) ${CDEP_FLAG}
	$(Q_)$(CC) -o ${NEXUS_BIN_DIR}/$@ $(SERVER_OBJS) $(EXTRA_OBJS) $(CFLAGS) ${CDEP_FLAG} $(LDFLAGS)
	$(Q_)$(CP) -f ${NEXUS_TOP}/../${B_THIS_DIR}/bsysperf.sh   ${NEXUS_BIN_DIR}
	$(Q_)rm -f ${NEXUS_BIN_DIR}/bsysperf_server.d

-include *.d

install: all
	@echo [Installing to ... ${NEXUS_BIN_DIR}]
	$(Q_)$(CP) -f ${APPS} ${NEXUS_BIN_DIR}

endif # B_REFSW_REAL_MAKE
