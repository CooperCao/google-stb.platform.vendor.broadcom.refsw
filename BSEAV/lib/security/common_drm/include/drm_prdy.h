/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
#ifndef DRM_PRDY_H__
#define DRM_PRDY_H__

#include "drm_prdy_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
** Function: DRM_Prdy_GetDrmAppContext
**
** Description:
**   Retrieve the DRM_APP_CONTEXT
**
** Parameters:
**  handle - DRM_Prdy_Handle_t
**
** Return:
**  On success - a valid pointer of DRM_APP_CONTEXT
**
******************************************************************************/
void* DRM_Prdy_GetDrmAppContext(DRM_Prdy_Handle_t handle);

/******************************************************************************
** Function: DRM_Prdy_GetDefaultParamSettings
**
** Description:
**   Retrieve the default settings
**
** Parameters:
** pPrdyParamSettings - pointer to settings structure
**
** Return:
**   void.
**
******************************************************************************/
void DRM_Prdy_GetDefaultParamSettings(
		DRM_Prdy_Init_t *pPrdyParamSettings);

/***********************************************************************************
 * Function: DRM_Prdy_Initialize()
 *
 * Description:
 * The DRM_Prdy_Initialize() creates and initializes the DRM_APP_CONTEXT. It also
 * initialzes OEM_Settings and OEM8_Context
 *
 * Maps to:
 * call Drm_Platform_Initialize()
 * call Drm_Initialize()
 * call Drm_Revocation_SetBuffer()
 *
 * Parameters:
 *  N/A
 *
 * Return:
 *  On Success:   A valid DRM_Prdy_Handle_t
 *
 ***********************************************************************************/
DRM_Prdy_Handle_t  DRM_Prdy_Initialize(DRM_Prdy_Init_t * pInitSetting);

/***********************************************************************************
 * Function: DRM_Prdy_Initialize_Ex()
 *
 * Description:
 * The DRM_Prdy_Initialize_Ex() creates and initializes the DRM_APP_CONTEXT.
 * It also initialzes OEM_Settings and OEM_Context.
 * Same behavior with DRM_Prdy_Initialize except additional argument for containing error code.
 *
 * Maps to:
 * call Drm_Platform_Initialize()
 * call Drm_Initialize()
 * call Drm_Revocation_SetBuffer()
 *
 * Parameters:
 *  N/A
 *
 * Return:
 *  On Success:   A valid DRM_Prdy_Handle_t
 *  On Failure:   Null handle with corresponding error code.
 *
 ***********************************************************************************/
DRM_Prdy_Handle_t  DRM_Prdy_Initialize_Ex(DRM_Prdy_Init_t * pInitSetting, DRM_Prdy_Error_e * Drm_Prdy_Error);



/***********************************************************************************
 * Function: DRM_Prdy_Uninitialize()
 *
 * Description:
 * The DRM_Prdy_Uninitialize() releases all the resouces allocated.
 *
 * Maps to:
 * call Drm_Platform_Uninitialize()
 * call Drm_UnInitialize()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Uninitialize( DRM_Prdy_Handle_t  pPrdyContext );

/***********************************************************************************
 * Function: DRM_Prdy_Reinitialize()
 *
 * Description:
 * The DRM_Prdy_Reinitialize() cleans up the header information stored in the DRM context.
 *
 * Maps to:
 * call Drm_Rennitialize()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reinitialize( DRM_Prdy_Handle_t  pPrdyContext );

/***********************************************************************************
 * Function: DRM_Prdy_Get_Buffer_Size()
 *
 * Description:
 * The DRM_Prdy_Get_Buffer_Size() will calculate the proper buffer size for the type
 * of operation requested (operations such as content property, license challenge,
 * metering challenge, or client info, etc.).
 * Application will allocate memory for the buffer based on the size returned by
 * this function for the operation to be performed. In this case, applicaton will be
 * responsible to free the memory after using it.
 *
 * Maps to:
 *  - Drm_LicenseAcq_GenerateChallenge() to get the sizes of the url and
 *    challenge OR
 *  - Drm_Device_GetProperty to get the size of the client info
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  chType:       [in] The type of buffer requesting for
    pData:        [in] Pointer to a buffer of the custom data that needs to be passed to the sdk function. can be NULL
    dataLen:      [in] The size of the custom data if any
 *  pData1_size:  [out] The size of the data1, see Note for details
 *  pData2_size:  [out] The size of the data2, see Note for details
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  Support only the following buffer types:
 *
 *   DRM_Prdy_getBuffer_content_property
 *    - pData1_size contains the required size for the content property
 *    - pData2_size is not used.
 *
 *   DRM_Prdy_getBuffer_licenseAcq_challenge,
 *    - pData1_size contains the required size for the URL string
 *    - pData2_size contains the required size for the Challenge Data
 *
 *   DRM_Prdy_getBuffer_client_info,
 *    - pData1_size contains the required size for the client info string
 *    - pData2_size is not used.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Get_Buffer_Size(
        DRM_Prdy_Handle_t            pPrdyContext,
        DRM_Prdy_GetBuffer_Type_e    chType,
        const uint8_t               *pData,
        uint32_t                     dataLen,
        uint32_t                    *pData1_size,  /* [out] */
        uint32_t                    *pData2_size); /* [out] */


/***********************************************************************************
 * Function: DRM_Prdy_Content_GetProperty()
 *
 * Description:
 * The DRM_Prdy_Content_GetProperty function retrieves property data from the DRM
 * header of the content file associated with a given application context.
 *
 * Maps to:
 *  Drm_Content_GetProperty()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  propertyType: [in] The property type requesting for
 *  pData:        [out] Pointer to a buffer that receives the property data.
 *  dataSize:     [in] The buffer size of the pData
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  Application must allocate memory for the property data buffer for this function
 *  to work. Application should call DRM_Prdy_Get_Buffer_Size with the buffer type
 *  set to "DRM_Prdy_getBuffer_content_property" to determine the required size of
 *  the buffer.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Content_GetProperty(
        DRM_Prdy_Handle_t                pPrdyContext,
        DRM_Prdy_ContentGetProperty_e    propertyType,
        uint8_t                         *pData,
        uint32_t                         dataSize);


/***********************************************************************************
 * Function: DRM_Prdy_GetAdditionalResponseData()
 *
 * Description:
 * The DRM_Prdy_GetAdditionalResponseData function retrieves additional data from a license
 * response property.
 *
 * Maps to:
 *  Drm_GetAdditionalResponseData()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  propertyType: [in] The property type requesting for
 *  f_pbResponse: [in] Pointer to the response to fetch additional data from.
 *  f_cbResponse: [in] Size of the response buffer in bytes.
 *  f_dwDataType: [in] Tag identifying the information to retrieve.
 *  f_pchDataString: [out] Information found
 *  f_cbchDataString: Size of the buffer used to store the information retrived in bytes.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  Application must allocate memory for the property data buffer for this function
 *  to work. Application should call DRM_Prdy_Get_Buffer_Size with the buffer type
 *  set to "DRM_Prdy_getBuffer_Additional_Response_ZZZZ" to determine the required
 *  size of the buffer.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetAdditionalResponseData(
        DRM_Prdy_Handle_t   pPrdyContext,
        uint8_t            *f_pbResponse,
        uint32_t            f_cbResponse,
        uint32_t            f_dwDataType,
        char               *f_pchDataString,
        uint32_t            f_cbchDataString);

/***********************************************************************************
 * Function: DRM_Prdy_Content_SetProperty()
 *
 * Description:
 * The DRM_Prdy_Content_SetProperty function sets content properties used for DRM
 * tasks.
 *
 * Please note that the format of the property data is property-type specific. This
 * function does not perform any checking or processing on the data, and will directly
 * call the SDK SetProperty function. Application should construct the type specific
 * raw bytes properly and pass it to this function. For example, for the type of
 * DRM_Prdy_contentSetProperty_eAutoDetectHeader, the data is usually extracted from
 * PSSH container.
 *
 * Maps to:
 *  Drm_Content_SetProperty()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  propertyType: [in] The property type requesting for
 *  pData:        [in] Pointer to a buffer that contains the property data.
 *  dataSize:     [in] The buffer size of the pData
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Content_SetProperty(
        DRM_Prdy_Handle_t                pPrdyContext,
        DRM_Prdy_ContentSetProperty_e    propertyType,
        const uint8_t                   *pData,
        uint32_t                         dataSize);


/***********************************************************************************
 * Function: DRM_Prdy_Get_Protection_Policy()
 *
 * Description:
 * The DRM_Prdy_Get_Protection_Policy function retrieves the protection information
 * for the content associated with a given application context. This function is
 * ususlly used after calling DRM_Prdy_Reader_Bind(), which binds a valid license to
 * a content.
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pPolicy:      [out] The protection informaiton about the content
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Get_Protection_Policy(
        DRM_Prdy_Handle_t      pPrdyContext,
        DRM_Prdy_policy_t     *pPolicy);


/***********************************************************************************
 * Function: DRM_Prdy_GetDefaultDecryptSettings()
 *
 * Description:
 * The DRM_Prdy_GetDefaultDecryptSettings function retrieves the default settings for
 * for setting a Decrypt context.
 *
 * Parameters:
 *  pSettings : [out] A pointer to a DRM_Prdy_DecryptSettings_t.
 *
 * Returns:
 *  n/a
 *
 ***********************************************************************************/
void DRM_Prdy_GetDefaultDecryptSettings(
        DRM_Prdy_DecryptSettings_t  *pSettings
        );


/***********************************************************************************
 * Function: DRM_Prdy_SetDecryptContext()
 *
 * Description:
 * The DRM_Prdy_AllocateDecryptContext function allocates a decryption context
 * and sets the parameters of a Playready decrypt context.
 *
 * Parameters:
 *  pSettings       : [in] A pointer to a DRM_Prdy_DecryptSettings_t.
 *
 * Returns:
 *   On Success:   A pointer to a DRM_Prdy_DecryptContext_t context
 *   On Failure:   NULL
 *
 ***********************************************************************************/
DRM_Prdy_DecryptContext_t * DRM_Prdy_AllocateDecryptContext (
        const DRM_Prdy_DecryptSettings_t  *pSettings
        );



