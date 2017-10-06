#
# Makefile for rsock Linux user mode libusb applications
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
RM = rm

ARCH = unix
CFLAGS = -g -Wall -Werror -MD -D_REENTRANT -D$(ARCH) -DIPv4 -pedantic
CFLAGS += -DRSOCK_BUS_MTU=500 --std=gnu9x
LDFLAGS =

SRCBASE = ../../../..

VPATH = \
	$(SRCBASE)/toe/rsock \
	$(SRCBASE)/toe/rsock/user \
	$(SRCBASE)/toe/rsock/exe \
	$(SRCBASE)/shared \
	$(SRCBASE)/wl/exe

LIBUSB = libusb/libusb.a

CFLAGS += \
	-I$(SRCBASE)/toe/rsock/user/sim \
	-I$(SRCBASE)/toe/rsock/user \
	-I$(SRCBASE)/toe/rsock/exe \
	-I$(SRCBASE)/toe/include \
	-I$(SRCBASE)/include

all: ttcp ifconfig wl

sinclude *.d

$(LIBUSB): $(wildcard libusb/*.[ch])
	$(MAKE) -C libusb

TTCP_OBJ = ttcp_app.o ttcp.o \
	hostbus_usb.o setup.o rsock.o util.o os.o

ttcp: $(TTCP_OBJ) $(LIBUSB)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o ttcp $(TTCP_OBJ) $(LIBUSB)

IFCONFIG_OBJ = ifconfig_app.o ifconfig.o \
	hostbus_usb.o setup.o rsock.o util.o os.o

ifconfig: $(IFCONFIG_OBJ) $(LIBUSB)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o ifconfig $(IFCONFIG_OBJ) $(LIBUSB)

WL_OBJ = wl_app.o wlu_rsock.o wlu.o wlu_common.o wlu_cmd.o wlu_iov.o bcmwifi_channels.o bcmutils.o \
	hostbus_usb.o setup.o rsock.o util.o os.o

wl: $(WL_OBJ) $(LIBUSB)
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o wl $(WL_OBJ) $(LIBUSB)

clean:
	$(MAKE) -C libusb clean
	$(RM) -f *.o *.d ttcp ifconfig wl core core.* *.core
