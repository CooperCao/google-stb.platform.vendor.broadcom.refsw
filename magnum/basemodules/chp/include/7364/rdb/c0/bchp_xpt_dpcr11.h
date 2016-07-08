/****************************************************************************
 *     Copyright (c) 1999-2016, Broadcom Corporation
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
 * Date:           Generated on               Mon Feb  8 12:53:15 2016
 *                 Full Compile MD5 Checksum  7c463a9180016920b3e03273285ff33d
 *                     (minus title and desc)
 *                 MD5 Checksum               30fed0099690880293569d98807ed1d8
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     749
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_XPT_DPCR11_H__
#define BCHP_XPT_DPCR11_H__

/***************************************************************************
 *XPT_DPCR11 - XPT DPCR11 Control Registers
 ***************************************************************************/
#define BCHP_XPT_DPCR11_PID_CH                   0x00a02580 /* [RW] Data Transport PCR PID Channel Register */
#define BCHP_XPT_DPCR11_CTRL                     0x00a02584 /* [RW] Data Transport PCR Control Register */
#define BCHP_XPT_DPCR11_INTR_STATUS_REG          0x00a02588 /* [RW] Interrupt Status Register */
#define BCHP_XPT_DPCR11_INTR_STATUS_REG_EN       0x00a0258c /* [RW] Interrupt Status Enable Register */
#define BCHP_XPT_DPCR11_STC_EXT_CTRL             0x00a02590 /* [RW] Data Transport PCR STC Extension Control Register */
#define BCHP_XPT_DPCR11_MAX_PCR_ERROR            0x00a025a0 /* [RW] Data Transport PCR Max PCR Error Register */
#define BCHP_XPT_DPCR11_SEND_BASE                0x00a025a4 /* [RW] Data Transport PCR Send Base Register */
#define BCHP_XPT_DPCR11_SEND_EXT                 0x00a025a8 /* [RW] Data Transport PCR Send Extension Register */
#define BCHP_XPT_DPCR11_STC_EXT_CTRL27           0x00a025ac /* [RO] Data Transport PCR STC Extension Control Register (Test Only) */
#define BCHP_XPT_DPCR11_STC_HI                   0x00a025b0 /* [RO] Data Transport PCR STC MSBs Register */
#define BCHP_XPT_DPCR11_STC_LO                   0x00a025b4 /* [RO] Data Transport PCR STC LSBs Register */
#define BCHP_XPT_DPCR11_PWM_CTRLVALUE            0x00a025b8 /* [RO] Data Transport PCR PWM Control Value Register */
#define BCHP_XPT_DPCR11_LAST_PCR_HI              0x00a025bc /* [RO] Data Transport PCR Last PCR MSBs Register */
#define BCHP_XPT_DPCR11_LAST_PCR_LO              0x00a025c0 /* [RO] Data Transport PCR Last PCR LSBs Register */
#define BCHP_XPT_DPCR11_STC_BASE_LSBS            0x00a025c8 /* [RO] Data Transport PCR STC Base LSBs Register */
#define BCHP_XPT_DPCR11_PHASE_ERROR              0x00a025cc /* [RO] Timebase Last Phase Error */
#define BCHP_XPT_DPCR11_LOOP_CTRL                0x00a025d0 /* [RW] Timebase Control */
#define BCHP_XPT_DPCR11_REF_PCR_PRESCALE         0x00a025d4 /* [RW] Timebase Frequency Reference Prescale Control */
#define BCHP_XPT_DPCR11_REF_PCR_INC              0x00a025d8 /* [RW] Timebase Frequency Reference Increment Control */
#define BCHP_XPT_DPCR11_CENTER                   0x00a025dc /* [RW] Timebase Center Frequency */
#define BCHP_XPT_DPCR11_ACCUM_VALUE              0x00a025e0 /* [RW] Timebase Loop Filter Integrator */
#define BCHP_XPT_DPCR11_PCR_COUNT                0x00a025e4 /* [RO] Data Transport PCR Phase Error Register */
#define BCHP_XPT_DPCR11_SOFT_PCR_CTRL            0x00a025e8 /* [RW] Data Transport Soft PCR Control Register */
#define BCHP_XPT_DPCR11_SOFT_PCR_BASE            0x00a025ec /* [RW] Data Transport Soft PCR BASE Register */
#define BCHP_XPT_DPCR11_SOFT_PCR_EXT             0x00a025f0 /* [RW] Data Transport Soft PCR Extension Register */
#define BCHP_XPT_DPCR11_PHASE_ERROR_CLAMP        0x00a025f4 /* [RW] Timebase Phase Error Control */
#define BCHP_XPT_DPCR11_TIMEBASE_INPUT_SEL       0x00a025f8 /* [RW] Timebase Input Select for Timebase Loop */

#endif /* #ifndef BCHP_XPT_DPCR11_H__ */

/* End of File */
