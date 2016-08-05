/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description:
 *
 ***************************************************************************/
#ifndef BVBI_PRIV_H__
#define BVBI_PRIV_H__

#include "bvbi.h"
#include "bvbi_cap.h"
#include "bvbi_lcop.h"
#include "bvbi_util_priv.h"
#include "bvbi_chip_priv.h"
#include "blst_list.h"
#include "bavc_hdmi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Convenience macro for swapping two values */
#define P_SWAP(A,B,TEMP)    \
do {                        \
    (TEMP) = (A);           \
    (A)    = (B);           \
    (B)    = (TEMP);        \
} while (0)

#if (BSTD_CPU_ENDIAN != BSTD_ENDIAN_LITTLE)
/* Convenience function for doing little-endian <--> big-endian swap */
uint32_t BVBI_P_LEBE_SWAP (uint32_t ulDatum);
#endif

/***************************************************************************
 * VBI internal enumerations and constants
 ***************************************************************************/

/* bitmasks for individual VBI standards */
#define BVBI_P_SELECT_CC          0x0001
#define BVBI_P_SELECT_CGMSA       0x0002
#define BVBI_P_SELECT_WSS         0x0004
#define BVBI_P_SELECT_TT          0x0008
#define BVBI_P_SELECT_VPS         0x0010
#define BVBI_P_SELECT_GS          0x0020
#define BVBI_P_SELECT_AMOL        0x0040
#define BVBI_P_SELECT_SCTE        0x0080
#define BVBI_P_SELECT_MCC         0x0100
#define BVBI_P_SELECT_CGMSB       0x0200
#define BVBI_P_SELECT_VBIENC      0x0400    /* Pseudo standard */
#define BVBI_P_SELECT_ANCI        0x0800    /* Pseudo standard */
#define BVBI_P_SELECT_LAST        0x1000

/* Enumeration of VEC VBI cores. Not exactly the same as the above. */
typedef enum BVBI_P_EncCoreType
{
    BVBI_P_EncCoreType_eCCE = 0,
    BVBI_P_EncCoreType_eCGMSAE,
    BVBI_P_EncCoreType_eWSE,
    BVBI_P_EncCoreType_eTTE,
    BVBI_P_EncCoreType_eGSE,
    BVBI_P_EncCoreType_eAMOLE,
    BVBI_P_EncCoreType_eSCTE,
    BVBI_P_EncCoreType_eVBIENC,
    BVBI_P_EncCoreType_eANCI,
    BVBI_P_EncCoreType_eLAST    /* Must be last! */
}
BVBI_P_EncCoreType;


/***************************************************************************
 * VBI Internal data structures
 ***************************************************************************/

/* Linked list support: define decode_head and encode_head. */
typedef struct decode_head decode_head;
BLST_D_HEAD(decode_head, BVBI_P_Decode_Handle);
typedef struct encode_head encode_head;
BLST_D_HEAD(encode_head, BVBI_P_Encode_Handle);

/* The chip-compatible storage of teletext data */
typedef BVBI_P_LCOP_COUNTEDPROP(BVBI_P_TTData) BVBI_P_TTData_Counted;
typedef struct BVBI_P_TTData
{
    BVBI_P_TTData_Counted clink;
    uint16_t  ucDataSize;
    uint8_t   ucLines;
    uint8_t   ucLineSize;
    uint8_t   firstLine;
    uint32_t  lineMask;
    uint8_t  *pucData;

} BVBI_P_TTData;
typedef BVBI_P_LCOP_OWNERPROP(BVBI_P_TTData) BVBI_P_TTData_Owner;

BDBG_OBJECT_ID_DECLARE (BVBI);
typedef struct BVBI_P_Handle
{
    BDBG_OBJECT (BVBI)

    /* handed down from app. */
    BCHP_Handle hChip;
    BREG_Handle hReg;
    BMEM_Handle hMem;
    size_t      in656bufferSize;
    bool        tteShiftDirMsb2Lsb;

    /* List of decode contexts */
    decode_head decode_contexts;

    /* List of encode contexts */
    encode_head encode_contexts;

    /* Free lists for bulky data */
    BVBI_P_TTData_Counted ttFreelist;

    /* Usage of VEC hardware cores.
     * Each entry refers to a type of hardware core.
     * Each entry is a bitmask.
     * In each entry, the bits positioned 0, 1, 2 indicate "in use." */
    uint8_t vecHwCoreMask[BVBI_P_EncCoreType_eLAST];
    uint8_t vecHwCoreMask_656[BVBI_P_EncCoreType_eLAST];

} BVBI_P_Handle;

