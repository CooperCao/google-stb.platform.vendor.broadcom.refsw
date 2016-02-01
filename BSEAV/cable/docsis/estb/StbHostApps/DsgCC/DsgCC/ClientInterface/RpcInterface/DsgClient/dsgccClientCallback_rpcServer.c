#include <stdio.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "unp.h"

#include "dsgcc_client_api.h"

#include "dsgccClientCallback_rpc.h"
#include "dsgccClientCallback_rpcServer.h"

static int retVal = 0;		/* for the RPC return value */
int clientProgNum, callbackSockFd;

char* TunnelStatusTypeName( unsigned int value );
extern void dsgcc_ClientNotification(struct dsgccClientNotification *clientNotification);


/* These are the RPC Callback service handlers */
int *
dsgccclientnotification_1_svc(struct dsgccClientNotification *clientNotification, struct svc_req * svcReqInfo) {
 
	unsigned int value = 0;
  if (clientNotification == NULL) {
    printf("Client got bad clientNotification address.\n");
    retVal = 1;
  } 
  else
  {
		switch(clientNotification->eventType)
		{
			case DSGCC_TUNNEL_OPEN:
			{
				printf("NOTIFICATION from DSG-CC -> %s \n", TunnelStatusTypeName(clientNotification->eventType)); 
				
				value = 1;
				tunnelpresent_1_svc(&value, NULL);
			}
			break;

			case DSGCC_TUNNEL_NOT_AVAILABLE:
			{
				printf("NOTIFICATION from DSG-CC -> %s \n", TunnelStatusTypeName(clientNotification->eventType)); 
				value = 0;
				tunnelpresent_1_svc(&value, NULL);
			}
			break;

			case DSGCC_TWOWAYMODE_CHANGE:
			{
				printf("NOTIFICATION from DSG-CC -> %d  value = %d\n", clientNotification->eventType, clientNotification->eventValue);
				twowaymode_1_svc(&clientNotification->eventValue, NULL);
			}
			break;

			case DSGCC_ADVANCEMODE_CHANGE:
			{
				printf("NOTIFICATION from DSG-CC -> %d \n", clientNotification->eventType);
				advancedmode_1_svc(&clientNotification->eventValue, NULL);
			}
			break;

			default:
			{
				printf("NOTIFICATION code not recognized!\n");
			}
		}
	  
	  /* Send it to the client */
	  dsgcc_ClientNotification(clientNotification);
  }

  return (&retVal);

} 


/* These RPC call handlers below are replaced with dsgccclientnotification_1_svc() 
   Function protoypes are left unchanged to make this transition make easier. */

int *
twowaymode_1_svc(int *mode, struct svc_req * svcReqInfo) {
 
  if (mode == NULL) {
    printf("Client got bad mode address.\n");
    retVal = 1;
  } else if (*mode == 0) {
    printf("Client got two-way mode off indication.\n");
    retVal = 1;
  } else if (*mode == 1) {
    printf("Client got two-way mode on indication.\n");
    retVal = 1;
  } else {
    printf("Client got invalid two-way mode indication %d\n", mode);
    retVal = 0;
  }

  return (&retVal);

} 

int *
advancedmode_1_svc(int *mode, struct svc_req * svcReqInfo) {
 
  if (mode == NULL) {
    printf("Client got bad mode address.\n");
    retVal = 1;
  } else if (*mode == 0) {
    printf("Client got dsg disabled indication.\n");
    retVal = 1;
  } else if (*mode == 1) {
    printf("Client got dsg basic mode indication.\n");
    retVal = 1;
  } else if (*mode == 2) {
    printf("Client got dsg advanced mode indication.\n");
    retVal = 1;
  } else {
    printf("Client got invalid dsg mode indication %d\n", *mode);
    retVal = 0;
  }

  return (&retVal);

} 

int *
tunnelpresent_1_svc(int *mode, struct svc_req * svcReqInfo) {
 
	printf("tunnelpresent_1_svc\n");
  if (mode == NULL) {
    printf("Client got bad mode address.\n");
    retVal = 1;
  } else if (*mode == 0) {
    printf("Client got tunnel absent indication.\n");
    retVal = 1;
  } else if (*mode == 1) {
    printf("Client got tunnel present indication.\n");
    retVal = 1;
  } else {
    printf("Client got invalid tunnel presence indication %d\n", mode);
    retVal = 0;
  }

  return (&retVal);

} 


int
getTransientProgramNumber(int *sockp)
{
      static int prognum = 0x40000000;
      int s, len;
      struct sockaddr_in addr;

      printf ("getTransientProgramNumber: starting\n");
      if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return (0);
      }
      *sockp = s;
      printf ("getTransientProgramNumber: got sock %d\n", s);

      addr.sin_addr.s_addr = 0;
      addr.sin_family = AF_INET;
      addr.sin_port = 0;
      len = sizeof(addr);
      /*
       * may be already bound, so don't check for error
       */
      bind(s, (SA *) &addr, len);
      printf ("getTransientProgramNumber: bound\n");
      if (getsockname(s, (SA *) &addr, &len)< 0) {
           perror("getsockname");
           return (0);
      }
      printf ("getTransientProgramNumber: getsockname OK\n");
      while (!pmap_set(prognum++, 1, IPPROTO_UDP,
           ntohs(addr.sin_port))) continue;
      printf ("getTransientProgramNumber: pmap_set OK\n");
      return (prognum-1);
}



/* everything below this line will go away when we integrate
 * with the real client controller
 */

void
dsgccstatuscallback_1(struct svc_req *rqstp, register SVCXPRT *transp);

int
dsgClientCallbackInit()
{

        printf ("dsgClientCallbackInit: starting\n");
        /* get getTransientProgramNumber to get the number and set up the socket*/
        callbackSockFd = RPC_ANYSOCK;
        clientProgNum = getTransientProgramNumber(&callbackSockFd);
        if (clientProgNum == 0) {
                printf ("dsgClientCallbackInit: cannot get client program number.\n");
                exit(1);
        }
        printf ("dsgClientCallbackInit: got client program number %d.\n", clientProgNum);

        return (clientProgNum);
}

int
dsgClientCallbackSvcRun()
{
        register SVCXPRT *transp;

        /* create the RPC callback service*/
        transp = svcudp_create(callbackSockFd);
        if (transp == NULL) {
                printf ("dsgClientCallbackSvcRun: cannot create callback service.\n");
                exit(1);
        }

        /* register the RPC callback service - proto is 0, getTransientProgramNumber registers*/
        if (!svc_register(transp, clientProgNum, 1, dsgccstatuscallback_1, 0)) {
                printf ("dsgClientCallbackSvcRun: unable to register callback service.\n");
                exit(1);
        }
        
        printf ("dsgClientCallbackSvcRun: calling svc_run() \n");
        svc_run ();
        printf ("dsgClientCallbackSvcRun: svc_run returned callback\n");
        exit (1);
        /* NOTREACHED */
}

char* TunnelStatusTypeName( unsigned int value ) 
{
	char* names[] =
	{
		"",
		"TUNNEL REQUEST PENDING",	/* = 1 */
		"TUNNEL OPEN",              /* = 2 */
		"TUNNEL NOT AVAILABLE",		/* = 3 */
		"TUNNEL PENDING"            /* = 4 */
	};

	return names[ value ];
}

