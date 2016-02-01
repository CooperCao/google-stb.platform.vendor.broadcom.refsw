/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 ****************************************************************************/

#include "b_dvr_manager.h"
#include "b_dvr_drmservice.h"
#include "b_dvr_manager_priv.h"

BDBG_MODULE(b_dvr_drmservice);
BDBG_OBJECT_ID(B_DVR_DRMService);
#if DRM_SUPPORT
static void B_DVR_DRMService_P_InvalidateKey(NEXUS_KeySlotHandle keySlot)
{
    int index;
    BDBG_MSG(("%s Enter",__FUNCTION__));
    NEXUS_SecurityInvalidateKeySettings invSettings;
    
    invSettings.keyDestEntryType  = NEXUS_SecurityKeyType_eOddAndEven;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;

    for (index=0;index<4;index++)
    {
        switch(index)
        {
            case 0:
            invSettings.keyDestBlckType   = NEXUS_SecurityAlgorithmConfigDestination_eCa;       
            break;

            case 1:
            invSettings.keyDestBlckType   = NEXUS_SecurityAlgorithmConfigDestination_eCps;                  
            break;

            case 2:
            invSettings.keyDestBlckType   = NEXUS_SecurityAlgorithmConfigDestination_eCps;                  
            break;

            case 3:
            invSettings.keyDestEntryType  = NEXUS_SecurityKeyType_eClear;
            invSettings.keyDestBlckType   = NEXUS_SecurityAlgorithmConfigDestination_eCps;                  
            break;

            default:
            break;
        }
        if(NEXUS_Security_InvalidateKey(keySlot, &invSettings))
        {
            BDBG_MSG(("%s NEXUS Security Invalidate Key error (keySlot = 0x%x)",__FUNCTION__,keySlot));
        }
    }
    
    BDBG_MSG(("%s Exit",__FUNCTION__)); 
}

B_DVR_ERROR B_DVR_DRMService_P_KeyloaderCallback(B_DVR_DRMServiceHandle drmService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_MSG(("%s Enter",__FUNCTION__));
    
    switch(drmService->drmServiceRequest.keyType)
    {           
        case eB_DVR_DRMServiceKeyTypeClear:
            drmService->drmServiceRequest.controlWordKeyLoader(drmService->keySlot);
        break;
#if KEYLADDER_SUPPORT   
        case eB_DVR_DRMServiceKeyTypeProtected:
            drmService->drmServiceRequest.sessionKeyLoader(drmService->keySlot);
            drmService->drmServiceRequest.controlWordKeyLoader(drmService->keySlot);
            drmService->drmServiceRequest.ivKeyLoader(drmService->keySlot);
        break;
#endif  
        default:
        break;
    }           
    
    BDBG_MSG(("%s Exit",__FUNCTION__));
    return rc;

}

static void B_DVR_DRMServide_P_SetDefaultConfigure(B_DVR_DRMServiceHandle drmService,
                                                                    B_DVR_DRMServiceSettings *drmServiceSettings,
                                                                    NEXUS_SecurityAlgorithmSettings *algConfig)
{
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_MSG(("%s Enter",__FUNCTION__));
    
    NEXUS_Security_GetDefaultAlgorithmSettings(algConfig);  
    algConfig->algorithm            = drmServiceSettings->drmAlgoType;
    algConfig->ivMode               = NEXUS_SecurityIVMode_eRegular;
    algConfig->solitarySelect       = NEXUS_SecuritySolitarySelect_eClear;
    algConfig->caVendorID           = 0x1234;   
    algConfig->askmModuleID         = NEXUS_SecurityAskmModuleID_eModuleID_4;
    algConfig->otpId                = NEXUS_SecurityOtpId_eOtpVal;
    algConfig->key2Select           = NEXUS_SecurityKey2Select_eReserved1;
    algConfig->dest                 = drmServiceSettings->drmDest;

    if (drmService->drmServiceRequest.keyType == eB_DVR_DRMServiceKeyTypeProtected)
    {
        algConfig->algorithmVar     = NEXUS_SecurityAlgorithmVariant_eCbc;
        algConfig->terminationMode  = NEXUS_SecurityTerminationMode_eClear;
        BDBG_MSG(("%s Protected config ",__FUNCTION__));
    }
    else if (drmService->drmServiceRequest.keyType == eB_DVR_DRMServiceKeyTypeClear)
    {

        algConfig->algorithmVar     = NEXUS_SecurityAlgorithmVariant_eEcb;          
        algConfig->terminationMode      = NEXUS_SecurityTerminationMode_eCipherStealing;        
        BDBG_MSG(("%s ClearKey config ",__FUNCTION__));
    }
        
        switch(algConfig->dest)
        {
            case NEXUS_SecurityAlgorithmConfigDestination_eCps:
            {
                algConfig->operation            = drmServiceSettings->operation;
                algConfig->bRestrictEnable     = false;
                algConfig->bEncryptBeforeRave  = false;
                algConfig->modifyScValue[NEXUS_SecurityPacketType_eRestricted]  = true;
                algConfig->modifyScValue[NEXUS_SecurityPacketType_eGlobal]      = true;
            /* per CPS encryption and M2m Playback , scPolarity should be set to eClear*/
            algConfig->scValue[NEXUS_SecurityPacketType_eRestricted] = drmServiceSettings->scPolarity; 
            algConfig->scValue[NEXUS_SecurityPacketType_eGlobal]      = drmServiceSettings->scPolarity;
                BDBG_MSG(("%s CPS config ",__FUNCTION__));
            }
            break;
            
            case NEXUS_SecurityAlgorithmConfigDestination_eCa:
            case NEXUS_SecurityAlgorithmConfigDestination_eCpd:
            {
                algConfig->modifyScValue[NEXUS_SecurityPacketType_eRestricted]  = false;
                algConfig->modifyScValue[NEXUS_SecurityPacketType_eGlobal]      = false;
                algConfig->operation = drmServiceSettings->operation;
                BDBG_MSG(("%s CA config",__FUNCTION__));
            }
            break;

            case NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem:
            {
                algConfig->operation = drmServiceSettings->operation;
                BDBG_MSG(("%s M2M config",__FUNCTION__));           
            }
            break;

            default:
                break;
        }
        
    NEXUS_Security_ConfigAlgorithm (drmService->keySlot,algConfig);

    if(algConfig->dest == NEXUS_SecurityAlgorithmConfigDestination_eCps)
    {
        algConfig->keyDestEntryType = NEXUS_SecurityKeyType_eClear;
        NEXUS_Security_ConfigAlgorithm (drmService->keySlot,algConfig);     
    }

    
    BDBG_MSG(("%s Exit (algorithm(%d),drmDest(%d)),algorithmVar(%d),operation(%d)",__FUNCTION__,
                                                    drmServiceSettings->drmAlgoType,
                                                    drmServiceSettings->drmDest,
                                                    algConfig->algorithmVar,
                                                    algConfig->operation)); 
}

