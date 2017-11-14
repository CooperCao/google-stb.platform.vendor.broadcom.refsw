/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
//
//
//**************************************************************************
//    Filename: Kickstart.cpp
//    Author:   T. Clack
//    Creation Date: 21-May-03
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Agent API functions for SNMPv3 Diffie-Hellman Kickstart process.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#include <stdio.h>
#include <string.h>
#if ((__GNUC__ >= 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#else
#include <iostream.h>
#endif

#include "SnmpApi.h"
#include "usmDhBridge.h"
#include "BcmSnmpAgent.h"
#include "DhKickstartThread.h"
#include "usmDhMib.h"
#include "SnmpLog.h"
#include "OperatingSystemFactory.h"
#include "snmpDH.h"
#include <net-snmp/net-snmp-config.h>

#include <net-snmp/types.h>
#include <net-snmp/library/snmpv3.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/library/snmp_secmod.h>
#include <net-snmp/library/snmpusm.h>
#include "BcmV2Glue.h" 

#include "AgentDefs.h"

extern struct usmUser *userList; //the global user list
extern int    trace_level;


extern unsigned char MY_ENGINE_ID[];
extern int LEN_ENGINE_ID;

SnmpDh*   FindUserSnmpDhObject(BcmSnmpV3Agent *pAgent, BcmString &securityName);
void      UpdateUsrDhObjectContainer(BcmSnmpV3Agent *pAgent);

/********************************************************************
* Function Name: AgentKickstartRegister()
* Returns:  
*
* Description: Should be called DURING CM Registration to start the  
*  Diffie-Hellman kickstart process.  Called for each SecurityName
*  read from the config file.
* 
*  If pVacmUserName is in the usmUsrEntry table it is set to active
*  and an entry is placed in the usmDHKickstartTable.
*********************************************************************/
SNMP_STATUS AgentKickstartRegister (int IfNum, char * pSecurityName, char * pMgrPublicNum, int len_mgrPublicNum)
{
    BcmString    securityName, mgrPublic; 
    BcmSnmpV3Agent *pAgent;

    //printf ("Starting Kickstart Register for user name %s\n", pSecurityName);
    securityName = pSecurityName;
    mgrPublic.assign((char*)pMgrPublicNum,len_mgrPublicNum);

    // Enable the Security name in the usmUserTable MIB.
    if ( SNMP_NO_ERROR != AgentEnableUser (pSecurityName) )
    {
        printf("Unable to enable kickstart user <%s>!\n",pSecurityName);
        return SNMP_BAD_VALUE;
    }

    // Add the Security name to the dhKickstart MIB.
    BcmMibBridge *pBridge;
    SnmpDh       *dh;

    pAgent = BcmSnmpThread::Singleton().FindV3AgentByIf (IfNum);
    if ( pAgent == NULL )
        return SNMP_RESOURCE_UNAVAILABLE;

    //UpdateUsrDhObjectContainer();			 //Create a snmpDh obj for all usm entries.
    dh = FindUserSnmpDhObject(pAgent, securityName); //Get the snmpDh obj for this name.
    if ( dh == NULL )
    {
        SnmpLogRaw <<"dh null from findusersnmpdhobject - calling update..container"<<endl;
        UpdateUsrDhObjectContainer(pAgent);    //Create a snmpDh obj for all usm entries.
        //try again to find it
        dh = FindUserSnmpDhObject(pAgent, securityName); //Get the snmpDh obj for this name.
        if ( dh == NULL )
        {
            SnmpLogRaw <<"dh null from findusersnmpdhojbet"<<endl;
            return SNMP_BAD_VALUE;
        }
    }
    dh->SetMgrPublic(mgrPublic);

    // Put a DH kikstart entry in our agent.  To do this, we need to assign
    // an index which is 1 greater than the index of the last entry that's
    // currently in the table.  So find the table first of all:
    BcmMibTable *pDHTable = pAgent->FindTable (BcmString ("usmDHKickstartTable"));
    if ( pDHTable )
    {
        // Initialize Index to 1, that's what we'll use if the table is empty.
        int Index = 1;

        // Get the last entry in the table.
        BcmMibEntry *pEntry = pDHTable->LastEntry();
        if ( pEntry )
        {
            BcmObjectId LastIx;

            // Our new index is his index + 1.
            pEntry->GetIndex (LastIx);
            Index = LastIx[0] + 1;
        }

        pBridge = new usmDHKickstartEntryBridge(Index,dh,pAgent);
        if ( pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR )
            delete pBridge;
    }
    return SNMP_NO_ERROR;
}

