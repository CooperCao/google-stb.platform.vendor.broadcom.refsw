/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
#include "bhdr.h"
#include "bhdr_priv.h"

#include "bhdr_fe.h"
#include "bhdr_fe_priv.h"

#include "bchp_hdmi_rx_eq_0.h"
#include "bchp_hdmi_rx_shared.h"
#include "bchp_hdmi_rx_fe_shared.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif


BDBG_MODULE(BHDR_FE_PRIV) ;


#define	BHDR_CHECK_RC( rc, func )	          \
do                                                \
{										          \
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										      \
		goto done;							      \
	}										      \
} while(0)


typedef struct BHDR_FE_P_InterruptCbTable
{
	BINT_Id       IntrId;
	int               iParam2;
	bool             enable ; /* debug purposes */
} BHDR_FE_P_InterruptCbTable ;


static const BHDR_FE_P_InterruptCbTable BHDR_FE_P_Intr[MAKE_INTR_FE_ENUM(LAST)] =
{
	/* 16 */   { BCHP_INT_ID_TMR_IRQ_0, BHDR_FE_INTR_eTMR_IRQ_0, true },
	/* 17 */   { BCHP_INT_ID_TMR_IRQ_1, BHDR_FE_INTR_eTMR_IRQ_1, true }
} ;


static const BHDR_FE_P_InterruptCbTable BHDR_FE_P_ChannelIntr0[MAKE_INTR_ENUM(LAST)] =
{
	/* 00 */   { BCHP_INT_ID_HP_CONNECTED_0, BHDR_FE_CHN_INTR_eHPD_CONNECTED, true},
	/* 01 */   { BCHP_INT_ID_HP_REMOVED_0, BHDR_FE_CHN_INTR_eHPD_REMOVED, true},
	/* 02 */   { BCHP_INT_ID_CLOCK_STOP_0, BHDR_FE_CHN_INTR_eCLOCK_STOP_0, true},
	/* 03 */   { BCHP_INT_ID_PLL_UNLOCK_0, BHDR_FE_CHN_INTR_ePLL_UNLOCK_0, true},
	/* 04 */   { BCHP_INT_ID_PLL_LOCK_0, BHDR_FE_CHN_INTR_ePLL_LOCK_0, true},
	/* 05 */   { BCHP_INT_ID_FREQ_CHANGE_0,  BHDR_FE_CHN_INTR_eFREQ_CHANGE_0, true},
} ;

typedef struct BHDR_FE_RangeSetting
{
	uint32_t MIN ;
	uint32_t MAX ;

	/* A Settings */
	uint8_t PLL_VCO_GAIN ;
	uint8_t PLL_PDIV ;
	uint8_t PLL_CP1 ;
	uint8_t PLL_CP ;
	uint8_t PLL_ICP ;
	uint8_t PLL_CZ ;
	uint16_t PLL_NDIV_INT ;
	uint8_t PLL_VCO_SEL ;

	/* B Settings */
	uint8_t FREQSEL ;
	uint8_t PLL_REF_DBL_EN ;
	uint8_t EQ_6G_ENB ;
	uint8_t PLL_RZ ;
	uint8_t PLL_RP ;
	uint8_t SEL_INPUT_DIVIDER ;
	uint8_t STABLE_COUNT ;

	uint16_t HOLD_THRESHOLD  ;
	uint16_t STABLE_THRESHOLD ;
} BHDR_FE_RangeSetting ;

#define BHDR_FE_P_FREQUENCY_RANGES 13
static const BHDR_FE_RangeSetting RangeSettings[BHDR_FE_P_FREQUENCY_RANGES] =
{
	/* Range 0*/
	{
		/* MIN, MAX */ 136113, 125829,
		/* A Settings: PLL_VCO_GAIN,  PLL_PDIV PLL_CP1  PLL_CP  PLL_ICP  PLL_CZ  PLL_NDIV_INT  PLL_VCO_SEL   */
		15, 1, 1, 1, 11, 3, 160, 0,
		/* B Settings: FREQSEL, PLL_REF_DBL_EN, EQ_6G_ENB, PLL_RZ, PLL_RP, SEL_INPUT_DIVIDER, STABLE_COUNT */
		0,  1,  1,  4,  4,  2,  3,
		/* C Settings: STABLE Threshold, HOLD Threshold */
		1814, 452
	} ,

	/* Range 1 */
	{
	 	128689, 94372,
		12, 1, 1, 1, 10, 3, 160, 1,
		4, 1, 1, 6, 4, 3, 3,
		2046, 510
	},

	/* Range 2 */
	{
		95647, 62915,
		15, 1,  1, 1, 11, 3, 80, 0,
		8, 1, 1, 4, 4, 3, 3 ,
		1814, 452
	},

	/* Range 3 */
	{
		64344, 47186,
		12, 1, 1, 1, 31, 3, 80, 1,
		12, 1, 1, 6, 4, 4, 3,
		2046, 510
	} ,

	/* Range 4 */
	{
		47824, 31457,
		15, 1, 1, 1, 6, 3, 40, 0,
		16, 1, 1, 4, 4, 4, 3,
		1814, 452
	} ,

	/* Range 5 */
	{
		32172, 23593,
		12, 1, 1, 1, 31, 3, 80, 1,
		20, 0, 1, 6, 4, 5, 3,
		2046, 510
	} ,

	/* Range 6 */
	{
		23912, 15729,
		15, 1, 1, 1, 6, 3, 40, 0,
		24, 0, 1, 4, 4, 5, 3,
		1806, 451
	} ,

	/* Range 7 */
	{
		16086, 11796,
		10, 2, 1, 1, 6, 3, 80, 1,
		28, 0, 1, 6, 4, 6, 3,
		2038, 508
	} ,

	/* Range 8 */
	{
		11876, 7864,
		15 , 2, 1, 1, 6, 3, 40, 0,
		32, 0, 1, 4, 4, 6, 3,
		1815, 453
	} ,

	/* Range 9 */
	{
		8043, 5898,
		10, 4 , 1, 1, 6, 3, 80, 1,
		36, 0, 1, 6, 4, 7, 3,
		2047, 511
	} ,

	/* Range 10 */
	{
		5918, 5204,
		15, 4, 1, 1, 6, 3, 40, 0,
		40, 0,  1,  4,  4,  7, 3,
		1815, 453
	} ,

	/* Range 11 */
	{
		5204, 3932,
		15, 1, 1, 1, 6, 3, 40, 0,
		40, 0, 0, 4, 4, 8, 3,
		2047, 511
	} ,

	/* Range 12 */
	{
		4022, 2949,
		10, 2, 1, 1, 6, 3, 80, 1 ,
		44, 0, 0, 6, 4, 8, 3,
		1505, 375
	} ,
} ;