B_DVR_ERROR B_DVR_DRMService_P_LoadClearKey(B_DVR_DRMServiceHandle drmService,
                                                           NEXUS_SecurityClearKey *clearKey)
{
    uint8_t index;
    unsigned keyIndex;
    uint8_t numOfCpsKeyEntry =3;
    unsigned char evenKey[B_DVR_DEFAULT_KEY_LENGTH];
    unsigned char oddKey[B_DVR_DEFAULT_KEY_LENGTH]; 
    B_DVR_ERROR rc = B_DVR_SUCCESS; 
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_MSG(("%s Enter keySlot(0x%x)",__FUNCTION__,drmService->keySlot));

    if((drmService->drmServiceRequest.securityEngine == NEXUS_SecurityEngine_eCaCp)&&
        (drmService->drmServiceSettings.drmDest == NEXUS_SecurityAlgorithmConfigDestination_eCps))
    {
        BKNI_Memcpy(clearKey->keyData,drmService->drmServiceSettings.keys,clearKey->keySize);
        for(index=0;index<numOfCpsKeyEntry;index++)
        {
            /* CPS needs even ,odd and clear key entry type for one keys */
            switch(index)
            {
                case 0:
                    clearKey->keyEntryType = NEXUS_SecurityKeyType_eOdd;
                    break;

                case 1:
                    clearKey->keyEntryType = NEXUS_SecurityKeyType_eEven;       
                    break;

                case 2:
                    clearKey->keyEntryType = NEXUS_SecurityKeyType_eClear;
                    break;
                default:
                    break;                  
            }
            
            if(NEXUS_Security_LoadClearKey(drmService->keySlot, clearKey))
            {
                BDBG_MSG(("%s NEXUS load %d clearKey failed",__FUNCTION__,index));  
                rc = B_DVR_FAIL_TO_LOAD_CLEARKEY;
                return rc;
            }           

        }
    }
    else if((drmService->drmServiceRequest.securityEngine == NEXUS_SecurityEngine_eCa)||
             (drmService->drmServiceRequest.securityEngine == NEXUS_SecurityEngine_eM2m)||
             (drmService->drmServiceSettings.drmDest == NEXUS_SecurityAlgorithmConfigDestination_eCa))
    {
        if(drmService->drmServiceSettings.keyLength > B_DVR_DEFAULT_KEY_LENGTH) 
        {
            /* consider if (keyLength>B_DVR_DEFAULT_KEY_LENGTH) , it was set to use even/odd keys*/
            for(keyIndex=0;keyIndex<drmService->drmServiceSettings.keyLength;keyIndex++)
            {

                (keyIndex<B_DVR_DEFAULT_KEY_LENGTH)? (evenKey[keyIndex] = *(drmService->drmServiceSettings.keys+keyIndex)):
                                                     (oddKey[keyIndex-B_DVR_DEFAULT_KEY_LENGTH] = *(drmService->drmServiceSettings.keys+keyIndex)); 
                if(keyIndex<B_DVR_DEFAULT_KEY_LENGTH)
                    BDBG_MSG(("loaded evenKey[%d] = 0x%x",keyIndex,evenKey[keyIndex]));
                else
                    BDBG_MSG(("loaded oddKey[%d] = 0x%x",keyIndex-B_DVR_DEFAULT_KEY_LENGTH,oddKey[keyIndex-B_DVR_DEFAULT_KEY_LENGTH]));
            }

            clearKey->keyEntryType = NEXUS_SecurityKeyType_eEven; 
            BKNI_Memcpy(clearKey->keyData,evenKey,clearKey->keySize);
            NEXUS_Security_LoadClearKey(drmService->keySlot, clearKey);

            clearKey->keyEntryType = NEXUS_SecurityKeyType_eOdd; 
            BKNI_Memcpy(clearKey->keyData,oddKey,clearKey->keySize);
            NEXUS_Security_LoadClearKey(drmService->keySlot, clearKey);
            BDBG_MSG(("%s CPS even and odd Keys loaded",__FUNCTION__));         

        }
        else
        {
            /* CA and M2M use same key values , ans load odd keys  */
            BKNI_Memcpy(clearKey->keyData,drmService->drmServiceSettings.keys,clearKey->keySize);

            clearKey->keyEntryType = NEXUS_SecurityKeyType_eOdd;            
            NEXUS_Security_LoadClearKey(drmService->keySlot, clearKey);

            for(index=0;index<(clearKey->keySize);index++)
            {
                BDBG_MSG(("loaded keys[%d] = 0x%x",index,*(clearKey->keyData+index)));
            }
            
            BDBG_MSG(("%s CA or M2M , same Key loaded for Even and Odd ",__FUNCTION__));
        }

    }

    BDBG_MSG(("%s Exit",__FUNCTION__));
    return rc;

}

