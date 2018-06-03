/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bint.h"
#include "bchp_int_id_bsp.h"
#include "bchp_bsp_cmdbuf.h"
#include "bchp_bsp_glb_control.h"
#include "bhsm.h"
#include "bhsm_datatypes.h"
#include "bhsm_private.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_keyslots_private.h"
#include "bhsm_verify_reg.h"
#ifdef BSP_PKL
#include "bchp_bsp_pkl.h"
#endif

BDBG_MODULE(BHSM);

BDBG_OBJECT_ID( BHSM_P_Handle );


/*******************************************************************************
*    Public Module Functions
*******************************************************************************/
BERR_Code BHSM_GetDefaultSettings( BHSM_Settings *pSettings, BCHP_Handle chipHandle )
{

    BDBG_ENTER( BHSM_GetDefaultSettings );

    BSTD_UNUSED( chipHandle );

    if( pSettings == NULL )
    {
        return BHSM_STATUS_INPUT_PARM_ERR;
    }

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );
    pSettings->ulTimeOutInMilSecs   = 2000;
    pSettings->hHeap                = NULL;
    pSettings->clientType           = BHSM_ClientType_eHost;

    BDBG_LEAVE( BHSM_GetDefaultSettings );
    return BERR_SUCCESS ;
}


BERR_Code BHSM_GetCapabilities( BHSM_Handle hHsm, BHSM_Capabilities_t *pCaps )
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_KeyslotTypes_t keyslotTypes;
    unsigned int x = 0;

    BDBG_ENTER( BHSM_GetCapabilities );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( pCaps == NULL )
    {
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

    pCaps->version.zeus     = BHSM_ZEUS_VERSION;
    pCaps->version.firmware = hHsm->firmwareVersion;
    BDBG_MSG(("VERSION Zeus[%d,%d], FW[0x%X,0x%X]",  BHSM_ZEUS_VER_MAJOR,
                                                       BHSM_ZEUS_VER_MINOR,
                                                       pCaps->version.firmware.platform.major,
                                                       pCaps->version.firmware.platform.minor ));

    /* Get KeySlot details. */
    keyslotTypes.numKeySlotTypes = hHsm->numKeySlotTypes;
    BDBG_MSG(("Num Keyslot type%d", keyslotTypes.numKeySlotTypes ));

    for( x = 0; x < keyslotTypes.numKeySlotTypes; x++ )
    {
        keyslotTypes.numKeySlot[x] = (uint8_t)hHsm->keySlotTypes[x].unKeySlotNum;
        BDBG_MSG(("Keyslot type%d [%d]", x, keyslotTypes.numKeySlot[x] ));
    }

    keyslotTypes.numMulti2KeySlots = hHsm->numMulti2KeySlots;
    BDBG_MSG(("Multi2 Keyslots [%d]", keyslotTypes.numMulti2KeySlots ));

    if( keyslotTypes.numKeySlot[0] >= BHSM_NUM_BYPASS_KEYSLOTS )
    {
        keyslotTypes.numKeySlot[0] -= BHSM_NUM_BYPASS_KEYSLOTS;   /* Two keyslots of type zero are reserved for bypass keyslots. */
    }
    else
    {
        BDBG_WRN(("Fewer Type0 keyslots available that expected [%d].", keyslotTypes.numKeySlot[0] ));
    }

    pCaps->keyslotTypes = keyslotTypes;
    pCaps->keyslotOwnershipSupported = isKeySlotOwershipSupported( hHsm );

    BDBG_LEAVE( BHSM_GetCapabilities );
    return rc;
}






