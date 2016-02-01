/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
 *  No portions of this material may be reproduced in any form without the
 *  written permission of:
 *          Broadcom Corporation
 *          16215 Alton Parkway
 *          Irvine, California 92619
 *  All information contained in this document is Broadcom Corporation
 *  company private, proprietary, and trade secret.
 *
 *  Author:         Weimin Feng
 *  Creation Date:  November 11, 2004
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************/
/*****************************************************************************
*  Description:
*
*      This file defines Remote Magnum (Rmagnum) message set. The Rmagnum messsge
*  set is bus/media independent. It can be carried in the payload of various protocols
*  such as UDP/IP, Ethernet, 802.2/802.3 and PPPoE.
*
*  There are four types of messages: REQUEST, RESPONSE, NOTIFICATION and NOTIFICATION_ACK.
*
*  The byte order of Rmagnum messages is big endian.
*
*  A Rmagnum message has a fixed length header and variable length block, which contains
*  parameters for a REQUEST message, results for a RESPONSE message, or event data for a
*  Event message repectively.
*
*  Here's a detailed view of the packet format when UDP/IP is used:
*
*  +---+---+---+---+---+---+---+---+---+---+---+---+---------------/\/-------------+
*  |       IP Hdr      |UDP Hdr|  Rmagnum Msg Hdr  |      Parameter/Result/Event   |
*  +---+---+---+---+---+---+---+---+---+---+---+---+----------- ---/\/-------------+
*  |<-----20 bytes---->|<--8-->|<-------20-------->|<--------------N-------------->|
*
*  Here's detailed view of a Rmagnum REQUEST message header:
*
*  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
*  |    version    |   sessionId   | transactionId |    msgType    | procedureCode |
*  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
*
*  Here's detailed view of a Rmagnum RESPONSE message header:
*
*  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
*  |    version    |   sessionId   | transactionId |    msgType    |  returnCode   |
*  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
*
*  Here's detailed view of a Rmagnum NOTIFICATION/NOTIFICATION_ACK message header:
*
*  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
*  |    version    |   sessionId   | transactionId |    msgType    |   eventCode   |
*  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
*
*  The 'version' is used for both client and server to detect version mismatching.
*
*  The 'sessionId' is a unique identifier of a client/server communcation session. It is
*  negotiated between the client and the server.
*
*  The 'transactionId' is set by the client, and returned by the server. The client checks
*  'transactionId' in the RESPONSE message against the one it caches for an outstanding
*  REQUEST messages. If they match, the client will accept the 'returnCode' from the server.
*  If they don't match, the client will keep waiting until a timeout occurs.
*
*  The 'msgType' marks a message to be: REQUEST(0), RESPONSE(1), NOTIFICATION(2), NOTIFICATION_ACK(3)
*
*  The 'procedureCode' identifies which procedure the client wants to invoke on the server.
*
*  The 'returnCode' is the value returned by the procedure from the server.
*
*  The 'eventCode' identifies which event the server wants to notify the client.
*
*******************************************************************************************/

#ifndef __BRPC_RMAGNUM_H
#define __BRPC_RMAGNUM_H

