/***************************************************************************
*     (c)2008-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
/*=********************************************
This is one implementation of the BRPC api using sockets.
*********************************************/
#include <stdio.h>
#include <string.h>
#include "cfe_error.h"
#include "lib_queue.h"
#include "net_ebuf.h"
#include "net_api.h"
#include "net_ether.h"
#include "bstd.h"
#include "bkni.h"
#include "brpc_plat.h"
#include "brpc_socket.h"
#include "rmagnum.h"

/* this should be defined in berr.h ? */
#define BERR_RPC_ERROR              10  /* RPC server returns error */
#define	RANDOM_TBASE				0xFFFFFFFF

/* This is assigned to BRPC_Handle->private_data. */
typedef struct BRPC_SocketData {
	int32_t sock_req; /* socket descriptor for sending requests and notification ack*/
	int32_t sock_ntf; /* socket descriptor for waiting for notifications */
	int32_t sock_ack; /* socket descriptor for responses */
	int32_t timeout; /* timeout in ms */
	uint32_t ipaddr; /* server IP address in network byte order */
	uint16_t port_req;	/* server port number in network byte order */
	uint16_t port_ack;	/* server port number in network byte order */
	uint16_t port_ntf_ack; /* port for notification ack */
} BRPC_SocketData;

#define GET_SOCKETDATA(handle) ((BRPC_SocketData*)(handle)->private_data)
#define BYTE_SWAP16(x)	((((x) >> 8) & 0xff) | (((x) << 8) & 0xff00))
BDBG_MODULE(BRPC);

static int32_t random(void)
{
    extern int32_t _getticks(void);		/* return value of CP0 COUNT */
    return RANDOM_TBASE + (uint32_t)_getticks();
}

/* Utility function */
unsigned long SWAP32 (unsigned long val)
{
  return ((val & 0x000000ff) << 24) | ((val & 0x0000ff00) <<  8) | ((val & 0x00ff0000) >>  8) | ((val & 0xff000000) >> 24);
}

/***************************************************************************
Summary:
	Dump the content of a Rmagnum message

Description:
	This function displays the content of a Rmagnum message

Parameters:
    buf --- pointer to message buffer
	len --- length of the buffer

Returns: none

See Also:

****************************************************************************/
static void BRPC_P_DumpRmagnumMsg(uint8_t * buf, uint32_t len)
{
	RmagnumResponseHdr * phdr = (RmagnumResponseHdr *)buf;
	len = len;

	switch(SWAP32(phdr->msgType))
	{
		case kRequest:
			{
			RmagnumRequestMsg * pMsg = (RmagnumRequestMsg * )buf;
			    BDBG_MSG(("REQUEST: version = %08x sessionId = %08x transactionId = %08x msgType = %08x procedureCode = %08x\n",
				SWAP32(pMsg->hdr.version),
				SWAP32(pMsg->hdr.sessionId),
				SWAP32(pMsg->hdr.transactionId),
				SWAP32(pMsg->hdr.msgType),
				SWAP32(pMsg->hdr.procedureCode)));
			break;
			}
		case kResponse:
			{
			RmagnumResponseMsg * pMsg = (RmagnumResponseMsg * )buf;
			    BDBG_MSG(("REPONSE: version = %08x sessionId = %08x transactionId = %08x msgType = %08x returnCode = %08x\n",
				SWAP32(pMsg->hdr.version),
				SWAP32(pMsg->hdr.sessionId),
				SWAP32(pMsg->hdr.transactionId),
				SWAP32(pMsg->hdr.msgType),
				SWAP32(pMsg->hdr.returnCode)));
			break;
			}
		case kNotification:
			{
			RmagnumNotificationMsg * pMsg = (RmagnumNotificationMsg * )buf;
			    BDBG_MSG(("NOTIFICATION: version = %08x sessionId = %08x transactionId = %08x msgType = %08x eventCode = %08x\n",
				SWAP32(pMsg->hdr.version),
				SWAP32(pMsg->hdr.sessionId),
				SWAP32(pMsg->hdr.transactionId),
				SWAP32(pMsg->hdr.msgType),
				SWAP32(pMsg->hdr.eventCode)));
			break;
			}
		case kNotificationAck:
			{
			RmagnumNotificationAckMsg * pMsg = (RmagnumNotificationAckMsg * )buf;
			    BDBG_MSG(("NOTIFICATION_ACK: version = %08x sessionId = %08x transactionId = %08x msgType = %08x eventCode = %08x\n",
				SWAP32(pMsg->hdr.version),
				SWAP32(pMsg->hdr.sessionId),
				SWAP32(pMsg->hdr.transactionId),
				SWAP32(pMsg->hdr.msgType),
				SWAP32(pMsg->hdr.eventCode)));
			break;
			}
		default:
			printf("unkown type!\n");
	}
}

