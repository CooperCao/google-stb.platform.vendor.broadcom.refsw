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
#ifndef BODS_DVBT2_H__
#define BODS_DVBT2_H__ 

#include "bods_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BODS_MAX_PLP 255

/***************************************************************************
Summary:
    Enumeration for channel bandwidth

Description:
    This enumeration defines the valid channel bandwidth settings.

See Also:
    BODS_Acquire(), BODS_GetSelevtiveAsyncStatus()
****************************************************************************/
typedef enum BODS_Dvbt2Bandwidth
{
    BODS_Dvbt2Bandwidth_e8MHz,   /* 8 MHz */
    BODS_Dvbt2Bandwidth_e7MHz,   /* 7 MHz */
    BODS_Dvbt2Bandwidth_e6MHz,   /* 6 MHz */
    BODS_Dvbt2Bandwidth_e5MHz,   /* 5 MHz */
    BODS_Dvbt2Bandwidth_e1p7MHz, /* 1.7 MHz, only valid for DVB-T2 and DVB-C2 standards */
    BODS_Dvbt2Bandwidth_eLast
} BODS_Dvbt2Bandwidth;

/***************************************************************************
Summary:
    Enumeration for L1 FEC type.

Description:
    This enumeration represents type of the L1 FEC used for the L1-post 
    signalling data block. Please refer to DVB-T2 ETSI EN 302 755 
    and DVB-C2 ETSI EN 302 769 standards for more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2FecType
{
    BODS_Dvbt2FecType_e16K, /* 16K */
    BODS_Dvbt2FecType_e64K, /* 64k */
    BODS_Dvbt2FecType_eLast
} BODS_Dvbt2FecType;

/***************************************************************************
Summary:
    Enumeration for Guard Interval

Description:
    This enumeration represents the guard interval of the current super-frame.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2GuardInterval
{
    BODS_Dvbt2GuardInterval_e1_32,   /* Guard 1/32 */
    BODS_Dvbt2GuardInterval_e1_16,   /* Guard 1/16 */
    BODS_Dvbt2GuardInterval_e1_8,    /* Guard 1/8 */
    BODS_Dvbt2GuardInterval_e1_4,    /* Guard 1/4 */
    BODS_Dvbt2GuardInterval_e1_128,  /* Guard 1/128 */
    BODS_Dvbt2GuardInterval_e19_128, /* Guard 19/128 */
    BODS_Dvbt2GuardInterval_e19_256, /* Guard 19/256 */
    BODS_Dvbt2GuardInterval_eLast
} BODS_Dvbt2GuardInterval;

/***************************************************************************
Summary:
    Enumeration for DVB-T2 stream types

Description:
    This enumeration indicates the types of the Tx input streams carried 
    within the current DVB-T2 super-frame. Please refer to DVB-T2 ETSI EN 302
    755 standard for more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2StreamType
{
    BODS_Dvbt2StreamType_eTS,       /* Transport Stream */
    BODS_Dvbt2StreamType_eGS,       /* Generic Stream (GSE [i.2] and/or GFPS and/or GCS) but not TS */
    BODS_Dvbt2StreamType_eMixed,    /* Both TS and Generic Stream (i.e. TS and at least one of GSE, GFPS, GCS) */
    BODS_Dvbt2StreamType_eLast
} BODS_Dvbt2StreamType;

/***************************************************************************
Summary:
    Enumeration for DVB-T2 PAPR reduction.

Description:
    This enumeration represents the DVB-T2 PAPR reduction used. Please refer 
    to DVB-T2 ETSI EN 302 755 standard for more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2Papr
{
    BODS_Dvbt2Papr_eNone,   /* L1-ACE is used and TR is used on P2 symbols only */
    BODS_Dvbt2Papr_eACE,    /* L1-ACE and ACE only are used */
    BODS_Dvbt2Papr_eTR,     /* L1-ACE and TR only are used */
    BODS_Dvbt2Papr_eBoth,   /* L1-ACE, ACE and TR are used */
    BODS_Dvbt2Papr_eLast
} BODS_Dvbt2Papr;

