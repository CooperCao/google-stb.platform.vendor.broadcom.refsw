/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "bhsm.h"
#include "bhsm_private.h"
#include "bhsm_misc.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_mem_auth.h"
#include "bhsm_keyladder_enc.h"

BDBG_MODULE(BHSM);



BERR_Code BHSM_SetMiscBits (
    BHSM_Handle                hHsm,
    BHSM_SetMiscBitsIO_t    *pMiscBits
)
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t status;
    uint8_t controlBits = 0;
    uint8_t forceSc = 0;

    BDBG_ENTER( BHSM_SetMiscBits );

    BDBG_ASSERT( pMiscBits );

    if( ( hHsm == NULL ) || ( hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) ) {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( pMiscBits == NULL ) {
        return BERR_TRACE(  BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOFFLINE_SetMiscBits_COMMAND, &header );
    #else
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOFFLINE_RAVE_COMMAND, &header );
    #endif

    BHSM_BspMsg_Pack8( hMsg, BCMD_SetMiscBitsField_InCmdField_eSubCommandId, pMiscBits->setMiscBitsSubCmd );

    switch( pMiscBits->setMiscBitsSubCmd )
    {
        case BCMD_SetMiscBitsSubCmd_eRaveBits:
        {
            controlBits = 0;
            if( pMiscBits->bDisableClear         ) controlBits &= BCMD_MISC_RaveCtrlBitsMask_eDISABLE_CLR;
            if( pMiscBits->bRAVEEncryptionBypass ) controlBits &= BCMD_MISC_RaveCtrlBitsMask_eRESERVED;
            if( pMiscBits->bEnableReadDMem       ) controlBits &= BCMD_MISC_RaveCtrlBitsMask_eDMEM_RD;
            if( pMiscBits->bEnableReadIMem       ) controlBits &= BCMD_MISC_RaveCtrlBitsMask_eIMEM_RD;
            if( pMiscBits->bEnableWriteIMem      ) controlBits &= BCMD_MISC_RaveCtrlBitsMask_eIMEM_WR;
            BHSM_BspMsg_Pack8( hMsg, BCMD_SetMiscBitsField_InCmdField_eCtrlBits, controlBits );
            break;
        }
        case BCMD_SetMiscBitsSubCmd_eReserved1:
        {
            controlBits = 0;
            if( pMiscBits->bDMAM2MPacketBypassEnable ) controlBits &= (0x1 << 2);
            if( pMiscBits->bNMAPacketBypassEnable    ) controlBits &= (0x1 << 1);
            if( pMiscBits->bDMAXPTPacketBypassEnable ) controlBits &=  0x1;
            BHSM_BspMsg_Pack8( hMsg, BCMD_SetMiscBitsField_InCmdField_eCtrlBits, controlBits );
            break;
        }
        case BCMD_SetMiscBitsSubCmd_eM2MEndianSwapBits:
        {
            controlBits = 0;
            if( pMiscBits->bCBCMACKeySwapEnable     ) controlBits &= (0x1<< 2);
            if( pMiscBits->bCBCMACDataOutSwapEnable ) controlBits &= (0x1<< 1);
            if( pMiscBits->bCBCMACDataInSwapEnable  ) controlBits &=  0x1;
            BHSM_BspMsg_Pack8( hMsg, BCMD_SetMiscBitsField_InCmdField_eCtrlBits, controlBits );
            break;
        }
        case BCMD_SetMiscBitsSubCmd_eForceSCBit:
        {
            BHSM_BspMsg_Pack8( hMsg, BCMD_SetMiscBitsField_InCmdField_eBandSel, pMiscBits->bBandSelect_PB );
            BHSM_BspMsg_Pack8( hMsg, BCMD_SetMiscBitsField_InCmdField_eBandNum, pMiscBits->bandNumber );

            forceSc = 0;
            if( pMiscBits->SCForceEnValue ) forceSc &= (0x3 << 2);
            if( pMiscBits->bSCForceEnNZ   ) forceSc &= (0x1 << 1);
            if( pMiscBits->bSCForceEnAll  ) forceSc &=  0x1;
            BHSM_BspMsg_Pack8( hMsg, BCMD_SetMiscBitsField_InCmdField_eForceSCbits,forceSc );
            break;
        }
        case BCMD_SetMiscBitsSubCmd_eXPTPipesBypass:
        {
            controlBits = 0;
            if( pMiscBits->bXPTPipesBypass ) controlBits &=  0x1;
            BHSM_BspMsg_Pack8( hMsg, BCMD_SetMiscBitsField_InCmdField_eCtrlBits, controlBits );
            break;
        }
        case BCMD_SetMiscBitsSubCmd_eXPTReset:
        {
            controlBits = 0;
            if( pMiscBits->bXPTReset ) controlBits &=  0x1;
            BHSM_BspMsg_Pack8( hMsg, BCMD_SetMiscBitsField_InCmdField_eCtrlBits, controlBits );
            break;
        }
        case BCMD_SetMiscBitsSubCmd_eXPTM2MStatusDump:  /* No parameter, just get status */
        {
            break;
        }
        default:
        {
            rc = BERR_TRACE((BHSM_STATUS_INPUT_PARM_ERR));
            goto BHSM_P_DONE_LABEL;
        }
    }

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pMiscBits->unStatus = status;
    if( status != 0 ) {
        BDBG_ERR(("BHSM_SetMiscBits FAILED [%d]", status  ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    if (pMiscBits->setMiscBitsSubCmd == BCMD_SetMiscBitsSubCmd_eXPTM2MStatusDump)
    {
        BHSM_BspMsg_Get8( hMsg, BCMD_SetMiscBitsField_OutCmdField_eM2MStatusBits, &status );
        pMiscBits->unM2MSecurityErrStatus = (uint32_t)status;

        BHSM_BspMsg_Get8( hMsg, BCMD_SetMiscBitsField_OutCmdField_eXPTStatusBits, &status );
        pMiscBits->unXPTSecurityErrStatus = (uint32_t)status;
    }

BHSM_P_DONE_LABEL:

    BDBG_LEAVE( BHSM_SetMiscBits );
    return rc;
}


BERR_Code BHSM_SetPciMaxWindowSize (
        BHSM_Handle hHsm,
        BHSM_PciMaxWindowConfig_t *pMaxConf )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t             status;

    BDBG_ENTER( BHSM_SetPciMaxWindowSize );

    if( ( hHsm == NULL ) || ( hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) || ( pMaxConf == NULL ) ) {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_ePCIE_Window_Size, &header );

    BHSM_BspMsg_PackArray( hMsg,
                           BCMD_PCIE_Window_SizeField_InCmdField_ePayloadStart,
                           pMaxConf->signedCommand,
                           BHSM_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE );

    BHSM_BspMsg_Pack8( hMsg, BCMD_PCIE_Window_SizeField_InCmdField_eVKLId,    BHSM_RemapVklId(pMaxConf->vklId ));
    BHSM_BspMsg_Pack8( hMsg, BCMD_PCIE_Window_SizeField_InCmdField_eKeyLayer, pMaxConf->keyLayer );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        (void)BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0  ) {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_WRN(("%s: BSP error status [0x%0X] returned", __FUNCTION__, status ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_SetPciMaxWindowSize );
    return rc;
#else
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pMaxConf );
    return BERR_TRACE( BERR_NOT_SUPPORTED );
#endif
}



BERR_Code BHSM_SetArch( BHSM_Handle hHsm, BHSM_SetArchIO_t *pSetArch )
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t             status;
    uint8_t             pciConf = 0;

    BDBG_ENTER( BHSM_SetArch );

    if( (hHsm == NULL) || (hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER) || (pSetArch == NULL) )
    {
        return  BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOFFLINE_ARCH_COMMAND, &header );

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack32( hMsg, BCMD_MISC_SetArchField_InCmdField_eAddrRangeLowMsb,  pSetArch->unLowerRangeAddressMsb );
   #endif
    BHSM_BspMsg_Pack32( hMsg, BCMD_MISC_SetArchField_InCmdField_eAddrRangeLow,     pSetArch->unLowerRangeAddress    );

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack32( hMsg, BCMD_MISC_SetArchField_InCmdField_eAddrRangeHighMsb, pSetArch->unUpperRangeAddressMsb );
   #endif
    BHSM_BspMsg_Pack32( hMsg, BCMD_MISC_SetArchField_InCmdField_eAddrRangeHigh,    pSetArch->unUpperRangeAddress    );

    BHSM_BspMsg_Pack8(  hMsg, BCMD_MISC_SetArchField_InCmdField_eArchSel,          pSetArch->ArchSel );

    if( ( pSetArch->ArchSel == BHSM_ArchSelect_ePciE0 ) || ( pSetArch->ArchSel == BHSM_ArchSelect_ePciE1 ) )
    {
        BDBG_CASSERT( BHSM_MAX_PCIE==2 ); /* assumption in following code (forced by BSP interface).*/

        if( (pSetArch->PciEConfig[0].activate) || (/*deprecated*/pSetArch->PCIArch == BCMD_PCIArchType_ePCIWin) ) pciConf |= (0x1 << 0);   /* bit 8 */
        if( (pSetArch->PciEConfig[1].activate) || (/*deprecated*/pSetArch->PCIArch == BCMD_PCIArchType_ePCIeWin) ) pciConf |= (0x1 << 1);   /* bit 9 */

        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
            if( pSetArch->pciEExcusive )
            {
                pciConf |= (0x1 << 2);                                            /* bit 10 -- exclusive mode is enabled */
                if( pSetArch->PciEConfig[0].enableWindow ) pciConf |= (0x1 << 3); /* bit 11 */
                if( pSetArch->PciEConfig[1].enableWindow ) pciConf |= (0x1 << 4); /* bit 12 */
            }
            else
            {
                BDBG_ERR(("%s Non-Exclusive mode for Zeus 4.2+ not implemented ", __FUNCTION__ ));
                goto BHSM_P_DONE_LABEL;
            }
        #endif
        BHSM_BspMsg_Pack8(  hMsg, BCMD_MISC_SetArchField_InCmdField_ePCIWin, pciConf );
    }

    BHSM_BspMsg_Pack8(  hMsg, BCMD_MISC_SetArchField_InCmdField_eDramSel, pSetArch->DRAMSel );

    if( ( rc = BHSM_BspMsg_SubmitCommand ( hMsg ) ) != BERR_SUCCESS ) {
        (void)BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );

    pSetArch->unStatus = status;

    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        switch( status )
        {
            case 0x20:
            {
                BDBG_ERR(("%s Arch config error. [0x%02X]", __FUNCTION__, status ));
                goto BHSM_P_DONE_LABEL;
            }
            case 0x21:
            {
                BDBG_ERR(("%s Arch size error. [0x%02X]", __FUNCTION__, status ));
                goto BHSM_P_DONE_LABEL;
            }
            case 0x23:
            {
                BDBG_ERR(("%s Arch already configured. [0x%02X]", __FUNCTION__, status ));
                goto BHSM_P_DONE_LABEL;
            }
            case 0x24:
            {
                BDBG_ERR(("%s Arch address error. [0x%02X]", __FUNCTION__, status ));
                goto BHSM_P_DONE_LABEL;
            }
            default:
            {
                BDBG_ERR(("%s BSP status error. [0x%02X]", __FUNCTION__, status ));
                goto BHSM_P_DONE_LABEL;
            }
        }
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE(BHSM_SetArch);
    return rc;
}



BERR_Code  BHSM_SetVichRegPar (
    BHSM_Handle             hHsm,
    BHSM_SetVichRegParIO_t *pSetVich
)
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t             i;
    uint8_t             status;

    BDBG_ENTER( BHSM_SetVichRegPar );

    if( (hHsm == NULL) || (hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER) || (pSetVich == NULL) )
    {
        return  BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if (pSetVich->nRanges < 1)
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOFFLINE_SET_VICH_REG_PAR, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_SetVichRegParField_InCmdField_eVDEC_Id,   pSetVich->VDECId );
    BHSM_BspMsg_Pack8( hMsg, BCMD_SetVichRegParField_InCmdField_eNumRanges, pSetVich->nRanges );

    for (i = 0; i < pSetVich->nRanges; i++)
    {
        BHSM_BspMsg_Pack32( hMsg, BCMD_SetVichRegParField_InCmdField_eRegPar0Start + (i * 8), pSetVich->unRangeLo[i] );
        BHSM_BspMsg_Pack32( hMsg, BCMD_SetVichRegParField_InCmdField_eRegPar0End + (i * 8),   pSetVich->unRangeHi[i] );
    }

    #define HSM_BCMD_SetVichRegParField_InCmdField_eSig         (BCMD_SetVichRegParField_InCmdField_eRegPar0Start + (pSetVich->nRanges* 8) )
    #define HSM_BCMD_VichRegParSignatureWordsLen                (8)
    #define HSM_BCMD_SetVichRegParVklInfoIdx                    (HSM_BCMD_SetVichRegParField_InCmdField_eSig + (HSM_BCMD_VichRegParSignatureWordsLen * 4))
    #define HSM_BCMD_SetVichRegParField_InCmdField_eKeyLayer    (HSM_BCMD_SetVichRegParVklInfoIdx + 2)
    #define HSM_BCMD_SetVichRegParField_InCmdField_eVKL         (HSM_BCMD_SetVichRegParVklInfoIdx + 3)

    BHSM_BspMsg_PackArray( hMsg, HSM_BCMD_SetVichRegParField_InCmdField_eSig,   pSetVich->aucSignature, sizeof(pSetVich->aucSignature) );
    BHSM_BspMsg_Pack8( hMsg, HSM_BCMD_SetVichRegParField_InCmdField_eKeyLayer,  pSetVich->keyLayer );
    BHSM_BspMsg_Pack8( hMsg, HSM_BCMD_SetVichRegParField_InCmdField_eVKL,       BHSM_RemapVklId( pSetVich->virtualKeyLadderID ));


    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        (void)BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
	pSetVich->unStatus = status;

    switch( status )
    {
        case 0:
        {
            break; /* success */
        }
        case 0x22:
        {
            BDBG_WRN(("%s: VICH#%d Already Configured.", __FUNCTION__, pSetVich->VDECId));
			/* Pass on success to the client. */
			pSetVich->unStatus = 0;
            break; /* success */
		}
	    default:
        {
            rc = BHSM_STATUS_BSP_ERROR;
            BDBG_ERR(("%s BSP status error. [0x%02X]", __FUNCTION__, status ));
            break;
        }
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_SetVichRegPar );
    return rc;
}


