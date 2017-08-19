/*
 *  USB specific types.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#ifndef _USBTYPES_H_
#define _USBTYPES_H_

extern uint32 bdc_msg_level;	/**< Print messages even for non-debug drivers */

#define BDC_ERROR_VAL	0x0001
#define BDC_TRACE_VAL	0x0002
#define BDC_DBG_VAL	0x0004
#define BDC_DBG_LTSSM	0x0008
#define BDC_DBG_FUNCTION 0x0010

#if defined(BCMDBG) || defined(BDC_DEBUG)
#define bdc_err(args) do {if (bdc_msg_level & BDC_ERROR_VAL) printf args;} while (0)
#define bdc_trace(args)	do {if (bdc_msg_level & BDC_TRACE_VAL) printf args;} while (0)
#define bdc_dbg(args)	do {if (bdc_msg_level & BDC_DBG_VAL) printf args;} while (0)
#define bdc_fn(fname)	do {if (bdc_msg_level & BDC_DBG_FUNCTION) printf fname;} while (0)
#else
#define bdc_err(args) do {if (bdc_msg_level & BDC_ERROR_VAL) printf args;} while (0)
#define bdc_trace(args)
#define bdc_dbg(args)
#define bdc_fn(fname)
#endif	/* BCMDBG */

#define bdc_dbg_ltssm(args) do {if (bdc_msg_level & BDC_DBG_LTSSM) printf args;} while (0)

typedef enum _status_code {
	BDC_STATUS_SUCCESS,
	BDC_STATUS_PENDING,
	BDC_STATUS_UNKNOWN,
	BDC_STATUS_IMPLEMENTED_BY_SUBCLASS,
	BDC_STATUS_INVALID_FIELD,
	BDC_STATUS_DATA_READY,
	BDC_STATUS_BUS_RESET,
	BDC_STATUS_NOT_USED,
	BDC_STATUS_FAILED
} bdc_status_code;

#endif /* _USBTYPES_H_ */
