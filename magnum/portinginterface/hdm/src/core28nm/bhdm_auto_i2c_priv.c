/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
*
 ******************************************************************************/
#include "../common/bhdm_priv.h"


#define BHDM_AUTO_I2C_P_POLL_SCDC_UPDATE 250  /* ms */
#define BHDM_AUTO_I2C_P_POLL_HDCP_RX_STATUS 1  /* ms */


#define BHDM_AUTO_I2C_P_BSCx_CHIP_ADDR                      0x00
#define BHDM_AUTO_I2C_P_BSCx_DATA_IN0                        0x01
#define BHDM_AUTO_I2C_P_BSCx_DATA_IN1                        0x02
#define BHDM_AUTO_I2C_P_BSCx_DATA_IN2                        0x03
#define BHDM_AUTO_I2C_P_BSCx_DATA_IN3                        0x04
#define BHDM_AUTO_I2C_P_BSCx_DATA_IN4                        0x05
#define BHDM_AUTO_I2C_P_BSCx_DATA_IN5                        0x06
#define BHDM_AUTO_I2C_P_BSCx_DATA_IN6                        0x07
#define BHDM_AUTO_I2C_P_BSCx_DATA_IN7                        0x08
#define BHDM_AUTO_I2C_P_BSCx_CNT_REG                          0x09
#define BHDM_AUTO_I2C_P_BSCx_CTL_REG                          0x0a
#define BHDM_AUTO_I2C_P_BSCx_IIC_ENABLE                     0x0b
#define BHDM_AUTO_I2C_P_BSCx_DATA_OUT0                      0x0c
#define BHDM_AUTO_I2C_P_BSCx_DATA_OUT1                      0x0d
#define BHDM_AUTO_I2C_P_BSCx_DATA_OUT2                      0x0e
#define BHDM_AUTO_I2C_P_BSCx_DATA_OUT3                      0x0f
#define BHDM_AUTO_I2C_P_BSCx_DATA_OUT4                      0x10
#define BHDM_AUTO_I2C_P_BSCx_DATA_OUT5                      0x11
#define BHDM_AUTO_I2C_P_BSCx_DATA_OUT6                      0x12
#define BHDM_AUTO_I2C_P_BSCx_DATA_OUT7                      0x13
#define BHDM_AUTO_I2C_P_BSCx_CTLHI_REG                       0x14
#define BHDM_AUTO_I2C_P_BSCx_SCL_PARAM                      0x15


#define BHDM_AUTO_I2C_READ_ENABLE 1
#define BHDM_AUTO_I2C_READ_DISABLE 0

#define BHDM_AUTO_I2C_BSCx_WRITE_ENABLE 1
#define BHDM_AUTO_I2C_BSCx_WRITE_DISABLE 0


/******************************************************************************
Summary:
Interrupt Callback Table to describe interrupt Names, isrs, and Masks
*******************************************************************************/
typedef struct BHDM_AUTO_I2C_P_InterruptMap
{
	BINT_Id        IntrId;  /* unique interrurpt identifier */
	int               eIntrEnumeration;  /* descriptive interrrupt enumeration */
	bool             enable ; /* debug purposes */
} BHDM_AUTO_I2C_P_InterruptMap ;


#if BHDM_CONFIG_HAS_HDCP22
static void BHDM_AUTO_I2C_P_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] not used */
) ;

static const BHDM_AUTO_I2C_P_InterruptMap
	s_BHDM_AUTO_I2C_IntrMap[BHDM_CONFIG_NUM_HDMI_CORES][MAKE_AUTO_I2C_INTR_ENUM(LAST)] =
{
	{
		{ BCHP_INT_ID_I2C_CH0_DONE_INTR, BHDM_AUTO_I2C_INTR_eHdcp22RxStatus, true},
		{ BCHP_INT_ID_I2C_CH1_DONE_INTR, BHDM_AUTO_I2C_INTR_eScdcUpdate, true},
		{ BCHP_INT_ID_I2C_CH2_DONE_INTR, BHDM_AUTO_I2C_INTR_eRead, false},
		{ BCHP_INT_ID_I2C_CH3_DONE_INTR, BHDM_AUTO_I2C_INTR_eWrite, false}
	}

	#if !(BHDM_CONFIG_NUM_HDMI_CORES == 1)
	#error "Number of HDMI Cores incorrectly specified"
	#endif
} ;


BDBG_MODULE(BHDM_AUTO_I2C_PRIV) ;


static void BHDM_AUTO_I2C_P_ResetHwStateMachine(BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/***********/
	/* Reset AutoI2C HW statemachine */
	/***********/
	Register = BREG_Read32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset) ;
	Register |= BCHP_FIELD_DATA(DVP_HT_SW_INIT, HDCP2_I2C, 1);   /* Write 1 */
	BREG_Write32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset, Register) ;

	Register &= ~BCHP_MASK(DVP_HT_SW_INIT, HDCP2_I2C); 	/* Follow by a 0 */
	BREG_Write32(hRegister, BCHP_DVP_HT_SW_INIT + ulOffset, Register) ;
}


