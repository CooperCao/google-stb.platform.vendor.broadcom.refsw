/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

/******************************************************************************
**
** This file declares the interface between the DRM blackbox and the secure crypto core.
** If the device supports a secure crypto core:
**      This file should define the interface between the unsecure core
**      (where the rest of DRM code runs) and the secure crypto core.
**      OEMs will need to create whatever OEM-hardware-specific layering is
**      required for this interface to become that boundary.
** If the device does not support a secure crypto core:
**      OEMs should not need to change this file in any way (except to get it to compile properly).
**
*******************************************************************************
*/

#ifndef __OEMHAL_H__
#define __OEMHAL_H__

#include <oemhalbasic.h>
#include <oemcommon.h>
#include <oemcryptoctx.h>
#include <oemkeyfileconsts.h>

#include "drmlicense.h"
ENTER_PK_NAMESPACE

typedef struct {
    DRM_WORD       wKeypairDecryptionMethod;
    DRM_WORD       wKeySize;
    DRM_BYTE       rgbInitializationVector[64];
DRM_OBFUS_FILL_BYTES(4)
/* This structure is passed to the TEE from the REE. When the REE is running on a 64 bit processor, this will cause trouble
because the size of the pointer will 8 bytes instead of 4. As a result, we wont be able to do a memcpy in the TEE and
the check to validate the buffer size received will fail. As a workaround, use an array of pre-allocated bytes instead
*/
/*DRM_OBFUS_PTR_TOP
    DRM_BYTE      *pbEncryptedKey;
DRM_OBFUS_PTR_BTM*/
    DRM_BYTE       encryptedKey[KF_MAX_ENCRYPTION_BUFFER];
    DRM_DWORD      cbEncryptedKey;
    DRM_GUID       guidKeypairDecryptionKeyID;
    DRM_WORD       wReserved;
} DRM_ENCRYPTED_KEY;

typedef DRM_OBFUS_FIXED_ALIGN struct __tagDRM_BB_CHECKSUM_CONTEXT
{
    DRM_BOOL   m_fIgnoreChecksum;
    DRM_BYTE   m_rgbChecksumData[DRM_MAX_CHECKSUM];
    DRM_DWORD  m_cbChecksum;  /* Should be 20 bytes for normal checksum, 24 bytes for V1 header checksum */
    DRM_KID    m_KID;
DRM_OBFUS_PTR_TOP
    DRM_BYTE  *m_pbV1KID;     /* If not NULL then use the old V1 checksum algorithm */
DRM_OBFUS_PTR_BTM
} DRM_BB_CHECKSUM_CONTEXT;

typedef DRM_OBFUS_FIXED_ALIGN struct __tagDRM_XMR_UNWRAP_INFO
{
DRM_OBFUS_PTR_TOP
    DRM_BYTE                *pbXMRLic;
DRM_OBFUS_PTR_BTM
    DRM_DWORD                cbXMRLic;
/* This structure is passed to the TEE from the REE. When the REE is running on a 64 bit processor, this will cause trouble
because the size of the pointer will 8 bytes instead of 4. As a result, we wont be able to do a memcpy in the TEE and
the check to validate the buffer size received will fail. As a workaround, use a copy of the DRM_BB_CHECKSUM_CONTEXT context
instead of a reference.
*/
    DRM_BB_CHECKSUM_CONTEXT  checksum;
} DRM_XMR_UNWRAP_INFO;

DRM_API DRM_NO_INLINE DRM_BOOL DRM_CALL OEM_HAL_IsHalDevCertValidationSupported(DRM_VOID) DRM_NO_INLINE_ATTRIBUTE;
DRM_API DRM_NO_INLINE DRM_BOOL DRM_CALL OEM_HAL_IsHalDevCertValidationUnsupported(DRM_VOID) DRM_NO_INLINE_ATTRIBUTE;

