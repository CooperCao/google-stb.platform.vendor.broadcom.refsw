/*
 * Dongle BUS interface
 * SDIO NDIS Implementation
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <ntddsd.h>
#include "dbus.h"
#include <sdio.h>
#include <sbsdio.h>
#include <bcmsdh.h>
#include <siutils.h>
#include <bcmsrom.h>

#define SDIOH_MODE_SD1		1
#define SDIOH_MODE_SD4		2

#define SDIO_BUS_WIDTH_1	1
#define SDIO_BUS_WIDTH_4	4

#define BLOCK_TRANSFER_LENGTH_FUNC_0	64
#define BLOCK_TRANSFER_LENGTH_FUNC_1	64
#define BLOCK_TRANSFER_LENGTH_FUNC_2	64

#define SDIO_BUS_CLOCK_KHZ	50000		/* 50 Mhz */
#define SDIO_DIVISOR_DEFAULT	1

#define TOTAL_FUNCS	3

uint sd_divisor = SDIO_DIVISOR_DEFAULT;

typedef struct {
	dbus_pub_t			*pub; /* Must be first */

	void				*cbarg;
	dbus_intf_callbacks_t		*cbs;

	NDIS_HANDLE			AdapterHandle;

	SDBUS_INTERFACE_STANDARD	bus_intf;	/* os bus interface */

	uint16				bus_drvr_ver;	/* bus driver version */
	uint16				func_type; /* function type */
	uint8				func_num; /* function number */
	uint8				bus_width;		/* bus width */
	uint32				bus_clock_khz;	/* clock speed */
	uint8				bus_control;	/* bus control */
	uint32				sd_divisor; /* Bus clock divisor */
	bool				high_speed_enabled;

	bool				block_mode; /* use block mode */
	uint16				host_block_sz; /* def block size */
	uint16				block_sz[TOTAL_FUNCS];

	bool				fn2_enabled; /* set if function 2 enabled */
	uint32				sbwad;	/* back plane address window */
	uint				fn1_cis_ptr;	/* function 1 cis pointer */

	NDIS_SPIN_LOCK			lock;
	NDIS_SPIN_LOCK			rxlock;
	NDIS_SPIN_LOCK			txlock;
	shared_sem_t			iflock;
	bool				lock_init;

	NDIS_EVENT			irqevent;
	HANDLE				irqthread;
	bool				irqhalt;

} sdos_info_t;



/*
 * SDIO ndis dbus_intf_t
 */
static void * dbus_sdos_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs);
static void dbus_sdos_detach(dbus_pub_t *pub, void *info);
static int dbus_sdos_send_irb(void *bus, dbus_irb_tx_t *txirb);
static int dbus_sdos_cancel_irb(void *bus, dbus_irb_tx_t *txirb);
static int dbus_sdos_get_attrib(void *bus, dbus_attrib_t *attrib);
int dbus_sdos_pnp(void *bus, int event);
static int dbus_sdos_up(void *bus);
static int dbus_sdos_down(void *bus);
static int dbus_sdos_stop(void *bus);
static bool dbus_sdos_recv_needed(void *bus);
static void *dbus_sdos_exec_rxlock(void *bus, exec_cb_t cb, struct exec_parms *args);
static void *dbus_sdos_exec_txlock(void *bus, exec_cb_t cb, struct exec_parms *args);
static int dbus_sdos_lock(void *bus);
static int dbus_sdos_unlock(void *bus);

static dbus_intf_t dbus_sdos_intf = {
	dbus_sdos_attach,
	dbus_sdos_detach,
	dbus_sdos_up,
	dbus_sdos_down,
	dbus_sdos_send_irb,
	NULL,
	dbus_sdos_cancel_irb,
	NULL,
	NULL,
	NULL, /* get_stats */
	NULL, /* get_attrib */
	dbus_sdos_pnp,
	NULL,
	NULL, /* resume */
	NULL, /* suspend */
	dbus_sdos_stop,
	NULL, /* reset */
	NULL, /* pktget */
	NULL, /* pktfree */
	NULL, /* iovar_op */
	NULL, /* dump */
	NULL, /* set_config */
	NULL, /* get_config */
	NULL,
	NULL,
	NULL,
	NULL,
	dbus_sdos_recv_needed,
	dbus_sdos_exec_rxlock,
	dbus_sdos_exec_txlock,
	NULL, /* tx_timer_init */
	NULL, /* tx_timer_start */
	NULL, /* tx_timer_stop */
	NULL,
	dbus_sdos_lock,
	dbus_sdos_unlock,
	NULL,
	NULL, /* shutdown */
	NULL, /* recv_stop */
	NULL  /* recv_resume */
};

static probe_cb_t probe_cb = NULL;
static disconnect_cb_t disconnect_cb = NULL;
static void *probe_arg = NULL;
static void *disc_arg = NULL;

#define LOCK_INIT(lock)		NdisAllocateSpinLock(lock)
#define LOCK(lock)		NdisAcquireSpinLock(lock)
#define UNLOCK(lock)		NdisReleaseSpinLock(lock)
#define LOCK_FREE(lock)		NdisFreeSpinLock(lock)

#define	IFLOCK_INIT(lock)	shared_sem_init(lock, 1)
#define IFLOCK(lock)		shared_sem_acquire(lock)
#define IFUNLOCK(lock)		shared_sem_release(lock)
#define	IFLOCK_FREE(lock)	shared_sem_free(lock)

#define	EVENT_INIT(e)		NdisInitializeEvent(e)
#define EVENT_WAIT(e, ms)	NdisWaitEvent(e, ms)
#define EVENT_CLEAR(e)		NdisResetEvent(e)
#define EVENT_SIGNAL(e)		NdisSetEvent(e)
#define	EVENT_FREE(e)

static int
dbus_sdos_probe()
{
	DBUSTRACE(("%s:\n", __FUNCTION__));

	if (probe_cb) {
		disc_arg = probe_cb(probe_arg, "", SDIO_BUS, 0);
		if (disc_arg == NULL) {
			DBUSERR(("g_probe_cb return NULL\n"));
			return DBUS_ERR;
		}
	}

	return DBUS_OK;
}

int
dbus_bus_osl_register(int vid, int pid, probe_cb_t prcb,
	disconnect_cb_t discb, void *prarg, dbus_intf_t **intf, void *param1, void *param2)
{
	probe_cb = prcb;
	probe_arg = prarg;
	disconnect_cb = discb;
	*intf = &dbus_sdos_intf;

	return dbus_sdos_probe();
}

