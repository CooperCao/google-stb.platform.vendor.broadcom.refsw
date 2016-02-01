/******************************************************************************
 *    (c)2011-2013 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/

#include "bstd.h"
#include "bads_def.h"
#include "bads_coef.h"

/***********************************************************************************************************************/
/*the Baud Rates are broken up into 33 regions                                                                         */
/*The table was created using a baud spacing of 0.0602%, nominal values are within +-3.01%                             */ 
/*This is so all of the loops can be lookuptables																	                                     */
/***********************************************************************************************************************/
const uint32_t BaudRates_TBL_u32[NUM_BAUD_RATES] =
{
	7300000,              /*upperBR= 7300000   centerBR= 7080157   lowerBR= 6860314*/
	6860314,              /*upperBR= 6860314   centerBR= 6653713   lowerBR= 6447111*/
	6447111,              /*upperBR= 6447111   centerBR= 6252954   lowerBR= 6058796*/
	6058796,              /*upperBR= 6058796   centerBR= 5876333   lowerBR= 5693869*/
	5693869,              /*upperBR= 5693869   centerBR= 5522396   lowerBR= 5350922*/
	5350922,              /*upperBR= 5350922   centerBR= 5189777   lowerBR= 5028631*/
	5028631,              /*upperBR= 5028631   centerBR= 4877192   lowerBR= 4725752*/
	4725752,              /*upperBR= 4725752   centerBR= 4583434   lowerBR= 4441116*/
	4441116,              /*upperBR= 4441116   centerBR= 4307370   lowerBR= 4173624*/
	4173624,              /*upperBR= 4173624   centerBR= 4047934   lowerBR= 3922243*/
	3922243,              /*upperBR= 3922243   centerBR= 3804123   lowerBR= 3686003*/
	3686003,              /*upperBR= 3686003   centerBR= 3574998   lowerBR= 3463992*/
	3463992,              /*upperBR= 3463992   centerBR= 3359673   lowerBR= 3255353*/
	3255353,              /*upperBR= 3255353   centerBR= 3157317   lowerBR= 3059280*/
	3059280,              /*upperBR= 3059280   centerBR= 2967149   lowerBR= 2875017*/
	2875017,              /*upperBR= 2875017   centerBR= 2788435   lowerBR= 2701852*/
	2701852,              /*upperBR= 2701852   centerBR= 2620485   lowerBR= 2539117*/
	2539117,              /*upperBR= 2539117   centerBR= 2462651   lowerBR= 2386184*/
	2386184,              /*upperBR= 2386184   centerBR= 2314323   lowerBR= 2242462*/
	2242462,              /*upperBR= 2242462   centerBR= 2174929   lowerBR= 2107396*/
	2107396,              /*upperBR= 2107396   centerBR= 2043931   lowerBR= 1980466*/
	1980466,              /*upperBR= 1980466   centerBR= 1920824   lowerBR= 1861181*/
	1861181,              /*upperBR= 1861181   centerBR= 1805131   lowerBR= 1749080*/
	1749080,              /*upperBR= 1749080   centerBR= 1696406   lowerBR= 1643731*/
	1643731,              /*upperBR= 1643731   centerBR= 1594230   lowerBR= 1544728*/
	1544728,              /*upperBR= 1544728   centerBR= 1498208   lowerBR= 1451688*/
	1451688,              /*upperBR= 1451688   centerBR= 1407970   lowerBR= 1364252*/
	1364252,              /*upperBR= 1364252   centerBR= 1323167   lowerBR= 1282082*/
	1282082,              /*upperBR= 1282082   centerBR= 1243472   lowerBR= 1204861*/
	1204861,              /*upperBR= 1204861   centerBR= 1168576   lowerBR= 1132291*/
	1132291,              /*upperBR= 1132291   centerBR= 1098192   lowerBR= 1064092*/
	1064092,              /*upperBR= 1064092   centerBR= 1032046   lowerBR= 1000000*/
	1000000              
};

