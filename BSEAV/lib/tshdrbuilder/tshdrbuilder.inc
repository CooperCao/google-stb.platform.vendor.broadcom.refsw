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
#   TS header builder library
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

TSHDRBUILDER_INCLUDES := \
    $(BSEAV)/lib/tshdrbuilder \
    $(BSEAV)/lib/mpeg2_ts_parse

TSHDRBUILDER_SOURCES :=  \
    ${BSEAV}/lib/tshdrbuilder/tshdrbuilder.c \
    ${BSEAV}/lib/mpeg2_ts_parse/ts_psi.c \
    ${BSEAV}/lib/mpeg2_ts_parse/ts_pat.c \
    ${BSEAV}/lib/mpeg2_ts_parse/ts_pmt.c \
    ${BSEAV}/lib/mpeg2_ts_parse/ts_priv.c


