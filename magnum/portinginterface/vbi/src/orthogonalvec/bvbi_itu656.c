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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"			/* standard types */
#include "bdbg.h"			/* Dbglib */
#include "bvbi.h"			/* VBI processing, this module. */
#include "bvbi_priv.h"		/* VBI internal data structures */

#if (BVBI_P_HAS_XSER_TT != 0) /** { **/

#include "bchp_itu656_0.h"	/* RDB info for ITU656 registers */

BDBG_MODULE(BVBI);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

/***************************************************************************
* Implementation of supporting VBI_ENC functions that are not in API
***************************************************************************/

BERR_Code BVBI_P_ITU656_Init( 
	BREG_Handle hReg, const BVBI_XSER_Settings* pXSERdefaultSettings )
{
	uint32_t ulReg;
	uint32_t iMode;

	BDBG_ENTER(BVBI_P_ITU656_Init);

	switch (pXSERdefaultSettings->ttSerialDataSync)
	{
    case BVBI_TTserialDataSync_EAV:
		iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_EAV;
		break;
    case BVBI_TTserialDataSync_SAV:
		iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_SAV;
		break;
    case BVBI_TTserialDataSync_RQ:
		iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_RQ;
		break;
	default:
		iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_DISABLED;
		break;
	}

	ulReg = (
	BCHP_FIELD_DATA (ITU656_0_ITU656_TTE_CTRL, TTE_MODE,    iMode) |
	BCHP_FIELD_DATA (ITU656_0_ITU656_TTE_CTRL, DELAY_COUNT, 
	      pXSERdefaultSettings->iTTserialDataSyncDelay) );
	BREG_Write32 (hReg, BCHP_ITU656_0_ITU656_TTE_CTRL, ulReg);

	BDBG_LEAVE(BVBI_P_ITU656_Init);

	return BERR_SUCCESS;
}

BERR_Code BVBI_P_ITU656_Enc_Program (
	BREG_Handle hReg,
	BVBI_XSER_Settings* pSettings,
	uint32_t ulActive_XSER_Standards)
{
	uint32_t ulReg;
	uint32_t iMode;

	BDBG_ENTER(BVBI_P_ITU656_Enc_Program);

	switch (pSettings->ttSerialDataSync)
	{
    case BVBI_TTserialDataSync_EAV:
		iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_EAV;
		break;
    case BVBI_TTserialDataSync_SAV:
		iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_SAV;
		break;
    case BVBI_TTserialDataSync_RQ:
		iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_RQ;
		break;
	default:
		iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_DISABLED;
		break;
	}

	if ((ulActive_XSER_Standards & BVBI_P_SELECT_TT) == 0)
		iMode = BCHP_ITU656_0_ITU656_TTE_CTRL_TTE_MODE_DISABLED;

	ulReg = (
	BCHP_FIELD_DATA (ITU656_0_ITU656_TTE_CTRL, TTE_MODE,    iMode) |
	BCHP_FIELD_DATA (ITU656_0_ITU656_TTE_CTRL, DELAY_COUNT, 
	      pSettings->iTTserialDataSyncDelay) );
	BREG_Write32 (hReg, BCHP_ITU656_0_ITU656_TTE_CTRL, ulReg);

	BDBG_LEAVE(BVBI_P_ITU656_Enc_Program);


	return BERR_SUCCESS;
}

#endif /** }  BVBI_P_HAS_XSER_TT **/
