/*
 * Code for mesh in wl command-line swiss-army-knife utility
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
#include <wlioctl.h>
#include <bcmendian.h>
#include "wlu_common.h"
#include "wlu.h"

static cmd_func_t wl_mesh_control;
typedef struct wl_mesh_sub_cmd wl_mesh_sub_cmd_t;
typedef int (mesh_cmd_handler_t)(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv);

struct wl_mesh_sub_cmd {
	char *name;
	uint8  version;              /* cmd  version */
	uint16 id;                   /* id for the dongle f/w switch/case */
	uint16 type;                 /* base type of argument */
	mesh_cmd_handler_t *handler; /* cmd handler  */
};

#define MESH_PARAMS_USAGE        \
"\tUsage: wl mesh [command] [cmd options] as follows:\n" \
"\t\twl mesh enable [1/0] - enable disable mesh functionality\n" \
"\t\twl mesh join meshid\n" \
"\t\twl mesh peer_status\n" \
"\t\twl mesh add route [dest mac address][next hop]\n" \
"\t\twl mesh delete route [mac address]\n" \
"\t\twl mesh add_filter [mac address]\n" \
"\t\twl mesh enable_al_metric [1/0] - enable disable al metric\n"

#define WL_MESH_FUNC(suffix) wl_mesh_subcmd_ ##suffix
static int wl_mesh_subcmd_enable(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv);
static int wl_mesh_subcmd_join(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv);
static int wl_mesh_subcmd_peer_status(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv);
static int wl_mesh_subcmd_add_route(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv);
static int wl_mesh_subcmd_del_route(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv);
static int wl_mesh_subcmd_add_filter(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv);
static int wl_mesh_subcmd_enable_al_metric(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv);

static cmd_t wl_mesh_cmds[] = {
	{ "mesh", wl_mesh_control, WLC_GET_VAR, WLC_SET_VAR,
	"superfunction for mesh protocol commands \n\n"
	MESH_PARAMS_USAGE},
	{ NULL, NULL, 0, 0, NULL }
};

static const wl_mesh_sub_cmd_t mesh_cmd_list[] = {
	/* wl mesh enable [0/1] or new: "wl mesh [0/1]" */
	{"enable", 0x01, WL_MESH_CMD_ENABLE,
	IOVT_BUFFER, WL_MESH_FUNC(enable)
	},
	{"join", 0x01, WL_MESH_CMD_JOIN,
	IOVT_BUFFER, WL_MESH_FUNC(join)
	},
	{"peer_status", 0x01, WL_MESH_CMD_PEER_STATUS,
	IOVT_BUFFER, WL_MESH_FUNC(peer_status)
	},
	{"add_route", 0x01, WL_MESH_CMD_ADD_ROUTE,
	IOVT_BUFFER, WL_MESH_FUNC(add_route)
	},
	{"del_route", 0x01, WL_MESH_CMD_DEL_ROUTE,
	IOVT_BUFFER, WL_MESH_FUNC(del_route)
	},
	{"add_filter", 0x01, WL_MESH_CMD_ADD_FILTER,
	IOVT_BUFFER, WL_MESH_FUNC(add_filter)
	},
	{"link_metric", 0x1, WL_MESH_CMD_ENAB_AL_METRIC,
	IOVT_BUFFER, WL_MESH_FUNC(enable_al_metric)
	},
	{NULL, 0, 0, 0, NULL}
};

void
wluc_mesh_module_init(void)
{
	/* register mesh commands */
	wl_module_cmds_register(wl_mesh_cmds);
}

