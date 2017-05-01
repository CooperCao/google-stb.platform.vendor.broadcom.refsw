###############################################################################
 # Broadcom Proprietary and Confidential. (c) 2017 Broadcom. All rights reserved.
 #
 # This program is the proprietary software of Broadcom and/or its
 # licensors, and may only be used, duplicated, modified or distributed pursuant
 # to the terms and conditions of a separate, written license agreement executed
 # between you and Broadcom (an "Authorized License").  Except as set forth in
 # an Authorized License, Broadcom grants no license (express or implied), right
 # to use, or waiver of any kind with respect to the Software, and Broadcom
 # expressly reserves all rights in and to the Software and all intellectual
 # property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 # HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 # NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 #
 # Except as expressly set forth in the Authorized License,
 #
 # 1. This program, including its structure, sequence and organization,
 #    constitutes the valuable trade secrets of Broadcom, and you shall use all
 #    reasonable efforts to protect the confidentiality thereof, and to use
 #    this information only in connection with your use of Broadcom integrated
 #    circuit products.
 #
 # 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 #    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 #    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 #    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 #    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 #    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 #    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 #    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 #
 # 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 #    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 #    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 #    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 #    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 #    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 #    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 #    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 #
 #############################################################################/
APP = dynrng

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifneq ($(DYNRNG_PLATFORM),)
$(error Dynrng only supports nxclient for now)
else
DYNRNG_PLATFORM:=nxclient
endif

B_THIS_DIR:=rockford/applications/dynrng/build
ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

ifeq ($(B_REFSW_REAL_MAKE),)

.PHONY:	app clean_app

all install:
	$(MAKE) -C . ${APP}

all: platform_all
install: platform_install

clean: clean_app

app:
	$(MAKE) -C . ${APP}

clean_app:
	-$(RM) -r $(BINDIR)

ifeq ($(DYNRNG_PLATFORM),nxclient)
.PHONY:	server clean_server

SERVER := ${NEXUS_TOP}/nxclient/server

platform_all:
	$(MAKE) -C ${SERVER} all

platform_install:
	$(MAKE) -C ${SERVER} install

clean: clean_server

server:
	$(MAKE) -C ${SERVER}

clean_server:
	$(MAKE) -C ${SERVER} clean
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
ifeq ($(NEXUS_MODE),)
NEXUS_MODE:=$(MODE)
endif
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/nxclient/$(B_REFSW_ARCH)/dynrng.$(NEXUS_MODE)

${APP}: ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} BINDIR=${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile

${BINDIR}/exists:
	@mkdir -p $@

else # B_REFSW_REAL_MAKE

APP_DEPENDENCIES = \
	platform.c \
	platform_display.c \
	platform_graphics.c \
	platform_input.c \
	platform_media_player.c \
	platform_picture.c \
	platform_plm.c \
	platform_receiver.c \
	platform_scheduler.c \
	app.c \
	app_shell.c \
	args.c \
	bwt.c \
	file_switcher.c \
    image_viewer.c \
	osd.c \
	plm.c \
	pwl.c \
	scenario_player.c \
	shell.c \
	stream_player.c \
	util.c \
	nl2l.c

APP_OBJS = $(APP_DEPENDENCIES:c=o)

install: all

all: $(EXTRA_BUILD_RULES) $(APP) copy_files

ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make nexus" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
NEXUS_BIN_DIR ?= $(NEXUS_TOP)/../obj.${NEXUS_PLATFORM}/nexus/bin
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

# NEXUS flags
CFLAGS += -g $(NEXUS_CFLAGS) ${NEXUS_INTERNAL_CFLAGS} $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

CFLAGS += -MMD

-include *.d

# dynrng app includes and source
CFLAGS += -I$(NEXUS_TOP)/../rockford/applications/dynrng/include
vpath %.c $(NEXUS_TOP)/../rockford/applications/dynrng/src

ifeq ($(DYNRNG_QUIT_SUPPORT),y)
CFLAGS += -DDYNRNG_QUIT_SUPPORT=1
endif

# nxclient version
ifeq ($(DYNRNG_PLATFORM),nxclient)

NXCLIENT_P_SHARED_LIB := $(BINDIR)/libnxclient.so

include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_LDFLAGS) $(NXCLIENT_LDFLAGS)
else
LDFLAGS += $(NXCLIENT_LDFLAGS)
endif
LDFLAGS += -g

$(NXCLIENT_P_SHARED_LIB):
	${Q_}$(MAKE) -C $(NEXUS_TOP)/nxclient/server B_REFSW_REAL_MAKE=

B_LIB_TOP = $(NEXUS_TOP)/lib

#
# playback_ip additions
#
ifeq ($(PLAYBACK_IP_SUPPORT),y)
.PHONY: playback_ip clean_playback_ip
EXTRA_BUILD_RULES += playback_ip
EXTRA_CLEAN_RULES += clean_playback_ip
include $(B_LIB_TOP)/playback_ip/b_playback_ip_lib.inc
CFLAGS += $(PLAYBACK_IP_CFLAGS)
LDFLAGS += $(PLAYBACK_IP_LDFLAGS)
# Add BIP
include $(NEXUS_TOP)/lib/bip/bip_lib.inc
CFLAGS += $(BIP_ALL_LIB_CFLAGS)
LDFLAGS += $(BIP_ALL_LIB_LDFLAGS)

playback_ip:
	${Q_}$(MAKE) REFSW_BUILD=y -C $(NEXUS_TOP)/lib/bip/build B_NEXUS_API_BUILD_COMPLETED=y install
	$(Q_)$(MAKE) -C $(B_LIB_TOP)/playback_ip/apps B_NEXUS_API_BUILD_COMPLETED=y install
