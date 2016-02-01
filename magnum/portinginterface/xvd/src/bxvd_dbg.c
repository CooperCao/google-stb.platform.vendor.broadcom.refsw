
/***************************************************************************
 *     Copyright (c) 2004-2013, Broadcom Corporation
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
 *  Various debug utilities that the application can use to gather 
 *  information about XVD state. 
 *
 *  Decoder debug information can be obtained using the following routines:
 *
 *      BXVD_DBG_ControlDecoderDebugLog
 *      BXVD_DBG_SendDecoderDebugCommand
 *      BXVD_DBG_ReadDecoderDebugLog 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"                /* standard types */
#include "bxvd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_dbg.h"
#include "bxvd_errors.h"

BDBG_MODULE(BXVD_DBG);

#define BXVD_P_SWAP32(a) (((a&0xFF)<<24)|((a&0xFF00)<<8)|((a&0xFF0000)>>8)|((a&0xFF000000)>>24))

/***************************************************************************
Summary:
  Controls decoder debug logging

Description:
  This API controls the logging of decoders outer loop debug output. The debug 
  logging can be started or stopped. 

  Once the debug logging is enabled, the application must call 
  BXVD_DBG_ReadDecoderDebugLog to read the log. The 

  Decoder debug commands can be sent to the decoder using 
  BXVD_SendDecoderDebugCommand. The output of the command would then be 
  read using BXVD_DBG_ReadDecoderDebugLog.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER
  BXVD_ERR_DEBUG_LOG_NOBUFFER  

See Also: BXVD_DBG_ReadDecoderDebugLog, BXVD_DBG_SendDecoderDebugCommand
****************************************************************************/
BERR_Code BXVD_DBG_ControlDecoderDebugLog
(
   BXVD_Handle            hXvd,          /* [in] device handle */
   BXVD_DBG_DebugLogging  eDebugLogging  /* [in] start/stop logging */
)
{
   bool bStart;
   BERR_Code rc;

   BDBG_ENTER(BXVD_DBG_ControlDecoderDebugLog);

   if (hXvd == NULL)
   {
      return BERR_INVALID_PARAMETER;
   }
   
   if (hXvd->uiDecoderDbgBufVirtAddr == 0)
   {
      BXVD_DBG_MSG(hXvd, ("Decoder debug log buffer not allocated!"));
      return BXVD_ERR_DEBUG_LOG_NOBUFFER;
   }

   if (eDebugLogging == BXVD_DBG_DebugLogging_eStart)
   {
      bStart = true;
   }
   else
   {
      bStart = false;
   }

   hXvd->bFWDbgLoggingStarted = bStart;

   rc = BXVD_P_HostCmdDbgLogControl( hXvd, bStart);

   BDBG_LEAVE(BXVD_DBG_ControlDecoderDebugLog);

   return rc;
}

/***************************************************************************
Summary:
  Send decoder debug commands to the decoder.

Description:
  This API sends a command string to the decoder. The command format is the
  same format as used when using the debug prompt via the decoders outer loop
  UART.  

  Decoder debug commands are sent to the decoder using 
  BXVD_DBG_SendDecoderDebugCommand. The output of the command would then be 
  read using BXVD_DBG_ReadDecoderDebugLog.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER
  BXVD_ERR_DEBUG_LOG_NOBUFFER  

See Also: BXVD_DBG_ControlDecoderDebugLog, BXVD_DBG_ReadDecoderDebugLog
****************************************************************************/
BERR_Code BXVD_DBG_SendDecoderDebugCommand
(
   BXVD_Handle hXvd,      /* [in] device handle */   
   char        *pCommand  /* [in] pointer to a null terminated command string */
)
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_DBG_SendDecoderDebugCommand);

   if (hXvd == NULL)
   {
      return BERR_INVALID_PARAMETER;
   }

   if (hXvd->uiDecoderDbgBufVirtAddr == 0)
   {
      BXVD_DBG_MSG(hXvd, ("Decoder debug log buffer not allocated!"));
      return BXVD_ERR_DEBUG_LOG_NOBUFFER;
   }

   rc = BXVD_P_HostCmdDbgLogCommand(hXvd, pCommand);

   BDBG_LEAVE(BXVD_DBG_SendDecoderDebugCommand);

   return rc;
}

