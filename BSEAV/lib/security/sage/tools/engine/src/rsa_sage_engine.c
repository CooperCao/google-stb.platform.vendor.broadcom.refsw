/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/

#include <openssl/crypto.h>
#include <openssl/objects.h>
#include <openssl/engine.h>
#include <openssl/rsa.h>
#ifndef ENGINE_CMD_BASE
#error did not get engine.h
#endif

#include "bstd.h"
#include "berr.h"
#include "bkni_multi.h"
#include "blst_list.h"
#include "rsa_sage_engine.h"
#include "utility_ids.h"
#include "bsagelib_types.h"
#include "bsagelib_crypto_types.h"
#include "rsa_tl.h"


BDBG_MODULE(rsa_sage_engine);


#define OPENSSL_SHA1_SIZE   (20)
#define OPENSSL_SHA256_SIZE (32)

#define MAX_ID_VALUE                7
#define RSA_SAGE_ENGINE             "rsa_sage_engine"
#define CMD_SAGE_UTILITY_TL     (ENGINE_CMD_BASE)

/* static variable definitions */
typedef struct RsaSageEngine_Context_s
{
    BLST_D_ENTRY(RsaSageEngine_Context_s) link;
    RSA *rsa;
    RsaTl_Handle hRsaTl;
    uint32_t drmBinFileKeyIndex;
} RsaSageEngine_Context;

static int _init_g_ctx = 0;
static struct {
    BKNI_MutexHandle hMutex;
    BLST_D_HEAD(RsaSageEngine_ContextList, RsaSageEngine_Context_s) list;
    RSA_METHOD current_method;
    RsaSettings rsaSettings;
} _g_ctx;


static void _display_engine_list(void);
static int _rsa_sage_engine_ctrl(ENGINE * e, int cmd, long i, void *p, void (*f) (void));
static int _init_rsa_ctx_cb(RSA *rsa);
static int _finish_rsa_ctx_cb(RSA *rsa);
static int _bind_helper(ENGINE * e);
static int _bind_fn(ENGINE * e, const char *id);
static RsaSageEngine_Context *_rsaengine_search(const RSA *rsa, int pop);
static void _rsaengine_insert(RsaSageEngine_Context * rsaEngine);


#if 0
void DBG_RSA_DUMP(unsigned len, unsigned char * ptr)
{
    int i;
    char    str0[2048]={0,},*str=str0;
    for (i=0; i<len; i++)
    {
        str+=sprintf(str,"%02x ", ptr[i]);
        if ((i&0x1f)==0x1f) { BDBG_LOG(("%s",str0)); str=str0; }
    }
    if (i&0x1f) { BDBG_LOG(("%s",str0)); str=str0; }
}
#define DBG_RSA BDBG_LOG
#else
#define DBG_RSA_DUMP(...)
#define DBG_RSA(...)
#endif

static RsaSageEngine_Context *
_rsaengine_search(const RSA *rsa, int pop)
{
    RsaSageEngine_Context *pSeek;

    BKNI_AcquireMutex(_g_ctx.hMutex);
    for(pSeek = BLST_D_FIRST(&_g_ctx.list); pSeek; pSeek = BLST_D_NEXT(pSeek, link))
    {
        if(pSeek->rsa == rsa)
        {
            if (pop)
            {
                BLST_D_REMOVE(&_g_ctx.list, pSeek, link);
            }
            break;
        }
    }
    BKNI_ReleaseMutex(_g_ctx.hMutex);

    return pSeek;
}

static void
_rsaengine_insert(RsaSageEngine_Context * rsaEngine)
{
    BKNI_AcquireMutex(_g_ctx.hMutex);
    BLST_D_INSERT_HEAD(&_g_ctx.list, rsaEngine, link);
    BKNI_ReleaseMutex(_g_ctx.hMutex);
 }

