/******************************************************************************
 *  Copyright (C) 2017-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
#include "monitor.h"
#include "cpu_data.h"
#include "context_mgt.h"
#include "gic.h"
#include "interrupt.h"
#include "brcmstb_svc.h"
#include "scmi_private.h"
#include "mon_params.h"
#include "dvfs.h"
#include "avs.h"
#include "brcmstb_priv.h"
#include "string.h"

#define SCMI_VERSION			0x10000
#define SCMI_PROTO_PERF_VERSION		0x10000
#define SCMI_PROTO_SENSOR_VERSION	0x10000
#define SCMI_PROTO_BRCM_VERSION		0x10000
#define BRCMSTB_SVC_SCMI_MAX		0x01
#define BRCM_NUM_PROTOS			3 /* perf, sensor, brcm */
#define BRCM_SCMI_FW_VER		1
#define BRCM_NUM_PERF_DOM		1
#define BRCM_DEFAULT_LATENCY		5000 /* usec */
#define BRCM_MAX_CPU_PSTATES		10
#define BRCM_DEFAULT_SENSOR_TYPE	0xe805

extern int avs_read_sensor(uint32_t, int*);


struct __attribute__ ((__packed__)) scmi_perf
{
	uint32_t perf;	/* Percent is being used, but this could be
			 * freq as well. */
	uint32_t power;	/* Power cost metric of our choosing */
	uint16_t latency;	/* Transition latency in uSec */
	uint16_t reserved;
};

static struct {
	int scmi_intr;
	int num_sensors;
	uint32_t cpu_sustained_freq;
	uint32_t cpu_sustained_perf;
	size_t num_cpu_pstates;
	uint32_t cur_cpu_pstate;
	struct scmi_perf perf_table[BRCM_MAX_CPU_PSTATES];
} info;

static void do_scmi_process_msg(uint64_t *ctx);
static void brcmstb_svc_scmi_init(void);
static void brcmstb_svc_scmi_plat_init(void);
static volatile struct mailbox_mem *scmi_mem;

static service_do_func_t do_scmi_funcs[BRCMSTB_SVC_SCMI_MAX] =
{
	do_scmi_process_msg,
};

static void brcmstb_svc_scmi_proc(
	uint32_t fid,
	uint64_t *ctx,
	uint64_t flags)
{
	service_do_func_t pfunc;

	UNUSED(flags);

	if (BRCMSTB_SVC_FUNC(fid) < BRCMSTB_SVC_SCMI_MAX) {
		pfunc = do_scmi_funcs[BRCMSTB_SVC_FUNC(fid)];
		if (pfunc)
			pfunc(ctx);
		return;
	}

	/* All fall-through cases */
	ctx[0] = SMC_UNK;
}

service_mod_desc_t brcmstb_svc_scmi_desc = {
	BRCMSTB_SVC_SCMI_MAX,
	NULL,
	brcmstb_svc_scmi_init,
	brcmstb_svc_scmi_proc
};

/*----------------------------------------------------------------------------*/

static inline uint32_t ioread32(volatile void *addr)
{
	return MMIO32(addr);
}

static inline void iowrite32(uint32_t val, volatile void *addr)
{
	MMIO32(addr) = val;
}

static void memcpy_toio(volatile void *dst_in, const void *src_in,
			size_t size)
{
	unsigned i;
	const uint8_t *src = src_in;
	volatile uint8_t *dst = dst_in;

	for (i = 0; i < size; i++) {
		dst[i] = src[i];
	}
}

static void mon64_freq_table_to_scmi_perf_table(uint32_t freqs[], unsigned n)
{
	uint32_t max = 0;
	uint32_t min = ~0;
	unsigned int i;

	for (i = 0; i < n; i++) {
		info.perf_table[i].perf = freqs[i] * 1000; /* Mhz => khz */
		info.perf_table[i].power = freqs[i] * 1000;
		if (freqs[i] > max)
			max = freqs[i];
		if (freqs[i] < min)
			min = freqs[i];
		info.perf_table[i].latency = BRCM_DEFAULT_LATENCY;
	}
	info.cpu_sustained_perf = max * 1000;
	info.cpu_sustained_freq = max * 1000;
	info.num_cpu_pstates = n;
}

