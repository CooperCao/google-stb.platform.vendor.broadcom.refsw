############################################################
#     (c)2008-2015 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
############################################################

SHELL := /bin/bash

B_THIS_DIR:=nexus/lib/playback_ip/apps

ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

# Build the name of this makefile, relative to B_REFSW_TOP (used for recipe tracing)
# Keep this line before any includes!
B_THIS_MAKEFILE_NAME := $(subst $(abspath ${NEXUS_TOP}/..),,$(abspath $(lastword $(MAKEFILE_LIST))))
# Uncomment the following two lines to enable recipe tracing.
# B_TRACE_RECIPE_BEGIN = @echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>  ${B_THIS_MAKEFILE_NAME}: Begin recipe: $@"
# B_TRACE_RECIPE_END   = @echo "<<<<<<<<<<<<<<<<<<<<<<<<<<<<  ${B_THIS_MAKEFILE_NAME}:   End recipe: $@"

# If NEXUS_PLATFORM not defined, populate it from PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
    $(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(NEXUS_PREBUILT_BINARY),y)
    # do "make api" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
    NEXUS_BIN_DIR ?= $(NEXUS_TOP)/../obj.${NEXUS_PLATFORM}/nexus/bin
    include $(NEXUS_BIN_DIR)/include/platform_app.inc
    Q_ ?= @
else
    include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
IP_APPLIB_TEST_ODIR := ${B_REFSW_OBJ_ROOT}/nexus/lib/playback_ip/apps
BSEAV := $(NEXUS_TOP)/../BSEAV
BSEAV_BIN_DIR = ${B_REFSW_OBJ_ROOT}/BSEAV/bin

B_PSIP_TOP := $(NEXUS_TOP)/../rockford/lib/psip
B_MPOD_TOP := $(NEXUS_TOP)/../rockford/lib/ocap/mpod
B_MPOD_DRIVERDIR = $(B_MPOD_TOP)/src/driver/build/bcm$(NEXUS_PLATFORM)$(shell awk 'BEGIN{print tolower("$(BCHP_VER)")}')/linux

SSL_CERT_FILE := host.cert
HLS_CH_FILE := ch.htm
IPSTREAMER_CONFIG_FILE := ipStreamerConfig.xml

ifeq ($(findstring $(NEXUS_PLATFORM),93380sms,93383sms), $(NEXUS_PLATFORM))
    CFLAGS += -DB_HAS_SMS_GATEWAY
    export SMS_GATEWAY=y
    ifneq ($(EROUTER_SUPPORT),n)
        export EROUTER_SUPPORT=y
    CFLAGS += -DEROUTER_SUPPORT
    endif

    # When compiling for SMS platforms, .inc files of Nexus Playback, Audio, Video modules are not included.
    # Thus, we explictly include their paths for header files.
    CFLAGS += -DNEXUS_HAS_PLAYBACK -I$(NEXUS_TOP)/modules/playback/include -I$(NEXUS_TOP)/modules/video_decoder/${BCHP_CHIP}/include -I$(NEXUS_TOP)/modules/audio/${BCHP_CHIP}/include
endif

# enable IP Streamer in demo mode (default setting): loops around when EOF is reached during trickplays
ifneq ($(IP_STREAMER_DEMO_MODE),n)
    IP_STREAMER_DEMO_MODE=y
    export IP_STREAMER_DEMO_MODE
    CFLAGS += -DIP_STREAMER_DEMO_MODE
endif

ifeq ($(HLS_DISABLE_RAMPUP),y)
    CFLAGS += -DHLS_DISABLE_RAMPUP
endif

ifeq ($(DISPLAY_LOCAL_FOR_ENCODE),y)
    CFLAGS += -DB_HAS_DISPLAY_LOCAL_FOR_ENCODE
endif

ifeq ($(B_REFSW_ARCH),)
    export B_REFSW_ARCH=mipsel-linux
endif

ifneq ($(B_REFSW_DEBUG),n)
    export B_REFSW_DEBUG=y
endif

ifeq ($(B_REFSW_DEBUG),y)
    DEBUG_SUFFIX=debug
else
    DEBUG_SUFFIX=release
endif

ifeq ($(USE_HTTP_KEEPALIVE),y)
    export USE_HTTP_KEEPALIVE
    CFLAGS += -DB_USE_HTTP_KEEPALIVE
endif

ifeq ($(USE_HTTP_CHUNK_ENCODING),y)
    export USE_HTTP_CHUNK_ENCODING
    CFLAGS += -DB_USE_HTTP_CHUNK_ENCODING
endif

# Get OpenSSl make defines
include $(BSEAV)/lib/openssl/openssl.inc

ifeq ($(RAD_EA_SUPPORT), y)
    CFLAGS += -DB_HAS_RAD_EA
endif #RAD_EA_SUPPORT

include $(BSEAV)/lib/tshdrbuilder/tshdrbuilder.inc
CFLAGS += $(addprefix -I,$(TSHDRBUILDER_INCLUDES))

