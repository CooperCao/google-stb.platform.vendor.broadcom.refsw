/***************************************************************************
*     (c)2004-2008 Broadcom Corporation
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
* Description:
*   API name: SI
*    Library routines for SI abstraction
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_PSIP_LIB_H__
#define B_PSIP_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The PSIP Application Library provides an asynchronous API for acquiring 
Program and System Information Protocol for terrestrial and cable 
(ATSC A/65C).  The library supports multiple, simultaneous calling threads
and section filters.  It is designed to be independent of the underlying
section filtering allowing for straightforward integration with existing 
software.

There are APIs for basic PSIP table acquisition (PAT, PMT, MGT, TVCT, CVCT,
RRT, EIT, ETT, STT, DCCT, and DCCSCT) as well as higher level APIs 
(GetTime, GetTableVersions, GetNumChannels, GetChannelList, GetProgram)
which encompass multiple table retrievals.
***************************************************************************/

#include "b_os_lib.h"
#include "psip_common.h"
#include <time.h>

#include "ts_psi.h"
#include "ts_pat.h"
#include "ts_pmt.h"
#include "psip_mgt.h"
#include "psip_stt.h"
#include "psip_vct.h"
#include "psip_rrt.h"
#include "psip_dcct.h"
#include "psip_dccsct.h"

#define MAX_CH_NAME_CHARS       	7
#define MAX_LONG_NAME_CHARS     	128
#define MAX_AUDIO_PIDS          	5
#define MAX_VIDEO_PIDS          	3
#define MAX_TITLE_CHARS         	96
#define MAX_DESC_CHARS          	512
#define MAX_PROGRAM_NUM         	256
#define MAX_RATING_STR          	32
#define MAX_RATING_DIMS         	256
#define MAX_CAPTION_SERVICES    	16
#define MAX_AC3_AUDIO_DESC_LEN  	8
#define MAX_AC3_DESC            	3
#define MAX_GENRES              	31
#define MAX_TIME_SHIFTS_SERVICES 	20

/* Multiple current lang codes are often necessary for languages such as
   Spanish (SPA/ESP) and French (FRE/FRA). */
#define NUM_LANG_CODES              3 

/***************************************************************************
Summary:
    SI Error Codes

Description:
    Additional error codes used exclusively by the PSIP applib.  These
    codes are extensions to those found in the OS applib.
***************************************************************************/
#define B_ERROR_PSIP_BUSY           10  /* busy processing previous request */
#define B_ERROR_PSIP_CANCEL         11  /* cancelled */
#define B_ERROR_PSIP_RETRY          12  /* function did not fail - retry later */

#define B_PSIP_PAT_TIMEOUT                  300
#define B_PSIP_PMT_TIMEOUT                  300
#define B_PSIP_MGT_TIMEOUT                  300
#define B_PSIP_TVCT_TIMEOUT                 1200
#define B_PSIP_CVCT_TIMEOUT                 1200
#define B_PSIP_RRT_TIMEOUT                  60000
#define B_PSIP_EIT_TIMEOUT                  4000
#define B_PSIP_ETT_TIMEOUT                  5000
#define B_PSIP_STT_TIMEOUT                  2000
#define B_PSIP_DCCT_TIMEOUT                 2000
#define B_PSIP_DCCSCT_TIMEOUT               2000

#define B_PSIP_GETTIME_TIMEOUT              3000
#define B_PSIP_GETTABLEVERSIONS_TIMEOUT     3000
#define B_PSIP_GETCHANNELLIST_TIMEOUT       10000
#define B_PSIP_GETPROGRAM_TIMEOUT           20000


/***************************************************************************
Summary:
    Handle to a PSIP instance.
***************************************************************************/
typedef struct B_PSIP * B_PSIP_Handle;

