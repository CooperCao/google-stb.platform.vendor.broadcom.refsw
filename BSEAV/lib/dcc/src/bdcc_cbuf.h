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



/*************************************************************************
 * 
 * MODULE DETAILS:
 *
 * A special feature of this circular buffer manager is that a portion of
 * the end of the buffer is reserved providing a contiguous buffer interface
 * during wrapping reads.  When a caller calls cbReadPtr() or BDCC_CBUF_PeekPtr(), a
 * pointer into the buffer is returned, guaranteed to be good for the 
 * requested number of bytes.  If this read/peek wraps past the end of
 * the circular buffer, the reserved portion is filled with bytes from
 * the beginning of the buffer.  
 *
 *************************************************************************/

#ifndef BDCC_CBUF_H
#define BDCC_CBUF_H

#ifdef __cplusplus
extern "C" {
#endif



 typedef struct BDCC_CBUF
 {
 	/* C o n s t a n t */
 	unsigned char *			pBuf ;
 	unsigned char *			pEnd ;			/* just past end of 'usable' size */
 	unsigned int			TotalBufSize ;
 	unsigned int			UsableBufSize ;
 	unsigned int			ResvSize ;

 	/* D y n a m i c */
 	unsigned char *			pWrite ;
 	unsigned char *			pRead ;
 	unsigned char *			pPost ;
 	unsigned char *			pPeek ;
 	unsigned int			NumBytes ;
 	unsigned int			FreeBytes ;
 	unsigned int			Error ;
 	unsigned int			ErrorCount ;	/* accum, cleared on BDCC_CBUF_Reset and BDCC_CBUF_Init */
 	unsigned int			BytesWritten ;
 	unsigned int			BytesRead ;
 	unsigned int			AlreadyPeeked ;
 	unsigned int			AlreadyPosted ;
 } BDCC_CBUF ;

 typedef enum BDCC_CBUF_Error
 {
 	BDCC_CBUF_Error_eSuccess,
 	BDCC_CBUF_Error_eInvalidParameter,
 	BDCC_CBUF_Error_eBadCount,
	BDCC_CBUF_Error_eFull,
	BDCC_CBUF_Error_eEmpty
 } BDCC_CBUF_Error ;



/**************************************************************************
 *
 * Function:		BDCC_CBUF_Init
 *
 * Inputs:			
 *					pBuf				- ptr to byte array
 *					TotalBufSize		- size in bytes of *pBuf
 *					ResvSize			- number of bytes from buffer to 
 *										  reserve for providing a contiguous
 *										  read buffer when wrapping (see
 *										  Description below)
 *
 * Outputs:		
 * 					pCBuf				- object to init
 *
 * Returns:			BDCC_CBUF_Error_eSuccess or a standard BDCC_CBUF_Error error code
 *
 * Description:
 * 
 * This function initializes the given BDCC_CBUF structure for use in
 * managing a circular buffer.  The caller supplies the memory for this
 * structure as well as the memory for the underlying buffer.
 * 
 * A special feature of this circular buffer manager is that a portion of
 * the end of the buffer is reserved providing a contiguous buffer interface
 * during wrapping reads.  When a caller calls cbReadPtr() or BDCC_CBUF_PeekPtr(), a
 * pointer into the buffer is returned, guaranteed to be good for the 
 * requested number of bytes.  If this read/peek wraps past the end of
 * the circular buffer, the reserved portion is filled with bytes from
 * the beginning of the buffer.  
 * 
 * The ResvSize argument should be chosen carefully.  It should be equal to
 * the largest size that will be requested by the cbReadPtr() or
 * BDCC_CBUF_PeekPtr() functions.
 *
 **************************************************************************/
BDCC_CBUF_Error BDCC_CBUF_Init(BDCC_CBUF * pCBuf, 
					unsigned char * pBuf, 
					unsigned int TotalBufSize, 
					unsigned int ResvSize);

/**************************************************************************
 *
 * Function:		BDCC_CBUF_Reset
 *
 * Inputs:			
 *					pBuf				- ptr to byte array
 *
 * Outputs:		
 * 					None
 *
 * Returns:			BDCC_CBUF_Error_eSuccess or a standard BDCC_CBUF_Error error code
 *
 * Description:
 * 
 * This function reinitializes the circular buffer to zero bytes.
 *
 **************************************************************************/
void BDCC_CBUF_Reset(BDCC_CBUF * pCBuf);

/**************************************************************************
 *
 * Function:		BDCC_CBUF_ResetPeek
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *
 * Outputs:		
 *
 * Returns:			void
 *
 * Description:
 * 
 * This function resets the internal peek pointer.  The peek pointer
 * is set to the read pointer.
 *
 **************************************************************************/
void BDCC_CBUF_ResetPeek(BDCC_CBUF * pCBuf);

/**************************************************************************
 *
 * Function:		BDCC_CBUF_PeekByte
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *
 * Outputs:		
 *					pCBuf->Error		- standard cbErr error code
 *
 * Returns:			byte
 *
 * Description:
 * 
 * This function returns the byte pointed to by the peek pointer.
 * The peek pointer is updated.
 *
 * See Also:  BDCC_CBUF_ResetPeek() and BDCC_CBUF_UpdatePeek()
 *
 **************************************************************************/
unsigned char BDCC_CBUF_PeekByte(BDCC_CBUF * pCBuf);

/**************************************************************************
 *
 * Function:		BDCC_CBUF_PeekPtr
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *					cnt					- number of bytes to peek
 *
 * Outputs:		
 *
 * Returns:			NULL or a pointer to the requested bytes
 *
 * Description:
 * 
 * This function returns a pointer to the requested bytes without
 * updating the read pointer.  This is accomplished by using the
 * peek pointer.  The peek pointer is updated.
 *
 * If the requested bytes wrap past the end of the buffer, the
 * reserved portion of the buffer is filled with the beginning
 * part of the buffer so that the caller is given a pointer
 * to a contiguous memory block.
 *
 * See Also:  BDCC_CBUF_ResetPeek() and BDCC_CBUF_UpdatePeek()
 *
 **************************************************************************/
unsigned char * BDCC_CBUF_PeekPtr(BDCC_CBUF * pCBuf, unsigned int cnt);

/**************************************************************************
 *
 * Function:		BDCC_CBUF_UpdatePeek
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *
 * Outputs:		
 *
 * Returns:			void
 *
 * Description:
 * 
 * This function commits the previous peeks.  It does so by advancing the
 * read pointer to the peek pointer.  The various byte count members are
 * updated accordingly.
 *
 **************************************************************************/
void BDCC_CBUF_UpdatePeek(BDCC_CBUF * pCBuf);


/**************************************************************************
 *
 * Function:		BDCC_CBUF_UnPeek
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *					amt					- number of bytes to backup peek ptr
 *
 * Outputs:		
 *
 * Returns:			void
 *
 * Description:
 * 
 * This function backs up the peek pointer, in effect, unpeeking a number
 * of bytes.  
 *
 **************************************************************************/
void BDCC_CBUF_UnPeek(BDCC_CBUF * pCBuf, unsigned int amt);


/**************************************************************************
 *
 * Function:		BDCC_CBUF_ResetPost
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *
 * Outputs:		
 *
 * Returns:			void
 *
 * Description:
 * 
 * This function resets the internal post pointer.  The post pointer
 * is set to the write pointer.
 *
 **************************************************************************/
void BDCC_CBUF_ResetPost(BDCC_CBUF * pCBuf);

/**************************************************************************
 *
 * Function:		BDCC_CBUF_PostByte
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *					b					- byte to post
 *
 * Outputs:		
 *
 * Returns:			BDCC_CBUF_Error_eSuccess or a standard BDCC_CBUF_Error error code
 *
 * Description:
 * 
 * This function writes a byte into the given buffer, but without updating
 * the write pointer.  It does this by using and updating the post pointer.
 *
 * See Also:  BDCC_CBUF_ResetPost() and BDCC_CBUF_UpdatePost()
 *
 **************************************************************************/
BDCC_CBUF_Error BDCC_CBUF_PostByte(BDCC_CBUF * pCBuf, unsigned char b);

/**************************************************************************
 *
 * Function:		BDCC_CBUF_PostPtr
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *					pInBuf				- pointer to input buffer
 *					Len					- number of input bytes
 *
 * Outputs:		
 *
 * Returns:			BDCC_CBUF_Error_eSuccess or a standard BDCC_CBUF_Error error code
 *
 * Description:
 * 
 * This function writes bytes into the given buffer, but without updating
 * the write pointer.  It does this by using and updating the post pointer.
 *
 * See Also:  BDCC_CBUF_ResetPost() and BDCC_CBUF_UpdatePost()
 *
 **************************************************************************/
BDCC_CBUF_Error BDCC_CBUF_PostPtr(BDCC_CBUF * pCBuf, unsigned char * pInBuf, unsigned int Len);

/**************************************************************************
 *
 * Function:		BDCC_CBUF_UpdatePost
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *
 * Outputs:		
 *
 * Returns:			void
 *
 * Description:
 * 
 * This function commits the previous posts.  It does so by advancing the
 * write pointer to the post pointer.  The various byte count members are
 * updated accordingly.
 *
 **************************************************************************/
void BDCC_CBUF_UpdatePost(BDCC_CBUF * pCBuf);

/**************************************************************************
 *
 * Function:		BDCC_CBUF_GetWritePtrs
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *
 * Outputs:		
 *					ppWrite				- write pointer returned here
 *					pWriteLen			- write length returned here
 *					ppWriteWrap			- wrap pointer returned here
 *					pWriteLenWrap		- wrap length returned here
 *
 * Returns:			void
 *
 * Description:
 * 
 * This function returns pointer(s) to the un-written portion of the
 * circular buffer.  The caller may use this pointer to write directly
 * into the buffer and call BDCC_CBUF_FinishWritePtrs to declare how many bytes
 * were actually written.  If the un-written portion of the buffer wraps,
 * then both pointer, and both lengths, will be returned.  *pWriteLenWrap
 * is 0 if there is no wrapping portion.
 *
 **************************************************************************/
void BDCC_CBUF_GetWritePtrs(
	BDCC_CBUF * pCBuf, 
	unsigned char ** ppWrite, 
	unsigned int * pWriteLen,
	unsigned char ** ppWriteWrap, 
	unsigned int * pWriteLenWrap);


/**************************************************************************
 *
 * Function:		BDCC_CBUF_FinishWritePtrs
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *					Len					- bytes to advance write pointer
 *
 * Outputs:		
 *
 * Returns:			void
 *
 * Description:
 * 
 * This function completes a write operation started by BDCC_CBUF_GetWritePtrs().
 * The write pointer is advanced by 'Len' bytes, accounting for wrap.
 *
 **************************************************************************/
void BDCC_CBUF_FinishWritePtrs(BDCC_CBUF * pCBuf, unsigned int Len);


/**************************************************************************
 *
 * Function:		BDCC_CBUF_WritePtr
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *					pInBuf				- pointer to input buffer
 *					Len					- number of input bytes
 *
 * Outputs:		
 *
 * Returns:			void
 *
 * Description:
 * 
 * This function writes 'Len' bytes from 'pInBuf' to the circular buffer.
 *
 **************************************************************************/
void BDCC_CBUF_WritePtr(BDCC_CBUF * pCBuf, unsigned char * pInBuf, unsigned int Len);



#ifdef __cplusplus
}
#endif

#endif /* BDCC_CBUF_H */



