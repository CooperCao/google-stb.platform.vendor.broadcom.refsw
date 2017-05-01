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
############################################################

NEXUS_TOP := $(shell cd ../../../../nexus; pwd)
BSEAV_TOP := $(shell cd ../../../../BSEAV; pwd)

#APPS = encode_piff playback_piff playback_piff_PR_drm
#SRCS = encode_piff.c playback_piff.c playback_piff_PR_drm.c
APPS = encode_piff encode_playback_piff
SRCS = encode_piff.c encode_playback_piff.c
OBJS = ${SRCS:.c=.o}

ifeq ($(USE_TEST_CA_TA), y)
UTILS_SRCS = configure_keyslots.o
UTILS_OBJS = ${UTILS_SRCS:.c=.o}
USE_LOADABLE_TA ?= y
endif

USE_DIF ?= n
ifeq ($(USE_DIF), y)
DIF_APPS += encode_playback_dif
DIF_SRCS += encode_playback_dif.cpp
OBJS += ${DIF_SRCS:.cpp=.o}
endif

# Redirect logs to
LOG_REDIRECT_FILE ?= /dev/null

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
include $(BSEAV_TOP)/lib/security/common_drm/common_drm.inc
include $(BSEAV_TOP)/lib/security/bcrypt/bcrypt_libdir.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc

ifeq ($(APPS),)
$(error Must define APPS)
endif

ifneq ($(findstring 2.5, ${MSDRM_PRDY_SDK_VERSION}), 2.5)
$(error unsupported Playready VERSION = ${MSDRM_PRDY_SDK_VERSION})
endif

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
PIFF_INSTALL	:= lib/debug
else
PIFF_INSTALL	:= lib/retail
endif

include $(BSEAV_TOP)/lib/security/common_drm/lib/prdy_libdir.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc

ifeq ($(USE_DIF), y)
include $(BSEAV_TOP)/lib/security/dif/dif_libdir.inc
-include $(BSEAV_TOP)/opensource/protobuf/protobuf.inc
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= $(abspath ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR})
BPIFF_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/bpiff

END_OBJS := $(addprefix ${BPIFF_OBJ_ROOT}/,${OBJS})

ifeq ($(USE_TEST_CA_TA), y)
END_UTILS_OBJS := $(addprefix ${BPIFF_OBJ_ROOT}/,${UTILS_OBJS})
endif

ifeq ($(SAGE_VERSION), 2x)
PRDY_LIB_PATH := ${BSEAV_TOP}/thirdparty/playready/2.5/bin/${ARCH_DIR_PRDY}/lib/$(SAGE_VERSION)
else
PRDY_LIB_PATH := ${BSEAV_TOP}/thirdparty/playready/2.5/bin/${ARCH_DIR_PRDY}/lib
endif

APP_BINS = $(addprefix ${BPIFF_OBJ_ROOT}/,${APPS})
ifeq ($(USE_DIF), y)
DIF_APP_BINS = $(addprefix ${BPIFF_OBJ_ROOT}/,${DIF_APPS})
endif

.PHONY: api clean prep_drmrootfs

all: do_prep $(APP_BINS) $(DIF_APP_BINS) copy

