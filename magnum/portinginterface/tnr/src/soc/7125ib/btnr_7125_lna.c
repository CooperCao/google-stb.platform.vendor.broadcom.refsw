/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "breg_mem.h"
#include "bchp.h"
#include "btnr_7125ib_priv.h"
#include "btnr_7125_tune.h"
#include "bchp_bscd.h"
#include "bchp_bcm3412_regs.h"
#include "breg_i2c.h"
#include "bchp_ds_tuner_ref_0.h"


/*registers needed for this function */
BDBG_MODULE(btnr_7125_LNA);
#define BCHP_BCM341X_regs_LNA1_CNTL_MODE_SHIFT     BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_FB_PUP_SHIFT
#define BCHP_BCM341X_regs_LNA1_CNTL_MODE_MASK (BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_FB_PUP_MASK | BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_FB_CURRENT_MASK | BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_TAIL3_MASK | BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_TAIL2_MASK |\
	BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_TAIL2_MASK | BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_TAIL1_MASK | BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_CSC3_MASK | BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_CSC2_MASK | BCHP_BCM341X_regs_LNA1_CNTL_STAGE1_CSC1_MASK)

/****************************************************************************************************/
/* START OF LNA FUNCTIONS MAINTAINED BY PI **********************************************************/
/****************************************************************************************************/

void BTNR_7125_P_LNA_WriteRegister(BTNR_7125_Handle, uint8_t, uint32_t);
uint32_t BTNR_7125_P_LNA_ReadRegister(BTNR_7125_Handle, uint8_t);

#define BTNR_7125_P_LNA_ReadRegisterField(h, Register, Field) \
  ((((BTNR_7125_P_LNA_ReadRegister(h, BCHP_##Register)) & BCHP_MASK(Register,Field)) >> \
  BCHP_SHIFT(Register,Field)))

#define BTNR_7125_P_LNA_WriteRegisterField(h, Register, Field, Data) \
  BTNR_7125_P_LNA_WriteRegister(h, BCHP_##Register, ((BTNR_7125_P_LNA_ReadRegister(h, BCHP_##Register) & \
  ~((uint32_t)BCHP_MASK(Register, Field))) | \
  BCHP_FIELD_DATA(Register, Field, Data)))

void BTNR_7125_P_Test_LNA(BTNR_7125_Handle h)
{
	uint32_t  reg;

	BTNR_7125_P_LNA_WriteRegister(h,  BCHP_BCM341X_regs_LNA1_CNTL, 0x00);
	BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_LNA1_CNTL, STAGE1_CSC3, 1);
	reg = BTNR_7125_P_LNA_ReadRegisterField(h, BCM341X_regs_LNA1_CNTL, STAGE1_CSC3);
	BDBG_MSG(("we are reading address 0x34 = %0x", reg));

	reg = BTNR_7125_P_LNA_ReadRegister(h, BCHP_BCM341X_regs_LNA1_CNTL);
	BDBG_MSG(("we are reading address 0x34 = %0x", reg));

}


/*---------------------------------------------------------------------------
* Function: BTNR_7125_P_LNA_WriteRegister
*---------------------------------------------------------------------------*/
void BTNR_7125_P_LNA_WriteRegister(BTNR_7125_Handle h, uint8_t registerAddress, uint32_t value)
{	
	uint8_t buf[8];
			
	buf[0]  = (char)(value & 0xFF);
	buf[1]  = (char)((value >> 8) & 0xFF);
	buf[2]  = (char)((value >> 16) & 0xFF);
	buf[3]  = (char)((value >> 24) & 0xFF);

	if (NULL==h->hI2cLNA) {
		BDBG_ERR(("%s() BTNR_7125 direct LNA control not enabled.",__FUNCTION__));
	}
	else{
		BREG_I2C_Write(h->hI2cLNA, (uint16_t)(h->pTnrModeData->BBS_Cable_LNA_Address), (registerAddress >> 2), &buf[0], 4);
	}
}
/*---------------------------------------------------------------------------
* Function: BTNR_7125_P_LNA_ReadRegister
*---------------------------------------------------------------------------*/
uint32_t BTNR_7125_P_LNA_ReadRegister(BTNR_7125_Handle h, uint8_t registerAddress )
{

	uint32_t reg = 0;
	uint8_t	 buf[5];

	if (NULL==h->hI2cLNA) {
		BDBG_ERR(("%s() BTNR_7125 direct LNA control not enabled.",__FUNCTION__));
	}
	else{
		BREG_I2C_Read(h->hI2cLNA, (uint16_t)(h->pTnrModeData->BBS_Cable_LNA_Address), (registerAddress >> 2), &buf[0], 4);
		reg = 	 buf[0];
		reg |= 	 buf[1] << 8;
		reg |=	 buf[2] << 16;
		reg |=	 buf[3] << 24;
	}

	return reg;
}


