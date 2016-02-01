/***************************************************************************
 *     Copyright (c) 2005-2010, Broadcom Corporation
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
#include "bchp_3548.h"
#include "bkni.h"
#include "bvsb.h"
#include "bvsb_priv.h"
#include "bvsb_3548_priv.h"
#include "bvsb_3548_ifd.h"
#include "bvsb_3548_ifdcoef.h"

/* Register defintion from RDB */
#include "bchp_ap_32.h"
#include "bchp_ifd_ni.h"
#include "bchp_ifd_core.h"
#include "bchp_ifd_top.h"
#include "bchp_ifd_core_a2.h"
#include "bchp_ifd_ss.h"
#include "brsp.h"
#include "bchp_ds_afe.h"
#include "bchp_clkgen.h"

/* For math processing */
#include "bmth.h"

BDBG_MODULE(BVSB_3548_ifd);

/* IFD Inputs */
/* #define AGC_DEBUG */
#define PULL_IN_RANGE 0
#define RF_AGC_BW 0xA
#define IF_AGC_BW 0x2
#define AGC_DELAY 100
#define AGC_SETUP_1 0x0
#define AGC_TOP 0x5A80
#define AGC_IF_FIXED 0x9300
#define AGC_RF_FIXED 0
#define IF_FREQ 45750000

/******************************************************************************
  BVSB_P_3548_IFD_Enable_New_SyncStrip()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Ifd_Enable_New_SyncStrip(
        BVSB_Handle h,
        BVSB_IfdMode eVideoMode
        )
{
    /* IFD_TOP.IF_CNTL.HSYNC_SEL and .LOCK_SEL moved to init above*/
    /*   NOTE: to use VDEC SS to drive PD mode, then IF_CNTL.LOCK_SEL= 0,  FE_TMUXC.VDEC_LOCK_SEL= 1*/
    /*         to use IFD  SS to drive PD mode, use any of the three other combinations*/

    /*  AGC keying parameters set up here based on the HSYNC and VSYNC frequencies.*/
    /*  FE_AWIN notes:*/
        /*  NOTE: unlike past IFD's (pre 3560B?), there is not a setting of FE_AWIN*/
        /*  which works for both Field 1 and 2 Vert windows.  Thus the new value*/
        /*  below extends over Field 1 Lines 10&11, which shouldn//t really affect perf.*/
        /*    3560A and earlier: DELAY_CYCLES based on 54 MHz cntr*/
        /*    3560B and later  : DELAY_CYCLES based on 27 MHz cntr*/
    /* Key offset notes: The time difference between Fh=15734 Hz and Fh= 15625 Hz is 443ns*/
    /*    which is 6 samples delays at 13.5 MHz.  However, there seems to be a few extra*/
    /*    sample delays based on the initial SS freq (why?).  The total is about*/
    /*    9 samples which is coincidentally the DC group delay between the NTSC and*/
    /*    PAL filters, thus the key delays are the same between the modes.  The*/
    /*    exception is "N", which is uses "M" GDF and non-"M" Fh=15625.*/

    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    if (eVideoMode <= 0x02)   /* 0,1,2 [$$$3548A0]  "M" standards & reserved*/
    {

        BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_HCNT,    0x069AD6B4);  /* .HCNT_MAX= 1716, .HCNT_MID= 858  for Fh= 15.734 kHz  ("M")*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AWIN,   0x0C8087F0);
             /* Changed VSYNC window start to match VDEC_FE_0.AGC_REGION.END= 124*/
	         /*   .DELAY_CYCLES =  800    [TLS 20080428]  was 7464*/
	         /*   .VDEC_FRZ_PER = 34800  (not changed)*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_APDCV,  0xE8C64BD8);
             /* .UPPER_CNTR_VAL  = 0xC64  // &HC40 + 4*9  [TLS 20081112] */
	         /* .LOWER_CNTR_VAL  = 0xBD8  // &HBB4 + 4*9  [TLS 20081112] */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AVPDCV, 0x60C04BD8);
             /* .UPPER_CNTR_VAL = 0xC04  // &HBE0 + 4*9  [TLS 20081112] */
             /* .LOWER_CNTR_VAL = 0xBD8  // &HBB4 + 4*9  [TLS 20081112] */
    }
	else if  (eVideoMode == 0x03)   /* 3 [$$$3548A0] "N"   [TLS 20080505]*/
    {
        /* SS same as 15625 line PAL standards.  However since "M" GDF used in this mode,*/
        /* need to delay keys by 9 samples (13.5 MHz) to match the PAL GDF delay.*/
        /* Note the counters are based on 54 MHz though.*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_HCNT,    0x069B06C0);  /* .HCNT_MAX= 1728, .HCNT_MID= 864  for Fh= 15.625 kHz*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AWIN,   0x0C8087F0);
             /* Changed VSYNC window start to match VDEC_FE_0.AGC_REGION.END= 124*/
	         /*   .DELAY_CYCLES =  800    [TLS 20080428]  was 7464*/
	         /*   .VDEC_FRZ_PER = 34800  (not changed) Could reduce 2 lines for PAL, but OK*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_APDCV,  0xE8C88BFC);
        /*  ' .UPPER_CNTR_VAL = 0xC88  // &HC40 + 4*9 + 4*9 [TLS 20081112] */
        /* ' .LOWER_CNTR_VAL = 0xBFC  // &HBB4 + 4*9 + 4*9 [TLS 20081112] */

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AVPDCV, 0x60C28BFC);
        /* ' .UPPER_CNTR_VAL = 0xC28  // &HBE0 + 4*9 + 4*9 [TLS 20081112] */
        /* ' .LOWER_CNTR_VAL = 0xBFC  // &HBB4 + 4*9 + 4*9 [TLS 20081112] */
    }
    else if  (eVideoMode == 14)   /*   positive modulation video (only "L" for now)*/
	{
	    /*!!! need to change HCNT to Steve's settings???*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_HCNT,    0x069B06C0);  /* .HCNT_MAX= 1728, .HCNT_MID= 864  for Fh= 15.625 kHz*/

		/* from stj's SECAM script modified for VDEC_FE_0.AGC_REGION.END= 0x7C (124)*/
		/* keying OK for both VSYNC regions (lines 1 & 313) (NOTE: a few lines not updated at all)*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AWIN, 0x360038A4);
	         /*   .DELAY_CYCLES = 1728 * 2*/
	         /*   .VDEC_FRZ_PER = 14500*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_APDCV,  0xE8DA2CE4);
	       /* .UPPER_CNTR_VAL = 0xDA2  // 3490*/
	       /* .LOWER_CNTR_VAL = 0xCE4  // 3300*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AVPDCV, 0x60B4CB04);
	       /* .UPPER_CNTR_VAL = 0xB4C  // 2820 + 72*/
	       /* .LOWER_CNTR_VAL = 0xB04  // 2820*/
	}
	else      /* all other modes (formerly just PAL-D)*/
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_HCNT,    0x069B06C0);  /* .HCNT_MAX= 1728, .HCNT_MID= 864  for Fh= 15.625 kHz*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AWIN,   0x0C8087F0);
             /* Changed VSYNC window start to match VDEC_FE_0.AGC_REGION.END= 124*/
	         /*   .DELAY_CYCLES =  800    [TLS 20080428]  was 7464*/
	         /*   .VDEC_FRZ_PER = 34800  (not changed) Could reduce 2 lines for PAL-D, but OK*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_APDCV,  0xE8C40BB4);
	       /* .UPPER_CNTR_VAL = 0xC40  // 3136*/
	       /* .LOWER_CNTR_VAL = 0xBB4  // 2996*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AVPDCV, 0x60BE0BB4);
	       /* .UPPER_CNTR_VAL = 0xBE0  // 3040*/
	       /* .LOWER_CNTR_VAL = 0xBB4  // 2996*/
    }

    /* BCHP_IFD_SS regs from dump after running BBS init*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_TMUX_RVAL,      0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_TMUX_CTRL,  0x80000);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_LFCS, 0x00010190);  /* [TLS 20080422] back to &h00010190& (see history comments)*/
        /* .INT_COEF = 0x1         // 1*/
        /* .LIN_COEF = 0x190       // 400*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_AVG_CTRL,    0x1A0000BD);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_ROLLO_CTRL,  0xD3A79B0D);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_LF_INT,             0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_LF_UPTHRESH,     0xCCCC);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_LF_LWTHRESH, 0xFFFF3334);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_MID_HCNT,         0x400);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_LD_COMP,       0x927C00);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_LD_COEF,       0x309988);
    BREG_Write32(p3548->hRegister, BCHP_IFD_SS_SS_LD_CIC,          0x0100);    /* [$$$3563A0]*/
      /* .CIC_511_DIS = 0   // 0=En, 1=Dis (backwards compatible)*/
	  /* .CIC_511_RST = 0*/
	  /* .CIC_511_COMP , 0x100  // midpoint of [0, 0x1ff] empirically set by stj and tls*/
      /* [$$$3563A0] BCHP_IFD_SS hardware change logic :*/
      /*   .SS_STATUS.LOCK_DETECT = "SS LOW VAR LOCK" AND (.SS_STATUS.NOVIDEO_DETECT OR .SS_LD_CIC.CIC_511_DIS)*/
      /* NOTE: NOVIDEO_DETECT false locks with no signal and high var noise, but works with noiseless no signal.*/
      /*      "SS LOW VAR LOCK" has the opposite problem, so the 2 together make a reliable sync detect.*/
      /* *** use BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_APDCTL.ADC_REGIN_SEL=1 to zero out A/D to test NOVIDEO_DETECT.*/

    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_Init_IFD_AGC()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Init_IFD_AGC(
        BVSB_Handle h,
        BVSB_IfdMode eVideoMode
        )
{
	uint8_t buffer[3];
    uint8_t data0, ntsc_agc_setup1, rfBandwidth, ifBandwidth, delay;
    uint32_t lval1, lval2, ntsc_agc_top, ntsc_agc_if_fixed, ntsc_agc_rf_fixed;
    uint32_t regVal;

    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

	/* Get these setting from AP, so that BBS/Nexus can both update it */
	BVSB_ReadConfig(h, BVSB_CONFIG_NTSC_TER_SETUP1, &ntsc_agc_setup1, 1);

	BVSB_ReadConfig(h, BVSB_CONFIG_NTSC_TER_SETUP3, &delay, 1);

	BVSB_ReadConfig(h, BVSB_CONFIG_NTSC_TER_SETUP4, buffer, 1);
	rfBandwidth = ((buffer[0] & 0xF0) >> 4);
	ifBandwidth = (buffer[0] & 0xF);

	BVSB_ReadConfig(h, BVSB_CONFIG_NTSC_TER_SETUP5, buffer, 2);
	ntsc_agc_top = (buffer[0] << 8) | (buffer[1]);
	BDBG_MSG(("AGC TOP = %x\n", ntsc_agc_top));

	BVSB_ReadConfig(h, BVSB_CONFIG_NTSC_TER_SETUP7, buffer, 2);
	ntsc_agc_if_fixed = (buffer[0] << 8) | (buffer[1]);
	BDBG_MSG(("IF AGC FIXED = %x\n", ntsc_agc_if_fixed));

	BVSB_ReadConfig(h, BVSB_CONFIG_NTSC_TER_SETUP9, buffer, 2);
	ntsc_agc_rf_fixed = (buffer[0] << 8) | (buffer[1]);
	BDBG_MSG(("RF AGC FIXED = %x\n", ntsc_agc_rf_fixed));


      BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ADSFCNT, 0x00030003); /*.DS_APGA_FCNT = 3, .DS_RF_FCNT = 3 */
      BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_WTACQ, 0x00010844);  /*.SDM_IF_SEL=0  (dflt) 1st or 2nd order on IF pin */
      BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_MISCTL, 0x70010001);  /* RF/IF/APGA RTZ=1, RF=1st, IF=2nd */

      /* Change pin muxing */
      /*TODO: Remove this if not neededif((ntsc_agc_setup1 & 0x08) && (ntsc_agc_if_fixed==0x0000)) */

      if (h->settings.bRfAgcZero)
      {
          ntsc_agc_setup1 |= 0x80;     /* force IF FRZ bit */
          ntsc_agc_rf_fixed = 0x0000;  /* force fixed level to 0 */
      }

      if (h->settings.bIfAgcZero)
      {
          /* Special case to force 1st ord IF AGC all 0's output (no pin toggling) */
          BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ADSFCNT, 0x00030003); /*.DS_APGA_FCNT = 3, .DS_RF_FCNT = 3 */
          BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_WTACQ, 0x00010844);  /*.SDM_IF_SEL=0  (dflt) 1st or 2nd order on IF pin */
          BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_MISCTL, 0x70000001);  /* RF/IF/APGA RTZ=1, RF=1st, IF=1st */
          ntsc_agc_setup1 |= 0x08;     /* force IF FRZ bit */
          ntsc_agc_if_fixed = 0x0000;  /* force fixed level to 0 */
      }
      else if (h->settings.IfAgcPinMode == BVSB_AgcPinMode_e25V)
      {
          BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ADSFCNT, 0x00030003); /*.DS_APGA_FCNT = 3, .DS_RF_FCNT = 3 */
          BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_WTACQ, 0x00010844);  /*.SDM_IF_SEL=0  (dflt) 1st or 2nd order on IF pin */
          BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_MISCTL, 0x70010001);  /* RF/IF/APGA RTZ=1, RF=1st, IF=2nd */
      }
      else   /* BTHD_AgcPinMode_e33V    (use 4th order IF AGC) */
      {
          BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ADSFCNT, 0x00030000); /*.DS_APGA_FCNT = 0, .DS_RF_FCNT = 3 */
          BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_WTACQ, 0x00010944);  /*.SDM_IF_SEL=1  4th order on IF pin */
          BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_MISCTL, 0x70010001);  /* RF/IF/APGA RTZ=1, RF=1st, IF=2nd */
      }


