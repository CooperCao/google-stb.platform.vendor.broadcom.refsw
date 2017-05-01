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
URSR_TOP ?= ../../..
MAGNUM_TOP := $(URSR_TOP)/magnum
NEXUS_TOP := $(URSR_TOP)/nexus
BSEAV_TOP := $(URSR_TOP)/BSEAV
APPLIBS_TOP := $(URSR_TOP)/AppLibs

USE_DIF ?= n

# Include nexus definitions
CHECK_APPLIBS = $(shell ls $(APPLIBS_TOP)/common/common.inc 2> /dev/null)
ifeq ($(CHECK_APPLIBS),)
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
else
include $(APPLIBS_TOP)/common/common.inc
endif

include $(NEXUS_TOP)/build/os/linuxuser/os_tools.inc

include $(BSEAV_TOP)/opensource/openssl/openssl.inc

# Sanity check that we received a valid platform
ifndef BCHP_CHIP
$(error Unsupported platform $(NEXUS_PLATFORM))
endif

ifneq ($(findstring 2.5, ${MSDRM_PRDY_SDK_VERSION}), 2.5)
$(error unsupported Playready VERSION = ${MSDRM_PRDY_SDK_VERSION})
endif

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

include $(NEXUS_TOP)/base/base.inc
include $(NEXUS_TOP)/modules/dma/dma.inc
include $(NEXUS_TOP)/modules/core/core.inc

include $(MAGNUM_TOP)/basemodules/mem/bmem.inc
include $(MAGNUM_TOP)/portinginterface/hsm/bhsm.inc

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

include $(BSEAV_TOP)/lib/security/common_drm/lib/cmndrm_dir.inc

include $(BSEAV_TOP)/lib/bpiff/bpiff.inc

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= $(abspath ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR})
BPIFF_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/bpiff
UTILS_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/utils

