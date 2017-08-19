# Makefile for standalone programs based on hndrte
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#

WLAN_ComponentsInUse := bcmwifi clm-api ppr olpc keymgmt iocv dump phy dongle rte msch
include ../../../makefiles/WLAN_Common.mk

BASE_DIR := $(WLAN_SrcBaseA)

# rtesim sources
WLCFG_DIR	= $(BASE_DIR)/wl/config
OBJ_DIR		= $(TARGET)

# generate core target list
DEBUG_VARS	= debug nodebug
OPT_VARS	= optnone optspeed optsize
RADIO_VARS	= g ag agn
PROTO_VARS	= cdc rndis rndis-retail rsock
BUS_VARS	= usb sdio

TARGET_LIST := \
$(foreach debug,	$(DEBUG_VARS),			\
$(foreach opt,		$(OPT_VARS),			\
$(foreach radio,	$(RADIO_VARS),			\
$(foreach proto,	$(PROTO_VARS),			\
$(foreach bus,		$(BUS_VARS),			\
$(debug)-$(opt)-$(radio)-$(proto)-$(bus)		\
)))))

# add optional features to the core targets
TARGET_LIST := $(foreach target, $(TARGET_LIST), $(target) $(target)-ccx)
TARGET_LIST := $(foreach target, $(TARGET_LIST), $(target) $(target)-wme)
TARGET_LIST := $(foreach target, $(TARGET_LIST), $(target) $(target)-cram)
TARGET_LIST := $(foreach target, $(TARGET_LIST), $(target) $(target)-wlcq)
TARGET_LIST := $(foreach target, $(TARGET_LIST), $(target) $(target)-wlcnt)
TARGET_LIST := \
	$(foreach target, $(TARGET_LIST), $(target) $(target)-arm7tdmi $(target)-arm7tdmi-thumb)
TARGET_LIST := $(foreach target, $(TARGET_LIST), $(target) $(target)-extcball)

# default target list
#DEFAULT_TARGET_LIST = debug-optnone-ag-cdc-usb-ccx-wme-cram-wlcq-wlcnt-arm7tdmi-thumb
#DEFAULT_TARGET_LIST += debug-optnone-ag-cdc-sdio-ccx-wme-cram-wlcq-wlcnt-arm7tdmi-thumb
DEFAULT_TARGET_LIST = debug-optnone-ag-cdc-usb-ccx-wme-cram-wlcq-wlcnt-arm7tdmi-thumb-extcball
DEFAULT_TARGET_LIST += debug-optnone-ag-cdc-sdio-ccx-wme-cram-wlcq-wlcnt-arm7tdmi-thumb-extcball
DEFAULT_TARGET_LIST += nodebug-optsize-agn-rsock-usb-wlcnt-arm7tdmi-thumb-extcball
DEFAULT_TARGET_LIST += nodebug-optsize-agn-rsock-sdio-wlcnt-arm7tdmi-thumb-extcball
DEFAULT_TARGET_LIST += nodebug-optsize-agn-cdc-usb-wlcnt-arm7tdmi-thumb-extcball
DEFAULT_TARGET_LIST += nodebug-optsize-agn-cdc-sdio-wlcnt-arm7tdmi-thumb-extcball

RTESIM = $(OBJ_DIR)/rtesim

# target specific configuration
ifeq ($(findstring arm7tdmi,$(TARGET)),arm7tdmi)
	CROSS_COMPILE = arm-elf-
# Codesourcery tools
#	CROSS_COMPILE = arm-none-linux-gnueabi-
	EXTRA_CFLAGS += -gdwarf-2 -mcpu=arm7tdmi -mstructure-size-boundary=8
	EXTRA_DFLAGS += -DHNDRTE_PMUTMR=0
	# Add toolchain to the path
	TOOLSVER ?= 4.0.1
endif

TOOLSROOT ?= /projects/hnd/tools/linux
TOOLSDIR ?= $(TOOLSROOT)/hndtools-armelf-linux-$(TOOLSVER)/bin

PATH := $(TOOLSDIR):$(PATH)

ifeq ($(findstring thumb,$(TARGET)),thumb)
	EXTRA_CFLAGS += -mthumb
	EXTRA_DFLAGS += -DTHUMB
