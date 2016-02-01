/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

/****************************************************************************
Module Overview:
    BMXT is a Mini XPT (Transport) module
****************************************************************************/

#ifndef BMXT_H__
#define BMXT_H__

#include "bstd.h"
#include "breg_mem.h"
#include "bchp.h"
#include "bmem.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    The maximum number of each type of resource that could be present on a
    given chip. To get the actual number of resources for a specific chip,
    use BMXT_GetNumResources().
****************************************************************************/
#define BMXT_MAX_NUM_MTSIF_TX    (3)
#define BMXT_MAX_NUM_MTSIF_RX    (1)
#define BMXT_MAX_NUM_INPUT_BANDS (26)
#define BMXT_MAX_NUM_PARSERS     (32)
#define BMXT_MAX_NUM_TSMF        (16)

/* for backward-compatibility */
#define BMXT_NUM_MTSIF           BMXT_MAX_NUM_MTSIF_TX
#define BMXT_NUM_INPUT_BANDS     BMXT_MAX_NUM_INPUT_BANDS
#define BMXT_NUM_PARSERS         BMXT_MAX_NUM_PARSERS
#define BMXT_NUM_TSMF            BMXT_MAX_NUM_TSMF

/***************************************************************************
Summary:
    Frontend chip to be targeted.
****************************************************************************/
typedef enum BMXT_Chip
{
    BMXT_Chip_e3128 = 0,
    BMXT_Chip_e3158,
    BMXT_Chip_e3383,
    BMXT_Chip_e3384,
    BMXT_Chip_e3472,
    BMXT_Chip_e4517,
    BMXT_Chip_e4528,
    BMXT_Chip_e4538,
    BMXT_Chip_e4548,
    BMXT_Chip_e45216,
    BMXT_Chip_e45308,
    BMXT_Chip_e7145,
    BMXT_Chip_e7366,
    BMXT_Chip_e7364,
    BMXT_Chip_eMax
} BMXT_Chip;

/***************************************************************************
Summary:
    Frontend chip revision to be targeted.
****************************************************************************/
typedef enum BMXT_ChipRev
{
    BMXT_ChipRev_eA0 = 0,
    BMXT_ChipRev_eA1,
    BMXT_ChipRev_eA2,
    BMXT_ChipRev_eB0,
    BMXT_ChipRev_eB1,
    BMXT_ChipRev_eB2,
    BMXT_ChipRev_eC0,
    BMXT_ChipRev_eC1,
    BMXT_ChipRev_eC2,
    BMXT_ChipRev_eMax
} BMXT_ChipRev;

/***************************************************************************
Summary:
    The handle for the transport module. Users should not directly access the
    contents.
****************************************************************************/
typedef struct BMXT_P_TransportData *BMXT_Handle;

/***************************************************************************
Summary:
    The types of HW resources whose numbers can vary from chip to chip.
****************************************************************************/
typedef enum BMXT_ResourceType
{
    BMXT_ResourceType_eInputBand = 0,
    BMXT_ResourceType_eParser,
    BMXT_ResourceType_eTsmf,
    BMXT_ResourceType_eMtsifRx,
    BMXT_ResourceType_eMtsifTx,
    BMXT_ResourceType_eParserTbg,
    BMXT_ResourceType_eMax
} BMXT_ResourceType;

#define BMXT_ResourceType_eMtsif BMXT_ResourceType_eMtsifTx /* for backward compatibility */

/***************************************************************************
Summary:
    Returns the actual number of resources for a given type, for the given
    chip. The chip ID and revision are specified as part of BMXT_Settings
    to BMXT_Open().
****************************************************************************/
unsigned BMXT_GetNumResources(
    BMXT_Chip chip, /* [in] */
    BMXT_ChipRev rev, /* [in] */
    BMXT_ResourceType resource /* [in] */
    );

