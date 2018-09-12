/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMMANAGER_H__
#define __DRMMANAGER_H__

#include <drmrevocationtypes.h>
#include <drmcontextsizes.h>
#include <drmenvelope.h>
#include <drmcallbacks.h>
#include <drmdomainimp.h>
#include <drmlicacqv3.h>
#include <drmmanagertypes.h>
#include <drmlicgentypes.h>
#include <drmm2tstypes.h>
#include <drmsecurestoptypes.h>
#include <drmsecuretimetypes.h>

PREFAST_PUSH_DISABLE_EXPLAINED( __WARNING_NONCONST_BUFFER_PARAM_25033, "Out params can't be const" )
PREFAST_PUSH_DISABLE_EXPLAINED( __WARNING_NONCONST_PARAM_25004, "Out params can't be const" )

ENTER_PK_NAMESPACE

DRM_API DRM_RESULT DRM_CALL Drm_Initialize(
    __in                                    DRM_APP_CONTEXT     *f_poAppContext,
    __in_opt                                DRM_VOID            *f_pOEMContext,
    __in_bcount( f_cbOpaqueBuffer )         DRM_BYTE            *f_pbOpaqueBuffer,
    __in                                    DRM_DWORD            f_cbOpaqueBuffer,
    __in                              const DRM_CONST_STRING    *f_pdstrDeviceStoreName );

DRM_API DRM_RESULT DRM_CALL Drm_ResizeOpaqueBuffer(
    __in                                    DRM_APP_CONTEXT     *f_poAppContext,
    __in_bcount( f_cbOpaqueBuffer )         DRM_BYTE            *f_pbOpaqueBuffer,
    __in                                    DRM_DWORD            f_cbOpaqueBuffer );

DRM_API DRM_RESULT DRM_CALL Drm_GetOpaqueBuffer(
    __in                                    DRM_APP_CONTEXT     *f_poAppContext,
    __deref_out_bcount(*f_pcbOpaqueBuffer)  DRM_BYTE           **f_ppbOpaqueBuffer,
    __out                                   DRM_DWORD           *f_pcbOpaqueBuffer );

DRM_API DRM_RESULT DRM_CALL Drm_ResizeInMemoryLicenseStore(
    __in                                    DRM_APP_CONTEXT     *f_poAppContext,
    __in                                    DRM_DWORD            f_cbInMemoryLicenseBuffer );

DRM_API_VOID DRM_VOID DRM_CALL Drm_Uninitialize(
    __in DRM_APP_CONTEXT *f_poAppContext );

DRM_API DRM_RESULT DRM_CALL Drm_Reinitialize(
    __in DRM_APP_CONTEXT *f_poAppContext );

DRM_API_VOID DRM_VOID DRM_CALL Drm_ClearAllCaches( DRM_VOID );

DRM_API DRM_RESULT DRM_CALL Drm_DisableBlobCache(
    __in    DRM_APP_CONTEXT     *f_poAppContext );

DRM_API DRM_RESULT DRM_CALL Drm_Content_SetProperty(
    __in                                      DRM_APP_CONTEXT          *f_poAppContext,
    __in                                      DRM_CONTENT_SET_PROPERTY  f_eProperty,
    __in_bcount_opt( f_cbPropertyData ) const DRM_BYTE                 *f_pbPropertyData,
    __in                                      DRM_DWORD                 f_cbPropertyData );

DRM_API DRM_RESULT DRM_CALL Drm_Content_GetProperty(
    __inout                                DRM_APP_CONTEXT          *f_poAppContext,
    __in                                   DRM_CONTENT_GET_PROPERTY  f_eProperty,
    __out_bcount_opt( *f_pcbPropertyData ) DRM_BYTE                 *f_pbPropertyData,
    __inout                                DRM_DWORD                *f_pcbPropertyData );

DRM_API DRM_RESULT DRM_CALL Drm_Content_UpdateEmbeddedStore(
    __in    DRM_APP_CONTEXT     *f_poAppContext );

DRM_API DRM_RESULT DRM_CALL Drm_Content_UpdateEmbeddedStore_Commit(
    __in const DRM_APP_CONTEXT     *f_poAppContext );

DRM_API DRM_RESULT DRM_CALL Drm_Device_GetProperty(
    __in                                DRM_APP_CONTEXT         *f_poAppContext,
    __in                                DRM_DEVICE_GET_PROPERTY  f_eProperty,
    __out_bcount_opt( *f_pcbProperty )  DRM_BYTE                *f_pbProperty,
    __inout                             DRM_DWORD               *f_pcbProperty );