/***********************************************************************************
 * Function: DRM_Prdy_SetDecryptContext()
 *
 * Description:
 * The DRM_Prdy_SetDecryptContext function sets the parameters of a Playready
 * decrypt context.
 *
 * Parameters:
 *  pSettings       : [in] A pointer to a DRM_Prdy_DecryptSettings_t.
 *  pDecryptContext : [out] A pointer to a DRM_Prdy_DecryptContext_t.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e  DRM_Prdy_SetDecryptContext (
        const DRM_Prdy_DecryptSettings_t  *pSettings,
        DRM_Prdy_DecryptContext_t         *pDecryptContext
        );


/***********************************************************************************
 * Function: DRM_Prdy_FreeDecryptContext()
 *
 * Description:
 * The DRM_Prdy_FreeDecryptContext function frees the parameters of a Playready
 * decrypt context.
 *
 * Parameters:
 *  pDecryptContext : [in] A pointer to a DRM_Prdy_DecryptContext_t.
 *
 * Returns:
 *   n/a
 *
 ***********************************************************************************/
void DRM_Prdy_FreeDecryptContext (
        DRM_Prdy_DecryptContext_t  *pDecryptContext
        );


/***********************************************************************************
 * Function: DRM_Prdy_Reader_Bind()
 *
 * Description:
 * The DRM_Prdy_Reader_Bind function searches the license store for a valid license
 * to bind to and render the content. The license store is searched until a valid
 * license is found (A valid license is one that matches the header that was set in
 * a call to DRM_Prdy_Content_SetProperty). The function stops searching if all
 * valid licenses have been enumerated and none is found to satisfy all license
 * requirements. This function will return a valid decryptor context upon a valid
 * license is found, which can be used to decrypt the license binded content.
 *
 * Note that this function will allocate memory sizeof(DRM_DECRYPT_CONTEXT) bytes for
 * pDecryptContext->pDecrypt internally.
 *
 * Maps to:
 *  Drm_Reader_Bind()
 *  Drm_ResizeOpaqueBuffer() if needed
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pDecryptContext: [out] A decryption context to be used to decrypt data.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  In order to keep this function simple and make it easy to use, it always use
 *  g_dstrWMDRM_RIGHT_PLAYBACK for the rights and no callback is supported. However,
 *  application can call DRM_Prdy_Get_Protection_Policy to retrieve the protection
 *  information for the content after this function has been successfully executed.
 *
 *  Also, for pDecryptContext, the application must call DRM_Prdy_Reader_Close on
 *  this context when finishes using it.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Bind(
        DRM_Prdy_Handle_t           pPrdyContext,
        DRM_Prdy_DecryptContext_t  *pDecryptContext);

/***********************************************************************************
 * Function: DRM_Prdy_Reader_Bind_Netflix()
 *
 * Description:
 * Performs the same operation as Drm_Reader_Bind followed by a bind
 * of the decryption context key index to the session identified by
 * the provided session ID.
 *
 * Note that this function will allocate memory sizeof(DRM_DECRYPT_CONTEXT) bytes for
 * pDecryptContext->pDecrypt internally.
 *
 * Maps to:
 *  Drm_Reader_Bind_Netflix()
 *  Drm_ResizeOpaqueBuffer() if needed
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pSessionID       [in] A 16-byte secure stop session ID received via a call to DRM_Prdy_LicenseAcq_ProcessResponseEx
 *  pDecryptContext: [out] A decryption context to be used to decrypt data.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  In order to keep this function simple and make it easy to use, it always use
 *  g_dstrWMDRM_RIGHT_PLAYBACK for the rights and no callback is supported. However,
 *  application can call DRM_Prdy_Get_Protection_Policy to retrieve the protection
 *  information for the content after this function has been successfully executed.
 *
 *  Also, for pDecryptContext, the application must call DRM_Prdy_Reader_Close on
 *  this context when finishes using it.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Bind_Netflix(
        DRM_Prdy_Handle_t           pPrdyContext,
        uint8_t					   *pSessionID,
        DRM_Prdy_DecryptContext_t  *pDecryptContext);


#ifndef CMD_DRM_PLAYREADY_SAGE_IMPL
/***********************************************************************************
 * Function: DRM_Prdy_Reader_Bind_Netflix_WCK()
 *
 * Description:
 * Performs the same operation as Drm_Reader_Bind_Netflix after
 * configuring PlayReady to protect the content key.  Applies primarily
 * to devices without a TEE since the content key is always protected on
 * TEE-based devices.
 *
 * Note that this function will allocate memory sizeof(DRM_DECRYPT_CONTEXT) bytes for
 * pDecryptContext->pDecrypt internally.
 *
 * Maps to:
 *  Drm_Reader_Bind_Netflix()
 *  Drm_ResizeOpaqueBuffer() if needed
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pSessionID       [in] A 16-byte secure stop session ID received via a call to DRM_Prdy_LicenseAcq_ProcessResponseEx
 *  pDecryptContext: [out] A decryption context to be used to decrypt data.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  In order to keep this function simple and make it easy to use, it always use
 *  g_dstrWMDRM_RIGHT_PLAYBACK for the rights and no callback is supported. However,
 *  application can call DRM_Prdy_Get_Protection_Policy to retrieve the protection
 *  information for the content after this function has been successfully executed.
 *
 *  Also, for pDecryptContext, the application must call DRM_Prdy_Reader_Close on
 *  this context when finishes using it.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Bind_Netflix_WCK(
        DRM_Prdy_Handle_t           pPrdyContext,
        uint8_t					   *pSessionID,
        DRM_Prdy_DecryptContext_t  *pDecryptContext);

#endif

/***********************************************************************************
 * Function: DRM_Prdy_Reader_Commit()
 *
 * Description:
 * The Drm_Reader_Commit function is called by the player to commit to disk all
 * state data (including metering and play counts) changes in the data store.
 *
 * Maps to:
 *  Drm_Reader_Commit()
 *
 * Parameters:
 *  pPrdyContext:  [in] A valid DRM_Prdy_Context
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  In order to keep this function simple and make it easy to use, no callback is
 *  supported but application can call DRM_Prdy_Get_Protection_Policy to retrieve the
 *  protection information for the content.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Commit(
        DRM_Prdy_Handle_t      pPrdyContext);


/***********************************************************************************
 * Function: DRM_Prdy_Reader_Decrypt()
 *
 * Description:
 * The Drm_Reader_Decrypt function decrypts protected content. The caller must provide
 * the destination data buffer where the encrypted data will be placed. This function
 * will return failure if destnation data buffer is missing.
 *
 * Maps to:
 *  Drm_Reader_DecryptOpaque()
 *
 * Parameters:
 *  pDecryptContext:     [in] A valid decryption context
 *  pAesCtrInfo:         [in] AES Counter settings
 *  pBuf                 [in|out] data buffer
 *  dataSize             [in] size of the encrypted data
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  The encrypted and decrypted data buffers must be allocated using Nexus memory
 *  allocation function, othewise the decryption will fail.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Decrypt(
        DRM_Prdy_DecryptContext_t        *pDecryptContext,
        DRM_Prdy_AES_CTR_Info_t          *pAesCtrInfo,
        uint8_t                          *pBuf,
        uint32_t                          dataSize);

/***********************************************************************************
 * Function: DRM_Prdy_Reader_DecryptOpaque()
 *
 * Description:
 * The Drm_Reader_DecryptOpaque function decrypts protected content. The caller must provide
 * Scatter/Gather list of buffer segments allocated from Nexus memory, otherwise the decryption will fail.
 *
 * Maps to:
 *  Drm_Reader_DecryptOpaque()
 *
 * Parameters:
 *  pDecryptContext:     [in] A valid decryption context
 *  pAesCtrInfo:         [in] AES Counter settings
 *  pScatterGatherList   [in|out] Source and destination segments as NEXUS_DmaJobBlockSettings
 *  nelem                [in] Number of elements in S/G list
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  The encrypted and decrypted data buffers must be allocated using Nexus memory
 *  allocation function, othewise the decryption will fail.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_DecryptOpaque(
        DRM_Prdy_DecryptContext_t        *pDecryptContext,
        DRM_Prdy_AES_CTR_Info_t          *pAesCtrInfo,
        void *                            pScatterGatherList,
        uint32_t                          nelem);

/***********************************************************************************
 * Function: DRM_Prdy_Reader_CloneDecryptContext()
 *
 * Description:
 * The DRM_Prdy_Reader_CloneDecryptContext function is used to create another instance
 * of a decryption context. After the functions call, both contexts will share the
 * same key slot.
 *
 * Maps to:
 *  Drm_Reader_CloneDecryptContext()
 *
 * Parameters:
 *  pDecryptContext:       [in] A valid decryption context
 *  pClonedDecryptContext: [in|out] A copy of the decryption context, The caller must allocate this structure by calling
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
  *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_CloneDecryptContext(
    DRM_Prdy_DecryptContext_t  *pDecryptContext,
    DRM_Prdy_DecryptContext_t  *pClonedDecryptContext
    );

/***********************************************************************************
 * Function: DRM_Prdy_Reader_Close()
 *
 * Description:
 * The DRM_Prdy_Reader_Close functions releases resources allocated for the content
 * decryption operations.
 *
 * Maps to:
 *  Drm_Reader_Close()
 *  Call OEM_MemFree() to release the memory of the context itself
 *
 * Parameters:
 *  pDecryptContext:     [in] A valid decryption context
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Close(
        DRM_Prdy_DecryptContext_t   *pDecryptContext);


/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_GenerateChallenge()
 *
 * Description:
 * The DRM_Prdy_LicenseAcq_GenerateChallenge function initiates license acquisition
 * by generating a license challenge.
 *
 * Maps to:
 *  Drm_LicenseAcq_GenerateChallenge()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pCustomData:  [in] Pointer to a buffer that contains custom data to be sent to
 *                     the server. It can be NULL
 *  customDataLen [in] Size fo the pCustomData
 *  pCh_url       [out] A pointer to a buffer for retrieving the URL string
 *  url_len       [in|out] size of the URL buffer
 *  pCh_data      [out] A pointer to a buffer for retrieving the challenge data
 *  pCh_len       [in|out] Size of the challenge data buffer
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  Application must allocate memory for the challenge data for this function to work.
 *  Application should call DRM_Prdy_Get_Buffer_Size to determine the proper buffer
 *  size of the challenge data.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GenerateChallenge(
        DRM_Prdy_Handle_t      pPrdyContext,
        const char            *pCustomData, /* [in] - can be NULL */
        uint32_t               customDataLen,
        char                  *pCh_url,     /* [out] */
        uint32_t              *pUrl_len,
        char                  *pCh_data,    /* [out] */
        uint32_t              *pCh_len);

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix()
 *
 * Description:
 * Same functionality as DRM_Prdy_LicenseAcq_GenerateChallenge with input LDL flag
 * added as part of Netflix's Limited Duration License feature.
 *
 * Maps to:
 *  Drm_LicenseAcq_GenerateChallenge_Netflix()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pCustomData:  [in] Pointer to a buffer that contains custom data to be sent to
 *                     the server. It can be NULL
 *  customDataLen [in] Size fo the pCustomData
 *  pCh_url       [out] A pointer to a buffer for retrieving the URL string
 *  url_len       [in|out] size of the URL buffer
 *  pCh_data      [out] A pointer to a buffer for retrieving the challenge data
 *  pCh_len       [in|out] Size of the challenge data buffer
 *  pNonce        [out] A 16-byte nonce
 *  isLDL   	  [in] Flag indicating if this is an LDL challenge
 *
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  Application must allocate memory for the challenge data for this function to work.
 *  Application should call DRM_Prdy_Get_Buffer_Size to determine the proper buffer
 *  size of the challenge data.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix(
        DRM_Prdy_Handle_t      pPrdyContext,
        const char            *pCustomData, /* [in] - can be NULL */
        uint32_t               customDataLen,
        char                  *pCh_url,     /* [out] */
        uint32_t              *pUrl_len,
        char                  *pCh_data,    /* [out] */
        uint32_t              *pCh_len,
        uint8_t               *pNonce,      /* [out] */
        bool                   isLDL);      /* [in] */