static void brcmstb_svc_scmi_init(void)
{
	scmi_mem = (volatile void *) nsec_mbox_addr();
	info.scmi_intr = nsec_mbox_sgi();
}

static void brcmstb_svc_scmi_plat_init(void)
{
	uint32_t freqs[BRCM_MAX_CPU_PSTATES];
	int ret;

	info.num_sensors = avs_num_sensors();
	if (info.num_sensors < 0)
		goto OUT;

	info.num_cpu_pstates = BRCM_MAX_CPU_PSTATES;
	ret = avs_cpu_pstate_freqs(&info.num_cpu_pstates, freqs);
	if (ret < 0)
		goto OUT;

	info.cur_cpu_pstate = 0;
	ret = dvfs_set_nsec_pstate_all(info.cur_cpu_pstate);
	if (ret < 0)
		goto OUT;

	mon64_freq_table_to_scmi_perf_table(freqs, info.num_cpu_pstates);
	return;
OUT:
	ERR_MSG("mon: Failed to init SCMI engine!");
}

#ifdef DEBUG
static const char *proto_to_str(int proto)
{
	static char *strs[] = { "Base", "Power", "System", "Perf", "Clock",
				"Sensor", };
	static const int N = sizeof(strs)/sizeof(strs[0]);
	int idx = proto - SCMI_PROTOCOL_BASE;

	if (proto == 0x80)
		return "Brcm";

	if (idx < 0 || idx >= N)
		return "unknown";
	else
		return strs[idx];
}


static const char *msg_id_to_str(int proto, unsigned int id)
{
	static char *base_strs[] = { "Version", "Attrs", "Msg_attrs", "Vendor",
				     "Sub_vendor", "Impl_version", "List_protos",
				     "Agent", "Notify_errors", };
	static char *perf_strs[] = { "Version", "Attrs", "Msg_attrs",
				     "Dom_attrs", "Desc_levels", "Limits_set",
				     "Limits_get", "Level_set", "Level_get",
				     "Notify_limits", "Notify_level", };
	static char *sens_strs[] = { "Version", "Attrs", "Msg_attrs",
				     "Desc_get", "Trip_Pt_notify", "Trip_Pt_config",
				     "Reading_get", "Trip_Pt_event", };

	static char *brcm_strs[] = { "Version", "Attrs", "Msg_attrs", "Send_Avs_Cmd", };
	const unsigned int base_num = sizeof(base_strs)/sizeof(base_strs[0]);
	const unsigned int perf_num = sizeof(perf_strs)/sizeof(perf_strs[0]);
	const unsigned int sens_num = sizeof(sens_strs)/sizeof(sens_strs[0]);
	const unsigned int brcm_num = sizeof(brcm_strs)/sizeof(brcm_strs[0]);

	if (proto == SCMI_PROTOCOL_BASE && id < base_num)
		return base_strs[id];
	else if (proto == SCMI_PROTOCOL_PERF && id < perf_num)
		return perf_strs[id];
	else if (proto == SCMI_PROTOCOL_SENSOR && id < sens_num)
		return sens_strs[id];
	else if (proto == SCMI_PROTOCOL_BRCM && id < brcm_num)
		return brcm_strs[id];
	else
		return "Unknown";
}
#endif /* DEBUG */

/* === Base Protocol ===================================== */
static void do_scmi_process_base_msg(unsigned int id, uint32_t arg1, uint32_t arg2)
{
	UNUSED(arg2);

	switch (id) {

	case PROTOCOL_VERSION:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, SCMI_VERSION);
		break;

	case PROTOCOL_ATTRIBUTES:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, BRCM_NUM_PROTOS);
		break;

	case PROTOCOL_MESSAGE_ATTRIBUTES:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, 0x0);
		break;

	case BASE_DISCOVER_VENDOR:
		scmi_mem->len = 2 * SCMI_ARG_SZ + SCMI_STRING_SZ;
		SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_SUCCESS);
		memcpy_toio(&scmi_mem->payload[1], "brcm-scmi", sizeof("brcm-scmi"));
		break;

	case BASE_DISCOVER_IMPLEMENT_VERSION:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, BRCM_SCMI_FW_VER);
		break;

	case BASE_DISCOVER_LIST_PROTOCOLS:
		/* Arg1 is the skip amount */
		if (arg1 >= BRCM_NUM_PROTOS) {
			scmi_mem->len = 3 * SCMI_ARG_SZ;
			SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, 0x0);
		} else {
			scmi_mem->len = 4 * SCMI_ARG_SZ;
			/* Declare that we have one (Perf) protocol */
			SCMI_PAYLOAD_ARG3(scmi_mem->payload, SCMI_SUCCESS,
					  BRCM_NUM_PROTOS, SCMI_PROTOCOL_PERF
					  | (SCMI_PROTOCOL_SENSOR << 8)
					  | (SCMI_PROTOCOL_BRCM << 16));
		}
		break;

	case BASE_DISCOVER_SUB_VENDOR:
	case BASE_DISCOVER_AGENT:
	case BASE_NOTIFY_ERRORS:
	default:
		/* Optional commands that are not implemented. */
		SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ENTRY);
	}
}

