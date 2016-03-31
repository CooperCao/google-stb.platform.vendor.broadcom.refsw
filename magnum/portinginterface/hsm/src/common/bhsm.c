/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bint.h"
#include "bchp_int_id_bsp.h"
#include "bchp_bsp_cmdbuf.h"
#include "bchp_bsp_glb_control.h"
#if BHSM_SAGE_INTF_SUPPORT
#include "bchp_scpu_globalram.h"
#endif

#include "bhsm.h"
#include "bhsm_datatypes.h"
#include "bhsm_private.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_bsp_interface_legacy.h"
#include "bhsm_keyslots_private.h"
#include "bhsm_verify_reg.h"

BDBG_MODULE(BHSM);

static BERR_Code BHSM_P_BspChannel_Open ( BHSM_Handle hHsm, BHSM_ChannelHandle *phChannel, BHSM_HwModule channelNo );
static BERR_Code BHSM_P_BspChannel_Close( BHSM_ChannelHandle hChannel );


/*******************************************************************************
*  This global variable control which BSP command interface is to be used
*  1. Host MIPS - BSP  ( BHSM_HwModule_eCmdInterface2 )
*  2. SAGE - BSP        ( BHSM_HwModule_eCmdInterface1 ).
*  It is initialized at BHSM_Open().
*******************************************************************************/

unsigned int        BSP_CmdInterface;


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
    pSettings->ucMaxChannels        = 2;
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

    if( (hHsm == NULL) || (hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER) )
    {
        return BERR_TRACE(BHSM_STATUS_INPUT_PARM_ERR);
    }

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

    if( keyslotTypes.numKeySlot[0] >= BHSM_NUM_BYPASS_KEYLSOTS )
    {
        keyslotTypes.numKeySlot[0] -= BHSM_NUM_BYPASS_KEYLSOTS;   /* Two keyslots of type zero are reserved for bypass keyslots. */
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
    unsigned int channelNum, i, j;
    BCHP_Info chipInfo;
    BHSM_BspMsgConfigure_t bspMsgConfig;

    BDBG_ENTER( BHSM_Open );
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hReg );
    BDBG_ASSERT( hInterrupt );

    *hpHsm = NULL;

    /* First set up the BSP Command interface to use */
#if BHSM_SAGE_BSP_PI_SUPPORT
    BSP_CmdInterface = BHSM_HwModule_eCmdInterface1;
#else
    BSP_CmdInterface = BHSM_HwModule_eCmdInterface2;
#endif

    /* Alloc memory from the system heap */
    hHsm = (BHSM_Handle)BKNI_Malloc( sizeof(BHSM_P_Handle) );
    if( hHsm == NULL )
    {
        errCode = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto BHSM_P_DONE_LABEL;
    }

    BKNI_Memset( hHsm, 0, sizeof( BHSM_P_Handle) );

    hHsm->ulMagicNumber = BHSM_P_HANDLE_MAGIC_NUMBER;
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

    /*  Enable interrupt   */
    /* With 40-nm and 28-nm platforms, OLOAD1_INTR is for SAGE and OLOAD2_INTR for host MIPS */

    errCode = BINT_CreateCallback( &(hHsm->IntCallback), hHsm->interruptHandle, HSM_L2_INTR, BHSM_P_IntHandler_isr, (void*)hHsm, 0x00 );
    if( errCode != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    if( ( errCode = BINT_EnableCallback( hHsm->IntCallback ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

#if BHSM_SAGE_BSP_PI_SUPPORT
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_OLOAD1, 0);
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_ILOAD1, 0);
#else
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN, 0);
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS, 0);
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_OLOAD2, 0);
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_ILOAD2, 0);
    #ifdef BCHP_BSP_GLB_CONTROL_GLB_RAAGA_INTR_STATUS /* define may not be available/relevant on legacy platforms */
    BREG_Write32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_RAAGA_INTR_STATUS, 0xFFFFFFFFL );
    #endif
