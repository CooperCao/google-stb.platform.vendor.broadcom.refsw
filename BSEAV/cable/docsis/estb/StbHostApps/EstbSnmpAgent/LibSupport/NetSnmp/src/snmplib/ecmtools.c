/*
/ ****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
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
// ****************************************************************************
//
//  Filename:       ecmtools.c
//  Author:         Maurice Turcotte
//  Creation Date:  21 SEP 10
//
// ****************************************************************************
//  Description:
//      Various tools to access SNMP MIB objects on the eCM. The objects
//      must be implemented on the eCM, accessible, and in view.
//
// ****************************************************************************
*/

#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/snmp_api.h>
#ifdef USE_OPENSSL
#	include <openssl/hmac.h>
#else
#ifdef USE_INTERNAL_MD5
#include <net-snmp/library/md5.h>
#endif
#endif

#include <net-snmp/library/scapi.h>
#include <net-snmp/library/keytools.h>

#include <net-snmp/library/transform_oids.h>

char           ecm_community[] = "public\0";
char           ecm_ip_address[] = "192.168.100.1";

void
dumpPdu(netsnmp_pdu * pdu)
{
  int i, len;
  long *subId;
  struct variable_list *var;
  
  if (pdu == NULL) {
    printf("dumpPdu got a NULL!\n");
    return;
  }

  printf ("pdu at %p\n", pdu);
  printf ("\tversion = %d\n", pdu->version);
  printf ("\tcommand = %d\n", pdu->command);
  printf ("\treqid = %d\n", pdu->reqid);
  printf ("\tflags = 0x%lx\n", pdu->flags);
  printf ("\ttransport_data_length = %s\n", pdu->transport_data_length);
  printf ("\ttDomainLen = %d\n", pdu->tDomainLen);
  printf ("\tcommunity = %s\n", pdu->community);
  printf ("\tcommunity_len = %d\n", pdu->community_len);

  var = pdu->variables;
  while (var) {
    printf ("\tvar at %p\n", var);
    len = var->name_length;
    subId = (long *) var->name;
    printf ("\t\toid_len = %d\n\t\t", len);
    for (i=len; i > 0; i--) {
      printf (".%d", *subId);
      subId++;
    }
    printf ("\n");
    var = var->next_variable;
  }
}

void
dumpSession(netsnmp_session * session)
{
  if (session == NULL) {
    printf("dumpSession got a NULL!\n");
    return;
  }

  printf("strlen comm = %d sizeof comm = %d\n", strlen(ecm_community), sizeof(ecm_community));

  printf ("Session at %p\n", session);
  printf ("\tversion = %d\n", session->version);
  printf ("\tretries = %d\n", session->retries);
  printf ("\ttimeout = %d\n", session->timeout);
  printf ("\tflags = 0x%lx\n", session->flags);
  printf ("\tpeername = %s\n", session->peername);
  printf ("\tremote_port = %d\n", session->remote_port);
  printf ("\tlocal_port = %d\n", session->local_port);
  printf ("\tcommunity = %s\n", session->community);
  printf ("\tcommunity_len = %d\n", session->community_len);
}

/*******************************************************************-o-******
 * ecmLibInit
 *
 * Parameters:
 *      *type   Label for the config file "type" used by calling entity.
 *
 * Call appropriately the functions to do config file loading and
 * mib module parsing in the correct order.
 */
void
ecmLibInit(const char *type)
{
    static int      done_init = 0;      /* To prevent double init's. */

    if (done_init) {
        return;
    }

    done_init = 1;

    /*
     * make the type available everywhere else
    if (type && !netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
                                       NETSNMP_DS_LIB_APPTYPE)) {
        netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID,
                              NETSNMP_DS_LIB_APPTYPE, type);
    }
     */

    _init_snmp();

    /*
     * set our current locale properly to initialize isprint() type functions
#ifdef HAVE_SETLOCALE
    setlocale(LC_CTYPE, "");
#endif
     */

    snmp_debug_init();    /* should be done first, to turn on debugging ASAP */
    netsnmp_container_init_list();
    init_callbacks();
    init_snmp_logging();
    snmp_init_statistics();
    register_mib_handlers();
    /* register_default_handlers(); */
    init_snmp_alarm();
    init_snmp_enums();

    read_premib_configs();
    init_mib();

    /* read_configs(); */

}                               /* end init_snmp() */