int
dbus_bus_osl_deregister()
{
	if (disconnect_cb)
		disconnect_cb(disc_arg);
	return DBUS_OK;
}

static NTSTATUS
dbus_sdos_property(sdos_info_t *sdos_info, SDBUS_PROPERTY Property, bool set, void *buf, uint len)
{
	SDBUS_REQUEST_PACKET sdrp;
	NTSTATUS status;

	bzero(&sdrp, sizeof(SDBUS_REQUEST_PACKET));
	sdrp.RequestFunction = set ? SDRF_SET_PROPERTY : SDRF_GET_PROPERTY;
	sdrp.Parameters.GetSetProperty.Property = Property;
	sdrp.Parameters.GetSetProperty.Buffer = buf;
	sdrp.Parameters.GetSetProperty.Length = len;

	status = SdBusSubmitRequest(sdos_info->bus_intf.Context, &sdrp);

	return status;
}

int dbus_sdos_bcme_error(NTSTATUS status)
{
	switch (status) {
		case STATUS_SUCCESS:
			return BCME_OK;
		case STATUS_PENDING:
			return BCME_PENDING;
		case STATUS_DEVICE_REMOVED:
			return BCME_NODEVICE;
	}

	return BCME_SDIO_ERROR;
}

static int
dbus_sdos_cmd52_rw(sdos_info_t *sdos_info, bool read, uint func_num, uint addr, uint8 *buf, int len)
{
	SDBUS_REQUEST_PACKET sdrp;
	SD_RW_DIRECT_ARGUMENT rw_arg;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	int i;
	const SDCMD_DESCRIPTOR ReadIoDirectDesc =
		{SDCMD_IO_RW_DIRECT, SDCC_STANDARD, SDTD_READ, SDTT_CMD_ONLY, SDRT_5};
	const SDCMD_DESCRIPTOR WriteIoDirectDesc =
		{SDCMD_IO_RW_DIRECT, SDCC_STANDARD, SDTD_WRITE, SDTT_CMD_ONLY, SDRT_5};

#ifdef BCMDBG
	if (read)
		DBUSINFO(("%s: r: func %d, addr 0x%x, buffer 0x%8.8lx\n",
			__FUNCTION__, func_num, addr, (ULONG_PTR)buf));
	else
		DBUSINFO(("%s: w: func %d, addr 0x%x, buffer 0x%8.8lx, data 0x%x\n",
			__FUNCTION__, func_num, addr, (ULONG_PTR)buf,
			len == 1 ? *buf : (len == 2 ? *(uint16*)buf : *(uint32*)buf)));
#endif /* BCMDBG */

	if (len == 0)
		return BCME_OK;

	/* start building request */
	bzero(&sdrp, sizeof(SDBUS_REQUEST_PACKET));

	/* start building request */
	sdrp.RequestFunction = SDRF_DEVICE_COMMAND;

	/* set request descriptor */
	sdrp.Parameters.DeviceCommand.CmdDesc = read ? ReadIoDirectDesc : WriteIoDirectDesc;

	for (i = 0; i < len; i++) {
		/* set up the argument and command descriptor */
		rw_arg.u.AsULONG = 0;
		rw_arg.u.bits.Address = addr + i;
		rw_arg.u.bits.Function = func_num;
		if (!read) {
			rw_arg.u.bits.Data = buf[i];
			rw_arg.u.bits.WriteToDevice = 1;
		}
		sdrp.Parameters.DeviceCommand.Argument = rw_arg.u.AsULONG;

		/* submit the request */
		status = SdBusSubmitRequest(sdos_info->bus_intf.Context, &sdrp);

		if (!NT_SUCCESS(status))
			break;
		else if (read) {
			/* for direct I/O data comes in the response */
			buf[i] = sdrp.ResponseData.AsUCHAR[0];
		}
	}

	return dbus_sdos_bcme_error(status);
}

static int
dbus_sdos_cmd53_rw(sdos_info_t *sdos_info, bool read, uint func_num, uint addr,
	bool byte_mode, bool fixed_addr, uint8 *buf, int len)
{
	SDBUS_REQUEST_PACKET sdrp;
	SD_RW_EXTENDED_ARGUMENT rw_arg;
	bool block_mode;
	NTSTATUS status;
	PMDL mdl;
	const SDCMD_DESCRIPTOR ReadIoExtendedDesc =
		{SDCMD_IO_RW_EXTENDED, SDCC_STANDARD, SDTD_READ, SDTT_SINGLE_BLOCK, SDRT_5};
	const SDCMD_DESCRIPTOR WriteIoExtendedDesc =
		{SDCMD_IO_RW_EXTENDED, SDCC_STANDARD, SDTD_WRITE, SDTT_SINGLE_BLOCK, SDRT_5};

#ifdef BCMDBG
	if (read)
		DBUSINFO(("%s: r: func %d, addr 0x%x, buf 0x%8.8lx, len %d, byte-mode %d, "
			"fixed-addr %d\n", __FUNCTION__, func_num, addr, (ULONG_PTR)buf, len,
			byte_mode, fixed_addr));
	else
		DBUSINFO(("%s: w: func %d, addr 0x%x, buf 0x%8.8lx, data 0x%x, len %d, "
			"byte-mode %d, fixed-addr %d\n",
			__FUNCTION__, func_num, addr, (ULONG_PTR)buf,
			(len == 1 ? *buf : (len == 2 ? *(uint16*)buf : *(uint32*)buf)),
			len, byte_mode, fixed_addr));
#endif /* BCMDBG */

	if (len == 0)
		return BCME_OK;

	mdl = NdisAllocateMdl(sdos_info->AdapterHandle, buf, len);
	if (mdl == NULL) {
		DBUSERR(("%s: NdisAllocateMdl failed\n", __FUNCTION__));
		return BCME_ERROR;
	}
	bzero(&sdrp, sizeof(SDBUS_REQUEST_PACKET));

	/* start building request */
	sdrp.RequestFunction = SDRF_DEVICE_COMMAND;

	/* get block mode value */
	block_mode = (!sdos_info->block_mode || byte_mode) ? 0 : 1;

	/* set up the argument and command descriptor */
	rw_arg.u.AsULONG = 0;
	rw_arg.u.bits.Address = addr;
	rw_arg.u.bits.Function = func_num;
	rw_arg.u.bits.OpCode = fixed_addr ? 0 : 1;	/* increment address */
	rw_arg.u.bits.BlockMode = block_mode;
	if (!read)
		rw_arg.u.bits.WriteToDevice = 1;
	sdrp.Parameters.DeviceCommand.Argument = rw_arg.u.AsULONG;

	sdrp.Parameters.DeviceCommand.CmdDesc =
		read ? ReadIoExtendedDesc : WriteIoExtendedDesc;

	if (block_mode)
		sdrp.Parameters.DeviceCommand.CmdDesc.TransferType =
			SDTT_MULTI_BLOCK_NO_CMD12;

	/* set buf info */
	sdrp.Parameters.DeviceCommand.Mdl = mdl;
	sdrp.Parameters.DeviceCommand.Length = len;

	status = SdBusSubmitRequest(sdos_info->bus_intf.Context, &sdrp);

	NdisFreeMdl(mdl);

	return dbus_sdos_bcme_error(status);
}