/***************************************************************************
Summary:
    Enumeration for DVB-T2 Pilot Pattern. 

Description:
    This enumeration represents the scattered pilot pattern used for DVB-T2
    data OFDM symbols. Please refer to DVB-T2 ETSI EN 302 755 standard for
    more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2PilotPattern
{
    BODS_Dvbt2PilotPattern_e1, /* Pilot Pattern 1 */
    BODS_Dvbt2PilotPattern_e2, /* Pilot Pattern 2 */
    BODS_Dvbt2PilotPattern_e3, /* Pilot Pattern 3 */
    BODS_Dvbt2PilotPattern_e4, /* Pilot Pattern 4 */
    BODS_Dvbt2PilotPattern_e5, /* Pilot Pattern 5 */
    BODS_Dvbt2PilotPattern_e6, /* Pilot Pattern 6 */
    BODS_Dvbt2PilotPattern_e7, /* Pilot Pattern 7 */
    BODS_Dvbt2PilotPattern_e8, /* Pilot Pattern 8 */
    BODS_Dvbt2PilotPattern_eLast
} BODS_Dvbt2PilotPattern;

/***************************************************************************
Summary:
    Enumeration for DVB-T2 PLP modulation.

Description:
    This enumeration represents the the constellation of the L1-post
    signalling data block. Please refer to DVB-T2 ETSI EN 302 755 standard 
    for more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2PlpModulation
{
    BODS_Dvbt2PlpModulation_eQpsk,      /* QPSK */
    BODS_Dvbt2PlpModulation_e16Qam,     /* 16-QAM */
    BODS_Dvbt2PlpModulation_e64Qam,     /* 64-QAM */
    BODS_Dvbt2PlpModulation_e256Qam,    /* 256-QAM */
    BODS_Dvbt2PlpModulation_eLast
} BODS_Dvbt2PlpModulation;

/***************************************************************************
Summary:
    Enumeration for DVB-T2 L1 Modulation.

Description:
    This enumeration represents the Modulation of DVB-T2 L1-post signalling 
    data block. Please refer to DVB-T2 ETSI EN 302 755 standard for more 
    information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2L1Modulation
{   
    BODS_Dvbt2L1Modulation_eBpsk,   /* QPSK Modulation */
    BODS_Dvbt2L1Modulation_eQpsk,   /* QPSK Modulation */
    BODS_Dvbt2L1Modulation_e16Qam,  /* 16-QAM Modulation */
    BODS_Dvbt2L1Modulation_e64Qam,  /* 64-QAM Modulation */
    BODS_Dvbt2L1Modulation_eLast
} BODS_Dvbt2L1Modulation;

/***************************************************************************
Summary:
    Enumeration for Code Rate for DVB-T2 L1-post signaling data block.

Description:
    This enumeration represents the coding of DVB-T2 L1-post signaling data 
    block. Please refer to DVB-T2 ETSI EN 302 755 standard for more 
    information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2L1CodeRate
{
    BODS_Dvbt2L1CodeRate_e1_2, /* code rate 1/2 */
    BODS_Dvbt2L1CodeRate_eLast
} BODS_Dvbt2L1CodeRate;

/***************************************************************************
Summary:
    Enumeration for DVB-T2 PLP Code Rate.

Description:
    This enumeration represents the code rate used by the associated PLP. 
    Please refer to DVB-T2 ETSI EN 302 755 standard for more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2PlpCodeRate
{
    BODS_Dvbt2PlpCodeRate_e1_2, /* Code Rate 1/2 */
    BODS_Dvbt2PlpCodeRate_e3_5, /* Code Rate 3/5 */
    BODS_Dvbt2PlpCodeRate_e2_3, /* Code Rate 2/3 */
    BODS_Dvbt2PlpCodeRate_e3_4, /* Code Rate 3/4 */
    BODS_Dvbt2PlpCodeRate_e4_5, /* Code Rate 4/5 */
    BODS_Dvbt2PlpCodeRate_e5_6, /* Code Rate 5/6 */
    BODS_Dvbt2PlpCodeRate_eLast
} BODS_Dvbt2PlpCodeRate;

/***************************************************************************
Summary:
    Enumeration for DVB-T2 PLP type

Description:
    This enumeration represents the type of the associated PLP. Please refer
    to DVB-T2 ETSI EN 302 755 standard for more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2PlpType
{
    BODS_Dvbt2PlpType_eCommon,       /* Common PLP */
    BODS_Dvbt2PlpType_eDataType1,    /* DATA PLP Type 1 */
    BODS_Dvbt2PlpType_eDataType2,    /* DATA PLP Type 2 */
    BODS_Dvbt2PlpType_eLast
} BODS_Dvbt2PlpType;

