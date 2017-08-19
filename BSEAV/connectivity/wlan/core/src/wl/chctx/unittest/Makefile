# Makefile for Channel Context unit test.
#
# $ Copyright Broadcom Corporation $
#
#
# <<Broadcom-WL-IPTag/Proprietary:.*>>
#
# $Id:$

include ../../../makefiles/WLAN_Common.mk

#####################################################################################
##################### Following items require developer customization. ##############
#####################################################################################

# Source file path
SOURCE = ../src/wlc_chctx_reg.c

# Header file path
INCLUDE = $(WLAN_ComponentIncDirsR) $(WLAN_IncDirsR)

# Unit tests file path
TEST_SRC = main.c test_chctx_reg.c stubs.c

# Check tool directory path (from the unit test source directory)
CHECK_ROOT_PATH = ../../../unittest

# Relevant flags for wlc_flow_ctx.c compilation.
DEBUGS = -DBCMDBG_ERR -DBCMDBG_ASSERT -DBCMDBG_EXCLUDE_HW_TIMESTAMP -DBCMDBG_DUMP -DBCMDBG

CFLAGS += -DWL_UNITTEST -DBCMDRIVER $(DEBUGS)

#####################################################################################
##################### None customized parameter #####################################
#####################################################################################

# Path to central make file
include $(CHECK_ROOT_PATH)/unittest.mk
