############################################################################
#     Copyright (c) 2003-2011, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
# 
###########################################################################


B_REFSW_DEBUG ?= y
# apply  ODIR conversion for debug/release build
ifdef  ODIR
ifeq    ($(B_REFSW_DEBUG),y)
ODIR	:= $(ODIR).debug
MAGNUM_CFLAGS += -DBDBG_DEBUG_BUILD=1 
else
ODIR 	:= $(ODIR).release
endif
else
ODIR=.
endif

_ODIR := $(ODIR)/.dir
APP := $(ODIR)/$(APP)