/********************************************************************
* Function Name: AgentKickstartFinish()
* Returns:  
*				 
* Description: Should be called AFTER CM Registration to complete the 
*  Diffie-Hellman usmDHUserTable key change feature and to complete the 
*  Diffie-Hellman kickstart process.
*
*  1.Generate a DH mypublic number for each entry in the usmUserTable.
*  2.Put the mypublic number in the usmDHUserTable.
*  3.Find the user in the usmDhKickstartTable.
*  4. Calculate DH secret key, then the Auth and Priv keys.
*    
*********************************************************************/
SNMP_STATUS AgentKickstartFinish(int IfNum)
{
#if (SNMP_SUPPORT != 3)
    return SNMP_GEN_ERR;
#endif

    //ks stuff
    usmDHKickstartEntryBridge * pKsBridge;
    usmDHKickstartTable       * pKsEntry;
    usmDHKickstartTable       * kickstartTable;
    char usmDHKickstartTableOid[20] = {"1.3.6.1.3.101.1.2.1"};
    BcmString           secName;
    MibEntryIterator    Pos;
    BcmString           sSecName;
    int                 /*ret,*/ foundKsEntry;
    BcmSnmpV3Agent *pAgent;
    BcmDhKickstartThread *pDhThread;


    //usmDhKey stuff
    BcmString     indexId,indexName;
    BcmMibBridge  *pDhKeyBridge;

    // Diffie stuff
    SnmpDh*       dh;
    BcmString     mypublic, mgrpublic;
    unsigned char v3AuthKey[16], v3PrivKey[16];
/*  unsigned char authKey[] = {0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
        0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44};*/

    pAgent = BcmSnmpThread::Singleton().FindV3AgentByIf (IfNum);
    if ( pAgent == NULL )
        return SNMP_RESOURCE_UNAVAILABLE;

    pDhThread = pAgent->DhKickstartThread();
    if ( pDhThread == NULL )
        return SNMP_RESOURCE_UNAVAILABLE;

    //
    // Start a loop getting each entry in the usmUser MIB.

    //Get a ptr to the user container
    // BcmDhKickstartThread *pDhThread = pAgent->DhKickstartThread();
    // DhUserContainer* uc = pDhThread->GetDhUserContainer ();

    struct usmUser *user = userList; //start at the head of the global userList

    while ( user != NULL )
    {
        if ( pDhThread->ShouldAbortCalculations() == true )
        {
            SnmpLogRaw  << "Aborting DH kickstart calculations!" << endl;
            break;
        }

        // Give the OS a chance to task-switch
        BcmOperatingSystemFactory::ThreadSleep (10);

        //should be walking the user list 
        sSecName.assign (user->name,strlen(user->name));

        //SnmpLogRaw <<"Performing KICKSTART calculation for "<<sSecName<<endl;
        // PR 8478 - allow any user, not just these ones
        // if ( !((sSecName == "docsisManager") ||
        //        (sSecName == "docsisOperator")||
        //        (sSecName == "docsisMonitor") ||
        //        (sSecName == "docsisUser")    ||
        //        (sSecName == "CHAdministrator") ) )
        // {
             //Only allow the docis users to be in this table
             //SnmpLogRaw <<"Not performing calculations for "<<sSecName<<endl;
        //     user = user->next;
        //     continue;
        // }

        //	
        // 1.Generate a DH public number for each entry in the usmUserTable.		

        //Get the SnmpDh object for this securityName
        dh = FindUserSnmpDhObject(pAgent, sSecName);
        if ( dh == NULL )
        {
            SnmpLogRaw <<"FindUserSnmpDhObject returned NULL!"<<endl;
            user = user->next;
            continue;
        }

        //Generate a public key using a random number
        dh->GenMyPublic(mypublic);
        //dh->Dump("mypublic number",mypublic);

        //
        // 2.Put the public number in the usmDHUserTable.
        if ( trace_level > 1 )
            printf("Creating a usmDHUserKeyEntryBridge object\n");

        indexId.assign((char *) MY_ENGINE_ID, LEN_ENGINE_ID);    

        pDhKeyBridge = new usmDHUserKeyEntryBridge (indexId,  //indexId
                                                    dh,       //SnmpDH object
                                                    pAgent);

        if ( pDhKeyBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR )
            delete pDhKeyBridge;

        //
        // 3.Find this user in the usmDhKickstartTable:

        BcmObjectId tempBcmObjectId = BcmObjectId(usmDHKickstartTableOid);
        kickstartTable = (usmDHKickstartTable *) pAgent->FindTable(tempBcmObjectId);
        //get the first entry in the usmDHKickstartTable
        pKsEntry = (usmDHKickstartTable *) kickstartTable->FirstEntry(Pos);
        if ( pKsEntry == NULL )
        {
            SnmpLogRaw <<"FAILED pksEntry == NULL in AgentKickstartFinish\n"<<endl;
            return SNMP_RESOURCE_UNAVAILABLE;
        }
        foundKsEntry = 0;

        while ( pKsEntry != NULL )
        {
            if ( pDhThread->ShouldAbortCalculations() == true )
            {
                SnmpLogRaw  << "Aborting DH kickstart calculations!" << endl;
                break;
            }

            // Give the OS a chance to task-switch
            BcmOperatingSystemFactory::ThreadSleep (10);

            //get the bridge object for this entry
            pKsBridge =  (usmDHKickstartEntryBridge *)pKsEntry->fpMibBridge;
            pKsBridge->Get_usmDHKickstartSecurityName(secName);
            foundKsEntry = 0;
            if ( secName == sSecName )
            {
                //SnmpLogRaw <<secName<<" - "<<sSecName<<" Security name found in the KS mib"<<endl;
                foundKsEntry = 1;
                break;
            }

            pKsEntry = (usmDHKickstartTable *) kickstartTable->NextEntry(Pos); 

            foundKsEntry = 0;
        }

        if ( foundKsEntry )
        {
            //dh->DumpAll("");

            // 4. Calculate DH secret key, then the Auth and Priv keys.
            //

            //Auth key
            dh->GenV3Key(AUTHKEY, v3AuthKey);
            //SnmpLogRaw <<"SecurityName = <"<<secName<<">"<<endl;
            //dh->Dump("AuthKey for V3",v3AuthKey,sizeof(v3AuthKey));

            if ( (user->authKey = (u_char *) malloc(LEN_MD5_KEY)) != NULL )
            {
                user->authKeyLen = LEN_MD5_KEY;
                memcpy(user->authKey, v3AuthKey, LEN_MD5_KEY);
            }
            else
            {
                printf("kickstart - Couldn't malloc for auth key.\n");
                return SNMP_BAD_VALUE;
            }


            //Priv Key
            //docsisManager and docsisOperator use DES privacy
            // if ( (sSecName == "docsisManager")||(sSecName == "docsisOperator") || (sSecName == "CHAdministrator") )
            //PR8478 Other TLV34 non-defined users may use privacy
            if ( (sSecName != "docsisMonitor")||(sSecName != "docsisUser") )
            {
                dh->GenV3Key(PRIVKEY, v3PrivKey);
                //dh->Dump("PrivKey for V3",v3PrivKey,sizeof(v3PrivKey));   

                if ( (user->privKey = (u_char *) malloc(LEN_DES_KEY)) != NULL )
                {
                    user->privKeyLen = LEN_DES_KEY;
                    memcpy(user->privKey, v3PrivKey, LEN_DES_KEY);
                }
                else
                {
                    printf("kickstart - Couldn't malloc for priv key.\n");
                    return SNMP_BAD_VALUE;
                }     
            }
        } //end of (if pKsBridge)

        user = user->next;
    } //end of (while user)


    if ( pDhThread->ShouldAbortCalculations() == true )
    {
        SnmpLogRaw  << "DH kickstart calculations aborted." << endl;
        return SNMP_GEN_ERR;
    }

/* Plug memory leak - do this in CmSnmpAgent instead.
    // add support for the dh params object
    pDhKeyBridge = new usmDHPublicObjectsGroupBridge (pAgent);
    if (pDhKeyBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
        delete pDhKeyBridge;
*/

    // Send a trap here to let them know we are finally ready.
    // Note: traps only go if notification sender is setup properly (TLV38)
    // Do this in DhKickstartThread instead for portability.
    //SendSnmpTrap (NULL, COLD_START_TRAP );

    SnmpLogRaw <<"... SNMPv3 usmUsers initialization completed." <<endl;

    return SNMP_NO_ERROR; 
}


