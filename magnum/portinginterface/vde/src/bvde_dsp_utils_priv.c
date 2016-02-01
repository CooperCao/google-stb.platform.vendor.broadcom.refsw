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
#include "bvde.h"
#include "bvde_priv.h"
#include "bdsp_raaga.h"
#include "bvde_dsp_utils_priv.h"

BDBG_MODULE(bvde_dsp_utils);

static const BVDE_CodecAttributes g_codecAttributes[] =
{
/*   AVC Codec                              DSP Type                        Name            */
    {BAVC_VideoCompressionStd_eVP6,         BDSP_Algorithm_eVp6Decode,      "VP6",          },
    
    /* This entry must be last */
    {BAVC_VideoCompressionStd_eMax,         BDSP_Algorithm_eMax,            "Unknown",      }
};

const BVDE_CodecAttributes *BVDE_P_GetCodecAttributes(
    BAVC_VideoCompressionStd codec
    )
{
    unsigned i, tableSize;

    tableSize = sizeof(g_codecAttributes)/sizeof(BVDE_CodecAttributes);

    for ( i = 0; i < tableSize; i++ )
    {
        if ( codec == g_codecAttributes[i].codec )
        {
            return &g_codecAttributes[i];
        }
    }

    return &g_codecAttributes[tableSize-1];
}


bool BVDE_DSP_P_AlgorithmSupported(BVDE_Handle handle, BDSP_Algorithm algorithm)
{
    BDSP_AlgorithmInfo algoInfo;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BVDE_Device);

    errCode = BDSP_GetAlgorithmInfo(handle->dspHandle, algorithm, &algoInfo);
    if ( errCode || !algoInfo.supported )
    {
        return false;
    }

    return true;
}


