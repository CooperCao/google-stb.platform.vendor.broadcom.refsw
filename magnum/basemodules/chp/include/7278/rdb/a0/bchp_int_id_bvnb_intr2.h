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
 * Date:           Generated on               Mon Mar 21 13:46:36 2016
 *                 Full Compile MD5 Checksum  48e7e549bb13082ab30187cb156f35ed
 *                     (minus title and desc)
 *                 MD5 Checksum               949df837b98c31b52074d06d129f7b79
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                unknown
 *                 RDB.pm                     880
 *                 generate_int_id.pl         1.0
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/generate_int_id.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#include "bchp.h"
#include "bchp_bvnb_intr2.h"

#ifndef BCHP_INT_ID_BVNB_INTR2_H__
#define BCHP_INT_ID_BVNB_INTR2_H__

#define BCHP_INT_ID_CAP0_INTR                 BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CAP0_INTR_SHIFT)
#define BCHP_INT_ID_CAP1_INTR                 BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CAP1_INTR_SHIFT)
#define BCHP_INT_ID_CAP2_INTR                 BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CAP2_INTR_SHIFT)
#define BCHP_INT_ID_CAP3_INTR                 BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CAP3_INTR_SHIFT)
#define BCHP_INT_ID_CMP0_G0_INTR              BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CMP0_G0_INTR_SHIFT)
#define BCHP_INT_ID_CMP0_V0_INTR              BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CMP0_V0_INTR_SHIFT)
#define BCHP_INT_ID_CMP0_V1_INTR              BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CMP0_V1_INTR_SHIFT)
#define BCHP_INT_ID_CMP1_G0_INTR              BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CMP1_G0_INTR_SHIFT)
#define BCHP_INT_ID_CMP1_V0_INTR              BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CMP1_V0_INTR_SHIFT)
#define BCHP_INT_ID_CMP2_G0_INTR              BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CMP2_G0_INTR_SHIFT)
#define BCHP_INT_ID_CMP2_V0_INTR              BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CMP2_V0_INTR_SHIFT)
#define BCHP_INT_ID_CMP3_G0_INTR              BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CMP3_G0_INTR_SHIFT)
#define BCHP_INT_ID_CMP3_V0_INTR              BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_CMP3_V0_INTR_SHIFT)
#define BCHP_INT_ID_GFD0_INTR                 BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_GFD0_INTR_SHIFT)
#define BCHP_INT_ID_GFD1_INTR                 BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_GFD1_INTR_SHIFT)
#define BCHP_INT_ID_GFD2_INTR                 BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_GFD2_INTR_SHIFT)
#define BCHP_INT_ID_GFD3_INTR                 BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_GFD3_INTR_SHIFT)
#define BCHP_INT_ID_LAB_HISTO_DONE_INTR       BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_LAB_HISTO_DONE_INTR_SHIFT)
#define BCHP_INT_ID_LAB_MIN_MAX_DONE_INTR     BCHP_INT_ID_CREATE(BCHP_BVNB_INTR2_CPU_STATUS, BCHP_BVNB_INTR2_CPU_STATUS_LAB_MIN_MAX_DONE_INTR_SHIFT)

#endif /* #ifndef BCHP_INT_ID_BVNB_INTR2_H__ */

/* End of File */
