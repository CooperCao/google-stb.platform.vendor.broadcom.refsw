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

TARGET_OPTIONS_config_pcie_base	:= pcie-ag-splitrx-splitbuf-proptxstatus-nocis-sr-logtrace_pcie-srscan-clm_min-idsup-arpoe-txpwrcap-keepalive
#-srscan-sr-txbf-idsup

TARGET_OPTIONS_config_pcie_nan	:= pcie-ag-splitrx-splitbuf-proptxstatus-txpwrcap-idsup-nocis-clm_min-mcnx-nan-keepalive

TARGET_OPTIONS_config_pcie_dbgnan	:= pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-txpwrcap-nosec-nocis-clm_min-mcnx-nan-logtrace_pcie-dbgnan

TARGET_OPTIONS_config_pcie_nosec:= pcie-ag-splitrx-splitbuf-pktctx-proptxstatus-txpwrcap-nosec-nocis-clm_min