/*!!!!!! NEEDS TO BE FIXED...JUST HARDCODED VALUES FOR NOW, look for //!*/
    /* ********** AGC Setup table registers*/
    /*  The following commented out hardcoded reg writes are modified later using AGC table (!!! what about OD, etc???)*/
    /*  Refer to CONFIG.XML for default values set in HAB Setup init*/
    /* BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ARFTHR  , 0x10000000); // for IF_TOP  // AGC FIX*/

    /* Dig AGC Freeze (dis clip prevent), Integrator= 1.0, (large A/V or N-1 handled by wideband AGC) */
    /* Peak Detect mode enabled, loop BW adjustment for peak detect mode */
    if (eVideoMode == 14)  /*  positive modulation video (only "L" for now)*/
	{
       BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ACTL, 0x1A002021);  /* .AGC_AGDZ=1, .AGC_AGAZ=0, .AGC_AGRZ=0, .PD_MODE_SEL=0*/
       BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_VARDEL, 0x7C801300);  /*.CORDIC_SCALE=  76 (0.3 * 256), .CORDIC_COMB_EN = 0  ("PeakDet")*/
    }
    else                 /*  negative modulated video*/
	{
       BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ACTL, 0x1A002023);  /* .AGC_AGDZ=1, .AGC_AGAZ=0, .AGC_AGRZ=0, .PD_MODE_SEL=1*/
       BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_VARDEL, 0x7C803800);  /*  .CORDIC_SCALE= 224, .CORDIC_COMB_EN = 0  ("PeakDet")*/
	}
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINDPGA, 0x88000000); /* 1.0 integrator init*/

    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_CTL2);
    regVal &= 0xFE3FFFFF;
    regVal |= 0x00C00000; /* .SAT_LIN_PROD = 3 */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_CTL2, regVal);

    /* BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINAPGA , 0x00000000);  // AGC FIX*/
    /* BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINI    , 0x00000000);  // AGC FIX*/
    /* BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINR    , 0x00000000);  // AGC FIX*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINX    , 0x03F91510);  /* AGC FIX*/

    /* [TLS 20080422] Add hooks to support Digital AGC BW settings (even if not enabled)*/
    data0 = 0x02;   /* nom Dig AGC BW (later will be HAB AGC param using reserved fields?)*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ADPGAINTCS, (data0 << 5) ); /* .TRK_COEF [9:5] (all other fields set to 0)*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINTVCS,  data0);   /*  .TRK_1_VCOEF[4:0] init, other fields 0 till later*/


    /* RF AGC BW*/
    lval1 = rfBandwidth;
    /* .TRK_COEF [9:5] (all other fields set to 0)  ([7:4] moved to [8:5])*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ARFINTCS, lval1 << 5);

	/* RF vert. AGC is same BW*/
    lval2= (BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_ATHRNVCS) & 0xffe0ffff);  /* [20:16] cleared*/
    lval2 |= (lval1 << 16);   /* [4:0] --> [19:16] - DCL: note its bits 4:0 which is different from AP code */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ATHRNVCS, lval2);    /* .TRK_1_VCOEF=[20:16] written*/

    /* IF AGC BW     (write both "APGA" and "IF" for future compatibility!)*/
    lval1 = ifBandwidth;
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AAPGAINTCS, lval1 << 5); /* .TRK_COEF[9:5] (other fields=0)*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AIFINTCS  , lval1 << 5); /* .TRK_COEF[9:5] (other fields=0)*/

	/* IF vert. AGC is same BW (write both "APGA" and "IF" for future compatibility!)*/
    lval2= BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_AINTVCS) & 0xfe0f83ff;  /* [24:20], [14:10] cleared*/
    lval2 |= (lval1 << 10);   /* [4:0] --> .TRK_2_VCOEF[14:10] (APGA vert. AGC)*/
    lval2 |= (lval1 << 20);   /* [4:0] --> .TRK_3_VCOEF[24:20] ("IF" vert. AGC)*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINTVCS, lval2);

        /* IF_TOP setup  (MSB flipped and moved to bit 28)*/
        lval2 = (unsigned long)(ntsc_agc_top ^ 0x8000) << 13;  /* [15]-->[28]*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ARFTHR, lval2);

		if (ntsc_agc_setup1 & 0x80) /* RF is frozen, output the fixed RF level*/
		{
            lval1= BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_AINX);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINX, lval1 | 0x00000008); /* [3]= .ARFINTG_FRZ = 1*/
            BKNI_Sleep(10);  /* otherwise fixed value will drift by a few LSB's before freezing???*/
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINR, (unsigned long)(ntsc_agc_rf_fixed ^ 0x8000) << 16);
		}

	  	if(ntsc_agc_setup1 & 0x08) /* IF is frozen, output the fixed IF level, error to RF*/
   		{
            /*  [10]= .RF_INTG_IN_SEL = 0  ' bypass the cascade error for the RF*/
            /*   [2]= .AIFINTG_FRZ    = 1  ' frz both IF regs for future compat.*/
            /*   [1]= .AAPGAINT_FRZ   = 1  ' frz both IF regs for future compat.*/
            lval1= BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_AINX);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINX, (lval1 & 0xFFFFFBF9) | 0x00000006);

            /* the error threshold must be set to 0 in this mode*/
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ARFTHR, 0x00000000);

            BKNI_Sleep(10);  /* otherwise fixed value will drift by a few LSB's before freezing???*/
			lval1 = (unsigned long)(ntsc_agc_if_fixed ^ 0x8000) << 16;
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINI,    lval1);  /* both IF regs written for future compat.*/
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_AINAPGA, lval1);  /* both IF regs written for future compat.*/
		  }

    /* Dwell AGC time, then freeze IF & RF values at "good" value */
	  BKNI_Sleep(delay);

    if (eVideoMode == 14)  /*  positive modulation video (only "L" for now)*/
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ACTL, 0x3E002021);  /*.AGC_AGDZ=1, .AGC_AGAZ=1, .AGC_AGRZ=1, .PD_MODE_SEL=1*/
    }
    else  /*negative modulated video*/
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ACTL, 0x3E002023);  /* .AGC_AGDZ=1, .AGC_AGAZ=1, .AGC_AGRZ=1, .PD_MODE_SEL=0*/
    }

    /* ****************** End of AGC Setup code*/
    return BERR_SUCCESS;
}


