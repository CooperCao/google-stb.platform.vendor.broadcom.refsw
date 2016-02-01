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
#ifndef BODS_DVBT_H__
#define BODS_DVBT_H__

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
typedef enum BODS_DvbtBandwidth
{
    BODS_DvbtBandwidth_e8MHz,   /* 8 MHz */
    BODS_DvbtBandwidth_e7MHz,   /* 7 MHz */
    BODS_DvbtBandwidth_e6MHz,   /* 6 MHz */
    BODS_DvbtBandwidth_e5MHz,   /* 5 MHz */
    BODS_DvbtBandwidth_e1p7MHz, /* 1.7 MHz, only valid for DVB-T2 and DVB-C2 standards */
    BODS_DvbtBandwidth_eLast
} BODS_DvbtBandwidth;

/***************************************************************************
Summary:
	Enumeration for receiver modulaton type

Description:
	This enum specifies the modulation type of the channel received

See Also:
	None.

****************************************************************************/
typedef enum BODS_DvbtModulation
{
    BODS_DvbtModulation_eQpsk,  /* QPSK Modulation */
    BODS_DvbtModulation_e16Qam, /* 16-QAM Modulation */
    BODS_DvbtModulation_e64Qam, /* 64-QAM Modulation */
    BODS_DvbtModulation_eLast
} BODS_DvbtModulation;

/***************************************************************************
Summary:
	Enumeration for ODS Carrier Range

Description:
	This enum specifies the valid carrier frequency acquisition range.

See Also:
	None.

****************************************************************************/
typedef enum BODS_DvbtCarrierRange
{
    BODS_DvbtCarrierRange_eNarrow,   /* carrier acquisition range is narrow +/-200kHz */
    BODS_DvbtCarrierRange_eWide,     /* carrier acquisition range is wide +/-600kHz */
    BODS_DvbtCarrierRange_eLast
} BODS_DvbtCarrierRange;

/***************************************************************************
Summary:
	Enumeration for receiver transmission mode

Description:
	This enum specifies the transmission modes supported by the ODS channel.

See Also:
	None.

****************************************************************************/
typedef enum BODS_DvbtTransmissionMode
{
    BODS_DvbtTransmissionMode_e2K,  /* 2K Mode */
    BODS_DvbtTransmissionMode_e8K,  /* 8K Mode */    
    BODS_DvbtTransmissionMode_e4K,  /* 4K Mode */
    BODS_DvbtTransmissionMode_eLast
} BODS_DvbtTransmissionMode;

/***************************************************************************
Summary:
	Enumeration for receiver code rate

Description:
	This enum specifies the code rate of the ODS channel.

See Also:
	None.

****************************************************************************/
typedef enum BODS_DvbtCodeRate
{
    BODS_DvbtCodeRate_e1_2,     /* Rate 1/2 */
    BODS_DvbtCodeRate_e2_3,     /* Rate 2/3 */
    BODS_DvbtCodeRate_e3_4,     /* Rate 3/4 */
    BODS_DvbtCodeRate_e5_6,     /* Rate 5/6 */
    BODS_DvbtCodeRate_e7_8,     /* Rate 7/8 */
    BODS_DvbtCodeRate_eLast
} BODS_DvbtCodeRate;

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
typedef enum BODS_DvbtOfdmMode
{
    BODS_DvbtOfdmMode_eAuto,      /* Automatic:  Automatically discover Transmission Mode (FFT Size) and Guard Interval.
                                       All combinations of Transmission Mode={2K,4K,8K} and Guard Interval Fraction={1/32,1/16,1/8,1/4} are allowed. */
    BODS_DvbtOfdmMode_eManual,    /* Manual: Use TransmissionMode and GuardInterval fields. */                                       
    BODS_DvbtOfdmMode_eLast
} BODS_DvbtOfdmMode;

