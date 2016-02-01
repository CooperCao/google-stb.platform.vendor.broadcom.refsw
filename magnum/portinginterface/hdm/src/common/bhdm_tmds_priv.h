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

/*****************************************************************************/
/*                                                                           */
/*                            DO NOT HAND EDIT!!!                            */
/*                                                                           */
/* Parameters generated on 07/19/2014 at 03:54:25 PM                         */
/*                                                                           */
/*                            DO NOT HAND EDIT!!!                            */
/*                                                                           */
/*    WARNING!!! This file is auto-generated file.  The ordering of the      */
/*               various enums and tables is important to the overall        */
/*               functionality!!                                             */
/*                                                                           */
/*   Did we mention?:        DO NOT HAND EDIT!!!                             */
/*                                                                           */
/*****************************************************************************/
#ifndef BHDM_TMDS_PRIV_H__
#define BHDM_TMDS_PRIV_H__


typedef enum BHDM_P_TmdsClock
{

	BHDM_P_TmdsClock_e25_2   =   0,                   /*   0 */
	BHDM_P_TmdsClock_e31_5,                           /*   1 */
	BHDM_P_TmdsClock_e37_8,                           /*   2 */

	BHDM_P_TmdsClock_e25_2_DIV_1_001,                 /*   3 */
	BHDM_P_TmdsClock_e31_5_DIV_1_001,                 /*   4 */
	BHDM_P_TmdsClock_e37_8_DIV_1_001,                 /*   5 */

	BHDM_P_TmdsClock_e27,                             /*   6 */
	BHDM_P_TmdsClock_e33_75,                          /*   7 */
	BHDM_P_TmdsClock_e40_5,                           /*   8 */
	BHDM_P_TmdsClock_e54,                             /*   9 */
	BHDM_P_TmdsClock_e67_5,                           /*  10 */
	BHDM_P_TmdsClock_e81,                             /*  11 */
	BHDM_P_TmdsClock_e108,                            /*  12 */
	BHDM_P_TmdsClock_e135,                            /*  13 */
	BHDM_P_TmdsClock_e162,                            /*  14 */

	BHDM_P_TmdsClock_e27_MUL_1_001,                   /*  15 */
	BHDM_P_TmdsClock_e33_75_MUL_1_001,                /*  16 */
	BHDM_P_TmdsClock_e40_5_MUL_1_001,                 /*  17 */
	BHDM_P_TmdsClock_e54_MUL_1_001,                   /*  18 */
	BHDM_P_TmdsClock_e67_5_MUL_1_001,                 /*  19 */
	BHDM_P_TmdsClock_e81_MUL_1_001,                   /*  20 */
	BHDM_P_TmdsClock_e108_MUL_1_001,                  /*  21 */
	BHDM_P_TmdsClock_e135_MUL_1_001,                  /*  22 */
	BHDM_P_TmdsClock_e162_MUL_1_001,                  /*  23 */



	BHDM_P_TmdsClock_e74_25,                          /*  24 */
	BHDM_P_TmdsClock_e92_8125,                        /*  25 */
	BHDM_P_TmdsClock_e111_375,                        /*  26 */

	BHDM_P_TmdsClock_e74_25_DIV_1_001,                /*  27 */
	BHDM_P_TmdsClock_e92_8125_DIV_1_001,              /*  28 */
	BHDM_P_TmdsClock_e111_375_DIV_1_001,              /*  29 */


	BHDM_P_TmdsClock_e148_5,                          /*  30 */
	BHDM_P_TmdsClock_e185_625,                        /*  31 */
	BHDM_P_TmdsClock_e222_75,                         /*  32 */

	BHDM_P_TmdsClock_e148_5_DIV_1_001,                /*  33 */
	BHDM_P_TmdsClock_e185_625_DIV_1_001,              /*  34 */
	BHDM_P_TmdsClock_e222_75_DIV_1_001,               /*  35 */

	BHDM_P_TmdsClock_e297,                            /*  36 */
	BHDM_P_TmdsClock_e371_25,                         /*  37 */
	BHDM_P_TmdsClock_e445_5,                          /*  38 */

	BHDM_P_TmdsClock_e297_DIV_1_001,                  /*  39 */
	BHDM_P_TmdsClock_e371_25_DIV_1_001,               /*  40 */
	BHDM_P_TmdsClock_e445_5_DIV_1_001,                /*  41 */

	BHDM_P_TmdsClock_e594,                            /*  42 */

	BHDM_P_TmdsClock_e594_DIV_1_001,                  /*  43 */

	BHDM_P_TmdsClock_e65,                             /*  44 */
	BHDM_P_TmdsClock_e81_25,                          /*  45 */
	BHDM_P_TmdsClock_e97_5,                           /*  46 */

	BHDM_P_TmdsClock_e65_DIV_1_001,                   /*  47 */
	BHDM_P_TmdsClock_e81_25_DIV_1_001,                /*  48 */
	BHDM_P_TmdsClock_e97_5_DIV_1_001,                 /*  49 */


	BHDM_P_TmdsClock_eMax
} BHDM_P_TmdsClock;

