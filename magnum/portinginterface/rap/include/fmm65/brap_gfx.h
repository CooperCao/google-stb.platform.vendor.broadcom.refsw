/***************************************************************************
*     Copyright (c) 2006-2010, Broadcom Corporation
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
*	This file contains structures, enums, macros and function prototypes, 
*	which are exposed to the upper layer for Grahics on ZSP by Rap PI. In other 
*	words this file defines the interfaces between the upper layer and the 
*	Raptor Gfx PI layer.
*
* Revision History:
*
* $brcm_Log: $
* 
*****************************************************************************/
#ifndef _BRAP_GFX_H_
#define _BRAP_GFX_H_
    
#ifdef __cplusplus
    extern "C" {
#endif

#include "brap.h"

/****************************************************************************
Supported surface/palette Pixel Formats:
BPXL_eP8
BPXL_eA8_R8_G8_B8
BPXL_eR8_G8_B8_A8
BPXL_eR5_G6_B5
BPXL_eA1_R5_G5_B5
BPXL_eR5_G5_B5_A1
BPXL_eA4_R4_G4_B4
BPXL_eR4_G4_B4_A4
BPXL_eA8_Y8_Cb8_Cr8
***************************************************************************/

/****************************Structure Defenition Starts here*************************/

#define BRAP_MAX_GFX_OP_SUPPORTED   19


/***************************************************************************
Summary:
    Graphics Handle
****************************************************************************/
typedef BRAP_ChannelHandle BRAP_GFX_Handle;

/****Typedefs****/

/***************************************************************************
Summary:
    Pixel Type
****************************************************************************/
typedef uint32_t BRAP_GFX_Pixel; 

/****Enums*****/

/***************************************************************************
Summary:
    Fill operations
****************************************************************************/
typedef enum BRAP_GFX_eFillOperation
{
    BRAP_GFX_eFillOperation_eIgnore,   /* Do not change the values for the channel (either color or alpha) */
    BRAP_GFX_eFillOperation_eCopy,     /* Copy the value of the channel (color or alpha) from the constant color to the surface */
    BRAP_GFX_eFillOperation_eBlend,    /* For color channel, blend surface color with constant color using constant alpha.
                                     This operation is not support for the alpha channel. */
    BRAP_GFX_eFillOperation_eMax
}BRAP_GFX_eFillOperation;

/***************************************************************************
Summary:
    Color blit operations
****************************************************************************/
typedef enum BRAP_GFX_eBlitColorOperation
{
    BRAP_GFX_eBlitColorOperation_eCopySource,      /* Copy the source color with no blending. Valid only with B_SoftGfx_BlitAlphaOp_eCopySource. This means Blitting. */
    BRAP_GFX_eBlitColorOperation_eUseConstantAlpha,    /* Blend the source and dest colors using the alpha from the constAlphaColorOp param */
    BRAP_GFX_eBlitColorOperation_eUseSourceAlpha,      /* Blend the source and dest colors using the alpha from the source pixels */
    BRAP_GFX_eBlitColorOperation_eUseDestinationAlpha, /* Blend the source and dest colors using the alpha from the dest pixels */
    BRAP_GFX_eBlitColorOperation_eUseBlendEquation,    /* Blend the source and dest colors using constant alpha for color & blend operations */
    BRAP_GFX_eBlitColorOperation_eIgnore,
    BRAP_GFX_eBlitColorOperation_eMax
}BRAP_GFX_eBlitColorOperation;


/***************************************************************************
Summary:
    Alpha blit operations
****************************************************************************/
typedef enum BRAP_GFX_eBlitAlphaOperation
{
    BRAP_GFX_eBlitAlphaOperation_eCopySource,          /* Oa = Sa. Copy the source alpha to the output alpha. */
    BRAP_GFX_eBlitAlphaOperation_eCopyDestination,     /* Oa = Da. Copy the dest alpha to the output alpha */
    BRAP_GFX_eBlitAlphaOperation_eCopyConstant,        /* Oa = constAlphaAlphaOp. Copy the constant alpha for alphaOp parameter as the output alpha */    
    BRAP_GFX_eBlitAlphaOperation_eUseBlendEquation,    /* Alpha = 1 - (1 - Alpha1) * (1 - Alpha2) 
                                                     where, Alpha1 and Alpha2 are defined as per the BlitColorOp given 
                                                     in the description of B_SoftGfx_BlitColorOp above */
    BRAP_GFX_eBlitAlphaOperation_eCombine,             /* Oa = Sa*Sa + Da*(1-Sa). Combine source and dest alpha. */                                                     
    BRAP_GFX_eBlitAlphaOperation_eIgnore,
    BRAP_GFX_eBlitAlphaOperation_eMax
}BRAP_GFX_eBlitAlphaOperation;




/****Structs****/

/***************************************************************************
Summary:
    Open-time Settings for a graphics channel
****************************************************************************/
typedef struct BRAP_GFX_OpenSettings    
{
    unsigned maxOperations;         /* Maximum number of concurrent operations */

} BRAP_GFX_OpenSettings;

/***************************************************************************
Summary:
    Run-time Settings for a graphics channel
****************************************************************************/
typedef struct  BRAP_GFX_Settings    
{
    void (*BRAP_CallbackFunc) (void *pParm1, int parm2, void *pgfx_data);
    void  *pCheckpointParam1;
    int    iCheckpointParam2;
}BRAP_GFX_Settings;

/***************************************************************************
Summary:
    Graphics rectangle

****************************************************************************/
typedef struct BRAP_GFX_Rect
{
    int32_t  iX;
    int32_t  iY;
    uint32_t uiWidth;
    uint32_t uiHeight;
}BRAP_GFX_Rect;

/***************************************************************************
Summary:
    Settings for a fill operation
****************************************************************************/
typedef struct BRAP_GFX_FillSettings
{
    BSUR_Surface_Handle hOutputSurface;
    BRAP_GFX_Rect           sFillRect;
    BRAP_GFX_Pixel          uiFillColor;

    BRAP_GFX_eFillOperation    eColorOperation;
    BRAP_GFX_eFillOperation    eAlphaOperation;
}BRAP_GFX_FillSettings;

/***************************************************************************
Summary:
    Color space conversion matrix
****************************************************************************/
#define BRAP_GFX_COLOR_MATRIX_COEFF_COUNT   20
typedef struct BRAP_GFX_CSCMatrix
{
    unsigned int uiShift;
    int  iCoeffMatrix[BRAP_GFX_COLOR_MATRIX_COEFF_COUNT];
} BRAP_GFX_CSCMatrix;

/***************************************************************************
Summary:
    Filter used for Scaling.
****************************************************************************/

typedef enum BRAP_GFX_eScalingFilter
{
    BRAP_GFX_eScalingFilter_LinearInterpolation,
    BRAP_GFX_eScalingFilter_Gaussian,
    BRAP_GFX_eScalingFilter_eMax
}BRAP_GFX_eScalingFilter;



typedef  struct BRAP_GFX_ColorKeyParams    
{
    struct {
        bool bEnabled;     /* Enable colorkey. If disabled, the following values are ignored. */
        BRAP_GFX_Pixel uiLower;   /* Lower colorkey value. */
        BRAP_GFX_Pixel uiUpper;   /* Upper colorkey value.*/
        BRAP_GFX_Pixel uiMask;    /* Mask to use when comparing color key to source color */
        BRAP_GFX_Pixel uiReplace; /* Replacement color */
        BRAP_GFX_Pixel uiReplaceMask; /*  Mask for replacement color */
    } sSource,sDest;
} BRAP_GFX_ColorKeyParams;


/***************************************************************************
Summary:
    Settings for a blit operation
****************************************************************************/
typedef struct BRAP_GFX_BlitSettings
{
    BSUR_Surface_Handle hSourceSurface;
    BSUR_Surface_Handle hDestinationSurface;
    BSUR_Surface_Handle hOutputSurface;

    BRAP_GFX_Rect       sSourceRect;
    BRAP_GFX_Rect       sDestinationRect;
    BRAP_GFX_Rect       sOutputRect;

    BRAP_GFX_eBlitColorOperation   eColorOperation;
    BRAP_GFX_eBlitAlphaOperation   eAlphaOperation;

    unsigned int                uiConstAlphaColorOp;
    unsigned int                uiConstAlphaAlphaOp;

    bool                        bEnableAntiFlutterFilter;  
    bool                        bEnableCSC;                 /* Enable color space conversion (CSC) */

    BRAP_GFX_CSCMatrix          sCSCMatrix;                 /* Color space conversion matrix */

    BRAP_GFX_eScalingFilter     eScalingFilter;/*Filter used for scaling*/

    BRAP_GFX_ColorKeyParams sColorKeyParams;
}BRAP_GFX_BlitSettings;


/****************************Structure Defenition Ends here*************************/



/****************************API Defenition Starts here*************************/


/***************************************************************************
Summary:
	Gets the default channel settings for an Graphics channel.
	
Description:
	This API returns the default channel settings of a Graphics channel.

Returns:
	BERR_SUCCESS on success else error
	
See Also:
	BRAP_GFX_Open
***************************************************************************/
BERR_Code BRAP_GFX_GetDefaultOpenSettings(
	BRAP_GFX_OpenSettings    *pDefSettings	/* [out] Default channel settings */
	);    


/***************************************************************************
Summary:
	API used to open a Graphics channel.
	
Description:
    It is used to instantiate a Graphics channel. It allocates the handle and
    starts the GFX task in the DSP.
	
	Note: This should be called only after the device has been opened with 
	BRAP_Open().
	
Returns:
	BERR_SUCCESS on success else error
	
See Also:
	BRAP_GFX_Close
****************************************************************************/
BERR_Code BRAP_GFX_Open(
	BRAP_Handle 			    hRap,		/* [in] The Raptor Audio device handle*/
	BRAP_GFX_Handle 		    *phGfxCh,	/* [out] The Gfx Channel handle */
	const BRAP_GFX_OpenSettings	*pSettings  /* [in] Open-time Settings*/ 
	);



    /***************************************************************************
    Summary:
        API used to close a Raptor Graphics Channel.
    
    Description:
        It closes the instance of a Graphics channel operation. It frees the channel
    handle and stops the gfx task in the DSP.
    
    Returns:
        BERR_SUCCESS on success else error
    
    See Also:
    BRAP_GFX_Open
    ****************************************************************************/
BERR_Code BRAP_GFX_Close( 
    BRAP_GFX_Handle  hGfxCh                  /* [in]The Gfx Channel handle */
        );

/***************************************************************************
Summary:
    Get default settings for a fill operation
****************************************************************************/
BERR_Code BRAP_GFX_GetDefaultFillSettings(        
    BRAP_GFX_FillSettings                   *psFillSettings /* [out] */              
    );            

    /***************************************************************************
    Summary:
        It does the Gfx Fill Operation.
    
    Description:
        This API does the Gfx Fill Operation as per the setting passed. It returns Operation Id which 
        corresponds to current request.
    
    Returns:
        BERR_SUCCESS if successful else error value.            
    
    See Also:
        BRAP_GFX_BlitOperation
    ****************************************************************************/
BERR_Code BRAP_GFX_Fill(        
    BRAP_GFX_Handle          hGfxCh,         /* [in] Gfx channel handle */
    const BRAP_GFX_FillSettings      *psFillSettings /* [in] Fill Settings*/
    );    

/***************************************************************************
Summary:
    Get default settings for a blit operation
****************************************************************************/
BERR_Code BRAP_GFX_GetDefaultBlitSettings(        
    BRAP_GFX_BlitSettings                   *psBlitSettings /* [out] */              
        );    

        /***************************************************************************
    Summary:
        It does the Gfx Blit Operation.
    
    Description:
        This API does the Gfx Blit Operation as per the setting passed. It returns Operation Id which 
        corresponds to current request. It can perform following as per settings passed to this function
        -Blit
        -Blend
        -CSC
        -Scaling
    
    Returns:
        BERR_SUCCESS if successful else error value.            
    
    See Also:
        BRAP_GFX_FillOperation
    ****************************************************************************/
BERR_Code BRAP_GFX_Blit(
    BRAP_GFX_Handle              hGfxCh,         /* [in] Gfx channel handle */
    const BRAP_GFX_BlitSettings  *psBlitSettings /* [in] Blit Settings*/
        );    

/***************************************************************************
Summary:
    Get current settings for a graphics channel
 
Description:
    Get current settings for a graphics channel
 
See Also: 
    BRAP_GFX_SetSettings
****************************************************************************/
BERR_Code BRAP_GFX_GetSettings(
    BRAP_GFX_Handle              hGfxCh,        /* [in] Gfx channel handle */
    BRAP_GFX_Settings           *pSettings      /* [out] Current settings */
    );            

/***************************************************************************
Summary:
    Set current settings for a graphics channel
 
Description:
    Set current settings for a graphics channel
 
See Also: 
    BRAP_GFX_GetSettings
****************************************************************************/
BERR_Code BRAP_GFX_SetSettings(
    BRAP_GFX_Handle              hGfxCh,        /* [in] Gfx channel handle */
    const BRAP_GFX_Settings     *pSettings      /* [in] Current settings */
    );            

/*******************************************************************************
Summary:
Check status of the graphics on DSP for completion of all blit and fill operations.

Description:
BRAP_GFX_Checkpoint will notify the caller via callback when all gfx operations prior to it being 
called have completed.  BRAP_GFX_Checkpoint never blocks.

Be aware that if BRAP_GFX_Checkpoint schedules a checkpointCallback, it is possible for other blits to be issued after BRAP_GFX_Checkpoint is called
but before the checkpointCallback arrives. When the checkpointCallback arrives, it only means that all blits prior to BRAP_GFX_Checkpoint have completed.

BRAP_GFX_Checkpoint should not be called from multiple threads with the same handle.
Instead, each thread should open its own BRAP_GFX_Handle.

Return values :
    BERR_SUCCESS - The DSP job queue is empty.
    BRAP_GFX_QUEUED - Callback will be issued when the DSP has completed all pending operations.
*******************************************************************************/
BERR_Code BRAP_GFX_Checkpoint(        
    BRAP_GFX_Handle          hGfxCh          /* [in] Gfx channel handle */
    );            

/*******************************************************************************
Summary:
Check status of the graphics on DSP for completion of all blit and fill operations.

Description:
BRAP_GFX_Idle will check if the DSP job queue is empty.

Return values :
    BERR_SUCCESS - The DSP job queue is empty.
    BRAP_GFX_BUSY - The DSP job queue is not empty.
 
*******************************************************************************/
BERR_Code BRAP_GFX_Idle(        
    BRAP_GFX_Handle          hGfxCh          /* [in] Gfx channel handle */
    );            

/****************************API Defenition Ends here*************************/

#ifdef __cplusplus
    }
#endif
    
    
#endif /* !_BRAP_GFX_H_ */

