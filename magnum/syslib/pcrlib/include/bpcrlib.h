/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
#ifndef BPCRLIB_H_
#define BPCRLIB_H_

#include "bxpt.h"
#include "bxpt_pcr.h"
#include "bchp.h"
#include "bavc.h"
#include "bkni.h"

/*= Module Overview *********************************************************
Overview
PCRlib module manages local timebase, aka STC (System Time Clock) of 
a decoder channel. Decoder channel is a combination of single MPEG video
decoder and audio decoder. Task of PCRlib is to keep decoders reference
time in sync with each other (to maintain lip-sync) and with incoming
MPEG stream.

User controls PCRLib module by connecting video and audio decoders
into the single timebase. For PVR type of applications users could
choose to connect record channel. For systems with dual audio
output user have to connect secondary audio decoder to the 
selected timebase channel.  For systems with three audio decodes,
the user will need to connect the tertiary audio decoder as well.

***************************************************************************/

/* This tyoe is used to represent instance of PCRLib module */
typedef struct BPCRlib_P_Handle *BPCRlib_Handle;

/***************************************************************************
Summary:
    Activates PCRLib module
Description:
    This function initializes internal structures of the PCRLib module
    and returns handle to the module instance.
See Also:
    BPCRlib_Close
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code 
BPCRlib_Open(
        BPCRlib_Handle *handle, /* [out] the BPCRlib handle */
        BCHP_Handle hChip   /* [in] the chip handle */
        );

/***************************************************************************
Summary:
    Deactivates PCRlib module
Description:
    This function release all resources allocated during call to 
    the BPCRlib_Open function.
See Also:
    BPCRlib_Open
Returns:
    N/A
***************************************************************************/
void 
BPCRlib_Close(
        BPCRlib_Handle handle /* [in] the BPCRlib handle */
        );

/***************************************************************************
Summary:
    Standard decoder interface for time-related functions
***************************************************************************/
typedef struct BPCRlib_StcDecIface {
    BERR_Code (*getPts)(void *dec, BAVC_PTSInfo *pts);
    BERR_Code (*getStc)(void *trp, void *dec, uint32_t *stc);
    BERR_Code (*getCdbLevel)(void *dec, unsigned *level);
    BERR_Code (*setStc)(void *trp, void *dec, bool dss, uint32_t stc);
    BERR_Code (*updateStc)(void *trp, bool is_request_stc);
    bool useAuxTrp;
} BPCRlib_StcDecIface;

/***************************************************************************
Summary:
    <Needs summary>
***************************************************************************/
typedef enum BPCRlib_Mode {
    BPCRlib_Mode_eAutoPts,
    BPCRlib_Mode_eConstantDelay
} BPCRlib_Mode;

/***************************************************************************
Summary:
    Synchronization mode
***************************************************************************/
typedef enum BPCRlib_TsmMode 
{
    BPCRlib_TsmMode_eSTCMaster = 0,             /* STC is seeded with either the video or audio PTS, depending on which decoder makes an STC request first */
    BPCRlib_TsmMode_eVideoMaster,           /* STC is seeded with the video PTS */
    BPCRlib_TsmMode_eAudioMaster,           /* STC is seeded with the audio PTS */
    BPCRlib_TsmMode_eOutputMaster,          /* No tsm is performed.  Output clock pulls data through decoder.  Also called VSYNC mode. */
    BPCRlib_TsmMode_eMax
} BPCRlib_TsmMode;

/***************************************************************************
Summary:  
    This structure is used for run-time configuration of PCRLib channel
Description:
    This structure has a information about managed timebase channel. 
    It's an user role to fill in this structure with values appropriate for
    desired configuration.
***************************************************************************/

