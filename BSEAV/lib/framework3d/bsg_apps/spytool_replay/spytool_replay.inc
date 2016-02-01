###############################################################################
# Copyright (c) 2008 Broadcom Europe Limited.
# All rights reserved.
#
# Project  :  spytool_replay
# Module   :
#
###############################################################################

CXXFLAGS += -I$(ROCKFORD_TOP)/middleware/$(V3D_PREFIX)/tools/$(HOOK_NAME)
CXXFLAGS += -I$(ROCKFORD_TOP)/middleware/$(V3D_PREFIX)/driver
CXXFLAGS += -I$(ROCKFORD_TOP)/middleware/$(V3D_PREFIX)/driver/interface/khronos/include

SOURCES =             \
   spytool_replay.cpp \
   CapturePost.cpp    \
   CapturePre.cpp		 \
   Command.cpp        \
   Loader.cpp         \
   ../../../../../rockford/middleware/$(V3D_PREFIX)/tools/$(HOOK_NAME)/packet.cpp

TARGET := spytool_replay
