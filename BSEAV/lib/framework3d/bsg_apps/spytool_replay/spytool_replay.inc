###############################################################################
# Copyright (C) 2008 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
###############################################################################

CXXFLAGS += -I$(BSEAV_DIR)/lib/gpu/$(V3D_PREFIX)/tools/$(HOOK_NAME)
CXXFLAGS += -I$(BSEAV_DIR)/lib/gpu/$(V3D_PREFIX)/driver
CXXFLAGS += -I$(BSEAV_DIR)/lib/gpu/$(V3D_PREFIX)/driver/interface/khronos/include

VPATH = $(BSEAV_DIR)/lib/gpu/$(V3D_PREFIX)/tools/$(HOOK_NAME)

SOURCES =             \
   spytool_replay.cpp \
   Command.cpp        \
   DeviceCaps.cpp     \
   Dispatch.cpp       \
   GetProcAddress.cpp \
   Loader.cpp         \
   packet.cpp         \
   packetreader.cpp

TARGET := spytool_replay
