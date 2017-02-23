###############################################################################################################
#  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
###############################################################################################################

ifeq (${URSR_TOP},)
ifneq (${NEXUS_TOP},)
URSR_TOP = ${NEXUS_TOP}/..
else
URSR_TOP = ../../../..
endif
endif

# Unified Reference Software path
NEXUS_TOP ?= ${URSR_TOP}/nexus
MAGNUM_TOP ?= ${URSR_TOP}/magnum
# openssl looks for BSEAV instead of BSEAV_TOP
BSEAV ?= ${URSR_TOP}/BSEAV
SAGE_PUBLIC_TOP ?= ${BSEAV}/lib/security/sage
SRAI_TOP  ?= ${SAGE_PUBLIC_TOP}/srai

# Sage Software path
SAGE_TOP ?= ${URSR_TOP}/sage

APP = test_mediaroom

# Add utilities sources
SRCS  = utils/drm_mediaroom_tl.c
# Add application sources
SRCS += app/test_mediaroom.c

OBJS = ${SRCS:.c=.o}

# Host/Sage shared headers
APP_INCLUDES  = ${SRAI_TOP}/include
# Host specific headers
APP_INCLUDES += ./app ./utils
# host includes
APP_INCLUDES += $(BSEAV)/lib/security/common_drm/include \
                $(BSEAV)/lib/security/common_drm/include/tl \
                $(BSEAV)/lib/security/common_crypto/include \
                $(BSEAV)/lib/security/bcrypt/include \
                $(BSEAV)/lib/drmrootfs \
                $(BSEAV)/lib/security/sage/platforms/include \
                $(B_REFSW_OBJ_ROOT)/target/usr/local/include

# add SAGElib related includes
include ${MAGNUM_TOP}/syslib/sagelib/bsagelib_public.inc
APP_INCLUDES += ${BSAGELIB_INCLUDES}

# include bcrypt and drmrootfs
include $(BSEAV)/opensource/openssl/b_openssl_targets.inc
include $(BSEAV)/lib/security/bcrypt/bcrypt_libdir.inc
include $(BSEAV)/lib/drmrootfs/drmrootfs_libdir.inc

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/${NEXUS_PLATFORM}/build/platform_app.inc

# nxclient
ifeq ($(NXCLIENT_SUPPORT),y)
include $(URSR_TOP)/nexus/nxclient/include/nxclient.inc
endif

# bcrypt, drmrootfs lib
LDFLAGS += -lbcrypt -L$(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/bcrypt/${BCRYPT_LIBDIR} \
		   -ldrmrootfs -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR}
#srai lib path first to get latest SRAI lib
LDFLAGS += -L${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai -lsrai
LDFLAGS += -L${NEXUS_BIN_DIR} -lnexus -lssl -lcrypto
LDFLAGS +=  -lpthread -lm

#debug symbols
CFLAGS += -g

CFLAGS += $(addprefix -I,${APP_INCLUDES})

CFLAGS += -DPIC -fpic

# This is the minimum needed to compile and link with Nexus
CFLAGS += ${NEXUS_CFLAGS} $(addprefix -I,${NEXUS_APP_INCLUDE_PATHS})  $(addprefix -D,${NEXUS_APP_DEFINES})

#allow c++ style comments
CFLAGS += -std=c99

.PHONY: all clean uninstall install re

all: ${APP}

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

drmrootfs:
	echo [compiling drmrootfs ...];
	$(Q)$(MAKE) -C $(BSEAV)/lib/drmrootfs install > /dev/null
	${CP} -pv ${B_REFSW_OBJ_ROOT}/BSEAV/lib/drmrootfs/${DRMROOTFS_LIBDIR}/libdrmrootfs.so ${NEXUS_BIN_DIR}

bcrypt:
	@echo [Compiling bcrypt ...] && \
	$(MAKE) -C $(BSEAV)/lib/security/bcrypt install > /dev/null
	${CP} -pv ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/bcrypt/libbcrypt.so ${NEXUS_BIN_DIR}

srai:
	echo [compiling srai ...];
	make -C ${SRAI_TOP} install

uninstall:
	@echo [Uninstall... ${APP}]
	${Q_} rm -f ${NEXUS_BIN_DIR}/${APP}
	@echo [Uninstall... SRAI]
	${Q_} rm -f ${NEXUS_BIN_DIR}/libsrai.so

install:
	@echo [Installing ${APP}... in ${NEXUS_BIN_DIR}]
	${Q_} install -m 0755 ${APP} ${NEXUS_BIN_DIR}
	@echo [Installing SRAI... in ${NEXUS_BIN_DIR}]
	${Q_} install -m 0755 ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/sage/srai/libsrai.so ${NEXUS_BIN_DIR}

re: uninstall clean drmrootfs bcrypt srai all install