clean_playback_ip:
	${Q_}$(MAKE) REFSW_BUILD=y -C $(NEXUS_TOP)/lib/bip/build clean
	$(Q_)$(MAKE) -C $(B_LIB_TOP)/playback_ip/apps clean
endif

#
# tr69c additions
#
ifeq ($(B_REFSW_TR69C_SUPPORT),y)
EXTRA_BUILD_RULES += tr69c
EXTRA_CLEAN_RULES += clean_tr69c
CFLAGS += -I$(NEXUS_TOP)/../BSEAV/app/tr69c/lib -DB_REFSW_TR69C_SUPPORT=1
LDFLAGS += -ltr69c
tr69c: $(NEXUS_BIN_DIR)/libtr69c.so
$(NEXUS_BIN_DIR)/libtr69c.so:
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/app/tr69c libtr69c.so
clean_tr69c:
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/app/tr69c clean
endif

ifeq ($(NXCLIENT_LOCAL),y)
# Don't link with -lnxclient.
# Instead, client will link with nxclient_local.c which will init the server with default params.
# Server will still provide IPC for other clients.
LDFLAGS = $(NEXUS_LD_LIBRARIES) -lnxserver
endif

# for faster compilation, archive nexus/nxclient/apps/utils
NXCLIENT_APPS_UTILS_LIB := $(BINDIR)/nxclient_apps_utils.a
include $(NEXUS_TOP)/nxclient/apps/utils/nxutils.inc
NXCLIENT_APPS_UTILS_SRC := $(foreach src,$(NXUTILS_SOURCES), $(notdir $(src)))
NXCLIENT_APPS_UTILS_OBJS := $(addprefix $(BINDIR)/,$(patsubst %.c,%.o,$(NXCLIENT_APPS_UTILS_SRC)))
CFLAGS += $(addprefix -I,$(NXUTILS_INCLUDES)) -I$(BINDIR)
vpath %.c $(foreach src,$(NXUTILS_SOURCES), $(dir $(src)))

ifeq ($(FREETYPE_SUPPORT),y)
FREETYPE_DIR = $(NEXUS_TOP)/../BSEAV/opensource/freetype
include $(FREETYPE_DIR)/b_freetype_targets.inc
EXTRA_BUILD_RULES += freetype
EXTRA_CLEAN_RULES += clean_freetype
CFLAGS += -DFREETYPE_SUPPORT -I$(FREETYPE_SOURCE_PATH)/include
LDFLAGS += -L$(FREETYPE_LIB_FOLDER) -lfreetype
freetype: b_freetype_target_build
clean_freetype: b_freetype_target_clean_all
endif

$(NXCLIENT_APPS_UTILS_OBJS): $(EXTRA_BUILD_RULES)

$(BINDIR)/$(APP): $(NXCLIENT_P_SHARED_LIB) $(EXTRA_BUILD_RULES)

DYNRNG_PLATFORM_LIBS += $(NXCLIENT_APPS_UTILS_LIB)
vpath %.c $(NEXUS_TOP)/../rockford/applications/dynrng/src/nxclient

$(NXCLIENT_APPS_UTILS_LIB): $(NXCLIENT_APPS_UTILS_OBJS)
	@echo [Archive... $@]
	$(Q_)$(AR) rcs $@ $(filter %.o, $^)

copy_platform_files:
	$(MKDIR) ${NEXUS_BIN_DIR}/nxclient
	$(CP) $(NEXUS_TOP)/nxclient/apps/resources/* ${NEXUS_BIN_DIR}/nxclient

clean_platform:
	${Q_}$(MAKE) -C $(NEXUS_TOP)/nxclient/server clean

endif # nxclient version

# rule for nxclient utils and such
$(BINDIR)/%.o: %.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) -c -o $@ $< $(CFLAGS)

# rule for app objects
%.o: %.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) -c -o $@ $< $(CFLAGS)

$(BINDIR)/$(APP): %: $(APP).c $(APP_OBJS) $(DYNRNG_PLATFORM_LIBS)
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) -o $@ $(filter %.c %.s %.o %.a, $^) $(CFLAGS) $(LDFLAGS)
	$(CP) $@ ${NEXUS_BIN_DIR}

$(APP): $(BINDIR)/$(APP)
	@test 1

clean: clean_app $(EXTRA_CLEAN_RULES)

clean_app: clean_platform

copy_files: copy_platform_files
	$(MKDIR) ${NEXUS_BIN_DIR}/../etc/dynrng
	$(MKDIR) ${NEXUS_BIN_DIR}/../share/dynrng/images/sdr
	$(MKDIR) ${NEXUS_BIN_DIR}/../share/dynrng/backgrounds/sdr
	$(MKDIR) ${NEXUS_BIN_DIR}/../share/dynrng/streams/sdr
	$(MKDIR) ${NEXUS_BIN_DIR}/../share/dynrng/streams/hdr
	$(MKDIR) ${NEXUS_BIN_DIR}/../share/dynrng/streams/hlg
	$(MKDIR) ${NEXUS_BIN_DIR}/../share/dynrng/streams/mix
	$(MKDIR) ${NEXUS_BIN_DIR}/../share/dynrng/streams/dvs
	$(CP) -ar $(NEXUS_TOP)/../rockford/applications/dynrng/etc/* ${NEXUS_BIN_DIR}/../etc/dynrng

endif # B_REFSW_REAL_MAKE
