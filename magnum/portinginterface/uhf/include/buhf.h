/***************************************************************************
*     Copyright (c) 2005-2013, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Porting Interface for the UHF Receiver Module.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 * ***************************************************************************/

/*= Module Overview *********************************************************
<verbatim>

Overview
The UHF PI module controls the UHF receiver core within the part.

Design
The design for BUHF PI API is broken into two parts.
  Part 1 (open/close/configuration):
    These APIs are used for opening and closing BUHF device/device channel.
  Part 2 (enable device):
    These APIs are used to enable a UHF device type.
  Part 3 (get receiver data):
    These APIs are used to check for data received, read it, and return it
    to the caller.

Usage
The usage of BUHF involves the following:
   * Configure/Open of BUHF
      * Configure BUHF device for the target system
      * Open BUHF device
      * Configure BUHF device channel for the target system
      * Open BUHF device channel
   * Enable device
      * Enable the UHF device type.
   * Check to see if data is received.
   * Get the data received.

Sample Code
void main( void )
{
    BUHF_Handle         hUhf;
    BREG_Handle         hReg;
    BCHP_Handle         hChip;
    BINT_Handle         hInt;
    unsigned char       readyFlag = 0;
    BUHF_Data           data;

    // Do other initialization, i.e. for BREG, BCHP, etc

    // Make any changes required from the default values
    BUHF_Open(&hUhf, hChip, hReg, hInt, (BUHF_Settings *)NULL);

    do
    {
        // Using polling
        BUHF_IsDataReady (hUhf, &readyFlag);
        if (readyFlag)
            BUHF_Read (hUhf, &data);
    } while (readyFlag);
}

Note: an alternate method of determining if data is ready (instead of polling)
is to wait for an event or provide a callback.
The event handle can be retreived with a call to: BUHF_GetEventHandle
The callback can be supplied with a call to: BUHF_RegisterCallback

</verbatim>
***************************************************************************/


#ifndef _BUHF_H_
#define _BUHF_H_


/*=************************ Module Overview ********************************

Overview
--------


***************************************************************************/

#include "bstd.h"
#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"
#if BCHP_CHIP==7400
#include "bchp_7400.h"
#include "bchp_uhfr_2.h"
#elif BCHP_CHIP==7401
#include "bchp_7401.h"
#elif BCHP_CHIP==7403
#include "bchp_7403.h"
#elif BCHP_CHIP==7405
#include "bchp_7405.h"
#elif BCHP_CHIP==7325
#include "bchp_7325.h"
#elif BCHP_CHIP==7340
#include "bchp_7340.h"
#elif BCHP_CHIP==7342
#include "bchp_7342.h"
#elif BCHP_CHIP==7335
#include "bchp_7335.h"
#elif BCHP_CHIP==7420
#include "bchp_7420.h"
#elif BCHP_CHIP==7413
#include "bchp_7413.h"
#elif BCHP_CHIP==7408
#include "bchp_7408.h"
#elif ((BCHP_CHIP == 7422) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 7435) || (BCHP_CHIP == 73465))
#define BUHF_7422_FAMILY 1
#else
#error "Not supported"
#endif

#if (BUHF_7422_FAMILY==1)
#include "bchp_uhfr.h"
#include "bchp_uhfr_intr2.h"
#include "bchp_int_id_uhfr_intr2.h"
#else
#include "bchp_uhfr_1.h"
#include "bchp_uhfr_glbl.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

    
/*{{{ Defines */


#if ((BCHP_CHIP == 7400) || (BCHP_CHIP == 7420))
#define BUHF_MAX_DEVICES  2 /* Number of UHF devices on this chip. */
#else
#define BUHF_MAX_DEVICES  1 /* Number of UHF devices on this chip. */
#endif

