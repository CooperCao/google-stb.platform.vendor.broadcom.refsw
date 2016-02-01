/******************************************************************************
* (c) 2004-2015 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef BSID_DBG_H__
#define BSID_DBG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************
/////////////////////// Defines, Typedef, Structs ////////////////////////////////
*********************************************************************************/

/* Enable these to turn on specific debug capability */
/*#define BSID_P_DEBUG_ENABLE_ARC_UART*/
/*#define BSID_P_DEBUG_TRACE_PLAYBACK*/
/*#define BSID_P_DEBUG_SAVE_BUFFER*/
/*#define BSID_P_DEBUG_SAVE_AS_YUV*/ /* else saves as PPM */
/*#define BSID_P_DEBUG_FW_DUMP_TO_FILE*/


#ifdef BSID_P_DEBUG_SAVE_BUFFER
#define BSID_P_CreateSaveImagesThread(ctx) BSID_P_Debug_CreateSaveImagesThread(ctx)
#else
#define BSID_P_CreateSaveImagesThread(ctx)
#endif

/********************************************************************************/

#ifdef BSID_P_DEBUG_SAVE_BUFFER
typedef struct BSID_P_DebugSaveData
{
   pthread_t SaveImagesThread;
} BSID_P_DebugSaveData;
#endif

#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
typedef struct BSID_P_TracePlayback
{
   uint32_t vsync_index;
   uint32_t display_time_current;
   uint32_t itb_update_time;
   uint32_t display_time_previous;
} BSID_P_TracePlayback;
#endif

/*********************************************************************************
////////////////////// Function prototypes declaration ///////////////////////////
*********************************************************************************/

#ifdef BSID_P_DEBUG_SAVE_BUFFER
void BSID_P_Debug_CreateSaveImagesThread(BSID_ChannelHandle  hSidCh);
void BSID_P_Debug_SaveImageData(BSID_ChannelHandle  hSidCh, uint32_t image_index,
    void *output_buffer_address,
    uint32_t output_buffer_width,
    uint32_t output_buffer_pitch, uint32_t output_buffer_height,
    uint32_t input_buffer_offset[2], uint32_t input_buffer_size[2],
    uint32_t num_input_buffer);
#endif

#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
uint32_t BSID_P_Debug_Gettimeus(void);
#endif

#if BDBG_DEBUG_BUILD
void BSID_P_Debug_CheckFirmwareAPI(void);
#define BSID_P_CheckFirmwareAPI() BSID_P_Debug_CheckFirmwareAPI()
#else
#define BSID_P_CheckFirmwareAPI()
#endif

/*********************************************************************************
//////////////////////////////////////////////////////////////////////////////////
*********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BSID_DBG_H__ */

/* end of file */
