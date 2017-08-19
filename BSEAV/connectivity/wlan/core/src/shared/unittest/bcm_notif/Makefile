# Makefile for bcm_notif unit test.
#
# Copyright (C) 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile xxxxxx 2013-11-14 06:00:44Z emanuell,shaib $


#####################################################################################
##################### Following items require developer customization. ##############
#####################################################################################

# path of source file
SOURCE = ../../bcm_notif.c ../../bcm_mpool.c ../../../wl/ppr/src/wlc_ppr.c ../../../shared/bcmutils.c

# path of header file
INCLUDE = ../../  ../../include ../../../include  ../../../wl/sys/ ../../../wl/dos/   ../../../wl/ppr/include ../../../shared/bcmwifi/include ../../../../components/shared

# path of unit test file
TEST_SRC= test_bcm_notif.c

# Path to Check tool directory from the unit test source directory
CHECK_ROOT_PATH = ../../../unittest

# Relevant flags for bcm_notif.c compilation.
CFLAGS += -DWL11H
CFLAGS += -DWL_UNITTEST

#####################################################################################
##################### None customized parameter #####################################
#####################################################################################

# Path to central make file
include $(CHECK_ROOT_PATH)/unittest.mk

