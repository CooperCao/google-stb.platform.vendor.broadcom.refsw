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
#ifndef BODS_DVBC2_H__
#define BODS_DVBC2_H__

#include "bods_defines.h"

#define BODS_L1PLP_C2_INFO_NUM_MAX 256
#define BODS_L1PLP_C2_DSLICE_NUM_MAX 8 /* This is 256 bits packed in 32 bit integers */
#define BODS_L1NOTCH_C2_INFO_NUM_MAX 16
#define BODS_C2_MAX_PLP_INFO 2
#define BODS_C2_MAX_DSLICE_INFO 1
#define BODS_C2_MAX_NOTCH_INFO 1
#define BODS_C2_MAX_L1_PART2_WORDS 1050

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
typedef enum BODS_Dvbc2Bandwidth
{
    BODS_Dvbc2Bandwidth_e8MHz,   /* 8 MHz */
    BODS_Dvbc2Bandwidth_e7MHz,   /* 7 MHz */
    BODS_Dvbc2Bandwidth_e6MHz,   /* 6 MHz */
    BODS_Dvbc2Bandwidth_e5MHz,   /* 5 MHz */
    BODS_Dvbc2Bandwidth_e1p7MHz, /* 1.7 MHz, only valid for DVB-T2 and DVB-C2 standards */
    BODS_Dvbc2Bandwidth_eLast
} BODS_Dvbc2Bandwidth;

/***************************************************************************
Summary:
    Enumeration for Guard Interval

Description:
    This enumeration represents the guard interval of the current super-frame.

See Also:

****************************************************************************/
typedef enum BODS_Dvbc2GuardInterval
{
    BODS_Dvbc2GuardInterval_e1_32,   /* Guard 1/32 */
    BODS_Dvbc2GuardInterval_e1_16,   /* Guard 1/16 */
    BODS_Dvbc2GuardInterval_e1_8,    /* Guard 1/8 */
    BODS_Dvbc2GuardInterval_e1_4,    /* Guard 1/4 */
    BODS_Dvbc2GuardInterval_e1_128,  /* Guard 1/128 */
    BODS_Dvbc2GuardInterval_e19_128, /* Guard 19/128 */
    BODS_Dvbc2GuardInterval_e19_256, /* Guard 19/256 */
    BODS_Dvbc2GuardInterval_eLast
} BODS_Dvbc2GuardInterval;

/***************************************************************************
Summary:
    Enumeration for L1 FEC type.

Description:
    This enumeration represents type of the L1 FEC used for the L1-post 
    signalling data block. Please refer to DVB-T2 ETSI EN 302 755 
    and DVB-C2 ETSI EN 302 769 standards for more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbc2FecType
{
    BODS_Dvbc2FecType_e16K, /* 16K */
    BODS_Dvbc2FecType_e64K, /* 64k */
    BODS_Dvbc2FecType_eLast
} BODS_Dvbc2FecType;

/***************************************************************************
Summary:
    Enumeration for PLP type

Description:
    This enumeration represents the type of the associated PLP. Please refer
    to DVB-C2 ETSI EN 302 769 standard for more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbc2PlpType
{
    BODS_Dvbc2PlpType_eCommon,      /* Common PLP */
    BODS_Dvbc2PlpType_eGroupedData, /* Grouped Data PLP */
    BODS_Dvbc2PlpType_eNormalData,  /* Normal Data PLP */
    BODS_Dvbc2PlpType_eLast
} BODS_Dvbc2PlpType;

/***************************************************************************
Summary:
    Enumeration for PLP Payload type

Description:
    This enumeration represents the type of the payload data carried by the 
    associated PLP. Please to DVB-C2 ETSI EN 302 769 standard for more 
    information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbc2PlpPayloadType
{
    BODS_Dvbc2PlpPayloadType_eGFPS,  /* GFPS */
    BODS_Dvbc2PlpPayloadType_eGCS,   /* GCS */
    BODS_Dvbc2PlpPayloadType_eGSE,   /* GSE */
    BODS_Dvbc2PlpPayloadType_eTS,    /* TS */
    BODS_Dvbc2PlpPayloadType_eLast
} BODS_Dvbc2PlpPayloadType;

