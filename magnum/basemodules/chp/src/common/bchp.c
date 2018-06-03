/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bchp.h"
#include "bchp_priv.h"
#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_memc_offsets_priv.h"

BDBG_MODULE(BCHP);

static void BCHP_P_StandbyMode( BCHP_Handle hChip, bool activate );

BERR_Code BCHP_Close(
    BCHP_Handle hChip               /* [in] Chip handle */
    )
{
    BDBG_OBJECT_ASSERT(hChip, BCHP);
    if (hChip->hAvsHandle) {
        BCHP_P_AvsClose(hChip->hAvsHandle);
    }
    BCHP_PWR_Close(hChip->pwrManager);
    BDBG_OBJECT_DESTROY(hChip, BCHP);
    BKNI_Free(hChip);
    return BERR_SUCCESS;
}

BERR_Code BCHP_GetChipInfo(
    const BCHP_Handle hChip,            /* [in] Chip handle */
    uint16_t *pChipId,                  /* [out] Chip Id */
    uint16_t *pChipRev                  /* [out] Chip Rev. */
    )
{
    BCHP_Info info;
    BDBG_OBJECT_ASSERT(hChip, BCHP);
    BCHP_GetInfo(hChip, &info);
    *pChipId = info.familyId;
    *pChipRev = info.rev;
    return BERR_SUCCESS;
}

BERR_Code BCHP_GetFeature(
    const BCHP_Handle hChip,            /* [in] Chip handle */
    const BCHP_Feature eFeature,        /* [in] Feature to be queried. */
    void *pFeatureValue                 /* [out] Feature value. */
    )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BCHP_GetFeature );
    BDBG_OBJECT_ASSERT(hChip, BCHP);

    if (hChip->pGetFeatureFunc)
        rc = hChip->pGetFeatureFunc( hChip, eFeature, pFeatureValue );
    else
        rc = BERR_TRACE(BERR_UNKNOWN);

    BDBG_LEAVE( BCHP_GetFeature );
    return( rc );
}

void BCHP_MonitorPvt(
    BCHP_Handle hChip,              /* [in] Chip handle */
    BCHP_AvsSettings *pSettings     /* [in] AVS settings. */
    )
{
    BSTD_UNUSED(pSettings);
    BDBG_OBJECT_ASSERT(hChip, BCHP);
    if (hChip->hAvsHandle)
        BCHP_P_AvsMonitorPvt(hChip->hAvsHandle);
}

BERR_Code BCHP_GetAvsData_isrsafe(
    BCHP_Handle hChip,   /* [in] Chip handle */
    BCHP_AvsData *pData) /* [out] pointer to location to return data */
{
    BDBG_OBJECT_ASSERT(hChip, BCHP);
    if (hChip->hAvsHandle)
        BCHP_P_GetAvsData_isrsafe(hChip->hAvsHandle, pData);
    return BERR_SUCCESS;
}

BERR_Code BCHP_Standby(
    BCHP_Handle hChip)   /* [in] Chip handle */
{
    BCHP_P_StandbyMode(hChip, true);
    return BERR_SUCCESS;
}

BERR_Code BCHP_Resume(
    BCHP_Handle hChip)  /* [in] Chip handle */
{
    BCHP_P_StandbyMode(hChip, false);
    return BERR_SUCCESS;
}

/* For testing purposes only */
void *BCHP_GetAvsHandle(
    BCHP_Handle hChip)   /* [in] Chip handle */
{
    void *result;

    BDBG_ENTER( BCHP_GetAvsHandle );
    BDBG_OBJECT_ASSERT(hChip, BCHP);

    result = (void*)hChip->avsHandle; /* will be NULL for platforms without AVS support */

    BDBG_LEAVE( BCHP_GetAvsHandle );
    return( result );
}

/* NOTE: do not add BCHP_CHIP list here. instead, use #if defined(RDB MACRO) where the RDB macros come from bchp_common.h.
There are three register fields to define per MEMC:
    DEVICE_TECH = size (computed with computeDeviceTech)
    DDR3 = ddr3 capable?
    WIDTH = 16 bit, 32 bit, etc.
*/
#if defined BCHP_MEMC_DDR_0_REG_START
#include "bchp_memc_ddr_0.h"
#if defined BCHP_MEMC_DDR_0_CNTRLR_CONFIG_DRAM_DEVICE_TYPE_MASK
/* example chip: 7445 */
#define BCHP_MEMC_0_DEVICE_TECH_REG BCHP_MEMC_DDR_0_CNTRLR_CONFIG
#define MEMC_0_DEVICE_TECH_REG      MEMC_DDR_0_CNTRLR_CONFIG
#define MEMC_0_DEVICE_TECH_FIELD    DRAM_DEVICE_SIZE
#define MEMC_0_GROUPAGE_ENABLED_FIELD    GROUPAGE_ENABLE
#define BCHP_MEMC_0_DRAM_TYPE_REG        BCHP_MEMC_DDR_0_CNTRLR_CONFIG
#define MEMC_0_DRAM_TYPE_REG             MEMC_DDR_0_CNTRLR_CONFIG
#define MEMC_0_DRAM_TYPE_FIELD           DRAM_DEVICE_TYPE
#else
/* example chip: 7231 */
#define BCHP_MEMC_0_DEVICE_TECH_REG BCHP_MEMC_DDR_0_CNTRLR_CONFIG
#define MEMC_0_DEVICE_TECH_REG      MEMC_DDR_0_CNTRLR_CONFIG
#define MEMC_0_DEVICE_TECH_FIELD    DEVICE_TECH
#define BCHP_MEMC_0_DRAM_TYPE_REG        BCHP_MEMC_DDR_0_DRAM_INIT_CNTRL
#define MEMC_0_DRAM_TYPE_REG             MEMC_DDR_0_DRAM_INIT_CNTRL
#define MEMC_0_DRAM_TYPE_FIELD           DDR_TECH
#endif

#if defined BCHP_MEMC_DDR_0_CNTRLR_CONFIG_DRAM_TOTAL_WIDTH_MASK
#define BCHP_MEMC_0_WIDTH_REG       BCHP_MEMC_DDR_0_CNTRLR_CONFIG
#define MEMC_0_WIDTH_REG            MEMC_DDR_0_CNTRLR_CONFIG
#define MEMC_0_WIDTH_FIELD          DRAM_TOTAL_WIDTH
#define MEMC_0_DEVICE_WIDTH_FIELD   DRAM_DEVICE_WIDTH
#elif defined BCHP_MEMC_DDR23_APHY_AC_0_REG_START
#include "bchp_memc_ddr23_aphy_ac_0.h"
#define BCHP_MEMC_0_WIDTH_REG       BCHP_MEMC_DDR23_APHY_AC_0_CONFIG
#define MEMC_0_WIDTH_REG            MEMC_DDR23_APHY_AC_0_CONFIG
#define MEMC_0_WIDTH_FIELD          DRAM_WIDTH
#elif defined BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_REG_START
#include "bchp_memc_ddr23_shim_addr_cntl.h"
#define BCHP_MEMC_0_WIDTH_REG       BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_CONFIG
#define MEMC_0_WIDTH_REG            MEMC_DDR23_SHIM_ADDR_CNTL_CONFIG
#define MEMC_0_WIDTH_FIELD          DRAM_WIDTH
#elif defined BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_REG_START
#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#define BCHP_MEMC_0_WIDTH_REG       BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG
#define MEMC_0_WIDTH_REG            MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG
#define MEMC_0_WIDTH_FIELD          DRAM_WIDTH
#else
#error
#endif
#else
/* MEMC0 is required */
#error unsupported chip
#endif


