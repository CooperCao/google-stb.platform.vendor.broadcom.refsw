##############################################################################
#
#  Copyright (c) 2004-2009 Broadcom Corporation
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
#  Filename:       BfcEmta.mak
#  Author:         Sean Dunlap
#  Creation Date:  July 20, 2004
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the MTA subsystem, which
#	are common to all MTA platforms.
#
##############
#
# External Variables (input/output):
#
#	BFC_OBJECTS - the list of object files that need to be built for the
#		      project based on the current configuration.
#
#	BFC_COMPILER_DEFINES - the list of definitions that need to be passed
#			       to the compiler.  This may be deprecated in the
#			       future, in favor of just using BfcSystemConfig.h.
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

# We only include IPSEC support if its for ECOS and Not a SIP build
ifeq ($(BFC_TARGET_OS),Ecos)
    ifeq ($(BFC_INCLUDE_SIP_SUPPORT),0)
        BFC_INCLUDE_IPSEC_SUPPORT = 1
    endif
endif

# UNFINISHED - Currently, we can't have VPN and IPSEC in same build.
#              so disable IPSEC if this is the case
#              VPN has not been implemented as conditional in SNMP code so it is
#              not possible to remove it now.

ifeq ($(BFC_INCLUDE_IPSEC_SUPPORT),1)
    BFC_EMTA_DEFINES = IPSEC_SUPPORT=1
endif


##############################################################################
#
# Paths - definitions of relative paths for this component.  These are used
#         below, and should be defined once, rather than using the actual
#         value in multiple places.
#
#         Note that the path needs to be specified relative to the "mount point"
#         of the parent project, not relative to the output directory.  The
#         make processing will specify the rest of the path for you.

BFC_MTA_DIR = Mta

BFC_EDOCSIS_DIR = $(BFC_DOCSIS_DIR)/eDOCSIS

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.

BFC_EMTA_OBJECTS += EmtaCmDocsisStatusClientACT.o \
                    EmtaCmDownstreamScanClientACT.o \
                    EmtaCmDhcpCompletionHandlerACT.o \
                    EmtaDsxCompletionHandlerACT.o \
                    EmtaServiceFlowEventACT.o \
                    IpDatagramFragmenter.o \
                    IpDatagramFragmentReassembler.o  \
                    MtaApplication.o \
                    AnnouncementDownloadThread.o \
                    LanBroadcastRateLimiterSnoop.o \
                    EmtaRemoteAccessCommandTable.o \
                    TelRootCertDownloadThread.o \
                    mspidriver.o

ifeq ($(BFC_EROUTER_SUPPORT),1)
    BFC_EMTA_OBJECTS += EmtaeRouterCmDocsisSystem.o
else
endif 

ifeq ($(BFC_CABLEHOME_SUPPORT),1)
    BFC_EMTA_OBJECTS += EmtaEpsCmDocsisSystem.o
else
    # EPS plus EMTA (triple-play)
    BFC_EMTA_OBJECTS += EmtaCmDocsisSystem.o
endif

ifeq ($(BFC_INCLUDE_SIP_SUPPORT),1)
    BFC_EMTA_OBJECTS += EmtaSipNonVolSettings.o \
                        EmtaSipNonVolSettingsCommandTable.o \
                        EmtaSipRemoteAccessCommandTable.o \
                        MtaSipNonVolAPI.o
endif


ifeq ($(BFC_INCLUDE_EDVA_SUPPORT),1)
    BFC_EMTA_OBJECTS += EdvaNonVolSettings.o 
    BFC_EMTA_OBJECTS += EdvaNonVolSettingsCommandTable.o
endif



BFC_EMTA_SNMP_OBJECTS = EmtaSnmpAgentCmDocsisStatusACT.o \
                        esafeMib.o \
                        esafeBridge.o

BFC_EMTA_FACTORY_OBJECTS =