/***************************************************************************
Summary:
  Read decoder debug log

Description:
  This API copies a maxium uiBufferSize of data from the decoders debug log
  buffer to pLogBuffer. The number of bytes copied to the supplied buffer is
  returned in pBufferCount. 

  If there are no bytes to be read, pBufferCount will be 0.

  If an overflow of the decoder debug log buffer has occured,  
  BERR_ERR_DEBUG_LOG_OVERFLOW will be returned as the function status.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER
  BXVD_ERR_DEBUG_LOG_OVERFLOW   

See Also: BXVD_DBG_ControlDecoderDebugLog, BXVD_DBG_SendDecoderDebugCommand
****************************************************************************/
BERR_Code BXVD_DBG_ReadDecoderDebugLog
(
   BXVD_Handle hXvd,          /* [in] device handle */   
   char        *pLogBuffer,   /* [in] pointer to buffer where log is copied to */
   uint32_t    uiBufferSize,  /* [in] maximum number of bytes to copy to buffer */
   uint32_t    *pBufferCount  /* [out] number of bytes copied from decoder log */
)
{
   uint32_t * pDbgBufWrPtr;  /* Log ring buffer WR pointer, Decoder managed */
   uint32_t * pDbgBufRdPtr;  /* Log ring buffer RD pointer, XVD managed */

   uint32_t uiBufRdIndex;    /* Last Read char from buffer */
   uint32_t uiBufWrIndex;    /* Last written char in buffer */
   uint32_t uiByteCount;     /* Byte count of log segment to copy */

   uint32_t * pDbgBufPtr;    /* Log ring buffer long word pointer */

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
   uint32_t * pBuf32;        /* Application buffer long word pointer */

   uint32_t ui;
#endif

   uint32_t uiDecoderLogSize; 

   char *pBuf;

   bool bDone;
   bool bBufferOverflow;

   BERR_Code rc;
   
   BDBG_ENTER(BXVD_DBG_ReadDecoderDebugLog);

   *pBufferCount = 0;
   uiByteCount = 0;
   pBuf = pLogBuffer;  

   if (hXvd == NULL)
   {
      return BERR_INVALID_PARAMETER;
   }

   uiDecoderLogSize = (hXvd->stSettings.uiDecoderDebugLogBufferSize / BXVD_P_DBGLOG_ITEM_SIZE);

   if (hXvd->uiDecoderDbgBufVirtAddr == 0)
   {
      BXVD_DBG_MSG(hXvd, ("Debug log buffer not allocated!"));
      return BERR_INVALID_PARAMETER;
   }

   /* Read and write pointers */ 
   pDbgBufRdPtr = (uint32_t *)(hXvd->uiDecoderDbgBufVirtAddr + BXVD_P_DBGLOG_RD_PTR);
   pDbgBufWrPtr = (uint32_t *)(hXvd->uiDecoderDbgBufVirtAddr + BXVD_P_DBGLOG_WR_PTR);

   BMMA_FlushCache(hXvd->hFWDbgBuf_MemBlock, (void *) pDbgBufRdPtr, 8);

   /* Last char written */
   uiBufWrIndex = *pDbgBufWrPtr;

   /* Determine if overflow bit is set */
   bBufferOverflow = ((uiBufWrIndex & BXVD_P_DBGLOG_BUFFER_OVERFLOW) != 0);

   /* Get Wr index */
   uiBufWrIndex &= (~BXVD_P_DBGLOG_BUFFER_OVERFLOW); 

   bDone = false;
   while (!bDone)
   {
      uiBufRdIndex = *pDbgBufRdPtr;
      if (uiBufRdIndex < uiBufWrIndex)
      {
         /* Read pointer less than Write pointer */
         uiByteCount = (uiBufWrIndex  - uiBufRdIndex) * BXVD_P_DBGLOG_ITEM_SIZE;

         if (uiByteCount > uiBufferSize)
         {
            uiByteCount = uiBufferSize;
         }

         pDbgBufPtr = (uint32_t *)(hXvd->uiDecoderDbgBufVirtAddr + (uiBufRdIndex * BXVD_P_DBGLOG_ITEM_SIZE));

         BMMA_FlushCache(hXvd->hFWDbgBuf_MemBlock, pDbgBufPtr, uiByteCount);

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
         BKNI_Memcpy((void *)pBuf,
                     (void *)pDbgBufPtr,
                     uiByteCount);
#else
         pBuf32 = (uint32_t *) pBuf;

         for (ui = 0; ui < uiByteCount/4; ui++)
         {
            *pBuf32 = BXVD_P_SWAP32(*pDbgBufPtr);

            pDbgBufPtr++;
            pBuf32++;
         }
#endif
         /* Update temp app dest buf ptr */
         pBuf += uiByteCount;
         *pDbgBufRdPtr += (uiByteCount / BXVD_P_DBGLOG_ITEM_SIZE);

         *pBufferCount += uiByteCount;

         if ( *pDbgBufRdPtr >= uiDecoderLogSize)
         {
            *pDbgBufRdPtr = BXVD_P_DBGLOG_INITIAL_INDEX;
         }

         /* Rd pointer was less than Wr pointer, so we are done */
         bDone = true; 
      }
      else if (uiBufRdIndex > uiBufWrIndex)
      {
         /* Write pointer back behind read pointer, read to end of buffer */
         uiByteCount = (uiDecoderLogSize - uiBufRdIndex) * BXVD_P_DBGLOG_ITEM_SIZE;

         /* If un-read data size greater then app buffer size, use ap size */
         if (uiByteCount > uiBufferSize)
         {
            uiByteCount = uiBufferSize;
         }

         pDbgBufPtr = (uint32_t *)(hXvd->uiDecoderDbgBufVirtAddr + (uiBufRdIndex * BXVD_P_DBGLOG_ITEM_SIZE));

         BMMA_FlushCache(hXvd->hFWDbgBuf_MemBlock, pDbgBufPtr, uiByteCount);

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
         BKNI_Memcpy((void *)pBuf,
                     (void *)pDbgBufPtr,
                     uiByteCount);
#else
         pBuf32 = (uint32_t *) pBuf;

         for (ui = 0; ui < uiByteCount/4; ui++)
         {
            *pBuf32 = BXVD_P_SWAP32(*pDbgBufPtr);
            pDbgBufPtr++;
            /* Update temp app dest buf ptr */            
            pBuf32++;;
         }
#endif
         /* Update temp app dest buf ptr */
         pBuf += uiByteCount;
         *pDbgBufRdPtr += (uiByteCount / BXVD_P_DBGLOG_ITEM_SIZE);

         *pBufferCount = uiByteCount; 

         /* If end of buffer, reset to beginning */ 
         if ( *pDbgBufRdPtr >= uiDecoderLogSize)
         {
            *pDbgBufRdPtr = BXVD_P_DBGLOG_INITIAL_INDEX;
         }
      }
      else if (uiBufRdIndex == uiBufWrIndex)
      {
         *pBufferCount = uiByteCount;

         /* Buffer is now empty, we are done */
         bDone = true; 
      }

      /* Adjust remaining buf size for next time through loop */ 
      uiBufferSize -= uiByteCount; 

      if (uiBufferSize == 0)
      {
         /* No more space in application supplied buffer, we are done */
         bDone = true; 
      }
   }

   if (bBufferOverflow)
   {
      /* Overflow bit was set, return overflow */
      rc = BXVD_ERR_DEBUG_LOG_OVERFLOW;
   }
   else
   {
      rc = BERR_SUCCESS;
   }

   BDBG_LEAVE(BXVD_DBG_ReadDecoderDebugLog);

   return rc;
}


