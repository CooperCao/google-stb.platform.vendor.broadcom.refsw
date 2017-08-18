#
# GNUmakefile for wl/exe
#
# Copyright (c) 2000, Broadcom Corp.
# $Id: GNUmakefile,v 1.2 2008-04-02 14:46:04 $

ifndef	SRCBASE
	SRCBASE = ../../../../..
endif

UNAME = $(shell uname)

#-----------------------------------------------------------------
# Windows build
# 1) windows, don't include Makerules due to all: conflict

ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)

include ../../../../../GNUmakefile.inc

endif