/***************************************************************************
Summary:
    SI AppLib Settings
***************************************************************************/
typedef struct B_PSIP_Settings
{
    uint32_t         PatTimeout;                                /* PAT table timeout */
    uint32_t         PmtTimeout;                                /* PMT table timeout */
    uint32_t         MgtTimeout;                                /* MGT table timeout */
    uint32_t         TvctTimeout;                               /* TVCT table timeout */
    uint32_t         CvctTimeout;                               /* CVCT table timeout */
    uint32_t         RrtTimeout;                                /* RRT table timeout */
    uint32_t         EitTimeout;                                /* EIT table timeout */
    uint32_t         EttTimeout;                                /* ETT table timeout */
    uint32_t         SttTimeout;                                /* STT table timeout */
    uint32_t         DcctTimeout;                               /* DCCT table timeout */
    uint32_t         DccsctTimeout;                             /* DCCSCT table timeout */
    uint16_t         RatingRegion;                              /* current rating region (USA = 1) */
    uint8_t        * RedistributionControlData;                 /* pointer to application buffer */
    uint8_t          ISO_639_language_code[NUM_LANG_CODES][3];  /* valid language codes */ 
} B_PSIP_Settings;

/***************************************************************************
Summary:
    Filter used for limiting data collection.
***************************************************************************/
#define B_PSIP_FILTER_SIZE 32
typedef struct B_PSIP_Filter
{
    uint8_t mask[B_PSIP_FILTER_SIZE];           
    uint8_t coef[B_PSIP_FILTER_SIZE];
    uint8_t excl[B_PSIP_FILTER_SIZE];
} B_PSIP_Filter;

/***************************************************************************
Summary:
    Commands issued by PSIP applib to direct data collection callback
***************************************************************************/
typedef enum B_PSIP_CollectCmd
{
    B_PSIP_eCollectStart,       /* start section data retrieval */
    B_PSIP_eCollectStop,        /* stop section data retrieval */
    B_PSIP_eCollectGetBuffer,   /* get pointer to data buffer/length */
    B_PSIP_eCollectCopyBuffer   /* get copy of data buffer/length */
} B_PSIP_CollectCmd;

/***************************************************************************
Summary:
    Generic pointer to the section filter instance used to retrieve data for 
    the given job in SI.  This allows SI to service jobs for
    information from multiple sources simultaneously.
***************************************************************************/
typedef void * B_PSIP_FilterHandle;

/***************************************************************************
Summary:
    SI Data Collection (Data Ready) Callback Function Prototype
    This function is called when a started section filter has acquired data
    and is ready for retrieval.
***************************************************************************/
typedef void (* B_PSIP_DataReadyCallback)(void * context);

/***************************************************************************
Summary:
    Request to application data collection callback function.              
***************************************************************************/
typedef struct B_PSIP_CollectionRequest
{
    B_PSIP_Handle               si;             /* handle to PSIP instance */
    B_PSIP_CollectCmd           cmd;            /* command to execute */
    B_PSIP_FilterHandle         filterHandle;   /* filterhandle to use to retrieve data */
    B_PSIP_DataReadyCallback    callback;       /* callback to execute when data is command has been completed */
    void                      * callbackParam;  /* context for callback */
    uint16_t                    pid;            /* PID to use to retrieve data */
    B_PSIP_Filter               filter;         /* filter to use to retrieve data */
    uint8_t                     section;        /* section where data resides */
    uint8_t                   * pBuffer;        /* buffer to store data */
    uint32_t                  * pBufferLength;  /* size of buffer */
} B_PSIP_CollectionRequest;

/***************************************************************************
Summary:
    Channel modulation type
***************************************************************************/
typedef enum B_PSIP_ModulationMode
{
    B_PSIP_eModAnalog,
    B_PSIP_eModSCTE1,
    B_PSIP_eModSCTE2,
    B_PSIP_eModATSC8,
    B_PSIP_eModATSC16,
    B_PSIP_eModUnknown
} B_PSIP_ModulationMode;