typedef struct BPCRlib_Config {
    bool playback; /* true if PCRlib channel is used to manage playback type of sources, false otherwise */
    BAVC_StreamType stream; /* Type of the managed stream */
    void *audio; /* Audio decoder handle for the primary audio channel */
    const BPCRlib_StcDecIface *audio_iface; /* interface to control the primary audio decoder */
    void *video; /* Video decoder handle */
    const BPCRlib_StcDecIface *video_iface; /* interface to control the video decoder */
    void *secondary_audio; /* Audio decode handle for the secondary audio channel */
    const BPCRlib_StcDecIface *secondary_audio_iface; /* interface to control the secondary audio decoder */
    void *tertiary_audio; /* Audio decode handle for the tertiary audio channel */
    const BPCRlib_StcDecIface *tertiary_audio_iface; /* interface to control the tertiary audio decoder */
    void *aux_transport; /* pointer for the auxilary transport engine */
    unsigned video_cdb_level; /* threshold for the video decoder's  compressed buffer, this is used to detect underflow condition in the decoder */
    int video_pts_offset; /* this offset is used in playback mode, when STC initially loaded from the video PTS, 45KHz units */
    int video_stc_discard; /* this delta is a discard threashold between video PTS and STC, 45KHz units */
    unsigned audio_cdb_level; /* threshold for the video decoder's  compressed buffer, this is used to detect underflow condition in the decoder */
    int audio_pts_offset; /* this offset is used in playback mode, when STC initially loaded from the audio PTS, 45KHz units */
    int audio_stc_discard; /* this delta is a discard threashold between audio PTS and STC, 45KHz units  */
    int pcr_offset; /* offset between PCR and STC. thise offset is used to set an additional dealy in the decoders in order to compoensate fro the jitter, 45KHz units */
    int pcr_discard; /* threshold to discard PCR-PTS , to discard PCR value , 45KHz units */
    BPCRlib_Mode mode; /* operation mode for the PCR channel */
    bool is_playback_ip; /* true if PCRlib channel is used to manage IP playback type of sources, false otherwise */
    BPCRlib_TsmMode tsm_mode; /* allows the user to control which decoder will control the STC --> defaults to whoever requests an STC first */
    unsigned int sync_limit; /* the limit outside of which the decoders will not apply master mode TSM */
    BKNI_EventHandle flush; /* requests that decoder CDBs be flushed */
    uint32_t consecutive_pts_error_limit; /* how many consecutive pts errors before we do something different */
    bool refresh_stc_on_invalidate; /* force a refresh of the STC when invalidate is called, defaults true */
    bool non_real_time; /* causes various changes to behavior when in non real time mode */
    bool paired; /* causes changes to behavior when paired with another pcrlib instance */
} BPCRlib_Config;

/* This tyoe is used to control PCRLib initial configuration */
typedef void *BPCRlib_ChannelSettings;

/***************************************************************************
Summary:  
    This function initializes PCRlib channel initial configuration
Description:
    This function is used to populate channel initial configuration
    with default parameters.
Returns:
    BERR_SUCCESS                - if successful 
***************************************************************************/
BERR_Code BPCRlib_Channel_GetChannelDefSettings(
        BPCRlib_Handle handle,  /* [in] PCRlib handle */
        BPCRlib_ChannelSettings *config /* [out] default PCRlib channel settings */
    );

/* This tyoe is used to represent instance of the PCRlib channel */
typedef struct BPCRlib_P_Channel *BPCRlib_Channel;

/***************************************************************************
Summary:
    Activates PCRlib channel
Description:
    This function initializes internal structures of the PCRLib channel
    and returns handle to the channel instance.
See Also:
    BPCRlib_DestroyChannel
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_Channel_Create(
        BPCRlib_Handle handle,  /* [in] PCRlib handle */
        BXPT_PCR_Handle pcr,    /* [in] XPT_PCR handle */
        BPCRlib_Channel *channel, /* [out] PCRlib channel handle */
        const BPCRlib_ChannelSettings *settings /* [in[ PCRlib channel configuration */
    );

/***************************************************************************
Summary:
    Deactivates PCRlib channel
Description:
    This function release all resources allocated during call to 
    the BPCRlib_Channel_Create function.
See Also:
    BPCRlib_Channel_Create
Returns:
    N/A
***************************************************************************/
void
BPCRlib_Channel_Destroy(
        BPCRlib_Channel channel /* [in] PCRlib channel handle */
    );

/***************************************************************************
Summary:
    Get current configuration for PCRlib channel
Description:
    This function is used to retrieve current configuration of the PCRlib channel
See Also:
    BPCRlib_Channel_SetConfig
Returns:
    BERR_SUCCESS                - if successful 
***************************************************************************/
void
BPCRlib_Channel_GetConfig(
        BPCRlib_Channel channel, /* [in] PCRlib channel handle */
        BPCRlib_Config *config /* [out] current configuration of the PCRlib channel  */
        );

/***************************************************************************
Summary: 
    Set new configuration to PCRlib channel
Description:
    This function is used to sent new configuration to the PCRlib channel
See Also:
    BPCRlib_Channel_SetConfig
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_Channel_SetConfig(
        BPCRlib_Channel channel,/* [in] PCRlib channel handle */
        const BPCRlib_Config *config /* [out] new configuration for the PCRlib channel  */
        );

