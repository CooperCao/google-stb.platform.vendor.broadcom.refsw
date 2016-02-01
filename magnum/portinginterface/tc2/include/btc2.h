/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/*********************** Sample code for 3461 *********************************

</verbatim>

******************* Handle L1 Interrupt from 3461 *****************************

static void App_L1_isr_handler(void *pParam1, int pParam2)
{
    BHAB_Handle hab = (BHAB_Handle)pParam2;
    BHAB_HandleInterrupt_isr(hab);
}

static void Interrupt_event_handler(void *pParam)
{
    BHAB_Handle hab = (BHAB_Handle)pParam;
    BHAB_ProcessInterruptEvent(hab);
}

***************************** Create HAB ************************************
//The application needs to create the instance of this basemodule for 3461/3114/3112 or 3109
//and pass the handle to the pi as mentioned in the sample code below.

BHAB_Handle hab;

static void Hab_create()
{
    BHAB_Settings stHabSettings;
   
    BHAB_3461_GetDefaultSettings(&stHabSettings);

    Populate the HAB settings structure. 

    BHAB_Open( &hab, i2c/spi handle, &stHabSettings);

    //Download code binary for the Acquisition Processon
    BHAB_InitAp(&hab, AP_binary);
    
    Get interrupt_event handle and register interrupt_event callback 

    BHAB_GetInterruptEventHandle(&hab, interrupt_event handle);

    Interrupt_event_handler should be called when interrupt event is set by HAB
}

******************* Notify the app of lock status change ************************
static BTC2_CallbackFunc cbLockChangeFunc(
    BTC2_ChannelHandle hIbChn,          
    void *pParam
    )
{
    bool *lockStatus = (bool *) pParam;

    BTC2_GetAsyncStatus( hIbChn, lockStatus );

    return;
}

**************************************************************************
static BTNR_Handle hTnr; 
static BTC2_Handle hTc2;
static BTC2_ChannelHandle ibChn[2];
static BCHP_Handle hChip3461;
static BREG_Handle hReg3461;


main( void )
{
    BTC2_Settings settings;
    BTC2_ChannelSettings tc2ChnDevSettings;
    BTC2_InbandParam ibParam;
    bool isLock;
    BTNR_3461Ib_Settings tnrIb3461_cfg;
    uint8_t frequency = 765; //in MHZ (for example).
        
    //Open Tuner . . .
    BTNR_3461Ib_GetDefaultSettings(&tnrIb3461_cfg);
    BTNR_3461Ib_Open(&hTnr,&tnrIb3461_cfg, hab);
    
    BTC2_3461_GetDefaultSettings( &settings );
    settings.hGeneric = hab;
    BTC2_Open( hTc2, hChip3461, hReg3461, &settings );

    BTC2_GetChannelDefaultSettings( hTc2, &tc2ChnDevSettings );
    BTC2_OpenChannel( hTc2, &ibChn[0], 0, &tc2ChnDevSettings );

    // Install callback for state change event, where last parameter
    // is for general use.
    BTC2_InstallCallback( ibChn[0], BTC2_Callback_eLockChange, cbLockChangeFunc, (void *) &isLock );

    //Tune Tuner . . .
    BTNR_SetTunerRfFreq(hTnr, frequency, BTNR_TunerMode_eDigital);

    // Acquire
    BTC2_Acquire( ibChn[0], &ibParam );

    while(1)
    {
        // Print the lock status every second
        BKNI_Sleep(1000);
        printf("Lock Status: %s\n", (isLock==true): "locked", "not lock");
    }
}

</verbatim>

**************************************************************************/
#ifndef BTC2_H__
#define BTC2_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"
#include "bmem.h"
#include "btmr.h"
#include "bfec.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Error Codes specific to BTC2

Description:

See Also:

****************************************************************************/
#define BTC2_ERR_NOTAVAIL_MODULATION_TYPE       BERR_MAKE_CODE(BERR_TC2_ID, 0)
#define BTC2_ERR_NOTAVAIL_CHN_NO                BERR_MAKE_CODE(BERR_TC2_ID, 1)
#define BTC2_ERR_SYMBOLRATE_EXCEEDED            BERR_MAKE_CODE(BERR_TC2_ID, 2)
#define BTC2_ERR_POWER_DOWN                     BERR_MAKE_CODE(BERR_TC2_ID, 3) /* This is a BERR_Code to indicate a TC2 api was called while power is not enabled */


/***************************************************************************
Summary:
    The handle for TC2 module.

Description:
    An opaque handle for BTC2 device.

See Also:
    BTC2_Open()

****************************************************************************/
typedef struct BTC2_P_Handle                *BTC2_Handle;

/***************************************************************************
Summary:
    The handle for TC2 module.

Description:
    An opaque handle for BTC2 device channel.

See Also:
    BTC2_OpenChannel()

****************************************************************************/
typedef struct BTC2_P_ChannelHandle         *BTC2_ChannelHandle;


/***************************************************************************
 * Enumerated Type Definitions (General)
 ***************************************************************************/

/***************************************************************************
Summary:
    Enumeration for Modulation Type

Description:
    This enumeration defines the valid modulation types.

See Also:
    BTC2_Acquire(), BTC2_GetAsyncStatus()
****************************************************************************/
typedef enum BTC2_ModulationType
{
    BTC2_ModulationType_eDvbt2,
    BTC2_ModulationType_eDvbc2,
    BTC2_ModulationType_eLast
} BTC2_ModulationType;

/***************************************************************************
Summary:
    Enumeration for Acquisition Mode

Description:
    This enumeration defines the valid acquisition modes.

See Also:
    BTC2_Acquire(), BTC2_GetAsyncStatus()
****************************************************************************/
typedef enum BTC2_AcquireMode
{
    BTC2_AcquireMode_eAuto,
    BTC2_AcquireMode_eManual,
    BTC2_AcquireMode_eScan,
    BTC2_AcquireMode_eLast
} BTC2_AcquireMode;

/***************************************************************************
Summary:
    Enumeration for channel bandwidth

Description:
    This enumeration defines the valid channel bandwidth setting.

See Also:
    BTC2_Acquire(), BTC2_GetAsyncStatus()
****************************************************************************/
typedef enum BTC2_Bandwidth
{
    BTC2_Bandwidth_e8MHz,
    BTC2_Bandwidth_e7MHz,
    BTC2_Bandwidth_e6MHz,
    BTC2_Bandwidth_e5MHz,
    BTC2_Bandwidth_e1p7MHz,
    BTC2_Bandwidth_eLast
} BTC2_Bandwidth;

