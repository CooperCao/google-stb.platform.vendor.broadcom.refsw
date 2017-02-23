/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
***************************************************************************/
#ifndef NEXUS_FRONTEND_DVBC2_H__
#define NEXUS_FRONTEND_DVBC2_H__
    
#include "nexus_frontend_ofdm_types.h"
#include "nexus_frontend_ofdm.h"

#ifdef __cplusplus
    extern "C" {
#endif


#define NEXUS_MAX_DVBC2_PLP_STATUS 255
#define NEXUS_MAX_DVBC2_L1NOTCH_C2_INFO 16
#define NEXUS_MAX_DVBC2_DSLICE_INFO 1
#define NEXUS_MAX_DVBC2_L1PLP_C2_INFO 256
#define NEXUS_MAX_DVBC2_PLP_INFO 2
#define NEXUS_MAX_DVBC2_L1PLP_DSLICE 8 /* This is 256 bits packed in 32 bit integers */

/***************************************************************************
Summary:
    This enum specifies the different types of Data Slices. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information. 
****************************************************************************/
typedef enum NEXUS_FrontendDvbc2DsliceType
{
    NEXUS_FrontendDvbc2DsliceType_e1,    /* Data Slice type 1 */
    NEXUS_FrontendDvbc2DsliceType_e2,    /* Data Slice type 2 */
    NEXUS_FrontendDvbc2DsliceType_eMax
}NEXUS_FrontendDvbc2DsliceType;

/***************************************************************************
Summary:
    This enum specifies the different types of FEC Frame headers. Please 
    refer to DVB-C2 ETSI EN 302 769 standard for more information. 
****************************************************************************/
typedef enum NEXUS_FrontendDvbc2FecHeaderType
{
    NEXUS_FrontendDvbc2FecHeaderType_e0,   /* Robust mode */
    NEXUS_FrontendDvbc2FecHeaderType_e1,   /* High efficiency mode */
    NEXUS_FrontendDvbc2FecHeaderType_eMax
}NEXUS_FrontendDvbc2FecHeaderType;


/***************************************************************************
Summary:
    This enumeration represents type of the L1 FEC used for the L1-post 
    signalling data block. Please refer to DVB-T2 ETSI EN 302 755 
    and DVB-C2 ETSI EN 302 769 standards for more information.
****************************************************************************/
typedef enum NEXUS_FrontendDvbc2FecType
{
    NEXUS_FrontendDvbc2FecType_e16K, /* 16K */
    NEXUS_FrontendDvbc2FecType_e64K, /* 64K */
    NEXUS_FrontendDvbc2FecType_eMax
} NEXUS_FrontendDvbc2FecType;

/***************************************************************************
Summary:
    This enumeration represents the constellation of the L1 part2 signalling 
    data block. Please refer to DVB-C2 ETSI EN 302 769 standard for more 
    information.
****************************************************************************/
typedef enum NEXUS_FrontendDvbc2L1Part2Modulation
{   
    NEXUS_FrontendDvbc2L1Part2Modulation_eBpsk,      /* Bpsk constellation */
    NEXUS_FrontendDvbc2L1Part2Modulation_eQpsk,      /* Qpsk  constellation */
    NEXUS_FrontendDvbc2L1Part2Modulation_eQam16,     /* 16-QAM constellation */
    NEXUS_FrontendDvbc2L1Part2Modulation_eQam64,     /* 64-QAM constellation */
    NEXUS_FrontendDvbc2L1Part2Modulation_eMax
} NEXUS_FrontendDvbc2L1Part2Modulation;

/***************************************************************************
Summary:
    This enumeration represents the coding of the L1 part2 signaling data 
    block. Please refer to DVB-C2 ETSI EN 302 769 standard for more 
    information.
****************************************************************************/
typedef enum NEXUS_FrontendDvbc2L1CodeRate
{
    NEXUS_FrontendDvbc2L1CodeRate_e2_3,  /* code rate 2/3 */
    NEXUS_FrontendDvbc2L1CodeRate_e3_4,  /* code rate 3/4 */
    NEXUS_FrontendDvbc2L1CodeRate_e4_5,  /* code rate 4/5 */
    NEXUS_FrontendDvbc2L1CodeRate_e5_6,  /* code rate 5/6 */
    NEXUS_FrontendDvbc2L1CodeRate_e8_9,  /* code rate 8/9 (16K LDPC code) */
    NEXUS_FrontendDvbc2L1CodeRate_e9_10, /* code rate 9/10 (64K LDPC code) */
    NEXUS_FrontendDvbc2L1CodeRate_eMax
} NEXUS_FrontendDvbc2L1CodeRate;

/***************************************************************************
Summary:
    This structure contains ODS FEC Statistics. Please refer to DVB-C2 ETSI 
    EN 302 769 standard for more information.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1part2PlpParams
{
    uint16_t                             plpStart;   /* This 14-bit field indicates the start position of the first complete XFECframe
                                                        of the associated PLP within the current C2 Frame. */
    NEXUS_FrontendDvbc2FecType           plpFecType; /* This 1-bit field indicates the FEC type used by the associated PLP. */
    NEXUS_FrontendDvbc2L1Part2Modulation plpMod;     /* This 3-bit field indicates the modulation used by the associated PLP. */
    NEXUS_FrontendDvbc2L1CodeRate        plpCod;     /* This 3-bit field indicates the code rate used by the associated PLP. */
} NEXUS_FrontendDvbc2L1part2PlpParams;

