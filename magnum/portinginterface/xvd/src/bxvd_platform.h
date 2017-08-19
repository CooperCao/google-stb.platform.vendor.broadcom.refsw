/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * [File Description:]
 *
 ***************************************************************************/

/***************************************************************************
XVD Code Stucture

Overview
--------

XVD was restructured in July/Aug 2006 (See PR22673) to better support
multiple chips and revisions using the same code base.  Prior to the
restructure, the code was littered with chip/revision #ifdefs that
made the code difficult to understand and maintain.

The restructure goals, though all very important, were prioritized as
follows:

  1) Readability - the code flow/logic should be easily readable to
     someone who is looking at the code for the first time.  We wanted
     to split the "what" from the "how".  i.e. We wanted the
     high-level functionality to be apparent by hiding the details of
     the actual implementation.

  2) Debuggability - We wanted to minimize the impact on run-time
     debugging using tools like gdb.  e.g. it should be possible to
     know exactly which function implementation is being executed by
     looking at the function call trace.  The function names should
     not be ambigious.

  3) Maintainability - often times, the chip families share 90%+ of
     the code.  There are subtle differences in things such as
     register names or init values. We wanted an simple mechanism to
     share code among chip families and revisions.

  4) Modularity - multiple chips may need to be supported in parallel
     by different developers.  We wanted to minimize the possibility
     of changes in one chip affecting another.  Unfortunately, this
     goal is somewhat converse of the previous goal.

Approach
--------

The XVD implementation was categorized as either common XVD code,
platform specific code, or code specific to a AVD decoder core.  The
bulk of the code is common to all XVD decoders and platforms, and
forms the main line.  Platform and core code is broken out into
functions and macros which implement the specific requirements of the
respective hardware.


Definitions
-----------

 common code: non-platform specific XVD code.  Uses macros to call
               the chip-specific implementations.

 top-level platform header: bxvd_platform.h. Included by the common
                            code.  Defines the function prototypes and
                            ensures all required chip-specific
                            definitions have been made.  Includes the
                            appropriate platform and chip specific
                            platform headers based on the value of
                            BCHP_CHIP.

 chip-specific platform header: bxvd_platform_xxxx.h (where xxxx is
                                the chip, e.g. 7401).  This file is
                                never included directly by the common
                                code.  It defines all the required
                                macros.

 core-specific platform header: bxvd_core_xxxx.h (where xxxx is the
                                core, e.g. avd_reve0).  Defines
                                core-specific macros.


Implementation
--------------

The chip-specific implementations were abstracted out of the main XVD
code and moved to chip-specific files using function macros.

   rc = BXVD_P_CHIP_RESET(pXvd);

The chip-specific platform header needs *3* different definitions in
order for the function abstraction to work.  (We will use 7445, for
the examples below):

 1) The macro itself needs to be defined to specify which
    implementation to call: (in bxvd_platform_revn0.h)

    #define BXVD_P_RESET_CHIP BXVD_P_ChipReset_RevN0


 2) The implementation needs to be conditionally compiled in.  (For
    any platform, all source files are included as part of the build
    to enable sharing of code between platforms).  The header needs to
    enable a particular version to be compiled in: (in
    bxvd_platform_revn0.h)

    #define BXVD_P_USE_CORE_RESET_CHIP_REVN0 1

    The corresponding implementation is conditionally wrapped around
    this define: (in bxvd_platform_revn0.c)

    #if BXVD_P_USE_CORE_RESET_CHIP_REVN0 1
    BERR_Code BXVD_P_ChipReset_RevN0(BXVD_Handle hXvd)
    {
       ...
    }
    #endif


 3) In order for the common code to compile without "missing
    prototype" warnings, the function prototype for the specific
    implementation needs to be declared: (in bxvd_platform_revn0.h)

    SETUP_INTERRUPTS_PROTOTYPE(RevN0);

    To enforce prototype consistency amongst various platforms, the
    prototype declaration in the chip-specific header is done via a
    macro that is defined in the top-level platform header: (in
    bxvd_platform.h)

    #define BXVD_P_CHIP_RESET_PROTOTYPE(family) \
    BERR_Code BXVD_P_ChipReset_##family \
    ( \
      BXVD_Handle hXvd \
    )


The top-level header also verifies that the required definitions have
been declared by the chip-specific header: (in bxvd_platform.h)

    #ifndef BXVD_P_RESET_CHIP
    #error BXVD_P_RESET_CHIP is undefined!
    #endif

 ***************************************************************************/

