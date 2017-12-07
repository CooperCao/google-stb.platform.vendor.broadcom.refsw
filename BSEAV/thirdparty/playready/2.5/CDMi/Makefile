############################################################
#  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

include ../../../../api/build/tools.mak
BSEAV_TOP = $(shell cd "../../../../" && ${PWD})
NEXUS_TOP = ${BSEAV_TOP}/../nexus

# Include nexus definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(NEXUS_TOP)/nxclient/include/nxclient.inc

# Include BSEAV definitions
include $(BSEAV_TOP)/opensource/openssl/openssl.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc
include $(BSEAV_TOP)/lib/security/common_drm/lib/cmndrm_dir.inc
include $(BSEAV_TOP)/lib/security/common_drm/common_drm.inc
include $(BSEAV_TOP)/lib/security/common_drm/lib/prdy_libdir.inc
include $(BSEAV_TOP)/thirdparty/playready/2.5/CDMi/prdy25_cdmi_libdir.inc

# Playready 2.5 CDMi only supported with SAGE
ifneq ($(SAGE_SUPPORT), y)
$(error Playready 2.5 CDMi only supported with SAGE.  Detected non-SAGE build.  Exiting...)
endif

# Variables and directories for out-of-source build
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}.${BCHP_VER}
B_REFSW_OBJ_ROOT ?= $(abspath ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR})
PRDY25_CDMI_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/thirdparty/playready/2.5/CDMi
D_FOR_LIB_INSTALL = ${PRDY25_CDMI_OBJ_ROOT}/${PRDY25_CDMI_LIBDIR}

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

CDMi_LIB_NAME=libcdmi
CDMi_LIB=$(D_FOR_LIB_INSTALL)/$(CDMi_LIB_NAME).a

CDMi_EXAMPLE=example/test_prdy25_cdmi

CDMi_SRC=\
	mediaenginesession.cpp \
	mediakeys.cpp \
	mediakeysession.cpp

CDMi_EXAMPLE_SRC=\
	example/http.cpp \
	example/main.cpp

CDMi_OBJECTS=$(foreach src,$(CDMi_SRC:.cpp=.o), $(src))
TARGET_CDMi_OBJECTS=$(addprefix ${PRDY25_CDMI_OBJ_ROOT}/,${CDMi_OBJECTS})
CDMi_EXAMPLE_OBJECTS=$(foreach src,$(CDMi_EXAMPLE_SRC:.cpp=.o), $(src))
TARGET_CDMi_EXAMPLE_OBJECTS=$(addprefix ${PRDY25_CDMI_OBJ_ROOT}/,${CDMi_EXAMPLE_OBJECTS})

CXXFLAGS = -MD -O2 -fno-rtti -fno-exceptions -I.
CXXFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS)
CXXFLAGS += -fPIC
CXXFLAGS += -I${BSEAV_TOP}/lib/security/common_crypto/include
# This is the minimum needed to compile and link with Nexus
CXXFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
CXXFLAGS += $(NXCLIENT_CFLAGS)

ifneq ($(MSDRM_PRDY_SUPPORT), y)
$(error MSDRM_PRDY_SUPPORT must be set to y)
endif

ifeq ($(SAGE_VERSION), 2x)
CXXFLAGS += -I${BSEAV_TOP}/thirdparty/playready/2.5/inc/2x
CXXFLAGS += -DSAGE_VERSION_IS_2X
else
CXXFLAGS += -I${BSEAV_TOP}/thirdparty/playready/2.5/inc
endif
CXXFLAGS += -DDRM_INLINING_SUPPORTED=1 -DDRM_BUILD_PROFILE=900 -DTARGET_LITTLE_ENDIAN=1 -DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1

PRDY_LIB_PATH := ${BSEAV_TOP}/${CMNDRM_PRDY_LIBDIR}

# Common DRM Flags
LDFLAGS += ${COMMON_DRM_LDFLAGS}
CXXFLAGS += ${COMMON_DRM_CFLAGS}

ifeq ($(SAGE_SUPPORT), y)
	CXXFLAGS += -DSAGE_SUPPORT
	CXXFLAGS += -I$(BSEAV_TOP)/../magnum/syslib/sagelib/include
	LDFLAGS += -L${NEXUS_BIN_DIR} -lcmndrm_tl
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
	LDFLAGS +=  -L${PRDY_LIB_PATH} -lplayreadypk64_host
	PLAYREADY_LIB_PATH = ${CP} -pv ${PRDY_LIB_PATH}/libplayreadypk64_host.so  ${NEXUS_BIN_DIR}
else
	LDFLAGS +=  -L${PRDY_LIB_PATH} -lplayreadypk_host
	PLAYREADY_LIB_PATH = ${CP} -pv ${PRDY_LIB_PATH}/libplayreadypk_host.so  ${NEXUS_BIN_DIR}
