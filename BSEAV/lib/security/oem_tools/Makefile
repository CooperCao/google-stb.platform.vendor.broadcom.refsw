############################################################
#  Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

ifeq ($(URSR_TOP),)
$(warning URSR_TOP is not defined. Establishing relative paths to nexus and BSEAV)
NEXUS_TOP := $(shell cd ../../../../nexus; pwd)
BSEAV_TOP := $(shell cd ../../../../BSEAV; pwd)
else
NEXUS_TOP := $(URSR_TOP)/nexus
BSEAV_TOP := $(URSR_TOP)/BSEAV
endif

$(warning target = $(target))

ifeq ($(target), )
$(error Makefile usage:  make target=<directory> (i.e. make target=msp))
else
include ./$(target)/$(target)_app.inc
TARGET_DIR := BSEAV/lib/security/oem_tools
endif

APPS += $(SECURITY_APPS)
SRCS = $(SECURITY_APP_SRCS)
OBJS = ${SRCS:.c=.o}

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

# create 'target' dir in out of source dir
$(shell mkdir -p ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target})

ifeq ($(NXCLIENT_SUPPORT),y)
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
endif

TARGET_OBJS = $(addprefix  ${target}/, ${OBJS})
INSTALLED_APPS = $(addprefix  ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/, ${APPS})
CLEAN_APPS = $(addprefix  $(NEXUS_BIN_DIR)/, ${APPS})

all:  $(APPS)
	@$(MAKE) copy

# For linux builds, link to the correct libraries
LDFLAGS := -L$(NEXUS_BIN_DIR)/ -lpthread -lm -lnexus$(NEXUS_LIB_SUFFIX)

CFLAGS += $(SECURITY_APP_CFLAGS)
LDFLAGS += $(SECURITY_APP_LDFLAGS)
INCLUDES += $(SECURITY_APP_INCLUDES)

# This cleans nexus and local apps
clean:
	rm -rf ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}
	rm -f ${CLEAN_APPS}

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Always build with debug
CFLAGS += -g

# Implicit rule for building local apps
$(APPS): do_prep
	@$(MAKE) $(TARGET_OBJS) && \
	echo [Link... $@] && \
	$(CC) -o ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/$@ ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/${target}/$@.o $(LDFLAGS)

do_prep:
	@$(MAKE) prep_nexus

prep_nexus:
ifeq ($(NXCLIENT_SUPPORT),y)
	@if [ ! -e ${NEXUS_BIN_DIR}/nxserver ]; then \
		echo [ building nxclient .... ] && \
		$(MAKE) -C $(NEXUS_TOP)/nxclient >/dev/null; \
	fi
else
	@if [ ! -e ${NEXUS_BIN_DIR}/libnexus.so ]; then \
		echo [ building nexus .... ] && \
		$(MAKE) -C $(NEXUS_TOP)/build >/dev/null; \
	fi
endif

${target}/%.o: ${target}/%.c
	@echo [Compiling  with $(CC).... $< ] && \
	${CC} ${CFLAGS} -c -o  ${B_REFSW_OBJ_ROOT}/${TARGET_DIR}/$@ $<

copy:
	@echo [Copying executables...]
	cp -f $(INSTALLED_APPS) $(NEXUS_BIN_DIR)