#include "bxvd.h"

#include "bstd.h"
#include "bchp.h"

#ifndef _BXVD_PLATFORM_H_
#define _BXVD_PLATFORM_H_

#include "bxvd_userdata.h"

/* prototype definition macros */
#define BXVD_P_SETUP_GET_BUFFER_ATOM_SIZE_PROTOTYPE(family) \
void BXVD_P_GetBufferAtomSize_##family \
( \
   BXVD_Handle hXvd,                        \
   const BXVD_ChannelSettings *pChSettings, \
   BXVD_P_VideoAtomIndex vidBlkIndex,       \
   uint32_t *puiLumaAtomSize,               \
   uint32_t *puiChromaAtomSize              \
)

#define BXVD_P_DETERMINE_STRIPE_INFO_PROTOTYPE(family) \
void BXVD_P_DetermineStripeInfo_##family \
( \
  BCHP_DramType ddrType,        \
  uint32_t uiMemPartSize,       \
  uint32_t uiMemBusWidth,       \
  uint32_t uiMemDeviceWidth,    \
  bool     bDDRGroupageEnabled, \
  uint32_t *puiStripeWidth,     \
  uint32_t *puiBankHeight       \
)

#define BXVD_P_FW_LOAD_PROTOTYPE(family) \
BERR_Code BXVD_P_FWLoad_##family \
( \
  BXVD_Handle hXvd, \
  uint32_t uiDecoderInstance \
)

#define BXVD_P_CHIP_ENABLE_PROTOTYPE(family) \
BERR_Code BXVD_P_ChipEnable_##family \
( \
  BXVD_Handle hXvd \
)

#define BXVD_P_CHIP_RESET_PROTOTYPE(family) \
BERR_Code BXVD_P_ChipReset_##family \
( \
  BXVD_Handle hXvd \
)

#define BXVD_P_INIT_REG_PTRS_PROTOTYPE(family) \
void BXVD_P_InitRegPtrs_##family \
( \
  BXVD_Handle hXvd \
)

#define BXVD_P_SETUP_FW_MEMORY_PROTOTYPE(family) \
BERR_Code BXVD_P_SetupFWMemory_##family \
( \
  BXVD_Handle hXvd \
)

#define BXVD_P_TEAR_DOWN_FW_MEMORY_PROTOTYPE(family) \
BERR_Code BXVD_P_TearDownFWMemory_##family \
( \
  BXVD_Handle hXvd \
)

#define BXVD_P_VERIFY_WATCHDOG_FIRED_PROTOTYPE(family) \
bool BXVD_P_VerifyWatchdogFired_##family##_isr \
( \
  BXVD_Handle hXvd, \
  int param2 \
)

#if ((BCHP_CHIP == 7405) && ((BCHP_VER == BCHP_VER_A0) || (BCHP_VER == BCHP_VER_A1))) || \
    (BCHP_CHIP == 7335) || \
    (BCHP_CHIP == 7325)

/* 7405 A0 rev H core */
#define BXVD_CHIP 740510

#elif ((BCHP_CHIP == 7405) && (BCHP_VER >= BCHP_VER_B0)) || \
      (BCHP_CHIP == 3548) || \
      (BCHP_CHIP == 3556) || \
      (BCHP_CHIP == 7336) || \
      (BCHP_CHIP == 7340) || \
      (BCHP_CHIP == 7342) || \
      (BCHP_CHIP == 7125) || \
      (BCHP_CHIP == 7408) || \
      (BCHP_CHIP == 7468)

/* 7405 B0 rev I core */
#define BXVD_CHIP 740520

#elif (BCHP_CHIP == 7135) || \
      (BCHP_CHIP == 7228) || \
      (BCHP_CHIP == 7231) || \
      (BCHP_CHIP == 7344) || \
      (BCHP_CHIP == 7346) || \
      (BCHP_CHIP == 7358) || \
      (BCHP_CHIP == 7360) || \
      (BCHP_CHIP == 7362) || \
      (BCHP_CHIP == 7422) || \
      (BCHP_CHIP == 7425) || \
      (BCHP_CHIP == 7429) || \
      (BCHP_CHIP == 7435) || \
      (BCHP_CHIP == 7543) || \
      (BCHP_CHIP == 7552) || \
      (BCHP_CHIP == 7563) || \
      (BCHP_CHIP == 7584) || \
      (BCHP_CHIP == 7640) || \
      (BCHP_CHIP == 35233)