/***************************************************************************
Summary:
    Channel service type
***************************************************************************/
typedef enum B_PSIP_ServiceType
{
    B_PSIP_eServiceAnalog,
    B_PSIP_eServiceAtscDigital,
    B_PSIP_eServiceAtscAudio,
    B_PSIP_eServiceAtscData,
    B_PSIP_eServiceUnknown
} B_PSIP_ServiceType;

/***************************************************************************
Summary:
    Rating dimension/value pair
***************************************************************************/
typedef struct B_PSIP_RatingDimVal
{
    uint8_t     ratingDim;
    uint8_t     ratingVal;
} B_PSIP_RatedDimension;

/***************************************************************************
Summary:
    Structure defining a single program rating
***************************************************************************/
typedef struct B_PSIP_Rating
{
    uint8_t               description[MAX_RATING_STR];
    uint8_t               region;
    uint8_t               numDims;
    B_PSIP_RatedDimension dim[MAX_RATING_DIMS];
} B_PSIP_Rating;

/***************************************************************************
Summary:
    Structure defining closed captioning service data
***************************************************************************/
typedef struct B_PSIP_CaptionService
{
    uint8_t     language[3];
    bool        ccType;
    union
    {
        bool    line21Field;
        uint8_t captionServiceNumber;
    } cc;
    bool        easyReader;
    bool        wideAspectRatio;
} B_PSIP_CaptionService;

/***************************************************************************
Summary:
    Structure defining all caption services for a given program
***************************************************************************/
typedef struct B_PSIP_Captions
{
    uint8_t               numCaptions;
    B_PSIP_CaptionService captionService[MAX_CAPTION_SERVICES];
} B_PSIP_Captions;

/***************************************************************************
Summary:
    Structure containing the raw ac3 audio descriptors for a given program
***************************************************************************/
typedef struct B_PSIP_AC3Audio
{
    uint8_t descriptor[MAX_AC3_AUDIO_DESC_LEN];
} B_PSIP_AC3Audio;


/***************************************************************************
Summary:
    Structure defining all genres for a given program
***************************************************************************/
typedef struct B_PSIP_Genres
{
    uint8_t numGenres;
    uint8_t genre[MAX_GENRES];
} B_PSIP_Genres;

/***************************************************************************
Summary:
    Structure defining a single program
***************************************************************************/
typedef struct B_PSIP_Program
{
    uint32_t           startTime;
    uint32_t           length;
    uint16_t           eventId;
    uint8_t            title[MAX_TITLE_CHARS];
    uint8_t            description[MAX_DESC_CHARS];
    bool               ettAvailable;
    B_PSIP_Rating      rating;
    B_PSIP_Captions    captions;
    uint8_t            numAC3AudioDesc;
    B_PSIP_AC3Audio    AC3Audio[MAX_AC3_DESC];
    B_PSIP_Genres      genres;
} B_PSIP_Program;

/***************************************************************************
Summary:
    Structure defining a time shifted service
***************************************************************************/
typedef struct B_PSIP_TimeShift
{
	uint16_t		timeShift;
	uint16_t        major;
	uint16_t        minor;
} B_PSIP_TimeShift;

/***************************************************************************
Summary:
    Structure defining a single channel
***************************************************************************/
typedef struct B_PSIP_Channel
{
    uint16_t              major;
    uint16_t              minor;
    bool                  psi;
    uint16_t              pcrPID;
    uint16_t              numVideoPids;
    uint16_t              videoPID[MAX_VIDEO_PIDS];
    uint16_t              numAudioPids;
    uint16_t              audioPID[MAX_AUDIO_PIDS];
    uint8_t               audioLang[MAX_AUDIO_PIDS][3];
    uint16_t              chName[MAX_CH_NAME_CHARS];
    uint8_t               extChName[MAX_LONG_NAME_CHARS];
    bool                  hidden;
    bool                  hideGuide;
	uint8_t               pathSelect;
	uint8_t               OOB;
    B_PSIP_ModulationMode modulation;
    B_PSIP_ServiceType    serviceType;
    uint16_t              sourceId;
    uint16_t              programNum;
	uint8_t				  numTimeShiftServices;
	B_PSIP_TimeShift      timeShift[MAX_TIME_SHIFTS_SERVICES];
} B_PSIP_Channel;

