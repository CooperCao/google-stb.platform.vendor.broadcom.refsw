/***************************************************************************
 *     Copyright (c) 1999-2012, Broadcom Corporation
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
 * Date:           Generated on         Tue Feb 28 11:03:21 2012
 *                 MD5 Checksum         d41d8cd98f00b204e9800998ecf8427e
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_XPT_DPCR8_H__
#define BCHP_XPT_DPCR8_H__

/***************************************************************************
 *XPT_DPCR8 - XPT DPCR8 Control Registers
 ***************************************************************************/
#define BCHP_XPT_DPCR8_PID_CH                    0x00947400 /* Data Transport PCR PID Channel Register */
#define BCHP_XPT_DPCR8_CTRL                      0x00947404 /* Data Transport PCR Control Register */
#define BCHP_XPT_DPCR8_INTR_STATUS_REG           0x00947408 /* Interrupt Status Register */
#define BCHP_XPT_DPCR8_INTR_STATUS_REG_EN        0x0094740c /* Interrupt Status Enable Register */
#define BCHP_XPT_DPCR8_STC_EXT_CTRL              0x00947410 /* Data Transport PCR STC Extension Control Register */
#define BCHP_XPT_DPCR8_MAX_PCR_ERROR             0x00947420 /* Data Transport PCR Max PCR Error Register */
#define BCHP_XPT_DPCR8_SEND_BASE                 0x00947424 /* Data Transport PCR Send Base Register */
#define BCHP_XPT_DPCR8_SEND_EXT                  0x00947428 /* Data Transport PCR Send Extension Register */
#define BCHP_XPT_DPCR8_STC_EXT_CTRL27            0x0094742c /* Data Transport PCR STC Extension Control Register (Test Only) */
#define BCHP_XPT_DPCR8_PWM_CTRLVALUE             0x00947438 /* Data Transport PCR PWM Control Value Register */
#define BCHP_XPT_DPCR8_LAST_PCR_HI               0x0094743c /* Data Transport PCR Last PCR MSBs Register */
#define BCHP_XPT_DPCR8_LAST_PCR_LO               0x00947440 /* Data Transport PCR Last PCR LSBs Register */
#define BCHP_XPT_DPCR8_STC_BASE_LSBS             0x00947448 /* Data Transport PCR STC Base LSBs Register */
#define BCHP_XPT_DPCR8_PHASE_ERROR               0x0094744c /* Timebase Last Phase Error */
#define BCHP_XPT_DPCR8_LOOP_CTRL                 0x00947450 /* Timebase Control */
#define BCHP_XPT_DPCR8_REF_PCR_PRESCALE          0x00947454 /* Timebase Frequency Reference Prescale Control */
#define BCHP_XPT_DPCR8_REF_PCR_INC               0x00947458 /* Timebase Frequency Reference Increment Control */
#define BCHP_XPT_DPCR8_CENTER                    0x0094745c /* Timebase Center Frequency */
#define BCHP_XPT_DPCR8_STC_HI                    0x00947430 /* Data Transport PCR STC MSBs Register */
#define BCHP_XPT_DPCR8_STC_LO                    0x00947434 /* Data Transport PCR STC LSBs Register */
#define BCHP_XPT_DPCR8_ACCUM_VALUE               0x00947460 /* Timebase Loop Filter Integrator */
#define BCHP_XPT_DPCR8_PCR_COUNT                 0x00947464 /* Data Transport PCR Phase Error Register */
#define BCHP_XPT_DPCR8_SOFT_PCR_CTRL             0x00947468 /* Data Transport Soft PCR Control Register */
#define BCHP_XPT_DPCR8_SOFT_PCR_BASE             0x0094746c /* Data Transport Soft PCR BASE Register */
#define BCHP_XPT_DPCR8_SOFT_PCR_EXT              0x00947470 /* Data Transport Soft PCR Extension Register */
#define BCHP_XPT_DPCR8_PHASE_ERROR_CLAMP         0x00947474 /* Timebase Phase Error Control */

#endif /* #ifndef BCHP_XPT_DPCR8_H__ */

/* End of File */
