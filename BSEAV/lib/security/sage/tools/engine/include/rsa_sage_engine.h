/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef RSA_SAGE_ENGINE_H__
#define RSA_SAGE_ENGINE_H__

#include "bstd.h"
#include "bkni.h"

#include <openssl/rsa.h>
#include <openssl/obj_mac.h>
#include <openssl/engine.h>
#include <openssl/ssl.h>

/************************************************************************************
 * Link a particular index (found in a DRM bin file containing the SSL_CERT drm_type)
 * to a particular OpenSSL EVP_PKEY context.
 ************************************************************************************/
EVP_PKEY *RsaSageEngine_Link(ENGINE* engine, const char* key_id, UI_METHOD *ui_method, void *callback_data);


/*****************************************************************
 * RsaSageEngine_Unlink
 * Unlink (disassociate) a context with its index
 * Performs the clean-up of the context/index list management
 *****************************************************************/
void RsaSageEngine_Unlink(EVP_PKEY *context);


/*****************************************************************
 * Compliant to the OpenSSL API, the prototype for this function is:
 * Where:
 *  - the message digest m of size m_len
 *  - the signature is stored in sigret (of size in siglen).
 *  - type denotes the message digest algorithm (NID_sha1 or NID_sha256)
 *  - rsa is the context previously allocated with RSA_new();
 *****************************************************************/
int RsaSageEngine_Sign(int type, const unsigned char *m, unsigned int m_len,
                       unsigned char *sigret, unsigned int *siglen, const RSA *rsa);


/*****************************************************************
 * Compliant to the OpenSSL API, the prototype for this function is:
 * Where:
 *  - the message digest m of size m_len
 *  - the signature is stored in sigret (of size in siglen).
 *  - type denotes the message digest algorithm (NID_sha1 or NID_sha256)
 *  - rsa is the context previously allocated with RSA_new();
 *****************************************************************/
int RsaSageEngine_Verify(int type, const unsigned char *m, unsigned int m_len,
                         const unsigned char *sigret, unsigned int siglen, const RSA *rsa);

int RsaSageEngine_Private_Decrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding);
int RsaSageEngine_Public_Encrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding);
int	RsaSageEngine_Private_Encrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa,int padding);
int	RsaSageEngine_Public_Decrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa,int padding);


#endif /*RSA_SAGE_ENGINE_H__*/