/****************************************************************************************************/
/* END OF LNA FUNCTIONS MAINTAINED BY PI ************************************************************/
/****************************************************************************************************/
/****************************************************************************************************/
/* START OF LNA FUNCTIONS MAINTAINED BY TUNING CODE *************************************************/
/****************************************************************************************************/

/****************************************************************************************************
 * BTNR_7125_P_Init_LNA()  This routine initializes the external LNA
 ****************************************************************************************************/
void BTNR_7125_P_Init_LNA(BTNR_7125_Handle h)
{
	BTNR_OFFON_t BCM3410_Auto;
	uint32_t BCM3410_Gain_State;
	uint32_t LNA_chip_id;

	uint8_t high = 63;
	uint8_t low = 0;
	uint8_t mid = 31;

	int i;
	int PVT_COMP = 0;

	/*Set the LNA to Fixed Gain or Auto mode based on the DS setting*/
	BCM3410_Auto = (BTNR_OFFON_t)h->pTnrModeData->BBS_DS_BCM3410_Auto;
	BCM3410_Gain_State = h->pTnrModeData->BBS_DS_BCM3410_Gain_State;

	/* WE SHOULD NOT WRITE TO LNA ABOVE THIS LINE */

	LNA_chip_id = BTNR_7125_P_LNA_ReadRegisterField(h,  BCM341X_regs_CHIP_ID, CHIP_PART_NUMBER);
	if (LNA_chip_id == 0x3410)
	{
		h->pTnrModeData->LNA_ChipID = BTNR_LNAID_3410;
		BDBG_MSG(("LNA Chip ID is %x",LNA_chip_id));
	}
	else if (LNA_chip_id == 0x3412)
	{
		h->pTnrModeData->LNA_ChipID = BTNR_LNAID_3412;
		BDBG_MSG(("LNA Chip ID is %x",LNA_chip_id));
	}
	else
	{
		BDBG_WRN(("LNA Chip ID is INVALID received value is %x",LNA_chip_id));;
	}
	 
	BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_MWC_CNTL, SM_RESET, 0x1);   
	BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_MWC_CNTL, SM_RESET, 0x0);

	/* Change default values */
	BTNR_7125_P_LNA_WriteRegister(h, BCHP_BCM341X_regs_MWC_CNTL, 0x3a252000); 
	BTNR_7125_P_LNA_WriteRegister(h, BCHP_BCM341X_regs_MWC_PM_CNTL, 0x101b0100); /* Change default MWC_LO, enable PM, select Poly High */

	if( (LNA_chip_id==0x3412) || (LNA_chip_id==0x3406) || (LNA_chip_id==0x3413) )  /* chip with superboost mode */
	{ /* chips with superboost mode */
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_PD_OOB_OSC_CNTL, PD_PSET_B, 0x3); /* set TOP */
	}
	else
	{
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_PD_OOB_OSC_CNTL, PD_PSET_B, 0x4);  /* set TOP to -23dB */
	}
	BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_PD_OOB_OSC_CNTL, PD_BO, 0x1); 

	for (i=0; i<6; i++)
	{
	
		  /* BCM93378.Bcm3410.regset0.MWC_PM_CNTL.PM_VREF = CInt(mid) */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_MWC_PM_CNTL, PM_VREF, mid);
		  BKNI_Sleep(2);
		  /* BCM93378.Bcm3410.regset0.MWC_PM_CNTL.COMP_LAT_EN = &h0 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_MWC_PM_CNTL, COMP_LAT_EN, 0);
		  /* BCM93378.Bcm3410.regset0.MWC_PM_CNTL.COMP_LAT_EN = &h1 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_MWC_PM_CNTL, COMP_LAT_EN, 1);
		  /* PVT_COMP = CInt(BCM93378.Bcm3410.regset0.STATUS_READ2.COMP_OUT) */
		  PVT_COMP = BTNR_7125_P_LNA_ReadRegisterField(h, BCM341X_regs_STATUS_READ2, COMP_OUT);
		
		  if (PVT_COMP == 1)
		  {
			 low = mid+1;
			 mid = ((high+low)/2);
		  }
		  else
		  {
			 high = mid-1;
			 mid = ((high+low)/2);
		  }
		  /* BDBG_MSG(("Iteration %d 3410 PM_VREF low = %d", (i+1) , low)); */
		  /* BDBG_MSG(("Iteration %d 3410 PM_VREF mid = %d", (i+1) , mid)); */
		  /* BDBG_MSG(("Iteration %d 3410 PM_VREF high = %d", (i+1) , high)); */
	}
	
	/* BDBG_MSG(("3410 PM_VREF mid = %d", mid)); */
	
	if (mid > 36)
	{
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE1_RCAL = &h00  */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 0);
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE2_RCAL = &h01 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 1);
	   }
	else if (mid > 31)
	{
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE1_RCAL = &h01 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 1);
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE2_RCAL = &h03 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 3);
	   }
	else if (mid > 26)
	{
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE1_RCAL = &h03 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 3);
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE2_RCAL = &h03 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 3);
	   }
	else if (mid > 21)
	{
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE1_RCAL = &h05 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 5);
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE2_RCAL = &h03 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 3);
	   }
	else
	{
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE1_RCAL = &h07 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE1_RCAL, 7);
		  /* bcm93378.Bcm3410.regset0.BG_LNA2_RCAL_CNTL.STAGE2_RCAL = &h05 */
		  BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE2_RCAL, 5);
	}
	   
	/* saved power for single output LNA. Power down the 2nd output */
	/*if( (LNA_chip_id==0x3410) || (LNA_chip_id==0x3408)|| (LNA_chip_id==0x3413) )
	    BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_BG_LNA2_RCAL_CNTL, 15, 15, 1); */
	
	/*BCM93378.Bcm3410.regset0.MWC_PM_CNTL.PM_PD = &h1	' Turn off the process monitor */
	BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_MWC_PM_CNTL, PM_PD, 1);

	if(BCM3410_Auto == BTNR_OFF)
	{
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_LNA_ADDR, STG1_GAIN_I2C, BCM3410_Gain_State);		 
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_SM_BYPASS, DEC_LAT_ENB_STG1_I2C, 0x1);		  
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_SM_BYPASS, DEC_LAT_ENB_STG1_I2C, 0x0);	 	
	}
	   
	if( (LNA_chip_id==0x3412) || (LNA_chip_id==0x3413) )  /*chip with superboost mode */
	{
		/*Bcm93382.bcm3412.regset0.SM_BYPASS.ATE = &h0 */
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_SM_BYPASS, ATE, 0x0);	 	
	}
	   
	if( (LNA_chip_id==0x3412) || (LNA_chip_id==0x3406)|| (LNA_chip_id==0x3413) )  /* chip with superboost mode */
	{
		/*Bcm93382.bcm3412.regset0.MWC_CNTL.SM_RESET = &h1 */
		BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_MWC_CNTL, SM_RESET, 0x01);
		/*Bcm93382.bcm3412.regset0.MWC_CNTL.SM_RESET = &h0 */
		BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_MWC_CNTL, SM_RESET, 0x00);
	}
	   
	/* setup RF splitter gain */
	/*tgt_ds_tnr_ref.SPLIT_00.RF_ATT =&h0& */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 0, 0);
	
	#if (BCM_STB_OOB_SUPPORT)
		/*Power up 3410 OOB output pin. Bcm3410.regset0.PD_OOB_OSC_CNTL.OOB_PD = &h0 */
		BTNR_7125_P_LNA_WriteRegisterField(h, BCM341X_regs_PD_OOB_OSC_CNTL, OOB_PD, 0);
	#endif
}

