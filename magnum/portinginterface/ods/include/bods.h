/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * [File Description:]
 *
 ***************************************************************************/

/*********************** Sample code for 3462 *******************************

</verbatim>

******************* Handle L1 Interrupt from 3462 ***************************

static void App_L1_isr_handler(void *pParam1, int pParam2)
{
    BHAB_Handle hab = (BHAB_Handle)pParam1;
    BHAB_HandleInterrupt_isr(hab);
}

static void Interrupt_event_handler(void *pParam)
{
    BHAB_Handle hab = (BHAB_Handle)pParam;
    BHAB_ProcessInterruptEvent(hab);
}

***************************** Create HAB ************************************
// The application needs to create the instance of this basemodule for 3462
// and pass the handle to the pi as mentioned in the sample code below.

BHAB_Handle hab;
BCHP_Handle hChip3462;
BREG_Handle hReg3462;
BINT_Handle hInt3462;
BINT_Settings *pDefSettings;

static void Hab_create()
{
    BREG_I2C_Handle i2cHandle;
    BI2C_Handle hI2c;
    BI2C_ChannelHandle hI2cChan;
    BHAB_Settings stHabSettings;
    BI2C_ChannelSettings pChnDefSettings;
    BKNI_EventHandle hIrqEvent;
    pthread_t pth;
    BKNI_EventHandle threadStartEvent = NULL;
    
    BREG_Open( &hReg3462, pMem, (size_t)(0x00d00004) );
    BCHP_Open7425(&hChip3462, hReg3462);
    pDefSettings = BINT_7425_GetSettings();
    BINT_Open(&hInt3462, hReg3462, pDefSettings);

    BI2C_Open(&hI2c, hChip3462, hReg3462, hInt3462, NULL);
    BI2C_GetChannelDefaultSettings(hI2c, 2, &pChnDefSettings);
    BI2C_OpenChannel(hI2c, &hI2cChan, 2, &pChnDefSettings);  
    BI2C_CreateI2cRegHandle(hI2cChan, &i2cHandle);

    BHAB_3462_GetDefaultSettings(&stHabSettings);

    // Populate the HAB settings structure.
    BHAB_Open( &hab, (void *)i2cHandle, &stHabSettings);

    // Get interrupt_event handle and register interrupt_event callback
    BHAB_GetInterruptEventHandle(&hab, &hIrqEvent);
    
    BKNI_CreateEvent(&threadStartEvent);
     
    // Interrupt_event_handler should be called when interrupt event is set by HAB

   // start a thread that runs bcm3462_background(&hab) - see below   
    pthread_create(&pth, NULL, bcm3462_background, hab);
    
    BKNI_WaitForEvent(threadStartEvent, BKNI_INFINTE);
  
    // Download code binary for the Acquisition Processor
    BHAB_InitAp(&hab, AP_binary);
    
}

// The application must call BAST_ProcessInterruptEvent() every time the
// BAST interrupt event is set.
void bcm3462_background(void *pParam)
{
    BKNI_EventHandle hIrqEvent;
    BHAB_Handle hab = (BHAB_Handle)pParam;    
 
    BKNI_SetEvent(threadStartEvent);
    // Get interrupt_event handle and register interrupt_event callback
    BHAB_GetInterruptEventHandle(&hab, &hIrqEvent);

    while (1)
    {
        BKNI_WaitForEvent(hIrqEvent, BKNI_INFINTE);    
        Interrupt_event_handler(&hab);   
    }
}

******************* Notify the app of lock status change ********************
static BODS_CallbackFunc cbLockChangeFunc(
    void *pParam
    )
{
    BODS_LockStatus lockStatus;
    BODS_ChannelHandle *hChn = (BODS_ChannelHandle *) pParam;

    BODS_GetLockStatus( hChn, &lockStatus );

    // Print the lock status
    printf("Lock Status: %s\n", (lockStatus==BODS_LockStatus_eLocked)?: "locked", "not locked");

    return 0;
}

******************* Notify the app of status ready **************************
static BODS_CallbackFunc cbStatusReadyFunc(
    void *pParam
    )
{
    BODS_SelectiveAsyncStatusReadyType type;
    BODS_SelectiveStatus status;    
    
    BODS_ChannelHandle *hChn = (BODS_ChannelHandle *) pParam;   

    BODS_GetSelectiveAsyncStatusReadyType(hChn, &type);

    BODS_GetSelectiveAsyncStatus(hChn, type, &status);

    return 0;
}

*****************************************************************************
static BTNR_Handle hTnr; 
static BODS_Handle hOds;
static BODS_ChannelHandle hChn[2];
static BCHP_Handle hChip3462;
static BREG_Handle hReg3462;


int main( void )
{
    BODS_Settings settings;
    BODS_ChannelSettings odsChnDefSettings;
    BODS_AcquireParams acquireParams;   
    BTNR_3462Ib_Settings tnrIb3462_cfg;
    uint8_t num_ch, frequency = 602; //in MHZ (for example).
    BODS_Standard standard;
        
    // Open Tuner . . .
    BTNR_3462Ib_GetDefaultSettings(&tnrIb3462_cfg);
    BTNR_3462Ib_Open(&hTnr,&tnrIb3462_cfg, hab);
    
    BODS_3462_GetDefaultSettings( &settings );
    settings.hGeneric = hab;
    BODS_Open( &hOds, hChip3462, hReg3462, &settings );

    BODS_Init(pDevice->ods);
    standard = BOBS_Standard_eDvbt; // for example
    BODS_GetTotalChannels(pDevice->ods, standard, &num_ch);    
    BODS_GetChannelDefaultSettings( hOds, standard, &odsChnDefSettings );
    chNum = 0; // for example
    BODS_OpenChannel( hOds, &hChn[chNum], chNum, &odsChnDefSettings );

    // Install callback for state change event, where last parameter is for general use.
    BODS_InstallCallback( hChn[chNum], BODS_Callback_eLockChange, cbLockChangeFunc, (void *) &hChn[chNum] );
    
    // Install callback for async status ready
    BODS_InstallCallback( hChn[chNum], BODS_Callback_eAsyncStatusReady, cbStatusReadyFunc, (void *) &hChn[chNum] );

    // Tune Tuner . . .
    BTNR_SetTunerRfFreq(hTnr, frequency, BTNR_TunerMode_eDigital);

    // Acquire
    BODS_GetDefaultAcquireParams(hChn[chNum], &odsParam);
    BODS_Acquire( hChn[chNum], &acquireParams );

    BODS_RequestSelectiveAsyncStatus(hChn[chNum], BODS_SelectiveAsyncStatusType_eDvbt);
    
    printf("Press enter to exit\n");    
    getchar();
    
    return 0;    
}

</verbatim>

**************************************************************************/
#ifndef BODS_H__
#define BODS_H__

