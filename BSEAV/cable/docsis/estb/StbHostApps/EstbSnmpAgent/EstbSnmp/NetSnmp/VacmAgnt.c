//**************************************************************************
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
//
//
//**************************************************************************
//    Filename: VacmAgnt.c
//    Author:   Thomas Clack
//    Creation Date: 15-April-03
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Agent API functions for configuring SNMPv3 VACM.
//    VacmAgnt ported for use with NetSnmp.
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "SnmpType.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>

#include <net-snmp/library/tools.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/vacm.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/library/snmp_secmod.h>
#include <net-snmp/library/snmpusm.h>
#include <net-snmp/library/transform_oids.h>
#include <net-snmp/library/keytools.h>
#include <net-snmp/library/scapi.h>

#include "BcmV2Glue.h"
#include "AgentDefs.h"
#include "AgentUtils.h"

 
//This is a place holder for our engine Id. 
unsigned char MY_ENGINE_ID[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int LEN_ENGINE_ID = sizeof(MY_ENGINE_ID); 

extern struct vacm_viewEntry *viewList; // from vacm.c
extern struct usmUser *userList;extern struct usmUser *userList;

extern unsigned long AppGetEngineBoots();
#ifdef NOTDEF
extern char* strdup (char * str);
#endif
extern int bcmV2_update_engine_times();


/////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
void AgentVacmTest();  //local debug function
SNMP_STATUS AgentInstallTestManager (char *pVacmUserName);
SNMP_STATUS AgentGetUserKeys(unsigned char *engineId, int engineIdLen, char *userName,
                             unsigned char *authKey, int *authKeyLen,
                             unsigned char *privKey, int *privKeyLen);
////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentGetEngineId
//                      
// PURPOSE:    Gets our Agent's SNMPv3 EngineId.
//             NOTE that this returns the global which was set in the NetSnmp
//             It is not asking NetSnmp for what it thinks is the EngineId.
//
// PARAMETERS: pId:      A ptr to a buffer where the EngineId will be copied.
//             MaxLen:   The length of pId buffer. 
//             
// RETURNS:    int: Number of bytes copied into pId
////////////////////////////////////////////////////////////////////////////////
int AgentGetEngineId (unsigned char *pId, int MaxLen)
{
#if (SNMP_SUPPORT == 3)
    int CopyLen;

    if ( MaxLen < LEN_ENGINE_ID )
        CopyLen = MaxLen;
    else
        CopyLen = LEN_ENGINE_ID;

    memcpy (pId, MY_ENGINE_ID, CopyLen);

    return LEN_ENGINE_ID;
#else
    printf ("AgentGetEngineId stub for SNMPv1/v2c, doing nothing.\n");
    return 0;
#endif
}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentUpdateEngineTimes
//                      
// PURPOSE:    Sets our Agent's SNMPv3 EngineId boots and time values.
//             
// PARAMETERS: 
//             
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentUpdateEngineTimes ()
{
    return bcmV2_update_engine_times();
}

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
SNMP_STATUS AgentSetEngineId (unsigned char *pId, int Length)
{
    unsigned char NullEngineId[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#if (SNMP_SUPPORT == 3)

    if ( pId == NULL )
    {
        printf("AgentSetEngineId: There was a NULL ptr.\n");
        return SNMP_WRONG_VALUE;
    }
    if ( Length > MAX_ENGINE_ID_LEN )
        return SNMP_WRONG_VALUE;

    if ( Length >= SNMP_MAXBUF_SMALL )
    {
        printf("AgentSetEngineId: Engine ID too long, not set.\n");
        return SNMP_WRONG_VALUE;
    }

    DumpBufTrace("AgentSetEngineId: Setting Engine ID ==",pId,Length, 'x',4);

    // Only allow this to be set one time.
    if ( memcmp(MY_ENGINE_ID, NullEngineId, sizeof(NullEngineId)) != 0 )
        return SNMP_RESOURCE_UNAVAILABLE;

    memcpy (MY_ENGINE_ID, pId, Length);
    LEN_ENGINE_ID = Length;


    return(bcmV2_set_engineID(pId, (size_t) Length));

    //TODO
    //What about setting engine boots? - we used to do it here 
    

#else
    printf ("AgentSetEngineId stub for SNMPv1/v2c, doing nothing.\n");
#endif


    return SNMP_NO_ERROR;
}

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
SNMP_STATUS AgentGetEngineParams(unsigned char * pEngineId, int MaxEngineLen, int *pEngineIdLen, int *pEngineBoots, int *pEngineTime)
{
#if (SNMP_SUPPORT == 3)
    *pEngineIdLen = AgentGetEngineId (pEngineId, MaxEngineLen);
    *pEngineBoots = snmpv3_local_snmpEngineBoots();
    *pEngineTime  = snmpv3_local_snmpEngineTime();
    return SNMP_NO_ERROR;
#else
    printf ("AgentGetEngineParams stub for SNMPv1/v2c, doing nothing.\n");
    return SNMP_NO_ERROR;
#endif
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentGetContext
//                      
// PURPOSE:    Gets the agents's SNMPv3 Context.
//             This is actually stored here and not in the NetSnmp.
//
// PARAMETERS: pContext: A ptr to a buffer where this function will put the Context.
//             MaxLen:   The maximum length of the buffer passed in.
//             
// RETURNS:    Length of context copied to the pContext buffer.
////////////////////////////////////////////////////////////////////////////////
#define MAX_CONTEXT_LEN 64
char pMyContext[MAX_CONTEXT_LEN] = "";

// Note: this is kind of a cheesy way of doing this, but there's no
// easy way to extract the context from Epilogue...
int AgentGetContext (char *pContext, int MaxLen)
{
    int CopyLen;

    if ( MaxLen > MAX_CONTEXT_LEN )
        CopyLen = MAX_CONTEXT_LEN - 1;
    else
        CopyLen = MaxLen - 1;

    memset (pContext, CopyLen, MaxLen);
    strncpy (pContext, pMyContext, CopyLen);

    return strlen (pMyContext);
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentSetContext
//                      
// PURPOSE:    Sets the agents's SNMPv3 Context.
//             This is actually stored here and not in the NetSnmp.
//
// PARAMETERS: pContext: A ptr to a buffer containing the context to set.
//             
// RETURNS:    SNMP_NO_ERROR on success.
//
// NOTE: This is not really setting in the lib. !!!
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentSetContext (char *pContext)
{
    memset (pMyContext, 0, MAX_CONTEXT_LEN);
    strncpy (pMyContext, pContext, MAX_CONTEXT_LEN-1);

    if ( !bcmV2_set_context((unsigned char*)pMyContext, strlen(pMyContext)) )
        return SNMP_GEN_ERR;

    return SNMP_NO_ERROR;
}

SNMP_STATUS AgentDestroyContext (char *pContext)
{
    return SNMP_NO_ERROR;
}

void DebugOid (unsigned long *pSubtree, int SubtreeSize)
{
  int Loop;
  
  for (Loop = 0; Loop < SubtreeSize; Loop++)
    DEBUG (".%d", pSubtree[Loop]);
}

struct vacm_viewEntry *FindViewEntry (char *pViewName, unsigned long *pSubtree, int SubtreeSize)
{
  struct vacm_viewEntry *pView;
  int nameLen = strlen (pViewName);
  
  
  DEBUG ("FindViewEntry %s ", pViewName);
  DebugOid (pSubtree, SubtreeSize);
  DEBUG ("\n");
  
  // Iterate over all view entries and return one which matches the subtree
  // EXACTLY.
  for (pView = viewList; pView; pView = pView->next)
  {
    DEBUG ("  %s, ", pView->viewName+1);
    DebugOid (pView->viewSubtree + 1, pView->viewSubtreeLen - 1);
    DEBUG ("\n");
  
    // NOTE: you gotta look at pView->viewName+1 because the first character
    // in the name is the length.  Ditto with the OID, plus you gotta take one
    // off the length to compensate for the extra element up front.
    if ((memcmp (pViewName, pView->viewName+1, nameLen) == 0)
    &&  (netsnmp_oid_equals (pSubtree, SubtreeSize, pView->viewSubtree + 1, pView->viewSubtreeLen - 1) == 0))
    {
      DEBUG ("  MATCH!\n");
      return pView;
    }
  }
  
  return NULL;
}

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
SNMP_STATUS AgentConfigView (char *pViewName, unsigned long *pSubtree, int SubtreeSize, SNMP_VIEW_STATUS Status)
{
    struct vacm_viewEntry *viewEntryPtr;
    int Loop = 0;

    // DEBUG
    // trace_level = 2;

    if (trace_level)
    {
        printf ("AgentConfigView %s ", pViewName);
        for ( Loop = 0; Loop < SubtreeSize; Loop++ )
            printf (".%d", (int)pSubtree[Loop]);
        if ( Status == VIEW_STATUS_EXCLUDE )
            printf (": EXCLUDE\n");
        else
            printf (": INCLUDE\n");
    }

    if ((Status != VIEW_STATUS_EXCLUDE)
    &&  (Status != VIEW_STATUS_INCLUDE))
    {
        if (trace_level)
            printf ("  Invalid status %d\n", (int)Status);
        return SNMP_WRONG_VALUE;
    }


    // See if we can find the view already existing first.
    // We can't use vacm_getViewEntry because it will return a match on any
    // view of which the desired OID is a subtree.  We must use the
    // FindViewEntry helper function above.
    // viewEntryPtr =  vacm_getViewEntry(pViewName, pSubtree, SubtreeSize, VACM_MODE_FIND);
    viewEntryPtr = FindViewEntry (pViewName, pSubtree, SubtreeSize);
    
    // If not, create it.
    if (viewEntryPtr == NULL)
    {
      DEBUG ("  installing view...\n");

      viewEntryPtr = vacm_createViewEntry(pViewName, pSubtree, SubtreeSize);

      if ( viewEntryPtr == NULL )
      {
          printf("AgentConfigView COULD NOT create view!\n");
          return SNMP_RESOURCE_UNAVAILABLE;
      }

      DEBUG("  created view @ %p\n", viewEntryPtr);
    }
    else
      DEBUG ("  found view @ %p\n", viewEntryPtr);
      
    // V2 and netsnmp use different view constants.  Gotta map 'em:
    if (Status == VIEW_STATUS_EXCLUDE) // V2 constant
      viewEntryPtr->viewType = SNMP_VIEW_EXCLUDED; // netsnmp constant
    else 
      viewEntryPtr->viewType = SNMP_VIEW_INCLUDED;

    //per OSS-N-02063 storageType must be 'volatile' for all but our DOCSIS defined users
    if ( !strcmp(pViewName, "@config") )
        viewEntryPtr->viewStorageType = SNMP_STORAGE_VOLATILE;
    else
        viewEntryPtr->viewStorageType = SNMP_STORAGE_PERMANENT;
        
    // Set the mask length to 0.  Otherwise the view restrictions won't work properly.
    memset (viewEntryPtr->viewMask, 0xFF, sizeof (viewEntryPtr->viewMask));
    viewEntryPtr->viewMaskLen = 0;

    viewEntryPtr->viewStatus = SNMP_ROW_ACTIVE;

    DEBUG ("  exit.\n");
    
    // DEBUG
    // trace_level = 0;

    return SNMP_NO_ERROR;

}


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
SNMP_STATUS AgentDestroyView (char *pViewName, unsigned long *pSubtree, int SubtreeLen)
{
#if (SNMP_SUPPORT == 3)

    if ( trace_level > 0 )
        printf ("AgentDestroyView %s\n", pViewName);

    vacm_destroyViewEntry(pViewName,
                          (oid *) pSubtree, 
                          SubtreeLen);
#else
    printf ("AgentDestroyView stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentCleanupViews
//                      
// PURPOSE:    This function is called to cleanup ALL views. 
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentCleanupViews ()
{
#if (SNMP_SUPPORT == 3)

    if ( trace_level > 0 )
        printf ("AgentCleanupViews\n");

    bcmV2_destroy_all_views(); 


#else
    printf ("AgentCleanupViews stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

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
SNMP_STATUS AgentInstallGroup (char *pGroupName, char *pSecurityName, SNMP_SEC_MODEL SecurityModel)
{
#if (SNMP_SUPPORT == 3)

    int GroupNameLen;
    struct vacm_groupEntry *groupEntryPtr;

    if ( trace_level > 0 )
        printf ("AgentInstallGroup for group name %s\n", pGroupName);


    //TODO
    //if ( SNMP_Group_Lookup (3, (unsigned char *)pGroupName, strlen(pGroupName)) != NULL )
    //{
    //    if ( trace_level > 0 )
    //        printf ("  Group already exists, doing nothing.\n");
    //
    //    return SNMP_NO_ERROR;
    //}

    groupEntryPtr = vacm_createGroupEntry(SecurityModel, pSecurityName);
    if ( groupEntryPtr == NULL )
    {
        return SNMP_GEN_ERR;
    }

    GroupNameLen = strlen(pGroupName);
    groupEntryPtr->groupName[0] = GroupNameLen;
    //memcpy(groupEntryPtr->groupName+1, pGroupName, GroupNameLen + 1);
    strcpy(groupEntryPtr->groupName, pGroupName);

    //per OSS-N-02063 storageType must be 'volatile' for all but our DOCSIS defined users
    if ( !strcmp(pGroupName, "docsisManager"    )
         || !strcmp(pGroupName, "docsisOperator")
         || !strcmp (pGroupName,"docsisMonitor")
         || !strcmp (pGroupName, "docsisUser"   )
         || !strcmp (pGroupName, "dhKickstart"  ) 
         || !strcmp (pGroupName, "CHAdministrator"  ) )

        groupEntryPtr->storageType = SNMP_STORAGE_PERMANENT;
    else
        groupEntryPtr->storageType = SNMP_STORAGE_VOLATILE;

    groupEntryPtr->status = SNMP_ROW_ACTIVE;

#else
    printf ("AgentInstallGroup stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentDestroyGroup
//                      
// PURPOSE:    Removes an SNMPv3 group and cleans up any allocated memory.
//
// PARAMETERS: char *pGroupName:         The name of the group to remove.
//  
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentDestroyGroup (char *pGroupName)
{
#if (SNMP_SUPPORT == 3)

    if ( trace_level > 0 )
        printf ("AgentDestroyGroup for group name %s\n", pGroupName);

    vacm_destroyGroupEntry(3, //USM_SEC_MODEL_NUMBER
                           pGroupName);

#else
    printf ("AgentDestroyGroup stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentCleanupGroups
//                      
// PURPOSE:    Removes ALL SNMPv3 groups and cleans up any allocated memory.
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentCleanupGroups ()
{
#if (SNMP_SUPPORT == 3)

    if ( trace_level > 0 )
        printf ("AgentCleanupGroups\n");

    //vacm_destroyAllGroupEntries();
    bcmV2_destroy_all_groups();

#else
    printf ("AgentCleanupGroups stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

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
SNMP_STATUS AgentInstallAccess (char *pGroupName, char *pReadView, char *pWriteView, char *pNotifyView, SNMP_SEC_LEVEL Security, SNMP_SEC_MODEL SecModel)
{
#if (SNMP_SUPPORT == 3)
    struct vacm_accessEntry *accessEntryPtr;

    accessEntryPtr = vacm_createAccessEntry(pGroupName, "", SecModel, Security);
    if ( accessEntryPtr == NULL )
        return SNMP_GEN_ERR;

    accessEntryPtr->contextMatch = 1;

    //per OSS-N-02063 storageType must be 'volatile' for all but our DOCSIS defined users
    if ( !strcmp(pGroupName, "docsisManager"    )
         || !strcmp(pGroupName, "docsisOperator")
         || !strcmp (pGroupName,"docsisMonitor")
         || !strcmp (pGroupName, "docsisUser"   )
         || !strcmp (pGroupName, "dhKickstart"  ) 
         || !strcmp (pGroupName, "CHAdministrator"  ) )
        accessEntryPtr->storageType = SNMP_STORAGE_PERMANENT;
    else
        accessEntryPtr->storageType = SNMP_STORAGE_VOLATILE;

    accessEntryPtr->contextPrefix[0] = strlen(pMyContext);
    memcpy(accessEntryPtr->contextPrefix+1, pMyContext, strlen(pMyContext) + 1);
    /*
    accessEntryPtr->readView[0] = strlen(pReadView);
    memcpy(accessEntryPtr->readView+1, pReadView, strlen(pReadView) + 1);

    accessEntryPtr->writeView[0] = strlen(pWriteView);
    memcpy(accessEntryPtr->writeView+1, pWriteView, strlen(pWriteView) + 1);

    accessEntryPtr->notifyView[0] = strlen(pNotifyView);
    memcpy(accessEntryPtr->notifyView+1, pNotifyView, strlen(pNotifyView) + 1);
    */

    strcpy(accessEntryPtr->readView, pReadView);
    strcpy(accessEntryPtr->writeView, pWriteView);
    strcpy(accessEntryPtr->notifyView, pNotifyView);

    accessEntryPtr->status = SNMP_ROW_ACTIVE;

#else
    printf ("AgentInstallAccess stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

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
SNMP_STATUS AgentDestroyAccess (char *pGroupName, SNMP_SEC_LEVEL Security, SNMP_SEC_MODEL SecModel)
{
#if (SNMP_SUPPORT == 3)

    if ( trace_level > 0 )
        printf ("AgentDestroyAccess for group name %s\n", pGroupName);

    vacm_destroyAccessEntry(pGroupName, NULL, SecModel, Security);

#else
    printf ("AgentDestroyAccess stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentCleanupAccess
//                      
// PURPOSE:    Removes ALL SNMPv3 accesses and any allocated memory.
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentCleanupAccess ()
{
#if (SNMP_SUPPORT == 3)

    if ( trace_level > 0 )
        printf ("AgentCleanupAccesses\n");

    //vacm_destroyAllAccessEntries();
    bcmV2_destroy_all_accessEntries();

#else
    printf ("AgentCleanupAccess stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

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
SNMP_STATUS AgentInstallUser (char *pUserName, SNMP_SEC_LEVEL Security)
{
#if (SNMP_SUPPORT == 3)

    struct usmUser * user;

    if ( !bcmV2_install_user((unsigned char*)pUserName, Security) )
        return SNMP_GEN_ERR;

    user = usm_get_user(MY_ENGINE_ID, LEN_ENGINE_ID, pUserName);
    if ( user == NULL )
        return SNMP_GEN_ERR;

    //per OSS-N-02063 storageType must be 'volatile' for all but our DOCSIS defined users
    if ( !strcmp(pUserName, "docsisManager"    )
         || !strcmp(pUserName, "docsisOperator")
         || !strcmp (pUserName,"docsisMonitor")
         || !strcmp (pUserName, "docsisUser"   )
         || !strcmp (pUserName, "dhKickstart"  ) 
         || !strcmp (pUserName, "CHAdministrator"  ) )
        user->userStorageType = ST_PERMANENT;
    else
        user->userStorageType = ST_VOLATILE;

#else
    printf ("AgentInstallUser stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}                

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentEnableUser
//                      
// PURPOSE:    Enables an SNMPv3 user by setting it's status to Active   
//
// PARAMETERS: pUserName:  The SNMPv3 security user name.                                 
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentEnableUser (char *pUserName)
{
#if (SNMP_SUPPORT == 3)
    if ( trace_level > 0 )
        printf ("AgentEnableUser %s\n", pUserName);


    if ( !bcmV2_enable_user(MY_ENGINE_ID, LEN_ENGINE_ID, (unsigned char*) pUserName) )
        return SNMP_GEN_ERR;

#else
    printf ("AgentEnableUser stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentDisableUser
//                      
// PURPOSE:    Disables an SNMPv3 user by setting it's status to NotInService   
//
// PARAMETERS: pUserName:  The SNMPv3 security user name.                                 
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentDisableUser (char *pUserName)
{
#if (SNMP_SUPPORT == 3)

    if ( trace_level > 0 )
        printf ("AgentDisableUser %s\n", pUserName);

    if ( !bcmV2_disable_user(MY_ENGINE_ID, LEN_ENGINE_ID, (unsigned char*)pUserName) )
        return SNMP_GEN_ERR;

#else
    printf ("AgentDisableUser stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentDestroyUser
//                      
// PURPOSE:    Removes an SNMPv3 user and any allocated memory.   
//
// PARAMETERS: pUserName:  The SNMPv3 security user name.                                 
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentDestroyUser (char *pUserName)
{
#if (SNMP_SUPPORT == 3)

    if ( trace_level > 0 )
        printf ("AgentDisableUser %s\n", pUserName);

    if ( !bcmV2_destroy_user(MY_ENGINE_ID, LEN_ENGINE_ID, (unsigned char*)pUserName) )
        return SNMP_GEN_ERR;

#else
    printf ("AgentDestroyUser stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentCleanupUsers
//                      
// PURPOSE:    Removes ALL SNMPv3 users and allocated memory.   
//
// PARAMETERS: pUserName:  The SNMPv3 security user name.                                 
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentCleanupUsers ()
{
#if (SNMP_SUPPORT == 3)

    if ( trace_level > 0 )
        printf ("AgentCleanupUsers\n");

    bcmV2_destroy_all_users(MY_ENGINE_ID, LEN_ENGINE_ID);

#else
    printf ("AgentCleanupUsers stub for SNMPv1/v2c, doing nothing.\n");
#endif

    return SNMP_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentDumpV3Keys
//                      
// PURPOSE:    LOCAL debug function for dumping the V3 user keys   
//
// PARAMETERS:                          
//
// RETURNS:    
////////////////////////////////////////////////////////////////////////////////

void AgentDumpV3Keys(char *userName)
{
    
    unsigned char authKey[32], privKey[32];
    int authKeyLen, privKeyLen;
    int i;
    //char completeName[64];
   
    printf("\nAgentDumpV3Keys:\n");
    printf("  user name = <%s>\n",userName); 
    
    // found the userName - get the keys
    if (SNMP_NO_ERROR == AgentGetUserKeys((unsigned char *)MY_ENGINE_ID,
                           LEN_ENGINE_ID, userName,
                           (unsigned char *)authKey, &authKeyLen,
                           (unsigned char *)privKey, &privKeyLen))
    {
        printf("\n mg authKey = ");
        if (authKey != NULL)
        {  
            for ( i=0; i<authKeyLen; i++ )
                printf("0x%02x ",authKey[i]);

            ////////////////////////////////
            printf("\n sc authKey = 0x");
            for ( i=0; i<authKeyLen; i++ )
            {
                printf("%02x",authKey[i]);
                if (i<15)
                    printf(":");
            }// SilverCreek format
            ////////////////////////////////
                
        }
        if (privKey != NULL)
        {  
            printf("\n    privKey = ");
            for ( i=0; i<privKeyLen; i++ )
                 printf("0x%02x ",privKey[i]);
                 printf("\n\n");
        }
    }
    else
        printf("Problem getting keys\n");

    return;
}


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
                              int Password2Key, int LocalizePwd)  
{
    struct usmUser *newUser;
    u_char          userKey[SNMP_MAXBUF_SMALL];
    size_t          userKeyLen = SNMP_MAXBUF_SMALL;
    size_t          ret;
    char            secNameBuf[64];

    //if ( trace_level > 1 )
    {
        printf("AgentInstallUserComplete: Installing SNMPv3 user = <%s>  SecLevel = %d \n", pSecName,SecLevel);
        printf(" \t\t\t\t\t\tAuthType = %d, AuthKeyLen = %d, PrivKeyLen = %d\n",AuthType, AuthKeyLen, PrivKeyLen);
    }

    newUser = usm_create_user();
    if ( newUser == NULL )
        return 0;

    //make sure our pSecName is null terminated
    memset(secNameBuf,'\0', sizeof(secNameBuf));
    strncpy(secNameBuf,(char*)pSecName,SecNameLen);

    //Set Security Name
    if ( (newUser->name = strdup(secNameBuf)) == NULL )
    {
        usm_free_user(newUser);
        return 0;
    }

    if ( (newUser->secName = strdup(secNameBuf)) == NULL )
    {
        usm_free_user(newUser);
        return 0;
    }

    //Set Engine ID
    //if we were not passed an engine, then use the local engine    
    if ( (pEngineId == NULL) || (EngineIdLen == 0) )
    {
        EngineIdLen = LEN_ENGINE_ID;  //snmpv3_get_engineID(myId, USM_MAX_ID_LENGTH);
        pEngineId = &MY_ENGINE_ID[0]; //&myId[0];
    }

    newUser->engineID = (u_char *) malloc(EngineIdLen);
    memcpy(newUser->engineID, pEngineId, EngineIdLen);
    newUser->engineIDLen = EngineIdLen;

    //set the auth key if needed
    if ( (SecLevel == SNMP_SEC_AUTH) || (SecLevel == SNMP_SEC_PRIV) )
    {
        if ( (pAuthKey != NULL) && (AuthKeyLen > 0) )
        {
            //we are doing auth!
            //bet i need to malloc protocols space this is debug function but need to fix it
            //set the auth type
            if ( AuthType == AUTH_MD5 )
                memcpy(newUser->authProtocol, usmHMACMD5AuthProtocol,sizeof(usmHMACMD5AuthProtocol));
            else if ( AuthType == AUTH_SHA )
                memcpy(newUser->authProtocol, usmHMACSHA1AuthProtocol,sizeof(usmHMACSHA1AuthProtocol));
            else
            {
                // if ( trace_level > 1 )
                printf("AgentInstallUserComplete: Unknown Auth type.");
                usm_free_user(newUser);
                return SNMP_BAD_VALUE;
            }

            if ( ( Password2Key == 1 ) && ( LocalizePwd == 0 ) )
            {
                ret = generate_Ku(newUser->authProtocol, newUser->authProtocolLen,
                                  pAuthKey, AuthKeyLen, userKey, &userKeyLen);
                if ( ret != SNMPERR_SUCCESS )
                {
                    //  if ( trace_level > 1 )
                    printf("AgentInstallUserComplete: could not generate the authentication key from the suppiled pass phrase.\n");
                    usm_free_user(newUser);
                    return SNMP_BAD_VALUE;
                }
            }
            else
            {
                //not converting password to a key
                memcpy(userKey,pAuthKey,AuthKeyLen);
                userKeyLen = AuthKeyLen;
            }

            //now we have the password or key in userKey.
            //it may or may not be converted to a key

            newUser->authKeyLen =sc_get_properlength(newUser->authProtocol,newUser->authProtocolLen);
            newUser->authKey = (u_char *) malloc(newUser->authKeyLen);

            if ( LocalizePwd == 1 )
            {
                ret = generate_kul(newUser->authProtocol, newUser->authProtocolLen,
                                   newUser->engineID, newUser->engineIDLen,
                                   userKey, userKeyLen,
                                   newUser->authKey, &newUser->authKeyLen);
                if ( ret != SNMPERR_SUCCESS )
                {
                    printf("AgentInstallUserComplete: could not generate localized authentication key (Kul) from the master key (Ku).");
                    usm_free_user(newUser);
                    return SNMP_BAD_VALUE;
                }
            }
            else
            {
                //Not localizing password
                memcpy(newUser->authKey,userKey,userKeyLen);
            }
        }
    }// if (SecLevel == ...  for setting Auth key

    //PRIVACY Section

    //set the privacy key
    if ( (pPrivKey != NULL) && (PrivKeyLen > 0) && (SecLevel == SNMP_SEC_PRIV) )
    {
        //we are doing priv!

        userKeyLen = SNMP_MAXBUF_SMALL;
        //set the priv type
        memcpy(newUser->privProtocol, usmDESPrivProtocol,sizeof(usmDESPrivProtocol));

        if ( ( Password2Key == 1 ) && ( LocalizePwd == 0 ) )
        {
            ret = generate_Ku(newUser->authProtocol, newUser->authProtocolLen,
                              pPrivKey, PrivKeyLen, userKey, &userKeyLen);
            if ( ret != SNMPERR_SUCCESS )
            {
                //if ( trace_level > 1 )
                printf("AgentInstallUserComplete: could not generate the privacy key from the suppiled pass phrase.");
                usm_free_user(newUser);
                return SNMP_BAD_VALUE;
            }
        }
        else
        {
            //not converting password to a key
            memcpy(userKey,pPrivKey,PrivKeyLen);
            userKeyLen = PrivKeyLen;
        }

        //now we have the password or key in userKey.
        //it may or may not be converted to a key

        //newUser->privKeyLen =sc_get_properlength(newUser->privProtocol,newUser->privProtocolLen);
        newUser->privKeyLen = 16;
        newUser->privKey = (u_char *) malloc(newUser->privKeyLen);

        if ( LocalizePwd == 1 )
        {
            //note: we do want the AUTH transform used by this user here
            ret = generate_kul(newUser->authProtocol, newUser->authProtocolLen,
                               newUser->engineID, newUser->engineIDLen,
                               userKey, userKeyLen,
                               newUser->privKey, &newUser->privKeyLen);
            if ( ret != SNMPERR_SUCCESS )
            {
                printf("AgentInstallUserComplete: could not generate localized privacy key (Kul) from the master key (Ku).");
                usm_free_user(newUser);
                return SNMP_BAD_VALUE;
            }
        }
        else
        {
            //Not localizing password
            memcpy(newUser->privKey,userKey,userKeyLen);
        }
    }

    //newUser->userStatus = RS_NOTINSERVICE;
    newUser->userStatus = RS_ACTIVE;
    newUser->userStorageType = ST_VOLATILE;

    usm_add_user(newUser);

    //  if ( trace_level > 4 )
    printf("AgentInstallUserComplete: created a new user <%s>\n", newUser->secName);
    AgentDumpV3Keys(newUser->secName);
    //#endif // if 0

    return SNMP_NO_ERROR;
}

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
SNMP_STATUS AgentGetUserKeys(unsigned char *engineId, int engineIdLen, char *userName,
                             unsigned char *authKey, int *authKeyLen,
                             unsigned char *privKey, int *privKeyLen)
{
    struct usmUser *newUser;

    if (engineId == NULL)
       newUser = usm_get_user(MY_ENGINE_ID, LEN_ENGINE_ID, (char *)userName);
    else
       newUser = usm_get_user(engineId, engineIdLen, (char *)userName);
    
    if ( newUser == NULL )
    {
        DEBUGMSGTL(("AgentGetUserKeys", "couldn't find user %s\n", userName));
        return SNMP_GEN_ERR;
    }
     
    *authKeyLen = newUser->authKeyLen;
    memcpy(authKey,newUser->authKey, newUser->authKeyLen);

    *privKeyLen = newUser->privKeyLen;
    memcpy(privKey,newUser->privKey, newUser->privKeyLen);
    return SNMP_NO_ERROR;
}

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
SNMP_STATUS AgentGetUserNameFromMatch(unsigned char *engineId, int engineIdLen, char *partName, char* completeName)
{
    struct usmUser *newUser;
    
    for ( newUser = userList; newUser != NULL; newUser = newUser->next )
    {
       DEBUG("AgentGetUserNameFromMatch:  Compare user <%s> for <%s>\n",newUser->name, partName);
        
       if ( (newUser->engineIDLen == engineIdLen) &&
             (newUser->engineID != NULL) && 
             (engineId != NULL) &&
             (memcmp(newUser->engineID, engineId, engineIdLen) == 0) &&
             (memcmp(newUser->name, partName, strlen(partName)) == 0) )
        {
            /* found first user name that contains partName */
            strcpy(completeName, newUser->name);
            return SNMP_NO_ERROR;
        }
    }
    /* we did not find it */
    return SNMP_GEN_ERR;
}







/************************************************************************************
// API for adding DOCSIS 1.1 defined VACM views (from SP-OSSIv1.1-I02, section 2.2.4):
// pViewName could be "docsisManager", "docsisOperator", "docsisMonitor", 
// or "docsisUser". These would come from the DOCSIS 1.1 config file.
// pDhPublicKey is the diffie-helman public key for this view, and
// DhKeySize is the size of the key.
************************************************************************************/

SNMP_STATUS AgentInstallProvisionedView (char *pVacmUserName, unsigned char *pDhPublicKey, int DhKeySize)
{

#if (SNMP_SUPPORT != 3)
    return SNMP_GEN_ERR;
#else
    return (AgentInstallTestManager (pVacmUserName));


#if 0
//    int            /*ret,*/ secLevel;
/*    
    unsigned char  authKey[] = {0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
        0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44};
*/

//    unsigned long subtree_entireMib[] = {1,3,6,1}; 

//    printf(" Calling AgentVacmTest      TRYING---- TODO: AgentInstallProvisionedView not yet implemented\n");
//    AgentVacmTest();
//    return 0;



    printf ("AgentAddProvisionedView for user name %s\n", pVacmUserName);

    if ( strcmp (pVacmUserName, "testManager") == 0 )
    {
        printf("Installing the testManager\n");
        //clean up if we have an old testManager hanging around
//        AgentDestroyUser(pVacmUserName);
//        AgentDestroyAccess(pVacmUserName,SNMP_SEC_NONE, SNMP_SEC_MDL_USM);
//        AgentDestroyAccess(pVacmUserName,SNMP_SEC_AUTH, SNMP_SEC_MDL_USM);
//        AgentDestroyAccess(pVacmUserName,SNMP_SEC_PRIV, SNMP_SEC_MDL_USM);    

        if ( 0 == strcmp((char *)pDhPublicKey,"snmpauth") )
            secLevel = SNMP_SEC_AUTH;
        else if ( 0 == strcmp((char *)pDhPublicKey,"snmppriv") )
            secLevel = SNMP_SEC_PRIV;
        else
            secLevel = SNMP_SEC_NONE;

        AgentConfigView ("testManagerView", subtree_entireMib, sizeof(subtree_entireMib)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);
        AgentInstallGroup ("testManager", "testManager", SNMP_SEC_MDL_USM);
        AgentInstallUser ("testManager", secLevel);
        AgentInstallAccess ("testManager", "testManagerView", "testManagerView", "testManagerView", secLevel, SNMP_SEC_MDL_USM);
        //AgentInstallDebugUser();
    }
    printf("NOT SETTING KEYS FOR USER _ WARNING\n");
#endif
#if 0

    user = SNMP_User_Lookup((bits8_t *)&MY_ENGINE_ID, LEN_ENGINE_ID,
                            (unsigned char *)pVacmUserName, strlen(pVacmUserName));

    if ( user == 0 )
    {
        printf("User lookup failed!\n");
        return SNMP_BAD_VALUE;
    }

    //for now we are putting the DhPublicKey in as the password!
    ret = GetMyAuthKeyFromPassword((char *)pDhPublicKey,strlen((char *)pDhPublicKey),authKey);
    if ( ret != 0 )
    {
        printf("GetMyAuthKeyFromPassword failed!\n");
        return SNMP_BAD_VALUE;
    }

    ret = SNMP_User_Set_AuthKey(user, authKey, LEN_MD5_KEY);
    if ( trace_level > 1 )
        printf("SNMP_USER_SET_AuthKey returned with a %d - false = set ok\n",ret);
    ret = SNMP_User_Set_PrivKey(user, authKey, LEN_MD5_KEY);
    if ( trace_level > 1 )
        printf("SNMP_USER_SET_PrivKey returned with a %d - false = set ok\n",ret);

    ret = SNMP_User_Set_Status(user, 1); //enable the user
    if ( trace_level > 1 )
        printf("SNMP_USER_SET_Status returned with a %d  - true = set ok\n",ret);

    return SNMP_NO_ERROR;
#endif
#endif

}

/*******************************************************************************
// FUNCTION:   AgentInstallTestManager()
//                      
// PURPOSE:    A debug function.  Called to setup some VACM stuff for testing.
//                          
// PARAMETERS: NONE
// RETURNS:     
*******************************************************************************/
SNMP_STATUS AgentInstallTestManager (char *pVacmUserName)
{
#if (SNMP_SUPPORT != 3)
    return SNMP_GEN_ERR;
#else

   static unsigned long subtree_all[] = {1,3,6,1}; 
   char pAuthKey[20], pPrivKey[20];
   int  AuthKeyLen, PrivKeyLen, secLevel;
       
   //printf("AgentVacmTest: Installing user for debug!");

    if (strcmp (pVacmUserName, "testManager") == 0)
    {
        secLevel = SNMP_SEC_NONE;
        memset(pAuthKey, '\0',sizeof(pAuthKey));
        memset(pPrivKey, '\0',sizeof(pAuthKey));
        AuthKeyLen = 0;
        PrivKeyLen = 0;
    }
    else if(strcmp (pVacmUserName, "testManager_auth") == 0)
    {
        secLevel = SNMP_SEC_AUTH;
        memset(pAuthKey, '\0',sizeof(pAuthKey));
        memset(pPrivKey, '\0',sizeof(pAuthKey));
        strcpy(pAuthKey,"testManager_auth");
        AuthKeyLen = 16;
        PrivKeyLen = 0;
    }
    else if(strcmp (pVacmUserName, "testManager_priv") == 0)
    {
        secLevel = SNMP_SEC_PRIV;
        memset(pAuthKey, '\0',sizeof(pAuthKey));
        memset(pPrivKey, '\0',sizeof(pAuthKey));
        strcpy(pAuthKey,"testManager_priv");
        strcpy(pAuthKey,"testManager_priv");
        AuthKeyLen = 16;
        PrivKeyLen = 16;
    }
    else
    {
        printf("AgentInstallTestManager: Failed - no valid userName\n");
        return 1;
    }
    
    if (AgentInstallUserComplete ((unsigned char *)(pVacmUserName), //unsigned char *pSecName,
                                   strlen(pVacmUserName),            //unsigned int SecNameLen,
                                   secLevel,   //int SecLevel,
                                   NULL,       //unsigned char *pEngineId, 
                                   0,          //int EngineIdLen,
                                   (u_char *) pAuthKey,   //unsigned char *pAuthKey,
                                   AuthKeyLen, //int AuthKeyLen,
                                   AUTH_MD5,   //int AuthType,
                                   (u_char *) pPrivKey,   //unsigned char *pPrivKey,
                                   PrivKeyLen, //int PrivKeyLen,
                                   0,          //int Password2Key, 
                                   1) )        //int LocalizePwd)
        printf("AgentInstallUserComplete FAILED\n");

    if ( AgentConfigView ("testView", subtree_all, sizeof(subtree_all)/sizeof(unsigned long), VIEW_STATUS_INCLUDE) )
         printf("AgentConfig FAILED\n");
    
    if ( AgentInstallGroup  ("testGrp", pVacmUserName, SNMP_SEC_MDL_USM) )
         printf("AgentInstallGroup FAILED\n");
    
    if ( AgentInstallAccess ("testGrp", "testView", "testView", "testView", secLevel, SNMP_SEC_MDL_USM) )
         printf("AgentInstallAccess FAILED\n");


#endif
 return SNMP_NO_ERROR;
}




SNMP_STATUS AgentInstallInformUserFromProbe (unsigned char *pUserName, unsigned int UserNameLen, unsigned char * pEngId, int EngIdLen)
{
    printf("TODO: AgentInstallInformUserFromProbe not yet implemented\n");
    return 1;


}

/*******************************************************************************
// FUNCTION:   AgentVacmTest()
//                      
// PURPOSE:    A debug function.  Called to setup some VACM stuff for testing.
//                          
// PARAMETERS: NONE
// RETURNS:     
*******************************************************************************/
void AgentVacmTest()
{

    static unsigned long subtree_all[] = {1,3,6,1}; 
    int tl, ret;
    //unsigned char id[] = {0x80, 0x00, 0x11, 0x3d, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

    tl = trace_level;
    trace_level = 9;
    printf("AgentVacmTest: Starting!");
    printf("AgentVacmTest: Installing user for debug!");

    //AgentSetEngineId ((unsigned char *)&id, 12);

    AgentConfigView ("testView", subtree_all, sizeof(subtree_all)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);
    AgentInstallGroup("testGroup", "testUser", SNMP_SEC_MDL_USM);
    AgentInstallUser("testUser", SNMP_SEC_NONE);
    AgentInstallAccess("testGroup", "testView", "testView", "testView", SNMP_SEC_NONE, SNMP_SEC_MDL_USM);
    AgentEnableUser("testUser");

    //setup SNMPv1 also
    //note: these are in mibgroup/mibII - not supported for now
    //vacm_parse_simple("rocommunity", "public");
    //vacm_parse_simple("rwcommunity", "private");

    /////////////////
    ret= AgentInstallUserComplete ((u_char *) "testComplete", //unsigned char *pSecName,
                                   12,            //unsigned int SecNameLen,
                                   SNMP_SEC_AUTH, //int SecLevel,
                                   NULL,          //unsigned char *pEngineId, 
                                   0,             //int EngineIdLen,
                                   (u_char *) "basketballthomas",          //unsigned char *pAuthKey,
                                   16,             //int AuthKeyLen,
                                   AUTH_MD5,             //int AuthType,
                                   (u_char *) "basketballthomas",          //unsigned char *pPrivKey,
                                   16,             //int PrivKeyLen,
                                   0,             //int Password2Key, 
                                   1);            //int LocalizePwd)

    printf("AgentInstallUserComplete returned with a <%d>\n",ret);

    AgentInstallGroup  ("testCompleteGrp", "testComplete", SNMP_SEC_MDL_USM);
    AgentInstallAccess ("testCompleteGrp", "testView", "testView", "testView", SNMP_SEC_AUTH, SNMP_SEC_MDL_USM);


    //--Do an auth user
    ret= AgentInstallUserComplete ((u_char *) "testCompleteP", //unsigned char *pSecName,
                                   13,            //unsigned int SecNameLen,
                                   SNMP_SEC_PRIV, //int SecLevel,
                                   NULL,          //unsigned char *pEngineId, 
                                   0,             //int EngineIdLen,
                                   (u_char *) "basketballthomas",          //unsigned char *pAuthKey,
                                   16,             //int AuthKeyLen,
                                   AUTH_MD5,       //int AuthType,
                                   (u_char *) "basketballthomas",          //unsigned char *pPrivKey,
                                   16,             //int PrivKeyLen,
                                   0,             //int Password2Key, 
                                   1);            //int LocalizePwd)

    printf("AgentInstallUserComplete P returned with a <%d>\n",ret);

    AgentInstallGroup  ("testCompleteGrpP", "testCompleteP", SNMP_SEC_MDL_USM);
    AgentInstallAccess ("testCompleteGrpP", "testView", "testView", "testView", SNMP_SEC_PRIV, SNMP_SEC_MDL_USM);




    //////////////////
    printf("AgentVacmTest: Finished!");
    trace_level = tl;
//    #endif
}


#ifdef x0

/********************************************************************
* Function Name:  AgentInstallInformUserFromProbe
*
* Description:  This function installs a user with the given engine
* id.  It attempts to copy the auth and priv passwords from a same
* named user which should be installed with our agent's engine id.
* This function is typically called from the
* InstallInformUsersFromTablesWithAddress().
*
*********************************************************************/
SNMP_STATUS AgentInstallInformUserFromProbe (unsigned char *pUserName, unsigned int UserNameLen, unsigned char * pEngId, int EngIdLen)
{
#if (SNMP_SUPPORT == 3)
    SNMP_USER_T *LookUpUser;
    SNMP_AUTH_T *pAuth = NULL; 
    SNMP_PRIV_T *pPriv = NULL;
    int ret, AuthType=0, SecLevel=SNMP_SEC_NONE, AuthKeyLen=0, PrivKeyLen=0;
    //char debugUser[14];
    //unsigned char NotKey[] = {"basketballthomas"};
    //unsigned char * pNotKey = &NotKey[0];
    unsigned char AuthKey[LEN_SHA_KEY];  //sha key is longer than md5 key;
    unsigned char PrivKey[LEN_DES_KEY];
    unsigned char * pAuthKey = (unsigned char*)&AuthKey[0];
    unsigned char * pPrivKey = (unsigned char*)&PrivKey[0];
    OIDC_T md5_obj[] = {1,3,6,1,6,3,10,1,1,2};

    if ( trace_level > 0 )
    {
        DumpBuf("AgentInstallInformUser - userName=",pUserName,UserNameLen,'c');
        DumpBuf("AgentInstallInformUser - engineId=",pEngId,EngIdLen,'x');
    }

    //if engine id is our engine id exit
    if ( EngIdLen==LEN_ENGINE_ID )
    {
        if ( memcmp(pEngId, &MY_ENGINE_ID, EngIdLen) == 0 )
            return -1;
    }

    if ( (UserNameLen < 1) || (EngIdLen < 1) )
    {
        if ( trace_level > 0 )
            printf("AgentInstallInformUserFromProbe: bad input, so exit without installing user\n");
        return -1;
    }

    // Do user lookup - if don't find then return
    LookUpUser = SNMP_User_Lookup (MY_ENGINE_ID, LEN_ENGINE_ID, pUserName, UserNameLen);
    if ( LookUpUser == NULL )
    {
        if ( trace_level > 0 )
            printf ("AgentInstallInformUserFromProbe:SNMP_User_Lookup failure\n");
        return SNMP_NO_SUCH_NAME;
    }

    // Now we know there exists a user with this name and MY_ENGINE_ID.
    // Want to copy it but use the Manager's Engine Id

    // Get the Auth and Priv protocols to use for our new user
    pAuth = SNMP_User_Get_Auth(LookUpUser);

    if ( pAuth )
    {
        SecLevel = SNMP_SEC_AUTH;
        if ( pAuth == SNMP_Auth_Find(sizeof(md5_obj)/sizeof(OIDC_T), md5_obj) )
        {
            AuthKeyLen = LEN_MD5_KEY;
            AuthType = 1;
        }
        else
        {
            AuthKeyLen = LEN_SHA_KEY;
            AuthType = 2;
        }

        pAuthKey = SNMP_User_Get_AuthKey(LookUpUser);
        //DumpBuf("THE AUTH KEY",pAuthKey,AuthKeyLen, 'x');

        pPriv = SNMP_User_Get_Priv(LookUpUser);
        if ( pPriv )
        {
            pPrivKey = SNMP_User_Get_PrivKey(LookUpUser);
            PrivKeyLen = LEN_DES_KEY;
            SecLevel  = SNMP_SEC_PRIV;
            //DumpBuf("THE PRIV KEY",pPrivKey,PrivKeyLen, 'x');

        }
    }

    ret=  AgentInstallUserComplete (pUserName,  // pSecName
                                    UserNameLen,// UserName length
                                    SecLevel,   // SecLevel
                                    pEngId,     // EngineId
                                    EngIdLen,   // MgrEngineIdLen, 
                                    pAuthKey,   //pNotKey,    // NULL, *pAuthKey
                                    AuthKeyLen, // AuthKeyLen
                                    AuthType,   // AuthType 2=SHA, 1=MD5
                                    pPrivKey,   //pNotKey,    // NULL, *pPrivKey
                                    PrivKeyLen, // PrivKeyLen
                                    0,          // convert Password2Key
                                    0);         // LocalizePwd  

    if ( trace_level > 1 )
    {
        DumpBuf("AgentInstallInformUserFromProbe: Installed user with name  =", pUserName, UserNameLen,'c');
        DumpBuf("AgentInstallInformUserFromProbe: Installed user with engId =", pEngId, EngIdLen,'x');
        printf("Installing Inform User - AgentInstallUserComplete returned with a (%d)\n",ret);

    }

#else  
    printf ("AgentInstallInformUser stub for SNMPv1/v2c, doing nothing.\n");
#endif

    //printf("   Exiting from AgentInstallInformUser\n");
    return SNMP_NO_ERROR;
} 



#endif //x0