/*Loop is scaled from a 5.38 MBaud phase loop parameters                 */
/*PhaseLoopAcqCoeffs are calculated with Fn = 22275 Hz and zeta = 1.4568 above  3 MBaud*/
/*PhaseLoopAcqCoeffs are calculated with Fn = 10000 Hz and zeta = 1.4568  below 3 MBaud*/
const uint32_t PhaseLoopAcqCoeffs_TBL_u32[NUM_BAUD_RATES] =
{
0xFDA6FFBE,      /*centerBR= 7080157*/
0xFD80FFB6,      /*centerBR= 6653713*/
0xFD45FFA8,      /*centerBR= 6252954*/
0xFD19FF9C,      /*centerBR= 5876333*/
0xFCE9FF8F,      /*centerBR= 5522396*/
0xFCB6FF80,      /*centerBR= 5189777*/
0xFC80FF6F,      /*centerBR= 4877192*/
0xFC47FF5C,      /*centerBR= 4583434*/
0xFC0AFF47,      /*centerBR= 4307370*/
0xFBC9FF2E,      /*centerBR= 4047934*/
0xFB84FF13,      /*centerBR= 3804123*/
0xFB3AFEF3,      /*centerBR= 3574998*/
0xFAECFED0,      /*centerBR= 3359673*/
0xFA99FEA8,      /*centerBR= 3157317*/
0xFD8FFFB9,        /*centerBR= 2967149*/
0xFD67FFB0,        /*centerBR= 2788435*/
0xFD3CFFA5,        /*centerBR= 2620485*/
0xFD0FFF99,        /*centerBR= 2462651*/
0xFCDFFF8C,        /*centerBR= 2314323*/
0xFCACFF7C,        /*centerBR= 2174929*/
0xFC75FF6B,        /*centerBR= 2043931*/
0xFC3BFF57,        /*centerBR= 1920824*/
0xFBFDFF41,        /*centerBR= 1805131*/
0xFBBBFF28,        /*centerBR= 1696406*/
0xFB75FF0C,        /*centerBR= 1594230*/
0xFB2BFEEB,        /*centerBR= 1498208*/
0xFADBFEC7,        /*centerBR= 1407970*/
0xFA87FE9D,        /*centerBR= 1323167*/
0xFA2DFE6E,        /*centerBR= 1243472*/
0xF9CEFE39,        /*centerBR= 1168576*/
0xF968FDFD,        /*centerBR= 1098192*/
0xF8FCFDB9,        /*centerBR= 1032046*/
0xF889FD6C,        /*centerBR= 1000000*/
};

/*Loop is scaled from a 5.38 MBaud phase loop parameters                 */
/*PhaseLoopTrkCoeffs are calculated with zeta = 0.69 and Fn = 28884 Hz  above  3 MBaud */
/*PhaseLoopTrkCoeffs are calculated with zeta = 1.00 and Fn = 20000 Hz  below 3 MBaud */
const uint32_t PhaseLoopTrkCoeffs_TBL_u32[NUM_BAUD_RATES] =
{
0xFE85FF8C,      /*centerBR= 7080157*/
0xFE6CFF7D,      /*centerBR= 6653713*/
0xFE53FF6C,      /*centerBR= 6252954*/
0xFE37FF58,      /*centerBR= 5876333*/
0xFE1AFF42,      /*centerBR= 5522396*/
0xFDFBFF29,      /*centerBR= 5189777*/
0xFDDAFF0D,      /*centerBR= 4877192*/
0xFDB6FEED,      /*centerBR= 4583434*/
0xFD91FEC8,      /*centerBR= 4307370*/
0xFD69FE9F,      /*centerBR= 4047934*/
0xFD3EFE70,      /*centerBR= 3804123*/
0xFD11FE3C,      /*centerBR= 3574998*/
0xFCE1FE00,      /*centerBR= 3359673*/
0xFCAEFDBC,      /*centerBR= 3157317*/
0xFC8CFED4,      /*centerBR= 2967149*/
0xFC53FEAC,      /*centerBR= 2788435*/
0xFC17FE80,      /*centerBR= 2620485*/
0xFBD6FE4D,      /*centerBR= 2462651*/
0xFB92FE13,      /*centerBR= 2314323*/
0xFB4AFDD2,      /*centerBR= 2174929*/
0xFAFCFD88,      /*centerBR= 2043931*/
0xFAAAFD35,      /*centerBR= 1920824*/
0xFA53FCD6,      /*centerBR= 1805131*/
0xF9F5FC6B,      /*centerBR= 1696406*/
0xF992FBF2,      /*centerBR= 1594230*/
0xF929FB69,      /*centerBR= 1498208*/
0xF8B9FACD,      /*centerBR= 1407970*/
0xF841FA1D,      /*centerBR= 1323167*/
0xF7C2F956,      /*centerBR= 1243472*/
0xF73BF875,      /*centerBR= 1168576*/
0xF6ABF775,      /*centerBR= 1098192*/
0xF5C1F5B3,      /*centerBR= 1000000*/
0xF5C1F5B3
};

