/******************************************************************************
 * (c) 2003-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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


BERR_Code BCHP_Close(
    BCHP_Handle hChip               /* [in] Chip handle */
    )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BCHP_Close );
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hChip->pCloseFunc != NULL );
    rc = hChip->pCloseFunc( hChip );
    BDBG_LEAVE( BCHP_Close );
    return( rc );
}

BERR_Code BCHP_GetChipInfo(
    const BCHP_Handle hChip,            /* [in] Chip handle */
    uint16_t *pChipId,                  /* [out] Chip Id */
    uint16_t *pChipRev                  /* [out] Chip Rev. */
    )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BCHP_GetChipInfo );
    BDBG_ASSERT( hChip );
    if (!hChip->pGetChipInfoFunc) {
        BCHP_Info info;
        BCHP_GetInfo(hChip, &info);
        *pChipId = info.familyId;
        *pChipRev = info.rev;
    }
    else {
        rc = hChip->pGetChipInfoFunc( hChip, pChipId, pChipRev );
    }
    BDBG_LEAVE( BCHP_GetChipInfo );
    return( rc );
}

BERR_Code BCHP_GetFeature(
    const BCHP_Handle hChip,            /* [in] Chip handle */
    const BCHP_Feature eFeature,        /* [in] Feature to be queried. */
    void *pFeatureValue                 /* [out] Feature value. */
    )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BCHP_GetFeature );
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( pFeatureValue );

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
    BDBG_ENTER( BCHP_MonitorPvt );
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( pSettings );

    if (hChip->pMonitorPvtFunc)
        hChip->pMonitorPvtFunc(hChip, pSettings);

    BDBG_LEAVE( BCHP_MonitorPvt );
}

BERR_Code BCHP_GetAvsData_isrsafe(
    BCHP_Handle hChip,   /* [in] Chip handle */
    BCHP_AvsData *pData) /* [out] pointer to location to return data */
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BCHP_GetAvsData );
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( pData );

    if (hChip->pGetAvsDataFunc)
        rc = hChip->pGetAvsDataFunc(hChip, pData);
    else
        rc = BERR_TRACE(BERR_UNKNOWN);

    BDBG_LEAVE( BCHP_GetAvsData );
    return( rc );
}

BERR_Code BCHP_Standby(
    BCHP_Handle hChip)   /* [in] Chip handle */
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BCHP_Standby );
    BDBG_ASSERT( hChip );

    if (hChip->pStandbyModeFunc)
        rc = hChip->pStandbyModeFunc(hChip, true);

    BDBG_LEAVE( BCHP_Standby );
    return( rc );
}

BERR_Code BCHP_Resume(
    BCHP_Handle hChip)  /* [in] Chip handle */
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BCHP_Resume );
    BDBG_ASSERT( hChip );

    if (hChip->pStandbyModeFunc)
        rc = hChip->pStandbyModeFunc(hChip, false);

    BDBG_LEAVE( BCHP_Resume );
    return( rc );
}

/* For testing purposes only */
void *BCHP_GetAvsHandle(
    BCHP_Handle hChip)   /* [in] Chip handle */
{
    void *result;

    BDBG_ENTER( BCHP_GetAvsHandle );
    BDBG_ASSERT( hChip );

    result = (void*)hChip->avsHandle; /* will be NULL for platforms without AVS support */

    BDBG_LEAVE( BCHP_GetAvsHandle );
    return( result );
}

#if BCHP_CHIP == 7325 || BCHP_CHIP == 7335 || BCHP_CHIP == 7336 || BCHP_CHIP == 7400 || BCHP_CHIP == 7403 || BCHP_CHIP == 7405
/* this silicon uses SUN_TOP_CTRL_STRAP_VALUE_0 and its strap_ddr_configuration, strap_ddr0_device_config, and
strap_ddr1_device_config fields which are not generic. */
BERR_Code BCHP_GetMemoryInfo(BREG_Handle hReg, BCHP_MemoryInfo *pInfo)
{
    BSTD_UNUSED(hReg);

    pInfo->memc[0].offset = BCHP_P_MEMC_0_OFFSET;
    pInfo->memc[0].ulStripeWidth = BCHP_INVALID_STRIPE_WIDTH;

#ifdef BCHP_S_MEMC_1_REG_START
    pInfo->memc[1].offset = BCHP_P_MEMC_1_OFFSET;
    pInfo->memc[1].ulStripeWidth = BCHP_INVALID_STRIPE_WIDTH;
#endif

#ifdef BCHP_S_MEMC_2_REG_START
    pInfo->memc[2].offset = BCHP_P_MEMC_2_OFFSET;
    pInfo->memc[2].ulStripeWidth = BCHP_INVALID_STRIPE_WIDTH;
#endif

    return BERR_SUCCESS;
}
BERR_Code BCHP_P_GetDefaultFeature(const BCHP_Handle hChip, const BCHP_Feature eFeature, void *pFeatureValue)
{
    BSTD_UNUSED(hChip);
    BSTD_UNUSED(eFeature);
    BSTD_UNUSED(pFeatureValue);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#else

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
       case BCHP_DramType_eLPDDR4:
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
BERR_Code BCHP_GetMemoryInfo(BREG_Handle hReg, BCHP_MemoryInfo *pInfo)
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
#endif
    pInfo->memc[0].offset = BCHP_P_MEMC_0_OFFSET;
#if BCHP_CHIP != 11360
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
    pInfo->memc[1].offset = BCHP_P_MEMC_1_OFFSET;
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
    pInfo->memc[2].offset = BCHP_P_MEMC_2_OFFSET;
#endif

#if BCHP_CHIP == 7445
/* TEMP: 7445 2 MEMC bondouts have bad reset value for MEMC2 size */
    switch (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PRODUCT_ID) >> 8) {
    case 0x725200:
    case 0x72520:
    case 0x744800:
    case 0x74480:
    case 0x744900:
    case 0x74490:
        pInfo->memc[2].width = 0; /* will short circuit for loop below */
        pInfo->memc[2].offset = 0;
		break;
	default:
		break;
    }
