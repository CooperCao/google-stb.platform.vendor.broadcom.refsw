#
# GNUmakefile for dhd/exe
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
#
# <<Broadcom-WL-IPTag/Proprietary:>>
#
# $Id: GNUmakefile,v 1.27 2010-09-13 14:20:34 $

WLAN_ComponentsInUse := bcmwifi
ifeq ($(CONFIG_DHDAP),y)
    export WLAN_TreeBaseA=$(abspath ../../..)
endif
include ../../makefiles/WLAN_Common.mk
SRCBASE := $(WLAN_SrcBaseR)

UNAME = $(shell uname)

#-----------------------------------------------------------------
# Windows build
# 1) windows, don't include Makerules due to all: conflict

ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)

include ../../GNUmakefile.inc

else # UNAME

# 2) not windows, need to include first to pick up TARGETENV dependent vars
include $(SRCBASE)/Makerules

# GNU make function to do compilation and dependency generation
# in one step.
define CompileAndMakedep
$(strip $1 -c -MMD -MF $(@D)/.$(@F).depend -MP $2)
endef

#ifdef BCMWPA2
CFLAGS += -DBCMWPA2
#endif
# Always include pktgen commands in the application
CFLAGS += -DSDTEST
#ifndef OEM_ANDROID
CFLAGS += -DBCMSPI
#endif

#ifdef WLCNT
CFLAGS += -DWLCNT
#endif

CFLAGS += -DBCMDONGLEHOST

CFLAGS += -DPCIE_DMA_LOAD #DMA upload/download for 4345/4350

vpath %.c $(SRCBASE)/shared $(SRCBASE)/wl/exe $(WLAN_StdSrcDirsR) $(WLAN_ComponentSrcDirsR) $(SRCBASE)/dhd/sys

ifeq ($(REPORT_FATAL_TIMEOUTS), 1)
CFLAGS += -DREPORT_FATAL_TIMEOUTS
endif

#-----------------------------------------------------------------
# Linux build
#
# This should be one of values recognized in src/Makerules
ifneq ($(findstring $(TARGETENV), "linux linuxmips linuxmips_be linuxarm linuxarm_le android android_ndk_r6b freebsd freebsd_be"),)

# use nl80211 for driver access (ioctl replacement)
ifdef NL80211
CFLAGS += -DNL80211
endif

CFLAGS += -DWLPFN -DLINUX -g -I$(SRCBASE)/wl/exe
CFLAGS += -DSHOW_LOGTRACE

ifneq (,$(findstring freebsd, $(TARGETENV)))
    CC := $(CROSS_COMPILE)gcc
    # CFLAGS += -D__FreeBSD__
    CFLAGS += -I$(FREEBSD_DIR)
    ifneq (,$(findstring freebsd_be, $(TARGETENV)))
        CFLAGS += -DIL_BIGENDIAN
    endif
endif

# Change to the new dhd driver trunk build structure as discussed 
# by the build team 
ifeq ($(BUILD_TAG),TRUNK)

# $(TARGETARCH) is set based on TARGETENV in src/Makerules.* files
DHD_OBJS := $(TARGETARCH)/dhdu.o $(TARGETARCH)/dhdu_linux.o $(TARGETARCH)/bcmutils.o $(TARGETARCH)/bcmxtlv.o $(TARGETARCH)/bcmwifi_channels.o $(TARGETARCH)/wlu_client_shared.o $(TARGETARCH)/wlu_pipe_linux.o $(TARGETARCH)/wlu_pipe.o $(TARGETARCH)/miniopt.o $(TARGETARCH)/bcm_app_utils.o $(TARGETARCH)/ucode_download.o $(TARGETARCH)/wlu_common.o
DHD_EXE  := $(TARGETARCH)/dhd
ifeq ($(REPORT_FATAL_TIMEOUTS), 1)
DHD_DAEMON_OBJS := $(TARGETARCH)/dhd_daemon.o
DHD_DAEMON_EXE  := $(TARGETARCH)/dhd_daemon
endif