#include "bods_dvbt.h"
#include "bods_isdbt.h"
#include "bods_dvbt2.h"
#include "bods_dvbc2.h"
#include "bfec.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Callback used for event notification.

Description:
    When this PI wants to notify an application, it will call this callback
    function and the callback function is registered.

See Also:

****************************************************************************/
typedef BERR_Code (*BODS_CallbackFunc)(void *pParam );

/***************************************************************************
Summary:
    Enumeration for BERT Header Length

Description:
    This enumeration defines the valid BERT header lengths.

See Also:
    BODS_Acquire()
****************************************************************************/
typedef enum BODS_BertHeaderLength
{
    BODS_BertHeaderLength_e4Byte, /* strip out 4-byte header */
    BODS_BertHeaderLength_e1Byte, /* strip out 1-byte header (sync byte only) */
    BODS_BertHeaderLength_eLast
} BODS_BertHeaderLength;

/***************************************************************************
Summary:
    Enumeration for BERT Polynomial

Description:
    This enumeration defines the valid BERT polynomials.

See Also:
    BODS_Acquire()
****************************************************************************/
typedef enum BODS_BertPolynomial
{
    BODS_BertPolynomial_e23, /* 23-bit polynomial */
    BODS_BertPolynomial_e15, /* 15-bit polynomial */
    BODS_BertPolynomial_eLast
} BODS_BertPolynomial;

