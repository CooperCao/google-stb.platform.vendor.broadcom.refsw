#############################################################################
#
#       Copyright (c) 2007-2014, Broadcom Corporation.
#       All rights reserved.
#       Confidential Property of Broadcom Corporation.
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# File Description:
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################

MAGNUM_DIR=$(shell cd ../../../magnum;pwd)
ROCKFORD_DIR=$(shell cd ../..;pwd)
FRAMEWORK_DIR=$(ROCKFORD_DIR)/appframework

# for ${BCHP_CHIP}
include $(ROCKFORD_DIR)/appframework/build/plat.inc

# Make sure we have appropriate lower-case equivalents of BCHP_VER and BCHP_7411_VER
BCHP_VER_LOWER := $(subst A,a,$(BCHP_VER))
BCHP_VER_LOWER := $(subst B,b,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst C,c,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst D,d,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst E,e,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst F,f,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(subst G,g,$(BCHP_VER_LOWER))
BCHP_VER_LOWER := $(strip $(subst H,h,$(BCHP_VER_LOWER)))

INCLUDE_DIR=$(MAGNUM_DIR)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)

# Put your custom flags here:
UCFLAGS=

CFLAGS= -g $(UCFLAGS)
CC= gcc
COMPILE= $(CC) $(CFLAGS)

all: printrul

printrul: m2mc_hw_pkt.c
	$(COMPILE) -I $(INCLUDE_DIR) m2mc_hw_pkt.c -o printpkt

clean:
	$(RM) printpkt