/***************************************************************************
Summary:
    Enumeration for L1 part2 constellations.

Description:
    This enumeration represents the constellation of the L1 part2 signalling 
    data block. Please refer to DVB-C2 ETSI EN 302 769 standard for more 
    information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbc2L1Part2Modulation
{   
    BODS_Dvbc2L1Part2Modulation_eBpsk,      /* Bpsk constellation */
    BODS_Dvbc2L1Part2Modulation_eQpsk,      /* Qpsk  constellation */
    BODS_Dvbc2L1Part2Modulation_e16Qam,     /* 16-QAM constellation */
    BODS_Dvbc2L1Part2Modulation_e64Qam,     /* 64-QAM constellation */
    BODS_Dvbc2L1Part2Modulation_eLast
} BODS_Dvbc2L1Part2Modulation;

/***************************************************************************
Summary:
    Enumeration for Code Rate for L1 part2 signaling data block.

Description:
    This enumeration represents the coding of the L1 part2 signaling data 
    block. Please refer to DVB-C2 ETSI EN 302 769 standard for more 
    information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbc2L1CodeRate
{
    BODS_Dvbc2L1CodeRate_e2_3,  /* code rate 2/3 */
    BODS_Dvbc2L1CodeRate_e3_4,  /* code rate 3/4 */
    BODS_Dvbc2L1CodeRate_e4_5,  /* code rate 4/5 */
    BODS_Dvbc2L1CodeRate_e5_6,  /* code rate 5/6 */
    BODS_Dvbc2L1CodeRate_e8_9,  /* code rate 8/9 (16K LDPC code) */
    BODS_Dvbc2L1CodeRate_e9_10, /* code rate 9/10 (64K LDPC code) */
    BODS_Dvbc2L1CodeRate_eLast
} BODS_Dvbc2L1CodeRate;


/***************************************************************************
Summary:
	Enumeration for DVB-C2 time interleaving

Description:
	This enum specifies the DVB-C2 time interleaving modes. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information. 

See Also:
	None.

****************************************************************************/
typedef enum BODS_Dvbc2TimeInterleavingDepth
{
    BODS_Dvbc2TimeInterleavingDepth_eNone,  /* No time interleaving */
    BODS_Dvbc2TimeInterleavingDepth_e4Symbols,  /* 4 OFDM Symbols */
    BODS_Dvbc2TimeInterleavingDepth_e8Symbols,  /* 8 OFDM Symbols */
    BODS_Dvbc2TimeInterleavingDepth_e16Symbols, /* 16 OFDM Symbols */
    BODS_Dvbc2TimeInterleavingDepth_eLast
}BODS_Dvbc2TimeInterleavingDepth;

/***************************************************************************
Summary:
	Enumeration for DVB-C2 Data Slice Type

Description:
    This enum specifies the different types of Data Slices. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information. 

See Also:
	None.

****************************************************************************/
typedef enum BODS_Dvbc2DsliceType
{
    BODS_Dvbc2DsliceType_e1,    /* Data Slice type 1 */
    BODS_Dvbc2DsliceType_e2,    /* Data Slice type 2 */
    BODS_Dvbc2DsliceType_eLast
}BODS_Dvbc2DsliceType;

/***************************************************************************
Summary:
	Enumeration for DVB-C2 FEC Frame header type

Description:
    This enum specifies the different types of FEC Frame headers. Please 
    refer to DVB-C2 ETSI EN 302 769 standard for more information. 

See Also:
	None.

****************************************************************************/
typedef enum BODS_Dvbc2FecHeaderType
{
    BODS_Dvbc2FecHeaderType_e0,   /* Robust mode */
    BODS_Dvbc2FecHeaderType_e1,   /* High efficiency mode */
    BODS_Dvbc2FecHeaderType_eLast
}BODS_Dvbc2FecHeaderType;

