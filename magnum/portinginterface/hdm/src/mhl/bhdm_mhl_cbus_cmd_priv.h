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

#ifndef BHDM_MHL_CBUS_CMD_PRIV_H__
#define BHDM_MHL_CBUS_CMD_PRIV_H__


#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"   	/* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */

#include "bhdm_mhl_cfifo_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Refer to RDB for definitions shown below */
#define BHDM_P_MHL_CBUS_PKT_FIFO_SIZE   (24) /* 24 packet registers */

/* Each packet is packed to 16-bit, and two of these fit
   into a 32-bit register */
/* Offset to data bits in packed packet. See packet register definition. */
#define BHDM_P_MHL_CBUS_PKT_DATA_OFFSET (0)
#define BHDM_P_MHL_CBUS_PKT_DATA_MASK   (0xFF) /* data is always 8 bits */

/* Offset to type bit in packed packet. See packet register definition. */
#define BHDM_P_MHL_CBUS_PKT_TYPE_OFFSET (8)
#define BHDM_P_MHL_CBUS_PKT_TYPE_MASK   (0x1)

/* Offset to direction bit in packed packet. See packet register definition. */
#define BHDM_P_MHL_CBUS_PKT_DIR_OFFSET  (12)
#define BHDM_P_MHL_CBUS_PKT_DIR_MASK    (0x1)

/* We have two free pools of commands,
   the short one can only have up to 4 packets,
   while the long one will have the full fifo */
#define BHDM_P_MHL_CBUS_SHORT_CMD_LENGTH    (4) /* short command length */
#define BHDM_P_MHL_CBUS_LONG_CMD_LENGTH     BHDM_P_MHL_CBUS_PKT_FIFO_SIZE

/* We have two types of commands, short and long.
   Short commands have only up to 4 packets per command,
   which is suitable for most MSC commands (except BURST_WRITE).
   Long commands will have up to 24 packets per command,
   and are intended for DDC requests (and BURST_WRITE).
   The purpose of splitting up these command types is to
   reduce the amount of memory wastage when most of the
   queued commands are MSC requests. This allows software to
   have one single pending command queue while having
   two separate command queues.

   Implementation: we just have a static packet buffer array,
   with each command struct points to a different offset
   within the array, rather than attaching a static packet array
   to each command struct variable.
 */

/* Worst case short queue is 16 READ_DEVCAP message + a few MSC requests. Used with MSC REQ. */
#define BHDM_P_MHL_CBUS_SHORT_Q_SIZE (20)

/* Worst case long queue is 12+1 DDC message for reading 1 EDID block + WRITE_BURST. Used with DDC REQ. */
#define BHDM_P_MHL_CBUS_LONG_Q_SIZE (13)

#define BHDM_P_MHL_CBUS_PKT_POOL_SIZE (BHDM_P_MHL_CBUS_SHORT_Q_SIZE * BHDM_P_MHL_CBUS_SHORT_CMD_LENGTH + \
									   BHDM_P_MHL_CBUS_LONG_Q_SIZE  * BHDM_P_MHL_CBUS_LONG_CMD_LENGTH)


/* Data types */
/*
   packet direction, note that the direction
   is implicit in the responder and does not
   get programmed into the hardware
*/
typedef enum
{
	BHDM_P_Mhl_CbusPktDirection_eResp  = 0, /* Incoming */
	BHDM_P_Mhl_CbusPktDirection_eReq  = 1  /* Outgoing */
} BHDM_P_Mhl_CbusPktDirection;

/* packet type */
typedef enum
{
	BHDM_P_Mhl_CbusPktType_eData = 0, /* Data packet */
	BHDM_P_Mhl_CbusPktType_eCtrl = 1  /* Control packet */
} BHDM_P_Mhl_CbusPktType;

/* A packet - we use packing to reduce storage here,
   all 3 fields will fit in 32-bit.
   Note we explicitly know the many bits each field
   require */