static void BHDR_FE_P_Channel_isr(void *pParam1, int parm2) ;


/******************************************************************************
void BHDR_FE_P_Initialize
Summary:  Initialize/Reset Front End registers
*******************************************************************************/
void BHDR_FE_P_Initialize(BHDR_FE_Handle hFrontEnd)
{

	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t RangeRegisterSize ;
	uint32_t RangeOffset ;
	uint8_t i ;


	BDBG_ENTER(BHDR_FE_P_Initialize) ;
	BDBG_OBJECT_ASSERT(hFrontEnd, BHDR_FE_P_Handle) ;

	hRegister = hFrontEnd->hRegister ;

	Register = BREG_Read32(hRegister, BCHP_DVP_HR_TOP_SW_INIT) ;
	Register &= ~BCHP_MASK(DVP_HR_TOP_SW_INIT, RX_0) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_TOP_SW_INIT, Register) ;

	Register = BREG_Read32(hRegister, BCHP_DVP_HR_TOP_SW_INIT) ;
	Register &= ~ BCHP_MASK(DVP_HR_TOP_SW_INIT, FE_0) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_TOP_SW_INIT, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0) ;
		Register &= ~ (
			  BCHP_MASK(HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0, UPDATE_THRESHOLD)
			| BCHP_MASK(HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0, SYMBOL_LOCK_THRESHOLD)) ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0, UPDATE_THRESHOLD, 6)
			| BCHP_FIELD_DATA(HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0, SYMBOL_LOCK_THRESHOLD, 6) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_SHARED_DIGITAL_FRONT_END_CFG_0,  Register) ;

	/* Initialize Shared Setings */
	RangeRegisterSize =
		  BCHP_HDMI_RX_FE_SHARED_REF_RANGE_1_MIN
		- BCHP_HDMI_RX_FE_SHARED_REF_RANGE_0_MIN ;
	BDBG_MSG(("RangeRegisterSize: %d", RangeRegisterSize)) ;

	for (i = 0 ; i < BHDR_FE_P_FREQUENCY_RANGES; i++ )
	{
		RangeOffset = i * RangeRegisterSize ;
		BDBG_MSG(("BCHP_HDMI_RX_FE_SHARED_REF_RANGE_0_MIN address: %x", BCHP_HDMI_RX_FE_SHARED_REF_RANGE_0_MIN + RangeOffset)) ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_SHARED_REF_RANGE_0_MIN + RangeOffset) ;
			Register &= ~ BCHP_MASK(HDMI_RX_FE_SHARED_REF_RANGE_0_MIN, RANGE_0_MIN) ;
			Register |= BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_REF_RANGE_0_MIN, RANGE_0_MIN, RangeSettings[i].MIN) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_SHARED_REF_RANGE_0_MIN + RangeOffset, Register) ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_SHARED_REF_RANGE_0_MAX + RangeOffset) ;
			Register &= ~ BCHP_MASK(HDMI_RX_FE_SHARED_REF_RANGE_0_MAX, RANGE_0_MAX) ;
			Register |= BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_REF_RANGE_0_MAX, RANGE_0_MAX, RangeSettings[i].MAX) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_SHARED_REF_RANGE_0_MAX + RangeOffset, Register) ;


		/* RANGE 0A Settings */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_SHARED_SETTING_RANGE_0A + RangeOffset) ;
			Register &= ~ (
				  BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_VCO_GAIN)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_PDIV)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CP1)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CP)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_ICP)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CP1)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CP)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CZ)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_NDIV_INT)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_VCO_SEL)) ;

			Register |=
				  BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_VCO_GAIN, RangeSettings[i].PLL_VCO_GAIN)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_PDIV, RangeSettings[i].PLL_PDIV)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CP1, RangeSettings[i].PLL_CP1)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CP, RangeSettings[i].PLL_CP)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_ICP, RangeSettings[i].PLL_ICP)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CP1, RangeSettings[i].PLL_CP1)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CP, RangeSettings[i].PLL_CP)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_CZ, RangeSettings[i].PLL_CZ)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_NDIV_INT, RangeSettings[i].PLL_NDIV_INT)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0A, PLL_VCO_SEL, RangeSettings[i].PLL_VCO_SEL) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_SHARED_SETTING_RANGE_0A + RangeOffset, Register) ;


		/* RANGE 0B Settings */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_SHARED_SETTING_RANGE_0B + RangeOffset) ;
			Register &= ~ (
				  BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, FREQSEL)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, PLL_REF_DBL_EN)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, EQ_6G_ENB)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, PLL_RZ)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, PLL_RP)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, SEL_INPUT_DIVIDER)
				| BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, STABLE_COUNT) ) ;

			Register |=
				  BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, FREQSEL, RangeSettings[i].FREQSEL)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, PLL_REF_DBL_EN, RangeSettings[i].PLL_REF_DBL_EN)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, EQ_6G_ENB, RangeSettings[i].EQ_6G_ENB)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, PLL_RZ, RangeSettings[i].PLL_RZ)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, PLL_RP, RangeSettings[i].PLL_RP)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, SEL_INPUT_DIVIDER, RangeSettings[i].SEL_INPUT_DIVIDER)
				| BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0B, STABLE_COUNT, RangeSettings[i].STABLE_COUNT) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_SHARED_SETTING_RANGE_0B + RangeOffset, Register) ;


		/* RANGE 0C Settings */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_SHARED_SETTING_RANGE_0C + RangeOffset) ;
			Register &= ~ BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0C, HOLD_THRESHOLD) ;
			Register &= ~ BCHP_MASK(HDMI_RX_FE_SHARED_SETTING_RANGE_0C, STABLE_THRESHOLD) ;
			Register |= BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0C, HOLD_THRESHOLD, RangeSettings[i].HOLD_THRESHOLD) ;
			Register |= BCHP_FIELD_DATA(HDMI_RX_FE_SHARED_SETTING_RANGE_0C, STABLE_THRESHOLD, RangeSettings[i].STABLE_THRESHOLD) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_SHARED_SETTING_RANGE_0C + RangeOffset, Register) ;
	}

	BDBG_LEAVE(BHDR_FE_P_Initialize) ;

}