#elif BCHP_CHIP==7439
    switch (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PRODUCT_ID) >> 8) {
       case 0x725110:
       case 0x72511:
         pInfo->memc[1].width = 0; /* will short circuit for loop below */
         pInfo->memc[1].offset = 0;
         break;
      default:
         break;
    }
#endif
    for (i=0;i < (sizeof(pInfo->memc)/sizeof(pInfo->memc[0])) && pInfo->memc[i].width;i++) {
        BDBG_ASSERT(pInfo->memc[i].width >= pInfo->memc[i].deviceWidth);
        pInfo->memc[i].size = pInfo->memc[i].deviceTech / 8 * (pInfo->memc[i].width/pInfo->memc[i].deviceWidth) * 1024 * 1024;
        BDBG_MSG(("MEMC%d: offset " BDBG_UINT64_FMT " %d MB, %d bits", i, BDBG_UINT64_ARG(pInfo->memc[i].offset), (unsigned)(pInfo->memc[i].size / 1024 / 1024), pInfo->memc[i].width));
    }
#else
    /* Generic MEMC */
    pInfo->memc[0].width = 0x20;
    pInfo->memc[0].type = BCHP_DramType_eDDR3;
    pInfo->memc[0].deviceTech = 0x1000;
    pInfo->memc[0].ddr3Capable = 1;
    pInfo->memc[0].size = 0xffffffff;
#endif
    BCHP_P_GetStripeMemoryInfo(pInfo);
    return BERR_SUCCESS;
}

BERR_Code BCHP_P_GetDefaultFeature
    ( const BCHP_Handle                hChip,
      const BCHP_Feature               eFeature,
      void                            *pFeatureValue )
{
    BERR_Code rc;
    if (!hChip->memoryInfoSet) {
        if (!hChip->regHandle) {
            return BERR_TRACE(BERR_NOT_AVAILABLE);
        }
        rc = BCHP_GetMemoryInfo(hChip->regHandle, &hChip->memoryInfo);
        if (rc) return BERR_TRACE(rc);
        hChip->memoryInfoSet = true;
    }
    switch (eFeature) {
    case BCHP_Feature_eMemCtrl0DDRDeviceTechCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[0].deviceTech;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl0DramWidthCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[0].width;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl0DDR3ModeCapable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[0].ddr3Capable;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl1Capable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[1].size != 0;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl1DDRDeviceTechCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[1].deviceTech;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl1DramWidthCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[1].width;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl1DDR3ModeCapable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[1].ddr3Capable;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl2Capable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[2].size != 0;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl2DDRDeviceTechCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[2].deviceTech;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl2DramWidthCount:
        *(int *)pFeatureValue = hChip->memoryInfo.memc[2].width;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eMemCtrl2DDR3ModeCapable:
        *(bool *)pFeatureValue = hChip->memoryInfo.memc[2].ddr3Capable;
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eProductId:
        {
            BCHP_Info info;
            BCHP_GetInfo(hChip, &info);
            ((BCHP_FeatureData*)pFeatureValue)->data.productId = info.productId;
        }
        rc = BERR_SUCCESS;
        break;
    case BCHP_Feature_eDisabledL2Registers:
        /* by default, none are disabled */
        BKNI_Memset(pFeatureValue, 0, sizeof(BCHP_FeatureData));
        rc = BERR_SUCCESS;
        break;
    default:
        rc = BERR_TRACE(BERR_UNKNOWN);
        break;
    }
    return rc;
}
#endif