/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_CancelChallenge_Netflix()
 *
 * Description:
 * Cancels a license challenge generated by a previous call to
 * DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix.
 *
 * Maps to:
 *  Drm_LicenseAcq_CancelChallenge_Netflix()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pNonce        [in] A 16-byte nonce obtained during call to DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix.
 *
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_CancelChallenge_Netflix(
    DRM_Prdy_Handle_t   pPrdyContext,
    uint8_t            *pNonce);

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_GetLdlSessionsLimit_Netflix()
 *
 * Description:
 * Returns the maximum number of LDL sessions that are supported.
 *
 * Maps to:
 *  Drm_LicenseAcq_GetLdlSessionsLimit_Netflix()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pNonce        [out] The maximum number of supported LDL sessions.
 *
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GetLdlSessionsLimit_Netflix(
    DRM_Prdy_Handle_t   pPrdyContext,
    uint32_t           *pLdlSessionsLimit);

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_FlushLdlChallenges_Netflix()
 *
 * Description:
 * Clears all LDL sessions.
 *
 * Maps to:
 *  Drm_LicenseAcq_FlushLdlChallenges_Netflix()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_FlushLdlChallenges_Netflix(DRM_Prdy_Handle_t  pPrdyContext);

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_ProcessResponse()
 *
 * Description:
 * The DRM_Prdy_LicenseAcq_ProcessResponse function processes a license response,
 * which results from posting a license challenge. To process the license,
 * Drm_LicenseAcq_ProcessResponse() will be called using DRM_PROCESS_LIC_RESPONSE_NO_FLAGS
 * as the flag.
 *
 * Maps to:
 *  Drm_LicenseAcq_ProcessResponse(DRM_PROCESS_LIC_RESPONSE_NO_FLAGS)
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pData         [out] A pointer to a buffer containing the challenge response data
 *  dataLen       [in] Size of the challenge response data buffer
 *  pResponse     [out] Optional parameter to the response structure
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponse(
        DRM_Prdy_Handle_t       pPrdyContext,
        const char             *pData,
        uint32_t                dataLen,
        DRM_Prdy_License_Response_t *pResponse);

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_ProcessResponseNonPersistent()
 *
 * Description:
 * The DRM_Prdy_LicenseAcq_ProcessResponseNonPersistent function processes a license response for
 * non persistent licenses, which results from posting a license challenge. To process the license,
 * Drm_LicenseAcq_ProcessResponse() will be called using DRM_PROCESS_LIC_RESPONSE_SIGNATURE_NOT_REQUIRED
 * as the flag.
 *
 * Maps to:
 *  Drm_LicenseAcq_ProcessResponse(DRM_PROCESS_LIC_RESPONSE_SIGNATURE_NOT_REQUIRED)
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pData         [out] A pointer to a buffer containing the challenge response data
 *  dataLen       [in] Size of the challenge response data buffer
 *  pResponse     [out] Optional parameter to the response structure
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponseNonPersistent(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen,
        DRM_Prdy_License_Response_t *pResponse);

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_GenerateAck()
 *
 * Description:
 * The DRM_Prdy_LicenseAcq_GenerateChallenge function generates license acknowledge challenge
 *
 * Maps to:
 *  Drm_LicenseAcq_GenerateAck()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pResponse     [in] pointer to a structure that contains the license processing/storage result to send to the server
 *  pCh_data      [out] A pointer to a buffer for retrieving the challenge data
 *  pCh_len       [in|out] Size of the challenge data buffer
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  Application must allocate memory for the challenge data for this function to work.
 *  Application should call DRM_Prdy_Get_Buffer_Size to determine the proper buffer
 *  size of the challenge data.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GenerateAck(
        DRM_Prdy_Handle_t            pPrdyContext,
        DRM_Prdy_License_Response_t *pResponse, /* [in] */
        char                        *pCh_data,    /* [out] */
        uint32_t                    *pCh_len);

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_ProcessAckResponse()
 *
 * Description:
 * The DRM_Prdy_LicenseAcq_ProcessResponse function processes a license ACK response,
 * which results from posting a license ACK challenge.
 *
 * Maps to:
 *  Drm_LicenseAcq_ProcessAckResponse()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pData         [out] A pointer to a buffer containing the challenge ACK response data
 *  dataLen       [in] Size of the challenge response data buffer
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessAckResponse(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen);


/***********************************************************************************
 * Function: DRM_Prdy_Device_GetProperty()
 *
 * Description:
 * The Drm_Device_GetProperty function retrieves device properties
 * (DRM_DGP_CLIENT_INFO), which are the client and the device certificates).
 *
 * Maps to:
 *  Drm_Device_GetProperty()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pClient_into  [out] A pointer to a buffer for retrieving the client info data
 *  pCliInfo_lend [in] Size of the client info data buffer
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  Application must allocate memory for the client info for this function to work.
 *  Application should call DRM_Prdy_Get_Buffer_Size to determine the proper buffer
 *  size of the client info.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Device_GetProperty(
        DRM_Prdy_Handle_t      pPrdyContext,
        char                  *pClient_info,     /* [out] */
        uint32_t               pCliInfo_len);