static void
wl_mesh_print_peer_info(mesh_peer_info_ext_t *mpi_ext, uint32 peer_results_count)
{
	char *peering_map[] = MESH_PEERING_STATE_STRINGS;
	uint32 count = 0;

	printf("no: ------addr------ : l.aid : state : p.aid : mppid : llid : plid : "
		" entry_state : rssi\n");
	for (count = 0; count < peer_results_count; count++) {
		if (mpi_ext->entry_state != MESH_SELF_PEER_ENTRY_STATE_TIMEDOUT) {
			printf("%d: %s : 0x%x : %s : 0x%x : %4d : %4d : %4d : %s : %d\n",
				count, wl_ether_etoa(&mpi_ext->ea),
				dtoh16(mpi_ext->local_aid),
				peering_map[mpi_ext->peer_info.state],
				dtoh16(mpi_ext->peer_info.peer_aid),
				dtoh16(mpi_ext->peer_info.mesh_peer_prot_id),
				dtoh16(mpi_ext->peer_info.local_link_id),
				dtoh16(mpi_ext->peer_info.peer_link_id),
				(dtoh32(mpi_ext->entry_state) ==
					MESH_SELF_PEER_ENTRY_STATE_ACTIVE) ?
				"ACTIVE" :
				"EXTERNAL",
				mpi_ext->rssi);
		}
		else {
			printf("%d: %s : %s : %s : %s : %s : %s : %s : %s : %s\n",
				count, wl_ether_etoa(&mpi_ext->ea), "NA     ",
				"NA    ",
				"NA    ",
				"NA    ",
				"NA    ",
				"NA    ",
				"TIMEDOUT",
				"NA    ");
		}
		mpi_ext++;
	}
}

static int
wlu_mesh_set_vars_cbfn(void *ctx, uint8 *data, uint16 type, uint16 len)
{
	int res = BCME_OK;

	UNUSED_PARAMETER(ctx);
	UNUSED_PARAMETER(len);

	switch (type) {

	case WL_MESH_XTLV_ENABLE:
		printf("mesh: %s\n", *data?"enabled":"disabled");
		break;

	case WL_MESH_XTLV_STATUS:
	{
		mesh_peer_info_dump_t *peer_results;
		mesh_peer_info_ext_t *mpi_ext;

		peer_results = (mesh_peer_info_dump_t *)data;
		mpi_ext = (mesh_peer_info_ext_t *)peer_results->mpi_ext;
		wl_mesh_print_peer_info(mpi_ext, dtoh16(peer_results->count));

		if (dtoh16(peer_results->remaining)) {
			printf("more peer status to retrieve = %d\n",
				(dtoh16(peer_results->remaining)));
			res = BCME_BUFTOOSHORT;
		}
		break;
	}

	case WL_MESH_XTLV_ENAB_AIRLINK:
		printf("mesh airlink metric %s\n", *data?"enabled":"disabled");
		break;

	default:
		/* ignore */
		break;
	}
	return res;
}

/*
 *   --- common for all mesh get commands ----
 */
static int
wl_mesh_get_ioctl(void *wl, wl_mesh_ioc_t *meshioc, uint16 iocsz)
{
	/* for gets we only need to pass ioc header */
	wl_mesh_ioc_t *iocresp = NULL;
	int res;

	/*  send getbuf mesh iovar */
	res = wlu_var_getbuf(wl, "mesh", meshioc, iocsz, (void *)&iocresp);
	/*  check the response buff  */
	if ((res == BCME_OK) && (iocresp != NULL)) {
		/* scans ioctl tlvbuf f& invokes the cbfn for processing  */
		/* prhex("iocresp buf:", iocresp->data, iocresp->len); */
		res = bcm_unpack_xtlv_buf(meshioc, iocresp->data, iocresp->len,
				BCM_XTLV_OPTION_ALIGN32, wlu_mesh_set_vars_cbfn);
	}

	return res;
}

static int
wl_mesh_subcmd_enable(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv)
{
	int res = BCME_OK;
	wl_mesh_ioc_t *meshioc;
	uint16 buflen;
	uint16 iocsz = sizeof(*meshioc) + MESH_IOC_BUFSZ;
	bcm_xtlv_t *pxtlv = NULL;

	/*  alloc mem for ioctl headr +  tlv data  */
	meshioc = calloc(1, iocsz);
	if (meshioc == NULL) {
		 return BCME_NOMEM;
	}

	/* make up mesh cmd ioctl header */
	meshioc->version = htod16(WL_MESH_IOCTL_VERSION);
	meshioc->id = htod16(cmd->id);
	meshioc->len = htod16(MESH_IOC_BUFSZ);
	pxtlv = (bcm_xtlv_t *)meshioc->data;

	if(*argv == NULL) { /* get */
		iocsz = sizeof(*meshioc) + sizeof(*pxtlv);
		res = wl_mesh_get_ioctl(wl, meshioc, iocsz);

	} else {	/* set */
		/* max tlv data we can write, it will be decremented as we pack */
		uint8 val =  atoi(*argv);

		buflen = MESH_IOC_BUFSZ;
		/* save buflen at start */
		uint16 buflen_at_start = buflen;

		/* we'll adjust final ioc size at the end */
		res = bcm_pack_xtlv_entry((uint8**)&pxtlv, &buflen, WL_MESH_XTLV_ENABLE,
			sizeof(uint8), &val, BCM_XTLV_OPTION_ALIGN32);

		if (res != BCME_OK) {
			goto fail;
		}

		/* adjust iocsz to the end of last data record */
		meshioc->len = (buflen_at_start - buflen);
		iocsz = sizeof(*meshioc) + meshioc->len;

		res = wlu_var_setbuf(wl, "mesh", meshioc, iocsz);
	}
fail:

	free(meshioc);
	return res;
}