B_DVR_ERROR B_DVR_DRMService_P_ClearKeyConfigure(B_DVR_DRMServiceHandle drmService,
                                                 B_DVR_DRMServiceSettings *drmServiceSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_SecurityClearKey *key;
    
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_ASSERT(drmServiceSettings);    
    BDBG_MSG(("%s Enter",__FUNCTION__));

    if(drmServiceSettings->keys != NULL)
    {

        key = (NEXUS_SecurityClearKey *)BKNI_Malloc(sizeof(NEXUS_SecurityClearKey));
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(NEXUS_SecurityClearKey),
                                                true, __FUNCTION__,__LINE__);
        BKNI_Memset(key,0,sizeof(NEXUS_SecurityClearKey));

        NEXUS_Security_GetDefaultClearKey(key);
        /* key length sets to 16 bytes. fixed*/
        key->keySize = B_DVR_DEFAULT_KEY_LENGTH; 
        key->dest = drmServiceSettings->drmDest;
        key->keyIVType = NEXUS_SecurityKeyIVType_eNoIV;

        rc = B_DVR_DRMService_P_LoadClearKey(drmService,key);       

        if(rc != B_DVR_SUCCESS)
        {
            BDBG_MSG(("%s Load Clear Key fail",__FUNCTION__));
            goto error;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*key), false,
                                                __FUNCTION__,__LINE__);
        BKNI_Free(key);

    }
    BDBG_MSG(("%s Exit ",__FUNCTION__));
    return rc;

    error:
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*key), false,
                                            __FUNCTION__,__LINE__);
    BKNI_Free(key);
    return rc;

}

B_DVR_ERROR B_DVR_DRMService_P_KeyladderConfigure(B_DVR_DRMServiceHandle drmService,
                                                  B_DVR_DRMServiceSettings *drmServiceSettings,
                                                  NEXUS_SecurityAlgorithmSettings *algConfig)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_MSG(("%s Enter",__FUNCTION__));
