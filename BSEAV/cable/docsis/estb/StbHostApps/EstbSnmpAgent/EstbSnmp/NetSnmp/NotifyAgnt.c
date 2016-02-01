//**************************************************************************
//
//    Copyright 2003  Broadcom Corporation
//    All Rights Reserved
//    No portions of this material may be reproduced in any form without the
//    written permission of:
//             Broadcom Corporation
//             16251 Laguna Canyon Road
//             Irvine, California  92618
//    All information contained in this document is Broadcom Corporation
//    company private, proprietary, and trade secret.
//
//
//
//**************************************************************************
//    Filename: NotifyAgnt.c
//    Author:   T. Clack
//    Creation Date: 20-May-03
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Agent API functions for configuring SNMPv3 Notifications.
//    NotifyAgnt ported for use with NetSnmp.
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "SnmpType.h"
#include "SnmpApi.h"

#ifdef NOTDEF
#include <mem.h> // for strdup
#endif
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>
#include <net-snmp/library/snmpv3.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/snmp_secmod.h>
#include <net-snmp/library/snmpusm.h>
#include <net-snmp/library/transform_oids.h>
#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/agent/snmp_agent.h>
#include <net-snmp/agent/agent_callbacks.h>
#include <net-snmp/agent/agent_trap.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/snmp_vars.h>

#include <net-snmp/agent/mibgroup/target/snmpTargetParamsEntry.h>
#include <net-snmp/agent/mibgroup/target/snmpTargetAddrEntry.h>
#include <net-snmp/agent/mibgroup/notification/snmpNotifyTable.h>
#include <net-snmp/agent/mibgroup/notification/snmpNotifyFilterTable.h>
#include <net-snmp/agent/mibgroup/notification/snmpNotifyFilterProfileTable.h>
#include <net-snmp/agent/mibgroup/mibII/vacm_vars.h>  // for vacm_in_view_callback
#include <net-snmp/agent/mibgroup/header_complex.h> 

#include "BcmV2Glue.h"
#include "AgentDefs.h"
#include "AgentUtils.h"


//externs for functions that are not prototyped in NetSnmp
extern void snmpTargetParamTable_dispose(struct targetParamTable_struct *reaped);
extern  int snmpTargetParams_addSecName(struct targetParamTable_struct *entry,char *cptr);
extern  int snmpTargetParams_addParamName(struct targetParamTable_struct *entry,char *cptr);
extern  int snmpTargetAddr_addTagList(struct targetAddrTable_struct *entry, char *cptr);
extern void snmpTargetAddrTable_dispose(struct targetAddrTable_struct *reaped);
extern  int snmpTargetAddr_addName(struct targetAddrTable_struct *entry, char *cptr);
extern  int snmpTargetAddr_addParams(struct targetAddrTable_struct *entry, char *cptr);
extern  int snmpNotifyFilterProfileTable_add(struct snmpNotifyFilterProfileTable_data *thedata);
extern  int snmpNotifyFilterTable_add(struct snmpNotifyFilterTable_data *thedata);

extern struct targetParamTable_struct *aPTable;   //the global targetParamsTable
extern struct targetAddrTable_struct *aAddrTable; //the global targetAddrTable
extern struct header_complex_index *snmpNotifyTableStorage; //the global notifyTable
extern struct header_complex_index *snmpNotifyFilterTableStorage; //the global notifyFilterTable
extern struct header_complex_index *snmpNotifyFilterProfileTableStorage; //the global notifyProfileTable

