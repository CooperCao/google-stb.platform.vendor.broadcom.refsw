/***************************************************************************
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
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/x509.h>

#include "nexus_platform.h"
#include "drm_secure_sw_rsa.h"
#include "drm_common_swcrypto.h"
#include "drm_common_swcrypto_types.h"

#define TEST_SECURE_SW_RSA_VERBOSE 0


static DrmRC Display_Certificate(drm_secure_sw_rsa_certificate_e certType);
static DrmRC Get_Certificate(drm_secure_sw_rsa_certificate_e certType, unsigned char **ppCert, unsigned int *pSize);
void Print_Buffer(char *pTitle, unsigned char *pBuffer, unsigned int uiSize);

#if 0
static DrmRC Run_EncryptAndDecrypt_Test(drm_secure_sw_rsa_key_e keyType);
static DrmRC Run_SignAndVerify_Test(drm_secure_sw_rsa_key_e keyType);

static unsigned char randomData[] = {
                                     0xdb, 0xbb, 0x84, 0xa3, 0x6b, 0x08, 0xe7, 0x7c, 0xe1, 0xb1, 0x17, 0x1d, 0xda, 0x6c, 0xac, 0x4c,
                                     0x23, 0xe3, 0xfe, 0xb4, 0xde, 0xec, 0xf0, 0x55, 0x14, 0xda, 0x68, 0xce, 0x96, 0x31, 0xf6, 0x00,
                                     0xaf, 0x3f, 0xf1, 0xb6, 0xaf, 0x7c, 0xf3, 0xa4, 0x67, 0xdc, 0xfd, 0xad, 0xf5, 0xc8, 0xce, 0x5e,
                                     0x96, 0xb8, 0xcb, 0xb4, 0xca, 0x0e, 0xb1, 0x6a, 0xbc, 0x41, 0x4e, 0x96, 0x50, 0xa7, 0x4a, 0xda,
                                     0xab, 0xcc, 0x34, 0x5e, 0x26, 0x8f, 0x4b, 0x41, 0xee, 0xa3, 0x47, 0x68, 0xd6, 0xea, 0xca, 0x3a,
                                     0xbd, 0x77, 0x4e, 0x47, 0x2f, 0xc8, 0xe8, 0x10, 0x87, 0x30, 0x00, 0x19, 0xc8, 0xf2, 0xfa, 0x73,
                                     0xeb, 0xfb, 0x6b, 0xe0, 0x00, 0xd6, 0xa8, 0x37, 0xbb, 0x2b, 0xc7, 0x2f, 0xd4, 0x77, 0x5b, 0x0a,
                                     0x7c, 0x2a, 0xab, 0x81, 0x26, 0xa6, 0x6a, 0xff, 0x95, 0x4f, 0x00, 0xf5, 0xd7, 0x95, 0x22, 0xfe,
                                     0x52, 0xea, 0x65, 0x8c, 0x0e, 0xcf, 0x5b, 0x69, 0xd6, 0xf0, 0xc0, 0x7a, 0x95, 0xb9, 0xe1, 0xec,
                                     0x7e, 0xed, 0x2f, 0xa5, 0xa6, 0xc3, 0x06, 0x6a, 0x38, 0x75, 0xde, 0x24, 0x5d, 0x37, 0xbc, 0x34,
                                     0x60, 0x74, 0x6e, 0xaf, 0x29, 0x78, 0xc2, 0x28, 0xf6, 0x53, 0x4b, 0x02, 0xb8, 0x6c, 0x15, 0xcf,
                                     0xe5, 0x66, 0x20, 0x23, 0xf0, 0x84, 0xc4, 0x8d, 0x40, 0x22, 0xf3, 0xf1, 0xf2, 0xab, 0xb5, 0x75,
                                     0xab, 0x08, 0x42, 0xcd, 0x30, 0x8d, 0x75, 0x44, 0xfe, 0x51, 0x64, 0x03, 0x29, 0x8a, 0x94, 0xfa,
                                     0xeb, 0x3d, 0x92, 0x0b, 0x64, 0x91, 0xda, 0x0c, 0x84, 0x44, 0x91, 0x56, 0xce, 0xf1, 0x4f, 0x8e,
                                     0xa2, 0x27, 0x45, 0x64, 0x60, 0xc6, 0xf0, 0x98, 0x51, 0x0c, 0x04, 0x28, 0x5b, 0x39, 0x5d, 0x9d,
                                     0x96, 0x4f, 0x4c, 0x24, 0xca, 0x4f, 0xde, 0xd9, 0x01, 0x08, 0xd5, 0xa9, 0x98, 0xc0, 0x7a, 0x1a};
#endif


int main(int argc, char* argv[])
{
    DrmRC rc = Drm_Success;

    NEXUS_PlatformSettings platformSettings;
    char deviceID[64];
    char *customDRMBinaryPath = NULL;

    /* Verify parameter */
    if(argc > 2)
    {
        rc = Drm_InvalidParameter;
        printf("\nUsage:\n\n    test_secure_sw_rsa [DRM binary file name]\n\n");
        printf("        When DRM binary file is not specified, the test application\n");
        printf("        will load the DRM binary from flash.\n\n");
        goto handle_error;
    }


    /* Call Platform Init */
    NEXUS_Platform_GetDefaultSettings(&platformSettings); 
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    if(argc == 2)
    {
        /* Use specified DRM binary file */
        customDRMBinaryPath = argv[1];
    }
    else
    {
        /* Use DRM binary in flash */
        customDRMBinaryPath = NULL;
    }

    rc = DRM_SecureSwRsa_Initialize(customDRMBinaryPath);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_SecureSwRsa_Initialize()\n");
        goto handle_error;
    }

    /* Display device ID */
    rc = DRM_SecureSwRsa_GetDeviceID(deviceID, sizeof(deviceID));
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_SecureSwRsa_GetDeviceID\n");
        goto handle_error;
    }
    printf("\nDevice Unique ID: %s\n", deviceID);


    /* Display certificates */
    rc = Display_Certificate(DRM_SECURE_SW_RSA_CON_CERTIFICATE);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Display_Certificate(DRM_SECURE_SW_RSA_CON_CERTIFICATE)\n");
        goto handle_error;
    }

    rc = Display_Certificate(DRM_SECURE_SW_RSA_INT_CERTIFICATE);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Display_Certificate(DRM_SECURE_SW_RSA_INT_CERTIFICATE)\n");
        goto handle_error;
    }

    rc = Display_Certificate(DRM_SECURE_SW_RSA_TLS_CERTIFICATE);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Display_Certificate(DRM_SECURE_SW_RSA_TLS_CERTIFICATE)\n");
        goto handle_error;
    }

    rc = Display_Certificate(DRM_SECURE_SW_RSA_TA_CERTIFICATE);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Display_Certificate(DRM_SECURE_SW_RSA_TA_CERTIFICATE)\n");
        goto handle_error;
    }

    rc = Display_Certificate(DRM_SECURE_SW_RSA_TD_CERTIFICATE);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Display_Certificate(DRM_SECURE_SW_RSA_TD_CERTIFICATE)\n");
        goto handle_error;
    }

    rc = Display_Certificate(DRM_SECURE_SW_RSA_TRUST_STATUS_CERTIFICATE);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Display_Certificate(DRM_SECURE_SW_RSA_TRUST_STATUS_CERTIFICATE)\n");
        goto handle_error;
    }

