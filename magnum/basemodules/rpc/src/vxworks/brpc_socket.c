/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *********************************************************************/

/*=********************************************
This is one implementation of the BRPC api using sockets.
*********************************************/
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "brpc_plat.h"
#include "brpc_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include "brpc_rmagnum.h"
#include "blst_slist.h"

#include <sockLib.h>

typedef struct BRPC_ClntData {
	BLST_S_ENTRY(BRPC_ClntData) link; /* link to the next node */
	int32_t sock_req; /* socket descriptor for sending requests and notification acknowledgements */
	uint8_t *sock_req_buf; /* buffer for sending requests and notification acknowledgements */
	uint32_t dynamic; /* flag to indicate being dynamic or not */
} BRPC_ClntData;

/* This is assigned to BRPC_Handle->private_data. */
typedef struct BRPC_SocketData {
	BLST_S_HEAD(BRPC_ClntData_LinK, BRPC_ClntData) free_cd_list;
	uint32_t free_cnt;
	BKNI_MutexHandle lock; /* mutex for protecting the fifo */
	int32_t sock_ntf; /* socket descriptor for waiting for notifications */
	int32_t timeout; /* timeout in ms */
	uint32_t ipaddr; /* server IP address in network byte order */
	uint16_t port_req;	/* server port number in network byte order */
	uint16_t port_ack;	/* server port number in network byte order */
	uint8_t *sock_ntf_buf; /* buffer for waiting for notifications */
} BRPC_SocketData;

#define GET_SOCKETDATA(handle) ((BRPC_SocketData*)(handle)->private_data)

BDBG_MODULE(BRPC_SOCKET);

#undef RPC_SOCKET_DEBUG
#ifdef RPC_SOCKET_DEBUG
#ifdef BDBG_MSG
#undef BDBG_MSG
#define BDBG_MSG BDBG_WRN
#endif
#endif

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
#if BDBG_DEBUG_BUILD
	BRPC_RmagnumRequestHdr * phdr = (BRPC_RmagnumRequestHdr *)buf;
	len = len;

	switch(ntohl(phdr->msgType))
	{
		case kRequest:
			{
			BRPC_RmagnumRequestMsg * pMsg = (BRPC_RmagnumRequestMsg * )buf;
			BDBG_MSG(("REQUEST: version = %08x sessionId = %08x transactionId = %08x msgType = %08x procedureCode = %08x\n",
				ntohl(pMsg->hdr.version),
				ntohl(pMsg->hdr.sessionId),
				ntohl(pMsg->hdr.transactionId),
				ntohl(pMsg->hdr.msgType),
				ntohl(pMsg->hdr.procedureCode)));
			break;
			}
		case kResponse:
			{
			BRPC_RmagnumResponseMsg * pMsg = (BRPC_RmagnumResponseMsg * )buf;
			BDBG_MSG(("REPONSE: version = %08x sessionId = %08x transactionId = %08x msgType = %08x returnCode = %08x\n",
				ntohl(pMsg->hdr.version),
				ntohl(pMsg->hdr.sessionId),
				ntohl(pMsg->hdr.transactionId),
				ntohl(pMsg->hdr.msgType),
				ntohl(pMsg->hdr.returnCode)));
			break;
			}
		case kNotification:
			{
			BRPC_RmagnumNotificationMsg * pMsg = (BRPC_RmagnumNotificationMsg * )buf;
			BDBG_MSG(("NOTIFICATION: version = %08x sessionId = %08x transactionId = %08x msgType = %08x eventCode = %08x\n",
				ntohl(pMsg->hdr.version),
				ntohl(pMsg->hdr.sessionId),
				ntohl(pMsg->hdr.transactionId),
				ntohl(pMsg->hdr.msgType),
				ntohl(pMsg->hdr.eventCode)));
			break;
			}
		case kNotificationAck:
			{
			BRPC_RmagnumNotificationAckMsg * pMsg = (BRPC_RmagnumNotificationAckMsg * )buf;
			BDBG_MSG(("NOTIFICATION_ACK: version = %08x sessionId = %08x transactionId = %08x msgType = %08x eventCode = %08x\n",
				ntohl(pMsg->hdr.version),
				ntohl(pMsg->hdr.sessionId),
				ntohl(pMsg->hdr.transactionId),
				ntohl(pMsg->hdr.msgType),
				ntohl(pMsg->hdr.eventCode)));
			break;
			}
		default:
			BDBG_MSG(("unkown type!\n"));
	}