BERR_Code  BHDM_AUTO_I2C_P_AllocateResources(const BHDM_Handle hHDMI)
{
	BERR_Code rc ;
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint8_t i ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;


#define BHDM_AUTO_I2C_MODIFY_CTL_CTLHI_MASK \
		Register &= ~ ( \
			  BCHP_MASK(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTL) \
			| BCHP_MASK(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTLHI)) ;
 /* end define */


	BHDM_AUTO_I2C_P_ResetHwStateMachine(hHDMI) ;

	/********/
	/* RM_M0 */
	/********/
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_RM_M0 + ulOffset) ;
		BHDM_AUTO_I2C_MODIFY_CTL_CTLHI_MASK ;
		Register |=
				  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTL, 0xE0)
				| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTLHI, 0x0C) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_RM_M0 + ulOffset, Register) ;

	/********/
	/* RM_M1 */
	/********/
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_RM_M1  + ulOffset) ;
		BHDM_AUTO_I2C_MODIFY_CTL_CTLHI_MASK ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTL, 0xE1)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTLHI, 0x0C) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_RM_M1 + ulOffset, Register) ;

	/********/
	/* RM_M2 */
	/********/
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_RM_M2  + ulOffset) ;
		BHDM_AUTO_I2C_MODIFY_CTL_CTLHI_MASK ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTL, 0xE1)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTLHI, 0x0C) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_RM_M2 + ulOffset, Register) ;

	/********/
	/* RM_M3 */
	/********/
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_RM_M3  + ulOffset) ;
		BHDM_AUTO_I2C_MODIFY_CTL_CTLHI_MASK ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTL, 0xE1)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTLHI, 0x0C) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_RM_M3 + ulOffset, Register) ;




	/********/
	/* WM_M0 */
	/********/
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0  + ulOffset) ;
		BHDM_AUTO_I2C_MODIFY_CTL_CTLHI_MASK ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTL, 0xE0)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTLHI, 0x0C) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0 + ulOffset, Register ) ;

	/********/
	/* WM_M1 */
	/********/
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_WM_M1  + ulOffset) ;
		BHDM_AUTO_I2C_MODIFY_CTL_CTLHI_MASK ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTL, 0xE0)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTLHI, 0x0C) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_WM_M1 + ulOffset, Register ) ;

	/********/
	/* WM_M2 */
	/********/
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_WM_M2  + ulOffset) ;
		BHDM_AUTO_I2C_MODIFY_CTL_CTLHI_MASK ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTL, 0xE0)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_WM_M0, CTLHI, 0x0C) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_WM_M2 + ulOffset, Register ) ;


	/* Create Events for use with Interrupts */
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->AutoI2CEvent_ScdcUpdate))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->AutoI2CEvent_Hdcp22RxStatusUpdate))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->AutoI2CEvent_Write))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->AutoI2CEvent_Read))) ;



	/* Register/enable interrupt callbacks */

	for (i = 0; i < MAKE_AUTO_I2C_INTR_ENUM(LAST) ; i++)
	{
		BHDM_CHECK_RC( rc, BINT_CreateCallback(&(hHDMI->hAutoI2cCallback[i]),
			hHDMI->hInterrupt, s_BHDM_AUTO_I2C_IntrMap[hHDMI->eCoreId][i].IntrId,
			BHDM_AUTO_I2C_P_isr, hHDMI, i ));

		/* clear interrupt callback */
		BHDM_CHECK_RC(rc, BINT_ClearCallback( hHDMI->hAutoI2cCallback[i])) ;

		/* skip interrupt if not enabled in table...  */
		if (!s_BHDM_AUTO_I2C_IntrMap[hHDMI->eCoreId][i].enable)
		{
			continue ;
		}

		BHDM_CHECK_RC(rc, BINT_EnableCallback( hHDMI->hAutoI2cCallback[i])) ;
	}

	/*configure polling channels; NOTE: Polling is not started here */

	/*************/
	/* SCDC  */
	/*************/
	rc = BHDM_AUTO_I2C_P_ConfigureReadChannel(hHDMI,
		BHDM_AUTO_I2C_MODE_ePollScdcUpdate0, BHDM_AUTO_I2C_P_SCDC_SLAVE_ADDR,
		BHDM_AUTO_I2C_P_SCDC_UPDATE0_OFFSET, 2) ;
	if (rc) { (void)BERR_TRACE(rc) ; goto done ;}

	/************/
	/* HDCP2.2  */
	/************/
	rc = BHDM_AUTO_I2C_P_ConfigureReadChannel(hHDMI,
		BHDM_AUTO_I2C_MODE_ePollHdcp22RxStatus, BHDM_AUTO_I2C_P_HDCP22_SLAVE_ADDR,
		BHDM_AUTO_I2C_P_HDCP22_RXSTATUS_OFFSET, 2) ;
	if (rc) { (void)BERR_TRACE(rc) ; goto done ;}

done :
	if (rc)
	{
		/* error has been detectedl; free any previously allocated resources */
		BHDM_AUTO_I2C_P_FreeResources(hHDMI) ;
	}

	return rc ;
}