/******************************************************************************
  BVSB_3548_P_Init_Ifd_FE()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Init_Ifd_FE(
        BVSB_Handle h,
        BVSB_IfdMode videoMode
        )
{
    BERR_Code retCode;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    uint32_t lval1;

    /*!!!!!! NEEDS TO BE FIXED...look for */
    static BRSP_ScriptTbl positive_modulation_video[] =
    {
        /* all values use "back_porch" keying parameters from Steve's script */
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ATHRNVCS, 0x000A015E,  /* .FE_ATHR= 350, .TRK_1_VCOEF modified later*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_CORDICRC, 0x09AB0132,  /* .CORDIC_RC = 306 (= FE_ATHR * 0.875)*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_TMUXC   , 0x00000400,  /* .CNTR_SCLE_EN=0, .CL_LFIN_REGW_SEL=1 (CL= "frz")*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_PDPGAPD , 0x1FFFFF9C,  /* decrease_val = -0.001 * 100000*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_NDPGAPD , 0x00002710,  /* increase_val =  0.1   * 100000*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ACDTHRS ,     0x3666,  /* .LN_CTHR=3, .AD_ABS_THR=0x666 (80% FS)*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ACDCTLTHR, 0x1000202,  /* .CLIP_THR=514 = 5% smpls over (LN_CTHR*3432 pixels)*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_CTL    ,       0x480,  /* .ADCIF_BYP=0, .EDGE_SEL=2, .INVT_OUT=0 (overmod=OFF)*/
        /* The following coefs are used since FE_ACOEF.AUTO_BW_CF_M=0*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ALFCS  ,       0x7D0,  /* 2000*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AVLFCS ,       0x000,  /* 0*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AGCALFCS, 0x000007D0,  /* .LF_AVKL=0x0000, .LF_AKL=0x07D0, */
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ACFSCLE,  0x248E0810,  /* .CCAM_M_EN=1, .SEL_COR_EN_DLY=1, .CORDIC_EN_TAPS=&H48*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AHSYNC  ,   0xF00000,  /* .SECAM_THR = 60*/
        BRSP_ScriptType_eEndOfScript, 0, 0
    };

    static BRSP_ScriptTbl negative_modulation_video[] =
    {
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ATHRNVCS, 0x000A041A,   /* .FE_ATHR set here, .TRK_1_VCOEF modified later*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_CORDICRC, 0x09AB0384,   /* .CORDIC_RC = 900*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_TMUXC   , 0x40000400,   /* .CNTR_SCLE_EN=1, .CL_LFIN_REGW_SEL=1 (CL= "frz")*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_PDPGAPD , 0x1FFFFC18,   /* decrease_val = -0.01 * 100000*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_NDPGAPD ,    0x186A0,   /* increase_val =  1.0  * 100000*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ACDTHRS ,     0x3598,   /* .LN_CTHR=3, .AD_ABS_THR=0x598 (70% FS)*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ACDCTLTHR, 0x1000202,   /* .CLIP_THR=514 = 5% smpls over (LN_CTHR*3432 pixels)*/
        /* replaced by stj on 04/20/05 for 3560 and thompson tuner*/
        /*! these "custom" values will be re-visited for BCM3411 + BPF*/
        /*! BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_PDPGAPD  , 0x1FFFFE0C,*/
        /*! BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_nDPGAPD  , 0x1388,*/
        /*! BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ACDTHRS  , 0x44CC,      // .LN_CTHR, 0x4, .AD_ABS_THR, 0x4CC*/
        /*! BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ACDCTLTHR, 0x10002AE,   // .CLIP_THR, 0x2AE*/

        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_CTL    ,       0x4C0,  /* .ADCIF_BYP=0, .EDGE_SEL=2, .INVT_OUT=1 (overmod=ON)*/
        /*   NOTE: enabling ADC FIFO here (after FIFO reset) is OK because this mux is AFTER the FIFO*/

        /* The following coefs are ignored unless FE_ACOEF.AUTO_BW_CF_M=0 which turns OFF*/
        /* auto AGC LF coeff mode (which uses 4000 if locked, and 1000 if not).  For now, leave enabled.*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ALFCS  ,       0xFA0,  /* 4000*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AVLFCS ,       0xFA0,  /* 4000*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AGCALFCS, 0x0C990FA0,  /* !!!should be 0x0FA00FA0???*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ACFSCLE,  0x00060810,  /* NEW 3560B: FE AGC Auto Update Coefficient Scale*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AHSYNC  ,        0x0,
        BRSP_ScriptType_eEndOfScript, 0, 0
    };

    static BRSP_ScriptTbl ifd_fe[] =
    {
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ADSG   ,         0x0,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AAPGAUT,  0x7FFFFFFF,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AIFUT  ,  0x7FFFFFFF,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ARFUT  ,  0x7FFFFFFF,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AAPGALT,  0x80000000,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AIFLT  ,  0x80000000,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ARFLT  ,  0x80000000,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_LFCS   ,  0x7E000000,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_DSLFCS ,         0x0,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_FCMNX  ,  0x7FFF8000,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_RSTNAFT ,  0x4100000,

        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ADPGAUT , 0xC0000000,  /* 8.0*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ADPGALT , 0x88000000,  /* 1.0*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AIFTHR  ,        0x0,  /*  *NOT* AGC FIX (should always be 0)*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AAPGATHR,        0x0,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_CLIPD   ,        0x0,

        /* Reset FE DC canceller   [TLS 20080505]*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_DCOC    ,      0x482,   /* .FE_DCOC_RST=1*/
        /* BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_DCOC_INT , 0x00000000,   // done by RST*/
        /* BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_DCOC_LPB , 0x00000000,   // done by RST*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_DCOC    ,      0x480,   /* .FE_DCOC_RST=0*/

        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_PGAH    ,  0x4000001,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_TLFIN   ,        0x0,

        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_GPDS_INT ,       0x0,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_GPDS_INTG_SWP,   0x0,

        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AFTFCMNX, 0x7FFF8000,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_AACQLFCS,     0x1388,

        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_CL_INTG_SWP ,     0x0,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_APDCTL ,   0x00000020,  /* .PD_NUM_LINES=1 (or 4 for some VCR's...see note)*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_TRICKP1  ,    0x38D67,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_TRICKP2  ,    0x1041A,

        /* [$$$B0] The following were new registers beginning with the 3560B*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_VARIANCE , 0x230000,     /* NEW 3560B: Register phase detector output variance value */
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_CORDICEN , 0x9C4000,     /* NEW 3560B: !!! Dinker says not used???*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_ACOEFM ,  0x3E80BC4,  /* NEW 3560B: FE AGC Auto Update Coefficient Slope*/

        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_INT ,           0x0,
        BRSP_ScriptType_eEndOfScript, 0, 0
    };

    /* Start of code section */
    lval1= (BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_APGA) & 0x0000E000) | 0x00000BE8;
    if (h->settings.bRfDeltaSigmaOpenDrain)
        lval1 |= 0x00004000; /* [14]=RF_DELSIG_OD_M, maybe define: BCHP_IFD_CORE_FE_APGA_OD_DELSIG_RF_MASK */
	else 
		lval1 &= ~0x00004000;

    if (h->settings.bIfDeltaSigmaOpenDrain)
        lval1 |= 0x00002000; /* [13]=IF_DELSIG_OD_M, maybe define: BCHP_IFD_CORE_FE_APGA_OD_DELSIG_IF_MASK */
	else 
		lval1 &= ~0x00002000;
    BREG_Write32( p3548->hRegister, BCHP_IFD_CORE_FE_APGA, lval1);   /* [TLS 20080729] */

        /* preserve [15:13] AGC PP/OD set by "SYMBOLS.PIN_CTRL3 / 1" in "configure_pin_select" */
        /* .CS_ALP_COEF , 0xB&  // [$$$B0] NEW 3560B: Counter peak scale leak averager alpha coeff */
        /* .CS_UPPER    , 0xE8& // [$$$B0] NEW 3560B: Counter peak scale upper limit */
    lval1= BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_CTL2) & 0x00380000; /* ( | 0x00000000); */
    if (h->settings.bRfDeltaSigmaInvert)
        lval1 |= BCHP_IFD_CORE_FE_CTL2_INVT_DELSIG_RF_MASK;
	else
        lval1 &= ~BCHP_IFD_CORE_FE_CTL2_INVT_DELSIG_RF_MASK;

    if (h->settings.bIfDeltaSigmaInvert)
        lval1 |= BCHP_IFD_CORE_FE_CTL2_INVT_DELSIG_IF_MASK;
	else
        lval1 &= ~BCHP_IFD_CORE_FE_CTL2_INVT_DELSIG_IF_MASK;
    
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_CTL2, lval1);   /* [TLS 20080729] */
        /* preserve [21:19] AGC polarity set by "SYMBOLS.PIN_CTRL5" in "configure_ds_oob_pins"  */

    if (videoMode == BVSB_IfdMode_eL)  /*  positive modulation video (only "L" for now)*/
	{
        /* all values use "back_porch" keying parameters from Steve's script*/
        /**/
	    retCode = BRSP_ParseRegScript( p3548->hRegister, positive_modulation_video, NULL );
        lval1 = 0x00200190;  /* FE_ACOEF value with .AUTO_BW_CF_M= 0*/
    }
	else                 /*  negative modulated video*/
    {
	    retCode = BRSP_ParseRegScript( p3548->hRegister, negative_modulation_video, NULL );
        lval1 = 0x00200191;  /* FE_ACOEF value with .AUTO_BW_CF_M= 1*/
	}

    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ACOEF  , lval1 | 0x02);  /*.hsynclock_rst=1 // NEW 3560B: FE AGC Auto Update Coefficient*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ACOEF  , lval1);        /*.hsynclock_rst=0  */

    retCode = BRSP_ParseRegScript( p3548->hRegister, ifd_fe, NULL );
    return retCode;
}

/******************************************************************************
  BVSB_3548_P_Init_Ifd_Carrier_Limits()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Init_Ifd_Carrier_Limits(
        BVSB_Handle h
        )
{
    uint8_t data0;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    /* allow user to limit carrier pull-in range*/
    /* bit[7]=1 "Allow VIF BW limits",  bits[6:4] are the limits*/
   	/* OLD CODE data0 = ifd_acqcmd3 & 0xf0; */  /* preserve [7:4]*/
    data0 = PULL_IN_RANGE;

    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_CRLPLFTU, 0x00F2B9D6);   /*  50 kHz [TLS 20080513]*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_CRLPLFTL, 0xFF0D462A);   /* -50 kHz [TLS 20080513]*/

#ifdef AGC_DEBUG
	if (data0 == 0x80)   /*  Then  1_000_xxxx  // +/- (  0 + 32 kHz)     */
	{
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFHI , 0x0);     /* =      freq * 32 * 2^31 / 54e6*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFLO , 0x0);     /* =  -1* freq * 32 * 2^31 / 54e6*/

        /* Only use +/-32 kHz back loop when front range is +/-0*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_CRLPLFTU , 0x009B5837);   /*=  32000 / 13.5e6 * 2^32*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_CRLPLFTL , 0xFF64A7C9);   /*= -32000 / 13.5e6 * 2^32    */
    }
    else if (data0 == 0x90)  /* Then  1_001_xxxx // +/- ( 250 + 50 kHz)*/
	{
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFHI , 0x12F684BD);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFLO , 0xED097B43);
    }
    else if (data0 == 0xA0)  /* Then  1_010_xxxx // +/- ( 500 + 50 kHz)*/
	{
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFHI , 0x25ED097B);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFLO , 0xDA12F685);
	}
    else if (data0 == 0xB0)  /* Then  1_011_xxxx // +/- ( 750 + 50 kHz)*/
    {
	    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFHI , 0x38E38E38);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFLO , 0xC71C71C8);
    }
	else if (data0 == 0xC0)  /* Then  1_100_xxxx // +/- (1000 + 50 kHz)*/
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFHI , 0x4BDA12F6);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFLO , 0xB425ED0A);
    }
	else if (data0 == 0xD0)  /* Then  1_101_xxxx // +/- (1250 + 50 kHz)*/
    {
	    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFHI , 0x5ED097B4);      /* [TLS 20080422]*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFLO , 0xA12F684C);
    }
    else        /*  [7]=0 or other RESERVED values   // NORMAL limits*/
    {
#endif
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFHI , 0x536FE1A8);     /* +1100000 * 2^31 * 32 / (54e6)*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFLO , 0xAC901E58);     /* -1100000 * 2^31 * 32 / (54e6)*/
#ifdef AGC_DEBUG
	}
