############################################################
#  Copyright (C) 2018 Broadcom.
#  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to
#  the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied),
#  right to use, or waiver of any kind with respect to the Software, and
#  Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
# Module Description:
#   This file defines the sources, include paths, and definitions required
#   to use this library
#
############################################################

#
# Conditional options for this makefile
#
# None at this time
#

#
# Identify this library
#
B_LIBS += B_BMON_SCHEDULER

#
# Define source files in this library
#
B_BMON_SCHEDULER_SOURCES += \
                    $(B_LIB_TOP)/../../BSEAV/tools/bmon/scheduler/scheduler.c \
                    $(B_LIB_TOP)/../../BSEAV/tools/bmon/scheduler/uri.c \
                    $(B_LIB_TOP)/../../BSEAV/tools/bmon/scheduler/rule.c

# common source
B_BMON_SCHEDULER_SOURCES += \
                    $(B_LIB_TOP)/../../BSEAV/tools/bmon/common/bmon_json.c \
                    $(B_LIB_TOP)/../../BSEAV/tools/bmon/common/bmon_uri.c

#
# Define public include path(s) for this library
#
B_BMON_SCHEDULER_PUBLIC_INCLUDES += \
                    $(B_LIB_TOP)/../../BSEAV/tools/bmon/scheduler \
                    $(B_LIB_TOP)/../../BSEAV/tools/bmon/common

#
# Define private include path(s) for this library
#
B_BMON_SCHEDULER_PRIVATE_INCLUDES += \
                    $(B_OS_LIB_PUBLIC_INCLUDES) \
                    $(CJSON_SOURCE_PATH) \
                    $(PLUGIN_LAUNCHER_SOURCE_PATH)

# Define other libs that this library is dependent upon.
# This will check for cyclic dependencies and setup include paths
#
B_BMON_SCHEDULER_DEPENDENCIES := B_OS_LIB

#
# Define extra C definitions
#
#B_BMON_SCHEDULER_DEFINES += ## N/A for this library

#
# Define possible options - this is only for debug purposes, it does _not_ actually define them.
#
#B_BMON_SCHEDULER_OPTIONS += ## N/A for this library

B_BMON_SCHEDULER_CFLAGS := $(addprefix -I,$(B_BMON_SCHEDULER_PUBLIC_INCLUDES))

ifdef B_LIB_STATIC
B_BMON_SCHEDULER_LDFLAGS := -L$(B_REFSW_OBJ_ROOT)/nexus/bin libbmon_scheduler.a
else
B_BMON_SCHEDULER_LDFLAGS := -L$(B_REFSW_OBJ_ROOT)/nexus/bin -lbmon_scheduler
endif
