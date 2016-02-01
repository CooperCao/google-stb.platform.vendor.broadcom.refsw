/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*   API name: Frontend OFDM
*    Generic APIs for OFDM (Orthogonal Frequency-Division Multiplexing) tuning.
*    This is used in DVB-H and DVB-T environments.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_T2_TYPES_H__
#define NEXUS_FRONTEND_T2_TYPES_H__

#include "nexus_frontend_ofdm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_MAX_DVBT2_PLP_STATUS 255

/***************************************************************************
Summary:
    This structure contains DVBT2 FEC Statistics. 
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2FecStatistics
{
    bool           lock;        /* This field indicates the lock status for the given PLP*/
    int32_t        snrData;     /* This signed number represents the Signal-to-Noise Ratio (SNR) computed from slicing the data carriers in units of dB. */
    uint32_t       ldpcAvgIter; /* This unsigned number represents the average number of iterations by the LDPC decoder. */
    uint32_t       ldpcTotIter; /* This unsigned number represents the accumulated number of  iterations by the LDPC decoder */   
    uint32_t       ldpcTotFrm;  /* This unsigned number represents the accumulated number of  total frames processed by the LDPC decoder. */
    uint32_t       ldpcUncFrm;  /* This unsigned number represents the accumulated number of  uncorrectable frames processed by the LDPC decoder */
    uint32_t       ldpcBER;     /* This unsigned number represents the LDPC BER as a fraction of 2^32.  The actual LDPC BER is therefore computed by dividing this integer value by 2^32 */
    uint32_t       bchCorBit;   /* This unsigned number represents the accumulated number of  bits corrected by the BCH decoder */
    uint32_t       bchTotBlk;   /* This unsigned number represents the accumulated number of  total blocks processed by the BCH decoder */
    uint32_t       bchClnBlk;   /* This unsigned number represents the accumulated number of  clean blocks processed by the BCH decoder */
    uint32_t       bchCorBlk;   /* This unsigned number represents the accumulated number of  corrected blocks processed by the BCH decoder */  
    uint32_t       bchUncBlk;   /* This unsigned number represents the accumulated number of  uncorrectable blocks processed by the BCH decoder */
} NEXUS_FrontendDvbt2FecStatistics;


/***************************************************************************
Summary:
    This enumeration indicates the types of the Tx input streams carried 
    within the current DVBT2 super-frame. 
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2StreamType
{
  NEXUS_FrontendDvbt2StreamType_eTs, /* Transport Stream */
  NEXUS_FrontendDvbt2StreamType_eGs, /* Generic Stream (GSE [i.2] and/or GFPS and/or GCS) but not TS */
  NEXUS_FrontendDvbt2StreamType_eMixed, /* Both TS and Generic Stream (i.e. TS and at least one of GSE, GFPS, GCS) */
  NEXUS_FrontendDvbt2StreamType_eMax
} NEXUS_FrontendDvbt2StreamType;

/***************************************************************************
Summary:
   This enumeration represents the PAPR reduction used. 
   Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2Papr
{
  NEXUS_FrontendDvbt2Papr_eNone, /* L1-ACE is used and TR is used on P2 symbols only */
  NEXUS_FrontendDvbt2Papr_eAce,  /* L1-ACE and ACE only are used */
  NEXUS_FrontendDvbt2Papr_eTr,   /* L1-ACE and TR only are used */
  NEXUS_FrontendDvbt2Papr_eBoth, /* L1-ACE, ACE and TR are used */
  NEXUS_FrontendDvbt2Papr_eMax
} NEXUS_FrontendDvbt2Papr;