/***************************************************************************
Summary:
	Structure for DVB-C2 acquisition parameters

Description:
	This structure contains parameters used for a DVB-C2 channel acquisition.
    Please refer to DVB-C2 ETSI EN 302 769 standard for more information.    

See Also:
	BODS_Acquire()

****************************************************************************/
typedef struct BODS_Dvbc2AcquireParams
{
    BODS_Dvbc2Bandwidth bandwidth;  /* bandwidth */
    bool                plpMode;    /* This bit controls whether the output  PLP is  manually or automatically selected.  When plpMode is false(manual mode), the user must supply the desired plpId.  */ 
    uint8_t             plpId;      /* In single-PLP applications, this unsigned number specifies the desired PLP for output by the T2 receiver.  In multiple-PLP applications, this unsigned number 
                                       specifies the ID for the data  (Type1 or Type2) PLP 
                                       and the common (Type0) PLP is automatically determined. */
    bool                changePlpOnly; /* When this bit is set, the demodulator only changes the PLP of the received signal. It does not do a full acquire. 
                                    There is no need for the application to retune as the frequency does not change. PLP Mode should be set to Manual alond with the new plpId for this field to take effect. 
                                    This is useful only for Multi PLP signals. When a Multi PLP signal is already locked to one of the PLPs in the signal, using changePlpOnly locks faster than a full acquire */                                        
} BODS_Dvbc2AcquireParams;

/***************************************************************************
Summary:
	Structure for FEC Statistics

Description:
	This structure contains ODS FEC Statistics. Please refer to DVB-C2 ETSI 
    EN 302 769 standard for more information.

See Also:
	BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2FecStatistics
{
    bool        lock;           /* This field indicates the lock status for the given PLP. */
    int32_t     snrData;        /* This signed number represents the Signal-to-Noise Ratio (SNR) computed from slicing the data carriers in units of 1/256th of a dB. */
    uint32_t    ldpcAvgIter;    /* This unsigned number represents the average number of iterations by the LDPC decoder. */
    uint32_t    ldpcTotIter;    /* This unsigned number represents the accumulated number of  iterations by the LDPC decoder. */   
    uint32_t    ldpcTotFrm;     /* This unsigned number represents the accumulated number of  total frames processed by the LDPC decoder. */
    uint32_t    ldpcUncFrm;     /* This unsigned number represents the accumulated number of  uncorrectable frames processed by the LDPC decoder. */
    uint32_t    ldpcBER;        /* This unsigned number represents the LDPC BER as a fraction of 2^31.  The actual LDPC BER is therefore computed by dividing this integer value by 2^31. */
    uint32_t    bchCorBit;      /* This unsigned number represents the accumulated number of  bits corrected by the BCH decoder. */
    uint32_t    bchTotBlk;      /* This unsigned number represents the accumulated number of  total blocks processed by the BCH decoder. */
    uint32_t    bchClnBlk;      /* This unsigned number represents the accumulated number of  clean blocks processed by the BCH decoder. */
    uint32_t    bchCorBlk;      /* This unsigned number represents the accumulated number of  corrected blocks processed by the BCH decoder. */  
    uint32_t    bchUncBlk;      /* This unsigned number represents the accumulated number of  uncorrectable blocks processed by the BCH decoder. */
} BODS_Dvbc2FecStatistics;

/***************************************************************************
Summary:
	Structure for L1 Part 2 PLP Parameters

Description:
	This structure contains ODS FEC Statistics. Please refer to DVB-C2 ETSI 
    EN 302 769 standard for more information.

See Also:
	BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1part2PlpParams
{
    uint16_t                    plpStart;   /* This 14-bit field indicates the start position of the first complete XFECframe of the associated PLP within the current C2 Frame. */
    BODS_Dvbc2FecType           plpFecType; /* This field indicates the FEC type used by the associated PLP. */
    BODS_Dvbc2L1Part2Modulation plpMod;     /* This field indicates the modulation used by the associated PLP. */
    BODS_Dvbc2L1CodeRate        plpCod;     /* This field indicates the code rate used by the associated PLP. */
} BODS_Dvbc2L1part2PlpParams;

