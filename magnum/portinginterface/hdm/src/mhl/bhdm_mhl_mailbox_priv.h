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

#ifndef BHDM_MHL_MAILBOX_PRIV_H__
#define BHDM_MHL_MAILBOX_PRIV_H__

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"   	/* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */

#include "bhdm_mhl_const_priv.h"
#include "bhdm_mhl_common_priv.h"

#include "bchp_mpm_cpu_data_mem.h"

/* The mailbox structure.
   We just define this as an array of
   8-bit values to make it portable across MPM
   and host CPU.
   Each field will be defined with the following:
   Offset byte, mask, lsb, with the exception of
   EDID, which will start at particular offset
   continue for CBUS_MAILBOX_EDID_SIZE bytes
 */


/* All the offsets in the mailbox are represented by the enums below
   If you need to change the ordering of the fields,
   change the enums */

typedef enum {
	/* Mailbox revision */
	BHDM_P_Mhl_MailboxField_eRev                  = 0,
	/* MPM processor state */
	BHDM_P_Mhl_MailboxField_eMpmState             = 1,
	/* Link state: HPD, DCAP valid, request pending in each channel, etc. */
	BHDM_P_Mhl_MailboxField_eLinkState            = 2,
	/* MSC/DDC source error codes */
	BHDM_P_Mhl_MailboxField_eSrcMscErrorCode      = 3,
	BHDM_P_Mhl_MailboxField_eSrcDdcErrorCode      = 4,
	/* MSC/DDC sink error codes */
	BHDM_P_Mhl_MailboxField_eSinkMscErrorCode     = 5,
	BHDM_P_Mhl_MailboxField_eSinkDdcErrorCode     = 6,
	/* Source stat (to be sent to sink) */
	BHDM_P_Mhl_MailboxField_eSrcConnectedRdy      = 7,
	BHDM_P_Mhl_MailboxField_eSrcLinkMode1         = 8,
	/* Sink stat (received from sink) */
	BHDM_P_Mhl_MailboxField_eSinkConnectedRdy     = 9,
	BHDM_P_Mhl_MailboxField_eSinkLinkMode         = 10,
	/* Source interrupt is not included */
	/* Interrupts received from sink */
	BHDM_P_Mhl_MailboxField_eSinkRchangeInt       = 11,
	BHDM_P_Mhl_MailboxField_eSinkDchangeInt       = 12,

	/* Source DCAP 16 bytes */
	BHDM_P_Mhl_MailboxField_eSrcDcapDevState, /* 13 */
	BHDM_P_Mhl_MailboxField_SrcDcapMhlVer         = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 1,
	BHDM_P_Mhl_MailboxField_eSrcDcapDevCat        = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 2,
	BHDM_P_Mhl_MailboxField_eSrcDcapAdtIdHi       = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 3,
	BHDM_P_Mhl_MailboxField_eSrcDcapAdtIdLo       = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 4,
	BHDM_P_Mhl_MailboxField_eSrcDcapVlinkMode     = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 5,
	BHDM_P_Mhl_MailboxField_eSrcDcapAlinkMode     = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 6,
	BHDM_P_Mhl_MailboxField_eSrcDcapVtype         = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 7,
	BHDM_P_Mhl_MailboxField_eSrcDcapLogDevMap     = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 8,
	BHDM_P_Mhl_MailboxField_eSrcDcapBandwidth     = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 9,
	BHDM_P_Mhl_MailboxField_eSrcDcapFeature       = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 10,
	BHDM_P_Mhl_MailboxField_eSrcDcapDevIdHi       = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 11,
	BHDM_P_Mhl_MailboxField_eSrcDcapDevIdLo       = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 12,
	BHDM_P_Mhl_MailboxField_eSrcDcapScratchSize   = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 13,
	BHDM_P_Mhl_MailboxField_eSrcDcapIntStatSize   = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 14,
	BHDM_P_Mhl_MailboxField_eSrcDcapRsvr          = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + 15, /* 28 */

	/* Sink DCAP 16 bytes */
	BHDM_P_Mhl_MailboxField_eSinkDcapDevState     = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE,
	BHDM_P_Mhl_MailboxField_eSinkDcapMhlVer       = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 1,
	BHDM_P_Mhl_MailboxField_eSinkDcapDevCat       = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 2,
	BHDM_P_Mhl_MailboxField_eSinkDcapAdtIdHi      = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 3,
	BHDM_P_Mhl_MailboxField_eSinkDcapAdtIdLo      = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 4,
	BHDM_P_Mhl_MailboxField_eSinkDcapVlinkMode    = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 5,
	BHDM_P_Mhl_MailboxField_eSinkDcapAlinkMode    = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 6,
	BHDM_P_Mhl_MailboxField_eSinkDcapVtype        = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 7,
	BHDM_P_Mhl_MailboxField_eSinkDcapLogDevMap    = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 8,
	BHDM_P_Mhl_MailboxField_eSinkDcapBandwidth    = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 9,
	BHDM_P_Mhl_MailboxField_eSinkDcapFeature      = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 10,
	BHDM_P_Mhl_MailboxField_eSinkDcapDevIdHi      = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 11,
	BHDM_P_Mhl_MailboxField_eSinkDcapDevIdLo      = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 12,
	BHDM_P_Mhl_MailboxField_eSinkDcapScratchSize  = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 13,
	BHDM_P_Mhl_MailboxField_eSinkDcapIntStatSize  = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 14,
	BHDM_P_Mhl_MailboxField_eSinkDcapRsvr         = BHDM_P_Mhl_MailboxField_eSrcDcapDevState + \
                                                     BHDM_P_MHL_CBUS_CAP_REG_SIZE + 15, /* 43 */

	/* Scratchpad register 16 bytes */
	BHDM_P_Mhl_MailboxField_eScratchpad           = BHDM_P_Mhl_MailboxField_eSinkDcapDevState + \
													 BHDM_P_MHL_CBUS_CAP_REG_SIZE, /* 45 */
	/* EDID state */
	BHDM_P_Mhl_MailboxField_eEdidState            = BHDM_P_Mhl_MailboxField_eScratchpad + \
													 BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE, /* 61 */
	/* EDID itself 512 bytes */
	BHDM_P_Mhl_MailboxField_eEdid, /* 76 */

	/* Host fields - must start on word boundry */
	/* Host state */
	BHDM_P_Mhl_MailboxField_eHostState            = (BHDM_P_Mhl_MailboxField_eEdid + \
													  BHDM_P_MHL_MAILBOX_EDID_SIZE + 3) & ~0x3, /* 576 */

	/* Source link mode updated by host */
	BHDM_P_Mhl_MailboxField_eSrcLinkMode0, /* 577 */

	/* End */
	BHDM_P_Mhl_MailboxField_eEnd
} BHDM_P_Mhl_MailboxField;

