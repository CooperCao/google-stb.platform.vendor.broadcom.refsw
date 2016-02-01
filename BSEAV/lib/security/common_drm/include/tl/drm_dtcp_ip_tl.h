/******************************************************************************
 *    (c)2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *
 *****************************************************************************/

#ifndef DRM_DTCP_IP_TL_H_
#define DRM_DTCP_IP_TL_H_

#include "drm_types.h"
#include "nexus_security_datatypes.h"
#include "nexus_dma.h"
#include "nexus_security.h"

#ifdef __cplusplus
extern "C" {
#endif
#define DTCP_DEVICE_KEY_SIZE                             8   /*!< Size of device key in bytes */
#define DTCP_BASELINE_FULL_CERT_SIZE                     88  /*!< Size of baseline full-auth device certificate */
#define DTCP_EXTENDED_FULL_CERT_SIZE                     132 /*!< size of extended full-auth  device certificate */
#define DTCP_RESTRICTED_CERT_SIZE                        48  /*!< size of restricted auth device certificate */

#define DTCP_DEVICE_ID_SIZE                              5   /*!< size of device id */
#define DTCP_PUBLIC_KEY_SIZE                             40  /*!< size of public key */
#define DTCP_PRIVATE_KEY_SIZE                            20  /*!< private key size */
#define DTCP_SIGNATURE_SIZE                              40  /*!< EC-DSA signature size */

/* AKE data sizes */

#define DTCP_FULL_AUTH_NONCE_SIZE                        16 /*!< Size of the random nonce for full auth */
#define DTCP_RESTRICTED_AUTH_NONCE_SIZE                  8  /*!< Size of the random nonce for restricted auth */
#define DTCP_RESTRICTED_AUTH_RESPONSE_SIZE               8  /*!< Size of the response for restricted auth */
#define DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE                40 /*!< Size of the Diffie-Hellman first phase value */
#define DTCP_DH_FIRST_PHASE_SECRET_SIZE                  20 /*!< Size of the Diffie-Hellman first phase secret */
#define DTCP_AUTH_KEY_SIZE                               12 /*!< Size of the authentication key */
#define DTCP_RESTRICTED_AUTH_KEY_SIZE                    8  /*!< Size of the restrcited auth authentication key */
#define DTCP_EXCHANGE_KEY_SIZE                           12 /*!< Size of the exchange key */
#define DTCP_CONTENT_KEY_NONCE_SIZE                      8  /*!< Size of the nonce for computing the content key */

#define DTCP_SRM_VERSION_NUMBER_SIZE                     2  /*!< Size of the SRM version number */
#define DTCP_SRM_GENERATION_SIZE                         1  /*!< size of SRM generation in challenge response */
#define DTCP_SRM_HEADER_SIZE                             4  /*!< size of SRM header.*/
#define DTCP_SRM_FIRST_GEN_MAX_SIZE                     128 /*!< first generation SRM max size */
#define DTCP_SRM_SECOND_GEN_MAX_SIZE                    1024    /*!< Second generation SRM max size */
#define DTCP_EXCHANGE_KEY_LABEL_SIZE                     1  /*!< Size of the exchange key label */
#define DTCP_CONTENT_KEY_CONSTANT_SIZE                   12 /*!< Size of the content key constant */
#define DTCP_IP_CONTENT_KEY_SIZE                         16 /*!< Size of the content key for IP */

/* Sink capbilities for RESP2 */
#define DTCP_DEVICE_CAPABILITY_SIZE                      4  /*!< Size of the device capability for IP */

#define DTCP_FULL_AUTH_CHALLENGE_SIZE                   (DTCP_FULL_AUTH_NONCE_SIZE + DTCP_BASELINE_FULL_CERT_SIZE) /*!< Size of a full auth challenge */

#define DTCP_FULL_AUTH_RESPONSE_SIZE                    (DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + \
                                                         DTCP_SRM_VERSION_NUMBER_SIZE + \
                                                         DTCP_SRM_GENERATION_SIZE + \
                                                         DTCP_SIGNATURE_SIZE)                           /*!< Size of a full auth response */
