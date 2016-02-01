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
#include "bmth.h"
#include "btnr_7125ib_priv.h"
#include "btnr_7125_tune.h"

/*registers needed for this function */
#include "bchp_ds_0.h"
#include "bchp_ds_tuner_anactl_0.h"
#include "bchp_ds_tuner_anactl_1.h"
#include "bchp_ds_tuner_digctl_0_0.h"
#include "bchp_ds_tuner_digctl_0_1.h"
#include "bchp_ds_tuner_chnz_0.h"
#include "bchp_ds_qafe_if_0_0.h" 
#include "bchp_ds_qdsafe_0_0.h"
#include "bchp_ds_tuner_ref_0.h"
#include "bchp_us_top.h"
#include "bchp_int.h"
#include "bchp_clkgen.h"
#include "bchp_vcxo_ctl_misc.h"
#include "bchp_oob.h"
#include "bchp_jtag_otp.h"

BDBG_MODULE(btnr_7125_utils);

/***************************************************************************
 * PowerDn_TNR_Core0()
 ***************************************************************************/
BERR_Code BTNR_7125_P_PowerDn_TNR_Core0(BTNR_7125_Handle h)
{
  BERR_Code retCode;
#if (BCHP_VER>=BCHP_VER_C0)
	{
        unsigned unit = h->settings.unitNumber;

		if (!BTNR_7125_P_ReadField(h->hRegister, VCXO_CTL_MISC_DOCSIS_CTRL, POWERDOWN, 0))
		{ /* if whole frontend is not already powered off */
			if (unit==0)
			{
				/*DS0.power_down_DS0_AFE*/
				BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE0, PWRUP_CMLBUF, 0, 0);
				BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE0, PWRUP, 0, 0);
				BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE0, EN_CLK, 0, 0);
			}
			else
			{
				/*DS1.power_down_DS1_AFE*/
				BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE1, PWRUP_CMLBUF, 0, 0);
				BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE1, PWRUP, 0, 0);
				BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE1, EN_CLK, 0, 0);
			}
	
			BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, PD_ADC, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, PWDN_BG, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, CLK_RESET, 1, unit);
	
			/*DS_0.power_down_DS0_Tuner*/
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_PWRDN, 1, unit);
	
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, MIX_PWRDN, 1, unit);
	
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, PGA_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_PWROFF, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_PWROFFI, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_PWROFFQ, 1, unit);
		
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_IQ_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, I_DCO_PRE_IQ_PWRDN_VBAL, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, REG1P2V_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_IDCO_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_QDCO_PWRDN, 1, unit);
	
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, PWRDN_BG_1P2, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_VCO_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_PWRUP_WD, 0, unit);
	
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_PWRUP_LF, 0, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_QP2_PD, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_MANIBAL, 0, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_QP_PD, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_VC_PRB_EN, 0, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_VZ_PRB_EN, 0, unit);
		
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, DACIMFREG_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, IP_PD_LO, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, IMF_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, I_DAC_PWRUP, 0, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, CML1200_PWRDN, 1, unit);
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, D2D1200_PWRDN, 1, unit);
		
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXR_DCODAC_PWRDN_1P2, 1, unit);
	
			if ((!BTNR_7125_P_ReadField(h->hRegister, US_TOP_PLL_CLK_DS_AFE0, PWRUP, 0)) &&
				(!BTNR_7125_P_ReadField(h->hRegister, US_TOP_PLL_CLK_DS_AFE1, PWRUP, 0)) )
			{
				BDBG_MSG(("%s(%u) powering down shared DS_TUNER_REF\n",__FUNCTION__,unit));
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_02, I_OUTCML_PWRDN_M, 0x7f, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_02, I_CLKEN_M, 0, 0);
			
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_MUX_PWRUP, 0, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_UGB_PWRUP, 0, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_DIV_REFBUF_PWRDN, 1, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_DIV_FB_PWRDN, 1, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_DIV4_PWRDN, 1, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_DIV_FBREG_PWRDN, 1, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_VCO_PWRUP, 0, 0);
			
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_REGQPPFD_PWRDN, 1, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_QP_PWRDN, 1, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_OUTDIV_REG_PWRDN, 1, 0);
		
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04, I_OUTDIV_IGEN_PWRDN, 1, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04, I_IGEN_PWRDN, 1, 0);
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04, I_OUTCML_DIODE_PWRDN, 1, 0);
				/* This will indicate whether shared REF has been initialized in first tune */
				BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00,I_VCOBUF_LATCH_ON,0, 0);
			}
			else
			{
				BDBG_MSG(("%s(%u) NOT powering down shared DS_TUNER_REF, still in use \n",__FUNCTION__,unit));
			}
	
		}
		h->pTnrModeData->DoneFirstTimeFlag = BTNR_OFF; /* force re-init next tune */
		h->pTnrModeData->TunerFreq=0;
	}
#endif
	
	BDBG_MSG(("%s() Complete\n",__FUNCTION__));
	
	retCode = BERR_SUCCESS;
  return retCode;
}

/***************************************************************************
 * PowerUp_TNR_Core0()
 ***************************************************************************/
