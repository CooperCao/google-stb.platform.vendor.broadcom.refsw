/***************************************************************************
 * Copyright (C) 2003-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ***************************************************************************/
#ifndef _BVEE_CHANNEL_
#define _BVEE_CHANNEL_

#include "bavc.h"
#include "bavc_vee.h"
#include "budp_vce.h"

/***************************************************************************
Summary:
VEE Channel Handle
***************************************************************************/
typedef struct BVEE_Channel *BVEE_ChannelHandle;


/***************************************************************************
Summary:
Live Transcode for video phone: 640x480 @ 15fps
Live Transcode for iPad: 440x224 @ 30fps
Offline Transcode: 1280x720 @ 30fps 1/6 X real time 
                    640x360 @ 30fps 1~0.7 X real time
***************************************************************************/
typedef struct BVEE_Resolution
{
   unsigned width;
   unsigned height;
} BVEE_Resolution;

typedef enum
{
   BVEE_ChromaSampling_e420,
   BVEE_ChromaSampling_e422,
   BVEE_ChromaSampling_e444,
   BVEE_ChromaSampling_eLast,
   BVEE_ChromaSampling_eInvalid               = 0x7FFFFFFF
}BVEE_ChromaSampling;

typedef enum
{
    BVEE_VideoH264Profile_eBaseline		                            = 66,
    BVEE_VideoH264Profile_eMain									    = 77,
    BVEE_VideoH264Profile_eExtended								    = 88,
    BVEE_VideoH264Profile_eFRExtProfileHigh						    = 100,							
    BVEE_VideoH264Profile_eFRExtProfileHigh10							= 110,	
    BVEE_VideoH264Profile_eFRExtProfileHigh422					    = 122,	
    BVEE_VideoH264Profile_eFRExtProfileHigh444					    = 244,	
    BVEE_VideoH264Profile_eFRExtProfileHighCavlc444					= 44	
}BVEE_VideoH264ProfileIdc;

typedef enum
{
    BVEE_VideoEncodeMode_eHighDelay   				    = 0x0,      
    BVEE_VideoEncodeMode_eLowDelay    				    = 0x1,      
    BVEE_VideoEncodeMode_eAfap         				    = 0x2,
    BVEE_VideoEncodeMode_eInvalid		                = 0x7FFFFFFF
}BVEE_VideoEncodeMode;

typedef enum
{
    BVEE_VideoGopStruct_eIOnly		       			    = 0x0,      
    BVEE_VideoGopStruct_eIP			       			    = 0x1,      
    BVEE_VideoGopStruct_eIPB   							= 0x2,
    BVEE_VideoGopStruct_eIPBB			       			    = 0x3,
    BVEE_VideoGopStruct_eMax,
    BVEE_VideoGopStruct_eInvalid					        = 0x7FFFFFFF
}BVEE_VideoGopStruct;

typedef enum
{
   BVEE_AspectRatio_eUnknown = 0,
   BVEE_AspectRatio_eSquare,
   BVEE_AspectRatio_e12_11,
   BVEE_AspectRatio_e10_11,
   BVEE_AspectRatio_e16_11,
   BVEE_AspectRatio_e40_33,
   BVEE_AspectRatio_e24_11,
   BVEE_AspectRatio_e20_11,
   BVEE_AspectRatio_e32_11,
   BVEE_AspectRatio_e80_33,
   BVEE_AspectRatio_e18_11,
   BVEE_AspectRatio_e15_11,
   BVEE_AspectRatio_e64_33,
   BVEE_AspectRatio_e160_99,
   BVEE_AspectRatio_e4_3,
   BVEE_AspectRatio_e3_2,
   BVEE_AspectRatio_e2_1,
   BVEE_AspectRatio_e16_9,
   BVEE_AspectRatio_e221_1,
   BVEE_AspectRatio_eLast,
   BVEE_AspectRatio_eInvalid      = 0x7FFFFFFF
}BVEE_AspectRatio;

