/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btmr.h"
#include "btnr.h"
#include "bdbg.h"
#include "bmem.h"
#include "btnr_priv.h"
#include "btnr_ob_3x7x.h"
#include "btnr_3x7xob_priv.h"
#include "btnr_ob_tune.h"
#include "bmth.h"
#include "bchp_ufe_afe.h"
#include "bchp_sdadc.h"
#include "bchp_ufe.h"

#define abs(x) ((x)<0?-(x):(x))

BDBG_MODULE(btnr_ob_tune);
/******************************************************************************
 BTNR_Ob_P_CalFlashSDADC() - calibrate SD ADC flash
******************************************************************************/
void BTNR_Ob_P_CalFlashSDADC(BTNR_Ob_3x7x_Handle h)
{
	uint8_t idx;
	uint32_t ICalOffset, QCalOffset;
	uint8_t statusIch, statusQch=0;

	BDBG_MSG(("BTNR_Ob_P_CalFlashSDADC"));
	/*I Channel Calibration*/
	BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_Ich_flash_resetCal, 0x1);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Ich_reset, 0x1);
	BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_Ich_flash_resetCal, 0x0);
	BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_Ich_flash_cal_on, 0x1);

	/*Q Channel Calibration*/
    BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_Qch_flash_resetCal, 0x1);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Qch_reset, 0x1);
    BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_Qch_flash_resetCal, 0x0);
    BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_Qch_flash_cal_on, 0x1);

	for (statusIch=0, idx =0; (idx<2) && ( !(statusIch) || !(statusQch) ) ;idx++){
		BKNI_Sleep(1);
		statusIch = BREG_ReadField(h->hRegister,SDADC_STATUS_ICH, o_Ich_flash_cal_done);
		statusQch = BREG_ReadField(h->hRegister,SDADC_STATUS_QCH, o_Qch_flash_cal_done);
	}

	switch (statusIch)
	{
	case 0 : BDBG_MSG(("SDADC I channel calibration NOT done"));
			 if (idx>99){
				 BDBG_ERR(("SDADC I channel calibration timeout"));
				 }
			 break;
	case 1 : BDBG_MSG(("SDADC I channel calibrationis done")); break;
	default :BDBG_ERR(("ERROR!!! INVALID SDADC I Channel Calibration Value: value is %d",statusIch)); break;
	}

	ICalOffset = BREG_ReadField(h->hRegister,SDADC_STATUS_ICH, o_Ich_flash_caldata);
	BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_ctl_Ich_flash_offset, ICalOffset);
	BREG_WriteField(h->hRegister, SDADC_CTRL_ICH, i_Ich_flash_cal_on, 0);

	switch (statusQch)
	{
	case 0 : BDBG_MSG(("SDADC Q channel calibration NOT done"));
			 if (idx>99){
				 BDBG_ERR(("SDADC Q channel calibration timeout"));
				 }
			 break;
	case 1 : BDBG_MSG(("SDADC Q channel calibrationis done")); break;
	default :BDBG_ERR(("ERROR!!! INVALID SDADC Q Channel Calibration Value: value is %d",statusQch)); break;
	}

	QCalOffset = BREG_ReadField(h->hRegister,SDADC_STATUS_QCH, o_Qch_flash_caldata);
	BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_ctl_Qch_flash_offset, QCalOffset);
	BREG_WriteField(h->hRegister, SDADC_CTRL_QCH, i_Qch_flash_cal_on, 0);

	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Ich_reset, 0x0);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Qch_reset, 0x0);
}


/*******************************************************************************************************************
 * BTNR_Ob_P_TunerSetRFAGC()  This routine sets the tuner RF AGC
 ******************************************************************************************************************/
