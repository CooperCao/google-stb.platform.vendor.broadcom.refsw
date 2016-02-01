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
*   This file lists all 2x coefficients to be programmed at the FMM init for 
*   2x SRC conversion.
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
BRAP_SRC_P_2x_Coeff_Array [64] =
#else
BRAP_SRC_P_2x_Coeff_Array [BCHP_AUD_FMM_SRC_CTRL0_COEFF2X_i_ARRAY_END + 1] =
#endif
{
    0x000000cf,     0x000005a7,    0x00001198,    0x00001eb4,    0x00001c58,
    0x00000381,     0x00ffeba6,    0x00fff3ec,    0x000010b1,    0x00001208,
    0x00fff0dd,     0x00ffe699,    0x00000d4d,    0x000022c1,    0x00fff625,
    0x00ffd1f9,     0x000003d8,    0x00003af1,    0x0000057b,    0x00ffb6dd,
    0x00ffed3d,     0x00005848,    0x000024c6,    0x00ff9829,    0x00ffc3bd,
    0x0000771d,     0x000059de,    0x00ff7a91,    0x00ff818b,    0x000091be,
    0x0000aaa4,     0x00ff64f3,    0x00ff20be,    0x0000a004,    0x00011d0c,
    0x00ff60cf,     0x00fe9b22,    0x000096d9,    0x0001b7b4,    0x00ff7b0e,
    0x00fde938,     0x00006702,    0x000283b8,    0x00ffc60c,    0x00fcff25,
    0x00fff9b7,     0x000391aa,    0x00005f52,    0x00fbc45a,    0x00ff2695,
    0x000507e9,     0x000181c5,    0x00f9f9a8,    0x00fd90e2,    0x000754fd,
    0x0003cd56,     0x00f6cd23,    0x00fa0203,    0x000c3c16,    0x000a1013,
    0x00ed8c14,     0x00eb89a8,    0x00289ee4,    0x0070d56e
};

/* End of File */

