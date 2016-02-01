#include "nexus_platform.h"
#include "nexus_security.h"
#include "nexus_keyladder.h"
#include "nexus_dma.h"

BDBG_MODULE(key);

NEXUS_Error Generate_HardwareKey(uint8_t *hardware_secret)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    
    uint8_t *pSrc = NULL;
    uint8_t *pDest = NULL;
    
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_KeySlotHandle hKeySlot = NULL;
    NEXUS_SecurityAlgorithmSettings algoSettings;
    NEXUS_SecurityEncryptedSessionKey sessionKey;
    NEXUS_SecurityEncryptedControlWord ControlWord;

    NEXUS_DmaHandle dma = NULL;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob = NULL;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobStatus jobStatus;

    unsigned char klowVar;
    unsigned char khighVar;
    unsigned char cust_key;

    unsigned char ucProcInForKey3[16] = {  	
	0x43, 0xd6, 0x86, 0xd6, 0xee, 0x34, 0x14, 0x7d, 
        0x7a, 0x5d, 0xb6, 0xce, 0x47, 0x50, 0xd6, 0x71
    };

    unsigned char ucProcInKey4[16] = { 
	0x58, 0x85, 0x1d, 0x2d, 0x67, 0xf4, 0xd8, 0x8e,
        0x41, 0x0f, 0xb6, 0xc9, 0x54, 0x22, 0xa7, 0xb1
    };

    unsigned char encHWKey[32] = {
	0x1c, 0x15, 0x87, 0x67, 0x0c, 0x03, 0xdb, 0x9e, 
        0x13, 0xb4, 0x2b, 0x62, 0xde, 0x41, 0x10, 0x58, 
        0x27, 0x2f, 0x99, 0x5c, 0x35, 0x0a, 0xa6, 0xeb, 
        0xb1, 0xc8, 0x12, 0x5e, 0x50, 0x51, 0x80, 0x10
    };
    

#if (BCHP_CHIP == 7429)||(BCHP_CHIP == 7241) 
	klowVar =  0x1C;
	khighVar = 0x1B;
	cust_key = 0x3F;
