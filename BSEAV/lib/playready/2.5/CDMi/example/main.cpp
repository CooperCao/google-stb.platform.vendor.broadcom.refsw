/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#include <stdio.h>

#include "cdmi.h"
#include "http.h"

// Note: Remove the following block
// if building from Visual Studio is
// unsupported.
#ifdef VISUAL_STUDIO_BUILD
#include "drmlibs.h"

// DRM_BUILD_PROFILE_OEM
#define DRM_BUILD_PROFILE   10
#include <drmbuild_oem.h>
#endif

#include <drmtypes.h>
#include <drmcommon.h>

// Need to use some PK APIs for XML parsing
// and base 64 decoding.
#include <drmxmlparser.h>
#include <drmconstants.h>
#include <drmbase64.h>

// Need to use some PK APIs to do ECC 256 decryption
// and AES decryption.
#include <oemeccp256.h>
#include <oemaes.h>

#include "nexus_platform.h"
#include "nxclient.h"
#include "nexus_platform_client.h"

// If USE_PRO is true then init data which contains
// a PRO is used, otherwise init data is not used
// and CDM data which contains a key ID is used.
#define USE_PRO            0

using namespace PRCDMi;

// Path to retrieve base64 encoded license challenge from key message XML.
static DRM_ANSI_CONST_STRING g_dastrKeyMessageChallengePath = CREATE_DRM_ANSI_STRING("LicenseAcquisition/Challenge");


// Class that implements the IMediaKeySessionCallback callback interface.
class CCallback : public IMediaKeySessionCallback
{
public:
    // Constructor
    CCallback(
        __in const IMediaKeySession *f_piMediaKeySession)
    {
        m_piMediaKeySession = const_cast<IMediaKeySession *>(f_piMediaKeySession);
    }

    // Destructor
    virtual ~CCallback(void) {}

    // Called when a key message is ready to be delivered to a license server.
    virtual void OnKeyMessage(
        __in_bcount(cbKeyMessage) const uint8_t *pbKeyMessage,
        __in uint32_t cbKeyMessage,
        __in_z_opt char *f_pszUrl)
    {
        DRM_RESULT dr = DRM_SUCCESS;
        DRM_SUBSTRING dasstrKeyMessage;
        DRM_SUBSTRING dasstrChallenge;

        uint8_t *pbChallenge = NULL;
        uint32_t cbChallenge = 0;


        unsigned char* pResponse;
        uint32_t responseLength;

        uint32_t result;

        printf("Key message received (%d bytes).\n", cbKeyMessage);

        // Extract challenge from key message.
        // Challenge inside of the key message is base64 encoded.
        dasstrKeyMessage.m_ich = 0;
        dasstrKeyMessage.m_cch = cbKeyMessage;

        ChkDR(DRM_XML_GetSubNodeByPathA((DRM_CHAR *)pbKeyMessage,
                                        &dasstrKeyMessage,
                                        &g_dastrKeyMessageChallengePath,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &dasstrChallenge,
                                        g_chForwardSlash));

        cbChallenge = CB_BASE64_DECODE(dasstrChallenge.m_cch);
        ChkMem(pbChallenge = new uint8_t[cbChallenge]);

        // Base64 decode the challenge.
        ChkDR(DRM_B64_DecodeA((DRM_CHAR *)pbKeyMessage,
                              &dasstrChallenge,
                              (DRM_DWORD *)&cbChallenge,
                              pbChallenge,
                              0));

        printf("License challenge extracted (%d bytes).\n", cbChallenge);

        printf("Acquiring license...\n");

        // Start license acquisition. If a license server URL is supplied by OnKeyMessage
        // then use it. Otherwise a fixed license server URL is used.

        result = bhttpclient_license_post_soap ((char *) (f_pszUrl == NULL ? "http://playready.directtaps.net/pr/svc/rightsmanager.asmx" : f_pszUrl),
                                                             (char *)pbChallenge,
                                                             &pResponse,
                                                             &responseLength);
        if (result == bdrm_http_status_ok)
        {
            printf("Processing license response (%d bytes)...\n", responseLength);

            // Call IMediaKeySession::Update to process the license acquisition response.
            m_piMediaKeySession->Update(pResponse,
                                        responseLength);

        }
        else
        {
            printf("License acquisition failed.\n");
        }

ErrorExit:
        if (DRM_FAILED(dr))
        {
            printf("Error occurred: 0x%x.\n", (unsigned int)dr);
        }
        if(pResponse!= NULL) BKNI_Free(pResponse);
        delete[] pbChallenge;
    }

