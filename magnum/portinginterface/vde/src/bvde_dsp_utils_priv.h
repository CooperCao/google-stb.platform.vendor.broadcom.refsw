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
 * Module Description: DSP Utility Functions
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bvde_priv.h"
#include "bdsp.h"
#include "bdsp_task.h"
#include "bdsp_audio_task.h"

#ifndef BVDE_DSP_UTILS_PRIV_H_
#define BVDE_DSP_UTILS_PRIV_H_

/***************************************************************************
Summary:
Codec Attribute Table Entry
***************************************************************************/
typedef struct BVDE_CodecAttributes
{
    BAVC_VideoCompressionStd codec;
    BDSP_Algorithm decodeAlgorithm;
    const char *pName;
} BVDE_CodecAttributes;

/***************************************************************************
Summary:
Get Codec Attributes
***************************************************************************/
const BVDE_CodecAttributes *BVDE_P_GetCodecAttributes(
    BAVC_VideoCompressionStd codec
    );


#define BVDE_P_GetCodecName(codec) (BVDE_P_GetCodecAttributes((codec))->pName)
#define BVDE_P_GetCodecVideoDecode(codec) (BVDE_P_GetCodecAttributes((codec))->decodeAlgorithm)


/***************************************************************************
Summary:
Helper to print a variable name and value when assigning DSP settings structures
***************************************************************************/
#define BVDE_DSP_P_SET_VARIABLE(st,var,val) do { st.var = (val); BDBG_MSG(("%s: %s = %#x", __FUNCTION__, #var, (st.var))); } while (0)

/***************************************************************************
Summary:
Check if an algorithm is supported
***************************************************************************/
bool BVDE_DSP_P_AlgorithmSupported(BVDE_Handle handle, BDSP_Algorithm algorithm);
#endif