/***************************************************************************
Summary:
    Controls the type of timestamp generated as packets pass through this parser band.
****************************************************************************/
typedef enum BMXT_ParserTimestampMode
{
    BMXT_ParserTimestampMode_eAutoSelect = 0, /* Use Mod-300 timestamp if the transport stream is MPEG; use binary timestamp otherwise */
    BMXT_ParserTimestampMode_eMod300 = 2,     /* Use Mod-300 timestamp */
    BMXT_ParserTimestampMode_eBinary = 3,     /* Use binary timestamp */
    BMXT_ParserTimestampMode_eMax
} BMXT_ParserTimestampMode;

/***************************************************************************
Summary:
    Config for the MTSIF TX interface.
****************************************************************************/
typedef struct BMXT_MtsifTxConfig
{
    unsigned TxInterfaceWidth; /* Width of the MTSIF bus in bits.
                                  Allowable values are 1, 2, 4, and 8 */

    unsigned TxClockPolarity; /* 1 = rising edge, 0 = falling edge */
    bool Encrypt; /* enable encryption */
    bool Enable;
} BMXT_MtsifTxConfig;

/***************************************************************************
Summary:
    Config for the MTSIF RX interface.
****************************************************************************/
typedef struct BMXT_MtsifRxConfig
{
    unsigned RxInterfaceWidth; /* Width of the MTSIF bus in bits.
                                  Allowable values are 1, 2, 4, and 8 */

    unsigned RxClockPolarity; /* 1 = rising edge, 0 = falling edge */

    bool Decrypt; /* enable decryption */
    bool Enable;
} BMXT_MtsifRxConfig;

/***************************************************************************
Summary:
    MTSIF TX/RX status
****************************************************************************/
typedef struct BMXT_MtsifStatus
{
    struct {
        unsigned interfaceWidth;
        unsigned clockPolarity;
        bool enabled;
    } tx[BMXT_MAX_NUM_MTSIF_TX], rx[BMXT_MAX_NUM_MTSIF_RX];
} BMXT_MtsifStatus;

/***************************************************************************
Summary:
    Input band status
****************************************************************************/
typedef struct BMXT_InputBandStatus
{
    unsigned syncCount[BMXT_MAX_NUM_INPUT_BANDS];
} BMXT_InputBandStatus;

/***************************************************************************
Summary:
    Settings for a parser band.
****************************************************************************/
typedef struct BMXT_ParserConfig
{
    bool ErrorInputIgnore;  /* PID parser ignores the error input signal and TEI bit */
    BMXT_ParserTimestampMode TsMode; /* Controls the type of timestamp generated as packets pass through the parser band */
    bool AcceptNulls;       /* NULL packets are not discarded if true */
    bool AllPass;           /* Parser is in all-pass mode */
    bool DssMode;           /* Parser is in DSS packet mode */
    unsigned InputBandNumber; /* Which input band the data comes from. Defaults to 1-to-1 mapping */
    bool Enable;            /* Parser on or off */
    unsigned mtsifTxSelect;    /* which MTSIF TX */
    unsigned virtualParserNum; /* remapped BAND_ID */

    bool streamIdFilterEn; /* Enable stream ID filtering. Only supported in conjunction with BMXT_InputBandConfig.streamIdExtractEn */
    unsigned streamId;
} BMXT_ParserConfig;

/***************************************************************************
Summary:
    Settings for a input band.
****************************************************************************/
typedef struct BMXT_InputBandConfig
{
    bool DssMode; /* Input band processes DSS packets */
    bool ParallelInputSel; /* Select parallel data input for this port instead of serial. Not available on all input bands */
    bool streamIdExtractEn; /* Enable sampling of stream ID field. Only supported in conjunction with ParallelInputSel = true */
} BMXT_InputBandConfig;

/***************************************************************************
Summary:
    DEPRECATED. Use BMXT_ParserConfig.virtualParserNum instead
****************************************************************************/
typedef struct BMXT_BandMap
{
    unsigned VirtualParserBandNum;
    bool VirtualParserIsPlayback;
} BMXT_BandMap;

