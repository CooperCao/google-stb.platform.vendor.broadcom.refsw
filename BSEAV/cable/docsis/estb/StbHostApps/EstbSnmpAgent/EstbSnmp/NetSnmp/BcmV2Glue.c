/******************************************************************************
 *    (c)2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 *****************************************************************************/

//**************************************************************************
//    Filename: BcmV2glue.c
//    Author:  
//    Creation Date: Moved from NetSnmp library side 11-18-03
//
//**************************************************************************
//    Description:
//
//      		
//    Agent API functions for configuring NetSnmp.
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <sys/types.h>
#if TIME_WITH_SYS_TIME
    #include <sys/time.h>
    #include <time.h>
#else
    #if HAVE_SYS_TIME_H
        #include <sys/time.h>
    #else
        #include <time.h>
    #endif
#endif
#if HAVE_STRING_H
    #include <string.h>
#else
    #include <strings.h>
#endif
#include <ctype.h>
#if HAVE_NETINET_IN_H
    #include <netinet/in.h>
#endif
#if HAVE_UNISTD_H
    #include <unistd.h>
#endif
#if HAVE_WINSOCK_H
    #include <winsock.h>
#endif
#if HAVE_SYS_SOCKET_H
    #include <sys/socket.h>
#endif
#if HAVE_NETDB_H
    #include <netdb.h>
#endif
#if HAVE_STDLIB_H
    #include <stdlib.h>
#endif

/*
 * Stuff needed for getHwAddress(...) 
 */
#ifdef HAVE_SYS_IOCTL_H
    #include <sys/ioctl.h>
#endif
#ifdef HAVE_NET_IF_H
    #include <net/if.h>
#endif

#if HAVE_DMALLOC_H
    #include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>
#include <net-snmp/library/container.h>
#include <net-snmp/library/snmpv3.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/lcd_time.h>
#include <net-snmp/library/scapi.h>
#include <net-snmp/library/keytools.h>
#include <net-snmp/library/lcd_time.h>
#include <net-snmp/library/snmp_secmod.h>
#include <net-snmp/library/snmpusm.h>
#include <net-snmp/library/transform_oids.h>
#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/default_store.h>
#include <net-snmp/library/mt_support.h>
#include <net-snmp/agent/snmp_agent.h>
#include <net-snmp/agent/agent_callbacks.h>
#include <net-snmp/agent/agent_handler.h>
#include <net-snmp/agent/agent_trap.h>
#include <net-snmp/agent/mib_modules.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/snmp_vars.h>

#include <net-snmp/agent/mibgroup/target/snmpTargetParamsEntry.h>
#include <net-snmp/agent/mibgroup/target/snmpTargetAddrEntry.h>
#include <net-snmp/agent/mibgroup/target/target.h>
#include <net-snmp/agent/mibgroup/notification/snmpNotifyTable.h>
#include <net-snmp/agent/mibgroup/notification/snmpNotifyFilterTable.h>
#include <net-snmp/agent/mibgroup/notification/snmpNotifyFilterProfileTable.h>
#include <net-snmp/agent/mibgroup/mibII/vacm_vars.h>  // for vacm_in_view_callback

#include "BcmV2Glue.h"
#include "BrcmDomain.h"
#include "NetsnmpAgnt.h"

// V2 for ObjectId and Varbind types
#include "SnmpType.h"
#include <SnmpApi.h>
#include "SystemTime.h"

#if HAVE_LOCALE_H
    #include <locale.h>
#endif

#undef SEG_FAULT_DUMP

#ifdef SEG_FAULT_DUMP
#include <signal.h>
#include <pthread.h>
#include <ucontext.h>
static void sigsegv_handler (int n, siginfo_t *si, struct ucontext *uc);
#endif


// local functions
netsnmp_session * bcmV2_get_target_sessions(char *taglist, TargetFilterFunction * filterfunct, void *filterArg,  const unsigned char *pFromIp);
int bcmV2_FindViewsFromSecurityName(char *securityName, char **readView, char **writeView,char **notifyView, int secModel, int secLevel);
void bcmV2_free_vb_data (void *pData);



// Some NetSnmp functions that are not prototyped in header files.            
#ifndef HAVE_GETTIMEOFDAY
extern void gettimeofday(struct timeval *tv, struct timezone *tz);
#endif
extern void _init_snmp(void);
extern void init_snmp_enums();
extern  int snmp_build(u_char ** pkt, size_t * pkt_len,size_t * offset, netsnmp_session * pss,netsnmp_pdu *pdu);
//extern char *strdup(const char *oldString);
extern netsnmp_mib_handler *clone_handler(netsnmp_mib_handler *it);
extern int netsnmp_check_requests_status(netsnmp_agent_session *asp, netsnmp_request_info *requests, int look_for_specific);
extern void snmpv3_set_local_snmpEngineBoots(int boots);

extern int trace_level;

/****************************************************************************************/


int bcmV2_dontprintf( const char *format, ... )
{
    return 0;
}

// #define DEBUG printf


extern u_long   engineBoots;
extern unsigned int engineIDType;
extern unsigned char *engineID;
extern size_t   engineIDLength;
extern unsigned char *engineIDNic;
extern unsigned int engineIDIsSet;  /* flag if ID set by config */
extern unsigned char *oldEngineID;
extern size_t   oldEngineIDLength;
extern struct timeval snmpv3starttime;

extern struct usmUser *userList;
extern struct vacm_viewEntry *viewList;
extern struct vacm_accessEntry *accessList;
extern struct vacm_groupEntry *groupList;

extern int snmp_enableauthentraps;

/* Local prototypes */
int bcmV2_init_snmp(const char *type);

void init_bcmV2SessionAndTransport ();
void bcmV2_init_netsnmp_precheck ();
void printTransport (netsnmp_transport *pTransport);


// Global pointers used in call to _sess_process_packet, which is called
// in bcmV2_process_snmp_packet
static netsnmp_session *gpSession;
static netsnmp_transport *gpTransport;

netsnmp_mib_handler *gpNetsnmpPrecheckHandler = NULL;

int gCoreObjects = 0;


/*******************************************************************-o-******
 * bcmV2_set_engineID
 *
 * Parameters:
 *      *buf
 *       buflen
 *
 * Returns:
 *      1       On Success
 *      0       Otherwise.
 *
 *
 * Malloc space for the new engine ID, copy it, and store the new space
 * and the length in engineID and engineIDLen.
 *
 */
int
bcmV2_set_engineID(unsigned char *buf, int bufLen)
{
    u_char *newID;

    /*
     * Sanity check.
     */
    if ( !buf || (bufLen > 1500) || (bufLen < 1) )
    {
        return 0;
    }

    newID = (u_char *) malloc(bufLen);
    if ( !newID )
        return 0;
    SNMP_FREE(engineID);
    memcpy(newID, buf, bufLen);
    engineID = newID;
    engineIDLength = bufLen;
    
    set_enginetime(engineID, engineIDLength,
                   1,    //note: we really set the boots in bcmV2_update_engine_times
                   snmpv3_local_snmpEngineTime(),
                   TRUE);

    return 1;

}  /* end bcmV2_set_engineID() */

/*
 *
 * bcmV2_set_context
 * returns 1 for success, 0 for failure.
 *
 */

int
bcmV2_set_context(unsigned char *buf, int bufLen)
{
    return 1;
}

/*
 *
 * bcmV2_get_context
 * returns 1 for success, 0 for failure.
 *
 */

int
bcmV2_get_context(unsigned char *buf, int bufLen)
{
/* do something here */
    return 1;
}
   
 
/*
 *
 * bcmV2_get_authFailTrapEnabled
 * returns authFailTrapEnabled.
 *
 */

int
bcmV2_get_authFailTrapEnabled(void)
{
    // Translate SNMP 1/2 true/false to 'normal' 1/0
    if (snmp_enableauthentraps == 1)
      return 1;
    else
      return 0;
}


#ifdef NOTNOW
/* Print out all users - FOR DEBUG ONLY!!! */
void
dump_users(void)
{
    struct usmUser *newUser;

    printf("\n\nDumping userList contents\n");
    fflush(stdout);
    for ( newUser = userList; newUser != NULL; newUser = newUser->next )
        if ( newUser->next == NULL )
        {
            printf("\tuser %s is the last one\n\n", newUser->name);
            fflush(stdout);
        }
        else
        {
            printf("\tuser %s is linked to user %s\n", newUser->name, newUser->next->name);
            fflush(stdout);
        }
}
#endif

/* creates a user with slim information passed across vta interface */
int
bcmV2_install_user(unsigned char *userName, int secLevel)
{
    u_char myId[USM_MAX_ID_LENGTH];
    int   myIdLen = snmpv3_get_engineID(myId, USM_MAX_ID_LENGTH);

    // printf("\n\nInstalling user %s\n", userName);
    // fflush(stdout);
    // dump_users();
    struct usmUser *newUser  = usm_create_user();
    if ( newUser == NULL )
        return 0;

    if ( (newUser->name = strdup((char *)userName)) == NULL )
    {
        usm_free_user(newUser);
        return 0;
    }

    if ( (newUser->secName = strdup((char *)userName)) == NULL )
    {
        usm_free_user(newUser);
        return 0;
    }

    newUser->engineID = (u_char *) malloc(myIdLen);
    memcpy(newUser->engineID, myId, myIdLen);
    newUser->engineIDLen = myIdLen;

    if ( secLevel == SNMP_SEC_LEVEL_AUTHPRIV )
    {
        SNMP_FREE(newUser->privProtocol);
        newUser->privProtocol = snmp_duplicate_objid(usmDESPrivProtocol, USM_LENGTH_OID_TRANSFORM);
        newUser->privProtocolLen = USM_LENGTH_OID_TRANSFORM;

        SNMP_FREE(newUser->authProtocol);
        newUser->authProtocol = snmp_duplicate_objid(usmHMACMD5AuthProtocol, USM_LENGTH_OID_TRANSFORM);
        newUser->authProtocolLen = USM_LENGTH_OID_TRANSFORM;

    }
    else if ( secLevel == SNMP_SEC_LEVEL_AUTHNOPRIV )
    {
        SNMP_FREE(newUser->authProtocol);
        newUser->authProtocol = snmp_duplicate_objid(usmHMACMD5AuthProtocol, USM_LENGTH_OID_TRANSFORM);
        newUser->authProtocolLen = USM_LENGTH_OID_TRANSFORM;
    }

    newUser->userStatus = RS_NOTINSERVICE;
    newUser->userStorageType = ST_VOLATILE;

    usm_add_user(newUser);
    // printf("\n\nInstalled user %s\n", userName);
    // fflush(stdout);
    // dump_users();
    return 1;

}

int
bcmV2_enable_user(unsigned char *engineId, int engineIdLen, unsigned char *userName)
{
    struct usmUser *newUser;

    // printf("\n\nEnabling user %s\n", userName);
    // fflush(stdout);
    // dump_users();
    newUser = usm_get_user(engineId, engineIdLen, (char *)userName);
    if ( newUser == NULL )
        return 0;

    newUser->userStatus = RS_ACTIVE;
    // printf("\n\nEnabled user %s\n", userName);
    // fflush(stdout);
    // dump_users();
    return 1;
}

int
bcmV2_disable_user(unsigned char *engineId, int engineIdLen, unsigned char *userName)
{
    struct usmUser *newUser;

    // printf("\n\nDisabling user %s\n", userName);
    // fflush(stdout);
    // dump_users();
    newUser = usm_get_user(engineId, engineIdLen, (char *)userName);
    if ( newUser == NULL )
        return 0;

    newUser->userStatus = RS_NOTINSERVICE;
    // printf("\n\nDisabled user %s\n", userName);
    // fflush(stdout);
    // dump_users();
    return 1;
}

int
bcmV2_destroy_user(unsigned char *engineId, int engineIdLen, unsigned char *userName)
{
    struct usmUser *newUser;

    // printf("\n\nDestroying user %s\n", userName);
    // fflush(stdout);
    // dump_users();
    newUser = usm_get_user(engineId, engineIdLen, (char *)userName);
    if ( newUser == NULL )
        return 0;

    usm_remove_user(newUser);
    usm_free_user(newUser);
    // printf("\n\nDestroyed user %s\n", userName);
    // fflush(stdout);
    // dump_users();
    return 1;
}

struct usmUser *
bcmV2_usm_get_user_from_list(unsigned char *engineID, size_t engineIDLen,
                             struct usmUser *puserList)
{
    struct usmUser *ptr;

    for ( ptr = puserList; ptr != NULL; ptr = ptr->next )
    {
        if ( ptr->engineIDLen == engineIDLen &&
             (ptr->engineID != NULL && engineID != NULL &&
              memcmp(ptr->engineID, engineID, engineIDLen) == 0) )
            return ptr;
    }
    return NULL;
}

void
bcmV2_destroy_all_users(unsigned char *engineId, int engineIdLen)
{
    struct usmUser *newUser;

    // printf("\n\nDestroying all bcmV2 users\n");
    // fflush(stdout);
    // dump_users();
    /* this assumes that BCMv2 users all have the same engineId,
       which they should */
    newUser = bcmV2_usm_get_user_from_list(engineId, engineIdLen, userList);
    while ( newUser != NULL )
    {
        usm_remove_user(newUser);
        usm_free_user(newUser);
        newUser = bcmV2_usm_get_user_from_list(engineId, engineIdLen, userList);
    }
    // printf("\n\nDestroyed all bcmV2 users\n");
    // fflush(stdout);
    // dump_users();

}

/* Get all the users with this engine Id that have a name beginning "docsis" */
int
bcmV2_get_docsis_users(unsigned char *engineId, int engineIdLen, unsigned char *buf)
{
    struct usmUser *newUser;
    int bufLen = 0;
    int *userCount, nameLen;

    /* the userCount comes first in the buffer, but we don't know what it is yet */
    userCount = (int *) buf;
    *userCount = 0;
    buf += sizeof(userCount);
    bufLen += sizeof(userCount);

    for ( newUser = userList; newUser != NULL; newUser = newUser->next )
    {
        if ( newUser->engineIDLen == engineIdLen &&
             (newUser->engineID != NULL && engineId != NULL &&
              memcmp(newUser->engineID, engineId, engineIdLen) == 0) &&
             memcmp(newUser->name, "docsis", 6) == 0 )
        {

            /* got a hit! add name length to buffer first */
            nameLen = strlen(newUser->name);
            memcpy(buf, &nameLen, sizeof(nameLen));
            buf += sizeof(nameLen);
            bufLen += sizeof(nameLen);

            /* add name to buffer */
            memcpy(buf, newUser->name, nameLen);
            buf += nameLen;
            bufLen += nameLen;

            /* and increment user counter */
            (*userCount)++;

        }
    }

    return bufLen;
}

int
bcmV2_set_userKey(unsigned char *engineId, int engineIdLen, unsigned char *userName,
                  int keyId, unsigned char *key, int keyLen)
{
    struct usmUser *newUser;

    newUser = usm_get_user(engineId, engineIdLen, (char *)userName);
    if ( newUser == NULL )
    {
        DEBUGMSGTL(("set_bcmV2_user_key", "couldn't find user %s\n", userName));
        return 0;
    }

    if ( keyId == 1 )
    {
        if ( (newUser->authKey = (u_char *) malloc(keyLen)) != NULL )
        {
            newUser->authKeyLen = keyLen;
            memcpy(newUser->authKey, key, keyLen);
            DEBUGMSGTL(("set_bcmV2_user_key", "Setting MD5 auth key for user %s.\n", userName));
        }
        else
        {
            DEBUGMSGTL(("set_bcmV2_user_key", "Couldn't malloc for auth key.\n"));
            return 0;
        }
    }
    else if ( keyId == 2 )
    {
        if ( (newUser->privKey = (u_char *) malloc(keyLen)) != NULL )
        {
            newUser->privKeyLen = keyLen;
            memcpy(newUser->privKey, key, keyLen);
            DEBUGMSGTL(("set_bcmV2_user_key", "Setting DES priv key for user %s.\n", userName));
        }
        else
        {
            DEBUGMSGTL(("set_bcmV2_user_key", "Couldn't malloc for priv key.\n"));
            return 0;
        }
    }
    else
    {
        DEBUGMSGTL(("set_bcmV2_user_key", "Unknown key ID %d. Should be MD5 (1) or DES (2).\n", keyId));
        return 0;
    }
    return 1;
}


void
bcmV2_destroy_all_views (void)
{
    struct vacm_viewEntry *vp, *lastvp = NULL;
    char namePrefix[] = "anonymous";

    vp = viewList;
    while ( vp )
    {
        if ( !memcmp(vp->viewName+1, (char *) namePrefix, strlen(namePrefix)) )
        {
            lastvp = vp;
            vp = vp->next;
        }
        else
        {
            if ( vp == viewList )
            {    /* remove head of list */
                viewList = lastvp = vp->next;
                if ( vp->reserved )
                    free(vp->reserved);
                free(vp);
                vp = viewList;
            }
            else
            {
                lastvp->next = vp->next;
                if ( vp->reserved )
                    free(vp->reserved);
                free(vp);
                vp = lastvp->next;
            }
        }
    }
}

void
bcmV2_destroy_all_groups (void)
{
    struct vacm_groupEntry *gp, *lastgp = NULL;
    char namePrefix[] = "anonymous";

    // printf("\n\ndestroy_all_bcmV2_groups() starting\n");
    // dump_groups();
    gp = groupList;
    while ( gp )
    {
        if ( !memcmp(gp->groupName, (char *) namePrefix, strlen(namePrefix)) )
        {
            lastgp = gp;
            gp = gp->next;
        }
        else
        {
            if ( gp == groupList )
            {    /* remove head of list */
                groupList = lastgp = gp->next;
                if ( gp->reserved )
                    free(gp->reserved);
                free(gp);
                gp = groupList;
            }
            else
            {
                lastgp->next = gp->next;
                if ( gp->reserved )
                    free(gp->reserved);
                free(gp);
                gp = lastgp->next;
            }
        }
    }
    // printf("\n\ndestroy_all_bcmV2_groups() done\n");
    // dump_groups();
}

void
bcmV2_destroy_all_accessEntries (void)
{
    struct vacm_accessEntry *ap, *lastap = NULL;
    char namePrefix[] = "anonymous";

    // printf("destroy_all_bcmV2_accessEntries: starting\n");
    // fflush(stdout);

    ap = accessList;
    while ( ap )
    {
        if ( !memcmp(ap->groupName+1, (char *) namePrefix, strlen(namePrefix)) )
        {
            lastap = ap;
            ap = ap->next;
        }
        else
        {
            if ( ap == accessList )
            {    /* remove head of list */
                accessList = lastap = ap->next;
                if ( ap->reserved )
                    free(ap->reserved);
                free(ap);
                ap = accessList;
            }
            else
            {
                lastap->next = ap->next;
                if ( ap->reserved )
                    free(ap->reserved);
                free(ap);
                ap = lastap->next;
            }
        }
    }
}


extern struct header_complex_index *snmpNotifyTableStorage;
struct header_complex_index
{
    oid            *name;
    size_t          namelen;
    void           *data;
    struct header_complex_index *next;
    struct header_complex_index *prev;
};

