##############################################################################
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
##############################################################################

PHONY=all clean install nexus_headers
.PHONY: $(PHONY)

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)

NEXUS_TOP := $(shell cd ../../; pwd)

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/nxclient/build

${PHONY} ${APPS}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} BINDIR=${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile $(MAKECMDGOALS)

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make nexus" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
NEXUS_BIN_DIR ?= $(NEXUS_TOP)/../obj.${NEXUS_PLATFORM}/nexus/bin
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

LDFLAGS += $(NEXUS_CLIENT_LD_LIBRARIES)
ifeq ($(B_REFSW_ANDROID),y)
LDFLAGS += -nostdlib \
	-Wl,--gc-sections -Wl,-shared,-Bsymbolic \
	-L$(B_REFSW_ANDROID_LIB) \
	-Wl,--no-whole-archive -lcutils -lc -lm -ldl \
	-Wl,--whole-archive
ifeq ($(B_REFSW_USES_CLANG),y)
ifeq ($(B_REFSW_ARCH), aarch64-linux)
LDFLAGS += \
   ${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
   -target aarch64-linux-android \
   -B${ANDROID_TOP}/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
else
LDFLAGS += \
   ${ANDROID_TOP}/${ANDROID_PREBUILT_LIBGCC} \
   -target arm-linux-androideabi \
   -B${ANDROID_TOP}/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
endif
endif
else
LDFLAGS += -Wl,-Map,$(NEXUS_BIN_DIR)/libnxclient.map
endif

IPC_LIB=$(NEXUS_BIN_DIR)/libnxclient.so
IPC_LIB_STATIC=$(NEXUS_BIN_DIR)/libnxclient_static.a
LOCAL_LIB=$(NEXUS_BIN_DIR)/libnxclient_local.so
Q_?=@

all: $(IPC_LIB) $(IPC_LIB_STATIC) $(LOCAL_LIB)
ifeq ($(NEXUS_HEADERS),y)
all: nexus_headers
endif

B_LINKER=$(CC)

# allow override of socket interface
NXCLIENT_SOCKET_INTF ?= $(NEXUS_TOP)/nxclient/server/nxclient_socket.c
vpath %.c $(dir $(NXCLIENT_SOCKET_INTF))

IPC_SERVER_STUB := $(BINDIR)/stubs/ipc_stubs_server.h
IPC_CLIENT_STUB := $(BINDIR)/stubs/ipc_stubs_client.h
IPC :=  ${NEXUS_TOP}/lib/ipc
CFLAGS += -I${IPC} -I$(BINDIR)/stubs
EXTRA_OBJS := bipc_util.o bipc_server.o bipc_client.o
vpath %.c ${IPC}
vpath %.c ${NEXUS_TOP}/nxclient/server

${IPC_CLIENT_STUB}: ${IPC_SERVER_STUB}

${IPC_SERVER_STUB}: ${NEXUS_TOP}/nxclient/server/nxserver_ipc.h
	${Q_}${MKDIR} -p $(BINDIR)/stubs
	${Q_}${PERL} -I ${NEXUS_TOP}/build/tools/common ${IPC}/build/bipc_build.pl ${IPC_SERVER_STUB} ${IPC_CLIENT_STUB} $^

nxclient_ipc.o: ${IPC_SERVER_STUB} ${IPC_CLIENT_STUB}

# NEXUS flags
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# nxclient
CFLAGS += -I$(NEXUS_TOP)/nxclient/include -I$(NEXUS_TOP)/nxclient/server
CFLAGS += -fPIC

nxclient_local.o nxclient_ipc.o: $(NEXUS_TOP)/nxclient/include/nxclient.h $(NEXUS_TOP)/nxclient/include/nxclient_config.h

# use special file to trigger relink of libnxclient.so (and then all client apps) if there's a switch between user and kernel modes
ifeq ($(NEXUS_MODE),proxy)
NXCLIENT_MODE_CHECK := $(B_REFSW_OBJ_ROOT)/nexus/nxclient/proxymode
$(NXCLIENT_MODE_CHECK):
	@$(RM) $(B_REFSW_OBJ_ROOT)/nexus/nxclient/clientmode
	@touch $@
else
NXCLIENT_MODE_CHECK := $(B_REFSW_OBJ_ROOT)/nexus/nxclient/clientmode
$(NXCLIENT_MODE_CHECK):
	@$(RM) $(B_REFSW_OBJ_ROOT)/nexus/nxclient/proxymode
	@touch $@
endif

IPC_LIB_OBJS := nxclient_ipc.o nxclient_common.o $(patsubst %.c,%.o,$(notdir $(NXCLIENT_SOCKET_INTF))) bipc_util.o bipc_client.o $(NXCLIENT_MODE_CHECK)

$(IPC_LIB): $(IPC_LIB_OBJS)
	${Q_}echo "[Linking $(notdir $@)]"
	${Q_}$(B_LINKER) ${LDFLAGS} -shared -Wl,--cref -Wl,-x -o $@ -Wl,-soname,libnxclient.so $(filter %.o,$^)

$(IPC_LIB_STATIC): $(IPC_LIB_OBJS)
	${Q_}echo "[Archive... $(notdir $@)]"
	$(Q_)$(AR) rcs $@ $^

$(LOCAL_LIB): nxclient_local.o nxclient_common.o
	${Q_}echo "[Linking $(notdir $@)]"
	${Q_}$(B_LINKER) ${LDFLAGS} -shared -Wl,--cref -Wl,-x -o $@ -Wl,-soname,libnxclient_local.so $^

%.o: %.c
	${Q_}$(CC) -c -o $@ $(filter %.c %.s %.o, $<) ${CFLAGS}

clean:
	$(RM) -r $(IPC_LIB) $(IPC_LIB_STATIC) $(LOCAL_LIB) $(NEXUS_BIN_DIR)/libnxclient.map *.o stubs

install: all

nexus_headers:
	${Q_}echo Copying header files to ${NEXUS_BIN_DIR}/include
	${Q_}$(MKDIR) ${NEXUS_BIN_DIR}/include
	${Q_}${CP} $(NEXUS_TOP)/nxclient/include/* ${NEXUS_BIN_DIR}/include

endif # B_REFSW_REAL_MAKE