#define DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE             (DTCP_RESTRICTED_AUTH_NONCE_SIZE + 2 )  /*!< Size of a restricted auth challenge ( nonce + 12 bits SRM version + 4 bits padding) */
#define DTCP_RESTRICTED_AUTH_RESPONSE_SIZE              8
#define DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE    (DTCP_RESTRICTED_CERT_SIZE + DTCP_RESTRICTED_AUTH_NONCE_SIZE)

#define DTCP_FULL_AUTH_RESPONSE2_SIZE                   (DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + \
                                                         DTCP_SRM_VERSION_NUMBER_SIZE + 1 + \
                                                         DTCP_DEVICE_CAPABILITY_SIZE + \
                                                         DTCP_DEVICE_ID_SIZE + \
                                                         DTCP_SIGNATURE_SIZE)                       /*!< Size of a enhanced restricted auth challenge>*/
#define DTCP_EXTENDED_FULL_AUTH_CHALLENGE_SIZE          (DTCP_FULL_AUTH_NONCE_SIZE + DTCP_EXTENDED_FULL_CERT_SIZE)    /*!<size of extended full-auth challenge*/

#define DTCP_CAPABILITY_REQ_SIZE                        (4)                                     /*!< capability request command */

#define DTCP_EXCHANGE_KEY_CMD_DATA_SIZE                 (DTCP_EXCHANGE_KEY_SIZE + 2)            /*!< Size of a exchange key command( lable + cipher_algorithm + key ) */
#define DTCP_CONTENT_KEY_REQUEST_SIZE                   (DTCP_EXCHANGE_KEY_LABEL_SIZE + 3 + DTCP_CONTENT_KEY_NONCE_SIZE) /*!< Size of a content key request */


#define DTCP_SRM_FIRST_GEN_MAX_SIZE                 128     /*!< Maximum size of a first generation SRM */
#define DTCP_SRM_SECOND_GEN_MAX_SIZE                1024    /*!< Maximum size of a second generation SRM */
#define DTCP_SRM_MAX_SIZE                           DTCP_SRM_SECOND_GEN_MAX_SIZE                /*!< Maximum SRM size*/
#define DTCP_SRM_CRL_MIN_SIZE                       (DTCP_SRM_CRL_LENGTH_SIZE + DTCP_SIGNATURE_SIZE) /*!< Minimum SRM CRL size*/
/*!< Size of the full auth response buffer that is signed */
#define DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE    (DTCP_FULL_AUTH_NONCE_SIZE + \
                                                     DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + \
                                                     DTCP_SRM_VERSION_NUMBER_SIZE + \
                                                     1)
/*!< Size of the full auth response buffer that is signed */
#define DTCP_FULL_AUTH_RESPONSE2_SIGN_BUFFER_SIZE   (DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE + \
                                                     DTCP_DEVICE_CAPABILITY_SIZE + \
                                                     DTCP_DEVICE_ID_SIZE)

#define DTCP_CONTENT_PACKET_HEADER_SIZE             14 /*!< Size of a protected content packet header */
#define DTCP_AES_KEY_SIZE                           16 /*!< Size of an AES key */
#define DTCP_AES_IV_SIZE                            16 /*!< Size of an AES IV */
#define DTCP_AES_BLOCK_SIZE                         16 /*!< Size of an AES block */
#define DTCP_MAXIMUM_PROTECTED_PACKET_SIZE          134217728   /*!< Maximum payload size of a protected content packet */
#define DTCP_SINK_COUNT_LIMIT                       32  /*!< Sink count limit*/

#define DTCP_RTT_N_SIZE                             2                       /*!< Size of the RTT Trials counter */
#define DTCP_RTT_MAC_SIZE                           10                      /*!< Size of the RTT MAC */
#define DTCP_RTT_SETUP_SIZE                         (DTCP_RTT_N_SIZE)       /*!< Size of RTT setup command */
#define DTCP_RTT_TEST_SIZE                          (DTCP_RTT_MAC_SIZE)     /*!< Size of a RTT Test command*/
#define DTCP_RTT_VERIFY_SIZE                        (DTCP_RTT_MAC_SIZE)     /*!< Size of a RTT Verify command */
#define DTCP_CONT_KEY_CONF_MAC_SIZE                 10                      /*!< Size of the Cont Key Conf MAC */

