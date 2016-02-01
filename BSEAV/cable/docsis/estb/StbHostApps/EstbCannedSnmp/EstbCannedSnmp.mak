##############################################################################
#
# Copyright (c) 2009 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
# AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
# SOFTWARE.  
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
##############################################################################

############################################################################
##                 FLAGS
############################################################################
##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.
  
STB_HOST_APP_CANNED_SNMP_DEFINES += BCM_CANNED_SNMP_SUPPORT=1
	
############################################################################
##                 V2 COMPONENT INCLUDES
############################################################################


############################################################################
##                 PATHS
############################################################################

STB_HOST_APP_CANNED_SNMP  = StbHostApps/EstbCannedSnmp

############################################################################
##                 OBJECT FILES
############################################################################

STB_HOST_APP_CANNED_SNMP_OBJECTS +=	EstbCannedSnmp.o

ifeq ($(BCM_CONSOLE_SUPPORT),1)
	STB_HOST_APP_CANNED_SNMP_OBJECTS +=	EstbCannedSnmpCommandTable.o
endif

############################################################################
##                 EXTRA_INCLUDE - include path
############################################################################

STB_HOST_APP_CANNED_SNMP_INCLUDE_DIRS += $(STB_HOST_APP_CANNED_SNMP)

############################################################################
##                 VPATH - source path
############################################################################

STB_HOST_APP_CANNED_SNMP_SOURCE_DIRS += $(STB_HOST_APP_CANNED_SNMP)

############################################################################
##                 LIB_EXTRA - libraries
############################################################################

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add ESTB CANNED SNMP components, if included by BfcSystemConfig.mak.
ifeq ($(BCM_CANNED_SNMP_SUPPORT), 1)

	BFC_OBJECTS += $(STB_HOST_APP_CANNED_SNMP_OBJECTS) 

	BFC_COMPILER_DEFINES += $(STB_HOST_APP_CANNED_SNMP_DEFINES)

	BFC_INCLUDE_DIRS += $(STB_HOST_APP_CANNED_SNMP_INCLUDE_DIRS) 

	BFC_SOURCE_DIRS += $(STB_HOST_APP_CANNED_SNMP_SOURCE_DIRS) 

	BFC_LIBRARIES += 

	BFC_LIBRARIES_ABS_PATH += 
endif


