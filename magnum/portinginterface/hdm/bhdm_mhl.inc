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
# Created:
#
# $brcm_Log: $
#
############################################################

ifeq ($(BHDM_MHL_TEST_DEVEL_MODE), y)
BHDM_DEFINES += BHDM_MHL_TEST_DEVEL_MODE
endif

# Can only be enabled in user-mode
ifeq ($(BHDM_MHL_ENABLE_DEBUG), y)
BHDM_DEFINES += BHDM_MHL_ENABLE_DEBUG
BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_debug_priv.c
endif

# Module Includes
BHDM_INCLUDES += \
	$(MAGNUM)/portinginterface/hdm/src/mhl


# Module Sources
BHDM_SOURCES += \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_cbus_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_cfifo_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_host_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_mailbox_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_req_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_msc_resp_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_ddc_req_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_msc_req_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/mhl/bhdm_mhl_hdcp_priv.c
