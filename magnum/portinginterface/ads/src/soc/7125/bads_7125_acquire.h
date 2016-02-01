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

#ifndef _BADS_7125_ACQUIRE_H__
#define _BADS_7125_ACQUIRE_H__           

#ifdef __cplusplus
extern "C" {
#endif

/*DPM Tone locations used by bads_7125_dpm()*/
/*The DPM is called by the tuner*/
#define ANNEXA_DPM_TONE 3970588
#define ANNEXB_DPM_TONE 3000000


/*Lock Detector Values*/
/*Every 25 mS the PI calles the BADS_7125_P_Get_LockStatus function*/
/*Lock is declared IF the number of clean blocks detected in since the previous call is >= NUM_CLEAN_BLOCKS_TO_LOCK*/
/*if this condition is not met then*/
/*Lock is declared if there have NOT been more then NUM_BAD_BLOCK_TO_UNLOCK bad blocks have accumulated*/
/*if this condition is not met then*/
/*Unlock is declared  if there have been more then NUM_BAD_BLOCK_TO_UNLOCK bad blocks have accumulated*/
#define NUM_CLEAN_BLOCKS_TO_LOCK 1
#define NUM_BAD_BLOCK_TO_UNLOCK 1000
#define STUCK_FEC_RESET_COUNT 2

#define MAIN_PLL_REFERENCE_FREQUENCY 54000000

#define MIN_BAUD_RATE 1000000
#define MAX_BAUD_RATE 7300000

#define Q64_ANNEXB_SYMBOL_RATE 5056941
#define Q256_Q1024_ANNEXB_SYMBOL_RATE 5360537

#define MAX_TIMING_OFFSET_FOR_LOCK 2500

/*Inital Values for BADS_7125_P_AcquireParams_t Structure*/
/*These values can also be overwriten by BBS*/
/*BBS needs everything passed as uint32_t*/
#define BBS_ACQWORD0 512		/*SNR Leaky Average Constant*/
#define BBS_ACQWORD1 0			/*Print Timing Params*/  /*0 off, 1 on*/
#define BBS_ACQWORD2 0			/*unused*/
#define BBS_ACQWORD3 0			/*unused*/
#define RERUN_INIT 0				/*0 NoRerun, 1 ReRun*/
#define BURST_MODE 0				/*0 off, 1 on*/
#define CCW_ENABLE 1				/*0 off, 1 on*/
#define CCW1_FREQUENCY 0
#define CCW2_FREQUENCY 0
#define CCW3_FREQUENCY 0
#define CCW4_FREQUENCY 0
#define IQ_IMB_ON 1					/*0 off, 1 on*/
#define CFL_ON 1					/*0 off, 1 on*/
#define SWEEP_DIRECTION 0			/*0 sweep up from neg freq, 1 sweep down from pos freq*/
#define DDAGC_ON 1					/*0 off, 1 on*/
#define IMC_ON 1					/*0 off, 1 on*/
#define IQPHS_ON 1					/*0 off, 1 on*/			

#define POWER2_16 65536
#define POWER2_24 16777216
#define POWER2_25 33554432
#define POWER2_30 1073741824
#define POWER2_31 2147483648UL

#define NUM_QAM_MODES 9
#define NUM_BAUD_RATES 16
#define DECIMATION_TABLE_SIZE 4

/*Combined Timing Loop Coefficients*/
#define BCHP_DS_0_TLC_COMBO_COEFFS_SHIFT BCHP_DS_0_TLC_TLICOEFF_SHIFT
#define BCHP_DS_0_TLC_COMBO_COEFFS_MASK (BCHP_DS_0_TLC_reserved1_MASK | BCHP_DS_0_TLC_TLLCOEFF_SIGN_MASK | BCHP_DS_0_TLC_TLLCOEFF_MASK | BCHP_DS_0_TLC_reserved2_MASK | BCHP_DS_0_TLC_TLICOEFF_SIGN_MASK | BCHP_DS_0_TLC_TLICOEFF_MASK)

/*Combined Frequency Loop Coefficients*/
#define BCHP_DS_0_CFLC_COMBO_COEFFS_SHIFT BCHP_DS_0_CFLC_CFLICOEFF_SHIFT
#define BCHP_DS_0_CFLC_COMBO_COEFFS_MASK (BCHP_DS_0_CFLC_reserved1_MASK | BCHP_DS_0_CFLC_CFLLCOEFF_SIGN_MASK | BCHP_DS_0_CFLC_CFLLCOEFF_MASK | BCHP_DS_0_CFLC_reserved2_MASK | BCHP_DS_0_CFLC_CFLICOEFF_SIGN_MASK | BCHP_DS_0_CFLC_CFLICOEFF_MASK)

/*Combined Phase Loop Coefficients*/
#define BCHP_DS_EQ_0_CPLC_COMBO_COEFFS_SHIFT BCHP_DS_EQ_0_CPLC_CPLICOEFF_SHIFT
#define BCHP_DS_EQ_0_CPLC_COMBO_COEFFS_MASK (BCHP_DS_EQ_0_CPLC_CPLLCOEFF_MASK | BCHP_DS_EQ_0_CPLC_CPLICOEFF_MASK)

/*Combined DFE Taps*/
#define BCHP_DS_0_FRZ_COMBO_DFEFRZ_SHIFT BCHP_DS_0_FRZ_DFEFRZ1_6_SHIFT
#define BCHP_DS_0_FRZ_COMBO_DFEFRZ_MASK (BCHP_DS_0_FRZ_DFEFRZ31_36_MASK | BCHP_DS_0_FRZ_DFEFRZ25_30_MASK | BCHP_DS_0_FRZ_DFEFRZ19_24_MASK | BCHP_DS_0_FRZ_DFEFRZ13_18_MASK | BCHP_DS_0_FRZ_DFEFRZ7_12_MASK | BCHP_DS_0_FRZ_DFEFRZ1_6_MASK)

/*combined FEC counter clears*/
#define BCHP_DS_0_TPFEC_CLEARCNT2_SHIFT  BCHP_DS_0_TPFEC_CLR_BMPG2_SHIFT
#define BCHP_DS_0_TPFEC_CLEARCNT2_MASK   (BCHP_DS_0_TPFEC_CLR_UERC2_MASK | BCHP_DS_0_TPFEC_CLR_NBERC2_MASK | BCHP_DS_0_TPFEC_CLR_CBERC2_MASK | BCHP_DS_0_TPFEC_CLR_BMPG2_MASK)

typedef struct MVAR_s
{	
	uint8_t		BaudRateIndex;
	uint32_t	mvarDownSymbolRate;
	uint32_t	mvarSwp_Delay;
	int32_t		mvarSwp_Rate;
	int32_t		mvarSwp_Start;
}MVAR_t;





/*The timing loop coefficients are scaled from the 256 QAM 5.056/5.36 MBaud coefficients that had a Filter Factor of 1*/
/*The scaling for the Linear Term is C_Linear_original*FilterFactor                                                   */
/*The scaling for the Integrator Term is C_Integrator_original*FilterFactor*Fb_original/Fb_new                        */
/*Fb original = 5.056/5.36 MBaud, original FF = 1, original coeffs for Acq are 0x0x00003839,                          */
/*original coeffs for Trk1 0x00003634, original coeffs for Trk2 0x00003430                                            */

/*The table is based on the decimation rate due to the ARF (1 or 2) and FilterFactor (1, 2 or 4)					  */
/*The total decimation to the VID is then 1,2,4 or 8 corresponding to table entries of 0,1,2, and 3                   */

static const uint16_t TimingLoopAcqCoeffs_TBL[NUM_BAUD_RATES][DECIMATION_TABLE_SIZE] =
{
{0x383B, 0x383B, 0x393C, 0x393C},
{0x383B, 0x383B, 0x393C, 0x393C},
{0x383B, 0x383B, 0x393C, 0x393C},
{0x383B, 0x383B, 0x393C, 0x393C},
{0x383B, 0x383B, 0x393C, 0x393C},
{0x383A, 0x383A, 0x393B, 0x393B},
{0x383A, 0x383A, 0x393B, 0x393B},
{0x383A, 0x383A, 0x393B, 0x393B},
{0x383A, 0x383A, 0x393B, 0x393B},
{0x383A, 0x383A, 0x393B, 0x393B},
{0x383A, 0x383A, 0x393B, 0x393B},
{0x3839, 0x3839, 0x393A, 0x393A},
{0x3839, 0x3839, 0x393A, 0x393A},
{0x3839, 0x3839, 0x393A, 0x393A},
{0x3839, 0x3839, 0x393A, 0x393A},
{0x3839, 0x3839, 0x393A, 0x393A}
};

static const uint16_t TimingLoopTrk1Coeffs_TBL[NUM_BAUD_RATES][DECIMATION_TABLE_SIZE] =
{
{0x3636, 0x3636, 0x3737, 0x3737},
{0x3636, 0x3636, 0x3737, 0x3737},
{0x3636, 0x3636, 0x3737, 0x3737},
{0x3636, 0x3636, 0x3737, 0x3737},
{0x3636, 0x3636, 0x3737, 0x3737},
{0x3635, 0x3635, 0x3736, 0x3736},
{0x3635, 0x3635, 0x3736, 0x3736},
{0x3635, 0x3635, 0x3736, 0x3736},
{0x3635, 0x3635, 0x3736, 0x3736},
{0x3635, 0x3635, 0x3736, 0x3736},
{0x3635, 0x3635, 0x3736, 0x3736},
{0x3634, 0x3634, 0x3735, 0x3735},
{0x3634, 0x3634, 0x3735, 0x3735},
{0x3634, 0x3634, 0x3735, 0x3735},
{0x3634, 0x3634, 0x3735, 0x3735},
{0x3634, 0x3634, 0x3735, 0x3735}
};

static const uint16_t TimingLoopTrk2Coeffs_TBL[NUM_BAUD_RATES][DECIMATION_TABLE_SIZE] =
{
{0x3432, 0x3432, 0x3533, 0x3533},
{0x3432, 0x3432, 0x3533, 0x3533},
{0x3432, 0x3432, 0x3533, 0x3533},
{0x3432, 0x3432, 0x3533, 0x3533},
{0x3432, 0x3432, 0x3533, 0x3533},
{0x3431, 0x3431, 0x3532, 0x3532},
{0x3431, 0x3431, 0x3532, 0x3532},
{0x3431, 0x3431, 0x3532, 0x3532},
{0x3431, 0x3431, 0x3532, 0x3532},
{0x3431, 0x3431, 0x3532, 0x3532},
{0x3431, 0x3431, 0x3532, 0x3532},
{0x3430, 0x3430, 0x3531, 0x3531},
{0x3430, 0x3430, 0x3531, 0x3531},
{0x3430, 0x3430, 0x3531, 0x3531},
{0x3430, 0x3430, 0x3531, 0x3531},
{0x3430, 0x3430, 0x3531, 0x3531}
};


static const uint16_t FrequencyLoopCoeffs_TBL[NUM_BAUD_RATES] =
{
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923,
0x2923
};

/********************************************************************************************************/
/****** To program the phase loop linear and integrator coefficients use the following formulas in excel*/
/****** 	Clin_dec = -2*Zeta*(2*3.14*Fn/(Fb*1000000))								 ********************/
/****** 	Clin_hex = DEC2HEX(POWER(2,16)+ROUND((Clin_dec*182/0.018245512),0))      ********************/
/****** 	Cint_dec = -POWER((2*3.14*Fn/(Fb*1000000)),2)                            ********************/          
/****** 	Cint_hex = DEC2HEX(POWER(2,16)+ROUND((Cint_dec*27/0.000167446),0))       ********************/
/******     Zeta is the desired Zeta, Fn is the desired actual natural frequency     ********************/
/******     Fb is the baud rate see above for the table							     ********************/
/********************************************************************************************************/
/*PhaseLoopAcqCoeffs are calculated with zeta = 1.50 and Fn = 22500 Hz        */
/*PhaseLoopTrkCoeffs are calculated with zeta = 0.71 and Fn = 29200 Hz        */
/*In the future we might want to change the zeta and Fn based on the Baud Rate*/

static const uint32_t PhaseLoopAcqCoeffs_TBL[NUM_BAUD_RATES] =
{
0xF07EF4EA,
0xF246F751,
0xF3DBF933,
0xF540FAAD,
0xF67DFBD4,
0xF795FCBC,
0xF88DFD71,
0xF968FDFF,
0xFA2BFE6E,
0xFAD6FEC5,
0xFB6EFF0A,
0xFBF5FF3F,
0xFC6CFF69,
0xFCD5FF8A,
0xFD33FFA3,
0xFD85FFB7
};

static const uint32_t PhaseLoopTrkCoeffs_TBL[NUM_BAUD_RATES] =
{
0xF679ED53,
0xF792F160,
0xF88AF48C,
0xF966F708,
0xFA28F8FA,
0xFAD4FA80,
0xFB6DFBB1,
0xFBF3FCA0,
0xFC6BFD5C,
0xFCD4FDEE,
0xFD32FE61,
0xFD84FEBB,
0xFDCDFF02,
0xFE0EFF39,
0xFE47FF64,
0xFE7AFF86
};

/*SweepRates and SweepTimes and Delay in ms                                                    */
/*PosSweepRate = SweepRate*2^26/BaudRate^2                                                     */
/*PosSweepStart = (-SweepRange/2)*(2^30/BaudRate)                                              */
/*NegSweepRate, and NegSweepRate are the 2's compement of the Pos values                       */
/*SweepTime = (SweepRange*2^26)/(PosSweepRate*BaudRate^2 )                                     */
typedef struct PhaseLoopSweep_s
{
uint16_t PosSweepRate;
uint32_t PosSweepStart;
uint16_t NegSweepRate;
uint32_t NegSweepStart;
uint16_t SweepTime;
}PhaseLoopSweep_t; 


/*Values for PhaseLoopSweepAnnexA use SweepRange = 420KHz, SweepRate 5.95MHz/sec at 5.21 MBaud */  
static const PhaseLoopSweep_t PhaseLoopSweepAnnexA_TBL[NUM_BAUD_RATES] =
{
{0x048, 0x33615A24, 0xFB8, 0x0C9EA5DC, 345},
{0x040, 0x34D50502, 0xFC0, 0x0B2AFAFE, 304},
{0x038, 0x361DEDC7, 0xFC8, 0x09E21239, 272},
{0x032, 0x3740FFBC, 0xFCE, 0x08BF0044, 239},
{0x02C, 0x38429549, 0xFD4, 0x07BD6AB7, 212},
{0x027, 0x392688A4, 0xFD9, 0x06D9775C, 188},
{0x023, 0x39F0428D, 0xFDD, 0x060FBD73, 164},
{0x01F, 0x3AA2C75B, 0xFE1, 0x055D38A5, 145},
{0x01B, 0x3B40C28B, 0xFE5, 0x04BF3D75, 130},
{0x018, 0x3BCC90FA, 0xFE8, 0x04336F06, 115},
{0x015, 0x3C4849ED, 0xFEB, 0x03B7B613, 103},
{0x013, 0x3CB5C716, 0xFED, 0x034A38EA, 89},
{0x011, 0x3D16ABA7, 0xFEF, 0x02E95459, 78},
{0x00F, 0x3D6C6A98, 0xFF1, 0x02939568, 69},
{0x00D, 0x3DB84C35, 0xFF3, 0x0247B3CB, 62},
{0x00B, 0x3DFB7304, 0xFF5, 0x02048CFC, 58}
};

/*Values for PhaseLoopSweepAnnexB are for fixed Baud Rates and are from the cable script       */ 
static const PhaseLoopSweep_t PhaseLoopSweepAnnexB_TBL[2] =
{
/*{0x010, 0x3DB8D140, 0xFF0, 0x02472EC0, 59},
{0x010, 0x3DD9D89D, 0xFF0, 0x02262763, 53},*/
{0x012, 0x3DB8D140, 0xFEE, 0x02472EC0, 53},
{0x010, 0x3DD9D89D, 0xFEF, 0x02262763, 53},
};

static const uint32_t SNRLTHRESH_TBL[NUM_QAM_MODES] =
{
0xb54,		/*16Qam Low threshold approx. 55 dB*/	
0xb54,		/*32Qam Low threshold approx. 55 dB*/	
0xb54,		/*64Qam Low threshold approx. 55 dB*/	
0xb54,		/*128Qam Low threshold approx. 55 dB*/	
0xb54,		/*256Qam Low threshold approx. 55 dB*/	
0xb54,		/*512Qam Low threshold approx. 55 dB*/	
0xb54,		/*1024Qam Low threshold approx. 55 dB*/	
0x0,		/*2048Qam Low threshold undefined*/	
0x0			/*4096Qam Low threshold undefined*/	
};

/*I think this table is wrong*/
#if 0
static const uint32_t SNRHTHRESH_TBL[NUM_QAM_MODES] =
{
0x49AB3,	/*16Qam High Threshold 15 dB*/
0x34831,	/*32Qam High Threshold 19 dB*/
0x34831,	/*64Qam High Threshold 19 dB*/
0x178C4,	/*128Qam High Threshold 25 dB*/
0x1A798,	/*256Qam High Threshold 25 dB*/
0x093E4,	/*512Qam High Threshold 28 dB*/
0x06A86,	/*1024Qam High Threshold 31 dB*/
0x0,		/*2048Qam High threshold undefined*/	
0x0			/*4096Qam High threshold undefined*/	
};
#endif

static const uint32_t SNRHTHRESH_TBL[NUM_QAM_MODES] =
{
0x664DA,	/*16Qam High Threshold 13 dB*/
0x486CD,	/*32Qam High Threshold 16 dB*/
0x3345E,	/*64Qam High Threshold 19 dB*/
0x244C7,	/*128Qam High Threshold 22 dB*/
0x19B28,	/*256Qam High Threshold 25 dB*/
0x12314,	/*512Qam High Threshold 28 dB*/
0x0CE11,	/*1024Qam High Threshold 31 dB*/
0x0,		/*2048Qam High threshold undefined*/	
0x0			/*4096Qam High threshold undefined*/	
};


typedef struct FEC_s
{
uint32_t DS_FECU;
uint32_t DS_FECM;
uint32_t DS_FECL;
uint32_t DS_FECOUT_NCON;
uint32_t DS_FECOUT_NCODL;
}FEC_t;

static const FEC_t AnnexA_FEC_TBL[NUM_QAM_MODES] =
{
	{0x004A0000, 0xFF800620, 0x08050039, 0x0000002F, 0x0000006A},	/* Annex A 16QAM*/
	{0x004A0000, 0xFF800620, 0x08050049, 0x000000EB, 0x00000179},	/* Annex A 32QAM*/
	{0x004A0000, 0xFF800620, 0x08050059, 0x0000002F, 0x00000037},	/* Annex A 64QAM*/
	{0x004A0000, 0xFF800620, 0x08050069, 0x00000149, 0x0000011B},	/* Annex A 128QAM*/
	{0x004A0000, 0xFF800620, 0x08050079, 0x0000005E, 0x0000003B},	/* Annex A 256QAM*/
	{0x004A0000, 0xFF800620, 0x08050089, 0x0000002F, 0x00000015},	/* Annex A 512QAM*/
	{0x004A0000, 0xFF800620, 0x08050099, 0x000000EB, 0x00000047},	/* Annex A 1024QAM*/
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	/* Annex A 2048QAM unsupported*/
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}	/* Annex A 4096QAM unsupported*/
};