/***************************************************************************
Summary: 
    Invalidates state of PCRlib channel
Description:
    This function is used to invalidate state of all decoders attached to the PCRlib channels, usually it cases 
    to reload STC to the decoders on earliest opportunity.
See Also:
    BPCRlib_Channel_SetConfig
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_Channel_Invalidate(
        BPCRlib_Channel channel /* [in] PCRlib channel handle */
        );

/***************************************************************************
Summary:
    Update STC in audio decoder
Description:
    This function shall be called by an user when audio decoder
    has requested STC. Usually user have to install ISR callback
    and call this function from a callback.
See Also:
    BPCRlib_Channel_SetConfig, BPCRlib_Channel_VideoRequestStc_isr
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_Channel_AudioRequestStc_isr(
    BPCRlib_Handle handle, /* [in] PCRlib channel handle */
    void *audio,           /* [in] audio decoder handle */
    uint32_t audio_pts     /* [in] last PTS from the audio stream, in 45KHz units */
    );

/***************************************************************************
Summary:
    Update STC in video decoder
Description:
    This function shall be called by an user when video decoder
    has requested STC. Usually user have to install ISR callback
    and call this function from a callback.
See Also:
    BPCRlib_Channel_SetConfig, BPCRlib_Channel_AudioRequestStc_isr
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_Channel_VideoRequestStc_isr(
    BPCRlib_Handle handle, /* [in] PCRlib channel handle */
    void *video, /* [in] video decoder handle */
    const BAVC_PTSInfo *video_pts /* [in] last PTS from the video stream, in 45KHz units */
    );

/***************************************************************************
Summary:
    Resynchronize video decoder
Description:
    This function shall be called by an user when video decoder
    has received PTS Error. Usually user have to install ISR callback
    and call this function from a callback.
See Also:
    BPCRlib_Channel_SetConfig, BPCRlib_Channel_VideoRequestStc_isr
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_Channel_VideoPtsError_isr(
    BPCRlib_Handle handle, /* [in] PCRlib channel handle */
    void *video, /* [in] video decoder handle */
    const BAVC_PTSInfo *video_pts, /* [in] last PTS from the video stream, in 45KHz units */
    uint32_t video_stc /* [in] snapshot value of STC from the video decoder, in 45KHz units */ 
    );

/***************************************************************************
Summary:
    Resynchronize audio decoder
Description:
    This function shall be called by an user when video decoder
    has received PTS Error. Usually user have to install ISR callback
    and call this function from a callback.
See Also:
    BPCRlib_Channel_SetConfig, BPCRlib_Channel_AudioRequestStc_isr
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_Channel_AudioPtsError_isr(
    BPCRlib_Handle handle, /* [in] PCRlib channel handle */
    void *audio, /* [in] audio decoder handle */
    const BAVC_PTSInfo *audio_pts, /* [in] last PTS from the audio stream, in 45KHz units */
    uint32_t audio_stc /* [in] snapshot value of STC from the audio decoder, in 45KHz units */ 
    );

/***************************************************************************
Summary: 
    Set new STC
Description:
    This function is used to update STC value for the PCRLib channel
See Also:
    BPCRlib_Channel_GetStc 
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_Channel_PcrUpdate(
        BPCRlib_Channel channel,/* [in] PCRlib channel handle */
        uint32_t stc/* [in] new stc */
        );

/***************************************************************************
Summary: 
    Get current STC
Description:
    This function is used to retrieve STC  value for the PCRLib channel
See Also:
    BPCRlib_Channel_PcrUpdate
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_Channel_GetStc(BPCRlib_Channel channel, uint32_t *stc);

/***************************************************************************
Summary: 
    Calculate STC difference
Description:
    This function takes stc in the 45KHz domain (MPEG) or 27MHz domain (DSS)
    as 32 bit unsigned and returns delta in the 22.5KHz domain, 32 bit signed
Returns:

***************************************************************************/
int32_t BPCRlib_StcDiff_isrsafe(bool dss, uint32_t stc1, uint32_t stc2);

/***************************************************************************
Summary: 
    Check to see if PCR discontinuity has occured
Description:
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code
BPCRlib_HasPcrDisco(
    BPCRlib_Handle handle, /* [in] PCRlib channel handle */
    void *decoder, /* [in] decoder handle */
    bool *has_disco
    );

#endif /* BPCRLIB_H_ */


