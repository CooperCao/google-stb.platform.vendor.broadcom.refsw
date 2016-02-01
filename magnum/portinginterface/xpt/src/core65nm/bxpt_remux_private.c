/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
 * Implemtation of the remux module private functions.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


#include "bstd.h"
#include "bxpt_remux_private.h"
#include "bdbg.h"
#include "bkni.h"
#include "bxpt_priv.h"

#if ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) \
    || ( BCHP_CHIP == 7635 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 7630 )
    #include "bchp_xpt_rmx0_io.h"
    #include "bchp_xpt_rmx1_io.h"
#endif

#include "bchp_xpt_rmx0.h"

static uint32_t ComputeRegAddress(
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	uint32_t Reg0Addr
    )
{
    uint32_t RegAddr;

#if ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) \
    || ( BCHP_CHIP == 7635 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 7630 )
    switch( Reg0Addr )
    {
        case BCHP_XPT_RMX0_IO_FORMAT:
        if( hRmx->ChannelNo )
            RegAddr = BCHP_XPT_RMX1_IO_FORMAT;
        else
            RegAddr = BCHP_XPT_RMX0_IO_FORMAT;
        break;

        case BCHP_XPT_RMX0_IO_PKT_DLY_CNT:
        if( hRmx->ChannelNo )
            RegAddr = BCHP_XPT_RMX1_IO_PKT_DLY_CNT;
        else
            RegAddr = BCHP_XPT_RMX0_IO_PKT_DLY_CNT;
        break;

        default:
        RegAddr = Reg0Addr - BCHP_XPT_RMX0_CTRL + hRmx->BaseAddr;
        break;
    }
#else
    /* 
    ** The address is the offset of the register from the beginning of the 
    ** block, plus the base address of the block ( which changes from 
    ** channel to channel ). 
    */
    RegAddr = Reg0Addr - BCHP_XPT_RMX0_CTRL + hRmx->BaseAddr;
#endif
    
    return RegAddr;
}

void BXPT_Remux_P_WriteReg(
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	uint32_t Reg0Addr,
	uint32_t RegVal
	)
{
	BREG_Write32( hRmx->hRegister, ComputeRegAddress( hRmx, Reg0Addr ), RegVal );
}		

uint32_t BXPT_Remux_P_ReadReg(
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	uint32_t Reg0Addr
	)
{
	return( BREG_Read32( hRmx->hRegister, ComputeRegAddress( hRmx, Reg0Addr ) ));
}


