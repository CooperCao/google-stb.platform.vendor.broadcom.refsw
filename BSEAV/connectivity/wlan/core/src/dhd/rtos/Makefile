#
# GNUmakefile for cxnn2/libcx
#
# Copyright (c) 2005, Broadcom Corp.
# $Id$

# TODO:
# TODO: THIS MAKEFILE NEEDS TO BE CHANGED TO use src/wl/config model by rtos
# TODO: dhd owners.
# TODO: List of source files need to come from WLFILES and flags from WLFLAGS
# TODO:

WLAN_ComponentsInUse := bcmwifi ppr
include ../../makefiles/WLAN_Common.mk
SRCBASE := $(WLAN_SrcBaseR)

# Nucleus ARM RVDS Build Environment
# ARMROOT is set, when ARM tool chain is deployed locally.
# If not supply "make ARMROOT=<your-arm-path>" if Z: drive isn't accessible
ifndef ARMROOT
  export ARMROOT     ?= Z:\\projects\\hnd\\tools\\win\\ARM_RVDS
  export ARMDIR      ?= $(ARMROOT)\\RVCT
  export ARMBIN      ?= $(ARMDIR)\\Programs\\2.2\\349\\win_32-pentium
  export ARMLIB      ?= $(ARMDIR)\\Data\\2.2\\349\\lib
  export ARMINC      ?= $(ARMDIR)\\Data\\2.2\\349\\include\\windows
  export RVCT22INC   ?= $(ARMINC)
  export IS_RVDS_2_2 ?= TRUE
  export LM_LICENSE_FILE ?= 38224@lic-sj1-300.sj.broadcom.com
  export PATH        := $(shell cygpath -u $(ARMBIN)):$(PATH)
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

# WPS base directory
WPS_DIR := $(SRCBASE)/wps/rtos

# epi_ttcp base directory
EPI_TTCP_DIR := $(SRCBASE)/tools/misc

# Include generic environment makefile.
include $(SRCBASE)/Makerules

# Default dongle image. This will be embedded in the host driver (dhd).
#   4325b0/sdio-g-cdc-reclaim-idsup-wme-ccx-wapi
# Use this image for Soft AP:
#   4329b1/sdio-g-cdc-reclaim-roml-apsta-af-idauth-idsup
DNGL_IMAGE_NAME ?=


# Search path for source files.
vpath %.c $(addprefix $(SRCBASE)/,bcmsdio/sys dhd/exe dhd/sys shared wl/exe wl/sys) \
   $(WLAN_StdSrcDirsR) $(WLAN_ComponentSrcDirsR)

# Add include directories.
CFLAGS += -I$(SRCBASE)/include
CFLAGS += -I$(SRCBASE)/dhd/sys
CFLAGS += -I$(SRCBASE)/dongle
CFLAGS += -I$(SRCBASE)/dongle/rte/wl/builds/$(DNGL_IMAGE_NAME)
CFLAGS += -I$(SRCBASE)/dongle/rte/wl/$(DNGL_IMAGE_NAME)
CFLAGS += -I$(SRCBASE)/wl/exe
CFLAGS += $(WLAN_ComponentIncPathR)
CFLAGS += $(WLAN_StdIncPathR)


# Basic options
CFLAGS += -DSRCBASE=\"$(SRCBASE)\"
CFLAGS += -DBCMDRIVER
CFLAGS += -DBCMDONGLEHOST
CFLAGS += -DUNRELEASEDCHIP
CFLAGS += -DBCMEMBEDIMAGE=\"rtecdc.h\"
CFLAGS += -DBDC
CFLAGS += -DTOE
CFLAGS += -DDHD_BCMEVENTS
CFLAGS += -DSHOW_EVENTS
CFLAGS += -DBCMSDIO
CFLAGS += -DBCMSDIOH_STD
CFLAGS += -DBWL_NO_FILESYSTEM_SUPPORT
CFLAGS += -DBWL_MOBCOM_DBGPRINTF
CFLAGS += -DBWL_STRICMP
CFLAGS += -DBWL_NO_INTERNAL_STDLIB_SUPPORT
CFLAGS += -DBWL_OSL_USE_STDLIB_MALLOC
CFLAGS += -DBWL_SMALL_WLU_DUMP_BUF