BERR_Code BTNR_7125_P_PowerUp_TNR_Core0(BTNR_7125_Handle h)
{
  BERR_Code retCode;
#if (BCHP_VER>=BCHP_VER_C0)
	{
        unsigned unit;
        uint32_t reg;
        BREG_Handle hReg = h->hRegister;

        for (unit=0;unit<=h->settings.unitNumber;unit++)
        {
            if (unit==0)
            {
        		/* resume DOCSIS PLL */
        		reg = BREG_Read32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_CTRL);
        		reg &= ~(
        				BCHP_MASK(VCXO_CTL_MISC_DOCSIS_CTRL, ARESET) |
        				BCHP_MASK(VCXO_CTL_MISC_DOCSIS_CTRL, POWERDOWN)
        				);
        		reg |=
        				BCHP_FIELD_DATA(VCXO_CTL_MISC_DOCSIS_CTRL, ARESET,  0 ) |
        				BCHP_FIELD_DATA(VCXO_CTL_MISC_DOCSIS_CTRL, POWERDOWN, 0 )
        		;
        		BREG_Write32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_CTRL, reg);
        	
        		BKNI_Sleep(2);
        	
        		reg = BREG_Read32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_CTRL);
        		reg &= ~(
        				BCHP_MASK(VCXO_CTL_MISC_DOCSIS_CTRL, DRESET)
        				);
        		reg |=
        				BCHP_FIELD_DATA(VCXO_CTL_MISC_DOCSIS_CTRL, DRESET,  0 )
        		;
        		BREG_Write32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_CTRL, reg);
                
        		/*Resume UBUS and SCB_BRIDGE clock */
        		reg = BREG_Read32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_2);
        		reg &= ~(
        				BCHP_MASK(VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_2, CLOCK_ENA)
        				);
        		reg |=
        				BCHP_FIELD_DATA(VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_2, CLOCK_ENA,  1)
        		;
        		BREG_Write32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_2, reg);
                
        		/*Resume DOCSIS DS MAC clock */
        		reg = BREG_Read32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_4);
        		reg &= ~(
        				BCHP_MASK(VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_4, CLOCK_ENA)
        				);
        		reg |=
        				BCHP_FIELD_DATA(VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_4, CLOCK_ENA,  1 )
        		;
        		BREG_Write32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_4, reg);
                
        		/*Resume RBUS clock */
        		reg = BREG_Read32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_5);
        		reg &= ~(
        				BCHP_MASK(VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_5, CLOCK_ENA)
        				);
        		reg |=
        				BCHP_FIELD_DATA(VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_5, CLOCK_ENA,  1 )
        		;
        		BREG_Write32(hReg, BCHP_VCXO_CTL_MISC_DOCSIS_PM_DIS_CHL_5, reg);
                
        		/*Bring up the US PLL*/
        		reg = BREG_Read32(hReg, BCHP_US_TOP_PWRDN);
        		reg &= ~(
        				BCHP_MASK(US_TOP_PWRDN, VREG) |
        				BCHP_MASK(US_TOP_PWRDN, PLL)
        				);
        		reg |=
        				BCHP_FIELD_DATA(US_TOP_PWRDN, VREG,  0 ) |
        				BCHP_FIELD_DATA(US_TOP_PWRDN, PLL,  0 )
        		;
        		BREG_Write32(hReg, BCHP_US_TOP_PWRDN, reg);
        	
        		reg = BREG_Read32(hReg, BCHP_US_TOP_PLL_RST);
        		reg &= ~(
        				BCHP_MASK(US_TOP_PLL_RST, PHASE) |
        				BCHP_MASK(US_TOP_PLL_RST, DIGITAL) |
        				BCHP_MASK(US_TOP_PLL_RST, ANALOG)
        				);
        		reg |=
        				BCHP_FIELD_DATA(US_TOP_PLL_RST, PHASE,  0 ) |
        				BCHP_FIELD_DATA(US_TOP_PLL_RST, DIGITAL,  0 ) |
        				BCHP_FIELD_DATA(US_TOP_PLL_RST, ANALOG,  0 )
        		;
        		BREG_Write32(hReg, BCHP_US_TOP_PLL_RST, reg);
                
        		reg = BREG_Read32(hReg, BCHP_INT_CLKCONTROL);
        		reg &= ~(
        				BCHP_MASK(INT_CLKCONTROL, D3DSMAC_CLK_EN)
        				);
        		reg |=
        				BCHP_FIELD_DATA(INT_CLKCONTROL, D3DSMAC_CLK_EN,  1 )
        		;
        		BREG_Write32(hReg, BCHP_INT_CLKCONTROL, reg);
                
                /*DS_0.power_up_DS0_receiver*/
                reg = BREG_Read32(hReg, BCHP_US_TOP_PLL_CLK_DS0);
                if (!(reg & BCHP_FIELD_DATA(US_TOP_PLL_CLK_DS0, EN_CLK,  1 )))
                {
                    reg &= ~(
                            BCHP_MASK(US_TOP_PLL_CLK_DS0, PWRUP_CMLBUF) |
                            BCHP_MASK(US_TOP_PLL_CLK_DS0, PWRUP) |
                            BCHP_MASK(US_TOP_PLL_CLK_DS0, EN_CLK)
                            );
                    reg |=
                            BCHP_FIELD_DATA(US_TOP_PLL_CLK_DS0, PWRUP_CMLBUF,  1 ) |
                            BCHP_FIELD_DATA(US_TOP_PLL_CLK_DS0, PWRUP,  1 ) |
                            BCHP_FIELD_DATA(US_TOP_PLL_CLK_DS0, EN_CLK,  1 )
                    ;
                    BREG_Write32(hReg, BCHP_US_TOP_PLL_CLK_DS0, reg);
        
                    reg = BREG_Read32(hReg, BCHP_INT_CLKCONTROL);
                    reg &= ~(
                            BCHP_MASK(INT_CLKCONTROL, DS0_CLK_EN)
                            );
                    reg |=
                            BCHP_FIELD_DATA(INT_CLKCONTROL, DS0_CLK_EN,  1 )
                    ;
                    BREG_Write32(hReg, BCHP_INT_CLKCONTROL, reg);
                }
                /*DS0.power_up_DS0_AFE*/
                if (BTNR_7125_P_ReadField(h->hRegister, US_TOP_PLL_CLK_DS_AFE0, PWRUP_CMLBUF, 0))
                {
                    BDBG_MSG(("tuner 0 already powered up"));
                    continue;
                }
                BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE0, PWRUP_CMLBUF, 1, 0);
                BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE0, PWRUP, 1, 0);
                BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE0, EN_CLK, 1, 0);
            }
            else
            {
                /*DS_1.power_up_DS1_receiver*/
                reg = BREG_Read32(hReg, BCHP_US_TOP_PLL_CLK_DS1);
                if (!(reg & BCHP_FIELD_DATA(US_TOP_PLL_CLK_DS1, EN_CLK,  1 )))
                {
                    reg &= ~(
                            BCHP_MASK(US_TOP_PLL_CLK_DS1, PWRUP_CMLBUF) |
                            BCHP_MASK(US_TOP_PLL_CLK_DS1, PWRUP) |
                            BCHP_MASK(US_TOP_PLL_CLK_DS1, EN_CLK)
                            );
                    reg |=
                            BCHP_FIELD_DATA(US_TOP_PLL_CLK_DS1, PWRUP_CMLBUF,  1 ) |
                            BCHP_FIELD_DATA(US_TOP_PLL_CLK_DS1, PWRUP,  1 ) |
                            BCHP_FIELD_DATA(US_TOP_PLL_CLK_DS1, EN_CLK,  1 )
                    ;
                    BREG_Write32(hReg, BCHP_US_TOP_PLL_CLK_DS1, reg);
        
                    reg = BREG_Read32(hReg, BCHP_INT_CLKCONTROL);
                    reg &= ~(
                            BCHP_MASK(INT_CLKCONTROL, DS1_CLK_EN)
                            );
                    reg |=
                            BCHP_FIELD_DATA(INT_CLKCONTROL, DS1_CLK_EN,  1 )
                    ;
                    BREG_Write32(hReg, BCHP_INT_CLKCONTROL, reg);    
                }
                /*DS1.power_up_DS1_AFE*/
                if (BTNR_7125_P_ReadField(h->hRegister, US_TOP_PLL_CLK_DS_AFE1, PWRUP_CMLBUF, 0))
                {
                    BDBG_MSG(("tuner 1 already powered up"));
                    continue;
                }
                BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE1, PWRUP_CMLBUF, 1, 0);
                BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE1, PWRUP, 1, 0);
                BTNR_7125_P_WriteField(h->hRegister, US_TOP_PLL_CLK_DS_AFE1, EN_CLK, 1, 0);
            }
    
            BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, PD_ADC, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, PWDN_BG, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, CLK_RESET, 0, unit);
    
    		/*DS_0.power_up_DS0_Tuner*/
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_PWRDN, 0, unit);
    
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, MIX_PWRDN, 0, unit);
    
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, PGA_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_PWROFF, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_PWROFFI, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, IFLPF_PWROFFQ, 0, unit);
    	
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_IQ_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, I_DCO_PRE_IQ_PWRDN_VBAL, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, REG1P2V_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_IDCO_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_QDCO_PWRDN, 0, unit);
    
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, PWRDN_BG_1P2, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_VCO_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_05, IP_PWRUP_WD, 1, unit);
    
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_PWRUP_LF, 1, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_QP2_PD, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_MANIBAL, 1, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_QP_PD, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_VC_PRB_EN, 1, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_06, IP_VZ_PRB_EN, 1, unit);
    	
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, DACIMFREG_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, IP_PD_LO, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, IMF_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, I_DAC_PWRUP, 1, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, CML1200_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_07, D2D1200_PWRDN, 0, unit);
    	
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXR_DCODAC_PWRDN_1P2, 0, unit);
    
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_02, I_OUTCML_PWRDN_M, 0x7c, unit);
    	
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_MUX_PWRUP, 1, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_UGB_PWRUP, 1, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_DIV_REFBUF_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_DIV_FB_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_DIV4_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_DIV_FBREG_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_00, I_VCO_PWRUP, 1, unit);
    	
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_REGQPPFD_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_QP_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_OUTDIV_REG_PWRDN, 0, unit);
    
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04, I_OUTDIV_IGEN_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04, I_IGEN_PWRDN, 0, unit);
            BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04, I_OUTCML_DIODE_PWRDN, 0, unit);
            BDBG_MSG(("BTNR_7125_P_PowerUp_TNR_Core0(%u) Complete\n",unit));
        }
	}