/***************************************************************************
Summary:
	Enumeration for receiver Co-channel interference(CCI) Mode

Description:
	This enum specifies Co-channel interference(CCI) filter options

See Also:
	None.

****************************************************************************/
typedef enum BODS_DvbtCciMode
{
    BODS_DvbtCciMode_eAuto, /* Automatic:  Co-channel interference is automatically detected and the notch filter is enabled appropriately */
    BODS_DvbtCciMode_eNone, /* Co-channel notch filter disabled */    
    BODS_DvbtCciMode_eLast
} BODS_DvbtCciMode;

/***************************************************************************
Summary:
    Enumeration for Guard Interval

Description:
    This enumeration represents the guard interval of the current super-frame.

See Also:

****************************************************************************/
typedef enum BODS_DvbtGuardInterval
{
    BODS_DvbtGuardInterval_e1_32,   /* Guard 1/32 */
    BODS_DvbtGuardInterval_e1_16,   /* Guard 1/16 */
    BODS_DvbtGuardInterval_e1_8,    /* Guard 1/8 */
    BODS_DvbtGuardInterval_e1_4,    /* Guard 1/4 */
    BODS_DvbtGuardInterval_eLast
} BODS_DvbtGuardInterval;

/***************************************************************************
Summary:
	Enumeration for DVB-T receiver's decode mode

Description:
	This enum specifies the hierarchical stream within the DVB-T transmission
    to be decoded.

See Also:
	None.

****************************************************************************/
typedef enum BODS_DvbtDecodeMode
{
    BODS_DvbtDecodeMode_eLowPriority,   /* Low priority decode */
    BODS_DvbtDecodeMode_eHighPriority,  /* High priority decode */
    BODS_DvbtDecodeMode_eLast
} BODS_DvbtDecodeMode;

/***************************************************************************
Summary:
	Enumeration for DVB-T hierarchical mode

Description:
	This enum specifies the hierarchy of a DVB-T channel

See Also:
	None.

****************************************************************************/
typedef enum BODS_DvbtHierarchy
{
    BODS_DvbtHierarchy_e0,  /* select hierarchy = 0 */
    BODS_DvbtHierarchy_e1,  /* select hierarchy = 1 */
    BODS_DvbtHierarchy_e2,  /* select hierarchy = 2 */
    BODS_DvbtHierarchy_e4,  /* select hierarchy = 4 */
    BODS_DvbtHierarchy_eLast
} BODS_DvbtHierarchy;

/***************************************************************************
Summary:
	Enumeration for DVB-T Transmission Parameter Signaling (TPS) mode

Description:
	This enum specifies different DVB-T Transmission Parameter Signaling (TPS)
    modes

See Also:
	None.

****************************************************************************/
typedef enum BODS_DvbtTpsMode
{
    BODS_DvbtTpsMode_eAuto,     /* Automatically discover and apply received TPS parameters */
    BODS_DvbtTpsMode_eManual,   /* Manual: caller will provide the modulation,high priority code rate, low priority code rate, hierarchy TPS parameters */
    BODS_DvbtTpsMode_eLast
} BODS_DvbtTpsMode;

/***************************************************************************
Summary:
	Structure for DVB-T acquisition parameters

Description:
	This structure contains parameters used for an DVB-T channel acquisition.

See Also:
	BODS_DvbtCciMode, BODS_DvbtDecodeMode

****************************************************************************/
typedef struct BODS_DvbtAcquireParams
{
    BODS_DvbtBandwidth          bandwidth;              /* bandwidth */
    BODS_DvbtCciMode            cciMode;                /* Selects co-channel filter options */  
    BODS_DvbtCarrierRange       carrierRange;           /* Set wide or narrow carrier acquisition range */
    BODS_DvbtOfdmMode           transGuardMode;         /* If Manual, caller will provide transmissionMode and guardInterval, else if true, receiver automatically discovers this info */
    BODS_DvbtTransmissionMode   transmissionMode;       /* Manually set the transmission mode. Only used if modeGuardAcquire=false */
    BODS_DvbtGuardInterval      guardInterval;          /* Manually set the guard interval. Only used if modeGuardAcquire=false */
    BODS_DvbtDecodeMode         decodeMode;             /* Selects the hierarchical stream to decode (High Priority or Low Priority) */
    BODS_DvbtTpsMode            tpsMode;                /* If false, caller will provide the {modulation,high priority code rate, low priority code rate, hierarchy } TPS parameters */
    BODS_DvbtModulation         modulation;             /* Manually set modulation type. Only used if tpsAcquire=false */
    BODS_DvbtCodeRate           codeRateHighPriority;   /* Manually set high priority code rate. Only used if tpsAcquire=false */
    BODS_DvbtCodeRate           codeRateLowPriority;    /* Manually set low priority code rate. Only used if tpsAcquire=false */
    BODS_DvbtHierarchy          hierarchy;              /* Manually set hierarchy. Only used if tpsAcquire=false */
} BODS_DvbtAcquireParams;