/***************************************************************************
Summary:
    This structure contains L1 Part 2 Reprocessing information. Please refer
    to DVB-C2 ETSI EN 302 769 standard for more information.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1part2Reprocessing
{
    uint16_t    transportStreamId;  /* This is a 16-bit field which serves as a label for identification of this TS from any other multiplex
                                       within the delivery system */
    uint16_t    originalNetworkId;  /* This 16-bit field gives the label identifying the network_id of the originating delivery system */
} NEXUS_FrontendDvbc2L1part2Reprocessing;


/***************************************************************************
Summary:
    This enumeration represents the type of the associated PLP. Please refer
    to DVB-C2 ETSI EN 302 769 standard for more information.
****************************************************************************/
typedef enum NEXUS_FrontendDvbc2PlpType
{
    NEXUS_FrontendDvbc2PlpType_eCommon,      /* Common PLP */
    NEXUS_FrontendDvbc2PlpType_eGroupedData, /* Grouped Data PLP */
    NEXUS_FrontendDvbc2PlpType_eNormalData,  /* Normal Data PLP */
    NEXUS_FrontendDvbc2PlpType_eMax
} NEXUS_FrontendDvbc2PlpType;

/***************************************************************************
Summary:
    This enumeration represents the type of the payload data carried by the 
    associated PLP. Please to DVB-C2 ETSI EN 302 769 standard for more 
    information.
****************************************************************************/
typedef enum NEXUS_FrontendDvbc2PlpPayloadType
{
    NEXUS_FrontendDvbc2PlpPayloadType_eGfps,  /* GFPS */
    NEXUS_FrontendDvbc2PlpPayloadType_eGcs,   /* GCS */
    NEXUS_FrontendDvbc2PlpPayloadType_eGse,   /* GSE */
    NEXUS_FrontendDvbc2PlpPayloadType_eTs,    /* TS */
    NEXUS_FrontendDvbc2PlpPayloadType_eMax
} NEXUS_FrontendDvbc2PlpPayloadType;