#if defined BCHP_MEMC_DDR_1_REG_START
#include "bchp_memc_ddr_1.h"

#if defined BCHP_MEMC_DDR_1_CNTRLR_CONFIG_DRAM_DEVICE_TYPE_MASK
#define BCHP_MEMC_1_DEVICE_TECH_REG BCHP_MEMC_DDR_1_CNTRLR_CONFIG
#define MEMC_1_DEVICE_TECH_REG      MEMC_DDR_1_CNTRLR_CONFIG
#define MEMC_1_DEVICE_TECH_FIELD    DRAM_DEVICE_SIZE
#define MEMC_1_GROUPAGE_ENABLED_FIELD    GROUPAGE_ENABLE
#define BCHP_MEMC_1_DRAM_TYPE_REG        BCHP_MEMC_DDR_1_CNTRLR_CONFIG
#define MEMC_1_DRAM_TYPE_REG             MEMC_DDR_1_CNTRLR_CONFIG
#define MEMC_1_DRAM_TYPE_FIELD           DRAM_DEVICE_TYPE
#else
#define BCHP_MEMC_1_DEVICE_TECH_REG BCHP_MEMC_DDR_1_CNTRLR_CONFIG
#define MEMC_1_DEVICE_TECH_REG      MEMC_DDR_1_CNTRLR_CONFIG
#define MEMC_1_DEVICE_TECH_FIELD    DEVICE_TECH
#define BCHP_MEMC_1_DRAM_TYPE_REG        BCHP_MEMC_DDR_1_DRAM_INIT_CNTRL
#define MEMC_1_DRAM_TYPE_REG             MEMC_DDR_1_DRAM_INIT_CNTRL
#define MEMC_1_DRAM_TYPE_FIELD           DDR_TECH
#endif

#if defined BCHP_MEMC_DDR_1_CNTRLR_CONFIG_DRAM_TOTAL_WIDTH_MASK
#define BCHP_MEMC_1_WIDTH_REG       BCHP_MEMC_DDR_1_CNTRLR_CONFIG
#define MEMC_1_WIDTH_REG            MEMC_DDR_1_CNTRLR_CONFIG
#define MEMC_1_WIDTH_FIELD          DRAM_TOTAL_WIDTH
#define MEMC_1_DEVICE_WIDTH_FIELD   DRAM_DEVICE_WIDTH
#elif defined BCHP_MEMC_DDR23_APHY_AC_1_REG_START
#include "bchp_memc_ddr23_aphy_ac_1.h"
#define BCHP_MEMC_1_WIDTH_REG       BCHP_MEMC_DDR23_APHY_AC_1_CONFIG
#define MEMC_1_WIDTH_REG            MEMC_DDR23_APHY_AC_1_CONFIG
#define MEMC_1_WIDTH_FIELD          DRAM_WIDTH
#elif defined BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_REG_START
#include "bchp_memc_ddr23_shim_addr_cntl_1.h"
#define BCHP_MEMC_1_WIDTH_REG       BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_1_CONFIG
#define MEMC_1_WIDTH_REG            MEMC_DDR23_SHIM_ADDR_CNTL_1_CONFIG
#define MEMC_1_WIDTH_FIELD          DRAM_WIDTH
#endif
#endif

#if defined BCHP_MEMC_DDR_2_REG_START
#include "bchp_memc_ddr_2.h"
#if defined BCHP_MEMC_DDR_2_CNTRLR_CONFIG_DRAM_DEVICE_TYPE_MASK
#define BCHP_MEMC_2_DEVICE_TECH_REG BCHP_MEMC_DDR_2_CNTRLR_CONFIG
#define MEMC_2_DEVICE_TECH_REG      MEMC_DDR_2_CNTRLR_CONFIG
#define MEMC_2_DEVICE_TECH_FIELD    DRAM_DEVICE_SIZE
#define MEMC_2_GROUPAGE_ENABLED_FIELD    GROUPAGE_ENABLE
#define BCHP_MEMC_2_DRAM_TYPE_REG        BCHP_MEMC_DDR_2_CNTRLR_CONFIG
#define MEMC_2_DRAM_TYPE_REG             MEMC_DDR_2_CNTRLR_CONFIG
#define MEMC_2_DRAM_TYPE_FIELD           DRAM_DEVICE_TYPE
#define BCHP_MEMC_2_WIDTH_REG       BCHP_MEMC_DDR_2_CNTRLR_CONFIG
#define MEMC_2_WIDTH_REG            MEMC_DDR_2_CNTRLR_CONFIG
#define MEMC_2_WIDTH_FIELD          DRAM_TOTAL_WIDTH
#define MEMC_2_DEVICE_WIDTH_FIELD   DRAM_DEVICE_WIDTH
#else
#error
#endif
#endif

#if defined BCHP_MEMC_GEN_0_REG_START
#include "bchp_memc_gen_0.h"
#endif
#if defined BCHP_MEMC_GEN_1_REG_START
#include "bchp_memc_gen_1.h"
#endif
#if defined BCHP_MEMC_GEN_2_REG_START
#include "bchp_memc_gen_2.h"
#endif
#define BCHP_P_SCB_V8               0xB200
#define BCHP_P_SCB_V5               0xB100

static unsigned computeDeviceTech(unsigned index)
{
    unsigned size = 256;
    while (index--) size *= 2;
    return size;
}

static unsigned computeMemWidth(unsigned value)
{
#if defined BCHP_MEMC_DDR_0_CNTRLR_CONFIG_DRAM_DEVICE_SIZE_MASK
    switch (value) {
    case 0: return 8;
    case 1: return 16;
    case 2: return 32;
    case 3: return 64;
    default: BDBG_ERR(("unknown mem width index %d", value)); return 32;
    }
#else
    switch (value) {
    case 0: return 32;
    case 1: return 16;
    default: BDBG_ERR(("unknown mem width index %d", value)); return 32;
    }
#endif
}

static unsigned computeDramDeviceType(unsigned value)
{
#if defined BCHP_MEMC_DDR_0_DRAM_INIT_CNTRL_DDR_TECH_MASK
    /* only DDR2/DDR3 map, so don't allow other values through */
    return value?BCHP_DramType_eDDR3:BCHP_DramType_eDDR2;
#else
    /* all values map to register */
    if (value >= BCHP_DramType_eMax) {
        BDBG_ERR(("unknown dram type %d", value));
        return 0;
    }
    return value;
#endif
}

