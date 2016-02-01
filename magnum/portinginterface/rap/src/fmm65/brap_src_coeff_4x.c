/***************************************************************************
*     Copyright (c) 2004-2008, Broadcom Corporation
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
*   Module name: SRC
*   This file lists all 4x coefficients to be programmed at the FMM init for 
*   4x SRC conversion.
* 
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "brap.h"
#include "brap_priv.h"

const uint32_t 
#if (BRAP_3548_FAMILY == 1)
BRAP_SRC_P_4x_Coeff_Array [128] =
#else
BRAP_SRC_P_4x_Coeff_Array [BCHP_AUD_FMM_SRC_CTRL0_COEFF4X_i_ARRAY_END + 1] =
#endif
{
    0x0000002f,     0x000000cf,    0x00000247,    0x000004eb,    0x000008b5,
    0x00000cfd,     0x00001056,    0x000010cc,    0x00000c98,    0x0000030a,
    0x00fff560,     0x00ffe6ec,    0x00ffdc48,    0x00ffd99f,    0x00ffe0b2,
    0x00ffef7d,     0x0000006b,    0x00000c56,    0x00000db3,    0x00000387,
    0x00fff281,     0x00ffe353,    0x00ffdeae,    0x00ffe8a3,    0x00fffdd1,
    0x0000143f,     0x00001ff1,    0x00001949,    0x000001dc,    0x00ffe4d4,
    0x00ffd1f4,     0x00ffd557,    0x00fff014,    0x0000162a,    0x0000338d,
    0x0000368e,     0x00001a3a,    0x00ffeadb,    0x00ffc129,    0x00ffb5b0,
    0x00ffd2bb,     0x00000c9e,    0x000045b8,    0x00005d4a,    0x000041ee,
    0x00fffd76,     0x00ffb278,    0x00ff8b4a,    0x00ffa278,    0x00fff1a8,
    0x00005186,     0x00008cc7,    0x00007d64,    0x0000243d,    0x00ffacd6,
    0x00ff58c9,     0x00ff5c32,    0x00ffbe1f,    0x00004fcb,    0x0000c20e,
    0x0000cfdd,     0x0000673f,    0x00ffb8df,    0x00ff2289,    0x00fefd5d,
    0x00ff69bf,     0x0000370d,    0x0000f7ef,    0x00013bd0,    0x0000cfbb,
    0x00ffe1bc,     0x00feef51,    0x00fe8432,    0x00feeab5,    0x00fffab6,
    0x0001264c,     0x0001c2af,    0x0001687d,    0x000035c2,    0x00fec89b,
    0x00fdef2c,     0x00fe3482,    0x00ff89fd,    0x0001420c,    0x00026688,
    0x000240d6,     0x0000c96c,    0x00febbce,    0x00fd3b5c,    0x00fd33bb,
    0x00fecb46,     0x00013afd,    0x00032c67,    0x00037306,    0x0001be6f,
    0x00fedd36,     0x00fc5fde,    0x00fbc2c8,    0x00fd8fab,    0x0000f661,
    0x000423f7,     0x00053808,    0x00035a1e,    0x00ff524f,    0x00fb4092,
    0x00f985b0,     0x00fb6906,    0x00003afd,    0x00058103,    0x00082f36,
    0x00065aee,     0x00007c1f,    0x00f978d1,    0x00f54da7,    0x00f6e830,
    0x00fe4ce3,     0x00081c26,    0x000eec56,    0x000dffda,    0x000414cb,
    0x00f4d5c7,     0x00e7fb75,    0x00e63ea5,    0x00f555c4,    0x0014cf15,
    0x003d58b4,     0x006300da,    0x0079a156
};

/* End of File */