#if 0 /* TLS key no longer found in DRM bin */
    /* Run encrypt and decrypt test */
    rc = Run_EncryptAndDecrypt_Test(DRM_SECURE_SW_RSA_TLS_KEY);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Run_EncryptAndDecrypt_Test()\n");
        goto handle_error;
    }

    /* Run sign and verify test */
    rc = Run_SignAndVerify_Test(DRM_SECURE_SW_RSA_TLS_KEY);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Run_SignAndVerify_Test()\n");
        goto handle_error;
    }
#endif
    printf("\nAll tests are successful. Please manually compare certificates with originals.\n\n");


handle_error:
    DRM_SecureSwRsa_Finalize();

    return 0;
}



static DrmRC Display_Certificate(drm_secure_sw_rsa_certificate_e certType)
{
    DrmRC rc = Drm_Success;

    unsigned char *certBuffer = NULL; 
    unsigned int certSize = 0;

    rc = Get_Certificate(certType, &certBuffer, &certSize);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Get_Certificate()\n");
        goto handle_error;
    }

    /* Retrieve matching certificate type */
    switch(certType)
    {
        case DRM_SECURE_SW_RSA_CON_CERTIFICATE:
            printf("\nCON certificate");
            break;
        case DRM_SECURE_SW_RSA_INT_CERTIFICATE:
            printf("\nINT certificate");
            break;
        case DRM_SECURE_SW_RSA_TLS_CERTIFICATE:
            printf("\nTLS certificate");
            break;
        case DRM_SECURE_SW_RSA_TA_CERTIFICATE:
            printf("\nTA certificate");
            break;
        case DRM_SECURE_SW_RSA_TD_CERTIFICATE:
            printf("\nTD certificate");
            break;
        case DRM_SECURE_SW_RSA_TRUST_STATUS_CERTIFICATE:
            printf("\nTRUST_STATUS certificate");
            break;
        default:
            printf("\nCertificate");
            break;
    }

    /* Print the certificate */
    Print_Buffer("", certBuffer, certSize);

