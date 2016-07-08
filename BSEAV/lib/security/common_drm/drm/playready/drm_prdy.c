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

#include "drm_prdy.h"
#include <string.h>
#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "drmmanager.h"
#include "drmbase64.h"
#include "drmmanagertypes.h"
#include "drmsoapxmlutility.h"
#include "oemcommon.h"
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "drmconstants.h"
#include "drm_data.h"

BDBG_MODULE(drm_prdy);

#define DUMP_DATA_HEX(string,data,size) {        \
   char tmp[512]= "\0";                          \
   uint32_t i=0, l=strlen(string);               \
   sprintf(tmp,"%s",string);                     \
   while( i<size && l < 512) {                   \
    sprintf(tmp+l," %02x", data[i]); ++i; l+=3;} \
   printf(tmp); printf("\n");                    \
}

#define POLICY_POOL_SIZE (5)

// ~100 KB to start * 64 (2^6) ~= 6.4 MB, don't allocate more than ~6.4 MB
#define DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE ( 64 * MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )

#define DRM_LICENSE_STORAGE_FILE "sample.hds"
#define DRM_DEFAULT_REVOCATION_LIST_FILE "revpackage.xml"

typedef struct PRDY_APP_CONTEXT
{
    DRM_APP_CONTEXT     *pDrmAppCtx;          /* drm application context */
    DRM_VOID            *pOEMContext;         /* Oem Context */
    OEM_Settings         oemSettings;
    DRM_BYTE            *pbOpaqueBuffer;      /* Opaque buffer */
    DRM_DWORD            cbOpaqueBuffer;
    DRM_BYTE            *pbRevocationBuffer;
    DRM_Prdy_policy_t    protectionPolicy[POLICY_POOL_SIZE]; /* Some license have more than 1 policy to enforce before playing content. This array is
                                                                used to store them up when the bdrm policy callback fires. */
    uint32_t nbOfPolicyQueued;
} PRDY_APP_CONTEXT;

/*
typedef struct PRDY_DECRYPTOR_CONTEXT
{
    DRM_DECRYPT_CONTEXT *pDecryptor;

} PRDY_DECRYPTOR_CONTEXT;
*/

typedef struct DRM_PRDY_ND_TRANSMITTER_CONTEXT
{
    DRM_PRND_TRANSMITTER_CONTEXT  *pPrndTxContext;

} DRM_PRDY_ND_TRANSMITTER_CONTEXT;

typedef struct DRM_PRDY_ND_RECEIVER_CONTEXT
{
    DRM_PRND_RECEIVER_CONTEXT *pPrndRxContext;

} DRM_PRDY_ND_RECEIVER_CONTEXT;

typedef enum
{
    eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_INVALID             = 0,
    eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_CLEAR_DATA_SOURCE   = 1,    /* Input to encrypt (clear source about to be
                                                                           encrypted) */
    eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_ENCRYPTED_DATA      = 2,    /* Output from encrypt (clear source after
                                                                           encryption -> encrypted) OR Input to decrypt
                                                                           (encrypted source about to be decrypted) */
    eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_CLEAR_DATA_RENDER   = 3,    /* Output from decrypt (encrypted source after
                                                                           decryption -> clear) */
} DRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE;


typedef struct DRM_OPAQUE_BUFFER_HANDLE_INTERNAL
{
    DRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE   eType;
    DRM_BYTE                                *pbData;
    DRM_DWORD                                cbData;
} DRM_OPAQUE_BUFFER_HANDLE_INTERNAL;


static DRM_CONST_STRING   sDstrHDSPath = EMPTY_DRM_STRING;
static DRM_WCHAR          sRgwchHDSPath[ DRM_MAX_PATH ];

/***********************************************************************************
 * Internal function to convert PlayReady SDK result into DRM_Prdy_Error_e.
 *
 * **FixMe** doesn't yet cover the complete list of DRM_Prdy_Error_e errors.
 ***********************************************************************************/
static
DRM_Prdy_Error_e convertDrmResult(DRM_RESULT result)
{
    switch (result) {
        case DRM_SUCCESS:
            return DRM_Prdy_ok;
        case DRM_E_FAIL:
            return DRM_Prdy_fail;
        case DRM_E_BUFFERTOOSMALL:
            return DRM_Prdy_buffer_size;
        case DRM_E_INVALIDARG:
            return DRM_Prdy_invalid_parameter;
        default:
            return DRM_Prdy_fail;
    }
}

static
bool convertCStringToWString( char * pCStr, DRM_WCHAR * pWStr, DRM_DWORD * cchStr)
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

        /* Make sure tmpWChar is NULL terminated */
        BKNI_Memset(tmpWChar, 0, (DRM_MAX_PATH * SIZEOF(DRM_WCHAR)));

        tmpSubStr.m_ich = 0;
        tmpSubStr.m_cch = strlen( (char*)tmpCStr );

        /* Convert the DRM_CHAR * to DRM_STRING. */
        tmpWStr.pwszString = tmpWChar;
        tmpWStr.cchString  = DRM_MAX_PATH;
        DRM_UTL_PromoteASCIItoUNICODE( tmpCStr,
                                       &tmpSubStr,
                                       &tmpWStr);

        BKNI_Memcpy(pWStr, tmpWStr.pwszString, (tmpWStr.cchString+1) * SIZEOF (DRM_WCHAR));
        *cchStr = tmpWStr.cchString;
        pWStr[tmpWStr.cchString] = g_wchNull;
        result = true;
    }

ErrorExit:
    return result;
}

static
bool convertWStringToCString( DRM_WCHAR * pWStr, char * pCStr, DRM_DWORD cchStr)
{
    if(( pCStr != NULL) && (pWStr != NULL))
    {
        DRM_UTL_DemoteUNICODEtoASCII( pWStr,
                                      pCStr,
                                      cchStr);

        return true;
    }

    return false;
}

/* Following OEM_Opaque static functions may be required for
 * Drm_LocalLicense_EncryptOpaqueSample() in the future.
 * They are copied from SDK: oemhaloemimpl.c.
 * Since they are not being used at the moment, commment them out */

#if 0
static
bool OEM_OpaqueBufferCreate( OEM_OPAQUE_BUFFER_HANDLE    *f_phBuf )
{
    DRM_RESULT                           dr     = DRM_SUCCESS;
    DRM_OPAQUE_BUFFER_HANDLE_INTERNAL   *pBuf   = NULL;

    ChkPtr( f_phBuf );
    *f_phBuf = OEM_OPAQUE_BUFFER_HANDLE_INVALID;

    ChkMem( pBuf = (DRM_OPAQUE_BUFFER_HANDLE_INTERNAL*)Oem_MemAlloc( SIZEOF(DRM_OPAQUE_BUFFER_HANDLE_INTERNAL) ) );
    OEM_SECURE_ZERO_MEMORY( pBuf, SIZEOF(DRM_OPAQUE_BUFFER_HANDLE_INTERNAL) );

    BDBG_ASSERT( pBuf->eType == eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_INVALID );

    *f_phBuf = (OEM_OPAQUE_BUFFER_HANDLE)pBuf;
    pBuf     = NULL;

    SAFE_OEM_FREE( pBuf );
    return true;

ErrorExit:
    SAFE_OEM_FREE( pBuf );
    BDBG_ERR(("%s: OEM_OpaqueBufferCreate failed [0x%X], exiting...", __FUNCTION__,dr));
    return false;
}

static
bool OEM_OpaqueBufferCreateWithData(
    const DRM_BYTE                         *f_pbData,
    DRM_DWORD                               f_cbData,
    DRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE  f_type,
    OEM_OPAQUE_BUFFER_HANDLE               *f_phBuf )
{
    DRM_RESULT                           dr     = DRM_SUCCESS;
    DRM_OPAQUE_BUFFER_HANDLE_INTERNAL   *pBuf   = NULL;
    /*DRM_BYTE                            *pbData = NULL;*/

    ChkPtr( f_pbData );
    ChkArg( f_cbData != 0    );
    ChkPtr( f_phBuf );
    *f_phBuf = OEM_OPAQUE_BUFFER_HANDLE_INVALID;

    ChkMem( pBuf = (DRM_OPAQUE_BUFFER_HANDLE_INTERNAL*)Oem_MemAlloc( SIZEOF(DRM_OPAQUE_BUFFER_HANDLE_INTERNAL) ) );
    OEM_SECURE_ZERO_MEMORY( pBuf, SIZEOF(DRM_OPAQUE_BUFFER_HANDLE_INTERNAL) );

    /*ChkMem( pbData = (DRM_BYTE*)Oem_MemAlloc( f_cbData ) );
      OEM_SECURE_MEMCPY( pbData, f_pbData, f_cbData ); */

    pBuf->cbData = f_cbData;
    pBuf->eType  = f_type;
    pBuf->pbData = (DRM_BYTE *)f_pbData;
    /*pBuf->pbData = (DRM_BYTE *)pbData;*/
    /*pbData       = NULL;*/
    *f_phBuf = (OEM_OPAQUE_BUFFER_HANDLE)pBuf;
    pBuf     = NULL;

    SAFE_OEM_FREE( pBuf );
    /*SAFE_OEM_FREE( pbData );*/
    return true;

ErrorExit:
    SAFE_OEM_FREE( pBuf );
    /*SAFE_OEM_FREE( pbData );*/
    BDBG_ERR(("%s: OEM_OpaqueBufferCreateWithData failed [0x%X], exiting...\n", __FUNCTION__,dr));
    return false;
}


static
bool OEM_OpaqueBufferDestroy( OEM_OPAQUE_BUFFER_HANDLE   *f_phBuf )
{
    if( f_phBuf != NULL )
    {
        DRM_OPAQUE_BUFFER_HANDLE_INTERNAL *pBuf = (DRM_OPAQUE_BUFFER_HANDLE_INTERNAL*)*f_phBuf;
        if( *f_phBuf != OEM_OPAQUE_BUFFER_HANDLE_INVALID && pBuf != NULL )
        {
            /*SAFE_OEM_FREE( pBuf->pbData );*/
            pBuf->pbData = NULL; /* we don't free the pbData for the application */
            SAFE_OEM_FREE( pBuf );
        }
        *f_phBuf = OEM_OPAQUE_BUFFER_HANDLE_INVALID;
    }
    return true;
}
#endif


static
bool load_revocation_list(
        PRDY_APP_CONTEXT    *pPrdyCxt,
        char                *revListFile)
{
    DRM_RESULT dr = DRM_SUCCESS;
    FILE    * fRev;
    uint8_t * revBuf = NULL;
    size_t    fileSize = 0;
    uint32_t  currSize = 0;

    BDBG_ASSERT(pPrdyCxt != NULL);
    BDBG_ASSERT(revListFile != NULL);

    fRev = fopen(revListFile, "rb");
    if( fRev == NULL)
    {
        BDBG_WRN(("[WARNING] %s %d: Failed to open %s \n",__FUNCTION__,__LINE__,revListFile));
        return true;
    }

    /* get the size of the file */
    fseek(fRev, 0, SEEK_END);
    fileSize = ftell(fRev);
    fseek(fRev, 0, SEEK_SET);

    revBuf = BKNI_Malloc(fileSize);
    if( revBuf == NULL)
    {
        BDBG_ERR(("%s: Failed to allocate memory.\n",__FUNCTION__));
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
                pPrdyCxt->pDrmAppCtx,
                ( DRM_CHAR * )revBuf,
                fileSize ) );

    BDBG_MSG(("Drm_Revocation_StorePackage succeeded.\n"));

    if( revBuf != NULL)
        BKNI_Free(revBuf);

    return true;

ErrorExit:
    if( revBuf != NULL)
        BKNI_Free(revBuf);

    BDBG_ERR(("Revocation Store Package failed [0x%X]\n",(unsigned int)dr));
    return false;
}


