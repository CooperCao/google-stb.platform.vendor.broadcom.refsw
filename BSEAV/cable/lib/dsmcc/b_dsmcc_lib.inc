############################################################
#     (c)2008 Broadcom Corporation
#  
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
#   
#  Except as expressly set forth in the Authorized License,
#   
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#   
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
#  USE OR PERFORMANCE OF THE SOFTWARE.
#  
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
#  ANY LIMITED REMEDY.
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

#
# Conditional options for this makefile
#
# None at this time
#

#
# Identify this library
#
B_LIBS += B_DSMCC_LIB

#
# Define source files in this library
#
B_DSMCC_LIB_SOURCES += \
		$(B_CABLE_TOP)/lib/dsmcc/src/b_dsmcc_lib_priv_dsg.c \
		$(B_CABLE_TOP)/lib/dsmcc/src/b_dsmcc_lib_priv_file.c \
		$(B_CABLE_TOP)/lib/dsmcc/src/b_dsmcc_lib_priv_inband.c \
		$(B_CABLE_TOP)/lib/dsmcc/src/b_dsmcc_lib.c 
#
# Define public include path(s) for this library
#
B_DSMCC_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/os/include $(B_LIB_TOP)/os/include/linuxuser
B_DSMCC_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dsmcc/include

#
# Define private include path(s) for this library
#
B_DSMCC_LIB_PRIVATE_INCLUDES += $(B_CABLE_TOP)/lib/dsmcc/src \
                             $(B_CABLE_TOP)/lib/dsmcc/src/$(SAMPLE)

#
# Define other libs that this library is dependent upon.  
# This will check for cyclic dependencies and setup include paths
#
#B_DSMCC_LIB_DEPENDENCIES := ## N/A for this library

#
# Define extra C definitions
#
#B_DSMCC_LIB_DEFINES += ## N/A for this library

#
# Define possible options - this is only for debug, it does _not_ actually define them.
#
#B_DSMCC_LIB_OPTIONS += ## N/A for this library