/* User Data */
#define BVEE_FW_P_UserData_Payload_CC_608_BYTES_PER_LINE_MAX 2
#define BVEE_FW_P_UserData_Payload_CC_608_LINES_PER_FIELD_MAX 5
#define BVEE_FW_P_UserData_Payload_CC_608_LENGTH ( 1 + ( 1 + BVEE_FW_P_UserData_Payload_CC_608_BYTES_PER_LINE_MAX ) * BVEE_FW_P_UserData_Payload_CC_608_LINES_PER_FIELD_MAX )


#define BVEE_FW_P_UserData_Payload_CC_708_BYTES_PER_LINE_MAX 2
#define BVEE_FW_P_UserData_Payload_CC_708_LINES_PER_FIELD_MAX 32


#define BVEE_FW_P_UserData_Payload_CC_708_LENGTH ( 1 + ( 1 + BVEE_FW_P_UserData_Payload_CC_708_BYTES_PER_LINE_MAX ) * BVEE_FW_P_UserData_Payload_CC_708_LINES_PER_FIELD_MAX )

#define BVEE_FW_P_UserData_Payload_CC_MAX_LENGTH ( BVEE_FW_P_UserData_Payload_CC_608_LENGTH + BVEE_FW_P_UserData_Payload_CC_708_LENGTH )

#define BVEE_FW_P_USERDATA_QUEUE_LENGTH 16

typedef enum BVEE_FW_UserData_PacketType
{
        BVEE_FW_P_UserData_PacketType_eSCTE_20 = 0,
        BVEE_FW_P_UserData_PacketType_eSCTE_21,
        BVEE_FW_P_UserData_PacketType_eATSC_A53,

        /* Add new user data encapsulation type ABOVE this line */
        BVEE_FW_P_UserData_PacketType_eMax
} BVEE_FW_UserData_PacketType;

/* BVEE_FW_P_UserData_PacketDescriptor contains the information necessary to create a single user data packet of ePacketType */
#define BVEE_FW_P_UserData_PacketDescriptor_MAX_LENGTH ( 4 + BVEE_FW_P_UserData_Payload_CC_MAX_LENGTH )

/***************************************************************************
Summary:
VEE channel open settings
***************************************************************************/
typedef struct BVEE_ChannelOpenSettings
{
    /* JDG: Capture buffers should not be allocated by the PI.  Only reference frame buffers. */
    BMMA_Heap_Handle bufferHeap;             /* Heap to allocate any reference buffers required by this channel.  
                                           If NULL, the heap passed to BVEE_Open will be used. */
    BAVC_VideoCompressionStd codec;   /* List of Video compression standards used by this channel */

    BVEE_Resolution resolution;         /* Maximum Encoder Resolution */

    unsigned maxQueuedPictures;         /* Maximum number of buffers that may be queued at any time.
                                           Requests to encode more buffers will result in an error returned
                                           from BVEE_Channel_EnqueuePicture */

    bool enableExternalTrigger;         /* If true, an external vsync trigger will be enabled.  When this is true,
                                           the register defined in BVEE_ChannelTriggerInfo must be written with the
                                           appropriate value each vsync.  If false (default), the encoder will automatically
                                           be triggered each time a BAVC_Channel_EnqueuePicture() is called. */
}BVEE_ChannelOpenSettings;

/***************************************************************************
Summary:
Get default open settings for a video channel
***************************************************************************/
 void BVEE_Channel_GetDefaultOpenSettings(
    BVEE_ChannelOpenSettings *pSettings             /* [out] */
    ); 

/***************************************************************************
Summary:
Open an instance of video encoding
***************************************************************************/
 BERR_Code BVEE_Channel_Open(
    BVEE_Handle deviceHandle,
    unsigned index,
    const BVEE_ChannelOpenSettings *pSettings,      /* Pass NULL for default settings */
    BVEE_ChannelHandle *pHandle                     /* [out] Returned Handle */
    );

/***************************************************************************
Summary:
Close an instance of video encoding
***************************************************************************/
 BERR_Code BVEE_Channel_Close(
    BVEE_ChannelHandle handle
    );
