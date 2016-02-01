/******************************************************************************
 *    (c)2012 Broadcom Corporation
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
 * Module Description:   Software RSA operations with encrypted certificates 
 *                       and private keys.
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *****************************************************************************/

#ifndef DRM_SECURE_SW_RSA_H_
#define DRM_SECURE_SW_RSA_H_

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "drm_types.h"
#include "drm_metadata.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum drm_secure_sw_rsa_certificate_e
{
    DRM_SECURE_SW_RSA_CON_CERTIFICATE = 0,
    DRM_SECURE_SW_RSA_INT_CERTIFICATE,
    DRM_SECURE_SW_RSA_TLS_CERTIFICATE,
    DRM_SECURE_SW_RSA_TA_CERTIFICATE,
    DRM_SECURE_SW_RSA_TD_CERTIFICATE,
    DRM_SECURE_SW_RSA_TRUST_STATUS_CERTIFICATE  
} drm_secure_sw_rsa_certificate_e;



typedef enum drm_secure_sw_rsa_key_e
{
    DRM_SECURE_SW_RSA_CON_KEY = 0,   /* Not supported */
    DRM_SECURE_SW_RSA_INT_KEY,       /* Not supported */
    DRM_SECURE_SW_RSA_TLS_KEY
} drm_secure_sw_rsa_key_e;


/* SecureSwRsa Parameter Settings structure */
typedef struct DrmSecureSwRsaParamSettings_t
{
	DrmCommonInit_t drmCommonInit;
}DrmSecureSwRsaParamSettings_t;

/******************************************************************************
** FUNCTION:
**  DRM_SecureSwRsa_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pSecureSwRsaParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_SecureSwRsa_GetDefaultParamSettings(
		DrmSecureSwRsaParamSettings_t *pSecureSwRsaParamSettings);

/******************************************************************************
** FUNCTION:
**  DRM_SecureSwRsa_SetParamSettings
**
** DESCRIPTION:
**   Set param settings
**
** PARAMETERS:
** pSecureSwRsaParamSettings - settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_SecureSwRsa_SetParamSettings(
		DrmSecureSwRsaParamSettings_t secureSwRsaParamSettings);

/******************************************************************************
** FUNCTION:
**  DRM_SecureSwRsa_Initialize
**
** DESCRIPTION:
**   Initialize the DRM module and save the custom path to the DRM binary. 
**
******************************************************************************/
DrmRC DRM_SecureSwRsa_Initialize(char *customDRMBinaryPath);



/******************************************************************************
** FUNCTION:
**  DRM_SecureSwRsa_Finalize
**
** DESCRIPTION:
**   Clean the DRM module. 
**
******************************************************************************/
DrmRC DRM_SecureSwRsa_Finalize(void);


/******************************************************************************
** FUNCTION:
**  DRM_SecureSwRsa_GetDeviceID
**
** DESCRIPTION:
**  Reads the DRM binary provided in DRM_SecureSwRsa_Initialize() and extract
**  the device unique ID. The function provides a NULL terminated string.
**    
******************************************************************************/
DrmRC DRM_SecureSwRsa_GetDeviceID(char *deviceIDBuffer, unsigned int bufferSize);


/******************************************************************************
** FUNCTION:
**  DRM_SecureSwRsa_GetRsaCertificateSize
**
** DESCRIPTION:
**  Reads the DRM binary provided in DRM_SecureSwRsa_Initialize() and extract the
**  size of the certificate specified by certType.
**	
******************************************************************************/
DrmRC DRM_SecureSwRsa_GetRsaCertificateSize(drm_secure_sw_rsa_certificate_e certType, unsigned int *certSize);



/******************************************************************************
** FUNCTION:
**  DRM_SecureSwRsa_GetRsaCertificate
**
** DESCRIPTION:
**  Reads the DRM binary provided in DRM_SecureSwRsa_Initialize(), decrypts the 
**  certificate specified by certType and copy the decrypted certificate into 
**  the certificate buffer (certBuffer). The buffer size (bufferSize) shall
**  be large enough to hold the certificate. The caller can use 
**  DRM_SecureSwRsa_GetRsaCertificateSize() to retrieve the certificate size.
**	
******************************************************************************/
DrmRC DRM_SecureSwRsa_GetRsaCertificate(drm_secure_sw_rsa_certificate_e certType, unsigned char *certBuffer, unsigned int bufferSize);



/******************************************************************************
** FUNCTION:
**  DRM_SecureSwRsa_RsaDecrypt
**
** DESCRIPTION:
**  Reads the DRM binary provided in DRM_SecureSwRsa_Initialize(), decrypt the RSA
**  private key specified by keyType and use this key to decrypt the data 
**  pointed by encData. The size of the data is specified by dataSize. The 
**  decrypted data is written to decData. The RSA key size is assumed to be 
**  2048 bits.
**	
******************************************************************************/
DrmRC DRM_SecureSwRsa_RsaDecrypt(drm_secure_sw_rsa_key_e keyType, unsigned char *encData, unsigned int dataSize, unsigned char *decData);



/******************************************************************************
** FUNCTION:
**  DRM_SecureSwRsa_RsaSign
**
** DESCRIPTION:
**  Reads the DRM binary provided in DRM_SecureSwRsa_Initialize(), decrypt the
**  RSA private key specified by keyType and use this key to sign the data 
**  pointed by dataBuffer. The size of the data is specified by dataSize. 
**  The signature is computed using 2048-bit RSA on the SHA-1 digest padded 
**  using PKCS #1 v2.0. The computed signature is written to signatureBuffer. 
**  signatureBuffer shall be at least 256 bytes	 (2048 bits).
******************************************************************************/
DrmRC DRM_SecureSwRsa_RsaSign(drm_secure_sw_rsa_key_e keyType, unsigned char *dataBuffer, unsigned int dataSize, unsigned char *signatureBuffer);



#ifdef __cplusplus
}
#endif

#endif