    // Called when license response is successfully processed
    // and CDM has a usable license.
    virtual void OnKeyReady(void)
    {
        printf("Key is ready.\n");
    }

    // Called when CDM encounters an error. The more generic EME
    // error is returned in the nError parameter and PlayReady CDM specific
    // error is returned in the crSysError parameter.
    virtual void OnKeyError(
        __in int16_t nError,
        __in CDMi_RESULT crSysError)
    {
        printf("Key error is detected: 0x%x (0x%x).\n", nError, crSysError);
    }

private:
    IMediaKeySession *m_piMediaKeySession;
};

// Read all data of a file into a single byte array.
// It is caller's responsibility to release the array
// if it is not needed.
// Return true when succeeds and false otherwise.
bool _ReadData(
    __in const char *pszFileName,
    __out_bcount(cbData) uint8_t *&pbData,
    __out uint32_t &cbData)
{
    bool result = false;
    FILE *fp = NULL;

    fp = fopen(pszFileName, "rb");
    if (fp == NULL)
    {
        printf("Failed to open: %s.\n", pszFileName);
        goto ErrorExit;
    }

    fseek(fp, 0l, SEEK_END);
    cbData = (uint32_t)ftell(fp);
    fseek(fp, 0l, SEEK_SET);

    pbData = new uint8_t[cbData];
    if (pbData == NULL)
    {
        goto ErrorExit;
    }

    fread(pbData, 1, cbData, fp);

    result = true;

ErrorExit:
    if (fp != NULL)
    {
        fclose(fp);
    }
    return result;
}

// Extract clear data from the data returned from CDM.
// The data returned from CDM is encrypted using the session
// key shared between media engine and CDM.
// Return true when succeeds and false otherwise.
bool _ExtractClearData(
    __in_bcount(cbEncryptedSessionKey) uint8_t *pbEncryptedSessionKey,
    __in uint32_t cbEncryptedSessionKey,
    __in_bcount(cbData) uint8_t *pbData,
    __in uint32_t cbData)
{
    DRM_RESULT dr = DRM_SUCCESS;
    PRIVKEY_P256 oECC256PrivKey;
    uint8_t *pbPrivKeyData = NULL;
    uint32_t cbPrivKeyData = 0;
    DRM_CRYPTO_CONTEXT *poCryptoContext = NULL;
    PLAINTEXT_P256 oPlainText;
    CIPHERTEXT_P256 oCipherText;
    DRM_AES_KEY oAESKey;
    DRM_AES_COUNTER_MODE_CONTEXT oAESCtrContext;

    // Read the private key of the media engine. The private key should
    // match the public key in the media engine certificate chain.
    if (!_ReadData("mediaengine_privkey.dat", pbPrivKeyData, cbPrivKeyData))
    {
        goto ErrorExit;
    }

    ChkArg(cbPrivKeyData == SIZEOF(PRIVKEY_P256));

    MEMCPY(&oECC256PrivKey, pbPrivKeyData, cbPrivKeyData);

    ZEROMEM(&oPlainText, SIZEOF(PLAINTEXT_P256));
    ZEROMEM(&oCipherText, SIZEOF(CIPHERTEXT_P256));
    ZEROMEM(&oAESKey, SIZEOF(DRM_AES_KEY));
    ZEROMEM(&oAESCtrContext, SIZEOF(DRM_AES_COUNTER_MODE_CONTEXT));

    MEMCPY(oCipherText.m_rgbCiphertext, pbEncryptedSessionKey, cbEncryptedSessionKey);

    ChkMem(poCryptoContext = (DRM_CRYPTO_CONTEXT *)Oem_MemAlloc(SIZEOF(DRM_CRYPTO_CONTEXT)));
    ZEROMEM(poCryptoContext, SIZEOF(DRM_CRYPTO_CONTEXT));

    // Use the media engine private key to decrypt the encrypted session key.
    ChkDR(OEM_ECC_Decrypt_P256(&oECC256PrivKey,
                               &oCipherText,
                               &oPlainText,
                               (struct bigctx_t *)poCryptoContext->rgbCryptoContext));

    ChkDR(Oem_Aes_SetKey(&oPlainText.m_rgbPlaintext[DRM_AES_KEYSIZE_128], &oAESKey));

    // Use the clear session key to decrypt the sample protected data.
    ChkDR(Oem_Aes_CtrProcessData(&oAESKey,
                                 pbData,
                                 cbData,
                                 &oAESCtrContext));

ErrorExit:
    SAFE_OEM_FREE(poCryptoContext);
    delete[] pbPrivKeyData;
    return DRM_SUCCEEDED(dr);
}

