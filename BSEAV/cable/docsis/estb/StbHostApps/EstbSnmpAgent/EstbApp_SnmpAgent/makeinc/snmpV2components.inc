
# ****************************************************************************
#
#  Copyright (c) 2007-2010 Broadcom Corporation
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
# ****************************************************************************
#     Description:
#
# ****************************************************************************
#     Revision History:
#
# ****************************************************************************
############################################################################
##                 FLAGS
############################################################################

ADDED_SNMP_FLAGS += -DTARGETOS_Linux=1

ifeq ($(SNMP_SUPPORT), 2)
  ADDED_SNMP_FLAGS += -DSNMPv2=1 -DSNMP_SUPPORT=2
endif

ifeq ($(SNMP_SUPPORT), 3)
  ADDED_SNMP_FLAGS += -DSNMPv3=1 -DSNMP_SUPPORT=3
endif

ifeq ($(SNMP_SUPPORT), 0)
  ADDED_SNMP_FLAGS += -DSNMP_SUPPORT=0
endif

ifeq ($(BCM_RUNTIMEMIB_SUPPORT), 1)
  ADDED_SNMP_FLAGS += -DBCM_RUNTIMEMIB_SUPPORT=1
endif

ifeq ($(SNMP_PROXY_SUPPORT), 1)
  ADDED_SNMP_FLAGS += -DSNMP_PROXY_SUPPORT=1
endif

ifeq ($(LOCAL_SNMP_AGENT_SUPPORT), 1)
  ADDED_SNMP_FLAGS += -DLOCAL_SNMP_AGENT_SUPPORT=1
endif

ifeq ($(MOCA_MIB_SUPPORT), 1)
  ADDED_SNMP_FLAGS += -DMOCA_MIB_SUPPORT=1
endif

############################################################################
##                 PATHS
############################################################################

BASE_DIR                =  ../../../../..
ESTB_SNMP_DIR           =  ../../../EstbSnmp
BASE_SNMP_DIR           =  $(BASE_DIR)/Bfc/IpHelpers/Snmp
DOCSIS_DIR	        =  $(BASE_DIR)/Docsis
DOCSIS_OSS_DIR		=  $(DOCSIS_DIR)/Oss
STB_SNMP_DIR            =  $(BASE_DIR)/Stb/Snmp					
TARGETOS_V2_SNMP_DIR    =  $(ESTB_SNMP_DIR)/NetSnmp

ifeq ($(origin OPENSSL_DIR), undefined)
  OPENSSL_DIR        = ../../../LibSupport/OpenSSL
endif

ifeq ($(GCC_VERSION),4)
  OPENSSL_DIR     = ../../../LibSupport/OpenSSL
endif

ifeq ($(SNMP_PROXY_SUPPORT), 1)
  SNMP_PROXY_DIR      = ../../../EstbSnmp/Proxy
endif

NETSNMP_LIB_DIR = ../../../LibSupport/NetSnmp
ifeq ($(GCC_VERSION),4)
  NETSNMP_LIB_DIR = ../../../LibSupport/NetSnmp
endif
NETSNMP_LIB_INCLUDE_DIR = $(NETSNMP_LIB_DIR)/include

NETSNMP_OS = linux

NETSNMP_LIBS = -Wl,--start-group $(NETSNMP_LIB_DIR)/libnetsnmp_$(NETSNMP_OS).a \
                        $(NETSNMP_LIB_DIR)/libnetsnmphelpers_$(NETSNMP_OS).a \
                        $(NETSNMP_LIB_DIR)/libnetsnmpmibs_$(NETSNMP_OS).a \
                        $(NETSNMP_LIB_DIR)/libnetsnmpagent_$(NETSNMP_OS).a -Wl,--end-group

MOCA_LIB_DIR = ../../../LibSupport/MoCA

MOCA_LIB_INCLUDE_DIR = $(MOCA_LIB_DIR)/include

## MOCA_LIB = -Wl,--start-group $(MOCA_LIB_DIR)/libmoca.a -Wl,--end-group
MOCA_LIB = $(MOCA_LIB_DIR)/libmoca.a

include $(ESTB_SNMP_DIR)/EstbSnmpAgent.mk

############################################################################
##                 OBJECT FILES
############################################################################

############################################################################
##                 SNMP_OBJS
############################################################################


ifeq ($(SNMP_SUPPORT), 0)
  #----------------------------------------------------------------------
  # If SNMP_SUPPORT is set to 0, just build the stub objects
  #----------------------------------------------------------------------

  # From Snmp/DocsisSnmp.mk
  SNMP_OBJS         = $(DOCSIS_STUB_SNMP_OBJS)
  CONSOLE_SNMP_OBJS = $(DOCSIS_STUB_SNMP_CONSOLE_OBJS)
else
  #----------------------------------------------------------------------
  # Otherwise, build components as indicated by various flags
  #----------------------------------------------------------------------


