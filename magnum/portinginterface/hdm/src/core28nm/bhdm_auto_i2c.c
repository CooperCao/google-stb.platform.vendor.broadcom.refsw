/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

 ******************************************************************************/#include "bstd.h"

#include "bhdm.h"
#include "../common/bhdm_priv.h"


BDBG_MODULE(BHDM_AUTO_I2C) ;

#if BHDM_CONFIG_HAS_HDCP22
/***************************************************************************
BERR_Code BHDM_AUTO_I2C_GetEventHandle
Summary: Get the event handle for checking HDMI events.
****************************************************************************/
BERR_Code BHDM_AUTO_I2C_GetEventHandle(
   const BHDM_Handle hHDMI,           /* [in] HDMI handle */
   BHDM_AUTO_I2C_EVENT eEventChannel, /* [in] I2C Channel where event will occur */
   BKNI_EventHandle *pAutoI2cEvent	/* [out] event handle */
)
{
	BERR_Code      rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_AUTO_I2C_GetEventHandle) ;
	BDBG_ASSERT( hHDMI );

	/* See BHDM_AUTO_I2C_P_CHANNEL for channel mapping */

	switch (eEventChannel)
	{
	case BHDM_AUTO_I2C_EVENT_eScdcUpdate :
		*pAutoI2cEvent = hHDMI->AutoI2CEvent_ScdcUpdate ;
		break ;

	case BHDM_AUTO_I2C_EVENT_eHdcp22RxStatusUpdate :
		*pAutoI2cEvent = hHDMI->AutoI2CEvent_Hdcp22RxStatusUpdate ;
		break ;

	case BHDM_AUTO_I2C_EVENT_eRead :
		*pAutoI2cEvent = hHDMI->AutoI2CEvent_Read ;
		break ;

	case BHDM_AUTO_I2C_EVENT_eWrite :
		*pAutoI2cEvent = hHDMI->AutoI2CEvent_Write ;
		break ;

	default :
		BDBG_ERR(("Invalid Event Type: %d", eEventChannel)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

done:
	BDBG_LEAVE(BHDM_AUTO_I2C_GetEventHandle) ;
	return rc ;
}


/* Get the SCDC data read by the auto i2c circuit */
BERR_Code BHDM_AUTO_I2C_GetScdcUpdate0Data(const BHDM_Handle hHDMI,
	uint8_t *buffer, uint8_t length)
{
	BERR_Code rc = BERR_SUCCESS ;

	BSTD_UNUSED(length) ;

	BDBG_ENTER(BHDM_AUTO_I2C_GetScdcUpdate0Data) ;

	buffer[0] = hHDMI->stStatusControlData.Update_0 ;
	buffer[1] = hHDMI->stStatusControlData.Update_1 ;

	BDBG_LEAVE(BHDM_AUTO_I2C_GetScdcUpdate0Data) ;
	return rc ;
}


/* Get the HDCP Rx data read by the auto i2c circuit */
BERR_Code BHDM_AUTO_I2C_GetHdcp22RxStatusData(const BHDM_Handle hHDMI,
	uint8_t *RxStatus, uint8_t length)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_AUTO_I2C_GetHdcp22RxStatusData) ;

	BSTD_UNUSED(length) ;

	RxStatus[0] = hHDMI->Hdcp22RxStatusBuffer[0] ;
	RxStatus[1] = hHDMI->Hdcp22RxStatusBuffer[1] ;

	BDBG_LEAVE(BHDM_AUTO_I2C_GetHdcp22RxStatusData) ;

	return rc ;
}

void BHDM_AUTO_I2C_EnableReadChannel_isr(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel, uint8_t enable
)
{
	BHDM_AUTO_I2C_P_TriggerConfiguration stTriggerConfiguration ;

	BDBG_MSG(("Auto I2C Read Channel %d: %s",
		eChannel, enable ? "ENABLED" : "DISABLED")) ;

	BKNI_Memset(&stTriggerConfiguration, 0, sizeof(BHDM_AUTO_I2C_P_TriggerConfiguration)) ;

	/* enable/disable Auto I2c channel */
	BHDM_AUTO_I2C_P_GetTriggerConfiguration_isrsafe(hHDMI, eChannel, &stTriggerConfiguration) ;
		stTriggerConfiguration.enable = enable ;
		stTriggerConfiguration.activePolling = enable ;
	BHDM_AUTO_I2C_P_SetTriggerConfiguration_isr(hHDMI, eChannel, &stTriggerConfiguration) ;
}