/***********************************************************************************
 * Function: DRM_Prdy_Revocation_StorePackage()
 *
 * Description:
 * The DRM_Prdy_Revocation_StorePackage function stores the revocation package
 * obtained from the license server to the secure store.
 *
 * Maps to:
 *  Drm_Revocation_StorePackage()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context.
 *  pbRevPackage: [in] Pointer to the revocation package that needs to be stored.
 *  cbRevPackage: [in] Size, in bytes, of the revocation package.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Revocation_StorePackage(
        DRM_Prdy_Handle_t      pPrdyContext,
        char                  *pbRevPackage,
        uint32_t               cbRevPackage);


/***********************************************************************************
 * Function: DRM_Prdy_LocalLicense_InitializePolicyDescriptor()
 *
 * Description:
 * Initializes a policy descriptor with following default values:
 *   - wSecurityLevel 2000
 *   - fCannotPersist TRUE
 *   - oExpirationAfterFirstPlay fValid = FALSE
 *   - oSourceId fValid = FALSE
 *   - fRestrictedSourceId FALSE
 *   - wCompressedDigitalVideo 0
 *   - wUncompressedDigitalVideo 0
 *   - wAnalogVideo 0
 *   - wCompressedDigitalAudio 0
 *   - wUncompressedDigitalAudio 0
 *   - cExplicitAnalogVideoOPs 0
 *   - cExplicitDigitalAudioOPs 0
 *   - cPlayEnablers 0
 *
 * Maps to:
 *  Drm_LocalLicense_InitializePolicyDescriptor()
 *
 * Parameters:
 *  pPoDescriptor: [in, out] A DRM_Prdy_local_license_policy_descriptor_t object to be
 *                 initialized.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *   Please refer to Microsoft PlayReady Compliance and
 *   Robustness rules:  https://www.microsoft.com/playready/licensing/compliance/
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LocalLicense_InitializePolicyDescriptor(
        DRM_Prdy_local_license_policy_descriptor_t    *pPoDescriptor);


/***********************************************************************************
 * Function: DRM_Prdy_LocalLicense_CreateLicense()
 *
 * Description:
 * Creates a new license given the specified polices and license type.  This local
 * license must be released after use by calling the DRM_Prdy_LocalLicense_Release
 * function.
 *
 * Maps to:
 *  Drm_LocalLicense_CreateLicense()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context.
 *  pPoDescriptor:[in] A DRM_Prdy_local_license_policy_descriptor_t object containing
 *                the policies for the license to be created.
 *  eLicensType:  [in] A DRM_Prdy_local_license_type_e specifying the license type to
 *                be created.
 *  pKeyId        [in] Pointer to a Drm_Prdy_KID_t holding the key ID of the license to
 *                be created.
 *  cbRemoteCert: [in, optional] Size, in bytes, of the remote license to be bound to,
 *                if any.
 *  pbRemoteCert: [in, optional] Remote license to be bound to. NULL if none.
 *  hRootLicense: [in, optional] Handle of the root license if the created license is
 *                to be a leaf license. Otherwise, should be set to
 *                DRM_PRDY_LICENSE_HANDLE_INVALID.
 *  phLicense:    [out] Handle of the new license.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LocalLicense_CreateLicense(
        DRM_Prdy_Handle_t                                  pPrdyContext,
        const DRM_Prdy_local_license_policy_descriptor_t  *pPoDescriptor,
        DRM_Prdy_local_license_type_e                      elicenseType,
        const Drm_Prdy_KID_t                              *pKeyId,
        uint16_t                                           cbRemoteCert,
        const uint8_t                                     *pbRemoteCert,
        const DRM_Prdy_license_handle                      hRootLicense,
        const DRM_Prdy_license_handle                     *phLicense);


/***********************************************************************************
 * Function: DRM_Prdy_LocalLicense_StoreLicense()
 *
 * Description:
 * Stores a license in a specified license store.
 *
 * Maps to:
 *  Drm_LocalLicense_StoreLicense()
 *
 * Parameters:
 *  hLicense:      [in] Handle of the license to be stored.
 *  eLicenseStore: [in] A DRM_Prdy_local_license_store_e value specifying where to
 *                 store the license.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LocalLicense_StoreLicense(
        const DRM_Prdy_license_handle     hLicense,
        DRM_Prdy_local_license_store_e    eLicenseStore);


/***********************************************************************************
 * Function: DRM_Prdy_LocalLicense_Release()
 *
 * Description:
 * Decrements the reference count for a specified license handle.
 *
 * Maps to:
 *  Drm_LocalLicense_Release()
 *
 * Parameters:
 *  hLicense: [in|out] Handle for the license for which the reference count is to
 *            be decremented. See Note below.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *   If the reference count for the given license handle falls to zero, the memory
 *   associated with the license handle is freed from memory and the license handle
 *   is set to DRM_PRDY_LICENSE_HANDLE_INVALID.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LocalLicense_Release(DRM_Prdy_license_handle   *hLicense);


/***********************************************************************************
 * Function: DRM_Prdy_LocalLicense_EncryptSample()
 *
 * Description:
 * Encrypts a sample buffer using AES in counter mode.
 *
 * Maps to:
 *  Drm_LocalLicense_EncryptSample()
 *
 * Parameters:
 *  hLicense: [in] Handle of the license to use for the encryption.
 *  pInBuf:   [in] A memory buffer containing an input sample.
 *  pOutBuf:  [in|out] A memory buffer to receive output encrypted samples.
 *  cbData:   [in] Size, in bytes, of the input sample.
 *  pIV:      [out] The initialization vector (IV) used to encrypt the samples
 *            to be returned. Only with IV size 8 is supported.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *   Please make sure that the pOutBuf is properly allocated using
 *   NEXUS_Memory_Allocate.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LocalLicense_EncryptSample(
        DRM_Prdy_license_handle   *hLicense,
        uint8_t                   *pInBuf,
        uint8_t                   *pOutBuf,
        uint32_t                   cbData,
        uint8_t                   *pIV);


/***********************************************************************************
 * Function: DRM_Prdy_LocalLicense_EncryptSubsamples()
 *
 * Description:
 * Encrypts a sample buffer using AES in counter mode with the suport of subsampling
 * encryption .
 *
 * Maps to:
 *  Drm_LocalLicense_EncryptSample()
 *
 * Parameters:
 *  hLicense: [in] Handle of the license to use for the encryption.
 *  pInBuf:   [in] A memory buffer containing an input subsamples.
 *  pOutBuf:  [in|out] A memory buffer to receive output encrypted subsamples.
 *  pIV:      [out] The initialization vector (IV) used to encrypt the samples
 *            to be returned. Only with IV size 8 is supported.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *   Please make sure that the pOutBuf is properly allocated using
 *   NEXUS_Memory_Allocate.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LocalLicense_EncryptSubsamples(
        DRM_Prdy_license_handle   *hLicense,
        DRM_Prdy_sample_t         *pClearSamples,
        DRM_Prdy_sample_t         *pEncSamples,
        uint8_t                   *pIV);

/***********************************************************************************
 * Function: DRM_Prdy_LocalLicense_GetKID()
 *
 * Description:
 * Returns the key ID (KID) for a specified license handle.
 *
 * Maps to:
 *  Drm_LocalLicense_GetKID()
 *
 * Parameters:
 *  hLicense: [in] Handle of the license.
 *  pKeyID:   [out] Key ID of the license.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LocalLicense_GetKID(
        const DRM_Prdy_license_handle    hLicense,
        Drm_Prdy_KID_t                  *pKeyID );


/***********************************************************************************
 * Function: DRM_Prdy_LocalLicense_GetKID_Base64W()
 *
 * Description:
 * Returns the key ID (KID) for a specified license handle in base64 wchar format.
 *
 * Maps to:
 *  Drm_LocalLicense_GetKID()
 *
 * Parameters:
 *  hLicense:      [in] Handle of the license.
 *  pKidBase64W:   [out] Key ID of the license.
 *  pRequiredSize: [out, optional] required buffer size.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *   If the buffer is too small or the buffer are NULL, we return the required
 *   buffer siz
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LocalLicense_GetKID_base64W(
        const DRM_Prdy_license_handle    hLicense,
        uint16_t                        *pKidBase64W,
        uint32_t                        *pRequiredSize );


/***********************************************************************************
 * Function: DRM_Prdy_StoreMgmt_DeleteLicenses()
 *
 * Description:
 * Deletes all licenses with a given KID, and removes them from the sync list to
 * prevent them from being automatically reacquired.
 *
 * Maps to:
 *  Drm_StoreMgmt_DeleteLicenses()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pKidBase64W: [in, Optional] KID of licenses to delete, if NULL then the KID must
 *               have been passed in via a content header (via
 *               DRM_Prdy_Content_SetProperty for example).
 * pcLicDeted: [out] to receive the number of licenses deleted.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_StoreMgmt_DeleteLicenses(
        DRM_Prdy_Handle_t                   pPrdyContext,
        const uint16_t                     *pKidBase64W,
        uint32_t                            cbKidSize,
        uint32_t                           *pcLicDeleted);


/***********************************************************************************
 * Function: DRM_Prdy_Cleanup_LicenseStores()
 *
 * Description:
 * The DRM_Prdy_Cleanup_LicenseStores function deletes all licenses from the license
 * store.
 *
 * Maps to:
 *  Drm_StoreMgmt_CleanupStore()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Cleanup_LicenseStores( DRM_Prdy_Handle_t  pPrdyContext);


/***********************************************************************************
 * Function: DRM_Prdy_Cleanup_Expired_LicenseStores()
 *
 * Description:
 * The DRM_Prdy_Cleanup_Expired_LicenseStores function deletes expired licenses from
 * the license store.
 *
 * Maps to:
 *  Drm_StoreMgmt_CleanupStore()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Cleanup_Expired_LicenseStores( DRM_Prdy_Handle_t  pPrdyContext);


/*
************************************************************************************
** Prdy ND APIs
************************************************************************************
*/

/***********************************************************************************
 * Function: DRM_Prdy_ND_MemFree()
 *
 * Description:
 *  Frees a block of memory that is previously created and allocated by a Prdy ND
 *  function.
 *
 * Maps to:
 *  Drm_Prnd_MemFree.
 *
 * Parameters:
 *  pbToFree: [in] Pointer to a previously allocated block of memory to be freed.
 *
 * Return:
 *  n/a
 *
 * Note:
 *
 ***********************************************************************************/
void DRM_Prdy_ND_MemFree( uint8_t   *pbToFree);


/***********************************************************************************
 * Function: DRM_Prdy_ND_GetMessageType()
 *
 * Description:
 *  Returns the message type for a given message.
 *
 * Maps to:
 *  Drm_Prnd_GetMessageType.
 *
 * Parameters:
 *  pbUnknownMessage: [in] Pointer to a message whose type is to be determined.
 *  cbUnknownMessage: [in] Size, in bytes, of the message.
 *  peMessageType: [out] One of the DRM_Prdy_ND_Message_Type_e values indicating the
 *                 message type.
 *
 * Return:
 *   On Success:   DRM_Prdy_ok and pPrdyRxCtx becames NULL
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_GetMessageType(
        const uint8_t               *pbUnknownMessage,
        uint16_t                     cbUnknownMessage,
        DRM_Prdy_ND_Message_Type_e  *peMessageType );


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_Initialize()
 *
 * Description:
 * The DRM_Prdy_ND_Transmitter_Initialize() creates and initializes the
 * DRM_PRDY_ND_TRANSMITTER_CONTEXT. The transmitter context will be used to begin
 * a new PRND transmitter session.
 *
 * Maps to:
 *  N/A
 *
 * Parameters:
 *  N/A
 *
 * Return:
 *  On Success:   A valid DRM_Prdy_ND_Transmitter_Context_t
 *
 * Note:
 *  Application must call DRM_Prdy_ND_Transmitter_Uninitialize() to release the
 *  resources when finishes using it.
 *
 ***********************************************************************************/
DRM_Prdy_ND_Transmitter_Context_t DRM_Prdy_ND_Transmitter_Initialize(void);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_Uninitialize()
 *
 * Description:
 * The DRM_Prdy_ND_Transmitter_Uninitialize() releases all the resouces allocated.
 *
 * Maps to:
 *  N/A
 *
 * Parameters:
 *  pPrdyTxCtr: [in, out] a pointer to the DRM_Prdy_ND_Transmitter_Context_t
 *
 * Return:
 *   On Success:   DRM_Prdy_ok and pPrdyTxCtx becames NULL
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_Uninitialize(
        DRM_Prdy_ND_Transmitter_Context_t  pPrdyTxCtx);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_Initialize()
 *
 * Description:
 * The DRM_Prdy_ND_Receiver_Initialize() creates and initializes the
 * DRM_PRDY_ND_RECEIVER_CONTEXT. The receiver context will be used to begin a new
 * PRND receiver session.
 *
 * Maps to:
 *  N/A
 *
 * Parameters:
 *  N/A
 *
 * Return:
 *  On Success:   A valid DRM_Prdy_ND_Receiver_Context_t
 *
 * Note:
 *  Application must call DRM_Prdy_ND_Receiver_Uninitialize() to release the
 *  resources when finishes using it.
 *
 ***********************************************************************************/
