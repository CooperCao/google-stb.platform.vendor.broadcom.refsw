/********************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
 * Date:           Generated on               Mon Sep 12 14:24:34 2016
 *                 Full Compile MD5 Checksum  2d2ed423991a1e4e4d03ca98bc390b2c
 *                     (minus title and desc)
 *                 MD5 Checksum               8cad5c3953d7e5df4439153720b00628
 *
 * lock_release:   r_1099
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     1119
 *                 unknown                    unknown
 *                 Perl Interpreter           5.014001
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_UARTB_H__
#define BCHP_UARTB_H__

/***************************************************************************
 *UARTB - UART B
 ***************************************************************************/
#define BCHP_UARTB_RBR                           0x0040d000 /* [RO][32] Receive Buffer Register */
#define BCHP_UARTB_THR                           0x0040d000 /* [WO][32] Transmit Holding Register */
#define BCHP_UARTB_DLL                           0x0040d000 /* [RW][32] Divisor Latch Low */
#define BCHP_UARTB_DLH                           0x0040d004 /* [RW][32] Divisor Latch High */
#define BCHP_UARTB_IER                           0x0040d004 /* [RW][32] Interrupt Enable Register */
#define BCHP_UARTB_IIR                           0x0040d008 /* [RO][32] Interrupt Identity Register */
#define BCHP_UARTB_FCR                           0x0040d008 /* [WO][32] FIFO Control Register */
#define BCHP_UARTB_LCR                           0x0040d00c /* [RW][32] Line Control Register */
#define BCHP_UARTB_MCR                           0x0040d010 /* [RW][32] Modem Control Register */
#define BCHP_UARTB_LSR                           0x0040d014 /* [RO][32] Line Status Register */
#define BCHP_UARTB_MSR                           0x0040d018 /* [RO][32] Modem Status Register */
#define BCHP_UARTB_SCR                           0x0040d01c /* [RW][32] Scratchpad Register */

#endif /* #ifndef BCHP_UARTB_H__ */

/* End of File */
