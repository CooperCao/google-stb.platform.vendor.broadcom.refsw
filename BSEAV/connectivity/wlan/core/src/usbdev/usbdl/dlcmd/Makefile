#
# USB Remote Download Makefile - Host linux user mode driver
#
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$


SRCBASE = ../../..
include $(SRCBASE)/Makerules

CFLAGS= -Wall -Wstrict-prototypes -g -O2 -I$(SRCBASE)/include -I$(SRCBASE)/../components/shared
LIBS= -lusb
BINARIES = dlcmd
TARGET = dlcmd
OBJECTS = dlcmd.o

ifeq ($(TARGETENV), linux)
	OBJECTS += ../usb_linux.o
	COMPILE = gcc
	CLEAN = clean_linux
endif
ifeq ($(TARGETENV), macos)
	TARGET = bcmdl_macos
	CLEAN = clean_macos
	COMPILE = xcodebuild
	PROJECT = bcmdl.xcodeproj
endif

all: $(TARGET)

dlcmd: $(OBJECTS)
	$(COMPILE) $(OBJECTS) $(CFLAGS) $(LIBS) -o $@

# Executable is located in build/Debug/bcmdl
#
bcmdl_macos:
	$(COMPILE) -project $(PROJECT) -target $(BINARIES) -configuration Debug build

clean_macos:
	$(COMPILE) -project $(PROJECT) -target $(BINARIES) -configuration Debug clean

clean_linux:
	rm -rf $(BINARIES) $(OBJECTS)

clean: $(CLEAN)