/****************************************************************************************************
 * BTNR_7125_P_Probe_First_LNA()  
 ****************************************************************************************************/
void BTNR_7125_P_Probe_LNA(BTNR_7125_Handle h)
{
	uint8_t	 buf[5];
	BERR_Code ret;
	uint16_t addr[]={0x60,0x6c,0x61, 0x6d, 0x62, 0x6e, 0x63, 0x6f};
	int addr_count = 8;
	int i;

	buf[0]  = 0;
	buf[1]  = 0;
	buf[2]  = 0;
	buf[3]  = 1;
	
	for(i=0; i<addr_count; i++)
	{
		
		ret = BREG_I2C_Read(h->hI2cLNA, addr[i], (BCHP_BCM341X_regs_CHIP_ID >> 2), &buf[0], 4);
		BDBG_MSG(("I2C 0x%x,BREG_I2C_Read=%d\n",addr[i],ret));
		if(ret != BERR_SUCCESS)
		{
			continue;
		}

		ret = BREG_I2C_Write(h->hI2cLNA, addr[i], (BCHP_BCM341X_regs_MWC_CNTL >> 2), &buf[0], 4);
		BDBG_MSG(("I2C 0x%x BREG_I2C_Write=%d\n",addr[i],ret));
		if(ret != BERR_SUCCESS)
		{
			continue;
		}

		BDBG_MSG(("Set LNA I2C 0x%x \n",addr[i]));
		break;	
	}
	if (i < addr_count)
	{
		h->pTnrModeData->BBS_Cable_LNA_Address = addr[i];
	}
	else
		BDBG_WRN(("None of the LNA address work\n"));
	
}

