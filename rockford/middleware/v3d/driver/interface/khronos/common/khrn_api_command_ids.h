/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  GPUMonitor Hook
Module   :  API command ids

FILE DESCRIPTION
API command ids
=============================================================================*/

#ifndef __COMMAND_IDS_H__
#define __COMMAND_IDS_H__

typedef enum
{
/* pull the command list from the vc5, so they are in sync */
#include "api_command_ids.inc"
} eGLCommand;

#endif /* __COMMAND_IDS_H__ */
