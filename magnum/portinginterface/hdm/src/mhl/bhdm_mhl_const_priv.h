/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
#ifndef BHDM_MHL_CONST_PRIV_H__
#define BHDM_MHL_CONST_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MHL Constants
 */
/* CBUS spec requires sleeping for at least 80ms between
   impedance change detection and impedance
   measurement. We give it a bit more time */
#define BHDM_P_MHL_CBUS_T_SRC_START_IMP_MEAS  100

/* These probably should go somewhere else
  as they are basically the same as the EDDC spec */
#define BHDM_P_MHL_DDC_EDID_ADDR              0xA0
#define BHDM_P_MHL_DDC_SEGPTR_ADDR            0x60
#define BHDM_P_MHL_I2C_HDCP                   0x74
#define BHDM_P_MHL_I2C_HDCP2                  0x76

/* CBUS Link Layer -------------------------------------------------------------- */

/* CBUS Translation Layer ------------------------------------------------------- */
typedef enum
{
	/** MSC/DDC */
	BHDM_P_Mhl_Command_eEof                 = 0x32,
	BHDM_P_Mhl_Command_eAck                 = 0x33,
	BHDM_P_Mhl_Command_eNack                = 0x34,
	BHDM_P_Mhl_Command_eAbort               = 0x35,

	/** MSC **/
	BHDM_P_Mhl_Command_eWriteStat           = 0x60,
	BHDM_P_Mhl_Command_eSetInt              = 0x60,
	BHDM_P_Mhl_Command_eReadDevCap          = 0x61,
	BHDM_P_Mhl_Command_eGetState            = 0x62,
	BHDM_P_Mhl_Command_eGetVendorId         = 0x63,
	BHDM_P_Mhl_Command_eSetHpd              = 0x64,
	BHDM_P_Mhl_Command_eClrHpd              = 0x65,
	BHDM_P_Mhl_Command_eMscMsg              = 0x68,
	BHDM_P_Mhl_Command_eGetSc1ErrorCode     = 0x69,
	BHDM_P_Mhl_Command_eGetDdcErrorCode     = 0x6A,
	BHDM_P_Mhl_Command_eGetMscErrorCode     = 0x6B,
	BHDM_P_Mhl_Command_eWriteBurst          = 0x6C,
	BHDM_P_Mhl_Command_eGetSc3ErrorCode     = 0x6D,

	/** DDC **/
	BHDM_P_Mhl_Command_eSof                 = 0x30,
	BHDM_P_Mhl_Command_eCont                = 0x50,
	BHDM_P_Mhl_Command_eStop                = 0x51
} BHDM_P_Mhl_Command;

/* Device Register Addresses ---------------------------------------------------- */

/* Device Capability Registers from address 0x0 to 0xF */
typedef enum {
	BHDM_P_Mhl_DevCapOffset_eDevStateAddr          = 0x0,
	BHDM_P_Mhl_DevCapOffset_eMhlVersionAddr        = 0x1,
	BHDM_P_Mhl_DevCapOffset_eDevCatAddr            = 0x2,
	BHDM_P_Mhl_DevCapOffset_eAdopterIdHiAddr       = 0x3,
	BHDM_P_Mhl_DevCapOffset_eAdopterIdLoAddr       = 0x4,
	BHDM_P_Mhl_DevCapOffset_eVidLinkModeAddr       = 0x5,
	BHDM_P_Mhl_DevCapOffset_eAudLinkModeAddr       = 0x6,
	BHDM_P_Mhl_DevCapOffset_eVideoTypeAddr         = 0x7,
	BHDM_P_Mhl_DevCapOffset_eLogDevMapAddr         = 0x8,
	BHDM_P_Mhl_DevCapOffset_eBandwidthAddr         = 0x9,
	BHDM_P_Mhl_DevCapOffset_eFeatureFlagAddr       = 0xA,
	BHDM_P_Mhl_DevCapOffset_eDeviceIdHiAddr        = 0xB,
	BHDM_P_Mhl_DevCapOffset_eDeviceIdLoAddr        = 0xC,
	BHDM_P_Mhl_DevCapOffset_eScratchpadSizeAddr    = 0xD,
	BHDM_P_Mhl_DevCapOffset_eIntStatSizeAddr       = 0xE,
	BHDM_P_Mhl_DevCapOffset_eDevCapAddrMax
} BHDM_P_Mhl_DevCapOffset;

#define BHDM_P_MHL_DEVCAP_ADDR_MIN BHDM_P_Mhl_DevCapOffset_eDevStateAddr

