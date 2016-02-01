/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * Porting interface code for the DirecTV portion of the data transport core. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/*= Module Overview *********************************************************
This module provides the API to control the DirecTV sections of the data
transport. Many calls are simply DirecTV versions of call in the MPEG portion
of the API. 
***************************************************************************/

#ifndef BXPT_DIRECTV_H__
#define BXPT_DIRECTV_H__

#include "bxpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Defines the options regarding the MPT flag.
****************************************************************************/
typedef enum BXPT_DirecTvMessageFlags
{
    BXPT_DirecTvMessageFlags_eSaveNone,     /* Don't save any MPT flag */
    BXPT_DirecTvMessageFlags_eSaveFirst,    /* Save 1st MPT flag only, per MPT message */   
    BXPT_DirecTvMessageFlags_eSaveAll       /* Save all MPT flags in each transport packet of an MPT message */
}
BXPT_DirecTvMessageFlags;

/***************************************************************************
Summary:
Defines the types of SCID PSI filtering that can be performed.
****************************************************************************/
typedef enum BXPT_DirecTvMessageType
{
	BXPT_DirecTvMessageType_eAuxOnlyPackets,
	BXPT_DirecTvMessageType_eMpt,
	BXPT_DirecTvMessageType_eRegular,

	/* Regular messages with CAP filtering. */
	BXPT_DirecTvMessageType_eRegular_CapFilter0,
	BXPT_DirecTvMessageType_eRegular_CapFilter1,
	BXPT_DirecTvMessageType_eRegular_CapFilter2,
	BXPT_DirecTvMessageType_eRegular_CapFilter3,
	BXPT_DirecTvMessageType_eRegular_CapFilter4
}									    
BXPT_DirecTvMessageType;

/***************************************************************************
Summary:
Defines the types of SCID PES filtering that can be performed.
****************************************************************************/
typedef enum BXPT_DirecTvRecordType
{
	BXPT_DirecTvRecordType_eAudio,
	BXPT_DirecTvRecordType_eVideo,
	BXPT_DirecTvRecordType_ePacketSaveAll
}
BXPT_DirecTvRecordType;

/***************************************************************************
Summary:
Enumerations for the parser band modes. 
****************************************************************************/
typedef enum BXPT_ParserMode
{
	BXPT_ParserMode_eMpeg,
	BXPT_ParserMode_eDirecTv
}
BXPT_ParserMode;


/***************************************************************************
Summary:
Additional options for DirecTV message filtering.
****************************************************************************/
typedef struct BXPT_DirecTvMessageOptions
{
    BXPT_DirecTvMessageFlags Flags;
}
BXPT_DirecTvMessageOptions;


/***************************************************************************
Summary:
Data needed to configure a single SCID channel recording. 
****************************************************************************/
typedef struct BXPT_ScidChannelRecordSettings
{												
	unsigned int Pid;								/* Which SCID to record. */ 
	unsigned int Band;								/* The band the SCID is on. */
	BXPT_DirecTvRecordType RecordType;				/* Type of recording to do. */

	/*
	** Set to true to have data byte-aligned in the message buffer. If false,
	** the data is word aligned, with bytes of value 0x55 inserted as padding.
	** This is done for software compatability with older chips that didn't
	** support byte-alignment.
	*/
	bool ByteAlign;					
} 
BXPT_ScidChannelRecordSettings;
		   
/***************************************************************************
Summary:
Enumerations for the PID channel HD filtering modes. 
****************************************************************************/
typedef enum BXPT_HdFilterMode
{
	BXPT_HdFilterMode_eNonAuxOnly = 0,	/* only non-AUX packets are considered a match for the pid table entry */
	BXPT_HdFilterMode_eAuxOnly = 1		/* only AUX packets (HD=0, orig_cf bit=1) are considered a match for this pid table entry */
}
BXPT_HdFilterMode;