static BERR_Code BCHP_P_GetStripeMemoryInfo(BCHP_MemoryInfo *pstMemoryInfo)
{
   unsigned i=0;
   BERR_Code rc = BERR_SUCCESS;

   for(i=0; i < (sizeof(pstMemoryInfo->memc)/sizeof(pstMemoryInfo->memc[0])) && pstMemoryInfo->memc[i].width; i++) {
   switch ( pstMemoryInfo->memc[i].type )
   {
      case BCHP_DramType_eDDR3:
         switch ( pstMemoryInfo->memc[i].deviceTech )
         {
            case 1024:
            case 2048:
            case 4096:
               switch ( pstMemoryInfo->memc[i].width )
               {
                  case 16:
                     pstMemoryInfo->memc[i].ulPageSize = 2048;
                     pstMemoryInfo->memc[i].ulStripeWidth = 128;
                     pstMemoryInfo->memc[i].ulMbMultiplier = 8;
                     pstMemoryInfo->memc[i].ulMbRemainder = 4; /* Use 4 instead of 3 for backwards compatibility with older chips */
                     break;

                  case 32:
                     pstMemoryInfo->memc[i].ulPageSize = 4096;
                     pstMemoryInfo->memc[i].ulStripeWidth = 128;
                     pstMemoryInfo->memc[i].ulMbMultiplier = 16;
                     pstMemoryInfo->memc[i].ulMbRemainder = 6;
                     break;

                  default:
                     BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
                     rc = BERR_TRACE( BERR_NOT_SUPPORTED );
               }
               break;

            case 8192:
               switch ( pstMemoryInfo->memc[i].deviceWidth )
               {
                  case 8:
                     switch ( pstMemoryInfo->memc[i].width )
                     {
                        case 16:
                           pstMemoryInfo->memc[i].ulPageSize = 4096;
                           pstMemoryInfo->memc[i].ulStripeWidth = 128;
                           pstMemoryInfo->memc[i].ulMbMultiplier = 16;
                           pstMemoryInfo->memc[i].ulMbRemainder = 6;
                           break;

                        case 32:
                           pstMemoryInfo->memc[i].ulPageSize = 8192;
                           pstMemoryInfo->memc[i].ulStripeWidth = 128;
                           pstMemoryInfo->memc[i].ulMbMultiplier = 32;
                           pstMemoryInfo->memc[i].ulMbRemainder = 12;
                           break;

                        default:
                           BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
                           rc = BERR_TRACE( BERR_NOT_SUPPORTED );
                     }
                     break;

                  case 16:
                     switch ( pstMemoryInfo->memc[i].width )
                     {
                        case 16:
                           pstMemoryInfo->memc[i].ulPageSize = 2048;
                           pstMemoryInfo->memc[i].ulStripeWidth = 128;
                           pstMemoryInfo->memc[i].ulMbMultiplier = 8;
                           pstMemoryInfo->memc[i].ulMbRemainder = 4; /* Use 4 instead of 3 for backwards compatibility with older chips */
                           break;

                        case 32:
                           pstMemoryInfo->memc[i].ulPageSize = 4096;
                           pstMemoryInfo->memc[i].ulStripeWidth = 128;
                           pstMemoryInfo->memc[i].ulMbMultiplier = 16;
                           pstMemoryInfo->memc[i].ulMbRemainder = 6;
                           break;

                        default:
                           BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
                           rc = BERR_TRACE( BERR_NOT_SUPPORTED );
                     }
                     break;

                  default:
                     BDBG_ERR(("Unsupported device width (%d)", pstMemoryInfo->memc[i].deviceWidth ));
                     rc = BERR_TRACE( BERR_NOT_SUPPORTED );
               }
               break;

            default:
               BDBG_ERR(("Unsupported device tech (%d)", pstMemoryInfo->memc[i].deviceTech ));
               rc = BERR_TRACE( BERR_NOT_SUPPORTED );
         }
         break;

       case BCHP_DramType_eDDR4:
         switch ( pstMemoryInfo->memc[i].width )
         {
            case 16:
               pstMemoryInfo->memc[i].ulPageSize = 4096;
               pstMemoryInfo->memc[i].ulStripeWidth = 128;
               pstMemoryInfo->memc[i].ulMbMultiplier = 16;
               pstMemoryInfo->memc[i].ulMbRemainder = 6;
               break;

            case 32:
               pstMemoryInfo->memc[i].ulPageSize = 8192;
               pstMemoryInfo->memc[i].ulStripeWidth = 256;
               pstMemoryInfo->memc[i].ulMbMultiplier = 16;
               pstMemoryInfo->memc[i].ulMbRemainder = 6;
               break;

            default:
               BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
               rc = BERR_TRACE( BERR_NOT_SUPPORTED );
         }
         break;

      case BCHP_DramType_eLPDDR4:
               pstMemoryInfo->memc[i].ulPageSize     = 4096;
               pstMemoryInfo->memc[i].ulStripeWidth  = 256;
               pstMemoryInfo->memc[i].ulMbMultiplier = 8;
               pstMemoryInfo->memc[i].ulMbRemainder  = 3;
               break;

      case BCHP_DramType_eGDDR5M:
      case BCHP_DramType_eGDDR5:
         if ( false == pstMemoryInfo->memc[i].groupageEnabled )
         {
            /* DDR5 */
            switch ( pstMemoryInfo->memc[i].deviceTech )
            {
               case 1024:
               case 2048:
               case 4096:
                  switch ( pstMemoryInfo->memc[i].width )
                  {
                     case 16:
                        pstMemoryInfo->memc[i].ulPageSize = 2048;
                        pstMemoryInfo->memc[i].ulStripeWidth = 128;
                        pstMemoryInfo->memc[i].ulMbMultiplier = 8;
                        pstMemoryInfo->memc[i].ulMbRemainder = 3;
                        break;

                     case 32:
                        pstMemoryInfo->memc[i].ulPageSize = 4096;
                        pstMemoryInfo->memc[i].ulStripeWidth = 128;
                        pstMemoryInfo->memc[i].ulMbMultiplier = 16;
                        pstMemoryInfo->memc[i].ulMbRemainder = 6;
                        break;

                     default:
                        BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
                        rc = BERR_TRACE( BERR_NOT_SUPPORTED );
                  }
                  break;

               case 8192:
                  switch ( pstMemoryInfo->memc[i].width )
                  {
                     case 16:
                        pstMemoryInfo->memc[i].ulPageSize = 4096;
                        pstMemoryInfo->memc[i].ulStripeWidth = 128;
                        pstMemoryInfo->memc[i].ulMbMultiplier = 16;
                        pstMemoryInfo->memc[i].ulMbRemainder = 6;
                        break;

                     case 32:
                        pstMemoryInfo->memc[i].ulPageSize = 8192;
                        pstMemoryInfo->memc[i].ulStripeWidth = 128;
                        pstMemoryInfo->memc[i].ulMbMultiplier = 32;
                        pstMemoryInfo->memc[i].ulMbRemainder = 12;
                        break;

                     default:
                        BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
                        rc = BERR_TRACE( BERR_NOT_SUPPORTED );
                  }
                  break;

               default:
                  BDBG_ERR(("Unsupported device tech (%d)", pstMemoryInfo->memc[i].deviceTech ));
                  rc = BERR_TRACE( BERR_NOT_SUPPORTED );
            }
         }
         else
         {
            /* DDR5-Fast */
            switch ( pstMemoryInfo->memc[i].deviceTech )
            {
               case 1024:
               case 2048:
               case 4096:
                  switch ( pstMemoryInfo->memc[i].width )
                  {
                     case 16:
                        pstMemoryInfo->memc[i].ulPageSize = 4096;
                        pstMemoryInfo->memc[i].ulStripeWidth = 128;
                        pstMemoryInfo->memc[i].ulMbMultiplier = 16;
                        pstMemoryInfo->memc[i].ulMbRemainder = 6;
                        break;

                     case 32:
                        pstMemoryInfo->memc[i].ulPageSize = 8192;
                        pstMemoryInfo->memc[i].ulStripeWidth = 256;
                        pstMemoryInfo->memc[i].ulMbMultiplier = 16;
                        pstMemoryInfo->memc[i].ulMbRemainder = 6;
                        break;

                     default:
                        BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
                        rc = BERR_TRACE( BERR_NOT_SUPPORTED );
                  }
                  break;

               case 8192:
                  switch ( pstMemoryInfo->memc[i].width )
                  {
                     case 16:
                        pstMemoryInfo->memc[i].ulPageSize = 8192;
                        pstMemoryInfo->memc[i].ulStripeWidth = 128;
                        pstMemoryInfo->memc[i].ulMbMultiplier = 32;
                        pstMemoryInfo->memc[i].ulMbRemainder = 12;
                        break;

                     case 32:
                        pstMemoryInfo->memc[i].ulPageSize = 16*1024;
                        pstMemoryInfo->memc[i].ulStripeWidth = 256;
                        pstMemoryInfo->memc[i].ulMbMultiplier = 32;
                        pstMemoryInfo->memc[i].ulMbRemainder = 12;
                        break;

                     default:
                        BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
                        rc = BERR_TRACE( BERR_NOT_SUPPORTED );
                  }
                  break;

               default:
                  BDBG_ERR(("Unsupported device tech (%d)", pstMemoryInfo->memc[i].deviceTech ));
                  rc = BERR_TRACE( BERR_NOT_SUPPORTED );
            }
         }
         break;

      case BCHP_DramType_eDDR2:
      switch ( pstMemoryInfo->memc[i].deviceTech )
          {
              case 256:
                 switch ( pstMemoryInfo->memc[i].width )
                 {
                    case 16:
                       pstMemoryInfo->memc[i].ulPageSize = 1024;
                       pstMemoryInfo->memc[i].ulStripeWidth = 64;
                       pstMemoryInfo->memc[i].ulMbMultiplier = 8;
                       pstMemoryInfo->memc[i].ulMbRemainder = 2;
                       break;

                    case 32:
                       pstMemoryInfo->memc[i].ulPageSize = 2048;
                       pstMemoryInfo->memc[i].ulStripeWidth = 64;
                       pstMemoryInfo->memc[i].ulMbMultiplier = 8;
                       pstMemoryInfo->memc[i].ulMbRemainder = 2;
                       break;

                    default:
                       BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
                       rc = BERR_TRACE( BERR_NOT_SUPPORTED );
                 }
                 break;
               case 512:
               case 1024:
               case 2048:
                 switch ( pstMemoryInfo->memc[i].width )
                 {
                        case 16:
                    pstMemoryInfo->memc[i].ulPageSize = 2048;
                    pstMemoryInfo->memc[i].ulStripeWidth = 64;
                    pstMemoryInfo->memc[i].ulMbMultiplier = 8;
                    pstMemoryInfo->memc[i].ulMbRemainder = 2;
                    break;

                    case 32:
                    pstMemoryInfo->memc[i].ulPageSize = 4096;
                    pstMemoryInfo->memc[i].ulStripeWidth = 128;
                    pstMemoryInfo->memc[i].ulMbMultiplier = 16;
                    pstMemoryInfo->memc[i].ulMbRemainder = 4;
                    break;

                        default:
                    BDBG_ERR(("Unsupported memory width (%d)", pstMemoryInfo->memc[i].width ));
                    rc = BERR_TRACE( BERR_NOT_SUPPORTED );
                     }
                 break;
               default:
                    BDBG_ERR(("Unsupported device tech/density (%d)", pstMemoryInfo->memc[i].deviceTech ));
                    rc = BERR_TRACE( BERR_NOT_SUPPORTED );
          }
      break;
      default:
         BDBG_ERR(("Unsupported DRAM type (%d)", pstMemoryInfo->memc[i].type ));
         rc = BERR_TRACE( BERR_NOT_SUPPORTED );
   }
   }

   return BERR_TRACE( rc );
}

