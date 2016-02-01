/******************************************************************************
 *    (c)2010-2012 Broadcom Corporation
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
 *****************************************************************************/

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "drm_dtcp_ip_vendor.h"
#include "drm_dtcp_ip.h"
#include "drm_common.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


BDBG_MODULE(drm_dtcp_ip_vendor);

#ifndef B_DTCP_IP_DATA_BRCM
static 	DrmDtcpIpParamSettings_t dtcpIpParamSettings;
#endif /* B_DTCP_IP_DATA_BRCM */
/**********************************************************************************************
DRM_DtcpIp_Vendor_Initialize

Must be called only once prior to any other module API call.

Input Parameters:
	N/A

Returns:
    SUCCESS: Drm_Success
    FAILURE: other
**********************************************************************************************/
DrmRC DRM_DtcpIp_Vendor_Initialize(void)
{
	DrmRC rc = Drm_Success;


	BDBG_MSG(("%s - Entering function", __FUNCTION__));

#ifdef B_DTCP_IP_DATA_BRCM
	/* char *key_file = "DRM_BRCM_7425_DTCP_IP_SET_1.bin"; */
        char *key_file = NULL;

	/* Set key file value */
	/* IF bin file is flashed, please set key_file to NULL, otherwise, set it to the path of the bin file */

	rc = DRM_DtcpIp_Initialize(key_file);
#else /* third-party vendor */
	/*
	* IF THE THIRD-PARTY IS NOT USING BROADCOM KEY PROTECTION MECHANISM "DRM UTILITY" A, INSERT CODE HERE
	*/
	BDBG_MSG(("%s - Third-party support", __FUNCTION__));

    dtcpIpParamSettings.use_external_keys = true;
	rc = DRM_Common_AllocKeySlot(NEXUS_SecurityEngine_eM2m, &dtcpIpParamSettings.encryptKeyHandle);
	if (rc != Drm_Success)
	{
		BDBG_ERR(("%s - Error allocating enc keyslot", __FUNCTION__));
	}

	rc = DRM_Common_AllocKeySlot(NEXUS_SecurityEngine_eM2m, &dtcpIpParamSettings.decryptKeyHandle);
	if (rc != Drm_Success)
	{
		BDBG_ERR(("%s - Error allocating dec keyslot", __FUNCTION__));
	}

	DRM_DtcpIp_SetParamSettings(dtcpIpParamSettings);

	rc = DRM_Common_BasicInitialize(&dtcpIpParamSettings.drmCommonInit);
	if(rc != Drm_Success)
	{
		BDBG_ERR(("%s - Error initializing Common module", __FUNCTION__));
	}
/* PLEASE PLACE YOUR OWN INITIALIZATION CODE HERE.  IF NO INITIALIZATION IS REQUIRED THEN THIS CAN BE LEFT AS IS */

#endif /* B_DTCP_IP_DATA_BRCM */

    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
	return rc;
}


/******************************************************************************
 FUNCTION:
  DRM_DtcpIp_Vendor_Finalize

 DESCRIPTION:
   Must be called only once prior to any other module API call.

 PARAMETERS:
	N/A

******************************************************************************/
void DRM_DtcpIp_Vendor_Finalize(void)
{
#ifdef B_DTCP_IP_DATA_BRCM
	DRM_DtcpIp_Finalize();
#else
    BDBG_MSG(("%s - Entering third-party finalize",__FUNCTION__));

    if(dtcpIpParamSettings.encryptKeyHandle != NULL){
    	DRM_Common_FreeKeySlot(dtcpIpParamSettings.encryptKeyHandle);
        dtcpIpParamSettings.encryptKeyHandle = NULL;
    }

    if(dtcpIpParamSettings.decryptKeyHandle != NULL){
    	DRM_Common_FreeKeySlot(dtcpIpParamSettings.decryptKeyHandle);
        dtcpIpParamSettings.decryptKeyHandle = NULL;
    }

	DRM_Common_Finalize();
#endif
    BDBG_MSG(("%s - Exiting function",__FUNCTION__));
    return;
}


