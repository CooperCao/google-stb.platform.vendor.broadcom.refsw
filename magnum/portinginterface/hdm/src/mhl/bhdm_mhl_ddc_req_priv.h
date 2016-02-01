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

#ifndef BHDM_MHL_DDC_REQ_PRIV_H__
#define BHDM_MHL_DDC_REQ_PRIV_H__

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"   	/* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */

#include "bchp_common.h"

#include "bhdm_hdcp.h"
#include "bhdm_mhl_const_priv.h"
#include "bhdm_mhl_common_priv.h"
#include "bhdm_mhl_req_priv.h"
#include "bhdm_mhl_msc_req_priv.h"


#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BHDM_MHL_DDC_REQ);


/* Maximum to split long ">>CONT <DATA..." or ">DATA <<ACK" sequence */
#define BHDM_P_CBUS_DDC_MAX_PKTS_SPLIT_DATA = 12
#define BHDM_P_CBUS_DDC_MAX_PKTS_IN_BUFFER  = 24  /* Hardware maximum */

/* Some states EDID and DCAP */
typedef enum
{
	BHDM_P_Mhl_EdidReadState_eStart  = 0,  /* Start of reading EDID (pending) */
	BHDM_P_Mhl_EdidReadState_eMiddle = 1,  /* In the middle of reading EDID, at least we have queued the first block to read */
	BHDM_P_Mhl_EdidReadState_eEnd    = 2   /* Finished reading EDID */
} BHDM_P_Mhl_EdidReadState;

/* DDC Events */
#define	BHDM_P_MHL_DDCREQ_EVENT_NONE               (0) /* place holder */

	/* events from MSC Responder task */
#define	BHDM_P_MHL_DDCREQ_EVENT_SET_HPD            (1 << 0) /* SET_HPD */
#define	BHDM_P_MHL_DDCREQ_EVENT_CLR_HPD            (1 << 1) /* CLR_HPD */
#define	BHDM_P_MHL_DDCREQ_EVENT_EDID_CHG_INT       (1 << 2) /* SET_INT received - with EDID_CHG set */

	/* Internal events from DDC Requester task */
#define	BHDM_P_MHL_DDCREQ_EVENT_EDID_BLOCK_READ    (1 << 3) /* Complete one block of EDID read */
#define	BHDM_P_MHL_DDCREQ_EVENT_SET_HPD_COMPLETED  (1 << 4) /* SET_HPD completed */
#define	BHDM_P_MHL_DDCREQ_EVENT_CLR_HPD_COMPLETED  (1 << 5) /* CLR_HPD completed */

	/* DDC REQ HW INTR events (CBUS_INTR2_1 register) */
	/* for optimized implementation while posting flag for task from IRQ4, these events have been assign same bit position as in the CBUS_INTR2_1 register */
#define	BHDM_P_MHL_DDCREQ_EVENT_DONE             BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_DONE_MASK  /* (1 << 20) */
#define	BHDM_P_MHL_DDCREQ_EVENT_RX_MISMATCH      BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_RX_MISMATCH_MASK
#define	BHDM_P_MHL_DDCREQ_EVENT_RX_CMD_ERROR     BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_RX_CMD_ERROR_MASK
#define	BHDM_P_MHL_DDCREQ_EVENT_RX_TIMEOUT       BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_RX_TIMEOUT_MASK
#define	BHDM_P_MHL_DDCREQ_EVENT_ILLEGAL_SW_WR    BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_ILLEGAL_SW_WR_MASK
#define	BHDM_P_MHL_DDCREQ_EVENT_MAX_RETRIES_EXCEEDED    BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_MAX_RETRIES_EXCEEDED_MASK
#define	BHDM_P_MHL_DDCREQ_EVENT_UNEXPECTED_INBOUND_PKT  BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_UNEXPECTED_INBOUND_PKT_MASK /* (1<<26) */

