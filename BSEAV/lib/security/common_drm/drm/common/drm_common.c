/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <pthread.h>
#include <string.h>

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_memory.h"
#include "nexus_platform_client.h"
#include "nexus_random_number.h"

#if (NEXUS_SECURITY_API_VERSION==1)
#include "nexus_otpmsp.h"
#else
#include "nexus_otp_msp.h"
#include "nexus_otp_key.h"
#include "nexus_otp_msp_indexes.h"
#endif

#include "nexus_base_os.h"

#include "bcrypt.h"
#include "bcrypt_sha1_sw.h"
#include "bcrypt_sha256_sw.h"
#include "bcrypt_ecdsa_sw.h"
#include "bcrypt_aescbc_sw.h"
#include "bcrypt_aesecb_sw.h"
#include "bcrypt_cmac_sw.h"
#include "bcrypt_descbc_sw.h"
#include "bcrypt_desecb_sw.h"
#include "bcrypt_dsa_sw.h"
#include "bcrypt_md5_sw.h"
#include "bcrypt_rc4_sw.h"
#include "bcrypt_rng_sw.h"
#include "bcrypt_rsa_sw.h"
#include "bcrypt_x509_sw.h"
#include "bcrypt_dh_sw.h"

#include "drm_common.h"
#include "drm_common_priv.h"
#ifdef NONSAGE_CREDENTIAL_CAPABLE
#include "drm_key_region.h"
#endif
#include "drm_common_swcrypto.h"
#include "drm_key_binding.h"

static BCRYPT_Handle g_bcrypt_handle = 0x00;
static BKNI_MutexHandle drmCommonMutex = 0;
static DrmCommonSha1_t drmSha1;
static NEXUS_MemoryAllocationSettings memorySettings;
static int DrmCommon_InitCounter = 0;
static bool bUseExternalHeap = false;

/* Private functions for SW crypto */
static int DRM_Common_P_CheckAndAssignContext(
    uint32_t *return_context,
    uint32_t thread_id);

static int DRM_Common_P_CheckAndFreeContext(
    uint32_t *return_context,
    uint32_t thread_id);

/* OTP  */
static DrmRC DRM_Common_P_DetermineAskmMode(void);
static uint8_t askmMode = 0;
static uint8_t askmProcInForKey3[16] = {0x00};
static uint8_t askmProcInForKey4[16] = {0x00};

#define MAX_DMA_BLOCKS 5

/* Add module to BDBG */
BDBG_MODULE(drm_common);

DrmRC DrmAssertParam(
    char* param_string,
    uint32_t value,
    uint32_t max)
{
    DrmRC rc = Drm_Success;

    if(value >= max)
    {
        BDBG_ERR(("%s - Invalid '%s' value detected. '%u' exceeds enum limit of '%u'", BSTD_FUNCTION, param_string, value, max));
        rc = Drm_InvalidParameter;
    }

    return rc;
}

/****
 * As decided on 04/10/2012 there will be a multi process + multi thread (MPMT)
 *  CommonCrypto handle table.
 * Each method, which requires a CommonCrypto handle will fetch it from there (MPMTT)
 * At the moment that's implemented in the following set of methods, which is
 * more like
 *  a singleton: i.e. if needed creates the CommonCrypto handle and returns it.
 ****/
static DRM_Common_Handle drmCmnHnd;

DrmRC DRM_Common_AcquireHandle(DRM_Common_Handle* drmHnd)
{
    DrmRC drmRc = Drm_Success;
    if (!drmCmnHnd) {
        DRM_Common_Handle hnd = BKNI_Malloc(sizeof(DRM_Common_Settings)); /* or sizeof(*DRM_Common_Handle) */
        if (hnd) {
            const CommonCryptoSettings* cryptoSettings = NULL;
            BKNI_Memset(hnd, 0, sizeof(DRM_Common_Settings));
            hnd->cryptHnd = CommonCrypto_Open(cryptoSettings);
            if (hnd->cryptHnd) {
                hnd->pid = getpid();
#if defined(ANDROID)
                hnd->tid = gettid();
#else
                hnd->tid = syscall(SYS_gettid);
#endif
                hnd->drmRc = Drm_Success;
                drmCmnHnd = hnd;
            }
            else drmRc = Drm_CryptoOpenErr;
        }
        else drmRc = Drm_MemErr;
    }
    *drmHnd = drmCmnHnd;
    return drmRc;
}

DrmRC DRM_Common_GetHandle(DRM_Common_Handle* drmHnd)
{
    *drmHnd = drmCmnHnd;
    return *drmHnd ? Drm_Success : Drm_NotFound;
}

