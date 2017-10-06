#
# Copied part of WL makefile (just for building tags)
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: GNUmakefile,v 1.3 2006-02-04 19:40:52 $
#

SRCBASE	= ../..

TAGFILES := $(shell find . -maxdepth 1 -regex '.*/[^/.]+\.[ch]' -printf "%f ")

# The Emacs-supplied etags needs -r <regexp> to find reclaimables.
ifeq ($(shell if etags -o - -r "" $(firstword $(TAGFILES)) > /dev/null 2>&1; then echo 0; fi),0)
TAGEXP := -r "/BCM\(UN\)?INITFN(\([^(]+\))/"
endif

TAGS: $(TAGFILES)
	etags $(TAGEXP) $(TAGFILES)
tags: $(TAGFILES)
	ctags $(TAGEXP) $(TAGFILES)

MOREFILES := $(shell find $(SRCBASE) -maxdepth 1 -mindepth 1 -not -name router -not -name linux -not -name cfe -exec find '{}' -name '*.[ch]' ';')

MORETAGS: $(TAGFILES) $(MOREFILES)
	etags $(TAGEXP) $(TAGFILES) $(MOREFILES)
moretags: $(TAGFILES) $(MOREFILES)
	ctags $(TAGEXP) $(TAGFILES) $(MOREFILES)

ALLFILES := $(shell find $(SRCBASE) -iname '*.[ch]')

ALLTAGS: $(ALLFILES)
	etags $(TAGEXP) $(ALLFILES)
alltags: $(ALLFILES)
	ctags $(TAGEXP) $(ALLFILES)

clean:
	rm -rf `find . -name "obj" -or -name "obj-*"`
	rm -rf `find . -name "build.*" -or -name "*.exp" -or -name "*.gen"`
	rm -rf `find . -name "lib" -o -name "target"`

.PHONY: all clean TAGS $(TARGETS)