#-------------------------------------------------------------------------------
# Core SNMP objects for any kind of build
#-------------------------------------------------------------------------------
  SNMP_OBJS          = $(CORE_SNMP_OBJS)
  CONSOLE_SNMP_OBJS += $(CORE_SNMP_CONSOLE_OBJS)

  ifeq "$(TARGETOS)" "Linux"
        SNMP_OBJS += $(STBSNMP_OBJS)
        CONSOLE_SNMP_OBJS += $(STBSNMP_CONSOLE_OBJS)
        SNMP_OBJS += $(STBSNMP_NETSNMP_OBJS)
  else
        ifeq ($(SNMP_STUBS),1)
                # To allow building the app with V2-SNMP support, but no
                # actual agent.
                SNMP_OBJS += $(STUB_SNMP_OBJS)
                CONSOLE_SNMP_OBJS += $(STUB_SNMP_CONSOLE_OBJS)
        endif
  endif

  ifeq ($(BCM_RUNTIMEMIB_SUPPORT), 1)
        SNMP_OBJS += $(BRCM_SNMP_OBJS)
  endif

  ifeq ($(SNMP_SUPPORT),3)
        # SNMPv3 objects from Snmp/DocsisSnmp.mk
        SNMP_OBJS += $(V3_SNMP_OBJS)
        SNMP_OBJS += $(STBSNMP_SNMPV3_OBJS)
        SNMP_OBJS += $(STBSNMP_NETSNMP_V3_OBJS)
        CONSOLE_SNMP_OBJS += $(V3_SNMP_CONSOLE_OBJS)
  endif

endif

#-------------------------------------------------------------------------------
# Local SNMP Agent objects
#-------------------------------------------------------------------------------
ifeq ($(LOCAL_SNMP_AGENT_SUPPORT),1)
  SNMP_OBJS += $(LOCAL_SNMP_AGENT_OBJS)
endif

#-------------------------------------------------------------------------------
# SNMP Proxy objects
#-------------------------------------------------------------------------------
ifeq ($(SNMP_PROXY_SUPPORT),1)
  SNMP_OBJS += $(SNMP_PROXY_OBJS)
endif

#-------------------------------------------------------------------------------
# MOCA MIB objects
#-------------------------------------------------------------------------------
ifeq ($(MOCA_MIB_SUPPORT),1)
  SNMP_OBJS += $(MOCA_MIB_OBJS)
endif

#-------------------------------------------------------------------------------
# If console support is included, all the console objects from above
#-------------------------------------------------------------------------------
ifeq ($(BCM_CONSOLE_SUPPORT),1)
  # From Snmp/DocsisSnmp.mk: console objects (command tables, etc)
  SNMP_OBJS += $(CONSOLE_SNMP_OBJS)
endif


############################################################################
##                 CFLAGS
############################################################################

ADDED_CFLAGS += $(ADDED_SNMP_FLAGS)

ifeq ($(SNMP_SUPPORT), 3)

  ifeq ($(BCM_KERBEROS_SUPPORT), 0)
   ADDED_BPI_LIB += $(OPENSSL_DIR)/$(SEC_LIBNAME)
  endif
endif


############################################################################
##                 EXTRA_INCLUDE - include path
############################################################################

EXTRA_INCLUDE  +=  -I$(TARGETOS_V2_SNMP_DIR) \
		   -I$(NETSNMP_LIB_INCLUDE_DIR) \
		   -I$(STB_SNMP_DIR) \
		   -I$(ESTB_SNMP_DIR)

ifeq ($(SNMP_PROXY_SUPPORT), 1)
    EXTRA_INCLUDE  +=  -I$(SNMP_PROXY_DIR)
endif

ifeq ($(MOCA_MIB_SUPPORT), 1)
    EXTRA_INCLUDE  +=  -I$(MOCA_LIB_INCLUDE_DIR)
endif

#
# Additional include directories form DocsisSnmp.mk, if defined
#
ifeq ($(origin CORE_SNMP_INCS), undefined)
else
  EXTRA_INCLUDE += -I$(CORE_SNMP_INCS)
endif

ifeq ($(origin DOCSIS_10_SNMP_INCS), undefined)
else
  EXTRA_INCLUDE += -I$(DOCSIS_10_SNMP_INCS)
endif

ifeq ($(origin DOCSIS_11_SNMP_INCS), undefined)
else
  EXTRA_INCLUDE += -I$(DOCSIS_11_SNMP_INCS)
endif

ifeq ($(SNMP_SUPPORT), 3)
  ifeq ($(origin V3_SNMP_INCS), undefined)
  else
    EXTRA_INCLUDE += -I$(V3_SNMP_INCS)
  endif
endif

############################################################################
##                 VPATH - source path
############################################################################

VPATH += $(TARGETOS_V2_SNMP_DIR) \
	$(STB_SNMP_DIR) \
	$(ESTB_SNMP_DIR)

ifeq ($(SNMP_PROXY_SUPPORT), 1)
    VPATH +=  \
        $(SNMP_PROXY_DIR)
endif

############################################################################
##                 LIB_EXTRA - libraries
############################################################################

ifeq ($(MOCA_MIB_SUPPORT), 1)
    LIB_EXTRA  +=  $(MOCA_LIB)
endif

# Additional libraries from DocsisSnmp.mk, if defined
ifeq ($(origin CORE_SNMP_LIBS), undefined)
else
  LIB_EXTRA += $(CORE_SNMP_LIBS)
endif

ifeq ($(origin DOCSIS_10_SNMP_LIBS), undefined)
else
  LIB_EXTRA += $(DOCSIS_10_SNMP_LIBS)
endif

ifeq ($(origin DOCSIS_11_SNMP_LIBS), undefined)
else
  LIB_EXTRA += $(DOCSIS_11_SNMP_LIBS)
endif

ifeq ($(SNMP_SUPPORT), 3)
  ifeq ($(origin V3_SNMP_LIBS), undefined)
  else
    LIB_EXTRA += $(V3_SNMP_LIBS)
  endif
endif

ifeq ($(origin NETSNMP_LIBS), undefined)
else
    LIB_EXTRA += $(NETSNMP_LIBS)
endif