#endif

	retCode = BERR_SUCCESS;
  return retCode;
}

/*******************************************************************************************
 * BTNR_7125_P_Range_Check()	 This routine checks that parameters from the PI are in range
 *******************************************************************************************/
void BTNR_7125_P_Range_Check(BTNR_7125_Handle h)
{

	/* Range check the inputs */

	/*Check/Print Tuner Tuner Acquisition Type*/
	switch (h->pTnrModeData->TunerAcquisitionType)
	{
	case BTNR_7125_TunerAcquisitionType_eInitTune : 	
		BDBG_MSG(("Range_Check TunerAcquisitionType is Tune/Init"));
		break;
	case BTNR_7125_TunerAcquisitionType_eTune : 	
		BDBG_MSG(("Range_Check TunerAcquisitionType is Tune"));
		break;
	case BTNR_7125_TunerAcquisitionType_eMiniTune : 	
		BDBG_MSG(("Range_Check TunerAcquisitionType is MiniTune"));
		break;
	default :
		BDBG_ERR(("ERROR!!! Invalid TunerAcquisitionType selected, Value is %d",h->pTnrModeData->TunerAcquisitionType));
		break;
	}

	/*Check/Print Tuner Bandwidth*/
	switch (h->pTnrModeData->TunerBw)
	{
	case BTNR_7125_Bandwidth_e8MHz : 	
		BDBG_MSG(("Range_Check 8 MHz Channel BW selected"));
		break;
	case BTNR_7125_Bandwidth_e7MHz : 	
		BDBG_MSG(("Range_Check 7 MHz Channel BW selected"));
		break;
	case BTNR_7125_Bandwidth_e6MHz : 	
		BDBG_MSG(("Range_Check 6 MHz Channel BW selected"));
		break;
	case BTNR_7125_Bandwidth_e5MHz : 	
		BDBG_MSG(("Range_Check 5 MHz Channel BW selected"));
		break;
	default :
		BDBG_ERR(("ERROR!!! Invalid Tuner_BW selected, Value is %d",h->pTnrModeData->TunerBw));
		break;
	}

	/*Check/Print Tuner Frequency*/
	if ((h->pTnrModeData->TunerFreq < MIN_TUNER_FREQ) || (h->pTnrModeData->TunerFreq > MAX_TUNER_FREQ)) 
	{
		BDBG_ERR(("ERROR UNSUPPORTED Tuner_Freq PASSED TO BTNR_7125_P_Init_TNR_Core0()"));
	}

	BDBG_MSG(("BTNR_7125_P_Range_Check() Complete\n"));
}