/********************************************************************
* Function Name: FindUserSnmpDHObject()
* Returns:  
*				 
* Description: Search the DhUserContainer for the object described
* by the given SecurityName.
*********************************************************************/
SnmpDh* FindUserSnmpDhObject(BcmSnmpV3Agent *pAgent, BcmString &securityName)
{
    SnmpDh* tdm;
    BcmDhKickstartThread *pDhThread = pAgent->DhKickstartThread();
    //SnmpLogRaw <<"FindUserSnmpDhObject -start"<<endl;
    if ( pDhThread == NULL )
        return NULL;

    // Spin through all Objects and see if we find the one passed in.
    // 

    //Get a ptr to the user container


    DhUserContainer* uc = pDhThread->GetDhUserContainer ();

    DhUserContainer::iterator dhPos;

    dhPos = uc->begin();

    while ( dhPos != uc->end() )
    {
        tdm = (SnmpDh *) *dhPos;
        if ( *(tdm->GetSecurityName()) == securityName )
        {
            return tdm;
        }

        dhPos++;
    }
    if ( trace_level >1 )
        SnmpLogRaw <<"FindUserSNmpDHObject returning NULL - "<<securityName<<" not found"<<endl;
    return NULL;
}

/********************************************************************
* Function Name: RecalculateMyPublicNumbers()
* Returns:  
*				 
* Description: Recalculate myPublic number for each SnmpDh object in
* the DhUserContainer.
*********************************************************************/
void RecalculateMyPublicNumbers(BcmSnmpV3Agent *pAgent)
{
    SnmpDh* tdm;
    BcmString mypublic;
    BcmDhKickstartThread *pDhThread = pAgent->DhKickstartThread();

    if ( pDhThread == NULL )
        return;

    // Get a ptr to the user container
    DhUserContainer* uc = pDhThread->GetDhUserContainer ();

    DhUserContainer::iterator dhPos;

    dhPos = uc->begin();

    while ( dhPos != uc->end() )
    {
        tdm = (SnmpDh *) *dhPos;
        //SnmpLogRaw <<"calculating myPublic for "<<*(tdm->GetSecurityName())<<endl;
        tdm->GenMyPublic(mypublic);

        dhPos++;
    }
}

