/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* API Description:
*   API name: FMT
*    Audio Format Related Routines
*
***************************************************************************/
#include "bape_priv.h"

BDBG_MODULE(bape_fmt_priv);

/***************************************************************************
Summary:
Initialize a format descriptor
***************************************************************************/
void BAPE_FMT_P_InitDescriptor(
    BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    BKNI_Memset(pDesc, 0, sizeof(BAPE_FMT_Descriptor));
    pDesc->source = BAPE_DataSource_eMax;
    pDesc->type = BAPE_DataType_eMax;
}

/***************************************************************************
Summary:
Get the number of channel pairs required from a format descriptor
***************************************************************************/
unsigned BAPE_FMT_P_GetNumChannelPairs_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    switch ( pDesc->type )
    {
    case BAPE_DataType_ePcm5_1:
        return 3;
    case BAPE_DataType_ePcm7_1:
        return 4;
    default:
        return 1;
    }
}

/***************************************************************************
Summary:
Get the number of channels required from a format descriptor
***************************************************************************/
unsigned BAPE_FMT_P_GetNumChannels_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    switch ( pDesc->type )
    {
    case BAPE_DataType_eRave:
    case BAPE_DataType_ePcmStereo:
        return 2;
    case BAPE_DataType_ePcm5_1:
        return 6;
    case BAPE_DataType_ePcm7_1:
        return 8;
    default:
        return 1;
    }
}


/***************************************************************************
Summary:
Convert a BAPE_DataType enum to a text string.
***************************************************************************/
const char *BAPE_FMT_P_TypeToString_isrsafe(
    const BAPE_DataType type
    )
{
    switch ( type )
    {
    case BAPE_DataType_ePcmMono:
        return "PCM Mono";
    case BAPE_DataType_ePcmStereo:
        return "PCM Stereo";
    case BAPE_DataType_ePcm5_1:
        return "PCM 5.1";
    case BAPE_DataType_ePcm7_1:
        return "PCM 7.1";
    case BAPE_DataType_ePcmRf:
        return "RF Encoded PCM";
    case BAPE_DataType_eIec61937:
        return "IEC-61937 Compressed";
    case BAPE_DataType_eIec61937x4:
        return "IEC-61937 Compressed 4x Rate";
    case BAPE_DataType_eIec61937x16:
        return "IEC-61937 HBR Compressed 16x Rate";
    case BAPE_DataType_eIec60958Raw:
        return "IEC-60958 RAW";
    case BAPE_DataType_eCompressedRaw:
        return "Compresed RAW";
    case BAPE_DataType_eRave:
        return "RAVE";
    case BAPE_DataType_eUnknown:
        return "Unknown";
    default:
        return "Invalid";
    }
}

/***************************************************************************
Summary:
Get the name of a data type from a format descriptor
***************************************************************************/
const char *BAPE_FMT_P_GetTypeName_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);

    return BAPE_FMT_P_TypeToString_isrsafe(pDesc->type);
}

/***************************************************************************
Summary:
Convert a BAPE_DataSource enum to a text string.
***************************************************************************/
const char *BAPE_FMT_P_SourceToString_isrsafe(
    const BAPE_DataSource source
    )
{
    switch ( source )
    {
    case BAPE_DataSource_eDspBuffer:
        return "DSP Buffer";
    case BAPE_DataSource_eHostBuffer:
        return "Host Buffer";
    case BAPE_DataSource_eDfifo:
        return "DFIFO";
    case BAPE_DataSource_eFci:
        return "FCI";
    case BAPE_DataSource_eRave:
        return "RAVE";
    default:
        return "Unknown";
    }
}

/***************************************************************************
Summary:
Get the name of a data source from a format descriptor
***************************************************************************/
const char *BAPE_FMT_P_GetSourceName_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    
    return BAPE_FMT_P_SourceToString_isrsafe(pDesc->source);
}