/******************************************************************************
void BHDR_FE_P_OpenChannel
Summary:  Open Frontend for HDMI Rx channel
*******************************************************************************/
void BHDR_FE_P_OpenChannel(
	BHDR_FE_ChannelHandle hFeChannel) /* [out] Created channel handle */
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;

	BDBG_ENTER(BHDR_FE_P_OpenChannel) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle);

	hRegister = hFeChannel->hRegister ;
	ulOffset = hFeChannel->ulOffset ;


	/* allow the Hot Plug to power down the PHY */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1 + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1,
		ALLOW_HOTPLUG_TO_POWER_DOWN_PHY) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1,
		ALLOW_HOTPLUG_TO_POWER_DOWN_PHY, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1 + ulOffset, Register) ;

	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL  + ulOffset) ;
	Register |= BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, ENABLE_CLOCK_STOPPED_AS_HOTPLUG) ; /* 1 */
#if BHDR_CONFIG_HDCP_REPEATER
	/* HDMI_TODO Hot Plug bypass must be disabled to force HPD signal */
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, HOTPLUG_BYPASS) ;
#endif
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0 + ulOffset) ;
		Register &= ~ (
			  BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0, MON_CORRECT_EN)
			| BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0, MON_MARGIN_VAL))  ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0, MON_CORRECT_EN, 1)
			| BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0, MON_MARGIN_VAL, 0x51) ;
	BREG_Write32(hRegister,
		BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH0 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1 + ulOffset) ;
		Register &= ~ (
			  BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1, MON_CORRECT_EN)
			| BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1, MON_MARGIN_VAL))  ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1, MON_CORRECT_EN, 1)
			| BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1, MON_MARGIN_VAL, 0x51) ;
	BREG_Write32(hRegister,
		BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH1 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2 + ulOffset) ;
		Register &= ~ (
			  BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2, MON_CORRECT_EN)
			| BCHP_MASK(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2, MON_MARGIN_VAL))  ;
		Register |=
			  BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2, MON_CORRECT_EN, 1)
			| BCHP_FIELD_DATA(HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2, MON_MARGIN_VAL, 0x51) ;
	BREG_Write32(hRegister,
		BCHP_HDMI_RX_EQ_0_RX_FREQ_MON_CONTROL1_CH2 + ulOffset, Register) ;

	/* Enable Charge Pump Program  down bleeding current 	*/
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_2 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_2 , CPP) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_2 , CPP, 0x80) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_2  + ulOffset, Register) ;

	/* update RDB defaults */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_0 + ulOffset) ;
		Register &= ~ (
		    BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, ENA_VCO_CLK)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, VCO_CONT_EN)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, VCO_POST_DIV2)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, VCO_FB_DIV2)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, PLL_CTRL_0_RESERVED)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, VC_HIGH)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, VC_LOW)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, VC_RANGE_EN)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, MASH11_MODE)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, RM_MODE)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, CODE_VCOCAL)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, BYPASS_MODE)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, BYPASS_DAC)
		  | BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, WATCHDOG_DISABLE)) ;

		Register |=
		    BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, ENA_VCO_CLK, 1)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, VCO_CONT_EN, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, VCO_POST_DIV2, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, VCO_FB_DIV2, 1)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, PLL_CTRL_0_RESERVED, 0 )
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, VC_HIGH, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, VC_LOW, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, VC_RANGE_EN, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, MASH11_MODE, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, RM_MODE, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, CODE_VCOCAL, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, BYPASS_MODE, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, BYPASS_DAC, 0)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, WATCHDOG_DISABLE, 0) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_0 + ulOffset, Register) ;



	/**********************/
	/*   CHANNEL_CTRL_0   */
	/**********************/
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_CHANNEL_CTRL_0 + ulOffset) ;
		Register &= ~ (
		    BCHP_MASK(HDMI_RX_FE_0_CHANNEL_CTRL_0, DATARATE_MAP)
		  | BCHP_MASK(HDMI_RX_FE_0_CHANNEL_CTRL_0, RCLKRATIO_MAP)) ;

		Register |=
		    BCHP_FIELD_DATA(HDMI_RX_FE_0_CHANNEL_CTRL_0, DATARATE_MAP, 43328)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_CHANNEL_CTRL_0, RCLKRATIO_MAP, 21930) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_CHANNEL_CTRL_0 + ulOffset, Register) ;

	/**********************/
	/*   CHANNEL_CTRL_1   */
	/**********************/
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_CHANNEL_CTRL_1 + ulOffset) ;
		Register &= ~ (
		    BCHP_MASK(HDMI_RX_FE_0_CHANNEL_CTRL_1, SUBRATIO_MAP)
		  | BCHP_MASK(HDMI_RX_FE_0_CHANNEL_CTRL_1, DIVCK124_MAP)) ;

		Register |=
		    BCHP_FIELD_DATA(HDMI_RX_FE_0_CHANNEL_CTRL_1, SUBRATIO_MAP, 65444)
		  | BCHP_FIELD_DATA(HDMI_RX_FE_0_CHANNEL_CTRL_1, DIVCK124_MAP, 21930) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_CHANNEL_CTRL_1 + ulOffset, Register) ;

	/**********************/
	/*   CHANNEL_CTRL_3  */
	/**********************/
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_CHANNEL_CTRL_3 + ulOffset , 0xF0) ;

	BDBG_LEAVE(BHDR_FE_P_OpenChannel) ;
}


