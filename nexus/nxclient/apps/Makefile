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

# no extra source needed (other than GUI_SOURCE)
BASIC_BUILD_APPS = \
	3dtv_client \
	alpha_hole \
	analog_input \
	animation_client \
	audio_capture \
	audio_crc \
	audio_decode_playback \
	audio_fade \
	audio_fade_toggle \
	audio_karaoke \
	blit_client \
	cap \
	config \
	crc \
	cursor \
	desktop \
	dual_transcode \
	encode_display \
	encode_graphics_as_video \
	graphics_as_video \
	hdmi_input \
	heaps \
	insert_ir_input \
	live \
	mosaic_video_as_graphics \
	osd \
	picviewer \
	play \
	playmosaic \
	playpcm \
	playpump_sg_client \
	record \
	screenshot \
	sd_video_override \
	setaudio \
	setdisplay \
	sethdcp \
	simpleplay \
	thumbnail \
	video_as_graphics \
	wait_for_server

# extra source needed
CUSTOM_BUILD_APPS = \
	ccgfx \
	getstatus \
	list_all_boxmodes \
	playviewer \
	standby \
	transcode

# not built by default
MANUAL_APPS = switched_decode

APPS = $(BASIC_BUILD_APPS) $(CUSTOM_BUILD_APPS)

PHONY=all clean clean_apps copy_files clean_playback_ip tr69c clean_tr69c dcclib clean_dcclib
.PHONY: $(PHONY) server clean_server

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(B_REFSW_REAL_MAKE),)

NEXUS_TOP := $(shell cd ../../; pwd)

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
ifeq ($(NEXUS_MODE),)
NEXUS_MODE:=$(MODE)
endif
BINDIR := ${B_REFSW_OBJ_ROOT}/nexus/nxclient/apps.$(NEXUS_MODE)

${PHONY} ${APPS} $(MANUAL_APPS): ${BINDIR}/exists
	@${MAKE} -C ${BINDIR} BINDIR=${BINDIR} B_REFSW_REAL_NEXUS_TOP=${NEXUS_TOP} B_REFSW_REAL_MAKE=1 -f ${CURDIR}/Makefile $@

${BINDIR}/exists:
	@mkdir -p $@

server:
	$(MAKE) -C $(NEXUS_TOP)/nxclient/server

clean_server:
	$(MAKE) -C $(NEXUS_TOP)/nxclient/server clean

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

install: all

all: $(APPS) copy_files

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

# NXCLIENT flags
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_LDFLAGS) $(NXCLIENT_LDFLAGS)
else
LDFLAGS += $(NXCLIENT_LDFLAGS)
endif

vpath %.c $(NEXUS_TOP)/nxclient/apps

$(APPS) $(MANUAL_APPS): $(NXCLIENT_SHARED_LIB)

$(NXCLIENT_SHARED_LIB):
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build B_REFSW_REAL_MAKE=

B_LIB_TOP = $(NEXUS_TOP)/lib

#
# playback_ip additions
#
ifeq ($(PLAYBACK_IP_SUPPORT),y)
EXTRA_BUILD_RULES += $(NEXUS_BIN_DIR)/libb_playback_ip.so
EXTRA_CLEAN_RULES += clean_playback_ip
include $(B_LIB_TOP)/playback_ip/b_playback_ip_lib.inc
CFLAGS += $(PLAYBACK_IP_CFLAGS)
LDFLAGS += $(PLAYBACK_IP_LDFLAGS)
# Add BIP
include $(NEXUS_TOP)/lib/bip/bip_lib.inc
CFLAGS += $(BIP_ALL_LIB_CFLAGS)
LDFLAGS += $(BIP_ALL_LIB_LDFLAGS)

$(NEXUS_BIN_DIR)/libb_playback_ip.so:
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

$(APPS): $(EXTRA_BUILD_RULES)

# for faster compilation, archive nexus/nxclient/apps/utils
NXCLIENT_APPS_UTILS_LIB := nxclient_apps_utils.a
include $(NEXUS_TOP)/nxclient/apps/utils/nxutils.inc
NXCLIENT_APPS_UTILS_SRC := $(foreach src,$(NXUTILS_SOURCES), $(notdir $(src)))
CFLAGS += $(addprefix -I,$(NXUTILS_INCLUDES)) -I$(BINDIR)
vpath %.c $(foreach src,$(NXUTILS_SOURCES), $(dir $(src)))