#endif
    return BERR_SUCCESS;

}

/******************************************************************************
  BVSB_3548_P_Ifd_load_coef()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Ifd_load_coef(
        BVSB_Handle h,
        uint16_t *pCoef,
        uint16_t nCoefLen,
        uint16_t nCoefStart
        )
{
    uint32_t ulCoef, ulOffset;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    ulOffset = nCoefStart;

    while( nCoefLen-- )
    {
        ulCoef = (*pCoef) | (ulOffset << 16);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_COEF, ulCoef);
        ulOffset += 8;
        pCoef++;
    }
    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_Init_Ifd_NICAM()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Init_Ifd_NICAM(
        BVSB_Handle h
        )
{

    /* RO status registers*/
    /*   IFD_NI.DEC_STAT, IFD_NI.DEC_ERROR, IFD_NI.AGC_GAIN, IFD_NI.DEMOD_DOUT,*/
    /*   IFD_NI.TIM_LOOP_FRQ, IFD_NI.PH_LOOP_FRQ, IFD_NI.DEMOD_ERRSQ*/

    /* R/W registers for debug or test not initialized here*/
    /*   IFD_NI.INTRPT_STAT, IFD_NI.INTRPT_MASK, IFD_NI.BERT_CNTR*/

    /* R/W registers for NICAM receiver*/
    /*  leave these as defaults?*/
    /*  ?  IFD_NI.DEC_FAW = &H1FF&         ' dflt*/
    /*  ?  IFD_NI.DEC_ERROR_THRES = &HFF&  ' dflt*/
    /*  ?  IFD_NI.AGC_COEF = &H0D000000&*/
    /*  ?  IFD_NI.AGC_LOOP_INT = &H0&    ' dflt*/
    /*  ?  IFD_NI.TIM_LOOP_INT = &H0&    ' dflt*/
    /*  ?  IFD_NI.PH_LOOP_INT = &H0&     ' dflt*/
    /*  ?  IFD_NI.LOOP_FRZ    = &H0&     ' dflt*/
    /*  ?  IFD_NI.ACQ_TIMER = &H00010000&  ' dflt*/
    /*  ?  IFD_NI.LOOP_CLP = &H003FC03C&   ' dflt*/
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    BREG_Write32(p3548->hRegister, BCHP_IFD_NI_DEMOD_CTRL, 0x0002);   /* dflt + .AUTO_ACQ= 1*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_NI_TP_CTRL   , 0x2000);   /* dflt + .OUT_MUX= 4*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_NI_DEC_CTRL  , 0x080B);   /* dflt + .AUTO_CORRECT= 0, .EN= 1*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_NI_TIM_COEF, 0x101D0000); /* dflt (.KIT= &h10&, .KPT= &h1d&)*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_NI_CAR_COEF, 0x051C0000); /* dflt (.KIP = &H5&, .KPP = &H1C&)*/

    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_Init_AP1AP2_and_VarDet()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Init_AP1AP2_and_VarDet(
        BVSB_Handle h,
        BVSB_IfdMode eVideoMode,
        BVSB_AnalogAudioDeviation eAnalogAudioDeviation
        )
{
    uint32_t lval1, regVal;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    /* Negative SIF audio frequencies are used to get the correct spectral inversion*/
    /*  Ex.:  round( -4.5 /54 * 2^29)*/

    /* ****************************************************************    */
    /* ******************** AP1 setup only *****************************/
    /* *****************************************************************/
    /* COMMON REGISTERS FOR AP1*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICCTL , 0xF);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICAGCTL , 0x1);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICAGCT , 0x1000000);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICAGCI , 0x89000000);   /* nom starting point*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICAGCL , 0x01000000);   /* init don't care?    */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICAGCPDBYPVAL , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CRCAGCPDBYPVAL , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CRCAGCTL , 0x1);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CRCAGCT , 0x4000);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CRCAGCI , 0x04000000);   /* nom starting point*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CRCAGCL , 0x00004000);   /* init don't care?    */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_BYP , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_TPCTL , 0x0);
    /*??? BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTORRFM , 0x1FFFF);*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_DCOC , 0x81);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_DCOC_INT , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_DCOC_LPB , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SPARE1 , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SPARE2 , 0x0);

    if (eVideoMode == 14)  /*  AM audio (only "L" for now)*/
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICSHFT , 0x779); /* .SUBTHRESH=0x7, .LKINTGFB=0x7, .SQUARE=0x9*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CRCSHFT , 0x977); /* .SUBTHRESH=0xC, .LKINTGFB=0x7, .SQUARE=0x7*/
    }
    else                 /*  FM audio*/
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICSHFT , 0x377); /* .SUBTHRESH=0x3, .LKINTGFB=0x7, .SQUARE=0x7*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CRCSHFT , 0x877); /* .SUBTHRESH=0x8, .LKINTGFB=0x7, .SQUARE=0x7*/
    }


    /* AP1 audio carrier freq*/
    if      (eVideoMode <= 0x04)    /* M,N  (6 MHz channel)*/
    {
        lval1 = 0x1D555555;  /* -4.5*/
    }
    else if (eVideoMode <= 0x07)    /* B/G/H*/
    {
        lval1 = 0x1CBDA12F;  /* -5.5*/
    }
    else if  (eVideoMode == 0x08)   /* I*/
    {
        lval1 = 0x1C71C71C;  /* -6.0*/
    }
    else           /*  D/K, L*/
    {
        lval1 = 0x1C25ED09;   /* -6.5*/
    }
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_FCW,  lval1);

    if (( eVideoMode == 0 ) || (eVideoMode ==3))
    { /*  M-BTSC,  N */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICDIV,  0x12);
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef6, COEF6_LEN, COEF6_START); /* TBA */
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef8, COEF8_LEN, COEF8_START); /* TBC */

        if (eAnalogAudioDeviation == BVSB_AnalogAudioDeviation_eHigh) /* "AP1_FM_BW"=1  so use wide LPFB filter*/
        {
            BVSB_3548_P_Ifd_load_coef(h, ifd_coef7_BTSC_LPFB_Wide, COEF7_LEN, COEF7_START);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTOR, 0x8000);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SHFT     ,  0x575);
        }
        else if (eAnalogAudioDeviation == BVSB_AnalogAudioDeviation_eMedium)
        {
            BVSB_3548_P_Ifd_load_coef(h, ifd_coef7_BTSC_LPFB_Medium, COEF7_LEN, COEF7_START);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTOR, 0x8000);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SHFT     ,  0x665);
        }
        else
        {
            BVSB_3548_P_Ifd_load_coef(h, ifd_coef7, COEF7_LEN, COEF7_START);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTOR, 0x8000);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SHFT     ,  0x765);
        }

    }
    else if ((eVideoMode == 1) || (eVideoMode == 2) || (eVideoMode==5) || (eVideoMode==10) || (eVideoMode==11) || (eVideoMode==12))
    { /* M-A2, reserved2 (maybe J-BTSC later?), B/G/H-A2, D/K1-A2, D/K2-A2, D/K3-A2 */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICDIV,  0x17);
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef6_alt, COEF6_LEN, COEF6_START);
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef8_alt, COEF8_LEN, COEF8_START);

        if (eAnalogAudioDeviation == BVSB_AnalogAudioDeviation_eHigh) /* "AP1_FM_BW"=1  so use wide LPFB filter*/
        {
            BVSB_3548_P_Ifd_load_coef(h, ifd_coef7_A2_LPFB_Wide, COEF7_LEN, COEF7_START);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTOR, 0x6555);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SHFT     ,  0x575);
        }
        else
        {
            BVSB_3548_P_Ifd_load_coef(h, ifd_coef7_alt, COEF7_LEN, COEF7_START);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTOR, 0x6555);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SHFT     ,  0x765);
        }
    }
    else if ((eVideoMode == 6) || (eVideoMode == 8) || (eVideoMode==9))
    { /* B/G/H-NICAM, I,  D/K-NICAM */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICDIV,  0x17);
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef6_alt, COEF6_LEN, COEF6_START); /* TBA */
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef8_alt, COEF8_LEN, COEF8_START); /* TBC */
        if (eAnalogAudioDeviation == BVSB_AnalogAudioDeviation_eHigh) /* "AP1_FM_BW"=1  so use wide LPFB filter*/
        {
            BVSB_3548_P_Ifd_load_coef(h, ifd_coef7_A2_LPFB_Wide, COEF7_LEN, COEF7_START);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTOR, 0x6555);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SHFT     ,  0x575);
        }
        else
        {
            BVSB_3548_P_Ifd_load_coef(h, ifd_coef7_alt, COEF7_LEN, COEF7_START);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTOR, 0x6555);
            BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SHFT     ,  0x765);
        }

    }
    else if (eVideoMode == 14)
    { /* L */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CICDIV,  0x17);  /***/
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef6_alt, COEF6_LEN, COEF6_START); /* TBA */
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef8_alt, COEF8_LEN, COEF8_START); /* TBC */
        /*!!! init_coef_7_SECAM-L    '30kHz BW Filter For AM Audio*/
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef7_alt, COEF7_LEN, COEF7_START); /* TBC */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTOR, 0x8000);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_SHFT     ,  0x765);  /***/
        /*!!!'AND MORE REGISTER CHANGES FOR AM!!!!!!!!!!!!!!!!!!!!!!!*/

    }


    /* ****************************************************************    */
    /* ******************** AP2 setup only *****************************/
    /* *****************************************************************/
    /* COMMON REGISTERS FOR AP2	*/

    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICCTL , 0xF);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICAGCTL , 0x1);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICAGCT , 0x1000000);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICAGCI , 0x89000000);   /* nom starting point*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICAGCL , 0x01000000);   /* init don't care?*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICAGCPDBYPVAL , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CRCAGCPDBYPVAL , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICSHFT , 0x377);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CRCAGCTL , 0x1);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CRCAGCT , 0x4000);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CRCAGCI , 0x04000000);   /* nom starting point*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CRCAGCL , 0x00004000);   /* init don't care?*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CRCSHFT , 0x877);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_BYP , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_TPCTL , 0x0);
    /*???    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTORRFM , 0x1FFFF);*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_DCOC , 0x81);  /* match AP1 path*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_DCOC_INT , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_DCOC_LPB , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_SPARE1 , 0x0);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_SPARE2 , 0x0);

    BVSB_3548_P_Ifd_load_coef(h, ifd_coef9, COEF9_LEN, COEF9_START); /* _A2 path TBA*/
    BVSB_3548_P_Ifd_load_coef(h, ifd_coef11, COEF11_LEN, COEF11_START);  /* _A2 path TBC*/

    /* AP2 audio carrier freq*/
    if (eVideoMode == 1 )  /* case  1:    // M - A2*/
    {
        lval1 = 0x1D335245;   /* -4.7242*/
    }
    else if (eVideoMode == 5)	  /* case  5:    // B/G/H - A2        */
    {
        lval1 = 0x1C98EAD6;   /* -5.742*/
    }
    else if (eVideoMode == 6)   /* case  6:    // B/G/H - NICAM*/
    {
        lval1 = 0x1C888889;   /* -5.85*/
    }
    else if (eVideoMode == 8)  /*	case  8:    // I*/
    {
        lval1 = 0x1C1E098F;   /* -6.552      */
    }
    else if (eVideoMode == 9)  /* case  9:    // D/K  - NICAM*/
    {
        lval1 = 0x1C888889;   /* -5.85*/
    }
    else if (eVideoMode == 10) /* case  10:   // D/K1 - A2*/
    {
        lval1 = 0x1C4ACA39;   /* -6.257*/
    }
    else if (eVideoMode == 11) /* case  11:   // D/K2 - A2*/
    {
        lval1 = 0x1C0136B0;   /* -6.742*/
    }
    else if (eVideoMode == 12) /* case  12:   // D/K3 - A2*/
    {
        lval1 = 0x1C98EAD6;   /* -5.742*/
    }
    else if (eVideoMode == 14) /* case  14:   // L*/
    {
        lval1 = 0x1C888889;   /* -5.85*/
    }
    else
    {
        /*case  0:    // M - BTSC*/
        /*case  2:    // reserved2*/
        /*case  3:    // N*/
        /*case  4:    // reserved4*/
        /*case  7:    // reserved7*/
        /*case  13:   // reserved13*/
        /*case  15:   // reserved15*/
        /*default:*/
        lval1 = 0x1D555555;  /* -4.5*/
    }
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_FCW,  lval1);

    /* Now configure receiver based on one of the following modes*/
    /*    FM demod (Fs=315) (for A2 systems)*/
    /*    FM demod (Fs= 250)*/
    /*    NICAM demod (50% Nyq)*/
    /*    NICAM demod (100% Nyq)*/
    BVSB_3548_P_Init_Ifd_NICAM(h); /*!!!??? always init even if not used?*/

    if ((eVideoMode == 1) || (eVideoMode == 5) || (eVideoMode==10) || (eVideoMode==11) || (eVideoMode==12))
    {
        /* case  1:    // M - A2*/
        /* case  5:    // B/G/H - A2*/
        /* case  10:   // D/K1 - A2*/
        /* case  11:   // D/K2 - A2*/
        /* case  12:   // D/K3 - A2*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICDIV,      0x17);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTOR, 0x6555);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_SHFT     ,  0x755); /* [TLS 20080729] (was &H765) */
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef10, COEF10_LEN, COEF10_START); /* TBA */
    }
    else if ((eVideoMode == 6) || (eVideoMode == 9) || (eVideoMode==14))
    {
        /* case  6:    // B/G/H - NICAM*/
        /* case  9:    // D/K  - NICAM*/
        /* case  14:   // L*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICDIV,      0x12);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTOR, 0x6000);  /* Needs to be finalized*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_SHFT     ,  0x765);  /* Needs to be finalized*/
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef10_NICAM_Nyq_Type0, COEF10_LEN, COEF10_START); /* TBA */

        regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_TOP_IF_CNTL);
        regVal |= BCHP_IFD_TOP_IF_CNTL_NI_ENABLE_MASK;
        BREG_Write32(p3548->hRegister, BCHP_IFD_TOP_IF_CNTL, regVal);
    }
    else if (eVideoMode == 8)
    {
        /* case  8:    // I*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICDIV,      0x12);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTOR, 0x6000);  /* Needs to be finalized*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_SHFT     ,  0x765);  /* Needs to be finalized*/
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef10_NICAM_Nyq_Type1, COEF10_LEN, COEF10_START); /* TBA */

        regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_TOP_IF_CNTL);
        regVal |= BCHP_IFD_TOP_IF_CNTL_NI_ENABLE_MASK;
        BREG_Write32(p3548->hRegister, BCHP_IFD_TOP_IF_CNTL, regVal);
    }
    else
    {
        /*case  0:    // M - BTSC*/
        /*case  2:    // reserved2*/
        /*case  3:    // N*/
        /*case  4:    // reserved4*/
        /*case  7:    // reserved7*/
        /*case  13:   // reserved13*/
        /*case  15:   // reserved15*/
        /*default:      // default to unused "M" mode*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICDIV,      0x12);
        /* BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTOR, 0x6000);*/
        /* BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_SHFT     ,  0x765);*/
    }



    /*!!! KEEP THIS JUNK BELOW?*/

    /* [TLS 20060113] I made the following empirical observations the Samsung DLP TV Ch 3:*/
    /*   -- AP_CDET_VAR didn//t vary by more than 1 or 2 dB (assuming linear scale) as*/
    /*      I swept in 2 dimensions: fdev= 0 to 100kHz, fm= 10Hz to 10kHz using Marconi*/
    /*   -- for Fv= -5 dBmV, I reduced Fa until TV muted.  It was about A/V= -44dBc*/
    /*      with about 1 dB of hysteresis.  AP_CDET_VAR, 0x02B00000 (MUTED), &H02700000 (ON)*/
    /*   -- Fa reduced another 2 dB:  AP_CDET_VAR, 0x03400000...use this as our mute thresh*/
    /*   -- For now put "MONO" thresh same as Samsung mute: AP_CDET_VAR, 0x02B00000);*/

    /* Variance Detect for AP1*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CDET_CNTL , 0x37FF88);
    /* .CIC_COMP= 55,  .COEF4= 15,  .COEF3= 15, .COEF2= 8,  .COEF1= 8*/
    /* Registers AP_CDET_Mxxx now configured in Set_IFD_APx_CDET_Mono_Mute_Thresholds()*/
    /*   BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CDET_MUTE , 0x03400000);   // [TLS 20060113]    */
    /*   BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_CDET_MONO , 0x02ED0000);   // SET TO (0.9 * AP_CDET_MUTE)   [TLS 20060120]*/

    /* Variance Detect for AP2    */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CDET_CNTL , 0x37FF88);
    /* .CIC_COMP= 55,  .COEF4= 15,  .COEF3= 15, .COEF2= 8,  .COEF1= 8*/
    /* Registers AP_CDET_Mxxx now configured in Set_IFD_APx_CDET_Mono_Mute_Thresholds()*/
    /*   BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CDET_MUTE , 0x02000000);    // [TLS 20060118] was &H03400000&*/
    /*   BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CDET_MONO , 0x01CD0000);    // SET TO (0.9 * AP_CDET_MUTE)   [TLS 20060120]*/


    /* ... !!! A lot deleted here !!!*/

    /* Perform soft reset on IFD Audio which improves A2 stereo separation  [TLS 20060516]*/
    /* (per Dinker and Hung//s advice...reset AFTER filters programmed)*/
    /*not bit 8???  BCHP_IFD_CORE.RESET , 0x0102);    // [8]= .EXT_DP_B=1,   [1]= .AP=1*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_RESET , 0x00000002);    /* BCHP_IFD_CORE.RESET.AP = 1*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_RESET , 0x00000000);    /* BCHP_IFD_CORE.RESET.AP = 0*/
    return BERR_SUCCESS;

}