/**********************************************************************************
 * Link a particular index from the DRM bin file containing the SSL_CERT drm_type
 * to a particular OpenSSL RSA context.
 *    - context is the RSA context previously allocated with a call to RSA_new();
 *    - index specifies a particular index from the DRM bin file containing the
 *      SSL_CERT drm_type to link the context to
 * This API will take care of every action required in order to configure OpenSSL
 * to use the RsaSageEngine for further Sign/Verify operation on this RSA context
 * (using RSA_set_method)
 **********************************************************************************/
EVP_PKEY *
RsaSageEngine_Link(
    ENGINE* engine,
    const char* key_id,
    UI_METHOD *ui_method,
    void *callback_data)
{
    BERR_Code rc            = BERR_SUCCESS;
    uint32_t key_uint       = 0;
    RSA *rsa                = NULL;
    EVP_PKEY *evp_key       = NULL;
    uint8_t modulus[512]    = {0x00};
    uint32_t modulusLength  = 0;
    uint8_t publicExponent[4] = {0x00};

    const char *slot_string = "slot_";
    const char *id_string = "-id_";
    char *p_slot = NULL;
    char *p_id = NULL;
    char *p_id_value = NULL;
#ifdef DEBUG_OPENSSL_PRINTOUT
    BIO *out = NULL;
#endif
    RsaSageEngine_Context *rsaEngine = NULL;

    BDBG_ENTER(RsaSageEngine_Link);

    BSTD_UNUSED(ui_method);
    BSTD_UNUSED(callback_data);

    DBG_RSA(("%s(%p, %s, %p, %p)", __FUNCTION__, engine, key_id, ui_method, callback_data));

#ifdef DEBUG_OPENSSL_PRINTOUT
    out=BIO_new(BIO_s_file());
    if (out == NULL)
    {
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }
    BIO_set_fp(out,stderr,BIO_NOCLOSE);
#endif

    if(engine == NULL)
    {
        BDBG_ERR(("# %s - engine is NULL (or was not loaded).", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    if(key_id == NULL)
    {
        BDBG_ERR(("# %s - key_id is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    p_slot = strstr(key_id, slot_string);
    p_id = strstr(key_id, id_string);

    /* extract id (we don't care about the slot value for now) */
    if( (p_slot == NULL) && (p_id == NULL))
    {
        BDBG_MSG(("# %s - string format not detected, assuming simple index (i.e. '0' to '7' inclusive", __FUNCTION__));
        /* assume simple index (uint32_t) 0-7 */
        key_uint = atoi(key_id);
    }
    else
    {
        BDBG_MSG(("# %s - p_slot ='%s'   p_id = '%s'", __FUNCTION__, p_slot, p_id));
        p_id_value = p_id + strlen(id_string);
        BDBG_MSG(("# %s - key index string value = '%s'", __FUNCTION__, p_id_value));
        key_uint = atoi(p_id_value);
    }

    if(key_uint > MAX_ID_VALUE)
    {
        BDBG_ERR(("# %s - invalid index detected (%u).  Valid range is 0-7 inclusive", __FUNCTION__, key_uint));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    BDBG_MSG(("# %s - key index value = '%u'", __FUNCTION__, key_uint));

    if(engine == NULL)
    {
        BDBG_ERR(("# %s - engine pointer is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    evp_key = EVP_PKEY_new();
    if(evp_key == NULL)
    {
        BDBG_ERR(("# %s - error allocating EVP_PKEY context", __FUNCTION__));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    rsa = RSA_new();
    if(rsa == NULL)
    {
        BDBG_ERR(("# %s - error allocating RSA context", __FUNCTION__));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    /* Will fire internal RSA init callback (_init_rsa_ctx_cb) that will initialized RsaTl context */
    RSA_set_method(rsa, &_g_ctx.current_method);

    rsaEngine = _rsaengine_search(rsa, 0);
    if (rsaEngine == NULL)
    {
        BDBG_ERR(("%s: cannot found RSA %p in SAGE Engine", __FUNCTION__, rsa));
        goto ErrorExit;
    }

    rc = RsaTl_GetPublicKey(rsaEngine->hRsaTl, key_uint, modulus, &modulusLength, publicExponent);
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("# %s - error fetching public key info (index = '%u')", __FUNCTION__, key_uint));
        goto ErrorExit;
    }
    rsaEngine->drmBinFileKeyIndex = key_uint;

    rsa->n = BN_bin2bn(modulus, modulusLength, NULL);
#ifdef DEBUG_OPENSSL_PRINTOUT
    BN_print(out, rsa->n);
#endif
    /* DBG_RSA_DUMP(key_len,publicExponent); */

    rsa->e = BN_bin2bn(publicExponent, 4, rsa->e);
#ifdef DEBUG_OPENSSL_PRINTOUT
    BN_print(out, rsa->e);
#endif
    evp_key->type = EVP_PKEY_RSA;

    BDBG_MSG(("# %s - evp_key = %p, rsa = '%p'", __FUNCTION__, evp_key, rsa));

    if(EVP_PKEY_assign_RSA(evp_key, rsa) != 1)
    {
        BDBG_ERR(("# %s - error assigning RSA context to EVP_PKEY context", __FUNCTION__));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

ErrorExit:
    BDBG_LEAVE(RsaSageEngine_Link);
    return evp_key;
}


/*
 * Compliant to the OpenSSL API, the prototype for this function is:
 * Where:
 *  - the message digest m of size m_len
 *  - the signature is stored in sigret (of size in siglen).
 *  - type denotes the message digest algorithm (NID_sha1 or NID_sha256)
 *  - rsa is the context previously allocated with RSA_new();
 */
int
RsaSageEngine_Sign(
    int type,
    const unsigned char *m,
    unsigned int m_len,
    unsigned char *sigret,
    unsigned int *siglen,
    const RSA *rsa)
{
    BERR_Code rc = BERR_SUCCESS;
    int openssl_rc = 0;
    RsaSageEngine_Context *rsaEngine;

    BDBG_ENTER(RsaSageEngine_Sign);

    BSTD_UNUSED(type);

    DBG_RSA(("# Arguments for RsaSageEngine_Sign:"));
    DBG_RSA(("# type: %d", type));
    DBG_RSA(("# m_len: %i", m_len));
    DBG_RSA(("# m: 0x%08x", (uint32_t)m));
    DBG_RSA(("# Content of m:"));
    DBG_RSA_DUMP(m_len,m);
    DBG_RSA(("# *siglen: %i", *siglen));
    DBG_RSA(("# sigret: 0x%08x", (uint32_t)sigret));

    if(m == NULL)
    {
        BDBG_ERR(("# %s - Message is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    if(sigret == NULL || siglen == NULL)
    {
        BDBG_ERR(("# %s - Pointer to return signature (%p) or signature length (%p) is NULL", __FUNCTION__, sigret, siglen));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    *siglen = RSA_size(rsa);

    rsaEngine = _rsaengine_search(rsa, 0);
    if (rsaEngine == NULL)
    {
        BDBG_ERR(("%s: cannot found RSA %p in SAGE Engine", __FUNCTION__, rsa));
        goto ErrorExit;
    }

    rc = RsaTl_SignVerify(rsaEngine->hRsaTl,
                          Rsa_CommandId_eSign,
                          (uint8_t *)m,
                          (uint32_t)m_len,
                          rsaEngine->drmBinFileKeyIndex,
                          BSAGElib_Crypto_ShaVariant_eNone,
                          sigret,
                          *siglen);

    BDBG_MSG(("# %s - *siglen after SAGE operation ='%u' -> size '%u'", __FUNCTION__, *siglen, RSA_size(rsa)));

    DBG_RSA(("# *siglen: %i", *siglen));
    DBG_RSA(("# Content of sigret:"));
    DBG_RSA_DUMP(*siglen,sigret);
    DBG_RSA(("# openssl_rc: %d", openssl_rc));

ErrorExit:

    if(rc == BERR_SUCCESS){
        openssl_rc = 1;
    }

    BDBG_LEAVE(RsaSageEngine_Sign);
    return openssl_rc;
}




/*
 * Compliant to the OpenSSL RSA_verify API
 * Where:
 *  - the message digest m of size m_len
 *  - the signature is stored in sigret (of size in siglen).
 *  - type denotes the message digest algorithm (NID_sha1 or NID_sha256)
 *  - rsa is the context previously allocated with RSA_new();
 */

int
RsaSageEngine_Verify(
    int type,
    const unsigned char *m,
    unsigned int m_len,
    const unsigned char *sigret,
    unsigned int siglen,
    const RSA *rsa)
{
    BERR_Code rc = BERR_SUCCESS;
    int openssl_rc = 0;
    RsaSageEngine_Context *rsaEngine;

    BDBG_ENTER(RsaSageEngine_Verify);

    BSTD_UNUSED(type);

    DBG_RSA(("# Arguments for RsaSageEngine_Verify:"));
    DBG_RSA(("# type: %d", type));
    DBG_RSA(("# m_len: %i", m_len));
    DBG_RSA(("# m: %p", m));
    DBG_RSA(("# Content of from:"));
    DBG_RSA_DUMP(m_len,m);
    DBG_RSA(("# siglen: %i", siglen));
    DBG_RSA(("# sigret: %p", sigret));
    DBG_RSA(("# rsa: %p",rsa));

    if((m_len != OPENSSL_SHA1_SIZE) && (m_len != OPENSSL_SHA256_SIZE))
    {
        BDBG_ERR(("%s - Invalid digest message length (%u)", __FUNCTION__, m_len));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    if(m == NULL)
    {
        BDBG_ERR(("# %s - Digest message is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }
    DBG_RSA_DUMP(m_len,m);

    if(sigret == NULL)
    {
        BDBG_ERR(("# %s - Pointer to signature is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    if((unsigned int)RSA_size(rsa) != siglen)
    {
        BDBG_ERR(("# %s - RSA context key size (%u) != signature length (%u)", __FUNCTION__, RSA_size(rsa), siglen));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    rsaEngine = _rsaengine_search(rsa, 0);
    if (rsaEngine == NULL)
    {
        BDBG_ERR(("%s: cannot found RSA %p in SAGE Engine", __FUNCTION__, rsa));
        goto ErrorExit;
    }

    rc = RsaTl_SignVerify(rsaEngine->hRsaTl,
                          Rsa_CommandId_eVerify,
                          (uint8_t *)m,
                          (uint32_t) m_len,
                          rsaEngine->drmBinFileKeyIndex,
                          BSAGElib_Crypto_ShaVariant_eNone,
                          (uint8_t *)sigret,
                          (uint32_t) siglen);

    DBG_RSA(("# siglen: d", siglen));
    DBG_RSA(("# Content of sigret:"));
    DBG_RSA_DUMP(siglen,sigret);
    DBG_RSA(("# openssl_rc: %d", openssl_rc));

ErrorExit:

    if(rc == BERR_SUCCESS){
        openssl_rc = 1;
    }

    BDBG_LEAVE(RsaSageEngine_Verify);
    return openssl_rc;
}

static int
RsaSageEngine_P_EncDec(
    int flen,
    const unsigned char *from,
    int tlen,
    unsigned char *to,
    RSA *rsa,
    int padding,
    int commandId)
{
    BERR_Code rc = BERR_SUCCESS;
    int openssl_rc = 0;
    uint32_t retlen;
    RsaSageEngine_Context *rsaEngine;

    DBG_RSA(("# Arguments for RsaSageEngine_P_EncDec:"));
    DBG_RSA(("# flen: %i", flen));
    DBG_RSA(("# from: %p", from));
    DBG_RSA(("# Content of from:"));
    DBG_RSA_DUMP(flen,from);
    DBG_RSA(("# tlen: %i", tlen));
    DBG_RSA(("# to: %p", to));
    DBG_RSA(("# rsa: %p", rsa));
    DBG_RSA(("# padding: %i", padding));
    DBG_RSA(("# commandId: 0x%x", commandId));

    rsaEngine = _rsaengine_search(rsa, 0);
    if (rsaEngine == NULL)
    {
        BDBG_ERR(("%s: cannot found RSA %p in SAGE Engine", __FUNCTION__, rsa));
        goto ErrorExit;
    }

    retlen = (uint32_t)tlen;
    rc = RsaTl_EncryptDecrypt(rsaEngine->hRsaTl,
                              commandId,
                              (uint8_t *)from,
                              (uint32_t)flen,
                              to,
                              &retlen,
                              (uint32_t)padding,
                              rsaEngine->drmBinFileKeyIndex);
    if (rc == BERR_SUCCESS) {
        openssl_rc = (int)retlen;
    }

    DBG_RSA(("# to: 0x%08x", (uint32_t)to));
    DBG_RSA(("# Content of to:"));
    DBG_RSA_DUMP(tlen,to);
    DBG_RSA(("# openssl_rc: %d", openssl_rc));

ErrorExit:

    return openssl_rc;
}

int
RsaSageEngine_Private_Decrypt(
    int flen,
    const unsigned char *from,
    unsigned char *to,
    RSA *rsa,
    int padding)
{
    int openssl_rc;
    BDBG_ENTER(RsaSageEngine_Private_Decrypt);

    openssl_rc = RsaSageEngine_P_EncDec(flen, from, RSA_size(rsa), to, rsa, padding, Rsa_CommandId_ePrivateDecrypt);

    BDBG_LEAVE(RsaSageEngine_Private_Decrypt);
    return openssl_rc;
}

int
RsaSageEngine_Private_Encrypt(
    int flen,
    const unsigned char *from,
    unsigned char *to,
    RSA *rsa,
    int padding)
{
    int openssl_rc;
    BDBG_ENTER(RsaSageEngine_Private_Encrypt);

    openssl_rc = RsaSageEngine_P_EncDec(flen, from, RSA_size(rsa), to, rsa, padding, Rsa_CommandId_ePrivateEncrypt);

    BDBG_LEAVE(RsaSageEngine_Private_Encrypt);
    return openssl_rc;
}

int
RsaSageEngine_Public_Encrypt(
    int flen,
    const unsigned char *from,
    unsigned char *to,
    RSA *rsa,
    int padding)
{
    int openssl_rc;
    BDBG_ENTER(RsaSageEngine_Public_Encrypt);

    openssl_rc = RsaSageEngine_P_EncDec(flen, from, RSA_size(rsa), to, rsa, padding, Rsa_CommandId_ePublicEncrypt);

    BDBG_LEAVE(RsaSageEngine_Public_Encrypt);
    return openssl_rc;
}


int
RsaSageEngine_Public_Decrypt(
    int flen,
    const unsigned char *from,
    unsigned char *to,
    RSA *rsa,
    int padding)
{
    int openssl_rc;
    BDBG_ENTER(RsaSageEngine_Public_Decrypt);

    openssl_rc = RsaSageEngine_P_EncDec(flen, from, RSA_size(rsa), to, rsa, padding, Rsa_CommandId_ePublicDecrypt);

    BDBG_LEAVE(RsaSageEngine_Public_Decrypt);
    return openssl_rc;
}


static const ENGINE_CMD_DEFN rsa_sage_cmd_defns[] = {
    { CMD_SAGE_UTILITY_TL, "SAGE_UTILITY_TL", "Open SAGE Utility Thin Layer. Argument is DRM BinFile path.", ENGINE_CMD_FLAG_STRING },
    {0, NULL, NULL, 0}
};

static int
_rsa_sage_engine_ctrl(
    ENGINE * engine,
    int cmd,
    long i,
    void *p,
    void (*f) (void))
{
    int rc = 1;

    BSTD_UNUSED(engine);
    BSTD_UNUSED(i);
    BSTD_UNUSED(f);
    DBG_RSA(("%s(%p, %d, %ld, %p, %p)", __FUNCTION__, engine, cmd, i, p, f));

    switch(cmd) {
    case CMD_SAGE_UTILITY_TL:
            if (p) {
                size_t len = strlen(p);
                if ((len+1) > sizeof(_g_ctx.rsaSettings.drm_binfile_path)) {
                    rc = 0;
                    BDBG_ERR(("%s: path too long", __FUNCTION__));
                }
                BKNI_Memcpy(&_g_ctx.rsaSettings.drm_binfile_path, p, len);
                _g_ctx.rsaSettings.drm_binfile_path[len] = '\0';
            }
            else {
                RsaTl_GetDefaultSettings(&_g_ctx.rsaSettings);
            }
        break;
    default:
        rc = 0;
        BDBG_WRN(("%s: unsupported control command %d", __FUNCTION__, cmd));
        break;
    }
    return rc;
}

/* This internal function is used by ENGINE_pkcs11() and possibly by the
 * "dynamic" ENGINE support too */
static int _init_rsa_ctx_cb(RSA *rsa)
{
    int rc = 1;
    RsaSageEngine_Context *rsaEngine = NULL;
    RsaTl_Handle hRsaTl = NULL;

    BDBG_LOG(("SAGE Engine RSA Init(%p)", rsa));

    rc = RsaTl_Init(&hRsaTl, &_g_ctx.rsaSettings);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - Call to initialize RSA TL failed", __FUNCTION__));
        rc = 0;
        hRsaTl = NULL;
        goto ErrorExit;
    }

    rsaEngine = (RsaSageEngine_Context *)BKNI_Malloc(sizeof(*rsaEngine));
    if(rsaEngine == NULL)
    {
        BDBG_ERR(("# %s - Cannot allocate '%u' bytes of memory for Engine context",
                  __FUNCTION__, sizeof(*rsaEngine)));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    rsaEngine->drmBinFileKeyIndex = MAX_ID_VALUE;
    rsaEngine->rsa = rsa;
    rsaEngine->hRsaTl = hRsaTl;

    BDBG_MSG(("%s - RSA TL module %p initialized for RSA %p", __FUNCTION__, hRsaTl, rsa));

    hRsaTl = NULL;/* eat-up for error handling */

    _rsaengine_insert(rsaEngine);

ErrorExit:
    if (hRsaTl != NULL)
    {
        RsaTl_Uninit(hRsaTl);
    }
    return rc;
}

static int _finish_rsa_ctx_cb(RSA *rsa)
{
    RsaSageEngine_Context *rsaEngine;

    BDBG_LOG(("SAGE Engine RSA Finish(%p)", rsa));

    rsaEngine = _rsaengine_search(rsa, 1);
    if (rsaEngine == NULL)
    {
        BDBG_ERR(("%s: cannot found RSA %p in SAGE Engine", __FUNCTION__, rsa));
        goto ErrorExit;
    }

    RsaTl_Uninit(rsaEngine->hRsaTl);

    BKNI_Memset(rsaEngine, 0, sizeof(*rsaEngine));
    BKNI_Free(rsaEngine);

ErrorExit:
    return 1;
}

static int
_bind_helper(ENGINE * e)
{
    int rc = 0;
    BERR_Code magnum_rc;

    DBG_RSA(("%s(%u)", __FUNCTION__, e));

    if (_init_g_ctx != 0)
    {
        BDBG_ERR(("RSA SAGE Engine already initialized"));
        rc = -1;
        goto end_no_lock;
    }
    BKNI_Memset(&_g_ctx, 0x00, sizeof(_g_ctx));
    RsaTl_GetDefaultSettings(&_g_ctx.rsaSettings);
    magnum_rc = BKNI_CreateMutex(&_g_ctx.hMutex);
    if (magnum_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("RSA SAGE Engine already initialized"));
        rc = -2;
        goto end_no_lock;
    }
    _init_g_ctx = 1;
    BKNI_AcquireMutex(_g_ctx.hMutex);

    BLST_D_INIT(&_g_ctx.list);

    _g_ctx.current_method.name = RSA_SAGE_ENGINE;
    _g_ctx.current_method.rsa_sign     = RsaSageEngine_Sign;
    _g_ctx.current_method.rsa_verify   = RsaSageEngine_Verify;
    _g_ctx.current_method.rsa_priv_dec = RsaSageEngine_Private_Decrypt; /* decrypt with private key, remove PKCS1.5 padding */
    _g_ctx.current_method.rsa_priv_enc = RsaSageEngine_Private_Encrypt; /* encrypt with private key, with PKCS1.5 padding */
    _g_ctx.current_method.rsa_pub_dec  = RsaSageEngine_Public_Decrypt;  /* decrypt with public  key, remove PKCS1.5 padding */
    _g_ctx.current_method.rsa_pub_enc  = RsaSageEngine_Public_Encrypt;  /* encrypt with public  key, with or without padding */
    _g_ctx.current_method.init = _init_rsa_ctx_cb;
    _g_ctx.current_method.finish = _finish_rsa_ctx_cb;
    _g_ctx.current_method.flags |= RSA_FLAG_SIGN_VER;
    _g_ctx.current_method.flags |= RSA_METHOD_FLAG_NO_CHECK;

    if (!ENGINE_set_id(e, RSA_SAGE_ENGINE) ||
        !ENGINE_set_ctrl_function(e, _rsa_sage_engine_ctrl) ||
        !ENGINE_set_cmd_defns(e, rsa_sage_cmd_defns) ||
        !ENGINE_set_name(e, RSA_SAGE_ENGINE) ||
        !ENGINE_set_RSA(e, &_g_ctx.current_method) ||
        !ENGINE_set_load_pubkey_function(e, RsaSageEngine_Link) ||
        !ENGINE_set_load_privkey_function(e, RsaSageEngine_Link))
    {
        BDBG_ERR(("# %s - (%u) ENGINE_set_xyz(%p) calls failed", __FUNCTION__, __LINE__, e));
        rc = -3;
        goto end;
    }

    /* if we don't add the reference we don't see rsa_sage_engine loaded dynamically
     * (i.e. printed on the console by _display_engine_list) */
    if(!ENGINE_add(e))
    {
        BDBG_ERR(("%s - ENGINE_add(%p) failed!", __FUNCTION__, e));
        rc = -4;
        goto end;
    }

    _display_engine_list();

end:
    BKNI_ReleaseMutex(_g_ctx.hMutex);
end_no_lock:
    return rc;
}

static int
_bind_fn(
    ENGINE * e,
    const char *id)
{
    DBG_RSA(("%s(%p, %s)", __FUNCTION__, e, id));

    if (id && (strcmp(id, RSA_SAGE_ENGINE) != 0)) {
        BDBG_ERR(("# %s - --------------bad engine id='%s'", __FUNCTION__, id));
        return 0;
    }

    if (_bind_helper(e) != 0) {
        BDBG_ERR(("# %s - --------------bind failed id='%s'", __FUNCTION__, id));
        return 0;
    }

    return 1;
}

IMPLEMENT_DYNAMIC_CHECK_FN()
IMPLEMENT_DYNAMIC_BIND_FN(_bind_fn)

static void
_display_engine_list(void)
{
    ENGINE *h;
    int loop;

    h = ENGINE_get_first();
    loop = 0;
    BDBG_MSG(("# %s - listing available engine types", __FUNCTION__));
    while(h)
    {
        BDBG_MSG(("# %s - engine %i, id = \"%s\", name = \"%s\"", __FUNCTION__, loop++, ENGINE_get_id(h), ENGINE_get_name(h)));
        h = ENGINE_get_next(h);
    }

    BDBG_MSG(("# %s - end of list",__FUNCTION__));

    ENGINE_free(h);
}