ifeq ($(FREETYPE_SUPPORT),y)
EXTRA_BUILD_RULES += freetype
EXTRA_CLEAN_RULES += clean_freetype
FREETYPE_DIR = $(NEXUS_TOP)/../BSEAV/lib/freetype-2.1.5
CFLAGS += -DFREETYPE_SUPPORT -I$(FREETYPE_DIR)/include
FREETYPE_ODIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/freetype-2.1.5
LDFLAGS += -L$(FREETYPE_ODIR) -lfreetype
freetype:
	$(MAKE) -C $(FREETYPE_DIR)
clean_freetype:
	$(MAKE) -C $(FREETYPE_DIR) clean
endif

$(NXCLIENT_APPS_UTILS_LIB): $(EXTRA_BUILD_RULES)
$(NXCLIENT_APPS_UTILS_LIB): $(patsubst %.c,%.o,$(NXCLIENT_APPS_UTILS_SRC))
	@echo [Archive... $@]
	$(Q_)$(AR) rcs $@ $(filter %.o, $^)

%.o: %.c
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS)

$(BASIC_BUILD_APPS): %: %.c $(NXCLIENT_APPS_UTILS_LIB)
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o %.a, $^) $(CFLAGS) $(LDFLAGS)
	$(CP) $@ ${NEXUS_BIN_DIR}

# CUSTOM_BUILD_APPS
ifeq ($(NEXUS_POWER_MANAGEMENT),y)
PMLIB_TOP=$(NEXUS_TOP)/../BSEAV/lib/pmlib
include $(PMLIB_TOP)/pmlib.inc
$(PMLIB_LIBDIR)/pmlib.o: pmlib
pmlib:
	$(Q_)$(MAKE) -C $(PMLIB_TOP)
clean_pmlib:
	$(Q_)$(MAKE) -C $(PMLIB_TOP) clean
standby: CFLAGS += $(PMLIB_CFLAGS)
STANDBY_DEPENDENCIES = $(PMLIB_LIBDIR)/pmlib.o
EXTRA_CLEAN_RULES += clean_pmlib
endif
STANDBY_DEPENDENCIES += $(NXCLIENT_APPS_UTILS_LIB)
standby: %: %.c $(STANDBY_DEPENDENCIES)
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o %.a, $^) $(CFLAGS) $(LDFLAGS)
	$(CP) $@ ${NEXUS_BIN_DIR}

transcode: CFLAGS += -I$(NEXUS_TOP)/../magnum/commonutils/vlc
ifneq ($(NEXUS_PLAYBACK_SUPPORT),n)
TRANSCODE_DEPENDENCIES = $(NEXUS_TOP)/../BSEAV/lib/bcmplayer/src/bcmindexer.c $(NEXUS_TOP)/../magnum/commonutils/vlc/bvlc.c $(NXCLIENT_APPS_UTILS_LIB)
else
TRANSCODE_DEPENDENCIES =
endif
transcode: %: %.c $(TRANSCODE_DEPENDENCIES)
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o %.a, $^) $(CFLAGS) $(LDFLAGS)
	$(CP) $@ ${NEXUS_BIN_DIR}

list_all_boxmodes:
	$(Q_)echo Auto-generating $@
	$(Q_)($(MAKE) -s -f $(NEXUS_TOP)/nxclient/apps/list_all_boxmodes.mk NEXUS_TOP=$(NEXUS_TOP)) > $@
	$(Q_)chmod +x $@
	$(CP) $@ ${NEXUS_BIN_DIR}

playviewer: %: %.c $(NXCLIENT_APPS_UTILS_LIB)
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o %.a, $^) $(CFLAGS) $(LDFLAGS)
	$(CP) $@ ${NEXUS_BIN_DIR}

GETSTATUS_AUTOGEN_INCLUDES=$(BINDIR)/getstatus_autogen_includes.txt
GETSTATUS_AUTOGEN=$(BINDIR)/getstatus_autogen.c
OTHER_CLEANS=$(GETSTATUS_AUTOGEN_INCLUDES) $(GETSTATUS_AUTOGEN)
$(GETSTATUS_AUTOGEN):
	${Q_}echo Auto-generating $@
	${Q_}$(RM) $(GETSTATUS_AUTOGEN_INCLUDES)