/* Total mailbox size, which should be supplied by the compiler
*/
#ifndef BHDM_P_MHL_MAILBOX_SIZE
#define BHDM_P_MHL_MAILBOX_SIZE                 BHDM_P_Mhl_MailboxField_eEnd
#endif


/* Byte 0: REV (8 bits) */
#define BHDM_P_MHL_MAILBOX_REV_OFFSET           BHDM_P_Mhl_MailboxField_eRev
#define BHDM_P_MHL_MAILBOX_REV_MASK             (0xFF)
#define BHDM_P_MHL_MAILBOX_REV_LSB              (0)

/* Byte 1: MPM state (2 bits) */
#define BHDM_P_MHL_MAILBOX_MPM_STATE_OFFSET     BHDM_P_Mhl_MailboxField_eMpmState
#define BHDM_P_MHL_MAILBOX_MPM_STATE_MASK       (0x3)
#define BHDM_P_MHL_MAILBOX_MPM_STATE_LSB        (0)

/* Byte 2: Link state
   Bit 0: MSC requester pending
   Bit 1: MSC responder pending
   Bit 2: DDC requester pending
   Bit 3: HPD up (1) or down (0)
   Bit 4: DCAP valid this is NOT DCAP_CHG
   Bit 5: Scratch pad updated.

   PATH_EN is in the source/sink LINK_MODE register respectively

*/
#define BHDM_P_MHL_MAILBOX_LINK_STATE_OFFSET        BHDM_P_Mhl_MailboxField_eLinkState
#define BHDM_P_MHL_MAILBOX_LINK_STATE_MASK          (0xFF)
#define BHDM_P_MHL_MAILBOX_LINK_STATE_LSB           (0)

/* MSC requester pending */
#define BHDM_P_MHL_MAILBOX_MSC_REQ_PENDING_OFFSET   BHDM_P_MHL_MAILBOX_LINK_STATE_OFFSET
#define BHDM_P_MHL_MAILBOX_MSC_REQ_PENDING_MASK     (0x1)
#define BHDM_P_MHL_MAILBOX_MSC_REQ_PENDING_LSB      (0)

/* MSC responder pending */
#define BHDM_P_MHL_MAILBOX_MSC_RESP_PENDING_OFFSET  BHDM_P_MHL_MAILBOX_LINK_STATE_OFFSET
#define BHDM_P_MHL_MAILBOX_MSC_RESP_PENDING_MASK    (0x1)
#define BHDM_P_MHL_MAILBOX_MSC_RESP_PENDING_LSB     (1)

/* DDC requester pending */
#define BHDM_P_MHL_MAILBOX_DDC_REQ_PENDING_OFFSET   BHDM_P_MHL_MAILBOX_LINK_STATE_OFFSET
#define BHDM_P_MHL_MAILBOX_DDC_REQ_PENDING_MASK     (0x1)
#define BHDM_P_MHL_MAILBOX_DDC_REQ_PENDING_LSB      (2)

#define BHDM_P_MHL_MAILBOX_HPD_OFFSET               BHDM_P_MHL_MAILBOX_LINK_STATE_OFFSET
#define BHDM_P_MHL_MAILBOX_HPD_MASK                 (0x1)
#define BHDM_P_MHL_MAILBOX_HPD_LSB                  (3)

/* Sink DCAP valid */
#define BHDM_P_MHL_MAILBOX_DCAP_VALID_OFFSET        BHDM_P_MHL_MAILBOX_LINK_STATE_OFFSET
#define BHDM_P_MHL_MAILBOX_DCAP_VALID_MASK          (0x1)
#define BHDM_P_MHL_MAILBOX_DCAP_VALID_LSB           (4)