/*
** ---------------------------------------
** License acquisition API functions
** ---------------------------------------
*/
DRM_API DRM_RESULT DRM_CALL Drm_LicenseAcq_GenerateChallenge(
    __in                                          DRM_APP_CONTEXT  *f_poAppContext,
    __in_ecount_opt( f_cRights )            const DRM_CONST_STRING *f_rgpdstrRights[ ],
    __in                                          DRM_DWORD         f_cRights,
    __in_opt                                const DRM_DOMAIN_ID    *f_poDomainID,
    __in_ecount_opt( f_cchCustomData )      const DRM_CHAR         *f_pchCustomData,
    __in                                          DRM_DWORD         f_cchCustomData,
    __out_ecount_opt( *f_pcchSilentURL )          DRM_CHAR         *f_pchSilentURL,
    __inout_opt                                   DRM_DWORD        *f_pcchSilentURL,
    __out_ecount_opt( *f_pcchNonSilentURL )       DRM_CHAR         *f_pchNonSilentURL,
    __inout_opt                                   DRM_DWORD        *f_pcchNonSilentURL,
    __out_bcount_opt( *f_pcbChallenge )           DRM_BYTE         *f_pbChallenge,
    __inout                                       DRM_DWORD        *f_pcbChallenge,
    __out_opt                                     DRM_ID           *f_poBatchID );

DRM_API DRM_RESULT DRM_CALL Drm_LicenseAcq_ProcessResponse(
    __in                              DRM_APP_CONTEXT                *f_poAppContext,
    __in                              DRM_PROCESS_LIC_RESPONSE_FLAG   f_dwFlags,
    __in_bcount( f_cbResponse ) const DRM_BYTE                       *f_pbResponse,
    __in                              DRM_DWORD                       f_cbResponse,
    __inout                           DRM_LICENSE_RESPONSE           *f_poLicenseResponse );

DRM_API DRM_RESULT DRM_CALL Drm_LicenseAcq_GenerateAck(
    __in                                DRM_APP_CONTEXT      *f_poAppContext,
    __in                                DRM_LICENSE_RESPONSE *f_poLicenseResponse,
    __out_bcount_opt( *f_pcbChallenge ) DRM_BYTE             *f_pbChallenge,
    __inout                             DRM_DWORD            *f_pcbChallenge );

DRM_API DRM_RESULT DRM_CALL Drm_LicenseAcq_ProcessAckResponse(
    __in                        const DRM_APP_CONTEXT *f_poAppContext,
    __in_bcount( f_cbResponse )       DRM_BYTE        *f_pbResponse,
    __in                              DRM_DWORD        f_cbResponse,
    __out_opt                         DRM_RESULT      *f_pResult );

DRM_API DRM_RESULT DRM_CALL Drm_GetAdditionalResponseData(
    __in                            const DRM_APP_CONTEXT  *f_poAppContext,
    __in_bcount( f_cbResponse )     const DRM_BYTE         *f_pbResponse,
    __in                                  DRM_DWORD         f_cbResponse,
    __in                                  DRM_DWORD         f_dwDataType,
    __out_ecount_opt( *f_pcchDataString ) DRM_CHAR         *f_pchDataString,
    __inout                               DRM_DWORD        *f_pcchDataString );

DRM_API DRM_RESULT DRM_CALL Drm_Reader_Bind(
    __in                           DRM_APP_CONTEXT      *f_poAppContext,
    __in_ecount( f_cRights ) const DRM_CONST_STRING     *f_rgpdstrRights[],
    __in                           DRM_DWORD             f_cRights,
    __in_opt                       DRMPFNPOLICYCALLBACK  f_pfnPolicyCallback,
    __in_opt                 const DRM_VOID             *f_pv,
    __out_opt                      DRM_DECRYPT_CONTEXT  *f_pcontextDCRY );

DRM_API_VOID DRM_VOID DRM_CALL Drm_Reader_Close(
    __in_ecount_opt( 1 ) DRM_DECRYPT_CONTEXT *f_pcontextDCRY );

DRM_API DRM_RESULT DRM_CALL Drm_Reader_Commit(
    __in           DRM_APP_CONTEXT      *f_poAppContext,
    __in_opt       DRMPFNPOLICYCALLBACK  f_pfnPolicyCallback,
    __in_opt const DRM_VOID             *f_pvCallbackData );

DRM_API DRM_RESULT DRM_CALL Drm_Reader_DecryptLegacy(
    __in_ecount( 1 )           DRM_DECRYPT_CONTEXT          *f_pDecryptContext,
    __inout                    DRM_AES_COUNTER_MODE_CONTEXT *f_pCtrContext,
    __inout_bcount( f_cbData ) DRM_BYTE                     *f_pbData,
    __in                       DRM_DWORD                     f_cbData );

DRM_API DRM_RESULT DRM_CALL Drm_Reader_DecryptOpaque(
    __in                                            DRM_DECRYPT_CONTEXT      *f_pDecryptContext,
    __in                                            DRM_DWORD                 f_cEncryptedRegionMappings,
    __in_ecount( f_cEncryptedRegionMappings ) const DRM_DWORD                *f_pdwEncryptedRegionMappings,
    __in                                            DRM_UINT64                f_ui64InitializationVector,
    __in                                            DRM_DWORD                 f_cbEncryptedContent,
    __in_bcount( f_cbEncryptedContent )       const DRM_BYTE                 *f_pbEncryptedContent,
    __out                                           DRM_DWORD                *f_pcbOpaqueClearContent,
    __deref_out_bcount( *f_pcbOpaqueClearContent )  DRM_BYTE                **f_ppbOpaqueClearContent );