/***************************************************************************
Summary:
    Enumeration for spectrum Mode.
****************************************************************************/
typedef enum BODS_SpectrumMode
{
    BODS_SpectrumMode_eAuto,
    BODS_SpectrumMode_eManual,
    BODS_SpectrumMode_eLast
} BODS_SpectrumMode;

/***************************************************************************
Summary:
    Enumeration for Inverted or non-nverted spectrum setting.
****************************************************************************/
typedef enum BODS_InvertSpectrum
{
    BODS_InvertSpectrum_eNormal,
    BODS_InvertSpectrum_eInverted,
    BODS_InvertSpectrum_eLast
} BODS_InvertSpectrum;

/***************************************************************************
Summary:
    Structure for acquisition parameters

Description:
    This structure contains parameters used for an channel acquisition.

See Also:
    BODS_Acquire()

****************************************************************************/
typedef struct BODS_AcquireParams
{
    /* Common Parameters */
    BODS_AcquisitionMode        acquisitionMode;    /* Acquisition Mode */
    BODS_TuneAcquire            tuneAcquire;        /* This field directs the Downstream Demodulator to initiate an acquire immediately after its associated WFE or Tuner is tuned.  
                                                       If the tuneAcquire is set to Acquire after Tune, then the Demodulator will initiate an acquire immediately after the tuner has completed a tune  regardless of autoacquire.  
                                                       Note: Unless otherwise indicated, this flag only applies to the Acquisition Processors Internal WFE or Internal Tuner. */
    bool                        bertEnable;         /* Enable or disable BER to save power and to avoid returning meaningless numbers given none prbs data */
    BODS_BertHeaderLength       bertHeaderLength;   /* Selects the number of header bytes to be stripped at the BER module input */
    BODS_BertPolynomial         bertPolynomial;     /* PRBS polynomial used by the BER module. */
    bool                        spurDetectAndCancel;/* If true, enables the detection and cancellation of the strongest spur if it crosses a interally fixed threshold. 
                                                                                               By default, this is disabled as it increases the acquisition time. */
    BODS_SpectrumMode           spectrumMode;       /* Defines manual or auto spectrum mode. */
    BODS_InvertSpectrum         invertSpectrum;     /* Defines normal or inverted spectrum selection for acquisition. */
    union
    {
        BODS_DvbtAcquireParams  dvbt;   /* DVB-T Acquire Parameters */
        BODS_IsdbtAcquireParams isdbt;  /* ISDB-T Acquire Parameters */
        BODS_Dvbt2AcquireParams dvbt2;  /* DVB-T2 Acquire Parameters */
        BODS_Dvbc2AcquireParams dvbc2;  /* DVB-C2 Acquire Parameters */         
    } acquireParams;        
} BODS_AcquireParams;