/******************************************************************************
Summary:
28nm Pre-emphasis configurations settings

Description:
This structure can be used to set up values for Pre-Emphasis Control


See Also:

Note:
This structure applies to 28nm and smaller process.
See RDB for more detail
*******************************************************************************/
typedef struct
{

	/*******************************************/
	/* Amplitude Values */

	 struct
	{
		/*******************************************/
		/*  pre-emphasis amplitude settings: 3 MSB's of the PREEMP_0/1/2/CK fields
		0x0	0.0 mA	0x1	1.0 mA	0x2	2.0 mA
		0x3	3.0 mA	0x4	0.5 mA	0x5	1.5 mA
		0x6	2.5 mA	0x7	3.5 mA  */

		uint8_t PreEmphasis ;

		/*******************************************/
		/* main driver amplitude settings: 5 LSB's of the PREEMP_0/1/2/CK fields
		0x00  0 mA	0x01	  1 mA	0x02	  2 mA
		0x03	  3 mA	0x04	  4 mA	0x05	  5 mA
		0x06	  6 mA	0x07	  7 mA	0x08	  8 mA
		0x09	  9 mA	0x0A 10 mA	0x0B	 11 mA
		0x0C 12 mA	0x0D 13 mA	0x0E	 14 mA
		0x0F	15 mA	0x10	  8 mA	0x11	  9 mA
		0x12	 10 mA	0x13	 11 mA	0x14	 12 mA
		0x15	 13 mA	0x16	 14 mA	0x17	 15 mA
		0x18	 16 mA	0x19	 17 mA	0x1A 18 mA
		0x1B	 19 mA	0x1C 20 mA	0x1D 21 mA
		0x1E	 22 mA	0x1F	 23 mA */

		uint8_t MainDriver ;
	}  Amplitude ;

	/*******************************************/
	/* Set the resistor bias:
		For bit rate < 1.6Gbps, set to 11000
		> 1.6Gbps, set to 10010
		Maximum total current from this section cannot exceed 1.2mA
	*/
	uint8_t ResSelData ;

	/*******************************************/
	/* Set the source termination resistor */
	/*	0000: no terminal resistor from TX
		0001: 100 ohms differential termination resistor from TX
		0011: 50 ohms differential termination resistor from TX (Without intra-pair skew boosting)
		1111: 50 ohms differential termination resistor from TX (With intra-pair skew boosting)
		other settings: not valid
	*/
	uint8_t TermResSelData ;
} BHDM_PreEmphasisChannelSettings ;

typedef struct {
	uint32_t MinTmdsRate ;
	uint32_t MaxTmdsRate ;
	BHDM_PreEmphasisChannelSettings Channel[3] ;
	BHDM_PreEmphasisChannelSettings Clock ;
} BHDM_TmdsRatePreEmphasisSettings ;


void BHDM_TMDS_P_GetDefaultPreEmphasisRegisters(
	BHDM_TmdsPreEmphasisRegisters *PreEmphasisRegisters) ;

const char * BHDM_P_TmdsClockToText_isrsafe(BHDM_P_TmdsClock eTmdsClock) ;


uint32_t BHDM_P_TmdsClockToValue_isrsafe(
	BHDM_P_TmdsClock eTmdsClock
) ;

BERR_Code BHDM_TMDS_P_VideoFormatSettingsToTmdsRate(
	const BHDM_Handle hHDMI,		/* [in] HDMI handle */
	const BFMT_VideoFmt eVideoFmt, /* [in] eVideoFmt */
	const BHDM_Video_Settings *settings, /* [in] settings */
	uint32_t *tmdsRate /* [in] settings */
) ;

#endif
