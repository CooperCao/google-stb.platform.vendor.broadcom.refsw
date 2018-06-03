/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Further modified by Broadcom Corp, Jan 25 2017
 */

#ifndef __CSS_SCMI_PRIVATE_H__
#define __CSS_SCMI_PRIVATE_H__

/*
 * SCMI power domain management protocol message and response lengths. It is
 * calculated as sum of length in bytes of the message header (4) and payload
 * area (the number of bytes of parameters or return values in the payload).
 */
#define SCMI_PROTO_VERSION_MSG_LEN		4
#define SCMI_PROTO_VERSION_RESP_LEN		12

#define SCMI_PROTO_MSG_ATTR_MSG_LEN		8
#define SCMI_PROTO_MSG_ATTR_RESP_LEN		12

#define SCMI_PWR_STATE_SET_MSG_LEN		16
#define SCMI_PWR_STATE_SET_RESP_LEN		8

#define SCMI_PWR_STATE_GET_MSG_LEN		8
#define SCMI_PWR_STATE_GET_RESP_LEN		12

#define SCMI_SYS_PWR_STATE_SET_MSG_LEN		12
#define SCMI_SYS_PWR_STATE_SET_RESP_LEN		8

#define SCMI_SYS_PWR_STATE_GET_MSG_LEN		4
#define SCMI_SYS_PWR_STATE_GET_RESP_LEN		12

/* SCMI message header format bit field */
#define SCMI_MSG_ID_SHIFT		0
#define SCMI_MSG_ID_WIDTH		8
#define SCMI_MSG_ID_MASK		((1 << SCMI_MSG_ID_WIDTH) - 1)

#define SCMI_MSG_TYPE_SHIFT		8
#define SCMI_MSG_TYPE_WIDTH		2
#define SCMI_MSG_TYPE_MASK		((1 << SCMI_MSG_TYPE_WIDTH) - 1)

#define SCMI_MSG_PROTO_ID_SHIFT		10
#define SCMI_MSG_PROTO_ID_WIDTH		8
#define SCMI_MSG_PROTO_ID_MASK		((1 << SCMI_MSG_PROTO_ID_WIDTH) - 1)

#define SCMI_MSG_TOKEN_SHIFT		18
#define SCMI_MSG_TOKEN_WIDTH		10
#define SCMI_MSG_TOKEN_MASK		((1 << SCMI_MSG_TOKEN_WIDTH) - 1)


/* SCMI mailbox flags */
#define SCMI_FLAG_RESP_POLL	0
#define SCMI_FLAG_RESP_INT	1

/* SCMI power domain protocol `POWER_STATE_SET` message flags */
#define SCMI_PWR_STATE_SET_FLAG_SYNC	0
#define SCMI_PWR_STATE_SET_FLAG_ASYNC	1

/*
 * Helper macro to create an SCMI message header given protocol, message id
 * and token.
 */
#define SCMI_MSG_CREATE(protocol, msg_id, token)				\
	((((protocol) & SCMI_MSG_PROTO_ID_MASK) << SCMI_MSG_PROTO_ID_SHIFT) |	\
	(((msg_id) & SCMI_MSG_ID_MASK) << SCMI_MSG_ID_SHIFT) |			\
	(((token) & SCMI_MSG_TOKEN_MASK) << SCMI_MSG_TOKEN_SHIFT))

/* Helper macro to get the token from a SCMI message header */
#define SCMI_MSG_GET_TOKEN(msg)				\
	(((msg) >> SCMI_MSG_TOKEN_SHIFT) & SCMI_MSG_TOKEN_MASK)

/* Helper macro to get the id from a SCMI message header */
#define SCMI_MSG_GET_ID(msg)				\
	(((msg) >> SCMI_MSG_ID_SHIFT) & SCMI_MSG_ID_MASK)

/* Helper macro to get the proto from a SCMI message header */
#define SCMI_MSG_GET_PROTO(msg)				\
	(((msg) >> SCMI_MSG_PROTO_ID_SHIFT) & SCMI_MSG_PROTO_ID_MASK)

/* Helper macro to get the type from a SCMI message header */
#define SCMI_MSG_GET_TYPE(msg)				\
	(((msg) >> SCMI_MSG_TYPE_SHIFT) & SCMI_MSG_TYPE_MASK)

/* SCMI Channel Status bit fields */
#define SCMI_CH_STATUS_RES0_MASK	0xFFFFFFFE
#define SCMI_CH_STATUS_FREE_SHIFT	0
#define SCMI_CH_STATUS_FREE_WIDTH	1
#define SCMI_CH_STATUS_FREE_MASK	((1 << SCMI_CH_STATUS_FREE_WIDTH) - 1)

/* Helper macros to check and write the channel status */
#define SCMI_IS_CHANNEL_FREE(status)					\
	(!!(((status) >> SCMI_CH_STATUS_FREE_SHIFT) & SCMI_CH_STATUS_FREE_MASK))

#define SCMI_MARK_CHANNEL_BUSY(status) do {				\
		assert(SCMI_IS_CHANNEL_FREE(status));			\
		(status) &= ~(SCMI_CH_STATUS_FREE_MASK <<		\
				SCMI_CH_STATUS_FREE_SHIFT);		\
	} while (0)

/* Helper macros to copy arguments to the mailbox payload */
#define SCMI_PAYLOAD_ARG1(payld_arr, arg1)				\
		iowrite32(arg1, &payld_arr[0])

#define SCMI_PAYLOAD_ARG2(payld_arr, arg1, arg2) do {			\
		SCMI_PAYLOAD_ARG1(payld_arr, arg1);			\
		iowrite32(arg2, &payld_arr[1]);				\
	} while (0)