#define DTCP_CONT_KEY_CONF_R_SIZE                   8                       /*!< Size of the Cont Key Conf R value */

#define DTCP_CONT_KEY_CONF_SIZE                     (DTCP_CONTENT_KEY_NONCE_SIZE + DTCP_CONT_KEY_CONF_R_SIZE + DTCP_RTT_MAC_SIZE)   /*!< Size of Cont Key Conf command */
#define SHA1_DIGEST_SIZE                            20                      /*!< Size of the SHA1 digest*/

#define DTCP_RTT_MK_SIZE                            (SHA1_DIGEST_SIZE)      /*!< Size of MK value */
#define DTCP_RTT_MAC_DATA_SIZE                      (SHA1_DIGEST_SIZE)      /*!< Number of bits in the RTT MAC value */
#define DTCP_RTT_MK_DATA_SIZE                       (DTCP_AUTH_KEY_SIZE + DTCP_AUTH_KEY_SIZE) /*!< Size of the RTT MK value */
#define DTCP_RTT_MACAB_SIZE                         (DTCP_RTT_MAC_SIZE + DTCP_RTT_MAC_SIZE)   /*!< Size of MAC1A + MAC1B */

#define DTCP_RTT_MAX_RETRIES                        (1024)                  /*!< Maxium number of RTT retries */
#define DTCP_RTT_MAX_RTT_MS                         (7)                     /*!< Maxium RTT in milliseconds */

#define DTCP_CONT_KEY_CONF_MX_SIZE                  (SHA1_DIGEST_SIZE)
#define DTCP_CONT_KEY_CONF_MAC_DATA_SIZE            (DTCP_CONT_KEY_CONF_MX_SIZE)
#define DTCP_IV_CONSTANT_SIZE                       8



/* DTCP-IP Parameter Settings structure */
typedef struct DrmDtcpIpTlParamSettings_t
{
    DrmCommonInit_t drmCommonInit;
    NEXUS_KeySlotHandle encryptKeyHandle;
    NEXUS_KeySlotHandle decryptKeyHandle;
    bool use_external_keys;
}DrmDtcpIpTlParamSettings_t;


typedef struct DRM_DrmDtcpIpTl_P_Context_t *DRM_DtcpIpTlHandle;

/******************************************************************************
 FUNCTION:
  DRM_DtcpIpTl_Initialize

 DESCRIPTION:
   Must be called only once prior to any other module API call.

 PARAMETERS:
    N/A

******************************************************************************/
DrmRC DRM_DtcpIpTl_Initialize(char *key_file, uint32_t mode, DRM_DtcpIpTlHandle *hDtcpIpTl);


/******************************************************************************
 FUNCTION:
  DRM_DtcpIpTl_Finalize

 DESCRIPTION:
   Must be called only once prior to any other module API call.

 PARAMETERS:
    N/A

******************************************************************************/
void DRM_DtcpIpTl_Finalize(DRM_DtcpIpTlHandle hDtcpIpTl);

/*! \brief generate RNG of length len.
 */
DrmRC DRM_DtcpIpTl_GetRNG(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *r,
    uint32_t len
    );

/*! \brief generate RNG of length len, less then max (exclude max).
 */
DrmRC DRM_DtcpIpTl_GetRNGMax(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *r,
    uint8_t *max,
    uint32_t len
    );


/*! \brief
   Get the Public Cert and device public key.
 */
DrmRC DRM_DtcpIpTl_GetDeviceCertificate(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *cert,
    uint32_t certLength,
    uint8_t *dtlaPublicKey
    );

/*! \brief big number modulo addition,
  compute a+b mod m and place result in r.
 */
DrmRC DRM_DtcpIpTl_ModAdd(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * r,
    uint8_t * a,
    uint8_t * b,
    uint8_t * m,
    uint32_t size_a,
    uint32_t size_b,
    uint32_t size_m
    );

