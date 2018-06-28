###############################################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
###############################################################################
B_THIS_DIR:=BSEAV/tools/bmon/plugins

B_REFSW_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))

include $(CURDIR)/Makefile_plugin_defs.inc

###################################################################
#  Define some debug aids.
###################################################################
# Build the name of this makefile, relative to B_REFSW_TOP (used for recipe tracing)
# Keep this line before any includes!
B_THIS_MAKEFILE_NAME := $(subst $(abspath ${BSEAV}/..),,$(abspath $(lastword $(MAKEFILE_LIST))))
ifeq (${BMON_MAKEFILE_DEBUG},y)
    B_TRACE_RECIPE_BEGIN = @echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>  ${B_THIS_MAKEFILE_NAME}: Begin recipe: $@"
    B_TRACE_RECIPE_END   = @echo "<<<<<<<<<<<<<<<<<<<<<<<<<<<<  ${B_THIS_MAKEFILE_NAME}:   End recipe: $@"
endif

# Define a debug function for printing a variable name and it's contents:
ifeq (${BMON_MAKEFILE_DEBUG},y)
BMON_PLUGIN_PRINTVAR =               \
    $(if $($1),                         \
        $(info )                        \
        $(info ---[debug]: $1:)         \
        $(foreach j,$($(1)),            \
            $(info $(j))                \
        )                               \
     ,                             		\
        $(info )                        \
        $(info ---[debug]: $1 is undefined (or empty))  \
    )
endif  # ifeq (${BMON_MAKEFILE_DEBUG},y)

###################################################################
#  Default rule:
###################################################################
.PHONY: all
all: install
	$(B_TRACE_RECIPE_BEGIN)
	$(B_TRACE_RECIPE_END)

# Each plugin is in its own subdirectory, and must have a b_xxx_targets.inc file
# that has a b_xxx_target_build target to build itself along with any
# prerequisite sub-packages.

# Build a list of full paths of the targets.inc files for all of the plugins.
SUB_TARGETINC_PATH_LIST:=$(wildcard $(CURDIR)/*/*targets.inc)

# Reduce to list of file names.
SUB_TARGETINC_FILENAME_LIST:=$(foreach p,${SUB_TARGETINC_PATH_LIST},$(notdir ${p}))

# Finally, reduce to list of package names based on the targets.inc file naming convention (b_<package-name>_targets.inc)
SUB_TARGETINC_PACKAGE_LIST:=$(patsubst b_%_targets.inc,%,${SUB_TARGETINC_FILENAME_LIST})

# From the list of package names, build lists of the build, clean, and clean_all targets
# for all of the plugins, based on the naming convention of the targets (b_<package-name>_target_build, etc.
SUB_TARGETINC_BUILD_TARGET_LIST:=$(patsubst %,b_%_target_build,${SUB_TARGETINC_PACKAGE_LIST})
SUB_TARGETINC_CLEAN_TARGET_LIST:=$(patsubst %,b_%_target_clean,${SUB_TARGETINC_PACKAGE_LIST})
SUB_TARGETINC_CLEAN_ALL_TARGET_LIST:=$(patsubst %,b_%_target_clean_all,${SUB_TARGETINC_PACKAGE_LIST})

# Now include all of the b_xxx_targets.inc files that we found.
include ${SUB_TARGETINC_PATH_LIST}

# For "make", "make all", or "make install":
.PHONY: install
install: ${SUB_TARGETINC_BUILD_TARGET_LIST}

# For "make clean"
.PHONY: clean
clean:  ${SUB_TARGETINC_CLEAN_TARGET_LIST}

# For "make clean_all" ("clean_all" may do more thorough cleaning of certain sub-packages)
.PHONY: clean_all
clean_all:  ${SUB_TARGETINC_CLEAN_TARGET_LIST}

$(call BMON_PLUGIN_PRINTVAR ,B_REFSW_TOP)
$(call BMON_PLUGIN_PRINTVAR ,SUB_TARGETINC_PATH_LIST)
$(call BMON_PLUGIN_PRINTVAR ,SUB_TARGETINC_FILENAME_LIST)
$(call BMON_PLUGIN_PRINTVAR ,SUB_TARGETINC_PACKAGE_LIST)
$(call BMON_PLUGIN_PRINTVAR ,SUB_TARGETINC_BUILD_TARGET_LIST)
$(call BMON_PLUGIN_PRINTVAR ,SUB_TARGETINC_CLEAN_TARGET_LIST)