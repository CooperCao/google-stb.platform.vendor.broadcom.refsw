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
#ifndef BHDM_MHL_COMMON_PRIV_H__
#define BHDM_MHL_COMMON_PRIV_H__

#include "bhdm_mhl.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Generic set/get/compare field
 ***************************************************************************/
/* Get register field value by register name and field name.
 *
 * This is equivalent to:
 * ulFoo = (regvar & BCHP_MT_CBUS_xyz_MASK) >>
 *    BCHP_MT_CBUS_xyz_SHIFT);
 *
 * Example:
 *   ulReg = BREG_Read32(hRegister, BCHP_MT_CBUS);
 *   BHDM_P_MHL_GET_FIELD(ulReg, MT_CBUS, xyz);
 *   BREG_Write32(BCHP_MT_CBUS, &ulReg).
 *
 */
#define BHDM_P_MHL_GET_REG_FIELD(regvar, reg, field) \
	(((regvar) & BCHP_##reg##_##field##_MASK) >> \
	BCHP_##reg##_##field##_SHIFT)

/* Compare a register field by value or name.
 * Example:
 *   ...
 *   ulReg = BREG_Read32(hRegister, BCHP_MT_CBUS);
 *   if(BHDM_P_MHL_COMPARE_FIELD_NAME(ulReg, MT_CBUS, xyxz 0))
 *   {
 *      BDBG_MSG("Input video is NTSC");
 *   }
 *or
 *   if(BHDM_P_MHL_COMPARE_FIELD_NAME(ulReg, MT_CBUS, xyz, 1))
 *   {
 *      BDBG_MSG("xyz is 1");
 *   }
 */
#define BHDM_P_MHL_COMPARE_FIELD_DATA(regvar, reg, field, data) \
	(BHDM_P_MHL_GET_FIELD((regvar), reg, field)==(data))

#define BHDM_P_MHL_COMPARE_FIELD_NAME(regvar, reg, field, name) \
	(BHDM_P_MHL_GET_FIELD((regvar), reg, field)==BCHP##_##reg##_##field##_##name)

/* utility macro: to calculate how many entries from "start" register
   to "end" register inclusively. */
#define BHDM_P_MHL_REGS_ENTRIES(start, end)    \
	((((BCHP##_##end) - (BCHP##_##start)) / sizeof(uint32_t)) + 1)

/* number of registers in one block. */
#define BHDM_P_MHL_CBUS_REGS_COUNT      \
	BHDM_P_MHL_REGS_ENTRIES(CBUS_REG_START, CBUS_REG_END)

#define BHDM_P_MHL_SET_FIELD(val, mask, lsb, a) \
	do { \
		val &= ~mask; \
		a &= (mask>>lsb);\
		val |= (a<<lsb);\
	} while (0);

#define BHDM_P_MHL_GET_FIELD(val, mask, lsb) \
	((val & mask)>>lsb)

/***************************************************************************
Summary:
	List of errors unique to BHDM_P_MHL
****************************************************************************/

#define BHDM_P_MHL_CBUS_SUCCESS 		     BERR_SUCCESS
#define BHDM_P_MHL_CBUS_ERROR                BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 1)
#define BHDM_P_MHL_CBUS_NOMEMORY             BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 2)  /* Out of memory */
#define BHDM_P_MHL_CBUS_UNEXPECTED_CMD       BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 3)  /* Unexpected incoming command */
#define BHDM_P_MHL_CBUS_NOTFOUND             BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 4)  /* Does not found a matching command */
#define BHDM_P_MHL_CBUS_INVALID              BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 5)  /* Invalid argument */
#define BHDM_P_MHL_CBUS_ABORT                BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 6)  /* Abort received */
#define BHDM_P_MHL_CBUS_DISCONNECTED         BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 7)  /* Disconnect received */
#define BHDM_P_MHL_CBUS_XMIT_SUCCESS         BERR_SUCCESS                                    /* Transmission completed normally */
#define BHDM_P_MHL_CBUS_XMIT_TIMEOUT         BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 8)  /* Transmission timeout */
#define BHDM_P_MHL_CBUS_XMIT_NACK            BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 9)  /* NACK byte */
#define BHDM_P_MHL_CBUS_XMIT_PROTO_ERROR     BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 10) /* unexpected packet */
#define BHDM_P_MHL_CBUS_XMIT_CANCELLED       BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 11) /* Cancelled */
#define BHDM_P_MHL_CBUS_XMIT_BUS_STOLEN      BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 12) /* Bus stolen, must cancel and then retry */
#define BHDM_P_MHL_CBUS_XMIT_RETRY_EXCEEDED  BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 13) /* Max retries exceeded */
#define BHDM_P_MHL_CBUS_XMIT_ERROR           BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 14) /* CBUS transmit error */
#define BHDM_P_MHL_HDCP_BKSV_INVALID         BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 15) /* HDCP Bksv is invalid */
#define BHDM_P_MHL_CMD_Q_FULL                BERR_MAKE_CODE(BERR_HDM_ID, BHDM_MHL_ERRS + 16) /* CBUS command queue is full */


#define	MAKE_MHL_CBUS_INTR_0_ENUM(IntName)	    BHDM_MHL_CBUS_INTR_0_e##IntName
#define	MAKE_MHL_CBUS_INTR_1_ENUM(IntName)	    BHDM_MHL_CBUS_INTR_1_e##IntName

/******************************************************************************
Summary:
Enumeration of MHL_CBUS_Interrupts
*******************************************************************************/
typedef enum
{
	/* CBUS Level 2 Interrupt Status 0 */
	/* Wake-up and discovery */
	/* 00 */ MAKE_MHL_CBUS_INTR_0_ENUM(DISCOVERY_SUCCEEDED),
	/* 01 */ MAKE_MHL_CBUS_INTR_0_ENUM(DISCOVERY_FAILED),
	/* 02 */ MAKE_MHL_CBUS_INTR_0_ENUM(DISCONNECT_RECEIVED),
	/* 03 */ MAKE_MHL_CBUS_INTR_0_ENUM(DISCONNECT_DONE),
	/* 04 */ MAKE_MHL_CBUS_INTR_0_ENUM(INITIATOR_PKT),
	/* 05 */ MAKE_MHL_CBUS_INTR_0_ENUM(FOLLOWER_PKT),
	/* 06 */ MAKE_MHL_CBUS_INTR_0_ENUM(FOLLOWER_DROP_PKT),
	/* 07 */ MAKE_MHL_CBUS_INTR_0_ENUM(SCHEDULER_DROP_PKT),
	/* 08 */ MAKE_MHL_CBUS_INTR_0_ENUM(IMP_CHANGED),
	/* 09 */ MAKE_MHL_CBUS_INTR_0_ENUM(LAST)
} BHDM_P_MHL_CbusInterrupt0;

typedef enum
{
	/* CBUS Level 2 Interrupt Status 1 */
	/* MSC Requester */
	/* 0 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_DONE),
	/* 1 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_MISMATCH),
	/* 2 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_CMD_ERROR),
	/* 3 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_TIMEOUT),
	/* 4 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_ILLEGAL_SW_WR),
	/* 5 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_MAX_RETRIES_EXCEEDED),
	/* 6 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_UNEXPECTED_INBOUND_PKT),
	/* 7 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_BUS_STOLEN),
	/* 8 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_CANCELLED),

	/* MSC Responder */
	/* 9 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_IB_DONE),
	/* 10 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_OB_DONE),
	/* 11 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_BAD_CMD),
	/* 12 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_UNEXPECTED_CMD),
	/* 13 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_DATA_RECEIVED),
	/* 14 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_DATA_OVERFLOW),
	/* 15 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_RX_TIMEOUT),
	/* 16 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_SW_TIMEOUT),
	/* 17 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_ILLEGAL_SW_WR),
	/* 18 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_MAX_RETRIES_EXCEEDED),
	/* 19 */ MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_UNEXPECTED_INBOUND_PKT),

	/* DDC Requester */
	/* 20 */ MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_DONE),
	/* 21 */ MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_RX_MISMATCH),
	/* 22 */ MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_RX_CMD_ERROR),
	/* 23 */ MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_RX_TIMEOUT),
	/* 24 */ MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_ILLEGAL_SW_WR),
	/* 25 */ MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_MAX_RETRIES_EXCEEDED),
	/* 26 */ MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_UNEXPECTED_INBOUND_PKT),

	/* ABORT */
	/* 27 */ MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_MSC_RECEIVED),
	/* 28 */ MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_DDC_RECEIVED),
	/* 29 */ MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_MSC_INBOUND_TIMEOUT_DONE),
	/* 30 */ MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_MSC_OUTBOUND_TIMEOUT_DONE),
	/* 31 */ MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_DDC_TIMEOUT_DONE),

	/* 32 */ MAKE_MHL_CBUS_INTR_1_ENUM(LAST)

} BHDM_P_MHL_CbusInterrupt1;