/***************************************************************************
Summary:
    Enumeration for Callback types

Description:
    This enumeration defines Callback types.

See Also:

****************************************************************************/
typedef enum BTC2_Callback
{
    BTC2_Callback_eLockChange,          /* Callback to notify application of lock change */
    BTC2_Callback_eUpdateGain,          /* Callback to notify application to update gain */ 
    BTC2_Callback_eNoSignal,            /* Callback to notify application there is no signal */ 
    BTC2_Callback_eAsyncStatusReady,    /* Callback to notify application there is no signal */     
    BTC2_Callback_eLast                 /* More may be required */
} BTC2_Callback;

/***************************************************************************
Summary:
    Enumeration for stream types

Description:
    This enumeration indicates the types of the Tx input streams carried 
    within the current T2 super-frame.

See Also:

****************************************************************************/
typedef enum BTC2_StreamType
{
  BTC2_StreamType_eTS, /* Transport Stream */
  BTC2_StreamType_eGS, /* Generic Stream (GSE [i.2] and/or GFPS and/or GCS) but not TS */
  BTC2_StreamType_eMixed, /* Both TS and Generic Stream (i.e. TS and at least one of GSE, GFPS, GCS) */
  BTC2_StreamType_eLast
} BTC2_StreamType;

/***************************************************************************
Summary:
    Enumeration for Guard Interval

Description:
    This enumeration represents the guard interval of the current super-frame.

See Also:

****************************************************************************/
typedef enum BTC2_GuardInterval
{
  BTC2_GuardInterval_e1_32,
  BTC2_GuardInterval_e1_16,
  BTC2_GuardInterval_e1_8,
  BTC2_GuardInterval_e1_4,
  BTC2_GuardInterval_e1_128,
  BTC2_GuardInterval_e19_128,
  BTC2_GuardInterval_e19_256,
  BTC2_GuardInterval_eLast
} BTC2_GuardInterval;

/***************************************************************************
Summary:
    Enumeration for PAPR reduction

Description:
    This enumeration represents the PAPR reduction used

See Also:

****************************************************************************/
typedef enum BTC2_Papr
{
  BTC2_Papr_eNone, /* L1-ACE is used and TR is used on P2 symbols only */
  BTC2_Papr_eACE, /* L1-ACE and ACE only are used */
  BTC2_Papr_eTR, /* L1-ACE and TR only are used */
  BTC2_Papr_eBoth, /* L1-ACE, ACE and TR are used */
  BTC2_Papr_eLast
} BTC2_Papr;

/***************************************************************************
Summary:
    Enumeration for L1-post constellations

Description:
    This enumeration represents the constellation of the L1-post signalling 
    data block.

See Also:

****************************************************************************/
typedef enum BTC2_L1Mod
{
  BTC2_L1Mod_eBpsk, /* BPSK constellation */
  BTC2_L1Mod_eQpsk, /* QPSK constellation */
  BTC2_L1Mod_e16Qam, /* 16-QAM constellation */
  BTC2_L1Mod_e64Qam, /* 64-QAM constellation */
  BTC2_L1Mod_eLast
} BTC2_L1Mod;

/***************************************************************************
Summary:
    Enumeration for L1-post signalling data block

Description:
    This enumeration represents the coding of the L1-post signalling data block.

See Also:

****************************************************************************/
typedef enum BTC2_L1Cod
{
  BTC2_L1Cod_e1_2, /* code rate 1/2 */
  BTC2_L1Cod_eLast
} BTC2_L1Cod;

/***************************************************************************
Summary:
    Enumeration for L1 FEC type

Description:
    This enumeration represents type of the L1 FEC used for the L1-post 
    signalling data block

See Also:

****************************************************************************/
typedef enum BTC2_FecType
{
  BTC2_FecType_e16K, /* LDPC 16K */
  BTC2_FecType_e64K, /* LDPC 64k */
  BTC2_FecType_eLast
} BTC2_FecType;

/***************************************************************************
Summary:
    Enumeration for Pilot Pattern

Description:
    This enumeration represents the scattered pilot pattern used for the data
    OFDM symbols.

See Also:

****************************************************************************/
typedef enum BTC2_PilotPattern
{
  BTC2_PilotPattern_e1, /* PP1 */
  BTC2_PilotPattern_e2, /* PP2 */
  BTC2_PilotPattern_e3, /* PP3 */
  BTC2_PilotPattern_e4, /* PP4 */
  BTC2_PilotPattern_e5, /* PP5 */
  BTC2_PilotPattern_e6, /* PP6 */
  BTC2_PilotPattern_e7, /* PP7 */
  BTC2_PilotPattern_e8, /* PP8 */
  BTC2_PilotPattern_eLast
} BTC2_PilotPattern;

/***************************************************************************
Summary:
    Enumeration for PLP type

Description:
    This enumeration represents the type of the associated PLP.

See Also:

****************************************************************************/
typedef enum BTC2_PlpType
{
  BTC2_PlpType_eCommon, /* Common PLP */
  BTC2_PlpType_eDataType1, /* DATA PLP Type 1 */
  BTC2_PlpType_eDataType2, /* DATA PLP Type 2 */
  BTC2_PlpType_eLast
} BTC2_PlpType;

/***************************************************************************
Summary:
    Enumeration for PLP PAYLOAD type

Description:
    This enumeration represents the type of the payload data carried by the 
    associated PLP.

See Also:

****************************************************************************/
typedef enum BTC2_PlpPayloadType
{
  BTC2_PlpPayloadType_eGFPS, /* GFPS */
  BTC2_PlpPayloadType_eGCS, /* GCS */
  BTC2_PlpPayloadType_eGSE, /* GSE */
  BTC2_PlpPayloadType_eTS, /* TS */
  BTC2_PlpPayloadType_eLast
} BTC2_PlpPayloadType;

/***************************************************************************
Summary:
    Enumeration for PLP Code Rate

Description:
    This enumeration represents the code rate used by the associated PLP.

See Also:

****************************************************************************/
typedef enum BTC2_PlpCodeRate
{
  BTC2_PlpCodeRate_e1_2,
  BTC2_PlpCodeRate_e3_5,
  BTC2_PlpCodeRate_e2_3,
  BTC2_PlpCodeRate_e3_4,
  BTC2_PlpCodeRate_e4_5,
  BTC2_PlpCodeRate_e5_6,
  BTC2_PlpCodeRate_eLast
} BTC2_PlpCodeRate;

/***************************************************************************
Summary:
    Enumeration for PLP modulation

Description:
    This enumeration represents the modulation used by the associated PLP.

See Also:

****************************************************************************/
typedef enum BTC2_PlpModulation
{
  BTC2_PlpModulation_eQpsk,
  BTC2_PlpModulation_e16Qam,
  BTC2_PlpModulation_e64Qam,
  BTC2_PlpModulation_e256Qam,
  BTC2_PlpModulation_eLast
} BTC2_PlpModulation;

