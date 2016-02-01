/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
*   Module name: SRC
*   This file contains the implementation of all PIs for the Sample
*   Rate Converter abstraction. 
* 
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap_src_priv.h"

/* TODO:
    To be programmed at FMM Init
    AUD_FMM_SRC_CTRL0_COEFF4X_[0..127].DAT and AUD_FMM_SRC_CTRL0_COEFF2X_[0..63].DAT
*/

BDBG_MODULE(rap_src);

#define BRAP_SRCCH_P_INVALID_SAMPLING_RATE  (-1)


/* Default Settings and Parameters */
static const BRAP_SRC_P_Params defSrcParams =
{
    BRAP_SRC_P_Type_eUnknown,           /* eSrcType */
    false,                              /* bRampEnable */
    false,                              /* bPriorityHigh */
    BRAP_SRCCH_P_INVALID_SAMPLING_RATE, /* eInputSR */
    BRAP_SRCCH_P_INVALID_SAMPLING_RATE, /* eOutputSR */
    BRAP_INVALID_VALUE,                 /* uiGroupID */
    BRAP_INVALID_VALUE,                 /* ui32FciId */
};

static BERR_Code BRAP_SRC_P_EnableDisable(
            BRAP_SRC_P_Handle   hSrc,       /* [in] SRC Handle */
            unsigned int        uiEnable    /* [in] Enable/Disable */
);

/***************************************************************************
Summary:
    Returns default values for SRC Start time parameters.

Description:
    For parameters that the system cannot assign default values to, 
    an invalid value is returned. Note that the default parameters are common
    for all SRC.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SRC_P_GetDefaultSettings.
**************************************************************************/
BERR_Code 
BRAP_SRC_P_GetDefaultParams ( 
    BRAP_SRC_P_Params    *pDefParams   /* [out] Pointer to memory where 
                                          default settings should be 
                                          written */    
)
{
    BDBG_ENTER (BRAP_SRC_P_GetDefaultParams);
    BDBG_ASSERT (pDefParams);

    *pDefParams = defSrcParams;

    BDBG_LEAVE(BRAP_SRC_P_GetDefaultParams);
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Returns the SRC type.
Description:
    Returs the SRC type based on the Input and Output Sampling Rates.
Returns:
    BERR_SUCCESS on success else error
See Also:
**************************************************************************/
static BERR_Code BRAP_SRC_P_GetSrcType(
    unsigned int        uiInSr,     /* [in] In SR */      
    unsigned int        uiOutSr,    /* [in] Out SR */
    BRAP_SRC_P_Type     *peSrcType  /* [out] SRC type */
)
{
    BERR_Code       ret = BERR_SUCCESS;
    unsigned int    uiSrcRatio = 0;
    unsigned int    uiSrcRemainder = 0;
    
    BDBG_ENTER(BRAP_SRC_P_GetSrcType);
    BDBG_ASSERT(peSrcType);
    
    *peSrcType = BRAP_SRC_P_Type_eUnknown;

    if((0 == uiInSr)||(0 == uiOutSr))
    {
        BDBG_ERR(("Input/Output Sampling Rate Invalid"));
        BDBG_ASSERT(0);
    }
    
    /* If Input and Output Sampling Rate are Equal */
    if(uiInSr == uiOutSr)
    {
        /* Program SRC type to Bypass Mode */
        *peSrcType = BRAP_SRC_P_Type_eBypass;
    }
    /* If the Input Sample Rate is higher than Output Sample Rate */
    else if(uiInSr > uiOutSr)
    {
        /* Calculate the SRC ratio and Find if Input SR is Integral
           multiple of Output SR*/
        uiSrcRatio = uiInSr / uiOutSr;
        uiSrcRemainder = uiInSr % uiOutSr;

        /* If Input SR is Integral multiple of Output SR and Ratio is either 2 
           or 4,program the SRC in DownSampling Mode else Program SRC Type to 
           be Linear Interpolation */
        if(0 == uiSrcRemainder)
        {
            if(2 == uiSrcRatio)
            {
                *peSrcType = BRAP_SRC_P_Type_eDown2;
            }
            else if(4 == uiSrcRatio)
            {
                *peSrcType = BRAP_SRC_P_Type_eDown4;
            }
            else
            {
                /* 96 -> 32 etc. 
                   Note: This would require 2 SRCs in cascade
                   AM shouldn't pass such combinations.
                   If at all such a case is required to be supported
                   AM should break it down to 2 steps and then 
                   call start for each of them with proper in and out
                   sampling rates */

                BDBG_ERR(("BRAP_SRC_P_GetSrcType: SRC Ratio not Supported"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);             
            }
        }
        else
        {
            if(2 > uiSrcRatio)
            {
                *peSrcType = BRAP_SRC_P_Type_eLinInt;
            }
            else
            {
                /* Linear Interpolation supports only up 0 to 2 Sample rate 
                   conversion */
                BDBG_ERR(("BRAP_SRC_P_GetSrcType: SRC Ratio not Supported"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
    }
    /* If the Output Sample Rate is higher than Input Sample Rate */
    else /* if(eInputSR < eOutputSR) */
    {
        /* Calculate the SRC ratio and Find if Output SR is Integral
           multiple of Input SR*/
        uiSrcRatio = uiOutSr / uiInSr;
        uiSrcRemainder = uiOutSr % uiInSr;

        /* If Output SR is Integral multiple of Input SR and Ratio is either 2 
           or 4, program the SRC in UpSampling Mode else Program SRC Type to be 
           Linear Interpolation */
        if(0 == uiSrcRemainder)
        {
            if(2 == uiSrcRatio)
            {
                *peSrcType = BRAP_SRC_P_Type_eUp2;
            }
            else if(4 == uiSrcRatio)
            {
                *peSrcType = BRAP_SRC_P_Type_eUp4;
            }
            else
            {
                /* 32->96 etc. 
                   Note: This would require 2 SRCs in cascade
                   AM shouldn't pass such combinations.
                   If at all such a case is required to be supported
                   AM should break it down to 2 steps and then 
                   call start for each of them with proper in and out
                   sampling rates */

                BDBG_ERR(("BRAP_SRC_P_GetSrcType: SRC Ratio not Supported"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);             
            }
        }
        else
        {
            if(2 > uiSrcRatio)
            {
                *peSrcType = BRAP_SRC_P_Type_eLinInt;
            }
            else
            {
                /* Linear Interpolation supports only up 0 to 2 Sample rate 
                   conversion */
                BDBG_ERR(("BRAP_SRC_P_GetSrcType: SRC Ratio not Supported"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
    }

    
    BDBG_LEAVE(BRAP_SRC_P_GetSrcType);
    return ret;
}


/***************************************************************************
Summary:
    Returns the SRC Info.
Description:
    Returs the SRC Info based on the Input and Output Sampling Rates and 
    bCompressed.
Returns:
    BERR_SUCCESS on success else error
See Also:
**************************************************************************/
BERR_Code BRAP_SRC_P_GetRequiredSrc(
    BRAP_SRC_P_SrcInfo          *pSrcInfo,  /* [out] SRC Info */
    BAVC_AudioSamplingRate      eInputSR,   /* [in] Input SRC Type */
    BAVC_AudioSamplingRate      eOutputSR,  /* [in] Output SRC Type*/
    bool                        bCompressed /* [in] True: compressed data
                                                    False: PCM data
                                                    Note: If true, eInputSR and
                                                    eOutputSR are ignored. */
    )
{
    BERR_Code           ret = BERR_SUCCESS;
    unsigned int        uiInSr = 0;
    unsigned int        uiOutSr = 0;
    unsigned int        i =0;
    
    BDBG_ENTER(BRAP_SRC_P_GetRequiredSrc);
    BDBG_ASSERT(pSrcInfo);

    /* Init to invalid values */
    pSrcInfo->uiNumSrc = 0;
    for(i = 0; i < BRAP_RM_P_MAX_SRC_IN_CASCADE; i++)
    {
        pSrcInfo->eSrcType[i] = BRAP_SRC_P_Type_eUnknown;
        pSrcInfo->eInSR[i] = BAVC_AudioSamplingRate_eUnknown;
        pSrcInfo->eOutSR[i] = BAVC_AudioSamplingRate_eUnknown;
    }

    /* If compressed data, ignore the eInputSR and eOutputSR and prepare the
       pSrcInfo for bypass mode */       
    if(true == bCompressed)
    {
        BDBG_MSG(("BRAP_SRC_P_GetRequiredSrc: SRC carrying compressed data"));
        pSrcInfo->uiNumSrc = 1;        
        pSrcInfo->eSrcType[0] = BRAP_SRC_P_Type_eBypass;

        /* We are done! */
        goto exit;
    }

    BDBG_MSG(("BRAP_SRC_P_GetRequiredSrc: SRC carrying PCM data"));
    
    /* Get absolute SR values */
    ret = BRAP_P_ConvertSR(eInputSR,&uiInSr);
    ret |= BRAP_P_ConvertSR(eOutputSR,&uiOutSr);
    if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 
     
    if((0 == uiInSr)||(0 == uiOutSr))
    {
        BDBG_ERR(("Input/Output Sampling Rate Invalid"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(uiInSr >= uiOutSr)
    {
        if(2>(uiInSr / uiOutSr))
        {
            pSrcInfo->uiNumSrc = 1;
            
            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiOutSr,&pSrcInfo->eSrcType[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}

            ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}
        }
        else if((2== (uiInSr / uiOutSr)) && (0==(uiInSr % uiOutSr)))
        {
            pSrcInfo->uiNumSrc = 1;
            
            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiOutSr,&pSrcInfo->eSrcType[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}

            ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}
            
        }
        else if(((2== (uiInSr / uiOutSr)) && (0!=(uiInSr % uiOutSr)))||
                ((2<(uiInSr / uiOutSr)) && (4 > (uiInSr / uiOutSr))))
        {
            pSrcInfo->uiNumSrc = 2;

            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiInSr/2,&pSrcInfo->eSrcType[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 

            ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum((uiInSr/2),&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}

            ret = BRAP_SRC_P_GetSrcType(uiInSr/2, uiOutSr,&pSrcInfo->eSrcType[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 

            ret = BRAP_P_ConvertSrToEnum((uiInSr/2),&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}
            
        }
        else if((4 == (uiInSr / uiOutSr))&&(0==(uiInSr % uiOutSr)))
        {
            pSrcInfo->uiNumSrc = 1;

            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiOutSr,
                    &pSrcInfo->eSrcType[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}

            ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}
            
        }
        else if(((4 == (uiInSr / uiOutSr))&&(0!=(uiInSr % uiOutSr)))||
                (4 < (uiInSr / uiOutSr)))
        {
            pSrcInfo->uiNumSrc = 2;

            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiInSr/4,&pSrcInfo->eSrcType[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 

            ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum((uiInSr/4),&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}
            
            ret = BRAP_SRC_P_GetSrcType(uiInSr/4, uiOutSr,&pSrcInfo->eSrcType[1]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 

            ret = BRAP_P_ConvertSrToEnum((uiInSr/4),&pSrcInfo->eInSR[1]);
            ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[1]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}
            
        }
    }
    else
    {
        if(2>(uiOutSr / uiInSr))
        {
            pSrcInfo->uiNumSrc = 1;

            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiOutSr,&pSrcInfo->eSrcType[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}

            ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 
            
        }
        else if((2== (uiOutSr / uiInSr)) && (0==(uiOutSr % uiInSr)))
        {
            pSrcInfo->uiNumSrc = 1;

            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiOutSr,&pSrcInfo->eSrcType[0]);

            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}

            ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 
                
        }
        else if( ((2== (uiOutSr / uiInSr)) && (0!=(uiOutSr % uiInSr))) ||
                 ((2<(uiOutSr / uiInSr)) && (4 > (uiOutSr / uiInSr))))
        {
            pSrcInfo->uiNumSrc = 2;
            
            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiInSr*2,&pSrcInfo->eSrcType[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}

            ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum((uiInSr*2),&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 

            ret = BRAP_SRC_P_GetSrcType(uiInSr*2, uiOutSr,&pSrcInfo->eSrcType[1]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 

            ret = BRAP_P_ConvertSrToEnum((uiInSr*2),&pSrcInfo->eInSR[1]);
            ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[1]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 
            
        }
        else if((4 == (uiOutSr / uiInSr))&& (0==(uiOutSr % uiInSr)))
        {
            pSrcInfo->uiNumSrc = 1;

            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiOutSr,
                        &pSrcInfo->eSrcType[0]);
                if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}

                ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
                ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[0]);
                if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 
            
        }
        else if(((4 == (uiOutSr / uiInSr))&& (0!=(uiOutSr % uiInSr)))||
                 (4 < (uiOutSr / uiInSr)))
        {
            pSrcInfo->uiNumSrc = 2;
            ret = BRAP_SRC_P_GetSrcType(uiInSr, uiInSr*4,&pSrcInfo->eSrcType[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);}

            ret = BRAP_P_ConvertSrToEnum(uiInSr,&pSrcInfo->eInSR[0]);
            ret |= BRAP_P_ConvertSrToEnum((uiInSr*4),&pSrcInfo->eOutSR[0]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 

            ret = BRAP_SRC_P_GetSrcType(uiInSr*4, uiOutSr,&pSrcInfo->eSrcType[1]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 

            ret = BRAP_P_ConvertSrToEnum((uiInSr*4),&pSrcInfo->eInSR[1]);
            ret |= BRAP_P_ConvertSrToEnum(uiOutSr,&pSrcInfo->eOutSR[1]);
            if(ret != BERR_SUCCESS){return BERR_TRACE(ret);} 

        }
    }

    BDBG_MSG(("BRAP_SRC_P_GetRequiredSrc: pSrcInfo->uiNumSrc = %d",
                                                            pSrcInfo->uiNumSrc));
exit:
    BDBG_LEAVE(BRAP_SRC_P_GetRequiredSrc);
    return ret;
}
    
/***************************************************************************
Summary:
    Configures the SRC hardware before start.

Description:
    Before starting the SRC, programs the SRC hardware

Returns:
    BERR_SUCCESS on success

See Also:
    BRAP_SRC_P_Start
**************************************************************************/

static BERR_Code BRAP_SRC_P_HWConfig ( 
    BRAP_SRC_P_Handle   hSrc 		/* [in] SRC handle */
)
{
    BERR_Code           ret = BERR_SUCCESS;
    uint32_t            ui32RegVal = 0;
    BRAP_SRC_P_Params   sParams;
    BRAP_SRC_P_Type     eSrcType;
    unsigned int        uiDenScale = 0;
    unsigned int        uiInSr = 0;
    unsigned int        uiOutSr = 0;
    unsigned int        uiDivider = 1;    
    
    BDBG_ENTER(BRAP_SRC_P_HWConfig);
    BDBG_ASSERT(hSrc);
    
    sParams = hSrc->sParams;
    
    /* Program the stream configuration */
    ui32RegVal = BRAP_Read32 (hSrc->hRegister,
        hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
        (4 * hSrc->uiIndex)));

    ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi,RAMP_ENA))
                    |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi,MUTE_ENA))
                    |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi,STARTUP_RAMP_ENA)));

    if(sParams.bRampEnable)
    {
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,RAMP_ENA,1));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,MUTE_ENA,0));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,STARTUP_RAMP_ENA,1));
    }
#ifdef AUTOTRAP
    else
    {
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,RAMP_ENA,0));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,MUTE_ENA,0));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,STARTUP_RAMP_ENA,0));
    }