#define BUHF_MAX_LOG_SIZE         10    /* Number of entries to be maintained for moving average */
#define BUHF_NUM_FILTER_COEFF     41    /* The number of filter coefficients */
#define BUHF_NUM_FILTERS          2     /* The number of filters. Only 2 at present : LPF and BPF */
#define BUHF_NUM_CHANNELS         9     /* The number of channels supported */  
#define BUHF_PR_CORR_PEAK_THRES   0xa00 /* If DC offset estimate is greater than this value, take it for averaging, else discard */
#define BUHF_FREQ_RATIO_NUM       976   /* (976 / 660) used to calcluate freqOffset from average DC level */     
#define BUHF_FREQ_RATIO_DEN       660   /* (976 / 660) used to calcluate freqOffset from average DC level */     
#define BUHF_FREQ_OFFSET_THRES    1000  /*  Frequency offset threshold */     
#define BUHF_MIN_STEP_FACTOR      5149  /* 27*divN*10**6/2**19 */
      
  
/*}}}*/

/*{{{ Typedefs */

/***************************************************************************
Summary:  
    Indicates UHFR Modes - Advanced and Legacy
    
Description: 
    Part of BUHF_Settings. Set while calling BUHF_Open(). 
    Cannot be changed later.
****************************************************************************/
typedef enum BUHF_Mode
{
    BUHF_Mode_eLegacy = 0,     /* Packet is recieved thru IR path */
    BUHF_Mode_eAdvanced        /* Packet is recieved by UHFR. */
} BUHF_Mode;

  
/***************************************************************************
Summary:   
    Lists UHFR Filter types.
    
Description: 
    Part of BUHF_Settings. Set while calling BUHF_Open(). 
    Cannot be changed later.

****************************************************************************/
typedef enum BUHF_Filter
{
    BUHF_eLPF = 0,  /* Low pass filter */
    BUHF_eBPF       /* Band pass filter */
} BUHF_Filter;


/***************************************************************************
Summary:   
    Preamble types of the recieved data packet

Description: 
    Part of BUHF_Data. Obtained on calling BUHF_Read.
    
****************************************************************************/
typedef enum BUHF_PrType
{
    BUHF_PrType_eNone = 0,   /* Dont know yet. unitialised value */
    BUHF_PrType_e1,          /* Preamble 1 */
    BUHF_PrType_e2           /* Preamble 2 */
} BUHF_PrType;


/***************************************************************************
Summary:  
    Enum for UHFR Channel numbers

Description: 
Channels 1 through 8  are defined for Cust1 UHF remote usage
Channel 9 is for Cust2 remote usage.

    Part of BUHF_Settings. Set while calling BUHF_Open(). 
    Cannot be changed later.    
****************************************************************************/
typedef enum BUHF_ChanNum
{
    BUHF_ChanNum_eChan1 = 0,    /* 369.5 Mhz Channel 1 */
    BUHF_ChanNum_eChan2,        /* 371.1 Mhz Channel 2 */
    BUHF_ChanNum_eChan3,        /* 375.3 Mhz Channel 3 */
    BUHF_ChanNum_eChan4,        /* 376.9 Mhz Channel 4 */
    BUHF_ChanNum_eChan5,        /* 388.3 Mhz Channel 5 */
    BUHF_ChanNum_eChan6,        /* 391.5 Mhz Channel 6 */
    BUHF_ChanNum_eChan7,        /* 394.3 Mhz Channel 7 */
    BUHF_ChanNum_eChan8,        /* 395.9 Mhz Channel 8 */
    BUHF_ChanNum_eChan9         /* 433.92 Mhz Channel 9 */
} BUHF_ChanNum;


/***************************************************************************
Summary:  
    Enum for injection types
    
Description: 
    Part of BUHF_Settings. Set while calling BUHF_Open(). 
    Cannot be changed later.     
****************************************************************************/
typedef enum BUHF_InjType
{
    BUHF_InjType_eLow = 0,    /* Low side injection */
    BUHF_InjType_eHigh        /* High Side injection */
} BUHF_InjType;



#if 0
/* rj: TODO: does it have to be public?? */
/***************************************************************************
Summary:  Maintains status values of different fields required for the Freq 
offset adjustment calculcations
****************************************************************************/
typedef struct BUHF_MovingAvg {
    unsigned int    logCount;       /* Current no. of entries in the log */     
    unsigned char   overflowFlag;   /* Whether log has overflown */ 
    int             prDcOffset[BUHF_MAX_LOG_SIZE];    /* Preamble DC offset: 
                                                         UHFR_COR13.dc_level */
    int             prDcOffsetSum;  /* Sum of DC offset values */
    int             prDcOffsetAvg;  /* Running Average of DC offset values */
} BUHF_MovingAvg;
#endif