/* read/write config register in particular function */
uint8
dbus_sdos_cfg_read(sdos_info_t *sdos_info, uint func_num, uint32 addr, int *err)
{
	uint8 data = 0xff;
	uint status;

	ASSERT(func_num <= SBSDIO_NUM_FUNCTION);

	status = dbus_sdos_cmd52_rw(sdos_info, TRUE, func_num, addr, &data, 1);
	if (err)
		*err = status;

	return data;
}

void
dbus_sdos_cfg_write(sdos_info_t *sdos_info, uint func_num, uint32 addr, uint8 data, int *err)
{
	uint status;

	ASSERT(func_num <= SBSDIO_NUM_FUNCTION);

	if ((func_num == 0) && (addr >= SBSDIO_CIS_BASE_COMMON)) {
		DBUSERR(("Write abort: sdio func0 CIS is write-protected\n"));
		if (err)
			*err = BCME_ERROR;
		return;
	}

	status = dbus_sdos_cmd52_rw(sdos_info, FALSE, func_num, addr, &data, 1);
	if (err)
		*err = (int)status;
}
int
dbus_sdos_get_sbaddr_window(sdos_info_t *sdos_info)
{
	return sdos_info->sbwad;
}

int
dbus_sdos_set_sbaddr_window(sdos_info_t *sdos_info, uint32 address, bool force_set)
{
	int err = 0;
	uint bar0 = address & (~SBSDIO_SB_OFT_ADDR_MASK);

	if (bar0 != sdos_info->sbwad || force_set) {
		/* invalidate cached window var */
		sdos_info->sbwad = 0;

		dbus_sdos_cfg_write(sdos_info, SDIO_FUNC_1, SBSDIO_FUNC1_SBADDRLOW,
			(address >> 8) & SBSDIO_SBADDRLOW_MASK, &err);
		if (!err)
			dbus_sdos_cfg_write(sdos_info, SDIO_FUNC_1, SBSDIO_FUNC1_SBADDRMID,
				(address >> 16) & SBSDIO_SBADDRMID_MASK, &err);
		if (!err)
			dbus_sdos_cfg_write(sdos_info, SDIO_FUNC_1, SBSDIO_FUNC1_SBADDRHIGH,
				(address >> 24) & SBSDIO_SBADDRHIGH_MASK, &err);

		if (!err)
			sdos_info->sbwad = bar0;
	}

	return err;
}

/* access register in function 1 for 1/2/4 bytes */
uint32
dbus_sdos_reg_read(sdos_info_t *sdos_info, uint32 addr, uint size, uint32 *data)
{
	uint func_num = SDIO_FUNC_1;	/* chip registers are through function 1 */
	ALIGNED_LOCAL_VARIABLE(rval, SDALIGN)

	ASSERT(size == 2 || size == 4);

	if (!sdos_info)
		return (uint32)BCME_ERROR;

	IFLOCK(&sdos_info->iflock);

	/* need to rebase back plane address window */
	if (dbus_sdos_set_sbaddr_window(sdos_info, addr, FALSE)) {
		IFUNLOCK(&sdos_info->iflock);
		return (uint32)BCME_ERROR;
	}

	addr &= SBSDIO_SB_OFT_ADDR_MASK;

	if (size == 4)
		addr |= SBSDIO_SB_ACCESS_2_4B_FLAG;
	if (size == 1) {
		if (dbus_sdos_cmd52_rw(sdos_info, TRUE, func_num, addr,
			(uint8*)data, size) == BCME_OK) {
			IFUNLOCK(&sdos_info->iflock);
			return BCME_OK;
		}
	} else {
		if (dbus_sdos_cmd53_rw(sdos_info, TRUE, func_num, addr, TRUE,
			FALSE, rval, size) == BCME_OK) {
			*data = *((uint32 *)rval);
			IFUNLOCK(&sdos_info->iflock);
			return BCME_OK;
		}
	}

	IFUNLOCK(&sdos_info->iflock);
	return BCME_ERROR;
}

uint32
dbus_sdos_reg_write(sdos_info_t *sdos_info, uint32 addr, uint size, uint32 data)
{
	uint func_num = SDIO_FUNC_1;	/* chip registers are through function 1 */
	ALIGNED_LOCAL_VARIABLE(rval, SDALIGN)

	ASSERT(size == 2 || size == 4);

	if (!sdos_info)
		return (uint32)BCME_ERROR;

	IFLOCK(&sdos_info->iflock);

	/* need to rebase back plane address window */
	if (dbus_sdos_set_sbaddr_window(sdos_info, addr, FALSE)) {
		IFUNLOCK(&sdos_info->iflock);
		return (uint32)BCME_ERROR;
	}

	addr &= SBSDIO_SB_OFT_ADDR_MASK;

	if (size == 4)
		addr |= SBSDIO_SB_ACCESS_2_4B_FLAG;
	if (size == 1) {
		if (dbus_sdos_cmd52_rw(sdos_info, FALSE, func_num, addr,
			(uint8*)&data, size) == BCME_OK) {
			IFUNLOCK(&sdos_info->iflock);
			return BCME_OK;
		}
	} else {
		*((uint32 *)rval) = data;
		if (dbus_sdos_cmd53_rw(sdos_info, FALSE, func_num, addr, TRUE,
			FALSE, rval, size) == BCME_OK) {
			IFUNLOCK(&sdos_info->iflock);
			return BCME_OK;
		}
	}

	IFUNLOCK(&sdos_info->iflock);
	return BCME_ERROR;
}