/******************************************************************************
  BVSB_3548_P_Init_SIF_Source()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Init_SIF_Source(
        BVSB_Handle h,
        BVSB_IfdMode eVideoMode,
        BVSB_AnalogInput eAnalogInput
        )
{
     /* VAFE MSB flipped in IFD_TOP.IF_VAFE_CNTL.CH1_MSFLIP, so always use .AP_A2CNTL.AFE_MSFLIP = 0*/

     /* NOTES on using .AP_A2CNTL.SHFT for SIF modes per Dinker email & conversation:*/
     /*  -- 10-bit VAFE & 12-bit EDSAFE directly to ap_mixer_input are both MSB aligned*/
     /*  -- .AP_A2CNTL.SHFT =5 shouldn't limit performance since 19-bits are retained into ap_mixer_input*/
     /*  -- Empirical lab results show that performance doesn't degrade until SHFT >= 0xA.*/
     /*     (performance included both the AP_CDET_VAR and output audio THD+N for 1kHz @ 25 kHz dev)*/

    uint32_t lval1;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    if (eVideoMode == 14)  /*  AM audio (only "L" for now)*/
	{
       lval1 = 0x00000001;  /*  .AP_A2CNTL.TBC_IN_SEL = 1*/
    }
	else                 /*  FM audio*/
    {
       lval1 = 0x00000000;  /*  .AP_A2CNTL.TBC_IN_SEL = 0    //IFD_CORE.AP_A2CNTL.TBC_IN_SEL = 0*/
    }

    if (eAnalogInput == BVSB_AnalogInput_eIf) /* NORMAL IFD Operation */
	{
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_MISCTL    , 0x8);  /* .LF_EN = 1   // normal operation*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_MISCTL , 0x8);  /* .LF_EN = 1   // normal operation*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_A2CNTL    , 0x400 | lval1);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_A2CNTL , 0x400);
        /*   x.AFE_MSFLIP = 0   // valid for both EDSAFE or VAFE (flipped elsewhere)       */
        /*   x.MIXER_SEL = 0    //Real and Imaginary from IFD Frontend (Default)*/
        /*   x.SHFT_BYP = 1     // dflt=1 (no extra shift)*/
        /*   x.SHFT     = 0     // (dflt=0) for normal IFD*/
    }
    else
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_MISCTL    , 0x0);  /* .LF_EN = 0   turn OFF LF out from VP (no need for audio buzz)*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_MISCTL , 0x0);  /* .LF_EN = 0   turn OFF LF out from VP (no need for audio buzz)*/

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_A2CNTL    , 0x160 | lval1);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_A2CNTL , 0x160);
        /*   x.AFE_MSFLIP = 0   // valid for both EDSAFE or VAFE (flipped elsewhere)*/
        /*   x.MIXER_SEL = 2    // Real From VAFE Channel 1, Imaginary set to zero*/
        /*   x.SHFT_BYP = 0     // needed for shift register to work (dflt=1)*/
        /*   x.SHFT     = 5     // >>5 so SIF has similar gain into ap_mixer_input [TLS 20060327]*/
    }
    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_Init_Ifd_VP()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Init_Ifd_VP(
        BVSB_Handle h,
        BVSB_IfdMode videoMode
        )
{
    /* Here just put changes specific to positive or negative modulation */
	/* (useful if pos/neg is ever broken out as a new enable bit) */

    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    static BRSP_ScriptTbl ifdVP[] =
    {
        /* BCHP_IFD_CORE.VP_SHFT : set based on ATF & GDF filters*/
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_BYP,              0x4,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_MISCTL,           0x2,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_OUTOFS ,       0x2F00,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CRLPPDCTL, 0xC0709298,

        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CRLPPDBYP ,        0x0,

        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CRLPLFCOEF, 0x70000100,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CRLPLFINT ,        0x0,

        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CRLPNCOFCW,        0x0,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CRLPNCO ,    0x7C098BB,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLAGMISC ,     0x35F00,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLHSYNC ,     0x800130,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGHSYNC ,      0x80130,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLREFLEV ,       0x2F0,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGREFLEV ,        0xDC,

        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLPDBYP ,          0x0,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGPDBYP ,          0x0,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLLFCTL ,          0x0,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGLFCTL ,          0x0,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLLFCOEF ,         0x1,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGLFCOEF ,       0x100,

        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLINTCTL ,         0x2,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGINTCTL ,         0x2,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLINTTU ,    0x4580000,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGINTTU ,   0x86720000,
        /*BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLINTTL ,   0xECAB999A,  -89 IRE*/
        /*BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLINTTL ,   0xFEEA0000,   -5 IRE*/
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLINTTL ,   0xF9800000,  /* -29.9 IRE*/
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGINTTL ,   0x84000000,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CLINT ,            0x0,
        /*BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGINT , 0x86720000,*/
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_AGINT , 0x86280000, /* *** Tom Spieker Change 8/12/05*/
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_VSYNCDELAY ,       0x0,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_VSYNCWIDTH ,    0x8610,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CDAGCINC ,         0x1,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CDAGCDEC ,  0xFFFFFFFF,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CDAGCABS ,      0x2710,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CDAGCCLIP ,       0x22,
        BRSP_ScriptType_eWrite32, BCHP_IFD_CORE_VP_CDAGCLINE ,        0x1,
        BRSP_ScriptType_eEndOfScript, 0x00,			     0x00000000
    };

    /* Function created to organize VP_x reg writes*/


    /*!!! first here just put changes specific to positive or negative modulation*/
	/*!!! (useful if pos/neg is ever broken out as a new enable bit)*/

    if (videoMode == 14)  /*  positive modulation video (only "L" for now)*/
	{
        /* inverted LPF filter for positive modulation*/
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef1_AllPass, COEF1_LEN, COEF1_START);

        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_INOFS,       0xFED7);  /* -297 & 0xffff (backporch keying offset)*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_TPCTL,     0x041000);  /* .OVERM_F_EN=0 (overmod=OFF)*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_CRLPLFCTL,   0x1790);  /* .EXP_INT=1, .EXP_LIN=0, .ZERO_IN=1, .DIN_INVERT=0 */
    }
	else                 /*  negative modulated video*/
    {
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef1_AllPass, COEF1_LEN, COEF1_START);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_INOFS,       0x02B2);  /* 690  (normal offset)*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_TPCTL,     0x441000);  /* .OVERM_F_EN=1 (overmod=ON)*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_CRLPLFCTL,   0x22798); /* .EXP_INT=2, .EXP_LIN=1, .ZERO_IN=1, .DIN_INVERT=1 */
    }

    BVSB_3548_P_Ifd_load_coef(h, ifd_coef2, COEF2_LEN, COEF2_START);

    /* [$$$3548A0/3548A0] New table values*/
    if      (videoMode <= 0x04)    /* "M","N"  (6 MHz channel)*/
    {
        /* ap->init_coef_3_GDF_Byp_NTSC() * .GDLY , 0x7 +1 */
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef3, COEF3_LEN, COEF3_START);
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef4, COEF4_LEN, COEF4_START);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_SHFT, 0x05A86C23);   /* .TRAP= 0xa, .GDLY= 0x7 +1*/
    }
    else if (videoMode <= 0x07)    /*  "B/G/H"    Is "H" GDF bypass???*/
    {
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef3_GDF_PAL_B_CurveA, COEF3_LEN, COEF3_START);
        /* init_coef_3_GDF_PAL_B_CurveB();  ???which one!*/
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef4_ATF_PAL_D, COEF4_LEN, COEF4_START);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_SHFT, 0x05B86C23);   /* .TRAP= 0xa +1, .GDLY= 0x7 +1 */
    }
	else if  (videoMode == 0x08)   /* "I"   ... needs different ATF???*/
    {
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef3_GDF_Byp_PAL_D, COEF3_LEN, COEF3_START);
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef4_ATF_PAL_D, COEF4_LEN, COEF4_START);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_SHFT, 0x05B86C23);   /* .TRAP= 0xa +1, .GDLY= 0x7 +1 */
    }
	else if  (videoMode <= 0x0D)   /* "D/K"*/
    {
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef3_GDF_Byp_PAL_D, COEF3_LEN, COEF3_START);
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef4_ATF_PAL_D, COEF4_LEN, COEF4_START);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_SHFT, 0x05B86C23);   /* .TRAP= 0xa +1, .GDLY= 0x7 +1 */
    }
	else            /*  "L"    .... needs different GDF???!!!*/
    {
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef3_GDF_Byp_PAL_D, COEF3_LEN, COEF3_START);
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef4_ATF_PAL_D, COEF4_LEN, COEF4_START);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_SHFT, 0x05B86C23);   /* .TRAP= 0xa +1, .GDLY= 0x7 +1 */
    }
	BRSP_ParseRegScript( p3548->hRegister, ifdVP, NULL );

    /* Change desired VDEC clamp level */
    if (videoMode >= 0x03)    /* all but "M" */
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_CLREFLEV , 0x02FC);
    }
    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_Ifd_set_if_freq()
 ******************************************************************************/