#if KEYLADDER_SUPPORT   
    int index;
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;
    NEXUS_SecurityClearKey  key;

    NEXUS_Security_GetDefaultSessionKeySettings(&encryptedSessionkey);
    encryptedSessionkey.keyladderID     = NEXUS_SecurityKeyladderID_eA; 
    encryptedSessionkey.keyladderType   = NEXUS_SecurityKeyladderType_e3Des;
    encryptedSessionkey.swizzleType     = NEXUS_SecuritySwizzleType_eNone;
    encryptedSessionkey.cusKeyL         = 0x00; 
    encryptedSessionkey.cusKeyH         = 0x00; 
    encryptedSessionkey.cusKeyVarL      = 0x00; 
    encryptedSessionkey.cusKeyVarH      = 0xFF;
    encryptedSessionkey.keyGenCmdID     = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encryptedSessionkey.sessionKeyOp    = NEXUS_SecuritySessionKeyOp_eNoProcess;
    encryptedSessionkey.bASKMMode       = false;
    encryptedSessionkey.rootKeySrc      = NEXUS_SecurityRootKeySrc_eOtpKeyA;
    encryptedSessionkey.bSwapAESKey     = false;
    encryptedSessionkey.keyDestIVType   = NEXUS_SecurityKeyIVType_eNoIV;
    encryptedSessionkey.bRouteKey       = false;
    encryptedSessionkey.operation       = NEXUS_SecurityOperation_eDecrypt;  
    encryptedSessionkey.operationKey2   = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encryptedSessionkey.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKL6;
    encryptedSessionkey.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    encryptedSessionkey.dest               =  algConfig->dest; 

    BKNI_Memcpy(encryptedSessionkey.keyData,drmServiceSettings->sessionKeys, drmServiceSettings->keyLength);

    for(index=0;index<(int)(drmServiceSettings->keyLength);index++)
    {
        BDBG_MSG(("session = 0x%x ",encryptedSessionkey.keyData[index]));
    }

    encryptedSessionkey.keyEntryType    = NEXUS_SecurityKeyType_eOdd;
    if(NEXUS_Security_GenerateSessionKey(drmService->keySlot, &encryptedSessionkey))
    {
        BDBG_MSG(("%s NEXUS Generate Session Key Odd failed",__FUNCTION__));
        rc = B_DVR_FAIL_TO_GENERATE_SESSION_KEY;
        return rc;              
    }

    if (algConfig->dest == NEXUS_SecurityAlgorithmConfigDestination_eCps)
    {
        encryptedSessionkey.keyEntryType    = NEXUS_SecurityKeyType_eEven;
        if(NEXUS_Security_GenerateSessionKey(drmService->keySlot, &encryptedSessionkey))
        {
            BDBG_MSG(("%s NEXUS Generate Session Key Even failed",__FUNCTION__));
            rc = B_DVR_FAIL_TO_GENERATE_SESSION_KEY;
            return rc;              
        }
        
        encryptedSessionkey.keyEntryType    = NEXUS_SecurityKeyType_eClear;
    if(NEXUS_Security_GenerateSessionKey(drmService->keySlot, &encryptedSessionkey))
    {
            BDBG_MSG(("%s NEXUS Generate Session Key Clear failed",__FUNCTION__));
        rc = B_DVR_FAIL_TO_GENERATE_SESSION_KEY;
        return rc;              
    }
    }

    NEXUS_Security_GetDefaultControlWordSettings(&encrytedCW);
    encrytedCW.keyladderID        = NEXUS_SecurityKeyladderID_eA;
    encrytedCW.keyladderType      = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize            = drmServiceSettings->keyLength; 
    encrytedCW.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encrytedCW.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKL6;
    encrytedCW.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    encrytedCW.operation          = NEXUS_SecurityOperation_eDecrypt;   
    encrytedCW.bRouteKey         = true;
    encrytedCW.keyDestIVType      = NEXUS_SecurityKeyIVType_eNoIV;
    encrytedCW.keyGenCmdID        = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encrytedCW.bSwapAESKey        = false;              
    encrytedCW.bASKMMode          = false; 
    encrytedCW.dest               = algConfig->dest; 

    BKNI_Memcpy(encrytedCW.keyData, drmServiceSettings->keys, drmServiceSettings->keyLength);

    for(index=0;index<(int)(drmServiceSettings->keyLength);index++)
    {
        BDBG_MSG(("contro Word  = 0x%x ",encrytedCW.keyData[index]));
    }           
    
    encrytedCW.keyEntryType       = NEXUS_SecurityKeyType_eOdd;         
    if (NEXUS_Security_GenerateControlWord(drmService->keySlot, &encrytedCW))
    {
        BDBG_MSG(("%s NEXUS Generate Odd ControlWord failed",__FUNCTION__));
        rc = B_DVR_FAIL_TO_GENERATE_CONTROLWORD;
        return rc;          
    }

    if (algConfig->dest == NEXUS_SecurityAlgorithmConfigDestination_eCps)
    {
        encrytedCW.keyEntryType       = NEXUS_SecurityKeyType_eEven;        
        if (NEXUS_Security_GenerateControlWord(drmService->keySlot, &encrytedCW))
        {
            BDBG_MSG(("%s NEXUS Generate Even ControlWord failed",__FUNCTION__));
            rc = B_DVR_FAIL_TO_GENERATE_CONTROLWORD;
            return rc;          
        }

        encrytedCW.keyEntryType       = NEXUS_SecurityKeyType_eClear;           
        if (NEXUS_Security_GenerateControlWord(drmService->keySlot, &encrytedCW))
        {
            BDBG_MSG(("%s NEXUS Generate Clear ControlWord failed",__FUNCTION__));
            rc = B_DVR_FAIL_TO_GENERATE_CONTROLWORD;
            return rc;          
        }
    }

    /* Load IV*/
    NEXUS_Security_GetDefaultClearKey(&key);
    key.keySize = drmServiceSettings->keyLength;
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keyIVType = NEXUS_SecurityKeyIVType_eIV;
    key.dest = algConfig->dest;
    BKNI_Memcpy(key.keyData,drmServiceSettings->ivKeys,drmServiceSettings->keyLength);

    for(index=0;index<(int)(drmServiceSettings->keyLength);index++)
    {
        BDBG_MSG(("iv  = 0x%x ",key.keyData[index]));
    }   
        
    if(NEXUS_Security_LoadClearKey(drmService->keySlot,&key))
    {
        BDBG_MSG(("%s NEXUS load IV failed",__FUNCTION__));
        rc = B_DVR_FAIL_TO_LOAD_IV;
        return rc;  
    }
#else
    BSTD_UNUSED(drmService);
    BSTD_UNUSED(drmServiceSettings);
    BSTD_UNUSED(algConfig);
#endif  
    BDBG_MSG(("%s Exit ",__FUNCTION__));
    return rc;
}

static void B_DVR_DRMService_P_DataEncComplete(void *context,
                                               int param)
{
    B_DVR_DRMServiceHandle drmService = (B_DVR_DRMServiceHandle)context;    
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_MSG(("%s 0x%lx DMA complete fired, drmService[%d]",__FUNCTION__,drmService->dmaJob,param));
    BKNI_SetEvent(drmService->dmaEvent);        
}

static void B_DVR_DRMService_P_DataDecComplete(void *context,
                                               int param)
{
    B_DVR_DRMServiceHandle drmService = (B_DVR_DRMServiceHandle)context;    
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_MSG(("%s 0x%lx DMA complete fired, drmService[%d]",__FUNCTION__,drmService->dmaJob,param));
    BKNI_SetEvent(drmService->dmaEvent);    

}

