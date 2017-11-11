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

# Required:
# export BP3_PROVISIONING=y

# Directories relative to location of makefile
ifeq (${URSR_TOP},)
ifneq (${NEXUS_TOP},)
URSR_TOP = ${NEXUS_TOP}/..
else
URSR_TOP = ../../..
endif
endif

# Unified Reference Software path
NEXUS_TOP ?= ${URSR_TOP}/nexus
BSEAV_TOP ?= ${URSR_TOP}/BSEAV

# Sage Software path. need to be built first
APP_TOP := ${BSEAV_TOP}/lib/security/sage/bp3
# Add utilities sources
SRCS = $(APP_TOP)/utils/sage_app_utils.c
# Add application sources
SRCS += $(APP_TOP)/app/bp3_platform_host.c
SRCS += $(APP_TOP)/app/bp3_module_host.c
SRCS += $(APP_TOP)/app/main.c

ifneq ($(NEXUS_MODE),proxy)
ifeq ($(NEXUS_SAGE_BP3_BIN_PATH),)
NEXUS_SAGE_BP3_BIN_PATH=$(NEXUS_TOP)/modules/sage/src/linuxuser/nexus_sage_bp3_bin_path.c
endif
SRCS += $(NEXUS_SAGE_BP3_BIN_PATH)
endif

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/${NEXUS_PLATFORM}/build/platform_app.inc
include $(BSEAV_TOP)/opensource/zlib/zlib.inc
include $(BSEAV_TOP)/opensource/openssl/openssl.inc
include $(BSEAV_TOP)/opensource/curl/curl.inc

# Name of application
APP = bp3

# Application source file list, include path, C flags, and link
# flags
APP_SOURCES = ./main.c

APP_CFLAGS = -DWITH_POSIX -Wno-pedantic -Wno-variadic-macros -fPIC -I$(APP_TOP)/app -I${APP_TOP}/utils

# This is the minimum needed to compile and link with Nexus
APP_CFLAGS += ${NEXUS_CFLAGS} $(addprefix -I,${NEXUS_APP_INCLUDE_PATHS})  $(addprefix -D,${NEXUS_APP_DEFINES})
APP_CFLAGS += $(B_OPENSSL_CFLAGS) $(CURL_CFLAGS)
#allow c++ style comments
APP_CFLAGS += -std=c99 -g

APP_OBJS := ${SRCS:.c=.o} ${APP_SOURCES:.c=.o}

APP_LDFLAGS = -L$(abspath ${B_REFSW_OBJ_ROOT}/nexus/bin) -lsrai $(NEXUS_LDFLAGS) -lm
APP_LDFLAGS += $(B_OPENSSL_LDFLAGS) $(CURL_LDFLAGS) $(ZLIB_LDFLAGS)

ifeq ($(NXCLIENT_SUPPORT),y)
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
APP_CFLAGS += $(addprefix -I, $(NXCLIENT_INCLUDES)})
APP_CFLAGS += $(NXCLIENT_CFLAGS)
APP_LDFLAGS += $(NXCLIENT_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES)
else
APP_LDFLAGS += $(NEXUS_LD_LIBRARIES)
endif

all: curl sage_bp3 $(APP)

sage_bp3:
	make -C $(APP_TOP) install

curl:
	make -j -C ${BSEAV_TOP}/opensource/openssl
	make -j -C ${BSEAV_TOP}/opensource/curl install_to_nexus_bin
	@echo [Installing CURL related libraries in ${NEXUS_BIN_DIR}]
	@if ls ${B_REFSW_OBJ_ROOT}/target/usr/local/lib/libz* > /dev/null 2>&1 ; then install -m 0755 ${B_REFSW_OBJ_ROOT}/target/usr/local/lib/libz* ${NEXUS_BIN_DIR}; fi
	${Q_} install -m 0755 ${B_REFSW_OBJ_ROOT}/target/usr/local/lib/libcrypto* ${NEXUS_BIN_DIR}
	${Q_} install -m 0755 ${B_REFSW_OBJ_ROOT}/target/usr/local/lib/libssl* ${NEXUS_BIN_DIR}

$(APP): $(APP_OBJS)
	@echo [Linking... $@]
	@$(CC) -o $@ $(APP_OBJS) $(APP_LDFLAGS)

%.o: %.c
	@echo [Compile... $<]
	@$(CC) -c -o $@ $< $(APP_CFLAGS)

install: all
	@echo [Installing to ... ${NEXUS_BIN_DIR}]
	$(Q_)$(CP) -f ${APP} ${NEXUS_BIN_DIR}

clean:
	make clean -C $(APP_TOP)
	make clean -C ${BSEAV_TOP}/opensource/openssl
	make clean -C ${BSEAV_TOP}/opensource/curl
	-${RM} -f ${APP_OBJS} ${APP}
