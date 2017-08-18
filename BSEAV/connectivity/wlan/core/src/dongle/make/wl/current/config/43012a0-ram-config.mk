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
BASE_FW_IMAGE := sdio-g-p2p-proptxstatus-nocis
BASE_FW_IMAGE_MFGTEST := sdio-ag-threadx

TARGET_OPTIONS_config_sdio_debug := $(BASE_FW_IMAGE)-noap-wowl-ulp-fcbs-dsi

TARGET_OPTIONS_config_sdio_assert := $(BASE_FW_IMAGE)-assert

# For SVT STA Security Testing
TARGET_OPTIONS_config_sdio_noap := $(BASE_FW_IMAGE)-noap

# For Mfgtest Release
TARGET_OPTIONS_config_sdio_mfgtest :=  $(BASE_FW_IMAGE_MFGTEST)-mfgtest

# For olympic Mfgtest Release
TARGET_OPTIONS_config_sdio_olympic_mfgtest :=  $(BASE_FW_IMAGE_MFGTEST)-mfgtest
