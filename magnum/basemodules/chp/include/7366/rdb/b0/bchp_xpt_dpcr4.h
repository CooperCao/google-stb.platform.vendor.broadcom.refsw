/***************************************************************************
 *     Copyright (c) 1999-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on              Mon Dec 23 13:19:23 2013
 *                 Full Compile MD5 Checksum e5d1378cc1475b750905e70cb70c73d9
 *                   (minus title and desc)  
 *                 MD5 Checksum              aa943f3142a624837db5321711723fcf
 *
 * Compiled with:  RDB Utility               combo_header.pl
 *                 RDB Parser                3.0
 *                 unknown                   unknown
 *                 Perl Interpreter          5.008008
 *                 Operating System          linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_XPT_DPCR4_H__
#define BCHP_XPT_DPCR4_H__

/***************************************************************************
 *XPT_DPCR4 - XPT DPCR4 Control Registers
 ***************************************************************************/
#define BCHP_XPT_DPCR4_PID_CH                    0x00a02200 /* Data Transport PCR PID Channel Register */
#define BCHP_XPT_DPCR4_CTRL                      0x00a02204 /* Data Transport PCR Control Register */
#define BCHP_XPT_DPCR4_INTR_STATUS_REG           0x00a02208 /* Interrupt Status Register */
#define BCHP_XPT_DPCR4_INTR_STATUS_REG_EN        0x00a0220c /* Interrupt Status Enable Register */
#define BCHP_XPT_DPCR4_STC_EXT_CTRL              0x00a02210 /* Data Transport PCR STC Extension Control Register */
#define BCHP_XPT_DPCR4_MAX_PCR_ERROR             0x00a02220 /* Data Transport PCR Max PCR Error Register */
#define BCHP_XPT_DPCR4_SEND_BASE                 0x00a02224 /* Data Transport PCR Send Base Register */
#define BCHP_XPT_DPCR4_SEND_EXT                  0x00a02228 /* Data Transport PCR Send Extension Register */
#define BCHP_XPT_DPCR4_STC_EXT_CTRL27            0x00a0222c /* Data Transport PCR STC Extension Control Register (Test Only) */
#define BCHP_XPT_DPCR4_STC_HI                    0x00a02230 /* Data Transport PCR STC MSBs Register */
#define BCHP_XPT_DPCR4_STC_LO                    0x00a02234 /* Data Transport PCR STC LSBs Register */
#define BCHP_XPT_DPCR4_PWM_CTRLVALUE             0x00a02238 /* Data Transport PCR PWM Control Value Register */
#define BCHP_XPT_DPCR4_LAST_PCR_HI               0x00a0223c /* Data Transport PCR Last PCR MSBs Register */
#define BCHP_XPT_DPCR4_LAST_PCR_LO               0x00a02240 /* Data Transport PCR Last PCR LSBs Register */
#define BCHP_XPT_DPCR4_STC_BASE_LSBS             0x00a02248 /* Data Transport PCR STC Base LSBs Register */
#define BCHP_XPT_DPCR4_PHASE_ERROR               0x00a0224c /* Timebase Last Phase Error */
#define BCHP_XPT_DPCR4_LOOP_CTRL                 0x00a02250 /* Timebase Control */
#define BCHP_XPT_DPCR4_REF_PCR_PRESCALE          0x00a02254 /* Timebase Frequency Reference Prescale Control */
#define BCHP_XPT_DPCR4_REF_PCR_INC               0x00a02258 /* Timebase Frequency Reference Increment Control */
#define BCHP_XPT_DPCR4_CENTER                    0x00a0225c /* Timebase Center Frequency */
#define BCHP_XPT_DPCR4_ACCUM_VALUE               0x00a02260 /* Timebase Loop Filter Integrator */
#define BCHP_XPT_DPCR4_PCR_COUNT                 0x00a02264 /* Data Transport PCR Phase Error Register */
#define BCHP_XPT_DPCR4_SOFT_PCR_CTRL             0x00a02268 /* Data Transport Soft PCR Control Register */
#define BCHP_XPT_DPCR4_SOFT_PCR_BASE             0x00a0226c /* Data Transport Soft PCR BASE Register */
#define BCHP_XPT_DPCR4_SOFT_PCR_EXT              0x00a02270 /* Data Transport Soft PCR Extension Register */
#define BCHP_XPT_DPCR4_PHASE_ERROR_CLAMP         0x00a02274 /* Timebase Phase Error Control */

#endif /* #ifndef BCHP_XPT_DPCR4_H__ */

/* End of File */
