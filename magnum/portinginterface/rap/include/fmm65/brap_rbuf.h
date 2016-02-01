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
*   prototypes for the RingBuffer abstraction, which are exposed to the 
*   application developer.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _BRAP_RBUF_H_
#define _BRAP_RBUF_H_     

#include "brap.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Parameters to be passed by application on Opening a Ring Buffer.

Description:
    Note: Free watermark percentage, say x, implies that if free number of 
          bytes in the associated ring buffer(s) exceeds x% of the total 
          buffer size, a ring buffer free water mark interrupt is generated.
See Also:

***************************************************************************/
typedef struct BRAP_RBUF_Settings
{
    void *          pBufferStart; /* Pointer to the start of this RBUF.
                                     If it is NULL, internally allocated memory
                                     is used. It has to be 256 byte aligned. */
    size_t          uiSize;       /* Ring Buffer size must be multiple of 
                                     256 bytes. If passed as 0, default value 
                                     indicated by BRAP_RBUF_P_DEFAULT_SIZE 
                                     will be used. 
                                     Note: Even if pBufferStart is NULL,
                                     uiSize can be passed for internal use */
    unsigned int    uiWaterMark;  /* Water Mark: Percentage of Free/Full Bytes.
                                     If passed as 0, default value indicated 
                                     by BRAP_RBUF_P_DEFAULT_WATER_MARK will 
                                     be used. The Water Mark Level depends on 
                                     the type of the System. 
                                     If Edge Triggered system, the application 
                                     provided Water Mark is Ignored and internally
                                     forced to be (RBuf Size-FrameSize), where 
                                     FrameSize is one-fourth of the Rbuf Size.
                                     If Level Triggered System, the user provided 
                                     Water Mark is Used.
                                     Note: Even if pBufferStart is NULL,
                                     uiWaterMark can be passed for internal use */
                                  
} BRAP_RBUF_Settings;

/***************************************************************************
Summary:
	Gets the base and end pointers of the ring buffer.

Description:
	Gets the base and end pointers of the ring buffer associated with an 
	output audio channel for a RAP Channel. This is a debug API.

Returns:
	BERR_SUCCESS 

See Also:
	

****************************************************************************/
BERR_Code 
BRAP_RBUF_GetBaseAndEndAddress( 
            BRAP_ChannelHandle hRapCh,        /* [in] Audio Device handle */
            BRAP_OutputChannel eOpCh,    /* [in] Output channel type */
            uint32_t         *pBaseAddr,      /* [Out] Ring buffer base address */   
            uint32_t         *pEndAddr        /* [Out] Ring buffer end address */ 
            );

BERR_Code   BRAP_GetDefaultRingBufferSize(unsigned int *uiSize);

#ifdef __cplusplus
}
#endif

#endif /* _BRAP_RBUF_H_ */

/* End of File */
