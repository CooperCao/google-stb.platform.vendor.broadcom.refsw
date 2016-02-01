############################################################
#     Copyright (c) 2009-2014, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
############################################################
#
# Conditional Options for this Makefile
#
# N/A
#

V3D_HAS_BPCM = $(shell test -e $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_v3d_bpcm.h && echo y)
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
				$(V3D_MAGNUM_TOP)/src/$(B_REFSW_OS)/bvc5_os.c

ifeq ($(B_REFSW_ANDROID),y)
BVC5_SOURCES += \
      $(V3D_MAGNUM_TOP)/src/$(B_REFSW_OS)/bvc5_android_fences.c \
      $(V3D_MAGNUM_TOP)/src/$(B_REFSW_OS)/bvc5_android_bin_pool_alloc.c
else
BVC5_SOURCES += \
      $(V3D_MAGNUM_TOP)/src/bvc5_fence.c \
      $(V3D_MAGNUM_TOP)/src/bvc5_bin_pool_alloc.c
endif


# Sanity Check OS provided
BVC5_SUPPORTED_OS := linuxuser linuxkernel linuxclient linuxemu linuxpal
ifeq ($(findstring $(B_REFSW_OS), $(BVC5_SUPPORTED_OS)),)
$(error [VC5] OS must be one of the following: $(BVC5_SUPPORTED_OS))
endif

BVC5_DEFINES += BVC5_HARDWARE_REAL
ifeq ($(findstring $(B_REFSW_OS), linuxuser linuxclient linuxemu linuxpal),${B_REFSW_OS})
BVC5_DEFINES += BVC5_USERMODE
else
BVC5_DEFINES += BVC5_KERNELMODE
endif