/***************************************************************************
Summary:
    SI Data Collection Callback Function Prototype
***************************************************************************/
typedef B_Error (* B_PSIP_CollectionCallback)(B_PSIP_CollectionRequest * pRequest, void * context);

/***************************************************************************
Summary:
    SI Application "Data Ready" Callback Function Prototype
    This callback is invoked when SI lib has completed acquisition and
    processing of data previously requested by the application 
    (i.e. from an API call).
***************************************************************************/
typedef void (* B_PSIP_AppDataReadyCallback)(B_Error status, void * context);

/***************************************************************************
Summary:
    Structure defining settings which must be set before calling any SI API.
***************************************************************************/
typedef struct B_PSIP_ApiSettings
{
    B_PSIP_Handle                   siHandle;
    B_PSIP_FilterHandle             filterHandle;
    B_PSIP_AppDataReadyCallback     dataReadyCallback;
    void                          * dataReadyCallbackParam;
    uint32_t                        timeout;
} B_PSIP_ApiSettings;


/***************************************************************************
Summary:
    Get the default settings for PSIP instantiation (see B_PSIP_Open())

Description:
    Initialize PSIP settings structure with default values.  In most cases
    these values can be used without modification.

Input:
    None

Output:
    pSettings               Struct containing default PSIP applib settings 

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_GetDefaultSettings(
    B_PSIP_Settings * pSettings 
    );

/***************************************************************************
Summary:
    Get the default settings for the API settings structure

Description:
    Initialize PSIP API settings structure with default values.  

Input:
    None

Output:
    pSettings               Struct containing default PSIP API applib settings 

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_GetDefaultApiSettings(
    B_PSIP_ApiSettings * pSettings
    );

/***************************************************************************
Summary:
    Create a PSIP instance

Description:
    Creates and initializes a PSIP instance

Input:
    pSettings               Struct containing default PSIP applib settings 
    CollectionFunc          Function to be called to retrieve SI data and 
                            save in a given buffer 
    CollectionContext       Parameter to be passed to CollectionFunc() 

Output:
    None

Returns:
    B_PSIP_Handle           Handle to a new instance of PSIP

See Also:
    None
****************************************************************************/
B_PSIP_Handle B_PSIP_Open(
    B_PSIP_Settings           * pSettings,
    B_PSIP_CollectionCallback   CollectionFunc,    
    void                      * CollectionContext 
    );