void BCHP_GetInfo( BCHP_Handle hChip, BCHP_Info *pInfo )
{
#if !BCHP_UNIFIED_IMPL
/* 40nm and 65nm: only 16 bit ids */
    if (!hChip->infoSet) {
        unsigned val;
        if (!hChip->regHandle) {
            BERR_TRACE(BERR_NOT_AVAILABLE);
            BKNI_Memset(pInfo, 0, sizeof(*pInfo));
            return;
        }
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
        hChip->infoSet = true;
    }
#endif
    *pInfo = hChip->info;
}

#if BCHP_UNIFIED_IMPL
/* decompose 32 bit chip family id for use with printf format string %x%c%d
Example: 0x74450000 becomes "7445A0" */
#define PRINT_CHIP(CHIPID) \
    ((CHIPID)&0xF0000000?(CHIPID)>>16:(CHIPID)>>8), ((((CHIPID)&0xF0)>>4)+'A'), ((CHIPID)&0x0F)

static BERR_Code BCHP_P_VerifyChip(BCHP_Handle hChip, const struct BCHP_P_Info *pChipInfo)
{
    unsigned i;
    unsigned ulChipFamilyIdReg;

    ulChipFamilyIdReg = BREG_Read32(hChip->regHandle, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);
    for (i=0; pChipInfo[i].ulChipFamilyIdReg; i++) {
        BDBG_MSG(("Supported chip family and revision: %x%c%d", PRINT_CHIP(pChipInfo[i].ulChipFamilyIdReg)));
    }
    for(i = 0; pChipInfo[i].ulChipFamilyIdReg; i++) {
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

static BERR_Code BCHP_P_Close( BCHP_Handle hChip )
{
    BDBG_ENTER(BCHP_P_Close);
    BDBG_OBJECT_ASSERT(hChip, BCHP);
    if (hChip->hAvsHandle) {
        BCHP_P_AvsClose(hChip->hAvsHandle);
    }
    BCHP_PWR_Close(hChip->pwrManager);
    BDBG_OBJECT_DESTROY(hChip, BCHP);
    BKNI_Free(hChip);
    BDBG_LEAVE(BCHP_P_Close);
    return BERR_SUCCESS;
}

/* This gets called regularly to handle the AVS processing */
static void BCHP_P_MonitorPvt( BCHP_Handle hChip, BCHP_AvsSettings *pSettings )
{
    BDBG_ENTER(BCHP_P_MonitorPvt);
    BSTD_UNUSED(pSettings);

    BDBG_OBJECT_ASSERT(hChip, BCHP);

    if (hChip->hAvsHandle)
        BCHP_P_AvsMonitorPvt(hChip->hAvsHandle);

    BDBG_LEAVE(BCHP_P_MonitorPvt);
}

static BERR_Code BCHP_P_GetAvsData( BCHP_Handle hChip, BCHP_AvsData *pData )
{
    BDBG_ENTER(BCHP_GetAVdata);
    BDBG_OBJECT_ASSERT(hChip, BCHP);

    if (hChip->hAvsHandle)
        BCHP_P_AvsGetData(hChip->hAvsHandle, pData);

    BDBG_LEAVE(BCHP_GetAVdata);
    return BERR_SUCCESS;
}

static BERR_Code BCHP_P_StandbyMode( BCHP_Handle hChip, bool activate )
{
    BDBG_ENTER(BCHP_P_StandbyMode);
    BDBG_OBJECT_ASSERT(hChip, BCHP);

    /* Do anything required for CHP Standby changes */

    if (hChip->hAvsHandle)
        BCHP_P_AvsStandbyMode(hChip->hAvsHandle, activate);

    BDBG_LEAVE(BCHP_P_StandbyMode);
    return BERR_SUCCESS;
}

BCHP_Handle BCHP_P_Open(const BCHP_OpenSettings *pSettings, const struct BCHP_P_Info *pChipInfo)
{
    BCHP_Handle pChip;
    BERR_Code rc;
    unsigned val;

    pChip = BKNI_Malloc(sizeof(*pChip));
    if(!pChip) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(pChip, 0x0, sizeof(*pChip));
    BDBG_OBJECT_SET(pChip, BCHP);
    pChip->openSettings     = *pSettings;
    pChip->regHandle        = pSettings->reg;
    pChip->pCloseFunc       = BCHP_P_Close;
    pChip->pMonitorPvtFunc  = BCHP_P_MonitorPvt;
    pChip->pGetAvsDataFunc  = BCHP_P_GetAvsData;
    pChip->pStandbyModeFunc = BCHP_P_StandbyMode;

    rc = BCHP_P_VerifyChip(pChip, pChipInfo);
    if (rc) {
        BKNI_Free(pChip);
        BERR_TRACE(rc);
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

    return pChip;
}

void BCHP_GetDefaultOpenSettings( BCHP_OpenSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/* BCHP_Open must be implemented in each bchp_xxxx.c file */
#endif

BDBG_OBJECT_ID(BCHP);

/* end of file */