/* === Perf Protocol ===================================== */
static void do_scmi_process_perf_msg(unsigned int id, uint32_t arg1, uint32_t arg2)
{
	int ret;

	switch (id) {
	case PROTOCOL_VERSION:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS,
				  SCMI_PROTO_PERF_VERSION);
		break;

	case PROTOCOL_ATTRIBUTES:
		scmi_mem->len = 24;
		SCMI_PAYLOAD_ARG5(scmi_mem->payload, SCMI_SUCCESS,
				  /* Using only one power domain */
				  BRCM_NUM_PERF_DOM,
				  0, /* Lo addr of shared stat region */
				  0, /* Hi addr of shared stat region */
				  0); /* Len  of shared stat region */
		break;

	case PROTOCOL_MESSAGE_ATTRIBUTES:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		/* Arg1 is message ID */
		if (arg1 >= PERF_NOTIFY_LIMITS) {
			SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_ERR_ENTRY, 0x0);
		} else {
			SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, 0x0);
		}
		break;

	case PERF_DOMAIN_ATTRIBUTES:
		/* Arg1 is the domain */
		if (arg1 >= BRCM_NUM_PERF_DOM) {
			SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ENTRY);
		} else {
			scmi_mem->len = 40;
			SCMI_PAYLOAD_ARG5
				(scmi_mem->payload, SCMI_SUCCESS,
				 PERF_DOM_ATTR_SET_PERF,
				 0, /* rate limit not supported */
				 info.cpu_sustained_freq,
				 info.cpu_sustained_perf);
			memcpy_toio(&scmi_mem->payload[5], "brcm-cpus",
				    sizeof("brcm-cpus"));
		}
		break;

	case PERF_DESCRIBE_LEVELS:
		/* Arg1 is the domain, arg2 is the "starting" pstate */
		if (arg1 >= BRCM_NUM_PERF_DOM) {
			SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ENTRY);
		} else if (arg2 >= info.num_cpu_pstates) {
			scmi_mem->len = 3 * SCMI_ARG_SZ;
			SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_ERR_ENTRY, 0);
		} else {
			scmi_mem->len = 12 + info.num_cpu_pstates
				* sizeof(info.perf_table[0]);
			SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS,
					  info.num_cpu_pstates);
			memcpy_toio(&scmi_mem->payload[2], &info.perf_table[0],
				    info.num_cpu_pstates * sizeof(info.perf_table[0]));
		}
		break;

	case PERF_LIMITS_GET:
		/* Arg1 is the domain */
		if (arg1 >= BRCM_NUM_PERF_DOM) {
			SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ENTRY);
		} else {
			scmi_mem->len = 4 * SCMI_ARG_SZ;
			SCMI_PAYLOAD_ARG3(scmi_mem->payload, SCMI_SUCCESS,
					  info.num_cpu_pstates - 1, /* max level */
					  0); /* min level */
		}
		break;

	case PERF_LIMITS_SET:
		/* Arg1 is the domain */
		if (arg1 >= BRCM_NUM_PERF_DOM) {
			SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ENTRY);
		} else {
			SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ACCESS);
		}
		break;

	case PERF_LEVEL_SET:
		/* Arg1 is the domain */
		if (arg1 >= BRCM_NUM_PERF_DOM) {
			SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ENTRY);
		} else {
			unsigned ui;

			/* Arg2 is the desired performance level */
			for (ui = 0; ui < info.num_cpu_pstates; ui++)
				if (info.perf_table[ui].perf == (uint32_t)arg2)
					break;
			if (ui >= info.num_cpu_pstates) {
				SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ACCESS);
			} else {
				ret = dvfs_set_nsec_pstate_all(ui);
				if (ret >= 0)
					info.cur_cpu_pstate = ui;
				DBG_MSG("dvfs_set_nsec_pstate_all(%x (aka %dMHz) => %d\n",
					ui, arg2, ret);
				SCMI_PAYLOAD_ARG1(scmi_mem->payload,ret ?
						  SCMI_ERR_ACCESS :SCMI_SUCCESS);
			}
		}
		break;

	case PERF_LEVEL_GET:
		/* Arg1 is the domain */
		if (arg1 >= BRCM_NUM_PERF_DOM) {
			SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ENTRY);
		} else {
			scmi_mem->len = 3 * SCMI_ARG_SZ;
			SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS,
					  info.perf_table[info.cur_cpu_pstate].perf);
		}
		break;

	case PERF_NOTIFY_LIMITS:
	case PERF_NOTIFY_LEVEL:
	default:
		SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_SUPPORT);
	}
}


