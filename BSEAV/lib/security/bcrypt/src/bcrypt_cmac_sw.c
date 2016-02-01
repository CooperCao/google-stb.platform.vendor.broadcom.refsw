/***************************************************************************
 *     (c)2014 Broadcom Corporation
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
 **************************************************************************/
#include <stdio.h>
#include "bstd.h"
#include "bcrypt.h"
#include "bcrypt_cmac_sw.h"
#include "bcrypt_aes_sw.h"

unsigned char const_Rb[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
};
unsigned char const_Zero[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define BLOCK_SIZE_IN_BYTES        16
#define MAC_SIZE_IN_BYTES        16

/* AES-CMAC Generation Function */

void BCrypt_CMACSw_Leftshift_Onebit(unsigned char *input,unsigned char *output)
{
    int        i;
    unsigned char overflow = 0;

    for ( i=15; i>=0; i-- )
    {
        output[i] = input[i] << 1;
        output[i] |= overflow;
        overflow = (input[i] & 0x80)?1:0;
    }
    return;
}

void BCrypt_CMACSw_Xor_128(unsigned char *a, unsigned char *b, unsigned char *out)
{
    int i;
    for (i=0;i<16; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}

void BCrypt_CMACSw_Padding ( unsigned char *lastb, unsigned char *pad, int length )
{
    int        j;

    /* original last block */
    for ( j=0; j<16; j++ )
    {
        if ( j < length )
        {
            pad[j] = lastb[j];
        }
        else if ( j == length )
        {
            pad[j] = 0x80;
        }
        else
        {
            pad[j] = 0x00;
        }
    }
}

void BCrypt_CMACSw_Generate_Subkey (unsigned char *key,
                                    unsigned char *K1,
                                    unsigned char *K2)
{
    unsigned char L[16];
    unsigned char Z[16];
    unsigned char tmp[16];
    int    i;

    for ( i=0; i<16; i++ )
        Z[i] = 0;

    AES_128(key,Z,L);

    if ( (L[0] & 0x80) == 0 )                 /* If MSB(L) = 0, then K1 = L << 1 */
    {
        BCrypt_CMACSw_Leftshift_Onebit(L,K1);
    }
    else                                    /* Else K1 = ( L << 1 ) (+) Rb */
    {
        BCrypt_CMACSw_Leftshift_Onebit(L,tmp);
        BCrypt_CMACSw_Xor_128(tmp,const_Rb,K1);
    }

    if ( (K1[0] & 0x80) == 0 )
    {
        BCrypt_CMACSw_Leftshift_Onebit(K1,K2);
    }
    else
    {
        BCrypt_CMACSw_Leftshift_Onebit(K1,tmp);
        BCrypt_CMACSw_Xor_128(tmp,const_Rb,K2);
    }
    return;
}

/*********************************************************************************/
/* CMAC function                                                                 */
/*********************************************************************************/
BCRYPT_STATUS_eCode BCrypt_CMACSw (    BCRYPT_Handle  hBcrypt,
                                    BCRYPT_S_CMACSwParam_t *pInputParam     )
{
    unsigned char    X[16],Y[16], M_last[16], padded[16];
    unsigned char    K1[16], K2[16];
    int                numBlocks, i;
    bool            completeBlock = false;


     BSTD_UNUSED(hBcrypt);

    /*
     * Generate subkeys
     */
    BCrypt_CMACSw_Generate_Subkey(pInputParam->pKey,K1,K2);

    /*
     * Calculate number of complete blocks
     */
    if ( pInputParam->length == 0 )
    {
        numBlocks = 1;
        completeBlock = false;
    }
    else
    {
        numBlocks = pInputParam->length / BLOCK_SIZE_IN_BYTES;
        if ( (pInputParam->length % BLOCK_SIZE_IN_BYTES) == 0 )     /* complete blocks */
        {
            completeBlock = true;
        }
        else                         /* last block is incomplete */
        {
            numBlocks++;
            completeBlock = false;
        }
    }

    if ( completeBlock )
    {
        BCrypt_CMACSw_Xor_128 (&pInputParam->pBuffer[BLOCK_SIZE_IN_BYTES * (numBlocks-1)],K1,M_last);
    }
    else
    {
        /*
         * Padding for the last block
         */
        BCrypt_CMACSw_Padding (&pInputParam->pBuffer[BLOCK_SIZE_IN_BYTES * (numBlocks-1)],padded,pInputParam->length%16);
        BCrypt_CMACSw_Xor_128 (padded,K2,M_last);
    }

    /*
     * Input starts with all 0's
     */
    for ( i = 0; i < BLOCK_SIZE_IN_BYTES; i++ )
        X[i] = 0;

    /*
     * Process all the blocks, except for the last one
     */
    for ( i=0; i<numBlocks-1; i++ )
    {
        BCrypt_CMACSw_Xor_128(X,&pInputParam->pBuffer[BLOCK_SIZE_IN_BYTES * i],Y); /* Y := Mi (+) X  */
        AES_128(pInputParam->pKey,Y,X);      /* X := AES-128(KEY, Y); */
    }

    /*
     * Process last block
     */
    BCrypt_CMACSw_Xor_128(X,M_last,Y);
    AES_128(pInputParam->pKey,Y,X);

    /*
     * Copy resulting MAC value to user buffer
     */
    for (i = 0; i < MAC_SIZE_IN_BYTES; i++ )
    {
        pInputParam->pMac[i] = X[i];
    }
    return BCRYPT_STATUS_eOK;
}