/* Source scratch pad updated by sink? */
#define BHDM_P_MHL_MAILBOX_SCRATCHPAD_VALID_OFFSET    BHDM_P_MHL_MAILBOX_LINK_STATE_OFFSET
#define BHDM_P_MHL_MAILBOX_SCRATCHPAD_VALID_MASK      (0x1)
#define BHDM_P_MHL_MAILBOX_SCRATCHPAD_VALID_LSB       (5)

/* Byte 3: Source MSC error code (7 bits) */
#define BHDM_P_MHL_MAILBOX_SRC_MSC_ERRORCODE_OFFSET   BHDM_P_Mhl_MailboxField_eSrcMscErrorCode
#define BHDM_P_MHL_MAILBOX_SRC_MSC_ERRORCODE_MASK     (0x7F)
#define BHDM_P_MHL_MAILBOX_SRC_MSC_ERRORCODE_LSB      (0)

/* Byte 4: Source DDC error code (4 bits) */
#define BHDM_P_MHL_MAILBOX_SRC_DDC_ERRORCODE_OFFSET   BHDM_P_Mhl_MailboxField_eSrcDdcErrorCode
#define BHDM_P_MHL_MAILBOX_SRC_DDC_ERRORCODE_MASK     (0xF)
#define BHDM_P_MHL_MAILBOX_SRC_DDC_ERRORCODE_LSB      (0)

/* Byte 5 and 6, same as byte 3 and 4 but received from sink */
#define BHDM_P_MHL_MAILBOX_SINK_MSC_ERRORCODE_OFFSET  BHDM_P_Mhl_MailboxField_eSinkMscErrorCode
#define BHDM_P_MHL_MAILBOX_SINK_MSC_ERRORCODE_MASK    (0x7F)
#define BHDM_P_MHL_MAILBOX_SINK_MSC_ERRORCODE_LSB     (0)

#define BHDM_P_MHL_MAILBOX_SINK_DDC_ERRORCODE_OFFSET  BHDM_P_Mhl_MailboxField_eSinkDdcErrorCode
#define BHDM_P_MHL_MAILBOX_SINK_DDC_ERRORCODE_MASK    (0xF)
#define BHDM_P_MHL_MAILBOX_SINK_DDC_ERRORCODE_LSB     (0)

/* Byte 7 Source CONNECTED_RDY
   Bit 0 DCAP_RDY
*/
#define BHDM_P_MHL_MAILBOX_SRC_CONNECTED_RDY_OFFSET   BHDM_P_Mhl_MailboxField_eSrcConnectedRdy
#define BHDM_P_MHL_MAILBOX_SRC_CONNECTED_RDY_MASK     (0x1)
#define BHDM_P_MHL_MAILBOX_SRC_CONNECTED_RDY_LSB      (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_RDY_OFFSET        BHDM_P_MHL_MAILBOX_SRC_CONNECTED_RDY_OFFSET
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_RDY_MASK          (0x1)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_RDY_LSB           (0)

/* Byte 8 Source LINK_MODE_1
   Bit 0-2 CLK_MODE
   Bit 3 PATH_EN
   Bit 4 MUTED

   LINK_MODE_1 is updated from FCB's LINK_MODE_2 at boot (if FCB is valid)
   and by LINK_MODE_0 at run time just before source sends PATH_EN=1
 */
#define BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_1_OFFSET     BHDM_P_Mhl_MailboxField_eSrcLinkMode1
#define BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_1_MASK       (0x1F)
#define BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_1_LSB        (0)
#define BHDM_P_MHL_MAILBOX_SRC_CLK_MODE_1_OFFSET      BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_1_OFFSET
#define BHDM_P_MHL_MAILBOX_SRC_CLK_MODE_1_MASK        (0x7)
#define BHDM_P_MHL_MAILBOX_SRC_CLK_MODE_1_LSB         (0)
#define BHDM_P_MHL_MAILBOX_SRC_PATH_EN_1_OFFSET       BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_1_OFFSET
#define BHDM_P_MHL_MAILBOX_SRC_PATH_EN_1_MASK         (0x1)
#define BHDM_P_MHL_MAILBOX_SRC_PATH_EN_1_LSB          (3)
#define BHDM_P_MHL_MAILBOX_SRC_MUTED_1_OFFSET         BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_1_OFFSET
#define BHDM_P_MHL_MAILBOX_SRC_MUTED_1_MASK           (0x1)
#define BHDM_P_MHL_MAILBOX_SRC_MUTED_1_LSB            (4)

/* Byte 9-10 same as byte 7 and 8 but for sink,
   sink only has PATH_EN and DCAP_RDY bits */