/******************************************************************************
void BHDR_FE_P_CloseChannel
Summary: Close HDMI Rx Frontend
*******************************************************************************/
void BHDR_FE_P_CloseChannel(BHDR_FE_ChannelHandle hFeChannel)
{
	BDBG_ENTER(BHDR_FE_P_CloseChannel) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	BDBG_LEAVE(BHDR_FE_P_CloseChannel) ;
}


/******************************************************************************
void BHDR_FE_P_CreateInterrupts
Summary: Create interrupts for Frontend
*******************************************************************************/
void BHDR_FE_P_CreateInterrupts(
	BHDR_FE_Handle hFrontEnd,       /* [in] HDMI Rx handle */
	BHDR_FE_ChannelHandle hFeChannel, /* [out] Created channel handle */
	const BHDR_FE_ChannelSettings  *pChannelSettings) /* [in] default HDMI settings */
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;
	const BHDR_FE_P_InterruptCbTable *pInterrupts ;

	BDBG_ENTER(BHDR_FE_P_CreateInterrupts) ;

	/* Register/enable interrupt callbacks for channel */

	pInterrupts = BHDR_FE_P_ChannelIntr0 ;

	for (i = 0; i < MAKE_INTR_FE_CHN_ENUM(LAST) ; i++)
	{
		BHDR_CHECK_RC( rc, BINT_CreateCallback( &(hFeChannel->hCallback[i]),
			hFrontEnd->hInterrupt, pInterrupts[i].IntrId,
			BHDR_FE_P_Channel_isr, (void *) hFeChannel, i ));

		/* clear interrupt callback */
		BHDR_CHECK_RC(rc, BINT_ClearCallback( hFeChannel->hCallback[i])) ;

		/* skip interrupt if not enabled in table...  */
		if (!pInterrupts[i].enable)
			continue ;

		/* enable interrupts if HPD signal is connected  */
		/* i.e. direct connection or switch with HPD connected */
		if (!pChannelSettings->bHpdDisconnected)
		{
			BHDR_CHECK_RC( rc, BINT_EnableCallback( hFeChannel->hCallback[i] )) ;
		}
	}

done :
	BDBG_LEAVE(BHDR_FE_P_CreateInterrupts) ;
}