#define	BHDM_P_MHL_DDCREQ_EVENT_ABORTDDCRECEIVED        BCHP_CBUS_INTR2_1_CPU_STATUS_ABORT_DDC_RECEIVED_MASK /* (1<<28) */
#define	BHDM_P_MHL_DDCREQ_EVENT_ABORTDDCTIMEOUTDONE     BCHP_CBUS_INTR2_1_CPU_STATUS_ABORT_DDC_TIMEOUT_DONE_MASK /* (1<<31) */

typedef uint32_t BHDM_P_Mhl_DdcReq_Event;

/* HDCP Events */
#define	BHDM_P_MHL_HDCP_EVENT_NONE                  (0)
#define	BHDM_P_MHL_HDCP_EVENT_VERSION               (1 << 1)
#define	BHDM_P_MHL_HDCP_EVENT_RX_BCAPS              (1 << 2)
#define	BHDM_P_MHL_HDCP_EVENT_RX_STATUS             (1 << 3)
#define	BHDM_P_MHL_HDCP_EVENT_KSV                   (1 << 4)
#define	BHDM_P_MHL_HDCP_EVENT_RX_KSV_LIST           (1 << 5)
#define	BHDM_P_MHL_HDCP_EVENT_RX_PJ                 (1 << 6)
#define	BHDM_P_MHL_HDCP_EVENT_RX_RI                 (1 << 7)
#define	BHDM_P_MHL_HDCP_EVENT_TX_AKSV               (1 << 8)
#define	BHDM_P_MHL_HDCP_EVENT_RX_BKSV               (1 << 9)
#define	BHDM_P_MHL_HDCP_EVENT_AN                    (1 << 10)
#define	BHDM_P_MHL_HDCP_EVENT_AINFO                 (1 << 11)

typedef int BHDM_P_Mhl_Hdcp_Event;


typedef enum
{
	BHDM_P_Mhl_HdcpEvent_VersionValue,
	BHDM_P_Mhl_HdcpEvent_RxBcapsValue,
	BHDM_P_Mhl_HdcpEvent_RxStatusValue,
	BHDM_P_Mhl_HdcpEvent_KsvValue,
	BHDM_P_Mhl_HdcpEvent_RepeaterKsvListValue,
	BHDM_P_Mhl_HdcpEvent_RxPjValue,
	BHDM_P_Mhl_HdcpEvent_RxRiValue,
	BHDM_P_Mhl_HdcpEvent_RxBksvValue,
	BHDM_P_Mhl_HdcpEvent_Invalid
} BHDM_P_Mhl_HdcpEventType ;

#define BHDM_P_MHL_HDCP_REPEATER_KSV_SIZE  (5)
#define BHDM_P_MHL_HDCP_RX_STATUS_SIZE     (2)
#define BHDM_P_MHL_HDCP_REPEATER_SHA_SIZE  (4)
#define BHDM_P_MHL_HDCP_RX_PJ_SIZE         (1)
#define BHDM_P_MHL_HDCP_RX_RI_SIZE         (2)

typedef struct BHDM_P_Mhl_HdcpInfo
{
	uint8_t                        ucHdcpVersion;
	uint8_t                        ucRxBcaps;
	uint8_t                        aucRxStatus[BHDM_P_MHL_HDCP_RX_STATUS_SIZE];
	uint8_t                        aucKsv[BHDM_P_MHL_HDCP_REPEATER_KSV_SIZE];
	uint8_t                       *pucRepeaterKsvList;
	uint16_t                       uiRepeaterKsvListSize;
	uint8_t                        ucRxPj;
	uint8_t                        aucRxRi[BHDM_P_MHL_HDCP_RX_RI_SIZE];
	uint8_t                        aucTxAksv[BHDM_HDCP_KSV_LENGTH];
	uint8_t                        aucRxBksv[BHDM_HDCP_KSV_LENGTH];
	uint8_t                        aucAnValue[BHDM_HDCP_AN_LENGTH];
	uint8_t                        ucAinfoByte;

} BHDM_P_Mhl_HdcpInfo;

