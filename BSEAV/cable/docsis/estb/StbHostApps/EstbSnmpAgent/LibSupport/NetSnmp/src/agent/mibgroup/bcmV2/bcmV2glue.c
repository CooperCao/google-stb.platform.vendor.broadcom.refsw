/*
 * bcmV2glue.c
 */

#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <sys/types.h>
#if TIME_WITH_SYS_TIME
#  include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
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
#       include <stdlib.h>
#endif

/*
 * Stuff needed for getHwAddress(...) 
 */
#ifdef HAVE_SYS_IOCTL_H
#	include <sys/ioctl.h>
#endif
#ifdef HAVE_NET_IF_H
#	include <net/if.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>

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
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/snmp_vars.h>

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/default_store.h>
#include <net-snmp/library/mt_support.h>
#include <net-snmp/agent/snmp_agent.h>
#include <net-snmp/agent/agent_callbacks.h>
#include <net-snmp/agent/agent_trap.h>

#include "target/snmpTargetParamsEntry.h"
#include "target/snmpTargetAddrEntry.h"
#include "notification/snmpNotifyTable.h"
#include "notification/snmpNotifyFilterTable.h"
#include "notification/snmpNotifyFilterProfileTable.h"

// for vacm_in_view_callback
#include "../mibII/vacm_vars.h"

#include <net-snmp/bcmV2glue.h>

// V2 for ObjectId and Varbind types
#include "SnmpType.h"
#if HAVE_LOCALE_H
#include <locale.h>
#endif


int bcmV2_dontprintf( const char *format, ... )
{
  return 0;
}

// Packet process prototype.  Note that this must be changed from 'static' in
// snmp_api.c or else we will get linker errors.
int
_sess_process_packet(void *sessp, netsnmp_session * sp,
                     void *isp,
                     netsnmp_transport *transport,
                     void *opaque, int olength,
                     u_char * packetptr, int length);

// from snmp_api.c (bummer that this structure is not defined in a 
// header file so we could #include it instead of having to redefine it here):
struct session_list {
    struct session_list *next;
    netsnmp_session *session;
    netsnmp_transport *transport;
    struct snmp_internal_session *internal;
};
// Master list of sessions from snmp_api.c
extern struct session_list *Sessions;

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
void printTransport (netsnmp_transport *pTransport);

/* These are fixed values that we pass to V2 code */
static unsigned char pNullIpAddr[4] = {0x00,0x00,0x00,0x00};
static    int SnmpVersion = 0;


/* temporary holders for values we pass to V2 code */
#define COMMUNITY_MAX_V2_LEN 32
char pCommunity[COMMUNITY_MAX_V2_LEN];
unsigned char pSrcIpAddr[4];
unsigned char pDstIpAddr[4];

// Global pointers used in call to _sess_process_packet, which is called
// in bcmV2_process_snmp_packet
static struct session_list *gpSessionList;
static netsnmp_session *gpSession;
static void *gpInternalSession;
static netsnmp_transport *gpTransport;
extern int gMaxPacketSize; // From NetsnmpAgnt.c

/*
 * The list of active/open bcmV2 sessions.
 */
struct bcmV2_session_list {
    struct bcmV2_session_list *next;
    netsnmp_session *session;
};


struct bcmV2_session_list *BcmV2Sessions;

/* find the netsnmp_session struct with a given interface */
netsnmp_session *
bcmV2_get_session_by_interface(int interface) {
  netsnmp_session *sessPtr;
  struct bcmV2_session_list *sessListPtr = BcmV2Sessions;
  struct bcmV2_if_data *bcmV2IfData;

  DEBUGMSGTL(("snmp_get_session_by_interface", "find session for interface %d\n", interface));
  while (sessListPtr != NULL) {
    sessPtr = sessListPtr->session;
    if (sessPtr->myvoid != NULL) {
      bcmV2IfData = (struct bcmV2_if_data *) sessPtr->myvoid;
      DEBUGMSGTL(("snmp_get_session_by_interface",
                  "found session for interface %d\n", bcmV2IfData->ifIndex));
    if (bcmV2IfData->ifIndex == interface)
      return (sessPtr);
    } else {
      DEBUGMSGTL(("snmp_get_session_by_interface",
                  "found a non BRCM session \n"));
    }
    sessListPtr = sessListPtr->next;
  }
  DEBUGMSGTL(("snmp_get_session_by_interface", "could not find session for interface %d\n", interface));

  return NULL;
}

/* write the v1v2 view into the netsnmp_session struct that has this interface */
/* return the number of bytes written */
int
bcmV2_set_v1v2view(int interface, unsigned char *view, int viewLen) {
  netsnmp_session *sessPtr;
  struct bcmV2_if_data *bcmV2IfData;

  DEBUGMSGTL(("snmp_set_v1v2view", "setting view of length %d for interface %d.\n", viewLen, interface));
  sessPtr = bcmV2_get_session_by_interface( interface);
  if (sessPtr != NULL) {
    if (sessPtr->myvoid) {
      bcmV2IfData = (struct bcmV2_if_data *) sessPtr->myvoid;
      SNMP_FREE(bcmV2IfData->v1v2view);
      bcmV2IfData->v1v2view = (u_char *) malloc(viewLen+1);
      memset(bcmV2IfData->v1v2view, 0, viewLen+1);
      memcpy(bcmV2IfData->v1v2view, view, viewLen);
      bcmV2IfData->v1v2viewLen = viewLen;
      return viewLen;
    } else {
      DEBUGMSGTL(("snmp_set_v1v2view", "could not find a bcmV2 session .\n"));
      return 0;
    }
  }
  DEBUGMSGTL(("snmp_set_v1v2view", "could not find a session for this interface.\n"));
  return 0;
}


/* get the v1v2 view from the snmp_session struct that has this interface */
/* return a pointer to the view and the size */
int
bcmV2_get_v1v2view(int interface, unsigned char **view) {
  struct snmp_session *sessPtr;
  struct bcmV2_if_data *bcmV2IfData;

  sessPtr = bcmV2_get_session_by_interface( interface);
  if (sessPtr != NULL) {
    if (sessPtr->myvoid) {
      bcmV2IfData = (struct bcmV2_if_data *) sessPtr->myvoid;
      *view = bcmV2IfData->v1v2view;
      return bcmV2IfData->v1v2viewLen;
    }
  }
  view = NULL;
  return 0;
}

/* eventually we may need to create a new session here, but for STBs we still need to handle
 * all incoming traffic */
