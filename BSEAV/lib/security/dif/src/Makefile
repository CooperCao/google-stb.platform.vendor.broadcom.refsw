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
URSR_TOP ?= ../../../../..
NEXUS_TOP := $(URSR_TOP)/nexus
BSEAV_TOP := $(URSR_TOP)/BSEAV/
APPLIBS_TOP := $(URSR_TOP)/AppLibs

NXCLIENT_SUPPORT ?= y

USE_CURL ?= y

# This will be turned on after DecryptOpaque is enabled in sage/playready
USE_PR_DECRYPT_OPAQUE ?= n

# Build the name of this makefile, relative to B_REFSW_TOP (used for recipe tracing)
# Keep this line before any includes!
B_THIS_MAKEFILE_NAME := $(subst $(abspath ${NEXUS_TOP}/..),,$(abspath $(lastword $(MAKEFILE_LIST))))

# variables for out of source builds
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= $(abspath ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR})
DIF_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/dif

# Include nexus/applibs definitions
CHECK_APPLIBS = $(shell ls $(APPLIBS_TOP)/common/common.inc 2> /dev/null)
ifeq ($(CHECK_APPLIBS),)
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
else
include $(APPLIBS_TOP)/common/common.inc
endif

ifeq ($(NXCLIENT_SUPPORT),y)
include $(URSR_TOP)/nexus/nxclient/include/nxclient.inc
endif

# Sanity check that we received a valid platform
ifndef BCHP_CHIP
$(error Unsupported platform $(NEXUS_PLATFORM))
endif

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

SECURITY_TOP := $(BSEAV_TOP)/lib/security
COMMON_DRM_TOP := $(SECURITY_TOP)/common_drm
DIF_TOP := $(SECURITY_TOP)/dif

include $(COMMON_DRM_TOP)/lib/prdy_libdir.inc
include $(COMMON_DRM_TOP)/common_drm.inc
include $(DIF_TOP)/dif_libdir.inc

# Custom include paths
CFLAGS += -I./include
ifeq ($(WIDEVINE_SUPPORT),y)
ifneq ($(MAKECMDGOALS),clean)
include $(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/third_party/widevine/CENC21/widevine/widevine21.inc
endif
CFLAGS += -DENABLE_WIDEVINE $(WIDEVINE21_CFLAGS)
endif
CFLAGS += -I$(SECURITY_TOP)/common_drm/include
CFLAGS += -I$(SECURITY_TOP)/sage/srai/include
CFLAGS += -I$(URSR_TOP)/magnum/syslib/sagelib/include
CFLAGS += -DPIC -fpic
CFLAGS += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})
CFLAGS += $(addprefix -I, ${DIF_INCLUDES})
CFLAGS += $(addprefix -D, ${BSAGELIB_DEFINES})

# PlayreadyPK 3.0 is only supported with Sage
ifeq ($(SAGE_SUPPORT), y)
include $(BSEAV_TOP)/thirdparty/playready/http/prdyhttp.inc
CFLAGS += -DUSE_PK_NAMESPACES=1
CFLAGS += -DMSDRM_PRDY30
CFLAGS += -DDRM_BUILD_PROFILE=900
CFLAGS += -DTARGET_LITTLE_ENDIAN=1
CFLAGS += -DTARGET_SUPPORTS_UNALIGNED_DWORD_POINTERS=1
CFLAGS += -I${BSEAV_TOP}/lib/drmrootfs
CFLAGS += -I${BSEAV_TOP}/thirdparty/playready/3.0/inc
CFLAGS += $(addprefix -I, ${PRDY_HTTP_INCLUDES})
LDFLAGS += ${PRDY_HTTP_LDFLAGS}
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
CFLAGS += -DDRM_64BIT_TARGET=1 -D__arch64__
endif
endif

ifeq ($(USE_PR_DECRYPT_OPAQUE), y)
CFLAGS += -DUSE_PR_DECRYPT_OPAQUE
endif

ifeq ($(USE_CURL),y)
include $(BSEAV_TOP)/opensource/curl/curl.inc
CFLAGS += $(CURL_CFLAGS)
CFLAGS += -DUSE_CURL
CFLAGS += -DSKIP_PEER_VERIFICATION
ifneq ($(APPLIBS_TARGET_INC_DIR),)
CFLAGS += -I$(APPLIBS_TARGET_INC_DIR)
endif
LDFLAGS += -lcurl $(CURL_LDFLAGS)
ifneq ($(APPLIBS_TARGET_LIB_DIR),)
LDFLAGS += -L$(APPLIBS_TARGET_LIB_DIR)
endif
endif

ifeq ($(SAGE_SUPPORT),y)
CFLAGS += -DUSE_SECURE_PLAYBACK
endif

ifeq ($(NXCLIENT_SUPPORT),y)
CFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS += $(NXCLIENT_LDFLAGS)
endif

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

#allow c++ style comments
CFLAGS := $(filter-out -std=c89 -Wstrict-prototypes, $(CFLAGS))

#allow long long & variadic macros
CFLAGS += -Wno-long-long -Wno-variadic-macros

############################################################################
#
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

# DRM Makefile exclude the 'include' subfolder which is built with the makefile
SRC_DIR := .
D_ALL_MODULE_DIR := $(filter-out %include, $(shell find . -maxdepth 1 -type d))

