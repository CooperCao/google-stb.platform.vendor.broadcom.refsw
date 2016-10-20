############################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
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
	##################################################
	##### SAGE PLATFORMS WITHOUT HDCP2.x SUPPORT  ####
	##################################################
	ifeq ($(filter $(BCHP_CHIP), 7435), $(BCHP_CHIP))
		### No DEFINES ####

	##################################################
	##### SAGE PLATFORMS WITH HDCP2.x SUPPORT     ####
	##################################################
	else
		BHDCPLIB_DEFINES += BHDCPLIB_HAS_SAGE
		include $(MAGNUM)/syslib/sagelib/bsagelib_public.inc
		BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/common/bhdcplib_hdcp2x.c
	endif

endif

# HDCP Option
ifneq ($(BHDCPLIB_HDCP_SUPPORT),)
BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/core40nm/bhdcplib_keyloader.c
else
BHDCPLIB_SOURCES += $(MAGNUM)/syslib/hdcplib/src/common/bhdcplib_keyloader_stub.c
endif

ifeq ($(BHDCPLIB_HDR_SUPPORT),y)
BHDCPLIB_DEFINES += BHDCPLIB_HDR_SUPPORT
endif