static int
wl_mesh_subcmd_join(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv)
{
	int res = BCME_USAGE_ERROR;
	uint16 buflen, buflen_at_start;
	wl_mesh_ioc_t *meshioc = NULL;
	uint16 iocsz = sizeof(*meshioc) + MESH_IOC_BUFSZ;
	uint8 *pxtlv;

	/*  alloc mem for ioctl headr +  tlv data  */
	meshioc = calloc(1, iocsz);
	if (meshioc == NULL) {
		return BCME_NOMEM;
	}

	/* make up mesh cmd ioctl header */
	meshioc->version = htod16(WL_MESH_IOCTL_VERSION);
	meshioc->id = htod16(cmd->id);
	meshioc->len = htod16(MESH_IOC_BUFSZ);
	pxtlv = meshioc->data;

	/* max data we can write, it will be decremented as we pack */
	buflen = MESH_IOC_BUFSZ;
	buflen_at_start = buflen;

	res = bcm_pack_xtlv_entry(&pxtlv,
		&buflen, WL_MESH_XTLV_JOIN, strlen(*argv), *argv, BCM_XTLV_OPTION_ALIGN32);

	if (res != BCME_OK) {
		goto exit;
	}

	/* adjust iocsz to the end of last data record */
	meshioc->len = (buflen_at_start - buflen);
	iocsz = sizeof(*meshioc) + meshioc->len;
	res = wlu_var_setbuf(wl, "mesh", meshioc, iocsz);

exit:
	free(meshioc);
	return res;
}

static int
wl_mesh_subcmd_peer_status(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv)
{
	int res = BCME_USAGE_ERROR;
	wl_mesh_ioc_t *meshioc = NULL;
	uint16 iocsz = sizeof(*meshioc) + MESH_IOC_BUFSZ;

	BCM_REFERENCE(wl);

	/*  alloc mem for ioctl headr +  tlv data  */
	meshioc = calloc(1, iocsz);
	if (meshioc == NULL) {
		return BCME_NOMEM;
	}

	/* make up mesh cmd ioctl header */
	meshioc->version = htod16(WL_MESH_IOCTL_VERSION);
	meshioc->id = htod16(cmd->id);
	meshioc->len = htod16(MESH_IOC_BUFSZ);

	if(*argv == NULL) { /* get */
		iocsz = sizeof(*meshioc);
		res = wl_mesh_get_ioctl(wl, meshioc, iocsz);
	}

	free(meshioc);
	return res;
}