/* === Sensor Protocol =================================== */
static void do_scmi_process_sensor_msg(unsigned int id, uint32_t arg1, uint32_t arg2)
{
	int idx, n, tmp, i, ret;
	uint32_t sensor_val;

	UNUSED(arg2);

	switch (id) {
	case PROTOCOL_VERSION:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS,
				  SCMI_PROTO_SENSOR_VERSION);
		break;
	case PROTOCOL_ATTRIBUTES:
		scmi_mem->len = 6 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG5(scmi_mem->payload, SCMI_SUCCESS,
				  ((1 << 16) | info.num_sensors), 0, 0, 0);
		break;

	case PROTOCOL_MESSAGE_ATTRIBUTES:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, 0);
		break;

	case SENS_DESC_GET:
		idx = (int) arg1;
		/* send at most info on four sensors at a time */
		if (idx < 0 || idx > info.num_sensors-1)
			n = 0;
		else if ((info.num_sensors - idx) <= 4)
			n = info.num_sensors - idx;
		else
			n = 4;

		/* [31:16] num remaining sensors, [11:0] num sensors in reply */
		tmp = info.num_sensors - (idx + n);
		tmp = ((tmp > 0 ? tmp : 0) << 16) | n;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, tmp);

		/* Set size of returning payload */
		scmi_mem->len = (3 + n*7) * SCMI_ARG_SZ;

		for (i = 0; i < n; i++) {
			static const char hex[16] = "0123456789abcdef";
			char name[16];
			int s;

			/* We don't have sprintf() or strcat() */
			strcpy(name, "sensor_");
			s = strlen(name);
			name[s++] = hex[((i + idx) >> 4) & 0xf];
			name[s++] = hex[((i + idx) >> 0) & 0xf];
			name[s++] = 0;

			tmp = i * 7 + 2;
			scmi_mem->payload[tmp + 0] = i + idx;
			scmi_mem->payload[tmp + 1] = 0;
			/* We specify all sensors to be of type
			 * voltage because scmi-hwmon.c then gives
			 * them the names in0, in1, in2, ... The
			 * values given for the sensors are bogus as
			 * they are just used for testing.
			 */
			scmi_mem->payload[tmp + 2] = BRCM_DEFAULT_SENSOR_TYPE;
			memcpy_toio(&scmi_mem->payload[tmp + 3], name,
				    sizeof(name));
		}
		break;

	case SENS_READ_GET:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		if ((int)arg1 > info.num_sensors) {
			SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_ENTRY);
		} else {
			ret = avs_sensor_read(arg1, &sensor_val);
			ret = ret < 0 ? SCMI_ERR_ACCESS : SCMI_SUCCESS;
			SCMI_PAYLOAD_ARG3(scmi_mem->payload, ret, sensor_val, 0);
		}
		break;

	case SENS_TRIP_PT_NOTIFY:
	case SENS_TRIP_PT_CONFIG:
	case SENS_TRIP_PT_EVENT:
	default:
		SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_SUPPORT);
	}
}

