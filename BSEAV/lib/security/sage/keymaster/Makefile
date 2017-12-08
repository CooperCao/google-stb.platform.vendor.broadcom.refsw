#############################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#############################################################################

NEXUS_PLATFORM ?= $(PLATFORM)
NEXUS_TOP := $(shell cd ../../../../../nexus; pwd)
MAGNUM_TOP := $(shell cd ../../../../../magnum; pwd)
BSEAV_TOP := $(shell cd ../../../../../BSEAV; pwd)
SAGE_PUBLIC_TOP  := $(BSEAV_TOP)/lib/security/sage

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
KEYMASTER_OBJ_ROOT = ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/keymaster

# Include nexus definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc

# Sanity check that we received a valid platform
ifndef BCHP_CHIP
$(error Unsupported platform $(NEXUS_PLATFORM))
endif

LIB_BASENAME        := sage_keymaster

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))


# add SAGElib related includes
include ${MAGNUM_TOP}/syslib/sagelib/bsagelib_public.inc
include ${MAGNUM_TOP}/syslib/sagelib/bsagelib_tools.inc
APP_INCLUDES += ${BSAGELIB_INCLUDES} ${BSAGELIB_TOOLS_INCLUDES}

# Custom include paths
CFLAGS += -I./include \
          -I$(SAGE_PUBLIC_TOP)/include \
          -I$(SAGE_PUBLIC_TOP)/srai/include \
          -I$(SAGE_PUBLIC_TOP)/platforms/include \
          -I$(BSEAV_TOP)/lib/security/common_drm/include/tl

CFLAGS += $(addprefix -I,${BSAGELIB_INCLUDES})
CFLAGS += $(addprefix -D,${BSAGELIB_DEFINES})

CFLAGS += -DPIC -fpic -g
CFLAGS += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

#allow c++ style comments
CFLAGS += -std=c99
ifeq ($(ANDROID_BUILD),y)
CFLAGS += -EL -DANDROID
endif

F_SRCS := $(foreach keymastersrc, $(shell ls ${BSEAV_TOP}/lib/security/sage/keymaster/src/*.c), $(notdir ${keymastersrc}))
vpath %.c ${BSEAV_TOP}/lib/security/sage/keymaster/src
vpath %.h ${BSEAV_TOP}/lib/security/sage/keymaster/src ${BSEAV_TOP}/lib/security/sage/srai/include

ifeq ($(NEXUS_MODE),proxy)
vpath %.c $(foreach SDIR, $(BSAGELIB_TOOLS_SOURCES), $(dir $(SDIR)))
vpath %.h $(foreach SDIR, $(BSAGELIB_TOOLS_SOURCES), $(dir $(SDIR)))
F_SRCS += $(foreach depSRC, ${BSAGELIB_TOOLS_SOURCES}, $(notdir ${depSRC}))
endif

F_LIBNAMES += lib${LIB_BASENAME}.a lib${LIB_BASENAME}.so
ifneq ($(NEXUS_BIN_DIR),)
D_FOR_LIB_INSTALL := $(NEXUS_BIN_DIR)
else
$(error define NEXUS_BIN_DIR for install dir)
endif
F_OBJS := $(patsubst %.c,%.o, ${F_SRCS})
END_OBJS := $(addprefix ${KEYMASTER_OBJ_ROOT}/,${F_OBJS})

F_INSTALLED_LIBS := $(addprefix ${D_FOR_LIB_INSTALL}/, ${F_LIBNAMES})
F_PUBLIC_LIBS := $(addprefix ${KEYMASTER_OBJ_ROOT}/, ${F_LIBNAMES})

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
all: prep_folder ${F_PUBLIC_LIBS}

prep_folder:
	@ mkdir -p ${KEYMASTER_OBJ_ROOT}/

clean:
	@ echo clean objects [${F_OBJS}]
	@ rm -f ${END_OBJS}
	@ echo clean libraries [${F_LIBNAMES}]
	@ rm -f ${F_INSTALLED_LIBS} ${F_PUBLIC_LIBS}

re: clean install

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##


############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
$(filter %.so, ${F_PUBLIC_LIBS}): ${END_OBJS}
	@ echo [Linking shared library .... $(notdir $@) ]
	${CC} -shared -o $@ ${LDFLAGS} $^ -Wl,-dy
	${STRIP_COMMAND}

$(filter %.a, ${F_PUBLIC_LIBS}): ${END_OBJS}
	@ echo [Linking static library .... $(notdir $@) ]
	${AR} rc $@ $^
	${STRIP_COMMAND}

${D_FOR_LIB_INSTALL}:
	mkdir -p ${D_FOR_LIB_INSTALL}

install: all ${F_INSTALLED_LIBS}

${D_FOR_LIB_INSTALL}/%.a : ${KEYMASTER_OBJ_ROOT}/%.a | ${D_FOR_LIB_INSTALL}
	install -m 0755 $< $@

${D_FOR_LIB_INSTALL}/%.so : ${KEYMASTER_OBJ_ROOT}/%.so | ${D_FOR_LIB_INSTALL}
	install -m 0755 $< $@

${KEYMASTER_OBJ_ROOT}/%.o: %.c
	@ echo [Compiling .... $(notdir $<) ]
	@ ${CC} ${CFLAGS} -c -o $@ $<