/***************************************************************************
Summary:
	Write data to a given socket

Description:
	This function writes certain amount of data to a given datagram socket

Parameters:
    fd  --- a datagram socket descriptor
    pBuf --- pointer to a buffer
	ip_addr --- the destination ip address in NBO
	port --- destination port number in NBO

Returns: the amount of data writen

See Also:

****************************************************************************/
static int32_t BRPC_P_UdpWrite( int32_t fd, ebuf_t *pBuf, uint32_t ipaddr, uint16_t port)
{
    int32_t result;
	char ip_address[4];
	int i;

	/* connect to remote port first */
    ip_address[0] = ipaddr;
	ip_address[1] = ipaddr>>8;
	ip_address[2] = ipaddr>>16;
	ip_address[3] = ipaddr>>24;

    result = udp_connect(fd, port);
	if(result < 0) return result;

	/* send udp packet */
	result = udp_send(fd,pBuf,&ip_address[0]);

	if(result < 0 )
	{
	   return result;
	}
	else
	{
	   return 0;
	}
}


/***************************************************************************
Summary:
     Request function

Description:

Parameters:
	port --- destination port number in NBO

Returns: the amount of data writen

See Also:

****************************************************************************/
BERR_Code BRPC_P_SocketCallProc(BRPC_Handle handle, unsigned proc_id, const uint32_t *inparams, unsigned inparams_num,
	uint32_t *outparams, unsigned outparams_num, BERR_Code *retVal)
{
	int8_t      buf[RMAGNUM_MAX_MSG_LEGNTH];       /* ptr to dynamic buffer */
	ebuf_t      *ebuf = NULL;
	ebuf_t      *recv_ebuf = NULL;
	RmagnumRequestMsg * preq;
	uint32_t xid;
	uint32_t i;
	BERR_Code ret = BERR_UNKNOWN;
	int32_t msg_len,rc;
    RmagnumResponseMsg * prsp;
    uint16_t cmd;

	/* contruct the REQUEST message */
	xid = random();
    ebuf = udp_alloc();
	if(ebuf == NULL) return -1;
    /* hdr.version */
    ebuf_append_u32_be(ebuf,1);
	/* hdr.sessionId */
    ebuf_append_u32_be(ebuf,0);
	/* hdr.transactionId */
    ebuf_append_u32_be(ebuf,xid);
	/* hdr.msgType */
    ebuf_append_u32_be(ebuf,kRequest);
	/* hdr.procedureCode */
    ebuf_append_u32_be(ebuf,proc_id);

	/* copy the parameters */
	if(inparams)
	{
		for(i = 0; i < inparams_num; i++)
		{
			/* IMPORTANT! 4 byte alignment is assumed!!! */
            ebuf_append_u32_be(ebuf, (*(inparams+i)));
		}
	}
	/* send the message */
	BRPC_P_UdpWrite( GET_SOCKETDATA(handle)->sock_req, ebuf, GET_SOCKETDATA(handle)->ipaddr, GET_SOCKETDATA(handle)->port_req);

	while(1)
	{
	    if(GET_SOCKETDATA(handle)->timeout < 0)
	    {
		   /* need a default timeout value */
           recv_ebuf = udp_recv_with_timeout(GET_SOCKETDATA(handle)->sock_req,10);
      	}
	    else
	    {
           recv_ebuf = udp_recv_with_timeout(GET_SOCKETDATA(handle)->sock_req,(GET_SOCKETDATA(handle)->timeout < 1000) ? 1 : GET_SOCKETDATA(handle)->timeout/1000);
	    }
        if (recv_ebuf != NULL)
		{
			/* get the message from ebuf */
			msg_len = (uint32_t) ebuf_length(recv_ebuf);
			ebuf_get_bytes(recv_ebuf,buf,ebuf_length(recv_ebuf));
            udp_free(recv_ebuf);
			prsp = (RmagnumResponseMsg *)buf;
#if 0
    printf("in brcp_socket\n");
	for (i = 0; i < ebuf_length(ebuf); i++)
	{
		printf("%2x", buf[i]);
		if ((i+1)%4 == 0) printf ("   ");
		if ((i+1)%16 == 0) printf ("\n");
	}
	printf("\n");
#endif

			BRPC_P_DumpRmagnumMsg((uint8_t *) &buf[0], msg_len);

			/* validate the xid in RESPONSE message */
			if (xid == SWAP32(prsp->hdr.transactionId))
		    {
				/* get the return code */
				rc = SWAP32(prsp->hdr.returnCode);
				/* TODO check rc against size_result */
				if(msg_len < (int32_t)(sizeof(RmagnumResponseHdr) + outparams_num*sizeof(uint32_t)))
				{
					BDBG_WRN(("Response message length too small: %d", msg_len));
				}
				/* get results */
				if(outparams)
				{
					for(i = 0; i < outparams_num; i++)
					{
						/* IMPORTANT! 4 byte alignment is assumed!!! */
						*(outparams+i) = SWAP32(*(uint32_t *)(prsp->result + i*4));
					}
				}
				*retVal = (BERR_Code)rc;
				ret = BERR_SUCCESS;
				break;
			}
			continue;
		}
		else
		{
		    printf("recv buff empty\n");
			break;
		}
    } /* end of while */
done:
	return ret;
}