BERR_Code BHSM_Open( BHSM_Handle            *hpHsm,
                     BREG_Handle            hReg,
                     BCHP_Handle            hChip,
                     BINT_Handle            hInterrupt,
                     const BHSM_Settings    *pSettings )
{
    BERR_Code errCode = BERR_SUCCESS;
    BHSM_Handle hHsm;
    unsigned i;
    unsigned j;
    BCHP_Info chipInfo;
    BHSM_BspMsgConfigure_t bspMsgConfig;

    BDBG_ENTER( BHSM_Open );
    BDBG_ASSERT( hReg );
    BDBG_ASSERT( hInterrupt );

    *hpHsm = NULL;

    hHsm = (BHSM_Handle)BKNI_Malloc( sizeof(BHSM_P_Handle) );
    if( hHsm == NULL )
    {
        errCode = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto BHSM_P_DONE_LABEL;
    }

    BKNI_Memset( hHsm, 0, sizeof( BHSM_P_Handle) );

    BDBG_OBJECT_SET( hHsm, BHSM_P_Handle );

    hHsm->chipHandle = hChip;
    hHsm->regHandle = hReg;
    hHsm->interruptHandle = hInterrupt;
    if (pSettings == NULL)
    {
        BHSM_GetDefaultSettings( &hHsm->currentSettings, NULL ); /* ignore return value */
    }
    else
    {
        hHsm->currentSettings = *pSettings;
    }

    if ( hHsm->currentSettings.hHeap != NULL )
    {
        hHsm->hHeap = hHsm->currentSettings.hHeap;

       #if  (BHSM_IPTV == 1)
        hHsm->pContiguousMem = BMEM_AllocAligned( hHsm->hHeap, BHSM_CONTIGUOUS_MEMORY_SIZE, 6,0 ); /* 64 bytes allignement*/
        if( hHsm->pContiguousMem == NULL )
        {
            /* failed to allocate memory for IPTV operations */
            errCode = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
            goto BHSM_P_DONE_LABEL;
        }
       #endif
    }

    if( hHsm->chipHandle )
    {
        BCHP_GetInfo( hHsm->chipHandle, &chipInfo);  /* Get the chip information */
        BDBG_MSG(("Chip[%x], Rev[0x%x] Client[%s]", chipInfo.familyId
                                                , chipInfo.rev
                                                , hHsm->currentSettings.clientType==BHSM_ClientType_eHost?"HOST":"SAGE" ));
    }

    /* Initialize PidChannelToKeySlotNum matrices */
    for( i = 0; i < BCMD_TOTAL_PIDCHANNELS; i++ )
    {
        for( j = 0; j < 2; j++ )
        {
            hHsm->mapPidChannelToKeySlot[i][j].keySlotType  = BCMD_XptSecKeySlot_eTypeMax;
            hHsm->mapPidChannelToKeySlot[i][j].unKeySlotNum = BHSM_SLOT_NUM_INIT_VAL;
        }
    }

    /* initialise the external key table */
    for( i = 0; i < BHSM_EXTERNAL_KEYSLOTS_MAX; i++ )
    {
        hHsm->externalKeySlotTable[i].allocated = false;
        hHsm->externalKeySlotTable[i].slotPtr   = (i * BHSM_EXTERNAL_KEYSLOT_SLOT_SIZE);
        hHsm->externalKeySlotTable[i].key.offset = 0;
        hHsm->externalKeySlotTable[i].iv.offset  = BHSM_EXTERNAL_KEYSLOT_KEY_SIZE;
    }


    hHsm->bIsOpen = true;

    BKNI_Memset( &bspMsgConfig, 0, sizeof(bspMsgConfig) );
    #if BHSM_BUILD_HSM_FOR_SAGE
    if( hHsm->currentSettings.secureMemory.size )
    {
        if( hHsm->currentSettings.secureMemory.p == NULL  )
        {
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto BHSM_P_DONE_LABEL;
        }
        bspMsgConfig.secureMemory.size = hHsm->currentSettings.secureMemory.size;
        bspMsgConfig.secureMemory.p    = hHsm->currentSettings.secureMemory.p;
    }
    #endif
    if( ( errCode = BHSM_BspMsg_Init( hHsm, &bspMsgConfig ) != BERR_SUCCESS ) )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    if( ( errCode = loadBspVersion( hHsm ) ) != BERR_SUCCESS )
    {
        BERR_TRACE(errCode);  /* failed to determine the BSP version. */
        goto BHSM_P_DONE_LABEL;
    }


   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    if( hHsm->currentSettings.clientType == BHSM_ClientType_eSAGE )
    {
        BHSM_KeyslotTypes_t keyslots;

        BDBG_MSG(("SAGE init HSM"));

        if( (errCode = BHSM_P_KeySlotsTableConfGet( hHsm, &keyslots )) != BERR_SUCCESS )
        {
            (void)BERR_TRACE( errCode );
            goto BHSM_P_DONE_LABEL;
        }

        /* configure internal data structures */
        if( (errCode = BHSM_P_KeySlotsInitialise( hHsm, &keyslots )) !=  BERR_SUCCESS )
        {
            (void)BERR_TRACE( errCode );
            goto BHSM_P_DONE_LABEL;
        }
    }
   #endif

    hHsm->hsmPiRunningFullRom = isSecurityInRom( hHsm );

    if( ( errCode = BHSM_RegionVerification_Init( hHsm ) ) != BERR_SUCCESS )
    {
        BERR_TRACE(errCode);  /* Failed to initialise Region verification. */
        goto BHSM_P_DONE_LABEL;
    }

    *hpHsm = hHsm;

BHSM_P_DONE_LABEL:

    if( errCode != BERR_SUCCESS )
    {
        if( hHsm )
        {
            for( i = 0; i < BHSM_MAX_KEYLSOTS; i++ )
            {
                if( hHsm->pKeySlot[i] )
                {
                    BKNI_Free( hHsm->pKeySlot[i] );
                }
            }
            BKNI_Free( hHsm );
        }
    }

    BDBG_LEAVE( BHSM_Open );
    return errCode;
}