DRM_Prdy_ND_Receiver_Context_t DRM_Prdy_ND_Receiver_Initialize(void);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_Uninitialize()
 *
 * Description:
 * The DRM_Prdy_ND_Receiver_Uninitialize() releases all the resouces allocated.
 *
 * Maps to:
 *  N/A
 *
 * Parameters:
 *  pPrdyTxCtr: [in, out] a pointer to the DRM_Prdy_ND_Receiver_Context_t
 *
 * Return:
 *   On Success:   DRM_Prdy_ok and pPrdyRxCtx becames NULL
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_Uninitialize(
        DRM_Prdy_ND_Receiver_Context_t  pPrdyRxCtx);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_BeginSession()
 *
 * Description:
 * The DRM_Prdy_ND_Transmitter_BeginSession() initializes a Transmitter for
 * PlayReady-ND service. A valid DRM_Prdy_ND_Transmitter_Context_t must be initialized
 * by calling DRM_Prdy_ND_Transmitter_Initialize before using this function.
 *
 * Maps to:
 *  N/A
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pPrdyTxCtr:   [in] A valid DRM_Prdy_ND_Transmitter_Context_t
 *
 * Return:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_BeginSession(
        DRM_Prdy_Handle_t                   pPrdyContext,
        DRM_Prdy_ND_Transmitter_Context_t   pPrdyTxCtx);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_BeginSession()
 *
 * Description:
 * The DRM_Prdy_ND_Receiver_BeginSession() initializes a Receiver for
 * PlayReady-ND service. A valid DRM_Prdy_ND_Receiver_Context_t must be initialized
 * by calling DRM_Prdy_ND_Receiver_Initialize before using this function.
 *
 * Maps to:
 *  N/A
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pPrdyRxCtr:   [in] A valid DRM_Prdy_ND_Receiver_Context_t
 *
 * Return:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_BeginSession(
        DRM_Prdy_Handle_t                pPrdyContext,
        DRM_Prdy_ND_Receiver_Context_t   pPrdyRxCtx);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_EndSession()
 *
 * Description:
 * The DRM_Prdy_ND_Transmitter_EndSession() terminates the Transmitter
 * PlayReady-ND service.
 *
 * Maps to:
 *  N/A
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *
 * Return:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_EndSession(
        DRM_Prdy_Handle_t                   pPrdyContext);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_EndSession()
 *
 * Description:
 * The DRM_Prdy_ND_Receiver_EndSession() terminates the Receiver
 * PlayReady-ND service.
 *
 * Maps to:
 *  N/A
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *
 * Return:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_EndSession(
        DRM_Prdy_Handle_t                   pPrdyContext);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_RegistrationRequest_Process
 *
 * Description:
 * Processes a registration request from a Receiver.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_RegistrationRequest_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbReqMessage: [in] Pointer to a registration request message from a Receiver.
 *  cbReqMessage: [in] Size, in bytes, of the registration request message.
 *  pfnDataCallback: [in, optional] Pointer to a DRM_Prdy_ND_Data_Callback function
 *                that will handle the Custom Data and/or Certificate processing.
 *                NULL, if processing is not required.
 *  pDataCallbackContext: [in, out, optional] Optional context information that the
 *                application wishes to pass to the callback function. NULL if none
 *                required.
 *  pSessionID: [out] An identifier for this session with the Receiver. This is a
 *                cryptographically random unique ID generated each time a message of
 *                this type is sent, except in the case of Renewal, and is generated
 *                by the function all.
 *  peProximityDetectionType: [out] A set of DRM_Prdy_ND_Proximity_Detection_Type_e
 *                bits indicating which proximity detection types the Receiver
 *                supports.
 *  ppbTransmitterProximityDetectionChannel: [out] Pointer to a buffer pointer that
 *                will receive data for the proximity detection channel.
 *  pcbTransmitterProximityDetectionChannel: [out] Pointer to a uint16_t that will
 *                receive the size of the proximity detection channel data.
 *  pdwFlags: [out] Pointer to a uint16_t containing flags indicating additional
 *                modifications to the registration request message.
 *  pPrdyResultCode: [out] A Playready result code. If this function fails,
 *                   pass this result code to generate error response message to
 *                   the receiver.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_RegistrationRequest_Process(
        DRM_Prdy_Handle_t                         pPrdyContext,
        const uint8_t                            *pbReqMessage,
        uint16_t                                  cbReqMessage,
        DRM_Prdy_ND_Data_Callback                 pfnDataCallback,
        void                                     *pvDataCallbackContext,
        DRM_Prdy_ID_t                            *pSessionID,
        DRM_Prdy_ND_Proximity_Detection_Type_e   *peProximityDetectionType,
        uint8_t                                 **ppbTransmitterProximityDetectionChannel,
        uint16_t                                 *pcbTransmitterProximityDetectionChannel,
        uint16_t                                 *pdwFlags,
        uint32_t                                 *pPrdyResultCode );


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_RegistrationResponse_Generate
 *
 * Description:
 * Generates a registration response to be sent to a Receiver.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_RegistrationResponse_Generate
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pCustomDataTypeID: [in, optional] The type of Custom Data being used.
 *  pbCustomData: [in, optional] Any optional custom data the Receiver sent to the
 *                Transmitter in the Registration Request. This data is treated as
 *                opaque by the PRND protocol and is returned unaltered inthe response.
 *  cbCustomData: [in, optional] Size, in bytes, of the Custom Data.
 *  dwFlags: [in] Reserved for future use. Should be set to DRM_PRDY_ND_NO_FLAGS.
 *  ppbRespMessage: [out] Pointer to a buffer pointer that will receive the
 *                  Registration Response message. This buffer must be freed after use
 *                  by calling the Drm_Prnd_MemFree function.
 *  pcbRespMessage: [out] Pointer to a uint16_t that will receive the size, in bytes,
 *                  of the Registration Response message.
 *
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_RegistrationResponse_Generate(
        DRM_Prdy_Handle_t        pPrdyContext,
        const DRM_Prdy_ID_t     *pCustomDataTypeID,
        const uint8_t           *pbCustomData,
        uint16_t                 cbCustomData,
        uint16_t                 dwFlags,
        uint8_t                **ppbRespMessage,
        uint16_t                *pcbRespMessage);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_RegistrationError_Generate
 *
 * Description:
 * Generates a registration error response message to be sent to a Receiver.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_RegistrationError_Generate
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  prdyResultCode : [in] Error code indicating why the Registration Request failed.
 *  dwFlags: [in] Reserved for future use. Should be set to DRM_PRDY_ND_NO_FLAGS.
 *  ppbErrMessage: [out] Pointer to a buffer pointer that will receive the Registration
 *                  Error message to be sent to the Receiver. This buffer must be
 *                  freed after use by calling the Drm_Prnd_MemFree function.
 *  pcbErrMessage: [out] Pointer to a uint16_t that will receive the size, in bytes,
 *                 of the Registration Error message.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_RegistrationError_Generate(
        DRM_Prdy_Handle_t        pPrdyContext,
        uint32_t                 prdyResultCode,
        uint16_t                 dwFlags,
        uint8_t                **ppbErrMessage,
        uint16_t                *pcbErrMessage);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_LicenseRequest_Process
 *
 * Description:
 * Processes a license request message from a Receiver.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_LicenseRequest_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbLicReqMessage: [in] Pointer to a buffer holding the License Request message
 *                   from a Receiver.
 *  cbLicReqMessage: [in] Size, in bytes, of the License Request message.
 *  pfnDataCallback: [in, optional] Pointer to a Drm_Prnd_Data_Callback function
 *                   implementation that will handle the Custom Data and Certificate
 *                   processing for the License Request.
 *  pvDataCallbackContext: [in, out, optional] Optional context information that the
 *                          application wishes to pass to the callback function.
 *                          NULL if none required.
 *  pfnContentIdentifierCallback: [in, out, optional] Pointer to a
 *                                Drm_Prdy_ND_Content_Identifier_Callback function
 *                                implementation that will handle the Content Identifier
 *                                processing for the License Request.
 *  pvContentIdentifierCallbackContext: [in, out, optional] Optional context information
 *                                      that the application wishes to pass to the
 *                                      callback function. NULL if none required.
 *  pdwFlags: [out] Reserved for future use. Will always be DRM_PRDY_ND_NO_FLAGS.
 *  pPrdyResultCode: [out] A Playready result code. If this function fails,
 *                   pass this result code to generate error response message to
 *                   the receiver.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_LicenseRequest_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbLicReqMessage,
        uint16_t                                   cbLicReqMessage,
        DRM_Prdy_ND_Data_Callback                  pfnDataCallback,
        void                                      *pvDataCallbackContext,
        Drm_Prdy_ND_Content_Identifier_Callback    pfnContentIdentifierCallback,
        void                                      *pvContentIdentifierCallbackContext,
        uint16_t                                  *pdwFlags,
        uint32_t                                  *pPrdyResultCode );


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_LicenseTransmit_Generate
 *
 * Description:
 * Generates a license transmit message for a Receiver's license request.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_LicenseTransmit_Generate
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pCustomDataTypeID: [in, optional] The type of Custom Data being used. See Note
 *                     below.
 *  pbCustomData: [in, optional] Any optional custom data the Receiver wishes to
 *                send to the Transmitter during this License Request. This data is
 *                treated as opaque by the PRND protocol and is delivered back to the
 *                Receiver.
 *  cbCustomData: [in] Size, in bytes, of the custom data.
 *  dwFlags: [in] additional flags indicating modifications to this message.
 *  ppbLicTransmitMessage: [out] Pointer to a buffer pointer that will receive the
 *                         License Message to be sent to the Receiver. This buffer
 *                         must be freed after use by calling the Drm_Prnd_MemFree
 *                         function.
 *  pcbLicTransmitMessage: [out] Pointer to a uint16_t that will receive the size,
 *                         in bytes, of the License Message.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_LicenseTransmit_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pCustomDataTypeID,
        const uint8_t                             *pbCustomData,
        uint16_t                                   cbCustomData,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbLicTransmitMessage,
        uint16_t                                  *pcbLicTransmitMessage);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_PrepareLicensesForTransmit
 *
 * Description:
 * ares licenses to be sent in the response to a Receiver's license request.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_PrepareLicensesForTransmit
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *  This function assumes that a license or license chain is already bound using
 *  DRM_Prdy_Reader_Bind. It rebinds that license or license chain from the Transmitter
 *  to the Receiver.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_PrepareLicensesForTransmit(
        DRM_Prdy_Handle_t                          pPrdyContext);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_LicenseError_Generate
 *
 * Description:
 * Generates an error response message for a Receiver's license request.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_LicenseError_Generate
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pSessionID: [in, optional] The Session Identifier. The value must match the value
 *              originally sent from Transmitter to Receiver in the Registration
 *              Response Message or, if unavailable, the value sent in the message
 *              to which this message is responding.
 *  prdyResultCode : [in] Error code indicating why the License Request failed.
 *  dwFlags: [in] Reserved for future use. Should be set to DRM_PRDY_ND_NO_FLAGS.
 *  ppbErrMessage: [out] Pointer to a buffer pointer that will receive the Registration
 *                  Error message to be sent to the Receiver. This buffer must be
 *                  freed after use by calling the Drm_Prnd_MemFree function.
 *  pcbErrMessage: [out] Pointer to a uint16_t that will receive the size, in bytes,
 *                 of the Registration Error message.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_LicenseError_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pSessionID,
        uint32_t                                   prdyResultCode,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbErrMessage,
        uint16_t                                  *pcbErrMessage);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_ProximityDetectionStart_Process
 *
 * Description:
 *  Processes a proximity detection start request from a Receiver.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_ProximityDetectionStart_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbPDStartMessage: [in] Proximity Start request message from a Receiver.
 *  cbPDStartMessage: [in] Size, in bytes, of the Proximity Start message.
 *  dwFlags: [in] Reserved for future use. Will always be DRM_PRDY_ND_NO_FLAGS.
 *  ppbPDChlgMessage: [out] Pointer to a buffer pointer that will receive the
 *                    Proximity Detection Challenge message to be sent to the Receiver.
 *                    This buffer must be freed after use by calling the Drm_Prnd_MemFree
 *                    function.
 *  pcbPDChlgMessage: [out] Pointer to a uint16_t that will receive the size,
 *                      in bytes, of the Proximity Result message.
 *  pdwFlags: [out] Reserved for future use. Will always be DRM_PRDY_ND_NO_FLAGS.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_ProximityDetectionStart_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbPDStartMessage,
        uint16_t                                   cbPDStartMessage,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbPDChlgMessage,
        uint16_t                                  *pcbPDChlgMessage,
        uint16_t                                  *pdwFlags);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_ProximityDetectionResponse_Process
 *
 * Description:
 * Processes a proximity detection response message from a Receiver.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_ProximityDetectionResponse_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbPDRespMessage: [in] Pointer to a buffer containing the Proximity Response message
 *                   from the Receiver.
 *  cbPDRespMessage: [in] Size, in bytes, of the Proximity Response message.
 *  dwFlags: [in] Reserved for future use. Will always be DRM_PRDY_ND_NO_FLAGS.
 *  ppbPDResultMessage: [out] Pointer to a buffer pointer that will receive the
 *                      Proximity Result message to be sent to the Receiver. This
 *                      buffer must be freed after use by calling the Drm_Prnd_MemFree
 *                      function.
 *  pcbPDResultMessage: [out] Pointer to a uint16_t that will receive the size,
 *                      in bytes, of the Proximity Result message.
 *  pdwFlags: [out] Reserved for future use. Will always be DRM_PRDY_ND_NO_FLAGS.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_ProximityDetectionResponse_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbPDRespMessage,
        uint16_t                                   cbPDRespMessage,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbPDResultMessage,
        uint16_t                                  *pcbPDResultMessage,
        uint16_t                                  *pdwFlags);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Transmitter_ProximityDetectionResult_Generate
 *
 * Description:
 *  Generates a proximity detection result message to be sent to a Receiver.
 *
 * Maps to:
 *  Drm_Prnd_Transmitter_ProximityDetectionResult_Generate
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pSessionID: [in, optional] The Session Identifier. The value must match the value
 *               originally sent from Transmitter to Receiver in the Registration
 *               Response Message or, if unavailable, the value sent in the message
 *               to which this message is responding.
 *  prdyResultCode : [in] The failure code indicating why the proximity detection
 *                   operation failed.
 *  dwFlags: [in] Reserved for future use. Should be set to DRM_PRDY_ND_NO_FLAGS.
 *  ppbPDResultMessage: [out] Pointer to a buffer pointer that will receive the
 *                      Proximity Result message to be sent to the Receiver. This
 *                      buffer must be freed after use by calling the Drm_Prnd_MemFree
 *                      function.
 *  pcbPDResultMessage: [out] Pointer to a uint16_t that will receive the size,
 *                      in bytes, of the Proximity Result message.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_ProximityDetectionResult_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pSessionID,
        uint32_t                                   prdyResultCode,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbPDResultMessage,
        uint16_t                                  *pcbPDResultMessage);