BERR_Code  BHDM_AUTO_I2C_P_FreeResources(const BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;

	/* Disable and Destroy the HDMI Auto I2C Callbacks */
	BHDM_AUTO_I2C_P_DisableInterrupts(hHDMI) ;

	/* disable any auto transactions that may be in progress */
	BHDM_AUTO_I2C_EnableReadChannel(hHDMI,
		BHDM_AUTO_I2C_P_CHANNEL_ePollScdcUpdate0, 0) ;
	BHDM_AUTO_I2C_EnableReadChannel(hHDMI,
		BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus, 0) ;

	for (i = 0; i < MAKE_AUTO_I2C_INTR_ENUM(LAST); i++)
	{
		/* skip interrupt if not enabled in table...  */
		if (!s_BHDM_AUTO_I2C_IntrMap[hHDMI->eCoreId][i].enable)
		{
			continue ;
		}

		/* disable and destroy all AutoI2c Callbacks */
		if (hHDMI->hAutoI2cCallback[i])
		{
			if (BINT_DisableCallback( (hHDMI->hAutoI2cCallback[i])) != BERR_SUCCESS)
			{
				(void)BERR_TRACE(rc) ;
				rc = BERR_UNKNOWN ;
			}

			if  (BINT_DestroyCallback( hHDMI->hAutoI2cCallback[i]) != BERR_SUCCESS)
			{
				BDBG_ERR(("Error Destroying Callback %d", i)) ;
				(void)BERR_TRACE(rc) ;
				rc = BERR_UNKNOWN ;

			}
		}
	}

	/* Destroy the Auto I2C events for use with Interrupts */
	if (hHDMI->AutoI2CEvent_ScdcUpdate)
	{
		BKNI_DestroyEvent(hHDMI->AutoI2CEvent_ScdcUpdate) ;
	}

	if (hHDMI->AutoI2CEvent_Hdcp22RxStatusUpdate)
	{
		BKNI_DestroyEvent(hHDMI->AutoI2CEvent_Hdcp22RxStatusUpdate) ;
	}

	if (hHDMI->AutoI2CEvent_Write)
	{
		BKNI_DestroyEvent(hHDMI->AutoI2CEvent_Write) ;
	}

	if (hHDMI->AutoI2CEvent_Read)
	{
		BKNI_DestroyEvent(hHDMI->AutoI2CEvent_Read) ;
	}

	BHDM_AUTO_I2C_P_ResetHwStateMachine(hHDMI) ;

	return rc ;
}


BERR_Code BHDM_AUTO_I2C_P_EnableInterrupts(BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS ;

	/* enable the auto i2c channels */
	BKNI_EnterCriticalSection() ;
		BHDM_AUTO_I2C_SetChannels_isr(hHDMI, 1) ;
	BKNI_LeaveCriticalSection() ;

	return rc ;
}


BERR_Code BHDM_AUTO_I2C_P_DisableInterrupts(BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS ;

	/* disable the auto i2c channels */
	BKNI_EnterCriticalSection() ;
		BHDM_AUTO_I2C_SetChannels_isr(hHDMI, 0) ;
	BKNI_LeaveCriticalSection() ;

	return rc ;
}


void  BHDM_AUTO_I2C_P_SetTriggerConfiguration_isr(const BHDM_Handle hHDMI, BHDM_AUTO_I2C_P_CHANNEL eChannel,
	const BHDM_AUTO_I2C_P_TriggerConfiguration *pstTriggerConfig
)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint32_t autoI2cChxOffset ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	autoI2cChxOffset = BHDM_AUTO_I2C_P_NUM_CHX_REGISTERS * eChannel ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_CH0_CFG + ulOffset + autoI2cChxOffset) ;
		Register &= ~ (
			  BCHP_MASK(HDMI_TX_AUTO_I2C_CH0_CFG, TIMER)
			| BCHP_MASK(HDMI_TX_AUTO_I2C_CH0_CFG, RD_ADDR_OFFSET)
			| BCHP_MASK(HDMI_TX_AUTO_I2C_CH0_CFG, MODE)
			| BCHP_MASK(HDMI_TX_AUTO_I2C_CH0_CFG, TRIGGER_SRC)
			| BCHP_MASK(HDMI_TX_AUTO_I2C_CH0_CFG, ENABLE)) ;

		Register |=
			  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, TIMER, pstTriggerConfig->timerMs)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, RD_ADDR_OFFSET, pstTriggerConfig->bscxDataOffset)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, MODE, pstTriggerConfig->eMode)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, TRIGGER_SRC, pstTriggerConfig->triggerSource)
			| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_CFG, ENABLE, pstTriggerConfig->enable) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_CH0_CFG + ulOffset + autoI2cChxOffset, Register) ;

	hHDMI->AutoI2CChannel_TriggerConfig[eChannel] = *pstTriggerConfig ;

}


void BHDM_AUTO_I2C_P_GetTriggerConfiguration_isrsafe(const BHDM_Handle hHDMI, BHDM_AUTO_I2C_P_CHANNEL eChannel,
	BHDM_AUTO_I2C_P_TriggerConfiguration *pstTriggerConfig
)
{
	BKNI_Memset(pstTriggerConfig, 0, sizeof(BHDM_AUTO_I2C_P_TriggerConfiguration)) ;
	*pstTriggerConfig = hHDMI->AutoI2CChannel_TriggerConfig[eChannel] ;
}


static BERR_Code BHDM_AUTO_I2C_P_SetBSCx_isr(const BHDM_Handle hHDMI,

	BHDM_AUTO_I2C_P_CHANNEL eChannel,  /* channel: CH0, CH1, CH2, or CH3 */
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET eRegOffset, /* CHy_REGx it can range from 0 to 13 */
	uint8_t writeEnable,
	uint8_t bscxOffset, /* BSCx offset register can range from 0 to 21; Maps to BSCx registers*/
	uint32_t data /* data to be written to the BSCx register */
	)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint32_t
		channelOffset, regOffset ;

#if 0
--------------------------------------------------------------------------
 function  write_chx_register (channel, chx_addr, i2c_addr_offset, data)
    write CHx register
    channel:         one of CH0, CH1, CH2, CH3
    chx_addr:        CHy_REGx it can range from 0 to 13
    i2c_addr_offset: BBSCx address to be written
    data:            datat to be written with
