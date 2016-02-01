//****************************************************************************
//
// Copyright (c) 2007-2008 Broadcom Corporation
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
//    Filename: NetsnmpAgnt.c
//    Author:   T. Clack
//    Creation Date: 8-April-2003
//
//**************************************************************************
//    Description:
//
//		Interface functions for NetSnmp Library.
//		
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#include "BcmV2Glue.h"
#include <SnmpStatus.h>
#include <SnmpApi.h>

#include <stdio.h>
#include <string.h>
#include <net-snmp/agent/agent_registry.h>
#include <net-snmp/library/vacm.h>

#include "NetsnmpAgnt.h"

int trace_level = 0;    /* Execution trace level */
  
extern void snmp_root (void);
extern void register_snmpNotifyTable(void);
extern void register_snmpNotifyFilterProfileTable(void);
extern void register_snmpNotifyFilterTable(void);
extern void register_snmpTargetAddrEntry(void);
extern void register_snmpTargetParamsEntry(void);
extern void register_usmUser(void);
extern void register_usmStats(void);
extern void register_vacm_vars (void);


int gAgentPriority = 85;    //200;
//int    trace_level = 0;    /* Execution trace level */

int gRegisteredScalars = 0;
int gDuplicateScalars = 0;
int gFailedScalars = 0;
int gRegisteredTabulars = 0;
int gDuplicateTabulars = 0;
int gFailedTabulars = 0;
int gRegisteredGroups = 0;
int gDuplicateGroups = 0;
int gFailedGroups = 0;

extern int gCoreObjects; // from BcmV2Glue.c


int GetAgentPriority ()
{
    return gAgentPriority;
}


void AgentInitSnmp (int Priority)
{
   /* printf("T-LinuxAgent:AgentInitSnmp\n"); */
}

int AgentSetMaxPacketSize (int MaxSize)
{
    // No particular max packet size for Net-SNMP any more...
    return 0;
}

const char *AgentGetName ()
{
    return "Net-SNMP 5.0.9";
}

void RegisterNetSnmpObjects()
{
  register_snmpNotifyTable();
  register_snmpNotifyFilterProfileTable();
  register_snmpNotifyFilterTable();
  register_snmpTargetAddrEntry();
  register_snmpTargetParamsEntry();
  register_usmUser();
  register_usmStats();
  register_vacm_vars ();
}



void AgentStartSnmp()
{
    static int Started = 0;

    /* printf("LX- Now in AgentStartSnmp()\n"); */
    //return;

    if ( Started == 0 )
    {
        Started = 1;

        /* printf("LX-AgentStartSnmp:\n"); */

        //turn on some Net-Snmp Debugging - for now
        bcmV2_input_parameters('v');      //display version info
        // bcmV2_input_parameters('d');      //dump sent and received SNMP packets
        // bcmV2_input_parameters('V');      //verbose display
        // bcmV2_input_parameters('D');    //turn on debugging output

        // Initialize the SNMP library
        bcmV2_agent_init();
        //bcmV2_using_openssl_check();
        
        // Register the MIB objects that are embedded in the NetSnmp libs
        RegisterNetSnmpObjects();

        /* printf("\n\n NOT Calling VacmTest for debug users\n"); */
       /// AgentVacmTest();
       
//start the thread for the agent here; otherwise, use the V2 thread for the agent.
#ifdef NETSNMP_RCV_THREAD

        //call snmp_root to start the snmp thread
        /* printf("LX-AgentStartSnmp: calling snmp_root to start thread\n"); */
        snmp_root ();
#else
       /* printf("NOT STARTING THE LOCAL snmp thread!! - must be using the v2\n"); */
#endif

        // Chill for a second to let all those threads we just started spin up...
        //tm_wkafter (100);
    }
     /* printf("LX- Now in AgentStartSnmp() - finished\n"); */
}

int AgentGetDebugLevel ()
{
    return trace_level;
}