#endif /* AUTOTRAP */

    /* Program the User provided Group ID */
    if(BRAP_INVALID_VALUE != sParams.uiGroupId)
    {
        ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi,GROUP));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi, GROUP ,
                                       sParams.uiGroupId));
    }

    BRAP_Write32 (hSrc->hRegister,
            hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
            (4 * hSrc->uiIndex)),
            ui32RegVal);
    
#if (BRAP_3548_FAMILY == 1)
    ui32RegVal = BRAP_Read32 (hSrc->hRegister,
        hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
        (4 * hSrc->uiIndex)));

    ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi,FCI_ID));
    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,FCI_ID,sParams.ui32FciId));

    BRAP_Write32 (hSrc->hRegister,
            hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
            (4 * hSrc->uiIndex)),
            ui32RegVal);


    ui32RegVal = BRAP_Read32 (hSrc->hRegister,
        hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE +
        (4 * hSrc->uiIndex)));


    ui32RegVal &= ~((BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi,SRC_TYPE))
                    |(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi,WR_BANK_SEL))        
                    |(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi,SRC_ONLY))                            
                    |(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi,TM_BASE_ADDR)));
    

    if (BRAP_SRC_P_Type_eIIR == sParams.eSrcType)
    {
        ui32RegVal &= ~((BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi,IIR_NUM_CASCADE))
                        |(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi,IIR_STEP_SIZE)));        
    }

    /* If SRC type is passed by Applicaion */
    if (BRAP_SRC_P_Type_eUnknown != sParams.eSrcType)
    {
        eSrcType = sParams.eSrcType;
    }
    else
    {
        /* Else derive the SRC Type internally */
        /* Get absolute SR values */
        BRAP_P_ConvertSR(sParams.eInputSR,&uiInSr);
        BRAP_P_ConvertSR(sParams.eOutputSR,&uiOutSr);

        ret = BRAP_SRC_P_GetSrcType(uiInSr, uiOutSr, &eSrcType);
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("BRAP_SRC_P_HWConfig : could not get SRC type"));
            return ret;
        }
        
        /* Note: Do not modify the SRCType in sParams */
    }
    
    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi,SRC_TYPE,eSrcType));
    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi,TM_BASE_ADDR,(hSrc->uiBaseAddr+48)));
    ui32RegVal |= (BCHP_FIELD_ENUM(AUD_FMM_SRC_CTRL0_SRC_CFGi,SRC_ONLY,FIR));    
    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi,WR_BANK_SEL,0));        

    if (BRAP_SRC_P_Type_eIIR == eSrcType)
    {
        /* Program IIR_NUM_CASCADE as BRAP_P_MAX_IIR_FILTERS_IN_SRC. 0 is interpreted as 8 */
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi,IIR_NUM_CASCADE,0));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi,IIR_STEP_SIZE,0xA));        
    }
    
    BRAP_Write32(hSrc->hRegister,
        hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE +
        (4 * hSrc->uiIndex)), ui32RegVal);

   
    /* Program the Numerator, Denominator and Den_Scale.
       Numerator :  Output Sampling Rate.
       Denominator: Input Sampling Rate.
       Den_Scale:   (2^22)/Denominator.
       Numerator, Denominator and Den_Scale programming is valid only if SRC type is 
       Linear interpolation. For all other Cases, need not program it */

    

    if(BRAP_SRC_P_Type_eLinInt == eSrcType)
    {
        /* Get absolute SR values */
        BRAP_P_ConvertSR(sParams.eInputSR,&uiInSr);
        BRAP_P_ConvertSR(sParams.eOutputSR,&uiOutSr);
        BDBG_ERR (("The InputSR = %u and OutputSR = %u",uiInSr,uiOutSr));

        /* For better results scale down the samplerate values by a factor without
           the loss of information, as DEN_SCALE will have more precision */
        if((uiInSr % 100 == 0)&&(uiOutSr % 100 == 0))
        {
            uiDivider = 100;
        }
        else if((uiInSr % 10 == 0)&&(uiOutSr % 10 == 0))
        {
            uiDivider = 10;
        }
        else
        {
            uiDivider = 1;
        }
        uiInSr = uiInSr/uiDivider;
        uiOutSr = uiOutSr/uiDivider;
        uiDenScale = (1<<24)/uiOutSr;

        BRAP_Write32(hSrc->hRegister,
                        hSrc->uiBlkOffset +
                        BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + (hSrc->uiCoeffAddr*4),
                        uiInSr);
        
        BRAP_Write32(hSrc->hRegister,
                        hSrc->uiBlkOffset +
                        BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + ((hSrc->uiCoeffAddr+1)*4),
                        uiOutSr);

        BRAP_Write32(hSrc->hRegister,
                        hSrc->uiBlkOffset +
                        BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + ((hSrc->uiCoeffAddr+2)*4),
                        uiDenScale);

        /* Program the coefficient base address for Linear Interpolation */
        ui32RegVal = BRAP_Read32 (hSrc->hRegister,
            hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE +
            (4 * hSrc->uiIndex)));

        ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_CF_SELi,CF_BASE_ADDR0));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_CF_SELi,CF_BASE_ADDR0,(hSrc->uiCoeffAddr+24)));

        BRAP_Write32(hSrc->hRegister,
            hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE +
            (4 * hSrc->uiIndex)), ui32RegVal);
    }    
    else if ( (BRAP_SRC_P_Type_eDown4 == eSrcType) || (BRAP_SRC_P_Type_eUp4 == eSrcType) )
    {
        ui32RegVal = BRAP_Read32 (hSrc->hRegister,
            hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE +
            (4 * hSrc->uiIndex)));

        ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_CF_SELi,CF_BASE_ADDR0));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_CF_SELi,CF_BASE_ADDR0,24));

        BRAP_Write32(hSrc->hRegister,
            hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE +
            (4 * hSrc->uiIndex)), ui32RegVal);           
    }
    else if (BRAP_SRC_P_Type_eIIR == eSrcType)
    {
        ui32RegVal = BRAP_Read32 (hSrc->hRegister,
            hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE +
            (4 * hSrc->uiIndex)));

        ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_CF_SELi,CF_BASE_ADDR0));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_CF_SELi,CF_BASE_ADDR0,hSrc->uiCoeffAddr+24));

        ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_CF_SELi,CF_BASE_ADDR1));
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_CF_SELi,CF_BASE_ADDR1,
                                        (hSrc->uiCoeffAddr+24+(BRAP_P_MAX_IIR_FILTERS_IN_SRC*5))));        

        BRAP_Write32(hSrc->hRegister,
            hSrc->uiBlkOffset +(BCHP_AUD_FMM_SRC_CTRL0_CF_SELi_ARRAY_BASE +
            (4 * hSrc->uiIndex)), ui32RegVal);           
    }    
    
