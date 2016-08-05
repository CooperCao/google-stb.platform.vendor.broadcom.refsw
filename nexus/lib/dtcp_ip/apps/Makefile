############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
# Module Description: Makefile for DTCP IP example apps
#
############################################################

ifdef COMSPEC
# Any DOS environment
NEXUS_TOP := $(shell cd ../../../../nexus && cd)
else
NEXUS_TOP := $(shell cd ../../../../nexus ; pwd)
endif

# If NEXUS_PLATFORM not defined, populate it from PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
    $(error NEXUS_PLATFORM is not defined)
endif

SSL_CERT_FILE := host.cert

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

B_PSIP_TOP := $(NEXUS_TOP)/../rockford/lib/psip
B_MPOD_TOP := $(NEXUS_TOP)/../rockford/lib/ocap/mpod
B_MPOD_DRIVERDIR = $(B_MPOD_TOP)/src/driver/build/bcm$(NEXUS_PLATFORM)$(shell awk 'BEGIN{print tolower("$(BCHP_VER)")}')/linux
BSEAV := $(NEXUS_TOP)/../BSEAV

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

ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mipsel-linux
export B_REFSW_ARCH
endif

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
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
DTCP_IP_APPLIB_TEST_LDFLAGS += $(PLAYBACK_IP_LDFLAGS)

include $(NEXUS_TOP)/lib/os/b_os_lib.inc
include $(B_PSIP_TOP)/b_psip_lib.inc

ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
ESTB_CFG_SUPPORT=y
export ESTB_CFG_SUPPORT
include $(B_MPOD_TOP)/b_mpod_lib.inc
include $(B_MPOD_TOP)/../estb_cfg/b_estb_cfg_lib.inc
endif #STREAMER_CABLECARD_SUPPORT

DTCP_IP_APPLIB_TEST_SRC_PATH = .
DTCP_IP_APPLIB_TEST_INCLUDE_PATH = .
DTCP_IP_APPLIB_TEST_ODIR = $(NEXUS_PLATFORM).$(B_REFSW_ARCH)

ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
CFLAGS += -DSTREAMER_CABLECARD_SUPPORT -DDSG_SUPPORT -DBCM_DSG_DUAL_PROCESSOR_INTERFACE -DMPOD_SUPPORT -DINTEGRATE_BCM_DSG_HOST -DESTB_CFG_SUPPORT
ifeq ($(MPODXSIX),y)
CFLAGS += -DMPODXSIX
endif #MPODXSIX
DTCP_IP_APPLIB_TEST_LDFLAGS += -lb_mpod -lb_estb_cfg -L$(B_MPOD_TOP)/lib/cp/build -lcp -L$(B_MPOD_TOP)/lib/cp/crypto/linux -lcrypto.0.9.7b_uclibc $(BSEAV)/docsis/StbHostApps/DsgCC/build/dsgcclib.a -lrt
endif #STREAMER_CABLECARD_SUPPORT

DTCP_IP_APPLIB_TEST_INCLUDE_PATH += \
	$(NEXUS_APP_INCLUDE_PATHS) \
	$(B_PSIP_LIB_PUBLIC_INCLUDES) \
	$(NEXUS_TOP)/lib/os/include/ \
	$(NEXUS_TOP)/lib/os/include/linuxuser \
	$(NEXUS_TOP)/lib/playback_ip/src/ \
    $(BSEAV)/lib/rtp \
    $(BSEAV)/lib/utils \
	$(BSEAV)/lib/mpeg2_ts_parse \
	$(BSEAV)/lib/tspsi \
	$(BSEAV)/api/include \
	$(BSEAV)/lib/netaccel/include \
	$(BSEAV)/lib/media \
	$(BSEAV)/lib/bfile \
	$(NEXUS_TOP)/lib/dtcp_ip/include \

ifeq ($(STREAMER_CABLECARD_SUPPORT),y)
DTCP_IP_APPLIB_TEST_INCLUDE_PATH += \
	$(B_MPOD_TOP)/include \
	$(B_MPOD_TOP)/src/driver \
	$(B_MPOD_TOP)/src/app_layer \
	$(B_MPOD_TOP)/src/session_layer \
	$(B_MPOD_TOP)/src/link_layer \
	$(B_MPOD_TOP)/src/util

DTCP_IP_APPLIB_TEST_INCLUDE_PATH += \
	$(B_MPOD_TOP)/../estb_cfg/include
endif #STREAMER_CABLECARD_SUPPORT

DTCP_IP_APPLIB_TEST_DEFINES := $(NEXUS_APP_DEFINES)
DTCP_IP_APPLIB_TEST_DEFINES += LINUX BCM_DEBUG
DTCP_IP_APPLIB_TEST_DEFINES += MAJOR_VERSION=0 MINOR_VERSION=0 SUB_VERSION=1

DTCP_IP_CLIENT_APP = http_client
DTCP_IP_CLIENT_SRCS = \
	http_client.c

