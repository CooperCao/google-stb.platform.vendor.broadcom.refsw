#
# GNUmakefile for dhd/exe
# (Basically a copy of wl/exe linux portion.)
#
# Copyright (C) 2017, Broadcom. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id: GNUmakefile,v 1.1 2010-11-24 05:54:12 $

SRCBASE = ../..

UNAME = $(shell uname)

#-----------------------------------------------------------
# Windows build
# 1) windows, don't include Makerules due to all: conflict
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)

SRCFILE = 
include ../../GNUmakefile.inc

else # UNAME

# 2) not windows, need to include first to pick up TARGETENV dependent vars
include $(SRCBASE)/Makerules


# Always include pktgen commands in the application
#CFLAGS += -DSDTEST
#CFLAGS += -DBCMSPI

CFLAGS += -DTARGETENV_android -DLINUX -Dlinux -DWPS_WIRELESS_ENROLLEE -DANDROID_AFTERCUPCAKE -D__linux__ -DWPSENR_BINARY_SINGLE

#CFLAGS += -DDONGLEOVERLAYS
GCINCS	+= -I$(SRCBASE)/include \
			-I$(SRCBASE)/common/include \
			-I$(SRCBASE)/wps/common/include \
			-I$(SRCBASE)/wps/linux/inc \
			-I$(SRCBASE)/include/bcmcrypto \
			-I$(SRCBASE)/include/proto \
			-I$(SRCBASE)/common/include/proto 

#-----------------------------------------------------------
# Linux build
#
ifneq ($(findstring $(TARGETENV), "linux linuxmips linuxmips_be linuxarm linuxarm_le linuxarm_omap android"),)
# the below WL_OID_BASE is not visible from wlioctl.h. Hence including
# it from here for file wlu_linux_common.c.
#CFLAGS += -DWL_OID_BASE=0
# $(TARGETARCH) is set based on TARGETENV in src/Makerules.* files
WPSENR_OBJS:= buffobj.o \
			     reg_proto_utils.o \
			     reg_proto_msg.o \
			     tlv.o \
			     state_machine.o \
			     tutrace.o \
			     wps_utils.o \
			     wps_sslist.o \
			     enr_api.o \
			     dev_config.o \
			     sta_eap_sm.o \
			     enr_reg_sm.o \
			     reg_sm.o \
			     wps_api.o \
			     wps_linux_hooks.o \
			     aes.o \
			     rijndael-alg-fst.o \
			     dh.o \
			     bn.o \
			     sha256.o \
			     hmac_sha256.o \
			     random.o \
			     wl_wps.o \
			     wps_enr.o
WPSENR_EXE  := wpsenr
ifneq ($(findstring x86,$(TARGETARCH)),x86)
  WPSENR_EXE := $(WPSENR_EXE)$(TARGETARCH)
  WPSENR_OBJS := $(WPSENR_OBJS:%.o=$(TARGETARCH)/%.o)
endif

# extra warnings
CFLAGS += -Wextra $(CUSTOM_FLAGS)

ifeq ($(TARGETENV), linuxarm_omap)
CFLAGS += -DBGBRD
endif

vpath %.c $(SRCBASE)/wps/common
vpath %.c $(SRCBASE)/wps/common/shared
vpath %.c $(SRCBASE)/wps/common/sta
vpath %.c $(SRCBASE)/wps/common/enrollee
vpath %.c $(SRCBASE)/wps/common/registrar
vpath %.c $(SRCBASE)/wps/linux/enr
vpath %.c $(SRCBASE)/bcmcrypto

all: $(WPSENR_EXE)

$(WPSENR_EXE): $(WPSENR_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGETARCH)/%.o: %.c
	@mkdir -p $(TARGETARCH)
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	@rm -rf $(TARGETARCH)
	rm -f $(WPSENR_EXE) *.o
	rm -rf build

endif # linux

#-----------------------------------------------------------
# MacOS
#
ifeq ($(TARGETENV), macos)
DHD_EXE := dhd_macos
BUILD   := xcodebuild
PROJECT := dhd.xcodeproj
PROJTGT := dhd

all: $(DHD_EXE)

# Executable is located in build/Debug/dhd
dhd_macos:
	$(BUILD) -project $(PROJECT) -target $(PROJTGT) -configuration Debug build

clean:
	rm -f $(DHD_EXE) *.o
	rm -rf build

endif # macos


.PHONY: all clean

endif # UNAME
