#
# epi412xx/Makefile
#
# Copyright 1998, Epigram, Inc.
#
# $Id$
#

SRCBASE	= ..

include $(SRCBASE)/Makerules

TARGET_DIRS	= sys

all release clean:
	for f in $(TARGET_DIRS) ; do \
		if [ -d $$f ]; then \
			$(MAKE) -k -w -C $$f $@ ; \
		fi; \
	done

.PHONY: all release clean
