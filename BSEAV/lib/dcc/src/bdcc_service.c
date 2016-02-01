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

#include "b_api_shim.h"
#include "bdcc_service.h"
#include "bdcc_priv.h"

BDBG_MODULE(BDCCSERVICE);

typedef enum
 {
 	BDCC_SRV_P_State_eLookingForHeader,
 	BDCC_SRV_P_State_eLookingForExtendedHeader,
	BDCC_SRV_P_State_eCollectingBlock,
	BDCC_SRV_P_State_eSkippingBlock,
	BDCC_SRV_P_State_eFinishedHeader,
	BDCC_SRV_P_State_eFinishedBlock
 } BDCC_SRV_P_State ;



/**************************************************************************
 *
 * Function:		DccService_Process
 *
 * Inputs:			
 * 					pInBuf				- input buffer
 *					ServiceA			- service number
 *					ServiceB			- service number, or BDCC_SRV_SERVICE_ILLEGAL
 *
 * Outputs:		
 *					pOutBufA			- output buffer
 *					pOutBufB			- output buffer, or NULL
 *                  pNewActivity      - activity on selected caption service?
 *
 * Returns:			BDCC_Error_eSuccess or a standard BDCC_Error error code
 *
 * Description:
 * 
 * This function reads the input stream and outputs complete
 * DTVCC service blocks, not including the header.
 * 
 * The input stream format is that produced by DccPacket_Process and
 * contains DTVCC packets.
 * 
 * This version of the function does not consume the input and generate
 * output for partial blocks.  Only when the input contains one complete
 * block will the input be consumed and the output generated.  The output
 * may contain multiple complete blocks.
 * 
 **************************************************************************/
BDCC_Error
BDCC_SRV_P_Process(
	BDCC_CBUF *		pInBuf,
	unsigned int			ServiceA,
	unsigned int			ServiceB,
	BDCC_CBUF *		pOutBufA,
	BDCC_CBUF *		pOutBufB,
	bool *          pNewActivity
	)
{
	BDCC_SRV_P_State	state ;
	unsigned int			BytesThisBlock ;
	BDCC_CBUF *		pOutInfo ;
	unsigned int			NumBytes ;
	unsigned int			HeaderBytes ;
	unsigned int			block_size = 0 ;
	unsigned int			service_number = 0 ;
	unsigned int			curIndex ;
	unsigned char			curByte ;
	
    *pNewActivity = false;
    
	/*
	 * Validate Arguments
	 */
	if ( pInBuf == NULL )
	{
		BDBG_ERR(("BDCC_SRV_P_Process:  null pointer as argument\n")) ;
		return(BDCC_Error_eNullPointer) ;
	}

	/*
	 * Prep output buffers.
	 */
	if ( pOutBufA )
	{
		BDCC_CBUF_ResetPost(pOutBufA) ;
		pOutBufA->Error = BDCC_Error_eSuccess ;
	}
	else
		ServiceA = BDCC_SRV_SERVICE_ILLEGAL ;
		
	if ( pOutBufB )
	{
		BDCC_CBUF_ResetPost(pOutBufB) ;
		pOutBufB->Error = BDCC_Error_eSuccess ;
	}
	else
		ServiceB = BDCC_SRV_SERVICE_ILLEGAL ;

	BDCC_CBUF_ResetPeek(pInBuf) ;
	state = BDCC_SRV_P_State_eLookingForHeader ;
	BytesThisBlock = 0 ;
	pOutInfo = NULL ;
	HeaderBytes = 0 ;
	NumBytes = pInBuf->NumBytes ;

	for ( curIndex=0 ; curIndex < NumBytes ; curIndex++ )
	{
		curByte = BDCC_CBUF_PeekByte(pInBuf) ;
		switch ( state )
		{
			case BDCC_SRV_P_State_eLookingForHeader :

				block_size		= curByte & 0x1F ;
				service_number	= (curByte >> 5) & 0x07 ;
				BytesThisBlock	= 0 ;
				pOutInfo		= NULL ;
				HeaderBytes		= 1 ;

				BDBG_MSG(("%s: Service Number = %d", __FUNCTION__,service_number));

				if ( service_number == 7 )
					state = BDCC_SRV_P_State_eLookingForExtendedHeader ;
				else
					state = BDCC_SRV_P_State_eFinishedHeader ;
					
				break ;

			case BDCC_SRV_P_State_eLookingForExtendedHeader :

				HeaderBytes++ ;
				service_number = curByte & 0x3F ;
				state = BDCC_SRV_P_State_eFinishedHeader ;
				break ;

			case BDCC_SRV_P_State_eCollectingBlock :

				BDBG_MSG(("%s: CollectingBlock = %d", __FUNCTION__,service_number));
				BDCC_CBUF_PostByte(pOutInfo, curByte) ;
					
				/* fall thru... */
				
			case BDCC_SRV_P_State_eSkippingBlock :
			
				BDBG_MSG(("%s: SkippingBlock", __FUNCTION__));
				BytesThisBlock++ ;
				if ( BytesThisBlock == block_size )
				{
					/* done */
					state = BDCC_SRV_P_State_eFinishedBlock ;
				}
				break ;

			default :
				break ;
				
		} /* switch (state) */


		/*
		 * Handle the sequential states now.
		 */

		if ( state == BDCC_SRV_P_State_eFinishedHeader )
		{
			BDBG_MSG(("%s: FinishedHeader", __FUNCTION__));
			if ( block_size == 0   ||   service_number == 0 )
			{
				/* done, looking for next header */
				state = BDCC_SRV_P_State_eFinishedBlock ;
			}
			else if ( service_number == ServiceA )
			{
				pOutInfo = pOutBufA ;
				state = BDCC_SRV_P_State_eCollectingBlock ;
			}
			else if ( service_number == ServiceB )
			{
				pOutInfo = pOutBufB ;
				state = BDCC_SRV_P_State_eCollectingBlock ;
			}
			else
				state = BDCC_SRV_P_State_eSkippingBlock ;
				}
		
		if ( state == BDCC_SRV_P_State_eFinishedBlock )
		{
			BDBG_MSG(("%s: FinishedBlock", __FUNCTION__));
			if ( pOutInfo )
				BDCC_CBUF_UpdatePost(pOutInfo) ;
                *pNewActivity = true;
			BDCC_CBUF_UpdatePeek(pInBuf) ;

			state = BDCC_SRV_P_State_eLookingForHeader ;
		}
		
	} /* for (each input byte) */

	/*
	 * this final BDCC_CBUF_UpdatePeek will ensure that any partial
	 * block at the end of the packet will get dropped and
	 * not be incorrectly interpreted -- note that this requires
	 * that the packet layer above stops at packet boundaries
	 */
	BDCC_CBUF_UpdatePeek(pInBuf) ;

	return(BDCC_Error_eSuccess) ;

} /* DccService_Process */