--------------------------------------------------------------------------
#endif

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;


	channelOffset = BHDM_AUTO_I2C_P_NUM_CHX_REGISTERS * eChannel ;

	regOffset = BHDM_AUTO_I2C_P_NUM_CH_REGX_REGISTERS * eRegOffset ;

	BDBG_MSG(("Channel: %d Offset: %x regOffset: %d",
		eChannel, channelOffset, regOffset)) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_CH0_REG0_CFG + ulOffset + channelOffset + regOffset) ;
		Register &=
			~ (BCHP_MASK(HDMI_TX_AUTO_I2C_CH0_REG0_CFG, WR_EN)
			| BCHP_MASK(HDMI_TX_AUTO_I2C_CH0_REG0_CFG, WR_ADDR_OFFSET)) ;

		Register |=
			  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_REG0_CFG, WR_EN, writeEnable)
			|BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_CH0_REG0_CFG, WR_ADDR_OFFSET, bscxOffset) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_CH0_REG0_CFG + ulOffset + channelOffset + regOffset, Register) ;


	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_CH0_REG0_WD + ulOffset + channelOffset + regOffset, data) ;

	return rc ;

}


BERR_Code BHDM_AUTO_I2C_P_ConfigureReadChannel_isr(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_MODE eAutoI2cMode,
	uint8_t slaveAddress, uint8_t slaveOffset, uint8_t length)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t channelOffset ;
	uint16_t pollingInterval ;
	uint8_t triggerSource ;
	uint8_t eAutoI2cConfig ;
	BHDM_AUTO_I2C_P_CHANNEL eChannel ;
	BHDM_AUTO_I2C_P_TriggerConfiguration stTriggerConfig ;

	switch (eAutoI2cMode)
	{
		case BHDM_AUTO_I2C_MODE_ePollScdcUpdate0 :
			triggerSource = BHDM_AUTO_I2C_P_TRIGGER_BY_TIMER ; /* 1 = TIMER  */
			pollingInterval = BHDM_AUTO_I2C_P_POLL_SCDC_UPDATE ;
			eChannel = BHDM_AUTO_I2C_P_CHANNEL_ePollScdcUpdate0 ;
			break ;

		case BHDM_AUTO_I2C_MODE_ePollHdcp22RxStatus :
			triggerSource = BHDM_AUTO_I2C_P_TRIGGER_BY_TIMER ; /* 1 = TIMER  */
			pollingInterval = BHDM_AUTO_I2C_P_POLL_HDCP_RX_STATUS ;
			eChannel = BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus;
			break ;

		case BHDM_AUTO_I2C_MODE_eRead :
			triggerSource = BHDM_AUTO_I2C_P_TRIGGER_BY_SW ; /* 0 = SOFTWARE */
			pollingInterval = BHDM_AUTO_I2C_P_POLL_HDCP_RX_STATUS ;
			eChannel = BHDM_AUTO_I2C_P_CHANNEL_eRead ;
			break ;

		case BHDM_AUTO_I2C_MODE_eWrite :
			BDBG_ERR(("Auto I2C Mode of Write for Read Configuration is incorrect")) ;
			rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
			goto done ;
			break ;

		default :
			BDBG_WRN(("Unknown HDMI Auto I2c mode %d", eAutoI2cMode)) ;
			rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
			goto done ;
	}


	channelOffset = BCHP_HDMI_TX_AUTO_I2C_CH0_REG0_CFG
		+ BHDM_AUTO_I2C_P_NUM_CHX_REGISTERS * (uint8_t) eChannel ;

	BDBG_MSG(("Channel: %d Offset: %x", eChannel, channelOffset)) ;
        BSTD_UNUSED(channelOffset);

	if (length > BHDM_AUTO_I2C_P_MAX_READ_BYTES)
	{
		BDBG_ERR(("Auto I2C Read of %d exceeds auto maximum of %d",
			length, BHDM_AUTO_I2C_P_MAX_READ_BYTES)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	eAutoI2cConfig = 0 ;

	BDBG_MSG(("Configure Read Channel BSC%d", eAutoI2cConfig)) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel,
		eAutoI2cConfig++, BHDM_AUTO_I2C_BSCx_WRITE_ENABLE,
		BHDM_AUTO_I2C_P_BSCx_IIC_ENABLE, 0) ;

	BDBG_MSG(("Configure Read Channel BSC%d", eAutoI2cConfig)) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(
		hHDMI,		eChannel, eAutoI2cConfig++,
		BHDM_AUTO_I2C_BSCx_WRITE_ENABLE, BHDM_AUTO_I2C_P_BSCx_CHIP_ADDR,
		slaveAddress) ;

	/* TODO investigate use of FastRead */

#if TODO
/* for all CTL registers */
/*
   misc.SetRegField tmp_data, BSCA.CTL_REG.INT_EN, 1            'Turn on interrupt
   misc.SetRegField tmp_data, BSCA.CTL_REG.DTF,    IIC_TRANS_WR 'Data Transfer Format
           ' IIC_TRANS_W    ' Master-transmitter transmits to slave-receiver.
           ' IIC_TRANS_R    ' Master reads slave immediately after transmitting slave Chip-Address (Read only).
           ' IIC_TRANS_RW   ' Combined Read-then-Write format. Master repeats START without asserting STOP
           ' IIC_TRANS_WR   ' Combined Write-then-Read format. Master repeats START without asserting STOP
   misc.SetRegField tmp_data, BSCA.CTL_REG.SCL_SEL, CH_SCL_SEL ' 375kHz base frequency
   misc.SetRegField tmp_data, BSCA.CTL_REG.DIV_CLK, 1 ' Actual frequency is 375kHz/4 = 93.75kHz
   call write_chx_register (channel,  2, BSCx_CTL_REG,  tmp_data)
*/
#endif

	BDBG_MSG(("Configure Read Channel BSC%d", eAutoI2cConfig)) ;
	Register =
		  BCHP_MASK(BSCA_CTL_REG, INT_EN)
		| BCHP_FIELD_DATA(BSCA_CTL_REG, DTF, BHDM_AUTO_I2C_P_WRITEREAD)
		| BCHP_FIELD_DATA(BSCA_CTL_REG, SCL_SEL, BHDM_AUTO_I2C_P_SCL_SEL_187)
		| BCHP_FIELD_DATA(BSCA_CTL_REG, DIV_CLK, BHDM_AUTO_I2C_P_DIV_CLK_BY_4) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++,
		BHDM_AUTO_I2C_BSCx_WRITE_ENABLE, BHDM_AUTO_I2C_P_BSCx_CTL_REG, Register) ;


	BDBG_MSG(("Configure Read Channel BSC%d", eAutoI2cConfig)) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++,
		BHDM_AUTO_I2C_BSCx_WRITE_ENABLE, BHDM_AUTO_I2C_P_BSCx_DATA_IN0, slaveOffset) ;


	/*
	   tmp_data = &h'X'1                             '1 byte write followed by 'X' byte read
	   call write_chx_register (channel,  4, BBSCx_CNT_REG,  tmp_data)
	*/
	BDBG_MSG(("Configure Read Channel BSC%d", eAutoI2cConfig)) ;
	Register =
		 BCHP_FIELD_DATA(BSCA_CNT_REG , CNT_REG1, 1)
		|BCHP_FIELD_DATA(BSCA_CNT_REG , CNT_REG2, length) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++,
		BHDM_AUTO_I2C_BSCx_WRITE_ENABLE, BHDM_AUTO_I2C_P_BSCx_CNT_REG, Register) ;


	/*
	   tmp_data = &hC0                             'select 32-bit mode, bit [6]
	   call write_chx_register (channel,  5, BBSCx_CTLHI_REG, tmp_data )
	   TODO Should IGNORE_ACK be set/disabled for all Read Channels?
	*/
	BDBG_MSG(("Configure Read Channel BSC%d", eAutoI2cConfig)) ;
	Register =
		  BCHP_FIELD_DATA(BSCA_CTLHI_REG, INPUT_SWITCHING_LEVEL, 1)
		| BCHP_FIELD_DATA(BSCA_CTLHI_REG, DATA_REG_SIZE, 1) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel,  eAutoI2cConfig++,
		BHDM_AUTO_I2C_BSCx_WRITE_ENABLE, BHDM_AUTO_I2C_P_BSCx_CTLHI_REG, Register) ;


	/*
	   tmp_data = &h1                              'enable BBSCx
	   call write_chx_register (channel,  6, BBSCx_IIC_ENABLE, tmp_data )
	*/
	BDBG_MSG(("Configure Read Channel BSC%d", eAutoI2cConfig)) ;
	Register =  BCHP_MASK(BSCA_IIC_ENABLE, ENABLE) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++,
		BHDM_AUTO_I2C_BSCx_WRITE_ENABLE, BHDM_AUTO_I2C_P_BSCx_IIC_ENABLE, Register) ;

	/*  After configuring BSCx register; add a terminator for all transactions AutoRead, AutoWrite, Read, Write */
	if (eAutoI2cConfig < BHDM_AUTO_I2C_P_CHANNEL_OFFSET_eMax)
	{
		Register = 0x0;
		BDBG_MSG(("Configure Read Channel BSC%d", eAutoI2cConfig)) ;
		BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++,
			BHDM_AUTO_I2C_BSCx_WRITE_DISABLE, 0, Register) ;  /* is a don't care for this part of the transaction; valid for <14 */
	}
	else
	{
		BDBG_ERR(("Auto I2C transactions of %d exceed maximum of %d",
			eAutoI2cConfig, BHDM_AUTO_I2C_P_CHANNEL_OFFSET_eMax)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	BKNI_Memset(&stTriggerConfig, 0, sizeof(BHDM_AUTO_I2C_P_TriggerConfiguration )) ;
		stTriggerConfig.timerMs = pollingInterval ;
		stTriggerConfig.bscxDataOffset = BHDM_AUTO_I2C_P_BSCx_DATA_OUT0 ;
		stTriggerConfig.eMode = eAutoI2cMode ;
		stTriggerConfig.triggerSource = triggerSource ;

		/* DO NOT enable; Use BHDM_AUTO_I2C_EnableReadChannel */
		stTriggerConfig.enable = BHDM_AUTO_I2C_READ_DISABLE ;
		stTriggerConfig.activePolling = false ;
	BHDM_AUTO_I2C_P_SetTriggerConfiguration_isr(hHDMI, eChannel, &stTriggerConfig) ;

done:
	return rc ;

}


BERR_Code BHDM_AUTO_I2C_P_ConfigureReadChannel(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_MODE eAutoI2cMode,
	uint8_t slaveAddress, uint8_t slaveOffset, uint8_t length)
{
	BERR_Code rc = BERR_SUCCESS ;

	BKNI_EnterCriticalSection() ;
		rc = BHDM_AUTO_I2C_P_ConfigureReadChannel_isr(hHDMI, eAutoI2cMode,
			slaveAddress, slaveOffset, length)	;
	BKNI_LeaveCriticalSection() ;
	return rc ;
}


BERR_Code BHDM_AUTO_I2C_P_ConfigureWriteChannel_isr(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel,
	uint8_t slaveAddress, uint8_t slaveOffset,
	uint8_t *buffer, uint8_t length)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint32_t Register, ulOffset ;
	uint32_t channelOffset  ;
	uint8_t index ;

	BHDM_AUTO_I2C_P_CHANNEL_OFFSET eAutoI2cConfig ;
	BHDM_AUTO_I2C_P_TriggerConfiguration stTriggerConfig ;

	ulOffset = hHDMI->ulOffset ;

	channelOffset = BCHP_HDMI_TX_AUTO_I2C_CH0_REG0_CFG + ulOffset
		+ BHDM_AUTO_I2C_P_NUM_CHX_REGISTERS * eChannel ;
	BDBG_MSG(("Channel: %d Offset: %x", eChannel, channelOffset)) ;
        BSTD_UNUSED(channelOffset);

	if (length > BHDM_AUTO_I2C_P_MAX_WRITE_BYTES)
	{
		BDBG_ERR(("Auto I2C Write of %d exceeds auto maximum of %d",
			length, BHDM_AUTO_I2C_P_MAX_WRITE_BYTES)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	/*
		tmp_data = &h0
		call write_chx_register (channel,  0, BSCx_IIC_ENABLE, tmp_data) 'disable any outstanding transactions
	*/
	eAutoI2cConfig = 0 ;
	BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++, BHDM_AUTO_I2C_BSCx_WRITE_ENABLE,
		BHDM_AUTO_I2C_P_BSCx_IIC_ENABLE, 0) ;

	BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++, BHDM_AUTO_I2C_BSCx_WRITE_ENABLE,
		BHDM_AUTO_I2C_P_BSCx_CHIP_ADDR, slaveAddress) ;

	BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
	Register =
		  BCHP_MASK(BSCA_CTL_REG, INT_EN)
		| BCHP_FIELD_DATA(BSCA_CTL_REG, DTF, BHDM_AUTO_I2C_P_WRITE)
		| BCHP_FIELD_DATA(BSCA_CTL_REG, SCL_SEL, 0)
		| BCHP_FIELD_DATA(BSCA_CTL_REG, DIV_CLK, 1) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++, BHDM_AUTO_I2C_BSCx_WRITE_ENABLE,
		BHDM_AUTO_I2C_P_BSCx_CTL_REG, Register) ;


	/*
		tmp_data = 0
		isc.set_field tmp_data,  5, 0, 9       'first transaction is 9-byte
		                                                '1-byte register offset + length byte data
		call write_chx_register (channel,  3, BSCx_CNT_REG,  tmp_data)
	*/
	BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
	Register =
		  BCHP_FIELD_DATA(BSCA_CNT_REG, CNT_REG1, length + 1)
		| BCHP_FIELD_DATA(BSCA_CNT_REG, CNT_REG2, 0) ;  /* Don't Care; Unused */
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++, BHDM_AUTO_I2C_BSCx_WRITE_ENABLE,
		BHDM_AUTO_I2C_P_BSCx_CNT_REG, Register) ;

	BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
	Register =
		  BCHP_FIELD_DATA(BSCA_CTLHI_REG, INPUT_SWITCHING_LEVEL, 1)
		| BCHP_FIELD_DATA(BSCA_CTLHI_REG, DATA_REG_SIZE, 1) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++, BHDM_AUTO_I2C_BSCx_WRITE_ENABLE,
		BHDM_AUTO_I2C_P_BSCx_CTLHI_REG, Register) ;

	/* write the first data register
           which contains slave offset + and up to 3 bytes of data
	*/
	Register = slaveOffset ;
	index = 0 ;
	while ((index < 3) && (index < length))
	{
		Register |= buffer[index]  << (8 * (index + 1)) ;
		index++ ;
	}
	BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++, BHDM_AUTO_I2C_BSCx_WRITE_ENABLE,
		BHDM_AUTO_I2C_P_BSCx_DATA_IN0, Register) ;
	BDBG_MSG(("Write %08x to DATA_IN0 %d",
		Register, BHDM_AUTO_I2C_P_BSCx_DATA_IN0 + (index / 4))) ;

	if (length > 3)
	{
		BDBG_ERR(("No Support for length > 3")) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}
