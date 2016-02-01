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

#ifndef BVCE_DEBUG_COMMON_H_
#define BVCE_DEBUG_COMMON_H_

#include "bvce_fw_api.h"

typedef struct BVCE_P_CommandDebug
{
      uint32_t uiCommand;
      char *szCommandParameterName[HOST_CMD_BUFFER_SIZE/sizeof(uint32_t)];
      char *szResponseParameterName[HOST_CMD_BUFFER_SIZE/sizeof(uint32_t)];
      size_t uiCommandSize;
      size_t uiResponseSize;
} BVCE_P_CommandDebug;

typedef enum BVCE_DebugFifo_EntryType
{
   BVCE_DebugFifo_EntryType_eConfig, /* Encode Configuration */
   BVCE_DebugFifo_EntryType_eStatus, /* Encode Status */
   BVCE_DebugFifo_EntryType_eBufferDescriptor, /* Buffer Descriptor */
   BVCE_DebugFifo_EntryType_eMetadataDescriptor, /* Metadata Descriptor */
   BVCE_DebugFifo_EntryType_eITB, /* Raw ITB Descriptor */
   BVCE_DebugFifo_EntryType_eCommand, /* VCE FW Command */
   BVCE_DebugFifo_EntryType_eResponse, /* VCE FW Response */
   BVCE_DebugFifo_EntryType_eTrace0, /* VCE Function Trace 0 */
   BVCE_DebugFifo_EntryType_eTrace1, /* VCE Function Trace 1 */

   /* Add new enums ABOVE this line */
   BVCE_DebugFifo_EntryType_eMax
} BVCE_DebugFifo_EntryType;

extern const char* const BVCE_P_StatusLUT[];
extern const char* const BVCE_P_ErrorLUT[];
extern const char* const BVCE_P_EventLUT[];
extern const BVCE_P_CommandDebug BVCE_P_CommandLUT[];
extern const char* const BVCE_P_Debug_EntryTypeLUT[];

extern const unsigned int BVCE_P_CommandLUT_size;

#endif /* BVCE_DEBUG_COMMON_H_ */
