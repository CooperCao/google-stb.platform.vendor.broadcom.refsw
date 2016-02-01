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
*   Module name: Mixer
*   This file lists all Soft coefficients to be programmed at the FMM init for 
*   Mixer initialization.
* 
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"
#include "brap_priv.h"


const uint32_t
BRAP_MIXER_P_Soft_Coeff_Array [BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFCi_ARRAY_END+8+1] =
{
    /* Soft Coeff Array elements */
    0x007cbc6a,    0x0075c28f,    0x00703127,    0x00684189,    0x0063d70a,
    0x005f4bc7,    0x005b22d1,    0x00574bc7,    0x0053b646,    0x00508312,
    0x004d8106,    0x004ae148,    0x00479db2,    0x00453f7d,    0x0043126f,
    0x00410625,
    /* Soft Coeff A to F and 0.5, 0.8 and 1.25 */
    0x003d1eb8,    0x005a3d71,    0x00666666,    0x0040cccd,    0x0019eb85,    
    0x00400000,    0x00666666,    0x00a00000
};
