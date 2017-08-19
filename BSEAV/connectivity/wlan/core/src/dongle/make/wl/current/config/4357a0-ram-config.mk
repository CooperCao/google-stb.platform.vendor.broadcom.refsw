# Config makefile that maps config based target names to features.
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

# Variables that map config names to features - 'TARGET_OPTIONS_config_[bus-type]_xxx'.

TARGET_OPTIONS_config_pcie_base    := ag-splitbuf-pktctx-proptxstatus-pno-nocis-keepalive-aoe-idsup-wapi-ve-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-txbf-logtrace_pcie-srscan-txpwrcap-err-ecounters-hchk-bssinfo-bdo-tko-chanim-wlassocnbr-mcnx-ddchk-mscheventlog-ucm
TARGET_OPTIONS_config_pcie_base2   := pcie-ag-splitbuf-pktctx-proptxstatus-nocis-aoe-idsup-ndoe-pf2-wl11u-anqpo-logtrace_pcie-err-bssinfo-chanim-mcnx-clm_min
TARGET_OPTIONS_config_pcie_min    := pcie-ag-splitbuf-pktctx-proptxstatus-srscan-mcnx-mscheventlog
TARGET_OPTIONS_config_pcie_reduce    := pcie-ag-splitbuf-pktctx-proptxstatus-idsup-logtrace_pcie-dlystat-dmaindex32-srscan-mcnx-txbf-txpwrcap-bssinfo-chanim-sr-srpwr-epa-pwrstats-clm_min-ddchk-consuartseci-apsta-err-assert-logtrace-mscheventlog
TARGET_OPTIONS_config_pcie_min_nan   := pcie-ag-splitbuf-pktctx-proptxstatus-srscan-mcnx-clm_min-logtrace_pcie-err-nan-dbgnan-norsdb

TARGET_OPTIONS_config_pcie_release      := pcie-$(TARGET_OPTIONS_config_pcie_base)-clm_min
TARGET_OPTIONS_config_pcie_debug        := $(TARGET_OPTIONS_config_pcie_release)-assert-err-logtrace-apsta
TARGET_OPTIONS_config_pcie_debug_sfd        := $(TARGET_OPTIONS_config_pcie_release)-assert-err-logtrace-apsta-sfd
TARGET_OPTIONS_config_pcie_debug_atf    := $(TARGET_OPTIONS_config_pcie_release)-assert-err-logtrace-apsta-atfd
TARGET_OPTIONS_config_pcie_debug_norsdb	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-logtrace-norsdb
TARGET_OPTIONS_config_pcie_debug_rsdb	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-logtrace
TARGET_OPTIONS_config_pcie_debug_rsdb2	:= $(TARGET_OPTIONS_config_pcie_base2)-assert-err-logtrace
TARGET_OPTIONS_config_pcie_awdl		:= $(TARGET_OPTIONS_config_pcie_debug)-awdl-norsdb
TARGET_OPTIONS_config_pcie_awdl_debug		:= $(TARGET_OPTIONS_config_pcie_debug)-awdl-norsdb-mschdbg
TARGET_OPTIONS_config_pcie_nan_base	   := $(TARGET_OPTIONS_config_pcie_debug_norsdb)-nan-nonanavail
TARGET_OPTIONS_config_pcie_nan        := $(TARGET_OPTIONS_config_pcie_debug_norsdb)-nan
TARGET_OPTIONS_config_pcie_nan_range     := $(TARGET_OPTIONS_config_pcie_min_nan)-proxd-nanrange-assert-heapcheck
TARGET_OPTIONS_config_pcie_nan_rsdb        := $(TARGET_OPTIONS_config_pcie_debug_rsdb)-nan
TARGET_OPTIONS_config_pcie_dbgnan        := $(TARGET_OPTIONS_config_pcie_debug_norsdb)-nan-dbgnan
TARGET_OPTIONS_config_pcie_dbgnan_rsdb        := $(TARGET_OPTIONS_config_pcie_debug_rsdb2)-nan-dbgnan-srampmac-utrace-dbguc-cbndstate-dbgrsdb
TARGET_OPTIONS_config_pcie_natoe        := $(TARGET_OPTIONS_config_pcie_release)-apsta-natoe
TARGET_OPTIONS_config_pcie_dbgnatoe     := $(TARGET_OPTIONS_config_pcie_natoe)-dbgnatoe
TARGET_OPTIONS_config_pcie_awdl_min                := $(TARGET_OPTIONS_config_pcie_min)-awdl
TARGET_OPTIONS_config_pcie_awdl_min_norsdb               := $(TARGET_OPTIONS_config_pcie_min)-awdl-norsdb