endif

ifeq ($(findstring optsize,$(TARGET)),optsize)
	EXTRA_CFLAGS += -Os -fno-unroll-loops -finline-functions -finline-limit=0
else
	ifeq ($(findstring optspeed,$(TARGET)),optspeed)
		EXTRA_CFLAGS += -O3
	endif
endif

BCM_DNGL_BL_PID ?= 0xbdc

ifeq ($(findstring -usb,$(TARGET)),-usb)
	EXTRA_OBJECTS	+= usbdev_sb.o usbdev_cdc.o usbdev_rte.o
	EXTRA_DFLAGS	+= -DUSBBULK_RXBUFS=4 -DUSBCTL_RXBUFS=4 -DBCMUSBDEV \
				-DUSB_NRXD=8 -DUSB_NTXD=8 -DUSB_RXBND=-1
	EXTRA_DFLAGS	+= -DUSB_DNGL
	EXTRA_DFLAGS	+= -DBCM_DNGL_BL_PID=$(BCM_DNGL_BL_PID)
	EXTRA_IFLAGS	+= -I$(BASE_DIR)/usbdev/dongle
endif

ifeq ($(findstring -sdio,$(TARGET)),-sdio)
	EXTRA_OBJECTS	+= sdpcmdev.o sdpcmdev_rte.o
	EXTRA_DFLAGS	+= -DSDPCMD_RXBUFS=4 -DSDPCMD_NRXD=8 -DSDPCMD_NTXD=8 -DSDPCMD_RXBND=-1
	EXTRA_DFLAGS	+= -DSDIO_DNGL
	EXTRA_IFLAGS	+= -I$(BASE_DIR)/sdpcmdev
endif

ifeq ($(findstring -rsock,$(TARGET)),-rsock)
	EXTRA_OBJECTS	+= dngl_rte_rsock.o dngl_todo.o nvram_ro.o
	ifeq ($(findstring arm7tdmi,$(TARGET)),arm7tdmi)
		CSUM_OBJ = csumarm.o
	endif
	ifeq ($(findstring thumb,$(TARGET)),thumb)
		CSUM_OBJ = csumthumb.o
	endif
else
	EXTRA_OBJECTS	+= dngl_rte.o dngl_todo.o nvram.o
endif

EXTRA_CFLAGS	+= -fshort-wchar

ifeq ($(findstring -cdc,$(TARGET)),-cdc)
	EXTRA_OBJECTS	+= bcmcdc.o
	EXTRA_DFLAGS	+= -DHND_STACK_SIZE=12288
endif

EXTRA_DFLAGS	+= -DGLOBAL_STACK

# Remote sockets offload protocol
ifeq ($(findstring -rsock,$(TARGET)),-rsock)
	EXTRA_OBJECTS += bcmcdc.o

	EXTRA_OBJECTS += rserv.o rserv_rte.o rserv_if.o \
			   inet.o mem.o memp.o netif.o dhcp.o pbuf.o \
			   raw.o stats.o sys.o tcp.o tcp_in.o tcp_out.o udp.o etharp.o
	# IPV4 subdir
	EXTRA_OBJECTS += icmp.o ip.o ip_addr.o ip_frag.o

	EXTRA_DFLAGS += -I$(BASE_DIR)/toe/include \
			-I$(BASE_DIR)/toe/rserv \
			-I$(BASE_DIR)/toe/rserv/rte \
			-I$(BASE_DIR)/toe/lwip/src/include \
			-I$(BASE_DIR)/toe/lwip/src/include/ipv4
#	EXTRA_DFLAGS += -DRSOCK -Dsize_t=unsigned
	EXTRA_DFLAGS += -DRSOCK
endif

ifeq ($(findstring -ccx,$(TARGET)),-ccx)
	BCMCCX = 1
	WLRM = 1
endif

ifeq ($(findstring -wlcq,$(TARGET)),-wlcq)
	WLCQ=1
endif

ifeq ($(findstring -wlcnt,$(TARGET)),-wlcnt)
	WLCNT=1
endif