DRM_API DRM_RESULT DRM_CALL Oem_Hal_AllocateRegister(
    __in              Oem_HalHandle               pHalHandle,
    __in              OEM_HAL_KEY_TYPE            f_eKeyType,
    __out_ecount( 1 ) OEM_HAL_KEY_REGISTER_INDEX *f_pKeyRegisterIndex );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_FreeRegister(
    __in Oem_HalHandle              pHalHandle,
    __in OEM_HAL_KEY_TYPE           f_eKeyType,
    __in OEM_HAL_KEY_REGISTER_INDEX f_keyRegisterIndex );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_RegisterCount(
    __in Oem_HalHandle         pHalHandle,
    __in OEM_HAL_KEY_TYPE      f_eKeyType,
    __out_ecount(1) DRM_DWORD *f_pdwTotalRegisters,
    __out_ecount(1) DRM_DWORD *f_pdwAllocatedRegisters );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_GetPreloadedIndex(
    __in                     Oem_HalHandle               pHalHandle,
    __in                           OEM_HAL_KEY_TYPE            f_eKeyType,
    __in_bcount( f_cbKeyId ) const DRM_BYTE                   *f_pbKeyId,
    __in                     DRM_DWORD                   f_cbKeyId,
    __out_ecount( 1 )        OEM_HAL_KEY_REGISTER_INDEX *f_pIndexKey );



DRM_API Oem_HalHandle DRM_CALL Oem_Hal_Initialize(DRM_VOID *pOemContext);


DRM_API DRM_RESULT DRM_CALL Oem_Hal_Deinitialize(
    __in              Oem_HalHandle               pHalHandle
    );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_VerifyMessageSignature(
    __in                               Oem_HalHandle              pHalHandle,
    __in_bcount( f_cbMessage )   const DRM_BYTE                  *f_pbMessage,
    __in                               DRM_DWORD                  f_cbMessage,
    __in_bcount( f_cbSignature ) const DRM_BYTE                  *f_pbSignature,
    __in                               DRM_DWORD                  f_cbSignature,
    __in                               OEM_HAL_HASH_TYPE          f_eHashType,
    __in                               OEM_HAL_SIGNATURE_SCHEME   f_eSignatureScheme,
    __in                               OEM_HAL_KEY_REGISTER_INDEX f_indexIntegrityKey );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_SignWithPrivateKey(
    __in                                Oem_HalHandle              pHalHandle,
    __inout_ecount_opt( 1 )             DRM_CRYPTO_CONTEXT        *f_pCryptoContext,
    __in_bcount( f_cbMessage )    const DRM_BYTE                  *f_pbMessage,
    __in                                DRM_DWORD                  f_cbMessage,
    __out_bcount_opt( *f_pcbSignature ) DRM_BYTE                  *f_pbSignature,
    __inout                             DRM_DWORD                 *f_pcbSignature,
    __in                                OEM_HAL_HASH_TYPE          f_eHashType,
    __in                                OEM_HAL_SIGNATURE_SCHEME   f_eSignatureScheme,
    __in                                OEM_HAL_KEY_TYPE           f_eKeyType,
    __in                                OEM_HAL_KEY_REGISTER_INDEX f_indexIntegrityKey );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_VerifyOMAC1Signature(
    __in                               Oem_HalHandle               pHalHandle,
    __in_bcount( f_cbData )      const DRM_BYTE                   *f_pbData,
    __in                               DRM_DWORD                   f_cbData,
    __in_bcount( f_cbSignature ) const DRM_BYTE                   *f_pbSignature,
    __in                               DRM_DWORD                   f_cbSignature,
    __in                               OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                               OEM_HAL_KEY_REGISTER_INDEX  f_indexKey );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_CreateOMAC1Signature(
    __in                                 Oem_HalHandle               pHalHandle,
    __in_bcount( f_cbData )        const DRM_BYTE                   *f_pbData,
    __in                                 DRM_DWORD                   f_cbData,
    __out_bcount_opt( *f_pcbSignature )  DRM_BYTE                   *f_pbSignature,
    __inout                              DRM_DWORD                  *f_pcbSignature,
    __in                                 OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                                 OEM_HAL_KEY_REGISTER_INDEX  f_indexKey );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_CreateOMAC1SignatureForGlobalStorePassword(
    __in_opt                             DRM_VOID                   *f_pOEMContext,
    __in                                 Oem_HalHandle               pHalHandle,
    __out_bcount_opt( *f_pcbSignature )  DRM_BYTE                   *f_pbSignature,
    __inout                              DRM_DWORD                  *f_pcbSignature,
    __in                                 OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                                 OEM_HAL_KEY_REGISTER_INDEX  f_indexKey );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_EncryptDataBlock(
    __in                                 Oem_HalHandle               pHalHandle,
    __in_bcount( f_cbData )             const DRM_BYTE              *f_pbBlock,
    __in                                 DRM_DWORD                   f_cbBlock,
    __out_bcount_opt( *f_pcbEncryptedBlock )  DRM_BYTE              *f_pbEncryptedBlock,
    __inout                              DRM_DWORD                  *f_pcbEncryptedBlock,
    __in                                 OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                                 OEM_HAL_KEY_REGISTER_INDEX  f_indexMessageEncryptKey );

