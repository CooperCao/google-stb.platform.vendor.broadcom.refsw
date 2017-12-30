/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#include "cdmi_imp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <drmrevocation.h>
#include <drmxmlparser.h>

#include <byteorder.h>
#include <drmmathsafe.h>

#include "bstd.h"           /* brcm includes */
#include "bkni.h"

#include "nxclient.h"
#include "nexus_platform_client.h"

#if SAGE_SUPPORT
#include "bsagelib_types.h"
#include "bsagelib_drm_types.h"
#endif

namespace PRCDMi
{

static const char *DRM_LICENSE_STORAGE_FILE="sample.hds";
static const char *DRM_DEFAULT_REVOCATION_LIST_FILE="revpackage.xml";

const DRM_CONST_STRING  *g_rgpdstrRights[1] = {&g_dstrWMDRM_RIGHT_PLAYBACK};

extern const char *g_pszPLAYREADY_KEYSYSTEM;

// The following function is missing from the official PK 2.5 release but
// will be available in the next PK release.
// It should be removed if the source is building with the next PK release.
DRM_API DRM_RESULT DRM_CALL DRM_UTL_ReadNetworkBytesToNativeGUID(
   __in_bcount( cbData ) const DRM_BYTE  *pbData,
   __in                  const DRM_DWORD  cbData,
   __in                        DRM_DWORD  ibGuidOffset,
   __out                       DRM_GUID  *pDrmGuid )
{
    DRM_RESULT dr       = DRM_SUCCESS;
    DRM_DWORD  dwResult = 0;

    ChkArg( pbData != NULL );
    ChkArg( pDrmGuid != NULL );
    ChkOverflow( cbData, ibGuidOffset );
    ChkDR( DRM_DWordSub( cbData, ibGuidOffset, &dwResult ) );
    ChkBOOL( dwResult >= DRM_GUID_LEN, DRM_E_BUFFERTOOSMALL );

    // Convert field by field.
    NETWORKBYTES_TO_DWORD( pDrmGuid->Data1, pbData, ibGuidOffset );
    ChkDR( DRM_DWordAdd( ibGuidOffset, SIZEOF( DRM_DWORD ), &ibGuidOffset ) );

    NETWORKBYTES_TO_WORD( pDrmGuid->Data2,  pbData, ibGuidOffset );
    ChkDR( DRM_DWordAdd( ibGuidOffset, SIZEOF( DRM_WORD ), &ibGuidOffset ) );

    NETWORKBYTES_TO_WORD( pDrmGuid->Data3, pbData, ibGuidOffset );
    ChkDR( DRM_DWordAdd( ibGuidOffset, SIZEOF( DRM_WORD ), &ibGuidOffset ) );

    // Copy last 8 bytes.
    DRM_BYT_CopyBytes( pDrmGuid->Data4, 0, pbData, ibGuidOffset, 8 );

ErrorExit :
    return dr;
}

// Constructor.
CMediaKeySession::CMediaKeySession(void) :
    m_pbOpaqueBuffer(NULL),
    m_cbOpaqueBuffer(0),
    m_pbRevocationBuffer(NULL),
    m_pbPRO(NULL),
    m_cbPRO(0),
    m_fKeyIdSet(FALSE),
    m_pchCustomData(NULL),
    m_cchCustomData(0),
    m_piCallback(NULL),
    m_eKeyState(KEY_CLOSED),
    m_fCommit(FALSE),
    m_pOEMContext(NULL)
{
    ZEROMEM(&m_oKeyId, SIZEOF(m_oKeyId));
}

// Destructor.
CMediaKeySession::~CMediaKeySession(void)
{
    Close();
}

#define ChkBufferSize(a,b) do { \
    ChkBOOL((a) <= (b), DRM_E_FAIL); \
} while(FALSE)

// The standard PlayReady protection system ID.
static DRM_ID CLSID_PlayReadyProtectionSystemID =
    {{0x79, 0xf0, 0x04, 0x9a,
     0x40, 0x98,
     0x86, 0x42,
     0xab, 0x92, 0xe6, 0x5b, 0xe0, 0x88, 0x5f, 0x95}};

// The standard ID of the PSSH box wrapped inside of a UUID box.
static DRM_ID PSSH_BOX_GUID =
    {{0x18, 0x4f, 0x8a, 0xd0,
     0xf3, 0x10,
     0x82, 0x4a,
     0xb6, 0xc8, 0x32, 0xd8, 0xab, 0xa1, 0x83, 0xd3}};