typedef enum {
	BHDM_P_Mhl_DevCat_eReserved      = 0,
	BHDM_P_Mhl_DevCat_eSink          = 1,
	BHDM_P_Mhl_DevCat_eSource        = 2,
	BHDM_P_Mhl_DevCat_eDongle        = 3,
	BHDM_P_Mhl_DevCat_eMax
} BHDM_P_Mhl_DevCat;

/* defines the bitfields of BHDM_P_Mhl_DevCapOffset_eVidLinkModeAddr */
#define BHDM_P_MHL_VIDEO_LINK_MODE_RGB444         0x01
#define BHDM_P_MHL_VIDEO_LINK_MODE_YCBCR444       0x02
#define BHDM_P_MHL_VIDEO_LINK_MODE_YCBCR422       0x04
#define BHDM_P_MHL_VIDEO_LINK_MODE_PACKED_PIXEL   0x08
#define BHDM_P_MHL_VIDEO_LINK_MODE_DATA_ISLANDS   0x10
#define BHDM_P_MHL_VIDEO_LINK_MODE_VGA            0x20
#define BHDM_P_MHL_VIDEO_LINK_MODE_RESERVED1      0x40
#define BHDM_P_MHL_VIDEO_LINK_MODE_RESERVED2      0x80

typedef enum {
	BHDM_P_Mhl_PowerLimit_e500       = 0,
	BHDM_P_Mhl_PowerLimit_e900       = 1,
	BHDM_P_Mhl_PowerLimit_e1500      = 2,
	BHDM_P_Mhl_PowerLimit_eMax
} BHDM_P_Mhl_PowerLimit;

#define BHDM_P_MHL_DEV_CAT_MASK       (0xF << 0)
#define BHDM_P_MHL_DEV_CAT_LSB         0
#define BHDM_P_MHL_POW_OUT_MASK       (0x1 << 4)
#define BHDM_P_MHL_POW_OUT_LSB         4
#define BHDM_P_MHL_POW_LIMIT_MASK     (0x7 << 5)
#define BHDM_P_MHL_POW_LIMIT_LSB       5

/* Power capability constant */
#define BHDM_P_MHL_POW_CAP_MIN        3
#define BHDM_P_MHL_POW_CAP_500        0
#define BHDM_P_MHL_POW_CAP_900        1
#define BHDM_P_MHL_POW_CAP_1500       2

/* Device Interrupt Registers from address 0x20 to 0x2F */
typedef enum {
	BHDM_P_Mhl_IntAddr_eRchangeIntAddr     = 0x20,
	BHDM_P_Mhl_IntAddr_eDchangeIntAddr     = 0x21,
	BHDM_P_Mhl_IntAddr_eRsvrInt1Addr       = 0x22,
	BHDM_P_Mhl_IntAddr_eRsvrInt2Addr       = 0x23
} BHDM_P_Mhl_IntAddr;

/* Device Status Registers from address 0x30 to 0x3F */
typedef enum {
	BHDM_P_Mhl_StatusAddr_eConnectedRdyAddr = 0x30,
	BHDM_P_Mhl_StatusAddr_eLinkModeAddr     = 0x31,
	BHDM_P_Mhl_StatusAddr_eRsvrStat1Addr    = 0x32,
	BHDM_P_Mhl_StatusAddr_eRsvrStat2Addr    = 0x33
} BHDM_P_Mhl_StatusAddr;

typedef enum
{
	BHDM_P_Mhl_CtrlMode_eHdmi           = 0,
	BHDM_P_Mhl_CtrlMode_eMhl24bpp       = 2,
	BHDM_P_Mhl_CtrlMode_eMhlPackedPixel = 3
} BHDM_P_Mhl_CtrlMode;

typedef enum
{
	BHDM_P_Mhl_ClkMode_eHdmi         = 0x0,
	BHDM_P_Mhl_ClkMode_ePacked       = 0x2,
	BHDM_P_Mhl_ClkMode_e24bit        = 0x3
} BHDM_P_Mhl_ClkMode;

typedef enum
{
	BHDM_P_Mhl_PathEn_e0             = 0,
	BHDM_P_Mhl_PathEn_e1             = 1
} BHDM_P_Mhl_PathEn;

typedef enum
{
	BHDM_P_Mhl_MuteState_eUnmuted    = 0,
	BHDM_P_Mhl_MuteState_eMuted      = 1
} BHDM_P_Mhl_MuteState;

#define BHDM_P_MHL_LINK_MODE_CLK_MODE_SHIFT       0
#define BHDM_P_MHL_LINK_MODE_PATH_EN_SHIFT        3
#define BHDM_P_MHL_LINK_MODE_MUTED_SHIFT          4

