/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#include "cdmi_imp.h"

#include <drmbcertparser.h>
#include <drmsoapxmlutility.h>

#include "bstd.h"           /* brcm includes */
#include "bkni.h"

namespace PRCDMi
{

// Constructor
CMediaEngineSession::CMediaEngineSession(void) :
    m_piMediaKeySession(NULL),
    m_pbSessionKey(NULL),
    m_cbSessionKey(0)
{
}

// Destructor
CMediaEngineSession::~CMediaEngineSession(void)
{
    SAFE_OEM_FREE(m_pbSessionKey);
}

// Check whether the supplied media engine certificate
// chain is valid. The media engine certificate chain is
// protected by the Microsoft root certificate and is used
// to encrypt the AES session key to establish a secure
// communication channel between media engine and CDM.
CDMi_RESULT _ValidateCertChain(
    __in_bcount(f_cbCertChain) const uint8_t *f_pbCertChain,
    __in uint32_t f_cbCertChain)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRMFILETIME ftCurrentTime = {0, 0};
    DRM_BCERT_VERIFICATIONCONTEXT oVerificationContext; // Will be initialized by DRM_BCert_InitVerificationContext.
    DRM_CRYPTO_CONTEXT *poCryptoContext = NULL;

    ChkArg(f_pbCertChain != NULL && f_cbCertChain > 0);

    ChkMem(poCryptoContext = (DRM_CRYPTO_CONTEXT *)Oem_MemAlloc(SIZEOF(DRM_CRYPTO_CONTEXT)));
    ZEROMEM(poCryptoContext, SIZEOF(DRM_CRYPTO_CONTEXT));

    Oem_Clock_GetSystemTimeAsFileTime(NULL, &ftCurrentTime);

    ChkDR(DRM_BCert_InitVerificationContext(&ftCurrentTime,
                                            // TODO: (TFS #1722617) This should point to the public key from the trusted root.
                                            NULL,
                                            // TODO: (TFS #1722617) This should indicate the desired cert type. Currently there
                                            // is not yet a cert type defined for sample protection.
                                            DRM_BCERT_CERTTYPE_UNKNOWN,
                                            poCryptoContext,
                                            TRUE,
                                            FALSE,
                                            // The next two parameters specified the required key usage.
                                            // DRM_BCERT_KEYUSAGE_ENCRYPTKEY_SAMPLE_PROTECTION should be added.
                                            NULL,
                                            0,
                                            FALSE,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &oVerificationContext));

    ChkDR(DRM_BCert_ParseCertificateChain(f_pbCertChain,
                                          f_cbCertChain,
                                          &oVerificationContext));

    // TODO: (TFS #1722617) Ensure the leaf certificate is of type of sample protection.

ErrorExit:
    SAFE_OEM_FREE(poCryptoContext);
    return dr;
}

// Retrive the public key from the leaf certificate of the
// supplied certificate chain.
CDMi_RESULT _GetPublicKeyFromLeafCert(
    __in_bcount(f_cbCertChain) const uint8_t *f_pbCertChain,
    __in uint32_t f_cbCertChain,
    __out PUBKEY_P256 *f_poECC256PubKey)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_BCERT_CHAIN_HEADER oCertChainHeader; // Will be initialized by DRM_BCert_GetChainHeader.
    DRM_BCERT_CERTIFICATE oCertificate; // Will be initialized by DRM_BCert_GetCertificate.
    DRM_DWORD bCurrOffset = 0;

    ChkArg(f_pbCertChain != NULL && f_cbCertChain > 0);
    ChkArg(f_poECC256PubKey != NULL);

    ChkDR(DRM_BCert_GetChainHeader(f_pbCertChain,
                                   f_cbCertChain,
                                   &bCurrOffset,
                                   &oCertChainHeader));

    ChkDR(DRM_BCert_GetCertificate(f_pbCertChain,
                                   f_cbCertChain,
                                   &bCurrOffset,
                                   &oCertificate,
                                   DRM_BCERT_CERTTYPE_UNKNOWN));

    // TODO: (TFS #1722617) Enforce key usage check.
    ChkBOOL(oCertificate.KeyInfo.dwNumKeys >= 1, DRM_E_FAIL);
    ChkBOOL(oCertificate.KeyInfo.rgoKeys[0].wLength / 8 == SIZEOF(PUBKEY_P256), DRM_E_FAIL);

    MEMCPY(f_poECC256PubKey,
           oCertificate.KeyInfo.rgoKeys[0].pValue->m_rgbPubkey,
           SIZEOF(PUBKEY_P256));