/***************************************************************************
Summary:
    DEPRECATED. Use BMXT_ParserConfig.virtualParserNum instead
****************************************************************************/
typedef struct BMXT_ParserBandMapping
{
    /* The indexes into these arrays represent physical parser bands */
    BMXT_BandMap FrontEnd[BMXT_MAX_NUM_PARSERS];
} BMXT_ParserBandMapping;

/***************************************************************************
Summary:
    TSMF Inputs selections.
Description:
    This enum represents the input selections that a TSMF module could take
***************************************************************************/
typedef enum
{
    BMXT_TSMFInputSel_eIB0 = 0,
    BMXT_TSMFInputSel_eIB1,
    BMXT_TSMFInputSel_eIB2,
    BMXT_TSMFInputSel_eIB3,
    BMXT_TSMFInputSel_eIB4,
    BMXT_TSMFInputSel_eIB5,
    BMXT_TSMFInputSel_eIB6,
    BMXT_TSMFInputSel_eIB7,
    BMXT_TSMFInputSel_eIB8,
    BMXT_TSMFInputSel_eIB9,
    BMXT_TSMFInputSel_ePB0,
    BMXT_TSMFInputSel_eRMX = 0x10
} BMXT_TSMFInputSel;

/***************************************************************************
Summary:
    Interrupt status registers that can be provided as arguments to
    BMXT_ReadIntStatusRegister
****************************************************************************/
typedef enum
{
    BMXT_IntReg_eFE_INTR_STATUS0 = 0, /* DEMOD_XPT_FE_INTR_STATUS0_REG */
    BMXT_IntReg_eFE_INTR_STATUS1      /* DEMOD_XPT_FE_INTR_STATUS1_REG */
} BMXT_IntReg;

/***************************************************************************
Summary:
    Default settings and/or actions to used during BMXT_Open().
****************************************************************************/
typedef struct BMXT_Settings
{
    BMXT_MtsifTxConfig MtsifTxCfg[BMXT_MAX_NUM_MTSIF_TX];
    BMXT_MtsifRxConfig MtsifRxCfg[BMXT_MAX_NUM_MTSIF_RX];

    void *hHab; /* HAB handle, used to communicate to mini-transport core. Set to NULL if not used */
    void *hRpc; /* RPC (RMagnum) handle, used to communicate to mini-transport core. Set to NULL if not used */
    BMXT_Handle hVirtual; /* if this is a slave MTSIF device, then set this to the BMXT_Handle of the master device */

    BMXT_Chip chip; /* which chip to target. automatically populated via BMXT_XXXX_GetDefaultSettings */
    BMXT_ChipRev chipRev; /* which chip revision to target. defaults to A0 */
} BMXT_Settings;

typedef BMXT_Settings BMXT_DefaultSettings; /* for backward-compatibility */

/***************************************************************************
Summary:
    Opens the transport module.

Description:
    The transport core is reset and initialized. Dynamic resources needed
    by the transport module are allocated. A handle to use when configuring
    the core is created and returned. The caller may specify certain default
    and/or initialization actions by setting the appropriate members of the
    default settings parameter.

See Also:
    BMXT_GetDefaultSettings, BMXT_Close
****************************************************************************/
BERR_Code BMXT_Open(
    BMXT_Handle *pHandle,          /* [out] */
    BCHP_Handle hChp,              /* [in] */
    BREG_Handle hReg,              /* [in] */
    const BMXT_Settings *pSettings /* [in] */
    );

/***************************************************************************
Summary:
    Close the transport module.

Description:
    Release any memory and other system resources allocated by the data
    transport core. Reset the transport core to insure that all interrupt
    sources are disabled.

See Also:
    BMXT_Open
****************************************************************************/
void BMXT_Close(
    BMXT_Handle handle /* [in] */
    );