/*Loop is scaled from a 5.38 MBaud phase loop parameters                 */
/*PhaseLoopTrkBurstModeCoeffs are calculated with zeta = 0.442 and Fn = 29560 Hz */
const uint32_t PhaseLoopTrkBurstModeCoeffs_TBL_u32[NUM_BAUD_RATES] =
{
0xFF07FF87,      /*centerBR= 7080157*/
0xFEF7FF77,      /*centerBR= 6653713*/
0xFEE6FF65,      /*centerBR= 6252954*/
0xFED4FF50,      /*centerBR= 5876333*/
0xFEC1FF39,      /*centerBR= 5522396*/
0xFEACFF1F,      /*centerBR= 5189777*/
0xFE97FF01,      /*centerBR= 4877192*/
0xFE7FFEDF,      /*centerBR= 4583434*/
0xFE67FEB9,      /*centerBR= 4307370*/
0xFE4DFE8E,      /*centerBR= 4047934*/
0xFE31FE5D,      /*centerBR= 3804123*/
0xFE13FE26,      /*centerBR= 3574998*/
0xFDF4FDE7,      /*centerBR= 3359673*/
0xFDD2FDA0,      /*centerBR= 3157317*/
0xFDAEFD50,      /*centerBR= 2967149*/
0xFD88FCF5,      /*centerBR= 2788435*/
0xFD60FC8E,      /*centerBR= 2620485*/
0xFD35FC1A,      /*centerBR= 2462651*/
0xFD07FB96,      /*centerBR= 2314323*/
0xFCD6FB00,      /*centerBR= 2174929*/
0xFCA2FA57,      /*centerBR= 2043931*/
0xFC6BF997,      /*centerBR= 1920824*/
0xFC30F8BE,      /*centerBR= 1805131*/
0xFBF2F7C9,      /*centerBR= 1696406*/
0xFBAFF6B2,      /*centerBR= 1594230*/
0xFB69F577,      /*centerBR= 1498208*/
0xFB1DF413,      /*centerBR= 1407970*/
0xFACDF27F,      /*centerBR= 1323167*/
0xFA78F0B6,      /*centerBR= 1243472*/
0xFA1DEEB0,      /*centerBR= 1168576*/
0xF9BDEC66,      /*centerBR= 1098192*/
0xF956E9CE,      /*centerBR= 1032046*/
0xF923E85C       /*centerBR= 1000000*/
};