static const FEC_t AnnexB_FEC_TBL[NUM_QAM_MODES] =
{
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	/* Annex A 16QAM unsupported*/
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	/* Annex A 32QAM unsupported*/
	{0x00E00000, 0x1E01E0FF, 0x01000059, 0x00000004, 0x00000005},	/* Annex A 64QAM*/
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	/* Annex A 128QAM unsupported*/
	{0x00E00000, 0x1E01E0FF, 0x01000079, 0x00001259, 0x00000C10},	/* Annex A 256QAM*/
	{0x00E00000, 0x1E01FE01, 0x01000089, 0x00001259, 0x00000887},	/* Annex A 512QAM*/
	{0x00E00000, 0x1E01FE01, 0x01000099, 0x00003DB7, 0x00001343},	/* Annex A 1024QAM*/
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	/* Annex A 2048QAM unsupported*/
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}	/* Annex A 4096QAM unsupported*/
};

static const uint32_t DS_EQ_CTL_TBL[NUM_QAM_MODES] =
{
0x00040909,	/*16Qam DS_EQ_CTL*/
0x000C0A12,	/*32Qam DS_EQ_CTL*/
0x00040B1B,	/*64Qam DS_EQ_CTL*/
0x000C0C24,	/*128Qam DS_EQ_CTL*/
0x00040D2D,	/*256Qam DS_EQ_CTL*/
0x000C0E36,	/*512Qam DS_EQ_CTL*/
0x00040F3F,	/*1024Qam DS_EQ_CTL*/
0x0,		/*2048Qam High threshold undefined*/	
0x0			/*4096Qam High threshold undefined*/	
};