/***************************************************************************
Summary:
    This structure contains L1 Part 2 Dslice PLP Loop information. Please 
    refer to DVB-C2 ETSI EN 302 769 standard for more information.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1part2DslicePlp
{
    uint8_t                                dsliceId;        /* This 8-bit field uniquely identifies a Data Slice within the C2 System. */
    uint8_t                                plpId;           /* This 8-bit field identifies a PLP within the C2 System. */
    bool                                   plpBundled;      /* This 1-bit field indicates whether the associated PLP is bundled with other
                                                               PLP(s) or not within the current C2 System. If the associated PLP is bundled,
                                                               this field shall be set to '1'. Otherwise this field shall be set to '0'. */
    NEXUS_FrontendDvbc2PlpType             plpType;         /* This 2-bit field indicates the type of the associated PLP. PLP_TYPE shall be
                                                               signalled according to table 23. */
    NEXUS_FrontendDvbc2PlpPayloadType      plpPayloadType;  /* This 5-bit field indicates the type of the payload data carried by the associated PLP. */
    uint8_t                                plpGroupId;      /* This 8-bit field identifies with which PLP group within the C2 System the current
                                                               PLP is associated. This can be used by a receiver to link the Data PLP to its
                                                               associated Common PLP, which will have the same PLP_GROUP_ID. */
    NEXUS_FrontendDvbc2L1part2PlpParams    plpParams;       /* L1 Part 2 PLP Parameters */
    bool                                   psiReprocessing; /* If this is set to true, then PSI/SI reprocessing is performed.*/
    NEXUS_FrontendDvbc2L1part2Reprocessing reprocessing;    /* L1 Part 2 Reprocessing, only valid if psiReprocessing = true. */
} NEXUS_FrontendDvbc2L1part2DslicePlp;

/***************************************************************************
Summary:
    This enum specifies the DVB-C2 time interleaving modes. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information. 
****************************************************************************/
typedef enum NEXUS_FrontendDvbc2TimeInterleaving
{
    NEXUS_FrontendDvbc2TimeInterleaving_e0x, /* Time Interleaver Depth 0 */
    NEXUS_FrontendDvbc2TimeInterleaving_e1x, /* Time Interleaver Depth 1 */
    NEXUS_FrontendDvbc2TimeInterleaving_e2x, /* Time Interleaver Depth 2 */
    NEXUS_FrontendDvbc2TimeInterleaving_e3x, /* Time Interleaver Depth 3 */
    NEXUS_FrontendDvbc2TimeInterleaving_eMax
}NEXUS_FrontendDvbc2TimeInterleaving;

/***************************************************************************
Summary:
    This structure contains L1 Part 2 Dslice loop information. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1Part2Dslice
{
    uint8_t                              dsliceId;          /* This 8-bit field uniquely identifies a Data Slice within the C2 System. */
    uint16_t                             dsliceTunePos;     /* This field indicates the tuning position of the associated Data Slice relative to
                                                               the startFrequency. Its bit width shall be 13 bits or 14 bits according to the
                                                               guardInterval value. */
    uint16_t                             dsliceOffsetLeft;  /* This field indicates the start position of the associated Data Slice by means
                                                               of the distance to the left from the tuning position and shall be two's complement
                                                               integer of 8 bits or 9 bits according to the guardInterval value.*/
    uint16_t                             dsliceOffsetRight; /* This field indicates the end position of the associated Data Slice by means of
                                                               the distance to the right from the tuning position and shall be two's complement
                                                               integer of 8 bits or 9 bits according to the guardInterval value. */
    NEXUS_FrontendDvbc2TimeInterleaving  timeInterLeaving;  /* This 2-bit field indicates the time interleaving depth within the associated Data Slice. */
    NEXUS_FrontendDvbc2DsliceType        dsliceType;        /* This 1-bit field indicates the type of the associated Data Slice. */
    NEXUS_FrontendDvbc2FecHeaderType     fecHeaderType;     /* This 1-bit field indicates the type of the FEC Frame header within the associated Data Slice. */
    bool                                 dsliceConstConf;   /* This 1-bit field indicates whether the configuration of the associated Data Slice is variable or fixed. */
    bool                                 dsliceLeftNotch;   /* This 1-bit field indicates the presence of the left neighboured Notch band of the associated Data Slice.*/
    uint8_t                              dsliceNumPlp;      /* This 8-bit field indicates the number of PLPs carried within the associated Data Slice. */
    uint32_t                             plpsPresent[NEXUS_MAX_DVBC2_L1PLP_DSLICE];  /* Plps present in the Dslice */
} NEXUS_FrontendDvbc2L1Part2Dslice;

/***************************************************************************
Summary:
    This structure contains L1 Dslice loop information. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1Dslice
{
    uint8_t                     dsliceId;                                  /* This 8-bit field uniquely identifies a Data Slice within the C2 System. */
    uint8_t                     dsliceNumPlp;                              /* This 8-bit field indicates the number of PLPs carried within the associated Data Slice. */
    uint32_t                    plpsPresent[NEXUS_MAX_DVBC2_L1PLP_DSLICE]; /* Plps present in the Dslice */
} NEXUS_FrontendDvbc2L1Dslice;

