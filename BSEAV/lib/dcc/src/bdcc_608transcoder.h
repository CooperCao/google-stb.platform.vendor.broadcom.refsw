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


#ifndef BDCC608TRANSCODER_H
#define BDCC608TRANSCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bdcc_cbuf.h"

typedef struct BDCC_608_TranscoderObject *BDCC_608_hTranscoder;

			/****************************
			 *
			 * Exported API Functions
			 *
			 ****************************/

/**************************************************************************
 *
 * Function:		BDCC_608_TranscodeOpen
 *
 * Inputs:			
 * 					BDCC_608_hTranscoder				- object to init
 *					Field				- defines 608 field, 1 or 2
 *					Channel				- defines 608 CC channel, 1 or 2
 *
 * Outputs:		
 * 					BDCC_608_hTranscoder				- object state is modified
 *
 * Returns:			BDCC_Error_eSuccess or a standard BDCC_Error error code
 *
 * Description:
 *
 * This function initializes a 608-708 Transcoder session object.
 *
 **************************************************************************/
BDCC_Error BDCC_608_TranscodeOpen(
	BDCC_608_hTranscoder 		*ph608Transcoder,	
	int 						Field, 
	int 						Channel
	); 

/**************************************************************************
 *
 * Function:		BDCC_608_TranscodeReset
 *
 * Inputs:			
 * 					BDCC_608_hTranscoder				- object to init
 *
 * Outputs:		
 *
 * Returns:			BDCC_Error_eSuccess or a standard BDCC_Error error code
 *
 * Description:
 *
 * This function resets a 608-708 Transcoder session object.
 *
 **************************************************************************/
BDCC_Error BDCC_608_TranscodeReset(
	BDCC_608_hTranscoder 		h608Transcoder,
	int 						Field, 
	int 						Channel
	); 


/**************************************************************************
 *
 * Function:		BCCGFX_TranscodeClose
 *
 * Inputs:			
 * 					BDCC_608_hTranscoder	- object to Close
 *
 * Outputs:		
 * 					h608Transcoder		- Handle is freed
 *
 * Returns:			None
 *
 * Description:
 *
 * This function closes a 608-708 Transcoder session object.
 *
 **************************************************************************/
void BDCC_608_TranscodeClose(
	BDCC_608_hTranscoder		h608Transcoder
	); 

/**************************************************************************
 *
 * Function:		BDCC_608_TranscodeProcess
 *
 * Inputs:			
 * 					BDCC_608_hTranscoder				- object, previously init'ed
 * 					pInBuf				- input buffer
 *
 * Outputs:		
 *					pOutBuf				- output buffer
 *
 * Returns:			BDCC_Error_eSuccess or a standard BDCC_Error error code
 *
 * Description:
 * 
 * This function reads (field,cc_byte1,cc_byte2) triplets from the 
 * input circular buffer and writes DTVCC EIA-708-B codes to the
 * output buffer.
 *
 **************************************************************************/
BDCC_Error BDCC_608_TranscodeProcess(
	BDCC_608_hTranscoder 	h608Transcoder,
	BDCC_CBUF 				*pInBuf,
	BDCC_CBUF 				*pOutBuf
	);

/**************************************************************************
 *
 * Function:		BDCC_608_GetNewActivity
 *
 * Inputs:			
 * 					BDCC_608_hTranscoder - object, previously init'ed
 *
 * Outputs:		
 *
 * Returns:			true/false depending on whether there has been any 
 *                  caption activity on the 608 caption channel that is
 *                  selected
 *
 * Description:
 * 
 * Indicates whether any processing (control or text) has occurred on the
 * selected caption channel since the previous call to this functions
 * 
 *
 **************************************************************************/

bool BDCC_608_GetNewActivity(
	BDCC_608_hTranscoder h608Transcoder
);


#ifdef __cplusplus
}
#endif

#endif /* BDCC608TRANSCODER_H */