void BTNR_Ob_P_TunerSetRFAGC(BTNR_Ob_3x7x_Handle h)
{
	uint32_t ReadReg;

	BDBG_MSG(("BTNR_Ob_P_TunerSetRFAGC"));

	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_RFAGC_CLK_sel, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_dsm_sigdel_en, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, RFAGC_DSM_intg_reset, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, RFAGC_DSM_resetb, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, RFAGC_reset, 1);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, RFAGC_DSM_intg_reset, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, RFAGC_DSM_resetb, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, RFAGC_reset, 0);

	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, RF_Ios_PRG, 0x7);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, RF_delay, 0x2);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, RF_clk_extend, 0x2);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB2_sel_IQ, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB2_Ios_PRG, 0x5);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB2_delay, 0x2);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB2_clk_extend, 0x2);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB1_sel_IQ, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB1_Ios_PRG, 0x5);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB1_delay, 0x2);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_03, BB1_clk_extend, 0x2);

	/*RF AGC initial gain settings*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_01, i_waddr, 0x4);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_04, i_wdata, 0x10);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x1);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);


	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_01, i_waddr, 0x2);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_04, i_wdata, 0x4E2);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x1);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);

	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_01, i_waddr, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_04, i_wdata, 0x31BD31BD);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x1);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);


	/*RF AGC close loop settings*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_01, i_raddr, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_rload, 0x0);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_rload, 0x1);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_rload, 0x0);
	BKNI_Sleep(1);

	ReadReg = BREG_ReadField(h->hRegister, UFE_AFE_TNR0_RFAGC_05, o_rdata);

	ReadReg = ReadReg & 0xFFF3BDC0;
	ReadReg = ReadReg | 0x0000401B;

	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_01, i_waddr, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_04, i_wdata, ReadReg);

	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x1);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);

		/*RF AGC initial gain settings*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_freeze_gain, 0x1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_01, i_waddr, 0x5);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_04, i_wdata, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x1);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_wload, 0x0);
	BKNI_Sleep(1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RFAGC_02, i_freeze_gain, 0x0);

	BDBG_MSG(("BTNR_Ob_P_TunerSetRFAGC() Complete"));

}

/*******************************************************************************************************************
 * BTNR_Ob_P_TunerSetLNAAGC()  This routine sets the tuner LNA AGC
 ******************************************************************************************************************/
void BTNR_Ob_P_TunerSetLNAAGC(BTNR_Ob_3x7x_Handle h)
{

	/* negative edge */
	/* De-assert reset/resetb for LNA AGC */

	BDBG_MSG(("BTNR_Ob_P_TunerSetLNAAGC"));
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, LNAAGC_resetb, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, LNAAGC_resetb, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_LNAAGC_dsm_srst, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_LNAAGC_dsm_srst, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_LNAAGC_agc_srst, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, i_LNAAGC_agc_srst, 0);

		/* freeze LNA AGC */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_03, AGC_frz, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_01, LNA_Kbw, 0x0);
	/* peak detection  check with dave*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_02, negedge_sel, 1);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, peak_pwrb, 1);

	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, pd_thresh, 0x7);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, peak_pwr_set_pt, 46515);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_01, hi_thresh, 24856);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, win_len, 0xA);

	/*set initial gain*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_01, init_LNA_gain, 29);

	/*unfreeze, un-bypass*/
    BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_03, AGC_byp, 0);
    BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_03, AGC_frz, 0);
	/*start LNA AGC*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, lna_start, 0x0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_LNAAGC_00, lna_start, 0x1);

	BDBG_MSG(("BTNR_Ob_P_TunerSetLNAAGC() Complete"));

}
/*******************************************************************************************
 * BTNR_Ob_P_Tuner_Power_Control()		This routine controls the power up/down of the tuner blocks
 *******************************************************************************************/
