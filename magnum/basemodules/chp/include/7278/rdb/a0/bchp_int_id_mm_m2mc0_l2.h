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
 ******************************************************************************/



#include "bchp.h"
#include "bchp_m2mc_l2.h"
#include "bchp_mm_m2mc0_l2.h"

#ifndef BCHP_INT_ID_MM_M2MC0_L2_H__
#define BCHP_INT_ID_MM_M2MC0_L2_H__

#define BCHP_INT_ID_MM_M2MC0_L2_DEST_DECOMPRESS_MIN_GT_MAX_ERROR BCHP_INT_ID_CREATE(BCHP_MM_M2MC0_L2_CPU_STATUS, BCHP_M2MC_L2_CPU_STATUS_DEST_DECOMPRESS_MIN_GT_MAX_ERROR_SHIFT)
#define BCHP_INT_ID_MM_M2MC0_L2_GR_BRIDGE_INTR   BCHP_INT_ID_CREATE(BCHP_MM_M2MC0_L2_CPU_STATUS, BCHP_M2MC_L2_CPU_STATUS_GR_BRIDGE_INTR_SHIFT)
#define BCHP_INT_ID_MM_M2MC0_L2_M2MC_INTR        BCHP_INT_ID_CREATE(BCHP_MM_M2MC0_L2_CPU_STATUS, BCHP_M2MC_L2_CPU_STATUS_M2MC_INTR_SHIFT)
#define BCHP_INT_ID_MM_M2MC0_L2_OUTPUT_DECOMPRESS_MIN_GT_MAX_ERROR BCHP_INT_ID_CREATE(BCHP_MM_M2MC0_L2_CPU_STATUS, BCHP_M2MC_L2_CPU_STATUS_OUTPUT_DECOMPRESS_MIN_GT_MAX_ERROR_SHIFT)
#define BCHP_INT_ID_MM_M2MC0_L2_SRC_DECOMPRESS_MIN_GT_MAX_ERROR BCHP_INT_ID_CREATE(BCHP_MM_M2MC0_L2_CPU_STATUS, BCHP_M2MC_L2_CPU_STATUS_SRC_DECOMPRESS_MIN_GT_MAX_ERROR_SHIFT)

#endif /* #ifndef BCHP_INT_ID_MM_M2MC0_L2_H__ */

/* End of File */