// Retrieve PRO from init data.
CDMi_RESULT CMediaKeySession::_GetPROFromInitData(
    __in_bcount(f_cbInitData) const DRM_BYTE *f_pbInitData,
    __in DRM_DWORD f_cbInitData,
    __out DRM_DWORD *f_pibPRO,
    __out DRM_DWORD *f_pcbPRO)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_DWORD ibCur = 0;
    DRM_DWORD cbSize = 0;
    DRM_DWORD dwType = 0;
    DRM_WORD wVersion = 0;
    DRM_WORD wFlags = 0;
    DRM_GUID guidSystemID = EMPTY_DRM_GUID;
    DRM_GUID guidUUID = EMPTY_DRM_GUID;
    DRM_DWORD cbSystemSize = 0;
    DRM_BOOL fFound = FALSE;
    DRM_BOOL fUUIDBox = FALSE;
    DRM_DWORD cbMultiKid = 0;
    DRM_DWORD dwResult = 0;

    ChkArg(f_pbInitData != NULL);
    ChkArg(f_cbInitData > 0);
    ChkArg(f_pibPRO != NULL);
    ChkArg(f_pcbPRO != NULL);

    *f_pibPRO = 0;
    *f_pcbPRO = 0;

    while (ibCur < f_cbInitData && !fFound)
    {
        ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_DWORD), &dwResult));
        ChkBufferSize(dwResult, f_cbInitData);
        NETWORKBYTES_TO_DWORD(cbSize, f_pbInitData, ibCur);
        ibCur = dwResult;

        ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_DWORD), &dwResult));
        ChkBufferSize(dwResult, f_cbInitData);
        NETWORKBYTES_TO_DWORD(dwType, f_pbInitData, ibCur);
        ibCur = dwResult;

        // 0x64697575 in big endian stands for "uuid".
        if (dwType == 0x75756964)
        {
            ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_GUID), &dwResult));
            ChkBufferSize(dwResult, f_cbInitData);

            ChkDR(DRM_UTL_ReadNetworkBytesToNativeGUID(f_pbInitData,
                                                       f_cbInitData,
                                                       ibCur,
                                                       &guidUUID));
            ibCur = dwResult;

            ChkBOOL(MEMCMP(&guidUUID, &PSSH_BOX_GUID, SIZEOF(DRM_ID)) == 0, DRM_E_FAIL);
            fUUIDBox = TRUE;
        }
        else
        {
            // 0x68737370 in big endian stands for "pssh".
            ChkBOOL(dwType == 0x70737368, DRM_E_FAIL);
        }

        // Read "version" of PSSH box.
        ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_WORD), &dwResult));
        ChkBufferSize(dwResult, f_cbInitData);
        NETWORKBYTES_TO_WORD(wVersion, f_pbInitData, ibCur);
        ibCur = dwResult;

        // Read "flags" of PSSH box.
        ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_WORD), &dwResult));
        ChkBufferSize(dwResult, f_cbInitData);
        NETWORKBYTES_TO_WORD(wFlags, f_pbInitData, ibCur);
        ibCur = dwResult;

        ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_GUID), &dwResult));
        ChkBufferSize(dwResult, f_cbInitData);

        // Read "system ID" of PSSH box.
        ChkDR(DRM_UTL_ReadNetworkBytesToNativeGUID(f_pbInitData,
                                                   f_cbInitData,
                                                   ibCur,
                                                   &guidSystemID));
        ibCur = dwResult;

        // Handle multi-KIDs pssh box.
        if (wVersion > 0)
        {
            DRM_DWORD cKids = 0;

            ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_DWORD), &dwResult));
            ChkBufferSize(dwResult, f_cbInitData);
            NETWORKBYTES_TO_DWORD(cKids, f_pbInitData, ibCur);
            ibCur = dwResult;

            // Ignore the KIDs.
            // ibCur + cKids * sizeof( GUID )
            ChkDR(DRM_DWordMult(cKids, SIZEOF(DRM_GUID), &dwResult));
            ChkDR( DRM_DWordAdd(ibCur, dwResult, &dwResult));
            ChkBufferSize(dwResult, f_cbInitData);
            ibCur = dwResult;

            cbMultiKid = SIZEOF(DRM_DWORD) + (cKids * SIZEOF(DRM_GUID));
        }

        ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_DWORD), &dwResult));
        ChkBufferSize(dwResult, f_cbInitData);
        NETWORKBYTES_TO_DWORD(cbSystemSize, f_pbInitData, ibCur);
        ibCur = dwResult;

        // Make sure the payload is still within the limit.
        ChkDR(DRM_DWordAdd(ibCur, cbSystemSize, &dwResult));
        ChkBufferSize(dwResult, f_cbInitData);

        // Check whether the "system ID" just read is for PlayReady.
        if (MEMCMP(&guidSystemID, &CLSID_PlayReadyProtectionSystemID, SIZEOF(DRM_GUID)) == 0)
        {
            fFound = TRUE;
        }
        else
        {
            ibCur  = dwResult;
        }
    }

    if (!fFound)
    {
        ChkDR(DRM_E_FAIL);
    }

    // Make sure the total size of all components
    // match the overall size.
    if (fUUIDBox)
    {
        ChkBOOL(cbSystemSize     + SIZEOF(cbSize)   +
                SIZEOF(dwType)   + SIZEOF(DRM_GUID) +
                SIZEOF(wVersion) + SIZEOF(wFlags)   +
                SIZEOF(DRM_GUID) + SIZEOF(cbSystemSize) == cbSize, DRM_E_FAIL);
    }
    else
    {
        ChkBOOL(cbSystemSize          + SIZEOF(cbSize)   +
                SIZEOF(dwType)        + SIZEOF(wVersion) +
                 SIZEOF(wFlags)       + SIZEOF(DRM_GUID) +
                 SIZEOF(cbSystemSize) + cbMultiKid == cbSize, DRM_E_FAIL);
    }

    *f_pibPRO = ibCur;
    *f_pcbPRO = cbSystemSize;

ErrorExit:
    return dr;
}

