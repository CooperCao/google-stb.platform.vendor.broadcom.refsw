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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include "bvce.h"
#include "bvce_priv.h"
#include "bvce_debug_priv.h"
#include "bvce_platform.h"

#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bavc_vce_mbox.h"

/* VCE Instance 0 */
#include "bchp_vice2_misc_0.h"
#include "bchp_vice2_arcss_ess_ctrl_0_0.h"
#include "bchp_vice2_arcss_ess_ctrl_1_0.h"
#include "bchp_vice2_arcss_misc_0.h"
#include "bchp_vice2_arcss_ess_dccm_0_0.h"
#include "bchp_vice2_arcss_ess_dccm_1_0.h"
#include "bchp_vice2_arcss_ess_p1_intr2_0_0.h"
#include "bchp_vice2_l2_0.h"
#include "bchp_vice2_cabac_0_0.h"
#include "bchp_vice2_vip_0_0.h"
#include "bchp_vice2_vip1_0_0.h"
#include "bchp_int_id_vice2_l2_0.h"
/* For Debug */
#include "bchp_vice2_cme_0_0.h"

/* VCE Instance 1 */
#include "bchp_vice2_misc_1.h"
#include "bchp_vice2_arcss_ess_ctrl_0_1.h"
#include "bchp_vice2_arcss_ess_ctrl_1_1.h"
#include "bchp_vice2_arcss_misc_1.h"
#include "bchp_vice2_arcss_ess_dccm_0_1.h"
#include "bchp_vice2_arcss_ess_dccm_1_1.h"
#include "bchp_vice2_l2_1.h"
#include "bchp_vice2_cabac_0_1.h"
#include "bchp_vice2_vip_0_1.h"
#include "bchp_vice2_vip1_0_1.h"
#include "bchp_int_id_vice2_l2_1.h"
#include "bchp_vice2_cme_0_1.h"
#include "bchp_vice2_arcss_ess_hostif_0_0.h"
#include "bchp_vice2_arcss_ess_hostif_1_0.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if (BCHP_VER >= BCHP_VER_C0)
#include "bchp_memc_sentinel_0_0.h"
#include "bchp_memc_sentinel_0_1.h"
#include "bchp_memc_sentinel_0_2.h"
#endif

BDBG_MODULE(BVCE_PLATFORM_7445);

#define BVCE_P_REGISTER_BASE 0x700000
#define BVCE_P_REGISTER_START (BCHP_PHYSICAL_OFFSET + 0x700000)
#define BVCE_P_REGISTER_INSTANCE_OFFSET (BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_SYS_ARC_INSTR_ADDR_OFFSET - BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_ARC_INSTR_ADDR_OFFSET)