LDFLAGS := ${COMMON_DRM_LDFLAGS}
LDFLAGS += -L$(NEXUS_BIN_DIR)/ -lpthread -lm -lnexus${NEXUS_LIB_SUFFIX}
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs
ifeq ($(SAGE_SUPPORT), y)
LDFLAGS += -L$(BSEAV_TOP)/lib/security/sage/srai/lib -lsrai
LDFLAGS += -L${PRDY_LIB_PATH} -lplayreadypk_host
else
LDFLAGS += -L$(BSEAV_TOP)/lib/security/common_drm/lib/${CMNDRM_DIR} -lcmndrm
LDFLAGS += -L${PRDY_LIB_PATH} -lplayreadypk
endif
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR) -lcmndrmprdy
LDFLAGS += $(B_OPENSSL_LDFLAGS)
LDFLAGS += -L$(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/bcrypt/$(BCRYPT_LIBDIR)/libbcrypt.so -lbcrypt
LDFLAGS += -L$(BSEAV_TOP)/lib/bpiff -lbpiff

ifeq ($(USE_DIF), y)
DIF_LDFLAGS += -L$(NEXUS_BIN_DIR) -ldif -lcurl
ifeq ($(SAGE_SUPPORT),y)
DIF_LDFLAGS += -lplayready30pk
endif
#ifeq ($(WIDEVINE_SUPPORT),y)
DIF_LDFLAGS += $(PROTOBUF_LDFLAGS) $(GTEST_LDFLAGS)
ifeq ($(SAGE_SUPPORT),y)
DIF_LDFLAGS += -lwvcdm -loemcrypto_tl
else
DIF_LDFLAGS += -lwvcdm -loemcrypto
endif
#endif
endif

#$(error XXX $(LDFLAGS))

# Always build with debug

# common crypto include path

CFLAGS += -I$(BSEAV_TOP)/lib/utils
CFLAGS += -I$(BSEAV_TOP)/lib/media
CFLAGS += -I${BSEAV_TOP}/lib/media/build
CFLAGS += -I${BSEAV_TOP}/lib/piff

ifeq ($(USE_DIF), y)
DIF_CFLAGS = -I${BSEAV_TOP}/lib/security/dif/include
endif

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS += -g

# bpiff encoder
CFLAGS += -I$(BSEAV_TOP)/lib/bpiff/include
CFLAGS += ${COMMON_DRM_CFLAGS}
CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/media -I$(NEXUS_TOP)/../BSEAV/lib/utils -I$(NEXUS_TOP)/../BSEAV/lib/asf

ifeq ($(SAGE_SUPPORT), y)
CFLAGS += -DSAGE_SUPPORT
CFLAGS += -I$(BSEAV_TOP)/lib/security/sage/srai/include
endif

ifeq ($(USE_TEST_CA_TA), y)
CFLAGS += -DUSE_TEST_CA_TA
ifeq ($(USE_LOADABLE_TA), y)
CFLAGS += -DUSE_LOADABLE_TA
endif
endif

CXXFLAGS = $(CFLAGS) $(DIF_CFLAGS) -fpermissive

#$(error XXX $(CFLAGS))

# This cleans nexus and local apps
clean:
	-$(RM) $(APP_BINS) $(DIF_APP_BINS) $(END_OBJS) $(END_UTILS_OBJS);

#prep_openssl:
#	$(MAKE) -C $(B_OPENSSL_BUILD_DIR)

do_prep:
	$(MAKE) prep_drmrootfs && \
	$(MAKE) prep_bcrypt && \
	$(MAKE) prep_cmndrmprdy && \
	$(MAKE) prep_bpiff

prep_drmrootfs:
	@echo [Compiling drmrootfs dependency ....];
	@$(MAKE) -C $(BSEAV_TOP)/lib/drmrootfs install > $(LOG_REDIRECT_FILE);
	@echo [Installing drmrootfs lib ....];
	install -c -m 0755 ${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR}/libdrmrootfs.so $(NEXUS_BIN_DIR)

prep_bcrypt:
	@echo [Compiling Bcrypt dependency ....];
	@$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install > $(LOG_REDIRECT_FILE);
	@$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt copy > $(LOG_REDIRECT_FILE);

prep_cmndrmprdy:
	@echo [..... Common DRM Playready target];
	@$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm playready > $(LOG_REDIRECT_FILE);
	@$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm copy > $(LOG_REDIRECT_FILE);

prep_bpiff:
	@echo [Compiling bpiff dependency ....];
	@$(MAKE) -C $(BSEAV_TOP)/lib/bpiff install > $(LOG_REDIRECT_FILE);

$(APP_BINS): $(END_OBJS) $(END_UTILS_OBJS)
	$(CC) -o $@ $@.o $(END_UTILS_OBJS) $(LDFLAGS)

$(DIF_APP_BINS): $(END_OBJS)
	$(CXX) -o $@ $@.o $(LDFLAGS) $(DIF_LDFLAGS)

${BPIFF_OBJ_ROOT}/%.o: %.c
	@echo [Compile... $<]
	$(CC) -c $< $(CFLAGS) -o $@

${BPIFF_OBJ_ROOT}/%.o: %.cpp
	@echo [Compile... $<]
	$(CXX) -c $< $(CXXFLAGS) -o $@

copy:
	@echo [Copying proper lib version...]
	cp -f -P $(B_OPENSSL_LIB_INSTALL_DIR)/libcrypto.* $(NEXUS_BIN_DIR)/
	cp -f ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR)/libcmndrmprdy.so $(NEXUS_BIN_DIR)
ifeq ($(SAGE_SUPPORT), y)
	cp -f $(PRDY_LIB_PATH)/libplayreadypk_host.so  ${NEXUS_BIN_DIR}
else
	cp -f $(PRDY_LIB_PATH)/libplayreadypk.so  ${NEXUS_BIN_DIR}
endif
	@echo [Copying executables...]
	cp $(APP_BINS) $(NEXUS_BIN_DIR)/
