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
*   This file lists all data structures, macros, enumerations and function 
*   prototypes for the Sample Rate Converter abstraction,which are internal
*   ie NOT exposed to the application developer. These can be used only by 
*   the Audio Manager and other FMM submodules.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _BRAP_SRC_PRIV_H_
#define _BRAP_SRC_PRIV_H_

#include "brap.h"
#include "brap_priv.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
    This enumeration defines the type of the SRC operation.
    
***************************************************************************/
typedef enum BRAP_SRC_P_Type
{
    BRAP_SRC_P_Type_eBypass=0,      /* Bypass */
#if (BRAP_3548_FAMILY == 1)        
    BRAP_SRC_P_Type_eX1,            /* X1 */
#endif
    BRAP_SRC_P_Type_eUp2,           /* Up Sample 2:1 */
    BRAP_SRC_P_Type_eDown2,         /* Down Sample 2:1 */
    BRAP_SRC_P_Type_eUp4,           /* Up Sample 4:1 */
    BRAP_SRC_P_Type_eDown4,         /* Down Sample 4:1 */
    BRAP_SRC_P_Type_eLinInt,        /* Linear Interpolation */
#if (BRAP_3548_FAMILY == 1)        
    BRAP_SRC_P_Type_eIIR,           /* IIR */
#endif
    BRAP_SRC_P_Type_eUnknown        /* Unknown / illegal */
}BRAP_SRC_P_Type;


/***************************************************************************
Summary:
    
Description:

See Also:

***************************************************************************/
typedef struct BRAP_SRC_P_SrcInfo
{
    unsigned int            uiNumSrc;       /* Number of SRCs */       
	BRAP_SRC_P_Type         eSrcType[BRAP_RM_P_MAX_SRC_IN_CASCADE]; /* SRC Type */
    BAVC_AudioSamplingRate  eInSR[BRAP_RM_P_MAX_SRC_IN_CASCADE];    /* In SR */
    BAVC_AudioSamplingRate  eOutSR[BRAP_RM_P_MAX_SRC_IN_CASCADE];   /* Out SR */
}BRAP_SRC_P_SrcInfo;

/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on Opening a SRC.
    Note that this includes both public and private settings.

Description:

See Also:

***************************************************************************/
typedef struct BRAP_SRC_P_Settings
{
	unsigned int tbd;
}BRAP_SRC_P_Settings;