/***************************************************************************
Summary:
    Enumeration for Lock Status

Description:
    This enumeration represents the lock status of of a TC2 channel.

See Also:

****************************************************************************/
typedef enum BTC2_LockStatus
{           
    BTC2_LockStatus_eUnlocked,
    BTC2_LockStatus_eLocked, 
    BTC2_LockStatus_eNoSignal,      
    BTC2_LockStatus_eLast    
} BTC2_LockStatus;

/***************************************************************************
Summary:
    Enumeration for selective status types

Description:
    This enumeration represents the  status types for TC2 selective status.

See Also:
****************************************************************************/
typedef enum BTC2_SelectiveAsyncStatusType
{
    BTC2_SelectiveAsyncStatusType_eFecStatisticsL1Pre,
    BTC2_SelectiveAsyncStatusType_eFecStatisticsL1Post,
    BTC2_SelectiveAsyncStatusType_eFecStatisticsPlpA,
    BTC2_SelectiveAsyncStatusType_eFecStatisticsPlpB, 
    BTC2_SelectiveAsyncStatusType_eL1Pre,
    BTC2_SelectiveAsyncStatusType_eL1PostConfigurable,
    BTC2_SelectiveAsyncStatusType_eL1PostDynamic,
    BTC2_SelectiveAsyncStatusType_eL1Plp,
    BTC2_SelectiveAsyncStatusType_eShort,
    BTC2_SelectiveAsyncStatusType_eLast 
} BTC2_SelectiveAsyncStatusType;

/***************************************************************************
Summary:
    Enumeration for spectrum Mode.
****************************************************************************/
typedef enum BTC2_SpectrumMode
{
    BTC2_SpectrumMode_eAuto,
    BTC2_SpectrumMode_eManual,
    BTC2_SpectrumMode_eLast
} BTC2_SpectrumMode;

/***************************************************************************
Summary:
    Enumeration for Inverted or non-nverted spectrum setting.
****************************************************************************/
typedef enum BTC2_InvertSpectrum
{
    BTC2_InvertSpectrum_eNormal,
    BTC2_InvertSpectrum_eInverted,
    BTC2_InvertSpectrum_eLast
} BTC2_InvertSpectrum;

/***************************************************************************
Summary:
    Structure for inband acquisition parameters

Description:
    This structure contains parameters used for an inband channel acquisition.

See Also:
    BTC2_Acquire()

****************************************************************************/
typedef struct BTC2_InbandParam
{
    /* Common Parameters */
    BTC2_ModulationType     modType;                                   
    BTC2_AcquireMode        acqMode;
    BTC2_Bandwidth          bandwidth;
    bool                    plpMode; /* This bit controls whether the output  PLP is  manually or automatically selected.  When plpMode is false(manual mode), the user must supply the desired plpId.  */ 
    uint8_t                 plpId; /* In single-PLP applications, this unsigned number specifies the desired PLP for output by the T2 receiver. 
                                                                       In multiple-PLP applications, this unsigned number specifies the ID for the data  (Type1 or Type2) PLP 
                                                                       and the common (Type0) PLP is automatically determined. */
    bool                    tuneAcquire; /* This bit directs the Downstream Demodulator to initiate an acquire immediately after its associated WFE or Tuner is tuned.  
                                        If the bit is set, then the Demodulator will initiate an acquire immediately after the tuner has completed a tune  regardless of autoacquire.   
                                        Note: Unless otherwise indicated, this flag only applies to the Acquisition Processors Internal WFE or Internal Tuner. */
    bool                    changePlpOnly; /* When this bit is set, the demodulator only changes the PLP of the received signal. It does not do a full acquire. 
                                        There is no need for the application to retune as the frequency does not change. PLP Mode should be set to Manual alond with the new plpId for this field to take effect. 
                                        This is useful only for Multi PLP signals. When a Multi PLP signal is already locked to one of the PLPs in the signal, using changePlpOnly locks faster than a full acquire */                                        
    BTC2_SpectrumMode       spectrumMode;    /* Defines manual or auto spectrum mode. */
    BTC2_InvertSpectrum     invertSpectrum;  /* Defines normal or inverted spectrum selection for acquisition. */
} BTC2_InbandParam;


/***************************************************************************
Summary:
    This structure represents TC2 version.

Description:

See Also:
    BTC2_GetVersion()

****************************************************************************/
typedef struct BTC2_Version
{
    uint16_t    chipId;  /* chip ID */
    uint8_t     majVer;  /* major chip revision number */
    uint8_t     minVer;  /* minor chip revision number */
    uint8_t     apVer;   /* major AP microcode version */
    uint8_t     minApVer;   /* minor AP microcode version */    
} BTC2_Version;


