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
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BDCM

# Module Includes
BDCM_INCLUDES += $(MAGNUM)/portinginterface/dcm
BDCM_INCLUDES += $(MAGNUM)/portinginterface/dcm/rpc/include

BDCM_SOURCES += $(MAGNUM)/portinginterface/dcm/rpc/src/brpc.c
BDCM_SOURCES += $(MAGNUM)/portinginterface/dcm/rpc/src/$(B_REFSW_OS)/brpc_socket.c
BDCM_SOURCES += $(MAGNUM)/portinginterface/dcm/bdcm_device.c
BDCM_SOURCES += $(MAGNUM)/portinginterface/dcm/bdcm_ads.c
BDCM_SOURCES += $(MAGNUM)/portinginterface/dcm/bdcm_aob.c
BDCM_SOURCES += $(MAGNUM)/portinginterface/dcm/bdcm_aus.c
BDCM_SOURCES += $(MAGNUM)/portinginterface/dcm/bdcm_tnr.c
BDCM_SOURCES += $(MAGNUM)/portinginterface/dcm/bdcm_rtnr.c
 
