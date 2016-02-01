//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//**************************************************************************
//    Filename: SnmpApi.h
//    Author:   Kevin O'Neal
//    Creation Date: 21-Jan-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Basic classes for dealing with SNMP objects and concepts
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef SNMPAPI_H
#define SNMPAPI_H

#include "SnmpType.h"
#include <time.h>

#define UNSPECIFIED_IF_INDEX    0


#if defined (__cplusplus)

#include "BcmSnmpAgent.h"

#ifndef BCM_STANDALONE_SNMP_AGENT 

#include "HalIf.h"

// C++ only APIs for use internally by V2
int SendLinkTrap (int TrapType, BcmHalIf *pHal);
int SendLinkTrap (int TrapType, BcmHalIf::IanaInterfaceType ifType);

#endif


extern "C"
{
#endif

  /*\
   *  Non-portable functions which must be supplied by particular agent
  \*/

  void AgentInitSnmp (int Priority);
  const char *AgentGetName ();
  void AgentStartSnmp ();
  int AgentGetDebugLevel ();
  int AgentSetDebugLevel (int Level);
  void AgentShow ();
  
  // Configure the agent for a packet of MaxSize.  Note that this may be called
  // on the fly, not just at startup.
  int AgentSetMaxPacketSize (int MaxSize);
  
  SNMP_STATUS AgentRegisterScalar (const ObjectId *pOid, const char *pName);
  SNMP_STATUS AgentRegisterGroup (const ObjectId *pOid, const char *pName);
  SNMP_STATUS AgentRegisterTabular (const ObjectId *pOid, const char *pName);
  
  int AgentGetAuthFailTrapsEnabled ();
  
  SNMP_STATUS AgentSendV1Trap (char *pCommunity, unsigned char *pFromIP, 
              unsigned char *pToIP, int ToPort, ObjectId *pEnterprise,  
              int GenericId, int SpecificId, time_t SysUpTime, 
              MibObject *pObjects);
              
  SNMP_STATUS AgentSendV2Trap (char *pCommunity, unsigned char *pFromIP, 
              unsigned char *pToIP, int ToPort, ObjectId *pTrapOid, 
              time_t SysUpTime, MibObject *pObjects);
              
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentSendV2TrapInform
  //
  // PURPOSE:    Send a SNMPv2 trap or Inform message with the specified OID and variable
  //             bindings. This function is a replacement for AgentSendV2Trap.  It takes
  //             an extra parameter 'ptype' to specify whether to send a trap or inform.
  //             Both trap and inform are sent directly from here.  That is, the inform 
  //             is only sent once and is not retried.  
  //
  // PARAMETERS: int ptype:  PDU_TRAP2' or 'PDU_INFORM_REQUEST'
  //             
  // RETURNS:    SNMP_NO_ERROR on success
  //             
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentSendV2TrapInform (int ptype, char *pCommunity, unsigned char *pFromIP,
              unsigned char *pToIP, int ToPort, ObjectId *pTrapOid, 
              time_t SysUpTime, MibObject *pObjects);
  
  // Return -1 if you want the app to read the data from the socket and pass it
  // to the agent core later in an AgentProcessSnmpPacket call.  If you want to
  // read the data yourself in the agent, then do so and return something
  // other than -1.  0 is fine, we don't check anything but != -1 in the app.
  // If something besides -1 is returned, the app will assume the agent has
  // handled this packet and will not read the data from the socket or call
  // AgentProcessSnmpPacket.
  int AgentInformPacketAvailable (unsigned long DstIp, unsigned short DstPort, int Socket);

  // IMPORTANT: the pPkt pointer here is allocated by the V2 code for the
  // benefit of those agents that use it directly instead of copying it.  The
  // agent is responsible for freeing this buffer when finished with it!
  SNMP_STATUS AgentProcessSnmpPacket (unsigned char *pPkt, int PktLen, int Socket,
                                      unsigned long SrcIp, unsigned long DstIp, 
                                      int SrcPort, int DstPort);
                                      
  // Returns 1 if supported, 0 if not.  This is necessary because Epilogue
  // will return an error code of NOT_WRITABLE if you try to set an unknown
  // OID.  So this allows us to determine if the object is really not writable,
  // or if it's just flat out not supported.  If a pView pointer is provided,
  // we'll check the specified view as well.
  int AgentIsObjectSupported (const ObjectId *pOid, const char *pView);
              
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentSendNotify
  //
  // PURPOSE:    Send a RFC 2573 SNMPv3 Notify message with the specified OID and variable
  //             bindings.  The snmpNotify, snmpTargetAddr, and snmpTargetParams tables
  //             determine how and where this notify is sent.  These tables must be initialized
  //             prior to this call. 
  //
  // PARAMETERS: uchar     *pFromIp:   IP address notify should originate from.
  //             ObjectId  *pTrapOid:  The OID used to identify this trap.
  //             time_t     SysUpTime: Time system has been up.
  //             MibObject *pObjects:  Ptr to objects to include in notify.
  //
  // RETURNS:    SNMP_NO_ERROR on success
  //             
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentSendNotify (unsigned char *pFromIp, ObjectId *pTrapOid, time_t SysUpTime, MibObject *pObjects);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentGetEngineId
  //                      
  // PURPOSE:    Gets our Agent's SNMPv3 EngineId 
  //
  // PARAMETERS: pId:      A ptr to a buffer where the EngineId will be copied.
  //             MaxLen:   The length of pId buffer. 
  //             
  // RETURNS:    int: Number of bytes copied into pId
  ////////////////////////////////////////////////////////////////////////////////
  int AgentGetEngineId (unsigned char *pId, int MaxLen);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentSetEngineId
  //                      
  // PURPOSE:    Sets our Agent's SNMPv3 EngineId to the desired value.
  //             EngineBoots is set to the value from NonVol Mem and EngineTime
  //             is set to zero.  This is an initalization function and should only
  //             be called once. 
  //
  // PARAMETERS: pId:      A ptr to the buffer containing the engineId to set.
  //             MaxLen:   The length of pId buffer. This may not exceed MAX_ENGINE_ID_LEN.
  //             
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentSetEngineId (unsigned char *pId, int Length);

  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentUpdateEngineTimes
  //                      
  // PURPOSE:    Sets our Agent's SNMPv3 EngineId boots and time values.
  //             
  // PARAMETERS: 
  //             
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentUpdateEngineTimes ();

  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentGetContext
  //                      
  // PURPOSE:    Get the SNMPv3 context. 
  //
  // PARAMETERS: char *pContext:   The buffer that the context will be copied into.
  //             int  MaxLen:      The length of  the pContext buffer.
  //             
  // RETURNS:    SNMP_NO_ERROR on success.
  ////////////////////////////////////////////////////////////////////////////////
  int AgentGetContext (char *pContext, int MaxLen);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentSetContext
  //                      
  // PURPOSE:    Adds an SNMPv3 Context.
  //
  // PARAMETERS: char *Context:  The context to add.
  //             
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentSetContext (char *pContext);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentDestroyContext
  //                      
  // PURPOSE:    Removes an SNMPv3 Context.
  //
  // PARAMETERS: char *Context:  The context to remove.
  //             
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentDestroyContext (char *pContext);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentConfigView
  //                      
  // PURPOSE:    This function is called to add SNMPv3 views. 
  //             It may be called any number of times to either create a new
  //             view or to include/exclude elements of an exiting view.
  //
  // PARAMETERS: char *pViewName:         The name of the view.
  //             unsigned long *pSubtree: The OID to exclude/include.
  //             int SubtreeSize:         The size of pSubtree.
  //             SNMP_VIEW_STATUS Status: VIEW_STATUS_INCLUDE, VIEW_STATUS_EXCLUDE
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentConfigView (char *pViewName, unsigned long *pSubtree, int SubtreeSize, SNMP_VIEW_STATUS Status);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentDestroyView
  //                  
  // PURPOSE:    Called to remove a view.  Any allocated memory is cleaned up.
  //
  // PARAMETERS: char *pViewName:         The name of the view to destroy.
  //             unsigned long *pSubtree: The subtree associated with the view.
  //             int SubtreeSize:         The size of pSubtree.
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentDestroyView (char *pViewName, unsigned long *pSubtree, int SubtreeLen);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentCleanupViews
  //                      
  // PURPOSE:    This function is called to cleanup ALL views. 
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentCleanupViews ();
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentInstallGroup
  //                      
  // PURPOSE:    Installs an SNMPv3 group and associates it with a user.
  //
  // PARAMETERS: char *pGroupName:         The name of the group to create.
  //             char *pSecurityName:      The user name.
  //             SNMP_SEC_MODEL SecModel:  The security model - SNMP_SEC_MDL_USM.
  //  
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentInstallGroup (char *pGroupName, char *pSecurityName, SNMP_SEC_MODEL SecModel);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentDestroyGroup
  //                      
  // PURPOSE:    Removes an SNMPv3 group and cleans up any allocated memory.
  //
  // PARAMETERS: char *pGroupName:         The name of the group to remove.
  //  
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentDestroyGroup (char *pGroupName);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentCleanupGroups
  //                      
  // PURPOSE:    Removes ALL SNMPv3 groups and cleans up any allocated memory.
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentCleanupGroups ();
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentInstallAccess
  //                      
  // PURPOSE:    Installs an SNMPv3 access.  It associates views with a group.
  //
  // PARAMETERS: char *pGroupName:    The group name.
  //             char *pReadView:   The view this user will use for read access.
  //             char *pWriteView:  The view this user will use for write access.
  //             char *pNotifyView: The view this user will use for notify access.
  //             SNMP_SEC_LEVEL Security:  SNMP_SEC_NONE, SNMP_SEC_AUTH, SNMP_SEC_PRIV. 
  //             SNMP_SEC_MODEL SecModel:  SNMP_SEC_MDL_USM.  
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentInstallAccess (char *pGroupName, char *pReadView, char *pWriteView, char *pNotifyView, SNMP_SEC_LEVEL Security, SNMP_SEC_MODEL SecModel);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentDestroyAccess
  //                      
  // PURPOSE:    Removes an SNMPv3 access and any allocated memory.
  //
  // PARAMETERS: char *pGroupName:    The group name.
  //             SNMP_SEC_LEVEL Security:  SNMP_SEC_NONE, SNMP_SEC_AUTH, SNMP_SEC_PRIV. 
  //             SNMP_SEC_MODEL SecModel:  SNMP_SEC_MDL_USM.  
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentDestroyAccess (char *pGroupName, SNMP_SEC_LEVEL Security, SNMP_SEC_MODEL SecModel);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentCleanupAccess
  //                      
  // PURPOSE:    Removes ALL SNMPv3 accesses and any allocated memory.
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentCleanupAccess ();
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentInstallUser
  //                      
  // PURPOSE:    Installs an SNMPv3 user   
  //
  // PARAMETERS: pUserName:  The SNMPv3 security user name to install.                      
  //             Security:   SNMP_SEC_NONE (no auth no priv)                     
  //                         SNMP_SEC_AUTH (auth no priv)                        
  //                         SNMP_SEC_PRIV (auth and priv)
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentInstallUser (char *pUserName, SNMP_SEC_LEVEL Security);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentEnableUser
  //                      
  // PURPOSE:    Enables an SNMPv3 user by setting it's status to Active   
  //
  // PARAMETERS: pUserName:  The SNMPv3 security user name.                                 
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentEnableUser (char *pUserName);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentDisableUser
  //                      
  // PURPOSE:    Disables an SNMPv3 user by setting it's status to NotInService   
  //
  // PARAMETERS: pUserName:  The SNMPv3 security user name.                                 
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentDisableUser (char *pUserName);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentDestroyUser
  //                      
  // PURPOSE:    Removes an SNMPv3 user and any allocated memory.   
  //
  // PARAMETERS: pUserName:  The SNMPv3 security user name.                                 
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentDestroyUser (char *pUserName);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentCleanupUser
  //                      
  // PURPOSE:    Removes ALL SNMPv3 users and allocated memory.   
  //
  // PARAMETERS: pUserName:  The SNMPv3 security user name.                                 
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentCleanupUsers ();
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentProvTargetParamsTable
  //                      
  // PURPOSE:    Creates a row in the SNMPv3 snmpTargetParamsTable.  This is 
  //             usually done for each TLV 38 element found in the Config File 
  //
  // PARAMETERS: int     TrapType:        1- SNMPv1  trap in SNMPv1  packet
  //                                      2- SNMPv2c trap in SNMPv2c packet
  //                                      3- SNMP  inform in SNMPv2c packet
  //                                      4- SNMPv2c trap in SNMPv3  packet
  //                                      5- SNMP  inform in SNMPv3  packet                               
  //             uchar * ElementName:     The snmpTargetParamsName
  //             int     ElementNameLen:  Length of the ElementName
  //             uchar * SecurityName:    snmpTargetParamsSecurityName
  //             int     SecurityNameLen: Length of SecurityName
  //          
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentProvTargetParamsTable (int TrapType,unsigned char * ElementName,
										  int ElementNameLen, unsigned char * SecurityName,
										  int SecurityNameLen, unsigned char *tlv38Base);

  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentProvTargetAddrTable
  //                      
  // PURPOSE:    Creates a row in the SNMPv3 snmpTargetAddrTable.  This is 
  //             usually done for each TLV 38 element found in the Config File 
  //
  // PARAMETERS: uchar *TargetIpAddr: value of ip Address for snmpTargetAddrTAddress
  //             int    Port:         value of port for snmpTargetAddrTAddress
  //             int    TrapType:     1- SNMPv1  trap in SNMPv1  packet
  //                                  2- SNMPv2c trap in SNMPv2c packet
  //                                  3- SNMP  inform in SNMPv2c packet
  //                                  4- SNMPv2c trap in SNMPv3  packet
  //                                  5- SNMP  inform in SNMPv3  packet 
  //             int    TimeOut:      value for snmpTargetAddrTimeout
  //             int    Retries:      value for snmpTargetAddrRetryCount
  //             uchar *ElementName:     The snmpTargetAddrParamsName
  //             int    ElementNameLen:  Length of the ElementName
  //             
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentProvTargetAddrTable (unsigned char * TargetIpAddr,int  Port,
  									    int  TrapType,int  TimeOut,int  Retries,
  									    unsigned char * ElementName,int ElementNameLen,
										unsigned char *tlv38BaseInform, unsigned char *tlv38BaseTrap);

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentProvTLV53TargetAddrTable
//
// PURPOSE:    Creates a row in the SNMPv3 snmpTargetAddrTable.  This is
//             done for each TLV 53 element found in the Config File
//
// PARAMETERS: uchar *name:     The snmpTargetAddrName
//             uchar *addr:     The snmpTargetAddrTAddress
//             uchar *addr:     The snmpTargetAddrTagList
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentProvTLV53TargetAddrTable (unsigned char *name, unsigned char* addr,
                                      unsigned char *tagList);

  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentProvNotifyFilterProfileTable
  //                      
  // PURPOSE:    Configures the SNMPv3 snmpNotifyFilterProfileTable. May be done 
  //             for TLV 38 elements found in the Config File. 
  //
  // PARAMETERS: uchar *ElementName:     The snmpNotifyFilterProfileName
  //             int    ElementNameLen:  Length of the ElementName
  //             
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentProvNotifyFilterProfileTable (unsigned char *ElementName,int ElementNameLen);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentProvNotifyFilterTable
  //                      
  // PURPOSE:    Configures the snmpNotifyFilterTable. May be done for TLV38
  //             elements found in the Config File. 
  //
  // PARAMETERS: uchar *ElementName:     The snmpNotifyFilterProfileName
  //             int    ElementNameLen:  Length of the ElementName
  //             ulong *FilterOid:       The Oid to use for the filter.
  //             int    FilterOidLen:    Length of the FilterOid
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentProvNotifyFilterTable (unsigned char *ElementName, int elementNameLen, unsigned long *FilterOid, int FilterOidLen);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentProvNotifyTable
  //                      
  // PURPOSE:    Configures the snmpNotifyTable per the DOCSIS 1.1 requirements. 
  //             Typically called once if any TLV38 element found in the Config File. 
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentProvNotifyTable (unsigned char * tlv38inform, unsigned char * tlv38trap);
  
  ////////////////////////////////////////////////////////////////////////////////
  // FUNCTION:   AgentCleanupNotifies
  //                      
  // PURPOSE:    Clean up notify tables 
  //
  // RETURNS:    SNMP_NO_ERROR on success
  ////////////////////////////////////////////////////////////////////////////////
  SNMP_STATUS AgentCleanupNotifies ();

  // API for adding DOCSIS 1.1 defined VACM views (from SP-OSSIv1.1-I02, section 2.2.4):
  // pViewName could be "docsisManager", "docsisOperator", "docsisMonitor", 
  // or "docsisUser". These would come from the DOCSIS 1.1 config file.
  // pDhPublicKey is the diffie-helman public key for this view, and
  // DhKeySize is the size of the key.
  SNMP_STATUS AgentInstallProvisionedView (char *pVacmUserName, unsigned char *pDhPublicKey, int DhKeySize);
   
  // API to allow the non portable agent to do any necessary cleanup at
  // restart or shutdown
  SNMP_STATUS AgentInformRestartPending (int IfNum);
  SNMP_STATUS AgentInformShutdownPending (int IfNum);
  
  SNMP_STATUS AgentKickstartRegister (int IfNum, char * pSecurityName, char * pMgrPublicNum, int len_mgrPublicNum);
  SNMP_STATUS AgentKickstartFinish (int IfNum);							  
  SNMP_STATUS AgentInstallInformUserFromProbe (unsigned char *pUserName, unsigned int UserNameLen, unsigned char * pEngId, int EngIdLen);

       
  /*\
   *  Portable functions called by the non-portable agent
  \*/
  
  void AppSnmpStatusAsString (SNMP_STATUS Status, char *pStatusStr, int MaxSize);
  
  unsigned long AppGetEngineBoots ();
  int AppGetInterfaceNumber (unsigned char *AgentIp);
  
  // Opens a socket and binds it to IpAddr:Port, unless Port is not specified.
  // In that case it simply opens a socket, using IpAddr to determine the IP
  // stack to use.  In either case, the socket descriptor is 
  // returned.  The socket is created by and stored in the BcmSnmpThread.
  // Note that the socket created is appropriate for SNMP, which means it's
  // a UDP socket, etc.
  int AppOpenSnmpSocket (unsigned long IpAddr, unsigned short Port);
  int AppCloseSnmpSocket (int Socket);
  
  // Read data from a socket (recvfrom).
  int AppRecvfromSnmpSocket (int Socket, unsigned char *pBuf, int BufSize, unsigned long *pFromIp, unsigned short *pFromPort);
  
  // Write data to a socket (sendto).
  int AppSendtoSnmpSocket (int Socket, const unsigned char *pBuf, int BufSize, unsigned long ToIp, unsigned short ToPort);
  
  const char *AppGetV1V2View (const unsigned char *AgentIp);
  
  // This function is called early, to validate the entire SNMP request.  It's
  // called once per PDU, not once per VB.  The app at this point has the
  // opportunity to reject the entire PDU (manager receives a timeout).  You
  // can't evaluate individual VB's here.
  SNMP_STATUS AppValidateSnmpRequest (int PduType, int SnmpVersion, const unsigned char *SA, int SrcPort, const unsigned char *pDA, int DstPort, const char *pCommunity, char *pSecurityName, int MaxLen);
  
  // This function is called once per VB, before calling AppTestObject.  The
  // point of this function is to allow us to apply any special rules to 
  // each VB individually and possibly reject it.  The important difference
  // between this function and AppTestObject is that this one is called for
  // ALL VB's in the PDU, even if they are handled by the core agent and not
  // V2 (ie USM/VACM objects).  An example of something which uses this type
  // of validation is DOCSIS TLV-10's from the config file (write access
  // control).
  SNMP_STATUS AppValidateSetRequest (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion);
  
  // generic API's for dealing with all objects, called by the
  // non-portable SNMP agent.
  SNMP_STATUS AppGetObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion);
  SNMP_STATUS AppNextObject (ObjectId *pOID, ObjectId *pIndex, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion);
  SNMP_STATUS AppTestObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion);
  SNMP_STATUS AppSetObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion);
  
  // This API is called from the agent to the app, since the agent doesn't
  // know anything about the NmAccessTable and the trap addr list.
  SNMP_STATUS AppSendTrap (const unsigned char *AgentIp, int TrapType, int IfIndex);
  
  int AppInformSentNotify ();

  SNMP_STATUS AppProvTargetAddrExtTable (unsigned char *pIndexOid, int IndexOidLen);
  SNMP_STATUS AppDestroyTargetAddrExtTable (unsigned char *snmpTargetAddrName, int snmpTargetAddrNameLen);
  SNMP_STATUS AppResolveSecNameToCommName ( unsigned char *pToIp, int ToPort, unsigned char *pFromIp, char *pCommunity, char *pSecurityName, int MaxLen);

  // Note that this structure only describes the elements of the 
  // snmpTargetAddrTable which are interesting for snmpCommunityTable based
  // coexistence.  We can add other elements if they are ever needed.
  typedef struct _TargetAddr
  {
    char *pName;
    int NameLen;
    unsigned long *pDomain; // snmpUDPDomain = 1.3.6.1.6.1.1
    int DomainLen;
    unsigned char *pAddr; // for snmpUDPDomain, 4 bytes are IPv4 addr, 2 bytes port #
    int AddrLen;
    char *pTagList;
    int TagListLen;
    int Status;
  } TargetAddr;
  
  SNMP_STATUS AgentFindTargetAddr (const char *pName, int NameLen, TargetAddr *pAddr);


