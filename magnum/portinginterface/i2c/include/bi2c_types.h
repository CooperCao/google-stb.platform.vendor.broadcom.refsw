/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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

#define BI2C_MAX_I2C_BSC_INDICES 7


/* From BSCA to BSCG, there are seven channels. If more cores are added in the future, this number needs to be increased. */
#define BI2C_MAX_I2C_CHANNELS (BSCA_AVAILABLE+BSCB_AVAILABLE+BSCC_AVAILABLE+BSCD_AVAILABLE+BSCE_AVAILABLE+BSCF_AVAILABLE+BSCG_AVAILABLE)

#define BI2C_TOTAL_SKIPPED_CHANNELS (BSCA_SKIPPED+BSCB_SKIPPED+BSCC_SKIPPED+BSCD_SKIPPED+BSCE_SKIPPED+BSCF_SKIPPED+BSCG_SKIPPED)

#endif
