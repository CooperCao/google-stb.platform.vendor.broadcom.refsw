/******************************************************************************
 *    (c)2009-2015 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
#ifndef B_ECC_WRAPPER_H
#define B_ECC_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char gBn160[SHA1_DIGEST_SIZE];

/*! \brief initialize ECC parameters and key parameters, convert from binary to string.
 *  \param[in] ghBcrypt bcrypt handle
 *  \param[in,out] pEccParams pointer to ECC parameters.
 *  \retval BCRYPT_STATUS_eOk or other error code.
 */
BERR_Code B_DTCP_InitEccParams(DRM_DtcpIpTlHandle hDtcpIpTl, BCRYPT_ECCParam_t * pEccParams);
/* !\brief Get RNG seed, in iRngSeedSizeInBytes length.
 */
void B_GetRand(char *pRngSeed, int iRngSeedSizeInBytes);

/*! \clean up ecc parameters , free resources
 */
void B_DTCP_CleanupEccParams(BCRYPT_ECCParam_t * pEccParams);

/*! \brief big number modulo addition, compute a+b mod m and place result in r.
 */
BERR_Code B_ModAdd(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * r, unsigned char * a, unsigned char * b, unsigned char * m,
        int size_a, int size_b, int size_m);

/*! \brief compute MAC value for RTT procedure.
 *  \param[in] AuthKey authentication key.
 *  \param[in] RttN Rtt trial counter.
 *  \param[out] MacValue computed MAC value
 *  \retval BCRYPT_STATUS_eOK or other error code.
 */
BERR_Code B_DTCP_IP_ComputeRttMac(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * AuthKey, unsigned char * RttN, unsigned char * MacValue);

BERR_Code B_DTCP_IP_ComputeRttMac_2(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * AuthKey,
        unsigned char * RttN,
        int RttN_sz,
        unsigned char * MacValue);

/*! \brief generate RNG of length len.
 */
#if 0
BCRYPT_STATUS_eCode B_RNG(BCRYPT_Handle ghBcrypt, unsigned char * r, int len);
#else
BERR_Code B_RNG(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * r, int len);
#endif

/*! \brief generate RNG of length len, less then max (exclude max).
 */
#if 0
BCRYPT_STATUS_eCode B_RNG_max(BCRYPT_Handle ghBcrypt, unsigned char * r, unsigned char *max, int len);
#else
BERR_Code B_RNG_max(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * r, unsigned char *max, int len);
#endif
/*
 * Generate random number in range (0, max), exclude max using CommonDRM -> HW random number gererator.
 */
BERR_Code B_RNG_max_common_drm(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * r, unsigned char * max, int len);

/*! \brief get DTCP EC-DH first phase value.
 *  \param[out] pXv EC-DH first phase value.
 *  \param[out] pXk Secret information, random number.
 *  \param[in] EccParams ECC parameters.
 *  \retval BCRYPT_STATUS_eOK or other error code.
 */
BERR_Code B_DTCP_GetFirstPhaseValue(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * pXv, unsigned char * pXk) ;

/*! \brief Get EC-DSA shared secret (Xk*Yv), where Yv is other device's EC-DH first phase value.
 *  \param[out] pKauth shared secret (Authentication key)
 *  \param[in]  pXk secret information.
 *  \param[in]  pYv other device's EC-DH first phase value.
 *  \param[in]  EccParams Ecc parameters.
 */
BERR_Code B_DTCP_GetSharedSecret(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * pKauth, unsigned char * pXk,
        unsigned char *pYv);

/*! \brief sign data using EC-DSA algorithm.
 *  \param[out] pSignature computed signature,
 *  \param[in]  pBuffer input data to be signed.
 *  \param[in]  len input data length.
 *  \param[in]  pKey input private key.
 *  \param[in]  EccParams input ECC parameters.
 */
BERR_Code B_DTCP_SignData(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * pSignature, unsigned char * pBuffer,
        int len);

/* \brief same as above but sign data using a binary key.
 */
BERR_Code B_DTCP_SignData_BinKey(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * pSignature, unsigned char * pBuffer,
        int len);

/*! \brief Verify data using EC-DSA algorithm.
 *  \param[out] valid 1 , signature is valid, 0 signature is invalid.
 *  \param[in] pSignature, input signature,
 *  \param[in] pBuffer input data to be verified.
 *  \param[in] len input data length.
 *  \param[in] PublicKeyX public key x component.
 *  \param[in] PublicKeyY public key y component
 *  \param[in] EccParams ECC parameters.
 */
BERR_Code B_DTCP_VerifyData(DRM_DtcpIpTlHandle hDtcpIpTl, int *valid, unsigned char * pSignature, unsigned char * pBuffer,
        int len, char * PublicKeyX, char * PublicKeyY, BCRYPT_ECCParam_t * EccParams);

/* Verify data, same as above, but using a binary public key(combined x and y components) .
 */
BERR_Code B_DTCP_VerifyData_BinKey(DRM_DtcpIpTlHandle hDtcpIpTl, int *valid, unsigned char * pSignature, unsigned char * pBuffer,
        int len, unsigned char * BinKey);

BERR_Code B_DTCP_GenDtcpCert(DRM_DtcpIpTlHandle hDtcpIpTl, BCRYPT_ECCParam_t *pEccParams,
        unsigned char *pDtlaPrivateKey, unsigned char *pDtlaPublicKey, unsigned char *pPrivateKey,
        unsigned char *pCert, int OverideDevID, char *devid);
#ifdef __cplusplus
}
#endif

#endif /* B_ECC_WRAPPER_H */