/***************************************************************************
Summary:
    Structure for FEC Statistics

Description:
    This structure contains TC2 FEC Statistics

See Also:
    BTC2_Status, BTC2_GetAsyncStatus()

****************************************************************************/
typedef struct BTC2_FecStatistics
{
    bool                        lock; /* This field indicates the lock status for the given PLP*/
    int32_t                     snrData; /* This signed number represents the Signal-to-Noise Ratio (SNR) computed from slicing the data carriers in units of 1/256th of a dB. */
    uint32_t                    ldpcAvgIter; /* This unsigned number represents the average number of iterations by the LDPC decoder */
    uint32_t                    ldpcTotIter; /* This unsigned number represents the accumulated number of  iterations by the LDPC decoder */   
    uint32_t                    ldpcTotFrm; /* This unsigned number represents the accumulated number of  total frames processed by the LDPC decoder. */
    uint32_t                    ldpcUncFrm; /* This unsigned number represents the accumulated number of  uncorrectable frames processed by the LDPC decoder */
    uint32_t                    ldpcBER; /* This unsigned number represents the LDPC BER as a fraction of 2^31.  The actual LDPC BER is therefore computed by dividing this integer value by 2^31 */
    uint32_t                    bchCorBit; /* This unsigned number represents the accumulated number of  bits corrected by the BCH decoder */
    uint32_t                    bchTotBlk; /* This unsigned number represents the accumulated number of  total blocks processed by the BCH decoder */
    uint32_t                    bchClnBlk; /* This unsigned number represents the accumulated number of  clean blocks processed by the BCH decoder */
    uint32_t                    bchCorBlk; /* This unsigned number represents the accumulated number of  corrected blocks processed by the BCH decoder */  
    uint32_t                    bchUncBlk; /* This unsigned number represents the accumulated number of  uncorrectable blocks processed by the BCH decoder */
} BTC2_FecStatistics;

  
/***************************************************************************
Summary:
    This structure represents the TC2 L1 Pre Status.

Description:
    This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete L1 Pre status of a TC2 channel.

See Also:
    BTC2_GetAsyncStatus()

****************************************************************************/
typedef struct BTC2_L1PreStatus
{
    BTC2_StreamType         streamType;         /* streamType */   
    bool                    bwtExt;             /* This field indicates whether the extended carrier mode is used in the case of 8K, 16K and 32K FFT 
                                                                                         sizes. When this field is set to '1', the extended carrier mode is used. If this field is set to '0', 
                                                                                         the normal carrier mode is used */
    uint8_t                 s1;                 /* This field has the same value as in the P1 signalling. */   
    uint8_t                 s2;                 /* This field has the same value as in the P1 signalling. */   
    bool                    l1RepetitionFlag;   /* This flag indicates whether the dynamic L1-post signalling is provided also for the next frame. */
    BTC2_GuardInterval      guardInterval;      /* This field indicates the guard interval of the current super-frame */
    BTC2_Papr               papr;               /* This field describes what kind of PAPR reduction is used */
    BTC2_L1Mod              l1Mod;              /* This field field describes the coding of the L1-post signalling data block */
    BTC2_L1Cod              l1Cod;              /*  This field describes the coding of the L1-post signalling data block */
    BTC2_FecType            l1FecType;          /* This field indicates the type of the L1 FEC used for the L1-post signalling data block */
    BTC2_PilotPattern       pilotPattern;       /* This 4-bit field indicates the scattered pilot pattern used for the data OFDM symbols. */
    uint8_t                 regenFlag;          /* This 3-bit field indicates how many times the DVB-T2 signal has been re-generated */
    bool                    l1PostExt;          /* This 1-bit field indicates the presence of the L1-post extension field */
    uint8_t                 numRf;              /* This 3-bit field indicates the number of frequencies in the current T2 system */
    uint8_t                 currentRfIndex;     /* If the TFS mode is supported, this 3-bit field indicates the index of the current RF channel
                                                                                         within its TFS structure, between 0 and numRf-1, otherwise it is set to 0 */
    uint8_t                 txIdAvailability;   /* This 8-bit field is used to signal the availability of transmitter identification signals within
                                                                                         the current geographic cell. */
    uint8_t                 numT2Frames;        /* This 8-bit field indicates the number of T2-frames per super-frame. The minimum value of
                                                                                         numT2Frames shall be 2. */
    uint16_t                numDataSymbols;     /* This 12-bit field indicates Ldata= LF - NP2, the number of data OFDM symbols per
                                                                                        T2-frame, excluding P1 and P2. */
    uint16_t                cellId;             /* This is a 16-bit field which uniquely identifies a geographic cell in a DVB-T2 network */
    uint16_t                networkId;          /* This is a 16-bit field which uniquely identifies the current DVB network */
    uint16_t                t2SystemId;         /* This 16-bit field uniquely identifies a T2 system within the DVB network */
    uint32_t                l1PostSize;         /* This 18-bit field indicates the size of the coded and modulated L1-post signalling data block, in OFDM cells. */
    uint32_t                l1PostInfoSize;     /* This 18-bit field indicates the size of the information part of the L1-post signalling data
                                                                                         block, in bits, including the extension field, if present, but excluding the CRC */
} BTC2_L1PreStatus;

/***************************************************************************
Summary:
    This structure represents the TC2 L1 Post Configurable PLP structure.

Description:
    This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete L1 Post Configurable PLP parameters of a TC2 channel.

See Also:
    BTC2_GetAsyncStatus()

****************************************************************************/
typedef struct BTC2_L1PostCfgPlp
{
    uint8_t                 plpId; /* This 8-bit field identifies uniquely a PLP within the T2 system. */
    uint8_t                 plpGroupId; /* This 8-bit field identifies with which PLP group within the T2 system the current PLP is associated. */
    BTC2_PlpType            plpType; /* This 3-bit field indicates the type of the associated PLP */  
    BTC2_PlpPayloadType     plpPayloadType; /* This 5-bit field indicates the type of the payload data carried by the associated PLP */
    bool                    ffFlag; /* This flag is set to '1' if a PLP of type 1 in a TFS system occurs on the same RF channel in each
                                                                    T2-frame. This flag is set to '0' if inter-frame TFS is applied as described in annex E */
    uint8_t                 firstRfIdx; /* This 3-bit field indicates on which RF channel a type 1 data PLP occurs in the first frame of
                                                                          a super-frame in a TFS system. */
    uint8_t                 firstFrameIdx; /* This 8-bit field indicates the IDX of the first frame of the super-frame in which the current PLP occurs. */
    BTC2_PlpCodeRate        plpCodeRate; /* This field represents the code rate used by the associated PLP. */
    BTC2_PlpModulation      plpMod; /* This 3-bit field indicates the modulation used by the associated PLP */
    bool                    plpRotation; /* This flag indicates whether constellation rotation is in use or not by the associatedPLP */
    BTC2_FecType            plpFecType; /* This 2-bit field indicates the FEC type used by the associated PLP */
    uint16_t                plpNumBlocksMax; /* This 10-bit field indicates the maximum value of plpNumBlocksMax */
    uint8_t                 frameInterval; /* This 8-bit field indicates the T2-frame interval within the super-frame for the associated PLP */
    uint8_t                 timeIlLength;
    bool                    timeIlType; /* This 1-bit field indicates the type of time-interleaving */
    bool                    inBandFlag; /* This 1-bit field indicates whether the current PLP carries in-band type A signalling information */
} BTC2_L1PostCfgPlp;

/***************************************************************************
Summary:
    This structure represents the TC2 L1 Post Configurable status.

Description:
    This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete L1 Post Configurable status of a TC2 channel.

See Also:
    BTC2_GetAsyncStatus()

****************************************************************************/
typedef struct BTC2_L1PostConfigurableStatus
{
    uint16_t                subSlicesPerFrame; /* This 15-bit field indicates Nsubslices_total, the total number of sub-slices for the type 2
                                                                                       data PLPs across all RF channels in one T2-frame */
    uint8_t                 numPlp; /* This 8-bit field indicates the number of PLPs carried within the current super-frame */ 
    uint8_t                 numAux; /* This 4-bit field indicates the number of auxiliary streams */
    uint8_t                 auxConfigRFu;
    uint8_t                 fefType; /* This 4-bit field shall indicate the type of the associated FEF part */
    uint8_t                 rfIdx; /* This 3-bit field indicates the index of each FREQUENCY listed within this loop */
    uint8_t                 fefInterval; /* This 8-bit field indicates the number of T2-frames between two FEF parts */
    uint32_t                frequency; /* This 32-bit field indicates the centre frequency in Hz of the RF channel whose index is rfIdx */
    uint32_t                fefLength; /* This 22-bit field indicates the length of the associated FEF part as the number of elementary
                                                                         periods T, from the start of the P1 symbol of the FEF part to the start of the P1 symbol of the
                                                                         ext T2-frame*/
    BTC2_L1PostCfgPlp       plpA; /* PLP loop A */
    BTC2_L1PostCfgPlp       plpB; /* PLP loop B */
    uint8_t                 auxStreamType; /* This 4-bit field indicates the type of the current auxiliary stream */
    uint32_t                auxPrivateConf; /* This 28-bit field is for future use for signalling auxiliary streams */
} BTC2_L1PostConfigurableStatus;

