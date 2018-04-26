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
NEXUS_PLATFORM ?= $(PLATFORM)
NEXUS_TOP := $(shell cd ../../../../../nexus; pwd)
MAGNUM_TOP := $(shell cd ../../../../../magnum; pwd)
BSEAV_TOP := $(shell cd ../../../../../BSEAV; pwd)
SAGE_TOP  := $(BSEAV_TOP)/lib/security/sage

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
SRAI_OBJ_ROOT = ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai

# Include nexus definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
ifeq ($(NXCLIENT_SUPPORT),y)
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
endif

# Sanity check that we received a valid platform
ifndef BCHP_CHIP
$(error Unsupported platform $(NEXUS_PLATFORM))
endif

LIB_BASENAME        := srai

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

include ${MAGNUM_TOP}/portinginterface/hsm/bhsm_defs.inc
CFLAGS += -DBHSM_ZEUS_VER_MAJOR=$(BHSM_ZEUS_VER_MAJOR) -DBHSM_ZEUS_VER_MINOR=$(BHSM_ZEUS_VER_MINOR)

# Custom include paths
CFLAGS += -I./include -I$(SAGE_TOP)/include/private
CFLAGS += $(addprefix -I,${BSAGELIB_INCLUDES})
CFLAGS += $(addprefix -D,${BSAGELIB_DEFINES})

CFLAGS += -DPIC -fpic
CFLAGS += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

ifeq ($(NXCLIENT_SUPPORT),y)
CFLAGS += $(NXCLIENT_CFLAGS)
endif

#allow c++ style comments
CFLAGS += -std=c99
ifeq ($(ANDROID_BUILD),y)
CFLAGS += -DANDROID
endif

ifeq ($(ANDROID_BUILD),y)

LDFLAGS += --sysroot=$(ANDROID)/out/target/product/bcm_platform/system
endif

F_SRCS := $(foreach sraisrc, $(shell ls ${BSEAV_TOP}/lib/security/sage/srai/src/*.c), $(notdir ${sraisrc}))
vpath %.c ${BSEAV_TOP}/lib/security/sage/srai/src
vpath %.h ${BSEAV_TOP}/lib/security/sage/srai/src ${BSEAV_TOP}/lib/security/sage/srai/include

ifeq ($(findstring $(NEXUS_MODE),proxy client), $(NEXUS_MODE))
vpath %.c $(foreach SDIR, $(BSAGELIB_TOOLS_SOURCES), $(dir $(SDIR)))
vpath %.h $(foreach SDIR, $(BSAGELIB_TOOLS_SOURCES), $(dir $(SDIR)))
F_SRCS += $(foreach depSRC, ${BSAGELIB_TOOLS_SOURCES}, $(notdir ${depSRC}))
endif

F_PUBLIC_LIBS += $(addprefix $(SRAI_OBJ_ROOT)/, lib${LIB_BASENAME}.a lib${LIB_BASENAME}.so)
ifneq ($(NEXUS_BIN_DIR),)
D_FOR_LIB_INSTALL := $(NEXUS_BIN_DIR)
else
$(error define NEXUS_BIN_DIR for install dir)
endif
F_OBJS := $(patsubst %.c,%.o, ${F_SRCS})
END_OBJS := $(addprefix ${SRAI_OBJ_ROOT}/,${F_OBJS})

F_INSTALLED_LIBS := $(addprefix ${D_FOR_LIB_INSTALL}/, $(notdir ${F_PUBLIC_LIBS}))

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
.PHONY: reinstall all prep_folder clean re install

all: prep_folder install

prep_folder:
	@ mkdir -p ${SRAI_OBJ_ROOT}/

clean:
	@rm -fr ${SRAI_OBJ_ROOT}
	@rm -fr ${F_INSTALLED_LIBS}

re: clean reinstall

reinstall: install |clean

#^^^^#####################^^^^^^^^^^^^^^^^^^^^^^^#####################^^^^##

#$(error END_OBJS $(END_OBJS))

############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
${SRAI_OBJ_ROOT}/lib${LIB_BASENAME}.so: ${END_OBJS} |prep_folder
	@ echo "[Link...... $(notdir $@) (srai)]"
ifeq ($(B_REFSW_CROSS_COMPILE),arm-linux-androideabi-)
	@ ${CC} -shared -o $@ ${LDFLAGS} ${END_OBJS} -Wl,-Bdynamic
else
	@ ${CC} -shared -o $@ ${LDFLAGS} ${END_OBJS} -Wl,-dy
endif
	${STRIP_COMMAND}

${SRAI_OBJ_ROOT}/lib${LIB_BASENAME}.a: ${END_OBJS} |prep_folder
	@ echo "[Archive... $(notdir $@) (srai)]"
	@ ${AR} rc $@ ${END_OBJS}
	${STRIP_COMMAND}

${D_FOR_LIB_INSTALL}:
	@ mkdir -p ${D_FOR_LIB_INSTALL}

install: $(F_INSTALLED_LIBS) | ${D_FOR_LIB_INSTALL}

${D_FOR_LIB_INSTALL}/%.a : ${SRAI_OBJ_ROOT}/%.a | ${D_FOR_LIB_INSTALL}
	@echo "[Installing $(notdir $@)]"
	@install -m 0755 $< $@

${D_FOR_LIB_INSTALL}/%.so : ${SRAI_OBJ_ROOT}/%.so | ${D_FOR_LIB_INSTALL}
	@echo "[Installing $(notdir $@)]"
	@install -m 0755 $< $@

${SRAI_OBJ_ROOT}/%.o: %.c |prep_folder
	@ echo "[Compile... $(notdir $<) (srai)]"
	@ ${CC} ${CFLAGS} -c -o $@ $<
