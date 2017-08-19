#
# Helper makefile for building the Adaptive Voltage Scaling component included in wl.mk
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

AVS_TOP_DIR := components/avs

# Files
AVS_SRC := $(AVS_TOP_DIR)/src/avs.c
AVS_SRC += src/wl/sys/wl_avs.c

AVS_FLAGS := -DAVS -DAVS_ENABLE_LVM
AVS_FLAGS += -DDISABLE_DEADMAN						# Temporarily disable, doesn't work for multiple threads

# AVS configuration, to be moved to chip-specific makefiles
AVS_FLAGS += -DAVS_GET_BOARD_DAC_CODE					# Get approximate starting DAC value from board voltage
AVS_FLAGS += -DAVS_FIND_NOMINAL_VOLTAGE					# Find nominal voltage (~1.03V)

# Debugging
AVS_FLAGS += -DAVS_ENABLE_SIMULATION					# Simulate hardware
AVS_FLAGS += -DAVSDBG_ERROR -DAVSDBG_INFORM -DAVS_DEBUG -DBCM_BIG_LOG	# Enable logging
#AVS_FLAGS += -DAVS_LOG_SIM						# Log simulated values
AVS_FLAGS += -DAVS_INIT_AFTER_ATTACH					# Initialize after attach (when thread scheduler is started)
AVS_FLAGS += -DAVS_ENABLE_LOGDATA					# Dump process values to console
