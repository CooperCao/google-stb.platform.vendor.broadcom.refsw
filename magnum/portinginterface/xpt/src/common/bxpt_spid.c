/******************************************************************************
 * (c) 2003-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/


#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt_spid.h"
#include "bchp_xpt_fe.h"

#include "bxpt.h"

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_spid );
#endif

BERR_Code BXPT_Spid_ConfigureChannel(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int ChannelNum,    /* [in] Which secondary channel to configure. */
    unsigned int Spid,              /* [in] Secondary PID to use. */
    BXPT_Spid_eChannelMode Mode     /* [in] Mode for secondary PIDs on this channel. */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( Spid >= 0x2000 )
    {
        /* Bad PID. Complain. */
        BDBG_ERR(( "Spid %lu is out of range!", ( unsigned long ) Spid ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else if( ChannelNum >= hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) ChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * ChannelNum );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        Reg &= ~(
            BCHP_MASK( XPT_FE_SPID_TABLE_i, SPID_MODE ) |
            BCHP_MASK( XPT_FE_SPID_TABLE_i, PID_FUNCTIONS_SPID_CHANNEL_PID )
            );

        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, SPID_MODE, Mode ) |
            BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, PID_FUNCTIONS_SPID_CHANNEL_PID, Spid )
            );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
    }

    return( ExitCode );
}


BERR_Code BXPT_Spid_P_ConfigureChannelFilter(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int ChannelNum,        /* [in] Which secondary channel to configure. */
    BXPT_Spid_eChannelFilter Filter /* [in] Set a filter to use with this channel */
    )
{
    uint32_t Reg, RegAddr,i;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    if( ChannelNum >= hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) ChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * ChannelNum );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        i = BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, SPID_MODE );

        /* throw an error if SPID channel is being used for SPID functions */
        if(i > BXPT_Spid_eChannelMode_Disable && i <= BXPT_Spid_eChannelMode_Remap)
        {
            BDBG_ERR(("PidChannelNum %lu already in use for SPID functions!",( unsigned long ) ChannelNum ));
            ExitCode = BERR_TRACE( BERR_UNKNOWN );
        }
        Reg &= ~(
            BCHP_MASK( XPT_FE_SPID_TABLE_i, SPID_MODE ) |
            BCHP_MASK( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_HI ) |
            BCHP_MASK( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_LO )
            );

        Reg |= BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, SPID_MODE, Filter.Mode );

        switch(Filter.Mode)
        {
        case BXPT_Spid_eChannelFilterMode_StreamId:
            Reg |= BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_FILTER_STREAM_ID, Filter.FilterConfig.StreamId );
            break;
        case BXPT_Spid_eChannelFilterMode_StreamIdRange:
            Reg |= ( BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_HI, Filter.FilterConfig.StreamIdRange.Hi ) |
                     BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_LO, Filter.FilterConfig.StreamIdRange.Lo )
                     );
            break;
        case BXPT_Spid_eChannelFilterMode_StreamIdExtension:
            Reg |= ( BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_EXTENSION_FILTER_STREAM_ID, Filter.FilterConfig.StreamIdAndExtension.Id ) |
                     BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_EXTENSION_FILTER_STREAM_ID_EXTENSION, Filter.FilterConfig.StreamIdAndExtension.Extension )
                     );
            break;
        case BXPT_Spid_eChannelFilterMode_SubStreamId:
            Reg |= ( BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, SUBSTREAM_ID_FILTER_STREAM_ID, Filter.FilterConfig.StreamIdAndSubStreamId.Id ) |
                     BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, SUBSTREAM_ID_FILTER_SUBSTREAM_ID, Filter.FilterConfig.StreamIdAndSubStreamId.SubStreamId )
                     );
            break;
        case BXPT_Spid_eChannelFilterMode_Disable:
            break;
        default:
            BDBG_ERR(("Invalid filter mode %u",(unsigned) Filter.Mode));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        }
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
    }

    return( ExitCode );
}