/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_RegistrationRequest_Generate
 *
 * Description:
 *  Generates a registration request for sending to the Transmitter.
 *
 * Maps to:
 *  Drm_Prnd_Receiver_RegistrationRequest_Generate
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pPreviousSessionID: [in, optional] Pointer to a Session Identifier from a previous
 *                      session, if this is a renewal request. The Session Identifier
 *                      must match the one from the session that is being renewed.
 *                      If this is not a renewal request, this parameter should be
 *                      set to NULL.
 *  pCustomDataTypeID: [in, optional] The type of Custom Data being used.
 *  pbCustomData: [in, optional] Pointer to a buffer containing Custom Data. This is
 *                any optional custom data the Receiver wishes to send to the
 *                Transmitter during Registration. For example, this may include
 *                Authentication and/or Authorization data that the Receiver requires.
 *                This data is treated as opaque by the PlayReady-ND protocol.
 *  cbCustomData: [in] Size, in bytes, of the Custom Data.
 *  dwFlags: [in] Flags indicating additional modifications to this message.
 *  ppbReqMessage: [out] Pointer to a buffer pointer that will receive the registration
 *                 request message. This buffer must be freed after use by calling
 *                 the Drm_Prnd_MemFree function.
 *  pcbReqMessage: [in, out] Pointer to a uint16_t that will receive the size,
 *                 in bytes, of the registration request message.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_RegistrationRequest_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pPreviousSessionID,
        const DRM_Prdy_ID_t                       *pCustomDataTypeID,
        const uint8_t                             *pbCustomData,
        uint16_t                                   cbCustomData,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbReqMessage,
        uint16_t                                  *pcbReqMessage);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_RegistrationResponse_Process
 *
 * Description:
 *  Processes a registration response from the Transmitter.
 *
 * Maps to:
 *  Drm_Prnd_Receiver_RegistrationResponse_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbRespMessage: [in] Pointer to the Registration response message from the
 *                 Transmitter.
 *  cbRespMessage: [in] Size, in bytes, of the Registration response message from
 *                 the Transmitter.
 *  pfnDataCallback: [in, optional] Pointer to a Drm_Prnd_Data_Callback function
 *                   that will handle the Custom Data and Certificate processing.
 *                   NULL, if not required.
 *  pvDataCallbackContext: [in, out, optional] Optional context information that the
 *                         application wishes to pass to the callback function. NULL
 *                         if none required.
 *  pSessionID: [out] Pointer to a Session Identifier generated by the Transmitter,
 *              or NULL if the Registration Request was for a Renewal.
 *  peProximityDetectionType: [out] One of DRM_Prdy_ND_Proximity_Detection_Type_e types.
 *  ppbTransmitterProximityDetectionChannel: [out] Pointer to a buffer pointer to a
 *                  null-terminated ANSI string describing the Transmitter Proximity
 *                  Detection Channel.
 *  pcbTransmitterProximityDetectionChannel: [out] Size, in bytes, of the Transmitter
 *                  Proximity Detection Channel description string.
 *  pdwFlags: [out] Reserved for future use. Will always be DRM_PRDY_ND_NO_FLAGS.
 *  pPrdyResultCode: [out] A Playready result code. If this function fails,
 *                   pass this result code to generate error response message to
 *                   the receiver.
 *
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_RegistrationResponse_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbRespMessage,
        uint16_t                                   cbRespMessage,
        DRM_Prdy_ND_Data_Callback                  pfnDataCallback,
        void                                      *pvDataCallbackContext,
        DRM_Prdy_ID_t                             *pSessionID,
        DRM_Prdy_ND_Proximity_Detection_Type_e    *peProximityDetectionType,
        uint8_t                                  **ppbTransmitterProximityDetectionChannel,
        uint16_t                                  *pcbTransmitterProximityDetectionChannel,
        uint16_t                                  *pdwFlags,
        uint32_t                                  *pPrdyResultCode );


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_RegistrationError_Process
 *
 * Description:
 *  Processes a registration error message from the Transmitter.
 *
 * Maps to:
 *  Drm_Prnd_Receiver_RegistrationError_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbErrMessage: [in] Pointer to a Registration Error message received from the
 *                 Transmitter.
 *  cbErrMessage: [in] Size, in bytes, of the Registration Error message.
 *  pPrdyResultCode: [out] Error code indicating why the Registration Request failed.
 *  pdwFlags: [out] Reserved for future use. Should be set to DRM_PRDY_ND_NO_FLAGS.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_RegistrationError_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbErrMessage,
        uint16_t                                 cbErrMessage,
        uint32_t                                *pPrdyResultCode,
        uint16_t                                *pdwFlags);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_LicenseRequest_Generate
 *
 * Description:
 *  Generates a license request message for sending to the Transmitter.
 *
 * Maps to:
 *  Drm_Prnd_Receiver_LicenseRequest_Generate
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pguidRequestedAction [in] This parameter should always be set to DRM_PRDY_ND_ACTION_PLAY.
 *  pguidRequestedActionQualifier: [in, optional] Reserved for future use. Should be set to NULL.
 *  eContentIdentifierType: [in] One of the DRM_Prdy_ND_Content_Identifier_Type_e
 *                         values indicating the Content Identifier type.
 *  pbContentIdentifier: [in] Pointer to a buffer containing a Content Identifier
 *                       identifying the content the Receiver wishes to act upon.
 *  cbContentIdentifier: [in] Size, in bytes, of the requested Content Identifier buffer.
 *  pCustomDataTypeID: [in, optional] The type of Custom Data being used.
 *  pbCustomData: [in, optional] Any optional custom data the Receiver wishes to
 *                send to the Transmitter during this License Request. This data is
 *                treated as opaque by the PRND protocol and must be delivered to
 *                the Transmitter application.
 *  cbCustomData: [in] Size, in bytes, of the Custom Data.
 *  dwFlags: [in] Reserved for future use. Should be set to DRM_PRDY_ND_NO_FLAGS.
 *  ppbLicReqMessage: [out] Pointer to a buffer pointer that will receive the License
 *                     Request Message. This buffer must be freed after use by calling
 *                     the Drm_Prnd_MemFree function.
 *  pcbLicReqMessage: [out] Size, in bytes, of the License Request Message.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_LicenseRequest_Generate(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const DRM_Prdy_guid_t                   *pguidRequestedAction,
        const DRM_Prdy_guid_t                   *pguidRequestedActionQualifier,
        DRM_Prdy_ND_Content_Identifier_Type_e    eContentIdentifierType,
        const uint8_t                           *pbContentIdentifier,
        uint16_t                                 cbContentIdentifier,
        const DRM_Prdy_ID_t                     *pCustomDataTypeID,
        const uint8_t                           *pbCustomData,
        uint16_t                                 cbCustomData,
        uint16_t                                 dwFlags,
        uint8_t                                **ppbLicReqMessage,
        uint16_t                                *pcbLicReqMessage);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Drm_Prnd_Receiver_LicenseTransmit_Process
 *
 * Description:
 *  Processes a license response message from the Transmitter.
 *
 * Maps to:
 *  Drm_Prnd_Receiver_LicenseTransmit_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbLicTransmitMessage: [in] Pointer to a buffer containing the License message
 *                        from the Transmitter
 *  cbLicTransmitMessage: [in] Size, in bytes, of the License message buffer.
 *  pfnDataCallback: [in, optional] Pointer to a DRM_Prdy_ND_Data_Callback function
 *                   implementation that will handle the Custom Data and Certificate
 *                  processing for the License message.
 *  pvDataCallbackContext: [in, out, optional] Optional context information that the
 *                         application wishes to pass to the callback function.
 *                         NULL if none required.
 *  pdwFlags: [out] Pointer to additional flags indicating modifications to this
 *                  message.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_LicenseTransmit_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbLicTransmitMessage,
        uint16_t                                 cbLicTransmitMessage,
        DRM_Prdy_ND_Data_Callback                pfnDataCallback,
        void                                    *pvDataCallbackContext,
        uint16_t                                *pdwFlags);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_LicenseError_Process
 *
 * Description:
 *  Processes a license error message from the Transmitter.
 *
 * Maps to:
 *  Drm_Prnd_Receiver_LicenseError_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbErrMessage: [in] Pointer to the License error message received from the
 *                 Transmitter.
 *  cbErrMessage: [in] Size, in bytes, of the License Error message.
 *  pPrdyResultCode: [out] Error code indicating why the License Request failed.
 *  pdwFlags: [out] Reserved for future use. Should be set to DRM_PRDY_ND_NO_FLAGS.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_LicenseError_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbErrMessage,
        uint16_t                                 cbErrMessage,
        uint32_t                                *pPrdyResultCode,
        uint16_t                                *pdwFlags);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_ProximityDetectionStart_Generate
 *
 * Description:
 *  Generates a proximity detection start request message for sending to the
 *  Transmitter.
 *
 * Maps to:
 *  Drm_Prnd_Receiver_ProximityDetectionStart_Generate
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  dwFlags: [in] Reserved for future use. Should be set to DRM_PRDY_ND_NO_FLAGS.
 *  ppbPDStartMessage: [out] Pointer to a buffer pointer that will receive the
 *                     Proximity Start message.
 *  pcbPDStartMessage: [out] Pointer to an uint16_t that will receive the size,
 *                     in bytes, of the Proximity Start message. This buffer must be
 *                     freed after use by calling the Drm_Prnd_MemFree function.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_ProximityDetectionStart_Generate(
        DRM_Prdy_Handle_t                        pPrdyContext,
        uint16_t                                 dwFlags,
        uint8_t                                **ppbPDStartMessage,
        uint16_t                                *pcbPDStartMessage);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_ProximityDetectionChallenge_Process
 *
 * Description:
 *  Processes the proximity detection challenge message from the Transmitter.
 *
 * Maps to:
 *  Drm_Prnd_Receiver_ProximityDetectionChallenge_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbPDChlgMessage: [in] Challenge Message from the Transmitter.
 *  cbPDChlgMessage: [in] Size, in bytes, of the Challenge Message.
 *  dwFlags: [in] Reserved for future use. Should be set to DRM_PRDY_ND_NO_FLAGS.
 *  ppbPDRespMessage: [out] Pointer to a buffer pointer that will return the Response
 *                    Message to be sent to the Transmitter. This buffer must be freed
 *                    after use by calling the Drm_Prnd_MemFree function.
 *  pcbPDRespMessage: [out] Pointer to a DRM_DWORD that will receive the size, in bytes,
 *                    of the Response Message.
 *  pdwFlags: [out] Reserved for future use. Will always be DRM_PRDY_ND_NO_FLAGS.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_ProximityDetectionChallenge_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbPDChlgMessage,
        uint16_t                                 cbPDChlgMessage,
        uint16_t                                 dwFlags,
        uint8_t                                **ppbPDRespMessage,
        uint16_t                                *pcbPDRespMessage,
        uint16_t                                *pdwFlags);


