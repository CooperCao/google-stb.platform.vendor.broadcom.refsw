# Makefile for hndrte based 43684a0 full ram Image
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

# chip specification
CHIP		:= 43684
REV		:= a0
REVID		:= 0

#need to set correct values
#PCIEREVID	:= 24
#CCREV		:= 61
#PMUREV		:= 33
#GCIREV		:= 12
#AOBENAB		:= 1
#OTPWRTYPE	:= 2
#BUSCORETYPE	:= PCIE2_CORE_ID

EXTRA_DFLAGS	+= -DSI_ENUM_BASE_DEFAULT=0x28000000
EXTRA_DFLAGS	+= -DSI_WRAP_BASE_DEFAULT=0x28100000
EXTRA_DFLAGS	+= -DSI_ARMCA7_RAM=0x00000000
EXTRA_DFLAGS	+= -DL2_PAGETABLE_ARRAY_SIZE=0

# default targets
TARGETS		:= \
	pcie-ag-mfgtest-seqcmds-phydbg \
	pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-pktfilter-pf2-keepalive \
	pcie-ag-p2p-mchan-idauth-idsup-pno-aoe-noe-ndoe-pktfilter-pf2-keepalive-toe-ccx-mfp-anqpo-p2po-wl11k-wl11u-wnm-relmcast-txbf-fbt-tdls-sr-pktctx-amsdutx-proxd \
	pcie-ag-fdap-mbss-mfp-wl11k-wl11u-wnm-txbf-pktctx-amsdutx-ampduretry-proptxstatus-chkd2hdma-11nprop-obss-dbwsw-ringer-dmaindex16-bgdfs-hostpmac-murx

TEXT_START	:= 0x0
DATA_START	:= 0x100

# common target attributes
TARGET_ARCH	:= arm
# actually, it is a ca7v2
TARGET_CPU	:= ca7
TARGET_HBUS	:= pcie
THUMB		:= 1
HBUS_PROTO	:= msgbuf

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_43684a0_dev
WLTUNEFILE	:= wltunable_rte_43684a0.h

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (pci)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif
REMAIN := $(subst /,,$(REMAIN))

# features
MEMBASE		:= $(TEXT_START)
# 7MB of memory, reserve 1MB for BMC, so 6MB for SW
MEMSIZE		:= 6291456
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 1
DBG_ERROR	:= 1
WLRXOV		:= 0
PROP_TXSTATUS	:= 1

# Memory reduction features:
# - HNDLBUFCOMPACT: Compacts head/end pointers in lbuf to single word
#   To disable set HNDLBUFCOMPACT = 0
# - BCMPKTIDMAP: Suppresses pkt pointers to Ids in lbuf<next,link>, pktpool, etc
#   Must specify max number of packets (various pools + heap)
# 43684a0 has >2M MEMSIZE, so need to disable HNDLBUFCOMPACT feature.
HNDLBUFCOMPACT	:= 0
BCMPKTIDMAP     := 1
BCMFRAGPOOL	:= 1
BCMRXFRAGPOOL	:= 1
BCMLFRAG	:= 1
BCMSPLITRX	:= 1

DLL_USE_MACROS	:= 1
HNDLBUF_USE_MACROS := 1

POOL_LEN_MAX    := 2048
POOL_LEN        := 32
WL_POST_FIFO1   := 2
MFGTESTPOOL_LEN := 10
FRAG_POOL_LEN	:= 2048
RXFRAG_POOL_LEN	:= 512
PKT_MAXIMUM_ID  := 5120

# Because 43684 cannot support HNDLBUFCOMPACT memory enhancement feature,
# the lbuf size will be 4-bytes larger than HNDLBUFCOMPACT enabled (4365B1).
# By default MAXPKTFRAGSZ is 338 and the PKTTAILROOM will be not enough in TKIP
# case. Add more 4-bytes for 43684 here.
EXTRA_DFLAGS	+= -DMAXPKTFRAGSZ=342

H2D_DMAQ_LEN	:= 256
D2H_DMAQ_LEN	:= 256

PCIE_NTXD	:= 256
PCIE_NRXD	:= 256

WL_SPLITRX_MODE	:= 2
WL_CLASSIFY_FIFO := 2
ifeq ($(WL_SPLITRX_MODE),2)
EXTRA_DFLAGS	+= -DFORCE_RX_FIFO1
endif

ifeq ($(call opt,ate),1)
	TARGET_HBUS     := sdio
endif

ifeq ($(call opt,mfgtest),1)
	#allow MFG image to write OTP
	BCMNVRAMR	:= 0
	BCMNVRAMW	:= 1
endif