ErrorExit:
    return dr;
}

// Initialize a media engine session. Caller supplies a media
// key session and a session key that is used to establish a
// secure communication channel between media engine and CDM.
// The supplied session key is in clear and is derived from the
// encrypted session key returned from CreateMediaEngineSession.
CDMi_RESULT CMediaEngineSession::Init(
    __in const IMediaKeySession *f_piMediaKeySession,
    __in_bcount(f_cbSessionKey) const DRM_BYTE *f_pbSessionKey,
    __in DRM_DWORD f_cbSessionKey)
{
    DRM_RESULT dr = DRM_SUCCESS;

    ChkArg(f_piMediaKeySession != NULL);
    ChkArg(f_pbSessionKey != NULL && f_cbSessionKey > 0);

    m_piMediaKeySession = const_cast<IMediaKeySession *>(f_piMediaKeySession);

    // Remember the clear session key.
    m_cbSessionKey = f_cbSessionKey;
    ChkMem(m_pbSessionKey = (DRM_BYTE *)Oem_MemAlloc(m_cbSessionKey));
    MEMCPY(m_pbSessionKey, f_pbSessionKey, m_cbSessionKey);

ErrorExit:
    return dr;
}

// Decrypt a block of data using supplied IV and the optional
// subsample mapping data. The decrypted data is immediately
// re-encrypted using the cached session key before being returned
// to the caller.
CDMi_RESULT CMediaEngineSession::Decrypt(
    __in_bcount(f_cbIV) const uint8_t *f_pbIV,
    __in uint32_t f_cbIV,
    __inout_bcount(f_cbData) uint8_t *f_pbData,
    __in uint32_t f_cbData,
    __in_ecount_opt(f_cdwSubSampleMapping) const uint32_t *f_pdwSubSampleMapping,
    __in uint32_t f_cdwSubSampleMapping)
{
    DRM_RESULT dr = DRM_SUCCESS;
    CMediaKeySession *poMediaKeySession = NULL;

    ChkArg(f_pbIV != NULL && f_cbIV > 0);
    ChkArg(f_pbData != NULL && f_cbData > 0);
    ChkArg((f_pdwSubSampleMapping == NULL) == (f_cdwSubSampleMapping == 0));

    // Note: CMediaKeySession::Decrypt is not an interface method of
    // IMediaKeySession. CMediaEngineSession has internal knowledge
    // of how to down cast IMediaKeySession into its implementation
    // class.
    poMediaKeySession = static_cast<CMediaKeySession *>(m_piMediaKeySession);
    ChkBOOL(poMediaKeySession != NULL, DRM_E_FAIL);

    // The actual work is done by media key session of CDM.
    ChkDR(poMediaKeySession->Decrypt(m_pbSessionKey,
                                     m_cbSessionKey,
                                     f_pbIV,
                                     f_cbIV,
                                     f_pbData,
                                     f_cbData,
                                     f_pdwSubSampleMapping,
                                     f_cdwSubSampleMapping));

ErrorExit:
    return dr;
}