/***************************************************************************
Summary:
    This structure represents the ODS Selective Status for an ODS channel.

Description:
    This structure is returned when BODS_GetSelectiveAsyncStatus() is called.  
    This structure contains selective status of an ODS channel.

See Also:
    BODS_GetSelectiveAsyncStatus()

****************************************************************************/
typedef struct BODS_SelectiveStatus
{
    BODS_SelectiveAsyncStatusType   type;   /* Async Status Type returned */         
    union
    {
        BODS_DvbtStatus                     dvbt;                       /* DVB-T Status */
        BODS_IsdbtStatus                    isdbt;                      /* ISDB-T Status */      
        BODS_Dvbt2FecStatistics             dvbt2FecStatistics;         /* DVB-T2 FEC Statistics */
        BODS_Dvbt2L1PreStatus               dvbt2L1Pre;                 /* DVB-T2 L1 Pre Status */
        BODS_Dvbt2L1PostConfigurableStatus  dvbt2L1PostConfigurable;    /* DVB-T2 L1 Post Configurable Status */
        BODS_Dvbt2L1PostDynamicStatus       dvbt2L1PostDynamic;         /* DVB-T2 L1 Post Dynamic Status */
        BODS_Dvbt2L1PlpStatus               dvbt2L1Plp;                 /* DVB-T2 L1 PLP Status */        
        BODS_Dvbt2ShortStatus               dvbt2Short;                 /* DVB-T2 Short Status */      
        BODS_Dvbc2FecStatistics             dvbc2FecStatistics;         /* DVB-C2 FEC Statistics */
        BODS_Dvbc2L1Part2Status             dvbc2L1Part2;               /* DVB-C2 L1 Part2 Status */        
        BODS_Dvbc2L1DsliceStatus            dvbc2L1Dslice;              /* DVB-C2 L1 Dslice Status */              
        BODS_Dvbc2L1NotchStatus             dvbc2L1Notch;               /* DVB-C2 L1 Notch Status */              
        BODS_Dvbc2L1PlpStatus               dvbc2L1Plp;                 /* DVB-C2 L1 PLP Status */                    
        BODS_Dvbc2ShortStatus               dvbc2Short;                 /* DVB-C2 Short Status */       
    } status;
} BODS_SelectiveStatus;

/***************************************************************************
Summary:
    Required default channel settings structure for ODS module.

Description:
    The default settings structure defines the default configuration of
    ODS channel when the device channel is opened.

See Also:
    BODS_OpenChannel()

****************************************************************************/
typedef struct BODS_ChannelSettings
{  
    unsigned int        channelNo;  /* [in] Channel number to open */
    BODS_Standard       standard;   /* Standard */
    BMEM_Heap_Handle    hHeap;      /* Heap memory */
} BODS_ChannelSettings;

/***************************************************************************
Summary:
    Config settings structure for ODS module.

Description:
    This structure defines the config settings for the ODS module.

See Also:
    BODS_GetConfigSettings(), BODS_SetConfigSettings

****************************************************************************/
typedef struct BODS_ConfigSettings
{
    unsigned numDiversityChannels; /* If numDiversityChannels is set to 1, diversity mode is enabled, where all the tuners/receivers of the chip enter into an "Antenna Diversity" mode. In this
                                      mode, channel 0 of the Frontend chip acts as the primary channel, while the slave channels (channels 1 through N-1) automatically mirror or follow the
                                      primary channel. While diversity mode is enabled, only channel 0 of the Frontend chip is addressable and any commands sent to other channels of the chip
                                      are rejected. For normal mode of operation, numDiversityChannels should be set to 0, where all the channels are enabled */
} BODS_ConfigSettings;

/***************************************************************************
Summary:
    Structure for API function table

Description:
   This structure contains pointers to all public BODS functions.

See Also:
    None.

****************************************************************************/
struct BODS_Settings;   /* ODS Settings */