ifeq ($(call opt,fdap),1)
	#router image, enable router specific features
	CLM_TYPE			:= 43684a0_access

	AP				:= 1
	WLC_DISABLE_DFS_RADAR_SUPPORT	:= 0
	# Max MBSS virtual slave devices
	MBSS_MAXSLAVES			:= 8
	# Max Tx Flowrings, 128 STAs plus 16 for BCMC
	PCIE_TXFLOWS			:= 144

	EXTRA_DFLAGS			+= -DPKTC_FDAP
else
	# CLM info
	CLM_TYPE			:= 43684a0
endif

BIN_TRX_OPTIONS_SUFFIX := -x 0x0 -x 0x0 -x 0x0

# Reduce stack size to increase free heap
HNDRTE_STACK_SIZE	:= 4608
EXTRA_DFLAGS		+= -DHNDRTE_STACK_SIZE=$(HNDRTE_STACK_SIZE)

# Add flops support
FLOPS_SUPPORT	:= 0

#Enable GPIO
EXTRA_DFLAGS	+= -DWLGPIOHLR

TOOLSVER	:= 2013.11
NOFNRENAME	:= 0
NOINLINE        :=

# Hard code some PHY characteristics to reduce RAM code size
# RADIO
EXTRA_DFLAGS	+= -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS	+= -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS	+= -DBCMRADIOID=$(BCMRADIOID)
# Only support EPA
EXTRA_DFLAGS	+= -DWLPHY_EPA_ONLY -DEPA_SUPPORT=1
# Don't support PAPD
EXTRA_DFLAGS	+= -DEPAPD_SUPPORT=0 -DWLC_DISABLE_PAPD_SUPPORT -DPAPD_SUPPORT=0

EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=64
PKTC_DONGLE	:= 1
EXTRA_DFLAGS	+= -DPCIEDEV_USE_EXT_BUF_FOR_IOCTL
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

# Ideal: (MAX_HOST_RXBUFS > (RXFRAG_POOL_LEN + POOL_LEN)); At least (MAX_HOST_RXBUFS > (WL_POST + WL_POST_FIFO1)) for pciedev_fillup_rxcplid callback from pktpool_get
# Also increase H2DRING_RXPOST_MAX_ITEM to match WL_POST
EXTRA_DFLAGS	+= -DMAX_HOST_RXBUFS=512

#wowl gpio pin 14, Polarity at logic low is 1
WOWL_GPIOPIN	:= 0xe
WOWL_GPIO_POLARITY := 0x1
EXTRA_DFLAGS    += -DWOWL_GPIO=$(WOWL_GPIOPIN) -DWOWL_GPIO_POLARITY=$(WOWL_GPIO_POLARITY)

# max fetch count at once
EXTRA_DFLAGS    += -DPCIEDEV_MAX_PACKETFETCH_COUNT=64
EXTRA_DFLAGS	+= -DPCIEDEV_MAX_LOCALBUF_PKT_COUNT=512
EXTRA_DFLAGS    += -DPD_NBUF_D2H_TXCPL=32
EXTRA_DFLAGS    += -DPD_NBUF_D2H_RXCPL=32
# PD_NBUF_H2D_RXPOST * items(32) > MAX_HOST_RXBUFS for pciedev_fillup_haddr=>pciedev_get_host_addr callback from pktpool_get
EXTRA_DFLAGS    += -DPD_NBUF_H2D_RXPOST=16
EXTRA_DFLAGS    += -DMAX_TX_STATUS_QUEUE=256
EXTRA_DFLAGS    += -DMAX_TX_STATUS_COMBINED=128

# Size of local queue to store completions
EXTRA_DFLAGS    += -DPCIEDEV_CNTRL_CMPLT_Q_SIZE=16

# RxOffsets for the PCIE mem2mem DMA
EXTRA_DFLAGS    += -DH2D_PD_RX_OFFSET=0
EXTRA_DFLAGS    += -DD2H_PD_RX_OFFSET=0

# Support for SROM format
EXTRA_DFLAGS	+= -DBCMPCIEDEV_SROM_FORMAT

# Support for sliding window within flowrings
# This allows an option to set a large flowring size, but operate in a sliding
# window model where dongle only consumes packets upto the window size.
EXTRA_DFLAGS    += -DFLOWRING_SLIDING_WINDOW -DFLOWRING_SLIDING_WINDOW_SIZE=512

# Support for using smaller bitsets on each flowring - instead of the full flowring depth
EXTRA_DFLAGS    += -DFLOWRING_USE_SHORT_BITSETS

# Enabled BCMDBG for QT bring-up
#EXTRA_DFLAGS	+= -DBCMDBG -DBCMDBG_BOOT

EXTRA_DFLAGS	+= -DLARGE_NVRAM_MAXSZ=8192
