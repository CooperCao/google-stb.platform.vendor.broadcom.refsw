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
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************
/*****************************************************************************
 *
 * FILENAME: $Workfile: trunk/stack/ZbPro/SSP/include/bbZbProSsp.h $
 *
 * DESCRIPTION:
 *   Declares Security Service Provider component.
 *
 * $Revision: 2595 $
 * $Date: 2014-06-03 15:11:16Z $
 *
 ****************************************************************************************/

#ifndef _ZBPRO_SSP_H
#define _ZBPRO_SSP_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysPayload.h"
#include "bbSysQueue.h"

#include "bbSecurity.h"

#include "bbMacSapAddress.h"
#include "bbZbProSspFrameBuffer.h"

/*********************** DEFINITIONS ***************************************************/

/**//**
 * \brief Macro calculates size of Auxiliary header. Numbers were taken from ZigBee Spec r20, 4.5.1.
 */
#define ZBPRO_SSP_AUX_HEADER_SIZE_BY_PARAMS(keyIdentifier, extendedNonce) \
                (/* Security Control */ 1  + /* Frame Counter */ 4 + \
                 /* Source Address */ (extendedNonce ? 8 : 0) + \
                 /* Key Sequence number */ (ZBPRO_SSP_KEY_ID_NWK == keyIdentifier ? 1 : 0))

/**//**
 * \brief Macro calculates size of Auxiliary header. Numbers were taken from ZigBee Spec r20, 4.5.1.
 */
#define ZBPRO_SSP_AUX_HEADER_SIZE(auxiliaryHeader) \
        ZBPRO_SSP_AUX_HEADER_SIZE_BY_PARAMS((auxiliaryHeader)->securityControl.keyId, \
                                            (auxiliaryHeader)->securityControl.extNonce)

/**//**
 * \brief Maximum Size of an Auxiliary header.
 */
#define ZBPRO_SSP_AUX_HEADER_MAX_SIZE  ZBPRO_SSP_AUX_HEADER_SIZE_BY_PARAMS(ZBPRO_SSP_KEY_ID_NWK, true)

/**//**
 * \brief Minimum Size of an Auxiliary header.
 */
#define ZBPRO_SSP_AUX_HEADER_MIN_SIZE  ZBPRO_SSP_AUX_HEADER_SIZE_BY_PARAMS(ZBPRO_SSP_KEY_ID_DATA, false)

/**//**
 * \brief Size of an CCM* Nonce. See ZigBee Spec r20, Figure 4.25.
 */
#define ZBPRO_SSP_NONCE_SIZE  (/* Extended Source Address */ 8 + \
                               /* Frame Counter */ 4 +\
                               /* Security Control */ 1)

/**//**
 * \brief Calculates MIC size. ZigBee Spec r20, Table 4.40.
 */
#define ZBPRO_SSP_MIC_SIZE(securityLevel)       (2 << (securityLevel & 0x03))

/**//**
 * \brief MIC maximum size.
 */
#define ZBPRO_SSP_MIC_MAX_SIZE                  ZBPRO_SSP_MIC_SIZE(ZBPRO_SSP_SECURITY_LEVEL_MIC_128)

/**//**
 * \brief MIC minimum size.
 */
#define ZBPRO_SSP_MIC_MIN_SIZE                  ZBPRO_SSP_MIC_SIZE(ZBPRO_SSP_SECURITY_LEVEL_MIC_32)

/**//**
 * \brief Checks the key for "validness". It is assumed that a key consisted of only zeros is invalid.
 */
#define ZBPRO_SSP_IS_KEY_VALID(key) ((key).raw32[0] || (key).raw32[1] || (key).raw32[2] || (key).raw32[3])
/************************* TYPES *******************************************************/

/**//**
 * Security Level enumeration. See ZigBee Spec r20, Table 4.40.
 */
typedef enum _ZbProSspSecurityLevel_t
{
    ZBPRO_SSP_SECURITY_LEVEL_NONE               = 0,
    ZBPRO_SSP_SECURITY_LEVEL_MIC_32             = 1,
    ZBPRO_SSP_SECURITY_LEVEL_MIC_64             = 2,
    ZBPRO_SSP_SECURITY_LEVEL_MIC_128            = 3,
    ZBPRO_SSP_SECURITY_LEVEL_ENC                = 4,
    ZBPRO_SSP_SECURITY_LEVEL_ENC_MIC_32         = 5,
    ZBPRO_SSP_SECURITY_LEVEL_ENC_MIC_64         = 6,
    ZBPRO_SSP_SECURITY_LEVEL_ENC_MIC_128         = 7,
    /* TODO */
} ZbProSspSecurityLevel_t;

/**//**
 * \brief Security Key Identifier type. See ZigBee Spec r20, Table 4.41.
 */
typedef enum _ZbProSspKeyId_t
{
    ZBPRO_SSP_KEY_ID_DATA                       = 0,
    ZBPRO_SSP_KEY_ID_NWK                        = 1,
    ZBPRO_SSP_KEY_ID_TRANSPORT                  = 2,
    ZBPRO_SSP_KEY_ID_LOAD                       = 3
} ZbProSspKeyId_t;

/**//**
 * \brief Security Frame Counter type.
 */
typedef uint32_t ZbProSspFrameCnt_t;

