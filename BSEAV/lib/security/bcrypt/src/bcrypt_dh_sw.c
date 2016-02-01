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

/* General purpose Diffie Hellman routines. */
#include "bstd.h"
#include "berr.h"
#include "bkni.h"
#include "bdbg.h"

#include <stdio.h>
#include <stdlib.h>

#include <openssl/pem.h>
#include <openssl/dh.h>

#include "bcrypt_dh_sw.h"

BDBG_MODULE(BCRYPT);

/* local functions definitions */
static BCRYPT_DH_t * _contextAlloc(void);
static void _contextFree(BCRYPT_DH_t * context);
static void _sharedSecretFree(BCRYPT_DH_t * context);

static void _sharedSecretFree(BCRYPT_DH_t * context)
{
    if (context->sharedSecret) {
        /* clear memory before release */
        BKNI_Memset(context->sharedSecret, 0, context->sharedSecretLen);
        BKNI_Free(context->sharedSecret);
        context->sharedSecret = NULL;
    }
    context->sharedSecretLen = 0;
}

static BCRYPT_DH_t * _contextAlloc(void)
{
    BCRYPT_DH_t *context = BKNI_Malloc(sizeof(*context));

    if (context) {
        BKNI_Memset(context, 0, sizeof(*context));
    }

    return context;
}

static void _contextFree(BCRYPT_DH_t * context)
{
    if (context->DHhandle) {
        /* DH_free is secure and uses BN_clear_free to clear BIGNUMs */
        DH_free((DH *)context->DHhandle);
        context->DHhandle = NULL;
    }
    if (context->pubKey) {
        BKNI_Free(context->pubKey);
        context->pubKey = NULL;
    }
    context->pubKeyLen = 0;
    _sharedSecretFree(context);
    BKNI_Free(context);
}


BCRYPT_STATUS_eCode BCrypt_DH_FromPem(const uint8_t * pem,
                                      int pemLen,
                                      BCRYPT_DH_t ** pContext)
{
    BCRYPT_STATUS_eCode rc = BCRYPT_STATUS_eOK;
    BIO * bio = NULL;
    BCRYPT_DH_t *context = NULL;
    DH * dh;
    int retLen;

    if (!pContext) {
        BDBG_ERR(("%s - invalid context holder %p", __FUNCTION__, pContext));
        rc = BCRYPT_STATUS_eINVALID_PARAMETER;
        goto end;
    }

    *pContext = NULL;

    if (!pem) {
        BDBG_ERR(("%s - invalid pem parameter %p", __FUNCTION__, pem));
        rc = BCRYPT_STATUS_eINVALID_PARAMETER;
        goto end;
    }

    context = _contextAlloc();
    if (!context) {
        BDBG_ERR(("%s - context allocation failed", __FUNCTION__));
        rc = BCRYPT_STATUS_eOUT_OF_SYSTEM_MEMORY;
        goto end;
    }

    bio = BIO_new_mem_buf((void *)pem, (int)pemLen);
    if (!bio) {
        BDBG_ERR(("%s - BIO allocation failed", __FUNCTION__));
        rc = BCRYPT_STATUS_eOUT_OF_SYSTEM_MEMORY;
        goto end;
    }

    dh = PEM_read_bio_DHparams(bio,
                               (DH **)NULL,
                               (pem_password_cb *)NULL,
                               (void *)NULL);
    if (!dh) {
        BDBG_ERR(("%s - PEM_read_BIO_DHparams failed", __FUNCTION__));
        rc = BCRYPT_STATUS_eFAILED;
        goto end;
    }
    /* save dh immediately for ressource cleanup in error handling */
    context->DHhandle = dh;

    if (!DH_generate_key(dh)) {
        BDBG_ERR(("%s - DH_generate_key failed", __FUNCTION__));
        rc = BCRYPT_STATUS_eFAILED;
        goto end;
    }

    retLen = BN_num_bytes(dh->pub_key);
    if (retLen <= 0) {
        rc = BCRYPT_STATUS_eFAILED;
        goto end;
    }
    context->pubKeyLen = (uint16_t)retLen;
    context->pubKey = (uint8_t *)BKNI_Malloc(retLen);
    if (!context->pubKey) {
        BDBG_ERR(("%s - public key allocation (%u bytes) failed",
                  __FUNCTION__, context->pubKeyLen));
        rc = BCRYPT_STATUS_eOUT_OF_SYSTEM_MEMORY;
        goto end;
    }

    if (BN_bn2bin(dh->pub_key, context->pubKey) != retLen) {
        BDBG_ERR(("%s - BN_bn2bin failed", __FUNCTION__));
        rc = BCRYPT_STATUS_eFAILED;
        goto end;
    }

    /* Everything is OK */
    *pContext = context;
end:
    if ((rc != BCRYPT_STATUS_eOK) && (context != NULL)) {
        _contextFree(context);
    }
    if (bio) {
        BIO_free(bio);
    }
    return rc;
}

