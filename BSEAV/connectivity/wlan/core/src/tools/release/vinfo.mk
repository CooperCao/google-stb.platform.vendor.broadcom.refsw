# WLAN build/release makefile fragment for determining version info.
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: $

empty      :=
comma      := $(empty),$(empty)
space      := $(empty) $(empty)

ifdef TAG
  vinfo    := $(subst _,$(space),$(TAG))
else
  vinfo    := $(shell date '+D11 REL %Y %m %d')
endif

maj            := $(word 3,$(vinfo))
min            := $(word 4,$(vinfo))
rcnum          := $(or $(patsubst RC%,%,$(word 5,$(vinfo))),0)
incr           := $(or $(word 6,$(vinfo)),0)
export RELNUM  := $(maj).$(min).$(rcnum).$(incr)