#endif
    BCHP_GetInfo( hHsm->chipHandle, &chipInfo);  /* Get the chip information */

    BDBG_MSG(("Chip[%x], Rev[0x%x] Client[%s]", chipInfo.familyId
                                                , chipInfo.rev
                                                , hHsm->currentSettings.clientType==BHSM_ClientType_eHost?"HOST":"SAGE" ));

    if (hHsm->currentSettings.ucMaxChannels == 0)
    {
        hHsm->currentSettings.ucMaxChannels =  BHSM_MAX_SUPPOTED_CHANNELS;
    }

    for( channelNum = 0;
        channelNum < hHsm->currentSettings.ucMaxChannels;
        channelNum++ )
    {
        hHsm->channelHandles[channelNum] = NULL;
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

    if( ( errCode = BHSM_P_BspChannel_Open( hHsm, &(hHsm->channelHandles[0]), 0 ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    if( ( errCode = BHSM_P_BspChannel_Open( hHsm, &(hHsm->channelHandles[1]), 1 ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }


    BKNI_Memset( &bspMsgConfig, 0, sizeof(bspMsgConfig) );

    bspMsgConfig.ulInCmdBufAddr  = hHsm->channelHandles[BSP_CmdInterface]->ulInCmdBufAddr;
    bspMsgConfig.ulOutCmdBufAddr = hHsm->channelHandles[BSP_CmdInterface]->ulOutCmdBufAddr;
    bspMsgConfig.ulILoadRegAddr  = hHsm->channelHandles[BSP_CmdInterface]->ulILoadRegAddr;
    bspMsgConfig.ulILoadVal      = hHsm->channelHandles[BSP_CmdInterface]->ulILoadVal;
    bspMsgConfig.ulIReadyRegAddr = hHsm->channelHandles[BSP_CmdInterface]->ulIReadyRegAddr;
    bspMsgConfig.ulIReadyVal     = hHsm->channelHandles[BSP_CmdInterface]->ulIReadyVal;
    bspMsgConfig.oLoadWait       = hHsm->channelHandles[BSP_CmdInterface]->oLoadWait;
    bspMsgConfig.oLoadSet        = hHsm->channelHandles[BSP_CmdInterface]->oLoadSet;

   #if BHSM_SAGE_BSP_PI_SUPPORT
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

#if BHSM_SAGE_INTF_SUPPORT

    if( hHsm->currentSettings.clientType == BHSM_ClientType_eSAGE )
    {
        BHSM_KeyslotTypes_t keyslots;

        BDBG_MSG(("SAGE init HSM"));

        /* load the keyslot config from SRAM */
        BHSM_P_StashKeySlotTableRead( hHsm, &keyslots );

        /* configure internal data structures */
        if( ( errCode = BHSM_P_KeySlotsInitialise( hHsm, &keyslots ) ) !=  BERR_SUCCESS )
        {
            (void)BERR_TRACE( errCode );
            goto BHSM_P_DONE_LABEL;
        }
    }

    hHsm->hsmPiRunningFullRom = isSecurityInRom( hHsm );

#endif /* BHSM_SAGE_INTF_SUPPORT  */

    if( ( errCode = loadBspVersion( hHsm ) ) != BERR_SUCCESS )
    {
        BERR_TRACE(errCode);  /* failed to determine the BSP version. */
        goto BHSM_P_DONE_LABEL;
    }

    if( ( errCode = BHSM_RegionVerification_Init( hHsm ) ) != BERR_SUCCESS )
    {
        BERR_TRACE(errCode);  /* Failed to initialise Region verification. */
        goto BHSM_P_DONE_LABEL;
    }

    *hpHsm = hHsm;
    hHsm->bIsOpen = true;

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

    if( ( hHsm ==  NULL ) || ( hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) )
    {
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    BHSM_BspMsg_Uninit( hHsm );

    if( hHsm->bIsOpen ==  false )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( BINT_DisableCallback( hHsm->IntCallback ) != BERR_SUCCESS )
    {
        errCode |= BERR_TRACE( BHSM_STATUS_FAILED );  /* continue, best effort */
    }

    if( BINT_DestroyCallback( hHsm->IntCallback ) != BERR_SUCCESS )
    {
        errCode |= BERR_TRACE( BHSM_STATUS_FAILED );
    }

    #if  (BHSM_IPTV == 1)
    if( hHsm->pContiguousMem )
    {
        BMEM_Free ( hHsm->hHeap, (void*)hHsm->pContiguousMem );
        hHsm->pContiguousMem = NULL;
    }
    #endif

    hHsm->bIsOpen = false;
    hHsm->ulMagicNumber = 0; /* kill the magic */

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

    if( ( errCode = BHSM_P_BspChannel_Close( hHsm->channelHandles[0] ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );     /* continue, best effort */
    }

    if( ( errCode = BHSM_P_BspChannel_Close( hHsm->channelHandles[1] ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );     /* continue, best effort */
    }

    hHsm->channelHandles[0] = NULL;
    hHsm->channelHandles[1] = NULL;

    BKNI_Free(  hHsm );
    hHsm = NULL;

    BDBG_LEAVE( BHSM_Close );
    return errCode;
}




BERR_Code BHSM_P_BspChannel_Open( BHSM_Handle         hHsm,
                                  BHSM_ChannelHandle *phChannel,
                                  BHSM_HwModule       channelNo )
{
    BERR_Code errCode = BERR_SUCCESS;
    BHSM_ChannelHandle hChannel = NULL;

    BDBG_ENTER( BHSM_P_BspChannel_Open );
    BDBG_ASSERT( hHsm );

    if( hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( channelNo >= hHsm->currentSettings.ucMaxChannels )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    if( hHsm->bIsOpen == true )
    {
        return BERR_TRACE(BHSM_STATUS_FAILED);
    }

    *phChannel = NULL;

    if ( ( hChannel = (BHSM_ChannelHandle)BKNI_Malloc( sizeof(BHSM_P_ChannelHandle) ) ) == NULL )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto BHSM_P_DONE_LABEL;
    }

    BKNI_Memset(hChannel, 0, sizeof( BHSM_P_ChannelHandle ));

    hChannel->ulMagicNumber = BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER;
    hChannel->moduleHandle = hHsm;
    hChannel->oLoadSet = 0;
    hChannel->ulSequenceNum = 42;

    if( ( errCode = BKNI_CreateEvent( &(hChannel->oLoadWait) ) ) != BERR_SUCCESS )
    {
        (void)BERR_TRACE( errCode );
        goto BHSM_P_DONE_LABEL;
    }

    BDBG_MSG(("Allocated Channel Number [%d]", channelNo));

    switch( channelNo ) {
        case BHSM_HwModule_eCmdInterface1:
            hChannel->ulInCmdBufAddr = BHSM_IN_BUF1_ADDR;
            hChannel->ulOutCmdBufAddr = BHSM_OUT_BUF1_ADDR;
            hChannel->ulILoadRegAddr = BCHP_BSP_GLB_CONTROL_GLB_ILOAD1;
            hChannel->ulILoadVal = BCHP_BSP_GLB_CONTROL_GLB_ILOAD1_CMD_ILOAD1_MASK;
            hChannel->ulIReadyRegAddr = BCHP_BSP_GLB_CONTROL_GLB_IRDY;
            hChannel->ulIReadyVal= BCHP_BSP_GLB_CONTROL_GLB_IRDY_CMD_IDRY1_MASK;

            break;

        case BHSM_HwModule_eCmdInterface2:
            hChannel->ulInCmdBufAddr = BHSM_IN_BUF2_ADDR;
            hChannel->ulOutCmdBufAddr = BHSM_OUT_BUF2_ADDR;
            hChannel->ulILoadRegAddr = BCHP_BSP_GLB_CONTROL_GLB_ILOAD2;
            hChannel->ulILoadVal = BCHP_BSP_GLB_CONTROL_GLB_ILOAD2_CMD_ILOAD2_MASK;
            hChannel->ulIReadyRegAddr = BCHP_BSP_GLB_CONTROL_GLB_IRDY;
            hChannel->ulIReadyVal= BCHP_BSP_GLB_CONTROL_GLB_IRDY_CMD_IDRY2_MASK;

            break;

        default:
            errCode = BERR_INVALID_PARAMETER;
            goto BHSM_P_DONE_LABEL;
    }

    BDBG_MSG(("ulInCmdBufAddr = 0x%lx, ulOutCmdBufAddr = 0x%lx",  hChannel->ulInCmdBufAddr, hChannel->ulOutCmdBufAddr));

    hChannel->ucChannelNumber =  channelNo;

    *phChannel = hChannel;
    hChannel->bIsOpen = true;

BHSM_P_DONE_LABEL:
    if( errCode != BERR_SUCCESS )
    {
        if( hChannel != NULL )
        {
            BKNI_Free( hChannel );
        }
    }

    BDBG_LEAVE( BHSM_BspChannel_Open );
    return errCode;
}

BERR_Code BHSM_P_BspChannel_Close( BHSM_ChannelHandle hChannel )
{

    BDBG_ENTER( BHSM_P_BspChannel_Close );
    BDBG_ASSERT( hChannel );

    if( hChannel->ulMagicNumber != BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( hChannel->bIsOpen == false )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    BKNI_DestroyEvent( hChannel->oLoadWait );

    hChannel->bIsOpen = false;
    hChannel->ulMagicNumber = 0; /* kill the magic  */

    BKNI_Free( hChannel );
    hChannel = NULL;

    BDBG_LEAVE( BHSM_P_BspChannel_Close );

    return BERR_SUCCESS;
}


/* Function Deprecated */
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

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    header.isRaw =  true;
    header.commandLength = inputParamLenInWord * sizeof(uint32_t);
    header.hChannel = hHsm->channelHandles[interface];
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

    if( hMsg )
    {
        (void)BHSM_BspMsg_Destroy( hMsg );
    }

    BDBG_LEAVE( BHSM_SubmitRawCommand );
    return rc;
}



/* Function DEPRECATED */
BERR_Code BHSM_GetTotalChannels( BHSM_Handle hHsm, unsigned char *outp_ucTotalChannels )
{
    BDBG_ENTER( BHSM_GetTotalChannels );
    BSTD_UNUSED( hHsm );
    *outp_ucTotalChannels = 0;
    BDBG_ERR(( "Function [%s] is DEPRECATED. Do not call.", __FUNCTION__ ));
    BDBG_LEAVE( BHSM_GetTotalChannels );
    return BERR_NOT_SUPPORTED;
}

/* Function DEPRECATED */
BERR_Code BHSM_GetChannelDefaultSettings(
        BHSM_Handle              hHsm,
        BHSM_HwModule            in_channelNo,
        BHSM_ChannelSettings    *outp_sSettings )
{
    BDBG_ENTER( BHSM_GetChannelDefaultSettings );
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( in_channelNo );
    BDBG_ERR(( "Function [%s] is DEPRECATED. Do not call.", __FUNCTION__ ));
    BKNI_Memset( outp_sSettings, 0, sizeof( *outp_sSettings ) );
    BDBG_LEAVE( BHSM_GetChannelDefaultSettings );
    return BERR_NOT_SUPPORTED;
}

/* Function DEPRECATED */
BERR_Code BHSM_Channel_Open(
        BHSM_Handle                  hHsm,
        BHSM_ChannelHandle          *outp_channelHandle,
        BHSM_HwModule                in_channelNo,
        const BHSM_ChannelSettings  *inp_channelDefSettings )
{
    BDBG_ENTER( BHSM_Channel_Open );
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( in_channelNo );
    BSTD_UNUSED( inp_channelDefSettings );
    *outp_channelHandle =  NULL;
    BDBG_ERR(( "Function [%s] is DEPRECATED. Do not call.", __FUNCTION__ ));
    BDBG_LEAVE( BHSM_Channel_Open );
    return BERR_NOT_SUPPORTED;
}

/* Function DEPRECATED */
BERR_Code BHSM_Channel_Close( BHSM_ChannelHandle inout_channelHandle )
{
    BDBG_ENTER( BHSM_Channel_Close );
    BSTD_UNUSED( inout_channelHandle );
    BDBG_ERR(( "Function [%s] is DEPRECATED. Do not call.", __FUNCTION__ ));
    BDBG_LEAVE( BHSM_Channel_Close );
    return BERR_NOT_SUPPORTED;
}

/* Function DEPRECATED */
BERR_Code BHSM_Channel_GetDevice(
        BHSM_ChannelHandle   in_channelHandle,
        BHSM_Handle         *hpHsm )
{
    BDBG_ENTER( BHSM_Channel_GetDevice );
    BSTD_UNUSED( in_channelHandle );
    BSTD_UNUSED( hpHsm );
    BDBG_ERR(( "Function [%s] is DEPRECATED. Do not call.", __FUNCTION__ ));
    BDBG_LEAVE( BHSM_Channel_GetDevice );
    return BERR_NOT_SUPPORTED;
}

/* Function DEPRECATED */
BERR_Code BHSM_GetChannel( BHSM_Handle hHsm, BHSM_HwModule channelNo, BHSM_ChannelHandle *pChannelHandle )
{
    BDBG_ENTER( BHSM_GetChannel );
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( channelNo );
    BSTD_UNUSED( pChannelHandle );
    BDBG_ERR(( "Function [%s] is DEPRECATED. Do not call.", __FUNCTION__ ));
    BDBG_LEAVE( BHSM_GetChannel );
    return BERR_NOT_SUPPORTED;
}

/* Function DEPRECATED */
BERR_Code BHSM_SetSettings( BHSM_Handle hHsm, BHSM_NewSettings_t *pNewSettings )
{
    BDBG_ENTER( BHSM_SetSettings );
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pNewSettings );
    BDBG_ERR(( "Function [%s] is DEPRECATED. Do not call.", __FUNCTION__ ));
    BDBG_LEAVE( BHSM_SetSettings );
    return BERR_NOT_SUPPORTED;
}

/* Function DEPRECATED */
BERR_Code BHSM_SetIntrCallback( BHSM_Handle hHsm, BHSM_IntrType pIntType, BHSM_IsrCallbackFunc callback )
{
    BDBG_ENTER( BHSM_SetIntrCallback );
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pIntType );
    BSTD_UNUSED( callback );
    BDBG_ERR(( "Function [%s] is DEPRECATED. Do not call.", __FUNCTION__ ));
    BDBG_LEAVE( BHSM_SetIntrCallback );
    return BERR_NOT_SUPPORTED;
}
