/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "dsgccClientCallback_rpc.h"

bool_t
xdr_dsgccClientNotification (XDR *xdrs, dsgccClientNotification *objp)
{

	 if (!xdr_int (xdrs, &objp->eventType))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->eventValue))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->clientPort))
		 return FALSE;
	return TRUE;
}
