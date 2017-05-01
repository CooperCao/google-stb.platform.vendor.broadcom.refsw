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
*
* Module Description:
*
***************************************************************************/

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "breg_mem.h"            /* Chip register access memory mapped */
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv.h"
#include "bbox_vdc.h"

BDBG_MODULE(BBOX);
BDBG_FILE_MODULE(BBOX_CFG);
BDBG_OBJECT_ID(BBOX_BOX);

void BBOX_P_PrintBoxConfig
    ( BBOX_Config                     *pBoxConfig )
{
#define BBOX_VDC_DEFAULT "VDC default"
#define BBOX_VDC_FTR_INVALID "invalid"

    BBOX_Vdc_Capabilities *pBoxVdcCap;
    uint32_t i, j;
    uint32_t ulXcodeCapVfd, ulXcodeGfd;

    BDBG_ASSERT(pBoxConfig);

    pBoxVdcCap = &pBoxConfig->stVdc;

    BDBG_MODULE_MSG(BBOX_CFG, ("-- Box Mode %d VDC limits --", pBoxConfig->stBox.ulBoxId));

    /* Print source limits */
    for (i=0; i<BAVC_SourceId_eMax; i++)
    {
        uint32_t ulWidth, ulHeight;
        BBOX_Vdc_Colorspace eColorSpace;

        BDBG_MODULE_MSG(BBOX_CFG, ("    Source %d:", i));
        BDBG_MODULE_MSG(BBOX_CFG, ("        available: %s", (pBoxVdcCap->astSource[i].bAvailable == true) ? "true" : "false"));
        BDBG_MODULE_MSG(BBOX_CFG, ("        Mtg: %s", (pBoxVdcCap->astSource[i].bMtgCapable == true) ? "true" : "false"));
        BDBG_MODULE_MSG(BBOX_CFG, ("        bpp: %x", pBoxVdcCap->astSource[i].eBpp));
        BDBG_MODULE_MSG(BBOX_CFG, ("        compressed: %s", (pBoxVdcCap->astSource[i].bCompressed == true) ? "true" : "false"));

        eColorSpace = pBoxVdcCap->astSource[i].eColorSpace;
        if (eColorSpace == BBOX_VDC_DISREGARD)
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        colorspace: %s", BBOX_VDC_DEFAULT));
        }
        else
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        colorspace: %d", eColorSpace));
        }
        ulWidth = pBoxVdcCap->astSource[i].stSizeLimits.ulWidth;
        ulHeight = pBoxVdcCap->astSource[i].stSizeLimits.ulHeight;
        if (ulWidth == BBOX_VDC_DISREGARD && ulHeight == BBOX_VDC_DISREGARD)
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        width: %s, height: %s", BBOX_VDC_DEFAULT, BBOX_VDC_DEFAULT));
        }
        else
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        width: %d, height: %d", ulWidth, ulHeight));
        }
    }

    /* Print display and window limits */
    for (i=0; i<BBOX_VDC_DISPLAY_COUNT; i++)
    {
        BDBG_MODULE_MSG(BBOX_CFG, ("    Display %d:", i));
        BDBG_MODULE_MSG(BBOX_CFG, ("        available: %s", (pBoxVdcCap->astDisplay[i].bAvailable == true) ? "true" : "false"));
        BDBG_MODULE_MSG(BBOX_CFG, ("        max format: %d", pBoxVdcCap->astDisplay[i].eMaxVideoFmt));
        BDBG_MODULE_MSG(BBOX_CFG, ("        max HDMI format: %d", pBoxVdcCap->astDisplay[i].eMaxHdmiDisplayFmt));
        BDBG_MODULE_MSG(BBOX_CFG, ("        mosaic mode class: %d", pBoxVdcCap->astDisplay[i].eMosaicModeClass));
        if (pBoxVdcCap->astDisplay[i].stStgEnc.ulStgId == BBOX_FTR_INVALID)
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        Stg %s:", BBOX_VDC_FTR_INVALID));
            BDBG_MODULE_MSG(BBOX_CFG, ("        Stg available: no"));
            BDBG_MODULE_MSG(BBOX_CFG, ("        Stg Core: %s; Stg Channel: %s", BBOX_VDC_FTR_INVALID, BBOX_VDC_FTR_INVALID));
        }
        else
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        Stg %d:", pBoxVdcCap->astDisplay[i].stStgEnc.ulStgId));
            BDBG_MODULE_MSG(BBOX_CFG, ("        Stg available: %s", (pBoxVdcCap->astDisplay[i].stStgEnc.bAvailable == true) ? "true" : "false"));
            BDBG_MODULE_MSG(BBOX_CFG, ("        Stg Core: %d; Stg Channel: %d", pBoxVdcCap->astDisplay[i].stStgEnc.ulEncoderCoreId,
                pBoxVdcCap->astDisplay[i].stStgEnc.ulEncoderChannel));
        }

        for (j=0; j<BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; j++)
        {
            uint32_t ulWidthFraction, ulHeightFraction, ulMad;
            BBOX_Vdc_Resource_Capture eCap;
            BBOX_Vdc_Resource_Feeder eVfd;
            BBOX_Vdc_Resource_Scaler eScl;
            BBOX_Vdc_SclCapBias eSclCapBias;

            BDBG_MODULE_MSG(BBOX_CFG, ("        Window %d:", j));
            BDBG_MODULE_MSG(BBOX_CFG, ("            available: %s", (pBoxVdcCap->astDisplay[i].astWindow[j].bAvailable == true) ? "true" : "false"));

            ulWidthFraction = pBoxVdcCap->astDisplay[i].astWindow[j].stSizeLimits.ulWidthFraction;
            ulHeightFraction = pBoxVdcCap->astDisplay[i].astWindow[j].stSizeLimits.ulHeightFraction;
            if (ulWidthFraction == BBOX_VDC_DISREGARD && ulHeightFraction == BBOX_VDC_DISREGARD)
            {
                BDBG_MODULE_MSG(BBOX_CFG, ("            width fraction = %s, height fraction = %s", BBOX_VDC_DEFAULT, BBOX_VDC_DEFAULT));
            }
            else
            {
                BDBG_MODULE_MSG(BBOX_CFG, ("            width fraction = %d, height fraction = %d", ulWidthFraction, ulHeightFraction));
            }

            eSclCapBias = pBoxVdcCap->astDisplay[i].astWindow[j].eSclCapBias;
            if (eSclCapBias == BBOX_VDC_DISREGARD)
            {
                BDBG_MODULE_MSG(BBOX_CFG, ("            scl-cap bias: %s", BBOX_VDC_DEFAULT));
            }
            else
            {
                BDBG_MODULE_MSG(BBOX_CFG, ("            scl-cap bias: %d", eSclCapBias));
            }

            ulMad = pBoxVdcCap->astDisplay[i].astWindow[j].stResource.ulMad;
            if (ulMad == BBOX_FTR_INVALID)
            {
                BDBG_MODULE_MSG(BBOX_CFG, ("            deinterlacer: invalid"));
            }
            else
            {
                if (ulMad == BBOX_VDC_DISREGARD)
                {
                    BDBG_MODULE_MSG(BBOX_CFG, ("            deinterlacer: %s", BBOX_VDC_DEFAULT));
                }
                else
                {
                    BDBG_MODULE_MSG(BBOX_CFG, ("            deinterlacer: %x", ulMad));
                }
            }

            eCap = pBoxVdcCap->astDisplay[i].astWindow[j].stResource.eCap;
            if (eCap == BBOX_Vdc_Resource_Capture_eUnknown)
            {
                BDBG_MODULE_MSG(BBOX_CFG, ("            capture: invalid"));
            }
            else
            {
                if (eCap == BBOX_VDC_DISREGARD)
                {
                    BDBG_MODULE_MSG(BBOX_CFG, ("            capture: %s", BBOX_VDC_DEFAULT));
                }
                else
                {
                    BDBG_MODULE_MSG(BBOX_CFG, ("            capture: %x", eCap));
                }
            }

            eVfd = pBoxVdcCap->astDisplay[i].astWindow[j].stResource.eVfd;
            if (eVfd == BBOX_Vdc_Resource_Feeder_eUnknown)
            {
                BDBG_MODULE_MSG(BBOX_CFG, ("            feeder: invalid"));
            }
            else
            {
                if (eVfd == BBOX_VDC_DISREGARD)
                {
                    BDBG_MODULE_MSG(BBOX_CFG, ("            feeder: %s", BBOX_VDC_DEFAULT));
                }
                else
                {
                    BDBG_MODULE_MSG(BBOX_CFG, ("            feeder: %x", eVfd));
                }
            }

            eScl = pBoxVdcCap->astDisplay[i].astWindow[j].stResource.eScl;
            if (eScl == BBOX_Vdc_Resource_Scaler_eUnknown)
            {
                BDBG_MODULE_MSG(BBOX_CFG, ("            scaler: invalid"));
            }
            else
            {
                if (eScl == BBOX_VDC_DISREGARD)
                {
                    BDBG_MODULE_MSG(BBOX_CFG, ("            scaler: %s", BBOX_VDC_DEFAULT));
                }
                else
                {
                    BDBG_MODULE_MSG(BBOX_CFG, ("            scaler: %x", eScl));
                }
            }


        }
    }

    /* Print deinterlacer limits */
    for (i=0; i<BBOX_VDC_DEINTERLACER_COUNT; i++)
    {
        uint32_t ulWidth, ulHeight, ulHsclThreshold;

        BDBG_MODULE_MSG(BBOX_CFG, ("    Deinterlacer %d:", i));

        ulWidth = pBoxVdcCap->astDeinterlacer[i].stPictureLimits.ulWidth;
        ulHeight = pBoxVdcCap->astDeinterlacer[i].stPictureLimits.ulHeight;
        ulHsclThreshold = pBoxVdcCap->astDeinterlacer[i].ulHsclThreshold;

        if (ulWidth == BBOX_VDC_DISREGARD && ulHeight == BBOX_VDC_DISREGARD)
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        width: %s, height: %s", BBOX_VDC_DEFAULT, BBOX_VDC_DEFAULT));
        }
        else
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        width: %d, height: %d", ulWidth, ulHeight));
        }

        if (ulHsclThreshold == BBOX_VDC_DISREGARD)
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        HSCL threshold: %s", BBOX_VDC_DEFAULT));
        }
        else
        {
            BDBG_MODULE_MSG(BBOX_CFG, ("        HSCL threshold: %d", pBoxVdcCap->astDeinterlacer[i].ulHsclThreshold));
        }
    }

    ulXcodeCapVfd = pBoxVdcCap->stXcode.ulNumXcodeCapVfd;
    ulXcodeGfd = pBoxVdcCap->stXcode.ulNumXcodeGfd;

    /* Print  transcode limits */
    if (ulXcodeCapVfd == BBOX_VDC_DISREGARD)
    {
        BDBG_MODULE_MSG(BBOX_CFG, ("    Xcode cap-vfd: %s", BBOX_VDC_DEFAULT));
    }
    else
    {
        BDBG_MODULE_MSG(BBOX_CFG, ("    Xcode cap-vfd: %d", pBoxVdcCap->stXcode.ulNumXcodeCapVfd));
    }

    if (ulXcodeGfd == BBOX_VDC_DISREGARD)
    {
        BDBG_MODULE_MSG(BBOX_CFG, ("    Xcode gfd: %s", BBOX_VDC_DEFAULT));
    }
    else
    {
        BDBG_MODULE_MSG(BBOX_CFG, ("    Xcode gfd: %d", pBoxVdcCap->stXcode.ulNumXcodeGfd));
    }
}

