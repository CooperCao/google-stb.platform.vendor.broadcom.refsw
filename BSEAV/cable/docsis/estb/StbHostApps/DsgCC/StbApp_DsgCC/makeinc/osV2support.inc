#############################################################################
# (c) 2003-2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################

############################################################################
##                 DEFINES
############################################################################



############################################################################
##                 PATHS
############################################################################

# For most plaforms and OSs, we can't use the BCM HeapManager.
BCM_HEAPMANAGER_SUPPORT=0

ifeq "$(TARGETOS)" "vxWorks"
  VXWORKS_BASE_INC  = $(WIND_BASE)/target/h

  ifeq ($(SNMP_WINDMANAGE), 1)
    TARGETOS_SNMP_INC = ../../../Snmp/Envoy/vxworks
  else
    TARGETOS_SNMP_INC  = $(WIND_BASE)/target/h/snmp
  endif

  ifeq ($(TORNADO_VERSION), 20)
    # Heap Manager - replacement for malloc/free.  Only supported
    # for Tornado 2.0 for now.
    BCM_HEAPMANAGER_SUPPORT=1
  endif

  ifeq ($(TORNADO_VERSION), 22)
    VXWORKS_GCC_INC   = $(WIND_BASE)/host/x86-win32/include/g++-3
    EXTRA_INCLUDE    += -I$(VXWORKS_GCC_INC)
  endif

  ifeq ($(TORNADO_VERSION), 21)
    VXWORKS_GCC_INC   = $(WIND_BASE)\host\x86-win32\lib\gcc-lib\mips-wrs-vxWorks\gcc-2.96\include
    EXTRA_INCLUDE    += -I$(VXWORKS_GCC_INC)
  endif

  ifeq ($(CPU), VR5400)
    VXWORKS_FCS2_INC  = $(WIND_BASE)/host/x86-win32/include/g++-2
  endif
endif

ifeq "$(TARGETOS)" "Linux"
  TARGETOS_SNMP_INC = /tools/crossdev/snmp/h
endif

############################################################################
##                 OBJECT FILES
############################################################################

############################################################################
##                 DSGCC_OBJS - object files
############################################################################

ifeq "$(TARGETOS)" "vxWorks"
  DSGCC_OBJS    += $(ALL_VXWORKS_OBJS)
endif

ifeq ($(BCM_HEAPMANAGER_SUPPORT),1)
  DSGCC_OBJS     += $(TARGETDIR)/BcmHeapManager.o $(TARGETDIR)/BcmHeapManagerMutexApiVxWorks.o \
                    $(TARGETDIR)/HeapManagerCommandTable.o
  ADDED_CFLAGS   += -DBCM_HEAPMANAGER_SUPPORT=1
endif

############################################################################
##                 EXTRA_INCLUDE - include path
############################################################################

ifeq "$(TARGETOS)" "vxWorks"
  ifeq ($(CPU), VR5400)
    EXTRA_INCLUDE += -I$(VXWORKS_FCS2_INC)
  endif
endif

EXTRA_INCLUDE += -I$(TARGETOS_SNMP_INC) -I$(V2_COMMON_DIR)/$(TARGETOS)

ifeq ($(BCM_HEAPMANAGER_SUPPORT),1)
  EXTRA_INCLUDE  += -I../../../Common/BcmHeapManager
endif


############################################################################
##                 VPATH - source path
############################################################################

ifeq "$(TARGETOS)" "vxWorks"
  VPATH         += $(VXWORKS_SNMP_OBJS_DIR)
endif

VPATH += $(V2_COMMON_DIR)/$(TARGETOS)

ifeq ($(BCM_HEAPMANAGER_SUPPORT),1)
  VPATH          += ../../../Common/BcmHeapManager
endif

############################################################################
##                 LIB_EXTRA - libraries
############################################################################


