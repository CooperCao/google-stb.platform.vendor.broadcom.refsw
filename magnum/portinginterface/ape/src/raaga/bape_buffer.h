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
 * Module Description: Simple Buffer Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_BUFFER_H_
#define BAPE_BUFFER_H_

#include "bmem.h"

#define BAPE_MIN( v0, v1 )        (((v0) < (v1)) ? (v0) : (v1))

/***************************************************************************
Summary:
Simple Buffer Descriptor
***************************************************************************/
typedef struct BAPE_SimpleBufferDescriptor
{
    void *pBuffer;              /* Buffer base address prior to wraparound */
    void *pWrapBuffer;          /* Buffer address after wraparound (NULL if no wrap has occurred) */

    unsigned bufferSize;            /* Buffer size before wraparound in bytes */
    unsigned wrapBufferSize;        /* Buffer size after wraparound in bytes */        
} BAPE_SimpleBufferDescriptor;

typedef struct BAPE_Buffer *BAPE_BufferHandle;

/***************************************************************************
Summary:
Buffer Settings
***************************************************************************/
typedef struct BAPE_BufferSettings
{
    BMEM_Handle heap;                   /* Heap to use for internal allocation */
    void *userBuffer;                   /* User can pass in an externally allocated buffer, 
                                           Heap should be null in that case */
    size_t bufferSize;                  /* Buffer size in bytes */
} BAPE_BufferSettings;
 
/***************************************************************************
Summary:
Buffer Get Default Settings
***************************************************************************/
void BAPE_Buffer_GetDefaultSettings(
    BAPE_BufferSettings *pSettings /*[out] */
    );

/***************************************************************************
Summary:
Buffer Open
***************************************************************************/
BERR_Code BAPE_Buffer_Open(
    const BAPE_BufferSettings * pSettings, 
    BAPE_BufferHandle * pHandle /* [out] */
    );

/***************************************************************************
Summary:
Buffer Close
***************************************************************************/
void BAPE_Buffer_Close(
    BAPE_BufferHandle handle
    );

/***************************************************************************
Summary:
Buffer Read
***************************************************************************/
unsigned BAPE_Buffer_Read_isr(
    BAPE_BufferHandle handle, 
    BAPE_SimpleBufferDescriptor * pDescriptor /* [out] */
    );

/***************************************************************************
Summary:
Buffer Write
***************************************************************************/
unsigned BAPE_Buffer_Write_isr(
    BAPE_BufferHandle handle, 
    void * pData,
    unsigned size
    );

/***************************************************************************
Summary:
Buffer Advance
***************************************************************************/
unsigned BAPE_Buffer_Advance_isr(
    BAPE_BufferHandle handle, 
    unsigned size
    );
 
#endif /* #ifndef BAPE_BUFFER_H_ */