int AgentSetDebugLevel (int Level)
{
    trace_level = Level;
    
    if (Level == 9)
      snmp_set_do_debugging(1);
    else if (Level == 0)
      snmp_set_do_debugging(0);
      
    return trace_level;
}

void AgentShow ()
{
#ifndef BCM_NULL_CONSOLE
  printf ("Agent core: %s\n", AgentGetName());
  printf ("  V2/BFC debug level = %d\n", trace_level);
#if SNMP_NO_DEBUGGING
  printf ("  Agent core debugging DISABLED at compile time.\n");
#else
  if (snmp_get_do_debugging() == 1)
    printf ("  Agent core debugging ACTIVE\n");
  else
    printf ("  Agent core debugging not active\n");
#endif

  printf ("  V2/BFC scalars: %d (%d duplicates, %d failed)\n", gRegisteredScalars, gDuplicateScalars, gFailedScalars);
  printf ("  V2/BFC tables: %d (%d duplicates, %d failed)\n", gRegisteredTabulars, gDuplicateTabulars, gFailedTabulars);
  printf ("  V2/BFC groups: %d (%d duplicates, %d failed)\n", gRegisteredGroups, gDuplicateGroups, gFailedGroups);
  printf ("  Agent core objects: %d\n", gCoreObjects);
  
  // Call the glue file for any stuff he might like to show us.
  bcmV2_agent_show ();
#endif
}

// Increment some stats that get missed if the V2 agent drops a packet for
// authentication purposes.  These all return the new counter value.
static unsigned long gSnmpInPkts = 0;
unsigned long AgentIncrSnmpInPkts ()
{
    gSnmpInPkts++;
    return gSnmpInPkts;
}

static unsigned long gBadCommunityNames = 0;
unsigned long AgentIncrSnmpInBadCommunityNames ()
{
    gBadCommunityNames++;
    return gBadCommunityNames;
}

static unsigned long gBadCommunityUses = 0;
unsigned long AgentIncrSnmpInBadCommunityUses ()
{
    gBadCommunityUses++;
    return gBadCommunityUses;
}


/*******************************************************************************
// FUNCTION:   AgentGetAuthFailTrapsEnabled
//                      
// PURPOSE:     
//             
// PARAMETERS: 
//                         
// RETURNS:    
*******************************************************************************/
int AgentGetAuthFailTrapsEnabled ()
{
    return bcmV2_get_authFailTrapEnabled();
}

int AgentInformPacketAvailable (unsigned long DstIp, unsigned short DstPort, int Socket)
{
  DEBUG ("AgentInformPacketAvailable %08X:%d sock %d\n", DstIp, DstPort, Socket);
  return bcmV2_snmp_read (DstIp, DstPort, Socket);
}


SNMP_STATUS AgentProcessSnmpPacket (unsigned char *pPkt, int PktLen, int Socket,
                                    unsigned long SrcIp, unsigned long DstIp, 
                                    int SrcPort, int DstPort)
{
    // This should never be called any more.  We respond to the
    // AgentInformPacketAvailable API which should prevent this from ever
    // being called.  But we need the API just to compile.
    /* printf ("WHY is AgentProcessSnmpPacket being called for Net-SNMP??\n"); */
    
    free (pPkt);
    
    return SNMP_NO_ERROR;
}


