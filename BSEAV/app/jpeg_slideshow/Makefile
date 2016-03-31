#############################################################################
#
#       Copyright (c) 2009, Broadcom Corporation.
#       All rights reserved.
#       Confidential Property of Broadcom Corporation.
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# File Description:
#
# Revision History:
#
# $brcm_Log: $
# 
#############################################################################
# Nexus examples

ifdef COMSPEC
# Any DOS environment
NEXUS_TOP := $(shell cd ../../../nexus && cd)
else
NEXUS_TOP := $(shell cd ../../../nexus; pwd)
endif
LIB=$(NEXUS_TOP)/../BSEAV/lib

ifndef PLATFORM
$(error PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(PLATFORM)/build/platform_app.inc

ifdef APP
APPS=$(APP)
else
THEAPPS = jpeg_slideshow show_jpeg
endif

ifeq ($(B_REFSW_OS),vxworks)
# VxWorks needs a wrapper function to call main.
VXOBJS = vxworks_cmd.o
# We like to use .out for loadable objects.
APPSUFFIX = .out
APPS = $(addsuffix .out, $(THEAPPS))
else
APPS = $(THEAPPS)
endif

.PHONY: api clean libjpeg config-libjpeg

all: libjpeg $(APPS)
$(APPS): api

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lnexus -L$(NEXUS_TOP)/bin -lpthread
endif

api:
	$(MAKE) -C $(NEXUS_TOP)/build

# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	-$(RM) $(APPS) *.d *.o *.out;

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

LIBJPEG_DIR=$(BSEAV)/lib/jpeg-6b
CFLAGS += -I$(LIBJPEG_DIR)
LDFLAGS += -L$(LIBJPEG_DIR) -ljpeg

# Always build with debug
CFLAGS += -g

ifeq ($(NEXUS_DTV_PLATFORM),y)
SRCS = b_hw_jpeg.c
else
SRCS = b_soft_jpeg.c
endif
OBJS = $(patsubst %.c,%.o,$(SRCS))

%.o: %.c
	@echo [Compile... $<]
	$(Q_)$(CC) -c -o $@ $^ $(CFLAGS)

show_jpeg: show_jpeg.o $(OBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	cp $@ $(NEXUS_TOP)/bin

jpeg_slideshow: jpeg_slideshow.o $(OBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	cp $@ $(NEXUS_TOP)/bin

ifeq ($(SYSTEM),vxworks)
# Explicit rule for building vxworks wrapper app
vxworks_cmd.o: vxworks_cmd.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
endif

install: all

# There is no separate configure step for bwin, so handle it with make,
# but don't configure if Makefile already exists.
libjpeg: $(LIBJPEG_DIR)/libjpeg.a

# configure libjpeg if Makefile doesn't exist
config-libjpeg:
#speed of machine vs. granularity of creation/modification creates the need for a little sleep here
	@if [ ! -e $(LIBJPEG_DIR)/Makefile ]; then \
		echo [Config.... libjpeg]; \
		cd $(LIBJPEG_DIR); RANLIB=$(RANLIB) AR="$(AR) rc" CC=$(CC) sh ./configure 1> /dev/null; \
		sleep 1; \
	fi

$(LIBJPEG_DIR)/libjpeg.a: config-libjpeg
	@if ! $(MAKE) -q -C $(LIBJPEG_DIR) libjpeg.a; then \
		echo [Build..... libjpeg]; \
		$(MAKE) -C $(LIBJPEG_DIR) libjpeg.a; \
	fi

clean-libjpeg:
	@if [ -e $(LIBJPEG_DIR)/Makefile ]; then \
		$(MAKE) -C $(LIBJPEG_DIR) distclean; \
	fi