/***************************************************************************
Summary:
    Enumeration for DVB-T2 PLP Payload type

Description:
    This enumeration represents the type of the payload data carried by the 
    associated PLP. Please refer to DVB-T2 ETSI EN 302 755 standard for more
    information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2PlpPayloadType
{
    BODS_Dvbt2PlpPayloadType_eGFPS,  /* GFPS */
    BODS_Dvbt2PlpPayloadType_eGCS,   /* GCS */
    BODS_Dvbt2PlpPayloadType_eGSE,   /* GSE */
    BODS_Dvbt2PlpPayloadType_eTS,    /* TS */
    BODS_Dvbt2PlpPayloadType_eLast
} BODS_Dvbt2PlpPayloadType;

/***************************************************************************
Summary:
    Enumeration for DVB-T2 Profile Type

Description:
    This enumeration represents the type of the DVB-T2 profile present.
    Please refer to DVB-T2 ETSI EN 302 755 standard for more information.

See Also:

****************************************************************************/
typedef enum BODS_Dvbt2Profile
{
    BODS_Dvbt2Profile_eBase,    /* Base Profile */
    BODS_Dvbt2Profile_eLite,    /* DVB-T2 Lite Profile */
    BODS_Dvbt2Profile_eLast
} BODS_Dvbt2Profile;

/***************************************************************************
Summary:
	Structure for DVB-T2 acquisition parameters.

Description:
	This structure contains parameters used for an DVB-T2 channel acquisition.
    Please refer to DVB-T2 ETSI EN 302 755 standard for more information.

See Also:
	BODS_Acquire()

****************************************************************************/
typedef struct BODS_Dvbt2AcquireParams
{
    BODS_Dvbt2Bandwidth bandwidth;  /* bandwidth */
    bool                plpMode;    /* This bit controls whether the output  PLP is  manually or automatically selected.  When plpMode is false(manual mode), the user must supply the desired plpId.  */ 
    uint8_t             plpId;      /* In single-PLP applications, this unsigned number specifies the desired PLP for output by the T2 receiver. 
                                       In multiple-PLP applications, this unsigned number specifies the ID for the data  (Type1 or Type2) PLP 
                                       and the common (Type0) PLP is automatically determined. */
    BODS_Dvbt2Profile   profile;    /* Type of DVB-T2 profile to acquire, base or lite */
    bool                changePlpOnly; /* When this bit is set, the demodulator only changes the PLP of the received signal. It does not do a full acquire. 
                                    There is no need for the application to retune as the frequency does not change. PLP Mode should be set to Manual alond with the new plpId for this field to take effect. 
                                    This is useful only for Multi PLP signals. When a Multi PLP signal is already locked to one of the PLPs in the signal, using changePlpOnly locks faster than a full acquire */ 
} BODS_Dvbt2AcquireParams;

/***************************************************************************
Summary:
	Structure for DVB-T2 FEC Statistics.

Description:
	This structure contains ODS FEC Statistics. Please refer to DVB-T2 ETSI 
    EN 302 755 standard for more information.

See Also:
	BODS_GetSelectiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbt2FecStatistics
{
    bool        lock;           /* This field indicates the lock status for the given PLP*/
    int32_t     snrData;        /* This signed number represents the Signal-to-Noise Ratio (SNR) computed from slicing the data carriers in units of 1/256th of a dB. */
    uint32_t    ldpcAvgIter;    /* This unsigned number represents the average number of iterations by the LDPC decoder */
    uint32_t    ldpcTotIter;    /* This unsigned number represents the accumulated number of  iterations by the LDPC decoder */   
    uint32_t    ldpcTotFrm;     /* This unsigned number represents the accumulated number of  total frames processed by the LDPC decoder. */
    uint32_t    ldpcUncFrm;     /* This unsigned number represents the accumulated number of  uncorrectable frames processed by the LDPC decoder */
    uint32_t    ldpcBER;        /* This unsigned number represents the LDPC BER as a fraction of 2^31.  The actual LDPC BER is therefore computed by dividing this integer value by 2^31 */
    uint32_t    bchCorBit;      /* This unsigned number represents the accumulated number of bits corrected by the BCH decoder */
    uint32_t    bchTotBlk;      /* This unsigned number represents the accumulated number of total blocks processed by the BCH decoder */
    uint32_t    bchClnBlk;      /* This unsigned number represents the accumulated number of clean blocks processed by the BCH decoder */
    uint32_t    bchCorBlk;      /* This unsigned number represents the accumulated number of corrected blocks processed by the BCH decoder */  
    uint32_t    bchUncBlk;      /* This unsigned number represents the accumulated number of uncorrectable blocks processed by the BCH decoder */
} BODS_Dvbt2FecStatistics;
  
