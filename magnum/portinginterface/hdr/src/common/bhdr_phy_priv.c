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
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bhdr.h"

#include "bhdr_priv.h"
#include "bhdr_phy_priv.h"

#include "bhdr_config.h"

BDBG_MODULE(BHDR_PHY) ;

BERR_Code BHDR_P_SetHdmiPassthroughMode(BHDR_Handle hHDR, bool enable)
{

	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register ;

	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;
	BDBG_ENTER(BHDR_P_SetHdmiPassthroughMode) ;

	if (enable == hHDR->DeviceSettings.bHdmiHardwarePassthrough)
		goto done ;

	hRegister = hHDR->hRegister ;

#if BHDR_CONFIG_HW_PASSTHROUGH_SUPPORT
	BDBG_WRN(("HDMI Rx Passthrough Setting: %d", enable)) ;
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_PTHRU_CFG) ;
	Register &= ~ BCHP_MASK(DVP_HR_PTHRU_CFG, MODE) ;
	Register |= BCHP_FIELD_DATA(DVP_HR_PTHRU_CFG,MODE,enable) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_PTHRU_CFG, Register) ;
#else
	BSTD_UNUSED(hRegister) ;
	BSTD_UNUSED(Register) ;
#endif

done:
	BDBG_LEAVE(BHDR_P_SetHdmiPassthroughMode) ;
	return rc ;
}


