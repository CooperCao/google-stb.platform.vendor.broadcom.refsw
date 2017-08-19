#
# GNUmakefile for wps/libwps
#
# Copyright (c) 2005, Broadcom Corp.
# $Id$

# TODO:
# TODO: THIS MAKEFILE NEEDS TO BE CHANGED TO use src/wl/config model by rtos
# TODO: dhd owners.
# TODO: List of source files need to come from WLFILES and flags from WLFLAGS
# TODO:

ifeq ($(SRCBASE),)
   SRCBASE := ../..
endif

SUPPORTED_TARGETENVS := nucleusarm

# Error check that target environment is valid.
ifeq ($(filter $(TARGETENV),$(SUPPORTED_TARGETENVS)),)
$(warning Unsupported value for TARGETENV: '$(TARGETENV)')
$(warning TARGETENV must be one of the following:)
$(warning $(space))
$(foreach target,$(SUPPORTED_TARGETENVS),$(warning $(space)  $(target)))
$(error $(space))
endif

# Set output directory.
OBJDIR := $(TARGETENV)/$(TARGETCPU)/

# Include generic environment makefile.
include $(SRCBASE)/Makerules

# Search path for source files.
vpath %.c $(SRCBASE)/bcmcrypto/
vpath %.c $(SRCBASE)/wps/common/enrollee
vpath %.c $(SRCBASE)/wps/common/shared
vpath %.c $(SRCBASE)/wps/common/sta
vpath %.c $(SRCBASE)/wps/common/registrar
vpath %.c $(SRCBASE)/wps/rtos/Enr


# Add include directories.
CFLAGS += -I$(SRCBASE)/include
CFLAGS += -I$(SRCBASE)/include/proto
CFLAGS += -I$(SRCBASE)/../components/shared
CFLAGS += -I$(SRCBASE)/../components/shared/proto
CFLAGS += -I$(SRCBASE)/include/bcmcrypto
CFLAGS += -I$(SRCBASE)/wps/common/include
CFLAGS += -I$(SRCBASE)/wps/rtos/inc
CFLAGS += -I$(SRCBASE)/wl/sys
CFLAGS += -I$(SRCBASE)/../components/phy/old

# Basic options
CFLAGS += -DSRCBASE=\"$(SRCBASE)\" 
CFLAGS += -DBCMDRIVER   
CFLAGS += -DBCMSDIO
CFLAGS += -DBWL_MOBCOM_DBGPRINTF 
CFLAGS += -DBWL_STRICMP
CFLAGS += -DBCMWPA2
#CFLAGS += -DBWL_NO_INTERNAL_STDLIB_SUPPORT
CFLAGS += -DBWL_OSL_USE_STDLIB_MALLOC
CFLAGS += -DWPS_WIRELESS_ENROLLEE 
CFLAGS += -DBCMINTERNAL
CFLAGS += -DBCMDONGLEHOST
CFLAGS += -DLINKED_WITH_OPENSSL

ifdef BCMINTERNAL
CFLAGS += -DBCMDBG 
CFLAGS += -D_TUDEBUGTRACE
endif

# COMPILER 
#CFLAGS += --loose_implicit_cast
CFLAGS += --diag_suppress 1295

# Source files.

WPS_FILES :=  wl_wps.c \
	wps_rtos_hooks.c \
	wps_api.c	\
	wps_enr.c	

SAMPLEAPPFILES := wps_api_tester.c

CRYPTFILES := aes.c \
	rijndael-alg-fst.c \
	dh.c \
	bn.c \
	sha256.c \
	hmac_sha256.c \
	random.c

COMM_ENR_FILES :=  enr_api.c \
	dev_config.c \
	sta_eap_sm.c \
	enr_reg_sm.c \
	reg_sm.c \
	tutrace.c \
	wps_sslist.c \
	reg_proto_utils.c \
	reg_proto_msg.c \
	tlv.c \
	wps_utils.c \
	state_machine.c \
	buffobj.c 

CFILES := $(COMM_ENR_FILES) \
	$(CRYPTFILES) \
	$(WPS_FILES) \
	$(SAMPLEAPPFILES)

DEP_FILES := $(wildcard $(OBJDIR)/*.d)
OBJ_FILES := $(addprefix $(OBJDIR),$(CFILES:.c=.o))

TARGETLIB = $(OBJDIR)/libwps.a

# Create output directory.
$(shell mkdir -p $(OBJDIR))


.PHONY: all
all: lib

.PHONY: lib
lib: $(TARGETLIB)
$(TARGETLIB): $(OBJ_FILES) 
	$(AR) $(TARGETLIB) $^

.PHONY: clean
clean:
	@rm -f $(OBJ_FILES) $(DEP_FILES) $(TARGETLIB)

.PHONY: clobber
clobber:
	rm -Rf $(OBJDIR)
   
.PHONY: clobber_all
clobber_all:
	rm -Rf $(SUPPORTED_TARGETENVS)
   
# Include dependencies.
ifeq ($(strip $(filter clean% clobber%, $(MAKECMDGOALS))),)
   ifneq ($(DEP_FILES),)
      include $(DEP_FILES)
   endif
endif	