/***************************************************************************
Summary:
    Retrieves current MTSIF TX/RX status
****************************************************************************/
BERR_Code BMXT_GetMtsifStatus(
    BMXT_Handle handle, /* [in] */
    BMXT_MtsifStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
    Retrieves current input band status
****************************************************************************/
BERR_Code BMXT_GetInputBandStatus(
    BMXT_Handle handle, /* [in] */
    BMXT_InputBandStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
    Retrieves the current parser band configuration.

Description:
    Read the current settings for the given parser band from the chip.
    Settings are returned in the structure that is passed in. See the
    structure for the complete list of supported settings.

See Also:
    BMXT_SetParserConfig
****************************************************************************/
BERR_Code BMXT_GetParserConfig(
    BMXT_Handle handle, /* [in] */
    unsigned parserNum, /* [in] which parser band */
    BMXT_ParserConfig *pConfig /* [out] current settings */
    );

/***************************************************************************
Summary:
    Sets the current parser band configuration.

Description:
    Writes the given parser band configuration to the device. The caller
    supplies a structure containing the new parser band settings. The new
    settings take effect as soon as the function returns.

See Also:
    BMXT_GetParserConfig
****************************************************************************/
BERR_Code BMXT_SetParserConfig(
    BMXT_Handle handle, /* [in] */
    unsigned parserNum, /* [in] which parser band */
    const BMXT_ParserConfig *pConfig /* [in] new settings */
    );

/***************************************************************************
Summary:
    DEPRECATED. Use BMXT_ParserConfig.virtualParserNum instead
****************************************************************************/
BERR_Code BMXT_GetParserMapping(BMXT_Handle handle, BMXT_ParserBandMapping *pMapping);

/***************************************************************************
Summary:
    DEPRECATED. Use BMXT_ParserConfig.virtualParserNum instead
****************************************************************************/
BERR_Code BMXT_SetParserMapping(BMXT_Handle handle, const BMXT_ParserBandMapping *pMapping);

/***************************************************************************
Summary:
    Retrieves the current input band configuration.

Description:
    Read the current settings for the given input band from the chip.
    Settings are returned in the structure that is passed in. See the
    structure for the complete list of supported settings.

See Also:
    BMXT_SetInputBandConfig
****************************************************************************/
BERR_Code BMXT_GetInputBandConfig(
    BMXT_Handle handle, /* [in] */
    unsigned bandNum,   /* [in] which input band */
    BMXT_InputBandConfig *pConfig /* [out] current settings */
    );

/***************************************************************************
Summary:
    Sets the current input band configuration.

Description:
    Writes the given input band configuration to the device. The caller
    supplies a structure containing the new input band settings. The new
    settings take effect as soon as the function returns.

See Also:
    BMXT_GetInputBandConfig
****************************************************************************/
BERR_Code BMXT_SetInputBandConfig(
    BMXT_Handle handle, /* [in] */
    unsigned bandNum,   /* [in] which input band */
    const BMXT_InputBandConfig *pConfig /* [in] new settings */
    );

/***************************************************************************
Summary:
    Get default MXT settings.
    BMXT_Settings.chip must be specified correctly for proper functionality
****************************************************************************/
BERR_Code BMXT_GetDefaultSettings(
    BMXT_Settings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    DEPRECATED. call BMXT_GetDefaultSettings and set BMXT_Settings.chip instead
****************************************************************************/
void BMXT_3128_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_3383_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_3472_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_4517_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_4528_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_4538_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_4548_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_45216_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_3384_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_7366_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_7145_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_7364_GetDefaultSettings(BMXT_Settings *pSettings);
void BMXT_45308_GetDefaultSettings(BMXT_Settings *pSettings);

/***************************************************************************
Summary:
    Reads and clears an interrupt status register

Description:
    Reads the specified interrupt status register and returns the 32-bit
    value that was read. The same register is then cleared by writing 0 to it.
****************************************************************************/
uint32_t BMXT_ReadIntStatusRegister(
    BMXT_Handle handle, /* [in] */
    BMXT_IntReg intReg  /* [in] */
    );

/***************************************************************************
TSMF APIs

TSMF APIs provide configuration of transport TSMF (Transport Stream
Multiplex Frame)-related modules.

TSMF, a frame structure of 53 slots of 188 byte MPEG packets is parsed and
certain streams are accepted based on slot assignments configured by host
or extracted by hardware.  In the automatic mode, the host programs a 4 bit
relative_ts_number, the hardware will generate the slot assignment table
automatically once per frame.  In semi automatic mode, host will need to program
the slot assignment registers.  TSMF hardware also verifies certain fields(TEI,
PUSI,CC,ADAP,SC,...) and host can disable the verification for each field.

Example:
    int tsmfnum = 0;
    BMXT_TSMFFldVerifyConfig tsmfConfig;
    BMXT_TSMFInputSel inputSel = BMXT_TSMFInputSel_eIB0;
    uint32_t slotMapLo = 0xABCD1111;
    uint32_t slotMapHi = 0x22222222;

    // Disable CC check and TEI check
    BMXT_TSMF_GetFldVerifyConfig(mxt, tsmfnum, &tsmfConfig);
    tsmfConfig.CCCkDis = 1;
    tsmfConfig.TeiChkDis = 1;
    BMXT_TSMF_SetFldVerifyConfig(mxt, tsmfnum, &tsmfConfig);

    // Set semi auto mode
    BMXT_TSMF_EnableSemiAutoMode(mxt, tsmfnum, inputSel, slotMapHi, slotMapLo, 1);

    // Set parserband2 TSMF 0 enable
    BMXT_TSMF_SetParserConfig(mxt, 2, 0, 1);
    // Set PCR1 TSMF0 enable
    BMXT_TSMF_SetPcrConfig(mxt, 1, 0, 1);

****************************************************************************/

/***************************************************************************
Summary:
    Modes supported for accepting packets.
Description:
    This enum represents the modes that TSMF supports when there is a change for
    the version number field inside a frame header
***************************************************************************/
typedef enum
{
    BMXT_TSMFVersionChgMode_eAllFrame = 0, /* accept all Frame Header packets */
    BMXT_TSMFVersionChgMode_eFrameChgVer   /* accept only Frame Header packets that
                                              have a change in version number field */
} BMXT_TSMFVersionChgMode;

/***************************************************************************
Summary:
    Structure for field verification configuration of a TSMF module
***************************************************************************/
typedef struct BMXT_TSMFFldVerifyConfig
{
    bool CrcChkDis;         /* TSMF Frame Header CRC Check Disable */
    bool RelTsStatusChkDis; /* TSMF Frame Header Rel_TS_Status Check Disable */
    bool FrameTypeChkDis;   /* TSMF Frame Header Frame_Type Check Disable */
    bool RelTsModeChkDis;   /* TSMF Frame Header REL_TS_MODE Check Disable */
    bool SyncChkFDis;       /* TSMF Frame Header SYNC Check Disable */
    bool CCCkDis;           /* TSMF Frame Header CC Check Disable */
    bool AdapChkDis;        /* TSMF Frame Header ADAP Check Disable */
    bool ScChkDis;          /* TSMF Frame Header SC Check Disable */
    bool TsPriorChkDis;     /* TSMF Frame Header TS_PRIOR Check Disable */
    bool PusiChkDis;        /* TSMF Frame Header PUSI Check Disable */
    bool TeiChkDis;         /* TSMF Frame Header TEI Check Disable */
    BMXT_TSMFVersionChgMode VersionChgMode; /* TSMF Version Change Mode */
} BMXT_TSMFFldVerifyConfig;

/***************************************************************************
Summary:
    Get field verification configuration of TSMF module
***************************************************************************/
BERR_Code BMXT_TSMF_GetFldVerifyConfig(
    BMXT_Handle handle, /* [in] */
    unsigned tsmfNum,   /* [in] which TSMF */
    BMXT_TSMFFldVerifyConfig *pConfig /* [in] TSMF config */
    );

/***************************************************************************
Summary:
    Set field verification configuration of TSMF module
***************************************************************************/
BERR_Code BMXT_TSMF_SetFldVerifyConfig(
    BMXT_Handle handle, /* [in] */
    unsigned tsmfNum,   /* [in] which TSMF */
    const BMXT_TSMFFldVerifyConfig *pConfig /* [in] TSMF config */
    );

/***************************************************************************
Summary:
    Enable/Disable automatic mode TSMF
***************************************************************************/
BERR_Code BMXT_TSMF_EnableAutoMode(
    BMXT_Handle handle,   /* [in] */
    BMXT_TSMFInputSel input, /* [in] TSMF parser input select */
    unsigned tsmfNum,     /* [in] which TSMF */
    uint8_t relativeTsNum /* [in] TSMF relative TS number */
    );
/***************************************************************************
Summary:
    Enable/Disable semi-automatic mode TSMF
***************************************************************************/
BERR_Code BMXT_TSMF_EnableSemiAutoMode(
    BMXT_Handle handle,   /* [in] */
    BMXT_TSMFInputSel input, /* TSMF parser input select */
    unsigned tsmfNum,     /* [in] which TSMF */
    uint32_t slotMapLo,   /* [in] lower 32 bits (LSBs) of the 52-bit TSMF slot map vector */
    uint32_t slotMapHi,   /* [in] upper 21 bits of the 53-bit slot map vector for TSMF demultiple */
    uint8_t relativeTsNum /* [in] TSMF relative TS number */
    );

/***************************************************************************
Summary:
    Disable TSMF module
***************************************************************************/
BERR_Code BMXT_TSMF_DisableTsmf(
    BMXT_Handle handle, /* [in] */
    unsigned tsmfNum    /* [in] which TSMF */
    );

/***************************************************************************
Summary:
    Enable/Disable TSMF control for a given parser band
Description:
    This function sets TSMF related configurations for a given parser band. It
    selects a TSMF module and disable/enables TSMF parsing for the parser band
    module.
***************************************************************************/
BERR_Code BMXT_TSMF_SetParserConfig(
    BMXT_Handle handle, /* [in] */
    unsigned parserNum, /* [in] which parser */
    unsigned tsmfNum,   /* [in] which TSMF */
    bool enable         /* [in] enable/disable the selected TSMF to the parser */
    );

/*******************************************************************************
Summary:
    Per-parser TBG config
*******************************************************************************/
typedef struct BMXT_Tbg_ParserConfig
{
    bool latchSyncCount;     /* latch SYNC_COUNT into timestamp fields in xpacket */
    unsigned primaryBandNum; /* primary band number */
    bool enable;
} BMXT_Tbg_ParserConfig;

BERR_Code BMXT_Tbg_GetParserConfig(
    BMXT_Handle handle,
    unsigned parserNum,
    BMXT_Tbg_ParserConfig *pConfig
);

BERR_Code BMXT_Tbg_SetParserConfig(
    BMXT_Handle handle,
    unsigned parserNum,
    const BMXT_Tbg_ParserConfig *pConfig
);

/*******************************************************************************
Summary:
    Global TBG config
*******************************************************************************/
typedef struct BMXT_Tbg_GlobalConfig
{
    unsigned markerTag;
    unsigned markerPidValue;
    unsigned markerFeedId;
} BMXT_Tbg_GlobalConfig;

BERR_Code BMXT_Tbg_GetGlobalConfig(
    BMXT_Handle handle,
    BMXT_Tbg_GlobalConfig *pConfig
);

BERR_Code BMXT_Tbg_SetGlobalConfig(
    BMXT_Handle handle,
    const BMXT_Tbg_GlobalConfig *pConfig
);

/*******************************************************************************
Summary:
    Power saver settings
*******************************************************************************/
typedef struct BMXT_PowerSaverSettings {
    bool disableMtsif;
} BMXT_PowerSaverSettings;

/*******************************************************************************
Summary:
    Get power saver settings
*******************************************************************************/
void BMXT_GetPowerSaverSettings(
    BMXT_Handle handle,
    BMXT_PowerSaverSettings *pSettings /* [out] */
    );

/*******************************************************************************
Summary:
    Set power saver settings
*******************************************************************************/
BERR_Code BMXT_SetPowerSaverSettings(
    BMXT_Handle handle,
    const BMXT_PowerSaverSettings *pSettings
    );



#ifdef __cplusplus
}
#endif

#endif /* #ifndef BMXT_H__ */