DRM_API DRM_BOOL DRM_CALL Oem_Hal_IsDevelopmentPlatform(void);

DRM_API DRM_RESULT DRM_CALL Oem_Hal_UnwrapKey(
    __in                                    Oem_HalHandle               pHalHandle,
    __inout_ecount( 1 )                     DRM_CRYPTO_CONTEXT         *f_pCryptoContext,
    __in                                    OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                                    OEM_HAL_KEY_REGISTER_INDEX  f_indexKey,
    __in                                    OEM_HAL_KEY_TYPE            f_eWrappingKeyType,
    __in                                    OEM_HAL_KEY_REGISTER_INDEX  f_indexWrappingKey,
    __in_bcount( f_cbWrappedKeyData ) const DRM_BYTE                   *f_pbWrappedKeyData,
    __in                                    DRM_DWORD                   f_cbWrappedKeyData,
    __in_bcount( f_cbParameterData )  const DRM_BYTE                   *f_pbParameterData,
    __in                                    DRM_DWORD                   f_cbParameterData );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_UnwrapDrmEncryptedKey(
    __in                                    Oem_HalHandle               halHandle,
    __inout_ecount( 1 )                     DRM_CRYPTO_CONTEXT         *f_pCryptoContext,
    __in                                    OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                                    OEM_HAL_KEY_REGISTER_INDEX  f_indexKey,
    __in                                    OEM_HAL_KEY_TYPE            f_eWrappingKeyType,
    __in                                    OEM_HAL_KEY_REGISTER_INDEX  f_indexWrappingKey,
    __in                              const DRM_ENCRYPTED_KEY          *f_pbDrmEncryptedKey,
    __in_bcount( f_cbParameterData )  const DRM_BYTE                   *f_pbParameterData,
    __in                                    DRM_DWORD                   f_cbParameterData );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_UnwrapDrmXmrUnwrapLicense(
    __in                                    Oem_HalHandle               halHandle,
    __inout_ecount( 1 )                     DRM_CRYPTO_CONTEXT         *f_pCryptoContext,
    __in                                    OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                                    OEM_HAL_KEY_REGISTER_INDEX  f_indexKey,
    __in                                    OEM_HAL_KEY_TYPE            f_eWrappingKeyType,
    __in                                    OEM_HAL_KEY_REGISTER_INDEX  f_indexWrappingKey,
    __in                              const DRM_XMR_UNWRAP_INFO        *f_pbDrmXrmUnwrapData,
    __in_bcount( f_cbParameterData )  const DRM_BYTE                   *f_pbParameterData,
    __in                                    DRM_DWORD                   f_cbParameterData );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_WrapKey(
    __in                                 Oem_HalHandle               pHalHandle,
    __inout_ecount( 1 )                  DRM_CRYPTO_CONTEXT         *f_pCryptoContext,
    __in                                 OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                                 OEM_HAL_KEY_REGISTER_INDEX  f_indexKey,
    __in                                 OEM_HAL_KEY_TYPE            f_eWrappingKeyType,
    __in                                 OEM_HAL_KEY_REGISTER_INDEX  f_indexWrappingKey,
    __out_bcount_opt( *f_pcbWrappedKey ) DRM_BYTE                   *f_pbWrappedKey,
    __inout_ecount(1)                    DRM_DWORD                  *f_pcbWrappedKey );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_WrapDrmEncryptedKey(
    __in                                 Oem_HalHandle               pHalHandle,
    __inout_ecount( 1 )                  DRM_CRYPTO_CONTEXT         *f_pCryptoContext,
    __in                                 OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                                 OEM_HAL_KEY_REGISTER_INDEX  f_indexKey,
    __in                                 OEM_HAL_KEY_TYPE            f_eWrappingKeyType,
    __in                                 OEM_HAL_KEY_REGISTER_INDEX  f_indexWrappingKey,
    __out                                DRM_ENCRYPTED_KEY          *f_pbWrappedKey);


#if(PLAYREADY_SAGE_IMPL==1)
DRM_API DRM_RESULT DRM_CALL Oem_Hal_LoadIv(
    __in                       Oem_HalHandle               pHalHandle,
    __in                       OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                       OEM_HAL_KEY_REGISTER_INDEX  f_indexContentKey,
    __in                       DRM_UINT64 *                f_qwInitializationVector,
    __in                       DRM_UINT64 *                f_qwSampleByteOffset,
    __in                       DRM_BYTE                    *uc_btp_packet,
    __in                       DRM_SIZE_T                   f_btp_packet_size
);

