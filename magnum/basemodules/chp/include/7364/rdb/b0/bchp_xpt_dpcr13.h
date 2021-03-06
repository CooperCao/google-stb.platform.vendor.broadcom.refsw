/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
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
 * Date:           Generated on               Wed Apr  1 11:23:11 2015
 *                 Full Compile MD5 Checksum  267f8e92d9b43928c0a06f1ab29c511c
 *                     (minus title and desc)
 *                 MD5 Checksum               0548f7f0a8e20364fd383a7aa29c0b86
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15956
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
 ***************************************************************************/

#ifndef BCHP_XPT_DPCR13_H__
#define BCHP_XPT_DPCR13_H__

/***************************************************************************
 *XPT_DPCR13 - XPT DPCR13 Control Registers
 ***************************************************************************/
#define BCHP_XPT_DPCR13_PID_CH                   0x00a02680 /* [RW] Data Transport PCR PID Channel Register */
#define BCHP_XPT_DPCR13_CTRL                     0x00a02684 /* [RW] Data Transport PCR Control Register */
#define BCHP_XPT_DPCR13_INTR_STATUS_REG          0x00a02688 /* [RW] Interrupt Status Register */
#define BCHP_XPT_DPCR13_INTR_STATUS_REG_EN       0x00a0268c /* [RW] Interrupt Status Enable Register */
#define BCHP_XPT_DPCR13_STC_EXT_CTRL             0x00a02690 /* [RW] Data Transport PCR STC Extension Control Register */
#define BCHP_XPT_DPCR13_MAX_PCR_ERROR            0x00a026a0 /* [RW] Data Transport PCR Max PCR Error Register */
#define BCHP_XPT_DPCR13_SEND_BASE                0x00a026a4 /* [RW] Data Transport PCR Send Base Register */
#define BCHP_XPT_DPCR13_SEND_EXT                 0x00a026a8 /* [RW] Data Transport PCR Send Extension Register */
#define BCHP_XPT_DPCR13_STC_EXT_CTRL27           0x00a026ac /* [RO] Data Transport PCR STC Extension Control Register (Test Only) */
#define BCHP_XPT_DPCR13_STC_HI                   0x00a026b0 /* [RO] Data Transport PCR STC MSBs Register */
#define BCHP_XPT_DPCR13_STC_LO                   0x00a026b4 /* [RO] Data Transport PCR STC LSBs Register */
#define BCHP_XPT_DPCR13_PWM_CTRLVALUE            0x00a026b8 /* [RO] Data Transport PCR PWM Control Value Register */
#define BCHP_XPT_DPCR13_LAST_PCR_HI              0x00a026bc /* [RO] Data Transport PCR Last PCR MSBs Register */
#define BCHP_XPT_DPCR13_LAST_PCR_LO              0x00a026c0 /* [RO] Data Transport PCR Last PCR LSBs Register */
#define BCHP_XPT_DPCR13_STC_BASE_LSBS            0x00a026c8 /* [RO] Data Transport PCR STC Base LSBs Register */
#define BCHP_XPT_DPCR13_PHASE_ERROR              0x00a026cc /* [RO] Timebase Last Phase Error */
#define BCHP_XPT_DPCR13_LOOP_CTRL                0x00a026d0 /* [RW] Timebase Control */
#define BCHP_XPT_DPCR13_REF_PCR_PRESCALE         0x00a026d4 /* [RW] Timebase Frequency Reference Prescale Control */
#define BCHP_XPT_DPCR13_REF_PCR_INC              0x00a026d8 /* [RW] Timebase Frequency Reference Increment Control */
#define BCHP_XPT_DPCR13_CENTER                   0x00a026dc /* [RW] Timebase Center Frequency */
#define BCHP_XPT_DPCR13_ACCUM_VALUE              0x00a026e0 /* [RW] Timebase Loop Filter Integrator */
#define BCHP_XPT_DPCR13_PCR_COUNT                0x00a026e4 /* [RO] Data Transport PCR Phase Error Register */
#define BCHP_XPT_DPCR13_SOFT_PCR_CTRL            0x00a026e8 /* [RW] Data Transport Soft PCR Control Register */
#define BCHP_XPT_DPCR13_SOFT_PCR_BASE            0x00a026ec /* [RW] Data Transport Soft PCR BASE Register */
#define BCHP_XPT_DPCR13_SOFT_PCR_EXT             0x00a026f0 /* [RW] Data Transport Soft PCR Extension Register */
#define BCHP_XPT_DPCR13_PHASE_ERROR_CLAMP        0x00a026f4 /* [RW] Timebase Phase Error Control */
#define BCHP_XPT_DPCR13_TIMEBASE_INPUT_SEL       0x00a026f8 /* [RW] Timebase Input Select for Timebase Loop */

#endif /* #ifndef BCHP_XPT_DPCR13_H__ */

/* End of File */
