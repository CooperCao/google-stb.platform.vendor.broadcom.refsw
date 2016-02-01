############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# Created: 08/28/2007 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################
# Nexus examples

ifdef COMSPEC
# Any DOS environment
B_LIB_TOP := $(shell cd ../../../../nexus/lib && cd)
else
B_LIB_TOP := $(shell cd ../../../../nexus/lib; pwd)
B_CABLE_TOP := $(shell cd ../../../../BSEAV/cable; pwd)
ifeq ($(findstring $(PLATFORM), 97445 97145), $(PLATFORM))
B_CP_LIB := $(B_CABLE_TOP)/lib/mpod/lib/cp_arm
else
B_CP_LIB := $(B_CABLE_TOP)/lib/mpod/lib/cp
endif
B_SI_LIB := $(shell cd ../../../../BSEAV/cable/lib/si; pwd)
endif
NEXUS_TOP := $(B_LIB_TOP)/..

ifndef PLATFORM
$(error PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(B_LIB_TOP)/build/b_lib_defs.inc

APPS = mpod_test

.PHONY: api clean

ifeq ($(APP),)
all: $(APPS)
$(APPS): api
else
all: $(APP)
$(APP): api
endif

# include applib specifics
include $(B_LIB_TOP)/os/b_os_lib.inc
include $(B_CABLE_TOP)/lib/mpod/b_mpod_lib.inc



ARCH = ${B_REFSW_ARCH}
TOOLCHAIN_DIR:=$(shell dirname $(shell dirname $(shell which ${ARCH}-gcc 2>/dev/null) 2>/dev/null))
CROSS_COMPILE ?= ${ARCH}-

# This builds nexus and needed applibs
api:
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(B_LIB_TOP)/os
	$(MAKE) -C $(B_CABLE_TOP)/lib/mpod DSG_SUPPORT=$(DSG_SUPPORT) ESTB_CFG_SUPPORT=$(ESTB_CFG_SUPPORT)
	$(MAKE) -C $(B_CP_LIB)/build -f makefile.cplib TOOLCHAIN=uclibc TOOLCHAIN_DIR=$(TOOLCHAIN_DIR) CROSS_COMPILE=$(CROSS_COMPILE)
ifeq ($(ESTB_CFG_SUPPORT),y)
	$(MAKE) -C $(B_CABLE_TOP)/lib/estb_cfg
endif

# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(B_LIB_TOP)/os clean
	$(MAKE) -C $(B_CABLE_TOP)/lib/mpod clean DSG_SUPPORT=$(DSG_SUPPORT) ESTB_CFG_SUPPORT=$(ESTB_CFG_SUPPORT)
	$(MAKE) -C $(B_CP_LIB)/build -f makefile.cplib TOOLCHAIN=uclibc clean
ifeq ($(ESTB_CFG_SUPPORT),y)
	$(MAKE) -C $(B_CABLE_TOP)/lib/estb_cfg clean
endif
	-$(RM) $(B_CP_LIB)/build/cplib.lib $(B_CP_LIB)/build/libcp.a
	-$(RM) $(APPS) *.d *.o *.out;

ifneq ($(findstring $(NEXUS_PLATFORM), 97445 97145), $(NEXUS_PLATFORM))
LDFLAGS := -lb_mpod -lcp -L$(B_CP_LIB)/build -lcrypto.0.9.7b_uclibc -L$(B_CP_LIB)/crypto/linux 
else
LDFLAGS := -lb_mpod -lcp -L$(B_CP_LIB)/build -lcrypto -L$(B_CP_LIB)/crypto/linux 
endif
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lpthread -lm
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lnexus
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lb_os
ifeq ($(ESTB_CFG_SUPPORT),y)
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lb_estb_cfg
endif


# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
ifeq ($(ESTB_CFG_SUPPORT),y)
CFLAGS += -DESTB_CFG_SUPPORT -I$(B_CABLE_TOP)/lib/estb_cfg/include
endif


# Convert applib include paths into single variable.
APPLIB_INCLUDES := $(foreach lib,$(B_LIBS),$($(lib)_PUBLIC_INCLUDES))
CFLAGS += $(addprefix -I,$(APPLIB_INCLUDES))
APPLIB_DEFINES := $(foreach lib,$(B_LIBS),$($(lib)_DEFINES))
CFLAGS += $(addprefix -D,$(APPLIB_DEFINES))

# Always build with debug
CFLAGS += -g

# Implicit rule for building local apps
%$(APPSUFFIX): %.c mpod_test_io.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	$(Q_)$(CP) $@ $(NEXUS_TOP)/bin

#
# Installation: copying app to DESTDIR
#
ifeq ($(DESTDIR),)
DESTDIR = $(NEXUS_TOP)/bin
endif

.PHONY: install install_only
install: all install_only

ifeq ($(APP),)
install_only:
	$(CP) $(APPS) $(DESTDIR)
else
install_only:
	$(CP) $(APP) $(DESTDIR)
endif