/***************************************************************************
Summary:
    This structure represents DVB-T2 L1 Pre Status.

Description:
    This structure is returned when BODS_GetSelectiveAsyncStatus() is called.
    This structure contains the complete L1 Pre status of an ODS channel.
    Please refer to DVB-T2 ETSI EN 302 755 standard for more information.

See Also:
    BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbt2L1PreStatus
{
    BODS_Dvbt2StreamType    streamType;         /* streamType */   
    bool                    bwtExt;             /* This field indicates whether the extended carrier mode is used in the case of 8K, 16K and 32K FFT 
                                                   sizes. When this field is set to '1', the extended carrier mode is used. If this field is set to '0', 
                                                   the normal carrier mode is used */
    uint8_t                 s1;                 /* The preamble format is carried in the S1 field of the P1 symbol. It identifies the format of the P2
                                                   symbol(s) that take part of the preamble*/   
    uint8_t                 s2;                 /* The first 3 bits of the S2 field are referred to as S2 field 1. When the preamble format is of the type "T2"
                                                   (either "T2_MISO" or "T2_SISO"), S2 field 1 indicates the FFT size and gives partial information about
                                                   the guard interval for the remaining symbols in the T2-frameThe last bit indicates whether the
                                                   preambles are all of the same type or not */   
    bool                    l1RepetitionFlag;   /* This flag indicates whether the dynamic L1-Pre signalling is provided also for the next frame. */
    BODS_Dvbt2GuardInterval guardInterval;      /* This field indicates the guard interval of the current super-frame */
    BODS_Dvbt2Papr          papr;               /* This field describes what kind of PAPR reduction is used */
    BODS_Dvbt2L1Modulation  l1Modulation;       /* This field describes the Modulation of the L1-Pre signalling data block */
    BODS_Dvbt2L1CodeRate    l1CodeRate;         /* This field describes the code rate of the L1-Pre signalling data block */
    BODS_Dvbt2FecType       l1FecType;          /* This field indicates the type of the L1 FEC used for the L1-Pre signalling data block */
    BODS_Dvbt2PilotPattern  pilotPattern;       /* This 4-bit field indicates the scattered pilot pattern used for the data OFDM symbols. */
    uint8_t                 regenFlag;          /* This 3-bit field indicates how many times the DVB-T2 signal has been re-generated */
    bool                    l1PostExt;          /* Indicates the presence of the L1-Pre extension field */
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
} BODS_Dvbt2L1PreStatus;