B_DVR_ERROR B_DVR_DRMService_P_DmaJob(B_DVR_DRMServiceHandle drmService,
                                      B_DVR_DRMServiceStreamBufferInfo *pBufferInfo)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    NEXUS_Error nexusErr;
    NEXUS_DmaJobBlockSettings blockSettings;
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr                  = pBufferInfo->streamBuf;
    blockSettings.pDestAddr                 = pBufferInfo->streamBuf;
    blockSettings.blockSize                 = pBufferInfo->streamBufLen;
    blockSettings.resetCrypto               = true;
    blockSettings.scatterGatherCryptoStart  = true;
    blockSettings.scatterGatherCryptoEnd    = true;
    blockSettings.cached                    = true;

    nexusErr = NEXUS_DmaJob_ProcessBlocks(drmService->dmaJob, &blockSettings, 1);

    /********  
     NOTICE :   
    When NEXUS_DmaJob_ProcessBlocks returns NEXUS_SUCCESS, then the job finished before the function returned.
    In other words, the job is already done.When it returns NEXUS_DMA_QUEUED, then you'll always get a callback when it finishes
    For small chuck of data, NEXUS_DmaJob_ProcessBlocks will return NEXUS_SUCCESS without even going through DMA ISR which actally sets the event
    *********/ 

    if (nexusErr == NEXUS_DMA_QUEUED)
    {
        BDBG_MSG(("0x%lx DMA is in queue ",(unsigned long)drmService->dmaJob));

        B_Mutex_Unlock(drmService->drmMutex);
        BKNI_WaitForEvent(drmService->dmaEvent, BKNI_INFINITE);
        B_Mutex_Lock(drmService->drmMutex);
    
        NEXUS_DmaJob_Destroy(drmService->dmaJob);
        rc = B_DVR_SUCCESS;
    }
    else if (nexusErr != NEXUS_SUCCESS)
    {
        BDBG_WRN(("0x%lx DMA is not success to complete . please retyr again ",(unsigned long)drmService->dmaJob));
        NEXUS_DmaJob_Destroy(drmService->dmaJob);
        rc = B_DVR_DMAJOB_FAIL;
    }
    
    else if (nexusErr == NEXUS_SUCCESS)
    {
        BDBG_MSG(("0x%lx DMA success ",(unsigned long)drmService->dmaJob));
        NEXUS_DmaJob_Destroy(drmService->dmaJob);
        rc = B_DVR_SUCCESS;
    }


    return rc;

}

B_DVR_ERROR B_DVR_DRMService_P_ShowData(B_DVR_DRMServiceStreamBufferInfo *pBufferInfo)
{
    unsigned int i;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    for (i=0;i<pBufferInfo->streamBufLen;i+=10)
    {
        BDBG_MSG(("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                  pBufferInfo->streamBuf[i],  pBufferInfo->streamBuf[i+1],
                  pBufferInfo->streamBuf[i+2],pBufferInfo->streamBuf[i+3],
                  pBufferInfo->streamBuf[i+4],pBufferInfo->streamBuf[i+5],
                  pBufferInfo->streamBuf[i+6],pBufferInfo->streamBuf[i+7],
                  pBufferInfo->streamBuf[i+8],pBufferInfo->streamBuf[i+9]));
    }
    return rc;
}


