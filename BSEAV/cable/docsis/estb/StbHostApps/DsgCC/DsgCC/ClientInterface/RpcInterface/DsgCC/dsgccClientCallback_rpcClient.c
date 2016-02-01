
#include "dsgccClientCallback_rpc.h"

/* By default we use local loopback for callback RPCs */
#define  CALLBACK_RPC_SERV_ADDR                "127.0.0.1"

int
rpc_dsgccClientNotification(int clientProgNum, dsgccClientNotification *pDsgccClientNotification)
{
	int cbRes; // callback result	

        if (clientProgNum <= 0) {
          printf("dsgcc_ClientNotification: bad clientProgNum %d\n", clientProgNum);
          return (0);
        }

        cbRes = callrpc(CALLBACK_RPC_SERV_ADDR, clientProgNum, DSGCCSTATUSCALLBACKVERS,
                        DSGCCCLIENTNOTIFICATION, (xdrproc_t) xdr_dsgccClientNotification, (char*)pDsgccClientNotification, (xdrproc_t) xdr_void, 0);

	if (cbRes == RPC_SUCCESS) 
	  return (1);

	return (0);
}

