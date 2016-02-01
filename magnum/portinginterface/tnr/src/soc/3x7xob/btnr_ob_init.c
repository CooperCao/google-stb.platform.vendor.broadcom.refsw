/*************************************************************************
*     (c)2005-2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
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
#include "bkni.h"
#include "bmth.h"
#include "btmr.h"
#include "btnr.h"
#include "bdbg.h"
#include "bmem.h"
#include "btnr_priv.h"
#include "btnr_ob_3x7x.h"
#include "btnr_3x7xob_priv.h"
#include "bchp_ufe_oob.h"
#include "bchp_ufe_misc.h"
#include "bchp_ufe_saw_oob.h"
#include "btnr_ob_struct.h"
#include "bchp_obadc.h"

#include "btnr_ob_init.h"
#include "btnr_ob_tune.h"

#ifndef LEAP_BASED_CODE
BDBG_MODULE(btnr_ob_init);
#define POWER2_31 2147483648UL
#define POWER2_16 65536
#define LOG10_POWER2_9_X5120 13871
#define Twos_Complement32(x) ((uint32_t)((x ^ 0xFFFFFFFF) + 1))
#endif

#define BCHP_UFE_MISC_CTRL_CLKGEN_DATA_RESET_SHIFT     BCHP_UFE_MISC_CTRL_DATA_RESET_SHIFT
#define BCHP_UFE_MISC_CTRL_CLKGEN_DATA_RESET_MASK  (BCHP_UFE_MISC_CTRL_DATA_RESET_MASK | BCHP_UFE_MISC_CTRL_CLKGEN_RESET_MASK)

#define OOB_PHYPLL6_FREQ  540000000UL /* 526500000UL */



/******************************************************************************
*  BTNR_P_TnrInit
******************************************************************************/
BERR_Code BTNR_Ob_P_TnrInit(BTNR_Ob_3x7x_Handle hTnr, uint32_t RfFreq)
{
	
	BREG_WriteField(hTnr->hRegister, OBADC_CNTL3, PWRUP, 1);
	BREG_WriteField(hTnr->hRegister, OBADC_CNTL3, RSTB, 0);
	BREG_WriteField(hTnr->hRegister, OBADC_CNTL3, RSTB, 1);

	BTNR_Ob_P_Program_TNR(hTnr, RfFreq);
	return BERR_SUCCESS;
}


/******************************************************************************
*  BTNR_Ob_P_Program_Back_DDFS()
******************************************************************************/
void BTNR_Ob_P_Program_Back_DDFS(BTNR_Ob_3x7x_Handle hTnr, uint32_t RfFreq)
{

	uint32_t ulMultA, ulMultB, ulNrmHi, ulNrmLo, ulDivisor;

	ulMultA = 2*(uint32_t)RfFreq;
	ulMultB = POWER2_31;
	ulDivisor = OOB_PHYPLL6_FREQ;
	BMTH_HILO_32TO64_Mul(ulMultA, ulMultB, &ulNrmHi, &ulNrmLo);
	BMTH_HILO_64TO64_Div32(ulNrmHi, ulNrmLo, ulDivisor, &ulNrmHi, &ulNrmLo);

	/*Write FCW value*/
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_FMIX_FCW, Twos_Complement32(ulNrmLo));


}

/******************************************************************************
*  BTNR_Ob_P_Program_TNR()
******************************************************************************/
void BTNR_Ob_P_Program_TNR(BTNR_Ob_3x7x_Handle hTnr, uint32_t RfFreq)
{
	uint8_t  CoeffSet, Index;

	BDBG_MSG(("BTNR_Ob_P_Program_TNR"));

	 
	if (BREG_ReadField(hTnr->hRegister, UFE_MISC_CTRL, CLKGEN_DATA_RESET) != BCHP_UFE_MISC_CTRL_CLKGEN_DATA_RESET_MASK)
		BREG_WriteField(hTnr->hRegister, UFE_MISC_CTRL, CLKGEN_DATA_RESET, 0x0); /* clear data/clkgen reset*/
	/*HRC is bypassed*/
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_CTRL,			0x001B0024);
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_BYP,				0x00001004);
		  BDBG_MSG(("BTNR_Ob_P_Program_TNR 1"));
	BTNR_Ob_P_Program_Back_DDFS(hTnr, RfFreq);

	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_RST,             0xC0000FFF );        /* reset the clock gen after BYP/CIC programming*/
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_RST,             0x40000FFF );        /* clear clock gen reset, but hold datapath reset*/
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_DCO_CTRL,        0x00000008 );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_AGC1,            0x0000000D );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_AGC2,            0x0000000B );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_AGC3,            0x00000009 );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_AGC1_THRESH,     0x00033333 );        /* 15dB back-off dec2hex(num2tc(10^(-15/10),2,21,0))*/
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_AGC2_THRESH,     0x00033333 );        /* 15dB back-off dec2hex(num2tc(10^(-15/10),2,21,0))*/
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_AGC3_THRESH,     0x00033333 );        /* 15dB back-off dec2hex(num2tc(10^(-15/10),2,21,0))*/
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_IQIMB_AMP_CTRL,  0x00000007 );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_IQIMB_PHS_CTRL,  0x00000007 );

	for (CoeffSet=0, Index=0;Index<BTNR_OB_SAW_COEFF_SIZE;Index++)
	BREG_Write32(hTnr->hRegister, BCHP_UFE_SAW_OOB_COEFF0+(4*Index), BTNR_Ob_SAW_Table[CoeffSet][Index]);


	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_AGC1_LF,         0x06000000 );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_AGC2_LF,         0x02000000 );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_AGC3_LF,         0x20000000 );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_IQIMB_AMP_LF,    0x00000000 );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_IQIMB_PHS_LF,    0x00000000 );
	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_FRZ,             0x00000000 );
	BREG_WriteField(hTnr->hRegister, UFE_OOB_CTRL, VID_QUANT,   0x4);

	BREG_Write32(hTnr->hRegister, BCHP_UFE_OOB_RST,             0x00000000 );        /* clear any remaining data reset*/
	BREG_WriteField(hTnr->hRegister, UFE_MISC_OOB_CTRL, RESET, 0x1);
	BREG_WriteField(hTnr->hRegister, UFE_MISC_OOB_CTRL, RESET, 0x0);
	BREG_WriteField(hTnr->hRegister, UFE_MISC_CTRL, BYP_HRC,	1 );   

}


