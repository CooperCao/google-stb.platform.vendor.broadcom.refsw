#############################################################################
#    (c)2014 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#
#############################################################################


######## localisation
ifeq (${URSR_TOP},)
ifneq (${NEXUS_TOP},)
URSR_TOP = ${NEXUS_TOP}/..
else
URSR_TOP = ../../../../../../
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
UTILITY_TOP ?= ${SAGE_PUBLIC_TOP}/utility


include ${MAGNUM_TOP}/syslib/sagelib/bsagelib_public.inc

SRCS += configure_keyslots.c

ifeq ($(PVR_APP),recorder)
SRCS += recpump_with_wrap.c
else ifeq ($(PVR_APP),player)
SRCS += playpump_scatter_gather.c
else
$(error PVR_APP is not set or invalid, export PVR_APP=recorder or PVR_APP=player)
endif

APP = brcm_pvr_$(PVR_APP)

OBJS = ${SRCS:.c=.o}

# Host/Sage shared headers
APP_INCLUDES  = ${SRAI_TOP}/include \
                ${BSEAV_TOP}/lib/security/sage/utility/include


# add SAGElib related includes
APP_INCLUDES += ${BSAGELIB_INCLUDES}

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/${NEXUS_PLATFORM}/build/platform_app.inc

#srai lib path first to get latest SRAI lib
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai -lsrai
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/utility -lutility
LDFLAGS += -L${NEXUS_BIN_DIR} -lnexus
LDFLAGS +=  -lpthread -lm

#debug symbols
CFLAGS += -g

CFLAGS += $(addprefix -I,${APP_INCLUDES})
CFLAGS += $(addprefix -I,${BSAGELIB_INCLUDES})
CFLAGS += -DPIC -fpic

# This is the minimum needed to compile and link with Nexus
CFLAGS += ${NEXUS_CFLAGS} $(addprefix -I,${NEXUS_APP_INCLUDE_PATHS})  $(addprefix -D,${NEXUS_APP_DEFINES})

#allow c++ style comments
CFLAGS += -std=c99

.PHONY: all clean uninstall install re

all: srai utility ${APP}

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

utility:
	make -C ${UTILITY_TOP} install

uninstall:
	@echo [Uninstall... ${APP}]
	${Q_} rm -f ${NEXUS_BIN_DIR}/${APP}
	@echo [Uninstall... Utility]
	${Q_} rm -f ${NEXUS_BIN_DIR}/libutility.so
	@echo [Uninstall... SRAI]
	${Q_} rm -f ${NEXUS_BIN_DIR}/libsrai.so

install:
	@echo [Installing ${APP}... in ${NEXUS_BIN_DIR}]
	${Q_} install -m 0755 ${APP} ${NEXUS_BIN_DIR}
	@echo [Installing SRAI... in ${NEXUS_BIN_DIR}]
	${Q_} install -m 0755 ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai/libsrai.so ${NEXUS_BIN_DIR}
	@echo [Installing Utility... in ${NEXUS_BIN_DIR}]
	${Q_} install -m 0755 ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/utility/libutility.so ${NEXUS_BIN_DIR}

re: srai utility uninstall clean all install