B_DVR_DRMServiceHandle B_DVR_DRMService_Open(
    B_DVR_DRMServiceRequest *drmServiceRequest)
{
    int num;
    B_DVR_DRMServiceHandle drmService=NULL;
    B_DVR_ManagerHandle dvrManager;
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    
    BDBG_ASSERT(drmServiceRequest);
    dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("%s Total number of drmService assigned =  %d",__FUNCTION__,dvrManager->drmServiceCount));
    if(dvrManager->drmServiceCount>B_DVR_MAX_DRM) 
    {
        BDBG_MSG(("%s DRM index = %d. exceed the max number of DRM service ",__FUNCTION__,dvrManager->drmServiceCount));    
        goto error1;
    }

    drmService = (B_DVR_DRMServiceHandle)BKNI_Malloc(sizeof(*drmService));
    if(!drmService)
    {
        BDBG_ERR(("%s B_DVR_DRMService alloc failed",__FUNCTION__));
        goto error2;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*drmService), true,
                                            __FUNCTION__,__LINE__);
    BKNI_Memset(drmService,0,sizeof(*drmService));
    BDBG_OBJECT_SET(drmService,B_DVR_DRMService);
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    if (dvrManager->drmServiceCount == 0)
    {
        for (num=0;num<B_DVR_MAX_DRM;num++)
        {
            dvrManager->drmService[num] = NULL;             
        }       
        drmService->index = dvrManager->drmServiceCount;        
    }
    else
    {
        for (num=0;num<B_DVR_MAX_DRM;num++)
        {
            if (dvrManager->drmService[num] == NULL)
            {
                drmService->index = num;
                break;
            }
        }               
        if (B_DVR_MAX_DRM == num )
        {
            B_Mutex_Unlock(dvrManager->dvrManagerMutex);
            BDBG_ERR(("$s can not find avaiable drmService structure, failed",__FUNCTION__));
            goto error3;
    }
    }

    dvrManager->drmServiceCount++; /* drmServiceCount means the total number of drmService used */
    dvrManager->drmService[drmService->index] = drmService;

    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    BDBG_MSG(("%s drmService assigned index is %d ",__FUNCTION__,drmService->index));
    BKNI_Memcpy(&drmService->drmServiceRequest,drmServiceRequest,sizeof(drmService->drmServiceRequest));

    drmService->drmMutex = B_Mutex_Create(NULL);
    if(!drmService->drmMutex) 
    {
        BDBG_ERR(("$s B_Mutex_Create failed",__FUNCTION__));
        goto error3;
    }

    if (
        (drmService->drmServiceRequest.drmServiceType == eB_DVR_DRMServiceTypeBroadcastMedia)&&
        ((drmService->drmServiceRequest.keyType == eB_DVR_DRMServiceKeyTypeClear)||
        (drmService->drmServiceRequest.keyType == eB_DVR_DRMServiceKeyTypeProtected))
        )
        {
            if (drmService->drmServiceRequest.keySlot == NULL) 
            {
               NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
               keySlotSettings.keySlotEngine = drmService->drmServiceRequest.securityEngine;
               drmService->keySlot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);      
               drmService->internalAssignedKeyslot = true;
               if (drmService->drmServiceRequest.securityEngine != NEXUS_SecurityEngine_eM2m)
               {
               B_DVR_DRMService_P_InvalidateKey(drmService->keySlot);              
               }
               switch(drmService->drmServiceRequest.securityEngine)                     
               {
                    case NEXUS_SecurityEngine_eCaCp:
                    case NEXUS_SecurityEngine_eM2m:
                    {
                        if((drmService->drmServiceRequest.service == eB_DVR_ServiceTSB)||
                            (drmService->drmServiceRequest.service == eB_DVR_ServiceRecord)||
                            (drmService->drmServiceRequest.service == eB_DVR_ServiceHomeNetworkDRM))
                        {
                            B_DVR_DRMService_P_KeyloaderCallback(drmService);                                   
                        }
                    }
                    break;
                    case NEXUS_SecurityEngine_eCp:
                    {
                        BDBG_MSG(("%s CP engine is not supported ",__FUNCTION__));
                    }
                    break;
                    case NEXUS_SecurityEngine_eCa:
                    {
                        if(drmService->drmServiceRequest.service == eB_DVR_ServicePlayback)
                            BDBG_MSG(("%s Ca & Playback Selected",__FUNCTION__));
                        else
                            BDBG_MSG(("Ca Selected"));
                    }
                    default:
                    {
                        BDBG_MSG(("%s DRM service is for playback",__FUNCTION__));
                    }
                    break;
                }

                BDBG_MSG(("%s NEXUS security keyslot allocated successfully [KeyHandle(0x%x)]",
                                                                                        __FUNCTION__,
                                                                                        drmService->keySlot));
            }   
            else if (drmService->drmServiceRequest.keySlot != NULL)
            {
               drmService->keySlot = drmService->drmServiceRequest.keySlot;
               drmService->internalAssignedKeyslot = false;
                 
               BDBG_MSG(("%s in this DRM index[%d] keyslot was provided from application",__FUNCTION__,
                                                                                          drmService->index));
            }

        }
    else if(drmService->drmServiceRequest.drmServiceType == eB_DVR_DRMServiceTypeContainerMedia)
    {
        BDBG_MSG(("%s eB_DVR_DRMServiceTypeContainerMedia is NOT supported"));
        goto error3;
    }

    if (drmService->drmServiceRequest.dma){
        drmService->dma = drmService->drmServiceRequest.dma;
    }
    
    return drmService;
    
error3:
    BDBG_OBJECT_DESTROY(drmService,B_DVR_DRMService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*drmService),
                                            false, __FUNCTION__,__LINE__);
    BKNI_Free(drmService);
   	drmService = NULL;
error2:
error1:
    return drmService;
}