/* Options for parsing SMPTE 291M ancillary data packets from ITU-R 656
   bitstream input */
typedef struct
{
    BVBI_Decode_656_SMPTE291M_Cb fParseCb;
    void* fParseCbArg0;
    bool bLongHeader;
    bool bBrokenDataCount;

} BVBI_P_SMPTE291Moptions;

/*
 * SCTE data internal format. This supports interleaved CC, PAM, and NRTV data,
 * ordered by line number.
 */
typedef struct
{
    uint8_t cc_data_1;
    uint8_t cc_data_2;

} BVBI_P_SCTE_CC_Line_Data;

typedef struct
{
    uint8_t sequence_number;
    uint8_t segment_number;

} BVBI_P_SCTE_NTRV_Line_Data;

typedef struct
{
    bool first_segment_flag;
    bool last_segment_flag;
    uint16_t first_pixel_position;
    uint8_t n_pixels;
    int iArray;

} BVBI_P_SCTE_Mono_Line_Data;

typedef struct
{
    uint8_t valid;
    uint8_t priority;
    uint8_t line_number;
    BVBI_SCTE_Type eType;
    union
    {
        BVBI_P_SCTE_CC_Line_Data     CC;
        BVBI_P_SCTE_NTRV_Line_Data NRTV;
        BVBI_P_SCTE_Mono_Line_Data Mono;
    } d;
} BVBI_P_SCTE_Line_Data;

typedef struct
{
    uint8_t  field_number;
    size_t line_count;
    size_t line_size;
    BVBI_P_SCTE_Line_Data* pLine;
    uint8_t* nrtv_data[2];
    size_t pam_data_count;
    size_t pam_data_size;
    uint8_t* pam_data;
    uint8_t* mono_data[2];

} BVBI_P_SCTE_Data;

/* The current/next state for decoder */
typedef struct BVBI_P_Decode_CNState
{
    /* What video display format to assume */
    BFMT_VideoFmt eVideoFormat;

    /* What format to expect for incoming VBI data in
       ITU-R 656 ancillary data packets */
    BVBI_656Fmt e656Format;

    /* ITU-R 656 / SMPTE 291M options */
    BVBI_P_SMPTE291Moptions SMPTE291Moptions;

    /* WSS options */
    uint16_t wssVline576i;

    /* Gemstar options */
    BVBI_GSOptions gsOptions;

    /* Which VBI standards are active (bitmask of BVBI_P_SELECT_xyz) */
    uint32_t ulActive_Standards;

} BVBI_P_Decode_CNState;

/* The full state for decoder */
BDBG_OBJECT_ID_DECLARE(BVBI_DEC);
typedef struct BVBI_P_Decode_Handle
{
    BDBG_OBJECT (BVBI_DEC)

    /* Current/next information */
    BVBI_P_Decode_CNState curr;
    BVBI_P_Decode_CNState next;
    bool gsOptionsChanged;

    /* Back pointer to the VBI context */
    BVBI_P_Handle *pVbi;

    /* Where to decode from */
    BAVC_SourceId eSource;

    /* LCO storage for bulky VBI data */
    BVBI_P_TTData_Owner topTTDataO;
    BVBI_P_TTData_Owner botTTDataO;

    /* Double buffer storage for ITU-R 656 ancillary data */
    uint8_t* top656Data;
    uint8_t* bot656Data;

    /* Linked list membership */
    BLST_D_ENTRY(BVBI_P_Decode_Handle) link;

} BVBI_P_Decode_Handle;

/* Options for writing ITU-R 656 ancillary data packets */
typedef struct
{
    BVBI_656Fmt e656Format;
    uint8_t sdid;

} BVBI_P_Encode_656_Options;