#include applib definitions
include $(NEXUS_TOP)/lib/playback_ip/b_playback_ip_lib.inc
IP_APPLIB_TEST_LDFLAGS += $(PLAYBACK_IP_LDFLAGS)

include $(NEXUS_TOP)/lib/os/b_os_lib.inc
include $(B_PSIP_TOP)/b_psip_lib.inc

ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
    ESTB_CFG_SUPPORT=y
    export ESTB_CFG_SUPPORT
    include $(B_MPOD_TOP)/b_mpod_lib.inc
    include $(B_MPOD_TOP)/../estb_cfg/b_estb_cfg_lib.inc
endif #STREAMER_CABLECARD_SUPPORT

IP_APPLIB_TEST_SRC_PATH = .
IP_APPLIB_TEST_INCLUDE_PATH = .

ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
    CFLAGS += -DSTREAMER_CABLECARD_SUPPORT -DDSG_SUPPORT -DBCM_DSG_DUAL_PROCESSOR_INTERFACE -DMPOD_SUPPORT -DINTEGRATE_BCM_DSG_HOST -DESTB_CFG_SUPPORT
    ifeq ($(MPODXSIX),y)
        CFLAGS += -DMPODXSIX
    endif #MPODXSIX
    IP_APPLIB_TEST_LDFLAGS += -lb_mpod -lb_estb_cfg -L$(B_MPOD_TOP)/lib/cp/build -lcp -L$(B_MPOD_TOP)/lib/cp/crypto/linux -lcrypto.0.9.7b_uclibc $(BSEAV)/docsis/StbHostApps/DsgCC/build/dsgcclib.a -lrt
endif #STREAMER_CABLECARD_SUPPORT


IP_APPLIB_TEST_INCLUDE_PATH += \
    $(NEXUS_APP_INCLUDE_PATHS) \
    $(B_PSIP_LIB_PUBLIC_INCLUDES) \
    $(NEXUS_TOP)/lib/playback_ip/src/ \
    $(BSEAV)/lib/rtp \
    $(BSEAV)/lib/utils \
    $(BSEAV)/lib/mpeg2_ts_parse \
    $(BSEAV)/lib/tspsi \
    $(BSEAV)/api/include \
    $(BSEAV)/lib/netaccel/include \
    $(BSEAV)/lib/media \
    $(BSEAV)/lib/bfile \

ifeq ($(DTCP_IP_SUPPORT),y)
    ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif

    IP_APPLIB_TEST_INCLUDE_PATH += $(NEXUS_TOP)/lib/dtcp_ip/include \

