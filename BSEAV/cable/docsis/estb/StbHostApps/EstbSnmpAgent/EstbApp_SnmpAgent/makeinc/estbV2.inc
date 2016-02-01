##############################################################################
#
# Copyright (c) 2007-2010 Broadcom Corporation
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

############################################################################
##                 V2 COMPONENT INCLUDES 
############################################################################


############################################################################
##                 PATHS
############################################################################  
                  
ifeq ($(origin OPENSSL_DIR), undefined)
  OPENSSL_DIR     = ../../../LibSupport/OpenSSL
endif

ifeq ($(GCC_VERSION),4)
  OPENSSL_DIR     = ../../../LibSupport/OpenSSL
endif

ifeq "$(TARGETOS)" "Linux"
  CORE_LINUX_APP_DIR = ../../../EstbApp_SnmpAgent/linux
endif

############################################################################
##                 OBJECT FILES
############################################################################                                       

############################################################################
##                 CFLAGS
############################################################################                                       

ifeq ($(BCM_STANDALONE_SNMP_AGENT),1)
ADDED_CFLAGS += -DBCM_STANDALONE_SNMP_AGENT=1
endif

ifneq ($(SNMP_SUPPORT), 0)
ifeq ($(ESTB_SNMP_AGENT), 1)
  ADDED_SNMP_FLAGS += -DESTB_SNMP_AGENT=1
endif
endif

ifeq ($(SNMP_SOCKET_MODE),1)
ADDED_CFLAGS += -DBCMV2_SOCK_UCD=1
endif

ifeq ($(SETTOP_DHCP_DNS_SUPPORT),1)
ADDED_CFLAGS += -DBCM_DHCP_DNS_SUPPORT=1
endif

ifeq ($(ESTBSNMP_BUILT_AS_LIB),1)
ADDED_CFLAGS += -DESTBSNMP_BUILT_AS_LIB -DBCM_NULL_CONSOLE -DBCM_NULL_MESSAGE_LOG
endif

ifneq ($(ESTBSNMP_BUILT_AS_LIB),1)
ifeq ($(BCM_NULL_MESSAGE_LOG),1)
ADDED_CFLAGS += -DBCM_NULL_MESSAGE_LOG
endif
ifeq ($(BCM_NULL_CONSOLE),1)
ADDED_CFLAGS += -DBCM_NULL_CONSOLE
endif
endif

ifeq ($(CORE_ESTBSNMP_BUILT_AS_LIB),1)
ADDED_CFLAGS += -DCORE_ESTBSNMP_BUILT_AS_LIB 

ifeq ($(BCM_CONSOLE_SUPPORT),1)
ADDED_CFLAGS   +=  -DBCM_CONSOLE_SUPPORT=1      
endif
endif

ADDED_CFLAGS      += -DOPENSSL=1 -DSOFTWARE_CRYPTO_LIB=1 -DSOFTWARE_CRYPTO_LIB_INCLUDED=1
ADDED_OPENSSL_LIB  = $(OPENSSL_DIR)/$(SEC_LIBNAME)
ADDED_OPENSSL_LIB  += $(OPENSSL_DIR)/$(OPENSSL_LIBNAME)

############################################################################
##                 EXTRA_INCLUDE - include path
############################################################################     
                                  
ifeq "$(TARGETOS)" "Linux"
  EXTRA_INCLUDE += -I$(CORE_LINUX_APP_DIR)
endif

ifeq ($(SOFTWARE_CRYPTO_LIB),OPENSSL)
      EXTRA_INCLUDE  += -I$(OPENSSL_DIR)/include/openssl
      EXTRA_INCLUDE  += -I$(OPENSSL_DIR)/include
endif


############################################################################
##                 VPATH - source path
############################################################################     
                                  
ifeq "$(TARGETOS)" "Linux"
  VPATH += $(CORE_LINUX_APP_DIR)
endif

############################################################################
##                 MACH_EXTRA - object files
############################################################################                                       

MACH_EXTRA += $(SNMP_OBJS)

############################################################################
##                 LIB_EXTRA - libraries
############################################################################                                       
ifeq ($(SOFTWARE_CRYPTO_LIB),OPENSSL)
   LIB_EXTRA   += $(ADDED_OPENSSL_LIB)
endif
