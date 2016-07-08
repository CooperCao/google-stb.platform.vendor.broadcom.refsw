/********************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Date:           Generated on               Fri Feb 26 13:24:09 2016
 *                 Full Compile MD5 Checksum  1560bfee4f086d6e1d49e6bd3406a38d
 *                     (minus title and desc)
 *                 MD5 Checksum               8d7264bb382089f88abd2b1abb2a6340
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     823
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_HEVD_OL_CPU_DEBUG_0_H__
#define BCHP_HEVD_OL_CPU_DEBUG_0_H__

/***************************************************************************
 *HEVD_OL_CPU_DEBUG_0
 ***************************************************************************/

/***************************************************************************
 *CORE_REG%i - Core Registers
 ***************************************************************************/
#define BCHP_HEVD_OL_CPU_DEBUG_0_CORE_REGi_ARRAY_BASE              0x20000800
#define BCHP_HEVD_OL_CPU_DEBUG_0_CORE_REGi_ARRAY_START             0
#define BCHP_HEVD_OL_CPU_DEBUG_0_CORE_REGi_ARRAY_END               63
#define BCHP_HEVD_OL_CPU_DEBUG_0_CORE_REGi_ARRAY_ELEMENT_SIZE      32

/***************************************************************************
 *CORE_REG%i - Core Registers
 ***************************************************************************/
/* HEVD_OL_CPU_DEBUG_0 :: CORE_REGi :: Register [31:00] */
#define BCHP_HEVD_OL_CPU_DEBUG_0_CORE_REGi_Register_MASK           0xffffffff
#define BCHP_HEVD_OL_CPU_DEBUG_0_CORE_REGi_Register_SHIFT          0
#define BCHP_HEVD_OL_CPU_DEBUG_0_CORE_REGi_Register_DEFAULT        0x00000000


/***************************************************************************
 *AUX_REG%i - Aux Registers
 ***************************************************************************/
#define BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_BASE               0x20000c00
#define BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_START              0
#define BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_END                255
#define BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *AUX_REG%i - Aux Registers
 ***************************************************************************/
/* HEVD_OL_CPU_DEBUG_0 :: AUX_REGi :: Register [31:00] */
#define BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_Register_MASK            0xffffffff
#define BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_Register_SHIFT           0
#define BCHP_HEVD_OL_CPU_DEBUG_0_AUX_REGi_Register_DEFAULT         0x00000000


#endif /* #ifndef BCHP_HEVD_OL_CPU_DEBUG_0_H__ */

/* End of File */