BERR_Code BHSM_Close( BHSM_Handle hHsm )
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned i = 0;

    BDBG_ENTER( BHSM_Close );

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    BHSM_BspMsg_Uninit( hHsm );

    if( hHsm->bIsOpen ==  false )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }


    #if  (BHSM_IPTV == 1)
    if( hHsm->pContiguousMem )
    {
        BMEM_Free ( hHsm->hHeap, (void*)hHsm->pContiguousMem );
        hHsm->pContiguousMem = NULL;
    }
    #endif

    hHsm->bIsOpen = false;

    for( i=0; i < BHSM_MAX_KEYLSOTS; i++ )
    {
        if( hHsm->pKeySlot[i] )
        {
            BKNI_Free( hHsm->pKeySlot[i] );
        }
    }

    if( ( errCode = BHSM_RegionVerification_UnInit( hHsm ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( errCode );  /* Failed to uninitialise Region verification. Continue */
    }

    BDBG_OBJECT_DESTROY( hHsm, BHSM_P_Handle );

    BKNI_Free(  hHsm );
    hHsm = NULL;

    BDBG_LEAVE( BHSM_Close );
    return errCode;
}

/* Function Deprecated. Please request that the BSP comamand be exposed via a decicated API function. */
BERR_Code BHSM_SubmitRawCommand (
    BHSM_Handle         hHsm,
    BHSM_HwModule        interface,
    unsigned            inputParamLenInWord,
    uint32_t            *pInputParamsBuf,
    unsigned            *pOutputParamLenInWord,  /* in-out */
    uint32_t            *pOutputParamsBuf
    )
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    unsigned            i;
    uint16_t            outputLength = 0;

    BSTD_UNUSED( interface );
    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle );

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    header.isRaw =  true;
    header.commandLength = inputParamLenInWord * sizeof(uint32_t);
    #define RAW_COMMAND NULL
    BHSM_BspMsg_Header( hMsg, (BCMD_cmdType_e)RAW_COMMAND, &header );

    /* Load input command */
    for( i = 0; i < inputParamLenInWord; i++ )
    {
        BHSM_BspMsg_Pack32( hMsg, i*4, pInputParamsBuf[i] );
    }

    /* Submit the command */
    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    /* Read command response length */
    BHSM_BspMsg_Get16( hMsg, BCMD_CommonBufferFields_eParamLen, &outputLength );

    outputLength = outputLength + (((BCMD_CommonBufferFields_eParamLen)/4)*4) + 4; /*include header */

    if( outputLength  > *pOutputParamLenInWord * sizeof(uint32_t) )
    {
        rc = BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
        goto BHSM_P_DONE_LABEL;
    }

    if( outputLength > BHSM_P_BSP_MSG_SIZE )
    {
        rc = BERR_TRACE( BHSM_STATUS_BSP_ERROR );
        goto BHSM_P_DONE_LABEL;
    }

    BDBG_MSG(("Raw command len in[%d] out[%d] out[%d]", inputParamLenInWord, outputLength, *pOutputParamLenInWord ));

    *pOutputParamLenInWord = outputLength / sizeof(uint32_t);

    for( i = 0; i < *pOutputParamLenInWord; i++ )
    {
        BHSM_BspMsg_Get32( hMsg, i*4, &pOutputParamsBuf[i] );
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_SubmitRawCommand );
    return rc;
}
