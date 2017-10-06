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

TARGET_OPTIONS_config_pcie_release	:= pcie-ag-err-splitrx
TARGET_OPTIONS_config_pcie_debug	:= pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux
TARGET_OPTIONS_config_pcie_nan_debug	:= $(TARGET_OPTIONS_config_pcie_debug)-nan
TARGET_OPTIONS_config_pcie_tdls_debug	:= pcie-ag-p2p-splitrx-proptxstatus-ampduhostreorder-tdls-assert-redux
TARGET_OPTIONS_config_pcie_aibss_debug	:= pcie-ag-splitrx-pktctx-proptxstatus-ampduhostreorder-aibss-relmcast-btcdyn-redux-assert
TARGET_OPTIONS_config_pcie_nontxqmux	:= pcie-ag-err-assert-splitrx
TARGET_OPTIONS_config_pcie_txqmux	:= pcie-ag-err-assert-splitrx-txqmux
TARGET_OPTIONS_config_pcie_awdl		:= pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-awdl-assert-norsdb-redux
TARGET_OPTIONS_config_pcie_oce		:= pcie-ag-err-assert-splitrx-oce
TARGET_OPTIONS_config_pcie_filsauth		:= pcie-ag-err-assert-splitrx-filsauth
