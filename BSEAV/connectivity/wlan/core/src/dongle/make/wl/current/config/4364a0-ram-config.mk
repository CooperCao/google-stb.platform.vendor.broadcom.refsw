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
# <<Broadcom-WL-IPTag/Proprietary:>>
#
# $Id$

# Variables that map config names to features - 'TARGET_OPTIONS_config_[bus-type]_xxx'.
TARGET_OPTIONS_config_pcie_release				:= pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-err-logtrace-betdls-idsup-ddchk
TARGET_OPTIONS_config_pcie_release_row				:= $(TARGET_OPTIONS_config_pcie_release)-xorcsum
TARGET_OPTIONS_config_pcie_release_norsdb			:= $(TARGET_OPTIONS_config_pcie_release)-norsdb
TARGET_OPTIONS_config_pcie_release_norsdb_row			:= $(TARGET_OPTIONS_config_pcie_release_row)-norsdb
TARGET_OPTIONS_config_pcie_mfgtest				:= pcie-ag-msgbuf-splitrx-p2p-mfgtest
TARGET_OPTIONS_config_pcie_mfgtest_row				:= $(TARGET_OPTIONS_config_pcie_mfgtest)-xorcsum
TARGET_OPTIONS_config_pcie_assert_row				:= $(TARGET_OPTIONS_config_pcie_release_row)-assert
TARGET_OPTIONS_config_pcie_assert_norsdb_row			:= $(TARGET_OPTIONS_config_pcie_release_norsdb_row)-assert