int AgentIsObjectSupported (const ObjectId *pOid, const char *pView)
{
  int Loop;
  oid *pNetsnmpOid;
  netsnmp_subtree *pTree = netsnmp_subtree_find_first (NULL);
  
  
  // Get the root subtree for NULL context.
  if (pTree == NULL)
  {
    DEBUG ("Couldn't find root subtree!\n");
    return 0;
  }
  else
  {
    DEBUG ("Found root subtree at %p for OID ", pTree);
    for (Loop = 0; Loop < pTree->namelen; Loop++)
      DEBUG (".%d", pTree->name_a[Loop]);
    DEBUG ("\n");
  }
  
  // Cast here is just to eliminate 'const' warning from the compiler.
  // the types are the same.
  pNetsnmpOid = (oid *)pOid->pObjects;
  
  // Now search for a subtree that matches this OID
  pTree = netsnmp_subtree_find (pNetsnmpOid, pOid->Length, pTree, NULL);
  
  if (pTree == NULL)
  {
    DEBUG ("Couldn't find subtree for given OID!\n");
    return 0;
  }
  else
  {
    DEBUG ("Found subtree at %p for OID ", pTree);
    for (Loop = 0; Loop < pTree->namelen; Loop++)
      DEBUG (".%d", pTree->name_a[Loop]);
  }
  
  // OK, net-snmp by default registers "null" handlers for the root
  // OIDs which are allowed by the ASN.1: ccitt(0), iso(1), and
  // joint_ccitt_iso(2).  I am going to assume here that we will never see
  // a handler registered for any OID with less than some reasonable number
  // of elements.  For example, the system group is at .1.3.6.1.2.1.1, which
  // has 7 elements.  So lets assume that we will never have a handler for,
  // say, the whole internet group which is .1.3.6.1.  So my criteria for
  // a supported object here is that the subtree it's under must have more
  // than 4 elements in the OID.  I think that is reasonable and should
  // work unless something REALLY goofy comes along.
  if (pTree->namelen < 5)
  {
    DEBUG ("Subtree has only %d elements, not supported!\n", pTree->namelen);
    return 0;
  }
  
  DEBUG ("Subtree has reasonably large number of elements, supported!\n");
  
  // Check to see if the object is in view.  Unless no view is specified.
  if (pView)
  {
    if (vacm_checkSubtree (pView, pTree->name_a, pTree->namelen) == VACM_NOTINVIEW)
    {
      DEBUG ("Subtree is not in view, not supported!\n");
      return 0;
    }
  }
  
  DEBUG ("Subtree is in view, supported!\n");
  return 1;
}

int IsHandlerRegistered (const unsigned long *pOid, unsigned long Length)
{
  int Loop;
  // Cast here is just to eliminate 'const' warning from the compiler.
  // the types are the same.
  oid *pNetsnmpOid = (oid *)pOid;
  
  // Get the root subtree for NULL context to start our search.
  netsnmp_subtree *pTree = netsnmp_subtree_find_first (NULL);
  
  // Now search for a subtree that EXACTLY matches this OID
  pTree = netsnmp_subtree_find (pNetsnmpOid, Length, pTree, NULL);
  while (pTree)
  {
    DEBUG ("Found subtree at %p for OID ", pTree);
    
    for (Loop = 0; Loop < pTree->namelen; Loop++)
      DEBUG (".%d", pTree->name_a[Loop]);
      
    if (pTree->namelen == Length)
      break;
      
    pTree = netsnmp_subtree_find_next (pNetsnmpOid, Length, pTree, NULL);
  }
  
  if (pTree)
  {
    DEBUG ("Match!\n");
    return 1;
  }
  else
  {
    DEBUG ("No match!\n");
    return 0;
  }
}

