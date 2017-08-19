#
# Internal makefile for convenience, not release
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

SRCBASE	= ../..

include $(SRCBASE)/Makerules

ifeq ($(TARGETENV), win32)

TARGETS =  all_wdm

all: $(TARGETS)

$(TARGETS): all_%:
	$(MAKE) -C $* -k -w all

else # !win32

all:

endif # win32

TAGFILES := $(shell find . -maxdepth 1 -iname '*.[ch]')
TAGFILES += $(shell find ../../../components/phy -maxdepth 3 -iname '*.[ch]')
TAGFILES += $(shell for d in ../*/{src,include}; do find $$d -maxdepth 1 -iname '*.[ch]'; done)

TAGS: $(TAGFILES)
	etags -r '/BCM\(INIT\|UNINIT\|ATTACH\|PREATTACH\|NMIATTACH\)\(FN\|DATA\)(\([^(]+\))/\3/' $(TAGFILES)

MORETAGS := $(shell find $(SRCBASE)/shared -maxdepth 1 -iname '*.[ch]')
MORETAGS += $(shell find $(SRCBASE)/include -maxdepth 2 -iname '*.[ch]')
MORETAGS += $(shell find $(SRCBASE)/../components/shared -maxdepth 2 -iname '*.[ch]')

ALLTAGS: $(TAGFILES) $(MORETAGS)
	etags -r '/BCM\(INIT\|UNINIT\|ATTACH\|PREATTACH\|NMIATTACH\)\(FN\|DATA\)(\([^(]+\))/\3/' $(TAGFILES) $(MORETAGS)

clean:
	rm -rf `find . -name "obj" -or -name "obj-*"`
	rm -rf `find . -name "build.*" -or -name "*.exp" -or -name "*.gen"`
	rm -rf `find . -name "lib" -o -name "target"`

.PHONY: all clean TAGS ALLTAGS $(TARGETS)