/***************************************************************************
Summary:
    This structure represents the TC2 L1 Post Dynamic PLP structure.

Description:
    This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete L1 Post Dynamic PLP parameters of a TC2 channel.

See Also:
    BTC2_GetAsyncStatus()

****************************************************************************/
typedef struct BTC2_L1PostDynamicPlp
{
    uint8_t                 plpId;
    uint16_t                plpNumBlocks; /* This 10-bit field indicates the number of FEC blocks contained in the current
                                                                              Interleaving Frame for the current PLP */
    uint32_t                plpStart; /* This 22-bit field indicates the start position of the associated PLP within the current T2-frame
                                                                        using the cell addressing scheme */
} BTC2_L1PostDynamicPlp;

/***************************************************************************
Summary:
    This structure represents the TC2 L1 Post Dynamic status.

Description:
    This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete L1 Post Dynamic status of a TC2 channel.

See Also:
    BTC2_GetAsyncStatus()

****************************************************************************/
typedef struct BTC2_L1PostDynamicStatus
{
    uint8_t                 frameIdx; /* This 8-bit field is the index of the current T2-frame within a super-frame */
    uint8_t                 l1ChanlgeCounter; /* This 8-bit field indicates the number of super-frames ahead where the configuration will change */
    uint8_t                 startRfIdx; /* This 3-bit field indicates the ID of the starting frequency of the TFS scheduled frame, for the next T2-frame */
    uint32_t                subSliceInterval; /* This 22-bit field indicates the number of OFDM cells from the start of one sub-slice of one
                                                                                     PLP to the start of the next sub-slice of the same PLP on the same RF channel for the current T2-frame */
    uint32_t                type2Start;/* This 22-bit field indicates the start position of the first of the type 2 PLPs using the cell addressing scheme */
    BTC2_L1PostDynamicPlp   plpA; /* plp loop A */
    BTC2_L1PostDynamicPlp   plpB; /* plp loop B */
    uint32_t                auxPrivateDyn_47_32; /* This 48-bit field is reserved for future use for signalling auxiliary streams. */
    uint32_t                auxPrivateDyn_31_0; /* This 48-bit field is reserved for future use for signalling auxiliary streams. */
} BTC2_L1PostDynamicStatus;

/***************************************************************************
Summary:
    This structure represents the TC2 PLP.

Description:
    This structure is returned when BTC2_GetAsyncStatus()  is called.  This
    structure contains the PLP informationl.

See Also:
    BTC2_GetAsyncStatus()

****************************************************************************/
typedef struct BTC2_Plp
{
    uint8_t                 plpId;
    uint8_t                 plpGroupId;
    BTC2_PlpPayloadType     plpPayloadType;
    BTC2_PlpType            plpType;
} BTC2_Plp;


/***************************************************************************
Summary:
    This structure represents the TC2 L1 Post Configurable status.

Description:
    This structure is returned when BTC2_GetAsyncStatus()  is called.  This
    structure contains the complete L1 Post Configurable status of a TC2 channel.

See Also:
    BTC2_GetAsyncStatus()

****************************************************************************/
typedef struct BTC2_L1PlpStatus
{
    uint8_t                 numPlp; /* number of PLPs found in the RF signal */
    BTC2_Plp                plp[255];
} BTC2_L1PlpStatus;

/***************************************************************************
Summary:
    This structure represents the TC2 Status for a TC2 channel.

Description:
    This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete status of TC2
    channel.

See Also:
    BTC2_GetAsyncStatus()

****************************************************************************/
typedef struct BTC2_Status
{
    BTC2_ModulationType         modType; /* Modulation type */
    BTC2_AcquireMode            acquisitionMode;    
    BTC2_Bandwidth              bandwidth;    
    bool                        lock;
    bool                        isSpectrumInverted; /* If 1, Spectrum is inverted relative to the AP bootup. */   
    uint32_t                    reacqCount; /* Number of reacquisitions performed */                 
    int32_t                     snrEstimate; /* This unsigned number represents the Signal-to-Noise Ratio (SNR) in units of 1/256th of a dB.  The SNR in dB is therefore achieved by dividing the unsigned integer value by 256 */
    int32_t                     gainOffset; /* internal AGC gain offset in units of 1/256th of a dB */                          
    int32_t                     carrierFreqOffset; /* carrier frequency offset in units of Hz */                        
    int32_t                     timingOffset; /* timing frequency offset in units of Hz */           
    int16_t                     signalStrength; /* represents the entire AGC (LNA+TNR+UFE+T2)gain in units of 1/100th of a dBm */
    BTC2_FecStatistics          l1PreStatistics; /* This Structure contains the lock status, SNR and FEC Statistics for L1 Pre */
    BTC2_FecStatistics          l1PostStatistics; /* This Structure contains the lock status, SNR and FEC Statistics for L1 Post */
    BTC2_FecStatistics          plpAStatistics; /* This Structure contains the lock status, SNR and FEC Statistics for PLPA */
    BTC2_FecStatistics          plpBStatistics; /* This Structure contains the lock status, SNR and FEC Statistics for PLPB */
    BTC2_L1PreStatus            l1PreStatus; /* This Structure contains the decoded L1 Pre parameters */
    BTC2_L1PostConfigurableStatus l1PostCfgStatus; /* This Structure contains the decoded L1 Post Configurable parameters */
    BTC2_L1PostDynamicStatus    l1PostDynamicStatus; /* This Structure contains the decoded L1 Post Dynamic parameters */
    BTC2_L1PlpStatus            l1PlpStatus; /* This Structure contains a list of the PLPs found on the RF signal */
} BTC2_Status;

/***************************************************************************
Summary:
    This structure represents TC2 short Status

Description:
    This structure is returned when BTC2_GetSelectiveAsyncStatus() is called.

See Also:
    BTC2_GetAsyncStatus()
****************************************************************************/
typedef struct BTC2_ShortStatus
{
    BTC2_ModulationType         modType; /* Modulation type */
    BTC2_AcquireMode            acquisitionMode;    
    BTC2_Bandwidth              bandwidth;    
    bool                        lock;
    bool                        isSpectrumInverted; /* If 1, Spectrum is inverted relative to the AP bootup. */   
    uint32_t                    reacqCount; /* Number of reacquisitions performed */                 
    int32_t                     snrEstimate; /* This unsigned number represents the Signal-to-Noise Ratio (SNR) in units of 1/256th of a dB.  The SNR in dB is therefore achieved by dividing the unsigned integer value by 256 */
    int32_t                     gainOffset; /* internal AGC gain offset in units of 1/256th of a dB */                          
    int32_t                     carrierFreqOffset; /* carrier frequency offset in units of Hz */                        
    int32_t                     timingOffset; /* timing frequency offset in units of Hz */           
    int16_t                     signalStrength; /* represents the entire AGC (LNA+TNR+UFE+T2)gain in units of a dBm */
    uint8_t                     signalLevelPercent; /* Signal Level Percent represents the Signal Strength of the received signal in percent */
    uint8_t                     signalQualityPercent; /* Signal Quality Percent represents the Signal Quality of the received signal in percent */   
} BTC2_ShortStatus;

