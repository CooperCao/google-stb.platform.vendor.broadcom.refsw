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



/*************************************************************************
 * 
 * MODULE DETAILS:
 *
 * A special feature of this circular buffer manager is that a portion of
 * the end of the buffer is reserved providing a contiguous buffer interface
 * during wrapping reads.  When a caller calls BDCC_CBUF_ReadPtr() or BDCC_CBUF_PeekPtr(), a
 * pointer into the buffer is returned, guaranteed to be good for the 
 * requested number of bytes.  If this read/peek wraps past the end of
 * the circular buffer, the reserved portion is filled with bytes from
 * the beginning of the buffer.  
 *
 *************************************************************************/

#include "b_api_shim.h"
#include "bdcc_cbuf.h"
#include "bdcc_priv.h"

BDBG_MODULE(BDCC_CBUF);

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
 * Returns:			BDCC_CBUF_Success or a standard BDCC_CBUF_Error error code
 *
 * Description:
 * 
 * This function initializes the given BDCC_CBUF structure for use in
 * managing a circular buffer.  The caller supplies the memory for this
 * structure as well as the memory for the underlying buffer.
 * 
 * A special feature of this circular buffer manager is that a portion of
 * the end of the buffer is reserved providing a contiguous buffer interface
 * during wrapping reads.  When a caller calls BDCC_CBUF_ReadPtr() or BDCC_CBUF_PeekPtr(), a
 * pointer into the buffer is returned, guaranteed to be good for the 
 * requested number of bytes.  If this read/peek wraps past the end of
 * the circular buffer, the reserved portion is filled with bytes from
 * the beginning of the buffer.  
 * 
 * The ResvSize argument should be chosen carefully.  It should be equal to
 * the largest size that will be requested by the BDCC_CBUF_ReadPtr() or
 * BDCC_CBUF_PeekPtr() functions.
 *
 **************************************************************************/
BDCC_CBUF_Error BDCC_CBUF_Init(BDCC_CBUF * pCBuf, unsigned char * pBuf, unsigned int TotalBufSize, unsigned int ResvSize)
{
	if ( NULL == pCBuf   
	||   NULL == pBuf   
	||   TotalBufSize == 0   
	||   ResvSize >= TotalBufSize )
	{
		return(BDCC_CBUF_Error_eInvalidParameter) ;
	}

	pCBuf->pBuf				= pBuf ;
	pCBuf->TotalBufSize		= TotalBufSize ;
	pCBuf->UsableBufSize	= TotalBufSize - ResvSize ;
	pCBuf->ResvSize			= ResvSize ;
	pCBuf->pEnd				= &pBuf[pCBuf->UsableBufSize] ;
	BDCC_CBUF_Reset(pCBuf) ;
	return(BDCC_CBUF_Error_eSuccess) ;
	
} /* BDCC_CBUF_Init */


/**************************************************************************
 *
 * Function:		BDCC_CBUF_Reset
 *
 * Inputs:			
 *					pBuf				- ptr to byte array
 *
 * Outputs:		
 * 					pCBuf				- object to init
 *
 * Returns:			BDCC_CBUF_Error_eSuccess or a standard BDCC_CBUF_Error error code
 *
 * Description:
 * 
 * This function reinitializes the circular buffer to zero bytes.
 *
 **************************************************************************/
void BDCC_CBUF_Reset(BDCC_CBUF * pCBuf)
{
	pCBuf->pWrite			= pCBuf->pBuf ;
	pCBuf->pRead			= pCBuf->pBuf ;
	pCBuf->pPost			= pCBuf->pWrite ;
	pCBuf->pPeek			= pCBuf->pRead ;
	pCBuf->NumBytes			= 0 ;
	pCBuf->FreeBytes		= pCBuf->UsableBufSize ;
	pCBuf->Error			= BDCC_CBUF_Error_eSuccess ;
	pCBuf->BytesWritten		= 0 ;
	pCBuf->BytesRead		= 0 ;
	pCBuf->AlreadyPeeked	= 0 ;
	pCBuf->AlreadyPosted	= 0 ;
	pCBuf->ErrorCount		= 0 ;

} /* BDCC_CBUF_Reset */

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
void BDCC_CBUF_ResetPeek(BDCC_CBUF * pCBuf)
{
	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return ;
	}

	pCBuf->pPeek = pCBuf->pRead ;	
	pCBuf->AlreadyPeeked = 0 ;
	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;
	
} /* BDCC_CBUF_ResetPeek */