int
dbus_sdos_recv_buf(sdos_info_t *sdos_info, uint32 addr, uint fn, uint flags,
	uint8 *buf, uint len)
{
	int ret;
	ASSERT(sdos_info);

	/* async not supported */
	if (flags & SDIO_REQ_ASYNC) {
		ASSERT(!(flags & SDIO_REQ_ASYNC));
		return BCME_UNSUPPORTED;
	}

	addr &= SBSDIO_SB_OFT_ADDR_MASK;

	if (flags & SDIO_REQ_4BYTE)
		addr |= SBSDIO_SB_ACCESS_2_4B_FLAG;

	ret = dbus_sdos_cmd53_rw(sdos_info, TRUE, fn, addr,
		(flags & SDIO_BYTE_MODE) ? TRUE : FALSE,
		(flags & SDIO_REQ_FIXED) ? TRUE : FALSE,
		buf, len);

	return ret;
}

int
dbus_sdos_send_buf(sdos_info_t *sdos_info, uint32 addr, uint fn, uint flags,
	uint8 *buf, uint len)
{
	int ret;
	ASSERT(sdos_info);

	/* async not supported */
	if (flags & SDIO_REQ_ASYNC) {
		ASSERT(!(flags & SDIO_REQ_ASYNC));
		return BCME_UNSUPPORTED;
	}

	addr &= SBSDIO_SB_OFT_ADDR_MASK;

	if (flags & SDIO_REQ_4BYTE)
		addr |= SBSDIO_SB_ACCESS_2_4B_FLAG;

	ret = dbus_sdos_cmd53_rw(sdos_info, FALSE, fn, addr,
		(flags & SDIO_BYTE_MODE) ? TRUE : FALSE,
		(flags & SDIO_REQ_FIXED) ? TRUE : FALSE,
		buf, len);

	return ret;
}

int
dbus_sdos_rwdata(sdos_info_t *sdos_info, uint rw, uint32 addr, uint8 *buf, uint len)
{
	uint flags;

	if (!sdos_info)
		return BCME_ERROR;

	addr |= SBSDIO_SB_ACCESS_2_4B_FLAG;
	return dbus_sdos_cmd53_rw(sdos_info, (rw ? FALSE : TRUE),
		SDIO_FUNC_1, addr, FALSE, FALSE, buf, len);
}

int
dbus_sdos_cis_read(sdos_info_t *sdos_info, uint func_num, uint8 *cis, uint32 length)
{
	uint addr;

	if (length > SBSDIO_CIS_SIZE_LIMIT || length == 0)
		return BCME_ERROR;

	if (func_num == SDIO_FUNC_0) {
		addr = SBSDIO_CIS_BASE_COMMON;
	} else if (func_num == SDIO_FUNC_1) {
		addr = sdos_info->fn1_cis_ptr;
	} else {
		ASSERT(func_num == SDIO_FUNC_0 || func_num == SDIO_FUNC_1);
		return BCME_ERROR;
	}

	return dbus_sdos_cmd52_rw(sdos_info, TRUE, SDIO_FUNC_0, addr, cis, length);
}

static void dbus_sdos_intr_handler(void *context)
{
	KPRIORITY priority;
	sdos_info_t *sdos_info = (sdos_info_t *)context;

	priority =  KeSetPriorityThread(KeGetCurrentThread(), HIGH_PRIORITY);

	while (!sdos_info->irqhalt) {

		EVENT_WAIT(&sdos_info->irqevent, 0);

		if (sdos_info->irqhalt)
			break;

		if (sdos_info->cbs->dpc)
			sdos_info->cbs->dpc(sdos_info->cbarg, FALSE);

		EVENT_CLEAR(&sdos_info->irqevent);

		(sdos_info->bus_intf.AcknowledgeInterrupt)(sdos_info->bus_intf.Context);

	}

	sdos_info->irqhalt = FALSE;

	KeSetPriorityThread(KeGetCurrentThread(), priority);

	PsTerminateSystemThread(0);
}

int
dbus_sdos_intr_enable(sdos_info_t *sdos_info)
{
	int err;
	NTSTATUS status;
	uint8 val;
	BOOLEAN enable = TRUE;

	ASSERT(sdos_info);

	status = PsCreateSystemThread(&sdos_info->irqthread, GENERIC_ALL, NULL, NULL, NULL,
		dbus_sdos_intr_handler, sdos_info);
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: PsCreateSystemThread failed (0x%08X)\n", __FUNCTION__, status));
		return BCME_NORESOURCE;
	}

	val = dbus_sdos_cfg_read(sdos_info, SDIO_FUNC_0, SDIOD_CCCR_INTEN, &err);
	if (err == BCME_OK) {
		val |= INTR_CTL_MASTER_EN | INTR_CTL_FUNC1_EN | INTR_CTL_FUNC2_EN;
		dbus_sdos_cfg_write(sdos_info, SDIO_FUNC_0, SDIOD_CCCR_INTEN, val, &err);
	}

	if (err != BCME_OK) {
		DBUSERR(("%s: interrupt enable failed\n", __FUNCTION__));
		err = BCME_SDIO_ERROR;
	}

	dbus_sdos_property(sdos_info, SDP_FUNCTION_INT_ENABLE, TRUE, &enable, sizeof(BOOLEAN));

	return err;
}

int
dbus_sdos_intr_disable(sdos_info_t *sdos_info)
{
	int err;
	uint8 val;
	BOOLEAN enable = FALSE;

	ASSERT(sdos_info);

	dbus_sdos_property(sdos_info, SDP_FUNCTION_INT_ENABLE, TRUE, &enable, sizeof(BOOLEAN));

	dbus_sdos_cfg_write(sdos_info, SDIO_FUNC_0, SDIOD_CCCR_INTEN, 0, &err);

	if (err != BCME_OK) {
		DBUSERR(("%s: interrupt disable failed\n", __FUNCTION__));
		err = BCME_SDIO_ERROR;
	}

	sdos_info->irqhalt = TRUE;

	EVENT_SIGNAL(&sdos_info->irqevent);

	while (sdos_info->irqhalt)
		NdisMSleep(1);

	ZwClose(sdos_info->irqthread);

	return err;
}

/* read 24 bits and return valid 17 bit addr */
static uint
sd_get_cis_addr(sdos_info_t *sdos_info, uint32 addr, int *err)
{
	uint8 val;
	uint32 scratch = 0;
	uint8 *ptr = (uint8 *)&scratch;
	int i, status = BCME_OK;

	for (i = 0; i < 3; i++) {
		status = dbus_sdos_cmd52_rw(sdos_info, TRUE, SDIO_FUNC_0, addr, &val, 1);
		if (status != BCME_OK) {
			*err = status;
			DBUSERR(("%s: dbus_sdos_cmd52_rw failed\n", __FUNCTION__));
			return 0;
		}

		*ptr++ = val;
		addr++;
	}

	/* only the lower 17-bits are valid */
	scratch = ltoh32(scratch);
	scratch &= 0x0001ffff;
	*err = status;
	return scratch;
}