#if ( (BVCE_P_CORE_MAJOR >= 3) || ( (BVCE_P_CORE_MAJOR == 2) && (BVCE_P_CORE_MINOR == 1) ) )
static const BVCE_Platform_P_RegisterSetting s_astViceResetRegisterSettings[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][14] =
#else
static const BVCE_Platform_P_RegisterSetting s_astViceResetRegisterSettings[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][13] =
#endif
{
   {
      { "Arc[0] Boot",               BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START,                                    0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START_ENABLE_MASK },
      { "Vice SUNTOP Reset (Set)",   BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,                                                0xFFFFFFFF,             BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_vice20_sw_init_MASK },
      { "Vice SUNTOP Reset (Clear)", BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,                                              0xFFFFFFFF,             BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_vice20_sw_init_MASK },
      { "Vice CPU Speed (MISC_0_SCRATCH_0)", BCHP_VICE2_MISC_0_SCRATCH,                                                0x00000000,             0xFFFFFFFF },
      { "Reg Addr Offset",           BCHP_VICE2_ARCSS_MISC_0_INIT_SYS_REG_ADDR_OFFSET,                                 BCHP_PHYSICAL_OFFSET,   BCHP_VICE2_ARCSS_MISC_0_INIT_SYS_REG_ADDR_OFFSET_OFFSET_MASK },
      { "Arc Config",                BCHP_VICE2_ARCSS_MISC_0_INIT_SYS_ARC_CONFIG,                                      0x00000000,             BCHP_VICE2_ARCSS_MISC_0_INIT_SYS_ARC_CONFIG_SEL_MASK },
      { "Arc[0] Host I/F",           BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_HOST_IF,                                   0xFFFFFFFF,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_HOST_IF_SEL_MASK },
      { "Cache Miss",                BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_BVCI2SCB_BRIDGE_CTRL,                      0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_BVCI2SCB_BRIDGE_CTRL_CACHE_MISS_EN_MASK },
      { "Read Swap",                 BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_BVCI2SCB_BRIDGE_CTRL,                      0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_BVCI2SCB_BRIDGE_CTRL_DATA_RD_SWAP_MASK },
      { "Write Swap",                BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_BVCI2SCB_BRIDGE_CTRL,                      0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_BVCI2SCB_BRIDGE_CTRL_DATA_WR_SWAP_MASK },
      { "Host2Vice",                 BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_ARRAY_BASE + HOST2VICE_MBOX_OFFSET,           VICE_BOOT_STATUS_INIT, BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_DATA_MASK },
      { "Vice2Host",                 BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_ARRAY_BASE + VICE2HOST_MBOX_OFFSET,           0x00000000,             BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_DATA_MASK },
#if ( (BVCE_P_CORE_MAJOR >= 3) || ( (BVCE_P_CORE_MAJOR == 2) && (BVCE_P_CORE_MINOR == 1) ) )
      { "PDA Per Module Enable",     BCHP_VICE2_MISC_0_VICE2_CLOCK_CTRL,                                               0xFFFFFFFF,             BCHP_VICE2_MISC_0_VICE2_CLOCK_CTRL_PDA_PER_MODULE_ON_MASK },
#endif
      { "Arc[0] Boot",               BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START,                                    0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START_ENABLE_MASK },
   },
   {
      { "Arc[0] Boot",               BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_PROC_START,                                    0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START_ENABLE_MASK },
      { "Vice SUNTOP Reset (Set)",   BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,                                                0xFFFFFFFF,             BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_vice21_sw_init_MASK },
      { "Vice SUNTOP Reset (Clear)", BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,                                              0xFFFFFFFF,             BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_vice21_sw_init_MASK },
      { "Vice CPU Speed (MISC_0_SCRATCH_0)", BCHP_VICE2_MISC_1_SCRATCH,                                                0x00000000,             0xFFFFFFFF },
      { "Reg Addr Offset",           BCHP_VICE2_ARCSS_MISC_1_INIT_SYS_REG_ADDR_OFFSET,                                 BCHP_PHYSICAL_OFFSET,  BCHP_VICE2_ARCSS_MISC_0_INIT_SYS_REG_ADDR_OFFSET_OFFSET_MASK },
      { "Arc Config",                BCHP_VICE2_ARCSS_MISC_1_INIT_SYS_ARC_CONFIG,                                      0x00000000,             BCHP_VICE2_ARCSS_MISC_0_INIT_SYS_ARC_CONFIG_SEL_MASK },
      { "Arc[0] Host I/F",           BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_SYS_HOST_IF,                                   0xFFFFFFFF,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_HOST_IF_SEL_MASK },
      { "Cache Miss",                BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_SYS_BVCI2SCB_BRIDGE_CTRL,                      0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_BVCI2SCB_BRIDGE_CTRL_CACHE_MISS_EN_MASK },
      { "Read Swap",                 BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_SYS_BVCI2SCB_BRIDGE_CTRL,                      0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_BVCI2SCB_BRIDGE_CTRL_DATA_RD_SWAP_MASK },
      { "Write Swap",                BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_SYS_BVCI2SCB_BRIDGE_CTRL,                      0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_BVCI2SCB_BRIDGE_CTRL_DATA_WR_SWAP_MASK },
      { "Host2Vice",                 BCHP_VICE2_ARCSS_ESS_DCCM_0_1_DATAi_ARRAY_BASE + HOST2VICE_MBOX_OFFSET,           VICE_BOOT_STATUS_INIT, BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_DATA_MASK },
      { "Vice2Host",                 BCHP_VICE2_ARCSS_ESS_DCCM_0_1_DATAi_ARRAY_BASE + VICE2HOST_MBOX_OFFSET,           0x00000000,             BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_DATA_MASK },
#if ( (BVCE_P_CORE_MAJOR >= 3) || ( (BVCE_P_CORE_MAJOR == 2) && (BVCE_P_CORE_MINOR == 1) ) )
      { "PDA Per Module Enable",     BCHP_VICE2_MISC_1_VICE2_CLOCK_CTRL,                                               0xFFFFFFFF,             BCHP_VICE2_MISC_1_VICE2_CLOCK_CTRL_PDA_PER_MODULE_ON_MASK },
#endif
      { "Arc[0] Boot",               BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_PROC_START,                                    0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START_ENABLE_MASK },
   }
};

static const BVCE_Platform_P_RegisterSetting s_astViceWatchdogHandlerEnableRegisterSettings[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][1] =
{
   {
      { "Vice SW INIT Mode (Set)",   BCHP_VICE2_MISC_0_SW_INIT_CONTROL,                                              0xFFFFFFFF,             BCHP_VICE2_MISC_0_SW_INIT_CONTROL_SW_INIT_MODE_MASK },
   },
   {
      { "Vice SW INIT Mode (Set)",   BCHP_VICE2_MISC_1_SW_INIT_CONTROL,                                              0xFFFFFFFF,             BCHP_VICE2_MISC_0_SW_INIT_CONTROL_SW_INIT_MODE_MASK },
   }
};