/* The current/next state for encoder */
typedef struct BVBI_P_Encode_CNState
{
    /* What video display format to assume */
    BFMT_VideoFmt eVideoFormat;

    /* Options for encapsulating VBI data into
       ITU-R 656 ancillary data packets */
    BVBI_P_Encode_656_Options h656options;

    /* Which VBI standards are active for analog output
       (bitmask of BVBI_P_SELECT_xyz) */
    uint32_t ulActive_Standards;

    /* Same as above, but for ITU-R 656 output instead of analog output. */
    uint32_t ulActive_656_Standards;

    /* Same as above, but for serial output. */
    uint32_t ulActive_XSER_Standards;

    /* Serial output options */
    BVBI_XSER_Settings xserSettings;

    /* Gemstar options */
    BVBI_GSOptions gsOptions;

    /* AMOL options */
    BVBI_AMOL_Type amolType;

    /* SCTE options */
    BVBI_SCTE_Type scteType;
    BVBI_CSC eCsc;

    /* CGMS-B options */
    bool bCea805dStyle;

    /* As of 2005-12-05, all chip products 97038 and later are affected by
     * PR18010. The BVBI porting interface "works around" this bug in software,
     * by default. However, when this flag is "true," then the buggy behavior
     * will occur. This flag is provided for customers who have accomodated the
     * buggy behavior and do not wish to switch over to correct behavior.
     */
    bool bPR18010_bad_line_number;

    /* This identifies each VEC VBI core to use.
     * Each entry refers to a type of hardware core.
     * Each entry is either 0, 1, 2, or 0xFF.
     * 0xFF means "unassigned."
     * Entry BVBI_P_EncCoreType_eVBIENC is a special case. There is only a
     * single core of type VBI_ENC. In this case, the value stored in the
     * array indicates which VBI path is in use.                           */
    uint8_t hwCoreIndex[BVBI_P_EncCoreType_eLAST];

} BVBI_P_Encode_CNState;

/* The full state for encoder */
BDBG_OBJECT_ID_DECLARE (BVBI_ENC);
typedef struct BVBI_P_Encode_Handle
{
    BDBG_OBJECT (BVBI_ENC)

    /* Current/next information */
    BVBI_P_Encode_CNState curr;
    BVBI_P_Encode_CNState next;

    /* Back pointer to the VBI context */
    BVBI_P_Handle *pVbi;

    /* Where to decode to */
    BAVC_VbiPath eDest;

    /* Used to temporarily disable VBI encoding */
    bool bDisabled;

    /* Double buffer storage for bulky VBI data */
    BVBI_P_TTData_Owner topTTDataO;
    BVBI_P_TTData_Owner botTTDataO;
    BVBI_LineBuilder_Handle hTopScteNrtv[2];
    BVBI_LineBuilder_Handle hBotScteNrtv[2];
    BVBI_LineBuilder_Handle hTopScteMono[2];
    BVBI_LineBuilder_Handle hBotScteMono[2];
    uint8_t* sctePamData;

    /* Cache CGMS data from user */
#ifdef P_CGMS_SOFTWARE_CRC
    uint32_t last_cgms_user;
    uint32_t last_cgms_formatted;
#endif /* P_CGMS_SOFTWARE_CRC */

    /* ARIB video? */
    bool bArib480p;

    /* Support teletext output? */
    bool bSupportTeletext;

    /* Linked list membership */
    BLST_D_ENTRY(BVBI_P_Encode_Handle) link;

} BVBI_P_Encode_Handle;