/***************************************************************************
 *
 */
BERR_Code BBOX_Open
    ( BBOX_Handle                     *phBox,
      const BBOX_Settings             *pBoxSettings )
{
    BBOX_P_Context *pBox = NULL;
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BBOX_Open);
    BDBG_ASSERT(phBox);
    BDBG_ASSERT(pBoxSettings);

    /* The handle will be NULL if create fails. */
    *phBox = NULL;

    /* (1) Alloc the main BOX context. */
    pBox = (BBOX_P_Context*)(BKNI_Malloc(sizeof(BBOX_P_Context)));
    if(NULL == pBox)
    {
        eStatus = BERR_OUT_OF_SYSTEM_MEMORY;
        goto BBOX_Open_Done;
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pBox, 0x0, sizeof(BBOX_P_Context));
    BDBG_OBJECT_SET(pBox, BBOX_BOX);

    /* Validate box id */
    eStatus = BBOX_P_ValidateId(pBoxSettings->ulBoxId);
    if (eStatus == BERR_SUCCESS)
    {
        pBox->stBoxConfig.stBox.ulBoxId = pBoxSettings->ulBoxId;
    }
    else
    {
        goto BBOX_Open_Done;
    }

    pBox->bRtsLoaded = false;

    /* All done. now return the new fresh context to user. */
    *phBox = (BBOX_Handle)pBox;

