#
# Makefile for usb
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

DIRTARGETS	= dongle hostdrv usbdl

all clean:
	@for f in $(DIRTARGETS) ; do						\
		if [ -d $$f ]; then						\
			$(MAKE) -k -w -C $$f $@;				\
		fi;								\
	done

mrproper: clean
