############################################################
#     Copyright (c) 2003-2014, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#  $brcm_Workfile: $
#  $brcm_Revision: $
#  $brcm_Date: $
#
#  Revision History:
#  $brcm_Log: $
#
############################################################

B_THIS_DIR:=BSEAV/lib/livemedia/blive_ext/build
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

#  Sanity check... Make sure that $(NEXUS_TOP) exists and is accessible.
$(if $(realpath ${NEXUS_TOP}),,$(error Cannot find (or access) NEXUS_TOP: "$(NEXUS_TOP)"))

#  More Nexus definitions...
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

#  More sanity checks... By now, the following variables should be defined... fail if not.
$(if ${B_REFSW_ARCH},,$(error B_REFSW_ARCH is not defined))
$(if ${B_REFSW_OBJ_DIR},,$(error B_REFSW_OBJ_DIR is not defined))
$(if ${B_REFSW_OBJ_ROOT},,$(error B_REFSW_OBJ_ROOT is not defined))

include ../include/blive_ext.mak
include ../../livemedia.inc

include ../../../../api/build/tools.mak
BSEAV = ${NEXUS_TOP}/../BSEAV

include $(BSEAV)/build/refsw_inc.mak

BLIVE_EXT_ODIR := ${BLIVE_EXT_LIBDIR}
BLIVE_EXT_LIB = $(BLIVE_EXT_ODIR)/libblive_ext.a

.PHONY: check_environment clean
all: $(BLIVE_EXT_LIB) LM_ADD_TO_LIVEMEDIA

ifeq ($(VERBOSE),)
MAKEFLAGS += -s
endif

ifeq ($(ANDROID_BUILD),y)
CFLAGS += $(NEXUS_CFLAGS)
endif

ifeq ($(SYSTEM),vxworks)
CFLAGS += -I$(WIND_BASE)/target/h
endif
CFLAGS += -W -Wall
CFLAGS += -I$(BLIVE_EXT_DIR)/include -I$(BLIVE_EXT_DIR)/src \
	-I$(BSEAV)/api/include \
	-I$(BSEAV)/lib/rtp \
    ${B_LIVEMEDIA_CFLAGS}

CFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS) -fPIC
CXXFLAGS += $(CFLAGS)

vpath %.cpp $(BLIVE_EXT_DIR)/src

BLIVE_EXT_OBJS = bsap.o bscheduler.o

$(BLIVE_EXT_ODIR)/%.o: %.cpp         |     check_environment
	@echo [Compile... $(notdir $<)]
	${Q_}$(CXX) $(CXXFLAGS) -c $< -o $@

LM_ADD_TO_LIVEMEDIA: $(BLIVE_EXT_ODIR)/lm_lgpl.o
ifeq ($(SPF_SUPPORT),y)
	#Changes for RTSP-ES support
	export SPF_SUPPORT=y

	@echo [Compile... lm_lgpl.cpp ]
	${Q_}$(CXX) -DSPF_SUPPORT $(CXXFLAGS) -c $(BLIVE_EXT_DIR)/src/lm_lgpl.cpp -o $(BLIVE_EXT_ODIR)/lm_lgpl.o
endif

$(BLIVE_EXT_LIB): $(addprefix $(BLIVE_EXT_ODIR)/,$(BLIVE_EXT_OBJS))
	@echo [Linking... $(notdir $@)]
	${Q_}$(RM) $@
	${Q_}$(AR) cr $@ $^
	${Q_}$(RANLIB) $@

clean:
	-${Q_}$(RM) -rf $(BLIVE_EXT_ODIR)

check_environment:  $(BLIVE_EXT_ODIR)

$(BLIVE_EXT_ODIR):
	${Q_}$(MKDIR) "$@"

include $(BSEAV)/api/build/rules.mak
