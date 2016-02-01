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
#ifndef BODS_ISDBT_H__
#define BODS_ISDBT_H__

#include "bods_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Enumeration for channel bandwidth

Description:
    This enumeration defines the valid channel bandwidth settings.

See Also:
    BODS_Acquire(), BODS_GetSelevtiveAsyncStatus()
****************************************************************************/
typedef enum BODS_IsdbtBandwidth
{
    BODS_IsdbtBandwidth_e8MHz,   /* 8 MHz */
    BODS_IsdbtBandwidth_e7MHz,   /* 7 MHz */
    BODS_IsdbtBandwidth_e6MHz,   /* 6 MHz */
    BODS_IsdbtBandwidth_e5MHz,   /* 5 MHz */
    BODS_IsdbtBandwidth_e1p7MHz, /* 1.7 MHz, only valid for DVB-T2 and DVB-C2 standards */
    BODS_IsdbtBandwidth_eLast
} BODS_IsdbtBandwidth;

/***************************************************************************
Summary:
	Enumeration for receiver modulaton type

Description:
	This enum specifies the modulation type of the channel received

See Also:
	None.

****************************************************************************/
typedef enum BODS_IsdbtModulation
{
    BODS_IsdbtModulation_eQpsk,  /* QPSK Modulation */
    BODS_IsdbtModulation_e16Qam, /* 16-QAM Modulation */
    BODS_IsdbtModulation_e64Qam, /* 64-QAM Modulation */
    BODS_IsdbtModulation_eDqpsk, /* DQPSK Modulation */    
    BODS_IsdbtModulation_eLast
} BODS_IsdbtModulation;

/***************************************************************************
Summary:
	Enumeration for ODS Carrier Range

Description:
	This enum specifies the valid carrier frequency acquisition range.

See Also:
	None.

****************************************************************************/
typedef enum BODS_IsdbtCarrierRange
{
    BODS_IsdbtCarrierRange_eNarrow,   /* carrier acquisition range is narrow +/-200kHz */
    BODS_IsdbtCarrierRange_eWide,     /* carrier acquisition range is wide +/-600kHz */
    BODS_IsdbtCarrierRange_eLast
} BODS_IsdbtCarrierRange;

/***************************************************************************
Summary:
	Enumeration for receiver transmission mode

Description:
	This enum specifies the transmission modes supported by the ODS channel.

See Also:
	None.

****************************************************************************/
typedef enum BODS_IsdbtTransmissionMode
{
    BODS_IsdbtTransmissionMode_e2K,  /* 2K Mode */
    BODS_IsdbtTransmissionMode_e8K,  /* 8K Mode */    
    BODS_IsdbtTransmissionMode_e4K,  /* 4K Mode */
    BODS_IsdbtTransmissionMode_eLast
} BODS_IsdbtTransmissionMode;

/***************************************************************************
Summary:
	Enumeration for receiver Co-channel interference(CCI) Mode

Description:
	This enum specifies Co-channel interference(CCI) filter options

See Also:
	None.

****************************************************************************/
typedef enum BODS_IsdbtCciMode
{
    BODS_IsdbtCciMode_eAuto, /* Automatic:  Co-channel interference is automatically detected and the notch filter is enabled appropriately */
    BODS_IsdbtCciMode_eNone, /* Co-channel notch filter disabled */    
    BODS_IsdbtCciMode_eLast
} BODS_IsdbtCciMode;

/***************************************************************************
Summary:
	Enumeration for receiver code rate

Description:
	This enum specifies the code rate of the ODS channel.

See Also:
	None.

****************************************************************************/
typedef enum BODS_IsdbtCodeRate
{
    BODS_IsdbtCodeRate_e1_2,     /* Rate 1/2 */
    BODS_IsdbtCodeRate_e2_3,     /* Rate 2/3 */
    BODS_IsdbtCodeRate_e3_4,     /* Rate 3/4 */
    BODS_IsdbtCodeRate_e5_6,     /* Rate 5/6 */
    BODS_IsdbtCodeRate_e7_8,     /* Rate 7/8 */
    BODS_IsdbtCodeRate_eLast
} BODS_IsdbtCodeRate;