SNMP_STATUS RegisterHandler (const unsigned long *pOid, 
                             unsigned long Length, 
                             const char *pName, 
                             Netsnmp_Node_Handler *handler_access_method)
{
    oid *netsnmp_oid;
    netsnmp_handler_registration *my_handler;
    int Loop;
  
    // First, see if somebody already registered this handler.  That is
    // entirely possible (likely, even) in a multi-agent implementation.
    // If it's already registered, no point in doing it again and wasting
    // memory.  Return NO_ERROR if already registered.
    if (IsHandlerRegistered (pOid, Length))
        return SNMP_RESOURCE_UNAVAILABLE;

    // Can we get away with using the OID directly instead of copying?
    // Saves quite a bit of RAM that way...
    // Seems that we can, although RAM savings are not as substantial as I
    // had thought, only on the order of 22k for DOCSIS CM...
    // Cast here is just to eliminate 'const' warning from the compiler.
    // the types are the same.
    netsnmp_oid = (oid *)pOid;

    my_handler =
        netsnmp_create_handler_registration(pName,
                                            handler_access_method,
                                            netsnmp_oid,
                                            Length,
                                            HANDLER_CAN_RWRITE);

    if (my_handler)  
    {
        // This might fail, and that's OK.  It just means that somebody
        // else has already registered a handler for this OID.
        if (netsnmp_register_handler(my_handler)) 
        {
#ifndef BCM_NULL_CONSOLE
            printf ("Failed to register handler for %s (", pName);
            for (Loop = 0; Loop < Length; Loop++)
              printf (".%ld", pOid[Loop]);
            printf (")\n");
#endif
                        
            /* One would like to free my_handler here via 
               netsnmp_handler_registration_free, but there is apparently
               a bug somewhere because when we do this, all hell breaks loose.
               So live with the memory leak, it's a one time thing anyway. */
            
            return SNMP_GEN_ERR;
        }
    } 
    else 
    {
        /* printf ("Failed to create handler for %s\n", pName); */
        return SNMP_GEN_ERR;
    }
    DEBUGMSGTL(("AgentRegisterScalar",
                "Handler created and registered.\n"));
                
    DEBUG ("Registered handler for %s (", pName);
    for (Loop = 0; Loop < Length; Loop++)
      DEBUG (".%ld", pOid[Loop]);
    DEBUG (")\n");
   
    return SNMP_NO_ERROR;                                      
}

SNMP_STATUS AgentRegisterScalar (const ObjectId *pOid, const char *pName)
{
    SNMP_STATUS Status;
    unsigned long Length;
    
    // If the OID has the .0 instance attached, register with length-1.  We
    // must not register just for the .0 instance of the scalar, or else we
    // will muck up noSuchObject vs. noSuchInstance.
    if (pOid->pObjects[pOid->Length-1] == 0)
      Length = pOid->Length-1;
    else
      Length = pOid->Length;
    
    Status = RegisterHandler (pOid->pObjects, Length, pName, bcmV2_do_scalar);
    
    // Handler registered OK
    if (Status == SNMP_NO_ERROR)
      gRegisteredScalars++;
    // Handler wasn't registered because a handler was already registered
    // for this OID.  Not an error condition, probably due to multiple
    // agents.
    else if (Status == SNMP_RESOURCE_UNAVAILABLE)
    {
      gDuplicateScalars++;
      Status = SNMP_NO_ERROR;
    }
    // Handler registration failed for some reason.
    else
      gFailedScalars++;
      
    return Status;
}


SNMP_STATUS AgentRegisterGroup (const ObjectId *pOid, const char *pName)
{
    SNMP_STATUS Status = RegisterHandler (pOid->pObjects, pOid->Length, pName, bcmV2_do_group);
    
    // Handler registered OK
    if (Status == SNMP_NO_ERROR)
      gRegisteredGroups++;
    // Handler wasn't registered because a handler was already registered
    // for this OID.  Not an error condition, probably due to multiple
    // agents.
    else if (Status == SNMP_RESOURCE_UNAVAILABLE)
    {
      gDuplicateGroups++;
      Status = SNMP_NO_ERROR;
    }
    // Handler registration failed for some reason.
    else
      gFailedGroups++;
      
    return Status;
}


SNMP_STATUS AgentRegisterTabular (const ObjectId *pOid, const char *pName)
{
    SNMP_STATUS Status = RegisterHandler (pOid->pObjects, pOid->Length, pName, bcmV2_do_tabular);
    
    // Handler registered OK
    if (Status == SNMP_NO_ERROR)
      gRegisteredTabulars++;
    // Handler wasn't registered because a handler was already registered
    // for this OID.  Not an error condition, probably due to multiple
    // agents.
    else if (Status == SNMP_RESOURCE_UNAVAILABLE)
    {
      gDuplicateTabulars++;
      Status = SNMP_NO_ERROR;
    }
    // Handler registration failed for some reason.
    else
      gFailedTabulars++;
      
    return Status;
}