BERR_Code  BHSM_StartAVD(
    BHSM_Handle        hHsm,
    BHSM_StartAVDIO_t  *pStartAvd )
{
    BERR_Code           rc = BERR_SUCCESS;
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t             status;
    unsigned            i;

    BDBG_ENTER( BHSM_StartAVD );

    if( (hHsm == NULL) || (hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER) || (pStartAvd == NULL) )
    {
        return  BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( pStartAvd->numAVDReg < 1 )
    {
        return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eSTART_AVD, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_MISC_START_AVD_InCmdField_eAVDId,    pStartAvd->avdID );
    BHSM_BspMsg_Pack8( hMsg, BCMD_MISC_START_AVD_InCmdField_eNumofReg, pStartAvd->numAVDReg );

    for (i = 0; i < pStartAvd->numAVDReg; i++)
    {
        BHSM_BspMsg_Pack32( hMsg, BCMD_MISC_START_AVD_InCmdField_eAddr1 + (i * 8),  pStartAvd->avdRegAddr[i] );
        BHSM_BspMsg_Pack32( hMsg, BCMD_MISC_START_AVD_InCmdField_eValue1 + (i * 8), pStartAvd->avdRegVal[i]  );
    }

    #define HSM_BCMD_MISC_START_AVD_InCmdField_eSig             ((pStartAvd->numAVDReg * 8)+BCMD_MISC_START_AVD_InCmdField_eAddr1)
    #define HSM_BCMD_StartAvdSignatureWordsLen                  (8)
    #define HSM_BCMD_StartAvdVklInfoIdx                         (HSM_BCMD_MISC_START_AVD_InCmdField_eSig + (HSM_BCMD_StartAvdSignatureWordsLen * 4))
    #define HSM_BCMD_MISC_START_AVD_InCmdField_eKeyLayer        (HSM_BCMD_StartAvdVklInfoIdx + 2)
    #define HSM_BCMD_MISC_START_AVD_InCmdField_eVKL             (HSM_BCMD_StartAvdVklInfoIdx + 3)

    BHSM_BspMsg_PackArray( hMsg, HSM_BCMD_MISC_START_AVD_InCmdField_eSig,      pStartAvd->aucSignature, sizeof(pStartAvd->aucSignature) );
    BHSM_BspMsg_Pack8( hMsg,     HSM_BCMD_MISC_START_AVD_InCmdField_eVKL,      BHSM_RemapVklId( pStartAvd->virtualKeyLadderID ));
    BHSM_BspMsg_Pack8( hMsg,     HSM_BCMD_MISC_START_AVD_InCmdField_eKeyLayer, pStartAvd->keyLayer );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        (void)BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pStartAvd->unStatus = status;
    if( status != 0 )
    {
        rc = BHSM_STATUS_BSP_ERROR;
        BDBG_ERR(("%s BSP status error. [0x%02X]", __FUNCTION__, status ));
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_StartAVD );
    return rc;
}