/* internal call back function for Drm_Reader_Bind */
static
DRM_RESULT DRM_API DRM_Prdy_policy_callback(
    __in const DRM_VOID  *f_pvCallbackData,
    __in       DRM_DWORD  f_dwCallbackType,
    __in const DRM_VOID  *f_pv )
{
    PRDY_APP_CONTEXT  *pPrdyContext = (PRDY_APP_CONTEXT  *)f_pv;
    DRM_RESULT rc = DRM_SUCCESS;
    BDBG_MSG(("%s - entering", __FUNCTION__));
    DRM_Prdy_policy_t *policy = NULL;
    DRM_PLAY_OPL_EX2 *pPolicyData=NULL;

    BDBG_ASSERT(f_pvCallbackData != NULL);
    BDBG_ASSERT(f_pv != NULL);

    if(pPrdyContext->nbOfPolicyQueued < POLICY_POOL_SIZE){
         policy = &pPrdyContext->protectionPolicy[pPrdyContext->nbOfPolicyQueued];
         pPrdyContext->nbOfPolicyQueued++;
    }
    else {
        BDBG_ERR(("Policy lost. Policy Queue is full"));
        return DRM_E_RIGHTS_NOT_AVAILABLE;
    }


    policy->type = f_dwCallbackType;

    switch(policy->type){
        case PLAY_OPL:
            pPolicyData = (DRM_PLAY_OPL_EX2 *)f_pvCallbackData;
            policy->t.play.dwVersion = pPolicyData->dwVersion;
            BKNI_Memcpy(&policy->t.play.minOPL, &pPolicyData->minOPL, sizeof(DRM_Prdy_Minimum_Output_Protection_Levels_t));

            BKNI_Memcpy(&policy->t.play.oplIdReserved, &pPolicyData->oplIdReserved, sizeof(DRM_Prdy_Opl_Output_Ids_t));
            if(pPolicyData->oplIdReserved.cIds != 0)
                BKNI_Memcpy(policy->t.play.oplIdReserved.rgIds, &pPolicyData->oplIdReserved.rgIds, sizeof(DRM_Prdy_guid_t) * pPolicyData->oplIdReserved.cIds);

            BKNI_Memcpy(&policy->t.play.vopi, &pPolicyData->vopi, sizeof(DRM_Prdy_video_out_protection_ids_ex_t));
            BKNI_Memcpy(&policy->t.play.aopi, &pPolicyData->aopi, sizeof(DRM_Prdy_audio_out_protection_ids_ex_t));
#if 0 /* Depracated */
            policy->t.play.i_compressedDigitalVideo   =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wCompressedDigitalVideo;
            policy->t.play.i_uncompressedDigitalVideo =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wUncompressedDigitalVideo;
            policy->t.play.i_analogVideo              =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wAnalogVideo;
            policy->t.play.i_compressedDigitalAudio   =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wCompressedDigitalAudio;
            policy->t.play.i_uncompressedDigitalAudio =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wUncompressedDigitalAudio;

            policy->t.play.i_resv_cIds  =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->oplIdReserved.cIds;
            policy->t.play.i_resv_rgIds =
                (DRM_Prdy_guid_t *)((DRM_PLAY_OPL *)f_pvCallbackData)->oplIdReserved.rgIds;

            policy->t.play.i_vop_cEntries  =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->vopi.cEntries;
            policy->t.play.i_vop           =
                (DRM_Prdy_video_out_protection_t *)((DRM_PLAY_OPL *)f_pvCallbackData)->vopi.rgVop;
#endif
            break;

        case COPY_OPL:
            policy->t.copy.i_minimumCopyLevel =
                (uint16_t)((DRM_COPY_OPL *)f_pvCallbackData)->wMinimumCopyLevel;
            policy->t.copy.i_includes_cIds    =
                (uint16_t)((DRM_COPY_OPL *)f_pvCallbackData)->oplIdIncludes.cIds;
            policy->t.copy.i_includes_rgIds   =
                (DRM_Prdy_guid_t *)((DRM_COPY_OPL *)f_pvCallbackData)->oplIdIncludes.rgIds;
            policy->t.copy.i_excludes_cIds    =
                (uint16_t)((DRM_COPY_OPL *)f_pvCallbackData)->oplIdExcludes.cIds;
            policy->t.copy.i_excludes_rgIds   =
                (DRM_Prdy_guid_t *)((DRM_COPY_OPL *)f_pvCallbackData)->oplIdExcludes.rgIds;
            break;
        case INCLUSION_LIST:
            BKNI_Memcpy(&policy->t.inc_list, f_pvCallbackData, sizeof(DRM_INCLUSION_LIST_CALLBACK_STRUCT));
            break;
        case EXTENDED_RESTRICTION_CONDITION:
            BKNI_Memcpy(&policy->t.condition, f_pvCallbackData, sizeof(DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT));
            break;
        case EXTENDED_RESTRICTION_ACTION:
            BKNI_Memcpy(&policy->t.action, f_pvCallbackData, sizeof(DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT));
            break;
        case EXTENDED_RESTRICTION_QUERY:
            BKNI_Memcpy(&policy->t.query, f_pvCallbackData, sizeof(DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT));
            break;
        case SECURE_STATE_TOKEN_RESOLVE:
            BKNI_Memcpy(&policy->t.token_res, f_pvCallbackData, sizeof(DRM_SECURE_STATE_TOKEN_RESOLVE_DATA));
            break;
        case RESTRICTED_SOURCEID:
            BKNI_Memcpy(&policy->t.restr_src, f_pvCallbackData, sizeof(DRM_RESTRICTED_SOURCEID_CALLBACK_STRUCT));
            break;
        default:
            /* Unsupported operation */
            rc = DRM_E_RIGHTS_NOT_AVAILABLE;
            break;
    }

    BDBG_MSG(("%s: Exiting, rc %d.\n", __FUNCTION__, rc));
    return rc;
}

void* DRM_Prdy_GetDrmAppContext(DRM_Prdy_Handle_t handle)
{
    PRDY_APP_CONTEXT  *pPrdyCxt = (PRDY_APP_CONTEXT*)handle;
    if(pPrdyCxt == NULL) {
        BDBG_ERR(("%s - Given handle is null\n", __FUNCTION__));
        return NULL;
    }

    return (void *)pPrdyCxt->pDrmAppCtx;
}

void DRM_Prdy_GetDefaultParamSettings( DRM_Prdy_Init_t *pPrdyParamSettings)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));
    BDBG_ASSERT(pPrdyParamSettings != NULL);

    BKNI_Memset(pPrdyParamSettings, 0x00, sizeof(DRM_Prdy_Init_t));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
}

DRM_Prdy_Handle_t  DRM_Prdy_Initialize(DRM_Prdy_Init_t * pInitSetting)
{
    DRM_RESULT         dr = DRM_SUCCESS;
    PRDY_APP_CONTEXT  *pPrdyCxt = NULL;
    NEXUS_MemoryAllocationSettings heapSettings;
    NEXUS_ClientConfiguration platformConfig;
    char *             revFile = NULL;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    /* initialize the PRDY_APP_CONTEXT */
    pPrdyCxt = Oem_MemAlloc(sizeof(PRDY_APP_CONTEXT));
    if( pPrdyCxt == NULL ) {
        BDBG_ERR(("%s: Prdy Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }
    BKNI_Memset(pPrdyCxt, 0, sizeof(PRDY_APP_CONTEXT));

    pPrdyCxt->oemSettings.binFileName = NULL;
    pPrdyCxt->oemSettings.keyFileName = NULL;
    pPrdyCxt->oemSettings.keyHistoryFileName = NULL;
    pPrdyCxt->oemSettings.defaultRWDirName = NULL;

    /* initialize the DRM_APP_CONTEXT */
    pPrdyCxt->pDrmAppCtx = Oem_MemAlloc(SIZEOF(DRM_APP_CONTEXT));
    ChkMem(pPrdyCxt->pDrmAppCtx);
    ZEROMEM( ( uint8_t * )pPrdyCxt->pDrmAppCtx, SIZEOF( DRM_APP_CONTEXT));

    /* copy the binFileName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->binFileName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.binFileName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        if( !convertCStringToWString(pInitSetting->binFileName, pPrdyCxt->oemSettings.binFileName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.binFileName);
        }
    }

    /* copy the keyFileName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->keyFileName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.keyFileName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        if( !convertCStringToWString(pInitSetting->keyFileName, pPrdyCxt->oemSettings.keyFileName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.keyFileName);
        }
    }

    /* copy the keyHistoryFileName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->keyHistoryFileName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.keyHistoryFileName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        if( !convertCStringToWString(pInitSetting->keyHistoryFileName, pPrdyCxt->oemSettings.keyHistoryFileName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.keyHistoryFileName);
        }
    }

    /* copy the defaultRWDirName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->defaultRWDirName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.defaultRWDirName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        if( !convertCStringToWString(pInitSetting->defaultRWDirName, pPrdyCxt->oemSettings.defaultRWDirName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.defaultRWDirName);
        }
    }

    /* Drm_Platform_Initialize */
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

    pPrdyCxt->oemSettings.heap = heapSettings.heap;
    pPrdyCxt->pOEMContext = Drm_Platform_Initialize(&pPrdyCxt->oemSettings);
    ChkMem(pPrdyCxt->pOEMContext);

    /* Initialize OpaqueBuffer and RevocationBuffer */
    pPrdyCxt->cbOpaqueBuffer = MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE;
    ChkMem( pPrdyCxt->pbOpaqueBuffer = ( uint8_t * )Oem_MemAlloc(MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE));
    ChkMem( pPrdyCxt->pbRevocationBuffer = ( uint8_t * )Oem_MemAlloc( REVOCATION_BUFFER_SIZE));

    /* Drm_Initialize */
    sDstrHDSPath.pwszString = sRgwchHDSPath;
    sDstrHDSPath.cchString = DRM_MAX_PATH;

    /* Convert the HDS path to DRM_STRING. */
    if((pInitSetting != NULL) && (pInitSetting->hdsFileName != NULL))
    {
        if( !convertCStringToWString(pInitSetting->hdsFileName, (DRM_WCHAR*)sDstrHDSPath.pwszString, &sDstrHDSPath.cchString))
        {
            goto ErrorExit;
        }
    }
    else
    {
        DRM_WCHAR *hdsDir = bdrm_get_hds_dir();
        DRM_WCHAR *hdsFname = bdrm_get_hds_fname();
        if (hdsFname != NULL && bdrm_get_hds_fname_lgth() > 0) {
            if (bdrm_get_hds_dir_lgth() > 0)
                BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString, hdsDir, bdrm_get_hds_dir_lgth() * sizeof(DRM_WCHAR));
            BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString + bdrm_get_hds_dir_lgth(),
                hdsFname, (bdrm_get_hds_fname_lgth() + 1) * sizeof(DRM_WCHAR));
        }
        else if( !convertCStringToWString(DRM_LICENSE_STORAGE_FILE, (DRM_WCHAR*)sDstrHDSPath.pwszString, &sDstrHDSPath.cchString))
        {
            goto ErrorExit;
        }
    }

   /* TODO: perform synchronous activation if Drm_Initialize fails */
   ChkDR( Drm_Initialize( pPrdyCxt->pDrmAppCtx,
                          pPrdyCxt->pOEMContext,
                          pPrdyCxt->pbOpaqueBuffer,
                          pPrdyCxt->cbOpaqueBuffer,
                          &sDstrHDSPath) );

   ChkDR( Drm_Revocation_SetBuffer( pPrdyCxt->pDrmAppCtx,
                                    pPrdyCxt->pbRevocationBuffer,
                                    REVOCATION_BUFFER_SIZE ) );

   /* load the revocation list */

   if( (revFile = pInitSetting->revocationListFileName) == NULL)
   {
       revFile = DRM_DEFAULT_REVOCATION_LIST_FILE;
   }

   if( !load_revocation_list( pPrdyCxt,revFile))
   {
       goto ErrorExit;
   }

#ifdef CMD_DRM_PLAYREADY_SAGE_IMPL
   {
    DRMSYSTEMTIME     systemTime;
    /* initialize the systemtime */
    DRM_APP_CONTEXT_INTERNAL    *poAppContextInternal = ( DRM_APP_CONTEXT_INTERNAL * )pPrdyCxt->pDrmAppCtx;
    Oem_Clock_GetSystemTime( pPrdyCxt->pOEMContext, &systemTime);
    Oem_Clock_SetSystemTime( &poAppContextInternal->oBlackBoxContext, &systemTime);
   }
#endif
   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   return (DRM_Prdy_Handle_t ) pPrdyCxt;

ErrorExit:
   BDBG_ERR(("%s: Exiting with error [0x%X].\n", __FUNCTION__,(unsigned int)dr));
   if( pPrdyCxt != NULL)
       DRM_Prdy_Uninitialize( pPrdyCxt);
   return NULL;
}

DRM_Prdy_Error_e DRM_Prdy_Uninitialize(DRM_Prdy_Handle_t  pPrdyContext)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    if( pPrdyContext->pDrmAppCtx ) {
        Drm_Uninitialize( pPrdyContext->pDrmAppCtx );
        SAFE_OEM_FREE( pPrdyContext->pDrmAppCtx );
    }
    SAFE_OEM_FREE(pPrdyContext->pbOpaqueBuffer);
    SAFE_OEM_FREE(pPrdyContext->pbRevocationBuffer);

    Drm_Platform_Uninitialize(pPrdyContext->pOEMContext);

    if(pPrdyContext->oemSettings.binFileName != NULL) SAFE_OEM_FREE(pPrdyContext->oemSettings.binFileName);
    if(pPrdyContext->oemSettings.keyFileName != NULL) SAFE_OEM_FREE(pPrdyContext->oemSettings.keyFileName);
    if(pPrdyContext->oemSettings.keyHistoryFileName != NULL) SAFE_OEM_FREE(pPrdyContext->oemSettings.keyHistoryFileName);
    if(pPrdyContext->oemSettings.defaultRWDirName != NULL) SAFE_OEM_FREE(pPrdyContext->oemSettings.defaultRWDirName);

    SAFE_OEM_FREE(pPrdyContext);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;
}

DRM_Prdy_Error_e DRM_Prdy_Reinitialize(DRM_Prdy_Handle_t  pPrdyContext)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_Prdy_Error_e  result = DRM_Prdy_fail;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    if( pPrdyContext->pDrmAppCtx ) {
        dr = Drm_Reinitialize( pPrdyContext->pDrmAppCtx );
        if (dr == DRM_SUCCESS) {
            result = DRM_Prdy_ok;
        }
    }
    else {
        result = DRM_Prdy_invalid_parameter;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;
}