typedef struct BHDM_P_Mhl_DdcReq_Object
{
	BDBG_OBJECT(BHDM_MHL_DDC_REQ)

	BHDM_P_Mhl_Req_Handle          hReq;

	/* Extra 4 bytes, ie., sizeof(uint32_t), are added as guard bytes to
	   avoid WriteAddr becoming equal to ReadAddr in case of the fifo being full. */
	uint8_t                        astCmdQueue[sizeof(BHDM_P_Mhl_CbusCmd) * BHDM_P_MHL_CBUS_LONG_Q_SIZE + sizeof(uint32_t)];

	/* EDID states */
	BHDM_P_Mhl_EdidReadState       eEdidReadState;
	bool                           bEdidChanged;
	uint8_t                       *pucEdidBlock;
	uint32_t                       ulEdidNumBlocks;
	uint8_t                        ucEdidCurrentBlock;
	uint8_t                        ucEdidReadAttempt;
	uint32_t                       ulEdidSize;
	bool                           bEdidValid;
	bool                           bSinkEdidChg;
	bool                           bHpd;

	/* Events */
	BHDM_P_Mhl_DdcReq_Event        eEvent;
	BHDM_P_Mhl_DdcReq_Event        eUnhandledEvent;

	/* HDCP */
	BHDM_P_Mhl_HdcpInfo			   stHdcpInfo;
	BHDM_P_Mhl_Hdcp_Event          eHdcpEvent;

	BKNI_EventHandle               hHdcpVersionValueEvent;
	BKNI_EventHandle               hHdcpRxBcapsValueEvent;
	BKNI_EventHandle               hHdcpRxStatusValueEvent;
	BKNI_EventHandle               hHdcpKsvValueEvent;
	BKNI_EventHandle               hHdcpRepeaterKsvListValueEvent;
	BKNI_EventHandle               hHdcpRxPjValueEvent;
	BKNI_EventHandle               hHdcpRxRiValueEvent;
	BKNI_EventHandle               hHdcpTxAksvValueEvent;
	BKNI_EventHandle               hHdcpRxBksvValueEvent;
	BKNI_EventHandle               hHdcpAnValueEvent;
	BKNI_EventHandle               hHdcpAinfoByteValueEvent;

} BHDM_P_Mhl_DdcReq_Object;

typedef struct BHDM_P_Mhl_DdcReq_Object *BHDM_P_Mhl_DdcReq_Handle;


BERR_Code BHDM_P_Mhl_DdcReq_Create
	( BHDM_P_Mhl_DdcReq_Handle   *hDdcReq );

BERR_Code BHDM_P_Mhl_DdcReq_Destroy
	( BHDM_P_Mhl_DdcReq_Handle    hDdcReq );

BERR_Code BHDM_P_Mhl_DdcReq_Init
	( BHDM_P_Mhl_DdcReq_Handle    hDdcReq,
	  BREG_Handle                 hRegister,
	  bool                        bReadEdid );

/* General READ function with offset.
   Argument: command queue
             dev address
             offset
	     no. of bytes to read
	     data array to hold result
	     short_read or not?
   Return: cbus_success if all the requests are
           successfully added

   If short_read is set to true, then offset is NOT
   sent in the command. Note this function will generate multiple
   DDC commands in the queue if necessary.

*/

BERR_Code BHDM_P_Mhl_DdcReq_Read_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  BHDM_P_Mhl_CbusCmdType     eCmdType,
	  uint8_t                    ucDevAddr,
	  uint8_t                    ucOffset,
	  uint32_t                   ulNumBytes,
	  uint8_t                   *pucData,
	  bool                       bShortRead,
	  uint8_t                    ucDelay );

/* A short read is just a DDC read without offset */
BERR_Code BHDM_P_Mhl_DdcReq_ShortRead_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  BHDM_P_Mhl_CbusCmdType     eCmdType,
	  uint8_t                    ucDevAddr,
	  uint32_t                   ulNumBytes,
	  uint8_t                   *pucData,
	  uint8_t                    ucDelay );