int
send_bcmV2_notifications(struct snmp_pdu *template_pdu, const unsigned char *pFromIp)
{
    struct header_complex_index *hptr;
    struct snmpNotifyTable_data *nptr;
    struct snmp_session *sess, *sptr;
    

    //printf("send_bcmV2_notifications: starting\n");
    
    for ( hptr = snmpNotifyTableStorage; hptr; hptr = hptr->next )
    {
        DEBUG("send_bcmV2_notifications: processing Notify Table storage\n");
        nptr = (struct snmpNotifyTable_data *) hptr->data;
        if ( nptr->snmpNotifyRowStatus != RS_ACTIVE )
            continue;
        if ( !nptr->snmpNotifyTag )
            continue;

        DEBUG("send_bcmV2_notifications: getting target sessions\n");
        sess = bcmV2_get_target_sessions(nptr->snmpNotifyTag, (TargetFilterFunction *)NULL, (void*)NULL, pFromIp);


        if ( sess == NULL )
        {
            DEBUG("send_bcmV2_notifications: no target sessions found\n");
            continue;
        }
        else
            DEBUG("send_bcmV2_notifications: got target sessions\n");


        /* XXX: filter appropriately */

        for ( sptr=sess; sptr; sptr = sptr->next )
        {
            DEBUG("send_bcmV2_notifications: target sessions %p\n", sptr);
            if ( sptr->version == SNMP_VERSION_1 )
            {
                DEBUG("send_bcmV2_notifications: sending a V1 trap on sess %p\n", sptr);
                template_pdu->command = SNMP_MSG_TRAP;

                //We had assumed v2c or v3 traps when we created the pdu.
                //Now we need to remove the systime and trapoid varbinds.
                DEBUG("V1 Trap - remove the systime and trapoid varbinds\n");
                template_pdu->variables = template_pdu->variables->next_variable;
                template_pdu->variables = template_pdu->variables->next_variable;
                
                send_trap_to_sess(sptr, template_pdu);
            }
            else
            {
                template_pdu->reqid = snmp_get_next_reqid();
                template_pdu->msgid = snmp_get_next_msgid();
                if ( nptr->snmpNotifyType == SNMPNOTIFYTYPE_INFORM )
                {
                    DEBUG("send_bcmV2_notifications: sending an inform on sess %p\n", sptr);
                    template_pdu->command = SNMP_MSG_INFORM;
                }
                else
                {
                    DEBUG("send_bcmV2_notifications: sending a V2 trap on sess %p\n", sptr);
                    template_pdu->command = SNMP_MSG_TRAP2;
                }
                send_trap_to_sess(sptr, template_pdu);
            }
        }
    }
    //printf("send_bcmV2_notifications: Done processing Notify Table storage\n");
    return 1;
}
/* returns 0 on success, -1 on failure */
//This is similar to bcmV2_send_V2_trap

int bcmV2_send_notify (const unsigned char *pFromIp, 
                       oid *pTrapOID, int TrapOIDLength, time_t sysUpTime,
                       netsnmp_variable_list *vars)
{

    static oid sysUpTime_oid[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
    static oid snmpTrapOID_oid[] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};
    netsnmp_variable_list uptime_var, snmptrap_var;
    netsnmp_variable_list *v2_vars;
    netsnmp_pdu    *template_pdu;
    in_addr_t      *pdu_in_addr_t;
    int trapType   = 0;
    oid             trap_prefix[] = { 1, 3, 6, 1, 6, 3, 1, 1, 5 };

    /*
     * Initialise SNMPv2 required variables: sysUpTime and trap OID
     */

    /* sysUpTime */

    memset(&uptime_var, 0, sizeof(netsnmp_variable_list));
    snmp_set_var_objid(&uptime_var, sysUpTime_oid, OID_LENGTH(sysUpTime_oid));
    snmp_set_var_value(&uptime_var, (u_char *) &sysUpTime, sizeof(sysUpTime));
    uptime_var.type = ASN_TIMETICKS;
    uptime_var.next_variable = &snmptrap_var;

    /* trap OID */

    memset(&snmptrap_var, 0, sizeof(netsnmp_variable_list));
    snmp_set_var_objid(&snmptrap_var, snmpTrapOID_oid, OID_LENGTH(snmpTrapOID_oid));
    snmp_set_var_value(&snmptrap_var, (u_char *) pTrapOID, TrapOIDLength * sizeof(oid));
    snmptrap_var.type = ASN_OBJECT_ID;

    /* Now attach VB list */

    if ( vars )
    {
        DEBUG ("  Attaching variable list @ %p\n", vars);
        snmptrap_var.next_variable = vars;
    }
    else
    {
        DEBUG ("  Notify -No variable list.\n");
        snmptrap_var.next_variable = NULL;
    }

    v2_vars = &uptime_var;

    /*
     *  Create a template PDU, ready for sending
     */

    template_pdu = snmp_pdu_create(SNMP_MSG_TRAP);//INFORM);
    if ( template_pdu == NULL )
    {
        DEBUG ("  snmp_pdu_create returned NULL!\n");
        return -1;
    }
    else
        DEBUG ("  got PDU @ %p\n", template_pdu);



    /*
    * Is this a 'standard' trap?
    *  Or at least, does it have the correct prefix?
    */
    //printf("\n pTrapOid = ");
    //dumpUCDOid(pTrapOID, TrapOIDLength);
    //printf("\n trap prefix = ");
    //dumpUCDOid(trap_prefix, OID_LENGTH(trap_prefix));
    
     
    if (netsnmp_oid_equals(pTrapOID, OID_LENGTH(trap_prefix),
                         trap_prefix, OID_LENGTH(trap_prefix)) == 0)
    {
        //printf("standard trap\n");
        trapType = ((int)(pTrapOID[TrapOIDLength-1]))-1;
    }
    else
    { 
       DEBUG("trapType was <%d> must be enterprise so force traptype to a 6\n",trapType);
       trapType = 6;
    }
    DEBUG("TRAP TYPE = <%d> \n",trapType);
    
    /* UNFINISHED: What generic and specific values should we use for SNMPv2? */
    //This only matters if it turns out to be a V1 trap.
    template_pdu->trap_type = trapType; //1;//-1;
    template_pdu->specific_type = 0;

    /* No enterprise info for SNMPv2 trap */
    //template_pdu->enterprise = NULL;
    //template_pdu->enterprise_length = 0;

    template_pdu->enterprise = pTrapOID;
    template_pdu->enterprise_length = (TrapOIDLength - 2);//OID_LENGTH(snmpTrapOID_oid);
    
    /* UNFINISHED: do I need this since we didn't clone enterprise? */    
    template_pdu->flags |= UCD_MSG_FLAG_FORCE_PDU_COPY;

    /* Load local addr from supplied args */
    pdu_in_addr_t = (in_addr_t *) template_pdu->agent_addr;
    *pdu_in_addr_t = htonl(*pFromIp);
    template_pdu->time = sysUpTime;
    template_pdu->variables = v2_vars;


    send_bcmV2_notifications ( template_pdu, pFromIp);

    // Set variables to NULL so they won't get called below.
    // The 'vars' list will be freed in NotifyAgnt.c where it was created.
    template_pdu->variables = NULL;
    
    // Clear enterprise so we don't free it, since it's being used directly
    // instead of copied.
    template_pdu->enterprise = NULL;
    template_pdu->enterprise_length = 0;

    // Free the PDU
    DEBUG("  freeing INFORM PDU\n");
    snmp_free_pdu(template_pdu);

    return 0;
}


int extractV2VarBind(VarBind *v2VarBindPtr, void **valPtr)
{
    unsigned long int *longPtr;
    int length;
    void *value = NULL;

    switch ( v2VarBindPtr->Type )
    {
    case VBT_COUNTER:
    case VBT_GAUGE:
    case VBT_TIMETICKS:
    case VBT_OPAQUE:
    case VBT_UINTEGER32:
    case VBT_NUMBER:
        // Allocate storage:
        *valPtr = (char *) malloc (sizeof(unsigned long int));
        value = *valPtr;
    
        longPtr = (unsigned long int *) value;
        *longPtr = VARBIND_GET_INT32(v2VarBindPtr);
        //printf("extractV2VarBind got value = %ld\n", *longPtr);
        return(sizeof(unsigned long int));
        break;
        //case VBT_BITS:
    case VBT_STRING:
        length = (VARBIND_GET_STRING(v2VarBindPtr))->Length;
        
        // Allocate storage:
        *valPtr = (char *) malloc (length);
        value = *valPtr;
        
        memcpy(value, (VARBIND_GET_STRING(v2VarBindPtr))->pOctets, length);
        return(length);
        break;
    case VBT_OBJECT:
        length = (VARBIND_GET_OID(v2VarBindPtr))->Length * sizeof(unsigned long);
        
        // Allocate storage:
        *valPtr = (char *) malloc (length);
        value = *valPtr;
        
        memcpy(value, (VARBIND_GET_OID(v2VarBindPtr))->pObjects, length);
        return(length);
        break;
    case VBT_IPADDRESS:
        // Allocate storage:
        *valPtr = (char *) malloc (sizeof(unsigned long int));
        value = *valPtr;
    
        VARBIND_GET_IPADDR(v2VarBindPtr, value);
        return(sizeof(unsigned long int));
        break;
    case VBT_COUNTER64:
        length = 8;
        
        // Allocate storage:
        *valPtr = (char *) malloc (length);
        value = *valPtr;
        
        memcpy (value, &v2VarBindPtr->Value.C64.Hi, 4);
        memcpy ((void*)((char*)value + 4), &v2VarBindPtr->Value.C64.Lo, 4);
        return(length);
        break;

    default:
        // printf("extractV2VarBind: Type UNKNOWN %x\n", VARBIND_GET_TYPE(v2VarBindPtr));
        *valPtr = NULL;
        return -1;
    }
}

void dumpV2String(OctetString *v2StringPtr)
{
#ifndef BCM_NULL_CONSOLE
    int i;
    unsigned char *octetPtr;

/*  PR 5404: no max string size anymore.  Really I guess I should define
    this as 65535, but I am being lazy and assuming nobody's going to be
    hitting us with a string that big...
    if((v2StringPtr->Length > MAX_STRING_SIZE) && (v2StringPtr->Length < 0)) {
        // printf("v2 String: Bad Length %d, ", v2StringPtr->Length);
        return;
    }
*/
    octetPtr = v2StringPtr->pOctets;
    printf("v2 String Length = %d, ", v2StringPtr->Length);
    for ( i = v2StringPtr->Length; i > 0; i--, octetPtr++ )
    {
        printf("%x ", *octetPtr);
    }
    printf("= ");
    for ( i = v2StringPtr->Length; i > 0; i--, octetPtr++ )
    {
        printf("%c", *((char *) octetPtr));
    }
    printf("\n");
#endif
}

void dumpV2Oid(ObjectId *v2OidPtr)
{
#ifndef BCM_NULL_CONSOLE
    int i;
    unsigned long *subIdPtr;

    if ( (v2OidPtr->Length > MAX_OID_SIZE) && (v2OidPtr->Length < 0) )
    {
        printf("v2 OID: Bad Length %d, ", v2OidPtr->Length);
        return;
    }
    printf("v2 OID Length = %d, ", v2OidPtr->Length);
    subIdPtr = v2OidPtr->pObjects;
    for ( i = v2OidPtr->Length; i > 0; i--, subIdPtr++ )
    {
        printf(".%ld", *subIdPtr);
    }
    printf("\n");
#endif
}

void dumpV2VarBind(VarBind *v2VarBindPtr)
{
#ifndef BCM_NULL_CONSOLE

    unsigned long int ipAddr;

    switch ( v2VarBindPtr->Type )
    {
    case VBT_NUMBER:
        printf("v2 VarBind: Type INTEGER, Value %ld\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
        //case VBT_BITS:
    case VBT_STRING:
        printf("v2 VarBind: Type STRING/BITS, ");
        dumpV2String(VARBIND_GET_STRING(v2VarBindPtr));
        break;
    case VBT_OBJECT:
        printf("v2 VarBind: Type OBJECT ID, ");
        dumpV2Oid(VARBIND_GET_OID(v2VarBindPtr));
        break;
    case VBT_EMPTY:
        printf("v2 VarBind: Type EMPTY\n");
        break;
    case VBT_IPADDRESS:
        VARBIND_GET_IPADDR(v2VarBindPtr, &ipAddr);
        printf("v2 VarBind: Type IPADDRESS, Value %lx\n", ipAddr);
        break;
    case VBT_COUNTER:
        printf("v2 VarBind: Type COUNTER, Value %ld\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
        //case VBT_UNSIGNED32:
    case VBT_GAUGE:
        printf("v2 VarBind: Type GAUGE/UNSIGNED32, Value %ld\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
    case VBT_TIMETICKS:
        printf("v2 VarBind: Type TIMETICKS, Value %ld\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
    case VBT_OPAQUE:
        printf("v2 VarBind: Type OPAQUE, Value %ld\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
        //case VBT_COUNTER64:
        //printf("v2 VarBind: Type COUNTER64, Value %d\n", VARBIND_GET_INT32(v2VarBindPtr));
        //break;
    case VBT_UINTEGER32:
        printf("v2 VarBind: Type UINTEGER32, Value %ld\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
    default:
        printf("v2 VarBind: Type UNKNOWN %x", VARBIND_GET_TYPE(v2VarBindPtr));
    }
#endif
}

void buildV2VarBind(VarBind *v2VarBindPtr, u_char *value, size_t valueLen, u_char type)
{

    switch ( type )
    {
    case ASN_INTEGER:
        VARBIND_SET_INT32(v2VarBindPtr, *((long *) value));
        break;
    case ASN_OCTET_STR:
        // PR 5404: OctetString.pOctets is a pointer not a fixed length field
        // now, so we can't memcpy into it, which is what VARBIND_SET_STRING
        // does.  Use the value pointer directly instead.
        // VARBIND_SET_STRING(v2VarBindPtr, value, valueLen);
        v2VarBindPtr->Type = VBT_STRING;
        v2VarBindPtr->Value.String.Length = valueLen;
        v2VarBindPtr->Value.String.pOctets = value;
        break;
    case ASN_OBJECT_ID:
        memcpy(v2VarBindPtr->Value.OID.pObjects, value,
               valueLen);
        //PR6497: at this point valueLen is already numObjects * 4
        //memcpy(v2VarBindPtr->Value.OID.pObjects, value,
        //       valueLen * sizeof(unsigned long));
        
        v2VarBindPtr->Value.OID.Length = valueLen;
        v2VarBindPtr->Type = VBT_OBJECT;
        break;
    case ASN_NULL:
        VARBIND_SET_EMPTY(v2VarBindPtr);
        break;
    case ASN_IPADDRESS:
        VARBIND_SET_IPADDR(v2VarBindPtr, value);
        break;
    case ASN_COUNTER:
    case ASN_GAUGE:               // same as ASN_UNSIGNED:
    case ASN_TIMETICKS:
        VARBIND_SET_UINT32(v2VarBindPtr, *((unsigned long *) value), type);
        break;
    // default:
        // printf("buildV2BarBind: Type %x is not supported", type);
    }
}

void dumpUCDOid(oid *oidPtr, int oidLen)
{
#ifndef BCM_NULL_CONSOLE
    int i;

    if ( (oidLen > MAX_OID_SIZE) && (oidLen < 0) )
    {
        printf("UCD OID: Bad Length %d, ", oidLen);
        return;
    }
    printf("UCD OID Length = %d, ", oidLen);
    for ( i = oidLen; i > 0; i--, oidPtr++ )
    {
        printf(".%ld", *oidPtr);
    }
    printf("\n");
#endif
}


// This call will allocate as much space as necessary for value.  The caller
// must free it!
int v2GetObject (netsnmp_variable_list *requestvb, char **value, size_t *valueLen, netsnmp_agent_request_info *reqInfo, int indexOffset, NetSnmpRawApiRec *pRaw)
{
    ObjectId v2Oid, *v2OidPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    //struct snmp_session *sessPtr;
    char *pCommunity;
    SNMP_STATUS retVal = SNMP_NO_ERROR;

    DEBUG("v2GetObject: starting\n");
    //fflush(stdout);
    v2OidPtr = &v2Oid;
    v2VarBindPtr = &v2VarBind;

    memset (v2VarBindPtr, 0, sizeof (VarBind));
    v2VarBindPtr->pMisc = pRaw;

    v2OidPtr->Length = requestvb->name_length;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) requestvb->name, requestvb->name_length * sizeof(oid));
    
    v2OidPtr->IndexOffset = indexOffset;
    
    if ((indexOffset > 0) && (indexOffset < v2OidPtr->Length))
      v2OidPtr->Leaf = v2OidPtr->pObjects[indexOffset-1];
    else if (v2OidPtr->Length > 0)
      v2OidPtr->Leaf = v2OidPtr->pObjects[v2OidPtr->Length-1];
    else
      v2OidPtr->Leaf = UNSPECIFIED_LEAF;
      
    DEBUG ("  IndexOffset = %d\n", v2OidPtr->IndexOffset);

    pduPtr = reqInfo->asp->pdu;
    if ( pduPtr == NULL )
    {
        // printf("appGet: reqInfo->pdu is NULL! Should never happen! \n");
        fflush(stdout);
        return SNMP_ERR_GENERR;
    }

    // Allocate and copy community string.  It will be freed later.
    pCommunity = (char *) malloc (pduPtr->community_len + 1);
    memset (pCommunity, 0, pduPtr->community_len + 1);
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);

    retVal = AppGetObject( v2OidPtr, v2VarBindPtr, gpTransport->local,
                           gpTransport->remote, pCommunity, pduPtr->version);

    free (pCommunity);

    if ( retVal == SNMP_NO_ERROR )
    {

        /* get returned value.  Note that extractV2VarBind will allocate
           storage.  It's up to the caller of this function to free it. */
        *valueLen = extractV2VarBind(v2VarBindPtr, (void **)value);
        if ( *valueLen == -1 )
            retVal = SNMP_ERR_GENERR;
        else
            // Load type from V2 varbind
            requestvb->type = v2VarBindPtr->Type;
    }

    // If the app has allocated storage in C++ land, free it now.
    ExpungeVarBind (v2VarBindPtr);

    return retVal;
}


int v2GetNextObject (netsnmp_variable_list *requestvb, int indexOffset, char **value, size_t *valueLen, netsnmp_agent_request_info *reqInfo, NetSnmpRawApiRec *pRaw)
{
    ObjectId v2Oid, *v2OidPtr, v2Index, *v2IndexPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    char *pCommunity;
    SNMP_STATUS retVal = SNMP_NO_ERROR;


    DEBUG("v2GetNextObject: Starting - indexOffset = %d\n", indexOffset);

    v2OidPtr = &v2Oid;
    v2IndexPtr = &v2Index;
    v2VarBindPtr = &v2VarBind;

    memset (v2VarBindPtr, 0, sizeof (VarBind));
    v2VarBindPtr->pMisc = pRaw;
    
    if (indexOffset <= requestvb->name_length)
      v2OidPtr->Length = indexOffset;
    else
      v2OidPtr->Length = requestvb->name_length;
      
    memcpy( (char *) (v2OidPtr->pObjects),(char *) requestvb->name, v2OidPtr->Length * sizeof(oid));
    
    v2OidPtr->IndexOffset = indexOffset;
    
    if ((indexOffset > 0) && (indexOffset < v2OidPtr->Length))
      v2OidPtr->Leaf = v2OidPtr->pObjects[indexOffset-1];
    else if (v2OidPtr->Length > 0)
      v2OidPtr->Leaf = v2OidPtr->pObjects[v2OidPtr->Length-1];
    else
      v2OidPtr->Leaf = UNSPECIFIED_LEAF;
      
    v2IndexPtr->Length = requestvb->name_length - indexOffset;
    
    if (v2IndexPtr->Length < 0)
      v2IndexPtr->Length = 0;
      
    memcpy( (char *) (v2IndexPtr->pObjects), (char *) (requestvb->name + indexOffset),
            ((int) v2IndexPtr->Length) * sizeof(oid));
    v2IndexPtr->IndexOffset = 0;

    pduPtr = reqInfo->asp->pdu;
    if ( pduPtr == NULL )
    {
        return SNMP_ERR_GENERR;
    }
    
    // Allocate and copy community string.  It will be freed later.
    pCommunity = (char *) malloc (pduPtr->community_len + 1);
    memset (pCommunity, 0, pduPtr->community_len + 1);
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);

    retVal = AppNextObject ( v2OidPtr, v2IndexPtr, v2VarBindPtr,
                             gpTransport->local, gpTransport->remote, pCommunity,
                             pduPtr->version);

    free (pCommunity);
    
    if ( retVal == SNMP_NO_ERROR )
    {
        DEBUG ("  v2GetNextObject: NOERROR\n");
        
        /* Adopt the OID and index returned by the app. */
        
        // OID base
        memcpy( (char *) (requestvb->name), (char *) (v2OidPtr->pObjects),
                ((int) v2OidPtr->Length) * sizeof(oid));
        // Index
        memcpy( (char *) (requestvb->name + indexOffset), (char *) (v2IndexPtr->pObjects),
                ((int) v2IndexPtr->Length) * sizeof(oid));
        // Length
        requestvb->name_length = v2OidPtr->Length + v2IndexPtr->Length;

        /* get returned value.  Note that extractV2VarBind will allocate
           storage.  It's up to the caller of this function to free it. */
        *valueLen = extractV2VarBind(v2VarBindPtr, (void **)value);

        if ( *valueLen == -1 )
            retVal = SNMP_ERR_GENERR;
        else
            // Load type from V2 varbind
            requestvb->type = v2VarBindPtr->Type;
    }
    
    // In case V2/BFC has changed the OID for us, copy the returned OID 
    // back over the original OID.
    memcpy((char *) requestvb->name,
           (char *) (v2OidPtr->pObjects), 
           indexOffset * sizeof(oid));

    // If the app has allocated storage in C++ land, free it now.
    ExpungeVarBind (v2VarBindPtr);
 
    return retVal;
}


SNMP_STATUS v2Set(oid *oidPtr, int oidLen, u_char *value, size_t valueLen, u_char type, netsnmp_agent_request_info *reqInfo, int indexOffset, NetSnmpRawApiRec *pRaw)
{
    ObjectId v2Oid, *v2OidPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    char *pCommunity;
    SNMP_STATUS retVal = SNMP_NO_ERROR;

    DEBUG("v2Set: Starting\n");
    fflush(stdout);
    v2OidPtr = &v2Oid;
    v2VarBindPtr = &v2VarBind;

    DEBUG("v2Set: calling VARBIND_SET_EMPTY() \n");
    fflush(stdout);
    memset (v2VarBindPtr, 0, sizeof (VarBind));
    v2VarBindPtr->pMisc = pRaw;

    DEBUG("v2Set: dereferencing oidLen\n");
    fflush(stdout);
    
    v2OidPtr->Length = oidLen;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) oidPtr, oidLen * sizeof(oid));
    
    v2OidPtr->IndexOffset = indexOffset;
    
    if ((indexOffset > 0) && (indexOffset < v2OidPtr->Length))
      v2OidPtr->Leaf = v2OidPtr->pObjects[indexOffset-1];
    else if (v2OidPtr->Length > 0)
      v2OidPtr->Leaf = v2OidPtr->pObjects[v2OidPtr->Length-1];
    else
      v2OidPtr->Leaf = UNSPECIFIED_LEAF;

    DEBUG("v2Set: calling buildV2VarBind() \n");
    fflush(stdout);
    buildV2VarBind(v2VarBindPtr, value, valueLen, type);

    pduPtr = reqInfo->asp->pdu;
    if ( pduPtr == NULL )
    {
        // printf("v2Set: reqInfo->pdu is NULL! Should never happen! \n");
        fflush(stdout);
        return SNMP_ERR_GENERR;
    }

    // Allocate and copy community string.  It will be freed later.
    pCommunity = (char *) malloc (pduPtr->community_len + 1);
    memset (pCommunity, 0, pduPtr->community_len + 1);
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);

    DEBUG("v2Set: calling AppSetObject() with \n");
    if ( trace_level > 0 )
    {
        dumpV2Oid(v2OidPtr);
        dumpV2VarBind(v2VarBindPtr);
    }

    retVal = AppSetObject (v2OidPtr, v2VarBindPtr, gpTransport->local, gpTransport->remote,
                           pCommunity, pduPtr->version);

    free (pCommunity);

    if ( retVal != SNMP_NO_ERROR )
    {
        DEBUG("v2Set:  failed with snmp error code %d\n", retVal);
        fflush(stdout);
    }
    else
    {
        DEBUG("v2Set:  success!\n");
        fflush(stdout);
    }

    return retVal;
}