include netlink.inc

all: $(DHD_EXE) $(DHD_DAEMON_EXE)

clean:
	 rm -fr $(DHD_EXE) $(DHD_OBJS) $(TARGETARCH) $(DHD_DAEMON_EXE) $(DHD_DAEMON_OBJS)

$(DHD_EXE): $(DHD_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
ifeq ($(REPORT_FATAL_TIMEOUTS), 1)
$(DHD_DAEMON_EXE): $(DHD_DAEMON_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
endif
$(TARGETARCH)/%.o: %.c
	@mkdir -p $(TARGETARCH)
	$(CC) -c $(CFLAGS) -DRWL_SOCKET -DRWL_WIFI -DRWL_DONGLE -o $@ $^

else # BUILD_TAG

# Preserve the old build structure for other components that still haven't not
# changed to the new build structure, once everyone moved to the new structure
# , we can remove the whole else BUILD_TAG code

# $(TARGETARCH) is set based on TARGETENV in src/Makerules.* files
DHD_OBJS := dhdu.o dhdu_linux.o bcmutils.o bcmxtlv.o bcmwifi_channels.o wlu_client_shared.o wlu_pipe_linux.o wlu_pipe.o miniopt.o bcm_app_utils.o ucode_download.o wlu_common.o
DHD_EXE  := dhd
DHD_OBJS := $(DHD_OBJS:%.o=%.o)
ifeq ($(REPORT_FATAL_TIMEOUTS), 1)
DHD_DAEMON_OBJS := dhd_daemon.o
DHD_DAEMON_EXE  := dhd_daemon
DHD_DAEMON_OBJS := $(DHD_DAEMON_OBJS:%.o=%.o)
endif
include netlink.inc

ifeq ($(findstring $(TARGETARCH), "x86 x86_mmx"),)
  DHD_EXE  := $(DHD_EXE)$(TARGETARCH)
  DHD_OBJS := $(DHD_OBJS:%.o=$(TARGETARCH)/%.o)
 ifeq ($(REPORT_FATAL_TIMEOUTS), 1)
  DHD_DAEMON_EXE  := $(DHD_DAEMON_EXE)$(TARGETARCH)
  DHD_DAEMON_OBJS := $(DHD_DAEMON_OBJS:%.o=$(TARGETARCH)/%.o)
endif
endif

all: $(DHD_EXE) $(DHD_DAEMON_EXE)

clean:
	rm -fr $(DHD_EXE) $(DHD_OBJS) $(TARGETARCH) $(DHD_DAEMON_EXE) $(DHD_DAEMON_OBJS)

$(DHD_EXE): $(DHD_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
ifeq ($(REPORT_FATAL_TIMEOUTS), 1)
$(DHD_DAEMON_EXE): $(DHD_DAEMON_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
endif
# net_priv target is for building the dhd with NET_ADMIN privilege (i.e. to avoid running as root)
# note: The 'setcap' command is availble from the libcap2-bin package
# note: linux kernel < 2.6.33 doesn't support Security File Capabilities by default
net_priv: $(DHD_EXE)
	sudo setcap CAP_NET_ADMIN=eip $^

%.o: %.c
	$(CC) -c $(CFLAGS) -DRWL_SOCKET -DRWL_WIFI -DRWL_DONGLE -o $@ $^


ifeq ($(findstring $(TARGETARCH), "x86 x86_mmx"),)
$(TARGETARCH)/%.o: %.c
	@mkdir -p $(TARGETARCH)
	$(CC) -c $(CFLAGS) -DRWL_SOCKET -DRWL_WIFI -DRWL_DONGLE -o $@ $^
endif

endif # BUILD_TAG

endif # TARGETENV linux

#-----------------------------------------------------------------
# MacOS build
#

ifeq ($(TARGETENV), macos)

include $(SRCBASE)/Makerules.env

CFLAGS += -I$(SRCBASE)/dhd/sys
CFLAGS += -I$(SRCBASE)/wl/exe

.PHONY: all clean net_priv

MACOS_VER ?:= $(shell sw_vers -productVersion)

ifneq (,$(findstring 10.11,$(MACOS_VER)))
        GCDEFS  += -iwithsysroot -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/sys/  -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include   -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders/IOKit/apple80211   -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/Kernel.framework/Versions/A/Headers   -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/IOKit/apple80211  -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/IOKit.framework/Versions/A/Headers
	GCDEFS  += -isysroot /Developer/SDKs/MacOSX10.4u.sdk
else
ifneq (,$(findstring 10.10,$(MACOS_VER)))
        GCDEFS  += -iwithsysroot -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk/usr/include/sys/  -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk/usr/include  -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders/IOKit/apple80211  -I/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders/IOKit/apple80211
else
ifneq (,$(findstring 10.9,$(MACOS_VER)))
        GCDEFS  += -iwithsysroot /System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders/IOKit/apple80211
else
ifneq (,$(findstring 10.8.3,$(MACOS_VER)))
        GCDEFS  += -I/usr/include
        GCDEFS  += -iwithsysroot /System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders/IOKit/apple80211
else
ifneq (,$(findstring 10.8,$(MACOS_VER)))
	GCDEFS	:= $(GCDEFS) -I/usr/include -I/System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders/IOKit/apple80211
else
ifneq (,$(findstring 10.7,$(MACOS_VER)))
	GCDEFS	:= $(GCDEFS)
else
ifneq (,$(findstring 10.6,$(MACOS_VER)))
	GCDEFS	:= $(GCDEFS)
endif
endif
endif
endif
endif
endif
endif

DHD_OBJS := dhdu.o dhdu_macos.o bcmutils.o bcmxtlv.o bcmwifi_channels.o wlu_client_shared.o wlu_pipe_linux.o wlu_pipe.o miniopt.o bcm_app_utils.o ucode_download.o wlu_common.o
DHD_EXE  := dhd
DHD_OBJS := $(DHD_OBJS:%.o=%.o)
ifeq ($(REPORT_FATAL_TIMEOUTS), 1)
DHD_DAEMON_OBJS := dhd_daemon.o
DHD_DAEMON_EXE  := dhd_daemon
DHD_DAEMON_OBJS := $(DHD_DAEMON_OBJS:%.o=%.o)
endif
include netlink.inc

ifeq ($(findstring $(TARGETARCH), "x86 x86_mmx"),)
  DHD_EXE  := $(DHD_EXE)$(TARGETARCH)
  DHD_OBJS := $(DHD_OBJS:%.o=$(TARGETARCH)/%.o)
 ifeq ($(REPORT_FATAL_TIMEOUTS), 1)
  DHD_DAEMON_EXE  := $(DHD_DAEMON_EXE)$(TARGETARCH)
  DHD_DAEMON_OBJS := $(DHD_DAEMON_OBJS:%.o=$(TARGETARCH)/%.o)
endif
endif

all: $(DHD_EXE)

clean:
	rm -fr $(DHD_EXE) $(DHD_OBJS) $(TARGETARCH) $(DHD_DAEMON_EXE) $(DHD_DAEMON_OBJS)

$(DHD_EXE): $(DHD_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
ifeq ($(REPORT_FATAL_TIMEOUTS), 1)
$(DHD_DAEMON_EXE): $(DHD_DAEMON_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
endif
# net_priv target is for building the dhd with NET_ADMIN privilege (i.e. to avoid running as root)
# note: The 'setcap' command is availble from the libcap2-bin package
# note: linux kernel < 2.6.33 doesn't support Security File Capabilities by default
net_priv: $(DHD_EXE)
	sudo setcap CAP_NET_ADMIN=eip $^

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

ifeq ($(findstring $(TARGETARCH), "x86 x86_mmx"),)
$(TARGETARCH)/%.o: %.c
	@mkdir -p $(TARGETARCH)
	$(CC) -c $(CFLAGS) -o $@ $^
endif

endif # TARGETENV macos

endif # UNAME