////////////////////////////////////////////////////////////////////////////////
//                                                                           
//  FUNCTION:  AgentTargetParamsTableAddRow                                   
//                                                                              
//  DESCRIPTION: Sets a row in the SnmpTargetParamsTable                                 
//  ARGUMENTS:   elementName    - The name to use as the index for this row in the table.
//               elementNameLen - The length of the elementName.
//               mpModel        - The MessageProcessing Model.
//               secModel       - The Security Model.
//               securityName   - The security/user name to use for sending this notify.
//               securityNameLen- The length of the securityName.
//               secLevel       - The security level.  This should match that used by the securityName
//                                Possible values are: SNMP_SEC_LEVEL_AUTHPRIV                                                     
//               storageType     - The storage type.
//
//  RETURNS:     SNMP_NO_ERROR if successful
//                                                                           
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentTargetParamsTableAddRow(unsigned char * elementName, int  elementNameLen,int mpModel,int secModel,unsigned char * securityName,int securityNameLen,int secLevel,int storageType);

////////////////////////////////////////////////////////////////////////////////
//                                                                            
//  FUNCTION:  AgentTargetAddrTableAddRow                                         
//                                                                              
//  DESCRIPTION: Sets the snmpTargetAddrTable
//                                                                             
//  ARGUMENTS:   TagName - The name for the tag object.
//               targetIpAddr  - Addr to send traps to.
//               Port          - Port to send traps to (usually 162).
//               TimeOut       - Timeout before retry.
//               Retries       - Number of times to retry.
//               ElementName   - Index and snmpTargetAddrParams object value.
//               ElementNameLen- Length of elementName.
//                                                            
//  RETURNS:     Zero if successful                                                               
//                                                                            
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentTargetAddrTableAddRow(char * TagName,unsigned char *TargetIpAddr,int Port,int TimeOut,int Retries,unsigned char *ElementName,int ElementNameLen);