#else /* if not 3548 family */

    /* Program the FCI ID of the Input */
    switch (hSrc->uiIndex)
    {
        case 0:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP10);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP10,STREAM0));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP10,STREAM0,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP10,ui32RegVal);
            break;

        case 1:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP10);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP10,STREAM1));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP10,STREAM1,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP10,ui32RegVal);
            break;

        case 2:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP32);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP32,STREAM2));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP32,STREAM2,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP32,ui32RegVal);
            break;

        case 3:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP32);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP32,STREAM3));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP32,STREAM3,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP32,ui32RegVal);
            break;

        case 4:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP54);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP54,STREAM4));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP54,STREAM4,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP54,ui32RegVal);
            break;

        case 5:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP54);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP54,STREAM5));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP54,STREAM5,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP54,ui32RegVal);
            break;

        case 6:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP76);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP76,STREAM6));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP76,STREAM6,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP76,ui32RegVal);
            break;

        case 7:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP76);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP76,STREAM7));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP76,STREAM7,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP76,ui32RegVal);
            break;

        case 8:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP98);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP98,STREAM8));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP98,STREAM8,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP98,ui32RegVal);
            break;

        case 9:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP98);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAP98,STREAM9));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAP98,STREAM9,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAP98,ui32RegVal);
            break;

        case 10:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAPba);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAPba,STREAM10));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAPba,STREAM10,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAPba,ui32RegVal);
            break;

        case 11:
            ui32RegVal = BRAP_Read32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAPba);
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_IDMAPba,STREAM11));
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_IDMAPba,STREAM11,sParams.ui32FciId));
            BRAP_Write32 (hSrc->hRegister, hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_IDMAPba,ui32RegVal);
            break;

        default:
            BDBG_ASSERT (0);
    }

    /* Program the SRC Configuration */
    ui32RegVal = BRAP_Read32(hSrc->hRegister,
                                hSrc->uiBlkOffset+BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0+hSrc->uiSrcOffset);

    ui32RegVal &= ~((BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFG0,MI_SKEW))
                   |(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFG0,BASE_ADDR))
                   |(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFG0,TYPE)));

    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFG0,MI_SKEW,0));

    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFG0,BASE_ADDR,hSrc->uiBaseAddr));

    /* If SRC type is passed by Applicaion */
    if (BRAP_SRC_P_Type_eUnknown != sParams.eSrcType)
    {
        eSrcType = sParams.eSrcType;
    }
    else
    {
        /* Else derive the SRC Type internally */
        /* Get absolute SR values */
        BRAP_P_ConvertSR(sParams.eInputSR,&uiInSr);
        BRAP_P_ConvertSR(sParams.eOutputSR,&uiOutSr);

        ret = BRAP_SRC_P_GetSrcType(uiInSr, uiOutSr, &eSrcType);
        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("BRAP_SRC_P_HWConfig : could not get SRC type"));
            return ret;
        }
        
        /* Note: Do not modify the SRCType in sParams */
    }

    /* Program the SRC type */
    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFG0,TYPE,eSrcType));

    BRAP_Write32(hSrc->hRegister,
                    hSrc->uiBlkOffset+BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0+hSrc->uiSrcOffset,
                    ui32RegVal);

    /* Program the Numerator, Denominator and Den_Scale.
       Numerator :  Input Sampling Rate.
       Denominator: Output Sampling Rate.
       Den_Scale:   (2^22)/Denominator.
       Numerator, Denominator and Den_Scale programming is valid only if SRC type is 
       Linear interpolation. For all other Cases, need not program it 
        For example, to convert from 44.1 to 48 kHz, set NUMERATOR=44100, DENOMINATOR=48000
        , and DEN_SCALE=2^22/48000. Using 441, 480 and 2^22/480 also works and actually gives 
        better results because DEN_SCALE has more precision.
       */

    if(BRAP_SRC_P_Type_eLinInt == eSrcType)
    {
        /* Get absolute SR values */
        BRAP_P_ConvertSR(sParams.eInputSR,&uiInSr);
        BRAP_P_ConvertSR(sParams.eOutputSR,&uiOutSr);

        if((uiInSr % 100 == 0)&&(uiOutSr % 100 == 0))
        {
            uiDivider =100;
        }
        else if((uiInSr % 10 == 0)&&(uiOutSr % 10 == 0))
        {
            uiDivider =10;
        }
        else
        {
            uiDivider =1;
        }    

        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_NUM0,NUMERATOR,uiInSr/uiDivider)); /*For More accuracy divide uiInSr  by 100*/
        BRAP_Write32(hSrc->hRegister,
                        hSrc->uiBlkOffset+BCHP_AUD_FMM_SRC_CTRL0_LI_NUM0+hSrc->uiSrcOffset,
                        ui32RegVal);

        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_DEN0,DENOMINATOR,uiOutSr/uiDivider));/*For More accuracy divide uiOutSr  by 100*/
        BRAP_Write32(hSrc->hRegister,
                        hSrc->uiBlkOffset+BCHP_AUD_FMM_SRC_CTRL0_LI_DEN0+hSrc->uiSrcOffset,
                        ui32RegVal);
        uiDenScale = (0x400000)/(uiOutSr/uiDivider); /*(2^22)/(Den) = (2^22)/(uiOutSr/100)*//*For More accuracy divide uiOutSr  by 100*/


        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0,DEN_SCALE,uiDenScale));
        BRAP_Write32(hSrc->hRegister,
                        hSrc->uiBlkOffset+BCHP_AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0+hSrc->uiSrcOffset,
                        ui32RegVal);
    }
    else     if(BRAP_SRC_P_Type_eBypass== eSrcType)
    {
         ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_NUM0,NUMERATOR,0)); /*For More accuracy divide uiInSr  by 100*/
        BRAP_Write32(hSrc->hRegister,
                        hSrc->uiBlkOffset+BCHP_AUD_FMM_SRC_CTRL0_LI_NUM0+hSrc->uiSrcOffset,
                        ui32RegVal);

        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_DEN0,DENOMINATOR,0));/*For More accuracy divide uiOutSr  by 100*/
        BRAP_Write32(hSrc->hRegister,
                        hSrc->uiBlkOffset+BCHP_AUD_FMM_SRC_CTRL0_LI_DEN0+hSrc->uiSrcOffset,
                        ui32RegVal);


        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0,DEN_SCALE,0));
        BRAP_Write32(hSrc->hRegister,
                        hSrc->uiBlkOffset+BCHP_AUD_FMM_SRC_CTRL0_LI_DEN_SCALE0+hSrc->uiSrcOffset,
                        ui32RegVal);
    }

