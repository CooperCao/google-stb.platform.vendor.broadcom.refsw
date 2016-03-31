#/******************************************************************************
#* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#*
#* This program is the proprietary software of Broadcom and/or its
#* licensors, and may only be used, duplicated, modified or distributed pursuant
#* to the terms and conditions of a separate, written license agreement executed
#* between you and Broadcom (an "Authorized License").  Except as set forth in
#* an Authorized License, Broadcom grants no license (express or implied), right
#* to use, or waiver of any kind with respect to the Software, and Broadcom
#* expressly reserves all rights in and to the Software and all intellectual
#* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#*
#* Except as expressly set forth in the Authorized License,
#*
#* 1. This program, including its structure, sequence and organization,
#*    constitutes the valuable trade secrets of Broadcom, and you shall use all
#*    reasonable efforts to protect the confidentiality thereof, and to use
#*    this information only in connection with your use of Broadcom integrated
#*    circuit products.
#*
#* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#*
#* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#******************************************************************************/
NEXUS_PLATFORM ?= $(PLATFORM)

ifeq ($(URSR_TOP),)
$(error URSR_TOP is not defined! Make it point to your view's root folder [i.e. one directory above where nexus, magnum, BSEAV exist in your view])
else
NEXUS_TOP ?= $(URSR_TOP)/nexus
BSEAV_TOP ?= $(URSR_TOP)/BSEAV
APPLIBS_TOP ?= $(URSR_TOP)/AppLibs
endif

ifeq ($(COMMON_DRM_TOP),)
$(warning COMMON_DRM_TOP is not defined! Assuming it points to the current directory)
COMMON_DRM_TOP := ../../../../common_drm
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
FLAGS += -O2
endif
endif

BRCM_IMPL ?= y
STANDALONE ?= n

# set target dir
INSTALL_PREFIX ?= usr/local
TARGET_TOP ?= ${APPLIBS_TOP}/target
APPLIBS_TARGET_ROOTDIR ?= ${TARGET_TOP}/${PLATFORM}${BCHP_VER_LOWER}.${B_REFSW_ARCH}.${BUILD_TYPE}
APPLIBS_TARGET_DIR ?= ${APPLIBS_TARGET_ROOTDIR}/${INSTALL_PREFIX}
APPLIBS_TARGET_LIB_DIR ?= ${APPLIBS_TARGET_DIR}/lib

ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(COMMON_DRM_TOP)/common_drm.inc

CURRENT_DIR := $(shell pwd)
WIDEVINE_CENC_DIR := $(shell readlink -f ..)
$(warning WIDEVINE_CENC_DIR is $(WIDEVINE_CENC_DIR))

include $(WIDEVINE_CENC_DIR)/oemcrypto_libdir.inc

OUT_ROOT_DIR := $(CURRENT_DIR)/bin

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

CFLAGS += -DPIC -fpic
CFLAGS += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})
CFLAGS += $(B_OPENSSL_CFLAGS)
CFLAGS += $(COMMON_DRM_CFLAGS)

OEMCRYPTO_SRCS := ./src/oemcrypto_brcm_TL.cpp

ifeq ($(SAGE_SUPPORT),y)
include $(URSR_TOP)/magnum/syslib/sagelib/bsagelib_public.inc
CFLAGS += -I${BSAGELIB_INCLUDES}
CFLAGS += -I$(URSR_TOP)/BSEAV/lib/security/sage/platforms/include
CFLAGS += -I$(URSR_TOP)/BSEAV/lib/security/sage/srai/include
endif

DEFINES := OEC_MOCK CAN_INSTALL_KEYBOX CDM_IMPLEMENTATION NDEBUG

ifeq ($(BRCM_IMPL),y)
DEFINES += BRCM_IMPL
endif

ifeq ($(STANDALONE),y)
DEFINES += OEC_TEST_STANDALONE
endif

ifeq ($(SINGLE_PROCESS),y)
DEFINES += SINGLE_PROCESS
endif

INCLUDES :=  \
    $(CURRENT_DIR)/include \
    $(WIDEVINE_CENC_DIR)/core/include

# Filter out all non c/cpp files
OEMCRYPTO_SRCS := $(filter %.c %.cpp, $(OEMCRYPTO_SRCS))

# Make object files to use as dependencies
OEMCRYPTO_OBJS := $(patsubst %.cpp, %.o, $(OEMCRYPTO_SRCS))
OEMCRYPTO_OBJS := $(patsubst %.c, %.o, $(OEMCRYPTO_OBJS))
OEMCRYPTO_OBJS := $(patsubst ./%, $(OUT_ROOT_DIR)/%, $(OEMCRYPTO_OBJS))
OEMCRYPTO_OBJS := $(patsubst ../%, $(OUT_ROOT_DIR)/CENC/%, $(OEMCRYPTO_OBJS))

