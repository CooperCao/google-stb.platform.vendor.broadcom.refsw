############################################################
#  Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# Module Description:
#
############################################################

B_THIS_DIR:=BSEAV/tools/bmon/plugins/wifi
B_REFSW_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))

# Define this plugin's name.
B_THIS_PLUGIN := $(notdir ${B_THIS_DIR})

# Define some variables that point to directories that we might want to use here.
include $(CURDIR)/../Makefile_plugin_defs.inc

# Include any ".inc" files for sub-packages to define header locations, defines, and resulting binaries.
include ${B_REFSW_TOP}/BSEAV/opensource/cjson/cjson.inc

# Include any ".inc" files for sub-packages to define header locations, defines, and resulting binaries.
include $(B_REFSW_TOP)/BSEAV/lib/bwl/bwl.inc

# Define the build and clean targets from this plugin's targets.inc file:
BMON_THIS_PLUGIN_BUILD_WITH_SUBS_TARGET=b_bmon_plugin_wifi_target_build
BMON_THIS_PLUGIN_CLEAN_WITH_SUBS_TARGET=b_bmon_plugin_wifi_target_clean
BMON_THIS_PLUGIN_CLEAN_ALL_WITH_SUBS_TARGET=b_bmon_plugin_wifi_target_clean_all

# Select Nexus/NxClient support (or not)
BMON_THIS_PLUGIN_USES_NEXUS := n

# Add any extra include directories:
# For example:
#     BMON_THIS_PLUGIN_EXTRA_INCLUDE_DIR_LIST += $(abspath ${B_REFSW_TOP}/BSEAV/tools/bmemperf/include)
BMON_THIS_PLUGIN_EXTRA_INCLUDE_DIR_LIST += $(abspath ${B_REFSW_TOP}/BSEAV/tools/bmon/common)
BMON_THIS_PLUGIN_EXTRA_INCLUDE_DIR_LIST += $(B_BWL_INCLUDES)
BMON_THIS_PLUGIN_EXTRA_INCLUDE_DIR_LIST += $(CJSON_LIB_FOLDER)

# Add any extra source files to be compiled and statically linked to the plugin.
# For example:
#     BMON_THIS_PLUGIN_EXTRA_SRC_PATH_LIST += $(abspath ${BMON_PLUGINS_COMMON_INSRC_DIR}/bmon_nx.c)
BMON_THIS_PLUGIN_EXTRA_SRC_PATH_LIST += $(abspath ${BMON_PLUGINS_COMMON_INSRC_DIR}/bmon_utils.c)
BMON_THIS_PLUGIN_EXTRA_SRC_PATH_LIST += $(abspath ${BMON_PLUGINS_COMMON_INSRC_DIR}/plugin_header.c)
BMON_THIS_PLUGIN_EXTRA_SRC_PATH_LIST += $(abspath ${BMON_PLUGINS_COMMON_INSRC_DIR}/bmon_json.c)
BMON_THIS_PLUGIN_EXTRA_SRC_PATH_LIST += $(abspath ${BMON_PLUGINS_COMMON_INSRC_DIR}/bmon_uri.c)

# Add any extra compiler flags that you need.
# For example:
#    BMON_THIS_PLUGIN_EXTRA_CFLAGS += -DDEBUG
BMON_THIS_PLUGIN_EXTRA_CFLAGS += -DBWL_SUPPORT=1

# Add any extra linker flags to link with external libraries (or whatever).
# For example:
BMON_THIS_PLUGIN_EXTRA_LDFLAGS += -L$(abspath $(B_REFSW_OBJ_ROOT)/nexus/bin)
BMON_THIS_PLUGIN_EXTRA_LDFLAGS += $(B_BWL_LDFLAGS)

BMON_PLUGINS_EXE_PRIVATE_LDFLAGS += -L$(abspath $(B_REFSW_OBJ_ROOT)/BSEAV/lib/bwl)
BMON_PLUGINS_EXE_PRIVATE_LDFLAGS += $(CJSON_LDFLAGS)
$(warning "$(CJSON_LDFLAGS)")
# Include the generic plugin makefile fragment that does all the work
include $(CURDIR)/../Makefile_generic.inc

# And finally, include this plugin's targets.inc file:
include $(BMON_PLUGINS_INSRC_DIR)/wifi/b_bmon_plugin_wifi_targets.inc
