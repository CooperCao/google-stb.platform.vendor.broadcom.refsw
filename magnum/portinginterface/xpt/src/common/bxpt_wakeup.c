/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Porting interface code for the data transport core.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt_wakeup.h"
#include "bkni.h"
#include "bchp_xpt_wakeup.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if( BDBG_DEBUG_BUILD == 1 )
    BDBG_MODULE( xpt_wakeup );
#endif

void BXPT_Wakeup_GetDefaults(
    BXPT_Wakeup_Settings *Settings
    )
{
    BDBG_ASSERT( Settings );
    BKNI_Memset( (void *)Settings, 0, sizeof(BXPT_Wakeup_Settings));
    Settings->PacketLength = 188;
}

#if (!B_REFSW_MINIMAL)
void BXPT_Wakeup_GetSettings(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    BXPT_Wakeup_Settings *Settings
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Settings );

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_WAKEUP_CTRL );
    Settings->InputBand = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_CTRL, INPUT_SEL );
    Settings->InputBand |= (BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_CTRL, INPUT_SEL_MSB ) << 4 );
    Settings->PacketLength = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_CTRL, PKT_LENGTH );
    Settings->ErrorInputIgnore = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_CTRL, ERROR_INPUT_IGNORE );
}
#endif

BERR_Code BXPT_Wakeup_SetSettings(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    const BXPT_Wakeup_Settings *Settings
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Settings );

    if( Settings->PacketLength > BXPT_WAKEUP_PACKET_SIZE )
    {
        BDBG_ERR(( "PacketLength %u is out-of-bounds (max is %u).", Settings->PacketLength, BXPT_WAKEUP_PACKET_SIZE ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else if ( !BXPT_P_InputBandIsSupported( Settings->InputBand ) )
    {
        BDBG_ERR(( "InputBand %u is not supported on this chip.", Settings->InputBand ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_WAKEUP_CTRL );
        Reg &= ~(
            BCHP_MASK( XPT_WAKEUP_CTRL, PKT_LENGTH ) |
            BCHP_MASK( XPT_WAKEUP_CTRL, ERROR_INPUT_IGNORE ) |
            BCHP_MASK( XPT_WAKEUP_CTRL, INPUT_SEL_MSB ) |
            BCHP_MASK( XPT_WAKEUP_CTRL, INPUT_SEL )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_WAKEUP_CTRL, PKT_LENGTH, Settings->PacketLength ) |
            BCHP_FIELD_DATA( XPT_WAKEUP_CTRL, ERROR_INPUT_IGNORE, Settings->ErrorInputIgnore ) |
            BCHP_FIELD_DATA( XPT_WAKEUP_CTRL, INPUT_SEL_MSB, (Settings->InputBand >> 4) & 0x1 ) |
            BCHP_FIELD_DATA( XPT_WAKEUP_CTRL, INPUT_SEL, Settings->InputBand & 0xF)
        );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_WAKEUP_CTRL, Reg );
    }

    return ExitCode;
}

#if (!B_REFSW_MINIMAL)
void BXPT_Wakeup_GetStatus(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    BXPT_Wakeup_Status *Status
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Status );

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_WAKEUP_STATUS );
    Status->Found = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_STATUS, PKT_FOUND );
    Status->Type = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_STATUS, PKT_FOUND_TYPE );
    Status->Count = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_STATUS, PACKET_COUNT );
}

void BXPT_Wakeup_ClearInterruptToPMU(
    BXPT_Handle hXpt
    )
{
    BKNI_EnterCriticalSection();
    BXPT_Wakeup_ClearInterruptToPMU_isr(hXpt);
    BKNI_LeaveCriticalSection();
}
#endif

void BXPT_Wakeup_ClearInterruptToPMU_isr(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hXpt );

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_WAKEUP_STATUS );
    Reg &= ~(
        BCHP_MASK( XPT_WAKEUP_STATUS, PKT_FOUND )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_WAKEUP_STATUS, Reg );
}