DRM_API DRM_RESULT DRM_CALL Oem_Hal_LoadIv_ToEncrypt(
    __in                       Oem_HalHandle               pHalHandle,
    __in                       OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                       OEM_HAL_KEY_REGISTER_INDEX  f_indexContentKey,
    __in                       DRM_UINT64 *                f_qwInitializationVector,
    __in                       DRM_UINT64 *                f_qwSampleByteOffset,
    __in                       DRM_BYTE                    *uc_btp_packet,
    __in                       DRM_SIZE_T                   f_btp_packet_size
);
#endif

#ifdef PLAYREADY_HOST_IMPL
DRM_API DRM_RESULT DRM_CALL Oem_Hal_Load_ExternalIv_Process(
    __in                       Oem_HalHandle                    halHandle,
    __in                       Oem_AesHwHandle                  aesHwHandle,
    __in                       OEM_HAL_KEY_TYPE                 f_eKeyType,
    __in                       OEM_HAL_KEY_REGISTER_INDEX       f_indexContentKey,
    __in                       DRM_UINT64 *                     f_qwInitializationVector,
    __in                       DRM_UINT64 *                     f_qwSampleByteOffset,
    __in                       DRM_SIZE_T                       f_btp_packet_size,
    __in                       const NEXUS_DmaJobBlockSettings  *pBlks,
    __in                       uint32_t                         nDmaBlocks);
#endif

DRM_API DRM_RESULT DRM_CALL Oem_Hal_GenerateKey(
    __in                Oem_HalHandle              pHalHandle,
    __inout_ecount_opt( 1 ) DRM_CRYPTO_CONTEXT        *f_pCryptoContext,
    __in                OEM_HAL_KEY_TYPE           f_eKeyType,
    __in                OEM_HAL_KEY_REGISTER_INDEX f_indexKey,
    __in                DRM_DWORD                  f_dwRequestedSecurityLevel );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_DecryptContent(
    __in                       Oem_HalHandle               pHalHandle,
    __in                       Oem_AesHwHandle             hAesHw,
    __inout_bcount( f_cbData ) DRM_BYTE                   *f_pbData,
    __in                       DRM_DWORD                   f_cbData,
    __in                       OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                       OEM_HAL_KEY_REGISTER_INDEX  f_indexContentKey,
    __in                       DRM_UINT64                  f_qwInitializationVector,
    __in                       DRM_UINT64                  f_qwSampleByteOffset );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_DecryptContentOpaque(
    __in                       Oem_HalHandle               pHalHandle,
    __in                       Oem_AesHwHandle             hAesHw,
    __in                       OEM_OPAQUE_BUFFER_HANDLE    f_hInBuf,
    __in                       OEM_OPAQUE_BUFFER_HANDLE    f_hOutBuf,
    __in                       DRM_DWORD                   f_cbData,
    __in                       OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                       OEM_HAL_KEY_REGISTER_INDEX  f_indexContentKey,
    __in                       DRM_UINT64                  f_qwInitializationVector,
    __in                       DRM_UINT64                  f_qwSampleByteOffset );

#if(PLAYREADY_SAGE_IMPL==1)
DRM_API DRM_RESULT DRM_CALL Oem_Hal_CreateSampleEncryptionContext(
    __in            Oem_HalHandle                   pHalHandle,
    __in            OEM_HAL_KEY_TYPE                f_eKeyType,
    __in            OEM_HAL_KEY_REGISTER_INDEX      f_indexKey,
    __in            OEM_HAL_SAMPLE_ENCRYPTION_MODE  f_eSampleEncryptionMode,
    __inout_ecount(1) OEM_ENCRYPTION_HANDLE         *f_phEncryptionContext);
#else
DRM_API DRM_RESULT DRM_CALL Oem_Hal_CreateSampleEncryptionContext(
    __in            Oem_HalHandle                   pHalHandle,
    __in            OEM_HAL_KEY_TYPE                f_eKeyType,
    __in            OEM_HAL_KEY_REGISTER_INDEX      f_indexKey,
    __in            OEM_HAL_SAMPLE_ENCRYPTION_MODE  f_eSampleEncryptionMode,
    __out_ecount(1) OEM_ENCRYPTION_HANDLE          *f_phEncryptionContext );
#endif