/***********************************************************************************
 * Function: DRM_Prdy_ND_Receiver_ProximityDetectionResult_Process
 *
 * Description:
 *  Processes the proximity detection operation results.
 *
 * Maps to:
 *  Drm_Prnd_Receiver_ProximityDetectionResult_Process
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context that contains the state data of the
 *                current DRM session.
 *  pbPDResultMessage: [in] Pointer to the Proximity Detection Result message from
 *                     the Transmitter.
 *  cbPDResultMessage: [in] Size, in bytes, of the Proximity Detection Result message.
 *  pdwFlags: [out] Reserved for future use. Will always be DRM_PRDY_ND_NO_FLAGS.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_ProximityDetectionResult_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbPDResultMessage,
        uint16_t                                 cbPDResultMessage,
        uint16_t                                *pdwFlags);


/* Helper functions */
DRM_Prdy_Error_e DRM_Prdy_B64_EncodeW( uint8_t *pBytes, uint32_t cbBytes, uint16_t *pBase64W, uint32_t *pReqSize);
bool DRM_Prdy_convertCStringToWString( char * pCStr, wchar_t * pWStr, uint16_t * cchStr);
bool DRM_Prdy_convertWStringToCString( wchar_t * pWStr, char * pCStr, uint32_t cchStr);
void DRM_Prdy_qwordToNetworkbytes(uint8_t *bytes,unsigned index,uint64_t qword);
uint32_t DRM_Prdy_Cch_Base64_Equiv(size_t cb);


/***********************************************************************************
 * Function: DRM_Prdy_License_GetProperty()
 *
 * Description:
 *  DRM_Prdy_License_GetProperty() gets properties for bound licenses.
 *  Must be called after a successful bind via DRM_Prdy_Reader_Bind() and prior to
 *  DRM_Prdy_Reader_Commit().
 *
 * Maps to:
 *  Drm_License_GetProperty
 *
 * Parameters:
 *  pPrdyContext:                   [in] A valid DRM_Prdy_Context
 *  licenseProperty:                [in] Property type
 *  pLicensePropertyExtraData:      [in] Pointer to the extra data buffer (uint8_t)
 *  pLicensePropertyExtraDataSize:  [in] Pointer to the extra data size (uint32_t)
 *  pLicensePropertyOutputData:     [out] Pointer to the output data (uint32_t)
 *
 * Returns:
 *  On Success:   DRM_Prdy_ok
 *  On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_License_GetProperty(
        DRM_Prdy_Handle_t pPrdyContext,
        DRM_Prdy_license_property_e licenseProperty,
        const uint8_t *pLicensePropertyExtraData,
        const uint32_t *pLicensePropertyExtraDataSize,
        uint32_t *pLicensePropertyOutputData);



/***********************************************************************************
 * Function: DRM_Prdy_LicenseQuery_GetState()
 *
 * Description:
 * DRM_Prdy_LicenseQuery_GetState reads a license state for a requested right type
 * and returns the state to the application.
 *
 * Maps to:
 *  Drm_LicenseQuery_GetState
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  licenseRight:    [in] enum value for a right to query
 *  pLicenseState:   [out] A pointer to a structure to store the license state
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseQuery_GetState(
        DRM_Prdy_Handle_t pPrdyContext,
        DRM_Prdy_license_right_e licenseRight,
        DRM_Prdy_license_state_t *pLicenseState);

/***********************************************************************************
 * Function: DRM_Prdy_SecureClock_GenerateChallenge()
 *
 * Description:
 * The DRM_Prdy_SecureClock_GenerateChallenge function creates a secure clock challenge,
 * which is sent over the Internet directly from a device or by a proxy application on
 * a computer.
 *
 * Maps to:
 *  Drm_SecureClock_GenerateChallenge
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pURL:            [out] Pointer to a NULL-terminated, user-allocated buffer to hold
 *                         the secure clock service URL. If NULL is specified, the
 *                         required buffer size is returned.
 *  pURL_len:        [in, out] On input, this parameter should point to the current
 *                             buffer size of pURL. On output, it will point to the
 *                             number of characters that were placed in pURL.
 *  pCh_data:        [out] Pointer to a user-allocated buffer to hold the secure clock
 *                         challenge. If NULL is specified, the required buffer size is
 *                         returned.
 *  pCh_len:         [in, out] On input, this parameter should point to the current buffer
 *                             size of f_pbChallenge. On output, it will point to the number
 *                             of bytes that were placed in pCh_data.
 * Returns:
 *   On Success:   DRM_Prdy_ok or DRM_Prdy_buffer_size
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_SecureClock_GenerateChallenge(
        DRM_Prdy_Handle_t      pPrdyContext,
        wchar_t               *pURL,      /* [out] */
        uint32_t              *pURL_len,  /* [in,out] */
        uint8_t               *pCh_data,  /* [out] */
        uint32_t              *pCh_len);  /* [in,out] */

/***********************************************************************************
 * Function: DRM_Prdy_SecureClock_ProcessResponse()
 *
 * Description:
 * The DRM_Prdy_SecureClock_ProcessResponse function processes the response received
 * from the clock service, verifies and stores the clock packet, and sets the clock time.
 *
 * Maps to:
 *  Drm_SecureClock_ProcessResponse
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pChResponse:     [in] Response string received from the clock service.
 *  pChResponselen:  [in] Size of f_pbResponse in bytes.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_SecureClock_ProcessResponse(
        DRM_Prdy_Handle_t      pPrdyContext,     /*[in] */
        uint8_t               *pChResponse,      /*[in] */
        uint32_t               pChResponselen);  /*[in] */

/***********************************************************************************
 * Function: DRM_Prdy_SecureClock_GetStatus()
 *
 * Description:
 * The DRM_Prdy_SecureClock_ProcessResponse function gets the state of the secure clock.
 *
 * Maps to:
 *  Drm_SecureClock_ProcessResponse
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pStatus:         [out] Pointer to flag indicating the state of the secure clock,
 *                         which can be one of the following values:
 *
 *                         DRM_PRDY_CLK_NOT_SET
 *                         DRM_PRDY_CLK_SET
 *                         DRM_PRDY_CLK_NEEDS_REFRESH
 *                         DRM_PRDY_CLK_NOT_PRESENT
 *
 *                         Please check the drm_prdy_types.h for the actual values.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_SecureClock_GetStatus(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint32_t              *pStatus );


/***********************************************************************************
 * Function: DRM_Prdy_GetSystemTime()
 *
 * Description:
 * The DRM_Prdy_GetSystemTime function gets the current system date and time.
 * The system time is expressed in Coordinated Universal Time (UTC).
 *
 * Maps to:
 *  Oem_Clock_GetSystemTime
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pYear:           [out] Pointer to the current year
 *  pMonth:          [out] Pointer to the current month; January is 1.
 *  pDayOfWeek:      [out] Pointer to the current day of the week; Sunday is 0, Monday is 1, and so on.
 *  pDay:            [out] Pointer to the current day of the month
 *  pHour:           [out] Pointer to the current hour
 *  pMinute:         [out] Pointer to the current minute
 *  pSecond:         [out] Pointer to the current second
 *  pMilliseconds:   [out] Pointer to the current milliseconds
 *
 * Returns:
 *
 * Note:
 *
 ***********************************************************************************/
