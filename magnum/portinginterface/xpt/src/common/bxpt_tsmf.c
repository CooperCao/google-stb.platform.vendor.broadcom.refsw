/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 *  Users of this technology  are responsible for complying to all laws of 
 *  Japan Ministry of Internal Affairs and Communication Requirements and 
 *  securing any applicable license requirements of the JCTEA.  BRCM conveys 
 *  no license to the TSMF technology or right to use.
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h" 
#include "bxpt_tsmf.h"

/*
** NOTE: This software requires approval from BRCM marketing before it can be 
** released to customers. 
*/


BDBG_MODULE(xpt_tsmf);

BERR_Code BXPT_TSMF_GetFldVerifyConfig( 
	BXPT_Handle hXpt, 	   		/* [In] Handle for the transport */
	int TSMFNum,			/* [In] Which TSMF. */
	BXPT_TSMFFldVerifyConfig *TsmfConfig	/* [In] Which TSMF configuraion. */ 
 	)
{
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( TSMFNum );
    BSTD_UNUSED( TsmfConfig );
    BDBG_ERR(( "TSMF not supported in this build" ));
    return BERR_NOT_SUPPORTED;
}

BERR_Code BXPT_TSMF_SetFldVerifyConfig( 
	BXPT_Handle hXpt, 	   		
	int TSMFNum,			
	const BXPT_TSMFFldVerifyConfig *TsmfConfig  
	)
{
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( TSMFNum );
    BSTD_UNUSED( TsmfConfig );
    BDBG_ERR(( "TSMF not supported in this build" ));
    return BERR_NOT_SUPPORTED;
}

BERR_Code BXPT_TSMF_EnableAutoMode( 
	BXPT_Handle hXpt, 	   		
	BXPT_TSMF_InputSel	InputSel,  
	int TSMFNum,			
	uint8_t RelativeTsNo     
 	)
{
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( InputSel );
    BSTD_UNUSED( TSMFNum );
    BSTD_UNUSED( RelativeTsNo );
    BDBG_ERR(( "TSMF not supported in this build" ));
    return BERR_NOT_SUPPORTED;
}

BERR_Code BXPT_TSMF_EnableSemiAutoMode( 
	BXPT_Handle hXpt, 	   		 /* [In] Handle for the transport */
	BXPT_TSMF_InputSel	eInputSel, /*TSMF Parser Input Select*/
	int TSMFNum,			/* [In] Which TSMF. */
	uint32_t SlotMapLo,	        /*Lower 32 bits (LSBs) of the 52-bit TSMF Slot Map vector*/
	uint32_t SlotMapHi,         /* Upper 21 bits of the 53-bit Slot Map vector for TSMF Demultiple*/ 
	uint8_t RelativeTsNo     
	)
{
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( eInputSel );
    BSTD_UNUSED( TSMFNum );
    BSTD_UNUSED( SlotMapLo );
    BSTD_UNUSED( SlotMapHi );
    BSTD_UNUSED( RelativeTsNo );
    BDBG_ERR(( "TSMF not supported in this build" ));
    return BERR_NOT_SUPPORTED;
}

BERR_Code BXPT_TSMF_DisableTsmf( 
	BXPT_Handle hXpt, 	   		/* [In] Handle for the transport */
	int TSMFNum			/* [In] Which TSMF. */
 	)
{
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( TSMFNum );
    BDBG_ERR(( "TSMF not supported in this build" ));
    return BERR_NOT_SUPPORTED;
}

BERR_Code BXPT_TSMF_SetParserConfig( 
	BXPT_Handle hXpt, 	   		/* [In] Handle for the transport */
	int uiParserBandNum,	/* [In] Which Parser band. */
	int TSMFNum,			/* [In] Which TSMF. */
	bool   TsmftoParserEn	    /* [In] enable/disable the selected TSMF to the parser*/
	)
{
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( uiParserBandNum );
    BSTD_UNUSED( TSMFNum );
    BSTD_UNUSED( TsmftoParserEn );
    BDBG_ERR(( "TSMF not supported in this build" ));
    return BERR_NOT_SUPPORTED;
}


