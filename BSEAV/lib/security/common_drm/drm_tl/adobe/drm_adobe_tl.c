/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/
#include <string.h>
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include "drm_adobe.h"
#include "drm_common_tl.h"
#include "drm_common_swcrypto_types.h"
#include "drm_common_swcrypto.h"
#include "drm_secure_store.h"

#include "drm_data.h"

#include "bsagelib_types.h"
#include "sage_srai.h"

#define BCRYPT_2048_RSA_KEY_SZ 256
#define SHA1_DIGEST_SZ 20
#define SHA256_DIGEST_SZ 32

static uint8_t* gpRootCertficateDigest    = NULL;
static uint32_t gRootCertficateDigestSz   = 0;

static uint8_t* gpIndivTransportCert      = NULL;
static uint32_t gIndivTransportCertSz     = 0;

#define ROOTCERT_DIGEST_SZ 32

//#define BUFFER_DUMP
#ifndef BUFFER_DUMP
#ifdef DRM_MSG_PRINT_BUF
#undef DRM_MSG_PRINT_BUF
#define DRM_MSG_PRINT_BUF(a,b,c)
#endif
#endif

/*
 * List of supported SAGE commands
 * */
typedef enum DrmAdobe_CommandId_e
{
    DrmAdobe_CommandId_eLoadKey                 = 0,
    DrmAdobe_CommandId_eCreateSessionContext    = 1,
    DrmAdobe_CommandId_ePublicKeyOperation      = 2,
    DrmAdobe_CommandId_ePrivateKeyOperation     = 3,
    DrmAdobe_CommandId_eDecryptInit             = 4,
    DrmAdobe_CommandId_eDecryptUpdate           = 5,
    DrmAdobe_CommandId_eDecryptFinalize         = 6,
    DrmAdobe_CommandId_eGetDecryptionCert       = 7,
    DrmAdobe_CommandId_eDestroySessionContext   = 8,
    DrmAdobe_CommandId_eUnInit                  = 9,
    DrmAdobe_CommandId_eMax
}DrmAdobe_CommandId_e;

#define DRM_ADOBE_IV_SIZE (16)
#define DRM_ADOBE_AES_KEY_SIZE (16)

/* SRAI module handle */
static SRAI_ModuleHandle moduleHandle = NULL;

/* static variables */
static DrmAdobe_DeviceCapabilities capabilities = 0; /* can be static since applies to all clients */

typedef struct DrmAdobeKeyIndexMapList
{
    DrmAdobeKeyIndexMap_t keyEntry;
    struct DrmAdobeKeyIndexMapList   *pNext;
}DrmAdobeKeyIndexMapList ;

static DrmAdobeParamSettings_t gAdobeParamSettings;
static DrmAdobeKeyIndexMapList *gpAdobeHostKeyIndexMapHead = NULL;
static DrmAdobeKeyIndexMapList *gpAdobeHostKeyIndexMapTail = NULL;
static uint8_t gHostKeySequenceNumber = 0;


static DrmRC DRM_Adobe_HostKeyIndexMapList_Create(DrmAdobeKeyIndexMap_t *pEntry);
static DrmRC DRM_Adobe_HostKeyIndexMapList_AddEntry(DrmAdobeKeyIndexMap_t *pEntry);
static DrmRC DRM_Adobe_HostKeyIndexMapList_DeleteEntry(uint32_t index);
static bool DRM_Adobe_HostKeyIndexMapList_Search(DrmAdobeKeyIndexMap_t **pEntry, uint32_t   index);

static bool paramStructureSetExternally = false;

BDBG_MODULE(drm_adobe_tl);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pAdobeParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Adobe_GetDefaultParamSettings(
    DrmAdobeParamSettings_t *pAdobeParamSettings)
{
    ChipType_e chip_type;
    BDBG_ENTER(DRM_Adobe_GetDefaultParamSettings);

    BKNI_Memset((uint8_t*)pAdobeParamSettings, 0x00, sizeof(DrmAdobeParamSettings_t));
    pAdobeParamSettings->drmCommonInit.drmCommonInit.heap = NULL;
#ifdef USE_UNIFIED_COMMON_DRM
    pAdobeParamSettings->drmCommonInit.drmType = 0;
#else
    pAdobeParamSettings->drmCommonInit.drmType = BSAGElib_BinFileDrmType_eAdobeAxcess;
#endif
    chip_type = DRM_Common_GetChipType();
#if USE_UNIFIED_COMMON_DRM
    if(chip_type == ChipType_eZS)
    {
        pAdobeParamSettings->drmCommonInit.ta_bin_file_path = bdrm_get_ta_dev_bin_file_path();
    }
    else
    {
        pAdobeParamSettings->drmCommonInit.ta_bin_file_path = bdrm_get_ta_bin_file_path();
    }
#else
    if(chip_type == ChipType_eZS)
    {
        pAdobeParamSettings->drmCommonInit.ta_bin_file_path = bdrm_get_ta_adobe_dev_bin_file_path();
    }
    else
    {
        pAdobeParamSettings->drmCommonInit.ta_bin_file_path = bdrm_get_ta_adobe_bin_file_path();
    }
#endif

    pAdobeParamSettings->drm_bin_file_path = bdrm_get_drm_bin_file_path();

    BDBG_MSG(("%s DRM %s TA %s", __FUNCTION__, pAdobeParamSettings->drm_bin_file_path, pAdobeParamSettings->drmCommonInit.ta_bin_file_path));
    return;
}


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_SetParamSettings
**
** DESCRIPTION:
**   Set param settings
**
** PARAMETERS:
** vuduParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Adobe_SetParamSettings(
        DrmAdobeParamSettings_t *pAdobeParamSettings)
{
    BDBG_ENTER(DRM_Adobe_SetParamSettings);

    gAdobeParamSettings.drmCommonInit.drmType = pAdobeParamSettings->drmCommonInit.drmType;
    gAdobeParamSettings.drmCommonInit.drmCommonInit.heap = pAdobeParamSettings->drmCommonInit.drmCommonInit.heap;
    paramStructureSetExternally = true;
    gAdobeParamSettings.drm_bin_file_path = pAdobeParamSettings->drm_bin_file_path;
    gAdobeParamSettings.drmCommonInit.ta_bin_file_path = pAdobeParamSettings->drmCommonInit.ta_bin_file_path;

    BDBG_MSG(("%s - Exiting function (%s)", __FUNCTION__, gAdobeParamSettings.drm_bin_file_path));
    return;
}

/******************************************************************************
** FUNCTION:
**   DRM_Adobe_UnInit
***************************************************************************/
DrmRC DRM_Adobe_UnInit(void)
{
    BDBG_ENTER(DRM_Adobe_UnInit);
    DrmAdobeKeyIndexMapList *ptr = gpAdobeHostKeyIndexMapHead;
    int count = 0;

    paramStructureSetExternally = false;

    while(ptr != NULL)
    {
        BDBG_MSG(("%s - Deleting index (0x%08x)", __FUNCTION__, ptr->keyEntry.adobeIndex));

        if(DRM_Adobe_HostKeyIndexMapList_DeleteEntry(ptr->keyEntry.adobeIndex) != Drm_Success)
        {
            BDBG_ERR(("%s - Error deleting index '0x%08x'. Forcing break", __FUNCTION__, ptr->keyEntry.adobeIndex));
            break;
        }

        /* reset pointer to head */
        ptr = gpAdobeHostKeyIndexMapHead;

        /* force a break if necessary (i.e. an error cleaning up has occured) */
        count++;
        if(count > 10*1024)
        {/* arbitrary large number */
            BDBG_ERR(("%s - Error occured during cleanup of the keyList, Forcing break", __FUNCTION__));
            break;
        }
    }

    /*Free Rootcertdigest memeory*/
    if(gpRootCertficateDigest )
    {
        DRM_Common_MemoryFree(gpRootCertficateDigest );
        gpRootCertficateDigest = NULL;
    }

    if(gpIndivTransportCert)
    {
        DRM_Common_MemoryFree(gpIndivTransportCert);
        gpIndivTransportCert = NULL;
    }

    gpAdobeHostKeyIndexMapHead = NULL;
    gpAdobeHostKeyIndexMapTail = NULL;
    gHostKeySequenceNumber     = 0;

    /* After sending UnInit command to SAGE, close the module handle (i.e. send DRM_Adobe_UnInit to SAGE) */
#ifdef USE_UNIFIED_COMMON_DRM
    DRM_Common_TL_ModuleFinalize(moduleHandle);
#else
    DRM_Common_TL_ModuleFinalize_TA(Common_Platform_AdobeAxcess, moduleHandle);
#endif

    BDBG_LEAVE(DRM_Adobe_UnInit);
    return Drm_Success;
}

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_Initialize
******************************************************************************/
DrmRC DRM_Adobe_Initialize(DrmAdobeParamSettings_t *pAdobeParamSettings)
{
    DrmRC rc = Drm_Success;
    BSAGElib_InOutContainer *pContainer = NULL;

    BDBG_ENTER(DRM_Adobe_Initialize);

    if(pAdobeParamSettings == NULL)
    {
        BDBG_ERR(("%s - Parameter settings are NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(paramStructureSetExternally == false){
        BDBG_MSG(("%s - Parameter structure not explicitely set", __FUNCTION__));
    }
    BDBG_ERR(("%s:Calling DRM_Common_TL_Initialize",__FUNCTION__));
    rc = DRM_Common_TL_Initialize(&pAdobeParamSettings->drmCommonInit);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module", __FUNCTION__));
        goto ErrorExit;
    }

    pContainer = SRAI_Container_Allocate();
    if(pContainer == NULL)
    {
        BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    pContainer->basicIn[0] = ((pAdobeParamSettings->capabilities&0xFFFFFFFF00000000)>>32);
    pContainer->basicIn[1] = (uint32_t)(pAdobeParamSettings->capabilities&0x00000000FFFFFFFF);

    /* Initialize SAGE Adobe module */
#ifdef USE_UNIFIED_COMMON_DRM
    rc = DRM_Common_TL_ModuleInitialize(DrmCommon_ModuleId_eAdobe, pAdobeParamSettings->drm_bin_file_path, pContainer, &moduleHandle);
#else
    rc = DRM_Common_TL_ModuleInitialize_TA(Common_Platform_AdobeAxcess, Adobe_ModuleId_eDRM, pAdobeParamSettings->drm_bin_file_path, pContainer, &moduleHandle);
#endif
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module (0x%08x)", __FUNCTION__, pContainer->basicOut[0]));
        goto ErrorExit;
    }

ErrorExit:
    BDBG_LEAVE(DRM_Adobe_Initialize);

    if(pContainer != NULL){
        SRAI_Container_Free(pContainer);
    }

    return rc;
}

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_P_PrivateKeyDecrypt
**
** DESCRIPTION:Uses bcrypt to do private key decrypt
**
******************************************************************************/
DrmRC DRM_Adobe_P_PrivateKeyDecrypt( uint32_t index,         /* Input: Index of key to use */
                                     DrmAdobe_RsaSwSettings_t *pAxsRsaSettings
                                    )
{
    DrmRC rc = Drm_Success;
    DrmAdobeKeyIndexMap_t *pKey = NULL;
    DrmCommon_RsaSwParam_t rsaSwIO;
    DrmCommon_RsaKey_t rsaKey;

#if FILE_DUMP
    FILE * fsig=NULL;
    FILE * fdigest=NULL;
    FILE * fpubkey=NULL;
    FILE * fprivatekey=NULL;
    FILE * fswkeybuf=NULL;
    FILE * fPadding= NULL;
#endif

    BDBG_MSG(("%s - Entered function",__FUNCTION__));

    BDBG_ASSERT(pAxsRsaSettings != NULL);

    BDBG_MSG(("%s:srcDataSize = %d",__FUNCTION__,pAxsRsaSettings->operation_struct.sign_op.srcDataSize));
    BDBG_MSG(("%s:*signatureSize = %d",__FUNCTION__,*pAxsRsaSettings->operation_struct.sign_op.signatureSize));

    BKNI_Memset((void *)&rsaKey, 0x0,sizeof(DrmCommon_RsaKey_t));
    BKNI_Memset((void *)&rsaSwIO, 0x0,sizeof(DrmCommon_RsaSwParam_t ));

    if (true != DRM_Adobe_HostKeyIndexMapList_Search(&pKey,index))
    {
        BDBG_ERR(("%s:KeyIndex %d Not found ",__FUNCTION__,index));
        return Drm_Err;
    }

    if(!pKey -> swKey) /*call nexus_secirity_RsaSign*/
    {
        BDBG_ERR(("%s:ERROR: You should not be here ",__FUNCTION__));
        return Drm_Err;

    }
    else //if swkey call bcrypt api for private key signing
    {
#if FILE_DUMP
        fsig = fopen("signature.bin", "wb");
        fdigest = fopen("digest.bin", "wb");
        fprivatekey = fopen("privatekey.bin", "wb");
        fswkeybuf = fopen("swkeybuf.bin", "wb");
        fwrite(pKey->keybuf, 1, pKey->keybufsz, fswkeybuf);
#endif
        BDBG_MSG(("%s:Calling SW rsa dec api priv key sz is %d",__FUNCTION__,pKey->keybufsz));


        DRM_MSG_PRINT_BUF("priv key buf: ", pKey->keybuf,pKey->keybufsz);

        rsaKey.n.pData = BKNI_Malloc(BCRYPT_2048_RSA_KEY_SZ);
        rsaKey.e.pData = BKNI_Malloc(BCRYPT_2048_RSA_KEY_SZ);
        rsaKey.d.pData = BKNI_Malloc(BCRYPT_2048_RSA_KEY_SZ);

        rc = DRM_Common_GetRsa_From_PrivateKeyInfo(pKey->keybuf,pKey->keybufsz,&rsaKey);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s :DRM_Common_GetRsa_From_PrivateKeyInfo() failed",__FUNCTION__));
            goto ErrorExit;
        }

        DRM_MSG_PRINT_BUF("pub Modulus:",&rsaKey.n.pData[0],rsaKey.n.len);
        DRM_MSG_PRINT_BUF("pub exp:",&rsaKey.e.pData[0],rsaKey.e.len);
        DRM_MSG_PRINT_BUF("&pKey->keybuf[173]",&pKey->keybuf[173],128);

        BDBG_MSG(("%s:lenof priv exp is %lu",__FUNCTION__,rsaKey.d.len));
        DRM_MSG_PRINT_BUF("priv exp:",&rsaKey.d.pData[0],rsaKey.d.len);



        rsaSwIO.bRSAop      = drmRsadec;
        rsaSwIO.key         = &rsaKey;
        rsaSwIO.pbDataIn    = pAxsRsaSettings->operation_struct.sign_op.pSrcAddr;
        rsaSwIO.cbDataIn    = pAxsRsaSettings->operation_struct.sign_op.srcDataSize;
        rsaSwIO.pbDataOut   = pAxsRsaSettings->operation_struct.sign_op.pSignatureAddr;
        rsaSwIO.cbDataOut   = (unsigned long *)pAxsRsaSettings->operation_struct.sign_op.signatureSize;

        switch(pAxsRsaSettings->operation_struct.sign_op.padType)
        {
        case DrmAdobe_PadType_ePKCS1_OAEP:
            rsaSwIO.padType     =  DrmCommon_RSAPaddingType_eOAEP;
            break;
        case DrmAdobe_PadType_eNO_PADDING:
            rsaSwIO.padType     =  DrmCommon_RSAPaddingType_eNOPADDING;
            break;
        case DrmAdobe_PadType_ePKCS1V15:
        default:
            rsaSwIO.padType     =  DrmCommon_RSAPaddingType_ePKCS1;
        }

        BDBG_MSG(("%s: call DRM_Common_SwRsa",__FUNCTION__));
        rc = DRM_Common_SwRsa(&rsaSwIO);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s :Public key operation failed",__FUNCTION__));
            goto ErrorExit;

        }



    }
    BDBG_MSG(("%s: size of signature is %d",__FUNCTION__,*pAxsRsaSettings->operation_struct.sign_op.signatureSize ));