static NTSTATUS
dbus_sdos_card_init(sdos_info_t *sdos_info)
{
	uint8 io_en, bus_width, cap, clk, speed, io_rdy;
	uint8 *ptr;
	int func_num, err;
	NTSTATUS status = STATUS_UNSUCCESSFUL;


	speed = dbus_sdos_cfg_read(sdos_info, SDIO_FUNC_0, SDIOD_CCCR_SPEED_CONTROL, &err);
	if (err != BCME_OK) {
		DBUSERR(("%s: get SDIOD_CCCR_SPEED_CONTROL failed\n", __FUNCTION__));
		return status;
	}
	DBUSINFO(("%s: get SDIOD_CCCR_SPEED_CONTROL 0x%x\n", __FUNCTION__, speed));

	/* read card cap */
	cap = dbus_sdos_cfg_read(sdos_info, SDIO_FUNC_0, SDIOD_CCCR_CAPABLITIES, &err);
	if (err != BCME_OK) {
		DBUSERR(("%s: get SDIOD_CCCR_CAPABLITIES failed\n", __FUNCTION__));
		return status;
	}
	DBUSINFO(("%s: get SDIOD_CCCR_CAPABLITIES 0x%x\n", __FUNCTION__, cap));

	if (cap & SDIO_CAP_LSC) {
		if (cap & SDIO_CAP_4BLS) {
			bus_width = SDIO_BUS_WIDTH_4;
		}
		else {
			bus_width = SDIO_BUS_WIDTH_1;
		}

	}
	else {
		bus_width = SDIO_BUS_WIDTH_4;
	}

	if ((cap & SDIO_CAP_LSC) || (!(speed & SDIO_SPEED_SHS))) {
		sd_divisor = 2;
		speed &= ~SDIO_SPEED_EHS;
		sdos_info->high_speed_enabled = FALSE;
	}
	else {
		speed |= SDIO_SPEED_EHS;
		sdos_info->high_speed_enabled = TRUE;
	}

	sdos_info->block_mode = (cap & SDIO_CAP_SMB) ? TRUE : FALSE;

	dbus_sdos_cfg_write(sdos_info, SDIO_FUNC_0, SDIOD_CCCR_SPEED_CONTROL, speed, &err);
	if (err != BCME_OK) {
		DBUSERR(("%s: read fn1 cis pointer failed\n", __FUNCTION__));
		status = STATUS_UNSUCCESSFUL;
		return status;
	}

	if (sdos_info->bus_width != bus_width) {
		sdos_info->bus_width = bus_width;
		status = dbus_sdos_property(sdos_info, SDP_BUS_WIDTH, TRUE,
			&sdos_info->bus_width, sizeof(sdos_info->bus_width));
		if (!NT_SUCCESS(status)) {
			DBUSERR(("%s: Error setting bus width %d\n",
				__FUNCTION__, sdos_info->bus_width));
			return status;
		}
	}

	if ((sd_divisor < 1) || (sd_divisor > 2)) {
		sd_divisor = SDIO_DIVISOR_DEFAULT;
	}

	sdos_info->sd_divisor = sd_divisor;
	sdos_info->bus_clock_khz = SDIO_BUS_CLOCK_KHZ / sdos_info->sd_divisor;

	/* set bus clock */
	status = dbus_sdos_property(sdos_info, SDP_BUS_CLOCK, TRUE,
		&sdos_info->bus_clock_khz, sizeof(sdos_info->bus_clock_khz));
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: get SDP_BUS_CLOCK failed (0x%08X)\n",
			__FUNCTION__, status));
		return status;
	}

	/* get bus clock */
	status = dbus_sdos_property(sdos_info, SDP_BUS_CLOCK, FALSE,
		&sdos_info->bus_clock_khz, sizeof(sdos_info->bus_clock_khz));
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: get SDP_BUS_CLOCK failed (0x%08X)\n",
			__FUNCTION__, status));
		return status;
	}

	DBUSERR(("%s: SD Clock Divisor =  %d\n", __FUNCTION__, sdos_info->sd_divisor));
	DBUSERR(("%s: SD Clock Speed =  %d Hz\n", __FUNCTION__, sdos_info->bus_clock_khz*1000));

	if (sdos_info->block_sz[0] != MIN(BLOCK_TRANSFER_LENGTH_FUNC_0, sdos_info->host_block_sz)) {
		sdos_info->block_sz[0] =
			MIN(BLOCK_TRANSFER_LENGTH_FUNC_0, sdos_info->host_block_sz);
		status = dbus_sdos_property(sdos_info, SDP_FN0_BLOCK_LENGTH, TRUE,
			&sdos_info->block_sz[0], sizeof(sdos_info->block_sz[0]));
		if (!NT_SUCCESS(status)) {
			DBUSERR(("%s: set func 0 SDP_FUNCTION_BLOCK_LENGTH failed (0x%08X)\n",
				__FUNCTION__, status));
			return status;
		}
	}

	if (sdos_info->block_sz[1] != MIN(BLOCK_TRANSFER_LENGTH_FUNC_1, sdos_info->host_block_sz)) {
		sdos_info->block_sz[1] =
			MIN(BLOCK_TRANSFER_LENGTH_FUNC_1, sdos_info->host_block_sz);
		status = dbus_sdos_property(sdos_info, SDP_FUNCTION_BLOCK_LENGTH, TRUE,
			&sdos_info->block_sz[1], sizeof(sdos_info->block_sz[1]));
		if (!NT_SUCCESS(status)) {
			DBUSERR(("%s: set func 1 SDP_FUNCTION_BLOCK_LENGTH failed (0x%08X)\n",
				__FUNCTION__, status));
			return status;
		}
	}

	sdos_info->block_sz[2] = BLOCK_TRANSFER_LENGTH_FUNC_2;
	sdos_info->block_sz[2] = MIN(BLOCK_TRANSFER_LENGTH_FUNC_2, sdos_info->host_block_sz);

	ptr = (uint8*)&sdos_info->block_sz[2];
	dbus_sdos_cfg_write(sdos_info, SDIO_FUNC_0,
		(SDIOD_FBR_SIZE * 2) + SDIOD_CCCR_BLKSIZE_0, *ptr++, &err);
	if (err == BCME_OK)
		dbus_sdos_cfg_write(sdos_info, SDIO_FUNC_0,
			(SDIOD_FBR_SIZE * 2) + SDIOD_CCCR_BLKSIZE_1, *ptr++, &err);
	if (err != BCME_OK) {
		DBUSERR(("%s: set block size for func 2 failed\n", __FUNCTION__));
		return status;
	}

	/* get cis addr */
	sdos_info->fn1_cis_ptr = sd_get_cis_addr(sdos_info,
		SDIOD_FBR_CISPTR_0 + SDIOD_FBR_STARTADDR, &err);
	if (err != BCME_OK) {
		DBUSERR(("%s: read fn1 cis pointer failed\n", __FUNCTION__));
		return status;
	}
	DBUSINFO(("%s: get fn1 cis pointer 0x%x\n", __FUNCTION__, sdos_info->fn1_cis_ptr));

	/* get chip clock */
	clk = dbus_sdos_cfg_read(sdos_info, SDIO_FUNC_1, SBSDIO_FUNC1_CHIPCLKCSR, &err);
	if (err != BCME_OK) {
		DBUSERR(("%s: get SBSDIO_FUNC1_CHIPCLKCSR failed\n", __FUNCTION__));
		return status;
	}
	DBUSINFO(("%s: get SBSDIO_FUNC1_CHIPCLKCSR 0x%x\n", __FUNCTION__, clk));

	/* read io-ready */
	io_rdy = dbus_sdos_cfg_read(sdos_info, SDIO_FUNC_0, SDIOD_CCCR_IORDY, &err);
	if (err != BCME_OK) {
		DBUSERR(("%s: get SDIOD_CCCR_IORDY failed\n", __FUNCTION__));
		return status;
	}
	DBUSINFO(("%s: io-ready 0x%x\n", __FUNCTION__, io_rdy));

	/* card io should be ready now */
	ASSERT(io_rdy & SDIO_FUNC_READY_1);

	DBUSINFO(("%s: bus width %d, bus clock %dMHz, bus control 0x%x, host block size %d,"
		" block size: fn 0 %d, fn1 %d, fn 2 %d\n", __FUNCTION__,
		sdos_info->bus_width, sdos_info->bus_clock_khz/1000, sdos_info->bus_control,
		sdos_info->host_block_sz, sdos_info->block_sz[0], sdos_info->block_sz[1],
		sdos_info->block_sz[2]));

	return STATUS_SUCCESS;
}