// Parse init data to retrieve PRO from it.
CDMi_RESULT CMediaKeySession::_ParseInitData(
    __in_bcount(f_cbInitData) const uint8_t *f_pbInitData,
    __in uint32_t f_cbInitData)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_DWORD ibPRO = 0;
    DRM_BYTE *pbPRO = NULL;
    DRM_DWORD cbPRO = 0;

    ChkArg(f_pbInitData != NULL && f_cbInitData > 0);

    // If key ID is already specified by CDM data then PRO is
    // not allowed to be specified in init data.
    // In the current implementation this should never happen
    // since init data is always processed before CDM data.
    DRMASSERT(!m_fKeyIdSet);

    // Parse init data to retrieve PRO.
    ChkDR(_GetPROFromInitData(f_pbInitData,
                              f_cbInitData,
                              &ibPRO,
                              &cbPRO));
    ChkBOOL(cbPRO > 0, DRM_E_FAIL);
    ChkMem(pbPRO = (DRM_BYTE *)Oem_MemAlloc(cbPRO));

    MEMCPY(pbPRO, f_pbInitData + ibPRO, cbPRO);

    m_cbPRO = cbPRO;
    m_pbPRO = pbPRO;

ErrorExit:
    return dr;
}

static DRM_ANSI_CONST_STRING g_dastrCDMDataKeyIdPath = CREATE_DRM_ANSI_STRING("LicenseAcquisition/KeyIDs/KeyID");
static DRM_ANSI_CONST_STRING g_dastrCDMDataCustomDataPath = CREATE_DRM_ANSI_STRING("LicenseAcquisition/CustomData");

//
// Sample PlayReady key system CDMData:
// <PlayReadyCDMData type="LicenseAcquisition">
//     <LicenseAcquisition version="1.0" Proactive="true">
//         <KeyIDs>
//             <KeyID>B64 encoded key ID</KeyID>
//         </KeyIDs>
//         <CustomData encoding="base64encoded">
//             B64 encoded custom data
//         </CustomData>
//     </LicenseAcquisition>
// </PlayReadyCDMData>
//
// Note:
// * CDM data uses UTF-8 encoding.
// * The Proactive attribute is ignored.
// * Only a single KeyID is allowed (Win8.1 allows more than one).
//
// Parse CDM data to retrieve key ID and custom data (if exists).
//
CDMi_RESULT CMediaKeySession::_ParseCDMData(
    __in_bcount(f_cbCDMData) const uint8_t *f_pbCDMData,
    __in uint32_t f_cbCDMData)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_SUBSTRING dasstrCDMData;
    DRM_SUBSTRING dasstrKeyId = EMPTY_DRM_SUBSTRING;
    DRM_DWORD cbKeyId = SIZEOF(DRM_ID);
    DRM_SUBSTRING dasstrCustomData = EMPTY_DRM_SUBSTRING;
    DRM_CHAR *pchCustomData = NULL;
    DRM_DWORD cchCustomData = 0;

    ChkArg(f_pbCDMData != NULL && f_cbCDMData > 0);

    dasstrCDMData.m_ich = 0;
    dasstrCDMData.m_cch = f_cbCDMData;

    // Parse CDM data to retrieve key ID.
    dr = DRM_XML_GetSubNodeByPathA((DRM_CHAR *)f_pbCDMData,
                                    &dasstrCDMData,
                                    &g_dastrCDMDataKeyIdPath,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &dasstrKeyId,
                                    g_chForwardSlash);
    if (dr == DRM_E_XMLNOTFOUND)
    {
        // It's fine that <KeyID> node does not exist.
        dr = DRM_SUCCESS;
    }
    else
    {
        ChkDR(dr);

        if (dasstrKeyId.m_cch > 0)
        {
            // If PRO is already specified by init data then key is
            // not allowed to be specified in CDM data.
            ChkBOOL(m_cbPRO == 0, DRM_E_FAIL);

            ChkDR(DRM_B64_DecodeA((DRM_CHAR *)f_pbCDMData,
                                  &dasstrKeyId,
                                  &cbKeyId,
                                  (DRM_BYTE *)&m_oKeyId,
                                  0));

            m_fKeyIdSet = TRUE;
        }
    }

    dr = DRM_XML_GetSubNodeByPathA((DRM_CHAR *)f_pbCDMData,
                                    &dasstrCDMData,
                                    &g_dastrCDMDataCustomDataPath,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &dasstrCustomData,
                                    g_chForwardSlash);

    if (dr == DRM_E_XMLNOTFOUND)
    {
        // It's fine that <CustomData> node does not exist.
        dr = DRM_SUCCESS;
    }
    else
    {
        ChkDR(dr);

        if (dasstrCustomData.m_cch > 0)
        {
            // Parse CDM data to retrieve custom data.
            cchCustomData = CB_BASE64_DECODE(dasstrCustomData.m_cch);
            ChkMem(pchCustomData = (DRM_CHAR *)Oem_MemAlloc(cchCustomData));
            ChkDR(DRM_B64_DecodeA((DRM_CHAR *)f_pbCDMData,
                                  &dasstrCustomData,
                                  &cchCustomData,
                                  (DRM_BYTE *)pchCustomData,
                                  0));
        }
    }

    m_cchCustomData = cchCustomData;
    m_pchCustomData = pchCustomData;

ErrorExit:
    return dr;
}

static const DRM_CHAR *g_pszKeyMessagePrefix = "<PlayReadyKeyMessage type=\"LicenseAcquisition\"><LicenseAcquisition version=\"1.0\"><Challenge encoding=\"base64encoded\">";
static const DRM_CHAR *g_pszKeyMessageSuffix = "</Challenge><HttpHeaders><HttpHeader><name>Content-Type</name><value>text/xml; charset=utf-8</value></HttpHeader><HttpHeader><name>SOAPAction</name><value>http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense</value></HttpHeader></HttpHeaders></LicenseAcquisition></PlayReadyKeyMessage>";