/***************************************************************************
Summary:  
    Maintains status of some registers that user may need

Description:
    Status can be obtained by calling BUHF_GetStatus()
    
****************************************************************************/
typedef struct BUHF_Status {
    unsigned int bchError;    /* 0=> no BCH error. 1=> BCH error. UHFR_ISTS[23] */
    unsigned int prCorrPeak;  /* UHCOR13[31:16] */
    int          dcLevel;     /* UHCOR13[15:0] */
    int          slowRssiOut; /* UHLDAS[31:0] */
    
    /* Temporary: only for diagnostic purposes. Remove later */
    unsigned int totalPkt;        /* Number of packets entering the system */
    unsigned int errPkt;          /* Number of incoming packets that have BCH error */
    unsigned int correctedErrPkt; /* Number of packets with BCH error that have been corrected */
    uint32_t     ui32IntStatus;   /* Value of BCHP_UHFR_1_ISTS is saved into it */
} BUHF_Status;

    

/***************************************************************************
Summary:  
    Options to be taken from the user at Device Open
    
Description: 
    Set while calling BUHF_Open().     
****************************************************************************/
typedef struct BUHF_Settings {
    BUHF_Mode     mode;           /* Legacy mode or Advanced mode. Default = Advanced mode. */      
    BUHF_ChanNum  channel;        /* Which channel is the UHFR currently set for. Default = Channel 1*/
    BUHF_InjType  injection;      /* High Side injection or Low side. Default = Low side injection */
    BUHF_Filter   filter;         /* LPF or BPF. Default = LPF */
    volatile bool bchErrorCorrection; /* TRUE: BCH error correction is enabled,
                                         FALSE: Disabled 
                                         Default = False */
    volatile bool bFreqAdjust;    /* TRUE: Frequency adjustment is enabled, 
                                     FALSE: Disabled 
                                     Default = False */
    volatile bool bShiftIf;       /* TRUE: Shift IF to avoid cross-channel 
                                     interference between two receivers.
                                     Can be set to 1 only for 65 nm 
                                     chips with 2 UHF recievers, when both 
                                     recievers are in use. If shift is enabled
                                     when only one UHF reciever is in use, it
                                     may result in invalid behaviour.
                                     bShiftIf is valid only for channels 1 to 8.
                                     FALSE: Disable shift IF 
                                     Default = False */
    unsigned int uiFreqShift;     /* How much to shift the frequency by. 
                                     Please note:
                                     - Used only if bShiftIf is true.
                                     - Both UHF devices MUST be given the 
                                     same shift vaule for proper functioning.
                                     - uiFreqShift should preferably be even 
                                     numbers to ensure calculations are 
                                     accurate down to +/- 1LSB 
                                     - Shift value should be between 160 & 240 KHz*/
} BUHF_Settings;   

/***************************************************************************
Summary:  The data and preamble type

Description: 
    Obtained on calling BUHF_Read()    
****************************************************************************/
typedef struct BUHF_Data {
    unsigned int        value;          /* Value of the COR12 register.
                                           Need to store the data internally 
                                           otherwise it is lost on a Soft Reset*/
    BUHF_PrType         prType;         /* The Preamble type for this data packet: 1 or 2 */

} BUHF_Data;

/***************************************************************************
Summary:  
    Handle to the UHFR device
Description: 
    Obtained on calling BUHF_Open()  
****************************************************************************/
typedef struct BUHF_P_Device * BUHF_Handle;


/***************************************************************************
Summary:
    UHF user callback function

Description:
    The is the user callback function.  It allows a user to register a
    callback function with the UHF PI.  When a UHF interrupt happens,
    this callback function gets called if it's registered.

See Also:
    BUHF_RegisterCallback(), BUHF_UnregisterCallback()

****************************************************************************/
typedef BERR_Code (*BUHF_Callback)(BUHF_Handle hChn, void *pData);