ifeq ($(findstring -agn,$(TARGET)),-agn)
	WLTUNEFILE = wltunable_rte_sim_agn_dev.h
	WL11H = 1
	WL11D = 1
	DBAND = 1
	WL11N=1
	WLAMPDU = 1
	WLAMSDU = 1
	WLAMSDU_SWDEAGG = 1
	BCMSIM_D11PHY = N
else
	ifeq ($(findstring -ag,$(TARGET)),-ag)
		WLTUNEFILE = wltunable_rte_sim_ag_dev.h
		WL11H = 1
		WL11D = 1
		DBAND = 1
	else
		WLTUNEFILE = wltunable_rte_sim_dev.h
	endif
endif

ifeq ($(findstring -wme,$(TARGET)),-wme)
	WME = 1
endif

ifeq ($(findstring nodebug-,$(TARGET)),nodebug-)
	DEBUG = nodebug
else
	ifeq ($(findstring debug-,$(TARGET)),debug-)
		DEBUG = debug
		EXTRA_DFLAGS += -DDEBUG -DBCMDBG -DBCMDBG_MEM -DBCMINTERNAL
	endif
endif

ifeq ($(BCMSIM_D11PHY),N)
	EXTRA_DFLAGS += -DBCMSIM_D11PHY_N
endif

ifeq ($(findstring -retail,$(TARGET)),-retail)
	CONFIG_USBRNDIS_RETAIL = 1
	EXTRA_DFLAGS += -DCONFIG_USBRNDIS_RETAIL
endif

ifeq ($(findstring -extcball,$(TARGET)),-extcball)
	EXTRA_DFLAGS += -DEXT_CBALL -DRAMADDRESS=0x8000000
else
	EXTRA_DFLAGS += -D_RTE_SIM_ -DRTE_POLL -D_NEED_SIZE_T_
endif

# wl driver configuration
UPDATESH	= $(WLCFG_DIR)/diffupdate.sh
WLCONFFILE	= wlconfig_rte_sim_dev
WLRTE		= 1

include $(WLCFG_DIR)/$(WLCONFFILE)
include $(WLCFG_DIR)/wl.mk

# rtesim objects
COMMON_OBJECTS  = sromstubs.o hnddma.o hnddma_rx.o hnddma_tx.o siutils.o sbutils.o aiutils.o hndpmu.o bcmotp.o bcmutils.o

WL_OBJECTS	= $(WLFILES:.c=.o)

RTE_OBJECTS	= rtesim.o hnd_pktq.o hnd_pktpool.o hnd_pkt.o hnd_lbuf.o hnd_pt.o \
		rte_osl.o rte.o hndrte.o rte_dev.o rte_fetch.o rte_pktfetch.o \
		rte_heap.o rte_mem.o rte_isr.o rte_pktpool.o


SIM_OBJECTS	= sbsim.o sbdma.o utils.o 4320sim.o traffic.o chipc.o \
			d11_core.o usbdev.o sdram.o sdpcm_core.o hndrte_sim.o

OBJECTS	= $(WL_OBJECTS) $(RTE_OBJECTS) $(COMMON_OBJECTS) $(EXTRA_OBJECTS)
ifeq ($(findstring -extcball,$(TARGET)),-extcball)
OBJECTS += headarm.o hndarm.o hndchipc.o sflash.o \
	rte_arm.o rte_uart.o rte_cons.o rte_chipc.o \
	hndrte_arm.o rte_pmu.o
OBJECTS += armulatorsim.o $(CSUM_OBJ)
else
OBJECTS += $(SIM_OBJECTS)
endif

OBJS	= $(sort $(patsubst %,$(OBJ_DIR)/%,$(OBJECTS)))

# compiler configuration
CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip

CFLAGS		= -g -fdata-sections -ffunction-sections -Wall -Werror

IFLAGS		= -I$(BASE_DIR)/dongle/hndrte/sim -I$(BASE_DIR)/dongle/rte -I$(OBJ_DIR)
IFLAGS		+= -I$(BASE_DIR)/tools/shared -I$(BASE_DIR)/tools/4710sim
IFLAGS 		+= -I$(BASE_DIR)/rte/posix
IFLAGS		+= $(WLAN_ComponentIncPathR)
IFLAGS		+= $(WLAN_IncPathR)