ErrorExit:
    if(rsaKey.n.pData)
        BKNI_Free(rsaKey.n.pData);
    if(rsaKey.e.pData)
        BKNI_Free(rsaKey.e.pData);
    if(rsaKey.d.pData)
        BKNI_Free(rsaKey.d.pData);
#if FILE_DUMP
        if (fsig)
            fclose(fsig);
        if (fpubkey)
            fclose(fpubkey);
#endif

#if FILE_DUMP
     if (fprivatekey)
            fclose(fprivatekey);
        if (fswkeybuf)
            fclose(fswkeybuf);
         if (fdigest)
             fclose(fdigest);
#endif

    BDBG_MSG(("%s - Exiting function",__FUNCTION__));
    return rc;
}


/*****************************************************************************************
Function:DRM_Adobe_LoadKey
******************************************************************************************/
DrmRC DRM_Adobe_LoadKey(uint32_t EncKeyIndex,            /* Input: Index of key used to decrypt key data. If Index = 0, then decrypt with DRM key */
                      uint8_t *pKeyData,                /* Input: Key data. Format determined by key type */
                      uint32_t keyLen,                    /* Input: Length of key data */
                      DrmAdobe_KeyType_e keyType,
                      DrmAdobe_AesMode_e mode,            /* Input: AESECB || AESCBC || AESCTR. If doing AES */
                      uint8_t *pIv,                        /* Input: Pointer to a 16 byte IV buffer. Can be NULL if doing algorithm or mode with no IV */
                      DrmAdobe_KeyLadderType_e keyladderType,    /* Input: i.e. DrmAdobe_ResultType_SESSION_KEY */
                      DrmAdobe_PadType_e padtype,        /* Input: Type of padding used on the data */
                      uint8_t *pMetadata,                /* Input: TBD */
                      uint32_t *pIndex)
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *pContainer = NULL;
    DrmAdobeKeyIndexMap_t *pEncKey =NULL;
    BSTD_UNUSED(pMetadata);

    BDBG_ENTER(DRM_Adobe_LoadKey);

    if(pKeyData == NULL)
    {
        BDBG_ERR(("%s - key data buffer is NULL", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }


   if( DRM_Adobe_HostKeyIndexMapList_Search(&pEncKey, EncKeyIndex)==true)
   {
        //The enc key/wrapping key is a swkey

       uint8_t *pClearKey = NULL;
        uint32_t clearKeySz = 256;

        if(DRM_Common_MemoryAllocate((uint8_t **)&pClearKey, clearKeySz) != Drm_Success)
        {
            BDBG_ERR(("%s:Memory Allocation err",__FUNCTION__));
            return Drm_MemErr;
        }

        /*decrypt  the key using bcrypt rsadec*/
        DrmAdobe_RsaSwSettings_t drmAdobeSettings;
        DRM_Adobe_GetDefaultRsaSettings(&drmAdobeSettings);
        /*IMP NOTE:the Rsasetting structure may need to acccomodate the padding field also. originally it was not accomodated because bcrypt hardcodes the
        padding.*/

        drmAdobeSettings.operation_struct.sign_op.pSignatureAddr    = pClearKey;
        drmAdobeSettings.operation_struct.sign_op.signatureSize     = &clearKeySz;
        drmAdobeSettings.operation_struct.sign_op.pSrcAddr          = pKeyData;
        drmAdobeSettings.operation_struct.sign_op.srcDataSize       = keyLen;
        drmAdobeSettings.operation_type                             = DrmAdobe_RsaPubkeyOpType_eDec;

        DRM_Adobe_P_PrivateKeyDecrypt(EncKeyIndex,&drmAdobeSettings);

        /*setClearKey*/
        DRM_Adobe_SetClearKey(pClearKey,clearKeySz,keyType,keyladderType,pIndex);
        goto ErrorExit;

    }



    pContainer = SRAI_Container_Allocate();
    if(pContainer == NULL)
    {
        BDBG_ERR(("%s - Error allocating pContainer", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* allocate buffers accessible by Sage*/
    if(keyLen != 0)
    {
        pContainer->blocks[0].data.ptr = SRAI_Memory_Allocate(keyLen, SRAI_MemoryType_Shared);
        pContainer->blocks[0].len = keyLen;
        BKNI_Memcpy(pContainer->blocks[0].data.ptr, pKeyData, keyLen);
    }

    if(pIv != NULL)
    {
        pContainer->blocks[1].data.ptr = SRAI_Memory_Allocate(16, SRAI_MemoryType_Shared);
        BKNI_Memcpy(pContainer->blocks[1].data.ptr, pIv, DRM_ADOBE_IV_SIZE);
        pContainer->blocks[1].len = DRM_ADOBE_IV_SIZE;
    }

    /* map to parameters into srai_inout_pContainer */
    pContainer->basicIn[0] = EncKeyIndex;
    pContainer->basicIn[1] = (uint32_t)keyType;
    pContainer->basicIn[2] = (uint32_t)mode;
    pContainer->basicIn[3] = (uint32_t)keyladderType;
    pContainer->basicIn[4] = (uint32_t)padtype;

    sage_rc = SRAI_Module_ProcessCommand(moduleHandle, DrmAdobe_CommandId_eLoadKey, pContainer);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract status from pContainer */
    sage_rc = pContainer->basicOut[0];
    /* how are results/status returned again? */
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent succuessfully to loadkey but actual operation failed (0x%08x)", __FUNCTION__, sage_rc));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* if success, extract index from pContainer */
    (*pIndex) = pContainer->basicOut[1];
    BDBG_MSG(("%s - Returned index = '0x%08x'", __FUNCTION__, (*pIndex)));

ErrorExit:
    if(pContainer !=NULL)
    {
        if(pContainer->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(pContainer->blocks[0].data.ptr);
            pContainer->blocks[0].data.ptr = NULL;
        }

        if(pContainer->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(pContainer->blocks[1].data.ptr);
            pContainer->blocks[1].data.ptr = NULL;
        }

        //if(pContainer != NULL){
            SRAI_Container_Free(pContainer);
        //}
    }

    BDBG_LEAVE(DRM_Adobe_LoadKey);

    return rc;
}

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetRand
**
** DESCRIPTION:
**
******************************************************************************/
DrmRC DRM_Adobe_DeleteKey(uint32_t index)
{
    BDBG_ENTER(DRM_Adobe_DeleteKey);
    return DRM_Adobe_HostKeyIndexMapList_DeleteEntry(index);
}

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetRand
**
** DESCRIPTION:
**
******************************************************************************/
DrmRC DRM_Adobe_GetRand(uint8_t *pRandom,
                        uint32_t size)
{

    BDBG_ENTER(DRM_Adobe_GetRand);

    uint32_t alignedSz  = size;
    uint8_t* pBufAligned = NULL;
    DrmRC rc = Drm_Success;


    if(pRandom == NULL || size == 0)
    {
        BDBG_ERR(("%s: the input arguments are invalid",__FUNCTION__));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    /* the random number generatore needs the size to be word aligned*/
    if((size % 4)!=0)
        alignedSz = size+(4 - size % 4);

    rc = DRM_Common_MemoryAllocate(&pBufAligned, alignedSz);
    if(rc !=Drm_Success)
    {
        BDBG_ERR(("%s: Error Allocating Memory", __FUNCTION__));
        goto ErrorExit;
    }

    rc = DRM_Common_GenerateRandomNumber(alignedSz,pBufAligned);
    if(rc!=Drm_Success)
    {
        BDBG_ERR(("%s:Error generating random number",__FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memcpy(pRandom,pBufAligned,size);

ErrorExit:
    if(pBufAligned)
       DRM_Common_MemoryFree(pBufAligned);

    BDBG_MSG(("%s - Exiting function",__FUNCTION__));
    return rc;
}





/******************************************************************************
** FUNCTION:
**  DRM_Adobe_CreateDRMSessionCtx
**
** DESCRIPTION:
**
******************************************************************************/
DrmRC DRM_Adobe_CreateDRMSessionCtx(uint32_t index,                    /* Input: Index to key to be used */
                                    DrmAdobe_PadType_e padtype,    /* Input: Type of padding used on the data */
                                    DrmAdobe_AesMode_e mode,            /* Input: AESECB || AESCBC || AESCTR */
                                    uint8_t *pIv,                /* Input: Pointer to a 16 byte IV buffer. Will be NULL if doing AES ECB */
                                    uint8_t *pMetadata,
                                    DrmAdobeSessionContext_t **pCTX)    /* HW DRM Context. Created and freed by HW */
{
    BERR_Code sage_rc = BERR_SUCCESS;
    DrmRC rc = Drm_Success;
    NEXUS_SecurityKeySlotSettings keyslotSettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
    BSAGElib_InOutContainer *pContainer = NULL;
    DrmAdobeKeyIndexMap_t *pSearchEntry = NULL;
    DrmAdobeMetaDataStructure *pMeta = (DrmAdobeMetaDataStructure *)pMetadata;
    DrmAdobeSessionContext_t *pSessCtx = NULL;


    BDBG_ENTER(DRM_Adobe_CreateDRMSessionCtx);

    if(index == 0x00000000)
    {
        BDBG_ERR(("%s - Invalid index", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    /* check if pointer is null */
    if(*pCTX != NULL)
    {
        BDBG_ERR(("%s - Pointer to context is NOT null", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    pSessCtx = (DrmAdobeSessionContext_t *)BKNI_Malloc(sizeof(DrmAdobeSessionContext_t));
    if(pSessCtx == NULL)
    {
        BDBG_ERR(("%s - error allocating memory", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    BKNI_Memset((uint8_t *)pSessCtx, 0x00, sizeof(DrmAdobeSessionContext_t) );

    if(pMeta != NULL)
    {
        if(pMeta->DiskIO)
        {
            BDBG_MSG(("%s: Context associated with index '0x%08x' can only be used for secure store operations",__FUNCTION__, index));
            pSessCtx->bUseCtxforDiskIO = true;
        }
        else{
            BDBG_MSG(("%s: Context associated with index '0x%08x' CANNOT be used for secure store operations",__FUNCTION__, index));
            pSessCtx->bUseCtxforDiskIO = false;
        }
    }
    else
    {
        BDBG_MSG(("%s: No metadata specified for index '0x%08x', setting use option for non-DiskIO operations",__FUNCTION__, index));
        pSessCtx->bUseCtxforDiskIO = false;
    }



    /*
     *
     * Regardless if index is found on Host or not we need to perform the following actions:
     *  1) allocate key slot and assign to session context
     *     if index is found on Host side, specify eTypeHost otherwise eTypeSage
     *  2) fetch keyslot index, send it to sage and assign it to the session context
     *  3) assign the 'mode' to the session context
     *  4) copy IV if necessary
     **/
    NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
    if((DRM_Adobe_HostKeyIndexMapList_Search(&pSearchEntry, index) == true)||( pSessCtx->bUseCtxforDiskIO == true))
    {
        BDBG_MSG(("%s - Allocating keyslot for Host", __FUNCTION__));
        keyslotSettings.client = NEXUS_SecurityClientType_eHost;
         BDBG_MSG(("%s:loadSWKeyToKeySlot !!!",__FUNCTION__));

    }
    else
    {
        BDBG_MSG(("%s - Allocating keyslot for SAGE", __FUNCTION__));
        keyslotSettings.client = NEXUS_SecurityClientType_eSage;
    }

    //NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
    keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

    /* assign keyslot address to context (just expose nexus datatype instead?) */
    pSessCtx->drmCommonOpStruct.keyConfigSettings.keySlot = NEXUS_Security_AllocateKeySlot(&keyslotSettings);
    if(pSessCtx->drmCommonOpStruct.keyConfigSettings.keySlot == NULL)
    {
        BDBG_ERR(("%s - Error allocating keyslot", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    NEXUS_Security_GetKeySlotInfo(pSessCtx->drmCommonOpStruct.keyConfigSettings.keySlot, &keyslotInfo);

    BDBG_MSG(("%s - Keyslot index = '%u'", __FUNCTION__, keyslotInfo.keySlotNumber));

    /* set mode */
    pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.algType = NEXUS_SecurityAlgorithm_eAes;
    if(mode == DrmAdobe_AesMode_eAESCBC)
    {
        BDBG_MSG(("%s - Setting AES-CBC operation", __FUNCTION__));
        pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eCbc;
        pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.algType = NEXUS_SecurityAlgorithm_eAes;
    }
    else if(mode == DrmAdobe_AesMode_eAESECB)
    {
        BDBG_MSG(("%s - Setting AES-ECB operation", __FUNCTION__));
        pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eEcb;
        pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.algType = NEXUS_SecurityAlgorithm_eAes;
    }
    else if(mode == DrmAdobe_AesMode_eAESCTR)
    {
        BDBG_MSG(("%s - Setting AES-CTR operation", __FUNCTION__));
        pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eCounter;
        pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.algType = NEXUS_SecurityAlgorithm_eAesCounter;
        }
    else
    {
        BDBG_ERR(("%s -  Mode type not supported (0x%08x)", __FUNCTION__, mode));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    /* set the index */
    pSessCtx->adobeIndex = (DrmAdobe_AdobeKeyIndex_e)index;

    /* if context wasnt created for diskIO operations look at the IV and see where the index resides (on host or sage) */
    if(pSessCtx->bUseCtxforDiskIO == false)
    {
        /* if IV NULL, set all zero and set length */
        if(pIv == NULL)
        {
            /* fill with zeros if null?? */
            BKNI_Memset(pSessCtx->iv0, 0x00, 16);
            BKNI_Memset(pSessCtx->iv_curr, 0x00, 16);
        }
        else
        {
             uint32_t *pIvkeys = NULL;
             if(DRM_Common_MemoryAllocate((uint8_t **)&pIvkeys, 4*sizeof(uint32_t)) != Drm_Success)
            {
             BDBG_ERR(("%s: Memory Allocation err",__FUNCTION__));
             return Drm_MemErr;
            }
            pIvkeys[0] = (*((uint32_t*)&pIv[0]));
            pIvkeys[1] = (*((uint32_t*)&pIv[4]));
            pIvkeys[2] = (*((uint32_t*)&pIv[8]));
            pIvkeys[3] = (*((uint32_t*)&pIv[12]));

            BKNI_Memcpy(pSessCtx->iv0, pIvkeys /*&test_iv[0]*/, DRM_ADOBE_IV_SIZE);
            BKNI_Memcpy(pSessCtx->iv_curr, pIvkeys/*&test_iv[0]*/, DRM_ADOBE_IV_SIZE); /* for initialization iv_curr = iv0 */
            DRM_MSG_PRINT_BUF("input iv:",pIv,DRM_ADOBE_IV_SIZE);
            DRM_MSG_PRINT_BUF("pSessCtx->iv_curr:",pSessCtx->iv_curr,DRM_ADOBE_IV_SIZE);

            BKNI_Memcpy(pSessCtx->drmCommonOpStruct.keyIvSettings.iv, pIvkeys,/*&test_iv[0],*/ DRM_ADOBE_IV_SIZE);
            pSessCtx->drmCommonOpStruct.keyIvSettings.ivSize = DRM_ADOBE_IV_SIZE;
            DRM_MSG_PRINT_BUF("pSessCtx->drmCommonOpStruct.keyIvSettings.iv", pSessCtx->drmCommonOpStruct.keyIvSettings.iv, pSessCtx->drmCommonOpStruct.keyIvSettings.ivSize);
        }

        BDBG_MSG(("%s - ********************** pSearchEntry = '%p' ***************************", __FUNCTION__, (void *)pSearchEntry));
        if(pSearchEntry != NULL)
        {
            BDBG_MSG(("%s -  pSearchEntry->keybufsz = '%u'", __FUNCTION__, pSearchEntry->keybufsz));
            BDBG_MSG(("%s -  pSearchEntry->keybuf = '%p", __FUNCTION__, pSearchEntry->keybuf));
            if(pSearchEntry->keybufsz != 0)
            {
                BKNI_Memcpy(pSessCtx->drmCommonOpStruct.keyIvSettings.key, pSearchEntry->keybuf /*&test_key[0]*/, pSearchEntry->keybufsz);
                pSessCtx->drmCommonOpStruct.keyIvSettings.keySize = pSearchEntry->keybufsz;
                DRM_MSG_PRINT_BUF("pSessCtx->drmCommonOpStruct.keyIvSettings.key", pSessCtx->drmCommonOpStruct.keyIvSettings.key, pSessCtx->drmCommonOpStruct.keyIvSettings.keySize);
            }
        }

        /* Search for keyindex in Host list */
        if(DRM_Adobe_HostKeyIndexMapList_Search(&pSearchEntry, index) == true)
        {
            BDBG_MSG(("%s -  Index '0x%08x' found on Host side", __FUNCTION__, index));

            if(pMeta == NULL)
            {
                BDBG_MSG(("%s -  metadata is null, setting decrypt operation", __FUNCTION__));
                pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.opType =  NEXUS_SecurityOperation_eDecrypt;
            }
            else
            {
                if(pMeta->encrypt != 0)
                {
                    BDBG_MSG(("%s -  meta->encrypt != 0, setting encrypt operation", __FUNCTION__));
                    pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.opType =  NEXUS_SecurityOperation_eEncrypt;
                }
                else
                {
                    BDBG_MSG(("%s -  meta->encrypt == 0 setting decrypt operation", __FUNCTION__));
                    pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.opType =  NEXUS_SecurityOperation_eDecrypt;
                }
            }
            pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.algType       = NEXUS_SecurityAlgorithm_eAes;
            pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.algVariant     = NEXUS_SecurityAlgorithmVariant_eCbc;
            pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.solitaryMode   = NEXUS_SecuritySolitarySelect_eClear;
            pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.enableExtKey   = false;
            pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.enableExtIv    = false;
            pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.keySlotType    = NEXUS_SecurityKeyType_eOdd;
            pSessCtx->drmCommonOpStruct.keyConfigSettings.settings.termMode       = NEXUS_SecurityTerminationMode_eClear;
            pSessCtx->drmCommonOpStruct.pKeyLadderInfo                            = NULL;
            pSessCtx->drmCommonOpStruct.keySrc                                    = CommonCrypto_eClearKey;


            /* configure host side keyslot */
            if(DRM_Common_KeyConfigOperation(&pSessCtx->drmCommonOpStruct) != Drm_Success)
            {
                BDBG_ERR(("%s - Error configuring keyslot", __FUNCTION__));
                rc = BERR_INVALID_PARAMETER;
                goto ErrorExit;
            }
        }
        else
        {
            BDBG_MSG(("%s -  Index '0x%08x' NOT found on Host side", __FUNCTION__, index));

            pContainer = SRAI_Container_Allocate();
            if(pContainer == NULL)
            {
                BDBG_ERR(("%s - error allocating pContainer", __FUNCTION__));
                rc = Drm_MemErr;
                goto ErrorExit;
            }

            if(pIv != NULL)
            {
                BDBG_MSG(("%s -  IV is not null", __FUNCTION__));

                /* copy iv if != 0 */
                pContainer->blocks[0].data.ptr = SRAI_Memory_Allocate(16, SRAI_MemoryType_Shared);
                if(pContainer->blocks[0].data.ptr == NULL)
                {
                    BDBG_ERR(("%s - error allocating memory", __FUNCTION__));
                    rc = Drm_MemErr;
                    goto ErrorExit;
                }
                pContainer->blocks[0].len = DRM_ADOBE_IV_SIZE;
                BKNI_Memcpy(pContainer->blocks[0].data.ptr, pIv, DRM_ADOBE_IV_SIZE);

            }
            else{
                BDBG_MSG(("%s -  IV is null", __FUNCTION__));
                pContainer->blocks[0].len = 0;
                pContainer->blocks[0].data.ptr = NULL;
            }

            pContainer->basicIn[0] = index;
            pContainer->basicIn[1] = (uint32_t)padtype;
            pContainer->basicIn[2] = (uint32_t)mode;
            pContainer->basicIn[3] =  keyslotInfo.keySlotNumber;

            /*
             * If meta is NULL, this indicates decryption, otherwise look at meta->encrypt  Ignored for rest
             * */
            if(pMeta == NULL)
            {
                BDBG_MSG(("%s -  specifying DECRYPTION operation for context", __FUNCTION__));
                pContainer->basicIn[4] = 0; /* DECRYPT */
            }
            else
            {
                BDBG_MSG(("%s -  specifying ENCRYPTION operation for context", __FUNCTION__));
                if(pMeta->encrypt != 0)
                {
                    BDBG_MSG(("%s -  meta->encrypt != 0, setting encrypt operation", __FUNCTION__));
                    pContainer->basicIn[4] = 1; /* ENCRYPT */
                }
                else
                {
                    BDBG_MSG(("%s -  meta->encrypt == 0 setting decrypt operation", __FUNCTION__));
                    pContainer->basicIn[4] = 0; /* DECRYPT */
                }
            }

            sage_rc = SRAI_Module_ProcessCommand(moduleHandle, DrmAdobe_CommandId_eCreateSessionContext, pContainer);
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
                rc = Drm_Err;
                goto ErrorExit;
            }

            /* if success, extract status from pContainer */
            sage_rc = pContainer->basicOut[0];
            if(sage_rc != BERR_SUCCESS)
            {
                *pCTX = NULL;
                BDBG_MSG(("%s - Operation failed. Setting session context to NULL", __FUNCTION__));

                if(pContainer->blocks[0].data.ptr != NULL){
                    SRAI_Memory_Free(pContainer->blocks[0].data.ptr);
                }

                BDBG_MSG(("%s - Freeing allocated context", __FUNCTION__));

                if(pSessCtx != NULL){
                    BKNI_Free(pSessCtx);
                }
            }
        }
    }/* end of if(sessCtx->bUseCtxforDiskIO == false) */

    else
    {
        BDBG_MSG(("%s - session context created for DiskIO operations. Index value '0x%08x' will be ignored for future operations that are not DiskIO related", __FUNCTION__,index));
        pSessCtx->drmCommonOpStruct.keyConfigSettings.keySlot = NULL;
        pSessCtx->drmCommonOpStruct.pKeyLadderInfo = NULL;
        pSessCtx->drmCommonOpStruct.keySrc = CommonCrypto_eOtpKey;

        /* augment the secure_store API by being able to specify custom procIns? */
    }

    BDBG_MSG(("%s - returning session context %p", __FUNCTION__, (void *)pSessCtx));
    *pCTX = pSessCtx;


ErrorExit:

    if(pContainer != NULL)
    {
        if(pContainer->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(pContainer->blocks[0].data.ptr);
            pContainer->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(pContainer);
        pContainer = NULL;
    }

    BDBG_LEAVE(DRM_Adobe_CreateDRMSessionCtx);
    return rc;
}

/*****************************************************************************************
Function: DRM_Adobe_DestroyDRMSessionCtx
******************************************************************************************/
void DRM_Adobe_DestroyDRMSessionCtx(DrmAdobeSessionContext_t *pCTX)
{
    BERR_Code sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *pContainer = NULL;
    DrmAdobeKeyIndexMap_t *pSearchEntry = NULL;

    BDBG_ENTER(DRM_Adobe_DestroyDRMSessionCtx);

    if(pCTX == NULL)
    {
        BDBG_MSG(("%s - nothing to do since context is null", __FUNCTION__));
        goto ErrorExit;
    }

    if((DRM_Adobe_HostKeyIndexMapList_Search(&pSearchEntry, pCTX->adobeIndex) == false) && (pCTX->bUseCtxforDiskIO == false))
    {
        BDBG_MSG(("%s - Index associated with SAGE-side context '0x%08x'", __FUNCTION__, pCTX->adobeIndex));
        pContainer = SRAI_Container_Allocate();
        if(pContainer == NULL)
        {
            BDBG_ERR(("%s - Error allocating pContainer", __FUNCTION__));
            goto ErrorExit;
        }

        /* Send index (associated to context) to destroy */
        pContainer->basicIn[0] = pCTX->adobeIndex;

        if (SRAI_Module_ProcessCommand(moduleHandle, DrmAdobe_CommandId_eDestroySessionContext, pContainer) != BERR_SUCCESS){
            BDBG_ERR(("%s - Error destroying context", __FUNCTION__));
        }

        sage_rc = pContainer->basicOut[0];
        if (sage_rc != BERR_SUCCESS){
            BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", __FUNCTION__, sage_rc));
        }
    }

    /* regardless of host or sage side context, free the keyslot */
    if(pCTX->drmCommonOpStruct.keyConfigSettings.keySlot != NULL)
    {
        BDBG_MSG(("%s - Freeing keyslot...", __FUNCTION__));
        NEXUS_Security_FreeKeySlot(pCTX->drmCommonOpStruct.keyConfigSettings.keySlot);

        pCTX->drmCommonOpStruct.keyConfigSettings.keySlot = NULL;
    }

    BDBG_MSG(("%s - Freeing context memory...", __FUNCTION__));
    BKNI_Free(pCTX);
    pCTX = NULL;

ErrorExit:
    if(pContainer != NULL){
        SRAI_Container_Free(pContainer);
    }

    BDBG_LEAVE(DRM_Adobe_DestroyDRMSessionCtx);

    return;
}


/*****************************************************************************************
Function: DRM_Adobe_DecryptInit
******************************************************************************************/
DrmRC DRM_Adobe_DecryptInit(DrmAdobeSessionContext_t *pCTX)
{
    DrmRC rc = Drm_Success;
    BSAGElib_InOutContainer *pContainer = NULL;
    BERR_Code sage_rc = BERR_SUCCESS;
    DrmAdobeKeyIndexMap_t *pSearchEntry = NULL;

    BDBG_ENTER(DRM_Adobe_DecryptInit);

    if(pCTX == NULL)
    {
        BDBG_MSG(("%s - Context is null", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    /* see if init has been called twice with the same context (error?) */
    if(pCTX->firstChunkDecrypted == true)
    {
        BDBG_WRN(("%s -  Context is resetting IV since first chunk was already decrypted", __FUNCTION__));
    }

    /**/
    if(pCTX->drmCommonOpStruct.keyConfigSettings.settings.algVariant == NEXUS_SecurityAlgorithmVariant_eCbc)
    {
        DRM_MSG_PRINT_BUF("pCTX->drmCommonOpStruct.keyIvSettings.iv (current iv)", pCTX->drmCommonOpStruct.keyIvSettings.iv, pCTX->drmCommonOpStruct.keyIvSettings.ivSize);
        DRM_MSG_PRINT_BUF("pCTX->iv0", pCTX->iv0, 16);

        /* if index is on host side, update normally, otherwise send to sage */
        if(DRM_Adobe_HostKeyIndexMapList_Search(&pSearchEntry, pCTX->adobeIndex) == true)
        {
            BKNI_Memcpy(pCTX->drmCommonOpStruct.keyIvSettings.iv, pCTX->iv0, DRM_ADOBE_IV_SIZE);

            /* update the host side keyslot with the IV */
            if(DRM_Common_KeyConfigOperation(&pCTX->drmCommonOpStruct) != Drm_Success)
            {
                BDBG_ERR(("%s - Error configuring keyslot", __FUNCTION__));
                rc = Drm_Err;
                goto ErrorExit;
            }
        }
        else
        {
            BDBG_MSG(("%s - updating IV on SAGE side", __FUNCTION__));

            pContainer = SRAI_Container_Allocate();
            if(pContainer == NULL)
            {
                BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
                rc = Drm_Err;
                goto ErrorExit;
            }

            pContainer->basicIn[0] = (uint32_t)pCTX->adobeIndex;

            pContainer->blocks[0].data.ptr = SRAI_Memory_Allocate(16, SRAI_MemoryType_Shared);
            if(pContainer->blocks[0].data.ptr == NULL)
            {
                BDBG_ERR(("%s - error allocating memory", __FUNCTION__));
                rc = Drm_MemErr;
                goto ErrorExit;
            }
            pContainer->blocks[0].len = 16;
            BKNI_Memcpy(pContainer->blocks[0].data.ptr, pCTX->iv0, DRM_ADOBE_IV_SIZE);

            sage_rc = SRAI_Module_ProcessCommand(moduleHandle, DrmAdobe_CommandId_eDecryptInit, pContainer);
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
                rc = Drm_MemErr;
                goto ErrorExit;
            }

            sage_rc = pContainer->basicOut[0];
            if (sage_rc != BERR_SUCCESS)
            {
                BDBG_ERR(("%s - Command was sent succuessfully to init key but actual operation failed (0x%08x)", __FUNCTION__, sage_rc));
                rc = Drm_Err;
                goto ErrorExit;
            }
        }
    }
    else
    {
        BDBG_ERR(("%s -  Mode type not supported", __FUNCTION__));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    pCTX->firstChunkDecrypted = false;

ErrorExit:

    if(pContainer != NULL)
    {
        if(pContainer->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(pContainer->blocks[0].data.ptr);
            pContainer->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(pContainer);
    }
    BDBG_LEAVE(DRM_Adobe_DecryptInit);

    return rc;
}


/*****************************************************************************************
Function: DRM_Adobe_DecryptUpdate
******************************************************************************************/
DrmRC DRM_Adobe_DecryptUpdate(  DrmAdobeSessionContext_t *pCTX,/* Input: Pointer to HW DRM context. Can be NULL if not required by HW */
                                uint8_t *pInput,                /* Input: Data to be decrypted */
                                uint32_t length,            /* Input: Length of data to be decrypted */
                                uint8_t *pOutput,            /* Output: Decrypted Output Data. Memory to be malloced by caller */
                                uint32_t *pOutLength)    /* Output: Data Length */
{
    DrmRC  rc                           = Drm_Success;
    uint8_t* pInputDecMem              = NULL;
    uint32_t decryptLength              = 0;
    uint32_t leftOverLength             = 0;
    BSAGElib_InOutContainer *pContainer = NULL;
    DrmAdobeKeyIndexMap_t *pSearchEntry = NULL;
    BERR_Code sage_rc                   = BERR_SUCCESS;

    BDBG_ENTER(DRM_Adobe_DecryptUpdate);

    if(pCTX == NULL)
    {
        BDBG_MSG(("%s - Context is null", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    leftOverLength = (pCTX->prevTrailingBytesSize + length)%16;
    decryptLength = length - leftOverLength;

    if(pCTX->drmCommonOpStruct.keyConfigSettings.settings.algVariant == NEXUS_SecurityAlgorithmVariant_eCbc)
    {
        /* if the first chunk was already decrypted with Iv0 then we can overwite the keyslot with the new IV */
        if(pCTX->firstChunkDecrypted == true)
        {
            /* if index is on host side, update normally, otherwise send to sage */
            if(DRM_Adobe_HostKeyIndexMapList_Search(&pSearchEntry, pCTX->adobeIndex) == true)
            {
                BDBG_MSG(("%s - Index found on host side", __FUNCTION__));
                BKNI_Memcpy(pCTX->drmCommonOpStruct.keyIvSettings.iv, pCTX->iv_curr, DRM_ADOBE_IV_SIZE);

                /* update the host side keyslot with the IV */
                if(DRM_Common_KeyConfigOperation(&pCTX->drmCommonOpStruct) != Drm_Success)
                {
                    BDBG_ERR(("%s - Error configuring keyslot", __FUNCTION__));
                    rc = BERR_INVALID_PARAMETER;
                    goto ErrorExit;
                }
            }
            else
            {
                BDBG_MSG(("%s - first chunk already decrypted, sending update to SAGE", __FUNCTION__));
                pContainer = SRAI_Container_Allocate();
                if(pContainer == NULL)
                {
                    BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
                    rc = Drm_Err;
                    goto ErrorExit;
                }

                pContainer->basicIn[0] = (uint32_t)pCTX->adobeIndex;

                pContainer->blocks[0].data.ptr = SRAI_Memory_Allocate(16, SRAI_MemoryType_Shared);
                if(pContainer->blocks[0].data.ptr == NULL)
                {
                    BDBG_ERR(("%s - error allocating memory", __FUNCTION__));
                    rc = Drm_MemErr;
                    goto ErrorExit;
                }
                pContainer->blocks[0].len = 16;
                BKNI_Memcpy(pContainer->blocks[0].data.ptr, pCTX->iv_curr, DRM_ADOBE_IV_SIZE);

                sage_rc = SRAI_Module_ProcessCommand(moduleHandle, DrmAdobe_CommandId_eDecryptUpdate, pContainer);
                if (sage_rc != BERR_SUCCESS)
                {
                    BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
                    rc = Drm_Err;
                    goto ErrorExit;
                }

                sage_rc = pContainer->basicOut[0];
                if (sage_rc != BERR_SUCCESS)
                {
                    BDBG_ERR(("%s - Command was sent succuessfully to update key but actual operation failed (0x%08x)", __FUNCTION__, sage_rc));
                    rc = Drm_Err;
                    goto ErrorExit;
                }
            }
        }
        else
        {
            BDBG_MSG(("%s -  Iv0 already loaded therefore skipped loading IV", __FUNCTION__));
        }
    }
    else
    {
        BDBG_ERR(("%s -  Mode type not supported", __FUNCTION__));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    DRM_Common_MemoryAllocate(&pInputDecMem, decryptLength);
    if(pInputDecMem == NULL)
    {
        rc = Drm_MemErr;
        BDBG_ERR(("%s -  Error Allocating drm Memory for input dec buf", __FUNCTION__));
        goto ErrorExit;
    }

    /* first copy the bytes leftover in previous call */
    BDBG_MSG(("%s -  Leftover bytes from previous call '%u'", __FUNCTION__, pCTX->prevTrailingBytesSize));
    BKNI_Memcpy(pInputDecMem, pCTX->prevTrailingBytes, pCTX->prevTrailingBytesSize);

    /*then copy the current bytes in this call */
    BKNI_Memcpy(&pInputDecMem[pCTX->prevTrailingBytesSize], pInput, (decryptLength - pCTX->prevTrailingBytesSize));


    /* now save the left over bytes and their size */
    pCTX->prevTrailingBytesSize = leftOverLength;
    BDBG_MSG(("%s -  UPDATED Leftover bytes for next call '%u'", __FUNCTION__, pCTX->prevTrailingBytesSize));

    /* save the last 16 bytes of the current data as next IV */
    BKNI_Memcpy(pCTX->iv_curr, &pInputDecMem[decryptLength-16], 16);

    /* save the leftover bytes for next round */
    BKNI_Memcpy(pCTX->prevTrailingBytes, &pInput[length-leftOverLength], leftOverLength);


    rc = DRM_Adobe_DecryptFullSampleAndReturn(pCTX, pInputDecMem, &decryptLength, pOutput, pOutLength);
    if(rc != Drm_Success)
    {
       BDBG_ERR(("%s -  Error decrypting", __FUNCTION__));
       goto ErrorExit;
    }

    if(pCTX->firstChunkDecrypted == false)
    {
        BDBG_MSG(("%s -  Iv0 used therefore subsequent Ivs can loaded, first chunk has been decrypted.", __FUNCTION__));
        pCTX->firstChunkDecrypted = true;
    }

ErrorExit:
    if(pInputDecMem != NULL)
    {
        DRM_Common_MemoryFree(pInputDecMem);
        pInputDecMem = NULL;
    }

    BDBG_LEAVE(DRM_Adobe_DecryptUpdate);
    return rc;
}


/*****************************************************************************************
Function: DRM_Adobe_DecryptFinal
******************************************************************************************/
DrmRC DRM_Adobe_DecryptFinal(   DrmAdobeSessionContext_t *pCTX,/* Input: Pointer to HW DRM context. Can be NULL if not required by HW */
                                uint8_t *pInput,                /* Input: Data to be decrypted */
                                uint32_t length,            /* Input: Length of data to be decrypted */
                                uint8_t *pOutput,            /* Output: Decrypted Output Data. Memory to be malloced by caller */
                                uint32_t *pOutLength)        /* Output: Data Length */
{

    DrmRC  rc                           = Drm_Success;
    uint8_t* pInputDecMem               = NULL;
    DrmAdobeKeyIndexMap_t *pSearchEntry = NULL;
    BSAGElib_InOutContainer *pContainer = NULL;
    BERR_Code sage_rc                   = BERR_SUCCESS;

    BDBG_ENTER(DRM_Adobe_DecryptFinal);
    BSTD_UNUSED(pInput);

    if(pCTX == NULL)
    {
        BDBG_MSG(("%s - Context is null", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(pCTX->drmCommonOpStruct.keyConfigSettings.settings.algVariant == NEXUS_SecurityAlgorithmVariant_eCbc)
    {
        /* if the first chunk was already decrypted with Iv0 then we can overwite the keyslot with the new IV */
        if(pCTX->firstChunkDecrypted == true)
        {
            /* if index is on host side, update normally, otherwise send to sage */
            if(DRM_Adobe_HostKeyIndexMapList_Search(&pSearchEntry, pCTX->adobeIndex) == true)
            {
                BKNI_Memcpy(pCTX->drmCommonOpStruct.keyIvSettings.iv, pCTX->iv_curr, DRM_ADOBE_IV_SIZE);

                /* update the host side keyslot with the IV */
                if(DRM_Common_KeyConfigOperation(&pCTX->drmCommonOpStruct) != Drm_Success)
                {
                    BDBG_ERR(("%s - Error configuring keyslot", __FUNCTION__));
                    rc = BERR_INVALID_PARAMETER;
                    goto ErrorExit;
                }
            }
            else
            {
                pContainer = SRAI_Container_Allocate();
                if(pContainer == NULL)
                {
                    BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
                    rc = Drm_Err;
                    goto ErrorExit;
                }

                pContainer->basicIn[0] = (uint32_t)pCTX->adobeIndex;

                pContainer->blocks[0].data.ptr = SRAI_Memory_Allocate(DRM_ADOBE_IV_SIZE, SRAI_MemoryType_Shared);
                if(pContainer->blocks[0].data.ptr == NULL)
                {
                    BDBG_ERR(("%s - error allocating memory", __FUNCTION__));
                    rc = Drm_MemErr;
                    goto ErrorExit;
                }
                pContainer->blocks[0].len = DRM_ADOBE_IV_SIZE;
                BKNI_Memcpy(pContainer->blocks[0].data.ptr, pCTX->iv_curr, DRM_ADOBE_IV_SIZE);

                sage_rc = SRAI_Module_ProcessCommand(moduleHandle, DrmAdobe_CommandId_eDecryptUpdate, pContainer);
                if (sage_rc != BERR_SUCCESS)
                {
                    BDBG_ERR(("%s - Error loading key parameters", __FUNCTION__));
                    rc = Drm_Err;
                    goto ErrorExit;
                }

                sage_rc = pContainer->basicOut[0];/* double check */
                if (sage_rc != BERR_SUCCESS)
                {
                    BDBG_ERR(("%s - Command was sent succuessfully to update key but actual operation failed (0x%08x)", __FUNCTION__, sage_rc));
                    rc = Drm_Err;
                    goto ErrorExit;
                }
            }
        }
        else
        {
            BDBG_MSG(("%s -  Iv0 already loaded therefore skipped loading IV", __FUNCTION__));
        }
    }
    else
    {
        BDBG_ERR(("%s -  Mode type not supported", __FUNCTION__));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    BDBG_MSG(("%s -  prevTrailingBytesSize  = '%u' input length = '%u'   OutLength = '%u' ", __FUNCTION__, pCTX->prevTrailingBytesSize, length, (*pOutLength)));
    if(pCTX->prevTrailingBytesSize != 0)
    {
        DRM_Common_MemoryAllocate(&pInputDecMem, pCTX->prevTrailingBytesSize);
        if(pInputDecMem == NULL)
        {
            rc = Drm_MemErr;
            BDBG_ERR(("%s -  Error Allocating drm Memory for input dec buf", __FUNCTION__));
            goto ErrorExit;
        }

        BKNI_Memcpy(pInputDecMem, pCTX->prevTrailingBytes, pCTX->prevTrailingBytesSize);

        rc = DRM_Adobe_DecryptFullSampleAndReturn(pCTX, pInputDecMem, &pCTX->prevTrailingBytesSize, pOutput, pOutLength);
        if(rc !=Drm_Success)
        {
           BDBG_ERR(("%s -  Error decrypting", __FUNCTION__));
           goto ErrorExit;
        }
    }
    else{
        BDBG_MSG(("%s -  length of trailing bytes = '%u'", __FUNCTION__, pCTX->prevTrailingBytesSize));
    }

ErrorExit:

    if(pContainer != NULL)
    {
        if(pContainer->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(pContainer->blocks[0].data.ptr);
            pContainer->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(pContainer);
    }

    if(pInputDecMem != NULL){
        DRM_Common_MemoryFree(pInputDecMem);
        pInputDecMem = NULL;
    }

    BDBG_LEAVE(DRM_Adobe_DecryptFinal);

    return rc;

}

/*****************************************************************************************
Function: DRM_Adobe_DecryptFullSampleAndReturn
******************************************************************************************/
DrmRC DRM_Adobe_DecryptFullSampleAndReturn(DrmAdobeSessionContext_t *pCTX,/* Input: Pointer to HW DRM context. Can be NULL if not required by HW */
                                        uint8_t *pInput,                /* Input/Output: Data to be inline decrypted */
                                        uint32_t *pLength,            /* Input/Output: Length of encrypted data, and length of decrypted data */
                                        uint8_t *pOutput,            /* Output: Decrypted Output Data. Memory to be malloced by caller */
                                        uint32_t *pOutLength)
{
    DrmRC  rc = Drm_Success;
    DmaBlockInfo_t dmaBlock;
    uint8_t *pInputNexusMem = NULL;
    uint8_t *pOutputNexusMem = NULL;
    DrmCommonOperationStruct_t drmCommonOpStruct;

    BDBG_MSG(("%s - Entered function.  Session context %p, data length is '%u'", __FUNCTION__, (void *)pCTX, *pLength));

    if(pCTX == NULL)
    {
        BDBG_MSG(("%s - Context is null", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = DRM_Common_MemoryAllocate((uint8_t**)&pInputNexusMem, *pLength);
    if(rc !=Drm_Success)
    {
       BDBG_ERR(("%s -  Error Allocating drm Memory for input", __FUNCTION__));
       goto ErrorExit;
    }

    BKNI_Memcpy(pInputNexusMem, pInput, *pLength);

    /* if bi-directional is not defined we have to allocate a local buffer */
    if((capabilities & DrmAdobe_CAPABILITY_BI_DIRECTIONAL_MALLOC) == 0)
    {
        BDBG_MSG(("%s - Capability is NOT bi-directional", __FUNCTION__));
        rc = DRM_Common_MemoryAllocate((uint8_t**)&pOutputNexusMem, *pLength);
        if(rc !=Drm_Success)
        {
           BDBG_ERR(("%s - Error Allocating drm Memory for output", __FUNCTION__));
           goto ErrorExit;
        }
    }
    else{
        pOutputNexusMem = pOutput;
    }

    if(pCTX->bUseCtxforDiskIO == false)
    {
        /* set DMA parameters */
        dmaBlock.pDstData   = pOutputNexusMem;
        dmaBlock.pSrcData   = pInputNexusMem;
        dmaBlock.uiDataSize = *pLength;
        dmaBlock.sg_start   = true;
        dmaBlock.sg_end     = true;

        drmCommonOpStruct.pDmaBlock     = &dmaBlock;
        drmCommonOpStruct.num_dma_block = 1;

        drmCommonOpStruct.keyConfigSettings.keySlot = pCTX->drmCommonOpStruct.keyConfigSettings.keySlot;
        BDBG_MSG(("%s - drmCommonOpStruct.keyConfigSettings.keySlot = %p", __FUNCTION__, (void *)drmCommonOpStruct.keyConfigSettings.keySlot));

        /*start M2M transfer*/
        rc = DRM_Common_M2mOperation(&drmCommonOpStruct);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - Call to 'DRM_Common_M2mOperation' failed", __FUNCTION__));
            goto ErrorExit;
        }
    }
    else
    {
        rc = DRM_SecureStore_BufferOperation(pInputNexusMem, *pLength, pOutputNexusMem, DrmDestinationType_eExternal, DrmCryptoOperation_eDecrypt);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s:secure store buffer operation failed",__FUNCTION__));
            goto ErrorExit;
        }
    }

    /* if bi-directional is not defined we have to copy form the local buffer to output */
    if((capabilities & DrmAdobe_CAPABILITY_BI_DIRECTIONAL_MALLOC) == 0)
    {
        if(pCTX->bUseCtxforDiskIO == false){
            BDBG_MSG(("%s - Copying result of M2M operation", __FUNCTION__));
        }
        else{
            BDBG_MSG(("%s - Copying result of secure store operation", __FUNCTION__));
        }
        BKNI_Memcpy(pOutput, pOutputNexusMem, *pLength);
        DRM_Common_MemoryFree(pOutputNexusMem);
        pOutputNexusMem = NULL;
    }

    *pOutLength = *pLength; /*this needs to be length - padding. need to fix*/

    /*DRM_MSG_PRINT_BUF("output", output, *length);*/

ErrorExit:
    if(pInputNexusMem != NULL){
        DRM_Common_MemoryFree(pInputNexusMem);
        pInputNexusMem = NULL;
    }

    BDBG_LEAVE(DRM_Adobe_DecryptFullSampleAndReturn);

    return rc;
}

/*****************************************************************************************
Function: DRM_Adobe_SetClearKey
******************************************************************************************/
DrmRC DRM_Adobe_SetClearKey(uint8_t *pKey,
                            uint32_t length,
                            DrmAdobe_KeyType_e keyType,
                            DrmAdobe_KeyLadderType_e keyladderType,
                            uint32_t *pIndex)
{
    /*Add the Sw publick key to the keyindex list*/
    DrmAdobeKeyIndexMap_t pKeyEntry;
    DrmRC rc = Drm_Success;

    BDBG_ENTER(DRM_Adobe_SetClearKey);

    BKNI_Memset(&pKeyEntry, 0x0 ,sizeof(DrmAdobeKeyIndexMap_t));

    pKeyEntry.swKey = true;
    pKeyEntry.keybufsz = length;
    pKeyEntry.keyType   = keyType;

    if(length != 0)
    {
        BDBG_MSG(("%s - Allocating '%u' bytes for key", __FUNCTION__, length));
        pKeyEntry.keybuf = (uint8_t*)BKNI_Malloc(length);
        if(pKeyEntry.keybuf == NULL)
        {
            BDBG_ERR(("%s - error allocating '%u' bytes for clearkey", __FUNCTION__, length));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        BKNI_Memcpy(pKeyEntry.keybuf, pKey, length);
    }
    else
    {
        BDBG_ERR(("%s - Cannot set 0 bytes for clearkey", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if((keyladderType == DrmAdobe_KeyLadderType_eSESSION_KEY) && (keyType == DrmAdobe_KeyType_eAES128))
    {
        BDBG_MSG(("%s - Setting clear sw key", __FUNCTION__));
        pKeyEntry.adobeIndex = DrmAdobe_AdobeKeyIndex_eSwKey|gHostKeySequenceNumber;
        DRM_MSG_PRINT_BUF("swkey", pKey, length);
    }
    else if((keyladderType == DrmAdobe_KeyLadderType_eTRANSPORT_PUB) && (keyType == DrmAdobe_KeyType_eRSA1024 || keyType == DrmAdobe_KeyType_eRSA2048))
    {
        BDBG_MSG(("%s - Setting public transport key", __FUNCTION__));
        pKeyEntry.adobeIndex = DrmAdobe_AdobeKeyIndex_eTransportPublicKey|gHostKeySequenceNumber;
         DRM_MSG_PRINT_BUF("swkey", pKey, length);
    }
    else if((keyladderType == DrmAdobe_KeyLadderType_eTRANSPORT_PRIV) && (keyType == DrmAdobe_KeyType_eRSA1024 || keyType == DrmAdobe_KeyType_eRSA2048))
    {
        BDBG_MSG(("%s - Setting private transport key", __FUNCTION__));
        pKeyEntry.adobeIndex = DrmAdobe_AdobeKeyIndex_eTransportPrivateKey|gHostKeySequenceNumber;
         DRM_MSG_PRINT_BUF("swkey", pKey, length);
    }
    else
    {
        BDBG_ERR(("%s - key type (%u) and algorithm (%u) not supported", __FUNCTION__, keyType, keyladderType));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = DRM_Adobe_HostKeyIndexMapList_AddEntry(&pKeyEntry);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error loading sw key", __FUNCTION__));
        goto ErrorExit;
    }
    else{
        *pIndex = pKeyEntry.adobeIndex;
    }

    BDBG_MSG(("%s - Sw key successfully loaded and assigned index 0x%08x", __FUNCTION__, (*pIndex)));

ErrorExit:
    BDBG_LEAVE(DRM_Adobe_SetClearKey);
    return rc;
}

/*****************************************************************************************
Function: DRM_Adobe_GetDefaultRsaSettings
******************************************************************************************/
void DRM_Adobe_GetDefaultRsaSettings(
        DrmAdobe_RsaSwSettings_t *pAdobeRsaSettings)
{
    BDBG_ENTER(DRM_Adobe_GetDefaultRsaSettings);

    BKNI_Memset((uint8_t*)pAdobeRsaSettings, 0x00, sizeof(DrmAdobe_RsaSwSettings_t));

    BDBG_LEAVE(DRM_Adobe_GetDefaultRsaSettings);
    return;
}


/*****************************************************************************************
Function: DRM_Adobe_PublicKeyOperation
******************************************************************************************/
DrmRC DRM_Adobe_PublicKeyOperation( uint32_t index,         /* Input: Index of key to use */
                                    DrmAdobe_RsaSwSettings_t *pAdobeRsaSettings)
{
    DrmRC rc = Drm_Success;
    DrmCommon_RsaKey_t *pPublicKey = NULL;
    DrmCommon_RsaSwParam_t rsa_params;
    DrmAdobeKeyIndexMap_t *pPubKey = NULL;
    char algo[]="RSA-SHA256";


    BDBG_MSG(("Entered Function %s, index = 0x%x",__FUNCTION__,index));

    BKNI_Memset((void *)&rsa_params, 0x0, sizeof(DrmCommon_RsaSwParam_t));

    if(DRM_Common_MemoryAllocate((uint8_t **)&pPublicKey, (sizeof(struct DrmCommon_RsaKey_t ))) != Drm_Success)
    {
        BDBG_ERR(("%s: Memory Allocation err",__FUNCTION__));
        return Drm_MemErr;
        goto ErrorExit;
    }
    if(DRM_Adobe_HostKeyIndexMapList_Search(&pPubKey, index)==false)
    {
        BDBG_ERR(("%s - Index 0x%x not found on host side", __FUNCTION__,index));
        return Drm_MemErr;
        goto ErrorExit;
    }
    //DRM_Adobe_KeyIndexMapList_Search(&pPubKey, index);
    BDBG_MSG((" %s: populate the public key openssl struct,pubKey->keybufsz is %d ",__FUNCTION__,pPubKey->keybufsz));
    if (pPubKey->keyType == DrmAdobe_KeyType_eRSA1024)
    {
        pPublicKey->n.len = DRM_ADOBE_RSA1024_KEYSZ;
        strcpy(algo,"RSA-SHA1");
    }
    else if (pPubKey->keyType == DrmAdobe_KeyType_eRSA2048)
    {
        pPublicKey->n.len = DRM_ADOBE_RSA2048_KEYSZ;
    }
    else
    {
         BDBG_ERR(("%s: Unsupported RSA key size.",__FUNCTION__));
         rc = Drm_Err;
         goto ErrorExit;
    }


    pPublicKey->n.pData = BKNI_Malloc(BCRYPT_2048_RSA_KEY_SZ);
    pPublicKey->e.pData = BKNI_Malloc(BCRYPT_2048_RSA_KEY_SZ);
    DRM_Common_GetRSA_From_SubjectPublicKeyInfo(pPubKey->keybuf, pPubKey->keybufsz, pPublicKey);

    //DRM_MSG_PRINT_BUF("rsa pub key mod",pPublicKey->n.pData, pPublicKey->n.len);
    pPublicKey->e.len=DRM_ADOBE_RSA_PUBLIC_EXP_SZ;
    //DRM_MSG_PRINT_BUF("pub exp",pPublicKey->e.pData, pPublicKey->e.len);

    rsa_params.key = pPublicKey;
    if (pAdobeRsaSettings->operation_type ==  DrmAdobe_RsaPubkeyOpType_eEnc)
    {
        BDBG_MSG((" %s:pub key encryption",__FUNCTION__));
        rsa_params.bRSAop = drmRsaenc;
        rsa_params.pbDataIn = pAdobeRsaSettings->operation_struct.encrypt_op.pSrcAddr;
        rsa_params.cbDataIn = pAdobeRsaSettings->operation_struct.encrypt_op.srcDataSize;
        // DRM_MSG_PRINT_BUF("source",pAdobeRsaSettings->operation_struct.encrypt_op.pSrcAddr, pAdobeRsaSettings->operation_struct.encrypt_op.srcDataSize);
        rsa_params.pbDataOut = pAdobeRsaSettings->operation_struct.encrypt_op.pDestAddr;
        rsa_params.cbDataOut = (unsigned long *)pAdobeRsaSettings->operation_struct.encrypt_op.pDestDataSize;
        switch(pAdobeRsaSettings->operation_struct.encrypt_op.padType)
        {
        case DrmAdobe_PadType_ePKCS1_OAEP:
            rsa_params.padType     =  DrmCommon_RSAPaddingType_eOAEP;
            break;
        case DrmAdobe_PadType_eNO_PADDING:
            rsa_params.padType     =  DrmCommon_RSAPaddingType_eNOPADDING;
            break;
        case DrmAdobe_PadType_ePKCS1V15:
        default:
            rsa_params.padType     =  DrmCommon_RSAPaddingType_ePKCS1;
        }
    }
    else if (pAdobeRsaSettings->operation_type ==  DrmAdobe_RsaPubkeyOpType_eVerify)
    {

        BDBG_MSG((" %s: pub key decryption/verify optype=%d ",__FUNCTION__,pAdobeRsaSettings->operation_type));
        rsa_params.bRSAop = drmRsaverify;

        /*The openssl RSA APi for public key verify takes the digest as pbDataIn,  signature as pbDataOut.
        *computesthe signature on the digest and verifies with the supplied signature in pbDataOut*/

        rsa_params.pbDataIn = pAdobeRsaSettings->operation_struct.verify_op.pDigestAddr;
        //DRM_MSG_PRINT_BUF("input/signature",pAdobeRsaSettings->operation_struct.verify_op.pSignatureAddr, pAdobeRsaSettings->operation_struct.verify_op.signatureSize);
        rsa_params.cbDataIn = pAdobeRsaSettings->operation_struct.verify_op.digestSize;
        BDBG_MSG((" %s:pSrcAddr=%p,size=%d ",__FUNCTION__,(void *)pAdobeRsaSettings->operation_struct.verify_op.pDigestAddr,pAdobeRsaSettings->operation_struct.verify_op.digestSize));
        rsa_params.pbDataOut =  pAdobeRsaSettings->operation_struct.verify_op.pSignatureAddr;
        rsa_params.cbDataOut = (unsigned long *)&pAdobeRsaSettings->operation_struct.verify_op.signatureSize;
        switch(pAdobeRsaSettings->operation_struct.verify_op.padType)
        {
        case DrmAdobe_PadType_ePKCS1_OAEP:
            rsa_params.padType     =  DrmCommon_RSAPaddingType_eOAEP;
            break;
        case DrmAdobe_PadType_eNO_PADDING:
            rsa_params.padType     =  DrmCommon_RSAPaddingType_eNOPADDING;
            break;
        case DrmAdobe_PadType_ePKCS1V15:
        default:
            rsa_params.padType     =  DrmCommon_RSAPaddingType_ePKCS1;
        }

    }
    else
    {
        BDBG_ERR(("%s:Invalid public key operation",__FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }


    DRM_MSG_PRINT_BUF("rsa_params.pbDatain:",rsa_params.pbDataIn,rsa_params.cbDataIn);

    BDBG_MSG((" %s: program rsa params 3 ",__FUNCTION__));

    rsa_params.psAlgorithmId = (unsigned char *)algo;
    rsa_params.csAlgorithmId = sizeof(algo);;


    BDBG_MSG((" %s: program rsa params 4 ",__FUNCTION__));

    BDBG_MSG(("%s: rsa params output len=%lu ",__FUNCTION__, *rsa_params.cbDataOut));

    BDBG_MSG(("%s: calling DRM_Common_SwRsa",__FUNCTION__));

    rc = DRM_Common_SwRsa(&rsa_params);
    BDBG_MSG(("%s:cbDataIn=%lu, cbDataOut=%p",__FUNCTION__,rsa_params.cbDataIn, (void *)rsa_params.cbDataOut));

    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s :Public key operation failed",__FUNCTION__));
        goto ErrorExit;

    }

    DRM_MSG_PRINT_BUF("rsa_params.pbDataOut:",rsa_params.pbDataOut,*rsa_params.cbDataOut);

ErrorExit:
    if(pPublicKey->n.pData)
        BKNI_Free(pPublicKey->n.pData);
    if(pPublicKey->e.pData)
        BKNI_Free(pPublicKey->e.pData);
    if(pPublicKey)
       DRM_Common_MemoryFree((uint8_t*)pPublicKey);

    BDBG_MSG(("Exiting Function %s",__FUNCTION__));
    return rc;
}


/*****************************************************************************************
Function: DRM_Adobe_PrivateKeyEncrypt
******************************************************************************************/
DrmRC DRM_Adobe_PrivateKeyEncrypt(
    uint32_t index,         /* Input: Index of key to use */
    DrmAdobe_RsaSwSettings_t *pAdobeRsaSettings)
{
    DrmRC rc = Drm_Success;
    BSAGElib_InOutContainer *pContainer = NULL;
    BERR_Code sage_rc = BERR_SUCCESS;
    DrmAdobeKeyIndexMap_t *pKey = NULL;
    DrmCommon_RsaSwParam_t rsaSwIO;
    DrmCommon_RsaKey_t rsaKey;

    BDBG_ENTER(DRM_Adobe_PrivateKeyEncrypt);
    BDBG_MSG(("%s:srcDataSize = %d",__FUNCTION__,pAdobeRsaSettings->operation_struct.sign_op.srcDataSize));
    BDBG_MSG(("%s:*signatureSize = %d",__FUNCTION__,*pAdobeRsaSettings->operation_struct.sign_op.signatureSize));

    if(index > DrmAdobe_AdobeKeyIndex_eSwKey)
    {

        DRM_Adobe_HostKeyIndexMapList_Search(&pKey, index);
    }


    if(index == 0)
    {
        index = DrmAdobe_AdobeKeyIndex_eRsaDrmPrivSigningkey;
    }

    if(pAdobeRsaSettings->operation_struct.sign_op.pSignatureAddr == NULL)
    {
        BDBG_ERR(("%s - Output buffer is null", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    if(index == DrmAdobe_AdobeKeyIndex_eRsaDrmPrivSigningkey) /*call sage */
    {

        BDBG_LOG(("%s:Private key signing with HW key, calling Sage",__FUNCTION__));
        pContainer = SRAI_Container_Allocate();
        if(pContainer == NULL)
        {
            rc = Drm_MemErr;
            BDBG_ERR(("%s - Error allocating SRAI pContainer", __FUNCTION__));
            goto ErrorExit;
        }

        /* pass index */
        pContainer->basicIn[0] = index;

        /* allocate shared buffer and copy input */
        pContainer->blocks[0].len = pAdobeRsaSettings->operation_struct.sign_op.srcDataSize;

        pContainer->blocks[0].data.ptr = (uint8_t*)SRAI_Memory_Allocate(pAdobeRsaSettings->operation_struct.sign_op.srcDataSize, SRAI_MemoryType_Shared);
        if(pContainer->blocks[0].data.ptr == NULL)
        {
            rc = Drm_MemErr;
            BDBG_ERR(("%s - error allocating SRAI memory for input buffer of size 0x%x", __FUNCTION__,pAdobeRsaSettings->operation_struct.sign_op.srcDataSize));
            goto ErrorExit;
        }

        BKNI_Memcpy(pContainer->blocks[0].data.ptr,  pAdobeRsaSettings->operation_struct.sign_op.pSrcAddr,  pAdobeRsaSettings->operation_struct.sign_op.srcDataSize);

        DRM_MSG_PRINT_BUF("input data for privkey signing",  pAdobeRsaSettings->operation_struct.sign_op.pSrcAddr, pAdobeRsaSettings->operation_struct.sign_op.srcDataSize);
        /* Allocate same size for output buffer */
        pContainer->blocks[1].len = *pAdobeRsaSettings->operation_struct.sign_op.signatureSize;

        pContainer->blocks[1].data.ptr = (uint8_t*)SRAI_Memory_Allocate(*pAdobeRsaSettings->operation_struct.sign_op.signatureSize, SRAI_MemoryType_Shared);
        if(pContainer->blocks[1].data.ptr == NULL)
        {
            rc = Drm_MemErr;
            BDBG_ERR(("%s - error allocating SRAI memory for output buffer", __FUNCTION__));
            goto ErrorExit;
        }

        BKNI_Memset(pContainer->blocks[1].data.ptr, 0x00, *pAdobeRsaSettings->operation_struct.sign_op.signatureSize);

        /* Send command to SAGE */
        sage_rc = SRAI_Module_ProcessCommand(moduleHandle, DrmAdobe_CommandId_ePrivateKeyOperation, pContainer);
        if (sage_rc != BERR_SUCCESS)
        {
            rc = Drm_MemErr;
            BDBG_ERR(("%s - Error performing private key operation", __FUNCTION__));
            goto ErrorExit;
        }

        /* get result and copy to output buffer */
        sage_rc = pContainer->basicOut[0];
        if(sage_rc != BERR_SUCCESS)
        {
            rc = Drm_MemErr;
            BDBG_ERR(("%s - SAGE error with private key operation", __FUNCTION__));
            goto ErrorExit;
        }

      // (*pAdobeRsaSettings->operation_struct.sign_op.signatureSize) = pAdobeRsaSettings->operation_struct.sign_op.srcDataSize;
        *pAdobeRsaSettings->operation_struct.sign_op.signatureSize = pContainer->blocks[1].len;
        BKNI_Memcpy(pAdobeRsaSettings->operation_struct.sign_op.pSignatureAddr, pContainer->blocks[1].data.ptr, *pAdobeRsaSettings->operation_struct.sign_op.signatureSize);
         DRM_MSG_PRINT_BUF("priv key signature", pAdobeRsaSettings->operation_struct.sign_op.pSignatureAddr, *pAdobeRsaSettings->operation_struct.sign_op.signatureSize);
    }else //swkey, call bcrypt
    {
        {
#if FILE_DUMP
        fsig = fopen("signature.bin", "wb");
        fdigest = fopen("digest.bin", "wb");
        fprivatekey = fopen("privatekey.bin", "wb");
        fswkeybuf = fopen("swkeybuf.bin", "wb");
        fwrite(pKey->keybuf, 1, pKey->keybufsz, fswkeybuf);
#endif

        BDBG_MSG(("%s:Calling SW rsa SIgn api priv key sz is %d",__FUNCTION__,pKey->keybufsz));

        //DRM_MSG_PRINT_BUF("priv key buf: ",pKey->keybuf,pKey->keybufsz);

        rsaKey.n.pData = BKNI_Malloc(BCRYPT_2048_RSA_KEY_SZ);
        rsaKey.e.pData = BKNI_Malloc(BCRYPT_2048_RSA_KEY_SZ);
        rsaKey.d.pData = BKNI_Malloc(BCRYPT_2048_RSA_KEY_SZ);

        DRM_Common_GetRsa_From_PrivateKeyInfo(pKey->keybuf,pKey->keybufsz,&rsaKey);

        //DRM_MSG_PRINT_BUF("pub Modulus:",&rsaKey.n.pData[0],rsaKey.n.len);
        //DRM_MSG_PRINT_BUF( "pub exp:",&rsaKey.e.pData[0],rsaKey.e.len);
        //DRM_MSG_PRINT_BUF("&pKey->keybuf[173]",&pKey->keybuf[173],128);

        BDBG_MSG(("%s:lenof priv exp is %lu",__FUNCTION__,rsaKey.d.len));
        //DRM_MSG_PRINT_BUF("priv exp:",&rsaKey.d.pData[0],rsaKey.d.len);

        rsaSwIO.bRSAop      = drmRsasign;
        rsaSwIO.key         = &rsaKey;
        rsaSwIO.pbDataIn    = pAdobeRsaSettings->operation_struct.sign_op.pSrcAddr;
        rsaSwIO.cbDataIn    = pAdobeRsaSettings->operation_struct.sign_op.srcDataSize;
        rsaSwIO.pbDataOut   = pAdobeRsaSettings->operation_struct.sign_op.pSignatureAddr;
        rsaSwIO.cbDataOut   = (unsigned long *)pAdobeRsaSettings->operation_struct.sign_op.signatureSize;

        /*The type is implicit with the digest input length, e.g. 20 byte digest == SHA-1, 32 byte digest == SHA-256*/
        const char algo1[] = "RSA-SHA1";
        const char algo2[] = "RSA-SHA256";

        if(pAdobeRsaSettings->operation_struct.sign_op.srcDataSize == SHA1_DIGEST_SZ)
        {
            BDBG_ERR(("%s:using SHA1",__FUNCTION__));
            rsaSwIO.psAlgorithmId = (unsigned char *)algo1;
            rsaSwIO.csAlgorithmId = sizeof(algo1);
        }
        else if(pAdobeRsaSettings->operation_struct.sign_op.srcDataSize==SHA256_DIGEST_SZ)
        {
            BDBG_ERR(("%s:using SHA256",__FUNCTION__));
            rsaSwIO.psAlgorithmId = (unsigned char *)algo2;
            rsaSwIO.csAlgorithmId = sizeof(algo2);
        }


        BDBG_ERR(("%s:call DRM_Common_SwRsa",__FUNCTION__));
        rc = DRM_Common_SwRsa(&rsaSwIO);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s :private key encrypt/signing failed",__FUNCTION__));
            goto ErrorExit;

        }
    }
    }
ErrorExit:

    if(pContainer != NULL)
    {
        if(pContainer->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(pContainer->blocks[0].data.ptr);
        }

        if(pContainer->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(pContainer->blocks[1].data.ptr);
        }

        SRAI_Container_Free(pContainer);
    }

    BDBG_LEAVE(DRM_Adobe_PrivateKeyEncrypt);

    return rc;
}



/*****************************************************************************************
Function: DRM_Adobe_HostKeyIndexMapList_Create
Description: Create list that stores the loaded keys
******************************************************************************************/
static DrmRC DRM_Adobe_HostKeyIndexMapList_Create(DrmAdobeKeyIndexMap_t *pEntry)
{
    DrmAdobeKeyIndexMapList *ptr = NULL;
    DrmRC rc = Drm_Success;

    BDBG_ENTER(DRM_Adobe_HostKeyIndexMapList_Create);

    ptr = (DrmAdobeKeyIndexMapList *)BKNI_Malloc(sizeof(DrmAdobeKeyIndexMapList ));
    if(ptr == NULL)
    {
        BDBG_ERR(("%s - BKNI_Malloc err", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    /* copy entry structure */
    BKNI_Memcpy(&ptr->keyEntry, pEntry, sizeof(DrmAdobeKeyIndexMap_t));

    /* assign null to next entry in chain */
    ptr->pNext = NULL;

    gpAdobeHostKeyIndexMapHead = gpAdobeHostKeyIndexMapTail = ptr;
    gHostKeySequenceNumber++;

ErrorExit:
    BDBG_LEAVE(DRM_Adobe_HostKeyIndexMapList_Create);
    return Drm_Success;
}

/*****************************************************************************************
Function: DRM_Adobe_HostKeyIndexMapList_Addentry
Description: Add a key to the list that stotres the loaded keys
******************************************************************************************/

static DrmRC DRM_Adobe_HostKeyIndexMapList_AddEntry(DrmAdobeKeyIndexMap_t *pEntry)
{
    DrmAdobeKeyIndexMapList  *ptr = NULL;
    BDBG_ENTER(DRM_Adobe_HostKeyIndexMapList_AddEntry);
    if(gpAdobeHostKeyIndexMapHead == NULL)
    {
        return (DRM_Adobe_HostKeyIndexMapList_Create(pEntry));
    }

    ptr = (DrmAdobeKeyIndexMapList *)BKNI_Malloc(sizeof(DrmAdobeKeyIndexMapList ));
    if(ptr == NULL)
    {
        BDBG_ERR(("%s - BKNI_Malloc err", __FUNCTION__));
        return Drm_MemErr;
    }

    /* copy entry structure */
    BKNI_Memcpy(ptr, pEntry, sizeof(DrmAdobeKeyIndexMap_t));

    ptr->pNext = NULL;

    /*add  to the end of the list*/
    gpAdobeHostKeyIndexMapTail->pNext = ptr;
    gpAdobeHostKeyIndexMapTail = ptr;
    gHostKeySequenceNumber++;

    BDBG_LEAVE(DRM_Adobe_HostKeyIndexMapList_AddEntry);

    return Drm_Success;
}

/*****************************************************************************************
Function: DRM_Adobe_HostKeyIndexMapList_Search
Description: Search for a key in the list that stotres the loaded keys
******************************************************************************************/
static bool DRM_Adobe_HostKeyIndexMapList_Search(DrmAdobeKeyIndexMap_t **pEntry, uint32_t   index)
{
    DrmAdobeKeyIndexMapList *ptr = gpAdobeHostKeyIndexMapHead;
    DrmAdobeKeyIndexMapList *tmp = NULL;
    bool found = false;

    BDBG_MSG(("%s - Entered function search for Index 0x%x", __FUNCTION__,index));
    while(ptr != NULL)
    {
        if(ptr->keyEntry.adobeIndex == index){
            found = true;
            break;
        }
        else{
            tmp = ptr;
            ptr = ptr->pNext;
        }
    }

    if(true == found){
        BDBG_MSG(("%s - FOUND KEY index '0x%08x' in the List !!!!", __FUNCTION__, index));
        *pEntry = &ptr->keyEntry;
    }
    else{
        *pEntry = NULL;
    }

    BDBG_LEAVE(DRM_Adobe_HostKeyIndexMapList_Search);

    return found;
}

/*****************************************************************************************
Function: DRM_Adobe_HostKeyIndexMapList_DeleteEntry
Description: Delete a key to the list that stotres the loaded keys
******************************************************************************************/
static DrmRC DRM_Adobe_HostKeyIndexMapList_DeleteEntry(uint32_t index)
{
    DrmAdobeKeyIndexMapList *prev = gpAdobeHostKeyIndexMapHead;
    bool found = false;
    DrmAdobeKeyIndexMapList *ptr = gpAdobeHostKeyIndexMapHead;

    BDBG_ENTER(DRM_Adobe_HostKeyIndexMapList_DeleteEntry);

    while(ptr != NULL)
    {
        if(ptr->keyEntry.adobeIndex == index)
        {
            found = true;
            break;
        }
        else
        {
            prev = ptr;
            ptr = ptr->pNext;
        }
    }


    if(found == false)
    {
       BDBG_MSG(("%s -  index '0x%08x' not found in KeyIndexMapList", __FUNCTION__, index));
       return Drm_Err;
    }
    else
    {
        if(ptr->keyEntry.adobeIndex == gpAdobeHostKeyIndexMapHead->keyEntry.adobeIndex)
        {
            BDBG_MSG(("%s - entry to be deleted is the HEAD", __FUNCTION__));
            gpAdobeHostKeyIndexMapHead = gpAdobeHostKeyIndexMapHead->pNext;
        }
        else if(ptr->keyEntry.adobeIndex == gpAdobeHostKeyIndexMapTail->keyEntry.adobeIndex)
        {
            BDBG_MSG(("%s - entry to be deleted is the TAIL", __FUNCTION__));
            gpAdobeHostKeyIndexMapTail = prev;
            gpAdobeHostKeyIndexMapTail->pNext = NULL;
        }
        else
        {
            BDBG_MSG(("%s - entry to be deleted is not the head or the tail",__FUNCTION__));
            prev->pNext = ptr->pNext;
        }

        BKNI_Free(ptr);

        if(gHostKeySequenceNumber == 0){
            BDBG_WRN(("%s - HostSequenceNumber is '0' and decrementing!!!", __FUNCTION__));
        }

        //gHostKeySequenceNumber--;
    }


    BDBG_LEAVE(DRM_Adobe_HostKeyIndexMapList_DeleteEntry);
    return Drm_Success;
}

//#define FILE_DUMP 1

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetDRMCert
**
** DESCRIPTION:
**
******************************************************************************/
DrmRC DRM_Adobe_GetDRMCert(uint32_t keyType, /* Input: If the HW supports multiple DRM keys, then need to specify what type, else dual */
                           uint8_t *pCert,        /* Output: Certificate of the HW block. Memory allocated by SW */
                           uint32_t *pSize)     /* Input/Output: Size of certificate. If not large enough, then the HW will return the correct size. */
{
    DrmRC rc = Drm_Success;
    BSAGElib_InOutContainer *pContainer = NULL;
    BERR_Code sage_rc = BERR_SUCCESS;
    const int max_cert_size = 10*1024;
#if FILE_DUMP
    size_t noOfBytesWritten = 0;
    FILE * f_cert=NULL;

#endif
    BDBG_ENTER(DRM_Adobe_GetDRMCert);

    pContainer = SRAI_Container_Allocate();
    if(pContainer == NULL)
    {
        rc = Drm_MemErr;
        BDBG_ERR(("%s - Error allocating SRAI pContainer", __FUNCTION__));
        goto ErrorExit;
    }

    pContainer->basicIn[0] = keyType;
    BDBG_LOG(("%s:Cert TYPE is %d",__FUNCTION__,keyType));
    if(keyType == DrmAdobe_CertType_eDUAL||keyType ==  DrmAdobe_Certype_eEncryption)
    {
        BDBG_LOG(("%s: RETURNING DECRYPTION CERT",__FUNCTION__));
 #if FILE_DUMP
            f_cert = fopen("DRM_dec_cert_sage.cer", "wb");
#endif
    }else if (keyType == DrmAdobe_CertType_eSIGN)
    {
        BDBG_LOG(("%s: RETURNING SIGNING CERT",__FUNCTION__));
#if FILE_DUMP
            f_cert = fopen("DRM_Sign_cert_sage.cer", "wb");
#endif

    }
    else
    {
        BDBG_ERR(("%s: Invalid CERT Type requested ",__FUNCTION__));
        goto ErrorExit;
    }

    if(pCert == NULL)
    {
        BDBG_ERR(("%s - Setting shared pointer to null since going to return size only", __FUNCTION__));
        pContainer->blocks[0].data.ptr = NULL;
        pContainer->blocks[0].len = 0;
    }
    else
    {
        pContainer->blocks[0].data.ptr = SRAI_Memory_Allocate( max_cert_size, SRAI_MemoryType_Shared);
        pContainer->blocks[0].len = max_cert_size;
    }

    sage_rc = SRAI_Module_ProcessCommand(moduleHandle, DrmAdobe_CommandId_eGetDecryptionCert, pContainer);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error sending command to SAGE", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    sage_rc = pContainer->basicOut[0];
    if(sage_rc != BERR_SUCCESS)
    {
        rc = Drm_MemErr;
        BDBG_ERR(("%s - Command sent to SAGE successfully but error fetching decryption certificate", __FUNCTION__));
        goto ErrorExit;
    }

    (*pSize) = pContainer->basicOut[1];
    if(pCert == NULL)
    {
        BDBG_MSG(("%s - Size retrieved is '%u' bytes.  Certificate not copied to destination buffer", __FUNCTION__, (*pSize)));
    }
    else
    {
        BDBG_MSG(("%s - Copying certificate to buffer *** size=%d", __FUNCTION__,*pSize));
        BKNI_Memcpy(pCert, pContainer->blocks[0].data.ptr, (*pSize));
        DRM_MSG_PRINT_BUF("HWDRMCERT:",pCert,64 /**size*/);
 #if FILE_DUMP
            noOfBytesWritten = fwrite(pCert, 1, *pSize, f_cert);
#endif
    }

ErrorExit:

    if(pContainer != NULL)
    {
        if(pContainer->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(pContainer->blocks[0].data.ptr);
            pContainer->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(pContainer);
    }

    BDBG_LEAVE(DRM_Adobe_GetDRMCert);
#if FILE_DUMP
    if (f_cert)
        fclose(f_cert);
#endif
    return rc;
}


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetHWID
**
** DESCRIPTION:
**
******************************************************************************/
DrmRC DRM_Adobe_GetHWID(
    uint8_t *pHWID,             /* Pointer to a 64 byte buffer that is malloced by caller */
    uint32_t *pHWIDLength,     /* Size of returned HWID, must be 64 bytes or less */
    uint8_t *pMagicNumber)
{
    DrmRC rc = Drm_Success;
    uint8_t temp_buf[32]={0x0};
    uint8_t digest[32]={0x0};
    drm_chip_info_t chipInfo;

    BDBG_ENTER(DRM_Adobe_GetHWID);

    rc = DRM_Common_FetchDeviceIds(&chipInfo);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error fetching device IDs", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memcpy(temp_buf, chipInfo.devIdA, 8);
    BKNI_Memcpy(temp_buf+8, pMagicNumber, 16);

    DRM_MSG_PRINT_BUF("temp_buf", temp_buf, 32);

    rc = DRM_Common_SwSha256(temp_buf, digest, 24);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error computing SHA", __FUNCTION__));
        goto ErrorExit;
    }

    DRM_MSG_PRINT_BUF("digest", digest, 32);

    BKNI_Memcpy(pHWID, digest, 32);
    *pHWIDLength    = 32;

ErrorExit:
    BDBG_LEAVE(DRM_Adobe_GetHWID);
    return rc;

}

DrmRC DRM_Adobe_MemoryFree(uint8_t *pHWMemPTR)
{
    DrmRC rc = Drm_Success;
    BDBG_ENTER(DRM_Adobe_MemoryFree);

    if(pHWMemPTR)
    {
        rc = DRM_Common_MemoryFree(pHWMemPTR);
    }
    else
    {
        BDBG_WRN(("%s - NULL Pointer.No action taken",__FUNCTION__));
        rc = Drm_Success;
    }

    BDBG_LEAVE(DRM_Adobe_MemoryFree);
    return rc;
}




/******************************************************************************
** FUNCTION:
**  DRM_Adobe_EncryptData
**
** DESCRIPTION:
**
******************************************************************************/
DrmRC DRM_Adobe_EncryptData(DrmAdobeSessionContext_t *pCTX,/* Input: Pointer to HW DRM context. Can be NULL if not required by HW */
                            uint8_t *pInput,                /* Input: Data to be encrypted */
                            uint32_t length,            /* Input: Length of data to be encrypted */
                            uint32_t *pOutLength,        /* Output: Data Length */
                            uint8_t *pOutput)
{
    DrmRC  rc = Drm_Success;
    DmaBlockInfo_t dmaBlock;
    uint8_t *pInputNexusMem = NULL;
    uint8_t *pOutputNexusMem = NULL;
    uint32_t padded_encrypted_length = 0;


    BDBG_MSG(("%s - Entered function.  Session context %p, data length is '%u', output buffer length = '%u'", __FUNCTION__, (void *)pCTX, length, (*pOutLength)));

    if(pCTX == NULL)
    {
        BDBG_MSG(("%s - Context is null", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    padded_encrypted_length = length;
    if(padded_encrypted_length % 16 != 0)
    {
        BDBG_MSG(("%s - adjusting length for padding", __FUNCTION__));
        padded_encrypted_length += (16 - (length % 16));
        BDBG_MSG(("%s - padded length = '%u'", __FUNCTION__, padded_encrypted_length));
    }

    rc = DRM_Common_MemoryAllocate(&pInputNexusMem,  padded_encrypted_length);
    if(rc != Drm_Success)
    {
       BDBG_ERR(("%s -  Error Allocating drm Memory for input", __FUNCTION__));
       goto ErrorExit;
    }
    BKNI_Memset(pInputNexusMem, 0x00,  padded_encrypted_length);
    BKNI_Memcpy(pInputNexusMem, pInput,  length);

    /* if bi-directional is not defined we have to allocate a local buffer */
    if((capabilities & DrmAdobe_CAPABILITY_BI_DIRECTIONAL_MALLOC) == 0)
    {
        BDBG_MSG(("%s - Capability is NOT bi-directional", __FUNCTION__));

        rc = DRM_Common_MemoryAllocate((uint8_t**)&pOutputNexusMem,  padded_encrypted_length);
        if(rc != Drm_Success)
        {
           BDBG_ERR(("%s - Error Allocating drm Memory for output", __FUNCTION__));
           goto ErrorExit;
        }
    }
    else{
        pOutputNexusMem = pOutput;
    }

    if(pCTX->bUseCtxforDiskIO == false)
    {
        BDBG_MSG(("%s - Starting M2M operation ***", __FUNCTION__));

        /* set DMA parameters */
        dmaBlock.pDstData = pOutputNexusMem;
        dmaBlock.pSrcData = pInputNexusMem;
        dmaBlock.uiDataSize =  padded_encrypted_length;
        dmaBlock.sg_start = true;
        dmaBlock.sg_end = true;

        pCTX->drmCommonOpStruct.pDmaBlock = &dmaBlock;
        pCTX->drmCommonOpStruct.num_dma_block = 1;

        //drmCommonOpStruct.keyConfigSettings.keySlot = pCTX->drmCommonOpStruct.keyConfigSettings.keySlot;

        rc = DRM_Common_M2mOperation(&pCTX->drmCommonOpStruct);
        if(rc != Drm_Success)
        {
            BDBG_ERR(("%s - Call to 'DRM_Common_M2mOperation' failed", __FUNCTION__));
            goto ErrorExit;
        }
    }
    else /* do a secure buffer operation */
    {
        rc = DRM_SecureStore_BufferOperation(pInputNexusMem, padded_encrypted_length, pOutputNexusMem, DrmDestinationType_eExternal, DrmCryptoOperation_eEncrypt);
        if(rc!=Drm_Success)
        {
            BDBG_ERR(("%s:secure store buffer operation failed",__FUNCTION__));
            goto ErrorExit;
        }
    }

    /* if bi-directional is not defined we have to copy form the local buffer to output */
    if((capabilities & DrmAdobe_CAPABILITY_BI_DIRECTIONAL_MALLOC) == 0)
    {
        if(pCTX->bUseCtxforDiskIO == false){
            BDBG_MSG(("%s - Copying result of M2M operation", __FUNCTION__));
        }
        else{
            BDBG_MSG(("%s - Copying result of secure store operation", __FUNCTION__));
        }

        BKNI_Memcpy(pOutput, pOutputNexusMem,  length);
        DRM_Common_MemoryFree(pOutputNexusMem);
        pOutputNexusMem = NULL;

        DRM_MSG_PRINT_BUF("output", pOutput, /*padded_encrypted_length*/64);
    }

    *pOutLength = length;

ErrorExit:
    if(pInputNexusMem != NULL){
        DRM_Common_MemoryFree(pInputNexusMem);
       pInputNexusMem = NULL;
    }

    BDBG_LEAVE(DRM_Adobe_EncryptData);
        return rc;
}





/******************************************************************************
** FUNCTION:
**   DRM_Adobe_GetRootCertDigest
**
** DESCRIPTION:
**
******************************************************************************/
DrmRC DRM_Adobe_GetRootCertDigest( uint8_t *pCert,       /* Output: Certificate of the HW block. Memory allocated by SW */
                                   uint32_t *pSize       /* Input/Output: Size of certificate. If not large enough, then the HW will return the correct size. */
                                 )
{
    /*TO DO: we need to store the drm cert size in drmroofs or compute the size by looking at the next offset in the header in the adobe_key.bin*/
    DrmRC rc = Drm_Success;
    /*Use "./prodAdobeRootDigest.bin" if you arre using production keys*/
    const char * fname ="./testAdobeRootDigest.bin";
    FILE *fp =NULL;

    BDBG_MSG(("Entered %s that reads form loacl file , ",__FUNCTION__ ));
    BDBG_ASSERT(pSize != NULL);
    BDBG_MSG(("Entered %s, cert buf pointer:%p,cert buf size 0x%x",__FUNCTION__,(void *)pCert,*pSize ));

    if(gpRootCertficateDigest == NULL)
    {
        gRootCertficateDigestSz = 0;

        fp = fopen(fname, "r");
        if (fp == NULL)
        {
            BDBG_ERR(("%s: (%s) not found",__FUNCTION__,fname));
            rc = Drm_FileErr;
            goto ErrorExit;
        }
        BDBG_MSG(("%s: %s ",__FUNCTION__,fname));
        rc = DRM_Common_MemoryAllocate(&gpRootCertficateDigest, ROOTCERT_DIGEST_SZ);
        if(rc != Drm_Success)
        {
           BDBG_ERR(("%s: Error Allocating Memory", __FUNCTION__));
           goto ErrorExit;
        }

        while (!feof(fp))
        {
             gpRootCertficateDigest[gRootCertficateDigestSz++] = fgetc(fp);
        }
        gRootCertficateDigestSz--;
        fclose(fp);
    }

    if(pCert == NULL)
    {
        BDBG_MSG(("%s Cert is NULL sz %d",__FUNCTION__, gRootCertficateDigestSz));
    }
    else
    {
        BKNI_Memcpy(pCert,gpRootCertficateDigest,gRootCertficateDigestSz);
    }
        *pSize = gRootCertficateDigestSz;

ErrorExit:
    /*need to free the cert memory in drm_finalize.*/
    BDBG_MSG(("%s - Exiting",__FUNCTION__ ));
    return rc;
}


/******************************************************************************
** FUNCTION:
**   DRM_Adobe_GetIndivTransportCert
**
** DESCRIPTION:
**
******************************************************************************/

DrmRC   DRM_Adobe_GetIndivTransportCert( uint8_t *pCert,     /* Output: Certificate of the HW block. Memory allocated by SW */
                                         uint32_t *pSize     /* Input/Output: Size of certificate. If not large enough, then the HW will return the correct size. */
                                       )
{
    /*TO DO: we need to store the drm cert size in drmroofs or compute the size by looking at the next offset in the header in the adobe_key.bin*/
    DrmRC rc = Drm_Success;
    BDBG_MSG(("Entered %s, ",__FUNCTION__ ));

    /*pCert Can be NULL when app queries the size, so only do parameter check on pSize*/
    if(pSize == NULL)
    {
        BDBG_ERR(("%s: Invalid Input Parameter",__FUNCTION__));
        rc = Drm_InvalidParameter;
        goto ErrorExit;
    }

    BDBG_MSG(("%s:, cert buf pointer:%p,cert buf size 0x%x",__FUNCTION__,(void *)pCert,*pSize ));

    if(gpIndivTransportCert == NULL)
    {
        /* if using production keys, use "./ProdIndivServerTransportCert.cer" */
        const char * fname = "./testIndivServerTransportCert.cer";
        FILE *fp = fopen(fname, "r");
        if (fp == NULL)
        {
          BDBG_ERR(("%s: file(%s) not found",__FUNCTION__,fname));
          rc = Drm_FileErr;
          goto ErrorExit;
        }
        rc = DRM_Common_MemoryAllocate(&gpIndivTransportCert, 1024*2);
        if(rc !=Drm_Success)
        {
           BDBG_ERR(("%s: Error Allocating Memory", __FUNCTION__));
           goto ErrorExit;
        }

        gIndivTransportCertSz = 0;
        while (!feof(fp))
        {
             gpIndivTransportCert[gIndivTransportCertSz++] = fgetc(fp);
        }
        gIndivTransportCertSz --;
        fclose(fp);
    }
        /* Adobe Has changed and will allocate memory for cert at their end. then we need to change this to memcpy instead of
        assigning ptr. but since the current test harness is not allocating memory we are assigning pointer*/

    if(pCert==NULL)
    {
        pCert = gpIndivTransportCert;
    }
    else
    {
        BKNI_Memcpy(pCert,gpIndivTransportCert,gIndivTransportCertSz);
    }
    *pSize = gIndivTransportCertSz;
     DRM_MSG_PRINT_BUF("IndivTransportCert", pCert, 64);
ErrorExit:
    /*need to free the cert memory in drm_finalize.*/
    BDBG_MSG(("%s - Exiting",__FUNCTION__ ));
    return rc;
}
