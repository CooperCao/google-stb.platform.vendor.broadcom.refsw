/**
 * @file
 * @brief
 * Ultra Low Power [ulp] base functions.
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

/**
 * @file
 * @brief
 * Cubby infrastructure to save restore module data while transitioning
 * between active and low power modes.
 */


/* ---- Include Files ---------------------------------------------------- */
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <d11.h>
#include <hndd11.h>
#include <sbhndarm.h>
#include <hndpmu.h>
#include <hndcpu.h>
#include <hndlhl.h>
#include <hndmem.h>
#include <sbchipc.h>
#include <sbgci.h>
#include <ulp.h>
#include <lpflags.h>
#include <fcbs.h>
#include <bcmdevs.h>
#include <saverestore.h>
#include <sbsdpcmdev.h>
#include <siutils.h>
#include <d11ucode.h>
#include <fcbsdata.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

/* 4 bytes to hold size of dynamic data */
#define SECTION_SZ_HOLDER	(sizeof(uint32))

#ifndef BCMULP_DISABLED
bool _bcmulp = TRUE;
#else
bool _bcmulp = FALSE;
#endif

/* private cubby context per phase1 cubby */
typedef struct ulp_p1_cubby_ctx {
	ulp_module_id_t mod_id;				/* module id. reqd to match p1 & p2 data */
	const ulp_p1_module_pubctx_t *cubby_pub_ctx;	/* public const ptr passed by modules */
	uint8 *cubby_dat_ptr;				/* Required for exit and recreate */
	uint32 csz;					/* cubby size */
	void *hdl;					/* hdl passed to ulp_p1_module_pubctx_t */
} ulp_p1_cubby_ctx_t;

/* private cubby context per phase2 cubby */
typedef struct ulp_p2_cubby_ctx {
	ulp_module_id_t mod_id;				/* module id. reqd to match p1 & p2 data */
	const ulp_p2_module_pubctx_t *cubby_pub_ctx;	/* public const ptr passed by modules */
	void *cubby_dat_ptr;				/* Required for exit and recreate */
} ulp_p2_cubby_ctx_t;

/* header for cache data */
typedef struct cache_hdr {
	uint crc;	/* crc of bytes below */
	uint gtotsz;	/* grand total size excluding crc */
} cache_hdr_t;

/* cubbylist. A module reserves a cubbylist so that other modules can reserve
 * a cubby when the box is allocated.
 */
typedef struct ulp_cubby_list {
	uint32 type;		/* module type, which provides this cubby. 0 is invalid */
	uint32 max;		/* max number of modules who can reserve cubby */
	uint32 cubby_ctx_sz;	/* actual size of context */
	uint32 cur;		/* current idx in cubby ctx list */
	uint totsz_dat;		/* total data to be allocated for the box */
	uint8 *cache_head;	/* head of the box / cache data */
	uint8 *cache_cur_ptr;	/* current ptr of the box / cache data */
	void *cubby_ctx;	/* These are the cubby contexts */
} ulp_cubby_list_t;

/* common ulp info */
typedef struct ulp_info {
	osl_t *osh;		/* osh */
	si_t *sih;		/* sih */
	uint32 max;		/* max number of lists */
	ulp_cubby_list_t *ucl;	/* this is the list of ulp_cubby_list_t */
	uint16 driver_blk_addr;
	uint16 cubby_cache_mem;
	const shmdefs_t *shmdefs;
} ulp_info_t;

/* Cubby cache memory */
enum {
	CUBBYCACHE_SHM = 0,
	CUBBYCACHE_BM = 1,
	};

#define CUBBYCACHE_BM_ADDRESS (FCBS_DS1_BM_DATPTR_BASE + FCBS_DS1_BM_DAT_SZ)

/* given the cubby list, get the cubby context identified with idx */
#define ULP_GET_CUBBYCTX(ucl, idx)	\
	(void *)(((uint8 *)(ucl)->cubby_ctx) + \
		((ucl)->cubby_ctx_sz) * (idx))

/* ---- Private Variables ------------------------------------------------ */
static ulp_info_t *g_ulp_info = NULL;

/* ---- Private Function Prototypes -------------------------------------- */
static ulp_info_t *ulp_get_uinfo(void);
static void ulp_set_uinfo(ulp_info_t *ui);
static int ulp_reserve_clist(uint32 type, uint32 max_cubbies,
	uint32 cubby_ctx_sz);
static void *ulp_reserve_cubby(uint32 type, ulp_module_id_t mod_id);
static int ulp_infra_init(osl_t *osh, si_t *sih);
static void ulp_infra_deinit(osl_t *osh, si_t *sih);
static ulp_cubby_list_t *ulp_get_clist(ulp_info_t *ui, uint32 type);
static void *ulp_get_cubby_ctx(ulp_info_t *ui, uint32 type, ulp_module_id_t mid);
static uint ulp_p1_fetch_cubby_sizes(ulp_cubby_list_t *ucl, ulp_ext_info_t *einfo);
static int ulp_p1_retrieve(osl_t *osh);
static uint8 *ulp_p2_get_cache_dat(ulp_module_id_t mid);
static int ulp_post_ulpucode_switch(osl_t *osh, ulp_ext_info_t *einfo);
static void ulp_cleanup_cache_head(osl_t *osh, ulp_cubby_list_t *ucl);

#ifdef ULP_DBG_ON
static void ulp_dump_cache(uint8 *ulp_cache);
#endif

/* ---- Functions -------------------------------------------------------- */
/* returns ulp info global ptr */
static ulp_info_t*
BCMRAMFN(ulp_get_uinfo)(void)
{
	return g_ulp_info;
}

/* sets ulp info global ptr */
static void
BCMRAMFN(ulp_set_uinfo)(ulp_info_t *ui)
{
	ULP_DBG(("%s\n", __FUNCTION__));
	g_ulp_info = ui;
}

static int
ulp_reserve_clist(uint32 type, uint32 max_cubbies, uint32 cubby_ctx_sz)
{
	int err = BCME_OK;
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *ucl = NULL;

	if (!ui) {
		err = BCME_ERROR;
		goto done;
	}

	/* assumptions:
	 * a) module deregistration not required
	 * b) 'type' is used to index into clist
	 * c) no duplicate clist reserve for a type
	 */
	if (type >= ui->max) {
		err = BCME_RANGE;
		goto done;
	}

	ucl = &ui->ucl[type];

	/* free mlist index found. allocate max module structures */
	if ((ucl->cubby_ctx = (void *) MALLOCZ(ui->osh,
		(cubby_ctx_sz * max_cubbies))) == NULL) {
		err = BCME_NOMEM;
		goto done;
	}
	ucl->type = type;
	ucl->max = max_cubbies;
	ucl->cubby_ctx_sz = cubby_ctx_sz;

	ULP_DBG(("%s: type: %d, max_cubbies: %d, esz: %d, ucl:%p\n",
		__FUNCTION__, type, max_cubbies, cubby_ctx_sz, ucl));
done:
	return err;
}