#define BHDM_P_MHL_MAILBOX_SINK_CONNECTED_RDY_OFFSET  BHDM_P_Mhl_MailboxField_eSinkConnectedRdy
#define BHDM_P_MHL_MAILBOX_SINK_CONNECTED_RDY_MASK    (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_CONNECTED_RDY_LSB     (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_RDY_OFFSET       BHDM_P_MHL_MAILBOX_SINK_CONNECTED_RDY_OFFSET
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_RDY_MASK         (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_RDY_LSB          (0)

#define BHDM_P_MHL_MAILBOX_SINK_LINK_MODE_OFFSET      BHDM_P_Mhl_MailboxField_eSinkLinkMode
#define BHDM_P_MHL_MAILBOX_SINK_LINK_MODE_MASK        (0x1F)
#define BHDM_P_MHL_MAILBOX_SINK_LINK_MODE_LSB         (0)
#define BHDM_P_MHL_MAILBOX_SINK_PATH_EN_OFFSET        BHDM_P_MHL_MAILBOX_SINK_LINK_MODE_OFFSET
#define BHDM_P_MHL_MAILBOX_SINK_PATH_EN_MASK          (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_PATH_EN_LSB           (3)

/* Byte 11 Sink INT register RCHANGE_INT
   Bit 0: DCAP_CHG
   Bit 1: DSCR_CHG
   Bit 2: REQ_WRT
   Bit 3: GRT_WRT
   Bit 4: 3D_REQ
 */
#define BHDM_P_MHL_MAILBOX_SINK_RCHANGE_INT_OFFSET    BHDM_P_Mhl_MailboxField_eSinkRchangeInt
#define BHDM_P_MHL_MAILBOX_SINK_RCHANGE_INT_MASK      (0x1F)
#define BHDM_P_MHL_MAILBOX_SINK_RCHANGE_INT_LSB       (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_CHG_OFFSET       BHDM_P_MHL_MAILBOX_SINK_RCHANGE_INT_OFFSET
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_CHG_MASK         (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_CHG_LSB          (0)
#define BHDM_P_MHL_MAILBOX_SINK_DSCR_CHG_OFFSET       BHDM_P_MHL_MAILBOX_SINK_RCHANGE_INT_OFFSET
#define BHDM_P_MHL_MAILBOX_SINK_DSCR_CHG_MASK         (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_DSCR_CHG_LSB          (1)
#define BHDM_P_MHL_MAILBOX_SINK_REQ_WRT_OFFSET        BHDM_P_MHL_MAILBOX_SINK_RCHANGE_INT_OFFSET
#define BHDM_P_MHL_MAILBOX_SINK_REQ_WRT_MASK          (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_REQ_WRT_LSB           (2)
#define BHDM_P_MHL_MAILBOX_SINK_GRT_WRT_OFFSET        BHDM_P_MHL_MAILBOX_SINK_RCHANGE_INT_OFFSET
#define BHDM_P_MHL_MAILBOX_SINK_GRT_WRT_MASK          (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_GRT_WRT_LSB           (3)
#define BHDM_P_MHL_MAILBOX_SINK_3D_REQ_OFFSET         BHDM_P_MHL_MAILBOX_SINK_RCHANGE_INT_OFFSET
#define BHDM_P_MHL_MAILBOX_SINK_3D_REQ_MASK           (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_3D_REQ_LSB            (4)

/* Byte 12 Sink INT register DCHANGE_INT
   Bit 1: EDID_CHG
 */
#define BHDM_P_MHL_MAILBOX_SINK_DCHANGE_INT_OFFSET    BHDM_P_Mhl_MailboxField_eSinkDchangeInt
#define BHDM_P_MHL_MAILBOX_SINK_DCHANGE_INT_MASK      (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_DCHANGE_INT_LSB       (1)
#define BHDM_P_MHL_MAILBOX_SINK_EDID_CHG_OFFSET       BHDM_P_MHL_MAILBOX_SINK_DCHANGE_INT_OFFSET
#define BHDM_P_MHL_MAILBOX_SINK_EDID_CHG_MASK         (0x1)
#define BHDM_P_MHL_MAILBOX_SINK_EDID_CHG_LSB          (1)

/* Byte 13 to 28 Source DCAP (used as reply to READ_DEVCAP from sink) */
/* Offset to the first byte of DCAP */
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_OFFSET  	           BHDM_P_Mhl_MailboxField_eSrcDcapDevState
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_MASK                  (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_LSB                   (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_STATE_OFFSET      BHDM_P_Mhl_MailboxField_eSrcDcapDevState
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_STATE_MASK        (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_STATE_LSB         (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_MHL_VER_OFFSET        BHDM_P_Mhl_MailboxField_SrcDcapMhlVer
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_MHL_VER_MASK          (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_MHL_VER_LSB           (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_CAT_OFFSET        BHDM_P_Mhl_MailboxField_eSrcDcapDevCat
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_CAT_MASK          (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_CAT_LSB           (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_ADT_ID_H_OFFSET       BHDM_P_Mhl_MailboxField_eSrcDcapAdtIdHi
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_ADT_ID_H_MASK         (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_ADT_ID_H_LSB          (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_ADT_ID_L_OFFSET       BHDM_P_Mhl_MailboxField_eSrcDcapAdtIdLo
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_ADT_ID_L_MASK         (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_ADT_ID_L_LSB          (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_VLINK_MODE_OFFSET     BHDM_P_Mhl_MailboxField_eSrcDcapVlinkMode
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_VLINK_MODE_MASK       (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_VLINK_MODE_LSB        (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_ALINK_MODE_OFFSET     BHDM_P_Mhl_MailboxField_eSrcDcapAlinkMode
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_ALINK_MODE_MASK       (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_ALINK_MODE_LSB        (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_VTYPE_OFFSET          BHDM_P_Mhl_MailboxField_eSrcDcapVtype
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_VTYPE_MASK            (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_VTYPE_LSB             (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_LOG_DEV_MAP_OFFSET    BHDM_P_Mhl_MailboxField_eSrcDcapLogDevMap
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_LOG_DEV_MAP_MASK      (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_LOG_DEV_MAP_LSB       (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_BANDWIDTH_OFFSET      BHDM_P_Mhl_MailboxField_eSrcDcapBandwidth
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_BANDWIDTH_MASK        (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_BANDWIDTH_LSB         (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_FEATURE_OFFSET        BHDM_P_Mhl_MailboxField_eSrcDcapFeature
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_FEATURE_MASK          (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_FEATURE_LSB           (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_ID_H_OFFSET       BHDM_P_Mhl_MailboxField_eSrcDcapDevIdHi
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_ID_H_MASK         (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_ID_H_LSB          (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_ID_L_OFFSET       BHDM_P_Mhl_MailboxField_eSrcDcapDevIdLo
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_ID_L_MASK         (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_DEV_ID_L_LSB          (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_SCRATCH_SIZE_OFFSET   BHDM_P_Mhl_MailboxField_eSrcDcapScratchSize
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_SCRATCH_SIZE_MASK     (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_SCRATCH_SIZE_LSB      (0)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_INT_STAT_SIZE_OFFSET  BHDM_P_Mhl_MailboxField_eSrcDcapIntStatSize
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_INT_STAT_SIZE_MASK    (0xFF)
#define BHDM_P_MHL_MAILBOX_SRC_DCAP_INT_STAT_SIZE_LSB     (0)
/* Reserved field not listed */

/* Bytes 29-44 Sink's DCAP (obtained from READ_DEVCAP sent to sink) */
/* Offset to the first byte of DCAP */
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_OFFSET  	            BHDM_P_Mhl_MailboxField_eSinkDcapDevState
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_MASK                  (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_LSB                   (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_STATE_OFFSET      BHDM_P_Mhl_MailboxField_eSinkDcapDevState
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_STATE_MASK        (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_STATE_LSB         (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_MHL_VER_OFFSET        BHDM_P_Mhl_MailboxField_eSinkDcapMhlVer
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_MHL_VER_MASK          (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_MHL_VER_LSB           (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_CAT_OFFSET        BHDM_P_Mhl_MailboxField_eSinkDcapDevCat
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_CAT_MASK          (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_CAT_LSB           (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_ADT_ID_H_OFFSET       BHDM_P_Mhl_MailboxField_eSinkDcapAdtIdHi
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_ADT_ID_H_MASK         (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_ADT_ID_H_LSB          (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_ADT_ID_L_OFFSET       BHDM_P_Mhl_MailboxField_eSinkDcapAdtIdLo
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_ADT_ID_L_MASK         (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_ADT_ID_L_LSB          (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_VLINK_MODE_OFFSET     BHDM_P_Mhl_MailboxField_eSinkDcapVlinkMode
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_VLINK_MODE_MASK       (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_VLINK_MODE_LSB        (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_ALINK_MODE_OFFSET     BHDM_P_Mhl_MailboxField_eSinkDcapAlinkMode
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_ALINK_MODE_MASK       (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_ALINK_MODE_LSB        (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_VTYPE_OFFSET          BHDM_P_Mhl_MailboxField_eSinkDcapVtype
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_VTYPE_MASK            (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_VTYPE_LSB             (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_LOG_DEV_MAP_OFFSET    BHDM_P_Mhl_MailboxField_eSinkDcapLogDevMap
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_LOG_DEV_MAP_MASK      (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_LOG_DEV_MAP_LSB       (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_BANDWIDTH_OFFSET      BHDM_P_Mhl_MailboxField_eSinkDcapBandwidth
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_BANDWIDTH_MASK        (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_BANDWIDTH_LSB         (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_FEATURE_OFFSET        BHDM_P_Mhl_MailboxField_eSinkDcapFeature
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_FEATURE_MASK          (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_FEATURE_LSB           (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_ID_H_OFFSET       BHDM_P_Mhl_MailboxField_eSinkDcapDevIdHi
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_ID_H_MASK         (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_ID_H_LSB          (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_ID_L_OFFSET       BHDM_P_Mhl_MailboxField_eSinkDcapDevIdLo
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_ID_L_MASK         (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_DEV_ID_L_LSB          (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_SCRATCH_SIZE_OFFSET   BHDM_P_Mhl_MailboxField_eSinkDcapScratchSize
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_SCRATCH_SIZE_MASK     (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_SCRATCH_SIZE_LSB      (0)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_INT_STAT_SIZE_OFFSET  BHDM_P_Mhl_MailboxField_eSinkDcapIntStatSize
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_INT_STAT_SIZE_MASK    (0xFF)
#define BHDM_P_MHL_MAILBOX_SINK_DCAP_INT_STAT_SIZE_LSB     (0)
/* Reserved field not listed */

/* Byte 45 to 60 is scratchpad register (16 bytes)
   This marco only gives you the first byte */
#define BHDM_P_MHL_MAILBOX_SCRATCHPAD_OFFSET               BHDM_P_Mhl_MailboxField_eScratchpad
#define BHDM_P_MHL_MAILBOX_SCRATCHPAD_MASK                 (0xFF)
#define BHDM_P_MHL_MAILBOX_SCRATCHPAD_LSB                  (0)

/* Byte 61 is EDID state
   Bit 0-4: no. of blocks of EDID
   Bit 7: EDID valid or not
 */
#define BHDM_P_MHL_MAILBOX_EDID_STATE_OFFSET     BHDM_P_Mhl_MailboxField_eEdidState
#define BHDM_P_MHL_MAILBOX_EDID_STATE_MASK       (0xFF)
#define BHDM_P_MHL_MAILBOX_EDID_STATE_LSB        (0)
#define BHDM_P_MHL_MAILBOX_EDID_VALID_OFFSET     BHDM_P_Mhl_MailboxField_eEdidState
#define BHDM_P_MHL_MAILBOX_EDID_VALID_MASK       (0x1)
#define BHDM_P_MHL_MAILBOX_EDID_VALID_LSB        (7)
#define BHDM_P_MHL_MAILBOX_EDID_SIZE_OFFSET      BHDM_P_Mhl_MailboxField_eEdidState
#define BHDM_P_MHL_MAILBOX_EDID_SIZE_MASK        (0x1F)
#define BHDM_P_MHL_MAILBOX_EDID_SIZE_LSB         (0)

/* Byte 62 to 62 + BHDM_P_MHL_MAILBOX_EDID_SIZE-1 , EDID */
#define BHDM_P_MHL_MAILBOX_EDID_OFFSET           BHDM_P_Mhl_MailboxField_eEdid

/* The last few bytes of the mailbox are written to by host
   Host byte 0: Host state
   Bit 0 TX_READY
   Bit 1 STOP_REQUEST
 */

#define BHDM_P_MHL_MAILBOX_HOST_STATE_OFFSET     BHDM_P_Mhl_MailboxField_eHostState
#define BHDM_P_MHL_MAILBOX_HOST_STATE_MASK       (0x3)
#define BHDM_P_MHL_MAILBOX_HOST_STATE_LSB        0
#define BHDM_P_MHL_MAILBOX_HOST_TX_READY_OFFSET  BHDM_P_MHL_MAILBOX_HOST_STATE_OFFSET
#define BHDM_P_MHL_MAILBOX_HOST_TX_READY_MASK    (0x1)
#define BHDM_P_MHL_MAILBOX_HOST_TX_READY_LSB     (0)
#define BHDM_P_MHL_MAILBOX_HOST_STOP_REQ_OFFSET  BHDM_P_MHL_MAILBOX_HOST_STATE_OFFSET
#define BHDM_P_MHL_MAILBOX_HOST_STOP_REQ_MASK    (0x1)
#define BHDM_P_MHL_MAILBOX_HOST_STOP_REQ_LSB     (1)
/* Host byte 1 Source LINK_MODE_0
   Bit 0-2 CLK_MODE
   Bit 3 PATH_EN
   Bit 4 MUTED

   LINK_MODE_0 is used to update LINK_MODE_1 at run time
   just before source sends PATH_EN=1. Host must set this before
   setting TX_READY=1
 */
#define BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_0_OFFSET   BHDM_P_Mhl_MailboxField_eSrcLinkMode0
#define BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_0_MASK     (0x1F)
#define BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_0_LSB      (0)
#define BHDM_P_MHL_MAILBOX_SRC_CLK_MODE_0_OFFSET    BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_0_OFFSET
#define BHDM_P_MHL_MAILBOX_SRC_CLK_MODE_0_MASK      (0x7)
#define BHDM_P_MHL_MAILBOX_SRC_CLK_MODE_0_LSB       (0)
#define BHDM_P_MHL_MAILBOX_SRC_PATH_EN_0_OFFSET     BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_0_OFFSET
#define BHDM_P_MHL_MAILBOX_SRC_PATH_EN_0_MASK       (0x1)
#define BHDM_P_MHL_MAILBOX_SRC_PATH_EN_0_LSB        (3)
#define BHDM_P_MHL_MAILBOX_SRC_MUTED_0_OFFSET       BHDM_P_MHL_MAILBOX_SRC_LINK_MODE_0_OFFSET
#define BHDM_P_MHL_MAILBOX_SRC_MUTED_0_MASK         (0x1)
#define BHDM_P_MHL_MAILBOX_SRC_MUTED_0_LSB          (4)

/* Read a byte from Mailbox */
uint8_t BHDM_P_Mhl_Mailbox_Read
	( BREG_Handle hRegister,
	  uint32_t    ulOffset );

/* Write a byte from Mailbox */
void BHDM_P_Mhl_Mailbox_Write
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucValue );

uint8_t BHDM_P_Mhl_Mailbox_Read_isr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset );

void BHDM_P_Mhl_Mailbox_Write_isr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucValue );

/* Get a field from MAILBOX */
uint8_t BHDM_P_Mhl_Mailbox_GetField
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucMask,
	  uint8_t     ucLsb );

/* Set a field in MAILBOX */
void BHDM_P_Mhl_Mailbox_SetField
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucMask,
	  uint8_t     ucLsb,
	  uint8_t     ucValue );

/* Get a field from MAILBOX */
uint8_t BHDM_P_Mhl_Mailbox_GetField_isr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucMask,
	  uint8_t     ucLsb );

/* Set a field in MAILBOX */
void BHDM_P_Mhl_Mailbox_SetField_isr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset,
	  uint8_t     ucMask,
	  uint8_t     ucLsb,
	  uint8_t     ucValue );

uint32_t BHDM_P_Mhl_Mailbox_GetFieldAddr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset );

uint32_t BHDM_P_Mhl_Mailbox_GetFieldAddr_isr
	( BREG_Handle hRegister,
	  uint32_t    ulOffset );


/* Macro to get a field from mail box, except EDID
   Set field as XXX, where XXX is the middle part
   of the macro field name defined above, e.g. SINK_DCAP0
 */
#define BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(reghandle,field)  BHDM_P_Mhl_Mailbox_GetField_isr( \
		reghandle,                                          \
		BHDM_P_MHL_MAILBOX_##field##_OFFSET,	  \
		BHDM_P_MHL_MAILBOX_##field##_MASK,		  \
		BHDM_P_MHL_MAILBOX_##field##_LSB)


/* Macro to set a field in mail box, except EDID */
#define BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(reghandle,field,value)  BHDM_P_Mhl_Mailbox_SetField_isr( \
		reghandle,                                          \
		BHDM_P_MHL_MAILBOX_##field##_OFFSET,	  \
		BHDM_P_MHL_MAILBOX_##field##_MASK,		  \
		BHDM_P_MHL_MAILBOX_##field##_LSB,		  \
		value)

/* Macro to clear a field, just set it to zero */
#define BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(reghandle,field)  BHDM_P_MHL_MAILBOX_SET_FIELD_ISR( \
		reghandle,field,0)

/* Macro to get the offset of a byte in the mailbox to assist in reading the mailbox field */
#define BHDM_P_MHL_MAILBOX_GET_FIELD_ADDR_ISR(reghandle,field) BHDM_P_Mhl_Mailbox_GetFieldAddr_isr( \
		reghandle,BHDM_P_MHL_MAILBOX_##field##_OFFSET)


/* Macro to get a field from mail box, except EDID
   Set field as XXX, where XXX is the middle part
   of the macro field name defined above, e.g. SINK_DCAP0
 */
#define BHDM_P_MHL_MAILBOX_GET_FIELD(reghandle,field)  BHDM_P_Mhl_Mailbox_GetField( \
		reghandle,                                          \
		BHDM_P_MHL_MAILBOX_##field##_OFFSET,	  \
		BHDM_P_MHL_MAILBOX_##field##_MASK,		  \
		BHDM_P_MHL_MAILBOX_##field##_LSB)


/* Macro to set a field in mail box, except EDID */
#define BHDM_P_MHL_MAILBOX_SET_FIELD(reghandle,field,value)  BHDM_P_Mhl_Mailbox_SetField( \
		reghandle,                                          \
		BHDM_P_MHL_MAILBOX_##field##_OFFSET,	  \
		BHDM_P_MHL_MAILBOX_##field##_MASK,		  \
		BHDM_P_MHL_MAILBOX_##field##_LSB,		  \
		value)

/* Macro to clear a field, just set it to zero */
#define BHDM_P_MHL_MAILBOX_CLR_FIELD(reghandle,field)  BHDM_P_MHL_MAILBOX_SET_FIELD( \
		reghandle,field,0)

/* Macro to get the offset of a byte in the mailbox to assist in reading the mailbox field */
#define BHDM_P_MHL_MAILBOX_GET_FIELD_ADDR(reghandle,field) BHDM_P_Mhl_Mailbox_GetFieldAddr( \
		reghandle,BHDM_P_MHL_MAILBOX_##field##_OFFSET)


/*
 * Fields which can be used in GET/SET_FIELD:
 * <General>
 * REV, MPM_STATE
 * MSC_REQ_PENDING, MSC_RESP_PENDING, DDC_REQ_PENDING
 * HPD,  DCAP_VALID
 *
 * <Errorcodes>
 * SRC_MSC_ERRORCODE
 * SRC_DDC_ERRORCODE
 * SINK_MSC_ERRORCODE
 * SINK_DDC_ERRORCODE
 *
 * <STAT>
 * SRC_CONNECTED_RDY, SRC_DCAP_RDY
 * SRC_LINK_MODE, SRC_CLK_MODE, SRC_PATH_EN, SRC_MUTED
 * SINK_CONNECTED_RDY, SINK_DCAP_RDY
 * SINK_LINK_MODE, SINK_PATH_EN
 *
 * <Interrupts>
 * SINK_RCHANGE_INT, SINK_DCHANGE_INT
 * SINK_DCAP_CHG, SINK_DSCR_CHG, SINK_REQ_WRT, SINK_GRT_WRT, SINK_3D_REQ
 * SINK_EDID_CHG
 *
 * <DCAP>
 * SRC_DCAP, SINK_DCAP (only the first byte)
 * SRC_DCAP_DEV_STATE, SRC_DCAP_MHL_VER, SRC_DCAP_DEV_CAT,
 * SRC_DCAP_ADT_ID_H, SRC_DCAP_ADT_ID_L, SRC_DCAP_VLINK_MODE,
 * SRC_DCAP_ALINK_MODE, SRC_DCAP_VTYPE, SRC_DCAP_LOG_DEV_MAP,
 * SRC_DCAP_BANDWIDTH, SRC_DCAP_DEV_ID_H, SRC_DCAP_DEV_ID_L,
 * SRC_DCAP_SCRATCH_SIZE, SRC_DCAP_INT_STAT_SIZE, SRC_DCAP_FEATURE
 * SINK_DCAP_DEV_STATE, SINK_DCAP_MHL_VER, SINK_DCAP_DEV_CAT,
 * SINK_DCAP_ADT_ID_H, SINK_DCAP_ADT_ID_L, SINK_DCAP_VLINK_MODE,
 * SINK_DCAP_ALINK_MODE, SINK_DCAP_VTYPE, SINK_DCAP_LOG_DEV_MAP,
 * SINK_DCAP_BANDWIDTH, SINK_DCAP_DEV_ID_H, SINK_DCAP_DEV_ID_L,
 * SINK_DCAP_SCRATCH_SIZE, SINK_DCAP_INT_STAT_SIZE, SINK_DCAP_FEATURE
 *
 * <Misc>
 * EDID_VALID (non-zero if EDID is ready)
 * EDID_SIZE  (no. of EDID blocks)
 * EDID (just give you the first byte)
 * <Host>
 * HOST_STATE, HOST_TX_READY, HOST_STOP_REQ
 */

uint32_t BHDM_P_Mhl_Mailbox_GetEdidAddr
	( void );

/* Set the source DCAP in one go */
void BHDM_P_Mhl_Mailbox_SetSrcDcap
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap );

/* Get the source DCAP in one go */
void BHDM_P_Mhl_Mailbox_GetSrcDcap
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap );

/* Get the sink DCAP in one go */
void BHDM_P_Mhl_Mailbox_GetSinkDcap
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap );


/* Get the EDID location in mailbox */
uint32_t BHDM_P_Mhl_Mailbox_GetEdidAddr_isr
	( void );

/* Set the source DCAP in one go */
void BHDM_P_Mhl_Mailbox_SetSrcDcap_isr
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap );

/* Get the source DCAP in one go */
void BHDM_P_Mhl_Mailbox_GetSrcDcap_isr
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap );

/* Get the sink DCAP in one go */
void BHDM_P_Mhl_Mailbox_GetSinkDcap_isr
	( BREG_Handle   hRegister,
	  uint8_t      *pucDcap );

/* Getting/setting the scratchpad registers */
int BHDM_P_Mhl_Mailbox_GetScratchpad_isr
	( BREG_Handle  hRegister,
	  uint8_t     *pucScratchpad,
	  uint8_t      ucOffset,
	  uint8_t      ucSize );

int BHDM_P_Mhl_Mailbox_SetScratchpad_isr
	( BREG_Handle  hRegister,
	  uint8_t     *pucScratchpad,
	  uint8_t      ucOffset,
	  uint8_t      ucSize );

/* Process incoming SET_INT by setting a bit, if you want to clear
   a bit, call cbus_mailbox_clear_int.
   returns zero if success (offset is valid)
   if the return code is < 0, the host should
   reply with ABORT.
 */
BERR_Code BHDM_P_Mhl_Mailbox_ProcessInt_isr
	( BREG_Handle  hRegister,
	  uint8_t      ucOffset,
	  uint8_t      ucVal );

/* Clear one of the bits in the sink INT register
   after the host has processed it
   val should only have 1 bit set

   Returns 0 if the offset is valid
*/
BERR_Code BHDM_P_Mhl_Mailbox_ClearInt_isr
	( BREG_Handle  hRegister,
	  uint8_t      ucOffset,
	  uint8_t      ucVal );

/* Process incoming WRITE_STAT command. Note this does NOT update
   the source's STAT registers, which should be changed
   with the set_field function.
   Returns 0 if the offset is valid,
   if return < 0, the host should reply with ABORT
*/
BERR_Code BHDM_P_Mhl_Mailbox_ProcessStat_isr
	( BREG_Handle  hRegister,
	  uint8_t      ucOffset,
	  uint8_t      ucVal );

#if BHDM_MHL_CTS
/* Call this when firmware has updated scratchpad */
void BHDM_P_Mhl_Mailbox_ScratchpadUpdateDone_isr
	( BREG_Handle  hRegister );

/* Call this when firmware has read DCAP successfully */
void BHDM_P_Mhl_Mailbox_DcapUpdateDone_isr
	( BREG_Handle  hRegisterl );

/* Call this when firmware has read EDID successfully */
void BHDM_P_Mhl_Mailbox_EdidUpdateDone_isr
	( BREG_Handle  hRegister );
#endif

/* Initialise the mailbox from fresh start,
   do NOT call this if initialising from scratchpad.
   Argument is mailbox revision and source DCAP
 */
void BHDM_P_Mhl_Mailbox_Init
	( BREG_Handle  hRegister,
	  uint8_t      ucRev,
	  uint8_t     *pucDcap,
	  uint8_t      ucSrcLinkMode );

#if 0
/* Initialise the mailbox from scratchpad */
void BHDM_P_Mhl_Mailbox_Scratchpad
	( BREG_Handle  hRegister,
	  uint8_t     *pucDcap );
#endif
#endif /* BHDM_MHL_MAILBOX_PRIV_H__ */
