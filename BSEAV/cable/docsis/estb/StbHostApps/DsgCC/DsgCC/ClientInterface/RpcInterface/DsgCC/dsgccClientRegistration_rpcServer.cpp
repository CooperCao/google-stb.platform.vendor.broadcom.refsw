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
//****************************************************************************
//
//  Filename:       dsgcc_svc_if.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  Nov 25, 2004
//
//****************************************************************************
//  Description:
//  C to C++ mapping for the RPC interface to the DSG Client Registrar.
//****************************************************************************

//********************** Include Files ***************************************

// My api.
#include "dsgccClientRegistration_rpc.h"
#include "dsgccClientRegistration_rpcServer.h"
#include "DsgClientRegistrar.h"


// Other objects of interest.
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#else
#include <iostream.h>
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************
static int retVal;              /* for the RPC return value */
//********************** Class Method Implementations ************************

/* These are the RPC service handlers */

int *
registerdsgclient_1_svc(struct dsgClientRegInfo *regInfo, struct svc_req * svcReqInfo) {

  retVal = 0;

    // printf("registerdsgclient_1_svc: begin");
#ifdef NOTYET
  switch (regInfo->idType) {

  case 1:

    printf("DSGCC Registrar: registering a Client using well-known MAC addr");
    printf(" %02x:%02x:%02x:%02x:%02x:%02x\n",
      regInfo->clientName[0], regInfo->clientName[1],
      regInfo->clientName[2], regInfo->clientName[3],
      regInfo->clientName[4], regInfo->clientName[5]) ;
    break;

  case 2:
    printf("DSGCC Registrar: registering a Client using CAS ID %d, \n",
      regInfo->clientId);
    break;

  case 3:
    printf("DSGCC Registrar: registering a Client using Application ID %d\n",
      regInfo->clientId);
    break;

  case 4:
    printf("DSGCC Registrar: registering a Client for Broadcast traffic\n");
    break;

  case 5:
    printf("DSGCC Registrar: registering a Client using Application name %s\n",
      regInfo->clientName);
    break;

  case 6:
    printf("DSGCC Registrar: registering a Client using Vendor ID %d\n",
      regInfo->clientId);
    break;

  }

  printf("\t to receive data on UDP Port %d.\n", regInfo->clientPort );

#endif

  retVal = BcmDsgClientRegistrarThread::GetSingletonInstance()->addDsgClient(regInfo);

  return (&retVal);

}

int *
unregisterdsgclient_1_svc(struct dsgClientRegInfo *regInfo, struct svc_req * svcReqInfo) {

  retVal = 0;

#ifdef NOTYET
  printf("DSGCC Registrar: unregistering the Client on UDP Port %d\n",
      regInfo->clientPort );
#endif

  retVal = BcmDsgClientRegistrarThread::GetSingletonInstance()->deleteDsgClient(regInfo);

  return (&retVal);

}

int *
keepalivedsgclient_1_svc(struct dsgClientRegInfo *regInfo, struct svc_req * svcReqInfo) {

  retVal = 1;

#ifdef NOTYET
  printf("DSGCC Registrar: keep-alive from the Client on UDP Port %d\n",
      regInfo->clientPort );
#endif

  // if we can't find this client, then add it 
  // if (BcmDsgClientRegistrarThread::GetSingletonInstance()->findDsgClientEntry(regInfo) == -1)
  retVal = BcmDsgClientRegistrarThread::GetSingletonInstance()->statusDsgClientEntry(regInfo); 
  if (retVal == 0)
    retVal = BcmDsgClientRegistrarThread::GetSingletonInstance()->addDsgClient(regInfo);

  return (&retVal);

}

int *
registerdsgclientwithclassifier_1_svc(struct dsgClientClassifierRegInfo *regInfo, struct svc_req * svcReqInfo) {

  retVal = 0;

    // printf("registerdsgclient_1_svc: begin");
#ifdef NOTYET
  switch (regInfo->idType) {

  case 1:

    printf("DSGCC Registrar: registering a Client using well-known MAC addr");
    printf(" %02x:%02x:%02x:%02x:%02x:%02x\n",
      regInfo->clientName[0], regInfo->clientName[1],
      regInfo->clientName[2], regInfo->clientName[3],
      regInfo->clientName[4], regInfo->clientName[5]) ;
    break;

  case 2:
    printf("DSGCC Registrar: registering a Client using CAS ID %d, \n",
      regInfo->clientId);
    break;

  case 3:
    printf("DSGCC Registrar: registering a Client using Application ID %d\n",
      regInfo->clientId);
    break;

  case 4:
    printf("DSGCC Registrar: registering a Client for Broadcast traffic\n");
    break;

  case 5:
    printf("DSGCC Registrar: registering a Client using Application name %s\n",
      regInfo->clientName);
    break;

  case 6:
    printf("DSGCC Registrar: registering a Client using Vendor ID %d\n",
      regInfo->clientId);
    break;

  }

  printf("\t to receive data on UDP Port %d.\n", regInfo->clientPort );

#endif

  retVal = BcmDsgClientRegistrarThread::GetSingletonInstance()->addDsgClientWithClassifier(&regInfo->dsgClient_RegInfo, &regInfo->dsgClassifier);

  return (&retVal);

}

extern "C" {
void
dsgregistrar_1(struct svc_req *rqstp, register SVCXPRT *transp);
}

int 
initializeRPCinterface(void) {
        register SVCXPRT *transp;

        printf("initializeRPCinterface: begin \n");
        pmap_unset (DSGREGISTRAR, DSGREGISTRARVERS);

        printf("initializeRPCinterface: creating UDP transport \n");
        transp = svcudp_create(RPC_ANYSOCK);
        if (transp == NULL) {
                fprintf (stderr, "%s", "cannot create udp service.");
                return(0);
        }
        printf("initializeRPCinterface: registering service (UDP)\n");
        if (!svc_register(transp, DSGREGISTRAR, DSGREGISTRARVERS, dsgregistrar_1, IPPROTO_UDP)) {
                fprintf (stderr, "%s", "unable to register (DSGREGISTRAR, DSGREGISTRARVERS, udp).");
                return(0);
        }

        // printf("initializeRPCinterface: creating TCP transport \n");
        transp = svctcp_create(RPC_ANYSOCK, 0, 0);
        if (transp == NULL) {
                fprintf (stderr, "%s", "cannot create tcp service.");
                return(0);
        }
        // printf("initializeRPCinterface: registering service (TCP)\n");
        if (!svc_register(transp, DSGREGISTRAR, DSGREGISTRARVERS, dsgregistrar_1, IPPROTO_TCP)) {
                fprintf (stderr, "%s", "unable to register (DSGREGISTRAR, DSGREGISTRARVERS, tcp).");
                return(0);
        }

        printf("initializeRPCinterface: running service \n");
        svc_run ();
        fprintf (stderr, "%s", "svc_run returned");
        return(1);
        /* NOTREACHED */
}


