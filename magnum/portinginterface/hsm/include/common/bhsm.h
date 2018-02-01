/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#ifndef BHSM_H__
#define BHSM_H__

#include "bchp.h"
#include "bint.h"
#include "breg_mem.h"

#define BHSM_ZEUS_VERSION_CALC(major,minor)            ((((major) & 0xFF)<<16) | (((minor) & 0xFF)<<8)                      )
#define BHSM_ZEUS_VERSION_CALC_3(major,minor,subMinor) ((((major) & 0xFF)<<16) | (((minor) & 0xFF)<<8) | ((subMinor) & 0xFF))

#if BHSM_ZEUS_VER_MAJOR < 1
    #error This header is only for Zeus chips.
#endif

/* These platforms have basic ASKM support */
#define BHSM_ZEUS_VERSION BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR)
#define BHSM_ZEUS_FULL_VERSION BHSM_ZEUS_VERSION_CALC_3(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR,BHSM_ZEUS_VER_SUBMINOR)

/*    DEPRECATED  The following are replaced by BHSM_ZEUS_VERSION, BHSM_ZEUS_VER_MAJOR, and BHSM_ZEUS_VER_MINOR */
#define HSM_IS_ASKM 1
#define HSM_IS_ASKM_40NM 1

#if ( BHSM_ZEUS_VERSION_CALC(1,0) == BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR) ) /* Only active for Zeus 1*/
#define HSM_IS_ASKM_40NM_ZEUS_1_0 1
#endif

#if ( BHSM_ZEUS_VERSION_CALC(2,0) <= BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR) )
#define HSM_IS_ASKM_40NM_ZEUS_2_0 1
#endif

#if ( BHSM_ZEUS_VERSION_CALC(2,2) <= BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR) )
#define HSM_IS_ASKM_40NM_ZEUS_2_5 1  /* should be named*_2_2  */
#endif

#if ( BHSM_ZEUS_VERSION_CALC(3,0) <= BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR) )
#define HSM_IS_ASKM_40NM_ZEUS_3_0 1
#endif

#if ( BHSM_ZEUS_VERSION_CALC(4,0) <= BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR) )
#define HSM_IS_ASKM_28NM_ZEUS_4_0 1
#endif

#if ( BHSM_ZEUS_VERSION_CALC(4,1) <= BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR) )
#define HSM_IS_ASKM_28NM_ZEUS_4_1 1
#endif

#if ( BHSM_ZEUS_VERSION_CALC(4,2) <= BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR) )
#define HSM_IS_ASKM_28NM_ZEUS_4_2 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ZEUS_VERSION BHSM_ZEUS_VERSION
#define ZEUS_4_2 BHSM_ZEUS_VERSION_CALC(4,2)
#define ZEUS_4_1 BHSM_ZEUS_VERSION_CALC(4,1)
#define ZEUS_4_0 BHSM_ZEUS_VERSION_CALC(4,0)

#define FLASHMAP_VERSION_V3 3
#define FLASHMAP_VERSION_V5 5
#define FLASHMAP_VERSION FLASHMAP_VERSION_V5

#include "bhsm_datatypes.h"

/* supported features */
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
  #define BHSM_SUPPORT_INITIALIZE_BYPASS_KEYSLOTS 1   /* BHSM_InitialiseBypassKeyslots exposed on API */
#endif

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
#define BHSM_SUPPORT_KEYSLOT_OWNERSHIP 1  /* not all Zeus3 minor versions support keyslot ownership. */
#endif


/***************************************************************************
Summary:
Host Secure Module (HSM) module context handle.

Description:
BHSM_Handle holds the context of the Host Secure Module. The system should
have only one BHSM_Handle. Caller of BHSM_Open is responsible to store
this BHSM_Handle and uses it for the future function call after BHSM_Open function
returns successfully.

****************************************************************************/
typedef struct BHSM_P_Handle *BHSM_Handle;



/*****************************************************************************
Summary:
This function returns a recommended default settings for HSM module.

Description:
This function shall be called before BHSM_Open and the caller can then over-ride
any of the default settings required for the build and configuration by calling BHSM_Open.

Returns:
BERR_SUCCESS - success. Otherwise, there is an error.

*****************************************************************************/
BERR_Code BHSM_GetDefaultSettings( BHSM_Settings *pSettings, BCHP_Handle chipHandle );


/*****************************************************************************
Summary:
Returns the capabilities of the HSM module.

Description:

Calling Context:
It can be called anytime after BHSM initialization.

Performance and Timing:
Function will return immediately.
*****************************************************************************/
BERR_Code BHSM_GetCapabilities( BHSM_Handle hHsm, BHSM_Capabilities_t *pCaps );


/*****************************************************************************
Summary:
This function creates the Host Secure Module handle.

Description:

The caller can pass a NULL pointer for pSettings. If the
p_Settings pointer is NULL, default settings should be used.

It is the caller responsibility to store the returned handle and use
it for the future function call after this function returns
successfully.

Before calling this function, the only function that the caller
can call is BHSM_GetDefaultSettings. System shall not call
any other Host Secure functions prior to this function.

System shall not call this function more than once without calling BHSM_Close
previously.

If illegal settings are passed in an error should be
returned and the hardware state should not be modified.

The BINT_Handle is only required if this module needs to
associate ISR callback routines with L2 interrupts.

Calling Context:
The function shall be called from application level or from driver level
(for example in Linux, during insmod )

Performance and Timing:
This is a synchronous function that will return when it is done.

Returns:
BERR_SUCCESS - success

******************************************************************************/
BERR_Code BHSM_Open( BHSM_Handle            *hpHsm,
                     BREG_Handle            hReg,
                     BCHP_Handle            hChip,
                     BINT_Handle            hInterrupt,
                     const BHSM_Settings    *pSettings );


/*****************************************************************************
Summary:
This function frees the main handle and any resources contained in the main handle.

Description:
This function shall free the main handle and any resources contained
in the main handle. This function shall try to free any resources associated
with sub handles created from the main handle.

Other than BHSM_GetDefaultSettings, system shall not call any other HSM
functions after this function returns, regardless of the return result.

******************************************************************************/
BERR_Code BHSM_Close( BHSM_Handle hHsm );


/*****************************************************************************
Summary:
Function Deprecated.
******************************************************************************/
BERR_Code BHSM_SubmitRawCommand (
    BHSM_Handle         hHsm,
    BHSM_HwModule        interface,
    unsigned            inputParamLenInWord,
    uint32_t            *pInputParamsBuf,
    unsigned            *pOutputParamLenInWord, /* in-out */
    uint32_t            *pOutputParamsBuf );


void BHSM_MemcpySwap (
    unsigned char *pDest,
    unsigned char *pData,
    unsigned int  len,
    bool swap );

#ifdef __cplusplus
}
#endif


#endif /* BHSM_H__ */