//
// Sample key message:
// <PlayReadyKeyMessage type="LicenseAcquisition">
//     <LicenseAcquisition version="1.0">
//         <Challenge encoding="base64encoded">
//             B64 encoded license acquisition challenge
//         </Challenge>
//         <HttpHeaders>
//             <HttpHeader>
//                <name>Content-Type</name>
//                <value>text/xml; charset=utf-8</value>
//             </HttpHeader>
//             <HttpHeader>
//                <name>SOAPAction</name>
//                <value>http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense</value>
//             </HttpHeader>
//         </HttpHeaders>
//     </LicenseAcquisition>
// </PlayReadyKeyMessage>
//
// Build a key message using the supplied license challenge.
//
CDMi_RESULT CMediaKeySession::_BuildKeyMessage(
    __in_bcount(f_cbChallenge) const DRM_BYTE *f_pbChallenge,
    __in DRM_DWORD f_cbChallenge,
    __deref_out_bcount(*f_pcbKeyMessage) DRM_BYTE **f_ppbKeyMessage,
    __out DRM_DWORD *f_pcbKeyMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_BYTE *pbKeyMessage = NULL;
    DRM_DWORD cbKeyMessage = 0;
    DRM_DWORD cbPrefix = DRMCRT_strlen(g_pszKeyMessagePrefix);
    DRM_DWORD cbSuffix = DRMCRT_strlen(g_pszKeyMessageSuffix);
    DRM_DWORD dwIdx = 0;
    DRM_DWORD cchEncodedChallenge = CCH_BASE64_EQUIV(f_cbChallenge);

    ChkArg(f_pbChallenge != NULL && f_cbChallenge > 0);
    ChkArg(f_ppbKeyMessage != NULL && f_pcbKeyMessage != NULL);

    cbKeyMessage = cbPrefix + cchEncodedChallenge + cbSuffix;

    ChkMem(pbKeyMessage = (DRM_BYTE *)Oem_MemAlloc(cbKeyMessage));
    MEMCPY(pbKeyMessage, g_pszKeyMessagePrefix, cbPrefix);
    dwIdx += cbPrefix;

    // Base64 encode the license challenge before adding it to
    // the key message XML being built.
    ChkDR(DRM_B64_EncodeA(f_pbChallenge,
                          f_cbChallenge,
                          (DRM_CHAR *)(pbKeyMessage + dwIdx),
                          &cchEncodedChallenge,
                          0 ) );
    dwIdx += cchEncodedChallenge;
    MEMCPY(pbKeyMessage + dwIdx, g_pszKeyMessageSuffix, cbSuffix);

    *f_ppbKeyMessage = pbKeyMessage;
    *f_pcbKeyMessage = cbKeyMessage;

ErrorExit:
    if (DRM_FAILED(dr))
    {
        SAFE_OEM_FREE(pbKeyMessage);
    }
    return dr;
}

// Encrypt a block of data in place using the supplied AES session key.
CDMi_RESULT CMediaKeySession::_EncryptDataUsingSessionKey(
    __in_bcount(f_cbSessionKey) const DRM_BYTE *f_pbSessionKey,
    __in DRM_DWORD f_cbSessionKey,
    __inout_bcount(f_cbData) uint8_t *f_pbData,
    __in uint32_t f_cbData)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_AES_KEY oAESKey = {{0}};
    DRM_AES_COUNTER_MODE_CONTEXT oAESCtrContext = {0, 0, 0};

    ChkArg(f_pbSessionKey != NULL && f_cbSessionKey == DRM_AES_KEYSIZE_128);
    ChkArg(f_pbData != NULL && f_cbData > 0);

     // Reencrypt the data using the AES session key.
    ChkDR(Oem_Aes_SetKey(f_pbSessionKey, &oAESKey));

    ChkDR(Oem_Aes_CtrProcessData(&oAESKey,
                                 f_pbData,
                                 f_cbData,
                                 &oAESCtrContext));

ErrorExit:
    return dr;
}

// Map PlayReady specific CDMi error to one of the EME errors.
int16_t CMediaKeySession::_MapCDMiError(
    __in CDMi_RESULT f_crError)
{
    int16_t nError = MEDIA_KEYERR_UNKNOWN;

    switch (f_crError)
    {
        case CDMi_E_SERVER_INTERNAL_ERROR:
        case CDMi_E_SERVER_INVALID_MESSAGE:
        case CDMi_E_SERVER_SERVICE_SPECIFIC:
            nError = MEDIA_KEYERR_SERVICE;
            break;

        case CDMi_SUCCESS:
        case CDMi_S_FALSE:
            nError = 0;
            break;
    }

    return nError;
}

// PlayReady license policy callback which should be
// customized for platform/environment that hosts the CDM.
// It is currently implemented as a place holder that
// does nothing.
DRM_RESULT DRM_CALL CMediaKeySession::_PolicyCallback(
    __in const DRM_VOID *f_pvOutputLevelsData,
    __in DRM_POLICY_CALLBACK_TYPE f_dwCallbackType,
    __in const DRM_VOID *f_pv)
{
    /*!+!hla fix this, implement for something. */
    return DRM_SUCCESS;
}