int
bcmV2_register_interface(int interface, unsigned long ipAddr, int udpPort) {
  netsnmp_session *sessPtr;
  struct bcmV2_if_data *bcmV2IfData;

  /* first, check to see if interface is already registered */
  sessPtr = bcmV2_get_session_by_interface(interface);
  if (sessPtr != NULL) {
    if (sessPtr->myvoid) {
      bcmV2IfData = (struct bcmV2_if_data *) sessPtr->myvoid;
      bcmV2IfData->ipAddr = ipAddr;
      return 1;
    }
  }
  /* if not, look for the first one that is not registered and use it */
  sessPtr = bcmV2_get_session_by_interface(0);
  if (sessPtr != NULL) {
    if (sessPtr->myvoid) {
      SNMP_FREE(sessPtr->myvoid);
    }
    bcmV2IfData = (struct bcmV2_if_data *) malloc(sizeof(struct bcmV2_if_data));
    bcmV2IfData->v1v2view = NULL;
    bcmV2IfData->v1v2viewLen = 0;
    bcmV2IfData->ifIndex = interface;
    bcmV2IfData->ipAddr = ipAddr;
    return 1;
  }
  return 0;
}

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
  if ( !buf || (bufLen > 1500) || (bufLen < 1)) {
    return 0;
  }

  newID = (u_char *) malloc(bufLen);
  if (!newID)
     return 0;
  SNMP_FREE(engineID);
  memcpy(newID, buf, bufLen);
  engineID = newID;
  engineIDLength = bufLen;
  set_enginetime(engineID, engineIDLength,
                 1,//snmpv3_local_snmpEngineBoots(),
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
 * bcmV2_prov_targetParamsTable
 * returns 1 for success, 0 for failure.
 *
 */

int
bcmV2_prov_targetParamsTable(int trapType, unsigned char *elementName,
                                     unsigned char *securityName, unsigned char *tlv38Base)
{
  struct targetParamTable_struct *entry;
  int mpModel;
  u_char debugBuf[256];

  // printf("vtaProvTargetParamsTable: trapType = %d, elementName = %s, securityName = %s\n", trapType, elementName, securityName);
  if ( (entry = snmpTargetParamTable_create()) == NULL )
    return 0;

  if (strlen(securityName) == 0) {
    if (snmpTargetParams_addSecName(entry, (char *) tlv38Base) == 0) {
      snmpTargetParamTable_dispose(entry);
      return 0;
    }
    entry->secLevel = SNMP_SEC_LEVEL_NOAUTH; //noAuthnoPriv
  } else {
    if ( snmpTargetParams_addSecName(entry, securityName) == 0 ) {
      snmpTargetParamTable_dispose(entry);
      return 0;
    }
    if (strcmp(securityName,"docsisManager") == 0) {
      entry->secLevel = SNMP_SEC_LEVEL_AUTHPRIV; //authPriv 3
    } else {
      entry->secLevel = SNMP_SEC_LEVEL_AUTHNOPRIV; //authNoPriv 2
    }
  }

  if ( strcmp(securityName, tlv38Base) == 0 )
        entry->secLevel = SNMP_SEC_LEVEL_NOAUTH; //noAuthnoPriv 1

  if ( strcmp(securityName,"testManager") == 0 )
        entry->secLevel = SNMP_SEC_LEVEL_NOAUTH; //noAuthnoPriv 1

  switch ( trapType ) {
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

  if ( snmpTargetParams_addParamName(entry, elementName)  == 0) {
    snmpTargetParamTable_dispose(entry);
    return 0;
  }
  entry->rowStatus = SNMP_ROW_ACTIVE;
  sprintf(debugBuf, "snmp_parse_config_targetParams, read: %s %d %d %s %d %d %d\n",
          entry->paramName, entry->mpModel,  entry->secModel,
          entry->secName,   entry->secLevel, entry->storageType,
          entry->rowStatus);
  DEBUGMSGTL(("snmpTargetParamsEntry", debugBuf));

  snmpTargetParamTable_add(entry);
  return 1;
}


/*
 *
 * bcmV2_prov_targetAddrTable
 * returns 1 for success, 0 for failure.
 *
 */

int
bcmV2_prov_targetAddrTable(unsigned char *targetIpAddr, int targetPort,
                                     int trapType, int timeOut, int numRetries,
                                     unsigned char *elementName)
{
  u_char    tagName[15];
  int       tagNameLen, i;

  oid   udpDomain[] = { 1, 3, 6, 1, 6, 1, 1};
  int udpDomainLen = sizeof(udpDomain) / sizeof(oid);
  u_char  addr1[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  struct targetAddrTable_struct *entry;
  unsigned char debugBuf[1024];

  if ((entry = snmpTargetAddrTable_create()) == NULL)
    return SNMP_ERR_GENERR;

  for (i = udpDomainLen - 1; i >= 0; i--)
    entry->tDomain[i] = udpDomain[i];
  entry->tDomainLen = udpDomainLen;

  /* we malloc 7 bytes just in case someone thinks this is a string */
  if ((entry->tAddress = (char *)malloc(7)) == NULL) {
    snmpTargetAddrTable_dispose(entry);
    return SNMP_ERR_GENERR;
  }
  memset(entry->tAddress, 0, 7);
  memcpy(entry->tAddress,targetIpAddr,4);     // address should be 4 bytes of IP addr and
  entry->tAddress[5] = (unsigned char)(targetPort); // 2 bytes of port
  entry->tAddress[4] = (unsigned char)(targetPort>>8);

  entry->timeout = timeOut;
  entry->retryCount = numRetries;

  memset(tagName, 0, sizeof(tagName));
  switch ( trapType ) {
    case 1:
    case 2:
    case 4:
      tagNameLen = 12;
      memcpy(tagName,"@config_trap",tagNameLen);
      break;
    case 3:
    case 5:
      tagNameLen = 14;
      memcpy(tagName,"@config_inform",tagNameLen);
      break;
    default:
      snmpTargetAddrTable_dispose(entry);
      return SNMP_ERR_GENERR;
      break;
  }
  if ( snmpTargetAddr_addTagList(entry, tagName) == 0) {
    snmpTargetAddrTable_dispose(entry);
    return SNMP_ERR_GENERR;
  }

  if ((snmpTargetAddr_addName(entry, elementName)) == 0) {
    snmpTargetAddrTable_dispose(entry);
    return SNMP_ERR_GENERR;
  }

  if ((snmpTargetAddr_addParams(entry, elementName)) == 0) {
    snmpTargetAddrTable_dispose(entry);
    printf("vtaProvTargetAddrTable : did not add Params");
    return SNMP_ERR_GENERR;
  }

  entry->storageType = SNMP_STORAGE_VOLATILE;

  entry->rowStatus = SNMP_ROW_ACTIVE;

  sprintf(debugBuf, "snmp_parse_config_targetAddr, read: %s\n",
                   entry->name);
  for(i=0;i<entry->tDomainLen;i++) {
    sprintf(&debugBuf[strlen(debugBuf)], ".%d", (int)entry->tDomain[i]);
  }
  sprintf(&debugBuf[strlen(debugBuf)], " %s %d %d %s %s %d %d\n",
          entry->tAddress, entry->timeout, entry->retryCount,
          entry->tagList,  entry->params,  entry->storageType,
          entry->rowStatus);
  DEBUGMSGTL(("vtaProvTargetAddrTable", debugBuf));

  snmpTargetAddrTable_add(entry);

  return SNMP_ERR_NOERROR;
}


/*
 *
 * bcmV2_get_targetAddrEntry
 * returns 1 for success, 0 for failure.
 * elementName is the entry we are looking for
 * the other parameters are pointers that are set if an entry is found
 *
 */

int
bcmV2_get_targetAddrEntry(unsigned char *elementName,
                          unsigned long **ppDomain,
                          int *pDomainLen,
                          unsigned char **ppAddr, 
                          int *pAddrLen,
                          char **ppTagList,
                          int *pTagListLen,
                          int *pStatus)
{
  struct targetAddrTable_struct *entry;

  entry = get_addrForName(elementName);

  /* if the table is empty, return 0 */
  if (entry == NULL) {
    DEBUGMSGTL(("bcmV2_get_targetAddrEntry", "Entry not found!\n"));
    return (0);
  }


  /* we found a match */

  /* get domain */
  *ppDomain = entry->tDomain;
  *pDomainLen = entry->tDomainLen;

  /* get address */
  *ppAddr = entry->tAddress;
  *pAddrLen = entry->tAddressLen;

  /* get tag list */
  *pTagListLen = strlen(entry->tagList);
  *ppTagList = entry->tagList;

  /* get row status  */
  *pStatus = entry->rowStatus;

  DEBUGMSGTL(("bcmV2_get_targetAddrEntry", "Entry found for %s\n", elementName));
  return(1);

}

/*
 * bcmV2_prov_notifyFilterProfileTable(): 
 * adds an entry to the Notify Filter Profile Table
 */
int
bcmV2_prov_notifyFilterProfileTable(unsigned char *elementName)
{
  struct snmpNotifyFilterProfileTable_data *nfpPtr;

  nfpPtr = SNMP_MALLOC_STRUCT(snmpNotifyFilterProfileTable_data);
  nfpPtr->snmpNotifyFilterProfileName = (char *) strdup(elementName);
  nfpPtr->snmpNotifyFilterProfileNameLen = strlen(elementName);

  nfpPtr->snmpTargetParamsName = (char *) strdup(elementName);
  nfpPtr->snmpTargetParamsNameLen = strlen(elementName);

  nfpPtr->snmpNotifyFilterProfileStorType = ST_VOLATILE;
  nfpPtr->snmpNotifyFilterProfileRowStatus = RS_ACTIVE;

  return (snmpNotifyFilterProfileTable_add(nfpPtr));
}

/*
 * bcmV2_prov_notifyFilterTable(): adds an entry to the Notify Filter Table
 *   based on vta transaction from the brcm V2 code.
 */
int
bcmV2_prov_notifyFilterTable(unsigned char *elementName, int filterLen, oid *filter)
{
  struct snmpNotifyFilterTable_data *nfptr;

  nfptr = SNMP_MALLOC_STRUCT(snmpNotifyFilterTable_data);
  nfptr->snmpNotifyFilterProfileName = strdup(elementName);
  nfptr->snmpNotifyFilterProfileNameLen = strlen(elementName);

  memdup((u_char **) & (nfptr->snmpNotifyFilterSubtree),
               (u_char *) filter, (filterLen * sizeof(oid)));
  nfptr->snmpNotifyFilterSubtreeLen = filterLen;

  nfptr->snmpNotifyFilterMask = (char *) calloc(1, 1);
  nfptr->snmpNotifyFilterMaskLen = 0;

  nfptr->snmpNotifyFilterType = SNMPNOTIFYFILTERTYPE_INCLUDED;
  nfptr->snmpNotifyFilterStorageType = ST_VOLATILE;
  nfptr->snmpNotifyFilterRowStatus = RS_ACTIVE;

  return (snmpNotifyFilterTable_add(nfptr));
}




 /*
  * bcmV2_prov_notifyTable(): adds two entries to the Notify Table
  *   based on vta transaction from the brcm V2 code.
  *   1. an inform
  *   2. a trap
  */

int
bcmV2_prov_notifyTable(unsigned char *informTag, unsigned char *trapTag)
{
  struct snmpNotifyTable_data *nptr;

  // printf("bcmv2_provNotifyTable: informTag %s, trapTag %s\n", informTag, trapTag);
  // add the inform tag, but only if the string is non-null
  if (informTag != NULL) {
    if (strlen(informTag) > 0)  {
      nptr = (struct snmpNotifyTable_data *) SNMP_MALLOC_STRUCT(snmpNotifyTable_data);
      nptr->snmpNotifyName = (unsigned char *) strdup(informTag);
      nptr->snmpNotifyNameLen = strlen(informTag);
      nptr->snmpNotifyTag = (unsigned char *) strdup(informTag);
      nptr->snmpNotifyTagLen = strlen(informTag);
      nptr->snmpNotifyType = SNMPNOTIFYTYPE_INFORM;
      nptr->snmpNotifyStorageType = ST_VOLATILE;
      nptr->snmpNotifyRowStatus = RS_ACTIVE;

      snmpNotifyTable_add(nptr);
    }
  }
  // add the trap tag, but only if the string is non-null
  if (trapTag != NULL) {
    if (strlen(trapTag) > 0)  {
      nptr = (struct snmpNotifyTable_data *) SNMP_MALLOC_STRUCT(snmpNotifyTable_data);
      nptr->snmpNotifyName = (unsigned char *) strdup(trapTag);
      nptr->snmpNotifyNameLen = strlen(trapTag);
      nptr->snmpNotifyTag = (unsigned char *) strdup(trapTag);
      nptr->snmpNotifyTagLen = strlen(trapTag);
      nptr->snmpNotifyType = SNMPNOTIFYTYPE_TRAP;
      nptr->snmpNotifyStorageType = ST_VOLATILE;
      nptr->snmpNotifyRowStatus = RS_ACTIVE;

      snmpNotifyTable_add(nptr);
    }
  }
return 1;
}

/*
 *
 * bcmV2_cleanUpNotifies
 * returns 1 on success, 0 on failure.
 *
 */

int
bcmV2_cleanUpNotifies(void)
{
#ifdef NOTYET
do something!
#endif
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
  return snmp_enableauthentraps;
}

/*
 *
 * bcmV2_get_objects
 * returns SNMP error code.
 *
 */

int
bcmV2_set_objects(int objIdLen, unsigned long *objId, int type, int valueLen, void *value, int mode)
{
#ifdef NOTDEF
  u_char  statType;
  u_char *statP;
  size_t  statLen;
  u_short acl;
  WriteMethod *write_method;
  int       noSuchObject = TRUE;

  statP = getStatPtr(  objId, &objIdLen,
                           &statType, &statLen, &acl,
                           1, &write_method, NULL, &noSuchObject);

  if (statP == NULL)
    return SNMP_ERR_NOSUCHNAME;

  if ((acl & RWRITE) == 0)
    return SNMP_ERR_NOSUCHNAME;

  /* dump verbose info */
  if (ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_VERBOSE))
    dump_var(objId, objIdLen, type, value, valueLen);

  /*  FINALLY we can act on SET requests ....*/
  /*  If there is a write method, use it */
  if ( write_method != NULL ) {
    if (mode == COMMIT)  {
      /* first, do a RESERVE1, RESERVE2, ACTION, then COMMIT */
      statType = (*write_method)(RESERVE1,
                               (u_char *) value,
                               type,
                               valueLen, statP,
                               objId,
                               objIdLen);
      if (statType != SNMP_ERR_NOERROR)
        return (statType);
      statType = (*write_method)(RESERVE2,
                               (u_char *) value,
                               type,
                               valueLen, statP,
                               objId,
                               objIdLen);
      if (statType != SNMP_ERR_NOERROR)
        return (statType);
      statType = (*write_method)(ACTION,
                               (u_char *) value,
                               type,
                               valueLen, statP,
                               objId,
                               objIdLen);
      if (statType != SNMP_ERR_NOERROR)
        return (statType);
      statType = (*write_method)(COMMIT,
                               (u_char *) value,
                               type,
                               valueLen, statP,
                               objId,
                               objIdLen);
      if (statType != SNMP_ERR_NOERROR)
        return (statType);
    } else {
      statType = (*write_method)(mode,
                               (u_char *) value,
                               type,
                               valueLen, statP,
                               objId,
                               objIdLen);
      if (statType != SNMP_ERR_NOERROR)
        return (statType);
    }
  } else {
    /* no write method, just jam the value into the location
     * we got from getStatPtr
     */
    if(mode == COMMIT)
      setVariable(value, type, valueLen, statP, statLen);
  }
  return SNMP_ERR_NOERROR;
#endif
}


#ifdef NOTNOW
/* Print out all users - FOR DEBUG ONLY!!! */
void
dump_users(void)
{
  struct usmUser *newUser;

  printf("\n\nDumping userList contents\n");
  fflush(stdout);
  for (newUser = userList; newUser != NULL; newUser = newUser->next)
    if (newUser->next == NULL) {
      printf("\tuser %s is the last one\n\n", newUser->name);
      fflush(stdout);
    } else {
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
  if (newUser == NULL)
    return 0;

  if ((newUser->name = strdup(userName)) == NULL) {
    usm_free_user(newUser);
    return 0;
  }

  if ((newUser->secName = strdup(userName)) == NULL) {
    usm_free_user(newUser);
    return 0;
  }

  newUser->engineID = (u_char *) malloc(myIdLen);
  memcpy(newUser->engineID, myId, myIdLen);
  newUser->engineIDLen = myIdLen;

  if (secLevel == SNMP_SEC_LEVEL_AUTHPRIV) {
    SNMP_FREE(newUser->privProtocol);
    newUser->privProtocol = snmp_duplicate_objid(usmDESPrivProtocol, USM_LENGTH_OID_TRANSFORM);
    newUser->privProtocolLen = USM_LENGTH_OID_TRANSFORM;
    
    SNMP_FREE(newUser->authProtocol);
    newUser->authProtocol = snmp_duplicate_objid(usmHMACMD5AuthProtocol, USM_LENGTH_OID_TRANSFORM);
    newUser->authProtocolLen = USM_LENGTH_OID_TRANSFORM;

  } else if (secLevel == SNMP_SEC_LEVEL_AUTHNOPRIV) {
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
  newUser = usm_get_user(engineId, engineIdLen, userName);
  if (newUser == NULL)
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
  newUser = usm_get_user(engineId, engineIdLen, userName);
  if (newUser == NULL)
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
  newUser = usm_get_user(engineId, engineIdLen, userName);
  if (newUser == NULL)
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

  for (ptr = puserList; ptr != NULL; ptr = ptr->next) {
    if (ptr->engineIDLen == engineIDLen &&
         (ptr->engineID != NULL && engineID != NULL &&
          memcmp(ptr->engineID, engineID, engineIDLen) == 0))
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
  while (newUser != NULL) {
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

  for (newUser = userList; newUser != NULL; newUser = newUser->next) {
    if (newUser->engineIDLen == engineIdLen &&
        (newUser->engineID != NULL && engineId != NULL &&
         memcmp(newUser->engineID, engineId, engineIdLen) == 0) &&
        memcmp(newUser->name, "docsis", 6) == 0) {

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

  newUser = usm_get_user(engineId, engineIdLen, userName);
  if (newUser == NULL){
    DEBUGMSGTL(("set_bcmV2_user_key", "couldn't find user %s\n", userName));
    return 0;
  }

  if (keyId == 1) {
    if ((newUser->authKey = (u_char *) malloc(keyLen)) != NULL) {
        newUser->authKeyLen = keyLen;
        memcpy(newUser->authKey, key, keyLen);
        DEBUGMSGTL(("set_bcmV2_user_key", "Setting MD5 auth key for user %s.\n", userName));
    } else {
      DEBUGMSGTL(("set_bcmV2_user_key", "Couldn't malloc for auth key.\n"));
      return 0;
    }
  } else if (keyId == 2) {
      if ((newUser->privKey = (u_char *) malloc(keyLen)) != NULL) {
        newUser->privKeyLen = keyLen;
        memcpy(newUser->privKey, key, keyLen);
        DEBUGMSGTL(("set_bcmV2_user_key", "Setting DES priv key for user %s.\n", userName));
    } else {
      DEBUGMSGTL(("set_bcmV2_user_key", "Couldn't malloc for priv key.\n"));
      return 0;
    }
  } else {
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
    while (vp) {
      if (!memcmp(vp->viewName+1, (char *) namePrefix, strlen(namePrefix))){
        lastvp = vp;
        vp = vp->next;
      } else {
        if (vp == viewList) {    /* remove head of list */
          viewList = lastvp = vp->next;
          if (vp->reserved)
            free(vp->reserved);
          free(vp);
          vp = viewList;
        } else {
          lastvp->next = vp->next;
          if (vp->reserved)
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
    while (gp) {
      if (!memcmp(gp->groupName, (char *) namePrefix, strlen(namePrefix))){
        lastgp = gp;
        gp = gp->next;
      } else {
        if (gp == groupList) {    /* remove head of list */
          groupList = lastgp = gp->next;
          if (gp->reserved)
            free(gp->reserved);
          free(gp);
          gp = groupList;
        } else {
          lastgp->next = gp->next;
          if (gp->reserved)
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
    while (ap) {
      if (!memcmp(ap->groupName+1, (char *) namePrefix, strlen(namePrefix))){
        lastap = ap;
        ap = ap->next;
      } else {
        if (ap == accessList) {    /* remove head of list */
          accessList = lastap = ap->next;
          if (ap->reserved)
            free(ap->reserved);
          free(ap);
          ap = accessList;
        } else {
          lastap->next = ap->next;
          if (ap->reserved)
            free(ap->reserved);
          free(ap);
          ap = lastap->next;
        }
      }
    }
}

#ifdef NOTYET
int
send_bcmV2_notifications(struct snmp_pdu *template_pdu)
{
    struct header_complex_index *hptr;
    struct snmpNotifyTable_data *nptr;
    struct snmp_session *sess, *sptr;
    struct snmp_pdu *tmpPdu;

    // printf("send_bcmV2_notifications: starting\n");
    DEBUGMSGTL(("send_notifications","starting: pdu=%x, vars=%x\n",
                template_pdu, template_pdu->variables));

    for(hptr = snmpNotifyTableStorage; hptr; hptr = hptr->next) {
        //printf("send_bcmV2_notifications: processing Notify Table storage\n");
        nptr = (struct snmpNotifyTable_data *) hptr->data;
        if (nptr->snmpNotifyRowStatus != RS_ACTIVE)
            continue;
        if (!nptr->snmpNotifyTag)
            continue;

        // printf("send_bcmV2_notifications: getting target sessions\n");
        sess = get_target_sessions(nptr->snmpNotifyTag, NULL, NULL);

#ifdef NOTDEF
        if(sess == NULL)
          printf("send_bcmV2_notifications: no target sessions found\n");
        else
          printf("send_bcmV2_notifications: got target sessions\n");
#endif


        /* XXX: filter appropriately */

        for(sptr=sess; sptr; sptr = sptr->next) {
          //printf("send_bcmV2_notifications: target sessions %p\n", sptr);
          //fflush(stdout);
            if (sptr->version == SNMP_VERSION_1) {
              //printf("send_bcmV2_notifications: sending a V1 trap on sess %p\n", sptr);
              template_pdu->command = SNMP_MSG_TRAP;
              send_trap_to_sess(sptr, template_pdu);
            } else {
                template_pdu->reqid = snmp_get_next_reqid();
                template_pdu->msgid = snmp_get_next_msgid();
                if (nptr->snmpNotifyType == SNMPNOTIFYTYPE_INFORM) {
                  //printf("send_bcmV2_notifications: sending an inform on sess %p\n", sptr);
                  template_pdu->command = SNMP_MSG_INFORM;
                } else {
                  //printf("send_bcmV2_notifications: sending a V2 trap on sess %p\n", sptr);
                  template_pdu->command = SNMP_MSG_TRAP2;
                }
                send_trap_to_sess(sptr, template_pdu);
            }
        }
    }
    //printf("send_bcmV2_notifications: Done processing Notify Table storage\n");
    return 1;
 }

#endif	/* NOTYET */


int extractV2VarBind(VarBind *v2VarBindPtr, void *value) {
    unsigned long int *longPtr;
    int length;

    switch (v2VarBindPtr->Type) {
    case VBT_COUNTER:
    case VBT_GAUGE:
    case VBT_TIMETICKS:
    case VBT_OPAQUE:
    case VBT_UINTEGER32:
    case VBT_NUMBER:
        longPtr = (unsigned long int *) value;
        *longPtr = VARBIND_GET_INT32(v2VarBindPtr);
        //printf("extractV2VarBind got value = %ld\n", *longPtr);
        return(sizeof(unsigned long int));
        break;
    //case VBT_BITS:
    case VBT_STRING:
        length = (VARBIND_GET_STRING(v2VarBindPtr))->Length;
        memcpy(value, (VARBIND_GET_STRING(v2VarBindPtr))->pOctets, length);
        return(length);
        break;
    case VBT_OBJECT:
        length = (VARBIND_GET_OID(v2VarBindPtr))->Length * sizeof(unsigned long);
        memcpy(value, (VARBIND_GET_OID(v2VarBindPtr))->pObjects, length);
        return(length);
        break;
    case VBT_IPADDRESS:
        VARBIND_GET_IPADDR(v2VarBindPtr, value);
        return(sizeof(unsigned long int));
        break;
    case VBT_COUNTER64:
        length = 8;
        memcpy (value, &v2VarBindPtr->Value.C64.Hi, 4);
        memcpy (value + 4, &v2VarBindPtr->Value.C64.Lo, 4);
        return(length);
        break;
        
    default:
        printf("extractV2VarBind: Type UNKNOWN %x\n", VARBIND_GET_TYPE(v2VarBindPtr));
        return -1;
    }
}

void dumpV2String(OctetString *v2StringPtr) {
    int i;
    unsigned char *octetPtr;

/*  PR 5404: no max string size anymore.  Really I guess I should define
    this as 65535, but I am being lazy and assuming nobody's going to be
    hitting us with a string that big...
    if((v2StringPtr->Length > MAX_STRING_SIZE) && (v2StringPtr->Length < 0)) {
        printf("v2 String: Bad Length %d, ", v2StringPtr->Length);
        return;
    }
*/
    octetPtr = v2StringPtr->pOctets;
    printf("v2 String Length = %d, ", v2StringPtr->Length);
    for (i = v2StringPtr->Length; i > 0; i--, octetPtr++) {
        printf("%x ", *octetPtr);
    }
    printf("= ");
    for (i = v2StringPtr->Length; i > 0; i--, octetPtr++) {
        printf("%c", *((char *) octetPtr));
    }
    printf("\n");
}

void dumpV2Oid(ObjectId *v2OidPtr) {
    int i;
    unsigned long *subIdPtr;

    if((v2OidPtr->Length > MAX_OID_SIZE) && (v2OidPtr->Length < 0)) {
        printf("v2 OID: Bad Length %d, ", v2OidPtr->Length);
        return;
    }
    printf("v2 OID Length = %d, ", v2OidPtr->Length);
    subIdPtr = v2OidPtr->pObjects;
    for (i = v2OidPtr->Length; i > 0; i--, subIdPtr++) {
        printf(".%ld", *subIdPtr);
    }
    printf("\n");
}

void dumpV2VarBind(VarBind *v2VarBindPtr) {

    unsigned long int ipAddr;

    switch (v2VarBindPtr->Type) {
    case VBT_NUMBER:
        printf("v2 BarBind: Type INTEGER, Value %d\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
    //case VBT_BITS:
    case VBT_STRING:
        printf("v2 BarBind: Type STRING/BITS, ");
        dumpV2String(VARBIND_GET_STRING(v2VarBindPtr));
        break;
    case VBT_OBJECT:
        printf("v2 BarBind: Type OBJECT ID, ");
        dumpV2Oid(VARBIND_GET_OID(v2VarBindPtr));
        break;
    case VBT_EMPTY:
        printf("v2 BarBind: Type EMPTY\n");
        break;
    case VBT_IPADDRESS:
        VARBIND_GET_IPADDR(v2VarBindPtr, &ipAddr);
        printf("v2 BarBind: Type IPADDRESS, Value %hl\n", ipAddr);
        break;
    case VBT_COUNTER:
        printf("v2 BarBind: Type COUNTER, Value %d\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
    //case VBT_UNSIGNED32:
    case VBT_GAUGE:
        printf("v2 BarBind: Type GAUGE/UNSIGNED32, Value %d\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
    case VBT_TIMETICKS:
        printf("v2 BarBind: Type TIMETICKS, Value %d\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
    case VBT_OPAQUE:
        printf("v2 BarBind: Type OPAQUE, Value %d\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
    //case VBT_COUNTER64:
        //printf("v2 BarBind: Type COUNTER64, Value %d\n", VARBIND_GET_INT32(v2VarBindPtr));
        //break;
    case VBT_UINTEGER32:
        printf("v2 BarBind: Type UINTEGER32, Value %d\n", VARBIND_GET_INT32(v2VarBindPtr));
        break;
    default:
        printf("v2 BarBind: Type UNKNOWN %x", VARBIND_GET_TYPE(v2VarBindPtr));
    }
}

void buildV2VarBind(VarBind *v2VarBindPtr, u_char *value, size_t valueLen, u_char type) {

    switch (type) {
    case ASN_INTEGER:
        VARBIND_SET_INT32(v2VarBindPtr, *((long *) value));
        break;
      case ASN_OCTET_STR:
        // PR 5404: OctetString.pOctets is a pointer not a fixed length field
        // now, so we can't memcpy into it, which is what VARBIND_SET_STRING
        // does.  Use the value pointer directly instead.
        // VARBIND_SET_STRING(v2VarBindPtr, value, valueLen, VBT_STRING);
        v2VarBindPtr->Type = VBT_STRING;
        v2VarBindPtr->Value.String.Length = valueLen;
        v2VarBindPtr->Value.String.pOctets = value;
        break;
    case ASN_OBJECT_ID:
        memcpy(v2VarBindPtr->Value.OID.pObjects, value,
                                      valueLen * sizeof(unsigned long));
        v2VarBindPtr->Value.OID.Length = valueLen;
        v2VarBindPtr->Type = VBT_OBJECT;
        break;
    case ASN_NULL:
        VARBIND_SET_EMPTY(v2VarBindPtr);
        break;
    case ASN_IPADDRESS:
        // PR 5404: OctetString.pOctets is a pointer not a fixed length field
        // now, so we can't memcpy into it, which is what VARBIND_SET_IPADDR
        // does.  Use the value pointer directly instead.
        // VARBIND_SET_IPADDR(v2VarBindPtr, value);
        v2VarBindPtr->Type = VBT_IPADDRESS;
        v2VarBindPtr->Value.String.Length = 4;
        v2VarBindPtr->Value.String.pOctets = value;
        break;
    case ASN_COUNTER:
    case ASN_GAUGE:               // same as ASN_UNSIGNED:
    case ASN_TIMETICKS:
        VARBIND_SET_UINT32(v2VarBindPtr, *((unsigned long *) value), type);
        break;
    default:
        printf("buildV2BarBind: Type %x is not supported", type);
    }
}

void dumpUCDOid(oid *oidPtr, int oidLen) {
    int i;

    if((oidLen > MAX_OID_SIZE) && (oidLen < 0)) {
        printf("UCD OID: Bad Length %d, ", oidLen);
        return;
    }
    printf("UCD OID Length = %d, ", oidLen);
    for (i = oidLen; i > 0; i--, oidPtr++) {
        printf(".%ld", *oidPtr);
    }
    printf("\n");
}


int v2Get(oid *oidPtr, int oidLen, char *value, size_t *valueLen, netsnmp_agent_request_info *reqInfo)
{
    ObjectId v2Oid, *v2OidPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    struct snmp_session *sessPtr;
    struct sockaddr_in *saPtr;
    char pCommunity[COMMUNITY_MAX_V2_LEN];
    SNMP_STATUS retVal = SNMP_NO_ERROR;

    DEBUG("v2Get: starting\n");
    //fflush(stdout);
    v2OidPtr = &v2Oid;
    v2VarBindPtr = &v2VarBind;

    memset (v2VarBindPtr, 0, sizeof (VarBind));
    v2OidPtr->Length = oidLen;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) oidPtr, oidLen * sizeof(oid));

    pduPtr = reqInfo->asp->pdu;
    if(pduPtr == NULL) {
      printf("v2Get: reqInfo->pdu is NULL! Should never happen! \n");
      fflush(stdout);
      return SNMP_ERR_GENERR;
    }
    if(pduPtr->community_len >= COMMUNITY_MAX_V2_LEN) {
      return SNMP_ERR_GENERR;
    }
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);
    pCommunity[pduPtr->community_len] = '\0';

    if ((pduPtr->transport_data_length != sizeof(struct sockaddr_in)) ||	/* we don't know what this is */
        (pduPtr->transport_data == NULL)) {				/* no transport data */
      printf("v2Get: no source address in pdu struct, using 0.0.0.0\n");
      fflush(stdout);
      memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
    } else {
      saPtr = (struct sockaddr_in *) pduPtr->transport_data; 
      if (saPtr->sin_family != AF_INET) {
        memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
      } else {
        memcpy( (char *) pSrcIpAddr,(char *) &(saPtr->sin_addr.s_addr), (int) 4);
      }
    }

    // printf("Warning - calling AppGetObject with a zero ip address\n");
    //AppGetObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, 
    //              const char *pCommunity, int SnmpVersion)
    retVal = AppGetObject( v2OidPtr, v2VarBindPtr, pNullIpAddr,
                        pSrcIpAddr, pCommunity, SnmpVersion);
    //retVal = AppGetObject( v2OidPtr, v2VarBindPtr, pDstIpAddr,pSrcIpAddr, pCommunity, SnmpVersion);

    if (retVal == SNMP_NO_ERROR) {

      /* get returned value */
      *valueLen = extractV2VarBind(v2VarBindPtr, value);
      if (*valueLen == -1)
        retVal = SNMP_ERR_GENERR;

    }
    
    // If the app has allocated string storage for us, free it now.
    if ((v2VarBindPtr->Type == VBT_STRING)
    ||  (v2VarBindPtr->Type == VBT_IPADDRESS))
    {
      if (v2VarBindPtr->Value.String.pOctets)
        free (v2VarBindPtr->Value.String.pOctets);
    }
    
    return retVal;
}

int v2GetObject (netsnmp_variable_list *requestvb, char *value, size_t *valueLen, netsnmp_agent_request_info *reqInfo)
{
    ObjectId v2Oid, *v2OidPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    struct snmp_session *sessPtr;
    struct sockaddr_in *saPtr;
    char pCommunity[COMMUNITY_MAX_V2_LEN];
    SNMP_STATUS retVal = SNMP_NO_ERROR;

    DEBUG("v2GetObject: starting\n");
    //fflush(stdout);
    v2OidPtr = &v2Oid;
    v2VarBindPtr = &v2VarBind;
    
    memset (v2VarBindPtr, 0, sizeof (VarBind));

    v2OidPtr->Length = requestvb->name_length;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) requestvb->name, requestvb->name_length * sizeof(oid));

    pduPtr = reqInfo->asp->pdu;
    if(pduPtr == NULL) {
      printf("appGet: reqInfo->pdu is NULL! Should never happen! \n");
      fflush(stdout);
      return SNMP_ERR_GENERR;
    }
    if(pduPtr->community_len >= COMMUNITY_MAX_V2_LEN) {
      return SNMP_ERR_GENERR;
    }
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);
    pCommunity[pduPtr->community_len] = '\0';

    if ((pduPtr->transport_data_length != sizeof(struct sockaddr_in)) ||	/* we don't know what this is */
        (pduPtr->transport_data == NULL)) {				/* no transport data */
      printf("appGet: no source address in pdu struct, using 0.0.0.0\n");
      fflush(stdout);
      memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
    } else {
      saPtr = (struct sockaddr_in *) pduPtr->transport_data; 
      if (saPtr->sin_family != AF_INET) {
        memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
      } else {
        memcpy( (char *) pSrcIpAddr,(char *) &(saPtr->sin_addr.s_addr), (int) 4);
      }
    }

    // printf("Warning - calling AppGetObject with a zero ip address\n");
    //AppGetObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, 
    //              const char *pCommunity, int SnmpVersion)
    retVal = AppGetObject( v2OidPtr, v2VarBindPtr, pNullIpAddr,
                        pSrcIpAddr, pCommunity, SnmpVersion);
    //retVal = AppGetObject( v2OidPtr, v2VarBindPtr, pDstIpAddr,pSrcIpAddr, pCommunity, SnmpVersion);

    if (retVal == SNMP_NO_ERROR) {

      /* get returned value */
      *valueLen = extractV2VarBind(v2VarBindPtr, value);
      if (*valueLen == -1)
        retVal = SNMP_ERR_GENERR;
      else
        // Load type from V2 varbind
        requestvb->type = v2VarBindPtr->Type;
    }
    
    // If the app has allocated string storage for us, free it now.
    if ((v2VarBindPtr->Type == VBT_STRING)
    ||  (v2VarBindPtr->Type == VBT_IPADDRESS))
    {
      if (v2VarBindPtr->Value.String.pOctets)
        free (v2VarBindPtr->Value.String.pOctets);
    }
    
    return retVal;
}

int v2GetNext(oid *oidPtr, int *oidLen, int indexOffset, char *value, size_t *valueLen, netsnmp_agent_request_info *reqInfo)
{
    ObjectId v2Oid, *v2OidPtr, v2Index, *v2IndexPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    struct snmp_session *sessPtr;
    struct sockaddr_in *saPtr;
    char pCommunity[COMMUNITY_MAX_V2_LEN];
    SNMP_STATUS retVal = SNMP_NO_ERROR;


    DEBUG("v2GetNext: Starting\n");

    v2OidPtr = &v2Oid;
    v2IndexPtr = &v2Index;
    v2VarBindPtr = &v2VarBind;

    memset (v2VarBindPtr, 0, sizeof (VarBind));

    v2OidPtr->Length = indexOffset;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) oidPtr, indexOffset * sizeof(oid));

    v2IndexPtr->Length = *oidLen - indexOffset;
    memcpy( (char *) (v2IndexPtr->pObjects), (char *) (oidPtr + indexOffset),
        ((int) *oidLen - indexOffset) * sizeof(oid));

    pduPtr = reqInfo->asp->pdu;
    if(pduPtr == NULL) {
      return SNMP_ERR_GENERR;
    }
    if(pduPtr->community_len >= COMMUNITY_MAX_V2_LEN) {
      return SNMP_ERR_GENERR;
    }
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);
    pCommunity[pduPtr->community_len] = '\0';

    if ((pduPtr->transport_data_length != sizeof(struct sockaddr_in)) ||	/* we don't know what this is */
        (pduPtr->transport_data == NULL)) {				/* no transport data */
      memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
    } else {
      saPtr = (struct sockaddr_in *) pduPtr->transport_data; 
      if (saPtr->sin_family != AF_INET) {
        memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
      } else {
        memcpy( (char *) pSrcIpAddr,(char *) &(saPtr->sin_addr.s_addr), (int) 4);
      }
    }

  // SNMP_STATUS AppNextObject (ObjectId *pOID, ObjectId *pIndex, VarBind *pVB,
  //  const unsigned char *DA, connst unsigned char *SA, const char *pCommunity, int SnmpVersion);
    retVal = AppNextObject ( v2OidPtr, v2IndexPtr, v2VarBindPtr,
                        pNullIpAddr, pSrcIpAddr, pCommunity,
                        SnmpVersion);

    if (retVal == SNMP_NO_ERROR) 
    {
      DEBUG ("  NOERROR\n");
      memcpy( (char *) (oidPtr + indexOffset), (char *) (v2IndexPtr->pObjects),
        ((int) v2IndexPtr->Length) * sizeof(oid));
      *oidLen = indexOffset + v2IndexPtr->Length;

      /* get returned value */
      *valueLen = extractV2VarBind(v2VarBindPtr, value);
    }
    
    // If the app has allocated string storage for us, free it now.
    if ((v2VarBindPtr->Type == VBT_STRING)
    ||  (v2VarBindPtr->Type == VBT_IPADDRESS))
    {
      if (v2VarBindPtr->Value.String.pOctets)
        free (v2VarBindPtr->Value.String.pOctets);
    }
    
    return retVal;
}

/*******************/
//int v2GetNext(oid *oidPtr, int *oidLen, int indexOffset, char *value, size_t *valueLen, netsnmp_agent_request_info *reqInfo)
int v2GetNextObject (netsnmp_variable_list *requestvb, int indexOffset, char *value, size_t *valueLen, netsnmp_agent_request_info *reqInfo)
{
    ObjectId v2Oid, *v2OidPtr, v2Index, *v2IndexPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    struct snmp_session *sessPtr;
    struct sockaddr_in *saPtr;
    char pCommunity[COMMUNITY_MAX_V2_LEN];
    SNMP_STATUS retVal = SNMP_NO_ERROR;


    DEBUG("v2GetNextObject: Starting\n");

    v2OidPtr = &v2Oid;
    v2IndexPtr = &v2Index;
    v2VarBindPtr = &v2VarBind;

    memset (v2VarBindPtr, 0, sizeof (VarBind));

    v2OidPtr->Length = indexOffset;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) requestvb->name, indexOffset * sizeof(oid));

    v2IndexPtr->Length = requestvb->name_length - indexOffset;
    memcpy( (char *) (v2IndexPtr->pObjects), (char *) (requestvb->name + indexOffset),
        ((int) requestvb->name_length - indexOffset) * sizeof(oid));

    pduPtr = reqInfo->asp->pdu;
    if(pduPtr == NULL) {
      return SNMP_ERR_GENERR;
    }
    if(pduPtr->community_len >= COMMUNITY_MAX_V2_LEN) {
      return SNMP_ERR_GENERR;
    }
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);
    pCommunity[pduPtr->community_len] = '\0';

    if ((pduPtr->transport_data_length != sizeof(struct sockaddr_in)) ||	/* we don't know what this is */
        (pduPtr->transport_data == NULL)) {				/* no transport data */
      memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
    } else {
      saPtr = (struct sockaddr_in *) pduPtr->transport_data; 
      if (saPtr->sin_family != AF_INET) {
        memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
      } else {
        memcpy( (char *) pSrcIpAddr,(char *) &(saPtr->sin_addr.s_addr), (int) 4);
      }
    }

  // SNMP_STATUS AppNextObject (ObjectId *pOID, ObjectId *pIndex, VarBind *pVB,
  //  const unsigned char *DA, connst unsigned char *SA, const char *pCommunity, int SnmpVersion);
    retVal = AppNextObject ( v2OidPtr, v2IndexPtr, v2VarBindPtr,
                        pNullIpAddr, pSrcIpAddr, pCommunity,
                        SnmpVersion);

    if (retVal == SNMP_NO_ERROR) 
    {
      DEBUG ("  NOERROR\n");
      memcpy( (char *) (requestvb->name + indexOffset), (char *) (v2IndexPtr->pObjects),
        ((int) v2IndexPtr->Length) * sizeof(oid));
      requestvb->name_length = indexOffset + v2IndexPtr->Length;
      requestvb->type = 

      /* get returned value */
      *valueLen = extractV2VarBind(v2VarBindPtr, value);
      
      if (*valueLen == -1)
        retVal = SNMP_ERR_GENERR;
      else
        // Load type from V2 varbind
        requestvb->type = v2VarBindPtr->Type;
    }
    
    // If the app has allocated string storage for us, free it now.
    if ((v2VarBindPtr->Type == VBT_STRING)
    ||  (v2VarBindPtr->Type == VBT_IPADDRESS))
    {
      if (v2VarBindPtr->Value.String.pOctets)
        free (v2VarBindPtr->Value.String.pOctets);
    }
    
    return retVal;
}
/*******************/

SNMP_STATUS v2Set(oid *oidPtr, int oidLen, u_char *value, size_t valueLen, u_char type, netsnmp_agent_request_info *reqInfo)
{
    ObjectId v2Oid, *v2OidPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    struct snmp_session *sessPtr;
    struct sockaddr_in *saPtr;
    char pCommunity[COMMUNITY_MAX_V2_LEN];
    SNMP_STATUS retVal = SNMP_NO_ERROR;

    DEBUG("v2Set: Starting\n");
    fflush(stdout);
    v2OidPtr = &v2Oid;
    v2VarBindPtr = &v2VarBind;

    DEBUG("v2Set: calling VARBIND_SET_EMPTY() \n");
    fflush(stdout);
    memset (v2VarBindPtr, 0, sizeof (VarBind));

    DEBUG("v2Set: dereferencing oidLen\n");
    fflush(stdout);
    v2OidPtr->Length = oidLen;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) oidPtr, oidLen * sizeof(oid));

    DEBUG("v2Set: calling buildV2VarBind() \n");
    fflush(stdout);
    buildV2VarBind(v2VarBindPtr, value, valueLen, type);

    pduPtr = reqInfo->asp->pdu;
    if(pduPtr == NULL) {
      printf("v2Set: reqInfo->pdu is NULL! Should never happen! \n");
      fflush(stdout);
      return SNMP_ERR_GENERR;
    }
    if(pduPtr->community_len >= COMMUNITY_MAX_V2_LEN) {
      printf("v2Set: pdu->community_len is larger than COMMUNITY_MAX_V2_LEN! Should never happen! \n");
      return SNMP_ERR_GENERR;
    }
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);
    pCommunity[pduPtr->community_len] = '\0';
    DEBUG("v2Set: community = %s\n", pCommunity);
    fflush(stdout);

    if ((pduPtr->transport_data_length != sizeof(struct sockaddr_in)) ||	/* we don't know what this is */
        (pduPtr->transport_data == NULL)) {				/* no transport data */
      printf("v2Set: no source address in pdu struct, using 0.0.0.0\n");
      fflush(stdout);
      memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
    } else {
      DEBUG("v2Set: getting source address from transport_data in pdu struct\n");
      fflush(stdout);
      saPtr = (struct sockaddr_in *) pduPtr->transport_data; 
      if (saPtr->sin_family != AF_INET) {
        DEBUG("v2Set: source address not IPV4, using 0.0.0.\n");
        fflush(stdout);
        memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
      } else {
        DEBUG("v2Set: copying source address \n");
        fflush(stdout);
        memcpy( (char *) pSrcIpAddr,(char *) &(saPtr->sin_addr.s_addr), (int) 4);
        DEBUG("v2Set: pSrcIpAddr = %x.%x.%x.%x\n", pSrcIpAddr[0], pSrcIpAddr[1], pSrcIpAddr[2], pSrcIpAddr[3]);
        fflush(stdout);
      }
    }

    DEBUG("v2Set: calling AppSetObject() with \n");
    fflush(stdout);
    dumpV2Oid(v2OidPtr);
    dumpV2VarBind(v2VarBindPtr);

    // SNMP_STATUS AppSetObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion);
    retVal = AppSetObject (v2OidPtr, v2VarBindPtr, pNullIpAddr, pSrcIpAddr,
                            pCommunity, SnmpVersion);
    if( retVal != SNMP_NO_ERROR ) {
        DEBUG("v2Set:  failed with snmp error code %d\n", retVal);
        fflush(stdout);
    } else {
        DEBUG("v2Set:  success!\n");
        fflush(stdout);
    }

    return retVal;
}


SNMP_STATUS v2Test(oid *oidPtr, int oidLen, u_char *value, size_t valueLen, u_char type, netsnmp_agent_request_info *reqInfo)
{
    ObjectId v2Oid, *v2OidPtr;
    VarBind v2VarBind, *v2VarBindPtr;
    netsnmp_pdu *pduPtr;
    struct snmp_session *sessPtr;
    struct sockaddr_in *saPtr;
    char pCommunity[COMMUNITY_MAX_V2_LEN];
    SNMP_STATUS retVal = SNMP_NO_ERROR;

    DEBUG("v2Test: Starting\n");
    fflush(stdout);
    v2OidPtr = &v2Oid;
    v2VarBindPtr = &v2VarBind;

    DEBUG("v2Test: calling VARBIND_SET_EMPTY() \n");
    fflush(stdout);
    memset (v2VarBindPtr, 0, sizeof (VarBind));

    DEBUG("v2Test: dereferencing oidLen\n");
    fflush(stdout);
    v2OidPtr->Length = oidLen;
    memcpy( (char *) (v2OidPtr->pObjects),(char *) oidPtr, oidLen * sizeof(oid));

    DEBUG("v2Test: calling buildV2VarBind() \n");
    fflush(stdout);
    buildV2VarBind(v2VarBindPtr, value, valueLen, type);

    pduPtr = reqInfo->asp->pdu;
    if(pduPtr == NULL) {
      printf("v2Test: reqInfo->pdu is NULL! Should never happen! \n");
      fflush(stdout);
      return SNMP_ERR_GENERR;
    }
    if(pduPtr->community_len >= COMMUNITY_MAX_V2_LEN) {
      printf("v2Test: pdu->community_len is larger than COMMUNITY_MAX_V2_LEN! Should never happen! \n");
      return SNMP_ERR_GENERR;
    }
    memcpy( pCommunity,(char *) pduPtr->community, (int) pduPtr->community_len);
    pCommunity[pduPtr->community_len] = '\0';
    DEBUG("v2Test: community = %s\n", pCommunity);
    fflush(stdout);

    if ((pduPtr->transport_data_length != sizeof(struct sockaddr_in)) ||	/* we don't know what this is */
        (pduPtr->transport_data == NULL)) {				/* no transport data */
      printf("v2Test: no source address in pdu struct, using 0.0.0.0\n");
      fflush(stdout);
      memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
    } else {
      DEBUG("v2Test: getting source address from transport_data in pdu struct\n");
      fflush(stdout);
      saPtr = (struct sockaddr_in *) pduPtr->transport_data; 
      if (saPtr->sin_family != AF_INET) {
        printf("v2Test: source address not IPV4, using 0.0.0.\n");
        fflush(stdout);
        memcpy( (char *) pSrcIpAddr,(char *) pNullIpAddr, (int) 4);	/* use null address */
      } else {
        DEBUG("v2Test: copying source address \n");
        fflush(stdout);
        memcpy( (char *) pSrcIpAddr,(char *) &(saPtr->sin_addr.s_addr), (int) 4);
        DEBUG("v2Test: pSrcIpAddr = %x.%x.%x.%x\n", pSrcIpAddr[0], pSrcIpAddr[1], pSrcIpAddr[2], pSrcIpAddr[3]);
        fflush(stdout);
      }
    }

    DEBUG("v2Test: calling AppTestObject() with \n");
    fflush(stdout);
    dumpV2Oid(v2OidPtr);
    dumpV2VarBind(v2VarBindPtr);

    // SNMP_STATUS AppTestObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion);
    retVal = AppTestObject (v2OidPtr, v2VarBindPtr, pNullIpAddr, pSrcIpAddr,
                            pCommunity, SnmpVersion);
    if( retVal != SNMP_NO_ERROR ) {
        DEBUG("v2Test:  failed with snmp error code %d\n", retVal);
        fflush(stdout);
    } else {
        DEBUG("v2Test:  success!\n");
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
  printf("T-bcmV2_input_parameters\n");
  switch (param)
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
        printf("enable -D debugging\n");
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
    case 'q':
        snmp_set_quick_print(1);
        break;

    case 'v':
        printf("\nNET-SNMP version:  %s\n", netsnmp_get_version());
        break;

    case 'V':
        netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
               NETSNMP_DS_AGENT_VERBOSE, 1);
        break;

    default:
        printf("bcmV2_input_parameters: unknown param <%c>\n",param);
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

   if (done_init) {
        return 1;
   }

   DEBUG("bcmV2_init_snmp -starting\n");
   done_init = 1;

   /** make the type available everywhere else */
   if (type && !netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				       NETSNMP_DS_LIB_APPTYPE)) {
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
 
    /* setup_log(0, dont_zero_log, stderr_log, syslog_log, logfile);  */
  
#ifdef BUFSIZ
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
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
    
    //init_bcmV2SessionAndTransport();
    
    // snmp_shutdown("snmpd"); TODO may need this!!!!!!!
    DEBUG("bcmV2_agent_init: -finished \n");
    return 0;
}                               

void printTransport (netsnmp_transport *pTransport)
{
  int Loop;
  
  printf ("SNMP transport:\n");
  
  if (pTransport == NULL)
  {
    printf ("  NULL\n");
    return;
  }  
    
  printf ("  domain=");
    for (Loop = 0; Loop < pTransport->domain_length; Loop++)
      printf (".%d", pTransport->domain[Loop]);
    printf ("\n");
  printf ("  local=");
    for (Loop = 0; Loop < pTransport->local_length; Loop++)
      printf (".%d", pTransport->local[Loop]);
    printf ("\n");
  printf ("  remote=");
    for (Loop = 0; Loop < pTransport->remote_length; Loop++)
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

// Net-SNMP does not really handle a max send packet size gracefully.  When
// building a packet it never considers the packet size at all.  Instead, right
// before it goes to send it out, if the packet is too big it just discards it.
// Specifically, consider what it does for a bulkwalk.  Say we kick off a 
// bulkwalk using the UCD command line manager.  Well, UCD requests a bulk-get
// with 1000 varbinds in it.  Net-SNMP dutifully creates the packet, never mind
// the fact that it is freaking gigantic.  Then our poor little embedded OS
// totally barfs on this giant packet.  So let us protect our OS by letting
// Net-SNMP build the packet, but if it is too big then we will begin dropping
// varbinds and trying again until he produces something reasonable.
int bcmV2_realloc_build (netsnmp_session *session, netsnmp_pdu *pdu, u_char **pkt, size_t *pkt_len, size_t *offset)
{
  int rc;
  int numVBs = 0, vbCount = 0;
  netsnmp_variable_list *pVB = NULL, *pNextVB = NULL, *pPrevVB = NULL;
  u_char *newPkt = NULL;
  u_char *pduData;
  netsnmp_pdu *pReqPdu;
  
  
  DEBUG ("bcmV2_realloc_build: pkt=%p, length=%d, offset=%d, cmd=%d\n", *pkt, *pkt_len, *offset, pdu->command);
  memset (*pkt, 0, *pkt_len);
  
  // We remember the request PDU in our session's myvoid pointer in the
  // bcmV2_check_pdu function, below.  Retrieve it now so we can tell if
  // this is a bulk request.
  pReqPdu = (netsnmp_pdu *) session->myvoid;

  // Try building the packet the usual way, then check the length.  If it's
  // smaller than our max packet size, everything is cool and we are done.
  // Note that the pkt_len argument is how big the buffer is.  The actual
  // size of the resulting pkt is returned in *offset.
  rc = snmp_build (pkt, pkt_len, offset, session, pdu);
  
  // If we have no max size specified OR if the packet is smaller than our
  // max size, we are cool.  Also, if this is not a GETBULK then we can't do
  // the thing below where we drop off varbinds, so if this is not GETBULK then
  // we gotta just take our chances even if the packet is too big.  Someday 
  // should we insert a TOO_BIG error here instead?
  if ((gMaxPacketSize == 0) 
  ||  (*offset <= gMaxPacketSize)
  ||  (pReqPdu->command != SNMP_MSG_GETBULK))
  {
    if ((gMaxPacketSize == 0) 
    ||  (*offset <= gMaxPacketSize))
    {
      DEBUG ("  Net-SNMP built packet OK (%d bytes), nothing for me to do.\n", *offset);
      DEBUG ("  pkt=%p, length=%d, offset=%d\n", *pkt, *pkt_len, *offset);
    }
    else
      DEBUG ("  Packet is too big (%d bytes) but it's not GETBULK (%d) so we must try anyway\n",
             *offset, pdu->command);
    
    // Now we must do something odd.  snmp_build puts the PDU data
    // at the END of the supplied buffer, not at the start.  God alone knows
    // why.  What's more, the function that calls us is going to free the buffer
    // that is pointed to by pkt, so that means that we must re-allocate a new
    // buffer, copy the stuff from the END of the old buffer in there, then
    // free the old buffer and reassign.  Ugh, what a mess!
    pduData = (*pkt) + ((*pkt_len) - (*offset));
    newPkt = malloc(*offset);
    memcpy (newPkt, pduData, *offset);
    free (*pkt);
    *pkt = newPkt;
    
    // Set our session max packet size to 0.  Since NetSnmp will DROP any packet
    // bigger than this value, we would like him to try his best even if it
    // is too big.  This will get reset back to our global max packet size when
    // we get the next SNMP packet in via AgentProcessSnmpPacket.
    session->sndMsgMaxSize = 0;
    
    return rc;
  }
  
  // If this is an oversized GETBULK then count how many varbinds we have.  
  // We will start lopping off varbinds in the same proprortion to which we are 
  // oversized.  That is, if we are oversized by 2x, then we will lopp half the 
  // varbinds.  If we are oversized by 4x, we will lop 3/4 of the varbinds.  
  // Otherwise we might have to iterate here a seriously lot of times which
  // can make us very slow in responding.
  vbCount = 0;
  pNextVB = pdu->variables;
  while (pNextVB)
  {
    pNextVB = pNextVB->next_variable;
    vbCount++;
  }
  
  while (*offset > gMaxPacketSize)
  {
    DEBUG ("  pkt=%p, length=%d, offset=%d\n", *pkt, *pkt_len, *offset);
    DEBUG ("    GETBULK packet is too long (%d vs. %d), truncating and trying again.\n",
           *offset, gMaxPacketSize);
           
    // I can never remember when I need to do the old float-cast when doing
    // this kind of thing.  Better just cast the hell out of it...
    numVBs = (float)vbCount * ((float)gMaxPacketSize / (float)(*offset));
    
    // If the calculation above yields the same number we started with, then
    // reduce by one.
    if (numVBs == vbCount)
      numVBs = vbCount-1;
    
    DEBUG ("    We have %d varbinds.  Estimate we can only support %d.\n", 
           vbCount, numVBs);
           
    vbCount = 0;
    pVB = NULL;
    pNextVB = NULL;
    pPrevVB = NULL;
           
    // Spin down the varbind list and 86 the last one, then try again.
    pNextVB = pdu->variables;
    while (pNextVB)
    {
      vbCount++;
      pPrevVB = pVB;
      pVB = pNextVB;
      pNextVB = pNextVB->next_variable;
      
      if (vbCount == numVBs)
        break;
    }
    
    // If we can't fit even a single VB because it contains some huge string
    // or something, then should we bail with TOO_BIG or just try our best?
    // Try our best, I suppose.
    if (pPrevVB == NULL)
    {
      DEBUG ("    bcmV2_realloc_build: Even 1 VB is too big!  Oh well, try and see...\n");
      break;
    }
    
    DEBUG ("    Freeing from VB %d and retrying...\n", vbCount);
    pPrevVB->next_variable = NULL;
    snmp_free_varbind (pVB);
    
    // Let Net-SNMP try building again.  Bump our offset back to the
    // start of the packet.
    *offset = 0;
    rc = snmp_build (pkt, pkt_len, offset, session, pdu);
  }
  
  // Now we must do something odd.  snmp_build puts the PDU data
  // at the END of the supplied buffer, not at the start.  God alone knows
  // why.  What's more, the function that calls us is going to free the buffer
  // that is pointed to by pkt, so that means that we must re-allocate a new
  // buffer, copy the stuff from the END of the old buffer in there, then
  // free the old buffer and reassign.  Ugh, what a mess!
  pduData = (*pkt) + ((*pkt_len) - (*offset));
  newPkt = malloc(*offset);
  memcpy (newPkt, pduData, *offset);
  free (*pkt);
  *pkt = newPkt;
  
  // Set our session max packet size to 0.  Since NetSnmp will DROP any packet
  // bigger than this value, we would like him to try his best even if it
  // is too big.  This will get reset back to our global max packet size when
  // we get the next SNMP packet in via AgentProcessSnmpPacket.
  session->sndMsgMaxSize = 0;
  
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
   struct sockaddr_in *pRemoteAddr;
   netsnmp_transport *pTransport;
   SNMP_STATUS Status;
   int PduType;
   char *pCommunity;
   
   
   DEBUG ("bcmV2_check_pdu for PDU at %p, session=%p, securityName=%p\n", pdu, session, pdu->securityName);
   
   // Put our PDU in the session's myvoid pointer.  This is so we can retrieve
   // it later in our bcmV2_realloc_build function, above, where we need to
   // check the command type.
   DEBUG ("  Remembering request PDU in session->myvoid (previously %p)\n", session->myvoid);
   session->myvoid = pdu;
   
   // Get the transport associated with this session.  From this shall we 
   // extract the source & destination addresses and ports.
   // pTransport = snmp_sess_transport (session);
   pTransport = gpTransport;
   
   if (pTransport == NULL)
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
   if (pdu->community_len)
   {
     pCommunity = (char *) malloc (pdu->community_len + 1);
     memset (pCommunity, 0, pdu->community_len + 1);
     memcpy (pCommunity, pdu->community, pdu->community_len);
     DEBUG ("  Allocated %d bytes for community string '%s'\n", pdu->community_len + 1, pCommunity);
   }
   else // Probably (definitely?) SNMPv3
     pCommunity = "";
   
   // Clear the security name that we're passing over.
   memset (pSecName, 0, sizeof (pSecName));
   
   // Call our API to validate the request.
   Status = AppValidateSnmpRequest (PduType, pdu->version, pSA, SrcPort, pDA, DstPort, pCommunity, pSecName, sizeof(pSecName));
     
   // Now we can free the community copy we made.
   if (pdu->community_len)
     free (pCommunity);
   
   // If the app doesn't like the request, return non-zero here and we will
   // timeout.
   if ((Status != SNMP_NO_ERROR) && (Status != SNMP_NO_OPINION))
   {
     DEBUG ("AppValidateSnmpRequest returned error %d, no access.\n", Status);
     
     // Increment statistics.
     if (Status == SNMP_BAD_COMMUNITY_NAME)
       snmp_increment_statistic(STAT_SNMPINBADCOMMUNITYNAMES);
     else if (Status == SNMP_BAD_COMMUNITY_USE)
       snmp_increment_statistic(STAT_SNMPINBADCOMMUNITYUSES);
     
     return 0;
   }
   
   if ((pdu->version != SNMP_VERSION_3) 
   &&  (pSecName[0] != 0))
   {
     // Load the security name we got back from our validate call into the PDU.
     // Note that this will be freed by snmp_free_pdu later, so we must 
     // allocate storage.
     if (pdu->securityName == NULL)
     {
       DEBUG ("NULL security name in the PDU, allocating and copying.\n");
     }
     else
     { 
       DEBUG ("PDU security name WAS %d bytes, %s.  Freeing and overwriting with %s.\n", pdu->securityName, pdu->securityNameLen, pSecName);
       SNMP_FREE (pdu->securityName);
     }
     
     pdu->securityName = (char *) malloc (strlen (pSecName) + 1);
     strcpy (pdu->securityName, pSecName);
     pdu->securityNameLen = strlen(pSecName);
   }
   
   DEBUG ("AppValidatSnmpRequest returned NO_ERROR, access granted.\n");
   
   return 1;
}


// Return 0 if in view, 5 if not.  See vacm.h for real return codes, but 0 (IN_VIEW)
// and 5 (NOT_IN_VIEW) are the only ones that won't cause us to timeout...
// The reason we need this callback is to handle assigning views for SNMPv1 and
// SNMPv2c queries, either via coexistence (RFC-2576) or by the V1V2View
// associated with the BcmSnmpAgent object.
int bcmV2_vacm_in_view_callback(int majorID, int minorID, void *serverarg, void *clientarg)
{
    struct view_parameters *view_parms = (struct view_parameters *) serverarg;
    int retval;
    
    
    DEBUG ("bcmV2_vacm_in_view_callback\n");

    // If we have no view_parms we are sewiously scwewed.
    if (view_parms == NULL)
    {
        DEBUG ("  Couldn't get view_parms, return 0. (majorID=%d, minorID=%d)\n",
               majorID, minorID);
        return VACM_NOTINVIEW;
    }
    
    // If this is an SNMPv1 or SNMPv2c request, we need to assign a view to use.
    // If we're in coexistence mode as defined by RFC-2576, we will resolve the
    // view from the security name in the PDU (which was shoved in there in
    // bcmV2_check_pdu, above).  Otherwise, we'll simply get the view from the 
    // app based on the DA from the PDU.
    if (view_parms->pdu->version != SNMP_VERSION_3)
    {
      const char *pView;
      int Loop;
      
      
      // Note that the check for view_parms->namelen here is because this callback
      // gets called several times per VB, and not every time do we have an OID
      // to check yet.  So if that's the case, just return 0 and move on.
      if (view_parms->namelen == 0)
      {
          DEBUG ("  No OID specified, just return 0. (majorID=%d, minorID=%d)\n",
                 majorID, minorID);
          return VACM_SUCCESS;
      }
    
      if ((view_parms->pdu->securityName == NULL)
      ||  (view_parms->pdu->securityName[0] == 0))
      {
        DEBUG ("  No security name in this PDU, use view from the app.\n");
        
        // Extract the DA for this PDU from our transport.
        pView = AppGetV1V2View (gpTransport->local);
      }
      else
      {
        //printf ("UNFINISHED - Security name (%s) supplied in this PDU, resolve views!\n", 
        //view_parms->pdu->securityName);
        pView = "UNFINISHED";
        pView = "docsisManagerView";  //hack for now until I implement a findViewFromSecName function
      }
      
      DEBUG ("  Using view %s for this query, OID (%d elements) = ", pView);
      for (Loop = 0; Loop < view_parms->namelen; Loop++)
        DEBUG (".%d", view_parms->name[Loop]);
      DEBUG ("\n");
      
      /*\
       *  Now at this point I WOULD just call vacm_in_view same as if we had
       *  not created this callback.  But it always fails here for some
       *  reason.  So just check the view ourselves.
      \*/
      
      // Now check the OID for the view to use.  We can get the OID straight
      // from the supplied view_parms argument.  Note that this had better only
      // return VACM_SUCCESS(0) or VACM_NOTINVIEW(5) or else we will time out...
      retval = vacm_checkSubtree (pView, view_parms->name, view_parms->namelen);
    }
    else
    {
      // For SNMPv3 just call the standard function to check view status.
      DEBUG ("  SNMPv3 query or no OID, let NetSNMP handle it.\n");
      retval = vacm_in_view(view_parms->pdu, view_parms->name,
                            view_parms->namelen, view_parms->check_subtree);
    }
                          
    DEBUG ("bcmV2_vacm_in_view_callback returning %d\n", retval);
                          
    if (retval != VACM_SUCCESS)
        view_parms->errorcode = retval;
        
    return retval;
}


/* 
   In order to satisfy AgentProcessSnmpPkt, we must call the function
   _sess_process_packet.  This function takes the following arguments, which
   we must create and/or collect here for future use:
   
   void *sessp: gpSessionList
     This is a session_list structure, disguised as a void pointer.  We will 
     create such a list here. 
   
   netsnmp_session *sp: gpSession
     This is the one-and-only session in our session list.  We will be using
     the same session for every packet, regardless of which IP address it
     arrived on.  We have abandoned the net-snmp notion of sessions by using
     the V2 thread. 
   
   struct snmp_internal_session *isp: gpInternalSession
   
   netsnmp_transport *transport: gpTransport
     The one-and-only transport that we will be using.  On each packet we will
     change its socket and IP address info.  This transport is created here.
   
   void *opaque:
     A sockaddr_in structure, disguised as a void pointer.  This is the source
     address of the SNMP query.  We create this in AgentProcessSnmpPkt, not
     here.
     
   int olength:
     sizeof (sockaddr_in)
     
   u_char * packetptr:
     The actual packet buffer, which comes to us from V2.
     
   int length:
     The size of the packet buffer, also from V2.
*/
void init_bcmV2SessionAndTransport ()
{
  netsnmp_session TempSession;
  

  //DEBUG ("init_bcmV2SessionAndTransport\n");
  
  /* Create our transport. */
  
  // Create this transport on a phony port.  We will usurp the port field
  // each time AgentProcessSnmpPkt / bcmV2_process_snmp_packet is called anyway.
  gpTransport = netsnmp_tdomain_transport("udp:420", 1, "udp");
  
  // We are not going to use the socket that net-snmp created for us, so
  // close it and zero it out.  The actual socket is passed to us from
  // V2, and that's the one we'll be using.
  close (gpTransport->sock);
  gpTransport->sock = 0;
  
  // Make way for the local address if necessary.
  if (gpTransport->remote == NULL)
  {
    gpTransport->remote = (char *) malloc (6);
    gpTransport->remote_length = 6;
  }
  
  //printTransport (gpTransport);
  
  /* Create all of our various 'session' related things.  This was taken
     largely from netsnmp_register_agent_nsap, changed as necessary to suit
     our evil purposes. */
  
  // This call will initialize certain members of the TempSession and zero out
  // everything else.
  snmp_sess_init(&TempSession);
  
  // Other members we will set explicitly here:
  TempSession.version = SNMP_DEFAULT_VERSION;
  TempSession.callback = handle_snmp_packet;
  TempSession.authenticator = NULL;
  TempSession.flags = netsnmp_ds_get_int(NETSNMP_DS_APPLICATION_ID, 
                NETSNMP_DS_AGENT_FLAGS);
  TempSession.isAuthoritative = SNMP_SESS_AUTHORITATIVE;
  
  // Impose a reasonable constraint on our max message size to send.
  gMaxPacketSize = 1400;
  
  // Get at our session list pointer here by calling snmp_sess_add_ex instead
  // of snmp_add.  This call will copy our TempSession structure into
  // gpSessionList->session, from which we will extract it.  Then we are
  // finished with TempSession.
  
  /* Also hook any callbacks that we might be interested in here. */
  
  // realloc bulid is necessary because Net-SNMP ignores the sndMsgMaxSize
  // above.  Actually what it does is goes ahead and builds a big whopper of
  // a packet, then if it's too big it just dumps it.  No TOO_BIG error or
  // anything else.  So we'll hook realloc_build so we can make the packet
  // smaller if we need to.  BTW this is mainly an issue with BULK operations.
  
  // hook_pre is used for SNMP authentication, this is where we'll call
  // AppValidateSnmpRequest.
  
  // hook_post might be used for AppValidateSetRequest..
  
  gpSessionList = snmp_sess_add_ex(&TempSession, gpTransport, 
                  netsnmp_agent_check_packet,   /* pre-parse */
                  NULL,                         /* parse */
                  bcmV2_check_pdu,              /* post-parse */
                  NULL,                         /* build */
                  bcmV2_realloc_build,          /* realloc-build */
                  NULL,                         /* check */
                  NULL);                        /* create-pdu */
                  
  if (gpSessionList == NULL) {
      DEBUG ("gpSessionList = NULL!\n");
      free(gpSession);
      return;
  }
  //DEBUG ("gpSessionList=%p\n", gpSessionList);
  
  // Remember our session, get the one that snmp_sess_add_ex created for us.
  // The local TempSession structure we are now finished with.
  gpSession = gpSessionList->session;
  
  // Add our session list to the global Sessions list from snmp_api.c.  If we 
  // were operating within the normal bounds of net-snmp this would be done for
  // us in snmp_add.
  snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
  gpSessionList->next = Sessions;
  Sessions = gpSessionList;
  snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);

  // Remember the 'internal' session.
  gpInternalSession = gpSessionList->internal;
  if (gpInternalSession == NULL) {          /*  over-cautious  */
      DEBUG ("gpInternalSession = NULL!\n");
      free(gpSession);
      return;
  }
  
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
                         
  // Unregister the default callbacks.  We want to replace them with our own,
  // not add our own to them.
  snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_ACM_CHECK, vacm_in_view_callback,
                           NULL, 0);
  snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                         SNMPD_CALLBACK_ACM_CHECK_INITIAL,
                         vacm_in_view_callback, NULL, 0);
  snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                         SNMPD_CALLBACK_ACM_CHECK_SUBTREE,
                         vacm_in_view_callback, NULL, 0);
  
  //DEBUG ("gpInternalSession=%p\n", gpInternalSession);

  DEBUG ("SNMP session and transport initialized OK!\n");
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
    int i;
    int   engineIDLength;
    u_char engineID[USM_MAX_ID_LENGTH];
    int numOfBoots;
    
    numOfBoots = 1;
        
    //init the time the engine started
    gettimeofday(&snmpv3starttime, NULL);
     
    //get our engineId
    engineIDLength = snmpv3_get_engineID(engineID, USM_MAX_ID_LENGTH);

    //set boots and time for our engineId
    
    //printf("engineIdlen = %d\n",engineIDLength);
    //for (i=0; i<engineIDLength; i++)
    //{
    //    printf("%x",engineID[i]);
    //}
    
    set_enginetime(engineID, engineIDLength,
                    numOfBoots,//snmpv3_local_snmpEngineBoots(),
                    snmpv3_local_snmpEngineTime(),
                    TRUE);

    return 0;
}


/* This API is called by the V2 API AgentProcessSnmpPacket */

int bcmV2_process_snmp_packet (unsigned char *pPkt, int PktLen, int Socket,
                              unsigned long SrcIp, unsigned long DstIp, 
                              int SrcPort, int DstPort)
{
  int Result;
  struct sockaddr_in        *pRemoteAddr;    /* remote transport address */
  
  
  DEBUG("T-bcmV2_process_snmp_packet -start\n");
  DEBUG("pPkt=%d, PktLen=%d\n, Socket=%d, SrcIp=0x%x, DstIp=0x%x, SrcPort=%d, DstPort=%d\n",pPkt,PktLen,Socket,SrcIp,DstIp,SrcPort,DstPort);
/*  
  snmp_set_do_debugging(1);
  netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, 
              NETSNMP_DS_AGENT_VERBOSE, 1);
  printf("debugging and verbose on - YES\n");
*/  
  // The remote addr is passed as 'opaque' data in the _sess_process_packet
  // call and will be freed in that call.  So we must malloc it here.  
  pRemoteAddr = (struct sockaddr_in *) malloc (sizeof(struct sockaddr_in));
  memset (pRemoteAddr, 0, sizeof(struct sockaddr_in));

  pRemoteAddr->sin_family = AF_INET;
  pRemoteAddr->sin_addr.s_addr = SrcIp;
  pRemoteAddr->sin_port = htons(SrcPort);
  
  // Load local address into our transport.
  gpTransport->local[0] = (DstIp >> 24) & 0xFF;
  gpTransport->local[1] = (DstIp >> 16) & 0xFF;
  gpTransport->local[2] = (DstIp >> 8) & 0xFF;
  gpTransport->local[3] =  DstIp & 0xFF;
  gpTransport->local[4] = (DstPort >> 8) & 0xFF;
  gpTransport->local[5] = DstPort & 0xFF;

  // Load remote address into our transport.  
  gpTransport->remote[0] = (SrcIp >> 24) & 0xFF;
  gpTransport->remote[1] = (SrcIp >> 16) & 0xFF;
  gpTransport->remote[2] = (SrcIp >> 8) & 0xFF;
  gpTransport->remote[3] =  SrcIp & 0xFF;
  gpTransport->remote[4] = (SrcPort >> 8) & 0xFF;
  gpTransport->remote[5] = SrcPort & 0xFF;
  
  // Now the socket
  gpTransport->sock = Socket;

  //printf ("Here we go...\n");
  //printTransport (gpTransport);
  
  // Initialize the session's max packet size to our global max size.
  // This is necessary because we want to use this value in the NetSNMP code
  // to impose some sane limit to the number of varbinds that get loaded in
  // a GETBULK.  We must do this for each packet because later, we will
  // zero this value back out so that we don't wind up getting packets dropped
  // on the floor if we wind up with an unavoidably large packet.  NetSNMP will
  // drop any packet with a size larger than session->sndMsgMaxSize.
  gpSession->sndMsgMaxSize = gMaxPacketSize;

  // Call _sess_process_packet.  This is an internal function from snmp_api.c.
  Result = _sess_process_packet (gpSessionList, gpSession, gpInternalSession, gpTransport, pRemoteAddr, sizeof(struct sockaddr_in), pPkt, PktLen);
  
  //printf ("freeing packet.\n");
  free (pPkt);
  DEBUG("T-bcmV2_process_snmp_packet -end\n");

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


// Handler for other instance of scalar objects registered to bcmV2_do_scalar,
// below.  If we don't use this handler then we will get an incorrect
// noSuchObject error for scalar.X (where X is not zero) or just plain scalar
// queries rather than noSuchInstance.
int
bcmV2_scalar_nosuchinstance(netsnmp_mib_handler *handler,
                            netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo,
                            netsnmp_request_info *requests)
{
    int Loop;
    
    
    
    DEBUG ("bcmV2_scalar_nosuchinstance for OID \n");
    for (Loop = 0; Loop < requests->requestvb->name_length; Loop++)
      DEBUG (".%d", requests->requestvb->name[Loop]);
    DEBUG ("\n");

    switch (reqinfo->mode) {
    case MODE_GETNEXT:
    case MODE_GETBULK:
        return SNMP_ERR_NOERROR;

    case MODE_GET:
        netsnmp_set_all_requests_error(reqinfo, requests,
                                       SNMP_NOSUCHINSTANCE);
        return SNMP_ERR_NOERROR;

    default:
        netsnmp_set_all_requests_error(reqinfo, requests,
                                       SNMP_ERR_NOSUCHNAME);
        return SNMP_ERR_NOERROR;
    }
}


int
bcmV2_do_scalar (netsnmp_mib_handler *handler,
           netsnmp_handler_registration *reginfo,
           netsnmp_agent_request_info *reqinfo,
           netsnmp_request_info *requests)
{
    /*
     * We are never called for a GETNEXT if it's registered as an
     * "instance", as it's "magically" handled for us.  
     */

    /*
     * an instance handler also only hands us one request at a time, so
     * we don't need to loop over a list of requests; we'll only get one. 
     */

    int             retVal = SNMP_ERR_NOERROR;

    u_char          var[STRINGMAX];
    size_t          varLen;
    
    DEBUG ("bcmV2_do_scalar\n");

    switch (reqinfo->mode) {

    case MODE_GET:
        retVal =
            v2GetObject(requests->requestvb, var, &varLen, reqinfo);
        if (retVal == SNMP_ERR_NOERROR)
            snmp_set_var_typed_value(requests->requestvb, requests->requestvb->type,
                                     var, varLen);
        /*
         * if V2 returns NOSUCH, netsnmp wants this handler
         * to return NOERR, but we netsnmp_set_request_error
         * to SNMP_NOSUCHOBJECT or SNMP_NOSUCHINSTANCE.  
         * Tricky, tricky!
         */
        else if (retVal == SNMP_NO_SUCH_OBJECT)
        {
            DEBUG ("NO_SUCH_OBJECT\n");
            netsnmp_set_request_error(reqinfo, requests, SNMP_NOSUCHOBJECT);
            retVal = SNMP_ERR_NOERROR;
        }
        else if (retVal == SNMP_NO_SUCH_INSTANCE)
        {
            DEBUG ("NO_SUCH_INSTANCE\n");
            netsnmp_set_request_error(reqinfo, requests, SNMP_NOSUCHINSTANCE);
            retVal = SNMP_ERR_NOERROR;
        }
        else
            netsnmp_set_request_error(reqinfo, requests, retVal);
        break;

        /*
         * SET REQUEST
         *
         */
    case MODE_SET_RESERVE1:
/*      if (requests->requestvb->type != ASN_OCTET_STR) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_WRONGTYPE);
            retVal = SNMP_ERR_WRONGTYPE;
        } else*/ {
            retVal = v2Test(requests->requestvb->name,
                            requests->requestvb->name_length,
                            requests->requestvb->val.string,
                            requests->requestvb->val_len,
                            requests->requestvb->type, reqinfo);
            if (retVal != SNMP_ERR_NOERROR)
                netsnmp_set_request_error(reqinfo, requests, retVal);
        }
        break;

    case MODE_SET_RESERVE2:
        break;

    case MODE_SET_FREE:
        break;

    case MODE_SET_ACTION:
        break;

    case MODE_SET_COMMIT:
        /*
         * really do the set! 
         */
        retVal = v2Set(requests->requestvb->name,
                       requests->requestvb->name_length,
                       requests->requestvb->val.string,
                       requests->requestvb->val_len,
                       requests->requestvb->type, reqinfo);
        if (retVal != SNMP_ERR_NOERROR)
            netsnmp_set_request_error(reqinfo, requests, retVal);
        break;

    case MODE_SET_UNDO:
        /*
         * Can't undo V2 sets! 
         */
        break;

    default:
        /*
         * we should never get here, so this is a really bad error 
         */
        return SNMP_ERR_GENERR;
    }

    return retVal;
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
    char           *tmpVal[MAX_OID_LEN * sizeof(oid)];
    size_t          tmpValLen;
    int             tmpAsnType, tmpOidLen;
    oid             tmpOid[MAX_OID_LEN];
    int             retVal = SNMP_ERR_NOERROR;

    for (request = requests; request; request = request->next) {
        var = request->requestvb;
        if (request->processed != 0)
            continue;
            
        memset (&tmpVar, 0, sizeof(netsnmp_variable_list));
        tmpVar.name = tmpOid;

        /*
         * perform anything here that you need to do before each
         * request is processed. 
         */

        switch (reqinfo->mode) 
        {
        case MODE_GET:
            retVal = v2GetObject (var, tmpVal, &tmpValLen, reqinfo);
            if (retVal == SNMP_ERR_NOERROR)
                snmp_set_var_typed_value(request->requestvb, request->requestvb->type, (u_char *) tmpVal, tmpValLen);
            /*
             * if V2 returns NOSUCH, netsnmp wants this handler
             * to return NOERR, but we netsnmp_set_request_error
             * to SNMP_NOSUCHOBJECT or SNMP_NOSUCHINSTANCE.  
             * Tricky, tricky!
             */
            else if (retVal == SNMP_NO_SUCH_OBJECT) 
            {
                DEBUG ("  NO_SUCH_OBJECT\n");
                netsnmp_set_request_error(reqinfo, requests, SNMP_NOSUCHOBJECT);
                retVal = SNMP_ERR_NOERROR;
            } 
            else if (retVal == SNMP_NO_SUCH_INSTANCE)
            {
                DEBUG ("  NO_SUCH_INSTANCE\n");
                netsnmp_set_request_error(reqinfo, requests, SNMP_NOSUCHINSTANCE);
                retVal = SNMP_ERR_NOERROR;
            }
            else
                netsnmp_set_request_error(reqinfo, requests, retVal);
            break;
            
        case MODE_GETNEXT:
            DEBUGMSGTL(("bcmV2_do_tabular", "GETNEXT\n"));
            DEBUG ("bcmV2_do_tabular: GETNEXT.\n");
            
            /*
             * Check to see if var is in this subtree. If not, just
             * get the first object.
             */
             
            if ((netsnmp_oid_is_subtree (reginfo->rootoid, reginfo->rootoid_len, request->requestvb->name, request->requestvb->name_length) == 0)
            &&  (request->requestvb->name_length >= reginfo->rootoid_len + 2)) 
            {
                DEBUGMSGTL(("bcmV2_do_tabular",
                            "In Subtree - using var:\n"));
                DEBUG ("  In subtree.\n");
                
                tmpVar.name_length = request->requestvb->name_length;
                memcpy((char *) tmpVar.name, (char *) request->requestvb->name, request->requestvb->name_length * sizeof(oid));
                retVal = v2GetNextObject (&tmpVar, reginfo->rootoid_len + 2, tmpVal, &tmpValLen, reqinfo);
                
                if ((retVal == SNMP_ERR_NOSUCHNAME) || (retVal == SNMP_ERR_NOACCESS))
                {
                    /*
                     * more columns to try.  Make the name_length as long as 
                     * the rootoid + 2 (table.1.column) and increment the last
                     * element in the name, thus incrementing the column. 
                     */
                    tmpVar.name_length = reginfo->rootoid_len + 2;
                    tmpVar.name[tmpVar.name_length - 1]++;
                    
                    // If we have some columns which are not-accessible (as is often
                    // the case with index column(s)), V2 will return NO_ACCESS.  So
                    // as long as we keep getting NO_ACCESS keep incrementing the 
                    // column field and look for the first accessible column.
                    do
                    {
                        DEBUG ("  Move on to column %d...\n", tmpVar.name[tmpVar.name_length - 1]);
                        retVal = v2GetNextObject (&tmpVar, tmpVar.name_length, tmpVal, &tmpValLen, reqinfo);
                    } while (retVal == SNMP_ERR_NOACCESS);
                }
            } 
            else /* not in subtree */
            {            
                DEBUG ("  Not in subtree.  Get first column.\n");
                
                // Try column 1 first.  If that doesn't fly, then keep
                // incrementing. 
                tmpVar.name_length = reginfo->rootoid_len;
                memcpy((char *) tmpVar.name, (char *) reginfo->rootoid, reginfo->rootoid_len * sizeof(oid));
                tmpVar.name[tmpVar.name_length] = 1;
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
                    retVal = v2GetNextObject (&tmpVar, tmpVar.name_length, tmpVal, &tmpValLen, reqinfo);
                } while (retVal == SNMP_ERR_NOACCESS);
            }

            if (retVal == SNMP_ERR_NOERROR) 
            {
                int Loop;
                DEBUG ("  NOERROR\n");
                
                for (Loop = 0; Loop < tmpVar.name_length; Loop++)
                  DEBUG (".%d", tmpVar.name[Loop]);
                DEBUG ("\n");
                
                snmp_set_var_objid(var, tmpVar.name, tmpVar.name_length);
                snmp_set_var_typed_value(var, tmpVar.type, (u_char *) tmpVal, tmpValLen);
            } 
            /*
             * if V2 returns NOSUCH, netsnmp wants this handler
             * to return NOERR, but we netsnmp_set_request_error
             * to SNMP_NOSUCHOBJECT or SNMP_NOSUCHINSTANCE.  
             * Tricky, tricky!
             */
            else if (retVal == SNMP_NO_SUCH_OBJECT) 
            {
                DEBUG ("  NO_SUCH_OBJECT\n");
                // Don't do this here.  Otherwise we will get a nasty error
                // printed to the console...
                //netsnmp_set_request_error(reqinfo, requests, SNMP_NOSUCHOBJECT);
                retVal = SNMP_ERR_NOERROR;
            } 
            // I don't THINK that we can ever possibly get this back from
            // V2 on a GETNEXT, but just in case...
            else if (retVal == SNMP_NO_SUCH_INSTANCE)
            {
                DEBUG ("  NO_SUCH_INSTANCE\n");
                netsnmp_set_request_error(reqinfo, requests, SNMP_NOSUCHINSTANCE);
                retVal = SNMP_ERR_NOERROR;
            }
            else /* retVal has a real error code */
            {      
                DEBUG ("ERROR: %d\n", retVal);
                netsnmp_set_request_error(reqinfo, requests, retVal);
            }
            break;

        case MODE_SET_RESERVE1:
            DEBUG ("bcmV2_do_tabular: Mode = MODE_SET_RESERVE\n");
            retVal = v2Test(var->name,
                            var->name_length,
                            var->val.string,
                            var->val_len,
                            var->type, reqinfo);
            if (retVal != SNMP_ERR_NOERROR)
                netsnmp_set_request_error(reqinfo, requests, retVal);
            break;

        case MODE_SET_RESERVE2:
        case MODE_SET_ACTION:
        case MODE_SET_UNDO:
        case MODE_SET_FREE:
            break;
            
        case MODE_SET_COMMIT:
            DEBUG("bcmV2_do_tabular: Mode = MODE_SET_COMMIT\n");
            /*
             * really do the set! 
             */
            retVal = v2Set(var->name,
                           var->name_length,
                           var->val.string,
                           var->val_len,
                           var->type, reqinfo);
            if (retVal != SNMP_ERR_NOERROR)
                netsnmp_set_request_error(reqinfo, requests, retVal);
            break;


        default:
            snmp_log(LOG_ERR,
                     "problem encountered in _handler: unsupported mode\n");
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
    u_char var[STRINGMAX]; // How big is this?  Am I risking my stack?
    
    
    DEBUG ("bcmV2_get_variable_list\n");
    
    while (pMO)
    {
      DEBUG ("  Converting object %d\n", ++VbCount);
      
      // Allocate storage
      pVB = SNMP_MALLOC_STRUCT(variable_list);
      
      // Zero it out
      memset (pVB, 0, sizeof(netsnmp_variable_list));
      
      // Set OID and value.  OID first.
      snmp_set_var_objid(pVB, pMO->OID.pObjects, pMO->OID.Length);
      
      // Now value.
      varLen = extractV2VarBind (&pMO->VB, var);
      snmp_set_var_typed_value(pVB, pMO->VB.Type, var, varLen); 
      
      // If this is the first VB, remember it so we can return it.
      if (pFirstVB == NULL)
        pFirstVB = pVB;
      
      // If this is not the first item on the list, set the previous
      // item's next pointer to the new structure.
      if (pPrevVB)
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
 * by the caller.
\*/
netsnmp_session *bcmV2_get_trap_session (const unsigned char *pToIp, int ToPort, const char *pCommunity, int SnmpVersion)
{
    netsnmp_session session, *sesp;
    char           *peername = NULL;
    
    
    DEBUG ("bcmV2_get_trap_session\n");

    if ((peername = malloc(16 + 4 + 32)) == NULL) 
    {
        DEBUG ("  couldn't allocate peername!\n");
        return NULL;
    } 
    else 
    {
        snprintf(peername, 16 + 4 + 32, "udp:%d.%d.%d.%d:%hu", 
                 pToIp[0],pToIp[1],pToIp[2],pToIp[3], ToPort);
                 
        DEBUG ("  peername = '%s'\n", peername);
    }

    memset(&session, 0, sizeof(netsnmp_session));
    session.peername = peername;
    session.version = SnmpVersion;
    if (pCommunity) 
    {
        session.community = (u_char *) pCommunity;
        session.community_len = strlen(pCommunity);
    }
    sesp = snmp_open(&session);
    free(peername);

    /*
     * diagnose snmp_open errors with the input netsnmp_session pointer 
     */
    if (sesp == NULL)
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
    netsnmp_variable_list *v2_vars, *last_var = NULL;
    netsnmp_pdu    *template_pdu;
    in_addr_t      *pdu_in_addr_t;
    oid             temp_oid[MAX_OID_LEN];
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
    
    if (vars)
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
    if (template_pdu == NULL) 
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
    *pdu_in_addr_t = (pFromIp[0] << 24)
                   | (pFromIp[1] << 16)
                   | (pFromIp[2] << 8)
                   | (pFromIp[3] << 0);
    template_pdu->time = sysUpTime;
    template_pdu->variables = v2_vars;

    /* No enterprise info for SNMPv2 trap */
    last_var = NULL;
    
    /* Get a session based on our input parameters */
    pTrapSession = bcmV2_get_trap_session (pToIp, ToPort, pCommunity, SNMP_VERSION_2c);

    if (pTrapSession == NULL)
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
     * send stuff to registered callbacks 
     */
    /*
     * v2 traps/informs 
     */
    template_pdu->variables = v2_vars;

    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_SEND_TRAP2, template_pdu);

    /*
     * Free memory if values stored dynamically 
     */
    snmp_set_var_value(&snmptrap_var, NULL, 0);
    
    /*
     * Ensure we don't free anything we shouldn't 
     */
    if (last_var)
        last_var->next_variable = NULL;
    template_pdu->variables = NULL;
    
    snmp_free_pdu(template_pdu);
    
    return 0;
}


int bcmV2_send_V1_trap (const unsigned char *pFromIp, const unsigned char *pToIp, int ToPort, const char *pCommunity,
                 int generic, int specific, oid *enterprise, int enterprise_length,
                 time_t sysUpTime,
                 netsnmp_variable_list *vars)
{
    static oid sysUpTime_oid[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
    static oid snmpTrapOID_oid[] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};
    netsnmp_pdu    *template_pdu;
    in_addr_t      *pdu_in_addr_t;
    oid             temp_oid[MAX_OID_LEN];
    netsnmp_session *pTrapSession = NULL;
    
    
    DEBUG ("bcmV2_send_V1_trap\n");

    /*
     * SNMPv1 traps have no sysUpTime or trap OID VBs.
     */

    /*
     *  Create a template PDU, ready for sending
     */
     
    template_pdu = snmp_pdu_create(SNMP_MSG_TRAP);
    if (template_pdu == NULL) 
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
    *pdu_in_addr_t = (pFromIp[0] << 24)
                   | (pFromIp[1] << 16)
                   | (pFromIp[2] << 8)
                   | (pFromIp[3] << 0);
    template_pdu->time = sysUpTime;
    
    /* Load variable bindings list from args */
    template_pdu->variables = vars;

    /* Get a session based on our input parameters */
    pTrapSession = bcmV2_get_trap_session (pToIp, ToPort, pCommunity, SNMP_VERSION_1);

    if (pTrapSession == NULL)
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
     * send stuff to registered callbacks 
     */
    
    /*
     * v1 traps 
     */
    template_pdu->command = SNMP_MSG_TRAP;
    template_pdu->variables = vars;

    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_SEND_TRAP1, template_pdu);

    /*
     * Ensure we don't free anything we shouldn't 
     */
    
    template_pdu->variables = NULL;
    // Er, this makes us crash, dude...
    //snmp_free_pdu(template_pdu);
    
    return 0;
}