/***************************************************************************
Summary: 
	The structure contains all information regarding soft external interrupts to DSP 

Description:
	This structure contains configuration info of soft external interrupts to DSP.

See Also:
	None.
****************************************************************************/
typedef struct BVEE_ExtIntrptConfig
{
	/* If the dsp task will be interrupted by external client */
	bool	                enableInterrupts;
	uint32_t				numInterrupts;
	/* only numInterrupts of following struct will be valid */
	struct
	{
		/* ESR_SI register address. Full 32bit address */
		uint32_t				interruptRegAddr;
	    /* Trigger bit in the above register. Bit count [0...31]*/
		uint32_t				interruptBit;
	}interruptInfo[BDSP_MAX_EXT_INTERRUPT_PER_TASK];

}BVEE_ExtIntrptConfig;
/***************************************************************************
Summary:
VEE Channel settings
***************************************************************************/
typedef struct BVEE_ChannelStartSettings
{
    BAVC_VideoCompressionStd codec;         
    const BAVC_XptContextMap *pContextMap;  /* What RAVE context should be written while encoding. */
    bool                    nonRealTime;    /* Encode Mode.  Set to true for offline transcode and false otherwise. */
    BVEE_AspectRatio		eAspectRatio;
    BAVC_FrameRateCode      frameRate;
    BAVC_YCbCrType          pxlformat;
    BVEE_VideoH264ProfileIdc    eProfileIDC;
    uint32_t					ui32LevelIdc;			/* ranges from 9 to 51. For SD it is 30 */
    BVEE_VideoEncodeMode	 	eMode;					/* (High Delay, Low Delay, AFAP)	 */
    uint32_t 					ui32TargetBitRate;			/* Number of bits per sec.	*/
    uint32_t 					ui32EncodPicWidth;
    uint32_t 					ui32EncodPicHeight;
    uint32_t					ui32IntraPeriod;
    uint32_t					ui32IDRPeriod;
    BVEE_VideoGopStruct			eGopStruct;
    bool						bDblkEnable;
    bool                        bSubPelMvEnable;
    bool                        bVarModeOptEnable;
    bool						bRateControlEnable;    
    uint32_t					ui32End2EndDelay;
    bool                        sendMetadata;
    bool                        sendEos;
    BVEE_ExtIntrptConfig        extIntCfg;
    int                         stcIndx;
    bool                        bSendCC;
    BVEE_FW_UserData_PacketType ccPacketType;
}BVEE_ChannelStartSettings;

/***************************************************************************
Summary:
Get default start settings for a video channel
***************************************************************************/ 
 void BVEE_Channel_GetDefaultStartSettings(
    BVEE_ChannelStartSettings *settings
    ); 

/***************************************************************************
Summary:
Start video encoding
***************************************************************************/
 BERR_Code BVEE_Channel_Start(
    BVEE_ChannelHandle handle,
    const BVEE_ChannelStartSettings *psettings
    );

/***************************************************************************
Summary:
Stop Video encoding
***************************************************************************/
 void BVEE_Channel_Stop(
    BVEE_ChannelHandle handle
    );

