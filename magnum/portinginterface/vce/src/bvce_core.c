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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"
#include "bimg.h"
#include "bvce_platform.h"
#include "bvce_image.h"
#include "bvce_image_priv.h"

#if ( ( BVCE_P_CORE_MAJOR == 0 ) && ( BVCE_P_CORE_MINOR == 1 ) && ( BVCE_P_CORE_SUBMINOR == 8 ) && ( BVCE_P_CORE_REVISION == 2 ) )
#include "bvce_core_regs_hw_rev_0_1_8_2.c"
#include "bvce_core_rev_0_1_8_2.c"
#elif ( ( BVCE_P_CORE_MAJOR == 1 ) && ( BVCE_P_CORE_MINOR == 1 ) && ( BVCE_P_CORE_SUBMINOR == 1 ) && ( BVCE_P_CORE_REVISION == 1 ) )
#include "bvce_core_regs_hw_rev_1_1_1_1.c"
#include "bvce_core_rev_1_1_1_1.c"
#elif ( ( BVCE_P_CORE_MAJOR == 2 ) && ( BVCE_P_CORE_MINOR == 1 ) && ( BVCE_P_CORE_SUBMINOR == 0 ) && ( BVCE_P_CORE_REVISION == 3 ) )
#include "bvce_core_regs_hw_rev_2_1_0_3.c"
#include "bvce_core_rev_2_1_0_3.c"
#elif ( ( BVCE_P_CORE_MAJOR == 2 ) && ( BVCE_P_CORE_MINOR == 1 ) && ( BVCE_P_CORE_SUBMINOR == 2 ) && ( BVCE_P_CORE_REVISION == 2 ) )
#include "bvce_core_regs_hw_rev_2_1_2_2.c"
#include "bvce_core_rev_2_1_2_2.c"
#elif ( ( BVCE_P_CORE_MAJOR == 2 ) && ( BVCE_P_CORE_MINOR == 1 ) && ( BVCE_P_CORE_SUBMINOR == 3 ) && ( BVCE_P_CORE_REVISION == 2 ) )
#include "bvce_core_regs_hw_rev_2_1_3_2.c"
#include "bvce_core_rev_2_1_3_2.c"
#else
#error Unrecognized core version
#endif

unsigned
BVCE_P_ProtocolLUT(
   BAVC_VideoCompressionStd eProtocol
   )
{
   unsigned i=0;

   while ( 1 )
   {
      if ( ( eProtocol == BVCE_P_SupportedProtocols[i].eProtocol )
           || ( BAVC_VideoCompressionStd_eMax == BVCE_P_SupportedProtocols[i].eProtocol ) )
      {
         return BVCE_P_SupportedProtocols[i].uiEncodingStd;
      }

      i++;
   }
}
