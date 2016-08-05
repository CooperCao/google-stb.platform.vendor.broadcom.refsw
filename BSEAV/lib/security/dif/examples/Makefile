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
############################################################
NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(URSR_TOP),)
URSR_TOP = $(shell pwd)/../../../../..
$(warning URSR_TOP is not defined. Set it as default: $(URSR_TOP))
endif

NEXUS_TOP=$(URSR_TOP)/nexus
BSEAV_TOP=$(URSR_TOP)/BSEAV

# Redirect logs to
LOG_REDIRECT_FILE ?= /dev/null

target ?= playback
$(warning target = $(target))

ifeq ($(COMMON_DRM_TOP),)
COMMON_DRM_TOP := $(BSEAV_TOP)/lib/security/common_drm
$(warning COMMON_DRM_TOP is not defined! Set it as default: $(COMMON_DRM_TOP))
endif

ifeq ($(B_REFSW_DEBUG),y)
BUILD_TYPE ?= debug
FLAGS += -g
else
ifeq ($(B_REFSW_DEBUG),)
BUILD_TYPE ?= debug
FLAGS += -g
else
BUILD_TYPE ?= release
BUILD_MODE ?= release
FLAGS += -O2
endif
endif

STANDALONE ?= n
NXCLIENT_SUPPORT ?= y
USE_CURL ?= y
PLAYREADY_TEST := y

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(BSEAV_TOP)/lib/openssl/openssl.inc
include $(COMMON_DRM_TOP)/lib/prdy_libdir.inc
include $(COMMON_DRM_TOP)/common_drm.inc
include $(BSEAV_TOP)/lib/security/bcrypt/bcrypt_libdir.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc

include $(BSEAV_TOP)/lib/security/dif/dif_libdir.inc
-include $(BSEAV_TOP)/lib/protobuf/protobuf_ver.inc
ifeq ($(NXCLIENT_SUPPORT),y)
include $(URSR_TOP)/nexus/nxclient/include/nxclient.inc
endif

# variables for out of source builds
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= $(abspath ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR})
DIF_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/dif

# set target dir
OUT_ROOT_DIR := $(DIF_OBJ_ROOT)/examples/bin

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

CFLAGS += -DPIC -fpic
CFLAGS += -Wl,--exclude-libs,ALL
CFLAGS += -Wno-variadic-macros -Wno-sign-compare
ifeq ($(ARCH_DIR),arm)
CFLAGS += -DARM_PLATFORM
endif
CFLAGS += $(B_OPENSSL_CFLAGS)

ifeq ($(NXCLIENT_SUPPORT),y)
CFLAGS += $(NXCLIENT_CFLAGS)
endif

CFLAGS += -Wall -fexceptions -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS += -D_FILE_OFFSET_BITS=64 -D_REENTRANT -fPIC -DPIC
CFLAGS += -D_GNU_SOURCE=1
CFLAGS += $(FLAGS) $(NEXUS_CFLAGS)  $(addprefix -D,$(NEXUS_APP_DEFINES))

CFLAGS += $(COMMON_DRM_CFLAGS)

CFLAGS := $(filter-out -std=c89 -Wstrict-prototypes, $(CFLAGS))

ifeq ($(NXCLIENT_SUPPORT),y)
TARGET_SRCS := ./$(target)/$(target).cpp
TARGET_SRCS += ./$(target)/bgui.c
INCLUDES += -I$(NEXUS_TOP)/modules/graphics2d/include
else
TARGET_SRCS := ./$(target)/$(target)_single.cpp
endif

DEFINES := $(addprefix -D, $(DEFINES))

INCLUDES += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})

INCLUDES += $(addprefix -I, ${DIF_INCLUDES})

# Filter out all non c/cpp files
TARGET_SRCS    := $(filter %.c %.cpp, $(TARGET_SRCS))

# Make object files to use as dependencies
TARGET_OBJS := $(patsubst %.cpp, %.o, $(TARGET_SRCS))
TARGET_OBJS := $(patsubst %.c, %.o, $(TARGET_OBJS))
TARGET_OBJS := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(TARGET_OBJS))

ifeq ($(USE_CURL),y)
LDFLAGS := -lcurl $(CURL_LDFLAGS)
endif

ifeq ($(NXCLIENT_SUPPORT),y)
LDFLAGS += $(NXCLIENT_LDFLAGS)
endif

ifeq ($(WIDEVINE_SUPPORT),y)
LDFLAGS_THIRDPARTY_LIB := $(PROTOBUF_LDFLAGS) $(GTEST_LDFLAGS)
ifeq ($(SAGE_SUPPORT),y)
LDFLAGS_OEMCRYPTO_CDM_LIB := -L$(NEXUS_BIN_DIR) -lwvcdm -loemcrypto_tl -lsrai
else
LDFLAGS_OEMCRYPTO_CDM_LIB := -L$(NEXUS_BIN_DIR) -lwvcdm -loemcrypto
endif
endif