static void BCHP_P_GetMemoryShuffleInfo(
    BREG_Handle hReg,
    unsigned memcId,
    uint32_t regOffset,
    BCHP_MemoryInfo *pInfo)
{
    uint32_t regValue;

    /* get memc SCB/MAP version and blindshuffle */
    /* MEMC_GEN_0_CORE_REV_ID tells SCB version and MAP version for shuffle:
    if (coreID < B1.0) {
        scbVersion = SCB4; blindShuffle = false;
        mapVersion = MAP2; shuffleMap = NoShuffle;
    } else if (coreID < B2.0) {
        scbVersion = SCB5; blindShuffle = false;
        mapVersion = MAP5; shuffleMap = ShuffleMap5;
    } else {
        scbVersion = SCB8; blindShuffle = true;
        switch(DRAM_MAP_ENCODING) {
        case 0: mapVersion = MAP2: shuffleMap = NoShuffle; break;
        case 1: mapVersion = MAP5: shuffleMap = ShuffleMap5; break;
        case 2: mapVersion = MAP8: shuffleMap = ShuffleMap8; break;
      }
    }
     */
    regValue = BREG_Read32(hReg, BCHP_MEMC_GEN_0_CORE_REV_ID + regOffset);
    regValue &= (
        BCHP_MASK(MEMC_GEN_0_CORE_REV_ID, ARCH_REV_ID) |
        BCHP_MASK(MEMC_GEN_0_CORE_REV_ID, CFG_REV_ID));
    if(regValue < BCHP_P_SCB_V5) {
        pInfo->memc[memcId].blindShuffle = false;
        pInfo->memc[memcId].mapVer = BCHP_ScbMapVer_eMap2;
    } else if(regValue < BCHP_P_SCB_V8) {
        pInfo->memc[memcId].blindShuffle = false;
        pInfo->memc[memcId].mapVer = BCHP_ScbMapVer_eMap5;
    } else {
        pInfo->memc[memcId].blindShuffle = true; /* SCB v8 has blind shuffle */
#if defined BCHP_MEMC_GEN_0_CORE_OPTIONS_DRAM_MAP_ENCODING_MASK
        regValue = BREG_Read32(hReg, BCHP_MEMC_GEN_0_CORE_OPTIONS + regOffset);
        pInfo->memc[memcId].mapVer = BCHP_GET_FIELD_DATA(regValue, MEMC_GEN_0_CORE_OPTIONS, DRAM_MAP_ENCODING);
#endif
    }
}