/*Loop is scaled from a 5.38 MBaud TT loop parameters         */
/*Coefficients are for TT with Fn=1710 Hz and Zeta=0.732      */
/*TL PD Gain full precision/full precision = 0.012 TLAGC = 8X */
/*For FOI Loop match FOI gain to the TT loop                  */
const uint16_t TimingLoopTrk1Coeffs_TBL_u16[NUM_BAUD_RATES] =
{
0x3634,      /*centerBR= 7080157*/
0x3634,      /*centerBR= 6653713*/
0x3634,      /*centerBR= 6252954*/
0x3634,      /*centerBR= 5876333*/
0x3634,      /*centerBR= 5522396*/
0x3634,      /*centerBR= 5189777*/
0x3634,      /*centerBR= 4877192*/
0x3634,      /*centerBR= 4583434*/
0x3634,      /*centerBR= 4307370*/
0x3634,      /*centerBR= 4047934*/
0x3533,      /*centerBR= 3804123*/
0x3533,      /*centerBR= 3574998*/
0x3533,      /*centerBR= 3359673*/
0x3533,      /*centerBR= 3157317*/
0x3533,      /*centerBR= 2967149*/
0x3533,      /*centerBR= 2788435*/
0x3533,      /*centerBR= 2620485*/
0x3533,      /*centerBR= 2462651*/
0x3533,      /*centerBR= 2314323*/
0x3533,      /*centerBR= 2174929*/
0x3533,      /*centerBR= 2043931*/
0x3533,      /*centerBR= 1920824*/
0x3432,      /*centerBR= 1805131*/
0x3432,      /*centerBR= 1696406*/
0x3432,      /*centerBR= 1594230*/
0x3432,      /*centerBR= 1498208*/
0x3432,      /*centerBR= 1407970*/
0x3432,      /*centerBR= 1323167*/
0x3432,      /*centerBR= 1243472*/
0x3432,      /*centerBR= 1168576*/
0x3432,      /*centerBR= 1098192*/
0x3432,      /*centerBR= 1032046*/
0x3432       /*centerBR= 1000000*/
};

/*Loop is scaled from a 5.38 MBaud TT loop parameters         */
/*Coefficients are for TT with Fn=426 Hz and Zeta=0.732       */
/*TL PD Gain full precision/full precision = 0.012 TLAGC = 8X */
/*For FOI Loop match FOI gain to the TT loop                  */
const uint16_t TimingLoopTrk2Coeffs_TBL_u16[NUM_BAUD_RATES] =
{
0x3330,      /*centerBR= 7080157*/   /*China CHange CAB 4/24/2013*/
0x3330,      /*centerBR= 6653713*/   /*China CHange CAB 4/24/2013*/
0x3430,      /*centerBR= 6252954*/
0x3430,      /*centerBR= 5876333*/
0x3430,      /*centerBR= 5522396*/
0x3430,      /*centerBR= 5189777*/
0x3430,      /*centerBR= 4877192*/
0x3430,      /*centerBR= 4583434*/
0x3430,      /*centerBR= 4307370*/
0x3430,      /*centerBR= 4047934*/
0x332F,      /*centerBR= 3804123*/
0x332F,      /*centerBR= 3574998*/
0x332F,      /*centerBR= 3359673*/
0x332F,      /*centerBR= 3157317*/
0x332F,      /*centerBR= 2967149*/
0x332F,      /*centerBR= 2788435*/
0x332F,      /*centerBR= 2620485*/
0x332F,      /*centerBR= 2462651*/
0x332F,      /*centerBR= 2314323*/
0x332F,      /*centerBR= 2174929*/
0x332F,      /*centerBR= 2043931*/
0x332F,      /*centerBR= 1920824*/
0x322E,      /*centerBR= 1805131*/
0x322E,      /*centerBR= 1696406*/
0x322E,      /*centerBR= 1594230*/
0x322E,      /*centerBR= 1498208*/
0x322E,      /*centerBR= 1407970*/
0x322E,      /*centerBR= 1323167*/
0x322E,      /*centerBR= 1243472*/
0x322E,      /*centerBR= 1168576*/
0x322E,      /*centerBR= 1098192*/
0x322E,      /*centerBR= 1032046*/
0x322E       /*centerBR= 1000000*/ 
};

