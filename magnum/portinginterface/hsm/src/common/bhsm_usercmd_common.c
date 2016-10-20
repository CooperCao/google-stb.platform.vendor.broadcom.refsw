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
