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
# $brcm_Log: $
# 
############################################################

ifdef COMSPEC
# Any DOS environment
B_LIB_TOP := $(shell cd ../../../../nexus/lib && cd)
B_CABLE_TOP := $(shell cd ../../../../BSEAV/cable && cd)
B_BSEAV_TOP := $(shell cd ../../../../BSEAV && cd)
else
B_LIB_TOP := $(shell cd ../../../../nexus/lib; pwd)
B_CABLE_TOP := $(shell cd ../../../../BSEAV/cable; pwd)
B_BSEAV_TOP := $(shell cd ../../../../BSEAV && pwd)
CUR_DIR := $(shell pwd)
endif
NEXUS_TOP := $(B_LIB_TOP)/..

ifndef PLATFORM
$(error PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(B_LIB_TOP)/build/b_lib_defs.inc

THEAPPS = \
	dvr_test_v2 \
	dvr_auto_test \


ifeq ($(B_REFSW_OS),vxworks)
# VxWorks needs a wrapper function to call main.
VXOBJS = vxworks_cmd.o
# We like to use .out for loadable objects.
APPSUFFIX = .out
APPS = $(addsuffix .out, $(THEAPPS))
else
APPS = $(THEAPPS)
endif


#
# Build OBJ_DIR string to keep different binaries separate
#
SYSTEM = linux
OBJ_DIR := $(PLATFORM)-$(SYSTEM)
ifeq ($(ARCH),mipsel-linux-uclibc)
LIBC=uclibc
else
ifeq ($(ARCH),mipsel-linux)
LIBC=glibc
endif
endif
ifneq ($(LIBC),)
OBJ_DIR := $(OBJ_DIR)-$(LIBC)
endif

ifeq ($(DEBUG),)
DEBUG = $(B_REFSW_DEBUG)
endif
DEBUG ?= y
ifeq ($(DEBUG),y)
OBJ_DIR := $(OBJ_DIR).debug
else
OBJ_DIR := $(OBJ_DIR).release
endif


.PHONY: api clean

ifeq ($(APP),)
all: $(APPS)
$(APPS): api
else
all: $(APP)
$(APP): api
endif
# add flags for tshdrbuilder lib
include $(B_BSEAV_TOP)/lib/tshdrbuilder/tshdrbuilder.inc
CFLAGS += $(addprefix -I,$(TSHDRBUILDER_INCLUDES))

# include applib specifics
include $(B_LIB_TOP)/os/b_os_lib.inc
include $(B_CABLE_TOP)/lib/dvr/b_dvr_lib.inc

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lm 
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lpthread 
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lnexus
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lb_os
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lb_dvr
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -ltshdrbuilder
endif

# This builds nexus and needed applibs
api:
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(B_LIB_TOP)/os
	$(MAKE) -C $(B_CABLE_TOP)/lib/dvr
	$(MAKE) -C $(B_BSEAV_TOP)/lib/tshdrbuilder 
	
DVR_AUTO_TEST_DIRS = $(CUR_DIR)/common
DVR_AUTO_TEST_DIRS += $(CUR_DIR)/services
DVR_AUTO_TEST_DIRS += $(CUR_DIR)/utils
DVR_AUTO_TEST_INCLUDE_DIRS = $(addprefix -I,$(DVR_AUTO_TEST_DIRS))
DVR_TEXTINPUTS = $(CUR_DIR)/textinputs
DVR_AUTO_TEST_OBJECTS = \
	auto_test.o \
	CuTest.o \
	dvr_test_common.o \
	hnMulPFToMulStreamNet.o   
CFLAGS += \
	$(DVR_AUTO_TEST_INCLUDE_DIRS)

VPATH += $(DVR_AUTO_TEST_DIRS)


vpath %.h $(VPATH)
vpath %.c $(VPATH)
vpath %.o $(OBJ_DIR)

dvr_auto_test: $(OBJ_DIR) $(addprefix $(OBJ_DIR)/,$(DVR_AUTO_TEST_OBJECTS)) $(LIBS)
	@echo [Compile... $@]
	$(Q_)$(CC) -o $@ $(addprefix $(OBJ_DIR)/,$(DVR_AUTO_TEST_OBJECTS)) $(LDFLAGS)
#	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	@echo [Install... $@]
	@$(CP) dvr_auto_test $(DESTDIR)
	@$(CP) testlist.txt $(DESTDIR)
	@$(CP) $(DVR_TEXTINPUTS)/*.txt $(DESTDIR)
	@$(CP) $(CUR_DIR)/utils/hnstreaming.txt $(DESTDIR)
	@$(CP) $(CUR_DIR)/utils/hnrecord.txt $(DESTDIR)
	@$(CP) $(CUR_DIR)/utils/hnmediaplay.txt $(DESTDIR)
	@$(CP) mpstat_enable $(DESTDIR)


$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

%.o: %.c
	@echo '$(CC) $<'
	$(Q_)$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$@

$(OBJ_DIR)/%.o: %.c
	@echo '$(CC) $<'
	$(Q_)$(CC) $(CFLAGS) -c $< -o $@


# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(B_LIB_TOP)/os clean
	$(MAKE) -C $(B_CABLE_TOP)/lib/dvr clean
	$(MAKE) -C $(B_BSEAV_TOP)/lib/tshdrbuilder clean

	-$(RM) $(APPS) *.d *.o *.out;
	-$(RM) -fr $(OBJ_DIR)

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
	$(Q_)$(CP) $@ $(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin

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
DESTDIR = $(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin
endif

.PHONY: install install_only install_dvr_auto_test
install: all install_only install_dvr_auto_test

ifeq ($(APP),)
install_only:
	$(CP) $(APPS) $(DESTDIR)
else
install_only:
	$(CP) $(APP) $(DESTDIR)
endif

install_dvr_auto_test: dvr_auto_test
	@echo "[Install... dvr_auto_test]"
	@$(CP) dvr_auto_test $(DESTDIR)
	@$(CP) testlist.txt $(DESTDIR)
	@$(CP) $(DVR_TEXTINPUTS)/*.txt $(DESTDIR)
	@$(CP) $(CUR_DIR)/utils/hnstreaming.txt $(DESTDIR)
	@$(CP) $(CUR_DIR)/utils/hnmulstreaming.txt $(DESTDIR)
	@$(CP) $(CUR_DIR)/utils/hnpfstreaming.txt $(DESTDIR)
	@$(CP) $(CUR_DIR)/utils/hnmulpfstreaming.txt $(DESTDIR)	
	@$(CP) $(CUR_DIR)/utils/hnrecord.txt $(DESTDIR)
	@$(CP) $(CUR_DIR)/utils/hnmediaplay.txt $(DESTDIR)	