/**************************************************************************
 *
 * Function:		BDCC_CBUF_PeekByte
 *
 * Inputs:			
 *					pCBuf				- object previously init'ed
 *
 * Outputs:		
 *					pCBuf->Error		- standard BDCC_CBUF_Error error code
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
unsigned char BDCC_CBUF_PeekByte(BDCC_CBUF * pCBuf)
{
	unsigned char bReturn ;

	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return(0) ;
	}

	if ( (1 + pCBuf->AlreadyPeeked) > pCBuf->NumBytes )
	{
		pCBuf->Error = BDCC_CBUF_Error_eEmpty ;
		pCBuf->ErrorCount++ ;
		return(0) ;
	}

	bReturn = *(pCBuf->pPeek) ;
	pCBuf->pPeek += 1 ;
	if ( pCBuf->pPeek >= pCBuf->pEnd )
	{
		/* wrap */
		pCBuf->pPeek = pCBuf->pBuf + (pCBuf->pPeek - pCBuf->pEnd) ;
	}
	(pCBuf->AlreadyPeeked)++ ;

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;
	return(bReturn) ;

} /* BDCC_CBUF_PeekByte */

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
unsigned char * BDCC_CBUF_PeekPtr(BDCC_CBUF * pCBuf, unsigned int cnt)
{
	unsigned char * pReturn ;

	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return(NULL) ;
	}

	if ( cnt > pCBuf->ResvSize )
	{
		pCBuf->Error = BDCC_CBUF_Error_eBadCount ;
		pCBuf->ErrorCount++ ;
		return(NULL) ;
	}

	if ( (cnt + pCBuf->AlreadyPeeked) > pCBuf->NumBytes )
	{
		pCBuf->Error = BDCC_CBUF_Error_eEmpty ;
		pCBuf->ErrorCount++ ;
		return(NULL) ;
	}

	if ( &pCBuf->pPeek[cnt] > pCBuf->pEnd )
	{
		/* read past the end, back fill into resv */
		BKNI_Memcpy(pCBuf->pEnd, pCBuf->pBuf, pCBuf->ResvSize) ;
	}

	pReturn = pCBuf->pPeek ;
	pCBuf->pPeek += cnt ;
	if ( pCBuf->pPeek >= pCBuf->pEnd )
	{
		/* wrap */
		pCBuf->pPeek = pCBuf->pBuf + (pCBuf->pPeek - pCBuf->pEnd) ;
	}
	(pCBuf->AlreadyPeeked) += cnt ;

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;
	return(pReturn) ;
	
} /* BDCC_CBUF_PeekPtr */


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
void BDCC_CBUF_UpdatePeek(BDCC_CBUF * pCBuf)
{
	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return ;
	}

	pCBuf->pRead = pCBuf->pPeek ;
	pCBuf->FreeBytes += pCBuf->AlreadyPeeked ;
	pCBuf->NumBytes -= pCBuf->AlreadyPeeked ;
	pCBuf->BytesRead += pCBuf->AlreadyPeeked ;
	pCBuf->AlreadyPeeked = 0 ;

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;

} /* BDCC_CBUF_UpdatePeek */


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
void BDCC_CBUF_UnPeek(BDCC_CBUF * pCBuf, unsigned int amt)
{
	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return ;
	}

	if ( amt > pCBuf->AlreadyPeeked )
	{
		pCBuf->Error = BDCC_CBUF_Error_eBadCount ;
		pCBuf->ErrorCount++ ;
		return ;
	}

	pCBuf->AlreadyPeeked -= amt ;
	pCBuf->pPeek -= amt ;
	if ( pCBuf->pPeek < pCBuf->pBuf )
	{
		/* under wrap */
		pCBuf->pPeek = pCBuf->pEnd - (pCBuf->pBuf - pCBuf->pPeek) ;
	}

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;

} /* BDCC_CBUF_UnPeek */


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
void BDCC_CBUF_ResetPost(BDCC_CBUF * pCBuf)
{
	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return ;
	}

	pCBuf->pPost = pCBuf->pWrite ;	
	pCBuf->AlreadyPosted = 0 ;

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;
	
} /* BDCC_CBUF_ResetPost */


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
BDCC_CBUF_Error BDCC_CBUF_PostByte(BDCC_CBUF * pCBuf, unsigned char b)
{
	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return(BDCC_CBUF_Error_eInvalidParameter) ;
	}

	if ( pCBuf->AlreadyPosted >= pCBuf->FreeBytes )
	{
		pCBuf->Error = BDCC_CBUF_Error_eFull ;
		pCBuf->ErrorCount++ ;
		return(pCBuf->Error) ;
	}

	*(pCBuf->pPost)++ = b ;
	if ( pCBuf->pPost >= pCBuf->pEnd )
	{
		/* wrap */
		pCBuf->pPost = pCBuf->pBuf + (pCBuf->pPost - pCBuf->pEnd) ;
	}
	(pCBuf->AlreadyPosted)++ ;

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;
	return(pCBuf->Error) ;
	
} /* BDCC_CBUF_PostByte */



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
BDCC_CBUF_Error BDCC_CBUF_PostPtr(BDCC_CBUF * pCBuf, unsigned char * pInBuf, unsigned int Len)
{
	unsigned char * pPost ;
	unsigned char * pPostWrap ;
	unsigned int PostLen ;
	unsigned int PostLenWrap ;

	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return(BDCC_CBUF_Error_eInvalidParameter) ;
	}

	if ( (pCBuf->AlreadyPosted + Len - 1) >= pCBuf->FreeBytes )
	{
		pCBuf->Error = BDCC_CBUF_Error_eFull ;
		pCBuf->ErrorCount++ ;
		return(pCBuf->Error) ;
	}

	pPost = pCBuf->pPost ;
	PostLen = min(Len,(unsigned int)(pCBuf->pEnd-pCBuf->pPost)) ;
	pPostWrap = pCBuf->pBuf ;
	PostLenWrap = Len - PostLen ;

	if ( PostLen )
	{
		BKNI_Memcpy(pPost, pInBuf, PostLen) ;
		if ( PostLenWrap )
		{
			BKNI_Memcpy(pPostWrap, &pInBuf[PostLen], PostLenWrap) ;
		}
	}

	pCBuf->pPost += Len ;
	if ( pCBuf->pPost >= pCBuf->pEnd )
	{
		/* wrap */
		pCBuf->pPost = pCBuf->pBuf + (pCBuf->pPost - pCBuf->pEnd) ;
	}
	(pCBuf->AlreadyPosted) += Len ;

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;
	return(pCBuf->Error) ;
	
} /* BDCC_CBUF_PostPtr */


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
void BDCC_CBUF_UpdatePost(BDCC_CBUF * pCBuf)
{
	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return ;
	}

	pCBuf->pWrite = pCBuf->pPost ;
	pCBuf->FreeBytes -= pCBuf->AlreadyPosted ;
	pCBuf->NumBytes += pCBuf->AlreadyPosted ;
	pCBuf->BytesWritten += pCBuf->AlreadyPosted ;
	pCBuf->AlreadyPosted = 0 ;

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;

} /* BDCC_CBUF_UpdatePost */


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
	unsigned int * pWriteLenWrap)
{
	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return ;
	}

	if ( NULL == ppWrite  ||  NULL == pWriteLen  ||  NULL == ppWriteWrap || NULL == pWriteLenWrap )
	{
		pCBuf->Error = BDCC_CBUF_Error_eInvalidParameter ;
		pCBuf->ErrorCount++ ;
		return ;
	}

	*ppWrite = pCBuf->pWrite ;
	*pWriteLen = min(pCBuf->FreeBytes,(unsigned int)(pCBuf->pEnd-pCBuf->pWrite)) ;
	*ppWriteWrap = pCBuf->pBuf ;
	*pWriteLenWrap = pCBuf->FreeBytes - *pWriteLen ;

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;
	
} /* BDCC_CBUF_GetWritePtrs */


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
 * This function completes a write operation started by cbGetWritePtrs().
 * The write pointer is advanced by 'Len' bytes, accounting for wrap.
 *
 **************************************************************************/