static VOID
dbus_sdos_bus_close(sdos_info_t *sdos_info)
{
	int err;

	if (sdos_info->bus_intf.Context == NULL)
		return;

	if (sdos_info->bus_intf.InterfaceDereference)
		sdos_info->bus_intf.InterfaceDereference(sdos_info->bus_intf.Context);

	bzero(&sdos_info->bus_intf, sizeof(sdos_info->bus_intf));

	return;
}


/* interrupt(via bus driver) callback handler at passive level.
 * sdio bus driver disables controller interrupt before calling
 * this function and re-enabling controller interrupt after
 * interrupt acknowledged
 */
static void
dbus_sdos_intr_callback(void *context, ULONG intr_type)
{
	sdos_info_t *sdos_info = (sdos_info_t *)context;

	ASSERT(sdos_info);

	EVENT_SIGNAL(&sdos_info->irqevent);
}

int
dbus_sdos_abort(sdos_info_t *sdos_info, uint fn)
{
	int err;

	ASSERT(sdos_info);

	dbus_sdos_cfg_write(sdos_info, SDIO_FUNC_0, SDIOD_CCCR_IOABORT, (uint8)fn, &err);

	return err;
}

/* initialize sd bus and register interrupt handler */
static NTSTATUS
dbus_sdos_bus_open(sdos_info_t *sdos_info)
{
	NTSTATUS status;
	BOOLEAN enable = FALSE;
	dbus_pub_t *pub = sdos_info->pub;
	SDBUS_INTERFACE_PARAMETERS intf_params = {0};

	if (sdos_info->bus_intf.Context != NULL)
		return STATUS_RESOURCE_IN_USE;

	/* open an interface to sd bus driver */
	sdos_info->bus_intf.Size = sizeof(SDBUS_INTERFACE_STANDARD);
	sdos_info->bus_intf.Version = SDBUS_INTERFACE_VERSION;
	status = SdBusOpenInterface(pub->sh->PDO, &sdos_info->bus_intf,
		sizeof(SDBUS_INTERFACE_STANDARD), SDBUS_INTERFACE_VERSION);
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: SdBusOpenInterface failed (0x%08X)\n", __FUNCTION__, status));
		return status;
	}

	/* initialize sd bus interface */
	intf_params.Size = sizeof(SDBUS_INTERFACE_PARAMETERS);
	intf_params.TargetObject = WdfDeviceWdmGetAttachedDevice(pub->sh->wdfdevice);
	intf_params.DeviceGeneratesInterrupts = TRUE;
	intf_params.CallbackAtDpcLevel = TRUE;
	intf_params.CallbackRoutine = dbus_sdos_intr_callback;
	intf_params.CallbackRoutineContext = sdos_info;
	status = STATUS_UNSUCCESSFUL;
	if (sdos_info->bus_intf.InitializeInterface)
		status = (sdos_info->bus_intf.InitializeInterface)
			(sdos_info->bus_intf.Context, &intf_params);
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: init interface failed (0x%08X)\n", __FUNCTION__, status));
		goto open_error;
	}

	/* now fill in the function number */
	status = dbus_sdos_property(sdos_info, SDP_FUNCTION_NUMBER, FALSE,
		&sdos_info->func_num, sizeof(sdos_info->func_num));
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: get SDP_FUNCTION_NUMBER failed (0x%08X)\n",
			__FUNCTION__, status));
		goto open_error;
	}

	if (sdos_info->func_num != SDIO_FUNC_1) {
		ASSERT(sdos_info->func_num == SDIO_FUNC_1);
		DBUSERR(("%s: unsupported sd function %d\n", __FUNCTION__, sdos_info->func_num));
		goto open_error;
	}

	/* get sd bus driver version */
	sdos_info->bus_drvr_ver = SDBUS_DRIVER_VERSION_1;
	status = dbus_sdos_property(sdos_info, SDP_BUS_DRIVER_VERSION, FALSE,
		&sdos_info->bus_drvr_ver, sizeof(sdos_info->bus_drvr_ver));
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: get SDP_BUS_DRIVER_VERSION failed (0x%08X)\n",
			__FUNCTION__, status));
		goto open_error;
	}

	DBUSINFO(("%s: fun num %d, bus driver ver %0x%x\n",
		__FUNCTION__, sdos_info->func_num, sdos_info->bus_drvr_ver));

	if (sdos_info->bus_drvr_ver < SDBUS_DRIVER_VERSION_2)
		goto open_error;

	enable = FALSE;
	dbus_sdos_property(sdos_info, SDP_FUNCTION_INT_ENABLE, TRUE, &enable, sizeof(BOOLEAN));

	/* get bus width */
	status = dbus_sdos_property(sdos_info, SDP_BUS_WIDTH, FALSE,
		&sdos_info->bus_width, sizeof(sdos_info->bus_width));
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: get SDP_BUS_WIDTH failed (0x%08X)\n", __FUNCTION__, status));
		goto open_error;
	}

	/* get bus clock */
	status = dbus_sdos_property(sdos_info, SDP_BUS_CLOCK, FALSE,
		&sdos_info->bus_clock_khz, sizeof(sdos_info->bus_clock_khz));
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: get SDP_BUS_CLOCK failed (0x%08X)\n",
			__FUNCTION__, status));
		goto open_error;
	}

	DBUSERR(("%s: SD Clock Speed =  %d Hz\n", __FUNCTION__, sdos_info->bus_clock_khz*1000));

	/* get maximum block length supported by host controller */
	status = dbus_sdos_property(sdos_info, SDP_HOST_BLOCK_LENGTH, FALSE,
		&sdos_info->host_block_sz, sizeof(sdos_info->host_block_sz));
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: get SDP_HOST_BLOCK_LENGTH failed (0x%08X)\n",
			__FUNCTION__, status));
		goto open_error;
	}

	status = dbus_sdos_property(sdos_info, SDP_FN0_BLOCK_LENGTH, FALSE,
		&sdos_info->block_sz[0], sizeof(sdos_info->block_sz[0]));
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: get func 0 SDP_FUNCTION_BLOCK_LENGTH failed (0x%08X)\n",
			__FUNCTION__, status));
		goto open_error;
	}

	status = dbus_sdos_property(sdos_info, SDP_FUNCTION_BLOCK_LENGTH, FALSE,
		&sdos_info->block_sz[1], sizeof(sdos_info->block_sz[1]));
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: set func 1 SDP_FUNCTION_BLOCK_LENGTH failed (0x%08X)\n",
			__FUNCTION__, status));
		goto open_error;
	}

