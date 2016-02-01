#############################################################################
#
#       Copyright (c) 2003-2013, Broadcom Corporation.
#       All rights reserved.
#       Confidential Property of Broadcom Corporation.
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# [File Description:]
#
# Revision History:
#
# $brcm_Log: $
# 
#############################################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BXDM

# Module Includes
BXDM_INCLUDES += $(MAGNUM)/commonutils/xdm/

# Module Sources
BXDM_SOURCES+= $(MAGNUM)/commonutils/xdm/bxdm_dih.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_v2.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_qm.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_tsm.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_callback.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_output.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_fp.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_vtsm.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_dbg.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_clip.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_frd.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_fic.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_jrc.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_timer.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_avg.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_fix33.c \
               $(MAGNUM)/commonutils/xdm/bxdm_pp_xmo.c

ifeq ($(BXDM_PPJRC_DUMP),y)
BXDM_DEFINES += BXDM_PPJRC_P_DUMP=1
endif
