###############################################################################################################
#  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
MAGNUM_TOP ?= ${URSR_TOP}/magnum
SAGE_PUBLIC_TOP ?= ${BSEAV_TOP}/lib/security/sage
SRAI_TOP ?= ${SAGE_PUBLIC_TOP}/srai

# Sage Software path. need to be built first
APP_TOP := ${BSEAV_TOP}/lib/security/sage/bp3
# Add utilities sources
SRCS = sage_app_utils.c
# Add application sources
SRCS += bp3_platform_host.c
SRCS += bp3_module_host.c
SRCS += main.c

ifneq ($(NEXUS_MODE),proxy)
ifeq ($(NEXUS_SAGE_BP3_BIN_PATH),)
NEXUS_SAGE_BP3_BIN_PATH=nexus_sage_bp3_bin_path.c
endif
SRCS += $(NEXUS_SAGE_BP3_BIN_PATH)
endif
vpath %.c $(APP_TOP)/utils:$(APP_TOP)/app:$(NEXUS_TOP)/modules/sage/src/linuxuser

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/${NEXUS_PLATFORM}/build/platform_app.inc
include $(BSEAV_TOP)/opensource/zlib/zlib.inc
#include $(BSEAV_TOP)/opensource/openssl/openssl.inc
include $(BSEAV_TOP)/opensource/curl/curl.inc
include $(BSEAV_TOP)/opensource/cjson/cjson.inc
include $(MAGNUM_TOP)/portinginterface/hsm/bhsm_defs.inc

# Application source file list, include path, C flags, and link
# flags
LIB_SOURCES = bp3_host.c quick_ssdp.c civetweb.c
APP_SOURCES = bp3.c bp3_features.c

APP_CFLAGS = -DWITH_POSIX -Wno-pedantic -Wno-variadic-macros -fPIC -I$(APP_TOP)/app -I${APP_TOP}/utils

# This is the minimum needed to compile and link with Nexus
APP_CFLAGS += ${NEXUS_CFLAGS} $(addprefix -I,${NEXUS_APP_INCLUDE_PATHS})  $(addprefix -D,${NEXUS_APP_DEFINES})
APP_CFLAGS += $(B_OPENSSL_CFLAGS) $(CURL_CFLAGS) -I$(CJSON_LIB_FOLDER) $(CJSON_CFLAGS)
APP_CFLAGS += -I${BSEAV_TOP}/lib/security/sage/bp3/utils/
APP_CFLAGS += -I${BSEAV_TOP}/lib/security/sage/srai/include/
APP_CFLAGS += -I${BSEAV_TOP}/lib/security/sage/bp3/app/
APP_CFLAGS += -I${BSEAV_TOP}/lib/security/sage/platforms/include/
APP_CFLAGS += -DBHSM_ZEUS_VER_MAJOR=$(BHSM_ZEUS_VER_MAJOR) -DBHSM_ZEUS_VER_MINOR=$(BHSM_ZEUS_VER_MINOR)

# allow c++ style comments
APP_CFLAGS += -std=c99

LIB_OBJS := ${$(notdir SRCS):.c=.o} ${LIB_SOURCES:.c=.o}
APP_OBJS := ${APP_SOURCES:.c=.o}

APP_LDFLAGS = -L$(abspath ${B_REFSW_OBJ_ROOT}/nexus/bin) -lsrai $(NEXUS_LDFLAGS) -lm -lbp3_host
APP_LDFLAGS += $(B_OPENSSL_LDFLAGS) $(CURL_LDFLAGS) $(ZLIB_LDFLAGS) $(CJSON_LDFLAGS)

ifeq ($(NXCLIENT_SUPPORT),y)
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
APP_CFLAGS += $(addprefix -I, $(NXCLIENT_INCLUDES)})
APP_CFLAGS += $(NXCLIENT_CFLAGS)
APP_LDFLAGS += $(NXCLIENT_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES)
else
APP_LDFLAGS += $(NEXUS_LD_LIBRARIES)
endif
OUTPUT_DIR = $(B_REFSW_OBJ_ROOT)/BSEAV/tools/bp3/
LIB_OBJECTS = $(addprefix $(OUTPUT_DIR), $(LIB_OBJS))
OBJECTS = $(addprefix $(OUTPUT_DIR), $(APP_OBJS))
APP = $(OUTPUT_DIR)bp3
LIB = $(OUTPUT_DIR)libbp3_host.so
APPCONFIGFILE = bp3.provision.config

all: $(APP)

$(OUTPUT_DIR)%.o: %.c
	@echo [Compile... $(notdir $<)]
	@$(CC) -c -o $@ $< $(APP_CFLAGS)

srai:
	@$(MAKE) -C ${SRAI_TOP} install

# "order only" prerequisite rule so OBJECTS wait for libs to be built (for make -j option)
$(OBJECTS):  | srai folders curl cjson

nexus:
	@echo [Build..... Nexus]
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build
ifeq ($(NXCLIENT_SUPPORT),y)
	${Q_}$(MAKE) -C $(NEXUS_TOP)/nxclient server
endif

clean-nexus:
	@echo [Clean..... Nexus]
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build clean

$(LIB): $(LIB_OBJECTS)
	@echo [Linking... $@]
	@$(CC) -shared -o $@ $(LIB_OBJECTS)
	$(Q_)$(CP) -f $(LIB) ${NEXUS_BIN_DIR}

$(APP): nexus $(OBJECTS) $(LIB)
	@echo [Linking... $(notdir $@)]
	@$(CC) -o $@ $(OBJECTS) $(APP_LDFLAGS)

include $(BSEAV)/opensource/openssl/b_openssl_targets.inc

openssl: b_openssl_target_build

clean-openssl: b_openssl_target_clean_all

curl: openssl
	@echo [Build..... curl]
	@$(MAKE) -j -C ${BSEAV_TOP}/opensource/curl install_to_nexus_bin
	@echo [Installing CURL related libraries in ${NEXUS_BIN_DIR}]
	@if ls ${B_REFSW_OBJ_ROOT}/target/usr/local/lib/libz* > /dev/null 2>&1 ; then install -m 0755 ${B_REFSW_OBJ_ROOT}/target/usr/local/lib/libz* ${NEXUS_BIN_DIR}; fi

cjson:
	@echo [Build..... cjson]
	@$(MAKE) -j -C ${BSEAV_TOP}/opensource/cjson

install: all
	@echo [Installing to ... ${NEXUS_BIN_DIR}]
	$(Q_)$(CP) -f ${APP} ${NEXUS_BIN_DIR}
	$(Q_)$(CP) -f ${APPCONFIGFILE} ${NEXUS_BIN_DIR}

folders:
	@$(MKDIR) $(OUTPUT_DIR)

clean: clean-nexus
	$(MAKE) -C ${SRAI_TOP} clean
	@$(MAKE) -C ${BSEAV_TOP}/opensource/curl clean
	@$(MAKE) -C ${BSEAV_TOP}/opensource/cjson clean
	-${RM} -rf $(OUTPUT_DIR)

distclean:  clean-openssl clean