vpath %.cpp ${SRC_DIR}
vpath %.c ${SRC_DIR}
vpath %.h ${SRC_DIR}
LIB_BASENAME := dif

############
# Set F_PUBLIC_INCS to the basenames of include files that are to be
# used by other libraries or applications.  These files will be
# installed into .../${BCHP_VER}/include/
#####
F_PUBLIC_INCS += $(sort $(notdir $(shell find ${D_ALL_MODULE_DIR} -name '*.h')))

#$(error ${F_PUBLIC_INCS})

############
# If your library depends on another shared lib, you should add that
# lib here (and any additional lib directories to search).
#####
LDFLAGS += $(NEXUS_LD_LIBRARIES)

############
# Directory where to install the headers
#####
D_FOR_INC_INSTALL := include

############
# You probably shouldn't have to modify the text below this point.
#####
F_LIB_NAMES := lib${LIB_BASENAME}.a  lib${LIB_BASENAME}.so
F_PUBLIC_LIBS += $(addprefix ${DIF_OBJ_ROOT}/,${F_LIB_NAMES})
D_FOR_LIB_INSTALL := ${DIF_OBJ_ROOT}/${DIF_LIBDIR}

F_SRC_EXCLUDES = android_decryptor_factory.cpp android_wv_decryptor.cpp

ifneq ($(SAGE_SUPPORT), y)
F_SRC_EXCLUDES += pr30_decryptor.cpp
endif

ifneq ($(WIDEVINE_SUPPORT),y)
F_SRC_EXCLUDES += wv_decryptor.cpp
endif

F_SRCS := $(filter-out ${F_SRC_EXCLUDES}, $(notdir $(wildcard $(addsuffix /*.c*, ${SRC_DIR}))))
F_OBJS := $(patsubst %.cpp,%.o, ${F_SRCS})
F_OBJS := $(patsubst %.c,%.o, ${F_OBJS})
END_OBJS := $(addprefix ${DIF_OBJ_ROOT}/,${F_OBJS})
F_INSTALLED_LIBS := $(addprefix ${D_FOR_LIB_INSTALL}/, ${F_LIB_NAMES})
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##

#$(error END_OBJS = $(END_OBJS))
#$(error CFLAGS = $(CFLAGS))
#$(warning F_LIB_NAMES: ${F_LIB_NAMES} )
#$(warning F_PUBLIC_LIBS: ${F_PUBLIC_LIBS} )
#$(warning F_INSTALLED_LIBS: ${F_INSTALLED_LIBS} )
#$(warning D_FOR_LIB_INSTALL: ${D_FOR_LIB_INSTALL} )

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
.PHONY : all
all: ${F_PUBLIC_LIBS}

${F_PUBLIC_LIBS} ${END_OBJS} : | prep_folder

.PHONY : prep_folder
prep_folder:
	$(Q_)mkdir -p ${DIF_OBJ_ROOT}/

.PHONY : clean
clean:
	$(Q_)echo Cleaning objects... [${END_OBJS}]
	$(Q_)rm -f ${END_OBJS}
	$(Q_)echo Cleaning libraries... [${F_PUBLIC_LIBS}]
	$(Q_)rm -f ${F_INSTALLED_LIBS}
	$(Q_)rm -rf ${DIF_OBJ_ROOT}/$(DIF_LIBDIR)


############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
${D_FOR_LIB_INSTALL}/%.so : ${END_OBJS}
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	${CC} -shared -o $@  ${LDFLAGS}  $^ -Wl,-Bdynamic
else
	$(Q_)echo [Linking shared library .... $@ ]
	$(Q_)${CC} -shared -o $@ ${LDFLAGS} ${END_OBJS} -Wl,-dy
endif

${D_FOR_LIB_INSTALL}/%.a : ${END_OBJS}
	$(Q_)echo [Linking static library .... $@ ]
	$(Q_)${AR} rc $@ ${END_OBJS}

${DIF_OBJ_ROOT}/%.o: %.c
	$(Q_)echo [Compiling  with $(CC).... $< ]
	$(Q_)${CC} ${CFLAGS} -c -o $@ $<

${DIF_OBJ_ROOT}/%.o: %.cpp
	$(Q_)echo [Compiling  with $(CC).... $< ]
	$(Q_)${CC} ${CFLAGS} -c -o $@ $<

idirs:
	$(Q_)[ -d ${D_FOR_LIB_INSTALL} ] || mkdir -p ${D_FOR_LIB_INSTALL}
	$(Q_)[ -d ${D_FOR_INC_INSTALL} ] || mkdir -p ${D_FOR_INC_INSTALL}

install: all idirs ${F_INSTALLED_LIBS}
	$(Q_)echo [Copying proper dif lib version...]
	$(Q_)install -m 0755 ${D_FOR_LIB_INSTALL}/libdif.so $(NEXUS_BIN_DIR)
	$(Q_)if [ -d $(APPLIBS_TOP) ] && [ -d $(APPLIBS_TARGET_LIB_DIR) ]; then \
		echo [Copying libdif.so to applibs target]; \
		install -m 0755 ${D_FOR_LIB_INSTALL}/libdif.so $(APPLIBS_TARGET_LIB_DIR); \
	fi