/***************************************************************************
Summary:
    Destroy a PSIP instance

Description:
    Destroy a PSIP instance

Input:
    si                      Handle to PSIP instance to destroy.

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_Close(
    B_PSIP_Handle si 
    );

/***************************************************************************
Summary:
    Get the settings for the given PSIP instance.

Description:
    Get the settings for the given PSIP instance.

Input:
    si                      Handle to PSIP instance 

Output:
    pSettings               Settings structure containing current PSIP 
                            settings.

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_GetSettings(
    B_PSIP_Handle     si,       
    B_PSIP_Settings * pSettings
    );

/***************************************************************************
Summary:
    Set the settings for the given PSIP instance.

Description:
    Set the settings for the given PSIP instance.

Input:
    si                      Handle to PSIP instance 
    pSettings               Settings structure data to use.

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_SetSettings(
    B_PSIP_Handle     si,
    B_PSIP_Settings * pSettings
    );

/***************************************************************************
Summary:
    This API must be called before the PSIP applib will service any
    incomming PSIP requests.  

Description:
    Start the PSIP applib internal worker thread.

Input:
    si                      Handle to PSIP instance 

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
B_Error B_PSIP_Start(
    B_PSIP_Handle          si
    );

/***************************************************************************
Summary:
    Stops the PSIP applib from servicing any new incomming PSIP requests.

Description:
    Stop the PSIP applib internal worker thread.  This will also cancel any
    pending API jobs.

Input:
    si                      Handle to PSIP instance 

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_Stop(
    B_PSIP_Handle          si 
    );

/***************************************************************************
Summary:
    Cancel the currently executing job.

Description:
    Cancel the currently executing job associated with the given 
    filterHandle.  If the job has already completed, then this API will do
    nothing.
    

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
                            Note: the siHandle and filterHandle are the only
                            2 struct values that are necessary.

Output:
    None

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_Cancel(
    B_PSIP_ApiSettings   * pApiSettings
    );

/***************************************************************************
Summary:
    Process the MSS and retrieve the string corresponding to currently 
    selected language.

Description:
    None

Input:
    si                      Handle to PSIP instance 
    MSS                     MSS string data
    pBufferMSS              Buffer to hold retrieved string
    pLengthMSS              Size of pBufferMSS

Output:
    pBufferMSS              Buffer containing retrieved string
    pLengthMSS              Size of retrieved string

Returns:
    B_ERROR_SUCCESS         On valid string found
    B_ERROR_UNKNOWN         Otherwise

See Also:
    None
****************************************************************************/
B_Error B_PSIP_GetStringFromMSS(
    B_PSIP_Handle          si,
    PSIP_MSS_string        MSS,
    uint8_t              * pBufferMSS,
    uint16_t             * pLengthMSS
    );
 
/***************************************************************************
Summary:
    Structure defining the source of acquired time
***************************************************************************/
typedef enum B_PSIP_TimeSource
{
    B_PSIP_eTS_NONE,
    B_PSIP_eTS_TSID
    /*B_PSIP_eTS_XDS*/
} B_PSIP_TimeSource;

/***************************************************************************
Summary:
    Asynchronous API to get the current UTC time, local offset, 
    local offset source, and Daylight Savings Time indicator.

Description:
    Request the current UTC time, local offset, local offset source, and
    Daylight Savings Time indicator. This call will return before the data
    is actually retrieved.  pApiSettings->dataReadyCallback() will be called 
    when the requested data has been received.
    

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pUtcSecs                Storage for Current UTC time in seconds.
    pLocalOffset            Storage for Local time offset from UTC time.
    pLocalOffsetSource      Storage for Local offset source storage.
    pDst                    Storage for Daylight savings time flag.

Output:
    None

Returns:
	B_ERROR_SUCCESS         If time requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
B_Error B_PSIP_GetTime(
    B_PSIP_ApiSettings      * pApiSettings,
    time_t                  * pUtcSecs,             /* Current UTC time in seconds */
    int                     * pLocalOffset,         /* Local time offset from UTC time */
    B_PSIP_TimeSource       * pLocalOffsetSource,   /* Local offset source storage */
    bool                    * pDst                  /* Daylight savings time flag */
    );

/***************************************************************************
Summary:
    Structure defining how a given table listed in the MGT has changed.
***************************************************************************/
typedef enum B_PSIP_TableChangeType
{
    B_PSIP_eTableStatusInvalid          = 0x00,
    B_PSIP_eTableStatusNoChange         = 0x01,
    B_PSIP_eTableStatusPidChanged       = 0x02,
    B_PSIP_eTableStatusVersionChanged   = 0x04
} B_PSIP_TableChangeType;

/***************************************************************************
Summary:
    Structure defining versioning attributes of a table in the MGT
***************************************************************************/
typedef struct B_PSIP_TableVersion
{
    uint16_t                   tableType;
    uint16_t                   pid;
    uint8_t                    version;
    B_PSIP_TableChangeType     changed;
} B_PSIP_TableVersion;