////////////////////////////////////////////////////////////////////////////////
//                                                                            
//  FUNCTION:  AgentNotifyTableAddRow                                             
//                                                                              
//  DESCRIPTION: Sets the snmpNotifyTable 
//                                                               
//  ARGUMENTS:  Name - The Name to be used for the notify.
//              TagName - The name to be used for the snmpNotifyTable tag param.
//              NotifyType - should be either EMTA_SNMP_NOTIFY_TRAP or 
//                                            EMTA_SNMP_NOTIFY_INFORM                                                                *
//  RETURNS:    Zero if successful                                                                
//                                                                            
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentNotifyTableAddRow(char * Name, char * TagName, int NotifyType);

////////////////////////////////////////////////////////////////////////////////
//                                                                            
//  FUNCTION:   AgentGetEngineParams                                         
//                                                                              
//  PUROSE:     Gets the SNMP Engine parameters
//                                                                             
//  PARAMETERS: pEngineId -  The engine id.  This buffer should be at least 12 bytes 
//                           to get the current implementation of engine id. Success
//                           returns the engine Id in the pEngineId buffer.
//              MaxEngineLen - The max size of the buffer pointed to by pEngineId.
//                             This routine copies the Engine Id into the suppied
//                             pEngineId buffer or upto the Max size of the buffer.
//              EngineIdLen - Success returns the number of bytes of the pEngineId.              
//              EngineBoots - Success returns the engineBoots variable. (The number of
//                            times this SNMP engine has rebooted)  
//              EngineTime - Success returns the number of seconds since this SNMP engine
//                           has rebooted.
//                                                                           
//  RETURNS:    SNMP_NO_ERROR if successful                                                               
//                                                                            
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentGetEngineParams(unsigned char * pEngineId, int MaxEngineLen, int *pEngineIdLen, int *pEngineBoots, int *pEngineTime);

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentGetUserKeys
//                      
// PURPOSE:    Gets the Auth and Priv keys for a given SNMPv3 user.  The buffers
//             authKey and privKey must be passed in and must be at least 16 bytes.
//             long.  If Auth uses SHA, the authKey buffer should be at least 20 bytes.
//
// PARAMETERS: engineId -    The engine Id associated with this user. If NULL the
//                           local engine id will we used.
//             engineIdLen - Length of the engineId. 
//             userName    - The SNMPv3 userName to find the keys for.
//             authKey     - buffer on return will contain the Auth key
//             authKeyLen  - length of the retured Auth key.
//             privKey     - buffer on return will contain the Priv key.                                 
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentGetUserKeys(unsigned char *engineId,int engineIdLen,char *userName,unsigned char *authKey,int *authKeyLen,unsigned char *privKey,int *privKeyLen);

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentGetUserNameFromMatch
//                      
// PURPOSE:    Get the user with this engine Id that includes the chars passed in.
//             This function always gets the first instance that meets the test. 
//
// PARAMETERS: engineId -    The engine Id associated with this user. If NULL the
//                           local engine id will we used.
//             engineIdLen - Length of the engineId. 
//             partName    - The string of chars that are to be found in the userName.
//             completeName- On success this buffer will hold the userName that contains
//                           passed in partName.
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentGetUserNameFromMatch(unsigned char *engineId, int engineIdLen, char *partName, char* completeName);