typedef struct BODS_ApiFunctTable
{
    BERR_Code (*Open)(BODS_Handle *, BCHP_Handle, BREG_Handle, BINT_Handle, const struct BODS_Settings *);
    BERR_Code (*Close)(BODS_Handle);
    BERR_Code (*Init)(BODS_Handle);
    BERR_Code (*GetVersion)(BODS_Handle, BODS_Version *);
    BERR_Code (*GetVersionInfo)(BODS_Handle, BFEC_VersionInfo *);    
    BERR_Code (*GetTotalChannels)(BODS_Handle, BODS_Standard, unsigned int *);
    BERR_Code (*GetChannelDefaultSettings)(BODS_Handle, BODS_Standard, struct BODS_ChannelSettings *);    
    BERR_Code (*OpenChannel)(BODS_Handle, BODS_ChannelHandle *, const struct BODS_ChannelSettings *);
    BERR_Code (*CloseChannel)(BODS_ChannelHandle);
    BERR_Code (*GetDevice)(BODS_ChannelHandle, BODS_Handle *);
    BERR_Code (*GetCapabilities)(BODS_Handle, BODS_FrontendCapabilities *);
    BERR_Code (*GetLockStatus)(BODS_ChannelHandle, BODS_LockStatus *);
    BERR_Code (*GetSoftDecision)(BODS_ChannelHandle, int16_t nbrToGet, int16_t *iVal, int16_t *qVal, int16_t *nbrGot );
    BERR_Code (*InstallCallback)(BODS_ChannelHandle, BODS_Callback, BODS_CallbackFunc, void *);
    BERR_Code (*GetDefaultAcquireParams)(BODS_ChannelHandle, BODS_AcquireParams *);    
    BERR_Code (*SetAcquireParams)(BODS_ChannelHandle, const BODS_AcquireParams *);
    BERR_Code (*GetAcquireParams)(BODS_ChannelHandle, BODS_AcquireParams *);     
    BERR_Code (*Acquire)(BODS_ChannelHandle, BODS_AcquireParams *);   
    BERR_Code (*EnablePowerSaver)(BODS_ChannelHandle, BODS_PowerSaverSettings *);   
    BERR_Code (*DisablePowerSaver)(BODS_ChannelHandle, BODS_PowerSaverSettings *);
    BERR_Code (*ResetStatus)(BODS_ChannelHandle);
    BERR_Code (*RequestSelectiveAsyncStatus)(BODS_ChannelHandle, BODS_SelectiveAsyncStatusType);
    BERR_Code (*GetSelectiveAsyncStatusReadyType)(BODS_ChannelHandle, BODS_SelectiveAsyncStatusReadyType *);    
    BERR_Code (*GetSelectiveAsyncStatus)(BODS_ChannelHandle, BODS_SelectiveAsyncStatusType, BODS_SelectiveStatus *);    
} BODS_ApiFunctTable;

/***************************************************************************
Summary:
    Required default settings structure for ODS module.

Description:
    The default settings structure defines the default configuration of
    ODS when the device is opened.

See Also:
    BODS_Open()

****************************************************************************/
typedef struct BODS_Settings
{
    void                *hGeneric;      /* generic handle can be used for anything */   
    BODS_ApiFunctTable  funcPtr;        /* API function table */
} BODS_Settings;