handle_error:
    if(certBuffer != NULL)
    {
        free(certBuffer);
        certBuffer = NULL;
    }

    return rc;
}


#if 0
static DrmRC Run_EncryptAndDecrypt_Test(drm_secure_sw_rsa_key_e keyType)
{
    DrmRC rc = Drm_Success;
    
    drm_secure_sw_rsa_certificate_e certType;
    char *keyTypeString = NULL;

    unsigned char *certBuffer = NULL; 
    unsigned int certSize = 0;

    X509 *pX509Certificate = NULL;

    DrmCommon_RsaKey_t publicKey;

    DrmCommon_RsaSwParam_t rsa_params;
    unsigned char decData[256];
    unsigned char encData[256];
    unsigned long encDataSize = 0;


    /* Retrieve matching certificate type */
    switch(keyType)
    {
        case DRM_SECURE_SW_RSA_CON_KEY:
            certType = DRM_SECURE_SW_RSA_CON_CERTIFICATE;
            keyTypeString = "CON";
            break;
        case DRM_SECURE_SW_RSA_INT_KEY:
            certType = DRM_SECURE_SW_RSA_INT_CERTIFICATE;
            keyTypeString = "INT";
            break;
        case DRM_SECURE_SW_RSA_TLS_KEY:
            certType = DRM_SECURE_SW_RSA_TLS_CERTIFICATE;
            keyTypeString = "TLS";
            break;
        default:
            rc = Drm_InvalidParameter;
            keyTypeString = "Invalid";
            printf("test_secure_sw_rsa: Invalid key type.\n");
            goto handle_error;
            break;
    }

    /* Read certificate and decrypt from DRM binary */
    rc = Get_Certificate(certType, &certBuffer, &certSize);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Get_Certificate()\n");
        goto handle_error;
    }    

    /* Decode the certificate */
    rc = DRM_Common_Swx509ASN1DerDecode(certBuffer, certSize, &pX509Certificate);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_Common_Swx509ASN1DerDecode()\n");
        goto handle_error;
    }

    /* Extract the public key */
    rc = DRM_Common_Swx509GetRsaPublicKey(pX509Certificate, &publicKey);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_Common_Swx509GetRsaPublicKey()\n");
        goto handle_error;
    }