DRM_NO_INLINE DRM_API_VOID DRM_VOID DRM_CALL DRM_Reader_FreeOpaqueDecryptedContent(
    __in                                     DRM_DECRYPT_CONTEXT     *f_pDecryptContext,
    __in                                     DRM_DWORD                f_cbOpaqueClearContent,
    __inout_bcount( f_cbOpaqueClearContent ) DRM_BYTE                *f_pbOpaqueClearContent );

DRM_API DRM_RESULT DRM_CALL Drm_License_GetProperty(
    __in                                        DRM_APP_CONTEXT           *f_poAppContext,
    __in                                        DRM_LICENSE_GET_PROPERTY   f_eProperty,
    __inout_bcount_opt( *f_pcbExtraData )       DRM_BYTE                  *f_pbExtraData,
    __in_opt                              const DRM_DWORD                 *f_pcbExtraData,
    __out_opt                                   DRM_DWORD                 *f_pdwOutputData );

DRM_API DRM_RESULT DRM_CALL Drm_PlayReadyObject_ConvertFromWmdrmHeader(
    __in_bcount( f_cbWmdrmHeader )  const DRM_BYTE  *f_pbWmdrmHeader,
    __in                            const DRM_DWORD  f_cbWmdrmHeader,
    __in_ecount_nz_opt( f_cchPlayReadySilentURL )
                                    const DRM_WCHAR *f_pwchPlayReadySilentURL,
    __in                            const DRM_DWORD  f_cchPlayReadySilentURL,
    __in_ecount_nz_opt( f_cchPlayReadyNonSilentURL )
                                    const DRM_WCHAR *f_pwchPlayReadyNonSilentURL,
    __in                            const DRM_DWORD  f_cchPlayReadyNonSilentURL,
    __in_ecount_nz_opt( f_cchServiceID )
                                    const DRM_WCHAR *f_pwchServiceID,
    __in                            const DRM_DWORD  f_cchServiceID,
    __in                                  DRM_BOOL   f_fIncludeELS,
    __in_ecount_nz_opt( f_cchCustomAttributes )
                                    const DRM_WCHAR *f_pwchCustomAttributes,
    __in                            const DRM_DWORD  f_cchCustomAttributes,
    __inout_bcount_opt( *f_pcbPlayReadyObject )
                                          DRM_BYTE  *f_pbPlayReadyObject,
    __inout                               DRM_DWORD *f_pcbPlayReadyObject );

/*
** ---------------------------------------
** Secure Time API functions
** ---------------------------------------
*/
DRM_API DRM_RESULT DRM_CALL Drm_SecureTime_GenerateChallenge(
    __in                                        DRM_APP_CONTEXT             *f_poAppContext,
    __out                                       DRM_DWORD                   *f_pcbChallenge,
    __deref_out_bcount( *f_pcbChallenge )       DRM_BYTE                   **f_ppbChallenge );

DRM_API DRM_RESULT DRM_CALL Drm_SecureTime_ProcessResponse(
    __in                                        DRM_APP_CONTEXT             *f_poAppContext,
    __in                                        DRM_DWORD                    f_cbResponse,
    __in_bcount( f_cbResponse )           const DRM_BYTE                    *f_pbResponse );

DRM_API DRM_RESULT DRM_CALL Drm_SecureTime_GetValue(
    __in                                        DRM_APP_CONTEXT             *f_poAppContext,
    __out                                       DRMFILETIME                 *f_pftSystemTime,
    __out                                       DRM_SECURETIME_CLOCK_TYPE   *f_peClockType );

/*
** ---------------------------------------
** Store Management API functions
** ---------------------------------------
*/
DRM_API DRM_RESULT DRM_CALL Drm_StoreMgmt_CleanupStore(
    __in           DRM_APP_CONTEXT        *f_poAppContext,
    __in           DRM_DWORD               f_dwFlags,
    __in_opt const DRM_VOID               *f_pvCallerData,
    __in           DRM_DWORD               f_dwCallbackInterval,
    __in_opt       pfnStoreCleanupProgress f_pfnCallback );

DRM_API DRM_RESULT DRM_CALL Drm_StoreMgmt_DeleteLicenses(
    __in            DRM_APP_CONTEXT  *f_poAppContext,
    __in      const DRM_CONST_STRING *f_pdcstrKID,
    __in_opt  const DRM_CONST_STRING *f_pdcstrLID,
    __out_opt       DRM_DWORD        *f_pcLicDeleted );

DRM_API DRM_RESULT DRM_CALL Drm_StoreMgmt_DeleteInMemoryLicenses(
    __in            DRM_APP_CONTEXT  *f_poAppContext,
    __in      const DRM_ID           *f_poBatchID );

DRM_API DRM_RESULT DRM_CALL Drm_ProcessCommand(
    __in                                       DRM_APP_CONTEXT *f_poAppContext,
    __in                                       DRM_DWORD        f_dwOperationCode,
    __in                                       DRM_DWORD        f_dwRequestArgument1,
    __in                                       DRM_DWORD        f_dwRequestArgument2,
    __in                                       DRM_DWORD        f_dwRequestArgument3,
    __in                                       DRM_DWORD        f_dwRequestArgument4,
    __in_bcount( f_dwRequestDataLength ) const DRM_BYTE        *f_pbRequestData,
    __in                                       DRM_DWORD        f_dwRequestDataLength,
    __out_opt                                  DRM_DWORD       *f_pdwResponseResult1,
    __out_opt                                  DRM_DWORD       *f_pdwResponseResult2,
    __out_opt                                  DRM_DWORD       *f_pdwResponseResult3,
    __out_opt                                  DRM_DWORD       *f_pdwResponseResult4 );