/***************************************************************************
Summary:
    This structure is returned when BODS_GetSelevtiveAsyncStatus()  is called.
    This structure contains the complete L1 Dslice status of an ODS 
    channel. Please refer to DVB-C2 ETSI EN 302 769 standard for more 
    information.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1DsliceStatus
{
    uint8_t                      numDslices;                            /* number of Data Slices carried within the within the current C2 Frame. */
    NEXUS_FrontendDvbc2L1Dslice  dslices[NEXUS_MAX_DVBC2_DSLICE_INFO];  /* Dslice Table. */
} NEXUS_FrontendDvbc2L1DsliceStatus;

/***************************************************************************
Summary:
    This structure is returned when BODS_GetSelevtiveAsyncStatus() is called.
    This structure contains L1 Part 2 Notch loop information. Please refer to
    DVB-C2 ETSI EN 302 769 standard for more information.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1part2Notch
{
    uint16_t  notchStart;   /* This field indicates the start position of the associated Notch band. */
    uint16_t  notchWidth;   /* This field indicates the width of the associated Notch band. */
} NEXUS_FrontendDvbc2L1part2Notch;

/***************************************************************************
Summary:
    This structure is returned when BODS_GetSelevtiveAsyncStatus()  is called.
    This structure contains the complete L1 Notch status of an ODS 
    channel. Please refer to DVB-C2 ETSI EN 302 769 standard for more 
    information.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1NotchStatus
{
    uint8_t                          numNotches;                                /* number of PLPs found in the within the current C2 Frame. */
    NEXUS_FrontendDvbc2L1part2Notch  notches[NEXUS_MAX_DVBC2_L1NOTCH_C2_INFO];  /* Notch Table */
} NEXUS_FrontendDvbc2L1NotchStatus;

/***************************************************************************
Summary:
    This structure is returned when BTC2_GetAsyncStatus()  is called.  This
    structure contains the PLP informationl. 
    Applicable only to DVBC2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2Plp
{
    uint8_t                            plpId;
    uint8_t                            plpGroupId;
    NEXUS_FrontendDvbc2PlpPayloadType  plpPayloadType;
    NEXUS_FrontendDvbc2PlpType         plpType;
} NEXUS_FrontendDvbc2Plp;

/***************************************************************************
Summary:
    This structure is returned when BTC2_GetAsyncStatus()  is called.  This
    structure contains the complete L1 Post Configurable status. 
    Applicable only to DVBC2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1PlpStatus
{
    uint8_t                 numPlp;   /* number of PLPs found in the RF signal */
    NEXUS_FrontendDvbc2Plp  plp[NEXUS_MAX_DVBC2_PLP_STATUS];
} NEXUS_FrontendDvbc2L1PlpStatus;

/***************************************************************************
Summary:
    This structure contains DVBC2 FEC Statistics. 
    Applicable only to DVBC2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2FecStatistics
{
    bool           lock;        /* This field indicates the lock status for the given PLP*/
    int32_t        snrData;     /* This signed number represents the Signal-to-Noise Ratio (SNR) computed from slicing the data carriers in units of dB. */
    uint32_t       ldpcAvgIter; /* This unsigned number represents the average number of iterations by the LDPC decoder. */
    uint32_t       ldpcTotIter; /* This unsigned number represents the accumulated number of  iterations by the LDPC decoder */   
    uint32_t       ldpcTotFrm;  /* This unsigned number represents the accumulated number of  total frames processed by the LDPC decoder. */
    uint32_t       ldpcUncFrm;  /* This unsigned number represents the accumulated number of  uncorrectable frames processed by the LDPC decoder */
    uint32_t       ldpcBER;     /* This unsigned number represents the LDPC BER as a fraction of 2^32.  The actual LDPC BER is therefore computed by
                                   dividing this integer value by 2^32 */
    uint32_t       bchCorBit;   /* This unsigned number represents the accumulated number of  bits corrected by the BCH decoder */
    uint32_t       bchTotBlk;   /* This unsigned number represents the accumulated number of  total blocks processed by the BCH decoder */
    uint32_t       bchClnBlk;   /* This unsigned number represents the accumulated number of  clean blocks processed by the BCH decoder */
    uint32_t       bchCorBlk;   /* This unsigned number represents the accumulated number of  corrected blocks processed by the BCH decoder */  
    uint32_t       bchUncBlk;   /* This unsigned number represents the accumulated number of  uncorrectable blocks processed by the BCH decoder */
} NEXUS_FrontendDvbc2FecStatistics;