/***************************************************************************
Summary:
Set the MPEG or DirectTV mode in a given parser band.

Description:
Changes a parser band between MPEG and DirecTV mode. Also sets the packet 
length as appropriate. 

Returns:
    BERR_SUCCESS                - Change was successful.
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code BXPT_DirecTv_SetParserBandMode( 
	BXPT_Handle hXpt, 	   	/* [in] Handle for this transport */
	unsigned int Band, 			/* [in] Which parser band */
	BXPT_ParserMode Mode	/* [in] Which mode (packet format) is being used. */
	);

/***************************************************************************
Summary:
Set the limits for the PES stream ID.

Description:
Set the upper and lower boundary for the stream ID range check performed 
in DIRECTV mode during PES parsing. This configuration effects all parser
bands that are in DirecTV mode.

Returns:
    BERR_SUCCESS                - New boundaries have been set.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_DirecTv_SetStartcodeChecking
***************************************************************************/
void BXPT_DirecTv_SetPesStreamIdBoundaries( 
	BXPT_Handle hXpt, 	   	/* [in] Handle for this transport */
	unsigned int UpperId,		/* [in] The upper stream id. */
	unsigned int LowerId			/* [in] The lower stream id. */
	);

/***************************************************************************
Summary:
Enable or disable startcode checking. 

Description:
PES startcode fields can optionally be checked during PES parsing. Only PES
packets with startcodes that fall in the range set by 
BXPT_DirecTv_SetPesStreamIdBoundaries() are forwarded to the message buffers.

Returns:
    BERR_SUCCESS                - Startcode checking enabled.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_DirecTv_SetPesStreamIdBoundaries
***************************************************************************/
void BXPT_DirecTv_SetStartcodeChecking( 
	BXPT_Handle hXpt, 	   	/* [in] Handle for this transport */
	bool EnableChecking		/* [in] Enable checking, or not. */
	);

/***************************************************************************
Summary:
Save the MPT flag in front of the reconstructed message.

Description:
Configure the message filters to put the MPT flag in front of the 
reconstructed message. This configuration effects all parser bands that are
in DirecTV mode.

Returns:
	void
***************************************************************************/
void BXPT_DirecTv_SaveMptFlag( 
	BXPT_Handle hXpt, 	   	/* [in] Handle for this transport */
	bool Enable 			/* [in] Enable or disable flag saving. */
	);

#if BXPT_HAS_PID2BUF_MAPPING
/***************************************************************************
Summary:
Use this API only when you need PID to multiple buffer mapping capabilities.
Pass Message Buffer Number along with the PidChannelNumber.
Configure for capturing DirecTV Aux or MPT messages.

Description:
Associate one or more message filters with a given SCID and parser band.
This function will enable the SCID channel, but will not install an 
interrupt handler for the message interrupt. The interrupt handler should be
installed before calling this function.

To select filtering with a CAP address, use MessageType 
BXPT_DirecTvMessageType_eRegular_CapFilterX, where X is the CAP filter to be
used. The CAP address is loaded into the filter by calling 
BXPT_DirecTv_SetCapPattern ()

The caller must allocate a SCID channel before using the function. It should
NOT be called if the PID channel is already enabled.

Returns:
    BERR_SUCCESS                - Message capture is setup.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_AllocPSIFilter, BXPT_GetFilter, BXPT_SetFilter, BXPT_AddFilterToGroup,
BXPT_StopDirecTvMessageCapture, BXPT_DirecTv_SetCapPattern
***************************************************************************/
BERR_Code BXPT_Mesg_StartDirecTvMessageCapture( 
	BXPT_Handle hXpt, 							/* [in] Handle for this transport */
	unsigned int PidChannelNum, 					/* [in] Which PID channel. */
	unsigned int MesgBufferNum,                 /* [in] Which Message Buffer. */
	BXPT_DirecTvMessageType MessageType,		/* [in] What type of DirecTV messages. */
	const BXPT_PsiMessageSettings *Settings 	/* [in] PID, band, and filters to use. */
	);