#if 0
	/* write any remaning data if Addr + 3 bytes was not enough */
	if (index < length)
	{
		int j ;
		j = 0 ;
		for (  ; index < length ; index++)
		{
			Register |= buffer[index]  << (8 * j) ;
			if ((j % 4) == 0)
			{
				/* write the next BSCx_DATA_INn register */
				BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
				BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++,
					BHDM_AUTO_I2C_P_BSCx_DATA_IN0 + (index / 4) , Register) ;
				BDBG_MSG(("Write %08x to DATA_IN0 %d",
					Register, BHDM_AUTO_I2C_P_BSCx_DATA_IN0 + (index / 4))) ;
				j = 0 ;
			}
			j++ ;
			index++ ;
		}

		/* if the last byte did not fill the register, write the last BSCx_DATA_INn register */
		if (index % 4)
		{
			BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
			BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++,
				BHDM_AUTO_I2C_P_BSCx_DATA_IN0 + (index / 4) , Register) ;
			BDBG_MSG(("Write %08x to DATA_IN0 %d",
				Register, BHDM_AUTO_I2C_P_BSCx_DATA_IN0 + (index / 4))) ;
		}
	}
#endif

	/*
	   tmp_data = &h1                              'enable BBSCx
	   call write_chx_register (channel,  6, BBSCx_IIC_ENABLE, tmp_data )
	*/
	BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
	BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel, eAutoI2cConfig++, BHDM_AUTO_I2C_BSCx_WRITE_ENABLE,
		BHDM_AUTO_I2C_P_BSCx_IIC_ENABLE , 1) ;

	/*  After configuring BSCx register; add a terminator  for all transactions AutoRead, AutoWrite, Read, Write */
	if (eAutoI2cConfig < BHDM_AUTO_I2C_P_CHANNEL_OFFSET_eMax)
	{
		Register = 0x0;
		BDBG_MSG(("Configure Write Channel BSC%d", eAutoI2cConfig)) ;
		BHDM_AUTO_I2C_P_SetBSCx_isr(hHDMI, eChannel,
			eAutoI2cConfig++,  BHDM_AUTO_I2C_BSCx_WRITE_DISABLE, 0, Register) ;  /* is a don't care for this part of the transaction; valid for <14 */
	}
	else
	{
		BDBG_ERR(("Auto I2C transactions of %d exceed maximum of %d",
			eAutoI2cConfig, BHDM_AUTO_I2C_P_CHANNEL_OFFSET_eMax)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}



	BKNI_Memset(&stTriggerConfig, 0, sizeof(BHDM_AUTO_I2C_P_TriggerConfiguration)) ;
		/*
		    stTriggerConfig.timerMs and
		    stTriggerConfig.bscxDataOffset
		    are don't care when writing i2c data
		 */
		stTriggerConfig.eMode = BHDM_AUTO_I2C_MODE_eWrite ;
		stTriggerConfig.triggerSource = BHDM_AUTO_I2C_P_TRIGGER_BY_SW ;
		stTriggerConfig.enable = 0 ;  /* dont care for Write Channel */
	BHDM_AUTO_I2C_P_SetTriggerConfiguration_isr(hHDMI, eChannel, &stTriggerConfig) ;

done:
	return rc ;
}