/***************************************************************************
Summary:
    This enumeration represents the constellation of the L1-post signalling 
    data block. Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2L1Mod
{
  NEXUS_FrontendDvbt2L1Mod_eBpsk,  /* BPSK constellation */
  NEXUS_FrontendDvbt2L1Mod_eQpsk,  /* QPSK constellation */
  NEXUS_FrontendDvbt2L1Mod_eQam16, /* 16-QAM constellation */
  NEXUS_FrontendDvbt2L1Mod_eQam64, /* 64-QAM constellation */
  NEXUS_FrontendDvbt2L1Mod_eMax
} NEXUS_FrontendDvbt2L1Mod;

/***************************************************************************
Summary:
    This enumeration represents the coding of the L1-post signalling data block. 
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2L1CodeRate
{
  NEXUS_FrontendDvbt2L1CodeRate_e1_2, /* code rate 1/2 */
  NEXUS_FrontendDvbt2L1CodeRate_eMax
} NEXUS_FrontendDvbt2L1CodeRate;

/***************************************************************************
Summary:
    This enumeration represents type of the L1 FEC used for the L1-post 
    signalling data block. Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2FecType
{
  NEXUS_FrontendDvbt2FecType_e16K, /* LDPC 16K */
  NEXUS_FrontendDvbt2FecType_e64K, /* LDPC 64k */
  NEXUS_FrontendDvbt2FecType_eMax
} NEXUS_FrontendDvbt2FecType;

/***************************************************************************
Summary:
    This enumeration represents the scattered pilot pattern used for the data
    OFDM symbols. Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2PilotPattern
{
  NEXUS_FrontendDvbt2PilotPattern_e1, /* PP1 */
  NEXUS_FrontendDvbt2PilotPattern_e2, /* PP2 */
  NEXUS_FrontendDvbt2PilotPattern_e3, /* PP3 */
  NEXUS_FrontendDvbt2PilotPattern_e4, /* PP4 */
  NEXUS_FrontendDvbt2PilotPattern_e5, /* PP5 */
  NEXUS_FrontendDvbt2PilotPattern_e6, /* PP6 */
  NEXUS_FrontendDvbt2PilotPattern_e7, /* PP7 */
  NEXUS_FrontendDvbt2PilotPattern_e8, /* PP8 */
  NEXUS_FrontendDvbt2PilotPattern_eMax
} NEXUS_FrontendDvbt2PilotPattern;

/***************************************************************************
Summary:
    This structure contains the complete L1 Pre status. 
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2L1PreStatus
{
    NEXUS_FrontendDvbt2StreamType        streamType;         /* streamType */   
    bool                                 bwtExt;             /* This field indicates whether the extended carrier mode is used in the case of 8K, 16K and 32K FFT 
                                                                   sizes. When this field is set to '1', the extended carrier mode is used. If this field is set to '0', 
                                                                   the normal carrier mode is used */
    uint8_t                              s1;                 /* This field has the same value as in the P1 signalling. */   
    uint8_t                              s2;                 /* This field has the same value as in the P1 signalling. */   
    bool                                 l1RepetitionFlag;   /* This flag indicates whether the dynamic L1-post signalling is provided also for the next frame. */
    NEXUS_FrontendOfdmGuardInterval      guardInterval;      /* This field indicates the guard interval of the current super-frame */
    NEXUS_FrontendDvbt2Papr              papr;               /* This field describes what kind of PAPR reduction is used */
    NEXUS_FrontendDvbt2L1Mod             l1Mod;              /* This field field describes the coding of the L1-post signalling data block */
    NEXUS_FrontendDvbt2L1CodeRate        l1CodeRate;         /*  This field describes the coding of the L1-post signalling data block */
    NEXUS_FrontendDvbt2FecType           l1FecType;          /* This field indicates the type of the L1 FEC used for the L1-post signalling data block */
    NEXUS_FrontendDvbt2PilotPattern      pilotPattern;       /* This 4-bit field indicates the scattered pilot pattern used for the data OFDM symbols. */
    uint8_t                              regenFlag;          /* This 3-bit field indicates how many times the DVB-T2 signal has been re-generated */
    bool                                 l1PostExt;          /* This 1-bit field indicates the presence of the L1-post extension field */
    uint8_t                              numRf;              /* This 3-bit field indicates the number of frequencies in the current T2 system */
    uint8_t                              currentRfIndex;     /* If the TFS mode is supported, this 3-bit field indicates the index of the current RF channel
                                                                   within its TFS structure, between 0 and numRf-1, otherwise it is set to 0 */
    uint8_t                              txIdAvailability;   /* This 8-bit field is used to signal the availability of transmitter identification signals within
                                                                   the current geographic cell. */
    uint8_t                              numT2Frames;        /* This 8-bit field indicates the number of T2-frames per super-frame. The minimum value of
                                                                   numT2Frames shall be 2. */
    uint16_t                             numDataSymbols;     /* This 12-bit field indicates Ldata= LF - NP2, the number of data OFDM symbols per
                                                                   T2-frame, excluding P1 and P2. */
    uint16_t                             cellId;             /* This is a 16-bit field which uniquely identifies a geographic cell in a DVB-T2 network */
    uint16_t                             networkId;          /* This is a 16-bit field which uniquely identifies the current DVB network */
    uint16_t                             t2SystemId;         /* This 16-bit field uniquely identifies a T2 system within the DVB network */
    uint32_t                             l1PostSize;         /* This 18-bit field indicates the size of the coded and modulated L1-post signalling data block, in OFDM cells. */
    uint32_t                             l1PostInfoSize;     /* This 18-bit field indicates the size of the information part of the L1-post signalling data
                                                                    block, in bits, including the extension field, if present, but excluding the CRC */
} NEXUS_FrontendDvbt2L1PreStatus;

