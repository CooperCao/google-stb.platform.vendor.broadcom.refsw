/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Options.h"
#include "libs/util/gfx_options/gfx_options.h"
#include "libs/core/v3d/v3d_limits.h"

LOG_DEFAULT_CAT("bvk::Options")

namespace bvk {

uint32_t Options::renderSubjobs;
uint32_t Options::binSubjobs;
bool     Options::allCoresSameStOrder;
bool     Options::partitionSupertilesInSW;
uint32_t Options::isolateFrame;
uint32_t Options::isolateSupertileX;
uint32_t Options::isolateSupertileY;
uint32_t Options::minSupertileW;
uint32_t Options::minSupertileH;
uint32_t Options::maxSupertiles;
bool     Options::autoclifEnabled;
int32_t  Options::autoclifOnlyOneClifI;
char     Options::autoclifOnlyOneClifName[VCOS_PROPERTY_VALUE_MAX];
uint32_t Options::autoclifBinBlockSize;
bool     Options::zPrepass;
bool     Options::noGfxh1385;
bool     Options::earlyZ;
uint32_t Options::maxWorkerThreads;
bool     Options::dumpTransferImages;
bool     Options::dumpPresentedImages;
char     Options::dumpPath[VCOS_PROPERTY_VALUE_MAX];
uint32_t Options::numNxClientSurfaces;
bool     Options::fullSymbolNames;
bool     Options::dumpSPIRV;

void Options::Initialise()
{
   // NOTE: In Android, these option names are converted to lower-case, and the underscores are replaced
   // with periods. So, "V3D_GL_ERROR_ASSIST" becomes "v3d.gl.error.assist" under Android for example.
   // Since these key names are used in Android, remember to keep them <= 32 characters (including the
   // NULL terminator).

   unsigned numSubjobs      = gfx_options_uint32("KHRN_NUM_SUBJOBS",              0);
   renderSubjobs            = gfx_options_uint32("KHRN_RENDER_SUBJOBS",           numSubjobs);
   binSubjobs               = gfx_options_uint32("KHRN_BIN_SUBJOBS",              numSubjobs);

   allCoresSameStOrder      = gfx_options_bool("VK_ALL_CORES_SAME_ST_ORDER",      false);
   partitionSupertilesInSW  = gfx_options_bool("VK_PARTITION_SUPERTILES_IN_SW",   false);
   zPrepass                 = gfx_options_bool("VK_Z_PREPASS",                    false);
   noGfxh1385               = gfx_options_bool("VK_NO_GFXH_1385",                 false);
   earlyZ                   = gfx_options_bool("VK_EARLY_Z",                      true);

   isolateFrame             = gfx_options_int32("VK_ISOLATE_FRAME",               ~0u); // Disable by default
   if (isolateFrame != ~0u)
   {
      isolateSupertileX     = gfx_options_uint32("VK_ISOLATE_SUPERTILE_X",        0);
      isolateSupertileY     = gfx_options_uint32("VK_ISOLATE_SUPERTILE_Y",        0);
   }

   minSupertileW            = gfx_options_uint32("VK_MIN_SUPERTILE_W",            1);
   minSupertileH            = gfx_options_uint32("VK_MIN_SUPERTILE_H",            1);
   maxSupertiles            = gfx_options_uint32("VK_MAX_SUPERTILES",             V3D_MAX_SUPERTILES);

   autoclifEnabled          = gfx_options_bool("AUTOCLIF",                        false);
   autoclifOnlyOneClifI     = gfx_options_int32("AUTOCLIF_ONLY_ONE_CLIF_I",      -1);
   gfx_options_str("AUTOCLIF_ONLY_ONE_CLIF_NAME", "",
                   autoclifOnlyOneClifName, sizeof(autoclifOnlyOneClifName));
   autoclifBinBlockSize     = gfx_options_uint32("AUTOCLIF_BIN_BLOCK_SIZE",       4 * 1024 * 1024);

   maxWorkerThreads         = gfx_options_uint32( "VK_MAX_WORKER_THREADS",        3);
   dumpTransferImages       = gfx_options_bool("VK_DUMP_TRANSFER_IMAGES",         false);
   dumpPresentedImages      = gfx_options_bool("VK_DUMP_PRESENTED_IMAGES",        false);
   gfx_options_str("VK_IMAGE_DUMP_PATH", ".", dumpPath, sizeof(dumpPath));

   numNxClientSurfaces      = gfx_options_uint32("VK_NUM_NXCLIENT_SURFACES",      1);

   fullSymbolNames          = gfx_options_bool("VK_FULL_SYMBOL_NAMES",            false);

   dumpSPIRV                = gfx_options_bool("VK_DUMP_SPIRV",                   false);
}

} // namespace bvk