/**************************************************************************
void BHDR_FE_P_EnableInterrupts_isr
Summary: Enable/Disable Frontend Interrupts
**************************************************************************/
void BHDR_FE_P_EnableInterrupts_isr(BHDR_FE_ChannelHandle hFeChannel, bool enable)
{
	BERR_Code rc ;
	uint8_t i ;
	const BHDR_FE_P_InterruptCbTable *pInterrupts ;

	BDBG_ENTER(BHDR_FE_P_EnableInterrupts_isr) ;

	/* do not enable/disable interrupts unless HPD signal is disconnected  */
	if (!hFeChannel->settings.bHpdDisconnected)
		return ;

	pInterrupts = BHDR_FE_P_ChannelIntr0 ;

	for (i = 0; i < MAKE_INTR_FE_CHN_ENUM(LAST) ; i++)
	{
		/* clear interrupt callback */
		rc =  BINT_ClearCallback_isr( hFeChannel->hCallback[i]) ;
		if (rc) {BERR_TRACE(rc) ;}

		/* skip interrupt if not enabled in table...  */
		if (!pInterrupts[i].enable)
			continue ;

 		if (enable)
			BINT_EnableCallback_isr( hFeChannel->hCallback[i] ) ;

	   /* never disable hot plug interrupt */
		else if ((i != MAKE_INTR_FE_CHN_ENUM(HPD_CONNECTED))
		&&		 (i != MAKE_INTR_FE_CHN_ENUM(HPD_REMOVED)))
		{
			BINT_DisableCallback_isr( hFeChannel->hCallback[i] ) ;
			BDBG_WRN(("Disable Interrupt %d; ", i)) ;
		}
		else
		{
			BDBG_WRN(("Keep Interrupt %d Enabled", i)) ;
		}
	}
	BDBG_LEAVE(BHDR_FE_P_EnableInterrupts_isr) ;
}


/******************************************************************************
void BHDR_FE_P_ClearHdcpAuthentication_isr
Summary: Clear internal HDCP engine
*******************************************************************************/
static void BHDR_FE_P_ClearHdcpAuthentication_isr(BHDR_FE_ChannelHandle hFeChannel)
{
	BREG_Handle hRegister ;
	uint32_t ulOffset, Register  ;

	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle);
	hRegister= hFeChannel->hRegister ;

	ulOffset = hFeChannel->ulHdrOffset ;

	/* clear HDCP authenticated status in HDMI Rx Core */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG + ulOffset) ;

	Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_DEBUG, CLEAR_RX_AUTHENTICATED_P) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG + ulOffset , Register) ;

	Register |= BCHP_MASK(HDMI_RX_0_HDCP_DEBUG, CLEAR_RX_AUTHENTICATED_P) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG  + ulOffset , Register) ;
}


/******************************************************************************
void BHDR_FE_P_FireHotPlugCb_isr
Summary:  Fire Hot Plug isr callback
*******************************************************************************/
static void BHDR_FE_P_FireHotPlugCb_isr(BHDR_FE_ChannelHandle hFeChannel)
{
#if BHDR_CONFIG_DEBUG_FRONT_END
	BDBG_WRN(("FE_%d RX HOT PLUG update (HPD) : %s ",
		hFeChannel->eChannel,
		hFeChannel->bTxDeviceAttached ? "HIGH" : "LOW")) ;
#endif

	/* inform higher level of Connect/Disconnect interrupt */
	if (hFeChannel->pfHotPlugCallback_isr)
	{
		hFeChannel->pfHotPlugCallback_isr(hFeChannel->pvHotPlugParm1,
			hFeChannel->iHotPlugParm2, &hFeChannel->bTxDeviceAttached) ;
	}
	else
	{
		BDBG_WRN(("FE_%d No HotPlug callback installed...",
			hFeChannel->eChannel)) ;
	}

	/* clear HDCP authenticated status in HDMI Rx Core */
	BHDR_FE_P_ClearHdcpAuthentication_isr(hFeChannel) ;
}


/******************************************************************************
void BHDR_FE_P_Channel_isr
Summary: Handle interrupts from the HDMI core.
*******************************************************************************/
static void BHDR_FE_P_Channel_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] Interrupt ID */
)
{
	BHDR_FE_ChannelHandle hFeChannel  ;
	bool bPllLocked;

	BDBG_ENTER(BHDR_FE_P_Channel_isr) ;
	hFeChannel = (BHDR_FE_ChannelHandle) pParam1 ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	if (hFeChannel->uiHdrSel == BHDR_P_eHdrCoreIdNotAttached)
	{
#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("No HDMI Rx core (not ready) attached to frontend")) ;
#endif
		return ;
	}


	switch (parm2)
	{
	case MAKE_INTR_FE_CHN_ENUM(HPD_CONNECTED) :

		hFeChannel->bTxDeviceAttached = true ;
		BHDR_FE_P_FireHotPlugCb_isr(hFeChannel) ;

		break ;

	case MAKE_INTR_FE_CHN_ENUM(HPD_REMOVED) :

		hFeChannel->bTxDeviceAttached = false  ;
		BHDR_FE_P_FireHotPlugCb_isr(hFeChannel) ;

		break ;


	case MAKE_INTR_FE_CHN_ENUM(PLL_UNLOCK_0) :
#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("FE_%d   PLL -_-UNLOCKED-_-", hFeChannel->eChannel)) ;
#endif
		BHDR_FE_P_GetPllLockStatus_isr(hFeChannel, &bPllLocked) ;

		break ;

	case MAKE_INTR_FE_CHN_ENUM(PLL_LOCK_0) :

#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("FE_%d   PLL ----LOCKED----", hFeChannel->eChannel)) ;
#endif
		BHDR_FE_P_GetPllLockStatus_isr(hFeChannel, &bPllLocked) ;

#if BHDR_CONFIG_RESET_CLOCK_AND_DATA_CHANNELS
		/* enable the count down timer if not already enabled */
	 	BHDR_FE_P_ResetFeDataChannels_isr(hFeChannel) ;
		BHDR_FE_P_ResetFeClockChannel_isr(hFeChannel) ;
#endif

 		break ;

	case MAKE_INTR_FE_CHN_ENUM(FREQ_CHANGE_0) :
		hFeChannel->PreviousPixelClockCount = 0 ;
#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("FE_%d Frequency Changed", hFeChannel->eChannel)) ;
#endif

		break ;


	case MAKE_INTR_FE_CHN_ENUM(CLOCK_STOP_0) :
#if BHDR_CONFIG_DEBUG_FRONT_END
		BDBG_WRN(("FE_%d Clock STOPPED", hFeChannel->eChannel)) ;
#endif
		break ;


	default	:
		BDBG_ERR(("Unknown Interrupt ID:%d", parm2)) ;
	} ;

	/* L2 interrupts are reset automatically */
	BDBG_LEAVE(BHDR_FE_P_Channel_isr) ;
}


