/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BCHP_MEMC_CLIENTS_H_
#define BCHP_MEMC_CLIENTS_H_

#include "bchp_common.h"

#if    BCHP_CHIP==3548 || BCHP_CHIP==3556 || BCHP_CHIP==3560 \
    || BCHP_CHIP==7038 \
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
    || BCHP_CHIP==7325 || BCHP_CHIP==7335 || BCHP_CHIP==7336 || (BCHP_CHIP==7366 && (BCHP_VER >= BCHP_VER_B0)) \
    || BCHP_CHIP==7405 || BCHP_CHIP==7420 || BCHP_CHIP==7422 || BCHP_CHIP==7425 || BCHP_CHIP==7435 \
    || BCHP_CHIP==3563 || (BCHP_CHIP==35233 && BCHP_VER==A0)

#define BCHP_P_MEMC_COUNT 2

#elif BCHP_CHIP==7400 || BCHP_CHIP==7445 || BCHP_CHIP==11360
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