/***************************************************************************
Summary:
	Structure for DVB-T status

Description:
	This structure represents DVB-T specific status

See Also:
	None.

****************************************************************************/
typedef struct BODS_DvbtStatus
{
    bool                        receiverLock;           /* Receiver lock indicator */
    bool                        fecLock;                /* FEC lock indicator */
    bool                        noSignalDetected;       /* OFDM signal detected indicator. Should only be used by channel scan as a flag to move on to the next channel */
    BODS_DvbtTransmissionMode   transmissionMode;       /* Transmission mode */
    BODS_DvbtGuardInterval      guardInterval;          /* Guard interval */
    int32_t                     gainOffset;             /* gain offset in units of 1/256th of a dB */ 
    int16_t                     signalStrength;         /* Signal strength in 1/100th of a dBm */
    int32_t                     carrierOffset;          /* Carrier offset in Hz */
    int32_t                     timingOffset;           /* Timing offset in Hz */
    int16_t                     snr;                    /* SNR in 1/256th dB */
    bool                        spectrumInverted;       /* Spectral inversion indicator */
    uint32_t                    reacqCount;             /* Number of reacquisitions performed */
    BODS_DvbtDecodeMode         decodeMode;             /* Hierarchical decode mode (priority) that was set in the acquire parameters */
    BODS_DvbtModulation         modulation;             /* Modulation type */
    BODS_DvbtCodeRate           codeRate;               /* code rate */
    BODS_DvbtHierarchy          hierarchy;              /* hierarchy */
    uint16_t                    cellId;                 /* cell Id */
    uint32_t                    rsCorrectedBlocks;      /* accumulated corrected RS blocks */
    uint32_t                    rsUncorrectedBlocks;    /* accumlated uncorrected RS blocks */
    uint32_t                    rsCleanBlocks;          /* accumlated clean RS blocks */
    uint32_t                    rsTotalBlocks;          /* accumalted total RS blocks */
    uint32_t                    viterbiBer;             /* Viterbi bit error rate in 1/2147483648 th unit. To calculate the actual rate, take the value and divide by 2147483648 */
    uint32_t                    preViterbiBer;          /* pre-Viterbi bit error rate in 1/2147483648 th unit. To calculate the actual rate, take the value and divide by 2147483648 */
    uint8_t                     signalLevelPercent;     /* Signal Level Percent represents the Signal Strength of the received signal in percent */
    uint8_t                     signalQualityPercent;   /* Signal Quality Percent represents the Signal Quality of the received signal in percent */
    bool                        inDepthSymbolInterleave;/*If true, in depth symbol interleaving is enabled.  Applies only for DVB-H. */
    bool                        timeSlicing;            /* If true, time slicing of the services is used for receiver power savings. Applies only for DVB-H. */
    bool                        mpeFec;                 /* If true, MultiProtocol Encapsulation (MPE) Forward Error Correction (FEC) is used. Applies only for DVB-H*/    
} BODS_DvbtStatus;

#ifdef __cplusplus
}
#endif

#endif /* BODS_DVBT_H__ */