# Custom include paths
CFLAGS += -I./include
CFLAGS += -I$(NEXUS_TOP)/modules/dma/$(BCHP_CHIP)/include
CFLAGS += -I$(NEXUS_TOP)/modules/dma/$(BCHP_CHIP)/include/priv
CFLAGS += -I$(NEXUS_TOP)/modules/security/$(BCHP_CHIP)/include
CFLAGS += -I$(NEXUS_TOP)/modules/security/$(BCHP_CHIP)/include/priv
CFLAGS += -I$(NEXUS_TOP)/modules/core/$(BCHP_CHIP)/include
CFLAGS += -I$(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/include
CFLAGS += -I$(MAGNUM)/portinginterface/hsm/$(BCHP_CHIP)/${BCHP_VER}/common
CFLAGS += -I$(BSEAV_TOP)/lib/security/common_drm/include/priv
CFLAGS += -I$(BSEAV_TOP)/lib/security/common_drm/include
CFLAGS += -DPIC -fpic
CFLAGS += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

CFLAGS += $(B_OPENSSL_CFLAGS)

CFLAGS += -I${BSEAV_TOP}/lib/security/common_crypto/include
CFLAGS += -I$(BSEAV_TOP)/lib/utils
CFLAGS += -I$(BSEAV_TOP)/lib/media

ifeq ($(SAGE_SUPPORT), y)
OPAQUE_ENCRYPT ?= y
else
OPAQUE_ENCRYPT = n
endif

ifeq ($(OPAQUE_ENCRYPT), y)
CFLAGS += -DOPAQUE_ENCRYPT
endif

ifeq ($(USE_DIF), y)
CFLAGS += -I${BSEAV_TOP}/lib/bfile
endif

#allow c++ style comments
CFLAGS += -std=c99

############################################################################
#
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

# DRM Makefile exclude the 'include' subfolder which is built with the makefile
D_ALL_MODULE_DIR := $(filter-out %include, $(shell find . -maxdepth 1 -type d))
#$(error XXX D_ALL_SUBDIRS = ${D_ALL_SUBDIRS} XXX)

############
# Set F_SRC_EXCLUDES to the basenames of C/C++ files you would like to
# exclude from compilation.  By default, all *.c and *.cpp files will
# be compiled.
#####
#F_SRC_EXCLUDES =

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
LDFLAGS	+= -L$(NEXUS_BIN_DIR)/ -lnexus -lpthread -lm
LDFLAGS	+= $(B_OPENSSL_LDFLAGS)

############################################################################
#
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
SRC_DIR := src

vpath %.c ${SRC_DIR}
vpath %.h ${SRC_DIR}
LIB_BASENAME := bpiff

############
# Directory where to install the headers
#####
D_FOR_INC_INSTALL := include

############
# You probably shouldn't have to modify the text below this point.
#####
F_LIB_NAMES := lib${LIB_BASENAME}.a  lib${LIB_BASENAME}.so
F_PUBLIC_LIBS += $(addprefix ${BPIFF_OBJ_ROOT}/,${F_LIB_NAMES})
D_FOR_LIB_INSTALL := ${BPIFF_OBJ_ROOT}/${BPIFF_LIBDIR}

F_SRCS := $(filter-out ${F_SRC_EXCLUDES}, $(notdir $(wildcard $(addsuffix /*.c, ${SRC_DIR}))))
F_OBJS := $(patsubst %.c,%.o, ${F_SRCS})
END_OBJS := $(addprefix ${BPIFF_OBJ_ROOT}/,${F_OBJS})
END_OBJS += ${UTILS_OBJ_ROOT}/bbase64.o
F_INSTALLED_LIBS := $(addprefix ${D_FOR_LIB_INSTALL}/, ${F_LIB_NAMES})
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##

#$(error END_OBJS = $(END_OBJS))
#$(error CFLAGS = $(CFLAGS))
$(warning F_LIB_NAMES: ${F_LIB_NAMES} )
$(warning F_PUBLIC_LIBS: ${F_PUBLIC_LIBS} )
$(warning F_INSTALLED_LIBS: ${F_INSTALLED_LIBS} )
$(warning D_FOR_LIB_INSTALL: ${D_FOR_LIB_INSTALL} )

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
.PHONY: all
all: prep_openssl ${F_PUBLIC_LIBS}

prep_openssl:
	$(MAKE) -C $(B_OPENSSL_BUILD_DIR)

${F_PUBLIC_LIBS} ${END_OBJS} : | prep_folder

.PHONY : prep_folder
prep_folder:
	$(Q_)mkdir -p ${BPIFF_OBJ_ROOT}/
	$(Q_)mkdir -p ${UTILS_OBJ_ROOT}/

.PHONY : clean
clean:
	$(Q_)echo Cleaning objects... [${END_OBJS}]
	$(Q_)rm -f ${END_OBJS}
	$(Q_)echo Cleaning libraries... [${F_PUBLIC_LIBS} ${F_INSTALLED_LIBS}]
	$(Q_)rm -f ${F_PUBLIC_LIBS} ${F_INSTALLED_LIBS}
	$(Q_)rm -rf ${BPIFF_OBJ_ROOT} ${UTILS_OBJ_ROOT}

distclean: clean

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##



############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
${D_FOR_LIB_INSTALL}/%.so : ${BPIFF_OBJ_ROOT}/%.so
	cp $< $@

${D_FOR_LIB_INSTALL}/%.a : ${BPIFF_OBJ_ROOT}/%.a
	cp $< $@

${BPIFF_OBJ_ROOT}/%.so : ${END_OBJS}
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	${CC} -shared -o $@  ${LDFLAGS}  $^ -Wl,-Bdynamic
else
	$(Q_)echo [Linking shared library .... $@ ]
	$(Q_)${CC} -shared -o $@ ${LDFLAGS} ${END_OBJS} -Wl,-dy
endif

${BPIFF_OBJ_ROOT}/%.a : ${END_OBJS}
	$(Q_)echo [Linking static library .... $@ ]
	$(Q_)${AR} rc $@ ${END_OBJS}


${BPIFF_OBJ_ROOT}/%.o: %.c
	$(Q_)echo [Compiling  with $(CC).... $< ]
	$(Q_)${CC} ${CFLAGS} -c -o $@ $<

${UTILS_OBJ_ROOT}/%.o: ${BSEAV_TOP}/lib/utils/%.c
	$(Q_)echo [Compiling  with $(CC).... $< ]
	$(Q_)${CC} ${CFLAGS} -c -o $@ $<

idirs:
	$(Q_)[ -d ${D_FOR_LIB_INSTALL} ] || mkdir -p ${D_FOR_LIB_INSTALL}
	$(Q_)[ -d ${D_FOR_INC_INSTALL} ] || mkdir -p ${D_FOR_INC_INSTALL}

install: all idirs ${F_INSTALLED_LIBS}
	$(Q_)echo [Copying proper bpiff lib version...]
	$(Q_)install -m 0755 ${D_FOR_LIB_INSTALL}/libbpiff.so $(NEXUS_BIN_DIR)
	$(Q_)if [ -d $(APPLIBS_TOP) ] && [ -d $(APPLIBS_TARGET_LIB_DIR) ]; then \
		echo [Copying libbpiff.so to applibs target]; \
		install -m 0755 ${D_FOR_LIB_INSTALL}/libbpiff.so $(APPLIBS_TARGET_LIB_DIR); \
	fi