/***************************************************************************
Summary:
	Write data to a given socket

Description:
	This function writes certain amount of data to a given datagram socket

Parameters:
    fd  --- a datagram socket descriptor
    pBuf --- pointer to a buffer
	buf_size --- length of the buffer
	ip_addr --- the destination ip address in NBO
	port --- destination port number in NBO

Returns: the amount of data writen

See Also:

****************************************************************************/

BERR_Code BRPC_CheckNotification_SocketImpl(BRPC_Handle handle, uint32_t *devId, uint32_t *event, int32_t timeout)
{
	uint8_t              buf[RMAGNUM_MAX_MSG_LEGNTH] ;       /* ptr to dynamic buffer */
	ebuf_t               *ebuf = NULL;
	int32_t	           	buflen  = RMAGNUM_MAX_MSG_LEGNTH;       /* length of buffer */
	int32_t rc;
	BERR_Code ret = BERR_UNKNOWN;
	uint32_t ev_buflen;

	int32_t msg_len;
	RmagnumNotificationAckMsg *p_ntf_ack;
	RmagnumNotificationMsg *pntf;

	if( (event == NULL) || (devId == NULL) )
	{
		BDBG_ERR(("Invalid event/id buffer pointer!"));
		return BERR_TRACE(BERR_UNKNOWN);
	}

	/* save event buffer length */
	ev_buflen = sizeof(uint32_t);

	/* 0 is reserved */
	*devId = 0;
	*event = 0;

	/* timeout determines whether is blocking or non-blocking */
	if(timeout < 0)
	    {
 	       ebuf = udp_recv(GET_SOCKETDATA(handle)->sock_ntf);
           /* set default timeout value */
      	}
	 else
	    {
           ebuf = udp_recv_with_timeout(GET_SOCKETDATA(handle)->sock_ntf,8);

	 }

	if(ebuf !=NULL)
	{
		rc = ebuf_length(ebuf);
	    /* get the message from ebuf */
	    ebuf_get_bytes(ebuf,buf,ebuf_length(ebuf));
        /* free received ebuf */
        udp_free(ebuf);
		msg_len = rc;
		p_ntf_ack = (RmagnumNotificationAckMsg *)buf;
		pntf = (RmagnumNotificationMsg*)buf;

		/* dump the message */
		BRPC_P_DumpRmagnumMsg(buf, rc);

		/* process the event */
		*devId = SWAP32(pntf->hdr.eventCode);

		if(msg_len < (int32_t)(sizeof(RmagnumNotificationHdr) + ev_buflen))
		{
         	 printf("Notification message length too small: %d", msg_len);
		}
		else
		{
			*event = SWAP32(*(uint32_t *)(pntf->eventData));
		}
		/* get results */
#if 0
		if(pbuf)
		{
			if( msg_len > (int32_t)(sizeof(RmagnumNotificationHdr)) )
			{
				*pbuflen = (msg_len - (int32_t)(sizeof(RmagnumNotificationHdr)));
			}
			for(i = 0; i < ((*pbuflen)/sizeof(uint32_t)); i++)
			{
				/* IMPORTANT! 4 byte alignment is assumed!!! */
				*(pbuf+i) = SWAP32(*(uint32_t *)(pntf->eventData + i*4));
			}
		}
#endif

		/* acknowledge the notification */
		p_ntf_ack->hdr.msgType = SWAP32(kNotificationAck);

		/* put ack message in to ebuf */
		ebuf = udp_alloc();
		ebuf_append_bytes(ebuf,buf,msg_len);
		BRPC_P_UdpWrite(GET_SOCKETDATA(handle)->sock_ntf, ebuf, GET_SOCKETDATA(handle)->ipaddr, GET_SOCKETDATA(handle)->port_ntf_ack);
		ret = BERR_SUCCESS;
	}
	else
	{
	    BDBG_ERR(("Timeout, no notification response received "));
    }
	return ret;
}