#else
	BSTD_UNUSED(buf);
	BSTD_UNUSED(len);
#endif
}

/***************************************************************************
Summary:
	Read data from a given socket with a timeout

Description:
	This function reads a UDP packet from a socket and optionally times out
	after certain period of time. The socket needs to be bound to a port before
	this function is called.

Parameters:
    fd  --- a datagram socket descriptor
    pBuf --- pointer to a buffer
	buf_size --- length of the buffer
	timeout --- timeout value in miliseconds, if timeout is less than zero, then wait forever

Returns: the amount of data read or -1 in case of error.

See Also:

****************************************************************************/
static int32_t BRPC_P_UdpReadTimeout( int32_t fd, uint8_t *pBuf, uint32_t buf_size, int32_t timeoutMS)
{
	int32_t maxfdp1;
	fd_set rfds;
	int32_t retcode;
    register int32_t cnt = -1;
	struct sockaddr_in frominet;
	int32_t                inetsize;
	struct timeval tv;
#ifdef RPC_SOCKET_DEBUG
	uint32_t i;
#endif

	maxfdp1 = fd + 1;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	if(timeoutMS < 0)
	{
		retcode = select(maxfdp1, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);
	}
	else
	{
		tv.tv_sec = timeoutMS / 1000;
		tv.tv_usec = (timeoutMS - (1000 * tv.tv_sec)) * 1000;
		retcode = select(maxfdp1, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)&tv);
	}

	if (retcode < 0)
	{
		BDBG_MSG(("select(): %s\n", strerror(errno)));
	}
	else if (retcode == 0)
	{
		if(timeoutMS != 0)
		{
			BDBG_MSG(("select(): timeout!\n"));
		}
	}
	else
	{
		if(FD_ISSET(fd, &rfds))
		{
			inetsize = sizeof(frominet);
			cnt = recvfrom(fd, pBuf, buf_size, 0, (struct sockaddr*)&frominet, (socklen_t*) &inetsize);
			if( cnt <= 0 )
			{
				BDBG_MSG(("recvfrom error: %s\n", strerror(errno)));
			}
#ifdef RPC_SOCKET_DEBUG
			else
			{
				BDBG_MSG(("%d bytes read!\n", cnt));
				for(i = 0; i < (int32_t)cnt; i++)
				{
					BDBG_MSG(("%02x", *(pBuf+i)));
					if( i % 4 == 3)
						BDBG_MSG((" "));
				}
				BDBG_MSG(("\n"));
			}
#endif
		}
		retcode = cnt;
	}

	return retcode;
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
static int32_t BRPC_P_UdpWrite( int32_t fd, uint8_t *pBuf, uint32_t tpkt_size, uint32_t ipaddr, uint16_t port)
{
	struct sockaddr_in sin_dst;
    int32_t cnt;
#ifdef RPC_SOCKET_DEBUG
	uint32_t i;
#endif

	/* configure the destionation addr */
	bzero((int8_t*)&sin_dst, sizeof(sin_dst));
    sin_dst.sin_family = AF_INET;
    sin_dst.sin_port = port;
	sin_dst.sin_addr.s_addr = ipaddr;

#ifdef RPC_SOCKET_DEBUG
    printf("udp_write: sending %d bytes ...\n", tpkt_size);
    for(i = 0; i < tpkt_size; i++)
    {
            printf("%02x", *(pBuf+i));
            if( i % 4 == 3)
                    printf(" ");
    }
    printf("\n");
#endif

	/* send the packet */
    cnt=sendto(fd, pBuf, tpkt_size, 0, (struct sockaddr*)&sin_dst, sizeof(sin_dst));
	if( cnt != (int32_t)tpkt_size )
	{
		BDBG_MSG(("\nError: sendto returns %d  - %s\n", cnt, strerror(errno)));
		return cnt;
	}
	return 0;
}

