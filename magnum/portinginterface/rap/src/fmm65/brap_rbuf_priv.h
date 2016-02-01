/***************************************************************************
*     Copyright (c) 2004-2009, Broadcom Corporation
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
*   Module name: RBUF
*   This file lists all data structures, macros, enumerations and function 
*   prototypes for the RingBuffer abstraction, which are internal ie NOT
*   exposed to the application developer. These can be used only by the 
*   Audio Manager and other FMM submodules.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/


#ifndef _BRAP_RBUF_PRIV_H_
#define _BRAP_RBUF_PRIV_H_

#include "brap_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Free/Full byte mark percentage*/    
#define BRAP_RBUF_P_DEFAULT_WATER_MARK        100   
/* Free byte mark percentage: used for Src RBUFs */
#define BRAP_RBUF_P_DEFAULT_FREE_BYTE_MARK    BRAP_RBUF_P_DEFAULT_WATER_MARK 
/* Full byte mark percentage: used for DEST RBUFs */    
#define BRAP_RBUF_P_DEFAULT_FULL_BYTE_MARK    (100 - BRAP_RBUF_P_DEFAULT_WATER_MARK)  
/* Start WR Point percentage */     
/* TODO: START_WR_POINT = END_ADDR only for Decode channel. Will be different for
   Playback & Capture channels */
#define BRAP_RBUF_P_DEFAULT_START_WR_POINT    BRAP_RBUF_P_DEFAULT_SIZE 

/* Default size for RBUFs (without delay) ie these RBUFS cannot support independent output port delay*/
#if (BRAP_3548_FAMILY == 1) || (BRAP_7405_FAMILY == 1)

/* NEWFWARCH */
#define BRAP_RBUF_P_DEFAULT_SIZE    BRAP_AF_P_NON_DELAY_RBUF_SIZE
#define BRAP_P_HBR_RBUF_SIZE        BRAP_RBUF_P_DEFAULT_SIZE * 4
#else
#define BRAP_RBUF_P_DEFAULT_SIZE              (2048*5*4)
#define BRAP_P_HBR_RBUF_SIZE                        (2048*5*4*4)

#endif
/* Default size for RBUFs to be used for independent output port delay */

#if ((BRAP_7405_FAMILY == 1)||(BRAP_3548_FAMILY == 1))
/* NEWFWARCH */
#define BRAP_RBUF_P_DEFAULT_DELAY_RBUF_SIZE     BRAP_AF_P_DELAY_RBUF_SIZE 
#define BRAP_P_HBR_DELAY_RBUF_SIZE        BRAP_RBUF_P_DEFAULT_DELAY_RBUF_SIZE * 4
#else
#define BRAP_RBUF_P_DEFAULT_DELAY_RBUF_SIZE              (2048*17*4) 
#endif

/* No delay - 0 */
#define BRAP_RBUF_P_NO_DELAY                  0
#define BRAP_RBUF_P_ALIGNMENT                 256

  
/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on Opening the Rbuf.

Description:
    This lists all the parameters required to open the Ring Buffer, 
    including both internal and external parameters.
    Note that the internal parameters are not exposed to the application.
    Currently no extra private settings are required.

See Also:

***************************************************************************/
typedef struct BRAP_RBUF_P_Settings
{
    BRAP_RBUF_Settings sExtSettings; /* Parameters provided by the application */
    bool               bProgRdWrRBufAddr; 
                                     /* TRUE: If Read-Write pointers to be 
                                        programmed during channel open.
                                        Required for PCM PB and CAP channels.
                                        FALSE: Other wise. */
    bool bRbufOfClonedPort;         /*  Program this to true for Add output
                                        ports in 7400 across the DP's. This makes
                                        sure that ring buffers are allocated for 
                                        the cloned port but no memory is allocated 
                                        to them.*/

#if (BRAP_SECURE_HEAP==1)
	bool 			bSecureMemory;		/* TRUE: RBUFs are to be allocated from Secure region.
										    FALSE: RBufs are to be allocated from normal memory */
#endif    	
										
} BRAP_RBUF_P_Settings;


/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on Starting the Ring Buffer.
    
Description:
    This lists all the parameters required to start the Ring Buffer, 
    including both internal and external parameters.
    Note that the internal parameters are not exposed to the application.
    Currently no extra private Parameters are required for the RBUF.

See Also:
    BRAP_FMM_Params

***************************************************************************/
typedef struct BRAP_RBUF_P_Params
{
    unsigned int uiStartWRPoint;
    unsigned int uiStartSize;
}BRAP_RBUF_P_Params; 



/***************************************************************************
Summary:
    Abstraction of a Ring Buffer 
    
Description:
    It contains all the information required to handle the RBUF.
    Particularly, it stores the RBUF index, handles for all required chip 
    related information, parent FMM handle, offset required to access 
    different RBUFs etc

See Also:
***************************************************************************/
typedef struct BRAP_RBUF_P_Object
{

    BRAP_RBUF_P_Settings sSettings; /* Ring buffer settings provided during
                                       Open() */

    BRAP_RBUF_P_Params   sParams;   /* Ring buffer settings for 
                                       the current instance provided 
                                       in Start() */
    BCHP_Handle          hChip;     /* Handle to chip object */
    BREG_Handle          hRegister; /* Handle to register object */
    BMEM_Handle          hHeap;     /* Handle to memory object */
    BINT_Handle          hInt;      /* Handle to interrupt object */
    BRAP_FMM_P_Handle    hFmm;      /* Parent FMM handle */

    unsigned int         uiIndex;   /* Ring buffer index */
    uint32_t             ui32Offset;  /* Offset of a register of current ring 
                                       buffer from the corresponding register
                                       of the first ring buffer */
#if ( BRAP_7405_FAMILY == 1) || ( BRAP_3548_FAMILY == 1)		                                       
	bool 				bAllocatedInternally; /* TRUE = If Ring Buffer is allocated internally
												False = If Ring Buffer is allocatedd Externally */
#endif													
}BRAP_RBUF_P_Object;