bool CMediaKeySession::load_revocation_list(
    const char                *revListFile
    )
{
    DRM_RESULT dr = DRM_SUCCESS;
    FILE    * fRev;
    uint8_t * revBuf = NULL;
    size_t    fileSize = 0;
    uint32_t  currSize = 0;

    BDBG_ASSERT(revListFile != NULL);

    fRev = fopen(revListFile, "rb");
    if( fRev == NULL)
    {
        return true;
    }

    /* get the size of the file */
    fseek(fRev, 0, SEEK_END);
    fileSize = ftell(fRev);
    fseek(fRev, 0, SEEK_SET);

    revBuf = (uint8_t *)BKNI_Malloc(fileSize);
    if( revBuf == NULL)
    {
        goto ErrorExit;
    }

    BKNI_Memset(revBuf, 0x00, fileSize);

    for(;;) {
        uint8_t buf[512];
        int rc = fread(buf, 1, sizeof(buf), fRev);
        if(rc<=0) {
            break;
        }
        BKNI_Memcpy(revBuf+currSize, buf, rc);
        currSize += rc;
    }

    ChkDR( Drm_Revocation_StorePackage(
                m_poAppContext,
                ( DRM_CHAR * )revBuf,
                fileSize ) );

    if( revBuf != NULL)
        BKNI_Free(revBuf);

    return true;

ErrorExit:
    if( revBuf != NULL)
        BKNI_Free(revBuf);

    return false;
}

bool CMediaKeySession::convertCStringToWString(const char * pCStr, DRM_WCHAR * pWStr, DRM_DWORD * cchStr)
{
    DRM_RESULT         dr = DRM_SUCCESS;
    bool               result = false;
    DRM_SUBSTRING      tmpSubStr;
    DRM_CHAR           tmpCStr[ DRM_MAX_PATH ];
    DRM_WCHAR          tmpWChar[ DRM_MAX_PATH ];
    DRM_STRING         tmpWStr = EMPTY_DRM_STRING;

    if(( pCStr != NULL) && (pWStr != NULL))
    {
        /* Convert the given char * to DRM_CHAR * */
        ZEROMEM(tmpCStr,DRM_MAX_PATH);
        ChkDR( DRM_STR_StringCchCopyA(
                 tmpCStr,
                 SIZEOF(tmpCStr),
                 pCStr) );

        tmpSubStr.m_ich = 0;
        tmpSubStr.m_cch = strlen( (char*)tmpCStr );

        /* Convert the DRM_CHAR * to DRM_STRING. */
        tmpWStr.pwszString = tmpWChar;
        tmpWStr.cchString  = DRM_MAX_PATH;
        DRM_UTL_PromoteASCIItoUNICODE( tmpCStr,
                                       &tmpSubStr,
                                       &tmpWStr);

        BKNI_Memcpy(pWStr, tmpWStr.pwszString, tmpWStr.cchString * SIZEOF (DRM_WCHAR));
        *cchStr = tmpWStr.cchString;
        result = true;
    }

ErrorExit:
    return result;
}

// Init media key session using supplied init data and CDM data
// (both are optional).
CDMi_RESULT CMediaKeySession::Init(
    __in_bcount_opt(f_cbInitData) const uint8_t *f_pbInitData,
    __in uint32_t f_cbInitData,
    __in_bcount_opt(f_cbCDMData) const uint8_t *f_pbCDMData,
    __in uint32_t f_cbCDMData)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_ID oSessionID;
    DRM_DWORD cchEncodedSessionID = SIZEOF(m_rgchSessionID);
    DRM_WCHAR          rgwchHDSPath[ DRM_MAX_PATH ];
    DRM_CONST_STRING   dstrHDSPath = EMPTY_DRM_STRING;
    NEXUS_ClientConfiguration platformConfig;
    OEM_Settings         oemSettings;

    // The current state MUST be KEY_CLOSED otherwise error out.
    ChkBOOL(m_eKeyState == KEY_CLOSED, DRM_E_INVALIDARG);

    ChkArg((f_pbInitData == NULL) == (f_cbInitData == 0));
    ChkArg((f_pbCDMData == NULL) == (f_cbCDMData == 0));

    if (f_pbInitData != NULL)
    {
        ChkDR(_ParseInitData(f_pbInitData, f_cbInitData));
    }

    if (f_pbCDMData != NULL)
    {
        ChkDR(_ParseCDMData(f_pbCDMData, f_cbCDMData));
    }

    /* Drm_Platform_Initialize */
    NEXUS_Memory_GetDefaultAllocationSettings(&m_heapSettings);
    NEXUS_Platform_GetClientConfiguration(&platformConfig);
    if (platformConfig.heap[NXCLIENT_FULL_HEAP])
    {
        NEXUS_HeapHandle heap = platformConfig.heap[NXCLIENT_FULL_HEAP];
        NEXUS_MemoryStatus heapStatus;
        NEXUS_Heap_GetStatus(heap, &heapStatus);
        if (heapStatus.memoryType & NEXUS_MemoryType_eFull)
        {
            m_heapSettings.heap = heap;
        }
    }

    BKNI_Memset(&oemSettings, 0, sizeof(OEM_Settings));

    oemSettings.binFileName = NULL;
    oemSettings.keyFileName = NULL;
    oemSettings.keyHistoryFileName = NULL;
    oemSettings.defaultRWDirName = NULL;
    oemSettings.heap = m_heapSettings.heap;

/* Handle drmType for three cases: Non-SAGE, SAGE2x, SAGE3.2+ */
#if SAGE_SUPPORT
#if !SAGE_VERSION_IS_2X
    oemSettings.drmType = BSAGElib_BinFileDrmType_ePlayready;
#endif
#else
    oemSettings.drmType = 0;
