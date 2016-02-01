/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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

#ifndef BDBG_LOG_H
#define BDBG_LOG_H

#include "bdbg_fifo.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Registers Fifo that is used to log debug data

Description:
    When this function is called and fifo is not NULL, then debug output instead
    of being routed to the debug console, is stored in the provided FIFO.
    THen application could use BDBG_Dequeue_Fifo to extract and print messages 
    logged into FIFO. 
    If FIFO is NULL debug output then directed to the debug console .

Note:
    Element size of the provided FIFO should match number retuirned buy
    BDBG_Log_GetElementSize

See Also:
    BDBG_Log_Dequeue
    BDBG_Fifo_Create
    BDBG_Log_GetElementSize

Example:
    #include "bstd.h"
    BDBG_MODULE(some_module)

Input:
    <none>

Returns:
    <none>
****************************************************************************/
void BDBG_Log_SetFifo(
        BDBG_Fifo_Handle fifo /* instance of BDBG_Fifo_Handle */
        );

/***************************************************************************
Summary:
    Extracts and prints data from the debug FIFO

Description:
    WHen this function is called int extracts and prints not more then single 
    line to the debug console. And on success it returns timerout that application 
    should wait prior to calling into the BDBG_Dequeue_Fifo.

See Also:
    BDBG_Log_SetFifo
    BDBG_FifoReader_Create

****************************************************************************/
BERR_Code BDBG_Log_Dequeue(
        BDBG_FifoReader_Handle logReader,  /* Instance of BDBG_FifoReader */
        unsigned *timeout,   /* timeout, in miliseconds, that application should wait prior to calling again into BDBG_Dequeue_Fifo */
        char *str, /* destination buffer */
        size_t str_size, /* size of the buffer */
        size_t *str_len /* [out] length of the string */
        );

/***************************************************************************
Description:
    This function returns size of the element used for storing debug output
****************************************************************************/
void    BDBG_Log_GetElementSize(
        size_t *elementSize /* [out] size of the element in the debug FIFO */
        );

#ifdef __cplusplus
}
#endif

#endif  /* BDBG_LOG_H */
