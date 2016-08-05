/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/




#include "bhsm.h"
#include "bhsm_usercmd_common.h"
#include "bhsm_private.h"
#include "bhsm_bsp_msg.h"

#if defined(BSP_SECURE_RSA)
#include "bhsm_secure_rsa.h"
#endif /* BHSM_SECURE_RSA */

BDBG_MODULE ( BHSM );

BDBG_OBJECT_ID_DECLARE( BHSM_P_Handle );

void BHSM_MemcpySwap (
    unsigned char *pDest,
    unsigned char *pData,
    unsigned int  len,
    bool swap
)
{
    size_t alignment = 0;
	unsigned int tmp;
	unsigned int i;
	unsigned int runs, left_over;

    if( swap == false )
    {
        /* do a regular copy and exit. */
        BKNI_Memcpy( pDest, pData, len );
        return;
    }

    alignment = ((size_t)pDest) & 0x3;

	if ( alignment != 0 )
	{
		unsigned int left_over = 4 - alignment;
		unsigned int * pStart = (unsigned int*)(pDest-alignment);

		tmp = (*pStart) & (0xFFFFFFFF<< (left_over*8));
		do
		{
			tmp |= ((*pData) << ((left_over-1)*8) );
			len--;
			left_over--;
			pDest ++;
			pData ++;
		} while ((len>0) && (left_over>0 ));

		(*pStart) = tmp;
	}

	if ( len==0 )
		return;

	/* pDest should be aligned to 32 bits boundary by NOW */
	runs = len>>2;
	left_over = len & 3;

	for (i=0;i<runs;i++ )
	{
		tmp = (((unsigned int)pData[0])<<24) |
			(((unsigned int)pData[1])<<16) |
			(((unsigned int)pData[2])<<8) |
			((unsigned int)pData[3]);

		* ((unsigned int *)pDest) = tmp;
		pDest+=4;
		pData+=4;
	}

	if ( left_over )
	{
		tmp = 0;
		for (i=0;i<left_over;i++)
		{
			tmp |= (((unsigned int)pData[i])<<(24-i*8));
		}
		* ((unsigned int *)pDest) = tmp;
	}

    return;
}


/* This performs byte packing -- big endian */
/* 0102030401020304  --> 01020304 01020304 */

void BHSM_Pack_8BIT_TO_32BIT(
    uint32_t      *p_dest,
    unsigned char *p_src,
    unsigned int  len
)
{
	unsigned int i;

	if(len==0)
        return;

	for ( i = 0 ; i < len ; i++ )
	{
		if ( i % UINT32_SIZE_IN_BYTES == 0)
            *(p_dest + i/UINT32_SIZE_IN_BYTES ) = 0;
		*(p_dest + i /UINT32_SIZE_IN_BYTES )  |=  (*(p_src + i ) << (32 - ( i%UINT32_SIZE_IN_BYTES + 1)*8));
	}

}


/* This performs word  unpacking -- big endian */
/* 01020304 01020304  -->  0102030401020304 */

void BHSM_UnPack_32BIT_TO_8BIT(unsigned char *p_dest, uint32_t *p_src, unsigned int len)
{
	unsigned int i;

	for ( i = 0 ; i < len ; i++ )
		*(p_dest + i )  =  (unsigned char)((*(p_src+i /UINT32_SIZE_IN_BYTES)
										>> 	(32 - ( i %UINT32_SIZE_IN_BYTES +1) *8)) & 0xFF) ;

}

#if defined(BSP_SECURE_RSA)
BERR_Code      BHSM_PKEPollingCmd(
		BHSM_Handle				hHsm,
		BHSM_PKEPollingCmdIO_t	*pRsaPoll
)
{
    BERR_Code       errCode = BERR_SUCCESS;
    uint16_t        responceLength;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t         status;
	uint16_t        outputBufferSize;

    BDBG_ENTER ( BHSM_PKEPollingCmd );
    BDBG_ASSERT ( hHsm );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pRsaPoll == NULL )
    {
        return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if ( (errCode = BHSM_BspMsg_Create ( hHsm, &hMsg )) != BERR_SUCCESS )
    {
        return BERR_TRACE( errCode );
    }

    BHSM_BspMsg_GetDefaultHeader ( &header );
    BHSM_BspMsg_Header ( hMsg, BCMD_cmdType_ePKE_Cmd_Poll_Status, &header );

#if HSM_IS_ASKM_40NM_ZEUS_3_0
    BHSM_BspMsg_Pack8 ( hMsg, BCMD_PollingCommand_CmdInputField_ePollingTarget, pRsaPoll->pollTarget );
#endif

    errCode = BHSM_BspMsg_SubmitCommand ( hMsg );
    if ( errCode != BERR_SUCCESS )
    {
        errCode = BERR_TRACE ( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 ( hMsg, BCMD_SecureRsa_OutCmdOperation_eStatus, &status);

	/* if Status is still 0xA4, PKE_IN_PROGRESS, return back to the caller */
    if ( status == 0xA4 )
    {
        errCode = BHSM_STATUS_PKE_IN_PROGRESS;
        goto BHSM_P_DONE_LABEL;
    }

    /* If status is not 0, Error condition */
    if ( status != 0 )
    {
		BDBG_ERR ( ( "%s BSP status[0x%02X] error\n", __FUNCTION__, status) );
        errCode = BERR_TRACE ( BHSM_STATUS_FAILED );
        goto BHSM_P_DONE_LABEL;
    }

    /* Status is success; let's start gathering up data if there is */
    /* For UserRSA command, no data unless subcommand is LoadBaseStart  */
    if ( ( ( pRsaPoll->cmdToPoll >> 16 ) == 0x0E )
         && ( ( pRsaPoll->cmdToPoll & 0xFF ) != 0x02 ) )
    {
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get16 ( hMsg, BCMD_CommonBufferFields_eParamLen, &responceLength );

    if ( responceLength <= 4 )
    {
        /* There is no response data. */
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get16 ( hMsg, BCMD_SecureRsa_OutCmdOperation_eOutputDataSize, &outputBufferSize );

	pRsaPoll->outputBufSize = (uint32_t) outputBufferSize;

    if ( pRsaPoll->outputBufSize > BHSM_SECURE_PKE_OUTPUT_LEN )
    {
        errCode = BERR_TRACE ( BHSM_STATUS_PARM_LEN_ERR );
        BDBG_ERR ( ( "The output date size 0x%x is larger than the buffer size.",
                     pRsaPoll->outputBufSize ) );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetArray ( hMsg, BCMD_SecureRsa_OutCmdOperation_eOutputData, pRsaPoll->aucOutputBuf,
                           pRsaPoll->outputBufSize );

BHSM_P_DONE_LABEL:

    if ( hMsg )
    {
        ( void ) BHSM_BspMsg_Destroy ( hMsg );
    }

    BDBG_LEAVE(BHSM_PKEPollingCmd);
    return errCode;
}
#endif /* #if defined BSP_SECURE_RSA */
