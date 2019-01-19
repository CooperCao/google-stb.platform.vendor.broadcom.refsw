#
# Linux router wps for AP Makefile
#
# Copyright (C) 2018, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: $
#

include $(TOP)/.config

# linux kernel config
ifneq ($(CONFIG_STBAP),y)
include $(LINUXDIR)/.config
endif

LIB = ../lib/libwps_ap.a

WL_MODE = ap
WPS_CONF_FILE = wpsconfig_router_ap

include ../wps_generic.mk