static const uint32_t DS_EQ_FN_TBL[NUM_QAM_MODES] =
{
0x40002800,	/*16Qam DS_EQ_FN same as dflt*/
0x40001400,	/*32Qam DS_EQ_FN OVERRIDE Value (incorrect dflt= 0x238E)*/
0x40002A00,	/*64Qam DS_EQ_FN same as dflt*/
0x40001480,	/*128Qam DS_EQ_FN OVERRIDE Value (incorrect dflt= 0x24E1)*/
0x40002A80,	/*256Qam DS_EQ_FN same as dflt*/
0x400014A0,	/*512Qam DS_EQ_FN OVERRIDE Value (incorrect dflt= 0x24AA)*/
0x40002AA0,	/*1024Qam DS_EQ_FN same as dflt*/
0x0,		/*2048Qam High threshold undefined*/	
0x0			/*4096Qam High threshold undefined*/	
};

static const uint32_t DS_EQ_CPL_TBL[NUM_QAM_MODES] =
{
0xC1000001,	/*16Qam Freq Det=ON, diag points, reset CPL loop*/
0xC0400001,	/*32Qam Freq Det=ON, diag points, reset CPL loop*/
0x40000001,	/*64Qam Freq Det=Off, reset CPL loop, use diag points*/
0x40000001,	/*128Qam Freq Det=Off, reset CPL loop, use diag points*/
0x40000001,	/*256Qam Freq Det=Off, reset CPL loop, use diag points*/
0x40000001,	/*512Qam Freq Det=Off, reset CPL loop, use diag points*/
0x40000001,	/*1024Qam Freq Det=Off, reset CPL loop, use diag points*/
0x0,		/*2048Qam High threshold undefined*/	
0x0			/*4096Qam High threshold undefined*/	
};

