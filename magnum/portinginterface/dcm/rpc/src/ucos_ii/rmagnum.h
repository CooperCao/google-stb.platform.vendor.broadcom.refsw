/***************************************************************************
*     (c)2008-2009 Broadcom Corporation
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

#ifndef __BCM_RMAGNUM_H
#define __BCM_RMAGNUM_H

#if __cplusplus
extern "C" {
#endif

typedef unsigned long bcm_uint32_t;
typedef unsigned char bcm_uint8_t;
typedef int bcm_int32_t;

#define RMAGNUM_MAX_MSG_LEGNTH 1472
#define RMAGNUM_MAX_RESULT_LENGTH 1452
#define RMAGNUM_MAX_PARAMETER_LENGTH RMAGNUM_MAX_RESULT_LENGTH

/*#define RMAGNUM_SERVER_IP_ADDR_STR "192.168.100.1"*/
#define RMAGNUM_SERVER_IP_ADDR_STR	 "192.168.17.1"
/* IP address in UINT32 HEX */
#define RMAGNUM_SERVER_IP_ADDR	 0xC0A81101
#define RMAGNUM_REQUEST_PORT 0xBEEF
#define RMAGNUM_RESPONSE_PORT (RMAGNUM_REQUEST_PORT + 1)
#define RMAGNUM_NOTIFICATION_PORT (RMAGNUM_REQUEST_PORT + 2)
#define RMAGNUM_NOTIFICATION_ACK_PORT (RMAGNUM_REQUEST_PORT + 3)

/***************************************************************************
Summary:
	Header of Rmagnum REQUEST Message

Description:
	This structure defines the header of Rmagnum RESPONSE Message

See Also:	

****************************************************************************/
typedef struct RmagnumRequestHdr
{
	bcm_uint32_t version; /* the version of Rmagnum software */
	bcm_uint32_t sessionId; /* id for an eCM-eSTB communication session */
	bcm_uint32_t transactionId; /* transaction id for request/result */
	bcm_uint32_t msgType; /* request(0), reply(1) */
	bcm_uint32_t procedureCode; /* procedure ID */
} RmagnumRequestHdr;

/***************************************************************************
Summary:
	Header of Rmagnum RESPONSE Message

Description:
	This structure defines the header of Rmagnum RESPONSE Message

See Also:	

****************************************************************************/
typedef struct RmagnumResponseHdr
{
	bcm_uint32_t version; /* the version of Rmagnum software */
	bcm_uint32_t sessionId; /* id for an eCM-eSTB communication session */
	bcm_uint32_t transactionId; /* transaction id for request/result */
	bcm_uint32_t msgType; /* request(0), reply(1) */
	bcm_uint32_t returnCode; /* procedure ID */
} RmagnumResponseHdr;

/***************************************************************************
Summary:
	Header of Rmagnum NOTIFICATION Message

Description:
	This structure defines the header of Rmagnum RESPONSE Message

See Also:	

****************************************************************************/
typedef struct RmagnumNotificationHdr
{
	bcm_uint32_t version; /* the version of Rmagnum software */
	bcm_uint32_t sessionId; /* id for an eCM-eSTB communication session */
	bcm_uint32_t transactionId; /* transaction id for request/result */
	bcm_uint32_t msgType; /* request(0), reply(1) */
	bcm_uint32_t eventCode; /* procedure ID */
} RmagnumNotificationHdr;

/***************************************************************************
Summary:
	Header of Rmagnum NOTIFICATION ACK Message

Description:
	This structure defines the header of Rmagnum RESPONSE Message

See Also:	

****************************************************************************/
typedef struct RmagnumNotificationAckHdr
{
	bcm_uint32_t version; /* the version of Rmagnum software */
	bcm_uint32_t sessionId; /* id for an eCM-eSTB communication session */
	bcm_uint32_t transactionId; /* transaction id for request/result */
	bcm_uint32_t msgType; /* request(0), reply(1) */
	bcm_uint32_t eventCode; /* procedure ID */
} RmagnumNotificationAckHdr;

/***************************************************************************
Summary:
	Rmagnum REQUEST Message

Description:
	This structure defines the format of a Rmagnum RESPONSE Message

See Also:	

****************************************************************************/
typedef struct RmagnumRequestMsg
{
	RmagnumRequestHdr hdr; /* fixed length header */
	bcm_uint8_t parameter[RMAGNUM_MAX_PARAMETER_LENGTH]; /* variable length parameter block */
}RmagnumRequestMsg;

/***************************************************************************
Summary:
	Rmagnum RESPONSE Message

Description:
	This structure defines the format of a Rmagnum RESPONSE Message

See Also:	

****************************************************************************/
typedef struct RmagnumResponseMsg
{
	RmagnumResponseHdr hdr; /* fixed length header */
	bcm_uint8_t result[RMAGNUM_MAX_RESULT_LENGTH]; /* variable length result block */
}RmagnumResponseMsg;

/***************************************************************************
Summary:
	Rmagnum NOTIFICATION Message

Description:
	This structure defines the format of a Rmagnum NOTIFICATION Message

See Also:	

****************************************************************************/
typedef struct RmagnumNotificationMsg
{
	RmagnumNotificationHdr hdr; /* fixed length header */
	bcm_uint8_t eventData[RMAGNUM_MAX_RESULT_LENGTH]; /* variable length result block */
}RmagnumNotificationMsg;
/***************************************************************************
Summary:
	Rmagnum NOTIFICATION ACK Message

Description:
	This structure defines the format of a Rmagnum NOTIFICATION ACK Message

See Also:	

****************************************************************************/
typedef struct RmagnumNotificationAckMsg
{
	RmagnumNotificationAckHdr hdr; /* fixed length header */
}RmagnumNotificationAckMsg;

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
	/* more to come */
} RmagnumProcedureCode;

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
	/* more to come */
} RmagnumReturnCode;

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
	/* more to come */
}RmagnumEventCode;

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
}RmagnumMsgType;

/***************************************************************************
Summary:
	Rmagnum Parameter and Result 
Description:
	This structure defines the parameter and result block.

See Also:	

****************************************************************************/
typedef struct RmagnumParameterResult
{
	bcm_uint8_t * params;	/* [IN] pointer to the parameter block */ 
	bcm_uint32_t len_para;  /* [IN] length of the parameter block */
	bcm_uint8_t * res;      /* [IN] pointer to the result block */
	bcm_uint32_t maxlen_res;/* [IN] maximum length of the result block */
	bcm_uint32_t len_res;  /* [OUT] actual length of the returned result */
}RmagnumParameterResult;

/***************************************************************************
Summary:
	Rmagnum Procedure Prototype
Description:
	This typedef defines the prototype of a Rmagnum procedure.

See Also:	

****************************************************************************/
typedef bcm_int32_t (*PFN_RMAGNUM_PROCEDURE)(RmagnumParameterResult * pParaRes);

#if __cplusplus
}
#endif

#endif
