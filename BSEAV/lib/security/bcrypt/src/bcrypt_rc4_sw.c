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
#include "bcrypt_rc4_sw.h"

BDBG_MODULE( BCRYPT );

#ifdef USE_OPENSSL_RC4
void BCrypt_RC4SetKey (BCRYPT_RC4Key_t *inputKey, uint32_t keyLen, uint8_t *keyData)
{
    /*
     * Call openssl function
     */
    RC4_set_key (inputKey, keyLen, keyData);

}

BCRYPT_STATUS_eCode BCrypt_RC4Sw (    BCRYPT_Handle  hBcrypt,
                                    BCRYPT_S_RC4SwParam_t *pInputParam     )
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;

    BDBG_MSG(("Inside BCrypt_RC4Sw\n"));
    BDBG_ENTER(BCrypt_RC4Sw);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    /*
     * Call openssl code
     */
    RC4 (pInputParam->key, pInputParam->cbLen, pInputParam->pbDataIn, pInputParam->pbDataOut);

BCRYPT_P_DONE_LABEL:

    BDBG_LEAVE(BCrypt_RC4Sw);
    return( errCode );
}

#else

#define RC4_INT unsigned int

#define SK_LOOP(n) { \
  tmp=d[(n)]; \
  id2 = (keyData[id1] + tmp + id2) & 0xff; \
  if (++id1 == keyLen) id1=0; \
  d[(n)]=d[id2]; \
  d[id2]=tmp; }

#define LOOP(in,out) \
  x=((x+1)&0xff); \
  tx=d[x]; \
  y=(tx+y)&0xff; \
  d[x]=ty=d[y]; \
  d[y]=tx; \
  (out) = d[(tx+ty)&0xff]^ (in);

#define RC4_LOOP(a,b)    LOOP(*((a)++),*((b)++))

void BCrypt_RC4SetKey (BCRYPT_RC4Key_t *key, uint32_t keyLen, uint8_t *keyData)
{
    register RC4_INT tmp;
    register uint32_t id1,id2;
    register RC4_INT *d;
    unsigned int i;

    d = &(key->data[0]);
    for (i=0; i<256; i++)
        d[i]=i;
    key->x = 0;
    key->y = 0;
    id1=id2=0;

    for (i=0; i < RC4_TABLESIZE; i+=4)
    {
        SK_LOOP(i+0);
        SK_LOOP(i+1);
        SK_LOOP(i+2);
        SK_LOOP(i+3);
    }
}

BCRYPT_STATUS_eCode BCrypt_RC4Sw (    BCRYPT_Handle  hBcrypt,
                                    BCRYPT_S_RC4SwParam_t *pInputParam     )
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    register RC4_INT *d;
    register RC4_INT x,y,tx,ty;
    int i;
    uint8_t *in, *out;

    BDBG_MSG(("Inside BCrypt_RC4Sw\n"));
    BDBG_ENTER(BCrypt_RC4Sw);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );

    x = pInputParam->key->x;
    y = pInputParam->key->y;
    d = pInputParam->key->data;
    in = pInputParam->pbDataIn;
    out = pInputParam->pbDataOut;

    i = (int)(pInputParam->cbLen >> 3L);
    if (i)
    {
        for (;;)
        {
            RC4_LOOP(in, out);
            RC4_LOOP(in, out);
            RC4_LOOP(in, out);
            RC4_LOOP(in, out);
            RC4_LOOP(in, out);
            RC4_LOOP(in, out);
            RC4_LOOP(in, out);
            RC4_LOOP(in, out);
            if (--i == 0) break;
        }
    }
    i = (int)pInputParam->cbLen & 0x07;
    if (i)
    {
        for (;;)
        {
            RC4_LOOP(in, out); if (--i == 0) break;
            RC4_LOOP(in, out); if (--i == 0) break;
            RC4_LOOP(in, out); if (--i == 0) break;
            RC4_LOOP(in, out); if (--i == 0) break;
            RC4_LOOP(in, out); if (--i == 0) break;
            RC4_LOOP(in, out); if (--i == 0) break;
            RC4_LOOP(in, out); if (--i == 0) break;
        }
    }
    pInputParam->key->x = x;
    pInputParam->key->y = y;

BCRYPT_P_DONE_LABEL:

    BDBG_LEAVE(BCrypt_RC4Sw);
    return( errCode );
}

#endif