void DRM_Prdy_GetSystemTime(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint16_t              *pYear,
        uint16_t              *pMonth,
        uint16_t              *pDayOfWeek,
        uint16_t              *pDay,
        uint16_t              *pHour,
        uint16_t              *pMinute,
        uint16_t              *pSecond,
        uint16_t              *pMilliseconds);

/***********************************************************************************

 !!   WANINING! WARNING! WARNING!
 !!
 !!  This function is NOT for SAGE! Calling this function for SAGE will ASSERT!
 !!
 !!  For SAGE, this function should not be used due to the Pleayready's Secure
 !!  Clock.  Application should perform the formal time challenge request/respone
 !!  in order to sync the local Playready clock with the Server. Please see the
 !!  DRM_Prdy_SecureClock_GenerateChallenge() and DRM_Prdy_SecureClock_ProcessResponse()
 !!  for details.
 !!

 *
 * Function: DRM_Prdy_SetSystemTime()
 *
 * Description:
 * The DRM_Prdy_SetSystemTime function set the current system date and time.
 *
 * Maps to:
 *  Oem_Clock_SetSystemTime
 *
 * Parameters:
 *  pPrdyContext:   [in] A valid DRM_Prdy_Context
 *  year:           [in] Pointer to the current year
 *  month:          [in] Pointer to the current month; January is 1.
 *  dayOfWeek:      [in] Pointer to the current day of the week; Sunday is 0,
 *                       Monday is 1, and so on.
 *  day:            [in] Pointer to the current day of the month
 *  hour:           [in] Pointer to the current hour
 *  minute:         [in] Pointer to the current minute
 *  second:         [in] Pointer to the current second
 *  milliseconds:   [in] Pointer to the current milliseconds
 *
 * Returns:
 *
 * Note:
 *
 ***********************************************************************************/
void DRM_Prdy_SetSystemTime(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint16_t               year,
        uint16_t               month,
        uint16_t               dayOfWeek,
        uint16_t               day,
        uint16_t               hour,
        uint16_t               minute,
        uint16_t               second,
        uint16_t               milliseconds);

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_ProcessResponse_SecStop()
 *
 * Description:
 * A DRM_Prdy_LicenseAcq_ProcessResponse function processes a license response,
 * which results from posting a license challenge.  This function has an additional
 * parameter required to support the Secure Stop feature.
 *
 * Maps to:
 *  Drm_LicenseAcq_ProcessResponse_SecStop()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pData         [out] A pointer to a buffer containing the challenge response data
 *  dataLen       [in] Size of the challenge response data buffer
 *  pSessionID    [out] A 16-byte secure stop session ID
 *  pResponse     [out] Optional parameter to the response structure
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponse_SecStop(
        DRM_Prdy_Handle_t       pPrdyContext,
        const char             *pData,
        uint32_t                dataLen,
		uint8_t					*pSessionID,
        DRM_Prdy_License_Response_t *pResponse);

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_ProcessResponse_Netflix()
 *
 * Description:
 * A DRM_Prdy_LicenseAcq_ProcessResponse function processes a license response,
 * which results from posting a license challenge.  This function has an additional
 * parameters required to support the Secure Stop feature and invokes the
 * Netflix-specific version of DRM_Prdy_LicenseAcq_ProcessResponse.
 *
 * Maps to:
 *  Drm_LicenseAcq_ProcessResponse_Netflix()
 *
 * Parameters:
 *  pPrdyContext: [in] A valid DRM_Prdy_Context
 *  pData         [out] A pointer to a buffer containing the challenge response data
 *  dataLen       [in] Size of the challenge response data buffer
 *  pSessionID    [out] A 16-byte secure stop session ID
 *  pResponse     [out] Optional parameter to the response structure
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponse_Netflix(
        DRM_Prdy_Handle_t       pPrdyContext,
        const char             *pData,
        uint32_t                dataLen,
		uint8_t					*pSessionID,
        DRM_Prdy_License_Response_t *pResponse);

/***********************************************************************************
 * Function: DRM_Prdy_Reader_Unbind()
 *
 * Description:
 * Destroy content decryption context and all associated TEE resources
 *
 * Maps to:
 *  Drm_Reader_Unbind
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  inOnOff:    	 [in] A context that may be used to decrypt data.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Unbind(
		DRM_Prdy_Handle_t 			pPrdyContext,
		DRM_Prdy_DecryptContext_t  *pDecryptContext );

/***********************************************************************************
 * Function: DRM_Prdy_SupportSecureStop()
 *
 * Description:
 * DRM_Prdy_SupportSecureStop is used to determine if the platform supports the Secure
 * Stop feature.
 *
 * Maps to:
 *  Drm_SupportSecureStop
 *
 * Returns:
 *   true if Secure Stop feature is support; false if not.
 *
 * Note:
 *
 ***********************************************************************************/
bool DRM_Prdy_SupportSecureStop( void );

/***********************************************************************************
 * Function: DRM_Prdy_TurnSecureStop()
 *
 * Description:
 * DRM_Prdy_TurnSecureStop enables/disables the Secure Stop Functionality
 * If Secure Stop is disabled, other secure stops APIs become no-op. They do not fail,
 * they just return no secure stop / state data.
 *
 * Maps to:
 *  Drm_TurnSecureStop
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  inOnOff:    	 [in] enable (TRUE) or disable (FALSE) Secure Stop
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_TurnSecureStop(
		DRM_Prdy_Handle_t pPrdyContext,
		bool inOnOff );

/***********************************************************************************
 * Function: DRM_Prdy_GetSecureStopIds()
 *
 * Description:
 * DRM_Prdy_GetSecureStopIds retrieves the secure stop IDs for all outstanding
 * (i.e. ready to be signed and sent) secure stops.
 *
 * Maps to:
 *  Drm_GetSecureStopIds
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pSessionIDs:   	 [out] An array of 16-byte secure stop IDs
 *	pCount:			 [out] The number of returned secure stop IDs
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetSecureStopIds(
		DRM_Prdy_Handle_t	pPrdyContext,
		uint8_t				pSessionIDs[DRM_PRDY_MAX_NUM_SECURE_STOPS][DRM_PRDY_SESSION_ID_LEN],
		uint32_t			*pCount );

/***********************************************************************************
 * Function: DRM_Prdy_GetSecureStop()
 *
 * Description:
 * DRM_Prdy_GetSecureStop retrieves secure stop data for a given session ID.
 *
 * Maps to:
 *  Drm_GetSecureStop
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pSessionID:    	 [in] A 16-byte secure stop session ID
 *	pSecureStopData: [out] An opaque output buffer to hold the secure stop data
 *	pSecureStopLen:	 [io] On input, the size of the secure stop data buffer
 *						  On output, the actual size of the retrieved secure stop data
 *						  If input size is too small, the required size of the buffer
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetSecureStop(
		DRM_Prdy_Handle_t 	pPrdyContext,
		uint8_t				*pSessionID,
		uint8_t				*pSecureStopData,
		uint16_t			*pSecureStopLen );

/***********************************************************************************
 * Function: DRM_Prdy_CommitSecureStop()
 *
 * Description:
 * DRM_Prdy_CommitSecureStop deletes the secure stop with the given ID.
 *
 * Maps to:
 *  Drm_CommitSecureStop
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *  pSessionID:    	 [in] A 16-byte secure stop session ID
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_CommitSecureStop(
		DRM_Prdy_Handle_t 	pPrdyContext,
		uint8_t				*pSessionID );

/***********************************************************************************
 * Function: DRM_Prdy_ResetSecureStops()
 *
 * Description:
 * DRM_Prdy_ResetSecureStops deletes all secure stops.
 * WARNING:  Use with care.  Design intent is to allow for correction of an out-of-sync
 * condition between the application and playready.
 *
 * Maps to:
 *  Drm_ResetSecureStops
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *	pCount:			 [out] The number of deleted secure stops
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ResetSecureStops(
		DRM_Prdy_Handle_t 	pPrdyContext,
		uint16_t			*pCount );

/***********************************************************************************
 * Function: DRM_Prdy_DeleteSecureStore()
 *
 * Description:
 * DRM_Prdy_DeleteSecureStore is used to delete a corrupted Secure Store file.
 *
 * Maps to:
 *  Drm_DeleteSecureStore
 *
 * Parameters:
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_DeleteSecureStore(void);

/***********************************************************************************
 * Function: DRM_Prdy_GetSecureStoreHash()
 *
 * Description:
 * DRM_Prdy_GetSecureStoreHash is used to retrieve a hash of the secure store.
 *
 * Maps to:
 *  Drm_GetSecureStoreHash
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetSecureStoreHash(
		uint8_t				*pSecureStoreHash );

/***********************************************************************************
 * Function: DRM_Prdy_DeleteKeyStore()
 *
 * Description:
 * DRM_Prdy_DeleteKeyStore can be used to delete corrupted Key Store files.
 *
 * Maps to:
 *  Drm_DeleteKeyStore
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_DeleteKeyStore(
		DRM_Prdy_Handle_t 	pPrdyContext );

/***********************************************************************************
 * Function: DRM_Prdy_GetKeyStoreHash()
 *
 * Description:
 * DRM_Prdy_GetKeyStoreHash is used to retrieve a hash of the Key Store.
 *
 * Maps to:
 *  Drm_GetKeyStoreHash
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *	pCount:			 [io] A buffer to hold the key store hash.
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetKeyStoreHash(
		DRM_Prdy_Handle_t 	pPrdyContext,
		uint8_t				*pSecureStoreHash );

/***********************************************************************************
 * Function: DRM_Prdy_Clock_GetSystemTime()
 *
 * Description:
 * DRM_Prdy_Clock_GetSystemTime is used to retrieve the current System time.
 * It is expressed in UTC as a 64-bit time_t compatible value.
 *
 * Maps to:
 *  Drm_Clock_GetSystemTime
 *
 * Parameters:
 *  pPrdyContext:    [in] A valid DRM_Prdy_Context
 *	pSystemTime:     [out] An 8-byte/64-bit buffer to hold the system time
 *
 * Returns:
 *   On Success:   DRM_Prdy_ok
 *   On Failure:   any other error code.
 *
 * Note:
 *
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Clock_GetSystemTime(
		DRM_Prdy_Handle_t 	pPrdyContext,
        uint8_t            *pSystemTime);

#ifdef __cplusplus
}
#endif

#endif /*DRM_PRDY_H__*/