/*}}}*/


/*{{{ Function prototypes */

/***************************************************************************
Summary:
    Initializes & starts the UHFR Device.

Description:
    Allocates memory for the device handle, installs the UHFR interrupt 
    callback, sets up all registers etc required as per the given user settings.

Returns:
    BERR_SUCCESS - if open is successful.
    Error value  - if not successful.

See Also:
    BUHF_Close, BUHF_GetDefaultSettings, BUHF_IsDataReady, BUHF_Read
***************************************************************************/
BERR_Code BUHF_Open (
    BUHF_Handle *phUhfr,                /* [out] Allocated Device handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle */
    unsigned int devNum,                /* [in] UHFR 0 or 1 */
    const BUHF_Settings *pSettings      /* [in] Device settings */
);


/***************************************************************************
Summary:
    Stops and shuts down the UHFR Device.

Description:
    Frees device handle and associated memory. Also uninstalls the UHFR
    interrupt handler.

Returns:
    BERR_SUCCESS - if successful.
    Error value  - if not successful.

See Also:
    BUHF_Open
***************************************************************************/
BERR_Code BUHF_Close(
    BUHF_Handle hUhfr                   /* [in] UHFR Device handle */
);


/***************************************************************************
Summary:
    Check whether the UHFR has data to read. If yes, set flag to 1, else 0
    This PI is avilable only in Advanced mode

Description:
    This function checks the UHFR intertrupt status register bits 
    ISTS_CORR_DECODE_PR1_END and ISTS_CORR_DECODE_PR2_END along with the
    intFlag in the UHFR device's uhfrConfig structure. If any of these is
    true => there is data waiting to be read.   
    
     
Returns:
    BERR_SUCCESS - if successful.
    Error value  - if not successful.

See Also:
    BUHF_Read
***************************************************************************/
BERR_Code BUHF_IsDataReady
(
    BUHF_Handle     hUhfr,      /* [in] UHFR Device handle */
    unsigned char   *pFlag      /* [out] Flag indicating data is available or not 
                                         1=> data available
                                         0=> no data available */
);


/***************************************************************************
Summary:
    Read data from the UHFR device.
    This PI is avilable only in Advanced mode

Description:
    This function gets data from the UHFR decoder. It also clears the 
    interrupt flag in UHFR configuration data structure to indicate that 
    data has been read.

    Rather than reading data from COR12, this function reads data from 
    hUhfr->data; since COR12 gets cleared on a soft reset and may loose
    data.   

Returns:
    BERR_SUCCESS - if successful.
    Error value  - if not successful.

See Also:
    BUHF_IsDataReady
***************************************************************************/
BERR_Code BUHF_Read
(
    BUHF_Handle hUhfr,              /* [in] UHFR Device handle */
    BUHF_Data   *pData              /* [out] ptr to where data is to be saved */
);


/***************************************************************************
Summary:
    Get default settings for the UHFR device. Can be called before the device
    is opened.

Returns:
    BERR_SUCCESS - if successful.
    Error value  - if not successful.

See Also:
    BUHF_Open, BUHF_GetCurrentSettings
***************************************************************************/
BERR_Code BUHF_GetDefaultSettings(
    BUHF_Settings   *pDefSettings       /* [out] Returns default settings */
);



/***************************************************************************
Summary:
    Get current settings for the UHFR device. Can be called only if the device
    is already open.

Returns:
    BERR_SUCCESS - if successful.
    Error value  - if not successful.

See Also:
    BUHF_GetDefaultSettings
***************************************************************************/

BERR_Code BUHF_GetCurrentSettings(
    BUHF_Handle     hUhfr,          /* [in] UHFR Device handle */
    BUHF_Settings   *pSettings      /* [out] Returns current settings */
);