/* === Brcm Protocol ===================================== */
static void do_scmi_process_brcm_msg(unsigned int id, uint32_t arg1, uint32_t arg2)
{
	int num_in, num_out, cmd, i;
	uint32_t avs_status, out[AVS_MAX_PARAMS], in[AVS_MAX_PARAMS];

	UNUSED(arg2);

	switch (id) {
	case PROTOCOL_VERSION:
		/* This depends on the AVS command */
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS,
				  SCMI_PROTO_BRCM_VERSION);
		break;

	case PROTOCOL_ATTRIBUTES:
	case PROTOCOL_MESSAGE_ATTRIBUTES:
		scmi_mem->len = 3 * SCMI_ARG_SZ;
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, 0);
		break;

	case BRCM_SEND_AVS_CMD:
		/* Extract the meta data in the first word:
		 *  num_in .... num in params not including AVS cmd.
		 *  num_out ... num out params, not including AVS status.
		 *  cmd ....... the AVS cmd to be executed.
		 */
		num_in = (arg1 >> 8) & 0xff;
		num_out = (arg1 >> 16) & 0xff;
		cmd = arg1 & 0xff;

		for (i = 0; i < num_in; i++)
			in[i] = scmi_mem->payload[2+i];

		DBG_MSG("mon.scmi: AVS_COMMAND 0x%x (p_in=%d, p_out=%d)",
			cmd, num_in, num_out);
		/* Delegate the Monitor to do the AVS command.  The params
		 * are in the payload, and the contents of the AVS mbox
		 * after the command is executed should be copied back
		 * to the payload. */
		avs_cmd_send(cmd, &avs_status, in,
			     num_in * sizeof(uint32_t), out,
			     num_out * sizeof(uint32_t));

		scmi_mem->len = (3 + num_out) * SCMI_ARG_SZ;

		/* We'll say it is an SCMI success since the AVS status
		 * must be written after do_avs_cmd() is invoked. */
		SCMI_PAYLOAD_ARG2(scmi_mem->payload, SCMI_SUCCESS, avs_status);

		for (i = 0; i < num_out; i++)
			scmi_mem->payload[i+2] = out[i];
		break;

	default:
		SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_SUPPORT);
	}
}


/* === All SCMI messages are processed here === */
static void do_scmi_process_msg(uint64_t *ctx)
{
	static bool msg_received = false;
	uint32_t msg		= MMIO32(&scmi_mem->msg_header);
	unsigned int id		= SCMI_MSG_GET_ID(msg);
	unsigned int proto 	= SCMI_MSG_GET_PROTO(msg);
	uint32_t arg1, arg2;

	if (!msg_received) {
		dvfs_activate();
		brcmstb_svc_scmi_plat_init();
		msg_received = true;
	}

	SCMI_PAYLOAD_RET_VAL2(scmi_mem->payload, arg1, arg2);

	DBG_MSG("mon.scmi: writing tok=%d  type=%d  proto.ID=%s.%s(arg1=%x arg2=%x)",
		SCMI_MSG_GET_TOKEN(msg), SCMI_MSG_GET_TYPE(msg),
		proto_to_str(proto), msg_id_to_str(proto, id), arg1, arg2);

	ctx[0] = 0;

	scmi_mem->len = 2 * SCMI_ARG_SZ; /* minimum */

	switch (proto) {
	case SCMI_PROTOCOL_BASE:
		do_scmi_process_base_msg(id, arg1, arg2);
		break;
	case SCMI_PROTOCOL_PERF:
		do_scmi_process_perf_msg(id, arg1, arg2);
		break;
	case SCMI_PROTOCOL_SENSOR:
		do_scmi_process_sensor_msg(id, arg1, arg2);
		break;
	case SCMI_PROTOCOL_BRCM:
		do_scmi_process_brcm_msg(id, arg1, arg2);
		break;
	default:
		/* Unsupported Protocol */
		SCMI_PAYLOAD_ARG1(scmi_mem->payload, SCMI_ERR_SUPPORT);
	}

	/* Mark the mbox channel as free */
	MMIO32(&scmi_mem->status) = 1;

	/* Generate SGI */
	gic_sgi_intr_generate(info.scmi_intr, 1);
}
