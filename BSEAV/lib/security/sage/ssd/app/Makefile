#############################################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#
# Module Description:
#
#############################################################################


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

# Sage Software path
SAGE_TOP ?= ${URSR_TOP}/sage


include ${MAGNUM_TOP}/syslib/sagelib/bsagelib_public.inc
include ${BSEAV_TOP}/lib/security/sage/ssd/app/ssdd.inc

ifeq ($(B_REFSW_OBJ_ROOT),)
SSDD_OBJDIR:= obj
else
SSDD_OBJDIR:=${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/ssd
endif
export SSDD_OBJDIR

APPNAME = ssdd
APP = ${SSDD_OBJDIR}/${APPNAME}
INSTALL_APP = ${NEXUS_BIN_DIR}/${APPNAME}

# nxclient build
ifeq (${NXCLIENT_SUPPORT},y)
include $(URSR_TOP)/nexus/nxclient/include/nxclient.inc
CFLAGS += -DNXCLIENT_SUPPORT=1
CFLAGS += $(addprefix -I,${NXCLIENT_INCLUDES})
CFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS += $(NXCLIENT_LDFLAGS)
endif

F_SRCS := $(foreach ssddsrc, $(shell ls ${BSEAV_TOP}/lib/security/sage/ssd/app/*.c), $(notdir ${ssddsrc}))
F_SRCS += $(foreach ssdsrc, $(shell ls ${BSEAV_TOP}/lib/security/sage/ssd/src/*.c), $(notdir ${ssdsrc}))
vpath %.c ${BSEAV_TOP}/lib/security/sage/ssd/app ${BSEAV_TOP}/lib/security/sage/ssd/src
vpath %.h ${BSEAV_TOP}/lib/security/sage/ssd/include

F_OBJS := $(patsubst %.c,%.o, ${F_SRCS})
END_OBJS := $(addprefix ${SSDD_OBJDIR}/,${F_OBJS})

APP_INCLUDES  = ${BSAGELIB_INCLUDES} \
                ${SRAI_TOP}/include \
                ${BSEAV_TOP}/lib/security/common_drm/include/tl \
                ${SSDD_TOP}/include \
                ${SAGE_PUBLIC_TOP}/include \
                ${SAGE_PUBLIC_TOP}/platforms/include

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/${NEXUS_PLATFORM}/build/platform_app.inc

#srai lib path first to get latest SRAI lib
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai -lsrai
LDFLAGS += -L${NEXUS_BIN_DIR} -lnexus
LDFLAGS +=  -lpthread -lm

#debug symbols
CFLAGS += -g

CFLAGS += $(addprefix -I,${APP_INCLUDES})
CFLAGS += $(addprefix -I,${BSAGELIB_INCLUDES})
CFLAGS += $(addprefix -D,${BSAGELIB_DEFINES})
CFLAGS += -DPIC -fpic

# This is the minimum needed to compile and link with Nexus
CFLAGS += ${NEXUS_CFLAGS} $(addprefix -I,${NEXUS_APP_INCLUDE_PATHS})  $(addprefix -D,${NEXUS_APP_DEFINES})

#allow c++ style comments
CFLAGS += -std=c99

.PHONY: all clean install re srai distclean

all: prep_folder install

prep_folder:
	@mkdir -p ${SSDD_OBJDIR}/

install: ${INSTALL_APP}

${INSTALL_APP} : ${APP}
	@echo "[Installing $(notdir $<)... in $(dir ${INSTALL_APP})]"
	${Q_} install -m 0755 $< $(dir ${INSTALL_APP})

srai:
	@make -C ${SRAI_TOP} install

# Implicit rule for building local apps
${APP}: ${END_OBJS} | srai
	@echo "[Link... $(notdir $@)]"
	${Q_} ${CC} ${END_OBJS} -o $@ ${LDFLAGS}

${SSDD_OBJDIR}/%.o: %.c
	@echo "[Compile... $<]"
	${Q_} ${CC} ${CFLAGS} -c $< -o $@

clean:
	@echo "[Clean... $(notdir ${APP})]"
	${Q_} rm -f ${END_OBJS}
	${Q_} rm -f ${APP}
	@echo "[Uninstall... $(notdir ${APP})]"
	${Q_} rm -f ${NEXUS_BIN_DIR}/${APPNAME}

distclean: clean
	make -C ${SRAI_TOP} clean

re:
	@ make --no-print-directory distclean
	@ make --no-print-directory install