/***************************************************************************
Summary:

Description:

Parameters:

Returns: the amount of data writen

See Also:

****************************************************************************/
BERR_Code BRPC_Open_SocketImpl(BRPC_Handle *handle, const BRPC_OpenSocketImplSettings *openSettings)
{
	BRPC_SocketData *pSockData;
	/* allocate memory for handle */
	*handle = BKNI_Malloc(sizeof(BRPC_Handle));
	if (!(*handle))
		goto err_handle;
	/* allocate memory for socket-specific stuff */
	(*handle)->private_data = (void *)BKNI_Malloc(sizeof(BRPC_SocketData));
	if (!((*handle)->private_data))
		goto err_private_data;

	pSockData = (BRPC_SocketData *)(*handle)->private_data;

	/* create sockets */
	if ((pSockData->sock_req = udp_socket(RMAGNUM_REQUEST_PORT)) < 0)
    {
		BDBG_ERR(("Can not create sock_req!"));
		goto err_sock_req;
    }

	if ((pSockData->sock_ntf = udp_socket(RMAGNUM_NOTIFICATION_PORT)) < 0)
    {
		BDBG_ERR(("Can not create sock_ntf!"));
		goto err_sock_ntf;
    }

	/* bind to the notification port */
    if (udp_bind(pSockData->sock_ntf, RMAGNUM_NOTIFICATION_PORT) < 0)
    {
		BDBG_ERR(("Fail to bind sock_ntf!"));
		goto err_bind;
    }

	/* set the deault timeout value, 0 means non-blocking (polling) */
	pSockData->timeout = openSettings->timeout;

	/* set the server IP address */
    pSockData->ipaddr = SWAP32(openSettings->host);

	/* set the server port number */
	pSockData->port_ntf_ack	= RMAGNUM_NOTIFICATION_ACK_PORT;
	pSockData->port_req	= (openSettings->port_req);
	pSockData->port_ack = (openSettings->port_ack);

	(*handle)->rpc_disabled = openSettings->rpc_disabled;
	/* set the function pointers */
	(*handle)->callProc = BRPC_P_SocketCallProc;
	(*handle)->checkNotification = BRPC_CheckNotification_SocketImpl;

	/* done! */
	return BERR_SUCCESS;
err_bind:
 	udp_close(pSockData->sock_ntf);
err_sock_ntf:
	udp_close(pSockData->sock_req);
err_sock_req:
	BKNI_Free((*handle)->private_data);
err_private_data:
	BKNI_Free(handle);
err_handle:
	/* TODO: return a better error code */
	return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
}

/***************************************************************************
Summary:

Description:

Parameters:

Returns: the amount of data writen

See Also:

****************************************************************************/

void BRPC_Close_SocketImpl(BRPC_Handle handle)
{
	/* close sockets */
	udp_close(GET_SOCKETDATA(handle)->sock_ntf);
	udp_close(GET_SOCKETDATA(handle)->sock_req);
	/* free memory */
	BKNI_Free(GET_SOCKETDATA(handle));
	BKNI_Free(handle);
}
/***************************************************************************
Summary:

Description:

Parameters:

Returns: the amount of data writen

See Also:

****************************************************************************/

/* which socket do you want? */
BERR_Code BRPC_GetSocketDescriptor(BRPC_Handle handle, int *fd)
{
	*fd = GET_SOCKETDATA(handle)->sock_ntf;
	return BERR_SUCCESS;
}


/***************************************************************************
Summary:

Description:

Parameters:

Returns: the amount of data writen

See Also:

****************************************************************************/

void BRPC_GetDefaultOpenSocketImplSettings(BRPC_OpenSocketImplSettings *socketSettings)
{
	socketSettings->host = RMAGNUM_SERVER_IP_ADDR; /* whatever we agree to */
	socketSettings->port_req = RMAGNUM_REQUEST_PORT;
	socketSettings->port_ack = RMAGNUM_NOTIFICATION_ACK_PORT;
	socketSettings->timeout = 5000; /*5 second*/
	socketSettings->rpc_disabled = false;
}