BCRYPT_STATUS_eCode BCrypt_DH_Free(BCRYPT_DH_t * context)
{
    if (!context) {
        return BCRYPT_STATUS_eINVALID_PARAMETER;
    }

    _contextFree(context);
    /* from here context block is not useable and references should be set to NULL */

    return BCRYPT_STATUS_eOK;
}

BCRYPT_STATUS_eCode BCrypt_DH_ComputeSharedSecret(BCRYPT_DH_t * context,
                                                  const uint8_t * remotePublicKey,
                                                  int keyLen)
{
    BCRYPT_STATUS_eCode rc = BCRYPT_STATUS_eOK;
    DH * dh = (DH *)context->DHhandle;
    BIGNUM * bn_remotePublicKey = NULL;
    int length;

    if (!context) {
        BDBG_ERR(("%s - invalid parameter (context is NULL)", __FUNCTION__));
        rc = BCRYPT_STATUS_eINVALID_PARAMETER;
        goto end;
    }

    if (!dh || !dh->pub_key || !dh->priv_key) {
        BDBG_ERR(("%s - invalid dh context", __FUNCTION__));
        rc = BCRYPT_STATUS_eINVALID_PARAMETER;
        goto end;
    }

    if (context->sharedSecret) {
        BDBG_WRN(("%s - shared secret already computed", __FUNCTION__));
        goto end;
    }

    bn_remotePublicKey = BN_bin2bn(remotePublicKey, keyLen, NULL);
    if (!bn_remotePublicKey) {
        BDBG_ERR(("%s - BN_bin2bn failed", __FUNCTION__));
        rc = BCRYPT_STATUS_eFAILED;
        goto end;
    }

    length = DH_size(dh);
    if (length <= 0) {
        BDBG_ERR(("%s - DH_size returns %u", __FUNCTION__, length));
        rc = BCRYPT_STATUS_eFAILED;
        goto end;
    }

    context->sharedSecret = (uint8_t *)BKNI_Malloc(length);
    if (!context->sharedSecret) {
        BDBG_ERR(("%s - alloc shared secret (%u bytes) failed ",
                  __FUNCTION__, length));
        rc = BCRYPT_STATUS_eOUT_OF_SYSTEM_MEMORY;
        goto end;
    }

    length = DH_compute_key(context->sharedSecret,
                            bn_remotePublicKey, dh);
    if (length < 0) {
        BDBG_ERR(("%s - DH_compute_key failed ", __FUNCTION__));
        rc = BCRYPT_STATUS_eFAILED;
        goto end;
    }
    context->sharedSecretLen = (uint16_t) length;

    /* Success */
end:
    if (bn_remotePublicKey) {
        BN_free(bn_remotePublicKey);
    }
    if (rc != BCRYPT_STATUS_eOK) {
        if (context) {
            _sharedSecretFree(context);
        }
    }
    return rc;
}