BBOX_Open_Done:
    BDBG_LEAVE(BBOX_Open);

    if ((BERR_SUCCESS != eStatus) && (NULL != pBox))
    {
        BDBG_OBJECT_DESTROY(pBox, BBOX_BOX);
        BKNI_Free((void*)pBox);
    }

    return BERR_TRACE(eStatus);
}

/***************************************************************************
 *
 */
BERR_Code BBOX_Close
    ( BBOX_Handle                     hBox )
{
    BDBG_ENTER(BBOX_Close);

    if(!hBox)
    {
        goto done;
    }

    BDBG_OBJECT_ASSERT(hBox, BBOX_BOX);

    BDBG_OBJECT_DESTROY(hBox, BBOX_BOX);
    BKNI_Free((void*)hBox);

done:
    BDBG_LEAVE(BBOX_Close);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BBOX_GetConfig
    ( BBOX_Handle                      hBox,
      BBOX_Config                     *pBoxConfig )
{
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BBOX_GetConfig);

    if (hBox == NULL) /* Implied box mode ID 0 */
    {
        pBoxConfig->stBox.ulBoxId = 0;

        /* VDC */
        eStatus = BBOX_P_Vdc_SetBoxMode(pBoxConfig->stBox.ulBoxId, &pBoxConfig->stVdc);
        if (eStatus != BERR_SUCCESS)
        {
            goto BBOX_GetConfig_Done;
        }

        /* Get memconfig settings  */
        eStatus = BBOX_P_GetMemConfig(pBoxConfig->stBox.ulBoxId, &pBoxConfig->stMemConfig);
        if (eStatus != BERR_SUCCESS)
        {
            goto BBOX_GetConfig_Done;
        }

        /* VCE */
        eStatus = BBOX_P_Vce_SetBoxMode(pBoxConfig->stBox.ulBoxId, &pBoxConfig->stVce);
        if (eStatus != BERR_SUCCESS)
        {
            goto BBOX_GetConfig_Done;
        }

        /* XVD */
        eStatus = BBOX_P_Xvd_SetBoxMode(pBoxConfig->stBox.ulBoxId, &pBoxConfig->stXvd);
        if (eStatus != BERR_SUCCESS)
        {
            goto BBOX_GetConfig_Done;
        }

        /* Audio */
        eStatus = BBOX_P_Audio_SetBoxMode(pBoxConfig->stBox.ulBoxId, &pBoxConfig->stAudio);
        if (eStatus != BERR_SUCCESS)
        {
            goto BBOX_GetConfig_Done;
        }

        /* other PI/upper layer SW capabilities here */

    }
    else
    {
        BDBG_OBJECT_ASSERT(hBox, BBOX_BOX);

         /* VDC */
        eStatus = BBOX_P_Vdc_SetBoxMode(hBox->stBoxConfig.stBox.ulBoxId, &hBox->stBoxConfig.stVdc);
        if (eStatus != BERR_SUCCESS)
        {
            goto BBOX_GetConfig_Done;
        }

        eStatus = BBOX_P_GetMemConfig(hBox->stBoxConfig.stBox.ulBoxId, &hBox->stBoxConfig.stMemConfig);
        if (eStatus != BERR_SUCCESS)
        {
            goto BBOX_GetConfig_Done;
        }

        if (hBox->stBoxConfig.stBox.ulBoxId != 0)
        {
            /* Check memc assignment against BOX config */
            eStatus = BBOX_P_Vdc_SelfCheck(&hBox->stBoxConfig.stMemConfig, &hBox->stBoxConfig.stVdc);
            if (eStatus != BERR_SUCCESS)
            {
                eStatus = BERR_INVALID_PARAMETER;
                goto BBOX_GetConfig_Done;
            }
        }

        /* VCE */
        eStatus = BBOX_P_Vce_SetBoxMode(hBox->stBoxConfig.stBox.ulBoxId, &hBox->stBoxConfig.stVce);
        if (eStatus != BERR_SUCCESS)
        {
             goto BBOX_GetConfig_Done;
        }

        /* XVD */
        eStatus = BBOX_P_Xvd_SetBoxMode(hBox->stBoxConfig.stBox.ulBoxId, &hBox->stBoxConfig.stXvd);
        if (eStatus != BERR_SUCCESS)
        {
            goto BBOX_GetConfig_Done;
        }

        /* Audio */
        eStatus = BBOX_P_Audio_SetBoxMode(hBox->stBoxConfig.stBox.ulBoxId, &hBox->stBoxConfig.stAudio);
        if (eStatus != BERR_SUCCESS)
        {
             goto BBOX_GetConfig_Done;
        }

        /* other PI/upper layer SW capabilities here */

        *pBoxConfig = hBox->stBoxConfig;
    }

    BDBG_MODULE_MSG(BBOX_CFG, ("hBox[%p] : ID=%d", (void *)hBox, pBoxConfig->stBox.ulBoxId));

    BBOX_P_PrintBoxConfig(pBoxConfig);

BBOX_GetConfig_Done:
    BDBG_LEAVE(BBOX_GetConfig);
    return eStatus;
}