#endif /* BRAP_3548_FAMILY == 1 */

    BDBG_LEAVE(BRAP_SRC_P_HWConfig);
    return ret;
}

/***************************************************************************
Summary:
    Opens a SRC.

Description:
    Initializes the SRC and returns a SRC handle.The handle can then be 
    used for all other SRC function calls.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SRC_P_Close, BRAP_SRC_P_GetDefaultSettings.
**************************************************************************/
BERR_Code BRAP_SRC_P_Open (
    BRAP_FMM_P_Handle           hFmm,       /* [in] Parent FMM handle */
    BRAP_SRC_P_Handle *         phSrc,      /* [out] Pointer to SRC handle */
    unsigned int                uiSrcIndex, /* [in] SRC index */
    unsigned int                uiSrcBlk,   /* [in] SRC block index */
    const BRAP_SRC_P_Settings  *pSettings   /* [in] SRC settings */
)
{
    BERR_Code           ret = BERR_SUCCESS;
    BRAP_SRC_P_Handle   hSrc = NULL;
    uint32_t            ui32RegVal = 0;
    bool                bWdgRecovery = false;

    BDBG_ENTER (BRAP_SRC_P_Open);
    BDBG_MSG (("BRAP_SRC_P_Open:"
               "hFmm=0x%x, uiSrcIndex=%d, uiSrcBlk=%d"
               "No settings",
               hFmm, uiSrcIndex,uiSrcBlk));

    /* Check all input parameters to the function. Return error if
     * - FMM handle is NULL
     * - Given index exceeds maximum no. of SRCs
     * - Pointer to Settings structure is NULL
     * - Pointer to SRC handle is NULL     */
    BDBG_ASSERT (hFmm);
    BDBG_ASSERT (phSrc);
    
    bWdgRecovery = BRAP_FMM_P_GetWatchdogRecoveryFlag(hFmm);
    
    if (false == bWdgRecovery)
    {   /* If not in WatchDog recovery */    
        BDBG_ASSERT (pSettings);
    }
    BSTD_UNUSED(pSettings);
    
    if (uiSrcIndex >= BRAP_RM_P_MAX_SRC_PER_SRC_BLCK)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (uiSrcBlk >= BRAP_RM_P_MAX_SRC_BLCK)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    if (false == bWdgRecovery)
    {   
        /* If not in WatchDog recovery */  
        if (hFmm->hSrc[uiSrcBlk][uiSrcIndex] != NULL )
        {
            BDBG_MSG(("BRAP_SRC_P_Open: SRC %d is already open", uiSrcIndex));
            hFmm->hSrc[uiSrcBlk][uiSrcIndex]->uiSrcOpenCount++;
            hSrc = hFmm->hSrc[uiSrcBlk][uiSrcIndex];
            *phSrc = hSrc;
            return BERR_SUCCESS;
        }
    
        /* Allocate memory for the SRC handle. Fill in parameters in the SRC handle. */
    
        /* Allocate Src Channel handle */
        hSrc = (BRAP_SRC_P_Handle) BKNI_Malloc (sizeof(BRAP_SRC_P_Object));
        if (hSrc == NULL)
        {
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }
        
        /* Clear the handle memory */
        BKNI_Memset (hSrc, 0, sizeof(BRAP_SRC_P_Object));

        /* Initialise known elements in Source Channel handle */
        hSrc->hChip = hFmm->hChip;
        hSrc->hRegister = hFmm->hRegister;
        hSrc->hHeap = hFmm->hHeap;
        hSrc->hInt = hFmm->hInt;
        hSrc->hFmm = hFmm;
        hSrc->uiIndex = uiSrcIndex;
        hSrc->uiBlkId = uiSrcBlk;
        hSrc->uiCoeffAddr = BRAP_INVALID_VALUE;        
#if (BRAP_3548_FAMILY == 1)        
    hSrc->uiBlkOffset = (BCHP_AUD_FMM_SRC_CTRL1_RAMP_STEP - BCHP_AUD_FMM_SRC_CTRL0_RAMP_STEP) * uiSrcBlk;
    hSrc->uiSrcOffset = 4 * uiSrcIndex;
#else /* if not 3548 family */
#if ( BRAP_RM_P_MAX_SRC_BLCK > 1 )
        hSrc->uiBlkOffset = (BCHP_AUD_FMM_SRC_CTRL1_STRM_ENA - BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA) * uiSrcBlk;
#else
        hSrc->uiBlkOffset = 0;
#endif
        hSrc->uiSrcOffset = (BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG1 - BCHP_AUD_FMM_SRC_CTRL0_SRC_CFG0)* uiSrcIndex;
#endif /* (BRAP_3548_FAMILY == 1) */


        /*Program the Group Id Equal to the SCR ID*/        
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,GROUP,uiSrcIndex));
        BRAP_Write32 (hSrc->hRegister,
                      hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
                      (4 * uiSrcIndex),
                      ui32RegVal);
    }   /* End: If not in WatchDog recovery */
    else
    {
        hSrc = *phSrc;
    }

    if (false == bWdgRecovery)
    {   
        hSrc->eState = BRAP_P_State_eOpened;
        /* If not in WatchDog recovery */ 
        /* Increment the open Count */
        hSrc->uiSrcOpenCount++;

        /* Store SRC handle inside the FMM handle */
        hFmm->hSrc[uiSrcBlk][uiSrcIndex] = hSrc;

        *phSrc = hSrc;
    }   /* End: If not in WatchDog recovery */        

    BDBG_LEAVE (BRAP_SRC_P_Open);
    return ret;
}

/***************************************************************************
Summary:
    Releases all the resources associated with this SRC and frees the handles.

Description:
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SRC_P_Open
**************************************************************************/
BERR_Code BRAP_SRC_P_Close ( 
    BRAP_SRC_P_Handle hSrc      /* [in] SRC Handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t ui32RegVal = 0;    

    BDBG_ENTER (BRAP_SRC_P_Close);
    BDBG_ASSERT (hSrc);
    
    BDBG_MSG (("BRAP_SRC_P_Close(): hSrc=0x%x, Index=%d ", 
                hSrc, hSrc->uiIndex));
    
    /* Reduce the Open Count*/
    hSrc->uiSrcOpenCount--;

    if(0 == hSrc->uiSrcOpenCount)
    {
        /* Program the Group Id Equal to the SCR ID */        
        ui32RegVal = 0;
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,GROUP, hSrc->uiIndex));
        BRAP_Write32 (hSrc->hRegister,
                      hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
                      (4 * hSrc->uiIndex),
                      ui32RegVal);

        /* Remove referrence to this SRC from the parent FMM */ 
        hSrc->hFmm->hSrc[hSrc->uiBlkId][hSrc->uiIndex] = NULL;
    
        /* Free the Source Channel Handle memory*/
        BKNI_Free (hSrc); 
    }
                
    BDBG_LEAVE (BRAP_SRC_P_Close);
    return ret;
}

