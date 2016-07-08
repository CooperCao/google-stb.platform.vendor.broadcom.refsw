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
 * Date:           Generated on               Fri Feb 26 13:25:14 2016
 *                 Full Compile MD5 Checksum  1560bfee4f086d6e1d49e6bd3406a38d
 *                     (minus title and desc)
 *                 MD5 Checksum               8d7264bb382089f88abd2b1abb2a6340
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                unknown
 *                 RDB.pm                     823
 *                 generate_int_id.pl         1.0
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/generate_int_id.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#include "bchp.h"
#include "bchp_mcif_intr2.h"

#ifndef BCHP_INT_ID_MCIF_INTR2_H__
#define BCHP_INT_ID_MCIF_INTR2_H__

#define BCHP_INT_ID_CARD_ER                   BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_CARD_ER_SHIFT)
#define BCHP_INT_ID_HOST_ER                   BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_HOST_ER_SHIFT)
#define BCHP_INT_ID_HOST_HW_ER                BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_HOST_HW_ER_SHIFT)
#define BCHP_INT_ID_RX_ALL_DONE               BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_RX_ALL_DONE_SHIFT)
#define BCHP_INT_ID_RX_DONE                   BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_RX_DONE_SHIFT)
#define BCHP_INT_ID_RX_FIFO_OF                BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_RX_FIFO_OF_SHIFT)
#define BCHP_INT_ID_RX_MWPKT                  BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_RX_MWPKT_SHIFT)
#define BCHP_INT_ID_TX_DONE                   BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_TX_DONE_SHIFT)
#define BCHP_INT_ID_TX_FIFO_UF                BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_TX_FIFO_UF_SHIFT)
#define BCHP_INT_ID_TX_MEM_RDY                BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_TX_MEM_RDY_SHIFT)
#define BCHP_INT_ID_TX_MRPKT                  BCHP_INT_ID_CREATE(BCHP_MCIF_INTR2_CPU_STATUS, BCHP_MCIF_INTR2_CPU_STATUS_TX_MRPKT_SHIFT)

#endif /* #ifndef BCHP_INT_ID_MCIF_INTR2_H__ */

/* End of File */
