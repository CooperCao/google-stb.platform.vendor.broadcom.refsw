/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "breg_mem.h"
#include "bchp.h"
#include "bchp_priv.h"
#include "bchp_11360.h"
#include "bchp_v3d_dbg.h"
#include "bchp_v3d_ctl.h"

BDBG_MODULE(BCHP);

/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do
 * it this way to work-around those problems.
 *
 * TODO: Update this table to support new revisions.
 */
static const struct BCHP_P_Info s_aChipInfoTable[] =
{
    /* Chip Family contains the major and minor revs */
#if BCHP_VER == BCHP_VER_B0
    {0xd300},
#else
    #error "Port required"
#endif
    {0} /* terminate */
};

/* Static function prototypes */
static BERR_Code BCHP_P_GetFeature
    ( const BCHP_Handle                hChip,
      const BCHP_Feature               eFeature,
      void                            *pFeatureValue );

static BERR_Code BCHP_P_ResetMagnumCores
    ( const BCHP_Handle                hChip );

BERR_Code BCHP_Open11360
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_OpenSettings openSettings;
    BCHP_GetDefaultOpenSettings(&openSettings);
    openSettings.reg = hRegister;
    return BCHP_Open(phChip, &openSettings);
}

BERR_Code BCHP_Open( BCHP_Handle *phChip, const BCHP_OpenSettings *pSettings )
{
    BCHP_P_Context *pChip;

    BDBG_ENTER(BCHP_Open11360);

    /* If error ocurr user get a NULL *phChip */
    *phChip = NULL;

    pChip = BCHP_P_Open(pSettings, s_aChipInfoTable);
    if (!pChip) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;

    BCHP_P_ResetMagnumCores( pChip );

    /* All done. now return the new fresh context to user. */
    *phChip = (BCHP_Handle)pChip;


    BDBG_LEAVE(BCHP_Open11360);
    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_GetFeature
    ( const BCHP_Handle                hChip,
      const BCHP_Feature               eFeature,
      void                            *pFeatureValue )
{
    BERR_Code            rc = BERR_UNKNOWN;

    BDBG_ENTER(BCHP_P_GetFeature);

    BDBG_OBJECT_ASSERT(hChip, BCHP);

    /* which feature? */
    switch (eFeature)
    {
    case BCHP_Feature_e3DGraphicsCapable:
        /* 3D capable? (bool) */
        *(bool *)pFeatureValue = true;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eDvoPortCapable:
        /* dvo port capable? (bool) */
        *(bool *)pFeatureValue = false;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eMacrovisionCapable:
        *(bool *)pFeatureValue = false;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eMpegDecoderCount:
        /* number of MPEG decoders (int) */
        *(int *)pFeatureValue = 0;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eHdcpCapable:
        /* HDCP capable? (bool) */
        *(bool *)pFeatureValue = false;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_e3desCapable:
        /* 3DES capable? (bool) */
        *(bool *)pFeatureValue = false;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_e1080pCapable:
        *(bool *)pFeatureValue = false;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eDisabledL2Registers:
        BKNI_Memset(pFeatureValue, 0, sizeof(BCHP_FeatureData));
        rc = BERR_SUCCESS;
        break;

    default:
        rc = BCHP_P_GetDefaultFeature(hChip, eFeature, pFeatureValue);
        break;
    }

    /* return result */
    BDBG_LEAVE(BCHP_P_GetFeature);
    return rc;
}


#if !defined(EMULATION)
static void BCHP_P_ResetV3dCore( const BCHP_Handle hChip, const BREG_Handle hReg )
{
    BREG_Handle  hRegister = hChip->regHandle;
    BSTD_UNUSED(hReg);

    /* clear any pending interrupts */
    BREG_Write32( hRegister, BCHP_V3D_CTL_INTCTL, ~0);
    BREG_Write32( hRegister, BCHP_V3D_DBG_DBQITC, ~0);
}
#endif

/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_ResetMagnumCores
    ( const BCHP_Handle                hChip )
{
    BREG_Handle  hRegister = hChip->regHandle;
    uint32_t reg_value;
    BERR_Code result = BERR_SUCCESS;

    BDBG_ENTER(BCHP_P_ResetMagnumCores);
	BDBG_MSG(("Resetting V3D core\n"));

#if !defined(EMULATION)
    BCHP_P_ResetV3dCore(hChip, hRegister);
#endif
	BDBG_LEAVE(BCHP_P_ResetMagnumCores);
	return result;
}

/* End of File */