/*Loop is scaled from a 5.38 MBaud loop parameters      */
/*Coefficients are for CFL with Fn=875 Hz and Zeta=4.18 */
const uint16_t FrequencyLoopCoeffs_TBL_u16[NUM_BAUD_RATES] =
{
0x2923,      /*centerBR= 7080157*/
0x2923,      /*centerBR= 6653713*/
0x2923,      /*centerBR= 6252954*/
0x2923,      /*centerBR= 5876333*/
0x2923,      /*centerBR= 5522396*/
0x2923,      /*centerBR= 5189777*/
0x2923,      /*centerBR= 4877192*/
0x2923,      /*centerBR= 4583434*/
0x2923,      /*centerBR= 4307370*/
0x2923,      /*centerBR= 4047934*/
0x2924,      /*centerBR= 3804123*/
0x2924,      /*centerBR= 3574998*/
0x2924,      /*centerBR= 3359673*/
0x2924,      /*centerBR= 3157317*/
0x2924,      /*centerBR= 2967149*/
0x2924,      /*centerBR= 2788435*/
0x2924,      /*centerBR= 2620485*/
0x2924,      /*centerBR= 2462651*/
0x2924,      /*centerBR= 2314323*/
0x2924,      /*centerBR= 2174929*/
0x2924,      /*centerBR= 2043931*/
0x2924,      /*centerBR= 1920824*/
0x2925,      /*centerBR= 1805131*/
0x2925,      /*centerBR= 1696406*/
0x2925,      /*centerBR= 1594230*/
0x2925,      /*centerBR= 1498208*/
0x2925,      /*centerBR= 1407970*/
0x2925,      /*centerBR= 1323167*/
0x2925,      /*centerBR= 1243472*/
0x2925,      /*centerBR= 1168576*/
0x2925,      /*centerBR= 1098192*/
0x2925,      /*centerBR= 1032046*/
0x2925
};

/*Loop is scaled from a 5.38 MBaud loop parameters       */
/*Coefficients are for CFL with Fn=437 Hz and Zeta=0.523 */
const uint16_t FrequencyLoopBurstModeCoeffs_TBL_u16[NUM_BAUD_RATES] =
{
0x2521,      /*centerBR= 7080157*/
0x2521,      /*centerBR= 6653713*/
0x2521,      /*centerBR= 6252954*/
0x2521,      /*centerBR= 5876333*/
0x2521,      /*centerBR= 5522396*/
0x2521,      /*centerBR= 5189777*/
0x2521,      /*centerBR= 4877192*/
0x2521,      /*centerBR= 4583434*/
0x2521,      /*centerBR= 4307370*/
0x2521,      /*centerBR= 4047934*/
0x2521,      /*centerBR= 3804123*/
0x2522,      /*centerBR= 3574998*/
0x2522,      /*centerBR= 3359673*/
0x2522,      /*centerBR= 3157317*/
0x2522,      /*centerBR= 2967149*/
0x2522,      /*centerBR= 2788435*/
0x2522,      /*centerBR= 2620485*/
0x2522,      /*centerBR= 2462651*/
0x2522,      /*centerBR= 2314323*/
0x2522,      /*centerBR= 2174929*/
0x2522,      /*centerBR= 2043931*/
0x2522,      /*centerBR= 1920824*/
0x2523,      /*centerBR= 1805131*/
0x2523,      /*centerBR= 1696406*/
0x2523,      /*centerBR= 1594230*/
0x2523,      /*centerBR= 1498208*/
0x2523,      /*centerBR= 1407970*/
0x2523,      /*centerBR= 1323167*/
0x2523,      /*centerBR= 1243472*/
0x2523,      /*centerBR= 1168576*/
0x2523,      /*centerBR= 1098192*/
0x2523,      /*centerBR= 1032046*/
0x2523       /*centerBR= 1000000*/
};