SNMP_STATUS v2Test(oid *oidPtr, int oidLen, u_char *value, size_t valueLen, u_char type, netsnmp_agent_request_info *reqInfo, int indexOffset, NetSnmpRawApiRec *pRaw)
{
    ObjectId v2Oid, *v2OidPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    char *pCommunity;
    SNMP_STATUS retVal = SNMP_NO_ERROR;

    DEBUG("v2Test: Starting\n");
    fflush(stdout);
    v2OidPtr = &v2Oid;
    v2VarBindPtr = &v2VarBind;

    DEBUG("v2Test: calling VARBIND_SET_EMPTY() \n");
    
    fflush(stdout);
    memset (v2VarBindPtr, 0, sizeof (VarBind));
    v2VarBindPtr->pMisc = pRaw;

    DEBUG("v2Test: dereferencing oidLen\n");
    fflush(stdout);
    
    v2OidPtr->Length = oidLen;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) oidPtr, oidLen * sizeof(oid));
    
    v2OidPtr->IndexOffset = indexOffset;
    
    if ((indexOffset > 0) && (indexOffset < v2OidPtr->Length))
      v2OidPtr->Leaf = v2OidPtr->pObjects[indexOffset-1];
    else if (v2OidPtr->Length > 0)
      v2OidPtr->Leaf = v2OidPtr->pObjects[v2OidPtr->Length-1];
    else
      v2OidPtr->Leaf = UNSPECIFIED_LEAF;

    DEBUG("v2Test: calling buildV2VarBind() \n");
    fflush(stdout);
    buildV2VarBind(v2VarBindPtr, value, valueLen, type);

    pduPtr = reqInfo->asp->pdu;
    if ( pduPtr == NULL )
    {
        // printf("v2Test: reqInfo->pdu is NULL! Should never happen! \n");
        fflush(stdout);
        return SNMP_ERR_GENERR;
    }

    // Allocate and copy community string.  It will be freed later.
    pCommunity = (char *) malloc (pduPtr->community_len + 1);
    memset (pCommunity, 0, pduPtr->community_len + 1);
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);

    DEBUG("v2Test: calling AppTestObject() with \n");
    if ( trace_level > 0 )
    {
        dumpV2Oid(v2OidPtr);
        dumpV2VarBind(v2VarBindPtr);
    }

    retVal = AppTestObject (v2OidPtr, v2VarBindPtr, gpTransport->local, gpTransport->remote,
                            pCommunity, pduPtr->version);

    free (pCommunity);

    if ( retVal != SNMP_NO_ERROR )
    {
        DEBUG("v2Test:  failed with snmp error code %d\n", retVal);
        fflush(stdout);
    }
    else
    {
        DEBUG("v2Test:  success!\n");
        fflush(stdout);
    }

    return retVal;
}

SNMP_STATUS v2ValidateSet(oid *oidPtr, int oidLen, u_char *value, size_t valueLen, u_char type, netsnmp_agent_request_info *reqInfo, int indexOffset)
{
    ObjectId v2Oid, *v2OidPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    char *pCommunity;
    SNMP_STATUS retVal = SNMP_NO_ERROR;

    DEBUG("v2ValidateSet\n");
    fflush(stdout);
    v2OidPtr = &v2Oid;
    v2VarBindPtr = &v2VarBind;

    DEBUG("  calling VARBIND_SET_EMPTY() \n");
    fflush(stdout);
    memset (v2VarBindPtr, 0, sizeof (VarBind));

    DEBUG("  dereferencing oidLen\n");
    fflush(stdout);
    
    v2OidPtr->Length = oidLen;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) oidPtr, oidLen * sizeof(oid));
    
    v2OidPtr->IndexOffset = indexOffset;
    
    if ((indexOffset > 0) && (indexOffset < v2OidPtr->Length))
      v2OidPtr->Leaf = v2OidPtr->pObjects[indexOffset-1];
    else if (v2OidPtr->Length > 0)
      v2OidPtr->Leaf = v2OidPtr->pObjects[v2OidPtr->Length-1];
    else
      v2OidPtr->Leaf = UNSPECIFIED_LEAF;

    DEBUG("  calling buildV2VarBind() \n");
    fflush(stdout);
    buildV2VarBind(v2VarBindPtr, value, valueLen, type);

    pduPtr = reqInfo->asp->pdu;
    if ( pduPtr == NULL )
    {
        // printf("  reqInfo->pdu is NULL! Should never happen! \n");
        fflush(stdout);
        return SNMP_ERR_GENERR;
    }

    // Allocate and copy community string.  It will be freed later.
    pCommunity = (char *) malloc (pduPtr->community_len + 1);
    memset (pCommunity, 0, pduPtr->community_len + 1);
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);

    DEBUG("  calling AppValidateSetRequest() with \n");
    if ( trace_level > 0 )
    {
        dumpV2Oid(v2OidPtr);
        dumpV2VarBind(v2VarBindPtr);
    }

    retVal = AppValidateSetRequest (v2OidPtr, v2VarBindPtr, gpTransport->local, gpTransport->remote,
                                    pCommunity, pduPtr->version);

    free (pCommunity);

    if ( retVal != SNMP_NO_ERROR )
    {
        DEBUG("  failed with snmp error code %d\n", retVal);
        fflush(stdout);
    }
    else
    {
        DEBUG("  success!\n");
        fflush(stdout);
    }

    return retVal;
}

/*******************************************************************************
// FUNCTION:   bcmV2_input_parameters
//                      
// PURPOSE:    Sets the same options that could be used on the command line of the 
//             NetSnmp snmpd program.  Only the options below are supported.
//
//             C - Don't read conf files
//             d - Dump sent and received SNMP packets
//             D - Turn on debugging output
//             v - Print version info
//             V - Verbose display
//                          
// PARAMETERS: parm - the parameter to set
// RETURNS:    zero on success 
*******************************************************************************/
void
bcmV2_input_parameters (char param)
{
    //printf("bcmV2_input_parameters\n");
    switch ( param )
    {
    
    case 'C':
        netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, 
                               NETSNMP_DS_LIB_DONT_READ_CONFIGS, 1);
        break;

    case 'd':
        //snmp_set_dump_packet(++snmp_dump_packet);
        netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
                               NETSNMP_DS_AGENT_VERBOSE, 1);
        break;

    case 'D':
        /* debug_register_tokens(optarg); */
        snmp_set_do_debugging(1);
        // printf("enable -D debugging\n");
        break;

        /*
        case 'H':
            netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
                   NETSNMP_DS_AGENT_NO_ROOT_ACCESS, 1);
            init_agent("snmpd");        // register our .conf handlers 
            init_mib_modules();
            init_snmp("snmpd");
            fprintf(stderr, "Configuration directives understood:\n");
            read_config_print_usage("  ");
            exit(0);
        */
        /*
        case 'L':
            stderr_log = 1;
            break;
        */
    //case 'q':
    //    snmp_set_quick_print(1);
    //    break;

    case 'v':
#ifndef BCM_NULL_CONSOLE
        printf("\nNET-SNMP version:  %s\n", netsnmp_get_version());
#endif
        break;

    case 'V':
        netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
                               NETSNMP_DS_AGENT_VERBOSE, 1);
        break;

    default:
        // printf("bcmV2_input_parameters: unknown param <%c>\n",param);
        break;    
    }
}

/*******************************************************************************
// FUNCTION:   bcmV2_init_snmp (local function)
//                      
// PURPOSE:    Called locally. Similar to and called instead of init_snmp()
//             to suit our needs.
//                          
// PARAMETERS: NONE
// RETURNS:    zero on success 
*******************************************************************************/
int
bcmV2_init_snmp(const char *type)
{
    static int      done_init = 0;      /* To prevent double init's. */
#ifdef SEG_FAULT_DUMP
  struct sigaction new_action;
#endif

    if ( done_init )
    {
        return 1;
    }

    DEBUG("bcmV2_init_snmp -starting\n");
    done_init = 1;

    /** make the type available everywhere else */
    if ( type && !netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
                                        NETSNMP_DS_LIB_APPTYPE) )
    {
        netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, 
                              NETSNMP_DS_LIB_APPTYPE, type);
    }

    /* _init_snmp(); */
    _init_snmp();
    DEBUG("Back from call to _init_snmp\n");

    /** set our current locale properly to initialize isprint() type functions */
#ifdef HAVE_SETLOCALE
    setlocale(LC_CTYPE, "");
#endif

    snmp_debug_init();    /* should be done first, to turn on debugging ASAP */
    netsnmp_container_init_list();
    init_callbacks();
    init_snmp_logging();
    snmp_init_statistics();
    //register_mib_handlers();   //don't think we need this

    /** register_default_handlers();*******************/
    netsnmp_ds_register_config(ASN_BOOLEAN, "snmp", "dumpPacket", NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DUMP_PACKET);
    netsnmp_ds_register_config(ASN_BOOLEAN, "snmp", "reverseEncodeBER", NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_REVERSE_ENCODE);
    netsnmp_ds_register_config(ASN_INTEGER, "snmp", "defaultPort", NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DEFAULT_PORT);
    netsnmp_ds_register_config(ASN_OCTET_STR, "snmp", "defCommunity", NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_COMMUNITY);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "noTokenWarnings", NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_NO_TOKEN_WARNINGS);
    netsnmp_ds_register_config(ASN_BOOLEAN, "snmp", "noRangeCheck", NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_CHECK_RANGE);
    //netsnmp_ds_register_config(ASN_OCTET_STR, "snmp", "persistentDir", NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PERSISTENT_DIR);
    netsnmp_ds_register_config(ASN_BOOLEAN, "snmp", "noDisplayHint", NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_NO_DISPLAY_HINT);
    netsnmp_ds_register_config(ASN_BOOLEAN, "snmp", "16bitIDs", NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_16BIT_IDS);
    /***************************/

    init_snmpv3(type);
    //init_snmp_alarm();       //don't think we need this
    init_snmp_enums();
    //read_premib_configs();   //don't think we need this
    init_mib();

    /** read_configs();  below line is all we need from read_configs()*/
    snmp_call_callbacks(SNMP_CALLBACK_LIBRARY,SNMP_CALLBACK_POST_READ_CONFIG, NULL);


#ifdef SEG_FAULT_DUMP
  new_action.sa_handler = (void *)sigsegv_handler;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = SA_RESTART | SA_SIGINFO;

  /* Install my own SIGSEGV signal handler */
  sigaction (SIGSEGV, &new_action, NULL);
  /* Install my own SIGBUS signal handler */
  sigaction (SIGBUS, &new_action, NULL);
#endif

    return 0; 
}

/*******************************************************************************
// FUNCTION:   bcmV2_agent_init()
//                      
// PURPOSE:    Called to initialize the NetSnmp libraries.
//                          
// PARAMETERS: NONE
// RETURNS:    zero on success 
*******************************************************************************/
int
bcmV2_agent_init()
{
    /*
    int             dont_zero_log = 0;
    int             stderr_log = 0, syslog_log = 0;
    char            logfile[PATH_MAX + 1] = { 0 };
    */
    DEBUG("\nbcmV2_agent_init: -starting \n");

    //need this for informs
    init_snmpv3_post_premib_config(0, 0, NULL, NULL);

    DEBUG("\nbcmV2_agent_init: init_snmpv3_post_premib_config() done \n");
#ifdef NO_ROOT_ACCESS

    /** Default to no.*/
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
                           NETSNMP_DS_AGENT_NO_ROOT_ACCESS, 1);
#endif

    /** Default to NOT running an AgentX master.*/
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
                           NETSNMP_DS_AGENT_AGENTX_MASTER, 0);
    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                       NETSNMP_DS_AGENT_AGENTX_TIMEOUT, -1);
    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                       NETSNMP_DS_AGENT_AGENTX_RETRIES, -1);

    /* Do not read configuration files */
    bcmV2_input_parameters('C');

    DEBUG("\nbcmV2_agent_init: bcmV2_input_parameters() done \n");
    /* setup_log(0, dont_zero_log, stderr_log, syslog_log, logfile);  */

#ifdef BUFSIZ
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    DEBUG("\nbcmV2_agent_init: setvbuf() done \n");
#endif 

    SOCK_STARTUP;
    DEBUG("\nbcmV2_agent_init - call init_agent() ==============================================================\n");
    init_agent("snmpd");        

    DEBUG("\nbcmV2_agent_init - call init_mib_modules() ==============================================================\n");
    //bcmV2_input_parameters('D');
    init_mib_modules(); // this just includes mib_module_inits.h
    ///  snmp_set_do_debugging(0);

    /** start library */
    /* init_snmp("snmpd"); do below instead- */
    bcmV2_init_snmp("snmpd");

    // Install our pre-check handler for all netsnmp handled objects
    // (like VACM/USM things, SNMP stats, etc)
    bcmV2_init_netsnmp_precheck();

    //printf("for debug call move bcmV2_init_snmp above\n");
    DEBUG("\nbcmV2_agent_init - call init_bcmV2SessionAndTransport ==============================================================\n");
    init_bcmV2SessionAndTransport();

    DEBUG("\nbcmV2_agent_init - call bcmV2_init_snmp ==============================================================\n");
    ///bcmV2_init_snmp("snmpd");


    /* This causes problems for V2 thread - don't call it, -TC
    if ((init_master_agent()) != 0) {
        printf("Error opening one of the specified agent transports.\n");
        return 1;   
    }
    */

    /** Store persistent data immediately in case we crash later.*/
    /* snmp_store("snmpd"); //TODO findout who else is doing this!!!!!!!!  */
    DEBUG("\nbcmV2_agent_init - call netsnmp_addrcache_initialise() ==============================================================\n");
    netsnmp_addrcache_initialise();
    DEBUG("\nbcmV2_agent_init - netsnmp_addrcache_initialise() done\n");

    //init_bcmV2SessionAndTransport();




    //init_usm_post_config(int majorid, int minorid, void *serverarg,void *clientarg)
    //this is a callback that never got called since we did not do reads
    //the callback was SNMP_CALLBACK_POST_PREMIB_READ_CONFIG but be careful, it gets 
    //registered twice.  So just call the function
    //init_snmpv3_post_premib_config(0, 0, NULL, NULL) to setup the noname user;
    DEBUG("\nbcmV2_agent_init - call init_usm_post_config() ==============================================================\n");
    init_usm_post_config(0, 0, NULL, NULL);
    DEBUG("\nbcmV2_agent_init - init_usm_post_config() done\n");
    fflush(stdout);

    // snmp_shutdown("snmpd"); TODO may need this!!!!!!!
    DEBUG("bcmV2_agent_init: -finished \n");
    return 0;
}                               