endif

ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
    ifeq ($(DTCP_IP_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif

    IP_APPLIB_TEST_INCLUDE_PATH += $(NEXUS_TOP)/lib/dtcp_ip_sage/include \

endif

ifneq ($(DTCP_IP_SAGE_SUPPORT),y)
    ifneq ($(DTCP_IP_SUPPORT),y)
    IP_APPLIB_TEST_INCLUDE_PATH += $(NEXUS_TOP)/lib/dtcp_ip/include \

    endif
endif

ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
    IP_APPLIB_TEST_INCLUDE_PATH += \
        $(B_MPOD_TOP)/include \
        $(B_MPOD_TOP)/src/driver \
        $(B_MPOD_TOP)/src/app_layer \
        $(B_MPOD_TOP)/src/session_layer \
        $(B_MPOD_TOP)/src/link_layer \
        $(B_MPOD_TOP)/src/util

    IP_APPLIB_TEST_INCLUDE_PATH += \
        $(B_MPOD_TOP)/../estb_cfg/include
endif #STREAMER_CABLECARD_SUPPORT

IP_APPLIB_TEST_DEFINES := $(NEXUS_APP_DEFINES)
IP_APPLIB_TEST_DEFINES += LINUX BCM_DEBUG
IP_APPLIB_TEST_DEFINES += MAJOR_VERSION=0 MINOR_VERSION=0 SUB_VERSION=1

IP_CLIENT_APP = ip_client
IP_CLIENT_SRCS = \
    ip_client.c \
    ip_psi.c

IP_CLIENT_FCC_APP = ip_client_fcc
IP_CLIENT_FCC_SRCS = \
    ip_client_fcc.c

IP_STREAMER_APP = ip_streamer
IP_STREAMER_SRCS = \
    ip_http.c \
    ip_streamer_lib.c \
    ip_streamer_file_src.c \
    ip_psi.c \
    ip_streamer.c

IP_STREAMER_SRCS += \
    ip_streamer_qam_src.c \
    ip_streamer_streamer_src.c \
    ip_streamer_sat_src.c \
    ip_streamer_ofdm_src.c \
    ip_streamer_vsb_src.c \
    ip_streamer_ip_src.c \
    ip_streamer_ip_dst.c \
    ip_streamer_diags.c \
    ip_streamer_rec_dst.c

ifeq ($(VIDEO_ENCODER_SUPPORT),y)
    IP_STREAMER_SRCS += \
        ip_streamer_hdmi_src.c \
        ip_streamer_transcode.c
endif #VIDEO_ENCODER_SUPPORT

ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
    IP_STREAMER_SRCS += \
        ip_strm_cablecard.c
endif #STREAMER_CABLECARD_SUPPORT

vpath %.cpp $(IP_APPLIB_TEST_SRC_PATH)
vpath %.c $(IP_APPLIB_TEST_SRC_PATH)

ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
    CP_ARCH = mipsel-linux
    CP_TOOLCHAIN_DIR:=$(shell dirname $(shell dirname $(shell which ${CP_ARCH}-gcc 2>/dev/null) 2>/dev/null))
    CP_CROSS_COMPILE ?= ${CP_ARCH}-
    ECM_IF =
    ifeq ($(findstring $(NEXUS_PLATFORM), 97420), $(NEXUS_PLATFORM))
        ECM_IF = eth2
    endif
    CDL_CMDS =
    DSG_SUPPORT = y
endif #STREAMER_CABLECARD_SUPPORT


# For linux builds, link to the correct libraries
ifeq ($(B_REFSW_OS),linuxkernel)
    LDFLAGS := ${NEXUS_LDFLAGS}
else
    ifneq ($(findstring linux,$(B_REFSW_OS)),)
        LDFLAGS := -lnexus -L${NEXUS_BIN_DIR} -lm  $(EXTRA_DRM_OBJS)
        ifeq ($(NEXUS_ANDROID_SUPPORT),y)
            include $(NEXUS_TOP)/build/android.inc
            LDFLAGS := ${ANDROID_LDFLAGS} $(EXTRA_DRM_OBJS)
        else
            LDFLAGS += -lpthread
        endif
    endif
endif

LDFLAGS += -ltshdrbuilder

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(IP_APPLIB_TEST_INCLUDE_PATH)) $(addprefix -D,$(IP_APPLIB_TEST_DEFINES))

# Always build with debug
CFLAGS += -MD -g

C_ONLY_FLAGS := -std=c99 -Wstrict-prototypes

ifeq ($(findstring $(NEXUS_PLATFORM),93380sms,93383sms), $(NEXUS_PLATFORM))
    APPS := $(IP_STREAMER_APP)
else
    APPS := $(IP_CLIENT_APP) $(IP_STREAMER_APP) $(IP_CLIENT_FCC_APP)
endif

NEXUS_APPS :=$(foreach   this_app,$(APPS),$(NEXUS_BIN_DIR)/$(this_app))
ODIR_APPS  :=$(foreach   this_app,$(APPS),$(IP_APPLIB_TEST_ODIR)/$(this_app))

# Build a list of sub-libraries that need to get built (depending on various build flags).
# Each sub-library name matches the build target (below) for that library.  Also, prepending
# "clean_" to the sub-library name will match the name for the clean target for each library.
SUBLIBS:=

ifeq ($(SSL_SUPPORT),y)
    # Don't bother with the OpenSSL make if they want to use a customer-provided version.
    ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
        SUBLIBS += openssl
    endif   #  ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
endif #SSL_SUPPORT

ifneq ($(REFSW_BUILD),y)
    # B_NEXUS_API_BUILD_COMPLETED=y means that some higher level makefile has already built nexus, so we don't need to.
    ifneq ($(B_NEXUS_API_BUILD_COMPLETED),y)
        SUBLIBS += nexus
    endif
    SUBLIBS += b_os
endif #REFSW_BUILD

ifneq ($(LIVEMEDIA_SUPPORT),n)
    SUBLIBS += livemedia
    SUBLIBS += blive_ext
endif #LIVEMEDIA_SUPPORT

ifeq ($(DTCP_IP_SUPPORT),y)
    ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif

    ifeq ($(DTCP_IP_USE_EXT_VENDOR_LIB), y)
        SUBLIBS += dtcp_ip_ext
    endif
    SUBLIBS += dtcp_ip
endif #DTCP_IP_SUPPORT
ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
    ifeq ($(DTCP_IP_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif
    SUBLIBS += dtcp_ip
endif #DTCP_IP_SAGE_SUPPORT


SUBLIBS += tshdrbuilder
SUBLIBS += playback_ip
SUBLIBS += psip

ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
    SUBLIBS += cablecard
endif #STREAMER_CABLECARD_SUPPORT

ifeq ($(SETTOP_API_SUPPORT),y)
# If we need to work with SETTOP_API, then this needs to be reworked because SETTOP_API's
# Makefile will build all the same sub-libraries that we are building (at the same time!)
# and things clobber each other.  But everyone keeps saying that SETTOP_API is dead...
#   SUBLIBS += settop_api
endif #SETTOP_API_SUPPORT

ifneq ($(wildcard $(BSEAV)/lib/netaccel/module/Makefile),)
    ifeq ($(NETACCEL_SUPPORT),y)
        SUBLIBS += netaccel
    endif #NETACCEL_SUPPORT
endif

# Now, from the list of sub-libraries build targets, build an equivalent list
# of "clean" targets.
CLEAN_SUBLIBS  := $(foreach   this_sublib,$(SUBLIBS),clean_$(this_sublib))

# Remove a couple of the open source sublibs from the list of libs to clean.
CLEAN_SUBLIBS  := $(filter-out clean_openssl,$(CLEAN_SUBLIBS))
CLEAN_SUBLIBS  := $(filter-out clean_livemedia,$(CLEAN_SUBLIBS))

.PHONY: all
all: $(APPS)

.PHONY: $(APPS)
$(APPS): % : $(NEXUS_BIN_DIR)/%


# NEXUS_APPS come from APPS like this:
# If the playback_ip apps are missing from NEXUS_BIN_DIR, then copy them from IP_APPLIB_TEST_ODIR.
# Pattern rule to copy playback_ip apps to NEXUS_BIN_DIR.
$(NEXUS_BIN_DIR)/% : $(IP_APPLIB_TEST_ODIR)/%
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... $(notdir $<)]"
	$(Q_)cp -dfp $< $@
	$(B_TRACE_RECIPE_END)


$(ODIR_APPS): | playback_ip_api

.PHONY: playback_ip_api
playback_ip_api: $(SUBLIBS)

# This cleans nexus and local apps
.PHONY: clean
clean: clean_playback_ip_api
	$(B_TRACE_RECIPE_BEGIN)
	$(Q_)$(RM) $(IP_APPLIB_TEST_ODIR)/*.d   $(IP_APPLIB_TEST_ODIR)/*.o   $(IP_APPLIB_TEST_ODIR)/*.out;
	$(Q_)$(RM) $(IP_APPLIB_TEST_ODIR)/$(IP_CLIENT_APP)  $(IP_APPLIB_TEST_ODIR)/$(IP_CLIENT_FCC_APP)  $(IP_APPLIB_TEST_ODIR)/$(IP_STREAMER_APP)
	$(Q_)$(RM) $(NEXUS_BIN_DIR)/$(IP_CLIENT_APP)        $(NEXUS_BIN_DIR)/$(IP_CLIENT_FCC_APP)        $(NEXUS_BIN_DIR)/$(IP_STREAMER_APP)
	$(Q_)$(RM) $(BSEAV)/bin/$(IP_CLIENT_APP)            $(BSEAV)/bin/$(IP_CLIENT_FCC_APP)            $(BSEAV)/bin/$(IP_STREAMER_APP)
    ifneq ($(INSTALL_DIR),$(NEXUS_BIN_DIR))
        ifneq ($(INSTALL_DIR),)
			@if [ -d $(INSTALL_DIR) ]; then   \
				$(RM) $(INSTALL_DIR)/$(IP_CLIENT_APP)  $(INSTALL_DIR)/$(IP_CLIENT_FCC_APP)  $(INSTALL_DIR)/$(IP_STREAMER_APP) ; \
			fi
        endif
    endif
	$(B_TRACE_RECIPE_END)


.PHONY: clean_playback_ip_api
clean_playback_ip_api: $(CLEAN_SUBLIBS)


# This installs nexus and local apps from NEXUS_BIN_DIR to other places.
.PHONY: install
install: all
	$(B_TRACE_RECIPE_BEGIN)
	@if [ -d ${BSEAV_BIN_DIR} ]; then \
		cp -dfpu $(NEXUS_BIN_DIR)/*.so* ${BSEAV_BIN_DIR}; \
		cp -dfpu $(NEXUS_BIN_DIR)/*.a ${BSEAV_BIN_DIR}; \
		cp -dfpu $(NEXUS_BIN_DIR)/ip_* ${BSEAV_BIN_DIR}; \
		if [ -e $(NEXUS_BIN_DIR)/$(SSL_CERT_FILE) ]; then \
			cp -dfpu $(NEXUS_BIN_DIR)/$(SSL_CERT_FILE) $(BSEAV_BIN_DIR); \
		fi \
	fi

    ifneq ($(INSTALL_DIR),$(NEXUS_BIN_DIR))
        ifneq ($(INSTALL_DIR),)
			@if [ -d $(INSTALL_DIR) ]; then \
				cp -dfpu $(NEXUS_BIN_DIR)/*.so* $(INSTALL_DIR); \
				cp -dfpu $(NEXUS_BIN_DIR)/*.a  $(INSTALL_DIR); \
				cp -dfpu $(NEXUS_BIN_DIR)/ip_* $(INSTALL_DIR); \
				if [ -e $(NEXUS_BIN_DIR)/$(SSL_CERT_FILE) ]; then \
					cp -dfpu $(NEXUS_BIN_DIR)/$(SSL_CERT_FILE) $(INSTALL_DIR); \
				fi \
			fi
        endif
    endif
	$(B_TRACE_RECIPE_END)


# Implicit rule for building local app

$(IP_APPLIB_TEST_ODIR)/$(IP_CLIENT_APP): $(IP_APPLIB_TEST_ODIR)/%: %.c $(IP_CLIENT_SRCS) | $(IP_APPLIB_TEST_ODIR)
	$(B_TRACE_RECIPE_BEGIN)
	-$(Q_)$(MKDIR) -p $(IP_APPLIB_TEST_ODIR) >/dev/null 2>&1
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(IP_CLIENT_SRCS) $(IP_APPLIB_TEST_LDFLAGS) $(CFLAGS) $(C_ONLY_FLAGS) $(PLAYBACK_IP_CFLAGS) $(LDFLAGS)
	$(Q_)cp -dfp $(IP_APPLIB_TEST_ODIR)/$(IP_CLIENT_APP) $(NEXUS_BIN_DIR)
	@if [ -d ${BSEAV_BIN_DIR} ]; then \
		cp -dfp $(IP_APPLIB_TEST_ODIR)/$(IP_CLIENT_APP) ${BSEAV_BIN_DIR} ; \
	fi
	$(B_TRACE_RECIPE_END)


$(IP_APPLIB_TEST_ODIR)/$(IP_CLIENT_FCC_APP): $(IP_APPLIB_TEST_ODIR)/%: %.c $(IP_CLIENT_FCC_SRCS) | $(IP_APPLIB_TEST_ODIR)
	$(B_TRACE_RECIPE_BEGIN)
	-$(Q_)$(MKDIR) -p $(IP_APPLIB_TEST_ODIR) >/dev/null 2>&1
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(IP_CLIENT_FCC_SRCS) $(IP_APPLIB_TEST_LDFLAGS) $(CFLAGS) $(C_ONLY_FLAGS) $(PLAYBACK_IP_CFLAGS) $(LDFLAGS)
	$(Q_)cp -dfp $(IP_APPLIB_TEST_ODIR)/$(IP_CLIENT_FCC_APP) $(NEXUS_BIN_DIR)
	@if [ -d ${BSEAV_BIN_DIR} ]; then \
		cp -dfp $(IP_APPLIB_TEST_ODIR)/$(IP_CLIENT_FCC_APP) ${BSEAV_BIN_DIR} ; \
	fi
	$(B_TRACE_RECIPE_END)

$(IP_APPLIB_TEST_ODIR)/$(IP_STREAMER_APP): $(IP_APPLIB_TEST_ODIR)/%: %.c  $(IP_STREAMER_SRCS) | $(IP_APPLIB_TEST_ODIR)
	$(B_TRACE_RECIPE_BEGIN)
	-$(Q_)$(MKDIR) -p $(IP_APPLIB_TEST_ODIR) >/dev/null 2>&1
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(IP_STREAMER_SRCS) $(IP_APPLIB_TEST_LDFLAGS) $(CFLAGS) $(C_ONLY_FLAGS) $(PLAYBACK_IP_CFLAGS) $(LDFLAGS)
	$(Q_)cp -dfp $(IP_APPLIB_TEST_ODIR)/$(IP_STREAMER_APP) $(NEXUS_BIN_DIR)
	@if [ -d ${BSEAV_BIN_DIR} ]; then \
		cp -dfp $(IP_APPLIB_TEST_ODIR)/$(IP_STREAMER_APP) ${BSEAV_BIN_DIR} ; \
	fi
	$(B_TRACE_RECIPE_END)

$(IP_APPLIB_TEST_ODIR) :
	$(B_TRACE_RECIPE_BEGIN)
	-@if [[ ! -d $(IP_APPLIB_TEST_ODIR) ]]; then mkdir -p $(IP_APPLIB_TEST_ODIR); fi
	$(B_TRACE_RECIPE_END)


###############################################################################
#  Define build/clean targets for each individual sub-library.
###############################################################################
#  Nexus Targets  ("nexus", "clean_nexus").
###############################################################################
.PHONY: nexus
nexus:
	$(B_TRACE_RECIPE_BEGIN)
    # Don't build nexus if we have pre-built binaries
    ifneq ($(NEXUS_PREBUILT_BINARY),y)
	    @echo "[Build... nexus]"
	    $(Q_)$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build install
    endif
	$(B_TRACE_RECIPE_END)

.PHONY: clean_nexus
clean_nexus:
	$(B_TRACE_RECIPE_BEGIN)
    # Don't build nexus if we have pre-built binaries
    ifneq ($(NEXUS_PREBUILT_BINARY),y)
	    @echo "[Clean... nexus]"
	    $(Q_)$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build clean
    endif #NEXUS_PREBUILT_BINARY
	$(B_TRACE_RECIPE_END)


###############################################################################
#  b_os Targets  ("b_os", "clean_b_os").
###############################################################################
.PHONY: b_os
b_os:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... b_os]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/os install
	$(B_TRACE_RECIPE_END)

.PHONY: clean_b_os
clean_b_os:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... b_os]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/os clean
	$(B_TRACE_RECIPE_END)


#########################################################################################
#  livemedia Targets  ("livemedia", "clean_livemedia").
#########################################################################################
# Suppress LiveMedia Output for non VERBOSE builds

.PHONY: livemedia
livemedia:
	$(B_TRACE_RECIPE_BEGIN)
    ifneq ($(LIVEMEDIA_SUPPORT),n)
		@echo "[Build... livemedia]"
		$(Q_)$(MAKE)        -C $(NEXUS_TOP)/../BSEAV/lib/livemedia
    endif #LIVEMEDIA_SUPPORT
	$(B_TRACE_RECIPE_END)

.PHONY: clean_livemedia
clean_livemedia:
	$(B_TRACE_RECIPE_BEGIN)
    ifneq ($(LIVEMEDIA_SUPPORT),n)
		@echo "[Clean... livemedia]"
		$(Q_)$(MAKE)        -C $(NEXUS_TOP)/../BSEAV/lib/livemedia  clean
    endif #LIVEMEDIA_SUPPORT
	$(B_TRACE_RECIPE_END)


#########################################################################################
#  blive_ext Targets  ("blive_ext", "clean_blive_ext").
#########################################################################################
.PHONY: blive_ext
blive_ext:          livemedia  # Can't build blive_ext until livemedia is build
	$(B_TRACE_RECIPE_BEGIN)
    ifneq ($(LIVEMEDIA_SUPPORT),n)
		@echo "[Build... blive_ext]"
		$(Q_)$(MAKE)        -C $(NEXUS_TOP)/../BSEAV/lib/livemedia/blive_ext/build
    endif #LIVEMEDIA_SUPPORT
	$(B_TRACE_RECIPE_END)

.PHONY: clean_blive_ext
clean_blive_ext:
	$(B_TRACE_RECIPE_BEGIN)
    ifneq ($(LIVEMEDIA_SUPPORT),n)
		@echo "[Clean... blive_ext]"
		$(Q_)$(MAKE)        -C $(NEXUS_TOP)/../BSEAV/lib/livemedia/blive_ext/build clean
    endif #LIVEMEDIA_SUPPORT
	$(B_TRACE_RECIPE_END)


#################################################################################################
#  tshdrbuilder Targets  ("tshdrbuilder", "clean_tshdrbuilder").
#################################################################################################
.PHONY: tshdrbuilder
tshdrbuilder:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... tshdrbuilder]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../BSEAV/lib/tshdrbuilder install
	$(B_TRACE_RECIPE_END)

.PHONY: clean_tshdrbuilder
clean_tshdrbuilder:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... tshdrbuilder]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../BSEAV/lib/tshdrbuilder clean
	$(B_TRACE_RECIPE_END)


###############################################################################
#  psip Targets  ("psip", "clean_psip").
###############################################################################
.PHONY: psip
psip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... psip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../rockford/lib/psip install
	$(B_TRACE_RECIPE_END)

.PHONY: clean_psip
clean_psip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... psip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../rockford/lib/psip clean
	$(B_TRACE_RECIPE_END)


###############################################################################
#  dtcp_ip Targets  ("dtcp_ip", "clean_dtcp_ip").
###############################################################################
.PHONY: dtcp_ip
ifeq ($(DTCP_IP_SUPPORT),y)
    ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
		$(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif
dtcp_ip: openssl
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... dtcp_ip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/dtcp_ip install
	@echo "[Install... dtcp_ip]"
	$(Q_)cp -df $(COMMON_DRM_BIN_DEPS_LIST) $(NEXUS_BIN_DIR)
	$(B_TRACE_RECIPE_END)
endif
ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
    ifeq ($(DTCP_IP_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif
dtcp_ip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... dtcp_ip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/dtcp_ip_sage install
	@echo "[Install... dtcp_ip]"
	$(Q_)cp -df $(COMMON_DRM_BIN_DEPS_LIST) $(NEXUS_BIN_DIR)
	$(B_TRACE_RECIPE_END)
endif

.PHONY: clean_dtcp_ip
ifeq ($(DTCP_IP_SUPPORT),y)
    ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif
clean_dtcp_ip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... dtcp_ip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/dtcp_ip clean
	$(B_TRACE_RECIPE_END)
endif
ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
    ifeq ($(DTCP_IP_SUPPORT),y)
        $(error DTCP_IP_SAGE_SUPPORT & DTCP_IP_SUPPORT cannot be enabled at the same time)
    endif
clean_dtcp_ip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... dtcp_ip]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/dtcp_ip_sage clean
	$(B_TRACE_RECIPE_END)
endif


###############################################################################
#  dtcp_ip_ext Targets  ("dtcp_ip_ext", "clean_dtcp_ip_ext").
###############################################################################
.PHONY: dtcp_ip_ext
dtcp_ip_ext: openssl dtcp_ip
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... dtcp_ip_ext]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/security/third_party/dtcp_ip install
	$(B_TRACE_RECIPE_END)

.PHONY: clean_dtcp_ip_ext
clean_dtcp_ip_ext:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... dtcp_ip_ext]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/security/third_party/dtcp_ip clean
	$(B_TRACE_RECIPE_END)

###############################################################################
#  cablecard Targets  ("cablecard", "clean_cablecard").
###############################################################################
.PHONY: cablecard
cablecard:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... cablecard]"
	@if [ -d $(BSEAV)/docsis/StbHostApps ];then echo ""; else cd $(BSEAV)/docsis;tar fxvz stb_src.tgz;fi
	$(Q_)$(MAKE) -C $(B_MPOD_TOP) DSG_SUPPORT=$(DSG_SUPPORT)
	$(Q_)$(MAKE) -C $(B_MPOD_TOP)/lib/cp/build -f makefile.cplib TOOLCHAIN=uclibc TOOLCHAIN_DIR=$(CP_TOOLCHAIN_DIR) CROSS_COMPILE=$(CP_CROSS_COMPILE)
	$(Q_)$(MAKE) -C $(B_MPOD_TOP)/../estb_cfg
	$(Q_)$(MAKE) -C $(B_MPOD_DRIVERDIR) ARCH=mips CROSS_COMPILE=$(CP_CROSS_COMPILE)
	$(Q_)$(RM) -f $(BSEAV)/docsis/StbHostApps/DsgCC/build/dsgcclib.a
	$(Q_)export CROSSDEV_ROOT=$(CP_TOOLCHAIN_DIR);\
	cd $(BSEAV)/docsis/StbHostApps/DsgCC/build; \
	./build dsgcc pvnet $(ECM_IF) applib cablecard  $(CDL_CMDS);\
	cd $(NEXUS_TOP)/lib/playback_ip/apps;
	$(B_TRACE_RECIPE_END)

.PHONY: clean_cablecard
clean_cablecard:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... cablecard]"
    ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
		$(Q_)$(MAKE) -C $(B_MPOD_TOP)/../estb_cfg clean
		$(Q_)$(MAKE) -C $(B_MPOD_TOP)/lib/cp/build -f makefile.cplib TOOLCHAIN=uclibc clean
		-$(RM) $(B_MPOD_TOP)/lib/cp/build/cplib.lib $(B_CP_LIB)/build/libcp.a
		$(Q_)$(MAKE) -C $(POD_DRIVERDIR) clean ARCH=mips CROSS_COMPILE=$(CP_CROSS_COMPILE)
		$(Q_)$(RM) -f $(BSEAV)/docsis/StbHostApps/DsgCC/build/dsgcclib.a
		$(Q_)export CROSSDEV_ROOT=$(CP_TOOLCHAIN_DIR);\
		cd $(BSEAV)/docsis/StbHostApps/DsgCC/build; \
		./build dsgcc clean;\
		cd $(NEXUS_TOP)/lib/playback_ip/apps;
    endif
	$(B_TRACE_RECIPE_END)

###############################################################################
#  settop_api Targets  ("settop_api", "clean_settop_api".
###############################################################################
.PHONY: settop_api
settop_api:  nexus
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... settop_api]"
	$(Q_)$(MAKE) -C $(BSEAV)/api/build/nexus
	$(B_TRACE_RECIPE_END)

.PHONY: clean_settop_api
clean_settop_api:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... settop_api]"
	$(Q_)$(MAKE) -C $(BSEAV)/api/build/nexus clean
	$(B_TRACE_RECIPE_END)

###############################################################################
#  netaccel Targets  ("netaccel", "clean_netaccel".
###############################################################################
.PHONY: netaccel
netaccel:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... netaccel]"
	$(Q_)$(MAKE) NETACCEL_BUILD_PRIVATE=y IDLE_STATS=y NETACCEL_STANDALONE=y -C $(BSEAV)/lib/netaccel install
	$(B_TRACE_RECIPE_END)

.PHONY: clean_netaccel
clean_netaccel:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... netaccel]"
	$(Q_)$(MAKE) NETACCEL_BUILD_PRIVATE=y IDLE_STATS=y NETACCEL_STANDALONE=y -C $(BSEAV)/lib/netaccel clean
	$(B_TRACE_RECIPE_END)


###############################################################################
#  openssl Targets  ("openssl", "clean_openssl").
#
#  We'll take some shortcuts here, because the OpenSSL makefile takes quite
#  a long time (several seconds) when everything is up to date.  So as long
#  as the OpenSSL libraries exist, we'll assume that they're up to date.
#
#  Of course, this means that if somebody is changing something (like endianess)
#  that requires that OpenSSL gets rebuild, then they will need to do a
#  "make openssl_veryclean" that will force it to be rebuilt.
###############################################################################
.PHONY: openssl
ifeq ($(filter openssl,$(SUBLIBS)),openssl)
    openssl: $(NEXUS_BIN_DIR)/libcrypto.so $(NEXUS_BIN_DIR)/libssl.so \
             $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.so $(B_OPENSSL_LIB_INSTALL_DIR)/libssl.so \
             $(NEXUS_BIN_DIR)/$(SSL_CERT_FILE)
else
    openssl:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Skipping... OpenSSL]"
	$(B_TRACE_RECIPE_END)
endif

# If the OpenSSL libraries are missing from NEXUS_BIN_DIR, then copy them from the OpenSSL install dir.
# Note the use of a "pattern rule" so that the recipe will only run once to generate both targets.
$(NEXUS_BIN_DIR)/%crypto.so  $(NEXUS_BIN_DIR)/%ssl.so:   $(B_OPENSSL_LIB_INSTALL_DIR)/%crypto.so  $(B_OPENSSL_LIB_INSTALL_DIR)/%ssl.so
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Install... OpenSSL]"
    ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
		${Q_}mkdir -p $(NEXUS_BIN_DIR)
		${Q_}$(CP) -dfp $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.so*   $(NEXUS_BIN_DIR)
		${Q_}$(CP) -dfp $(B_OPENSSL_LIB_INSTALL_DIR)/libssl.so*      $(NEXUS_BIN_DIR)
    endif   #  ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	$(B_TRACE_RECIPE_END)

# If the OpenSSL ".so" files are missing from the OpenSSL install dir, then we'll run the OpenSSL make.
# Again, use a "pattern rule" because running the recipe once will generate all (both) of the targets.
$(B_OPENSSL_LIB_INSTALL_DIR)/%crypto.so $(B_OPENSSL_LIB_INSTALL_DIR)/%ssl.so:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... OpenSSL]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/../BSEAV/lib/openssl
	$(B_TRACE_RECIPE_END)

$(NEXUS_BIN_DIR)/$(SSL_CERT_FILE):  $(NEXUS_TOP)/lib/playback_ip/apps/$(SSL_CERT_FILE)
	$(B_TRACE_RECIPE_BEGIN)
	${Q_}mkdir -p $(NEXUS_BIN_DIR)
	${Q_}cp -dfp $(NEXUS_TOP)/lib/playback_ip/apps/$(SSL_CERT_FILE) $(NEXUS_BIN_DIR)
	${Q_}if [ -d ${BSEAV_BIN_DIR} ]; then \
		cp -dfp $(NEXUS_TOP)/lib/playback_ip/apps/$(SSL_CERT_FILE) ${BSEAV_BIN_DIR} ;\
	fi
    ifneq ($(INSTALL_DIR),)
		${Q_}if [ -d $(INSTALL_DIR) ]; then \
			cp -dfp $(NEXUS_TOP)/lib/playback_ip/apps/$(SSL_CERT_FILE) ${INSTALL_DIR} ;\
		fi
    endif
	$(B_TRACE_RECIPE_END)

.PHONY: clean_openssl
clean_openssl:
	$(B_TRACE_RECIPE_BEGIN)
ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	@echo "[Clean... OpenSSL]"
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/openssl clean target-clean
	@rm -f $(NEXUS_BIN_DIR)/libcrypto.so*
	@rm -f $(NEXUS_BIN_DIR)/libssl.so**
	@rm -f $(NEXUS_BIN_DIR)/$(SSL_CERT_FILE)
	@if [ -d $(BSEAV)/bin ]; then \
		rm -f $(BSEAV)/bin/$(SSL_CERT_FILE) ;\
	fi
endif   #  ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	$(B_TRACE_RECIPE_END)

###########################################################################################
#  playback_ip Targets  ("playback_ip", "clean_playback_ip").
###########################################################################################

.PHONY: playback_ip
playback_ip:   $(filter-out playback_ip,$(SUBLIBS))
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Build... playback_ip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/playback_ip
	@cp -u $(NEXUS_TOP)/lib/playback_ip/apps/hlsSamplePlaylists/$(HLS_CH_FILE) $(NEXUS_BIN_DIR)
	@cp -u $(NEXUS_TOP)/lib/playback_ip/apps/$(IPSTREAMER_CONFIG_FILE) $(NEXUS_BIN_DIR)
	@if [ -d ${BSEAV_BIN_DIR} ]; then \
		cp -u $(NEXUS_TOP)/lib/playback_ip/apps/hlsSamplePlaylists/$(HLS_CH_FILE) ${BSEAV_BIN_DIR} ;\
		cp -u $(NEXUS_TOP)/lib/playback_ip/apps/$(IPSTREAMER_CONFIG_FILE) ${BSEAV_BIN_DIR} ;\
	fi
        ifneq ($(INSTALL_DIR),)
		@if [ -d $(INSTALL_DIR) ]; then \
		cp -u $(NEXUS_TOP)/lib/playback_ip/apps/hlsSamplePlaylists/$(HLS_CH_FILE) ${INSTALL_DIR} ;\
		cp -u $(NEXUS_TOP)/lib/playback_ip/apps/$(IPSTREAMER_CONFIG_FILE) ${INSTALL_DIR} ;\
		fi
        endif
	$(B_TRACE_RECIPE_END)

.PHONY: clean_playback_ip
clean_playback_ip:
	$(B_TRACE_RECIPE_BEGIN)
	@echo "[Clean... playback_ip]"
	$(Q_)$(MAKE)            -C $(NEXUS_TOP)/lib/playback_ip clean
	@rm -f $(NEXUS_BIN_DIR)/$(HLS_CH_FILE)
	@rm -f $(NEXUS_BIN_DIR)/$(IPSTREAMER_CONFIG_FILE)
	@if [ -d $(BSEAV)/bin ]; then \
		rm -f $(BSEAV)/bin/$(HLS_CH_FILE) ;\
		rm -f $(BSEAV)/bin/$(IPSTREAMER_CONFIG_FILE) ;\
	fi
	$(B_TRACE_RECIPE_END)

-include $(IP_APPLIB_TEST_ODIR)/*.d