/****************************************************************************************************
 * BTNR_7125_P_Set_LNA_Boost()  This routine controls the boost function of the external LNA
 ****************************************************************************************************/
void BTNR_7125_P_Set_LNA_Boost(BTNR_7125_Handle h)
{
	BTNR_OFFON_t LNA_Boost_On;

	/*Set Boost in the LNA based on the DS setting if QAM mode or THD setting if DVB-T/ISDB-T mode from the BTNR_P_7125_AcquireSettings_t structure*/
	LNA_Boost_On = (BTNR_OFFON_t)h->pTnrModeData->BBS_DS_LNA_Boost_On;

	if (LNA_Boost_On == BTNR_ON)
	{
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_LNA1_CNTL, STAGE1_GAIN_BOOST, 0x1);
		BDBG_MSG(("LNA_BOOST is ON"));
	}
	else
	{
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_LNA1_CNTL, STAGE1_GAIN_BOOST, 0x0);
		BDBG_MSG(("LNA_BOOST is OFF"));
	}
}

/****************************************************************************************************
 * BTNR_7125_P_Set_LNA_Tilt()  This routine controls the tilt function of the external LNA
 ****************************************************************************************************/
void BTNR_7125_P_Set_LNA_Tilt(BTNR_7125_Handle h)
{
	/*variable passed from BTNR_Handle*/	
	BTNR_OFFON_t LNA_Tilt_On;
	uint32_t LNA_Tilt_Freq;

	/*Set the LNA Tilt and LNA Tilt Frequency based on the DS setting if QAM mode or THD setting if DVB-T/ISDB-T mode from the BTNR_P_7125_AcquireSettings_t structure*/
	LNA_Tilt_On = (BTNR_OFFON_t)h->pTnrModeData->BBS_DS_LNA_Tilt_On;
	LNA_Tilt_Freq = h->pTnrModeData->BBS_DS_LNA_Tilt_Freq;
		
	if ((LNA_Tilt_On == BTNR_ON) && (h->pTnrModeData->TunerFreq > LNA_Tilt_Freq))
	{
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_LNA1_CNTL, STAGE1_TILT_ON, 0x1);
		BDBG_MSG(("LNA_TILT is enabled, LNA Tilt Frequency is %d",LNA_Tilt_Freq));
	}
	else
	{
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_LNA1_CNTL, STAGE1_TILT_ON, 0x0); 
		BDBG_MSG(("LNA_TILT is disabled"));
	}
}

/****************************************************************************************************
 * BTNR_7125_P_Set_LNA_Output()  This routine enables the correct output ports of the external LNA
 ****************************************************************************************************/
void BTNR_7125_P_Set_LNA_Output(BTNR_7125_Handle h)
{

	if (h->pTnrModeData->LNA_ChipID == BTNR_LNAID_3412) /*Don't do anything if 3410 is used because there is only one output*/
	{
			BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE2_PD1, 0x0);
			BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE2_PD2, 0x1);
			BDBG_MSG(("LNA Output 1 is enabled"));
	}
}