BERR_Code BCHP_GetMemoryInfo_PreInit(BREG_Handle hReg, BCHP_MemoryInfo *pInfo)
{
    uint32_t regValue;
    unsigned i;

    BKNI_Memset(pInfo, 0, sizeof(*pInfo));
    /* Do NOT compute for generic MEMC */
#if BCHP_CHIP != 11360
    regValue = BREG_Read32(hReg, BCHP_MEMC_0_WIDTH_REG);
    pInfo->memc[0].width = computeMemWidth(BCHP_GET_FIELD_DATA(regValue, MEMC_0_WIDTH_REG, MEMC_0_WIDTH_FIELD));
#if defined MEMC_0_DEVICE_WIDTH_FIELD
    pInfo->memc[0].deviceWidth = computeMemWidth(BCHP_GET_FIELD_DATA(regValue, MEMC_0_WIDTH_REG, MEMC_0_DEVICE_WIDTH_FIELD));
#else
    pInfo->memc[0].deviceWidth = 16;
#endif
    regValue = BREG_Read32(hReg, BCHP_MEMC_0_DEVICE_TECH_REG);
    pInfo->memc[0].deviceTech = computeDeviceTech(BCHP_GET_FIELD_DATA(regValue, MEMC_0_DEVICE_TECH_REG, MEMC_0_DEVICE_TECH_FIELD));
#if defined MEMC_0_GROUPAGE_ENABLED_FIELD
    pInfo->memc[0].groupageEnabled = BCHP_GET_FIELD_DATA(regValue, MEMC_0_DEVICE_TECH_REG, MEMC_0_GROUPAGE_ENABLED_FIELD);
#endif
    regValue = BREG_Read32(hReg, BCHP_MEMC_0_DRAM_TYPE_REG);
    pInfo->memc[0].type = computeDramDeviceType(BCHP_GET_FIELD_DATA(regValue, MEMC_0_DRAM_TYPE_REG, MEMC_0_DRAM_TYPE_FIELD));
    pInfo->memc[0].ddr3Capable = (pInfo->memc[0].type == BCHP_DramType_eDDR3);

    /* get memc SCB/MAP version and blindshuffle */
    /* MEMC_GEN_0_CORE_REV_ID tells SCB version and MAP version for shuffle:
    if (coreID < B1.0) {
        scbVersion = SCB4; blindShuffle = false;
        mapVersion = MAP2; shuffleMap = NoShuffle;
    } else if (coreID < B2.0) {
        scbVersion = SCB5; blindShuffle = false;
        mapVersion = MAP5; shuffleMap = ShuffleMap5;
    } else {
        scbVersion = SCB8; blindShuffle = true;
        switch(DRAM_MAP_ENCODING) {
        case 0: mapVersion = MAP2: shuffleMap = NoShuffle; break;
        case 1: mapVersion = MAP5: shuffleMap = ShuffleMap5; break;
        case 2: mapVersion = MAP8: shuffleMap = ShuffleMap8; break;
      }
    }
     */
    BCHP_P_GetMemoryShuffleInfo(hReg, 0, 0, pInfo);

#if defined MEMC_1_WIDTH_REG
    regValue = BREG_Read32(hReg, BCHP_MEMC_1_WIDTH_REG);
    pInfo->memc[1].width = computeMemWidth(BCHP_GET_FIELD_DATA(regValue, MEMC_1_WIDTH_REG, MEMC_1_WIDTH_FIELD));
#if defined MEMC_1_DEVICE_WIDTH_FIELD
    pInfo->memc[1].deviceWidth = computeMemWidth(BCHP_GET_FIELD_DATA(regValue, MEMC_1_WIDTH_REG, MEMC_1_DEVICE_WIDTH_FIELD));
#else
    pInfo->memc[1].deviceWidth = 16;
#endif
    regValue = BREG_Read32(hReg, BCHP_MEMC_1_DEVICE_TECH_REG);
    pInfo->memc[1].deviceTech = computeDeviceTech(BCHP_GET_FIELD_DATA(regValue, MEMC_1_DEVICE_TECH_REG, MEMC_1_DEVICE_TECH_FIELD));
#if defined MEMC_1_GROUPAGE_ENABLED_FIELD
    pInfo->memc[1].groupageEnabled = BCHP_GET_FIELD_DATA(regValue, MEMC_1_DEVICE_TECH_REG, MEMC_1_GROUPAGE_ENABLED_FIELD);
#endif
#if defined BCHP_MEMC_1_DRAM_TYPE_REG
    regValue = BREG_Read32(hReg, BCHP_MEMC_1_DRAM_TYPE_REG);
    pInfo->memc[1].type = computeDramDeviceType(BCHP_GET_FIELD_DATA(regValue, MEMC_1_DRAM_TYPE_REG, MEMC_1_DRAM_TYPE_FIELD));
    pInfo->memc[1].ddr3Capable = (pInfo->memc[1].type == BCHP_DramType_eDDR3);
#endif
#if defined BCHP_MEMC_GEN_1_CORE_REV_ID
    BCHP_P_GetMemoryShuffleInfo(hReg, 1, BCHP_MEMC_GEN_1_CORE_REV_ID - BCHP_MEMC_GEN_0_CORE_REV_ID, pInfo);
#endif
#endif

#if defined MEMC_2_WIDTH_REG
    regValue = BREG_Read32(hReg, BCHP_MEMC_2_WIDTH_REG);
    pInfo->memc[2].width = computeMemWidth(BCHP_GET_FIELD_DATA(regValue, MEMC_2_WIDTH_REG, MEMC_2_WIDTH_FIELD));
    pInfo->memc[2].deviceWidth = computeMemWidth(BCHP_GET_FIELD_DATA(regValue, MEMC_2_WIDTH_REG, MEMC_2_DEVICE_WIDTH_FIELD));
    regValue = BREG_Read32(hReg, BCHP_MEMC_2_DEVICE_TECH_REG);
    pInfo->memc[2].deviceTech = computeDeviceTech(BCHP_GET_FIELD_DATA(regValue, MEMC_2_DEVICE_TECH_REG, MEMC_2_DEVICE_TECH_FIELD));
    pInfo->memc[2].groupageEnabled = BCHP_GET_FIELD_DATA(regValue, MEMC_2_DEVICE_TECH_REG, MEMC_2_GROUPAGE_ENABLED_FIELD);
    regValue = BREG_Read32(hReg, BCHP_MEMC_2_DRAM_TYPE_REG);
    pInfo->memc[2].type = computeDramDeviceType(BCHP_GET_FIELD_DATA(regValue, MEMC_2_DRAM_TYPE_REG, MEMC_2_DRAM_TYPE_FIELD));
    pInfo->memc[2].ddr3Capable = (pInfo->memc[2].type == BCHP_DramType_eDDR3);
#if defined BCHP_MEMC_GEN_2_CORE_REV_ID
    BCHP_P_GetMemoryShuffleInfo(hReg, 2, BCHP_MEMC_GEN_2_CORE_REV_ID - BCHP_MEMC_GEN_0_CORE_REV_ID, pInfo);
#endif
#endif

    for (i=0;i < (sizeof(pInfo->memc)/sizeof(pInfo->memc[0])) && pInfo->memc[i].width;i++) {
        BDBG_ASSERT(pInfo->memc[i].width >= pInfo->memc[i].deviceWidth);
        pInfo->memc[i].valid = (pInfo->memc[i].width != 0);
        BDBG_MSG(("MEMC%d: %d bits", i, pInfo->memc[i].width));
        BDBG_MSG(("mapVer: %d, blindShuffle: %d", pInfo->memc[i].mapVer, pInfo->memc[i].blindShuffle));
    }
#else
    /* Generic MEMC */
    pInfo->memc[0].width = 0x20;
    pInfo->memc[0].type = BCHP_DramType_eDDR3;
    pInfo->memc[0].deviceTech = 0x1000;
    pInfo->memc[0].ddr3Capable = 1;
#endif
    BCHP_P_GetStripeMemoryInfo(pInfo);
    return BERR_SUCCESS;
}

