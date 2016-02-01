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
#include "bkni.h"
#include "bxpt_priv.h"						   
#include "bxpt_directv_pcr.h"
#include "bchp_xpt_dpcr0.h"

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_directv_pcr );
#endif

BERR_Code BXPT_PCR_DirecTv_SetPcrMode( 
	BXPT_PCR_Handle hPcr, 			   /* [In]The Pcr handle */
	BXPT_PcrMode Mode
	)
{
	BREG_Handle			hRegister = hPcr->hRegister;
	uint32_t            Reg;
	BERR_Code		    ret = BERR_SUCCESS;
	uint32_t            Offset = hPcr->RegOffset;

	Reg = BREG_Read32( hRegister, BCHP_XPT_DPCR0_CTRL + Offset );
	Reg &= ~( BCHP_MASK( XPT_DPCR0_CTRL, PCR_PACKET_MODE ) );
	Reg |= BCHP_FIELD_DATA( XPT_DPCR0_CTRL, PCR_PACKET_MODE, Mode == BXPT_PcrMode_eDirecTv ? 1 : 0 );
	BREG_Write32( hRegister, BCHP_XPT_DPCR0_CTRL + Offset, Reg );

	hPcr->DMode = Mode == BXPT_PcrMode_eDirecTv ? true : false;
		
	return ret;
}

#if (!B_REFSW_MINIMAL)
BERR_Code  BXPT_PCR_DirecTv_GetLastPcr(
    BXPT_PCR_Handle hPcr,
    uint32_t *pPcr
    )
{
    BERR_Code ret = BERR_SUCCESS;

    BKNI_EnterCriticalSection();
    ret = BXPT_PCR_DirecTv_GetLastPcr_isr(hPcr, pPcr);
    BKNI_LeaveCriticalSection();
    return ret;
}
#endif

BERR_Code	BXPT_PCR_DirecTv_GetLastPcr_isr(
	BXPT_PCR_Handle hPcr, 			   /* [in]The Pcr handle */
	uint32_t *pPcr      			  /*[out] 32 bits of RTS*/
	)
{
	BREG_Handle			hRegister = hPcr->hRegister;
	uint32_t            RegHi, RegLo;
	BERR_Code		    ret = BERR_SUCCESS;
	uint32_t            Offset = hPcr->RegOffset;

	BDBG_ENTER( BXPT_PCR_DirecTv_GetLastPcr );

	RegHi = BREG_Read32( hRegister, BCHP_XPT_DPCR0_LAST_PCR_HI + Offset );
   	RegLo = BREG_Read32( hRegister, BCHP_XPT_DPCR0_LAST_PCR_LO + Offset );
	*pPcr = ( RegHi << 10 ) | RegLo;

	BDBG_LEAVE( BXPT_PCR_DirecTv_GetLastPcr );

	return ret;

}

#if (!B_REFSW_MINIMAL)
BERR_Code	BXPT_PCR_DirecTv_GetStc( 
	BXPT_PCR_Handle hPcr,          /* [in]The Pcr handle */
	uint32_t *pStcHi               /*[out] 32 bits of RTS*/
	)
{
	BREG_Handle			hRegister = hPcr->hRegister;
	uint32_t            RegHi, RegLo;
	BERR_Code		    ret = BERR_SUCCESS;
	uint32_t            Offset = hPcr->RegOffset;

	BDBG_ENTER( BXPT_PCR_DirecTv_GetStc );

	RegHi = BREG_Read32( hRegister, BCHP_XPT_DPCR0_STC_HI + Offset );
	RegLo = BREG_Read32( hRegister, BCHP_XPT_DPCR0_STC_LO + Offset );
	*pStcHi = ( RegHi << 10 ) | RegLo;

	BDBG_LEAVE( BXPT_PCR_DirecTv_GetStc );

	return ret;
}
#endif

