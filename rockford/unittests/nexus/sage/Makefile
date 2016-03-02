############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# Created: 08/28/2007 by Jon Garrett
#
# $brcm_Log: $
#
############################################################

ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
NEXUS_TOP ?= $(shell cd ../../../../nexus; pwd)

include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)

ifeq ($(B_REFSW_ANDROID),y)
        include $(NEXUS_TOP)/build/android.inc
	LDFLAGS := $(ANDROID_LDFLAGS) $(NXCLIENT_LDFLAGS)
else
        LDFLAGS += $(NXCLIENT_LDFLAGS)
endif

APPS = \
      heap_check \
      heap_create \
	crrurr_glrglr_decode \
	cdb_maxres_test

include ${NEXUS_TOP}/../rockford/unittests/nexus/common/Makefile.inc
