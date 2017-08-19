#
# Makefile for rserv Linux user mode simulation
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

CCDEP = gcc
CC = gcc

ARCH = unix
CFLAGS = -g -Wall -Werror -D_REENTRANT -D$(ARCH) -DIPv4 -DLWIP_DEBUG -pedantic
LDFLAGS =
ARFLAGS = rs

SRCBASE = ../../..
LWIPDIR = $(SRCBASE)/toe/lwip/src

CFLAGS += \
	-I$(SRCBASE)/include \
	-I$(SRCBASE)/toe/include \
	-I$(SRCBASE)/toe/rserv \
	-I$(SRCBASE)/toe/rserv/sim \
	-I$(LWIPDIR)/include -I$(LWIPDIR)/include/ipv4 -I$(LWIPDIR)

# COREFILES, CORE4FILES: The minimum set of files needed for lwIP.
COREFILES = \
	$(LWIPDIR)/core/mem.c $(LWIPDIR)/core/memp.c $(LWIPDIR)/core/netif.c \
	$(LWIPDIR)/core/pbuf.c $(LWIPDIR)/core/raw.c $(LWIPDIR)/core/stats.c \
	$(LWIPDIR)/core/sys.c $(LWIPDIR)/core/tcp.c $(LWIPDIR)/core/tcp_in.c \
	$(LWIPDIR)/core/tcp_out.c $(LWIPDIR)/core/udp.c $(LWIPDIR)/core/dhcp.c

CORE4FILES = $(wildcard $(LWIPDIR)/core/ipv4/*.c) $(LWIPDIR)/core/inet.c

# NETIFFILES: Files implementing various generic network interface functions.
NETIFFILES = $(LWIPDIR)/netif/loopif.c $(LWIPDIR)/netif/etharp.c

# ARCHFILES: Architecture specific files.
ARCHFILES = sys_arch.c venetif.c venetc.c

# LWIPFILES: All the above.
LWIPFILES=$(COREFILES) $(CORE4FILES) $(NETIFFILES) $(ARCHFILES)
LWIPFILESW=$(wildcard $(LWIPFILES))
LWIPOBJS=$(notdir $(LWIPFILESW:.c=.o))

LWIPLIB=liblwip4.a

%.o:
	$(CC) $(CFLAGS) -c $(<:.o=.c)

.PHONY: all
all: dongle venets venetc_test

depend dep: .depend

sinclude .depend

$(LWIPLIB): $(LWIPOBJS)
	$(AR) $(ARFLAGS) $(LWIPLIB) $?

# Simulated dongle running rserv

vpath %.c $(SRCBASE)/toe/rserv

DONGLE_OBJ = dongle.o devbus.o rserv.o os.o

dongle: .depend $(DONGLE_OBJ) $(LWIPLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o dongle $(DONGLE_OBJ) $(LWIPLIB)

# Virtual Ethernet Switch

VENETS_OBJ = venets.o

venets: .depend $(VENETS_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o venets $(VENETS_OBJ)

# Virtual Ethernet Client Test

VENETC_TEST_OBJ = venetc_test.o venetc.o

venetc_test: .depend $(VENETC_TEST_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o venetc_test $(VENETC_TEST_OBJ)

.depend: devbus.c dongle.c \
	rserv.c os.c venets.c venetc.c venetif.c venetc_test.c $(LWIPFILES)
	$(CCDEP) $(CFLAGS) -MM $^ > .depend || (rm -f .depend; exit 1)

clean:
	rm -f *.o $(LWIPLIB) \
		dongle venets venetc_test \
		core core.* *.core
