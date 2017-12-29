############################################################
#  Copyright (C) 2002-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
B_LIBS += B_OS_LIB

#
# Define source files in this library
#
B_OS_LIB_SOURCES += \
                    $(B_LIB_TOP)/os/src/$(B_REFSW_OS)/b_os_lib.c \
                    $(B_LIB_TOP)/os/src/$(B_REFSW_OS)/b_os_message_queue.c \
                    $(B_LIB_TOP)/os/src/$(B_REFSW_OS)/b_os_mutex.c \
                    $(B_LIB_TOP)/os/src/$(B_REFSW_OS)/b_os_thread.c \
                    $(B_LIB_TOP)/os/src/$(B_REFSW_OS)/b_os_time.c

# generic source
B_OS_LIB_SOURCES += \
                    $(B_LIB_TOP)/os/src/b_os_event.c \
                    $(B_LIB_TOP)/os/src/b_os_memory.c \
                    $(B_LIB_TOP)/os/src/b_os_scheduler.c \
                    $(B_LIB_TOP)/os/src/b_os_lock.c

#
# Define public include path(s) for this library
#
B_OS_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/os/include \
                            $(B_LIB_TOP)/os/include/$(B_REFSW_OS)

#
# Define private include path(s) for this library
#
B_OS_LIB_PRIVATE_INCLUDES += $(B_LIB_TOP)/os/src \
                             $(B_LIB_TOP)/os/src/$(B_REFSW_OS)

# Need some Magnum basemodules
MAGNUM := $(B_LIB_TOP)/../../magnum
include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/commonutils/lst/blst.inc

B_OS_LIB_PRIVATE_INCLUDES += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
B_OS_LIB_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))

#
# Define other libs that this library is dependent upon.
# This will check for cyclic dependencies and setup include paths
#
#B_OS_LIB_DEPENDENCIES := ## N/A for this library

#
# Define extra C definitions
#
#B_OS_LIB_DEFINES += ## N/A for this library

#
# Define possible options - this is only for debug purposes, it does _not_ actually define them.
#
#B_OS_LIB_OPTIONS += ## N/A for this library
