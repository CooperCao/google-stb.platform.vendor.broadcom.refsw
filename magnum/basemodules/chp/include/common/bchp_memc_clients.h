/***************************************************************************
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
 *
 ***************************************************************************/
#ifndef BCHP_MEMC_CLIENTS_H_
#define BCHP_MEMC_CLIENTS_H_

#include "bchp_common.h"

#if    BCHP_CHIP==3548 || BCHP_CHIP==3556 || BCHP_CHIP==3560 \
    || BCHP_CHIP==7118 || BCHP_CHIP==7125 || BCHP_CHIP==7135 \
    || BCHP_CHIP==7228 || BCHP_CHIP==7231 || BCHP_CHIP==7250 \
    || BCHP_CHIP==7340 || BCHP_CHIP==7342 || BCHP_CHIP==7344 || BCHP_CHIP==7346 || BCHP_CHIP==7358 || BCHP_CHIP==7360 || BCHP_CHIP==7362 || BCHP_CHIP==7364 || (BCHP_CHIP==7366 && (BCHP_VER < BCHP_VER_B0)) \
    || BCHP_CHIP==7401 || BCHP_CHIP==7403 || BCHP_CHIP==7408 || BCHP_CHIP==7429 || BCHP_CHIP==7438 || BCHP_CHIP==7440 || BCHP_CHIP==7468 \
    || BCHP_CHIP==7543 || BCHP_CHIP==7550 || BCHP_CHIP==7552 || BCHP_CHIP==7563 || BCHP_CHIP==7584 || BCHP_CHIP==7586 \
    || BCHP_CHIP==7601 || BCHP_CHIP==7630 || BCHP_CHIP==7635 || BCHP_CHIP==7640 \
    || BCHP_CHIP==35125 || BCHP_CHIP==35230  || (BCHP_CHIP==35233 && BCHP_VER!=A0) \
    || BCHP_CHIP==75635 || BCHP_CHIP==73625 || BCHP_CHIP==75845 || BCHP_CHIP==75525


#define BCHP_P_MEMC_COUNT 1

#elif  BCHP_CHIP==7145 \
    || (BCHP_CHIP==7366 && (BCHP_VER >= BCHP_VER_B0)) \
    || BCHP_CHIP==7420 || BCHP_CHIP==7422 || BCHP_CHIP==7425 || BCHP_CHIP==7435 \
    || BCHP_CHIP==3563 || (BCHP_CHIP==35233 && BCHP_VER==A0)

#define BCHP_P_MEMC_COUNT 2

#elif BCHP_CHIP==7445 || BCHP_CHIP==11360
#define BCHP_P_MEMC_COUNT 3
#elif defined BCHP_MEMC_DDR_2_REG_START
#define BCHP_P_MEMC_COUNT 3
#elif defined BCHP_MEMC_DDR_1_REG_START
#define BCHP_P_MEMC_COUNT 2
#elif defined BCHP_MEMC_DDR_0_REG_START
#define BCHP_P_MEMC_COUNT 1
#else
/* #warning "not supported BCHP_CHIP" */
#define BCHP_P_MEMC_COUNT 0
#endif

typedef enum BCHP_MemcClient {
#if BCHP_P_MEMC_COUNT == 1
#define BCHP_P_MEMC_DEFINE_CLIENT(x,m0) BCHP_MemcClient_e##x ,
#elif BCHP_P_MEMC_COUNT == 2
#define BCHP_P_MEMC_DEFINE_CLIENT(x,m0,m1) BCHP_MemcClient_e##x ,
#elif BCHP_P_MEMC_COUNT == 3
#define BCHP_P_MEMC_DEFINE_CLIENT(x,m0,m1,m2) BCHP_MemcClient_e##x ,
#else
#error "not supported"
#endif
#include "memc/bchp_memc_clients_chip.h"
BCHP_MemcClient_eMax
#undef  BCHP_P_MEMC_DEFINE_CLIENT
} BCHP_MemcClient;


#endif /* BCHP_MEMC_CLIENTS_H_ */
