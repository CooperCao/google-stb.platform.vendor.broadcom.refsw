#
# Windows tools Makefile
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#

## ethereal cvs module is not tagged regularly with rest of the

ifndef TAG
  TAG = HEAD
endif

SHELL   := bash.exe
DATE    := $(shell date +"%Y%m%d")
MSTOOLS := $(subst /,\\,$(MSDEV))

# CVS modules to check out
CVSMODULES := ethereal

# Miscellaneous tools

all: profile.log ethereal
	@date +"END: win_tools tag=${TAG} %D %T" | tee -a profile.log

# Checkout sources
checkout:
	@date +"START:   $@, %D %T" | tee -a profile.log
ifneq ($(SRC_CHECKOUT_DISABLED),1)
        # Export sources and remove keyword information
	cvs -Q export $(if $(TAG),-r $(TAG)) -kk -kb $(CVSMODULES)
endif # SRC_CHECKOUT_DISABLED
	@date +"END:   $@, %D %T" | tee -a profile.log

# Create batch file to build ethereal in a command shell
ethereal.bat: checkout
	echo "set SRCBASE=$(shell cygpath -w $(PWD))\src" > $@
	echo "cd src\tools\ethereal" >> $@
	echo "call \"$(MSTOOLS)\VC98\Bin\vcvars32.bat\"" >> $@
        # Remove UNIX generated files
	echo "call cleanbld.bat" >> $@
        # Don't confuse nmake with GNU make flags
	echo "set MAKEFLAGS=" >> $@
        # Build ethereal
	echo "nmake -f makefile.nmake" >> $@
        # Build installer
	echo "cd packaging\nsis" >> $@
	echo "nmake -f makefile.nmake" >> $@

ethereal: ethereal.bat
	@date +"START:   $@, %D %T" | tee -a profile.log
	cmd /c $<
	@date +"END:   $@, %D %T" | tee -a profile.log

profile.log :
	@date +"START: win_tools tag=${TAG} %D %T" | tee profile.log

.PHONY: all ethereal
