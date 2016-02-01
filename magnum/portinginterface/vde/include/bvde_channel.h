/***************************************************************************
 *     Copyright (c) 2006-2014, Broadcom Corporation
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
 * Module Description: Video Channel Interface for DSP
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BVDE_CHANNEL_H_
#define BVDE_CHANNEL_H_

#include "bxdm_decoder.h"

/***************************************************************************
Summary:
Channel Handle
***************************************************************************/
typedef struct BVDE_Channel *BVDE_ChannelHandle;

/***************************************************************************
Summary:
Picture frame sizes
***************************************************************************/
typedef enum BVDE_Resolution
{
   BVDE_Resolution_eFullHD,         /* 1920 x 1080 */
   BVDE_Resolution_eHD,             /* 1280 x 720  */
   BVDE_Resolution_ePAL,            /* 720 x 576   */
   BVDE_Resolution_eSD,             /* 720 x 480   */   
   BVDE_Resolution_eCIF,            /* 352  288   */
   BVDE_Resolution_eQCIF,           /* 176  144   */
   BVDE_Resolution_eMaxModes        
} BVDE_Resolution;

/***************************************************************************
Summary:
Channel Open Settings
***************************************************************************/
typedef struct BVDE_ChannelOpenSettings
{

    BMMA_Heap_Handle memPicHandle;
    BVDE_Resolution resolution;       /* Decoder Resolution */
    BAVC_VideoCompressionStd *pCodecList;   /* List of Video compression standards used by this channel */
    uint32_t codecCount;                    /* Number of video compression standards in list */
} BVDE_ChannelOpenSettings;

/***************************************************************************
Summary:
Get default settings for a video channel
***************************************************************************/
void BVDE_Channel_GetDefaultOpenSettings(
    BVDE_ChannelOpenSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Opens a video channel
***************************************************************************/
BERR_Code BVDE_Channel_Open(
    BVDE_Handle deviceHandle,
    unsigned index,
    const BVDE_ChannelOpenSettings *pSettings, 
    BVDE_ChannelHandle *pHandle                 /* [out] */
    );

/***************************************************************************
Summary:
Closes a video channel
***************************************************************************/
void BVDE_Channel_Close(
    BVDE_ChannelHandle handle
    );

/***************************************************************************
Summary:
Start-time settings for a video channel
***************************************************************************/
typedef struct BVDE_ChannelStartSettings
{
    BAVC_VideoCompressionStd codec;         
    const BAVC_XptContextMap *pContextMap;  /* What RAVE context should be read while decoding. */

    /* TBD: Error hanlding Mode might have to be added here */
    
} BVDE_ChannelStartSettings;

/***************************************************************************
Summary:
Get default settings for a video channel start
***************************************************************************/
void BVDE_Channel_GetDefaultStartSettings(
    BVDE_ChannelStartSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Start a video channel
***************************************************************************/
BERR_Code BVDE_Channel_Start(
    BVDE_ChannelHandle handle,
    const BVDE_ChannelStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop a video channel
***************************************************************************/
void BVDE_Channel_Stop(
    BVDE_ChannelHandle handle
    );

/***************************************************************************
Summary:
Provide the Display Manager (DM) public decoder interface function pointers

Description:
The DM mandates that all external video decoders support a list of predefined 
functions. This is provided as a structure of function pointers. The VDE 
implements each of these functions and populates the addresses of these 
functions in this call.
***************************************************************************/
BERR_Code BVDE_Channel_GetDMInterface(
    BVDE_ChannelHandle handle,
    BXDM_Decoder_Interface *pDMInterface,
    void **pContext    
    );


/***************************************************************************
Summary:
Get Default CDB/ITB configuration for decoding
***************************************************************************/
void BVDE_Channel_GetDefaultCdbItbConfig(
    BVDE_ChannelHandle handle,
    BAVC_CdbItbConfig *pConfig  /* [out] */
    );

#endif
