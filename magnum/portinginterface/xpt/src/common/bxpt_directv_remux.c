/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
#include "bxpt_directv_remux.h"	 
#include "bxpt_remux_priv.h"
#include "bchp_xpt_rmx0.h"

#if( BDBG_DEBUG_BUILD == 1 )
	BDBG_MODULE( xpt_directv_remux );
#endif

BERR_Code BXPT_DirecTvRemux_SetMode( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	BXPT_RemuxMode Mode			/* [in] Selects the mode. */
	)
{
 	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_ASSERT( hRmx );

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );
	Reg &= ~(
		BCHP_MASK( XPT_RMX0_CTRL, RMX_PKT_LENGTH ) |
		BCHP_MASK( XPT_RMX0_CTRL, RMX_PKT_MODE )
	);

	if( Mode == BXPT_RemuxMode_eDirecTv )
	{
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PKT_LENGTH, 130 ) |
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PKT_MODE, 1 )
		);
	}
	else
	{
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PKT_LENGTH, 188 ) |
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PKT_MODE, 0 )
		);
	}

	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );

	return( ExitCode );	    
}