extern "C"
{
// Note: Remove the following block
// if building from Visual Studio is
// unsupported.
int main(int argc, char* argv[])
{
    CDMi_RESULT cr = CDMi_SUCCESS;
    IMediaKeys *piMediaKeys = NULL;
    IMediaKeySession *piMediaKeySession = NULL;
    IMediaKeySessionCallback *piCallback = NULL;
    IMediaEngineSession *piMediaEngineSession = NULL;
#if USE_PRO
    uint8_t *pbInitData = NULL;
    uint32_t cbInitData = 0;

    uint8_t pbCDMData[] = "<PlayReadyCDMData type=\"LicenseAcquisition\"> \
                                <LicenseAcquisition version=\"1.0\" Proactive=\"true\"> \
                                    <CustomData encoding=\"base64encoded\">VGhpcyBpcyBjdXN0b20gZGF0YQ==</CustomData> \
                                </LicenseAcquisition> \
                           </PlayReadyCDMData>";
#else
    uint8_t pbCDMData[] = "<PlayReadyCDMData type=\"LicenseAcquisition\"> \
                                <LicenseAcquisition version=\"1.0\" Proactive=\"true\"> \
                                    <KeyIDs> \
                                        <KeyID>MXzKGGBN9k+VXcsiCFza0g==</KeyID> \
                                    </KeyIDs> \
                                    <CustomData encoding=\"base64encoded\">VGhpcyBpcyBjdXN0b20gZGF0YQ==</CustomData> \
                                </LicenseAcquisition> \
                           </PlayReadyCDMData>";
#endif
    uint32_t cbCDMData = sizeof(pbCDMData);

    uint8_t *pbCert = NULL;
    uint32_t cbCert = 0;

    uint8_t *pbEncryptedSessionKey = NULL;
    uint32_t cbEncryptedSessionKey = 0;

    // Hardcoded data for test purpose.
    uint8_t rgbIV[] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t rgbTestData[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t *pBufToDecrypt = NULL;

    uint32_t rgdwSubSampleMapping[] = {2, 2, 3, 3};

    NEXUS_MemoryAllocationSettings heapSettings;
    NEXUS_ClientConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;

    /* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_Memory_GetDefaultAllocationSettings(&heapSettings);
    NEXUS_Platform_GetClientConfiguration(&platformConfig);
    if (platformConfig.heap[NXCLIENT_FULL_HEAP])
    {
        NEXUS_HeapHandle heap = platformConfig.heap[NXCLIENT_FULL_HEAP];
        NEXUS_MemoryStatus heapStatus;
        NEXUS_Heap_GetStatus(heap, &heapStatus);
        if (heapStatus.memoryType & NEXUS_MemoryType_eFull)
        {
            heapSettings.heap = heap;
        }
    }

    if(NEXUS_Memory_Allocate(sizeof(rgbTestData), &heapSettings, (void **)&pBufToDecrypt) != NEXUS_SUCCESS)
        goto ErrorExit;

    // Phase one:
    // Create a media keys and use it to create a media key session.
    cr = CreateMediaKeys(&piMediaKeys);
    if (CDMi_FAILED(cr))
    {
        goto ErrorExit;
    }

#if USE_PRO
    if (!_ReadData("pssh.dat", pbInitData, cbInitData))
    {
        goto ErrorExit;
    }

    // Create a media key session using init data and/or CDM data.
    cr = piMediaKeys->CreateMediaKeySession(NULL,
                                            pbInitData,
                                            cbInitData,
                                            pbCDMData,
                                            cbCDMData,
                                            &piMediaKeySession);
#else
    // Create a media key session using CDM data only.
    cr = piMediaKeys->CreateMediaKeySession(NULL,
                                            NULL,
                                            0,
                                            pbCDMData,
                                            cbCDMData,
                                            &piMediaKeySession);
#endif
    if (CDMi_FAILED(cr))
    {
        goto ErrorExit;
    }

    piCallback = new CCallback(piMediaKeySession);
    piMediaKeySession->Run(piCallback);

    // Phase two:
    // Create a media engine session using the media key session
    // obtained earlier. Then try to use media engine session to
    // decrypt a piece of test data.

    // First read a media engine certificate chain.
    // The certificate chain is used to authenticate media engine
    // with CDM. After authentication completes a secure channel is
    // established between media engine and CDM and the secure channel
    // is protected by an encrypted session key that is used to
    // extract data from the secure channel.
    if (!_ReadData("mediaengine_certchain.dat", pbCert, cbCert))
    {
        goto ErrorExit;
    }

    // Create a media engine session using the media engine certificate
    // chain obtained earlier. An encrypted session key is returned and
    // it will be used to decrypt data returned from the following call of
    // IMediaEngineSession::Decrypt.
    cr = CreateMediaEngineSession(piMediaKeySession,
                                  pbCert,
                                  cbCert,
                                  &pbEncryptedSessionKey,
                                  &cbEncryptedSessionKey,
                                  &piMediaEngineSession);
    if (CDMi_FAILED(cr))
    {
        goto ErrorExit;
    }

    // Call IMediaEngineSession::Decrypt to decrypt the test data.
    // Please note that the returned data is still encrypted by
    // the session key. Another step is needed to extract the clear
    // data from the returned sample protected data.

    BKNI_Memcpy(pBufToDecrypt, rgbTestData, sizeof(rgbTestData));
    cr = piMediaEngineSession->Decrypt(rgbIV,
                                       sizeof(rgbIV),
                                       pBufToDecrypt,
                                       sizeof(rgbTestData),
                                       rgdwSubSampleMapping,
                                       sizeof(rgdwSubSampleMapping) / sizeof(rgdwSubSampleMapping[0]));
    if (CDMi_FAILED(cr))
    {
        goto ErrorExit;
    }

    // Extract the clear data from "decrypted" data returned from
    // CDM using the session key.
    // When the call finishes rgbTestData should contain the clear
    // data.
    if (!_ExtractClearData(pbEncryptedSessionKey,
                           cbEncryptedSessionKey,
                           pBufToDecrypt,
                           sizeof(rgbTestData)))
    {
        printf("Extracting clear data from secure channel failed.\n");
    }
    else
    {
        printf("Decryption completed successfully.\n");
    }

    // To prove that CDM is able to decrypt correctly, one may call
    // piMediaEngineSession->Decrypt and _ExtractClearData the second
    // time. If everything is ok then rgbTestData will be restored to
    // its initial value.


ErrorExit:
    if (CDMi_FAILED(cr))
    {
        printf("Error is detected: 0x%x.\n", cr);
    }

#if USE_PRO
    delete[] pbInitData;
#endif

    // Cleanup.
    delete[] pbCert;
    delete[] pbEncryptedSessionKey;

    if (piMediaEngineSession != NULL)
    {
        DestroyMediaEngineSession(piMediaEngineSession);
    }

    if (piMediaKeys != NULL && piMediaKeySession != NULL)
    {
        piMediaKeys->DestroyMediaKeySession(piMediaKeySession);
    }

    if (piMediaKeys != NULL)
    {
        DestroyMediaKeys(piMediaKeys);
    }

    printf("Press <Enter> to exit...\n");
    getchar();
    if(pBufToDecrypt) NEXUS_Memory_Free(pBufToDecrypt);
    NEXUS_Platform_Uninit();

    return cr;
}
}