ifeq ($(NEXUS_PREBUILT_BINARY),y)
	${Q_}for file in $(filter-out $(NEXUS_BIN_DIR)/include/nexus.h $(NEXUS_BIN_DIR)/include/nexus.cpp.h $(NEXUS_BIN_DIR)/include/nexus_config.h, $(shell ls $(NEXUS_BIN_DIR)/include/*.h 2>/dev/null)); do \
	    echo "$${file}"; \
	    done >> $(GETSTATUS_AUTOGEN_INCLUDES)
else
	${Q_}for file in $(foreach module,$(filter-out SECURITY,$(NEXUS_MODULES)),$(shell ls $(addsuffix /*.h, $(NEXUS_${module}_PUBLIC_INCLUDES)) 2>/dev/null)); do \
	    echo "$${file}"; \
	    done >> $(GETSTATUS_AUTOGEN_INCLUDES)
endif
	${Q_}cd $(NEXUS_TOP)/build/tools/common; perl nexus_getstatus_client.pl $(GETSTATUS_AUTOGEN_INCLUDES) >$@ || $(RM) $@

getstatus: %: %.c $(GETSTATUS_AUTOGEN)
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $(filter-out $(GETSTATUS_AUTOGEN), $^)) $(CFLAGS) $(LDFLAGS)
	$(CP) $@ ${NEXUS_BIN_DIR}

B_BSEAV_TOP = $(NEXUS_TOP)/../BSEAV
ifeq ($(shell test -e $(B_BSEAV_TOP)/lib/dcc/b_digital_closed_caption.inc && echo y),y)
EXTRA_BUILD_RULES += dcclib
EXTRA_CLEAN_RULES += clean_dcclib
include $(B_BSEAV_TOP)/lib/dcc/b_digital_closed_caption.inc
dcclib:
	$(MAKE) -C $(B_BSEAV_TOP)/lib/dcc B_NEXUS_API_BUILD_COMPLETED=y
clean_dcclib:
	$(MAKE) -C $(B_BSEAV_TOP)/lib/dcc clean

# Convert applib include paths into single variable.
APPLIB_INCLUDES := $(foreach lib,$(B_LIBS),$($(lib)_PUBLIC_INCLUDES))
APPLIB_DEFINES := $(foreach lib,$(B_LIBS),$($(lib)_DEFINES))
ccgfx: CFLAGS += $(addprefix -I,$(APPLIB_INCLUDES)) $(addprefix -D,$(APPLIB_DEFINES)) $(B_DCC_LIB_CFLAGS_OPTIONS) -I$(NEXUS_TOP)/../magnum/commonutils/udp/include -I$(NEXUS_TOP)/../magnum/commonutils/avc/include
ifneq ($(NEXUS_PLAYBACK_SUPPORT),n)
ccgfx: LDFLAGS += -lm -lb_dcc
ifeq ($(FREETYPE_SUPPORT),y)
ccgfx: LDFLAGS += -lfreetype
endif
CCGFX_DEPENDENCIES = dcclib $(NEXUS_TOP)/../magnum/commonutils/udp/src/budp_dccparse.c $(NEXUS_TOP)/../magnum/commonutils/udp/src/budp_bitread.c parse_userdata.c $(NXCLIENT_APPS_UTILS_LIB)
else
CCGFX_DEPENDENCIES =
endif
ccgfx: %: %.c $(CCGFX_DEPENDENCIES)
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o %.a, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	$(CP) $@ ${NEXUS_BIN_DIR}
	@if [ ! -d $(NEXUS_BIN_DIR)/cc_fonts ]; then \
		echo [Install Fonts]; \
		$(MKDIR) $(NEXUS_BIN_DIR)/cc_fonts; \
		$(CP) $(B_BSEAV_TOP)/app/atlas/fonts/cc_fonts/* $(NEXUS_BIN_DIR)/cc_fonts; \
	fi
else
ccgfx:
	$(Q_)echo dcc lib source not available for $@ >$@
endif

# equivalent of NXCLIENT_LOCAL=y
switched_decode: CFLAGS += -I$(NEXUS_TOP)/nxclient/server
switched_decode: switched_decode.c server $(NXCLIENT_APPS_UTILS_LIB)
	@echo [Compile... $(notdir $<)]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o %.a, $^) $(CFLAGS) $(NEXUS_LD_LIBRARIES) -lnxserver
	$(CP) $@ ${NEXUS_BIN_DIR}

playmosaic: LDFLAGS += -lm

clean: clean_server clean_apps $(EXTRA_CLEAN_RULES)

clean_apps:
	B_REFSW_REAL_MAKE= $(MAKE) -C $(NEXUS_TOP)/nxclient/build clean
	-$(RM) $(APPS) $(MANUAL_APPS) $(NXCLIENT_APPS_UTILS_LIB) *.d *.o *.out $(OTHER_CLEANS)

copy_files:
	cd $(NEXUS_TOP)/nxclient/apps/tests;for FILE in `ls`; do $(CP) $$FILE ${NEXUS_BIN_DIR}; chmod a+x ${NEXUS_BIN_DIR}/$$FILE; done
	$(MKDIR) ${NEXUS_BIN_DIR}/nxclient
	$(CP) $(NEXUS_TOP)/nxclient/apps/resources/* ${NEXUS_BIN_DIR}/nxclient

endif # B_REFSW_REAL_MAKE