static int
wl_mesh_subcmd_add_route(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv)
{
	int res = BCME_USAGE_ERROR;
	unsigned char data[2*ETHER_ADDR_LEN];
	struct ether_addr nexthop = ether_null;
	struct ether_addr dest = ether_null;
	wl_mesh_ioc_t *meshioc = NULL;
	uint16 buflen, buflen_at_start;
	uint16 iocsz = sizeof(*meshioc) + MESH_IOC_BUFSZ;
	uint8 *pxtlv;

	if (!wl_ether_atoe(*argv++, &dest)) {
		printf("%s: Invalid ether addr provided\n", __FUNCTION__);
		return BCME_USAGE_ERROR;
	}

	if (!wl_ether_atoe(*argv++, &nexthop)) {
		printf("%s: Invalid ether addr provided\n", __FUNCTION__);
		return BCME_USAGE_ERROR;
	}

	memcpy(data, dest.octet, ETHER_ADDR_LEN);
	memcpy(data + ETHER_ADDR_LEN, nexthop.octet, ETHER_ADDR_LEN);

	/*  alloc mem for ioctl headr +  tlv data  */
	meshioc = calloc(1, iocsz);
	if (meshioc == NULL) {
		return BCME_NOMEM;
	}

	/* make up mesh cmd ioctl header */
	meshioc->version = htod16(WL_MESH_IOCTL_VERSION);
	meshioc->id = htod16(cmd->id);
	meshioc->len = htod16(MESH_IOC_BUFSZ);
	pxtlv = meshioc->data;

	/* max data we can write, it will be decremented as we pack */
	buflen = MESH_IOC_BUFSZ;
	buflen_at_start = buflen;

	res = bcm_pack_xtlv_entry(&pxtlv,
		&buflen, WL_MESH_XTLV_ADD_ROUTE, (2 * ETHER_ADDR_LEN),
			data, BCM_XTLV_OPTION_ALIGN32);

	if (res != BCME_OK) {
		return res;
	}

	/* adjust iocsz to the end of last data record */
	meshioc->len = (buflen_at_start - buflen);
	iocsz = sizeof(*meshioc) + meshioc->len;
	res = wlu_var_setbuf(wl, "mesh", meshioc, iocsz);

	free(meshioc);
	return BCME_OK;
}

static int
wl_mesh_subcmd_del_route(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv)
{
	int res = BCME_USAGE_ERROR;
	struct ether_addr route = ether_null;
	wl_mesh_ioc_t *meshioc = NULL;
	uint16 buflen, buflen_at_start;
	uint16 iocsz = sizeof(*meshioc) + MESH_IOC_BUFSZ;
	uint8 *pxtlv;

	if (!wl_ether_atoe(*argv++, &route)) {
		printf("%s: Invalid ether addr provided\n", __FUNCTION__);
		return BCME_USAGE_ERROR;
	}

	/*  alloc mem for ioctl headr +  tlv data  */
	meshioc = calloc(1, iocsz);
	if (meshioc == NULL) {
		return BCME_NOMEM;
	}

	/* make up mesh cmd ioctl header */
	meshioc->version = htod16(WL_MESH_IOCTL_VERSION);
	meshioc->id = htod16(cmd->id);
	meshioc->len = htod16(MESH_IOC_BUFSZ);
	pxtlv = meshioc->data;

	/* max data we can write, it will be decremented as we pack */
	buflen = MESH_IOC_BUFSZ;
	buflen_at_start = buflen;

	res = bcm_pack_xtlv_entry(&pxtlv,
		&buflen, WL_MESH_XTLV_DEL_ROUTE, ETHER_ADDR_LEN, &route, BCM_XTLV_OPTION_ALIGN32);

	if (res != BCME_OK) {
		return res;
	}

	/* adjust iocsz to the end of last data record */
	meshioc->len = (buflen_at_start - buflen);
	iocsz = sizeof(*meshioc) + meshioc->len;
	res = wlu_var_setbuf(wl, "mesh", meshioc, iocsz);

	free(meshioc);
	return BCME_OK;
}

static int
wl_mesh_subcmd_add_filter(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv)
{
	int res = BCME_USAGE_ERROR;
	struct ether_addr route = ether_null;
	wl_mesh_ioc_t *meshioc = NULL;
	uint16 buflen, buflen_at_start;
	uint16 iocsz = sizeof(*meshioc) + MESH_IOC_BUFSZ;
	uint8 *pxtlv;

	if (!wl_ether_atoe(*argv++, &route)) {
		printf("%s: Invalid ether addr provided\n", __FUNCTION__);
		return BCME_USAGE_ERROR;
	}

	/*  alloc mem for ioctl headr +  tlv data  */
	meshioc = calloc(1, iocsz);
	if (meshioc == NULL) {
		return BCME_NOMEM;
	}

	/* make up mesh cmd ioctl header */
	meshioc->version = htod16(WL_MESH_IOCTL_VERSION);
	meshioc->id = htod16(cmd->id);
	meshioc->len = htod16(MESH_IOC_BUFSZ);
	pxtlv = meshioc->data;

	/* max data we can write, it will be decremented as we pack */
	buflen = MESH_IOC_BUFSZ;
	buflen_at_start = buflen;

	res = bcm_pack_xtlv_entry(&pxtlv,
		&buflen, WL_MESH_XTLV_ADD_FILTER, ETHER_ADDR_LEN, &route, BCM_XTLV_OPTION_ALIGN32);

	if (res != BCME_OK) {
		return res;
	}

	/* adjust iocsz to the end of last data record */
	meshioc->len = (buflen_at_start - buflen);
	iocsz = sizeof(*meshioc) + meshioc->len;
	res = wlu_var_setbuf(wl, "mesh", meshioc, iocsz);

	free(meshioc);
	return BCME_OK;
}