/***************************************************************************
Summary:
    This enumeration represents the type of the associated PLP. 
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2PlpType
{
  NEXUS_FrontendDvbt2PlpType_eCommon,    /* Common PLP */
  NEXUS_FrontendDvbt2PlpType_eDataType1, /* DATA PLP Type 1 */
  NEXUS_FrontendDvbt2PlpType_eDataType2, /* DATA PLP Type 2 */
  NEXUS_FrontendDvbt2PlpType_eMax
} NEXUS_FrontendDvbt2PlpType;

/***************************************************************************
Summary:
    This enumeration represents the type of the payload data carried by the 
    associated PLP. Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2PlpPayloadType
{
  NEXUS_FrontendDvbt2PlpPayloadType_eGfps, /* GFPS */
  NEXUS_FrontendDvbt2PlpPayloadType_eGcs,  /* GCS */
  NEXUS_FrontendDvbt2PlpPayloadType_eGse,  /* GSE */
  NEXUS_FrontendDvbt2PlpPayloadType_eTs,   /* TS */
  NEXUS_FrontendDvbt2PlpPayloadType_eMax
} NEXUS_FrontendDvbt2PlpPayloadType;

/***************************************************************************
Summary:
    This enumeration represents the code rate used by the associated PLP.
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2PlpCodeRate
{
  NEXUS_FrontendDvbt2PlpCodeRate_e1_2,
  NEXUS_FrontendDvbt2PlpCodeRate_e3_5,
  NEXUS_FrontendDvbt2PlpCodeRate_e2_3,
  NEXUS_FrontendDvbt2PlpCodeRate_e3_4,
  NEXUS_FrontendDvbt2PlpCodeRate_e4_5,
  NEXUS_FrontendDvbt2PlpCodeRate_e5_6,
  NEXUS_FrontendDvbt2PlpCodeRate_eMax
} NEXUS_FrontendDvbt2PlpCodeRate;

/***************************************************************************
Summary:
    This enumeration represents the modulation used by the associated PLP.
    data block. Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef enum NEXUS_FrontendDvbt2PlpMod
{
  NEXUS_FrontendDvbt2PlpMod_eQpsk,   /* QPSK constellation */
  NEXUS_FrontendDvbt2PlpMod_eQam16,  /* 16-QAM constellation */
  NEXUS_FrontendDvbt2PlpMod_eQam64,  /* 64-QAM constellation */
  NEXUS_FrontendDvbt2PlpMod_eQam256, /* 256-QAM constellation */
  NEXUS_FrontendDvbt2PlpMod_eMax
} NEXUS_FrontendDvbt2PlpMod;