void printTransport (netsnmp_transport *pTransport)
{
    int Loop;

    printf ("SNMP transport:\n");

    if ( pTransport == NULL )
    {
        printf ("  NULL\n");
        return;
    }

    printf ("  domain=");
    for ( Loop = 0; Loop < pTransport->domain_length; Loop++ )
        printf (".%ld", pTransport->domain[Loop]);
    printf ("\n");
    printf ("  local=");
    for ( Loop = 0; Loop < pTransport->local_length; Loop++ )
        printf (".%d", pTransport->local[Loop]);
    printf ("\n");
    printf ("  remote=");
    for ( Loop = 0; Loop < pTransport->remote_length; Loop++ )
        printf (".%d", pTransport->remote[Loop]);
    printf ("\n");
    printf ("  socket=%d\n", pTransport->sock);
    printf ("  flags=0x%04X\n", pTransport->flags);
    printf ("  data=%p (%d)\n", pTransport->data, pTransport->data_length);
    printf ("  msgMaxSize=%d\n", pTransport->msgMaxSize);
    printf ("  recv_callback=%p\n", pTransport->f_recv);
    printf ("  send_callback=%p\n", pTransport->f_send);
    printf ("  close_callback=%p\n", pTransport->f_close);
    printf ("  close_callback=%p\n", pTransport->f_close);
    printf ("  accept_callback=%p\n", pTransport->f_accept);
    printf ("  fmtaddr_callback=%p\n", pTransport->f_fmtaddr);
}

unsigned long GetVarbindSize (netsnmp_pdu *pdu)
{
  netsnmp_variable_list *pVB;
  int VBs = 0;
  unsigned long Size = 0;
  
  
  DEBUG ("GetVarbindSize\n");
  
  if (pdu == NULL)
  {
    DEBUG ("  no PDU!\n");
    return 0;
  }
  
  pVB = pdu->variables;
  while ( pVB )
  {
      // Size of the value...
      Size += pVB->val_len;
      
      // Plus size of the OID.
      Size += pVB->name_length;
      
      VBs++;
      pVB = pVB->next_variable;
  }
  
  DEBUG ("%d VBs, %d bytes\n", VBs, Size);
  
  return Size;
}

// Take an opportunity to check for a packet which is too big and return the
// TOO_BIG error if it is.  Also here is a chance for us to clean up some
// things we've allocated and stored off in the packet.  Is there a better
// place to do these things?
int bcmV2_realloc_build (netsnmp_session *session, netsnmp_pdu *pdu, u_char **pkt, size_t *pkt_len, size_t *offset)
{
    int rc;
    u_char *newPkt = NULL;
    u_char *pduData;
    netsnmp_pdu *pReqPdu;


    DEBUG ("bcmV2_realloc_build: pkt=%p, length=%d, offset=%d, cmd=%d\n", *pkt, *pkt_len, *offset, pdu->command);
    memset (*pkt, 0, *pkt_len);
    
    // We remember the request PDU in our session's myvoid pointer in the
    // bcmV2_check_pdu function, below.  Retrieve it now so we can tell what
    // kind of request this is in reply to.  Also so we can free some stuff.
    pReqPdu = (netsnmp_pdu *) session->myvoid;
    if (pReqPdu)
      DEBUG ("  Request command = %d\n", pReqPdu->command);
    else
      DEBUG ("  No request PDU, this must be an unsolicited message\n");
    
    // If this is an SNMPv1 or SNMPv2c packet, we must now free the
    // security name in the PDU, which was allocated in bcmV2_check_pdu
    // Note that we must check for NULL here on the pReqPdu because myvoid
    // gets assigned in check_pdu, and for SNMPv3 this might not have happened
    // yet.  Also, don't check this for REPORT (168) packets regardless.
    if ((pReqPdu != NULL) && (pdu->command != 168))
    {
      if ( (pReqPdu->version != SNMP_VERSION_3) 
      &&   (pReqPdu->securityName) )
      {
          DEBUG ("  freeing reqPdu security name.\n");
          free (pReqPdu->securityName);
          pReqPdu->securityName = NULL;

      }
      else
      {
          DEBUG ("  Not freeing reqPdu security name; version = %d, securityName = %p\n", 
                 pReqPdu->version, pReqPdu->securityName);
      }
    }
    else
      DEBUG ("  pReqPdu = NULL or REPORT, not checking security name.\n");
      
    // Handle the possibility of a packet bigger than we choose to deal with.
    // We choose 6k based on the fact that eCos and/or the IP stack will give
    // us a socket error on a sendto with a packet bigger than 8k.  So pick
    // 6k to allow some overhead after encoding.  Note that GetVarbindSize only
    // returns the length of the values + the length of the OIDs, it doesn't
    // account for ASN.1 encoding, let alone SNMPv3 encryption.  Hence the
    // 2k SWAG.
    if (GetVarbindSize (pdu) > 6124)
    {
      // We are going to free all our varbinds here regardless of what
      // we decide to do.  So do that now:
      DEBUG ("  Packet is too big!  Freeing varbinds.\n");
          
      // Free all VBs.  Our reply packet will be empty except for error code
      // and index of 0.  Note that snmp_free_varbind frees the whole list.
      snmp_free_varbind (pdu->variables);
      pdu->variables = NULL;
      
      DEBUG ("  Done freeing varbinds.\n");
      
      // if this is a SET, we will leave the error code and index as-is,
      // but nix the varbinds.  This way we at least get an error code and
      // index back to our manager.  Otherwise, we will bail and return a
      // tooBig error.
      if ((pReqPdu == NULL) || (pReqPdu->command != SNMP_MSG_SET))
      {
          DEBUG ("  Returning TOO_BIG.\n");
          pdu->errstat = SNMP_TOO_BIG;
          pdu->errindex = 0;
      }
      else
          DEBUG ("  SET request, not returning TOO_BIG.\n");
    }
    
    // Call Net-SNMP and build the packet.
    // Note that the pkt_len argument is how big the buffer is.  The actual
    // size of the resulting pkt is returned in *offset.
    DEBUG ("  Calling snmp_build\n");
    rc = snmp_build (pkt, pkt_len, offset, session, pdu);
    
    if (rc != 0)
    {
      DEBUG ("  snmp_build failed with return code %d\n", rc);
      return rc;
    }

    DEBUG ("  Net-SNMP returned %d: pkt=%p, length=%d, offset=%d\n", rc, *pkt, *pkt_len, *offset);
    
    // Now we must do something odd.  snmp_build puts the PDU data
    // at the END of the supplied buffer, not at the start because they do 
    // reverse encoding (turns out to be easier to build the packet backwards).
    // What's more, the function that calls us is going to free the buffer
    // that is pointed to by pkt, so that means that we must re-allocate a new
    // buffer, copy the stuff from the END of the old buffer in there, then
    // free the old buffer and reassign.  Ugh, what a mess!
    // We can't just call snmp_build and return what he does, because the 
    // returned *pkt from this function must point to the start of the real
    // data and be freeable.  So we gotta re-allocate and copy.
    pduData = (*pkt) + ((*pkt_len) - (*offset));
    newPkt = malloc(*offset);
    memcpy (newPkt, pduData, *offset);
    free (*pkt);
    *pkt = newPkt;

    // Set our session max packet size to 0.  Since NetSnmp will DROP any packet
    // bigger than this value, we would like him to try his best even if it
    // is too big.  Note that this should never be != 0 anyway, but still...
    session->sndMsgMaxSize = 0;

    // Ditto with the transport max size.
    gpTransport->msgMaxSize = 0;

    return rc;
}

// Returns 1 if the packet is OK, 0 if it is bad and is to be dropped.
// Replacement for netsnmp_agent_check_parse ("post-parse" callback).
// Here is where we call AppValidateSnmpRequest, also hopefully we can
// work out AppValidateSetRequest here as well.
int bcmV2_check_pdu (netsnmp_session *session, netsnmp_pdu *pdu, int result)
{
    char pSecName[128];
    unsigned char pSA[4] = {0,0,0,0}, pDA[4] = {0,0,0,0};
    unsigned short SrcPort = 0, DstPort = 0;
    netsnmp_transport *pTransport;
    SNMP_STATUS Status;
    int PduType;
    char *pCommunity;
    const char *pView = NULL;
    char  *readView, *writeView, *notifyView;
    
    DEBUG ("bcmV2_check_pdu for PDU at %p, session=%p, securityName=%p\n, result=%d", pdu, session, pdu->securityName, result);

    // Put our PDU in the session's myvoid pointer.  This is so we can retrieve
    // it later in our bcmV2_realloc_build function, above, where we need to
    // check the command type.
    DEBUG ("  Remembering request PDU in session->myvoid (previously %p)\n", session->myvoid);
    session->myvoid = pdu;

    // Get src & dest addrs and ports from our global transport.
    pTransport = gpTransport;

    if ( pTransport == NULL )
        printf ("bcmV2_check_pdu: ERROR - Couldn't resolve transport!  Use 0.0.0.0\n");
    else
    {
        DEBUG ("  Transport found at %p, local=%p, remote=%p\n", 
               pTransport, pTransport->local, pTransport->remote);

        memcpy (pSA, pTransport->remote, 4);  
        memcpy (pDA, pTransport->local, 4);

        SrcPort = (pTransport->remote[4] * 0x100) | pTransport->remote[5];
        DstPort = (pTransport->local[4] * 0x100) | pTransport->local[5];
    }

    // Mask off some bits to get the PduType from the pdu->command field
    PduType = pdu->command & ~(ASN_CONTEXT | ASN_CONSTRUCTOR);

    // Give the app a look at this request.
    // We need to make a copy of the community string in the SNMP packet
    // because we will need to NULL terminate it.  So malloc a big enough
    // buffer to use, we'll free it when we're done.
    if ( pdu->community_len )
    {
        pCommunity = (char *) malloc (pdu->community_len + 1);
        memset (pCommunity, 0, pdu->community_len + 1);
        memcpy (pCommunity, pdu->community, pdu->community_len);
        DEBUG ("  Allocated %d bytes for community string '%s'\n", pdu->community_len + 1, pCommunity);
    }
    else  {        // Probably (definitely?) SNMPv3
        pCommunity = (char *) malloc (pdu->securityNameLen + 1);
        memset (pCommunity, 0, pdu->securityNameLen + 1);
        memcpy (pCommunity, pdu->securityName, pdu->securityNameLen);
        DEBUG ("  Allocated %d bytes for security name string '%s'\n", pdu->securityNameLen + 1, pCommunity);
    }

    // Clear the security name that we're passing over.
    memset (pSecName, 0, sizeof (pSecName));

    // Call our API to validate the request.
    Status = AppValidateSnmpRequest (PduType, pdu->version, pSA, SrcPort, pDA, DstPort, pCommunity, pSecName, sizeof(pSecName));

    // Now we can free the community copy we made.
    if ( pdu->community_len )
        free (pCommunity);

    // If the app doesn't like the request, return non-zero here and we will
    // timeout.
    if ( (Status != SNMP_NO_ERROR) && (Status != SNMP_NO_OPINION) )
    {
        DEBUG ("AppValidateSnmpRequest returned error %d, no access.\n", Status);

        // Increment statistics.
        if ( Status == SNMP_BAD_COMMUNITY_NAME )
            snmp_increment_statistic(STAT_SNMPINBADCOMMUNITYNAMES);
        else if ( Status == SNMP_BAD_COMMUNITY_USE )
            snmp_increment_statistic(STAT_SNMPINBADCOMMUNITYUSES);

        return 0;
    }
    
    // PR 6218: If we get a bum PDU type, return zero so the packet will be 
    // dropped.
    if ((PduType != PDU_GET_REQUEST)    && (PduType != PDU_GET_NEXT_REQUEST)
    &&  (PduType != PDU_GET_RESPONSE)   && (PduType != PDU_GET_BULK_REQUEST)
    &&  (PduType != PDU_SET_REQUEST)    && (PduType != PDU_TRAP)
    &&  (PduType != PDU_INFORM_REQUEST) && (PduType != PDU_TRAP2))
    {
      DEBUG ("Unknown PDU type (0x%02x), no access.\n", PduType);
      snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
      return 0;
    }

    if ( pdu->version != SNMP_VERSION_3 )
    {
        // Load the view for this PDU into the security name field for the PDU.
        // Note that this will be freed by snmp_free_pdu later, so we must 
        // allocate storage.  Sadly, the netsnmp PDU structure has no storage
        // for a view, or any kind of void * that we can use for it.  So we'll
        // usurp the security name field instead.
        if ( pdu->securityName == NULL )
        {
            DEBUG ("NULL security name in the PDU, allocating and copying.\n");
        }
        else
        {
            DEBUG ("PDU security name WAS %d bytes, %s.  Freeing and overwriting with %s.\n", pdu->securityName, pdu->securityNameLen, pSecName);
            SNMP_FREE (pdu->securityName);
        }

        // If we get a security name back from the app, resolve the view to use
        // now.
        if ( pSecName[0] != 0 )
        {
            DEBUG ("  Security name for this PDU is %s, resolve to view.\n", pSecName);

            // map the view from the sec name.
            if( bcmV2_FindViewsFromSecurityName(pSecName, &readView, &writeView, &notifyView,
                               pdu->securityModel,
                               pdu->securityLevel) )
            {
                DEBUG("FindViewsFromSecurityName failed\n");
                
                snmp_increment_statistic(STAT_SNMPINBADCOMMUNITYUSES);
                AppSendTrap (pDA, AUTH_FAILURE_TRAP, 0);
                return 0;
            }
            //printf("Found Views readView<%s> writeView<%s> notifyView<%s>\n",readView,writeView,notifyView);
                        
            //use the view based on the pdu type
            switch (PduType)
            {
            case PDU_GET_REQUEST:                 
            case PDU_GET_NEXT_REQUEST:
            case PDU_GET_RESPONSE:             
            case PDU_GET_BULK_REQUEST:
                 pView = readView;
                 break;
                 
            case PDU_SET_REQUEST: 
                 pView = writeView;
                 break;
            
            case PDU_TRAP:                             
            case PDU_INFORM_REQUEST:             
            case PDU_TRAP2:
                 pView = notifyView;
                 break;
            default:
                break;                
            }
            
            // PR 6476: if we have no view for the associated PDU type,
            // reject the query and send an authFail trap.
            if ((pView == NULL) || (pView[0] == 0))
            {
              DEBUG ("  No view for security name, no access.\n");
              
              snmp_increment_statistic(STAT_SNMPINBADCOMMUNITYUSES);
              AppSendTrap (pDA, AUTH_FAILURE_TRAP, 0);
              return 0;
            }
        }
        else
        {
            DEBUG ("  No security name in this PDU, use view from the app.\n");

            // Extract the DA for this PDU from our transport.
            pView = AppGetV1V2View (gpTransport->local);
        }

        pdu->securityName = (char *) malloc (strlen (pView) + 1);
        strcpy (pdu->securityName, pView);
        pdu->securityNameLen = strlen(pView);

        DEBUG ("  Using view %s\n", pView);
    }

    DEBUG ("AppValidatSnmpRequest returned NO_ERROR, access granted.\n");

    return 1;
}


// Return 0 if in view, 5 if not.  See vacm.h for real return codes, but 0 (IN_VIEW)
// and 5 (NOT_IN_VIEW) are the only ones that won't cause us to timeout...
// The reason we need this callback is to handle checking views for SNMPv1 and
// SNMPv2c queries, either via coexistence (RFC-2576) or by the V1V2View
// associated with the BcmSnmpAgent object.  The usual netsnmp mechanism is no
// good for us because it expects a mapping similar to the snmpCommunityTable
// (com2sec) in some config file.
int bcmV2_vacm_in_view_callback(int majorID, int minorID, void *serverarg, void *clientarg)
{
    struct view_parameters *view_parms = (struct view_parameters *) serverarg;
    int retval;
    unsigned char *cp;


    DEBUG ("bcmV2_vacm_in_view_callback\n");

    // If we have no view_parms we are sewiously scwewed.
    if ( view_parms == NULL )
    {
        DEBUG ("  Couldn't get view_parms, return 0. (majorID=%d, minorID=%d)\n",
               majorID, minorID);
        return VACM_NOTINVIEW;
    }

    // If this is an SNMPv1 or SNMPv2c request, we need to check the view.
    // Note that since netsnmp doesn't have storage for the view in the PDU
    // structure, we usurp the security name field for it.  This allows us to
    // only resolve the view one time per PDU instead of one time for every
    // varbind in the PDU.  This is particularly important for bulk requests
    // when we could wind up doing this LOTS of times, and we wind up timing
    // out.
    if ( view_parms->pdu->version != SNMP_VERSION_3 )
    {
        // Note that the check for view_parms->namelen here is because this callback
        // gets called several times per VB, and not every time do we have an OID
        // to check yet.  So if that's the case, just return 0 and move on.
        if ( view_parms->namelen == 0 )
        {
            DEBUG ("  No OID specified, just return SUCCESS. (majorID=%d, minorID=%d)\n",
                   majorID, minorID);
            return VACM_SUCCESS;
        }

#ifdef NOTDEF
        cp = (unsigned char *) view_parms->pdu->transport_data;
        DEBUG ("  transport_data_length = %d \n", view_parms->pdu->transport_data_length);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d.", *cp++);
        DEBUG ("%d\n.", *cp);
#endif
        if ( view_parms->pdu->transport_data_length >= 8) {
          cp = (unsigned char *) view_parms->pdu->transport_data;
          if (cp) {
            if (cp[4] == 127) {
              DEBUG ("request from loopback address, always in view!");
              return VACM_SUCCESS;
            }
          }
        }

        if ( trace_level > 0 )
        {
            int Loop;

            DEBUG ("  Using view %s for this query, OID (%d elements) = ", 
                   view_parms->pdu->securityName, view_parms->namelen);
            for ( Loop = 0; Loop < view_parms->namelen; Loop++ )
                DEBUG (".%d", view_parms->name[Loop]);
            DEBUG ("\n");
        }

        // Now check the OID for the view to use.  We can get the OID straight
        // from the supplied view_parms argument.  Note that this had better only
        // return VACM_SUCCESS(0) or VACM_NOTINVIEW(5) or else we will time out...
        //
        // UNFINISHED: this function is very S L O W and can produce timeouts on
        // bulkwalks, plus herky-jerky response on regular walks.  Can we 
        // rewrite it to speed it up?
        if ( view_parms->pdu->securityName )
            retval = vacm_checkSubtree (view_parms->pdu->securityName, view_parms->name, view_parms->namelen);
        else
        {
            DEBUG ("  No view specified for this query!  NOTINVIEW.\n");
            retval = VACM_NOTINVIEW;
        }
    }
    else
    {
        // For SNMPv3 just call the standard function to check view status.
        DEBUG ("  SNMPv3 query or no OID, let NetSNMP handle it.\n");
        retval = vacm_in_view(view_parms->pdu, view_parms->name,
                              view_parms->namelen, view_parms->check_subtree);
    }

    DEBUG ("bcmV2_vacm_in_view_callback returning %d\n", retval);

    if ( retval != VACM_SUCCESS )
        view_parms->errorcode = retval;

    return retval;
}


