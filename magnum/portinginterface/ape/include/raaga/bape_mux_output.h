/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: MuxOutput
*    Specific APIs related to Audio Transcoder Output
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef BAPE_MUX_OUTPUT_H_
#define BAPE_MUX_OUTPUT_H_

#include "bape.h"
#include "bavc_rap.h"
#include "bavc_xpt.h"
#include "bmma.h"

/***************************************************************************
Summary:
    Mux Output Handle
***************************************************************************/
typedef struct BAPE_MuxOutput *BAPE_MuxOutputHandle;

/***************************************************************************
Summary:
    Creation Settings for a MuxOutput object
***************************************************************************/
typedef struct BAPE_MuxOutputCreateSettings
{
    struct
    {
        BMMA_Heap_Handle descriptor;    /* Heap used for allocating descriptors.
                                           If not set, the heap provided in 
                                           BAPE_Open will be used. */
    } heaps;

    unsigned numDescriptors;            /* Number of descriptors to allocate.
                                           If not set, a default allocation of 
                                           descriptors will be used. */

    const BAVC_XptContextMap *pContextMap;      /* Context data will be sent to */
    struct {
        BMMA_Block_Handle block;
        unsigned size;
    } itb, cdb;
    bool useRDB;                        /* If true we will use the ptrs provided
                                           and not the rave to handle mux output. */
} BAPE_MuxOutputCreateSettings;