/***************************************************************************
Summary:
     This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete L1 Post Configurable PLP parameters of a TC2 channel.
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2L1PostCfgPlp
{
    uint8_t                              plpId;           /* This 8-bit field identifies uniquely a PLP within the T2 system. */
    uint8_t                              plpGroupId;      /* This 8-bit field identifies with which PLP group within the T2 system the current PLP is associated. */
    NEXUS_FrontendDvbt2PlpType           plpType;         /* This 3-bit field indicates the type of the associated PLP */  
    NEXUS_FrontendDvbt2PlpPayloadType    plpPayloadType;  /* This 5-bit field indicates the type of the payload data carried by the associated PLP */
    bool                                 ffFlag;          /* This flag is set to '1' if a PLP of type 1 in a TFS system occurs on the same RF channel in each
                                                              T2-frame. This flag is set to '0' if inter-frame TFS is applied as described in annex E */
    uint8_t                              firstRfIdx;      /* This 3-bit field indicates on which RF channel a type 1 data PLP occurs in the first frame of
                                                              a super-frame in a TFS system. */
    uint8_t                              firstFrameIdx;   /* This 8-bit field indicates the IDX of the first frame of the super-frame in which the current PLP occurs. */
    NEXUS_FrontendDvbt2PlpCodeRate       plpCodeRate;     /* This field represents the code rate used by the associated PLP. */
    NEXUS_FrontendDvbt2PlpMod            plpMod;          /* This 3-bit field indicates the modulation used by the associated PLP */
    bool                                 plpRotation;     /* This flag indicates whether constellation rotation is in use or not by the associatedPLP */
    NEXUS_FrontendDvbt2FecType           plpFecType;      /* This 2-bit field indicates the FEC type used by the associated PLP */
    uint16_t                             plpNumBlocksMax; /* This 10-bit field indicates the maximum value of plpNumBlocksMax */
    uint8_t                              frameInterval;   /* This 8-bit field indicates the T2-frame interval within the super-frame for the associated PLP */
    uint8_t                              timeIlLength;
    bool                                 timeIlType;      /* This 1-bit field indicates the type of time-interleaving */
    bool                                 inBandFlag;      /* This 1-bit field indicates whether the current PLP carries in-band type A signalling information */
}NEXUS_FrontendDvbt2L1PostCfgPlp;

/***************************************************************************
Summary:
    This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete L1 Post Configurable status. 
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2L1PostConfigurableStatus
{
    uint16_t                        subSlicesPerFrame; /* This 15-bit field indicates Nsubslices_total, the total number of sub-slices for the type 2
                                                           data PLPs across all RF channels in one T2-frame */
    uint8_t                         numPlp;            /* This 8-bit field indicates the number of PLPs carried within the current super-frame */ 
    uint8_t                         numAux;            /* This 4-bit field indicates the number of auxiliary streams */
    uint8_t                         auxConfigRFu;
    uint8_t                         fefType;           /* This 4-bit field shall indicate the type of the associated FEF part */
    uint8_t                         rfIdx;             /* This 3-bit field indicates the index of each FREQUENCY listed within this loop */
    uint8_t                         fefInterval;       /* This 8-bit field indicates the number of T2-frames between two FEF parts */
    uint32_t                        frequency;         /* This 32-bit field indicates the centre frequency in Hz of the RF channel whose index is rfIdx */
    uint32_t                        fefLength;         /* This 22-bit field indicates the length of the associated FEF part as the number of elementary
                                                           periods T, from the start of the P1 symbol of the FEF part to the start of the P1 symbol of the ext T2-frame*/
    NEXUS_FrontendDvbt2L1PostCfgPlp plpA;              /* PLP loop A */
    NEXUS_FrontendDvbt2L1PostCfgPlp plpB;              /* PLP loop B */
    uint8_t                         auxStreamType;     /* This 4-bit field indicates the type of the current auxiliary stream */
    uint32_t                        auxPrivateConf;    /* This 28-bit field is for future use for signalling auxiliary streams */
} NEXUS_FrontendDvbt2L1PostConfigurableStatus;

