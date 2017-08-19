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
BASE_FW_IMAGE := sdio-g

# Variables that map config names to features - 'TARGET_OPTIONS_config_[bus-type]_xxx'.
TARGET_OPTIONS_config_sdio_release := $(BASE_FW_IMAGE)

# For olympic Mfgtest Release
TARGET_OPTIONS_config_sdio_olympic_mfgtest :=  $(BASE_FW_IMAGE)-mfgtest-nocis