/******************************************************************************
void BHDR_FE_P_ResetPixelClockEstimation_isr
Summary: Reset Pixel Clock Estimation circuit
*******************************************************************************/
void BHDR_FE_P_ResetPixelClockEstimation_isr(BHDR_FE_ChannelHandle hFeChannel)
{
	BREG_Handle hRegister ;
	uint32_t Register ;

	BDBG_ENTER(BHDR_FE_P_ResetPixelClockEstimation_isr) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	hRegister = hFeChannel->hRegister;

	Register = BREG_Read32(hRegister, BCHP_DVP_HR_HDMI_FE_0_SW_INIT ) ;
	Register &= ~ (BCHP_MASK(DVP_HR_HDMI_FE_0_SW_INIT, FREQ_EST)) ;

	Register |= BCHP_FIELD_DATA(DVP_HR_HDMI_FE_0_SW_INIT, FREQ_EST, 1) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_FE_0_SW_INIT, Register) ;

	Register &= ~ (BCHP_MASK(DVP_HR_HDMI_FE_0_SW_INIT, FREQ_EST)) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_FE_0_SW_INIT, Register) ;

	BDBG_LEAVE(BHDR_FE_P_ResetPixelClockEstimation_isr) ;
}


/******************************************************************************
void BHDR_FE_P_GetPixelClockStatus_isr
Summary: Reads appropriate FE registers to get Pixel Clock data
*******************************************************************************/
void BHDR_FE_P_GetPixelClockStatus_isr(BHDR_FE_ChannelHandle hFeChannel,
	BHDR_FE_P_PixelClockStatus *ClockStatus)

{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulFeOffset ;
	uint32_t RegAddr ;
	uint8_t Channel ;

	BDBG_ENTER(BHDR_FE_P_GetPixelClockStatus_isr) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	hRegister = hFeChannel->hRegister ;
	ulFeOffset = hFeChannel->ulOffset ;


	Register = BREG_Read32(hRegister, BCHP_DVP_HR_FREQ_MEASURE_CONTROL) ;
		Register &= ~ BCHP_MASK(DVP_HR_FREQ_MEASURE_CONTROL, ENABLE) ;
		Register |= BCHP_FIELD_DATA(DVP_HR_FREQ_MEASURE_CONTROL, ENABLE, 1) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_FREQ_MEASURE_CONTROL, Register) ;

	RegAddr = BCHP_HDMI_RX_FE_0_PIX_CLK_CNT + ulFeOffset ;
	for (Channel = 0 ; Channel < BHDR_FE_P_CLOCK_eChMax ; Channel++)
	{
		Register = BREG_Read32(hRegister, RegAddr) ;
		ClockStatus[Channel].PixelCount =
			BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_PIX_CLK_CNT, PIX_CLK_CNT) ;

		RegAddr = RegAddr + 4 ;

		Register = BREG_Read32(hRegister, BCHP_DVP_HR_FREQ_MEASURE_CONTROL) ;
			Register &= ~ BCHP_MASK(DVP_HR_FREQ_MEASURE_CONTROL, SOURCE) ;
			Register |= BCHP_FIELD_DATA(DVP_HR_FREQ_MEASURE_CONTROL, SOURCE, Channel) ;
		BREG_Write32(hRegister, BCHP_DVP_HR_FREQ_MEASURE_CONTROL, Register) ;

		Register = BREG_Read32(hRegister, BCHP_DVP_HR_FREQ_MEASURE) ;
			ClockStatus[Channel].Frequency =
				BCHP_GET_FIELD_DATA(Register, DVP_HR_FREQ_MEASURE, VALUE) ;
	}

	/* check the clock status stopped vs. running; circuit located in the FE */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_CLK_CNT_STATUS + ulFeOffset) ;
		ClockStatus[BHDR_FE_P_CLOCK_eChRef].bClockStopped =
			BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_CLK_CNT_STATUS, CLOCK_STOPPED_REF) ;

		ClockStatus[BHDR_FE_P_CLOCK_eCh0].bClockStopped =
			BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_CLK_CNT_STATUS, CLOCK_STOPPED_CH0) ;
		ClockStatus[BHDR_FE_P_CLOCK_eCh1].bClockStopped =
			BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_CLK_CNT_STATUS, CLOCK_STOPPED_CH1) ;
		ClockStatus[BHDR_FE_P_CLOCK_eCh2].bClockStopped =
			BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_CLK_CNT_STATUS, CLOCK_STOPPED_CH2) ;


