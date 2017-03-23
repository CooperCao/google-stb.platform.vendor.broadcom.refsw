#############################################################################
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
#############################################################################


#
# Reference Software Makefile
#
# This Makefile will build the following:
# 1) driver(s)
# 2) nexus api (shared or static)
# 3) libs
# 4) atlas
#

#########
#### Work out Kernel Version from Kernel Source's Top Level Makefile
#########
K_VERSION       := $(shell grep -m1 "^VERSION" $(LINUX)/Makefile | cut -d\  -f3)
K_PATCHLEVEL    := $(shell grep -m1 "^PATCHLEVEL" $(LINUX)/Makefile | cut -d\  -f3)
K_SUBLEVEL      := $(shell grep -m1 "^SUBLEVEL" $(LINUX)/Makefile | cut -d\  -f3)
K_EXTRAVERSION  := $(shell grep -m1 "^EXTRAVERSION" $(LINUX)/Makefile | cut -d-  -f2-)
KERNEL_VERSION  := $(K_VERSION).$(K_PATCHLEVEL).$(K_SUBLEVEL)-$(K_EXTRAVERSION)

check-linux-version = $(shell expr \( $(K_VERSION) \* 65536 + $(K_PATCHLEVEL) \* 256 + $(K_SUBLEVEL) \) \>= \( $(1) \* 65536 + $(2) \* 256 + $(3) \))

#####
###First thing we must include the tools
#####

#########
#### Figure out OS
#########
ifeq ($(B_REFSW_OS),)
B_REFSW_OS=linuxuser
endif

include ../../../../nexus/build/os/$(B_REFSW_OS)/os_tools.inc

BSEAV = $(shell cd "../../.." && ${PWD})
NEXUS_TOP = $(shell cd "../../../../nexus" && ${PWD})
ifeq ($(NEXUS_PLATFORM),)
# PLATFORM is deprecated
$(error Please define NEXUS_PLATFORM first)
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

ifeq ($(ATLAS_BIN_DIR),)
# location of final binaries
ATLAS_BIN_ROOT = $(B_REFSW_OBJ_ROOT)
ATLAS_BIN_DIR = $(abspath $(ATLAS_BIN_ROOT)/BSEAV/bin${ATLAS_BIN_DIR_SUFFIX})
else
ATLAS_BIN_ROOT = $(ATLAS_BIN_DIR)
endif

ifeq ($(NEXUS_MODE),)
# MODE is deprecated
NEXUS_MODE = $(MODE)
endif


#########
#### Figure out ARCH
#########
ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mips-linux
export B_REFSW_ARCH
endif

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
endif

#########
#### Include NEXUS .inc
#########
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
B_LIB_TOP := $(NEXUS_TOP)/lib
include $(B_LIB_TOP)/os/b_os_lib.inc

# Get system-specific CFLAGS and LDFLAGS based on SYSTEM, PLATFORM, DEBUG
include $(BSEAV)/build/refsw_inc.mak

#########
#### Include thumbnail .inc
#########
include $(BSEAV)/lib/thumbnail/bthumbnail.inc
CFLAGS += $(addprefix -I,$(BTHUMBNAIL_INCLUDES))
CFLAGS += $(addprefix -D,$(BTHUMBNAIL_DEFINES))
ATLAS_DIRS += $(BSEAV)/lib/thumbnail
APP_OBJECTS += bthumbnail_extractor.o

#################
############ Staging Area setup directory. It will be destroyed later
#################
STAGING_DIR = ${ATLAS_BIN_DIR}/setup

##############
##### ATLAS_ODIR keeps binary files separate
##### BCHP_REV is not included, so there should be no #ifdef BCHP_REV in Atlas
##############
ATLAS_ODIR = $(ATLAS_BIN_ROOT)/BSEAV/app/atlas/$(NEXUS_PLATFORM).$(B_REFSW_ARCH).$(DEBUG_SUFFIX)
APP := $(ATLAS_ODIR)/atlas