const PhaseLoopSweep_FFT_t PhaseLoopSweep_FFT_TBL_struct[NUM_BAUD_RATES] =
{
{0x00B, 0x3FFFEE99, 5},
{0x00B, 0x3FFFEFA5, 5},
{0x00D, 0x3FFFF0A1, 5},
{0x00D, 0x3FFFF18F, 5},
{0x00F, 0x3FFFF26D, 5},
{0x00F, 0x3FFFF33F, 5},
{0x011, 0x3FFFF403, 5},
{0x011, 0x3FFFF4BC, 5},
{0x013, 0x3FFFF56A, 5},
{0x013, 0x3FFFF60D, 5},
{0x015, 0x3FFFF6A7, 5},
{0x015, 0x3FFFF737, 5},
{0x018, 0x3FFFF7BE, 5},
{0x018, 0x3FFFF83E, 5},
{0x01B, 0x3FFFF8B5, 5},
{0x01B, 0x3FFFF926, 5},
{0x01F, 0x3FFFF98F, 5},
{0x01F, 0x3FFFF9F3, 5},
{0x023, 0x3FFFFA50, 5},
{0x023, 0x3FFFFAA8, 5},
{0x027, 0x3FFFFAFA, 5},
{0x027, 0x3FFFFB48, 5},
{0x02C, 0x3FFFFB91, 5},
{0x02C, 0x3FFFFBD5, 5},
{0x032, 0x3FFFFC15, 5},
{0x032, 0x3FFFFC52, 5},
{0x038, 0x3FFFFC8A, 5},
{0x038, 0x3FFFFCC0, 5},
{0x040, 0x3FFFFCF2, 5},
{0x040, 0x3FFFFD21, 5},
{0x048, 0x3FFFFD4D, 5},
{0x048, 0x3FFFFD77, 5},
{0x048, 0x3FFFFD9E, 5}
};

/**********************************************************************************************/
/* These are not used as interrupts, just as placeholders *************************************/
/**********************************************************************************************/
#if (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_6)  /*SNR Equation changed in core 9.6*/
	const uint32_t SNRLTHRESH_TBL_u32[NUM_QAM_MODES] =
	{
	0x000006,		/*16Qam Low threshold approx. 66 dB*/	
	0x000003,		/*32Qam Low threshold approx. 66 dB*/	
	0x000006,		/*64Qam Low threshold approx. 66 dB*/	
	0x000003,		/*128Qam Low threshold approx. 66 dB*/	
	0x000006,		/*256Qam Low threshold approx. 66 dB*/	
	0x000003,		/*512Qam Low threshold approx. 66 dB*/	
	0x000006,		/*1024Qam Low threshold approx. 66 dB*/	
	};
	const uint32_t SNRHTHRESH_TBL_u32[NUM_QAM_MODES] =
	{
	0x12014D,	/*16Qam High Threshold 13 dB*/
	0x0482AF,	/*32Qam High Threshold 16 dB*/
	0x04BF6D,	/*64Qam High Threshold 19 dB*/
	0x012971,	/*128Qam High Threshold 22 dB*/
	0x01349D,	/*256Qam High Threshold 25 dB*/
	0x004B75,	/*512Qam High Threshold 28 dB*/
	0x004DBB,	/*1024Qam High Threshold 31 dB*/
	};
#else
	const uint32_t SNRLTHRESH_TBL_u32[NUM_QAM_MODES] =
	{
	0x003A9,		/*16Qam Low threshold approx. 66 dB*/	
	0x00296,		/*32Qam Low threshold approx. 66 dB*/	
	0x003C0,		/*64Qam Low threshold approx. 66 dB*/	
	0x0029F,		/*128Qam Low threshold approx. 66 dB*/	
	0x003C6,		/*256Qam Low threshold approx. 66 dB*/	
	0x002A1,		/*512Qam Low threshold approx. 66 dB*/	
	0x003C7,		/*1024Qam Low threshold approx. 66 dB*/	
	};
	const uint32_t SNRHTHRESH_TBL_u32[NUM_QAM_MODES] =
	{
	0x66401,	/*16Qam High Threshold 13 dB*/
	0x332F9,	/*32Qam High Threshold 16 dB*/
	0x34831,	/*64Qam High Threshold 19 dB*/
	0x19F8E,	/*128Qam High Threshold 22 dB*/
	0x1A798,	/*256Qam High Threshold 25 dB*/
	0x0D0E7,	/*512Qam High Threshold 28 dB*/
	0x0D48B,	/*1024Qam High Threshold 31 dB*/
	};