BFC_EMTA_BRCM_RUNTIME_SNMP_OBJECTS = brcmEmtaMgmtMib.o brcmEmtaMgmtBridge.o \
                                     brcmCmEmtaMgmtMib.o brcmCmEmtaMgmtBridge.o

ifeq ($(BFC_INCLUDE_IPSEC_SUPPORT), 1)
    BFC_EMTA_OBJECTS += \
            WindRiver2XpressVpnAPI.o \
            IpsecNonVolSettings.o \
            IpsecNonVolSettingsCommandTable.o \
            EmtaIpsecSnoop.o \
            IpsecTimerThread.o
else
     BFC_EMTA_OBJECTS += IpReassemblerFragmenterSnoop.o
endif

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.
ifeq ($(BRCM_HTTP_SSL_SUPPORT), 1)

BFC_EMTA_DEFINES += BRCM_HTTP_SSL_SUPPORT=1

	ifeq ($(BRCM_HTTP_SSL_STRONG_CIPHER), 1)
		BFC_EMTA_DEFINES += BRCM_HTTP_SSL_STRONG_CIPHER=1
	else
		BFC_EMTA_DEFINES += BRCM_HTTP_SSL_STRONG_CIPHER=0
	endif

endif

BFC_EMTA_DEFINES += BFC_INCLUDE_SIP_SUPPORT=$(BFC_INCLUDE_SIP_SUPPORT)
BFC_EMTA_DEFINES += BFC_INCLUDE_SIPDQOS_SUPPORT=$(BFC_INCLUDE_SIPDQOS_SUPPORT)
BFC_EMTA_DEFINES += BFC_INCLUDE_EMTA_SIGTLS_SUPPORT=$(BFC_INCLUDE_EMTA_SIGTLS_SUPPORT)
BFC_EMTA_DEFINES += BFC_INCLUDE_MTA_CABLEMODEM_SUPPORT=$(BFC_INCLUDE_MTA_CABLEMODEM_SUPPORT)
BFC_EMTA_DEFINES += BFC_INCLUDE_EMTA_SIPDBG_SUPPORT=$(BFC_INCLUDE_EMTA_SIPDBG_SUPPORT)
BFC_EMTA_DEFINES += BFC_INCLUDE_IPV6_SUPPORT=$(BFC_INCLUDE_IPV6_SUPPORT)
BFC_EMTA_DEFINES += BFC_INCLUDE_EDVA_SUPPORT=$(BFC_INCLUDE_EDVA_SUPPORT)

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_EMTA_INCLUDE_DIRS += CmDocsisSystem \
                         $(BFC_MTA_DIR)/XpressVPN \
                         $(BFC_EDOCSIS_DIR)  \
                         Mta/MtaApi

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_EMTA_SOURCE_DIRS += CmDocsisSystem $(BFC_EDOCSIS_DIR) Mta/MtaApi Mta/Firewall

ifeq ($(BFC_INCLUDE_IPSEC_SUPPORT), 1)
    BFC_EMTA_SOURCE_DIRS += $(BFC_MTA_DIR)/XpressVPN
endif

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

