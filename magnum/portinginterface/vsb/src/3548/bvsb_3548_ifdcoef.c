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
unsigned short ifd_coef1[] =
{
     /*' --> Loading IFD Filter #1, # coeffs= 39*/
     /* (-22,-48,-89,-138,-185,-208,-182,-71,160,543,1099,1833,2727,3742,4813,5861,6799,7542,8019,8184,*/
	 /*  8019,7542,6799,5861,4813,3742,2727,1833,1099,543,160,-71,-182,-208,-185,-138,-89,-48,-22)*/
     0x3FEA,0x3FD0,0x3FA7,0x3F76,0x3F47,0x3F30,0x3F4A,0x3FB9,
     0x00A0,0x021F,0x044B,0x0729,0x0AA7,0x0E9E,0x12CD,0x16E5,
     0x1A8F,0x1D76,0x1F53,0x1FF8,0x1F53,0x1D76,0x1A8F,0x16E5,
     0x12CD,0x0E9E,0x0AA7,0x0729,0x044B,0x021F,0x00A0,0x3FB9,
     0x3F4A,0x3F30,0x3F47,0x3F76,0x3FA7,0x3FD0,0x3FEA
};

unsigned short ifd_coef1_AllPass[] =
{
    /* 2009.05.13:  all-pass filter (single tap) */
    /* --> Loading IFD Filter #1, # coeffs= 39 */
    /* IFD_CORE.VP_SHFT.LPF = 8+4   */
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,5786,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0
};

unsigned short ifd_coef1_LPF_L_Pos_Mod[] =   /* added [TLS 20080616]*/
{   /* for positive modulation, negate all ifd_coef1 taps*/
    /*' --> Loading IFD Filter #1, # coeffs= 39*/
    /* (22,48,89,138,185,208,182,71,-160,-543,-1099,-1833,-2727,-3742,-4813,-5861,-6799,-7542,-8019,-8184,*/
	/*  -8019,-7542,-6799,-5861,-4813,-3742,-2727,-1833,-1099,-543,-160,71,182,208,185,138,89,48,22)*/
    0x0016,0x0030,0x0059,0x008A,0x00B9,0x00D0,0x00B6,0x0047,0x3F60,0x3DE1,
    0x3BB5,0x38D7,0x3559,0x3162,0x2D33,0x291B,0x2571,0x228A,0x20AD,0x2008,
    0x20AD,0x228A,0x2571,0x291B,0x2D33,0x3162,0x3559,0x38D7,0x3BB5,0x3DE1,
    0x3F60,0x0047,0x00B6,0x00D0,0x00B9,0x008A,0x0059,0x0030,0x0016
};

unsigned short ifd_coef2[] = /*26*/
{   
    /*' --> Loading IFD Filter #2, # coeffs= 26*/
    0x0002,0x0000,0x3FFE,0x0000,0x0004,0x0000,0x3FFE,0x0000,
    0x3FFA,0x0000,0x0018,0x0000,0x3FD2,0x0000,0x0034,0x0000,
    0x0000,0x0000,0x3F5C,0x0000,0x021A,0x0000,0x3A90,0x0000,
    0x13E4,0x1FF0
};

unsigned short ifd_coef3[] = /*40    Normal NTSC GDF*/
{
    /*  OLD   */
    /* FREQ RESP (dBc rel to DC): (DC gain = 13008,   Gain/ 2^13 = 1.5879)
     *   0.00   3.58   3.75   3.80   4.00   4.10   4.20   4.30   4.40   4.45   4.50   4.55 
     *   0.00  -0.00   0.01   0.01  -0.01  -0.00   0.01   0.01   0.00  -0.00  -0.01  -0.00 
     *  GROUP DELAY RESP (ns rel to DC):  (DC grp dly = 9.059 samples at 13.5 MHz)
     *   0.00 185.10 237.05 249.10 292.36 325.56 371.23 419.16 449.77 451.89 442.56 421.00 
     *  IFD_CORE.VP_SHFT.GDLY = &H7

     * --> Loading IFD Filter #3, # coeffs= 40
     *0x0000,0x01EA,0x3B04,0x084F,0x34A7,0x0E22,0x3232,0x0588,
     *0x07EE,0x14F8,0x1FFF,0x39EC,0x344F,0x0BB3,0x3FAC,0x38B5,
     *0x04FF,0x0110,0x3C56,0x019F,0x0092,0x3E9F,0x00D0,0x0022,
     *0x3F5F,0x007A,0x0002,0x3FAA,0x0047,0x0001,0x3FCF,0x0025,
     *0x0002,0x3FE6,0x0013,0x0002,0x3FF3,0x0009,0x0001,0x3FFA
     */

    /*  NEW   */
    /* 2008.10.09:  New for optimal ringing.  For complete response,
    *   needs to be used with ATF called: Enable_NTSC_AUDTRAP_ImprovedRinging_Filter4_for_IFD()

    *Test_Audio_Trap (combined with GDF).bss
    * FREQ RESP (dBc rel to DC): (DC gain = 6504,   Gain/ 2^13 = 0.7939)
    *   0.00  2.00  2.50  3.00  3.10  3.20  3.30  3.40  3.50  3.58   3.60   3.70   3.80   3.90   4.00   4.10   4.20   4.30   4.40   4.50   4.60   4.70   4.72   4.80 
    *   0.00  0.18  0.30  0.47  0.51  0.54  0.56  0.57  0.55  0.51   0.49   0.35   0.08  -0.41  -1.26  -2.71  -5.03  -8.75 -15.31 -71.49 -16.93 -11.83 -11.07  -9.22 
    * GROUP DELAY RESP (ns rel to DC):  (DC grp dly = 0.158 samples at 13.5 MHz)
    *   0.00  9.53 18.06 39.74 46.67 55.88 67.77 81.99 99.85 118.11 123.28 152.96 189.58 233.03 277.70 312.33 326.02 313.65 277.78 -391271 187.03 152.64 145.40 124.42 
    *IFD_CORE.VP_SHFT.GDLY = &H7 + 1
    */

    0x12F7,0x0799,0x0246,0x39C6,0x0247,0x029F,0x3CF9,0x0023,0x01DB,0x3ECD,
    0x3F88,0x010C,0x3FAC,0x3F87,0x007F,0x0002,0x3FAE,0x0030,0x0018,0x3FD3,
    0x000C,0x0016,0x3FEB,0x3FFE,0x000E,0x3FF9,0x3FFB,0x0008,0x3FFE,0x3FFC,
    0x0003,0x0000,0x3FFE,0x0001,0x0001,0x3FFF,0x0000,0x0001,0x3FFF,0x0000
};

