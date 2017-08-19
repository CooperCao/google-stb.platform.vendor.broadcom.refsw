#
# GNUmakefile for epi_ttcp.
#
# Copyright (c) 2005, Broadcom Corp.
# $Id$


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

# Search path for epi_ttcp source files.
vpath %.c $(SRCBASE)/tools/misc

# Add include directories for epi_ttcp.
LWIP_BASE_DIR := $(BSP_BASE_DIR)/lwip
CFLAGS += -I$(LWIP_BASE_DIR)/dhdnu
CFLAGS += -I$(LWIP_BASE_DIR)/src/include
CFLAGS += -I$(LWIP_BASE_DIR)/src/include/ipv4


# Basic options
CFLAGS += -DBCMDRIVER 
CFLAGS += -DBCMDONGLEHOST 
CFLAGS += -DBWL_MOBCOM_DBGPRINTF 

# LWIP TCP/IP stack.
CFLAGS += -DBWL_TCPIP_LWIP


# Source files.
CFILES := \
	epi_ttcp.c



DEP_FILES := $(wildcard $(OBJDIR)/*.d)
OBJ_FILES := $(addprefix $(OBJDIR),$(CFILES:.c=.o))

TARGETLIB = $(OBJDIR)/libepittcp.a

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