BERR_Code BXPT_Wakeup_SetPacketFilterBytes(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned WhichPacketType,
    const BXPT_Wakeup_PacketFilter *Filter
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Filter );

    if( WhichPacketType > BXPT_WAKEUP_MAX_PACKET_TYPE )
    {
        BDBG_ERR(( "WhichPacketType %u is out-of-bounds (max is %u).", WhichPacketType, BXPT_WAKEUP_MAX_PACKET_TYPE ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        uint32_t Reg;
        unsigned Index;

        uint32_t ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE0_i_ARRAY_BASE;

        switch( WhichPacketType )
        {
            default:
            case 0: ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE0_i_ARRAY_BASE; break;
            case 1: ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE1_i_ARRAY_BASE; break;
            case 2: ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE2_i_ARRAY_BASE; break;
            case 3: ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE; break;
        }

        for( Index = 0; Index < BXPT_WAKEUP_PACKET_SIZE; Index++ )
        {
            Reg = BREG_Read32( hXpt->hRegister, ArrayBase + (Index * 4) );
            Reg &= ~(
                BCHP_MASK( XPT_WAKEUP_PKT_TYPE0_i, COMPARE_BYTE ) |
                BCHP_MASK( XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK_TYPE ) |
                BCHP_MASK( XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_WAKEUP_PKT_TYPE0_i, COMPARE_BYTE, Filter[ Index ].CompareByte ) |
                BCHP_FIELD_DATA( XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK_TYPE, Filter[ Index ].MaskType ) |
                BCHP_FIELD_DATA( XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK, Filter[ Index ].Mask )
            );
            BREG_Write32( hXpt->hRegister, ArrayBase + (Index * 4), Reg );
        }
    }

    return ExitCode;
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Wakeup_GetPacketFilterBytes(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned WhichPacketType,
    BXPT_Wakeup_PacketFilter *Filter
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Filter );

    if( WhichPacketType > BXPT_WAKEUP_MAX_PACKET_TYPE )
    {
        BDBG_ERR(( "WhichPacketType %u is out-of-bounds (max is %u).", WhichPacketType, BXPT_WAKEUP_MAX_PACKET_TYPE ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        uint32_t Reg;
        unsigned Index;

        uint32_t ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE0_i_ARRAY_BASE;

        switch( WhichPacketType )
        {
            default:
            case 0: ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE0_i_ARRAY_BASE; break;
            case 1: ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE1_i_ARRAY_BASE; break;
            case 2: ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE2_i_ARRAY_BASE; break;
            case 3: ArrayBase = BCHP_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE; break;
        }

        for( Index = 0; Index < BXPT_WAKEUP_PACKET_SIZE; Index++ )
        {
            Reg = BREG_Read32( hXpt->hRegister, ArrayBase + (Index * 4) );
            Filter[ Index ].CompareByte = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_PKT_TYPE0_i, COMPARE_BYTE );
            Filter[ Index ].MaskType = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK_TYPE );
            Filter[ Index ].Mask = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK );
        }
    }

    return ExitCode;
}

void BXPT_Wakeup_GetCapturedPacket(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned char *PacketBuffer
    )
{
    uint32_t Reg;
    unsigned Index;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( PacketBuffer );

    for( Index = 0; Index < BXPT_WAKEUP_PACKET_SIZE; Index++ )
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_WAKEUP_PKT_MEM_i_ARRAY_BASE + (Index * 4) );
        PacketBuffer[ Index ] = BCHP_GET_FIELD_DATA( Reg, XPT_WAKEUP_PKT_MEM_i, PKT_BYTE );
    }
}
#endif

void BXPT_Wakeup_Armed(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    bool Armed
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hXpt );

#ifdef BCHP_PWR_RESOURCE_XPT
    if( Armed && !hXpt->WakeupArmed )
    {
        BCHP_PWR_AcquireResource( hXpt->hChip, BCHP_PWR_RESOURCE_XPT_XMEMIF );
        hXpt->WakeupArmed = true;
    }
#endif

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_WAKEUP_CTRL );
    Reg &= ~(
        BCHP_MASK( XPT_WAKEUP_CTRL, PKT_DETECT_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_WAKEUP_CTRL, PKT_DETECT_EN, Armed ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_WAKEUP_CTRL, Reg );

#ifdef BCHP_PWR_RESOURCE_XPT
    if( !Armed && hXpt->WakeupArmed )
    {
        BCHP_PWR_ReleaseResource( hXpt->hChip, BCHP_PWR_RESOURCE_XPT_XMEMIF );
        hXpt->WakeupArmed = false;
    }
#endif
}

