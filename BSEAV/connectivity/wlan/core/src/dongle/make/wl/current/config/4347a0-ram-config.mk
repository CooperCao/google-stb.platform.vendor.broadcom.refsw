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
BASE_FW_IMAGE := ag-p2p-pno-aoe-pktfilter-keepalive-mchan-pktctx-proptxstatus-ampduhostreorder-mscheventlog

TARGET_OPTIONS_config_pcie_release	:= pcie-$(BASE_FW_IMAGE)
TARGET_OPTIONS_config_pcie_mfgtest_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-qt
TARGET_OPTIONS_config_pcie_mfgtest_ifrm_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-ifrm-qt
TARGET_OPTIONS_config_pcie_mfgtest_idma_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-idma-qt
TARGET_OPTIONS_config_pcie_mfgtest_d2hmsi_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-d2hmsi-qt
TARGET_OPTIONS_config_pcie_mfgtest_dma1_dma2_qt	:= $(TARGET_OPTIONS_config_pcie_release)-assert-err-mfgtest-seqcmds-dump-dma1-dma2-qt

TARGET_OPTIONS_config_pcie_olym_base	:= pcie-ag-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-mpf-mfp-ltecx-txbf-logtrace_pcie-srscan-txpwrcap-err-swdiv-ecounters-hchk-bssinfo-bdo-chanim-wlassocnbr-mscheventlog
TARGET_OPTIONS_config_pcie_olym_release	:= $(TARGET_OPTIONS_config_pcie_olym_base)-clm_min
TARGET_OPTIONS_config_pcie_olym_debug	:= $(TARGET_OPTIONS_config_pcie_olym_release)-assert-err