/******************************************************************************
 Summary:
       This function returns the Status of certain registers. 
       This function is available only in Advanced mode.
 
Description:
        This PI fills in the following values in the BUHF_Status structure 
        provided by the user.
          unsigned int bchError;
          unsigned int prCorrPeak;  
          int          dcLevel;    
          int          slowRssiOut;

Returns:
    BERR_SUCCESS - if successful.
    Error value  - if not successful.

*****************************************************************************/
BERR_Code BUHF_GetStatus 
(
    BUHF_Handle   hUhfr,    /* [in] UHFR Device handle */
    BUHF_Status * pStatus   /* [out] ptr to where Status is to be saved */
);

/***************************************************************************
Summary:
    This function gets the event handle for UHF device

Description:
    This function is responsible for getting the event handle. The
    application code should use this function get UHF's event handle, 
    which the application should use to wait for data ready.  The UHF ISR will 
    set the event when it recieves valid data.
    
Returns:
    BERR_SUCCESS
****************************************************************************/
BERR_Code BUHF_GetEventHandle(
    BUHF_Handle hUhfr,          /* [in] Device  handle */
    BKNI_EventHandle *phEvent   /* [out] Returns event handle */
);

typedef struct BUHF_StandbySettings
{
    bool bEnableWakeup;   /* If true, then allows wakeup from standby using UHF. 
                             If false, the device is completely shut down */

    bool bHwKeyFiltering; /* Enables HW key filtering. If disabled, then SW key filtering must be used.
                             This boolean is ignored if bEnableWakeup is false */
                             
    uint32_t ui32KeyPatternValue; /* If bHwKeyFiltering is true then UHF will respond to this
                                     pattern value. The default pattern value corresponds to the Power 
                                     button. Different remotes will have different pattern values. */
} BUHF_StandbySettings;

/***************************************************************************
Summary:
    This function will return default Standby settings for the UHF device.

Returns:
    BERR_SUCCESS
****************************************************************************/

BERR_Code BUHF_GetDefaultStandbySettings(
    BUHF_Handle hUhf,
    BUHF_StandbySettings *pSettings
    );



/***************************************************************************
Summary:
    This function will put the UHF device in Standby mode.

Returns:
    BERR_SUCCESS
****************************************************************************/

BERR_Code BUHF_Standby(
    BUHF_Handle hUhf,
    const BUHF_StandbySettings *pSettings
    );

/***************************************************************************
Summary:
    This function takes UHF device out from Standby mode.

Returns:
    BERR_SUCCESS
****************************************************************************/
BERR_Code BUHF_Resume(BUHF_Handle hUhf);

/******************************************************************************
Summary:
      Configures to one of the 9 RF channels defined .

 Description:
      Depending on the channel selected, we set the values for UHFR Analog 
      Control Register 3 ie fRF, fLO, fVCO, fPI, vco_div, div2, rot_dir, 
      divN[7:0] and fcw[13:0]. 

 Returns:
      BERR_SUCCESS on Success
      eDeviceBadDevice if invalid device number is given
      eDeviceBadParam if invalid channel number or injection value is given
      Error value on other errors.

Returns:
    BERR_SUCCESS - if open is successful.
    Error value  - if not successful. *
*****************************************************************************/
BERR_Code BUHF_SelectRfChan
(
    BUHF_Handle     hUhfr           /* [in] UHFR Device handle */
);

/***************************************************************************
Summary:
    This function registers a callback function with UHF PI

Description:
    This function is used to register a callback function with UHF PI.
    When a UHF interrupt happens, if a callback function has been registered
    for this channel, it will call that function.

Returns:
    TODO:

See Also:

****************************************************************************/
void BUHF_RegisterCallback (
    BUHF_Handle         hUhfr,      /* [in] UHFR Device handle */
    BUHF_Callback       callback,   /* [in] Callback function to register */
    void                *pData      /* [in] Data passed to callback function */
);

/***************************************************************************
Summary:
    This function unregisters a callback function with UHF PI

Description:
    This function is used to unregister a callback function for a
    particular UHF channel.

Returns:
    TODO:

See Also:

****************************************************************************/
void BUHF_UnregisterCallback (
    BUHF_Handle         hUhfr       /* [in] UHFR Device handle */
);

/*}}}*/

#ifdef __cplusplus
}
#endif

#endif  /* _UHFRDEV_H_ */