/***************************************************************************
Summary:
    This structure represents L1 Post Configurable PLP structure.

Description:
    This structure is returned when BODS_GetSelevtiveAsyncStatus() is called.
    This structure contains the complete L1 Post Configurable PLP parameters 
    of an ODS channel. Please refer to DVB-T2 ETSI EN 302 755 standard for
    more information.

See Also:
    BODS_GetSelectiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbt2L1PostCfgPlp
{
    uint8_t                     plpId;              /* This 8-bit field identifies uniquely a PLP within the T2 system. */
    uint8_t                     plpGroupId;         /* This 8-bit field identifies with which PLP group within the T2 system the current PLP is associated. */
    BODS_Dvbt2PlpType           plpType;            /* This 3-bit field indicates the type of the associated PLP */  
    BODS_Dvbt2PlpPayloadType    plpPayloadType;     /* This 5-bit field indicates the type of the payload data carried by the associated PLP */
    bool                        ffFlag;             /* This flag is set to '1' if a PLP of type 1 in a TFS system occurs on the same RF channel in each
                                                       T2-frame. This flag is set to '0' if inter-frame TFS is applied as described in annex E */
    uint8_t                     firstRfIdx;         /* This 3-bit field indicates on which RF channel a type 1 data PLP occurs in the first frame of
                                                       a super-frame in a TFS system. */
    uint8_t                     firstFrameIdx;      /* This 8-bit field indicates the IDX of the first frame of the super-frame in which the current PLP occurs. */
    BODS_Dvbt2PlpCodeRate       codeRate;        /* This field represents the code rate used by the associated PLP. */
    BODS_Dvbt2PlpModulation     modulation;             /* This 3-bit field indicates the modulation used by the associated PLP */
    bool                        plpRotation;        /* This flag indicates whether constellation rotation is in use or not by the associatedPLP */
    BODS_Dvbt2FecType           plpFecType;         /* This 2-bit field indicates the FEC type used by the associated PLP */
    uint16_t                    plpNumBlocksMax;    /* This 10-bit field indicates the maximum value of plpNumBlocksMax */
    uint8_t                     frameInterval;      /* This 8-bit field indicates the T2-frame interval within the super-frame for the associated PLP */
    uint8_t                     timeIlLength;       /* The use of this 8-bit field is determined by the values set within the timeIlType. If timeIlType is set to true,
                                                       this field shall indicate PI, the number of T2-frames to which each Interleaving Frame is mapped. If timeIlType
                                                       is set to false, this field shall indicate NTI, the number of TI-blocks per Interleaving Frame */
    bool                        timeIlType;         /* This 1-bit field indicates the type of time-interleaving */
    bool                        inBandFlag;         /* This 1-bit field indicates whether the current PLP carries in-band type A signalling information */
} BODS_Dvbt2L1PostCfgPlp;

/***************************************************************************
Summary:
    This structure represents L1 Post Configurable status.

Description:
    This structure is returned when BODS_GetSelevtiveAsyncStatus() is called. 
    This structure contains the complete L1 Post Configurable status of an
    ODS channel. Please refer to DVB-T2 ETSI EN 302 755 standard for more 
    information.

See Also:
    BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbt2L1PostConfigurableStatus
{
    uint16_t                subSlicesPerFrame;  /* This 15-bit field indicates Nsubslices_total, the total number of sub-slices for the type 2
                                                   data PLPs across all RF channels in one T2-frame */
    uint8_t                 numPlp;             /* This 8-bit field indicates the number of PLPs carried within the current super-frame */ 
    uint8_t                 numAux;             /* This 4-bit field indicates the number of auxiliary streams */
    uint8_t                 auxConfigRFu;       /* This 8-bit field is reserved for future use. */
    uint8_t                 fefType;            /* This 4-bit field shall indicate the type of the associated FEF part */
    uint8_t                 rfIdx;              /* This 3-bit field indicates the index of each FREQUENCY listed within this loop */
    uint8_t                 fefInterval;        /* This 8-bit field indicates the number of T2-frames between two FEF parts */
    uint32_t                frequency;          /* This 32-bit field indicates the centre frequency in Hz of the RF channel whose index is rfIdx */
    uint32_t                fefLength;          /* This 22-bit field indicates the length of the associated FEF part as the number of elementary
                                                   periods T, from the start of the P1 symbol of the FEF part to the start of the P1 symbol of the
                                                   ext T2-frame */
    BODS_Dvbt2L1PostCfgPlp  plpA;               /* PLP loop A */
    BODS_Dvbt2L1PostCfgPlp  plpB;               /* PLP loop B */
    uint8_t                 auxStreamType;      /* This 4-bit field indicates the type of the current auxiliary stream */
    uint32_t                auxPrivateConf;     /* This 28-bit field is for future use for signalling auxiliary streams */
} BODS_Dvbt2L1PostConfigurableStatus;

/***************************************************************************
Summary:
    This structure represents the ODS L1 Post Dynamic PLP structure.

Description:
    This structure is returned when BODS_GetSelevtiveAsyncStatus() is called.
    This structure contains the complete L1 Post Dynamic PLP parameters of 
    an ODS channel. Please refer to DVB-T2 ETSI EN 302 755 standard for more
    information.

See Also:
    BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbt2L1PostDynamicPlp
{
    uint8_t                 plpId;          /* plpId */
    uint16_t                plpNumBlocks;   /* This 10-bit field indicates the number of FEC blocks contained in the current
                                               Interleaving Frame for the current PLP */
    uint32_t                plpStart;       /* This 22-bit field indicates the start position of the associated PLP within the current T2-frame
                                               using the cell addressing scheme */
} BODS_Dvbt2L1PostDynamicPlp;