/***************************************************************************
Summary:
    This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete L1 Post Dynamic PLP parameters of a TC2 channel.
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2L1PostDynamicPlp
{
    uint8_t      plpId;
    uint16_t     plpNumBlocks; /* This 10-bit field indicates the number of FEC blocks contained in the current
                                                           Interleaving Frame for the current PLP */
    uint32_t     plpStart;     /* This 22-bit field indicates the start position of the associated PLP within the current T2-frame
                                                           using the cell addressing scheme */
} NEXUS_FrontendDvbt2L1PostDynamicPlp;

/***************************************************************************
Summary:
    This structure is returned when BTC2_GetAsyncStatus() is called.  This
    structure contains the complete L1 Post Dynamic status. 
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2L1PostDynamicStatus
{
    uint8_t                             frameIdx;            /* This 8-bit field is the index of the current T2-frame within a super-frame */
    uint8_t                             l1ChanlgeCounter;    /* This 8-bit field indicates the number of super-frames ahead where the configuration will change */
    uint8_t                             startRfIdx;          /* This 3-bit field indicates the ID of the starting frequency of the TFS scheduled frame, for the next T2-frame */
    uint32_t                            subSliceInterval;    /* This 22-bit field indicates the number of OFDM cells from the start of one sub-slice of one
                                                                    PLP to the start of the next sub-slice of the same PLP on the same RF channel for the current T2-frame */
    uint32_t                            type2Start;          /* This 22-bit field indicates the start position of the first of the type 2 PLPs using the cell addressing scheme */
    NEXUS_FrontendDvbt2L1PostDynamicPlp plpA;                /* plp loop A */
    NEXUS_FrontendDvbt2L1PostDynamicPlp plpB;                /* plp loop B */
    uint32_t                            auxPrivateDyn_47_32; /* This 48-bit field is reserved for future use for signalling auxiliary streams. */
    uint32_t                            auxPrivateDyn_31_0;  /* This 48-bit field is reserved for future use for signalling auxiliary streams. */
} NEXUS_FrontendDvbt2L1PostDynamicStatus;

/***************************************************************************
Summary:
    This structure is returned when BTC2_GetAsyncStatus()  is called.  This
    structure contains the PLP informationl. 
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2Plp
{
    uint8_t     plpId;
    uint8_t     plpGroupId;
    NEXUS_FrontendDvbt2PlpPayloadType    plpPayloadType;  /* This 5-bit field indicates the type of the payload data carried by the associated PLP */
    NEXUS_FrontendDvbt2PlpType           plpType;         /* This 3-bit field indicates the type of the associated PLP */
} NEXUS_FrontendDvbt2Plp;

/***************************************************************************
Summary:
    This structure is returned when BTC2_GetAsyncStatus()  is called.  This
    structure contains the complete L1 Post Configurable status. 
    Applicable only to DVBT2 channel according to ETSI EN 302 755 standard.
****************************************************************************/
typedef struct NEXUS_FrontendDvbt2L1PlpStatus
{
    uint8_t                 numPlp;   /* number of PLPs found in the RF signal */
    NEXUS_FrontendDvbt2Plp  plp[NEXUS_MAX_DVBT2_PLP_STATUS];
} NEXUS_FrontendDvbt2L1PlpStatus;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_T2_TYPES_H__ */

