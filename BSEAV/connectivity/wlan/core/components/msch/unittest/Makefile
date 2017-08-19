# Makefile for wlc msch unit test.
#
# $ Copyright Broadcom Corporation $
#
#
# <<Broadcom-WL-IPTag/Proprietary:.*>>
#
# $Id:$

include ../../../src/makefiles/WLAN_Common.mk
#####################################################################################
##################### Following items require developer customization. ##############
#####################################################################################
SHELL := /bin/bash
WL_MSCH_PROFILER := 1

# Source file path
SOURCE = ../src/wlc_msch.c ../src/wlc_mschutil.c ../../../src/wl/sys/wlc_utils.c
SOURCE += ../src/wlc_msch_profiler.c
SOURCE += ../../../src/wl/sys/wlc_hrt.c
# Header file path
INCLUDE = $(WLAN_ComponentIncDirsR) $(WLAN_IncDirsR)

# Unit tests file path
TEST_SRC = main.c test_msch.c stubs.c

# Check tool directory path (from the unit test source directory)
CHECK_ROOT_PATH = ../../../src/unittest
# Relevant flags for wlc_msch.c compilation.
DEBUGS = -DBCMDBG_ERR -DBCMDBG_ASSERT -DBCMDBG_EXCLUDE_HW_TIMESTAMP #-DBCMDBG

ifeq ($(MSCH_PROFILER),1)
	DEBUGS += -DMSCH_PROFILER
	DEBUGS += -DMSCH_TESTING
endif

CFLAGS += -DWL_UNITTEST -DBCMDRIVER -DWLC_HIGH -DWLC_LOW -DAP $(DEBUGS)

#####################################################################################
##################### None customized parameter #####################################
#####################################################################################

# Path to central make file
include $(CHECK_ROOT_PATH)/unittest.mk