DRM_API DRM_RESULT DRM_CALL Drm_ProcessRequest(
    __in                                     DRM_APP_CONTEXT *f_poAppContext,
    __in                                     DRM_DWORD        f_dwOperationCode,
    __in                                     DRM_DWORD        f_dwRequestArgument1,
    __in                                     DRM_DWORD        f_dwRequestArgument2,
    __in                                     DRM_DWORD        f_dwRequestArgument3,
    __in                                     DRM_DWORD        f_dwRequestArgument4,
    __out                                    DRM_DWORD       *f_pdwResponseResult1,
    __out                                    DRM_DWORD       *f_pdwResponseResult2,
    __out                                    DRM_DWORD       *f_pdwResponseResult3,
    __out                                    DRM_DWORD       *f_pdwResponseResult4,
    __out_bcount( *f_pdwResponseDataLength ) DRM_BYTE        *f_pbResponseData,
    __out                                    DRM_DWORD       *f_pdwResponseDataLength );

/*
** ---------------------------------------
** Envelope API functions
** ---------------------------------------
*/

DRM_API DRM_RESULT DRM_CALL Drm_Envelope_Open(
    __in           DRM_APP_CONTEXT            *f_poAppContext,
    __in_opt       DRM_VOID                   *f_pOEMContext,
    __in_z   const DRM_WCHAR                  *f_pwszFilename,
    __out          DRM_ENVELOPED_FILE_CONTEXT *f_pEnvFile );

DRM_API DRM_RESULT DRM_CALL Drm_Envelope_DuplicateFileContext(
    __in_opt       DRM_VOID                   *f_pOEMContext,
    __in_z   const DRM_WCHAR                  *f_pwszFilename,
    __in     const DRM_ENVELOPED_FILE_CONTEXT *f_pSourceHandle,
    __out          DRM_ENVELOPED_FILE_CONTEXT *f_pNewHandle );

DRM_API DRM_RESULT DRM_CALL Drm_Envelope_InitializeRead(
    __in DRM_ENVELOPED_FILE_CONTEXT *f_pEnvFile,
    __in DRM_DECRYPT_CONTEXT        *f_pDecrypt );

DRM_API DRM_RESULT DRM_CALL Drm_Envelope_Close(
    __in DRM_ENVELOPED_FILE_CONTEXT *f_pEnvFile );

DRM_API DRM_RESULT DRM_CALL Drm_Envelope_GetSize(
    __in  DRM_ENVELOPED_FILE_CONTEXT *f_pEnvFile,
    __out DRM_DWORD                  *f_pcbFileSize );

DRM_API DRM_RESULT DRM_CALL Drm_Envelope_Read(
    __in                                              DRM_ENVELOPED_FILE_CONTEXT *f_pEnvFile,
    __out_ecount_part( f_cbToRead, *f_pcbBytesRead )  DRM_BYTE                   *f_pbBuffer,
    __in                                              DRM_DWORD                   f_cbToRead,
    __out                                             DRM_DWORD                  *f_pcbBytesRead );

DRM_API DRM_RESULT DRM_CALL Drm_Envelope_WritePlayReadyObject(
    __in         DRM_APP_CONTEXT            *f_poAppContext,
    __inout_opt  DRM_VOID                   *f_pOEMContext,
    __in_z const DRM_WCHAR                  *f_pwszFilename,
    __inout      DRM_ENVELOPED_FILE_CONTEXT *f_pEnvFile );

DRM_API DRM_RESULT DRM_CALL Drm_Envelope_Seek(
    __in  DRM_ENVELOPED_FILE_CONTEXT *f_pEnvFile,
    __in  DRM_LONG                    f_lDistanceToMove,
    __in  DRM_DWORD                   f_dwMoveMethod,
    __out DRM_DWORD                  *f_pdwNewFilePointer );

DRM_API DRM_RESULT DRM_CALL Drm_Envelope_GetOriginalFilename(
    __in                                        DRM_ENVELOPED_FILE_CONTEXT *f_pEnvFile,
    __out_ecount_opt( *f_pcchOriginalFilename ) DRM_WCHAR                  *f_pwszOriginalFilename,
    __inout                                     DRM_DWORD                  *f_pcchOriginalFilename );

/*
** ---------------------------------------
** Revocation API functions
** ---------------------------------------
*/
DRM_API DRM_RESULT DRM_CALL Drm_Revocation_SetBuffer(
    __inout                                 DRM_APP_CONTEXT *f_poAppContext,
    __in_bcount_opt( f_cbRevocationBuffer ) DRM_BYTE        *f_pbRevocationBuffer,
    __in                                    DRM_DWORD        f_cbRevocationBuffer );

