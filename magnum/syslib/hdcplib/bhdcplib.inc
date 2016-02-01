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
# BHDCPLIB_HDCP_SUPPORT - If defined, HDCP support will be enabled.
#

# Define this module
MAGNUM_MODULES += BHDCPLIB

# Module Includes
BHDCPLIB_INCLUDES +=	$(MAGNUM)/syslib/hdcplib/include \
			$(MAGNUM)/syslib/hdcplib/src/common \
			$(MAGNUM)/syslib/hdcplib/src/common/priv

# Module Sources
BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/common/bhdcplib.c
BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/common/bhdcplib_hdcpkeys.c


########## HDCP 2.2 ####################
ifeq ($(SAGE_SUPPORT),y)
	##########################################################
	##### 65nm and 40nm PLATFORMS WITHOUT HDCP2.x SUPPORT ####
	##########################################################
	ifeq ($(filter $(BCHP_CHIP), 7325 7335 7400 7405 7125 7340 7342 7408 7420 7468 7550 7422 7425 7429 7435 7231 7358 7344 7346 7360 7362 73625 7552 7640 7584 75845 7563 75635 7543 7228 74295 73465), $(BCHP_CHIP))
		### No DEFINES ####

	#####################################################
	##### old 28nm PLATFORMS WITHOUT HDCP2.x SUPPORT ####
	#####################################################
	else ifeq ($(findstring $(BCHP_CHIP), 7445), $(BCHP_CHIP))
		ifeq ($(findstring $(BCHP_VER), c0 C0 ), $(BCHP_VER))
			### No DEFINES ####
		else
			BHDCPLIB_DEFINES += BHDCPLIB_HAS_SAGE
			include $(MAGNUM)/syslib/sagelib/bsagelib_public.inc
			BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/common/bhdcplib_hdcp2x.c
		endif


	#####################################################
	##### old 28nm PLATFORMS WITHOUT HDCP2.x SUPPORT ####
	#####################################################
	else ifeq ($(findstring $(BCHP_CHIP), 7366 7145 7439), $(BCHP_CHIP))
		ifeq ($(findstring $(BCHP_VER), a0 A0 ), $(BCHP_VER))
			### No DEFINES ####
		else
			BHDCPLIB_DEFINES += BHDCPLIB_HAS_SAGE
			include $(MAGNUM)/syslib/sagelib/bsagelib_public.inc
			BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/common/bhdcplib_hdcp2x.c
		endif


	##################################################
	##### new 28nm PLATFORMS WITH HDCP2.x SUPPORT ####
	##################################################
	else
		BHDCPLIB_DEFINES += BHDCPLIB_HAS_SAGE
		include $(MAGNUM)/syslib/sagelib/bsagelib_public.inc
		BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/common/bhdcplib_hdcp2x.c
	endif

endif

# HDCP Option
ifneq ($(BHDCPLIB_HDCP_SUPPORT),)

	############## 65nm platforms #################
	ifeq ($(filter $(BCHP_CHIP), 7125 7208 7325 7335 7336 7340 7342 7400 7405 7408 7420 7468 7550 7572 7580), $(BCHP_CHIP))

		######## non-BSP platforms ###########
		ifeq ($(filter $(BCHP_CHIP), 7550 7572 7580 7408), $(BCHP_CHIP))
		BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/core65nm/nonbsp/bhdcplib_keyloader.c
		endif

		######## non-ASKM platforms ###########
		ifeq ($(filter $(BCHP_CHIP), 7325 7335 7336 7400 7405), $(BCHP_CHIP))
		BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/core65nm/nonaskm/bhdcplib_keyloader.c
		endif

		######## ASKM platforms ###########
		ifeq ($(filter $(BCHP_CHIP), 7125 7208 7340 7342 7420 7468 ), $(BCHP_CHIP))
		BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/core65nm/askm/bhdcplib_keyloader.c
		endif

	else

	############# 40nm platforms ################
	BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/core40nm/bhdcplib_keyloader.c
	endif
else
BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/common/bhdcplib_keyloader_stub.c
endif

ifeq ($(BHDCPLIB_HDR_SUPPORT),y)
BHDCPLIB_DEFINES += BHDCPLIB_HDR_SUPPORT
endif