#define SCMI_PAYLOAD_ARG3(payld_arr, arg1, arg2, arg3) do {		\
		SCMI_PAYLOAD_ARG2(payld_arr, arg1, arg2);		\
		iowrite32(arg3, &payld_arr[2]);				\
	} while (0)

#define SCMI_PAYLOAD_ARG4(payld_arr, arg1, arg2, arg3, arg4) do {	\
		SCMI_PAYLOAD_ARG3(payld_arr, arg1, arg2, arg3);		\
		iowrite32(arg4, &payld_arr[3]);				\
	} while (0)

#define SCMI_PAYLOAD_ARG5(payld_arr, arg1, arg2, arg3, arg4, arg5) do {	\
		SCMI_PAYLOAD_ARG4(payld_arr, arg1, arg2, arg3, arg4);	\
		iowrite32(arg5, &payld_arr[4]);				\
	} while (0)

/* Helper macros to read return values from the mailbox payload */
#define SCMI_PAYLOAD_RET_VAL1(payld_arr, val1)				\
		(val1) = ioread32(&payld_arr[0])

#define SCMI_PAYLOAD_RET_VAL2(payld_arr, val1, val2) do {		\
		SCMI_PAYLOAD_RET_VAL1(payld_arr, val1);			\
		(val2) = ioread32(&payld_arr[1]);			\
	} while (0)

#define SCMI_PAYLOAD_RET_VAL3(payld_arr, val1, val2, val3) do {		\
		SCMI_PAYLOAD_RET_VAL2(payld_arr, val1, val2);		\
		(val3) = ioread32(&payld_arr[2]);			\
	} while (0)

/* Helper macro to ring doorbell */
#define SCMI_RING_DOORBELL(addr, modify_mask, preserve_mask) do {	\
		uint32_t db = ioread32(addr) & (preserve_mask);		\
		iowrite32(db | (modify_mask), addr);			\
	} while (0)

/*
 * Private data structure for representing the mailbox memory layout. Refer
 * the SCMI specification for more details.
 */
typedef struct mailbox_mem {
	uint32_t res_a; /* Reserved */
	volatile uint32_t status;
	uint64_t res_b; /* Reserved */
	uint32_t flags;
	volatile uint32_t len;
	uint32_t msg_header;
	uint32_t payload[];
} mailbox_mem_t;

/* SCMI Error codes */
#define SCMI_SUCCESS		0	/* Success */
#define SCMI_ERR_SUPPORT	-1	/* Not supported */
#define SCMI_ERR_PARAMS		-2	/* Invalid Parameters */
#define SCMI_ERR_ACCESS		-3	/* Invalid access/permission denied */
#define SCMI_ERR_ENTRY		-4	/* Not found */
#define SCMI_ERR_RANGE		-5	/* Value out of range */
#define SCMI_ERR_BUSY		-6	/* Device busy */
#define SCMI_ERR_COMMS		-7	/* Communication Error */
#define SCMI_ERR_GENERIC	-8	/* Generic Error */
#define SCMI_ERR_HARDWARE	-9	/* Hardware Error */
#define SCMI_ERR_PROTOCOL	-10	/* Protocol Error */

/* SCMI Protocols */
#define SCMI_PROTOCOL_BASE	0x10
#define	SCMI_PROTOCOL_POWER	0x11
#define	SCMI_PROTOCOL_SYSTEM	0x12
#define	SCMI_PROTOCOL_PERF	0x13
#define	SCMI_PROTOCOL_CLOCK	0x14
#define	SCMI_PROTOCOL_SENSOR	0x15
#define	SCMI_PROTOCOL_BRCM	0x80

/* SCMI Message IDs common to all Protocols */
#define PROTOCOL_VERSION		0x0
#define PROTOCOL_ATTRIBUTES		0x1
#define PROTOCOL_MESSAGE_ATTRIBUTES	0x2

/* SCMI Base Protocol Message IDs */
#define BASE_DISCOVER_VENDOR		0x3
#define BASE_DISCOVER_SUB_VENDOR	0x4
#define BASE_DISCOVER_IMPLEMENT_VERSION	0x5
#define BASE_DISCOVER_LIST_PROTOCOLS	0x6
#define BASE_DISCOVER_AGENT		0x7
#define BASE_NOTIFY_ERRORS		0x8

/* SCMI Performance Protocol Message IDs */
#define PERF_DOMAIN_ATTRIBUTES	0x3
#define PERF_DESCRIBE_LEVELS	0x4
#define PERF_LIMITS_SET		0x5
#define PERF_LIMITS_GET		0x6
#define PERF_LEVEL_SET		0x7
#define PERF_LEVEL_GET		0x8
#define PERF_NOTIFY_LIMITS	0x9
#define PERF_NOTIFY_LEVEL	0xa

/* SCMI Sensor Protocol Message IDs */
#define SENS_DESC_GET		0x3
#define SENS_TRIP_PT_NOTIFY	0x4
#define SENS_TRIP_PT_CONFIG	0x5
#define SENS_READ_GET		0x6
#define SENS_TRIP_PT_EVENT	0x7

/* SCMI Brcm Protocol Message IDs */
#define BRCM_SEND_AVS_CMD	0x3


/* SCMI Perf attritbutes */
#define PERF_ATTR_PWR_IN_MW		(1 << 16)

/* SCMI Perf domain attritbutes */
#define PERF_DOM_ATTR_SET_LIMIT		(1 << 31)
#define PERF_DOM_ATTR_SET_PERF		(1 << 30)
#define PERF_DOM_ATTR_LIMIT_NOTIFY	(1 << 29)
#define PERF_DOM_ATTR_CHANGE_NOTIFY	(1 << 28)

#define SCMI_ARG_SZ			4
#define SCMI_STRING_SZ			16


#endif	/* __CSS_SCMI_PRIVATE_H__ */