DRM_API DRM_RESULT DRM_CALL Drm_Revocation_GetBuffer(
    __inout                                      DRM_APP_CONTEXT  *f_poAppContext,
    __deref_out_bcount( *f_pcbRevocationBuffer ) DRM_BYTE        **f_ppbRevocationBuffer,
    __out                                        DRM_DWORD        *f_pcbRevocationBuffer );

DRM_API DRM_RESULT DRM_CALL Drm_Revocation_StoreRevListArray(
    __inout                           DRM_APP_CONTEXT *f_poAppContext,
    __in                              DRM_DWORD        f_cRevocationLists,
    __in_ecount( f_cRevocationLists ) DRM_RVK_LIST    *f_pRevocationLists );

DRM_API DRM_RESULT DRM_CALL Drm_Revocation_GetList(
    __inout                          DRM_APP_CONTEXT          *f_poAppContext,
    __in                             DRM_REVOCATION_TYPE_ENUM  f_eRevType,
    __out_bcount_opt( *f_pcbBuffer ) DRM_BYTE                 *f_pbBuffer,
    __inout_opt                      DRM_DWORD                *f_pcbBuffer,
    __out_opt                        DRM_DWORD                *f_pdwVersion );

DRM_API DRM_RESULT DRM_CALL Drm_Revocation_StorePackage(
    __inout                           DRM_APP_CONTEXT *f_poAppContext,
    __in_ecount( f_cchPackage ) const DRM_CHAR        *f_pchPackage,
    __in                              DRM_DWORD        f_cchPackage );

DRM_API DRM_RESULT DRM_CALL Drm_Platform_Initialize( __inout_opt DRM_VOID *f_pvUserCtx );

DRM_API DRM_RESULT DRM_CALL Drm_Platform_Uninitialize( __inout_opt DRM_VOID *f_pvUserCtx );

/*
** ---------------------------------------
** Domain API functions
** ---------------------------------------
*/
DRM_API DRM_RESULT DRM_CALL Drm_JoinDomain_GenerateChallenge(
    __in                                       DRM_APP_CONTEXT *f_poAppContext,
    __in                                       DRM_DWORD        f_dwFlags,
    __in_opt                                   DRM_DOMAIN_ID   *f_poDomainID,
    __in_ecount_opt( f_cchFriendlyName ) const DRM_CHAR        *f_pchFriendlyName,
    __in                                       DRM_DWORD        f_cchFriendlyName,
    __in_ecount_opt( f_cchData )         const DRM_CHAR        *f_pchData,
    __in                                       DRM_DWORD        f_cchData,
    __out_bcount_opt( *f_pcbChallenge )        DRM_BYTE        *f_pbChallenge,
    __inout                                    DRM_DWORD       *f_pcbChallenge );

DRM_API DRM_RESULT DRM_CALL Drm_JoinDomain_ProcessResponse(
    __in                        DRM_APP_CONTEXT              *f_poAppContext,
    __in                        DRM_PROCESS_DJ_RESPONSE_FLAG  f_dwFlags,
    __in_bcount( f_cbResponse ) DRM_BYTE                     *f_pbResponse,
    __in                        DRM_DWORD                     f_cbResponse,
    __out                       DRM_RESULT                   *f_pResult,
    __out_opt                   DRM_DOMAIN_ID                *f_poDomainID );

DRM_API DRM_RESULT DRM_CALL Drm_LeaveDomain_GenerateChallenge(
    __in                                      DRM_APP_CONTEXT *f_poAppContext,
    __in                                      DRM_DWORD        f_dwFlags,
    __in_opt                                  DRM_DOMAIN_ID   *f_poDomainID,
    __in_ecount_opt( f_cchData )        const DRM_CHAR        *f_pchData,
    __in                                      DRM_DWORD        f_cchData,
    __out_bcount_opt( *f_pcbChallenge )       DRM_BYTE        *f_pbChallenge,
    __inout                                   DRM_DWORD       *f_pcbChallenge );

DRM_API DRM_RESULT DRM_CALL Drm_LeaveDomain_ProcessResponse(
    __in                        const DRM_APP_CONTEXT *f_poAppContext,
    __in_bcount( f_cbResponse )       DRM_BYTE        *f_pbResponse,
    __in                              DRM_DWORD        f_cbResponse,
    __out                             DRM_RESULT      *f_pResult );

DRM_API DRM_RESULT DRM_CALL Drm_DomainCert_Find(
    __in                                       DRM_APP_CONTEXT *f_poAppContext,
    __in                                 const DRM_DOMAIN_ID   *f_poDomainID,
    __out_bcount_opt( *f_pcbDomainCert )       DRM_BYTE        *f_pbDomainCert,
    __inout                                    DRM_DWORD       *f_pcbDomainCert );

DRM_API DRM_RESULT DRM_CALL Drm_DomainCert_InitEnum(
    __in  DRM_APP_CONTEXT              *f_poAppContext,
    __out DRM_DOMAIN_CERT_ENUM_CONTEXT *f_poDomainCertEnumContext );

DRM_API DRM_RESULT DRM_CALL Drm_DomainCert_EnumNext(
    __in  DRM_DOMAIN_CERT_ENUM_CONTEXT  *f_poDomainCertEnumContext,
    __out DRM_DWORD                     *f_pcchDomainCert,
    __out DRM_DOMAINCERT_INFO           *f_poDomainCertInfo );