#if TEST_SECURE_SW_RSA_VERBOSE
    Print_Buffer("\nPublic key (modulus)", publicKey.n.pData, publicKey.n.len);
#endif

    /* Encrypt random data */
    rsa_params.key = &publicKey;
    rsa_params.bRSAop = drmRsaenc;
    rsa_params.pbDataIn = randomData;
    rsa_params.cbDataIn = 214;
    rsa_params.psAlgorithmId = NULL;
    rsa_params.csAlgorithmId = 0;
    rsa_params.pbDataOut = encData;
    rsa_params.cbDataOut = &encDataSize;

    rc = DRM_Common_SwRsa(&rsa_params);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_Common_SwRsa()\n");
        goto handle_error;
    }

#if TEST_SECURE_SW_RSA_VERBOSE
    Print_Buffer("\nEncrypted Data", encData, sizeof(encData));
#endif


    /* Decrypt buffer */
    rc = DRM_SecureSwRsa_RsaDecrypt(keyType, encData, sizeof(encData), decData);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_SecureSwRsa_RsaDecrypt()\n");
        goto handle_error;
    }


#if TEST_SECURE_SW_RSA_VERBOSE
    Print_Buffer("\nDecrypted data", decData, 256);
#endif

    if(memcmp(decData, randomData, 214) == 0)
    {
        printf("Encrypt and decrypt test: OK.\n");
    }
    else
    {
        rc = Drm_Err;
        printf("test_secure_sw_rsa: Decrypted data does not match clear text data.\n");
        goto handle_error;
    }

handle_error:
    if(pX509Certificate != NULL)
    {
        DRM_Common_Swx509Free(pX509Certificate);
        pX509Certificate = NULL;
    }

    if(certBuffer != NULL)
    {
        free(certBuffer);
        certBuffer = NULL;
    }

    return rc;
}



static DrmRC Run_SignAndVerify_Test(drm_secure_sw_rsa_key_e keyType)
{
    DrmRC rc = Drm_Success;

    drm_secure_sw_rsa_certificate_e certType;
    char *keyTypeString = NULL;

    unsigned char *certBuffer = NULL; 
    unsigned int certSize = 0;

    X509 *pX509Certificate = NULL;

    DrmCommon_RsaKey_t publicKey;

    DrmCommon_RsaSwParam_t rsa_params;
    unsigned char signatureBuffer[256];
    unsigned char digestBuffer[20];
    unsigned long signatureSize = 0;

    const char ALGORITHM_ID_SHA1[] = "sha1"; 
   

    /* Retrieve matching certificate type */
    switch(keyType)
    {
        case DRM_SECURE_SW_RSA_CON_KEY:
            certType = DRM_SECURE_SW_RSA_CON_CERTIFICATE;
            keyTypeString = "CON";
            break;
        case DRM_SECURE_SW_RSA_INT_KEY:
            certType = DRM_SECURE_SW_RSA_INT_CERTIFICATE;
            keyTypeString = "INT";
            break;
        case DRM_SECURE_SW_RSA_TLS_KEY:
            certType = DRM_SECURE_SW_RSA_TLS_CERTIFICATE;
            keyTypeString = "TLS";
            break;
        default:
            rc = Drm_InvalidParameter;
            keyTypeString = "Invalid";
            printf("test_secure_sw_rsa: Invalid key type.\n");
            goto handle_error;
            break;
    }

    /* Read certificate and decrypt from DRM binary */
    rc = Get_Certificate(certType, &certBuffer, &certSize);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in Get_Certificate()\n");
        goto handle_error;
    }    

    /* Decode de certificate */
    rc = DRM_Common_Swx509ASN1DerDecode(certBuffer, certSize, &pX509Certificate);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_Common_Swx509ASN1DerDecode()\n");
        goto handle_error;
    }

    /* Extract the public key */
    rc = DRM_Common_Swx509GetRsaPublicKey(pX509Certificate, &publicKey);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_Common_Swx509GetRsaPublicKey()\n");
        goto handle_error;
    }