#endif






const FEC_t AnnexA_FEC_TBL_struct[NUM_QAM_MODES] =
{
	{0x004A000C, 0xFF800620, 0x08050031, 0x0000002F, 0x0000006A},	/* Annex A 16QAM*/
	{0x004A000C, 0xFF800620, 0x08050041, 0x000000EB, 0x00000179},	/* Annex A 32QAM*/
	{0x004A000C, 0xFF800620, 0x08050051, 0x0000002F, 0x00000037},	/* Annex A 64QAM*/
	{0x004A000C, 0xFF800620, 0x08050061, 0x00000149, 0x0000011B},	/* Annex A 128QAM*/
	{0x004A000C, 0xFF800620, 0x08050071, 0x0000005E, 0x0000003B},	/* Annex A 256QAM*/
	{0x004A000C, 0xFF800620, 0x08050081, 0x0000002F, 0x00000015},	/* Annex A 512QAM*/
	{0x004A000C, 0xFF800620, 0x08050091, 0x000000EB, 0x00000047},	/* Annex A 1024QAM*/
};

const FEC_t AnnexB_FEC_TBL_struct[NUM_QAM_MODES] =
{
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	/* Annex B 16QAM unsupported*/
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	/* Annex B 32QAM unsupported*/
	{0x00E00000, 0x1E01E0FF, 0x01000059, 0x00000004, 0x00000005},	/* Annex B 64QAM*/
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	/* Annex B 128QAM unsupported*/
	{0x00E00000, 0x1E01E0FF, 0x01000079, 0x00001259, 0x00000C10},	/* Annex B 256QAM*/
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	/* Annex B 512QAM unsupported*/
	{0x00E00000, 0x1E01FE01, 0x01000099, 0x00003DB7, 0x00001343},	/* Annex B 1024QAM*/
};

const uint32_t DS_EQ_CTL_TBL_u32[NUM_QAM_MODES] =
{
0x00040909,	/*16Qam DS_EQ_CTL*/
0x00040A12,	/*32Qam DS_EQ_CTL*/
0x00040B1B,	/*64Qam DS_EQ_CTL*/
0x00040C24,	/*128Qam DS_EQ_CTL*/
0x00040D2D,	/*256Qam DS_EQ_CTL*/
0x00040E36,	/*512Qam DS_EQ_CTL*/
0x00040F3F,	/*1024Qam DS_EQ_CTL*/
};
 
const uint32_t DS_EQ_CPL_TBL_u32[NUM_QAM_MODES] =
{
0xC1001001,	/*16Qam   Freq Det=On,  CPLFTHRESH=0x100,  fixed leakage, use diag points, reset CPL loop*/
0xC0401001,	/*32Qam   Freq Det=On,  CPLFTHRESH=0x040,  fixed leakage, use diag points, reset CPL loop*/
0x40001001,	/*64Qam   Freq Det=Off, CPLFTHRESH=0x000,  fixed leakage, use diag points, reset CPL loop*/
0x40001001,	/*128Qam  Freq Det=Off, CPLFTHRESH=0x000,  fixed leakage, use diag points, reset CPL loop*/
0x40001001,	/*256Qam  Freq Det=Off, CPLFTHRESH=0x000,  fixed leakage, use diag points, reset CPL loop*/
0x40001001,	/*512Qam  Freq Det=Off, CPLFTHRESH=0x000,  fixed leakage, use diag points, reset CPL loop*/
0x40001001,	/*1024Qam Freq Det=Off, CPLFTHRESH=0x000,  fixed leakage, use diag points, reset CPL loop*/
};

