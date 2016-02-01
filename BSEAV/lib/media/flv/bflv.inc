############################################################
#     Copyright (c) 2007 Broadcom Corporation
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
#   FLV media library module descriptor
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

BFLV_INCLUDES := $(BSEAV)/lib/media/flv
BFLV_DEFINES := B_HAS_FLV=1

BFLV_FILTER_SOURCES := \
	$(BSEAV)/lib/media/flv/bflv_parser.c 

BFLV_PROBE_SOURCES := \
	$(BSEAV)/lib/media/flv/bflv_probe.c