# mfgtst target
BASE_FW_IMAGE := ag-pno-aoe-pktfilter-keepalive-pktctx-proptxstatus-ampduhostreorder-mscheventlog
TARGET_OPTIONS_config_pcie_mfgtest_base	:= pcie-$(BASE_FW_IMAGE)-mfgtest-seqcmds-txbf-logtrace-assert-err-epa-oclf
TARGET_OPTIONS_config_pcie_mfgtest	:= $(TARGET_OPTIONS_config_pcie_mfgtest_base)-dbgmac
TARGET_OPTIONS_config_pcie_mfgtest_debug	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-assert
TARGET_OPTIONS_config_pcie_mfgtest_mu	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-murx-dbgmu-txbf-dump
#TARGET_OPTIONS_config_pcie_mfgtest_ate	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-ate
TARGET_OPTIONS_config_pcie_tput		:= pcie-$(BASE_FW_IMAGE)-logtrace-err-epa-tput-dbgam-dbgams-dbgmac-dbgrsdb-mchan-murx-dbgmu-txbf
TARGET_OPTIONS_config_pcie_tput_atf	:= pcie-$(BASE_FW_IMAGE)-logtrace-err-epa-tput-dbgam-dbgams-dbgmac-dbgrsdb-mchan-atfd
TARGET_OPTIONS_config_pcie_mfgtest_sr	:= $(TARGET_OPTIONS_config_pcie_mfgtest_base)-sr-srpwr-dbgsr

TARGET_OPTIONS_config_pcie_mfgtest_vasip	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-vasip
TARGET_OPTIONS_config_pcie_mfgtest_idma	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-idma
TARGET_OPTIONS_config_pcie_mfgtest_ifrm	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-ifrm
TARGET_OPTIONS_config_pcie_mfgtest_d2hmsi	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-d2hmsi
TARGET_OPTIONS_config_pcie_mfgtest_dma1	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-dma1
TARGET_OPTIONS_config_pcie_mfgtest_dma2	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-dma2
TARGET_OPTIONS_config_pcie_mfgtest_dma1_dma2	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-dma1-dma2
TARGET_OPTIONS_config_pcie_mfgtest_uart	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-consuartseci
TARGET_OPTIONS_config_pcie_mfgtest_regtrydbg	:= $(TARGET_OPTIONS_config_pcie_mfgtest)-regtrydbg
TARGET_OPTIONS_config_sdio_release	:= sdio-$(BASE_FW_IMAGE)
TARGET_OPTIONS_config_sdio_mfgtest	:= $(TARGET_OPTIONS_config_sdio_release)-assert-err-mfgtest-seqcmds-txbf
TARGET_OPTIONS_config_sdio_mfgtest_ate := $(TARGET_OPTIONS_config_sdio_release)-assert-err-mfgtest-seqcmds-ate

# Below are the build targets for UDM dhd.
TARGET_OPTIONS_config_pcie_release_udm := $(TARGET_OPTIONS_config_pcie_release)-norsdb-monitor
TARGET_OPTIONS_config_pcie_debug_udm   := $(TARGET_OPTIONS_config_pcie_release_udm)-assert
TARGET_OPTIONS_config_pcie_tput_udm := $(TARGET_OPTIONS_config_pcie_tput)-norsdb-monitor

# build targets with modules offloaded to host memory
TARGET_OPTIONS_config_pcie_hmemtko := $(TARGET_OPTIONS_config_pcie_debug_norsdb)-hmemtko