/***************************************************************************
Summary:
    This structure represents the status ready types for selective status

Description:
    This structure is returned when BTC2_GetSelectiveAsyncStatusReadyType() 
    is called. This structure returns the available status types.

See Also:
    BTC2_GetAsyncStatus()
****************************************************************************/
typedef struct BTC2_SelectiveAsyncStatusReady
{
    uint32_t fecStatisticsL1Pre:1;
    uint32_t fecStatisticsL1Post:1;
    uint32_t fecStatisticsPlpA:1;
    uint32_t fecStatisticsPlpB:1;
    uint32_t l1Pre:1;
    uint32_t l1PostConfigurable:1;
    uint32_t l1PostDynamic:1;
    uint32_t l1Plp:1;
    uint32_t shortStatus:1;
} BTC2_SelectiveAsyncStatusReady;

/***************************************************************************
Summary:
    This structure represents the TC2 Selective Status for a TC2 channel.

Description:
    This structure is returned when BTC2_GetSelectiveAsyncStatus() is called.  
    This structure contains selective status of a TC2 channel.

See Also:
    BTC2_Status()
****************************************************************************/
typedef struct BTC2_SelectiveStatus
{
    BTC2_SelectiveAsyncStatusType type;
    union
    {
        BTC2_ShortStatus shortStatus;
        BTC2_FecStatistics fecStatistics;
        BTC2_L1PreStatus   l1PreStatus;
        BTC2_L1PostConfigurableStatus l1PostConfigurableStatus;
        BTC2_L1PostDynamicStatus l1PostDynamicStatus;
        BTC2_L1PlpStatus l1PlpStatus;       
    } status;
} BTC2_SelectiveStatus;

/***************************************************************************
Summary:
    Callback used for event notification.

Description:
    When this PI wants to notify an application, it will call this callback
    function the callback function is registered.

See Also:
    BTC2_InstallLockStateChangeCallback(), BTC2_InstallInterruptCallback()

****************************************************************************/
typedef BERR_Code (*BTC2_CallbackFunc)(void *pParam );


/***************************************************************************
Summary:
    Structure for API function table

Description:
   This structure contains pointers to all public BTC2 functions.

See Also:
    None.

****************************************************************************/
struct BTC2_Settings;
struct BTC2_ChannelSettings;

typedef struct BTC2_ApiFunctTable
{
    BERR_Code (*Open)(BTC2_Handle *, BCHP_Handle, BREG_Handle, BINT_Handle, const struct BTC2_Settings *);
    BERR_Code (*Close)(BTC2_Handle);
    BERR_Code (*Init)(BTC2_Handle);
    BERR_Code (*GetVersion)(BTC2_Handle, BTC2_Version *);
    BERR_Code (*GetVersionInfo)(BTC2_Handle, BFEC_VersionInfo *);
    BERR_Code (*GetTotalChannels)(BTC2_Handle, unsigned int *);
    BERR_Code (*OpenChannel)(BTC2_Handle, BTC2_ChannelHandle *, unsigned int, const struct BTC2_ChannelSettings *);
    BERR_Code (*CloseChannel)(BTC2_ChannelHandle);
    BERR_Code (*GetDevice)(BTC2_ChannelHandle, BTC2_Handle *);
    BERR_Code (*GetChannelDefaultSettings)(BTC2_Handle, unsigned int, struct BTC2_ChannelSettings *);
    BERR_Code (*GetStatus)(BTC2_ChannelHandle, BTC2_Status *);
    BERR_Code (*GetLockStatus)(BTC2_ChannelHandle, BTC2_LockStatus *);
    BERR_Code (*GetSoftDecision)(BTC2_ChannelHandle, int16_t nbrToGet, int16_t *iVal, int16_t *qVal, int16_t *nbrGot );
    BERR_Code (*InstallCallback)(BTC2_ChannelHandle, BTC2_Callback, BTC2_CallbackFunc, void *);
    BERR_Code (*GetDefaultAcquireParams)(BTC2_InbandParam *);    
    BERR_Code (*SetAcquireParams)(BTC2_ChannelHandle, BTC2_InbandParam *);
    BERR_Code (*GetAcquireParams)(BTC2_ChannelHandle, BTC2_InbandParam *);     
    BERR_Code (*Acquire)(BTC2_ChannelHandle, BTC2_InbandParam *);   
    BERR_Code (*EnablePowerSaver)(BTC2_ChannelHandle);  
    BERR_Code (*DisablePowerSaver)(BTC2_ChannelHandle);
    BERR_Code (*ProcessNotification)(BTC2_ChannelHandle, unsigned int event);
    BERR_Code (*ResetStatus)(BTC2_ChannelHandle);
    BERR_Code (*RequestAsyncStatus)(BTC2_ChannelHandle);
    BERR_Code (*GetAsyncStatus)(BTC2_ChannelHandle, BTC2_Status *);
    BERR_Code (*ReadSlave)(BTC2_ChannelHandle, uint8_t chipAddr, uint32_t subAddr, uint8_t subAddrLen, uint32_t *data, uint8_t dataLen);
    BERR_Code (*WriteSlave)(BTC2_ChannelHandle, uint8_t chipAddr, uint32_t subAddr, uint8_t subAddrLen, uint32_t *data, uint8_t dataLen);
    BERR_Code (*RequestSelectiveAsyncStatus)(BTC2_ChannelHandle, BTC2_SelectiveAsyncStatusType);
    BERR_Code (*GetSelectiveAsyncStatusReadyType)(BTC2_ChannelHandle, BTC2_SelectiveAsyncStatusReady *);    
    BERR_Code (*GetSelectiveAsyncStatus)(BTC2_ChannelHandle, BTC2_SelectiveAsyncStatusType, BTC2_SelectiveStatus *);    
} BTC2_ApiFunctTable;

/***************************************************************************
Summary:
    Required default settings structure for TC2 module.

Description:
    The default setting structure defines the default configuration of
    TC2 when the device is opened.

See Also:
    BTC2_Open()

****************************************************************************/
typedef struct BTC2_Settings
{
    unsigned int devId;     /* generic device ID */
    void *hGeneric;         /* generic handle can be used for anything */
    BTC2_ApiFunctTable funcPtr;
    bool isOpenDrain; /* true = Open Drain mode, false = Normal mode. */
    BMEM_Heap_Handle      hHeap;        /* require to be able to share memory with BBS required for 7550 */
    BTMR_Handle           hTmr;        /* for peridic check required for 7550 */
} BTC2_Settings;