/***************************************************************************
Summary:
Start message capturing with additional configuration options.  
 
Description:
This call performs the same service as BXPT_Mesg_StartDirecTvMessageCapture(), 
except with additional options. See the BXPT_DirecTvMessageOptions struct 
for the additions. 
 
Returns:
    BERR_SUCCESS                - Message capture is setup.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_AllocPSIFilter, BXPT_GetFilter, BXPT_SetFilter, BXPT_AddFilterToGroup,
BXPT_StopDirecTvMessageCapture, BXPT_DirecTv_SetCapPattern
***************************************************************************/
BERR_Code BXPT_Mesg_StartDirecTvMessageCaptureWithOptions( 
	BXPT_Handle hXpt, 							/* [in] Handle for this transport */
	unsigned int PidChannelNum, 					/* [in] Which PID channel. */
	unsigned int MesgBufferNum,                 /* [in] Which Message Buffer. */
	BXPT_DirecTvMessageType MessageType,		/* [in] What type of DirecTV messages. */
	const BXPT_PsiMessageSettings *Settings, 	/* [in] PID, band, and filters to use. */
    const BXPT_DirecTvMessageOptions *Options   /* [in] Additional options for message capture */
	);

/***************************************************************************
Summary:
Use this API only when you need PID to multiple buffer mapping capabilities.
Pass Message Buffer Number along with the PidChannelNumber.
Stop capturing DirecTV messages.

Description:
Disable the SCID channel being used to capture DirecTV messages. Flush any data
still in the chip. If the interrupts for the message buffer are still 
enabled, flushing the data may trigger another interrupt.

NOTE: This function may sleep for up to 200 microseconds, in order to flush 
any remaining data from the hardware's internal buffers.

Returns:
    BERR_SUCCESS                - Message capture is stopped.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_StartDirecTvMessageCapture
***************************************************************************/
BERR_Code BXPT_Mesg_StopDirecTvMessageCapture( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for this transport */
	unsigned int ScidChannelNum,	/* [in] Which SCID channel. */
	unsigned int MesgBufferNum      /* [in] Which Message Buffer. */
	);
#endif

/***************************************************************************
Summary:
Configure HD filtering for a PID channel.

Description:
Each PID channel can independently filter DirecTV packets based on their 
SCID and the value of the HD fields. Both the SCID and the HD values must 
match for the packet to be accepted. The filtering is done independently of
the PSI message filters; using this function will not consume PSI filter
resources.

Filtering is performed on the HD field based upon the value of the FilterMode
parameter. Only two values are supported: only AUX are accepted by the filter
and only non-AUX are accepted. 

NOTE: 
Since the channel will drop any packets that don not have the correct SCID 
and the correct HD value, the channel should be used for PSI message capture 
only.

The function should be called AFTER BXPT_ConfigurePidChannel(). Filtering may
not be configured correctly if the calls are made out of that seqeunce.

Returns:
    BERR_SUCCESS                - Filter configured.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_StartPidChannelRecord, BXPT_ConfigurePidChannel
***************************************************************************/
BERR_Code BXPT_DirecTv_ConfigHdFiltering( 
	BXPT_Handle hXpt, 	  				/* [in] Handle for this transport */
	unsigned int PidChannelNum,  		/* [in] Which PID channel. */
	bool EnableFilter, 					/* [in] Enable filtering if true, disable if false. */
	BXPT_HdFilterMode FilterMode		/* [in] HD values to filter on. Ignored if EnableFilter == false */
	);

/***************************************************************************
Summary:
Set the CAP filter pattern.

Description:
Load the CAP filter pattern into the given address filter. The pattern is 32 
bits long. 

Returns:
    BERR_SUCCESS                - Merging enabled or disabled successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code BXPT_DirecTv_SetCapPattern( 
	BXPT_Handle hXpt, 	  		/* [in] Handle for this transport */
	unsigned AddressFilter,  	/* [in] Which address filter gets the pattern. */
	uint32_t Pattern  	 	 	/* [in] The pattern to load. */
	); 