// Factory method that creates a media engine session. Caller
// supplies a media key session, a media engine certificate chain
// and gets back an encrypted session key as well as a media
// engine session interface.
// If the returned media engine session interface is not needed then
// it must be released via the call of DestroyMediaEngineSession.
// The returned encrypted session key should be released via delete[]
// if it is not used.
CDMi_RESULT CreateMediaEngineSession(
    __in const IMediaKeySession *f_piMediaKeySession,
    __in_bcount(f_cbCertChain) const uint8_t *f_pbCertChain,
    __in uint32_t f_cbCertChain,
    __deref_out_bcount(*f_pcbSessionKey) uint8_t **f_ppbEncryptedSessionKey,
    __out uint32_t *f_pcbEncryptedSessionKey,
    __deref_out IMediaEngineSession **f_ppiMediaEngineSession)
{
    DRM_RESULT dr = DRM_SUCCESS;
    CMediaEngineSession *poMediaEngineSession = NULL;
    PUBKEY_P256 oECC256PubKey;
    DRM_CRYPTO_CONTEXT *poCryptoContext = NULL;
    PLAINTEXT_P256 oPlainText;
    CIPHERTEXT_P256 oCipherText;

    ChkArg(f_piMediaKeySession != NULL);
    ChkArg(f_pbCertChain != NULL && f_cbCertChain > 0);
    ChkArg(f_ppbEncryptedSessionKey != NULL && f_pcbEncryptedSessionKey != NULL);
    ChkArg(f_ppiMediaEngineSession != NULL);

    BKNI_Memset(&oPlainText, 0, sizeof(oPlainText));
    BKNI_Memset(&oCipherText, 0, sizeof(oCipherText));

    *f_ppiMediaEngineSession = NULL;

    ChkMem(poCryptoContext = (DRM_CRYPTO_CONTEXT *)Oem_MemAlloc(SIZEOF(DRM_CRYPTO_CONTEXT)));
    ZEROMEM(poCryptoContext, SIZEOF(DRM_CRYPTO_CONTEXT));

    // Note: The following implementation assumes the sample protection
    // certificate chain is in the format of PlayReady binary certificate chain.
    // The implementation could be changed once the format is officially determined.
    // Currently there is no PlayReady certificate type of sample encryption
    // so some work needs to be done to update the binary cert parser/builder.

    // Validate sample protection cert chain.
    ChkDR(_ValidateCertChain(f_pbCertChain,
                             f_cbCertChain));

    // Extract public key from leaf cert and store it in
    // oECC256PubKey.
    ChkDR(_GetPublicKeyFromLeafCert(f_pbCertChain,
                                    f_cbCertChain,
                                    &oECC256PubKey));

    // Generate a random AES key (second 16 bytes). The second 16 bytes MUST
    // be filled ealier than the first 16 bytes.
    ChkDR( Oem_Random_GetBytes(&oPlainText.m_rgbPlaintext[DRM_AES_KEYSIZE_128],
                               DRM_AES_KEYSIZE_128 ) );

    // Generate random first 16 bytes of padding such that the entire buffer can be
    // encrypted with ECC 256.
    ChkDR(OEM_ECC_GenerateHMACKey_P256(&oPlainText,
                                       (struct bigctx_t *)poCryptoContext->rgbCryptoContext));

    // ECC 256 encrypt the clear AES session key using the public key extracted
    // from the sample protection certificate.
    ChkDR(OEM_ECC_Encrypt_P256(&oECC256PubKey,
                               &oPlainText,
                               &oCipherText,
                               (struct bigctx_t *)poCryptoContext->rgbCryptoContext));

    ChkMem(poMediaEngineSession = new CMediaEngineSession());

    // Pass the clear AES session key to MediaEngineSession for sample protection.
    ChkDR(poMediaEngineSession->Init(f_piMediaKeySession,
                                     &oPlainText.m_rgbPlaintext[DRM_AES_KEYSIZE_128],
                                     DRM_AES_KEYSIZE_128));

    // Return the ECC 256 encrypted AES session key to the caller who can decrypt
    // using the corresponding private key that it knows.
    ChkMem(*f_ppbEncryptedSessionKey = new uint8_t[SIZEOF(oCipherText)]);
    *f_pcbEncryptedSessionKey = SIZEOF(oCipherText);
    MEMCPY(*f_ppbEncryptedSessionKey, &oCipherText, SIZEOF(oCipherText));

    *f_ppiMediaEngineSession = static_cast<IMediaEngineSession *>(poMediaEngineSession);

ErrorExit:
    SAFE_OEM_FREE(poCryptoContext);
    return dr;
}

// Close a media engine session interface and frees
// all resources associated with it.
CDMi_RESULT DestroyMediaEngineSession(
    __in IMediaEngineSession *f_piMediaEngineSession)
{
    DRM_RESULT dr = DRM_SUCCESS;

    ChkArg(f_piMediaEngineSession != NULL);

    delete f_piMediaEngineSession;

ErrorExit:
    return dr;
}

} // namespace PRCDMi