/*Main functions used in bads_7125_acquire.c*/
void BADS_7125_P_ProgramFilters(BADS_ChannelHandle); 
void BADS_7125_P_ProgramFEC(BADS_ChannelHandle);
MVAR_t BADS_7500_P_Get_MVAR(BADS_ChannelHandle);
uint32_t BADS_7500_P_SLEEP(BADS_ChannelHandle, uint8_t, uint32_t);

/*Main functions used in bads_7125_status.c*/

/*Utility functions bads_7125_utils.c*/
void BADS_7125_P_ADS_Init_Buffer(BADS_7125_ChannelHandle);
void BADS_7125_P_Range_Check(BADS_7125_ChannelHandle);
uint32_t BADS_7125_P_2560log10(uint32_t);

void BADS_7125_P_Set_SymbolRate(BADS_ChannelHandle, uint32_t);
void BADS_7125_P_Set_TimingFrequency(BADS_ChannelHandle, uint32_t, uint32_t, uint8_t, uint8_t);
void BADS_7125_P_Set_IfFrequency(BADS_ChannelHandle, uint32_t, uint32_t);

uint32_t BADS_7125_P_Get_SampleRate(BADS_ChannelHandle);
uint8_t BADS_7125_P_Get_ARFFactor(BADS_ChannelHandle);
uint8_t  BADS_7125_P_Get_FilterFactor(BADS_ChannelHandle);
uint32_t BADS_7125_P_Get_SymbolRate(BADS_ChannelHandle, uint8_t, uint8_t, uint32_t);
int32_t BADS_7125_P_Get_IfFrequency(BADS_ChannelHandle, uint32_t, uint8_t);
int32_t BADS_7125_P_Get_VIDTimingError(BADS_ChannelHandle, uint8_t, uint8_t, uint32_t);
int32_t BADS_7125_P_Get_CarrierFrequencyError(BADS_ChannelHandle, uint32_t);
int32_t BADS_7125_P_Get_CarrierPhaseError(BADS_ChannelHandle, uint32_t);

/******************************/

#ifdef __cplusplus
}
#endif

#endif /* _BADS_7125_ACQUIRE_H__ */

