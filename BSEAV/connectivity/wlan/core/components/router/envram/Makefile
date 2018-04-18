#
# Linux ENVRAM Makefile
#
# Copyright (C) 2017, Broadcom Corporation. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id$
#

include $(TOP)/.config

all: $(if $(CONFIG_ENVRAM_UTILITY),envrams) $(if $(CONFIG_ENVRAM_UTILITY),envram)

envrams: envrams.o
	$(CC) $^ -o $@

envram: envramc.o
	$(CC) $^ -o $@

install: all
	install -D envrams $(INSTALLDIR)/usr/sbin/envrams
	install -D envram $(INSTALLDIR)/usr/sbin/envram
	$(STRIP) $(INSTALLDIR)/usr/sbin/envrams
	$(STRIP) $(INSTALLDIR)/usr/sbin/envram

clean:
	rm -f *.o envram envrams

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#
