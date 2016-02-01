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
#   AVI media library module descriptor
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

BAVI_INCLUDES := $(BSEAV)/lib/avi
BAVI_DEFINES := B_HAS_AVI=1

BAVI_SOURCES := \
	$(BSEAV)/lib/avi/bavi_util.c  \
	$(BSEAV)/lib/avi/bavi_parser.c 

BAVI_FILTER_SOURCES := \
	$(BSEAV)/lib/avi/bavi_stream.c 

BAVI_PLAYER_SOURCES := \
	$(BSEAV)/lib/avi/bavi_player.c \
	$(BSEAV)/lib/avi/bavi_player_custom.c

BAVI_PROBE_SOURCES := \
	$(BSEAV)/lib/avi/bavi_probe.c