#if !B_REFSW_MINIMAL
/***************************************************************************
Summary:
Get the name of a subtype's type from a format descriptor
***************************************************************************/
const char *BAPE_FMT_P_GetSubTypeTypeName_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);

    if (BAPE_FMT_P_GetAudioCompressionStd_isrsafe(pDesc) != BAVC_AudioCompressionStd_eMax)   return "Codec";
    else if (BAPE_FMT_P_GetRfAudioEncoding_isrsafe(pDesc) != BAPE_RfAudioEncoding_eMax)      return "RF Aud Enc";
    else                                                                             return "SubType";
}
#endif

/***************************************************************************
Summary:
Get the name of a subtype from a format descriptor
***************************************************************************/
const char *BAPE_FMT_P_GetSubTypeName_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);

    if (BAPE_FMT_P_GetAudioCompressionStd_isrsafe(pDesc) != BAVC_AudioCompressionStd_eMax)   return BAPE_P_GetCodecName(BAPE_FMT_P_GetAudioCompressionStd_isrsafe(pDesc)) ;
    else if (BAPE_FMT_P_GetRfAudioEncoding_isrsafe(pDesc) == BAPE_RfAudioEncoding_eBtsc)     return "RF Aud Enc: BTSC" ;
    else                                                                             return "N/A";
}

/***************************************************************************
Summary:
Determine if a data type is linear pcm
***************************************************************************/
bool BAPE_FMT_P_IsLinearPcm_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    switch ( pDesc->type )
    {
    case BAPE_DataType_ePcmMono:
    case BAPE_DataType_ePcmStereo:
    case BAPE_DataType_ePcm5_1:
    case BAPE_DataType_ePcm7_1:
        return true;
    default:
        return false;
    }
}

/***************************************************************************
Summary:
Determine if a data type is compressed
***************************************************************************/
bool BAPE_FMT_P_IsCompressed_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    return !BAPE_FMT_P_IsLinearPcm_isrsafe(pDesc);
}

/***************************************************************************
Summary:
Determine if a data type is compressed and content is DTS-CD
***************************************************************************/
bool BAPE_FMT_P_IsDtsCdCompressed_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    return (BAPE_FMT_P_IsCompressed_isrsafe(pDesc) && (BAPE_FMT_P_GetAudioCompressionStd_isrsafe(pDesc) == BAVC_AudioCompressionStd_eDtsLegacy));
}

/***************************************************************************
Summary:
Determine if a data type is 16x compressed mode
***************************************************************************/
bool BAPE_FMT_P_IsHBR_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    return (pDesc->type == BAPE_DataType_eIec61937x16);
}

/***************************************************************************
Summary:
Determine if ramping should be enabled for a particular data type
***************************************************************************/
bool BAPE_FMT_P_RampingValid_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    return BAPE_FMT_P_IsLinearPcm_isrsafe(pDesc);
}

/***************************************************************************
Summary:
Get the current codec from a format descriptor
***************************************************************************/
BAVC_AudioCompressionStd BAPE_FMT_P_GetAudioCompressionStd_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    switch ( pDesc->type )
    {
    case BAPE_DataType_eIec61937:
    case BAPE_DataType_eIec61937x4:
    case BAPE_DataType_eIec61937x16:
    case BAPE_DataType_eCompressedRaw:
        return pDesc->subType.codec;
    default:
        return BAVC_AudioCompressionStd_eMax;
    }
}

/***************************************************************************
Summary:
Get the current RF encoding from a format descriptor
***************************************************************************/
BAPE_RfAudioEncoding BAPE_FMT_P_GetRfAudioEncoding_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    switch ( pDesc->type )
    {
    case BAPE_DataType_ePcmRf:
        return pDesc->subType.rfEncoding;
    default:
        return BAPE_RfAudioEncoding_eMax;
    }
}

/***************************************************************************
Summary:
Set the current codec in a format descriptor
***************************************************************************/
void BAPE_FMT_P_SetAudioCompressionStd_isrsafe(
    BAPE_FMT_Descriptor *pDesc, 
    BAVC_AudioCompressionStd codec
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    pDesc->subType.codec = codec;
}

/***************************************************************************
Summary:
Set the current RF encoding in a format descriptor
***************************************************************************/
void BAPE_FMT_P_SetRfAudioEncoding(
    BAPE_FMT_Descriptor *pDesc, 
    BAPE_RfAudioEncoding encoding
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    pDesc->subType.rfEncoding = encoding;
}