open_error:

	if (!NT_SUCCESS(status))
		dbus_sdos_bus_close(sdos_info);

	return status;

}

static void *
dbus_sdos_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs)
{
	sdos_info_t *sdos_info;
	NDIS_STATUS status;

	sdos_info = MALLOC(pub->osh, sizeof(sdos_info_t));
	if (sdos_info == NULL)
		return NULL;

	ASSERT(OFFSETOF(sdos_info_t, pub) == 0);

	bzero(sdos_info, sizeof(sdos_info_t));

	sdos_info->pub = pub;
	sdos_info->cbarg = cbarg;
	sdos_info->cbs = cbs;

	sdos_info->AdapterHandle = pub->sh->adapterhandle;

	/* initialize locks */
	LOCK_INIT(&sdos_info->lock);
	LOCK_INIT(&sdos_info->rxlock);
	LOCK_INIT(&sdos_info->txlock);
	IFLOCK_INIT(&sdos_info->iflock);
	EVENT_INIT(&sdos_info->irqevent);

	sdos_info->lock_init = TRUE;
	sdos_info->irqhalt = FALSE;

	/* initialize sd bus and register interrupt handler */
	status = dbus_sdos_bus_open(sdos_info);
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: dbus_sdos_bus_open failed (0x%08X)\n", __FUNCTION__, status));
		return NULL;
	}

	/* initialize card */
	status = dbus_sdos_card_init(sdos_info);
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: dbus_sdos_card_init failed (0x%08X)\n", __FUNCTION__, status));
		goto attach_err;
	}

	return (void *) sdos_info;

attach_err:

	dbus_sdos_bus_close(sdos_info);
	MFREE(pub->osh, sdos_info, sizeof(sdos_info_t));
	return NULL;
}

static void
dbus_sdos_detach(dbus_pub_t *pub, void *info)
{
	sdos_info_t *sdos_info = (sdos_info_t *) info;
	osl_t *osh = pub->osh;

	if (sdos_info == NULL)
		return;

	dbus_sdos_bus_close(sdos_info);

	if (sdos_info->lock_init) {
		LOCK_FREE(&sdos_info->lock);
		LOCK_FREE(&sdos_info->rxlock);
		LOCK_FREE(&sdos_info->txlock);
		IFLOCK_FREE(&sdos_info->iflock);
		EVENT_FREE(&sdos_info->irqevent);
	}

	MFREE(osh, sdos_info, sizeof(sdos_info_t));
}


static bool
dbus_sdos_recv_needed(void *bus)
{
	return FALSE;
}

static void*
dbus_sdos_exec_rxlock(void *bus, exec_cb_t cb, struct exec_parms *args)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;
	void *ret;

	if (sdos_info == NULL)
		return NULL;

	LOCK(&sdos_info->rxlock);
	ret = cb(args);
	UNLOCK(&sdos_info->rxlock);

	return ret;
}

static void*
dbus_sdos_exec_txlock(void *bus, exec_cb_t cb, struct exec_parms *args)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;
	void *ret;

	if (sdos_info == NULL)
		return NULL;

	LOCK(&sdos_info->txlock);
	ret = cb(args);
	UNLOCK(&sdos_info->txlock);

	return ret;
}

static int
dbus_sdos_lock(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	LOCK(&sdos_info->lock);

	return DBUS_OK;
}

static int
dbus_sdos_unlock(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	UNLOCK(&sdos_info->lock);

	return DBUS_OK;
}

/* IOVar table */
enum {
	IOV_BLOCKSIZE = 1,
	IOV_SDMODE,
	IOV_SDIVISOR,
	IOV_RXCHAIN
};

const bcm_iovar_t sdioh_iovars[] = {
	{"sd_blocksize", IOV_BLOCKSIZE, 0, 0, 0, IOVT_UINT32, 0}, /* ((fn << 16) | size) */
	{"sd_divisor",	IOV_SDIVISOR, 0, 0, 0, IOVT_UINT32, 0},
	{"sd_mode",	IOV_SDMODE, 0, 0, 0, IOVT_UINT32, 0},
	{"sd_rxchain",	IOV_RXCHAIN, 0, 0, 0, IOVT_UINT32, 0},
	{NULL, 0, 0, 0, 0, 0, 0}
};

