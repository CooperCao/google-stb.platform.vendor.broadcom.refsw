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
include $(BSEAV_TOP)/lib/openssl/openssl.inc
include $(BSEAV_TOP)/lib/drmrootfs/drmrootfs_libdir.inc

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

# Set ARCH and lib installation path based on arch
ifeq ($(filter ${B_REFSW_ARCH}, arm-linux aarch64-linux), ${B_REFSW_ARCH})
ARCH_DIR_PRDY_CDMI=arm
else
# Distinguish between 65nm-askm and 40nm MIPS products
ifeq ($(filter $(BCHP_CHIP), 7400 7401 7405 7413 7325 7335 7125 7340 7342 7410 7420 7468), $(BCHP_CHIP))
ARCH_DIR_PRDY_CDMI=65nm-askm
else
ARCH_DIR_PRDY_CDMI=mips
endif
endif

ODIR=$(ARCH_DIR_PRDY_CDMI)

CDMi_LIB_NAME=libcdmi
CDMi_LIB=$(ODIR)/$(CDMi_LIB_NAME).a

CDMi_EXAMPLE=$(ODIR)/cdmi

CDMi_SRC=\
	mediaenginesession.cpp \
	mediakeys.cpp \
	mediakeysession.cpp

CDMi_EXAMPLE_SRC=\
	example/http.cpp \
	example/main.cpp

CDMi_OBJECTS=$(foreach src,$(CDMi_SRC:.cpp=.o), $(src))
CDMi_EXAMPLE_OBJECTS=$(foreach src,$(CDMi_EXAMPLE_SRC:.cpp=.o), $(src))

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

CXXFLAGS += -I${BSEAV_TOP}/lib/playready/2.5/inc
CXXFLAGS += -DDRM_INLINING_SUPPORTED=1 -DDRM_BUILD_PROFILE=900 -DTARGET_LITTLE_ENDIAN=1 -DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1

ifeq ($(SAGE_SUPPORT), y)
	include $(BSEAV_TOP)/lib/security/common_drm/common_drm.inc
	LDFLAGS += ${COMMON_DRM_LDFLAGS}
        CXXFLAGS += ${COMMON_DRM_CFLAGS}
	CXXFLAGS += -I$(BSEAV_TOP)/../magnum/syslib/sagelib/include
	LDFLAGS += -L${NEXUS_BIN_DIR} -lcmndrm_tl
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
	LDFLAGS +=  -L${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY_CDMI}/lib -lplayreadypk64_host
	PLAYREADY_LIB_PATH = ${CP} -pv ${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY_CDMI}/lib/libplayreadypk64_host.so  ${NEXUS_BIN_DIR}
else
	LDFLAGS +=  -L${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY_CDMI}/lib -lplayreadypk_host
	PLAYREADY_LIB_PATH = ${CP} -pv ${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY_CDMI}/lib/libplayreadypk_host.so  ${NEXUS_BIN_DIR}
endif
else
	LDFLAGS +=  -L${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY_CDMI}/lib -lplayreadypk
	PLAYREADY_LIB_PATH = ${CP} -pv ${BSEAV_TOP}/lib/playready/2.5/bin/${ARCH_DIR_PRDY_CDMI}/lib/libplayreadypk.so  ${NEXUS_BIN_DIR}
endif

LDFLAGS += -L${BSEAV_TOP}/lib/playready/2.5/CDMi/${ARCH_DIR_PRDY_CDMI} -lcdmi
LDFLAGS += -L$(NEXUS_BIN_DIR)/ -lpthread -lm -lnexus${NEXUS_LIB_SUFFIX}
LDFLAGS +=-ldrmrootfs -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR}

.PHONY:openssl srai cmndrm_tl drmrootfs cmndrmprdy cdmi install

all: openssl srai cmndrm_tl drmrootfs cmndrmprdy cdmi install

cdmi: $(CDMi_LIB) $(CDMi_EXAMPLE)

$(ODIR)/%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) -c -o $@ $< $(CXXFLAGS)

$(CDMi_LIB): $(CDMi_OBJECTS)
	@echo [Linking... $(notdir $@)]
	@$(AR) cr $@ $^
	@ echo [Shared Library: $(CDMi_LIB_NAME).so]
	${Q_}$(CXX) -shared -o $(ODIR)/$(CDMi_LIB_NAME).so -Wl,-soname,$(CDMi_LIB_NAME).so -Wl,--whole-archive  $(ODIR)/$(CDMi_LIB_NAME).a -Wl,--no-whole-archive

# Build Application Example
$(CDMi_EXAMPLE): $(CDMi_EXAMPLE_OBJECTS)
	@echo [Link... $@]
	${Q_}$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	-${Q_}$(RM) *.o *.d $(ODIR)/*.a $(ODIR)/*.so $(ODIR)/cdmi
	-${Q_}$(RM) example/*.o *example/*.d

drmrootfs:
	@echo [..... prep_drmrootfs] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/drmrootfs install > /dev/null

openssl:
	@echo [..... prep_openssl] && \
	$(MAKE) -C $(B_OPENSSL_BUILD_DIR) > /dev/null

srai:
ifeq ($(SAGE_SUPPORT),y)
	@echo [..... prep_srai] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/sage/srai re
endif

cmndrm_tl:
ifeq ($(SAGE_SUPPORT),y)
	@echo [..... Common DRM target] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm install > /dev/null
endif

cmndrmprdy:
	@echo [..... Common DRM Playready target] && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm playready && \
	$(MAKE) -C $(BSEAV_TOP)/lib/security/common_drm install > /dev/null

install:
	${CP} -pv ${BSEAV_TOP}/lib/playready/2.5/CDMi/$(ODIR)/libcdmi.so ${NEXUS_BIN_DIR}
	${CP} -pv ${BSEAV_TOP}/lib/playready/2.5/CDMi/$(ODIR)/cdmi ${NEXUS_BIN_DIR}
	${CP} -pv ${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR}/libdrmrootfs.so ${NEXUS_BIN_DIR}
	${PLAYREADY_LIB_PATH}

-include $(ODIR)/*.d