/***************************************************************************
 *
 */

void BBOX_GetDefaultLoadRtsSettings( BBOX_LoadRtsSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BBOX_LoadRts
    ( BBOX_Handle                 hBox,
      const BREG_Handle           hReg,
      const BBOX_LoadRtsSettings *pSettings )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BBOX_LoadRts);

    BDBG_ASSERT(hBox);
    BDBG_ASSERT(hReg);
    BDBG_OBJECT_ASSERT(hBox, BBOX_BOX);

    if (hBox->bRtsLoaded)
    {
        BDBG_ERR(("RTS is already loaded."));
        err = BBOX_RTS_ALREADY_LOADED;
        goto BBOX_LoadRts_Done;
    }
    else
    {
#if BBOX_LOAD_RTS
        BBOX_DramRefreshRate eRefreshRate;

        BDBG_ASSERT(pSettings->eRefreshRate < BBOX_DramRefreshRate_eInvalid);

        eRefreshRate = (pSettings->eRefreshRate == BBOX_DramRefreshRate_eDefault) ?
                        hBox->stBoxConfig.stMemConfig.eRefreshRate : pSettings->eRefreshRate;

        err = BBOX_P_LoadRts(hReg, hBox->stBoxConfig.stBox.ulBoxId, eRefreshRate);
#endif
        if (err != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to load RTS for Box Mode %d", hBox->stBoxConfig.stBox.ulBoxId));
        }
        else
        {
            hBox->bRtsLoaded = true;
        }
    }

BBOX_LoadRts_Done:
    BDBG_LEAVE(BBOX_LoadRts);
    return err;
}

/* end of file */