/***************************************************************************
Summary:
    This function opens the ODS module.

Description:
    This function is responsible for opening BODS module. When BODS is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    BERR_Code : BERR_SUCCESS = the returned BODS_Handle is valid

See Also:
    BODS_Close(), BODS_OpenChannel(), BODS_CloseChannel(),

****************************************************************************/
BERR_Code BODS_Open(
    BODS_Handle *pOds,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle, Bcm3250 */
    const BODS_Settings *pDefSettings   /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes the ODS module.

Description:
    This function is responsible for closing BODS module. Closing BODS
    will free main BODS handle. It is required that all opened
    BODS channels must be closed before calling this function. If this
    is not done, the results will be unpredictable.

Returns:
    BERR_Code

See Also:
    BODS_Open(), BODS_CloseChannel()

****************************************************************************/
BERR_Code BODS_Close(
    BODS_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function initializes the ODS module.

Description:
    This function is responsible for initializing BODS module. The initialize
    function may do none or one or more of the following:
    - Startup ODS core
    - Retrieve information from ODS core regarding ODS static configuration

Returns:
    BERR_Code

See Also:

****************************************************************************/
BERR_Code BODS_Init(
    BODS_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the version information.

Returns:
    BERR_Code

See Also:
    BODS_Open()

****************************************************************************/
BERR_Code BODS_GetVersion(
    BODS_Handle hDev,                   /* [in] Device handle */
    BODS_Version *pVersion              /* [out] Returns version */
    );


/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the core driver version 
    information. It return the majorVersion and minorVersion of the core
    driver.
Returns:
    TODO:

See Also:
    BODS_Open()

****************************************************************************/
BERR_Code BODS_GetVersionInfo(
    BODS_Handle hDev,                        /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    );  
    
/***************************************************************************
Summary:
    This function returns the total number of channels supported by the
    ODS module.

Description:
    This function is responsible for getting total number of channels
    supported by BODS module per standard.

Returns:
    BERR_Code

See Also:
    BODS_OpenChannel(), BODS_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BODS_GetTotalChannels(
    BODS_Handle hDev,                   /* [in] Device handle */
    BODS_Standard standard,             /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    );

/***************************************************************************
Summary:
    Return a list of capabilities for a given ODS module.

Description:
    This function returns a list of capabilities for an ODS module. Because
    many frontends support multiple types of standards, this allows the 
    application to distinguish the capabilities of one frontend versus
    another. If a frontend has multiple capabilities, only one of the modes
    may be used at any given time.
    
Returns:
    BERR_Code
    
See Also:

***************************************************************************/
BERR_Code BODS_GetCapabilities(
    BODS_Handle hDev,                           /* [in] Device handle */
    BODS_FrontendCapabilities *pCapabilities    /* [out] Returns frontend capabilities, all the standards supported by the chip */
    );
    
/***************************************************************************
Summary:
    This function gets the default settings for an ODS module channel.

Description:
    This function is responsible for returning the default settings for
    a BODS channel. The returned default settings is used when opening
    a channel.

Returns:
    BERR_Code

See Also:
    BODS_OpenChannel()

****************************************************************************/
BERR_Code BODS_GetChannelDefaultSettings(
    BODS_Handle hDev,                       /* [in] Device handle */
    BODS_Standard standard,                 /* [in] Device handle */
    BODS_ChannelSettings *pChnDefSettings   /* [out] Returns channel default setting */
    );
    
/***************************************************************************
Summary:
    This function opens ODS module channel.

Description:
    This function is responsible for opening BODS module channel. When a
    BODS channel is opened, it will create a module channel handle and
    configure the module based on the channel default settings. Once a
    channel is opened, it must be closed before it can be opened again.

Returns:
    BERR_Code : BERR_SUCCESS = the returned BODS_ChannelHandle is valid

See Also:
    BODS_CloseChannel(), BODS_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BODS_OpenChannel(
    BODS_Handle hDev,                           /* [in] Device handle */
    BODS_ChannelHandle *phChn,                  /* [out] Returns channel handle */
    const BODS_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    );

/***************************************************************************
Summary:
    This function closes ODS module channel.

Description:
    This function is responsible for closing BODS module channel. Closing
    BODS channel will free BODS channel handle and any resources contained in 
    the channel handle . It is required that all opened BODS channels must be
    closed before closing BODS handle.

Returns:
    BERR_Code

See Also:
    BODS_OpenChannel(), BODS_CloseChannel()

****************************************************************************/
BERR_Code BODS_CloseChannel(
    BODS_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets ODS module device handle based on
    the device channel handle.

Description:
    This function is responsible returning BODS module handle based on the
    BODS module channel.

Returns:
    BERR_Code

See Also:

****************************************************************************/
BERR_Code BODS_GetDevice(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_Handle *pOds                   /* [out] Returns Device handle */
    );

/***************************************************************************
Summary:    
    This function gets the lock status for an ODS  module channel.
    
Description:    
    This function is responsible for getting the lock status  for an ODS 
    module channel.    

Returns:
    BERR_Code
    
See Also:    
    BODS_GetSelevtiveAsyncStatus()
****************************************************************************/
BERR_Code BODS_GetLockStatus(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_LockStatus *pLockStatus        /* [out] Returns lock status */
    );

/***************************************************************************
Summary:
    This function gets the I and Q values for soft decision of an
    ODS module channel.

Description:
    This function is responsible for getting the I and Q values for soft
    decision of a BODS module channel.

Returns:
    BERR_Code

See Also:

****************************************************************************/
BERR_Code BODS_GetSoftDecision(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *iVal,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    );
    
/***************************************************************************
Summary:
    Default Acquire parameters for module channel.

Description:
    This function returns the default acquire parameters for a specific 
    ODS module channel.

Returns:
    BERR_Code

See Also:
    BODS_Acquire(), BODS_GetAcquireParams(), BODS_SetAcquireParams()

****************************************************************************/ 
BERR_Code BODS_GetDefaultAcquireParams(
    BODS_ChannelHandle hChn ,           /* [in] Device channel handle */
    BODS_AcquireParams *acquireParams   /* [out] default Acquire Parameters */
    );
    
/***************************************************************************
Summary:
    Set Acquire parameters for module channel.

Description:
    This function sends the acquire parameters for a specific ODS module channel.

Returns:
    BERR_Code

See Also:
    BODS_GetAcquireParams(), BODS_Acquire()
    
****************************************************************************/
BERR_Code BODS_SetAcquireParams(
    BODS_ChannelHandle hChn ,                   /* [in] Device channel handle */
    const BODS_AcquireParams *acquireParams     /* [in] Acquire Parameters to use */
    );

/***************************************************************************
Summary:
    Get Acquire parameters for module channel.

Description:
    This function gets the acquire parameters for a specific ODS module.

Returns:
    BERR_Code

See Also:
    BODS_SetAcquireParams(), BODS_Acquire()

****************************************************************************/ 
BERR_Code BODS_GetAcquireParams(
    BODS_ChannelHandle hChn ,               /* [in] Device channel handle */
    BODS_AcquireParams *acquireParams       /* [out] Acquire Parameters to use */
    );

/***************************************************************************
Summary:
    This function tries to acquire a specific ODS module channel.

Description:
    This function starts the process of trying to acquire a specific ODS
    channel. Once the acquistion process is started, the ODS module will
    notify the application when it's lock status changes through a Callback
    function. ODS will try to maintain lock with the downstream signal. 
    If ODS loses lock, it will notify the application and automatically 
    try to reacquire the downstream, if autoAcquire is on. Once it reacquires
    downstream lock, it will notify the application.

Returns:
    BERR_Code

See Also:
    BODS_SetAcquireParams(), BODS_GetAcquireParams(), BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
BERR_Code BODS_Acquire(
    BODS_ChannelHandle hChn ,               /* [in] Device channel handle */
    BODS_AcquireParams *acquireParams       /* [in] Acquire Parameters to use */
    );
    
/***************************************************************************
Summary:
    This function enables the power-saver mode.

Description:
    This function is responsible for enabling the downstream receiver
    power-saver mode.  When the BODS is in the power-saver mode, the
    ODS receiver is shutdown. BODS_DisablePowerSaver() API disables the
    power-saver mode.

Returns:
    BERR_Code

See Also:
    BODS_Acquire()

****************************************************************************/
BERR_Code BODS_EnablePowerSaver(
    BODS_ChannelHandle hChn,             /* [in] Device channel handle */
    BODS_PowerSaverSettings *pwrSettings /* [in] Power saver settings */
    );


/***************************************************************************
Summary:
    This function disables the power-saver mode.

Description:
    This function is responsible for disabling the downstream receiver
    power-saver mode. When the BODS is in the power-saver mode, the
    ODS receiver is shutdown. 

Returns:
    BERR_Code

See Also:
    BODS_Acquire()

****************************************************************************/
BERR_Code BODS_DisablePowerSaver(
    BODS_ChannelHandle hChn,             /* [in] Device channel handle */
    BODS_PowerSaverSettings *pwrSettings /* [in] Power saver settings */    
    );

/***************************************************************************
Summary:
    This function is responsible for installing a callback function.

Description:
    This function installs a callback function.

Returns:
    BERR_Code

See Also:

****************************************************************************/
BERR_Code BODS_InstallCallback(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_Callback callbackType,         /* [in] Type of callback */
    BODS_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter sent on callback */
    );

/***************************************************************************
Summary:
    This function resets the 3462 device's FEC bit error and block counters.

Description:
    This function clears the following counters in BODS Status: 
    reacqCount, rsCorrectedBlocks, rsUncorrectedBlocks, rsCleanBlocks,
    rsTotalBlocks, viterbiUncorrectedBits, viterbiTotalbits, viterbiBer and
    preViterbiBer.

Returns:
    TODO:

See Also:
    BODS_RequestSelectiveAsyncStatus(), BODS_GetSelectiveAsyncStatusReadyType(),
    BODS_GetSelectiveAsyncStatus()

****************************************************************************/
BERR_Code BODS_ResetStatus(
    BODS_ChannelHandle hChn             /* [in] Device channel handle */
    );

/*******************************************************************************
Summary:
    Request ODS selective status. 

Description:
    This function requests the ODS selective status of the requested type.
   
Returns:
    BERR_Code
    
See Also:  
    BODS_ResetStatus(), BODS_GetSelectiveAsyncStatusReadyType(),
    BODS_GetSelectiveAsyncStatus()

   
********************************************************************************/
BERR_Code BODS_RequestSelectiveAsyncStatus(
    BODS_ChannelHandle hChn,                 /* [in] Device channel handle */
    BODS_SelectiveAsyncStatusType type       /* [in] Device channel handle */
    );

/*******************************************************************************
Summary:
    Get Selective Status ready type.

Description:
    This function returns the ODS selective status ready type of the statuses that
    are ready.
   
Returns:
    BERR_Code

See Also:  
    BODS_ResetStatus(), BODS_RequestSelectiveAsyncStatus(),
    BODS_GetSelectiveAsyncStatus()   
   
********************************************************************************/
BERR_Code BODS_GetSelectiveAsyncStatusReadyType(
    BODS_ChannelHandle hChn,                    /* [in] Device channel handle */
    BODS_SelectiveAsyncStatusReadyType *ready   /* [in] Device channel handle */
    );

/*******************************************************************************
Summary:
    Get ODS Selective Status of the requested type.
   
Description:
    This function gets the ODS Selective status of the requested type.  

Returns:
    BERR_Code
    
See Also:     
    BODS_ResetStatus(), BODS_RequestSelectiveAsyncStatus(),
    BODS_GetSelectiveAsyncStatusReadyType()

********************************************************************************/
BERR_Code BODS_GetSelectiveAsyncStatus(
    BODS_ChannelHandle hChn,
    BODS_SelectiveAsyncStatusType type,          /* [in] Device channel handle */
    BODS_SelectiveStatus *pStatus                /* [out] */     
    );

/***************************************************************************
Summary:
    This function gets the current ODS config settings.

Description:
    This function returns the current ODS Config Settings.

Returns:
    TODO:

See Also:
    BODS_SetConfigSettings()

****************************************************************************/
BERR_Code BODS_GetConfigSettings(
    BODS_Handle hDev,                 /* [in] Device handle */
    BODS_ConfigSettings *settings     /* [out] ODS config settings. */
    );

/***************************************************************************
Summary:
    This function sets the ODS config  settings.

Description:
    This function sets the current ODS Config Settings.

Returns:
    TODO:

See Also:
    BODS_GetConfigSettings()
****************************************************************************/
BERR_Code BODS_SetConfigSettings(
    BODS_Handle hDev,                   /* [in] Device handle */
    const BODS_ConfigSettings *settings /* [in] ODS config settings. */
    );

#ifdef __cplusplus
}
#endif

#endif