#######
#### Font Directories
#######
ifeq ($(B_REFSW_ARCH),mips-linux)
PRERENDERED_FONTS = ../fonts/mswebfonts/be/*.bwin_font
else # goes for ARM as well which is LE only
PRERENDERED_FONTS = ../fonts/mswebfonts/le/*.bwin_font
endif
PRERENDERED_CC_FONTS = ../fonts/cc_fonts/*.bwin_font

ESTB_SNMP_SUPPORT ?= n
ifeq ($(ESTB_SNMP_SUPPORT), y)
CABLECARD_SUPPORT ?= y
ifeq ($(findstring $(NEXUS_PLATFORM), 97420 97425 97429 97435), $(NEXUS_PLATFORM))
SNMPCONFIG=doconfigure.mips
else
SNMPCONFIG=doconfigure.arm
endif
endif

ifneq ($(B_REFSW_ARCH),mips-linux)
DCC_SUPPORT ?= y
endif
ifeq ($(CABLE_SUPPORT), y)
CABLECARD_SUPPORT ?= y
DSG_SUPPORT ?= y
ESTB_CFG_SUPPORT ?= y
endif

ifeq ($(BCM3128_OOB_CABLECARD_SUPPORT),y)
CABLECARD_SUPPORT = y
DSG_SUPPORT = n
CDL_SUPPORT = n
CFLAGS += -DBCM3128_OOB_CABLECARD_SUPPORT
TWOWAY_OOB_CABLECARD_SUPPORT = n
endif


ifeq ($(CABLECARD_SUPPORT),y)

export B_REFSW_TOOLCHAIN_VER_GE_4_8

CABLECARD_SI_INCLUDE_PATH += \
	$(BSEAV)/cable/lib/si \
	$(BSEAV)/lib/mpeg2_ts_parse
CFLAGS += $(addprefix -I,$(CABLECARD_SI_INCLUDE_PATH))
endif
##########
#### Atlas now defaults to IP Settop mode for all platforms
#### You can turn this default off with PLAYBACK_IP_SUPPORT=n
#### Netaccel support is deprecated and not included in any further Releases
##########
ifneq ($(PLAYBACK_IP_SUPPORT),n)
PLAYBACK_IP_SUPPORT=y

# Add playback_ip, but remove the "-lnexus" link flag because we'll add the proper nexus link flags later.
include $(NEXUS_TOP)/lib/playback_ip/b_playback_ip_lib.inc
LDFLAGS += $(filter-out -lnexus,$(PLAYBACK_IP_LDFLAGS))
CFLAGS += -DPLAYBACK_IP_SUPPORT $(PLAYBACK_IP_CFLAGS)

# Add psip
B_PSIP_TOP := $(NEXUS_TOP)/../rockford/lib/psip
include $(B_PSIP_TOP)/b_psip_lib.inc
CFLAGS += $(addprefix -I,$(B_PSIP_LIB_PUBLIC_INCLUDES))

# Add tshdrbuilder
include $(BSEAV)/lib/tshdrbuilder/tshdrbuilder.inc
CFLAGS += $(addprefix -I,$(TSHDRBUILDER_INCLUDES))

# Add BIP
include $(NEXUS_TOP)/lib/bip/bip_lib.inc
# use all FLAGS!
CFLAGS += $(BIP_ALL_LIB_CFLAGS)
LDFLAGS += $(BIP_ALL_LIB_LDFLAGS)

AUTO_PSI_SUPPORT=y
endif  # PLAYBACK_IP_SUPPORT

.PHONY: bin_install api all $(ATLAS_ODIR)

# The name of the installation tar ball , Change to Atlas
#
BUILDDATE = `date +%Y%m%d`
ATLAS_TAR_BALL = refsw-$(BUILDDATE).$(NEXUS_PLATFORM)-$(SYSTEM)
ATLAS_TAR_BALL := $(ATLAS_TAR_BALL)-atlas
ATLAS_TAR_BALL := $(ATLAS_TAR_BALL).bin.tgz

#
# Set B_REFSW_SHAREABLE based on a variety of flags
#
ifeq ($(filter y,$(STATIC_ATLAS)),y)
B_REFSW_SHAREABLE=n
endif

# bin_install is the default rule.
# 'all' builds everything but doesn't perform bin_install or install.
# bin_install copies only binaries into BSEAV/bin. It allows you to
#  run from that directory without having your configuration files overwritten.
# install builds an installation tarball in BSEAV/bin.
bin_install:  $(ATLAS_BIN_DIR) api all
	@echo [Install... binaries to $(ATLAS_BIN_DIR)]
# Copy over the nexus script into the correct Directory
	${Q_}$(CP) $(APP) $(ATLAS_BIN_DIR)
	${Q_}$(CP) $(NEXUS_BIN_DIR)/logger $(ATLAS_BIN_DIR)
	${Q_}$(CP) $(NEXUS_BIN_DIR)/nexus $(ATLAS_BIN_DIR)
	${Q_}$(CP) start $(ATLAS_BIN_DIR)
	${Q_}chmod +x $(ATLAS_BIN_DIR)/nexus
	${Q_}chmod +x $(ATLAS_BIN_DIR)/start
	@echo [Install... Atlas Complete]




$(ATLAS_ODIR):
	${Q_}$(MKDIR) $(ATLAS_ODIR)

##########
##### NEXUS API Libraries
##########

include $(BSEAV)/lib/bwin/build/b_bwin_targets.inc
include $(BSEAV)/lib/mlibs/b_mlibs_targets.inc
include $(BSEAV)/lib/bwidgets/build/b_bwidgets_targets.inc

###############
####### Power Standby
###############
PMLIB_BUILD:=
PMLIB_CLEAN:=
ifeq ($(NEXUS_POWER_MANAGEMENT),y)
ifeq ($(call check-linux-version,2,6,30),1)
# if kernel >= 2.6.30
PMLIB_TOP=$(BSEAV)/lib/pmlib
include $(PMLIB_TOP)/pmlib.inc
include $(PMLIB_TOP)/b_pmlib_targets.inc
CFLAGS += -DPOWERSTANDBY_SUPPORT $(PMLIB_CFLAGS)
LDFLAGS += $(PMLIB_LDFLAGS)
PMLIB_BUILD += b_pmlib_target_build
PMLIB_CLEAN += b_pmlib_target_clean
endif
endif

### we need jpeg support here for cpuTest
CPUTEST_SUPPORT?=y
LIBJPEG_BUILD:=
LIBJPEG_CLEAN:=
ifeq ($(JPEG_SUPPORT),y)
ifeq ($(CPUTEST_SUPPORT),y)
include $(BSEAV)/opensource/jpeg/b_libjpeg_targets.inc
LIBJPEG_BUILD: b_libjpeg_target_build
LIBJPEG_CLEAN: b_libjpeg_target_clean_all
CFLAGS += -DJPEG_SUPPORT -DCPUTEST_SUPPORT -I$(B_REFSW_OBJ_ROOT)/BSEAV/opensource/jpeg-6b
endif
endif

###############
####### LUA Support
###############
LUA_BUILD:=
LUA_CLEAN:=
ifneq ($(LUA_SUPPORT),n)
include $(BSEAV)/lib/lua/b_lua_targets.inc
APP_OBJECTS += atlas_lua.o
LUA_DIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/lua/lua-5.2.4/install
CFLAGS += -DLUA_SUPPORT -I$(LUA_DIR)/include
LIBS += $(LUA_DIR)/lib/liblua.a
LINENOISE_DIR = $(BSEAV)/app/atlas/src/linenoise/antirez-linenoise-02d7935
CFLAGS += -DLINENOISE_SUPPORT -I$(LINENOISE_DIR)
VPATH += $(LINENOISE_DIR)
APP_OBJECTS += linenoise.o
LUA_BUILD += b_lua_target_build
LUA_CLEAN += b_lua_target_clean
endif

##########
#####always build main.o because it contains __DATE__
##########
.PHONY: build_date
build_date:
	${Q_}$(RM) -f $(ATLAS_ODIR)/main.o

$(ATLAS_ODIR)/main.o: build_date

clean-atlas:
	-@$(RM) -rf $(ATLAS_ODIR)

lib: b_bwin_target_build b_mlibs_target_build b_bwidgets_target_build

lib-clean: b_bwin_target_clean b_mlibs_target_clean b_bwidgets_target_clean
clean-lib: lib-clean

print-atlas-tarball:
	@echo $(ATLAS_TAR_BALL)

$(ATLAS_BIN_DIR):
	${Q_}${MKDIR} ${ATLAS_BIN_DIR}

##########
##### Rule for all and clean
##########
all: $(ATLAS_BIN_DIR) $(APP)
	${Q_}$(CP) -f $(APP) $(ATLAS_BIN_DIR)

clean: api-clean playback_ip_lib_clean lib-clean nxclient-clean $(LUA_CLEAN) clean-dvrlib clean-pod-driver clean-podlibs clean-dsgcclib clean-cdllib clean-estb-cfglibs clean-dcclib clean-snmp-agent clean-snmp-proxy clean-silib clean-atlas clean-perftools $(PMLIB_CLEAN)

dist-clean: clean dist-clean-netapp
	@echo [Clean..... libraries]
	${Q_}$(RM) -rf $(ATLAS_BIN_ROOT)/BSEAV/lib
	${Q_}$(RM) -rf $(ATLAS_BIN_ROOT)/BSEAV/opensource
	${Q_}$(RM) -rf $(ATLAS_BIN_ROOT)/target
	@echo [Clean..... linux]
	${Q_}$(RM) -rf $(ATLAS_BIN_ROOT)/BSEAV/linux

# oslib
b_os_lib_target_build: $(ATLAS_BIN_DIR) $(ATLAS_ODIR)
B_OSLIB_TARGET_MAKEFLAGS += INSTALL_DIR=$(ATLAS_BIN_DIR)
include $(NEXUS_TOP)/lib/os/b_os_lib_targets.inc

api:  $(ATLAS_BIN_DIR) $(ATLAS_ODIR) b_os_lib_target_build
	@echo [Build..... Nexus]
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build  INSTALL_DIR=$(ATLAS_BIN_DIR)
	${Q_}$(CP) -f $(NEXUS_BIN_DIR)/lib*.so $(ATLAS_BIN_DIR)
ifeq  ($(NEXUS_MODE),proxy)
	@echo [Install... KernelMode Driver to $(ATLAS_BIN_DIR)]
	${Q_}$(CP)   $(NEXUS_BIN_DIR)/nexus.ko $(ATLAS_BIN_DIR)
else
	@echo [Install... UserMode Driver to $(ATLAS_BIN_DIR)]
	${Q_}$(CP)   $(NEXUS_BIN_DIR)/bcmdriver.ko $(ATLAS_BIN_DIR)
endif
ifeq ($(SAGE_SUPPORT),y)
	@echo [Install... Sage binaries to $(ATLAS_BIN_DIR)]
	${Q_}$(CP)   $(NEXUS_BIN_DIR)/sage*.bin $(ATLAS_BIN_DIR)
endif
	${Q_}if [ -f $(NEXUS_BIN_DIR)/wakeup_drv.ko ] ; then  \
		$(CP) $(NEXUS_BIN_DIR)/wakeup_drv.ko $(ATLAS_BIN_DIR)/wakeup_drv.ko; \
	fi;

	${Q_}if [ -f $(NEXUS_BIN_DIR)/zigbee_drv.ko ] ; then  \
		$(CP) $(NEXUS_BIN_DIR)/zigbee_drv.ko $(ATLAS_BIN_DIR)/zigbee_drv.ko; \
	fi;

	${Q_}if [ -f $(NEXUS_BIN_DIR)/SoC_mailboxHost.elf ] ; then  \
		$(CP) $(NEXUS_BIN_DIR)/SoC_mailboxHost.elf $(ATLAS_BIN_DIR)/SoC_mailboxHost.elf; \
	fi;

api-install: api
	@echo [Build..... Nexus]
	${Q_}$(foreach file,$(B_OS_LIB_BUILD_FILES),cp -u $(file) $(STAGING_DIR);)
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build  NEXUS_BIN_DIR=$(STAGING_DIR)
ifeq  ($(NEXUS_MODE),proxy)
	@echo [Install... KernelMode Driver to $(STAGING_DIR)]
	${Q_}$(CP)   $(NEXUS_BIN_DIR)/nexus.ko $(STAGING_DIR)
else
	@echo [Install... UserMode Driver to $(STAGING_DIR)]
	${Q_}$(CP)   $(NEXUS_BIN_DIR)/bcmdriver.ko $(STAGING_DIR)
endif


api-clean: b_os_lib_target_clean
	${Q_}$(MAKE) -C $(NEXUS_TOP)/build clean
	${Q_}$(RM) -f $(ATLAS_BIN_DIR)/nexus.ko
	${Q_}$(RM) -f $(ATLAS_BIN_DIR)/bcmdriver.ko
clean-api: api-clean

################
######## Source Files for compiling
###############



##################
######## Atlas objs
#################

APP_OBJECTS += \
    atlas_main.o \
    board.o \
    platform.o \
    atlas_os.o \
    atlas_cfg.o \
    notification.o \
    resource.o \
    convert.o \
    display.o \
    graphics.o \
    video_window.o \
    output.o \
    video_decode.o \
    pid.o \
    stc.o \
    tuner.o \
    tuner_qam.o \
    tuner_vsb.o \
    tuner_sat.o \
    tuner_ofdm.o \
    band.o \
    mvc.o \
    pidmgr.o \
    config.o \
    channelmgr.o \
    channel.o \
    channel_streamer.o \
    channel_bip.o \
    channel_qam.o \
    channel_vsb.o \
    channel_sat.o \
    channel_ofdm.o \
    channel_playback.o \
    channel_mosaic.o \
    crypto.o \
    videolist.o \
    playback.o \
    record.o \
    encode.o \
    still_decode.o \
    dma.o \
    timebase.o \
    thumb.o \
    audio_decode.o \
    audio_processing.o \
    remote.o \
    control.o \
    power.o \
    audio_capture_client.o \
    audio_capture.o \
    bluetooth.o \
    timer.o \
    view.o \
    model.o \
    controller.o \
    widget_engine.o \
    widget_base.o \
    widget_label.o \
    widget_button.o \
    widget_listview.o \
    widget_progress.o \
    widget_meter.o \
    widget_grid.o \
    widget_edit.o \
    widget_menu.o \
    widget_check_button.o \
    widget_popup_list.o \
    widget_modalmsgbox.o \
    screen.o \
    screen_main.o \
    panel.o \
    panel_display.o \
    panel_decode.o \
    panel_audio.o \
    panel_playback.o \
    panel_record.o \
    panel_scan_qam.o \
    panel_scan_vsb.o \
    panel_scan_sat.o \
    panel_scan_ofdm.o \
    panel_tuner.o \
    panel_cablecard.o \
    panel_buffers.o \
    panel_vbi.o \
    panel_power.o \
    panel_network_wifi.o \
    panel_bluetooth.o \
    panel_keyboard.o \
    main.o

ifeq ($(JPEG_SUPPORT),y)
ifeq ($(CPUTEST_SUPPORT),y)
APP_OBJECTS += \
    cputest.o
endif
endif

ifeq ($(PLAYBACK_IP_SUPPORT),y)
APP_OBJECTS += \
    panel_streaming.o \
    server.o \
    servermgr.o \
    server_http.o \
    server_udp.o \
    streamer_http.o \
    streamer_udp.o \
    server_playlist.o \
    playlist.o \
    playlist_db.o \
    discovery.o
endif

#####
### Atlas Dirs
#####
ATLAS_DIRS += ../src
ATLAS_DIRS += ../src/mvc
ATLAS_DIRS += ../src/widgets
ATLAS_DIRS += ../src/platform
ATLAS_DIRS += ../src/views
ATLAS_DIRS += ../src/ocap

ifeq ($(DVR_LIB_SUPPORT),y)
B_CABLE_TOP = $(shell cd "../../../cable" && ${PWD})
B_LIB_TOP = $(shell cd "../../../../nexus/lib" && ${PWD})
B_BSEAV_TOP := $(shell cd "../../.." && ${PWD})
DVR_LIB_DIR := $(BSEAV)/cable/lib/dvr
LDFLAGS += -L$(NEXUS_BIN_DIR) -lb_dvr
CFLAGS += -DDVR_LIB_SUPPORT
include $(DVR_LIB_DIR)/b_dvr_lib.inc
CFLAGS += $(addprefix -D,$(B_DVR_LIB_DEFINES))
CFLAGS += $(addprefix -I,$(B_DVR_LIB_PUBLIC_INCLUDES))
APP_OBJECTS += dvrmgr.o
APP_OBJECTS += tsb.o
endif
#################
##### Cablecard
#################
ifeq ($(CABLECARD_SUPPORT),y)
TWOWAY_OOB_CABLECARD_SUPPORT ?= y
PSISCAN_SUPPORT = n
SI_SUPPORT = y
export NEXUS_MPOD_SUPPORT = y
BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC = 0
CFLAGS += -DMPOD_SUPPORT -DINTEGRATE_BCM_DSG_HOST
POD_DIR = $(BSEAV)/cable/lib/mpod
POD_STAGING_DIR = ${B_REFSW_OBJ_ROOT}/BSEAV/cable/lib/mpod
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux arm-linux), ${B_REFSW_ARCH})
CP_DIR = $(POD_DIR)/lib/cp_arm
else
CP_DIR = $(POD_DIR)/lib/cp
endif
#POD_DRIVERDIR = $(BSEAV)/cable/lib/mpod/src/driver/build/bcm$(NEXUS_PLATFORM)$(shell awk 'BEGIN{print tolower("$(BCHP_VER)")}')/$(SYSTEM)
POD_DRIVERDIR = $(BSEAV)/cable/lib/mpod/src/driver/build/$(SYSTEM)

LDFLAGS += -L${B_REFSW_OBJ_ROOT}/nexus/bin -lb_mpod
LDFLAGS += -L$(POD_STAGING_DIR) -lcp
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux arm-linux), ${B_REFSW_ARCH})
ifeq ($(B_REFSW_TOOLCHAIN_VER_GE_4_8),y)
LDFLAGS += -L$(POD_STAGING_DIR) -lcrypto
else
LDFLAGS += -L$(POD_STAGING_DIR) -lcrypto_uclibc
endif
else
LDFLAGS += -L$(POD_STAGING_DIR) -lcrypto.${B_OPENSSL_VER}_uclibc
endif

APP_OBJECTS += \
	tuner_oob.o \
	channel_oob.o \
	tuner_upstream.o \
	channel_upstream.o \
	cablecard.o
endif
#
# DSG support
#
ifeq ($(DSG_SUPPORT),y)
ifeq ($(DEBUG),y)
SLIM_FLAG =
else
SLIM_FLAG = slim
endif
ifeq ($(findstring $(NEXUS_PLATFORM), 97420 97422 97459), $(NEXUS_PLATFORM))
DOCSIS_CHIP = 3255
else ifeq ($(findstring $(NEXUS_PLATFORM), 97125), $(NEXUS_PLATFORM))
DOCSIS_CHIP = 7125
else ifeq ($(findstring $(NEXUS_PLATFORM), 97429 97425 97435), $(NEXUS_PLATFORM))
ifneq ($(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT), y)
DOCSIS_CHIP = 3383
else
DOCSIS_CHIP = 3384
endif
else
DOCSIS_CHIP = 3384
endif

ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux arm-linux), ${B_REFSW_ARCH})
export ARM_PLATFORM = yes
else
export ARM_PLATFORM = no
endif


DSG_DIR = $(BSEAV)/cable/docsis/estb

CFLAGS += -DDSG_SUPPORT -DBCM_DSG_DUAL_PROCESSOR_INTERFACE
LIBS += ${B_REFSW_OBJ_ROOT}/BSEAV/cable/docsis/estb/StbHostApps/DsgCC/StbApp_DsgCC/linux/bcm/dsgcclib.a

#remove this line if gcc3.3 is used
LDFLAGS += -lrt
ifeq ($(CABLECARD_SUPPORT),y)
CC_FLAG = cablecard
else
CC_FLAG =
endif

CDL_CMDS =
ifeq ($(CDL_SUPPORT),y)
CFLAGS += -DCDL_SUPPORT
#default is to support unsigned image and unsigned cvt
CDL_SUPPORT_ENABLE_SIGNED_IMAGE = n

ifeq ($(CDL_SUPPORT_ENABLE_SIGNED_IMAGE),y)
CFLAGS += -DCDL_SUPPORT_ENABLE_SIGNED_IMAGE
endif

LIBS += $(ATLAS_BIN_ROOT)/nexus/bin/libb_cdl.a
LIBS += $(ATLAS_BIN_ROOT)/nexus/bin/libb_dsmcc.a

endif

ATLAS_DIRS += -I$(DSG_DIR)/cm_bsp_v2/bsp_common/common_inc/
ATLAS_DIRS += -I$(DSG_DIR)/StbHostApps/DsgCC/
ATLAS_DIRS += -I$(DSG_DIR)/StbHostApps/RswDload
ATLAS_DIRS += -I$(DSG_DIR)/Stb/StbDocsisApi/Dsg
ATLAS_DIRS += -I$(DSG_DIR)/StbHostApps/StbEcmNvolEditor/Stb/StbDocsisApi/Linux

CDL_CMDS = cdl20 rswdload
endif

##########################
###### Common Download support
#########################
ifeq ($(CDL_SUPPORT),y)
CDL_LIB_DIR = $(BSEAV)/cable/lib/cdl
DSMCC_LIB_DIR = $(BSEAV)/cable/lib/dsmcc
ATLAS_INCLUDE_DIRS += -I$(CDL_LIB_DIR)/include
ATLAS_INCLUDE_DIRS += -I$(DSMCC_LIB_DIR)/include
LDFLAGS += -L$(ATLAS_BIN_ROOT)/nexus/bin -lb_cdl -lb_dsmcc
endif

#
# ESTB Non Vol support
#
ifeq ($(ESTB_CFG_SUPPORT),y)
ESTB_CFG_SUPPORT=y
export ESTB_CFG_SUPPORT
CFLAGS += -DESTB_CFG_SUPPORT
ESTB_CFG_LIB_DIR = $(BSEAV)/cable/lib/estb_cfg
ATLAS_INCLUDE_DIRS += -I$(ESTB_CFG_LIB_DIR)/include
LDFLAGS += -L$(ATLAS_BIN_ROOT)/nexus/bin -lb_estb_cfg
endif

#
# Digital Closed Caption support
#
ifeq ($(DCC_SUPPORT),y)
ATLAS_DIRS += \
	$(NEXUS_TOP)/../magnum/commonutils/udp/src \
	$(NEXUS_TOP)/nxclient/apps/utils
APP_OBJECTS += budp_dccparse.o
APP_OBJECTS += budp_bitread.o
APP_OBJECTS += parse_userdata.o
APP_OBJECTS += closed_caption.o
CFLAGS += -DDCC_SUPPORT
DCC_LIB_DIR = $(BSEAV)/lib/dcc
LDFLAGS += -L$(ATLAS_BIN_ROOT)/bin -lb_dcc
endif

#
# SNMP support
#
ifeq ($(ESTB_SNMP_SUPPORT),y)
SNMP_DIR = $(BSEAV)/cable/lib/snmp
CFLAGS += -DSNMP_SUPPORT \
		-I$(SNMP_DIR)/agent/mibgroup/ocmib \
		-I$(SNMP_DIR)/agent/mibgroup/ocmib/home_network \
		-I$(SNMP_DIR)/agent/mibgroup/ocmib/stb_host \
		-I$(SNMP_DIR)/include/net-snmp \
		-I$(SNMP_DIR)/include
LDFLAGS += -L$(SNMP_DIR)/agent/.libs -lnetsnmpmibs -lnetsnmpagent \
		-L$(SNMP_DIR)/agent -lsnmpd \
		-L$(SNMP_DIR)/agent/helpers/.libs -lnetsnmphelpers \
		-L$(SNMP_DIR)/snmplib/.libs -lnetsnmp
APP_OBJECTS += snmp.o
ATLAS_INCLUDE_DIRS += -I$(SNMP_DIR)/include
endif

#################
##### PSI/SCTE65/PSIP parsing support
#################
ifeq ($(SI_SUPPORT),y)
#ifeq ($(B_REFSW_ARCH),arm-linux)
#TOOLCHAIN_DIR:=$(shell dirname $(shell dirname $(shell which arm-linux-gcc 2>/dev/null) 2>/dev/null))
#else
#TOOLCHAIN_DIR:=$(shell dirname $(shell dirname $(shell which mips-linux-uclibc-gcc 2>/dev/null) 2>/dev/null))
#endif

TOOLCHAIN_DIR:=$(shell dirname $(shell dirname $(shell which $(CC) 2>/dev/null) 2>/dev/null))

CROSS_COMPILE ?= mips-linux-uclibc-
SI_DIR = $(BSEAV)/cable/lib/si
TSPSI_DIR = $(BSEAV)/lib/tspsi2
ATLAS_DIRS += \
	$(SI_DIR) \
	$(SI_DIR)/aeit \
	$(SI_DIR)/aett \
	$(SI_DIR)/ea \
	$(SI_DIR)/lvct \
	$(SI_DIR)/mgt \
	$(SI_DIR)/nit \
	$(SI_DIR)/ntt \
	$(SI_DIR)/os/linux \
	$(SI_DIR)/rrt \
	$(SI_DIR)/stt \
	$(SI_DIR)/svct \
	$(SI_DIR)/api \
	$(BSEAV)/lib/mpeg2_ts_parse \
	$(TSPSI_DIR)
ATLAS_INCLUDE_DIRS += -I$(TSPSI_DIR)
LDFLAGS += -L$(B_REFSW_OBJ_ROOT)/BSEAV/cable/lib/si -lsi
endif

#################
##### PSI SCAN
#################
ifneq ($(PSISCAN_SUPPORT),n)
	CFLAGS += -DPSISCAN_SUPPORT
	TRANSPORTDECODE_DIR=$(BSEAV)/lib/mpeg2_ts_parse
	TSPSI_DIR=$(BSEAV)/lib/tspsi2
	ATLAS_DIRS += \
		$(TRANSPORTDECODE_DIR) \
		$(TSPSI_DIR)
	# all other tspsi and mpeg2_ts_parse are compiled in with nexus
	APP_OBJECTS += tspsimgr2.o
endif


ATLAS_OBJS := $(addprefix $(ATLAS_ODIR)/,$(APP_OBJECTS))

# For linux builds, link to the correct libraries
ifneq ($(B_REFSW_OS),linuxkernel)
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS += -lpthread -lm  $(EXTRA_DRM_OBJS)
endif
endif

# Set up Nexus' and NxClient's LDFLAGS and CFLAGS depending on the build flags.
ifeq ($(NXCLIENT_SUPPORT),y)
        # Nexus NxClient Mode: NXCLIENT_SUPPORT=y, NEXUS_CLIENT_SUPPORT=<don't care>
        include $(NEXUS_TOP)/nxclient/include/nxclient.inc
        CFLAGS += ${NXCLIENT_CFLAGS}
        CXXFLAGS += ${NXCLIENT_CFLAGS}
        LDFLAGS += ${NXCLIENT_LDFLAGS}
else
    ifeq ($(NEXUS_CLIENT_SUPPORT),y)
        # Nexus Multiprocess non-NxClient Mode: NXCLIENT_SUPPORT=, NEXUS_CLIENT_SUPPORT=y
        $(info )
        $(error Atlas does not support Nexus Multiprocess No-NxClient mode! Please disable NEXUS_CLIENT_SUPPORT)
#       LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES) -lm
    else
        # Nexus Single-process Mode: NXCLIENT_SUPPORT=, NEXUS_CLIENT_SUPPORT=
        LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES) -lm
    endif
endif

###############
####### NxClient
###############
ifeq ($(NXCLIENT_SUPPORT),y)
	NXCLIENT_DIR = $(NEXUS_TOP)/nxclient

ATLAS_DIRS += ../src/nxclient
APP_OBJECTS += \
	atlas_main_nx.o \
	board_nx.o \
	control_nx.o \
	display_nx.o \
	audio_decode_nx.o \
	bluetooth_nx.o \
	audio_capture_nx.o \
	band_nx.o \
	config_nx.o \
	graphics_nx.o \
	output_nx.o \
	remote_nx.o \
	tuner_ofdm_nx.o \
	tuner_qam_nx.o \
	tuner_sat_nx.o \
	tuner_vsb_nx.o \
	video_decode_nx.o
ifeq ($(CABLECARD_SUPPORT),y)
APP_OBJECTS += \
	tuner_oob_nx.o \
	tuner_upstream_nx.o
endif
endif

###############
###### BWIN Widgets
###############
MLIBS=$(BSEAV)/lib/mlibs
CXXFLAGS += -I$(MLIBS)
CFLAGS += -I$(MLIBS)

# DEBUG SUFFIX Defined above
MLIBS_ODIR=$(abspath $(sort $(dir $(B_MLIBS_BUILD_FILES))))

CFLAGS += -I$(MLIBS) $(addprefix -L,$(MLIBS_ODIR))
LDFLAGS += $(addprefix -L,$(MLIBS_ODIR)) -lmcommon -lmnet

BWIN=$(BSEAV)/lib/bwin
include $(BWIN)/include/bwin.mak
CXXFLAGS += $(BWIN_CFLAGS) -DBWIN_SUPPORT
LDFLAGS += $(BWIN_LDFLAGS)
CFLAGS += -DBWIN_SUPPORT

VPATH += $(BWIDGETS_DIR)/src
include $(BSEAV)/lib/bwidgets/include/bwidgets.mak
CFLAGS += $(BWIDGETS_CFLAGS)
LDFLAGS += $(BWIDGETS_LDFLAGS)


###############
###### ASF AVI ETC MEDIA PROBE common Library
###############

# check to see if libraries exist, they should by default
ifeq ($(MEDIA_ASF_SUPPORT),)
ASF_SUPPORT= $(shell test -e ${BSEAV}/lib/asf && echo y)
else
ASF_SUPPORT=$(MEDIA_ASF_SUPPORT)
endif

ifeq ($(MEDIA_AVI_SUPPORT),)
AVI_SUPPORT= $(shell test -e ${BSEAV}/lib/avi && echo y)
else
AVI_SUPPORT=$(MEDIA_AVI_SUPPORT)
endif

ifeq ($(ASF_SUPPORT),y)
	include ${BSEAV}/lib/asf/basf.inc
endif
ifeq ($(AVI_SUPPORT),y)
	include ${BSEAV}/lib/avi/bavi.inc
endif
	include ${BSEAV}/../magnum/commonutils/vlc/bvlc.inc
	include ${BSEAV}/lib/media/bmedia.inc
        include ${BSEAV}/lib/utils/batom.inc
        include ${BSEAV}/lib/bfile/bfile.inc
ifneq ($(MEDIA_RMFF_SUPPORT),n)
        -include ${BSEAV}/lib/media/rmff/brmff.inc
endif
ifneq ($(MEDIA_FLV_SUPPORT),n)
        -include ${BSEAV}/lib/media/flv/bflv.inc
endif
        include ${BSEAV}/lib/bfile/bfile.inc
        ATLAS_MODULES := BMEDIA_PROBE BATOM BFILE_STDIO BVLC
	CFLAGS += -DPVR_SUPPORT

	# Direct bcmplayer access is needed for Video::indexToTime.
	CFLAGS += -I$(BSEAV)/lib/bcmplayer/include

	B_ATLAS_INC := $(addprefix -I,$(sort $(foreach module, $(ATLAS_MODULES), $($(module)_INCLUDES))))
	B_ATLAS_CFLAGS := $(addprefix -D,$(sort $(foreach module, $(ATLAS_MODULES), $($(module)_DEFINES))))
	B_ATLAS_SOURCES = $(sort $(foreach module, $(ATLAS_MODULES), $($(module)_SOURCES)))
	B_ATLAS_OBJS := $(patsubst %.c,%.o,$(notdir ${B_ATLAS_SOURCES}))
	vpath %c $(dir ${B_ATLAS_SOURCES})

	APP_OBJECTS += ${B_ATLAS_OBJS}
	CFLAGS += ${B_ATLAS_CFLAGS} ${B_ATLAS_INC}



# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Always build with debug
CFLAGS += -MD -g
CFLAGS += -DBDBG_DEBUG_BUILD
CFLAGS += -W -MMD -pedantic
LDFLAGS += -ldl -lpthread

ATLAS_INCLUDE_DIRS += $(addprefix -I,$(ATLAS_DIRS))
VPATH += $(ATLAS_DIRS)

####
## Include the B_OS abstraction layer
####
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/../nexus/lib/os/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/../nexus/lib/os/include/linuxuser

####
## Include cablecard library
####
ifeq ($(CABLECARD_SUPPORT), y)
ATLAS_INCLUDE_DIRS += -I$(POD_DIR)/include
ATLAS_INCLUDE_DIRS += -I$(POD_DIR)/src/util
ATLAS_INCLUDE_DIRS += -I$(POD_DIR)/src/link_layer
ATLAS_INCLUDE_DIRS += -I$(POD_DIR)/src/app_layer
ATLAS_INCLUDE_DIRS += -I$(POD_DIR)/src/session_layer
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/cable/lib/cdl/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/cable/lib/estb_cfg/include
ATLAS_INCLUDE_DIRS += -I$(BSEAV)/cable/lib/dsmcc/include
endif

####
## Include closed caption library
####
ifeq ($(DCC_SUPPORT), y)
ATLAS_INCLUDE_DIRS += -I$(NEXUS_TOP)/nxclient/apps/utils
ATLAS_INCLUDE_DIRS += -I$(DCC_LIB_DIR)/include
ATLAS_INCLUDE_DIRS += -I$(NEXUS_TOP)/../magnum/commonutils/udp/include
ATLAS_INCLUDE_DIRS += -I$(NEXUS_TOP)/../magnum/commonutils/avc/include
endif

####
## Include wlan driver
####

ifeq ($(findstring $(NEXUS_PLATFORM), 97271), $(NEXUS_PLATFORM))
#only build wlan support for 97271, default on
export WLAN_SUPPORT ?= y
else
export WLAN_SUPPORT=n
endif

ifeq ($(WLAN_SUPPORT), y)
include $(BSEAV)/connectivity/wlan/wlan.inc
endif

####
## Include zigbee
####

ifeq ($(filter $(NEXUS_PLATFORM), 97364 97366), $(NEXUS_PLATFORM))
ifeq ($(RF4CE_SUPPORT),)
export RF4CE_SUPPORT=n
endif
endif

ifeq ($(RF4CE_SUPPORT), y)
include $(BSEAV)/lib/zigbee/zigbee.inc
endif

####
## Include SI library
####
ifeq ($(SI_SUPPORT), y)
ATLAS_INCLUDE_DIRS += -I$(SI_DIR)
endif

####
## diagnostic tools support
####
ifeq ($(PERFTOOLS_SUPPORT),y)
BSYSPERF_DIR = $(BSEAV)/tools/bsysperf
BMEMPERF_DIR = $(BSEAV)/tools/bmemperf
endif

####
## NETAPP (WiFi and Bluetooth) support
####
ifeq ($(NETAPP_SUPPORT),y)
include $(BSEAV)/connectivity/netapp.inc
NETAPP_DIR = $(BSEAV)/../trellis/connectivity/netapp/build
NETAPP_OUTPUT_DIR = ${B_REFSW_OBJ_ROOT}/BSEAV/bin
INTERNAL_SERVER		?= git-irv-02.lvn.broadcom.net
FETCH_NETAPP_CMD 	:= repo init -u ssh://$(USER)@${INTERNAL_SERVER}:29418/AppLibs; repo sync
SERVER_ALIVE        ?= $(shell nslookup -timeout=1 ${INTERNAL_SERVER} | grep Address && echo "1s" || echo "")

ifneq ($(findstring clean,$(MAKECMDGOALS)), clean)
     include ${NETAPP_OUTPUT_DIR}/include/netapp.inc
endif
ifeq ($(NETAPP_BLUETOOTH),n)
CFLAGS += -DHIDE_NETAPP_BLUETOOTH_MENU
endif
ifeq ($(NETAPP_WIFI),n)
CFLAGS += -DHIDE_NETAPP_WIFI_MENU
endif
CFLAGS += -DNETAPP_SUPPORT $(NETAPP_CFLAGS)
LDFLAGS += $(NETAPP_LDFLAGS)
APP_OBJECTS += network.o

ifneq ($(NXCLIENT_SUPPORT),y)
export NETAPP_NXCLIENT=n
endif
export NETAPP_OUTPUT_DIR:=${NETAPP_OUTPUT_DIR}
export NEXUS_BIN_DIR:=${B_REFSW_OBJ_ROOT}/nexus/bin
export NETAPP_IEEE1905=n
export NETAPP_MIRACAST=n
export NETAPP_SKIP_NEXUS_BUILD=y
export NETAPP_VOICE_RECOGNITION=n
export NETAPP_ZEROCONF=n
export NETAPP_DHCP=n
export NETAPP_DHCPC=n
endif

# helper rule for generating defines for eclipse editor
eclipse:
	@echo [Generate Eclipse Defines...gccdefines_c.h]
	${Q_}$(CC) $(CFLAGS) -dM -E - < /dev/null > $(B_REFSW_OBJ_ROOT)/../gccdefines_c.h


#
# WPA Supplicant Support
#
ifeq ($(WLAN_SUPPORT),y)
include $(BSEAV)/opensource/wpa_supplicant/b_wpa_supplicant_targets.inc
CFLAGS += $(B_WPA_SUPPLICANT_CFLAGS) -DWPA_SUPPLICANT_SUPPORT
LDFLAGS += $(B_WPA_SUPPLICANT_LDFLAGS)
WPA_SUPPLICANT_BUILD += b_wpa_supplicant_target_build
WPA_SUPPLICANT_CLEAN += b_wpa_supplicant_target_clean
WPA_SUPPLICANT_INSTALL = wpa_supplicant_install_files
B_SUPPLICANT_INSTALL_FILES = $(sort $(filter-out %.a,$(B_WPA_SUPPLICANT_BUILD_FILES)))

wpa_supplicant_install_files:
	${Q_}$(foreach file, $(B_SUPPLICANT_INSTALL_FILES),cp -fP $(file) $(STAGING_DIR) &&) true

APP_OBJECTS += wifi.o panel_wps.o
endif

##########
#### Link nexus libraries
##########
LDFLAGS += -lb_os
CFLAGS += $(ATLAS_INCLUDE_DIRS)

C_ONLY_FLAGS := -std=c99 -std=c89 -Wstrict-prototypes

ifeq ($(RF4CE_SUPPORT), y)
C_ONLY_FLAGS += -pedantic
CXXFLAGS += -DRF4CE_NO_SHORT_ENUM
CXXFLAGS += -Wno-pragmas -Wno-enum-compare
#-fpack-struct=4 -fshort-enums
endif

CXXFLAGS += -fno-rtti -fno-exceptions

vpath %.cpp $(VPATH)
vpath %.c $(VPATH)

# Implicit rule for building local app
$(APP): api lib playback_ip_lib nxclient dvrlib pod-driver podlibs dsgcclib cdllib estb-cfglibs dcclib snmp-agent snmp-proxy silib netapp perftools $(PMLIB_BUILD) $(LIBJPEG_BUILD) $(LUA_BUILD) $(WPA_SUPPLICANT_BUILD) wlan $(ATLAS_ODIR) build_date $(addprefix $(ATLAS_ODIR)/,$(APP_OBJECTS))
	@echo [Linking... $(notdir $@)]
	${Q_}$(CXX) -o $@ $(addprefix $(ATLAS_ODIR)/,$(APP_OBJECTS)) $(filter-out -lsettop,$(LDFLAGS)) $(LIBS)

# "order only" prerequisite rule so APP_OBJECTS wait for playback_ip_lib to be built (for make -j option)
$(addprefix $(ATLAS_ODIR)/,$(APP_OBJECTS)): | playback_ip_lib nxclient $(LUA_BUILD) netapp

$(ATLAS_LIB): $(addprefix $(ATLAS_ODIR)/,$(APP_OBJECTS))
	@echo [Linking... $(notdir $@)]
	${Q_}$(RM) $@
	${Q_}$(AR) cr $@ $^
	${Q_}$(RANLIB) $@


%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) $(CXXFLAGS) $(filter-out $(C_ONLY_FLAGS),$(CFLAGS)) -c $< -o $(ATLAS_ODIR)/$@

%.o: %.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) $(CFLAGS) -c $< -o $(ATLAS_ODIR)/$@

$(ATLAS_ODIR)/%.o: %.c
	@echo [Compile... $(notdir $<)]
	${Q_}$(CC) -c -o $@ $< $(CFLAGS) $(C_ONLY_FLAGS)

$(ATLAS_ODIR)/%.o: %.cpp
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) -c -o $@ $(abspath $<) $(filter-out $(C_ONLY_FLAGS),$(CFLAGS)) $(CXXFLAGS)

ifeq ($(NXCLIENT_SUPPORT),y)
.PHONY: nxclient
nxclient: api
	${Q_}$(MAKE) -C $(NXCLIENT_DIR)/build -f Makefile
	${Q_}$(MAKE) -C $(NXCLIENT_DIR)/server -f Makefile
	${Q_}$(CP) -f $(NEXUS_BIN_DIR)/nxserver $(ATLAS_BIN_DIR)
	${Q_}$(CP) -fu $(NEXUS_BIN_DIR)/lib*.so $(ATLAS_BIN_DIR)

nxclient-clean:
	${Q_}$(MAKE) -C $(NXCLIENT_DIR)/build -f Makefile clean
	${Q_}$(MAKE) -C $(NXCLIENT_DIR)/server -f Makefile clean
clean-nxclient: nxclient-clean
else
nxclient:
#intentionally left blank
nxclient-clean:
#intentionally left blank
clean-nxclient:
#intentionally left blank
endif

#################
##### WLAN
#################
ifeq ($(WLAN_SUPPORT),y)
wlan:
	@echo [Build..... wlan]
	${Q_}$(MAKE) -C $(B_WLAN_INSRC_DIR)

clean-wlan:
	@echo [Clean..... wlan]
	${Q_}$(MAKE) -C $(B_WLAN_INSRC_DIR) clean
else
wlan:
clean-wlan:
endif

#################
##### DVR lib
#################
ifeq ($(DVR_LIB_SUPPORT),y)
dvrlib:
	${Q_}$(MAKE) -C $(DVR_LIB_DIR)

clean-dvrlib:
	${Q_}$(MAKE) -C $(DVR_LIB_DIR) clean
else
dvrlib:
clean-dvrlib:
endif

#################
##### Cablecard
#################
ifeq ($(CABLECARD_SUPPORT),y)
include $(BSEAV)/opensource/openssl/b_openssl_targets.inc
#this builds and cleans MPOD driver/lib
podlibs: b_openssl_target_build
	${Q_}$(MAKE) -C $(POD_DIR) DSG_SUPPORT=$(DSG_SUPPORT) ESTB_CFG_SUPPORT=$(ESTB_CFG_SUPPORT) TWOWAY_OOB_CABLECARD_SUPPORT=$(TWOWAY_OOB_CABLECARD_SUPPORT)
	${Q_}$(MAKE) -C $(CP_DIR)/build -f makefile.cplib

clean-podlibs: b_openssl_target_clean_all
	${Q_}$(MAKE) -C $(POD_DIR) clean DSG_SUPPORT=$(DSG_SUPPORT) ESTB_CFG_SUPPORT=$(ESTB_CFG_SUPPORT)
	${Q_}$(MAKE) -C $(CP_DIR)/build -f makefile.cplib clean

pod-driver:
	${Q_}$(MAKE) -C $(POD_DRIVERDIR)

#this builds and cleans pod driver
clean-pod-driver:
	${Q_}$(MAKE) -C $(POD_DRIVERDIR) clean

install-pod-driver:
ifeq ($(SYSTEM),linux)
	@echo "[Install... pod.ko to $(STAGING_DIR)]"
	${Q_}$(MAKE) INSTALL_DIR=$(STAGING_DIR) -C $(POD_DRIVERDIR) install
endif

else #CABLECARD_SUPPORT is not defined, so define empty targets
podlibs:
#intentionally left empty

clean-podlibs:
#intentionally left empty

pod-driver:
#intentionally left empty

clean-pod-driver:
#intentionally left empty

install-pod-driver:
#intentionally left empty
endif # CABLECARD_SUPPORT

#################
##### DSGCC
#################
ifeq ($(DSG_SUPPORT),y)
# This untar source ball for DSG-CC and estbsnmpagent
untar_estb:
	${Q_}if [ -d $(DSG_DIR)/StbHostApps ];then echo ""; else cd $(DSG_DIR);tar fxvz stb_src.tgz;fi
# This builds and cleans the dsgcc api
dsgcclib: untar_estb
	${Q_}$(RM) -f ${B_REFSW_OBJ_ROOT}/BSEAV/cable/docsis/estb/StbHostApps/DsgCC/StbApp_DsgCC/linux/bcm/dsgcclib.a
	export CROSSDEV_ROOT=$(TOOLCHAIN_DIR);\
	cd $(DSG_DIR)/StbHostApps/DsgCC/build; \
	env MAKE="${MAKE}" bash build dsgcc pvnet applib $(CC_FLAG) $(CDL_CMDS);\
	cd $(BSEAV)/app/atlas/build
clean-dsgcclib: untar_estb
	${Q_}$(RM) -f ${B_REFSW_OBJ_ROOT}/BSEAV/cable/docsis/estb/StbHostApps/DsgCC/StbApp_DsgCC/linux/bcm/dsgcclib.a
	export CROSSDEV_ROOT=$(TOOLCHAIN_DIR);\
	cd $(DSG_DIR)/StbHostApps/DsgCC/build; \
	bash build dsgcc clean;\
	cd $(BSEAV)/app/atlas/build
else # DSG_SUPPORT is not defined, so define empty targets
dsgcclib:
#intentionally left empty
clean-dsgcclib:
#intentionally left empty
endif # DSG_SUPPORT


#################
##### CDL_SUPPORT
#################
ifeq ($(CDL_SUPPORT),y)
API_SUPPORT = NEXUS
untar_estb_for_cdl:
	${Q_}if [ -d $(DSG_DIR)/StbHostApps ];then echo ""; else cd $(DSG_DIR);tar fxvz stb_src.tgz;fi
cdllib:untar_estb_for_cdl
	${Q_}$(MAKE) -C $(CDL_LIB_DIR)  DSG_SUPPORT=$(DSG_SUPPORT) API_SUPPORT=$(API_SUPPORT) CABLECARD_SUPPORT=$(CABLECARD_SUPPORT) CDL_SUPPORT_ENABLE_SIGNED_IMAGE=$(CDL_SUPPORT_ENABLE_SIGNED_IMAGE) DSG_DIR=$(DSG_DIR)
	${Q_}$(MAKE) -C $(DSMCC_LIB_DIR)  DSG_SUPPORT=$(DSG_SUPPORT) CABLECARD_SUPPORT=$(CABLECARD_SUPPORT)

clean-cdllib:untar_estb_for_cdl
	${Q_}$(MAKE) -C $(CDL_LIB_DIR)  DSG_SUPPORT=$(DSG_SUPPORT) API_SUPPORT=$(API_SUPPORT) CABLECARD_SUPPORT=$(CABLECARD_SUPPORT) DSG_DIR=$(DSG_DIR) clean
	${Q_}$(MAKE) -C $(DSMCC_LIB_DIR)  DSG_SUPPORT=$(DSG_SUPPORT) CABLECARD_SUPPORT=$(CABLECARD_SUPPORT) clean

else
cdllib:
clean-cdllib:
endif

#################
##### ESTB_CFG_SUPPORT
#################
ifeq ($(ESTB_CFG_SUPPORT),y)
estb-cfglibs:
	${Q_}$(MAKE) -C $(ESTB_CFG_LIB_DIR) DSG_SUPPORT=$(DSG_SUPPORT)
clean-estb-cfglibs:
	${Q_}$(MAKE) -C $(ESTB_CFG_LIB_DIR) DSG_SUPPORT=$(DSG_SUPPORT) clean
else # ESTB_CFG_SUPPORT is not defined, so define empty targets
estb-cfglibs:
#intentionally left empty
clean-estb-cfglibs:
#intentionally left empty
endif # ESTB_CFG_SUPPORT

#################
##### DCC_SUPPORT
#################
ifeq ($(DCC_SUPPORT),y)
dcclib:
	${Q_}$(MAKE) -C $(DCC_LIB_DIR) INTEGRATED_BWIN=y
	${Q_}$(CP) -fr $(NEXUS_BIN_DIR)/libb_dcc.so $(ATLAS_BIN_DIR)
clean-dcclib:
	${Q_}$(MAKE) -C $(DCC_LIB_DIR) INTEGRATED_BWIN=y clean
else # DCC_SUPPORT is not defined, so define empty targets
dcclib:
#intentionally left empty
clean-dcclib:
#intentionally left empty
endif # DCC_SUPPORT

#######################
##### ESTB_SNMP_SUPPORT
#######################
ifeq ($(ESTB_SNMP_SUPPORT),y)
snmp-agent:
	export CROSSDEV_ROOT=$(TOOLCHAIN_DIR); \
	cd $(DSG_DIR)/StbHostApps/EstbSnmpAgent/build; \
	bash ./build; \
	cd -
clean-snmp-agent:
	export CROSSDEV_ROOT=$(TOOLCHAIN_DIR); \
	cd $(DSG_DIR)/StbHostApps/EstbSnmpAgent/build; \
	bash ./build clean; \
	cd -
snmp-proxy:
	cd $(SNMP_DIR); \
	if [ ! -e /tmp/net-snmp-5.0.9.tar.gz ]; then \
		wget http://sourceforge.net/projects/net-snmp/files/OldFiles/net-snmp-5.0.x/5.0.9/net-snmp-5.0.9.tar.gz -P /tmp; \
	fi; \
	if [ -e /tmp/net-snmp-5.0.9.tar.gz ] && [ ! -e Makefile.top ]; then \
		tar zxvf /tmp/net-snmp-5.0.9.tar.gz --strip-components=1; \
		patch -p1 < snmp.patch; \
	fi; \
	if [ -e Makefile ]; then \
		${Q_}$(MAKE) depend; \
		${Q_}$(MAKE); \
	else \
		bash $(SNMPCONFIG); \
		${Q_}$(MAKE) depend; \
		${Q_}$(MAKE); \
	fi; \
	cd -
clean-snmp-proxy:
	cd $(SNMP_DIR); \
	if [ -e Makefile ]; then \
		${Q_}$(MAKE) distclean; \
		$(RM) -fr Makefile.bak; \
		$(RM) -fr agent/Makefile.bak; \
		$(RM) -fr agent/helpers/Makefile.bak; \
		$(RM) -fr agent/mibgroup/Makefile.bak; \
		$(RM) -fr agent/mibgroup/ocmib/.libs; \
		$(RM) -fr agent/mibgroup/ocmib/home_network/.libs; \
		$(RM) -fr agent/mibgroup/ocmib/stb_host/.libs; \
		$(RM) -fr include/net-snmp/library/snmpv3-security-includes.h; \
		$(RM) -fr snmplib/Makefile.bak; \
	fi; \
	cd -
else
snmp-agent:
#intentionally left empty
clean-snmp-agent:
#intentionally left empty
snmp-proxy:
#intentionally left empty
clean-snmp-proxy:
#intentionally left empty
endif

ifeq ($(SI_SUPPORT),y)
# This builds and cleans the si libs
silib:
	${Q_}$(MAKE) -C $(SI_DIR)/build TOOLCHAIN_DIR=$(TOOLCHAIN_DIR)
clean-silib:
	${Q_}$(MAKE) -C $(SI_DIR)/build TOOLCHAIN_DIR=$(TOOLCHAIN_DIR) clean

else # SI_SUPPORT is not defined, so define empty targets
silib:
#intentionally left empty

clean-silib:
#intentionally left empty
endif # SI_SUPPORT

ifeq ($(PERFTOOLS_SUPPORT),y)
perftools: api
	${Q_}$(MAKE) -C $(BSYSPERF_DIR)
	${Q_}$(MAKE) -C $(BMEMPERF_DIR)
clean-perftools:
	${Q_}$(MAKE) -C $(BSYSPERF_DIR) clean
	${Q_}$(MAKE) -C $(BMEMPERF_DIR) clean
else
perftools:
#intentionally left empty
clean-perftools:
#intentionally left empty
endif

ifeq ($(NETAPP_SUPPORT),y)
# This builds and cleans the netapp libs
${NETAPP_OUTPUT_DIR}/include/netapp.inc :
ifneq (${SERVER_ALIVE},)
ifeq ($(wildcard ${NETAPP_DIR}),)
	@echo "Fetching NetApp-API: [$(FETCH_NETAPP_CMD) Version:$(B_NETAPP_NAME)]"
	@-cd $(BSEAV)/..; $(FETCH_NETAPP_CMD)
endif
	@-cd $(BSEAV)/../trellis; git fetch origin; git checkout $(B_NETAPP_NAME); git pull origin $(B_NETAPP_NAME)
endif
	@if [ ! -d ${NETAPP_DIR} ] ; then \
		echo "======================================================================================================="; \
		echo "               NETAPP-API Missing!";\
		echo "Please see: http://confluence.broadcom.com/display/NetApp/Fetching+and+Building+The+Code";\
		echo "=======================================================================================================";\
		echo "";\
		exit -1;\
	fi
	${Q_}$(MAKE) -C $(NETAPP_DIR) install

ifneq ($(NETAPP_BUILD_EXAMPLES),n)
netapp : api
	${Q_}$(MAKE) -C $(NETAPP_DIR) examples
	${Q_}$(CP) ${NETAPP_OUTPUT_DIR}/bin/netapp_target_prep.sh ${B_REFSW_OBJ_ROOT}/BSEAV/bin
else
netapp :
	${Q_}$(MAKE) -C $(NETAPP_DIR) install
	${Q_}$(CP) ${NETAPP_OUTPUT_DIR}/bin/netapp_target_prep.sh ${B_REFSW_OBJ_ROOT}/BSEAV/bin
endif
clean-netapp:
	${Q_}$(MAKE) -C $(NETAPP_DIR) clean
	${Q_}rm -f ${B_REFSW_OBJ_ROOT}/BSEAV/bin/netapp_target_prep.sh
dist-clean-netapp:
	${Q_}$(MAKE) -C $(NETAPP_DIR) distclean
	${Q_}rm -f ${B_REFSW_OBJ_ROOT}/BSEAV/bin/netapp_target_prep.sh

else # NETAPP_SUPPORT is not defined, so define empty targets
netapp:
#intentionally left empty
clean-netapp:
#intentionally left empty
dist-clean-netapp:
#intentionally left empty
endif # SI_SUPPORT

###############
######### Playback IP and Releated libraries
###############
ifeq ($(PLAYBACK_IP_SUPPORT),y)
include $(NEXUS_TOP)/lib/bip/build/b_bip_targets.inc
b_bip_target_build: api
B_BIP_MAKEFLAGS += INSTALL_DIR=$(ATLAS_BIN_DIR)
playback_ip_lib: b_bip_target_build

B_PBIP_INSTALL_FILES = $(sort $(filter-out %.a,$(B_BIP_BUILD_FILES)))
playback_ip_lib_install: api
	${Q_}$(foreach file, $(B_PBIP_INSTALL_FILES),cp -fP $(file) $(STAGING_DIR);)

playback_ip_lib_clean: b_bip_target_clean

playback_ip_lib_min_clean: playback_ip_lib_clean

else #PLAYBACK_IP_SUPPORT is not defined, so define empty targets
playback_ip_lib:
#intentionally left empty

playback_ip_lib_install:
#intentionally left empty

playback_ip_lib_clean:
#intentionally left empty

playback_ip_lib_min_clean:
	@echo [clean.... bip (not openssl)]

clean-bip: playback_ip_lib_min_clean

endif # PLAYBACK_IP_SUPPORT

#Only Cleans Playback-IP library not SSL, only
min-clean: playback_ip_lib_min_clean clean-atlas api-clean

#
# This is the main installation rule.
# It will build the binaries and utilities, and then
# perform the actual installation procedure.
#
install:  bin_install $(STAGING_DIR)
	${Q_}$(MAKE) install-binaries
	${Q_}$(MAKE) make-tarball

#
# Install drivers, libraries and apps
#
.PHONY: install-binaries
install-binaries: api-install playback_ip_lib_install install-config-files install-pod-driver install-cable-files install-snmp-files install-netapp-files install-zigbee-files install-wlan-files $(WPA_SUPPLICANT_INSTALL)

#
# STAGING_DIR is phony because we always want to remove it and
# recreate it.
#
.PHONY: $(STAGING_DIR)
$(STAGING_DIR):
	${Q_}$(RM) -rf $(STAGING_DIR)
	${Q_}${MKDIR} $(STAGING_DIR)

#
# Copy everything into temporary STAGING_DIR and tar it up.
# The final tarball is left in ATLAS_BIN_DIR.
#

make-tarball: install-support-files
	@echo [Build..... tarball of $(STAGING_DIR)]
	${Q_}(cd $(STAGING_DIR);tar -pzckf $(ATLAS_BIN_DIR)/${ATLAS_TAR_BALL} .)
	${Q_}$(RM) -rf $(STAGING_DIR)
	@echo ""
	@echo "*********************************************"
	@echo "* Installation tarball created in $(ATLAS_BIN_DIR)."
	@echo "*"
	@echo "* To complete installation perform the"
	@echo "* following operations on the settop box:"
	@echo "*"
	@echo "* gunzip -c ${ATLAS_TAR_BALL} | tar xvpf -"
	@echo "* start atlas"
	@echo "*********************************************"
	@echo ""

#
#
.PHONY: install-support-files
install-support-files:
	@echo [Install... scripts to $(STAGING_DIR)]
	${Q_}$(CP) start $(STAGING_DIR)
	${Q_}chmod +x $(STAGING_DIR)/start
ifeq ($(NEXUS_MODE),proxy)
	${Q_}$(CP) $(NEXUS_BIN_DIR)/nexus.ko $(STAGING_DIR)
endif
	${Q_}$(CP) $(NEXUS_BIN_DIR)/logger $(STAGING_DIR)
	${Q_}$(CP) $(NEXUS_BIN_DIR)/nexus $(STAGING_DIR)
ifeq ($(NXCLIENT_SUPPORT),y)
	${Q_}$(CP) $(NEXUS_BIN_DIR)/nxserver $(STAGING_DIR)
	${Q_}$(CP) $(NEXUS_BIN_DIR)/lib*.so $(STAGING_DIR)
endif
	${Q_}chmod +x $(STAGING_DIR)/nexus
ifeq ($(CABLECARD_SUPPORT),y)
	${Q_}$(CP) -fr $(NEXUS_BIN_DIR)/libb_mpod.so $(STAGING_DIR)
endif
ifeq ($(DVR_LIB_SUPPORT),y)
	${Q_}$(CP) -fr $(NEXUS_BIN_DIR)/libb_dvr.so $(STAGING_DIR)
endif
ifeq ($(CDL_SUPPORT),y)
	${Q_}$(CP) -fr $(NEXUS_BIN_DIR)/libb_cdl.so $(STAGING_DIR)
	${Q_}$(CP) -fr $(NEXUS_BIN_DIR)/libb_dsmcc.so $(STAGING_DIR)
endif
ifeq ($(ESTB_CFG_SUPPORT),y)
	${Q_}$(CP) -fr $(NEXUS_BIN_DIR)/libb_estb_cfg.so $(STAGING_DIR)
endif
ifeq ($(DCC_SUPPORT),y)
	${Q_}$(CP) -fr $(NEXUS_BIN_DIR)/libb_dcc.so $(STAGING_DIR)
endif
	${Q_}$(CP) $(APP) $(STAGING_DIR)

	@echo [Install... fonts to $(STAGING_DIR)]
	${Q_}${MKDIR} $(STAGING_DIR)/fonts
	${Q_}$(CP) $(PRERENDERED_FONTS) $(STAGING_DIR)/fonts
ifeq ($(DCC_SUPPORT),y)
	${Q_}${MKDIR} $(STAGING_DIR)/fonts/cc_fonts
	${Q_}$(CP) $(PRERENDERED_CC_FONTS) $(STAGING_DIR)/fonts/cc_fonts
endif

ifeq ($(WLAN_SUPPORT),y)
# Install all files needed for wlan driver
install-wlan-files:
	@echo  [Install... wlan to $(STAGING_DIR)]
	${Q_}$(CP) $(ATLAS_BIN_DIR)/wl $(STAGING_DIR)
	${Q_}$(CP) $(ATLAS_BIN_DIR)/wlinstall $(STAGING_DIR)
	${Q_}$(CP) $(ATLAS_BIN_DIR)/wl.ko $(STAGING_DIR)
	${Q_}$(CP) $(ATLAS_BIN_DIR)/libnl-3.so* $(STAGING_DIR)
	${Q_}$(CP) $(ATLAS_BIN_DIR)/libnl-genl-3.so* $(STAGING_DIR)
	${Q_}$(CP)  ../scripts/atlas_wlan_prep.sh $(STAGING_DIR)
	${Q_}chmod 755 $(STAGING_DIR)/atlas_wlan_prep.sh
	${Q_}$(CP)  ../scripts/udhcpc.script $(STAGING_DIR)
	${Q_}chmod 755 $(STAGING_DIR)/udhcpc.script
	${Q_}$(CP) $(ATLAS_BIN_DIR)/bcm97271*.txt $(STAGING_DIR)
else
install-wlan-files:
#intentionally left empty
endif

ifeq ($(RF4CE_SUPPORT),y)
# Install all files needed to run rf4ce
install-zigbee-files:
	@echo  [Install... rf4ce to $(STAGING_DIR)]
	${Q_}$(CP) $(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide/atlas_zigbee_prep.sh $(STAGING_DIR)
	${Q_}$(CP) $(BSEAV)/lib/zigbee/broadbee_mailbox_host/projects/SoC_mailboxHostSide/stack_binary/broadbee_zrc11_target.bin $(STAGING_DIR)
else
install-zigbee-files:
#intentionally left empty
endif

ifeq ($(CABLE_SUPPORT),y)
# Install all files needed to run cable platforms
install-cable-files:
	@echo  [Install... cable to $(STAGING_DIR)]
	${Q_}$(CP) ../channelmap/cable.channels.xml $(STAGING_DIR)/channels.xml
ifeq ($(findstring $(NEXUS_PLATFORM), 97435), $(NEXUS_PLATFORM))
ifeq ($(findstring $(DOCSIS_CHIP), 3384), $(DOCSIS_CHIP))
	${Q_}$(CP) $(BSEAV)/cable/bin/$(NEXUS_PLATFORM)/33843/* $(STAGING_DIR)
	${Q_}$(CP) $(BSEAV)/cable/bin/$(NEXUS_PLATFORM)/rc.user $(STAGING_DIR)
	${Q_}$(CP) $(BSEAV)/cable/bin/$(NEXUS_PLATFORM)/estbconfigsettings.bin $(STAGING_DIR)
	${Q_}$(CP) $(BSEAV)/cable/bin/$(NEXUS_PLATFORM)/rnonvolhost $(STAGING_DIR)
else
	${Q_}$(CP) -r $(BSEAV)/cable/bin/$(NEXUS_PLATFORM)/* $(STAGING_DIR)
	${Q_}$(RM) -fr $(STAGING_DIR)/33843
endif
else
	${Q_}$(CP) -r $(BSEAV)/cable/bin/$(NEXUS_PLATFORM)/* $(STAGING_DIR)
endif

else
install-cable-files:
#intentionally left empty
endif

ifeq ($(NEXUS_USE_3390_VMS),y)
	@echo  [Install... cable for 93390 to $(STAGING_DIR)]
	${Q_}$(CP) ../channelmap/cable.channels.xml $(STAGING_DIR)/channels.xml
	${Q_}$(CP) -r $(BSEAV)/cable/bin/93390/* $(STAGING_DIR)
endif

ifeq ($(NEXUS_USE_7252S_GW), y)
	@echo  [Install... cable for 97252S_GW to $(STAGING_DIR)]
	${Q_}$(CP) ../channelmap/cable.channels.xml $(STAGING_DIR)/channels.xml
	${Q_}$(CP) -r $(BSEAV)/cable/bin/97252sgw/* $(STAGING_DIR)
endif

ifeq ($(ESTB_SNMP_SUPPORT),y)
install-snmp-files:
	${Q_}$(CP) -fr $(DSG_DIR)/StbHostApps/EstbSnmpAgent/build/estbsnmpagent $(STAGING_DIR)
	${Q_}$(CP) -fr $(SNMP_DIR)/agent/libsnmpd.so $(STAGING_DIR)
	${Q_}$(CP) -fr $(SNMP_DIR)/agent/.libs/libnetsnmpagent.so* $(STAGING_DIR)
	${Q_}$(CP) -fr $(SNMP_DIR)/agent/.libs/libnetsnmpmibs.so* $(STAGING_DIR)
	${Q_}$(CP) -fr $(SNMP_DIR)/agent/helpers/.libs/libnetsnmphelpers.so* $(STAGING_DIR)
	${Q_}$(CP) -fr $(SNMP_DIR)/snmplib/.libs/libnetsnmp.so* $(STAGING_DIR)
	${Q_}$(CP) -fr $(BSEAV)/cable/bin/mibs $(STAGING_DIR)
else
install-snmp-files:
#intentionally left empty
endif

ifeq ($(NETAPP_SUPPORT),y)
# Install all files needed to run WiFi and Bluetooth platforms
install-netapp-files:
	@echo  [Install... netapp to $(STAGING_DIR)]
	${Q_}$(MKDIR) -p $(STAGING_DIR)/lib
	${Q_}$(MKDIR) -p $(STAGING_DIR)/bin
	${Q_}$(CP) -fr $(ATLAS_BIN_DIR)/bin/netapp_target_prep.sh $(STAGING_DIR)
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/dhd ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/dhd $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/bcmdl ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/bcmdl $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/bsa_server ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/bsa_server $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/wpa_supplicant ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/wpa_supplicant $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/wpa_supplicant.conf ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/wpa_supplicant.conf $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/NetAppTester ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/NetAppTester $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/app_manager ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/app_manager $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/app_hh ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/app_hh $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/app_ble ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/app_ble $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/btdl ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/btdl $(STAGING_DIR)/bin;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/bin/txpower.txt ];then $(CP) -fr $(ATLAS_BIN_DIR)/bin/txpower.txt $(STAGING_DIR)/bin;fi
	${Q_}$(CP) -fr $(ATLAS_BIN_DIR)/bin/netapp_target_prep.sh $(STAGING_DIR)
	${Q_}$(CP) -fr $(ATLAS_BIN_DIR)/lib/libnetapp.so $(STAGING_DIR)/lib
	${Q_}$(CP) -fr $(ATLAS_BIN_DIR)/lib/libsqlite3.so* $(STAGING_DIR)/lib
	${Q_}$(CP) -fr $(ATLAS_BIN_DIR)/lib/libjson.so* $(STAGING_DIR)/lib
	${Q_}$(CP) -fr $(ATLAS_BIN_DIR)/lib/libudev.so* $(STAGING_DIR)/lib
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libbwl.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libbwl.so $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libwpsenr.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libwpsenr.so $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libwpscom.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libwpscom.so $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libwpsapi.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libwpsapi.so $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libwpsap.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libwpsap.so $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libwpscli.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libwpscli.so $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libbsa.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libbsa.so $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libcurl.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libcurl.so* $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libavahi-core.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libavahi-core.so* $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libavahi-common.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libavahi-common.so* $(STAGING_DIR)/lib;fi
	${Q_}if [ -e $(ATLAS_BIN_DIR)/lib/libnl-3.so ];then $(CP) -fr $(ATLAS_BIN_DIR)/lib/libnl*-3.so* $(STAGING_DIR)/lib;fi
	${Q_}$(CP) -fr $(ATLAS_BIN_DIR)/lib/modules $(STAGING_DIR)/lib
	${Q_}$(CP) -fr $(ATLAS_BIN_DIR)/firmware $(STAGING_DIR)
else
install-netapp-files:
#intentionally left empty
endif

####
## install Config Files
####
install-config-files:
	${Q_}cd ../; \
		$(CP) -r images/bcmlogo.png images scripts $(STAGING_DIR)
	${Q_}$(CP) -r ../scripts/atlas.cfg $(STAGING_DIR)/atlas.cfg
	${Q_}if [ "$(NEXUS_USE_7252S_GW)" = "y" ]; then \
		$(CP) ../channelmap/cable.channels.xml $(STAGING_DIR)/channels.xml; \
	elif [ -f ../channelmap/$(NEXUS_PLATFORM).channels.xml ] ; then  \
		$(CP) ../channelmap/$(NEXUS_PLATFORM).channels.xml $(STAGING_DIR)/channels.xml; \
	else  \
		if [ "$(CABLE_SUPPORT)" != "y" ]; then \
		$(CP) ../channelmap/channels.xml $(STAGING_DIR)/channels.xml; \
		fi; \
	fi;
	${Q_}if [ -f ../channelmap/udpUrl.xml ] ; then  \
		$(CP) ../channelmap/udpUrl.xml $(STAGING_DIR)/udpUrl.xml; \
	fi;
	${Q_}cd $(STAGING_DIR); chmod 664 atlas.cfg

bin-install-config-files:
# Copy over config files so we can run without doing a make install
# you can run this manually with "make bin-install-config-files"
	@echo  [Install... config files to $(ATLAS_BIN_DIR)]
	${Q_}cd ../; \
		$(CP) -r images/bcmlogo.png images scripts $(ATLAS_BIN_DIR)
	${Q_}$(CP) -r ../scripts/atlas.cfg $(ATLAS_BIN_DIR)/atlas.cfg
	${Q_}if [ "$(NEXUS_USE_7252S_GW)" = "y" ]; then \
		$(CP) ../channelmap/cable.channels.xml $(STAGING_DIR)/channels.xml; \
	elif [ -f ../channelmap/$(NEXUS_PLATFORM).channels.xml ] ; then  \
		$(CP) ../channelmap/$(NEXUS_PLATFORM).channels.xml $(ATLAS_BIN_DIR)/channels.xml; \
	else  \
		if [ "$(CABLE_SUPPORT)" != "y" ]; then \
		$(CP) ../channelmap/channels.xml $(ATLAS_BIN_DIR)/channels.xml; \
		fi; \
	fi;
	${Q_}if [ -f ../channelmap/udpUrl.xml ] ; then  \
		$(CP) ../channelmap/udpUrl.xml $(ATLAS_BIN_DIR)/udpUrl.xml; \
	fi;
	${Q_}cd $(ATLAS_BIN_DIR); chmod 664 atlas.cfg
	@echo [Install... fonts to $(ATLAS_BIN_DIR)]
	${Q_}${MKDIR} $(ATLAS_BIN_DIR)/fonts
	${Q_}$(CP) $(PRERENDERED_FONTS) $(ATLAS_BIN_DIR)/fonts
ifeq ($(DCC_SUPPORT),y)
	${Q_}${MKDIR} $(ATLAS_BIN_DIR)/fonts/cc_fonts
	${Q_}$(CP) $(PRERENDERED_CC_FONTS) $(ATLAS_BIN_DIR)/fonts/cc_fonts
endif

##########
##### Rule for help
##########
.PHONY: help
help:
	@echo "Atlas Make Options"
	@echo
	@echo "all             - build atlas and libraries"
	@echo "lib             - build bwin, mlibs and bwidgets"
	@echo "api             - build nexus"
	@echo "nxclient        - build nexus, nxclient and nxserver"
	@echo "perftools       - build bsysperf and bmemperf tools"
	@echo "clean           - clean atlas, nexus and libs"
	@echo "                  (skip: zlib/jpeg/png/openssl/netapp/libnl/bwl/wpa_supplicant)"
	@echo "clean-atlas     - clean atlas"
	@echo "clean-perftools - clean bsysperf and bmemperf tools"
	@echo "clean-api       - clean nexus"
	@echo "clean-nxclient  - clean nxclient and nxserver"
	@echo "clean-lib       - clean bwin, mlibs and bwidgets"
	@echo "clean-bip       - clean bip (skip: openssl)"
	@echo "dist-clean      - clean atlas and libraries"

-include $(ATLAS_ODIR)/*.d
