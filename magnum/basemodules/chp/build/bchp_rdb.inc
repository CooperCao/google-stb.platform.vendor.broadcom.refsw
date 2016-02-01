############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
############################################################
BCHP_BUILD_INC := $(MAGNUM)/basemodules/chp/build/bchp_${BCHP_CHIP}.inc
# There are cases where BCHP_CHIP either undefined or has a peculiar value
ifneq ($(wildcard ${BCHP_BUILD_INC}),)
include ${BCHP_BUILD_INC}
ifndef BCHP_RDB_PATH
$(error ${BCHP_BUILD_INC} must define BCHP_RDB_PATH)
endif
else
BCHP_RDB_PATH := $(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)
endif