BDBG_OBJECT_ID_DECLARE (BVBI_FIELD);
typedef struct BVBI_P_Field_Handle
{
    BDBG_OBJECT (BVBI_FIELD)

    /* Pointer back to BVBI main context */
    BVBI_P_Handle* pVbi;

    /* Raw data storage */
    uint16_t             usCCData;
    uint16_t             usWSSData;
    uint32_t             ulCGMSData;
    BVBI_MCCData         *pMCCData;
    BVBI_P_TTData_Owner TTDataO;
    BVBI_VPSData         *pVPSData;
    BVBI_GSData          *pGSData;
    BVBI_AMOL_Type       amolType;
    uint8_t              *pAmolData;
    int                  amolSize;
    BVBI_P_SCTE_Data*    pPScteData;
    BVBI_CGMSB_Datum*    pCgmsbDatum;

    /* Which of the above pieces of data is VALID
       (bitmask of BVBI_P_SELECT_xyz) */
    uint32_t ulWhichPresent;

    /* Bitmask of error conditions from most
    recent encode or decode operation */
    uint32_t ulErrInfo;

    /* Even/odd field parity bitmask.  May be "unspecified" or "any." */
    uint32_t polarityMask;

    /* In-use counter.  Used by BVBIlib syslib module. */
    int inUseCount;

} BVBI_P_Field_Handle;


/***************************************************************************
 * VBI private closed caption (CC) functions
 ***************************************************************************/
BERR_Code BVBI_P_CC_Init ( BVBI_P_Handle *pVbi );
void BVBI_P_CC_Enc_Init (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex);
BERR_Code BVBI_P_CC_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    bool bPR18010_bad_line_number,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p);
BERR_Code BVBI_P_CC_Encode_Data_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BAVC_Polarity polarity,
    uint16_t usData );
BERR_Code BVBI_P_CC_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable);
uint16_t BVBI_P_SetCCParityBits_isr (
    uint16_t uchData);
void BVBI_P_CC_Dec_Init (BREG_Handle hReg, uint32_t ulCoreOffset);
uint32_t BVBI_P_CC_Decode_Data_isr (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    BAVC_Polarity polarity,
    uint16_t *pusData );
BERR_Code BVBI_P_CC_Dec_Program (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    bool bActive,
    BFMT_VideoFmt eVideoFormat);


/***************************************************************************
 * VBI private CGMS functions
 ***************************************************************************/
BERR_Code BVBI_P_CGMS_Init ( BVBI_P_Handle *pVbi );

void BVBI_P_CGMS_Enc_Init (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex);
BERR_Code BVBI_P_CGMSA_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p);
BERR_Code BVBI_P_CGMSA_Encode_Data_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BAVC_Polarity polarity,
    uint32_t ulData );
BERR_Code BVBI_P_CGMSA_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable);
#if defined(BVBI_P_CGMSAE_VER2) || defined(BVBI_P_CGMSAE_VER3) || \
    defined(BVBI_P_CGMSAE_VER5) /** { **/
BERR_Code BVBI_P_CGMSB_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p,
    bool bCea805dStyle);

BERR_Code BVBI_P_CGMSB_Encode_Data_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BAVC_Polarity polarity,
    BVBI_CGMSB_Datum cgmsbDatum );

BERR_Code BVBI_P_CGMSB_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable);
#endif /** } **/

void BVBI_P_CGMS_Dec_Init (BREG_Handle hReg, uint32_t ulCoreOffset);

BERR_Code BVBI_P_CGMS_Dec_Program (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    bool bActive,
    BFMT_VideoFmt eVideoFormat);
uint32_t BVBI_P_CGMS_Decode_Data_isr (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    BAVC_Polarity polarity,
    uint32_t *pulData );

uint32_t BVPI_P_CGMS_format_data_isr (uint32_t userdata);


/***************************************************************************
 * VBI private WSS functions
 ***************************************************************************/
BERR_Code BVBI_P_WSS_Init ( BVBI_P_Handle *pVbi );

void BVBI_P_WSS_Enc_Init     (BREG_Handle hReg, uint8_t hwCoreIndex);
void BVBI_P_WSS_656_Enc_Init (BREG_Handle hReg, uint8_t hwCoreIndex);
BERR_Code BVBI_P_WSS_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    bool bPR18010_bad_line_number,
    BFMT_VideoFmt eVideoFormat);
BERR_Code BVBI_P_WSS_Encode_Data_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BAVC_Polarity polarity,
    uint16_t usData );
BERR_Code BVBI_P_WSS_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable);

