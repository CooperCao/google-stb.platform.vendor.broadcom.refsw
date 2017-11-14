/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 * The launch point for all information concerning RDB is found at:
 *   http://bcgbu.broadcom.com/RDB/SitePages/Home.aspx
 *
 * Date:           Generated on               Thu Apr 13 10:09:30 2017
 *                 Full Compile MD5 Checksum  7f180d7646477bba2bae1a701efd9ef5
 *                     (minus title and desc)
 *                 MD5 Checksum               a2a4a53aa20c0c2f46073b879159b85d
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     1395
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   LOCAL tools/dvtsw/current/Linux/combo_header.pl
 *
 *
********************************************************************************/

#ifndef BCHP_XPT_DPCR13_H__
#define BCHP_XPT_DPCR13_H__

/***************************************************************************
 *XPT_DPCR13 - XPT DPCR13 Control Registers
 ***************************************************************************/
#define BCHP_XPT_DPCR13_PID_CH                   0x02202d00 /* [RW][32] Data Transport PCR PID Channel Register */
#define BCHP_XPT_DPCR13_CTRL                     0x02202d04 /* [RW][32] Data Transport PCR Control Register */
#define BCHP_XPT_DPCR13_INTR_STATUS_REG          0x02202d08 /* [RW][32] Interrupt Status Register */
#define BCHP_XPT_DPCR13_INTR_STATUS_REG_EN       0x02202d0c /* [RW][32] Interrupt Status Enable Register */
#define BCHP_XPT_DPCR13_STC_EXT_CTRL             0x02202d10 /* [RW][32] Data Transport PCR STC Extension Control Register */
#define BCHP_XPT_DPCR13_MAX_PCR_ERROR            0x02202d20 /* [RW][32] Data Transport PCR Max PCR Error Register */
#define BCHP_XPT_DPCR13_SEND_BASE                0x02202d24 /* [RW][32] Data Transport PCR Send Base Register */
#define BCHP_XPT_DPCR13_SEND_EXT                 0x02202d28 /* [RW][32] Data Transport PCR Send Extension Register */
#define BCHP_XPT_DPCR13_SEND_BASE_EXT            0x02202d30 /* [RW][64] Data Transport PCR Send Base and Extension Register */
#define BCHP_XPT_DPCR13_STC_EXT_CTRL27           0x02202d38 /* [RO][32] Data Transport PCR STC Extension Control Register (Test Only) */
#define BCHP_XPT_DPCR13_STC_HI                   0x02202d3c /* [RO][32] Data Transport PCR STC MSBs Register */
#define BCHP_XPT_DPCR13_STC_LO                   0x02202d40 /* [RO][32] Data Transport PCR STC LSBs Register */
#define BCHP_XPT_DPCR13_STC                      0x02202d48 /* [RO][64] Data Transport PCR STC Register */
#define BCHP_XPT_DPCR13_PWM_CTRLVALUE            0x02202d50 /* [RO][32] Data Transport PCR PWM Control Value Register */
#define BCHP_XPT_DPCR13_LAST_PCR_HI              0x02202d54 /* [RO][32] Data Transport PCR Last PCR MSBs Register */
#define BCHP_XPT_DPCR13_LAST_PCR_LO              0x02202d58 /* [RO][32] Data Transport PCR Last PCR LSBs Register */
#define BCHP_XPT_DPCR13_LAST_PCR                 0x02202d60 /* [RO][64] Data Transport PCR Last PCR Register */
#define BCHP_XPT_DPCR13_STC_BASE_LSBS            0x02202d68 /* [RO][32] Data Transport PCR STC Base LSBs Register */
#define BCHP_XPT_DPCR13_PHASE_ERROR              0x02202d6c /* [RO][32] Timebase Last Phase Error */
#define BCHP_XPT_DPCR13_LOOP_CTRL                0x02202d70 /* [RW][32] Timebase Control */
#define BCHP_XPT_DPCR13_REF_PCR_PRESCALE         0x02202d74 /* [RW][32] Timebase Frequency Reference Prescale Control */
#define BCHP_XPT_DPCR13_REF_PCR_INC              0x02202d78 /* [RW][32] Timebase Frequency Reference Increment Control */
#define BCHP_XPT_DPCR13_CENTER                   0x02202d7c /* [RW][32] Timebase Center Frequency */
#define BCHP_XPT_DPCR13_ACCUM_VALUE              0x02202d80 /* [RW][32] Timebase Loop Filter Integrator */
#define BCHP_XPT_DPCR13_PCR_COUNT                0x02202d84 /* [RO][32] Data Transport PCR Phase Error Register */
#define BCHP_XPT_DPCR13_SOFT_PCR_CTRL            0x02202d88 /* [RW][32] Data Transport Soft PCR Control Register */
#define BCHP_XPT_DPCR13_SOFT_PCR_BASE            0x02202d8c /* [RW][32] Data Transport Soft PCR BASE Register */
#define BCHP_XPT_DPCR13_SOFT_PCR_EXT             0x02202d90 /* [RW][32] Data Transport Soft PCR Extension Register */
#define BCHP_XPT_DPCR13_SOFT_PCR_BASE_EXT        0x02202d98 /* [RW][64] Data Transport Soft PCR Base + Extension Register */
#define BCHP_XPT_DPCR13_PHASE_ERROR_CLAMP        0x02202da0 /* [RW][32] Timebase Phase Error Control */
#define BCHP_XPT_DPCR13_TIMEBASE_INPUT_SEL       0x02202da4 /* [RW][32] Timebase Input Select for Timebase Loop */

#endif /* #ifndef BCHP_XPT_DPCR13_H__ */

/* End of File */
