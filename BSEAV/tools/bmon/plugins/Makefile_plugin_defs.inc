#############################################################################
# Copyright (C) 2018 Broadcom.
# The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to
# the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied),
# right to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#############################################################################

# Protect against multiple/circular includes...
ifndef BMON_PLUGINS_INC_INCLUDED
BMON_PLUGINS_INC_INCLUDED := 1
export NEXUS_BMON_SUPPORT=y

ifdef B_REFSW_TOP
    NEXUS_TOP ?= $(abspath $(B_REFSW_TOP)/nexus)
endif
ifdef NEXUS_TOP
    B_REFSW_TOP ?= $(abspath $(NEXUS_TOP)/..)
endif

$(if $(NEXUS_PLATFORM),,  $(error ERROR: NEXUS_PLATFORM is not defined))
$(if $(B_REFSW_ARCH),,    $(error ERROR: B_REFSW_ARCH is not defined))
$(if $(B_REFSW_TOP),,     $(error ERROR: B_REFSW_TOP is not defined))

include $(NEXUS_TOP)/build/nexus_defs.inc

BMON_INSRC_TOP := $(abspath $(BSEAV)/tools/bmon)
BMON_OOSRC_TOP := $(abspath $(B_REFSW_OBJ_ROOT)/BSEAV/tools/bmon)

BMON_PLUGINS_INSRC_DIR := $(abspath $(BSEAV)/../BSEAV/tools/bmon/plugins)
BMON_PLUGINS_OOSRC_DIR := $(abspath $(B_REFSW_OBJ_ROOT)/BSEAV/tools/bmon/plugins)

BMON_PLUGINS_COMMON_INSRC_DIR := $(abspath $(BSEAV)/../BSEAV/tools/bmon/common)

BMON_PLUGINS_PRIVATE_INCLUDES += ${BSEAV}/tools/bmemperf/include

endif # ifndef BMON_PLUGINS_INC_INCLUDED
