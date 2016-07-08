#############################################################################
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
#############################################################################

NEXUS_PLATFORM ?= $(PLATFORM)
NEXUS_TOP := $(shell cd ../../../../nexus; pwd)
MAGNUM_TOP := $(shell cd ../../../../magnum; pwd)
BSEAV_TOP := $(shell cd ../../../../BSEAV; pwd)


B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
PIFF_OBJ_ROOT = ${B_REFSW_OBJ_ROOT}/BSEAV/lib/playready/piff

# Include nexus definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc

# Sanity check that we received a valid platform
ifndef BCHP_CHIP
$(error Unsupported platform $(NEXUS_PLATFORM))
endif

LIB_BASENAME        := prdypiff

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

# Custom include paths
CFLAGS += -I./include
CFLAGS += -DPIC -fpic
CFLAGS += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

include $(BSEAV_TOP)/lib/utils/batom.inc
include $(BSEAV_TOP)/lib/media/bmedia.inc
include $(BSEAV_TOP)/lib/bfile/bfile.inc

CFLAGS += $(addprefix -I,$(BFILE_INCLUDES))
CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES))
CFLAGS += $(addprefix -I,$(BATOM_INCLUDES))

#allow c++ style comments
CFLAGS += -std=c99
ifeq ($(ANDROID_BUILD),y)
CFLAGS += -DANDROID
endif

ifeq ($(ANDROID_BUILD),y)

LDFLAGS += --sysroot=$(ANDROID)/out/target/product/bcm_platform/system
endif

F_SRCS := $(foreach piffsrc, $(shell ls ${BSEAV_TOP}/lib/playready/piff/src/*.c), $(notdir ${piffsrc}))
vpath %.c ${BSEAV_TOP}/lib/playready/piff/src
vpath %.h ${BSEAV_TOP}/lib/playready/piff/src ${BSEAV_TOP}/lib/playready/piff/include


F_PUBLIC_LIBS += lib${LIB_BASENAME}.a lib${LIB_BASENAME}.so
ifneq ($(NEXUS_BIN_DIR),)
D_FOR_LIB_INSTALL := $(NEXUS_BIN_DIR)
else
$(error defien NEXUS_BIN_DIR for install dir)
endif
F_OBJS := $(patsubst %.c,%.o, ${F_SRCS})
END_OBJS := $(addprefix ${PIFF_OBJ_ROOT}/,${F_OBJS})

F_INSTALLED_LIBS := $(addprefix ${D_FOR_LIB_INSTALL}/, ${F_PUBLIC_LIBS})

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
prep_folder:
	@ mkdir -p ${PIFF_OBJ_ROOT}/
all: prep_folder ${F_PUBLIC_LIBS}

clean:
	@ echo clean objects [${F_OBJS}]
	@ rm -f ${F_OBJS}
	@ echo clean libraries [${F_PUBLIC_LIBS}]
	@ rm -f ${F_INSTALLED_LIBS} ${PIFF_OBJ_ROOT}/lib${LIB_BASENAME}.so ${PIFF_OBJ_ROOT}/lib${LIB_BASENAME}.a

re: clean install

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##


############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
lib${LIB_BASENAME}.so: ${F_OBJS}
	@ echo [Linking shared library .... ${PIFF_OBJ_ROOT}/$$@ ]
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	@ ${CC} -shared -o ${PIFF_OBJ_ROOT}/$@ ${LDFLAGS} ${END_OBJS} -Wl,-Bdynamic
else
	@ ${CC} -shared -o ${PIFF_OBJ_ROOT}/$@ ${LDFLAGS} ${END_OBJS} -Wl,-dy
endif
	${STRIP_COMMAND}

lib${LIB_BASENAME}.a: ${F_OBJS}
	@ echo [Linking static library .... ${PIFF_OBJ_ROOT}/$@ ]
	@ ${AR} rc ${PIFF_OBJ_ROOT}/$@ ${END_OBJS}
	${STRIP_COMMAND}

idirs:
	@ [ -d ${D_FOR_LIB_INSTALL} ] || mkdir -p ${D_FOR_LIB_INSTALL}

install: all idirs ${F_INSTALLED_LIBS}

${D_FOR_LIB_INSTALL}/%.a : %.a
	install -m 0755 ${PIFF_OBJ_ROOT}/$< $@

${D_FOR_LIB_INSTALL}/%.so : %.so
	install -m 0755 ${PIFF_OBJ_ROOT}/$< $@

%.o: %.c
	@ echo [Compiling .... $< ]
	@ ${CC} ${CFLAGS} -c -o ${PIFF_OBJ_ROOT}/$@ $<
