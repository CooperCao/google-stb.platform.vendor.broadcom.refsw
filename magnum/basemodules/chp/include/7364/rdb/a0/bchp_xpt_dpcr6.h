/****************************************************************************
 *     Copyright (c) 1999-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Aug 15 15:20:54 2014
 *                 Full Compile MD5 Checksum  a68bc62e9dd3be19fcad480c369d60fd
 *                     (minus title and desc)
 *                 MD5 Checksum               14382795d76d8497c2dd1bcf3f5d36da
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     14541
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_XPT_DPCR6_H__
#define BCHP_XPT_DPCR6_H__

/***************************************************************************
 *XPT_DPCR6 - XPT DPCR6 Control Registers
 ***************************************************************************/
#define BCHP_XPT_DPCR6_PID_CH                    0x00a02300 /* [RW] Data Transport PCR PID Channel Register */
#define BCHP_XPT_DPCR6_CTRL                      0x00a02304 /* [RW] Data Transport PCR Control Register */
#define BCHP_XPT_DPCR6_INTR_STATUS_REG           0x00a02308 /* [RW] Interrupt Status Register */
#define BCHP_XPT_DPCR6_INTR_STATUS_REG_EN        0x00a0230c /* [RW] Interrupt Status Enable Register */
#define BCHP_XPT_DPCR6_STC_EXT_CTRL              0x00a02310 /* [RW] Data Transport PCR STC Extension Control Register */
#define BCHP_XPT_DPCR6_MAX_PCR_ERROR             0x00a02320 /* [RW] Data Transport PCR Max PCR Error Register */
#define BCHP_XPT_DPCR6_SEND_BASE                 0x00a02324 /* [RW] Data Transport PCR Send Base Register */
#define BCHP_XPT_DPCR6_SEND_EXT                  0x00a02328 /* [RW] Data Transport PCR Send Extension Register */
#define BCHP_XPT_DPCR6_STC_EXT_CTRL27            0x00a0232c /* [RO] Data Transport PCR STC Extension Control Register (Test Only) */
#define BCHP_XPT_DPCR6_STC_HI                    0x00a02330 /* [RO] Data Transport PCR STC MSBs Register */
#define BCHP_XPT_DPCR6_STC_LO                    0x00a02334 /* [RO] Data Transport PCR STC LSBs Register */
#define BCHP_XPT_DPCR6_PWM_CTRLVALUE             0x00a02338 /* [RO] Data Transport PCR PWM Control Value Register */
#define BCHP_XPT_DPCR6_LAST_PCR_HI               0x00a0233c /* [RO] Data Transport PCR Last PCR MSBs Register */
#define BCHP_XPT_DPCR6_LAST_PCR_LO               0x00a02340 /* [RO] Data Transport PCR Last PCR LSBs Register */
#define BCHP_XPT_DPCR6_STC_BASE_LSBS             0x00a02348 /* [RO] Data Transport PCR STC Base LSBs Register */
#define BCHP_XPT_DPCR6_PHASE_ERROR               0x00a0234c /* [RO] Timebase Last Phase Error */
#define BCHP_XPT_DPCR6_LOOP_CTRL                 0x00a02350 /* [RW] Timebase Control */
#define BCHP_XPT_DPCR6_REF_PCR_PRESCALE          0x00a02354 /* [RW] Timebase Frequency Reference Prescale Control */
#define BCHP_XPT_DPCR6_REF_PCR_INC               0x00a02358 /* [RW] Timebase Frequency Reference Increment Control */
#define BCHP_XPT_DPCR6_CENTER                    0x00a0235c /* [RW] Timebase Center Frequency */
#define BCHP_XPT_DPCR6_ACCUM_VALUE               0x00a02360 /* [RW] Timebase Loop Filter Integrator */
#define BCHP_XPT_DPCR6_PCR_COUNT                 0x00a02364 /* [RO] Data Transport PCR Phase Error Register */
#define BCHP_XPT_DPCR6_SOFT_PCR_CTRL             0x00a02368 /* [RW] Data Transport Soft PCR Control Register */
#define BCHP_XPT_DPCR6_SOFT_PCR_BASE             0x00a0236c /* [RW] Data Transport Soft PCR BASE Register */
#define BCHP_XPT_DPCR6_SOFT_PCR_EXT              0x00a02370 /* [RW] Data Transport Soft PCR Extension Register */
#define BCHP_XPT_DPCR6_PHASE_ERROR_CLAMP         0x00a02374 /* [RW] Timebase Phase Error Control */
#define BCHP_XPT_DPCR6_TIMEBASE_INPUT_SEL        0x00a02378 /* [RW] Timebase Input Select for Timebase Loop */

#endif /* #ifndef BCHP_XPT_DPCR6_H__ */

/* End of File */