/* 
   New approach here, plays much nicer with netsnmp!  No more using 
   internal sneaky functions and structures.  
*/
void init_bcmV2SessionAndTransport ()
{
    netsnmp_session TempSession;
    void *sessp;


    /* DEBUG */
    //trace_level = 1;
    //snmp_set_do_debugging(1);

    DEBUG ("init_bcmV2SessionAndTransport\n");

    /* Register our 'brcm' domain - based on UDP but with some hooks */

    netsnmp_brcm_ctor ();

    /* Open a session using a brcm domain transport. */

    // This will initialize our temp session with reasonable values and zero
    // everything else out.    
    snmp_sess_init(&TempSession);

    // Other members we will set explicitly here:
    TempSession.version = SNMP_DEFAULT_VERSION;
    TempSession.callback = handle_snmp_packet;
    TempSession.authenticator = NULL;
    TempSession.flags = netsnmp_ds_get_int(NETSNMP_DS_APPLICATION_ID, 
                                           NETSNMP_DS_AGENT_FLAGS);
    TempSession.isAuthoritative = SNMP_SESS_AUTHORITATIVE;

    // Open a session using a brcm domain transport.
    // Use a bogus IP address and port.  We will replace these on each
    // query anyway.
    TempSession.peername = "udp:127.0.0.1:420";
    TempSession.local_port = 1; // Indicate that this is a 'local' (server) session

    // Open up a session and hook the callbacks we're interested in.    
    gpSession = snmp_open_ex(&TempSession,
                             netsnmp_agent_check_packet,   /* pre-parse */
                             NULL,                         /* parse */
                             bcmV2_check_pdu,              /* post-parse */
                             NULL,                         /* build */
                             bcmV2_realloc_build,          /* realloc-build */
                             NULL);                        /* check */
    DEBUG ("  got session @ %p\n", gpSession);

    // We have to get a pointer to the sneaky 'session list' associated
    // with the session we just got.  Why netsnmp is so covert with this
    // I dunno, but they return a void * so obviously they don't want us
    // to use this other than as a kind of handle.  Whatever.
    sessp = snmp_sess_pointer (gpSession);
    DEBUG ("  got sneaky-session @ %p\n", sessp);

    // Get the transport that was created for the session.    
    gpTransport = snmp_sess_transport (sessp);
    DEBUG ("  got transport @ %p\n", gpTransport);

    // We are not going to use the socket that net-snmp created for us, so
    // close it and zero (-1 actually) it out.  The actual socket is passed to 
    // us from V2, and that's the one we'll be using.
    if ( gpTransport->sock > 0 )
    {
        DEBUG ("  closing socket %d\n", gpTransport->sock);
        close (gpTransport->sock);
        gpTransport->sock = -1;
    }

    // Make way for the addresses if necessary.

    if ( gpTransport->remote == NULL )
    {
        DEBUG ("  allocating storage for remote address.\n");
        gpTransport->remote = (unsigned char *) malloc (6);
        gpTransport->remote_length = 6;
    }

    if ( gpTransport->local == NULL )
    {
        DEBUG ("  allocating storage for local address.\n");
        gpTransport->local = (unsigned char *) malloc (6);
        gpTransport->local_length = 6;
    }

    //printTransport (gpTransport);
    
    DEBUG ("  registering custom callbacks.\n");

    // Register our own VACM in-view callback so we can assign views for
    // SNMPv1 & SNMPv2c in the absence of coexistence operation.
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_ACM_CHECK, bcmV2_vacm_in_view_callback,
                           NULL);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_ACM_CHECK_INITIAL,
                           bcmV2_vacm_in_view_callback, NULL);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_ACM_CHECK_SUBTREE,
                           bcmV2_vacm_in_view_callback, NULL);

    DEBUG ("  unregistering default callbacks.\n");

    // Unregister the default callbacks.  We want to replace them with our own,
    // not add our own to them.
    /*
    trc- We are not even registering these now.
    
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_ACM_CHECK, vacm_in_view_callback,
                             NULL, 0);
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_ACM_CHECK_INITIAL,
                             vacm_in_view_callback, NULL, 0);
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_ACM_CHECK_SUBTREE,
                             vacm_in_view_callback, NULL, 0);
    */
    DEBUG ("SNMP session and transport initialization complete.\n");

    //trace_level = 0;
    //snmp_set_do_debugging(0);

    // Here is as good a place as any to turn on snmpEnableAuthenTraps.
    // No spec requirements about what this should be by default, but
    // test protocols seems to expect it to be 'on'
    snmp_enableauthentraps = 1;

}

/*
EngineId has been installed - but maybe the times and boots need re-initialized.
 
NetSnmp uses the Time function in get_engineTime().  It expects Time in getTimeofDay() and
does several calculations to to arrive at an engine time in several locations.  However, 
when we set our system time of day, the values returned by Time and other calls have changed.
This screws our engine time which has already been set and causes 'NotInTimeWindows' errors.
So we need to reset engine time once TOD has happened.   
A better way would be to use GetSystemUpTime() accross all time functions.  This would give
us a steady second timer that is adjusted for TOD changes.  However, this would require changes
in at least 4-5 functions in NetSnmp.  So - lets just reset engine time once TOD has finished.
*/
int bcmV2_update_engine_times()
{
    int   engineIDLength;
    u_char engineID[USM_MAX_ID_LENGTH];
    int numOfBoots;

    numOfBoots = AppGetEngineBoots();

    //init the time the engine started
    gettimeofday(&snmpv3starttime, NULL);

    //get our engineId
    engineIDLength = snmpv3_get_engineID(engineID, USM_MAX_ID_LENGTH);

    //set boots and time for our engineId

    //printf("bcmV2_update_engine_times engineIdlen = %d\n",engineIDLength);
    //for (i=0; i<engineIDLength; i++)
    //{
    //    printf("%x",engineID[i]);
    //}

    set_enginetime(engineID, engineIDLength,
                   numOfBoots,
                   snmpv3_local_snmpEngineTime(),
                   TRUE);

    // NetSnmp uses a separate local variable to check timeliness.  Set it
    snmpv3_set_local_snmpEngineBoots(numOfBoots);

    DEBUG ("BcmV2_update_engine_times() is setting engineTime<%d>, boots<%d>, snmpv3_local_snmpEngineTime(),numOfBoots\n");

    return 0;
}

/*
   PR 7842 moved this function from .../snmplib/snmpv3.c and implement
   it here.

 * snmpv3_local_snmpEngineTime(): return the number of seconds since the
 * snmpv3 engine last incremented engine_boots
 */
unsigned long
snmpv3_local_snmpEngineTime(void)
{
    unsigned long currentTime = (SystemTimeMS()/1000);

    //make sure these globals are zero
    snmpv3starttime.tv_sec = 0;
    snmpv3starttime.tv_usec= 0;

    //printf("glue- snmpv3_local_snmpEngineTime = <%ld>\n",currentTime);
    return currentTime;
}




// This handler is injected into the call chain for ALL netsnmp handlers.
// Which is to say, those objects handled natively by netsnmp which do not
// call into the app (eg VACM/USM things, snmp stats, etc.)
// So this gives us an opportunity to apply any upfront checking (in
// particular, AppValidateSetRequest) for those objects.  For V2/BFC handled
// objects, we call this function in bcmV2_do_scalar and bcmV2_do_tabular.  The
// reason we don't install this handler for the V2/BFC objects as well is that
// you gotta clone it, which eats up some RAM (which netsnmp is none too
// stingy about anyway).  So injecting it only into the netsnmp handlers
// helps us conserve some memory.
int
bcmV2_precheck_netsnmp (netsnmp_mib_handler *handler,
                        netsnmp_handler_registration *reginfo,
                        netsnmp_agent_request_info *reqinfo,
                        netsnmp_request_info *requests)
{
    netsnmp_request_info *request;
    netsnmp_variable_list *var;
    netsnmp_variable_list tmpVar;
    oid             tmpOid[MAX_OID_LEN];
    int             retVal = SNMP_ERR_NOERROR;
    int Loop;
    netsnmp_pdu *pdu = NULL;

    DEBUG ("bcmV2_precheck_netsnmp\n");
    
    // I need a pointer to my PDU here.  I can get it from gpSession->myvoid.
    // Is there a better way?  There is a field for it in the 
    // netsnmp_agent_request_info structure, but it is NULL at this time.
    pdu = (netsnmp_pdu *) gpSession->myvoid;
    
    for ( request = requests; request; request = request->next )
    {
        var = request->requestvb;
        if ( request->processed != 0 )
            continue;

        memset (&tmpVar, 0, sizeof(netsnmp_variable_list));
        tmpVar.name = tmpOid;

        DEBUG ("bcmV2_precheck_netsnmp: root OID = ");
        for ( Loop = 0; Loop < reginfo->rootoid_len; Loop++ )
            DEBUG (".%d", reginfo->rootoid[Loop]);
        DEBUG ("\n");

        DEBUG ("                  req OID = ");
        for ( Loop = 0; Loop < request->requestvb->name_length; Loop++ )
            DEBUG (".%d", request->requestvb->name[Loop]);
        DEBUG ("\n");

        switch ( reqinfo->mode )
        {
        case MODE_GET:
            // Nothing to do...
            break;

        case MODE_GETNEXT:
            // Nothing to do...
            break;

        case MODE_SET_RESERVE1: // AKA validate
            retVal = v2ValidateSet (var->name,
                                    var->name_length,
                                    var->val.string,
                                    var->val_len,
                                    var->type, reqinfo,
                                    UNSPECIFIED_INDEX_OFFSET);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;

        case MODE_SET_RESERVE2: // AKA test
        case MODE_SET_ACTION:
        case MODE_SET_COMMIT: // AKA set
        case MODE_SET_UNDO:
        case MODE_SET_FREE:
            // Nothing to do...
            break;

        default:
            // Nothing to do...
            break;
        }
        
        if ( retVal != SNMP_ERR_NOERROR )
          break;
    }

    if ( retVal != SNMP_ERR_NOERROR )
        return retVal;

    return netsnmp_call_next_handler (handler, reginfo, reqinfo, requests);
}


void bcmV2_init_netsnmp_precheck ()
{
    netsnmp_subtree *pRoot, *tptr;
    int Injected = 0;

    if ( gpNetsnmpPrecheckHandler )
        return;

    DEBUG ("bcmV2_init_netsnmp_precheck\n");

    // Create our netsnmp precheck handler
    gpNetsnmpPrecheckHandler = netsnmp_create_handler ("netsnp-precheck", bcmV2_precheck_netsnmp);

    if ( gpNetsnmpPrecheckHandler == NULL )
    {
        DEBUG ("  couldn't create handler!\n");
        return;
    }
    DEBUG ("  created handler @ %p\n", gpNetsnmpPrecheckHandler);

    // Find the subtree for OID .1
    pRoot = netsnmp_subtree_find_first (NULL);

    // Inject it.
    if ( pRoot == NULL )
    {
        DEBUG ("  couldn't find root subtree!\n");
        return;
    }
    DEBUG ("  found root @ %p\n", pRoot);

    for ( tptr = pRoot; tptr != NULL; tptr = tptr->next )
    {
        gCoreObjects++;

        // Currently our precheck handler is only used for checking SET operations
        // via AppValidateSetRequest.  So save a little RAM by not adding it to
        // read-only handlers.
        if ( tptr->reginfo->modes & HANDLER_CAN_SET )
        {
            netsnmp_inject_handler(tptr->reginfo, clone_handler(gpNetsnmpPrecheckHandler));
            Injected++;
        }
    }

    DEBUG ("  bcmV2_precheck_netsnmp injected into %d handlers.\n", Injected);
}


int bcmV2_snmp_read (unsigned long DstIp, unsigned short DstPort, int Socket)
{
    fd_set socket_set;

    // Load local address into our transport.
    gpTransport->local[0] = (DstIp >> 24) & 0xFF;
    gpTransport->local[1] = (DstIp >> 16) & 0xFF;
    gpTransport->local[2] = (DstIp >> 8) & 0xFF;
    gpTransport->local[3] =  DstIp & 0xFF;
    gpTransport->local[4] = (DstPort >> 8) & 0xFF;
    gpTransport->local[5] = DstPort & 0xFF;

    // Remote address is unknown at this point, zero it out.
    // The transport will fill it in later.
    gpTransport->remote[0] = 0;
    gpTransport->remote[1] = 0;
    gpTransport->remote[2] = 0;
    gpTransport->remote[3] = 0;
    gpTransport->remote[4] = 0;
    gpTransport->remote[5] = 0;

    // Now the socket
    gpTransport->sock = Socket;

    // Set our socket set to include only this socket.
    FD_ZERO (&socket_set);
    FD_SET  (gpTransport->sock, &socket_set);

    // Call netsnmp
    snmp_read (&socket_set);

    // Return something besides -1.  Would like to return bytes
    // read, but snmp_read is a void...
    return 0;
}


int 
bcmV2_using_openssl_check()
{
#if defined USE_OPENSSL
    printf("Auth Library = OPENSSL\n");
    return 0;
#elif defined USE_INTERNAL_MD5 
    printf("Auth Library = INTERNAL_MD5\n");
    return 1;
#else
    printf("Auth Library = NOT DEFINED\n");
    return 0;
#endif
}


int
bcmV2_do_scalar (netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests)
{
    netsnmp_request_info *request;
    netsnmp_variable_list *var;
    netsnmp_variable_list tmpVar;
//  char           *tmpVal[MAX_OID_LEN * sizeof(oid)];
    char           *tmpVal = NULL;
    size_t          tmpValLen;
    oid             tmpOid[MAX_OID_LEN];
    int             retVal = SNMP_ERR_NOERROR;
    int Loop;
    netsnmp_pdu *pdu = NULL;
    NetSnmpRawApiRec Raw;
    
    
    DEBUG ("bcmV2_do_scalar\n");
    
    // Populate a 'raw' API structure in case we need it.
    Raw.handler = handler;
    Raw.reginfo = reginfo;
    Raw.reqinfo = reqinfo;
    Raw.requests = requests;
    Raw.request = NULL;
    
    // I need a pointer to my PDU here.  I can get it from gpSession->myvoid.
    // Is there a better way?  There is a field for it in the 
    // netsnmp_agent_request_info structure, but it is NULL at this time.
    pdu = (netsnmp_pdu *) gpSession->myvoid;
    
    for ( request = requests; request; request = request->next )
    {
        var = request->requestvb;
        if ( request->processed != 0 )
            continue;

        memset (&tmpVar, 0, sizeof(netsnmp_variable_list));
        tmpVar.name = tmpOid;

        DEBUG ("bcmV2_do_scalar: root OID = ");
        for ( Loop = 0; Loop < reginfo->rootoid_len; Loop++ )
            DEBUG (".%d", reginfo->rootoid[Loop]);
        DEBUG ("\n");

        DEBUG ("                  req OID = ");
        for ( Loop = 0; Loop < request->requestvb->name_length; Loop++ )
            DEBUG (".%d", request->requestvb->name[Loop]);
        DEBUG ("\n");

        switch ( reqinfo->mode )
        {
        case MODE_GET:
            DEBUG ("  GET\n");
            
            // Note that v2GetObject will allocate storage for us.  We must
            // free it.
            retVal = v2GetObject (var, &tmpVal, &tmpValLen, reqinfo, reginfo->rootoid_len, &Raw);
            
            // If v2GetObject returns NO_OPINION then that means that someone
            // already processed the raw data.  Nothing left for us to do.
            if ( retVal == SNMP_NO_OPINION )
            {
                // Get the error code out of the Raw structure.
                //retVal = Raw.Something.Somewhere...
                DEBUG ("  NO_OPINION - externally processed.\n");
            }
            
            else if ( retVal == SNMP_ERR_NOERROR )
                snmp_set_var_typed_value(request->requestvb, request->requestvb->type, (u_char *) tmpVal, tmpValLen);
            /*
             * if V2 returns NOSUCH, netsnmp wants this handler
             * to return NOERR, but we netsnmp_set_request_error
             * to SNMP_NOSUCHOBJECT or SNMP_NOSUCHINSTANCE.  
             * Tricky, tricky!
             */
            else if ( retVal == SNMP_NO_SUCH_OBJECT )
            {
                DEBUG ("  NO_SUCH_OBJECT\n");
                netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
                retVal = SNMP_ERR_NOERROR;
            }
            else if ( retVal == SNMP_NO_SUCH_INSTANCE )
            {
                DEBUG ("  NO_SUCH_INSTANCE\n");
                netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
                retVal = SNMP_ERR_NOERROR;
            }
            else
                netsnmp_set_request_error(reqinfo, request, retVal);
                
            if (tmpVal)
            {
                free (tmpVal);
                tmpVal = NULL;
            }
            
            break;

        case MODE_GETNEXT:
            DEBUG ("  GETNEXT\n");

            // If the requested OID is a subtree of our root OID, then that
            // means that the request is for scalar.0, or perhaps scalar.1
            // or even scalar.2.3.4 or some such.  In any case, we know that
            // the correct response in this case is noSuchObject since there 
            // can only be exactly one instance of a scalar.  Note that somewhat
            // counter-intuitively, netsnmp_oid_is_subtree returns 0 if it is a 
            // subtree and 1 if not....  Also note that we must check the
            // OID lengths because if they're of equal length then they'll
            // be considered subtrees.
            if ( (request->requestvb->name_length > reginfo->rootoid_len)
                 &&  (netsnmp_oid_is_subtree (reginfo->rootoid, reginfo->rootoid_len, request->requestvb->name, request->requestvb->name_length) == 0) )
            {
                // If the requested OID is a subtree of our root OID, then that
                // means that the request is for scalar.0, or perhaps scalar.1
                // or even scalar.2.3.4 or some such.  In any case, we know that
                // the correct response in this case is noSuchObject.
                DEBUG ("  Next request for scalar.X.  noSuchObject.\n");
                retVal = SNMP_NO_SUCH_OBJECT;
            }

            // Otherwise, this might be a getnext request for our own root
            // OID.  For example, it might be a getnext on sysDescr.  In that
            // case, we simply set the request OID equal to our own OID and 
            // tack a 0 on the end for the instance, then get that from the
            // app.
            //
            // Alternately, we could be getting the result of a previous
            // getnext failure.  For example, if you request next on
            // sysDescr.1, we return noSuchObject above.  Then the request
            // for sysDescr.1 goes down to the next object in the list,
            // sysObjectID.  So in that case the response is the same as the
            // case above, get rootoid.0
            else
            {
                DEBUG ("  Next request for scalar, or previousScalar.X.  Get scalar.0.\n");

                tmpVar.name_length = reginfo->rootoid_len;
                memcpy((char *) tmpVar.name, (char *) reginfo->rootoid, reginfo->rootoid_len * sizeof(oid));
                tmpVar.name[tmpVar.name_length] = 0;
                tmpVar.name_length++;

                // Note that v2GetObject will allocate storage for us.  We must
                // free it.
                retVal = v2GetObject (&tmpVar, &tmpVal, &tmpValLen, reqinfo, reginfo->rootoid_len, &Raw);
            }

            if ( retVal == SNMP_ERR_NOERROR )
            {
                DEBUG ("  NOERROR\n");

                snmp_set_var_objid(var, tmpVar.name, tmpVar.name_length);
                snmp_set_var_typed_value(var, tmpVar.type, (u_char *) tmpVal, tmpValLen);
            }
            /*
             * if V2 returns NO_SUCH_OBJECT or NO_SUCH_INSTANCE on a NEXT, 
             * we actually return NOERROR but we don't call snmp_set_var_objid
             * pr snmp_set_var_typed_value.
             */
            else if ( retVal == SNMP_NO_SUCH_OBJECT )
            {
                DEBUG ("  NO_SUCH_OBJECT\n");
                retVal = SNMP_ERR_NOERROR;
            }
            // I don't THINK that we can ever possibly get this back from
            // V2 on a GETNEXT, but just in case...
            else if ( retVal == SNMP_NO_SUCH_INSTANCE )
            {
                DEBUG ("  NO_SUCH_INSTANCE\n");
                retVal = SNMP_ERR_NOERROR;
            }
            else /* retVal has a real error code */
            {
                DEBUG ("ERROR: %d\n", retVal);
                netsnmp_set_request_error(reqinfo, request, retVal);
            }
            
            if (tmpVal)
            {
                free (tmpVal);
                tmpVal = NULL;
            }
            break;

        // RESERVE1: check writability and type
        case MODE_SET_RESERVE1: // 0
            DEBUG ("  MODE_SET_RESERVE1 (aka VALIDATE)\n");
            retVal = v2ValidateSet (var->name,
                                    var->name_length,
                                    var->val.string,
                                    var->val_len,
                                    var->type, reqinfo,
                                    reginfo->rootoid_len);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;

        // RESERVE2: check values via TEST
        case MODE_SET_RESERVE2: // 1
            DEBUG ("  MODE_SET_RESERVE2 (aka TEST)\n");
            retVal = v2Test(var->name,
                            var->name_length,
                            var->val.string,
                            var->val_len,
                            var->type, reqinfo, 
                            reginfo->rootoid_len,
                            &Raw);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;

            // Note: constant values from snmp.h
        case MODE_SET_ACTION:   // 2
        case MODE_SET_FREE:     // 4
        case MODE_SET_UNDO:     // 5
            DEBUG ("  SET_%d - nothing to do.\n", reqinfo->mode);
            break;

        // COMMIT: do it to it!
        case MODE_SET_COMMIT:
            DEBUG("  MODE_SET_COMMIT (aka SET)\n");

            // Shouldn't be necessary to call v2ValidateSet here, that should've
            // been done prior to v2Test...

            /*
             * really do the set! 
             */
            retVal = v2Set(var->name,
                           var->name_length,
                           var->val.string,
                           var->val_len,
                           var->type, reqinfo, 
                           reginfo->rootoid_len,
                           &Raw);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;


        default:
            DEBUG ("problem encountered in _handler: unsupported mode\n");
            break;
        }

        // If we have already seen an error, break out of here and return.
        if ( retVal != SNMP_ERR_NOERROR )
        {
            DEBUG ("  error encountered!  buh-bye.\n");

            // Now, V2 might return any old error from AppSetObject.  Net-SNMP
            // seems to only expect an error of commitFailed from this set phase.
            // I guess we must be doing something peculiar in this regard since 
            // WindManage seems to expect the same thing.  But I digress.
            // Anyhoo, so now we need to call netsnmp_check_requests_status if we 
            // have an error, otherwise we won't get the error index set properly.  
            if ( reqinfo->mode == MODE_SET_COMMIT )
                netsnmp_check_requests_status (reqinfo->asp, requests, retVal);

            break;
        }
    }
    
    return retVal;
}