/*************************************************************************************************************
* BTNR_7125_P_Tuner_Init_Buffer()	 This routine loads the BTNR_P_7125_AcquireSettings_t with default values*
* The only values not initialized are the ones passed from the PI											 *
**************************************************************************************************************/
void BTNR_7125_P_Tuner_Init_Buffer(BTNR_7125_Handle h)
{
	/*Initialize BTNR_P_7125_AcquireSettings_t Structure*/
	/*PI parameters are not initialized and are never written to*/
	/*these parameters are used locally and for BBS to sent parameters into the function*/
	h->pTnrModeData->BBS_AcqWord0 =						(uint32_t)ACQWORD0;		/*Beginning of 37 words of memory for BBS*/ 
	h->pTnrModeData->BBS_AcqWord1 =						(uint32_t)ACQWORD1;
	h->pTnrModeData->BBS_AcqWord2 =						(uint32_t)ACQWORD2;
	h->pTnrModeData->BBS_AcqWord3 =						(uint32_t)ACQWORD3;
	h->pTnrModeData->BBS_LoopVGAGain_Init_Dwell =		(uint32_t)LOOPVGAGAIN_INIT_DWELL;				 
	h->pTnrModeData->BBS_LoopVGAGain_VGA_Dwell =		(uint32_t)LOOPVGAGAIN_VGA_DWELL;	 
	h->pTnrModeData->BBS_LoopVGAGain_notVGA_Dwell =		(uint32_t)LOOPVGAGAIN_NOTVGA_DWELL;
	h->pTnrModeData->BBS_LoopVGAGain_Display =			(uint32_t)LOOPVGAGAIN_DISPLAY;
	h->pTnrModeData->BBS_Input_Source =					(uint32_t)INPUT_SOURCE;
	h->pTnrModeData->BBS_Rerun_Init =					(uint32_t)RERUN_INIT;
	h->pTnrModeData->BBS_DS_RF_AGC_Acq_BW =				(uint32_t)DS_RF_AGC_ACQ_BW;
	h->pTnrModeData->BBS_DS_IF_AGC_Acq_BW =				(uint32_t)DS_IF_AGC_ACQ_BW;
	h->pTnrModeData->BBS_DS_RF_AGC_Trk_BW =				(uint32_t)DS_RF_AGC_TRK_BW;
	h->pTnrModeData->BBS_DS_IF_AGC_Trk_BW =				(uint32_t)DS_IF_AGC_TRK_BW;
	h->pTnrModeData->BBS_DS_DCOC_On =					(uint32_t)DS_DCOC_ON;
	h->pTnrModeData->BBS_DS_DCOC_Acq_BW =				(uint32_t)DS_DCOC_ACQ_BW;
	h->pTnrModeData->BBS_DS_DCOC_Trk_BW =				(uint32_t)DS_DCOC_TRK_BW;
	h->pTnrModeData->BBS_DS_BCM3410_Auto =				(uint32_t)DS_BCM3410_AUTO;
	h->pTnrModeData->BBS_DS_BCM3410_Gain_State =		(uint32_t)DS_BCM3410_GAIN_STATE;
	h->pTnrModeData->BBS_DS_LNA_Boost_On =				(uint32_t)DS_LNA_BOOST_ON;
	h->pTnrModeData->BBS_DS_LNA_Tilt_On =				(uint32_t)DS_LNA_TILT_ON;
	h->pTnrModeData->BBS_DS_LNA_Tilt_Freq =				(uint32_t)DS_LNA_TILT_FREQ;	
	h->pTnrModeData->BBS_Force_TunerAcquisitionType =	(uint32_t)FORCE_TUNERACQUISITIONTYPE;/*End of 37 words of memory for BBS*/ 
	h->pTnrModeData->DoneLoadFlag =						BTNR_ON;
	h->pTnrModeData->DoneFirstTimeFlag =				BTNR_OFF;
	h->pTnrModeData->ReRunInit =						BTNR_OFF;
	h->pTnrModeData->Ref_PLL_Status =					0;
	h->pTnrModeData->Mixer_PLL_Status =					0;
	h->pTnrModeData->LNA_ChipID =						BTNR_LNAID_3410;

	BDBG_MSG(("BTNR_7125_P_Tuner_Init_Buffer() Complete\n"));
}

/***************************************************************************
* BTNR_7125_P_2560log10() This function calculated 256*log10(x)
***************************************************************************/
uint32_t BTNR_7125_P_2560log10(uint32_t x)
{
  int32_t  x1;
  int32_t  x2;
  int32_t  x3;
  int32_t  x4;

  if (x == 0)
    return 0;

  x1 = 30;
  while (!((x >> x1) & 1))
    x1 = x1-1;
  x1 = x1+1;

  x2 = (int32_t)((x << 8) >> x1);
  x3 = -24381739 + x2*(62348 + (x2 << 7));
  x4 = 5907991 + (x2 << 16);

  return (unsigned)((770*(x3/(x4 >> 8) + (x1 << 8))) >> 8);
}

/*******************************************************************************************
 * BTNR_7125_P_Tuner_MXR_DCO_DAC()	This routine adjusts the mixer DCO DAC	
 *******************************************************************************************/
