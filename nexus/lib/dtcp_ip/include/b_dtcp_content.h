/********************************************************************************************
*     (c)2004-2011 Broadcom Corporation                                                     *
*                                                                                           *
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,   *
*  and may only be used, duplicated, modified or distributed pursuant to the terms and      *
*  conditions of a separate, written license agreement executed between you and Broadcom    *
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants*
*  no license (express or implied), right to use, or waiver of any kind with respect to the *
*  Software, and Broadcom expressly reserves all rights in and to the Software and all      *
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU       *
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY                    *
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.                                 *
*
*  Except as expressly set forth in the Authorized License,                                 *
*
*  1.     This program, including its structure, sequence and organization, constitutes     *
*  the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to      *
*  protect the confidentiality thereof,and to use this information only in connection       *
*  with your use of Broadcom integrated circuit products.                                   *
*                                                                                           *
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"          *
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR                   *
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO            *
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES            *
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,            *
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION             *
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF              *
*  USE OR PERFORMANCE OF THE SOFTWARE.                                                      *
*                                                                                           *
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS         *
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR             *
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR               *
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF             *
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT              *
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE            *
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF              *
*  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 *   DTCP content management functions.
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *********************************************************************************************/
/*! \file b_dtcp_content.h
 *  \brief define DTCP-IP content managment data and function prototypes.
 */
#ifndef __DTCP_CONTENT_MANAGEMENT_H__
#define __DTCP_CONTENT_MANAGEMENT_H__

#include "b_dtcp_constants.h"

/*! \struct B_DtcpIpPcpHeader 
 *
 * This header is only applied for DTCP/IP 
 */
typedef struct B_DtcpIpPcpHeader
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
	unsigned int Reserved:	3;		/* Reserved */
	unsigned int C_A		:	1;		/* Cipher algorithm, 0-AES128 1-Optional M6 */
	unsigned int Emi		:	4;		/* Extended EMI */
#else
	unsigned int Emi		:	4;
	unsigned int C_A		:	1;
	unsigned int Reserved:	3;
#endif
	unsigned char ExchKeyLabel;
	unsigned char Nc[DTCP_CONTENT_KEY_NONCE_SIZE];
	int CL;
} __attribute__((__packed__)) B_DTCP_IP_PCPHeader_T;

/* PCP_UR header fromat 
 * DTCP-IP V1SE 1.3 or later
 */
typedef struct B_DtcpIpPCP_UR
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
	unsigned int UR_Mode:	2;
	unsigned int Content_type:	2;
	unsigned int APS:		2;
	unsigned int ICT:		1;
	unsigned int Source:		1;
	unsigned int AST:		1;
	unsigned int Reserved:	7;
#else
	unsigned int Source:		1;
	unsigned int ICT:		1;
	unsigned int APS:		2;
	unsigned int Content_type:	2;
	unsigned int UR_Mode:	2;
	unsigned int Reserved:	7;
	unsigned int AST:		1;
#endif
}__attribute__((__packed__)) B_DTCP_IP_PCP_UR_T;
/* 
 * PCP header format when PCP-UR is used.
 * DTCP-IP spec V1SE 1.3 or later.
 */
typedef struct B_DtcpIpPcpURHeader
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
	unsigned int Reserved:	3;		/* Reserved */
	unsigned int C_A		:	1;		/* Cipher algorithm, 0-AES128 1-Optional M6 */
	unsigned int Emi		:	4;		/* Extended EMI */
#else
	unsigned int Emi		:	4;
	unsigned int C_A		:	1;
	unsigned int Reserved:	3;
#endif
	unsigned char ExchKeyLabel;
	B_DTCP_IP_PCP_UR_T pcp_ur;
	unsigned char SNc[DTCP_CONTENT_KEY_NONCE_SIZE - 2];
	int CL;
}__attribute__((__packed__)) B_DTCP_IP_PCPURHeader_T;

typedef void * B_PacketHandle_T;

/*! \brief B_DTCP_Content_CreatePacketHeader create pcp header, called by source device only.
 *  \param[in] hAkehandle handle to AKE session.
 *  \param[in] Transport stream transport type.
 *  \param[in] StreamData point to source stream data structure.
 *  \param[in] ContentSize total size of the content.
 *  \param[in,out] pPacketHeader Buffer to hold created packet header
 */
BERR_Code B_DTCP_Content_CreatePacketHeader(B_AkeHandle_T hAkeHandle, B_StreamTransport_T  Transport,
		struct __b_dtcp_stream_data *StreamData,
		int ContentSize,
		unsigned char pPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE]);

/*! \brief B_DTCP_Content_ConsumePacketHeader consume pcp header, called by sink device only.
 *  \param[in] hAkeHandle handle to active AKE session.
 *  \param[in] pPacketHeader Buffer to hold packet header
 *  \param[in] StreamData pointer to sink stream data structure
 */
BERR_Code B_DTCP_Content_ConsumePacketHeader(B_AkeHandle_T hAkeHandle,
		unsigned char * pPacketHeader,
		struct __b_dtcp_stream_data *StreamData);
/*! \brief B_DTCP_Content_EncryptData encrypt protected content, called by source only.
 *  \param[in] aAkeHandle AKE session handle.
 *  \param[in] aPacketHandle handle to packet data (obtained from create packet header function).
 *  \param[in] aInBuffer input buffer.
 *  \param[in,out] aOutBuffer output buffer.
 *  \param[in] aBufferLength length of input buffer.
 */
BERR_Code B_DTCP_Content_EncryptData(B_AkeHandle_T aAkeHandle, B_PacketHandle_T aPacketHandle, 
                            unsigned char    *aInBuffer,
                            unsigned char    *aOutBuffer,
                            unsigned int      aBufferLength);
/*! \brief B_DTCP_Content_DecryptData decrypt protected content, called by source only.
 *  \param[in] aAkeHandle AKE session handle.
 *  \param[in] aPacketHandle handle to packet data (obtained from create packet header function).
 *  \param[in] aInBuffer input buffer.
 *  \param[in,out] aOutBuffer output buffer.
 *  \param[in] aBufferLength length of input buffer.
 */
BERR_Code B_DTCP_Content_DecryptData(B_AkeHandle_T aAkeHandle, B_PacketHandle_T  aPacketHandle, 
                            unsigned char    *aInBuffer,
                            unsigned char    *aOutBuffer,
                            unsigned int      aBufferLength);
/*! \brief close packet , free packet handle and cipher context.
 */
void B_DTCP_Content_ClosePacket(B_PacketHandle_T aPacketHandle);

#endif