/***************************************************************************
Summary:
    Enumeration for Guard Interval

Description:
    This enumeration represents the guard interval of the current super-frame.

See Also:

****************************************************************************/
typedef enum BODS_IsdbtGuardInterval
{
    BODS_IsdbtGuardInterval_e1_32,   /* Guard 1/32 */
    BODS_IsdbtGuardInterval_e1_16,   /* Guard 1/16 */
    BODS_IsdbtGuardInterval_e1_8,    /* Guard 1/8 */
    BODS_IsdbtGuardInterval_e1_4,    /* Guard 1/4 */
    BODS_IsdbtGuardInterval_e1_128,  /* Guard 1/128 */
    BODS_IsdbtGuardInterval_e19_128, /* Guard 19/128 */
    BODS_IsdbtGuardInterval_e19_256, /* Guard 19/256 */
    BODS_IsdbtGuardInterval_eLast
} BODS_IsdbtGuardInterval;

/***************************************************************************
Summary:
	Enumeration for Transmission Mode and Guard Interval

Description:
	This enum specifies the Transmission Mode and Guard Interval. This field
    controls whether the transmission mode (FFT size) and guard interval
    fraction are automatically-detected from the received signal or specified
    by the user.  This field also specifies various automatic modes 
    corresponding to different allowed transmission mode candidates.

See Also:
	None.

****************************************************************************/
typedef enum BODS_IsdbtOfdmMode
{
    BODS_IsdbtOfdmMode_eAutoBrazil, /* Automatic:  Automatically discover Transmission Mode (FFT Size) and Guard Interval.
                                        All combinations of Transmission Mode={2K,4K,8K} and Guard Interval Fraction={1/32,1/16,1/8,1/4} are allowed. */
    BODS_IsdbtOfdmMode_eAutoJapan,  /* Automatic:  Automatically discover Transmission Mode (FFT Size) and Guard Interval.
                                        Partial Transmission Mode and Guard Interval settings are allowed. */                                            
    BODS_IsdbtOfdmMode_eManual,     /* Manual: Use TransmissionMode and GuardInterval fields. */                                       
    BODS_IsdbtOfdmMode_eLast
} BODS_IsdbtOfdmMode;

/***************************************************************************
Summary:
	Enumeration for ISDB-T time interleaving

Description:
	This enum specifies the ISDB-T time interleaving modes.

See Also:
	None.

****************************************************************************/
typedef enum BODS_IsdbtTimeInterleaving
{
    BODS_IsdbtTimeInterleaving_e0x, /* Time Interleaver Index 0 */
    BODS_IsdbtTimeInterleaving_e1x, /* Time Interleaver Index 1 */
    BODS_IsdbtTimeInterleaving_e2x, /* Time Interleaver Index 2 */
    BODS_IsdbtTimeInterleaving_e3x, /* Time Interleaver Index 3 */
    BODS_IsdbtTimeInterleaving_eLast
}BODS_IsdbtTimeInterleaving;

/***************************************************************************
Summary:
	ISDB-T Acquire Parameters

Description:
	This structure contains parameters used for an ISDB-T channel acquisition.

See Also:
	BODS_IsdbtCciMode

****************************************************************************/
typedef struct BODS_IsdbtLayerParams
{
    BODS_IsdbtModulation        modulation;        /* Manually set Layer modulation type. Only usedd if tmccAcquire=false */
    BODS_IsdbtCodeRate          codeRate;          /* Manually set Layer code rate. Only used if tmccAcquire=false */
    BODS_IsdbtTimeInterleaving  timeInterleaving;  /* Manually set Layer time interleaving. Only used if tmccAcquire=false */
    uint32_t                    numSegments;       /* Manually set Layer number of segments. Only used if tmccAcquire=false */
} BODS_IsdbtLayerParams;