/***************************************************************************
Summary:
	Structure for L1 Part 2 Reprocessing

Description:
	This structure contains L1 Part 2 Reprocessing information. Please refer
    to DVB-C2 ETSI EN 302 769 standard for more information.

See Also:
	BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1part2Reprocessing
{
    uint16_t    transportStreamId;  /* This is a 16-bit field which serves as a label for identification of this TS from any other multiplex within the delivery system */
    uint16_t    originalNetworkId;  /* This 16-bit field gives the label identifying the network_id of the originating delivery system */
} BODS_Dvbc2L1part2Reprocessing;

/***************************************************************************
Summary:
	Structure for L1 Part 2 Dslice PLP Loop

Description:
	This structure contains L1 Part 2 Dslice PLP Loop information. Please 
    refer to DVB-C2 ETSI EN 302 769 standard for more information.

See Also:
	BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1part2DslicePlp
{
    uint8_t                         dsliceId;           /* This 8-bit field uniquely identifies a Data Slice within the C2 System. */
    uint8_t                         plpId;              /* This 8-bit field identifies a PLP within the C2 System. */
    bool                            plpBundled;         /* This field indicates whether the associated PLP is bundled with other PLP(s) or not within the current C2 System.
                                                           If the associated PLP is bundled, this field shall be set to '1'. Otherwise this field shall be set to '0'. */
    BODS_Dvbc2PlpType               plpType;            /* This 2-bit field indicates the type of the associated PLP. PLP_TYPE shall be signalled according to table 23. */
    BODS_Dvbc2PlpPayloadType        plpPayloadType;     /* This 5-bit field indicates the type of the payload data carried by the associated PLP. */
    uint8_t                         plpGroupId;         /* This 8-bit field identifies with which PLP group within the C2 System the current PLP is associated. This can be used
                                                           by a receiver to link the Data PLP to its associated Common PLP, which will have the same PLP_GROUP_ID. */
    BODS_Dvbc2L1part2PlpParams      plpParams;          /* L1 Part 2 PLP Parameters */
    bool                            psiSiReprocessing;  /* If this is set to true, then PSI/SI reprocessing is performed.*/
    BODS_Dvbc2L1part2Reprocessing   reprocessing;       /* L1 Part 2 Reprocessing, only valid if psiSiReprocessing = true. */
} BODS_Dvbc2L1part2DslicePlp;

/***************************************************************************
Summary:
	Structure for L1 Part 2 Dslice Loop

Description:
	This structure contains L1 Part 2 Dslice loop information. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information.

See Also:
	BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1part2Dslice
{
    uint8_t   dsliceId;                                                                 /* This 8-bit field uniquely identifies a Data Slice within the C2 System. */
    uint16_t  dsliceTunePos;                                                            /* This field indicates the tuning position of the associated Data Slice relative to the startFrequency. Its bit
                                                                                           width shall be 13 bits or 14 bits according to the guardInterval value. */
    int16_t  dsliceOffsetLeft;                                                          /* This field indicates the start position of the associated Data Slice by means of the distance to the left from 
                                                                                           the tuning position and shall be two's complement integer of 8 bits or 9 bits according to the guardInterval value.*/
    int16_t  dsliceOffsetRight;                                                         /* This field indicates the end position of the associated Data Slice by means of the distance to the right from the 
                                                                                           tuning position and shall be two's complement integer of 8 bits or 9 bits according to the guardInterval value. */
    BODS_Dvbc2TimeInterleavingDepth timeInterLeavingDepth;                              /* This 2-bit field indicates the time interleaving depth within the associated Data Slice. */
    BODS_Dvbc2DsliceType            dsliceType;                                         /* This 1-bit field indicates the type of the associated Data Slice. */
    BODS_Dvbc2FecHeaderType         fecHeaderType;                                      /* This 1-bit field indicates the type of the FEC Frame header within the associated Data Slice. */
    bool                            dsliceConstConf;                                    /* This 1-bit field indicates whether the configuration of the associated Data Slice is variable or fixed. */
    bool                            dsliceLeftNotch;                                    /* This 1-bit field indicates the presence of the left neighboured Notch band of the associated Data Slice.*/
    uint8_t                         dsliceNumPlp;                                       /* This 8-bit field indicates the number of PLPs carried within the associated Data Slice. */
    uint32_t                        plpsPresent[BODS_L1PLP_C2_DSLICE_NUM_MAX];          /* Plps present in the Dslice */
} BODS_Dvbc2L1part2Dslice;