/* Interrupt register bit fields */
#define BHDM_P_MHL_INT_RCHANGE_DCAP_CHG          (1 << 0)
#define BHDM_P_MHL_INT_RCHANGE_DSCR_CHG          (1 << 1)
#define BHDM_P_MHL_INT_RCHANGE_REQ_WRT           (1 << 2)
#define BHDM_P_MHL_INT_RCHANGE_GRT_WRT           (1 << 3)
#define BHDM_P_MHL_INT_RCHANGE_3D_REQ            (1 << 4)

#define BHDM_P_MHL_INT_DCHANGE_EDID_CHG          (1 << 1)

/* Status register bit fields */
#define BHDM_P_MHL_STAT_CONNECTED_DCAP_RDY       (1 << 0)
#define BHDM_P_MHL_STAT_LINK_MODE_CLK_MODE_MASK  (0x7)
#define BHDM_P_MHL_STAT_LINK_MODE_PATH_EN        (1 << 3)
#define BHDM_P_MHL_STAT_LINK_MODE_MUTED          (1 << 4)

#define BHDM_P_MHL_INT_ADDR_MIN MHL_RCHANGE_INT_ADDR
#define BHDM_P_MHL_INT_ADDR_MAX MHL_DCHANGE_INT_ADDR

#define BHDM_P_MHL_STAT_ADDR_MIN MHL_CONNECTED_RDY_ADDR
#define BHDM_P_MHL_STAT_ADDR_MAX MHL_LINK_MODE_ADDR

#define BHDM_P_MHL_SCRATCH_ADDR_MIN 0x40
#define BHDM_P_MHL_SCRATCH_ADDR_MAX 0x7F

#define BHDM_P_MHL_EDID_BLOCK_SIZE      (128)
#define BHDM_P_MHL_EDID_SIZE_OFFSET     (0x7e)

/* CBUS register space */
/* We have:
   16 bytes of capability register,
   4 bytes of interrupt register,
   4 bytes of status register,
   16 bytes of scratchpad registers
*/
#define BHDM_P_MHL_CBUS_CAP_REG_SIZE    (16)
#define BHDM_P_MHL_CBUS_INT_REG_SIZE    (4) /* Only the first 2 registers are defined at the moment */
#define BHDM_P_MHL_CBUS_STAT_REG_SIZE   (4)
/* Scratch register size is defined in the mailbox */
#define BHDM_P_MHL_CBUS_MAX_MSG_SIZE    (16)

/* Device Capabilities */
#define BHDM_P_MHL_SINK_DCAP_SIZE (16)


/* Mailbox defines */
/* Mailbox revision */
#ifndef BHDM_P_MHL_MAILBOX_REVISION
#define BHDM_P_MHL_MAILBOX_REVISION 0x10
#endif

/* Minimum scratchpad size is 16 bytes */
#define BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE (16)

#ifndef BHDM_P_MHL_MAILBOX_EDID_SIZE
#define BHDM_P_MHL_MAILBOX_EDID_SIZE (512)
#endif

typedef enum
{
	BHDM_P_Mhl_MscMsgCommand_eRcp      = 0x10,
	BHDM_P_Mhl_MscMsgCommand_eRcpk     = 0x11,
	BHDM_P_Mhl_MscMsgCommand_eRcpe     = 0x12,
	BHDM_P_Mhl_MscMsgCommand_eRap      = 0x20,
	BHDM_P_Mhl_MscMsgCommand_eRapk     = 0x21,
	BHDM_P_Mhl_MscMsgCommand_eRape     = 0x22
} BHDM_P_Mhl_MscMsgCommand;

typedef enum
{
	BHDM_P_Mhl_RapAction_ePoll         = 0x00,
	BHDM_P_Mhl_RapAction_eContentOn    = 0x10,
	BHDM_P_Mhl_RapAction_eContentOff   = 0x11
} BHDM_P_Mhl_RapAction;

typedef enum
{
	BHDM_P_Mhl_RapError_eNone          = 0x00,
	BHDM_P_Mhl_RapError_eUnrecognised  = 0x01,
	BHDM_P_Mhl_RapError_eUnsupported   = 0x02,
	BHDM_P_Mhl_RapError_eBusy          = 0x03
} BHDM_P_Mhl_RapError;

typedef enum
{
	BHDM_P_Mhl_RcpError_eNone          = 0x00,
	BHDM_P_Mhl_RcpError_eIneffective   = 0x01,
	BHDM_P_Mhl_RcpError_eErrorBusy     = 0x02
} BHDM_P_Mhl_RcpError;

/* BURST_ID (for logging) */

#define BHDM_P_MHL_BURST_ID_3D_VIC       0x0010
#define BHDM_P_MHL_BURST_ID_3D_DTD       0x0011

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_CONST_PRIV_H__ */
