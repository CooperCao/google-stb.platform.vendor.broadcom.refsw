# Makefile for bcmutils unit test.
#
# $Copyright (C) 2014, Broadcom Corporation$
#
# $Id:$

include ../../../makefiles/WLAN_Common.mk

#####################################################################################
##################### Following items require developer customization. ##############
#####################################################################################

# path of source file
SOURCE = ../../bcmutils.c

# path of header file
INCLUDE = ../../../shared/bcmwifi/include ../bcmutils $(WLAN_IncDirsA)

# path of unit test file
TEST_SRC = main.c test_bcm_tlv.c test_bcm_bitop.c

CFLAGS += -DBCMDBG

# Path to Check tool directory from the unit test source directory
CHECK_ROOT_PATH = ../../../unittest

# Relevant flags for bcm_utils.c compilation.
CFLAGS += -DWL_UNITTEST
#####################################################################################
##################### None customized parameter #####################################
#####################################################################################

# Path to central make file
include $(CHECK_ROOT_PATH)/unittest.mk