endif
else
	LDFLAGS +=  -L${PRDY_LIB_PATH} -lplayreadypk
	PLAYREADY_LIB_PATH = ${CP} -pv ${PRDY_LIB_PATH}/libplayreadypk.so  ${NEXUS_BIN_DIR}
	LDFLAGS += -L${BSEAV}/lib/security/common_drm/lib/${CMNDRM_DIR} -lcmndrm
endif

LDFLAGS += -L${D_FOR_LIB_INSTALL} -lcdmi
LDFLAGS += -L$(NEXUS_BIN_DIR)/ -lpthread -lm -lnexus${NEXUS_LIB_SUFFIX}
LDFLAGS +=-ldrmrootfs -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR}

.PHONY: prep_folder openssl bcrypt srai prep_cmndrm cmndrm_tl drmrootfs cdmi install

all: prep_folder openssl bcrypt srai prep_cmndrm cmndrm_tl drmrootfs cdmi install

prep_folder:
	@ mkdir -p ${PRDY25_CDMI_OBJ_ROOT}
	@ mkdir -p ${PRDY25_CDMI_OBJ_ROOT}/example
	@ mkdir -p ${D_FOR_LIB_INSTALL}

clean:
	-${Q_}$(RM) ${PRDY25_CDMI_OBJ_ROOT}/*.o ${PRDY25_CDMI_OBJ_ROOT}/*.d
	-${Q_}$(RM) ${D_FOR_LIB_INSTALL}/*.a ${D_FOR_LIB_INSTALL}/*.so
	-${Q_}$(RM) ${PRDY25_CDMI_OBJ_ROOT}/example/test_prdy25_cdmi
	-${Q_}$(RM) ${PRDY25_CDMI_OBJ_ROOT}/example/*.o ${PRDY25_CDMI_OBJ_ROOT}/example/*.d

drmrootfs:
	@echo [..... prep_drmrootfs] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/drmrootfs install > /dev/null
	${CP} -pv ${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR}/libdrmrootfs.so ${NEXUS_BIN_DIR}

openssl:
	@echo [..... prep_openssl] && \
	$(MAKE) -C $(B_OPENSSL_BUILD_DIR) > /dev/null

bcrypt:
	@echo [..... prep bcrypt] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install > /dev/null
	${CP} -pv ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/bcrypt/${BCRYPT_LIBDIR}/libbcrypt.so ${NEXUS_BIN_DIR}

srai:
ifeq ($(SAGE_SUPPORT),y)
	@echo [..... prep_srai] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/sage/srai re
endif

prep_cmndrm:
	@echo [ ..... Installing common DRM lib ..... ]
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
	${CP} -pv $(BSEAV_TOP)/lib/security/common_drm/lib/$(CMNDRM_DIR)/libcmndrm64.so $(NEXUS_BIN_DIR)
else
	${CP} -pv $(BSEAV_TOP)/lib/security/common_drm/lib/$(CMNDRM_DIR)/libcmndrm.so $(NEXUS_BIN_DIR)
endif

cmndrm_tl:
ifeq ($(SAGE_SUPPORT),y)
	@echo [..... Common DRM target] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm install > /dev/null
	${CP} -pv ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/common_drm/libcmndrm_tl.so ${NEXUS_BIN_DIR}
endif

cdmi: $(CDMi_LIB) $(CDMi_EXAMPLE)

${PRDY25_CDMI_OBJ_ROOT}/%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) -c -o $@ $< $(CXXFLAGS)

$(CDMi_LIB): $(TARGET_CDMi_OBJECTS)
	@echo [Linking... $(notdir $@)]
	@$(AR) cr $@ $^
	@ echo [Shared Library: $(CDMi_LIB_NAME).so]
	${Q_}$(CXX) -shared -o ${D_FOR_LIB_INSTALL}/$(CDMi_LIB_NAME).so -Wl,--whole-archive ${D_FOR_LIB_INSTALL}/$(CDMi_LIB_NAME).a -Wl,--no-whole-archive

# Build Application Example
$(CDMi_EXAMPLE): $(TARGET_CDMi_EXAMPLE_OBJECTS)
	@echo [Link... $@]
	${Q_}$(CXX) -o ${PRDY25_CDMI_OBJ_ROOT}/$@ $^ $(LDFLAGS)

install:
	@echo [Installing cdmi into NEXUS_BIN_DIR]
	${CP} -pv ${D_FOR_LIB_INSTALL}/libcdmi.so ${NEXUS_BIN_DIR}
	${CP} -pv ${PRDY25_CDMI_OBJ_ROOT}/example/test_prdy25_cdmi ${NEXUS_BIN_DIR}
	${CP} -pv ${BSEAV_TOP}/thirdparty/playready/2.5/CDMi/example/*.dat ${NEXUS_BIN_DIR}
	${PLAYREADY_LIB_PATH}

-include $(PRDY25_CDMI_OBJ_ROOT)/*.d $(PRDY25_CDMI_OBJ_ROOT)/example/*.d