/***************************************************************************
Summary:
Determine if two format descriptors are identical
***************************************************************************/
bool BAPE_FMT_P_IsEqual_isrsafe(
    const BAPE_FMT_Descriptor *pDesc1, 
    const BAPE_FMT_Descriptor *pDesc2
    )
{
    if ( pDesc1->source == pDesc2->source &&
         pDesc1->type == pDesc2->type &&
         pDesc1->sampleRate == pDesc2->sampleRate &&
         pDesc1->ppmCorrection == pDesc2->ppmCorrection )
    {
        if ( pDesc1->type == BAPE_DataType_ePcmRf &&
             BAPE_FMT_P_GetRfAudioEncoding_isrsafe(pDesc1) == BAPE_FMT_P_GetRfAudioEncoding_isrsafe(pDesc2) )
        {
            return true;
        }
        if ( (pDesc1->type == BAPE_DataType_eIec61937 ||
             pDesc1->type == BAPE_DataType_eIec61937x4 ||
             pDesc1->type == BAPE_DataType_eIec61937x16 ||
             pDesc1->type == BAPE_DataType_eCompressedRaw) &&
             BAPE_FMT_P_GetAudioCompressionStd_isrsafe(pDesc1) == BAPE_FMT_P_GetAudioCompressionStd_isrsafe(pDesc2) )
        {
            return true;
        }
    }
    return false;
}

#if BAPE_DSP_SUPPORT
BDSP_DataType BAPE_FMT_P_GetDspDataType_isrsafe(
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL!=pDesc);
    switch ( pDesc->type )
    {
    case BAPE_DataType_ePcmMono:
        return BDSP_DataType_ePcmMono;
    case BAPE_DataType_ePcmStereo:
        return BDSP_DataType_ePcmStereo;
    case BAPE_DataType_ePcm5_1:
        return BDSP_DataType_ePcm5_1;
    case BAPE_DataType_ePcm7_1:
        return BDSP_DataType_ePcm7_1;
    case BAPE_DataType_ePcmRf:
        return BDSP_DataType_ePcmRf;
    case BAPE_DataType_eIec61937:
        return BDSP_DataType_eIec61937;
    case BAPE_DataType_eIec61937x4:
        return BDSP_DataType_eIec61937x4;
    case BAPE_DataType_eIec61937x16:
        return BDSP_DataType_eIec61937x16;
    case BAPE_DataType_eCompressedRaw:
        return BDSP_DataType_eCompressedRaw;
    default:
        return BDSP_DataType_eMax;
    }
}
#endif

/***************************************************************************
Summary:
Initialize Capabilities
***************************************************************************/
void BAPE_FMT_P_InitCapabilities(
    BAPE_FMT_Capabilities *pCaps, 
    const BAPE_DataSource *pSources,    /* Optional.  Pass NULL for no default sources. */
    const BAPE_DataType *pTypes         /* Optional.  Pass NULL for no default types. */
    )
{
    BDBG_ASSERT(NULL!=pCaps);
    BKNI_Memset(pCaps, 0, sizeof(BAPE_FMT_Capabilities));
    if ( pSources )
    {
        while ( *pSources != BAPE_DataSource_eMax )
        {
            BAPE_FMT_P_EnableSource(pCaps, *pSources++);
        }
    }
    if ( pTypes )
    {
        while ( *pTypes != BAPE_DataType_eMax )
        {
            BAPE_FMT_P_EnableType(pCaps, *pTypes++);
        }
    }
}

/***************************************************************************
Summary:
Enable an input data type
***************************************************************************/
void BAPE_FMT_P_EnableType(
    BAPE_FMT_Capabilities *pCaps, 
    BAPE_DataType type
    )
{
    BDBG_ASSERT(NULL != pCaps);
    BDBG_ASSERT(type < BAPE_DataType_eMax);
    pCaps->typeMask[type/32] |= 1UL<<(type%32);
}