/***************************************************************************
Summary:
    Starts a SRC.

Description:
    To start the SRC operation this function must be called. SRC will 
    start based on the existing settings. 

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SRC_P_Stop.
**************************************************************************/
BERR_Code BRAP_SRC_P_Start ( 
    BRAP_SRC_P_Handle           hSrc,       /* [in] SRC Handle */
    const BRAP_SRC_P_Params *   pParams     /* [in] Pointer to start
                                              parameters for this SRC */ 
)
{
    BERR_Code           ret = BERR_SUCCESS;
    bool                bWdgRecovery = false;

    BDBG_ENTER (BRAP_SRC_P_Start);
    BDBG_ASSERT (hSrc);

    bWdgRecovery = BRAP_FMM_P_GetWatchdogRecoveryFlag (hSrc->hFmm);

    if (false == bWdgRecovery)
    {   /* If not in WatchDog recovery */  
    
        BDBG_ASSERT (pParams);

        BDBG_MSG (("BRAP_SRC_P_Start(): hSrc=0x%x, Index=%d, Block=%d,"
                "sParams.bPriorityHigh=%d, "
                "sParams.bRampEnable=%d, "
                "sParams.eInputSR=%d, "
                "sParams.eOutputSR=%d, "
                "sParams.eSrcType=%d, "
                "sParams.uiGroupId=%d, "
                "sParams.ui32FciId=0x%x",
                hSrc, hSrc->uiIndex,hSrc->uiBlkId,
                pParams->bPriorityHigh,
                pParams->bRampEnable,
                pParams->eInputSR, 
                pParams->eOutputSR,
                pParams->eSrcType,
                pParams->uiGroupId,
                pParams->ui32FciId));    

    }   /* End: If not in WatchDog recovery */

    if ((false == bWdgRecovery))
    {
    if (BRAP_P_State_eStarted != hSrc->eState)
    {
#ifdef AUTOTRAP
        /* Disable ramping for Autotrap */
        hSrc->sParams.bRampEnable = false;
#endif /* AUTOTRAP */

        BDBG_MSG(("hSrc->eState = %d", hSrc->eState));
        /* Store the start parameters inside the handle */
        hSrc->sParams = *pParams;


        /* Allocate SRC SRAM */    
        ret = BRAP_SRC_P_AllocateBaseAddr(hSrc, 
                                      hSrc->sParams.eSrcType, 
                                      &(hSrc->uiBaseAddr));
        if(ret!=BERR_SUCCESS)
        {return BERR_TRACE(ret);}

#if (BRAP_3548_FAMILY ==1)
        if((BRAP_SRC_P_Type_eLinInt == hSrc->sParams.eSrcType)
#if BRAP_P_EQUALIZER
           ||(BRAP_SRC_P_Type_eIIR == hSrc->sParams.eSrcType)
#endif
          )
        {
            if(BRAP_INVALID_VALUE == hSrc->uiCoeffAddr)
            {        
                ret = BRAP_SRC_P_AllocateCoeffAddr(hSrc);
                if(BERR_SUCCESS != ret)
                {
                    BDBG_ERR(("BRAP_SRC_P_Start: BRAP_SRC_P_AllocateCoeffAddr returned error"));
                    return ret;
                }
            }
        }
#endif  
        /* Configure SRC Hardware */
        BRAP_SRC_P_HWConfig (hSrc); 

        /* Enable the SRC */
        BRAP_SRC_P_EnableDisable(hSrc,1);

        /* Change SRC state */
        hSrc->eState = BRAP_P_State_eStarted;
        hSrc->uiSrcStartCount++;
        BDBG_MSG(("hSrc->uiSrcStartCount = %d", hSrc->uiSrcStartCount));
    }
    else
    {
        /* Start can happen only after the Stopped state */
        BDBG_MSG (("BRAP_SRC_P_Start(): Start can happen only after the" 
                   " Stopped state. Current state = %d. "
                   "Incrementing The Start Count.", hSrc->eState));
        hSrc->uiSrcStartCount++;
            BDBG_MSG(("hSrc->uiSrcStartCount = %d", hSrc->uiSrcStartCount));

        }
    }
    else
    {
        BDBG_MSG(("hSrc->eState = %d", hSrc->eState));
        /* Configure SRC Hardware */
        BRAP_SRC_P_HWConfig (hSrc);         
        
        /* Enable the SRC */
        BRAP_SRC_P_EnableDisable(hSrc,1);        
        BDBG_MSG(("hSrc->uiSrcStartCount = %d", hSrc->uiSrcStartCount));
    }    
    
    BDBG_LEAVE (BRAP_SRC_P_Start);
    return ret;
}

/***************************************************************************
Summary:
    Stops a SRC.

Description:
  
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SRC_P_Start.
**************************************************************************/
BERR_Code BRAP_SRC_P_Stop ( 
    BRAP_SRC_P_Handle     hSrc        /* [in] SRC Handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t ui32RegVal = 0;
    
    BDBG_ENTER(BRAP_SRC_P_Stop);
    BDBG_ASSERT (hSrc);
    BDBG_MSG (("BRAP_SRC_P_Stop(): hSrc=0x%x, Index=%d ", hSrc, hSrc->uiIndex));
    
    if (BRAP_P_State_eStarted == hSrc->eState)
    {
        BDBG_MSG(("hSrc->eState STARTState = %d", hSrc->eState));
        BDBG_MSG(("Before decrement :: hSrc->uiSrcStartCount = %d", hSrc->uiSrcStartCount));
        hSrc->uiSrcStartCount--;
        
        if (0 == hSrc->uiSrcStartCount)
        {           
            BDBG_MSG((" hSrc->uiSrcStartCount = %d", hSrc->uiSrcStartCount));
            /*  Disable the SRC */
            BRAP_SRC_P_EnableDisable(hSrc,0);

            /* Program the Group Id Equal to the SCR ID */        
            ui32RegVal = 0;
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,GROUP, 
                                hSrc->uiIndex));
            BRAP_Write32 (hSrc->hRegister,
                          hSrc->uiBlkOffset + 
                          BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE +
                          (4 * hSrc->uiIndex),
                          ui32RegVal);

#if (BRAP_3548_FAMILY ==1)
            if((BRAP_SRC_P_Type_eLinInt == hSrc->sParams.eSrcType)
#if BRAP_P_EQUALIZER
               ||(BRAP_SRC_P_Type_eIIR == hSrc->sParams.eSrcType)
#endif
              )
            {
                if(BRAP_INVALID_VALUE != hSrc->uiCoeffAddr)
                {
                    ret = BRAP_SRC_P_DeallocateCoeffAddr (hSrc);
                    if(ret!=BERR_SUCCESS)
                    {
                        BDBG_ERR(("BRAP_SRC_P_Stop(): BRAP_SRC_P_DeallocateCoeffAddr failed!!!"));
                        return ret;
                    }
                }
            }
#endif
            ret = BRAP_SRC_P_DeallocateBaseAddr(hSrc);
            if(ret!=BERR_SUCCESS)
                {
                BDBG_ERR(("BRAP_SRC_P_Stop(): BRAP_SRC_P_DeallocateBaseAddr failed!!!"));
                    ret = BERR_TRACE(ret);
                }
            
            /* Change Software state */
            hSrc->eState = BRAP_SRCCH_P_State_eStopped;
        }
        else
        {
            BDBG_MSG(("Non-Zero hSrc->uiSrcStartCount = %d", hSrc->uiSrcStartCount));
        }
    }
    else
    {
        /* SRC was not running */
        BDBG_WRN (("BRAP_SRC_P_Stop(): This SRC was not running. Ignoring this fnc call"));
    }

    BDBG_LEAVE (BRAP_SRC_P_Stop);
    return ret;
}

/***************************************************************************
Summary:
    Enables/Disables a SRC.

Description:
    This function is called to program the Enable bit for a particular
    SRC.
    
Returns:
    BERR_SUCCESS on success
    
See Also:
    BRAP_SRC_P_Start,BRAP_SRC_P_STOP.
**************************************************************************/
static BERR_Code BRAP_SRC_P_EnableDisable(
            BRAP_SRC_P_Handle   hSrc,       /* [in] SRC Handle */
            unsigned int        uiEnable    /* [in] Enable/Disable */
)
{
    BERR_Code       ret = BERR_SUCCESS;
    uint32_t        ui32RegVal = 0;
    uint32_t        ui32Offset = 0;
    BREG_Handle     hRegister = NULL;

    BDBG_ENTER(BRAP_SRC_P_EnableDisable);
    BDBG_ASSERT(hSrc);
    
    hRegister = hSrc->hRegister;
    ui32Offset = hSrc->uiSrcOffset;

#if (BRAP_3548_FAMILY == 1)

    ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + hSrc->uiSrcOffset + hSrc->uiBlkOffset);
    ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi,PRIORITY))
                    |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_CFGi,ENABLE)));
    if (true == hSrc->sParams.bPriorityHigh)
    {
        ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,PRIORITY,1));
    }
    ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_CFGi,ENABLE,uiEnable));
    BRAP_Write32 (hRegister, BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_BASE + hSrc->uiSrcOffset + hSrc->uiBlkOffset,ui32RegVal);

#else /* if not 3548 family */
    /* Enable the SRC */
    ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA + hSrc->uiBlkOffset);

    switch (hSrc->uiIndex)
    {
        case 0:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY0))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA0)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY0,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA0,uiEnable));
            break;
            
        case 1:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY1))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA1)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY1,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA1,uiEnable));
            break;

        case 2:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY2))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA2)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY2,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA2,uiEnable));
            break;

        case 3:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY3))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA3)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY3,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA3,uiEnable));
            break;

        case 4:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY4))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA4)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY4,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA4,uiEnable));
            break;

        case 5:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY5))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA5)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY5,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA5,uiEnable));
            break;

        case 6:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY6))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA6)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY6,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA6,uiEnable));
            break;

        case 7:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY7))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA7)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY7,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA7,uiEnable));
            break;

        case 8:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY8))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA8)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY8,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA8,uiEnable));
            break;

        case 9:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY9))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA9)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY9,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA9,uiEnable));
            break;

        case 10:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY10))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA10)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY10,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA10,uiEnable));
            break;

        case 11:
            ui32RegVal &= ~( (BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY11))
                            |(BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA11)));
            if (true == hSrc->sParams.bPriorityHigh)
            {
                ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,PRIORITY11,1));
            }
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_STRM_ENA,ENA11,uiEnable));
            break;

        default:
            BDBG_ASSERT (0);
    }

    BRAP_Write32 (hRegister, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA + hSrc->uiBlkOffset,ui32RegVal);