DrmRC DRM_DtcpIp_Vendor_GetData(drm_dtcp_ip_data_t *pDtcpIpData)
{
	DrmRC rc = Drm_Success;

	BDBG_MSG(("%s - Entering function", __FUNCTION__));

	if(pDtcpIpData == NULL)
	{
		rc = Drm_InvalidParameter;
		BDBG_ERR(("%s - Invalid parameter, no structure passed as parameter ", __FUNCTION__));
		goto ErrorExit;
	}

#ifdef B_DTCP_IP_DATA_BRCM
	BDBG_MSG(("%s - DTLA data provisioning using proprietary scheme", __FUNCTION__));
	rc = DRM_DtcpIp_GetData(pDtcpIpData);
#else
	/*
	*
	* NOTE: THIRD-PARTY VENDOR CAN DEVELOP AND USE THEIR OWN DTLA
	*       PROVISIONING SCHEME AND MAP THE DATA RETRIEVED INTO THE
	*       drm_dtcp_ip_data_t STRUCTURE. FOR THE TIME BEING, THIS
	*       RETURNS AN ERROR.
	*       BELOW IS MAPPING OF THE KEYS  AND CERTIFICATES 
    *   Order Format 3 Full & Restricted (Key & Certificate
    * Field Name           Size (bits) Description                                                 Metadata equivalent 
    * Baseline_Full_Cert    704         Baseline Full Authentication Certificate                   unsigned char    BaselineFullCert[DRM_DTCP_IP_ENC_BASE_FULL_AUTH_CERT_SIZE]; 
    * X-1                   160         Device Full Authentication private key                     unsigned char    PrivateKey[DRM_DTCP_IP_ENC_PRIVATE_KEY_SIZE];
    * XKSV                  12          Key Selection Vector                                       unsigned char    Ksv[DRM_DTCP_IP_ENC_KSV_SIZE];
    * XKcosnk1  XKcosn12   768         Restricted Authentication Copy Once receiver keys          unsigned char    RestrictedCopyOnceSinkKey[DRM_DTCP_IP_ENC_COPY_KEY_SIZE];
    * XKnmsnk1  XKnmsnk12  768         Restricted Authentication Copy No More receiver keys       unsigned char    RestrictedCopyNoMoreSinkKey[DRM_DTCP_IP_ENC_COPY_KEY_SIZE];
    * XKcosrc1  XKcosrc12  768         Restricted Authentication Copy Once source keys            unsigned char    RestrictedCopyOnceSourceKey[DRM_DTCP_IP_ENC_COPY_KEY_SIZE];
    * XKnmsrc1  XKnmsrc12  768         Restricted Authentication Copy No More source keys         unsigned char    RestrictedCopyNoMoreSourceKey[DRM_DTCP_IP_ENC_COPY_KEY_SIZE];
    * RNG_Seed              160         Seed for RNGF or RNGR4 random number generator             unsigned char    RngSeed[DRM_DTCP_IP_ENC_SEED_SIZE];
    * Restricted_Cert       384         Restricted Authentication Certificate                      unsigned char    RestrictedCert[DRM_DTCP_IP_ENC_REST_AUTH_CERT_SIZE];
    *
    * DTLA Constants 
    *
    * L1                    320         DTLA Public Key c, d                                       unsigned char    DtlaPublicKey[DRM_DTCP_IP_ENC_PUBLIC_KEY_SIZE];
    * P                     160         Prime of GF(p) (little-endian)                             unsigned char    PrimeGF[DRM_DTCP_IP_ENC_ECC_DATA_SIZE];
    * a                     160         Coefficient of E Curve(little-endian)                      unsigned char    ACoefficientCurve[DRM_DTCP_IP_ENC_ECC_DATA_SIZE];
    * b                     160         Coefficient of E Curve(little-endian)                      usigned char     BCoefficientCurve[DRM_DTCP_IP_ENC_ECC_DATA_SIZE];
    * x of G                160         X coordinate of Base point G (little-endian)               unsigned char    XBasePoint[DRM_DTCP_IP_ENC_ECC_DATA_SIZE];
    * y of G                160         Y coordinate of Base point G (little-endian)               unsigned char    YBasePoint[DRM_DTCP_IP_ENC_ECC_DATA_SIZE];
    * r                     160         Order of base point (little-endian)                        unsigned char    OrderBasePoint[DRM_DTCP_IP_ENC_ECC_DATA_SIZE];

	*/
	BDBG_MSG(("%s - DTLA data provisioning using third-party scheme", __FUNCTION__));
	rc = Drm_InvalidStateErr;
#endif

	if(rc != Drm_Success)
	{
		BDBG_ERR(("%s - Error in DTLA data provisioning scheme", __FUNCTION__));
		goto ErrorExit;
	}

ErrorExit:
	return rc;
}
