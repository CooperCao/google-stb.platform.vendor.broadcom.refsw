# Makefile for txflow classification unit test.
#
# $ Copyright Broadcom Corporation $
#
#
# <<Broadcom-WL-IPTag/Proprietary:.*>>
#
# $Id:$

include ../../../../../src/makefiles/WLAN_Common.mk

#####################################################################################
##################### Following items require developer customization. ##############
#####################################################################################

# Source file path
SOURCE = ../../src/txflow_classifier.c

# Header file path
INCLUDE = $(WLAN_ComponentIncDirsR) $(WLAN_IncDirsR) ../../../include

# Unit tests file path
TEST_SRC = main.c test_txflow_classifier.c stubs.c

# Check tool directory path (from the unit test source directory)
CHECK_ROOT_PATH = ../../../../../src/unittest

# Relevant flags for txflow_classifier.c compilation.
DEBUGS = -DBCMDBG_ERR -DBCMDBG_ASSERT -DBCMDBG_EXCLUDE_HW_TIMESTAMP -DBCMDBG_DUMP #-DBCMDBG

CFLAGS += -DWL_UNITTEST $(DEBUGS)

#####################################################################################
##################### None customized parameter #####################################
#####################################################################################

# Path to central make file
include $(CHECK_ROOT_PATH)/unittest.mk
