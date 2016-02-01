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

#include "bstd.h"
#include "stdio.h"
#include <string.h>

#include <openssl/des.h>
#include <openssl/crypto.h>

#include "bcrypt.h"
#include "bcrypt_desecb_sw.h"

BDBG_MODULE( BCRYPT );


BCRYPT_STATUS_eCode BCrypt_DESECBSw(
    BCRYPT_Handle     hBcrypt,
    BCRYPT_S_DESECBSwCtrl_t *inoutp_DESECBSwCtrl
)
{
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
#if defined(USES_BORINGSSL)
    DES_key_schedule ks;
#else
    des_key_schedule ks;
#endif

    unsigned int i ;

    DES_cblock des_input ;
    DES_cblock des_output ;

    BDBG_MSG(("Inside BCrypt_DESECBSw\n"));
    BDBG_ENTER(BCrypt_DESECBSw);
    BDBG_ASSERT( hBcrypt );
    BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, BCRYPT_STATUS_eFAILED,
        (hBcrypt->ulMagicNumber != BCRYPT_P_HANDLE_MAGIC_NUMBER ) );



#if defined(USES_BORINGSSL)
    DES_set_key((const DES_cblock *)inoutp_DESECBSwCtrl->pkey, &ks);
#else
    DES_set_key_unchecked((const_DES_cblock *)inoutp_DESECBSwCtrl->pkey, &ks);
#endif

    for (i=0; i<(inoutp_DESECBSwCtrl->len/8)*8; i+=8)
        {
#if defined(USES_BORINGSSL)
            memcpy(&des_input,inoutp_DESECBSwCtrl->pIn+i,8);
#else
            memcpy(des_input,inoutp_DESECBSwCtrl->pIn+i,8);
#endif

            DES_ecb_encrypt(&des_input, &des_output, &ks,  (inoutp_DESECBSwCtrl->bEncFlag?1:0));

#if defined(USES_BORINGSSL)
            memcpy(inoutp_DESECBSwCtrl->pOut+i,&des_output,8);
#else
            memcpy(inoutp_DESECBSwCtrl->pOut+i,des_output,8);
#endif

        }



BCRYPT_P_DONE_LABEL:

    BDBG_LEAVE(BCrypt_DESECBSw);
    return( errCode );
}