#if TEST_SECURE_SW_RSA_VERBOSE
    Print_Buffer("\nPublic key (modulus)", publicKey.n.pData, publicKey.n.len);
#endif


    /* Generate signature on data */     
    rc = DRM_SecureSwRsa_RsaSign(DRM_SECURE_SW_RSA_TLS_KEY, randomData, 256, signatureBuffer);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_SecureSwRsa_RsaSign()\n");
        goto handle_error;
    }

#if TEST_SECURE_SW_RSA_VERBOSE
    Print_Buffer("\nSignature", signatureBuffer, 256);
#endif

    /* Compute digest on data */
    rc = DRM_Common_SwSha1(randomData, digestBuffer, 256);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_Common_SwSha1()\n");
        goto handle_error;
    }            

#if TEST_SECURE_SW_RSA_VERBOSE
    Print_Buffer("\nDigest", digestBuffer, 20);
#endif

    /* Verify signature */
    signatureSize = 256;

    rsa_params.key = &publicKey;
    rsa_params.bRSAop = drmRsaverify;
    rsa_params.pbDataIn = digestBuffer;
    rsa_params.cbDataIn = 20;
    rsa_params.psAlgorithmId = ALGORITHM_ID_SHA1;
    rsa_params.csAlgorithmId = sizeof(ALGORITHM_ID_SHA1);
    rsa_params.pbDataOut = signatureBuffer;
    rsa_params.cbDataOut = &signatureSize;

    rc = DRM_Common_SwRsa(&rsa_params);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_Common_SwRsa()\n");
        goto handle_error;
    }

handle_error:
    if(pX509Certificate != NULL)
    {
        DRM_Common_Swx509Free(pX509Certificate);
        pX509Certificate = NULL;
    }

    if(certBuffer != NULL)
    {
        free(certBuffer);
        certBuffer = NULL;
    }

    return rc;
}
#endif


static DrmRC Get_Certificate(drm_secure_sw_rsa_certificate_e certType, unsigned char **ppCert, unsigned int *pSize)
{
    DrmRC rc = Drm_Success;

    unsigned char *certBuffer = NULL; 
    unsigned int certSize = 0;


    rc = DRM_SecureSwRsa_GetRsaCertificateSize(certType, &certSize);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_SecureSwRsa_GetRsaCertificateSize()\n");
        goto handle_error;
    }

    /* Sanity check on certificate size. Do not allow certificate size more than 128 kB. */
    if(certSize > (128*1024))
    {
        rc = Drm_InvalidParameter;
        printf("test_secure_sw_rsa: Certificate size is too large.\n");
        goto handle_error;
    }

    /*printf("TLS certificate size: %d\n", certSize);*/

    /* Allocate memory for the certificate buffer. */
    certBuffer = malloc(certSize);
    if(certBuffer == NULL)
    {
        rc = Drm_MemErr;
        printf("test_secure_sw_rsa: Out of memory.\n");
        goto handle_error;
    }

    /* Retrieve the certificate */
    rc = DRM_SecureSwRsa_GetRsaCertificate(certType, certBuffer, certSize);
    if(rc != Drm_Success)
    {   
        printf("test_secure_sw_rsa: Failure in DRM_SecureSwRsa_GetRsaCertificate()\n");
        goto handle_error;
    }

    /* Return the certificate buffer and size. */
    *ppCert = certBuffer;
    *pSize  = certSize;

handle_error:
    return rc;
}



void Print_Buffer(char *pTitle, unsigned char *pBuffer, unsigned int uiSize)
{
    unsigned int byteNo = 0;

    printf("%s:", pTitle);
    for(byteNo = 0; byteNo < uiSize; byteNo++)
    {
        if((byteNo % 4) == 0)
        {
            printf(" ");
        }
        if((byteNo % 16) == 0)
        {
            printf("\n");
        }
        printf("%02x", pBuffer[byteNo]);
    }
    printf("\n");

    return;
}