/***************************************************************************
Summary:
	Structure for L1 Part 2 Notch Loop

Description:
	This structure contains L1 Part 2 Notch loop information. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information.

See Also:
	BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1part2Notch
{
    uint16_t  notchStart;   /* This field indicates the start position of the associated Notch band. */
    uint16_t  notchWidth;   /* This field indicates the width of the associated Notch band. */
} BODS_Dvbc2L1part2Notch;

/***************************************************************************
Summary:
	Structure for L1 Part 2 Status

Description:
	This structure contains L1 Part 2 Status. Please refer to DVB-C2 ETSI 
    EN 302 769 standard for more information.

See Also:
	BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1Part2Status
{
    uint16_t                    networkId;                              /* This is a 16-bit field which uniquely identifies the current DVB-C2 network. */
    uint16_t                    c2SystemId;                             /* This 16-bit field uniquely identifies a C2 System within the DVB-C2 network. */
    uint32_t                    startFrequency;                         /* This 24-bit field indicates the start frequency in Hz of the current C2 System by means of the distance from 0 Hz. */
    uint16_t                    c2Bandwidth;                            /* This 16-bit field indicates the bandwidth of the current C2 system. */
    BODS_Dvbc2GuardInterval     guardInterval;                          /* This field indicates the guard interval of the current C2 Frame. */
    uint16_t                    c2FrameLength;                          /* This 10-bit field gives the number of Data Symbols per C2 Frame */
    uint8_t                     l1Part2ChangeCounter;                   /* This 8-bit field indicates the number of C2 Frames ahead where the configuration will change. */
    uint8_t                     numDslice;                              /* This 8-bit field indicates the number of Data Slices carried within the current C2 Frame. */
    uint8_t                     numNotch;                               /* This 4-bit field indicates the number of Notch bands. If there is no Notch band within the current C2 Frame, this field shall be set to '0'. */
    BODS_Dvbc2L1part2Dslice     dslices[BODS_C2_MAX_DSLICE_INFO];       /* Dslice Loop */
    BODS_Dvbc2L1part2DslicePlp  plps[BODS_C2_MAX_PLP_INFO];             /* PLP Loop */
    BODS_Dvbc2L1part2Notch      notches[BODS_L1NOTCH_C2_INFO_NUM_MAX];  /* Notch Loop */
    bool                        reservedTone;                           /* This 1-bit field indicates whether some carriers are reserved. */
} BODS_Dvbc2L1Part2Status;

/***************************************************************************
Summary:
	Structure for L1 Dslice Loop

Description:
	This structure contains L1 Dslice loop information. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information.

See Also:
	BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1Dslice
{
    uint8_t                     dsliceId;                                           /* This 8-bit field uniquely identifies a Data Slice within the C2 System. */
    uint8_t                     dsliceNumPlp;                                       /* This 8-bit field indicates the number of PLPs carried within the associated Data Slice. */
    uint32_t                    plpsPresent[BODS_L1PLP_C2_DSLICE_NUM_MAX];          /* Plps present in the Dslice */
} BODS_Dvbc2L1Dslice;

/***************************************************************************
Summary:
    This structure represents the ODS L1 Dslice status.

Description:
    This structure is returned when BODS_GetSelevtiveAsyncStatus()  is called.
    This structure contains the complete L1 Dslice status of an ODS 
    channel. Please refer to DVB-C2 ETSI EN 302 769 standard for more 
    information.

See Also:
    BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1DsliceStatus
{
    uint8_t             numDslice;                          /* number of Data Slices carried within the within the current C2 Frame. */
    BODS_Dvbc2L1Dslice  dslices[BODS_C2_MAX_DSLICE_INFO];   /* Dslice Table. */
} BODS_Dvbc2L1DsliceStatus;