/*
** ---------------------------------------
** Metering Certificate API Functions
** ---------------------------------------
*/
DRM_API DRM_RESULT DRM_CALL Drm_MeterCert_InitEnum(
    __in    DRM_APP_CONTEXT     *pAppContext,
    __out   DRM_METERCERT_ENUM  *pMeterCertEnumContext );

DRM_API DRM_RESULT DRM_CALL Drm_MeterCert_EnumNext(
    __in                                DRM_METERCERT_ENUM  *pMeterCertEnumContext,
    __inout                             DRM_MID             *pmid,
    __out_ecount_opt( *f_cchLAINFO )    DRM_WCHAR           *pwszLAINFO,
    __out_opt                           DRM_DWORD           *f_cchLAINFO,
    __out_bcount_opt( *f_pcbMeterCert ) DRM_BYTE            *f_pbMeterCert,
    __out_opt                           DRM_DWORD           *f_pcbMeterCert );

DRM_API DRM_RESULT DRM_CALL Drm_MeterCert_Update(
    __in                              DRM_APP_CONTEXT    *pAppContext,
    __in_bcount( cbMeterCert )  const DRM_BYTE           *pbMeterCert,
    __in                              DRM_DWORD           cbMeterCert,
    __out_opt                         DRM_MID            *pmid);

DRM_API DRM_RESULT DRM_CALL Drm_MeterCert_Invalidate(
    __in       DRM_APP_CONTEXT  *f_pcontextMGR,
    __in const DRM_MID          *f_pmid );

DRM_API DRM_RESULT DRM_CALL Drm_MeterCert_Delete(
    __in       DRM_APP_CONTEXT  *f_pcontextMGR,
    __in const DRM_MID          *f_pmid );

DRM_API DRM_RESULT DRM_CALL Drm_MeterCert_ProcessResponse(
    __in                              DRM_APP_CONTEXT *f_poAppContext,
    __in_bcount( f_cbResponse ) const DRM_BYTE        *f_pbResponse,
    __in                              DRM_DWORD        f_cbResponse,
    __out                             DRM_RESULT      *f_pResult );

DRM_API DRM_RESULT DRM_CALL Drm_MeterCert_GenerateChallenge(
    __in                                    DRM_APP_CONTEXT *f_poAppContext,
    __in                              const DRM_MID         *f_poMID,
    __in_ecount_opt( f_cchCustomData) const DRM_CHAR        *f_pchCustomData,
    __in_opt                                DRM_DWORD        f_cchCustomData,
    __out_bcount_opt( *f_pcbChallenge )     DRM_BYTE        *f_pbChallenge,
    __inout                                 DRM_DWORD       *f_pcbChallenge );

DRM_API DRM_RESULT DRM_CALL Drm_Metering_GenerateChallenge(
    __in                                DRM_APP_CONTEXT  *f_poAppContext,
    __in_bcount( f_cbMeterCert ) const  DRM_BYTE         *f_pbMeterCert,
    __in                                DRM_DWORD         f_cbMeterCert,
    __in_ecount_opt( f_cchCustomData )
                                 const  DRM_CHAR         *f_pchCustomData,
    __in_opt                            DRM_DWORD         f_cchCustomData,
    __out_ecount_opt( *f_pcchURL )      DRM_CHAR         *f_pchURL,
    __inout_opt                         DRM_DWORD        *f_pcchURL,
    __out_bcount_opt( *f_pcbChallenge ) DRM_BYTE         *f_pbChallenge,
    __inout                             DRM_DWORD        *f_pcbChallenge );

DRM_API DRM_RESULT DRM_CALL Drm_Metering_ProcessResponse(
    __in                              DRM_APP_CONTEXT  *f_poAppContext,
    __in_bcount( f_cbResponse ) const DRM_BYTE         *f_pbResponse,
    __in                              DRM_DWORD         f_cbResponse,
    __out                             DRM_DWORD        *f_pfFlags );

