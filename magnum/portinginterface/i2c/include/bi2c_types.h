/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 ***************************************************************************/
#ifndef BI2C_TYEPES_H__
#define BI2C_TYEPES_H__

#include "bchp.h"
#include "bstd.h"
#include "breg_mem.h"
#include "breg_i2c.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"
#include "bchp_common.h"

#ifdef BCHP_BSC_REG_START
    #define BSC_AVAILABLE 1
    #define BSC_SKIPPED 0
    #include "bchp_bsc.h"
#else
    #define BSC_SKIPPED 1
    #define BSC_AVAILABLE 0
#endif
#ifdef BCHP_BSCA_REG_START
    #define BSCA_AVAILABLE 1
    #define BSCA_SKIPPED 0
    #include "bchp_bsca.h"
#else
    #define BSCA_SKIPPED 1
    #define BSCA_AVAILABLE 0
#endif
#ifdef BCHP_BSCB_REG_START
    #define BSCB_AVAILABLE 1
    #define BSCB_SKIPPED 0
    #include "bchp_bscb.h"
#else
    #define BSCB_SKIPPED 1
    #define BSCB_AVAILABLE 0
#endif
#ifdef BCHP_BSCC_REG_START
    #define BSCC_AVAILABLE 1
    #define BSCC_SKIPPED 0
    #include "bchp_bscc.h"
#else
    #define BSCC_SKIPPED 1
    #define BSCC_AVAILABLE 0
#endif
#ifdef BCHP_BSCD_REG_START
    #define BSCD_AVAILABLE 1
    #define BSCD_SKIPPED 0
    #include "bchp_bscd.h"
#else
    #define BSCD_SKIPPED 1
    #define BSCD_AVAILABLE 0
#endif
#ifdef BCHP_BSCE_REG_START
    #define BSCE_AVAILABLE 1
    #define BSCE_SKIPPED 0
    #include "bchp_bsce.h"
#else
    #define BSCE_SKIPPED 1
    #define BSCE_AVAILABLE 0
#endif
#ifdef BCHP_BSCF_REG_START
    #define BSCF_AVAILABLE 1
    #define BSCF_SKIPPED 0
    #include "bchp_bscf.h"
#else
    #define BSCF_SKIPPED 1
    #define BSCF_AVAILABLE 0
#endif
#ifdef BCHP_BSCG_REG_START
    #define BSCG_AVAILABLE 1
    #define BSCG_SKIPPED 0
    #include "bchp_bscg.h"
#else
    #define BSCG_SKIPPED 1
    #define BSCG_AVAILABLE 0
#endif

#define BI2C_MAX_I2C_BSC_INDICES 8


/* From BSCA to BSCG, there are seven channels. If more cores are added in the future, this number needs to be increased. */
#define BI2C_MAX_I2C_CHANNELS (BSC_AVAILABLE+BSCA_AVAILABLE+BSCB_AVAILABLE+BSCC_AVAILABLE+BSCD_AVAILABLE+BSCE_AVAILABLE+BSCF_AVAILABLE+BSCG_AVAILABLE)

#define BI2C_TOTAL_SKIPPED_CHANNELS (BSC_SKIPPED+BSCA_SKIPPED+BSCB_SKIPPED+BSCC_SKIPPED+BSCD_SKIPPED+BSCE_SKIPPED+BSCF_SKIPPED+BSCG_SKIPPED)

#endif