// Prototype that unfortunately we can't currently define in bcmV2Glue.h
// due to include path issues...
netsnmp_variable_list *bcmV2_get_variable_list (MibObject *pObjects);

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
SNMP_STATUS AgentProvTargetParamsTable(int  trapType, unsigned char * elementName,
                                       int  elementNameLen,unsigned char * securityName,
                                       int  securityNameLen, unsigned char * tlv38Base)
{
    struct targetParamTable_struct *entry;
    u_char debugBuf[256];
    char sElementName[32], sSecurityName[32];

    memset(sElementName,'\0',32);
    memset(sSecurityName,'\0',32);

    memcpy(sElementName,elementName,elementNameLen);
    memcpy(sSecurityName,securityName,securityNameLen);

    printf("AgentProvTargetParamsTable: trapType = %d, elementName = %s, securityName = %s\n", trapType, sElementName, sSecurityName);
    if ( (entry = snmpTargetParamTable_create()) == NULL )
        return SNMP_GEN_ERR;

    if ( strlen((char *)sSecurityName) == 0 )
    {
        if ( snmpTargetParams_addSecName(entry, (char *) tlv38Base) == 0 )
        {
            snmpTargetParamTable_dispose(entry);
            return SNMP_GEN_ERR;
        }
        entry->secLevel = SNMP_SEC_LEVEL_NOAUTH; //noAuthnoPriv
    }
    else
    {
        if ( snmpTargetParams_addSecName(entry, (char *)sSecurityName) == 0 )
        {
            snmpTargetParamTable_dispose(entry);
            return SNMP_GEN_ERR;
        }
        if ( strcmp((char *)sSecurityName,"docsisManager") == 0 )
        {
            entry->secLevel = SNMP_SEC_LEVEL_AUTHPRIV; //authPriv 3
        }
        else
        {
            entry->secLevel = SNMP_SEC_LEVEL_AUTHNOPRIV; //authNoPriv 2
        }
    }

    if ( strcmp((char *)sSecurityName,(char *)tlv38Base) == 0 )
        entry->secLevel = SNMP_SEC_LEVEL_NOAUTH; //noAuthnoPriv 1

    if ( strcmp((char *)sSecurityName,"testManager") == 0 )
        entry->secLevel = SNMP_SEC_LEVEL_NOAUTH; //noAuthnoPriv 1

    switch ( trapType )
    {
    case 1:
        entry->mpModel  = 0; //SNMPv1
        entry->secModel = 1;
        break;
    case 2:
    case 3:
        entry->mpModel  = 1; //SNMPv2c
        entry->secModel = 2;
        break;
    case 4:
    case 5:
        entry->mpModel  = 3; //SNMPv3
        entry->secModel = 3;
        break;
    default:
        snmpTargetParamTable_dispose(entry);
        return SNMP_ERR_GENERR;
        break;
    }
    entry->storageType = SNMP_STORAGE_VOLATILE;

    if ( snmpTargetParams_addParamName(entry, (char *)sElementName)  == 0 )
    {
        snmpTargetParamTable_dispose(entry);
        return SNMP_GEN_ERR;
    }
    entry->rowStatus = SNMP_ROW_ACTIVE;
    sprintf((char *)debugBuf, "snmp_parse_config_targetParams, read: %s %d %d %s %d %d %d\n",
            entry->paramName, entry->mpModel,  entry->secModel,
            entry->secName,   entry->secLevel, entry->storageType,
            entry->rowStatus);
    DEBUG("snmpTargetParamsEntry - %s\n", (char *)debugBuf);

    snmpTargetParamTable_add(entry);
    return SNMP_NO_ERROR;            
}

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
                                      unsigned char *tlv38BaseInform, unsigned char *tlv38BaseTrap)
{
    u_char    tagName[15];
    int       tagNameLen, i;
    oid udpDomain[] = { 1, 3, 6, 1, 6, 1, 1};
    int udpDomainLen = sizeof(udpDomain) / sizeof(oid);
    //u_char  addr1[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    struct targetAddrTable_struct *entry;
    unsigned char debugBuf[1024];
    char sElementName[32];

    memset(sElementName,'\0',32);
    memcpy(sElementName,ElementName,ElementNameLen);

    if ( (entry = snmpTargetAddrTable_create()) == NULL )
        return SNMP_GEN_ERR;

    for ( i = udpDomainLen - 1; i >= 0; i-- )
        entry->tDomain[i] = udpDomain[i];
    entry->tDomainLen = udpDomainLen;

    /* we malloc 7 bytes just in case someone thinks this is a string */
    if ( (entry->tAddress = (unsigned char *)malloc(7)) == NULL )
    {
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
    }
    DumpBuf("AgentProvTargetAddrTable input taddress is",TargetIpAddr, 4, 'x');

    memset(entry->tAddress, 0, 7);
    memcpy(entry->tAddress,TargetIpAddr,4);     // address should be 4 bytes of IP addr and
    entry->tAddress[5] = (unsigned char)(Port); // 2 bytes of port
    entry->tAddress[4] = (unsigned char)(Port>>8);
    entry->tAddressLen = 6;
    DumpBuf("AgentProvTargetAddrTable entry->taddress is",entry->tAddress, 6, 'x');
    entry->timeout = TimeOut;
    entry->retryCount = Retries;

    memset(tagName, 0, sizeof(tagName));
    switch ( TrapType )
    {
    case 1:
    case 2:
    case 4:
        tagNameLen = strlen((char *)tlv38BaseTrap);
        memcpy(tagName,tlv38BaseTrap,tagNameLen);
        break;
    case 3:
    case 5:
        tagNameLen = strlen((char *)tlv38BaseInform);
        memcpy(tagName,tlv38BaseInform,tagNameLen);
        break;
    default:
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
        break;
    }
    if ( snmpTargetAddr_addTagList(entry, (char *)tagName) == 0 )
    {
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
    }

    if ( (snmpTargetAddr_addName(entry, (char *)sElementName)) == 0 )
    {
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
    }

    if ( (snmpTargetAddr_addParams(entry, (char *)sElementName)) == 0 )
    {
        snmpTargetAddrTable_dispose(entry);
        printf("AgentProvTargetAddrTable : did not add Params");
        return SNMP_GEN_ERR;
    }

    entry->storageType = SNMP_STORAGE_VOLATILE;

    entry->rowStatus = SNMP_ROW_ACTIVE;

    sprintf((char *)debugBuf, "snmp_parse_config_targetAddr, read: %s\n",
            entry->name);
    for ( i=0;i<entry->tDomainLen;i++ )
    {
        sprintf((char *)(&debugBuf[strlen((char *)debugBuf)]), ".%d", (int)entry->tDomain[i]);
    }
    sprintf((char *)(&debugBuf[strlen((char *)debugBuf)]), " %s %d %d %s %s %d %d\n",
            entry->tAddress, entry->timeout, entry->retryCount,
            entry->tagList,  entry->params,  entry->storageType,
            entry->rowStatus);
    DEBUG("AgentProvTargetAddrTable %s\n", (char *)debugBuf);

    snmpTargetAddrTable_add(entry);

    return SNMP_NO_ERROR;

}

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
                                      unsigned char *tagList)
{
    oid udpDomain[] = { 1, 3, 6, 1, 6, 1, 1};
    int udpDomainLen = sizeof(udpDomain) / sizeof(oid);
    int i;

    struct targetAddrTable_struct *entry;
    unsigned char debugBuf[1024];

    if ( (entry = snmpTargetAddrTable_create()) == NULL )
        return SNMP_GEN_ERR;

    for ( i = udpDomainLen - 1; i >= 0; i-- )
        entry->tDomain[i] = udpDomain[i];
    entry->tDomainLen = udpDomainLen;

    /* we malloc 7 bytes just in case someone thinks this is a string */
    if ( (entry->tAddress = (unsigned char *)malloc(7)) == NULL )
    {
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
    }
    memset(entry->tAddress, 0, 7);
    // address should be 4 bytes of IP addr and 2 of UDP port
    memcpy(entry->tAddress,addr,6);     // address should be 4 bytes of IP addr and
    entry->tAddressLen = 6;
    DumpBuf("AgentProvTargetAddrTable entry->taddress is",entry->tAddress, 6, 'x');
    entry->timeout = 1500;  // Default
    entry->retryCount = 3;  // Default

    if ( snmpTargetAddr_addTagList(entry, (char *)tagList) == 0 )
    {
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
    }

    if ( (snmpTargetAddr_addName(entry, (char *)name)) == 0 )
    {
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
    }

    if ( (snmpTargetAddr_addParams(entry, (char *) "")) == 0 )
    {
        snmpTargetAddrTable_dispose(entry);
        printf("AgentProvTargetAddrTable : did not add Params");
        return SNMP_GEN_ERR;
    }

    entry->storageType = SNMP_STORAGE_VOLATILE;

    entry->rowStatus = SNMP_ROW_ACTIVE;

    sprintf((char *)debugBuf, "snmp_parse_config_TLV53targetAddr, read: %s\n",
            entry->name);
    for ( i=0;i<entry->tDomainLen;i++ )
    {
        sprintf((char *)(&debugBuf[strlen((char *)debugBuf)]), ".%d", (int)entry->tDomain[i]);
    }
    sprintf((char *)(&debugBuf[strlen((char *)debugBuf)]), " %s %d %d %s %s %d %d\n",
            entry->tAddress, entry->timeout, entry->retryCount,
            entry->tagList,  entry->params,  entry->storageType,
            entry->rowStatus);
    DEBUG("AgentProvTargetAddrTable %s\n", (char *)debugBuf);

    snmpTargetAddrTable_add(entry);

    return SNMP_NO_ERROR;

}

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
SNMP_STATUS AgentProvNotifyFilterProfileTable (unsigned char *ElementName,int ElementNameLen)
{
    struct snmpNotifyFilterProfileTable_data *nfpPtr;
    char sElementName[32];

    memset(sElementName,'\0',32);
    memcpy(sElementName,ElementName,ElementNameLen);

    nfpPtr = SNMP_MALLOC_STRUCT(snmpNotifyFilterProfileTable_data);
    nfpPtr->snmpNotifyFilterProfileName = (char *) strdup((char *)sElementName);
    nfpPtr->snmpNotifyFilterProfileNameLen = strlen((char *)sElementName);

    nfpPtr->snmpTargetParamsName = (char *) strdup((char *)sElementName);
    nfpPtr->snmpTargetParamsNameLen = strlen((char *)sElementName);

    nfpPtr->snmpNotifyFilterProfileStorType  = ST_VOLATILE;
    nfpPtr->snmpNotifyFilterProfileRowStatus = RS_ACTIVE;

    if ( snmpNotifyFilterProfileTable_add(nfpPtr) == SNMPERR_SUCCESS )
        return SNMP_NO_ERROR;
    else
        return SNMP_GEN_ERR;
}

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
SNMP_STATUS AgentProvNotifyFilterTable (unsigned char *ElementName, int ElementNameLen, unsigned long *FilterOid, int FilterOidLen)
{
    struct snmpNotifyFilterTable_data *nfptr;
    char sElementName[32];

    memset(sElementName,'\0',32);
    memcpy(sElementName,ElementName,ElementNameLen);

    nfptr = SNMP_MALLOC_STRUCT(snmpNotifyFilterTable_data);
    nfptr->snmpNotifyFilterProfileName = (char *)strdup((char *)sElementName);
    nfptr->snmpNotifyFilterProfileNameLen = strlen((char *)sElementName);

    memdup((u_char **) & (nfptr->snmpNotifyFilterSubtree),
           (u_char *) FilterOid, (FilterOidLen * sizeof(oid)));
    nfptr->snmpNotifyFilterSubtreeLen = FilterOidLen;

    nfptr->snmpNotifyFilterMask = (char *) calloc(1, 1);
    nfptr->snmpNotifyFilterMaskLen = 0;

    nfptr->snmpNotifyFilterType = SNMPNOTIFYFILTERTYPE_INCLUDED;
    nfptr->snmpNotifyFilterStorageType = ST_VOLATILE;
    nfptr->snmpNotifyFilterRowStatus   = RS_ACTIVE;

    if ( snmpNotifyFilterTable_add(nfptr) == SNMPERR_SUCCESS )
        return SNMP_NO_ERROR;
    else
        return SNMP_GEN_ERR;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentProvNotifyTable
//                      
// PURPOSE:    Configures the snmpNotifyTable per the DOCSIS 1.1 requirements. 
//             Typically called once if any TLV38 element found in the Config File. 
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentProvNotifyTable (unsigned char * tlv38inform, unsigned char * tlv38trap)
{
    struct snmpNotifyTable_data *nptr;

    // printf("AgentProvNotifyTable: informTag %s, trapTag %s\n", informTag, trapTag);
    // add the inform tag, but only if the string is non-null
    if ( tlv38inform != NULL )
    {
        if ( strlen((char *)tlv38inform) > 0 )
        {
            nptr = (struct snmpNotifyTable_data *) SNMP_MALLOC_STRUCT(snmpNotifyTable_data);
            nptr->snmpNotifyName = (char *) strdup((char *)tlv38inform);
            nptr->snmpNotifyNameLen = strlen((char *)tlv38inform);
            nptr->snmpNotifyTag = (char *) strdup((char *)tlv38inform);
            nptr->snmpNotifyTagLen = strlen((char *)tlv38inform);
            nptr->snmpNotifyType = SNMPNOTIFYTYPE_INFORM;
            nptr->snmpNotifyStorageType = ST_VOLATILE;
            nptr->snmpNotifyRowStatus = RS_ACTIVE;

            if ( snmpNotifyTable_add(nptr) != SNMPERR_SUCCESS )
            {
                //clean up first
                return SNMP_GEN_ERR;
            }
        }
    }
    // add the trap tag, but only if the string is non-null
    if ( tlv38trap != NULL )
    {
        if ( strlen((char *)tlv38trap) > 0 )
        {
            nptr = (struct snmpNotifyTable_data *) SNMP_MALLOC_STRUCT(snmpNotifyTable_data);
            nptr->snmpNotifyName = (char *) strdup((char *)tlv38trap);
            nptr->snmpNotifyNameLen = strlen((char *)tlv38trap);
            nptr->snmpNotifyTag = (char *) strdup((char *)tlv38trap);
            nptr->snmpNotifyTagLen = strlen((char *)tlv38trap);
            nptr->snmpNotifyType = SNMPNOTIFYTYPE_TRAP;
            nptr->snmpNotifyStorageType = ST_VOLATILE;
            nptr->snmpNotifyRowStatus = RS_ACTIVE;

            if ( snmpNotifyTable_add(nptr) != SNMPERR_SUCCESS )
            {
                //clean up first
                return SNMP_GEN_ERR;
            }

        }
    }
    return SNMP_NO_ERROR;

}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentCleanupNotifies
//                      
// PURPOSE:    Trys to clean up Notify, NotifyFilter, NotifyFilterProfile,
//             TargetAddr, and TargetParams tables. 
//
// RETURNS:    SNMP_NO_ERROR on success
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentCleanupNotifies ()
{
    //clean up the Notify table
    {
        struct header_complex_index *hptr, *nhptr;
        struct snmpNotifyTable_data *nptr;

        for ( hptr = snmpNotifyTableStorage; hptr; hptr = nhptr )
        {
            DEBUG("AgentCleanupNotifies: Removing ROW snmpNotifyTable \n");
            nptr = (struct snmpNotifyTable_data *) (hptr->data);
            nhptr = hptr->next;
            header_complex_extract_entry(&snmpNotifyTableStorage, hptr);
            SNMP_FREE(nptr->snmpNotifyName);
            SNMP_FREE(nptr->snmpNotifyTag);
            SNMP_FREE(nptr);

        }
    }

    //clean up the NotifyFilter table
    {
        struct header_complex_index *hptr, *nhptr;
        struct snmpNotifyFilterTable_data *nptr;

        for ( hptr = snmpNotifyFilterTableStorage; hptr; hptr = nhptr )
        {
            DEBUG("AgentCleanupNotifies: Removing ROW snmpNotifyFilterTableStorage \n");
            nptr = (struct snmpNotifyFilterTable_data *) (hptr->data);
            nhptr = hptr->next;
            header_complex_extract_entry(&snmpNotifyFilterTableStorage, hptr);
            SNMP_FREE(nptr->snmpNotifyFilterMask);
            SNMP_FREE(nptr);
        }
    }

    //clean up the NotifyFilterProfile table
    {
        struct header_complex_index *hptr, *nhptr;
        struct snmpNotifyFilterProfileTable_data *nptr;

        for ( hptr = snmpNotifyFilterProfileTableStorage; hptr; hptr = nhptr )
        {
            DEBUG("AgentCleanupNotifies: Removing ROW snmpNotifyFilterProfileTableStorage \n");
            nptr = (struct snmpNotifyFilterProfileTable_data *) (hptr->data);
            nhptr = hptr->next;
            header_complex_extract_entry(&snmpNotifyFilterProfileTableStorage, hptr);
            SNMP_FREE(nptr->snmpNotifyFilterProfileName);
            SNMP_FREE(nptr);
        }
    }

    //clean up the TargetAddr table
    {
        struct targetAddrTable_struct *pEntry, *pEntryNext;

        pEntry = aAddrTable;  //the global targetAddrTable
        while ( pEntry != NULL )
        {
            DEBUG("AgentCleanupNotifies: Removing ROW snmpTargetAddrTable \n");
            pEntryNext = pEntry->next;
            snmpTargetAddrTable_dispose (pEntry);
            pEntry = pEntryNext;
        }
        aAddrTable = NULL;
    }

    //clean up the TargetParams table
    {
        struct targetParamTable_struct *pEntry, *pEntryNext;

        pEntry = aPTable;  //the global targetParmTable
        while ( pEntry != NULL )
        {
            DEBUG("AgentCleanupNotifies: Removing ROW snmpTargetParamTable \n");
            pEntryNext = pEntry->next;
            snmpTargetParamTable_dispose (pEntry);
            pEntry = pEntryNext;
        }
        aPTable = NULL;
    }

    return SNMP_NO_ERROR;
}

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
extern int bcmV2_send_notify (const unsigned char *pFromIp,oid *pTrapOID, int TrapOIDLength, time_t sysUpTime,netsnmp_variable_list *vars);
SNMP_STATUS AgentSendNotify (unsigned char *pFromIp, ObjectId *pTrapOid, time_t SysUpTime, MibObject *pObjects)
{
    netsnmp_variable_list *pVBs = NULL;
    int Stat;

    //printf ("AgentSendNotify -starting \n");


    if ( pObjects )
    {
        // Create a net-snmp style variable list from our V2/BFC pObjects pointer.
        pVBs = bcmV2_get_variable_list (pObjects);

        if ( pVBs == NULL )
        {
            DEBUG (" AgentSendNotify: bcmV2_get_variable_list failed!\n");
            return SNMP_GEN_ERR;
        }
    }

    // Get Net-SNMP to send a trap for us.
    Stat = bcmV2_send_notify(pFromIp, pTrapOid->pObjects, pTrapOid->Length, SysUpTime, pVBs);

    // Free the varbind list that got created for us.
    if ( pVBs )
        snmp_free_varbind (pVBs);

    if ( Stat == -1 )
        return SNMP_GEN_ERR;

    return SNMP_NO_ERROR;
}



////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentSendTrap
//
// PURPOSE:    
// PARAMETERS: 
// RETURNS:    SNMP_NO_ERROR on success
//             
////////////////////////////////////////////////////////////////////////////////
SNMP_STATUS AgentSendTrap (char *pCommunity, 
                           unsigned char *pFromIP, unsigned char *pToIP, ObjectId *pTrapOid, 
                           int GenericId, int SpecificId, time_t SysUpTime, MibObject *pObjects)
{
    printf("TODO: AgentSendTrap not yet implemented\n");
    return 1;

}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentSendV1Trap
//
// PURPOSE:    
// PARAMETERS: 
// RETURNS:    SNMP_NO_ERROR on success
//             
////////////////////////////////////////////////////////////////////////////////           
SNMP_STATUS AgentSendV1Trap (char *pCommunity, unsigned char *pFromIP, 
                             unsigned char *pToIP, int ToPort, ObjectId *pEnterprise,  
                             int GenericId, int SpecificId, time_t SysUpTime, 
                             MibObject *pObjects)
{
    netsnmp_variable_list *pVBs = NULL;
    int Stat;


    DEBUG ("AgentSendV1Trap\n");

    if ( pObjects )
    {
        // Create a net-snmp style variable list from our V2/BFC pObjects pointer.
        pVBs = bcmV2_get_variable_list (pObjects);

        if ( pVBs == NULL )
        {
            DEBUG ("  bcmV2_get_variable_list failed!\n");
            return SNMP_GEN_ERR;
        }
    }

    // Get Net-SNMP to send a trap for us.
    Stat = bcmV2_send_V1_trap(pFromIP, pToIP, ToPort, pCommunity, GenericId, SpecificId, pEnterprise->pObjects, pEnterprise->Length, SysUpTime, pVBs);

    // Free the varbind list that got created for us.
    if ( pVBs )
        snmp_free_varbind (pVBs);

    if ( Stat == -1 )
        return SNMP_GEN_ERR;

    return SNMP_NO_ERROR;
}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   AgentSendV2Trap
//
// PURPOSE:    
// PARAMETERS: 
// RETURNS:    SNMP_NO_ERROR on success
//             
////////////////////////////////////////////////////////////////////////////////                       
SNMP_STATUS AgentSendV2Trap (char *pCommunity, unsigned char *pFromIP, 
                             unsigned char *pToIP, int ToPort, ObjectId *pTrapOid, 
                             time_t SysUpTime, MibObject *pObjects)
{
    netsnmp_variable_list *pVBs = NULL;
    int Stat;


    DEBUG ("AgentSendV2Trap\n");

    if ( pObjects )
    {
        // Create a net-snmp style variable list from our V2/BFC pObjects pointer.
        pVBs = bcmV2_get_variable_list (pObjects);

        if ( pVBs == NULL )
        {
            DEBUG ("  bcmV2_get_variable_list failed!\n");
            return SNMP_GEN_ERR;
        }
    }

    // Get Net-SNMP to send a trap for us.
    Stat = bcmV2_send_V2_trap(pFromIP, pToIP, ToPort, pCommunity, pTrapOid->pObjects, pTrapOid->Length, SysUpTime, pVBs);

    // Free the varbind list that got created for us.
    if ( pVBs )
        snmp_free_varbind (pVBs);

    if ( Stat == -1 )
        return SNMP_GEN_ERR;

    return SNMP_NO_ERROR;
}     



// Do any necessary cleanup here, destroy installed views, etc.
SNMP_STATUS AgentInformRestartPending (int IfNum)
{
    return SNMP_NO_ERROR;
}

SNMP_STATUS AgentInformShutdownPending (int IfNum)
{
    return SNMP_NO_ERROR;
}


SNMP_STATUS AgentFindTargetAddr (const char *pName, int NameLen, TargetAddr *pAddr)
{
    struct targetAddrTable_struct *entry;

    entry = get_addrForName((char *)pName);

    /* if the table is empty, return 0 */
    if ( entry == NULL )
    {
        DEBUG("AgentFindTargetAddr: Entry not found!\n");
        return(0);
    }

    /* Copy NetSnmp things to V2 things */

    // Name
    pAddr->pName = entry->name;
    pAddr->NameLen = strlen(entry->name);

    // Domain
    pAddr->pDomain = entry->tDomain;
    pAddr->DomainLen = entry->tDomainLen;

    // Address
    pAddr->pAddr = entry->tAddress;
    pAddr->AddrLen = entry->tAddressLen;

    // TagList
    pAddr->pTagList = entry->tagList;
    if ( pAddr->pTagList != NULL )
        pAddr->TagListLen = strlen(entry->tagList);
    else
        pAddr->TagListLen = 0;

    // Status
    pAddr->Status = entry->rowStatus;

    return SNMP_NO_ERROR;

}

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
SNMP_STATUS AgentTargetParamsTableAddRow(unsigned char * elementName, int  elementNameLen,
                                 int mpModel, int secModel,
                                 unsigned char * securityName,int  securityNameLen,
                                 int secLevel,  int storageType)
{
    struct targetParamTable_struct *entry;
    char sElementName[32], sSecurityName[32];

    memset(sElementName,'\0',32);
    memset(sSecurityName,'\0',32);

    //make sure elementName and SecurityName are null terminated
    memcpy(sElementName,elementName,elementNameLen);
    memcpy(sSecurityName,securityName,securityNameLen);

    if ( (entry = snmpTargetParamTable_create()) == NULL )
        return SNMP_GEN_ERR;

    if ( snmpTargetParams_addParamName(entry, (char *)sElementName)  == 0 )
    {
        snmpTargetParamTable_dispose(entry);
        return SNMP_GEN_ERR;
    }

    if ( snmpTargetParams_addSecName(entry, (char *)sSecurityName) == 0 )
    {
        snmpTargetParamTable_dispose(entry);
        return SNMP_GEN_ERR;
    }

    entry->mpModel     = mpModel;
    entry->secModel    = secModel;
    entry->secLevel    = secLevel;
    entry->storageType = storageType;
    entry->rowStatus   = SNMP_ROW_ACTIVE;

    DEBUG("AgentTargetParamsTableAddRow, <%s> <%d> <%d> <%s> <%d> <%d> <%d>\n",
          entry->paramName, entry->mpModel,  entry->secModel,
          entry->secName,   entry->secLevel, entry->storageType, entry->rowStatus);

    snmpTargetParamTable_add(entry);
    return SNMP_NO_ERROR;            

}
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
SNMP_STATUS AgentTargetAddrTableAddRow(char * TagName, unsigned char *TargetIpAddr,int Port,int TimeOut,
                            int Retries, unsigned char *ElementName,int ElementNameLen)
{
    oid udpDomain[] = { 1, 3, 6, 1, 6, 1, 1};
    int udpDomainLen = sizeof(udpDomain) / sizeof(oid);
    struct targetAddrTable_struct *entry;
    unsigned char debugBuf[1024];
    char sElementName[32];
    int i;

    memset(sElementName,'\0',32);
    memcpy(sElementName,ElementName,ElementNameLen);

    if ( (entry = snmpTargetAddrTable_create()) == NULL )
        return SNMP_GEN_ERR;

    for ( i = udpDomainLen - 1; i >= 0; i-- )
        entry->tDomain[i] = udpDomain[i];
    entry->tDomainLen = udpDomainLen;

    /* we malloc 7 bytes just in case someone thinks this is a string */
    if ( (entry->tAddress = (unsigned char *)malloc(7)) == NULL )
    {
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
    }
    DumpBuf("AgentTargetAddrTableAddRow input taddress is",TargetIpAddr, 4, 'x');

    memset(entry->tAddress, 0, 7);
    memcpy(entry->tAddress,TargetIpAddr,4);     // address should be 4 bytes of IP addr and
    entry->tAddress[5] = (unsigned char)(Port); // 2 bytes of port
    entry->tAddress[4] = (unsigned char)(Port>>8);
    entry->tAddressLen = 6;
    DumpBuf("AgentTargetAddrTableAddRow entry->taddress is",entry->tAddress, 6, 'x');
    entry->timeout = TimeOut;
    entry->retryCount = Retries;


    if ( snmpTargetAddr_addTagList(entry, TagName) == 0 )
    {
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
    }

    if ( (snmpTargetAddr_addName(entry, (char *)sElementName)) == 0 )
    {
        snmpTargetAddrTable_dispose(entry);
        return SNMP_GEN_ERR;
    }

    if ( (snmpTargetAddr_addParams(entry, (char *)sElementName)) == 0 )
    {
        snmpTargetAddrTable_dispose(entry);
        printf("AgentTargetAddrTableAddRow : did not add Params");
        return SNMP_GEN_ERR;
    }

    entry->storageType = SNMP_STORAGE_VOLATILE;

    entry->rowStatus = SNMP_ROW_ACTIVE;

    sprintf((char *)debugBuf, "snmp_parse_config_targetAddr, read: %s\n",
            entry->name);
    for ( i=0;i<entry->tDomainLen;i++ )
    {
        sprintf((char *)(&debugBuf[strlen((char *)debugBuf)]), ".%d", (int)entry->tDomain[i]);
    }
    sprintf((char *)(&debugBuf[strlen((char *)debugBuf)]), " %s %d %d %s %s %d %d\n",
            entry->tAddress, entry->timeout, entry->retryCount,
            entry->tagList,  entry->params,  entry->storageType,
            entry->rowStatus);
    DEBUG("AgentProvTargetAddrTable %s\n", (char *)debugBuf);

    snmpTargetAddrTable_add(entry);
    return SNMP_NO_ERROR;
}
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
SNMP_STATUS AgentNotifyTableAddRow(char * Name, char * TagName, int NotifyType)
{
    struct snmpNotifyTable_data *nptr;          

    if ( (TagName != NULL ) && (Name != NULL) )
    {
        if ( (strlen(TagName) > 0 ) && (strlen(Name) > 0 ) )
        {
            nptr = (struct snmpNotifyTable_data *) SNMP_MALLOC_STRUCT(snmpNotifyTable_data);
            nptr->snmpNotifyName = (char *) strdup(Name);
            nptr->snmpNotifyNameLen = strlen(Name);
            nptr->snmpNotifyTag = (char *) strdup(TagName);
            nptr->snmpNotifyTagLen = strlen(TagName);
            nptr->snmpNotifyType = NotifyType;
            nptr->snmpNotifyStorageType = ST_VOLATILE;
            nptr->snmpNotifyRowStatus = RS_ACTIVE;

            if ( snmpNotifyTable_add(nptr) != SNMPERR_SUCCESS )
            {
                //clean up first
                return SNMP_GEN_ERR;
            }
        }
    }

    return SNMP_NO_ERROR;
}



