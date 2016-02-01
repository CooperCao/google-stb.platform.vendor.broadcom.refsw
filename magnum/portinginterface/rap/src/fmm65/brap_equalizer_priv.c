/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
*   Module name: Hardware Equalizer
*   This file contains the implementation of all APIs for the hardware Equalizer
*   abstraction. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE(rap_equalizer);		/* Register software module with debug interface */


/* --------------------------- EQ SPECIFICS BEGIN ----------------------------- */

/******************************************************************************
  *
  *     Copyright (c) 2004-2010, Broadcom Corporation
  *     All Rights Reserved
  *     Confidential Property of Broadcom Corporation
  *
  *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
  *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
  *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
  *
  *	FILE NAME:	equalizer.c
  *
  *	Description:
  *			This file contains various equalizer generation algorithms.
  *		The fixed point code is written for 3548 HW where coefs are represented 
  *		in 2.26 format and the equation implemented is
  *		y(n) = b0 * x(n) + b1 * x(n-1) + b2 * x(n-2) + a1 * y(n-1) + a2 * y(n-2)
  *
  *	Revision History:
  *  20-Aug-08	P. Anandh	Initial Code to generate the coefs with sample rate = 192kHz
  *
  *  20-Aug-08	P. Anandh	Variable sample rate supported
  *							Check for valid ranges
  *							Modified the db2linear table to use multiplication
  *								instead of division
  *  17-Sep-08	P. Anandh	Support for PEQ, shelving filter and subsonic filter
  *							are added.
  *  29-Sep-08	P. Anandh	Fixed the bug in high-shelving filter for input 0.
  *
  ******************************************************************************/

/******************************************************************************
  *
  *	FUNCTION NAME:	EQ_generate_subsonic_coefs_isr
  *
  *	Description:
  *			This function takes the user defined cut-off level for high pass filter
  *		The function returns the pre-stored coefs in required format. The high pass
  *		filter is 4th order butterwoth filter. Hence the this function returns
  *		2 biquad filter coefs.
  *
  *	Input:	
  *		fs_index	- 0 for 32kHz
  *					- 1 for 44.1 kHz
  *					- 2 for 48 kHz
  *		subsonic_param
  *					- This value is an index as defined by sharp
  *					- 0 for 40 Hz
  *					- 1 for 45 Hz
  *					- 2 for 50 Hz
  *					- 3 for 56 Hz ...
  *					- 18 for 315 Hz
  *	Output:
  *		filter1_b [], filter2_b[], filter1_a[], filter2_a[]
  *			- biquad filter coefs.
  *
  *	Return:
  *		0 - On successful generation of coefs.
  *		Non-zero - In case of input values out of range
  *
  ******************************************************************************/

int32_t EQ_generate_subsonic_coefs_isr (
		int32_t fs_int,
		int32_t subsonic_freq,
		int32_t b0[],
		int32_t b1[],
		int32_t b2[],
		int32_t a1[],
		int32_t a2[]
		);

/******************************************************************************
  *
  *	FUNCTION NAME:	EQ_generate_high_shelving_isr
  *
  *	Description:
  *			This generates coefs. for high shelving filter. It takes input value
  *		in dB level. Allowed range is +/- 12dB. Since the filter coefs. exceeds
  *		+/- 4, the coefs are divided by 4 and returned. Hence after passing
  *		through this filter, the sample should be multiplied by 4
  *		This function generates single biquad filter.
  *
  *	Input:	
  *		dbval_int	- Gain in dB*10 to allow accuary of +/-0.1 dB
  *					(e.g) to set it to 3.2 dB, dbval_int should be 32
  *
  *	Output:
  *		b_int, a_int
  *			- biquad filter coefs.
  *
  *	Return:
  *		0 - On successful generation of coefs.
  *		Non-zero - In case of input values out of range
  *
  ******************************************************************************/
int32_t EQ_generate_high_shelving_isr (
		int32_t dbval_int,
	   	int32_t fs_int,
	   	int32_t b_int[],
	   	int32_t a_int[]);

/******************************************************************************
  *
  *	FUNCTION NAME:	EQ_generate_low_shelving_isr
  *
  *	Description:
  *			This generates coefs. for high shelving filter. It takes input value
  *		in dB level. Allowed range is +/- 12dB. This function generates
  *		single biquad filter.
  *
  *	Input:	
  *		dbval_int	- Gain in dB*10 to allow accuary of +/-0.1 dB
  *					(e.g) to set it to 3.2 dB, dbval_int should be 32
  *
  *	Output:
  *		b_int, a_int
  *			- biquad filter coefs.
  *
  *	Return:
  *		0 - On successful generation of coefs.
  *		Non-zero - In case of input values out of range
  *
  ******************************************************************************/
int32_t EQ_generate_low_shelving_isr (
		int32_t dbval,
	   	int32_t fs,
	   	int32_t b[],
	   	int32_t a[]);

/******************************************************************************
  *
  *	FUNCTION NAME:	generate_equaliser_coefs
  *
  *	Description:
  *			This function takes the user defined level for graphic equaliser
  *			and generates coefs required for 3548 hardware
  *
  *	Input:	
  *		req_db[]	- Array of values representing the level for 5 different
  *						center frequency.
  *						(e.g) req_db[0] = 43;	refers to 4.3dB for 100 Hz
  *							req_db[3] = -90;	refers to -9dB for 3160 Hz
  *		fs			- Sampling rate at which the equalizer runs
  *						Tested for 32000, 44100, 48000
  *
  *	Output:
  *		b0, b1, b2, a1, a2 - Array of values where the coefs are stored
  *	Return:
  *		0 - On successful generation of coefs.
  *		Non-zero - In case of input values out of range
  * Note:
  *		This function shall support the dB ranges from -12dB to +12dB.
  *		The frequencies are fixed at 100 Hz, 316 Hz, 1000 Hz, 3160 Hz, 10000 Hz
  *		The sampling rates are considered to be
  *		2^n * 32000, 2^n * 44100 or 2^n * 48000
  *
  ******************************************************************************/

int32_t EQ_generate_equaliser_coefs_isr(
		int32_t fs,
	   	int32_t req_db[5],
	   	int32_t b0[5],
	   	int32_t b1[5],
	   	int32_t b2[5],
	   	int32_t a1[5],
	   	int32_t a2[5]);

/******************************************************************************
  *
  *	FUNCTION NAME:	generate_equaliser_coefs
  *
  *	Description:
  *			This function takes the user defined level for parametric equaliser
  *		and generates coefs required for 3548 hardware
  *			This function generated only one biquad. For multiple band this
  *		function shall be called multiple times. For eg. to generate coef. for
  *		7-band equalizer, the function shall be called 7-times. Each function
  *		call shall give one set of parameters.
  *
  *	Input:	
  *		
  *		q_int		- 'Q' value of the equalizer
  *						'Q' = fc/bandwidth. The value q_int = Q * 100. This
  *						allows 'Q' to be represented in +/- 0.01 resolution.
  *		freq_int	- center frequency in Hz
  *		
  *		dbval_int	- Array of values representing the level for 5 different
  *						center frequency.
  *						(e.g) req_db[0] = 43;	refers to 4.3dB for 100 Hz
  *							req_db[3] = -90;	refers to -9dB for 3160 Hz
  *		fs			- Sampling rate at which the equalizer runs
  *						Tested for 32000, 44100, 48000
  *
  *	Output:
  *		b_int, a_int
  *			- Biquad filter coefs.
  *	Return:
  *		0 - On successful generation of coefs.
  *		Non-zero - In case of input values out of range
  * Note:
  *		This function shall support the dB ranges from -12dB to +12dB.
  *
  ******************************************************************************/

int32_t EQ_generate_peq_coefs_isr( 
    unsigned int q[], 
    unsigned int freq[], 
    int gain[], 
    uint32_t fs, 
    int32_t b0[], 
    int32_t b1[], 
    int32_t b2[], 
    int32_t a1[], 
    int32_t a2[]    
    );

/********  Equalizer Algorithm ************************/

#define EQ_CONST_2_X_PI_DIV_8   ( (int32_t) (0x6487ED51))
#define EQ_LIMIT_DW             ( (int32_t) (0x0274518B))
#define EQ_SQRT_2_IN_5_27       ( (int32_t) (0x0B504F33))
#define EQ_NSUBSONIC_FREQ       ( (int32_t) (19))

int32_t EQ_mul_32_32 (int32_t a, int32_t b);
int32_t EQ_div_32_32 (int32_t num, int32_t den);
void EQ_cos_sin (int32_t phase, int32_t *cos_val, int32_t *sin_val);
int32_t EQ_db_2_linear(int32_t in, int32_t table[]);
int32_t EQ_sqrt_32 (int32_t in);

void EQ_renormalise (int32_t *p_mant, int32_t *p_exp);

int32_t EQ_subsonic_cutoff_freq [EQ_NSUBSONIC_FREQ] = {
	40, 45, 50, 56, 63, 70, 80, 90, 100, 110, 125, 140, 160, 180, 200, 220, 250, 280, 315
};
int32_t EQ_subsonic_cutoff_freq_mid [EQ_NSUBSONIC_FREQ-1] = {
	43, 48, 53, 60, 67, 75, 85, 95, 105, 118, 133, 150, 170, 190, 210, 235, 265, 298
};

int32_t EQ_subwoofer_table[3][EQ_NSUBSONIC_FREQ][12] = {
	{

		{
			458,  917,  458,
			458,  917,  458,
			67108864,  -133569758,  66462725,
			67108864,  -133947495,  66840467
		},
		{
			580,  1160,  580,
			580,  1160,  580,
			67108864,  -133488944,  66382396,
			67108864,  -133913534,  66806993
		},
		{
			716,  1431,  716,
			716,  1431,  716,
			67108864,  -133408169,  66302162,
			67108864,  -133879534,  66773537
		},
		{
			897,  1794,  897,
			897,  1794,  897,
			67108864,  -133311294,  66206012,
			67108864,  -133838679,  66733411
		},
		{
			1135,  2270,  1135,
			1135,  2270,  1135,
			67108864,  -133198346,  66094011,
			67108864,  -133790942,  66686628
		},
		{
			1400,  2800,  1400,
			1400,  2800,  1400,
			67108864,  -133085476,  65982199,
			67108864,  -133743127,  66639878
		},
		{
			1827,  3655,  1827,
			1827,  3655,  1827,
			67108864,  -132924370,  65822795,
			67108864,  -133674684,  66573150
		},
		{
			2311,  4621,  2311,
			2311,  4621,  2311,
			67108864,  -132763424,  65663774,
			67108864,  -133606081,  66506489
		},
		{
			2850,  5701,  2850,
			2850,  5701,  2850,
			67108864,  -132602638,  65505135,
			67108864,  -133537318,  66439895
		},
		{
			3446,  6892,  3446,
			3446,  6892,  3446,
			67108864,  -132442011,  65346878,
			67108864,  -133468396,  66373370
		},
		{
			4444,  8888,  4444,
			4444,  8888,  4444,
			67108864,  -132201370,  65110205,
			67108864,  -133364716,  66273707
		},
		{
			5568,  11135,  5568,
			5568,  11135,  5568,
			67108864,  -131961086,  64874384,
			67108864,  -133260681,  66174197
		},
		{
			7260,  14520,  7260,
			7260,  14520,  7260,
			67108864,  -131641261,  64561274,
			67108864,  -133121415,  66041753
		},
		{
			9173,  18345,  9173,
			9173,  18345,  9173,
			67108864,  -131322067,  64249664,
			67108864,  -132981521,  65909579
		},
		{
			11305,  22610,  11305,
			11305,  22610,  11305,
			67108864,  -131003501,  63939544,
			67108864,  -132841002,  65777675
		},
		{
			13656,  27313,  13656,
			13656,  27313,  13656,
			67108864,  -130685561,  63630906,
			67108864,  -132699861,  65646042
		},
		{
			17590,  35180,  17590,
			17590,  35180,  17590,
			67108864,  -130209820,  63170709,
			67108864,  -132486990,  65449099
		},
		{
			22010,  44019,  22010,
			22010,  44019,  22010,
			67108864,  -129735473,  62713799,
			67108864,  -132272734,  65252765
		},
		{
			27774,  55548,  27774,
			27774,  55548,  27774,
			67108864,  -129183822,  62184854,
			67108864,  -132021032,  65024478
		},

	},
	{

		{
			458,  917,  458,
			458,  917,  458,
			67108864,  -133569758,  66462725,
			67108864,  -133947495,  66840467
		},
		{
			580,  1160,  580,
			580,  1160,  580,
			67108864,  -133488944,  66382396,
			67108864,  -133913534,  66806993
		},
		{
			716,  1431,  716,
			716,  1431,  716,
			67108864,  -133408169,  66302162,
			67108864,  -133879534,  66773537
		},
		{
			897,  1794,  897,
			897,  1794,  897,
			67108864,  -133311294,  66206012,
			67108864,  -133838679,  66733411
		},
		{
			1135,  2270,  1135,
			1135,  2270,  1135,
			67108864,  -133198346,  66094011,
			67108864,  -133790942,  66686628
		},
		{
			1400,  2800,  1400,
			1400,  2800,  1400,
			67108864,  -133085476,  65982199,
			67108864,  -133743127,  66639878
		},
		{
			1827,  3655,  1827,
			1827,  3655,  1827,
			67108864,  -132924370,  65822795,
			67108864,  -133674684,  66573150
		},
		{
			2311,  4621,  2311,
			2311,  4621,  2311,
			67108864,  -132763424,  65663774,
			67108864,  -133606081,  66506489
		},
		{
			2850,  5701,  2850,
			2850,  5701,  2850,
			67108864,  -132602638,  65505135,
			67108864,  -133537318,  66439895
		},
		{
			3446,  6892,  3446,
			3446,  6892,  3446,
			67108864,  -132442011,  65346878,
			67108864,  -133468396,  66373370
		},
		{
			4444,  8888,  4444,
			4444,  8888,  4444,
			67108864,  -132201370,  65110205,
			67108864,  -133364716,  66273707
		},
		{
			5568,  11135,  5568,
			5568,  11135,  5568,
			67108864,  -131961086,  64874384,
			67108864,  -133260681,  66174197
		},
		{
			7260,  14520,  7260,
			7260,  14520,  7260,
			67108864,  -131641261,  64561274,
			67108864,  -133121415,  66041753
		},
		{
			9173,  18345,  9173,
			9173,  18345,  9173,
			67108864,  -131322067,  64249664,
			67108864,  -132981521,  65909579
		},
		{
			11305,  22610,  11305,
			11305,  22610,  11305,
			67108864,  -131003501,  63939544,
			67108864,  -132841002,  65777675
		},
		{
			13656,  27313,  13656,
			13656,  27313,  13656,
			67108864,  -130685561,  63630906,
			67108864,  -132699861,  65646042
		},
		{
			17590,  35180,  17590,
			17590,  35180,  17590,
			67108864,  -130209820,  63170709,
			67108864,  -132486990,  65449099
		},
		{
			22010,  44019,  22010,
			22010,  44019,  22010,
			67108864,  -129735473,  62713799,
			67108864,  -132272734,  65252765
		},
		{
			27774,  55548,  27774,
			27774,  55548,  27774,
			67108864,  -129183822,  62184854,
			67108864,  -132021032,  65024478
		},

	},
	{

		{
			458,  917,  458,
			458,  917,  458,
			67108864,  -133569758,  66462725,
			67108864,  -133947495,  66840467
		},
		{
			580,  1160,  580,
			580,  1160,  580,
			67108864,  -133488944,  66382396,
			67108864,  -133913534,  66806993
		},
		{
			716,  1431,  716,
			716,  1431,  716,
			67108864,  -133408169,  66302162,
			67108864,  -133879534,  66773537
		},
		{
			897,  1794,  897,
			897,  1794,  897,
			67108864,  -133311294,  66206012,
			67108864,  -133838679,  66733411
		},
		{
			1135,  2270,  1135,
			1135,  2270,  1135,
			67108864,  -133198346,  66094011,
			67108864,  -133790942,  66686628
		},
		{
			1400,  2800,  1400,
			1400,  2800,  1400,
			67108864,  -133085476,  65982199,
			67108864,  -133743127,  66639878
		},
		{
			1827,  3655,  1827,
			1827,  3655,  1827,
			67108864,  -132924370,  65822795,
			67108864,  -133674684,  66573150
		},
		{
			2311,  4621,  2311,
			2311,  4621,  2311,
			67108864,  -132763424,  65663774,
			67108864,  -133606081,  66506489
		},
		{
			2850,  5701,  2850,
			2850,  5701,  2850,
			67108864,  -132602638,  65505135,
			67108864,  -133537318,  66439895
		},
		{
			3446,  6892,  3446,
			3446,  6892,  3446,
			67108864,  -132442011,  65346878,
			67108864,  -133468396,  66373370
		},
		{
			4444,  8888,  4444,
			4444,  8888,  4444,
			67108864,  -132201370,  65110205,
			67108864,  -133364716,  66273707
		},
		{
			5568,  11135,  5568,
			5568,  11135,  5568,
			67108864,  -131961086,  64874384,
			67108864,  -133260681,  66174197
		},
		{
			7260,  14520,  7260,
			7260,  14520,  7260,
			67108864,  -131641261,  64561274,
			67108864,  -133121415,  66041753
		},
		{
			9173,  18345,  9173,
			9173,  18345,  9173,
			67108864,  -131322067,  64249664,
			67108864,  -132981521,  65909579
		},
		{
			11305,  22610,  11305,
			11305,  22610,  11305,
			67108864,  -131003501,  63939544,
			67108864,  -132841002,  65777675
		},
		{
			13656,  27313,  13656,
			13656,  27313,  13656,
			67108864,  -130685561,  63630906,
			67108864,  -132699861,  65646042
		},
		{
			17590,  35180,  17590,
			17590,  35180,  17590,
			67108864,  -130209820,  63170709,
			67108864,  -132486990,  65449099
		},
		{
			22010,  44019,  22010,
			22010,  44019,  22010,
			67108864,  -129735473,  62713799,
			67108864,  -132272734,  65252765
		},
		{
			27774,  55548,  27774,
			27774,  55548,  27774,
			67108864,  -129183822,  62184854,
			67108864,  -132021032,  65024478
		},
	}
};