/***************************************************************************
Summary:
    Get default settings for a MuxOutput object
***************************************************************************/
void BAPE_MuxOutput_GetDefaultCreateSettings(
    BAPE_MuxOutputCreateSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open a MuxOutput object
***************************************************************************/
BERR_Code BAPE_MuxOutput_Create(
    BAPE_Handle hApe,
    const BAPE_MuxOutputCreateSettings *pSettings,
    BAPE_MuxOutputHandle *pHandle                   /* [out] */
    );

/***************************************************************************
Summary:
    Close a MuxOutput object
    
Description:
    Input must be removed prior to closing.
***************************************************************************/
void BAPE_MuxOutput_Destroy(
    BAPE_MuxOutputHandle hMuxOutput
    );

/***************************************************************************
Summary:
    Start-time settings for a MuxOutput object
***************************************************************************/
typedef struct BAPE_MuxOutputStartSettings
{
    unsigned stcIndex;              /* Which STC index should be used */
    unsigned presentationDelay;     /* Arrival to Presentation Delay in ms */

    const BAVC_Xpt_StcSoftIncRegisters *pNonRealTimeIncrement;  /* If provided, mux output will run 
                                                                   in non-realtime mode and update the
                                                                   soft increment registers provided. 
                                                                   Important: This field requires the 
                                                                   MuxOutput to be started prior to
                                                                   starting any inputs. */
} BAPE_MuxOutputStartSettings;

/***************************************************************************
Summary:
    Get Default Start-time Settings for a MuxOutput object
***************************************************************************/
void BAPE_MuxOutput_GetDefaultStartSettings(
    BAPE_MuxOutputStartSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Start a MuxOutput Object

Description:
    Typically, this is called prior to BAPE_Decoder_Start() in order to
    guarantee all data is captured.

See Also:
    BAPE_MuxOutput_GetDefaultStartSettings
    BAPE_MuxOutput_Stop
***************************************************************************/
BERR_Code BAPE_MuxOutput_Start(
    BAPE_MuxOutputHandle hMuxOutput,
    const BAPE_MuxOutputStartSettings *pSettings
    );

/***************************************************************************
Summary:
    Stop a MuxOutput Object

Description:
    Typically, this is called after BAPE_Decoder_Stop() in order to
    guarantee all data is captured.

See Also:
    BAPE_MuxOutput_Start
***************************************************************************/
void BAPE_MuxOutput_Stop(
    BAPE_MuxOutputHandle hMuxOutput
    );

/***************************************************************************
Summary:
    Get MuxOutput Buffer Descriptors

Description:
    Returns audio buffer descriptors for CDB content in the
    BAVC_AudioBufferDescriptor array(s)
***************************************************************************/
BERR_Code BAPE_MuxOutput_GetBufferDescriptors(
   BAPE_MuxOutputHandle hMuxOutput,
   const BAVC_AudioBufferDescriptor **pBuffer, /* [out] pointer to BAVC_AudioBufferDescriptor structs */
   size_t *pSize,  /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer */
   const BAVC_AudioBufferDescriptor **pBuffer2, /* [out] pointer to BAVC_AudioBufferDescriptor structs after wrap around */
   size_t *pSize2  /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer2 */
   );

/***************************************************************************
Summary:
    Consume MuxOutput Buffer Descriptors

Description:
    Reclaims the specified number of audio buffer descriptors
    The CDB read pointer is updated accordingly
***************************************************************************/
BERR_Code BAPE_MuxOutput_ConsumeBufferDescriptors(
    BAPE_MuxOutputHandle hMuxOutput,
    unsigned numBufferDecsriptors /* must be <= pSize+pSize2 returned by last BAPE_MuxOutput_GetBufferDescriptors call. */
    );

/***************************************************************************
Summary:
    Get MuxOutput Buffer Status

Description:
    Returns the output buffer status (e.g. the base virtual address)
***************************************************************************/
BERR_Code BAPE_MuxOutput_GetBufferStatus(
   BAPE_MuxOutputHandle hMuxOutput,
   BAVC_AudioBufferStatus *pBufferStatus    /* [out] */
   );

/***************************************************************************
Summary:
    Get the audio connector for a MuxOutput object
***************************************************************************/
void BAPE_MuxOutput_GetConnector(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_Connector *pConnector
    );

/***************************************************************************
Summary:
    Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_MuxOutput_AddInput(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
    Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_MuxOutput_RemoveInput(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
    Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_MuxOutput_RemoveAllInputs(
    BAPE_MuxOutputHandle hMuxOutput
    );

/***************************************************************************
Summary:
Audio Mux Output Delay Status
***************************************************************************/
typedef struct BAPE_MuxOutputDelayStatus 
{
    unsigned endToEndDelay; /* End to end delay for the current input and codec in ms */
} BAPE_MuxOutputDelayStatus;

/***************************************************************************
Summary:
Get Audio Mux Output delay 
 
Description: 
This routine will return the path delay status for the audio mux buffer. 
The MuxOutput handle must be connected to the expected data source prior 
to calling this routine in order to determine the correct delay value. 
***************************************************************************/
BERR_Code BAPE_MuxOutput_GetDelayStatus(
    BAPE_MuxOutputHandle hMuxOutput,
    BAVC_AudioCompressionStd codec,
    BAPE_MuxOutputDelayStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Audio Mux Output Interrupt Handlers
***************************************************************************/
typedef struct BAPE_MuxOutputInterruptHandlers
{
    /* This interrupt fires if the output buffers overflow */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } overflow;
} BAPE_MuxOutputInterruptHandlers;

/***************************************************************************
Summary:
Get Audio Mux Output Interrupt Handlers
***************************************************************************/
void BAPE_MuxOutput_GetInterruptHandlers(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_MuxOutputInterruptHandlers *pInterrupts /* [out] */
    );

/***************************************************************************
Summary:
Set Audio Mux Output Interrupt Handlers
***************************************************************************/
BERR_Code BAPE_MuxOutput_SetInterruptHandlers(
    BAPE_MuxOutputHandle hMuxOutput,
    const BAPE_MuxOutputInterruptHandlers *pInterrupts /* [out] */
    );

/***************************************************************************
Summary:
Audio Mux Output Status
***************************************************************************/
typedef struct BAPE_MuxOutputStatus 
{
    unsigned numFrames;         /* Number of successfully encoded frames */
    unsigned numErrorFrames;    /* Number of error frames */
    unsigned numDroppedOverflowFrames; /* Number of frames dropped due to overflow */
    unsigned decodePTS;         /* Decoder Orginal PTS.  This is also populated in BAVC_CompressedBufferDescriptor */
    uint64_t encodePTS;         /* 33-bit PTS value (in 90 Khz) This is also populated in BAVC_CompressedBufferDescriptor */
    unsigned escr;              /* Encoder generated ESCR. This is also populated in BAVC_CompressedBufferDescriptor. */
    uint64_t stcSnapshot;       /* Initial 42-bit STC Snapshot from audio encode.  This is also populated in BAVC_AudioMetadataDescriptor. */
    struct {
        unsigned fifoDepth;                  /* depth in bytes of buffer */
        unsigned fifoSize;                   /* size in bytes of buffer */
    } data, index;
} BAPE_MuxOutputStatus;

/***************************************************************************
Summary:
Get Audio Mux Output delay 
 
Description: 
This routine will return the path delay status for the audio mux buffer. 
The MuxOutput handle must be connected to the expected data source prior 
to calling this routine in order to determine the correct delay value. 
***************************************************************************/
BERR_Code BAPE_MuxOutput_GetStatus(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_MuxOutputStatus *pStatus    /* [out] */
    );

#endif /* !defined BAPE_MUX_OUTPUT_H_ */
