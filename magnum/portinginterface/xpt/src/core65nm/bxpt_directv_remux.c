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
#include "bxpt_directv_remux.h"	 
#include "bxpt_remux_private.h"
#include "bchp_xpt_rmx0.h"

BDBG_MODULE( xpt_directv_remux );

BERR_Code BXPT_DirecTvRemux_SetMode( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	BXPT_RemuxMode Mode			/* [in] Selects the mode. */
	)
{
 	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

#if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || \
    ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || ( BCHP_CHIP == 7340 ) || \
	( BCHP_CHIP == 7630 )
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
#else
	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_FORMAT );
	Reg &= ~(
		BCHP_MASK( XPT_RMX0_FORMAT, RMX_PKT_LENGTH ) |
		BCHP_MASK( XPT_RMX0_FORMAT, RMX_PKT_MODE )
	);

	if( Mode == BXPT_RemuxMode_eDirecTv )
	{
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_PKT_LENGTH, 130 ) |
			BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_PKT_MODE, 1 )
		);
	}
	else
	{
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_PKT_LENGTH, 188 ) |
			BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_PKT_MODE, 0 )
		);
	}

	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_FORMAT, Reg );
#endif
	return( ExitCode );	    
}

#if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || \
    ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || ( BCHP_CHIP == 7340 ) || \
	( BCHP_CHIP == 7630 )
    /* These chips don't have packet sub in the remux. The dedicated PSUB module is used instead */
#else
BERR_Code BXPT_Remux_PsubMatchHdField( 
	BXPT_Remux_Handle hRmx,	/* [in] Handle for the remux channel */
	int WhichTable,		/* [in] The remux packet sub to set. */
	bool MatchHd, 		  /* [in] Enable or disable HD match requirement */
	uint8_t HdCompValue	  /* [in] Value HD field must match. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( WhichTable >= BXPT_P_MAX_REMUX_PSUB_TABLES )
	{
		BDBG_ERR(( "WhichTable %lu is out of range!", ( unsigned long ) WhichTable ));
		ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	else
	{
		uint32_t Reg;
		
		uint32_t CtrlRegAddr = 0;

		switch( WhichTable )
		{
			case 0:
			CtrlRegAddr = BCHP_XPT_RMX0_PSUB1_CTRL;
			break;

			case 1:
			CtrlRegAddr = BCHP_XPT_RMX0_PSUB2_CTRL;
			break;
		}

		Reg = BXPT_Remux_P_ReadReg( hRmx, CtrlRegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_HD_EN ) |
			BCHP_MASK( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_HD )
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_HD_EN, MatchHd == true ? 1 : 0 ) |
			BCHP_FIELD_DATA( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_HD, HdCompValue )
		);
		BXPT_Remux_P_WriteReg( hRmx, CtrlRegAddr, Reg );
	}

	return( ExitCode );	    
}
#endif