static const BVCE_Platform_P_RegisterSetting s_astViceWatchdogHandlerDisableRegisterSettings[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][1] =
{
   {
      { "Vice SW INIT Mode (Clear)",         BCHP_VICE2_MISC_0_SW_INIT_CONTROL,                                              0x00000000,             BCHP_VICE2_MISC_0_SW_INIT_CONTROL_SW_INIT_MODE_MASK },
   },
   {
      { "Vice SW INIT Mode (Clear)",         BCHP_VICE2_MISC_1_SW_INIT_CONTROL,                                              0x00000000,             BCHP_VICE2_MISC_0_SW_INIT_CONTROL_SW_INIT_MODE_MASK },
   }
};

static const BVCE_Platform_P_RegisterSetting s_astViceBootRegisterSettings[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][1] =
{
   {
      { "Arc[0] Boot",               BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START,                                    0XFFFFFFFF,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START_ENABLE_MASK },
   },
   {
      { "Arc[0] Boot",               BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_PROC_START,                                    0XFFFFFFFF,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_PROC_START_ENABLE_MASK },
   }
};

static const BVCE_Platform_P_RegisterSetting s_astViceWatchdogDisableRegisterSettings[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES][2] =
{
   {
      { "Arc[0] Watchdog Enable",    BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_WATCHDOG_CTRL,                             0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_WATCHDOG_CTRL_ENABLE_MASK },
      { "Arc[1] Watchdog Enable",    BCHP_VICE2_ARCSS_ESS_CTRL_1_0_INIT_SYS_WATCHDOG_CTRL,                             0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_WATCHDOG_CTRL_ENABLE_MASK },
   },
   {
      { "Arc[0] Watchdog Enable",    BCHP_VICE2_ARCSS_ESS_CTRL_0_1_INIT_SYS_WATCHDOG_CTRL,                             0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_WATCHDOG_CTRL_ENABLE_MASK },
      { "Arc[1] Watchdog Enable",    BCHP_VICE2_ARCSS_ESS_CTRL_1_1_INIT_SYS_WATCHDOG_CTRL,                             0x00000000,             BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_WATCHDOG_CTRL_ENABLE_MASK },
   }
};

extern const BVCE_Platform_P_Register s_astViceHardwareRegisters[];
extern const unsigned s_uiViceHardwareRegistersCount;