BERR_Code BHDM_AUTO_I2C_P_ConfigureWriteChannel(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel,
	uint8_t slaveAddress, uint8_t slaveOffset,
	uint8_t *buffer, uint8_t length)
{
	BERR_Code rc = BERR_SUCCESS ;

	BKNI_EnterCriticalSection() ;
		rc = BHDM_AUTO_I2C_P_ConfigureWriteChannel_isr(hHDMI, eChannel,
			slaveAddress, slaveOffset, buffer, length) ;
	BKNI_LeaveCriticalSection() ;

	return rc ;
}


#if !B_REFSW_MINIMAL
static void BHDM_AUTO_I2C_P_EnableChannelWrite(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel
)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_START + ulOffset) ;
		switch (hHDMI->AutoI2CChannel_Read)
		{

		case BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus :
			Register |=  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_START, CH0, 1) ;
			break ;

		case BHDM_AUTO_I2C_P_CHANNEL_ePollScdcUpdate0 :
			Register |=  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_START, CH1, 1) ;
			break ;

		case BHDM_AUTO_I2C_P_CHANNEL_eRead :
			Register |=  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_START, CH2, 1) ;
			break ;

		case BHDM_AUTO_I2C_P_CHANNEL_eWrite :
			Register |=  BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_START, CH3, 1) ;
			break ;
		default :
			BDBG_ERR(("Unknown Channel %d for SCDC Update0 Polling", eChannel)) ;
		}
        BSTD_UNUSED(eChannel);
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_START + ulOffset, Register) ;
}


