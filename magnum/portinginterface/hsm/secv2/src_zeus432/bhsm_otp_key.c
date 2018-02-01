/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bhsm_priv.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_otp_common.h"
#include "bsp_s_otp.h"
#include "bsp_s_keycommon.h"
#include "bhsm_otp_key.h"
#include "bhsm_otp_priv.h"
#include "bhsm_keyladder.h"

BDBG_MODULE( BHSM );

typedef struct BHSM_OtpKeyModule
{
    BHSM_OtpKeyInfo KeyInfo[BPI_Otp_KeyType_eSize];
    bool cached[BPI_Otp_KeyType_eSize];
}BHSM_OtpKeyModule;

static BERR_Code _ReadUint32( BHSM_Handle hHsm, unsigned index, BPI_Otp_CmdReadRegister_e field, uint32_t *pData );
static BERR_Code _ReadArray( BHSM_Handle hHsm,
                             unsigned index,
                             BPI_Otp_CmdReadRegister_e field,
                             unsigned size,
                             uint8_t *pData );


BERR_Code BHSM_OtpKey_Init( BHSM_Handle hHsm, BHSM_OtpKeyModuleSettings *pSettings )
{

    BDBG_ENTER( BHSM_OtpKey_Init );
    BSTD_UNUSED( pSettings );

    hHsm->modules.pOtpKey = BKNI_Malloc( sizeof(BHSM_OtpKeyModule) );
    if( !hHsm->modules.pOtpKey ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( hHsm->modules.pOtpKey, 0, sizeof(BHSM_OtpKeyModule) );



    BDBG_LEAVE( BHSM_OtpKey_Init );
    return BERR_SUCCESS;
}


void BHSM_OtpKey_Uninit( BHSM_Handle hHsm )
{

    BDBG_ENTER( BHSM_OtpKey_Uninit );

    if( !hHsm->modules.pOtpKey ) {
        BERR_TRACE( BERR_NOT_INITIALIZED );
        return;
    }

    BKNI_Memset( hHsm->modules.pOtpKey, 0, sizeof(BHSM_OtpKeyModule) );
    BKNI_Free( hHsm->modules.pOtpKey );
    hHsm->modules.pOtpKey = NULL;

    BDBG_LEAVE( BHSM_OtpKey_Uninit );
    return;
}

BERR_Code BHSM_OtpKey_GetInfo( BHSM_Handle hHsm, BHSM_OtpKeyInfo *pKeyInfo )
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t  otpValue;
    unsigned index = 0;

    BDBG_ENTER( BHSM_OtpKey_GetInfo );

    if( !pKeyInfo )                      { return BERR_TRACE( BHSM_STATUS_FAILED ); }
    if( pKeyInfo->index >= BPI_Otp_KeyType_eSize ) { return BERR_TRACE( BHSM_STATUS_FAILED ); }

    index = pKeyInfo->index;
    BKNI_Memset( pKeyInfo, 0, sizeof(*pKeyInfo) );

    if(hHsm->modules.pOtpKey->cached[index] == false)
    {
        hHsm->modules.pOtpKey->cached[index] = true;

        rc = _ReadArray( hHsm, index, BPI_Otp_CmdReadRegister_eKeyHash, 8, pKeyInfo->hash );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }

        rc = _ReadArray( hHsm, index, BPI_Otp_CmdReadRegister_eKeyID, 8, pKeyInfo->id );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }

        rc = _ReadUint32( hHsm, index, BPI_Otp_CmdReadRegister_eKeyMc0_BlackBoxId, ( uint32_t* )&pKeyInfo->blackBoxId );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }

        rc = _ReadUint32( hHsm, index, BPI_Otp_CmdReadRegister_eKeyMc0_CaKeyLadderDisallow, &otpValue );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !otpValue ) pKeyInfo->caKeyLadderAllow = true;

        rc = _ReadUint32( hHsm, index, BPI_Otp_CmdReadRegister_eKeyMc0_CpKeyLadderDisallow, &otpValue );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !otpValue ) pKeyInfo->cpKeyLadderAllow = true;

        rc = _ReadUint32( hHsm, index, BPI_Otp_CmdReadRegister_eKeyMc0_Gp1KeyLadderDisallow, &otpValue );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !otpValue ) pKeyInfo->gp1KeyLadderAllow = true;

        rc = _ReadUint32( hHsm, index, BPI_Otp_CmdReadRegister_eKeyMc0_Gp2KeyLadderDisallow, &otpValue );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !otpValue ) pKeyInfo->gp2KeyLadderAllow = true;

        rc = _ReadUint32( hHsm, index, BPI_Otp_CmdReadRegister_eKeyMc0_SageKeyLadderDisallow, &otpValue );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !otpValue ) pKeyInfo->sageKeyLadderAllow = true;

        rc = _ReadUint32( hHsm, index, BPI_Otp_CmdReadRegister_eKeyMc0_RootKeySwapDisallow, &otpValue );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( !otpValue ) pKeyInfo->rootKeySwapAllow = true;

        rc = _ReadUint32( hHsm, index, BPI_Otp_CmdReadRegister_eKeyMc0_DeobfuscationEnable, &otpValue );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }
        if( otpValue ) pKeyInfo->deobfuscationEnabled = true;

        rc = _ReadUint32( hHsm, index, BPI_Otp_CmdReadRegister_eKeyMc0_CustomerMode, ( uint32_t* )&pKeyInfo->customerMode );
        if( rc != BERR_SUCCESS ) { goto BHSM_P_DONE_LABEL; }

        BKNI_Memcpy(&hHsm->modules.pOtpKey->KeyInfo[index],pKeyInfo, sizeof(*pKeyInfo));
    }
    else
    {
        BKNI_Memcpy(pKeyInfo, &hHsm->modules.pOtpKey->KeyInfo[index], sizeof(*pKeyInfo));
    }