BERR_Code
BVCE_Platform_P_GetConfig(
         BBOX_Handle hBox,
         unsigned uiInstance,
         BVCE_Platform_P_Config *pstPlatformConfig
         )
{
   BDBG_ASSERT( pstPlatformConfig );
   BDBG_ASSERT( uiInstance < BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES );

   BKNI_Memset(
            pstPlatformConfig,
            0,
            sizeof( BVCE_Platform_P_Config )
            );

   pstPlatformConfig->stViceReset.astRegisterSettings = s_astViceResetRegisterSettings[uiInstance];
   pstPlatformConfig->stViceReset.uiRegisterCount = sizeof( s_astViceResetRegisterSettings[uiInstance] ) / sizeof ( BVCE_Platform_P_RegisterSetting );

   pstPlatformConfig->stViceWatchdogHandlerEnable.astRegisterSettings = s_astViceWatchdogHandlerEnableRegisterSettings[uiInstance];
   pstPlatformConfig->stViceWatchdogHandlerEnable.uiRegisterCount = sizeof( s_astViceWatchdogHandlerEnableRegisterSettings[uiInstance] ) / sizeof ( BVCE_Platform_P_RegisterSetting );

   pstPlatformConfig->stViceWatchdogHandlerDisable.astRegisterSettings = s_astViceWatchdogHandlerDisableRegisterSettings[uiInstance];
   pstPlatformConfig->stViceWatchdogHandlerDisable.uiRegisterCount = sizeof( s_astViceWatchdogHandlerDisableRegisterSettings[uiInstance] ) / sizeof ( BVCE_Platform_P_RegisterSetting );

   pstPlatformConfig->stViceBoot.astRegisterSettings = s_astViceBootRegisterSettings[uiInstance];
   pstPlatformConfig->stViceBoot.uiRegisterCount = sizeof( s_astViceBootRegisterSettings[uiInstance] ) / sizeof ( BVCE_Platform_P_RegisterSetting );

   pstPlatformConfig->stViceWatchdogDisable.astRegisterSettings = s_astViceWatchdogDisableRegisterSettings[uiInstance];
   pstPlatformConfig->stViceWatchdogDisable.uiRegisterCount = sizeof( s_astViceWatchdogDisableRegisterSettings[uiInstance] ) / sizeof ( BVCE_Platform_P_RegisterSetting );

   {
      signed iInstanceRegisterOffset = 0;

      /* coverity[dead_error_condition] */
      switch ( uiInstance )
      {
         case 0:
            iInstanceRegisterOffset = 0;
            break;

         case 1:
            iInstanceRegisterOffset = BVCE_P_REGISTER_INSTANCE_OFFSET;
            break;

         /* coverity[dead_error_begin] */
         default:
            BDBG_ERR(("Unsupported Instance"));
            BDBG_ASSERT(0);
            return BERR_TRACE( BERR_NOT_SUPPORTED );
      }

      BDBG_CASSERT( 2 == BVCE_PLATFORM_P_NUM_ARC_CORES );
      pstPlatformConfig->stCore[0].uiInstructionStartPhysicalAddress = BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_ARC_INSTR_ADDR_OFFSET + iInstanceRegisterOffset;
      pstPlatformConfig->stCore[0].uiDataSpaceStartRelativeOffset = BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_DATA_SPACE_START + iInstanceRegisterOffset;
      pstPlatformConfig->stCore[0].uiDataSpaceStartSystemOffset = BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_ARC_DATA_ADDR_OFFSET + iInstanceRegisterOffset;
      pstPlatformConfig->stCore[0].uiDCCMBase = BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stCore[1].uiInstructionStartPhysicalAddress = BCHP_VICE2_ARCSS_ESS_CTRL_1_0_INIT_SYS_ARC_INSTR_ADDR_OFFSET + iInstanceRegisterOffset;
      pstPlatformConfig->stCore[1].uiDataSpaceStartRelativeOffset = BCHP_VICE2_ARCSS_ESS_CTRL_1_0_INIT_SYS_DATA_SPACE_START + iInstanceRegisterOffset;
      pstPlatformConfig->stCore[1].uiDataSpaceStartSystemOffset = BCHP_VICE2_ARCSS_ESS_CTRL_1_0_INIT_SYS_ARC_DATA_ADDR_OFFSET + iInstanceRegisterOffset;
      pstPlatformConfig->stCore[1].uiDCCMBase = BCHP_VICE2_ARCSS_ESS_DCCM_1_0_DATAi_ARRAY_BASE + iInstanceRegisterOffset;

      BDBG_CASSERT( 2 <= BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS );
      pstPlatformConfig->stOutput[0].stCDB.uiReadPointer = BCHP_VICE2_CABAC_0_0_CDB_CTX0_READ_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[0].stCDB.uiBasePointer = BCHP_VICE2_CABAC_0_0_CDB_CTX0_BASE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[0].stCDB.uiValidPointer = BCHP_VICE2_CABAC_0_0_CDB_CTX0_VALID_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[0].stCDB.uiWritePointer = BCHP_VICE2_CABAC_0_0_CDB_CTX0_WRITE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[0].stCDB.uiEndPointer = BCHP_VICE2_CABAC_0_0_CDB_CTX0_END_PTR + iInstanceRegisterOffset;

      pstPlatformConfig->stOutput[0].stITB.uiReadPointer = BCHP_VICE2_CABAC_0_0_ITB_CTX0_READ_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[0].stITB.uiBasePointer = BCHP_VICE2_CABAC_0_0_ITB_CTX0_BASE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[0].stITB.uiValidPointer = BCHP_VICE2_CABAC_0_0_ITB_CTX0_VALID_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[0].stITB.uiWritePointer = BCHP_VICE2_CABAC_0_0_ITB_CTX0_WRITE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[0].stITB.uiEndPointer = BCHP_VICE2_CABAC_0_0_ITB_CTX0_END_PTR + iInstanceRegisterOffset;

      pstPlatformConfig->stOutput[1].stCDB.uiReadPointer = BCHP_VICE2_CABAC_0_0_CDB_CTX1_READ_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[1].stCDB.uiBasePointer = BCHP_VICE2_CABAC_0_0_CDB_CTX1_BASE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[1].stCDB.uiValidPointer = BCHP_VICE2_CABAC_0_0_CDB_CTX1_VALID_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[1].stCDB.uiWritePointer = BCHP_VICE2_CABAC_0_0_CDB_CTX1_WRITE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[1].stCDB.uiEndPointer = BCHP_VICE2_CABAC_0_0_CDB_CTX1_END_PTR + iInstanceRegisterOffset;

      pstPlatformConfig->stOutput[1].stITB.uiReadPointer = BCHP_VICE2_CABAC_0_0_ITB_CTX1_READ_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[1].stITB.uiBasePointer = BCHP_VICE2_CABAC_0_0_ITB_CTX1_BASE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[1].stITB.uiValidPointer = BCHP_VICE2_CABAC_0_0_ITB_CTX1_VALID_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[1].stITB.uiWritePointer = BCHP_VICE2_CABAC_0_0_ITB_CTX1_WRITE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[1].stITB.uiEndPointer = BCHP_VICE2_CABAC_0_0_ITB_CTX1_END_PTR + iInstanceRegisterOffset;

#if BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS >= 3
      pstPlatformConfig->stOutput[2].stCDB.uiReadPointer = BCHP_VICE2_CABAC_0_0_CDB_CTX2_READ_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[2].stCDB.uiBasePointer = BCHP_VICE2_CABAC_0_0_CDB_CTX2_BASE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[2].stCDB.uiValidPointer = BCHP_VICE2_CABAC_0_0_CDB_CTX2_VALID_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[2].stCDB.uiWritePointer = BCHP_VICE2_CABAC_0_0_CDB_CTX2_WRITE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[2].stCDB.uiEndPointer = BCHP_VICE2_CABAC_0_0_CDB_CTX2_END_PTR + iInstanceRegisterOffset;

      pstPlatformConfig->stOutput[2].stITB.uiReadPointer = BCHP_VICE2_CABAC_0_0_ITB_CTX2_READ_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[2].stITB.uiBasePointer = BCHP_VICE2_CABAC_0_0_ITB_CTX2_BASE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[2].stITB.uiValidPointer = BCHP_VICE2_CABAC_0_0_ITB_CTX2_VALID_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[2].stITB.uiWritePointer = BCHP_VICE2_CABAC_0_0_ITB_CTX2_WRITE_PTR + iInstanceRegisterOffset;
      pstPlatformConfig->stOutput[2].stITB.uiEndPointer = BCHP_VICE2_CABAC_0_0_ITB_CTX2_END_PTR + iInstanceRegisterOffset;
#endif

      BDBG_CASSERT( 28 == HOST_CMD_MAILBOX_INTERRUPT_LEVEL );
      BDBG_CASSERT( 27 == HOST_EVENTS_INTERRUPT_LEVEL );
      pstPlatformConfig->stInterrupt.uiInterruptStatusRegister = BCHP_VICE2_L2_0_CPU_STATUS + iInstanceRegisterOffset;

      /* coverity[dead_error_condition] */
      switch ( uiInstance )
      {
         case 0:
            pstPlatformConfig->stInterrupt.idWatchdog[0] = BCHP_INT_ID_VICE2_L2_0_VICE2_WDOG_0_INTR;
            pstPlatformConfig->stInterrupt.idWatchdog[1] = BCHP_INT_ID_VICE2_L2_0_VICE2_WDOG_1_INTR;
            pstPlatformConfig->stInterrupt.idDataReady[0] = BCHP_INT_ID_VICE2_L2_0_VICE2_CABAC_RDY_0_INTR;
            pstPlatformConfig->stInterrupt.idDataReady[1] = BCHP_INT_ID_VICE2_L2_0_VICE2_CABAC_RDY_1_INTR;
			#if BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS >= 3
            pstPlatformConfig->stInterrupt.idDataReady[2] = BCHP_INT_ID_VICE2_L2_0_VICE2_CABAC_RDY_2_INTR;
			#endif
            pstPlatformConfig->stInterrupt.idMailbox = BCHP_INT_ID_VICE2_L2_0_VICE2_MBOX_INTR;
            pstPlatformConfig->stInterrupt.idEvent = BCHP_INT_ID_VICE2_L2_0_VICE2_ERROR_INTR;
            break;

         case 1:
            pstPlatformConfig->stInterrupt.idWatchdog[0] = BCHP_INT_ID_VICE2_L2_1_VICE2_WDOG_0_INTR;
            pstPlatformConfig->stInterrupt.idWatchdog[1] = BCHP_INT_ID_VICE2_L2_1_VICE2_WDOG_1_INTR;
            pstPlatformConfig->stInterrupt.idDataReady[0] = BCHP_INT_ID_VICE2_L2_1_VICE2_CABAC_RDY_0_INTR;
            pstPlatformConfig->stInterrupt.idDataReady[1] = BCHP_INT_ID_VICE2_L2_1_VICE2_CABAC_RDY_1_INTR;
			#if BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS >= 3
            pstPlatformConfig->stInterrupt.idDataReady[2] = BCHP_INT_ID_VICE2_L2_1_VICE2_CABAC_RDY_2_INTR;
			#endif
            pstPlatformConfig->stInterrupt.idMailbox = BCHP_INT_ID_VICE2_L2_1_VICE2_MBOX_INTR;
            pstPlatformConfig->stInterrupt.idEvent = BCHP_INT_ID_VICE2_L2_1_VICE2_ERROR_INTR;
            break;

         /* coverity[dead_error_begin] */
         default:
            BDBG_ERR(("Unsupported Instance"));
            BDBG_ASSERT(0);
            return BERR_TRACE( BERR_NOT_SUPPORTED );
      }


      pstPlatformConfig->stDebug.uiCMEPictureIndex = BCHP_VICE2_CME_0_0_PICTURE_INDEX + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiArcPC[0] = BCHP_VICE2_ARCSS_ESS_HOSTIF_0_0_HOSTIFi_ARRAY_BASE + 0x18 + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiArcPC[1] = BCHP_VICE2_ARCSS_ESS_HOSTIF_1_0_HOSTIFi_ARRAY_BASE + 0x18 + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiArcHostIF[0] =  BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_HOST_IF + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiArcHostIF[1] = BCHP_VICE2_ARCSS_ESS_CTRL_1_0_INIT_SYS_HOST_IF + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiArcHostIFMask[0] = BCHP_VICE2_ARCSS_ESS_CTRL_0_0_INIT_SYS_HOST_IF_SEL_MASK;
      pstPlatformConfig->stDebug.uiArcHostIFMask[1] = BCHP_VICE2_ARCSS_ESS_CTRL_1_0_INIT_SYS_HOST_IF_SEL_MASK;
      pstPlatformConfig->stDebug.uiPicArcStatus32 = BCHP_VICE2_ARCSS_ESS_HOSTIF_0_0_HOSTIFi_ARRAY_BASE + 0x28 + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiCDBDepth[0] = BCHP_VICE2_CABAC_0_0_CDB_CTX0_DEPTH + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiCDBDepth[1] = BCHP_VICE2_CABAC_0_0_CDB_CTX1_DEPTH + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiSTC[0] = BCHP_VICE2_ARCSS_MISC_0_STC0_LOWER + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiSTC[1] = BCHP_VICE2_ARCSS_MISC_0_STC1_LOWER + iInstanceRegisterOffset;
#if BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS >= 3
      pstPlatformConfig->stDebug.uiCDBDepth[2] = BCHP_VICE2_CABAC_0_0_CDB_CTX2_DEPTH + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.uiSTC[2] = BCHP_VICE2_ARCSS_MISC_0_STC2_LOWER + iInstanceRegisterOffset;
#endif
#if (BCHP_VER >= BCHP_VER_C0)
      pstPlatformConfig->stDebug.uiScratchRegister = BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_ARRAY_BASE + BAVC_VICE_MBOX_OFFSET_VICE_FW_SCRATCH + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.auiMemcRegBaseLUT[0] = BCHP_PHYSICAL_OFFSET + BCHP_MEMC_SENTINEL_0_0_SENTINEL_RANGE_START;
      pstPlatformConfig->stDebug.auiMemcRegBaseLUT[1] = BCHP_PHYSICAL_OFFSET + BCHP_MEMC_SENTINEL_0_1_SENTINEL_RANGE_START;
      pstPlatformConfig->stDebug.auiMemcRegBaseLUT[2] = BCHP_PHYSICAL_OFFSET + BCHP_MEMC_SENTINEL_0_2_SENTINEL_RANGE_START;
#endif

      pstPlatformConfig->stMailbox.uiHost2ViceMailboxAddress = BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_ARRAY_BASE + HOST2VICE_MBOX_OFFSET + iInstanceRegisterOffset;
      pstPlatformConfig->stMailbox.uiVice2HostMailboxAddress = BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_ARRAY_BASE + VICE2HOST_MBOX_OFFSET + iInstanceRegisterOffset;
      pstPlatformConfig->stMailbox.uiBvn2ViceMailboxAddress = BCHP_VICE2_ARCSS_ESS_DCCM_0_0_DATAi_ARRAY_BASE + BVN2VICE_MBOX_OFFSET + iInstanceRegisterOffset;
      pstPlatformConfig->stMailbox.uiHost2ViceInterruptAddress = BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_0_ARC_P0_SET + iInstanceRegisterOffset;
      pstPlatformConfig->stMailbox.uiHost2ViceInterruptMask = BCHP_VICE2_ARCSS_ESS_P1_INTR2_0_0_ARC_P0_SET_ARCESS_SOFT1_10_INTR_MASK;

   #ifdef BCHP_PWR_SUPPORT
      /* coverity[dead_error_condition] */
      switch ( uiInstance )
      {
         case 0:
#if BCHP_PWR_RESOURCE_VICE0_PWR
            pstPlatformConfig->stPower.astResource[BVCE_Power_Type_ePower].id = BCHP_PWR_RESOURCE_VICE0_PWR;
#endif
#if BCHP_PWR_RESOURCE_VICE0_CLK
            pstPlatformConfig->stPower.astResource[BVCE_Power_Type_eClock].id = BCHP_PWR_RESOURCE_VICE0_CLK;
#endif
            break;

         case 1:
#if BCHP_PWR_RESOURCE_VICE0_PWR
            pstPlatformConfig->stPower.astResource[BVCE_Power_Type_ePower].id = BCHP_PWR_RESOURCE_VICE1_PWR;
#endif
#if BCHP_PWR_RESOURCE_VICE0_CLK
            pstPlatformConfig->stPower.astResource[BVCE_Power_Type_eClock].id = BCHP_PWR_RESOURCE_VICE1_CLK;
#endif
            break;

         /* coverity[dead_error_begin] */
         default:
            BDBG_ERR(("Unsupported Instance"));
            BDBG_ASSERT(0);
            return BERR_TRACE( BERR_NOT_SUPPORTED );
      }
#endif

      pstPlatformConfig->stWatchdogRegisterDumpList.iInstanceOffset = BVCE_P_REGISTER_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stWatchdogRegisterDumpList.astRegisters = s_astViceHardwareRegisters;
      pstPlatformConfig->stWatchdogRegisterDumpList.uiCount = s_uiViceHardwareRegistersCount;

      pstPlatformConfig->stWatchdogRegisterDumpList.iInstanceOffset = BVCE_P_REGISTER_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stWatchdogRegisterDumpList.astRegisters = s_astViceHardwareRegisters;
      pstPlatformConfig->stWatchdogRegisterDumpList.uiCount = s_uiViceHardwareRegistersCount;

      pstPlatformConfig->stDebug.stCmd.uiBasePointer = BCHP_VICE2_CABAC_0_0_CMD_BUFF_START_ADDR + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stCmd.uiEndPointer = BCHP_VICE2_CABAC_0_0_CMD_BUFF_END_ADDR + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stCmd.uiReadPointer = BCHP_VICE2_CABAC_0_0_CMD_BUFF_RD_ADDR + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stCmd.uiWritePointer = BCHP_VICE2_CABAC_0_0_CMD_BUFF_WR_ADDR + iInstanceRegisterOffset;

      /* SW7445-665: D0 now has array of addresses to allow for VP8 partitions.  We use only the first (non-VP8) entry for D0 */
#if (BCHP_VER >= BCHP_VER_D0)
      pstPlatformConfig->stDebug.stBin[0].uiBasePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF0_START_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[0].uiEndPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF0_END_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[0].uiReadPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF0_RD_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[0].uiWritePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF0_WR_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;

      pstPlatformConfig->stDebug.stBin[1].uiBasePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF1_START_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[1].uiEndPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF1_END_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[1].uiReadPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF1_RD_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[1].uiWritePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF1_WR_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;

#if BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS >= 3
      pstPlatformConfig->stDebug.stBin[2].uiBasePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF2_START_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[2].uiEndPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF2_END_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[2].uiReadPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF2_RD_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[2].uiWritePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF2_WR_ADDRi_ARRAY_BASE + iInstanceRegisterOffset;
#endif
#else
      pstPlatformConfig->stDebug.stBin[0].uiBasePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF0_START_ADDR + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[0].uiEndPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF0_END_ADDR + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[0].uiReadPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF0_RD_ADDR + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[0].uiWritePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF0_WR_ADDR + iInstanceRegisterOffset;

      pstPlatformConfig->stDebug.stBin[1].uiBasePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF1_START_ADDR + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[1].uiEndPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF1_END_ADDR + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[1].uiReadPointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF1_RD_ADDR + iInstanceRegisterOffset;
      pstPlatformConfig->stDebug.stBin[1].uiWritePointer = BCHP_VICE2_CABAC_0_0_BIN_BUFF1_WR_ADDR + iInstanceRegisterOffset;
#endif
   }

   /* Set Box Mode Defaults */
   BVCE_Platform_P_GetTotalInstances( hBox, &pstPlatformConfig->stBox.uiTotalInstances );
   BVCE_Platform_P_GetTotalChannels( hBox, uiInstance, &pstPlatformConfig->stBox.uiTotalChannels );

   return BERR_TRACE( BERR_SUCCESS );
}