BERR_Code BVSB_3548_P_Ifd_set_if_freq(
        BVSB_Handle h
        )
{
    uint32_t if_freq, ValueHi, ValueLo;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    if_freq = IF_FREQ;
    BMTH_HILO_32TO64_Mul( 0x1000000, if_freq, &ValueHi, &ValueLo); /* make it 40.24 format */
    BMTH_HILO_64TO64_Div32(ValueHi, ValueLo, 54000000, &ValueHi, &ValueLo);
    BMTH_HILO_32TO64_Mul( 65536, ValueLo, &ValueHi, &ValueLo); /* make it 32.24 format */
    BMTH_HILO_64TO64_Div32(ValueHi, ValueLo, 0x1000000, &ValueHi, &ValueLo);
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_FCW, ValueLo );
    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_IfdTop()
 ******************************************************************************/
BERR_Code BVSB_3548_P_IfdTop(
      BVSB_Handle h,                      /* [in] BVSB handle */
      const BVSB_IfdParams* pParam,
      BVSB_IfdMode videoMode,
      BVSB_AnalogAudioDeviation eAnalogAudioDeviation
      )
{
    BERR_Code retCode;
    uint32_t regVal;
    uint8_t pgaGain;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    static BRSP_ScriptTbl ifdTop[] =
    {
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_BISTCNTL,        0x0,  /* .BISTMODE, .BISTMODE_EXT are dflts*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_COEF,            0x0,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_TESTCNTL,       0x10,
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_SPARE,    0x10030300,  /* Revision control*/

        BRSP_ScriptType_eWrite32,BCHP_IFD_TOP_IF_VDAC_CNTL, 0x00030000,
        BRSP_ScriptType_eWrite32,BCHP_IFD_TOP_TP_CNTL,          0x0,  /*[$$$3548A0] was IFD_TOP.TPCTL*/
        /* BRSP_ScriptType_eWrite32,IFD_TOP_SPARE1,        0x0,   Reserve for temp new Host I/F cmds*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_TOP_SPARE2,           0x0,
        BRSP_ScriptType_eWrite32,BCHP_IFD_TOP_IRQMSK,    0xffffffff,
        BRSP_ScriptType_eWrite32,BCHP_IFD_TOP_IRQSET,           0x0,
        BRSP_ScriptType_eWrite32,BCHP_IFD_TOP_IF_VAFE_CNTL, 0x0000AC2B,
        /* dflt (.CHx_INCLK_SEL= 1) + .CHx_MSFLIP= 1, .CHx_VAFE_SEL= 0     was A82A*/

        /* [$$$3548A0] new regs...for now just write to defaults*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_RCCTL,            0x0,  /* [$$$3548A0]*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_RCCLIPC,          0x0,  /* [$$$3548A0]*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_FE_RCCLIPTHR, 0x00000001,  /* [$$$3548A0]*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_VP_INTR,             0x0,  /* [$$$3548A0]*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_VP_INTR_MASK, 0x000001ff,  /* [$$$3548A0]*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_RESET,    0x00000068, /* [6]=.FE_DS, [5]=.EXT_IFC, [3]=.FE_AFE (DelSig & AFE FIFO's)*/
        BRSP_ScriptType_eWrite32,BCHP_IFD_CORE_RESET,    0x00000000,  /* clear RESET*/

        BRSP_ScriptType_eEndOfScript, 0x00,			     0x00000000
    };

    /* Setup PGA Gain */
    if (pParam->eAnalogInput == BVSB_AnalogInput_eIf) /* NORMAL IFD Operation */
    {
        /* grab PGA gain from AP space */
        BVSB_ReadConfig(h, BVSB_CONFIG_NTSC_TER_SETUP1, &pgaGain, 1);
        pgaGain &= 0x7;
		BDBG_MSG(("Analog PGA Gain = %d", pgaGain));

        /* The register is in DS space so we have to write it before we power down. Power
         * down has no effect on the PGA gain */
        regVal = BREG_Read32(p3548->hRegister, BCHP_DS_AFE_CTRL1);
        regVal &= ~BCHP_DS_AFE_CTRL1_PGACNTRL_MASK;
        regVal |= (pgaGain << BCHP_DS_AFE_CTRL1_PGACNTRL_SHIFT);
        BREG_Write32(p3548->hRegister, BCHP_DS_AFE_CTRL1, regVal);

        BREG_Write32(p3548->hRegister, BCHP_IFD_TOP_IF_CNTL,  0x000C3024);  /*[$$$3548A0] dflt &H24& + */
            /* .IFD_DELSIG_SEL=1 [12]  + .DELSIG_REG_SEL=1 [13] */
            /* .HSYNC_SEL = 1  '[19]  this replaces the ROLLO_CTRL bit??? */
            /* .LOCK_SEL  = 1  '[18]  1 -> IFD SS Lock drives PD mode. (see note in SS) */

#ifdef BVSB_POWER_MANAGEMENT
        /* Power off digital path only if we are not if SIF mode */
        BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_0,  0,
                BCHP_FIELD_DATA( CLKGEN_PWRDN_CTRL_0, PWRDN_CLOCK_216_CG_DS, 1 ));

        BREG_AtomicUpdate32(p3548->hRegister, BCHP_CLKGEN_PWRDN_CTRL_1, 0,
                BCHP_FIELD_DATA( CLKGEN_PWRDN_CTRL_1, PWRDN_CLOCK_108_CG_DS, 1 ));
#endif

    }
    else /* SIF MODE ( retain THD core settings!) */
    {
        if (pParam->ifdAgcControl == false)
        {
            BREG_Write32(p3548->hRegister, BCHP_IFD_TOP_IF_CNTL,  0x000C0024);  /*[$$$3548A0] dflt &H24& + */
        }
        else
        {
            BREG_Write32(p3548->hRegister, BCHP_IFD_TOP_IF_CNTL,  0x000C3024);  /*[$$$3548A0] dflt &H24& + */
            /* .IFD_DELSIG_SEL=1 [12]  + .DELSIG_REG_SEL=1 [13] */
            /* .HSYNC_SEL = 1  '[19]  this replaces the ROLLO_CTRL bit??? */
            /* .LOCK_SEL  = 1  '[18]  1 -> IFD SS Lock drives PD mode. (see note in SS) */
        }
    }

    /* Run script */
	retCode = BRSP_ParseRegScript( p3548->hRegister, ifdTop, NULL );

    BVSB_3548_P_Ifd_set_if_freq(h);          /*  dflt value= 45.75 MHz (use SYMBOLS.pix_carrier in BBS)*/

    BVSB_3548_P_Init_Ifd_Carrier_Limits(h);
    BVSB_3548_P_Init_Ifd_FE(h, videoMode);
    BVSB_3548_P_Init_Ifd_VP(h, videoMode);
    BVSB_3548_P_Init_SIF_Source(h, videoMode, pParam->eAnalogInput);
    BVSB_3548_P_Init_AP1AP2_and_VarDet(h, videoMode, eAnalogAudioDeviation);
    BVSB_3548_P_Ifd_Enable_New_SyncStrip(h, videoMode);
    BVSB_3548_P_Init_IFD_AGC(h, videoMode);

    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_VP_SHFT);
    regVal &= 0xFFFFF0FF;
    regVal |= 0x00000800;     /* .LPF = 8 */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_SHFT, regVal);

    if (videoMode == 14)  /*  positive modulation video (only "L" for now)*/
	{
        /* inverted LPF filter for positive modulation*/
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef1_LPF_L_Pos_Mod, COEF1_LEN, COEF1_START);
    }
	else                 /*  negative modulated video*/
    {
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef1, COEF1_LEN, COEF1_START);
    }

    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_TMUXC);
    regVal &= 0xFFFFFBFF; /* .CL_LFIN_REGW_SEL = 0 (FE carr loop= "unfrz") */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_TMUXC, regVal); 

    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_VP_CRLPLFCTL);
    regVal &= 0xFFFFFFEF; /* .ZERO_IN = 0 (Back loop = "unfrz") */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_CRLPLFCTL, regVal); 

    if (pParam->eAcquisitionMode == BVSB_IfdAcquisitionMode_eSlow)
    {
        /* Following lines added to sweep carrier up starting with most negative integrator value */
        /* -> if false lock is a customer problem, decrease acq range and increase AFT steps */
        /* originally was: coef=0x7C000000, sweep=0x0800, dwell 100 ms */
        /* now is        : coef=0x78000000, sweep=0x1000, dwell  50 ms   ' 5/27/2009 */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_INT, 0xAC901E58); /* should be same as LFLO limit */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFCS, 0x78000000);  /* 0x7800 = 4x wider coefficient */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_CL_INTG_SWP, 0x00001000); /* 4096 * 10.610 Hz/ms = 43.5 kHz/ms */
        BKNI_Sleep(50);  /* based on sweep over 2 MHz range */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_CL_INTG_SWP, 0x0); /* turn off sweep */
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_LFCS, 0x7E000000); /* restore back to "normal" coefficients */
    }

    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_CTL2);
    regVal &= 0xFE3FFFFF; /* .SAT_LIN_PROD = 0 */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_CTL2, regVal);

    if (videoMode == 14)  /*  positive modulation video (only "L" for now)*/
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_VARDEL, 0x7C809300);  /* .CORDIC_SCALE=  76 (0.3 * 256), .CORDIC_COMB_EN = 1  ("Combo")*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ACTL, 0x0A002021);  /* .AGC_AGDZ=0, .AGC_AGAZ=0, .AGC_AGRZ=0, .PD_MODE_SEL=0*/
        
    }
    else  /*negative modulated video*/
    {
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_VARDEL, 0x7C80B800);  /*.CORDIC_SCALE= 224, .CORDIC_COMB_EN = 1  ("Combo")*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_FE_ACTL, 0x0A002023);  /* .AGC_AGDZ=0, .AGC_AGAZ=0, .AGC_AGRZ=0, .PD_MODE_SEL=1*/
    }

    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_DCOC, 0x80);  /* .DCOC_RST=0*/
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_DCOC, 0x80);  /* .DCOC_RST=0*/
	BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_CLINTCTL, 0x0); /*' Commands in IFD_VP_CLAMP_Unfreeze()*/

    /*Init_AudioCarrier_Weak_Sig_Det;*/
    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTOR);
	BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_OUTFACTORRFM, regVal);

    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTOR);
	BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTORRFM, regVal);

    /* Clear NICAM error and resetting */
	BREG_Write32(p3548->hRegister, BCHP_IFD_NI_DEC_CTRL, 0x080B | 0x0080);  /* NICAM Error Register Clear */
	BREG_Write32(p3548->hRegister, BCHP_IFD_NI_DEC_CTRL, 0x080B);
	BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_RESET, 0x200);
	BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_RESET, 0x000);
	BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_VP_SPARE1, 0x5); /*This is for AGC stuck algorithm initial value */

    BTMR_StartTimer(p3548->hTimer, 16000);

    return retCode;
}