void BTNR_7125_P_Tuner_MXR_DCO_DAC(BTNR_7125_Handle h)
{
	
	/*local variables*/
	int iDCO;
	int dco_comp_val_i, dco_comp_val_q;
	int DCO_Data_i, DCO_Data_q;
	int HR_Mode;
	unsigned unit = h->settings.unitNumber;

	/* Hit the mixer DCO resets*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_I_RST, 0x01, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_Q_QP_RST, 0x01, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_I_RST, 0x00, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_Q_QP_RST, 0x00, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_QP_I, 7, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_GMICNTL, 0x04, unit);

	/*disable input and store the state of HR path*/
	HR_Mode = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, HR_EN, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, HR_EN, 0x1, unit);	/*Force HR path*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 0x1, unit);		
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_IQ_PWRDN, 0, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, FILTER_SEL, 2, unit);
	
	BKNI_Sleep(1);
	
	/*init values for I*/
	iDCO = 0;
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_I_1P2, 0x0a, unit);
	DCO_Data_i = BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_I_1P2, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_COMP_EN_1P2, 0x1, unit);	
	
	BKNI_Sleep(1);
	
	dco_comp_val_i = BTNR_7125_P_ReadField(h->hRegister,DS_TUNER_ANACTL_0_WDATA_00, DCO_PRE_I_COMP_OUT, unit);	

	/*init values for Q*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_Q_1P2, 0x0a, unit);	
	DCO_Data_q = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_Q_1P2, unit);	
	
	BKNI_Sleep(1);
	
	dco_comp_val_q = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_00, DCO_PRE_Q_COMP_OUT, unit);	
	

	while (iDCO < 10)
	{
		dco_comp_val_i = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_00, DCO_PRE_I_COMP_OUT, unit);	
		dco_comp_val_q = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_00, DCO_PRE_Q_COMP_OUT, unit);	
		
		if(dco_comp_val_i != 1) 
		{
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_I_1P2, DCO_Data_i, unit);	
		}

		if(dco_comp_val_q != 1) 
		{		
			BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_Q_1P2, DCO_Data_q, unit);	
		}

		DCO_Data_i = (BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_I_1P2, unit) + 1);	/*DCO_Data_i = DS_TUNER_ANACTL.WDATA_13.MXRDCO_DATA_I_1P2 + 1*/
		DCO_Data_q = (BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_Q_1P2, unit) + 1);	/*DCO_Data_q = DS_TUNER_ANACTL.WDATA_13.MXRDCO_DATA_Q_1P2 + 1*/

		iDCO = iDCO + 1;

		if( (dco_comp_val_i == 1) && (dco_comp_val_q == 1) )
		{
			BDBG_MSG(("dco_comp_val_i=%d, dco_comp_val_q=%d, after %d passes out of 10 through the loop",dco_comp_val_i,dco_comp_val_q, iDCO));
			iDCO = 10;	 /*terminate loop*/
		}
	}
	
	BKNI_Sleep(1);
	
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_QP_I, 0, unit);

	DCO_Data_i = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_I_1P2, unit);
	DCO_Data_q = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, MXRDCO_DATA_Q_1P2, unit);
	BDBG_MSG(("DCO_Data_i=0x%x,DCO_Data_q=0x%x",DCO_Data_i,DCO_Data_q));

	/*re-enable input*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 0x0, unit);	
	if (HR_Mode == 0)
	{
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, HR_EN, 0x0, unit);
	}
	
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_IQ_PWRDN, 0x1, unit);	
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, I_DCO_MIX_RESET, 0x1, unit);	

	BDBG_MSG(("BTNR_7125_P_Tuner_MXR_DCO_DAC(%u) Complete\n",unit));
}

/*******************************************************************************************
 * BTNR_7125_P_Tuner_MXR_DCO_LOOP()	This routine turns on the mixer DCO loop	
 *******************************************************************************************/
void BTNR_7125_P_Tuner_MXR_DCO_LOOP(BTNR_7125_Handle h)
{
	unsigned unit = h->settings.unitNumber;
	int HR_Mode;

	/*disable input and store the state of HR path*/
	HR_Mode = BTNR_7125_P_ReadField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, HR_EN, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, HR_EN, 0x1, unit);	/*Force HR path*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 0x1, unit);

	/*power down VGA DCO*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_IDCO_PWRDN, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_QDCO_PWRDN, 0x1, unit);

	/*freeze VGA DCO*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_17, VGA_DCO_FRZ, 0x1, unit);

	/*power up mxr_DCO_loop*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_IQ_PWRDN, 0, unit);

	/*Reset QP*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_I_RST, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_Q_QP_RST, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_IQ_PWRDN, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_QP_I, 0x7, unit);	/*set QP current setting = 320nA*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_IQ_PWRDN, 0x0, unit);

	/*Clear integrator*/
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_24, 0x0, unit);	/* Clear LSB integrator I[31:0]  */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_25, 0x0, unit);	/* Clear MSB integrator I[54:23] */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_26, 0x0, unit);	/* Clear LSB integrator Q[31:0]  */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_27, 0x0, unit);	/* Clear MSB integrator Q[54:23] */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20, 0xc0b10409, unit);
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_20, 0xc0b10408, unit);

	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_20, MXR_DCO_OUT_FRZ, 0x1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_20, MXR_DCO_LIN_COEF, 0x10, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_20, MXR_DCO_INT_COEF, 0xB, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_20, MXR_DCO_OUT_FRZ, 0x0, unit);
		
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_21, 0x00000000, unit);

	BKNI_Sleep(5);

	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, DCO_PRE_QP_I, 0x5, unit);	/* set QP current setting = 200nA */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_I_RST, 0x00, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_14, DCO_MIX_Q_QP_RST, 0x00, unit);

	/*re-enable input*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, PGALPF_PWRDN, 0, unit);
	if (HR_Mode == 0)
	{
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, HR_EN, 0, unit);	/*Force HR path*/
	}

	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_20, MXR_DCO_LIN_COEF, 0x0c, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_20, MXR_DCO_INT_COEF, 0x06, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_20, MXR_DCO_OUT_EDGE_SEL, 0x0, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_20, MXR_DCO_IN_EDGE_SEL, 0x0, unit);

	BKNI_Sleep(2);

	BDBG_MSG(("BTNR_7125_P_Tuner_MXR_DCO_LOOP() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7125_P_Tuner_VGA_DCO_LOOP()	This routine turns on the VGA DCO loop	
 *******************************************************************************************/
void BTNR_7125_P_Tuner_VGA_DCO_LOOP(BTNR_7125_Handle h)
{
	unsigned unit = h->settings.unitNumber;
	/* power up VGA DCO */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_IDCO_PWRDN, 0x0, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_QDCO_PWRDN, 0x0, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_IDCO_LBS, 0x4, unit);	/* LSB range = 4 */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, VGA_QDCO_LBS, 0x4, unit);	/* LSB range = 4 */

	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_17, VGA_DCO_FRZ, 0, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_14, VGA_DCO_OUT_FRZ, 0, unit);
	
	/* set VGA DCO */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17, 0x00006093, unit); /* BW=2^-17, out={int[31], int[24:21]}, reset, freeze */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17, 0x00006090, unit); /* Unreset, unfreeze */

	BKNI_Sleep(2);

	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17, 0x000060c0, unit); /* reduce BW */
	
	BKNI_Sleep(10);

	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_TUNER_DIGCTL_0_0_DIGCTL_17, 0x000060c1, unit); /* freeze it */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_DIGCTL_0_0_DIGCTL_14, VGA_DCO_OUT_FRZ, 1, unit);

	BDBG_MSG(("BTNR_7125_P_Tuner_VGA_DCO_LOOP() Complete\n"));
}