#if BHDR_CONFIG_DEBUG_FRONT_END
	BDBG_WRN(("Frontend Clock Status:")) ;
	for (Channel = 0 ; Channel < BHDR_FE_P_CLOCK_eChMax ; Channel++)
	{
		BDBG_WRN(("Channel_%d  Pixel Count= %d  Frequency= %d kHz Clock Status: %s",
			Channel,
			ClockStatus[Channel].PixelCount, ClockStatus[Channel].Frequency,
			ClockStatus[Channel].bClockStopped ? "Stopped" : "Running")) ;
	}
#endif

	BDBG_LEAVE(BHDR_FE_P_GetPixelClockStatus_isr) ;
}


/******************************************************************************
void BHDR_FE_P_GetPixelClockFromRange_isr
Summary:
*******************************************************************************/
void BHDR_FE_P_GetPixelClockFromRange_isr(BHDR_FE_ChannelHandle hFeChannel,
	uint32_t *EstimatedPixelClockRate)
{

enum
{
	BHDR_FE_P_PLL_CALIBRATION_STATE_eIdle,
	BHDR_FE_P_PLL_CALIBRATION_STATE_eLook,
	BHDR_FE_P_PLL_CALIBRATION_STATE_Wait,
	BHDR_FE_P_PLL_CALIBRATION_STATE_Verify,
	BHDR_FE_P_PLL_CALIBRATION_STATE_Stable,
	BHDR_FE_P_PLL_CALIBRATION_STATE_eHold
};

	BREG_Handle hRegister ;
	uint32_t
		RegAddr, Register,
		ulFeOffset,
		uiInputDivider,
		NumRangeRegisters ,
		uiMeasuredCount ;

	uint8_t
		uiRange, uiState ;

	hRegister = hFeChannel->hRegister ;
	ulFeOffset = hFeChannel->ulOffset ;

	BDBG_ENTER(BHDR_FE_P_GetPixelClockFromRange_isr) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CALIBRATION_STATUS_2) ;
		uiState = BCHP_GET_FIELD_DATA(Register,
			HDMI_RX_FE_0_PLL_CALIBRATION_STATUS_2, STATE) ;

	if (uiState < BHDR_FE_P_PLL_CALIBRATION_STATE_Stable)
	{
		BDBG_MSG(("PLL Calibration State: %#02x; Frequency measurement not available yet...", uiState)) ;
		goto done ;
	}

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_STATUS_2 + ulFeOffset) ;
	uiRange = BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_ANALOG_STATUS_2, RANGE_SETTING) ;

	if (uiRange > 12)
	{
		BDBG_ERR(("*** Undefined Frequency Range ***")) ;
	}

	/* calculate the number of registers that define a range */
	NumRangeRegisters =
		BCHP_HDMI_RX_FE_SHARED_SETTING_RANGE_1B -
		BCHP_HDMI_RX_FE_SHARED_SETTING_RANGE_0B ;

	/* get the corresponding RegAddr for the range we are in */
	RegAddr = BCHP_HDMI_RX_FE_SHARED_SETTING_RANGE_0B + NumRangeRegisters * uiRange ;

	Register = BREG_Read32(hRegister, RegAddr) ;
		uiInputDivider = BCHP_GET_FIELD_DATA(Register,
			HDMI_RX_FE_SHARED_SETTING_RANGE_0B, SEL_INPUT_DIVIDER) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CALIBRATION_STATUS_1) ;
		uiMeasuredCount = BCHP_GET_FIELD_DATA(Register,
			HDMI_RX_FE_0_PLL_CALIBRATION_STATUS_1, CAPTURE_MEAS) ;

#if BHDR_CONFIG_DEBUG_FRONT_END
	BDBG_WRN(("RANGE_nB Register Address %x InputDivider",
		RegAddr, uiInputDivider)) ;

	BDBG_WRN(("FreqRange: %d CalibrationState %#02x; Measured Count = %d InputDivider: %d",
		uiRange, uiState, uiMeasuredCount, uiInputDivider)) ;
#endif


	switch (uiInputDivider)
	{
	case  0 : uiInputDivider =     1 ;  break ;
	case  1 : uiInputDivider =     2 ;  break ;
	case  2 : uiInputDivider =     4 ;  break ;
	case  3 : uiInputDivider =     8 ;  break ;
	case  4 : uiInputDivider =    16 ;  break ;
	case  5 : uiInputDivider =    32 ;  break ;
	case  6 : uiInputDivider =    64 ;  break ;
	case  7 : uiInputDivider =   128 ;  break ;
	case  8 : uiInputDivider =   256 ;  break ;
	case  9 : uiInputDivider =   512 ;  break ;
	case 10 : uiInputDivider =  1024 ;  break ;
	case 11 : uiInputDivider =  2048 ;  break ;
	case 12 : uiInputDivider =  4096 ;  break ;
	case 13 : uiInputDivider =  8192 ;  break ;
	case 14 : uiInputDivider = 16384 ;  break ;
	case 15 : uiInputDivider = 32678 ;  break ;
	default :
		{
			BDBG_ERR(("Unknown Input Divider; Freq Estimation may be incorrect")) ;
			uiInputDivider = 1 ;
		}
	}

	*EstimatedPixelClockRate = (uint64_t) 108000000 * 8192 * uiInputDivider / uiMeasuredCount ;
	*EstimatedPixelClockRate = *EstimatedPixelClockRate / 1000 ;