B_DVR_ERROR B_DVR_DRMService_Close(
    B_DVR_DRMServiceHandle drmService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_ManagerHandle dvrManager=B_DVR_Manager_GetHandle();

    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("%s Enter",__FUNCTION__));

    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->drmService[drmService->index] = NULL;
    dvrManager->drmServiceCount--;
    BDBG_MSG(("%s drmService [%d] is closed. currently [%d] number of drmServices are being used",
              __FUNCTION__,drmService->index,dvrManager->drmServiceCount));
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    B_Mutex_Lock(drmService->drmMutex);
    if( (drmService->keySlot)&&
        (drmService->internalAssignedKeyslot == true))
    {
        NEXUS_Security_FreeKeySlot(drmService->keySlot);
        drmService->internalAssignedKeyslot = false;
        BDBG_MSG(("%s Keyslot free ",__FUNCTION__));
    }
    else
    {
        BDBG_MSG(("%s Keyslot shall be free by application ",__FUNCTION__));
    }
    B_Mutex_Unlock(drmService->drmMutex);
    B_Mutex_Destroy(drmService->drmMutex);
    BDBG_OBJECT_DESTROY(drmService,B_DVR_DRMService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*drmService), 
                                            false, __FUNCTION__,__LINE__);
    BKNI_Free(drmService);
    BDBG_MSG(("%s Exit",__FUNCTION__));
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_Configure(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMServiceSettings *drmServiceSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_ManagerHandle dvrManager;
    NEXUS_SecurityAlgorithmSettings *algConfig; 
 
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_ASSERT(drmServiceSettings);    
    BDBG_MSG(("%s Enter, drmDest(%d), keyLength(%d)",__FUNCTION__,
                                                      drmServiceSettings->drmDest,
                                                      drmServiceSettings->keyLength));
    
    B_Mutex_Lock(drmService->drmMutex);
    dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    dvrManager->drmService[drmService->index]->drmServiceSettings.drmDest= drmServiceSettings->drmDest;

    BKNI_Memcpy(&drmService->drmServiceSettings,drmServiceSettings,sizeof(drmService->drmServiceSettings));

    algConfig = (NEXUS_SecurityAlgorithmSettings *)BKNI_Malloc(sizeof(NEXUS_SecurityAlgorithmSettings));
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(NEXUS_SecurityAlgorithmSettings),
                                            true, __FUNCTION__,__LINE__);
    BKNI_Memset(algConfig,0,sizeof(NEXUS_SecurityAlgorithmSettings));

    B_DVR_DRMServide_P_SetDefaultConfigure(drmService,drmServiceSettings,algConfig);

    if (dvrManager->drmService[drmService->index]->drmServiceRequest.keyType == eB_DVR_DRMServiceKeyTypeClear)
    {
        B_DVR_DRMService_P_ClearKeyConfigure(drmService,drmServiceSettings);
    }
    else if (dvrManager->drmService[drmService->index]->drmServiceRequest.keyType == eB_DVR_DRMServiceKeyTypeProtected)
    {
#if KEYLADDER_SUPPORT   
        B_DVR_DRMService_P_KeyladderConfigure(drmService,drmServiceSettings,algConfig);
#endif
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*algConfig), false,
                                            __FUNCTION__,__LINE__);
    BKNI_Free(algConfig);
    B_Mutex_Unlock(drmService->drmMutex);
    BDBG_MSG(("%s Exit",__FUNCTION__));

    return rc;
}