BHSM_P_DONE_LABEL:

    if( rc != BERR_SUCCESS )
    {
        hHsm->modules.pOtpKey->cached[index] = false;
        rc = BERR_TRACE( BHSM_STATUS_FAILED );
    }

    BDBG_LEAVE( BHSM_OtpKey_GetInfo );
    return rc;
}



BERR_Code BHSM_OtpKey_Program(
    BHSM_Handle hHsm,
    const BHSM_OtpKeyProgram *pKeyConfig )
{
    BERR_Code     rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    uint8_t       status;
    BHSM_BspMsgHeader_t header;
    BHSM_KeyLadderInfo keyLadderInfo;

    BDBG_ENTER( BHSM_OtpKey_Program );

    if( !pKeyConfig )                                        { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( pKeyConfig->keyladder.level < BCMD_KeyRamBuf_eKey3 ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( pKeyConfig->keyladder.level >= BCMD_KeyRamBuf_eMax ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( pKeyConfig->index >= BPI_Otp_KeyType_eSize )         { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    /* TODO: check the encrypted key data length is 36 bytes. Currently it is 16 in the API? */
    rc = BHSM_GetKeyLadderInfo( pKeyConfig->keyladder.handle, &keyLadderInfo );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    rc = BHSM_BspMsg_Create( hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOTP_KEY_FIELD_PROG, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdKeyFieldProg_eKeyDest, pKeyConfig->index );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdKeyFieldProg_eVKLId, keyLadderInfo.index );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdKeyFieldProg_eKeyLayer, pKeyConfig->keyladder.level );

    BHSM_BspMsg_PackArray( hMsg, BCMD_Otp_InCmdKeyFieldProg_eOTPKeyData
                               , pKeyConfig->keyData
                               , BHSM_OTP_KEY_PROGRAM_LENGTH );

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    hHsm->modules.pOtpKey->cached[pKeyConfig->index] = false;

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BERR_TRACE( BHSM_STATUS_FAILED );
        BDBG_ERR( ( "%s BSP status[0x%02X] error", BSTD_FUNCTION, status ) );
        goto BHSM_P_DONE_LABEL;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_OtpKey_Program );

    return rc;
}

static BERR_Code _ReadUint32( BHSM_Handle hHsm,
                                          unsigned index,
                                          BPI_Otp_CmdReadRegister_e field,
                                          uint32_t *pData )
{
    BERR_Code     rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    uint8_t       status;
    BHSM_BspMsgHeader_t header;

    BDBG_ENTER( _ReadUint32 );

    if( !pData )                            { return BERR_TRACE( BHSM_STATUS_FAILED ); }
    if( index >= BPI_Otp_KeyType_eSize )    { return BERR_TRACE( BHSM_STATUS_FAILED ); }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOFFLINE_OTP_READ, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineOtpRead_eEnum, field );
    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineOtpRead_eKeyType, index );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR( ( "%s status[0x%02X]", BSTD_FUNCTION, status ) );
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get32( hMsg, BCMD_Otp_OutCmdOfflineOtpRead_eRegValueLo, pData );

  BHSM_P_DONE_LABEL:

    ( void ) BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _ReadUint32 );
    return rc;
}



static BERR_Code _ReadArray( BHSM_Handle hHsm,
                             unsigned index,
                             BPI_Otp_CmdReadRegister_e field,
                             unsigned size, /*must be 8 for now!*/
                             uint8_t *pData )
{
    BERR_Code     rc = BERR_SUCCESS;
    BHSM_BspMsg_h hMsg = NULL;
    uint8_t       status;
    BHSM_BspMsgHeader_t header;

    BDBG_ENTER( _ReadArray );

    if( index >= BPI_Otp_KeyType_eSize ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( !pData )                         { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( size != 8 )                      { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOFFLINE_OTP_READ, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineOtpRead_eEnum, field );
    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineOtpRead_eKeyType, index );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR( ( "%s status[0x%02X]", BSTD_FUNCTION, status ) );
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetArray( hMsg, BCMD_Otp_OutCmdOfflineOtpRead_eRegValueLo, &pData[4], 4 );
    BHSM_BspMsg_GetArray( hMsg, BCMD_Otp_OutCmdOfflineOtpRead_eRegValueHi, &pData[0], 4 );

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( _ReadArray );
    return rc;
}