/***************************************************************************
Summary:
    Required default settings structure for TC2 module.

Description:
    The default setting structure defines the default configuration of
    TC2 channel when the device channel is opened.

See Also:
    BTC2_OpenChannel()

****************************************************************************/
typedef struct BTC2_ChannelSettings
{  
    bool autoAcquire; /* 325X does auto-reacuire or not*/
} BTC2_ChannelSettings;


/***************************************************************************
Summary:
    This function opens TC2 module.

Description:
    This function is responsible for opening BTC2 module. When BTC2 is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BTC2_Close(), BTC2_OpenChannel(), BTC2_CloseChannel(),

****************************************************************************/
BERR_Code BTC2_Open(
    BTC2_Handle *pTc2,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle, Bcm3250 */
    const BTC2_Settings *pDefSettings   /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes TC2 module.

Description:
    This function is responsible for closing BTC2 module. Closing BTC2
    will free main BTC2 handle. It is required that all opened
    BDQS channels must be closed before calling this function. If this
    is not done, the results will be unpredicable.

Returns:
    TODO:

See Also:
    BTC2_Open(), BTC2_CloseChannel()

****************************************************************************/
BERR_Code BTC2_Close(
    BTC2_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function initialize TC2 module.

Description:
    This function is responsible for initializing BTC2 module. The initialize
    function may do none or one or more of the following:
    - Download FW into TC2 core
    - Startup TC2 core
    - Retrieve information from TC2 core regarding TC2 static configuration
    - etc

Returns:
    TODO:

See Also:
    BTC2_3255_Open(), BTC2_3255_Close()

****************************************************************************/
BERR_Code BTC2_Init(
    BTC2_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the version information.

Returns:
    TODO:

See Also:
    BTC2_Open()

****************************************************************************/
BERR_Code BTC2_GetVersion(
    BTC2_Handle hDev,                   /* [in] Device handle */
    BTC2_Version *pVersion              /* [out] Returns version */
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
    BTC2_Open()

****************************************************************************/
BERR_Code BTC2_GetVersionInfo(
    BTC2_Handle hDev,                   /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    );    
    
/***************************************************************************
Summary:
    This function returns the total number of channels supported by
    TC2 module.

Description:
    This function is responsible for getting total number of channels
    supported by BTC2 module, since BTC2 device is implemented as a
    device channel.

Returns:
    TODO:

See Also:
    BTC2_OpenChannel(), BTC2_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BTC2_GetTotalChannels(
    BTC2_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    );

/***************************************************************************
Summary:
    This function gets default setting for a TC2 module channel.

Description:
    This function is responsible for returning the default setting for
    channel of BTC2. The return default setting is used when opening
    a channel.

Returns:
    TODO:

See Also:
    BTC2_OpenChannel()

****************************************************************************/
BERR_Code BTC2_GetChannelDefaultSettings(
    BTC2_Handle hDev,                   /* [in] Device handle */
    unsigned int channelNo,             /* [in] Channel number to default setting for */
    BTC2_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
    This function opens TC2 module channel.

Description:
    This function is responsible for opening BTC2 module channel. When a
    BTC2 channel is opened, it will create a module channel handle and
    configure the module based on the channel default settings. Once a
    channel is opened, it must be closed before it can be opened again.

Returns:
    TODO:

See Also:
    BTC2_CloseChannel(), BTC2_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BTC2_OpenChannel(
    BTC2_Handle hDev,                   /* [in] Device handle */
    BTC2_ChannelHandle *phChn,          /* [out] Returns channel handle */
    unsigned int channelNo,             /* [in] Channel number to open */
    const BTC2_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    );

/***************************************************************************
Summary:
    This function closes TC2 module channel.

Description:
    This function is responsible for closing BTC2 module channel. Closing
    BTC2 channel it will free BTC2 channel handle. It is required that all
    opened BDQS channels must be closed before closing BTC2.

Returns:
    TODO:

See Also:
    BTC2_OpenChannel(), BTC2_CloseChannel()

****************************************************************************/
BERR_Code BTC2_CloseChannel(
    BTC2_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets TC2 module device handle based on
    the device channel handle.

Description:
    This function is responsible returning BTC2 module handle based on the
    BTC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_GetDevice(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Handle *pTc2                   /* [out] Returns Device handle */
    );

/***************************************************************************
Summary:
    This function gets the status synchronously of TC2 module channel.

Description:
    This function is responsible for synchronously getting the complete status for
    a TC2 module channel. 

Returns:
    TODO:

See Also: BTC2_RequestStatus

****************************************************************************/
BERR_Code BTC2_GetStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function requests the status asynchronously of TC2 module channel.

Description:
    This function is responsible for requesting the status to be calculated asynchronously for
    a TC2 module channel. The Qam frontend is responsible to inform 
    the backend when the status is ready either through an interrupt or by any other predetermined
    method.
    
Returns:
    TODO:

See Also: BTC2_GetAsyncStatus

****************************************************************************/
BERR_Code BTC2_RequestAsyncStatus(
    BTC2_ChannelHandle hChn            /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets the status asynchronously of TC2 module channel.

Description:
    This function is responsible for asynchronously getting the complete status for
    a TC2 module channel. BTC2_RequestAsyncStatus() is called before to
    notify the frontend to calculate the status and notify the backend asynchronously that 
    the status is ready then BTC2_GetAsyncStatus() needs to be called to retrieve the status.

Returns:
    TODO:

See Also: BTC2_RequestAsyncStatus

****************************************************************************/
BERR_Code BTC2_GetAsyncStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:    
    This function gets the lock status for a Tc2  module channel.
    
Description:    
    This function is responsible for getting the lock status  for a TC2 module channel.    

See Also:    
    BTC2_GetAsyncStatus()
****************************************************************************/
BERR_Code BTC2_GetLockStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_LockStatus *pLockStatus         /* [out] Returns lock status */
    );

/***************************************************************************
Summary:
    This function gets the I and Q values for soft decision of a
    TC2 module channel.

Description:
    This function is responsible for getting the I and Q values for soft
    decision of a BTC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_GetSoftDecision(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *iVal,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    );
    
/***************************************************************************
Summary:
    This function returns the default acquire parameters for a specific 
    TC2 module.

Description:
    This function returns the default acquire parameters for a specific 
    TC2 module.

Returns:
    TODO:

See Also:
    BTC2_GetAsyncStatus(), BTC2_GetSoftDecision()

****************************************************************************/ 
BERR_Code BTC2_GetDefaultAcquireParams(
    BTC2_ChannelHandle hChn ,           /* [in] Device channel handle */
    BTC2_InbandParam *ibParams          /* [out] default Inband Parameters */
    );
    
/***************************************************************************
Summary:
    This function sends the acquire parameters for a specific TC2 module.

Description:
    This function sends the acquire parameters for a specific TC2 module.
    This sets the acquire parameters for a specific acquire.

Returns:
    TODO:

See Also:
    BTC2_GetAsyncStatus(), BTC2_GetSoftDecision()

****************************************************************************/
BERR_Code BTC2_SetAcquireParams(
    BTC2_ChannelHandle hChn ,           /* [in] Device channel handle */
    BTC2_InbandParam *ibParams          /* [in] Inband Parameters to use */
    );

/***************************************************************************
Summary:
    This function retrieves the acquire parameters set for a specific TC2 module.

Description:
    This function gets the acquire parameters for a specific TC2 module.

Returns:
    TODO:

See Also:
    BTC2_GetAsyncStatus(), BTC2_GetSoftDecision()

****************************************************************************/ 
BERR_Code BTC2_GetAcquireParams(
    BTC2_ChannelHandle hChn ,           /* [in] Device channel handle */
    BTC2_InbandParam *ibParams          /* [out] Inband Parameters to use */
    );

/***************************************************************************
Summary:
    This function tries to acquire a specific TC2 module.

Description:
    This function starts the process of trying to acquire a specific TC2
    channel. Once the acquistion process is started, the TC2 module will
    notify the application when its lock status changes through a Callback
    function. TC2 will try to maintain lock with the downstream signal. 
    If TC2 loses lock, it will notify the application and automatically 
    try to reacquire the downstream. Once it reacquire downstream
    lock, it will notify the application.

Returns:
    TODO:

See Also:
    BTC2_GetAsyncStatus(), BTC2_GetSoftDecision()

****************************************************************************/
BERR_Code BTC2_Acquire(
    BTC2_ChannelHandle hChn ,           /* [in] Device channel handle */
    BTC2_InbandParam *ibParams          /* [in] Inband Parameters to use */
    );
    
/***************************************************************************
Summary:
    This function enable the power-saver mode.

Description:
    This function is responsible for enabling the downstream receiver
    power-saver mode.  When the BTC2 is in the power-saver mode, the
    TC2 receiver is shutdown. BTC2_Acquire() will
    disables power-saver mode automatically. Also BTC2_DisablePowerSaver()
    can be used to disable power-saver mode.


Returns:
    TODO:

See Also:
    BTC2_Acquire()

****************************************************************************/
BERR_Code BTC2_EnablePowerSaver(
    BTC2_ChannelHandle hChn             /* [in] Device channel handle */
    );


/***************************************************************************
Summary:
    This function disables the power-saver mode.

Description:
    This function is responsible for disabling the downstream receiver
    power-saver mode.  When the BTC2 is in the power-saver mode, the
    TC2 receiver is shutdown. 


Returns:
    TODO:

See Also:
    BTC2_Acquire()

****************************************************************************/
BERR_Code BTC2_DisablePowerSaver(
    BTC2_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function is responsible for processing a notificiation for the specific
    TC2 module channel.

Description:
    This function needs to called when notification is received.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_ProcessNotification(
    BTC2_ChannelHandle hChn,                /* [in] Device channel handle */
    unsigned int event                      /* [in] Event code and event data*/
    );

/***************************************************************************
Summary:
    This function is responsible for installing a callback function.

Description:
    This function installs a callback function.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_InstallCallback(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Callback callbackType,         /* [in] Type of callback */
    BTC2_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    );

/***************************************************************************
Summary:
    This function resets the 3461 device's FEC bit error and block counters.

Description:
    
Returns:
    TODO:

See Also:   
****************************************************************************/
BERR_Code BTC2_ResetStatus(
    BTC2_ChannelHandle hChn     /* [in] Device channel handle */
    );

/******************************************************************************
Summary:
   This function returns the BTC2 interrupt event handle.
Description:
  
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTC2_GetInterruptEventHandle(
   BTC2_Handle hDev,            /* [in] BTC2 handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
);

/******************************************************************************
Summary:
   This function returns the BTC2 interrupt event handle.
Description:
  
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTC2_ProcessInterruptEvent(
   BTC2_Handle hDev            /* [in] BTC2 handle */
);

/******************************************************************************
Summary:
   This function allows the reading from a slave device attached to the demod chip.
Description:
  
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTC2_ReadSlave(
    BTC2_ChannelHandle hChn,     /* [in] Device channel handle */
    uint8_t chipAddr,            /* [in] chip addr of the i2c slave device */
    uint32_t subAddr,            /* [in] sub addr of the register to read from the slave device */
    uint8_t subAddrLen,          /* [in] how many bytes is the sub addr? one to four*/
    uint32_t *data,              /* [out] ptr to the data that we will read from the slave device */
    uint8_t dataLen              /* [in] how many bytes are we going to read? one to four*/
);

/******************************************************************************
Summary:
   This function allows the writing to a slave device attached to the demod chip.
Description:
  
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTC2_WriteSlave(
    BTC2_ChannelHandle hChn,     /* [in] Device channel handle */
    uint8_t chipAddr,            /* [in] chip addr of the i2c slave device */
    uint32_t subAddr,            /* [in] sub addr of the register to read from the slave device */
    uint8_t subAddrLen,          /* [in] how many bytes is the sub addr? one to four*/
    uint32_t *data,              /* [in] ptr to the data that we will write to the slave device */
    uint8_t dataLen              /* [in] how many bytes are we going to write? one to four*/
);

/*******************************************************************************
Summary:
   This function requests the TC2 selective status of the requested type. 

Description:
  
Returns:
   BERR_Code
********************************************************************************/
BERR_Code BTC2_RequestSelectiveAsyncStatus(
    BTC2_ChannelHandle hChn,
    BTC2_SelectiveAsyncStatusType type
);

/*******************************************************************************
Summary:
   This function returns the TC2 selective status ready type of the statuses that
   are ready.
Description:
  
Returns:
   BERR_Code
********************************************************************************/
BERR_Code BTC2_GetSelectiveAsyncStatusReadyType(
    BTC2_ChannelHandle hChn,
    BTC2_SelectiveAsyncStatusReady *ready
);

/*******************************************************************************
Summary:
   This function gets the TC2 Selective status of the requested type.
Description:
  
Returns:
   BERR_Code
********************************************************************************/
BERR_Code BTC2_GetSelectiveAsyncStatus(
    BTC2_ChannelHandle hChn,
    BTC2_SelectiveAsyncStatusType type,
    BTC2_SelectiveStatus *pStatus   /* [out] */
);

#ifdef __cplusplus
}
#endif

#endif