#if __cplusplus
extern "C" {
#endif


#define BRPC_RMAGNUM_MAX_MSG_LEGNTH 1472
#define BRPC_RMAGNUM_MAX_RESULT_LENGTH 1452
#define BRPC_RMAGNUM_MAX_PARAMETER_LENGTH BRPC_RMAGNUM_MAX_RESULT_LENGTH

#if (BCHP_CHIP==7145)
#define BRPC_RMAGNUM_SERVER_IP_ADDR_STR	 "192.168.17.2"
#define BRPC_RMAGNUM_SERVER_IP_ADDR	 0xC0A81102
#define BRPC_RMAGNUM_LOCAL_IP_ADDR_STR	 "192.168.17.4"
#define BRPC_RMAGNUM_LOCAL_IP_ADDR	 0xC0A81104
#else
#define BRPC_RMAGNUM_SERVER_IP_ADDR_STR	 "192.168.17.1"
#define BRPC_RMAGNUM_SERVER_IP_ADDR	 0xC0A81101
#define BRPC_RMAGNUM_LOCAL_IP_ADDR_STR	 "192.168.17.10"
#define BRPC_RMAGNUM_LOCAL_IP_ADDR	 0xC0A8110A
#endif
#define BRPC_RMAGNUM_REQUEST_PORT 0xBEEF
#define BRPC_RMAGNUM_RESPONSE_PORT (BRPC_RMAGNUM_REQUEST_PORT + 1)
#define BRPC_RMAGNUM_NOTIFICATION_PORT (BRPC_RMAGNUM_REQUEST_PORT + 2)
#define BRPC_RMAGNUM_NOTIFICATION_ACK_PORT (BRPC_RMAGNUM_REQUEST_PORT + 3)
#define BRPC_RMAGNUM_CLNT_STARTPORT 0xBF00
#define BRPC_RMAGNUM_CLNT_NPORTS 32
#define BRPC_RMAGNUM_CLNT_NSOCKETS 4
#define BRPC_RMAGNUM_CLNT_DYNAMIC_STARTPORT (BRPC_RMAGNUM_CLNT_STARTPORT + BRPC_RMAGNUM_CLNT_NSOCKETS)

/***************************************************************************
Summary:
	Header of Rmagnum REQUEST Message

Description:
	This structure defines the header of Rmagnum RESPONSE Message

See Also:

****************************************************************************/
typedef struct BRPC_RmagnumRequestHdr
{
	uint32_t version; /* the version of Rmagnum software */
	uint32_t sessionId; /* id for an eCM-eSTB communication session */
	uint32_t transactionId; /* transaction id for request/result */
	uint32_t msgType; /* request(0), reply(1) */
	uint32_t procedureCode; /* procedure ID */
} BRPC_RmagnumRequestHdr;

/***************************************************************************
Summary:
	Header of Rmagnum RESPONSE Message

Description:
	This structure defines the header of Rmagnum RESPONSE Message

See Also:

****************************************************************************/
typedef struct BRPC_RmagnumResponseHdr
{
	uint32_t version; /* the version of Rmagnum software */
	uint32_t sessionId; /* id for an eCM-eSTB communication session */
	uint32_t transactionId; /* transaction id for request/result */
	uint32_t msgType; /* request(0), reply(1) */
	uint32_t returnCode; /* procedure ID */
} BRPC_RmagnumResponseHdr;

/***************************************************************************
Summary:
	Header of Rmagnum NOTIFICATION Message

Description:
	This structure defines the header of Rmagnum RESPONSE Message

See Also:

****************************************************************************/
typedef struct BRPC_RmagnumNotificationHdr
{
	uint32_t version; /* the version of Rmagnum software */
	uint32_t sessionId; /* id for an eCM-eSTB communication session */
	uint32_t transactionId; /* transaction id for request/result */
	uint32_t msgType; /* request(0), reply(1) */
	uint32_t eventCode; /* procedure ID */
} BRPC_RmagnumNotificationHdr;

/***************************************************************************
Summary:
	Header of Rmagnum NOTIFICATION ACK Message

Description:
	This structure defines the header of Rmagnum RESPONSE Message

See Also:

****************************************************************************/
typedef struct BRPC_RmagnumNotificationAckHdr
{
	uint32_t version; /* the version of Rmagnum software */
	uint32_t sessionId; /* id for an eCM-eSTB communication session */
	uint32_t transactionId; /* transaction id for request/result */
	uint32_t msgType; /* request(0), reply(1) */
	uint32_t eventCode; /* procedure ID */
} BRPC_RmagnumNotificationAckHdr;

/***************************************************************************
Summary:
	Rmagnum REQUEST Message

Description:
	This structure defines the format of a Rmagnum RESPONSE Message

See Also:

****************************************************************************/
typedef struct BRPC_RmagnumRequestMsg
{
	BRPC_RmagnumRequestHdr hdr; /* fixed length header */
	uint8_t parameter[BRPC_RMAGNUM_MAX_PARAMETER_LENGTH]; /* variable length parameter block */
}BRPC_RmagnumRequestMsg;

/***************************************************************************
Summary:
	Rmagnum RESPONSE Message

Description:
	This structure defines the format of a Rmagnum RESPONSE Message

See Also:

****************************************************************************/
typedef struct BRPC_RmagnumResponseMsg
{
	BRPC_RmagnumResponseHdr hdr; /* fixed length header */
	uint8_t result[BRPC_RMAGNUM_MAX_RESULT_LENGTH]; /* variable length result block */
}BRPC_RmagnumResponseMsg;

/***************************************************************************
Summary:
	Rmagnum NOTIFICATION Message

Description:
	This structure defines the format of a Rmagnum NOTIFICATION Message

See Also:

****************************************************************************/
typedef struct BRPC_RmagnumNotificationMsg
{
	BRPC_RmagnumNotificationHdr hdr; /* fixed length header */
	uint8_t eventData[BRPC_RMAGNUM_MAX_RESULT_LENGTH]; /* variable length result block */
}BRPC_RmagnumNotificationMsg;
/***************************************************************************
Summary:
	Rmagnum NOTIFICATION ACK Message

Description:
	This structure defines the format of a Rmagnum NOTIFICATION ACK Message

See Also:

****************************************************************************/
typedef struct BRPC_RmagnumNotificationAckMsg
{
	BRPC_RmagnumNotificationAckHdr hdr; /* fixed length header */
}BRPC_RmagnumNotificationAckMsg;

/***************************************************************************
Summary:
	Rmagnum Procedure Code

Description:
	This enum defines the procedure code of a Rmagnum server

See Also:

****************************************************************************/
typedef enum
{
	kRmagnumFirstProcedure = 0,
    kInitilizeSession   =  1,  /* Initialize a session */
	kNotify             =  2,  /* notification from server */
    kBadsGetTotalChannels      =  3,  /* BQDS_Get_Status */
	kRmagnumLastProcedure
} BRPC_RmagnumProcedureCode;

/***************************************************************************
Summary:
	Rmagnum Return Code

Description:
	This enum defines the return code from a Rmagnum procedure

See Also:

****************************************************************************/
typedef enum
{
    kOK         =  0,  /* procedure succeeds */
    kERROR      =  1  /* procedure fails */
} BRPC_RmagnumReturnCode;

/***************************************************************************
Summary:
	Rmagnum Event Code

Description:
	This enum defines the Rmagnum event code.

See Also:

****************************************************************************/
typedef enum
{
    kEventRmagnumLockLoss = 0,
    kEventRmagnumLockAcquired = 1
}BRPC_RmagnumEventCode;

/***************************************************************************
Summary:
	Rmagnum Message Type
Description:
	This enum defines the Rmagnum message type.

See Also:

****************************************************************************/
typedef enum
{
    kRequest = 0,
    kResponse = 1,
    kNotification = 2,
    kNotificationAck = 3
}BRPC_RmagnumMsgType;

/***************************************************************************
Summary:
	Rmagnum Parameter and Result
Description:
	This structure defines the parameter and result block.

See Also:

****************************************************************************/
typedef struct BRPC_RmagnumParameterResult
{
	uint8_t * params;	/* [IN] pointer to the parameter block */
	uint32_t len_para;  /* [IN] length of the parameter block */
	uint8_t * res;      /* [IN] pointer to the result block */
	uint32_t maxlen_res;/* [IN] maximum length of the result block */
	uint32_t len_res;  /* [OUT] actual length of the returned result */
}BRPC_RmagnumParameterResult;

/***************************************************************************
Summary:
	Rmagnum Procedure Prototype
Description:
	This typedef defines the prototype of a Rmagnum procedure.

See Also:

****************************************************************************/
typedef int32_t (*PFN_RMAGNUM_PROCEDURE)(BRPC_RmagnumParameterResult * pParaRes);

#if __cplusplus
}
#endif

#endif