void BDCC_CBUF_FinishWritePtrs(BDCC_CBUF * pCBuf, unsigned int Len)
{
	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return ;
	}

	if ( Len > pCBuf->FreeBytes )
	{
		pCBuf->Error = BDCC_CBUF_Error_eFull ;
		pCBuf->ErrorCount++ ;
		return ;
	}
		
	pCBuf->pWrite += Len ;
	if ( pCBuf->pWrite >= pCBuf->pEnd )
	{
		/* wrap */
		pCBuf->pWrite = pCBuf->pBuf + (pCBuf->pWrite - pCBuf->pEnd) ;
	}
	pCBuf->pPost = pCBuf->pWrite ;

	pCBuf->FreeBytes -= Len ;
	pCBuf->NumBytes += Len ;
	pCBuf->BytesWritten += Len ;
	pCBuf->AlreadyPosted = 0 ;

	pCBuf->Error = BDCC_CBUF_Error_eSuccess ;
	
} /* BDCC_CBUF_FinishWritePtrs */


/**************************************************************************
 *
 * Function:		cbWritePtr
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
void BDCC_CBUF_WritePtr(BDCC_CBUF * pCBuf, unsigned char * pInBuf, unsigned int Len)
{
	unsigned char * pWrite ;
	unsigned int LenWrite ;
	unsigned char * pWrap ;
	unsigned int LenWrap ;

	if ( NULL == pCBuf   ||   NULL == pCBuf->pBuf )
	{
		BDBG_ERR(("Error %s",BSTD_FUNCTION));;
		return ;
	}

	/* check it to make sure it is not DELAY update which will pass 0 size */
	if (0 == Len) {
		return;
	}

	BDCC_CBUF_GetWritePtrs(pCBuf, &pWrite, &LenWrite, &pWrap, &LenWrap) ;
	if ( pCBuf->Error == BDCC_CBUF_Error_eSuccess ) 
	{
		if ( (LenWrite + LenWrap) < Len )
		{
			pCBuf->Error = BDCC_CBUF_Error_eFull ;
			pCBuf->ErrorCount++ ;
		}
		else
		{
			LenWrite = min(Len,LenWrite) ;
			LenWrap = Len - LenWrite ;
			BKNI_Memcpy(pWrite, pInBuf, LenWrite) ;
			if ( LenWrap )
			{
				BKNI_Memcpy(pWrap, pInBuf + LenWrite, LenWrap) ;
			}
			BDCC_CBUF_FinishWritePtrs(pCBuf, Len) ;
		}
	}

	/* note: pCBuf->Error has already been set appropriately */
	
} /* BDCC_CBUF_WritePtr */