/***************************************************************************
Summary:
    Asynchronous API to get the versioning info for the currently available
    PSIP tables.

Description:
    Get the versioning info for the currently available PSIP tables.  
    This can be used to quickly determine which PSIP tables have changed.
    pLength should indicate the maximum size of the verList (for 
    terrestrial this may range from 6 to 370.  for cable this may range
    from 2 to 370. Calling code must also fill in the verList[].tableType 
    field for each requested table version to be retrieved.  This allows 
    full control over which exact table versions to compare.  The 
    verList[].changed field will indicate if the associated tableType 
    has changed in the MGT.  

    This call will return before the version info is actually retrieved.  
    pApiSettings->dataReadyCallback() will be called when the version
    info has been received.

    This API can also be called to retrieve the current pids and version for
    each requested table type.  The calling code must first set the 
    table types it is interested in, and set each pid to 0. The table 
    version data, and pid will be copied from the current MGT and marked 
    as changed.

    A typical use-case would be to call this API the first time with each
    node in the list having a pid of 0.  That same verList can be passed in
    on subsequent calls to obtain table change information.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    verList                 List of PSIP tables to monitor for version
                            changes.
    length                  Length of verList

Output:
    None

Returns:
	B_ERROR_SUCCESS         If table versions requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
B_Error B_PSIP_GetTableVersions(
    B_PSIP_ApiSettings      * pApiSettings,
    B_PSIP_TableVersion     * verList,
    uint16_t                  length);

/***************************************************************************
Summary:
    Structure defining the type of channel list
***************************************************************************/
typedef enum B_PSIP_ChannelListType
{
	B_PSIP_eCable,
	B_PSIP_eTerrestrial
} B_PSIP_ChannelListType;

/***************************************************************************
Summary:
    Asynchronous API to get the number of channels in the current channel 
    list.

Description:
    Counts the number of channels in the current channel list.  Channel 
    data such as PIDs, names, etc, are not saved.

    This call will return before the number of channels are actually 
    retrieved.  pApiSettings->dataReadyCallback() will be called when 
    the number of channels has been counted.

    This function is not very efficient when combined with 
    B_PSIP_GetChannelList() because a VCT scan is performed twice.  
    The only way to determine the number of channels is to traverse all 
    sections  of the VCT.  In nearly all cases, it is better to simply 
    allocate enough memory to hold a large channel list and just call 
    B_PSIP_GetChannelList().

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pNumChannels            Storage for total number of channels
    type                    Terrestrial or Cable channel list.

Output:
    None

Returns:
	B_ERROR_SUCCESS         If number of channels requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
void B_PSIP_GetNumChannels(
    B_PSIP_ApiSettings     * pApiSettings,
    uint16_t               * pNumChannels,    /* Number of channels in channel list */
	B_PSIP_ChannelListType   type
    );