BERR_Code BCHP_GetMemoryInfo(BCHP_Handle hChip, BCHP_MemoryInfo *pInfo)
{
    BERR_Code rc = BERR_SUCCESS;
#if BCHP_UNIFIED_IMPL
    unsigned i;
#endif
    if (!hChip->memoryInfoSet) {
        rc = BCHP_GetMemoryInfo_PreInit(hChip->regHandle, &hChip->memoryInfo);
        if (rc) return BERR_TRACE(rc);
        hChip->memoryInfoSet = true;
    }
    BKNI_Memcpy((void*)pInfo, (void*)&hChip->memoryInfo, sizeof(BCHP_MemoryInfo));
#if BCHP_UNIFIED_IMPL
    for (i=0;i < (sizeof(pInfo->memc)/sizeof(pInfo->memc[0])) && pInfo->memc[i].width;i++) {
        if (hChip->openSettings.memoryLayout.memc[i].size == 0) {
            pInfo->memc[i].valid = false;
        }
    }
#endif
    return rc;
}


BSTD_DeviceOffset BCHP_ShuffleStripedPixelOffset(BCHP_Handle hChp, unsigned memcIdx, unsigned offset)
{
    BCHP_MemoryInfo *pInfo = &hChp->memoryInfo;
    bool scbBitExtract = false; \
    unsigned shift = (pInfo->memc[memcIdx].ulStripeWidth == 256)? 9 : 8;

    /* SCB8.0/MAP8.0: When (ByteAddr[8] ^ ByteAddr[9]) == 1, ByteAddr[5] is inverted */
    if(pInfo->memc[memcIdx].mapVer == BCHP_ScbMapVer_eMap8) {
        scbBitExtract = (((offset)>>9) & 1) ^ (((offset)>>8) & 1);
    }
    /* MAP5.0:
        For 128B stripe width, When GWORD[4](or ByteAddr[8]) == 1, ByteAddr[5] is inverted;
        For 256B stripe width, When GWORD[5](or ByteAddr[9]) == 1, ByteAddr[5] is inverted;*/
    else if(pInfo->memc[memcIdx].mapVer == BCHP_ScbMapVer_eMap5) {
        scbBitExtract = ((offset)>>shift) & 1;
    }
    if(scbBitExtract) offset ^= 0x20; /* GWORD[1] (or byteAddr[5]) inversion, i.e., shuffle-map */
    /* blind shuffle applied to SCB8/MAP8 always: swap DWord order within JWord. */
    if(pInfo->memc[memcIdx].blindShuffle) offset =
        ((offset) & (~(BSTD_DeviceOffset)0x1F)) +
        ((offset) & 3) +
        0x1C - ((offset) & 0x1C);
    return offset;
}

BERR_Code BCHP_P_GetDefaultFeature
    ( const BCHP_Handle                hChip,
      const BCHP_Feature               eFeature,
      void                            *pFeatureValue )
{
    BERR_Code rc = BERR_SUCCESS;
    if (!hChip->memoryInfoSet) {
        if (!hChip->regHandle) {
            return BERR_TRACE(BERR_NOT_AVAILABLE);
        }
        rc = BCHP_GetMemoryInfo(hChip, &hChip->memoryInfo);
        if (rc) return BERR_TRACE(rc);
        hChip->memoryInfoSet = true;
    }
    switch (eFeature) {
    case BCHP_Feature_eMemCtrl0DDRDeviceTechCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[0].deviceTech;
        break;
    case BCHP_Feature_eMemCtrl0DramWidthCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[0].width;
        break;
    case BCHP_Feature_eMemCtrl0DDR3ModeCapable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[0].ddr3Capable;
        break;
    case BCHP_Feature_eMemCtrl1Capable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[1].valid;
        break;
    case BCHP_Feature_eMemCtrl1DDRDeviceTechCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[1].deviceTech;
        break;
    case BCHP_Feature_eMemCtrl1DramWidthCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[1].width;
        break;
    case BCHP_Feature_eMemCtrl1DDR3ModeCapable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[1].ddr3Capable;
        break;
    case BCHP_Feature_eMemCtrl2Capable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[2].valid;
        break;
    case BCHP_Feature_eMemCtrl2DDRDeviceTechCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[2].deviceTech;
        break;
    case BCHP_Feature_eMemCtrl2DramWidthCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[2].width;
        break;
    case BCHP_Feature_eMemCtrl2DDR3ModeCapable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[2].ddr3Capable;
        break;
    case BCHP_Feature_eProductId:
        {
            BCHP_Info info;
            BCHP_GetInfo(hChip, &info);
            ((BCHP_FeatureData*)pFeatureValue)->data.productId = info.productId;
        }
        break;
    case BCHP_Feature_eDisabledL2Registers:
        /* by default, none are disabled */
        BKNI_Memset(pFeatureValue, 0, sizeof(BCHP_FeatureData));
        break;

    /* Default bool features to false */
    case BCHP_Feature_e3DGraphicsCapable:
    case BCHP_Feature_eDvoPortCapable:
    case BCHP_Feature_eMacrovisionCapable:
    case BCHP_Feature_eHdcpCapable:
    case BCHP_Feature_e3desCapable:
    case BCHP_Feature_e1080pCapable:
    case BCHP_Feature_eRfmCapable:
       *(bool *)pFeatureValue = false;
       break;

     /* Default int features to 0 */
    case BCHP_Feature_eMpegDecoderCount:
    case BCHP_Feature_eAVDCoreFreq:
       *(int *)pFeatureValue = 0;
       break;

    default:
        rc = BERR_TRACE(BERR_UNKNOWN);
        break;
    }
    return rc;
}

void BCHP_GetInfo( BCHP_Handle hChip, BCHP_Info *pInfo )
{
    *pInfo = hChip->info;
}

/* decompose 32 bit chip family id for use with printf format string %x%c%d
Example: 0x74450000 becomes "7445A0" */
#define PRINT_CHIP(CHIPID) \
    ((CHIPID)&0xF0000000?(CHIPID)>>16:(CHIPID)>>8), ((((CHIPID)&0xF0)>>4)+'A'), ((CHIPID)&0x0F)

