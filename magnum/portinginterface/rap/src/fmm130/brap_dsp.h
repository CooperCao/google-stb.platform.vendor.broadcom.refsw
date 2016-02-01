/***************************************************************************
*     Copyright (c) 2004, Broadcom Corporation
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


/*=************************ Module Overview ********************************
The audio DSP is one of the object types of BRAP PI architecture. Each DSP
object is referred to as DSP channel. Physically each DSP object is
the actual DSP hardware processor.

The audio dsp channel API(BRAP_DSP_P/BRAP_DSP) is part of the BRAP PI
implementaton. This module manages a set of  audio DSP channels. These APIs 
deal with the operations carried out on DSP channel (like DSP booting etc).

Each DSP channel is capable of running multiple DSP contexts. Prototypes
of DSP context APIs (BRAP_DSPCHN_P/BRAP_DSPCHN) are in baud_dspchn.h. 
The various DSP contexts are decode, sample rate conversion (SRC), 
pass through etc.

DSP channel APIs (BRAP_DSP_P/BRAP_DSP) and DSP context APIs 
(BRAP_DSPCHN_P/BRAP_DSPCHN) are used by audio manager's decode channel 
APIs (BRAP_DEC).

Design
The design of this module has been broken down into the following sections:

o Part 1

  APIs that are not exposed to the application. These APIs have prefix
  BRAP_DSP_P. These APIs are used by audio manager's decode channel APIs 
  only.

o Part 2

  APIs exposed to the application. These APIs have prefix BRAP_DSP. They
  are implemented to expose DSP object's hardware features to application.
  They are only for unforeseen cases and are provided on need basis.
  These APIs are used by audio manager's decode channel APIs as well as
  by application.
***************************************************************************/

#ifndef BRAP_DSP_H__
#define BRAP_DSP_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	This structure contains DSP settings.

***************************************************************************/
typedef struct BRAP_DSP_Settings
{
	uint32_t	TBD;
} BRAP_DSP_Settings;



#ifdef __cplusplus
}
#endif

#endif /* BRAP_DSP_H__ */
/* End of File */

