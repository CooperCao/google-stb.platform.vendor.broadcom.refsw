############################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
############################################################

.PHONY: all server install clean nexus clean_nexus clean_apps nexus_headers nxserver

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

NEXUS_TOP := $(shell cd ../../;pwd)

# include cross-compiler definitions
ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make nexus" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
NEXUS_BIN_DIR ?= $(NEXUS_TOP)/../obj.${NEXUS_PLATFORM}/nexus/bin
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

BINDIR := $(B_REFSW_OBJ_ROOT)/nexus/nxclient/$(B_REFSW_ARCH)/server

# namevalue
vpath %.c ${NEXUS_TOP}/utils
CFLAGS += -I${NEXUS_TOP}/utils

all: nxserver
ifeq ($(NEXUS_HEADERS),y)
all: nexus_headers
endif

server: nxserver

IPC :=  ${NEXUS_TOP}/lib/ipc
CFLAGS += -I${IPC} -I$(BINDIR)/stubs -I$(NEXUS_TOP)/nxclient/server
EXTRA_OBJS := $(BINDIR)/bipc_util.o $(BINDIR)/bipc_server.o $(BINDIR)/bipc_client.o
vpath %.c ${IPC}

ifeq ($(NEXUS_MODE),)
ifneq ($(NEXUS_SERVER_SUPPORT),n)
# if not proxy, then force usermode multiprocess
NEXUS_BUILD_OPTIONS = NEXUS_MODE=client
endif
endif

clean: clean_nexus clean_apps
	${Q_}if [ -d ${B_REFSW_OBJ_ROOT} ]; then find ${B_REFSW_OBJ_ROOT} -type d -empty -delete; fi

ifeq ($(NEXUS_PREBUILT_BINARY),y)
SKIP_NEXUS := y
endif
ifeq ($(B_NEXUS_API_BUILD_COMPLETED),y)
SKIP_NEXUS := y
endif
ifeq ($(SKIP_NEXUS),y)
nexus:
clean_nexus:
else
nexus:
	${Q_}$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build
clean_nexus:
	${Q_}$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build clean
endif

# This is the minimum needed to compile and link with Nexus
CFLAGS += -g $(NEXUS_CFLAGS) ${NEXUS_INTERNAL_CFLAGS} $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)
# temporarily needed for namevalue.c conversion, should refactor
CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/media
CFLAGS += -MMD
# Some systems build libnxserver.a into a .so. This could increase code size, but I don't see it.
CFLAGS += -fPIC

NXCLIENT_P_SHARED_LIB := $(BINDIR)/libnxclient.so
NXCLIENT_P_LOCAL_SHARED_LIB := $(BINDIR)/libnxclient_local.so

$(BINDIR)/nxserver $(BINDIR)/nxserverlib_ipc.o $(BINDIR)/nxserverlib_ipc_verify.o: $(NXCLIENT_P_SHARED_LIB)

$(NXCLIENT_P_SHARED_LIB): nexus $(BINDIR)/nxclient_local.o $(BINDIR)/nxclient_common.o
	${Q_}BINDIR=$(BINDIR) NEXUS_TOP=$(NEXUS_TOP) $(MAKE) -f Makefile.lib

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_SERVER_LDFLAGS)
else
LDFLAGS += $(NEXUS_LD_LIBRARIES) -lm
endif

ifeq ($(B_REFSW_ANDROID),y)
LDFLAGS += -L${ANDROID_OUT_DIR}/target/product/${LOCAL_PRODUCT_OUT}/system/lib -llog
endif

NXSERVER_SRC = \
    nxserver_main.c

# allow override of socket interface
NXCLIENT_SOCKET_INTF ?= $(NEXUS_TOP)/nxclient/server/nxclient_socket.c
vpath %.c $(dir $(NXCLIENT_SOCKET_INTF))

NXSERVERLIB_SRCS := \
    nxserver.c \
    nxserver_cmdline.c \
    nxserverlib.c \
    nxserverlib_ipc.c \
    nxserverlib_ipc_verify.c \
    nxserverlib_audio.c \
    nxserverlib_encoder.c \
    nxserverlib_video.c \
    nxserverlib_transport.c \
    nxserverlib_input.c \
    nxserverlib_evdev.c \
    nxclient_local.c \
    nxclient_common.c \
    $(notdir $(NXCLIENT_SOCKET_INTF)) \
    namevalue.c

ifneq ($(NXSERVER_CUSTOM_HOOK),)
CFLAGS += -DNXSERVER_CUSTOM_HOOK=1
NXSERVERLIB_SRCS += $(NXSERVER_CUSTOM_HOOK)
endif

ifeq ($(B_REFSW_ARCH), aarch64-linux)
CFLAGS += -DB_IPC_COMPAT_SUPPORT=1
NXSERVERLIB_SRCS += nxserverlib_compat.c
endif

NXSERVERLIB = $(BINDIR)/libnxserver.a
NXSERVERLIB_OBJS := $(addprefix $(BINDIR)/,$(patsubst %.c,%.o,$(NXSERVERLIB_SRCS)))
NXSERVER_OBJS := $(addprefix $(BINDIR)/,$(patsubst %.c,%.o,$(NXSERVER_SRC)))

clean_apps:
	-$(RM) -r $(BINDIR)
	${Q_}BINDIR=$(BINDIR) NEXUS_TOP=$(NEXUS_TOP) $(MAKE) -f Makefile.lib clean

-include $(BINDIR)/*.d

$(BINDIR)/%.o: %.c
	@echo [Compile... $(notdir $@)]
	@mkdir -p $(dir $@)
	$(Q_)$(CC) -c -o $@ $(filter %.c,$<) $(CFLAGS)

$(NXSERVERLIB): $(NXSERVERLIB_OBJS) $(EXTRA_OBJS)
	@echo [Archive... $(notdir $@)]
	@mkdir -p $(dir $@)
	$(Q_)$(AR) rcs $@ $^
	$(CP) $@ ${NEXUS_BIN_DIR}

$(BINDIR)/nxserver: $(NXSERVER_OBJS) $(NXSERVERLIB)
	@echo [Link... $@]
	@mkdir -p $(dir $@)
	$(Q_)$(CC) -o $@ $(filter %.o %.a,$^) $(CFLAGS) $(LDFLAGS)
	$(CP) $@ ${NEXUS_BIN_DIR}

nxserver: $(BINDIR)/nxserver

install: all

all:
	@echo [Install... nxclient to $(realpath ${NEXUS_BIN_DIR})]
	${Q_}$(CP) $(BINDIR)/nxserver $(NXCLIENT_P_SHARED_LIB) $(NXCLIENT_P_LOCAL_SHARED_LIB) ${NEXUS_BIN_DIR}

nexus_headers:
	${Q_}echo Copying header files to ${NEXUS_BIN_DIR}/include
	${Q_}$(MKDIR) ${NEXUS_BIN_DIR}/include
	${Q_}${CP} $(NEXUS_TOP)/nxclient/include/* ${NEXUS_BIN_DIR}/include
