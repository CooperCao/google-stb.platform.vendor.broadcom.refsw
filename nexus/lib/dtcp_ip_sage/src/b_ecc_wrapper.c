/********************************************************************************************
*     (c)2004-2015 Broadcom Corporation                                                     *
*                                                                                           *
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,   *
*  and may only be used, duplicated, modified or distributed pursuant to the terms and      *
*  conditions of a separate, written license agreement executed between you and Broadcom    *
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants*
*  no license (express or implied), right to use, or waiver of any kind with respect to the *
*  Software, and Broadcom expressly reserves all rights in and to the Software and all      *
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU       *
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY                    *
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.                                 *
*
*  Except as expressly set forth in the Authorized License,                                 *
*
*  1.     This program, including its structure, sequence and organization, constitutes     *
*  the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to      *
*  protect the confidentiality thereof,and to use this information only in connection       *
*  with your use of Broadcom integrated circuit products.                                   *
*                                                                                           *
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"          *
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR                   *
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO            *
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES            *
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,            *
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION             *
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF              *
*  USE OR PERFORMANCE OF THE SOFTWARE.                                                      *
*                                                                                           *
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS         *
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR             *
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR               *
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF             *
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT              *
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE            *
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF              *
*  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *    DTCP Ecliptic Curve Algorithm wrappers.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************************************/
/*! \file b_ecc_wrapper.c
 *  \brief wrapper for Ecliptic curve Algorithm.
 */
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_status_codes.h"
#include "b_dtcp_ake.h"
#include "bcrypt_sha1_sw.h"

#include "openssl/bn.h"
#include "openssl/bio.h"
#include "openssl/rand.h"
#include "openssl/crypto.h"

#include "nexus_base_types.h"
#include "nexus_random_number.h"
#include "drm_common.h"
#include "drm_types.h"

BDBG_MODULE(b_dtcp_ip);
extern unsigned char gEccCoefficientA[];
extern unsigned char gEccCoefficientB[];
extern unsigned char gEccPrimeField[];
extern unsigned char gEccBasePointX[];
extern unsigned char gEccBasePointY[];
extern unsigned char gEccBasePointOrder[];
extern char gRngSeed[];
extern int gRngSeedSize;

unsigned char gBn160[SHA1_DIGEST_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*! \brief perform modulo 2^n addtion
 *  \param[in,out] r result.
 *  \param[in] a input
 *  \param[in] b input
 *  \param[in] m input modulo.
 *  \param[in] size size of the input/output paramaeters.
 */
BERR_Code B_ModAdd(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * r, unsigned char * a, unsigned char * b, unsigned char * m,
        int size_a, int size_b, int size_m)
{
    BERR_Code retValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_ModAdd(hDtcpIpTl, r, a, b, m, size_a, size_b, size_m);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_ModAdd() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }

    return retValue;
}
/*! brief  Utility function to compute RTT mac value(also used for OkMsg)
 *  param[in] AuthKey dtcp authentication key pointer.
 *  param[in] RttN RTT N value pointer.
 */
BERR_Code B_DTCP_IP_ComputeRttMac( DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * AuthKey,
        unsigned char * RttN,
        unsigned char * MacValue)
{
    BERR_Code retValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_ComputeRttMac(hDtcpIpTl, AuthKey, RttN, MacValue);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_ComputeRttMac() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }

    return BERR_SUCCESS;
}
/*! brief check two number addtion overflow
 *
 */
bool B_DTCP_IP_CheckOverFlow(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * a, unsigned char * b, int size)
{
    DrmRC rc = Drm_Err;
    bool retValue = false;
    rc = DRM_DtcpIpTl_CheckOverFlow(hDtcpIpTl, a, b, size, &retValue);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_CheckOverFlow() failed\n",__FUNCTION__,__LINE__));
    }
    return retValue;
}
/*! brief  Utility function to compute RTT mac value for CKC
 *  param[in] AuthKey dtcp authentication key pointer.
 *  param[in] RttN RTT N value pointer.
 */
BERR_Code B_DTCP_IP_ComputeRttMac_2( DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * AuthKey,
        unsigned char * RttN,
        int RttN_sz,
        unsigned char * MacValue)
{
    BERR_Code retValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_ComputeRttMac_Alt(hDtcpIpTl, AuthKey, RttN, RttN_sz, MacValue);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_ComputeRttMac_Alt() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }

    return BERR_SUCCESS;
}

/* Get RNG seed */
void B_GetRand(char *pRngSeed, int iRngSeedSizeInBytes)
{
    int i;

    srand((unsigned)time(NULL));

    for (i = 0; i < (iRngSeedSizeInBytes >> 2); i++)
    {
        pRngSeed[i] = rand();
    }
}

/* Generate rondom number */
BERR_Code B_RNG(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * r, int len)
{
    BERR_Code retValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_GetRNG(hDtcpIpTl, r, len);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_GetRNG() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }
    return retValue;
}
/*
 * Generate random number in range (0, max), exclude max.
 */
BERR_Code B_RNG_max(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * r, unsigned char * max, int len)
{
    BERR_Code retValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_GetRNGMax(hDtcpIpTl, r, max, len);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_GetRNGMax() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }
    return retValue;
}

/*! \brief Get EC-DH first phase value.
 *  \param[out] pXv EC-DH first phase value.
 *  \param[out] pXk Secret information, random number.
 *  \param[in] EccParams ECC parameters.
 *  \retval BCRYPT_STATUS_eOK or other error code.
 */
BERR_Code B_DTCP_GetFirstPhaseValue(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * pXv, unsigned char * pXk)
{
    BERR_Code retValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_GetFirstPhaseValue(hDtcpIpTl, pXv, pXk);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_GetFirstPhaseValue() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }

    return BERR_SUCCESS;
}

/*! \brief Get EC-DSA shared secret (Xk*Yv), where Yv is other device's EC-DH first phase value.
 *  \param[out] pKauth shared secret (Authentication key)
 *  \param[in]  pXk secret information.
 *  \param[in]  pYv other device's EC-DH first phase value.
 *  \param[in]  EccParams Ecc parameters.
 */
BERR_Code B_DTCP_GetSharedSecret(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * pKauth, unsigned char * pXk,
        unsigned char *pYv)
{
    BERR_Code retValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_GetSharedSecret(hDtcpIpTl, pKauth, pXk, pYv);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_GetSharedSecret() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }

    return BERR_SUCCESS;
}

/* \brief same as above but sign data using a binary key.
 */
BERR_Code B_DTCP_SignData_BinKey(DRM_DtcpIpTlHandle hDtcpIpTl, unsigned char * pSignature, unsigned char * pBuffer,
        int len)
{
    BERR_Code retValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_SignData_BinKey(hDtcpIpTl, pSignature, pBuffer, len);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_SignData_BinKey() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }

    return BERR_SUCCESS;
}

/* Verify data, but using a binary public key .
 */
BERR_Code B_DTCP_VerifyData_BinKey(DRM_DtcpIpTlHandle hDtcpIpTl, int *valid, unsigned char * pSignature, unsigned char * pBuffer,
        int len, unsigned char * BinKey)
{
    BERR_Code retValue = BERR_SUCCESS;
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_VerifyData_BinKey(hDtcpIpTl, (uint32_t *)valid, (uint8_t *)pSignature, (uint8_t *)pBuffer, (uint32_t)len, (uint8_t *)BinKey);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_SignData_BinKey() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }

    return BERR_SUCCESS;
}
/* End of file */