DTCP_IP_SERVER_APP = http_server
DTCP_IP_SERVER_SRCS = \
	http_server.c

vpath %.cpp $(DTCP_IP_APPLIB_TEST_SRC_PATH)
vpath %.c $(DTCP_IP_APPLIB_TEST_SRC_PATH)

.PHONY: api clean install all

all: $(DTCP_IP_CLIENT_APP) $(DTCP_IP_SERVER_APP)

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
ifeq ($(NXCLIENT_SUPPORT),y)
# Library: nxclient
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += ${NXCLIENT_CFLAGS}
LDFLAGS += ${NXCLIENT_LDFLAGS}
endif
endif
endif
endif

LDFLAGS += -ltshdrbuilder

# Suppress LiveMedia Output for non VERBOSE builds
ifeq ($(VERBOSE),)
REDIR_LIVEMEDIA := &> /dev/null
else
REDIR_LIVEMEDIA :=
endif

# This builds the nexus api and other library dependencies
ifeq ($(SSL_SUPPORT),y)
api: openssl_lib
else
api:
endif

ifneq ($(REFSW_BUILD),y)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/build
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/os
endif #REFSW_BUILD
ifeq ($(LIVEMEDIA_SUPPORT),y)
	$(Q_)$(MAKE) -C $(LIVEMEDIA_ORG) $(REDIR_LIVEMEDIA)
	$(Q_)$(MAKE) -C $(BLIVE_EXT_DIR)/build
endif #LIVEMEDIA_SUPPORT

ifeq ($(DTCP_IP_SUPPORT),y)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/dtcp_ip install
endif #DTCP_IP_SUPPORT

	$(Q_)$(MAKE) -C $(BSEAV)/lib/tshdrbuilder
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/playback_ip
	$(Q_)$(MAKE) -C $(B_PSIP_TOP)

ifeq ($(SETTOP_API_SUPPORT),y)
	$(Q_)$(MAKE) -C $(BSEAV)/api/build/nexus
endif #SETTOP_API_SUPPORT

ifneq ($(wildcard $(BSEAV)/lib/netaccel/module/Makefile),)
ifeq ($(NETACCEL_SUPPORT),y)
	$(Q_)$(MAKE) NETACCEL_BUILD_PRIVATE=y IDLE_STATS=y NETACCEL_STANDALONE=y -C $(BSEAV)/lib/netaccel
endif #NETACCEL_SUPPORT
endif


ifeq ($(SSL_SUPPORT),y)
.PHONY:  openssl_lib
openssl_lib:
	@echo [Build..... OpenSSL]
	$(Q_)$(MAKE) -C $(B_OPENSSL_BUILD_DIR)
endif #SSL_SUPPORT

