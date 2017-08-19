#
# Makefile for iPerf on real-time operating systems (RTOS).
#
# Copyright (c) 2005, Broadcom Corp.
# $Id$


ifeq ($(SRCBASE),)
   SRCBASE := ../../../../..
endif

SUPPORTED_TARGETENVS := nucleusarm

# Error check that target environment is valid.
ifeq ($(filter $(TARGETENV),$(SUPPORTED_TARGETENVS)),)
$(warning Unsupported value for TARGETENV: '$(TARGETENV)')
$(warning TARGETENV must be one of the following:)
$(warning $(space))
$(foreach target,$(SUPPORTED_TARGETENVS),$(warning $(space)  $(target)))
$(error $(space))
endif

# Set output directory.
OBJDIR := $(TARGETENV)/$(TARGETCPU)/

# Include generic environment makefile.
include $(SRCBASE)/Makerules

# iPerf base directory.
IPERF_DIR := $(SRCBASE)/tools/misc/iperf-2.0.4

# lwIP TCP/IP stack base directory.
LWIP_BASE_DIR := $(BSP_BASE_DIR)/lwip


# Search path for source files.
vpath %.c   $(IPERF_DIR)/compat
vpath %.cpp $(IPERF_DIR)/compat
vpath %.c   $(IPERF_DIR)/src
vpath %.cpp $(IPERF_DIR)/src


# Add include directories for iPerf.
CFLAGS += -I$(IPERF_DIR)/include
CFLAGS += -I$(IPERF_DIR)/build/rtos

# Add include directories for lwIP TCP/IP stack.
CFLAGS += -I$(LWIP_BASE_DIR)/dhdnu
CFLAGS += -I$(LWIP_BASE_DIR)/src/include
CFLAGS += -I$(LWIP_BASE_DIR)/src/include/ipv4


# Basic options
CFLAGS += -DHAVE_CONFIG_H

# Use Mobile Communications printf from BSP.
CFLAGS += -DBWL_MOBCOM_DBGPRINTF

# LWIP TCP/IP stack.
CFLAGS += -DBWL_TCPIP_LWIP

# C source files from /iperf/compat.
CFILES :=         \
	error.c        \
	gettimeofday.c \
	inet_ntop.c    \
	inet_pton.c    \
	signal.c       \
	snprintf.c     \
	string.c       \
	Thread.c

# C source files from /iperf/src.
CFILES +=            \
	Extractor.c       \
	gnu_getopt.c      \
	gnu_getopt_long.c \
	Locale.c          \
	ReportCSV.c       \
	ReportDefault.c   \
	Reporter.c        \
	service.c         \
	SocketAddr.c      \
	sockets.c         \
	stdio.c           \
	tcp_window_size.c
   
   
# C++ source files.
CPPFILES :=    \
	delay.cpp
   
# C++ source files from /iperf/src.
CPPFILES +=       \
	Client.cpp     \
	Launch.cpp     \
	List.cpp       \
	Listener.cpp   \
	main.cpp       \
	PerfSocket.cpp \
	Server.cpp     \
	Settings.cpp

DEP_FILES := $(wildcard $(OBJDIR)/*.d)
OBJ_FILES := $(addprefix $(OBJDIR),$(CFILES:.c=.o))
OBJ_FILES += $(addprefix $(OBJDIR),$(CPPFILES:.cpp=.o))

TARGETLIB = $(OBJDIR)/libiperf.a

# Create output directory.
$(shell mkdir -p $(OBJDIR))


.PHONY: all
all: libs

.PHONY: libs
libs: libiperf

libiperf: $(TARGETLIB)
$(TARGETLIB): $(OBJ_FILES) 
	$(AR) $(TARGETLIB) $^


.PHONY: clean
clean:
	@rm -f $(OBJ_FILES) $(DEP_FILES) $(TARGETLIB)

.PHONY: clobber
clobber:
	rm -Rf $(OBJDIR)
   
.PHONY: clobber_all
clobber_all:
	rm -Rf $(SUPPORTED_TARGETENVS)
   
# Include dependencies.
ifeq ($(strip $(filter clean% clobber%, $(MAKECMDGOALS))),)
   ifneq ($(DEP_FILES),)
      include $(DEP_FILES)
   endif
endif