/* Reading with offset */
BERR_Code BHDM_P_Mhl_DdcReq_OffsetRead_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  BHDM_P_Mhl_CbusCmdType     eCmdType,
	  uint8_t                    ucDevAddr,
	  uint8_t                    ucOffset,
	  uint32_t                   ulNumBytes,
	  uint8_t                   *pucData,
	  uint8_t                    ucDelay );


/* General WRITE function with offset.
   Argument: dev address
             offset
	     no. of bytes to read
	     data array to write (const)
   Return: cbus_success if all commands are queued

   Handling of NACK is done at a higher layer.
*/

BERR_Code BHDM_P_Mhl_DdcReq_Write_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  BHDM_P_Mhl_CbusCmdType     eCmdType,
	  uint8_t                    ucDevAddr,
	  uint8_t                    ucOffset,
	  uint32_t                   ulNumBytes,
	  const uint8_t             *pucData,
	  uint8_t                    ucDelay );


/* Removes all queued DDC command(s).
*/
void BHDM_P_Mhl_DdcReq_RemoveAllCmds_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq );


/* Cancel all queued DDC command(s)
   e.g. because of NACK. It is up to the caller
   to requeue the request again if desired.
*/

void BHDM_P_Mhl_DdcReq_CancelAllCmds_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq );

/* Read a block of EDID
   Argument: segment number, usually 0
               unless EDID has more than 2 blocks
             offset (read start point, usually 0 or 128)
	     read size (typically 128)
	     data array to hold the block
   Return: cbus_success if the commands are added successfully

   The caller can only queue reading of 1 block of EDID at a time
   and must either wait or cancel the request before queueing another
   EDID block read. One single EDID block read will generates 10+
   DDC commands in the queue.

 */
BERR_Code  BHDM_P_Mhl_DdcReq_EdidRead_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  uint8_t                    ucSegment,
	  uint8_t                    ucOffset,
	  uint32_t                   ulNumBytes,
	  uint8_t                   *pucBlock,
	  uint8_t                    ucDelay );

void BHDM_P_Mhl_DdcReq_ClrHpdState_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq );

void BHDM_P_Mhl_DdcReq_AbortTx_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq );

BERR_Code BHDM_P_Mhl_DdcReq_GetDdcErrorCode_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  uint8_t                   *pucErr,
	  uint8_t                    ucDelay,
	  bool                       bLastCmd );

BERR_Code BHDM_P_Mhl_DdcReq_CheckError_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  int                        interrupt,
	  bool                      *pbAbortRequired,
	  BHDM_P_Mhl_CbusPkt        *pstLastPacket );

void BHDM_P_Mhl_DdcReq_HandleAbortDdcReceived_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq );

void BHDM_P_Mhl_DdcReq_HandleAbortDdcTimeoutDone_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  BHDM_P_Mhl_CbusHpd        *peLastHpdState );

void BHDM_P_Mhl_DdcReq_HandleDone_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq );

BERR_Code BHDM_P_Mhl_DdcReq_HandleErrors_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  int                        interrupt,
	  bool                      *pbStopPending,
	  BHDM_P_Mhl_MscReq_Event   *pMscReqEvent );

void BHDM_P_Mhl_DdcReq_HandleSetHpd_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  BHDM_P_Mhl_CbusHpd       *peLastHpdState,
	  bool                      bStopPending );

BERR_Code BHDM_P_Mhl_DdcReq_HandleClrHpd_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  BHDM_P_Mhl_CbusState     *pstCbusState );

BERR_Code BHDM_P_Mhl_DdcReq_HandleEdidReadBlock_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  uint32_t                 *pulUnhandledEvents );

void BHDM_P_Mhl_DdcReq_HandleEdidChgInt_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  uint32_t                 *pulUnhandledEvents );

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_DDC_REQ_PRIV_H__ */