void BHDM_AUTO_I2C_EnableReadChannel(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel, uint8_t enable
)
{
	BKNI_EnterCriticalSection() ;
		BHDM_AUTO_I2C_EnableReadChannel_isr(hHDMI, eChannel, enable) ;
	BKNI_LeaveCriticalSection() ;
}



void BHDM_AUTO_I2C_SetChannels_isr(const BHDM_Handle hHDMI,
	uint8_t enable
)
{
	BHDM_AUTO_I2C_P_TriggerConfiguration stTriggerConfiguration ;
	BHDM_AUTO_I2C_P_CHANNEL eChannel ;

	for (eChannel = 0 ; eChannel < BHDM_AUTO_I2C_P_CHANNEL_eMax ; eChannel++)
	{
		/* all I2c transactions triggerred by timer are Auto Poll configurations
		    enable/disable as requested
		*/
		if (hHDMI->AutoI2CChannel_TriggerConfig[eChannel].triggerSource != BHDM_AUTO_I2C_P_TRIGGER_BY_TIMER)
			continue ;

		if (!hHDMI->AutoI2CChannel_TriggerConfig[eChannel].activePolling)
			continue ;

		BDBG_MSG(("Modify Auto I2c Channel %d from %s to %s", eChannel,
			!enable ? "On" : "Off",  enable ? "On" : "Off")) ;

		/* enable/disable Auto I2c channel */
		BHDM_AUTO_I2C_P_GetTriggerConfiguration_isrsafe(hHDMI, eChannel, &stTriggerConfiguration) ;
			stTriggerConfiguration.enable = enable ;
		BHDM_AUTO_I2C_P_SetTriggerConfiguration_isr(hHDMI, eChannel, &stTriggerConfiguration) ;
	}
}


BERR_Code BHDM_AUTO_I2C_IsHdcp2xHWTimersAvailable_isrsafe(
	const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
	bool *available
)
{
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;

	BDBG_ENTER(BHDM_AUTO_I2C_IsHdcp2xHWTimersAvailable_isrsafe);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI);

	hRegister = hHDMI->hRegister;
	ulOffset = hHDMI->ulOffset;

	/* Check Timer availability */
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_STATUS0 + ulOffset);
	if (BCHP_GET_FIELD_DATA(Register, HDMI_TX_AUTO_I2C_HDCP2TX_STATUS0, TIMER_0_ACTIVE)
		+ BCHP_GET_FIELD_DATA(Register, HDMI_TX_AUTO_I2C_HDCP2TX_STATUS0, TIMER_1_ACTIVE)
		+ BCHP_GET_FIELD_DATA(Register, HDMI_TX_AUTO_I2C_HDCP2TX_STATUS0, TIMER_2_ACTIVE))
	{
		*available = false;
	}
	else {
		*available = true;
	}

	BDBG_LEAVE(BHDM_AUTO_I2C_IsHdcp2xHWTimersAvailable_isrsafe);
	return BERR_SUCCESS;
}


BERR_Code BHDM_AUTO_I2C_Reset_isr(
	const BHDM_Handle hHDMI		   /* [in] HDMI handle */
)
{
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;

	BDBG_ENTER(BHDM_AUTO_I2C_Reset_isr);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI);

	hRegister = hHDMI->hRegister;
	ulOffset = hHDMI->ulOffset;

	/* Check Timer availability */
	Register = BREG_Read32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset);
	Register |= 0x00000080;	/* write 1 to HDCP2_I2C - a private field */
	BREG_Write32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset, Register);

	Register &= 0x0000017F;		/* write 0 to HDCP2_I2C - a private field */
	BREG_Write32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset, Register) ;

	BDBG_LEAVE(BHDM_AUTO_I2C_Reset_isr);
	return BERR_SUCCESS;
}
#endif