#endif

    m_pOEMContext = Drm_Platform_Initialize(&oemSettings);
    ChkMem(m_pOEMContext);

    ChkMem(m_pbOpaqueBuffer = (DRM_BYTE *)Oem_MemAlloc(MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE));
    m_cbOpaqueBuffer = MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE;

    ChkMem(m_poAppContext = (DRM_APP_CONTEXT *)Oem_MemAlloc(SIZEOF(DRM_APP_CONTEXT)));

    dstrHDSPath.pwszString = rgwchHDSPath;
    dstrHDSPath.cchString = DRM_MAX_PATH;
    if( !convertCStringToWString(DRM_LICENSE_STORAGE_FILE, (DRM_WCHAR*)dstrHDSPath.pwszString, &dstrHDSPath.cchString))
    {
        goto ErrorExit;
    }

    // Initialize DRM app context.
    ChkDR(Drm_Initialize(m_poAppContext,
                         m_pOEMContext,
                         m_pbOpaqueBuffer,
                         m_cbOpaqueBuffer,
                         &dstrHDSPath));
    if (DRM_REVOCATION_IsRevocationSupported())
    {
        ChkMem(m_pbRevocationBuffer = (DRM_BYTE *)Oem_MemAlloc(REVOCATION_BUFFER_SIZE));

        ChkDR(Drm_Revocation_SetBuffer(m_poAppContext,
                                       m_pbRevocationBuffer,
                                       REVOCATION_BUFFER_SIZE));
       if( !load_revocation_list(DRM_DEFAULT_REVOCATION_LIST_FILE))
       {
           goto ErrorExit;
       }
    }

    // Generate a random media session ID.
    ChkDR(Oem_Random_GetBytes((DRM_BYTE *)&oSessionID, SIZEOF(oSessionID)));
    ZEROMEM(m_rgchSessionID, SIZEOF(m_rgchSessionID));
    // Store the generated media session ID in base64 encoded form.
    ChkDR(DRM_B64_EncodeA((DRM_BYTE *)&oSessionID,
                          SIZEOF(oSessionID),
                          m_rgchSessionID,
                          &cchEncodedSessionID,
                          0));

    m_eKeyState = KEY_INIT;

ErrorExit:
    if (DRM_FAILED(dr))
    {
        m_eKeyState = KEY_ERROR;
    }
    return dr;
}

// Decrypt a block of data in place and immediately re-encrypt it
// using a supplied session key before returning the data to the
// caller.
CDMi_RESULT CMediaKeySession::Decrypt(
    __in_bcount(f_cbSessionKey) const DRM_BYTE *f_pbSessionKey,
    __in DRM_DWORD f_cbSessionKey,
    __in_bcount(f_cbIV) const uint8_t *f_pbIV,
    __in uint32_t f_cbIV,
    __inout_bcount(f_cbData) uint8_t *f_pbData,
    __in uint32_t f_cbData,
    __in_ecount_opt(f_cdwSubSampleMapping) const uint32_t *f_pdwSubSampleMapping,
    __in uint32_t f_cdwSubSampleMapping)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_AES_COUNTER_MODE_CONTEXT oAESContext = {0, 0, 0};
    DRM_BYTE *pbData = NULL;
    DRM_DWORD cbData = 0;

    NEXUS_Error                    rc = NEXUS_SUCCESS;

    // The current state MUST be KEY_READY otherwise error out.
    ChkBOOL(m_eKeyState == KEY_READY, DRM_E_INVALIDARG);

    ChkArg(f_pbSessionKey != NULL && f_cbSessionKey == DRM_AES_KEYSIZE_128);
    ChkArg(f_pbIV != NULL && f_cbIV == SIZEOF(DRM_UINT64));
    ChkArg(f_pbData != NULL && f_cbData > 0);

    // Subsample mapping is an array of integer pairs with the following properties:
    // 1. Each pair has two integers: <clear size><encrypted size>
    // 2. Total value of all integers should be the same as f_cbIV.
    // 3. f_cdwSubSampleMapping is the number of integer pairs times 2.
    if (f_cdwSubSampleMapping > 0)
    {
        DRM_DWORD cbTotal = 0;
        DRM_DWORD i;
        DRM_BYTE *pbCurrTarget;
        DRM_DWORD iCurrSource = 0;

        ChkArg(f_cdwSubSampleMapping % 2 == 0);

        // If subsample mapping is used, collect all encrypted bytes into
        // a continuous buffer.
        for (i = 0 ; i < f_cdwSubSampleMapping; i++)
        {
            cbTotal += f_pdwSubSampleMapping[i];
            i++;
            cbTotal += f_pdwSubSampleMapping[i];
            cbData += f_pdwSubSampleMapping[i];
        }

        // Make sure the sum of all subsample sizes is the same as
        // the size of buffer to be decrypted.
        ChkArg(cbTotal == f_cbData);

        // Make sure the total size of encrypted data is non-zero.
        ChkArg(cbData > 0);

        // Allocate a temporary buffer to store all encrypted bytes.
        rc = NEXUS_Memory_Allocate(cbData, &m_heapSettings, (void **)(&pbData));
        ChkMem(pbData);

        pbCurrTarget = pbData;

        for (i = 0 ; i < f_cdwSubSampleMapping; i++)
        {
            // Skip the clear byte range from source buffer.
            iCurrSource += f_pdwSubSampleMapping[i++];

            // Copy cipher bytes from f_pbData to target buffer.
            MEMCPY(pbCurrTarget, f_pbData + iCurrSource, f_pdwSubSampleMapping[i]);

            // Adjust current pointer of target buffer.
            pbCurrTarget += f_pdwSubSampleMapping[i];

            // Adjust current offset of source buffer.
            iCurrSource += f_pdwSubSampleMapping[i];
        }
    }
    else
    {
        pbData = f_pbData;
        cbData = f_cbData;
    }

    ChkDR(Drm_Reader_InitDecrypt(&m_oDecryptContext, NULL, 0));

    MEMCPY(&oAESContext.qwInitializationVector, f_pbIV, f_cbIV);
    ChkDR(Drm_Reader_Decrypt(&m_oDecryptContext, &oAESContext, pbData, cbData));

    if (f_cdwSubSampleMapping > 0)
    {
        // If subsample mapping is used, copy decrypted bytes back
        // to the original buffer.
        DRM_BYTE *pbCurrTarget = f_pbData;
        DRM_DWORD iCurrSource = 0;

        for (DRM_DWORD i = 0 ; i < f_cdwSubSampleMapping; i++)
        {
            // Skip the clear byte range from target buffer.
            pbCurrTarget += f_pdwSubSampleMapping[i++];

            // Copy decrypted bytes from pbData to target buffer.
            MEMCPY(pbCurrTarget, pbData + iCurrSource, f_pdwSubSampleMapping[i]);

            // Adjust current pointer of target buffer.
            pbCurrTarget += f_pdwSubSampleMapping[i];

            // Adjust current offset of source buffer.
            iCurrSource += f_pdwSubSampleMapping[i];
        }
    }

    ChkDR(_EncryptDataUsingSessionKey(f_pbSessionKey, f_cbSessionKey, f_pbData, f_cbData));
    // Call commit during the decryption of the first sample.
    if (!m_fCommit)
    {
        ChkDR(Drm_Reader_Commit(m_poAppContext, _PolicyCallback, NULL));
        m_fCommit = TRUE;
    }