/****************************************************************************************************
 * BTNR_7125_P_PowerDown_LNA_Output()  This routine powers down output ports of the external LNA
 ****************************************************************************************************/
void BTNR_7125_P_PowerDown_LNA_Output(BTNR_7125_Handle h, bool PowerDown)
{

	BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE2_PD1, PowerDown);
	if (h->pTnrModeData->LNA_ChipID == BTNR_LNAID_3412)
	{ 		/*Don't do anything if 3410 is used because there is only one output*/
			BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_BG_LNA2_RCAL_CNTL, STAGE2_PD2, PowerDown);
	}
	BDBG_MSG(("Powered %s LNA Output(s)",PowerDown?"down":"up"));
}

/****************************************************************************************************
 * BTNR_7125_P_Set_Rf_ByPass_mode()  This routine enables/disables the RF bypass
 ****************************************************************************************************/
void BTNR_7125_P_Set_Rf_ByPass_mode(BTNR_7125_Handle h, bool mode)
{
	if (mode) /* if bypass is enabled) */
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_PD_OOB_OSC_CNTL, LT_PD, 0x0);
	else
		BTNR_7125_P_LNA_WriteRegisterField(h,  BCM341X_regs_PD_OOB_OSC_CNTL, LT_PD, 0x1);
}

/****************************************************************************************************
 * BTNR_7125_P_Get_LNA_Gain()  This routine reads the gain of the external LNA
 ****************************************************************************************************/
uint8_t BTNR_7125_P_Get_LNA_Gain(BTNR_7125_Handle h)
{
	return (BTNR_7125_P_LNA_ReadRegisterField(h,  BCM341X_regs_STATUS_READ2, STG1_GAIN));
}

/****************************************************************************************************
 * BTNR_7125_P_Get_LNA_Status()  This routine get teh LNA Status fror the Tuner Status Page
 ****************************************************************************************************/
void BTNR_7125_P_Get_LNA_Status(BTNR_7125_Handle h)
{

	uint32_t ReadReg;
	int32_t GainReg;

	/*The LNA Gain is reported in 1/256 db*/	
	/*Get the LNA GAIN if Boost and SuperBoost are disabled*/
	ReadReg = BTNR_7125_P_LNA_ReadRegisterField(h,  BCM341X_regs_STATUS_READ2, STG1_GAIN);
	GainReg = (-14 + ReadReg)<<8;
	if (ReadReg == 0)
	{
		BDBG_WRN(("WARNING!!! Invalid STG1_GAIN in LNA detected, STG1_GAIN= %d",ReadReg));
	}

	/*Compenasate if Boost is enabled*/
	/*Check for LNA Boost*/
	ReadReg = BTNR_7125_P_LNA_ReadRegisterField(h,  BCM341X_regs_LNA1_CNTL, STAGE1_GAIN_BOOST);
	h->pTnrStatus->LNA_Boost_Status = 0;
	if (ReadReg == 1)
	{
		GainReg = GainReg + 768;
		h->pTnrStatus->LNA_Boost_Status = 1;
		/*Compensate again if SuperBoost is enabled in addition to Boost*/
		/*Check for LNA SuperBoost in 3412 Only*/
		if (h->pTnrModeData->LNA_ChipID == BTNR_LNAID_3412)
		{
			if (BTNR_7125_P_LNA_ReadRegisterField(h,  BCM341X_regs_LNA_ADDR, SuperBoost_en) == 1)
			{
				GainReg = GainReg + 768; 
				h->pTnrStatus->LNA_Boost_Status = 2;
			}
		}
	}

	/*Get tilt*/
	ReadReg = BTNR_7125_P_LNA_ReadRegisterField(h,  BCM341X_regs_LNA1_CNTL, STAGE1_TILT_ON);
	h->pTnrStatus->LNA_Tilt_Status = 0;
	if (ReadReg == 1)
	{
			h->pTnrStatus->LNA_Tilt_Status = 1;

			/*Compensate gain if tilt is on, a 5 db rise from 500 MHz to 1 GHz*/
			if ((h->pTnrStatus->TunerFreq > 500000000) && (h->pTnrStatus->TunerFreq <= 1000000000))
			{
				GainReg = GainReg + (((h->pTnrStatus->TunerFreq-500000000)>>8)*5*256)/(500000000>>8); /*keep in range of 32 bits*/
			}
			else if (h->pTnrStatus->TunerFreq > 1000000000)
			{
				GainReg = GainReg + 1280;  /*add 5 db*/
			}
	}

	/*assign the tilt compensated gain*/
	h->pTnrStatus->LNA_Gain_256db = (uint32_t)GainReg;
}

