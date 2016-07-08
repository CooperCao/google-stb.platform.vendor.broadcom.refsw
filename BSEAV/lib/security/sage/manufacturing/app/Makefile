#*******************************************************************************
#   Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to the terms and
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
#********************************************************************************


######## localisation
ifeq (${URSR_TOP},)
ifneq (${NEXUS_TOP},)
URSR_TOP = ${NEXUS_TOP}/..
else
URSR_TOP = ../../../../../..
endif
endif

ifeq (${NEXUS_PLATFORM}, )
$(error NEXUS_PLATFORM not set, run the plat command)
endif

# Unified Reference Software path
NEXUS_TOP ?= ${URSR_TOP}/nexus
MAGNUM_TOP ?= ${URSR_TOP}/magnum
BSEAV_TOP ?= ${URSR_TOP}/BSEAV
SAGE_PUBLIC_TOP ?= ${BSEAV_TOP}/lib/security/sage
SRAI_TOP  ?= ${SAGE_PUBLIC_TOP}/srai
IMAGE_DIR ?= ${SAGE_PUBLIC_TOP}/manufacturing/image
EXPECTED_PATH ?= $(IMAGE_DIR)/$(IMAGE_NAME)

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/${NEXUS_PLATFORM}/build/platform_app.inc

APP = brcm_manufacturing_tool

ifeq ($(IMAGE_NAME),)
$(error Image file not specified. Usage: make IMAGE_NAME=<file name> <target>)
endif




# Check for image file
IMAGE_PATH := $(shell find $(IMAGE_DIR) -name $(IMAGE_NAME))
ifneq ($(IMAGE_PATH),$(EXPECTED_PATH))
$(error Specified image file name not found in $(IMAGE_DIR). $(IMAGE_PATH), $(IMAGE_NAME))
else
$(warning SAGE Manufacturing file image specified $(IMAGE_PATH))
endif

ifeq (${USE_NXCLIENT},y)
include $(URSR_TOP)/nexus/nxclient/include/nxclient.inc
CFLAGS += -DUSE_NXCLIENT=1
CFLAGS += $(addprefix -I,${NXCLIENT_INCLUDES})
CFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS += $(NXCLIENT_LDFLAGS)
endif

# Add example sources
SRCS += sage_manufacturing_tool.c

OBJS = ${SRCS:.c=.o}

# Host/Sage shared headers
APP_INCLUDES  = ${BSEAV_TOP}/lib/security/sage/manufacturing/include \
		${BSEAV_TOP}/lib/security/sage/manufacturing

#srai lib path first to get latest SRAI lib
LDFLAGS += -L${NEXUS_TOP}/bin/${NEXUS_PLATFORM} -lb_sage_manufacturing
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai -lsrai
LDFLAGS += -L${NEXUS_BIN_DIR} -lnexus -lpthread

#debug symbols
CFLAGS += -g

CFLAGS += $(addprefix -I,${APP_INCLUDES})
CFLAGS += -DPIC -fpic

# This is the minimum needed to compile and link with Nexus
CFLAGS += ${NEXUS_CFLAGS} $(addprefix -I,${NEXUS_APP_INCLUDE_PATHS})  $(addprefix -D,${NEXUS_APP_DEFINES})

#allow c++ style comments
CFLAGS += -std=c99

.PHONY: all clean uninstall install re

all: srai  prep_manufacturing_lib ${APP}

# Implicit rule for building local apps
${APP}: ${OBJS}
	@echo [Link... $@]
	${Q_} ${CC} ${OBJS} -o ${APP} ${LDFLAGS}

%.o: %.c
	@echo [Compile... $<]
	${Q_} ${CC} -c -o $@ $< ${CFLAGS}

clean:
	@echo [Clean... ${APP}]
	${Q_} rm -f ${OBJS}
	${Q_} rm -f ${APP}

srai:
	make -C ${SRAI_TOP} install

prep_manufacturing_lib:
	@echo [Building SAGE manufacturing lib ...]
	make -C ${SAGE_PUBLIC_TOP}/manufacturing/src install

uninstall:
	@echo [Uninstall... ${APP}]
	${Q_} rm -f ${NEXUS_BIN_DIR}/${APP}
	@echo [Uninstall... SRAI]
	${Q_} rm -f ${NEXUS_BIN_DIR}/libsrai.*

install:
	@echo [Installing ${APP}... in ${NEXUS_BIN_DIR}]
	${Q_} install -m 0755 ${APP} ${NEXUS_BIN_DIR}
#	@echo [Installing SRAI... in ${NEXUS_BIN_DIR}]
#	${Q_} install -m 0755 ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai/* ${NEXUS_BIN_DIR}
	@echo [Installing Loadable Manufacturing TA binary ${IMAGE_NAME} in ${NEXUS_BIN_DIR}]
	${Q_} install -m 0755 ${EXPECTED_PATH} ${NEXUS_BIN_DIR}/sage_ta_manufacturing.bin



re: uninstall clean all install