# Enable queueing in host
CFLAGS += -DPROP_TXSTATUS
#CFLAGS += -DPROP_TXSTATUS_DEBUG

# PNO options.
CFLAGS += -DWLPFN
CFLAGS += -DWLPFN_AUTO_CONNECT

# Enable these debug options for now. Maybe add a separate debug target later.
CFLAGS += -DDHD_DEBUG


#ifdef BCMWPA2
CFLAGS += -DBCMWPA2
#endif

#ifdef WLCNT
CFLAGS += -DWLCNT
#endif

# Use newer 11ac ratespec for wl command line
CFLAGS += -DD11AC_IOTYPES

# Use ppr opaque structure in utility build
CFLAGS += -DPPR_API

ifeq ($(WLTEST),1)
	CFLAGS += -DWLTEST
	CFLAGS += -DIOCTL_RESP_TIMEOUT=8000
	CFLAGS += -DDHD_SPROM
endif

# remote WL server transport selection (dongle serial or wifi)
CFLAGS += -DRWL_DONGLE
#CFLAGS += -DRWL_WIFI

# APSTA options
CFLAGS += -DAP
CFLAGS += -DAPSTA_PINGTEST

# Source files.
CFILES := \
	aiutils.c \
	bcmotp.c \
	bcmsrom.c \
	bcmsdh.c \
	bcmsdh_generic.c \
	bcmevent.c \
	bcmutils.c \
	hnd_pktq.c \
	hnd_pktpool.c \
	bcmwifi_channels.c \
	dhd_cdc.c \
	dhd_wlfc.c \
	dhd_common.c \
	dhd_debug.c \
	dhd_mschdbg.c \
	dhd_ip.c \
	dhd_osl_ext.c \
	dhd_sdio.c \
	dhdu.c \
	dhdu_generic.c \
	generic_osl.c \
	hndpmu.c \
	lbuf.c \
	miniopt.c \
	bcm_app_utils.c \
	nvramstubs.c \
	pkt_lbuf.c \
	pkt_lbuf_generic.c \
	sbutils.c \
	siutils.c \
	wlu.c \
	wlu_common.c \
	wlu_cmd.c \
	wlu_iov.c \
	wlu_generic.c \
	wlu_server_generic.c \
	wlu_server_shared.c \
	wlu_pipe.c \
	wlu_rates_matrix.c \
	wlc_ppr.c \
	wlm.c \
	wlu_subcounters.c

# Operating system specific source files.
CFILES += $(TARGETOS)_osl.c
CFILES += $(TARGETOS)_osl_ext.c


ifeq ($(WLTEST),1)
	CFILES += bcmsrom.c bcmotp.c
endif

DEP_FILES := $(wildcard $(OBJDIR)/*.d)
OBJ_FILES := $(addprefix $(OBJDIR),$(CFILES:.c=.o))

TARGETLIB = $(OBJDIR)/libdhd.a

# Create output directory.
$(shell mkdir -p $(OBJDIR))


.PHONY: all
all: libs

.PHONY: libs
libs: libdhd

libdhd: $(TARGETLIB)
$(TARGETLIB): $(OBJ_FILES)
	$(AR) $(TARGETLIB) $^

libwps:
	$(MAKE) -C $(SRCBASE)/wps/rtos

libepittcp:
	$(MAKE) -C $(EPI_TTCP_DIR) -f rtos_utils.mk

.PHONY: clean
clean:
	@rm -f $(OBJ_FILES) $(DEP_FILES) $(TARGETLIB)
	$(MAKE) -C $(WPS_DIR) $@
	$(MAKE) -C $(EPI_TTCP_DIR) -f rtos_utils.mk $@

.PHONY: clobber
clobber:
	rm -Rf $(OBJDIR)
	$(MAKE) -C $(WPS_DIR) $@
	$(MAKE) -C $(EPI_TTCP_DIR) -f rtos_utils.mk $@

.PHONY: clobber_all
clobber_all:
	rm -Rf $(SUPPORTED_TARGETENVS)
	$(MAKE) -C $(WPS_DIR) $@
	$(MAKE) -C $(EPI_TTCP_DIR) -f rtos_utils.mk $@

# Include dependencies.
ifeq ($(strip $(filter clean% clobber%, $(MAKECMDGOALS))),)
   ifneq ($(DEP_FILES),)
      include $(DEP_FILES)
   endif
endif

#test