/* The following reset code is for 7445 */
#include "bchp_vice2_mau_0_0.h"
#include "bchp_vice2_mau_0_1.h"
BERR_Code
BVCE_Platform_P_PreReset(
   unsigned uiInstance,
   BREG_Handle hReg
   )
{
   uint32_t uiReg;
   unsigned uiMauStatusReg = 0;
   unsigned uiMauFwCtrlReg = 0;
   unsigned uiSwInitSetReg = 0;
   unsigned uiSwInitClrReg = 0;

   BDBG_ASSERT( uiInstance < BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES );

   BSTD_UNUSED( uiInstance );

   BDBG_ENTER( BVCE_Platform_P_PreReset );

#if (BCHP_CHIP == 7435)
   switch ( uiInstance )
   {
      case 0:
         uiMauStatusReg = BCHP_VICE2_MAU_0_0_STATUS;
         uiMauFwCtrlReg = BCHP_VICE2_MAU_0_0_FW_CONTROL;
         uiSwInitSetReg = BCHP_VICE2_MISC_0_SW_INIT_SET;
         uiSwInitClrReg = BCHP_VICE2_MISC_0_SW_INIT_CLR;
         break;
      case 1:
         uiMauStatusReg = BCHP_VICE2_MAU_0_1_STATUS;
         uiMauFwCtrlReg = BCHP_VICE2_MAU_0_1_FW_CONTROL;
         uiSwInitSetReg = BCHP_VICE2_MISC_1_SW_INIT_SET;
         uiSwInitClrReg = BCHP_VICE2_MISC_1_SW_INIT_CLR;
         break;
      default:
         BDBG_ERR(("Unsupported Instance"));
         BDBG_ASSERT(0);
         return BERR_TRACE( BERR_NOT_SUPPORTED );
   }

   /* VICE Shutdown Sequence from Brad Delanghe 05/06/2011:
    *
    * Proposal for 7425B0:
    *  1. SCB transaction will be discarded immediately
    *    Note:
    *       a. This does not comply with the SCB bus spec
    *       b. This could potentially corrupt data for downstream blocks
    *       c. This will not work for PFRI transactions, due to the multi-cycle nature of the standard
    *  2. PFRI transactions need to be shut-down using a multi cycle procedure.
    *       a. Write 1 to MAU register FORCE_SHUTOFF to stop any future patch requests from ViCE internal clients
    *       b. Poll MAU core status register to confirm it is IDLE
    *       c. Issue sw_init
    *  3. ViCE design team is making efforts to verify that the sw_init can be issued at any time (while idle or while processing), but there is no guarantee that all possible cases can be verified.  Please use the random sw_init with caution.
    *
    */

   /* Check MAU Status */
   uiReg = BREG_Read32( hReg, uiMauStatusReg );
   if ( BCHP_FIELD_ENUM( VICE2_MAU_0_0_STATUS, MAU_STATUS, BUSY) == BCHP_GET_FIELD_DATA( uiReg, VICE2_MAU_0_0_STATUS, MAU_STATUS ) )
   {
      BDBG_WRN(("MAU is BUSY before FORCE_SHUTOFF"));
   }

   /* Set Force Shutoff */
   uiReg = 0;
   BCHP_SET_FIELD_ENUM(uiReg, VICE2_MAU_0_0_FW_CONTROL, FORCE_SHUTOFF, ENABLE );
   BREG_Write32(hReg, uiMauFwCtrlReg, uiReg );

   /* SW7425-3816: SW INIT MAU_PFRI to clear the outstanding PFRI state */
   BKNI_Sleep(10);
   uiReg = 0;
   BCHP_SET_FIELD_DATA(uiReg, VICE2_MISC_0_SW_INIT_SET, MAU_PFRI, 1 );
   BREG_Write32(hReg, uiSwInitSetReg, uiReg );
   BKNI_Sleep(1);
   BREG_Write32(hReg, uiSwInitClrReg, uiReg );

   /* Check status until idle */
   uiReg = BREG_Read32( hReg, uiMauStatusReg );
   while ( BCHP_FIELD_ENUM( VICE2_MAU_0_0_STATUS, MAU_STATUS, BUSY) == BCHP_GET_FIELD_DATA( uiReg, VICE2_MAU_0_0_STATUS, MAU_STATUS ) )
   {
      BDBG_WRN(("MAU is BUSY..."));
      BKNI_Sleep(1);
      uiReg = BREG_Read32( hReg, uiMauStatusReg );
   }
#else
   BSTD_UNUSED( hReg );
   BSTD_UNUSED( uiReg );
   BSTD_UNUSED( uiMauStatusReg );
   BSTD_UNUSED( uiMauFwCtrlReg );
   BSTD_UNUSED( uiSwInitSetReg );
   BSTD_UNUSED( uiSwInitClrReg );
#endif

   BDBG_LEAVE( BVCE_Platform_P_PreReset );

   return BERR_TRACE( BERR_SUCCESS );
}