void bcmV2_free_vb_data (void *pData)
{
  DEBUG ("  bcmV2_free_vb_data: freeing VB data @ %p\n", pData);
  if (pData)
    free (pData);
}

int
bcmV2_do_tabular (netsnmp_mib_handler *handler,
                  netsnmp_handler_registration *reginfo,
                  netsnmp_agent_request_info *reqinfo,
                  netsnmp_request_info *requests)
{
    netsnmp_request_info *request;
    netsnmp_variable_list *var;
    netsnmp_variable_list tmpVar;
//  char           *tmpVal[MAX_OID_LEN * sizeof(oid)];
    char           *tmpVal = NULL;
    size_t          tmpValLen;
    oid             tmpOid[MAX_OID_LEN];
    int             retVal = SNMP_ERR_NOERROR;
    int VarCount = 0;
    netsnmp_pdu *pdu = NULL;
    NetSnmpRawApiRec Raw;


    {
        int Loop;

        DEBUG ("bcmV2_do_tabular for ");
        for ( Loop = 0; Loop < reginfo->rootoid_len; Loop++ )
            DEBUG (".%d", reginfo->rootoid[Loop]);
        DEBUG ("\n");
    }

    // Speed up 'set' processing, quickly skip unused set phases.
    if ( (reqinfo->mode == MODE_SET_ACTION)
         ||  (reqinfo->mode == MODE_SET_FREE)
         ||  (reqinfo->mode == MODE_SET_UNDO) )
    {
        DEBUG ("  Unhandled SET mode, quick return NO_ERROR\n");
        return SNMP_ERR_NOERROR;
    }

    // Populate a 'raw' API structure in case we need it.
    Raw.handler = handler;
    Raw.reginfo = reginfo;
    Raw.reqinfo = reqinfo;
    Raw.requests = requests;
    Raw.request = NULL;

    // I need a pointer to my PDU here.  I can get it from gpSession->myvoid.
    // Is there a better way?  There is a field for it in the
    // netsnmp_agent_request_info structure, but it is NULL at this time.
    pdu = (netsnmp_pdu *) gpSession->myvoid;

    for ( request = requests; request; request = request->next )
    {
        VarCount++;
        DEBUG ("  processing var %d\n", VarCount);

        Raw.request = request;

        var = request->requestvb;

        if ( request->processed != 0 )
        {
            DEBUG ("  already processed, continuing.\n");
            continue;
        }

        memset (&tmpVar, 0, sizeof(netsnmp_variable_list));
        tmpVar.name = tmpOid;

        /*
         * perform anything here that you need to do before each
         * request is processed.
         */

        switch ( reqinfo->mode )
        {
        case MODE_GET:
            DEBUG ("  GET\n");

            // Note that v2GetObject will allocate storage for us.  We must
            // free it.
            retVal = v2GetObject (var, &tmpVal, &tmpValLen, reqinfo, reginfo->rootoid_len + 2, &Raw);
            if ( retVal == SNMP_ERR_NOERROR )
                snmp_set_var_typed_value(request->requestvb, request->requestvb->type, (u_char *) tmpVal, tmpValLen);
            if ( tmpVal )
            {
                free (tmpVal);
                tmpVal = NULL;
            }
            /*
             * if V2 returns NOSUCH, netsnmp wants this handler
             * to return NOERR, but we netsnmp_set_request_error
             * to SNMP_NOSUCHOBJECT or SNMP_NOSUCHINSTANCE.
             * Tricky, tricky!
             */
            else if ( retVal == SNMP_NO_SUCH_OBJECT )
            {
                DEBUG ("  NO_SUCH_OBJECT\n");
                netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
                retVal = SNMP_ERR_NOERROR;
            }
            else if ( retVal == SNMP_NO_SUCH_INSTANCE )
            {
                DEBUG ("  NO_SUCH_INSTANCE\n");
                netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
                retVal = SNMP_ERR_NOERROR;
            }
            else
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;

        case MODE_GETNEXT:
            DEBUG ("  GETNEXT  on ");

            // Print OID
            {
                int Loop;
                for ( Loop = 0; Loop < request->requestvb->name_length; Loop++ )
                    DEBUG (".%d", request->requestvb->name[Loop]);
                DEBUG ("\n");
            }

            /*
             * Check to see if var is in this subtree. If not, just
             * get the first object.
             */

            // If the requested OID is not a subtree of our reginfo OID at all,
            // it's NO_SUCH_OBJECT.  Note that somewhat counter-intuitively,
            // netsnmp_oid_is_subtree returns 0 if OID2 is a subtree of OID1.
            if ( netsnmp_oid_is_subtree (reginfo->rootoid, reginfo->rootoid_len, request->requestvb->name, request->requestvb->name_length) )
            {
                DEBUG ("  Not for this handler.  Copying in my root OID.\n");

                // We now need to copy the reginfo OID into the tmpVar,
                // so lexigraphical ordering will proceed properly.  But first,
                // we must remember the original OID request so that if this
                // turns out to be an EndOfMibView situation, we can copy
                // it back in (PR 7435).
                if (request->requestvb->data == NULL)
                {
                  ObjectId *pStoreOID = (ObjectId *) malloc (sizeof(ObjectId));

                  DEBUG ("  Remembering original request OID @ %p.\n", pStoreOID);

                  memcpy (pStoreOID->pObjects, request->requestvb->name, request->requestvb->name_length * sizeof(unsigned long));
                  pStoreOID->Length = request->requestvb->name_length;
                  request->requestvb->data = pStoreOID;
                  request->requestvb->dataFreeHook = bcmV2_free_vb_data;
                }

                memcpy((char *) request->requestvb->name, (char *) reginfo->rootoid, reginfo->rootoid_len * sizeof(oid));
                request->requestvb->name_length = reginfo->rootoid_len;
            }

            // If we're a subtree and the length is at least 2 greater than our
            // reginfo OID, then that means we're dealing with a proper column.
            if ( request->requestvb->name_length >= reginfo->rootoid_len + 2 )
            {
                char *pView;

                DEBUG ("  In subtree.\n");

                tmpVar.name_length = request->requestvb->name_length;
                memcpy((char *) tmpVar.name, (char *) request->requestvb->name, request->requestvb->name_length * sizeof(oid));

                // Note that v2GetNextObject will allocate storage for us.
                // We must free it.
                retVal = v2GetNextObject (&tmpVar, reginfo->rootoid_len + 2, &tmpVal, &tmpValLen, reqinfo, &Raw);

                // PR 7176: we must check the view on the object returned by
                // BFC here, or else we can have lexigraphical ordering
                // problems on getbulks under certain conditions.  The view is
                // stored for us in the PDU securityName field if this is a V1
                // or V2 request, if it's V3 we must map it.
                if ( retVal == SNMP_ERR_NOERROR )
                {
                    if ( pdu->version == SNMP_VERSION_3 )
                    {
                        bcmV2_FindViewsFromSecurityName(pdu->securityName, &pView, NULL, NULL,
                                                        pdu->securityModel,
                                                        pdu->securityLevel);
                    }
                    else
                        pView = pdu->securityName;

                    DEBUG ("  Checking view %s for object...\n", pView);

                    if ( vacm_checkSubtree (pView, tmpVar.name, tmpVar.name_length) == VACM_NOTINVIEW )
                    {
                        DEBUG ("  OID is not in view %s, try again (NO_ACCESS).\n", pView);
                        retVal = SNMP_ERR_NOACCESS;
                    }
                    else
                        DEBUG ("  OID is in view, OK.\n");
                }
                // END PR 7176

                if ( (retVal == SNMP_ERR_NOSUCHNAME) || (retVal == SNMP_ERR_NOACCESS) )
                {
                    // PR 6762: check special case of our 'next' being for column
                    // 0xFFFFFFFF (4294967295) in which case don't move on to the
                    // next column, which would be column 0.
                    DEBUG ("  NOSUCHNAME or NOACCESS on column %u\n", tmpVar.name[reginfo->rootoid_len + 1]);
                    if ( tmpVar.name[reginfo->rootoid_len + 1] == 0xFFFFFFFF )
                    {
                        DEBUG ("  Hmmm, some sneaky test program is trying to trick us with a NEXT on column\n"
                               "  0xFFFFFFFF.  Thwart him!\n");
                    }
                    else
                    {
                        /*
                         * more columns to try.  Make the name_length as long as
                         * the rootoid + 2 (table.1.column) and increment the last
                         * element in the name, thus incrementing the column.
                         */

                        // If we have some columns which are not-accessible (as is often
                        // the case with index column(s)), V2 will return NO_ACCESS.  So
                        // as long as we keep getting NO_ACCESS keep incrementing the
                        // column field and look for the first accessible column.
                        do
                        {
                            if ( tmpVal )
                            {
                                free (tmpVal);
                                tmpVal = NULL;
                            }

                            tmpVar.name_length = reginfo->rootoid_len + 2;
                            tmpVar.name[tmpVar.name_length - 1]++;

                            DEBUG ("  Move on to column %u...\n", tmpVar.name[tmpVar.name_length - 1]);

                            // Note that v2GetNextObject will allocate storage for
                            // us.  We must free it.
                            retVal = v2GetNextObject (&tmpVar, tmpVar.name_length, &tmpVal, &tmpValLen, reqinfo, &Raw);

                            // PR 7176: we must check the view on the object returned by
                            // BFC here, or else we can have lexigraphical ordering
                            // problems on getbulks under certain conditions.  The view is
                            // stored for us in the PDU securityName field if this is a V1
                            // or V2 request, if it's V3 we must map it.
                            if ( retVal == SNMP_ERR_NOERROR )
                            {
                                if ( pdu->version == SNMP_VERSION_3 )
                                {
                                    bcmV2_FindViewsFromSecurityName(pdu->securityName, &pView, NULL, NULL,
                                                                    pdu->securityModel,
                                                                    pdu->securityLevel);
                                }
                                else
                                    pView = pdu->securityName;

                                DEBUG ("  Checking view %s for object...\n", pView);

                                if ( vacm_checkSubtree (pView, tmpVar.name, tmpVar.name_length) == VACM_NOTINVIEW )
                                {
                                    DEBUG ("  OID is not in view %s, try again (NO_ACCESS).\n", pView);
                                    retVal = SNMP_ERR_NOACCESS;
                                }
                                else
                                    DEBUG ("  OID is in view, OK.\n");
                            }
                            // END PR 7176
                        } while ( retVal == SNMP_ERR_NOACCESS );
                    }
                }
            }

            // Otherwise, we're in the subtree, but the OID requested is either
            // the length of the table, or 1 bigger.
            else
            {
                DEBUG ("  Short OID.  Get first column.\n");

                tmpVar.name_length = reginfo->rootoid_len;
                memcpy((char *) tmpVar.name, (char *) request->requestvb->name, request->requestvb->name_length * sizeof(oid));

                // If the requested OID length == the reginfo length in this
                // case, then we need to go ahead and tack on a '1' for the
                // entry OID before calling up to the app.
                if ( request->requestvb->name_length == reginfo->rootoid_len )
                {
                    DEBUG ("  Adding '1' for the entry.\n");
                    tmpVar.name[tmpVar.name_length] = 1;
                }

                // Try column 0 first.  If that doesn't fly, then keep
                // incrementing.
                tmpVar.name_length++;
                tmpVar.name[tmpVar.name_length] = 0;
                tmpVar.name_length++;

                // If we have some columns which are not-accessible (as is often
                // the case with index column(s)), V2 will return NO_ACCESS.  So
                // as long as we keep getting NO_ACCESS keep incrementing the
                // column field and look for the first accessible column.
                do
                {
                    tmpVar.name[tmpVar.name_length-1]++;
                    DEBUG ("  Trying column %d.\n", tmpVar.name[tmpVar.name_length-1]);

                    if ( tmpVal )
                    {
                        free (tmpVal);
                        tmpVal = NULL;
                    }

                    // Note that v2GetNextObject will allocate storage for us.
                    // We must free it.
                    retVal = v2GetNextObject (&tmpVar, tmpVar.name_length, &tmpVal, &tmpValLen, reqinfo, &Raw);
                } while ( retVal == SNMP_ERR_NOACCESS );
            }

            if ( retVal == SNMP_ERR_NOERROR )
            {
                int Loop;
                DEBUG ("  NOERROR\n");

                for ( Loop = 0; Loop < tmpVar.name_length; Loop++ )
                    DEBUG (".%d", tmpVar.name[Loop]);
                DEBUG ("\n");

                snmp_set_var_objid(var, tmpVar.name, tmpVar.name_length);
                snmp_set_var_typed_value(var, tmpVar.type, (u_char *) tmpVal, tmpValLen);
            }
            /*
             * if V2 returns NO_SUCH_OBJECT or NO_SUCH_INSTANCE on a NEXT,
             * we actually return NOERROR but we don't call snmp_set_var_objid
             * or snmp_set_var_typed_value.
             */
            else if ( retVal == SNMP_NO_SUCH_OBJECT )
            {
                DEBUG ("  NO_SUCH_OBJECT\n");
                retVal = SNMP_ERR_NOERROR;
            }
            // I don't THINK that we can ever possibly get this back from
            // V2 on a GETNEXT, but just in case...
            else if ( retVal == SNMP_NO_SUCH_INSTANCE )
            {
                DEBUG ("  NO_SUCH_INSTANCE\n");
                retVal = SNMP_ERR_NOERROR;
            }
            else /* retVal has a real error code */
            {
                DEBUG ("ERROR: %d\n", retVal);
                netsnmp_set_request_error(reqinfo, request, retVal);
            }

            if ( tmpVal )
            {
                free (tmpVal);
                tmpVal = NULL;
            }

            break;

            // RESERVE1: check writability and type
        case MODE_SET_RESERVE1: // 0
            DEBUG ("  MODE_SET_RESERVE1 (aka VALIDATE)\n");

            // Validate the set first
            retVal = v2ValidateSet (var->name,
                                    var->name_length,
                                    var->val.string,
                                    var->val_len,
                                    var->type, reqinfo,
                                    reginfo->rootoid_len + 2);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;

            // RESERVE2: check values via TEST
        case MODE_SET_RESERVE2: // 1
            DEBUG ("  MODE_SET_RESERVE2 (aka TEST)\n");
            retVal = v2Test(var->name,
                            var->name_length,
                            var->val.string,
                            var->val_len,
                            var->type, reqinfo,
                            reginfo->rootoid_len + 2,
                            &Raw);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;

            // Note: constant values from snmp.h
        case MODE_SET_ACTION:   // 2
        case MODE_SET_FREE:     // 4
        case MODE_SET_UNDO:     // 5
            DEBUG ("  SET_%d - nothing to do.\n", reqinfo->mode);
            break;

            // COMMIT: do it to it!
        case MODE_SET_COMMIT: // aka SET
            DEBUG("  SET_COMMIT (aka SET)\n");

            // Shouldn't be necessary to call v2ValidateSet here, that should've
            // been done prior to v2Test...

            /*
             * really do the set!
             */
            retVal = v2Set(var->name,
                           var->name_length,
                           var->val.string,
                           var->val_len,
                           var->type, reqinfo,
                           reginfo->rootoid_len + 2,
                           &Raw);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;


        default:
            DEBUG ("problem encountered in _handler: unsupported mode\n");
            break;
        }

        // If we have already seen an error, break out of here and return.
        if ( retVal != SNMP_ERR_NOERROR )
        {
            DEBUG ("  error encountered!  buh-bye.\n");

            // Now, V2 might return any old error from AppSetObject.  Net-SNMP
            // seems to only expect an error of commitFailed from this set phase.
            // I guess we must be doing something peculiar in this regard since
            // WindManage seems to expect the same thing.  But I digress.
            // Anyhoo, so now we need to call netsnmp_check_requests_status if we
            // have an error, otherwise we won't get the error index set properly.
            if ( reqinfo->mode == MODE_SET_COMMIT )
                netsnmp_check_requests_status (reqinfo->asp, requests, retVal);

            break;
        }
    }

    return retVal;
}