# Add compiler switches as needed
INCLUDES := $(addprefix -I, $(INCLUDES))
DEFINES := $(addprefix -D, $(DEFINES))

# Setup compilation flags
CFLAGS += -Wall -fexceptions -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS += -D_FILE_OFFSET_BITS=64 -D_REENTRANT -fPIC -DPIC
CFLAGS += -D_GNU_SOURCE=1
CFLAGS += $(FLAGS) $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

CFLAGS += $(COMMON_DRM_CFLAGS)

LDFLAGS += $(B_OPENSSL_LDFLAGS)

ifeq ($(BRCM_IMPL),y)
LDFLAGS += -L$(COMMON_DRM_TOP)/../bcrypt/$(BCRYPT_LIBDIR) -L$(BSEAV_TOP)/lib/drmrootfs/$(DRMROOTFS_LIBDIR)
LDFLAGS += $(COMMON_DRM_LDFLAGS)
LDFLAGS += -lbcrypt -ldrmrootfs
LDFLAGS+=-L$(NEXUS_BIN_DIR) -lnexus
endif # end of BRCM_IMPL

.PHONY: clean all install

.SUFFIXES: .c .cpp

$(OUT_ROOT_DIR)/src/%.o: src/%.c
	@ test -d $(OUT_ROOT_DIR) || mkdir -pm 775 $(OUT_ROOT_DIR)
	@ test -d $(@D) || mkdir -pm 775 $(@D)
	@ ${CC} -c $(FLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

$(OUT_ROOT_DIR)/src/%.o: src/%.cpp
	@ test -d $(OUT_ROOT_DIR) || mkdir -pm 775 $(OUT_ROOT_DIR)
	@ test -d $(@D) || mkdir -pm 775 $(@D)
	@ ${CXX} -c $(FLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

all: oemcrypto_tl

report:
	@echo "Reporting ..."
	@echo "CURRENT_DIR is $(CURRENT_DIR)"
	@echo "NEXUS_TOP is $(NEXUS_TOP)"
	@echo "WIDEVINE_CENC_DIR is $(WIDEVINE_CENC_DIR)"
	@echo WV SOURCES
	@echo $(OEMCRYPTO_SRCS)
	@echo $(OEMCRYPTO_OBJS)
	@echo
	@echo ALL INCLUDES
	@echo $(INCLUDES)
	@echo LDFLAGS
	@echo $(LDFLAGS)
	@echo
	@echo CFLAGS
	@echo $(CFLAGS)
	@echo
	@echo "bcrypt lib dir is $(BCRYPT_LIBDIR)"
	@echo "drmroots lib dir is $(DRMROOTFS_LIBDIR)"
	@echo "BRCM_IMPL = $(BRCM_IMPL)"
	@echo "NEXUS_BIN_DIR=$(NEXUS_BIN_DIR)"
	@echo "STAND ALONE TEST = $(STANDALONE)"
	@echo
	@echo "OEMCRYPTO_LIBDIR = $(WIDEVINE_CENC_DIR)/oemcrypto/$(OEMCRYPTO_LIBDIR)"
	@echo "=====================COMMON_DRM_LDFLAGS====================="
	@echo $(COMMON_DRM_LDFLAGS)
	@echo "=====================COMMON_DRM_CFLAGS====================="
	@echo $(COMMON_DRM_CFLAGS)
	@echo

clean:
	@echo "Cleaning ..."
	rm -f $(OUT_ROOT_DIR)/final/liboemcrypto_tl.so
	rm -f $(OEMCRYPTO_OBJS)
	rm -f $(WIDEVINE_CENC_DIR)/oemcrypto/$(OEMCRYPTO_LIBDIR)/liboemcrypto_tl.so

oemcrypto_tl: $(OEMCRYPTO_OBJS)
	@echo "Building $@ ..."
	mkdir -p $(OUT_ROOT_DIR)/final/
	@ ${CXX} $(FLAGS) $(CFLAGS) $(DEFINES) $(LDFLAGS) -lpthread $(OEMCRYPTO_OBJS)  -shared -o $(OUT_ROOT_DIR)/final/lib$@.so

copy: copy-oemcrypto-builds

install: copy-prebuilt
	@echo [oemcrypto_tl libs installed...]

copy-oemcrypto-builds:
	@echo [Copying liboemcrypto_tl.so...]
	cp -f $(OUT_ROOT_DIR)/final/liboemcrypto_tl.so $(NEXUS_BIN_DIR)
	cp -f $(OUT_ROOT_DIR)/final/liboemcrypto_tl.so $(APPLIBS_TARGET_LIB_DIR);

copy-prebuilt:
	@echo [Copying prebuilt liboemcrypto_tl.so...]
	cp -f $(OUT_ROOT_DIR)/final/liboemcrypto_tl.so $(WIDEVINE_CENC_DIR)/oemcrypto/$(OEMCRYPTO_LIBDIR)