static BERR_Code BCHP_P_VerifyChip(BCHP_Handle hChip, const struct BCHP_P_Info *pChipInfo, unsigned chipInfoSize)
{
    unsigned i;
    unsigned ulChipFamilyIdReg;

    ulChipFamilyIdReg = BREG_Read32(hChip->regHandle, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);
    if (chipInfoSize == 0) {
        while (pChipInfo[chipInfoSize].ulChipFamilyIdReg) chipInfoSize++;
    }
    for (i=0; i<chipInfoSize; i++) {
        BDBG_MSG(("Supported chip family and revision: %x%c%d", PRINT_CHIP(pChipInfo[i].ulChipFamilyIdReg)));
    }
    for(i = 0; i<chipInfoSize; i++) {
        if (pChipInfo[i].ulChipFamilyIdReg == ulChipFamilyIdReg) {
            hChip->pChipInfo = &pChipInfo[i];
            break;
        }
    }
    if(!hChip->pChipInfo) {
        BDBG_ERR(("*****************************************************************"));
        BDBG_ERR(("ERROR ERROR ERROR ERROR"));
        BDBG_ERR(("Unsupported chip: %x%c%d", PRINT_CHIP(ulChipFamilyIdReg)));
        BDBG_ERR(("*****************************************************************"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("found %x%c%d", PRINT_CHIP(hChip->pChipInfo->ulChipFamilyIdReg)));
    return 0;
}

#include "bchp_clkgen.h"
void BCHP_P_MuxSelect(BCHP_Handle hChip)
{
#if BCHP_CHIP == 7543 || BCHP_CHIP == 7552 || BCHP_CHIP == 7360 || BCHP_CHIP == 7228 || BCHP_CHIP == 7358 || BCHP_CHIP == 7362 || BCHP_CHIP == 73625
    uint32_t ulVal;

    ulVal = BREG_Read32 (hChip->regHandle, BCHP_CLKGEN_INTERNAL_MUX_SELECT);
#if BCHP_CHIP == 7543
    ulVal |=  (BCHP_FIELD_DATA(CLKGEN_INTERNAL_MUX_SELECT, M2MC_CORE_CLOCK, 0));
#else
    ulVal |=  (BCHP_FIELD_DATA(CLKGEN_INTERNAL_MUX_SELECT, GFX_M2MC_CORE_CLOCK, 1));
#endif
    BREG_Write32(hChip->regHandle, BCHP_CLKGEN_INTERNAL_MUX_SELECT, ulVal);
#else
    BSTD_UNUSED(hChip);
#endif
}

static void BCHP_P_StandbyMode( BCHP_Handle hChip, bool activate )
{
    BDBG_OBJECT_ASSERT(hChip, BCHP);

    /* Do anything required for CHP Standby changes */

    if (hChip->hAvsHandle)
        BCHP_P_AvsStandbyMode(hChip->hAvsHandle, activate);

    BCHP_P_MuxSelect(hChip);
}

#if BCHP_UNIFIED_IMPL
BCHP_Handle BCHP_P_Open(const BCHP_OpenSettings *pSettings, const struct BCHP_P_Info *pChipInfo)
{
    BCHP_Handle pChip;
    BERR_Code rc;
    unsigned val;

    pChip = BKNI_Malloc(sizeof(*pChip));
    if(!pChip) {
#if BDBG_DEBUG_BUILD || B_REFSW_DEBUG_COMPACT_ERR
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
#endif
        return NULL;
    }
    BKNI_Memset(pChip, 0x0, sizeof(*pChip));
    BDBG_OBJECT_SET(pChip, BCHP);
    pChip->openSettings     = *pSettings;
    pChip->regHandle        = pSettings->reg;

    rc = BCHP_P_VerifyChip(pChip, pChipInfo, 0);
    if (rc) {
        BKNI_Free(pChip);
#if BDBG_DEBUG_BUILD || B_REFSW_DEBUG_COMPACT_ERR
        BERR_TRACE(rc);
#endif
        return NULL;
    }

    /* read BCHP_Info once */
    val = BREG_Read32(pChip->regHandle, BCHP_SUN_TOP_CTRL_PRODUCT_ID);
    if (!val) {
        /* if PRODUCT_ID not burned in OTP, use FAMILY_ID */
        val = BREG_Read32(pChip->regHandle, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);
    }
    pChip->info.rev = val & 0xFF;
    if (val & 0xF0000000) {
        /* 4 digit */
        pChip->info.productId = val >> 16;
    }
    else {
        /* 5 digit */
        pChip->info.productId = val >> 8;
    }
    if (pChip->openSettings.productId) {
        pChip->info.productId = pChip->openSettings.productId;
        BDBG_WRN(("overriding PRODUCT_ID to %#x", pChip->info.productId));
    }
    /* TODO: override productId as needed */
    val = BREG_Read32(pChip->regHandle, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);
    if (val & 0xF0000000) {
        /* 4 digit */
        pChip->info.familyId = val >> 16;
    }
    else {
        /* 5 digit */
        pChip->info.familyId = val >> 8;
    }

    BCHP_GetMemoryInfo(pChip, &pChip->memoryInfo);
    return pChip;
}

void BCHP_GetDefaultOpenSettings( BCHP_OpenSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/* BCHP_Open must be implemented in each bchp_xxxx.c file */
#else
static void BCHP_P_SetInfo( BCHP_Handle hChip )
{
    unsigned val;
#ifdef BCHP_SUN_TOP_CTRL_PRODUCT_ID
    val = BREG_Read32(hChip->regHandle, BCHP_SUN_TOP_CTRL_PRODUCT_ID);
    if (!val) {
        val = BREG_Read32(hChip->regHandle, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);
    }
#else
    val = BREG_Read32(hChip->regHandle, BCHP_SUN_TOP_CTRL_PROD_REVISION);
#endif
    hChip->info.rev = val & 0xFF;
    hChip->info.productId = val >> 16;
#ifdef BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID
    hChip->info.familyId = BREG_Read32(hChip->regHandle, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID) >> 16;
#else
    hChip->info.familyId = hChip->info.productId;
#endif
}
BCHP_Handle BCHP_P_Open(BREG_Handle hRegister, const BCHP_P_Info *pChipInfo, unsigned chipInfoSize)
{
    BERR_Code rc;
    BCHP_Handle pChip;

    pChip = BKNI_Malloc(sizeof(*pChip));
    if (!pChip) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(pChip, 0x0, sizeof(*pChip));
    BDBG_OBJECT_SET(pChip, BCHP);
    pChip->regHandle = hRegister;

    rc = BCHP_P_VerifyChip(pChip, pChipInfo, chipInfoSize);
    if (rc) {
        BKNI_Free(pChip);
        return NULL;
    }
    BCHP_P_SetInfo( pChip );
    return pChip;
}
#endif

BDBG_OBJECT_ID(BCHP);

bool BCHP_OffsetOnMemc( BCHP_Handle chp, BSTD_DeviceOffset offset, unsigned memcIndex )
{
#if BCHP_UNIFIED_IMPL
    unsigned i;

    if (memcIndex > 2) {
       return false; /* There are a max of 3 mem controllers supported. */
    }
    for (i=0;i<BCHP_MAX_MEMC_REGIONS;i++) {
        if (offset >= chp->openSettings.memoryLayout.memc[memcIndex].region[i].addr &&
            offset < chp->openSettings.memoryLayout.memc[memcIndex].region[i].addr + chp->openSettings.memoryLayout.memc[memcIndex].region[i].size) return true;
    }
    return false;
#else
    BSTD_UNUSED(chp);
    BSTD_UNUSED(offset);
    BSTD_UNUSED(memcIndex);
    /* can't know */
    return true;
#endif
}

/* Remapped BCHP_MEMC_ARB_0_CLIENT_INFO_0 to regstruct array base. */
#include "bchp_memc_arb_rdb_remap.h"

BERR_Code BCHP_GetMemcClientConfig(
   BCHP_Handle hChp,
   unsigned memcIndex,
   unsigned clientIndex,
   BCHP_MemClientConfig *cfg
   )
{
   uint32_t reg, addr;

   BDBG_ASSERT(cfg);

   BKNI_Memset(cfg, 0, sizeof(*cfg));
   switch(memcIndex)
   {
      case 0: addr = BCHP_MEMC_ARB_0_CLIENT_INFO_0 + 4 * clientIndex; break;

      #ifdef BCHP_MEMC_ARB_1_CLIENT_INFO_0
      case 1: addr = BCHP_MEMC_ARB_1_CLIENT_INFO_0 + 4 * clientIndex; break;
      #endif

      #ifdef BCHP_MEMC_ARB_2_CLIENT_INFO_0
      case 2: addr = BCHP_MEMC_ARB_2_CLIENT_INFO_0 + 4 * clientIndex; break;
      #endif

      default: BDBG_ERR(("Unknown MEMC")); return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   reg = BREG_Read32(hChp->regHandle, addr);
   cfg->roundRobinEn = BCHP_GET_FIELD_DATA(reg, MEMC_ARB_0_CLIENT_INFO_0, RR_EN);
   cfg->blockout = BCHP_GET_FIELD_DATA(reg, MEMC_ARB_0_CLIENT_INFO_0, BO_VAL);
   return BERR_SUCCESS;
}

bool BCHP_SkipInitialReset(BCHP_Handle chp)
{
    return chp->skipInitialReset;
}

/* BP3 Do NOT Modify Start */
#if defined(BCHP_SCPU_GLOBALRAM_REG_START)
#include "bchp_scpu_globalram.h"
#if (defined BHSM_ZEUS_VER_MAJOR) && (defined BCHP_SAGE_SUPPORT)
#include "priv/bsagelib_shared_globalsram.h"
#endif
struct BCHP_P_LicenseLeaf {
    BCHP_LicensedFeature feature;
    unsigned bit;
};

struct BCHP_P_LicenseNode {
    uint32_t addr;
    unsigned leafs;
    const struct BCHP_P_LicenseLeaf *leaf;
};

#endif /* #if defined(BCHP_SCPU_GLOBALRAM_REG_START) */

#if (BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_END>255) /* Zeus 5.x or above */
#define BCHP_P_SAGE_IPLICENSING_BASE_OFFSET 0x22
#else
#define BCHP_P_SAGE_IPLICENSING_BASE_OFFSET 0x14
#endif
#define BCHP_P_SAGE_IPLICENSING_HOST_OFFSET  (BCHP_P_SAGE_IPLICENSING_BASE_OFFSET+3)
#define BCHP_P_SAGE_IPLICENSING_AUDIO_OFFSET (BCHP_P_SAGE_IPLICENSING_BASE_OFFSET+2)

BERR_Code BCHP_HasLicensedFeature_isrsafe(BCHP_Handle chp, BCHP_LicensedFeature feature)
{
#if defined(BCHP_SCPU_GLOBALRAM_REG_START)
    static const struct BCHP_P_LicenseLeaf BCHP_P_SCPU_GLOBALRAM_DMEM_LeafsHost[] = {
        {BCHP_LicensedFeature_eMacrovision, 0},
        {BCHP_LicensedFeature_eDolbyVision, 1},
        {BCHP_LicensedFeature_eTchPrime, 2},
        {BCHP_LicensedFeature_eItm, 3}
    };

    static const struct BCHP_P_LicenseLeaf BCHP_P_SCPU_GLOBALRAM_DMEM_LeafsAudio[] = {
        {BCHP_LicensedFeature_eDap, 0},
        {BCHP_LicensedFeature_eDolbyDigital, 1},
        {BCHP_LicensedFeature_eDolbyDigitalPlus, 2},
        {BCHP_LicensedFeature_eDolbyAc4, 3},
        {BCHP_LicensedFeature_eDolbyTrueHd, 4},
        {BCHP_LicensedFeature_eDolbyMS10_11, 5},
        {BCHP_LicensedFeature_eDolbyMS12v1, 6},
        {BCHP_LicensedFeature_eDolbyMS12v2, 7}
    };

    static const struct BCHP_P_LicenseNode BCHP_P_LicenseNodes[] = {
        {
            BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + BCHP_P_SAGE_IPLICENSING_HOST_OFFSET*4,
            sizeof(BCHP_P_SCPU_GLOBALRAM_DMEM_LeafsHost)/sizeof(BCHP_P_SCPU_GLOBALRAM_DMEM_LeafsHost[0]),
            BCHP_P_SCPU_GLOBALRAM_DMEM_LeafsHost
        },
        {
            BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + BCHP_P_SAGE_IPLICENSING_AUDIO_OFFSET*4,
            sizeof(BCHP_P_SCPU_GLOBALRAM_DMEM_LeafsAudio)/sizeof(BCHP_P_SCPU_GLOBALRAM_DMEM_LeafsAudio[0]),
            BCHP_P_SCPU_GLOBALRAM_DMEM_LeafsAudio
        }
    };
    unsigned node;
    BDBG_OBJECT_ASSERT(chp, BCHP);
#if (defined BHSM_ZEUS_VER_MAJOR) && (defined BCHP_SAGE_SUPPORT) && (defined GlobalSram_IPLicensing_Info)
    BDBG_CASSERT(GlobalSram_IPLicensing_Info==BCHP_P_SAGE_IPLICENSING_BASE_OFFSET);
    BDBG_CASSERT(BSAGElib_GlobalSram_eBP3HostFeatureList==BCHP_P_SAGE_IPLICENSING_HOST_OFFSET);
    BDBG_CASSERT(BSAGElib_GlobalSram_eBP3AudioFeatureList0==BCHP_P_SAGE_IPLICENSING_AUDIO_OFFSET);
#endif
    for(node=0;node<sizeof(BCHP_P_LicenseNodes)/sizeof(BCHP_P_LicenseNodes[0]);node++) {
        unsigned i;
        unsigned n = BCHP_P_LicenseNodes[node].leafs;
        const struct BCHP_P_LicenseLeaf *leaf = BCHP_P_LicenseNodes[node].leaf;
        for(i=0;i<n;i++) {
            if(leaf[i].feature==feature) {
                uint32_t data = BREG_Read32(chp->regHandle, BCHP_P_LicenseNodes[node].addr);
                BERR_Code result = (((data >> leaf[i].bit) & 1) == 0) ? BERR_SUCCESS:BERR_NOT_AVAILABLE;
                return result;
            }
        }
    }
#else /* #if defined(BCHP_SCPU_GLOBALRAM_REG_START) */
    BSTD_UNUSED(feature);
    BSTD_UNUSED(chp);
#endif /* #if defined(BCHP_SCPU_GLOBALRAM_REG_START) */
    return BERR_SUCCESS;
}
/* BP3 Do NOT Modify End */

/* end of file */
