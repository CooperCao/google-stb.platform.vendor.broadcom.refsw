/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#ifndef BBOX_H__
#define BBOX_H__

#include "bavc_types.h"
#include "berr_ids.h"            /* Error codes */
#include "bbox_vdc.h"
#include "bbox_vce.h"
#include "bbox_rts.h"
#include "bbox_audio.h"
#include "bbox_xvd.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The box module describes the list of capabilities available for a given
platform. The intent is to expose only those controls and capabilities that
pertain to the given platform.
****************************************************************************/

/***************************************************************************
Summary:
    List of errors unique to BOX
****************************************************************************/
#define BBOX_ID_NOT_SUPPORTED                  BERR_MAKE_CODE(BERR_BOX_ID, 0)
#define BBOX_RTS_ALREADY_LOADED                BERR_MAKE_CODE(BERR_BOX_ID, 1)
#define BBOX_ID_AND_RTS_MISMATCH               BERR_MAKE_CODE(BERR_BOX_ID, 2)
#define BBOX_INCORRECT_MEMC_COUNT              BERR_MAKE_CODE(BERR_BOX_ID, 3)
#define BBOX_WINDOW_SIZE_EXCEEDS_LIMIT         BERR_MAKE_CODE(BERR_BOX_ID, 4)
#define BBOX_FRAME_BUFFER_SIZE_EXCEEDS_LIMIT   BERR_MAKE_CODE(BERR_BOX_ID, 5)
#define BBOX_RTS_LOADED_BY_CFE                 BERR_MAKE_CODE(BERR_BOX_ID, 6)
#define BBOX_RTS_CFG_UNINITIALIZED             BERR_MAKE_CODE(BERR_BOX_ID, 7)
#define BBOX_MEM_CFG_UNINITIALIZED             BERR_MAKE_CODE(BERR_BOX_ID, 8)

/***************************************************************************
Summary:
    Used to specify memc index.
****************************************************************************/
typedef enum BBOX_MemcIndex
{
    BBOX_MemcIndex_0 = 0,
    BBOX_MemcIndex_1,
    BBOX_MemcIndex_2,
    BBOX_MemcIndex_Invalid
} BBOX_MemcIndex;

/***************************************************************************
Summary:
    Used to specify DRAM refresh rate.
****************************************************************************/
typedef enum BBOX_DramRefreshRate
{
    BBOX_DramRefreshRate_eDefault = 0,
    BBOX_DramRefreshRate_e1x,
    BBOX_DramRefreshRate_e2x,     /* high-temp */
    BBOX_DramRefreshRate_e4x = 4, /* high-temp */
    BBOX_DramRefreshRate_eInvalid
} BBOX_DramRefreshRate;


/***************************************************************************
Summary: Specifies settings for a box mode.

Description:
    This data structure specifies the unique identifier for a given box mode.

See Also:
    BBOX_Open
****************************************************************************/
typedef struct BBOX_Settings
{
    uint32_t           ulBoxId; /* This provided by app or upper layer SW
                                   and is determinate. */
} BBOX_Settings;


/***************************************************************************
Summary:
    Data structure describing memc index settings for the given box mode.

Description:

See Also: BBOX_GetMemConfig
****************************************************************************/
typedef struct BBOX_MemConfig
{
    /* VDC MEMC index */
    BBOX_Vdc_MemcIndexSettings     stVdcMemcIndex;

    uint32_t                       ulNumMemc;

    /* specifies refresh rate */
    BBOX_DramRefreshRate           eRefreshRate;

    /* TODO: Add XVD MEMC index */

} BBOX_MemConfig;

/***************************************************************************
Summary:
    Data structure describing capabilities exposed by given PI modules
    and/or upper layer SW for the given box mode.

Description:

See Also: BBOX_GetConfig
****************************************************************************/
typedef struct BBOX_Config
{
    BBOX_Settings         stBox;

    /* VDC features */
    BBOX_Vdc_Capabilities stVdc;

    /* VCE features */
    BBOX_Vce_Capabilities stVce;

    /* Audio features */
    BBOX_Audio_Capabilities stAudio;

    /* XVD features */
    BBOX_Xvd_Config stXvd;

    /* Add other module capabilities here */

    BBOX_MemConfig        stMemConfig;

} BBOX_Config;