/*******************************************************************************************
 * BTNR_7125_P_Tuner_VGA_CAL()	This routine computes the power of the DPM Tone in 256*db.

    db = 20*log10(sqrt(DPM_DCI/2^32*DPM_DCI/2^32 + DPM_DCQ/2^32*DPM_DCQ/2^32))

	fixed point implementation: 
	db = 10*log10(DPM_DCI*DPM_DCI + DPM_DCQ*DPM_DCQ) - 10*log10(1/2^64)
	db = 10*log10(((DPM_DCI*DPM_DCI)>>32 + (DPM_DCQ*DPM_DCQ)>>32))>>7) - 10*log10(1/2^25)
	256*db = 2560*log10(((DPM_DCI*DPM_DCI)>>32 + (DPM_DCQ*DPM_DCQ)>>32))>>7)) - 2560*log10(1/2^25)
	256*db = 2560*log10(((DPM_DCI*DPM_DCI)>>32 + (DPM_DCQ*DPM_DCQ)>>32))>>7)) + 19266)

	db = 10*log10(DPM_DCI*DPM_DCI + DPM_DCQ*DPM_DCQ) - 10*log10(1/2^64)
	256*db = 2560*log10(((DPM_DCI*DPM_DCI) + (DPM_DCQ*DPM_DCQ)))>>7)) - 2560*log10(1/2^57)
	256*db = 2560*log10(((DPM_DCI*DPM_DCI) + (DPM_DCQ*DPM_DCQ))))) +43926


 *******************************************************************************************/
int32_t DPM_TONE(BTNR_7125_Handle h)
{

	uint32_t DCIReg, DCQReg, ReadReg;
	uint32_t Gain_DPMDC, ulNrmHi, ulNrmLo;
	unsigned unit = h->settings.unitNumber;

	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMDCRST, 1, unit);
    BKNI_Sleep(40);

    /* Freeze the DPM DC canceller and read */
	BTNR_7125_P_WriteField(h->hRegister, DS_0_FRZ, DPMDCFRZ, 1, unit);
	/*These are 32 bit two's complement registers*/
	/*Calculate I*I*/
	ReadReg = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_TUNER_CHNZ_0_DPM_DCI, unit);
	BDBG_MSG(("CHNZ_0_DPM_DCI = 0x%08x",ReadReg));
	if ((ReadReg & 0x80000000) != 0)
	{
		if (ReadReg == 0x80000000)
		{
			ReadReg = 0x80000001; /*clamp value*/
		}
		ReadReg = ReadReg ^ 0xFFFFFFFF;	/*invert all bits*/
		ReadReg = ReadReg + 1;					/* add 1 */
	}

	/*square the I level, result will be 62 bits max*/
	BMTH_HILO_32TO64_Mul(ReadReg, ReadReg, &ulNrmHi, &ulNrmLo);
	/*Get (DPM_DCI*DPM_DCI): by keeping the 32 LSB's*/
	DCIReg = ulNrmLo;
	BDBG_MSG(("DCIReg = 0x%08x",DCIReg));

	/*Calculate Q*Q*/
	ReadReg = BTNR_7125_P_Read32(h->hRegister, BCHP_DS_TUNER_CHNZ_0_DPM_DCQ, unit);
	BDBG_MSG(("CHNZ_0_DPM_DCQ = 0x%08x",ReadReg));
	if ((ReadReg & 0x80000000) != 0)
	{
		if (ReadReg == 0x80000000)
		{
			ReadReg = 0x80000001; /*clamp value*/
		}
		ReadReg = ReadReg ^ 0xFFFFFFFF;	/*invert all bits*/
		ReadReg = ReadReg + 1;					/* add 1 */
	}
	BTNR_7125_P_WriteField(h->hRegister, DS_0_FRZ, DPMDCFRZ, 0, unit);

	/*square the Q level, result will be 62 bits max*/
	BMTH_HILO_32TO64_Mul(ReadReg, ReadReg, &ulNrmHi, &ulNrmLo);
	/*Get (DPM_DCI*DPM_DCI): by keeping the 32 LSB's*/
	DCQReg = ulNrmLo;
	BDBG_MSG(("DCQReg = 0x%08x",DCQReg));

	/*sum the I*I and Q*Q result will be 31 bits max, shift 7 to get into range of 24 bits*/
	/*Get ((DPM_DCI*DPM_DCI)>>32 + (DPM_DCQ*DPM_DCQ)>>32)>>7)*/
	BDBG_MSG(("(DCIReg+DCQReg)>>7 = 0x%08x",(DCIReg+DCQReg)>>7));
	Gain_DPMDC = BTNR_7125_P_2560log10((DCIReg+DCQReg)>>7) + 43926;
	BDBG_MSG(("Gain_DPMDC = %d",Gain_DPMDC));

    return (Gain_DPMDC);
}

typedef struct VGA_Cal
{
	uint32_t AGC_VGA[3];
	uint32_t AGC_Val;
} VGA_Cal;

/*******************************************************************************************
 * BTNR_7125_P_Tuner_VGA_CAL()	This routine calibrates VGA to set the AGC rails.
 *******************************************************************************************/