/***************************************************************************
Summary:
	ISDB-T Layer Acquire Parameters

Description:
	ISDB-T Layer Acquire Parameters.

See Also:
	BODS_IsdbtCciMode

****************************************************************************/
typedef struct BODS_IsdbtAcquireParams
{
    BODS_IsdbtBandwidth         bandwidth;          /* bandwidth */
    BODS_IsdbtCciMode           cciMode;            /* Selects co-channel filter options */
    BODS_IsdbtCarrierRange      carrierRange;       /* Set wide or narrow carrier acquisition range */
    BODS_IsdbtOfdmMode          transGuardMode;     /* If Manual, caller will provide transmissionMode and guardInterval, else if true, receiver automatically discovers this info */
    BODS_IsdbtTransmissionMode  transmissionMode;   /* Manually set the transmission mode. Only used if modeGuardAcquire=false */
    BODS_IsdbtGuardInterval     guardInterval;      /* Manually set the guard interval. Only used if modeGuardAcquire=false */
    bool                        tmccAcquire;        /* If false, caller will provide the {partial reception, modulation, code rate, time interleaving, number of segments} TMCC parameters */
    bool                        partialReception;   /* Manually set partial reception. Only used if tmccAcquire=false */
    BODS_IsdbtLayerParams       layerAParams;       /* Manually set Layer A parameters. Only used if tmccAcquire=false */
    BODS_IsdbtLayerParams       layerBParams;       /* Manually set Layer A parameters. Only used if tmccAcquire=false */
    BODS_IsdbtLayerParams       layerCParams;       /* Manually set Layer A parameters. Only used if tmccAcquire=false */
} BODS_IsdbtAcquireParams;

/***************************************************************************
Summary:
	ISDB-T Layer Status

Description:
	ISDB-T Layer Status.

See Also:
	BODS_IsdbtCciMode

****************************************************************************/
typedef struct BODS_IsdbtLayerStatus
{
    BODS_IsdbtModulation        modulation;             /* Layer C modulation type */
    BODS_IsdbtCodeRate          codeRate;               /* Layer C code rate */
    BODS_IsdbtTimeInterleaving  timeInterleaving;       /* Layer C time interleaving */
    uint32_t                    numSegments;            /* Layer C number of segments */
    uint32_t                    rsCorrectedBlocks;      /* Layer C accumulated corrected RS blocks */
    uint32_t                    rsUncorrectedBlocks;    /* Layer C accumlated uncorrected RS blocks */
    uint32_t                    rsCleanBlocks;          /* Layer C accumlated clean RS blocks */
    uint32_t                    rsTotalBlocks;          /* Layer C accumalted total RS blocks */   
    uint8_t                     signalLevelPercent;     /* Signal Level Percent represents the Signal Strength of the received signal in percent */
    uint8_t                     signalQualityPercent;   /* Signal Quality Percent represents the Signal Quality of the received signal in percent */
    bool                        bertSync;               /* This fields indicates the BER module sync state for each layer.  */    
    uint64_t                    bertErrorBits;          /* This 48-bit field indicates the number of errored bits reported by the BER module for each layer. */
    uint64_t                    bertTotalBits;          /* This 48-bit field indicates the total number of bits reported by the BER module for each layer. */
} BODS_IsdbtLayerStatus;
    
/***************************************************************************
Summary:
	Structure for ISDB-T status

Description:
	This structure contains ISDB-T specific status

See Also:
	None.

****************************************************************************/
typedef struct BODS_IsdbtStatus
{
    bool                        receiverLock;            /* Receiver lock indicator */
    bool                        fecLock;                 /* FEC lock indicator */
    bool                        noSignalDetected;        /* OFDM signal detected indicator. Should only be used by channel scan as a flag to move on to the next channel */
    BODS_IsdbtTransmissionMode  transmissionMode;        /* Transmission mode */
    BODS_IsdbtGuardInterval     guardInterval;           /* Guard interval */
    int32_t                     gainOffset;              /* gain offset in units of 1/256th of a dB */
    int32_t                     carrierOffset;           /* Carrier offset in Hz */
    int32_t                     timingOffset;            /* Timing offset in Hz */
    int16_t                     signalStrength;          /* Signal strength in 1/100th of a dBm */    
    int16_t                     snr;                     /* SNR in 1/256th dB */
    bool                        spectrumInverted;        /* Spectral inversion indicator */
    uint32_t                    reacqCount;              /* Number of reacquisitions performed */
    bool                        ews;                     /* Emergency Warning System(EWS) indicator */
    bool                        partialReception;        /* partial reception */
    BODS_IsdbtLayerStatus       layerAStatus;            /* Layer A Status */
    BODS_IsdbtLayerStatus       layerBStatus;            /* Layer b Status */
    BODS_IsdbtLayerStatus       layerCStatus;            /* Layer C Status */  
} BODS_IsdbtStatus;

#ifdef __cplusplus
}
#endif

#endif /* BODS_ISDBT_H__ */