void BVBI_P_WSS_Dec_Init (BREG_Handle hReg, uint32_t ulCoreOffset);

BERR_Code BVBI_P_WSS_Dec_Program (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    uint16_t wssVline576i);
uint32_t BVBI_P_WSS_Decode_Data_isr (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    BAVC_Polarity polarity,
    uint16_t *pusData );


/***************************************************************************
 * VBI private VPS functions
 ***************************************************************************/
BERR_Code BVBI_P_VPS_Init ( BVBI_P_Handle *pVbi );

void BVBI_P_VPS_Enc_Init (BREG_Handle hReg, uint8_t hwCoreIndex);
BERR_Code BVBI_P_VPS_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable);
BERR_Code BVBI_P_VPS_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat);
BERR_Code BVBI_P_VPS_Encode_Data_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BAVC_Polarity polarity,
    BVBI_VPSData *pVPSData );

/***************************************************************************
 * VBI private Teletext functions
 ***************************************************************************/

BERR_Code BVBI_P_TT_Init ( BVBI_P_Handle *pVbi );

void BVBI_P_TT_Enc_Init (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex);
BERR_Code BVBI_P_TT_Enc_Program (
    BREG_Handle hReg,
    BMEM_Handle hMem,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    bool bXserActive,
    BFMT_VideoFmt eVideoFormat,
    bool tteShiftDirMsb2Lsb,
    BVBI_XSER_Settings* xserSettings,
    BVBI_P_TTData* topData,
    BVBI_P_TTData* botData
);
uint32_t BVBI_P_TT_Encode_Data_isr (
    BREG_Handle hReg,
    BMEM_Handle hMem,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    bool bPR18010_bad_line_number,
    BVBI_P_TTData* pTTDataNext );
BERR_Code BVBI_P_TT_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable);

void BVBI_P_TT_Dec_Init (BREG_Handle hReg, uint32_t ulCoreOffset);
BERR_Code BVBI_P_TT_Dec_Program (
    BREG_Handle hReg,
    BMEM_Handle hMem,
    BAVC_SourceId eSource,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    BVBI_P_TTData* topData,
    BVBI_P_TTData* botData
);
uint32_t BVBI_P_TT_Decode_Data_isr (
    BREG_Handle hReg,
    BMEM_Handle hMem,
    BAVC_SourceId eSource,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    BVBI_P_TTData* pTTDataNext );

uint32_t BVBI_P_TT_Size_Storage(uint32_t ulMaxLines, uint32_t ulMaxLineSize);
BERR_Code BVBI_P_TTData_Alloc (
    BMEM_Handle hMem, uint8_t ucMaxLines, uint8_t ucLineSize,
    BVBI_P_TTData* pTTData);

/***************************************************************************
 * VBI private Gemstar functions
 ***************************************************************************/

BERR_Code BVBI_P_GS_Init ( BVBI_P_Handle *pVbi );
void BVBI_P_GS_Enc_Init (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex);

BERR_Code BVBI_P_GS_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p,
    BVBI_GSOptions* gsOptions
);
uint32_t BVBI_P_GS_Encode_Data_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    BVBI_GSData* pGSDataNext );
BERR_Code BVBI_P_GS_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable);

/***************************************************************************
 * VBI private AMOL functions
 ***************************************************************************/

BERR_Code BVBI_P_AMOL_Init ( BVBI_P_Handle *pVbi );
void BVBI_P_AMOL_Enc_Init (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex);

BERR_Code BVBI_P_AMOL_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p,
    BVBI_AMOL_Type amolType
);
uint32_t BVBI_P_AMOL_Encode_Data_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    BVBI_AMOL_Type amolType,
    uint8_t* pAMOLData );
BERR_Code BVBI_P_AMOL_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable);

/***************************************************************************
 * VBI private multi-line closed caption functions
 ***************************************************************************/

BERR_Code BVBI_P_MCC_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p
);
uint32_t BVBI_P_MCC_Encode_Data_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p,
    BAVC_Polarity polarity,
    bool bPR18010_bad_line_number,
    BVBI_MCCData* pMCCData );
BERR_Code BVBI_P_MCC_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    bool bEnable);

