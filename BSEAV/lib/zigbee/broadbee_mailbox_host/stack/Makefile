##############################################################################
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
##############################################################################
#
# FILENAME: $Workfile: trunk/stack/Makefile $
#
# DESCRIPTION:
#   Stack Makefile.
#
# $Revision: 3988 $
# $Date: 2014-10-10 01:36:37Z $
#
##########################################################################################


### Suppress info messages from the make utility 'Entering/Leaving directory ...'
MAKEFLAGS += --no-print-directory


### Workaround of the problem with path to make utility under the FAR explorer.
# Substitutes /d/SVN/BROADBEE/trunk/projects/ml507_macCert/C:/MinGW/msys/1.0/bin/make.EXE
# with just C:/MinGW/msys/1.0/bin/make.EXE
ifeq ($(findstring :,$(MAKE)),:)
  MAKE := $(shell printf $(MAKE) | sed -r 's/(.*)([A-Za-z]{1}:)(.*)/\2\3/')
endif


### Set up the default Stack root directory.
STACKDIR ?= .


### Include default configuration file.
include $(STACKDIR)/default.mk


##########################################################################################


### Set up the platform-specific source files directory name: SoC or ML507.
ifeq ($(PLATFORM), __SoC__)
  PLATFDIR = arc601-soc
else ifeq ($(PLATFORM), __ML507__)
  PLATFDIR = arc601-ml507
else ifeq ($(PLATFORM), __i386__)
  PLATFDIR = i386-utest
else ifeq ($(PLATFORM), __ARM__)
  PLATFDIR = arm-host
else
  $(error Unknown platform $(PLATFORM))
endif


### Set up the set of Stack and Project components to build.
ifndef COMPONENTS
  # Discover all components of the Stack.
  export COMPLIST := $(shell sed -r '/\#/ d; s/[ \t]*([A-Za-z0-9_]+).*/\1/' $(STACKDIR)/components.mk)
  # Expand all wildcards in Exclude and Include components custom lists.
  COMPEXCLEXP := $(filter $(COMPEXCL),$(COMPLIST))
  COMPINCLEXP := $(filter $(COMPINCL),$(COMPLIST))
  # Cut off all components that are to be reincluded (according to the Include list) from
  # the Exclude list. The main idea of doing so instead of simply exclude and then
  # (re-)include components is to preserve the original order of components and avoid
  # multiple inclusions of the same component.
  COMPEXCLEXP := $(filter-out $(COMPINCLEXP),$(COMPEXCLEXP))
  # Finally exclude components that are to be excluded.
  export COMPONENTS := $(filter-out $(COMPEXCLEXP),$(COMPLIST))
endif


### Set up the Stack and Project include directories.
ifndef INCFLAGS
  # Discover all include directories up to the third level.
  INCDIRS := $(shell ls -d ./include $(STACKDIR)/*/include $(STACKDIR)/*/*/include $(STACKDIR)/*/*/*/include 2>/dev/null)
  # Prepend all directories with '-I' for the tools set.
  export INCFLAGS := $(addprefix -I,$(INCDIRS))
endif


### Set up the Stack and Project definitions.
ifndef CDEFSFLAGS
  # Prepend all definitions with '-D' for the tools set.
  export CDEFSFLAGS := $(addprefix -D,$(CDEFS))
endif


##########################################################################################


### List of all non-file targets.
.PHONY: a all c cle clean ca $(COMPLIST) o obj objdir p pre prebuild component

.NOTPARALLEL: prebuild $(COMPONENTS)

### Rule to build the Project. This is the default rule for make utility.
a all: prebuild objdir $(COMPONENTS)
	@$(MAKE) $(BINTARGET_BUILD)
	@echo "=== all completed ==="


### Rule and recipe to clean up the Project objects directory.
c cle clean: $(BINTARGET_CLEAN)
	$(SILENT)-rm -fr $(OBJDIR) $(LOGDIR) 2>/dev/null
	@echo "=== clean completed ==="


### Rule to clean up and then to build the Project.
ca: clean all


### Include the Stack components list with their descriptions.
include $(STACKDIR)/components.mk


### Rule and recipe to build different components in the Project.
$(COMPLIST): objdir
	$(SILENT)$(MAKE) component