BERR_Code BTNR_Ob_P_Tuner_Power_Control(BTNR_Ob_3x7x_Handle h)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint32_t temp_UFE_AFE_TNR0_PWRUP_01 = 0;
	uint32_t temp_UFE_AFE_TNR0_PWRUP_02 = 0;
	uint32_t temp_UFE_AFE_TNR_PWRUP_01  = 0;
	uint32_t temp_SDADC_CTRL_PWRUP      = 0;

	BDBG_MSG(("BTNR_Ob_P_Tuner_Power_Control"));

	temp_UFE_AFE_TNR0_PWRUP_01 = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_01);
	temp_UFE_AFE_TNR0_PWRUP_02 = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02);
	temp_UFE_AFE_TNR_PWRUP_01 = BREG_Read32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01);
	temp_SDADC_CTRL_PWRUP = BREG_Read32(h->hRegister, BCHP_SDADC_CTRL_PWRUP);

	/* power up in VHF mode */
	temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x0FA383F7);
	temp_UFE_AFE_TNR0_PWRUP_02 = (temp_UFE_AFE_TNR0_PWRUP_02 | 0x0029FAFF);
	temp_UFE_AFE_TNR_PWRUP_01  = (temp_UFE_AFE_TNR_PWRUP_01  | 0x00006CFF);

	/* power up Daisy in VHF mode */
	temp_UFE_AFE_TNR0_PWRUP_01 = (temp_UFE_AFE_TNR0_PWRUP_01 | 0x0FA30817);
	temp_UFE_AFE_TNR0_PWRUP_02 = (temp_UFE_AFE_TNR0_PWRUP_02 | 0x00218000);
	temp_UFE_AFE_TNR_PWRUP_01  = (temp_UFE_AFE_TNR_PWRUP_01  | 0x00006CFF);
	temp_SDADC_CTRL_PWRUP =  (temp_SDADC_CTRL_PWRUP  | 0x00000003);

	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_01, temp_UFE_AFE_TNR0_PWRUP_01);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR0_PWRUP_02, temp_UFE_AFE_TNR0_PWRUP_02);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_PWRUP_01,  temp_UFE_AFE_TNR_PWRUP_01);
	BREG_Write32(h->hRegister, BCHP_SDADC_CTRL_PWRUP,      temp_SDADC_CTRL_PWRUP);


	/*TERR_PHYPLL1_FREQ=REF_FREQ*(ndiv_int+ndiv_frac/2^24)/p1div]/m1div, m1div is per channel*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_02, ndiv_frac, 0x0);/*ndiv_frac = 0*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, p1div, 0x01);   /*p1div = 1*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_03, ndiv_int, 0x32);/*ndiv_int = 50*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m1div, 0x01);   /*m1div = 1*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m2div, 0x02);   /*m2div = 2*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m3div, 0x0C);   /*m3div = 12*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_04, m4div, 0x1B);   /*m4div = 27*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, m5div, 0x00);   /*m5div = 256*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, m6div, 0x05);   /*m6div = 5*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_05, load_en_ch6_ch1, 0x3F);

	/*optimize PHYPLL*/
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_01, kvcox, 0x3);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_01, Icpx, 0x1F);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_PHYPLL_01, Rz, 0xF);

		/* De-assert reset/resetb for REFPLL/PHYPLL */
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_RESETB_01, 0x00000003);
	BREG_Write32(h->hRegister, BCHP_UFE_AFE_TNR_RESET_01, 0xFFFFFFF0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_dreset, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESET_01, PHYPLL_areset, 0);

	BKNI_Sleep(1);
	/*PhyPLL Lock*/
	if (BREG_ReadField(h->hRegister, UFE_AFE_TNR_REFPLL_04, o_PHYPLL_lock) == 1)
		BDBG_MSG(("Phy PLL is lock"));
	else
		BDBG_MSG(("Phy PLL is not lock"));

	/* De-assert reset/resetb for 6-bit ADC */
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, ADC6B_resetb, 0);
	BREG_WriteField(h->hRegister, UFE_AFE_TNR0_RESETB_01, ADC6B_resetb, 1);
	BKNI_Sleep(1);

	BREG_WriteField(h->hRegister, SDADC_CTRL_SYS0, i_adcclk_reset, 0x0);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Ich_reset, 0x0);
	BREG_WriteField(h->hRegister, SDADC_CTRL_RESET, i_Qch_reset, 0x0);

  return retCode;
}



/*******************************************************************************************
 * BTNR_Ob_P_TunerInit()		This routine initializes the tuner and is only run once
 *******************************************************************************************/
BERR_Code BTNR_Ob_P_TunerInit(BTNR_Ob_3x7x_Handle h)
{
	BERR_Code retCode = BERR_SUCCESS;

	BTNR_Ob_P_CalFlashSDADC(h);
	BTNR_Ob_P_TunerSetLNAAGC(h);
	BTNR_Ob_P_TunerSetRFAGC(h);

	return retCode;
}