/***************************************************************************
Summary:
    This structure represents the ODS L1 Notch status.

Description:
    This structure is returned when BODS_GetSelevtiveAsyncStatus()  is called.
    This structure contains the complete L1 Notch status of an ODS 
    channel. Please refer to DVB-C2 ETSI EN 302 769 standard for more 
    information.

See Also:
    BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1NotchStatus
{
    uint8_t                 numNotch;                               /* number of PLPs found in the within the current C2 Frame. */
    BODS_Dvbc2L1part2Notch  notches[BODS_L1NOTCH_C2_INFO_NUM_MAX];  /* Notch Table */
} BODS_Dvbc2L1NotchStatus;

/***************************************************************************
Summary:
    This structure represents the ODS PLP information.

Description:
    This structure contains the PLP information. Please refer to DVB-C2 ETSI 
    EN 302 769 standard for more information.

See Also:

****************************************************************************/
typedef struct BODS_Dvbc2Plp
{
    uint8_t                     plpId;          /* This 8-bit field identifies uniquely a PLP within the T2 system */
    uint8_t                     plpGroupId;     /* This 8-bit field identifies with which PLP group within the T2 system the current PLP is associated */
    BODS_Dvbc2PlpPayloadType    plpPayloadType; /* This 5-bit field indicates the type of the payload data carried by the associated PLP */
    BODS_Dvbc2PlpType           plpType;        /* This 3-bit field indicates the type of the associated PLP */
} BODS_Dvbc2Plp;
/***************************************************************************
Summary:
    This structure represents the ODS L1 PLP status.

Description:
    This structure is returned when BODS_GetSelevtiveAsyncStatus()  is called.
    This structure contains the complete L1 PLP status of an ODS 
    channel. Please refer to DVB-C2 ETSI EN 302 769 standard for more 
    information.

See Also:
    BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbc2L1PlpStatus
{
    uint8_t         numPlp;             /* number of PLPs found in the within the current C2 Frame. */
    BODS_Dvbc2Plp   plp[BODS_MAX_PLP];  /* PLP Table */
} BODS_Dvbc2L1PlpStatus;

/***************************************************************************
Summary:
    This structure represents ODS short Status

Description:
    This structure is returned when BODS_GetSelectiveAsyncStatus() is called.

See Also:
    BODS_GetSelevtiveAsyncStatus()
****************************************************************************/
typedef struct BODS_Dvbc2ShortStatus
{
    BODS_Standard               acquireStandard;        /* Standard */
    BODS_AcquisitionMode        acquisitionMode;        /* Acquisition Mode */
    BODS_Dvbc2Bandwidth         bandwidth;              /* bandwidth */
    bool                        lock;                   /* FEC lock */
    bool                        spectrumInverted;       /* If true, Spectrum is inverted */   
    uint32_t                    reacqCount;             /* Number of reacquisitions performed */                 
    int32_t                     snrEstimate;            /* This unsigned number represents the Signal-to-Noise Ratio (SNR) in units of 1/256th of a dB.  The SNR in dB is therefore achieved by dividing the unsigned integer value by 256 */
    int32_t                     gainOffset;             /* internal AGC gain offset in units of 1/256th of a dB */                          
    int32_t                     carrierFreqOffset;      /* carrier frequency offset in units of Hz */                        
    int32_t                     timingOffset;           /* timing frequency offset in units of Hz */           
    int16_t                     signalStrength;         /* represents the entire AGC (LNA+ODS+UFE+T2)gain in units of a dBm */
    uint8_t                     signalLevelPercent;     /* Signal Level Percent represents the Signal Strength of the received signal in percent */
    uint8_t                     signalQualityPercent;   /* Signal Quality Percent represents the Signal Quality of the received signal in percent */    
} BODS_Dvbc2ShortStatus;

#ifdef __cplusplus
}
#endif

#endif /* BODS_DVBC2_H__ */