/*! \brief compute MAC value for RTT procedure.
 *  \param[in] AuthKey authentication key.
 *  \param[in] RttN Rtt trial counter.
 *  \param[out] MacValue computed MAC value
 */
DrmRC DRM_DtcpIpTl_ComputeRttMac(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * AuthKey,
    uint8_t * RttN,
    uint8_t * MacValue
    );

/*! \brief compute MAC value for RTT procedure.
 *  \param[in] AuthKey authentication key.
 *  \param[in] RttN Rtt trial counter.
 *  \param[in] RttN_sz
 *  \param[out] MacValue computed MAC value
 */
DrmRC DRM_DtcpIpTl_ComputeRttMac_Alt(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * AuthKey,
    uint8_t * RttN,
    uint32_t RttN_sz,
    uint8_t * MacValue
    );

/*! brief check two number addtion overflow
 * [in] a
 * [in] b
 * [in] size
 * [out] retVal
 */
DrmRC DRM_DtcpIpTl_CheckOverFlow(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * a,
    uint8_t * b,
    uint32_t size,
    bool *retVal
    );

/*! \brief get DTCP EC-DH first phase value.
 *  \param[out] pXv EC-DH first phase value.
 *  \param[out] pXk Secret information, random number.
 */
DrmRC DRM_DtcpIpTl_GetFirstPhaseValue(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * pXv,
    uint8_t * pXk
    );

/*! \brief Get EC-DSA shared secret (Xk*Yv), where Yv is other device's EC-DH first phase value.
 *  \param[out] pKauth shared secret (Authentication key)
 *  \param[in]  pXk secret information.
 *  \param[in]  pYv other device's EC-DH first phase value.
 */
DrmRC DRM_DtcpIpTl_GetSharedSecret(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * pKauth,
    uint8_t * pXk,
    uint8_t *pYv
    );

/*! \brief sign data using EC-DSA algorithm.
 *  \param[out] pSignature computed signature,
 *  \param[in]  pBuffer input data to be signed.
 *  \param[in]  len input data length.
 */
DrmRC DRM_DtcpIpTl_SignData_BinKey(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * pSignature,
    uint8_t * pBuffer,
    uint32_t len
    );

/*! \brief Verify data using EC-DSA algorithm.
 *  \param[out] valid 1 , signature is valid, 0 signature is invalid.
 *  \param[in] pSignature, input signature,
 *  \param[in] pBuffer input data to be verified.
 *  \param[in] len input data length.
 *  \param[in] Binary public key .
 */
DrmRC DRM_DtcpIpTl_VerifyData_BinKey(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint32_t *valid,
    uint8_t * pSignature,
    uint8_t * pBuffer,
    uint32_t len,
    uint8_t * BinKey
    );


/*! \brief Create the Content key for Encrytion/Decryption and load it into keyslotID
  aCipherIv and aCipherKey are the unused output params, keeping them in the interface in case
  we need to bring the Key and IV to host for testing and debugging in future.
  */
DrmRC DRM_DtcpIpTl_CreateContentKey(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint32_t aExtendedEmi,
    uint8_t *aExchangeKey,
    uint8_t *aNonce,
    uint8_t *aCipherKey,
    uint8_t *aCipherIv,
    uint32_t keyslotID
    );


DrmRC DtcpIpTl_EncDecOperation(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *pSrc,
    uint32_t src_length,
    uint8_t *pDst,
    NEXUS_KeySlotHandle dtcpIpKeyHandle,
    bool scatterGatherStart,
    bool scatterGatherEnd,
    NEXUS_DmaHandle dtcpIpDmaHandle
    );

DrmRC DRM_DtcpIpTl_UpdateKeyIv(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *keyIv,
    uint32_t keyslotID
    );

DrmRC DRM_DtcpIpTl_FreeKeySlot(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint32_t keyslotID
    );
#ifdef __cplusplus
}
#endif

#endif /* DRM_DTCP_IP_TL_H_ */
