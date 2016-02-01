/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BVCE_DEBUG_H_
#define BVCE_DEBUG_H_

/* BVCE_Debug_PrintLogMessageEntry - prints the debug log entry to the console.
 *
 * The following messages can be enabled:
 *  BVCE_DBG_CFG - Encoder Configuration Changes
 *  BVCE_DBG_STS - Encoder Status
 *  BVCE_DBG_BUF - Buffer Descriptors sent by VCE PI
 *  BVCE_DBG_MTA - Metadata Descriptors sent by VCE PI
 *  BVCE_DBG_ITB - Encoder Raw ITB data
 *  BVCE_DBG_CMD - Encoder Commands
 *  BVCE_DBG_RSP - Encoder Responses
 */
void
BVCE_Debug_PrintLogMessageEntry(
      const void *pstFifoEntry /* Should point to an element of size BVCE_Debug_FifoInfo.uiElementSize */
      );

/* BVCE_Debug_FormatLogHeader - formats the entry type (uiIndex) for writing to file.
 * This should be called continuously incrementing uiIndex, starting with uiIndex=0 until
 * the function returns false
 */
bool
BVCE_Debug_FormatLogHeader(
   unsigned uiIndex,
   char *szMessage,
   size_t uiSize
   );

/* BVCE_Debug_FormatLogMessage - formats the debug log entry for writing to file.
 */
void
BVCE_Debug_FormatLogMessage(
   const void *pstFifoEntry, /* Should point to an element of size BVCE_Debug_FifoInfo.uiElementSize */
   char *szMessage,
   size_t uiSize
   );

/* BVCE_Debug_GetEntrySize - returns size of a log entry */
unsigned
BVCE_Debug_GetEntrySize( void );

#endif /* BVCE_DEBUG_H_ */
