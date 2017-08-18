# Makefile for wlc_dump_reg unit test.
#
# $Copyright (C) 2014, Broadcom Corporation$
#
# $Id:$

include ../../../makefiles/WLAN_Common.mk

#####################################################################################
##################### Following items require developer customization. ##############
#####################################################################################

# Source file path
SOURCE = ../src/wlc_dump_reg.c

# Header file path
INCLUDE = ../unittest $(WLAN_ComponentIncDirsR) $(WLAN_IncDirsR)

# Unit tests file path
TEST_SRC = main.c test_wlc_dump_reg.c

# Check tool directory path (from the unit test source directory)
CHECK_ROOT_PATH = ../../../unittest

# Relevant flags for wlc_dump_reg.c compilation.
CFLAGS += -DWL_UNITTEST
#####################################################################################
##################### None customized parameter #####################################
#####################################################################################

# Path to central make file
include $(CHECK_ROOT_PATH)/unittest.mk