/* Rev K core */
#define BXVD_CHIP 'K'

#elif ((BCHP_CHIP == 7445)  || (BCHP_CHIP == 7145)  || (BCHP_CHIP == 7250)  || ((BCHP_CHIP == 7260) && (BCHP_VER == BCHP_VER_A0)) || \
       (BCHP_CHIP == 7268)  || (BCHP_CHIP == 7271)  || (BCHP_CHIP == 7364)  || (BCHP_CHIP == 73625) || \
       (BCHP_CHIP == 7366)  || (BCHP_CHIP == 74295) || (BCHP_CHIP == 7439)  || (BCHP_CHIP == 74371) || \
       (BCHP_CHIP == 75525) || (BCHP_CHIP == 75635) || (BCHP_CHIP == 75845) || (BCHP_CHIP == 7586)  || \
       (BCHP_CHIP == 73465))
/* Rev N core */
#define BXVD_CHIP 'N'

#elif ((BCHP_CHIP == 7278) || ((BCHP_CHIP == 7260) && (BCHP_VER >= BCHP_VER_B0)))
/* Rev T core */
#define BXVD_CHIP 'T'

#else
#define BXVD_CHIP BCHP_CHIP
#endif

/* chip specific includes */
#if (BXVD_CHIP == 7401)
#include "bxvd_core_avd_reve0.h"
#include "bxvd_platform_7401.h"
#elif (BXVD_CHIP == 7403)
#include "bxvd_core_avd_reve0.h"
#include "bxvd_platform_7403.h"
#elif (BXVD_CHIP == 7118)
#include "bxvd_core_avd_reve0.h"
#include "bxvd_platform_7118.h"
#elif (BXVD_CHIP == 7400)
#include "bxvd_core_avd_reve0.h"
#include "bxvd_platform_7400.h"
#elif (BXVD_CHIP == 7440)
#include "bxvd_core_avd_reve0.h"
#include "bxvd_platform_7440.h"
#elif (BXVD_CHIP == 740510)
#include "bxvd_core_avd_revh0.h"
#include "bxvd_platform_7405.h"
#elif (BXVD_CHIP == 740520)
#include "bxvd_core_avd_revi0.h"
#include "bxvd_platform_7405.h"
#elif (BXVD_CHIP == 7601)
#include "bxvd_core_avd_reve0.h"
#include "bxvd_platform_7601.h"
#elif (BXVD_CHIP == 7635)
#include "bxvd_core_avd_reve0.h"
#include "bxvd_platform_7601.h"
#elif (BXVD_CHIP == 7420)
#include "bxvd_core_avd_revi0.h"
#include "bxvd_platform_7420.h"
#elif (BXVD_CHIP == 'K')
#include "bxvd_core_avd_revk0.h"
#include "bxvd_platform_revk0.h"
#elif (BXVD_CHIP == 'N')
#include "bxvd_core_avd_revn0.h"
#include "bxvd_platform_revn0.h"
#elif (BXVD_CHIP == 'T')
#include "bxvd_core_avd_revt0.h"
#include "bxvd_platform_revt0.h"
#else
#error Unsupported BCHP_CHIP version!
#endif

#if ((BXVD_CHIP != 'K') && (BXVD_CHIP != 'N') && (BXVD_CHIP != 'T'))
#define BXVD_P_USE_RELF 1
#else
#define BXVD_P_USE_RELF 0
#endif

/* make sure the required definitions exist */

#ifndef BXVD_P_SETUP_FW_MEMORY
#error BXVD_P_SETUP_FW_MEMORY is undefined!
#endif

#ifndef BXVD_P_TEAR_DOWN_FW_MEMORY
#error BXVD_P_TEAR_DOWN_FW_MEMORY is undefined!
#endif

#ifndef BXVD_P_FW_LOAD
#error BXVD_P_FW_LOAD is undefined!
#endif

#ifndef BXVD_P_CHIP_ENABLE
#error BXVD_P_CHIP_ENABLE is undefined!
#endif

