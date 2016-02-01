############################################################
#     Copyright (c) 2003-2011, Broadcom Corporation
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
# $brcm_Log: $
# 
############################################################

ifdef COMSPEC
# Any DOS environment
B_LIB_TOP := $(shell cd ../../../../../../../nexus/lib && cd)
B_ROCKFORD_TOP := $(shell cd ../../../../../../rockford && cd)
else
B_LIB_TOP := $(shell cd ../../../../../../../nexus/lib; pwd)
B_ROCKFORD_TOP := $(shell cd ../../../../../../../rockford; pwd)
endif
NEXUS_TOP := $(B_LIB_TOP)/..

ifndef PLATFORM
$(error PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(B_LIB_TOP)/build/b_lib_defs.inc

THEAPPS = \
	contentlist \
	navregenerator \
	metadatalist \
	removeall



ifeq ($(B_REFSW_OS),vxworks)
# VxWorks needs a wrapper function to call main.
VXOBJS = vxworks_cmd.o
# We like to use .out for loadable objects.
APPSUFFIX = .out
APPS = $(addsuffix .out, $(THEAPPS))
else
APPS = $(THEAPPS)
endif




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
include $(B_ROCKFORD_TOP)/lib/ocap/dvr/b_dvr_lib.inc

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lnexus -lb_os -lm -lb_dvr -L$(NEXUS_TOP)/bin -lpthread
endif

# This builds nexus and needed applibs
api:
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(B_LIB_TOP)/os
	$(MAKE) -C $(B_ROCKFORD_TOP)/lib/ocap/dvr
	

# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(B_LIB_TOP)/os clean
	$(MAKE) -C $(B_ROCKFORD_TOP)/lib/ocap/dvr

	-$(RM) $(APPS) *.d *.o *.out;

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Convert applib include paths into single variable.
APPLIB_INCLUDES := $(foreach lib,$(B_LIBS),$($(lib)_PUBLIC_INCLUDES))
CFLAGS += $(addprefix -I,$(APPLIB_INCLUDES))
APPLIB_DEFINES := $(foreach lib,$(B_LIBS),$($(lib)_DEFINES))
CFLAGS += $(addprefix -D,$(APPLIB_DEFINES))
CFLAGS += $(B_DCC_LIB_CFLAGS_OPTIONS)

# Always build with debug
CFLAGS += -g

# Implicit rule for building local apps
%$(APPSUFFIX): %.c $(VXOBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	$(Q_)$(CP) $@ $(NEXUS_TOP)/bin

ifeq ($(SYSTEM),vxworks)
# Explicit rule for building vxworks wrapper app
vxworks_cmd.o: vxworks_cmd.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
endif

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