void BTNR_7125_P_Tuner_VGA_CAL(BTNR_7125_Handle h)
{
	unsigned long rfFreq =  h->settings.rfFreq; /* in Hz */
	VGA_Cal *VGA_Cal_Table;

	int i=0;
	int iMax=0;
	unsigned long AGC_VAL;
	int AGC_STEP = 42949672;  /* roughly 25 mV step = 1 dB */
	bool DataValid = false;
	int32_t MinPower=0, Power, MaxPower=0;
	unsigned unit = h->settings.unitNumber;
	/* values from BcmInternalTuner.c in eCM_3.0.1alpha2_08052010 */
	uint32_t VGA_RANGE = 256*20;
	int32_t VGA_SLOPE = 42949672;
	int32_t Min_AGC_Rail = 0x0;
	int32_t Max_AGC_Rail = 0x7fffffff;
	int32_t Mid_AGC_Threshold = 0x280cb295;    /* Mid range of AGC */
	
	BDBG_MSG(("Starting VGA Calibration"));
	
	h->pTnrModeData->Min_AGC_Threshold = 0x22BFDB4C;
	h->pTnrModeData->Max_AGC_Threshold = 0x2D32B55E;

	BTNR_7125_P_PowerDown_LNA_Output(h, true);	/* Shut down the LNA output for calibration */
	
	/* Freeze the IF AGC loop */
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC_MISC, AGCIFRZ, 1, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC1, AGCIZ, 1, unit);
	
	/* Set the IF AGC Integrator upper and lower limits to allow full range sweep */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AIFLT, 0x80000000, unit);
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AIFUT, 0x7fffffff, unit);
	
	/* Initialize the IF AGC Integrator */
	BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, 0, unit);
	
	/* Set the IF AGC Bandwidth */
	BTNR_7125_P_WriteField(h->hRegister, DS_QAFE_IF_0_0_AGC2, AGCIFBW, 0x10, unit);
	
	/* Force the Fixed gain stages to maximum gain */
	#if 0
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_SPLIT_00, RF_ATT, 0, unit);	/* Overwrite 3410_init setting. Rayz: OK to leave this out */
	#endif
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, RFTRK1_GAIN, 3, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, MIX_ATT, 0x00, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_03, FGA_GAIN, 0x07, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_04, OUTVGA_ATT, 0x00, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_I_PGASET, 0x00, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_QDSAFE_0_0_CTRL1, INT_Q_PGASET, 0x00, unit);
	
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_04, I_OUTDIV_M6, 0x85, unit);		 

	/* Reset the DPM tone output divider */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_OUTDIV_RESET_M, 0x40, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_REF_0_REF_01, I_OUTDIV_RESET_M, 0x00, unit);

	/*Enable the DPM generator and configure the gain*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_RST, 1, unit); 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_RST, 0, unit);		 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_AMP, 3, unit); /* set smaller amp/gain */
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, DPM_6DB_GAIN, 1, unit);		 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_PWRDN, 0, unit); /* power up */
	
	if (rfFreq < 330000000)
	{
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, EN_DC_DPM, 1, unit); /* increase RFF Att first */
	}
	else
	{
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_02, EN_DC_DPM, 0, unit);
	}

	/*
	DPM_freq =  1.0 (This value, in MHz, is offset from the center of the channel being used.)
	DPM_FCW = ((16777216.0 * DPM_freq) / 29.25) = 8C08C
	DPM_FCW = DPM_FCW & 0x00FFFFFF = 8C08C   (FCW is 24 bits)
	DPM_FCW = DPM_FCW + 3 = 8C08F            (Offset FCW for better averaging)
	DPM_FCW = 8C08F
	*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMNOTCHBWC, 0x05, unit);
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMDCBYP, 0, unit);	/*unbypass*/
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMFCW, 0x8C08F, unit); /*DPM_FCW*/ 
	BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_CHNZ_0_DPM, DPMDCRST, 1, unit);	/*reset the DPM integrator*/
	BTNR_7125_P_WriteField(h->hRegister, DS_0_FRZ, DPMDCFRZ, 0x0, unit);

	VGA_Cal_Table = (VGA_Cal *)BKNI_Malloc(sizeof(VGA_Cal[50]));

	if (VGA_Cal_Table)
	{
		while (i < 50)
		{
		   AGC_VAL = i*AGC_STEP;
		   BTNR_7125_P_Write32(h->hRegister, BCHP_DS_QAFE_IF_0_0_AGCII, AGC_VAL, unit);
		   BKNI_Sleep(100);
		   Power = DPM_TONE(h);
		   if (i == 0)
		   {
			   MinPower = Power;
			   MaxPower = Power;
		   }
		   /*  Keep Track of DPM Power limits over sweep */
		   if (Power < MinPower)
			   MinPower = Power;
		   else if (Power > MaxPower)
			   MaxPower = Power;
		
		   /* Save the current DPM power and corresponding AGC Integrator */
		   VGA_Cal_Table[i].AGC_VGA[0] = i;
		   VGA_Cal_Table[i].AGC_VGA[1] = AGC_VAL;
		   VGA_Cal_Table[i].AGC_VGA[2] = Power;
		   VGA_Cal_Table[i].AGC_Val = AGC_VAL;
		   iMax = i;
		
		   /* Escape when max rail is reached to save time */
		   if ((MaxPower - MinPower) > 10) 
		   {
			   if ((Power - VGA_Cal_Table[i-1].AGC_VGA[2]) < 77) /* < 0.3 dB, our Power is dB*256 */
				   i = 50;
		   }
		   i = i+1;
		}
		
		BDBG_MSG(("AGC Sweep Complete"));
		
		/* Now turn off DPM signal */
		BTNR_7125_P_WriteField(h->hRegister, DS_TUNER_ANACTL_0_WDATA_01, DPM_PWRDN, 1, unit);	
	
		BTNR_7125_P_PowerDown_LNA_Output(h, false);	/* Re-Enable the LNA after completion of the sweep  */
		
		/*  Print raw values for debug */
		for (i=0; i<iMax; i++)
			BDBG_MSG(("%u\t%08x\t%u", VGA_Cal_Table[i].AGC_VGA[0], VGA_Cal_Table[i].AGC_Val, VGA_Cal_Table[i].AGC_VGA[2]));
		
		BDBG_MSG(("\nFinding Lower AGC Rail"));
		
		/* Find the first data point more than 1 dB above the minimum power */
		i = 1;
		while (i < (iMax + 1))
		{
			if ( (VGA_Cal_Table[i].AGC_VGA[2] - MinPower) > (1*256) )
			{
				Min_AGC_Rail = VGA_Cal_Table[i].AGC_Val - AGC_STEP;
				i = iMax+1;
			}
			else
				i++;
		}
		BDBG_MSG(("Min_AGC_Rail = 0x%08x", Min_AGC_Rail));
		
		BDBG_MSG(("Finding Upper AGC Rail"));
		
		/* Find the first data point within 1 dB of the maximum power */
		i = iMax - 1;
		while (i > 0)
		{
			if ( (MaxPower - VGA_Cal_Table[i].AGC_VGA[2]) > (1*256) )
			{
				Max_AGC_Rail = VGA_Cal_Table[i].AGC_Val + AGC_STEP;
				i = 0;
			}
			else
				i--;
		}
		BDBG_MSG(("Max_AGC_Rail = 0x%08x", Max_AGC_Rail));
		
		/* Validate Calibration Data */
		
		if ( (MaxPower - MinPower) > (16*256))
		{
		   if ((Max_AGC_Rail - Min_AGC_Rail) > (10*AGC_STEP))
		   {
			   DataValid = true;
			   BDBG_MSG(("VGA Calibration PASSED"));
			   VGA_RANGE = MaxPower - MinPower;
			   /* VGA_SLOPE Units are Counts/256*dB */
			   VGA_SLOPE = (Max_AGC_Rail - Min_AGC_Rail)/VGA_RANGE;
			   /* Mid_Range of AGC */
			   Mid_AGC_Threshold = (Max_AGC_Rail + Min_AGC_Rail)/2;
			   /* Lower AGC Threshold = 4 dB below mid */
			   h->pTnrModeData->Min_AGC_Threshold = Mid_AGC_Threshold - 4*256*VGA_SLOPE;
			   /* Upper AGC Threshold = 4 dB above mid */
			   h->pTnrModeData->Max_AGC_Threshold = Mid_AGC_Threshold + 4*256*VGA_SLOPE;
			}
		}
		BKNI_Free(VGA_Cal_Table);
	}
	else
	{
		BDBG_ERR(("Failed to alloc VGA_Cal_Table[]"));
	}
	if (DataValid == false)
	{
		BDBG_WRN(("VGA Calibration FAILED, Using Defaults"));
		h->pTnrModeData->Min_AGC_Threshold = 	DS_MIN_AGC_THRESHOLD;
		Mid_AGC_Threshold = 					DS_MID_AGC_THRESHOLD;
		h->pTnrModeData->Max_AGC_Threshold = 	DS_MAX_AGC_THRESHOLD;
		Min_AGC_Rail = 							DS_MIN_AGC_RAIL;
		Max_AGC_Rail = 							DS_MAX_AGC_RAIL;
		VGA_RANGE = 							256*20;
		VGA_SLOPE = 							42949672;

	}
	
	BDBG_MSG(("MaxPower = %u.%u", MaxPower/256,MaxPower%256));
	BDBG_MSG(("MinPower = %u.%u", MinPower/256,MinPower%256));
	BDBG_MSG(("Min AGC Threshold = 0x%08x", h->pTnrModeData->Min_AGC_Threshold));
	BDBG_MSG(("Mid AGC Threshold = 0x%08x", Mid_AGC_Threshold));
	BDBG_MSG(("Max AGC Threshold = 0x%08x", h->pTnrModeData->Max_AGC_Threshold));
	BDBG_MSG(("Min_AGC_Rail = 0x%08x", Min_AGC_Rail));
	BDBG_MSG(("Max_AGC_Rail = 0x%08x", Max_AGC_Rail));
	BDBG_MSG(("VGA Range = %u.%u dB", VGA_RANGE/256,VGA_RANGE%256));
}