#ifndef BXVD_P_RESET_CHIP
#error BXVD_P_RESET_CHIP is undefined!
#endif

#ifndef BXVD_P_INIT_REG_PTRS
#error BXVD_P_INIT_REG_PTRS is undefined!
#endif

#ifndef BXVD_P_WRITE_FWCMD_TO_MBX
#error  BXVD_P_WRITE_FWCMD_TO_MBX is undefined!
#endif

#ifndef BXVD_P_WRITE_FWRSP_MBX
#error BXVD_P_WRITE_FWRSP_MBX is indefined!
#endif

#ifndef BXVD_P_VERIFY_WATCHDOG_FIRED
#error BXVD_P_VERIFY_WATCHDOG_FIRED is undefined!
#endif

#ifndef BXVD_P_VALIDATE_PDEFSETTINGS
#define BXVD_P_VALIDATE_PDEFSETTINGS(pDefSettings) pDefSettings = pDefSettings
#endif

#ifndef BXVD_P_CONTEXT_PLATFORM
/* BXVD_P_Context_Platform can be overridden to define platform
 * specific variables in the BXVD_P_Context structure. */
#define BXVD_P_CONTEXT_PLATFORM
#endif

#ifndef BXVD_P_AVD_INIT_STRIPE_MULTIPLE
/* BXVD_P_AVD_INIT_STRIPE_MULTIPLE can be overridden to specify a
 * different stripe multiple */
#define BXVD_P_AVD_INIT_STRIPE_MULTIPLE 0
#endif

#ifndef BXVD_P_AVD_INIT_STRIPE_WIDTH
/* BXVD_P_AVD_INIT_STRIPE_WIDTH can be overridden to specify a
 * different stripe multiple */
#define BXVD_P_AVD_INIT_STRIPE_WIDTH 0
#endif

#ifndef BXVD_P_PLATFORM_STRIPE_WIDTH_NUM
/*  BXVD_P_PLATFORM_STRIPE_WIDTH can be overridden to specify a
 * differenct number of stipe widths */
#define  BXVD_P_PLATFORM_STRIPE_WIDTH_NUM 2
#endif

#ifndef BXVD_P_AVD_CORE_BAUD_RATE
/* BXVD_P_AVD_CORE_BAUD_RATE can be overridden to specify a different
 * baud rate */
#define BXVD_P_AVD_CORE_BAUD_RATE 115200        /* OL & IL UART baud rate */
#endif

#ifndef BXVD_P_AVD_CORE_UART_FREQ
/* BXVD_P_AVD_CORE_UART_FREQ can be overridden to specify a different
 * frequency */
#define BXVD_P_AVD_CORE_UART_FREQ (200*1000000) /* UART clock frequency */
#endif

#ifndef BXVD_P_WATCHDOG_TIMEOUT
/* BXVD_P_WATCHDOG_TIMEOUT can be overridden to specify a different
 * watchdog timeout value */
#define BXVD_P_WATCHDOG_TIMEOUT 0x0bebc200
#endif

#ifndef BXVD_P_FW_IMAGE_SIZE
/* BXVD_P_FW_IMAGE_SIZE can be overridden to specify a different
 * firmware image size */
#define BXVD_P_FW_IMAGE_SIZE 0x100000
#endif

#ifndef BXVD_P_FW_IMAGE_SIGN_SIZE
/* BXVD_P_FW_IMAGE_SIGN_SIZE can be overridden to specify a different
 * firmware image signature size */
#define BXVD_P_FW_IMAGE_SIGN_SIZE 256
#endif

#ifndef BXVD_P_CHIP_PRODUCT_REVISION
/* BXVD_P_AVD_CHIP_PROD_REVISION can be overridden to specify a
 * platform specific value */
#define BXVD_P_CHIP_PRODUCT_REVISION  BCHP_SUN_TOP_CTRL_PROD_REVISION
#endif

#ifndef BXVD_P_RAVE_CONTEXT_SIZE
/* BXVD_P_RAVE_XXXX register info can be overridden to specify a
 * platform specific value */
#define BXVD_P_RAVE_CONTEXT_SIZE        0
#define BXVD_P_RAVE_CX_HOLD_CLR_STATUS  0
#define BXVD_P_RAVE_PACKET_COUNT        0
#endif

#endif /* _BXVD_PLATFORM_H_ */
