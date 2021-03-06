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
 * Date:           Generated on               Tue Jan 27 12:19:20 2015
 *                 Full Compile MD5 Checksum  3788d4127f6320d7294fc780a1f038a5
 *                     (minus title and desc)
 *                 MD5 Checksum               bc21cc7e43ef60b83e7c02281647c8e6
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15579
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_XPT_DPCR1_H__
#define BCHP_XPT_DPCR1_H__

/***************************************************************************
 *XPT_DPCR1 - XPT DPCR1 Control Registers
 ***************************************************************************/
#define BCHP_XPT_DPCR1_PID_CH                    0x00a06080 /* [RW] Data Transport PCR PID Channel Register */
#define BCHP_XPT_DPCR1_CTRL                      0x00a06084 /* [RW] Data Transport PCR Control Register */
#define BCHP_XPT_DPCR1_INTR_STATUS_REG           0x00a06088 /* [RW] Interrupt Status Register */
#define BCHP_XPT_DPCR1_INTR_STATUS_REG_EN        0x00a0608c /* [RW] Interrupt Status Enable Register */
#define BCHP_XPT_DPCR1_STC_EXT_CTRL              0x00a06090 /* [RW] Data Transport PCR STC Extension Control Register */
#define BCHP_XPT_DPCR1_MAX_PCR_ERROR             0x00a060a0 /* [RW] Data Transport PCR Max PCR Error Register */
#define BCHP_XPT_DPCR1_SEND_BASE                 0x00a060a4 /* [RW] Data Transport PCR Send Base Register */
#define BCHP_XPT_DPCR1_SEND_EXT                  0x00a060a8 /* [RW] Data Transport PCR Send Extension Register */
#define BCHP_XPT_DPCR1_STC_EXT_CTRL27            0x00a060ac /* [RO] Data Transport PCR STC Extension Control Register (Test Only) */
#define BCHP_XPT_DPCR1_STC_HI                    0x00a060b0 /* [RO] Data Transport PCR STC MSBs Register */
#define BCHP_XPT_DPCR1_STC_LO                    0x00a060b4 /* [RO] Data Transport PCR STC LSBs Register */
#define BCHP_XPT_DPCR1_PWM_CTRLVALUE             0x00a060b8 /* [RO] Data Transport PCR PWM Control Value Register */
#define BCHP_XPT_DPCR1_LAST_PCR_HI               0x00a060bc /* [RO] Data Transport PCR Last PCR MSBs Register */
#define BCHP_XPT_DPCR1_LAST_PCR_LO               0x00a060c0 /* [RO] Data Transport PCR Last PCR LSBs Register */
#define BCHP_XPT_DPCR1_STC_BASE_LSBS             0x00a060c8 /* [RO] Data Transport PCR STC Base LSBs Register */
#define BCHP_XPT_DPCR1_PHASE_ERROR               0x00a060cc /* [RO] Timebase Last Phase Error */
#define BCHP_XPT_DPCR1_LOOP_CTRL                 0x00a060d0 /* [RW] Timebase Control */
#define BCHP_XPT_DPCR1_REF_PCR_PRESCALE          0x00a060d4 /* [RW] Timebase Frequency Reference Prescale Control */
#define BCHP_XPT_DPCR1_REF_PCR_INC               0x00a060d8 /* [RW] Timebase Frequency Reference Increment Control */
#define BCHP_XPT_DPCR1_CENTER                    0x00a060dc /* [RW] Timebase Center Frequency */
#define BCHP_XPT_DPCR1_ACCUM_VALUE               0x00a060e0 /* [RW] Timebase Loop Filter Integrator */
#define BCHP_XPT_DPCR1_PCR_COUNT                 0x00a060e4 /* [RO] Data Transport PCR Phase Error Register */
#define BCHP_XPT_DPCR1_SOFT_PCR_CTRL             0x00a060e8 /* [RW] Data Transport Soft PCR Control Register */
#define BCHP_XPT_DPCR1_SOFT_PCR_BASE             0x00a060ec /* [RW] Data Transport Soft PCR BASE Register */
#define BCHP_XPT_DPCR1_SOFT_PCR_EXT              0x00a060f0 /* [RW] Data Transport Soft PCR Extension Register */
#define BCHP_XPT_DPCR1_PHASE_ERROR_CLAMP         0x00a060f4 /* [RW] Timebase Phase Error Control */

#endif /* #ifndef BCHP_XPT_DPCR1_H__ */

/* End of File */
