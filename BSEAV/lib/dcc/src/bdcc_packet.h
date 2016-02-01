/***************************************************************************
 *     (c)2002-2008 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BDCCPACKET_H
#define BDCCPACKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bdcc.h"
#include "bdcc_priv.h"
#include "bdcc_cbuf.h"


typedef struct
{
	int				PktSeqNum ;
} BDCC_PKT_P_Object ;


typedef BDCC_PKT_P_Object *BDCC_PKT_P_Handle ;

/**************************************************************************
 *
 * Function:		BDCC_PKT_P_Init
 *
 * Inputs:			
 * 					BDCC_PKT_P_Handle				- object to init
 *
 * Outputs:		
 * 					BDCC_PKT_P_Handle				- object state is modified
 *
 * Returns:			BDCC_Error_eSuccess or a standard BDCC_Error error code
 *
 * Description:
 *
 * This function initializes a DTVCC packet object.
 *
 **************************************************************************/
BDCC_Error BDCC_PKT_P_Init(BDCC_PKT_P_Handle hPacketObj);


/**************************************************************************
 *
 * Function:		BDCC_PKT_P_Process
 *
 * Inputs:			
 *					BDCC_PKT_P_Handle	- object previously init'ed
 * 					pInBuf				- input buffer
 *
 * Outputs:		
 *					pOutBuf				- output buffer
 *
 * Returns:			BDCC_Error_eSuccess or a standard BDCC_Error error code
 *
 * Description:
 * 
 * This function reads the input stream and outputs complete
 * DTVCC packets, not including the packet header.
 * 
 * The input stream format is that produced by DccTransport_Process and
 * contains byte triplets (cc_type,cc_data1,cc_data2).
 * 
 * This version of the function does not consume the input and generate
 * output for partial packets.  Only when the input contains one complete
 * packet will the input be consumed and the output generated.  
 * 
 * The processing stops at the end of a packet.  This is done so that
 * the next layer (service) can detect if a packet ends with an incomplete
 * service block.
 *
 **************************************************************************/
BDCC_Error BDCC_PKT_P_Process(
			BDCC_PKT_P_Handle hPacketObj, 
			BDCC_CBUF * pInBuf, 
			BDCC_CBUF * pOutBuf);



#ifdef __cplusplus
}
#endif

#endif /* BDCCPACKET_H */