/*******************************************************************************************
 * BTNR_7550_P_Tuner_Device_type()	 This routine read the otp to see if it is 60 or 65nm
 *******************************************************************************************/
void BTNR_7125_P_Tuner_Device_type(BTNR_7125_Handle h)
{
	
	uint32_t temp, bit_mask, bit_value;
	
	/* Set the jtag_otp_cpu_mode bit high to provide a free running clock to the OTP module */
	BTNR_7125_P_WriteField(h->hRegister, JTAG_OTP_GENERAL_CTRL_1, jtag_otp_cpu_mode, 1, 0);
	temp = BREG_Read32(h->hRegister, BCHP_JTAG_OTP_GENERAL_STATUS_1);
    temp = BREG_Read32(h->hRegister, BCHP_JTAG_OTP_GENERAL_CTRL_1); 
	BREG_Write32(h->hRegister, BCHP_JTAG_OTP_GENERAL_CTRL_1, (temp | (0x65 << 16)));  /* Read OTP CPU STATUS */
	BREG_Write32(h->hRegister, BCHP_JTAG_OTP_GENERAL_CTRL_0, 0); /* set otp_prog_en high and Make sure Start = 0 */
    temp = BREG_Read32(h->hRegister, BCHP_JTAG_OTP_GENERAL_CTRL_0);
    BREG_Write32(h->hRegister, BCHP_JTAG_OTP_GENERAL_CTRL_0, (temp | 0x00000001));
    temp = BREG_Read32(h->hRegister, BCHP_JTAG_OTP_GENERAL_STATUS_0);
    bit_mask = 0x1 << 5;
    bit_value = (temp & bit_mask) >> 5;
    BDBG_MSG(("BTNR_7125_P_Tuner_Device_type() : %d \n", bit_value));
    h->pTnrStatus->Device_Tech_type = bit_value;
}