/* initializes ulp_info and cubbylist structures
 * called [ONLY] by ulp_module_init
 */
static int
ulp_infra_init(osl_t *osh, si_t *sih)
{
	int err = BCME_OK;
	ulp_info_t *ui = NULL;

	if ((ui = (ulp_info_t *) MALLOCZ(osh,
		sizeof(ulp_info_t) +
		(sizeof(ulp_cubby_list_t) * ULP_PHASE_LAST))) == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	ui->max = ULP_PHASE_LAST;
	ui->sih = sih;
	ui->osh = osh;
	ui->ucl = (ulp_cubby_list_t *) (((uint8*)(ui)) + sizeof(ulp_info_t));

	ulp_set_uinfo(ui);

	ULP_DBG(("%s: NEW ui, osh:%p\n", __FUNCTION__, osh));
done:
	return err;
}

/* de-initializes ulp_info and cubbylist structures */
static void
ulp_infra_deinit(osl_t *osh, si_t *sih)
{
	ulp_info_t *ui = ulp_get_uinfo();
	if (!ui)
		goto done;

	MFREE(osh, ui, sizeof(ulp_info_t) +
		(sizeof(ulp_cubby_list_t) * ULP_PHASE_LAST));
	ulp_set_uinfo(NULL);

	ULP_DBG(("%s: deinited!\n", __FUNCTION__));
done:
	return;
}

static ulp_cubby_list_t *
ulp_get_clist(ulp_info_t *ui, uint32 type)
{
	if (!ui || (type >= ui->max))
		return NULL;

	return &ui->ucl[type];
}

static void *
ulp_get_cubby_ctx(ulp_info_t *ui, uint32 type, ulp_module_id_t mid)
{
	ulp_cubby_list_t *ucl = ulp_get_clist(ui, type);
	ulp_p2_cubby_ctx_t *ucc = NULL;
	ulp_p2_cubby_ctx_t *p2cc = NULL;
	int i = 0;

	if (!ui)
		goto done;

	ucl = &ui->ucl[type];

	for (i = 0; i < ucl->cur; i++) {
		ucc = ULP_GET_CUBBYCTX(ucl, i);
		if (ucc->mod_id == mid) {
			p2cc = ucc;
			break;
		}
	}
done:
	return p2cc;
}

uint
ulp_p1_get_max_box_size(ulp_ext_info_t *einfo)
{
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *ucl = ulp_get_clist(ui, ULP_PHASE1);

	return ulp_p1_fetch_cubby_sizes(ucl, einfo);
}

static uint
ulp_p1_fetch_cubby_sizes(ulp_cubby_list_t *ucl, ulp_ext_info_t *einfo)
{
	int sz = 0, isz = 0;
	ulp_p1_cubby_ctx_t *p1cc = NULL;
	const ulp_p1_module_pubctx_t	*pubc = NULL;
	int i = 0;
	ULP_DBG(("%s: enter\n", __FUNCTION__));
	for (i = 0; i < ucl->cur; i++) {
		p1cc = ULP_GET_CUBBYCTX(ucl, i);
		pubc = p1cc->cubby_pub_ctx;
		if (pubc->fn_cubby_size) {
			isz = (pubc->fn_cubby_size)(p1cc->hdl, einfo);
			p1cc->csz = isz;
		} else {
			isz = 0;
		}
		ULP_DBG(("updateSizes: mid: %d, p1cc: %p, sz: %d [%s]\n",
			p1cc->mod_id, p1cc, isz, (pubc->flags & MODCBFL_CTYPE_DYNAMIC)?
			"dynamic" : "static"));
		if (pubc->flags & MODCBFL_CTYPE_DYNAMIC) {
			/* add size holder for dynamic modules */
			isz += SECTION_SZ_HOLDER;
		}

		sz += isz;
	}

	ucl->totsz_dat = sz;

	ULP_DBG(("%s: exit sz:%d\n", __FUNCTION__, sz));
	return sz;
}

static uint8 *
ulp_p2_get_cache_dat(ulp_module_id_t mid)
{
	uint8 *p2r_ptr = NULL;
	ulp_p2_cubby_ctx_t *p2cc = NULL;
	ulp_info_t *ui = ulp_get_uinfo();

	p2cc = (ulp_p2_cubby_ctx_t *) ulp_get_cubby_ctx(ui, ULP_PHASE2,
		mid);

	if (!p2cc) {
		goto done;
	}

	p2r_ptr = p2cc->cubby_dat_ptr;

	ULP_DBG(("%s: mid: %d, p2ptr:%p\n", __FUNCTION__, mid, p2r_ptr));
done:
	return p2r_ptr;
}

/* initializes ULP module, reserves cubby lists for phase1 and phase2
 * called [ONLY] by si_doattach
 * Returns:    BCME_OK on success, else error code.
 * Calls phase 1 ulp retrieve function, to collect module's data from AON memory to a
 * local buffer/box [after coming out of DS1 and in warm boot path]
 */
int
ulp_module_init(osl_t *osh, si_t *sih)
{
	int err = BCME_OK;
	uint16 mac_corerev;
	ulp_info_t *ui = ulp_get_uinfo();

	/* if the module is already inited, [indicates that infra init and
	* exit is already done]. then exit
	*/
	if (NULL != ui) {
		ULP_DBG(("%s: already inited!\n", __FUNCTION__));
		goto done;
	}

	err = ulp_infra_init(osh, sih);

	/* TODO: Provide option to configure this using build string or iovar */
	ui = ulp_get_uinfo();

	si_setcore(sih, D11_CORE_ID, 0);
	d11shm_select_ucode_ulp(&ui->shmdefs, si_corerev(sih));

	if (CHIPID(ui->sih->chip) == BCM43012_CHIP_ID) {
		ui->cubby_cache_mem = CUBBYCACHE_BM;
	} else {
		ui->cubby_cache_mem = CUBBYCACHE_SHM;
	}

	if (BCME_OK != err)
		goto done;

	/* reserve Phase1 ULP module */
	/* phase 1 is storage of data used only in DS0 */
	err = ulp_reserve_clist(ULP_PHASE1,
		ULP_MAX_P1_MODULES, sizeof(ulp_p1_cubby_ctx_t));

	if (BCME_OK != err)
		goto done;

	/* reserve Phase2 ULP module */
	/* phase 2 is storage of data used in both DS0 and DS1 modes */
	err = ulp_reserve_clist(ULP_PHASE2,
		ULP_MAX_P2_MODULES, sizeof(ulp_p2_cubby_ctx_t));

	if (!si_is_warmboot()) {
		ULP_DBG(("%s: COLDboot!\n", __FUNCTION__));
		goto done;
	}
	ULP_DBG(("%s: warmboot!\n", __FUNCTION__));

	hndd11_read_shm(sih, 0, M_MACHW_VER(ui), &mac_corerev);

	/* ULP exit routine */
	ulp_exit(sih, osh);

	/* ULP Phase 1 exit routine */
	err = ulp_p1_retrieve(osh);

done:
	if (err)
		ulp_infra_deinit(osh, sih);

	return err;
}

/* reserves a cubby for module: "mid" from a list identified by "type".
 * returns cubby ctx ptr
 */
static void *
ulp_reserve_cubby(uint32 type, ulp_module_id_t mod_id)
{
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *ucl = NULL;
	void *cubby_ctx = NULL;

	if (!ui) {
		goto done;
	}
	/* check and get module list */
	if (type >= ui->max) {
		ULP_ERR(("%s: type: %d >= ui->max %d\n", __FUNCTION__, type,
			ui->max));
		goto done;
	}

	ucl = &ui->ucl[type];

	/* check and get module context */
	if (ucl->cur >= ucl->max) {
		ULP_ERR(("%s: ucl->cur: %d >= ucl->max %d\n", __FUNCTION__,
			ucl->cur, ucl->max));
		goto done;
	}
	cubby_ctx = ULP_GET_CUBBYCTX(ucl, ucl->cur);

	ucl->cur++;

done:
	ULP_DBG(("%s, cur: %d ucl: %p, cubby_ctx: %p, max:%d\n",
		__FUNCTION__, ucl->cur-1, ucl, cubby_ctx, ucl->max));
	return cubby_ctx;
}

/* register the given module "mid" for phase1 processing with
 *	callbacks/handle. This will do following.
 *	a) reserve a cubby in phase1.
 *	b) if warm boot and if a cache ptr is available, call fn_exit callbk
 *	with respective "mid" cubby-ptrs for phase1 and phase2.
 *	c) Note that, the callbacks are called in the exact same sequence
 *	as that was done for ulp_enter_pre_ulpucode_switch.
 *	d) Also note that ulp_p1_module_register call occurs after
 *	ulp_module_init() and ulp_p2_retrieve()
 *
 * Note that,
 *	the user must allocate and maintain cubby_pub_ctx storage and the
 *	storage can't be reclaimable. This will also make sure that the const
 *	struct can goto ROM saving RAM and can be referenced directly.
 *
 *	returns status BCME_xxx.
 */
int
ulp_p1_module_register(ulp_module_id_t mid, const ulp_p1_module_pubctx_t *cubby_pub_ctx,
	void *hdl)
{
	int err = BCME_OK;
	int csz = 0;
	uint8 *p2_ptr = NULL;
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_p1_cubby_ctx_t *p1cc = NULL;
	ulp_cubby_list_t *ucl = NULL;

	if ((p1cc = ulp_reserve_cubby(ULP_PHASE1, mid)) == NULL) {
		ULP_ERR(("%s: NO p1cc!\n", __FUNCTION__));
		err = BCME_NORESOURCE;
		goto done;
	}
	ULP_DBG(("%s: enter, \"0x%x\" fl:%x szfn: %p, p1cc: %p\n",
		__FUNCTION__, mid, cubby_pub_ctx->flags,
		cubby_pub_ctx->fn_cubby_size, p1cc));

	p1cc->mod_id = mid;
	p1cc->cubby_pub_ctx = cubby_pub_ctx;
	p1cc->hdl = hdl;

	if (si_is_warmboot()) {
		ULP_DBG(("%s: warmboot!\n", __FUNCTION__));

		ucl = ulp_get_clist(ui, ULP_PHASE1);

		if (ucl->cache_head == NULL) {
			ULP_DBG(("%s: cache_head NULL!\n", __FUNCTION__));
			goto done;
		}

		/* get p2 ptr if any */
		p2_ptr = ulp_p2_get_cache_dat(mid);
		/* call the exit cb */
		p1cc->cubby_dat_ptr = ucl->cache_cur_ptr;

		/* update ptr_offset for next module */
		if (cubby_pub_ctx->flags & MODCBFL_CTYPE_DYNAMIC) {
			/* get size of this dynamic section. reuse csz */
			csz = *((uint32 *)ucl->cache_cur_ptr);
			/* skip size */
			p1cc->cubby_dat_ptr += SECTION_SZ_HOLDER;
			ULP_DBG(("%s: dynamic! csz: %d, mcp: %p\n",
				__FUNCTION__, csz, p1cc->cubby_dat_ptr));

			/* note: no error handling done here */
			if (csz && cubby_pub_ctx->fn_exit) {
				(cubby_pub_ctx->fn_exit)(p1cc->hdl,
					p1cc->cubby_dat_ptr, p2_ptr);
			}
			csz += SECTION_SZ_HOLDER;
		} else {
			/* note: no error handling done here */
			if (cubby_pub_ctx->fn_exit)
				(cubby_pub_ctx->fn_exit)(p1cc->hdl,
					p1cc->cubby_dat_ptr, p2_ptr);
			if ((cubby_pub_ctx->fn_cubby_size)) {
				csz = (cubby_pub_ctx->fn_cubby_size)(p1cc->hdl, NULL);
			} else
				csz = 0;
			ULP_DBG(("%s: static! csz: %d, mcp: %p\n",
				__FUNCTION__, csz, p1cc->cubby_dat_ptr));
		}
#ifdef ULP_DBG_ON
		prhex("modulech", ucl->cache_cur_ptr, csz);
#endif
		/* update for next module */
		ucl->cache_cur_ptr += csz;
		ULP_DBG(("%s cur sz: %d, next cache_cur_ptr:%p\n",
			__FUNCTION__, csz, ucl->cache_cur_ptr));
	} else {
		ULP_DBG(("%s coldboot\n", __FUNCTION__));
	}

done:
	return err;
}

/* phase 1 ulp entry function, to collect data to save to AON memory before going to DS1
 * returns: status BCME_xxx.
 */
int
ulp_enter_pre_ulpucode_switch(ulp_ext_info_t *einfo)
{
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *ucl = ulp_get_clist(ui, ULP_PHASE1);
	ulp_p1_cubby_ctx_t *p1cc = NULL;
	const ulp_p1_module_pubctx_t *pubc = NULL;
	uint8 *ulp_cache = NULL, *cubby_cur_ptr = NULL;
	int csz = 0, isz = 0;
	int err = BCME_OK;
	cache_hdr_t *chdr = NULL;
	uint32 *szholder = NULL;
	int i = 0;

	ULP_DBG(("%s: enter\n", __FUNCTION__));

	if (ucl->cache_head) {
		// in case if cache_head was already populated, return error
		ULP_ERR(("%s:cache_head is not null! %p\n", __FUNCTION__, ucl->cache_head));
		err = BCME_USAGE_ERROR;
		goto enter_done;
	}

	/* A. get and update sizes from all modules for retention-storage */
	csz = ulp_p1_fetch_cubby_sizes(ucl, einfo);
	ucl->totsz_dat = csz + sizeof(*chdr);
	WORD_ALIGN(ucl->totsz_dat);

	/* B. allocate structure corr to all modules with this size + hdr size */
	if ((ulp_cache = MALLOCZ(ui->osh, ucl->totsz_dat)) == NULL) {
		ULP_ERR(("wl: %s: out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(ui->osh)));
		err = BCME_NOMEM;
		goto enter_done;
	}

	chdr = (cache_hdr_t *)ulp_cache;

	/* C. call all ulp entry cb's registered by module's */
	/* C.1. fill g.total size [this doesn't include chdr] */
	chdr->gtotsz = csz;

	ULP_DBG(("%s: ulp cache basep: %p\n", __FUNCTION__, ulp_cache));

	/* C.2. call all module's fn_enter with corr cubby_cur_ptr */
	cubby_cur_ptr = ulp_cache + sizeof(*chdr);

	for (i = 0; i < ucl->cur; i++) {
		p1cc = ULP_GET_CUBBYCTX(ucl, i);
		pubc = p1cc->cubby_pub_ctx;

		ULP_DBG(("%s: midx: %d, ulpcb:%p, flag: %s\n", __FUNCTION__,
			p1cc->mod_id, pubc,
			(pubc->flags & MODCBFL_CTYPE_DYNAMIC)?"dyn":"static"));
		/* here modules have to take cubby_cur_ptr and update their data.
		*/
		/* specifically for dynamic */
		if (pubc->flags & MODCBFL_CTYPE_DYNAMIC) {
			szholder = (uint32*)(cubby_cur_ptr);
			*szholder = p1cc->csz;
			ULP_DBG(("mid:%d dyn sz: %d\n", p1cc->mod_id, *szholder));
			cubby_cur_ptr += SECTION_SZ_HOLDER;
			isz = p1cc->csz;
			/* note that,  if p1cc->csz 0 in this case, that will
			 * be written into SECTION_SZ_HOLDER because this
			 * module is dynamic.
			 */
		} else
			isz = p1cc->csz;
		if ((pubc->fn_enter_pre_ulpucode)) {
			/* note: err handling not done because if a module doesn't
			 * fill, this function ignores.
			 */
			if (p1cc->csz)
				(pubc->fn_enter_pre_ulpucode)(p1cc->hdl, einfo, cubby_cur_ptr);
			else {
				(pubc->fn_enter_pre_ulpucode)(p1cc->hdl, einfo, NULL);
				/* note: if csz is NULL, (ie if a module doesnt have any cubby
				* info to store
				* in SHM but still needs an enter cb during DS1 entry),
				* cubby_cur_ptr should be NULL as well. Eg: BTCX/LTECX module
				*/
			}
		}
		ULP_DBG(("%s: fn_enter_pre_ulpucode: cubby_cur_ptr :%p!, sz: %d\n",
			__FUNCTION__, cubby_cur_ptr, isz));

#ifdef ULP_DBG_ON
		prhex("module input", cubby_cur_ptr, p1cc->csz);
#endif /* ULP_DBG_ON */
		cubby_cur_ptr += isz;
	}
	/* C.3. add a crc */
	chdr->crc = hndcrc32(ulp_cache + sizeof(*chdr), chdr->gtotsz, CRC32_INIT_VALUE);
	/* TBD chdr->crc = ?? */
	ULP_DBG(("%s: caching..., cache: %p, sz: %d, crc: %x\n", __FUNCTION__,
		ulp_cache, csz, chdr->crc));

#ifdef ULP_DBG_ON
	ulp_dump_cache(ulp_cache);
	prhex("ulp_enter-cache", ulp_cache, csz);
#endif /* ULP_DBG_ON */

	/* D. store the ulp_cache to ulp module info for later transfer to shm */
	ucl->cache_head = ulp_cache;
	ucl->totsz_dat = csz + sizeof(*chdr);

enter_done:
	return err;
}

/* Transfer the cubby cache info to SHM or BM which is ON in ULP */
static int
ulp_save_ulp_cache(osl_t *osh, ulp_info_t *ui, ulp_cubby_list_t *ucl)
{
	int ret = BCME_OK;
	uint origidx = 0;
	d11regs_t *regs = NULL;

	origidx = si_coreidx(ui->sih);

	/* copy the driver block info to ulp_info_t structure */
	hndd11_read_shm(ui->sih, 0, M_DRVR_UCODE_IF_PTR(ui), &ui->driver_blk_addr);

	if (ui->cubby_cache_mem == CUBBYCACHE_SHM) {
		ULP_DBG(("%s: cubby cache memory - SHM\n", __FUNCTION__));

		/* transfer ulp_cache to shm */
		if (CHIPID(ui->sih->chip) == BCM43012_CHIP_ID)
			ulp_write_driver_shmblk(ui->sih, 0, M_DRIVER_BLOCK(ui), ucl->cache_head,
				ucl->totsz_dat);
		else
			hndd11_copyto_shm(ui->sih, 0, M_WOWL_ULP_SW_DAT_BLK,
				ucl->cache_head, ucl->totsz_dat);
	} else if (ui->cubby_cache_mem == CUBBYCACHE_BM) {
		/* transfer ulp_cache to BM */
		ULP_DBG(("%s: cubby cache memory - BM\n", __FUNCTION__));
		regs = si_setcore(ui->sih,  D11_CORE_ID, 0);
		WORD_ALIGN(ucl->totsz_dat);
		hndd11_bm_write(osh, regs, CUBBYCACHE_BM_ADDRESS, (ucl->totsz_dat)/4,
				(uint32 *)ucl->cache_head);
		si_setcoreidx(ui->sih, origidx);
	} else {
		ULP_ERR(("%s: cubby cache memory not specified\n", __FUNCTION__));
		ret = BCME_BADOPTION;
	}

	return ret;
}


/* transfer the cubby cache info stored in ucl->cache_head to shm */
static int
ulp_post_ulpucode_switch(osl_t *osh, ulp_ext_info_t *einfo)
{
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *ucl = ulp_get_clist(ui, ULP_PHASE1);
	ulp_p1_cubby_ctx_t *p1cc = NULL;
	const ulp_p1_module_pubctx_t *pubc = NULL;
	int i = 0;
	int ret = BCME_OK;

	ULP_DBG(("%s: transferring to AON memory\n", __FUNCTION__));

	if ((ret = ulp_save_ulp_cache(osh, ui, ucl)) != BCME_OK) {
		goto exit;
	}

	/* B. cleanup */
	ulp_cleanup_cache_head(ui->osh, ucl);

	/* C. call post ulp callbks registered by modules */
	for (i = 0; i < ucl->cur; i++) {
		p1cc = ULP_GET_CUBBYCTX(ucl, i);
		pubc = p1cc->cubby_pub_ctx;
		if ((pubc->fn_enter_post_ulpucode)) {
			/* note: err handling not done because if a module doesn't
			 * fill, this function ignores.
			 */
			(pubc->fn_enter_post_ulpucode)(p1cc->hdl, einfo);
		}
	}

exit:
	return ret;
}

/* Restore the cubby cache info from SHM or BM */
static int
ulp_restore_cubby_cache(osl_t *osh, ulp_info_t *ui, void *buf, uint sz)
{
	int ret = BCME_OK;
	uint origidx = 0;
	d11regs_t *regs = NULL;

	origidx = si_coreidx(ui->sih);

	ULP_DBG(("%s: Restoring %d bytes from cubby cache\n", __FUNCTION__, sz));

	if (ui->cubby_cache_mem == CUBBYCACHE_SHM) {
		ULP_DBG(("%s: cubby cache memory - SHM\n", __FUNCTION__));
		if (CHIPID(ui->sih->chip) == BCM43012_CHIP_ID) {
			hndd11_read_shm(ui->sih, 0, M_DRVR_UCODE_IF_PTR(ui), &ui->driver_blk_addr);
			ulp_read_driver_shmblk(ui->sih, 0, M_DRIVER_BLOCK(ui), buf, sz);
		} else {
			hndd11_copyfrom_shm(ui->sih, 0, M_WOWL_ULP_SW_DAT_BLK, buf, sz);
		}
	} else if (ui->cubby_cache_mem == CUBBYCACHE_BM) {
		ULP_DBG(("%s: cubby cache memory - BM\n", __FUNCTION__));
		regs = si_setcore(ui->sih,  D11_CORE_ID, 0);
		ASSERT(IS_WORD_ALIGN(sz));
		hndd11_bm_read(osh, regs, CUBBYCACHE_BM_ADDRESS, sz/4, (uint32 *)buf);
		si_setcoreidx(ui->sih, origidx);
	} else {
		ULP_ERR(("%s: cubby cache memory not specified\n", __FUNCTION__));
		ret = BCME_BADOPTION;
	}

	return ret;
}

/* allocate box based on retrieved size, and retrieve whole data from shm/AON
 * memory. store box into the cubbylist's cache_head and update cur ptr.
 * called ONLY by ulp_module_init and ONLY in warm_boot. No necessory error
 * checking for ui required here because it is already done by caller.
 */
static int
ulp_p1_retrieve(osl_t *osh)
{
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *uci_p1 = ulp_get_clist(ui, ULP_PHASE1);
	int err = BCME_OK;
	cache_hdr_t chdr;
	uint8 *ulp_cache = NULL;
	uint totsz = 0;
	uint32 calc_crc = 0;

	/* A. retrieve the header structure from shm */
	/* get the header first (&chdr) */
	ulp_restore_cubby_cache(osh, ui, &chdr, sizeof(chdr));

	/* first check on size */
	totsz = (chdr.gtotsz + sizeof(chdr));
	WORD_ALIGN(totsz);
	if (totsz > M_WOWL_ULP_SW_DAT_BLK_MAX_SZ) {
		err = BCME_ERROR;
		goto done;
	}
	/* D. allocate cache_data */
	if ((ulp_cache = MALLOCZ_NOPERSIST(osh, totsz)) == NULL) {
		ULP_ERR(("wl: %s: out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		err = BCME_NOMEM;
		goto done;
	}
	ULP_DBG(("%s: hdrsz: %d, max: %d!, cache: %p\n", __FUNCTION__, totsz,
		M_WOWL_ULP_SW_DAT_BLK_MAX_SZ, ulp_cache));

	/* E. retrieve the data from shm and store cache_data */
	ulp_restore_cubby_cache(osh, ui, ulp_cache, totsz);
#ifdef ULP_DBG_ON
	prhex("ulp_exit-cache", ulp_cache, totsz);
#endif /* ULP_DBG_ON */

	/* F. check CRC : TBD */
	calc_crc = hndcrc32(ulp_cache + sizeof(chdr), chdr.gtotsz, CRC32_INIT_VALUE);

	if (calc_crc != chdr.crc) {
		ULP_ERR(("%s: crc error: read:%x, calc:%x\n", __FUNCTION__, chdr.crc,
			calc_crc));
		err = BCME_DECERR;
		goto done;
	} else {
		ULP_DBG(("%s: crc MATCHED: read:%x, calc:%x\n", __FUNCTION__, chdr.crc,
			calc_crc));
	}

	/* G. update mod_base ptrs and leave. ulp_p1_register should take care now. */
	uci_p1->cache_head = ulp_cache;

	/* First location contains total sz of static dat in bytes */
	uci_p1->cache_cur_ptr = ulp_cache + sizeof(chdr);

	ULP_DBG(("%s: cdat: %p, mcp: %p\n", __FUNCTION__, uci_p1->cache_head,
		uci_p1->cache_cur_ptr));
done:
	return err;
}

/* called from assoc recreate to populate data into required structure's of
* registered modules. Mainly this will be useful for bsscfg related functions
* beucase, fn_exit() called in wlp_p1_module_register() will not have bsscfg's
* in place.
* returns: status: BCME_xxx now, always BCME_OK
*/
int
ulp_recreate(ulp_ext_info_t *einfo)
{
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_p1_cubby_ctx_t *p1cc = NULL;
	const ulp_p1_module_pubctx_t	*pubc = NULL;
	int err = BCME_OK;
	uint8 *p2_ptr = NULL;
	ulp_module_id_t cur_mid = 0;

	ULP_DBG(("%s enter\n", __FUNCTION__));
	for (cur_mid = 0; cur_mid < ULP_MODULE_ID_LAST; cur_mid++) {
		p1cc = ulp_get_cubby_ctx(ui, ULP_PHASE1, cur_mid);
		if (p1cc) {
			pubc = p1cc->cubby_pub_ctx;
			/* get p2 ptr if any */
			p2_ptr = ulp_p2_get_cache_dat(p1cc->mod_id);
			ULP_DBG(("%s: mod: mid: %d\n", __FUNCTION__, p1cc->mod_id));
			/* note: err handling not done because if a module doesn't
			 * fill, this function ignores.
			 */
			if (pubc->fn_recreate) {
				err = (pubc->fn_recreate)(p1cc->hdl, einfo,
					p1cc->cubby_dat_ptr, p2_ptr);
				if (err != BCME_OK)
					goto done;
			}
		}
	}
done:
	return err;
}

static
void ulp_cleanup_cache_head(osl_t *osh, ulp_cubby_list_t *ucl)
{
	if (ucl->cache_head) {
		ULP_DBG(("%s: freeing ucl type %d cache_head: %p, sz: %d\n",
			__FUNCTION__, ucl->type, ucl->cache_head, ucl->totsz_dat));
		MFREE(osh, ucl->cache_head, ucl->totsz_dat);
		ucl->cache_head = NULL;
		ucl->totsz_dat = 0;
	}
}

/* cleanup cache pointers [box's] allocated for p1 and p2 phases */
void
ulp_cleanup_cache(void)
{
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *ucl = ulp_get_clist(ui, ULP_PHASE1);

	ULP_DBG(("%s enter\n", __FUNCTION__));

	// phase1 cleanup
	ulp_cleanup_cache_head(ui->osh, ucl);

	ucl = ulp_get_clist(ui, ULP_PHASE2);

	// phase2 cleanup
	ulp_cleanup_cache_head(ui->osh, ucl);
}

/* register the given module "mid" for phase2 processing with
	callbacks/handle. This will reserve a cubby in phase2.
	returns status BCME_xxx.
*/
int
ulp_p2_module_register(ulp_module_id_t mid, const ulp_p2_module_pubctx_t *mod_ctx)
{
	int err = BCME_OK;
	ulp_p2_cubby_ctx_t *p2cc = NULL;
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *ucl = ulp_get_clist(ui, ULP_PHASE2);

	if ((p2cc = ulp_reserve_cubby(ULP_PHASE2, mid)) == NULL) {
		ULP_ERR(("%s:NO p2cc!\n", __FUNCTION__));
		err = BCME_NORESOURCE;
		goto done;
	}
	ULP_DBG(("%s: enter, \"0x%x\" osh:%p, retrfn: %p\n",
		__FUNCTION__, mid, ui->osh, mod_ctx->fn_p2_retriv));

	p2cc->mod_id = mid;
	p2cc->cubby_pub_ctx = mod_ctx;
	ucl->totsz_dat += mod_ctx->size;

	ULP_DBG(("%s: enter, \"0x%x\" size:%d ucl: %p, totsz: %d\n",
		__FUNCTION__, mid, mod_ctx->size, ucl, ucl->totsz_dat));

done:
	return err;
}

/* Allocate a "box" and trigger retrieval of phase2 data, by calling all registered p2 functions.
 * Call to this function occurs just before wowl/ulp ucode gets replaced with
 * full ucode. [wlc_bmac_process_ucode_sr]
 * "phase2 data" means the data shared by wowl ucode and driver. This data
 * would have plumbed for wowl-ucode in the interface shm's given by wowl-ucode
 * which need not be duplicated using p1 AON memory
 * returns status BCME_xxx.
 */
int
ulp_p2_retrieve(void *ctx)
{
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *ucl = ulp_get_clist(ui, ULP_PHASE2);
	ulp_p2_cubby_ctx_t *p2cc = NULL;
	const ulp_p2_module_pubctx_t	*pubc = NULL;
	uint8 *ulp_p2_cache = NULL;
	int err = BCME_OK;
	int i = 0;
	if (!si_is_warmboot()) {
		ULP_DBG(("%s: COLDboot!\n", __FUNCTION__));
		goto done;
	}

	ULP_DBG(("%s:warmboot! enter ctx: %p, ucl: %p sz: %d\n", __FUNCTION__,
		ctx, ucl, ucl->totsz_dat));

	/* B. allocate structure to osh->cmn->cache_data */
	if ((ulp_p2_cache = MALLOCZ_NOPERSIST(ui->osh, ucl->totsz_dat)) == NULL) {
		ULP_ERR(("wl: %s: out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(ui->osh)));
		err = BCME_NOMEM;
		goto done;
	}
	ucl->cache_head = ulp_p2_cache;

	for (i = 0; i < ucl->cur; i++) {
		p2cc = ULP_GET_CUBBYCTX(ucl, i);
		pubc = p2cc->cubby_pub_ctx;
		p2cc->cubby_dat_ptr = ulp_p2_cache;
		ULP_DBG(("%s: up2ctx:%p, p2cptr:%p, fn:%p\n", __FUNCTION__,
			p2cc, ulp_p2_cache, (pubc->fn_p2_retriv)));
		/* note: err handling not done because if a module doesn't
		 * fill, this function ignores.
		 */
		if (NULL != pubc->fn_p2_retriv) {
			(pubc->fn_p2_retriv)(ctx, ui->osh, p2cc->cubby_dat_ptr);
		}
		ulp_p2_cache += pubc->size;
	}

done:
	return err;
}

static void
ulp_disable_interrupts(si_t *sih, osl_t *osh)
{
	uint32 savecore, i;
	cm3regs_t *armregs;
	sdpcmd_regs_t *sdioregs;
	d11regs_t *d11regs;
	gciregs_t *gciregs;
	pmuregs_t *pmuregs;

	savecore = si_coreidx(sih);

#if defined(__ARM_ARCH_7M__)
	/* Disable ARM interrupts */
	armregs = si_setcore(sih, ARMCM3_CORE_ID, 0);
	W_REG(osh, &armregs->isrmask, 0);
	W_REG(osh, &armregs->nmimask, 0);
#endif

	/* Disable SDIOD interrupts */
	sdioregs = si_setcore(sih, SDIOD_CORE_ID, 0);
	W_REG(osh, &sdioregs->hostintmask, 0);
	W_REG(osh, &sdioregs->intmask, 0);
	W_REG(osh, &sdioregs->sbintmask, 0);
	W_REG(osh, &sdioregs->funcintmask, 0);
	W_REG(osh, &sdioregs->tosbmailbox, 0);

	/* Disable D11 interrupts */
	d11regs = si_setcore(sih, D11_CORE_ID, 0);
	W_REG(osh, &d11regs->altmacintmask, 0);
	W_REG(osh, &d11regs->macintmask, 0);
	for (i = 0; i < D11_DMA_CHANNELS; i++) {
		W_REG(osh, &d11regs->intctrlregs[i].intmask, 0);
		W_REG(osh, &d11regs->altintmask[i], 0);
	}

	/* Disable GCI interrupts */
	gciregs = si_setcore(sih, GCI_CORE_ID, 0);
	W_REG(osh, &gciregs->gci_intmask, 0);

	/* Disable LHL interrupts */
	W_REG(osh, &gciregs->lhl_wl_armtim0_intrp_adr, 0);
	W_REG(osh, &gciregs->lhl_wl_mactim0_intrp_adr, 0);
	W_REG(osh, &gciregs->lhl_wl_mactim1_intrp_adr, 0);
	W_REG(osh, &gciregs->gpio_int_en_port_adr[0], 0);
	W_REG(osh, &gciregs->gpio_int_en_port_adr[1], 0);
	W_REG(osh, &gciregs->gpio_int_en_port_adr[2], 0);

	/* Clear LHL ARM timer */
	W_REG(osh, &gciregs->lhl_wl_armtim0_adr, 0);

	/* Disable PMU interrupts */
	pmuregs = si_setcore(sih, PMU_CORE_ID, 0);
	W_REG(osh, &pmuregs->pmuintmask0, 0);
	W_REG(osh, &pmuregs->pmuintmask1, 0);

	/* Clear PMU ARM timer */
	W_REG(osh, &pmuregs->res_req_timer, 0);

	/* Clear LHL ARM timer interrupt status */
	LHL_REG(sih, lhl_wl_armtim0_st_adr, LHL_WL_ARMTIM0_ST_WL_ARMTIM_INT_ST,
			LHL_WL_ARMTIM0_ST_WL_ARMTIM_INT_ST);


	/* Clear LHL ARM timer interrupt status */
	LHL_REG(sih, lhl_wl_armtim0_st_adr, LHL_WL_ARMTIM0_ST_WL_ARMTIM_INT_ST,
			LHL_WL_ARMTIM0_ST_WL_ARMTIM_INT_ST);


	si_setcoreidx(sih, savecore);
}
static void
ulp_ds1_fast_pwrup_delay(si_t *sih, osl_t *osh)
{
	uint32 savecore;
	d11regs_t *regs;

	savecore = si_coreidx(sih);

	/* Disable D11 interrupts */
	regs = si_setcore(sih, D11_CORE_ID, 0);

	/* Configure DS1 fast power up delay */
	W_REG(osh, &regs->u.d11regs.scc_fastpwrup_dly, DS1_43012A0_FAST_PWRUP_DELAY);

	si_setcoreidx(sih, savecore);
}

/* This function clears pda configurations for socram, BM, UCM and SHM
*/
static void
ulp_clear_pda(si_t *sih)
{
	hndmem_sleeppda_config(sih, MEM_SOCRAM, PDA_CONFIG_CLEAR);
	hndmem_sleeppda_config(sih, MEM_BM, PDA_CONFIG_CLEAR);
	hndmem_sleeppda_config(sih, MEM_UCM, PDA_CONFIG_CLEAR);
	hndmem_sleeppda_config(sih, MEM_SHM, PDA_CONFIG_CLEAR);
}
static void
ulp_enter_sleeppda_config(si_t *sih)
{
	uint origidx = 0;
	uint corerev = 0;

	/* Get the current core index */
	origidx = si_coreidx(sih);

	/* First clear PDAs for all kind of memories */
	ulp_clear_pda(sih);

	if (sih->lpflags & LPFLAGS_SI_DS1_SLEEP_PDA_DISABLE) {
		goto exit;
	}

	/* DS1 Sleep PDA configuration */
	hndmem_sleeppda_config(sih, MEM_SOCRAM, PDA_CONFIG_SET_FULL);

	hndmem_sleeppda_bank_config(sih, MEM_BM, 1, PDA_CONFIG_SET_PARTIAL, 0xC);
	hndmem_sleeppda_bank_config(sih, MEM_BM, 2, PDA_CONFIG_SET_FULL, 0);

	/* Get d11 core rev after setting coreidx to d11 */
	si_setcore(sih, D11_CORE_ID, 0);
	corerev = si_corerev(sih);

	/* Switch back to originial core */
	si_setcoreidx(sih, origidx);

	/* If the size of ucode is <= 6K instructions, turn off Bank 1 */
	if (corerev == 60) {
		if (d11ucode_ulp60sz <= (6 * 1024 * 8))
			hndmem_sleeppda_bank_config(sih, MEM_UCM, 1, PDA_CONFIG_SET_FULL, 0);
	} else if (corerev == 62) {
		if (d11ucode_ulp62sz <= (6 * 1024 * 8))
			hndmem_sleeppda_bank_config(sih, MEM_UCM, 1, PDA_CONFIG_SET_FULL, 0);
	} else {
		/* P2P UCM PDA/PKILL is configured only for MAC rev 60 and 62
		* Add related changes here if needed for different MAC rev's
		*/
		ROMMABLE_ASSERT(0);
	}

	/* If the size of ucode is <= 7K instructions, turn off Bank 2 */
	if (corerev == 60) {
		if (d11ucode_ulp60sz <= (7 * 1024 * 8))
			hndmem_sleeppda_bank_config(sih, MEM_UCM, 2, PDA_CONFIG_SET_FULL, 0);
	} else if (corerev == 62) {
		if (d11ucode_ulp62sz <= (7 * 1024 * 8))
			hndmem_sleeppda_bank_config(sih, MEM_UCM, 2, PDA_CONFIG_SET_FULL, 0);
	} else {
		/* P2P UCM PDA/PKILL is configured only for MAC rev 60 and 62
		* Add related changes here if needed for different MAC rev's
		*/
		ROMMABLE_ASSERT(0);
	}

	hndmem_sleeppda_bank_config(sih, MEM_UCM, 3, PDA_CONFIG_SET_FULL, 0);

exit:
	return;
}

static void
ulp_exit_sleeppda_config(si_t *sih)
{
	uint origidx = 0;
	uint corerev = 0;

	/* Get the current core index */
	origidx = si_coreidx(sih);

	/* First clear PDAs for all kind of memories */
	ulp_clear_pda(sih);

	if (sih->lpflags & LPFLAGS_SI_DS0_SLEEP_PDA_DISABLE) {
		goto exit;
	}

	/* DS0 Sleep PDA configuration */
	hndmem_sleeppda_bank_config(sih, MEM_BM, 0, PDA_CONFIG_SET_PARTIAL, 0x8);
	hndmem_sleeppda_bank_config(sih, MEM_BM, 1, PDA_CONFIG_SET_PARTIAL, 0x1);
	hndmem_sleeppda_bank_config(sih, MEM_BM, 2, PDA_CONFIG_SET_PARTIAL, 0x80);

	/* Get d11 core rev after setting coreidx to d11 */
	si_setcore(sih, D11_CORE_ID, 0);
	corerev = si_corerev(sih);

	/* Switch back to originial core */
	si_setcoreidx(sih, origidx);

	if (corerev == 60) {
		/* If the size of ucode is <= 6K instructions, turn off Bank 1 */
		if (d11ucode_p2p60sz <= (6 * 1024 * 8))
			hndmem_sleeppda_bank_config(sih, MEM_UCM, 1, PDA_CONFIG_SET_FULL, 0);
	} else if (corerev == 62) {
		if (d11ucode_p2p62sz <= (6 * 1024 * 8))
			hndmem_sleeppda_bank_config(sih, MEM_UCM, 1, PDA_CONFIG_SET_FULL, 0);
	} else {
		/* ULP UCM PDA/PKILL is configured only for MAC rev 60 and 62
		* Add related changes here if needed for different MAC rev's
		*/
		ROMMABLE_ASSERT(0);
	}

	/* If the size of ucode is <= 7K instructions, turn off Bank 2 */
	if (corerev == 60) {
		if (d11ucode_p2p62sz <= (7 * 1024 * 8))
			hndmem_sleeppda_bank_config(sih, MEM_UCM, 2, PDA_CONFIG_SET_FULL, 0);
	} else if (corerev == 62) {
		if (d11ucode_p2p62sz <= (7 * 1024 * 8))
			hndmem_sleeppda_bank_config(sih, MEM_UCM, 2, PDA_CONFIG_SET_FULL, 0);
	} else {
		/* ULP UCM PDA/PKILL is configured only for MAC rev 60 and 62
		* Add related changes here if needed for different MAC rev's
		*/
		ROMMABLE_ASSERT(0);
	}

	hndmem_sleeppda_bank_config(sih, MEM_UCM, 3, PDA_CONFIG_SET_FULL, 0);

exit:
	return;
}

void
ulp_enter(si_t *sih, osl_t *osh, ulp_ext_info_t *einfo)
{
	ulp_post_ulpucode_switch(osh, einfo);

#ifdef BCMFCBS
	if (fcbsdata_pmuuptrigger_populate(sih) != BCME_OK) {
		ULP_ERR(("%s: FCBS pmu up trig populate failed!\n", __func__));
	}
	if (ulp_fcbs_init(sih, osh, FCBS_DS1) != BCME_OK) {
		ULP_ERR(("%s: FCBS init failed!\n", __func__));
	}
#endif /* BCMFCBS */

	if (CHIPID(sih->chip) == BCM43012_CHIP_ID) {
		si_pmu_ds1_res_init(sih, osh);
		ulp_ds1_fast_pwrup_delay(sih, osh);
		ulp_disable_interrupts(sih, osh);
	}
	si_lhl_timer_config(sih, osh, LHL_MAC_TIMER);
	si_lhl_timer_enable(sih);
	si_pmu_chipcontrol(sih, PMU_CHIPCTL2, PMUCCTL02_43012_LHL_TIMER_SELECT,
			PMUCCTL02_43012_LHL_TIMER_SELECT);

	si_lhl_setup(sih, osh);

	si_lhl_enable_sdio_wakeup(sih, osh);

#if defined(SAVERESTORE)
	if (SR_ENAB() && sr_cap(sih)) {
		/* Disable SR */
		sr_engine_enable(sih, IOV_SET, FALSE);
		/* Making RSRC 23 (Resource SR) UPDOWNTIME to 0 */
		PMU_REG(sih, res_table_sel, ~0x0, ULP_SR_RESOURCE);
		PMU_REG(sih, res_updn_timer, ~0x0, ULP_SR_UPDOWNTIME);
	}
#endif /* SAVERESTORE */

	ulp_enter_sleeppda_config(sih);
	si_pmu_ulp_chipconfig(sih, osh);

	/* Mark the next boot type (i.e. ULP exit) as warm boot path */
	if (sih->pmurev >= 30) {
		PMU_REG_NEW(sih, swscratch, ~0, WARM_BOOT);
	} else {
		CHIPC_REG(sih, flashdata, ~0, WARM_BOOT);
	}
}

void
ulp_exit(si_t *sih, osl_t *osh)
{
	if (!si_is_warmboot()) {
		ULP_DBG(("%s: coldboot!\n", __func__));
		goto exit;
	}

	ULP_DBG(("%s: warmboot!\n", __func__));

	ulp_exit_sleeppda_config(sih);
exit:
	return;
}

#ifdef ULP_DBG_ON
void
ulp_dump_cache(uint8 *ulp_cache)
{
	ulp_info_t *ui = ulp_get_uinfo();
	ulp_cubby_list_t *ucl = ulp_get_clist(ui, ULP_PHASE1);
	uint8 *cubby_cur_ptr = NULL;
	cache_hdr_t *chdr = NULL;
	ulp_p1_cubby_ctx_t *p1cc = NULL;
	const ulp_p1_module_pubctx_t *pubc = NULL;
	int i = 0;

	chdr = (cache_hdr_t *)ulp_cache;
	cubby_cur_ptr = ulp_cache + sizeof(*chdr);
	ULP_DBG(("***dump_cache:%p:hdr:crc:0x%x,sz:%d\n", ulp_cache, chdr->crc, chdr->gtotsz));

	for (i = 0; i < ucl->cur; i++) {
		p1cc = ULP_GET_CUBBYCTX(ucl, i);
		pubc = p1cc->cubby_pub_ctx;

		ULP_DBG(("mid:%d:%s:sz:%d\n", p1cc->mod_id,
			(pubc->flags & MODCBFL_CTYPE_DYNAMIC)?"dynamic":"static", p1cc->csz));
		ULP_DBG(("cacheptr: %p\n", cubby_cur_ptr));
		/* specifically for dynamic */
		if (pubc->flags & MODCBFL_CTYPE_DYNAMIC) {
			ULP_DBG(("[in cache]dyn_sz:%d\n", *((uint32*)cubby_cur_ptr)));
			prhex("modulecache", cubby_cur_ptr, p1cc->csz + SECTION_SZ_HOLDER);
			cubby_cur_ptr += *((uint32*)cubby_cur_ptr) + SECTION_SZ_HOLDER;
		} else {
			prhex("modulecache", cubby_cur_ptr, p1cc->csz);
			cubby_cur_ptr += p1cc->csz;
		}
	}
}
#endif /* ULP_DBG_ON */

/* Generic function to copy data to Driver SHM block */
void
ulp_write_driver_shmblk(si_t *sih, uint coreunit, uint offset, const void* buf, int len)
{
	ulp_info_t *ui = ulp_get_uinfo();
	uint16 drv_blk_addr = ui->driver_blk_addr;

	ULP_DBG(("[%s - %d] drv_blk_addr = [%x]\n", __FUNCTION__, __LINE__, drv_blk_addr));
	hndd11_copyto_shm(sih, 0, (drv_blk_addr * 2) + offset, buf, len);
}

/* Generic function to copy data from Driver SHM block */
void
ulp_read_driver_shmblk(si_t *sih, uint coreunit, uint offset, void* buf, int len)
{
	ulp_info_t *ui = ulp_get_uinfo();
	uint16 drv_blk_addr = ui->driver_blk_addr;
	ULP_DBG(("[%s - %d] drv_blk_addr = [%x]\n", __FUNCTION__, __LINE__, drv_blk_addr));
	hndd11_copyfrom_shm(sih, 0, (drv_blk_addr * 2) + offset, buf, len);
}

uint32
ulp_mac_ulp_features_support(si_t *sih)
{
	uint32 sup_feat = 0;
	ulp_info_t *ui = ulp_get_uinfo();

	BCM_REFERENCE(ui);
	switch (CHIPID(ui->sih->chip)) {
		case BCM43012_CHIP_ID:
			sup_feat = C_P2P_NOA | C_INFINITE_NOA | C_P2P_CTWIN |
				C_P2P_GC | C_BCN_TRIM | C_BT_COEX | C_LTE_COEX |
				C_ADS1 | C_LTECX_PSPOLL_PRIO_EN | C_ULP_SLOWCAL_SKIP;
			break;
		default:
			/* returns sup_feat as 0 */
			break;
	}

	return sup_feat;
}

uint32
ulp_mac_ulp_features_default(si_t *sih)
{
	uint32 default_feat = 0;
	ulp_info_t *ui = ulp_get_uinfo();

	BCM_REFERENCE(ui);
	switch (CHIPID(ui->sih->chip)) {
		case BCM43012_CHIP_ID:
			default_feat = C_BT_COEX | C_LTE_COEX |
				C_LTECX_PSPOLL_PRIO_EN | C_ULP_SLOWCAL_SKIP;
		break;
		default:
			/* returns default_feat as 0 */
		break;
	}

	return default_feat;
}
