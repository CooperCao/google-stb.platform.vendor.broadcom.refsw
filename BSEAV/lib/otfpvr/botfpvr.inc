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
#   OnTheFly PVR library descriptor
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

BOTFPVR_INCLUDES := $(BSEAV)/lib/otfpvr

BOTFPVR_SOURCES := \
    ${BSEAV}/lib/otfpvr/botf.c \
    ${BSEAV}/lib/otfpvr/botf_gop_manager.c \
    ${BSEAV}/lib/otfpvr/botf_gop_player.c \
    ${BSEAV}/lib/otfpvr/botf_itb_feeder.c \
    ${BSEAV}/lib/otfpvr/botf_mem.c \
    ${BSEAV}/lib/otfpvr/botf_scv_parser.c \
    ${BSEAV}/lib/otfpvr/botf_platform.c

BOTFPVR_APP_INCLUDES := $(BSEAV)/lib/otfpvr

BOTFPVR_APP_SOURCES := \
    ${BSEAV}/lib/otfpvr/botf_marker.c 