#endif /* BRAP_3548_FAMILY == 1 */

    BDBG_LEAVE(BRAP_SRC_P_EnableDisable);
    return ret;
}

/***************************************************************************
Summary:
    Allocates the Base address in SRC SRAM.

Description:
    Allocates the base address for the SRC in SRC internal SRAM.
    Base Address specifies the start address of this stream's temporary buffer 
    for storing samples when doing sample rate conversion (i.e. TYPE is 
    something other than Bypass).

    Note: This function doesn't do an actual allocation. It just returns the 
    base address from the already allocated SRAM memory in the hardware.

    SRC Block 0 has a total of 960 (0x3c0) words available for temporary buffers
    and SRC Block 1 has 1600 (0x640).

    The number of words needed by each stream of a given SRC type is:

    Up2     - 128 words
    Up4     - 128 words
    Down2   - 256 words
    Down4   - 512 words
    L_Int   - 4 words   

    The Memory limitation Equation looks like:
    SRC0: N_Down4*512+N_Down2*256+Upx*128+64(for max 16 L_Int) <= 960
    SRC1: N_Down4*512+N_Down2*256+Upx*128+64(for max 16 L_Int) <= 1600
    
    As an example, suppose streams 3 and 4 have TYPE=Down4, stream 5 has 
    TYPE=Down2, and stream 7 has TYPE=Up2. The following settings would be 
    appropriate.

    Stream 3: BASE_ADDR = 0x000 (0x000-0x1ff)
    Stream 4: BASE_ADDR = 0x200 (0x200-0x3ff)
    Stream 5: BASE_ADDR = 0x400 (0x400-0x4ff)
    Stream 7: BASE_ADDR = 0x500 (0x500-0x57f
    
Returns:
    BERR_SUCCESS on success else error
    
See Also:
    BRAP_SRC_P_DeallocateBaseAddr
**************************************************************************/
BERR_Code BRAP_SRC_P_AllocateBaseAddr (
            BRAP_SRC_P_Handle   hSrc,           /* [in] SRC Handle */
            BRAP_SRC_P_Type     eSrcType,       /* [in] SRC Type */
            unsigned int        *puiBaseAddr    /* [out] Base Address */
    )
{
    BERR_Code                       ret = BERR_SUCCESS;
    BRAP_SRC_P_SramMemLocationInfo  *pMemLocInfo = NULL;
    unsigned int                    i = 0;
    bool                            bMemLocAllocated = false;
    unsigned int                    uiSizeOfArray = 0;
        
    BDBG_ENTER(BRAP_SRC_P_AllocateBaseAddr);
    BDBG_ASSERT(hSrc);
    BDBG_ASSERT(puiBaseAddr);

    /* Check for the SRC Block */
    if(hSrc->uiBlkId >= BRAP_RM_P_MAX_SRC_BLCK)
    {
        BDBG_ERR(("BRAP_SRC_P_AllocateBaseAddr: Invalid SRC Block ID=%d",
                    hSrc->uiBlkId));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Get the SRAM Location Info stored in hFmm */
    pMemLocInfo = hSrc->hFmm->pSrcMemLocationInfo[hSrc->uiBlkId];

    /* Get the size of array */
    if (0 == hSrc->uiBlkId)
    {
        uiSizeOfArray =
            (BRAP_P_MAX_SRAM_MEMORY_SRCBLK_0/BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            +((BRAP_P_MAX_SRAM_MEMORY_SRCBLK_0%BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            /BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT);
    }
    else if (1 == hSrc->uiBlkId)
    {
        uiSizeOfArray =
            (BRAP_P_MAX_SRAM_MEMORY_SRCBLK_1/BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            +((BRAP_P_MAX_SRAM_MEMORY_SRCBLK_1%BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            /BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT);
    }

    /* Switch on SRC Type */
    switch (eSrcType)
    {
        case BRAP_SRC_P_Type_eUp2:
        case BRAP_SRC_P_Type_eUp4:
            /* Trace the array to find the free location */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if((false == pMemLocInfo[i].bAllocated ) && 
                   (BRAP_P_SRAM_MEMORY_REQUIRED_UPx == pMemLocInfo[i].uiSize ))
                {
                    /* If a block of 128 words free, Allocate it and set the 
                       flag accordingly */
                    *puiBaseAddr = pMemLocInfo[i].uiBaseAddr;
                    pMemLocInfo[i].bAllocated = true;
                    bMemLocAllocated = true;
                    break;
                }
            }

            /* if no location is available, return Error */
            if (false == bMemLocAllocated)
            {
                BDBG_ERR(("BRAP_SRC_P_AllocateBaseAddr: No memory location Available"));
                goto end_error;
            }
            break;

        case BRAP_SRC_P_Type_eDown2:
            /* Trace the array to find contiguous 256 word free location */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if((false == pMemLocInfo[i].bAllocated ) && 
                   (BRAP_P_SRAM_MEMORY_REQUIRED_UPx == pMemLocInfo[i].uiSize)&&
                   ((i+1) < uiSizeOfArray) &&
                   (false == pMemLocInfo[i+1].bAllocated ) && 
                   (BRAP_P_SRAM_MEMORY_REQUIRED_UPx==pMemLocInfo[i+1].uiSize))
                {
                    /* If two blocks of 128 words free, Allocate it and set the 
                       flag accordingly */
                    *puiBaseAddr = pMemLocInfo[i].uiBaseAddr;
                    pMemLocInfo[i].bAllocated = true;
                    pMemLocInfo[i+1].bAllocated = true;
                    bMemLocAllocated = true;
                    break;
                }
            }

            /* if no location is available, return Error */
            if (false == bMemLocAllocated)
            {
                BDBG_ERR(("BRAP_SRC_P_AllocateBaseAddr: No memory location Available"));
                goto end_error;
            }

            break;    

        case BRAP_SRC_P_Type_eDown4:
            /* Trace the array to find the free location */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if(( false == pMemLocInfo[i].bAllocated ) && 
                   (BRAP_P_SRAM_MEMORY_REQUIRED_UPx==pMemLocInfo[i].uiSize)&&
                   ( (i+1) < uiSizeOfArray) &&
                   (false == pMemLocInfo[i+1].bAllocated ) && 
                   (BRAP_P_SRAM_MEMORY_REQUIRED_UPx==pMemLocInfo[i+1].uiSize)&&
                   ( (i+2) < uiSizeOfArray) &&
                   (false == pMemLocInfo[i+2].bAllocated ) && 
                   (BRAP_P_SRAM_MEMORY_REQUIRED_UPx==pMemLocInfo[i+2].uiSize)&&
                   ( (i+3) < uiSizeOfArray) &&
                   (false == pMemLocInfo[i+3].bAllocated ) && 
                   (BRAP_P_SRAM_MEMORY_REQUIRED_UPx==pMemLocInfo[i+3].uiSize))
                {
                    /* If four blocks of 128 words free, Allocate it and set the 
                       flag accordingly */
                    *puiBaseAddr = pMemLocInfo[i].uiBaseAddr;
                    pMemLocInfo[i].bAllocated = true;
                    pMemLocInfo[i+1].bAllocated = true;
                    pMemLocInfo[i+2].bAllocated = true;
                    pMemLocInfo[i+3].bAllocated = true;
                    bMemLocAllocated = true;
                    break;
                }
            }

            /* if no location is available, return Error */
            if (false == bMemLocAllocated)
            {
                BDBG_ERR(("BRAP_SRC_P_AllocateBaseAddr: No memory location Available"));
                goto end_error;
            }

            break;

        case BRAP_SRC_P_Type_eBypass:
        case BRAP_SRC_P_Type_eLinInt:
            /* Trace the array to find the free location */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if(( false == pMemLocInfo[i].bAllocated ) && 
                  (BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT==pMemLocInfo[i].uiSize))
                {
                    /* If a block of 4 words free, Allocate it and set the 
                       flag accordingly */
                    *puiBaseAddr = pMemLocInfo[i].uiBaseAddr;
                    pMemLocInfo[i].bAllocated = true;
                    bMemLocAllocated = true;
                    break;
                }
            }

            /* if no location is available, return Error */
            if (false == bMemLocAllocated)
            {
                BDBG_ERR(("BRAP_SRC_P_AllocateBaseAddr: No memory location Available"));
                goto end_error;
            }
            
            break;
#if (BRAP_3548_FAMILY == 1)
        case BRAP_SRC_P_Type_eIIR:
            /* Trace the array to find the free location */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if(( false == pMemLocInfo[i].bAllocated ) && 
                  (BRAP_P_SRAM_MEMORY_REQUIRED_UPx==pMemLocInfo[i].uiSize))
                {
                    /* If a block of 35 words free, Allocate it and set the 
                       flag accordingly */
                    *puiBaseAddr = pMemLocInfo[i].uiBaseAddr;
                    pMemLocInfo[i].bAllocated = true;
                    bMemLocAllocated = true;
                    break;
                }
            }

            /* if no location is available, return Error */
            if (false == bMemLocAllocated)
            {
                BDBG_ERR(("BRAP_SRC_P_AllocateBaseAddr: No memory location Available"));
                goto end_error;
            }
            break;
#endif
        default:
            BDBG_ERR(("BRAP_SRC_P_AllocateBaseAddr: Unsupported SRC Type = %d",
                eSrcType));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    goto end;
    
end_error:
    /* Dump the Array */
    ret = BERR_OUT_OF_DEVICE_MEMORY;
    
end:
    BDBG_MSG(("BRAP_SRC_P_AllocateBaseAddr: *puiBaseAddr = %d", *puiBaseAddr));
    BDBG_LEAVE(BRAP_SRC_P_AllocateBaseAddr);
    return ret;
}

/***************************************************************************
Summary:
    Deallocates the Base address in SRC SRAM.

Description:
    Deallocates the base address for the SRC in SRC internal SRAM.

    Note: This function doesn't do an actual deallocation. It just tags the 
    base address in the SRAM memory to be free.
        
Returns:
    BERR_SUCCESS on success else error
    
See Also:
    BRAP_SRC_P_AllocateBaseAddr
**************************************************************************/
BERR_Code BRAP_SRC_P_DeallocateBaseAddr (
            BRAP_SRC_P_Handle   hSrc    /* [in] SRC Handle */
    )
{
    BERR_Code                       ret = BERR_SUCCESS;
    BRAP_SRC_P_SramMemLocationInfo  *pMemLocInfo = NULL;
    unsigned int                    i = 0;
    unsigned int                    uiSizeOfArray = 0;
        
    BDBG_ENTER(BRAP_SRC_P_DeallocateBaseAddr);
    BDBG_ASSERT(hSrc);

    /* Check for the SRC Block */
    if(hSrc->uiBlkId >= BRAP_RM_P_MAX_SRC_BLCK)
    {
        BDBG_ERR(("BRAP_SRC_P_DeallocateBaseAddr: Invalid SRC Block ID=%d",
                    hSrc->uiBlkId));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Get the SRAM Location Info stored in hFmm */
    pMemLocInfo = hSrc->hFmm->pSrcMemLocationInfo[hSrc->uiBlkId];

    /* Get the size of array */
    if (0 == hSrc->uiBlkId)
    {
        uiSizeOfArray =
            (BRAP_P_MAX_SRAM_MEMORY_SRCBLK_0/BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            +((BRAP_P_MAX_SRAM_MEMORY_SRCBLK_0%BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            /BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT);
    }
    else if (1 == hSrc->uiBlkId)
    {
        uiSizeOfArray =
            (BRAP_P_MAX_SRAM_MEMORY_SRCBLK_1/BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            +((BRAP_P_MAX_SRAM_MEMORY_SRCBLK_1%BRAP_P_SRAM_MEMORY_REQUIRED_UPx)
            /BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT);
    }

    /* Switch on SRC Type */
    switch (hSrc->sParams.eSrcType)
    {
        case BRAP_SRC_P_Type_eUp2:
        case BRAP_SRC_P_Type_eUp4:
            /* Trace the array to find the location for this base address and
               reset the flag */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if( hSrc->uiBaseAddr == pMemLocInfo[i].uiBaseAddr )
                {
                    BDBG_MSG(("Up4 hSrc->uiBaseAddr = %d found at pMemLocInfo[i=%d]"
                        ,hSrc->uiBaseAddr,i));
                    pMemLocInfo[i].bAllocated = false;
                    break;
                }
            }

            break;

        case BRAP_SRC_P_Type_eDown2:
            /* Trace the array to find the location for this base address and
               reset the flag */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if( hSrc->uiBaseAddr == pMemLocInfo[i].uiBaseAddr )
                {
                    pMemLocInfo[i].bAllocated = false;
                    pMemLocInfo[i+1].bAllocated = false;
                    BDBG_MSG(("Dn2 hSrc->uiBaseAddr = %d found at pMemLocInfo[i=%d]"
                    ,hSrc->uiBaseAddr,i));
                    break;
                }
            }
            
            break;    

        case BRAP_SRC_P_Type_eDown4:
            /* Trace the array to find the location for this base address and
               reset the flag */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if( hSrc->uiBaseAddr == pMemLocInfo[i].uiBaseAddr )
                {
                    pMemLocInfo[i].bAllocated = false;
                    pMemLocInfo[i+1].bAllocated = false;
                    pMemLocInfo[i+2].bAllocated = false;
                    pMemLocInfo[i+3].bAllocated = false;
                    BDBG_MSG(("Dn4 hSrc->uiBaseAddr = %d found at pMemLocInfo[i=%d]"
                    ,hSrc->uiBaseAddr,i));
                    
                    break;
                }
            }

            break;

        case BRAP_SRC_P_Type_eBypass:
        case BRAP_SRC_P_Type_eLinInt:
            /* Trace the array to find the location for this base address and
               reset the flag */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if( hSrc->uiBaseAddr == pMemLocInfo[i].uiBaseAddr )
                {
                    BDBG_MSG(("ByPass hSrc->uiBaseAddr = %d found at pMemLocInfo[i=%d]"
                    ,hSrc->uiBaseAddr,i));

                    pMemLocInfo[i].bAllocated = false;
                    break;
                }
            }

            break;
#if (BRAP_3548_FAMILY == 1)
        case BRAP_SRC_P_Type_eIIR:
            /* Trace the array to find the location for this base address and
               reset the flag */
            for (i=0;i<uiSizeOfArray;i++)
            {
                if( hSrc->uiBaseAddr == pMemLocInfo[i].uiBaseAddr )
                {
                    BDBG_MSG(("Up4 hSrc->uiBaseAddr = %d found at pMemLocInfo[i=%d]"
                        ,hSrc->uiBaseAddr,i));
                    pMemLocInfo[i].bAllocated = false;
                    break;
                }
            }
            break;
#endif
        default:
            BDBG_ERR(("BRAP_SRC_P_DeallocateBaseAddr: Unsupported SRC Type"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    BDBG_LEAVE(BRAP_SRC_P_DeallocateBaseAddr);
    return ret;
}

#if (BRAP_3548_FAMILY == 1)
BERR_Code BRAP_SRC_P_SetConfig_isr (
            BRAP_SRC_P_Handle   hSrc,           /* [in] SRC Handle */
            BRAP_SRC_P_ConfigParams *psSRCConfigParams  /* [in] Config Params */
    )
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int i;
    uint32_t   ui32RegVal = 0;
#if BRAP_P_EQUALIZER    
    uint32_t   ui32WrBankSel=0, ui32WrBankOffset=0;
#endif

    BDBG_ENTER (BRAP_SRC_P_SetConfig_isr);
    BDBG_ASSERT (hSrc);
    
    if(hSrc->eState != BRAP_P_State_eStarted)
    {
        BDBG_ERR(("SRC is not in running state. Cannot program the coefficients."));
        ret = BERR_NOT_SUPPORTED;
        goto exit;        
    }
    
    switch (hSrc->sParams.eSrcType)
    {
#if BRAP_P_EQUALIZER
        case BRAP_SRC_P_Type_eIIR:
            
            /* Program the coefficients considering the WR_BANK_SEL value */
            ui32RegVal = BRAP_Read32(hSrc->hRegister, hSrc->uiBlkOffset+
                                       (BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE + (4 * hSrc->uiIndex)));

            ui32WrBankSel = BCHP_GET_FIELD_DATA(ui32RegVal, AUD_FMM_SRC_CTRL0_SRC_CFGi, WR_BANK_SEL);
            ui32WrBankSel = ~(ui32WrBankSel);
            ui32WrBankSel = ui32WrBankSel & 1;

            ui32WrBankOffset = ui32WrBankSel * BRAP_P_MAX_IIR_FILTERS_IN_SRC*20;
            for (i=0; i<BRAP_P_MAX_IIR_FILTERS_IN_SRC; i++)
            {
                BDBG_MSG(("BRAP_SRC_P_SetConfig_isr: SRCConfigParams->uCoeff.sIIRCoeff"));
                BDBG_MSG(("b0 = %x",psSRCConfigParams->uCoeff.sIIRCoeff.b0[i]));
                BDBG_MSG(("b1 = %x",psSRCConfigParams->uCoeff.sIIRCoeff.b1[i]));
                BDBG_MSG(("b2 = %x",psSRCConfigParams->uCoeff.sIIRCoeff.b2[i]));
                BDBG_MSG(("a1 = %x",psSRCConfigParams->uCoeff.sIIRCoeff.a1[i]));
                BDBG_MSG(("a2 = %x",psSRCConfigParams->uCoeff.sIIRCoeff.a2[i]));

                BRAP_Write32(hSrc->hRegister, 
                    hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + ((hSrc->uiCoeffAddr+0)*4) + (i*20) + ui32WrBankOffset, 
                    psSRCConfigParams->uCoeff.sIIRCoeff.b0[i]);
                BRAP_Write32(hSrc->hRegister, 
                    hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + ((hSrc->uiCoeffAddr+1)*4) + (i*20) + ui32WrBankOffset, 
                    psSRCConfigParams->uCoeff.sIIRCoeff.b1[i]);
                BRAP_Write32(hSrc->hRegister, 
                    hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + ((hSrc->uiCoeffAddr+2)*4) + (i*20) + ui32WrBankOffset, 
                    psSRCConfigParams->uCoeff.sIIRCoeff.b2[i]);
                BRAP_Write32(hSrc->hRegister, 
                    hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + ((hSrc->uiCoeffAddr+3)*4) + (i*20) + ui32WrBankOffset, 
                    psSRCConfigParams->uCoeff.sIIRCoeff.a1[i]);
                BRAP_Write32(hSrc->hRegister, 
                    hSrc->uiBlkOffset + BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE + ((hSrc->uiCoeffAddr+4)*4) + (i*20) + ui32WrBankOffset, 
                    psSRCConfigParams->uCoeff.sIIRCoeff.a2[i]);
            }

            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_SRC_CTRL0_SRC_CFGi,WR_BANK_SEL));                    
            ui32RegVal |= (BCHP_FIELD_DATA(AUD_FMM_SRC_CTRL0_SRC_CFGi,WR_BANK_SEL,ui32WrBankSel));

            BRAP_Write32(hSrc->hRegister, hSrc->uiBlkOffset+ (BCHP_AUD_FMM_SRC_CTRL0_SRC_CFGi_ARRAY_BASE + (4 * hSrc->uiIndex)),ui32RegVal);
            
            
            BDBG_MSG(("BRAP_SRC_P_SetConfig_isr: Programmed coeff for hSrc = %x",hSrc));
            break;
#endif
        default:
            BSTD_UNUSED(psSRCConfigParams);
            BSTD_UNUSED(i);
            BSTD_UNUSED(ui32RegVal);            
            BDBG_ERR (("BRAP_SRC_P_SetConfig_isr: SRC type %d is not supported",hSrc->sParams.eSrcType));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

exit:
    BDBG_LEAVE (BRAP_SRC_P_SetConfig_isr);
    return ret;    
}


/***************************************************************************
Summary:
    Allocates SRC Coefficient Address.
    
Description:
    Provides the Index of the SRC Coefficient Array base from which the SRC 
    coefficients need to be programmed sequentially.

Returns:
    BERR_SUCCESS on success else error

See Also: 
    BRAP_SRC_P_DeallocateCoeffAddr
**************************************************************************/
BERR_Code BRAP_SRC_P_AllocateCoeffAddr (
            BRAP_SRC_P_Handle   hSrc           /* [in] SRC Handle */
    )
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int    uiCoeffIndex = 0;
    
    BDBG_ENTER (BRAP_SRC_P_AllocateCoeffAddr);
    BDBG_ASSERT (hSrc);

    if(BRAP_INVALID_VALUE != hSrc->uiCoeffAddr)
    {
        BDBG_ERR(("BRAP_SRC_P_AllocateCoeffAddr: SRC already has valid Coefficient address %d."
            "Check if the old value is not cleared", hSrc->uiCoeffAddr));
        return BERR_TRACE(BERR_INVALID_PARAMETER);            
    }
    
    switch (hSrc->sParams.eSrcType)
    {
        case BRAP_SRC_P_Type_eLinInt:
            if(hSrc->hFmm->bSrcCoeffUsed[0][128] == false)
            {
                hSrc->uiCoeffAddr = 128;
                for(uiCoeffIndex = 0; uiCoeffIndex < 3; uiCoeffIndex++)
                {
                    hSrc->hFmm->bSrcCoeffUsed[0][hSrc->uiCoeffAddr + uiCoeffIndex] = true;            
                }
            }
            else if(hSrc->hFmm->bSrcCoeffUsed[0][131] == false)
            {
                hSrc->uiCoeffAddr = 131;
                for(uiCoeffIndex = 0; uiCoeffIndex < 3; uiCoeffIndex++)
                {
                    hSrc->hFmm->bSrcCoeffUsed[0][hSrc->uiCoeffAddr + uiCoeffIndex] = true;            
                }            
            }
            /* Enable below code if 3 LInt SRC Types are required */
#if 0            
            else if(hSrc->hFmm->bSrcCoeffUsed[0][134] == false)
            {
                hSrc->uiCoeffAddr = 134;
                for(uiCoeffIndex = 0; uiCoeffIndex < 3; uiCoeffIndex++)
                {
                    hSrc->hFmm->bSrcCoeffUsed[0][134+uiCoeffIndex] = true;            
                }            
            }
#endif            
            else
            {
                BDBG_ERR(("BRAP_SRC_P_AllocateCoeffAddr: More than 2 Linear Interpolation SRCs not supported."));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            break;
#if BRAP_P_EQUALIZER    
        case BRAP_SRC_P_Type_eIIR:
            if(hSrc->hFmm->bSrcCoeffUsed[hSrc->uiBlkId][0+(200*!(hSrc->uiBlkId))] == false)
            {
                hSrc->uiCoeffAddr = 0+(200*!(hSrc->uiBlkId));
                hSrc->hFmm->bSrcCoeffUsed[hSrc->uiBlkId][hSrc->uiCoeffAddr] = true;
            }
            else if (hSrc->hFmm->bSrcCoeffUsed[hSrc->uiBlkId]
                [0+((BRAP_P_MAX_IIR_FILTERS_IN_SRC*5)*2)+(200*!(hSrc->uiBlkId))] == false)
            {
                hSrc->uiCoeffAddr = 0+((BRAP_P_MAX_IIR_FILTERS_IN_SRC*5)*2)+(200*!(hSrc->uiBlkId));
                hSrc->hFmm->bSrcCoeffUsed[hSrc->uiBlkId][hSrc->uiCoeffAddr] = true;
            }
            else
            {
                BDBG_ERR(("BRAP_SRC_P_AllocateCoeffAddr: No free Coefficient memory available."
                    "Check if more than 2 IIR filters are configured or unused coeff memory is not freed!"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }            
            break;
#endif

        default:
            BDBG_ERR(("BRAP_SRC_P_AllocateCoeffAddr: SRC type %d not supported", hSrc->sParams.eSrcType));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("BRAP_SRC_P_AllocateCoeffAddr: SRC Index = %d  Type = %d  CoeffAddr = %d",
        hSrc->uiIndex,hSrc->sParams.eSrcType,hSrc->uiCoeffAddr));

    BDBG_LEAVE (BRAP_SRC_P_AllocateCoeffAddr);
    return ret;
}

BERR_Code BRAP_SRC_P_DeallocateCoeffAddr (
            BRAP_SRC_P_Handle   hSrc           /* [in] SRC Handle */
    )
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int    uiCoeffIndex = 0;
    
    BDBG_ENTER (BRAP_SRC_P_DeallocateCoeffAddr);
    BDBG_ASSERT (hSrc);

    if(BRAP_INVALID_VALUE == hSrc->uiCoeffAddr)
    {
        BDBG_ERR(("BRAP_SRC_P_DeallocateCoeffAddr: Invalid Coefficient address %d",
            hSrc->uiCoeffAddr));
        return BERR_TRACE(BERR_INVALID_PARAMETER);            
    }
    
    BDBG_MSG(("BRAP_SRC_P_DeallocateCoeffAddr: SRC Index = %d  Type = %d  CoeffAddr = %d",
        hSrc->uiIndex,hSrc->sParams.eSrcType,hSrc->uiCoeffAddr));
    
    /* To-Do: Make this common for all SRC types */
    switch (hSrc->sParams.eSrcType)
    {
        case BRAP_SRC_P_Type_eLinInt:
            if(!((hSrc->uiCoeffAddr == 128) || (hSrc->uiCoeffAddr == 131)))
            {
                BDBG_ERR(("BRAP_SRC_P_DeallocateCoeffAddr: Coefficient address %d is"
                    "not allocated to the SRC Index %d. Can't return it", 
                    hSrc->uiCoeffAddr, hSrc->uiIndex));
                return BERR_TRACE(BERR_INVALID_PARAMETER);        
            }
            
            for(uiCoeffIndex = 0; uiCoeffIndex < 3; uiCoeffIndex++)
            {
                hSrc->hFmm->bSrcCoeffUsed[0][hSrc->uiCoeffAddr+uiCoeffIndex] = false;            
            } 
            hSrc->uiCoeffAddr = BRAP_INVALID_VALUE;            
            break;
#if BRAP_P_EQUALIZER
        case BRAP_SRC_P_Type_eIIR:
            if((hSrc->uiBlkId == 0) && (hSrc->uiCoeffAddr < 200))
            {
                BDBG_ERR(("BRAP_SRC_P_DeallocateCoeffAddr: Coefficient address %d is"
                    "not allocated to the SRC Index %d. Can't return it", 
                    hSrc->uiCoeffAddr, hSrc->uiIndex));
                return BERR_TRACE(BERR_INVALID_PARAMETER);        
            }  
            
            hSrc->hFmm->bSrcCoeffUsed[hSrc->uiBlkId][hSrc->uiCoeffAddr] = false;
            hSrc->uiCoeffAddr = BRAP_INVALID_VALUE;
            break;
#endif        
        default:
            BDBG_ERR(("BRAP_SRC_P_DeallocateCoeffAddr: SRC type %d not supported", 
                        hSrc->sParams.eSrcType));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    BDBG_LEAVE (BRAP_SRC_P_DeallocateCoeffAddr);
    return ret;
}
#endif