# if a local enviroinment variable defines the location of the CxC libraries,
# use them
ifdef HH_LIB_PATH
    BFC_MTA_DEFINES += MTA_LIBRARIES_EXIST=1
    EMTA_LIBRARIES =                           \
          $(HH_LIB_PATH)/mta.a                \
          $(HH_LIB_PATH)/common.a             \
          $(HH_LIB_PATH)/mta/krb5client.a     \
          $(HH_LIB_PATH)/libhausware.a

    # Add in TLS library if building with TLS support
    ifeq ($(BFC_INCLUDE_EMTA_SIGTLS_SUPPORT),1)
        EMTA_LIBRARIES += $(OPENSSL_ROOT)/libOpenSSLtlsecos.a
    endif

    # Add in SIP libraries if building with SIP protocol
    #
    # Creation of the SIP library name depends on the build option, order
    # is important.
    #
    ifeq ($(BFC_INCLUDE_SIP_SUPPORT),1)
       ifeq ($(BFC_INCLUDE_EDVA_SUPPORT),1)
          HH_LIB_SIP := $(HH_LIB_COMMON_PATH)/sipUa
          ifeq ($(BFC_INCLUDE_EMTA_SIPIPv6_SUPPORT),1)
             HH_LIB_SIP := $(HH_LIB_SIP)/v6
          else
             HH_LIB_SIP := $(HH_LIB_SIP)/v4
          endif
          ifeq ($(BFC_INCLUDE_EMTA_SIGTLS_SUPPORT),1)
             HH_LIB_SIP := $(HH_LIB_SIP)/tls
          else
             HH_LIB_SIP := $(HH_LIB_SIP)/notls
          endif
          ifeq ($(BFC_INCLUDE_EMTA_SIPDBG_SUPPORT),1)
             HH_LIB_SIP := $(HH_LIB_SIP)/dbg
          else
             HH_LIB_SIP := $(HH_LIB_SIP)/nodbg
          endif
          HH_LIB_SIP := $(HH_LIB_SIP)/libM5TSipUa.a
          HH_LIB_EXPAT := $(HH_LIB_COMMON_PATH)/sipUa/libExpat1958.a
          EMTA_LIBRARIES += $(HH_LIB_SIP)
          EMTA_LIBRARIES += $(HH_LIB_EXPAT)
       else
          ifeq ($(BFC_INCLUDE_EMTA_SIGTLS_SUPPORT),1)
             ifeq ($(BFC_INCLUDE_EMTA_SIPDBG_SUPPORT),1)
                EMTA_LIBRARIES += $(HH_LIB_COMMON_PATH)/libsipstk_tls_dbg.a
             else
                EMTA_LIBRARIES += $(HH_LIB_COMMON_PATH)/libsipstk_tls.a
             endif
          else
             ifeq ($(BFC_INCLUDE_EMTA_SIPDBG_SUPPORT),1)
                EMTA_LIBRARIES += $(HH_LIB_COMMON_PATH)/libsipstk_dbg.a
             else
                 EMTA_LIBRARIES += $(HH_LIB_COMMON_PATH)/libsipstk.a
             endif
          endif
       endif
    endif
endif

ifeq ($(BFC_INCLUDE_IPSEC_SUPPORT), 1)
    ifeq ($(BFC_INCLUDE_OPENSSL098), 1)
        BFC_EMTA_LIBRARIES += Bfc/LibSupport/Vpn/libXpressIpsecEcos_098.a
    else
        BFC_EMTA_LIBRARIES += Bfc/LibSupport/Vpn/libXpressIpsecEcos.a
    endif
endif

# TRW - need additional library for HTTPS support
ifeq ($(BRCM_HTTP_SSL_SUPPORT), 1)
       BFC_EMTA_LIBRARIES += Bfc/LibSupport/OpenSSL098/libOpenSSLtlsecos.a
endif

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

ifeq ($(BFC_INCLUDE_EMTA_SUPPORT), 1)

	BFC_OBJECTS += $(BFC_EMTA_OBJECTS) $(BFC_EMTA_SNMP_OBJECTS)

        # Add factory objects if we have factory support
        ifeq ($(BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT), 1)

            BFC_OBJECTS += $(BFC_EMTA_FACTORY_OBJECTS)

        endif

        # Add runtime objects if we have BRCM runtime MIB support
        ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)

            BFC_OBJECTS += $(BFC_EMTA_BRCM_RUNTIME_SNMP_OBJECTS)

        endif

	BFC_COMPILER_DEFINES += $(BFC_EMTA_DEFINES)

	BFC_INCLUDE_DIRS += $(BFC_EMTA_INCLUDE_DIRS)

	BFC_SOURCE_DIRS += $(BFC_EMTA_SOURCE_DIRS)

	BFC_LIBRARIES += $(BFC_EMTA_LIBRARIES)

	BFC_LIBRARIES_ABS_PATH += $(EMTA_LIBRARIES)
endif


