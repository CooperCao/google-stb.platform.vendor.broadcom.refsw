##############################################################################
#
#  Copyright (c) 2008 Broadcom Corporation
# 
#  This program is the proprietary software of Broadcom Corporation and/or
#  its licensors, and may only be used, duplicated, modified or distributed
#  pursuant to the terms and conditions of a separate, written license
#  agreement executed between you and Broadcom (an "Authorized License").
#  Except as set forth in an Authorized License, Broadcom grants no license
#  (express or implied), right to use, or waiver of any kind with respect to
#  the Software, and Broadcom expressly reserves all rights in and to the
#  Software and all intellectual property rights therein.  IF YOU HAVE NO
#  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
#  AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
#  SOFTWARE.  
# 
#  Except as expressly set forth in the Authorized License,
# 
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
# 
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
# 
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
##############################################################################
#  $Id$
#
#  Filename:       BfcL2tpV3Lib.mak
#  Author:         Rich Schwartz
#  Creation Date:  September 12, 2008
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the L2TPV3 library.
#
##############
#
# External Variables (input/output):
#
#	BFC_OBJECTS - the list of object files that need to be built for the
#		      project based on the current configuration.
#
#	BFC_COMPILER_DEFINES - the list of definitions that need to be passed
#			       to the compiler.
#
#	BFC_INCLUDE_DIRS - list of directories where header files can be found;
#			   this will be passed to the compiler.
#
#	BFC_SOURCE_DIRS - list of directories where source files can be found;
#			  this is used by the Make utility.
#
#	BFC_LIBRARIES - list of libraries that are to be used when linking the
#			application.
#
# External Constants (input only; do not modify these):
#
#	BFC_TARGET_OS - the OS that the project is being built for (i.e. Win32,
#			VxWorks, Ecos).  This can be used when making decisions
#			about which files/directories/libararies to include.
#
#	BFC_TARGET_PROCESSOR - the processor that the project is being built
#			       for (i.e. Intel, MIPS32, etc).  This can be used
#			       when making decisions about which
#			       files/directories/libararies to include.
#
#	BFC_ROOT_DIR - the "mount point" for the source code, relative to the
#		       output directory for the project.  This is defined in
#		       the top-level makefile for the OS/target.
#
#	BFC_MAKE_DIR - this is essentially $(BFC_ROOT_DIR)/Bfc/make, and is
#		       the directory where the core BFC component makefiles
#		       are located.
#
##############
#
# NOTE:  This file is geared toward the GNU Make utility, and may use any
#        syntax that is valid for recent-vintage GNU Make.
#
##############################################################################


##############################################################################
#
# These are the object modules that go in the L2TPV3 library.  This includes all
# the source modules which Broadcom considers proprietary.
BFC_BFCL2TPV3LIB_OBJECTS = \
 L2tpClientServiceAppIf.o \
 L2tpV3ClientIf.o \
 L2tpV3ClientSnoop.o \
 L2tpV3ClientThread.o \
 L2tpEventCodes.o  \
 L2tpAvpType.o \
 L2tpConnection.o \
 L2tpControlMessage.o \
 L2tpProtocolMessage.o \
 L2tpFramingMessage.o \
 L2tpBearerMessage.o \
 L2tpFirmwareMessage.o \
 L2tpHostNameMessage.o \
 L2tpSerialNumberMessage.o \
 L2tpVendorNameMessage.o \
 L2tpAssignTunnelMessage.o \
 L2tpZeroBitMessage.o \
 L2tpAssignSessionMessage.o \
 L2tpConnectSpeedMessage.o \
 L2tpFramingTypeMessage.o \
 L2tpReceiveWindowMessage.o \
 L2tpV3Header.o \
 L2tpV3DraftMessage.o \
 L2tpV3AssignedConnectionId.o \
 L2tpV3Session.o \
 L2tpV3MessageDigest.o \
 L2tpV3AuthNonce.o \
 L2tpDataSeq.o\
 L2tpV3Mtu.o  \
 L2tpV3LocalSession.o \
 L2tpV3RemoteSession.o \
 L2tpV3PseudowireType.o \
 L2tpV3TieBreakerMessage.o \
 L2tpV3CircuitStatus.o \
 L2tpV3RemoteEndId.o \
 L2tpV3ResultCode.o \
 L2tpV3PseudowireCapList.o \
 L2tpV3RemoteAccessCommandTable.o \
 L2tpV3AssignedCookie.o

ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1) 

        BFC_BFCL2TPV3LIB_OBJECTS += L2tpCommandTable.o
endif 


l2tpv3lib : ${BFC_BFCL2TPV3LIB_OBJECTS}
	@rm -f $@
	ar r $@ $(BFC_BFCL2TPV3LIB_OBJECTS)
	cp l2tpv3lib ../../../$(BFC_L2TPV3_LIB)

l2tpv3src : l2tpv3lib


##############################################################################
#
# These are all the directories which hold header files for the L2tpV3 library.

BFC_L2TPV3_INCLUDE_DIRS = \
   ../rbb_cm_lib/L2TPv3/Client \
   ../rbb_cm_lib/L2TPv3/Common \
   ../rbb_cm_lib/L2TPv3/Server

##############################################################################
#
# These are all the directories which hold source files for the L2TPV3 library.

BFC_L2TPV3_SOURCE_DIRS = \
    ../rbb_cm_lib/L2TPv3/Client \
    ../rbb_cm_lib/L2TPv3/Common \
   ../rbb_cm_lib/L2TPv3/Server

BFC_L2TPV3_DEFINES = BCM_L2TPV3_SUPPORT=1 BCM_L2TP_SUPPORT=1


##############################################################################
#
# Add the local definitions to the global variables.

BFC_INCLUDE_DIRS += $(BFC_L2TPV3_INCLUDE_DIRS) $(BFC_RESIDENTIAL_GATEWAY_DIR)/Ppp

BFC_SOURCE_DIRS += $(BFC_L2TPV3_SOURCE_DIRS) 

BFC_COMPILER_DEFINES += $(BFC_L2TPV3_DEFINES)