/**//**
 * \brief Invalid Frame Counter value.
 */
#define ZBPRO_SSP_INVALID_FRAME_COUNTER         UINT32_MAX

/**//**
 * \brief Network Key sequence number.
 */
typedef uint8_t ZbProSspNwkKeySeqNum_t;

/**//**
 * \brief Security Control field of the Aux header. See ZigBee Spec r20, 4.5.1.1.
 */
typedef union _ZbProSspAuxControl_t
{
    struct
    {
        BitField8_t securityLevel   : 3;
        BitField8_t keyId           : 2;
        BitField8_t extNonce        : 1;
        BitField8_t reserved        : 2;
    };
    uint8_t         raw;
} ZbProSspAuxControl_t;

/**//**
 * \brief Auxiliary header structure. See ZigBee Spec r20, 4.5.1.
 */
typedef struct _ZbProSspAuxHeader_t
{
    ZbProSspAuxControl_t    securityControl;    /*!< The security control field shall consist of a security level,
                                                     a key identifier, and an extended nonce sub-field.
                                                     See ZbProSspAuxControl_t structure. */
    ZbProSspFrameCnt_t      frameCounter;       /*!< The counter field is used to provide frame freshness and to
                                                     prevent processing of duplicate frames. */
    MAC_Addr64bit_t         extSrcAddr;         /*!< The source address field shall only be present when the
                                                     extended nonce sub-field of the security control field is 1. */
    ZbProSspNwkKeySeqNum_t  keySeqNumber;       /*!< The key sequence number field shall only be present when the
                                                     key identifier subfield of the security control field is 1
                                                     (that is, a network key). */
} ZbProSspAuxHeader_t;

/**//**
 * \brief Nonce type declaration. See ZigBee Spec r20, 4.5.2.2.
 */
typedef struct _ZbProSspNonce_t
{
    MAC_Addr64bit_t         extSrcAddr;
    ZbProSspFrameCnt_t      frameCounter;
    ZbProSspAuxControl_t    securityControl;
} ZbProSspNonce_t;

/**//**
 * \brief Security Key type.
 */
typedef union _ZbProSspKey_t
{
    uint8_t     raw[16];
    uint32_t    raw32[4];
} ZbProSspKey_t;

/**//**
 * \brief Encrypt request type declaration
 */
typedef struct _ZbProSspEncryptReq_t ZbProSspEncryptReq_t;

/**//**
 * \brief Encrypt request confirmation callback function type
 */
typedef void ZbProSspEncryptReqCb_t(ZbProSspEncryptReq_t *req);

/**//**
 * \brief Encrypt request type
 */
typedef struct _ZbProSspEncryptReq_t
{
    Security_CCMReq_t       ccmReq;
    ZbProSspFrameBuffer_t   *frameBuffer;
    ZbProSspEncryptReqCb_t  *confCb;
} ZbProSspEncryptReq_t;

/**//**
 * \brief Decrypt request type declaration
 */
typedef struct _ZbProSspDecryptReq_t ZbProSspDecryptReq_t;

/**//**
 * \brief Decrypt request confirmation callback function type
 */
typedef void ZbProSspDecryptReqCb_t(ZbProSspDecryptReq_t *req, bool success);

/**//**
 * \brief Decrypt request type
 */
typedef struct _ZbProSspDecryptReq_t
{
    Security_CCMReq_t       ccmReq;
    ZbProSspFrameBuffer_t   *frameBuffer;
    SYS_DataPointer_t       forSplit;       /* if mic isn't specified (decrypting of incoming frame),
                                               it points to reserved Memory Block for splitting */
    ZbProSspDecryptReqCb_t  *confCb;
} ZbProSspDecryptReq_t;

/************************* PROTOTYPES **************************************************/

/**//**
 * \brief Constructs Auxiliary header.
 */
void zbProSspMakeAuxHeader(ZbProSspKeyId_t keyId,
                           ZbProSspFrameCnt_t frameCounter,
                           bool extNonce,
                           uint8_t keySeqNum,
                           ZbProSspAuxHeader_t *parsedAuxHeader);

/**//**
 * \brief Deserializes Auxiliary header.
 */
bool zbProSspDeserializeAuxHeader(ZbProSspAuxHeader_t *const parsedAuxHeader,
                                  SYS_DataPointer_t auxHeader);

/**//**
 * \brief Detaches an Auxiliary frame header from the remaining part of the frame
 */
bool zbProSspDetachParseAuxHeader(ZbProSspFrameBuffer_t *frameBuffer,
                                  ZbProSspAuxHeader_t *parsedAuxHeader);

/**//**
 * \brief Encrypt request
 */
void ZbProSspEncryptReq(ZbProSspEncryptReq_t *const req, ZbProSspAuxHeader_t *const parsedAuxHeader, const ZbProSspKey_t *const key);

/**//**
 * \brief Decrypt request
 */
void ZbProSspDecryptReq(ZbProSspDecryptReq_t *const req, ZbProSspAuxHeader_t *const parsedAuxHeader, const ZbProSspKey_t *const key);

/**//**
 * \brief Generates a random key
 */
void zbProSspGenerateKey(ZbProSspKey_t *keyBuffer);

#endif /* _ZBPRO_SSP_H */