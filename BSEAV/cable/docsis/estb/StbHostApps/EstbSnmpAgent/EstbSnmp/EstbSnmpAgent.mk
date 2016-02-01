# ****************************************************************************
# 
#  Copyright (c) 2007-2012 Broadcom Corporation
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
#
# Include paths, MIBs, objects, and libraries for DOCSIS 1.0 specific things
#
STBSNMP_OBJS = ObjectIdentifier.o VariableBinding.o BerObjs.o BcmBasicString.o \
                      EstbSnmpAgent.o EstbSnmpAgentStatusACT.o \
                      EstbEventLog.o \
                      EstbEventLogNonVolSettings.o \
                      EventLogCommandTable.o \
                      EventLog.o EventLogThread.o EventLogNonVolSettings.o \
                      docsDevSyslog.o \
                      EstbSnmpNonVolSettings.o NmAccess.o \
                   MibObjs.o MibObjList.o MibBridge.o \
                   SnmpApi.o BcmSnmpAgent.o SnmpV3Agent.o SnmpThread.o \
                   BcmSyslog.o BcmTrap.o standardTrap.o \
                   BcmSnmpTrapHandler.o SnmpTrapExploderThread.o \
                   snmpV2Mib.o systemMib.o systemBridge.o SLog.o \
                      estbDocsDevMib.o estbDocsDevBridge.o docsDevTranslator.o \
                      docsDevEvMib.o docsDevEvBridge.o \
                      docsDevNmMib.o docsDevNmBridge.o \
                      ifMib.o estbIfBridge.o \
                      ipMib.o ipBridge.o lnxIpBridge.o \
                      udpMib.o udpBridge.o estbUdpBridge.o \
                      hrMib.o hrBridge.o lnxHrBridge.o \
                      MibFactory.o EstbConfig.o EstbTLV.o EstbTLVcodes.o \
                      EstbSnmpV3NotifyReceiver.o EstbSnmpV1V2Coexistence.o \
                      EstbSnmpV1V2Transport.o EstbVendorId.o \
                      EstbSnmpV3AccessView.o EstbOsMonitorThread.o \
                      DocsisCfgVarBindEngine.o DocsisCfgWacEngine.o \
                      PktFilterController.o IpFilter.o EthernetPacket.o

STBSNMP_CONSOLE_OBJS = SnmpCommandTable.o BcmSnmpAgentConsole.o \
                      EstbSnmpNonVolSettingsCommandTable.o \
                      EventLogNonVolSettingsCommandTable.o \
                      TrapHandlerCommandTable.o \
                      EstbSnmpAgentConsole.o SnmpV3AgentConsole.o

STBSNMP_SNMPV3_OBJS = snmpDH.o DhKickstartThread.o \
                     communityMib.o communityBridge.o \
                     usmDhMib.o usmDhBridge.o

STBSNMP_BRCM_RUNTIME_OBJECTS = brcmSnmpMgmtMib.o brcmSnmpMgmtBridge.o \
                           brcmBfcMgmtMib.o brcmBfcMgmtBridge.o

# Objects for NetSNMP agent core
STBSNMP_NETSNMP_OBJS = AgentUtils.o BcmV2Glue.o BrcmDomain.o NetsnmpAgnt.o
STBSNMP_NETSNMP_V3_OBJS = Kickstart.o NotifyAgnt.o VacmAgnt.o

# Objects for proxying for the eSTB code 
SNMP_PROXY_OBJS  = ProxySnmpAgent.o Proxy.o ProxyPdu.o 
SNMP_PROXY_OBJS += ocStbHostMib.o ocStbHostBridge.o ocStbHostTrap.o
SNMP_PROXY_OBJS += ocHnMib.o ocHnBridge.o 

# Objects for MOCA MIB support 
MOCA_MIB_OBJS  = mocaMib.o mocaBridge.o MocaMonitorThread.o dslcompat.o

# Objects for Local SNMP Agent support 
LOCAL_SNMP_AGENT_OBJS  = LocalSnmpAgent.o

# Object files which are compiled if FACTORY_SUPPORT=1.                    
STBSNMP_FACTORY_OBJECTS = brcmV2FactoryMib.o brcmV2FactoryBridge.o

# Object files which are compiled if SNMP_SUPPORT=0.                    
DOCSIS_STUB_SNMP_OBJS = SnmpNonVolSettings.o \
                        EventLogThread.o EventLog.o \
                        EventLogNonVolSettings.o \
                        EstbEventLogNonVolSettings.o \
                        EstbEventLog.o BcmSyslog.o docsDevSyslog.o BcmBasicString.o CoreObjs.o
DOCSIS_STUB_SNMP_CONSOLE_OBJS = SnmpNonVolSettingsCommandTable.o EventLogCommandTable.o \
                        EventLogNonVolSettingsCommandTable.o