### Recipe for objects directory.
o obj objdir:
	$(SILENT)mkdir -p $(OBJDIR)
	$(SILENT)mkdir -p $(OBJDIR)/server

### Recipe for prebuild.
p pre prebuild:
	$(SILENT)mkdir -p $(LOGDIR)
	$(SILENT)$(PY) $(STACKDIR)/prebuild/grepAtomics.py -i $(STACKDIR) 1>$(LOGDIR)/logAtomics.log 2>&1
	$(SILENT)$(PY) $(STACKDIR)/prebuild/grepAsserts.py -i $(STACKDIR) 1>$(LOGDIR)/logAsserts.log 2>&1
	@echo "=== prebuild completed ==="


##########################################################################################


### If a component is selected perform make of its sources.
ifdef COMPDIR


### Set up the virtual paths to the component source files.
SRCDIRCOMMON   := $(COMPDIR)/src
SRCDIRPLATFORM := $(COMPDIR)/$(PLATFDIR)/src
SRCDIRS := $(SRCDIRCOMMON) $(SRCDIRPLATFORM)
vpath %.c $(SRCDIRS)
vpath %.s $(SRCDIRS)


### Set up the list of source files of the component to be compiled.
ifndef SRCFILES
  # Discover all source files (*.c and *.s files) in the component's source directories.
  # List them without their extensions and directories.
  SRCLIST := $(basename $(notdir $(filter %.c %.s,$(shell ls $(SRCDIRCOMMON)/* $(SRCDIRPLATFORM)/* 2>/dev/null))))
  # Load lists (if they are provided) of files to be excluded/reincluded conditionally.
  -include $(SRCDIRCOMMON)/exclude.mk $(SRCDIRPLATFORM)/exclude.mk
  # Expand all wildcards in Exclude and Include files automatic conditional lists. Cut off
  # all possible extensions and paths coming with files' names.
  SRCEXC2EXP := $(filter $(basename $(notdir $(SRCEXC2))),$(SRCLIST))
  SRCINC2EXP := $(filter $(basename $(notdir $(SRCINC2))),$(SRCLIST))
  # Expand all wildcards in Exclude and Include files manual custom lists. Cut off all
  # possible extensions and paths coming with files' names.
  SRCEXCLEXP := $(filter $(basename $(notdir $(SRCEXCL))),$(SRCLIST))
  SRCINCLEXP := $(filter $(basename $(notdir $(SRCINCL))),$(SRCLIST))
  # For the automatic conditional Exclude list - reinclude all source files that are to be
  # reincluded according at least to one of the Include lists, automatic or manual. Any of
  # the Include lists has priority over the automatic conditional Exclude list.
  SRCEXC2EXP := $(filter-out $(SRCINCLEXP) $(SRCINC2EXP),$(SRCEXC2EXP))
  # For the manual custom Exclude list - reinclude all source files that are to be
  # reincluded according to the manual custom Include list. Only the manual custom Include
  # list has priority over the manual custom Exclude list; and the automatic conditional
  # Include list has the lower priority.
  SRCEXCLEXP := $(filter-out $(SRCINCLEXP),$(SRCEXCLEXP))
  # Finally exclude source files that are to be excluded. The main idea of cutting off the
  # Include lists from the Exclude lists instead of simply exclude files and then
  # (re-)include some of them back is to avoid multiple occurrences of the same files in
  # the final list in the case when a file was included but was not excluded.
  export SRCFILES := $(filter-out $(SRCEXCLEXP) $(SRCEXC2EXP),$(SRCLIST))
endif


### Set up the set of objects to be build for the component.
OBJFILES := $(addprefix $(OBJDIR)/,$(addsuffix .o,$(SRCFILES)))


### Rule to build an arbitrary component of the Project from its root directory.
component: $(OBJFILES)
	@-true 2>/dev/null


### Rule and recipe for compiling C source files.
$(OBJDIR)/%.o: %.c
	@echo "--- C99: $*"
	$(SILENT)$(CC) $(CFLAGS) $(INCFLAGS) $(CDEFSFLAGS) -c $< -o $@


### Rule and recipe for compiling ASM source files.
$(OBJDIR)/%.o: %.s
	@echo "--- ASM: $*"
	$(SILENT)$(AS) $(ASFLAGS) -c $< -o $@


endif	# ifdef COMPDIR


### eof Makefile #########################################################################
