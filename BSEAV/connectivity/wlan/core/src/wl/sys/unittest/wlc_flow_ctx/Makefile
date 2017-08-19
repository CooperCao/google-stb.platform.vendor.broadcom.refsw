# Makefile for wlc flow context table unit test.
#
# $ Copyright Broadcom Corporation $
#
#
# <<Broadcom-WL-IPTag/Proprietary:.*>>
#
# $Id:$

include ../../../../makefiles/WLAN_Common.mk

#####################################################################################
##################### Following items require developer customization. ##############
#####################################################################################

# Source file path
SOURCE = ../../wlc_flow_ctx.c

# Header file path
INCLUDE = $(WLAN_ComponentIncDirsR) $(WLAN_IncDirsR)

# Unit tests file path
TEST_SRC = main.c test_flow_tbl.c stubs.c

# Check tool directory path (from the unit test source directory)
CHECK_ROOT_PATH = ../../../../unittest

# Relevant flags for wlc_flow_ctx.c compilation.
DEBUGS = -DBCMDBG_ERR -DBCMDBG_ASSERT -DBCMDBG_EXCLUDE_HW_TIMESTAMP -DBCMDBG_DUMP #-DBCMDBG

CFLAGS += -DWL_UNITTEST -DBCMDRIVER -DWLC_HIGH -DWLC_LOW -DAP -DMAXSCB=64 $(DEBUGS)

#####################################################################################
##################### None customized parameter #####################################
#####################################################################################

# Path to central make file
include $(CHECK_ROOT_PATH)/unittest.mk
