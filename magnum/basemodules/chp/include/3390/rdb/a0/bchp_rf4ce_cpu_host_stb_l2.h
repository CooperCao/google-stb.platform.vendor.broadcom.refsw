/****************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Sep 12 18:15:19 2014
 *                 Full Compile MD5 Checksum  e99be3c13daca27e9926a57447425b81
 *                     (minus title and desc)
 *                 MD5 Checksum               2d87327d868ce154059ba648b14b6240
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     14796
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_RF4CE_CPU_HOST_STB_L2_H__
#define BCHP_RF4CE_CPU_HOST_STB_L2_H__

/***************************************************************************
 *RF4CE_CPU_HOST_STB_L2 - Host STB L2 Interrupt Controller Registers
 ***************************************************************************/
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_STATUS0   0x20e80500 /* [RO] CPU interrupt Status Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_SET0      0x20e80504 /* [WO] CPU interrupt Set Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_CLEAR0    0x20e80508 /* [WO] CPU interrupt Clear Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_MASK_STATUS0 0x20e8050c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_MASK_SET0 0x20e80510 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_MASK_CLEAR0 0x20e80514 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_STATUS1   0x20e80518 /* [RO] Host Interrupt Status Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_SET1      0x20e8051c /* [WO] Host Interrupt Set Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_CLEAR1    0x20e80520 /* [WO] Host Interrupt Clear Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_MASK_STATUS1 0x20e80524 /* [RO] Host Interrupt Mask Status Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_MASK_SET1 0x20e80528 /* [WO] Host Interrupt Mask Set Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_CPU_MASK_CLEAR1 0x20e8052c /* [WO] Host Interrupt Mask Clear Register */

#endif /* #ifndef BCHP_RF4CE_CPU_HOST_STB_L2_H__ */

/* End of File */