#if BHDR_CONFIG_DEBUG_FRONT_END
	BDBG_WRN(("Frequency %d kHz",  *EstimatedPixelClockRate)) ;
#endif

done :
	BDBG_LEAVE(BHDR_FE_P_GetPixelClockFromRange_isr) ;
}


/******************************************************************************
void BHDR_FE_P_SetHotPlug
Summary:  Set/force Hotplug signal high/low
*******************************************************************************/
void BHDR_FE_P_SetHotPlug(BHDR_FE_ChannelHandle hFeChannel, BHDR_HotPlugSignal eHotPlugSignal)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulFeOffset  ;

	BDBG_ENTER(BHDR_FE_P_SetHotPlug) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle) ;

	hRegister= hFeChannel->hRegister ;
	ulFeOffset = hFeChannel->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulFeOffset) ;

	Register &= ~ (
		  BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, HOTPLUG_BYPASS)
		| BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT)
		| BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT_VALUE))  ;

	Register |=
		  BCHP_FIELD_DATA(HDMI_RX_FE_0_HOTPLUG_CONTROL, HOTPLUG_BYPASS, eHotPlugSignal)
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT, !eHotPlugSignal)
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_HOTPLUG_CONTROL, OVERRIDE_HOTPLUG_OUT_VALUE, eHotPlugSignal) ;


	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulFeOffset, Register) ;

	BDBG_LEAVE(BHDR_FE_P_SetHotPlug) ;
}


/******************************************************************************
Summary:
*******************************************************************************/
void BHDR_FE_P_PowerResourceAcquire_DVP_HR(BHDR_FE_Handle hFrontEnd)
{
	BDBG_ENTER(BHDR_FE_P_PowerResourceAcquire_DVP_HR) ;
	BDBG_OBJECT_ASSERT(hFrontEnd, BHDR_FE_P_Handle) ;

#if BHDR_CONFIG_DEBUG_HDR_PWR
	BDBG_WRN(("Acquire HDMI_RX0_CLK Resource at line %d", __LINE__)) ;
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_CLK
	BCHP_PWR_AcquireResource(hFrontEnd->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_CLK);
#endif

	BDBG_LEAVE(BHDR_FE_P_PowerResourceAcquire_DVP_HR) ;
}


/******************************************************************************
Summary:
*******************************************************************************/
void BHDR_FE_P_PowerResourceRelease_DVP_HR(BHDR_FE_Handle hFrontEnd)
{
	BDBG_ENTER(BHDR_FE_P_PowerResourceRelease_DVP_HR) ;
	BDBG_OBJECT_ASSERT(hFrontEnd, BHDR_FE_P_Handle) ;

#if BHDR_CONFIG_DEBUG_HDR_PWR
	BDBG_WRN(("Release HDMI_RX0_CLK Resource at line %d", __LINE__)) ;
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_CLK
	BCHP_PWR_ReleaseResource(hFrontEnd->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_CLK);
#endif

	BDBG_LEAVE(BHDR_FE_P_PowerResourceRelease_DVP_HR) ;
}


/******************************************************************************
Summary:
*******************************************************************************/
void BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE(BHDR_FE_ChannelHandle hFeChannel)
{
	BDBG_ENTER(BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle);

#if BHDR_CONFIG_DEBUG_HDR_PWR
	BDBG_WRN(("Acquire HDMI_RX0_PHY Resource at line %d", __LINE__)) ;
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_PHY
	BCHP_PWR_AcquireResource(hFeChannel->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_PHY);
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_SRAM
	BCHP_PWR_AcquireResource(hFeChannel->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_SRAM);
#endif

	BDBG_LEAVE(BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE) ;
}


/******************************************************************************
Summary:
*******************************************************************************/
void BHDR_FE_P_PowerResourceRelease_HDMI_RX_FE(BHDR_FE_ChannelHandle hFeChannel)
{
	BDBG_ENTER(BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE) ;
	BDBG_OBJECT_ASSERT(hFeChannel, BHDR_FE_P_ChannelHandle);

#if BHDR_CONFIG_DEBUG_HDR_PWR
	BDBG_WRN(("Release HDMI_RX0_PHY Resource at line %d", __LINE__)) ;
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_PHY
	BCHP_PWR_ReleaseResource(hFeChannel->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_PHY);
#endif
#if BCHP_PWR_RESOURCE_HDMI_RX0_SRAM
	BCHP_PWR_ReleaseResource(hFeChannel->hChip, BCHP_PWR_RESOURCE_HDMI_RX0_SRAM);
#endif

	BDBG_LEAVE(BHDR_FE_P_PowerResourceAcquire_HDMI_RX_FE) ;
}