#if BXPT_HAS_STATIC_PID2BUF				
/***************************************************************************
Summary:
Configure for capturing DirecTV Aux or MPT messages.

Description:
Associate one or more message filters with a given SCID and parser band.
This function will enable the SCID channel, but will not install an 
interrupt handler for the message interrupt. The interrupt handler should be
installed before calling this function.

To select filtering with a CAP address, use MessageType 
BXPT_DirecTvMessageType_eRegular_CapFilterX, where X is the CAP filter to be
used. The CAP address is loaded into the filter by calling 
BXPT_DirecTv_SetCapPattern ()

The caller must allocate a SCID channel before using the function. It should
NOT be called if the PID channel is already enabled.

Returns:
    BERR_SUCCESS                - Message capture is setup.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_AllocPSIFilter, BXPT_GetFilter, BXPT_SetFilter, BXPT_AddFilterToGroup,
BXPT_StopDirecTvMessageCapture, BXPT_DirecTv_SetCapPattern
***************************************************************************/
BERR_Code BXPT_StartDirecTvMessageCapture( 
	BXPT_Handle hXpt, 							/* [in] Handle for this transport */
	unsigned int PidChannelNum, 					/* [in] Which PID channel. */
	BXPT_DirecTvMessageType MessageType,		/* [in] What type of DirecTV messages. */
	const BXPT_PsiMessageSettings *Settings 	/* [in] PID, band, and filters to use. */
	);


BERR_Code BXPT_StartDirecTvMessageCaptureWithOptions( 
	BXPT_Handle hXpt, 							/* [in] Handle for this transport */
	unsigned int PidChannelNum, 					/* [in] Which PID channel. */
	BXPT_DirecTvMessageType MessageType,		/* [in] What type of DirecTV messages. */
	const BXPT_PsiMessageSettings *Settings, 	/* [in] PID, band, and filters to use. */
    const BXPT_DirecTvMessageOptions *Options   /* [in] Additional options for message capture */
	);

/***************************************************************************
Summary:
Stop capturing DirecTV messages.

Description:
Disable the SCID channel being used to capture DirecTV messages. Flush any data
still in the chip. If the interrupts for the message buffer are still 
enabled, flushing the data may trigger another interrupt.

NOTE: This function may sleep for up to 200 microseconds, in order to flush 
any remaining data from the hardware's internal buffers.

Returns:
    BERR_SUCCESS                - Message capture is stopped.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_StartDirecTvMessageCapture
***************************************************************************/
BERR_Code BXPT_StopDirecTvMessageCapture( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for this transport */
	unsigned int PidChannelNum 				/* [in] Which PID channel. */
	);

/***************************************************************************
Summary:
Record a SCID channel through the message buffers.

Description:
Configure a single SCID channel for recording, using the message buffers. 
The SCID, parser band, and type of recording to do are specified in the 
Settings structure. This function will not enable the SCID channel, and will
not install an interrupt handler for the message interrupt. 

The caller must allocate a SCID channel before using the function. It should
NOT be called if the SCID channel is already enabled.

Returns:
    BERR_SUCCESS                - SCID recording has been configured.
    BERR_INVALID_PARAMETER      - Bad input parameter.
***************************************************************************/
BERR_Code BXPT_DirecTv_StartScidChannelRecord( 
	BXPT_Handle hXpt, 						/* [in] Handle for this transport. */
	unsigned int PidChannelNum, 				/* [in] Which PID channel. */
	BXPT_ScidChannelRecordSettings *Settings 	/* [in] SCID, etc. to record. */
	);

/***************************************************************************
Summary:
Disable recording of a PID channel.

Description:
Stops the recording of packet or PES data on the given PID channel. The PID
channel is disabled. The interrupt handler, if there was one installed, will
not be disabled; thus, any data still in the hardware may trigger an interrupt
after this function returns.

NOTE: This function may sleep for up to 200 microseconds, in order to flush 
any remaining data from the hardware's internal buffers.

Returns:
    BERR_SUCCESS                - PID channel record stopped.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_StartPidChannelRecord
***************************************************************************/
BERR_Code BXPT_DirecTv_StopScidChannelRecord( 
	BXPT_Handle hXpt, 	  						/* [in] Handle for this transport */
	unsigned int ScidChannelNum 					/* [in] Which SCID channel. */
	);

#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_DIRECTV_H__ */

/* end of file */