int32_t EQ_subsonic_table[3][EQ_NSUBSONIC_FREQ][12] = {
	{
		{
			0x03fac26b, 0xf80a7b27, 0x03fac26b,
			0x03fac26b, 0xf80a7b2f, 0x03fac26a,
			0x04000000, 0xf806332d, 0x03f9dcf2,
			0x04000000, 0xf80ed0ea, 0x03f13f24,
		},
		{
			0x03fa1b33, 0xf80bc995, 0x03fa1b34,
			0x03fa1b33, 0xf80bc99d, 0x03fa1b33,
			0x04000000, 0xf806fb2b, 0x03f9193b,
			0x04000000, 0xf810a96f, 0x03ef6ade,
		},
		{
			0x03f97417, 0xf80d17cd, 0x03f97419,
			0x03f97417, 0xf80d17d5, 0x03f97416,
			0x04000000, 0xf807c382, 0x03f855a9,
			0x04000000, 0xf8128199, 0x03ed9771,
		},
		{
			0x03f8abb4, 0xf80ea895, 0x03f8abb5,
			0x03f8abb4, 0xf80ea89d, 0x03f8abb2,
			0x04000000, 0xf808b462, 0x03f76b2d,
			0x04000000, 0xf814b7bc, 0x03eb67a4,
		},
		{
			0x03f7c21c, 0xf8107bc5, 0x03f7c21c,
			0x03f7c21c, 0xf8107bcd, 0x03f7c21b,
			0x04000000, 0xf809ce0b, 0x03f659e1,
			0x04000000, 0xf8174b96, 0x03e8dc13,
		},
		{
			0x03f6d8b9, 0xf8124e8a, 0x03f6d8ba,
			0x03f6d8b9, 0xf8124e93, 0x03f6d8b7,
			0x04000000, 0xf80ae863, 0x03f548e0,
			0x04000000, 0xf819debf, 0x03e65227,
		},
		{
			0x03f58bad, 0xf814e8a3, 0x03f58bad,
			0x03f58bad, 0xf814e8ab, 0x03f58bac,
			0x04000000, 0xf80c7cec, 0x03f3c35f,
			0x04000000, 0xf81d8b38, 0x03e2b489,
		},
		{
			0x03f43f0d, 0xf81781e2, 0x03f43f0e,
			0x03f43f0d, 0xf81781ea, 0x03f43f0d,
			0x04000000, 0xf80e12d9, 0x03f23e76,
			0x04000000, 0xf821364b, 0x03df1a40,
		},
		{
			0x03f2f2da, 0xf81a1a49, 0x03f2f2da,
			0x03f2f2da, 0xf81a1a50, 0x03f2f2d9,
			0x04000000, 0xf80faa2a, 0x03f0ba25,
			0x04000000, 0xf824dffa, 0x03db8349,
		},
		{
			0x03f1a713, 0xf81cb1d7, 0x03f1a713,
			0x03f1a713, 0xf81cb1df, 0x03f1a712,
			0x04000000, 0xf81142dc, 0x03ef366c,
			0x04000000, 0xf8288845, 0x03d7ef9f,
		},
		{
			0x03efb632, 0xf8209399, 0x03efb633,
			0x03efb632, 0xf82093a0, 0x03efb631,
			0x04000000, 0xf813aa7c, 0x03ecf1f3,
			0x04000000, 0xf82e021d, 0x03d2984b,
		},
		{
			0x03edc643, 0xf8247376, 0x03edc645,
			0x03edc643, 0xf824737e, 0x03edc641,
			0x04000000, 0xf8161533, 0x03eaaed1,
			0x04000000, 0xf83378db, 0x03cd4850,
		},
		{
			0x03eb327b, 0xf8299b05, 0x03eb327d,
			0x03eb327b, 0xf8299b0d, 0x03eb327a,
			0x04000000, 0xf81952eb, 0x03e7acb7,
			0x04000000, 0xf83abd0b, 0x03c63e5d,
		},
		{
			0x03e8a05e, 0xf82ebf3f, 0x03e8a05f,
			0x03e8a05e, 0xf82ebf47, 0x03e8a05e,
			0x04000000, 0xf81c960e, 0x03e4acfe,
			0x04000000, 0xf841fbc6, 0x03bf414b,
		},
		{
			0x03e60fea, 0xf833e028, 0x03e60feb,
			0x03e60fea, 0xf833e02f, 0x03e60fea,
			0x04000000, 0xf81fde94, 0x03e1afa6,
			0x04000000, 0xf8493512, 0x03b850fd,
		},
		{
			0x03e3811e, 0xf838fdc1, 0x03e3811e,
			0x03e3811e, 0xf838fdc9, 0x03e3811d,
			0x04000000, 0xf8232c72, 0x03deb4ae,
			0x04000000, 0xf85068fa, 0x03b16d57,
		},
		{
			0x03dfae00, 0xf840a3fd, 0x03dfae00,
			0x03dfae00, 0xf840a405, 0x03dfadff,
			0x04000000, 0xf8282b32, 0x03da40b2,
			0x04000000, 0xf85b2cc7, 0x03a72f5d,
		},
		{
			0x03dbde8f, 0xf84842dd, 0x03dbde90,
			0x03dbde8f, 0xf84842e5, 0x03dbde8f,
			0x04000000, 0xf82d35c5, 0x03d5d210,
			0x04000000, 0xf865e49d, 0x039d0d44,
		},
		{
			0x03d7712b, 0xf8511da6, 0x03d7712d,
			0x03d7712b, 0xf8511dae, 0x03d7712a,
			0x04000000, 0xf833263c, 0x03d0ad19,
			0x04000000, 0xf87256c3, 0x03915d94,
		},
	},
	{
		{
			0x03fc31d5, 0xf8079c52, 0x03fc31d5,
			0x03fc31d5, 0xf8079c5a, 0x03fc31d4,
			0x04000000, 0xf8047d6f, 0x03fb8b10,
			0x04000000, 0xf80ac27e, 0x03f545fa,
		},
		{
			0x03fbb851, 0xf8088f5b, 0x03fbb852,
			0x03fbb851, 0xf8088f62, 0x03fbb850,
			0x04000000, 0xf8050df4, 0x03fafccb,
			0x04000000, 0xf80c19f5, 0x03f3f0c1,
		},
		{
			0x03fb3edb, 0xf8098247, 0x03fb3edb,
			0x03fb3edb, 0xf809824e, 0x03fb3eda,
			0x04000000, 0xf8059ea9, 0x03fa6e9b,
			0x04000000, 0xf80d713e, 0x03f29bfa,
		},
		{
			0x03faad2d, 0xf80aa5a2, 0x03faad2e,
			0x03faad2d, 0xf80aa5a9, 0x03faad2d,
			0x04000000, 0xf8064c8e, 0x03f9c415,
			0x04000000, 0xf80f0cef, 0x03f103a1,
		},
		{
			0x03fa0352, 0xf80bf958, 0x03fa0353,
			0x03fa0352, 0xf80bf95f, 0x03fa0352,
			0x04000000, 0xf80717c5, 0x03f8fd48,
			0x04000000, 0xf810ece8, 0x03ef280b,
		},
		{
			0x03f95994, 0xf80d4cd5, 0x03f95994,
			0x03f95994, 0xf80d4cdd, 0x03f95993,
			0x04000000, 0xf807e358, 0x03f836a2,
			0x04000000, 0xf812cc83, 0x03ed4d53,
		},
		{
			0x03f86746, 0xf80f3170, 0x03f86747,
			0x03f86746, 0xf80f3177, 0x03f86745,
			0x04000000, 0xf80906cb, 0x03f71b1d,
			0x04000000, 0xf8157909, 0x03eaa8aa,
		},
		{
			0x03f77532, 0xf8111598, 0x03f77533,
			0x03f77532, 0xf811159f, 0x03f77532,
			0x04000000, 0xf80a2afc, 0x03f5ffe8,
			0x04000000, 0xf81824d2, 0x03e805c7,
		},
		{
			0x03f68358, 0xf812f94d, 0x03f68358,
			0x03f68358, 0xf812f954, 0x03f68357,
			0x04000000, 0xf80b4fe9, 0x03f4e503,
			0x04000000, 0xf81acfdd, 0x03e564a8,
		},
		{
			0x03f591b6, 0xf814dc8f, 0x03f591b7,
			0x03f591b6, 0xf814dc97, 0x03f591b6,
			0x04000000, 0xf80c7592, 0x03f3ca6f,
			0x04000000, 0xf81d7a2c, 0x03e2c54c,
		},
		{
			0x03f427af, 0xf817b09d, 0x03f427b0,
			0x03f427af, 0xf817b0a5, 0x03f427af,
			0x04000000, 0xf80e2f70, 0x03f22325,
			0x04000000, 0xf8217841, 0x03ded98c,
		},
		{
			0x03f2be29, 0xf81a83ab, 0x03f2be29,
			0x03f2be29, 0xf81a83b2, 0x03f2be28,
			0x04000000, 0xf80feaf2, 0x03f07c90,
			0x04000000, 0xf82574b0, 0x03daf1b9,
		},
		{
			0x03f0dce7, 0xf81e462e, 0x03f0dce7,
			0x03f0dce7, 0xf81e4636, 0x03f0dce6,
			0x04000000, 0xf8123cd5, 0x03ee4a36,
			0x04000000, 0xf82ac2b5, 0x03d5c2b5,
		},
		{
			0x03eefc88, 0xf82206ec, 0x03eefc88,
			0x03eefc88, 0xf82206f4, 0x03eefc87,
			0x04000000, 0xf814919e, 0x03ec191c,
			0x04000000, 0xf8300dd1, 0x03d09a99,
		},
		{
			0x03ed1d0b, 0xf825c5e6, 0x03ed1d0c,
			0x03ed1d0b, 0xf825c5ee, 0x03ed1d0b,
			0x04000000, 0xf816e948, 0x03e9e943,
			0x04000000, 0xf8355606, 0x03cb795b,
		},
		{
			0x03eb3e70, 0xf829831d, 0x03eb3e70,
			0x03eb3e70, 0xf8298325, 0x03eb3e6f,
			0x04000000, 0xf81943d1, 0x03e7baaa,
			0x04000000, 0xf83a9b59, 0x03c65ef0,
		},
		{
			0x03e8722b, 0xf82f1ba6, 0x03e8722c,
			0x03e8722b, 0xf82f1bad, 0x03e8722b,
			0x04000000, 0xf81cd0f7, 0x03e4771d,
			0x04000000, 0xf8427df2, 0x03bec403,
		},
		{
			0x03e5a7de, 0xf834b03f, 0x03e5a7df,
			0x03e5a7de, 0xf834b047, 0x03e5a7de,
			0x04000000, 0xf820647c, 0x03e13662,
			0x04000000, 0xf84a5a1d, 0x03b73833,
		},
		{
			0x03e268fe, 0xf83b2e01, 0x03e268fe,
			0x03e268fe, 0xf83b2e08, 0x03e268fd,
			0x04000000, 0xf8249897, 0x03dd6e6c,
			0x04000000, 0xf8537d93, 0x03ae7d56,
		},
	},
	{
		{
			0x03fc80d9, 0xf806fe4a, 0x03fc80db,
			0x03fc80d9, 0xf806fe52, 0x03fc80d7,
			0x04000000, 0xf8041f99, 0x03fbe793,
			0x04000000, 0xf809e321, 0x03f62406,
		},
		{
			0x03fc112b, 0xf807dda6, 0x03fc112c,
			0x03fc112b, 0xf807ddad, 0x03fc112a,
			0x04000000, 0xf804a442, 0x03fb64d2,
			0x04000000, 0xf80b1ed0, 0x03f4ea3c,
		},
		{
			0x03fba189, 0xf808bce9, 0x03fba18a,
			0x03fba189, 0xf808bcf1, 0x03fba189,
			0x04000000, 0xf8052912, 0x03fae221,
			0x04000000, 0xf80c5a57, 0x03f3b0d3,
		},
		{
			0x03fb1ba4, 0xf809c8b4, 0x03fb1ba5,
			0x03fb1ba4, 0xf809c8bb, 0x03fb1ba4,
			0x04000000, 0xf805c8a9, 0x03fa4563,
			0x04000000, 0xf80dd4c2, 0x03f2393c,
		},
		{
			0x03fa7f84, 0xf80b00f3, 0x03fa7f85,
			0x03fa7f84, 0xf80b00fb, 0x03fa7f84,
			0x04000000, 0xf8068322, 0x03f98ea4,
			0x04000000, 0xf80f8df6, 0x03f083bb,
		},
		{
			0x03f9e37c, 0xf80c3903, 0x03f9e37e,
			0x03f9e37c, 0xf80c390b, 0x03f9e37b,
			0x04000000, 0xf8073de9, 0x03f8d806,
			0x04000000, 0xf81146dc, 0x03eecef7,
		},
		{
			0x03f904bf, 0xf80df67f, 0x03f904bf,
			0x03f904bf, 0xf80df686, 0x03f904be,
			0x04000000, 0xf8084944, 0x03f7d35e,
			0x04000000, 0xf813bc2e, 0x03ec604a,
		},
		{
			0x03f82632, 0xf80fb399, 0x03f82632,
			0x03f82632, 0xf80fb3a1, 0x03f82631,
			0x04000000, 0xf8095540, 0x03f6cef9,
			0x04000000, 0xf81630e0, 0x03e9f31e,
		},
		{
			0x03f747d5, 0xf8117052, 0x03f747d5,
			0x03f747d5, 0xf811705a, 0x03f747d4,
			0x04000000, 0xf80a61da, 0x03f5cad7,
			0x04000000, 0xf818a4f2, 0x03e7876f,
		},
		{
			0x03f669a9, 0xf8132cab, 0x03f669aa,
			0x03f669a9, 0xf8132cb2, 0x03f669a8,
			0x04000000, 0xf80b6f14, 0x03f4c6f9,
			0x04000000, 0xf81b1865, 0x03e51d3e,
		},
		{
			0x03f51cc1, 0xf815c67b, 0x03f51cc1,
			0x03f51cc1, 0xf815c682, 0x03f51cc0,
			0x04000000, 0xf80d0414, 0x03f341ab,
			0x04000000, 0xf81ec466, 0x03e180bd,
		},
		{
			0x03f3d045, 0xf8185f72, 0x03f3d046,
			0x03f3d045, 0xf8185f79, 0x03f3d044,
			0x04000000, 0xf80e9a77, 0x03f1bcf5,
			0x04000000, 0xf8226f02, 0x03dde790,
		},
		{
			0x03f2159e, 0xf81bd4c0, 0x03f2159f,
			0x03f2159e, 0xf81bd4c7, 0x03f2159e,
			0x04000000, 0xf810ba79, 0x03efb799,
			0x04000000, 0xf8275053, 0x03d9207a,
		},
		{
			0x03f05bb7, 0xf81f488e, 0x03f05bb8,
			0x03f05bb7, 0xf81f4895, 0x03f05bb7,
			0x04000000, 0xf812dcef, 0x03edb34b,
			0x04000000, 0xf82c2f2d, 0x03d45f40,
		},
		{
			0x03eea28f, 0xf822bade, 0x03eea290,
			0x03eea28f, 0xf822bae5, 0x03eea28e,
			0x04000000, 0xf81501d6, 0x03ebb00b,
			0x04000000, 0xf8310b93, 0x03cfa3d8,
		},
		{
			0x03ecea26, 0xf8262baf, 0x03ecea28,
			0x03ecea26, 0xf8262bb8, 0x03ecea25,
			0x04000000, 0xf817292b, 0x03e9adda,
			0x04000000, 0xf835e587, 0x03caee3a,
		},
		{
			0x03ea56ed, 0xf82b5222, 0x03ea56ee,
			0x03ea56ed, 0xf82b522a, 0x03ea56ed,
			0x04000000, 0xf81a68b2, 0x03e6ac8b,
			0x04000000, 0xf83d27e4, 0x03c3e895,
		},
		{
			0x03e7c55e, 0xf8307540, 0x03e7c55f,
			0x03e7c55e, 0xf8307548, 0x03e7c55d,
			0x04000000, 0xf81dada2, 0x03e3ad9d,
			0x04000000, 0xf84464cf, 0x03bcefc7,
		},
		{
			0x03e4c84f, 0xf8366f5e, 0x03e4c850,
			0x03e4c84f, 0xf8366f65, 0x03e4c84f,
			0x04000000, 0xf82184d8, 0x03e031de,
			0x04000000, 0xf84ccfb2, 0x03b4dd96,
		},
	},
};

