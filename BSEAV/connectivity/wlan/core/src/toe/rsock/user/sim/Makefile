#
# Makefile for rsock Linux user mode simulation
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

CC = gcc

ARCH=unix
CFLAGS=-g -Wall -Werror -MD -D_REENTRANT -D$(ARCH) -DIPv4 -pedantic
LDFLAGS=

SRCBASE = ../../../..

VPATH = \
	$(SRCBASE)/toe/rsock \
	$(SRCBASE)/toe/rsock/user \
	$(SRCBASE)/toe/rsock/exe

CFLAGS += \
	-I$(SRCBASE)/toe/rsock/user/sim \
	-I$(SRCBASE)/toe/rsock/user \
	-I$(SRCBASE)/toe/rsock/exe \
	-I$(SRCBASE)/toe/include

all: ifconfig ttcp mttcp

sinclude *.d

# ifconfig

IFCONFIG_OBJ = ifconfig_app.o ifconfig.o hostbus_sim.o rsock.o setup.o util.o os.o

ifconfig: $(IFCONFIG_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o ifconfig $(IFCONFIG_OBJ)

# ttcp

TTCP_OBJ = ttcp_app.o ttcp.o hostbus_sim.o setup.o rsock.o util.o os.o

ttcp: $(TTCP_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o ttcp $(TTCP_OBJ)

# mttcp

MTTCP_OBJ = mttcp_app.o ttcp.o hostbus_sim.o setup.o rsock.o util.o os.o

mttcp: $(MTTCP_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o mttcp $(MTTCP_OBJ)

clean:
	rm -f *.o *.d ifconfig ttcp mttcp core core.* *.core
