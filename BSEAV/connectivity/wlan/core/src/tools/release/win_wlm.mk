#
# WLM (Wireless LAN Manufacturing) test library makefile for Windows
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

MODULE_FILES := \
	src/tools/wlm/wlmSampleTests.vcproj \
	src/tools/wlm/wlmSampleTests.sln \
	src/tools/wlm/wlmSampleTests.c \
	src/wl/exe/windows/win7/obj/wlm/free/wlm.lib \
	src/wl/exe/windows/win7/obj/wlm/free/wlm.dll \
	src/wl/exe/wlm.h

package:
	tar czvf wlm.tar.gz --exclude=*/.svn $(MODULE_FILES)