DRM_API DRM_RESULT DRM_CALL Oem_Hal_EncryptSampleData(
    __in                           Oem_HalHandle             pHalHandle,
    __in                           OEM_ENCRYPTION_HANDLE     f_hEncryptionContext,
    __out_ecount_opt(1)            DRM_UINT64               *f_pqwInitializationVector,
    __out_ecount_opt(1)            OEM_HAL_SAMPLE_METADATA  *f_pMetadata,
    __in_opt                       OEM_OPAQUE_BUFFER_HANDLE  f_hInBuf,
    __in_opt                       OEM_OPAQUE_BUFFER_HANDLE  f_hOutBuf,
    __in                           DRM_DWORD                 f_cbData,
    __inout_bcount_opt( f_cbData ) DRM_BYTE                 *f_pbData );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_QuerySampleMetadata(
    __in            Oem_HalHandle            pHalHandle,
    __in            OEM_ENCRYPTION_HANDLE    f_hEncryptionContext,
    __out_ecount(1) OEM_HAL_SAMPLE_METADATA *f_pMetadata );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_DestroySampleEncryptionContext(
    __in Oem_HalHandle         pHalHandle,
    __in OEM_ENCRYPTION_HANDLE f_hEncryptionContext );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_LoadPlayReadyRevocationInfo(
    __in                                    Oem_HalHandle               pHalHandle,
    __inout_ecount( 1 )                     DRM_CRYPTO_CONTEXT         *f_pCryptoContext,
    __in_bcount( f_cbRevocationInfo ) const DRM_BYTE                   *f_pbRevocationInfo,
    __in                                    DRM_DWORD                   f_cbRevocationInfo,
    __in                                    OEM_HAL_KEY_REGISTER_INDEX  f_indexSigningKey );

DRM_API DRM_RESULT DRM_CALL Oem_Hal_LoadPlayReadyCrl(
    __in                         Oem_HalHandle               pHalHandle,
    __inout_ecount( 1 )          DRM_CRYPTO_CONTEXT         *f_pCryptoContext,
    __in_bcount( f_cbCRL ) const DRM_BYTE                   *f_pbCRL,
    __in                         DRM_DWORD                   f_cbCRL,
    __in                         OEM_HAL_KEY_REGISTER_INDEX  f_indexSigningKey );


DRM_API DRM_RESULT DRM_CALL Oem_Hal_Aes_CbcOperation(
    __in                                      Oem_HalHandle               pHostHalHandle,
    __in                                      OEM_AES_OPERATION           op,
    __in_bcount( f_cbData )                   DRM_BYTE                   *f_pbData,
    __in                                      DRM_DWORD                   f_cbData,
    __in_bcount( f_cbData )             const DRM_BYTE                   *f_pbIv,
    __in                                      DRM_DWORD                   f_cbIv,
    __in                                      OEM_HAL_KEY_TYPE            f_eKeyType,
    __in                                      OEM_HAL_KEY_REGISTER_INDEX  f_indexKey );

DRM_API DRM_RESULT Oem_Hal_ConfigHwRegister(
    __in              Oem_HalHandle              pHalHandle,
    __in              OEM_HAL_KEY_TYPE           f_eKeyType,
    __in              OEM_HAL_KEY_REGISTER_INDEX indexKeyRegister,
    __in              uint32_t                   contextHandle,
    __in              DRM_WORD                   f_wUncompressedDigitalVideoPlayOpl );

DRM_API DRM_RESULT Oem_Hal_InvalidateHwRegister(
    __in              Oem_HalHandle              pHalHandle,
    __in              OEM_HAL_KEY_TYPE           f_eKeyType,
    __in              OEM_HAL_KEY_REGISTER_INDEX indexKeyRegister);

DRM_API DRM_RESULT Oem_Hal_ProtectContentKey(
    __in              Oem_HalHandle              pHalHandle,
    __in              DRM_BOOL                   inProtectContentKey);

DRM_API DRM_BOOL Oem_Hal_IsContentKeyProtected(
    __in              Oem_HalHandle              halHandle);

#ifdef PLAYREADY_HOST_IMPL
DRM_API DRM_RESULT Oem_Hal_Time_SetTime(
    __in              Oem_HalHandle              pHalHandle,
    __in              DRM_DWORD                  systemTime);

DRM_API DRM_RESULT Oem_Hal_Time_GetTime(
    __in              Oem_HalHandle              pHalHandle,
    __out             DRM_DWORD                 *f_systemTime);
#endif

EXIT_PK_NAMESPACE

#endif /* __OEMHAL_H__ */