/****************************************************************************************************/
/* END OF LNA FUNCTIONS MAINTAINED BY TUNING CODE *************************************************/
/****************************************************************************************************/

/****************************************************************************************************
 * BTNR_7125_P_WaitForLock_LNA()  This routine waits for the BCM3410 AGC state Machine to obtain lock
 * From DOCSIS code: WaitForLock()
 ****************************************************************************************************/
static void BTNR_7125_P_WaitForLock_LNA( BTNR_7125_Handle h )
{
    uint32_t Status, AGC_Lock, Gain, i;

    /* Temporarily speed up the response of the AGC in the 3410 */
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_RAMP_RATE_CNTL,SLOW_UP_RAMP_RATE,0x40);
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_RAMP_RATE_CNTL,SLOW_DWN_RAMP_RATE,0x40);

    /* max ramp */
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_SM_BYPASS,ATE,1);

    /*Freeze*/
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_LNA_ADDR,STG1_GAIN_I2C,0x0F);
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_SM_BYPASS,DEC_LAT_ENB_STG1_I2C,1);
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_SM_BYPASS,DEC_LAT_ENB_STG1_I2C,0);

    /*Release SM and PD*/
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_MWC_CNTL,SM_RESET,0);
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_PD_OOB_OSC_CNTL,PD_RESET,0);

	BKNI_Sleep(100);

    /*Normal Ramp*/
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_SM_BYPASS,ATE,0);

    i = 0;
    while( i < 100 ) /* max 5 sec loop */
    {
        i++;
        BKNI_Sleep(50);
        Status = BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_STATUS_READ2);
        /* BDBG_MSG((" inLoop%ld: LNA Status = 0x%08lx", i, Status)); */
        AGC_Lock = Status & BCHP_BCM341X_regs_STATUS_READ2_AGC_LOCK_MASK;
        Gain = Status & 0x0000001F;
        if( (AGC_Lock==0x80) || (Gain==0x1F) )
            break;
    }

    /*Restore ramp rate to defaults*/
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_RAMP_RATE_CNTL,SLOW_UP_RAMP_RATE,0x04);
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_RAMP_RATE_CNTL,SLOW_DWN_RAMP_RATE,0x10);
}

/****************************************************************************************************
 * BTNR_7125_P_ReConfig_LNA()  This routine optimizes tilt/boost settings of the external LNA
 * From DOCSIS code: BCM341X_regs_ReConfig(), formerly BCM341X_regs_TiltBoost() 
 ****************************************************************************************************/