/***************************************************************************
Summary:
Description of a picture buffer
***************************************************************************/ 
typedef struct BVEE_PictureDescriptor
{    
    BAVC_FrameRateCode  frameRate;          /* Picture frame rate */
    BAVC_PictureCoding  pictureCoding;      /* Picture Type */
    BAVC_Polarity       polarity;           /* Picture Polarity */
    unsigned            sarHorizontal;      /* Sample Aspect Ratio horizontal value (SAR = sarHorizontal/sarVertical) */
    unsigned            sarVertical;        /* Sample Aspect Ratio vertical value (SAR = sarHorizontal/sarVertical) */
    bool                repeat;             /* This indicates the picture repeat known by the caller.  This flag applies 
                                               to both interlaced and progressive sequences.  For interlaced, repeated pictures
                                               have the same polarity as the previous picture.*/
    bool                ignore;             /* This flag indicates an unintentional picture repeat due to decoder underflow in 
                                               non-realtime transcode mode.  The encoder will drop any pictures with this flag set,
                                               timestamps will not be adjusted and it will have no effect in the coded stream.  */
    BAVC_PTSInfo        originalPts;        /* Original PTS value */
    uint32_t            id;                 /* Picture ID.  A 32-bit counter value generated by the caller to identify pictures. */    
    BMMA_Block_Handle   hImageMmaBlock;     /* Raw picture Mma block */
    unsigned            offset;             /* Raw picture hardware address */
    unsigned            height;             /* Encoded picture height*/
    unsigned            width;              /* Encoded picture width*/
    uint32_t            STC_Lo;             /* STC_Lo Timestamp when picture captured*/
    bool                bIgnorePicture;     /* This flag indicates to encoder to ignore pictures */
    bool                bStallStc;          /* This flag indicates to encoder that the STC has been stalled */

#if BDSP_ENCODER_ACCELERATOR_SUPPORT
    /* encode picture Y/C buffer */
    BMMA_Block_Handle   hLumaBlock;
    uint32_t            ulLumaOffset;
    BMMA_Block_Handle   hChromaBlock;
    uint32_t            ulChromaOffset;

    bool                bStriped;           /* Y/C buffer format: 420; linear or striped */

    /* if striped format, the stripe parameters */
    unsigned            ulStripeWidth;
    unsigned            ulLumaNMBY;
    unsigned            ulChromaNMBY;

    BAVC_Polarity       ePolarity;          /* picture polarity: T/B/F */

    uint32_t            ulSTCSnapshotLo;    /* lower 32-bit STC snapshot when picture received at the displayable point (in 27Mhz) */
    uint32_t            ulSTCSnapshotHi;    /* high 10-bit STC snapshot when picture received at the displayable point (in 27Mhz) */

    uint32_t            ulPictureId;        /* Displayable point Picture ID */

    /* Optional cadence info for PicAFF encode */
    bool                bCadenceLocked;     /* false if unused */

    BAVC_PicStruct      ePicStruct;        /* h264 style pic_struct cadence info */

    /* Optional 2H1V/2H2V decimated luma buffers (share the same MMA block as Y/C buffer) */
    BMMA_Block_Handle   h2H1VLumaBlock;/* NULL if unused */
    uint32_t            ul2H1VLumaOffset;
    BMMA_Block_Handle   h2H2VLumaBlock;/* NULL if unused */
    uint32_t            ul2H2VLumaOffset;
#endif
} BVEE_PictureDescriptor;
/***************************************************************************
Summary:
Description of a Data Sync settings
***************************************************************************/ 
typedef struct BVEE_DatasyncSettings
{
    unsigned eEnableStc;		/*Set 0 to disable, 1 to enable*/
    uint32_t ui32StcAddress;
} BVEE_DatasyncSettings;
/***************************************************************************
Summary:
Initialize picture descriptor to default values
***************************************************************************/
void BVEE_Channel_InitPictureDescriptor(
    BVEE_ChannelHandle handle,
    BVEE_PictureDescriptor *pPicture    /* [out] */
    );

/***************************************************************************
Summary:
Encoder Interrupt Handlers
***************************************************************************/
typedef struct BVEE_ChannelInterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;        
    } vencDataDiscarded;
} BVEE_ChannelInterruptHandlers;

/***************************************************************************
Summary:
Get Currently Registered Interrupt Handlers
***************************************************************************/
void BVEE_Channel_GetInterruptHandlers(
    BVEE_ChannelHandle handle,
    BVEE_ChannelInterruptHandlers *pInterrupts     /* [out] */
    );
    
/***************************************************************************
Summary:
Set Interrupt Handlers 
 
Description: 
To disable any unwanted interrupt, pass NULL for its callback routine
***************************************************************************/
BERR_Code BVEE_Channel_SetInterruptHandlers(
    BVEE_ChannelHandle handle,
    const BVEE_ChannelInterruptHandlers *pInterrupts
    );

/***************************************************************************
Summary:
Add picture to encoder's queue. 
 
Description: 
The provided descriptor will be enqueued for the firmware.  If 
BVEE_ChannelOpenSettings.enableExternalTrigger is true, the register 
described in BVEE_ChannelTriggerInfo must also be written each vsync. 
Otherwise, the encoder will be automatically triggered. 
 
See Also: 
    BVEE_Channel_InitPictureDescriptor 
    BVEE_Channel_DequeuePicture_isr 
***************************************************************************/
BERR_Code BVEE_Channel_EnqueuePicture_isr(
    BVEE_ChannelHandle handle,
    const BVEE_PictureDescriptor *pPicture 
    );