/***************************************************************************
Summary:
Disable an input data type
***************************************************************************/
void BAPE_FMT_P_DisableType(
    BAPE_FMT_Capabilities *pCaps, 
    BAPE_DataType type
    )
{
    BDBG_ASSERT(NULL != pCaps);
    BDBG_ASSERT(type < BAPE_DataType_eMax);
    pCaps->typeMask[type/32] &= ~(1UL<<(type%32));
}

/***************************************************************************
Summary:
Enable an input data source
***************************************************************************/
void BAPE_FMT_P_EnableSource(
    BAPE_FMT_Capabilities *pCaps, 
    BAPE_DataSource source
    )
{
    BDBG_ASSERT(NULL != pCaps);
    BDBG_ASSERT(source < BAPE_DataSource_eMax);
    pCaps->sourceMask[source/32] |= 1UL<<(source%32);
}

/***************************************************************************
Summary:
Disable an input data source
***************************************************************************/
void BAPE_FMT_P_DisableSource(
    BAPE_FMT_Capabilities *pCaps, 
    BAPE_DataSource source
    )
{
    BDBG_ASSERT(NULL != pCaps);
    BDBG_ASSERT(source < BAPE_DataSource_eMax);
    pCaps->sourceMask[source/32] &= ~(1UL<<(source%32));
}

/***************************************************************************
Summary:
Check if a format descriptor is valid based on a set of capabilities
***************************************************************************/
bool BAPE_FMT_P_FormatSupported_isrsafe(
    const BAPE_FMT_Capabilities *pCaps, 
    const BAPE_FMT_Descriptor *pDesc
    )
{
    BDBG_ASSERT(NULL != pCaps);
    BDBG_ASSERT(NULL != pDesc);
    if ( BAPE_FMT_P_TypeSupported_isrsafe(pCaps, pDesc->type) &&
         BAPE_FMT_P_SourceSupported_isrsafe(pCaps, pDesc->source) )
    {
        return true;
    }
    return false;
}

/***************************************************************************
Summary:
Check if a data type is valid based on a set of capabilities
***************************************************************************/
bool BAPE_FMT_P_TypeSupported_isrsafe(
    const BAPE_FMT_Capabilities *pCaps, 
    BAPE_DataType type
    )
{
    BDBG_ASSERT(NULL != pCaps);
    BDBG_ASSERT(type < BAPE_DataType_eMax);
    return ( pCaps->typeMask[type/32] & (1UL<<(type%32)) ) ? true : false;
}

/***************************************************************************
Summary:
Check if a data source is valid based on a set of capabilities
***************************************************************************/
bool BAPE_FMT_P_SourceSupported_isrsafe(
    const BAPE_FMT_Capabilities *pCaps, 
    BAPE_DataSource source
    )
{
    BDBG_ASSERT(NULL != pCaps);
    BDBG_ASSERT(source < BAPE_DataSource_eMax);
    return ( pCaps->sourceMask[source/32] & (1UL<<(source%32)) ) ? true : false;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
Summary:
Do a multi-line print out of two BAPE_FMT_Descriptors, showing changes 
between them.  Since this is a function, the appropriate BDBG level must be
enabled for BDBG's BAPE_FMT_PRIV module (not the caller's module).  To use 
the caller's BDBG module, use the macro version: BAPE_FMT_P_LOG_FMT_CHANGE().
***************************************************************************/
void BAPE_FMT_P_LogFmtChange(
    BDBG_Level    bDbgLevel,
    const BAPE_FMT_Descriptor *pOld, 
    const BAPE_FMT_Descriptor *pNew
    )
{
    switch (bDbgLevel )
    {
        default:
        case BDBG_eMsg:     BAPE_FMT_P_LOG_FMT_CHANGE(BDBG_MSG, pOld, pNew);   break;
        case BDBG_eWrn:     BAPE_FMT_P_LOG_FMT_CHANGE(BDBG_WRN, pOld, pNew);   break;
        case BDBG_eErr:     BAPE_FMT_P_LOG_FMT_CHANGE(BDBG_ERR, pOld, pNew);   break;
        case BDBG_eLog:     BAPE_FMT_P_LOG_FMT_CHANGE(BDBG_LOG, pOld, pNew);   break;
    }
}
#endif