/***************************************************************************
Summary:
	De-initialize the socket/buffer pool

Description:
	This function closes sockets and frees associated buffers.

Parameters:
    pSockData  --- a pointer to the socket data

Returns: none

See Also:

****************************************************************************/
static void BRPC_P_DeInitSocketPool(BRPC_SocketData *pSockData)
{
	struct BRPC_ClntData *clnt_data;

	/* acquire the lock first */
	BKNI_AcquireMutex(pSockData->lock);

	/* work on the free list */
	while(!BLST_S_EMPTY(&pSockData->free_cd_list))
	{
		clnt_data=BLST_S_FIRST(&pSockData->free_cd_list);
		BLST_S_REMOVE_HEAD(&pSockData->free_cd_list, link);
		pSockData->free_cnt--;

		close(clnt_data->sock_req);
		BKNI_Free(clnt_data->sock_req_buf);
		BKNI_Free(clnt_data);
	}

	/* release the lock */
	BKNI_ReleaseMutex(pSockData->lock);
}

/***************************************************************************
Summary:
	Initialize the socket/buffer pool

Description:
	This function opens sockets, allocates buffers and bind sockets to certain
	ports.

Parameters:
    pSockData  --- a pointer to the socket data

Returns: 0 --- success
		 1 --- failure

See Also:

****************************************************************************/
static int32_t BRPC_P_InitSocketPool(BRPC_SocketData *pSockData)
{
	struct BRPC_ClntData *clnt_data;
	struct sockaddr_in sin_ctl;
	unsigned short int port;
	extern int errno;
	unsigned int j;
	int noblock = 1;
	int32_t rc;

	for(j = 0; j < BRPC_RMAGNUM_CLNT_NSOCKETS; j++)
	{
		/* allocate clnt data first */
		clnt_data = BKNI_Malloc(sizeof(*clnt_data));
		if (!clnt_data)
		{
			BDBG_ERR(("Can not allocate clnt_data!"));
			goto err_clnt_data;
		}

		/* allocate a socket for the current thread */
		if ((clnt_data->sock_req = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			BDBG_ERR(("Can not create sock_req!"));
			goto err_sock_req;
		}

		/* pre malloc socket buffer */
		if( (clnt_data->sock_req_buf = (uint8_t *)BKNI_Malloc(BRPC_RMAGNUM_MAX_MSG_LEGNTH)) == (uint8_t *)NULL)
		{
			BDBG_ERR(("can not allocate memory!"));
			goto err_req_buf;
		}

		/* configure the address and port */
		bzero((int8_t*)&sin_ctl, sizeof(sin_ctl));
		sin_ctl.sin_family = AF_INET;
		sin_ctl.sin_addr.s_addr = htonl(INADDR_ANY);

		/* our best guess for the initial port */
		port = BRPC_RMAGNUM_CLNT_STARTPORT + pSockData->free_cnt;

		sin_ctl.sin_port = htons(port);

		BDBG_MSG((" trying to bind to %x for thread %x", port, getpid()));

		/* bind to the request port */
		if ((rc = bind(clnt_data->sock_req, (struct sockaddr *)&sin_ctl, sizeof(sin_ctl))) < 0)
		{
			BDBG_ERR(("BRPC_P_InitSocketPool: binding error errno = %d", errno));
			goto err_bind;
		}
		else
		{
			/* success */
			BDBG_MSG((" UDP sock_ack ipany port %x sock_req %x ", port, clnt_data->sock_req));

			/* set to non-blocking */
			ioctl(clnt_data->sock_req, FIONBIO, (char *) &noblock);

			/* set the dynamic flag */
			clnt_data->dynamic = 0;

			/* add the node to the list */
			BLST_S_INSERT_HEAD(&(pSockData->free_cd_list), clnt_data, link);
			pSockData->free_cnt++;
			BDBG_MSG((" New client data for %08lx: fd = %d buf = 0x%08lx free_cnt = %d", getpid(), clnt_data->sock_req, clnt_data->sock_req_buf, pSockData->free_cnt));
		}
	}
	return 0;

err_bind:
	BKNI_Free(clnt_data->sock_req_buf);
err_req_buf:
	close(clnt_data->sock_req);
err_sock_req:
	BKNI_Free(clnt_data);
err_clnt_data:
	return 1;
}

/***************************************************************************
Summary:
	Get socket/buffer for a caller

Description:
	This function first attempts to allocate a socket/buffer pair from the local pool.
	If no one is available, it allocates a socket and a buffer from the system, and
	bind the socket to a certain port.

Parameters:
    pSockData  --- a pointer to the socket data

Returns: a pointer to client data (a socket/buffer pair)

See Also:

****************************************************************************/
static inline BRPC_ClntData* BRPC_P_GetClntData(BRPC_SocketData *pSockData)
{
	struct BRPC_ClntData *clnt_data;
	struct sockaddr_in sin_ctl;
	unsigned short int port;
	extern int errno;
	unsigned int i;
	int noblock = 1;
	int rc;

	/* try to locate an existing one */
	BKNI_AcquireMutex(pSockData->lock);
	if(!BLST_S_EMPTY(&(pSockData->free_cd_list)))
	{
		clnt_data=BLST_S_FIRST(&(pSockData->free_cd_list));
		BLST_S_REMOVE_HEAD(&(pSockData->free_cd_list), link);
		pSockData->free_cnt--;

		BDBG_MSG((" Got client data for %08lx: fd = %x buf = %08lx", getpid(), clnt_data->sock_req, clnt_data->sock_req_buf));
		BDBG_MSG((" free_cnt= %d", pSockData->free_cnt));
		BKNI_ReleaseMutex(pSockData->lock);
		return clnt_data;
	}
	BKNI_ReleaseMutex(pSockData->lock);

	/* no match found, allocate a new one */
	clnt_data = BKNI_Malloc(sizeof(*clnt_data));
	if (!clnt_data)
	{
		return NULL;
	}

	/* allocate a socket for the current thread */
	if ((clnt_data->sock_req = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
		BDBG_ERR(("Can not create sock_req!"));
		goto err_sock_req;
    }

	/* pre malloc socket buffer */
    if( (clnt_data->sock_req_buf = (uint8_t *)BKNI_Malloc(BRPC_RMAGNUM_MAX_MSG_LEGNTH)) == (uint8_t *)NULL)
	{
		BDBG_ERR(("can not allocate memory!"));
		goto err_req_buf;
	}

	/* configure the address and port */
	bzero((int8_t*)&sin_ctl, sizeof(sin_ctl));
	sin_ctl.sin_family = AF_INET;
    sin_ctl.sin_addr.s_addr = htonl(INADDR_ANY);

	/* our best guess for the initial port */
	port = BRPC_RMAGNUM_CLNT_DYNAMIC_STARTPORT;

	for (i = 0; i < BRPC_RMAGNUM_CLNT_NPORTS; i++)
	{
		sin_ctl.sin_port = htons(port);

		BDBG_MSG((" trying to bind to %x for thread %x", port, getpid()));

		/* bind to the request port */
		if ((rc = bind(clnt_data->sock_req, (struct sockaddr *)&sin_ctl, sizeof(sin_ctl))) < 0)
		{
			if(errno == EADDRINUSE)
			{
				port++;
				if (port > (BRPC_RMAGNUM_CLNT_DYNAMIC_STARTPORT + BRPC_RMAGNUM_CLNT_NPORTS - 1))
				{
					port = BRPC_RMAGNUM_CLNT_DYNAMIC_STARTPORT;
				}
				continue;
			}
			else
			{
				BDBG_ERR(("BRPC_P_InitSocketPool: fatal errno = %d", errno));
				goto err_bind;
			}
		}
		else
		{
			/* success */
			BDBG_MSG((" UDP sock_ack ipany port %x sock_req %x ", port, clnt_data->sock_req));

			/* set to non-blocking */
			ioctl(clnt_data->sock_req, FIONBIO, (char *) &noblock);

			/* set the busy flag */
			clnt_data->dynamic = 1;

			/* add the node to the list */
			BDBG_MSG((" New client data for %08lx: fd = %d buf = 0x%08lx", getpid(), clnt_data->sock_req, clnt_data->sock_req_buf));

			return clnt_data;
		}
	}

err_bind:
	BDBG_ERR(("fail to bind sock_req! errno = %d", errno));
	BKNI_Free(clnt_data->sock_req_buf);
err_req_buf:
	close(clnt_data->sock_req);
err_sock_req:
	BKNI_Free(clnt_data);

	return NULL;
}

/***************************************************************************
Summary:
	Release a socket/buffer pair

Description:
	This function checks a flag to determine whether the socket/buf pair should go
	back to the local pool or system. Then it releases it accordingly.

Parameters:
    pSockData  --- a pointer to the socket data
	clnt_data --- a pointer to the client data

Returns: none

See Also:

****************************************************************************/
static inline void BRPC_P_ReleaseClntData(BRPC_SocketData *pSockData, BRPC_ClntData *clnt_data)
{
	if(clnt_data->dynamic == 0)
	{
		BKNI_AcquireMutex(pSockData->lock);
		BLST_S_INSERT_HEAD(&(pSockData->free_cd_list), clnt_data, link);
		pSockData->free_cnt++;
		BKNI_ReleaseMutex(pSockData->lock);
	}
	else /* dynamic */
	{
		BDBG_MSG((" ReleaseClntData: releasing dynamic clnt data..."));
		BKNI_Free(clnt_data->sock_req_buf);
		close(clnt_data->sock_req);
		BKNI_Free(clnt_data);
	}
	BDBG_MSG((" ReleaseClntData: free_cnt= %d", pSockData->free_cnt));
}

BERR_Code BRPC_P_SocketCallProc_SocketImpl(BRPC_Handle handle, unsigned proc_id, const uint32_t *inparams, unsigned inparams_num,
	uint32_t *outparams, unsigned outparams_num, BERR_Code *retVal)
{
	uint8_t     *buf;       /* ptr to dynamic buffer */
	int32_t		buflen  = BRPC_RMAGNUM_MAX_MSG_LEGNTH;       /* length of buffer */
	int32_t		rc;
	BRPC_RmagnumRequestMsg * preq;
	uint32_t xid;
	uint32_t i;
	BERR_Code ret = BERR_UNKNOWN;
	BRPC_ClntData *cdata;

	BDBG_MSG((" call RPC "));

	/* get the client data */
	cdata = BRPC_P_GetClntData(GET_SOCKETDATA(handle));
	if(cdata == NULL)
	{
		return ret;
	}
	buf = cdata->sock_req_buf;

	/* contruct the REQUEST message */
	preq = (BRPC_RmagnumRequestMsg *)buf;
	preq->hdr.version = htonl(1);
	preq->hdr.sessionId = 0;
	/* get a random number for xid */
	xid = random();
	preq->hdr.transactionId = htonl(xid);
	preq->hdr.msgType = htonl(kRequest);
	preq->hdr.procedureCode = htonl(proc_id);

	buflen = sizeof(BRPC_RmagnumRequestHdr);

	/* endian conversion, byte swapping*/
	/* copy the parameters */
	if(inparams)
	{
		for(i = 0; i < inparams_num; i++)
		{
			/* IMPORTANT! 4 byte alignment is assumed!!! */
			*(uint32_t *)(buf+buflen) = htonl(*(inparams+i));
			buflen += 4;
		}
	}

	BDBG_MSG((" UDP write: req %d", cdata->sock_req));

	/* send the message */
	BRPC_P_UdpWrite( cdata->sock_req, buf, buflen, GET_SOCKETDATA(handle)->ipaddr, GET_SOCKETDATA(handle)->port_req);

	while(1)
	{
		memset(buf, 0, BRPC_RMAGNUM_MAX_MSG_LEGNTH);
		BDBG_MSG((" UDP read: ack %d", cdata->sock_req));
		/* wait for reply */
		rc = BRPC_P_UdpReadTimeout( cdata->sock_req, buf, BRPC_RMAGNUM_MAX_MSG_LEGNTH, GET_SOCKETDATA(handle)->timeout);
		if(rc > 0)
		{
			int32_t msg_len;
			BRPC_RmagnumResponseMsg *prsp = (BRPC_RmagnumResponseMsg *)buf;
			msg_len = rc;
			BRPC_P_DumpRmagnumMsg(buf, rc);
			/* validate the xid in RESPONSE message */
			if (xid == ntohl(prsp->hdr.transactionId))
		    {
				BDBG_MSG(("(%08lx)xid matches in RESPONSE\n", getpid()));
/*
				BDBG_MSG(("rc = %d msg_len = %d outparams_num = %d outparams = %08x", rc, msg_len, outparams_num, (uint32_t)outparams));
*/
				/* get the return code */
				rc = ntohl(prsp->hdr.returnCode);
				/* TODO check rc against size_result */
				if(msg_len < (int32_t)(sizeof(BRPC_RmagnumResponseHdr) + outparams_num*sizeof(uint32_t)))
				{
					BDBG_WRN(("Response message length too small: %d", msg_len));
				}
				/* get results */
				if(outparams)
				{
					for(i = 0; i < outparams_num; i++)
					{
						/* IMPORTANT! 4 byte alignment is assumed!!! */
						*(outparams+i) = ntohl(*(uint32_t *)(prsp->result + i*4));
					}
				}
/*				memcpy((void *)result, prsp->result, size_result);*/
/*				BDBG_MSG(("rc = %d\n", rc); */
				*retVal = (BERR_Code)rc;
				ret = BERR_SUCCESS;
				break;
			}
			continue;
		}
		else
		{
			break;
		}
	}

	/* release the client data */
	BRPC_P_ReleaseClntData(GET_SOCKETDATA(handle), cdata);

	return ret;
}

BERR_Code BRPC_P_CheckNotification_SocketImpl(BRPC_Handle handle, uint32_t *devId, uint32_t *event, int32_t timeout)
{
	uint8_t              *buf;       /* ptr to dynamic buffer */
	int32_t		buflen  = BRPC_RMAGNUM_MAX_MSG_LEGNTH;       /* length of buffer */
	int32_t rc;
	BERR_Code ret = BRPC_ERR_NOTIFICATION;
	uint32_t ev_buflen;

	int32_t msg_len;
	BRPC_RmagnumNotificationAckMsg *p_ntf_ack;
	BRPC_RmagnumNotificationMsg *pntf;

	if( (event == NULL) || (devId == NULL) )
	{
		BDBG_ERR(("Invalid event/id buffer pointer!"));
		return BERR_TRACE(BRPC_ERR_NOTIFICATION);
	}

	/* save event buffer length */
	ev_buflen = sizeof(uint32_t);

	/* use pre-allocated memory */
	buf = GET_SOCKETDATA(handle)->sock_ntf_buf;

	/* 0 is reserved */
	*devId = 0;
	*event = 0;

	/* timeout determines whether is blocking or non-blocking */
	rc = BRPC_P_UdpReadTimeout(GET_SOCKETDATA(handle)->sock_ntf, buf, buflen, timeout);
	if(rc > 0)
	{
		msg_len = rc;
		p_ntf_ack = (BRPC_RmagnumNotificationAckMsg *)buf;
		pntf = (BRPC_RmagnumNotificationMsg*)buf;

		/* 325x may send the same notification multiple times with same transaction ID*/
		/* host only needs to send one acknowledgement, otherwise there will be mismatch*/
		if (handle->prev_transactionId != p_ntf_ack->hdr.transactionId)
		{
		/* dump the message */
		BRPC_P_DumpRmagnumMsg(buf, rc);

		/* process the event */
		*devId = ntohl(pntf->hdr.eventCode);

		if(msg_len < (int32_t)(sizeof(BRPC_RmagnumNotificationHdr) + ev_buflen) )
		{
			BDBG_WRN(("Notification message length too small: %d", msg_len));
		}
		else
		{
			*event = ntohl(*(uint32_t *)(pntf->eventData));
		}

		/* acknowledge the notification */
		p_ntf_ack->hdr.msgType = htonl(kNotificationAck);
		BRPC_P_UdpWrite(GET_SOCKETDATA(handle)->sock_ntf, buf, sizeof(BRPC_RmagnumNotificationAckMsg), GET_SOCKETDATA(handle)->ipaddr, GET_SOCKETDATA(handle)->port_ack);
		ret = BERR_SUCCESS;
		}
		else
		{
			BDBG_WRN((" duplicated notification from 325x. Ignored!"));
		}
		handle->prev_transactionId = p_ntf_ack->hdr.transactionId;

	}

	return ret;
}

BERR_Code BRPC_Open_SocketImpl(BRPC_Handle *handle, const BRPC_OpenSocketImplSettings *openSettings)
{
	struct sockaddr_in sin_ctl;
	BRPC_SocketData *pSockData;
	int32_t buflen  = BRPC_RMAGNUM_MAX_MSG_LEGNTH;

	/* allocate memory for handle */
	*handle = BKNI_Malloc(sizeof(struct BRPC_P_Handle));
	if (!(*handle))
		goto err_handle;

	/* allocate memory for socket-specific stuff */
	(*handle)->private_data = (void *)BKNI_Malloc(sizeof(BRPC_SocketData));
		if (!((*handle)->private_data))
			goto err_private_data;

	pSockData = (BRPC_SocketData *)(*handle)->private_data;

	/* create a mutex to protect the fifo */
	if (BKNI_CreateMutex(&pSockData->lock)) {
		BDBG_ERR(("Can not create mutex!"));
		goto err_mutex;
	}

	/* create a socket for notification */
	if ((pSockData->sock_ntf = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
		BDBG_ERR(("Can not create sock_ntf!"));
		goto err_sock_ntf;
    }

	/* configure the address and port */
	bzero((int8_t*)&sin_ctl, sizeof(sin_ctl));
	sin_ctl.sin_family = AF_INET;
    sin_ctl.sin_addr.s_addr = htonl(INADDR_ANY);
    sin_ctl.sin_port = htons(BRPC_RMAGNUM_NOTIFICATION_PORT);

	/* bind to the notification port */
    if (bind(pSockData->sock_ntf, (struct sockaddr *)&sin_ctl, sizeof(sin_ctl)) < 0)
    {
		BDBG_ERR(("Fail to bind sock_ntf!"));
		goto err_bind;
    }

	/* set the deault timeout value, 0 means non-blocking (polling) */
	pSockData->timeout = openSettings->timeout;

	/* set the server IP address */
    pSockData->ipaddr = htonl(openSettings->host);

	/* set the server port number */
	pSockData->port_req = htons(openSettings->port_req);
	pSockData->port_ack = htons(openSettings->port_ack);

	(*handle)->rpc_disabled = openSettings->rpc_disabled;
	/* set the function pointers */
	(*handle)->callProc = BRPC_P_SocketCallProc_SocketImpl;
	(*handle)->checkNotification = BRPC_P_CheckNotification_SocketImpl;
	(*handle)->prev_transactionId = 0xffffffff;

	/* allocate memory */
	if( (pSockData->sock_ntf_buf = (uint8_t *)BKNI_Malloc(buflen)) == (uint8_t *)NULL)
	{
		BDBG_ERR(("can not allocate memory!"));
		goto err_ntf_buf;
	}

	/* initialize the head of the client data list */
	BLST_S_INIT(&pSockData->free_cd_list);
	pSockData->free_cnt = 0;

	/* initialize socket/memory pool */
	if(BRPC_P_InitSocketPool(pSockData))
	{
		goto err_sock_pool;
	}

	/* done! */
	return BERR_SUCCESS;

err_sock_pool:
	BRPC_P_DeInitSocketPool(pSockData);
err_ntf_buf:
err_bind:
	close(pSockData->sock_ntf);
err_sock_ntf:
	BKNI_DestroyMutex(pSockData->lock);
err_mutex:
	BKNI_Free((*handle)->private_data);
err_private_data:
	BKNI_Free(*handle);
err_handle:

	return BERR_TRACE(BRPC_ERR_OPEN);
}


void BRPC_Close_SocketImpl(BRPC_Handle handle)
{
	/* close sockets */
	close(GET_SOCKETDATA(handle)->sock_ntf);
	/* free buffer */
	BKNI_Free(GET_SOCKETDATA(handle)->sock_ntf_buf);
	/* free client data */
	BRPC_P_DeInitSocketPool(GET_SOCKETDATA(handle));
	/* destroy the mutex */
	BKNI_DestroyMutex(GET_SOCKETDATA(handle)->lock);
	/* free memory */
	BKNI_Free(GET_SOCKETDATA(handle));
	BKNI_Free(handle);
}

BERR_Code BRPC_GetSocketDescriptor_SocketImpl(BRPC_Handle handle, int *fd)
{
	*fd = GET_SOCKETDATA(handle)->sock_ntf;
	return BERR_SUCCESS;
}

void BRPC_GetDefaultOpenSocketImplSettings(BRPC_OpenSocketImplSettings *socketSettings)
{
	BKNI_Memset(socketSettings, 0, sizeof(BRPC_OpenSocketImplSettings));
	socketSettings->host = BRPC_RMAGNUM_SERVER_IP_ADDR; /* whatever we agree to */
	socketSettings->port_req = BRPC_RMAGNUM_REQUEST_PORT;
	socketSettings->port_ack = BRPC_RMAGNUM_NOTIFICATION_ACK_PORT;
	socketSettings->timeout = 5000; /*5 second*/
	socketSettings->rpc_disabled = false;
}