/***************************************************************************
Summary:
    This structure contains L1 Part 2 Status. Please refer to DVB-C2 ETSI 
    EN 302 769 standard for more information.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2L1Part2Status
{
    uint16_t                            networkId;                               /* This is a 16-bit field which uniquely identifies the current DVB-C2 network. */
    uint16_t                            c2SystemId;                              /* This 16-bit field uniquely identifies a C2 System within the DVB-C2 network. */
    uint32_t                            startFrequency;                          /* This 24-bit field indicates the start frequency in Hz of the current C2
                                                                                    System by means of the distance from 0 Hz. */
    uint16_t                            c2Bandwidth;                             /* This 16-bit field indicates the bandwidth of the current C2 system. */
    NEXUS_FrontendOfdmGuardInterval     guardInterval;                           /* This field indicates the guard interval of the current C2 Frame. */
    uint16_t                            c2FrameLength;                           /* This 10-bit field gives the number of Data Symbols per C2 Frame */
    uint8_t                             l1Part2ChangeCounter;                    /* This 8-bit field indicates the number of C2 Frames ahead where the configuration
                                                                                    will change. */
    uint8_t                             numDslice;                               /* This 8-bit field indicates the number of Data Slices carried within the current C2 Frame. */
    uint8_t                             numNotch;                                /* This 4-bit field indicates the number of Notch bands. If there is no Notch band
                                                                                    within the current C2 Frame, this field shall be set to '0'. */
    NEXUS_FrontendDvbc2L1Part2Dslice    dslices[NEXUS_MAX_DVBC2_DSLICE_INFO];    /* Dslice Loop */
    NEXUS_FrontendDvbc2L1part2DslicePlp plps[NEXUS_MAX_DVBC2_PLP_INFO];          /* PLP Loop */
    NEXUS_FrontendDvbc2L1part2Notch     notches[NEXUS_MAX_DVBC2_L1NOTCH_C2_INFO];/* Notch Loop */
    bool                                reservedTone;                            /* This 1-bit field indicates whether some carriers are reserved. */
} NEXUS_FrontendDvbc2L1Part2Status;

/***************************************************************************
Summary:
    This structure represents Dvbc2 Basic Status
    Applicable only to DVBC2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2BasicStatus
{
    NEXUS_FrontendOfdmSettings settings; /* Settings provided at last call to NEXUS_Frontend_TuneOfdm */
    bool                           fecLock;              /* Indicates whether the FEC is locked */
    bool                           spectrumInverted;     /* If true, the spectrum is inverted. */
    int32_t                        snr;                  /* SNR value of receiver in 1/100 dB */
    int32_t                        gainOffset;           /* Internal AGC gain offset in 1/100 dB */ 
    int32_t                        carrierOffset;        /* Offset of carrier loop in Hz */
    int32_t                        timingOffset;         /* Offset of timing loop in Hz */
    int32_t                        signalStrength;       /* represents the entire AGC (LNA+TNR+UFE+C2)gain in units of a dBm */
    unsigned                       signalLevelPercent;   /* Signal Level in percent */
    unsigned                       signalQualityPercent; /* Signal Quality in percent */
    unsigned                       reacquireCount;       /* number of reacquisitions performed */
} NEXUS_FrontendDvbc2BasicStatus;