/*
** ---------------------------------------
** Local License Generation API functions
** ---------------------------------------
*/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_InitializePolicyDescriptor(
    __inout                             DRM_LOCAL_LICENSE_POLICY_DESCRIPTOR *f_poDescriptor ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_CreateLicense(
    __inout                               DRM_APP_CONTEXT                     *f_poAppContext,
    __in                            const DRM_LOCAL_LICENSE_POLICY_DESCRIPTOR *f_poDescriptor,
    __in                                  DRM_LOCAL_LICENSE_TYPE               f_eLicenseType,
    __in                                  DRM_DWORD                            f_dwEncryptionMode,
    __in                            const DRM_KID                             *f_pKeyId,
    __in_opt                        const DRM_LICENSE_HANDLE                   f_hRootLicense,
    __out                                 DRM_LICENSE_HANDLE                  *f_phLicense ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_CreatePlayReadyObject(
    __in_opt                                  DRM_DWORD                            f_cLicenses,
    __in_ecount_opt( f_cLicenses )      const DRM_LICENSE_HANDLE                  *f_pLicenses,
    __in_opt                            const DRM_DWORD                            f_cbRMHeader,
    __in_bcount_opt( f_cbRMHeader )     const DRM_BYTE                            *f_pbRMHeader,
    __out                                     DRM_DWORD                           *f_pcbPROBlob,
    __deref_out_bcount( *f_pcbPROBlob )       DRM_BYTE                           **f_ppbPROBlob ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_CreateRMHeader(
    __in                                 eDRM_HEADER_VERSION                  f_eHeaderVersion,
    __in                                 DRM_DWORD                            f_cKeyIds,
    __in_ecount_opt(f_cKeyIds)     const DRM_KID                             *f_pKeyIds,
    __out                                DRM_DWORD                           *f_pcbRMHeader,
    __deref_out_bcount( *f_pcbRMHeader ) DRM_BYTE                           **f_ppbRMHeader ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_StoreLicense(
    __in                          const DRM_LICENSE_HANDLE                   f_hLicense,
    __in                          const DRM_LOCAL_LICENSE_STORE              f_eLicenseStore ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_EncryptOpaque(
    __in                                          const DRM_LICENSE_HANDLE   f_hLicense,
    __in                                                DRM_DWORD            f_cEncryptedRegionMappings,
    __in_ecount( f_cEncryptedRegionMappings )     const DRM_DWORD           *f_pdwEncryptedRegionMappings,
    __in                                                DRM_DWORD            f_cbOpaqueClearContent,
    __in_bcount( f_cbOpaqueClearContent )         const DRM_BYTE            *f_pbOpaqueClearContent,
    __out                                               DRM_DWORD           *f_pcbEncryptedContent,
    __deref_out_bcount( *f_pcbEncryptedContent )        DRM_BYTE           **f_ppbEncryptedContent,
    __out                                               DRM_UINT64          *f_pqwIV ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_GetKID(
    __in                          const DRM_LICENSE_HANDLE                   f_hLicense,
    __out                               DRM_KID                             *f_pKeyID ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_GetXMRLicense(
    __in                           const DRM_LICENSE_HANDLE                   f_hLicense,
    __out                                DRM_DWORD                           *f_pcbXMRLicense,
    __deref_out_bcount(*f_pcbXMRLicense) DRM_BYTE                           **f_ppbXMRLicense ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_AddRef (
    __inout                             DRM_LICENSE_HANDLE                   f_hLicense ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_LocalLicense_Release(
    __inout                             DRM_LICENSE_HANDLE                  *f_phLicense ) DRM_NO_INLINE_ATTRIBUTE;

/*
** ---------------------------------------
** MPEG2 Transport Stream Encryptor API functions
** ---------------------------------------
*/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_M2ts_Initialize(
    __in                              const DRM_DWORD                        f_cbMaxHeapAllowed,
    __out                                   DRM_M2TS_ENCRYPTOR_HANDLE       *f_phEncryptor ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_M2ts_Uninitialize(
    __in                                    DRM_M2TS_ENCRYPTOR_HANDLE        f_hEncryptor ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_M2ts_SetEncryptionProperty(
        __in                                DRM_M2TS_ENCRYPTOR_HANDLE        f_hEncryptor,
        __in                          const DRM_M2TS_ENCRYPTION_PROPERTY    *f_poProperty ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_M2ts_SetKeyData(
    __in                                    DRM_M2TS_ENCRYPTOR_HANDLE        f_hEncryptor,
    __in                                    DRM_LICENSE_HANDLE               f_hLicense,
    __in                              const DRM_GUID                        *f_poKeyId,
    __in                              const DRM_M2TS_PRO_TYPE                f_ePROType,
    __in                              const DRM_DWORD                        f_dwDuration,
    __in                                    DRM_WORD                         f_wPID,
    __in                                    DRM_BOOL                         f_fAllowKeyRotation ) DRM_NO_INLINE_ATTRIBUTE;

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_M2ts_Encrypt(
    __in                                    DRM_M2TS_ENCRYPTOR_HANDLE        f_hEncryptor,
    __in_bcount_opt( f_cbDataIn )     const DRM_BYTE                        *f_pbDataIn,
    __in                              const DRM_DWORD                        f_cbDataIn,
    __out_bcount( *f_pcbDataOut )           DRM_BYTE                        *f_pbDataOut,
    __inout                                 DRM_DWORD                       *f_pcbDataOut,
    __in_opt                                Drm_M2ts_Encrypt_Callback        f_pfnEncryptCallback,
    __inout_ecount_opt( 1 )                 DRM_VOID                        *f_pvEncryptCallbackContext,
    __in                              const DRM_BOOL                         f_fLastTSPackets ) DRM_NO_INLINE_ATTRIBUTE;

/*
** ---------------------------------------
** Activation API functions
** ---------------------------------------
*/
DRM_API DRM_RESULT DRM_CALL Drm_Activation_ProcessResponseGenerateChallenge(
    __inout_bcount( ACTIVATION_CONTEXT_SIZE )  DRM_BYTE        *f_pbActivationCtx,
    __in_bcount_opt( f_cbResponse )            DRM_BYTE        *f_pbResponse,
    __in_opt                                   DRM_DWORD        f_cbResponse,
    __out_bcount_opt( *f_pcbChallenge )        DRM_BYTE        *f_pbChallenge,
    __inout                                    DRM_DWORD       *f_pcbChallenge,
    __out_ecount_opt( *f_pcchUrl )             DRM_CHAR        *f_pszUrl,
    __inout                                    DRM_DWORD       *f_pcchUrl,
    __out                                      DRM_RESULT      *f_pResult );

DRM_API DRM_RESULT DRM_CALL Drm_GetContextSizes(
    __out     DRM_DWORD *f_pdwAppContextSize,
    __out     DRM_DWORD *f_pdwOpaqueBufferSize,
    __out     DRM_DWORD *f_pdwDecryptContextSize );

DRM_API DRM_RESULT DRM_CALL Drm_Reader_CloneDecryptContext(
    __inout_ecount(1)  DRM_DECRYPT_CONTEXT *f_pDecryptContextIn,
    __out_ecount(1)    DRM_DECRYPT_CONTEXT *f_pDecryptContextOut );

/*
** ---------------------------------------
** Policy State API functions
** ---------------------------------------
*/
DRM_API DRM_RESULT DRM_CALL Drm_Policy_GetStateData(
    __in    DRM_APP_CONTEXT         *f_poAppContext,
    __in    const DRM_CONST_STRING  *f_pdstrKID,
    __in    const DRM_CONST_STRING  *f_pdstrStateName,
    __out_bcount_opt( *f_pcbStateData )
            DRM_BYTE                *f_pbStateData,
    __inout DRM_DWORD               *f_pcbStateData );

DRM_API DRM_RESULT DRM_CALL Drm_Policy_SetStateData(
    __in    DRM_APP_CONTEXT         *f_poAppContext,
    __in    const DRM_CONST_STRING  *f_pdstrKID,
    __in    const DRM_CONST_STRING  *f_pdstrStateName,
    __in_bcount( f_cbStateData )
            DRM_BYTE                *f_pbStateData,
    __inout DRM_DWORD                f_cbStateData );

/*
** ---------------------------------------
** Secure Stop API functions
** ---------------------------------------
*/

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_SecureStop_GenerateChallenge(
    __in                                        DRM_APP_CONTEXT            *f_pAppContext,
    __in_opt                              const DRM_ID                     *f_pidSession,
    __in                                  const DRM_DWORD                   f_cbPublisherCert,
    __in_bcount(f_cbPublisherCert)        const DRM_BYTE                   *f_pbPublisherCert,
    __in                                  const DRM_DWORD                   f_cchCustomData,
    __in_ecount_opt(f_cchCustomData)      const DRM_CHAR                   *f_pchCustomData,
    __out                                       DRM_DWORD                  *f_pcbChallenge,
    __deref_out_bcount(*f_pcbChallenge)         DRM_BYTE                  **f_ppbChallenge );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_SecureStop_ProcessResponse(
    __in                                        DRM_APP_CONTEXT            *f_pAppContext,
    __in_opt                              const DRM_ID                     *f_pidSession,
    __in                                  const DRM_DWORD                   f_cbPublisherCert,
    __in_bcount(f_cbPublisherCert)        const DRM_BYTE                   *f_pbPublisherCert,
    __in                                  const DRM_DWORD                   f_cbResponse,
    __in_bcount(f_cbResponse)             const DRM_BYTE                   *f_pbResponse,
    __out_opt                                   DRM_DWORD                  *f_pcchCustomData,
    __deref_out_ecount_opt(*f_pcchCustomData)   DRM_CHAR                  **f_ppchCustomData );

DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL Drm_SecureStop_EnumerateSessions(
    __in                                        DRM_APP_CONTEXT            *f_pAppContext,
    __in                                  const DRM_DWORD                   f_cbPublisherCert,
    __in_bcount(f_cbPublisherCert)        const DRM_BYTE                   *f_pbPublisherCert,
    __out                                       DRM_DWORD                  *f_pcidSessions,
    __deref_out_ecount(*f_pcidSessions)         DRM_ID                    **f_ppidSessions );

/*
** ---------------------------------------
** BRCM added APIs
** ---------------------------------------
*/

DRM_API DRM_RESULT DRM_CALL Drm_Reader_DecryptOpaque_Brcm(
    __in_ecount( 1 )                    DRM_DECRYPT_CONTEXT          *f_pDecryptContext,
    __inout                             DRM_AES_COUNTER_MODE_CONTEXT *f_pCtrContext,
    __inout                             DRM_VOID                     *f_pDmaBlocks,
    __in                                DRM_DWORD                     f_cDmaBlocks );

DRM_API DRM_RESULT DRM_CALL Drm_AntiRollBackClock_Init(
      __in          DRM_VOID                    *f_poAppContext,
      __in          const DRMSYSTEMTIME         *f_lpSystemTime );

DRM_API DRM_RESULT DRM_CALL Drm_Clock_GetSystemTime(
                DRM_APP_CONTEXT *f_poAppContext,
    __out       DRM_UINT64 *f_psystem );
PREFAST_POP /* __WARNING_NONCONST_PARAM_25004 */
PREFAST_POP /* __WARNING_NONCONST_BUFFER_PARAM_25033 */

EXIT_PK_NAMESPACE

#endif /* __DRMMANAGER_H__ */