#ifdef BVCE_P_ENABLE_UART
/* Enable UART for Vice Arc 0/1 */
BERR_Code
BVCE_Platform_P_EnableUART(
   BREG_Handle hReg
   )
{
   uint32_t uiReg;

   uiReg = BREG_Read32(
            hReg,
            BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0
            );

#if (1 == BVCE_P_ENABLE_UART)
   BCHP_SET_FIELD_ENUM(uiReg, SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel, VICE20_ARC0);
   BCHP_SET_FIELD_ENUM(uiReg, SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_4_cpu_sel, VICE21_ARC0);
#else
   BCHP_SET_FIELD_ENUM(uiReg, SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel, VICE21_ARC0);
   BCHP_SET_FIELD_ENUM(uiReg, SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_4_cpu_sel, VICE20_ARC0);
#endif

   BREG_Write32(
            hReg,
            BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0,
            uiReg
            );

    return BERR_SUCCESS;
}
#endif

const BVCE_P_GetFeatureInfo BVCE_P_MemcLUT[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES] =
{
   {BCHP_Feature_eMemCtrl0DDRDeviceTechCount, BCHP_Feature_eMemCtrl0DramWidthCount},
   {BCHP_Feature_eMemCtrl1DDRDeviceTechCount, BCHP_Feature_eMemCtrl1DramWidthCount},
};