DFLAGS		= $(WLFLAGS) -DBCMDRIVER -DRAMSZ=0x80000
DFLAGS		+= -Ulinux -D_HNDRTE_ -D_RTE_
EXTRA_DFLAGS	+= -DRTE_CONS

LFLAGS		= $(CFLAGS) -Wl,--gc-sections
EXTRA_LFLAGS	= $(EXTRA_CFLAGS)

# build targets

ifneq	"$(filter $(TARGET), $(TARGET_LIST))"	""

all: $(RTESIM)

else

all:
	($(foreach target, $(DEFAULT_TARGET_LIST),  $(MAKE) $(target) &&) true)

endif

SRCDIRS := $(addprefix $(BASE_DIR)/,shared/nvram tools/4710sim tools/shared \
	   	dongle/hndrte/sim dongle/rte wl/exe) \
	   $(WLAN_StdSrcDirsA) $(WLAN_ComponentSrcDirs)

VPATH += $(SRCDIRS)

ifeq ($(findstring -usb,$(TARGET)),-usb)
VPATH += $(BASE_DIR)/usbdev/dongle
endif

ifeq ($(findstring -sdio,$(TARGET)),-sdio)
VPATH += $(BASE_DIR)/sdpcmdev
endif

ifeq ($(findstring -rsock,$(TARGET)),-rsock)
VPATH += $(BASE_DIR)/toe/shared
VPATH +=$(BASE_DIR)/toe/lwip/src/core \
	$(BASE_DIR)/toe/lwip/src/core/ipv4 \
	$(BASE_DIR)/toe/lwip/src/netif \
	$(BASE_DIR)/toe/rserv \
	$(BASE_DIR)/toe/rserv/rte
endif

# Update the CLM database C code from XML inputs if present.
ifneq (,$(TARGET))
$(call WLAN_GenClmCompilerRule,$(TARGET),$(BASE_DIR),--ccrev all/0 --include Mobility)
endif

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/wlconf.h
	@echo "Compiling $<"
	@$(CC) $(DFLAGS) $(EXTRA_DFLAGS) $(IFLAGS) $(EXTRA_IFLAGS) \
		$(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(OBJ_DIR)/%.c $(OBJ_DIR)/wlconf.h
	@echo "Compiling $<"
	@$(CC) $(DFLAGS) $(EXTRA_DFLAGS) $(IFLAGS) $(EXTRA_IFLAGS) \
		$(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.S
	@echo "Compiling $<"
	@$(CC) -D_LANGUAGE_ASSEMBLY -Wa,-a=$*.lst $(DFLAGS) $(EXTRA_DFLAGS) \
		$(CFLAGS) $(EXTRA_CFLAGS) $(IFLAGS) $(EXTRA_IFLAGS) -c -o $@ $<

# armsd will not run arm-elf-gcc image unless it's stripped of debug info
$(RTESIM): $(OBJ_DIR)/wlconf.h $(BASE_DIR)/include/epivers.h $(OBJS)
	$(LD) $(LFLAGS) $(EXTRA_LFLAGS) -o $@ $(OBJS)
	$(STRIP) -d $(RTESIM)

# Object directory is created by wlconf.h target
$(TARGET_LIST):
	install -d $@
	$(MAKE) TARGET=$@

clean:
	rm -rf debug-* nodebug-* wlc_clm_data.c *.lst

# Use help rule because list of targets is too large for shell
%.help:
	@echo $*

# List all valid targets
helpall: $(foreach target, $(TARGET_LIST), $(target).help)

# List all default targets
helpdef: $(foreach target, $(DEFAULT_TARGET_LIST), $(target).help)

# headarm.S includes startarm.S
$(OBJ_DIR)/headarm.o: startarm.S

$(BASE_DIR)/include/epivers.h:
	$(MAKE) -C $(BASE_DIR)/include

$(OBJ_DIR)/wlconf.h: $(WLCFG_DIR)/$(WLTUNEFILE)
	[ ! -f $@ ] || chmod +w $@
	@echo "check and update config file"
	cp $< $(OBJ_DIR)/wltemp
	$(UPDATESH) $(OBJ_DIR)/wltemp $@

.PHONY : all list $(TARGET_LIST)
