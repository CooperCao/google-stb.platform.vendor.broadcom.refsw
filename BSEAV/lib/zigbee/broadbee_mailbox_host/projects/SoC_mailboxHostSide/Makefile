#############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#############################################################################
#
# FILENAME: $Workfile: trunk/projects/ml507_zrc1SingleRF4CETarget/Makefile $
#
# DESCRIPTION:
#   Project Makefile.
#
# $Revision: 3368 $
# $Date: 2014-08-21 16:02:35Z $
#
##########################################################################################


### Suppress info messages from the make utility 'Entering/Leaving directory ...'
MAKEFLAGS += --no-print-directory


### Include custom configuration file (if it is provided) to override/append settings.
-include config.mk



##########################################################################################

override PLATFDIR := i386-utest

### Set up the binary output file name and path.
ifdef BINTARGET
  BINFILE := $(BINTARGET).elf
else
  $(error Target for Binary is not defined.)
endif

ifeq ($(NEXUS_PLATFORM),)
$(error You must define NEXUS_PLATFORM)
endif

### Set up the linker script file name and path.
ifdef LDSCRIPT
  LDSCRIPTFILE := ./$(LDSCRIPT).ld
endif


### Assemble application targets' names.
BINTARGET_BUILD := $(addsuffix _build,$(BINTARGET))
BINTARGET_CLEAN := $(addsuffix _clean,$(BINTARGET))
.PHONY: $(BINTARGET_CLEAN)

#$(BINTARGET_BUILD)
INCDIRS := $(shell ls -d ./include $(STACKDIR)/*/include $(STACKDIR)/*/*/include $(STACKDIR)/*/*/*/include 2>/dev/null)
# Prepend all directories with '-I' for the tools set.
INCDIRS += ./rf4ce_registration/include ./ha_registration/include ./common/include ../../../../../linux/driver/zigbee/
ifeq ($(BYPASS_RPC),y)
INCDIRS += ./non_rpc/include
else
INCDIRS += ./rpc/include
endif
export INCFLAGS := $(addprefix -I,$(INCDIRS))

sys : export COMPDIR := $(STACKDIR)/common/System
ext : export COMPDIR := $(STACKDIR)/common/External
hal : export COMPDIR := $(STACKDIR)/common/HAL
mbx : export COMPDIR := $(STACKDIR)/common/Mailbox
nvm : export COMPDIR := $(STACKDIR)/common/NVM
common : export COMPDIR := ./common
ifeq ($(BYPASS_RPC),y)
non_rpc : export COMPDIR := ./non_rpc
else
rpc : export COMPDIR := ./rpc
endif
rrn : export COMPDIR := ./rf4ce_registration
hrn : export COMPDIR := ./ha_registration
prj : export COMPDIR := ./

ifeq ($(BYPASS_RPC),y)
COMPONENTS := sys hal mbx non_rpc rrn hrn prj common ext
else
COMPONENTS := sys hal mbx rpc rrn hrn prj common ext
endif

### Include the Stack Makefile. The Stack target 'all' will become the default target.
include $(STACKDIR)/Makefile

$(COMPONENTS): objdir
	$(SILENT)$(MAKE) component

### Rule to build the application.
ifeq ($(BYPASS_RPC),y)
AR = arm-linux-ar
RANLIB = arm-linux-ranlib

$(BINTARGET_BUILD) : $(OBJDIR)/*.o
	@echo "--- AR: libzigbee.a"
	@touch $(BINTARGET_BUILD)
	$(SILENT)$(AR) cr $(OBJDIR)/libzigbee.a $(OBJDIR)/*.o
	$(SILENT)$(RANLIB) $(OBJDIR)/libzigbee.a
else
$(BINTARGET_BUILD) : $(OBJDIR)/*.o
	@echo "--- LNK: $(BINTARGET)"
	@touch $(BINTARGET_BUILD)
	$(SILENT)$(LD) $(LDFLAGS) $(LDSCRIPTFILE) $(OBJDIR)/*.o $(LIBS) -o ./obj.$(NEXUS_PLATFORM)/$(BINFILE)
	$(SILENT)-size -td ./obj.$(NEXUS_PLATFORM)/$(BINFILE) 2>/dev/null
endif


### Rule to clean the application.
$(BINTARGET_CLEAN):
	$(SILENT)-rm -fr $(OBJDIR) $(LOGDIR) ./$(BINTARGET).* 2>/dev/null


### eof Makefile #########################################################################