/***************************************************************************
Summary:
****************************************************************************/
typedef enum NEXUS_FrontendDvbc2StatusType
{
    NEXUS_FrontendDvbc2StatusType_eFecStatisticsL1Part2,   /* Dvbc2 L1 Part2 FEC Statistics */  
    NEXUS_FrontendDvbc2StatusType_eFecStatisticsPlpA,      /* Dvbc2 PLPA FEC Statistics */
    NEXUS_FrontendDvbc2StatusType_eFecStatisticsPlpB,      /* Dvbc2 PLPB FEC Statistics */
    NEXUS_FrontendDvbc2StatusType_eL1Part2,                /* Dvbc2 L1 Part2 Status */
    NEXUS_FrontendDvbc2StatusType_eL1Dslice,               /* Dvbc2 L1 Dslice Status*/
    NEXUS_FrontendDvbc2StatusType_eL1Notch,                /* Dvbc2 L1 Notch Status*/
    NEXUS_FrontendDvbc2StatusType_eL1Plp,                  /* Dvbc2 L1 PLP Status */
    NEXUS_FrontendDvbc2StatusType_eBasic,                  /* Dvbc2 Short Status */
    NEXUS_FrontendDvbc2StatusType_eMax 
} NEXUS_FrontendDvbc2StatusType;

/***************************************************************************
Summary:
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2StatusReady
{
    bool type[NEXUS_FrontendDvbc2StatusType_eMax];
} NEXUS_FrontendDvbc2StatusReady;

/***************************************************************************
Summary:
****************************************************************************/
typedef struct NEXUS_FrontendDvbc2Status /* attr{local=true} */
{
    NEXUS_FrontendDvbc2StatusType type;

    union
    {     
        NEXUS_FrontendDvbc2FecStatistics    fecStatistics;         /* DVB-C2 FEC Statistics */
        NEXUS_FrontendDvbc2L1Part2Status    l1Part2;               /* DVB-C2 L1 Part2 Status */        
        NEXUS_FrontendDvbc2L1DsliceStatus   l1Dslice;              /* DVB-C2 L1 Dslice Status */              
        NEXUS_FrontendDvbc2L1NotchStatus    l1Notch;               /* DVB-C2 L1 Notch Status */              
        NEXUS_FrontendDvbc2L1PlpStatus      l1Plp;                 /* DVB-C2 L1 PLP Status */                    
        NEXUS_FrontendDvbc2BasicStatus      basic;                 /* DVB-C2 Short Status */
    } status;    
} NEXUS_FrontendDvbc2Status;

/*******************************************************************************
Summary: Request the dvbc2 asynchronous status of NEXUS_FrontendDvbc2AsyncStatus type.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestDvbc2AsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2StatusType type
    );

/*******************************************************************************
Summary: Get the dvbc2 asynchronous status ready type.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncStatusReady(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2StatusReady *pStatusReady /* [out] */
    );

/*******************************************************************************
Summary: Get the dvbc2 asynchronous status.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncStatus( /* attr{local=true} */
     NEXUS_FrontendHandle handle,
     NEXUS_FrontendDvbc2StatusType type,
     NEXUS_FrontendDvbc2Status *pStatus   /* [out] */
     );

/*******************************************************************************
Summary: Get the dvbc2 asynchronous FEC statistics.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncFecStatistics(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2StatusType type,
    NEXUS_FrontendDvbc2FecStatistics *pStatistics
    );

/*******************************************************************************
Summary: Get the dvbc2 asynchronous L1 Part2 asynchronous status.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncL1Part2Status(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2L1Part2Status *pStatus
    );

/*******************************************************************************
Summary: Get the dvbc2 asynchronous L1 Dslice asynchronous status.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncL1DsliceStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2L1DsliceStatus   *pStatus
    );

/*******************************************************************************
Summary: Get the dvbc2 asynchronous L1 Notch asynchronous status.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncL1NotchStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2L1NotchStatus  *pStatus
    );

/*******************************************************************************
Summary: Get the dvbc2 asynchronous L1 Plp asynchronous status.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncL1PlpStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2L1PlpStatus  *pStatus
    );

/*******************************************************************************
Summary: Get the dvbc2 asynchronousbasic asynchronous status.
********************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncBasicStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2BasicStatus *pStatus
    );

#ifdef __cplusplus
    }
#endif
    
#endif /* #ifndef NEXUS_FRONTEND_DVBC2_H__ */


