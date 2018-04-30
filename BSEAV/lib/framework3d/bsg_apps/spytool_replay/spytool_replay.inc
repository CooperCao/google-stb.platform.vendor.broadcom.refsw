###############################################################################
# Copyright (C) 2008 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
###############################################################################

CXXFLAGS += -I$(BSEAV_DIR)/lib/gpu/tools/gpumon_hook
CXXFLAGS += -I$(BSEAV_DIR)/lib/gpu/tools/gpumon_tools
CXXFLAGS += -I$(BSEAV_DIR)/lib/gpu/$(V3D_PREFIX)/driver
CXXFLAGS += -I$(BSEAV_DIR)/lib/gpu/$(V3D_PREFIX)/driver/interface/khronos/include

ifeq ($(V3D_PREFIX),vc5)
CXXFLAGS += -DHAS_ES3=1
endif

VPATH = $(BSEAV_DIR)/lib/gpu/tools/gpumon_hook \
        $(BSEAV_DIR)/lib/gpu/tools/gpumon_tools

SOURCES =             \
   spytool_replay.cpp \
   Command.cpp        \
   DeviceCaps.cpp     \
   Dispatch.cpp       \
   GetProcAddress.cpp \
   Loader.cpp         \
   packet.cpp         \
   packetreader.cpp   \
   GLStdCommand.cpp   \
   GLStdCommandMap.cpp

TARGET := spytool_replay
