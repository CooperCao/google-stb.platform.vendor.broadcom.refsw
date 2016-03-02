############################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

PHONY=all server install clean nexus clean_nexus clean_apps
.PHONY: $(PHONY)
APPS = nxserver

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)

ifdef COMSPEC
# Any DOS environment
NEXUS_TOP := $(shell cd ../.. && cd)
else
NEXUS_TOP := $(shell cd ../../; pwd)
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/nxclient/server

${PHONY} ${APPS}: ${BINDIR}/exists
	@BINDIR=$(BINDIR) B_REFSW_REAL_MAKE=1 ${MAKE} -C ${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} -f ${CURDIR}/Makefile $@

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

# include cross-compiler definitions
ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make nexus" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
NEXUS_BIN_DIR ?= $(NEXUS_TOP)/../obj.${NEXUS_PLATFORM}/nexus/bin
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

vpath %.c ${NEXUS_TOP}/nxclient/server
vpath %.h ${NEXUS_TOP}/nxclient/server

# namevalue
vpath %.c ${NEXUS_TOP}/utils
CFLAGS += -I${NEXUS_TOP}/utils

all: nxserver

server: nxserver

IPC :=  ${NEXUS_TOP}/lib/ipc
CFLAGS += -I${IPC} -I$(BINDIR)/../build/stubs -I$(NEXUS_TOP)/nxclient/server
EXTRA_OBJS := bipc_util.o bipc_server.o bipc_client.o
vpath %.c ${IPC}

ifeq ($(NEXUS_MODE),)
ifneq ($(NEXUS_SERVER_SUPPORT),n)
# if not proxy, then force usermode multiprocess
NEXUS_BUILD_OPTIONS = NEXUS_MODE=client
endif
endif

unexport BINDIR
clean: clean_nexus clean_apps
	@B_REFSW_REAL_MAKE= $(MAKE) -C $(NEXUS_TOP)/nxclient/build clean

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
	$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build
clean_nexus:
	$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build clean
endif

clean_apps:
	-$(RM) $(APPS) *.d *.o *.a

# This is the minimum needed to compile and link with Nexus
CFLAGS += -g $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)
# temporarily needed for namevalue.c conversion, should refactor
CFLAGS += -I$(NEXUS_TOP)/../BSEAV/lib/media
CFLAGS += -MMD
# Some systems build libnxserver.a into a .so. This could increase code size, but I don't see it.
CFLAGS += -fPIC

-include *.d

nxserver nxserverlib_ipc.o: $(NXCLIENT_SHARED_LIB)

$(NXCLIENT_SHARED_LIB): nexus
	@B_REFSW_REAL_MAKE= $(MAKE) -C $(NEXUS_TOP)/nxclient/build

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_SERVER_LDFLAGS)
else
LDFLAGS += -L${NEXUS_BIN_DIR} -lnexus -lpthread -lm
endif

ifeq ($(B_REFSW_ANDROID),y)
LDFLAGS += -L${ANDROID_OUT_DIR}/target/product/${ANDROID_PRODUCT_OUT}/system/lib -llog
endif

NXSERVER_SRC = \
    nxserver_main.c

# allow override of socket interface
NXCLIENT_SOCKET_INTF ?= $(NEXUS_TOP)/nxclient/server/nxclient_socket.c
vpath %.c $(dir $(NXCLIENT_SOCKET_INTF))

NXSERVERLIB_SRCS = \
    nxserver.c \
    nxserver_cmdline.c \
    nxserverlib.c \
    nxserverlib_ipc.c \
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

NXSERVERLIB = libnxserver.a

%.o: %.c
	@echo [Compile... $@]
	$(Q_)$(CC) -c -o $@ $(filter %.c,$<) $(CFLAGS)

$(NXSERVERLIB): $(patsubst %.c,%.o,$(NXSERVERLIB_SRCS)) $(EXTRA_OBJS)
	@echo [Archive... $@]
	$(Q_)$(AR) rcs $@ $^
	$(CP) $@ ${NEXUS_BIN_DIR}

nxserver: $(patsubst %.c,%.o,$(NXSERVER_SRC)) $(NXSERVERLIB)
	@echo [Link... $@]
	$(Q_)$(CC) -o $@ $(filter %.o %.a,$^) $(CFLAGS) $(LDFLAGS)
	$(CP) $@ ${NEXUS_BIN_DIR}

install: all
	$(CP) $(APPS) ${NEXUS_BIN_DIR}

endif # B_REFSW_REAL_MAKE
