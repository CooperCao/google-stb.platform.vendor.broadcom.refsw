/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/

#include "b_api_shim.h"
#include "bdcc_packet.h"
#include "bdcc_priv.h"

BDBG_MODULE(BDCCPACKET);

typedef enum
 {
 	BDCC_PKT_P_State_eLookingForHeader,
 	BDCC_PKT_P_State_eCollectingPacket,
 	BDCC_PKT_P_State_eDone
 } BDCC_PKT_P_State ;
 
#define BDCC_PKT_P_AtscTypeDtvccPacketStart				3


static int CheckPktSeqOkay(int oldPktSeqNum, int newPktSeqNum);




/**************************************************************************
 *
 * Function:		BDCC_PKT_P_Init
 *
 * Inputs:			
 * 					hPacket				- object to init
 *
 * Outputs:		
 * 					hPacket				- object state is modified
 *
 * Returns:			BDCC_Error_eSuccess or a standard BDCC_Error error code
 *
 * Description:
 *
 * This function initializes a DTVCC packet object.
 *
 **************************************************************************/
BDCC_Error
BDCC_PKT_P_Init(BDCC_PKT_P_Handle hPacket)
{
	/*
	 * Validate
	 */
	if ( NULL == hPacket )
	{
		BDBG_ERR(("BDCC_PKT_P_Init:  null pointer as argument\n")) ;
		return(BDCC_Error_eNullPointer) ;
	}

	/*
	 * init it
	 */
	BKNI_Memset(hPacket, 0, sizeof(BDCC_PKT_P_Object)) ;
	hPacket->PktSeqNum = -1 ; /* <0 means no seq num has been set */
	
	/*
	 * all done
	 */
	return(BDCC_Error_eSuccess) ;
	
} /* BDCC_PKT_P_Init */



