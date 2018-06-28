###############################################################################
#   Copyright (C) 2018 Broadcom.
#   The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to
#   the terms and conditions of a separate, written license agreement executed
#   between you and Broadcom (an "Authorized License").  Except as set forth in
#   an Authorized License, Broadcom grants no license (express or implied),
#   right to use, or waiver of any kind with respect to the Software, and
#   Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#   THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#   IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization,
#   constitutes the valuable trade secrets of Broadcom, and you shall use all
#   reasonable efforts to protect the confidentiality thereof, and to use this
#   information only in connection with your use of Broadcom integrated circuit
#   products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#   OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#   RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#   IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#   A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#   THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#   OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#   INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#   RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#   EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#   WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#   FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################
.PHONY: all clean

#B_THIS_DIR:=rockford/dms/bmon
B_THIS_DIR:=BSEAV/tools/bmon/dashboard
SHELL:=/bin/bash

ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus
rockford ?= $(abspath ${NEXUS_TOP}/../rockford)
BSEAV ?= $(abspath ${NEXUS_TOP}/../BSEAV)
BMON_SRC_DIR = $(abspath ${BSEAV}/../$(B_THIS_DIR))

ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mipsel-linux
export B_REFSW_ARCH
endif

B_REFSW_DEBUG ?= y
ifeq ($(B_REFSW_DEBUG),y)
DEBUG_SUFFIX=debug
else
DEBUG_SUFFIX=release
endif

ifeq ($(NEXUS_PLATFORM),)
NEXUS_PLATFORM=$(PLATFORM)
endif

$(if ${NEXUS_PLATFORM},,$(error NEXUS_PLATFORM is not defined.))

# include cross-compiler definitions
include ${NEXUS_TOP}/platforms/common/build/platform_app.inc
include ${NEXUS_TOP}/platforms/common/build/nexus_platforms.inc

$(if ${NEXUS_BIN_DIR},,$(error NEXUS_BIN_DIR is not defined.))
$(if ${B_REFSW_OBJ_ROOT},,$(error B_REFSW_OBJ_ROOT is not defined.))

OBJ_BMON_DIR := ${B_REFSW_OBJ_ROOT}/rockford/dms/bmon/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)
OBJ_BMON_DIR := ${B_REFSW_OBJ_ROOT}/BSEAV/tools/bmon/dashboard/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)

# DESTDIR is where you want the shared object libraries to end up, default to NEXUS_BIN_DIR
DESTDIR ?= ${NEXUS_BIN_DIR}

BIT64 = $(findstring aarch64,${COMPILER})
WEB_FILES = power.html power.js bmon-dash-1.html wifi.html wificore.js wifiapp.js wifivars.js ampdu.js wifichanstats.js plugin-monitor.html
all: ${DESTDIR} chartjs-plugin-annotation $(WEB_FILES) boa nxclient_apps specA_html rest_server websocket plugin_cpu plugin_power plugin_wifi plugin_hdmi_output plugin_network plugin_screenshot dashboard_files
	$(Q_)if [ ! -d $(OBJ_BMON_DIR) ]; then \
		mkdir -p $(OBJ_BMON_DIR); \
	fi

	$(Q_)if [ ! -d $(DESTDIR) ]; then mkdir -p $(DESTDIR); fi;

dashboard_files: ${DESTDIR} boa
	$(Q_)$(MKDIR) -p $(DESTDIR)/img
	$(Q_)echo "[Install... (web files)]"
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/index.html $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/power.html $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/power.js $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/walktreestructure.js $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/img/power.png $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/wifi.html $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/wificore.js $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/wifiapp.js $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/wifivars.js $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/ampdu.js $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/wifichanstats.js $(DESTDIR)
	$(Q_)$(CP) -df $(BMON_SRC_DIR)/plugin-monitor.html $(DESTDIR)
	@(cd $(BMON_SRC_DIR) && tar cBf - img) | (cd $(DESTDIR) && tar xBf -)
	@(cd $(BMON_SRC_DIR) && tar cBf - css) | (cd $(DESTDIR) && tar xBf -)