B_DVR_ERROR B_DVR_DRMService_AddPidChannel(
    B_DVR_DRMServiceHandle drmService,
    NEXUS_PidChannelHandle pid)
{
    BDBG_MSG(("%s Enter",__FUNCTION__));
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_PidChannelStatus pidStatus;

    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_ASSERT(pid);

    B_Mutex_Lock(drmService->drmMutex);
    
    if(drmService->vendorSpecificKeyloader) 
    {
        drmService->vendorSpecificKeyloader(drmService->keySlot);
    }
    
    NEXUS_PidChannel_GetStatus(pid,&pidStatus); 

    if(NEXUS_Security_AddPidChannelToKeySlot(drmService->keySlot,pidStatus.pidChannelIndex))
    {
        rc = B_DVR_FAIL_TO_ADD_KEYSLOT;
        goto err;   
    }
    else
    {
        BDBG_MSG(("%s drmService[%d] is associated with pidchannelIndex[%d]",__FUNCTION__,
                                                                    drmService->index,
                                                                    pidStatus.pidChannelIndex));
    }

    B_Mutex_Unlock(drmService->drmMutex);
    
    return rc;
err:
    BDBG_MSG(("%s adding pidchannelIndex[%d] to keyslot was failed..!!!",__FUNCTION__,pidStatus.pidChannelIndex));
    BDBG_MSG(("%s Please check whether you set pid channel handle correctly or already set it before",__FUNCTION__));
    B_Mutex_Unlock(drmService->drmMutex);
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_RemovePidChannel(
    B_DVR_DRMServiceHandle drmService,
    NEXUS_PidChannelHandle pid)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_PidChannelStatus pidStatus;

    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_MSG(("%s Enter",__FUNCTION__));
    
    B_Mutex_Lock(drmService->drmMutex);

    NEXUS_PidChannel_GetStatus(pid,&pidStatus); 

    if (drmService->keySlot)
    {
        if(NEXUS_Security_RemovePidChannelFromKeySlot(drmService->keySlot,pidStatus.pidChannelIndex))
        {
            rc = B_DVR_FAIL_TO_REMOVE_KEYSLOT;
            goto err1;
        }       
        else
        {
            BDBG_MSG(("%s pidchannelIndex[%d] was removed from drmService[%d]",__FUNCTION__,
                                                                               pidStatus.pidChannelIndex,
                                                                               drmService->index));
        }       
    }
    else
    {
        BDBG_MSG(("\n\n\n %s Keyslot is already NULL..!! \n\n\n",__FUNCTION__));
        goto err2;
    }

    B_Mutex_Unlock(drmService->drmMutex);
    BDBG_MSG(("%s Exit",__FUNCTION__));
    return rc;
err1:
err2:
    BDBG_MSG(("%s removing pidcahnelIndex[%d] from keyslot failed..!!!! ",__FUNCTION__,pidStatus.pidChannelIndex));
    BDBG_MSG(("%s Please have a check whether you already remove pid channel from keyslot before",__FUNCTION__));
    B_Mutex_Unlock(drmService->drmMutex);
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_InstallKeyloaderCallback(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMService_KeyLoaderCallback vendorSprcificKeyloaderCallback,
    NEXUS_KeySlotHandle keySlotHandle)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    B_Mutex_Lock(drmService->drmMutex);
    drmService->vendorSpecificKeyloader = vendorSprcificKeyloaderCallback;
    drmService->keySlot = keySlotHandle;
    B_Mutex_Unlock(drmService->drmMutex);
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_RemoveKeyloaderCallback(
    B_DVR_DRMServiceHandle drmService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    B_Mutex_Lock(drmService->drmMutex);
    drmService->vendorSpecificKeyloader = NULL;
    B_Mutex_Unlock(drmService->drmMutex);
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_EncryptData(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMServiceStreamBufferInfo *pBufferInfo)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    B_Mutex_Lock(drmService->drmMutex);

    BKNI_EventHandle event; 
    BKNI_CreateEvent(&event);   

    NEXUS_DmaJobSettings jobSettings;

    if(drmService->vendorSpecificKeyloader) 
    {
        drmService->vendorSpecificKeyloader(drmService->keySlot);
    }

    drmService->dmaEvent = event; 
        
    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = drmService->keySlot;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eMpeg;
    jobSettings.completionCallback.callback = B_DVR_DRMService_P_DataEncComplete;
    jobSettings.completionCallback.context = drmService;
    jobSettings.completionCallback.param   = drmService->index;
    
    drmService->dmaJob = NEXUS_DmaJob_Create(drmService->dma, &jobSettings);    
    BDBG_MSG(("%s [ created dma job = 0x%lx ]",__FUNCTION__,(unsigned long)drmService->dmaJob));

    BDBG_MSG(("%s [ source data ( origin) before encrypt ]",__FUNCTION__)); 
    B_DVR_DRMService_P_ShowData(pBufferInfo);
    
    rc = B_DVR_DRMService_P_DmaJob(drmService,pBufferInfo);
    
    BDBG_MSG(("%s [ source data (encrypted) after encrypted ]",__FUNCTION__));  
    B_DVR_DRMService_P_ShowData(pBufferInfo);

    BKNI_DestroyEvent(event);
    B_Mutex_Unlock(drmService->drmMutex);
    
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_DecryptData(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMServiceStreamBufferInfo *pBufferInfo)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    B_Mutex_Lock(drmService->drmMutex);
    
    BKNI_EventHandle event; 
    BKNI_CreateEvent(&event);   
    
    NEXUS_DmaJobSettings jobSettings;

    if(drmService->vendorSpecificKeyloader) 
    {
        drmService->vendorSpecificKeyloader(drmService->keySlot);
    }   

    drmService->dmaEvent = event;   
        
    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks                   = 1;
    jobSettings.keySlot                     = drmService->keySlot;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eMpeg;
    jobSettings.completionCallback.callback = B_DVR_DRMService_P_DataDecComplete;
    jobSettings.completionCallback.context = drmService;
    jobSettings.completionCallback.param   = drmService->index;
    drmService->dmaJob = NEXUS_DmaJob_Create(drmService->dma, &jobSettings);    
    BDBG_MSG(("%s [ created dma job = 0x%lx ]",__FUNCTION__,(unsigned long)drmService->dmaJob));

    BDBG_MSG(("%s [ source data ( encrypted ) before decrypt ]",__FUNCTION__)); 
    B_DVR_DRMService_P_ShowData(pBufferInfo);
    
    rc = B_DVR_DRMService_P_DmaJob(drmService,pBufferInfo);
        
    BDBG_MSG(("%s [ source data ( decrypted ) after decrypt]",__FUNCTION__));
    B_DVR_DRMService_P_ShowData(pBufferInfo);

    BKNI_DestroyEvent(event);
    B_Mutex_Unlock(drmService->drmMutex);

    return rc;

}

#else

B_DVR_DRMServiceHandle B_DVR_DRMService_Open(
    B_DVR_DRMServiceRequest *drmServiceRequest)
{
    BSTD_UNUSED(drmServiceRequest);
    B_DVR_DRMServiceHandle drmService=NULL;
    BDBG_MSG(("No DRM supported"));
    return drmService;
}


B_DVR_ERROR B_DVR_DRMService_Close(
    B_DVR_DRMServiceHandle drmService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(drmService);    
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_Configure(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMServiceSettings   *drmServiceSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(drmService);
    BSTD_UNUSED(drmServiceSettings);
    return rc;
}
B_DVR_ERROR B_DVR_DRMService_AddPidChannel(
    B_DVR_DRMServiceHandle drmService,
    NEXUS_PidChannelHandle pid)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(drmService);
    BSTD_UNUSED(pid);
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_RemovePidChannel(
    B_DVR_DRMServiceHandle drmService,
    NEXUS_PidChannelHandle pid)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(drmService);
    BSTD_UNUSED(pid);
    return rc;
}


B_DVR_ERROR B_DVR_DRMService_InstallKeyloaderCallback(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMService_KeyLoaderCallback vendorSprcificKeyloaderCallback,
    NEXUS_KeySlotHandle keySlotHandle)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(drmService);
    BSTD_UNUSED(vendorSprcificKeyloaderCallback);
    BSTD_UNUSED(keySlotHandle);
    return rc;
}


B_DVR_ERROR B_DVR_DRMService_RemoveKeyloaderCallback(
    B_DVR_DRMServiceHandle drmService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(drmService);
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_EncryptData(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMServiceStreamBufferInfo *pBufferInfo)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(drmService);
    BSTD_UNUSED(pBufferInfo);
    return rc;
}

B_DVR_ERROR B_DVR_DRMService_DecryptData(
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMServiceStreamBufferInfo *pBufferInfo)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(drmService);
    BSTD_UNUSED(pBufferInfo);
    return rc;
}


#endif

