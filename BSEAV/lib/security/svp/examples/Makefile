############################################################
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
#
############################################################
NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(URSR_TOP),)
URSR_TOP = $(shell pwd)/../../../../../
$(warning URSR_TOP is not defined. Set it as default: $(URSR_TOP))
endif

NEXUS_TOP=$(URSR_TOP)/nexus
BSEAV_TOP=$(URSR_TOP)/BSEAV

# Redirect logs to
LOG_REDIRECT_FILE ?= /dev/null

ifeq ($(B_REFSW_DEBUG),y)
BUILD_TYPE ?= debug
FLAGS += -g
else
ifeq ($(B_REFSW_DEBUG),)
BUILD_TYPE ?= debug
FLAGS += -g
else
BUILD_TYPE ?= release
BUILD_MODE ?= release
FLAGS += -O2
endif
endif

NXCLIENT_APPS = heap_toggle

STANDALONE_APPS = crrurr_glrglr_decode

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(URSR_TOP)/nexus/nxclient/include/nxclient.inc

# variables for out of source builds
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= $(abspath ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR})

OUT_ROOT_DIR := ${B_REFSW_OBJ_ROOT}/BSEAV/lib/security/svp/examples

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

#Where does this come from
NEXUS_APP_INCLUDE_PATHS += $(NEXUS_TOP)/../BSEAV/lib/bfile

CFLAGS += -DPIC -fpic
CFLAGS += -Wl,--exclude-libs,ALL
CFLAGS += -Wno-variadic-macros -Wno-sign-compare -Wno-long-long

CFLAGS += -Wall -fexceptions -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS += -D_FILE_OFFSET_BITS=64 -D_REENTRANT -fPIC -DPIC
CFLAGS += -D_GNU_SOURCE=1
CFLAGS += $(FLAGS) $(NEXUS_CFLAGS)  $(addprefix -D,$(NEXUS_APP_DEFINES))
CFLAGS := $(filter-out -std=c89 -Wstrict-prototypes, $(CFLAGS))

NEXUS_STANDALONE_LDFLAGS =  -L$(NEXUS_BIN_DIR) -lnexus -lpthread

DEFINES := $(addprefix -D, $(DEFINES))
INCLUDES += $(addprefix -I, ${NEXUS_APP_INCLUDE_PATHS})

PHONY=clean all report
.PHONY: clean all report $(NXCLIENT_APPS) $(STANDALONE_APPS)

.SUFFIXES: .cpp .c

all: $(NXCLIENT_APPS) $(STANDALONE_APPS)

${OUT_ROOT_DIR}:
	@mkdir -p $@/nxclient
	@mkdir -p $@/standalone

$(OUT_ROOT_DIR)/nxclient/%.o: %.c ${OUT_ROOT_DIR}
	@echo [Compile: $<]
	$(Q_)${CC} -c $(FLAGS) $(CFLAGS) $(NXCLIENT_CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

$(OUT_ROOT_DIR)/standalone/%.o: %.c ${OUT_ROOT_DIR}
	@echo [Compile: $<]
	$(Q_)${CC} -c $(FLAGS) $(CFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

$(NXCLIENT_APPS): %: ${OUT_ROOT_DIR}/nxclient/%.o
	@echo [Link: $@]
	$(Q_)$(CC) -o ${OUT_ROOT_DIR}/nxclient/$@ $(filter %.c %.s %.o %.a, $^) $(NXCLIENT_LDFLAGS)
	@install -m 0755 $(OUT_ROOT_DIR)/nxclient/$@ $(NEXUS_BIN_DIR)

$(STANDALONE_APPS): %: ${OUT_ROOT_DIR}/standalone/%.o
	@echo [Link: $@]
	$(Q_)$(CC) -o ${OUT_ROOT_DIR}/standalone/$@ $(filter %.c %.s %.o %.a, $^) $(NEXUS_STANDALONE_LDFLAGS)
	@install -m 0755 $(OUT_ROOT_DIR)/standalone/$@ $(NEXUS_BIN_DIR)

report:
	@echo "Reporting ..."
	@echo NEXUS_BIN_DIR = "$(NEXUS_BIN_DIR)"
	@echo NEXUS_TOP = "$(NEXUS_TOP)"
	@echo
	@echo INCLUDES = "$(INCLUDES)"
	@echo
	@echo CFLAGS = "$(CFLAGS)"
	@echo
	@echo NEXUS_CFLAGS = "$(NEXUS_CFLAGS)"
	@echo
	@echo NXCLIENT_CFLAGS = "$(NXCLIENT_CFLAGS)"
	@echo
	@echo NXCLIENT_LDFLAGS = "$(NXCLIENT_LDFLAGS)"
	@echo
	@echo NEXUS_STANDALONE_LDFLAGS = "$(NEXUS_STANDALONE_LDFLAGS)"
	@echo
	@echo nexus bin dir="$(NEXUS_BIN_DIR)"
	@echo OUT_ROOT_DIR = "$(OUT_ROOT_DIR)"
	@echo CC = "${CC}"
	@echo CXX = "${CXX}"
	@echo
	@echo NXCLIENT_APPS = "${NXCLIENT_APPS}"
	@echo STANDALONE_APPS = "${STANDALONE_APPS}"
	@echo ...

clean:
	@echo "Cleaning ..."
	@rm -rf $(OUT_ROOT_DIR)