/***************************************************************************
Summary:
    IIR Coefficients.

Description:

See Also:

***************************************************************************/
#if BRAP_P_EQUALIZER
typedef struct BRAP_SRC_P_IIRCoeff
{
    int32_t b0[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t b1[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t b2[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t a1[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
    int32_t a2[BRAP_P_MAX_IIR_FILTERS_IN_SRC];
}BRAP_SRC_P_IIRCoeff;
#endif

/***************************************************************************
Summary:
    Linear Interpolation Coefficients.

Description:

See Also:

***************************************************************************/
typedef struct BRAP_SRC_P_LintCoeff
{
    int32_t uiNumerator;
    int32_t uiDenominator;
    int32_t uiDenScale;
}BRAP_SRC_P_LintCoeff;

/***************************************************************************
Summary:
    SRC Coefficients.

Description:

See Also:

***************************************************************************/
typedef union BRAP_SRC_P_Coeff
{
    BRAP_SRC_P_LintCoeff    sLintCoeff;
#if BRAP_P_EQUALIZER
    BRAP_SRC_P_IIRCoeff     sIIRCoeff;
#endif   
    /* Other SRC modes might need to be supported as and when the need arises */
}BRAP_SRC_P_Coeff;

/***************************************************************************
Summary:
    Different configuration parameters to be programmed.
    
Description:

See Also:

***************************************************************************/
typedef struct BRAP_SRC_P_ConfigParams
{
    BRAP_SRC_P_Coeff        uCoeff;
}BRAP_SRC_P_ConfigParams;

/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on starting a SRC.
    Note that this includes both public and private settings.

Description:

See Also:

***************************************************************************/
typedef struct BRAP_SRC_P_Params
{
    BRAP_SRC_P_Type         eSrcType;       /* Type of the SRC - optional. If 
                                               BRAP_SRC_P_Type_eunknown is passed
                                               based on the input and output SR
                                               the type is decided internally */
    bool                    bRampEnable;    /* Scale ramp enable: If TRUE the scaling
                                               is applied to a playback channel during
                                               start up. FALSE: for Compressed data */
    bool                    bPriorityHigh;  /* Priority of the client */
    BAVC_AudioSamplingRate  eInputSR;       /* The Input Sampling Rate to the SRC. 
                                               This would be used only if eSrcType
                                               is BRAP_SRC_P_Type_eUnknown or 
                                               BRAP_SRC_P_Type_eLinInt. For all
                                               other eSrcType, inputSR is required */
    BAVC_AudioSamplingRate  eOutputSR;      /* The Output Sampling Rate to the SRC. 
                                               This would be used only if eSrcType
                                               is BRAP_SRC_P_Type_eUnknown or 
                                               BRAP_SRC_P_Type_eLinInt. For all
                                               other eSrcType, outputSR is required */
    unsigned int            uiGroupId;      /* The Group ID of the SRC */
    uint32_t                ui32FciId;      /* The FCI ID of the Input Source to SRC */
}BRAP_SRC_P_Params;


/***************************************************************************
Summary:
    Abstraction of a SRC 
    
Description:
    It contains all the information required to handle the SRC.
    Particularly, it stores the indexes, handles for all required chip 
    related information, parent FMM handle, offset required to access 
    different SRC etc

See Also:
    
***************************************************************************/
typedef struct BRAP_SRC_P_Object
{
    BCHP_Handle             hChip;          /* Handle to chip object */
    BREG_Handle             hRegister;      /* Handle to register object */
    BMEM_Handle             hHeap;          /* Handle to memory object */
    BINT_Handle             hInt;           /* Handle to interrupt object */
    BRAP_FMM_P_Handle       hFmm;           /* Parent FMM handle */

    unsigned int            uiBlkId;        /* The SRC block ID */
    unsigned int            uiBlkOffset;    /* The block Offset */
    unsigned int            uiIndex;        /* The SRC Index in a particular block */
    unsigned int            uiSrcOffset;    /* The SRC Offset */
    unsigned int            uiSrcOpenCount; /* The SRC Open Count */
    unsigned int            uiSrcStartCount;/* The SRC start Count */

    BRAP_SRC_P_Settings     sSettings;      /* SRC settings provided during OPEN */
    BRAP_SRC_P_Params       sParams;        /* SRC settings provided during START */
    BRAP_P_State            eState;         /* State of SRC */
	unsigned int            uiBaseAddr;     /* Base Address of the SRAM allocated
	                                           for SRC. This depends on the 
	                                           type of SRC */ 
	unsigned int            uiCoeffAddr;    /* Coeff Address of the COEFF memory 
	                                           allocated for this SRC. 
	                                           This depends on the type of SRC */
                                               
}BRAP_SRC_P_Object;

/***************************************************************************
Summary:
    Opens an SRC.

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
    BRAP_SRC_P_Handle           *phSrc,     /* [out] Pointer to SRC handle */
    unsigned int                uiSrcIndex, /* [in] SRC index */
    unsigned int                uiSrcBlk,   /* [in] SRC block index */
    const BRAP_SRC_P_Settings   *pSettings  /* [in] SRC settings */
);

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
    BRAP_SRC_P_Handle           hSrc        /* [in] SRC Handle */
);

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
);

/***************************************************************************
Summary:
    Stops a SRC.

Description:
  
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SRC_Start.
**************************************************************************/
BERR_Code BRAP_SRC_P_Stop ( 
    BRAP_SRC_P_Handle           hSrc        /* [in] SRC Handle */
);

/***************************************************************************
Summary:
    Returns default values for SRC Open time settings.

Description:
    For settings that the system cannot assign default values to, 
    an invalid value is returned. Note that the default settings are common
    for all SRC.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SRC_P_GetDefaultParams.
**************************************************************************/
BERR_Code 
BRAP_SRC_P_GetDefaultSettings ( 
    BRAP_SRC_P_Settings         *pDefSettings   /* [out] Pointer to memory 
                                                   where default settings 
                                                   should be written */
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
    BRAP_SRC_P_Params           *pDefParams   /* [out] Pointer to memory 
                                                 where default settings 
                                                 should be written */ 
);

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
);


#if (BRAP_3548_FAMILY == 1)
/***************************************************************************
Summary:
    Allocates the coeffcient address in SRC Coeff space.
    
See Also:
    BRAP_SRC_P_DeallocateCoeffAddr
**************************************************************************/
    
BERR_Code BRAP_SRC_P_AllocateCoeffAddr (
            BRAP_SRC_P_Handle   hSrc           /* [in] SRC Handle */
);


/***************************************************************************
Summary:
    Deallocates the coeffcient address in SRC Coeff space.
    
See Also:
    BRAP_SRC_P_AllocateCoeffAddr
**************************************************************************/
    
BERR_Code BRAP_SRC_P_DeallocateCoeffAddr (
            BRAP_SRC_P_Handle   hSrc           /* [in] SRC Handle */
);


/***************************************************************************
Summary:
    Sets the config for an SRC.
    
See Also:
    
**************************************************************************/

BERR_Code BRAP_SRC_P_SetConfig_isr (
            BRAP_SRC_P_Handle   hSrc,           /* [in] SRC Handle */
            BRAP_SRC_P_ConfigParams *psSRCConfigParams  /* [in] Config Params */
    );

#endif

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
);

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
    );

#ifdef __cplusplus
}
#endif
#endif /* _BRAP_SRC_PRIV_H_ */

/*End of File*/