BERR_Code BHDM_AUTO_I2C_EnableWriteChannel(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel)
{
	/* TODO Debug for now */
	BDBG_WRN(("Enable/Start Write transaction on Auto i2c Channel %d... ", eChannel)) ;
	BHDM_AUTO_I2C_P_EnableChannelWrite(hHDMI, eChannel) ;
	return 0 ;
}
#endif


/* Read/store the  data read by the auto i2c circuit */
static void BHDM_AUTO_I2C_P_ReadData_isr(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, RegAddr, ulOffset ;
	uint32_t autoI2cChxOffset ;

	BDBG_ENTER(BHDM_AUTO_I2C_P_ReadData_isr) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	autoI2cChxOffset =  BHDM_AUTO_I2C_P_NUM_CHX_REGISTERS * hHDMI->AutoI2CChannel_Read ;
	RegAddr = BCHP_HDMI_TX_AUTO_I2C_CH0_RD_0 + ulOffset + autoI2cChxOffset ;

	/* max of 8 bytes auto reads are supported */
	Register = BREG_Read32(hRegister, RegAddr) ;
		hHDMI->AutoI2CBuffer[0] = (uint8_t) Register & 0x0000000FF ;
		hHDMI->AutoI2CBuffer[1] = (uint8_t) ((Register & 0x0000FF00) >> 8) ;
		hHDMI->AutoI2CBuffer[2] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;
		hHDMI->AutoI2CBuffer[3] = (uint8_t) ((Register & 0xFF000000) >> 24) ;

	Register = BREG_Read32(hRegister, RegAddr + 4) ;
		hHDMI->AutoI2CBuffer[4] = (uint8_t) Register & 0x0000000FF ;
		hHDMI->AutoI2CBuffer[5] = (uint8_t) ((Register & 0x0000FF00) >> 8) ;
		hHDMI->AutoI2CBuffer[6] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;
		hHDMI->AutoI2CBuffer[7] = (uint8_t) ((Register & 0xFF000000) >> 24) ;

	switch (hHDMI->ePendingReadType)
	{
	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_TMDS_Config :
	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Scrambler_Status :
	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Config_0 :
	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Status_Flags :
	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Err_Det :
	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Test_Config_0 :
		BHDM_SCDC_P_ProcessUpdate_isr(hHDMI) ;
		break ;

	case BHDM_AUTO_I2C_P_READ_DATA_eEDID :
		break ;

	case BHDM_AUTO_I2C_P_READ_DATA_eNone :
	default :
		BDBG_WRN(("No or unknown pending Read %d", hHDMI->ePendingReadType)) ;
	}


	BDBG_MSG(("Auto I2C Channel: %d Read Auto I2c Data", hHDMI->AutoI2CChannel_Read)) ;
	BDBG_MSG(("from address 0x%x = 0x%x", Register, RegAddr)) ;

	BDBG_LEAVE(BHDM_AUTO_I2C_P_ReadData_isr) ;

}


