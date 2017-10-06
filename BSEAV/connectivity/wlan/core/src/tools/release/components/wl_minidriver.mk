# NOTE: This file MUST list AFTER wl_driver in module_list.mk

override PROD_TAG = $Name: not supported by cvs2svn $

SOURCES := src/wl/config/wl.mk \
	src/wl/sys/wlc_phy_abg.c \
	src/wl/sys/wlc_phy_lpssn.c \
	src/wl/sys/wlc_phy_n.c \
	src/wl/sys/wlc_phy_int.h

include src/tools/release/components/makerules.mk
