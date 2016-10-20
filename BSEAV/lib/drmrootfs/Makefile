###############################################################################################
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
###############################################################################################
############################################################################
#
# DESCRIPTION:
#   Makefile for building drmrootfs library.
#
############################################################################

MAGNUM_TOP := $(shell cd ../../../magnum; pwd)
NEXUS_TOP := $(shell cd ../../../nexus; pwd)

PUBLIC_LIBS    += libdrmrootfs.a libdrmrootfs.so

#include drmrootfs_libdir.inc to see which directory the lib will be installed in $(DRMROOTFS_LIBDIR)
include drmrootfs_libdir.inc

# Include nexus definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc

# Variables and directories for out-of-source build
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= $(abspath ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR})
DRMROOTFS_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs
D_FOR_LIB_INSTALL = ${DRMROOTFS_OBJ_ROOT}/${DRMROOTFS_LIBDIR}

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

# Custom include paths
CFLAGS += -g -fshort-wchar -fpic
CFLAGS += -DBSTD_CPU_ENDIAN=${NEXUS_ENDIAN}
CFLAGS += $(addprefix -I, $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))
CFLAGS += $(addprefix -D, $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
CFLAGS += $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

TOOLCHAIN_DIR=$(TOOLCHAIN)

ifeq ($(ANDROID_BUILD),y)
ARCH ?= $(B_REFSW_ARCH)
endif

ifeq ($(ARCH),mipsel-linux)
TOOLCHAIN_DIR=/opt/toolchains/mipsel
else
#
# Discover the uclibc toolchain directory assuming the compiler exists in bin subdir
# Use which and dirname bash shell commands.
#
TOOLCHAIN_DIR=$(shell dirname $(shell dirname $(shell which mipsel-linux-uclibc-gcc)))
endif

# Must export MSDRM_PRDY_SUPPORT to build the lib with playready support.
CFLAGS += -DB_HAS_MSDRM_PRDY=1

############################################################################
#                              
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##

F_PUBLIC_LIBS           += $(addprefix ${DRMROOTFS_OBJ_ROOT}/,${PUBLIC_LIBS})

SRCS	+= $(wildcard *.c )
OBJS 	+= $(patsubst %.c,%.o, ${SRCS})
END_OBJS   := $(addprefix ${DRMROOTFS_OBJ_ROOT}/,${OBJS})
F_INSTALLED_LIBS := $(addprefix ${D_FOR_LIB_INSTALL}/, ${PUBLIC_LIBS})

PUBLIC_INCS    += drm_data.h

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
install: all ${F_INSTALLED_LIBS} 

all: prep_folder ${F_PUBLIC_LIBS}

clean: 
	@${RM} -fr ${F_INSTALLED_LIBS} ${DRMROOTFS_OBJ_ROOT}

############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
prep_folder:
	@ mkdir -p ${DRMROOTFS_OBJ_ROOT}
	@ mkdir -p ${D_FOR_LIB_INSTALL}

${DRMROOTFS_OBJ_ROOT}/libdrmrootfs.so: ${END_OBJS} |prep_folder
	@echo "[Link...... $(notdir $@) (drmrootfs)]"
ifeq ($(findstring $(B_REFSW_ARCH), arm-linux aarch64-linux), $(B_REFSW_ARCH))
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	@${CC} -shared -o $@  ${LDFLAGS}  $^ -Wl,-Bdynamic
else # B_REFSW_CROSS_COMPILE is not arm-linux-androideabi-
	@${CC} -shared -o $@  ${LDFLAGS}  $^ -Wl,-dy,--no-wchar-size-warning
endif # End B_REFSW_CROSS_COMPILE
else # B_REFSW_ARCH is not arm-linux or aarch64-linux
	@${CC} -shared -o $@  ${LDFLAGS}  $^ -Wl,-dy
endif # End B_REFSW_ARCH

${DRMROOTFS_OBJ_ROOT}/libdrmrootfs.a: ${END_OBJS} |prep_folder
	@echo "[Archive... $(notdir $@) (drmrootfs)]"
	@${AR} rc $@  $^

${DRMROOTFS_OBJ_ROOT}/%.o: %.c |prep_folder
	@echo "[Compile... $< (drmrootfs)]"
	${Q_}$(CC) -c -o $@ $< $(CFLAGS)

${D_FOR_LIB_INSTALL}/%.a : ${DRMROOTFS_OBJ_ROOT}/%.a |prep_folder
	@install -m 0755 $< $@

${D_FOR_LIB_INSTALL}/%.so : ${DRMROOTFS_OBJ_ROOT}/%.so |prep_folder
	@install -m 0755 $< $@

############################################################################
#                             DEP FILES, PHONY
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
-include *.d
.PHONY: all install clean distclean prep_folder
#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##
