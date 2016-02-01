/***************************************************************************
*     Copyright (c) 2004-2005, Broadcom Corporation
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
*   Module name: DSTCH
*   This file lists all data structures, macros, enumerations and function 
*   prototypes for the Source Channel (Fifo) abstraction, which are exposed 
*   to the application developer. It covers the functionality of the Capture 
*   Hardware Block also. This object is only used by a PCM Capture Channel.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _BRAP_DSTCH_H_
#define _BRAP_DSTCH_H_     

#ifdef __cplusplus
extern "C" {
#endif

/* Input I2S Params */
typedef struct BRAP_DSTCH_I2sParams
{
    bool    bLsbAtLRClk;    /* Data Justification. Controls whether LSB or 
                                MSB is at LRCK transition 
                                TRUE: LSB, FALSE: MSB */
    bool    bAlignedToLRClk;    /* Controls whether data is aligned with 
                                LRCK or delayed  by one SCLK period 
                                FALSE: Delayed. TRUE: Aligned */
    bool    bLRClkPolarity; /* Sets the polarity of the left/right clock 
                                TRUE: High for Left FALSE: Low for Left */
} BRAP_DSTCH_I2sParams;


/* Destination Channel External Params */
typedef struct BRAP_DSTCH_Params
{
    BRAP_CapInputPort   eCapPort;/* Capture Input Port: Capture Input Port to be used */    
    BRAP_DSTCH_I2sParams    sInputI2sParams;    /* Input I2S Params. It is used only if the Capture input port is of type I2S */
} BRAP_DSTCH_Params;

#ifdef __cplusplus
}
#endif

#endif /* _BRAP_DSTCH_H_ */


/* End of File */