#endif

    rc = NEXUS_Memory_Allocate(32, NULL, (void *)&pSrc);
    if(rc != NEXUS_SUCCESS)
    {
	BDBG_WRN(("Unable to Allocate src memory"));
        goto handle_error;
    }

    rc = NEXUS_Memory_Allocate(32, NULL, (void *)&pDest);
    if(rc != NEXUS_SUCCESS)
    {
	BDBG_WRN(("Unable to Allocate dest memory"));
        goto handle_error;
    }

    BKNI_Memset(pDest, 0xAA, 32);
    BKNI_Memcpy(pSrc, encHWKey, 32);
 
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    hKeySlot = NEXUS_Security_AllocateKeySlot(&keySettings);
    if(hKeySlot == NULL) 
    {
        BDBG_WRN(("Unable to allocate keyslot"));
        rc = NEXUS_UNKNOWN;
        goto handle_error;
    }

    NEXUS_Security_GetDefaultAlgorithmSettings(&algoSettings);
    algoSettings.algorithm 		= NEXUS_SecurityAlgorithm_eAes;
    algoSettings.algorithmVar  		= NEXUS_SecurityAlgorithmVariant_eEcb;
    algoSettings.terminationMode	= NEXUS_SecurityTerminationMode_eClear;
    algoSettings.operation 		= NEXUS_SecurityOperation_eDecrypt;
    algoSettings.keyDestEntryType 	= NEXUS_SecurityKeyType_eOdd;
	algoSettings.key2Select			= NEXUS_SecurityKey2Select_eReserved1;

    rc = NEXUS_Security_ConfigAlgorithm(hKeySlot, &algoSettings);
    if(rc != NEXUS_SUCCESS)
    {
        BDBG_WRN(("ConfigAlg keyladder keyslot failed"));
        goto handle_error;
    }
    
    /* Load session key */
	NEXUS_Security_GetDefaultSessionKeySettings(&sessionKey);
    sessionKey.keyladderID 	    = NEXUS_SecurityKeyladderID_eA;
	sessionKey.keyladderType 	= NEXUS_SecurityKeyladderType_e3Des;
    sessionKey.swizzleType	    = NEXUS_SecuritySwizzleType_eSwizzle0;
    sessionKey.cusKeyL 		    = cust_key;
    sessionKey.cusKeyH 		    = cust_key;
    sessionKey.cusKeyVarL 	    = klowVar;
    sessionKey.cusKeyVarH 	    = khighVar;
    
    sessionKey.rootKeySrc 	    = NEXUS_SecurityRootKeySrc_eCuskey;
    sessionKey.bRouteKey 	    = false;
    sessionKey.keyDestIVType	    = NEXUS_SecurityKeyIVType_eNoIV;
    sessionKey.operation 	    = NEXUS_SecurityOperation_eDecrypt;
    sessionKey.operationKey2 	    = NEXUS_SecurityOperation_eEncrypt;
    sessionKey.keyEntryType 	    = NEXUS_SecurityKeyType_eOdd;
    sessionKey.custSubMode          = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    sessionKey.virtualKeyLadderID   = NEXUS_SecurityVirtualKeyladderID_eVKL0;
    sessionKey.keyMode              = NEXUS_SecurityKeyMode_eRegular;

    BKNI_Memcpy(sessionKey.keyData, ucProcInForKey3, sizeof(ucProcInForKey3));
    
    rc = NEXUS_Security_GenerateSessionKey(hKeySlot, &sessionKey);
    if(rc != NEXUS_SUCCESS)
    {
	BDBG_WRN(("Loading session key failed"));
	goto handle_error;
    }
    
    
    /* Load CW */
    NEXUS_Security_GetDefaultControlWordSettings(&ControlWord);
    ControlWord.keyladderID = NEXUS_SecurityKeyladderID_eA;
    ControlWord.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    ControlWord.keySize = sizeof(ucProcInKey4); 
    ControlWord.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    ControlWord.operation = NEXUS_SecurityOperation_eDecrypt; 
    ControlWord.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    ControlWord.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKL0;
    ControlWord.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    BKNI_Memcpy(ControlWord.keyData, ucProcInKey4, ControlWord.keySize);
    ControlWord.bRouteKey = true;
    rc = NEXUS_Security_GenerateControlWord(hKeySlot, &ControlWord);
    if(rc != NEXUS_SUCCESS)
    {
	BDBG_WRN(("Routing session key failed"));
	goto handle_error;
    }
    
    /* Open DMA handle */
    dma = NEXUS_Dma_Open(0, NULL);
    if(dma == NULL)
    {
	BDBG_WRN(("Cannot open DMA"));
	rc = NEXUS_UNKNOWN;
	goto handle_error;
    }
    
    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks 					= 1;
    jobSettings.keySlot 					= hKeySlot;
    jobSettings.dataFormat 					= NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = NULL;
    
    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);
    if(dmaJob == NULL)
    {
	BDBG_WRN(("Cannot create DMA job"));
	rc = NEXUS_UNKNOWN;
	goto handle_error;
    }
    
    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings);
    blockSettings.pSrcAddr 			= pSrc;  
    blockSettings.pDestAddr 			= pDest; 
    blockSettings.blockSize 			= 32;
    blockSettings.resetCrypto 			= true;
    blockSettings.scatterGatherCryptoStart 	= true;
    blockSettings.scatterGatherCryptoEnd 	= true;
    blockSettings.cached 			= true;
    NEXUS_DmaJob_ProcessBlocks(dmaJob, &blockSettings, 1);    

    for(;;)
    {
	NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
	if(jobStatus.currentState == NEXUS_DmaJobState_eComplete)
	{
	    break;
	}
	BKNI_Delay(1);
    }
    
    BKNI_Memcpy(hardware_secret, pDest, 32);
    

handle_error:
    if(dmaJob != NULL)
    {
	NEXUS_DmaJob_Destroy(dmaJob);
        dmaJob = NULL;
    }
    if(dma != NULL)
    {
	NEXUS_Dma_Close(dma);
        dma = NULL;
    }
    if(hKeySlot != NULL) 
    {
	NEXUS_Security_FreeKeySlot(hKeySlot);
        hKeySlot = NULL;
    }
    if(pSrc != NULL)
    {
        NEXUS_Memory_Free(pSrc);
        pSrc = NULL;
    }
    if(pDest != NULL)
    {
        NEXUS_Memory_Free(pDest);
        pDest = NULL;
    }

    return rc;
}