/* Get the  data read by the auto i2c circuit */
/* TODO Review this function/purpose of  GetReadData; expecting to use for Read channel vs. Polling */
BERR_Code BHDM_AUTO_I2C_P_GetReadData(const BHDM_Handle hHDMI,
 uint8_t *buffer, uint8_t length)
{
	BERR_Code rc = BERR_SUCCESS ;

	BREG_Handle hRegister ;
	uint32_t Register, RegAddr, ulOffset ;
	uint32_t autoI2cChxOffset ;

	BDBG_ENTER(BHDM_AUTO_I2C_P_GetReadData) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	if (length > BHDM_AUTO_I2C_P_MAX_READ_BYTES)
	{
		BDBG_ERR(("Auto I2C Read length %d is greater than max value of %d",
			length, BHDM_AUTO_I2C_P_MAX_READ_BYTES)) ;
		BDBG_ERR(("Will return %d bytes only", BHDM_AUTO_I2C_P_MAX_READ_BYTES)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
	}

	autoI2cChxOffset =  BHDM_AUTO_I2C_P_NUM_CHX_REGISTERS * hHDMI->AutoI2CChannel_Read ;
	RegAddr = BCHP_HDMI_TX_AUTO_I2C_CH0_RD_0 + ulOffset + autoI2cChxOffset ;

	/* max of 8 bytes auto reads are supported */
	Register = BREG_Read32(hRegister, RegAddr) ;
		buffer[0] = (uint8_t) Register & 0x0000000FF ;
		buffer[1] = (uint8_t) ((Register & 0x0000FF00) >> 8) ;
		buffer[2] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;
		buffer[3] = (uint8_t) ((Register & 0xFF000000) >> 24) ;

	Register = BREG_Read32(hRegister, RegAddr + 4) ;
		buffer[4] = (uint8_t) Register & 0x0000000FF ;
		buffer[5] = (uint8_t) ((Register & 0x0000FF00) >> 8) ;
		buffer[6] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;
		buffer[7] = (uint8_t) ((Register & 0xFF000000) >> 24) ;

	BDBG_MSG(("Auto I2C Channel: %d Read Auto I2c Data", hHDMI->AutoI2CChannel_Read)) ;
	BDBG_MSG(("from address 0x%x = 0x%x", Register, RegAddr)) ;

	BDBG_LEAVE(BHDM_AUTO_I2C_P_GetReadData) ;

	return rc ;
}


static void BHDM_AUTO_I2C_P_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] not used */
)
{
	BHDM_Handle hHDMI ;

	BDBG_ENTER(BHDM_AUTO_I2C_P_isr) ;

	hHDMI = (BHDM_Handle) pParam1 ;

	switch (parm2)
	{
	case MAKE_AUTO_I2C_INTR_ENUM(ScdcUpdate) :
		BHDM_SCDC_P_ReadUpdate0Data_isr(hHDMI) ;
		BDBG_MSG(("AUTO I2C SCDC Update")) ;
		BKNI_SetEvent_isr(hHDMI->AutoI2CEvent_ScdcUpdate) ;
		break ;

	case MAKE_AUTO_I2C_INTR_ENUM(Hdcp22RxStatus) :
		BHDM_HDCP_P_ReadHdcp22RxStatus_isr(hHDMI) ;
		BDBG_MSG(("AUTO I2C HDCP Rx Status Update")) ;
		BKNI_SetEvent_isr(hHDMI->AutoI2CEvent_Hdcp22RxStatusUpdate) ;
		break ;

	case MAKE_AUTO_I2C_INTR_ENUM(Write) :
		BKNI_SetEvent_isr(hHDMI->AutoI2CEvent_Write) ;
		break ;

	case MAKE_AUTO_I2C_INTR_ENUM(Read) :
		BHDM_AUTO_I2C_P_ReadData_isr(hHDMI) ;
		BKNI_SetEvent_isr(hHDMI->AutoI2CEvent_Read) ;
		break ;

	default :
		BDBG_ERR(("Unknown Auto I2c Interrupt ID: %d", parm2)) ;
	}

#if BDBG_DEBUG_BUILD
	if (parm2 < MAKE_AUTO_I2C_INTR_ENUM(LAST))
	{
		BDBG_MSG(("I2C transaction completed on CH %d Done", parm2)) ;
	}
#endif

	BDBG_LEAVE(BHDM_AUTO_I2C_P_isr) ;
	return ;
}
#endif