/***************************************************************************
Summary:
    The handle for the BOX module.

Description:
    This is the main handle required to access box mode capabilities of a
    given module.

See Also:
    BBOX_Open
****************************************************************************/
typedef struct BBOX_P_Context *BBOX_Handle;

/***************************************************************************
Summary:
    Opens the BOX module.

Description:

    A box mode ID is passed to determine the appropriate configuration.

    See BBOX_GetConfig on how the box mode ID is used.

Returns:
    Returns a handle to the BOX module.

    BBOX_ID_NOT_SUPPORTED - passed ID is not supported
    BERR_OUT_OF_SYSTEM_MEMORY - failed to allocate memory for BOX module
    BERR_SUCCESS - Successfully opened the BOX module

See Also:
    BBOX_GetConfig, BBOX_Close, BBOX_Settings
****************************************************************************/
BERR_Code BBOX_Open
    ( BBOX_Handle                     *phBox,          /* [out] BOX handle */
      const BBOX_Settings             *pBoxSettings ); /* [in]  */


/***************************************************************************
Summary:
    Closes the BOX module.

Description:

Returns:

See Also:
    BBOX_Open
****************************************************************************/
BERR_Code BBOX_Close
    ( BBOX_Handle                      hBox ); /* [in] BOX handle to close */

/*****************************************************************************
Summary:
    Get a box mode configuration

Description:
    This has 3 usages:

    1. For legacy chips that do not know about box modes, an upper layer
    software can pass a NULL for the BBOX_Handle parameter.
    This implies that a box mode ID of 0 is used. This is to address
    backwards compatibility for these legacy chips and whose SW does not
    call BBOX_Open.

        A filled BBOX_Config is returned.

    2. For legacy chips that know about box modes, an upper layer software
    passes a valid BBOX_Handle obtained from BBOX_Open. BBOX_Open is called
    with a box mode ID and will return a BBOX_Config struct according to the
    following.

        a) A box mode ID of 0 returns a filled BBOX_Config.
        b) A non-zero box mode ID returns an empty BBOX_Config.

    3. For chips that know about box modes, an upper layer software
    passes a valid BBOX_Handle obtained from BBOX_Open. BBOX_Open is called
    with a box mode ID and will return a BBOX_Config struct according to the
    following.

        a) A box mode ID of 0 returns an empty BBOX_Config.
        b) A valid box mode ID returns a filled BBOX_Config.
        c) An invalid non-zero box mode ID returns an empty BBOX_Config.

Returns:
    BERR_INVALID_PARAMETER

See Also:
    BBOX_Open, BBOX_Config
*****************************************************************************/
BERR_Code BBOX_GetConfig
    ( BBOX_Handle                      hBox,
      BBOX_Config                     *pBoxConfig ); /* [out] Box configuration. */

/***************************************************************************
Summary:
    Data structure describing LoadRts settings

Description:

See Also: BBOX_LoadRts
****************************************************************************/

typedef struct BBOX_LoadRtsSettings
{
    /* BBOX may lower the refresh rate for certain clients if the RTS specified
       (default) refresh rate allows it. This gives more BW to these clients
       BUT the limits remain the same.

       The amount by which the refresh rate is lowered is the ratio of the
       default refresh rate and eRefreshRate. For example, default rate is 2x,
       eRefreshRate is 1x, then the amount to lower the default rate is 2.

       If the caller doesn't set this, the default refresh rate is used. */
    BBOX_DramRefreshRate               eRefreshRate;

} BBOX_LoadRtsSettings;

/***************************************************************************
Summary:
    Loads the default Load RTS settings

Description:

Returns:

See Also:
    BBOX_LoadRts
****************************************************************************/

void BBOX_GetDefaultLoadRtsSettings( BBOX_LoadRtsSettings *pSettings );

/***************************************************************************
Summary:
    Loads the RTS set according to the given box mode.

Description:
    Each box mode has an associated RTS. This function facilitates loading
    of this RTS set without rebooting the box. However, this function
    MUST only called at initialization and cannot be called at any point
    thereafter.

Returns:
    BERR_INVALID_PARAMETER
    BBOX_RTS_ALREADY_LOADED

See Also:
    BBOX_Open
****************************************************************************/

BERR_Code BBOX_LoadRts
    ( BBOX_Handle                      hBox,
      const BREG_Handle                hReg,
      const BBOX_LoadRtsSettings *pSettings );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BBOX_H__ */

/* end of file */
