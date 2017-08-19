#
# Linux 2.6 Makefile for remote socket client kernel module
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
# $Id$
#
# Note: if the use of $(M) for relative include directory looks funny, it's
# because the sub-make (build/scripts/Makefile.build) includes this Makefile.
#

obj-m	+= rsmod.o

# driver source base
SRCBASE := $(M)/../..
RSOCK := ../../toe/rsock

# basic options
EXTRA_CFLAGS += -I$(M) -I$(SRCBASE)/toe/rsock/exe -I$(SRCBASE)/include
EXTRA_CFLAGS += -Wall -Werror
EXTRA_CFLAGS += -fshort-wchar
EXTRA_CFLAGS += -DBCMDRIVER
#EXTRA_CFLAGS += -DDEBUG

RSOCK_SRC = \
	$(RSOCK)/rsock.c \
	$(RSOCK)/exe/ifconfig.c \
	$(RSOCK)/exe/ttcp.c \
	$(RSOCK)/exe/util.c

SYMLINK_SRC = $(notdir $(RSOCK_SRC))

RSOCK_OBJ = $(SYMLINK_SRC:.c=.o)

rsmod-objs := rstest26.o rstest_cmd.o $(RSOCK_OBJ) os.o

LINUXDIR := /lib/modules/$(shell uname -r)/build

default: $(SYMLINK_SRC)
	$(MAKE) -C $(LINUXDIR) M=$(PWD) V=1 modules

# Symlinks because vpath doesn't work with Linux makefiles

$(SYMLINK_SRC):
	ln -sf $(RSOCK_SRC) .

clean:
	$(MAKE) -C $(LINUXDIR) M=$(PWD) clean
	$(RM) $(SYMLINK_SRC)
