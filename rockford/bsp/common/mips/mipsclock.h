/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#ifndef MIPS_CLOCK_H
#define MIPS_CLOCK_H

#if defined(MTI34KF)
    #define INPUT_CLK_CYCLES_PER_COUNT_REG_INCR 1 /* MTI-R34Kf */
#elif defined(BMIPS3300)
    #define INPUT_CLK_CYCLES_PER_COUNT_REG_INCR 2 /* MIPS3300 */    /*32 entries 32MB*/
#elif defined(BMIPS4380)
    #define	INPUT_CLK_CYCLES_PER_COUNT_REG_INCR 2 /* BMIPS-4380	*/   /*32 entries 256MB*/
#elif defined(BMIPS5000)
    #define INPUT_CLK_CYCLES_PER_COUNT_REG_INCR 8 /* BMIPS-5000 */    /*64 entries 256MB*/
#else
    #define error unknown CPU defined.
#endif

#endif