/**************************************************************************
 *
 * Function:		BDCC_PKT_P_Process
 *
 * Inputs:	
 *					hPacket				- object previously init'ed
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
			BDCC_PKT_P_Handle hPacket, 
			BDCC_CBUF * pInBuf, 
			BDCC_CBUF * pOutBuf)
{
	BDCC_PKT_P_State		state = BDCC_PKT_P_State_eLookingForHeader ;
	unsigned int			InputBufTriplets ;
	unsigned int			curTriplet ;
	unsigned int			packet_data_size = 0 ;
	unsigned int			packet_size_code ;
	unsigned int			OutputBytesThisPkt = 0 ;
	unsigned char *			pCurTriplet ;
	int						PktSeqNum ;
	
	/*
	 * Validate Arguments
	 */

	/*
	 * Prep output buffers.
	 */

	InputBufTriplets = pInBuf->NumBytes / 3 ;
	pInBuf->Error = BDCC_Error_eSuccess ;
	pOutBuf->Error = BDCC_Error_eSuccess ;
	BDCC_CBUF_ResetPeek(pInBuf) ;
	BDCC_CBUF_ResetPost(pOutBuf) ;

	for ( curTriplet=0 ; curTriplet < InputBufTriplets ; curTriplet++ )
	{
		pCurTriplet = BDCC_CBUF_PeekPtr(pInBuf, 3) ;

		if ( (state==BDCC_PKT_P_State_eCollectingPacket) 
		  && (pCurTriplet[0] == BDCC_PKT_P_AtscTypeDtvccPacketStart) )
		{
			/* shortened pkt, this is unexpected but okay */

			PktSeqNum = (pCurTriplet[1] >> 6) & 3 ;	
			if ( CheckPktSeqOkay(hPacket->PktSeqNum,PktSeqNum) )
			{
				/* shortened pkt and correct seq, no problem */
				BDBG_WRN(("BDCC_PKT_P_Process:  shortened\n")) ;
				BDCC_CBUF_UpdatePost(pOutBuf) ;
			}
			else
			{
				/*
				 * shortened pkt and discontinuous seq num
				 * don't update the output (ie discard it)
				 * but do update the input (ie consume it)
				 */
				BDBG_WRN(("BDCC_PKT_P_Process:  shortened and discontinuous\n")) ;
				BDCC_CBUF_ResetPost(pOutBuf) ;
			}

			state = BDCC_PKT_P_State_eDone;
			/* update all but the last peek */
			BDCC_CBUF_UnPeek(pInBuf,3) ;
			BDCC_CBUF_UpdatePeek(pInBuf) ;
		}
		
		switch ( state )
		{
			case BDCC_PKT_P_State_eLookingForHeader :
			
				OutputBytesThisPkt = 0 ;
				if ( pCurTriplet[0] == BDCC_PKT_P_AtscTypeDtvccPacketStart )
				{
					BDBG_MSG(("%s: Found AtscPacketStart", BSTD_FUNCTION));
					packet_size_code = pCurTriplet[1] & 0x3F ;

					PktSeqNum = (pCurTriplet[1] >> 6) & 3 ;	
					if ( ! CheckPktSeqOkay(hPacket->PktSeqNum,PktSeqNum) )
					{
						/* discontinuous packet sequence */ 
						/* by setting the error, we reset everything downstream from here */
						BDBG_WRN(("PktSeqNum discontinuity, %d to %d  returning BDCC_Error_eSequence\n", hPacket->PktSeqNum, PktSeqNum)) ;
						pOutBuf->Error = BDCC_Error_eSequence ;
						state = BDCC_PKT_P_State_eDone ;
						hPacket->PktSeqNum = PktSeqNum ;
						break ;
					}
					hPacket->PktSeqNum = PktSeqNum ;
				
					if ( packet_size_code == 0 )
						packet_size_code = 64 ;
					packet_data_size = (packet_size_code * 2) - 1 ;
					if ( packet_data_size > pOutBuf->FreeBytes )
					{
						pOutBuf->Error = BDCC_Error_eOutputBufTooSmall ;
						state = BDCC_PKT_P_State_eDone ;
						BDBG_MSG(("%s: BDCC_Error_eOutputBufTooSmall", BSTD_FUNCTION));
					}
					else
					{
						BDCC_CBUF_PostByte(pOutBuf, pCurTriplet[2]) ;
						OutputBytesThisPkt++ ;
						state = BDCC_PKT_P_State_eCollectingPacket ;
					}
				}
				else
					BDCC_CBUF_UpdatePeek(pInBuf) ;
				break ;

			case BDCC_PKT_P_State_eCollectingPacket :
				BDBG_MSG(("%s: Collecting Packet", BSTD_FUNCTION));
				BDCC_CBUF_PostByte(pOutBuf, pCurTriplet[1]) ;
				BDCC_CBUF_PostByte(pOutBuf, pCurTriplet[2]) ;
				OutputBytesThisPkt += 2 ;
				break ;

			case BDCC_PKT_P_State_eDone :
			default :
				break ;
				
		}

		/*
		 * Process sequential states.
		 */
		if ( (state == BDCC_PKT_P_State_eCollectingPacket)
		&&   (OutputBytesThisPkt >= packet_data_size) )
		{
			BDCC_CBUF_UpdatePeek(pInBuf) ;
			BDCC_CBUF_UpdatePost(pOutBuf) ;
			state = BDCC_PKT_P_State_eDone;
		}

		if ( state == BDCC_PKT_P_State_eDone )
		{
			break ;
		}
	} /* for (each triplet) */

	if ( (curTriplet == InputBufTriplets)   ||   pOutBuf->Error ) 
		return(pOutBuf->Error) ;
	else
		return(BDCC_Error_eWrnPause) ;

} /* BDCC_PKT_P_Process */


int CheckPktSeqOkay(int oldPktSeqNum, int newPktSeqNum)
{
	if ( oldPktSeqNum < 0 )
		return(1) ;
		
	if ( newPktSeqNum == oldPktSeqNum )
		return(1) ;

	if ( newPktSeqNum == ((oldPktSeqNum+1)%4) )
		return(1) ;

	return(0) ;
}
