/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 #include "bvee_dsp_utils_priv.h"

 BDBG_MODULE(bvee_dsp_utils);
 
 static const BVEE_CodecAttributes g_codecAttributes[] =
 {
    /*   AVC Codec                              DSP Type            Name            */
#if 	BDSP_ENCODER_ACCELERATOR_SUPPORT
	{BAVC_VideoCompressionStd_eH264,   BDSP_Algorithm_eX264Encode, "X264", },
#else
     {BAVC_VideoCompressionStd_eH264,   BDSP_Algorithm_eH264Encode, "H264", },
#endif
     /* This entry must be last */
     {BAVC_VideoCompressionStd_eMax,    BDSP_Algorithm_eMax,        "Unknown",      }
 };

 const BVEE_CodecAttributes *BVEE_P_GetCodecAttributes(
    BAVC_VideoCompressionStd codec
    )
 {
     unsigned i, tableSize;
 
     tableSize = sizeof(g_codecAttributes)/sizeof(BVEE_CodecAttributes);
 
     for ( i = 0; i < tableSize; i++ )
     {
         if ( codec == g_codecAttributes[i].codec )
         {
             return &g_codecAttributes[i];
         }
     }
 
     return &g_codecAttributes[tableSize-1];
 }
 