/***************************************************************************
 * VBI private SCTE functions
 ***************************************************************************/

BERR_Code BVBI_P_SCTE_Init ( BVBI_P_Handle *pVbi );

void BVBI_P_SCTE_Enc_Init (BREG_Handle hReg, uint8_t hwCoreIndex);

BERR_Code BVBI_P_SCTE_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    bool bActive,
    BFMT_VideoFmt eVideoFormat,
    bool bArib480p,
    BVBI_SCTE_Type scteType,
    BVBI_CSC csc
);
uint32_t BVBI_P_SCTE_Encode_Data_isr (
    BREG_Handle hReg,
    BMEM_Handle hMem,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BAVC_Polarity polarity,
    BVBI_SCTE_Type scteType,
    BVBI_P_SCTE_Data* pData,
    BVBI_LineBuilder_Handle hTopScteNrtv[2],
    BVBI_LineBuilder_Handle hBotScteNrtv[2],
    BVBI_LineBuilder_Handle hTopScteMono[2],
    BVBI_LineBuilder_Handle hBotScteMono[2],
    uint8_t** pSctePamData
);
BERR_Code BVBI_P_SCTE_Encode_Enable_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    BFMT_VideoFmt eVideoFormat,
    BVBI_SCTE_Type scteType,
    bool bEnable);
BERR_Code BVBI_P_SCTEData_Alloc (
    BMEM_Handle hMem, size_t cc_size, bool scteEnableNrtv, size_t pam_size,
    bool scteEnableMono, BVBI_P_SCTE_Data* pPScteData);
BERR_Code BVBI_P_Encode_AllocScte (
    BMEM_Handle hMem,
    BVBI_LineBuilder_Handle topScteNrtv[2],
    BVBI_LineBuilder_Handle botScteNrtv[2],
    BVBI_LineBuilder_Handle topScteMono[2],
    BVBI_LineBuilder_Handle botScteMono[2],
    uint8_t** pSctePam);
void BVBI_P_Encode_FreeScte (
    BMEM_Handle hMem,
    BVBI_LineBuilder_Handle topScteNrtv[2],
    BVBI_LineBuilder_Handle botScteNrtv[2],
    BVBI_LineBuilder_Handle topScteMono[2],
    BVBI_LineBuilder_Handle botScteMono[2],
    uint8_t** pSctePam);

/***************************************************************************
 * VBI private VDTOP functions
 ***************************************************************************/
BERR_Code BVBI_P_VDTOP_Dec_Program (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    BFMT_VideoFmt eVideoFormat);

BERR_Code BVBI_P_VDTOP_Dec_Reset (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    uint32_t whichStandard);

BERR_Code BVBI_P_VDTOP_656_Dec_Program (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    BFMT_VideoFmt eVideoFormat);

/***************************************************************************
 * VBI private VBI_ENC functions
 ***************************************************************************/
BERR_Code BVBI_P_VE_Init  ( BVBI_P_Handle *pVbi );
BERR_Code BVBI_P_VE_Enc_Program (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    uint32_t ulActive_Standards,
    uint32_t ulActive_656_Standards,
    BFMT_VideoFmt eVideoFormat);
void BVBI_P_VE_Crossbar_Program (
    BREG_Handle hReg,
    BAVC_VbiPath eDest,
    uint8_t hwCoreIndex[BVBI_P_EncCoreType_eLAST]);

#ifdef BVBI_P_HAS_XSER_TT
/***************************************************************************
 * VBI private ITU656 functions
 ***************************************************************************/
BERR_Code BVBI_P_ITU656_Init(
    BREG_Handle hReg, const BVBI_XSER_Settings* pXSERdefaultSettings );
BERR_Code BVBI_P_ITU656_Enc_Program (
    BREG_Handle hReg,
    BVBI_XSER_Settings* pSettings,
    uint32_t ulActive_XSER_Standards);
#endif

/***************************************************************************
 * VBI private ANCI656 functions
 ***************************************************************************/
