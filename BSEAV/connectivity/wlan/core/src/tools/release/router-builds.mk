#
# Global BCM947xx Linux Router build/release Makefile
#
# Copyright 2008, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id$
#

BUILD_LIST= \
	linux-external-router \
	linux-external-router-partial-src \
	linux-external-router-partial-src-with-ses \
	linux-external-router-full-src \
	linux-external-router-combo \
	linux-external-router-combo-full-src \
	linux-external-router-sdio-std \
	linux-external-usbap \
	linux-external-usbap-full-src \
	linux-external-vista-router-full-src


all:
	cvs co src/hndcvs
	cp src/hndcvs/build-linux-router.sh .
	chmod a+x build-linux-router.sh
	./build-linux-router.sh -p -b "$(BUILD_LIST)" -r $(TAG)

.phony all