unsigned short ifd_coef3_GDF_Byp_NTSC[] = /*40    BYPASS NTSC GDF    [TLS 20060524] */
{   /* '*** NTSC GDF bypass (gain and delay matched using MATLAB.  Confirmed using CORDIC on oscilloscope)*/
    /*' *** Filter DC gain is 0.5 the original so:  IFD_CORE.VP_SHFT.GDLY = &H7 + 1*/
    /* '*** Filter #3:  VPGDLY  (40 regs)  14-bit coef*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 6504, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned short ifd_coef3_GDF_Byp_PAL_D[] = /*40    BYPASS PAL-D GDF    [TLS 20060524] */
{   /* '*** PAL-D GDF bypass (main tap position adjusted so that keyed AGC (CORDIC) lined up on oscilloscope)*/
    /*' *** Filter DC gain is 0.5 the original so:  IFD_CORE.VP_SHFT.GDLY = &H7 + 1*/
    /* '*** Filter #3:  VPGDLY  (40 regs)  14-bit coef*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6504, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0
};

unsigned short ifd_coef3_GDF_PAL_B_CurveA[] = /*40   */
{   /* %2007.10.16:  PAL-B "Curve A" filter from Sandeep using Mark's RFM routine flipped for the IFD.*/
    /* FREQ RESP (dBc rel to DC): (DC gain = 6504,   Gain/ 2^13 = 0.7939)*/
    /*   0.00   0.50   1.00   1.50   2.00   2.50   3.00   3.50   4.00   4.43   5.00   5.50   6.00   6.50 */
    /*   0.00   0.04   0.04   0.03   0.03   0.03   0.04   0.04   0.03   0.02  -1.46  -3.12  -2.91  -1.96 */
    /* GROUP DELAY RESP (ns rel to DC):  (DC grp dly = 19.497 samples at 13.5 MHz)*/
    /*   0.00 -16.48 -29.56 -46.82 -63.49 -66.10 -55.95 -26.05  66.07 180.56 199.22  91.55  49.35  32.62 */

    /*GDFcoef= Array(16,-69,153,-282,414,-519,536,-451,261,-30,-186,280,-221,9,180,-210,-145,-645,1206,4592,*/
    /*               2049,-1252,548,766,-908,384,313,-591,402,59,-489,737,-761,626,-416,219,-77,5,18,-17)*/
    /*IFD_CORE.VP_SHFT.GDLY = 0x7 + 1*/

    0x0010,0x3FBB,0x0099,0x3EE6,0x019E,0x3DF9,0x0218,0x3E3D,0x0105,0x3FE2,
    0x3F46,0x0118,0x3F23,0x0009,0x00B4,0x3F2E,0x3F6F,0x3D7B,0x04B6,0x11F0,
    0x0801,0x3B1C,0x0224,0x02FE,0x3C74,0x0180,0x0139,0x3DB1,0x0192,0x003B,
    0x3E17,0x02E1,0x3D07,0x0272,0x3E60,0x00DB,0x3FB3,0x0005,0x0012,0x3FEF
};

unsigned short ifd_coef3_GDF_PAL_B_CurveB[] = /*40   */
{   /* %2007.10.16:  PAL-B "Curve B" filter from Sandeep using Mark's RFM routine flipped for the IFD.*/
    /* FREQ RESP (dBc rel to DC): (DC gain = 6505,   Gain/ 2^13 = 0.7941)*/
    /*   0.00   0.50   1.00   1.50   2.00   2.50   3.00   3.50   4.00   4.43   5.00   5.50   6.00   6.50 */
    /*   0.00   0.04   0.04   0.03   0.02   0.02   0.03   0.05   0.03   0.02  -0.62  -3.06  -3.01  -5.51 */
    /* GROUP DELAY RESP (ns rel to DC):  (DC grp dly = 19.486 samples at 13.5 MHz)*/
    /*   0.00 -22.46 -52.05 -77.15 -93.16 -91.16 -68.31 -21.37  44.96 178.79 485.56 730.48 1050.36 1432.38 */

    /*GDFcoef= Array(13,-35,56,-79,75,-44,-32,134,-249,331,-348,275,-186,60,16,-164,-551,-311,2054,4611,*/
    /*               893,-1160,1045,464,-882,592,199,-712,743,-271,-401,973,-1237,1176,-901,565,-280,100,-15,-12)*/
    /*IFD_CORE.VP_SHFT.GDLY = 0x7 + 1*/

    0x000D,0x3FDD,0x0038,0x3FB1,0x004B,0x3FD4,0x3FE0,0x0086,0x3F07,0x014B,
    0x3EA4,0x0113,0x3F46,0x003C,0x0010,0x3F5C,0x3DD9,0x3EC9,0x0806,0x1203,
    0x037D,0x3B78,0x0415,0x01D0,0x3C8E,0x0250,0x00C7,0x3D38,0x02E7,0x3EF1,
    0x3E6F,0x03CD,0x3B2B,0x0498,0x3C7B,0x0235,0x3EE8,0x0064,0x3FF1,0x3FF4
};

unsigned short ifd_coef4[] = /*64*/
{
    /*  OLDEST   */
    /*' audio trap filter
    * DC gain (tap sum)= 13024    (Gain/ 2^13) = 1.589844
    * --> Loading IFD Filter #4, # coeffs= 64
    
    *  filter (TLS re-scaled on 8/12/2005 from STJ's summer 2004 design)
    *   fdatool 13.5 3.600000 4.400000 0.100000 60.000000
    * FREQ RESP (dBc rel to DC): (DC gain = 13024,   Gain/ 2^13 = 1.5898)
    *   0.00   3.00   3.58   3.75   3.80   4.00   4.10   4.20   4.30   4.40   4.45   4.50   4.55   4.72 
    *   0.00  -0.00  -0.06  -1.23  -2.00  -7.91 -13.17 -20.78 -32.41 -59.76 -58.39 -61.47 -71.85 -71.09 
    * GROUP DELAY RESP (ns rel to DC):  (DC grp dly = 63.000 samples at 13.5 MHz)
    *   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00 
    *IFD_CORE.VP_SHFT.TRAP = &Ha
 
    * 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    * 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    * 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    * 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    * 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    * 0x3FFC,0x0023,0x0024,0x3FDC,0x3FED,0x0045,0x3FF2,0x3FA2,
    * 0x004F,0x0057,0x3F5A,0x3FEC,0x00F5,0x3F82,0x3EF6,0x0159,
    * 0x00A0,0x3D9F,0x00A2,0x0360,0x3C6C,0x3BE6,0x0F74,0x1DA4   
    */
 
    /* OLD */
    /*' 2008.10.09:  New for optimal ringing.  For complete response,
    *   needs to be used with notch GDF called: Enable_NTSC_M_GDF_notch_Filter4_for_IFD()
 
    * FREQ RESP (dBc rel to DC): (DC gain = 13031,   Gain/ 2^13 = 1.5907)
    *  0.00  2.00  2.50  3.00  3.10  3.20  3.30  3.40  3.50  3.58   3.60   3.70   3.80   3.90   4.00   4.10   4.20   4.30   4.40   4.50   4.60   4.70   4.72   4.80 
    *  0.00  0.70  0.27 -0.88 -1.21 -1.64 -2.06 -2.63 -3.17 -3.74  -3.90  -4.64  -5.56  -6.68  -7.93  -9.74 -11.01 -16.95 -43.80 -40.57 -43.99 -80.21 -54.86 -25.56 
    * GROUP DELAY RESP (ns rel to DC):  (DC grp dly = 63.000 samples at 13.5 MHz)
    *  0.00  0.00  0.00  0.00  0.00  0.00  0.00  0.00  0.00  0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00 
    * IFD_CORE.VP_SHFT.TRAP = &Ha 
    */
 
	/*
   * 0x3FFE,0x3FFF,0x0005,0x3FFF,0x0000,0x3FFA,0x0009,0x3FFF,0x3FF2,0x0012,
   * 0x3FFC,0x3FEF,0x0018,0x3FF8,0x3FF0,0x001A,0x3FF5,0x3FF3,0x0018,0x3FF4,
   * 0x3FF8,0x0011,0x3FF7,0x3FFD,0x0006,0x3FFE,0x0000,0x3FF9,0x0009,0x0002,
   * 0x3FEC,0x0016,0x0002,0x3FE0,0x0024,0x3FFE,0x3FD7,0x0031,0x3FF8,0x3FD2,
   * 0x003C,0x3FEF,0x3FD3,0x0043,0x3FE6,0x3FD7,0x0046,0x3FDE,0x3FDE,0x0044,
   * 0x3FDA,0x3FE6,0x003C,0x3FDC,0x3FEE,0x002F,0x3FE4,0x3FF6,0x001B,0x3FF0,
   * 0x0001,0x3652,0x19BB,0x12D5  
   */
 

    /*  NEW   */
    /*' 2009.03.18:  Filter for Mitsubishi(Filter 6 for our reference). For complete response,
    *   needs to be used with notch GDF called: Enable_NTSC_M_GDF_notch_Filter4_for_IFD()
 
 * FREQ RESP (dBc rel to DC): (DC gain = 13030,   Gain/ 2^13 = 1.5906)
*0.00  2.00  2.50  3.00  3.10  3.20  3.30  3.40  3.50  3.58  3.60  3.70  3.80  3.90   4.00    4.10    4.20   4.30    4.40   4.50    4.60    4.70    4.72   4.80    4.90    5.00   5.10    
*0.00 -0.76 -0.83 -1.81 -2.09 -2.51 -2.95 -3.51 -4.19 -4.81 -4.99 -5.96 -7.24 -9.00 -11.72 -15.72 -19.89 -31.36 -60.67 -62.30 -51.64 -49.12 -51.63 -51.55 -50.82 -61.91 -55.28 
  
* GROUP DELAY RESP (ns rel to DC):  (DC grp dly = 63.000 samples at 13.5 MHz)
    *  0.00  0.00  0.00  0.00  0.00  0.00  0.00  0.00  0.00  0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00   0.00  
    * IFD_CORE.VP_SHFT.TRAP = &Ha 
    */   
 
   0x3FFA,0x3FFA,0x0006,0x3FFE,0x0000,0x0002,0x0000,0x3FFE,0x0002,0x0002,
   0x3FFC,0x0002,0x0004,0x3FFC,0x0000,0x0004,0x3FFE,0x3FFE,0x0006,0x0000,
   0x3FFC,0x0004,0x0002,0x3FFC,0x0004,0x0002,0x3FFA,0x0002,0x0004,0x3FFC,
   0x0000,0x0004,0x3FFE,0x0000,0x0004,0x3FFE,0x0000,0x0002,0x3FFE,0x0004,
   0x0002,0x3FF8,0x0008,0x0006,0x3FEE,0x0006,0x0010,0x3FE4,0x0000,0x0033,
   0x3FE8,0x3FF2,0x0049,0x3FBF,0x3F76,0x0057,0x3FFC,0x3F1F,0x018E,0x01A2,
   0x3D42,0x3E15,0x0F65,0x1924
 
};

unsigned short ifd_coef4_ATF_PAL_D[] = /*64   PAL-D Audio Trap  [TLS 20060524] */
{   /*' "new" PAL-D audio trap filter   */
    /*' print "fdatool  13.5 " & " 4.800000 "  & " 5.450000 " & " 0.100000 " & "62.000000 "*/
    /*' FREQ RESP:    (DC gain = 6517,   Gain/ 2^13 = 0.795532)*/
    /*'   0.00   4.43   4.80   4.90   5.00   5.10   5.20   5.30   5.40   5.50   6.00   6.50 */
    /*'   0.00   0.07  -0.01  -0.79  -2.84  -6.73 -13.12 -23.26 -41.17 -64.45 -65.55 -69.42 */
    /*' *** Filter DC gain is 0.5 the original so:  IFD_CORE.VP_SHFT.TRAP = &Ha + 1*/
    /*' Filter #4, Num Coefficients= 64, Coef Bit Width= 14*/

    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x3FEF,0x3FFB,0x000C,0x3FF0,0x000B,0x0004,
    0x3FEA,0x001F,0x3FEB,0x3FF8,0x0029,0x3FC8,0x0026,0x000C,
    0x3FBB,0x0061,0x3FBC,0x3FEF,0x0074,0x3F58,0x007C,0x0015,
    0x3F33,0x0142,0x3EF9,0x3FE9,0x01F3,0x3BF7,0x05AE,0x1347
};

unsigned short ifd_coef6[] = /*47*/
{
    /*' --> Loading IFD Filter #6, # coeffs= 47  (ORIGINAL)*/
    0x0002,0x0002,0x7FFC,0x7FE4,0x7FC8,0x7FC6,0x000E,0x008E,
    0x00E0,0x0074,0x7F2E,0x7DE4,0x7E0E,0x0074,0x03DA,0x052A,
    0x01B0,0x7A42,0x745C,0x7764,0x0738,0x1FF2,0x36BE,0x3FFE,
    0x36BE,0x1FF2,0x0738,0x7764,0x745C,0x7A42,0x01B0,0x052A,
    0x03DA,0x0074,0x7E0E,0x7DE4,0x7F2E,0x0074,0x00E0,0x008E,
    0x000E,0x7FC6,0x7FC8,0x7FE4,0x7FFC,0x0002,0x0002
};

unsigned short ifd_coef7[] = /*47    added [SandeepB 20070530] */
{
    /*
// "Bcm3560_Bx_IFD_Audio_FiltB__US_260k_asHEX.txt"
//
// IFD Audio Path Filter "B" (Filter #7,#10)
// Sample Rate: 947.368 kHz
// Passband Edge: 260.0 kHz
// Stopband Edge: 310.0 kHz
// ------------------------------------------------------
// DC gain: 1.6807 (4.510 dB)
// Coefficient Sum = 27537
// Maximum coefficient: 16127 (0x3eff = 98.43 %FS)
// Maximum FP coefficient: 0.984314 (1.14 binary format)
// Passband Ripple: 0.247 dB (4.511/4.264 dB)
// Stopband Rejection: 56.27 dB
// ------------------------------------------------------
*/

   0x7FED,0x0067,0x00A2,0x7FCD,0x7F95,0x009D,0x002E,0x7F05,0x0065,0x0118,
   0x7EBD,0x7F56,0x022A,0x7F77,0x7D62,0x0284,0x01F6,0x7B01,0x00A2,0x0775,
   0x7906,0x76BA,0x203C,0x3EFF,0x203C,0x76BA,0x7906,0x0775,0x00A2,0x7B01,
   0x01F6,0x0284,0x7D62,0x7F77,0x022A,0x7F56,0x7EBD,0x0118,0x0065,0x7F05,
   0x002E,0x009D,0x7F95,0x7FCD,0x00A2,0x0067,0x7FED
};

unsigned short ifd_coef7_BTSC_LPFB_Wide[] = /*47   added [SandeepB 20070530] */
{

    /* NOTE:  AP_SHFT.LPFB = 6 (was 5) since LPFB filter DC gain is 1/2 stj original*/
    /*        AP_SHFT.TBC  = 7 (was 7) for old 143 kHz max deviation:*/
    /*        AP_OUTFACTOR= &H08000&   for old 143 kHz max deviation    */
    

/* Filter B Coefficients (AP1)*/
/* Sample Rate: 947.368 kHz*/
/* Passband Edge: 290.000 kHz (designed)*/
/* Stopband Edge: 340.000 kHz (designed)*/
/* PB/SB Weighting: 1/10 (designed)*/
/* ------------------------------------------------------*/
/* DC gain: 0.8400 (-1.514 dB)*/
/* Maximum coefficient: 8931 (0x22e3 = 54.51 %FS)*/
/* Maximum FP coefficient: 0.545105 (1.14 binary format)*/
/* Passband Ripple: 0.229 dB (-1.508/-1.736 dB)*/
/* Stopband Rejection: 56.17 dB*/
/* ------------------------------------------------------*/



  0x0030,0x0046,0x7FD8,0x7FEB,0x004B,0x7FCA,0x7FD0,0x007F,0x7FB7,0x7FA0,
  0x00CC,0x7FA5,0x7F4D,0x0140,0x7F94,0x7EBC,0x0203,0x7F87,0x7D8A,0x03BA,
  0x7F7F,0x7907,0x0EDA,0x22E3,0x0EDA,0x7907,0x7F7F,0x03BA,0x7D8A,0x7F87,
  0x0203,0x7EBC,0x7F94,0x0140,0x7F4D,0x7FA5,0x00CC,0x7FA0,0x7FB7,0x007F,
  0x7FD0,0x7FCA,0x004B,0x7FEB,0x7FD8,0x0046,0x0030
    
    /* [TLS 20060710] Wider BW "B" filter AND dev. range for overmodulation*/
    /* NOTE:  AP_SHFT.LPFB = 6 (same 1/2 DC gain as 150kHz compared to stj original)*/
    /*        AP_SHFT.TBC  = 6 (was 7) for NEW 286 kHz max deviation:*/
    /*        AP_OUTFACTOR= &H08000&   same for NEW 286 kHz max deviation*/
    

 };




unsigned short ifd_coef8[] = /*47*/
{
    /*' --> Loading IFD Filter #8, # coeffs= 47  (ORIGINAL)*/
    0x0002,0x0002,0x7FFC,0x7FE4,0x7FC8,0x7FC6,0x000E,0x008E,
    0x00E0,0x0074,0x7F2E,0x7DE4,0x7E0E,0x0074,0x03DA,0x052A,
    0x01B0,0x7A42,0x745C,0x7764,0x0738,0x1FF2,0x36BE,0x3FFE,
    0x36BE,0x1FF2,0x0738,0x7764,0x745C,0x7A42,0x01B0,0x052A,
    0x03DA,0x0074,0x7E0E,0x7DE4,0x7F2E,0x0074,0x00E0,0x008E,
    0x000E,0x7FC6,0x7FC8,0x7FE4,0x7FFC,0x0002,0x0002
};

unsigned short ifd_coef6_alt[] = /*47*/
{   /* *** Designed for Korean A2 carrier mode*/
    /* IFD Audio Path "TBA" Filter Coeffs. (Filter #6/#9)*/
    /* Passband: 243   kHz (+/- 0.0138 dB)*/
    /* Stopband: 428.5 kHz ( < -70 dB)*/
    /* Sample Rate: 54 MHz/24 = 2.25 MHz*/
    /* Coefficient Sum = 55079, Max = 16031 (15-bit coeffs.)*/

    0x000E,0x0012,0x7FFC,0x7FC3,0x7F8E,0x7FA8,0x0038,0x00F7,0x012F,0x0049,
    0x7E8C,0x7D4E,0x7E1E,0x0142,0x04C0,0x0547,0x00CA,0x7917,0x7403,0x7844,
    0x0893,0x2089,0x3603,0x3E9F,0x3603,0x2089,0x0893,0x7844,0x7403,0x7917,
    0x00CA,0x0547,0x04C0,0x0142,0x7E1E,0x7D4E,0x7E8C,0x0049,0x012F,0x00F7,
    0x0038,0x7FA8,0x7F8E,0x7FC3,0x7FFC,0x0012,0x000E
};

unsigned short ifd_coef7_A2_LPFB_Medium[] = /*47      added [SandeepB 20070530] */
{   


	/**/
/* IFD Audio Path Filter "B" (Filter #7,#10)*/
/* Sample Rate: 750.000 kHz*/
/* Passband Edge: 150.0 kHz*/
/* Stopband Edge: 200.0 kHz*/
/* ------------------------------------------------------*/
/* DC gain: 1.6808 (4.510 dB)*/
/* Coefficient Sum = 27538*/
/* Maximum coefficient: 12682 (0x318a = 77.40 %FS)*/
/* Maximum FP coefficient: 0.774048 (1.14 binary format)*/
/* Passband Ripple: 0.090 dB (4.598/4.508 dB)*/
/* Stopband Rejection: 64.27 dB*/
/* ------------------------------------------------------*/
   0x0007,0x7FE3,0x7FB7,0x7FE2,0x0054,0x004A,0x7F96,0x7F64,0x0068,0x010F,
   0x7FC4,0x7E61,0x7FCC,0x0244,0x0108,0x7D11,0x7D8B,0x038F,0x04F4,0x7BEF,
   0x75C3,0x0467,0x21FD,0x318A,0x21FD,0x0467,0x75C3,0x7BEF,0x04F4,0x038F,
   0x7D8B,0x7D11,0x0108,0x0244,0x7FCC,0x7E61,0x7FC4,0x010F,0x0068,0x7F64,
   0x7F96,0x004A,0x0054,0x7FE2,0x7FB7,0x7FE3,0x0007
};

unsigned short ifd_coef7_alt[] = /*47      added [SandeepB 20070530] */
{   

/*  Filter B Coefficients (AP2)*/
/*  Sample Rate: 750.000 kHz*/
/*  Passband Edge: 100.000 kHz (designed)*/
/*  Stopband Edge: 150.000 kHz (designed)*/
/*  PB/SB Weighting: 1/10 (designed)*/
/*  ------------------------------------------------------*/
/*  DC gain: 1.6810 (4.511 dB)*/
/*  Maximum coefficient: 8899 (0x22c3 = 54.32 %FS)*/
/*  Maximum FP coefficient: 0.543152 (1.14 binary format)*/
/*  Passband Ripple: 0.096 dB (4.512/4.416 dB)*/
/*  Stopband Rejection: 62.69 dB*/
/*  ------------------------------------------------------*/


 0x7FF8,0x000E,0x0035,0x004E,0x0026,0x7FBD,0x7F6B,0x7FA1,0x0067,0x0116,
 0x00D0,0x7F74,0x7E20,0x7E69,0x00B0,0x032C,0x030A,0x7F32,0x7A45,0x7988,
 0x00E1,0x0EF4,0x1CED,0x22C3,0x1CED,0x0EF4,0x00E1,0x7988,0x7A45,0x7F32,
 0x030A,0x032C,0x00B0,0x7E69,0x7E20,0x7F74,0x00D0,0x0116,0x0067,0x7FA1,
 0x7F6B,0x7FBD,0x0026,0x004E,0x0035,0x000E,0x7FF8

};

unsigned short ifd_coef7_A2_LPFB_Wide[] = /*47     added [SandeepB 20070530] */
{   

/**/
/* IFD Audio Path Filter "B" (Filter #7,#10)*/
/* Sample Rate: 750.000 kHz*/
/* Passband Edge: 150.0 kHz*/
/* Stopband Edge: 200.0 kHz*/
/* ------------------------------------------------------*/
/* DC gain: 1.6808 (4.510 dB)*/
/* Coefficient Sum = 27538*/
/* Maximum coefficient: 12682 (0x318a = 77.40 %FS)*/
/* Maximum FP coefficient: 0.774048 (1.14 binary format)*/
/* Passband Ripple: 0.090 dB (4.598/4.508 dB)*/
/* Stopband Rejection: 64.27 dB*/
/* ------------------------------------------------------*/


   0x0007,0x7FE3,0x7FB7,0x7FE2,0x0054,0x004A,0x7F96,0x7F64,0x0068,0x010F,
   0x7FC4,0x7E61,0x7FCC,0x0244,0x0108,0x7D11,0x7D8B,0x038F,0x04F4,0x7BEF,
   0x75C3,0x0467,0x21FD,0x318A,0x21FD,0x0467,0x75C3,0x7BEF,0x04F4,0x038F,
   0x7D8B,0x7D11,0x0108,0x0244,0x7FCC,0x7E61,0x7FC4,0x010F,0x0068,0x7F64,
   0x7F96,0x004A,0x0054,0x7FE2,0x7FB7,0x7FE3,0x0007
      

   /* RESERVED FOR FUTURE A2 OVERMODULATION (NEED TO SCALE BOTH FILTERS)!!!  [TLS 20060710]*/
   /* !!!Remember DC gain of this filter is 1/4 of the original filter so AP_SHFT.LPFB= 7*/
  /*  Remember IFD_CORE_A2.AP_SHFT= 0x0755 for both the normal and wide band filters added [SandeepB 20070530] */
   
    
};

unsigned short ifd_coef7_BTSC_LPFB_Medium[] = /*47   added [SandeepB 20090215] */
{

/*
// "Bcm3560_Bx_IFD_Audio_FiltB__US_260k_asHEX.txt"
//
// IFD Audio Path Filter "B" (Filter #7,#10)
// Sample Rate: 947.368 kHz
// Passband Edge: 260.0 kHz
// Stopband Edge: 310.0 kHz
// ------------------------------------------------------
// DC gain: 1.6807 (4.510 dB)
// Coefficient Sum = 27537
// Maximum coefficient: 16127 (0x3eff = 98.43 %FS)
// Maximum FP coefficient: 0.984314 (1.14 binary format)
// Passband Ripple: 0.247 dB (4.511/4.264 dB)
// Stopband Rejection: 56.27 dB
// ------------------------------------------------------
*/

   0x7FED,0x0067,0x00A2,0x7FCD,0x7F95,0x009D,0x002E,0x7F05,0x0065,0x0118,
   0x7EBD,0x7F56,0x022A,0x7F77,0x7D62,0x0284,0x01F6,0x7B01,0x00A2,0x0775,
   0x7906,0x76BA,0x203C,0x3EFF,0x203C,0x76BA,0x7906,0x0775,0x00A2,0x7B01,
   0x01F6,0x0284,0x7D62,0x7F77,0x022A,0x7F56,0x7EBD,0x0118,0x0065,0x7F05,
   0x002E,0x009D,0x7F95,0x7FCD,0x00A2,0x0067,0x7FED
};

unsigned short ifd_coef8_alt[] = /*47*/
{   /* *** Designed for Korean A2 carrier mode*/
    /* IFD Audio Path TBC Filter Coeffs. (Filter #8/#11)*/
    /* Passband:  56 kHz (+/- 0.0138 dB)*/
    /* Stopband: 125 kHz ( < -70 dB)*/
    /* Sample Rate: 750 kHz (before decimation by 3 to 250 kHz output rate)*/
    /* Coefficient Sum = 55080, Max = 12782 (15-bit coeffs.)*/

    0x7FFD,0x0005,0x0019,0x0036,0x0048,0x002D,0x7FD2,0x7F49,0x7EDF,0x7F03,
    0x7FFA,0x0193,0x0308,0x0336,0x0146,0x7D63,0x7926,0x7738,0x7A40,0x037A,
    0x11B0,0x2141,0x2D5D,0x31EE,0x2D5D,0x2141,0x11B0,0x037A,0x7A40,0x7738,
    0x7926,0x7D63,0x0146,0x0336,0x0308,0x0193,0x7FFA,0x7F03,0x7EDF,0x7F49,
    0x7FD2,0x002D,0x0048,0x0036,0x0019,0x0005,0x7FFD
};

unsigned short ifd_coef9[] = /*47*/
{   /* *** Designed for Korean A2 carrier mode*/
    /* IFD Audio Path "TBA" Filter Coeffs. (Filter #6/#9)*/
    /* Passband: 243   kHz (+/- 0.0138 dB)*/
    /* Stopband: 428.5 kHz ( < -70 dB)*/
    /* Sample Rate: 54 MHz/24 = 2.25 MHz*/
    /* Coefficient Sum = 55079, Max = 16031 (15-bit coeffs.)*/

    0x000E,0x0012,0x7FFC,0x7FC3,0x7F8E,0x7FA8,0x0038,0x00F7,0x012F,0x0049,
    0x7E8C,0x7D4E,0x7E1E,0x0142,0x04C0,0x0547,0x00CA,0x7917,0x7403,0x7844,
    0x0893,0x2089,0x3603,0x3E9F,0x3603,0x2089,0x0893,0x7844,0x7403,0x7917,
    0x00CA,0x0547,0x04C0,0x0142,0x7E1E,0x7D4E,0x7E8C,0x0049,0x012F,0x00F7,
    0x0038,0x7FA8,0x7F8E,0x7FC3,0x7FFC,0x0012,0x000E
};

unsigned short ifd_coef10[] = /*47*/
{   /* *** Designed for Korean A2 carrier mode*/
    /* IFD Audio Path "LPFB" Filter Coeffs. (Filter #7/#10)*/
    /* Passband:  83 kHz (+/- 0.0138 dB)*/
    /* Stopband: 141 kHz ( < -65 dB)*/
    /* Sample Rate: 2.25 MHz/3 = 750 kHz (before CORDIC)*/
    /* Coefficient Sum = 55077, Max = 16041 (15-bit coeffs.)*/

    0x0012,0x0016,0x7FF9,0x7FB5,0x7F7A,0x7F9A,0x003F,0x0110,0x0149,0x004E,
    0x7E74,0x7D29,0x7E09,0x014E,0x04E3,0x0565,0x00CD,0x7900,0x73E7,0x7839,
    0x089B,0x209A,0x3610,0x3EA9,0x3610,0x209A,0x089B,0x7839,0x73E7,0x7900,
    0x00CD,0x0565,0x04E3,0x014E,0x7E09,0x7D29,0x7E74,0x004E,0x0149,0x0110,
    0x003F,0x7F9A,0x7F7A,0x7FB5,0x7FF9,0x0016,0x0012
};

unsigned short ifd_coef10_NICAM_Nyq_Type0[] = /*47*/
{   /* *** Designed for NICAM with 50% alpha*/
    /* IFD Audio Path "LPFB" Filter Coeffs. (Filter #10)*/
    /* Passband:  ??*/
    /* Stopband:  ??*/
    /* Sample Rate: (54.0 MHz/(18+1)) / 3  = 947.368 kHz (before CORDIC)*/
    /* Coefficient Sum = ??*/

    0xffd4,0xff5a,0x003c,0x001a,0xffd6,0xffdc,0x003a,0x001e,0xffb8,0xfffe,
    0x006e,0xffe6,0xff58,0x002a,0x00be,0xff78,0xff30,0x0202,0x0238,0xfb28,
	0xf78e,0x07dc,0x268e,0x369c,0x268e,0x07dc,0xf78e,0xfb28,0x0238,0x0202,
	0xff30,0xff78,0x00be,0x002a,0xff58,0xffe6,0x006e,0xfffe,0xffb8,0x001e,
	0x003a,0xffdc,0xffd6,0x001a,0x003c,0xff5a,0xffd4
};

unsigned short ifd_coef10_NICAM_Nyq_Type1[] = /*47*/
{   /* *** Designed for NICAM with 100% alpha*/
    /* IFD Audio Path "LPFB" Filter Coeffs. (Filter #10)*/
    /* Passband:  ??*/
    /* Stopband:  ??*/
    /* Sample Rate: (54.0 MHz/(18+1)) / 3  = 947.368 kHz (before CORDIC)*/
    /* Coefficient Sum = ??*/

    0x000c,0x004e,0xffe6,0x000a,0x0008,0xffea,0x001c,0xffee,0xfffc,0x001e,
	0xffd0,0x002a,0xfff8,0xffd2,0x005e,0xff96,0x0032,0x0056,0xfee4,0x01c8,
	0xfe76,0xfe8e,0x221c,0x3f3e,0x221c,0xfe8e,0xfe76,0x01c8,0xfee4,0x0056,
	0x0032,0xff96,0x005e,0xffd2,0xfff8,0x002a,0xffd0,0x001e,0xfffc,0xffee,
	0x001c,0xffea,0x0008,0x000a,0xffe6,0x004e,0x000c
};


unsigned short ifd_coef11[] = /*47*/
{   /* *** Designed for Korean A2 carrier mode*/
    /* IFD Audio Path TBC Filter Coeffs. (Filter #8/#11)*/
    /* Passband:  56 kHz (+/- 0.0138 dB)*/
    /* Stopband: 125 kHz ( < -70 dB)*/
    /* Sample Rate: 750 kHz (before decimation by 3 to 250 kHz output rate)*/
    /* Coefficient Sum = 55080, Max = 12782 (15-bit coeffs.)*/

    0x7FFD,0x0005,0x0019,0x0036,0x0048,0x002D,0x7FD2,0x7F49,0x7EDF,0x7F03,
    0x7FFA,0x0193,0x0308,0x0336,0x0146,0x7D63,0x7926,0x7738,0x7A40,0x037A,
    0x11B0,0x2141,0x2D5D,0x31EE,0x2D5D,0x2141,0x11B0,0x037A,0x7A40,0x7738,
    0x7926,0x7D63,0x0146,0x0336,0x0308,0x0193,0x7FFA,0x7F03,0x7EDF,0x7F49,
    0x7FD2,0x002D,0x0048,0x0036,0x0019,0x0005,0x7FFD
};