/***************************************************************************
Summary:
    Initializes the Ring Buffer and returns a Ring Buffer handle .

Description:
    This function must be called first to get a handle .  This
    handle is used for all other function calls.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_RBUF_P_Close, , BRAP_RBUF_P_Start, BRAP_RBUF_P_Stop, 
    BRAP_RBUF_P_GetDefaultSettings.
**************************************************************************/
BERR_Code BRAP_RBUF_P_Open (
    BRAP_FMM_P_Handle      hFmm,           /* [in] FMM handle */
    BRAP_RBUF_P_Handle *   phRBuf,         /* [out] Pointer to RBUF handle */
    unsigned int           uiRbufIndex,    /* [in] RBUF index */           
    const BRAP_RBUF_P_Settings * pSettings /* [in] Open time settings */
);


/***************************************************************************
Summary:
    Starts the Ring Buffer.

Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_RBUF_P_Open, BRAP_RBUF_P_Close, BRAP_RBUF_P_Stop, 
    BRAP_RBUF_P_GetDefaultSettings.
**************************************************************************/
BERR_Code BRAP_RBUF_P_Start (
    BRAP_RBUF_P_Handle   hRBuf,          /* [in] RBUF handle */
    const BRAP_RBUF_P_Params * pParams   /* [in] Start time settings */  
);


/***************************************************************************
Summary:
    Stops the Ring Buffer.

Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_RBUF_P_Open, BRAP_RBUF_P_Close, BRAP_RBUF_P_Start, 
    BRAP_RBUF_P_GetDefaultSettings.
**************************************************************************/
BERR_Code BRAP_RBUF_P_Stop (
    BRAP_RBUF_P_Handle    hRBuf     /* [in] RBUF handle */
);



/***************************************************************************
Summary:
    Releases the Ring Buffer handle

Description:
    This function will undo what has been done in Open.  

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_RBUF_P_Open.
**************************************************************************/
BERR_Code BRAP_RBUF_P_Close ( 
    BRAP_RBUF_P_Handle      hRbuf   /* [in] RBUF handle */
);


/***************************************************************************
Summary:
    Returns default values for RBuf Open time settings.

Description:
    For settings that the system cannot assign default values to, 
    an invalid value is returned. Note that the default settings are common
    for all Rbufs ie different Rbufs do not have different default settings.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_RBUF_P_GetDefaultParams.
**************************************************************************/
BERR_Code 
BRAP_RBUF_P_GetDefaultSettings ( 
    BRAP_RBUF_P_Settings   * pDefSettings  /* [out] Pointer to memory where default
                                              settings should be written */
);

/***************************************************************************
Summary:
    Returns default values for RBuf Start time parameters.

Description:
    For parameters that the system cannot assign default values to, 
    an invalid value is returned. Note that the default parameters are common
    for all Rbufs ie different Rbufs do not have different default parameters.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_RBUF_P_GetDefaultSettings.
**************************************************************************/
BERR_Code 
BRAP_RBUF_P_GetDefaultParams ( 
    BRAP_RBUF_P_Params    *pDefParams   /* [out] Pointer to memory where default
                                           parameters should be written */    
);


BERR_Code BRAP_RBUF_P_GetFrameSize(
    unsigned int uiRBufSize, 
    unsigned int *pFrameSize
);

/***************************************************************************
Summary:
    Configures the End address of the the Ring Buffer

Description:


Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    None.
**************************************************************************/
BERR_Code BRAP_RBUF_P_ProgramEndAddress (
    BRAP_RBUF_P_Handle  hRBuf  		/* [in] RBUF handle */
);

#ifdef DEBUG_RING_BUFF_CAPTURE
/*************************************************************************
Summary:
    These api's are defined to capture the Ring buffer
*************************************************************************/
void BRAP_RBUF_P_CapUninit(void);
void BRAP_RBUF_P_CapInit(BREG_Handle reg_handle, BMEM_Heap_Handle heap);
void BRAP_RBUF_P_CapChStopSM(uint32_t path,uint32_t channel);
void BRAP_RBUF_P_CapChStopEnc(uint32_t path,uint32_t channel);
void BRAP_RBUF_P_CapChStopMixer(uint32_t path,uint32_t channel);
void BRAP_RBUF_P_CapChStopTranscode(uint32_t path,uint32_t channel);
void BRAP_RBUF_P_CapChStopPassthru(uint32_t path,uint32_t channel);
void BRAP_RBUF_P_CapChStopDecoder(uint32_t path,uint32_t channel);
void BRAP_RBUF_P_CapChStartDecoder(uint32_t path,uint32_t channel,uint32_t rbuf_id);
void BRAP_RBUF_P_CapChStartPassthru(uint32_t path,uint32_t channel,uint32_t rbuf_id);
void BRAP_RBUF_P_CapChStartTranscode(uint32_t path,uint32_t channel,uint32_t rbuf_id);
void BRAP_RBUF_P_CapChStartMixer(uint32_t path,uint32_t channel,uint32_t rbuf_id);
void BRAP_RBUF_P_CapChStartEnc(uint32_t path,uint32_t channel,uint32_t rbuf_id);
void BRAP_RBUF_P_CapChStartSM(uint32_t path,uint32_t channel,uint32_t rbuf_id);
#endif

#ifdef __cplusplus
}
#endif


#endif /* _BRAP_RBUF_PRIV_H_ */

/* End of File */