int32_t EQ_pow_table_db_int_neg[] = {
    0x0ffffc50, 0x0ffff8a1, 0x0ffff143, 0x0fffe286,
    0x0fffc50e, 0x0fff8a1d, 0x0fff143d, 0x0ffe2889,
    0x0ffc5149, 0x0ff8a36a, 0x0ff14a39, 0x0fe2a1f8,
    0x0fc579d7, 0x0f8bc9c0, 0x0f1adf93, 0x0e429057,

    0x0cb59185, 0x0a1866ba, 0x065ea59f, 0x02892c18,
    0x0066e309, 0x0002959b, 0x0000006a, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

int32_t EQ_pow_table_db_int[] = {
    0x100003af, 0x1000075e, 0x10000ebc, 0x10001d79,
    0x10003af2, 0x100075e6, 0x1000ebcf, 0x1001d7ac,
    0x1003af90, 0x10075ff9, 0x100ec359, 0x101d9452,
    0x103b5f53, 0x10779af8, 0x10f2b408, 0x11f3c99f,

    0x14248ef8, 0x195bb8f6, 0x2830afd3, 0x64f40348,
    0x80000000, 0x80000000, 0x80000000, 0x80000000,
    0x80000000, 0x80000000, 0x80000000, 0x80000000,
    0x80000000, 0x80000000, 0x80000000, 0x80000000
};

int32_t EQ_matr_int[5][5] = {
    { 0x04b41d9c, 0xe6b58c1b, 0x021af614, 0xffd74c36, 0x000303f1 },
    { 0xe6b5ab37, 0x097f2df7, 0xe660ede3, 0x0222d777, 0xffd7503d },
    { 0x021ad54d, 0xe6622b92, 0x09869086, 0xe64dd084, 0x021c5dee },
    { 0xffd76060, 0x0221d872, 0xe6564cd7, 0x0981594e, 0xe6c01389 },
    { 0x00030012, 0xffd78307, 0x021a43b8, 0xe6be6ea4, 0x04b08528 }
};

int32_t EQ_GEQ_A_table[5][3] = {
    { 0x00a0d97b,    0x0074b75a,    0x006b3ba7 },
    { 0x01fc48d8,    0x0170d2c1,    0x0152db3b },
    { 0x06487ed5,    0x048f2987,    0x0430548e },
    { 0x13dad878,    0x0e683b8d,    0x0d3c9050 },
    { 0x3ed4f452,    0x2d979f4a,    0x29e34d8c }
};

int32_t EQ_GEQ_B_table[5][3] = {
    { 0x00000ca21, 0x000006a6d, 0x000059d5 },
    { 0x00007e262, 0x0000426bd, 0x0003810f },
    { 0x0004ef4f3, 0x0002992b7, 0x00231788 },
    { 0x003146e6e, 0x0019f21e7, 0x015e69f8 },
    { 0x01ed7aefb, 0x0103d4f9a, 0x0db53153 },
};

int32_t EQ_cos_table[] = {
    0x40000000, 0x40000000,
    0x40000000, 0x40000000, 0x40000000, 0x40000000,
    0x40000000, 0x40000000, 0x40000000, 0x40000000,
    0x40000000, 0x40000000, 0x40000000, 0x40000000,

    0x3ffffffe, 0x3ffffffb, 0x3fffffec, 0x3fffffb1,
    0x3ffffec4, 0x3ffffb10, 0x3fffec42, 0x3fffb10b,
    0x3ffec42d, 0x3ffb10c1, 0x3fec43c6, 0x3fb11b47,
    0x3ec52f9f, 0x3b20d79e, 0x2d413ccc, 0x00000000,
};
int32_t EQ_sin_table[] = {
    0x0000003, 0x0000006,
    0x0000000d, 0x00000019, 0x00000032, 0x00000065,
    0x000000c9, 0x00000192, 0x00000324, 0x00000649,
    0x00000c91, 0x00001922, 0x00003244, 0x00006488,

    0x0000c90f, 0x0001921f, 0x0003243f, 0x0006487e,
    0x000c90fd, 0x001921fa, 0x003243f1, 0x006487c3,
    0x00c90e8f, 0x0192155f, 0x0323ecbe, 0x0645e9af,
    0x0c7c5c1e, 0x187de2a6, 0x2d413ccc, 0x40000000,
};

int32_t EQ_pow_table_db_peq_7_by_10_int[] =
{0x0615417a,
    0x0621db42,
    0x062e8f24,
    0x063b5d56,
    0x0648460e,
    0x06554984,
    0x066267f0,
    0x066fa188,
    0x067cf685,
    0x068a6720,
    0x0697f391,
    0x06a59c14,
    0x06b360e2,
    0x06c14235,
    0x06cf4048,
    0x06dd5b58,
    0x06eb939f,
    0x06f9e95c,
    0x07085cca,
    0x0716ee27,
    0x07259db2,
    0x07346ba8,
    0x07435849,
    0x075263d4,
    0x07618e89,
    0x0770d8aa,
    0x07804276,
    0x078fcc30,
    0x079f7619,
    0x07af4075,
    0x07bf2b86,
    0x07cf3791,
    0x07df64d9,
    0x07efb3a4,
    0x08002436,
    0x0810b6d7,
    0x08216bcc,
    0x0832435c,
    0x08433dd0,
    0x08545b6f,
    0x08659c82,
    0x08770153,
    0x08888a2c,
    0x089a3757,
    0x08ac0920,
    0x08bdffd3,
    0x08d01bbb,
    0x08e25d27,
    0x08f4c463,
    0x090751be,
    0x091a0588,
    0x092ce00f,
    0x093fe1a4,
    0x09530a98,
    0x09665b3c,
    0x0979d3e3,
    0x098d74df,
    0x09a13e85,
    0x09b53127,
    0x09c94d1c,
    0x09dd92b8,
    0x09f20253,
    0x0a069c42,
    0x0a1b60df,
    0x0a305080,
    0x0a456b80,
    0x0a5ab237,
    0x0a702501,
    0x0a85c439,
    0x0a9b903c,
    0x0ab18964,
    0x0ac7b011,
    0x0ade04a1,
    0x0af48772,
    0x0b0b38e4,
    0x0b221959,
    0x0b392931,
    0x0b5068ce,
    0x0b67d894,
    0x0b7f78e6,
    0x0b974a29,
    0x0baf4cc2,
    0x0bc78118,
    0x0bdfe791,
    0x0bf88095,
    0x0c114c8d,
    0x0c2a4be3,
    0x0c437f00,
    0x0c5ce651,
    0x0c768241,
    0x0c90533e,
    0x0caa59b4,
    0x0cc49614,
    0x0cdf08cc,
    0x0cf9b24d,
    0x0d149309,
    0x0d2fab73,
    0x0d4afbfc,
    0x0d66851b,
    0x0d824743,
    0x0d9e42eb,
    0x0dba788b,
    0x0dd6e89a,
    0x0df39391,
    0x0e1079ea,
    0x0e2d9c21,
    0x0e4afab1,
    0x0e689617,
    0x0e866ed2,
    0x0ea48560,
    0x0ec2da42,
    0x0ee16df8,
    0x0f004105,
    0x0f1f53ec,
    0x0f3ea731,
    0x0f5e3b59,
    0x0f7e10ec,
    0x0f9e2870,
    0x0fbe826e,
    0x0fdf1f70,
    0x10000000,
    0x102124ab,
    0x10428dfd,
    0x10643c86,
    0x108630d3,
    0x10a86b76,
    0x10caed00,
    0x10edb605,
    0x1110c718,
    0x113420cf,
    0x1157c3bf,
    0x117bb082,
    0x119fe7af,
    0x11c469e1,
    0x11e937b3,
    0x120e51c2,
    0x1233b8ab,
    0x12596d0f,
    0x127f6f8d,
    0x12a5c0c7,
    0x12cc6161,
    0x12f351fe,
    0x131a9345,
    0x134225dd,
    0x136a0a6d,
    0x139241a1,
    0x13bacc22,
    0x13e3aa9e,
    0x140cddc3,
    0x1436663f,
    0x146044c4,
    0x148a7a04,
    0x14b506b3,
    0x14dfeb85,
    0x150b2932,
    0x1536c070,
    0x1562b1fb,
    0x158efe8c,
    0x15bba6e1,
    0x15e8abb8,
    0x16160dd0,
    0x1643cde9,
    0x1671ecc8,
    0x16a06b31,
    0x16cf49e8,
    0x16fe89b7,
    0x172e2b65,
    0x175e2fbe,
    0x178e978e,
    0x17bf63a3,
    0x17f094cd,
    0x18222bdc,
    0x185429a6,
    0x18868efd,
    0x18b95cb8,
    0x18ec93b0,
    0x192034bf,
    0x195440c1,
    0x1988b893,
    0x19bd9d14,
    0x19f2ef25,
    0x1a28afaa,
    0x1a5edf87,
    0x1a957fa3,
    0x1acc90e7,
    0x1b04143c,
    0x1b3c0a8f,
    0x1b7474cf,
    0x1bad53eb,
    0x1be6a8d6,
    0x1c207483,
    0x1c5ab7e9,
    0x1c9573ff,
    0x1cd0a9c0,
    0x1d0c5a27,
    0x1d488633,
    0x1d852ee3,
    0x1dc2553b,
    0x1dfffa3e,
    0x1e3e1ef2,
    0x1e7cc461,
    0x1ebbeb94,
    0x1efb9599,
    0x1f3bc37e,
    0x1f7c7656,
    0x1fbdaf32,
    0x1fff6f29,
    0x2041b753,
    0x208488c9,
    0x20c7e4a9,
    0x210bcc10,
    0x21504020,
    0x219541fd,
    0x21dad2cb,
    0x2220f3b4,
    0x2267a5e1,
    0x22aeea80,
    0x22f6c2bf,
    0x233f2fd2,
    0x238832eb,
    0x23d1cd42,
    0x241c0010,
    0x2466cc90,
    0x24b23402,
    0x24fe37a6,
    0x254ad8c0,
    0x25981896,
    0x25e5f870,
    0x2634799b,
    0x26839d63,
    0x26d3651b,
    0x2723d216,
    0x2774e5aa,
    0x27c6a130,
    0x28190604,
    0x286c1585,
    0x28bfd114,
    0x29143a16,
    0x296951f2,
    0x29bf1a12,
    0x2a1593e4};

int32_t EQ_pow_table_db_peq_int[] = 
{0x0404de62,
    0x0410c84d,
    0x041cd588,
    0x0429067e,
    0x04355b97,
    0x0441d53e,
    0x044e73e1,
    0x045b37ed,
    0x046821d0,
    0x047531fb,
    0x048268df,
    0x048fc6ef,
    0x049d4c9f,
    0x04aafa65,
    0x04b8d0b7,
    0x04c6d00e,
    0x04d4f8e4,
    0x04e34bb2,
    0x04f1c8f6,
    0x0500712e,
    0x050f44d9,
    0x051e4477,
    0x052d708b,
    0x053cc999,
    0x054c5026,
    0x055c04b8,
    0x056be7da,
    0x057bfa13,
    0x058c3bf1,
    0x059cadff,
    0x05ad50ce,
    0x05be24ed,
    0x05cf2aef,
    0x05e06368,
    0x05f1ceed,
    0x06036e15,
    0x0615417a,
    0x062749b6,
    0x06398767,
    0x064bfb2a,
    0x065ea5a0,
    0x0671876b,
    0x0684a12f,
    0x0697f391,
    0x06ab7f3b,
    0x06bf44d5,
    0x06d3450c,
    0x06e7808c,
    0x06fbf806,
    0x0710ac2c,
    0x07259db2,
    0x073acd4c,
    0x07503bb5,
    0x0765e9a4,
    0x077bd7d8,
    0x0792070e,
    0x07a87807,
    0x07bf2b86,
    0x07d62251,
    0x07ed5d2e,
    0x0804dce8,
    0x081ca24a,
    0x0834ae23,
    0x084d0145,
    0x08659c82,
    0x087e80b0,
    0x0897aea7,
    0x08b12742,
    0x08caeb5f,
    0x08e4fbdd,
    0x08ff599e,
    0x091a0588,
    0x09350082,
    0x09504b77,
    0x096be753,
    0x0987d507,
    0x09a41585,
    0x09c0a9c3,
    0x09dd92b8,
    0x09fad161,
    0x0a1866ba,
    0x0a3653c6,
    0x0a549987,
    0x0a733906,
    0x0a92334b,
    0x0ab18964,
    0x0ad13c62,
    0x0af14d57,
    0x0b11bd5a,
    0x0b328d85,
    0x0b53bef5,
    0x0b7552cb,
    0x0b974a29,
    0x0bb9a638,
    0x0bdc6821,
    0x0bff9112,
    0x0c23223d,
    0x0c471cd7,
    0x0c6b8219,
    0x0c90533e,
    0x0cb59186,
    0x0cdb3e35,
    0x0d015a92,
    0x0d27e7e9,
    0x0d4ee787,
    0x0d765ac1,
    0x0d9e42eb,
    0x0dc6a162,
    0x0def7784,
    0x0e18c6b4,
    0x0e429058,
    0x0e6cd5db,
    0x0e9798ae,
    0x0ec2da42,
    0x0eee9c10,
    0x0f1adf94,
    0x0f47a64e,
    0x0f74f1c4,
    0x0fa2c37f,
    0x0fd11d0d,
    0x10000000,
    0x102f6df0,
    0x105f6879,
    0x108ff13c,
    0x10c109dd,
    0x10f2b409,
    0x1124f16d,
    0x1157c3bf,
    0x118b2cb9,
    0x11bf2e17,
    0x11f3c99f,
    0x1229011a,
    0x125ed655,
    0x12954b24,
    0x12cc6161,
    0x13041ae9,
    0x133c79a2,
    0x13757f74,
    0x13af2e4e,
    0x13e98827,
    0x14248ef9,
    0x146044c4,
    0x149cab90,
    0x14d9c568,
    0x15179461,
    0x15561a92,
    0x15955a1b,
    0x15d55521,
    0x16160dd0,
    0x1657865a,
    0x1699c0f8,
    0x16dcbfe9,
    0x17208574,
    0x176513e5,
    0x17aa6d8f,
    0x17f094cd,
    0x18378c00,
    0x187f5591,
    0x18c7f3ef,
    0x19116992,
    0x195bb8f7,
    0x19a6e4a4,
    0x19f2ef25,
    0x1a3fdb10,
    0x1a8dab00,
    0x1adc619a,
    0x1b2c0189,
    0x1b7c8d81,
    0x1bce083e,
    0x1c207483,
    0x1c73d51c,
    0x1cc82cde,
    0x1d1d7ea6,
    0x1d73cd58,
    0x1dcb1be2,
    0x1e236d3a,
    0x1e7cc461,
    0x1ed7245d,
    0x1f329040,
    0x1f8f0b25,
    0x1fec982d,
    0x204b3a87,
    0x20aaf568,
    0x210bcc10,
    0x216dc1c8,
    0x21d0d9e2,
    0x223517bc,
    0x229a7ebd,
    0x23011255,
    0x2368d600,
    0x23d1cd42,
    0x243bfbab,
    0x24a764d7,
    0x25140c6a,
    0x2581f613,
    0x25f1258e,
    0x26619ea0,
    0x26d3651b,
    0x27467cdb,
    0x27bae9c7,
    0x2830afd4,
    0x28a7d2ff,
    0x29205754,
    0x299a40ea,
    0x2a1593e4,
    0x2a925471,
    0x2b1086cc,
    0x2b902f3f,
    0x2c11521e,
    0x2c93f3ca,
    0x2d1818b3,
    0x2d9dc555,
    0x2e24fe38,
    0x2eadc7f3,
    0x2f38272b,
    0x2fc42091,
    0x3051b8e5,
    0x30e0f4f6,
    0x3171d9a0,
    0x32046bcc,
    0x3298b076,
    0x332eaca3,
    0x33c6656d,
    0x345fdff7,
    0x34fb2178,
    0x35982f34,
    0x36370e80,
    0x36d7c4bf,
    0x377a5766,
    0x381ecbf8,
    0x38c5280b,
    0x396d7144,
    0x3a17ad58,
    0x3ac3e20e,
    0x3b72153e,
    0x3c224cd2,
    0x3cd48ec4,
    0x3d88e120,
    0x3e3f4a05,
    0x3ef7cfa4,
    0x3fb2783f};

int32_t EQ_generate_equaliser_coefs_isr( int32_t fs_int, int32_t req_db_int[5], int32_t b0_int[5], int32_t b1_int[5], int32_t b2_int[5], int32_t a1_int[5], int32_t a2_int[5])
{
    int32_t req_f[5];
    int32_t mag[5];
    int i,j;
    int32_t ret_val = 0;
    int32_t fac, fs_index;

	{
		int32_t max = -100;
		int32_t min = 100;
		int32_t flag = 0, diff;
		for (i=0;i<5;i++)
		{
			if (max < req_db_int[i]) max = req_db_int[i];
			if (min > req_db_int[i]) min = req_db_int[i];
		}
		if (max < -10)
			flag = 1;
		if (min > 10)
			flag = 1;
		if (flag == 1)
		{
			if (max < 0)
			{
				int32_t temp = max;
				max = -min;
				min = -temp;
			}

			if (2 * min >= max)
			{
				diff = (max - min) * 32768;
				diff = diff / max;
				diff = 16384 - diff;

				for (i=0;i<5;i++)
				{
					fac = req_db_int[i] * diff;
					fac = fac / 16384;
					fac = fac / 7;		/* 15% compensation */
					req_db_int[i] = req_db_int[i] - fac;
				}
			}
		}
	}

    fac = 1;
    fs_index = -1;
    while(fs_int)
    {
        if (fs_int == 32000) {
            fs_index = 0;
        } else if (fs_int == 44100) {
            fs_index = 1;
        } else if (fs_int == 48000) {
            fs_index = 2;
        }
        if (fs_index != -1)
            break;
        fac = fac << 1;
        fs_int = fs_int >> 1;
    }
    fs_int = fs_int * fac;

    /* Validate the sample rate */
    if (fs_index == -1)
    {
        ret_val = -1;
    }
    /* Validate the dB levels */
    for (i=0;i<5;i++)
    {
        if (req_db_int[i] > 120)
            ret_val = -1;
        if (req_db_int[i] < -120)
            ret_val = -1;
    }

    if (ret_val == 0)
    {
        for (i=0;i<5;i++)
        {
            req_db_int[i] = req_db_int[i] * 32768;
            req_db_int[i] = req_db_int[i] / 10;
        }
        for (i=0;i<5;i++)
        {
            req_f[i] = req_db_int[i];
            for(j=0;j<5;j++)
            {
                req_f[i] += EQ_mul_32_32 (EQ_matr_int[i][j], req_db_int[j]);
            }
        }
        /* Print the power table */
        for (i=0;i<5;i++)
        {
            int32_t q_inv = 0x48ba2e8b; /* 1/0.22 in 4.28 format */
            int32_t aa1, aa2, A1, A2, B1, B2, div;
            int32_t num;
			if (req_f[i] == 0)
			{
				a1_int[i] = 0x0;
				a2_int[i] = 0x0;
				b0_int[i] = 0x4000000 << 3;
				b1_int[i] = 0x0;
				b2_int[i] = 0x0;
				continue;
			}
            if (req_f[i] < 0)
                mag[i] = EQ_db_2_linear(-req_f[i], EQ_pow_table_db_int_neg);
            else
                mag[i] = EQ_db_2_linear(req_f[i], EQ_pow_table_db_int_neg);

            /* main for loop for generating coefs */
            if (req_f[i] < 0)
            {
                aa2 = q_inv;
                aa1 = EQ_mul_32_32 (mag[i], q_inv) << 3;
            }
            else
            {
                aa1 = q_inv;
                aa2 = EQ_mul_32_32 (mag[i], q_inv) << 3;
            }

            A1 = EQ_mul_32_32 (aa1, EQ_GEQ_A_table[i][fs_index]) / fac;
            A2 = EQ_mul_32_32 (aa2, EQ_GEQ_A_table[i][fs_index]) / fac;

            B1 = EQ_GEQ_B_table[i][fs_index] / 2;
            B2 = EQ_GEQ_B_table[i][fs_index] / 2;

            B1 = B1 / fac / fac;
            B2 = B2 / fac / fac;

            div = 0x10000000 + A2 + B2;

            num = -(2*B2 - 0x20000000);
            a1_int[i] = EQ_div_32_32 (num>>1 , div);

            num = -(0x10000000 + B2 - A2);
            a2_int[i] = EQ_div_32_32 (num>>1 , div);

            num = (0x10000000 + B1 + A1);
            b0_int[i] = EQ_div_32_32 (num>>1 , div);

            num = (2*B1 - 0x20000000);
            b1_int[i] = EQ_div_32_32 (num>>1 , div);

            num = (0x10000000 + B1 - A1);
            b2_int[i] = EQ_div_32_32 (num>>1 , div);
        }

        for (i=0;i<5;i++)
        {
            a1_int[i] = a1_int[i] >> 3;
            a2_int[i] = a2_int[i] >> 3;
            b0_int[i] = b0_int[i] >> 3;
            b1_int[i] = b1_int[i] >> 3;
            b2_int[i] = b2_int[i] >> 3;
        }
    }
    else
    {
        /* Disable all IIR filters (bypass) */
        for (i=0;i<5;i++)
        {
            a1_int[i] = 0;
            a2_int[i] = 0;
            b0_int[i] = 0x04000000;
            b1_int[i] = 0;
            b2_int[i] = 0;
        }
    }
    return ret_val;
}

void EQ_int2fix(int32_t inp, int32_t *mant, int32_t *exp)
{
    int32_t sign;
    int32_t i;

    sign = 1;
    if (inp < 0)
    {
        inp = -inp;
        sign = -1;
    }
    /* Normalize the input */
    i = 0;
    if (inp != 0)
    {
        while ((inp & 0xc0000000) == 0)
        {
            inp = inp << 1;
            i++;
        }
    }
    *mant = inp * sign;
    *exp = -i;
    return;
}

void EQ_add_32_32 (int32_t *dest_mant, int32_t *dest_exp, int32_t src_mant, int32_t src_exp)
{
    int32_t ret = 0;
    if (*dest_mant == 0)
    {
        *dest_mant = src_mant;
        *dest_exp = src_exp;
        ret = 1;
    }
    if (src_mant == 0)
    {
        /* Nothing to do */
        ret = 1;
    }
    if (ret == 0)
{
        /* Reduce the exp by one to avoid overflow */
        *dest_mant = *dest_mant >> 1;
        *dest_exp = *dest_exp + 1;
        src_mant = src_mant >> 1;
        src_exp = src_exp + 1;
        if (src_exp != *dest_exp)
        {
            if (src_exp > *dest_exp)
            {
                if (src_exp > *dest_exp + 31)
	{
                    *dest_mant = 0;
                    *dest_exp = src_exp;
	}
                else
	{
                    *dest_mant = (*dest_mant) >> (src_exp - (*dest_exp));
                    *dest_exp = src_exp;
                }
	}
            else
            {
                if (src_exp+31 < *dest_exp)
    {
                    src_mant = 0;
                    src_exp = *dest_exp;
    }
    else
    {
                    src_mant = (src_mant) >> (*dest_exp - (src_exp));
                    src_exp = *dest_exp;
                }
            }
        }
        *dest_mant = *dest_mant + src_mant;
    }
    return;
    }

void EQ_div (int32_t *dest_mant, int32_t *dest_exp, int32_t mant, int32_t exp, int32_t src_mant, int32_t src_exp)
{
    EQ_renormalise (&mant, &exp);
    EQ_renormalise (&src_mant, &src_exp);
    *dest_mant = EQ_div_32_32 (mant>>1, src_mant);
    *dest_exp = exp - src_exp - 30 + 1;
    return;
}

void EQ_mul (int32_t *dest_mant, int32_t *dest_exp, int32_t mant, int32_t exp, int32_t src_mant, int32_t src_exp)
{
    EQ_renormalise (&mant, &exp);
    EQ_renormalise (&src_mant, &src_exp);
    *dest_mant = EQ_mul_32_32 (mant, src_mant);
    *dest_exp = exp + src_exp + 31;
    return;
}

void EQ_add (int32_t *dest_mant, int32_t *dest_exp, int32_t mant, int32_t exp, int32_t src_mant, int32_t src_exp)
{
    EQ_renormalise (&mant, &exp);
    EQ_renormalise (&src_mant, &src_exp);
    EQ_add_32_32 (&mant, &exp, src_mant, src_exp);
    *dest_exp = exp;
    *dest_mant = mant;
    return;
	}

void EQ_sqrt (int32_t *dest_mant, int32_t *dest_exp, int32_t src_mant, int32_t src_exp)
{
    EQ_renormalise (&src_mant, &src_exp);
    if ((src_exp&1) == 0)
    {
        src_mant = src_mant >> 1;
        src_exp = src_exp + 1;
    }
    *dest_mant = EQ_sqrt_32 (src_mant);
    *dest_exp = (src_exp+1)/2 - 16;
    return;
}

void EQ_tan(int32_t f1, int32_t f2, int32_t *fc_a_mant, int32_t *fc_a_exp)
{
    int32_t phase;
    int32_t cos_val, sin_val;
    int32_t sin_mant, sin_exp;
    int32_t cos_mant, cos_exp;
    int32_t f1_exp = 0, f2_exp = 0;
    if (f1 > f2/2)
    {
        *fc_a_mant = 0;
        *fc_a_exp = 0;
    }

    EQ_renormalise (&f1, &f1_exp);
    EQ_renormalise (&f2, &f2_exp);
    phase = EQ_div_32_32(f1>>1, f2);
    phase = phase >> (f2_exp - f1_exp - 1);
    EQ_cos_sin (phase, &cos_val, &sin_val);

    EQ_int2fix (sin_val, &sin_mant, &sin_exp);
    EQ_int2fix (cos_val, &cos_mant, &cos_exp);
    EQ_div (fc_a_mant, fc_a_exp, sin_mant, sin_exp, cos_mant, cos_exp);
    return;
}

/******************************************************************************
  *
  *	FUNCTION NAME:	EQ_generate_peq_coefs_isr
  *
  *	Description:
  *			This function takes the user defined level for parametric equaliser
  *		and generates coefs required for 3548 hardware
  *			This function generated only one biquad. For multiple band this
  *		function shall be called multiple times. For eg. to generate coef. for
  *		7-band equalizer, the function shall be called 7-times. Each function
  *		call shall give one set of parameters.
  *
  *         The PEQ coefs generated by this algorithm is not representable in
  *     3548 HW IIR filter. To accomodate this filter, the coefs are adjusted
  *     such that the audio samples are divided by 2. To compensate this effect
  *     a separate multiply by 2 shall be applied for individual biquad filter.
  *
  *	Input:	
  *		
  *		q_int		- 'Q' value of the equalizer
  *						'Q' = fc/bandwidth. The value q_int = Q * 100. This
  *						allows 'Q' to be represented in +/- 0.01 resolution.
  *		fc_int	    - center frequency in Hz
  *		
  *		dbval_int	- The amount of Gain/attenuation required to be applied.
  *                        The supported range is +/- 12dB in steps of 0.1 dB
  *                        To set 8.3 dB, the dbval_int shall be 83.
  *		fs			- Sampling rate at which the equalizer runs
  *						Tested for 32000, 44100, 48000
  *
  *	Output:
  *		b_int, a_int
  *			- Biquad filter coefs.
  *	Return:
  *		0 - On successful generation of coefs.
  *		Non-zero - In case of input values out of range
  * Note:
  *		This function shall support the dB ranges from -12dB to +12dB.
  *
  ******************************************************************************/

int32_t EQ_generate_one_peq_band ( int32_t q_int, int32_t fc_int, int32_t dbval_int, int32_t fs_int, int32_t b_int[], int32_t a_int[])
{
    int32_t bypass = 0;
    int32_t g_mant, g_exp;
    int32_t gb_mant, gb_exp;
    int32_t fc_mant, fc_exp;
    int32_t fs_mant, fs_exp;
    int32_t q_mant, q_exp;
    int32_t t1_mant, t1_exp;
    int32_t t2_mant, t2_exp;
    int32_t f1_mant, f1_exp;
    int32_t fc_a_mant, fc_a_exp;
    int32_t f1_a_mant, f1_a_exp;
    int32_t g0_mant, g0_exp;
    int32_t w0_mant, w0_exp;
    int32_t dw_mant, dw_exp;
    int32_t p_mant, p_exp;
    int32_t temp1_mant, temp1_exp;
    int32_t temp2_mant, temp2_exp;
    int32_t t3_mant, t3_exp;
    int32_t t4_mant, t4_exp;
    int32_t g1_mant, g1_exp;
    int32_t X_mant, X_exp;
    int32_t v1_mant, v1_exp;
    int32_t v2_mant, v2_exp;
    int32_t B_mant, B_exp;
    int32_t A_mant, A_exp;
    int32_t a0_mant, a0_exp;
    int32_t a1_mant, a1_exp;
    int32_t a2_mant, a2_exp;
    int32_t b0_mant, b0_exp;
    int32_t b1_mant, b1_exp;
    int32_t b2_mant, b2_exp;

    bypass = 0;
    if (fc_int > 20000)
        bypass = 1;

    if (dbval_int == 0)
        bypass = 1;

    if (bypass != 0)
	{
        b_int[0] = 0x02000000;
        b_int[1] = b_int[2] = a_int[1] = a_int[2] = 0;
        return 0;
	}

    /* Limit the Q value */
    if (q_int > 820)
        q_int = 820;
    if (q_int < 33)
        q_int = 33;

    g_exp = -28;
    gb_exp = -28;
    g_mant = EQ_pow_table_db_peq_int[dbval_int + 120];
    gb_mant = EQ_pow_table_db_peq_7_by_10_int[dbval_int + 120];

    EQ_int2fix(fc_int, &fc_mant, &fc_exp);
    EQ_int2fix(fs_int, &fs_mant, &fs_exp);
    EQ_int2fix(q_int, &q_mant, &q_exp);

    EQ_int2fix(100, &t1_mant, &t1_exp);
    EQ_div (&q_mant, &q_exp, q_mant, q_exp, t1_mant, t1_exp);

    EQ_int2fix (1, &t1_mant, &t1_exp);
    EQ_div (&t1_mant, &t1_exp, t1_mant, t1_exp, q_mant, q_exp);
    EQ_mul (&t2_mant, &t2_exp, t1_mant, t1_exp, t1_mant, t1_exp);
    EQ_int2fix(4, &temp1_mant, &temp1_exp);
    EQ_add (&t2_mant, &t2_exp, temp1_mant, temp1_exp, t2_mant, t2_exp);

    EQ_sqrt(&t2_mant, &t2_exp, t2_mant, t2_exp);
    EQ_add (&f1_mant, &f1_exp, t2_mant, t2_exp-1, -t1_mant, t1_exp-1);
    EQ_mul (&f1_mant, &f1_exp, f1_mant, f1_exp, fc_mant, fc_exp);

    EQ_tan (fc_int, fs_int, &fc_a_mant, &fc_a_exp);
    EQ_tan (f1_mant >> (-f1_exp-14), fs_int<<14, &f1_a_mant, &f1_a_exp);

    p_mant = 0x6487ED51;
    p_exp = -29;
    EQ_mul (&fc_a_mant, &fc_a_exp, fc_a_mant, fc_a_exp, fs_mant, fs_exp);
    EQ_mul (&f1_a_mant, &f1_a_exp, f1_a_mant, f1_a_exp, fs_mant, fs_exp);
    EQ_div (&fc_a_mant, &fc_a_exp, fc_a_mant, fc_a_exp, p_mant, p_exp);
    EQ_div (&f1_a_mant, &f1_a_exp, f1_a_mant, f1_a_exp, p_mant, p_exp);

    EQ_div (&w0_mant, &w0_exp, fc_mant, fc_exp, fs_mant, fs_exp);
    EQ_mul (&w0_mant, &w0_exp, w0_mant, w0_exp, p_mant, p_exp+1);
    EQ_div (&dw_mant, &dw_exp, w0_mant, w0_exp, q_mant, q_exp);

    p_mant = 0x5FD18E31;
    p_exp = -29;
    EQ_int2fix(1, &g0_mant, &g0_exp);

    EQ_mul (&t1_mant, &t1_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_mul (&temp1_mant, &temp1_exp, gb_mant, gb_exp, gb_mant, gb_exp);
    EQ_add (&t1_mant, &t1_exp, t1_mant, t1_exp, -temp1_mant, temp1_exp);

    EQ_mul (&t2_mant, &t2_exp, gb_mant, gb_exp, gb_mant, gb_exp);
    EQ_mul (&temp1_mant, &temp1_exp, g0_mant, g0_exp, g0_mant, g0_exp);
    EQ_add (&t2_mant, &t2_exp, t2_mant, t2_exp, -temp1_mant, temp1_exp);

    EQ_mul (&t3_mant, &t3_exp, w0_mant, w0_exp, w0_mant, w0_exp);
    EQ_mul (&temp1_mant, &temp1_exp, p_mant, p_exp, p_mant, p_exp);
    EQ_add (&t3_mant, &t3_exp, t3_mant, t3_exp, -temp1_mant, temp1_exp);
    EQ_mul (&t3_mant, &t3_exp, t3_mant, t3_exp, g0_mant, g0_exp);
    EQ_mul (&t3_mant, &t3_exp, t3_mant, t3_exp, t3_mant, t3_exp);
    EQ_mul (&temp1_mant, &temp1_exp, g_mant, g_exp, p_mant, p_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, dw_mant, dw_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, temp1_mant, temp1_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, t2_mant, t2_exp);
    EQ_div (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, t1_mant, t1_exp);
    EQ_add (&t3_mant, &t3_exp, t3_mant, t3_exp, temp1_mant, temp1_exp);

    EQ_mul (&t4_mant, &t4_exp, w0_mant, w0_exp, w0_mant, w0_exp);
    EQ_mul (&temp1_mant, &temp1_exp, p_mant, p_exp, p_mant, p_exp);
    EQ_add (&t4_mant, &t4_exp, t4_mant, t4_exp, -temp1_mant, temp1_exp);
    EQ_mul (&t4_mant, &t4_exp, t4_mant, t4_exp, t4_mant, t4_exp);
    EQ_mul (&temp1_mant, &temp1_exp, dw_mant, dw_exp, p_mant, p_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, temp1_mant, temp1_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, t2_mant, t2_exp);
    EQ_div (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, t1_mant, t1_exp);
    EQ_add (&t4_mant, &t4_exp, t4_mant, t4_exp, temp1_mant, temp1_exp);

    EQ_div (&g1_mant, &g1_exp, t3_mant, t3_exp, t4_mant, t4_exp);
    EQ_sqrt(&g1_mant, &g1_exp, g1_mant, g1_exp);

    EQ_mul (&X_mant, &X_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_mul (&temp1_mant, &temp1_exp, g1_mant, g1_exp, g1_mant, g1_exp);
    EQ_add (&X_mant, &X_exp, X_mant, X_exp, -temp1_mant, temp1_exp);
    EQ_mul (&temp1_mant, &temp1_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_mul (&temp2_mant, &temp2_exp, g0_mant, g0_exp, g0_mant, g0_exp);
    EQ_add (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, -temp2_mant, temp2_exp);
    EQ_div (&X_mant, &X_exp, X_mant, X_exp, temp1_mant, temp1_exp);
    EQ_sqrt (&X_mant, &X_exp, X_mant, X_exp);

    EQ_mul (&t1_mant, &t1_exp, fc_a_mant, fc_a_exp, fc_a_mant, fc_a_exp);
    EQ_mul (&X_mant, &X_exp, X_mant, X_exp, t1_mant, t1_exp);

    EQ_add (&t2_mant, &t2_exp, t1_mant, t1_exp, -X_mant, X_exp);
    EQ_mul (&t2_mant, &t2_exp, t2_mant, t2_exp, t2_mant, t2_exp);

    EQ_mul (&temp1_mant, &temp1_exp, g1_mant, g1_exp, t1_mant, t1_exp);
    EQ_mul (&t3_mant, &t3_exp, X_mant, X_exp, g0_mant, g0_exp);
    EQ_add (&t3_mant, &t3_exp, temp1_mant, temp1_exp, -t3_mant, t3_exp);
    EQ_mul (&t3_mant, &t3_exp, t3_mant, t3_exp, t3_mant, t3_exp);

    EQ_mul (&t4_mant, &t4_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_mul (&t4_mant, &t4_exp, t2_mant, t2_exp, t4_mant, t4_exp);
    EQ_add (&t4_mant, &t4_exp, t4_mant, t4_exp, -t3_mant, t3_exp);
    EQ_div (&v1_mant, &v1_exp, t4_mant, t4_exp, t1_mant, t1_exp);

    EQ_mul (&t1_mant, &t1_exp, f1_a_mant, f1_a_exp, f1_a_mant, f1_a_exp);
    EQ_add (&t2_mant, &t2_exp, t1_mant, t1_exp, -X_mant, X_exp);
    EQ_mul (&t2_mant, &t2_exp, t2_mant, t2_exp, t2_mant, t2_exp);

    EQ_mul (&temp1_mant, &temp1_exp, g1_mant, g1_exp, t1_mant, t1_exp);
    EQ_mul (&t3_mant, &t3_exp, X_mant, X_exp, g0_mant, g0_exp);
    EQ_add (&t3_mant, &t3_exp, temp1_mant, temp1_exp, -t3_mant, t3_exp);
    EQ_mul (&t3_mant, &t3_exp, t3_mant, t3_exp, t3_mant, t3_exp);

    EQ_mul (&t4_mant, &t4_exp, gb_mant, gb_exp, gb_mant, gb_exp);
    EQ_mul (&t4_mant, &t4_exp, t2_mant, t2_exp, t4_mant, t4_exp);
    EQ_add (&t4_mant, &t4_exp, t4_mant, t4_exp, -t3_mant, t3_exp);
    EQ_div (&v2_mant, &v2_exp, t4_mant, t4_exp, t1_mant, t1_exp);

    EQ_mul (&temp1_mant, &temp1_exp, gb_mant, gb_exp, gb_mant, gb_exp);
    EQ_mul (&temp2_mant, &temp2_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_add (&temp1_mant, &temp1_exp, temp2_mant, temp2_exp, -temp1_mant, temp1_exp);
    EQ_add (&t1_mant, &t1_exp, v2_mant, v2_exp, -v1_mant, v1_exp);
    EQ_div (&t1_mant, &t1_exp, t1_mant, t1_exp, temp1_mant, temp1_exp);

    EQ_sqrt (&A_mant, &A_exp, t1_mant, t1_exp);

    EQ_mul (&B_mant, &B_exp, A_mant, A_exp, A_mant, A_exp);
    EQ_mul (&B_mant, &B_exp, B_mant, B_exp, g_mant, g_exp);
    EQ_mul (&B_mant, &B_exp, B_mant, B_exp, g_mant, g_exp);
    EQ_add (&B_mant, &B_exp, B_mant, B_exp, v1_mant, v1_exp);
    EQ_sqrt (&B_mant, &B_exp, B_mant, B_exp);

    p_mant = 0x6487ED51;
    p_exp = -29;
    EQ_div (&fs_mant, &fs_exp, fs_mant, fs_exp, p_mant, p_exp);
    EQ_div (&X_mant, &X_exp, X_mant, X_exp, fs_mant, fs_exp);
    EQ_div (&X_mant, &X_exp, X_mant, X_exp, fs_mant, fs_exp);
    EQ_div (&A_mant, &A_exp, A_mant, A_exp, fs_mant, fs_exp);
    EQ_div (&B_mant, &B_exp, B_mant, B_exp, fs_mant, fs_exp);

    EQ_int2fix(1, &t1_mant, &t1_exp);
    EQ_add (&t2_mant, &t2_exp, t1_mant, t1_exp, X_mant, X_exp);

    EQ_add (&a0_mant, &a0_exp, t2_mant, t2_exp, A_mant, A_exp);
    EQ_add (&a1_mant, &a1_exp, -t1_mant, t1_exp+1, X_mant, X_exp+1);
    EQ_add (&a2_mant, &a2_exp, t2_mant, t2_exp, -A_mant, A_exp);

    EQ_div (&a2_mant, &a2_exp, a2_mant, a2_exp, a0_mant, a0_exp);
    EQ_div (&a1_mant, &a1_exp, a1_mant, a1_exp, a0_mant, a0_exp);

    EQ_mul (&t1_mant, &t1_exp, g0_mant, g0_exp, X_mant, X_exp);
    EQ_add (&t2_mant, &t2_exp, t1_mant, t1_exp, g1_mant, g1_exp);

    EQ_add (&b0_mant, &b0_exp, t2_mant, t2_exp, B_mant, B_exp);
    EQ_add (&b1_mant, &b1_exp, t1_mant, t1_exp+1, -g1_mant, g1_exp+1);
    EQ_add (&b2_mant, &b2_exp, t2_mant, t2_exp, -B_mant, B_exp);

    EQ_div (&b0_mant, &b0_exp, b0_mant, b0_exp, a0_mant, a0_exp);
    EQ_div (&b1_mant, &b1_exp, b1_mant, b1_exp, a0_mant, a0_exp);
    EQ_div (&b2_mant, &b2_exp, b2_mant, b2_exp, a0_mant, a0_exp);

    b0_exp -= 1;
    b1_exp -= 1;
    b2_exp -= 1;

    b_int[0] = b0_mant >> (-b0_exp - 26);
    b_int[1] = b1_mant >> (-b1_exp - 26);
    b_int[2] = b2_mant >> (-b2_exp - 26);

    a_int[0] = a0_mant >> (-a0_exp - 26);
    a_int[1] = a1_mant >> (-a1_exp - 26);
    a_int[2] = a2_mant >> (-a2_exp - 26);

#if 0
    b_int[0] = b_int[0] << 1;
    b_int[1] = b_int[1] << 1;
    b_int[2] = b_int[2] << 1;
#endif

    a_int[1] = -a_int[1];
    a_int[2] = -a_int[2];

    return 0;
}

int32_t EQ_generate_peq_coefs_isr ( unsigned int q[], unsigned int freq[], int gain[], 
    uint32_t fs, int32_t b0[], int32_t b1[], 
    int32_t b2[], int32_t a1[], int32_t a2[])
{
	int32_t b_int[3], a_int[3];
	int32_t i, phase, l_freq;
	int32_t error_count, freq_error = 0, error=0;
	int32_t c1, c2, s1, s2, amp_num, amp_den;
	int32_t exp_num, exp_den;
	int32_t ar_mant, ar_exp, ai_mant, ai_exp, t1_mant, t1_exp;
                
	for (i=0;i<BRAP_MAX_PEQ_FILTERS ;i++)
	{
		EQ_generate_one_peq_band ( (int32_t)q[i], (int32_t)freq[i], 
          (int32_t)gain[i], (int32_t)fs, b_int, a_int);
		b0[i] = b_int[0];
		b1[i] = b_int[1];
		b2[i] = b_int[2];
		a1[i] = a_int[1];
		a2[i] = a_int[2];
	}
    
	/* Testing reliablity of PEQ */
	{
		l_freq = 20 * 256;
		phase = EQ_div_32_32 (20 << 9, fs << 8);
		error_count = 0;
		do {
			EQ_cos_sin (phase, &c1, &s1);
			c2 = EQ_mul_32_32 (c1, c1);
			s2 = EQ_mul_32_32 (s1, s1);
			c2 = 2*(c2 - s2);
			s2 = EQ_mul_32_32 (c1, s1);
			s2 = 4 * s2;
			amp_num = 0x40000000;
			exp_num = 0;
			amp_den = 0x40000000;
			exp_den = 0;
			for (i=0;i<BRAP_MAX_PEQ_FILTERS;i++)
			{
				EQ_mul (&ar_mant, &ar_exp, -a1[i], -26, c1, -30);
				EQ_mul (&t1_mant, &t1_exp, -a2[i], -26, c2, -30);
				EQ_add (&ar_mant, &ar_exp, ar_mant, ar_exp, t1_mant, t1_exp);
				EQ_add (&ar_mant, &ar_exp, ar_mant, ar_exp, 0x4000000, -26);

				EQ_mul (&ai_mant, &ai_exp, a1[i], -26, s1, -30);
				EQ_mul (&t1_mant, &t1_exp, a2[i], -26, s2, -30);
				EQ_add (&ai_mant, &ai_exp, ai_mant, ai_exp, t1_mant, t1_exp);

				EQ_mul (&ar_mant, &ar_exp, ar_mant, ar_exp, ar_mant, ar_exp);
				EQ_mul (&ai_mant, &ai_exp, ai_mant, ai_exp, ai_mant, ai_exp);

				EQ_add (&ar_mant, &ar_exp, ar_mant, ar_exp, ai_mant, ai_exp);
				EQ_mul (&amp_den, &exp_den, amp_den, exp_den, ar_mant, ar_exp);


				EQ_mul (&ar_mant, &ar_exp, b1[i], -25, c1, -30);
				EQ_mul (&t1_mant, &t1_exp, b2[i], -25, c2, -30);
				EQ_add (&ar_mant, &ar_exp, ar_mant, ar_exp, t1_mant, t1_exp);
				EQ_add (&ar_mant, &ar_exp, ar_mant, ar_exp, b0[i], -25);

				EQ_mul (&ai_mant, &ai_exp, b1[i], -25, s1, -30);
				EQ_mul (&t1_mant, &t1_exp, b2[i], -25, s2, -30);
				EQ_add (&ai_mant, &ai_exp, ai_mant, ai_exp, t1_mant, t1_exp);

				EQ_mul (&ar_mant, &ar_exp, ar_mant, ar_exp, ar_mant, ar_exp);
				EQ_mul (&ai_mant, &ai_exp, ai_mant, ai_exp, ai_mant, ai_exp);

				EQ_add (&ar_mant, &ar_exp, ar_mant, ar_exp, ai_mant, ai_exp);
				EQ_mul (&amp_num, &exp_num, amp_num, exp_num, ar_mant, ar_exp);

			}
			EQ_div (&amp_num, &exp_num, amp_num, exp_num, amp_den, exp_den);
			if (exp_num >= -23)
			{
			    error = 1;
				if (error_count == 0)
				{
					freq_error = l_freq/256;
				}
				error_count++;
			}
			else if (error_count != 0)
			{
				if (error_count == 1)
			    {
		    		BDBG_WRN(("PEQ_Coeffs: Gain saturation happening around %d HzS", freq_error));
    			}
				if (error_count > 1)
		    	{
	    			BDBG_WRN(("PEQ_Coeffs: Gain saturation happening in the range %d-%d Hz", 
                        freq_error, l_freq/256));
				}
                error_count = 0;         
			}
            
			phase = phase + (phase >> 4);
			l_freq = l_freq + (l_freq >> 4);
		} while( phase < 0x3fffffff);
	}
    
    if (error != 0)
    {
        for (i=0;i<BRAP_MAX_PEQ_FILTERS;i++)
        {
            b0[i] = 0x2000000;
            b1[i] = b2[i] = a1[i] = a2[i] = 0;
        }
        if (error_count != 0)
        {
    		BDBG_WRN(("PEQ_Coeffs: Gain saturation happening around %d Hz", freq_error));
        }
        BDBG_WRN(("Forcing PEQ to bypass. Reconfigure PEQ with valid settings!!!\n"));        
    }
	return 0;
}

/*----------- End of PEQ Coeff Generation ---------------- */

int32_t EQ_generate_low_shelving_isr (int32_t dbval_int, int32_t fs_int, int32_t b_int[], int32_t a_int[])
{
	int32_t ret_val;
    int32_t phase_int;
    int32_t f1_int, f2_int, fac_int;
    int32_t t1, t2, div_int;

    if (dbval_int == 0)
    {
        b_int[0] = 0x04000000;
        b_int[1] = 0x00000000;
        b_int[2] = 0x00000000;
        a_int[1] = 0x00000000;
        a_int[2] = 0x00000000;
        return 0;
    }

    dbval_int = (dbval_int * 32768) / 10;

	ret_val = 0;
    /* Low-Shelving filter */
    phase_int = EQ_div_32_32 (3141, fs_int) >> 1;	/* fc = 1000 Hz => sigma = pi * fc / fs  */

    if (dbval_int < 0)
        fac_int = EQ_db_2_linear(-dbval_int/2, EQ_pow_table_db_int_neg);
    else
        fac_int = EQ_db_2_linear(dbval_int/2, EQ_pow_table_db_int);

	if (dbval_int < 0)
	{
        f1_int = phase_int >> 4;
        f2_int = EQ_div_32_32 (phase_int, fac_int) >> 6;
	}
	else
	{
        f2_int = phase_int >> 4;
        f1_int = EQ_mul_32_32 (phase_int, fac_int) >> 1;
	}

    t1 = EQ_mul_32_32 (f2_int, f2_int);
    t2 = EQ_mul_32_32 (f2_int, EQ_SQRT_2_IN_5_27);
    div_int = 0x00800000 + t1 + t2;

    t1 = EQ_mul_32_32 (f1_int, f1_int);
    t2 = EQ_mul_32_32 (f1_int, EQ_SQRT_2_IN_5_27);
    t1 = 0x00800000 + t1 + t2;
    b_int[0] = EQ_div_32_32 (t1 >> 4, div_int);

    t1 = EQ_mul_32_32 (f1_int, f1_int);
    t1 = t1 - 0x00800000;
    t1 = t1 << 1;
    b_int[1] = EQ_div_32_32 (t1 >> 4, div_int);

    t1 = EQ_mul_32_32 (f1_int, f1_int);
    t2 = EQ_mul_32_32 (f1_int, EQ_SQRT_2_IN_5_27);
    t1 = 0x00800000 + t1 - t2;
    b_int[2] = EQ_div_32_32 (t1 >> 4, div_int);

    t1 = EQ_mul_32_32 (f2_int, f2_int);
    t1 = t1 - 0x00800000;
    t1 = t1 << 1;
    a_int[1] = -EQ_div_32_32 (t1 >> 4, div_int);

    t1 = EQ_mul_32_32 (f2_int, f2_int);
    t2 = EQ_mul_32_32 (f2_int, EQ_SQRT_2_IN_5_27);
    t1 = 0x00800000 + t1 - t2;
    a_int[2] = -EQ_div_32_32 (t1 >> 4, div_int);

	return 0;
}

int32_t EQ_generate_high_shelving_isr (int32_t dbval_int, int32_t fs_int, int32_t b_int[], int32_t a_int[])
{
	int32_t ret_val;
    int32_t phase_int;
    int32_t f1_int, f2_int, fac_int;
    int32_t t1, t2, div_int;

    if (dbval_int == 0)
    {
        b_int[0] = 0x01000000; /* The default value from 0x04000000 becomes 0x01000000 due to div4 factor */
        b_int[1] = 0x00000000;
        b_int[2] = 0x00000000;
        a_int[1] = 0x00000000;
        a_int[2] = 0x00000000;
        return 0;
    }

    dbval_int = (dbval_int * 32768) / 10;

	ret_val = 0;
    /* Low-Shelving filter */
    phase_int = EQ_div_32_32 (3141, fs_int) << 1;
	/* phase_int = phase_int << 3; */
	phase_int = EQ_div_32_32 (fs_int, 31410) >> 3;

    if (dbval_int < 0)
        fac_int = EQ_db_2_linear(-dbval_int/2, EQ_pow_table_db_int_neg);
    else
        fac_int = EQ_db_2_linear(dbval_int/2, EQ_pow_table_db_int);

	if (dbval_int < 0)
	{
        f1_int = phase_int >> 4;
        f2_int = EQ_div_32_32 (phase_int, fac_int) >> 6;
	}
	else
	{
        f2_int = phase_int >> 4;
        f1_int = EQ_mul_32_32 (phase_int, fac_int) >> 1;
	}

#define	FAC1	(32)
#define	FAC2	(FAC1*FAC1)
    t1 = EQ_mul_32_32 (f2_int, f2_int);
    t2 = EQ_mul_32_32 (f2_int, EQ_SQRT_2_IN_5_27);
    div_int = 0x00800000/FAC2 + t1 + t2/FAC1;

    t1 = EQ_mul_32_32 (f1_int, f1_int);
    t2 = EQ_mul_32_32 (f1_int, EQ_SQRT_2_IN_5_27);
    t1 = 0x00800000/FAC2 + t1 + t2/FAC1;
    b_int[0] = EQ_div_32_32 (t1 >> 4, div_int);

    t1 = EQ_mul_32_32 (f1_int, f1_int);
    t1 = t1 - 0x00800000/FAC2;
    t1 = t1 << 1;
    b_int[1] = EQ_div_32_32 (t1 >> 4, div_int);

    t1 = EQ_mul_32_32 (f1_int, f1_int);
    t2 = EQ_mul_32_32 (f1_int, EQ_SQRT_2_IN_5_27);
    t1 = 0x00800000/FAC2 + t1 - t2/FAC1;
    b_int[2] = EQ_div_32_32 (t1 >> 4, div_int);

    t1 = EQ_mul_32_32 (f2_int, f2_int);
    t1 = t1 - 0x00800000/FAC2;
    t1 = t1 << 1;
    a_int[1] = -EQ_div_32_32 (t1 >> 4, div_int);

    t1 = EQ_mul_32_32 (f2_int, f2_int);
    t2 = EQ_mul_32_32 (f2_int, EQ_SQRT_2_IN_5_27);
    t1 = 0x00800000/FAC2 + t1 - t2/FAC1;
    a_int[2] = -EQ_div_32_32 (t1 >> 4, div_int);

	a_int[1] = -a_int[1];
	b_int[1] = -b_int[1];

	b_int[0] = b_int[0] >> 2;
	b_int[1] = b_int[1] >> 2;
	b_int[2] = b_int[2] >> 2;

	return 0;
}

int32_t EQ_generate_subwoofer_coefs_isr (
		int32_t fs_int,
		int32_t subsonic_freq,
		int32_t b0[],
		int32_t b1[],
		int32_t b2[],
		int32_t a1[],
		int32_t a2[]
		)
{
    int32_t i, ret_val;
	int32_t fs_index;
	int32_t subsonic_param;

	fs_index = -1;
	subsonic_param = -1;
	ret_val = 0;

	if (fs_int == 32000)
		fs_index = 0;
	if (fs_int == 44100)
		fs_index = 1;
	if (fs_int == 48000)
		fs_index = 2;

    if ((fs_index < 0) || (fs_index > 2))
        ret_val = -1;      /* Error in sample rate index */

	if (subsonic_freq < EQ_subsonic_cutoff_freq[0])
		ret_val = -1;
	if (subsonic_freq > EQ_subsonic_cutoff_freq[EQ_NSUBSONIC_FREQ-1])
		ret_val = -1;
	subsonic_param = 0;
	for (i=0;i<EQ_NSUBSONIC_FREQ-1;i++)
    {
		if (EQ_subsonic_cutoff_freq_mid[i] < subsonic_freq)
			subsonic_param++;
    }

    /* Now fill all the coefs as it is */
	if (ret_val == 0)
    {
		b0 [0] =  EQ_subwoofer_table[fs_index][subsonic_param][0];
		b1 [0] =  EQ_subwoofer_table[fs_index][subsonic_param][1];
		b2 [0] =  EQ_subwoofer_table[fs_index][subsonic_param][2];
		b0 [1] =  EQ_subwoofer_table[fs_index][subsonic_param][3];
		b1 [1] =  EQ_subwoofer_table[fs_index][subsonic_param][4];
		b2 [1] =  EQ_subwoofer_table[fs_index][subsonic_param][5];

		a1 [0] = -EQ_subwoofer_table[fs_index][subsonic_param][7];
		a2 [0] = -EQ_subwoofer_table[fs_index][subsonic_param][8];
		a1 [1] = -EQ_subwoofer_table[fs_index][subsonic_param][10];
		a2 [1] = -EQ_subwoofer_table[fs_index][subsonic_param][11];
    }
    return 0;
}
int32_t EQ_generate_subsonic_coefs_isr (
		int32_t fs_int,
		int32_t subsonic_freq,
		int32_t b0[],
		int32_t b1[],
		int32_t b2[],
		int32_t a1[],
		int32_t a2[]
		)
{
    int32_t i, ret_val;
	int32_t fs_index;
	int32_t subsonic_param;

	fs_index = -1;
	subsonic_param = -1;
	ret_val = 0;

	if (fs_int == 32000)
		fs_index = 0;
	if (fs_int == 44100)
		fs_index = 1;
	if (fs_int == 48000)
		fs_index = 2;

    if ((fs_index < 0) || (fs_index > 2))
        ret_val = -1;      /* Error in sample rate index */

	if (subsonic_freq < EQ_subsonic_cutoff_freq[0])
		ret_val = -1;
	if (subsonic_freq > EQ_subsonic_cutoff_freq[EQ_NSUBSONIC_FREQ-1])
		ret_val = -1;
	subsonic_param = 0;
	for (i=0;i<EQ_NSUBSONIC_FREQ-1;i++)
    {
		if (EQ_subsonic_cutoff_freq_mid[i] < subsonic_freq)
			subsonic_param++;
    }

    /* Now fill all the coefs as it is */
	if (ret_val == 0)
    {
		b0 [0] = EQ_subsonic_table[fs_index][subsonic_param][0];
		b1 [0] = EQ_subsonic_table[fs_index][subsonic_param][1];
		b2 [0] = EQ_subsonic_table[fs_index][subsonic_param][2];
		b0 [1] = EQ_subsonic_table[fs_index][subsonic_param][3];
		b1 [1] = EQ_subsonic_table[fs_index][subsonic_param][4];
		b2 [1] = EQ_subsonic_table[fs_index][subsonic_param][5];

		a1 [0] = -EQ_subsonic_table[fs_index][subsonic_param][7];
		a2 [0] = -EQ_subsonic_table[fs_index][subsonic_param][8];
		a1 [1] = -EQ_subsonic_table[fs_index][subsonic_param][10];
		a2 [1] = -EQ_subsonic_table[fs_index][subsonic_param][11];
    }
    return 0;
}

int32_t EQ_mul_32_32 (int32_t a, int32_t b)
{
    int32_t a1, a0, b1, b0;
    int32_t acc;

    a0 = a & 0x0000ffff;
    b0 = b & 0x0000ffff;

    /* preserve the sign bits */
    a1 = (a >> 16);
    b1 = (b >> 16);

    acc = 0;
    /* First add the unsigned muls */
    acc += a1 * b1;
    acc = (acc >> 17) & 0x00007fff;     /* Unsigned number */
    /* a1 * b0 + a0 * b1; Shift by 1 to avoid overflow */
    acc += (a1 * b0) >> 1;
    acc += (b1 * a0) >> 1;

    acc = acc >> 15;
    /* Now add a1 * b1 */
    acc += a1 * b1;
    if (acc == 0x40000000) {
        /* Overflow. Will happen when a = 0x80000000 and b = 0x80000000 and numbers close to it */
        acc = 0x7fffffff;
    } else {
        acc = acc << 1;
    }

    return acc;
}

void EQ_renormalise (int32_t *p_mant, int32_t *p_exp)
{
    int32_t ret = 0;
    int32_t mant, exp;
    int32_t sign = 1;
    mant = *p_mant;
    exp = *p_exp;
    if (mant == 0)
        ret = 1;
    if (ret == 0)
    {
        if (mant < 0)
        {
            mant = -mant;
            sign = -1;
        }
        while ((mant & 0xc0000000) == 0)
        {
            mant = mant << 1;
            exp = exp - 1;
        }
        *p_mant = mant * sign;
        *p_exp = exp;
    }
    return;
}

int32_t EQ_div_32_32 (int32_t num, int32_t den)
{
    int32_t i, res;
    int32_t begin, end, mid;
    int32_t sign = 1;
    if (num == 0)
        return 0;
    if (den == 0)
        return 0;
    if (num < 0)
    {
        num = -num;
        sign = -sign;
    }
    if (den < 0)
    {
        den = -den;
        sign = -sign;
    }

    num = num/2;
    if (num > den) return 0;

    begin = 0x00000000;
    end = 0x40000000;
    for (i=0;i<32;i++)
    {
        mid = (begin + end)/2;

        res = EQ_mul_32_32 (mid, den);
        if (res > num )
            end = mid;
        else
            begin = mid;
    }

    return end*sign;        /* num is already divided by 2. */
                            /* The return value should be in 2.30 format. Both are compensated */
}

int32_t EQ_sqrt_32 (int32_t in)
{
    int32_t begin, end, mid;
    int32_t i, res;
    begin = 0;
    end = 0x40000000;
    if (in < 0) return 0;
    in = in >> 2;

    for (i=0;i<30;i++)
    {
        mid = (begin+end)/2;
        res = EQ_mul_32_32 (mid, mid);
        if (res > in)
            end = mid;
        else
            begin = mid;
    }
    end = end << 1;
    if (end < 0)		/* Corner case of 0x80000000 */
        end--;
    return end;
}

int32_t EQ_db_2_linear(int32_t in, int32_t table[])
{
    int32_t ret, i;
    ret = 0x10000000;
    for (i=0;i<20;i++)
    {
        if (in & (1 << i))
        {
            ret = EQ_mul_32_32 (ret, table[i]) << 3;
        }
    }
    return ret;
}

void EQ_cos_sin (int32_t phase, int32_t *cos_val, int32_t *sin_val)
{
    int32_t sign;
    int32_t sv, cv, i;
    *cos_val = 0;
    *sin_val = 0;
    sign = 1;
    if (phase < 0)
    {
        phase = -phase;
        sign = -1;
    }
    /* Supports only between +/- PI */
    if (phase > 0x3fffffff)
        return;

    sv = 0x00000000;
    cv = 0x40000000;
    for (i=0;i<30;i++)
    {
        if (phase & (1<<i))
        {
            int32_t stemp, ctemp;
            /* Update both sin and cos value */
            /* sin(A+B) = sinA cosB + cosA sinB */
            /* cos(A+B) = cosA cosB - sinA sinB */
            stemp = EQ_mul_32_32 (sv, EQ_cos_table[i]);
            stemp += EQ_mul_32_32 (cv, EQ_sin_table[i]);

            ctemp = EQ_mul_32_32 (cv, EQ_cos_table[i]);
            ctemp -= EQ_mul_32_32 (sv, EQ_sin_table[i]);

            sv = stemp << 1;
            cv = ctemp << 1;
        }
    }
    *sin_val = sv * sign;
    *cos_val = cv;
    return;
}
/* --------------------------- EQ SPECIFICS END ----------------------------- */


/* --------------------------- RAP PI SPECIFICS ----------------------------- */
#if BRAP_P_EQUALIZER

BERR_Code BRAP_P_ApplyCoefficients(
    BRAP_EqualizerHandle hEqualizer
    )
{
    BERR_Code   ret = BERR_SUCCESS;
    
    BDBG_ENTER(BRAP_P_ApplyCoefficients);
    
	BKNI_EnterCriticalSection();
	ret = BRAP_P_ApplyCoefficients_isr(hEqualizer);
	BKNI_LeaveCriticalSection();
    
    BDBG_LEAVE(BRAP_P_ApplyCoefficients);
    return ret;
}


BERR_Code BRAP_P_ApplyCoefficients_isr(
    BRAP_EqualizerHandle hEqualizer
    )
{
    BERR_Code   ret = BERR_SUCCESS;

    int32_t         b0[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t         b1[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t         b2[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t         a1[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t         a2[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t         b[3], a[3];
    int32_t         req_db[BRAP_P_MAX_IIR_FILTERS_IN_SRC];    
    unsigned int    i = 0, uiPp = 0, uiCsc = 0, uiPth = 0;
    BRAP_OutputChannelPair  eChPair = BRAP_OutputChannelPair_eMax;
    BRAP_EQ_P_FilterCoeffs  sFilterCoeffs[BRAP_RM_P_MAX_SRC_IN_CASCADE];
    BRAP_SRC_P_ConfigParams sSRCConfigParams[BRAP_RM_P_MAX_SRC_IN_CASCADE];

    BDBG_ENTER(BRAP_P_ApplyCoefficients_isr);
    
    BDBG_ASSERT(hEqualizer);
    
    /* Init default values for equalizer coefficients */
    for (i=0;i<BRAP_P_MAX_IIR_FILTERS_IN_SRC;i++)
    {
        b0[i] = 0x04000000;
        req_db[i] = b1[i] = b2[i] = a1[i] = a2[i] = 0;

        sFilterCoeffs[0].b0[i] = 0x04000000;
        sFilterCoeffs[0].b1[i] = 0x0;
        sFilterCoeffs[0].b2[i] = 0x0;
        sFilterCoeffs[0].a1[i] = 0x0;
        sFilterCoeffs[0].a2[i] = 0x0;

        sFilterCoeffs[1].b0[i] = 0x04000000;
        sFilterCoeffs[1].b1[i] = 0x0;
        sFilterCoeffs[1].b2[i] = 0x0;
        sFilterCoeffs[1].a1[i] = 0x0;
        sFilterCoeffs[1].a2[i] = 0x0;        
    }
                
    BDBG_MSG(("Coefficients Generation for Equalizer 0x%x, sampling rate %u:", 
                                    hEqualizer,hEqualizer->ui32SamplingRate));
    BDBG_MSG(("---------------------------------------------------------------------"));

    if(hEqualizer->sExtEqualizerDetails.bCustomMode == true)
    {
        BDBG_MSG(("The Equalizer type is Custom Mode"));

        if(hEqualizer->sExtEqualizerDetails.sCustomSettings.uiNumFilters > 8)
        {
            BDBG_ERR(("Custom Mode Equalizer: Number of IIR filters cannot be more than 8."
                "No programming done."));
            ret = BERR_INVALID_PARAMETER;
            goto stop_apply_coeff;
        }
            
        for (i=0; i < hEqualizer->sExtEqualizerDetails.sCustomSettings.uiNumFilters; i++)
        {
            if(((hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff0[i] < (int32_t)0xF8000000) ||
                (hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff0[i] > 0x7FFFFFF))  ||
               ((hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff1[i] < (int32_t)0xF8000000) ||
                (hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff1[i] > 0x7FFFFFF))  ||
               ((hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff2[i] < (int32_t)0xF8000000) ||
                (hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff2[i] > 0x7FFFFFF))  ||
               ((hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff3[i] < (int32_t)0xF8000000) ||
                (hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff3[i] > 0x7FFFFFF))  ||
               ((hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff4[i] < (int32_t)0xF8000000) ||               
                (hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff4[i] > 0x7FFFFFF)) )
            {
                BDBG_ERR(("Custom Mode Equalizer: Invalid Coefficients provided. No programming done."));
                ret = BERR_INVALID_PARAMETER;                
                goto stop_apply_coeff;            
            }
        }
        
        for (i=0; i < hEqualizer->sExtEqualizerDetails.sCustomSettings.uiNumFilters; i++)
        {
            sFilterCoeffs[0].b0[i] = hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff0[i];
            sFilterCoeffs[0].b1[i] = hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff1[i];
            sFilterCoeffs[0].b2[i] = hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff2[i];
            sFilterCoeffs[0].a1[i] = hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff3[i];
            sFilterCoeffs[0].a2[i] = hEqualizer->sExtEqualizerDetails.sCustomSettings.i32Coeff4[i];
        }

        if((hEqualizer->sExtEqualizerDetails.bToneControlEqualizer == true) ||
           (hEqualizer->sExtEqualizerDetails.bFiveBandEqualizer == true)    ||
           (hEqualizer->sExtEqualizerDetails.bSevenBandEqualizer == true)   ||
           (hEqualizer->sExtEqualizerDetails.bSubsonicEnable == true)       ||
           (hEqualizer->sExtEqualizerDetails.bSubwooferEnable == true))
        {
            BDBG_WRN(("Custom mode Equalizer cannot be cascaded with any other mode!!!"
                "Disable the other modes. Programming only Custom mode!!!"));
        }        
    } 

    else
    {
        if ( hEqualizer->sExtEqualizerDetails.bSubsonicEnable == true)
        {
            BDBG_MSG(("Subsonic Frequency = %u",hEqualizer->sExtEqualizerDetails.ui32SubsonicFrequency));
            
            EQ_generate_subsonic_coefs_isr(hEqualizer->ui32SamplingRate,
                                       hEqualizer->sExtEqualizerDetails.ui32SubsonicFrequency,
                                        b0,
                                        b1,
                                        b2,
                                        a1,
                                        a2);

        	for (i=0; i<2; i++)
        	{
                sFilterCoeffs[0].b0[i] = b0[i];
                sFilterCoeffs[0].b1[i] = b1[i];
                sFilterCoeffs[0].b2[i] = b2[i];
                sFilterCoeffs[0].a1[i] = a1[i];
                sFilterCoeffs[0].a2[i] = a2[i];
            }
        }

        if ( hEqualizer->sExtEqualizerDetails.bSubwooferEnable == true)
        {
            if(hEqualizer->sExtEqualizerDetails.bSevenBandEqualizer == true)
            {
                BDBG_WRN(("Enabling of Subwoofer along with PEQ is currently not supported."
                    " Programming only PEQ!!!"));
            }
            else
            {
                BDBG_MSG(("Subwoofer Frequency = %u",hEqualizer->sExtEqualizerDetails.ui32SubwooferFrequency));
                
                EQ_generate_subwoofer_coefs_isr(hEqualizer->ui32SamplingRate,
                                           hEqualizer->sExtEqualizerDetails.ui32SubwooferFrequency,
                                            b0,
                                            b1,
                                            b2,
                                            a1,
                                            a2);

            	for (i=0; i<2; i++)
            	{
                    sFilterCoeffs[1].b0[i] = b0[i];
                    sFilterCoeffs[1].b1[i] = b1[i];
                    sFilterCoeffs[1].b2[i] = b2[i];
                    sFilterCoeffs[1].a1[i] = a1[i];
                    sFilterCoeffs[1].a2[i] = a2[i];
                }
            }
        }
        
        if(hEqualizer->sExtEqualizerDetails.bSevenBandEqualizer == true)
        {
            BDBG_MSG(("The Equalizer is 7 band PEQ"));
            BDBG_MSG(("PEQ Parameters"));            
            BDBG_MSG(("---------------"));                        

            if((hEqualizer->sExtEqualizerDetails.bToneControlEqualizer == true) ||
               (hEqualizer->sExtEqualizerDetails.bFiveBandEqualizer == true) ||
               (hEqualizer->sExtEqualizerDetails.bCustomMode == true) )
            {
                BDBG_WRN(("When PEQ is enabled all other Equalizer Modes should be disabled."
                          "Check the Equalizer settings and set appropriately!!!"));
            }
            
            for (i=0;i<BRAP_MAX_PEQ_FILTERS;i++)
        	{
        	    BDBG_MSG(("Band %d :: Frequency=%u, Gain=%d, Q=%u",
                 i,hEqualizer->sExtEqualizerDetails.sSevenBandSettings.uiFpk[i],
                 hEqualizer->sExtEqualizerDetails.sSevenBandSettings.iGain[i],
                 hEqualizer->sExtEqualizerDetails.sSevenBandSettings.uiPeqQ[i]
                 ));
            }

        		EQ_generate_peq_coefs_isr (
                &(hEqualizer->sExtEqualizerDetails.sSevenBandSettings.uiPeqQ[0]),
                &(hEqualizer->sExtEqualizerDetails.sSevenBandSettings.uiFpk[0]),
                &(hEqualizer->sExtEqualizerDetails.sSevenBandSettings.iGain[0]),
                  hEqualizer->ui32SamplingRate, 
                b0,
                b1,
                b2,
                a1,
                a2);
            
            /* Coefficients to be programmed in first SRC */
        	for (i=0; i<(BRAP_P_MAX_IIR_FILTERS_IN_SRC/2 - 1); i++)
        	{
        		sFilterCoeffs[0].b0[i*2+2] = b0[i];
        		sFilterCoeffs[0].b1[i*2+2] = b1[i];
        		sFilterCoeffs[0].b2[i*2+2] = b2[i];
        		sFilterCoeffs[0].a1[i*2+2] = a1[i];
        		sFilterCoeffs[0].a2[i*2+2] = a2[i];
            }

            /* Multiply by two */
        	for (i=0; i<(BRAP_P_MAX_IIR_FILTERS_IN_SRC/2 - 1); i++)
        	{
                sFilterCoeffs[0].b0[i*2+3] = 0x08000000;
                sFilterCoeffs[0].b1[i*2+3] = 0x0;
                sFilterCoeffs[0].b2[i*2+3] = 0x0;
                sFilterCoeffs[0].a1[i*2+3] = 0x0;
                sFilterCoeffs[0].a2[i*2+3] = 0x0;
        	}

            /* Coefficients to be programmed in second SRC */
        	for (i=0; i<(BRAP_P_MAX_IIR_FILTERS_IN_SRC/2); i++)
        	{
        		sFilterCoeffs[1].b0[i*2] = b0[i+(BRAP_P_MAX_IIR_FILTERS_IN_SRC/2 - 1)];
        		sFilterCoeffs[1].b1[i*2] = b1[i+(BRAP_P_MAX_IIR_FILTERS_IN_SRC/2 - 1)];
        		sFilterCoeffs[1].b2[i*2] = b2[i+(BRAP_P_MAX_IIR_FILTERS_IN_SRC/2 - 1)];
        		sFilterCoeffs[1].a1[i*2] = a1[i+(BRAP_P_MAX_IIR_FILTERS_IN_SRC/2 - 1)];
        		sFilterCoeffs[1].a2[i*2] = a2[i+(BRAP_P_MAX_IIR_FILTERS_IN_SRC/2 - 1)];
            }

            /* Multiply by two */        
        	for (i=0; i<(BRAP_P_MAX_IIR_FILTERS_IN_SRC/2); i++)
        	{
                sFilterCoeffs[1].b0[i*2+1] = 0x08000000;
                sFilterCoeffs[1].b1[i*2+1] = 0x0;
                sFilterCoeffs[1].b2[i*2+1] = 0x0;
                sFilterCoeffs[1].a1[i*2+1] = 0x0;
                sFilterCoeffs[1].a2[i*2+1] = 0x0;
        	}
        }     
            /* GEQ/Tone Control (or) GEQ+Tone Control */    
        else if((hEqualizer->sExtEqualizerDetails.bFiveBandEqualizer == true) ||
                (hEqualizer->sExtEqualizerDetails.bToneControlEqualizer == true))
        {
            if(hEqualizer->sExtEqualizerDetails.bFiveBandEqualizer == true)        
            {
                BDBG_MSG(("The Equalizer type is Five Band Graphic Equalizer"));
                BDBG_MSG(("The Values are 100Hz:%d, 300Hz:%d, 1kHz:%d, 3kHz:%d, 10kHz:%d",
                    hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i100HzGain,
                    hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i300HzGain,
                    hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i1000HzGain,
                    hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i3000HzGain,
                    hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i10000HzGain));
                
                req_db[0] = hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i100HzGain;
                req_db[1] = hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i300HzGain;
                req_db[2] = hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i1000HzGain;
                req_db[3] = hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i3000HzGain;
                req_db[4] = hEqualizer->sExtEqualizerDetails.sFiveBandSettings.i10000HzGain;     
               
                EQ_generate_equaliser_coefs_isr (
                    hEqualizer->ui32SamplingRate, 
                    req_db, 
                    &b0[2], 
                    &b1[2], 
                    &b2[2], 
                    &a1[2], 
                    &a2[2]
                    );

            	for (i=2; i<(2+BRAP_P_MAX_GEQ_FILTERS); i++)
            	{
                    sFilterCoeffs[0].b0[i] = b0[i];
                    sFilterCoeffs[0].b1[i] = b1[i];
                    sFilterCoeffs[0].b2[i] = b2[i];
                    sFilterCoeffs[0].a1[i] = a1[i];
                    sFilterCoeffs[0].a2[i] = a2[i];
    	        }
            }            
            
            if(hEqualizer->sExtEqualizerDetails.bToneControlEqualizer == true)
            {    
                BDBG_MSG(("The Equalizer type is Tone Control"));
                BDBG_MSG(("The Values are Bass:%d, Treble:%d",
                    hEqualizer->sExtEqualizerDetails.sToneControlSettings.iBassGain,
                    hEqualizer->sExtEqualizerDetails.sToneControlSettings.iTrebleGain));

                /* Treble */
                EQ_generate_high_shelving_isr (
                    hEqualizer->sExtEqualizerDetails.sToneControlSettings.iTrebleGain, 
                    hEqualizer->ui32SamplingRate, 
                    b, 
                    a
                    );
                
                sFilterCoeffs[1].b0[2] = b[0];
                sFilterCoeffs[1].b1[2] = b[1];
                sFilterCoeffs[1].b2[2] = b[2];
                sFilterCoeffs[1].a1[2] = a[1];
                sFilterCoeffs[1].a2[2] = a[2];

                /* Bass */
                EQ_generate_low_shelving_isr (
                    hEqualizer->sExtEqualizerDetails.sToneControlSettings.iBassGain, 
                    hEqualizer->ui32SamplingRate, 
                    b, 
                    a
                    );            
                sFilterCoeffs[1].b0[3] = b[0];
                sFilterCoeffs[1].b1[3] = b[1];
                sFilterCoeffs[1].b2[3] = b[2];
                sFilterCoeffs[1].a1[3] = a[1];
                sFilterCoeffs[1].a2[3] = a[2];

                /* Set 2 more filters as multiply by -2 */
                sFilterCoeffs[1].b0[4] = 0x08000000;
                sFilterCoeffs[1].b0[5] = 0x08000000;
            }
        }
    }
    
    BDBG_MSG (("\n Programming generated Equalizer Coefficients "));
    BDBG_MSG (("-----------------------------------------------------"));

    for(uiCsc = 0; uiCsc < BRAP_RM_P_MAX_SRC_IN_CASCADE; uiCsc++)
    {
        for(i=0; i<BRAP_P_MAX_IIR_FILTERS_IN_SRC; i++)
        {
            sSRCConfigParams[uiCsc].uCoeff.sIIRCoeff.b0[i] = sFilterCoeffs[uiCsc].b0[i];
            sSRCConfigParams[uiCsc].uCoeff.sIIRCoeff.b1[i] = sFilterCoeffs[uiCsc].b1[i];
            sSRCConfigParams[uiCsc].uCoeff.sIIRCoeff.b2[i] = sFilterCoeffs[uiCsc].b2[i];
            sSRCConfigParams[uiCsc].uCoeff.sIIRCoeff.a1[i] = sFilterCoeffs[uiCsc].a1[i];
            sSRCConfigParams[uiCsc].uCoeff.sIIRCoeff.a2[i] = sFilterCoeffs[uiCsc].a2[i];
        }
    }
                        
    for(uiPth=0; uiPth<BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
    {
        for(uiPp=0; uiPp<BRAP_RM_P_MAX_PARALLEL_PATHS; uiPp++)
        {
            for(eChPair=0; eChPair<BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; eChPair++)
            {
                for(uiCsc = 0; uiCsc < BRAP_RM_P_MAX_SRC_IN_CASCADE; uiCsc++)
                {
                    if (hEqualizer->hSrcEq[uiCsc][eChPair][uiPp][uiPth] != NULL)
                    {
                        ret = BRAP_SRC_P_SetConfig_isr(hEqualizer->hSrcEq[uiCsc][eChPair][uiPp][uiPth],
                                                        &sSRCConfigParams[uiCsc]);
                        if(BERR_SUCCESS != ret)
                        {
                            BDBG_ERR(("BRAP_P_ApplyCoefficients_isr: BRAP_SRC_P_SetConfig_isr returned"
                                      "error for hSrcEq[0] = 0x%x", hEqualizer->hSrcEq[0]));
                            goto stop_apply_coeff;
                        }
                    }                
                }
            }
        }    
    }
                                
stop_apply_coeff:
    BDBG_LEAVE(BRAP_P_ApplyCoefficients_isr);    
    return ret;
}
#endif

/* End of File */