int
bcmV2_do_group (netsnmp_mib_handler *handler,
                 netsnmp_handler_registration *reginfo,
                 netsnmp_agent_request_info *reqinfo,
                 netsnmp_request_info *requests)
{
    netsnmp_request_info *request;
    netsnmp_variable_list *var;
    netsnmp_variable_list tmpVar;
//  char           *tmpVal[MAX_OID_LEN * sizeof(oid)];
    char           *tmpVal = NULL;
    size_t          tmpValLen;
    oid             tmpOid[MAX_OID_LEN];
    int             retVal = SNMP_ERR_NOERROR;
    int Loop;
    netsnmp_pdu *pdu = NULL;
    NetSnmpRawApiRec Raw;
    
    
    DEBUG ("bcmV2_do_group\n");
    
    // Populate a 'raw' API structure in case we need it.
    Raw.handler = handler;
    Raw.reginfo = reginfo;
    Raw.reqinfo = reqinfo;
    Raw.requests = requests;
    Raw.request = NULL;
    
    // I need a pointer to my PDU here.  I can get it from gpSession->myvoid.
    // Is there a better way?  There is a field for it in the 
    // netsnmp_agent_request_info structure, but it is NULL at this time.
    pdu = (netsnmp_pdu *) gpSession->myvoid;
    
    for ( request = requests; request; request = request->next )
    {
        var = request->requestvb;
        if ( request->processed != 0 )
            continue;

        memset (&tmpVar, 0, sizeof(netsnmp_variable_list));
        tmpVar.name = tmpOid;

        DEBUG ("bcmV2_do_group: root OID = ");
        for ( Loop = 0; Loop < reginfo->rootoid_len; Loop++ )
            DEBUG (".%d", reginfo->rootoid[Loop]);
        DEBUG ("\n");

        DEBUG ("                  req OID = ");
        for ( Loop = 0; Loop < request->requestvb->name_length; Loop++ )
            DEBUG (".%d", request->requestvb->name[Loop]);
        DEBUG ("\n");

        switch ( reqinfo->mode )
        {
        case MODE_GET:
            DEBUG ("  GET\n");
            
            // Note that v2GetObject will allocate storage for us.  We must
            // free it.
            retVal = v2GetObject (var, &tmpVal, &tmpValLen, reqinfo, reginfo->rootoid_len, &Raw);
            
            // If v2GetObject returns NO_OPINION then that means that someone
            // already processed the raw data.  Nothing left for us to do.
            if ( retVal == SNMP_NO_OPINION )
            {
                // Get the error code out of the Raw structure.
                //retVal = Raw.Something.Somewhere...
                DEBUG ("  NO_OPINION - externally processed.\n");
            }
            
            else if ( retVal == SNMP_ERR_NOERROR )
                snmp_set_var_typed_value(request->requestvb, request->requestvb->type, (u_char *) tmpVal, tmpValLen);
            /*
             * if V2 returns NOSUCH, netsnmp wants this handler
             * to return NOERR, but we netsnmp_set_request_error
             * to SNMP_NOSUCHOBJECT or SNMP_NOSUCHINSTANCE.  
             * Tricky, tricky!
             */
            else if ( retVal == SNMP_NO_SUCH_OBJECT )
            {
                DEBUG ("  NO_SUCH_OBJECT\n");
                netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
                retVal = SNMP_ERR_NOERROR;
            }
            else if ( retVal == SNMP_NO_SUCH_INSTANCE )
            {
                DEBUG ("  NO_SUCH_INSTANCE\n");
                netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
                retVal = SNMP_ERR_NOERROR;
            }
            else
                netsnmp_set_request_error(reqinfo, request, retVal);
                
            if (tmpVal)
            {
                free (tmpVal);
                tmpVal = NULL;
            }
            
            break;

        case MODE_GETNEXT:
            DEBUG ("  GETNEXT\n");
/*
            // If the requested OID is a subtree of our root OID, then that
            // means that the request is for scalar.0, or perhaps scalar.1
            // or even scalar.2.3.4 or some such.  In any case, we know that
            // the correct response in this case is noSuchObject since there 
            // can only be exactly one instance of a scalar.  Note that somewhat
            // counter-intuitively, netsnmp_oid_is_subtree returns 0 if it is a 
            // subtree and 1 if not....  Also note that we must check the
            // OID lengths because if they're of equal length then they'll
            // be considered subtrees.
            if ( (request->requestvb->name_length > reginfo->rootoid_len)
                 &&  (netsnmp_oid_is_subtree (reginfo->rootoid, reginfo->rootoid_len, request->requestvb->name, request->requestvb->name_length) == 0) )
            {
                // If the requested OID is a subtree of our root OID, then that
                // means that the request is for scalar.0, or perhaps scalar.1
                // or even scalar.2.3.4 or some such.  In any case, we know that
                // the correct response in this case is noSuchObject.
                DEBUG ("  Next request for scalar.X.  noSuchObject.\n");
                retVal = SNMP_NO_SUCH_OBJECT;
            }

            // Otherwise, this might be a getnext request for our own root
            // OID.  For example, it might be a getnext on sysDescr.  In that
            // case, we simply set the request OID equal to our own OID and 
            // tack a 0 on the end for the instance, then get that from the
            // app.
            //
            // Alternately, we could be getting the result of a previous
            // getnext failure.  For example, if you request next on
            // sysDescr.1, we return noSuchObject above.  Then the request
            // for sysDescr.1 goes down to the next object in the list,
            // sysObjectID.  So in that case the response is the same as the
            // case above, get rootoid.0
            else
*/            
            {
                DEBUG ("  Next request for scalar.\n");

                tmpVar.name_length = request->requestvb->name_length;
                memcpy((char *) tmpVar.name, (char *) request->requestvb->name, request->requestvb->name_length * sizeof(oid));

                // Note that v2GetNextObject will allocate storage for us.  We must
                // free it.
                retVal = v2GetNextObject (&tmpVar, reginfo->rootoid_len + 1, &tmpVal, &tmpValLen, reqinfo, &Raw);
            }

            if ( retVal == SNMP_ERR_NOERROR )
            {
                DEBUG ("  NOERROR\n");

                snmp_set_var_objid(var, tmpVar.name, tmpVar.name_length);
                snmp_set_var_typed_value(var, tmpVar.type, (u_char *) tmpVal, tmpValLen);
            }
            /*
             * if V2 returns NO_SUCH_OBJECT or NO_SUCH_INSTANCE on a NEXT, 
             * we actually return NOERROR but we don't call snmp_set_var_objid
             * pr snmp_set_var_typed_value.
             */
            else if ( retVal == SNMP_NO_SUCH_OBJECT )
            {
                DEBUG ("  NO_SUCH_OBJECT\n");
                retVal = SNMP_ERR_NOERROR;
            }
            // I don't THINK that we can ever possibly get this back from
            // V2 on a GETNEXT, but just in case...
            else if ( retVal == SNMP_NO_SUCH_INSTANCE )
            {
                DEBUG ("  NO_SUCH_INSTANCE\n");
                retVal = SNMP_ERR_NOERROR;
            }
            else /* retVal has a real error code */
            {
                DEBUG ("ERROR: %d\n", retVal);
                netsnmp_set_request_error(reqinfo, request, retVal);
            }
            
            if (tmpVal)
            {
                free (tmpVal);
                tmpVal = NULL;
            }
            break;

        // RESERVE1: check writability and type
        case MODE_SET_RESERVE1: // 0
            DEBUG ("  MODE_SET_RESERVE1 (aka VALIDATE)\n");
            retVal = v2ValidateSet (var->name,
                                    var->name_length,
                                    var->val.string,
                                    var->val_len,
                                    var->type, reqinfo,
                                    reginfo->rootoid_len);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;

        // RESERVE2: check values via TEST
        case MODE_SET_RESERVE2: // 1
            DEBUG ("  MODE_SET_RESERVE2 (aka TEST)\n");
            retVal = v2Test(var->name,
                            var->name_length,
                            var->val.string,
                            var->val_len,
                            var->type, reqinfo, 
                            reginfo->rootoid_len, 
                            &Raw);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;

            // Note: constant values from snmp.h
        case MODE_SET_ACTION:   // 2
        case MODE_SET_FREE:     // 4
        case MODE_SET_UNDO:     // 5
            DEBUG ("  SET_%d - nothing to do.\n", reqinfo->mode);
            break;

        // COMMIT: do it to it!
        case MODE_SET_COMMIT:
            DEBUG("  MODE_SET_COMMIT (aka SET)\n");

            // Shouldn't be necessary to call v2ValidateSet here, that should've
            // been done prior to v2Test...

            /*
             * really do the set! 
             */
            retVal = v2Set(var->name,
                           var->name_length,
                           var->val.string,
                           var->val_len,
                           var->type, reqinfo, 
                           reginfo->rootoid_len,
                           &Raw);
            if ( retVal != SNMP_ERR_NOERROR )
                netsnmp_set_request_error(reqinfo, request, retVal);
            break;


        default:
            DEBUG ("problem encountered in _handler: unsupported mode\n");
            break;
        }

        // If we have already seen an error, break out of here and return.
        if ( retVal != SNMP_ERR_NOERROR )
        {
            DEBUG ("  error encountered!  buh-bye.\n");

            // Now, V2 might return any old error from AppSetObject.  Net-SNMP
            // seems to only expect an error of commitFailed from this set phase.
            // I guess we must be doing something peculiar in this regard since 
            // WindManage seems to expect the same thing.  But I digress.
            // Anyhoo, so now we need to call netsnmp_check_requests_status if we 
            // have an error, otherwise we won't get the error index set properly.  
            if ( reqinfo->mode == MODE_SET_COMMIT )
                netsnmp_check_requests_status (reqinfo->asp, requests, retVal);

            break;
        }
    }
    
    return retVal;
}


// Convert a V2/BFC style MibObject list to a Net-SNMP varbind list.
// Note that this function allocates storage, it's up to the caller to free
// it.
netsnmp_variable_list *bcmV2_get_variable_list (MibObject *pObjects)
{
    int VbCount = 0;
    netsnmp_variable_list *pVB = NULL, *pPrevVB = NULL, *pFirstVB = NULL;
    MibObject *pMO = pObjects;
    int varLen;
//  u_char var[STRINGMAX]; // How big is this?  Am I risking my stack?
    char *var = NULL;


    DEBUG ("bcmV2_get_variable_list\n");

    while ( pMO )
    {
        DEBUG ("  Converting object %d\n", ++VbCount);

        // Allocate storage
        pVB = SNMP_MALLOC_STRUCT(variable_list);

        // Zero it out
        memset (pVB, 0, sizeof(netsnmp_variable_list));

        // Set OID and value.  OID first.
        snmp_set_var_objid(pVB, pMO->OID.pObjects, pMO->OID.Length);

        // Now value.  Note that extractV2VarBind will allocate storage.
        // It's up to us to free it.
        varLen = extractV2VarBind (&(pMO->VB), (void **)&var);
        snmp_set_var_typed_value(pVB, pMO->VB.Type, (u_char *) var, varLen); 
        
        // Free storage from extractV2VarBind.
        if (var)
        {
          free (var);
          var = NULL;
        }

        // If this is the first VB, remember it so we can return it.
        if ( pFirstVB == NULL )
            pFirstVB = pVB;

        // If this is not the first item on the list, set the previous
        // item's next pointer to the new structure.
        if ( pPrevVB )
            pPrevVB->next_variable = pVB;

        // Remember the new structure for next pass.
        pPrevVB = pVB;

        // Next in the chain.
        pMO = pMO->pNext;
    };

    DEBUG ("  Done!\n");

    return pFirstVB;
}

/*\
 * This is largely lifted from create_trap_session in agent_trap.c 
 * Note that this returns an allocated session pointer which must be freed
 * by the caller via snmp_close.  Note that it's OK for pCommunity to be
 * NULL if this session is used for an inform.
\*/
netsnmp_session *bcmV2_get_trap_session (const unsigned char *pFromIp, const unsigned char *pToIp, int ToPort, const char *pCommunity, int SnmpVersion)
{
    netsnmp_session session, *sesp;
    char           *peername = NULL;


    DEBUG ("bcmV2_get_trap_session.  From=%p, To=%p (%d), Community=%p, Version=%d\n",
           pFromIp, pToIp, ToPort, pCommunity, SnmpVersion);

    // Longest possible string here would be something like
    // udp:123.456.789.123:65535 123.456.789.123:65535
    // which would be 48 bytes in size.  So get 64.
    if ( (peername = (char *) malloc(64)) == NULL )
    {
        DEBUG ("  couldn't allocate peername!\n");
        return NULL;
    }
    else
    {
        DEBUG ("  got peername @ %p\n", peername);

        // Format the peername as expected by our transport domain.
        // udp:localIp:localPort remoteIp:remotePort
        // example: "udp:10.24.5.22:0 10.24.65.57:162"
        sprintf(peername, "udp:%d.%d.%d.%d:0 %d.%d.%d.%d:%d", 
                pFromIp[0],pFromIp[1],pFromIp[2],pFromIp[3],
                pToIp[0],pToIp[1],pToIp[2],pToIp[3], ToPort);

        DEBUG ("  peername = '%s'\n", peername);
    }

    memset(&session, 0, sizeof(netsnmp_session));
    session.peername = peername;
    session.version = SnmpVersion;
    if ( pCommunity )
    {
        session.community_len = strlen(pCommunity);

        // Cast away compiler warning...
        session.community = (unsigned char *)pCommunity;
    }

    sesp = snmp_open(&session);
    free(peername);

    /*
     * diagnose snmp_open errors with the input netsnmp_session pointer 
     */
    if ( sesp == NULL )
        DEBUG ("  snmp_open failed on %p", &session);

    return sesp;
}

/* returns 0 on success, -1 on failure */
int bcmV2_send_V2_trap (const unsigned char *pFromIp, const unsigned char *pToIp, int ToPort, const char *pCommunity,
                        oid *pTrapOID, int TrapOIDLength, time_t sysUpTime,
                        netsnmp_variable_list *vars)
{
    static oid sysUpTime_oid[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
    static oid snmpTrapOID_oid[] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};
    netsnmp_variable_list uptime_var, snmptrap_var;
    netsnmp_variable_list *v2_vars;
    netsnmp_pdu    *template_pdu;
    in_addr_t      *pdu_in_addr_t;
    netsnmp_session *pTrapSession = NULL;


    DEBUG ("bcmV2_send_V2_trap\n");

    /*
     * Initialise SNMPv2 required variables: sysUpTime and trap OID
     */

    /* sysUpTime */

    memset(&uptime_var, 0, sizeof(netsnmp_variable_list));
    snmp_set_var_objid(&uptime_var, sysUpTime_oid, OID_LENGTH(sysUpTime_oid));
    snmp_set_var_value(&uptime_var, (u_char *) &sysUpTime, sizeof(sysUpTime));
    uptime_var.type = ASN_TIMETICKS;
    uptime_var.next_variable = &snmptrap_var;

    /* trap OID */

    memset(&snmptrap_var, 0, sizeof(netsnmp_variable_list));
    snmp_set_var_objid(&snmptrap_var, snmpTrapOID_oid, OID_LENGTH(snmpTrapOID_oid));
    snmp_set_var_value(&snmptrap_var, (u_char *) pTrapOID, TrapOIDLength * sizeof(oid));
    snmptrap_var.type = ASN_OBJECT_ID;

    /* Now attach VB list */

    if ( vars )
    {
        DEBUG ("  Attaching variable list @ %p\n", vars);
        snmptrap_var.next_variable = vars;
    }
    else
    {
        DEBUG ("  No variable list.\n");
        snmptrap_var.next_variable = NULL;
    }

    v2_vars = &uptime_var;

    /*
     *  Create a template PDU, ready for sending
     */

    template_pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
    if ( template_pdu == NULL )
    {
        DEBUG ("  snmp_pdu_create returned NULL!\n");
        return -1;
    }
    else
        DEBUG ("  got PDU @ %p\n", template_pdu);

    /* UNFINISHED: What generic and specific values should we use for SNMPv2? */
    template_pdu->trap_type = -1;
    template_pdu->specific_type = 0;

    /* No enterprise info for SNMPv2 trap */
    template_pdu->enterprise = NULL;
    template_pdu->enterprise_length = 0;

    /* UNFINISHED: do I need this since we didn't clone enterprise? */    
    template_pdu->flags |= UCD_MSG_FLAG_FORCE_PDU_COPY;

    /* Load local addr from supplied args */
    pdu_in_addr_t = (in_addr_t *) template_pdu->agent_addr;
    *pdu_in_addr_t = htonl(*pFromIp);
    template_pdu->time = sysUpTime;
    template_pdu->variables = v2_vars;

    /* Get a session based on our input parameters */
    pTrapSession = bcmV2_get_trap_session (pFromIp, pToIp, ToPort, pCommunity, SNMP_VERSION_2c);

    if ( pTrapSession == NULL )
    {
        DEBUG ("  bcmV2_get_trap_session failed!\n");
        return -1;
    }
    else
        DEBUG ("  got session @ %p\n", pTrapSession);

    /* Send the trap.  Bummer that send_trap_to_sess is a void, so we must assume success. */
    send_trap_to_sess(pTrapSession, template_pdu);

    DEBUG ("  send_trap_to_sess complete.  success?  calling callbacks.\n");

    /*
     * send stuff to registered callbacks for V2 traps.
     */
    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION, SNMPD_CALLBACK_SEND_TRAP2, template_pdu);

    // Close the session.
    DEBUG ("  closing session\n");
    snmp_close (pTrapSession);

    // Set variables to NULL so they won't get called below.
    // The 'vars' list will be freed in NotifyAgnt.c where it was created.
    template_pdu->variables = NULL;

    // Free the PDU
    DEBUG ("  freeing PDU\n");
    snmp_free_pdu(template_pdu);

    return 0;
}