/******************************************************************************
  BVSB_3548_P_GetIfdStatus()
 ******************************************************************************/
BERR_Code BVSB_3548_P_GetNtscStatus(
	BVSB_Handle h,							/* [in] BVSB handle */
	BVSB_IfdStatus *pIfdStatus              /* [out] pointer to IFD Status structure */
	)
{
	uint32_t regVal, regVal2;
    uint32_t ValueHi, ValueLo;
    int32_t val, freq_error, freq_error2;
    bool signSwitched = false;

    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    if (pIfdStatus == NULL)
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    /* Determine lock status */
    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_SS_SS_STATUS);
    pIfdStatus->bHsyncLocked = (regVal & BCHP_IFD_SS_SS_STATUS_LOCK_DETECT_MASK) ? true : false;

    /* Freq Error */
    val = (int32_t)BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_INT);
    if ( val < 0 )
    {
        signSwitched = true;
        val = 0 - val;
    }

    BMTH_HILO_32TO64_Mul( val, 0x19BFCC, &ValueHi, &ValueLo);
    BMTH_HILO_64TO64_Div32(ValueHi, ValueLo, 0x80000000, &ValueHi, &ValueLo);
    freq_error = (signSwitched) ? (0-ValueLo) : ValueLo;

    /* error 2*/
    signSwitched = false;

    val = (int32_t)BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_VP_CRLPLFINT);
    if ( val < 0 )
    {
        signSwitched = true;
        val = 0 - val;
    }
    BMTH_HILO_32TO64_Mul( val, 0xCDFE60, &ValueHi, &ValueLo); /* mult by 13500000 */
    BMTH_HILO_64TO64_Div32(ValueHi, ValueLo, 0x80000000, &ValueHi, &ValueLo);
    ValueLo /= 2;
    freq_error2 = (signSwitched) ? (0-ValueLo) : ValueLo;
    pIfdStatus->pixCarrFreqError = freq_error + freq_error2;
    pIfdStatus->acqParams.ifFreqOffset = 0; /* Not used for now*/
    pIfdStatus->pixCarrFreq = 39800000;

    /* RF/IF AGC */
    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_AINR);
    regVal >>= 16;
    pIfdStatus->rf_agc = 1000 * ((regVal + 0x8000) & 0xFFFF) / 65535;
    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_FE_AINAPGA);
    regVal >>= 16;
    pIfdStatus->if_agc = 1000 * ((regVal + 0x8000) & 0xFFFF) / 65535;

	/* Get NICAM status */
    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_NI_DEC_STAT);
    if (regVal & BCHP_IFD_NI_DEC_STAT_FAW_LOCK_MASK)
        pIfdStatus->bNicamLock = true;
    else
        pIfdStatus->bNicamLock = false;

    pIfdStatus->eNicamEncoderMode = BCHP_GET_FIELD_DATA(regVal, IFD_NI_DEC_STAT, CONTROL_BITS);

    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_NI_DEC_ERROR);

    pIfdStatus->ulNicamParityError = ((regVal & BCHP_IFD_NI_DEC_ERROR_PAR_ERR_MASK) >>
                                        BCHP_IFD_NI_DEC_ERROR_PAR_ERR_SHIFT);
    pIfdStatus->ulNicamErrorCount = ((regVal & BCHP_IFD_NI_DEC_ERROR_CUR_CNT_MASK) >>
                                        BCHP_IFD_NI_DEC_ERROR_CUR_CNT_SHIFT);

    pIfdStatus->ulNicamSnr =BREG_Read32(p3548->hRegister, BCHP_IFD_NI_DEMOD_ERRSQ);

    /* Get AV ratio 1 */
    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_AP_CICAGCI);
    regVal >>= 14;
    regVal ^= 0x20000;
    regVal &= 0x3FFFF;
    regVal2 = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_AP_CRCAGCI);
    regVal2 >>= 9;
    regVal2 ^= 0x20000;
    regVal2 &= 0x3FFFF;

    BMTH_HILO_32TO64_Mul(regVal, regVal2, &ValueHi, &ValueLo);
    pIfdStatus->ulAVRatio1Hi = ValueHi;
    pIfdStatus->ulAVRatio1Lo = ValueLo;

    /* Get AV ratio 1 */
    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICAGCI);
    regVal >>= 14;
    regVal ^= 0x20000;
    regVal &= 0x3FFFF;
    regVal2 = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CRCAGCI);
    regVal2 >>= 9;
    regVal2 ^= 0x20000;
    regVal2 &= 0x3FFFF;

    BMTH_HILO_32TO64_Mul(regVal, regVal2, &ValueHi, &ValueLo);
    pIfdStatus->ulAVRatio2Hi = ValueHi;
    pIfdStatus->ulAVRatio2Lo = ValueLo;

    /* Calculate Audio Carrier 2 Variance */
    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CDET_VAR);
    regVal >>= 24;
    pIfdStatus->ulAudioCarrier2Variance = regVal;

    return BERR_SUCCESS;
}

/******************************************************************************
  BVSB_3548_P_ResetIfdStatus()
 ******************************************************************************/