#if (BVBI_NUM_ANCI656_656 > 0)
BERR_Code BVBI_P_A656_Init  ( BVBI_P_Handle *pVbi );
BERR_Code BVBI_P_A656_Enc_Program (
    BREG_Handle hReg,
    uint8_t hwCoreIndex,
    BVBI_P_Encode_656_Options* h656options,
    bool bPR18010_bad_line_number,
    BFMT_VideoFmt eVideoFormat);
#endif

/***************************************************************************
 * VBI private IN656 functions
 ***************************************************************************/
BERR_Code BVBI_P_IN656_Init  ( BVBI_P_Handle *pVbi );
BERR_Code BVBI_P_IN656_Dec_Program (
    BREG_Handle hReg,
    BMEM_Handle hMem,
    BAVC_SourceId eSource,
    bool bActive,
    BVBI_656Fmt anci656Fmt,
    BVBI_P_SMPTE291Moptions* pMoptions,
    BFMT_VideoFmt eVideoFormat,
    uint8_t* topData,
    uint8_t* botData);
BERR_Code BVBI_P_IN656_Decode_Data_isr (
    BREG_Handle hReg,
    BAVC_SourceId eSource,
    BAVC_Polarity polarity,
    bool* bDataFound);

/***************************************************************************
 * VBI private 656 (software parser) functions
 ***************************************************************************/
BERR_Code BVBI_P_P656_Init  ( BVBI_P_Decode_Handle* pVbi_Dec );
void      BVBI_P_P656_DeInit  ( BVBI_P_Decode_Handle* pVbi_Dec );
BERR_Code BVBI_P_P656_Process_Data_isr (
    BAVC_Polarity polarity,
    BVBI_P_Decode_Handle* pVbi_Dec,
    BVBI_P_Field_Handle* pVbi_Fld);
uint8_t BVBI_P_p656_SetEEbits (uint8_t arg);

/***************************************************************************
 * VBI private Video Encoder (top level) functions
 ***************************************************************************/
BERR_Code BVBI_P_VIE_SoftReset_isr (
    BREG_Handle hReg,
    bool is656,
    uint8_t hwCoreIndex,
    uint32_t whichStandard);
#if (BVBI_NUM_ANCI656_656 > 0)
BERR_Code BVBI_P_VIE_AncilSoftReset (
    BREG_Handle hReg,
    uint8_t hwCoreIndex);
#endif

/***************************************************************************
 * Other private, hardware dependent functions
 ***************************************************************************/

BERR_Code BVBI_P_Encode_ReserveCore (
    BAVC_VbiPath eDest, uint32_t ulSelect_Standard,
    uint8_t vecHwCoreMask[BVBI_P_EncCoreType_eLAST],
    uint8_t vecHwCoreMask_656[BVBI_P_EncCoreType_eLAST],
    uint8_t hwCoreIndex[BVBI_P_EncCoreType_eLAST]);
void BVBI_P_Encode_ReleaseCore (
    BAVC_VbiPath eDest, uint32_t ulSelect_Standard,
    uint32_t ulActive_Standards,
    uint8_t vecHwCoreMask[BVBI_P_EncCoreType_eLAST],
    uint8_t vecHwCoreMask_656[BVBI_P_EncCoreType_eLAST],
    uint8_t hwCoreIndex[BVBI_P_EncCoreType_eLAST]);
void BVBI_P_Encode_ConnectCores (
    BREG_Handle hReg, BAVC_VbiPath eDest,
    uint8_t hwCoreIndex[BVBI_P_EncCoreType_eLAST]);

/***************************************************************************
 * Other private functions
 ***************************************************************************/
bool BVBI_P_is656_isr (BAVC_VbiPath eDest);
const BVBI_XSER_Settings * BVBI_P_GetDefaultXserSettings (void);

/*
 * Tuning parameters for retry access to VBI_ENC_PRIM_Control register, via a
 * RDMA scratch variable. These numbers express ten tries per field for 3
 * fields, if a field is 1/60 second.
 */
#define BVBI_P_MAX_HW_TRIES   30
#define BVBI_P_SLEEP_HW        2

#ifdef __cplusplus
}
#endif

#endif /* BVBI_PRIV_H__ */

/* End of file. */