static int
wl_mesh_subcmd_enable_al_metric(void *wl, const wl_mesh_sub_cmd_t *cmd, char **argv)
{
	int res = BCME_OK;
	wl_mesh_ioc_t *meshioc;
	uint16 buflen;
	uint16 iocsz = sizeof(*meshioc) + MESH_IOC_BUFSZ;
	bcm_xtlv_t *pxtlv = NULL;

	/*  alloc mem for ioctl headr +  tlv data  */
	meshioc = calloc(1, iocsz);
	if (meshioc == NULL) {
		 return BCME_NOMEM;
	}

	/* make up mesh cmd ioctl header */
	meshioc->version = htod16(WL_MESH_IOCTL_VERSION);
	meshioc->id = htod16(cmd->id);
	meshioc->len = htod16(MESH_IOC_BUFSZ);
	pxtlv = (bcm_xtlv_t *)meshioc->data;

	if(*argv == NULL) { /* get */
		iocsz = sizeof(*meshioc) + sizeof(*pxtlv);
		res = wl_mesh_get_ioctl(wl, meshioc, iocsz);

	} else {	/* set */
		/* max tlv data we can write, it will be decremented as we pack */
		uint8 val =  atoi(*argv);
		/* save buflen at start */
		uint16 buflen_at_start;

		buflen = MESH_IOC_BUFSZ;
		buflen_at_start = buflen;

		/* we'll adjust final ioc size at the end */
		res = bcm_pack_xtlv_entry((uint8**)&pxtlv, &buflen, WL_MESH_XTLV_ENAB_AIRLINK,
			sizeof(uint8), &val, BCM_XTLV_OPTION_ALIGN32);

		if (res != BCME_OK) {
			goto fail;
		}

		/* adjust iocsz to the end of last data record */
		meshioc->len = (buflen_at_start - buflen);
		iocsz = sizeof(*meshioc) + meshioc->len;

		res = wlu_var_setbuf(wl, "mesh", meshioc, iocsz);
	}

fail:
	free(meshioc);
	return res;
}

static int
wl_mesh_control(void *wl, cmd_t *cmd, char **argv)
{
	int ret = BCME_USAGE_ERROR;
	char *mesh_query[2] = {"enable", NULL};
	char *mesh_en[3] = {"enable", "1", NULL};
	char *mesh_dis[3] = {"enable", "0", NULL};
	const wl_mesh_sub_cmd_t *meshcmd = &mesh_cmd_list[0];
	/* Skip the command name */
	UNUSED_PARAMETER(cmd);

	argv++;
	/* skip to cmd name after "mesh" */
	if (!*argv) {
		/* query mesh "enable" state */
		argv = mesh_query;
	} else if (*argv[0] == '1') {
		argv = mesh_en;
	} else if (*argv[0] == '0') {
		argv = mesh_dis;
	} else if (!strcmp(*argv, "-h") || !strcmp(*argv, "help")) {
		/* help , or -h* */
		return BCME_USAGE_ERROR;
	}

	while (meshcmd->name != NULL) {
		if (strcmp(meshcmd->name, *argv) == 0)  {
		/* dispacth cmd to appropriate handler */
			if (meshcmd->handler) {
				ret = meshcmd->handler(wl, meshcmd, ++argv);
			}
			return ret;
		}
		meshcmd++;
	}
	return BCME_OK;
}