typedef struct BHDM_P_Mhl_CbusPkt
{
	uint32_t ulType : 1; /* packet type */
	uint32_t ulDir  : 1; /* packet direction */
	uint8_t ucData;     /* data */
} BHDM_P_Mhl_CbusPkt;

/* command priority, smaller number = higher priority */
typedef enum
{
	BHDM_P_Mhl_CbusPriority_eResp    = 0,  /* Secondary MSC requester */
	BHDM_P_Mhl_CbusPriority_eReq     = 1,  /* Primary MSC requester */
	BHDM_P_Mhl_CbusPriority_eDdc     = 2,   /* DDC requester */
	BHDM_P_Mhl_CbusPriority_eUnkown  = 3
} BHDM_P_Mhl_CbusPriority;

/* command destination hardware request FIFO */
typedef enum
{
	BHDM_P_Mhl_CbusDest_eMscReq   = 0, /* MSC requester */
	BHDM_P_Mhl_CbusDest_eMscResp  = 1, /* MSC responder outbound FIFO */
	BHDM_P_Mhl_CbusDest_eDdcReq   = 2,  /* DDC requester */
	BHDM_P_Mhl_CbusDest_eUnknown   = 3
} BHDM_P_Mhl_CbusDest;

/* We combine the command state and result into one enum
   Each command will start with state pending
   and will transit to sent state after it is programmed in hardware
 */
typedef enum
{
	BHDM_P_Mhl_CbusCmdState_eFree       = 0,  /* initial state, not in use */
	BHDM_P_Mhl_CbusCmdState_ePending    = 1,  /* configured, but not yet sent */
	BHDM_P_Mhl_CbusCmdState_eIncoming   = 2,  /* only used for incoming request */
	BHDM_P_Mhl_CbusCmdState_eSent       = 3,  /* START has been triggered */
	BHDM_P_Mhl_CbusCmdState_eSuccess    = 4,  /* command completed normally and this
												 is the last command in a batch of commands */
	BHDM_P_Mhl_CbusCmdState_ePartDone	= 5,   /* The current command has completed
												 successfully, but there are more
												 commands in the batch */
	BHDM_P_Mhl_CbusCmdState_eCancelled  = 6,  /* command cancelled, e.g. we send ABORT */
	BHDM_P_Mhl_CbusCmdState_eError      = 7,  /* sink sent NACK bit */
	BHDM_P_Mhl_CbusCmdState_eMismatched = 8,  /* sink sent NACK byte */
	BHDM_P_Mhl_CbusCmdState_eAborted    = 9,  /* sink sent ABORT to us */
	BHDM_P_Mhl_CbusCmdState_eTimeout    = 10  /* timeout */
} BHDM_P_Mhl_CbusCmdState;

/* command type, this is used to help facilitate removal
   of certain command type from a queue */
#define BHDM_P_MHL_CBUS_CMD_CHAN_MASK 0xF0
#define BHDM_P_MHL_CBUS_CMD_CHAN_MSC  0x00
#define BHDM_P_MHL_CBUS_CMD_CHAN_DDC  0x10

typedef enum
{
	BHDM_P_Mhl_CbusCmdType_eMsc  = BHDM_P_MHL_CBUS_CMD_CHAN_MSC | 0x01, /* MSC general commands */
	BHDM_P_Mhl_CbusCmdType_eRcp  = BHDM_P_MHL_CBUS_CMD_CHAN_MSC | 0x02, /* MSC commands related RCP */
	BHDM_P_Mhl_CbusCmdType_eDcap = BHDM_P_MHL_CBUS_CMD_CHAN_MSC | 0x03, /* MSC READ_DEVCAP */
	BHDM_P_Mhl_CbusCmdType_eHb   = BHDM_P_MHL_CBUS_CMD_CHAN_MSC | 0x04, /* MSC heartbeat */
	BHDM_P_Mhl_CbusCmdType_eDdc  = BHDM_P_MHL_CBUS_CMD_CHAN_DDC | 0x01, /* DDC general commands */
	BHDM_P_Mhl_CbusCmdType_eEdid = BHDM_P_MHL_CBUS_CMD_CHAN_DDC | 0x02, /* DDC commands related to EDID */
	BHDM_P_Mhl_CbusCmdType_eHdcp = BHDM_P_MHL_CBUS_CMD_CHAN_DDC | 0x03  /* DDC commands related to HDCP */
} BHDM_P_Mhl_CbusCmdType;