BERR_Code BVSB_3548_P_ResetIfdStatus(
	BVSB_Handle h							/* [in] BVSB handle */
	)
{
    uint32_t reg;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);

    reg = BREG_Read32(p3548->hRegister, BCHP_IFD_NI_DEC_CTRL);
    reg |= BCHP_FIELD_DATA(IFD_NI_DEC_CTRL, CLR_ERR, 1);
    BREG_Write32(p3548->hRegister, BCHP_IFD_NI_DEC_CTRL, reg);
    BKNI_Delay(100);
    reg &= ~BCHP_MASK(IFD_NI_DEC_CTRL, CLR_ERR);
    BREG_Write32(p3548->hRegister, BCHP_IFD_NI_DEC_CTRL, reg);

    return BERR_SUCCESS;
}

/******************************************************************************
 BVSB_GetIfdAudioSettings()
******************************************************************************/
BERR_Code BVSB_3548_P_GetIfdAudioSettings(
    BVSB_Handle h,                          /* [in] BVSB handle */
    BVSB_IfdAudioSettings* audioSettings    /* [out] Audio settings structure */
)
{
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    BKNI_Memcpy((void*)audioSettings, (void*)&(p3548->audioSettings), sizeof(BVSB_IfdAudioSettings));
    return BERR_SUCCESS;
}

/******************************************************************************
 BVSB_SetIfdAudioSettings()
******************************************************************************/
BERR_Code BVSB_3548_P_SetIfdAudioSettings(
    BVSB_Handle h,                         /* [in] BVSB handle */
    const BVSB_IfdAudioSettings* audioSettings   /* [in] Audio settings structure */
)
{
    uint32_t regVal;
    BVSB_3548_P_Handle *p3548 = (BVSB_3548_P_Handle *)(h->pImpl);
    BVSB_IfdMode eIfdMode;

    /* Save current settings */
    BKNI_Memcpy((void*)&(p3548->audioSettings), (void*)audioSettings, sizeof(BVSB_IfdAudioSettings));

    BVSB_3548_P_GetIfdMode(audioSettings->eVideoMode, audioSettings->eAudioMode, &eIfdMode);

    /* Lookup carrier 1 FCW value in a table? */
    if (eIfdMode <= 0x04)    /* M,N  (6 MHz channel)*/
    {
        regVal = 0x1D555555;  /* -4.5*/
    }
    else if (eIfdMode <= 0x07)    /* B/G/H*/
    {
        regVal = 0x1CBDA12F;  /* -5.5*/
    }
    else if  (eIfdMode == 0x08)   /* I*/
    {
        regVal = 0x1C71C71C;  /* -6.0*/
    }
    else           /*  D/K, L*/
    {
        regVal = 0x1C25ED09;   /* -6.5*/
    }
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_AP_FCW, regVal);

    /* AP2 audio carrier freq*/
    if (eIfdMode == 1 )  /* case  1:    // M - A2*/
    {
        regVal = 0x1D335245;   /* -4.7242*/
    }
    else if (eIfdMode == 5)	  /* case  5:    // B/G/H - A2        */
    {
        regVal = 0x1C98EAD6;   /* -5.742*/
    }
    else if (eIfdMode == 6)   /* case  6:    // B/G/H - NICAM*/
    {
        regVal = 0x1C888889;   /* -5.85*/
    }
    else if (eIfdMode == 8)  /*	case  8:    // I*/
    {
        regVal = 0x1C1E098F;   /* -6.552      */
    }
    else if (eIfdMode == 9)  /* case  9:    // D/K  - NICAM*/
    {
        regVal = 0x1C888889;   /* -5.85*/
    }
    else if (eIfdMode == 10) /* case  10:   // D/K1 - A2*/
    {
        regVal = 0x1C4ACA39;   /* -6.257*/
    }
    else if (eIfdMode == 11) /* case  11:   // D/K2 - A2*/
    {
        regVal = 0x1C0136B0;   /* -6.742*/
    }
    else if (eIfdMode == 12) /* case  12:   // D/K3 - A2*/
    {
        regVal = 0x1C98EAD6;   /* -5.742*/
    }
    else if (eIfdMode == 14) /* case  14:   // L*/
    {
        regVal = 0x1C888889;   /* -5.85*/
    }
    else
    {
        /*case  0:    // M - BTSC*/
        /*case  2:    // reserved2*/
        /*case  3:    // N*/
        /*case  4:    // reserved4*/
        /*case  7:    // reserved7*/
        /*case  13:   // reserved13*/
        /*case  15:   // reserved15*/
        /*default:*/
        regVal = 0x1D555555;  /* -4.5*/
    }
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_FCW, regVal);

    /* Program this value directly */
    BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICDIV, audioSettings->ulCarrier2DecimationRatio);

    regVal = BREG_Read32(p3548->hRegister, BCHP_IFD_TOP_IF_CNTL);
    regVal &= ~BCHP_IFD_TOP_IF_CNTL_NI_ENABLE_MASK;

    /* Program filter accordingly */
    if ((eIfdMode == 1) || (eIfdMode == 5) || (eIfdMode==10) || (eIfdMode==11) || (eIfdMode==12))
    {
        /* case  1:    // M - A2*/
        /* case  5:    // B/G/H - A2*/
        /* case  10:   // D/K1 - A2*/
        /* case  11:   // D/K2 - A2*/
        /* case  12:   // D/K3 - A2*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICDIV,      0x17);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTOR, 0x6555);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_SHFT     ,  0x765);
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef10, COEF10_LEN, COEF10_START); /* TBA */
    }
    else if ((eIfdMode == 6) || (eIfdMode == 9) || (eIfdMode==14))
    {
        /* case  6:    // B/G/H - NICAM*/
        /* case  9:    // D/K  - NICAM*/
        /* case  14:   // L*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICDIV,      0x12);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTOR, 0x6000);  /* Needs to be finalized*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_SHFT     ,  0x765);  /* Needs to be finalized*/
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef10_NICAM_Nyq_Type0, COEF10_LEN, COEF10_START); /* TBA */

        regVal |= BCHP_IFD_TOP_IF_CNTL_NI_ENABLE_MASK;
    }
    else if (eIfdMode == 8)
    {
        /* case  8:    // I*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_CICDIV,      0x12);
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_OUTFACTOR, 0x6000);  /* Needs to be finalized*/
        BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_A2_AP_SHFT     ,  0x765);  /* Needs to be finalized*/
        BVSB_3548_P_Ifd_load_coef(h, ifd_coef10_NICAM_Nyq_Type1, COEF10_LEN, COEF10_START); /* TBA */

        regVal |= BCHP_IFD_TOP_IF_CNTL_NI_ENABLE_MASK;
    }
    BREG_Write32(p3548->hRegister, BCHP_IFD_TOP_IF_CNTL, regVal);

    /* Reset NICAM */
    BREG_Write32(p3548->hRegister, BCHP_IFD_NI_DEC_CTRL  , 0x080B | 0x0080);
	BREG_Write32(p3548->hRegister, BCHP_IFD_NI_DEC_CTRL  , 0x080B);
	BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_RESET  , 0x200);
	BREG_Write32(p3548->hRegister, BCHP_IFD_CORE_RESET  , 0x000);

    return BERR_SUCCESS;
}

BERR_Code BVSB_3548_P_GetIfdMode(
    BVSB_AnalogVideoMode    eVideoMode,     /* [in] Video mode */
    BVSB_AnalogAudioMode    eAudioMode,     /* [in] Audio mode */
    BVSB_IfdMode*           eIfdMode        /* [out] Returns desired IFD mode */
    )
{
	switch (eVideoMode)
	{
	case BVSB_AnalogVideoMode_eNtsc_M:
	case BVSB_AnalogVideoMode_ePal_M:
		if ( eAudioMode == BVSB_AnalogAudioMode_eBtsc)
			*eIfdMode = BVSB_IfdMode_eM_BTSC;
		else if ( eAudioMode == BVSB_AnalogAudioMode_eA2)
			*eIfdMode = BVSB_IfdMode_eM_A2;
		else
			return BVSB_ERR_INVALID_ANALOG_MODES;
		break;
	case BVSB_AnalogVideoMode_eNtsc_N:
	case BVSB_AnalogVideoMode_ePal_N:
		if ( eAudioMode != BVSB_AnalogAudioMode_eBtsc)
			return BVSB_ERR_INVALID_ANALOG_MODES;
		*eIfdMode = BVSB_IfdMode_eN;
		break;
   case BVSB_AnalogVideoMode_ePal_I:
		if ( eAudioMode != BVSB_AnalogAudioMode_eNicam)
			return BVSB_ERR_INVALID_ANALOG_MODES;
		*eIfdMode = BVSB_IfdMode_eI;
      break;
	case BVSB_AnalogVideoMode_ePal_BGH:
	case BVSB_AnalogVideoMode_eSecam_BGH:
		if ( eAudioMode == BVSB_AnalogAudioMode_eA2)
			*eIfdMode = BVSB_IfdMode_eBGH_A2;
		else if ( eAudioMode == BVSB_AnalogAudioMode_eNicam)
			*eIfdMode = BVSB_IfdMode_eBGH_NICAM;
		else
			return BVSB_ERR_INVALID_ANALOG_MODES;
		break;
	case BVSB_AnalogVideoMode_ePal_DK:
		if ( eAudioMode != BVSB_AnalogAudioMode_eNicam)
			return BVSB_ERR_INVALID_ANALOG_MODES;
		*eIfdMode = BVSB_IfdMode_eDK_NICAM;
		break;
	case BVSB_AnalogVideoMode_ePal_DK1:
	case BVSB_AnalogVideoMode_eSecam_DK:
		if ( eAudioMode != BVSB_AnalogAudioMode_eA2)
			return BVSB_ERR_INVALID_ANALOG_MODES;
		*eIfdMode = BVSB_IfdMode_eDK1_A2;
		break;
	case BVSB_AnalogVideoMode_ePal_DK2:
		if ( eAudioMode != BVSB_AnalogAudioMode_eA2)
			return BVSB_ERR_INVALID_ANALOG_MODES;
		*eIfdMode = BVSB_IfdMode_eDK2_A2;
		break;
	case BVSB_AnalogVideoMode_ePal_DK3:
		if ( eAudioMode != BVSB_AnalogAudioMode_eA2)
			return BVSB_ERR_INVALID_ANALOG_MODES;
		*eIfdMode = BVSB_IfdMode_eDK3_A2;
		break;
	case BVSB_AnalogVideoMode_eSecam_L:
		if ( eAudioMode != BVSB_AnalogAudioMode_eNicam)
			return BVSB_ERR_INVALID_ANALOG_MODES;
		*eIfdMode = BVSB_IfdMode_eL;
		break;
	default:
		return BVSB_ERR_INVALID_ANALOG_MODES;
	}
    return BERR_SUCCESS;
}

/* End of file */