# This cleans nexus and local apps
clean:
ifneq ($(REFSW_BUILD),y)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/os clean
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/build clean
endif
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/playback_ip clean
	$(Q_)$(RM) $(BSEAV)/bin/$(DTCP_IP_CLIENT_APP) $(DTCP_IP_APPLIB_TEST_ODIR)/*.d $(DTCP_IP_APPLIB_TEST_ODIR)/*.o $(DTCP_IP_APPLIB_TEST_ODIR)/*.out;
	$(Q_)$(RM) $(DTCP_IP_APPLIB_TEST_ODIR)/$(DTCP_IP_CLIENT_APP)
	$(Q_)$(RM) $(BSEAV)/bin/$(DTCP_IP_SERVER_APP)
ifeq ($(LIVEMEDIA_SUPPORT),y)
	$(Q_)$(MAKE) -C $(LIVEMEDIA_ORG) clean $(REDIR_LIVEMEDIA)
	$(Q_)$(MAKE) -C $(BLIVE_EXT_DIR)/build clean
endif
ifeq ($(SETTOP_API_SUPPORT),y)
	$(Q_)$(MAKE) -C $(BSEAV)/api/build/nexus clean
endif
ifeq ($(SSL_SUPPORT),y)
	$(Q_)$(MAKE) -C $(B_OPENSSL_BUILD_DIR) clean
	@rm -f $(NEXUS_BIN_DIR)/libcrypto.so*
	@rm -f $(NEXUS_BIN_DIR)/libssl.so**
	@rm -f $(NEXUS_BIN_DIR)/$(SSL_CERT_FILE)
	@if [ -d $(BSEAV)/bin ]; then \
	    rm -f $(BSEAV)/bin/$(SSL_CERT_FILE) ;\
	fi
endif
ifeq ($(DTCP_IP_SUPPORT),y)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/dtcp_ip clean
endif #DTCP_IP_SUPPORT
	$(Q_)$(MAKE) -C $(BSEAV)/lib/tshdrbuilder clean

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(DTCP_IP_APPLIB_TEST_INCLUDE_PATH)) $(addprefix -D,$(DTCP_IP_APPLIB_TEST_DEFINES))

# Always build with debug
CFLAGS += -MD -g

C_ONLY_FLAGS := -std=c99 -Wstrict-prototypes
CPPFLAGS := -fno-rtti -fno-exceptions


# Implicit rule for building local app

$(DTCP_IP_CLIENT_APP): %: %.c $(DTCP_IP_APPLIB_TEST_ODIR) $(DTCP_IP_CLIENT_SRCS) api
	@echo [Compile... $<]
	$(Q_)$(CC) -o $(DTCP_IP_APPLIB_TEST_ODIR)/$@ $(DTCP_IP_CLIENT_SRCS) $(DTCP_IP_APPLIB_TEST_LDFLAGS) $(CFLAGS) $(C_ONLY_FLAGS) $(PLAYBACK_IP_CFLAGS) $(LDFLAGS)
	$(Q_)cp -f $(DTCP_IP_APPLIB_TEST_ODIR)/$(DTCP_IP_CLIENT_APP) $(NEXUS_BIN_DIR)
	@if [ -d $(BSEAV)/bin ]; then \
		cp -f $(DTCP_IP_APPLIB_TEST_ODIR)/$(DTCP_IP_CLIENT_APP) $(BSEAV)/bin/ ; \
	fi

$(DTCP_IP_SERVER_APP): %: %.c $(DTCP_IP_APPLIB_TEST_ODIR) $(DTCP_IP_SERVER_SRCS) api
	@echo [Compile... $<]
	$(Q_)$(CC) -o $(DTCP_IP_APPLIB_TEST_ODIR)/$@ $(DTCP_IP_SERVER_SRCS) $(DTCP_IP_APPLIB_TEST_LDFLAGS) $(CFLAGS) $(C_ONLY_FLAGS) $(PLAYBACK_IP_CFLAGS) $(LDFLAGS)
	$(Q_)cp -f $(DTCP_IP_APPLIB_TEST_ODIR)/$(DTCP_IP_SERVER_APP) $(NEXUS_BIN_DIR)
	@if [ -d $(BSEAV)/bin ]; then \
		cp -f $(DTCP_IP_APPLIB_TEST_ODIR)/$(DTCP_IP_SERVER_APP) $(BSEAV)/bin/ ; \
	fi

$(DTCP_IP_APPLIB_TEST_ODIR) :
	-@if [ ! -d $(DTCP_IP_APPLIB_TEST_ODIR) ]; then mkdir $(DTCP_IP_APPLIB_TEST_ODIR); fi


install: all
ifneq ($(REFSW_BUILD),y)
ifeq ($(NETACCEL_SUPPORT),y)
	$(Q_)$(MAKE) -C $(BSEAV)/lib/netaccel  INSTALL_DIR=$(NEXUS_BIN_DIR) install
endif
endif

ifeq ($(DTCP_IP_SUPPORT),y)
	@cp -f $(BSEAV)/lib/security/bcrypt/lib/libbcrypt.so $(NEXUS_BIN_DIR)
	@cp -f $(BSEAV)/lib/security/common_drm/lib/$(CMNDRM_DIR)/libcmndrm.so $(NEXUS_BIN_DIR)
endif #DTCP_IP_SUPPORT


ifeq ($(SSL_SUPPORT),y)
ifneq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
	@cp -f -P $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.so* $(NEXUS_BIN_DIR)
	@cp -f -P $(B_OPENSSL_LIB_INSTALL_DIR)/libssl.so* $(NEXUS_BIN_DIR)
	@cp -f $(NEXUS_TOP)/lib/playback_ip/apps/$(SSL_CERT_FILE) $(NEXUS_BIN_DIR)
	@if [ -d $(BSEAV)/bin ]; then \
	    cp -f $(NEXUS_TOP)/lib/playback_ip/apps/$(SSL_CERT_FILE) $(BSEAV)/bin ;\
	fi
endif   #  ifeq ($(B_REFSW_OPENSSL_IS_EXTERNAL),y)
endif
	@if [ -d $(BSEAV)/bin ]; then \
	    cp -f $(NEXUS_BIN_DIR)/*.so* $(BSEAV)/bin; \
	    cp -f $(NEXUS_BIN_DIR)/*.a $(BSEAV)/bin; \
	    cp -f $(NEXUS_BIN_DIR)/ip_* $(BSEAV)/bin; \
	fi

ifneq ($(INSTALL_DIR),$(NEXUS_BIN_DIR))
ifneq ($(INSTALL_DIR),)
	@if [ -d $(INSTALL_DIR) ]; then \
	    cp -f $(NEXUS_BIN_DIR)/*.so* $(INSTALL_DIR); \
            cp -f $(NEXUS_BIN_DIR)/*.a  $(INSTALL_DIR); \
            cp -f $(NEXUS_BIN_DIR)/ip_* $(INSTALL_DIR); \
        fi
endif
endif


-include $(DTCP_IP_APPLIB_TEST_ODIR)/*.d