void BTNR_7125_P_ReConfig_LNA( BTNR_7125_Handle h )
{
    int currentGain, tiltGain;
    int tolBoost, tolTilt;
	int BoostOn=1;
	uint32_t LNA_chip_id;

	LNA_chip_id = BTNR_7125_P_LNA_ReadRegisterField(h,BCM341X_regs_CHIP_ID, CHIP_PART_NUMBER);

    BDBG_MSG(("\nInitial Boost Status=%d. ", ((BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_LNA1_CNTL) & 0x00000002)?1:0) ));
    BDBG_MSG(("Tilt Status=%d", ((BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_LNA1_CNTL) & 0x00000001)?1:0) ));

    /* reset SM and PD */
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_MWC_CNTL,SM_RESET,1);
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_PD_OOB_OSC_CNTL,PD_RESET,1);

    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_LNA1_CNTL,STAGE1_GAIN_BOOST,1);
    BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_LNA1_CNTL,STAGE1_TILT_ON,1);

    tolBoost = 3;  /* Adjustable tolerance on the decision */
    tolTilt  = 0;

    if( (LNA_chip_id==0x3412) || (LNA_chip_id==0x3406)|| (LNA_chip_id==0x3413) )  /* chip with superboost mode */
    {
        tolTilt  = 1;   /* script uses different value */
        BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_LNA_ADDR, SuperBoost_en, 1);
        BDBG_MSG(("(SuperBoost==1)"));
    }

    BTNR_7125_P_WaitForLock_LNA(h);
	/* valid value is 1-31 although it has 6 bits definition */
    currentGain = BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_STATUS_READ2) & 0x0000001F;  
    BDBG_MSG(("(Boost=Tilt=1)LNA Gain = 0x%x", currentGain));

    if( currentGain < 0x1F )
    {
        BDBG_MSG(("Turning off Tilt..."));
        BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_MWC_CNTL,        SM_RESET, 1);
        BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_PD_OOB_OSC_CNTL, PD_RESET, 1);
        BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_LNA1_CNTL,       STAGE1_TILT_ON, 0);
        BTNR_7125_P_WaitForLock_LNA(h);
        tiltGain = BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_STATUS_READ2) & 0x0000001F;
        BDBG_MSG(("(Boost/Tilt = 1/0)LNA Gain = 0x%x.", tiltGain));

        if( ((tiltGain-currentGain) <= tolTilt) )
        {
            BDBG_MSG(("Turning Tilt back on..."));
            BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_MWC_CNTL,        SM_RESET, 1);
            BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_PD_OOB_OSC_CNTL, PD_RESET, 1);
            BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_LNA1_CNTL,       STAGE1_TILT_ON, 1);
            BTNR_7125_P_WaitForLock_LNA(h);
            currentGain = BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_STATUS_READ2) & 0x0000001F;
            BDBG_MSG(( "LNA Gain = 0x%x", currentGain));
        }

        /* chip with superboost mode */
        if( (LNA_chip_id==0x3412) || (LNA_chip_id==0x3406)|| (LNA_chip_id==0x3413) )
        {
            if( currentGain < (0x1F-tolBoost) )
            {
                BDBG_MSG(("Turning off Enhanced Boost..."));
                BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_MWC_CNTL,			SM_RESET, 1);
                BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_PD_OOB_OSC_CNTL,	PD_RESET, 1);
                BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_LNA_ADDR,			SuperBoost_en, 0);
                BTNR_7125_P_WaitForLock_LNA(h);
                currentGain = BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_STATUS_READ2) & 0x0000001F;
                BDBG_MSG(( "LNA Gain = 0x%x", currentGain));
            }
        }

        if( currentGain < (0x1F-tolBoost) )
        {
            BDBG_MSG(("Turning off Boost..."));
            BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_MWC_CNTL,        SM_RESET, 1);
            BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_PD_OOB_OSC_CNTL, PD_RESET, 1);
            BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_LNA1_CNTL,       STAGE1_GAIN_BOOST, 0);
			BoostOn=0;
            BTNR_7125_P_WaitForLock_LNA(h);
            currentGain = BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_STATUS_READ2) & 0x0000001F;
            BDBG_MSG(( "LNA Gain = 0x%x", currentGain));
        }
    }
    else
    {
        BTNR_7125_P_LNA_WriteRegisterField(h,BCM341X_regs_LNA1_CNTL, STAGE1_TILT_ON, 0);
    }

    currentGain = BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_STATUS_READ2) & 0x0000003F;
    /* Configure RF Splitter Attenuator Based upon LNA Status */
    {
       uint8_t LNA_Gain_Split;

       LNA_Gain_Split = currentGain;

       if ( (LNA_Gain_Split <= 31) && (LNA_Gain_Split > 29) )
       {
		  BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 1, 0);
		  BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 1, 1);
       }
       else if ( LNA_Gain_Split <= 29 )
       {
		  BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 2, 0);
		  BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 2, 1);
       }
       BDBG_MSG(("BCM3410 LNA Gain State is = %d", LNA_Gain_Split ));

    }

    if( (LNA_chip_id==0x3412) || (LNA_chip_id==0x3406)|| (LNA_chip_id==0x3413) )  /* chip with superboost mode */
        BDBG_MSG(("(Final SuperBoost=%d)",((BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_LNA_ADDR) & 0x00800000)?1:0) ));
		BDBG_MSG(("Final(Boost/Tilt=%d/%d) LNA Gain=0x%x.\n",
				  ((BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_LNA1_CNTL) & 0x00000002)?1:0),
				  ((BTNR_7125_P_LNA_ReadRegister(h,BCHP_BCM341X_regs_LNA1_CNTL) & 0x00000001)?1:0),
				  currentGain ));
}