int
dbus_sdos_iovar_op(sdos_info_t *sdos_info, const char *name,
	void *params, int plen, void *arg, int len, bool set)
{
	const bcm_iovar_t *vi = NULL;
	int bcmerror = 0;
	int val_size;
	int32 int_val = 0;
	uint32 actionid;
	NTSTATUS status;

	/* get must have return space; Set does not take qualifiers */
	ASSERT(set || (arg && len));
	ASSERT(!set || (!params && !plen));

	if ((vi = bcm_iovar_lookup(sdioh_iovars, name)) == NULL) {
		bcmerror = BCME_UNSUPPORTED;
		goto exit;
	}

	if ((bcmerror = bcm_iovar_lencheck(vi, arg, len, set)) != 0)
		goto exit;

	/* Set up params so get and set can share the convenience variables */
	if (params == NULL) {
		params = arg;
		plen = len;
	}

	if (vi->type == IOVT_VOID)
		val_size = 0;
	else if (vi->type == IOVT_BUFFER)
		val_size = len;
	else
		val_size = sizeof(int);

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	actionid = set ? IOV_SVAL(vi->varid) : IOV_GVAL(vi->varid);
	switch (actionid) {
	case IOV_GVAL(IOV_BLOCKSIZE):
		if (int_val > TOTAL_FUNCS) {
			bcmerror = BCME_BADARG;
			break;
		}

		switch (int_val) {
		case SDIO_FUNC_0: int_val = BLOCK_TRANSFER_LENGTH_FUNC_0; break;
		case SDIO_FUNC_1: int_val = BLOCK_TRANSFER_LENGTH_FUNC_1; break;
		case SDIO_FUNC_2: int_val = BLOCK_TRANSFER_LENGTH_FUNC_2; break;
		}
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_GVAL(IOV_SDMODE):
		int_val = (sdos_info->bus_width == 4) ? SDIOH_MODE_SD4 : SDIOH_MODE_SD1;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_GVAL(IOV_SDIVISOR):
		int_val = sdos_info->sd_divisor;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_SDIVISOR):
		bcopy(arg, &int_val, val_size);

		if ((int_val < 1) || (int_val > 2)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		if (!sdos_info->high_speed_enabled) {
			if (int_val == 1)
				bcmerror = BCME_UNSUPPORTED;
			break;
		}

		sdos_info->sd_divisor = int_val;
		sdos_info->bus_clock_khz = SDIO_BUS_CLOCK_KHZ / sdos_info->sd_divisor;

		/* set bus clock */
		status = dbus_sdos_property(sdos_info, SDP_BUS_CLOCK, TRUE,
			&sdos_info->bus_clock_khz, sizeof(sdos_info->bus_clock_khz));
		if (!NT_SUCCESS(status)) {
			DBUSERR(("%s: set SDP_BUS_CLOCK failed (0x%08X)\n",
				__FUNCTION__, status));
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		/* get bus clock */
		status = dbus_sdos_property(sdos_info, SDP_BUS_CLOCK, FALSE,
			&sdos_info->bus_clock_khz, sizeof(sdos_info->bus_clock_khz));
		if (!NT_SUCCESS(status)) {
			DBUSERR(("%s: set SDP_BUS_CLOCK failed (0x%08X)\n",
				__FUNCTION__, status));
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		DBUSERR(("%s: SD Clock Divisor =  %d\n", __FUNCTION__, sdos_info->sd_divisor));
		DBUSERR(("%s: SD Clock Speed =  %d Hz\n",
			__FUNCTION__, sdos_info->bus_clock_khz*1000));
		break;

	default:
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

exit:
	return bcmerror;
}

static int
dbus_sdos_send_irb(void *bus, dbus_irb_tx_t *txirb)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;
	int ret = DBUS_ERR;

	if (sdos_info == NULL)
		return DBUS_ERR;

	return ret;
}

static int
dbus_sdos_cancel_irb(void *bus, dbus_irb_tx_t *txirb)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	/* FIX: Need to implement */
	return DBUS_ERR;
}

static int
dbus_sdos_stop(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	return DBUS_OK;
}

int
dbus_sdos_state_change(void *bus, int state)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

	if (sdos_info->cbarg && sdos_info->cbs) {
		if (sdos_info->cbs->state_change)
			sdos_info->cbs->state_change(sdos_info->cbarg, state);
	}

	return DBUS_OK;
}

int
dbus_sdos_pnp(void *bus, int event)
{
	NTSTATUS status;
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	DBUSERR(("PNP: %s event %d\n", __FUNCTION__, event));

	if (sdos_info == NULL)
		return DBUS_ERR;

	if (event == DBUS_PNP_RESUME) {
		/* initialize sd bus and register interrupt handler */
		if ((status = dbus_sdos_bus_open(sdos_info)) == STATUS_RESOURCE_IN_USE)
			return DBUS_OK;
		if (!NT_SUCCESS(status)) {
			DBUSERR(("%s: dbus_sdos_bus_open failed (0x%08X)\n",
				__FUNCTION__, status));
			return DBUS_ERR;
		}
		status = dbus_sdos_card_init(sdos_info);
		if (!NT_SUCCESS(status)) {
			DBUSERR(("%s: dbus_sdos_card_init failed (0x%08X)\n",
				__FUNCTION__, status));
			return DBUS_ERR;
		}
		dbus_sdos_state_change(bus, DBUS_STATE_UP);
	} else if (event == DBUS_PNP_SLEEP) {
		dbus_sdos_intr_disable(sdos_info);
		dbus_sdos_state_change(bus, DBUS_STATE_SLEEP);
		dbus_sdos_bus_close(sdos_info);
	} else if (event == DBUS_PNP_DISCONNECT) {
		dbus_sdos_state_change(bus, DBUS_STATE_DISCONNECT);
	}

	return DBUS_OK;
}

static int
dbus_sdos_up(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	if (sdos_info == NULL)
		return DBUS_ERR;

#ifdef BCMDONGLEHOST
	dbus_wd_timer_init(sdos_info, dhd_watchdog_ms);
#endif /* BCMDONGLEHOST */
	dbus_sdos_state_change(bus, DBUS_STATE_UP);

	return DBUS_OK;
}

static int
dbus_sdos_down(void *bus)
{
	sdos_info_t *sdos_info = (sdos_info_t *) bus;

	dbus_sdos_state_change(bus, DBUS_STATE_DOWN);
	return DBUS_OK;
}
