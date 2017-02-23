############################################################
#  Broadcom Proprietary and Confidential. (c)2009-2014 Broadcom. All rights reserved.
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
############################################################

############################################################
#
# Conditional Options for this Makefile
#
# N/A
#

V3D_HAS_BPCM = $(shell (test -e $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_v3d_bpcm.h || \
								(test -e $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_cfgctl.h && \
								 test -e $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_zone0_fs.h)) \
								&& echo y)
ifeq ($(V3D_HAS_BPCM),y)
BVC5_DEFINES += V3D_HAS_BPCM
endif

# Define this module
MAGNUM_MODULES += BVC5

V3D_MAGNUM_TOP ?= $(MAGNUM)/portinginterface/vc5

# Module Includes
BVC5_INCLUDES += $(V3D_MAGNUM_TOP)/include

# Module Sources
BVC5_SOURCES += \
				$(V3D_MAGNUM_TOP)/src/bvc5.c                \
				$(V3D_MAGNUM_TOP)/src/bvc5_client.c         \
				$(V3D_MAGNUM_TOP)/src/bvc5_jobq.c           \
				$(V3D_MAGNUM_TOP)/src/bvc5_activeq.c        \
				$(V3D_MAGNUM_TOP)/src/bvc5_scheduler_state.c\
				$(V3D_MAGNUM_TOP)/src/bvc5_scheduler.c      \
				$(V3D_MAGNUM_TOP)/src/bvc5_hardware.c       \
				$(V3D_MAGNUM_TOP)/src/bvc5_hardware_debug.c \
				$(V3D_MAGNUM_TOP)/src/bvc5_bin_mem.c        \
				$(V3D_MAGNUM_TOP)/src/bvc5_simpenrose.c     \
				$(V3D_MAGNUM_TOP)/src/bvc5_null.c           \
				$(V3D_MAGNUM_TOP)/src/bvc5_internal_job.c   \
				$(V3D_MAGNUM_TOP)/src/bvc5_bin_pool.c       \
				$(V3D_MAGNUM_TOP)/src/bvc5_perf_counters.c  \
				$(V3D_MAGNUM_TOP)/src/bvc5_event_monitor.c  \
				$(V3D_MAGNUM_TOP)/src/bvc5_usermode.c       \
				$(V3D_MAGNUM_TOP)/src/bvc5_bin_pool_alloc.c \
				$(V3D_MAGNUM_TOP)/src/$(B_REFSW_OS)/bvc5_os.c

ifeq ($(B_REFSW_ANDROID),y)
BVC5_SOURCES += \
				$(V3D_MAGNUM_TOP)/src/$(B_REFSW_OS)/bvc5_android_fences.c
else
BVC5_SOURCES += \
				$(V3D_MAGNUM_TOP)/src/bvc5_fence.c
endif


# Sanity Check OS provided
BVC5_SUPPORTED_OS := linuxuser linuxkernel linuxclient linuxemu linuxpal
ifeq ($(findstring $(B_REFSW_OS), $(BVC5_SUPPORTED_OS)),)
$(error [VC5] OS must be one of the following: $(BVC5_SUPPORTED_OS))
endif

BVC5_DEFINES += BVC5_HARDWARE_REAL

ifeq ($(findstring $(B_REFSW_OS), linuxuser linuxclient linuxemu linuxpal),${B_REFSW_OS})
   BVC5_DEFINES += BVC5_USERMODE
   ifeq ($(V3D_DRM_SUPPORT),y)
      BVC5_INCLUDES += $(B_REFSW_OBJ_ROOT)/BSEAV/linux/usr/include
   endif
else
   BVC5_DEFINES += BVC5_KERNELMODE
endif