/***************************************************************************
Summary:
Remove a picture that the encoder has finished encoding. 
 
Description: 
This call is non-blocking.  If no pictures are available, 
NULL will be returned. 
 
Returns: 
    BERR_SUCCESS - A frame was successfully dequeued.
    BVEE_ERR_QUEUE_EMPTY - No frames are available to dequeue.
 
See Also: 
    BVEE_Channel_EnqueuePicture_isr 
***************************************************************************/
BERR_Code BVEE_Channel_DequeuePicture_isr(
    BVEE_ChannelHandle handle,    
    BVEE_PictureDescriptor *pPicture 
    );

/***************************************************************************
Summary:
External Trigger Information 
 
Description: 
This structure describes a register that must be written each vsync by 
an external source (e.g. BVN/VDC).  This is only used when the channel was 
opened with BVEE_ChannelOpenSettings. 
***************************************************************************/
typedef struct BVEE_ChannelTriggerInfo
{
    uint32_t address;       /* The register address to be written each vsync */
    uint32_t value;         /* The value to be written into the register each vsync */
} BVEE_ChannelTriggerInfo;

/***************************************************************************
Summary:
Get External Trigger Information 
***************************************************************************/
void BVEE_Channel_GetTriggerInfo(
    BVEE_ChannelHandle handle,
    BVEE_ChannelTriggerInfo *pInfo  /* [out] */
    );

/***************************************************************************
Summary:
    Get Channel Buffer Descriptors

Description:
    Returns video buffer descriptors for CDB content in the
    BAVC_VideoBufferDescriptor array(s)
***************************************************************************/
BERR_Code BVEE_Channel_GetBufferDescriptors(
   BVEE_ChannelHandle handle,
   const BAVC_VideoBufferDescriptor **pBuffer, /* [out] pointer to BAVC_VideoBufferDescriptor structs */
   size_t *pSize, /* [out] size of pBuffer in bytes (not number of BAVC_VideoBufferDescriptor structs) */
   const BAVC_VideoBufferDescriptor **pBuffer2, /* [out] pointer to BAVC_VideoBufferDescriptor structs after wrap around */
   size_t *pSize2 /* [out] size of pBuffer2 in bytes (not number of BAVC_VideoBufferDescriptor structs) */
   );

/***************************************************************************
Summary:
    Consume Channel Buffer Descriptors

Description:
    Reclaims the specified number of video buffer descriptors
    The CDB read pointer is updated accordingly
***************************************************************************/
BERR_Code BVEE_Channel_ConsumeBufferDescriptors(
    BVEE_ChannelHandle handle,
    unsigned numBufferDecsriptors /* must be <= pSize+pSize2 returned by last BVEE_Channel_GetBufferDescriptors call. */
    );

/***************************************************************************
Summary:
    Get Channel Buffer Status

Description:
    Returns the output buffer status (e.g. the base virtual address)
***************************************************************************/
BERR_Code BVEE_Channel_GetBufferStatus(
   BVEE_ChannelHandle handle,
   BAVC_VideoBufferStatus *pBufferStatus    /* [out] */
   );

/***************************************************************************
Summary:
    Set Data Sync Settings

Description:
    Provide STC address to DSP to do a TSM based picture drop in case if it 
    is not running real time. This STC address should be the same which was
    used to generate time-stamp for captured picture and *not* the STC that
    is used by video/audio decoder
***************************************************************************/
BERR_Code BVEE_Channel_SetDataSyncSettings(
   BVEE_ChannelHandle handle,
   BVEE_DatasyncSettings DataSyncSettings
   );

/***************************************************************************
Summary:
    Get Data Sync Settings

Description:
    Returns STC address used to do a TSM based picture drop in case if it 
    is not running real time.
***************************************************************************/
BERR_Code BVEE_Channel_GetDataSyncSettings(
   BVEE_ChannelHandle handle,
   BVEE_DatasyncSettings *DataSyncSettings
   );

/***************************************************************************
Summary:
VEE External Interrupt Handle
***************************************************************************/
typedef struct BVEE_ExtInterrupt *BVEE_ExtInterruptHandle;