ErrorExit:
    if (f_cdwSubSampleMapping > 0)
    {
        // If subsample mapping is used, release the temporary buffer.
        NEXUS_Memory_Free(pbData);
    }
    return dr;
}

// Kick off the license acquisition workflow. Caller is supposed
// to receive status callback via the supplied callback interface.
void CMediaKeySession::Run(
    __in const IMediaKeySessionCallback *f_piMediaKeySessionCallback)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_BYTE *pbChallenge = NULL;
    DRM_DWORD cbChallenge = 0;
    DRM_CHAR *pchSilentURL = NULL;
    DRM_DWORD cchSilentURL = 0;
    DRM_BYTE *pbKeyMessage = NULL;
    DRM_DWORD cbKeyMessage = 0;

    // The current state MUST be KEY_INIT otherwise error out.
    ChkBOOL(m_eKeyState == KEY_INIT, DRM_E_INVALIDARG);

    ChkArg(f_piMediaKeySessionCallback != NULL);

    m_piCallback = const_cast<IMediaKeySessionCallback *>(f_piMediaKeySessionCallback);

    if (m_cbPRO > 0)
    {
        // If PRO is supplied (via init data) then it is used
        // to create the content header inside of the app context.
        ChkDR(Drm_Content_SetProperty(m_poAppContext,
                                      DRM_CSP_AUTODETECT_HEADER,
                                      m_pbPRO,
                                      m_cbPRO));
    }
    else if (m_fKeyIdSet)
    {
        // If a key ID is supplied (via CDM data) then it is used
        // to create the content header inside of the app context.
        DRM_WCHAR rgwchB64EncodedKID[CCH_BASE64_EQUIV(SIZEOF(m_oKeyId))];
        DRM_CSP_HEADER_COMPONENTS_DATA oKIDData;
        BKNI_Memset(&oKIDData, 0, sizeof(oKIDData));
        oKIDData.eHeaderVersion = DRM_HEADER_VERSION_4;
        oKIDData.eCipherType = eDRM_AES_COUNTER_CIPHER;

        DRM_DWORD cchB64EncodedKID = SIZEOF(rgwchB64EncodedKID) / SIZEOF(DRM_WCHAR);

        ChkDR(DRM_B64_EncodeW((DRM_BYTE *)&m_oKeyId,
                              SIZEOF(m_oKeyId),
                              rgwchB64EncodedKID,
                              &cchB64EncodedKID,
                              0));

        oKIDData.dstrKID.cchString = cchB64EncodedKID;
        oKIDData.dstrKID.pwszString = rgwchB64EncodedKID;

        ChkDR(Drm_Content_SetProperty(m_poAppContext,
                                      DRM_CSP_HEADER_COMPONENTS,
                                      (DRM_BYTE*)&oKIDData,
                                      SIZEOF(oKIDData)));
    }
    else
    {
        // Nothing is specified to prepare for the context of
        // license acquisition. Should fail here.
        ChkDR(DRM_E_FAIL);
    }

    if (DRM_SUCCEEDED(Drm_Reader_Bind(m_poAppContext,
                                      g_rgpdstrRights,
                                      NO_OF(g_rgpdstrRights),
                                      _PolicyCallback,
                                      NULL,
                                      &m_oDecryptContext)))
    {
        m_piCallback->OnKeyReady();
        m_eKeyState = KEY_READY;
        goto ErrorExit;
    }

    // Try to figure out the size of the license acquisition
    // challenge to be returned.
    dr = Drm_LicenseAcq_GenerateChallenge(m_poAppContext,
                                          g_rgpdstrRights,
                                          NO_OF(g_rgpdstrRights),
                                          NULL,
                                          m_pchCustomData,
                                          m_cchCustomData,
                                          NULL,
                                          m_fKeyIdSet ? NULL : &cchSilentURL,
                                          NULL,
                                          NULL,
                                          pbChallenge,
                                          &cbChallenge);
    if (dr == DRM_E_BUFFERTOOSMALL)
    {
        // Only retrieve LA URL if a PRO is specified.
        if (!m_fKeyIdSet && cchSilentURL > 0)
        {
            ChkMem(pchSilentURL = (DRM_CHAR *)Oem_MemAlloc(cchSilentURL + 1));
            ZEROMEM(pchSilentURL, cchSilentURL + 1);
        }

        // Allocate buffer that is sufficient to store the license acquisition
        // challenge.
        if (cbChallenge > 0)
        {
            ChkMem(pbChallenge = (DRM_BYTE *)Oem_MemAlloc(cbChallenge));
        }
        dr = DRM_SUCCESS;
    }
    else
    {
        ChkDR(dr);
    }

    // Supply a buffer to receive the license acquisition challenge.
    ChkDR(Drm_LicenseAcq_GenerateChallenge(m_poAppContext,
                                           g_rgpdstrRights,
                                           NO_OF(g_rgpdstrRights),
                                           NULL,
                                           m_pchCustomData,
                                           m_cchCustomData,
                                           m_fKeyIdSet ? NULL : pchSilentURL,
                                           m_fKeyIdSet ? NULL : &cchSilentURL,
                                           NULL,
                                           NULL,
                                           pbChallenge,
                                           &cbChallenge));

    // Build a key message XML to wrap the license acquisition challenge.
    ChkDR(_BuildKeyMessage(pbChallenge,
                           cbChallenge,
                           &pbKeyMessage,
                           &cbKeyMessage));

    m_eKeyState = KEY_PENDING;

    // Everything is OK and trigger a callback to let the caller
    // handle the key message.
    m_piCallback->OnKeyMessage(pbKeyMessage, cbKeyMessage, pchSilentURL);

