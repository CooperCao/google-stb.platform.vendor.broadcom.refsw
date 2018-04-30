############################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BV3D

V3D_MAGNUM_TOP ?= $(MAGNUM)/portinginterface/v3d

# Module Includes
BV3D_INCLUDES += $(V3D_MAGNUM_TOP)/include

# Module Sources
BV3D_SOURCES += $(V3D_MAGNUM_TOP)/src/bv3d.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_job.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_iq.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_notifyq.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_qmap.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_waitq.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_callbackmap.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_worker.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_binmem.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_perf_counters.c \
				$(V3D_MAGNUM_TOP)/src/bv3d_event_monitor.c \
				$(V3D_MAGNUM_TOP)/src/$(B_REFSW_OS)/bv3d_os.c

ifeq ($(B_REFSW_ANDROID),y)
BV3D_SOURCES += \
				$(V3D_MAGNUM_TOP)/src/$(B_REFSW_OS)/bv3d_android_fences.c
else
BV3D_SOURCES += \
				$(V3D_MAGNUM_TOP)/src/bv3d_fence.c
endif

# Sanity Check OS provided
BV3D_SUPPORTED_OS := linuxuser linuxkernel linuxclient linuxemu linuxpal
ifeq ($(findstring $(B_REFSW_OS), $(BV3D_SUPPORTED_OS)),)
$(error [v3d] OS must be one of the following: $(BV3D_SUPPORTED_OS))
endif

ifeq ($(findstring $(B_REFSW_OS), linuxuser linuxclient linuxemu linuxpal),${B_REFSW_OS})
BV3D_DEFINES += BV3D_USERMODE
else
BV3D_DEFINES += BV3D_KERNELMODE
endif