/***************************************************************************
Summary:
    Asynchronous API to get the current channel list.

Description:
    Retrieves the current channel list.  

    This call will return before the channel list is actually 
    retrieved.  pApiSettings->dataReadyCallback() will be called when 
    the channel list has been counted.

    ***Important Note for Cable Channel Lists 
    For (B_PSIP_eCable == type), major/minor channel numbers
    must be interpreted properly.  if the major/minor channel numbers 
    are below 1000, then the major/minor pair defines a 2 part channel
    number.  if the 6 most sig bits of the major channel number are
    111111, then use the following formula to calculate the 1 part
    channel number: onePartNumber = (major & 0x00F)<<10 + minor; 

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    chList                  Storage for list of channels.
    pLength                 Size of chList.
    type                    Terrestrial or Cable channel list.

Output:
    None

Returns:
	B_ERROR_SUCCESS         If number of channels requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
void B_PSIP_GetChannelList(
    B_PSIP_ApiSettings     * pApiSettings,
    B_PSIP_Channel         * chList,
    uint16_t               * pLength,
	B_PSIP_ChannelListType   type
    );

/***************************************************************************
Summary:
    Asynchronous API to get the program information associated with the 
    given channel and time.

Description:
    Retrieves the program information associated with the given channel
    and time.

    This call will return before the program data is actually 
    retrieved.  pApiSettings->dataReadyCallback() will be called when 
    the program data has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pChannel                Channel associated with desired program data.
    pProgram                Storage for program data
    pLength                 Number of programs that can be stored in pProgram.
                            Variable will contain the number of programs saved
                            saved in pProgram when API call completes.
    timeUTCStart            Start time associated with desired program data.
                            If timeUTC == 0, then current time will be used.
    timeUTCEnd              End time associated with desired program data.
                            If timeUTC == 0, then current time will be used.

Output:
    None

Returns:
	B_ERROR_SUCCESS         If program information requested successfully.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    None
****************************************************************************/
B_Error B_PSIP_GetProgram(
    B_PSIP_ApiSettings   * pApiSettings,
    B_PSIP_Channel       * pChannel,
    B_PSIP_Program       * pProgram,
    uint16_t             * pLength,
    time_t                 timeUTCStart,
    time_t                 timeUTCEnd
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Program Allocation Table (PAT)

Description:
    Request the Program Allocation Table (PAT).  This call will return before
    the PAT is actually retrieved.  pApiSettings->dataReadyCallback() will be 
    called when the PAT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of PAT. 
    B_ERROR_TIMEOUT         On failed acquisition of PAT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetPAT(
    B_PSIP_ApiSettings   * pApiSettings,
    uint8_t              * pBuffer,
    uint32_t             * pBufferLength
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Program Allocation Table (PAT)

Description:
    Request the Program Map Table (PMT).  This call will return before
    the PMT is actually retrieved.  pApiSettings->dataReadyCallback() will be 
    called when the PAT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of PMT. 
    B_ERROR_TIMEOUT         On failed acquisition of PMT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetPMT(
    B_PSIP_ApiSettings   * pApiSettings,
    uint16_t               pid,
    uint8_t              * pBuffer,
    uint32_t             * pBufferLength
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Master Guide Table (MGT)

Description:
    Request the Master Guide Table (MGT).  This call will return before
    the PMT is actually retrieved.  pApiSettings->dataReadyCallback() will be 
    called when the PAT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of MGT. 
    B_ERROR_TIMEOUT         On failed acquisition of MGT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetMGT(
    B_PSIP_ApiSettings   * pSettings,
    uint8_t              * pBuffer,
    uint32_t             * pBufferLength
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Terrestrial Virtual Channel Table 
    (TVCT)

Description:
    Request the Terrestrial Virtual Channel Table (TVCT).  This call will 
    return before the TVCT is actually retrieved.  
    pApiSettings->dataReadyCallback() will be called when the TVCT has 
    been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of TVCT. 
    B_ERROR_TIMEOUT         On failed acquisition of TVCT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetTVCT(
    B_PSIP_ApiSettings   * pApiSettings,
    uint8_t                section,
    uint8_t              * pBuffer,
    uint32_t             * pBufferLength
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Cable Virtual Channel Table (CVCT)

Description:
    Request the Cable Virtual Channel Table (CVCT).  This call will 
    return before the CVCT is actually retrieved.  
    pApiSettings->dataReadyCallback() will be called when the CVCT has 
    been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of CVCT. 
    B_ERROR_TIMEOUT         On failed acquisition of CVCT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetCVCT(
    B_PSIP_ApiSettings   * pApiSettings,
    uint8_t                section,
    uint8_t              * pBuffer,
    uint32_t             * pBufferLength
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the System Time Table (STT)

Description:
    Request the System Time Table (STT).  This call will return before the 
    STT is actually retrieved.  pApiSettings->dataReadyCallback() will be 
    called when the STT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of STT. 
    B_ERROR_TIMEOUT         On failed acquisition of STT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetSTT(
    B_PSIP_ApiSettings   * pSettings,
    uint8_t              * pBuffer,
    uint32_t             * pBufferLength
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Event Information Table (EIT)

Description:
    Request the Event Information Table (EIT).  This call will return 
    before the EIT is actually retrieved.  pApiSettings->dataReadyCallback() 
    will be called when the EIT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pid                     PID to seach for event information
    sourceId                Indentifies the program information to look for.
    section                 Sections to acquire (EIT can span multiple sections)                 
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of STT. 
    B_ERROR_TIMEOUT         On failed acquisition of STT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetEIT(
    B_PSIP_ApiSettings   * pApiSettings,
    uint16_t               pid,
    uint16_t               sourceId,
    uint8_t                section,
    uint8_t              * pBuffer,
    uint32_t             * pBufferLength
    );

/***************************************************************************
Summary:
    Structure defining the type of extended text message
***************************************************************************/
typedef enum B_PSIP_EtmIdType
{
    B_PSIP_eChannelETM,
    B_PSIP_eEventETM
}B_PSIP_EtmIdType;

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Extended Text Table (ETT)

Description:
    Request the Extended Text Table (ETT).  This call will return 
    before the ETT is actually retrieved.  pApiSettings->dataReadyCallback() 
    will be called when the ETT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pid                     PID to seach for extended text information
    sourceId                Indentifies the program information to look for.
    section                 Sections to acquire (ETT can span multiple sections)                 
    etmType                 Channel or Event ETM.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of ETT. 
    B_ERROR_TIMEOUT         On failed acquisition of ETT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetETT(
    B_PSIP_ApiSettings   * pApiSettings,
    uint16_t               pid,
    uint16_t               sourceId,
    uint16_t               section,
    B_PSIP_EtmIdType       etmType,
    uint8_t              * pBuffer,
    uint32_t             * pBufferLength
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Region Rating Table (RTT)

Description:
    Request the Region Rating Table (RTT).  This call will return 
    before the RTT is actually retrieved.  pApiSettings->dataReadyCallback() 
    will be called when the RTT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of RTT. 
    B_ERROR_TIMEOUT         On failed acquisition of RTT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetRRT(
    B_PSIP_ApiSettings   * pSettings,
    uint8_t              * pBuffer,
    uint32_t             * pBufferLength
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Directed Channel Change Table 
    (DCCT)

Description:
    Request the Directed Channel Change Table (DCCT).  This call will return 
    before the DCCT is actually retrieved.  pApiSettings->dataReadyCallback() 
    will be called when the DCCT has been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of DCCT. 
    B_ERROR_TIMEOUT         On failed acquisition of DCCT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetDCCT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    );

/***************************************************************************
Summary:
    Asynchronous API used to retrieve the Directed Channel Change 
    Selection Code Table (DCCSCT)

Description:
    Request the Directed Channel Change Selection Code Table (DCCSCT).  
    This call will return before the DCCSCT is actually retrieved.  
    pApiSettings->dataReadyCallback() will be called when the DCCSCT has 
    been received.

Input:
    pApiSettings            Struct containing handle to PSIP instance, 
                            handle to filter used to retrieve section data, 
                            callback when data is ready (plus associated 
                            callback param), and a timeout value in seconds.
    pBuffer                 Pointer to a buffer to hold table.
    pBufferLength           Length of pBuffer.

Output:
    None

Returns:
	B_ERROR_SUCCESS         On successful acquisition of DCCSCT. 
    B_ERROR_TIMEOUT         On failed acquisition of DCCSCT.
    B_ERROR_OUT_OF_MEMORY   On out of heap memory error.
    B_ERROR_PSIP_BUSY       If more than one API is called at a time 
                            per filter handle.
    B_ERROR_UNKNOWN         On event or scheduler creation error.

See Also:
    ATSC A/65C - Program and System Information Protocol for Terrestrial
                 Broadcast and Cable (Revision C) With Amendment No. 1.
****************************************************************************/
B_Error B_PSIP_GetDCCSCT(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_PSIP_LIB_H__ */