ErrorExit:
    if (DRM_FAILED(dr))
    {
        if (m_piCallback != NULL)
        {
            m_piCallback->OnKeyError(_MapCDMiError(dr), dr);
            m_eKeyState = KEY_ERROR;
        }
    }

    SAFE_OEM_FREE(pbKeyMessage);
    SAFE_OEM_FREE(pbChallenge);
    SAFE_OEM_FREE(pchSilentURL);
}

// Process license acquisition response wrapped inside
// of the key message response. Caller receives the result
// of processing via the IMediaKeySessionCallback interface
// (supplied when CMediaKeySession::Run is called).
void CMediaKeySession::Update(
    __in_bcount(f_cbKeyMessageResponse) const uint8_t *f_pbKeyMessageResponse,
    __in uint32_t f_cbKeyMessageResponse)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oLicenseResponse;

    BKNI_Memset(&oLicenseResponse, 0, sizeof(oLicenseResponse));

    // The current state MUST be KEY_PENDING otherwise error out.
    ChkBOOL(m_eKeyState == KEY_PENDING, DRM_E_INVALIDARG);

    ChkArg(f_pbKeyMessageResponse != NULL && f_cbKeyMessageResponse > 0);

    ChkDR(Drm_LicenseAcq_ProcessResponse(m_poAppContext,
                                         DRM_PROCESS_LIC_RESPONSE_SIGNATURE_NOT_REQUIRED,
                                         NULL,
                                         NULL,
                                         const_cast<DRM_BYTE *>(f_pbKeyMessageResponse),
                                         f_cbKeyMessageResponse,
                                         &oLicenseResponse));

    ChkDR(Drm_Reader_Bind(m_poAppContext,
                          g_rgpdstrRights,
                          NO_OF(g_rgpdstrRights),
                          _PolicyCallback,
                          NULL,
                          &m_oDecryptContext));

ErrorExit:
    if (DRM_FAILED(dr))
    {
        m_piCallback->OnKeyError(_MapCDMiError(dr), dr);
        m_eKeyState = KEY_ERROR;
    }
    else
    {
        m_piCallback->OnKeyReady();
        m_eKeyState = KEY_READY;
    }
    return;
}

// Shut down a media key session by releasing all the associated
// resources.
void CMediaKeySession::Close(void)
{
    // The current state MUST be KEY_PENDING otherwise do nothing.
    if (m_eKeyState != KEY_CLOSED)
    {

        SAFE_OEM_FREE(m_pbPRO);
        m_cbPRO = 0;

        SAFE_OEM_FREE(m_pchCustomData);
        m_cchCustomData = 0;

        if (m_eKeyState == KEY_READY)
        {
            Drm_Reader_Close(&m_oDecryptContext);
        }

        Drm_Uninitialize(m_poAppContext);

        SAFE_OEM_FREE(m_pbOpaqueBuffer);
        m_cbOpaqueBuffer = 0;

        SAFE_OEM_FREE(m_poAppContext);
        SAFE_OEM_FREE(m_pbRevocationBuffer);

        Drm_Platform_Uninitialize(m_pOEMContext);
        m_piCallback = NULL;

        m_eKeyState = KEY_CLOSED;

        m_fCommit = FALSE;
    }
}

// Return a base64 encoded session ID.
const char *CMediaKeySession::GetSessionId(void) const
{
    return (const char *)m_rgchSessionID;
}

// Return a key system ID.
const char *CMediaKeySession::GetKeySystem(void) const
{
    return g_pszPLAYREADY_KEYSYSTEM;
}

} // namespace PRCDMi
