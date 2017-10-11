#
# GNUmakefile for wl/exe
#
# Copyright (c) 2000, Broadcom Corp.
# $Id: GNUmakefile,v 1.3 2009-05-25 06:21:22 $

ifndef	SRCBASE
	SRCBASE = ../../..
endif

UNAME = $(shell uname)

#-----------------------------------------------------------------
# Windows build
# 1) windows, don't include Makerules due to all: conflict

ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)

include ../../../GNUmakefile.inc

else # UNAME

# 2) not windows, need to include first to pick up TARGETENV dependent vars
include $(SRCBASE)/Makerules

#-----------------------------------------------------------------
# Linux build
#
# This should be one of values recognized in src/Makerules

endif # UNAME