int bcmV2_send_V1_trap (const unsigned char *pFromIp, const unsigned char *pToIp, int ToPort, const char *pCommunity,
                        int generic, int specific, oid *enterprise, int enterprise_length,
                        time_t sysUpTime,
                        netsnmp_variable_list *vars)
{
    netsnmp_pdu    *template_pdu;
    in_addr_t      *pdu_in_addr_t;
    netsnmp_session *pTrapSession = NULL;


    DEBUG ("bcmV2_send_V1_trap\n");

    /*
     * SNMPv1 traps have no sysUpTime or trap OID VBs.
     */

    /*
     *  Create a template PDU, ready for sending
     */

    template_pdu = snmp_pdu_create(SNMP_MSG_TRAP);
    if ( template_pdu == NULL )
    {
        DEBUG ("  snmp_pdu_create returned NULL!\n");
        return -1;
    }
    else
        DEBUG ("  got PDU @ %p\n", template_pdu);

    /* Set trap type: generic and specific. */
    template_pdu->trap_type = generic;
    template_pdu->specific_type = specific;

    /* Load enterprise from supplied args */
    template_pdu->enterprise = enterprise;
    template_pdu->enterprise_length = enterprise_length;

    template_pdu->flags |= UCD_MSG_FLAG_FORCE_PDU_COPY;

    /* Load local addr from supplied args */
    pdu_in_addr_t = (in_addr_t *) template_pdu->agent_addr;
    *pdu_in_addr_t = htonl(*pFromIp);
    template_pdu->time = sysUpTime;

    /* Load variable bindings list from args */
    template_pdu->variables = vars;

    /* Set command */
    template_pdu->command = SNMP_MSG_TRAP;

    /* Get a session based on our input parameters */
    pTrapSession = bcmV2_get_trap_session (pFromIp, pToIp, ToPort, pCommunity, SNMP_VERSION_1);

    if ( pTrapSession == NULL )
    {
        DEBUG ("  bcmV2_get_trap_session failed!\n");
        return -1;
    }
    else
        DEBUG ("  got session @ %p\n", pTrapSession);

    /* Send the trap.  Bummer that send_trap_to_sess is a void, so we must assume success. */
    send_trap_to_sess(pTrapSession, template_pdu);

    DEBUG ("  send_trap_to_sess complete.  success?  calling callbacks.\n");

    /*
     * send stuff to registered callbacks for V1 traps.
     */

    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION, SNMPD_CALLBACK_SEND_TRAP1, template_pdu);

    // Close the session.
    DEBUG ("  closing session\n");
    snmp_close (pTrapSession);

    // Set variables to NULL so they won't get called below.
    // The 'vars' list will be freed in NotifyAgnt.c where it was created.
    template_pdu->variables = NULL;
    
    // Clear enterprise so we don't free it, since it's being used directly
    // instead of copied.
    template_pdu->enterprise = NULL;
    template_pdu->enterprise_length = 0;

    // Free the PDU
    DEBUG ("  freeing PDU\n");
    snmp_free_pdu(template_pdu);

    return 0;
}


void bcmV2_agent_show ()
{
    printf ("  Session = %p\n", gpSession);
    printf ("  Transport = %p\n", gpTransport);
    printf ("  Auth fail traps enabled = %d\n", snmp_enableauthentraps);
    
    printf ("  SNMP statistics:\n");
    printf ("    snmpInPkts              = %u\n", snmp_get_statistic (STAT_SNMPINPKTS));
    printf ("    snmpOutPkts             = %u\n", snmp_get_statistic (STAT_SNMPOUTPKTS));
    printf ("    snmpInBadVersions       = %u\n", snmp_get_statistic (STAT_SNMPINBADVERSIONS));
    printf ("    snmpInBadCommunityNames = %u\n", snmp_get_statistic (STAT_SNMPINBADCOMMUNITYNAMES));
    printf ("    snmpInBadCommunityUses  = %u\n", snmp_get_statistic (STAT_SNMPINBADCOMMUNITYUSES));
    printf ("    snmpInASNParseErrs      = %u\n", snmp_get_statistic (STAT_SNMPINASNPARSEERRS));
    printf ("    snmpInTooBigs           = %u\n", snmp_get_statistic (STAT_SNMPINTOOBIGS));
    printf ("    snmpInNoSuchNames       = %u\n", snmp_get_statistic (STAT_SNMPINNOSUCHNAMES));
    printf ("    snmpInBadValues         = %u\n", snmp_get_statistic (STAT_SNMPINBADVALUES));
    printf ("    snmpInReadOnlys         = %u\n", snmp_get_statistic (STAT_SNMPINREADONLYS));
    printf ("    snmpInGenErrs           = %u\n", snmp_get_statistic (STAT_SNMPINGENERRS));
    printf ("    snmpInTotalReqVars      = %u\n", snmp_get_statistic (STAT_SNMPINTOTALREQVARS));
    printf ("    snmpInTotalSetVars      = %u\n", snmp_get_statistic (STAT_SNMPINTOTALSETVARS));
    printf ("    snmpInGetRequests       = %u\n", snmp_get_statistic (STAT_SNMPINGETREQUESTS));
    printf ("    snmpInGetNexts          = %u\n", snmp_get_statistic (STAT_SNMPINGETNEXTS));
    printf ("    snmpInSetRequests       = %u\n", snmp_get_statistic (STAT_SNMPINSETREQUESTS));
    printf ("    snmpInGetResponses      = %u\n", snmp_get_statistic (STAT_SNMPINGETRESPONSES));
    printf ("    snmpInTraps             = %u\n", snmp_get_statistic (STAT_SNMPINTRAPS));
    printf ("    snmpOutTooBigs          = %u\n", snmp_get_statistic (STAT_SNMPOUTTOOBIGS));
    printf ("    snmpOutNoSuchNames      = %u\n", snmp_get_statistic (STAT_SNMPOUTNOSUCHNAMES));
    printf ("    snmpOutBadValues        = %u\n", snmp_get_statistic (STAT_SNMPOUTBADVALUES));
    printf ("    snmpOutGenErrs          = %u\n", snmp_get_statistic (STAT_SNMPOUTGENERRS));
    printf ("    snmpOutGetRequests      = %u\n", snmp_get_statistic (STAT_SNMPOUTGETREQUESTS));
    printf ("    snmpOutGetNexts         = %u\n", snmp_get_statistic (STAT_SNMPOUTGETNEXTS));
    printf ("    snmpOutSetRequests      = %u\n", snmp_get_statistic (STAT_SNMPOUTSETREQUESTS));
    printf ("    snmpOutGetResponses     = %u\n", snmp_get_statistic (STAT_SNMPOUTGETRESPONSES));
    printf ("    snmpOutTraps            = %u\n", snmp_get_statistic (STAT_SNMPOUTTRAPS));
    printf ("    snmpSilentDrops         = %u\n", snmp_get_statistic (STAT_SNMPSILENTDROPS));
    printf ("    snmpProxyDrops          = %u\n", snmp_get_statistic (STAT_SNMPPROXYDROPS));
}

//Note: the spec says we need to only support 10 TLV-38s: section 3.6 docsisV3NotificationReceiver
#define _MAX_TAGS 10
netsnmp_session *
bcmV2_get_target_sessions(char *taglist, TargetFilterFunction * filterfunct, void *filterArg,  const unsigned char *pFromIp)
{
    char fromToBuf[14];
    netsnmp_session *ret, thissess;
    struct targetAddrTable_struct *targaddrs;
    //char            buf[SPRINT_MAX_LEN];
    //char            tags[MAX_TAGS][SPRINT_MAX_LEN], *cp;
    char            buf[255];
    static char     tags[_MAX_TAGS][255];
    char            *cp;
    int             numtags = 0, i;
    static struct targetParamTable_struct *param;

    ret = NULL;
    DEBUGMSGTL(("target_sessions", "looking for: %s\n", taglist));
    for ( cp = taglist; cp && numtags < _MAX_TAGS; )
    {
        cp = copy_nword(cp, tags[numtags], sizeof(tags[numtags]));
        DEBUGMSGTL(("target_sessions", " for: %d=%s\n", numtags,
                    tags[numtags]));
        numtags++;
    }

    for ( targaddrs = get_addrTable(); targaddrs;
        targaddrs = targaddrs->next )
    {

        /*
         * legal row? 
         */
        if ( targaddrs->tDomain == NULL ||
             targaddrs->tAddress == NULL ||
             targaddrs->rowStatus != SNMP_ROW_ACTIVE )
        {
            DEBUGMSGTL(("target_sessions", "  which is not ready yet\n"));
            continue;
        }

        if ( netsnmp_tdomain_support
             (targaddrs->tDomain, targaddrs->tDomainLen, NULL, NULL) == 0 )
        {
            snmp_log(LOG_ERR,
                     "unsupported domain for target address table entry %s\n",
                     targaddrs->name);
            /* PR6554, add continue here - if no good domain, no good row */
            continue;
        }

        /*
         * check tag list to see if we match 
         */
        if ( targaddrs->tagList )
        {
            /*
             * loop through tag list looking for requested tags 
             */
            for ( cp = targaddrs->tagList; cp; )
            {
                cp = copy_nword(cp, buf, sizeof(buf));
                for ( i = 0; i < numtags; i++ )
                {
                    if ( strcmp(buf, tags[i]) == 0 )
                    {
                        /*
                         * found a valid target table entry 
                         */
                        DEBUGMSGTL(("target_sessions", "found one: %s\n",
                                    tags[i]));

                        if ( targaddrs->sess )
                        {
                            // This function returns a list of session pointers to send a notify to.
                            // This function is called once for each tag, e.g., once for @config_inform,
                            // once for @config_trap.  For example, if there are three rows with tag
                            // @config_trap, it returns a sess pointer with all three sessions for all
                            // three rows (sess pointer has a next*).  If this has been done once, the
                            // sessions may have a link to another session in it's next*.  This may cause
                            // a problem, especially if a row is not active anymore.  So, just set the
                            // next* to NULL.  We are not deleting sessions, just unlinking them.
                            // printf("setting targaddrs->sess->next to NULL \n");
                            targaddrs->sess->next = NULL;

                        }


                        if ( targaddrs->params )
                        {
                            param = get_paramEntry(targaddrs->params);
                            if ( !param
                                 || param->rowStatus != SNMP_ROW_ACTIVE )
                            {
                                /*
                                 * parameter entry must exist and be active 
                                 */
                                continue;
                            }
                        }
                        else
                        {
                            /*
                             * parameter entry must be specified 
                             */
                            continue;
                        }

                        /*
                         * last chance for caller to opt-out.  Call
                         * filtering function 
                         */
                        if ( filterfunct && (*(filterfunct)) ((void *)targaddrs, (void *)param,
                                                              filterArg) )
                        {
                            continue;
                        }

                        if ( targaddrs->storageType != ST_READONLY &&
                             targaddrs->sess &&
                             param->updateTime >=
                             targaddrs->sessionCreationTime )
                        {
                            /*
                             * parameters have changed, nuke the old session 
                             */
                            
                            /*
                            if ( targaddrs->storageType != ST_READONLY )
                                printf(" 1212 targaddrs->storageType not readonly\n");
                            if ( targaddrs->sess )
                                printf(" 1212 targaddrs->sess\n");
                            if ( param->updateTime >= targaddrs->sessionCreationTime )
                                printf(" 1212 uptime>sessCreation time\n");
                            printf(" parameters have changed nuke old session -WHY ARE WE DOING THIS?\n");
                            */
                            snmp_close(targaddrs->sess);
                            targaddrs->sess = NULL;
                        }

                        /*
                         * target session already exists? 
                         */
                        if ( targaddrs->sess == NULL )
                        {
                            /*
                             * create an appropriate snmp session and add
                             * it to our return list 
                             */
                            netsnmp_transport *t = NULL;

                            DEBUGMSGOID(("my domain udp",
                                         targaddrs->tDomain,
                                         targaddrs->tDomainLen));

                            //build a 12 byte buffer containing source ipAddress and port and 
                            //dest ipAddress and port
                            memset(fromToBuf, '\0',sizeof(fromToBuf));
                            memcpy(fromToBuf,pFromIp,4);  
                            fromToBuf[4] = 0x00;
                            fromToBuf[5] = 0xA3; // 2 bytes of source port
                            memcpy(&(fromToBuf[6]), (targaddrs->tAddress), 6);

                            //DumpBuf("fromtoBuf is ", fromToBuf ,12,'x');
                            //printf("calling netsnmp_tdomain_transport_oid\n");

                            t = netsnmp_tdomain_transport_oid(targaddrs->tDomain,
                                                              targaddrs->tDomainLen,
                                                              (u_char *)fromToBuf,//targaddrs->tAddress,
                                                              12,//targaddrs->tAddressLen,
                                                              0);

                            if ( t == NULL )
                            {
                                DEBUGMSGTL(("target_sessions",
                                            "bad dest \""));
                                DEBUGMSGOID(("target_sessions",
                                             targaddrs->tDomain,
                                             targaddrs->tDomainLen));
                                DEBUGMSG(("target_sessions", "\", \""));
                                DEBUGMSGHEX(("target_sessions",
                                             targaddrs->tAddress,
                                             targaddrs->tAddressLen));
                                DEBUGMSG(("target_sessions", "\n"));
                                continue;
                            }
                            else
                            {
                                char           *dst_str =
                                t->f_fmtaddr(t, NULL, 0);
                                if ( dst_str != NULL )
                                {
                                    DEBUGMSGTL(("target_sessions",
                                                "  to: %s\n", dst_str));
                                    free(dst_str);
                                }
                            }
                            memset(&thissess, 0, sizeof(thissess));
                            thissess.timeout = (targaddrs->timeout) * 1000;
                            thissess.retries = targaddrs->retryCount;
                            DEBUGMSGTL(("target_sessions",
                                        "timeout: %d -> %d\n",
                                        targaddrs->timeout,
                                        thissess.timeout));

                            if ( param->mpModel == SNMP_VERSION_3 &&
                                 param->secModel != 3 )
                            {
                                snmp_log(LOG_ERR,
                                         "unsupported model/secmodel combo for target %s\n",
                                         targaddrs->name);
                                /*
                                 * XXX: memleak 
                                 */
                                netsnmp_transport_free(t);
                                continue;
                            }
                            thissess.version = param->mpModel;

                            if ( param->mpModel == SNMP_VERSION_3 )
                            {
                                thissess.securityName = param->secName;
                                thissess.securityNameLen =
                                strlen(thissess.securityName);
                                thissess.securityLevel = param->secLevel;
                            }
                            else
                            {
                                thissess.community =
                                (u_char *) strdup(param->secName);
                                thissess.community_len =
                                strlen((char *) thissess.community);
                            }


                            //brcm_mod add the security model
                            thissess.securityModel = param->secModel;

                            DEBUG("CREATE TARGET SESSION <%p>:\n",&thissess);
                            DEBUG("   snmp version = <%d>\n",param->mpModel);
                            DEBUG(" security level = <%d>\n",thissess.securityLevel);
                            DEBUG(" security name  = <%s>\n",thissess.securityName);
                            DEBUG(" security model = <%d>\n",thissess.securityModel);


                            targaddrs->sess = snmp_add(&thissess, t,
                                                       NULL, NULL);
                            targaddrs->sessionCreationTime = time(NULL);
                        }
                        if ( targaddrs->sess )
                        {
                            if ( ret )
                            {
                                targaddrs->sess->next = ret;
                            }
                            ret = targaddrs->sess;
                        }
                        else
                        {
                            ret = NULL;
                            snmp_sess_perror("target session", &thissess);
                        }
                    }
                }
            }
        }
    }
    return ret;
}

/********************************************************************
* Function Name: bcmv2_FindViewsFromSecurityName()
* Returns: 0 = no error
*          1 = error
* Description:  Given the securityName, find the associated read, write,
*   and notify views.                           
*********************************************************************/
int bcmV2_FindViewsFromSecurityName(char   *securityName,
                              char   **readView,
                              char   **writeView,
                              char   **notifyView,
                              int    secModel,
                              int    secLevel)
{
    struct vacm_accessEntry *accessEntry;
    struct vacm_groupEntry  *groupEntry;
    
    DEBUG("Starting FindViewsFromSecurityName: securityName <%s>, secModel <%d>, secLevel <%d>\n",securityName, secModel, secLevel); 

    //1. Find the security name in the vacmSecurityToGroupTable
    groupEntry = vacm_getGroupEntry(secModel, securityName);

    if ( groupEntry == NULL )
    {
        DEBUG("GroupEntry was NULL\n");
        return 1;
    }

    //found the securityName in the group table - make sure the row is active
    if (groupEntry->status != SNMP_ROW_ACTIVE)
    {
       DEBUG ("group not active\n");
       return 1;
    }

    // 2. Use the GroupTable->vacmGroupName to find the corresponding row in the AccessTable
    accessEntry = vacm_getAccessEntry(groupEntry->groupName,
                                      "",//const char *contextPrefix,
                                      secModel,
                                      secLevel);
    if ( accessEntry == NULL )
    {
       DEBUG("accessEntry was NULL\n");
       return 1;
    }

    //found the access entry in the access table - make sure the row is active
    if (accessEntry->status != SNMP_ROW_ACTIVE)
    {
       DEBUG ("access not active\n");
       return 1;                               
    }
    
    // now that we have the correct access, get the views from it.
    // The caller might just want one or two views, so check for NULLs
    if (readView)
      *readView =   (char *) accessEntry->readView;
    if (writeView)
      *writeView =  (char *) accessEntry->writeView;
    if (notifyView)
      *notifyView = (char *) accessEntry->notifyView;
        
    //We have the correct views, 
    //    do we need to make sure the views are active, or does the vacm take care of this??
    
    return 0;
}

#ifdef SEG_FAULT_DUMP
static void sigsegv_handler (int n, siginfo_t *si, struct ucontext *uc)
{
  // struct sigcontext *sc = (struct sigcontext *)&uc->uc_mcontext;
  unsigned int pc;

  pc = uc->uc_mcontext.gpregs[CTX_RA];

  printf("The segmentation fault or bus error occurs in thread 0x%08x\n\n", (unsigned int)pthread_self());
  printf("Code dump:\n");
  printf("[%08x]\n", pc - 4); 
  // printf("[%0Ax]: %0Ax\n", pc - 4, *(unsigned int*)(pc-4)); 
  printf("[%08x]     <-----faulting instruction!!!\n", pc); 
  // printf("[%08x]: %08x     <-----faulting instruction!!!\n", pc, *(unsigned int*)(pc)); 
  // printf("[%0Ax]: %0Ax\n\n", pc + 4, *(unsigned int*)(pc+4));
  // DisplayThreadInfo();

  exit(1);
}
#endif

int GetSnmpV2cTrapType (unsigned char *data, int dataLen)
{
  // netsnmp_pdu pdu;
  // int retval = snmp_pdu_parse(&pdu,
  //                             (u_char *) data,
  //                             (size_t *) dataLen);
  // if (retval == SNMPERR_SUCCESS) {
    return(BCMV2_TRAP_TYPE_OCSTBPANICDUMP);
  // }

}



int
EcmGetObject(oid *ecmObj, int ecmObjLen, u_char *ecmObjType,
            void *val, size_t *valLen)
{
  printf("EcmGetObject() called. !!! NOT IMPLEMENTED !!!\n");
  return 1;
#ifdef NOTYET
  return (ecmSnmpGet(ecmObj, ecmObjLen, ecmObjType, val, valLen)); 
#endif
}