#define	MAKE_MHL_MPM_HOST_INTR_ENUM(IntName)	BHDM_MHL_MPM_HOST_INTR_e##IntName

/******************************************************************************
Summary:
Enumeration of MPM_HOST Interrupts
*******************************************************************************/
typedef enum
{
	/* MPM HOST Level 2 Interrupt Status */
	/* 00 */ MAKE_MHL_MPM_HOST_INTR_ENUM(MPM_INTR_SPARE0),
	/* 01 */ MAKE_MHL_MPM_HOST_INTR_ENUM(MPM_INTR_SPARE1),
	/* 02 */ MAKE_MHL_MPM_HOST_INTR_ENUM(MPM_INTR_SPARE2),
	/* 03 */ MAKE_MHL_MPM_HOST_INTR_ENUM(MPM_INTR_SPARE3),
	/* 04 */ MAKE_MHL_MPM_HOST_INTR_ENUM(MSPI_INTR_0),
	/* 05 */ MAKE_MHL_MPM_HOST_INTR_ENUM(MSPI_INTR_1),
	/* 06 */ MAKE_MHL_MPM_HOST_INTR_ENUM(A2R_TIMEOUT_INTR),
	/* 07 */ MAKE_MHL_MPM_HOST_INTR_ENUM(A2R_BAD_SIZE_INTR),
	/* 08 */ MAKE_MHL_MPM_HOST_INTR_ENUM(S3_TO_S0),
	/* 09 */ MAKE_MHL_MPM_HOST_INTR_ENUM(S0_TO_S3),
	/* 10 */ MAKE_MHL_MPM_HOST_INTR_ENUM(SW_INTR_0),
	/* 11 */ MAKE_MHL_MPM_HOST_INTR_ENUM(SW_INTR_1),
	/* 12 */ MAKE_MHL_MPM_HOST_INTR_ENUM(SW_INTR_2),
	/* 13 */ MAKE_MHL_MPM_HOST_INTR_ENUM(SW_INTR_3),
	/* 14 */ MAKE_MHL_MPM_HOST_INTR_ENUM(SW_INTR_4),
	/* 15 */ MAKE_MHL_MPM_HOST_INTR_ENUM(SW_INTR_5),
	/* 16 */ MAKE_MHL_MPM_HOST_INTR_ENUM(LAST)
} BHDM_P_MHL_MpmHostInterrupt;

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_COMMON_PRIV_H__ */
