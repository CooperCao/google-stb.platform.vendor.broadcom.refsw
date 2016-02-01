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

#ifndef _BTNR_7550IB_TUNE_H__
#define _BTNR_7550IB_TUNE__H__           

#ifdef __cplusplus
extern "C" {
#endif

/*DPM Offset that is board dependent, checked with a 0dbmV input @ 207 MHz*/
#define DPM_OFFSET_256dbmV 15384  /*This gain needs to be 256 times larger so 3 db is 768, -5.5 is -1408*/

/*Divider Values for DPM tone, only used for cable*/
#define DPM_8MHZ_OUTDIV_M6 0x20 /*540000000/(4*(0x20+2))= 3970588.235 Hz*/
#define DPM_7MHZ_OUTDIV_M6 0x25 /*540000000/(4*(0x25+2))= 3461538.462 Hz*/
#define DPM_6MHZ_OUTDIV_M6 0x2B /*540000000/(4*(0x2B+2))= 3000000.000 Hz*/
#define DPM_5MHZ_OUTDIV_M6 0x34 /*540000000/(4*(0x34+2))= 2500000.000 Hz*/

/*Minimum and Maximum tuner frequencies allowed*/
/*these are also the minimum and maximum checkpoints in the Tuner_Freq_Selection_Table*/
#define MIN_TUNER_FREQ 33437500
#define MAX_TUNER_FREQ 1162500000

/*breakpoint for use of Harmonic Rejection Filter*/
#define TUNER_F_RFTRK  330000000

/*breakpoint between VHF and UHF path when in Terrrestraial Mode*/
#define VHF_UHF_BREAKPOINT 240000000

/*Constants*/
#define POWER2_31 2147483648UL
#define POWER2_29 536870912
#define POWER2_24			 16777216
#define POWER2_14      16384
#define POWER3_3       27
#define POWER5_7       78125

/*Thresholds for the window used to determine fixed gain settings in*/
/*BTNR_7550_P_Tuner_LoopVGAGainRange()*/
#ifdef IFAGC_OD_MODE
	#define MAX_AGC_THRESHOLD 	0x3C000000 /*1.45v*/
	#define MIN_AGC_THRESHOLD 	0x26000000 /*1.20v*/
	#define MAX_TUNER_VGA_LEVEL 0x70000000 /*2.15v*/
	#define MIN_TUNER_VGA_LEVEL 0xD0000000 /*0.50v*/
	#define MAX_GAIN_VGA_LEVEL  0x4F000000 /*1.70v*/
	#define MIN_GAIN_VGA_LEVEL  0x1C000000 /*1.10v*/
#else
	#define MAX_AGC_THRESHOLD 	0x1B000000 /*1.45v*/
	#define MIN_AGC_THRESHOLD 	0x00000000 /*1.20v*/
	#define MAX_TUNER_VGA_LEVEL 0x7F000000 /*2.35v*/
	#define MIN_TUNER_VGA_LEVEL 0xC0000000 /*0.60v*/
	#define MAX_GAIN_VGA_LEVEL  0x33000000 /*1.70v*/
	#define MIN_GAIN_VGA_LEVEL  0xF2000000 /*1.10v*/
#endif
#define AVG_MAXMIN_THRESHOLD (MAX_AGC_THRESHOLD + MIN_AGC_THRESHOLD)/2

/*Initial Filter BW's for Cable Mode*/
#define DS_IFLPF_BWR_SEL_8MHz 0x0F
#define DS_FGA_RC_CNTL_SEL_8MHz 0x37
#define DS_IFLPF_BWR_SEL_7MHz 0x09
#define DS_FGA_RC_CNTL_SEL_7MHz 0x32
#define DS_IFLPF_BWR_SEL_6MHz 0x07
#define DS_FGA_RC_CNTL_SEL_6MHz 0x2F
#define DS_IFLPF_BWR_SEL_5MHz 0x04
#define DS_FGA_RC_CNTL_SEL_5MHz 0x29

/*Initial Filter BW's for Terrestrial Mode*/
#define THD_IFLPF_BWR_SEL_8MHz 0x08
#define THD_FGA_RC_CNTL_SEL_8MHz 0x27
#define THD_IFLPF_BWR_SEL_7MHz 0x06
#define THD_FGA_RC_CNTL_SEL_7MHz 0x23
#define THD_IFLPF_BWR_SEL_6MHz 0x04
#define THD_FGA_RC_CNTL_SEL_6MHz 0x1d
#define THD_IFLPF_BWR_SEL_5MHz 0x02
#define THD_FGA_RC_CNTL_SEL_5MHz 0x14



/*Breakpoints for BTNR_7550_P_Tuner_FGA_BW_Selection() and BTNR_7550_P_Get_Status_TNR_Core0*/
/*Changes in the number of breakpoints will require changes in both functions*/
#define IFLPF_BWR_BREAKPOINT1 3
#define IFLPF_BWR_BREAKPOINT2 4
#define IFLPF_BWR_BREAKPOINT3 15

/*Iniital Values for BTNR_P_75xx_AcquireSettings_t Structure*/
/*These values can also be overwriten by BBS*/
/*BBS needs everything passed as uint32_t*/
#define ACQWORD0 0								/*spare Acquisition Paramater*/ /*Begining of Debugging Words*/
#define ACQWORD1 0								/*spare Acquisition Paramater*/
#define ACQWORD2 0								/*spare Acquisition Paramater*/
#define ACQWORD3 0								/*spare Acquisition Paramater*/

#define FORCE_TUNERACQUISITIONTYPE 0          /* 0 Auto, 1 InitTune, 2 Tune, 3 MiniTune*/

#define LOOPVGAGAIN_INIT_DWELL 8	/*number BKNI_Delay(500) to let AGC converge*/  
#define LOOPVGAGAIN_VGA_DWELL 8		/*number BKNI_Delay(500) to let AGC converge during trimming when VGA is changing*/
#define LOOPVGAGAIN_NOTVGA_DWELL 6  /*number BKNI_Delay(500) to let AGC converge during trimming when VGA is not changing*/
#define LOOPVGAGAIN_DISPLAY   0		/*print mvarAGCTI during BTNR_7550_P_Tuner_LoopVGAGainRange*/
#define INPUT_SOURCE 0						/*0 Internal Tuner: 1 for external tuner or IF*/
#define RERUN_INIT 0							/*Rerun The BTNR_7550_P_Init_TNR_Core0() Routine from BTNR_7550_P_Tune_TNR_Core0()*/
#define THD_RF_AGC_ACQ_BW 0xd
#define THD_IF_AGC_ACQ_BW 0xd
#define THD_RF_AGC_TRK_BW 0xf
#define THD_IF_AGC_TRK_BW 0xf
#define THD_DCOC_ON 1							/* set to "1" to enable the IF DCOC in the AFE */
#define THD_DCOC_ACQ_BW 0x8
#define THD_DCOC_TRK_BW 0xB
#define THD_BCM3410_AUTO 1				/* 0 for fixed, 1 for Auto */
#define THD_BCM3410_GAIN_STATE 31	/* This can range from 1 to 31 (only affects fixed gain mode) */
#define THD_LNA_BOOST_ON 1				/* 0 for Manual Boost off, 1 for Manual Boost on */
#define THD_LNA_TILT_ON 0					/* 0 for Tilt off, 1 for Tilt on */
#define THD_LNA_TILT_FREQ 550000000			/* Tilt Frequency */
#define DS_RF_AGC_ACQ_BW 0xd
#define DS_IF_AGC_ACQ_BW 0xd
#define DS_RF_AGC_TRK_BW 0x15
#define DS_IF_AGC_TRK_BW 0x15
#define DS_DCOC_ON	1		 					/* set to "1" to enable the IF DCOC in the AFE */
#define DS_DCOC_ACQ_BW 0x8
#define DS_DCOC_TRK_BW 0xB
#define DS_BCM3410_AUTO	1					/* 0 for fixed, 1 for Auto */
#define DS_BCM3410_GAIN_STATE 31	/* This can range from 1 to 31 (only affects fixed gain mode) */
#define DS_LNA_BOOST_ON 0					/* 0 for Manual Boost off, 1 for Manual Boost on */
#define DS_LNA_AUTO_BOOST_ON 1 				/* 0 for Auto Boost off, 1 for Auto Boost on */
#define DS_LNA_AUTO_BOOST_TOL 3				/* Auto Boost threshold*/
#define DS_LNA_TILT_ON 1					/* 0 for Tilt off, 1 for Tilt on */
#define DS_LNA_TILT_FREQ 550000000			/* Tilt Frequency */
#define DEFAULT_TERRESTRIAL_LNA_ADDRESS 0x60  /*Default used to program initial value in structure, is then scanned for by LNA.c*/
#define DEFAULT_CABLE_LNA_ADDRESS 0x61        /*Default used to program initial value in structure, is then scanned for by LNA.c*/


#ifdef REF_7550_BOARD
#define CABLE_LNA_BASE_ADDRESS 0x61			/*Cable LNA Address for QAM*/
#endif

typedef enum IFVGA_BW_s
{
	IFVGA_BW_eAcquisition = 0,
	IFVGA_BW_eTracking = 1
}IFVGA_BW_t;

typedef enum DCOC_Mode_s
{
	DCOC_Mode_eFreezeReset = 0,
	DCOC_Mode_eRun = 1
}DCOC_Mode_t;

typedef enum LNA_AGC_BW_s
{
	LNA_AGC_BW_eAcquisition = 0,
	LNA_AGC_BW_eTracking = 1
}LNA_AGC_BW_t;


/*******************************************************************************************************
*declare table to get Daisy Gain BTNR_7550_P_Tune_TNR_Core0() ******************************************
********************************************************************************************************/
#define  Daisy_Gain_Table_Size 32

typedef struct Daisy_Gain_Selection_Elements_s
{
	uint8_t DS_GAIN_CTR;
}Daisy_Gain_Selection_Elements_t;

static const Daisy_Gain_Selection_Elements_t Daisy_Gain_Selection_Table[Daisy_Gain_Table_Size] =
{ 
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, 
	{0}, {0}, {0}, {0}, {1}, {1}, {1}, {1} 
};




/*******************************************************************************************************
*declare table to get LPF Gains for BTNR_7550_P_Tune_TNR_Core0() and BTNR_7550_P_Tuner_AlignAGC2() *****
********************************************************************************************************/
#define  AlignAgc2_Gain_Table_Size 32

typedef struct AlignAgc2_Gain_Selection_Elements_s
{
	uint8_t RF_ATT_Range;
	uint8_t  RFTRK_Gain_Range;
}AlignAgc2_Gain_Selection_Elements_t;

static const AlignAgc2_Gain_Selection_Elements_t AlignAgc2_Gain_Selection_Table[AlignAgc2_Gain_Table_Size] =
{
	{3, 0}, {3, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0},
	{2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0},
	{2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0},
	{2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {1, 0}, {1, 1}
};

/*******************************************************************************************************
*declare table to get FGA_RC parameters for BTNR_7550_P_Tuner_FGA_BW_Selection() routine ***************
********************************************************************************************************/
#define  FGA_RC_Table_Size 8
#define  FGA_RC_Num_Tables 14

typedef struct FGA_RC_Selection_Elements_s
{
	uint8_t  FGA_RC_CNTL;
}FGA_RC_Selection_Elements_t;

/* declare table to get FGA_RC parameters */
static const FGA_RC_Selection_Elements_t FGA_RC_Selection_Table[FGA_RC_Num_Tables][FGA_RC_Table_Size] = 
{
   {{0},{0},{2},{8},{17},{26},{33},{39}},
   {{0},{1},{11},{21},{29},{36},{41},{46}},
   {{0},{7},{16},{25},{32},{38},{43},{48}},
   {{0},{11},{20},{28},{35},{41},{45},{50}},
   {{4},{15},{23},{31},{37},{43},{47},{51}},
   {{8},{19},{26},{33},{39},{45},{49},{52}},
   {{12},{22},{29},{36},{41},{46},{50},{53}},
   {{15},{25},{31},{38},{43},{48},{51},{54}},
   {{18},{27},{33},{39},{44},{49},{52},{55}},
   {{21},{29},{35},{41},{46},{50},{53},{56}},
   {{23},{31},{37},{42},{47},{51},{54},{57}},
   {{25},{33},{39},{44},{48},{52},{55},{57}},
   {{27},{35},{40},{45},{49},{53},{55},{58}},
   {{29},{36},{41},{46},{50},{53},{56},{58}}
};

/*******************************************************************************************************
* declare table elements for VCO used in the BTNR_7550_P_Set_Tuner_Freq() routine **********************
*the ssval11 and div_sel need to have a 1-1 mapping for the BTNR_7550_P_Get_Status_TNR_Core0() routine *
********************************************************************************************************/
#define Tuner_Freq_Selection_Num_Tables 2 
#define Tuner_Freq_Selection_Table_Size 12
typedef struct Tuner_Freq_Selection_Elements_s
{
	uint32_t tuner_freq_range;
	uint8_t ssval1;
	uint8_t div_sel;
	uint8_t tunctl;
}Tuner_Freq_Selection_Elements_t;

/*declare table to get VCO and DDFS programming parameters */
static const Tuner_Freq_Selection_Elements_t Tuner_Freq_Selection_Table[Tuner_Freq_Selection_Num_Tables][Tuner_Freq_Selection_Table_Size] = 
{
	{
		/* for 65nm */
		{36328125,   0, 0, 0}, /*error frequency too low (MIN_TUNER_FREQ-1) */
		{51250000,  64, 6, 1},
		{72656250,  64, 6, 2},
		{102500000, 32, 2, 1},
		{145312500, 32, 2, 2},
		{205000000, 16, 4, 1},
		{290625000, 16, 4, 2},
		{410000000,  8, 0, 1},
		{581250000,  8, 0, 2},
		{820000000,  4, 1, 1},
		{1162500000, 4, 1, 2}, /*MAX_TUNER_FREQ*/
		{1162500001, 0, 0, 0} /*error frequency too high*/
	},
	{
		/* for 60nm */
		{38750000,   0, 0, 0}, /*error frequency too low (MIN_TUNER_FREQ-1) */
		{57812500,  64, 6, 1},
		{77500000,  64, 6, 2},
		{115625000, 32, 2, 1},
		{155000000, 32, 2, 2},
		{231250000, 16, 4, 1},
		{310000000, 16, 4, 2},
		{462500000,  8, 0, 1},
		{620000000,  8, 0, 2},
		{925000000,  4, 1, 1},
		{1240000000, 4, 1, 2}, /*MAX_TUNER_FREQ*/
		{1240000001, 0, 0, 0} /*error frequency too high*/
	}
};

/*******************************************************************************************************
* declare table for LPF parameters used in the BTNR_7550_P_Set_Tuner_LPF() routine *********************
********************************************************************************************************/
#define LPF_Table_Size 64

typedef struct LPF_Selection_Elements_s
{
	uint32_t freq;
	uint8_t  rftrk1_in;
}LPF_Selection_Elements_t;

/*this is the original table values with each value multiplied by 1000000*4/5*/
static const LPF_Selection_Elements_t LPF_Selection_Table[LPF_Table_Size] = 
{
	{440960000, 7},{368698400, 7},{327020800, 7},{295060800, 7},{267504800, 7},{246078400, 7},{226706400, 7},{211293600, 7},
	{196712000, 7},{184606400, 7},{173451200, 7},{164412800, 7},{155444000, 5},{148614400, 5},{141256000, 5},{135237600, 5},
	{129076800, 5},{121339200, 5},{116498400, 5},{112511200, 5},{108302400, 5},{105183200, 5},{101487200, 5},{ 98428800, 5},
	{ 95168800, 5},{ 92466400, 5},{ 89576800, 5},{ 87198400, 5},{ 84617600, 5},{ 82716800, 5},{ 80396800, 5},{ 78455360, 3},
	{ 76362640, 3},{ 70454720, 3},{ 68856720, 3},{ 67536960, 3},{ 66057680, 3},{ 64992960, 3},{ 63622160, 3},{ 62467840, 3},
	{ 61187200, 3},{ 60129120, 3},{ 58933920, 3},{ 57950560, 3},{ 56825360, 3},{ 56029360, 3},{ 54984240, 3},{ 54108240, 3},
	{ 53133200, 3},{ 52098560, 3},{ 51197600, 3},{ 50451040, 3},{ 49593440, 3},{ 48985840, 3},{ 48188960, 3},{ 47510000, 3},
	{ 46752000, 3},{ 46131360, 3},{ 45410400, 3},{ 44810880, 3},{ 44138560, 3},{ 43649520, 3},{ 43003040, 3},{ 42465200, 3}
};


/*******************************************************************************************************
* declare table for CAPBIAS used in the BTNR_7550_P_Set_Tuner_VCO() routine ****************************
********************************************************************************************************/
#define CAPBIAS_Num_Tables 2
#define CAPBIAS_Table_Size 64
typedef struct CAPBIAS_Selection_Elements_s
{
	uint8_t ip_qpbiascnt2;
	uint8_t ip_rcntl;
}CAPBIAS_Selection_Elements_t;

static const CAPBIAS_Selection_Elements_t CAPBIAS_Selection_Table[CAPBIAS_Num_Tables][CAPBIAS_Table_Size] =
{
	{
		/*Lookup Table for the IP_QPBIASCNT2 and IP_RCNTL for tunecnt = 1, LIMIT1=32, LIMIT2=11 */
		{0x04, 0x03}, {0x04, 0x03}, {0x04, 0x03}, {0x04, 0x03}, {0x04, 0x03}, {0x08, 0x02}, {0x08, 0x02}, {0x08, 0x02},
		{0x08, 0x02}, {0x08, 0x02}, {0x08, 0x02}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01},
		{0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01},
		{0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01},
		{0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00},
		{0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00},
		{0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00},
		{0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}
	},
	{	
		/*Lookup Table for the IP_QPBIASCNT2 and IP_RCNTL for tunecnt = 2, LIMIT1 47, LIMIT2 15*/
		{0x04, 0x03}, {0x04, 0x03}, {0x04, 0x03}, {0x04, 0x03}, {0x04, 0x03}, {0x08, 0x02}, {0x08, 0x02}, {0x08, 0x02},
		{0x08, 0x02}, {0x08, 0x02}, {0x08, 0x02}, {0x08, 0x02}, {0x08, 0x02}, {0x08, 0x02}, {0x08, 0x02}, {0x0C, 0x01},
		{0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01},
		{0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01},
		{0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01},
		{0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0C, 0x01}, {0x0D, 0x00},
		{0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00},
		{0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}, {0x0D, 0x00}
	}
};

/*Main functions used in btnr_7550_tune.c*/
void BTNR_7550_P_Set_Tuner_Freq(BTNR_75xx_Handle);
void BTNR_7550_P_Set_Tuner_DDFS(BTNR_75xx_Handle, uint8_t);
void BTNR_7550_P_Set_Tuner_VCO(BTNR_75xx_Handle, uint8_t);
void BTNR_7550_P_Set_Tuner_VHForUHF(BTNR_75xx_Handle);
void BTNR_7550_P_Set_Tuner_LPF(BTNR_75xx_Handle);
void BTNR_7550_P_Set_Tuner_IFVGA_BW(BTNR_75xx_Handle, IFVGA_BW_t);
void BTNR_7550_P_Set_Tuner_DCOC_Mode(BTNR_75xx_Handle, DCOC_Mode_t);
void BTNR_7550_P_Tuner_Set_BB_LPF_Filter(BTNR_75xx_Handle);
void BTNR_7550_P_Tuner_AlignAGCT(BTNR_75xx_Handle);
void BTNR_7550_P_Tuner_AlignAGC2(BTNR_75xx_Handle, uint8_t);
void BTNR_7550_P_Tuner_LoopVGAGainRange(BTNR_75xx_Handle);
void BTNR_7550_P_Tuner_FGA_BW_Selection(BTNR_75xx_Handle);

/*Utility functions btnr_7550_utils.c*/
void BTNR_7550_P_Tuner_Init_Buffer(BTNR_75xx_Handle);
void BTNR_7550_P_Range_Check(BTNR_75xx_Handle);
uint32_t BTNR_7550_P_2560log10(uint32_t);
void BTNR_7550_P_Tuner_MXR_DCO_DAC(BTNR_75xx_Handle);
void BTNR_7550_P_Tuner_MXR_DCO_LOOP(BTNR_75xx_Handle);				 
void BTNR_7550_P_Tuner_VGA_DCO_LOOP(BTNR_75xx_Handle);	
void BTNR_7550_P_Tuner_AGCT_DCO_LOOP(BTNR_75xx_Handle);
void BTNR_7550_P_Tuner_Device_type(BTNR_75xx_Handle);

/*EXTERNAL LNA FUNCTIONS*/
void BTNR_7550_P_Init_LNA(BTNR_75xx_Handle);
void BTNR_7550_P_Set_LNA_Boost(BTNR_75xx_Handle);
void BTNR_7550_P_Set_LNA_Tilt(BTNR_75xx_Handle);
uint8_t BTNR_7550_P_Get_LNA_Gain(BTNR_75xx_Handle);
void BTNR_7550_P_Set_LNA_Output(BTNR_75xx_Handle);
void BTNR_7550_P_Probe_First_LNA(BTNR_75xx_Handle);
void BTNR_7550_P_Probe_Second_LNA(BTNR_75xx_Handle);
void BTNR_7550_P_Wait_For_LNA_AGCLock(BTNR_75xx_Handle h);
uint8_t BTNR_7550_P_Get_LNA_AGCLock_Status(BTNR_75xx_Handle);
void BTNR_7550_P_Set_LNA_AGC_BW(BTNR_75xx_Handle, LNA_AGC_BW_t);


#ifdef __cplusplus
}
#endif

#endif /* _BTNR_7550_TUNE_PRIV_H__ */