/***************************************************************************
Summary:
    This structure represents the ODS L1 Post Dynamic status.

Description:
    This structure is returned when BODS_GetSelevtiveAsyncStatus() is called.
    This structure contains the complete L1 Post Dynamic status of an ODS
    channel. Please refer to DVB-T2 ETSI EN 302 755 standard for more
    information.

See Also:
    BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbt2L1PostDynamicStatus
{
    uint8_t                     frameIdx;               /* This 8-bit field is the index of the current T2-frame within a super-frame */
    uint8_t                     l1ChanlgeCounter;       /* This 8-bit field indicates the number of super-frames ahead where the configuration will change */
    uint8_t                     startRfIdx;             /* This 3-bit field indicates the ID of the starting frequency of the TFS scheduled frame, for the next T2-frame */
    uint32_t                    subSliceInterval;       /* This 22-bit field indicates the number of OFDM cells from the start of one sub-slice of one
                                                           PLP to the start of the next sub-slice of the same PLP on the same RF channel for the current T2-frame */
    uint32_t                    type2Start;             /* This 22-bit field indicates the start position of the first of the type 2 PLPs using the cell addressing scheme */
    BODS_Dvbt2L1PostDynamicPlp  plpA;                   /* plp loop A */
    BODS_Dvbt2L1PostDynamicPlp  plpB;                   /* plp loop B */
    uint32_t                    auxPrivateDyn_47_32;    /* This 48-bit field is reserved for future use for signalling auxiliary streams. */
    uint32_t                    auxPrivateDyn_31_0;     /* This 48-bit field is reserved for future use for signalling auxiliary streams. */
} BODS_Dvbt2L1PostDynamicStatus;

/***************************************************************************
Summary:
    This structure represents PLP loop.

Description:
    This structure is returned when BODS_GetSelectiveAsyncStatus()  is called.  
    This structure contains the PLP information. Please refer to DVB-T2 ETSI 
    EN 302 755 standard for more information.

See Also:

****************************************************************************/
typedef struct BODS_Dvbt2Plp
{
    uint8_t                     plpId;          /* This 8-bit field identifies uniquely a PLP within the T2 system */
    uint8_t                     plpGroupId;     /* This 8-bit field identifies with which PLP group within the T2 system the current PLP is associated */
    BODS_Dvbt2PlpPayloadType    plpPayloadType; /* This 5-bit field indicates the type of the payload data carried by the associated PLP */
    BODS_Dvbt2PlpType           plpType;        /* This 3-bit field indicates the type of the associated PLP */
} BODS_Dvbt2Plp;

/***************************************************************************
Summary:
    This structure represents the ODS L1 PLP status.

Description:
    This structure is returned when BODS_GetSelevtiveAsyncStatus()  is called.
    This structure contains the complete L1 PLP of an ODS channel. Please 
    refer to DVB-T2 ETSI EN 302 755 standard for more information.

See Also:
    BODS_GetSelevtiveAsyncStatus()

****************************************************************************/
typedef struct BODS_Dvbt2L1PlpStatus
{
    uint8_t         numPlp;             /* This 8-bit field indicates the number of PLPs carried within the current super-frame. The minimum value of this field shall be '1' */
    BODS_Dvbt2Plp   plp[BODS_MAX_PLP];  /* PLP information*/
} BODS_Dvbt2L1PlpStatus;

/***************************************************************************
Summary:
    This structure represents ODS short Status.

Description:
    This structure is returned when BODS_GetSelectiveAsyncStatus() is called.

See Also:
    BODS_GetSelevtiveAsyncStatus()
****************************************************************************/
typedef struct BODS_Dvbt2ShortStatus
{
    BODS_Standard               acquireStandard;        /* Standard */
    BODS_AcquisitionMode        acquisitionMode;        /* Acquisition Mode */
    BODS_Dvbt2Bandwidth         bandwidth;              /* bandwidth */
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
    BODS_Dvbt2Profile           profile;                /* represents the DVB-T2 profile set in the acquire parameters */
} BODS_Dvbt2ShortStatus;

#ifdef __cplusplus
}
#endif

#endif /* BODS_DVB-T2_H__ */

