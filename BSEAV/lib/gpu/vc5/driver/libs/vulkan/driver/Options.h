/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "vcos.h"

namespace bvk {

class Options
{
public:
   Options() = delete;

   static void Initialise();

   static uint32_t renderSubjobs;
   static uint32_t binSubjobs;

   static bool     allCoresSameStOrder;     // Essentially gives the same render list to each core
   static bool     partitionSupertilesInSW; // i.e. don't give all supertiles to all
                                            // cores, give each core an exclusive subset

   // isolate_frame == -1 disables tile isolation. Otherwise, isolate_supertile_x &
   // isolate_supertile_y determine the tile to isolate in frame number isolate_frame
   static uint32_t isolateFrame;
   static uint32_t isolateSupertileX;
   static uint32_t isolateSupertileY;

   static uint32_t minSupertileW;
   static uint32_t minSupertileH;
   static uint32_t maxSupertiles;

   static bool     autoclifEnabled;
   static int32_t  autoclifOnlyOneClifI;    // < 0 means capture all frames
   static char     autoclifOnlyOneClifName[VCOS_PROPERTY_VALUE_MAX];
   static uint32_t autoclifBinBlockSize;    // Set the size of the binning memory block in recorded CLIFs

   static bool     zPrepass;                // Z-prepass enabled
   static bool     noGfxh1385;              // Disable workarounds for GFXH-1385
   static bool     earlyZ;                  // Use early-Z
   static uint32_t maxWorkerThreads;        // Maximum number of worker threads to spawn for computing in parallel

   static bool     dumpTransferImages;                // output gfx buffer txt files on transfer operations
   static bool     dumpPresentedImages;               // output gfx buffer txt files on frame presentation
   static char     dumpPath[VCOS_PROPERTY_VALUE_MAX]; // directory path in which to output image dumps

   static bool     fullSymbolNames;         // Retain full symbol names in the compiler

   static bool     dumpSPIRV;               // Output binary SPIR-V code for each module as supplied

   // Platform specific configuration
   static uint32_t numNxClientSurfaces;
};

} // namespace bvk
