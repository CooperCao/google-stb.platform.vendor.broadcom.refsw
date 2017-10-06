# Makefile for hndrte based 4361a0 ROM Offload image building
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id:$

# chip specification

include $(TOPDIR)/current/4347a0-roml.mk

PCIE_ERR_ATTN_CHECK   := 1
DBG_ERROR             := 1

EXTRA_DFLAGS    += -DVSDBWAR