DRM_Prdy_Error_e DRM_Prdy_Get_Buffer_Size(
        DRM_Prdy_Handle_t            pPrdyContext,
        DRM_Prdy_GetBuffer_Type_e    chType,
        const uint8_t               *pData,
        uint32_t                     dataLen,
        uint32_t                    *pData1_size,
        uint32_t                    *pData2_size)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_Prdy_Error_e  result = DRM_Prdy_ok;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    switch(chType) {
        case DRM_Prdy_getBuffer_content_property_header_kid:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_HEADER_KID,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_header_type:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_HEADER_TYPE,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_header:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_HEADER,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_playready_obj:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_PLAYREADY_OBJ,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_cipher_type:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_CIPHER_TYPE,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_decryptor_setup:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_DECRYPTORSETUP,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_licenseAcq_challenge:
        {
            //DRM_DWORD cbChallenge = 0;
            //DRM_DWORD cchURL = 0;
            DRM_CHAR *pszCustomDataUsed = NULL;
            DRM_DWORD cchCustomDataUsed = 0;
            const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };
            if ((NULL != pData) && ( dataLen > 0))
            {
                pszCustomDataUsed = (DRM_CHAR *) pData;
                cchCustomDataUsed = (DRM_DWORD) dataLen;
            }

            dr = Drm_LicenseAcq_GenerateChallenge(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
                    NULL,
                    pszCustomDataUsed,
                    cchCustomDataUsed,
                    NULL,
                    pData1_size,
                    NULL,
                    NULL,
                    NULL,
                    pData2_size);

            if ( dr != DRM_E_BUFFERTOOSMALL ) {
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_client_info:
        {
            DRM_DWORD  cch_ci = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Device_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_DGP_CLIENT_INFO,
                    NULL,
                    &cch_ci);

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cch_ci;
            }
            else {
                BDBG_ERR(("%s: Drm_Device_GetProperty [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_Additional_Response_Data_Custom_Data:
        {
            DRM_DWORD  cchData = 0;
            BDBG_ASSERT(pData1_size != NULL);

            dr = Drm_GetAdditionalResponseData(pPrdyContext->pDrmAppCtx,
                            pData,
                            dataLen,
                            DRM_GARD_CUSTOM_DATA,
                            NULL,
                            &cchData );
            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cchData;
            }
            else if ( dr == DRM_E_XMLNOTFOUND ) {
                result = DRM_Prdy_xml_not_found;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_Additional_Response_Data_Redirect_Url:
        {
            DRM_DWORD  cchData = 0;
            BDBG_ASSERT(pData1_size != NULL);

            dr = Drm_GetAdditionalResponseData(pPrdyContext->pDrmAppCtx,
                            pData,
                            dataLen,
                            DRM_GARD_REDIRECT_URL,
                            NULL,
                            &cchData );
            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cchData;
            }
            else if ( dr == DRM_E_XMLNOTFOUND ) {
                result = DRM_Prdy_xml_not_found;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_Additional_Response_Data_Service_Id:
        {
            DRM_DWORD  cchData = 0;
            BDBG_ASSERT(pData1_size != NULL);

            dr = Drm_GetAdditionalResponseData(pPrdyContext->pDrmAppCtx,
                            pData,
                            dataLen,
                            DRM_GARD_SERVICE_ID,
                            NULL,
                            &cchData );
            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cchData;
            }
            else if ( dr == DRM_E_XMLNOTFOUND ) {
                result = DRM_Prdy_xml_not_found;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_Additional_Response_Data_Account_Id:
        {
            DRM_DWORD  cchData = 0;
            BDBG_ASSERT(pData1_size != NULL);

            dr = Drm_GetAdditionalResponseData(pPrdyContext->pDrmAppCtx,
                            pData,
                            dataLen,
                            DRM_GARD_ACCOUNT_ID,
                            NULL,
                            &cchData );
            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cchData;
            }
            else if ( dr == DRM_E_XMLNOTFOUND ) {
                result = DRM_Prdy_xml_not_found;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_licenseAcq_ack_challenge:
        {
            dr = Drm_LicenseAcq_GenerateAck(
                pPrdyContext->pDrmAppCtx,
                (DRM_LICENSE_RESPONSE *)pData,
                NULL,
                pData1_size);
            if ( dr != DRM_E_BUFFERTOOSMALL ) {
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_licenseAcq_challenge_Netflix:
        {
            //DRM_DWORD cbChallenge = 0;
            //DRM_DWORD cchURL = 0;
            DRM_CHAR *pszCustomDataUsed = NULL;
            DRM_DWORD cchCustomDataUsed = 0;
            DRM_BYTE tmp_nonce[DRM_PRDY_SESSION_ID_LEN];
            const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };
            if ((NULL != pData) && ( dataLen > 0))
            {
                pszCustomDataUsed = (DRM_CHAR *) pData;
                cchCustomDataUsed = (DRM_DWORD) dataLen;
            }

            dr = Drm_LicenseAcq_GenerateChallenge_Netflix(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
                    NULL,
                    pszCustomDataUsed,
                    cchCustomDataUsed,
                    NULL,
                    pData1_size,
                    NULL,
                    NULL,
                    NULL,
                    pData2_size,
                    tmp_nonce,
                    false);

            if ( dr != DRM_E_BUFFERTOOSMALL ) {
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_licenseAcq_challenge_NetflixWithLDL:
        {
            //DRM_DWORD cbChallenge = 0;
            //DRM_DWORD cchURL = 0;
            DRM_CHAR *pszCustomDataUsed = NULL;
            DRM_DWORD cchCustomDataUsed = 0;
            DRM_BYTE tmp_nonce[DRM_PRDY_SESSION_ID_LEN];
            const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };
            if ((NULL != pData) && ( dataLen > 0))
            {
                pszCustomDataUsed = (DRM_CHAR *) pData;
                cchCustomDataUsed = (DRM_DWORD) dataLen;
            }

            dr = Drm_LicenseAcq_GenerateChallenge_Netflix(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
                    NULL,
                    pszCustomDataUsed,
                    cchCustomDataUsed,
                    NULL,
                    pData1_size,
                    NULL,
                    NULL,
                    NULL,
                    pData2_size,
                    tmp_nonce,
                    true);

            if ( dr != DRM_E_BUFFERTOOSMALL ) {
                result = DRM_Prdy_fail;
            }
        }
        break;

        default:
            BDBG_ERR(("%s: Unknown buffer type %d \n",__FUNCTION__,chType));
            result = DRM_Prdy_fail;

    } /* end switch */

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;
}

DRM_Prdy_Error_e DRM_Prdy_Content_GetProperty(
        DRM_Prdy_Handle_t                pPrdyContext,
        DRM_Prdy_ContentGetProperty_e    propertyType,
        uint8_t                         *pData,
        uint32_t                         dataSize)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_Prdy_Error_e  result = DRM_Prdy_ok;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_Content_GetProperty(
            pPrdyContext->pDrmAppCtx,
            (DRM_CONTENT_GET_PROPERTY) propertyType,
            (DRM_BYTE *)pData,
            (DRM_DWORD *) &dataSize);

    if(dr != DRM_SUCCESS){
        if(dr == DRM_E_CH_INVALID_HEADER) {
            result = DRM_Prdy_invalid_header;
        }
        else if (dr == DRM_E_XMLNOTFOUND) {
            result = DRM_Prdy_xml_not_found;
        }
        else {
            result = DRM_Prdy_fail;
        }
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;
}

DRM_Prdy_Error_e DRM_Prdy_Content_SetProperty(
        DRM_Prdy_Handle_t                pPrdyContext,
        DRM_Prdy_ContentSetProperty_e    propertyType,
        const uint8_t                   *pData,
        uint32_t                         dataSize)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_Prdy_Error_e  result = DRM_Prdy_ok;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    dr = Drm_Content_SetProperty(
            pPrdyContext->pDrmAppCtx,
            (DRM_CONTENT_SET_PROPERTY) propertyType,
            (const DRM_BYTE *)pData,
            (DRM_DWORD)dataSize);

    if(dr == DRM_E_HEADER_ALREADY_SET)
    {
        Drm_Reinitialize(pPrdyContext->pDrmAppCtx);
        dr =Drm_Content_SetProperty( pPrdyContext->pDrmAppCtx,
                                     (DRM_CONTENT_SET_PROPERTY) propertyType,
                                     (const DRM_BYTE *)pData,
                                     (DRM_DWORD)dataSize);
    }

    if( dr != DRM_SUCCESS)
    {
        result = DRM_Prdy_fail;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;
}

DRM_Prdy_Error_e DRM_Prdy_Get_Protection_Policy(
        DRM_Prdy_Handle_t      pPrdyContext,
        DRM_Prdy_policy_t     *pPolicy)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));
    BDBG_ASSERT(pPrdyContext != NULL);
    DRM_Prdy_Error_e dr = DRM_Prdy_ok;

    if( pPolicy == NULL)
    {
        BDBG_ERR(("%s: pPolicy is NULL, xxiting\n", __FUNCTION__));
        return DRM_Prdy_fail;
    };

    if(pPrdyContext->nbOfPolicyQueued == 0) {
        BDBG_MSG(("%s: no policy found\n", __FUNCTION__));
        dr = DRM_Prdy_no_policy;
    }
    else {
        BDBG_MSG(("%s: policy found\n", __FUNCTION__));
        pPrdyContext->nbOfPolicyQueued--;
        BKNI_Memcpy(pPolicy, &pPrdyContext->protectionPolicy[pPrdyContext->nbOfPolicyQueued], sizeof(DRM_Prdy_policy_t));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return dr;
}

void DRM_Prdy_GetDefaultDecryptSettings(
        DRM_Prdy_DecryptSettings_t  *pSettings
        )
{
    if( pSettings != NULL)
    {
        BKNI_Memset(pSettings, 0, sizeof(DRM_Prdy_DecryptSettings_t));
        pSettings->opType = NEXUS_SecurityOperation_eDecrypt;
        pSettings->algType = NEXUS_SecurityAlgorithm_eAes128;
        pSettings->algVariant = NEXUS_SecurityAlgorithmVariant_eCounter;
        pSettings->termMode = NEXUS_SecurityTerminationMode_eClear;
        pSettings->enableExtKey = true;
        pSettings->enableExtIv = true;
        pSettings->aesCounterSize = NEXUS_SecurityAesCounterSize_e64Bits;
        pSettings->keySlotType = NEXUS_SecurityKeyType_eOdd;
        pSettings->aesCounterMode = NEXUS_SecurityCounterMode_ePartialBlockInNextPacket;
    }
}

static DRM_Prdy_Error_e Drm_Prdy_AllocateDecryptContextKeySlot (
   DRM_Prdy_DecryptContextKey_t  **pDecryptContextKey
   )
{
    NEXUS_Error                    rc = NEXUS_SUCCESS;
    DRM_RESULT                     dr = DRM_SUCCESS;
    NEXUS_SecurityKeySlotSettings  keySlotSettings;
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_ClientConfiguration      platformConfig;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    NEXUS_Platform_GetClientConfiguration(&platformConfig);

    if (platformConfig.heap[NXCLIENT_FULL_HEAP])
    {
        NEXUS_HeapHandle heap = platformConfig.heap[NXCLIENT_FULL_HEAP];
        NEXUS_MemoryStatus heapStatus;
        NEXUS_Heap_GetStatus(heap, &heapStatus);
        if (heapStatus.memoryType & NEXUS_MemoryType_eFull)
        {
            allocSettings.heap = heap;
        }
    }

    rc = NEXUS_Memory_Allocate(sizeof(DRM_Prdy_DecryptContextKey_t), &allocSettings, (void *)(pDecryptContextKey));
    if (rc)
    {
        BDBG_ERR(("%s: Failed to allocate memory %d, exiting...\n", __FUNCTION__,rc));
        dr = DRM_Prdy_fail;
        goto ErrorExit;
    }
    BKNI_Memset((*pDecryptContextKey), 0, sizeof(DRM_Prdy_DecryptContextKey_t));

    /* Allocate key slot */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    (*pDecryptContextKey)->keySlot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if((*pDecryptContextKey)->keySlot == NULL)
    {
        BDBG_ERR(("%s: Failed to allocate Key Slot, exiting...\n", __FUNCTION__));
        dr = DRM_Prdy_fail;
        goto ErrorExit;
    }

    (*pDecryptContextKey)->refCount = 1;

ErrorExit:
    return dr;
}

static void Drm_Prdy_FreeDecryptContextKeySlot (
    DRM_Prdy_DecryptContextKey_t  *pDecryptContextKey
    )
{
    if(pDecryptContextKey)
    {
        if(pDecryptContextKey->refCount > 0) pDecryptContextKey->refCount--;

        if(pDecryptContextKey->refCount == 0){
            NEXUS_Security_FreeKeySlot(pDecryptContextKey->keySlot);
            NEXUS_Memory_Free(pDecryptContextKey);
        }
    }
    return;
}


DRM_Prdy_DecryptContext_t * DRM_Prdy_AllocateDecryptContext (
    const DRM_Prdy_DecryptSettings_t  *pSettings
    )
{
    NEXUS_Error                    rc = NEXUS_SUCCESS;
    DRM_RESULT                     dr = DRM_SUCCESS;
    DRM_Prdy_DecryptContext_t     *pDecryptContext;
    CommonCryptoKeyConfigSettings  algSettings;
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_ClientConfiguration      platformConfig;
    CommonCryptoHandle             cryptoHandle=0;
    CommonCryptoSettings           cryptoSettings;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    NEXUS_Platform_GetClientConfiguration(&platformConfig);

    if (platformConfig.heap[NXCLIENT_FULL_HEAP])
    {
        NEXUS_HeapHandle heap = platformConfig.heap[NXCLIENT_FULL_HEAP];
        NEXUS_MemoryStatus heapStatus;
        NEXUS_Heap_GetStatus(heap, &heapStatus);
        if (heapStatus.memoryType & NEXUS_MemoryType_eFull)
        {
            allocSettings.heap = heap;
        }
    }

    rc = NEXUS_Memory_Allocate(sizeof(DRM_Prdy_DecryptContext_t), &allocSettings, (void *)(&pDecryptContext));
    if (rc)
    {
        BDBG_ERR(("%s: Failed to allocate memory %d, exiting...\n", __FUNCTION__,rc));
        goto ErrorExit;
    }
    BKNI_Memset(pDecryptContext, 0, sizeof(*pDecryptContext));

    ChkMem(pDecryptContext->pDecrypt = Oem_MemAlloc(SIZEOF(DRM_DECRYPT_CONTEXT)));
    BKNI_Memset(pDecryptContext->pDecrypt, 0, SIZEOF(DRM_DECRYPT_CONTEXT));

    /* Allocate key slot */
    ChkDR(Drm_Prdy_AllocateDecryptContextKeySlot(&pDecryptContext->pKeyContext));

    CommonCrypto_GetDefaultKeyConfigSettings(&algSettings);
    algSettings.keySlot = pDecryptContext->pKeyContext->keySlot;
    algSettings.settings.opType = pSettings->opType;
    algSettings.settings.algType = pSettings->algType;
    algSettings.settings.algVariant = pSettings->algVariant;
    algSettings.settings.termMode = pSettings->termMode;
    algSettings.settings.enableExtKey = pSettings->enableExtKey;
    algSettings.settings.enableExtIv = pSettings->enableExtIv;
    algSettings.settings.aesCounterSize = pSettings->aesCounterSize;
    algSettings.settings.keySlotType = pSettings->keySlotType;
    algSettings.settings.aesCounterMode = pSettings->aesCounterMode;

    /* Configure key slot */
    CommonCrypto_GetDefaultSettings(&cryptoSettings);
    cryptoHandle = CommonCrypto_Open(&cryptoSettings);

    if(CommonCrypto_LoadKeyConfig(cryptoHandle, &algSettings) != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - CommonCrypto_ConfigAlg failed aes ctr\n", __FUNCTION__));
        goto ErrorExit;
    }

    CommonCrypto_Close(cryptoHandle);
    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return pDecryptContext;

ErrorExit:
    if(cryptoHandle) CommonCrypto_Close(cryptoHandle);
    if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);
    if(pDecryptContext->pDecrypt) Oem_MemFree(pDecryptContext->pDecrypt);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return NULL;
}

DRM_Prdy_Error_e  DRM_Prdy_SetDecryptContext (
        const DRM_Prdy_DecryptSettings_t  *pSettings,
        DRM_Prdy_DecryptContext_t         *pDecryptContext
        )
{
    DRM_RESULT                     dr = DRM_Prdy_ok;
    CommonCryptoKeyConfigSettings  algSettings;
    CommonCryptoHandle             cryptoHandle=0;
    CommonCryptoSettings           cryptoSettings;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    if( pDecryptContext == NULL )
    {
        BDBG_ERR(("%s: Decrypt Context is NULL, exiting...\n", __FUNCTION__));
        goto ErrorExit;
    }

    if( pDecryptContext->pKeyContext != NULL )
    {
        BDBG_ERR(("%s: Decrypt Context key slot is already allocated...\n", __FUNCTION__));
        goto ErrorExit;
    }

    /* Allocate key slot */
    ChkDR(Drm_Prdy_AllocateDecryptContextKeySlot (&pDecryptContext->pKeyContext));

    CommonCrypto_GetDefaultKeyConfigSettings(&algSettings);
    algSettings.keySlot = pDecryptContext->pKeyContext->keySlot;
    algSettings.settings.opType = pSettings->opType;
    algSettings.settings.algType = pSettings->algType;
    algSettings.settings.algVariant = pSettings->algVariant;
    algSettings.settings.termMode = pSettings->termMode;
    algSettings.settings.enableExtKey = pSettings->enableExtKey;
    algSettings.settings.enableExtIv = pSettings->enableExtIv;
    algSettings.settings.aesCounterSize = pSettings->aesCounterSize;
    algSettings.settings.keySlotType = pSettings->keySlotType;
    algSettings.settings.aesCounterMode = pSettings->aesCounterMode;

    /* Configure key slot */
    CommonCrypto_GetDefaultSettings(&cryptoSettings);
    cryptoHandle = CommonCrypto_Open(&cryptoSettings);

    if(CommonCrypto_LoadKeyConfig(cryptoHandle, &algSettings) != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - CommonCrypto_ConfigAlg failed aes ctr\n", __FUNCTION__));
        goto ErrorExit;
    }

    CommonCrypto_Close(cryptoHandle);
    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    if(cryptoHandle) CommonCrypto_Close(cryptoHandle);
    if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_fail;
}

void DRM_Prdy_FreeDecryptContext (
   DRM_Prdy_DecryptContext_t  *pDecryptContext
   )
{
    if(pDecryptContext )
    {
        if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);
        if(pDecryptContext->pDecrypt) SAFE_OEM_FREE(pDecryptContext->pDecrypt);
        NEXUS_Memory_Free(pDecryptContext);
    }
    return;
}



DRM_Prdy_Error_e DRM_Prdy_Reader_Bind(
        DRM_Prdy_Handle_t          pPrdyContext,
        DRM_Prdy_DecryptContext_t  *pDecryptContext)
{
    DRM_RESULT              dr = DRM_SUCCESS;
    DRM_Prdy_Error_e        result = DRM_Prdy_ok;
    uint8_t                *pbNewOpaqueBuffer = NULL;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pDecryptContext != NULL);

    if(pDecryptContext->pDecrypt == NULL){
        ChkMem(pDecryptContext->pDecrypt = Oem_MemAlloc(SIZEOF(DRM_DECRYPT_CONTEXT)));
        BKNI_Memset(pDecryptContext->pDecrypt, 0, SIZEOF(DRM_DECRYPT_CONTEXT));
    }

    while( (dr = Drm_Reader_Bind(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    1,
                    (DRMPFNPOLICYCALLBACK)DRM_Prdy_policy_callback,
                    (void *) pPrdyContext,
                    (DRM_DECRYPT_CONTEXT *) pDecryptContext->pDecrypt)) == DRM_E_BUFFERTOOSMALL)
    {
        uint32_t cbNewOpaqueBuffer = pPrdyContext->cbOpaqueBuffer * 2;
        BDBG_ASSERT( cbNewOpaqueBuffer > pPrdyContext->cbOpaqueBuffer ); /* overflow check */

        if( cbNewOpaqueBuffer > DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )
        {
            ChkDR( DRM_E_OUTOFMEMORY );
        }

        ChkMem( pbNewOpaqueBuffer = ( uint8_t* )Oem_MemAlloc( cbNewOpaqueBuffer ) );

        ChkDR( Drm_ResizeOpaqueBuffer(
                    pPrdyContext->pDrmAppCtx,
                    pbNewOpaqueBuffer,
                    cbNewOpaqueBuffer ) );

        /*
         Free the old buffer and then transfer the new buffer ownership
         Free must happen after Drm_ResizeOpaqueBuffer because that
         function assumes the existing buffer is still valid
        */
        SAFE_OEM_FREE( pPrdyContext->pbOpaqueBuffer );
        pPrdyContext->cbOpaqueBuffer = cbNewOpaqueBuffer;
        pPrdyContext->pbOpaqueBuffer = pbNewOpaqueBuffer;
        pbNewOpaqueBuffer = NULL;
    }

    if (DRM_FAILED( dr )) {
        if (dr == DRM_E_LICENSE_NOT_FOUND) {
            /* could not find a license for the KID */
            BDBG_ERR(("%s: no licenses found in the license store. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_not_found;
        }
        else if(dr == DRM_E_LICENSE_EXPIRED) {
            /* License is expired */
            BDBG_ERR(("%s: License expired. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_expired;
        }
        else if(  dr == DRM_E_RIV_TOO_SMALL ||
                  dr == DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE )
        {
            /* Revocation Package must be update */
            BDBG_ERR(("%s: Revocation Package must be update. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_revocation_package_expired;
        }
        else {
            BDBG_ERR(("%s: unexpected failure during bind. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_fail;
        }
    }

   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   return result;

ErrorExit:
   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   if( pDecryptContext->pDecrypt != NULL) {
       SAFE_OEM_FREE(pDecryptContext->pDecrypt);
   }
   if( pbNewOpaqueBuffer != NULL) {
       SAFE_OEM_FREE(pbNewOpaqueBuffer);
   }

   return DRM_Prdy_fail;
}

/***********************************************************************************
 * Function: DRM_Prdy_Reader_Bind_Netflix()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Bind_Netflix(
        DRM_Prdy_Handle_t          pPrdyContext,
        uint8_t					   *pSessionID,
        DRM_Prdy_DecryptContext_t  *pDecryptContext)
{
    DRM_RESULT              dr = DRM_SUCCESS;
    DRM_Prdy_Error_e        result = DRM_Prdy_ok;
    uint8_t                *pbNewOpaqueBuffer = NULL;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pDecryptContext != NULL);

    if(pDecryptContext->pDecrypt == NULL){
        ChkMem(pDecryptContext->pDecrypt = Oem_MemAlloc(SIZEOF(DRM_DECRYPT_CONTEXT)));
        BKNI_Memset(pDecryptContext->pDecrypt, 0, SIZEOF(DRM_DECRYPT_CONTEXT));
    }

    while( (dr = Drm_Reader_Bind_Netflix(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    1,
                    (DRMPFNPOLICYCALLBACK)DRM_Prdy_policy_callback,
                    (void *) pPrdyContext,
                    (DRM_BYTE *) pSessionID,
                    (DRM_DECRYPT_CONTEXT *) pDecryptContext->pDecrypt)) == DRM_E_BUFFERTOOSMALL)
    {
        uint32_t cbNewOpaqueBuffer = pPrdyContext->cbOpaqueBuffer * 2;
        BDBG_ASSERT( cbNewOpaqueBuffer > pPrdyContext->cbOpaqueBuffer ); /* overflow check */

        if( cbNewOpaqueBuffer > DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )
        {
            ChkDR( DRM_E_OUTOFMEMORY );
        }

        ChkMem( pbNewOpaqueBuffer = ( uint8_t* )Oem_MemAlloc( cbNewOpaqueBuffer ) );

        ChkDR( Drm_ResizeOpaqueBuffer(
                    pPrdyContext->pDrmAppCtx,
                    pbNewOpaqueBuffer,
                    cbNewOpaqueBuffer ) );

        /*
         Free the old buffer and then transfer the new buffer ownership
         Free must happen after Drm_ResizeOpaqueBuffer because that
         function assumes the existing buffer is still valid
        */
        SAFE_OEM_FREE( pPrdyContext->pbOpaqueBuffer );
        pPrdyContext->cbOpaqueBuffer = cbNewOpaqueBuffer;
        pPrdyContext->pbOpaqueBuffer = pbNewOpaqueBuffer;
        pbNewOpaqueBuffer = NULL;
    }

    if (DRM_FAILED( dr )) {
        if (dr == DRM_E_LICENSE_NOT_FOUND) {
            /* could not find a license for the KID */
            BDBG_ERR(("%s: no licenses found in the license store. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_not_found;
        }
        else if(dr == DRM_E_LICENSE_EXPIRED) {
            /* License is expired */
            BDBG_ERR(("%s: License expired. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_expired;
        }
        else if(  dr == DRM_E_RIV_TOO_SMALL ||
                  dr == DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE )
        {
            /* Revocation Package must be update */
            BDBG_ERR(("%s: Revocation Package must be update. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_revocation_package_expired;
        }
        else {
            BDBG_ERR(("%s: unexpected failure during bind. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_fail;
        }
    }

   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   return result;

ErrorExit:
   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   if( pDecryptContext->pDecrypt != NULL) {
       SAFE_OEM_FREE(pDecryptContext->pDecrypt);
   }
   if( pbNewOpaqueBuffer != NULL) {
       SAFE_OEM_FREE(pbNewOpaqueBuffer);
   }

   return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Reader_Commit(
        DRM_Prdy_Handle_t      pPrdyContext)
{
    DRM_RESULT   dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Reader_Commit( pPrdyContext->pDrmAppCtx, NULL, NULL ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Reader Commit failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Reader_Decrypt(
        DRM_Prdy_DecryptContext_t        *pDecryptContext,
        DRM_Prdy_AES_CTR_Info_t          *pAesCtrInfo,
        uint8_t                          *pBuf,
        uint32_t                          dataSize)
{
    DRM_RESULT     dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pDecryptContext != NULL);
    BDBG_ASSERT(pDecryptContext->pDecrypt != NULL);
    BDBG_ASSERT(pAesCtrInfo != NULL);
    BDBG_ASSERT(pBuf != NULL);

    if(pDecryptContext->pDecrypt == NULL)
    {
        BDBG_ERR(("%s: Invalid Decryptor.\n", __FUNCTION__));
        goto ErrorExit;
    }
    ChkDR( Drm_Reader_Decrypt(
                    pDecryptContext->pDecrypt,
                    (DRM_AES_COUNTER_MODE_CONTEXT *) pAesCtrInfo,
                    pBuf,
                    dataSize ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: Reader Decrypt failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Reader_DecryptOpaque(
        DRM_Prdy_DecryptContext_t        *pDecryptContext,
        DRM_Prdy_AES_CTR_Info_t          *pAesCtrInfo,
        void *                            pScatterGatherList,
        uint32_t                          nelem)
{
    DRM_RESULT     dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pDecryptContext != NULL);
    BDBG_ASSERT(pDecryptContext->pDecrypt != NULL);
    BDBG_ASSERT(pAesCtrInfo != NULL);
    BDBG_ASSERT(pScatterGatherList != NULL);

    if(pDecryptContext->pDecrypt == NULL)
    {
        BDBG_ERR(("%s: Invalid Decryptor.\n", __FUNCTION__));
        goto ErrorExit;
    }
    ChkDR( Drm_Reader_DecryptOpaque(
                    pDecryptContext->pDecrypt,
                    (DRM_AES_COUNTER_MODE_CONTEXT *) pAesCtrInfo,
                    (OEM_OPAQUE_BUFFER_HANDLE)pScatterGatherList,
                    (OEM_OPAQUE_BUFFER_HANDLE)pScatterGatherList,
                    nelem ) );
    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: Reader Decrypt Opaque failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Reader_CloneDecryptContext(
    DRM_Prdy_DecryptContext_t  *pDecryptContext,
    DRM_Prdy_DecryptContext_t  *pClonedDecryptContext
    )
{
    DRM_RESULT     dr = DRM_SUCCESS;

    BDBG_ASSERT(pDecryptContext  != NULL);
    BDBG_ASSERT(pDecryptContext->pDecrypt != NULL);
    BDBG_ASSERT(pClonedDecryptContext != NULL);

    /* Clean up pre-existing nexus keys. Drm_Reader_CloneDecryptContext() will get ride of pre-existing DRM_DECRYPT_CONTEXT */
    if(pClonedDecryptContext->pKeyContext){
        Drm_Prdy_FreeDecryptContextKeySlot(pClonedDecryptContext->pKeyContext);

        if(pDecryptContext->pKeyContext != NULL){
            pClonedDecryptContext->pKeyContext = pDecryptContext->pKeyContext;
            pClonedDecryptContext->pKeyContext->refCount++;
        }
    }

    ChkDR( Drm_Reader_CloneDecryptContext(pDecryptContext->pDecrypt, pClonedDecryptContext->pDecrypt));


    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: DRM_Prdy_Reader_CloneDecryptContext failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Reader_Close( DRM_Prdy_DecryptContext_t  *pDecryptContext)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));
    BDBG_ASSERT(pDecryptContext != NULL);

    if( pDecryptContext->pDecrypt != NULL) {
        Drm_Reader_Close( (DRM_DECRYPT_CONTEXT *) pDecryptContext->pDecrypt );

        if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);

        SAFE_OEM_FREE(pDecryptContext->pDecrypt);
        pDecryptContext->pDecrypt = NULL;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GenerateChallenge(
        DRM_Prdy_Handle_t      pPrdyContext,
        const char            *pCustomData, /* [in] - can be NULL */
        uint32_t               customDataLen,
        char                  *pCh_url,     /* [out] */
        uint32_t              *pUrl_len,
        char                  *pCh_data,    /* [out] */
        uint32_t              *pCh_len)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_BYTE *pbChallenge = (DRM_BYTE *) pCh_data;
    DRM_CHAR *pchURL = pCh_url;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pCh_len != NULL);

    ChkDR( Drm_LicenseAcq_GenerateChallenge(
                pPrdyContext->pDrmAppCtx,
                rgstrRights,
                1,
                NULL,
                pCustomData,
                customDataLen,
                pchURL,
                pUrl_len, /*(pUrl_len>0)?&cchURL:NULL, */
                NULL,
                NULL,
                pbChallenge,
                pCh_len));

    if(dr != DRM_SUCCESS) {
       if(dr == DRM_E_BUFFERTOOSMALL) {
           BDBG_MSG(("%s - The given buffer size is too small.", __FUNCTION__));
           return DRM_Prdy_buffer_size;
       }
       else {
           goto ErrorExit;
       }
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License generation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix()
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
        bool                   isLDL)       /* [in] */
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_BYTE *pbChallenge = (DRM_BYTE *) pCh_data;
    DRM_CHAR *pchURL = pCh_url;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pCh_len != NULL);
    BDBG_ASSERT(pNonce != NULL);    /* **FixMe** add this for safety now; though it might actually be an optional parameter at this level. */

    ChkDR( Drm_LicenseAcq_GenerateChallenge_Netflix(
                pPrdyContext->pDrmAppCtx,
                rgstrRights,
                1,
                NULL,
                pCustomData,
                customDataLen,
                pchURL,
                pUrl_len, /*(pUrl_len>0)?&cchURL:NULL, */
                NULL,
                NULL,
                pbChallenge,
                pCh_len,
                pNonce,
                isLDL) );

    if(dr != DRM_SUCCESS) {
       if(dr == DRM_E_BUFFERTOOSMALL) {
           BDBG_MSG(("%s - The given buffer size is too small.", __FUNCTION__));
           return DRM_Prdy_buffer_size;
       }
       else {
           goto ErrorExit;
       }
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License generation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_CancelChallenge_Netflix()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_CancelChallenge_Netflix(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint8_t               *pNonce)     /* [in] */
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pNonce != NULL);

    ChkDR( Drm_LicenseAcq_CancelChallenge_Netflix(
                pPrdyContext->pDrmAppCtx,
                pNonce) );

    if(dr != DRM_SUCCESS) {
       goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Operation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_GetLdlSessionsLimit_Netflix()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GetLdlSessionsLimit_Netflix(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint32_t              *pLdlSessionsLimit)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pLdlSessionsLimit != NULL);

    ChkDR( Drm_LicenseAcq_GetLdlSessionsLimit_Netflix(
                pPrdyContext->pDrmAppCtx,
                pLdlSessionsLimit) );

    if(dr != DRM_SUCCESS) {
       goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Operation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_FlushLdlChallenges_Netflix()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_FlushLdlChallenges_Netflix(
        DRM_Prdy_Handle_t      pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_LicenseAcq_FlushLdlChallenges_Netflix(
                pPrdyContext->pDrmAppCtx) );

    if(dr != DRM_SUCCESS) {
       goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Operation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponse(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen,
        DRM_Prdy_License_Response_t *pResponse  /* [out]  can be null */
)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oResponse;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ZEROMEM( &oResponse, SIZEOF( DRM_LICENSE_RESPONSE ) );

    ChkDR( Drm_LicenseAcq_ProcessResponse(
                pPrdyContext->pDrmAppCtx,
                DRM_PROCESS_LIC_RESPONSE_NO_FLAGS,
                NULL,
                NULL,
                ( uint8_t * )pData,
                dataLen,
                &oResponse ) );

    if(pResponse != NULL)
    {
        BKNI_Memcpy(pResponse, (void*)&oResponse, sizeof(DRM_LICENSE_RESPONSE));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Process Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponseNonPersistent(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen,
        DRM_Prdy_License_Response_t *pResponse  /* [out]  can be null */
)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oResponse;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ZEROMEM( &oResponse, SIZEOF( DRM_LICENSE_RESPONSE ) );

    ChkDR( Drm_LicenseAcq_ProcessResponse(
                pPrdyContext->pDrmAppCtx,
                DRM_PROCESS_LIC_RESPONSE_SIGNATURE_NOT_REQUIRED,
                NULL,
                NULL,
                ( uint8_t * )pData,
                dataLen,
                &oResponse ) );

    if(pResponse != NULL)
    {
        BKNI_Memcpy(pResponse, (void*)&oResponse, sizeof(DRM_LICENSE_RESPONSE));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Process Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponse_SecStop(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen,
		uint8_t						*pSessionID,
        DRM_Prdy_License_Response_t *pResponse  /* [out]  can be null */
)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oResponse;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ZEROMEM( &oResponse, SIZEOF( DRM_LICENSE_RESPONSE ) );

    ChkDR( Drm_LicenseAcq_ProcessResponse_SecStop(
                pPrdyContext->pDrmAppCtx,
                DRM_PROCESS_LIC_RESPONSE_NO_FLAGS,
                NULL,
                NULL,
                ( uint8_t * )pData,
                dataLen,
				( DRM_BYTE *)pSessionID,
                &oResponse ) );

    if(pResponse != NULL)
    {
        BKNI_Memcpy(pResponse, (void*)&oResponse, sizeof(DRM_LICENSE_RESPONSE));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Process Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponse_Netflix(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen,
        uint8_t                     *pSessionID,
        DRM_Prdy_License_Response_t *pResponse  /* [out]  can be null */
)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oResponse;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ZEROMEM( &oResponse, SIZEOF( DRM_LICENSE_RESPONSE ) );

    ChkDR( Drm_LicenseAcq_ProcessResponse_Netflix(
                pPrdyContext->pDrmAppCtx,
                DRM_PROCESS_LIC_RESPONSE_NO_FLAGS,
                NULL,
                NULL,
                ( uint8_t * )pData,
                dataLen,
                ( DRM_BYTE *)pSessionID,
                &oResponse ) );

    if(pResponse != NULL)
    {
        BKNI_Memcpy(pResponse, (void*)&oResponse, sizeof(DRM_LICENSE_RESPONSE));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Process Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GenerateAck(
        DRM_Prdy_Handle_t            pPrdyContext,
        DRM_Prdy_License_Response_t *pResponse, /* [in] */
        char                        *pCh_data,    /* [out] */
        uint32_t                    *pCh_len)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pResponse != NULL);
    BDBG_ASSERT(pCh_data != NULL);
    BDBG_ASSERT(pCh_len != NULL);

    ChkDR( Drm_LicenseAcq_GenerateAck(
                pPrdyContext->pDrmAppCtx,
                (DRM_LICENSE_RESPONSE *)pResponse,
                (DRM_BYTE *)pCh_data,
                pCh_len ));

    if(dr != DRM_SUCCESS) {
       if(dr == DRM_E_BUFFERTOOSMALL) {
           BDBG_MSG(("%s - The given buffer size is too small.", __FUNCTION__));
           return DRM_Prdy_buffer_size;
       }
       else {
           goto ErrorExit;
       }
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Generating ACK for license's response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessAckResponse(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen
)
{
    DRM_RESULT dr = DRM_SUCCESS;

    DRM_RESULT f_pResult;
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ChkDR( Drm_LicenseAcq_ProcessAckResponse(
            pPrdyContext->pDrmAppCtx,
            ( DRM_BYTE * )pData,
            dataLen,
            &f_pResult));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Acq Process Ack Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}



DRM_Prdy_Error_e DRM_Prdy_Device_GetProperty(
        DRM_Prdy_Handle_t      pPrdyContext,
        char                  *pClient_info,     /* [out] */
        uint32_t               pCliInfo_len)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_CHAR *pbClientInfo = (DRM_CHAR *) pClient_info;
    DRM_BYTE *tmpClientInfo = NULL;
    DRM_DWORD cchLen = pCliInfo_len;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pClient_info != NULL);

    ChkMem(tmpClientInfo = (DRM_BYTE*) Oem_MemAlloc(cchLen * SIZEOF (DRM_BYTE)));

    ChkDR(Drm_Device_GetProperty(
                pPrdyContext->pDrmAppCtx,
                DRM_DGP_CLIENT_INFO,
                tmpClientInfo,
                &cchLen));

    /* DRM_DGP_CLIENT_INFO is UNICODE blob which is NOT NULL terminated */
    DRM_UTL_DemoteUNICODEtoASCII((DRM_WCHAR *)tmpClientInfo, pbClientInfo, cchLen);  /* transform from wide-char to ANSI */

    if( tmpClientInfo != NULL) {
        SAFE_OEM_FREE(tmpClientInfo);
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    if( tmpClientInfo != NULL) {
        SAFE_OEM_FREE(tmpClientInfo);
    }
    BDBG_ERR(("%s: Device GetProperty failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_GetAdditionalResponseData(
        DRM_Prdy_Handle_t   pPrdyContext,
        uint8_t            *f_pbResponse,
        uint32_t            f_cbResponse,
        uint32_t            f_dwDataType,
        char               *f_pchDataString,
        uint32_t            f_cbchDataString)
{

    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(f_pbResponse != NULL);
    BDBG_ASSERT(f_pchDataString != NULL);

    ChkDR(Drm_GetAdditionalResponseData(
                pPrdyContext->pDrmAppCtx,
                f_pbResponse,
                f_cbResponse,
                f_dwDataType,
                f_pchDataString,
                &f_cbchDataString ));


    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: Device GetProperty failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Revocation_StorePackage(
        DRM_Prdy_Handle_t      pPrdyContext,
        char                  *pbRevPackage,
        uint32_t               cbRevPackage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbRevPackage != NULL);

    ChkDR( Drm_Revocation_StorePackage(
                pPrdyContext->pDrmAppCtx,
                ( DRM_CHAR * )pbRevPackage,
                cbRevPackage ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Revocation Store Package failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_InitializePolicyDescriptor(
        DRM_Prdy_local_license_policy_descriptor_t    *pPoDescriptor)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPoDescriptor != NULL);

    ChkDR( Drm_LocalLicense_InitializePolicyDescriptor(
                                 (DRM_LOCAL_LICENSE_POLICY_DESCRIPTOR *) pPoDescriptor) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: InitializePolicyDescriptor failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;


}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_CreateLicense(
        DRM_Prdy_Handle_t                                  pPrdyContext,
        const DRM_Prdy_local_license_policy_descriptor_t  *pPoDescriptor,
        DRM_Prdy_local_license_type_e                      elicenseType,
        const Drm_Prdy_KID_t                              *pKeyId,
        uint16_t                                           cbRemoteCert,
        const uint8_t                                     *pbRemoteCert,
        const DRM_Prdy_license_handle                      hRootLicense,
        const DRM_Prdy_license_handle                     *phLicense)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pPoDescriptor  != NULL);
    BDBG_ASSERT(pKeyId       != NULL);
    BDBG_ASSERT(phLicense    != NULL);

    ChkDR( Drm_LocalLicense_CreateLicense(
                pPrdyContext->pDrmAppCtx,
                ( const DRM_LOCAL_LICENSE_POLICY_DESCRIPTOR * ) pPoDescriptor,
                ( DRM_LOCAL_LICENSE_TYPE ) elicenseType,
                ( const DRM_KID *) pKeyId,
                cbRemoteCert,
                pbRemoteCert,
                ( const DRM_LICENSE_HANDLE ) hRootLicense,
                ( DRM_LICENSE_HANDLE *) phLicense ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Locallicens_CreateLicense failed [0x%X], exiting...\n",__FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_StoreLicense(
        const DRM_Prdy_license_handle     hLicense,
        DRM_Prdy_local_license_store_e    eLicenseStore)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );

    ChkDR( Drm_LocalLicense_StoreLicense(
                ( const DRM_LICENSE_HANDLE ) hLicense,
                ( const DRM_LOCAL_LICENSE_STORE ) eLicenseStore ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: LocalLicense_StoreLicense failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;

}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_Release(DRM_Prdy_license_handle  *hLicense)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );

    ChkDR( Drm_LocalLicense_Release( (DRM_LICENSE_HANDLE ) hLicense) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: LocalLicense_Release failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_LocalLicense_EncryptSample(
        DRM_Prdy_license_handle   *hLicense,
        uint8_t                   *pInBuf,
        uint8_t                   *pOutBuf,
        uint32_t                   cbData,
        uint8_t                   *pIV)
{
    DRM_RESULT    dr = DRM_SUCCESS;
    DRM_DWORD     rgbSubsampleCount[1] = {0};
    DRM_BYTE      *rgbSubsamplePointer[1] = {0};

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );
    BDBG_ASSERT(pInBuf != NULL );
    BDBG_ASSERT(pOutBuf != NULL );
    BDBG_ASSERT(pIV != NULL );

    BKNI_Memcpy(pOutBuf, pInBuf, cbData);

    rgbSubsampleCount[0]   = cbData;
    rgbSubsamplePointer[0] = (DRM_BYTE*) pOutBuf;

    ChkDR( Drm_LocalLicense_EncryptSample(
                (DRM_LICENSE_HANDLE ) hLicense,
                1,
                rgbSubsampleCount,
                rgbSubsamplePointer,
                (DRM_UINT64 *)pIV) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: LocalLicense_EncryptSample failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_LocalLicense_EncryptSubsamples(
        DRM_Prdy_license_handle   *hLicense,
        DRM_Prdy_sample_t         *pClearSamples,
        DRM_Prdy_sample_t         *pEncSamples,
        uint8_t                   *pIV)
{
    DRM_RESULT              dr = DRM_SUCCESS;
    DRM_DWORD              *rgbSubsampleCount = NULL;
    DRM_BYTE              **rgbSubsamplePointer = NULL;
    uint32_t                numOfSubsamples;
    DRM_Prdy_subSample_t   *clrSub = NULL;
    DRM_Prdy_subSample_t   *encSub = NULL;
    uint32_t               i = 0;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );
    BDBG_ASSERT(pClearSamples != NULL );
    BDBG_ASSERT(pEncSamples != NULL );
    BDBG_ASSERT(pIV != NULL );

    numOfSubsamples = pClearSamples->numOfSubsamples;
    ChkMem(rgbSubsampleCount = Oem_MemAlloc(SIZEOF(DRM_DWORD) * numOfSubsamples));
    ChkMem(rgbSubsamplePointer = Oem_MemAlloc(SIZEOF(DRM_BYTE) * numOfSubsamples));

    clrSub = pClearSamples->subsamples;
    encSub = pEncSamples->subsamples;

    for( i =0; i < numOfSubsamples; ++i)
    {
        rgbSubsampleCount[i] = clrSub[i].size;
        /*
         * Assuming the memory of pEncSamples has been properly allocated
         * However, this memcpy can be avoided if we support in-place encryption
         */
        BKNI_Memcpy(encSub[i].sample, clrSub[i].sample, clrSub[i].size);
        /*
         * To avoid an extra memcpy for the output at the end,
         * we passed the pEncSamples[].subsamples.sample directly to the API
         * */
        rgbSubsamplePointer[i] = encSub[i].sample;
    }

    ChkDR( Drm_LocalLicense_EncryptSample(
                (DRM_LICENSE_HANDLE ) hLicense,
                numOfSubsamples,
                rgbSubsampleCount,
                rgbSubsamplePointer,
                (DRM_UINT64 *)pIV) );

    /* clean up */
    if( rgbSubsampleCount != NULL)  SAFE_OEM_FREE(rgbSubsampleCount);
    if( rgbSubsamplePointer  != NULL)  SAFE_OEM_FREE(rgbSubsamplePointer);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    if( rgbSubsampleCount != NULL)  SAFE_OEM_FREE(rgbSubsampleCount);
    if( rgbSubsamplePointer  != NULL)  SAFE_OEM_FREE(rgbSubsamplePointer);

    BDBG_ERR(("%s: LocalLicense_EncryptSample failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_GetKID(
        const DRM_Prdy_license_handle    hLicense,
        Drm_Prdy_KID_t                  *pKeyID )
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );
    BDBG_ASSERT(pKeyID != NULL );

    ChkDR( Drm_LocalLicense_GetKID(
                (DRM_LICENSE_HANDLE ) hLicense,
                (DRM_KID *)pKeyID) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: LocalLicense_GetKID failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_GetKID_base64W(
        const DRM_Prdy_license_handle    hLicense,
        uint16_t                        *pKidBase64W,
        uint32_t                        *pRequiredSize )
{
    DRM_RESULT      dr = DRM_SUCCESS;
    DRM_KID         pKeyID;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );
    BDBG_ASSERT(pKidBase64W != NULL );
    BDBG_ASSERT(pRequiredSize != NULL );

    ChkDR( Drm_LocalLicense_GetKID(
                (DRM_LICENSE_HANDLE ) hLicense,
                &pKeyID) );

    *pRequiredSize = CCH_BASE64_EQUIV( SIZEOF( DRM_GUID ));

    if( DRM_B64_EncodeW(
        (const DRM_BYTE*)&pKeyID,
        SIZEOF( DRM_KID ),
        (DRM_WCHAR *)pKidBase64W,
        (DRM_DWORD *)pRequiredSize,
        0 ) == DRM_E_BUFFERTOOSMALL )
    {
       BDBG_MSG(("%s: KeyID Buffer too small, requiring %d, exiting\n", __FUNCTION__, *pRequiredSize));
       return DRM_Prdy_buffer_size;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: LocalLicense_GetKID failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_StoreMgmt_DeleteLicenses(
        const DRM_Prdy_Handle_t            pPrdyContext,
        const uint16_t                     *pKidBase64W,
        uint32_t                            cbKidSize,
        uint32_t                           *pcLicDeleted)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_CONST_STRING dcstrKID = EMPTY_DRM_STRING;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pcLicDeleted != NULL);

    if( pKidBase64W != NULL)
    {
        dcstrKID.cchString = cbKidSize;
        ChkMem(dcstrKID.pwszString = (DRM_WCHAR *)Oem_MemAlloc(dcstrKID.cchString*SIZEOF(DRM_WCHAR)));
        BKNI_Memset((DRM_BYTE *)dcstrKID.pwszString, 0, dcstrKID.cchString*SIZEOF(DRM_WCHAR));
        BKNI_Memcpy((DRM_BYTE *)dcstrKID.pwszString, pKidBase64W, cbKidSize*SIZEOF(DRM_WCHAR));
        ChkDR( Drm_StoreMgmt_DeleteLicenses(
                    pPrdyContext->pDrmAppCtx,
                    &dcstrKID,
                    (DRM_DWORD *)pcLicDeleted));
    }
    else
    {
        ChkDR( Drm_StoreMgmt_DeleteLicenses(
                    pPrdyContext->pDrmAppCtx,
                    NULL,
                    (DRM_DWORD *)pcLicDeleted));
    }

    if( dcstrKID.pwszString != NULL) {
        SAFE_OEM_FREE(dcstrKID.pwszString);
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    if( dcstrKID.pwszString != NULL) {
        SAFE_OEM_FREE(dcstrKID.pwszString);
    }
    BDBG_ERR(("%s: Delete Licenses failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_Cleanup_LicenseStores( DRM_Prdy_Handle_t   pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR(Drm_StoreMgmt_CleanupStore( pPrdyContext->pDrmAppCtx,
                                      DRM_STORE_CLEANUP_ALL,
                                      NULL,
                                      5,
                                      NULL));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to cleanup licens store [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

/*********************************************************************************
 *********************************************************************************
 *
 *     Prdy ND APIs
 *
 *********************************************************************************
 *********************************************************************************/

void DRM_Prdy_ND_MemFree( uint8_t   *pbToFree)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    if( pbToFree != NULL)
    {
        Drm_Prnd_MemFree(pbToFree);
   }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
}

DRM_Prdy_Error_e DRM_Prdy_ND_GetMessageType(
        const uint8_t               *pbUnknownMessage,
        uint16_t                     cbUnknownMessage,
        DRM_Prdy_ND_Message_Type_e  *peMessageType )
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pbUnknownMessage != NULL);
    BDBG_ASSERT(peMessageType != NULL);

    ChkDR( Drm_Prnd_GetMessageType(
                (const DRM_BYTE *) pbUnknownMessage,
                (DRM_DWORD ) cbUnknownMessage,
                (DRM_PRND_MESSAGE_TYPE *) peMessageType));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to get message type [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

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
        uint32_t                                 *pPrdyResultCode )
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbReqMessage != NULL);
    BDBG_ASSERT(pSessionID != NULL);
    BDBG_ASSERT(peProximityDetectionType != NULL);
    BDBG_ASSERT(ppbTransmitterProximityDetectionChannel != NULL);
    BDBG_ASSERT(pcbTransmitterProximityDetectionChannel != NULL);
    BDBG_ASSERT(pdwFlags  != NULL);
    BDBG_ASSERT(pPrdyResultCode  != NULL);

    *pPrdyResultCode =
          (uint32_t) Drm_Prnd_Transmitter_RegistrationRequest_Process(
                            pPrdyContext->pDrmAppCtx,
                            (const DRM_BYTE *) pbReqMessage,
                            (DRM_DWORD) cbReqMessage,
                            (Drm_Prnd_Data_Callback) pfnDataCallback,
                            (DRM_VOID *) pvDataCallbackContext,
                            (DRM_ID *) pSessionID,
                            (DRM_PRND_PROXIMITY_DETECTION_TYPE *) peProximityDetectionType,
                            (DRM_BYTE **) ppbTransmitterProximityDetectionChannel,
                            (DRM_DWORD *) pcbTransmitterProximityDetectionChannel,
                            (DRM_DWORD *) pdwFlags );

    if( *pPrdyResultCode != DRM_SUCCESS) {
        goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Transmitter_RegistrationRequest_Process failed [0x%X], exiting...\n", __FUNCTION__,*pPrdyResultCode));
    return DRM_Prdy_fail;
}

DRM_Prdy_ND_Transmitter_Context_t DRM_Prdy_ND_Transmitter_Initialize(void)
{
    DRM_PRDY_ND_TRANSMITTER_CONTEXT   *pTxCtx = NULL;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    pTxCtx = Oem_MemAlloc(sizeof(DRM_PRDY_ND_TRANSMITTER_CONTEXT));
    if( pTxCtx == NULL ) {
        BDBG_ERR(("%s: Transmitter Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memset(pTxCtx, 0, sizeof(DRM_PRDY_ND_TRANSMITTER_CONTEXT));

    pTxCtx->pPrndTxContext = Oem_MemAlloc(sizeof(DRM_PRND_TRANSMITTER_CONTEXT));
    if( pTxCtx->pPrndTxContext == NULL ) {
        BDBG_ERR(("%s: DRM PRND Transmitter Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memset(pTxCtx->pPrndTxContext, 0, sizeof(DRM_PRND_TRANSMITTER_CONTEXT));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return (DRM_Prdy_ND_Transmitter_Context_t) pTxCtx;

ErrorExit:
    BDBG_ERR(("%s: Transmitter_RegistrationRequest_Process failed, exiting...\n", __FUNCTION__));
    return NULL;
}


DRM_Prdy_ND_Receiver_Context_t DRM_Prdy_ND_Receiver_Initialize(void)
{
    DRM_PRDY_ND_RECEIVER_CONTEXT   *pRxCtx = NULL;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    pRxCtx = Oem_MemAlloc(sizeof(DRM_PRDY_ND_RECEIVER_CONTEXT));
    if( pRxCtx == NULL ) {
        BDBG_ERR(("%s: Receiver Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memset(pRxCtx, 0, sizeof(DRM_PRDY_ND_RECEIVER_CONTEXT));

    pRxCtx->pPrndRxContext = Oem_MemAlloc(sizeof(DRM_PRND_RECEIVER_CONTEXT));
    if( pRxCtx->pPrndRxContext == NULL ) {
        BDBG_ERR(("%s: DRM PRND Receiver Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memset(pRxCtx->pPrndRxContext, 0, sizeof(DRM_PRND_RECEIVER_CONTEXT));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return (DRM_Prdy_ND_Receiver_Context_t) pRxCtx;

ErrorExit:
    BDBG_ERR(("%s: Receiver_RegistrationRequest_Process failed, exiting...\n", __FUNCTION__));
    return NULL;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_Uninitialize( DRM_Prdy_ND_Transmitter_Context_t  pPrdyTxCtx)
{
    if(pPrdyTxCtx != NULL)
    {
        if((pPrdyTxCtx)->pPrndTxContext != NULL)
        {
            SAFE_OEM_FREE( pPrdyTxCtx->pPrndTxContext );
            pPrdyTxCtx->pPrndTxContext = NULL;
        }
    }
    return DRM_Prdy_ok;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_Uninitialize( DRM_Prdy_ND_Receiver_Context_t  pPrdyRxCtx)
{
    if(pPrdyRxCtx != NULL)
    {
        if(pPrdyRxCtx->pPrndRxContext != NULL)
        {
            SAFE_OEM_FREE( pPrdyRxCtx->pPrndRxContext );
            pPrdyRxCtx->pPrndRxContext = NULL;
        }
    }
    return DRM_Prdy_ok;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_BeginSession(
        DRM_Prdy_Handle_t                   pPrdyContext,
        DRM_Prdy_ND_Transmitter_Context_t   pPrdyTxCtx)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pPrdyTxCtx != NULL);

    ChkDR( Drm_Prnd_Transmitter_BeginSession(
                pPrdyContext->pDrmAppCtx,
                pPrdyTxCtx->pPrndTxContext));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Transmitter_BeginSession failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_BeginSession(
        DRM_Prdy_Handle_t                pPrdyContext,
        DRM_Prdy_ND_Receiver_Context_t   pPrdyRxCtx)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pPrdyRxCtx != NULL);

    ChkDR( Drm_Prnd_Receiver_BeginSession(
                pPrdyContext->pDrmAppCtx,
                pPrdyRxCtx->pPrndRxContext));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Receiver_BeginSession failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_EndSession( DRM_Prdy_Handle_t   pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Prnd_Transmitter_EndSession(
                pPrdyContext->pDrmAppCtx ));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Transmitter_EndSession failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_EndSession( DRM_Prdy_Handle_t   pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Prnd_Receiver_EndSession(
                pPrdyContext->pDrmAppCtx ));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Receiver_EndSession failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_RegistrationResponse_Generate(
        DRM_Prdy_Handle_t        pPrdyContext,
        const DRM_Prdy_ID_t     *pCustomDataTypeID,
        const uint8_t           *pbCustomData,
        uint16_t                 cbCustomData,
        uint16_t                 dwFlags,
        uint8_t                **ppbRespMessage,
        uint16_t                *pcbRespMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Prnd_Transmitter_RegistrationResponse_Generate(
                pPrdyContext->pDrmAppCtx,
                (DRM_ID *) pCustomDataTypeID,
                (DRM_BYTE *) pbCustomData,
                (DRM_DWORD) cbCustomData,
                 DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbRespMessage,
                (DRM_DWORD *) pcbRespMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: RegistrationResponse Generation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_RegistrationError_Generate(
        DRM_Prdy_Handle_t        pPrdyContext,
        uint32_t                 prdyResultCode,
        uint16_t                 dwFlags,
        uint8_t                **ppbErrMessage,
        uint16_t                *pcbErrMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbErrMessage != NULL);
    BDBG_ASSERT(pcbErrMessage != NULL);

    ChkDR( Drm_Prnd_Transmitter_RegistrationError_Generate(
                pPrdyContext->pDrmAppCtx,
                (DRM_RESULT ) prdyResultCode,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbErrMessage,
                (DRM_DWORD *) pcbErrMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: RegistrationError Generation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_LicenseRequest_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbLicReqMessage,
        uint16_t                                   cbLicReqMessage,
        DRM_Prdy_ND_Data_Callback                  pfnDataCallback,
        void                                      *pvDataCallbackContext,
        Drm_Prdy_ND_Content_Identifier_Callback    pfnContentIdentifierCallback,
        void                                      *pvContentIdentifierCallbackContext,
        uint16_t                                  *pdwFlags,
        uint32_t                                  *pPrdyResultCode )
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbLicReqMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    *pPrdyResultCode =
        (uint32_t) Drm_Prnd_Transmitter_LicenseRequest_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbLicReqMessage,
                (DRM_DWORD) cbLicReqMessage,
                (Drm_Prnd_Data_Callback) pfnDataCallback,
                (DRM_VOID *) pvDataCallbackContext,
                (Drm_Prnd_Content_Identifier_Callback) pfnContentIdentifierCallback,
                (DRM_VOID *) pvContentIdentifierCallbackContext,
                (DRM_DWORD *) pdwFlags);

    if( *pPrdyResultCode != DRM_SUCCESS) {
        goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Processeing LicenseRequest failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)pPrdyResultCode));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_LicenseTransmit_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pCustomDataTypeID,
        const uint8_t                             *pbCustomData,
        uint16_t                                   cbCustomData,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbLicTransmitMessage,
        uint16_t                                  *pcbLicTransmitMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbLicTransmitMessage != NULL);
    BDBG_ASSERT(pcbLicTransmitMessage != NULL);

    ChkDR( Drm_Prnd_Transmitter_LicenseTransmit_Generate(
                pPrdyContext->pDrmAppCtx,
                (DRM_ID *) pCustomDataTypeID,
                (DRM_BYTE *) pbCustomData,
                (DRM_DWORD) cbCustomData,
                (DRM_DWORD) dwFlags,
                (DRM_BYTE **) ppbLicTransmitMessage,
                (DRM_DWORD *) pcbLicTransmitMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Generation for LicenseTransmit failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_PrepareLicensesForTransmit(
        DRM_Prdy_Handle_t         pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Prnd_Transmitter_PrepareLicensesForTransmit (
                pPrdyContext->pDrmAppCtx));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Preparing License for transmit failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_LicenseError_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pSessionID,
        uint32_t                                   prdyResultCode,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbErrMessage,
        uint16_t                                  *pcbErrMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbErrMessage != NULL);
    BDBG_ASSERT(pcbErrMessage != NULL);

    ChkDR( Drm_Prnd_Transmitter_LicenseError_Generate(
                pPrdyContext->pDrmAppCtx,
                (const DRM_ID *) pSessionID,
                (DRM_RESULT ) prdyResultCode,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbErrMessage,
                (DRM_DWORD *) pcbErrMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate License error [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_ProximityDetectionResponse_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbPDRespMessage,
        uint16_t                                   cbPDRespMessage,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbPDResultMessage,
        uint16_t                                  *pcbPDResultMessage,
        uint16_t                                  *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbPDRespMessage != NULL);
    BDBG_ASSERT(ppbPDResultMessage != NULL);
    BDBG_ASSERT(pcbPDResultMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Transmitter_ProximityDetectionResponse_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbPDRespMessage,
                (DRM_DWORD ) cbPDRespMessage,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDResultMessage,
                (DRM_DWORD *) pcbPDResultMessage,
                (DRM_DWORD *) pdwFlags ));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process Proximity Dectection Response [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_ProximityDetectionResult_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pSessionID,
        uint32_t                                   prdyResultCode,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbPDResultMessage,
        uint16_t                                  *pcbPDResultMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbPDResultMessage != NULL);
    BDBG_ASSERT(pcbPDResultMessage != NULL);

    ChkDR( Drm_Prnd_Transmitter_ProximityDetectionResult_Generate(
                pPrdyContext->pDrmAppCtx,
                (const DRM_ID *) pSessionID,
                (DRM_RESULT ) prdyResultCode,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDResultMessage,
                (DRM_DWORD *) pcbPDResultMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate proximity dectection result [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_ProximityDetectionStart_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbPDStartMessage,
        uint16_t                                   cbPDStartMessage,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbPDChlgMessage,
        uint16_t                                  *pcbPDChlgMessage,
        uint16_t                                  *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbPDStartMessage != NULL);
    BDBG_ASSERT(ppbPDChlgMessage != NULL);
    BDBG_ASSERT(pcbPDChlgMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Transmitter_ProximityDetectionStart_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbPDStartMessage,
                (DRM_DWORD ) cbPDStartMessage,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDChlgMessage,
                (DRM_DWORD *) pcbPDChlgMessage,
                (DRM_DWORD *) pdwFlags ));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process Proximity Dectection Start message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_RegistrationRequest_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pPreviousSessionID,
        const DRM_Prdy_ID_t                       *pCustomDataTypeID,
        const uint8_t                             *pbCustomData,
        uint16_t                                   cbCustomData,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbReqMessage,
        uint16_t                                  *pcbReqMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbReqMessage != NULL);
    BDBG_ASSERT(pcbReqMessage != NULL);

    ChkDR( Drm_Prnd_Receiver_RegistrationRequest_Generate(
                pPrdyContext->pDrmAppCtx,
                (DRM_ID *) pPreviousSessionID,
                (DRM_ID *) pCustomDataTypeID,
                (const DRM_BYTE *) pbCustomData,
                (DRM_DWORD ) cbCustomData,
                (DRM_DWORD ) dwFlags,
                (DRM_BYTE **) ppbReqMessage,
                (DRM_DWORD *) pcbReqMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate Registration request [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

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
        uint32_t                                  *pPrdyResultCode)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbRespMessage != NULL);
    BDBG_ASSERT(pSessionID != NULL);
    BDBG_ASSERT(peProximityDetectionType != NULL);
    BDBG_ASSERT(ppbTransmitterProximityDetectionChannel != NULL);
    BDBG_ASSERT(pcbTransmitterProximityDetectionChannel != NULL);
    BDBG_ASSERT(pdwFlags  != NULL);
    BDBG_ASSERT(pPrdyResultCode  != NULL);

    *pPrdyResultCode =
          (uint32_t) Drm_Prnd_Receiver_RegistrationResponse_Process(
                            pPrdyContext->pDrmAppCtx,
                            (const DRM_BYTE *) pbRespMessage,
                            (DRM_DWORD) cbRespMessage,
                            (Drm_Prnd_Data_Callback) pfnDataCallback,
                            (DRM_VOID *) pvDataCallbackContext,
                            (DRM_ID *) pSessionID,
                            (DRM_PRND_PROXIMITY_DETECTION_TYPE *) peProximityDetectionType,
                            (DRM_BYTE **) ppbTransmitterProximityDetectionChannel,
                            (DRM_DWORD *) pcbTransmitterProximityDetectionChannel,
                            (DRM_DWORD *) pdwFlags );

    if( *pPrdyResultCode != DRM_SUCCESS) {
        goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Processing Registration Response failed [0x%X], exiting...\n", __FUNCTION__,*pPrdyResultCode));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_RegistrationError_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbErrMessage,
        uint16_t                                 cbErrMessage,
        uint32_t                                *pPrdyResultCode,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbErrMessage != NULL);
    BDBG_ASSERT(pPrdyResultCode != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_RegistrationError_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbErrMessage,
                (DRM_DWORD) cbErrMessage,
                (DRM_RESULT * ) pPrdyResultCode,
                (DRM_DWORD *) pdwFlags));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process Registration Error message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

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
        uint16_t                                *pcbLicReqMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pguidRequestedAction != NULL);
    BDBG_ASSERT(pbContentIdentifier != NULL);
    BDBG_ASSERT(ppbLicReqMessage != NULL);
    BDBG_ASSERT(pcbLicReqMessage != NULL);

    ChkDR( Drm_Prnd_Receiver_LicenseRequest_Generate (
                pPrdyContext->pDrmAppCtx,
                (const DRM_GUID *) pguidRequestedAction,
                (const DRM_GUID *) pguidRequestedActionQualifier,
                (DRM_PRND_CONTENT_IDENTIFIER_TYPE) eContentIdentifierType,
                (const DRM_BYTE *)pbContentIdentifier,
                (DRM_DWORD)cbContentIdentifier,
                (const DRM_ID *) pCustomDataTypeID,
                (const DRM_BYTE *) pbCustomData,
                (DRM_DWORD) cbCustomData,
                (DRM_DWORD) dwFlags,
                (DRM_BYTE **) ppbLicReqMessage,
                (DRM_DWORD *) pcbLicReqMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate License Request [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_LicenseTransmit_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbLicTransmitMessage,
        uint16_t                                 cbLicTransmitMessage,
        DRM_Prdy_ND_Data_Callback                pfnDataCallback,
        void                                    *pvDataCallbackContext,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbLicTransmitMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_LicenseTransmit_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbLicTransmitMessage,
                (DRM_DWORD) cbLicTransmitMessage,
                (Drm_Prnd_Data_Callback) pfnDataCallback,
                (DRM_VOID *) pvDataCallbackContext,
                (DRM_DWORD *) pdwFlags));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process License Response from Transmitter [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_LicenseError_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbErrMessage,
        uint16_t                                 cbErrMessage,
        uint32_t                                *pPrdyResultCode,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbErrMessage != NULL);
    BDBG_ASSERT(pPrdyResultCode != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_LicenseError_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbErrMessage,
                (DRM_DWORD) cbErrMessage,
                (DRM_RESULT * ) pPrdyResultCode,
                (DRM_DWORD *) pdwFlags));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process License Error message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_ProximityDetectionStart_Generate(
        DRM_Prdy_Handle_t                        pPrdyContext,
        uint16_t                                 dwFlags,
        uint8_t                                **ppbPDStartMessage,
        uint16_t                                *pcbPDStartMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbPDStartMessage != NULL);
    BDBG_ASSERT(ppbPDStartMessage != NULL);
    BDBG_ASSERT(pcbPDStartMessage != NULL);

    ChkDR( Drm_Prnd_Receiver_ProximityDetectionStart_Generate(
                pPrdyContext->pDrmAppCtx,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDStartMessage,
                (DRM_DWORD *) pcbPDStartMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate a Proximity Detection Start message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_ProximityDetectionChallenge_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbPDChlgMessage,
        uint16_t                                 cbPDChlgMessage,
        uint16_t                                 dwFlags,
        uint8_t                                **ppbPDRespMessage,
        uint16_t                                *pcbPDRespMessage,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbPDChlgMessage != NULL);
    BDBG_ASSERT(ppbPDRespMessage != NULL);
    BDBG_ASSERT(pcbPDRespMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_ProximityDetectionChallenge_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbPDChlgMessage,
                (DRM_DWORD) cbPDChlgMessage,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDRespMessage,
                (DRM_DWORD *) pcbPDRespMessage,
                (DRM_DWORD *) pdwFlags));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process a Proximity Detection Challenge response message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_ProximityDetectionResult_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbPDResultMessage,
        uint16_t                                 cbPDResultMessage,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_RESULT drPDResult = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbPDResultMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_ProximityDetectionResult_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbPDResultMessage,
                (DRM_DWORD) cbPDResultMessage,
                &drPDResult,
                (DRM_DWORD *) pdwFlags));

    ChkDR( drPDResult);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process a Proximity Detection Challenge response message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


/* helper functions */
DRM_Prdy_Error_e DRM_Prdy_B64_EncodeW( uint8_t *pBytes, uint32_t cbBytes, uint16_t *pBase64W, uint32_t *pReqSize)
{
    if( DRM_B64_EncodeW(
        (const DRM_BYTE*)pBytes,
        cbBytes,
        (DRM_WCHAR *)pBase64W,
        (DRM_DWORD *)pReqSize,
        0 ) == DRM_E_BUFFERTOOSMALL )
    {
       return DRM_Prdy_buffer_size;
    }
    return DRM_Prdy_ok;
}

bool DRM_Prdy_convertCStringToWString( char * pCStr, wchar_t * pWStr, uint16_t * cchStr)
{
    return convertCStringToWString( pCStr, (DRM_WCHAR *) pWStr, (DRM_DWORD *) cchStr);
}

bool DRM_Prdy_convertWStringToCString( wchar_t * pWStr, char * pCStr, uint32_t cchStr)
{
    return convertWStringToCString( (DRM_WCHAR *) pWStr, pCStr, (DRM_DWORD ) cchStr);
}

void DRM_Prdy_qwordToNetworkbytes(uint8_t *pBytes, unsigned index, uint64_t qword)
{
    QWORD_TO_NETWORKBYTES(pBytes, index, qword );
}

uint32_t DRM_Prdy_Cch_Base64_Equiv(size_t cb)
{
    return CCH_BASE64_EQUIV(cb);
}


DRM_Prdy_Error_e DRM_Prdy_License_GetProperty(
        DRM_Prdy_Handle_t pPrdyContext,
        DRM_Prdy_license_property_e licenseProperty,
        const uint8_t *pLicensePropertyExtraData,
        const uint32_t *pLicensePropertyExtraDataSize,
        uint32_t *pLicensePropertyOutputData)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pLicensePropertyOutputData != NULL);

    ChkDR( Drm_License_GetProperty(
            pPrdyContext->pDrmAppCtx,
            (DRM_LICENSE_GET_PROPERTY)licenseProperty,
            (const DRM_BYTE *)pLicensePropertyExtraData,
            (const DRM_DWORD *)pLicensePropertyExtraDataSize,
            (DRM_DWORD *)pLicensePropertyOutputData) );

    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed [0x%X]\n", __FUNCTION__, (unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_LicenseQuery_GetState(
        DRM_Prdy_Handle_t pPrdyContext,
        DRM_Prdy_license_right_e licenseRight,
        DRM_Prdy_license_state_t *pLicenseState)
{
    DRM_RESULT dr = DRM_SUCCESS;
    const DRM_CONST_STRING *rightToQuery[1] = { NULL };
    DRM_LICENSE_STATE_DATA rightState[NO_OF(rightToQuery)];
    uint32_t i;

    BDBG_ASSERT(pPrdyContext != NULL);
    if (pLicenseState == NULL) {
        BDBG_ERR(("%s: output buffer is NULL\n", __FUNCTION__));
        return DRM_Prdy_invalid_parameter;
    }

    switch (licenseRight) {
        case eDRM_Prdy_license_right_none:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_NONE;
            break;
        case eDRM_Prdy_license_right_playback:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_PLAYBACK;
            break;
        case eDRM_Prdy_license_right_collaborative_play:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_COLLABORATIVE_PLAY;
            break;
        case eDRM_Prdy_license_right_copy_to_cd:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_COPY_TO_CD;
            break;
        case eDRM_Prdy_license_right_copy:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_COPY;
            break;
        case eDRM_Prdy_license_right_create_thumbnail_image:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_CREATE_THUMBNAIL_IMAGE;
            break;
        case eDRM_Prdy_license_right_move:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_MOVE;
            break;
        default:
            BDBG_ERR(("%s: Unknown right (%d)\n", __FUNCTION__, licenseRight));
            return DRM_Prdy_invalid_parameter;
    }
    ChkDR( Drm_LicenseQuery_GetState(
                pPrdyContext->pDrmAppCtx,
                rightToQuery,
                NO_OF( rightToQuery ),
                rightState,
                NULL,
                NULL) );

    pLicenseState->dwStreamId = rightState[0].dwStreamId;
    pLicenseState->dwCategory = rightState[0].dwCategory;
    pLicenseState->dwNumCounts = rightState[0].dwStreamId;
    for (i=0; i<pLicenseState->dwNumCounts; i++) {
        pLicenseState->dwCount[i] = rightState[0].dwCount[i];
    }
    pLicenseState->dwNumDates = rightState[0].dwNumDates;
    for (i=0; i<pLicenseState->dwNumDates; i++) {
        pLicenseState->date[i].dwFileTimeHigh = rightState[0].datetime[i].dwHighDateTime;
        pLicenseState->date[i].dwFileTimeLow = rightState[0].datetime[i].dwLowDateTime;
    }
    pLicenseState->dwVague = rightState[0].dwVague;

    return DRM_Prdy_ok;

ErrorExit:
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_SecureClock_GenerateChallenge(
        DRM_Prdy_Handle_t      pPrdyContext,
        wchar_t               *pURL,
        uint32_t              *pURL_len,
        uint8_t               *pCh_data,    /* [out] */
        uint32_t              *pCh_len)
{
    DRM_Prdy_Error_e  result = DRM_Prdy_ok;
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_DWORD         cchURL = 0;
    /*DRM_WCHAR        *pwszURL = NULL;*/
    DRM_DWORD        cchChallenge       = 0;

    BDBG_MSG(("%s:%d - entering.", __FUNCTION__,__LINE__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pURL_len != NULL);
    BDBG_ASSERT(pCh_len != NULL);
    cchURL = (DRM_DWORD) *pURL_len;
    cchChallenge  = (DRM_DWORD)*pCh_len;
    /*
    if( pURL != NULL)
    {
        if( !DRM_Prdy_convertCStringToWString(pURL,(wchar_t *) pwszURL, (uint16_t *) &cchURL))
        {
            goto ErrorExit;
        }
    }
    */

    dr = Drm_SecureClock_GenerateChallenge(
                    pPrdyContext->pDrmAppCtx,
                    (DRM_WCHAR *) pURL,
                    &cchURL,
                    (DRM_BYTE *) pCh_data,
                    &cchChallenge );

    if( dr == DRM_E_BUFFERTOOSMALL )
    {
        BDBG_MSG(("%s: Exiting with Buffer too small...\n", __FUNCTION__));
        result = DRM_Prdy_buffer_size;
    }
    else if( dr != DRM_SUCCESS)
    {
        BDBG_LOG(("%s:%d returning [0x%x]\n", __FUNCTION__,__LINE__,dr));
        BDBG_MSG(("%s:%d returning [0x%x]\n", __FUNCTION__,__LINE__,dr));
        goto ErrorExit;
    }

    *pURL_len = cchURL;
    *pCh_len = cchChallenge;

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;

ErrorExit:
    BDBG_ERR(("%s:%d failed [0x%x], exiting...\n", __FUNCTION__,__LINE__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_SecureClock_ProcessResponse(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint8_t               *pChResponse,
        uint32_t               pChResponselen)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_RESULT       drResponse = DRM_SUCCESS;
    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pChResponse != NULL);

    BDBG_MSG(("%s - entering", __FUNCTION__));

    ChkDR(Drm_SecureClock_ProcessResponse(
                    pPrdyContext->pDrmAppCtx,
                    pChResponse,
                    pChResponselen,
                    &drResponse ));

    if ( drResponse != DRM_SUCCESS )
    {
       dr = drResponse;
       ChkDR( drResponse);
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;

}

DRM_Prdy_Error_e DRM_Prdy_SecureClock_GetStatus(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint32_t              *pStatus )
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_DWORD         cchSecTime         = 0;
    DRM_WCHAR        *pwszSecTime        = NULL;
    DRM_BYTE         *pbTimeStatus       = NULL;
    DRM_DWORD         cbTimeStatus       = 0;
    DRM_DWORD         dwFlag             = 0;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pStatus != NULL);

    dr = Drm_SecureClock_GetValue( pPrdyContext->pDrmAppCtx, pwszSecTime, &cchSecTime, &dwFlag, pbTimeStatus, &cbTimeStatus );

    if ( dr != DRM_E_BUFFERTOOSMALL )
    {
        goto ErrorExit;
    }

    ChkMem( pwszSecTime = (DRM_WCHAR*) Oem_MemAlloc( cchSecTime * SIZEOF( DRM_WCHAR ) ) );
    ChkMem( pbTimeStatus = (DRM_BYTE*) Oem_MemAlloc( cbTimeStatus ) );
    MEMSET( pwszSecTime, 'a', cchSecTime * SIZEOF( DRM_WCHAR ) );
    MEMSET( pbTimeStatus, 'b', cbTimeStatus );

    ChkDR( Drm_SecureClock_GetValue( pPrdyContext->pDrmAppCtx, pwszSecTime, &cchSecTime, &dwFlag, pbTimeStatus, &cbTimeStatus ) );
    *pStatus = dwFlag;

ErrorExit:

    if( pwszSecTime != NULL )
    {
        Oem_MemFree( pwszSecTime );
    }

    if( pbTimeStatus != NULL )
    {
        Oem_MemFree( pbTimeStatus );
    }

    if( dr != DRM_SUCCESS)
    {
        BDBG_ERR(("%s: failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));

        return DRM_Prdy_fail;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));

    return DRM_Prdy_ok;

}

void DRM_Prdy_GetSystemTime(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint16_t              *pYear,
        uint16_t              *pMonth,
        uint16_t              *pDayOfWeek,
        uint16_t              *pDay,
        uint16_t              *pHour,
        uint16_t              *pMinute,
        uint16_t              *pSecond,
        uint16_t              *pMilliseconds)
{
    DRMSYSTEMTIME     systemTime;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pYear != NULL);
    BDBG_ASSERT(pMonth != NULL);
    BDBG_ASSERT(pDayOfWeek != NULL);
    BDBG_ASSERT(pDay != NULL);
    BDBG_ASSERT(pHour != NULL);
    BDBG_ASSERT(pMinute != NULL);
    BDBG_ASSERT(pSecond != NULL);
    BDBG_ASSERT(pMilliseconds != NULL);

    Oem_Clock_GetSystemTime( pPrdyContext->pOEMContext, &systemTime);

    *pYear = systemTime.wYear;
    *pMonth = systemTime.wMonth;
    *pDayOfWeek = systemTime.wDayOfWeek;
    *pDay = systemTime.wDay;
    *pHour = systemTime.wHour;
    *pMinute = systemTime.wMinute;
    *pSecond = systemTime.wSecond;
    *pMilliseconds = systemTime.wMilliseconds;

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));

}

void DRM_Prdy_SetSystemTime(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint16_t               year,
        uint16_t               month,
        uint16_t               dayOfWeek,
        uint16_t               day,
        uint16_t               hour,
        uint16_t               minute,
        uint16_t               second,
        uint16_t               milliseconds)
{
    DRMSYSTEMTIME     systemTime;
    DRM_APP_CONTEXT_INTERNAL    *poAppContextInternal = ( DRM_APP_CONTEXT_INTERNAL * )pPrdyContext->pDrmAppCtx;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    systemTime.wYear         = year;
    systemTime.wMonth        = month;
    systemTime.wDayOfWeek    = dayOfWeek;
    systemTime.wDay          = day;
    systemTime.wHour         = hour;
    systemTime.wMinute       = minute;
    systemTime.wSecond       = second;
    systemTime.wMilliseconds = milliseconds;

#ifdef CMD_DRM_PLAYREADY_SAGE_IMPL
    BDBG_MSG(("%s:%d - Calling Oem_Clock_SetSystemTime with BBX context %x\n",__FUNCTION__,__LINE__,&poAppContextInternal->oBlackBoxContext));
    Oem_Clock_SetSystemTime( &poAppContextInternal->oBlackBoxContext, &systemTime);
    poAppContextInternal->fClockSet = true;
#else
    Oem_Clock_SetSystemTime( pPrdyContext->pOEMContext, &systemTime);
#endif

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
}

/***********************************************************************************
 * Function: DRM_Prdy_Reader_Unbind()
  ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Unbind(
        DRM_Prdy_Handle_t           pPrdyContext,
        DRM_Prdy_DecryptContext_t  *pDecryptContext )
{
    DRM_RESULT             dr;

    BDBG_MSG(("%s - entering", __FUNCTION__));
    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pDecryptContext != NULL);
    BDBG_ASSERT(pDecryptContext->pDecrypt  != NULL);

    dr = Drm_Reader_Unbind(
            pPrdyContext->pDrmAppCtx,
            (DRM_DECRYPT_CONTEXT *) pDecryptContext->pDecrypt);

    if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);

    SAFE_OEM_FREE(pDecryptContext->pDecrypt);
    pDecryptContext->pDecrypt = NULL;

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_SupportSecureStop()
 ***********************************************************************************/
bool DRM_Prdy_SupportSecureStop( void )
{
    return Drm_SupportSecureStop();
}

/***********************************************************************************
 * Function: DRM_Prdy_TurnSecureStop()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_TurnSecureStop(
        DRM_Prdy_Handle_t pPrdyContext,
        bool inOnOff)
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_TurnSecureStop(pPrdyContext->pDrmAppCtx, inOnOff);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_GetSecureStopIds()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetSecureStopIds(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint8_t                pSessionIDs[DRM_PRDY_MAX_NUM_SECURE_STOPS][DRM_PRDY_SESSION_ID_LEN],
        uint32_t              *pCount )
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_GetSecureStopIds(
            pPrdyContext->pDrmAppCtx,
            pSessionIDs,
            pCount);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_GetSecureStop()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetSecureStop(
        DRM_Prdy_Handle_t       pPrdyContext,
        uint8_t                *pSessionID,
        uint8_t                *pSecureStopData,
        uint16_t               *pSecureStopLen )
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_GetSecureStop(
            pPrdyContext->pDrmAppCtx,
            pSessionID,
            pSecureStopData,
            pSecureStopLen);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_CommitSecureStop()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_CommitSecureStop(
        DRM_Prdy_Handle_t       pPrdyContext,
        uint8_t                *pSessionID )
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_CommitSecureStop(pPrdyContext->pDrmAppCtx, pSessionID);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_ResetSecureStops()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ResetSecureStops(
        DRM_Prdy_Handle_t     pPrdyContext,
        uint16_t             *pCount )
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_ResetSecureStops(pPrdyContext->pDrmAppCtx, pCount);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_DeleteSecureStore()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_DeleteSecureStore(void)
{
	DRM_RESULT 			dr;

	dr = Drm_DeleteSecureStore(&sDstrHDSPath);

	return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_GetSecureStoreHash()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetSecureStoreHash(
	uint8_t				*pSecureStoreHash )
{
	DRM_RESULT 			dr;

	dr = Drm_GetSecureStoreHash(&sDstrHDSPath, pSecureStoreHash);

	return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_DeleteKeyStore()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_DeleteKeyStore(
	DRM_Prdy_Handle_t 	pPrdyContext )
{
	DRM_RESULT 			dr;

	if (pPrdyContext != NULL)
		dr = Drm_DeleteKeyStore(pPrdyContext->pDrmAppCtx);
	else
		dr = Drm_DeleteKeyStore(NULL);

	return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_GetKeyStoreHash()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetKeyStoreHash(
		DRM_Prdy_Handle_t 	pPrdyContext,
		uint8_t				*pSecureStoreHash )
{
	DRM_RESULT 			dr;

	if (pPrdyContext != NULL)
		dr = Drm_GetKeyStoreHash(pPrdyContext->pDrmAppCtx, pSecureStoreHash);
	else
		dr = Drm_GetKeyStoreHash(NULL, pSecureStoreHash);

	return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_Clock_GetSystemTime()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Clock_GetSystemTime(
		DRM_Prdy_Handle_t 	pPrdyContext,
        uint8_t            *pSystemTime)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pSystemTime != NULL);

    ChkDR( Drm_Clock_GetSystemTime(
                pPrdyContext->pDrmAppCtx,
                (DRM_UINT64 *)pSystemTime) );

    if(dr != DRM_SUCCESS) {
       goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Operation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}