void
ecmSessionInit(netsnmp_session * session)
{
    /*
     * initialize session to default values
     */
    snmp_sess_init(session);

    /*
     * read in MIB database and initialize the snmp library
     */
     ecmLibInit("snmpapp");

    /*
     * init to the ecm-specific values
     */
    session->version = SNMP_VERSION_2c;

    session->peername = ecm_ip_address; /* hostname */
    session->remote_port = 161; 

    session->community = (unsigned char *) ecm_community;
    session->community_len = strlen(ecm_community);

    return;
}


/*******************************************************************-o-******
 * ecmSnmpGet
 *
 * Parameters:
 *	*ecmObj		MIB OID to get from eCM.
 *	 ecmObjLen	Length of OID in.
 *	*ecmObjType	Type of object returned.
 *	*val		Pre-allocated bytes.
 *	*valLen		Number of pre-allocated bytes.
 *      
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 *
 *
 * Do an SNMPv2c Get to the eCM SNMP Agent for the MIB object
 *
 * If successful, val has the value and val_len contains the size 
 * of the value, and ecmObjType has the type of the value / object.
 * 
 * If there is a error returned from the SNMPv2c Get operation,
 * then val, val_len, and ecmObjType are unused and the error code 
 * is returned.
 *
 * If the value that is returned by the SNMPv2 Get is too big for
 * the size of the pre-allocated val (bigger than val_len) then a
 * SNMP_TOO_BIG is returned and val and val-len are unused. 
 *
 */

int
ecmSnmpGet(oid *ecmObj, int ecmObjLen, u_char *ecmObjType,
            void *val, size_t *valLen)
{
  netsnmp_session session, *ss;
  netsnmp_pdu    *pdu;
  netsnmp_pdu    *response;
  netsnmp_variable_list *var;
  int             count;
  oid             name[MAX_OID_LEN];
  size_t          name_length;
  int             status = STAT_ERROR;
  int             retVal = SNMPERR_SUCCESS;

  /* printf("ecmSnmpGet() called\n"); */
  /*
   * set up the session
   */
  ecmSessionInit(&session);
  /* printf("ecmSnmpGet() session inited\n"); */


  if (ecmObjLen < 6) {
    return (SNMPERR_UNKNOWN_OBJID);
  }
  /* printf("ecmSnmpGet() ObjLen OK\n"); */


  SOCK_STARTUP;


  /*
   * Open an SNMP session.
   */
  ss = snmp_open(&session);
  if (ss == NULL) {
    /*
     * diagnose snmp_open errors with the input netsnmp_session pointer
     */
    snmp_sess_perror("snmpget", &session);
    SOCK_CLEANUP;
    return(SNMPERR_BAD_SESSION);
  }
  /* printf("ecmSnmpGet() session opened\n"); */


  /*
   * Create PDU for GET request and add the object name to the request.
   */
  pdu = snmp_pdu_create(SNMP_MSG_GET);
  snmp_add_null_var(pdu, ecmObj, ecmObjLen);

  /* 
  dumpSession(ss);
  dumpPdu(pdu);
  */

  /*
   * Perform the request.
   */
  status = snmp_synch_response(ss, pdu, &response);
  if (status == STAT_SUCCESS) {
    /* printf("ecmSnmpGet() STAT_SUCCESS\n"); */
    if (response->errstat == SNMP_ERR_NOERROR) {
      var = response->variables; 
      if (var->val_len <= *valLen) {   /* we have room! */
        *ecmObjType = var->type;
        *valLen = var->val_len;
        memcpy((u_char *) val, var->val.string, *valLen); 
      } else {
       retVal = SNMPERR_TOO_LONG;
      }
    } else {
      retVal = (int) (response->errstat);
    }                       /* endif -- SNMP_ERR_NOERROR */

  } else if (status == STAT_TIMEOUT) {
    /* printf("ecmSnmpGet() STAT_TIMEOUT\n"); */
    retVal = SNMPERR_TIMEOUT;
  } else {                    /* status == STAT_ERROR */
    /* printf("ecmSnmpGet() STAT_ERROR\n"); */
    retVal = SNMPERR_GENERR;
  }                           /* endif -- STAT_SUCCESS */


  /* printf("ecmSnmpGet() cleaning up\n"); */
  if (response)
    snmp_free_pdu(response);
  snmp_close(ss);
  SOCK_CLEANUP;
  return (retVal);

}                               /* end ecmSnmpGet() */

