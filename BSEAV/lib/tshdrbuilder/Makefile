############################################################
#     (c)2003-2008 Broadcom Corporation
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
#   This file defines the rules to build the tshdrbuilder library
#
# Revision History:
#
# Created: 05/18/2011 by Hongtao Zhu
#
# $brcm_Log: $
# 
############################################################

#
# Conditional options for this makefile
#
# None at this time
#

ifdef COMSPEC
# Any DOS environment
B_LIB_TOP := $(shell cd ../../../nexus/lib && cd)
TSHDRBUILDER_TOP := $(shell cd ../../lib/tshdrbuilder &&  cd )
BSEAV := $(shell cd ../../../BSEAV && cd)
else
BSEAV := $(shell cd ../../../BSEAV; pwd)
B_LIB_TOP := $(shell cd ../../../nexus/lib; pwd)
TSHDRBUILDER_TOP := $(shell cd ../../lib/tshdrbuilder; pwd)
endif
B_MPEG2_TS_PARSE := $(BSEAV)/lib/mpeg2_ts_parse

# Include library definitions
include $(B_LIB_TOP)/build/b_lib_defs.inc

# Include module sources
# Note: You must also include any dependent libraries here also.

#
# Identify this library
#
B_LIBS += TSHDRBUILDER

#
# Define source files in this library
#
include tshdrbuilder.inc

#
# Define public include path(s) for this library
#
TSHDRBUILDER_PUBLIC_INCLUDES += $(TSHDRBUILDER_TOP)/include  \
                              $(B_LIB_TOP)/os/include \
			      $(BSEAV)/lib/mpeg2_ts_parse 
#
# Define private include path(s) for this library
#
TSHDRBUILDER_PRIVATE_INCLUDES += $(TSHDRBUILDER_TOP)/src \
				 $(BSEAV)/lib/mpeg2_ts_parse 
				

#
# Define other libs that this library is dependent upon.  
# This will check for cyclic dependencies and setup include paths
#
# N/A

#
# Define extra C definitions
#
#TSHDRBUILDER_DEFINES += ## N/A for this library

#
# Define possible options - this is only for debug purposes, it does _not_ actually define them.
#
#TSHDRBUILDER_OPTIONS += ## N/A for this library

include $(B_LIB_TOP)/os/b_os_lib.inc

# Instruct build system to build only this library and name it libtshdrbuilder
B_BUILD_LIBS := TSHDRBUILDER
B_LIB_OUTPUT_NAME := tshdrbuilder

# Include library rules
include $(B_LIB_TOP)/build/b_lib_rules.inc