LDFLAGS += $(COMMON_DRM_LDFLAGS)
LDFLAGS += $(B_OPENSSL_LDFLAGS)
LDFLAGS += $(LDFLAGS_THIRDPARTY_LIB)

LDFLAGS += $(COMMON_DRM_LDFLAGS)
LDFLAGS += -L$(NEXUS_BIN_DIR) -lnexus
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/BSEAV/lib/security/bcrypt/$(BCRYPT_LIBDIR) -lbcrypt \
           -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR} -ldrmrootfs
LDFLAGS += -L$(COMMON_DRM_TOP)/lib/$(CMNDRM_DIR) -lcmndrm
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/$(CMNDRM_PRDY_LIBDIR) -lcmndrmprdy
ifeq ($(SAGE_SUPPORT),y)
LDFLAGS += -L$(COMMON_DRM_TOP)/lib/$(CMNDRM_DIR) -lcmndrm_tl
LDFLAGS += -lplayready30pk
LDFLAGS += -lplayreadypk_host
else
LDFLAGS += -lplayreadypk
endif
TARGET_LDFLAGS += $(LDFLAGS) -L$(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/dif/$(DIF_LIBDIR) -ldif

.PHONY: clean all $(target)_dif

.SUFFIXES: .cpp .c

$(OUT_ROOT_DIR)/%.o: %.c
	@test -d $(OUT_ROOT_DIR) || mkdir -pm 775 $(OUT_ROOT_DIR)
	@test -d $(@D) || mkdir -pm 775 $(@D)
	@echo [Compiling $<]
	${CC} -c $(FLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

$(OUT_ROOT_DIR)/%.o: %.cpp
	@test -d $(OUT_ROOT_DIR) || mkdir -pm 775 $(OUT_ROOT_DIR)
	@test -d $(@D) || mkdir -pm 775 $(@D)
	@echo [Compiling $<]
	${CXX} -c $(FLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

all: $(target)_dif

prep_dif:
	@echo [..... prep_dif target ]
	@if [ ! -d $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/BSEAV/lib/security/dif/$(DIF_LIBDIR) ]; then \
		echo [ building dif .... ]; \
		$(MAKE) -C $(BSEAV_TOP)/lib/security/dif  > $(LOG_REDIRECT_FILE); \
	fi
	@echo [..... prep_dif target done ]

report:
	@echo "Reporting ..."
	@echo NEXUS_BIN_DIR = "$(NEXUS_BIN_DIR)"
	@echo NEXUS_TOP = "$(NEXUS_TOP)"
	@echo
	@echo INCLUDES = "$(INCLUDES)"
	@echo
	@echo CFLAGS = "$(CFLAGS)"
	@echo
	@echo LDFLAGS = "$(LDFLAGS)"
	@echo
	@echo bcrypt lib dir is   "$(BCRYPT_LIBDIR)	"
	@echo drmrootfs lib dir is "$(DRMROOTFS_LIBDIR)"
	@echo nexus bin dir="$(NEXUS_BIN_DIR)"
	@echo STAND ALONE TEST = "$(STANDALONE)"
	@echo NXCLIENT_SUPPORT = "$(NXCLIENT_SUPPORT)"
	@echo
	@echo WVCDM_LIBDIR = "$(WVCDM_LIBDIR)"
	@echo OUT_ROOT_DIR = "$(OUT_ROOT_DIR)"
	@echo TARGET_OBJS = "$(TARGET_OBJS)"
	@echo CC = "${CC}"
	@echo CXX = "${CXX}"
	@echo ARCH_DIR = "${ARCH_DIR}"
	@echo B_OPENSSL_CFLAGS = "$(B_OPENSSL_CFLAGS)"
	@echo B_OPENSSL_LDFLAGS = "$(B_OPENSSL_LDFLAGS)"
	@echo B_OPENSSL_BUILD_DIR = "$(B_OPENSSL_BUILD_DIR)"
	@echo B_OPENSSL_LIB_INSTALL_DIR = "$(B_OPENSSL_LIB_INSTALL_DIR)"
	@echo ...

help:
	@echo "######################################"
	@echo " "
	@echo "First 'export URSR_TOP=<directory to the top of URSR source tree >'"
	@echo "Then  'make'"
	@echo " "
	@echo "######################################"

clean:
	@echo "Cleaning ..."
	@rm -rf $(OUT_ROOT_DIR)

$(target)_dif: $(TARGET_OBJS) prep_dif
	@echo "Building $@ ..."
	@mkdir -p $(OUT_ROOT_DIR)/final/
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEFINES) $(TARGET_OBJS) $(TARGET_LDFLAGS) $(CDM_API_LDFLAGS) $(LDFLAGS_OEMCRYPTO_CDM_LIB)  -o $(OUT_ROOT_DIR)/final/$@
	@install -m 0755 $(OUT_ROOT_DIR)/final/$@ $(NEXUS_BIN_DIR)