/********************************************************************
* Function Name: AgentDhKeyChange()
* Returns:  
*				 
* Description: Should be called to change a SNMPv3 user key.  Typically
*  this is in response to a SET to the usmDHUserKeyTable. The SnmpDh
*  object that is passed in should contain the new mgrPublic value.
**********************************************************************/
SNMP_STATUS AgentDhKeyChange(int keyType, SnmpDh* dh)
{

    int           ret=0;
    BcmString     bv3AuthKey;
    BcmString     securityName;
    unsigned char authKey[] = {0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
        0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44};

    securityName = *(dh->GetSecurityName());


    //NOTE be sure that the mgrPublic has been set

    if ( dh->GenV3KeyChange(bv3AuthKey) != 0 )
        return SNMP_WRONG_VALUE;

    //dh->Dump("key change - NEW Key for V3",bv3AuthKey);

    //Install Auth and Priv keys in the usmUser Mib to enable the ks user.

    //ret = GetMyAuthKeyFromPassword((char*)bv3AuthKey.data(),16,authKey);  //localize key
    memcpy(authKey,bv3AuthKey.data(),LEN_MD5_KEY);  //Not localizing key


    // set the Auth and Priv keys
    if ( keyType == AUTHKEY )
    {
        if ( trace_level > 1 )
            SnmpLogRaw <<"Changing AUTH key"<<endl;
        ret=bcmV2_set_userKey(MY_ENGINE_ID, LEN_ENGINE_ID, (unsigned char *)securityName.data(),
                              1, authKey, LEN_MD5_KEY);
    }
    else if ( keyType == PRIVKEY )
    {
        if ( trace_level>1 )
            SnmpLogRaw <<"Changing PRIV key"<<endl;
        ret=bcmV2_set_userKey(MY_ENGINE_ID, LEN_ENGINE_ID, (unsigned char *)securityName.data(),
                              2, authKey, LEN_MD5_KEY);

    }


    if ( trace_level > 1 )
    {
        SnmpLogRaw <<"key change "<< (ret==1 ? "PASSED" : "FAILED")<<endl;
        dh->DumpAll((char *) "key change at completion");
        SnmpLogRaw <<" SNMPv3 KEY CHANGE completed."<<endl;
    }
    return SNMP_NO_ERROR; 
}


/********************************************************************
* Function Name: UpdateUsrDhObjectContainer()
* Returns:  
*				 
* Description: Makes sure we have a SnmpDH object for each entry in 
* the usmUserTable.
*********************************************************************/
void UpdateUsrDhObjectContainer(BcmSnmpV3Agent *pAgent)
{
//  unsigned char    id[64],name[64];
//  int              idlen, nlen;
    BcmString        securityName;
    BcmDhKickstartThread *pDhThread = pAgent->DhKickstartThread();

    if ( pDhThread == NULL )
        return;

    //Get a ptr to the user container
    DhUserContainer* uc = pDhThread->GetDhUserContainer ();

    // Start a loop getting each entry in the usmUser MIB
    struct usmUser *user = userList; //start at the head of the global userList

    //while ( (user = bcmV2_usm_get_user_from_list(MY_ENGINE_ID, LEN_ENGINE_ID, user)) != NULL )
    while ( user != NULL )
    {
        //should be walking the user list 
        securityName.assign (user->name,strlen(user->name));

        if ( FindUserSnmpDhObject(pAgent, securityName) == NULL )
        {
            //object does not exist so.. create one.
            SnmpLogRaw <<"Creating a SnmpDh object for "<<securityName<<endl;
            SnmpDh* dh = new SnmpDh(securityName);

            uc->push_back(dh); //add to the container
        }
        user = user->next;
    }

}