typedef enum
{
	BHDM_P_Mhl_CbusCmdQueueType_eShort,
	BHDM_P_Mhl_CbusCmdQueueType_eLong,
	BHDM_P_Mhl_CbusCmdQueueType_eUnknown
} BHDM_P_Mhl_CbusCmdQueueType;

/* Translation Layer command structure
   We also use this structure for incoming requests.
   Incoming request is always one message,
   pucReplyBuf is always NULL,
   ulNumPacketsCfg is always going to be the same as ulNumPacketsDone,
   and bLastCmd is always going to be true.
   The state is always going to be BHDM_P_Mhl_CbusCmdState_eIncoming
   unless there is an error.
*/
typedef struct BHDM_P_Mhl_CbusCmd
{
	uint8_t                 *pucReplyBuf;         /* If this is non-NULL, any reply will be
													 copied to here, supplied by caller */
	uint8_t                  ucReplyBufSize;      /* Max. size of the return buffer */
	uint8_t                  ucReplyBufValidSize; /* Actual no. of bytes returned */

	/* We bit pack some of the following fields to reduce storage */
	uint8_t                  ucDelay;  /* Delay to send command in ticks */
	BHDM_P_Mhl_CbusCmdType	 eCmdType; /* command type */

    uint32_t                 ulNumPacketsCfg;  /* No. of packets configured. This is also used to determine
											      which free queue to recycle to */
	uint32_t                 ulNumPacketsDone; /* No. of packets processed by hardware */
	BHDM_P_Mhl_CbusCmdState  eState;           /* Command state */

	BHDM_P_Mhl_CbusPriority  ePriority;        /* Command priority */
	BHDM_P_Mhl_CbusDest      eDest;            /* Destination FIFO */
	bool                     bLastCmd;        /* Last command in a sequence of commands */

	union
	{
		BHDM_P_Mhl_CbusPkt   astLongCmd[BHDM_P_MHL_CBUS_LONG_CMD_LENGTH];
		BHDM_P_Mhl_CbusPkt   astShortCmd[BHDM_P_MHL_CBUS_SHORT_CMD_LENGTH];
	} cbusPackets;
} BHDM_P_Mhl_CbusCmd;

/* CBUS command queue
   A queue of the commands above.
 */

typedef struct BHDM_P_Mhl_Circular_Fifo BHDM_P_Mhl_CmdQueue;

/* When a transmission has completed, with error or not,
   the caller which queues the request will receive this
   callback.

   Arguments:
   Xmit result as above
   Private data (as passed in by user)
   Pointer to the current command which has just completed
   abort_required is true if an ABORT is due
*/
typedef void (*BHDM_P_Mhl_CbusCmdCb)(BERR_Code result,
									      void *pvPrivate,
									      BHDM_P_Mhl_CbusCmd *pCmd,
									      bool bAbortRequired);



/* Function to compare two commands to be passed to the remove function
   cmd1 is the command from the queue
   cmd2 is the command to be compare against
   private is some private state if required
   If this function returns true, then the command will be removed
   from the queue */

typedef bool (*BHDM_P_Mhl_CmdCompare)( const BHDM_P_Mhl_CbusCmd *pCmd1,
											const BHDM_P_Mhl_CbusCmd *pCmd2,
											void *pvPrivate );

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_CBUS_CMD_PRIV_H__ */