DrmRC DRM_Common_CloseHandle(void)
{
    DRM_Common_Handle drmHnd;
    DrmRC drmRc = DRM_Common_GetHandle(&drmHnd);
    if (Drm_Success == drmRc)
    {
        BDBG_MSG(("%s - Closing handles", BSTD_FUNCTION));
        CommonCrypto_Close(drmHnd->cryptHnd);
        drmHnd->cryptHnd = NULL;
        BKNI_Free(drmHnd);
        drmHnd = NULL;
        drmCmnHnd = NULL;
    }
    return drmRc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_Initialize
 **
 ** DESCRIPTION:
 **   Initializes the Common DRM module with a DRM bin file.
 **   Internally calls DRM_Common_BasicInitialize to create Bcrypt handle,
 **   mutexes, etc...
 **   Called by all DRM_xxxxxxx_Initialize APIs
 **
 ** PARAMETERS:
 **  commonDrmInit - DrmCommonInit_t structure to pass on to DRM_Common_BasicInitialize
 **  key_file - path to valid DRM bin file
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Common_Initialize(
    DrmCommonInit_t *pCommonDrmSettings,
    char *key_file)
{
    DrmRC rc = Drm_Success;

    BDBG_MSG(("%s - Entered function (init = '%d')", BSTD_FUNCTION, DrmCommon_InitCounter));

    rc = DRM_Common_BasicInitialize(pCommonDrmSettings);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error calling 'DRM_Common_BasicInitialize'", BSTD_FUNCTION));
        goto ErrorExit;
    }

#ifdef NONSAGE_CREDENTIAL_CAPABLE
    /* If a bin file is specified load it, otherwise
     * KeyRegion will read it from flash */
    if(key_file != NULL){
        DRM_KeyRegion_SetCustDrmFilePath(key_file);
    }

    BDBG_MSG(("%s - Calling 'DRM_KeyRegion_Init' ...", BSTD_FUNCTION));
    rc = DRM_KeyRegion_Init();
#endif

 ErrorExit:
    BDBG_MSG(("%s - Exiting function (init = '%d')", BSTD_FUNCTION, DrmCommon_InitCounter));
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_BasicInitialize
 **
 ** DESCRIPTION:
 **   Initialize the basic Common DRM functionality (no DRM bin file required)
 **
 ** PARAMETERS:
 **   pCommonDrmSettings - Contains a NEXUS_HeapHandle to be used during memory
 **                        allocation operations
 **
 ** RETURNS:
 **   Drm_Success or other
 **
 ******************************************************************************/
DrmRC DRM_Common_BasicInitialize(
    DrmCommonInit_t *pCommonSettings)
{
    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode eCode = BCRYPT_STATUS_eOK;
    int i = 0;
    BERR_Code bErr;

    NEXUS_Memory_GetDefaultAllocationSettings(&memorySettings);

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
    BDBG_WRN(("%s - ******* Initializing Common DRM core (%s) *******", BSTD_FUNCTION, COMMON_DRM_VERSION));

    if (drmCommonMutex != 0 || (bErr = BKNI_CreateMutex(&drmCommonMutex)) == BERR_SUCCESS)
    {
        BDBG_ASSERT(drmCommonMutex != NULL);
        BKNI_AcquireMutex(drmCommonMutex);
        DRM_Common_Handle drmHnd;

        if (DRM_Common_AcquireHandle(&drmHnd) == Drm_Success)
        {
            if(g_bcrypt_handle == 0)     /* g_bcrypt_handle zeroed out only for error condition. Normal case is to leave g_bcrypt_handle open. */
            {
                eCode = BCRYPT_Open(&g_bcrypt_handle);
                if(eCode == BCRYPT_STATUS_eOK)
                {
                    BDBG_MSG(("%s - Opened BCRYPT, g_bcrypt_handle = '0x%lx' ^^^^^^^^^^^^^^^^^^^^^^^", BSTD_FUNCTION, (long unsigned int)g_bcrypt_handle));
                }
                else {
                    BDBG_ERR(("%s - Failed to open BCRYPT, rc = '%d' ^^^^^^^^^^^^^^^^^^^^^^^\n", BSTD_FUNCTION, eCode));
                    g_bcrypt_handle = 0; /* BSTS cleanup */
                    rc = Drm_BcryptErr;
                    goto ErrorExit;
                }
            }

            if (g_bcrypt_handle) {
                /* Initialize thread context table for SHA-1 and Partial SHA-1 */
                for(i=0; i<DRM_COMMON_MAX_THREAD_SUPPORT; i++)
                {
                    drmSha1.thread_table[i] = 0x00;
                }
                drmSha1.curr_free_ctx_num = 0;
            }
        }
        else {
            rc = Drm_InitErr;
            BDBG_MSG(("In %s - DRM_Common_AcquireHandle failed. Set rc to %u but not returning early", BSTD_FUNCTION, rc));
        }
        BKNI_ReleaseMutex(drmCommonMutex);
    }
    else {
        BDBG_ERR(("%s - Failed to create mutex, error: %d", BSTD_FUNCTION, bErr));
        rc = Drm_InitErr;
        goto ErrorExit;
    }

    if(pCommonSettings->heap == NULL)
    {
        BDBG_MSG(("%s - Using default heap", BSTD_FUNCTION));
        memorySettings.heap = NULL;
        bUseExternalHeap = false;
    }
    else
    {
        BDBG_MSG(("%s - Using heap handle specified (0x%lx)", BSTD_FUNCTION, (long unsigned int)pCommonSettings->heap));
        memorySettings.heap = pCommonSettings->heap;
        bUseExternalHeap = true;
    }

    rc = DRM_Common_P_DetermineAskmMode();

    DrmCommon_InitCounter++;
    BDBG_MSG(("%s - incrementing drm common init counter to (0x%08x)", BSTD_FUNCTION, DrmCommon_InitCounter));

 ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_Finalize
 **
 ** DESCRIPTION:
 **   Close the DRM_Common module
 **
 ** PARAMETERS:
 **   void
 **
 ** RETURNS:
 **   Drm_Success or other
 **
 ******************************************************************************/
DrmRC DRM_Common_Finalize()
{
    DrmRC rc = Drm_Success;
    int i = 0;

    BDBG_MSG(("%s - Entered function (init = '%d')", BSTD_FUNCTION, DrmCommon_InitCounter));

    /* Sanity check */
    if(DrmCommon_InitCounter <= 0)
    {
        BDBG_MSG(("%s - DrmCommon_InitCounter value is invalid ('%d').  Possible bad thread exit or PMC finalize", BSTD_FUNCTION, DrmCommon_InitCounter));
        return Drm_InvalidParameter;
    }

    /* If there's one DRM module left calling DRM_Common_Finalize, clean everything up
     * Otherwise skip the clean up and decrement the counter */
    if(DrmCommon_InitCounter == 1)
    {
        BDBG_MSG(("%s - Cleaning up Common DRM parameters ***************************", BSTD_FUNCTION));

        if (drmCommonMutex)
            BKNI_AcquireMutex(drmCommonMutex);

        if (g_bcrypt_handle) {
            BCRYPT_Close(g_bcrypt_handle);
            g_bcrypt_handle = 0;
        }

        /* UnInitialize thread context table for SHA-1 and Partial SHA-1 */
        for(i=0; i<DRM_COMMON_MAX_THREAD_SUPPORT; i++)
        {
            drmSha1.thread_table[i] = 0x00;
        }
        drmSha1.curr_free_ctx_num = 0;

#ifdef NONSAGE_CREDENTIAL_CAPABLE
        rc = DRM_KeyRegion_UnInit();
        if(rc != Drm_Success){
            BDBG_ERR(("%s - Error uninitializing keyregion", BSTD_FUNCTION));
        }
#endif

        rc = DRM_Common_CloseHandle();
        if(rc != Drm_Success){
            BDBG_ERR(("%s - Error closing handle", BSTD_FUNCTION));
        }

        if (drmCommonMutex) {
            BKNI_ReleaseMutex(drmCommonMutex);
            BKNI_DestroyMutex(drmCommonMutex);
            drmCommonMutex = 0;
        }
    }

    DrmCommon_InitCounter--;
    BDBG_MSG(("%s - Exiting function (init = '%d')", BSTD_FUNCTION, DrmCommon_InitCounter));

    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_AllocKeySlot
 **
 ** DESCRIPTION:
 **   Helper function to allocate a Nexus keyslot and at the same time hide
 **         Nexus details from higher level callers
 **
 ** PARAMETERS:
 **   NEXUS_SecurityEngine securityEngine - security engine for keyslot allocation
 **   NEXUS_KeySlotHandle* keySlotHandle - address of a keyslot handle
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful, Drm_NexusErr error otherwise.
 **
 ******************************************************************************/
DrmRC DRM_Common_AllocKeySlot(NEXUS_SecurityEngine securityEngine,
                              NEXUS_KeySlotHandle* keySlotHandle)
{
    /**************************************************************************/
    /*          Allocate and configure keyslot                                */
    /**************************************************************************/
#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_SecurityKeySlotSettings keySettings;

    BDBG_MSG(("In function %s", BSTD_FUNCTION));

    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = securityEngine; /* NEXUS_SecurityEngine_eM2m,
                                                   NEXUS_SecurityEngine_eCa */
    *keySlotHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
#else
    NEXUS_KeySlotAllocateSettings keySettings;
    BSTD_UNUSED(securityEngine);

    BDBG_MSG(("In function %s", BSTD_FUNCTION));

    NEXUS_KeySlot_GetDefaultAllocateSettings(&keySettings);
    keySettings.useWithDma = true;
    keySettings.owner = NEXUS_SecurityCpuContext_eHost;
    keySettings.slotType = NEXUS_KeySlotType_eIvPerBlock;

    *keySlotHandle = NEXUS_KeySlot_Allocate(&keySettings);
#endif
    return NULL != *keySlotHandle ? Drm_Success : Drm_NexusErr; /*ToDDD: is Drm_NexusErr good enough? */

}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_FreeKeySlot
 **
 ** DESCRIPTION:
 **   The DRM_Common_AllocKeySlot counterpart - see above
 **
 ******************************************************************************/
void DRM_Common_FreeKeySlot(NEXUS_KeySlotHandle keySlotHandle)
{
    BDBG_ASSERT(keySlotHandle != NULL);
    if (keySlotHandle){
#if (NEXUS_SECURITY_API_VERSION==1)
        NEXUS_Security_FreeKeySlot(keySlotHandle);
#else
        NEXUS_KeySlot_Free(keySlotHandle);
#endif
    }

    return;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_GetDefaultStructSettings
 **
 ** DESCRIPTION:
 **   Retrieve default settings for the DrmCommonOperationStruct_t structure
 **
 ******************************************************************************/
void DRM_Common_GetDefaultStructSettings(DrmCommonOperationStruct_t *pDrmCommonOpStruct)
{
    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
    BDBG_ASSERT(pDrmCommonOpStruct != NULL);

    BKNI_Memset((uint8_t *)pDrmCommonOpStruct, 0x00, sizeof(DrmCommonOperationStruct_t) );

    CommonCrypto_GetDefaultKeyConfigSettings(&pDrmCommonOpStruct->keyConfigSettings);

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_OperationDma
 **
 ** DESCRIPTION:
 **   Performs both the key loading and DMA operation given the settings of the
 **   'DrmCommonOperationStruct_t' structure passed the function. Unlike
 **   'DRM_Common_KeyConfigOperation' and 'DRM_Common_M2mOperation', this function
 **   should be used in the case where the caller does not care about having access
 **   to the 'NEXUS_KeySlotHandle'.
 **
 ** PARAMETERS:
 **   pDrmCommonOpStruct* [in/out] - Structure containing key source and
 **                                  cryptographic operation to perform
 **
 ** RETURNS:
 **   Drm_Success (Success) or any other error code (Failure)
 **
 ******************************************************************************/
DrmRC DRM_Common_OperationDma(DrmCommonOperationStruct_t *pDrmCommonOpStruct)
{
    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
    DrmRC drmRc = DRM_Common_AllocKeySlot(NEXUS_SecurityEngine_eM2m,
                                          &pDrmCommonOpStruct->keyConfigSettings.keySlot);
    if (drmRc == Drm_Success)
    {
        drmRc = DRM_Common_KeyConfigOperation(pDrmCommonOpStruct);
        if (drmRc == Drm_Success)
        {
            drmRc = DRM_Common_M2mOperation(pDrmCommonOpStruct);
        }
        BDBG_MSG(("%s - Freeing keyslot", BSTD_FUNCTION));
#if (NEXUS_SECURITY_API_VERSION==1)
        NEXUS_Security_FreeKeySlot(pDrmCommonOpStruct->keyConfigSettings.keySlot);
#else
        NEXUS_KeySlot_Free(pDrmCommonOpStruct->keyConfigSettings.keySlot);
#endif
    }
    else {
        BDBG_ERR(("%s - Allocate keySlot failed ", BSTD_FUNCTION));
        drmRc = Drm_MemErr;
    }
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return drmRc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_KeyConfigOperation
 **
 ** DESCRIPTION:
 **   Performs the key loading operation given the settings of the
 **   'DrmCommonOperationStruct_t' structure passed the function. Unlike
 **   'DRM_Common_OperationDma', this function should be used in the
 **   case where the caller would like to have access to the
 **   to the 'NEXUS_KeySlotHandle'.
 **
 ** PARAMETERS:
 **   pDrmCommonOpStruct* [in/out] - Structure containing key source and
 **                                  cryptographic operation to perform
 **
 ** RETURNS:
 **   Drm_Success (Success) or any other error code (Failure)
 **
 ******************************************************************************/
DrmRC DRM_Common_KeyConfigOperation(DrmCommonOperationStruct_t *pDrmCommonOpStruct)
{
    BDBG_MSG(("In function %s", BSTD_FUNCTION));
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);
    DRM_Common_Handle drmHnd;
    NEXUS_Error nxs_rc = NEXUS_SUCCESS;
    DrmRC rc = DRM_Common_GetHandle(&drmHnd);
    if (Drm_Success == rc)
    {
#if (COMMON_CRYPTO_ZEUS_VERSION == 42)
        BDBG_MSG(("%s:COMMON_CRYPTO_ZEUS_VERSION  = %d",BSTD_FUNCTION,COMMON_CRYPTO_ZEUS_VERSION  ));
        if((pDrmCommonOpStruct->keyIvSettings.ivSize != 0)&&(pDrmCommonOpStruct->keyIvSettings.keySize==0))
        {
            pDrmCommonOpStruct->byPassKeyConfig= true;
        }
        else
        {
            pDrmCommonOpStruct->byPassKeyConfig= false;
        }
#endif

        if (!pDrmCommonOpStruct->byPassKeyConfig)
        {
            nxs_rc = CommonCrypto_LoadKeyConfig(drmHnd->cryptHnd, &pDrmCommonOpStruct->keyConfigSettings);
        }
        else
        {
            BDBG_MSG(("%s:Skipping Key config",BSTD_FUNCTION));
        }

        /* Config the keyslot */
        if( nxs_rc == NEXUS_SUCCESS)
        {
            /* If there's no sw key or iv to load, assume were performing a route key 4 operation */
            if((pDrmCommonOpStruct->keyIvSettings.ivSize == 0) && (pDrmCommonOpStruct->keyIvSettings.keySize == 0))
            {
                CommonCryptoCipheredKeySettings cipheredKeySettings;

                /* Set default values */
                CommonCrypto_GetDefaultCipheredKeySettings(&cipheredKeySettings);

                BDBG_MSG(("%s - Configuring/loading HW based key pKeySlot = %p", BSTD_FUNCTION, (void *)pDrmCommonOpStruct->keyConfigSettings.keySlot));

                cipheredKeySettings.keySlot = pDrmCommonOpStruct->keyConfigSettings.keySlot;
#if (NEXUS_SECURITY_API_VERSION==1)
                cipheredKeySettings.keySlotType = pDrmCommonOpStruct->keyConfigSettings.settings.keySlotType;

#ifndef COMMON_DRM_LEGACY_MODE
                if (pDrmCommonOpStruct->keySrc == CommonCrypto_eOtpKey)
                {
                    if(askmMode == 0x03 || askmMode == 0x02 || askmMode == 0x00)
                    {
                        BDBG_MSG(("case 1--%s - ASKM mode is 0x%02x*******************************", BSTD_FUNCTION,askmMode));
                        pDrmCommonOpStruct->pKeyLadderInfo->askmSupport = false;
                        cipheredKeySettings.settings.askmSupport = false;
                        cipheredKeySettings.keySrc = pDrmCommonOpStruct->keySrc;
                        cipheredKeySettings.settings.swizzleType   =  NEXUS_SecuritySwizzleType_eNone;
                    }
                    else if(askmMode == 0x01)
                    {
                        BDBG_MSG(("case 2--%s - ASKM mode is 0x%02x *******************************", BSTD_FUNCTION,askmMode));
                        cipheredKeySettings.settings.askmSupport = true;
                        pDrmCommonOpStruct->pKeyLadderInfo->askmSupport = true;
                        cipheredKeySettings.settings.swizzleType   =  NEXUS_SecuritySwizzleType_eNone;
                        cipheredKeySettings.keySrc = pDrmCommonOpStruct->keySrc;
                    }
                } else if (pDrmCommonOpStruct->keySrc == CommonCrypto_eCustKey)
                {
                    BDBG_MSG(("case 3--%s - ASKM mode is 0x%02x *******************************", BSTD_FUNCTION,askmMode));
                    cipheredKeySettings.keySrc = CommonCrypto_eCustKey;
                    cipheredKeySettings.settings.swizzleType   =  NEXUS_SecuritySwizzleType_eSwizzle0;
                } else
                {
                    BDBG_ERR(("case 4--%s - invalid parameters *******************************", BSTD_FUNCTION));
                    rc = Drm_CryptoConfigErr;
                }
#else /* COMMON_DRM_LEGACY_MODE - For compatibility with URSR 12.4-16.4 */
                if ((askmMode == 0x01) && (pDrmCommonOpStruct->keySrc == CommonCrypto_eOtpKey))
                {
                    BDBG_MSG(("%s - ASKM ENABLED, overriding variables *******************************", BSTD_FUNCTION));
                    cipheredKeySettings.keySrc = CommonCrypto_eCustKey;
                    BKNI_Memcpy(pDrmCommonOpStruct->pKeyLadderInfo->procInForKey3, askmProcInForKey3, 16);
                    BKNI_Memcpy(pDrmCommonOpStruct->pKeyLadderInfo->procInForKey4, askmProcInForKey4, 16);
                    pDrmCommonOpStruct->pKeyLadderInfo->custKeySelect = 0x35;
                    pDrmCommonOpStruct->pKeyLadderInfo->keyVarHigh = 0xC9;
                    pDrmCommonOpStruct->pKeyLadderInfo->keyVarLow = 0xC6;
                }
                else
                {
                    cipheredKeySettings.keySrc = pDrmCommonOpStruct->keySrc;
                }
#endif
#else /* NEXUS_SECURITY_API_VERSION != 1 */
                if(pDrmCommonOpStruct->keyConfigSettings.settings.opType==NEXUS_CryptographicOperation_eEncrypt)
                {
                    cipheredKeySettings.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpsClear;
                }
                if(pDrmCommonOpStruct->keyConfigSettings.settings.opType==NEXUS_CryptographicOperation_eDecrypt)
                {
                    cipheredKeySettings.keySlotEntryType = NEXUS_KeySlotBlockEntry_eCpdClear;
                }

                if (pDrmCommonOpStruct->keySrc == CommonCrypto_eOtpDirect)
                {
                   if(askmMode == 0x03 || askmMode == 0x02 || askmMode == 0x00)
                   {
                       BDBG_MSG(("case 1--%s - ASKM mode is 0x%02x*******************************", BSTD_FUNCTION,askmMode));
                       pDrmCommonOpStruct->pKeyLadderInfo->askmSupport = false;
                       cipheredKeySettings.settings.askmSupport = false;
                       cipheredKeySettings.keySrc = pDrmCommonOpStruct->keySrc;
                   }
                   else if(askmMode == 0x01)
                   {
                       BDBG_MSG(("case 2--%s - ASKM mode is 0x%02x *******************************", BSTD_FUNCTION,askmMode));
                       cipheredKeySettings.settings.askmSupport = true;
                       pDrmCommonOpStruct->pKeyLadderInfo->askmSupport = true;
                       cipheredKeySettings.keySrc = CommonCrypto_eOtpAskm; /*pDrmCommonOpStruct->keySrc;*/
                   }
                } else if (pDrmCommonOpStruct->keySrc == CommonCrypto_eGlobalKey)
                {
                       BDBG_MSG(("case 3--%s - ASKM mode is 0x%02x *******************************", BSTD_FUNCTION,askmMode));
                       cipheredKeySettings.keySrc = CommonCrypto_eGlobalKey;
                } else
                {
                       BDBG_ERR(("case 4--%s - invalid parameters *******************************", BSTD_FUNCTION));
                       rc = Drm_CryptoConfigErr;
                }
#endif


                /* check if caller wants to override keyladder operations (default should be false) */
                if(pDrmCommonOpStruct->pKeyLadderInfo->overwriteKeyLadderOperation == true)
                {
                    BDBG_MSG(("%s - OVERRIDING KEY LADDER OPERATION VALUES *******************************", BSTD_FUNCTION));
                    cipheredKeySettings.settings.KeyLadderOpStruct.SessionKeyOperation = pDrmCommonOpStruct->pKeyLadderInfo->KeyLadderOpStruct.SessionKeyOperation;
                    cipheredKeySettings.settings.KeyLadderOpStruct.SessionKeyOperationKey2 = pDrmCommonOpStruct->pKeyLadderInfo->KeyLadderOpStruct.SessionKeyOperationKey2;
                    cipheredKeySettings.settings.KeyLadderOpStruct.ControlWordKeyOperation = pDrmCommonOpStruct->pKeyLadderInfo->KeyLadderOpStruct.ControlWordKeyOperation;
                }
#if (NEXUS_SECURITY_API_VERSION==1)
                /* check if caller wants to override virtual keyladder settings (default should be false) */
                if(pDrmCommonOpStruct->pKeyLadderInfo->overwriteVKLSettings == true)
                {
                    BDBG_MSG(("%s - OVERRIDING VIRTUAL KEY LADDER VALUES *******************************", BSTD_FUNCTION));
                    cipheredKeySettings.settings.VirtualKeyLadderSettings.CustSubMode = pDrmCommonOpStruct->pKeyLadderInfo->VirtualKeyLadderSettings.CustSubMode;
                    cipheredKeySettings.settings.VirtualKeyLadderSettings.VklValue = pDrmCommonOpStruct->pKeyLadderInfo->VirtualKeyLadderSettings.VklValue;
                }

                /* If cust key, copy CKS, high, low values */
                if(cipheredKeySettings.keySrc == CommonCrypto_eCustKey)
                {
                    cipheredKeySettings.settings.custKeySelect = pDrmCommonOpStruct->pKeyLadderInfo->custKeySelect;
                    cipheredKeySettings.settings.keyVarHigh = pDrmCommonOpStruct->pKeyLadderInfo->keyVarHigh;
                    cipheredKeySettings.settings.keyVarLow = pDrmCommonOpStruct->pKeyLadderInfo->keyVarLow;
                }
#endif
                /* For HW keys, procIns must always be copied */
                BKNI_Memcpy(cipheredKeySettings.settings.procInForKey3, pDrmCommonOpStruct->pKeyLadderInfo->procInForKey3, 16);
                BKNI_Memcpy(cipheredKeySettings.settings.procInForKey4, pDrmCommonOpStruct->pKeyLadderInfo->procInForKey4, 16);

                if(CommonCrypto_LoadCipheredKey(drmHnd->cryptHnd, &cipheredKeySettings) != NEXUS_SUCCESS)
                {
                    BDBG_ERR(("%s - Error loading ciphered key", BSTD_FUNCTION));
                    rc = Drm_CryptoConfigErr;
                }
            }
            else
            {
                CommonCryptoClearKeySettings clearKeyettings;
                CommonCrypto_GetDefaultClearKeySettings(&clearKeyettings);

                if(pDrmCommonOpStruct->keyIvSettings.keySize != 0){
                    BDBG_MSG(("%s - Configuring/loading SW based key", BSTD_FUNCTION));
                }

                if(pDrmCommonOpStruct->keyIvSettings.ivSize != 0){
                    BDBG_MSG(("%s - Configuring/loading IV", BSTD_FUNCTION));
                }

                clearKeyettings.keySlot = pDrmCommonOpStruct->keyConfigSettings.keySlot;
#if (NEXUS_SECURITY_API_VERSION==1)
                clearKeyettings.keySlotType = pDrmCommonOpStruct->keyConfigSettings.settings.keySlotType;
#else
                clearKeyettings.keySlotEntryType = pDrmCommonOpStruct->keyConfigSettings.settings.keySlotEntryType;
#endif
                clearKeyettings.settings = pDrmCommonOpStruct->keyIvSettings;

                if(CommonCrypto_LoadClearKeyIv(drmHnd->cryptHnd, &clearKeyettings)!= NEXUS_SUCCESS)
                {
                    BDBG_ERR(("%s - Error loading clear key", BSTD_FUNCTION));
                    rc = Drm_CryptoConfigErr;
                }
            }
            if (Drm_Success != rc ){
                BDBG_ERR(("%s - Error routing key to keyslot", BSTD_FUNCTION));
            }
        }
        else {
            BDBG_ERR(("%s - Error Alloc&Config keyslot", BSTD_FUNCTION));
        }
    }
    else {
        BDBG_MSG(("In %s - DRM_Common_AcquireHandle failed", BSTD_FUNCTION));
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_M2mOperation
 **
 ** DESCRIPTION:
 **   Performs the DMA M2M operation given the settings of the
 **   'DrmCommonOperationStruct_t' structure passed the function. Unlike
 **   'DRM_Common_OperationDma', this function should be used in the
 **   case where the caller would like to have access to the
 **   to the 'NEXUS_KeySlotHandle'.
 **
 **   Note: The keyhandle is inside DrmCommonOpStruct.
 **
 ** PARAMETERS:
 **   pDrmCommonOpStruct* [in/out] - Structure containing key source and
 **                                  cryptographic operation to perform
 **
 ** RETURNS:
 **   Drm_Success (Success) or any other error code (Failure)
 **
 ******************************************************************************/
DrmRC DRM_Common_M2mOperation(DrmCommonOperationStruct_t *pDrmCommonOpStruct)
{
    NEXUS_DmaJobBlockSettings jobBlkSettings[MAX_DMA_BLOCKS];

    CommonCryptoJobSettings jobSettings;
    unsigned int i = 0;

    /* The mutex is still protecting the DRM Common Handle (resp CommonCrypto handle) table */
    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DRM_Common_Handle drmHnd;
    DrmRC drmRc = DRM_Common_GetHandle(&drmHnd);;

    if (drmRc == Drm_Success)
    {
        CommonCrypto_GetDefaultJobSettings(&jobSettings);

        /* Data format used when encrypting/decryption data. */
        /* jobSettings.dataFormat; *//* Set to NEXUS_DmaDataFormat_eBlock in CommonCrypto_GetDefaultJobSettings ?! */
        /* Key slot handle to use during the DMA transfer.  NULL(default) if not encrypting or decrypting data*/
        jobSettings.keySlot = pDrmCommonOpStruct->keyConfigSettings.keySlot;

        DmaBlockInfo_t* pDmaBlock = pDrmCommonOpStruct->pDmaBlock;
        for ( i = 0; i < pDrmCommonOpStruct->num_dma_block; i++)
        {
            NEXUS_DmaJob_GetDefaultBlockSettings (&jobBlkSettings[i]);
            jobBlkSettings[i].pSrcAddr = pDmaBlock->pSrcData;
            jobBlkSettings[i].pDestAddr = pDmaBlock->pDstData;
            jobBlkSettings[i].blockSize = pDmaBlock->uiDataSize;
            jobBlkSettings[i].scatterGatherCryptoStart = pDmaBlock->sg_start;
            jobBlkSettings[i].scatterGatherCryptoEnd = pDmaBlock->sg_end;
#if defined(ANDROID)
            jobBlkSettings[i].cached = true;
#endif
            pDmaBlock++;

#if !defined(ANDROID)
            NEXUS_FlushCache(jobBlkSettings[i].pSrcAddr, jobBlkSettings[i].blockSize);
#endif
        }

        jobBlkSettings[0].resetCrypto = true;

        if (CommonCrypto_DmaXfer(drmHnd->cryptHnd,
                                 &jobSettings,
                                 jobBlkSettings,
                                 pDrmCommonOpStruct->num_dma_block) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Error with M2M DMA operation", BSTD_FUNCTION));
            drmRc = Drm_CryptoDmaErr;
            goto ErrorExit;
        }

#if !defined(ANDROID)
        for (i = 0; i < pDrmCommonOpStruct->num_dma_block; i++)
        {
            NEXUS_FlushCache(jobBlkSettings[i].pDestAddr, jobBlkSettings[i].blockSize);
        }
#endif
    }

 ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return drmRc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_GenerateRandomNumber
 **
 ** DESCRIPTION:
 **   Generate a number of random bytes and return them to the caller
 **
 ** PARAMETERS:
 **   numberOfBytes[in] - Number of bytes to generate.  Hardware limitation is 360.
 **   pBuffer[in/out]   - Pointer to buffer that will contain the random bytes.
 **                       Must be large enough to contain numberOfBytes of data.
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_Common_GenerateRandomNumber(
    uint32_t numberOfBytes,
    uint8_t *pBuffer)
{
    DrmRC rc = Drm_Success;

#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_RandomNumberGenerateSettings settings;
    NEXUS_RandomNumberOutput rngOutput;
    NEXUS_Error nxs_rc = NEXUS_SUCCESS;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    if(numberOfBytes > NEXUS_MAX_RANDOM_NUMBER_LENGTH)
    {
        BDBG_ERR(("%s - Desired number of bytes exceeds maximum limit of '%u'.  Call API '%u' times", BSTD_FUNCTION,
                  NEXUS_MAX_RANDOM_NUMBER_LENGTH, (numberOfBytes / NEXUS_MAX_RANDOM_NUMBER_LENGTH) ));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    NEXUS_RandomNumber_GetDefaultGenerateSettings(&settings);
    settings.randomNumberSize = numberOfBytes;

    nxs_rc = NEXUS_RandomNumber_Generate(&settings, &rngOutput);
    if( (nxs_rc != NEXUS_SUCCESS) || (rngOutput.size != numberOfBytes) )
    {
        BDBG_ERR(("%s - Error generating '%u' random bytes (only '%u' bytes returned) ", BSTD_FUNCTION, numberOfBytes, rngOutput.size));
        rc = Drm_CryptoDmaErr;
        goto ErrorExit;
    }
    BKNI_Memcpy(pBuffer, rngOutput.buffer, numberOfBytes);
#else
    NEXUS_Error nxs_rc = NEXUS_SUCCESS;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    nxs_rc = NEXUS_GetRandomNumber(pBuffer, numberOfBytes);
    if( nxs_rc != NEXUS_SUCCESS )
    {
        BDBG_ERR(("%s - Error generating '%u' random bytes  ", BSTD_FUNCTION, numberOfBytes));
        rc = Drm_CryptoDmaErr;
        goto ErrorExit;
    }
#endif


 ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_MemoryAllocate
 **
 ** DESCRIPTION:
 **   Allocates an aligned contiguous block of memory
 **
 **   Example:
 **   uint8_t *pBuf = NULL;
 **   DRM_Common_MemoryAllocate(&pBuf, 32);
 **
 ** PARAMETERS:
 **   pBuffer     - address of a pointer
 **   buffer_size - size of the buffer to allocate
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Common_MemoryAllocate(uint8_t **pBuffer, uint32_t buffer_size)
{
    DrmRC rc = Drm_Success;
    NEXUS_Error nexerr = NEXUS_SUCCESS;
    BDBG_MSG(("%s - Entered function (%p)", BSTD_FUNCTION, (void *)pBuffer));

    if(bUseExternalHeap == true){
        nexerr = NEXUS_Memory_Allocate(buffer_size, &memorySettings,(void **) pBuffer);
    }
    else{
        nexerr = NEXUS_Memory_Allocate(buffer_size, NULL,(void **) pBuffer);
    }

    if(nexerr != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - Error allocating buffer (%u)", BSTD_FUNCTION, bUseExternalHeap));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

 ErrorExit:
    BDBG_MSG(("%s - Exiting function (%p)", BSTD_FUNCTION, (void *)pBuffer));

    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_MemoryFree
 **
 ** DESCRIPTION:
 **   Free a block of memory
 **
 ** PARAMETERS:
 **   pBuffer - address of memory to free
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_Common_MemoryFree(
    uint8_t *pBuffer)
{
    DrmRC rc = Drm_Success;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    NEXUS_Memory_Free(pBuffer);

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    return rc;
}

DrmRC DRM_Common_SwSha1(
    uint8_t *pSrc,
    uint8_t *pDigest,
    uint32_t size)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    int loop = 0;
    BCRYPT_Sha1Sw_t sha1_ctx;
    uint32_t ctx_num = 0;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    if(DRM_Common_P_CheckAndAssignContext(&ctx_num,
                                          (uint32_t)(pthread_self()) ) != 0)
    {
        rc = Drm_BcryptContextErr;
        goto ErrorExit;
    }

    for(loop = 0; loop < 3; loop++)
    {
        switch (loop)
        {
        case 0: /* INIT */
            sha1_ctx.ulctxnum = ctx_num;
            sha1_ctx.binitctx = true;
            sha1_ctx.bfinal = false;
            sha1_ctx.ulSrcDataLenInByte = 0;
            break;

        case 1: /* UPDATE */
            sha1_ctx.ulctxnum = ctx_num;
            sha1_ctx.binitctx = false;
            sha1_ctx.bfinal = false;
            sha1_ctx.pucSrcData = pSrc;
            sha1_ctx.ulSrcDataLenInByte = size;
            break;

        case 2: /* FINALIZE */
            sha1_ctx.ulctxnum = ctx_num;
            sha1_ctx.binitctx = false;
            sha1_ctx.bfinal = true;
            sha1_ctx.pucDigest = pDigest;
            sha1_ctx.ulSrcDataLenInByte = 0;
            break;
        }

        if( BCrypt_Sha1Sw(g_bcrypt_handle, &sha1_ctx) != BCRYPT_STATUS_eOK )
        {
            BDBG_ERR(("%s - Error calling 'BCrypt_Sha1Sw'", BSTD_FUNCTION));
            rc = Drm_BcryptErr;
            goto ErrorExit;
        }
    }/* end of for loop */

 ErrorExit:
    DRM_Common_P_CheckAndFreeContext(&ctx_num, (uint32_t)pthread_self() );

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/**********************************************************
 ** FUNCTION:
 **   DRM_Common_AttachPidChannel
 **
 ** DESCRIPTION:
 **   Attach a pid channel to the content key
 **
 ** RETURN:
 **   Drm_Success or other
 ***********************************************************/
#if (NEXUS_SECURITY_API_VERSION==1)
DrmRC DRM_Common_AttachPidChannel(NEXUS_KeySlotHandle keySlot, uint32_t pidChannel)
#else
DrmRC DRM_Common_AttachPidChannel(NEXUS_KeySlotHandle keySlot, NEXUS_PidChannelHandle pidChannel)
#endif

{
    DrmRC rc = Drm_Success;
    BDBG_MSG(("%s - Entering. (0x%04x)...", BSTD_FUNCTION, pidChannel));
#if (NEXUS_SECURITY_API_VERSION==1)
    if(NEXUS_Security_AddPidChannelToKeySlot(keySlot, pidChannel)!= NEXUS_SUCCESS)
#else
    if(NEXUS_KeySlot_AddPidChannel(keySlot, pidChannel)!= NEXUS_SUCCESS)
#endif
    {
        BDBG_ERR(("%s - Error adding PID channel '%u' to keyslot", BSTD_FUNCTION, pidChannel));
        rc = Drm_NexusErr;
    }

    BDBG_MSG(("%s - ...Exiting", BSTD_FUNCTION));
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_Common_DetachPidChannel
 **
 ** DESCRIPTION:
 **   Detach the pid channel from the content key keyslot
 ******************************************************************************/
#if (NEXUS_SECURITY_API_VERSION==1)
DrmRC DRM_Common_DetachPidChannel(NEXUS_KeySlotHandle keySlot, uint32_t pidChannel)
#else
DrmRC DRM_Common_DetachPidChannel(NEXUS_KeySlotHandle keySlot, NEXUS_PidChannelHandle pidChannel)
#endif
{
    DrmRC rc = Drm_Success;
    BDBG_MSG(("%s - Entering (0x%04x)...", BSTD_FUNCTION, pidChannel));

#if (NEXUS_SECURITY_API_VERSION==1)
    if(NEXUS_Security_RemovePidChannelFromKeySlot(keySlot, pidChannel)!= NEXUS_SUCCESS)
#else
    if(NEXUS_KeySlot_RemovePidChannel(keySlot, pidChannel)!= NEXUS_SUCCESS)
#endif

    {
        BDBG_ERR(("%s - Error removing PID channel '%u' from keyslot", BSTD_FUNCTION, pidChannel));
        rc = Drm_NexusErr;
    }

    BDBG_MSG(("%s - ...Exiting", BSTD_FUNCTION));
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_Common_P_DetermineAskmMode
 **
 ** DESCRIPTION:
 **   Private function used to determine OTP Mapping
 **   Presently it is only used for ASKM but can be expanded for future use.
 **
 ******************************************************************************/
static DrmRC DRM_Common_P_DetermineAskmMode(void)
{
    DrmRC rc = Drm_Success;
    unsigned i = 0;
    drm_chip_info_t chipInfo;

#if (NEXUS_SECURITY_API_VERSION==1)
{
    NEXUS_ReadMspParms readMsp;
    uint8_t arg_buffer[16] = {0x00};
    uint8_t digest[32] = {0x00};
    NEXUS_ReadMspIO mspStruct;
    readMsp.readMspEnum = NEXUS_OtpCmdMsp_eDestinationDisallowKeyA;

    BDBG_MSG(("%s - Entering function ...", BSTD_FUNCTION));

    NEXUS_Security_ReadMSP(&readMsp,&mspStruct);

    BDBG_MSG(("%s - mspDataBuf = 0x%02x 0x%02x 0x%02x 0x%02x \n\n", BSTD_FUNCTION,
              mspStruct.mspDataBuf[0], mspStruct.mspDataBuf[1], mspStruct.mspDataBuf[2], mspStruct.mspDataBuf[3]));

    askmMode = (0x03 & mspStruct.mspDataBuf[3]);
    BDBG_MSG(("%s - askmMode = '0x%02x'", BSTD_FUNCTION, askmMode));

#ifdef COMMON_DRM_LEGACY_MODE
    if(askmMode == 0x1)
#endif
    {
    rc = DRM_Common_FetchDeviceIds(&chipInfo);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error fetching device IDs", BSTD_FUNCTION));
        goto ErrorExit;
    }

    BKNI_Memcpy(arg_buffer, chipInfo.devIdA, 8);
    for(i=8; i<16; i++)
    {
        arg_buffer[i] = chipInfo.devIdA[15-i];
    }

    rc = DRM_Common_SwSha256(arg_buffer, digest, 16);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error computing SHA", BSTD_FUNCTION));
        goto ErrorExit;
    }
    DRM_MSG_PRINT_BUF("digest", digest, 32);
    BKNI_Memcpy(askmProcInForKey3, digest, 16);
    BKNI_Memcpy(askmProcInForKey4, &digest[16], 16);
    }
}
#else /* NEXUS_SECURITY_API_VERSION > 1 */
{
    NEXUS_Error nx_rc = NEXUS_SUCCESS;
    NEXUS_OtpMspRead mspStruct;
    uint8_t arg_buffer[16] = {0x00};
    uint8_t digest[32] = {0x00};
#if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(5,1)
    unsigned index = 120; /*NEXUS_OTPMSP_DESTINATION_DISALLOW_KEY_A;*/
#else
    unsigned index = 37; /*NEXUS_OtpCmdMsp_eDestinationDisallowKeyA;*/
#endif
    uint8_t *mspData = NULL;

    BDBG_MSG(("%s - Entering function ...", BSTD_FUNCTION));

    BKNI_Memset(&mspStruct, 0x00, sizeof(mspStruct));
    nx_rc = NEXUS_OtpMsp_Read( index, &mspStruct);
    if(nx_rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - Error Reading MSP", BSTD_FUNCTION));
        rc = Drm_NexusErr;
        goto ErrorExit;
    }

    mspStruct.data &= mspStruct.data & mspStruct.valid;
    /*BDBG_MSG(("%s - mspStruct.data = 0x%02x\n\n", BSTD_FUNCTION, mspStruct.data));*/

    askmMode = mspStruct.data & 0x03;
    BDBG_MSG(("%s - askmMode = '0x%2x'", BSTD_FUNCTION, askmMode));

    rc = DRM_Common_FetchDeviceIds(&chipInfo);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error fetching device IDs", BSTD_FUNCTION));
        goto ErrorExit;
    }

    BKNI_Memcpy(arg_buffer, chipInfo.devIdA, 8);
    for(i=8; i<16; i++)
    {
         arg_buffer[i] = chipInfo.devIdA[15-i];
    }

    rc = DRM_Common_SwSha256(arg_buffer, digest, 16);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error computing SHA", BSTD_FUNCTION));
        goto ErrorExit;
    }
    DRM_MSG_PRINT_BUF("digest", digest, 32);
    BKNI_Memcpy(askmProcInForKey3, digest, 16);
    BKNI_Memcpy(askmProcInForKey4, &digest[16], 16);
}
#endif

 ErrorExit:
    BDBG_MSG(("%s - ... Exiting function (askmMode = 0x%02x)", BSTD_FUNCTION, askmMode));
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_SwPartialSha1Sw
 **
 ** DESCRIPTION:
 **    Performs a partial SHA-1 on the buffer pointed to by 'pSrc' (of size 'size')and
 **    returns the calculated digest in 'pDigest'
 **
 ** PARAMETERS:
 **   pSrc - pointer to the buffer containing the source data
 **   pDigest - pointer to a buffer of at least 20 bytes that will contain
 **             the returned SHA-1 diagest.
 **   size - size of the data to perform the SHA-1 calculation on
 **   partialSha1Type - specifies whether the current packet to be hashed is
 **                     an 'init', 'middle' or 'finalize' packet.
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 ******************************************************************************/
DrmRC DRM_Common_SwPartialSha1(
    uint8_t *pSrc,
    uint8_t *pDigest,
    uint32_t size,
    DrmCommon_PartialSha1Type partialSha1Type,
    uint32_t *userContext)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_Sha1Sw_t sha1_ctx;

    BDBG_MSG(("%s - Entered function (type = '%d')\n", BSTD_FUNCTION, partialSha1Type));

    switch (partialSha1Type)
    {
    case DrmCommon_PartialSha1Type_Init: /* INIT */
        if(DRM_Common_P_CheckAndAssignContext(userContext,
                                              (uint32_t)(pthread_self()) ) != 0)
        {
            rc = Drm_BcryptContextErr;
            goto ErrorExit;
        }
        BDBG_MSG(("%s - INIT, thread_table[%u] = 0x%08x", BSTD_FUNCTION, (*userContext), (unsigned int)pthread_self()));
        sha1_ctx.ulctxnum = (*userContext);
        sha1_ctx.binitctx = true;
        sha1_ctx.bfinal = false;
        sha1_ctx.ulSrcDataLenInByte = 0;
        break;

    case DrmCommon_PartialSha1Type_Update: /* UPDATE */
        BDBG_MSG(("%s - UPDATE ctx = '%u', tid(0x%08x)", BSTD_FUNCTION, (*userContext), (unsigned int)pthread_self()));
        sha1_ctx.ulctxnum = (*userContext);
        sha1_ctx.binitctx = false;
        sha1_ctx.bfinal = false;
        sha1_ctx.pucSrcData = pSrc;
        sha1_ctx.ulSrcDataLenInByte = size;
        break;

    case DrmCommon_PartialSha1Type_Finalize: /* FINALIZE */
        BDBG_MSG(("%s - FINALIZE ctx = '%u', (tid)0x%08x", BSTD_FUNCTION, (*userContext), (unsigned int)pthread_self()));
        sha1_ctx.ulctxnum = (*userContext);
        sha1_ctx.binitctx = false;
        sha1_ctx.bfinal = true;
        sha1_ctx.pucDigest = pDigest;
        sha1_ctx.ulSrcDataLenInByte = 0;
        break;
    }

    if( BCrypt_Sha1Sw(g_bcrypt_handle, &sha1_ctx) != BCRYPT_STATUS_eOK )
    {
        BDBG_ERR(("%s - Error calling 'BCrypt_Sha1Sw'", BSTD_FUNCTION));
        rc = Drm_BcryptErr;
    }

    switch (partialSha1Type)
    {
    case DrmCommon_PartialSha1Type_Init:
        BDBG_MSG(("%s - Init (after bcrypt call) curr_free_ctx_num = '%u', (tid)0x%08x", BSTD_FUNCTION, drmSha1.curr_free_ctx_num, (unsigned int)pthread_self()));
        break;

    case DrmCommon_PartialSha1Type_Update:
        BDBG_MSG(("%s - Update (after bcrypt call) curr_free_ctx_num = '%u', (tid)0x%08x", BSTD_FUNCTION, drmSha1.curr_free_ctx_num, (unsigned int)pthread_self()));
        break;

    case DrmCommon_PartialSha1Type_Finalize:
        if(DRM_Common_P_CheckAndFreeContext(userContext,
                                            (uint32_t)pthread_self() ) != 0)
        {
            rc = Drm_BcryptContextErr;
            goto ErrorExit;
        }
        BDBG_MSG(("%s - Finalize (after bcrypt call) curr_free_ctx_num = '%u', (tid)0x%08x", BSTD_FUNCTION, drmSha1.curr_free_ctx_num, (unsigned int)pthread_self()));
        break;
    }

 ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

static int DRM_Common_P_CheckAndAssignContext(
    uint32_t *return_context,
    uint32_t thread_id)
{
    int rc = 0;
    uint32_t i = 0;

    if(drmSha1.curr_free_ctx_num == 15)
    {
        rc = -1;
        BDBG_ERR(("%s - Number of contexts supported exceeded ('%u')", BSTD_FUNCTION, drmSha1.curr_free_ctx_num));
        goto ErrorExit;
    }

    if(drmSha1.thread_table[drmSha1.curr_free_ctx_num] != 0)
    {
        BDBG_MSG(("%s - Current slot (%u) is taken, searching/assiging next available slot", BSTD_FUNCTION, drmSha1.curr_free_ctx_num));
        for(i = 0; i<16;i++)
        {
            /* If free slot assign return context, assign thread ID to table */
            if(drmSha1.thread_table[i] == 0)
            {
                BDBG_MSG(("%s - Found free slot at index (%u)", BSTD_FUNCTION, drmSha1.curr_free_ctx_num));
                (*return_context) = i;
                drmSha1.curr_free_ctx_num = i+1;
                drmSha1.thread_table[i] = thread_id;
                break;
            }
        }

        if(i == 16)
        {
            rc = -1;
            BDBG_ERR(("%s - NO FREE SLOT FOUND", BSTD_FUNCTION));
            goto ErrorExit;
        }
    }
    else
    {
        BDBG_MSG(("%s - Free slot at index (%u)", BSTD_FUNCTION, drmSha1.curr_free_ctx_num));
        /* Assign return context, increment static context, assign thread ID to table */
        (*return_context) = drmSha1.curr_free_ctx_num;
        drmSha1.thread_table[(*return_context)] = thread_id;
        drmSha1.curr_free_ctx_num = drmSha1.curr_free_ctx_num+1;
    }

 ErrorExit:
    return rc;
}

static int DRM_Common_P_CheckAndFreeContext(
    uint32_t *return_context,
    uint32_t thread_id)
{
    int rc = 0;
    uint32_t i = 0;
    uint32_t j = 0;

    if(drmSha1.thread_table[(*return_context)] != thread_id)
    {
        BDBG_MSG(("%s - thread_table['%u']->'0x%08x' != '0x%08x'  searching rest of table", BSTD_FUNCTION,
                  (*return_context), drmSha1.thread_table[(*return_context)], thread_id));
        for(i = 0; i<16;i++)
        {
            /* If free slot assign return context, assign thread ID to table */
            if(drmSha1.thread_table[i] == thread_id)
            {
                BDBG_MSG(("%s - Match at index '%u'", BSTD_FUNCTION, i));
                drmSha1.thread_table[i] = 0;

                /* Set 'curr_free_ctx_num' to first available slot */
                for(j=0;j<16;j++){
                    if(drmSha1.thread_table[j] == 0){
                        drmSha1.curr_free_ctx_num = j;
                        goto ErrorExit;
                    }
                }

                if(j == 16){
                    rc = -1;
                    BDBG_ERR(("%s - NO FREE SLOT FOUND...", BSTD_FUNCTION));
                    goto ErrorExit;
                }
            }
        }/* end of 'i' loop */

        if(i == 16)
        {
            rc = -1;
            BDBG_ERR(("%s - No match found", BSTD_FUNCTION));
            goto ErrorExit;
        }
    }
    else
    {
        BDBG_MSG(("%s - Freeing element at index '%u'", BSTD_FUNCTION, (*return_context)));
        drmSha1.thread_table[(*return_context)] = 0;

        /* Set 'curr_free_ctx_num' to first available slot */
        for(j=0;j<16;j++){
            if(drmSha1.thread_table[j] == 0){
                drmSha1.curr_free_ctx_num = j;
                BDBG_MSG(("%s - curr_free_ctx_num '%u'", BSTD_FUNCTION, j));
                goto ErrorExit;
            }
        }
    }

 ErrorExit:
    return rc;
}

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_SwSha256Sw
 **
 ** DESCRIPTION:
 **    Performs a SHA-256 on the buffer pointed to by 'pSrc' (of size 'size')and
 **    returns the calculated digest in 'pDigest'
 **
 ** PARAMETERS:
 **   pSrc - pointer to the buffer containing the source data
 **   pDigest - pointer to a buffer of at least 32 bytes that will contain
 **             the returned SHA-256 diagest.
 **   size - size of the data to perform the SHA-256 calculation on
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Common_SwSha256(
    uint8_t *pSrc,
    uint8_t *pDigest,
    uint32_t size)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);
    DrmRC rc = Drm_Success;
    BCRYPT_Sha256Sw_t sha256_ctx;
    int ctx_num = 1;
    BCRYPT_STATUS_eCode eCode = BCRYPT_STATUS_eOK;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    sha256_ctx.ulctxnum = ctx_num;
    sha256_ctx.binitctx = true;
    sha256_ctx.bfinal = true;
    sha256_ctx.pucDigest = pDigest;
    sha256_ctx.pucSrcData = pSrc;
    sha256_ctx.ulSrcDataLenInByte = size;
    eCode = BCrypt_Sha256Sw (g_bcrypt_handle, &sha256_ctx);

    BDBG_MSG(("%s - Exiting function with BCrypt_Sha256Sw (0x%08x)", BSTD_FUNCTION, eCode));

    BKNI_ReleaseMutex(drmCommonMutex);

    return rc;
}

/*************************************************************************************/
/*              ECDSA Functions                                                      */
/*************************************************************************************/
DrmRC DRM_Common_SwEcdsaVerify(
    DrmCommon_ECDSASw_Verify_t *inoutp_ecdsaSwIO)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_ECDSASw_Verify_t     *pBcrypt_ecdsaSwIO = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_ecdsaSwIO = (BCRYPT_ECDSASw_Verify_t*)inoutp_ecdsaSwIO;

    errCode = BCrypt_ECDSAVerifySw(g_bcrypt_handle, pBcrypt_ecdsaSwIO);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error verifying ECDSA signature (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - sigIsValid = '%d'", BSTD_FUNCTION, inoutp_ecdsaSwIO->sigIsValid));

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_SwEcdsaSign(
    DrmCommon_ECDSASw_Sign_t *inoutp_ecdsaSwIO)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_ECDSASw_Sign_t   *pBcrypt_ecdsaSwIO = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_ecdsaSwIO = (BCRYPT_ECDSASw_Sign_t*)inoutp_ecdsaSwIO;

    errCode = BCrypt_ECDSASignSw(g_bcrypt_handle, pBcrypt_ecdsaSwIO);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error signing ECDSA signature (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_SwEcdsaMultiply(
    DrmCommon_ECDSASw_Multiply_t *inoutp_ecdsaSwIO)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_ECDSASw_Multiply_t   *pBcrypt_ecdsaSwIO = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_ecdsaSwIO = (BCRYPT_ECDSASw_Multiply_t*)inoutp_ecdsaSwIO;

    errCode = BCrypt_ECDSAMultiplySw(g_bcrypt_handle, pBcrypt_ecdsaSwIO);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error multiplying ECDSA scalar (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/********************************************************************************/
/*                      AES Functionality                                       */
/********************************************************************************/
DrmRC DRM_Common_SwAesEcb(
    DrmCommon_AesEcbSw_t *inoutp_aesecbSwIO)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_AESECBSwCtrl_t *pBcrypt_aesecbSwIO = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_aesecbSwIO = (BCRYPT_S_AESECBSwCtrl_t*)inoutp_aesecbSwIO;

    errCode = BCrypt_AESECBSw(g_bcrypt_handle, pBcrypt_aesecbSwIO);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error AES-ECB SW (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_SwAesCbc(
    DrmCommon_AesCbcSw_t *inoutp_aescbcSwIO)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_AESCBCSwCtrl_t *pBcrypt_aescbcSwIO = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_aescbcSwIO = (BCRYPT_S_AESCBCSwCtrl_t*)inoutp_aescbcSwIO;

    errCode = BCrypt_AESCBCSw(g_bcrypt_handle, pBcrypt_aescbcSwIO);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error AES-CBC SW (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/********************************************************************************/
/*                      CMAC Functionality                                      */
/********************************************************************************/
DrmRC DRM_Common_SwCmac(
    DrmCommon_CmacSw_t *inoutp_cmacSwIO)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_CMACSwParam_t *pBcrypt_cmacSwIO = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_cmacSwIO = (BCRYPT_S_CMACSwParam_t*)inoutp_cmacSwIO;

    errCode = BCrypt_CMACSw(g_bcrypt_handle, pBcrypt_cmacSwIO);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error CMAC SW (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/********************************************************************************/
/*                      DES-xxx Functionality                                   */
/********************************************************************************/
DrmRC DRM_Common_SwDesEcb(
    DrmCommon_DesEcbSw_t *inoutp_desEcbSw)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_DESECBSwCtrl_t *pBcrypt_desEcbSw = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_desEcbSw = (BCRYPT_S_DESECBSwCtrl_t*)inoutp_desEcbSw;

    errCode = BCrypt_DESECBSw(g_bcrypt_handle, pBcrypt_desEcbSw);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error DES-ECB (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_SwDesCbc(
    DrmCommon_DesCbcSw_t *inoutp_desCbcSw)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_DESCBCSwCtrl_t *pBcrypt_desCbcSw = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_desCbcSw = (BCRYPT_S_DESCBCSwCtrl_t*)inoutp_desCbcSw;

    errCode = BCrypt_DESCBCSw(g_bcrypt_handle, pBcrypt_desCbcSw);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error DES-CBC (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/********************************************************************************/
/*                      DSA-xxx Functionality                                   */
/********************************************************************************/
DrmRC DRM_Common_SwDsa(
    DrmCommon_DsaSw_t *inoutp_dsaSw)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_DSASwParam_t *pBcrypt_dsaSw = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_dsaSw = (BCRYPT_S_DSASwParam_t*)inoutp_dsaSw;

    errCode = BCrypt_DSASw(g_bcrypt_handle, pBcrypt_dsaSw);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error DSA (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/********************************************************************************/
/*                      MD5 Functionality                                       */
/********************************************************************************/
DrmRC DRM_Common_SwMd5(
    DrmCommon_Md5Sw_t *inoutp_md5Sw)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_MD5SwParam_t *pBcrypt_md5Sw = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_md5Sw = (BCRYPT_S_MD5SwParam_t*)inoutp_md5Sw;

    errCode = BCrypt_MD5Sw(g_bcrypt_handle, pBcrypt_md5Sw);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error MD5 (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/********************************************************************************/
/*                      RC4 Functionality                                       */
/********************************************************************************/
DrmRC DRM_Common_SwRc4(
    DrmCommon_Rc4Sw_t *inoutp_rc4Sw)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_RC4SwParam_t *pBcrypt_rc4Sw = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_rc4Sw = (BCRYPT_S_RC4SwParam_t*)inoutp_rc4Sw;

    errCode = BCrypt_RC4Sw(g_bcrypt_handle, pBcrypt_rc4Sw);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error RC4 (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

void DRM_Common_SwRc4SetKey (
    DrmCommon_Rc4Key_t *key,
    uint32_t keyLen,
    uint8_t *keyData)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    BCrypt_RC4SetKey((BCRYPT_RC4Key_t *)key,
                     keyLen,
                     keyData);

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    BKNI_ReleaseMutex(drmCommonMutex);
    return;
}

/********************************************************************************/
/*                      RNG Functionality                                       */
/********************************************************************************/
DrmRC DRM_Common_SwRng(
    DrmCommon_RngSw_t *inoutp_rngSwIO)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_RNGSwCtrl_t *pBcrypt_rngSw = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_rngSw = (BCRYPT_S_RNGSwCtrl_t*)inoutp_rngSwIO;

    errCode = BCrypt_RNGSw(g_bcrypt_handle, pBcrypt_rngSw);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error RNG (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/********************************************************************************/
/*                      RSA Functionality                                       */
/********************************************************************************/
DrmRC DRM_Common_GetRSA_From_SubjectPublicKeyInfo(
    uint8_t *buf,
    uint32_t length,
    DrmCommon_RsaKey_t *pSwRsaKey)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BDBG_ASSERT(buf != NULL);
    BDBG_ASSERT(pSwRsaKey != NULL);
    BDBG_ASSERT(length != 0);

    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_RSAKey_t *pBcrypt_rsaSw = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_rsaSw = (BCRYPT_RSAKey_t*)pSwRsaKey;

    errCode = BCrypt_GetRSA_From_SubjectPublicKeyInfo(g_bcrypt_handle, buf, length, pBcrypt_rsaSw);

    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error RSA (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_GetRsa_From_PrivateKeyInfo(
    uint8_t *buf,
    uint32_t length,
    DrmCommon_RsaKey_t *pSwRsaKey)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BDBG_ASSERT(buf != NULL);
    BDBG_ASSERT(pSwRsaKey != NULL);
    BDBG_ASSERT(length != 0);

    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_RSAKey_t *pBcrypt_rsaSw = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    pBcrypt_rsaSw = (BCRYPT_RSAKey_t*)pSwRsaKey;

    BDBG_ERR(("%s -  RSA %d ", BSTD_FUNCTION, __LINE__));
    errCode = BCrypt_GetRSA_From_PrivateKeyInfo(g_bcrypt_handle, buf, length, pBcrypt_rsaSw);

    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error RSA (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_SwRsa(
    DrmCommon_RsaSwParam_t *inoutp_rsaSwIO)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BDBG_ASSERT(inoutp_rsaSwIO != NULL);

    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;
    BCRYPT_S_RSASwParam_t *pBcrypt_rsaSw = NULL;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
    /*Added field 'padType' to  rsaSWIo struct for Adobe. The following is done so that other drms using this API and not
      and not setting padType don't fail*/

    if(inoutp_rsaSwIO->padType < DrmCommon_RSAPaddingType_ePKCS1 || inoutp_rsaSwIO->padType > DrmCommon_RSAPaddingType_ePSS)
    {
        inoutp_rsaSwIO->padType = DrmCommon_RSAPaddingType_ePKCS1;
        BDBG_ERR(("%s - Unexpected padType value detected.  Overwriting to DrmCommon_RSAPaddingType_ePKCS1", BSTD_FUNCTION));
    }

    pBcrypt_rsaSw = (BCRYPT_S_RSASwParam_t*)inoutp_rsaSwIO;

    errCode = BCrypt_RSASw(g_bcrypt_handle, pBcrypt_rsaSw);

    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error RSA (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/********************************************************************************/
/*                      X509 Functionality                                      */
/********************************************************************************/
DrmRC DRM_Common_Swx509ASN1DerDecode(
    const unsigned char* x509Data,
    int nDataLen,
    X509** pCertificate)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    errCode = BCrypt_x509ASN1DerDecode(g_bcrypt_handle,
                                       x509Data,
                                       nDataLen,
                                       pCertificate);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error x509 DER decode (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

void DRM_Common_Swx509Free(
    X509* m_pCertificate)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    BCrypt_x509Free(g_bcrypt_handle, m_pCertificate);

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return;
}

DrmRC DRM_Common_Swx509GetDigestAlgorithm(
    X509* m_pCertificate,
    char* szAlgorithm,
    int len)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    errCode = BCrypt_x509GetDigestAlgorithm(g_bcrypt_handle,
                                            m_pCertificate,
                                            szAlgorithm,
                                            len);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error x509 get digest algorithm (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_Swx509GetRsaPublicKey(
    X509* m_pCertificate,
    DrmCommon_RsaKey_t * rsa_key)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    errCode = BCrypt_x509GetRsaPublicKey(g_bcrypt_handle,
                                         m_pCertificate,
                                         (BCRYPT_RSAKey_t*)rsa_key);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error x509 get RSA public key (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

void DRM_Common_SwRsaPublicKeyFree(
    DrmCommon_RsaKey_t* rsa_key)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    BCrypt_publicKeyFree(g_bcrypt_handle, (BCRYPT_RSAKey_t*)rsa_key);

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return;
}

DrmRC DRM_Common_SwRSAReadPrivateKeyPem(
    FILE* fp_privKeyIn,
    DrmCommon_RsaKey_t* rsa_key,
    long* p_nSize)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode = BCRYPT_STATUS_eOK;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    errCode = BCrypt_RSAReadPrivateKeyPem(g_bcrypt_handle,
                                          fp_privKeyIn,
                                          (BCRYPT_RSAKey_t*)rsa_key,
                                          p_nSize);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error RSA Read Private Key PEM (0x%08x)", BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
    }

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

void DRM_Common_SwRsaPrivateKeyFree(
    DrmCommon_RsaKey_t* rsa_key)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    BCrypt_privateKeyFree(g_bcrypt_handle, (BCRYPT_RSAKey_t*)rsa_key);

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return;
}

DrmRC DRM_Common_ConvBinToStr(
    uint8_t* data,
    uint32_t byteLen,
    DrmCommon_ConvertFormat_e drm_common_format,
    char* outStr)
{
    DrmRC rc = Drm_Success;

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
    BCRYPT_String_Format_t bcrypt_format = BCRYPT_String_Format_Decimal; /* Decimal default */

    if(drm_common_format == DrmCommon_CovertFormat_eDecimal){
        BDBG_MSG(("%s - input format is decimal", BSTD_FUNCTION));
        bcrypt_format = BCRYPT_String_Format_Decimal;
    }
    else{
        BDBG_MSG(("%s - input format is hex", BSTD_FUNCTION));
        bcrypt_format = BCRYPT_String_Format_Hex;
    }

    if(BCrypt_ConvBinToStr(g_bcrypt_handle,
                           (unsigned char *)data,
                           (unsigned long)byteLen,
                           bcrypt_format,  outStr) != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Error converting to string", BSTD_FUNCTION));
        rc = Drm_BcryptErr;
        goto ErrorExit;
    }

 ErrorExit:
    BDBG_MSG(("%s - Exiting function (string = '%s', length ='%u')", BSTD_FUNCTION, outStr, (unsigned int)strlen(outStr)));
    return rc;
}

/********************************************************************************/
/*                           Diffie Hellman Functionality                       */
/********************************************************************************/
static DrmRC _SwDHPemInit_LOCKED(const char * pem,
                                 int pemLen,
                                 void * *pHandle)
{
    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode;
    BCRYPT_DH_t *bcryptContext;

    if (!pHandle) {
        BDBG_ERR(("%s - Given context is NULL", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        goto end;
    }

    errCode = BCrypt_DH_FromPem((const uint8_t *)pem, pemLen, &bcryptContext);
    if(errCode != BCRYPT_STATUS_eOK) {
        BDBG_ERR(("%s - Cannot init DH instance with given Pem (0x%08x)",
                  BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
        goto end;
    }
    *pHandle = (void *)bcryptContext;

 end:
    return rc;
}

DrmRC DRM_Common_SwDHFromPem(const char * pem,
                             int pemLen,
                             void * *pHandle)
{
    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));

    DrmRC rc = _SwDHPemInit_LOCKED(pem, pemLen, pHandle);

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_SwDHComputeSharedSecret(void * handle,
                                         const uint8_t * remotePublicKey,
                                         int keyLen,
                                         uint8_t ** pSharedSecret,
                                         int *pSharedSecretLen)
{
    BCRYPT_DH_t * bcryptContext = (BCRYPT_DH_t *)handle;
    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode;

    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
    if (!handle) {
        BDBG_ERR(("%s - Given context is NULL", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        goto end_locked;
    }

    if (!pSharedSecret || !pSharedSecretLen) {
        BDBG_ERR(("%s - Given result holders are invalid", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        goto end_locked;
    }

    errCode = BCrypt_DH_ComputeSharedSecret(bcryptContext,
                                            remotePublicKey, keyLen);
    if(errCode != BCRYPT_STATUS_eOK) {
        BDBG_ERR(("%s - Cannot compute DH shared secret", BSTD_FUNCTION));
        rc = Drm_BcryptErr;
        goto end_locked;
    }

    *pSharedSecret = bcryptContext->sharedSecret;
    *pSharedSecretLen = bcryptContext->sharedSecretLen;

 end_locked:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_SwDHCleanup(void * handle)
{
    BCRYPT_DH_t * bcryptContext = (BCRYPT_DH_t *)handle;
    DrmRC rc = Drm_Success;
    BCRYPT_STATUS_eCode errCode;

    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
    if (!handle) {
        BDBG_ERR(("%s - Given context is NULL", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        goto end_locked;
    }

    errCode = BCrypt_DH_Free(bcryptContext);
    if(errCode != BCRYPT_STATUS_eOK)
    {
        BDBG_ERR(("%s - Cannot delete DH instance (0x%08x)",
                  BSTD_FUNCTION, errCode));
        rc = Drm_BcryptErr;
        goto end_locked;
    }

 end_locked:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

DrmRC DRM_Common_SwDHGetPublicKey(void * handle,
                                  uint8_t ** pPublicKey,
                                  int * pPublicKeyLen)
{
    BCRYPT_DH_t * bcryptContext = (BCRYPT_DH_t *)handle;
    DrmRC rc = Drm_Success;

    BDBG_ASSERT(drmCommonMutex != NULL);
    BKNI_AcquireMutex(drmCommonMutex);

    BDBG_MSG(("%s - Entered function", BSTD_FUNCTION));
    if (!handle) {
        BDBG_ERR(("%s - Given context is NULL", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        goto end_locked;
    }

    if (!pPublicKey || !pPublicKeyLen) {
        BDBG_ERR(("%s - Given result holders are invalid", BSTD_FUNCTION));
        rc = Drm_InvalidParameter;
        goto end_locked;
    }

    *pPublicKey = bcryptContext->pubKey;
    *pPublicKeyLen = bcryptContext->pubKeyLen;

 end_locked:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));

    BKNI_ReleaseMutex(drmCommonMutex);
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_Common_FetchDeviceIds
 **
 ** DESCRIPTION:
 **   Retrieve the OTP IDs
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
DrmRC DRM_Common_FetchDeviceIds(drm_chip_info_t *pStruct)
{
    unsigned count = 0;
    DrmRC       rc = Drm_Success;
#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_ReadOtpIO readOtpIO;
    NEXUS_OtpCmdReadRegister    readOtpEnum;
    NEXUS_OtpKeyType            keyType;

    BKNI_Memset(&readOtpIO, 0x00, sizeof(NEXUS_ReadOtpIO));

    for(count = 0; count < 2; count++)
    {
        readOtpEnum = NEXUS_OtpCmdReadRegister_eKeyID;

        if(count == 0)
        {
            keyType = NEXUS_OtpKeyType_eA;
        }
        else
        {
            keyType = NEXUS_OtpKeyType_eB;
        }

        if(NEXUS_Security_ReadOTP(readOtpEnum, keyType, &readOtpIO) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Call to 'NEXUS_Security_ReadOTP' FAILED.", BSTD_FUNCTION));
            rc = Drm_BindingErr;
            goto ErrorExit;
        }
        else
        {
            BDBG_MSG(("%s - Call to 'NEXUS_Security_ReadOTP' succeeded. otpKeyIdSize = '0x%08x'", BSTD_FUNCTION, readOtpIO.otpKeyIdSize));
            if(count == 0){
                BKNI_Memcpy(pStruct->devIdA, readOtpIO.otpKeyIdBuf, readOtpIO.otpKeyIdSize);
            }
            else{
                BKNI_Memcpy(pStruct->devIdB, readOtpIO.otpKeyIdBuf, readOtpIO.otpKeyIdSize);
            }
        }
    }//end of for-loop

#else
    NEXUS_OtpKeyInfo otpKeyInfo;
    BKNI_Memset(&otpKeyInfo, 0x00, sizeof(NEXUS_OtpKeyInfo));

    for(count = 0; count < 2; count++)
    {

        if(NEXUS_OtpKey_GetInfo( count, &otpKeyInfo ) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Call to 'NEXUS_OtpKey_GetInfo' FAILED.", BSTD_FUNCTION));
            rc = Drm_BindingErr;
            goto ErrorExit;
        }
        else
        {
            BDBG_MSG(("%s - Call to 'NEXUS_OtpKey_GetInfo' succeeded. otpKeyIdSize = '0x%08x'", BSTD_FUNCTION, NEXUS_OTP_KEY_ID_LENGTH));
            if(count == 0){
                BKNI_Memcpy(pStruct->devIdA, otpKeyInfo.id, NEXUS_OTP_KEY_ID_LENGTH);
            }
            else{
                BKNI_Memcpy(pStruct->devIdB, otpKeyInfo.id, NEXUS_OTP_KEY_ID_LENGTH);
            }
        }
    }//end of for-loop
#endif

ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}
