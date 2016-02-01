
/***************************************************************************
 *     Copyright (c) 2004-2012, Broadcom Corporation
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
 *   TBA
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BXVD_DBG_H__
#define BXVD_DBG_H__

#include "bstd.h"                /* standard types */
#include "bxvd.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
   Enum used to stop and start decoder debug logging

Description:
   This enum is used in calls to BXVD_ControlDecoderDebugLog.
   The decoder debug log must be started for the Decoder to write
   debug output to a buffer to be read by XVD. 

See Also:
	BXVD_ControlDecoderDebugLog
****************************************************************************/
typedef enum BXVD_DBG_DebugLogging
{
   BXVD_DBG_DebugLogging_eStop = 0,
   BXVD_DBG_DebugLogging_eStart
} BXVD_DBG_DebugLogging;



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
);


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
);


/***************************************************************************
Summary:
  Read decoder debug log

Description:
  This API copies a maxium uiBufferSize of data from the decoders debug log
  buffer to pLogBuffer. The number of bytes copied to the supplied buffer is
  returned in pBufferCount. 

  If there are no bytes to be read, pBufferCount will be 0.

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
);

#ifdef __cplusplus
}
#endif

#endif /* BXVD_DBG_H__ */
/* End of file. */