/***************************************************************************
Summary:
External Interrupt Information 
 
Description: 
This structure describes a register address and bit that must be set by 
an external source (e.g. BVN/VDC), when data is ready for encode.
***************************************************************************/
typedef struct BVEE_ExtInterruptInfo
{
    uint32_t address;  /* The register address to be written each vsync */
    uint32_t bit_num;  /* The value to be written into the register each vsync */
} BVEE_ExtInterruptInfo;

/***************************************************************************
Summary:
    Allocate the external interrupt.

Description:
    With this interrupt bdsp can be interrupted from an external source.
***************************************************************************/
BERR_Code BVEE_Channel_AllocExtInterrupt(
   BVEE_ChannelHandle handle,
   BVEE_ExtInterruptHandle *pIntHandle
   );

/***************************************************************************
Summary:
    Get external interrupt info

Description:
    Returns allocated register address and bit number which needs to be set
	to raise interrupt.
***************************************************************************/
BERR_Code BVEE_Channel_GetExtInterruptInfo(
   BVEE_ChannelHandle handle,
   BVEE_ExtInterruptHandle pIntHandle,
   BVEE_ExtInterruptInfo *pExtIntInfo 
   );

/***************************************************************************
Summary:
    Free external interrupt

Description:
    Free up the allocated external interrupt.
***************************************************************************/
BERR_Code BVEE_Channel_FreeExtInterrupt(
   BVEE_ChannelHandle handle,
   BVEE_ExtInterruptHandle pIntHandle
   );

BERR_Code
BVEE_Channel_UserData_AddBuffers_isr(
         BVEE_ChannelHandle hVeeCh,
         const BUDP_Encoder_FieldInfo *pstUserDataFieldInfo, /* Pointer to first field info descriptor */
         unsigned uiCount, /* Count of user data field buffer info structs */
         unsigned *puiQueuedCount /* Count of user data field info structs queued by encoder (*puiQueuedCount <= uiCount) */
         );
typedef struct BVEE_Channel_UserData_Status
{
        unsigned uiPendingBuffers; /* Buffers pending */
        unsigned uiCompletedBuffers; /* Buffers completed since previous call to BVEE_Channel_GetStatusUserDataBuffers_isr() */
} BVEE_Channel_UserData_Status;
BERR_Code
BVEE_Channel_UserData_GetStatus_isr(
      BVEE_ChannelHandle hVeeCh,
      BVEE_Channel_UserData_Status *pstUserDataStatus
      );


typedef struct BVEE_ChannelSettings
{
    uint32_t                ui32TargetBitRate;          /* Number of bits per sec.  */
    BAVC_FrameRateCode      frameRate;
    bool                    bSceneChangeEnable;    
} BVEE_ChannelSettings;

/***************************************************************************
Summary:
Set Encoder Settings
***************************************************************************/
BERR_Code BVEE_Channel_GetSettings(
   BVEE_ChannelHandle handle,
   BVEE_ChannelSettings *pSettings
   );


/***************************************************************************
Summary:
Get Encoder Settings
***************************************************************************/
BERR_Code BVEE_Channel_SetSettings(
   BVEE_ChannelHandle handle, 
   const BVEE_ChannelSettings *pSettings
   );

/***************************************************************************
Summary:
Encoder status information. 
 
Description: 
This structure describes the current encoder status.
***************************************************************************/
typedef struct BVEE_ChannelStatus
{
    uint32_t    ui32TotalFramesRecvd;
    uint32_t    ui32TotalFramesEncoded;
    uint32_t    ui32TotalFramesDropedForFRC;
    uint32_t    ui32TotalFramesDropedForLipSynch;
    uint32_t    ui32CdbFullCounter;
    uint32_t    ui32RelinquishCounter;
    uint32_t    ui32EncodedPTS;
    uint32_t    ui32StcValue;   
} BVEE_ChannelStatus;

/***************************************************************************
Summary:
    Get current encoder status.

Description:
    Get current encoder status.
***************************************************************************/
void BVEE_Channel_GetStatus(
    BVEE_ChannelHandle handle,
    BVEE_ChannelStatus *pStatus     /* [out] */
    );


#endif