${DESTDIR}:
	@mkdir -p $@

nxclient_apps:
	$(Q_)echo "[Install... $@]"
	make -C $(NEXUS_TOP)/../nexus/nxclient/apps -j32 play screenshot

rest_server:
	$(Q_)echo "[Install... $@]"
	make -C $(BMON_SRC_DIR)/../rest

websocket:
	$(Q_)echo "[Install... $@]"
	make -C $(BMON_SRC_DIR)/../websocket

boa:
	$(Q_)echo "[Install... $@]"
	make -C $(BMON_SRC_DIR)/../../../opensource/boa

plugin_cpu:
	$(Q_)echo "[Install... $@]"
	make -C $(BMON_SRC_DIR)/../plugins/cpu

plugin_network:
	$(Q_)echo "[Install... $@]"
	make -C $(BMON_SRC_DIR)/../plugins/network

plugin_power:
	$(Q_)echo "[Install... $@]"
	make -C $(BMON_SRC_DIR)/../plugins/power

plugin_wifi:
	$(Q_)echo "[Install... $@]"
	make -C $(BMON_SRC_DIR)/../plugins/wifi

plugin_hdmi_output:
	$(Q_)echo "[Install... $@]"
	make -C $(BMON_SRC_DIR)/../plugins/hdmi_output

plugin_screenshot:
	$(Q_)echo "[Install... $@]"
	make -C $(BMON_SRC_DIR)/../plugins/screenshot

chartjs-plugin-annotation:
	$(Q_)echo "[Install... $@]"
	make -C $(NEXUS_TOP)/../BSEAV/opensource/chartjs-plugin-annotation

specA_html:
	$(Q_)echo "[Install... $@]"
	@(cd $(BMON_SRC_DIR) && tar cBf - specA* ) | (cd $(DESTDIR) && tar xBf -)

clean:
	$(Q_)if [ -e $(DESTDIR)/power.html ]; then rm -f $(DESTDIR)/power.html ; fi;
	$(Q_)if [ -e $(DESTDIR)/power.js ]; then rm -f $(DESTDIR)/power.js ; fi;
	$(Q_)if [ -e $(DESTDIR)/walktreestructure.js ]; then rm -f $(DESTDIR)/walktreestructure.js ; fi;
	$(Q_)if [ -e $(DESTDIR)/wifi.html ]; then rm -f $(DESTDIR)/wifi.html ; fi;
	$(Q_)if [ -e $(DESTDIR)/wificore.js ]; then rm -f $(DESTDIR)/wificore.js ; fi;
	$(Q_)if [ -e $(DESTDIR)/wifiapp.js ]; then rm -f $(DESTDIR)/wifiapp.js ; fi;
	$(Q_)if [ -e $(DESTDIR)/wifivars.js ]; then rm -f $(DESTDIR)/wifivars.js ; fi;
	$(Q_)if [ -e $(DESTDIR)/ampdu.js ]; then rm -f $(DESTDIR)/ampdu.js ; fi;
	$(Q_)if [ -e $(DESTDIR)/wifichanstats.js ]; then rm -f $(DESTDIR)/wifichanstats.js ; fi;
	$(Q_)if [ -e $(DESTDIR)/bmon-dash-1.html ]; then rm -f $(DESTDIR)/bmon-dash-1.html ; fi;
	$(Q_)if [ -e $(DESTDIR)/img ]; then rm -rf $(DESTDIR)/img ; fi;
	$(Q_)if [ -e $(DESTDIR)/css ]; then rm -rf $(DESTDIR)/css ; fi;
	$(Q_)if [ -e $(DESTDIR)/specA.html ]; then rm -rf $(DESTDIR)/specA* ; fi;