////////////////////////////////////////////////////////////////////////////////
//                                                                          
//  FUNCTION:    AgentInstallUserComplete                                            
//                                                                              
//  DESCRIPTION: This function installs an SNMPv3 user given the input          
//               parameters.                   
//                                                                               
//  ARGUMENTS:   pSecName:  The SNMPv3 security user name.                      
//               UserNameLen:  The length of the userName.
//               SecLevel:  SNMP_SEC_NONE (no auth no priv)                     
//                           SNMP_SEC_AUTH (auth no priv)                        
//                           SNMP_SEC_PRIV (auth and priv)
//              pEngineId:  The engine id for this user.
//                           If NULL then the local agent engine id will be used.  In
//                           general this param will be NULL unless the user is for
//                           informs.
//              EngineIdLen:The length of the engine id.                       
//              pAuthKey:   The password for authorization. NULL if not used.   
//              AuthKeyLen: Length of the pAuthKey 
//              AuthType:   The type of authorization to use.
//                           1= AUTH_MD5,  2 = AUTH_SHA                           
//              pPrivKey:   The password for privacy.  NULL if not used.        
//              PrivKeyLen: Length of the pPrivKey.         
//              Password2Key: If true, the func performs a password-to-key translation
//                            on the values given for pAuthKey and pPrivKey. This parameter
//                            is ignored if LocalizePwd is set to 1.
//                            0- do not convert passwords to keys.
//                            1- change passwords to keys.                       
//              LocalizePwd: 0- do not localize keys, 1- localize keys                                
//  RETURNS:                                                                    
//                                                                             
////////////////////////////////////////////////////////////////////////////////
int AgentInstallUserComplete (unsigned char *pSecName, unsigned int SecNameLen, int SecLevel,
                              unsigned char *pEngineId, int EngineIdLen,
                              unsigned char *pAuthKey, int AuthKeyLen, int AuthType,
                              unsigned char *pPrivKey, int PrivKeyLen,
                              int Password2Key, int LocalizePwd);  


////////////////////////////////////////////////////////////////////////////////
//                                                                          
//  FUNCTION:    ExpungeVarBind                                            
//                                                                              
//  DESCRIPTION: This function cleans a varbind of all temporarily
//         allocated memory for those who live in C land.
//                                                                               
//  ARGUMENTS:   pVB:  A pointer to the var-bind.                      
////////////////////////////////////////////////////////////////////////////////

void ExpungeVarBind (VarBind *pVB);

#if defined (__cplusplus)
}
#endif


#endif
